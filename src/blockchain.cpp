#include "blockchain.hpp"

using namespace iotbc;
namespace fs = std::filesystem;

blockchain::blockchain(const std::string &folderPath):
    folderPath(folderPath), auth(folderPath + "/keys")
{
    if (!fs::exists(folderPath)) {
        fs::create_directory(folderPath);
    }

    for (const auto &entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            blocks.push_back(entry.path().filename().string());
            // std::cout << "Block found with ID: " << entry.path().filename().string() << std::endl;
        }
    }
    std::cout << "Blockchain loaded with " << blocks.size() << " blocks" << std::endl;
}

void blockchain::addBlock(const std::vector<char> &data, const std::string &privateKeyString) {
    if (!auth.authenticate(privateKeyString)) {
        throw std::runtime_error("Not authorized");
    }

    std::string uniqueId = calculateUniqueId(data);
    std::ofstream outFile(folderPath + "/" + uniqueId, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(data.data(), data.size());
        outFile.close();
        blocks.push_back(uniqueId);
        std::cout << "Block added with ID: '" << uniqueId << "'" << std::endl;
    } else {
        throw std::runtime_error("Unable to open file to write block");
    }
}

void blockchain::addBlock(const std::string &data, const std::string &privateKeyString) {
    addBlock(std::vector<char>(data.begin(), data.end()), privateKeyString);
}

std::vector<char> blockchain::getBlock(const std::string &uniqueId, const std::string &privateKeyString) {
    if (!auth.authenticate(privateKeyString)) {
        throw std::runtime_error("Not authorized");
    }

    if (std::find(blocks.begin(), blocks.end(), uniqueId) == blocks.end()) {
        throw std::runtime_error("Block of ID '" + uniqueId + "' not found");
    }

    std::ifstream inFile(folderPath + "/" + uniqueId, std::ios::binary);
    if (inFile.is_open()) {
        std::vector<char> data{std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>()};
        inFile.close();
        return data;
    } else {
        throw std::runtime_error("Unable to open file to read block");
    }
}

std::string blockchain::calculateUniqueId(const std::vector<char> &data) {
    std::stringstream ss;
    for (const auto &block : blocks) {
        ss << block;
    }
    ss.write(data.data(), data.size());
    return crypto::hash(ss.str());
}
