#include <Block.hpp>
#include <Blockchain.hpp>
#include <Exceptions.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

#include <chrono>
#include <numeric>
#include <iomanip>
#include <algorithm>

// For testing environments generating a pseudo-random private key is okay
iotbc::PrivateKey generatePseudoRandomPrivateKey() {
    iotbc::PrivateKey key;
    for (size_t i = 0; i < key.size(); i++) {
        key[i] = rand() % 256;
    }

    return key;
}

iotbc::PrivateKey readPKeyFromFile(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + path);
    }

    iotbc::PrivateKey key;
    file.read(reinterpret_cast<char*>(key.data()), key.size());

    if (!file) {
        throw std::runtime_error("Error reading file: " + path);
    }

    return key;
}

void savePKeyToFile(const iotbc::PrivateKey &key, const std::string &path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + path);
    }

    file.write(reinterpret_cast<const char*>(key.data()), key.size());

    if (!file) {
        throw std::runtime_error("Error writing file: " + path);
    }
}

std::ostream &operator<<(std::ostream &os, const iotbc::Hash &hash) {
    for (const auto &byte : hash) {
        os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return os;
}

void printBlock(const iotbc::Block &block) {
    std::cout << "PrevHash: " << block.prevHash << std::endl;
    std::cout << "Block: " << block.blockHash() << std::endl;

    for (const auto &tx : block.transactions) {
        std::cout << "  Tx: " << tx.txHash() << std::endl;
        std::cout << "  From: " << iotbc::Address::fromPublicKey(tx.from).toString() << std::endl;
        std::string dataAsString(tx.data.begin(), tx.data.end());
        std::cout << "  Data (as UTF-8): " << dataAsString << std::endl;
    }
}

void printCurrentChain(const iotbc::Blockchain &chain) {
    for (const auto &block : chain.chain) {
        printBlock(block);
        std::cout << std::endl;
    }
}

int main(int ac, char **av) {
    if (ac == 2 && std::string(av[1]) == "keygen") {
        iotbc::PrivateKey key = generatePseudoRandomPrivateKey();
        savePKeyToFile(key, "./private_key");
        return 0;
    }

    int difficulty = 12;

    iotbc::PrivateKey key = readPKeyFromFile("./private_key");

    iotbc::Blockchain chain;
    // chain.loadExistingBlocks("./blocks");
    // chain.verifyExistingChain();

    std::vector<unsigned char> data;
    data.reserve(4096);
    for (int i = 0; i < 4096; i++) {
        data.push_back(rand() % 256);
    }

    size_t blockCount = 24;
    size_t txPerBlock = 128;

    std::vector<std::chrono::duration<double>> miningTimes;
    std::vector<std::chrono::duration<double>> verificationTimes;

    for(size_t i = 0; i < blockCount; i++) {
        iotbc::Block block(chain.chain.empty() ? iotbc::NULL_HASH : chain.chain.back().blockHash());

        iotbc::Transaction tx(key, 0, {data.begin(), data.end()});
        tx.sign(key);
        for (size_t i = 0; i < txPerBlock; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            block.addTransaction(tx);
            auto end = std::chrono::high_resolution_clock::now();
            verificationTimes.push_back(end - start);
        }

        auto start = std::chrono::high_resolution_clock::now();
        block.mine(difficulty);
        auto end = std::chrono::high_resolution_clock::now();
        miningTimes.push_back(end - start);
        chain.addBlock(block);
    }

    std::cout << "Blocks mined: " << blockCount << std::endl;
    std::cout << "Transaction size: " << data.size() << " bytes" << std::endl;
    std::cout << "Transactions per block: " << txPerBlock << std::endl;

    std::cout << std::endl;

    std::chrono::duration<double> totalMiningTime = std::accumulate(miningTimes.begin(), miningTimes.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> averageMiningTime = totalMiningTime / blockCount;
    std::sort(miningTimes.begin(), miningTimes.end());
    std::chrono::duration<double> medianMiningTime = miningTimes[blockCount / 2];

    std::cout << "Average mining time: " << averageMiningTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Median mining time: " << medianMiningTime.count() * 1000 << "ms" << std::endl;

    std::chrono::duration<double> totalVerificationTime = std::accumulate(verificationTimes.begin(), verificationTimes.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> averageVerificationTime = totalVerificationTime / (blockCount * 100);
    std::sort(verificationTimes.begin(), verificationTimes.end());
    std::chrono::duration<double> medianVerificationTime = verificationTimes[verificationTimes.size() / 2];

    std::cout << std::endl;

    std::cout << "Average verification time: " << averageVerificationTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Median verification time: " << medianVerificationTime.count() * 1000 << "ms" << std::endl;

    size_t chainByteSize = 0;
    size_t txCount = 0;
    for (const auto &block : chain.chain) {
        chainByteSize += block.serialize().size();
        txCount += block.transactions.size();
    }

    std::cout << std::endl;

    std::cout << "Total transactions: " << txCount << std::endl;
    std::cout << "Chain size: " << chainByteSize << " bytes" << std::endl;

    return 0;
}
