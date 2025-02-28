#pragma once

#include "ASensor.hpp"

class Potentiometer: public ASensor {
    public:
        Potentiometer(const std::string &id):
            ASensor(id)
        {}
        virtual ~Potentiometer() = default;

    protected:
        virtual json genDataImpl() const override {
            return json{{ "state", getRandomNumber(0, 100) }};
        }
};
