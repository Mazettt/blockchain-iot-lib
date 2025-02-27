#pragma once

#include <fstream>

#include <Block.hpp>
#include <ILayer.hpp>
#include <json.hpp>

using json = nlohmann::json;

class GuiLayer: public iotbc::ILayer {
public:
    GuiLayer(const std::string &configPath) {
        std::ifstream configFile(configPath);
        if (!configFile) {
            throw std::runtime_error("Unable to open sensors configuration file.");
        }
        json config;
        configFile >> config;
        for (const auto &c : config["telemetry"]) {
            sensors[c["id"]] = c["access_token"];
        }
    }

    virtual void processBlock(const iotbc::Block &block) override final {
        for (const auto &tx : block.transactions) {
            json blockData = json::parse(tx.data.begin(), tx.data.end());
            std::string accessToken = sensors[blockData["id"]];
            std::string data = blockData["data"].dump();
            std::system(("curl -X POST http://localhost:8080/api/v1/" + accessToken + "/telemetry --header Content-Type:application/json --data \"" + data + "\"").c_str());
        }
    }

private:
    std::unordered_map<std::string, std::string> sensors;
};
