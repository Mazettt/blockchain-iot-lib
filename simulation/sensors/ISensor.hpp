#pragma once

#include <string>
#include <json.hpp>

using json = nlohmann::json;

class ISensor {
    public:
        virtual ~ISensor() = default;
        virtual json genData() const = 0;
        virtual std::string getId() const = 0;
};
