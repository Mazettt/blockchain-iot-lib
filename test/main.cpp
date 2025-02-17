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

std::string getRandomString() {
    return "Test " + std::to_string(rand() % 100);
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
        bc.addBlock(getRandomString(), privateKeyString);
        std::vector<char> data = bc.getBlock("a7cb0e4a4f443fdbd2c617d1f659aa9d819dc9f7d618164652dfd6f7ef5c0", privateKeyString);
        for (const auto &c : data) { std::cout << c; } std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
