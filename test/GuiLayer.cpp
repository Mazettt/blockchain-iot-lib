#include "GuiLayer.hpp"

GuiLayer::GuiLayer(const std::string &configPath) {
    std::ifstream configFile(configPath);
    if (!configFile) {
        throw std::runtime_error("Unable to open sensors configuration file.");
    }
    json config;
    configFile >> config;
    for (const auto &c : config["telemetry"]) {
        sensors.emplace(c["id"], std::make_unique<ThingsBoardClient>("tcp://localhost:1883", c["id"], c["access_token"]));
    }
}

void GuiLayer::processBlock(const iotbc::Block &block) {
    for (const auto &tx : block.transactions) {
        json blockData = json::parse(tx.data.begin(), tx.data.end());
        sensors.at(blockData["id"])->sendTelemetry(blockData["data"]);
    }
}
