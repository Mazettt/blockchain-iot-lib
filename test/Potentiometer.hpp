#pragma once

#include "ASensor.hpp"

class Potentiometer: public ASensor {
    public:
        Potentiometer(const std::string &id, const std::string &accessToken):
            ASensor(id, accessToken)
        {}
        virtual ~Potentiometer() = default;

    protected:
        virtual json genDataImpl() const override {
            json data;
            data["state"] = getRandomNumber(0, 100);
            return data;
        }
};
