#include "primality/fermat_test.h"

#include <cmath>
#include <limits>


bool FermatTest::single_iteration(const BigInt& n, const BigInt& a) const {
    if (number_theory.gcd(a, n) != 1) return false;
    return number_theory.mod_pow(a, n - 1, n) == 1;
}

long double FermatTest::log_error_bound(const BigInt& n) const {
    // Среди n - 3 оснований есть хотя бы один делитель составного n.
    // Проверка НОД обнаружит его и для числа Кармайкла.
    const BigInt base_count = n - 3;
    // При таком размере необходимое число итераций заведомо превышает предел.
    // До преобразования в long double оставляем запас на округление.
    if (boost::multiprecision::msb(base_count) >= std::numeric_limits<long double>::max_exponent - 1) {
        return 0.0L;
    }
    return std::log1p(-1.0L / base_count.convert_to<long double>());
}
