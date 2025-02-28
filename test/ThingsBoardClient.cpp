#include "ThingsBoardClient.hpp"

ThingsBoardClient::ThingsBoardClient(const std::string &host, const std::string &id, const std::string &accessToken):
    client(host, id),
    conn_opts()
{
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

ThingsBoardClient::~ThingsBoardClient() {
    try {
        client.disconnect();
    } catch (const mqtt::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void ThingsBoardClient::sendTelemetry(const json &data) {
    sendData("v1/devices/me/telemetry", data);
}

void ThingsBoardClient::sendAttributes(const json &data) {
    sendData("v1/devices/me/attributes", data);
}

void ThingsBoardClient::sendData(const std::string &topic, const json &data) {
    try {
        mqtt::message_ptr pubmsg = mqtt::make_message(topic, data.dump());
        pubmsg->set_qos(1);
        client.publish(pubmsg);
    } catch (const mqtt::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
