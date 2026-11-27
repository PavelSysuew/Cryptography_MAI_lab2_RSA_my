#include "primality/primality_test.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>


namespace {

constexpr std::size_t max_iterations = 1'000'000;

BigInt random_base(const BigInt& n) {
    // Генератор инициализируется один раз при первом обращении в каждом потоке.
    thread_local std::mt19937_64 generator = [] {
        std::random_device random_device;
        std::array<std::seed_seq::result_type, 8> seeds;
        for (auto& seed : seeds) {
            seed = random_device();
        }
        std::seed_seq sequence(seeds.begin(), seeds.end());
        return std::mt19937_64(sequence);
    }();

    // Сначала выбираем смещение из [0, n - 4], затем прибавляем 2.
    const BigInt max_offset = n - 4;
    const std::size_t bit_count =
        static_cast<std::size_t>(boost::multiprecision::msb(max_offset)) + 1;
    constexpr std::size_t word_size = std::mt19937_64::word_size;

    BigInt offset;
    do {
        offset = 0;
        for (std::size_t remaining = bit_count; remaining > 0;) {
            const std::size_t bits = std::min(remaining, word_size);
            offset <<= bits;
            offset |= generator() >> (word_size - bits);
            remaining -= bits;
        }
        // Отбрасываем лишние значения, чтобы сохранить равномерность.
    } while (offset > max_offset);
    return offset + 2;
}

} // namespace

bool PrimalityTest::is_prime(const BigInt& n, double probability) const {
    if (!(probability >= 0.5 && probability < 1.0)) {
        throw std::invalid_argument("Надёжность должна быть в диапазоне [0.5, 1)");
    }
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if ((n & 1) == 0) return false;

    // Ошибка после k независимых итераций не превышает q^k.
    const long double log_failure = std::log1p(-static_cast<long double>(probability));
    const long double log_error = log_error_bound(n);
    if (!(log_error < 0.0L) || !std::isfinite(log_error)) {
        throw std::length_error("Невозможно рассчитать число итераций для заданной надёжности");
    }

    const long double required = std::ceil(log_failure / log_error);
    if (!std::isfinite(required) || required > max_iterations) {
        throw std::length_error("Для заданной надёжности нужно больше 1 000 000 итераций");
    }

    const auto iterations = static_cast<std::size_t>(required);
    for (std::size_t i = 0; i < iterations; ++i) {
        if (!single_iteration(n, random_base(n))) return false;
    }
    return true;
}
