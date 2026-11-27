#include "rsa_test.hpp"

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
        {"Генерация ключей RSA", generate_keys_test},
        {"Шифрование и расшифрование RSA", encrypt_decrypt_test},
        {"Ошибки при использовании RSA", rsa_invalid_arguments_test},
        {"Шифрование и расшифрование файлов", rsa_file_test},
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
