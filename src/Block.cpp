#include <limits>
#include <bit>

#include <Block.hpp>
#include <Exceptions.hpp>

namespace iotbc {
    static bool hasLeadingZeroBits(const Hash& hash, int difficulty) {
        int zeroBits = 0;

        for (unsigned char byte : hash) {
            if (byte == 0) {
                zeroBits += 8;
            } else {
                zeroBits += std::countl_zero(byte);
                break;
            }

            if (zeroBits >= difficulty) {
                return true;
            }
        }

        return zeroBits >= difficulty;
    }
    
    static Hash hash_two_hashes(const Hash &a, const Hash &b) {
        EVP_MD_CTX *ctx = EVP_MD_CTX_new();

        if (ctx == nullptr) {
            throw EvpError("Failed to create EVP_MD_CTX");
        }

        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to initialize digest");
        }

        if (EVP_DigestUpdate(ctx, a.data(), sizeof(Hash)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        if (EVP_DigestUpdate(ctx, b.data(), sizeof(Hash)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        Hash result;
        unsigned int result_len = sizeof(Hash);

        if (EVP_DigestFinal_ex(ctx, result.data(), &result_len) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to finalize digest");
        }

        EVP_MD_CTX_free(ctx);

        return result;
    }

    Block::Block(Hash prevHash) : prevHash(prevHash), transactions(), merkleRoot(NULL_HASH), nonce(0) {
        
    }

    void Block::addTransaction(const Transaction &tx) {
        tx.verify();
        transactions.push_back(tx);
    }

    Hash Block::blockHash() const {
        EVP_MD_CTX *ctx = EVP_MD_CTX_new();

        if (ctx == nullptr) {
            throw EvpError("Failed to create EVP_MD_CTX");
        }

        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to initialize digest");
        }

        if (EVP_DigestUpdate(ctx, prevHash.data(), sizeof(Hash)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        if (EVP_DigestUpdate(ctx, merkleRoot.data(), sizeof(Hash)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        unsigned char nonce_bytes[sizeof(Nonce)];
        std::memcpy(nonce_bytes, &nonce, sizeof(Nonce));

        if (EVP_DigestUpdate(ctx, nonce_bytes, sizeof(Nonce)) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to update digest");
        }

        Hash result;
        unsigned int result_len = sizeof(Hash);

        if (EVP_DigestFinal_ex(ctx, result.data(), &result_len) != 1) {
            EVP_MD_CTX_free(ctx);
            throw EvpError("Failed to finalize digest");
        }

        EVP_MD_CTX_free(ctx);

        return result;   
    }

    void Block::mine(int difficulty) {
        merkleRoot = calculateMerkleRoot();

        std::array<unsigned char, sizeof(Hash) + sizeof(Hash) + sizeof(Nonce)> data;
        std::copy(prevHash.data(), prevHash.data() + sizeof(Hash), data.begin());
        std::copy(merkleRoot.data(), merkleRoot.data() + sizeof(Hash), data.begin() + sizeof(Hash));

        while (nonce < std::numeric_limits<Nonce>::max()) {
            Hash computedHash = blockHash();

            bool valid = hasLeadingZeroBits(computedHash, difficulty);

            if (valid) {
                return;
            }

            nonce++;
        }

        throw std::runtime_error("Failed to mine block");
    }

    void Block::verifyTransactions() const {
        for (const Transaction &tx : transactions) {
            tx.verify();
        }
    }

    std::vector<unsigned char> Block::serialize() const {
        std::vector<unsigned char> buf;

        buf.insert(buf.end(), prevHash.data(), prevHash.data() + sizeof(Hash));

        size_t txCount = transactions.size();
        for (size_t i = 0; i < sizeof(size_t); i++) {
            buf.push_back((txCount >> (i * sizeof(size_t)) & 0xFF));
        }

        for (const Transaction &tx : transactions) {
            size_t txSize = tx.size();
            for (size_t i = 0; i < sizeof(size_t); i++) {
                buf.push_back((txSize >> (i * sizeof(size_t)) & 0xFF));
            }

            std::vector<unsigned char> txData = tx.serialize();
            for (size_t i = 0; i < txData.size(); i++) {
                buf.push_back(txData[i]);
            }
        }

        buf.insert(buf.end(), merkleRoot.data(), merkleRoot.data() + sizeof(Hash));

        for (size_t i = 0; i < sizeof(Nonce); i++) {
            buf.push_back((nonce >> (i * sizeof(Nonce)) & 0xFF));
        }

        auto bHash = this->blockHash();
        buf.insert(buf.end(), bHash.data(), bHash.data() + sizeof(Hash));

        return buf;
    }

    Block Block::deserialize(const std::vector<unsigned char> &data) {
        size_t cur = 0;

        Hash prevHash;
        for (size_t i = 0; i < prevHash.size(); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            prevHash[i] = data[cur++];
        }

        size_t txCount = 0;

        for (size_t i = 0; i < sizeof(size_t); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            txCount |= data[cur++] << (i * sizeof(size_t));
        }

        Block block(prevHash);

        for (size_t i = 0; i < txCount; i++) {
            size_t txSize = 0;

            for (size_t j = 0; j < sizeof(size_t); j++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                txSize |= data[cur++] << (j * sizeof(size_t));
            }

            std::vector<unsigned char> txData;
            for (size_t j = 0; j < txSize; j++) {
                if (cur >= data.size()) {
                    throw DeserializationError("Overflow");
                }
                txData.push_back(data[cur++]);
            }

            block.addTransaction(Transaction::deserialize(txData));
        }

        for (size_t i = 0; i < block.merkleRoot.size(); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            block.merkleRoot[i] = data[cur++];
        }

        for (size_t i = 0; i < sizeof(Nonce); i++) {
            if (cur >= data.size()) {
                throw DeserializationError("Overflow");
            }
            block.nonce |= data[cur++] << (i * sizeof(Nonce));
        }

        return block;
    }

    Hash Block::calculateMerkleRoot() const {
        std::vector<Hash> merkleTree;

        if (transactions.empty()) {
            return EMPTY_STRING_HASH;
        }

        for (const Transaction &tx : transactions) {
            merkleTree.push_back(tx.txHash());
        }

        while (merkleTree.size() > 1) {
            std::vector<Hash> newTree;

            for (size_t i = 0; i < merkleTree.size(); i += 2) {
                if (i + 1 < merkleTree.size()) {
                    newTree.push_back(hash_two_hashes(merkleTree[i], merkleTree[i + 1]));
                } else {
                    newTree.push_back(merkleTree[i]);
                }
            }

            merkleTree = newTree;
        }

        return merkleTree[0];
    }
}
