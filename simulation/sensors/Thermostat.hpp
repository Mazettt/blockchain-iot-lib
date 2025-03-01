#pragma once

#include "ASensor.hpp"

class Thermostat: public ASensor {
    public:
        Thermostat(const std::string &id):
            ASensor(id)
        {}
        virtual ~Thermostat() = default;

    protected:
        virtual json genDataImpl() const override {
            return json{
                { "temperature", getRandomNumber(20, 30) },
                { "humidity", getRandomNumber(60, 70) },
                { "pressure", getRandomNumber(950, 1050) }
            };
        }
};
