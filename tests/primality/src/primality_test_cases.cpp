#include <charconv>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include "primality_test_cases.hpp"
#include "primality/fermat_test.h"
#include "primality/miller_rabin_test.h"
#include "primality/solovay_strassen_test.h"


namespace {

bool check_result(const std::string& call, const auto& actual, const auto& expected) {
    if (actual == expected) {
        std::cout << "\x1b[32m[OK]\x1b[0m " << call << " — ожидалось "
                  << std::boolalpha << expected << ", получено " << actual << '\n';
        return true;
    }

    std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call << " — ожидалось "
              << std::boolalpha << expected << ", получено " << actual << '\n';
    return false;
}

template <class Exception>
bool check_exception(const std::string& call, const char* expected, const auto& run) {
    try {
        const bool actual = run();
        std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call << " — ожидалось исключение "
                  << expected << ", получено " << std::boolalpha << actual << '\n';
        return false;
    } catch (const Exception& error) {
        std::cout << "\x1b[32m[OK]\x1b[0m " << call << " — ожидалось исключение "
                  << expected << ", получено " << expected << ": \"" << error.what() << "\"\n";
        return true;
    } catch (const std::exception& error) {
        std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call << " — ожидалось исключение "
                  << expected << ", получено другое исключение: \"" << error.what() << "\"\n";
        return false;
    }
}

template <class Test>
class IterationTest : public Test {
public:
    using Test::single_iteration;
};

template <class Test>
class RoundCountTest : public Test {
public:
    mutable std::size_t calls = 0;
    mutable bool bases_in_range = true;
    std::size_t reject_at = 0;

protected:
    bool single_iteration(const BigInt& n, const BigInt& a) const override {
        bases_in_range = bases_in_range && a >= 2 && a <= n - 2;
        ++calls;
        return calls != reject_at;
    }
};

std::string primality_call(const std::string& name, const BigInt& n, double probability) {
    // Выводим надёжность так, чтобы различались 1.0 и ближайшее к нему значение снизу.
    char buffer[64];
    const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), probability);
    const std::string value = error == std::errc{} ? std::string(buffer, end)
                                                   : std::to_string(probability);
    return name + "::is_prime(" + n.str() + ", " + value + ")";
}

bool check_primality_case(const IPrimalityTest& test, const std::string& name, const BigInt& n,
                          double probability, bool expected) {
    return check_result(primality_call(name, n, probability), test.is_prime(n, probability), expected);
}

bool check_invalid_probability_case(const IPrimalityTest& test, const std::string& name,
                                    const BigInt& n, double probability) {
    return check_exception<std::invalid_argument>(primality_call(name, n, probability),
                                                 "std::invalid_argument",
                                                 [&] { return test.is_prime(n, probability); });
}

bool check_iteration_case(const auto& test, const BigInt& n, const BigInt& a, bool expected) {
    return check_result("single_iteration(" + n.str() + ", " + a.str() + ")",
                        test.single_iteration(n, a), expected);
}

bool check_primality_cases(const IPrimalityTest& test, const std::string& name) {
    // Отрицательные и граничные значения
    if (!(check_primality_case(test, name, -7, 0.5, false))) return false;
    if (!(check_primality_case(test, name, 0, 0.5, false))) return false;
    if (!(check_primality_case(test, name, 1, 0.5, false))) return false;
    if (!(check_primality_case(test, name, 2, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 3, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 4, 0.5, false))) return false;
    if (!(check_primality_case(test, name, 100, 0.5, false))) return false;

    // Простые числа
    if (!(check_primality_case(test, name, 5, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 7, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 11, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 17, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 97, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 997, 0.5, true))) return false;
    if (!(check_primality_case(test, name, 65537, 0.5, true))) return false;

    // Допустимые уровни надёжности
    if (!(check_primality_case(test, name, 17, 0.75, true))) return false;
    if (!(check_primality_case(test, name, 17, 0.95, true))) return false;
    if (!(check_primality_case(test, name, 17, 0.99, true))) return false;
    if (!(check_primality_case(test, name, 17, std::nextafter(1.0, 0.0), true))) return false;

    // Некорректная надёжность отклоняется даже для n = 2
    if (!(check_invalid_probability_case(test, name, 2, -1.0))) return false;
    if (!(check_invalid_probability_case(test, name, 2, 0.49))) return false;
    if (!(check_invalid_probability_case(test, name, 2, 1.0))) return false;
    if (!(check_invalid_probability_case(test, name, 2, 2.0))) return false;
    if (!(check_invalid_probability_case(test, name, 2, std::numeric_limits<double>::infinity()))) return false;
    if (!(check_invalid_probability_case(test, name, 2, std::numeric_limits<double>::quiet_NaN()))) return false;
    return true;
}

bool check_divisor_cases(const auto& test) {
    // Составные числа и их делители
    if (!(check_iteration_case(test, 9, 3, false))) return false;
    if (!(check_iteration_case(test, 15, 3, false))) return false;
    if (!(check_iteration_case(test, 21, 3, false))) return false;
    if (!(check_iteration_case(test, 25, 5, false))) return false;
    if (!(check_iteration_case(test, 49, 7, false))) return false;
    if (!(check_iteration_case(test, 91, 7, false))) return false;

    // Числа Кармайкла и их делители
    if (!(check_iteration_case(test, 561, 3, false))) return false;
    if (!(check_iteration_case(test, 1105, 5, false))) return false;
    if (!(check_iteration_case(test, 1729, 7, false))) return false;
    if (!(check_iteration_case(test, 2465, 5, false))) return false;
    if (!(check_iteration_case(test, 2821, 7, false))) return false;
    if (!(check_iteration_case(test, 6601, 7, false))) return false;
    return true;
}

bool check_round_count_case(auto& test, const std::string& name, const BigInt& n,
                            double probability, bool expected_result, std::size_t expected_calls) {
    test.calls = 0;
    test.bases_in_range = true;
    const std::string call = primality_call(name, n, probability);
    if (!(check_result(call, test.is_prime(n, probability), expected_result))) return false;
    if (!(check_result(call + " — число итераций", test.calls, expected_calls))) return false;
    return check_result(call + " — основания в диапазоне [2, n - 2]", test.bases_in_range, true);
}

} // namespace

// Одинаковые проверки для трёх тестов через IPrimalityTest
int primality_interface_test() {
    const FermatTest fermat;
    const SolovayStrassenTest solovay_strassen;
    const MillerRabinTest miller_rabin;
    if (!(check_primality_cases(fermat, "FermatTest"))) return 1;
    if (!(check_primality_cases(solovay_strassen, "SolovayStrassenTest"))) return 1;
    if (!(check_primality_cases(miller_rabin, "MillerRabinTest"))) return 1;

    // Простое число, для которого основание может занимать больше 64 бит
    const BigInt large_prime = (BigInt(1) << 127) - 1;
    if (!(check_primality_case(solovay_strassen, "SolovayStrassenTest", large_prime, 0.99, true))) return 1;
    if (!(check_primality_case(miller_rabin, "MillerRabinTest", large_prime, 0.99, true))) return 1;

    // Превышение предела по числу итераций теста Ферма
    const double almost_one = std::nextafter(1.0, 0.0);
    if (!(check_exception<std::length_error>(primality_call("FermatTest", 65537, almost_one),
                                            "std::length_error",
                                            [&] { return fermat.is_prime(65537, almost_one); }))) return 1;
    if (!(check_exception<std::length_error>("FermatTest::is_prime(2^20000 + 1, 0.5)",
                                            "std::length_error",
                                            [&] { return fermat.is_prime((BigInt(1) << 20000) + 1, 0.5); }))) return 1;
    return 0;
}

// Тест Ферма с фиксированными основаниями
int fermat_iteration_test() {
    const IterationTest<FermatTest> test;
    if (!(check_divisor_cases(test))) return 1;
    if (!(check_iteration_case(test, 9, 2, false))) return 1;

    // Основание 2 взаимно просто с этими числами Кармайкла
    if (!(check_iteration_case(test, 561, 2, true))) return 1;
    if (!(check_iteration_case(test, 1105, 2, true))) return 1;
    if (!(check_iteration_case(test, 1729, 2, true))) return 1;
    if (!(check_iteration_case(test, 2465, 2, true))) return 1;
    if (!(check_iteration_case(test, 2821, 2, true))) return 1;
    if (!(check_iteration_case(test, 6601, 2, true))) return 1;
    return 0;
}

// Тест Соловея — Штрассена с фиксированными основаниями
int solovay_strassen_iteration_test() {
    const IterationTest<SolovayStrassenTest> test;
    if (!(check_divisor_cases(test))) return 1;

    // Символ Якоби соответственно равен -1 и 1
    if (!(check_iteration_case(test, 5, 2, true))) return 1;
    if (!(check_iteration_case(test, 7, 2, true))) return 1;
    if (!(check_iteration_case(test, 9, 2, false))) return 1;
    if (!(check_iteration_case(test, 561, 2, true))) return 1;
    return 0;
}

// Тест Миллера — Рабина с фиксированными основаниями
int miller_rabin_iteration_test() {
    const IterationTest<MillerRabinTest> test;
    if (!(check_divisor_cases(test))) return 1;

    // Начальный остаток равен 1 или n - 1
    if (!(check_iteration_case(test, 7, 2, true))) return 1;
    if (!(check_iteration_case(test, 7, 3, true))) return 1;

    // Последовательные квадраты и число, прошедшее одну итерацию
    if (!(check_iteration_case(test, 17, 2, true))) return 1;
    if (!(check_iteration_case(test, 561, 2, false))) return 1;
    if (!(check_iteration_case(test, 9, 2, false))) return 1;
    if (!(check_iteration_case(test, 2047, 2, true))) return 1;

    // Промежуточный квадрат занимает больше 128 бит
    if (!(check_iteration_case(test, (BigInt(1) << 128) + 1, BigInt(1) << 127, true))) return 1;
    return 0;
}

// Количество итераций при разных уровнях надёжности
int primality_round_count_test() {
    RoundCountTest<FermatTest> fermat;
    RoundCountTest<SolovayStrassenTest> solovay_strassen;
    RoundCountTest<MillerRabinTest> miller_rabin;

    if (!(check_round_count_case(fermat, "FermatTest", 5, 0.5, true, 1))) return 1;
    if (!(check_round_count_case(fermat, "FermatTest", 7, 0.5, true, 3))) return 1;
    if (!(check_round_count_case(solovay_strassen, "SolovayStrassenTest", 5, 0.75, true, 2))) return 1;
    if (!(check_round_count_case(solovay_strassen, "SolovayStrassenTest", 97, 0.99, true, 7))) return 1;
    if (!(check_round_count_case(miller_rabin, "MillerRabinTest", 5, 0.75, true, 1))) return 1;
    if (!(check_round_count_case(miller_rabin, "MillerRabinTest", 5,
                                 std::nextafter(0.75, 1.0), true, 2))) return 1;
    const BigInt large_prime = (BigInt(1) << 127) - 1;
    if (!(check_round_count_case(miller_rabin, "MillerRabinTest", large_prime, 0.99, true, 4))) return 1;

    // После обнаружения составного числа второй итерацией цикл завершается
    miller_rabin.reject_at = 2;
    if (!(check_round_count_case(miller_rabin, "MillerRabinTest", 9, 0.99, false, 2))) return 1;
    return 0;
}
