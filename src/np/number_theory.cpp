#include "np/number_theory.hpp"

#include <boost/multiprecision/cpp_dec_float.hpp>

#include <iostream>
#include <stdexcept>


int NumberTheory::legendre(const BigInt& a, const BigInt& p) const {

}
    

int NumberTheory::jacobi(const BigInt& a, const BigInt& n) const {
    
}

BigInt NumberTheory::gcd(const BigInt& a, const BigInt& b) const {
    BigInt v = abs(a), w = abs(b), tmp;
    while (w != 0) {
        tmp = v;
        v = w;
        w = tmp % w;
    }
    return v;
}

// Расширенный алгоритм Евклида
NumberTheory::ExtendedGcdResult NumberTheory::extended_gcd(const BigInt& a, const BigInt& b) const {
    if (a == 0 && b == 0) {
        throw std::invalid_argument("extended_gcd: хотя бы один аргумент должен быть ненулевым");
    }

    const BigInt abs_a = abs(a);
    const BigInt abs_b = abs(b);
    BigInt r0 = abs_a, x0 = 1, y0 = 0;
    BigInt r1 = abs_b, x1 = 0, y1 = 1;

    while (r1 != 0) {
        const BigInt q = r0 / r1;
        const BigInt next_r = r0 - q * r1;
        const BigInt next_x = x0 - q * x1;
        const BigInt next_y = y0 - q * y1;
        r0 = r1; r1 = next_r;
        x0 = x1; x1 = next_x;
        y0 = y1; y1 = next_y;
    }

    if (a < 0) x0 = -x0;
    if (b < 0) y0 = -y0;
    return {r0, x0, y0};
}


// Бинарный алгоритм Евклида
BigInt NumberTheory::binary_gcd(const BigInt& a, const BigInt& b) const {
    BigInt v = abs(a), w = abs(b);
    if (v == 0 && w > 0) return w;
    if (v > 0 && w == 0) return v;
    if (v == 0 && w == 0) {
        throw std::invalid_argument("binary_gcd: хотя бы один аргумент должен быть отличным от нуля");
    }

    unsigned long k = 0;


    // Пока оба числа чётны
    while (v % 2 == 0 && w % 2 == 0) {
        v = v >> 1;
        w = w >> 1;
        k += 1;
    }

    // Пока первое число чётно
    while (v % 2 == 0) {
        v = v >> 1;
    }

    BigInt tmp;  // Временная переменная
    // Будем повторять, пока не обнулим второе число
    while (w != 0) {
        while (w % 2 == 0) w = w >> 1;
        if (v > w) {
            // Обменяем значения местами
            tmp = v;
            v = w;
            w = tmp;
        }
        w = w - v;
    }
    return v << k;
}

BigInt NumberTheory::mod_pow(const BigInt& base, const BigInt& exp, const BigInt& mod) const {
    
}

// Функция Элера через сумму единиц
BigInt NumberTheory::euler_phi_by_definition(const BigInt& n) const {
    if (n < 1) {
        throw std::invalid_argument("Аргумент в функции Эйлера должен быть натуральным числом");
    }

    BigInt phi = 0;
    for (BigInt i = 1; i <= n; i++) {
        if (NumberTheory::binary_gcd(n, i) == 1) phi++;
    }
    return phi;

}

// Функция Эйлера через каноническое разложение
BigInt NumberTheory::euler_phi_by_factorization(const BigInt& n) const {
    if (n < 1) {
        throw std::invalid_argument(
            "Аргумент функции Эйлера должен быть натуральным числом"
        );
    }

    BigInt phi = n;
    BigInt value = n;

    // Отдельно обрабатываем единственный чётный простой делитель.
    if (value % 2 == 0) {
        phi -= phi / 2;
        while (value % 2 == 0) {
            value /= 2;
        }
    }

    // После 2 достаточно проверять только нечётные делители.
    for (BigInt p = 3; p * p <= value; p += 2) {
        if (value % p == 0) {
            phi -= phi / p;

            // Удаляем из value все степени p.
            while (value % p == 0) {
                value /= p;
            }
        }
    }

    // Оставшийся множитель является простым
    if (value > 1) {
        phi -= phi / value;
    }

    return phi;
}

// Функция Эйлера через ДПФ
BigInt NumberTheory::euler_phi_by_fourier(const BigInt& n) const {
    if (n < 1) {
        throw std::invalid_argument(
            "Аргумент функции Эйлера должен быть натуральным числом"
        );
    }

    using BigFloat = boost::multiprecision::cpp_dec_float_100;

    const BigFloat pi = acos(BigFloat(-1));
    const BigFloat n_float = n.convert_to<BigFloat>();
    BigFloat phi = 0;

    for (BigInt k = 1; k <= n; ++k) {
        const BigFloat k_float = k.convert_to<BigFloat>();
        const BigFloat gcd_float = gcd(k, n).convert_to<BigFloat>();
        const BigFloat angle = 2 * pi * k_float / n_float;

        phi += gcd_float * cos(angle);
    }

    // Точный результат целый, поэтому убираем погрешность ДПФ округлением.
    return round(phi).convert_to<BigInt>();
}



// Функция Кармайкла
BigInt NumberTheory::carmichael_lambda(const BigInt& n) const {
    if (n < 1) {
        throw std::invalid_argument("carmichael_lambda: аргумент функции Кармайкла должен быть натуральным числом");
    }
    BigInt L = 1, value = n;
    unsigned long long alpha = 0; // Количество одинаковвых простых множителей
    BigInt lambda; // Значение фуункции Кармайкла для простого нечётного числа и чётного

    // Случай 1. Если число чётное
    if (value % 2 == 0) {
        // Число содержит двойки в разложении
        while (value % 2 == 0) {
            value /= 2;
            alpha += 1;
        }
        if (alpha == 1) lambda = 1;
        else if (alpha == 2) lambda = 2;
        else lambda = BigInt(1) << (alpha - 2);
        L = lambda;
    }
    
    

    // Случай 2. Обработка нечётных простых делителей
    for (BigInt p = 3; p * p <= value; p += 2) {
        
        if (value % p == 0) {
            lambda = p - 1;
            while (value % p == 0) {
                value /= p;
                lambda *= p;
            }
            lambda /= p;
            L = (L * lambda) / (NumberTheory::binary_gcd(L, lambda));
        }
    }

    if (value > 1) {
        // Есть ещё один простой множитель
        lambda = value - 1;
        L = (L * lambda) / (NumberTheory::binary_gcd(L, lambda));
    }

    return L;

   
}



int menu_number_theory() {
    std::cout << "МЕНЮ. Математичсекие функции теории чисел" << std::endl;
    std::cout << "1.  Вычисление значения символа Лежандра" << std::endl;
    std::cout << "2.  Вычисление значения символа Якоби" << std::endl;
    std::cout << "3.  Алгоритм Евклида" << std::endl;
    std::cout << "4.  Расширенный алгоритм Евклида" << std::endl;
    std::cout << "5.  Бинарный алгоритм Евклида" << std::endl;
    std::cout << "6.  Выполнение операции возведени в степень по модулю" << std::endl;
    std::cout << "7.  Вычисление функции Эйлера через сумму единиц" << std::endl;
    std::cout << "8.  Вычисление функции Эйлера через каноническое разложение числа по основно1 теореме арифметики" << std::endl;
    std::cout << "9.  Вычисление функции Эйлера через дискретное преобразование Фурье" << std::endl;
    std::cout << "10. Выисление фнкции Кармайкла" << std::endl;
    std::cout << "11. Вернуться в главное меню" << std::endl;
    std::cout << "Введите номер пункта, который хотите выполнить >> ";
    int n; std::cin >> n;
    std::cout << std::endl;
    return n;
}


// Печатает подсказку и читает одно длинное число с клавиатуры
static BigInt read_big(const char* prompt) {
    std::cout << prompt;
    BigInt x;
    std::cin >> x;
    return x;
}


void number_theory() {
    const NumberTheory nt;   // сервис без состояния: одного объекта хватает на всё меню

    while(true) {
        int n_menu = menu_number_theory();

        // Ошибка ввода не завершает всю демонстрацию.
        try {
            switch (n_menu)
            {
                case 1: {
                    BigInt a = read_big("a = ");
                    BigInt p = read_big("p (нечётное простое) = ");
                    std::cout << "(" << a << "/" << p << ") = " << nt.legendre(a, p) << std::endl;
                    break;
                }
                case 2: {
                    BigInt a = read_big("a = ");
                    BigInt n = read_big("n (нечётное положительное) = ");
                    std::cout << "(" << a << "/" << n << ") = " << nt.jacobi(a, n) << std::endl;
                    break;
                }
                case 3: {
                    BigInt a = read_big("a = ");
                    BigInt b = read_big("b = ");
                    std::cout << "gcd(" << a << ", " << b << ") = " << nt.gcd(a, b) << std::endl;
                    break;
                }
                case 4: {
                    BigInt a = read_big("a = ");
                    BigInt b = read_big("b = ");
                    NumberTheory::ExtendedGcdResult r = nt.extended_gcd(a, b);
                    std::cout << "gcd = " << r.gcd << ", x = " << r.x << ", y = " << r.y << std::endl;
                    std::cout << "Проверка: " << a << "*" << r.x << " + " << b << "*" << r.y
                              << " = " << a * r.x + b * r.y << std::endl;
                    break;
                }
                case 5: {
                    BigInt a = read_big("a = ");
                    BigInt b = read_big("b = ");
                    std::cout << "binary_gcd(" << a << ", " << b << ") = " << nt.binary_gcd(a, b) << std::endl;
                    break;
                }
                case 6: {
                    BigInt base = read_big("основание = ");
                    BigInt exp  = read_big("показатель = ");
                    BigInt mod  = read_big("модуль = ");
                    std::cout << base << "^" << exp << " mod " << mod << " = "
                              << nt.mod_pow(base, exp, mod) << std::endl;
                    break;
                }
                case 7: {
                    BigInt n = read_big("n = ");
                    std::cout << "phi(" << n << ") = " << nt.euler_phi_by_definition(n) << std::endl;
                    break;
                }
                case 8: {
                    BigInt n = read_big("n = ");
                    std::cout << "phi(" << n << ") = " << nt.euler_phi_by_factorization(n) << std::endl;
                    break;
                }
                case 9: {
                    BigInt n = read_big("n = ");
                    std::cout << "phi(" << n << ") = " << nt.euler_phi_by_fourier(n) << std::endl;
                    break;
                }
                case 10: {
                    BigInt n = read_big("n = ");
                    std::cout << "lambda(" << n << ") = " << nt.carmichael_lambda(n) << std::endl;
                    break;
                }
                case 11: return;
                default:
                    std::cout << "Неверный номер пункта меню. Пожалуйста, выберите корректный пункт." << std::endl;
                    break;
            }
        } catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
}



