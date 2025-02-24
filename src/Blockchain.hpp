#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <openssl/sha.h>
#include <secp256k1.h>

#include <Block.hpp>
#include <Types.hpp>
#include <ILayer.hpp>

namespace iotbc {
    class Blockchain {
    public:
        std::vector<Block> chain;
        std::vector<std::shared_ptr<ILayer>> layers;

        Blockchain();

        /// @brief Loads existing blocks from a folder
        /// @param folderPath 
        /// @throws iotbc::IoError if there is an issue reading the folder
        /// @throws iotbc::InvalidBlockchainSave if the blockchain is invalid
        void loadExistingBlocks(const std::string &folderPath);

        /// @brief Verifies the existing chain has valid blocks connected together
        /// and each block has properly signed transactions
        /// @throws iotbc::InvalidBlockchain if the chain is invalid
        /// @note This function does not verify the genesis block. Nodes should agree on it before hand
        void verifyExistingChain();

        /// @brief Saves the blockchain to a folder
        /// @param folderPath The folder to save the blockchain to
        /// @throws iotbc::IoError if there is an issue writing the folder
        void saveBlocks(const std::string &folderPath) const;

        /// @brief Checks if the chain is empty
        /// @return True if the chain is empty, false otherwise
        inline bool empty() const
        {
            return chain.empty();
        }

        /// @brief Add a new block to the chain
        /// @param block The block to add
        /// @throws iotbc::InvalidBlock if the block is invalid
        void addBlock(const Block &block);

        /// @brief Adds a new layer to the blockchain
        /// @param layer The layer to add
        inline void addLayer(const std::shared_ptr<ILayer> &layer)
        {
            layers.emplace_back(layer);
        }
    };
}