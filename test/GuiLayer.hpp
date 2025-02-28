#pragma once

#include <fstream>

#include <Block.hpp>
#include <ILayer.hpp>
#include <json.hpp>
#include "MqttClient.hpp"

using json = nlohmann::json;
using sensorId = std::string;

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
            sensors.emplace(c["id"], std::make_unique<MqttClient>("tcp://localhost:1883", c["id"], c["access_token"]));
        }
    }

    virtual void processBlock(const iotbc::Block &block) override final {
        for (const auto &tx : block.transactions) {
            json blockData = json::parse(tx.data.begin(), tx.data.end());
            sensors.at(blockData["id"])->sendTelemetry(blockData["data"]);
        }
    }

private:
    std::unordered_map<sensorId, std::unique_ptr<MqttClient>> sensors;
};
