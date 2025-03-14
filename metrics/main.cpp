#include <Block.hpp>
#include <Blockchain.hpp>
#include <Exceptions.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <string>
#include <random>
#include <cmath>
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

size_t getMemoryUsage() {
    std::ifstream file("/proc/self/status");
    std::string line;
    size_t memoryUsage = 0;

    while (std::getline(file, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {  // Line starts with "VmRSS:"
            std::sscanf(line.c_str(), "VmRSS: %zu", &memoryUsage);
            return memoryUsage; // Memory usage in KB
        }
    }
    return 0; // Failed to read memory usage
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
    std::vector<size_t> memoryUsageWhileRunning;

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

        memoryUsageWhileRunning.push_back(getMemoryUsage());
    }

    std::cout << "Blocks mined: " << blockCount << std::endl;
    std::cout << "Transaction size: " << data.size() << " bytes" << std::endl;
    std::cout << "Transactions per block: " << txPerBlock << std::endl;

    std::cout << std::endl;

    std::chrono::duration<double> totalMiningTime = std::accumulate(miningTimes.begin(), miningTimes.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> averageMiningTime = totalMiningTime / blockCount;
    std::sort(miningTimes.begin(), miningTimes.end());
    std::chrono::duration<double> medianMiningTime = miningTimes[blockCount / 2];

    double miningTimeSum = std::accumulate(miningTimes.begin(), miningTimes.end(), 0.0, [](double sum, const std::chrono::duration<double>& d) {
        return sum + d.count();
    });
    double miningTimeMean = miningTimeSum / miningTimes.size();
    double miningTimeSqSum = std::accumulate(miningTimes.begin(), miningTimes.end(), 0.0, [miningTimeMean](double sum, const std::chrono::duration<double>& d) {
        return sum + (d.count() - miningTimeMean) * (d.count() - miningTimeMean);
    });
    double miningTimeStdDev = std::sqrt(miningTimeSqSum / miningTimes.size());

    std::cout << "Average mining time: " << averageMiningTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Median mining time: " << medianMiningTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Mining time stddev: " << miningTimeStdDev * 1000 << "ms" << std::endl;

    std::chrono::duration<double> totalVerificationTime = std::accumulate(verificationTimes.begin(), verificationTimes.end(), std::chrono::duration<double>(0));
    std::chrono::duration<double> averageVerificationTime = totalVerificationTime / (blockCount * txPerBlock);
    std::sort(verificationTimes.begin(), verificationTimes.end());
    std::chrono::duration<double> medianVerificationTime = verificationTimes[verificationTimes.size() / 2];

    double verificationTimeSum = std::accumulate(verificationTimes.begin(), verificationTimes.end(), 0.0, [](double sum, const std::chrono::duration<double>& d) {
        return sum + d.count();
    });
    double verificationTimeMean = verificationTimeSum / verificationTimes.size();
    double verificationTimeSqSum = std::accumulate(verificationTimes.begin(), verificationTimes.end(), 0.0, [verificationTimeMean](double sum, const std::chrono::duration<double>& d) {
        return sum + (d.count() - verificationTimeMean) * (d.count() - verificationTimeMean);
    });
    double verificationTimeStdDev = std::sqrt(verificationTimeSqSum / verificationTimes.size());

    std::cout << std::endl;

    std::cout << "Average verification time: " << averageVerificationTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Median verification time: " << medianVerificationTime.count() * 1000 << "ms" << std::endl;
    std::cout << "Verification time stddev: " << verificationTimeStdDev * 1000 << "ms" << std::endl;

    size_t chainByteSize = 0;
    size_t txCount = 0;
    for (const auto &block : chain.chain) {
        chainByteSize += block.serialize().size();
        txCount += block.transactions.size();
    }

    double memoryUsageMean = std::accumulate(memoryUsageWhileRunning.begin(), memoryUsageWhileRunning.end(), 0.0) / memoryUsageWhileRunning.size();
    double memoryUsageSqSum = std::accumulate(memoryUsageWhileRunning.begin(), memoryUsageWhileRunning.end(), 0.0, [memoryUsageMean](double sum, size_t mem) {
        return sum + (mem - memoryUsageMean) * (mem - memoryUsageMean);
    });
    double memoryUsageStdDev = std::sqrt(memoryUsageSqSum / memoryUsageWhileRunning.size());
    double minMemoryUsage = *std::min_element(memoryUsageWhileRunning.begin(), memoryUsageWhileRunning.end());
    double maxMemoryUsage = *std::max_element(memoryUsageWhileRunning.begin(), memoryUsageWhileRunning.end());

    std::cout << std::endl;

    std::cout << "Average memory usage: " << memoryUsageMean << "KB" << std::endl;
    std::cout << "Memory usage stddev: " << memoryUsageStdDev << "KB" << std::endl;
    std::cout << "Min memory usage: " << minMemoryUsage << "KB" << std::endl;
    std::cout << "Max memory usage: " << maxMemoryUsage << "KB" << std::endl;


    std::cout << std::endl;

    std::cout << "Total transactions: " << txCount << std::endl;
    std::cout << "Chain size: " << chainByteSize << " bytes" << std::endl;

    return 0;
}
