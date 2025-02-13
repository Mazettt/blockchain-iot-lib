#include <blockchain.hpp>
#include <iostream>

int main() {
    std::srand(std::time(nullptr));
    iotbc::blockchain bc("./db");

    try {
        std::vector<char> randomData(5);
        for (auto &c : randomData) {
            c = 'a' + rand() % 26;
        }
        bc.addBlock(randomData);
        std::vector<char> data = bc.getBlock("cab2fb941413ca57cdd93383ceeced039aca5b12cbb9a5d79d4662af35bd028");
        for (const auto &c : data) {
            std::cout << c;
        }
        std::cout << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
