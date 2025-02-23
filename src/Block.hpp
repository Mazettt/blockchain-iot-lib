#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <secp256k1.h>

#include <Types.hpp>
#include <Consts.hpp>

namespace iotbc {
    class Block {
    public:
        Hash prevHash;
        std::vector<Transaction> transactions;
        Hash merkleRoot;
        Nonce nonce;

        /// @brief Create a new block based on the previous block's hash
        /// @param prevHash The hash of the previous block
        Block(Hash prevHash);

        /// Copy constructor
        Block(const Block &other) = default;

        /// Move constructor
        Block(Block &&other) = default;

        /// Copy assignment operator
        Block &operator=(const Block &other) = default;

        /// Move assignment operator
        Block &operator=(Block &&other) = default;

        /// @brief Add a new transaction to the block
        /// @param tx The transaction to add
        /// @throws iotbc::InvalidTransaction if the transaction is invalid
        /// @throws iotbc::InvalidSignature if the transaction signature is invalid
        void addTransaction(const Transaction &tx);

        /// @brief Get the hash of the block
        /// @return The hash of the block
        Hash blockHash() const;

        /// @brief Mine the block
        /// @param difficulty The difficulty of the block
        void mine(int difficulty);

        /// @brief Verify all transactions in the block
        /// @throws iotbc::InvalidTransaction if a transaction is invalid
        /// @throws iotbc::InvalidSignature if a transaction signature is invalid
        void verifyTransactions() const;

        /// @brief Serialize the block into a byte array
        /// @return The serialized block
        std::vector<unsigned char> serialize() const;

        /// @brief Deserialize a block from a byte array
        /// @param data The byte array to deserialize
        /// @return The deserialized block
        /// @throws iotbc::DeserializationError if the data is invalid
        static Block deserialize(const std::vector<unsigned char> &data);
    private:
        /// @brief Calculate the merkle root of the block
        /// @return The merkle root of the block
        Hash calculateMerkleRoot() const;
    };
}
