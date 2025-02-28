#pragma once

#include <random>
#include "ISensor.hpp"

class ASensor: public ISensor {
public:
    ASensor(const std::string &id): id(id) {}
    virtual ~ASensor() = default;

    virtual json genData() const override final;
    virtual std::string getId() const override final { return id; }

protected:
    virtual json genDataImpl() const = 0;
    virtual int getRandomNumber(int min, int max) const final;
    virtual double getRandomNumber(double min, double max) const final;
    virtual bool getRandomBool() const final;

private:
    const std::string id;
};
