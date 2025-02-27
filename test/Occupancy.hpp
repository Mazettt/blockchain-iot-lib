#pragma once

#include "ASensor.hpp"

class Occupancy: public ASensor {
    public:
        Occupancy(const std::string &id, const std::string &accessToken):
            ASensor(id, accessToken)
        {}
        virtual ~Occupancy() = default;

    protected:
        virtual json genDataImpl() const override {
            json data;
            data["detected"] = getRandomBool();
            return data;
        }
};
