#pragma once

#include <array>
#include <cstring>

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <secp256k1.h>

#include <Exceptions.hpp>

namespace iotbc {
    using Nonce = uint64_t;
    using PublicKey = std::array<unsigned char, 64>;
    using PrivateKey = std::array<unsigned char, 32>;
    
    using Hash = std::array<unsigned char, SHA256_DIGEST_LENGTH>;

    struct Address {
        std::array<unsigned char, 20> bits;

        Address()
        {
            bits.fill(0);
        }

        Address(const std::array<unsigned char, 20> &bits)
            : bits(bits)
        {
        }

        bool operator==(const Address &other) const
        {
            return bits == other.bits;
        }

        bool operator!=(const Address &other) const
        {
            return bits != other.bits;
        }

        bool operator<(const Address &other) const
        {
            return bits < other.bits;
        }

        bool operator>(const Address &other) const
        {
            return bits > other.bits;
        }

        bool operator<=(const Address &other) const
        {
            return bits <= other.bits;
        }

        bool operator>=(const Address &other) const
        {
            return bits >= other.bits;
        }

        std::string toString() const
        {
            std::string str = "0x";
            str.reserve(42);

            for (unsigned char byte : bits) {
                str.push_back("0123456789abcdef"[byte >> 4]);
                str.push_back("0123456789abcdef"[byte & 0x0F]);
            }

            return str;
        }

        static Address fromString(const std::string &str)
        {
            if (str.size() != 42) {
                throw InvalidAddress("Invalid address length");
            }
            if (str.substr(0, 2) != "0x") {
                throw InvalidAddress("Invalid address prefix");
            }

            std::array<unsigned char, 20> bits;

            for (size_t i = 0; i < 20; i++) {
                if (!isxdigit(str[2 + i * 2]) || !isxdigit(str[3 + i * 2])) {
                    throw InvalidAddress("Invalid character in address");
                }

                bits[i] = std::stoi(str.substr(2 + i * 2, 2), nullptr, 16);
            }

            return Address(bits);
        }

        static Address fromPublicKey(const PublicKey &public_key)
        {
            Address address;

            EVP_MD_CTX *ctx = EVP_MD_CTX_new();

            if (ctx == nullptr) {
                throw EvpError("Failed to create EVP_MD_CTX");
            }

            if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to initialize digest");
            }

            if (EVP_DigestUpdate(ctx, public_key.data(), public_key.size()) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to update digest");
            }

            unsigned int len = 0;
            if (EVP_DigestFinal_ex(ctx, address.bits.data(), &len) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to finalize digest");
            }

            EVP_MD_CTX_free(ctx);

            return address;
        }
    };

    struct Signer {
        PrivateKey private_key;
        PublicKey public_key;
        Address address;
    
        Signer(const PrivateKey &private_key)
            : private_key(private_key)
        {
            secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

            if (ctx == nullptr) {
                throw Secp256k1Error("Failed to create secp256k1 context");
            }

            secp256k1_pubkey s_pubkey;

            if (secp256k1_ec_pubkey_create(ctx, &s_pubkey, private_key.data()) != 1) {
                secp256k1_context_destroy(ctx);
                throw Secp256k1Error("Failed to create public key");
            }

            memcpy(public_key.data(), s_pubkey.data, 64);

            secp256k1_context_destroy(ctx);

            address = Address::fromPublicKey(public_key);
        }
    };
    
    struct Transaction {
        PublicKey from;
        Nonce nonce;
        std::vector<unsigned char> data;
        std::array<unsigned char, 64> signature;

        /// @brief Creates a new transaction based on the sender, the nonce, and the data
        /// @param from The sender public key
        /// @param nonce The nonce of the transaction
        /// @param data The data of the transaction
        Transaction(const Signer &from, Nonce nonce, const std::vector<unsigned char> &data)
            : from(from.public_key), nonce(nonce), data(data), signature()
        {
        }

        /// @brief Creates a new transaction based on the sender address, the nonce, and the data
        /// @param from The sender public key
        /// @param nonce The nonce of the transaction
        /// @param data The data of the transaction
        Transaction(const PublicKey &from, Nonce nonce, const std::vector<unsigned char> &data)
            : from(from), nonce(nonce), data(data), signature()
        {
        }
        
        /// @brief Creates a hash of the transaction
        /// @return The hash of the transaction
        Hash txHash() const
        {
            Hash h;

            EVP_MD_CTX *ctx = EVP_MD_CTX_new();

            if (ctx == nullptr) {
                throw EvpError("Failed to create EVP_MD_CTX");
            }

            if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to initialize digest");
            }

            if (EVP_DigestUpdate(ctx, &nonce, sizeof(Nonce)) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to update digest");
            }

            if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to update digest");
            }

            if (EVP_DigestFinal_ex(ctx, h.data(), nullptr) != 1) {
                EVP_MD_CTX_free(ctx);
                throw EvpError("Failed to finalize digest");
            }

            EVP_MD_CTX_free(ctx);

            return h;
        }

        /// @brief Signs the transaction with the given private key
        /// @param private_key The private key to sign the transaction with
        /// @throws Secp256k1Error if an error occurs during the signing
        void sign(const PrivateKey &private_key)
        {
            secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

            if (ctx == nullptr) {
                throw Secp256k1Error("Failed to create secp256k1 context");
            }

            secp256k1_ecdsa_signature sig;

            Hash tx_hash = this->txHash();

            if (secp256k1_ecdsa_sign(ctx, &sig, tx_hash.data(), private_key.data(), nullptr, nullptr) != 1) {
                secp256k1_context_destroy(ctx);
                throw Secp256k1Error("Failed to sign transaction");
            }

            secp256k1_ecdsa_signature_serialize_compact(ctx, signature.data(), &sig);

            secp256k1_context_destroy(ctx);
        }

        /// @brief Signs the transaction with the given signer
        /// @param signer The signer to sign the transaction with
        /// @throws Secp256k1Error if an error occurs during the signing
        void sign(const Signer &signer)
        {
            this->sign(signer.private_key);
        }

        /// @brief Verifies if the given signature is valid for the transaction
        /// @throws InvalidSignature if the signature is invalid
        /// @throws Secp256k1Error if an error occurs during the verification
        void verify() const
        {
            secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);

            if (ctx == nullptr) {
                throw Secp256k1Error("Failed to create secp256k1 context");
            }

            secp256k1_ecdsa_signature sig;

            if (secp256k1_ecdsa_signature_parse_compact(ctx, &sig, signature.data()) != 1) {
                secp256k1_context_destroy(ctx);
                throw Secp256k1Error("Failed to parse signature");
            }

            secp256k1_pubkey pubkey;
            memcpy(pubkey.data, from.data(), 64);

            if (secp256k1_ecdsa_verify(ctx, &sig, this->txHash().data(), &pubkey) != 1) {
                secp256k1_context_destroy(ctx);
                throw InvalidSignature("Invalid signature");
            }

            secp256k1_context_destroy(ctx);
        }

        /// @brief Serializes the transaction into a byte array
        /// @return The serialized transaction
        std::vector<unsigned char> serialize() const
        {
            std::vector<unsigned char> buf;

            buf.insert(buf.end(), from.begin(), from.end());

            for (size_t i = 0; i < sizeof(Nonce); i++) {
                buf.push_back((nonce >> (i * sizeof(Nonce))) & 0xFF);
            }

            for (size_t i = 0; i < sizeof(size_t); i++) {
                buf.push_back((data.size() >> (i * sizeof(size_t)) & 0xFF));
            }
            buf.insert(buf.end(), data.begin(), data.end());

            buf.insert(buf.end(), signature.begin(), signature.end());

            return buf;
        }

        /// @brief Get the size of the transaction in bytes
        /// @return The size of the transaction in bytes
        size_t size() const
        {
            return from.size() + sizeof(Nonce) + sizeof(size_t) + data.size() + signature.size();
        }

        /// @brief Deserializes a transaction from a byte array
        /// @param data The byte array to deserialize
        /// @return The deserialized transaction
        static Transaction deserialize(const std::vector<unsigned char> &data)
        {
            std::size_t cur = 0;

            PublicKey from;
            for (size_t i = 0; i < from.size(); i++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                from[i] = data[cur++];
            }

            Nonce nonce = 0;
            for (size_t i = 0; i < sizeof(Nonce); i++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                nonce |= data[cur++] << (i * sizeof(Nonce));
            }

            size_t data_size = 0;
            for (size_t i = 0; i < sizeof(size_t); i++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                data_size |= data[cur++] << (i * sizeof(size_t));
            }

            std::vector<unsigned char> tx_data;
            tx_data.reserve(data_size);
            for (size_t i = 0; i < data_size; i++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                tx_data.push_back(data[cur++]);
            }

            std::array<unsigned char, 64> signature;
            for (size_t i = 0; i < signature.size(); i++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                signature[i] = data[cur++];
            }

            Transaction tx(from, nonce, tx_data);
            tx.signature = signature;

            return tx;
        }
    };
}
