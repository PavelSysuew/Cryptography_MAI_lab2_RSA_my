//  Лабораторная работа № 2 по основам криптографии. Задание 4.
//  Атака Винера на открытый ключ алгоритма RSA.
//
//  Требования задания:
//    * сервис с компонентным функционалом для атаки Винера;
//    * для открытого ключа (n, e) найти:
//        - секретную экспоненту d;
//        - значение phi(n);
//        - коллекцию подходящих дробей e/n, вычисленных в ходе атаки;
//    * реализовать генератор ключевых пар, уязвимых к атаке Винера;
//    * продемонстрировать атаку на таких ключах.
//
//  Ограничения (по заданию):
//    * Python не используется;
//    * готовые реализации криптоалгоритмов не используются;
//    * длинная арифметика разрешена -> используем Boost.Multiprecision.
//
//  Идея атаки Винера (кратко):
//    Если у RSA-ключа секретная экспонента удовлетворяет условию
//        d < (1/3) * n^(1/4),
//    то d является знаменателем одной из подходящих дробей разложения
//    e/n в цепную дробь. Перебирая подходящие дроби k/d, для каждой
//    проверяем гипотезу  e*d - 1 = k*phi(n):
//      1) phi = (e*d - 1) / k  — должно быть целым (и чётным);
//      2) по phi находим  p + q = n - phi + 1;
//      3) p и q — корни  x^2 - (p+q)x + n = 0; дискриминант
//         D = (p+q)^2 - 4n должен быть полным квадратом;
//      4) если p*q == n — атака удалась, d найдена.

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

#include <algorithm>
#include <cstdlib>
#include <string_view>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <optional>
#include <random>
#include <utility>
#include <vector>

using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;          // индекс старшего бита

using std::cout;
using std::endl;
using std::optional;
using std::vector;

//  БЛОК 1. Вспомогательные теоретико-числовые компоненты
//  (по заданию 1 эти компоненты должны быть реализованы вручную —
//   здесь они нужны для генератора ключей и для самой атаки).

// Модуль длинного целого
static cpp_int absVal(const cpp_int& x) { return x < 0 ? -x : x; }

// ---- НОД по алгоритму Евклида ---------------------------------------
static cpp_int gcdEuclid(cpp_int a, cpp_int b) {
    a = absVal(a);
    b = absVal(b);
    while (b != 0) {
        cpp_int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// ---- Неотрицательный остаток a mod m --------------------------------
static cpp_int modPos(const cpp_int& a, const cpp_int& m) {
    cpp_int r = a % m;
    if (r < 0) r += m;
    return r;
}

// ---- Расширенный алгоритм Евклида: a*x + b*y = gcd(a,b) -------------
// Возвращает gcd(a,b), по ссылкам — коэффициенты Безу x, y.
static cpp_int extGcd(const cpp_int& a, const cpp_int& b,
                      cpp_int& x, cpp_int& y) {
    if (b == 0) {          // база рекурсии
        x = 1;
        y = 0;
        return a;
    }
    cpp_int x1, y1;
    cpp_int g = extGcd(b, a % b, x1, y1);
    // из b*x1 + (a mod b)*y1 = g  выражаем a*x + b*y = g
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

// ---- Обратный элемент по модулю -------------------------------------
// Возвращает a^(-1) mod m либо std::nullopt, если обратного нет.
static optional<cpp_int> modInverse(const cpp_int& a, const cpp_int& m) {
    cpp_int x, y;
    cpp_int g = extGcd(modPos(a, m), m, x, y);
    if (g != 1) return std::nullopt;       // gcd != 1 -> обратного нет
    return modPos(x, m);
}

// ---- Целочисленный квадратный корень floor(sqrt(n)) ----------------
// Метод Ньютона для длинных целых.
static cpp_int isqrt(cpp_int n) {
    if (n < 2) return n;
    // начальное приближение 2^(ceil(bits/2)) сверху
    unsigned bits = msb(n) + 1;
    cpp_int x = cpp_int(1) << ((bits + 1) / 2);
    while (true) {
        cpp_int y = (x + n / x) >> 1;      // y = (x + n/x) / 2
        if (y >= x) break;                 // сошлись
        x = y;
    }
    // страховочная коррекция (на случай ошибок округления)
    while (x * x > n) --x;
    while ((x + 1) * (x + 1) <= n) ++x;
    return x;
}

// ---- Проверка, что n — полный квадрат; root = floor(sqrt(n)) -------
static bool isPerfectSquare(const cpp_int& n, cpp_int& root) {
    if (n < 0) return false;
    root = isqrt(n);
    return root * root == n;
}

// ---- Генератор псевдослучайных чисел --------------------------------
static std::mt19937_64& rng() {
    static std::mt19937_64 gen{std::random_device{}()};
    return gen;
}

// Случайное число из [0, 2^bits)
static cpp_int randomBits(unsigned bits) {
    cpp_int r = 0;
    for (unsigned i = 0; i < bits; i += 64) {
        r <<= 64;
        r += rng()();
    }
    return r & ((cpp_int(1) << bits) - 1); // обрезаем до bits бит
}

// Случайное нечётное число ровно с bits битами (верхний бит установлен)
static cpp_int randomOddBits(unsigned bits) {
    cpp_int r = randomBits(bits);
    r |= (cpp_int(1) << (bits - 1));      // верхний бит = 1
    r |= 1;                               // нечётность
    return r;
}

// ---- Возведение в степень по модулю (square-and-multiply) ----------
static cpp_int modPow(cpp_int base, cpp_int exp, const cpp_int& m) {
    base = modPos(base, m);
    cpp_int result = 1;
    while (exp > 0) {
        if ((exp & 1) != 0) result = modPos(result * base, m);
        base = modPos(base * base, m);
        exp >>= 1;
    }
    return result;
}

// ---- Вероятностный тест простоты Миллера–Рабина --------------------
// Нужен только генератору ключей (для поиска простых p и q).
static bool isProbablePrime(const cpp_int& n, unsigned rounds = 40) {
    if (n < 2) return false;
    // быстрый отсев по малым простым
    static const int smallPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int p : smallPrimes) {
        if (n == p) return true;
        if (n % p == 0) return false;
    }
    // n - 1 = 2^s * t, где t нечётно
    cpp_int t = n - 1;
    unsigned s = 0;
    while ((t & 1) == 0) { t >>= 1; ++s; }

    for (unsigned i = 0; i < rounds; ++i) {
        // случайное основание a из [2, n-2]
        cpp_int a = modPos(randomBits(msb(n) + 1), n - 3) + 2;
        cpp_int x = modPow(a, t, n);
        if (x == 1 || x == n - 1) continue;   // свидетель простоты
        bool composite = true;
        for (unsigned j = 1; j < s; ++j) {
            x = modPos(x * x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;          // нашли свидетеля составности
    }
    return true;                              // вероятно простое
}

// ---- Генерация простого числа заданной битовой длины ----------------
static cpp_int randomPrime(unsigned bits) {
    while (true) {
        cpp_int candidate = randomOddBits(bits);
        if (isProbablePrime(candidate)) return candidate;
    }
}

//  БЛОК 2. Сервис атаки Винера

// Подходящая дробь p/q
struct Convergent {
    cpp_int numerator;    // p_i
    cpp_int denominator;  // q_i
};

// Результат работы сервиса атаки
struct WienerResult {
    bool   success    = false;  // удалось ли восстановить ключ
    cpp_int d;                  // найденная секретная экспонента
    cpp_int phi;                // значение phi(n) от модуля RSA
    cpp_int p, q;               // простые множители n (если найдены)
    size_t matchedIndex = 0;    // индекс подходящей дроби, на которой успех

    // Коллекция ВСЕХ подходящих дробей e/n, вычисленных в ходе атаки
    vector<Convergent> convergents;
};

class WienerAttackService {
public:
    //  Компонент 1. Разложение рационального числа num/den в цепную
    //  дробь: возвращает список неполных частных [a0; a1, ..., am].
    static vector<cpp_int> continuedFraction(cpp_int num, cpp_int den) {
        vector<cpp_int> partialQuotients;
        while (den != 0) {
            cpp_int q = num / den;                 // очередное неполное частное
            partialQuotients.push_back(q);
            cpp_int r = num % den;                 // остаток
            num = den;
            den = r;
        }
        return partialQuotients;
    }

    //  Компонент 2. Вычисление подходящих дробей по списку неполных
    //  частных (рекуррентные формулы):
    //      p_i = a_i * p_{i-1} + p_{i-2}
    //      q_i = a_i * q_{i-1} + q_{i-2}
    //  с начальными условиями p_{-2}=0, p_{-1}=1, q_{-2}=1, q_{-1}=0.
    static vector<Convergent> convergentsOf(const cpp_int& num,
                                            const cpp_int& den) {
        vector<cpp_int> a = continuedFraction(num, den);

        vector<Convergent> result;
        cpp_int p_prev2 = 0, p_prev1 = 1;
        cpp_int q_prev2 = 1, q_prev1 = 0;

        for (const cpp_int& ai : a) {
            cpp_int p = ai * p_prev1 + p_prev2;
            cpp_int q = ai * q_prev1 + q_prev2;
            result.push_back({p, q});
            // сдвигаем «окно» истории
            p_prev2 = p_prev1; p_prev1 = p;
            q_prev2 = q_prev1; q_prev1 = q;
        }
        return result;
    }

    //  Компонент 3. Проверка одной подходящей дроби k/d как кандидата
    //  на восстановление ключа:
    //    k  — числитель подходящей дроби (кандидат в множитель phi),
    //    d  — знаменатель (кандидат в секретную экспоненту).
    //  Возвращает пару (phi, {p,q}) при успехе.
    static optional<std::pair<cpp_int, std::pair<cpp_int, cpp_int>>>
    tryCandidate(const cpp_int& n, const cpp_int& e,
                 const cpp_int& k, const cpp_int& d) {
        // k = 0 быть не может (e*d - 1 = 0*phi невозможно), d = 0 тоже
        if (k == 0 || d == 0) return std::nullopt;

        // Так как phi(n) чётно, а e*d - 1 = k*phi нечётно,
        // то и d, и k обязаны быть нечётными. Это лишь эвристика,
        // ускоряющая перебор (на корректность не влияет).
        if ((d & 1) == 0) return std::nullopt;

        // Гипотеза: e*d - 1 = k * phi(n)  =>  phi = (e*d - 1) / k
        cpp_int ed_minus_1 = e * d - 1;
        if (ed_minus_1 % k != 0) return std::nullopt;   // не делится нацело
        cpp_int phi = ed_minus_1 / k;
        if (phi <= 0 || (phi & 1) != 0) return std::nullopt;

        // Из phi = (p-1)(q-1) = n - (p+q) + 1  =>  p + q = n - phi + 1
        cpp_int sumPQ = n - phi + 1;
        if (sumPQ <= 0) return std::nullopt;

        // p и q — корни x^2 - (p+q)x + n = 0, дискриминант D = (p+q)^2 - 4n
        cpp_int D = sumPQ * sumPQ - 4 * n;
        if (D < 0) return std::nullopt;

        cpp_int root;
        if (!isPerfectSquare(D, root)) return std::nullopt;  // D не полный квадрат

        // p = (s + sqrt(D)) / 2, q = (s - sqrt(D)) / 2
        if (modPos(sumPQ + root, cpp_int(2)) != 0) return std::nullopt;

        cpp_int p = (sumPQ + root) / 2;
        cpp_int q = (sumPQ - root) / 2;
        if (p <= 1 || q <= 1) return std::nullopt;
        if (p * q != n)       return std::nullopt;   // контрольная проверка

        return std::make_pair(phi, std::make_pair(p, q));
    }

    //  Главный компонент. Атака Винера по открытому ключу (n, e).
    //  Возвращает WienerResult: найденные d, phi(n), p, q и коллекцию
    //  всех вычисленных подходящих дробей e/n.
    WienerResult attack(const cpp_int& n, const cpp_int& e) const {
        WienerResult result;

        // 1. Цепная дробь e/n и коллекция всех подходящих дробей.
        //    (Именно эта коллекция требуется в задании.)
        result.convergents = convergentsOf(e, n);

        // 2. Перебираем подходящие дроби k/d и проверяем каждую
        //    как кандидата в (множитель phi; секретная экспонента).
        for (size_t i = 0; i < result.convergents.size(); ++i) {
            const Convergent& c = result.convergents[i];
            auto found = tryCandidate(n, e, c.numerator, c.denominator);
            if (found.has_value()) {
                result.success      = true;
                result.phi          = found->first;
                result.p            = found->second.first;
                result.q            = found->second.second;
                result.d            = c.denominator;
                result.matchedIndex = i;
                return result;
            }
        }
        // Ни одна подходящая дробь не подошла — ключ не уязвим.
        return result;
    }

    //  Дополнительный компонент. Проверка условия уязвимости по
    //  теореме Винера: d < (1/3) * n^(1/4).
    static bool isVulnerable(const cpp_int& n, const cpp_int& d) {
        cpp_int fourthRoot = isqrt(isqrt(n));   // floor(n^(1/4))
        return 3 * d < fourthRoot;              // d < n^(1/4)/3
    }
};

//  БЛОК 3. Генератор ключевых пар RSA

struct RSAKeyPair {
    cpp_int p, q;   // простые множители
    cpp_int n;      // модуль  n = p*q
    cpp_int phi;    // phi(n) = (p-1)(q-1)
    cpp_int e;      // открытая экспонента
    cpp_int d;      // секретная экспонента, e*d ≡ 1 (mod phi)
};

class VulnerableKeyGenerator {
public:
    //  Генератор ключа, УЯЗВИМОГО к атаке Винера.
    //
    //  Схема генерации:
    //    1) выбираем два простых p, q заданной битовой длины;
    //    2) n = p*q, phi = (p-1)(q-1);
    //    3) выбираем МАЛУЮ d, удовлетворяющую условию Винера
    //       d < (1/3)*n^(1/4), и gcd(d, phi) = 1;
    //    4) открытая экспонента e = d^(-1) mod phi  (получается
    //       порядка phi, то есть «большая», как и требуется).
    RSAKeyPair generate(unsigned bitsP, unsigned bitsQ) {
        while (true) {
            RSAKeyPair key;

            // 1. Простые множители.
            key.p = randomPrime(bitsP);
            key.q = randomPrime(bitsQ);
            if (key.p == key.q) continue;

            // Чтобы ключ не был тривиально уязвим ещё и к атаке Ферма,
            // потребуем, чтобы |p - q| не было слишком маленьким.
            cpp_int minDiff = cpp_int(1) << (std::min(bitsP, bitsQ) / 2);
            if (absVal(key.p - key.q) <= minDiff) continue;

            key.n   = key.p * key.q;
            key.phi = (key.p - 1) * (key.q - 1);
            if (key.phi == 0) continue;

            // 2. Верхняя граница для d: floor(n^(1/4)) / 3
            //    (берём -1 для строгого неравенства теоремы Винера).
            cpp_int fourthRoot = isqrt(isqrt(key.n));
            cpp_int dMax = fourthRoot / 3 - 1;
            if (dMax < 3) continue;                 // ключ слишком мал

            // 3. Случайная малая d в диапазоне [3, dMax], взаимно простая с phi.
            unsigned bitsD = msb(dMax) + 1;         // сколько бит хватает для d
            for (int attempt = 0; attempt < 5000; ++attempt) {
                cpp_int d = modPos(randomBits(bitsD), dMax - 2) + 3; // [3, dMax]
                if ((d & 1) == 0) ++d;              // d должно быть нечётным
                if (d < 3 || d > dMax) continue;
                if (gcdEuclid(d, key.phi) != 1) continue;

                // 4. Открытая экспонента e = d^(-1) mod phi
                auto inv = modInverse(d, key.phi);
                if (!inv.has_value()) continue;

                key.d = d;
                key.e = *inv;
                return key;
            }
            // если не удалось подобрать d — генерируем новые p, q
        }
    }

    //  Генератор «нормального» (не уязвимого к Винеру) ключа — нужен
    //  для демонстрации того, что атака на стойком ключе не работает.
    RSAKeyPair generateSecure(unsigned bitsP, unsigned bitsQ) {
        while (true) {
            RSAKeyPair key;
            key.p = randomPrime(bitsP);
            key.q = randomPrime(bitsQ);
            if (key.p == key.q) continue;

            cpp_int minDiff = cpp_int(1) << (std::min(bitsP, bitsQ) / 2);
            if (absVal(key.p - key.q) <= minDiff) continue;

            key.n   = key.p * key.q;
            key.phi = (key.p - 1) * (key.q - 1);
            if (key.phi == 0) continue;

            // Большая случайная d (порядка n), заведомо нарушающая
            // условие Винера d < (1/3)*n^(1/4).
            unsigned bitsD = msb(key.n) - 1;
            if (bitsD < 4) continue;

            for (int attempt = 0; attempt < 5000; ++attempt) {
                cpp_int d = randomOddBits(bitsD);
                if (d < 3) continue;
                if (gcdEuclid(d, key.phi) != 1) continue;

                auto inv = modInverse(d, key.phi);
                if (!inv.has_value()) continue;

                key.d = d;
                key.e = *inv;
                return key;
            }
        }
    }
};

// В файле или канале оставляем обычный текст без управляющих кодов.
static bool useColor() {
    const char* term = std::getenv("TERM");
    if (std::getenv("NO_COLOR") || (term && std::string_view(term) == "dumb")) return false;
#ifdef _WIN32
    return _isatty(1) && (term || std::getenv("WT_SESSION"));
#else
    return isatty(STDOUT_FILENO);
#endif
}

static void printStatus(bool success) {
    const bool color = useColor();
    if (color) cout << (success ? "\033[32m" : "\033[31m");
    cout << (success ? "Взлом удался" : "Взлом не удался");
    if (color) cout << "\033[0m";
    cout << '\n';
}

static void printConvergents(const vector<Convergent>& conv) {
    cout << "  Подходящие дроби (" << conv.size() << " шт.)\n";
    for (size_t i = 0; i < conv.size(); ++i) {
        cout << "    [" << i << "] " << conv[i].numerator
             << " / " << conv[i].denominator << "\n";
    }
}

// Полный цикл демонстрации для одного уязвимого ключа
static void demonstrateVulnerableKey(WienerAttackService& service,
                                     VulnerableKeyGenerator& keygen,
                                     unsigned bitsP, unsigned bitsQ) {
    RSAKeyPair key = keygen.generate(bitsP, bitsQ);

    cout << "\nУчебный ключ (p — " << bitsP << " бит, q — " << bitsQ << " бит)\n";
    cout << "n   = " << key.n   << "\n";
    cout << "e   = " << key.e   << "\n";
    cout << "d   = " << key.d   << "   (исходное)\n";
    cout << "phi = " << key.phi << "   (исходное)\n";

    // Запускаем атаку
    WienerResult res = service.attack(key.n, key.e);

    printStatus(res.success);
    printConvergents(res.convergents);

    if (res.success) {
        cout << "    индекс подходящей дроби = " << res.matchedIndex << '\n';
        cout << "    найденное d     = " << res.d
             << (res.d == key.d ? "   (совпадает)" : "   (расхождение)") << "\n";
        cout << "    найденное phi(n)= " << res.phi
             << (res.phi == key.phi ? "   (совпадает)" : "   (расхождение)") << "\n";
        cout << "    найденные p, q  = " << res.p << ", " << res.q << "\n";
        cout << "    проверка p*q==n : " << (res.p * res.q == key.n ? "да" : "нет") << "\n";

        // Проверка работоспособности найденного d на шифровании/расшифровании
        cpp_int m  = modPos(randomBits(msb(key.n)), key.n);   // сообщение
        cpp_int c  = modPow(m, key.e, key.n);                 // шифрование
        cpp_int m2 = modPow(c, res.d, key.n);                 // расшифрование
        cout << "    проверка расшифрования найденным d:\n";
        cout << "      m  = " << m  << "\n";
        cout << "      c  = " << c  << "\n";
        cout << "      m' = " << m2 << "\n";
        cout << "      m' == m : " << (m2 == m ? "OK" : "FAIL") << "\n";
    }
}

int main() {
    // Устанавливаем русскую локаль для консоли Windows (на Linux — no-op,
    // там UTF-8 по умолчанию); на корректность работы не влияет.
    setlocale(LC_ALL, "");

    cout << "Лабораторная 2. Атака Винера на RSA\n";

    WienerAttackService    service;   // сервис атаки
    VulnerableKeyGenerator keygen;    // генератор ключей

    //  Часть 1. Демонстрация атаки на ключах, уязвимых к Винеру.
    //  Разные размеры: 64, 128, 256, 512 бит модуля n.
    demonstrateVulnerableKey(service, keygen, 32, 32);   //  n ~ 64  бит
    demonstrateVulnerableKey(service, keygen, 64, 64);   //  n ~ 128 бит
    demonstrateVulnerableKey(service, keygen, 128, 128); //  n ~ 256 бит
    demonstrateVulnerableKey(service, keygen, 256, 256); //  n ~ 512 бит

    //  Часть 2. Контроль: на стойком ключе (большая d) атака не работает.
    RSAKeyPair secure = keygen.generateSecure(128, 128);
    cout << "\nКонтрольный ключ\n";
    cout << "n   = " << secure.n   << "\n";
    cout << "e   = " << secure.e   << "\n";
    cout << "d   = " << secure.d   << "   (исходное)\n";

    WienerResult secureRes = service.attack(secure.n, secure.e);
    printStatus(secureRes.success);
    printConvergents(secureRes.convergents);
    if (secureRes.success) {
        cout << "  найденное d = " << secureRes.d << '\n';
    }
    cout << "  условие Винера выполнено: "
         << (WienerAttackService::isVulnerable(secure.n, secure.d)
                 ? "да" : "нет") << '\n';
    return 0;
}
