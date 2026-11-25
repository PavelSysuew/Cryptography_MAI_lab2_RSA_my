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
        {"Начальные значения", carmichael_lambda_test1},
        {"Степени двойки", carmichael_lambda_test2},
        {"Простые числа", carmichael_lambda_test3},
        {"Степени нечётных простых", carmichael_lambda_test4},
        {"Три различных простых множителя", carmichael_lambda_test5},
    };

    std::size_t passed = 0;
    for (const auto& group : groups) {
        try {
            if (group.run() == 0) {
                ++passed;
                std::cout << "[OK] " << group.name << '\n';
            } else {
                std::cerr << "[FAIL] " << group.name << '\n';
            }
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << group.name << " — " << error.what() << '\n';
        }
    }

    std::cout << "Прошло групп: " << passed << " из " << std::size(groups) << '\n';
    return passed == std::size(groups) ? EXIT_SUCCESS : EXIT_FAILURE;
}

