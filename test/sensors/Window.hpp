#pragma once

#include "ASensor.hpp"

class Window: public ASensor {
    public:
        Window(const std::string &id):
            ASensor(id)
        {}
        virtual ~Window() = default;

    protected:
        virtual json genDataImpl() const override {
            return json{{ "open", getRandomBool() }};
        }
};
