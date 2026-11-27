#include "number_theory_test.hpp"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>

int main() {
    struct TestGroup {
        const char* name;
        int (*run)();
    };

    const TestGroup groups[] = {
        {"Функция Кармайкла", carmichael_lambda_test},
        {"Классический алгоритм Евклида", gcd_test},
        {"Расширенный алгоритм Евклида", extended_gcd_test},
        {"Бинарный алгоритм Евклида", binary_gcd_test},
        {"Возведение в степень по модулю", mod_pow_test},
        {"Функция Эйлера по определению", euler_phi_by_definition_test},
        {"Функция Эйлера через разложение", euler_phi_by_factorization_test},
        {"Функция Эйлера через ДПФ", euler_phi_by_fourier_test},
        {"Символ Лежандра", legendre_test},
        {"Символ Якоби", jacobi_test},
    };

    std::size_t passed = 0;
    for (const auto& group : groups) {
        std::cout << group.name << std::endl;
        try {
            if (group.run() == 0) {
                ++passed;
            } 
        } catch (const std::exception& error) {
            std::cerr << "\x1b[31m[FAIL]\x1b[0m " << group.name << " — " << error.what() << '\n';
        }
        std::cout << std::endl;
    }

    std::cout << "Прошло групп: " << passed << " из " << std::size(groups) << '\n';
    return passed == std::size(groups) ? EXIT_SUCCESS : EXIT_FAILURE;  
}

