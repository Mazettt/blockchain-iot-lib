#pragma once

#include <array>
#include <cstring>
#include <vector>

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

        std::string toString() const;

        static Address fromString(const std::string &str);

        static Address fromPublicKey(const PublicKey &public_key);
    };

    struct Signer {
        PrivateKey private_key;
        PublicKey public_key;
        Address address;
    
        Signer(const PrivateKey &private_key);
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
        Hash txHash() const;

        /// @brief Signs the transaction with the given private key
        /// @param private_key The private key to sign the transaction with
        /// @throws Secp256k1Error if an error occurs during the signing
        void sign(const PrivateKey &private_key);

        /// @brief Signs the transaction with the given signer
        /// @param signer The signer to sign the transaction with
        /// @throws Secp256k1Error if an error occurs during the signing
        inline void sign(const Signer &signer)
        {
            this->sign(signer.private_key);
        }

        /// @brief Verifies if the given signature is valid for the transaction
        /// @throws InvalidSignature if the signature is invalid
        /// @throws Secp256k1Error if an error occurs during the verification
        void verify() const;

        /// @brief Serializes the transaction into a byte array
        /// @return The serialized transaction
        std::vector<unsigned char> serialize() const;

        /// @brief Get the size of the transaction in bytes
        /// @return The size of the transaction in bytes
        inline size_t size() const
        {
            return from.size() + sizeof(Nonce) + sizeof(size_t) + data.size() + signature.size();
        }

        /// @brief Deserializes a transaction from a byte array
        /// @param data The byte array to deserialize
        /// @return The deserialized transaction
        static Transaction deserialize(const std::vector<unsigned char> &data);
    };
}
