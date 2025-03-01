#include "ASensor.hpp"

json ASensor::genData() const {
    json data = genDataImpl();
    return {
        {"id", getId()},
        {"data", data}
    };
}

int ASensor::getRandomNumber(int min, int max) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

double ASensor::getRandomNumber(double min, double max) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

bool ASensor::getRandomBool() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    return dis(gen);
}
