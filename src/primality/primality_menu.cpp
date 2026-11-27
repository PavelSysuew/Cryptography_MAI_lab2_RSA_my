#include "function.hpp"
#include "primality/fermat_test.h"
#include "primality/miller_rabin_test.h"
#include "primality/solovay_strassen_test.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>


namespace {

// Читаем всю строку, чтобы после ошибочного ввода можно было вернуться в меню.
bool read_value(const char* prompt, auto& value) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;

    std::istringstream input(line);
    if (!(input >> value) || !(input >> std::ws).eof()) {
        throw std::invalid_argument("Некорректное значение");
    }
    return true;
}

void run_primality_test(const IPrimalityTest& test) {
    BigInt n;
    double probability = 0.0;
    if (!read_value("n = ", n)) return;
    if (!read_value("Надёжность [0.5, 1) = ", probability)) return;

    if (test.is_prime(n, probability)) {
        std::cout << "Число прошло тест простоты." << std::endl;
    } else {
        std::cout << "Число не является простым." << std::endl;
    }
}

} // namespace

void primality_tests() {
    const FermatTest fermat;
    const SolovayStrassenTest solovay_strassen;
    const MillerRabinTest miller_rabin;

    while (true) {
        std::cout << "\nМЕНЮ. Тесты простоты" << std::endl;
        std::cout << "1. Тест Ферма" << std::endl;
        std::cout << "2. Тест Соловея — Штрассена" << std::endl;
        std::cout << "3. Тест Миллера — Рабина" << std::endl;
        std::cout << "4. Вернуться в главное меню" << std::endl;

        try {
            int n_menu = 0;
            if (!read_value("Введите номер пункта, который хотите выполнить >> ", n_menu)) return;
            switch (n_menu) {
                case 1: run_primality_test(fermat); break;
                case 2: run_primality_test(solovay_strassen); break;
                case 3: run_primality_test(miller_rabin); break;
                case 4:
                    std::cout << '\n';
                    return;
                default:
                    std::cout << "Неверный номер пункта меню. Пожалуйста, выберите корректный пункт." << std::endl;
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << std::endl;
        }
        if (!std::cin) return;
    }
}
