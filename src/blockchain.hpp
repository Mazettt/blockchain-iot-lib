#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include "crypto.hpp"

namespace iotbc {
    class blockchain {
    public:
        blockchain(const std::string &folderPath);
        ~blockchain() = default;

        void addBlock(const std::vector<char> &data, const std::string &privateKeyString);
        std::vector<char> getBlock(const std::string &uniqueId, const std::string &privateKeyString);

    private:
        const std::string folderPath;
        std::vector<std::string> blocks;
        crypto auth;

        std::string calculateUniqueId(const std::vector<char> &data);
    };
}

#endif
