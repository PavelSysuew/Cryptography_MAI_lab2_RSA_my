#include <iostream>
#include <limits>

int menu_main() {
    std::cout << "ГЛАВНОЕ МЕНЮ" << std::endl;
    std::cout << "1. Математические функции" << std::endl;
    std::cout << "2. Тесты простоты" << std::endl;
    std::cout << "3. Генерация ключей, шифрование и дешифрование RSA" << std::endl;
    std::cout << "4. Атака Винера на уязвимый RSA-ключ" << std::endl;
    std::cout << "5. Выход" << std::endl;
    std::cout << "Введите номер пункта, который хотите выполнить >> ";
    int n = 0;
    if (!(std::cin >> n)) {
        if (std::cin.eof()) return 5;
        std::cin.clear();
        n = 0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << std::endl;
    return n;
}
