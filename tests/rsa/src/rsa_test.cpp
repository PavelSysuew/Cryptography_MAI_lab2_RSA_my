#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include "rsa_test.hpp"
#include "rsa/rsa.hpp"


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
        run();
    } catch (const Exception& error) {
        std::cout << "\x1b[32m[OK]\x1b[0m " << call << " — ожидалось исключение "
                  << expected << ", получено " << expected << ": \"" << error.what() << "\"\n";
        return true;
    }

    std::cerr << "\x1b[31m[FAIL]\x1b[0m " << call << " — ожидалось исключение "
              << expected << ", получено обычное завершение\n";
    return false;
}

bool check_round_trip(const RSA& rsa, const BigInt& message) {
    const BigInt ciphertext = rsa.encrypt(message);
    const BigInt decrypted = rsa.decrypt(ciphertext);
    return check_result("decrypt(encrypt(" + message.str() + "))", decrypted, message);
}

class TestDirectory {
public:
    TestDirectory() {
        std::string name = (std::filesystem::temp_directory_path() / "rsa-test-XXXXXX").string();
        if (mkdtemp(name.data()) == nullptr) {
            throw std::runtime_error("Не удалось создать каталог для теста");
        }
        path = name;
    }

    ~TestDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    std::filesystem::path path;
};

void write_test_file(const std::filesystem::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary);
    output << bytes;
    output.close();
    if (!output) throw std::runtime_error("Не удалось записать тестовый файл");
}

std::string read_test_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Не удалось открыть тестовый файл");
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

} // namespace

// Генерация новой пары тем же объектом RSA
int generate_keys_test() {
    RSA rsa;
    const BigInt min_n = BigInt(1) << 30;
    const BigInt max_n = BigInt(1) << 32;

    rsa.generate_keys(RSA::PrimalityTestType::MillerRabin, 0.999999999999, 16);
    const RSA::PublicKey first_key = rsa.get_public_key();
    if (!(check_result("e первого ключа", first_key.e, BigInt(65537)))) return 1;
    if (!(check_result("n первого ключа в диапазоне [2^30, 2^32)",
                        first_key.n >= min_n && first_key.n < max_n, true))) return 1;
    if (!(check_round_trip(rsa, 42))) return 1;

    rsa.generate_keys();
    const RSA::PublicKey second_key = rsa.get_public_key();
    if (!(check_result("e нового ключа", second_key.e, BigInt(65537)))) return 1;
    if (!(check_result("n первого и нового ключей различаются", first_key.n != second_key.n, true))) return 1;
    if (!(check_round_trip(rsa, 42))) return 1;
    return 0;
}

// Граничные сообщения и выбор разных тестов простоты
int encrypt_decrypt_test() {
    RSA miller_rabin(RSA::PrimalityTestType::MillerRabin, 0.999999999999, 16);
    miller_rabin.generate_keys();
    const RSA::PublicKey key = miller_rabin.get_public_key();
    if (!(check_round_trip(miller_rabin, 0))) return 1;
    if (!(check_round_trip(miller_rabin, 1))) return 1;
    if (!(check_round_trip(miller_rabin, 97))) return 1;
    if (!(check_round_trip(miller_rabin, key.n - 1))) return 1;

    RSA solovay_strassen(RSA::PrimalityTestType::SolovayStrassen, 0.999999999999, 16);
    solovay_strassen.generate_keys();
    if (!(check_round_trip(solovay_strassen, 97))) return 1;

    RSA fermat(RSA::PrimalityTestType::Fermat, 0.99, 12);
    fermat.generate_keys();
    if (!(check_round_trip(fermat, 97))) return 1;
    return 0;
}

// Некорректные настройки и значения за пределами модуля
int rsa_invalid_arguments_test() {
    using TestType = RSA::PrimalityTestType;
    if (!(check_exception<std::invalid_argument>("RSA(MillerRabin, 1.0, 16)",
                                                 "std::invalid_argument",
                                                 [] { RSA rsa(TestType::MillerRabin, 1.0, 16); }))) return 1;
    if (!(check_exception<std::invalid_argument>("RSA(MillerRabin, 0.99, 9)",
                                                 "std::invalid_argument",
                                                 [] { RSA rsa(TestType::MillerRabin, 0.99, 9); }))) return 1;

    RSA rsa(TestType::MillerRabin, 0.999999999999, 16);
    if (!(check_exception<std::logic_error>("encrypt(1) до генерации ключей", "std::logic_error",
                                            [&] { rsa.encrypt(1); }))) return 1;

    rsa.generate_keys();
    const RSA::PublicKey key = rsa.get_public_key();
    if (!(check_exception<std::invalid_argument>("encrypt(-1)", "std::invalid_argument",
                                                 [&] { rsa.encrypt(-1); }))) return 1;
    if (!(check_exception<std::invalid_argument>("encrypt(n)", "std::invalid_argument",
                                                 [&] { rsa.encrypt(key.n); }))) return 1;
    if (!(check_exception<std::invalid_argument>("decrypt(-1)", "std::invalid_argument",
                                                 [&] { rsa.decrypt(-1); }))) return 1;
    if (!(check_exception<std::invalid_argument>("decrypt(n)", "std::invalid_argument",
                                                 [&] { rsa.decrypt(key.n); }))) return 1;
    return 0;
}

// Двоичный файл, краткий последний блок и повторное открытие ключей
int rsa_file_test() {
    const TestDirectory directory;
    const auto source = directory.path / "исходный файл.bin";
    const auto encrypted = directory.path / "encrypted.rsa";
    const auto restored = directory.path / "restored.bin";
    const auto saved_keys = directory.path / "keys.rsa";
    const auto damaged = directory.path / "damaged.rsa";
    const auto rejected = directory.path / "rejected.bin";
    const std::string bytes("\0\x01\x80\xff\0" "AB\0\0\0", 10);
    write_test_file(source, bytes);

    RSA rsa(RSA::PrimalityTestType::MillerRabin, 0.999999999999, 16);
    rsa.generate_keys();
    rsa.save_keys(saved_keys);
    rsa.encrypt_file(source, encrypted);

    RSA another_rsa;
    another_rsa.load_keys(saved_keys);
    another_rsa.decrypt_file(encrypted, restored);
    if (!(check_result("Размер восстановленного файла",
                        std::filesystem::file_size(restored), bytes.size()))) return 1;
    if (!(check_result("Содержимое совпадает побайтно", read_test_file(restored) == bytes, true))) return 1;

    if (!(check_exception<std::system_error>("encrypt_file в исходный файл", "std::system_error",
                                             [&] { rsa.encrypt_file(source, source); }))) return 1;
    if (!(check_result("Исходный файл сохранился", read_test_file(source) == bytes, true))) return 1;
    if (!(check_exception<std::system_error>("decrypt_file в существующий файл", "std::system_error",
                                             [&] { another_rsa.decrypt_file(encrypted, restored); }))) return 1;

    rsa.generate_keys();
    if (!(check_exception<std::invalid_argument>("decrypt_file с новым ключом", "std::invalid_argument",
                                                 [&] { rsa.decrypt_file(encrypted, rejected); }))) return 1;

    std::string damaged_bytes = read_test_file(encrypted);
    damaged_bytes.pop_back();
    write_test_file(damaged, damaged_bytes);
    if (!(check_exception<std::runtime_error>("decrypt_file без последнего байта", "std::runtime_error",
                                              [&] { another_rsa.decrypt_file(damaged, rejected); }))) return 1;
    if (!(check_result("После ошибки файл результата отсутствует",
                        std::filesystem::exists(rejected), false))) return 1;

    const auto empty = directory.path / "empty.bin";
    const auto empty_encrypted = directory.path / "empty.rsa";
    const auto empty_restored = directory.path / "empty_restored.bin";
    write_test_file(empty, "");
    another_rsa.encrypt_file(empty, empty_encrypted);
    another_rsa.decrypt_file(empty_encrypted, empty_restored);
    if (!(check_result("Размер восстановленного пустого файла",
                        std::filesystem::file_size(empty_restored), 0U))) return 1;
    return 0;
}
