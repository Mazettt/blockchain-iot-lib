#pragma once

#include <random>
#include "ISensor.hpp"

class ASensor: public ISensor {
public:
    ASensor(const std::string &id, const std::string &accessToken):
        id(id),
        accessToken(accessToken)
    {}
    virtual ~ASensor() = default;

    virtual json genData() const override final {
        json data = genDataImpl();
        return {
            {"id", getId()},
            {"accessToken", getAccessToken()},
            {"data", data}
        };
    }

    virtual std::string getId() const override final {
        return id;
    }

    virtual std::string getAccessToken() const override final {
        return accessToken;
    }

protected:
    virtual json genDataImpl() const = 0;

    virtual int getRandomNumber(int min, int max) const final {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    virtual double getRandomNumber(double min, double max) const final {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }

    virtual bool getRandomBool() const final {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        return dis(gen);
    }

private:
    const std::string id;
    const std::string accessToken;
};
