#include "crypto.hpp"

using namespace iotbc;

crypto::crypto(const std::string &folderPath) {
    if (!std::filesystem::exists(folderPath)) {
        std::filesystem::create_directory(folderPath);
    }

    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".pem") {
            std::ifstream file(entry.path());
            std::string key((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            publicKeys.push_back(key);
        }
    }
}

void crypto::generateKeyPair(const std::string &publicFolderPath, const std::string &privateFolderPath) {
    std::string publicKeyPath = publicFolderPath + "/public_key.pem";
    std::string privateKeyPath = privateFolderPath + "/private_key.pem";

    EVP_PKEY *privateKey = nullptr;
    EVP_PKEY *publicKey = nullptr;

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize keygen");
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to set RSA keygen bits");
    }
    if (EVP_PKEY_keygen(ctx, &privateKey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to generate private key");
    }
    EVP_PKEY_CTX_free(ctx);

    BIO *bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(bio, privateKey)) {
        BIO_free(bio);
        throw std::runtime_error("Failed to write public key to BIO");
    }
    publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);

    FILE *privateKeyFile = fopen(privateKeyPath.c_str(), "w");
    FILE *publicKeyFile = fopen(publicKeyPath.c_str(), "w");

    PEM_write_PrivateKey(privateKeyFile, privateKey, NULL, NULL, 0, NULL, NULL);
    PEM_write_PUBKEY(publicKeyFile, publicKey);

    std::cout << "Key pair generated" << std::endl;

    fclose(privateKeyFile);
    fclose(publicKeyFile);

    EVP_PKEY_free(privateKey);
    EVP_PKEY_free(publicKey);
}

std::optional<std::string> crypto::authenticate(const std::string &privateKeyString) {
    EVP_PKEY *privateKey = getPrivateKey(privateKeyString);

    for (const auto &publicKeyString : publicKeys) {
        EVP_PKEY *publicKey = getPublicKey(publicKeyString);

        if (!EVP_PKEY_eq(privateKey, publicKey)) {
            EVP_PKEY_free(publicKey);
            continue;
        }
        EVP_PKEY_free(publicKey);
        EVP_PKEY_free(privateKey);
        return publicKeyString;
    }

    EVP_PKEY_free(privateKey);
    return std::nullopt;
}

std::string crypto::hash(const std::string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);

    std::stringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hexStream << std::hex << (int)hash[i];
    }

    return hexStream.str();
}

std::vector<char> crypto::encryptData(const std::vector<char> &data, const std::string &publicKeyString) {
    EVP_PKEY *publicKey = getPublicKey(publicKeyString);

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(publicKey, NULL);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }
    size_t outLen;
    if (EVP_PKEY_encrypt(ctx, NULL, &outLen, (unsigned char*)data.data(), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to determine buffer length for encryption");
    }
    std::vector<char> encryptedData(outLen);
    if (EVP_PKEY_encrypt(ctx, (unsigned char*)encryptedData.data(), &outLen, (unsigned char*)data.data(), data.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }
    encryptedData.resize(outLen);
    EVP_PKEY_CTX_free(ctx);
    return encryptedData;
}

std::vector<char> crypto::decryptData(const std::vector<char> &encryptedData, const std::string &privateKeyString) {
    EVP_PKEY *privateKey = getPrivateKey(privateKeyString);

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privateKey, NULL);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }
    size_t outLen;
    if (EVP_PKEY_decrypt(ctx, NULL, &outLen, (unsigned char*)encryptedData.data(), encryptedData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to determine buffer length for decryption");
    }
    std::vector<char> decryptedData(outLen);
    if (EVP_PKEY_decrypt(ctx, (unsigned char*)decryptedData.data(), &outLen, (unsigned char*)encryptedData.data(), encryptedData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }
    decryptedData.resize(outLen);
    EVP_PKEY_CTX_free(ctx);
    return decryptedData;
}

EVP_PKEY *crypto::getPrivateKey(const std::string &privateKeyString) {
    BIO *bio = BIO_new(BIO_s_mem());
    if (!BIO_write(bio, privateKeyString.c_str(), privateKeyString.size())) {
        BIO_free(bio);
        throw std::runtime_error("Failed to write private key string to BIO");
    }
    EVP_PKEY *privateKey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!privateKey) {
        throw std::runtime_error("Failed to read private key from BIO");
    }
    return privateKey;
}

EVP_PKEY *crypto::getPublicKey(const std::string &publicKeyString) {
    BIO *bio = BIO_new(BIO_s_mem());
    if (!BIO_write(bio, publicKeyString.c_str(), publicKeyString.size())) {
        BIO_free(bio);
        throw std::runtime_error("Failed to write public key string to BIO");
    }
    EVP_PKEY *publicKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!publicKey) {
        throw std::runtime_error("Failed to read public key from BIO");
    }
    return publicKey;
}
