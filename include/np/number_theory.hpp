#pragma once

#include "big_int.hpp"


class NumberTheory {
public:
    // Результат расширенного алгоритма Евклида: gcd = a*x + b*y
    struct ExtendedGcdResult {
        BigInt gcd;
        BigInt x;
        BigInt y;
    };

    // Символ Лежандра (a/p), p — нечётное простое. Возвращает -1, 0 или 1.
    int legendre(const BigInt& a, const BigInt& p) const;

    // Символ Якоби (a/n), n — нечётное положительное. Возвращает -1, 0 или 1.
    int jacobi(const BigInt& a, const BigInt& n) const;

    // НОД: классический алгоритм Евклида
    BigInt gcd(const BigInt& a, const BigInt& b) const;

    // НОД и коэффициенты Безу: расширенный алгоритм Евклида
    ExtendedGcdResult extended_gcd(const BigInt& a, const BigInt& b) const;

    // НОД: бинарный алгоритм Евклида
    BigInt binary_gcd(const BigInt& a, const BigInt& b) const;

    // Быстрое возведение в степень по модулю
    BigInt mod_pow(const BigInt& base, const BigInt& exp, const BigInt& mod) const;

    // Функция Эйлера тремя способами
    BigInt euler_phi_by_definition(const BigInt& n) const;     // через сумму единиц
    BigInt euler_phi_by_factorization(const BigInt& n) const;  // через каноническое разложение
    BigInt euler_phi_by_fourier(const BigInt& n) const;        // через ДПФ

    // Функция Кармайкла через каноническое разложение
    BigInt carmichael_lambda(const BigInt& n) const;

};

// Консольное меню задания 
int menu_number_theory();
void number_theory();
