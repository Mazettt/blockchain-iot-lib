#include "blockchain.hpp"

using namespace iotbc;
namespace fs = std::filesystem;

blockchain::blockchain(const std::string &folderPath):
    folderPath(folderPath)
{
    if (!fs::exists(folderPath)) {
        fs::create_directory(folderPath);
    }

    loadOrGenerateKey();
    populateBlocks();
}

void blockchain::addBlock(const std::vector<char> &data) {
    std::string uniqueId = calculateUniqueId(data);
    std::ofstream outFile(folderPath + "/" + uniqueId, std::ios::binary);
    if (outFile.is_open()) {
        std::vector<char> encryptedData = encryptData(data);
        outFile.write(encryptedData.data(), encryptedData.size());
        outFile.close();
        blocks.push_back(uniqueId);
        std::cout << "Block added with ID: '" << uniqueId << "'" << std::endl;
    } else {
        throw std::runtime_error("Unable to open file to write block");
    }
}

std::vector<char> blockchain::getBlock(const std::string &uniqueId) {
    if (std::find(blocks.begin(), blocks.end(), uniqueId) == blocks.end()) {
        throw std::runtime_error("Block of ID '" + uniqueId + "' not found");
    }

    std::ifstream inFile(folderPath + "/" + uniqueId, std::ios::binary);
    if (inFile.is_open()) {
        std::vector<char> encryptedData{std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>()};
        inFile.close();
        return decryptData(encryptedData);
    } else {
        throw std::runtime_error("Unable to open file to read block");
    }
}

void blockchain::populateBlocks() {
    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            blocks.push_back(entry.path().filename().string());
            // std::cout << "Block found with ID: " << entry.path().filename().string() << std::endl;
        }
    }
    std::cout << "Blockchain loaded with " << blocks.size() << " blocks" << std::endl;
}

void blockchain::loadOrGenerateKey() {
    std::string keyFilePath = folderPath + "/key.bin";
    std::ifstream keyFile(keyFilePath, std::ios::binary);
    if (keyFile.is_open()) {
        keyFile.read((char*)key, sizeof(key));
        keyFile.close();
    } else {
        if (!RAND_bytes(key, sizeof(key))) {
            throw std::runtime_error("Failed to generate key");
        }
        std::ofstream outFile(keyFilePath, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write((char*)key, sizeof(key));
            outFile.close();
        } else {
            throw std::runtime_error("Unable to open file to write key");
        }
    }
}

std::string blockchain::calculateUniqueId(const std::vector<char> &data) {
    std::stringstream ss;
    for (const auto &block : blocks) {
        ss << block;
    }
    ss.write(data.data(), data.size());

    std::string input = ss.str();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input.c_str(), input.size(), hash);

    std::stringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hexStream << std::hex << (int)hash[i];
    }

    return hexStream.str();
}

std::vector<char> blockchain::encryptData(const std::vector<char> &data) {
    std::vector<char> encryptedData;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    unsigned char iv[EVP_MAX_IV_LENGTH];
    if (!RAND_bytes(iv, sizeof(iv))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to generate IV");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    encryptedData.resize(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = (int)encryptedData.size();
    if (EVP_EncryptUpdate(ctx, (unsigned char*)encryptedData.data(), &outLen1, (const unsigned char*)data.data(), (int)data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }

    int outLen2 = (int)encryptedData.size() - outLen1;
    if (EVP_EncryptFinal_ex(ctx, (unsigned char*)encryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption");
    }

    encryptedData.resize(outLen1 + outLen2);
    EVP_CIPHER_CTX_free(ctx);

    encryptedData.insert(encryptedData.begin(), iv, iv + sizeof(iv));

    return encryptedData;
}

std::vector<char> blockchain::decryptData(const std::vector<char> &encryptedData) {
    if (encryptedData.size() < EVP_MAX_IV_LENGTH) {
        throw std::runtime_error("Invalid encrypted data");
    }

    std::vector<char> decryptedData;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    unsigned char iv[EVP_MAX_IV_LENGTH];
    std::copy(encryptedData.begin(), encryptedData.begin() + EVP_MAX_IV_LENGTH, iv);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    decryptedData.resize(encryptedData.size() - EVP_MAX_IV_LENGTH);
    int outLen1 = (int)decryptedData.size();
    if (EVP_DecryptUpdate(ctx, (unsigned char*)decryptedData.data(), &outLen1, (const unsigned char*)encryptedData.data() + EVP_MAX_IV_LENGTH, (int)encryptedData.size() - EVP_MAX_IV_LENGTH) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }

    int outLen2 = (int)decryptedData.size() - outLen1;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)decryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize decryption");
    }

    decryptedData.resize(outLen1 + outLen2);
    EVP_CIPHER_CTX_free(ctx);

    return decryptedData;
}
