#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <optional>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

namespace iotbc {
    class crypto {
    public:
        crypto(const std::string &folderPath);
        ~crypto() = default;

        static void generateKeyPair(const std::string &publicFolderPath, const std::string &privateFolderPath);
        std::optional<std::string> authenticate(const std::string &privateKeyString);

        static std::string hash(const std::string &data);
        static std::vector<char> encryptData(const std::vector<char> &data, const std::string &publicKeyString);
        static std::vector<char> decryptData(const std::vector<char> &encryptedData, const std::string &privateKeyString);

    private:
        std::vector<std::string> publicKeys;

        static EVP_PKEY *getPrivateKey(const std::string &privateKeyString);
        static EVP_PKEY *getPublicKey(const std::string &publicKeyString);
    };
}

#endif
