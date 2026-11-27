#pragma once

#include "np/number_theory.hpp"


class IPrimalityTest {
public:
    virtual ~IPrimalityTest() = default;

    // Надёжность в диапазоне [0.5, 1): для составного n вероятность
    // ошибочного результата true не превышает 1 - probability.
    // При ошибке ввода выбрасывается std::invalid_argument,
    // при превышении допустимого числа итераций — std::length_error.
    virtual bool is_prime(const BigInt& n, double probability) const = 0;
};

class PrimalityTest : public IPrimalityTest {
public:
    bool is_prime(const BigInt& n, double probability) const override final;

protected:
    const NumberTheory number_theory{};

    // Базовый класс передаёт нечётное n >= 5 и основание из [2, n - 2].
    virtual bool single_iteration(const BigInt& n, const BigInt& a) const = 0;
    virtual long double log_error_bound(const BigInt& n) const = 0;
};
