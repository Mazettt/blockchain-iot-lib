#include "Blockchain.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <optional>

#include <Utils.hpp>

namespace iotbc {
    struct ArrayHash {
        std::size_t operator()(const Hash &arr) const {
            std::size_t hash = 0;
            for (auto byte : arr) {
                hash ^= std::hash<unsigned char>{}(byte) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

    Blockchain::Blockchain() : chain(), layers() {
    }

    void Blockchain::loadExistingBlocks(const std::string &folderPath) {
        if (!std::filesystem::exists(folderPath)) {
            return;
        }

        std::unordered_map<Hash, Block, ArrayHash> existingBlocks;
        std::optional<Block> genesisBlock = std::nullopt;

        for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::ifstream file(entry.path(), std::ios::binary);

                if (!file.is_open()) {
                    throw IoError("Failed to open file");
                }

                std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                Block block = Block::deserialize(data);

                if (block.prevHash == NULL_HASH) {
                    if (genesisBlock.has_value()) {
                        throw InvalidBlockchainSave("Cannot have multiple genesis blocks");
                    }

                    genesisBlock = block;
                } else {
                    existingBlocks.insert({block.prevHash, block});
                }
            }
        }

        if (!genesisBlock.has_value() && !existingBlocks.empty()) {
            throw InvalidBlockchainSave("Some blocks were found but no genesis block");
        }

        if (!genesisBlock.has_value()) {
            return;
        }

        chain.push_back(genesisBlock.value());

        Hash lastHash = genesisBlock.value().blockHash();

        while (existingBlocks.find(lastHash) != existingBlocks.end()) {
            Block blockToAdd = existingBlocks.extract(lastHash).mapped();
            lastHash = blockToAdd.blockHash();
            chain.push_back(blockToAdd);
        }

        if (!existingBlocks.empty()) {
            std::cerr << "Warning: Some blocks were found but not part of the chain" << std::endl;
        }

        for (const auto &block : chain) {
            for (const auto &layer : layers) {
                layer->processBlock(block);
            }
        }
    }

    void Blockchain::verifyExistingChain() const {
        if (chain.empty()) {
            return;
        }

        chain[0].verifyTransactions();

        for (size_t i = 1; i < chain.size(); i++) {
            chain[i].verifyTransactions();

            if (chain[i].prevHash != chain[i - 1].blockHash()) {
                throw InvalidBlockchainSave("Hash mismatch");
            }
        }
    }

    void Blockchain::saveBlocks(const std::string &folderPath) const {
        if (!std::filesystem::exists(folderPath)) {
            std::filesystem::create_directory(folderPath);
        }

        for (const auto &block : chain) {
            std::ofstream file(folderPath + "/" + hashToString(block.blockHash()), std::ios::binary);

            if (!file.is_open()) {
                throw IoError("Failed to open file");
            }

            auto serialized = block.serialize();
            file.write(reinterpret_cast<const char *>(serialized.data()), serialized.size());

            if (!file) {
                throw IoError("Failed to write to file");
            }
        }
    }

    void Blockchain::addBlock(const Block &block) {
        if (block.merkleRoot == NULL_HASH) {
            throw InvalidBlock("Block should be mined before getting added (merkleRoot is NULL_HASH)");
        }

        try {
            block.verifyTransactions();
        } catch (const InvalidTransaction &e) {
            throw InvalidBlock("Block contains invalid transaction(s)");
        } catch (const InvalidSignature &e) {
            throw InvalidBlock("Block contains transaction(s) with invalid signature(s)");
        } catch (const std::exception &e) {
            throw e;
        }

        if (chain.empty()) {
            if (block.prevHash != NULL_HASH) {
                throw InvalidBlock("The first block of the chain should be the genesis block");
            }
        } else {
            if (block.prevHash != chain.back().blockHash()) {
                throw InvalidBlock("Block prevHash does not match the last block's hash");
            }
        }

        for (const auto &layer : layers) {
            layer->processBlock(block);
        }

        chain.push_back(block);
    }
}
