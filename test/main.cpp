#include <blockchain.hpp>
#include <iostream>

int main() {
    std::cout << "Testing the library..." << std::endl;

    iotbc::blockchain bc;
    bc.helloWorld();

    return 0;
}
