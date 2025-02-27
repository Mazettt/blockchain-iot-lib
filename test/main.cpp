#include <Block.hpp>
#include <Blockchain.hpp>
#include <Exceptions.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

#include "GuiLayer.hpp"

#include "ConfigLoader.hpp"
#include "ISensor.hpp"
#include "Door.hpp"
#include "Occupancy.hpp"
#include "Potentiometer.hpp"
#include "Thermostat.hpp"
#include "Window.hpp"

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

void sendBlockchainAttributes(const json &attributes, const iotbc::Blockchain &chain) {
    bool valid = true;
    try {
        chain.verifyExistingChain();
    } catch(const iotbc::InvalidBlockchainSave &e) {
        valid = false;
    }

    json data = {
        {"size", chain.chain.size()},
        {"valid", valid}
    };
    std::system(("curl -X POST http://localhost:8080/api/v1/" + std::string(attributes["access_token"]) + "/attributes --header Content-Type:application/json --data \"" + data.dump() + "\"").c_str());
}

int main(int ac, char **av) {
    if (ac == 2 && std::string(av[1]) == "keygen") {
        iotbc::PrivateKey key = generatePseudoRandomPrivateKey();
        savePKeyToFile(key, "./private_key");
        return 0;
    }

    int difficulty = 12;

    ConfigLoader loader("config.json");
    auto sensors = loader.getSensors();
    json attributes = loader.getAttributes();
    iotbc::PrivateKey key = readPKeyFromFile("./private_key");

    iotbc::Blockchain chain;
    chain.loadExistingBlocks("./blocks");
    chain.verifyExistingChain();
    chain.addLayer(std::make_unique<GuiLayer>("config.json"));

    // printCurrentChain(chain);

    while (true) {
        iotbc::Block block(chain.chain.empty() ? iotbc::NULL_HASH : chain.chain.back().blockHash());

        for (const auto &sensor : sensors) {
            std::string data = sensor->genData().dump();
            iotbc::Transaction tx(key, 0, {data.begin(), data.end()});
            tx.sign(key);
            block.addTransaction(tx);
        }

        block.mine(difficulty);
        chain.addBlock(block);
        std::cout << "Added block to chain:" << std::endl;
        printBlock(block);
        std::cout << std::endl;
        chain.saveBlocks("./blocks");

        sendBlockchainAttributes(attributes, chain);

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // std::string userInput = "";

    // std::cout << "Commands: " << std::endl;
    // std::cout << "  mine - Mines a new block" << std::endl;
    // std::cout << "  view - Views the current chain" << std::endl;
    // std::cout << "  exit - Exits the program and saves the current chain state" << std::endl;
    // std::cout << "  Anything else will be treated as a transaction with the input as data" << std::endl;

    // while (true) {
    //     std::cout << "> ";
    //     std::getline(std::cin, userInput);

    //     if (std::cin.eof() || userInput == "exit") {
    //         chain.saveBlocks("./blocks");
    //         break;
    //     } else if (userInput == "mine") {
    //         block.mine(difficulty);
    //         chain.addBlock(block);
    //         printCurrentChain(chain);
    //         block = iotbc::Block(chain.chain.back().blockHash());
    //     } else if (userInput == "view") {
    //         printCurrentChain(chain);
    //     } else {
    //         iotbc::Transaction tx(key, 0, {userInput.begin(), userInput.end()});
    //         tx.sign(key);
    //         block.addTransaction(tx);
    //     }
    // }

    return 0;
}
