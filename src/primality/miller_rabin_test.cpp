#include "primality/miller_rabin_test.h"

#include <cmath>
#include <cstddef>


bool MillerRabinTest::single_iteration(const BigInt& n, const BigInt& a) const {
    const BigInt n_minus_one = n - 1;
    BigInt d = n_minus_one;
    std::size_t s = 0;
    // Представляем n - 1 в виде 2^s * d, где d нечётное.
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    BigInt x = number_theory.mod_pow(a, d, n);
    if (x == 1 || x == n_minus_one) return true;

    for (std::size_t r = 1; r < s; ++r) {
        // Квадрат вычисляется в BigInt, поэтому промежуточное произведение не переполняется.
        x = (x * x) % n;
        if (x == n_minus_one) return true;
        if (x == 1) return false;
    }
    return false;
}

long double MillerRabinTest::log_error_bound(const BigInt&) const {
    return std::log(0.25L);
}
