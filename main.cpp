#include <exception>
#include <iostream>

#include "function.hpp"
#include "np/number_theory.hpp"

int main() {
    while (true) {
        const int n_menu = menu_main();
        try {
            switch (n_menu) {
                case 1: number_theory(); break;
                case 2: primality_tests(); break;
                case 3: rsa_demo(); break;
                case 4: rsa_key_check_demo(); break;
                case 5: return 0;
                default:
                    std::cout << "Неверный номер пункта меню. Пожалуйста, выберите корректный пункт." << std::endl;
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << std::endl;
        }
        std::cout << '\n';
    }
}
