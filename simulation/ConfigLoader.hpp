#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include <json.hpp>

#include "sensors/ISensor.hpp"
#include "sensors/Door.hpp"
#include "sensors/Occupancy.hpp"
#include "sensors/Potentiometer.hpp"
#include "sensors/Thermostat.hpp"
#include "sensors/Window.hpp"

using sensorList = std::vector<std::unique_ptr<ISensor>>;

class ConfigLoader {
public:
    ConfigLoader(const std::string &path);

    sensorList getSensors() const;
    nlohmann::json getAttributes() const { return config["attributes"]; }

private:
    nlohmann::json config;
};
