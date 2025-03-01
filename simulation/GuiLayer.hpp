#pragma once

#include <fstream>

#include <Block.hpp>
#include <ILayer.hpp>
#include <json.hpp>
#include "ThingsBoardClient.hpp"

using json = nlohmann::json;
using sensorId = std::string;
using clientPtr = std::unique_ptr<ThingsBoardClient>;

class GuiLayer: public iotbc::ILayer {
public:
    GuiLayer(const std::string &configPath);
    virtual void processBlock(const iotbc::Block &block) override final;

private:
    std::unordered_map<sensorId, clientPtr> sensors;
};
