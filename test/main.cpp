#include <Block.hpp>
#include <Blockchain.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

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

void printCurrentChain(const iotbc::Blockchain &chain) {
    for (const auto &block : chain.chain) {
        std::cout << "PrevHash: " << block.prevHash << std::endl;
        std::cout << "Block: " << block.blockHash() << std::endl;

        for (const auto &tx : block.transactions) {
            std::cout << "  Tx: " << tx.txHash() << std::endl;
            std::cout << "  From: " << iotbc::Address::fromPublicKey(tx.from).toString() << std::endl;
            std::string dataAsString(tx.data.begin(), tx.data.end());
            std::cout << "  Data (as UTF-8): " << dataAsString << std::endl;
        }
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
    chain.loadExistingBlocks("./blocks");
    chain.verifyExistingChain();

    printCurrentChain(chain);

    iotbc::Block block(chain.chain.empty() ? iotbc::NULL_HASH : chain.chain.back().blockHash());

    std::string userInput = "";

    std::cout << "Commands: " << std::endl;
    std::cout << "  mine - Mines a new block" << std::endl;
    std::cout << "  view - Views the current chain" << std::endl;
    std::cout << "  exit - Exits the program and saves the current chain state" << std::endl;
    std::cout << "  Anything else will be treated as a transaction with the input as data" << std::endl;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, userInput);

        if (std::cin.eof() || userInput == "exit") {
            chain.saveBlocks("./blocks");
            break;
        } else if (userInput == "mine") {
            block.mine(difficulty);
            chain.addBlock(block);
            printCurrentChain(chain);
            block = iotbc::Block(chain.chain.back().blockHash());
        } else if (userInput == "view") {
            printCurrentChain(chain);
        } else {
            iotbc::Transaction tx(key, 0, {userInput.begin(), userInput.end()});
            tx.sign(key);
            block.addTransaction(tx);
        }
    }

    return 0;
}
