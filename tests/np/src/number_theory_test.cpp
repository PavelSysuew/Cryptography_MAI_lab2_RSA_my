#include <iostream>
#include <stdexcept>
#include <string>
#include "number_theory_test.hpp"
#include "np/number_theory.hpp"
#include "np/big_int.hpp"


namespace {

bool check_result(const std::string& call, const BigInt& actual, const BigInt& expected) {
    if (actual == expected) {
        std::cout << "\x1b[32m[OK]\x1b[0m " << call << " — ожидалось "
                  << expected << ", получено " << actual << '\n';
        return true;
    }

    std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call << " — ожидалось "
              << expected << ", получено " << actual << '\n';
    return false;
}

bool check_invalid_argument(const std::string& call, const auto& run) {
    try {
        const auto actual = run();
        std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call
                  << " — ожидалось исключение std::invalid_argument, получено " << actual << '\n';
        return false;
    } catch (const std::invalid_argument& error) {
        std::cout << "\x1b[32m[OK]\x1b[0m " << call
                  << " — получено std::invalid_argument: \"" << error.what() << "\"\n";
        return true;
    }
}

bool check_carmichael_case(const NumberTheory& nt, const BigInt& n, const BigInt& expected) {
    return check_result("carmichael_lambda(" + n.str() + ")", nt.carmichael_lambda(n), expected);
}

bool check_invalid_carmichael_case(const NumberTheory& nt, const BigInt& n) {
    return check_invalid_argument("carmichael_lambda(" + n.str() + ")",
                                  [&] { return nt.carmichael_lambda(n); });
}

bool check_gcd_case(const NumberTheory& nt, const BigInt& a, const BigInt& b,
                    const BigInt& expected) {
    return check_result("gcd(" + a.str() + ", " + b.str() + ")", nt.gcd(a, b), expected);
}

bool check_extended_gcd_case(const NumberTheory& nt, const BigInt& a, const BigInt& b,
                             const BigInt& expected) {
    const auto result = nt.extended_gcd(a, b);
    const BigInt bezout = a * result.x + b * result.y;
    const std::string call = "extended_gcd(" + a.str() + ", " + b.str() + ")";
    if (!(check_result(call + " — НОД", result.gcd, expected))) return false;
    return check_result(call + " — a*x + b*y", bezout, expected);
}

bool check_invalid_extended_gcd_case(const NumberTheory& nt, const BigInt& a, const BigInt& b) {
    return check_invalid_argument("extended_gcd(" + a.str() + ", " + b.str() + ")",
                                  [&] { return nt.extended_gcd(a, b).gcd; });
}

bool check_binary_gcd_case(const NumberTheory& nt, const BigInt& a, const BigInt& b,
                           const BigInt& expected) {
    return check_result("binary_gcd(" + a.str() + ", " + b.str() + ")",
                        nt.binary_gcd(a, b), expected);
}

bool check_invalid_binary_gcd_case(const NumberTheory& nt, const BigInt& a, const BigInt& b) {
    return check_invalid_argument("binary_gcd(" + a.str() + ", " + b.str() + ")",
                                  [&] { return nt.binary_gcd(a, b); });
}

bool check_mod_pow_case(const NumberTheory& nt, const BigInt& base, const BigInt& exp,
                        const BigInt& mod, const BigInt& expected) {
    const std::string call = "mod_pow(" + base.str() + ", " + exp.str() + ", " + mod.str() + ")";
    return check_result(call, nt.mod_pow(base, exp, mod), expected);
}

bool check_invalid_mod_pow_case(const NumberTheory& nt, const BigInt& base,
                                const BigInt& exp, const BigInt& mod) {
    const std::string call = "mod_pow(" + base.str() + ", " + exp.str() + ", " + mod.str() + ")";
    return check_invalid_argument(call, [&] { return nt.mod_pow(base, exp, mod); });
}

bool check_euler_definition_case(const NumberTheory& nt, const BigInt& n, const BigInt& expected) {
    return check_result("euler_phi_by_definition(" + n.str() + ")",
                        nt.euler_phi_by_definition(n), expected);
}

bool check_invalid_euler_definition_case(const NumberTheory& nt, const BigInt& n) {
    return check_invalid_argument("euler_phi_by_definition(" + n.str() + ")",
                                  [&] { return nt.euler_phi_by_definition(n); });
}

bool check_euler_factorization_case(const NumberTheory& nt, const BigInt& n, const BigInt& expected) {
    return check_result("euler_phi_by_factorization(" + n.str() + ")",
                        nt.euler_phi_by_factorization(n), expected);
}

bool check_invalid_euler_factorization_case(const NumberTheory& nt, const BigInt& n) {
    return check_invalid_argument("euler_phi_by_factorization(" + n.str() + ")",
                                  [&] { return nt.euler_phi_by_factorization(n); });
}

bool check_euler_fourier_case(const NumberTheory& nt, const BigInt& n, const BigInt& expected) {
    return check_result("euler_phi_by_fourier(" + n.str() + ")", nt.euler_phi_by_fourier(n), expected);
}

bool check_invalid_euler_fourier_case(const NumberTheory& nt, const BigInt& n) {
    return check_invalid_argument("euler_phi_by_fourier(" + n.str() + ")",
                                  [&] { return nt.euler_phi_by_fourier(n); });
}

bool check_legendre_case(const NumberTheory& nt, const BigInt& a, const BigInt& p, int expected) {
    return check_result("legendre(" + a.str() + ", " + p.str() + ")",
                        nt.legendre(a, p), expected);
}

bool check_invalid_legendre_case(const NumberTheory& nt, const BigInt& a, const BigInt& p) {
    return check_invalid_argument("legendre(" + a.str() + ", " + p.str() + ")",
                                  [&] { return nt.legendre(a, p); });
}

bool check_jacobi_case(const NumberTheory& nt, const BigInt& a, const BigInt& n, int expected) {
    return check_result("jacobi(" + a.str() + ", " + n.str() + ")",
                        nt.jacobi(a, n), expected);
}

bool check_invalid_jacobi_case(const NumberTheory& nt, const BigInt& a, const BigInt& n) {
    return check_invalid_argument("jacobi(" + a.str() + ", " + n.str() + ")",
                                  [&] { return nt.jacobi(a, n); });
}

} // namespace

// Функция Кармайкла
int carmichael_lambda_test() {
    NumberTheory nt;
    // Начальные значения
    if (!(check_carmichael_case(nt, 1, 1))) return 1;
    if (!(check_carmichael_case(nt, 2, 1))) return 1;
    if (!(check_carmichael_case(nt, 4, 2))) return 1;

    // Степени двойки с показателем не меньше трёх
    if (!(check_carmichael_case(nt, 8, 2))) return 1;
    if (!(check_carmichael_case(nt, 16, 4))) return 1;
    if (!(check_carmichael_case(nt, 32, 8))) return 1;
    if (!(check_carmichael_case(nt, 65536, 16384))) return 1;

    // Простые числа
    if (!(check_carmichael_case(nt, 3, 2))) return 1;
    if (!(check_carmichael_case(nt, 7, 6))) return 1;
    if (!(check_carmichael_case(nt, 97, 96))) return 1;

    // Степени нечётных простых чисел
    if (!(check_carmichael_case(nt, 9, 6))) return 1;
    if (!(check_carmichael_case(nt, 25, 20))) return 1;
    if (!(check_carmichael_case(nt, 27, 18))) return 1;
    if (!(check_carmichael_case(nt, 121, 110))) return 1;

    // Три различных простых множителя
    if (!(check_carmichael_case(nt, 561, 80))) return 1;
    if (!(check_carmichael_case(nt, 1105, 48))) return 1;
    if (!(check_carmichael_case(nt, 1729, 36))) return 1;

    if (!(check_invalid_carmichael_case(nt, 0))) return 1;
    if (!(check_invalid_carmichael_case(nt, -5))) return 1;
    return 0;
}

// Классический алгоритм Евклида
int gcd_test() {
    NumberTheory nt;
    const BigInt scale = BigInt(1) << 100;
    if (!(check_gcd_case(nt, 48, 18, 6))) return 1;
    if (!(check_gcd_case(nt, -18, 48, 6))) return 1;
    if (!(check_gcd_case(nt, 35, 64, 1))) return 1;
    if (!(check_gcd_case(nt, 0, -25, 25))) return 1;
    if (!(check_gcd_case(nt, 25, 0, 25))) return 1;
    if (!(check_gcd_case(nt, 48 * scale, 18 * scale, 6 * scale))) return 1;
    return 0;
}

// Расширенный алгоритм Евклида и равенство Безу
int extended_gcd_test() {
    NumberTheory nt;
    if (!(check_extended_gcd_case(nt, 240, 46, 2))) return 1;
    if (!(check_extended_gcd_case(nt, -240, 46, 2))) return 1;
    if (!(check_extended_gcd_case(nt, 240, -46, 2))) return 1;
    if (!(check_extended_gcd_case(nt, 0, -25, 25))) return 1;
    if (!(check_extended_gcd_case(nt, -25, 0, 25))) return 1;
    if (!(check_invalid_extended_gcd_case(nt, 0, 0))) return 1;
    return 0;
}

// Бинарный алгоритм Евклида
int binary_gcd_test() {
    NumberTheory nt;
    const BigInt scale = BigInt(1) << 100;
    if (!(check_binary_gcd_case(nt, 48, 18, 6))) return 1;
    if (!(check_binary_gcd_case(nt, -18, 48, 6))) return 1;
    if (!(check_binary_gcd_case(nt, 35, 64, 1))) return 1;
    if (!(check_binary_gcd_case(nt, 0, -25, 25))) return 1;
    if (!(check_binary_gcd_case(nt, 25, 0, 25))) return 1;
    if (!(check_binary_gcd_case(nt, 48 * scale, 18 * scale, 6 * scale))) return 1;
    if (!(check_invalid_binary_gcd_case(nt, 0, 0))) return 1;
    return 0;
}

// Возведение в степень по модулю
int mod_pow_test() {
    NumberTheory nt;
    const BigInt large_exp = BigInt(1) << 100;
    if (!(check_mod_pow_case(nt, 4, 13, 497, 445))) return 1;
    if (!(check_mod_pow_case(nt, -2, 3, 5, 2))) return 1;
    if (!(check_mod_pow_case(nt, 14, 5, 7, 0))) return 1;
    if (!(check_mod_pow_case(nt, 7, 0, 11, 1))) return 1;
    if (!(check_mod_pow_case(nt, 7, 0, 1, 0))) return 1;
    // Степени двойки по модулю 7 повторяются через 3, а 2^100 даёт остаток 1 по модулю 3.
    if (!(check_mod_pow_case(nt, 2, large_exp, 7, 2))) return 1;
    if (!(check_invalid_mod_pow_case(nt, 2, 3, 0))) return 1;
    if (!(check_invalid_mod_pow_case(nt, 2, 3, -5))) return 1;
    if (!(check_invalid_mod_pow_case(nt, 2, -1, 5))) return 1;
    return 0;
}

// Функция Эйлера по определению
int euler_phi_by_definition_test() {
    NumberTheory nt;
    if (!(check_euler_definition_case(nt, 1, 1))) return 1;
    if (!(check_euler_definition_case(nt, 8, 4))) return 1;
    if (!(check_euler_definition_case(nt, 13, 12))) return 1;
    if (!(check_euler_definition_case(nt, 27, 18))) return 1;
    if (!(check_euler_definition_case(nt, 36, 12))) return 1;
    if (!(check_euler_definition_case(nt, 45, 24))) return 1;
    if (!(check_invalid_euler_definition_case(nt, 0))) return 1;
    if (!(check_invalid_euler_definition_case(nt, -5))) return 1;
    return 0;
}

// Функция Эйлера через разложение на простые множители
int euler_phi_by_factorization_test() {
    NumberTheory nt;
    if (!(check_euler_factorization_case(nt, 1, 1))) return 1;
    if (!(check_euler_factorization_case(nt, 8, 4))) return 1;
    if (!(check_euler_factorization_case(nt, 13, 12))) return 1;
    if (!(check_euler_factorization_case(nt, 27, 18))) return 1;
    if (!(check_euler_factorization_case(nt, 36, 12))) return 1;
    if (!(check_euler_factorization_case(nt, 45, 24))) return 1;
    if (!(check_invalid_euler_factorization_case(nt, 0))) return 1;
    if (!(check_invalid_euler_factorization_case(nt, -5))) return 1;
    return 0;
}

// Функция Эйлера через ДПФ
int euler_phi_by_fourier_test() {
    NumberTheory nt;
    if (!(check_euler_fourier_case(nt, 1, 1))) return 1;
    if (!(check_euler_fourier_case(nt, 8, 4))) return 1;
    if (!(check_euler_fourier_case(nt, 13, 12))) return 1;
    if (!(check_euler_fourier_case(nt, 27, 18))) return 1;
    if (!(check_euler_fourier_case(nt, 36, 12))) return 1;
    if (!(check_euler_fourier_case(nt, 45, 24))) return 1;
    if (!(check_invalid_euler_fourier_case(nt, 0))) return 1;
    if (!(check_invalid_euler_fourier_case(nt, -5))) return 1;
    return 0;
}

// Символ Лежандра
int legendre_test() {
    NumberTheory nt;
    const BigInt large_prime = (BigInt(1) << 61) - 1;
    if (!(check_legendre_case(nt, 0, 7, 0))) return 1;
    if (!(check_legendre_case(nt, 14, 7, 0))) return 1;
    if (!(check_legendre_case(nt, 2, 7, 1))) return 1;
    if (!(check_legendre_case(nt, 3, 7, -1))) return 1;
    if (!(check_legendre_case(nt, -3, 7, 1))) return 1;
    if (!(check_legendre_case(nt, -1, 7, -1))) return 1;
    if (!(check_legendre_case(nt, 9, 7, 1))) return 1;
    if (!(check_legendre_case(nt, -1, large_prime, -1))) return 1;
    if (!(check_invalid_legendre_case(nt, 3, 2))) return 1;
    if (!(check_invalid_legendre_case(nt, 3, 8))) return 1;
    if (!(check_invalid_legendre_case(nt, 3, 0))) return 1;
    if (!(check_invalid_legendre_case(nt, 3, 1))) return 1;
    if (!(check_invalid_legendre_case(nt, 3, -7))) return 1;
    return 0;
}

// Символ Якоби
int jacobi_test() {
    NumberTheory nt;
    if (!(check_jacobi_case(nt, 0, 1, 1))) return 1;
    if (!(check_jacobi_case(nt, 0, 9, 0))) return 1;
    if (!(check_jacobi_case(nt, 3, 7, -1))) return 1;
    if (!(check_jacobi_case(nt, 7, 15, -1))) return 1;
    if (!(check_jacobi_case(nt, 22, 15, -1))) return 1;
    if (!(check_jacobi_case(nt, 2, 3, -1))) return 1;
    if (!(check_jacobi_case(nt, 2, 5, -1))) return 1;
    if (!(check_jacobi_case(nt, 2, 9, 1))) return 1;
    if (!(check_jacobi_case(nt, 4, 5, 1))) return 1;
    if (!(check_jacobi_case(nt, 6, 15, 0))) return 1;
    if (!(check_jacobi_case(nt, -1, 15, -1))) return 1;
    if (!(check_invalid_jacobi_case(nt, 3, 0))) return 1;
    if (!(check_invalid_jacobi_case(nt, 3, -7))) return 1;
    if (!(check_invalid_jacobi_case(nt, 3, 2))) return 1;
    if (!(check_invalid_jacobi_case(nt, 3, 8))) return 1;
    return 0;
}
