#include "primality/solovay_strassen_test.h"

#include <cmath>


bool SolovayStrassenTest::single_iteration(const BigInt& n, const BigInt& a) const {
    const int jacobi = number_theory.jacobi(a, n);
    if (jacobi == 0) return false;

    const BigInt residue = number_theory.mod_pow(a, (n - 1) / 2, n);
    if (jacobi == -1) return residue == n - 1;
    return residue == 1;
}

long double SolovayStrassenTest::log_error_bound(const BigInt&) const {
    return std::log(0.5L);
}
