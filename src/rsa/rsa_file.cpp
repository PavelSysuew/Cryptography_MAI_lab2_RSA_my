#include "rsa/rsa.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <unistd.h>


namespace {

constexpr std::size_t file_size_bytes = 8;
constexpr const char* key_version = "RSAKEY1";

// Создаём новый файл с правами владельца. Существующий файл не перезаписываем.
class NewFile {
public:
    explicit NewFile(const std::filesystem::path& path) : path(path) {
        const int descriptor = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (descriptor == -1) {
            throw std::system_error(errno, std::generic_category(), "Не удалось создать " + path.string());
        }

        file = fdopen(descriptor, "wb");
        if (file == nullptr) {
            const int error = errno;
            close(descriptor);
            remove_file();
            throw std::system_error(error, std::generic_category(), "Не удалось открыть " + path.string());
        }
    }

    NewFile(const NewFile&) = delete;
    NewFile& operator=(const NewFile&) = delete;

    ~NewFile() {
        if (file != nullptr) std::fclose(file);
        if (!finished) remove_file();
    }

    void write(std::string_view bytes) {
        if (std::fwrite(bytes.data(), 1, bytes.size(), file) != bytes.size()) {
            throw std::runtime_error("Ошибка записи файла");
        }
    }

    void finish() {
        const int result = std::fclose(file);
        file = nullptr;
        if (result != 0) throw std::runtime_error("Ошибка записи файла");
        finished = true;
    }

private:
    std::filesystem::path path;
    std::FILE* file = nullptr;
    bool finished = false;

    void remove_file() {
        std::error_code error;
        std::filesystem::remove(path, error);
    }
};

std::ifstream open_input(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Не удалось открыть " + path.string());
    return input;
}

std::string read_block(std::istream& input, std::size_t size) {
    std::string bytes(size, '\0');
    if (!input.read(bytes.data(), static_cast<std::streamsize>(size))) {
        throw std::runtime_error("Не удалось прочитать полный блок файла");
    }
    return bytes;
}

void check_end(std::istream& input) {
    if (input.peek() != std::char_traits<char>::eof() || input.bad()) {
        throw std::runtime_error("Ошибка чтения или лишние данные в файле");
    }
}

// Первый байт блока содержит старшие биты числа.
BigInt block_to_number(std::string_view bytes) {
    BigInt number = 0;
    for (const unsigned char byte : bytes) {
        number <<= 8;
        number += byte;
    }
    return number;
}

std::string number_to_block(BigInt number, std::size_t size) {
    std::string bytes(size, '\0');
    for (std::size_t i = size; i > 0; --i) {
        const BigInt byte = number & 255;
        bytes[i - 1] = static_cast<char>(byte.convert_to<unsigned char>());
        number >>= 8;
    }
    if (number != 0) throw std::runtime_error("Число не поместилось в блок файла");
    return bytes;
}

std::string file_header(const RSA::PublicKey& key) {
    return "RSAFILE1\n" + key.n.str() + "\n" + key.e.str() + "\n";
}

} // namespace

void RSA::encrypt_file(const std::filesystem::path& source,
                        const std::filesystem::path& destination) const {
    const PublicKey key = get_public_key();
    auto input = open_input(source);
    const auto file_size = std::filesystem::file_size(source);
    if (file_size > std::numeric_limits<std::uint64_t>::max()) {
        throw std::length_error("Файл слишком большой");
    }

    // Число из открытого блока должно быть меньше модуля n.
    const std::size_t top_bit = boost::multiprecision::msb(key.n);
    const std::size_t plain_size = top_bit / 8;
    const std::size_t cipher_size = plain_size + 1;
    NewFile output(destination);
    output.write(file_header(key));
    output.write(number_to_block(BigInt(file_size), file_size_bytes));

    std::uint64_t remaining = file_size;
    while (remaining > 0) {
        const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(remaining, plain_size));
        const BigInt message = block_to_number(read_block(input, count));
        output.write(number_to_block(encrypt(message), cipher_size));
        remaining -= count;
    }
    check_end(input);
    output.finish();
}

void RSA::decrypt_file(const std::filesystem::path& source,
                        const std::filesystem::path& destination) const {
    const PublicKey key = get_public_key();
    auto input = open_input(source);
    const std::string header = file_header(key);
    if (read_block(input, header.size()) != header) {
        throw std::invalid_argument("Неверный формат файла или ключ RSA");
    }

    // Размер исходного файла задаёт длину последнего блока, включая его нулевые байты.
    const std::size_t top_bit = boost::multiprecision::msb(key.n);
    const std::size_t plain_size = top_bit / 8;
    const std::size_t cipher_size = plain_size + 1;
    std::uint64_t remaining = block_to_number(read_block(input, file_size_bytes)).convert_to<std::uint64_t>();
    NewFile output(destination);

    while (remaining > 0) {
        const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(remaining, plain_size));
        const BigInt ciphertext = block_to_number(read_block(input, cipher_size));
        output.write(number_to_block(decrypt(ciphertext), count));
        remaining -= count;
    }
    check_end(input);
    output.finish();
}

void RSA::save_keys(const std::filesystem::path& path) const {
    const KeyPair& key_pair = get_keys();
    NewFile output(path);
    output.write(std::string(key_version) + "\n" + key_pair.n.str() + "\n"
                 + key_pair.e.str() + "\n" + key_pair.d.str() + "\n");
    output.finish();
}

void RSA::load_keys(const std::filesystem::path& path) {
    auto input = open_input(path);
    std::string version;
    KeyPair loaded;
    if (!std::getline(input, version) || version != key_version
        || !(input >> loaded.n >> loaded.e >> loaded.d)
        || !(input >> std::ws).eof()
        || loaded.n <= 255 || (loaded.n & 1) == 0
        || loaded.e <= 1 || loaded.e >= loaded.n || loaded.d <= 1 || loaded.d >= loaded.n) {
        throw std::invalid_argument("Некорректный файл ключей RSA");
    }
    keys = std::move(loaded);
}
