#pragma once

#include "ASensor.hpp"

class Thermostat: public ASensor {
    public:
        Thermostat(const std::string &id, const std::string &accessToken):
            ASensor(id, accessToken)
        {}
        virtual ~Thermostat() = default;

    protected:
        virtual json genDataImpl() const override {
            json data;
            data["temperature"] = getRandomNumber(20, 30);
            data["humidity"] = getRandomNumber(60, 70);
            data["pressure"] = getRandomNumber(950, 1050);
            return data;
        }
};
