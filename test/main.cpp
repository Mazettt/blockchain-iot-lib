#include <blockchain.hpp>
#include <iostream>

std::string loadFile(const std::string &path) {
    std::ifstream inFile(path);
    if (!inFile.is_open()) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    std::string res = std::string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    return res;
}

std::vector<char> getRandLetters(size_t size) {
    std::vector<char> res(size);
    for (auto &c : res) {
        c = 'a' + rand() % 26;
    }
    return res;
}

int main(int ac, char **av) {
    if (ac == 2 && std::string(av[1]) == "keygen") {
        iotbc::crypto::generateKeyPair("./db", ".");
        return 0;
    }

    std::srand(std::time(nullptr));
    iotbc::blockchain bc("./db");

    try {
        std::string privateKeyString = loadFile("./private_key1.pem");
        bc.addBlock(getRandLetters(10), privateKeyString);
        std::vector<char> data = bc.getBlock("40a71e1cb62ab0eb08052ac78d31b9431c36ff246ad45a922b2785bbf14", privateKeyString);
        for (const auto &c : data) { std::cout << c; } std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
