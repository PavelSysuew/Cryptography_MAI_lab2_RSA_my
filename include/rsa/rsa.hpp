#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>

#include "np/number_theory.hpp"


class IPrimalityTest;

class RSA {
public:
    enum class PrimalityTestType {
        Fermat,
        SolovayStrassen,
        MillerRabin
    };

    struct PublicKey {
        BigInt n;
        BigInt e;
    };

    RSA() = default;
    RSA(PrimalityTestType test_type, double probability, std::size_t bit_length);

    void generate_keys();
    void generate_keys(PrimalityTestType test_type, double probability, std::size_t bit_length);
    PublicKey get_public_key() const;
    bool has_small_private_exponent() const;
    BigInt encrypt(const BigInt& message) const;
    BigInt decrypt(const BigInt& ciphertext) const;
    void encrypt_file(const std::filesystem::path& source, const std::filesystem::path& destination) const;
    void decrypt_file(const std::filesystem::path& source, const std::filesystem::path& destination) const;
    void save_keys(const std::filesystem::path& path) const;
    void load_keys(const std::filesystem::path& path);

private:
    struct KeyPair {
        BigInt n;
        BigInt e;
        BigInt d;
    };

    class KeyGenerator {
    public:
        KeyGenerator(PrimalityTestType test_type, double probability, std::size_t bit_length);
        KeyPair generate() const;

    private:
        PrimalityTestType test_type;
        double probability;
        std::size_t bit_length;

        BigInt generate_prime(const IPrimalityTest& test) const;
    };

    // Параметры генерации появляются при её настройке.
    std::optional<KeyGenerator> key_generator;
    KeyPair keys{};
    const NumberTheory number_theory{};

    void generate_keys(const KeyGenerator& generator);
    const KeyPair& get_keys() const;
};
