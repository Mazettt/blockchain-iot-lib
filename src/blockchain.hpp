#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace iotbc {
    class blockchain {
    public:
        blockchain(const std::string &folderPath);
        ~blockchain() = default;

        void addBlock(const std::vector<char> &data);
        std::vector<char> getBlock(const std::string &uniqueId);

    private:
        const std::string folderPath;
        std::vector<std::string> blocks;
        unsigned char key[EVP_MAX_KEY_LENGTH];

        void importBlocksFromFolder();
        void loadOrGenerateKey();

        std::string calculateUniqueId(const std::vector<char> &data);
        std::vector<char> encryptData(const std::vector<char> &data);
        std::vector<char> decryptData(const std::vector<char> &encryptedData);
    };
}

#endif
