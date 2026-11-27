#include "rsa/rsa.hpp"

#include <random>
#include <stdexcept>
#include <utility>

#include "primality/fermat_test.h"
#include "primality/miller_rabin_test.h"
#include "primality/solovay_strassen_test.h"


namespace {

const IPrimalityTest& select_primality_test(RSA::PrimalityTestType test_type) {
    static const FermatTest fermat;
    static const SolovayStrassenTest solovay_strassen;
    static const MillerRabinTest miller_rabin;

    switch (test_type) {
        case RSA::PrimalityTestType::Fermat: return fermat;
        case RSA::PrimalityTestType::SolovayStrassen: return solovay_strassen;
        case RSA::PrimalityTestType::MillerRabin: return miller_rabin;
    }
    throw std::invalid_argument("Неизвестный тест простоты");
}

// Старший и младший биты равны 1. Между ними записываем случайные биты.
BigInt random_odd_number(std::size_t bit_length) {
    thread_local std::random_device generator;
    std::uniform_int_distribution<unsigned int> random_byte(0, 255);

    BigInt number = 1;
    std::size_t bits_left = bit_length - 2;

    while (bits_left >= 8) {
        number <<= 8;
        number += random_byte(generator);
        bits_left -= 8;
    }

    if (bits_left > 0) {
        number <<= bits_left;
        number += random_byte(generator) >> (8 - bits_left);
    }

    number <<= 1;
    number += 1;
    return number;
}

bool has_fermat_distance(const BigInt& p, const BigInt& q, const BigInt& n,
                         const BigInt& min_steps) {
    // Метод Ферма начинает с ceil(sqrt(n)), а множители находит в (p + q) / 2.
    // Проверяем, что между этими значениями не меньше min_steps шагов.
    const BigInt limit = (p + q) / 2 - min_steps;
    return limit > 0 && limit * limit >= n;
}

bool small_exponent(const BigInt& n, const BigInt& d) {
    const std::size_t bit_length = static_cast<std::size_t>(boost::multiprecision::msb(n)) + 1;

    // Сравниваем d с 2^(bit_length / 2) через квадраты, чтобы обойтись целыми числами.
    const BigInt squared_limit = BigInt(1) << bit_length;
    return d * d <= squared_limit;
}

} // namespace

RSA::RSA(PrimalityTestType test_type, double probability, std::size_t bit_length)
    : key_generator(std::in_place, test_type, probability, bit_length) {
}

RSA::KeyGenerator::KeyGenerator(PrimalityTestType test_type, double probability,
                                std::size_t bit_length)
    : test_type(test_type), probability(probability), bit_length(bit_length) {
    if (!(probability >= 0.5 && probability < 1.0)) {
        throw std::invalid_argument("Надёжность должна быть в диапазоне [0.5, 1)");
    }
    if (bit_length < 10) {
        throw std::invalid_argument("Битовая длина p и q должна быть не меньше 10");
    }
}

BigInt RSA::KeyGenerator::generate_prime(const IPrimalityTest& test) const {
    while (true) {
        const BigInt candidate = random_odd_number(bit_length);
        if (test.is_prime(candidate, probability)) {
            return candidate;
        }
    }
}

RSA::KeyPair RSA::KeyGenerator::generate() const {
    const IPrimalityTest& test = select_primality_test(test_type);
    const NumberTheory nt{};
    const BigInt e = 65537;
    const BigInt min_fermat_steps = BigInt(1) << (bit_length / 2);

    while (true) {
        const BigInt p = generate_prime(test);
        const BigInt q = generate_prime(test);
        if (p == q) continue;

        const BigInt n = p * q;
        if (!has_fermat_distance(p, q, n, min_fermat_steps)) continue;

        const BigInt phi = (p - 1) * (q - 1);
        const auto result = nt.extended_gcd(e, phi);
        if (result.gcd != 1) continue;

        // Коэффициент Безу при e даёт обратный элемент e по модулю phi.
        BigInt d = result.x % phi;
        if (d < 0) {
            d += phi;
        }
        if (small_exponent(n, d)) continue;

        return {n, e, d};
    }
}

void RSA::generate_keys() {
    if (!key_generator) {
        throw std::logic_error("Сначала задайте параметры генерации ключей RSA");
    }
    generate_keys(*key_generator);
}

void RSA::generate_keys(PrimalityTestType test_type, double probability, std::size_t bit_length) {
    const KeyGenerator generator(test_type, probability, bit_length);
    generate_keys(generator);
    key_generator = generator;
}

void RSA::generate_keys(const KeyGenerator& generator) {
    KeyPair new_keys;
    do {
        new_keys = generator.generate();
    } while (new_keys.n == keys.n);
    keys = std::move(new_keys);
}

const RSA::KeyPair& RSA::get_keys() const {
    if (keys.n == 0) {
        throw std::logic_error("Сначала сгенерируйте или загрузите ключи RSA");
    }
    return keys;
}

RSA::PublicKey RSA::get_public_key() const {
    const KeyPair& key_pair = get_keys();
    return {key_pair.n, key_pair.e};
}

bool RSA::has_small_private_exponent() const {
    const KeyPair& key_pair = get_keys();
    return small_exponent(key_pair.n, key_pair.d);
}

BigInt RSA::encrypt(const BigInt& message) const {
    const KeyPair& key_pair = get_keys();
    if (message < 0 || message >= key_pair.n) {
        throw std::invalid_argument("Сообщение должно быть в диапазоне [0, n)");
    }
    return number_theory.mod_pow(message, key_pair.e, key_pair.n);
}

BigInt RSA::decrypt(const BigInt& ciphertext) const {
    const KeyPair& key_pair = get_keys();
    if (ciphertext < 0 || ciphertext >= key_pair.n) {
        throw std::invalid_argument("Шифртекст должен быть в диапазоне [0, n)");
    }
    return number_theory.mod_pow(ciphertext, key_pair.d, key_pair.n);
}
