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
                case 2: break;
                case 3: break;
                case 4: break;
                case 5: return 0;
                default:
                    std::cout << "Неверный номер пункта меню. Пожалуйста, выберите корректный пункт." << std::endl;
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << std::endl;
        }
    }
}
