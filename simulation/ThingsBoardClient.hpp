#pragma once

#include <string>
#include <json.hpp>
#include <mqtt/client.h>

using json = nlohmann::json;

class ThingsBoardClient {
public:
    ThingsBoardClient(const std::string &host, const std::string &id, const std::string &accessToken);
    ~ThingsBoardClient();

    ThingsBoardClient(const ThingsBoardClient&) = delete;
    ThingsBoardClient(ThingsBoardClient&&) = delete;
    ThingsBoardClient& operator=(const ThingsBoardClient&) = delete;
    ThingsBoardClient& operator=(ThingsBoardClient&&) = delete;

    void sendTelemetry(const json &data);
    void sendAttributes(const json &data);

private:
    void sendData(const std::string &topic, const json &data);

    mqtt::client client;
    mqtt::connect_options conn_opts;
};
