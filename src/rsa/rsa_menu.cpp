#include "function.hpp"
#include "rsa/rsa.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>


namespace {

const std::filesystem::path keys_directory = "keys";
const std::filesystem::path default_key_path = keys_directory / "rsa_keys.rsa";

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

bool read_path(const char* prompt, std::string& path, const std::string& default_path = "") {
    std::cout << prompt;
    if (!default_path.empty()) std::cout << " (Enter для " << default_path << ")";
    std::cout << " = ";
    if (!std::getline(std::cin, path)) return false;

    // При вставке из терминала путь может быть обёрнут в кавычки.
    const auto first = path.find_first_not_of(" \t");
    const auto last = path.find_last_not_of(" \t");
    if (first != std::string::npos && first < last && path[first] == path[last]
        && (path[first] == '\'' || path[first] == '"')) {
        path = path.substr(first + 1, last - first - 1);
    }

    if (path.empty()) path = default_path;
    if (path.empty()) throw std::invalid_argument("Укажите путь к файлу");
    return true;
}

bool read_key_path(const char* prompt, std::filesystem::path& path) {
    std::string name;
    if (!read_path(prompt, name, default_key_path.string())) return false;

    path = name;
    if (!path.has_parent_path()) path = keys_directory / path;
    return true;
}

void generate_rsa_keys(RSA& rsa) {
    using TestType = RSA::PrimalityTestType;

    std::cout << "\nМЕНЮ. Выбор теста простоты для RSA" << std::endl;
    std::cout << "1. Тест Ферма" << std::endl;
    std::cout << "2. Тест Соловея — Штрассена" << std::endl;
    std::cout << "3. Тест Миллера — Рабина" << std::endl;
    std::cout << "4. Вернуться в меню RSA" << std::endl;

    int test_menu = 0;
    if (!read_value("Введите номер пункта >> ", test_menu)) return;

    TestType test_type;
    switch (test_menu) {
        case 1: test_type = TestType::Fermat; break;
        case 2: test_type = TestType::SolovayStrassen; break;
        case 3: test_type = TestType::MillerRabin; break;
        case 4:
            std::cout << '\n';
            return;
        default: throw std::invalid_argument("Неверный номер пункта меню");
    }

    double probability = 0.0;
    long long bit_length = 0;
    if (!read_value("Надёжность [0.5, 1) = ", probability)) return;
    if (!read_value("Битовая длина p и q = ", bit_length)) return;
    if (!std::in_range<std::size_t>(bit_length)) {
        throw std::invalid_argument("Некорректная битовая длина");
    }

    rsa.generate_keys(test_type, probability, static_cast<std::size_t>(bit_length));
    const RSA::PublicKey key = rsa.get_public_key();
    std::cout << "Открытый ключ: n = " << key.n << ", e = " << key.e << std::endl;
}

void print_private_exponent_check(const RSA& rsa) {
    if (rsa.has_small_private_exponent()) {
        std::cout << "Закрытая экспонента не превышает нижнюю границу." << std::endl;
    } else {
        std::cout << "Закрытая экспонента превышает нижнюю границу." << std::endl;
    }
}

} // namespace

void rsa_demo() {
    RSA rsa;

    while (true) {
        std::cout << "\nМЕНЮ. RSA" << std::endl;
        std::cout << "1. Сгенерировать новую пару ключей" << std::endl;
        std::cout << "2. Зашифровать и расшифровать число" << std::endl;
        std::cout << "3. Зашифровать файл" << std::endl;
        std::cout << "4. Расшифровать файл" << std::endl;
        std::cout << "5. Сохранить ключи в файл" << std::endl;
        std::cout << "6. Загрузить ключи из файла" << std::endl;
        std::cout << "7. Вернуться в главное меню" << std::endl;

        try {
            int n_menu = 0;
            if (!read_value("Введите номер пункта >> ", n_menu)) return;

            switch (n_menu) {
                case 1:
                    std::cout << '\n';
                    generate_rsa_keys(rsa);
                    break;
                case 2: {
                    const RSA::PublicKey key = rsa.get_public_key();
                    std::cout << "Введите целое число от 0 до " << key.n - 1 << std::endl;

                    BigInt message;
                    if (!read_value("m = ", message)) return;
                    const BigInt ciphertext = rsa.encrypt(message);
                    const BigInt decrypted = rsa.decrypt(ciphertext);

                    std::cout << "Шифртекст: " << ciphertext << std::endl;
                    std::cout << "После расшифрования: " << decrypted << std::endl;
                    break;
                }
                case 3:
                case 4: {
                    std::string source;
                    std::string destination;
                    if (!read_path("Путь к исходному файлу", source)) return;

                    if (n_menu == 3) {
                        const std::filesystem::path source_path(source);
                        const std::string filename =
                            source_path.stem().string() + "_encrypted" + source_path.extension().string();
                        const auto default_path = source_path.parent_path() / filename;
                        if (!read_path("Путь к новому файлу", destination, default_path.string())) return;

                        rsa.encrypt_file(source, destination);
                        std::cout << "Файл зашифрован." << std::endl;
                    } else {
                        if (!read_path("Путь к новому файлу", destination)) return;
                        rsa.decrypt_file(source, destination);
                        std::cout << "Файл расшифрован." << std::endl;
                    }
                    break;
                }
                case 5: {
                    std::filesystem::path path;
                    if (!read_key_path("Имя или путь нового файла ключей", path)) return;
                    if (path.parent_path() == keys_directory) {
                        std::filesystem::create_directories(keys_directory);
                    }
                    rsa.save_keys(path);
                    std::cout << "Ключи сохранены в " << path.string() << std::endl;
                    break;
                }
                case 6: {
                    std::filesystem::path path;
                    if (!read_key_path("Имя или путь файла ключей", path)) return;
                    rsa.load_keys(path);
                    std::cout << "Ключи загружены из " << path.string() << std::endl;
                    break;
                }
                case 7:
                    std::cout << '\n';
                    return;
                default:
                    std::cout << "Неверный номер пункта меню." << std::endl;
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << std::endl;
        }
        if (std::cin.eof()) return;
    }
}

void rsa_key_check_demo() {
    while (true) {
        std::cout << "\nМЕНЮ. Проверка ключей RSA" << std::endl;
        std::cout << "1. Сгенерировать и проверить учебную пару RSA" << std::endl;
        std::cout << "2. Проверить сохранённую пару RSA" << std::endl;
        std::cout << "3. Вернуться в главное меню" << std::endl;

        try {
            int n_menu = 0;
            if (!read_value("Введите номер пункта >> ", n_menu)) return;

            switch (n_menu) {
                case 1: {
                    RSA rsa(RSA::PrimalityTestType::MillerRabin, 0.99, 16);
                    rsa.generate_keys();
                    print_private_exponent_check(rsa);
                    break;
                }
                case 2: {
                    std::filesystem::path path;
                    if (!read_key_path("Имя или путь файла ключей", path)) return;
                    RSA rsa;
                    rsa.load_keys(path);
                    print_private_exponent_check(rsa);
                    break;
                }
                case 3: return;
                default:
                    std::cout << "Неверный номер пункта меню." << std::endl;
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Ошибка: " << error.what() << std::endl;
        }
        if (std::cin.eof()) return;
    }
}
