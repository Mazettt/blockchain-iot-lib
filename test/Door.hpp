#pragma once

#include "ASensor.hpp"

class Door: public ASensor {
    public:
        Door(const std::string &id):
            ASensor(id)
        {}
        virtual ~Door() = default;

    protected:
        virtual json genDataImpl() const override {
            json data;
            data["open"] = getRandomBool();
            return data;
        }
};
