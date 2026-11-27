#include "primality_test_cases.hpp"

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
        {"Общий интерфейс тестов простоты", primality_interface_test},
        {"Одна итерация теста Ферма", fermat_iteration_test},
        {"Одна итерация теста Соловея — Штрассена", solovay_strassen_iteration_test},
        {"Одна итерация теста Миллера — Рабина", miller_rabin_iteration_test},
        {"Количество итераций и досрочный выход", primality_round_count_test},
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
