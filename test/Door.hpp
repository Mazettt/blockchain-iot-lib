#pragma once

#include "ASensor.hpp"

class Door: public ASensor {
    public:
        Door(const std::string &id, const std::string &accessToken):
            ASensor(id, accessToken)
        {}
        virtual ~Door() = default;

    protected:
        virtual json genDataImpl() const override {
            json data;
            data["open"] = getRandomBool();
            return data;
        }
};
