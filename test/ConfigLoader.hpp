#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include <json.hpp>

#include "ISensor.hpp"
#include "Door.hpp"
#include "Occupancy.hpp"
#include "Potentiometer.hpp"
#include "Thermostat.hpp"
#include "Window.hpp"

class ConfigLoader {
public:
    ConfigLoader(const std::string &path) {
        std::ifstream configFile(path);
        if (!configFile) {
            throw std::runtime_error("Unable to open sensors configuration file.");
        }
        configFile >> config;
    }

    std::vector<std::unique_ptr<ISensor>> getSensors() const {
        std::vector<std::unique_ptr<ISensor>> res;
        for (const auto &c : config["telemetry"]) {
            std::string type = c["type"];
            std::string id = c["id"];

            if (type == "Door") {
                res.push_back(std::make_unique<Door>(id));
            } else if (type == "Occupancy") {
                res.push_back(std::make_unique<Occupancy>(id));
            } else if (type == "Potentiometer") {
                res.push_back(std::make_unique<Potentiometer>(id));
            } else if (type == "Thermostat") {
                res.push_back(std::make_unique<Thermostat>(id));
            } else if (type == "Window") {
                res.push_back(std::make_unique<Window>(id));
            } else {
                throw std::runtime_error("Unknown sensor type: " + type);
            }

            std::cout << "Loaded sensor of type " << type << ": " << id << std::endl;
        }
        return res;
    }

    nlohmann::json getAttributes() const {
        return config["attributes"];
    }

private:
    nlohmann::json config;
};