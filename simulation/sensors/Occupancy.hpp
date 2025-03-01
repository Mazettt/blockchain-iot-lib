#pragma once

#include "ASensor.hpp"

class Occupancy: public ASensor {
    public:
        Occupancy(const std::string &id):
            ASensor(id)
        {}
        virtual ~Occupancy() = default;

    protected:
        virtual json genDataImpl() const override {
            return json{{ "detected", getRandomBool() }};
        }
};
