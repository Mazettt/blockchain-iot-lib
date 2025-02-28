#pragma once

#include <string>
#include <json.hpp>
#include <mqtt/client.h>

using json = nlohmann::json;

class MqttClient {
public:
    MqttClient(const std::string &host, const std::string &id, const std::string &accessToken): client(host, id), conn_opts() {
        conn_opts.set_keep_alive_interval(20);
        conn_opts.set_clean_session(false);
        conn_opts.set_user_name(accessToken);

        try {
            client.connect(conn_opts);
            std::cout << "Connected to MQTT at " << host << " with id " << id << std::endl;
        } catch (const mqtt::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }

    ~MqttClient() {
        try {
            client.disconnect();
        } catch (const mqtt::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    MqttClient(const MqttClient&) = delete;
    MqttClient(MqttClient&&) = delete;
    MqttClient& operator=(const MqttClient&) = delete;
    MqttClient& operator=(MqttClient&&) = delete;

    void sendTelemetry(const json &data) {
        sendMessage("v1/devices/me/telemetry", data);
    }

    void sendAttributes(const json &data) {
        sendMessage("v1/devices/me/attributes", data);
    }

    void sendMessage(const std::string &topic, const json &data) {
        try {
            mqtt::message_ptr pubmsg = mqtt::make_message(topic, data.dump());
            pubmsg->set_qos(1);
            client.publish(pubmsg);
        } catch (const mqtt::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

private:
    mqtt::client client;
    mqtt::connect_options conn_opts;
};
