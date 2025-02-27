#pragma once

#include <Block.hpp>
#include <ILayer.hpp>
#include <json.hpp>

using json = nlohmann::json;

class GuiLayer: public iotbc::ILayer {
    public:
        virtual void processBlock(const iotbc::Block &block) override final {
            for (const auto &tx : block.transactions) {
                json data = json::parse(tx.data.begin(), tx.data.end());
                std::system(("curl -X POST http://localhost:8080/api/v1/" + std::string(data["accessToken"]) + "/telemetry --header Content-Type:application/json --data \"" + data["data"].dump() + "\"").c_str());
            }
        }
};
