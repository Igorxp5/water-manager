#include <Arduino.h>

#include "Manager.h"

const long ITEM_NOT_FOUND = -1;

Manager::Manager() {

}

OperationMode Manager::getMode() {
    return this->mode;
}

RuntimeError** Manager::getErrors() {
    return this->errors;
}

WaterTank* Manager::getWanterTank(String name) {
    long waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        throw WATER_TANK_NOT_FOUND;
    }
    return this->waterTanks[waterTankIndex];
}

void Manager::setOperationMode(OperationMode mode) {
    this->mode = mode;
}

void Manager::registerWaterTank(String name, WaterTank* waterTank) {
    if (this->getWaterTankIndex(name) != ITEM_NOT_FOUND) {
        throw WATER_TANK_ALREADY_REGISTERED;
    }
    this->totalWaterTanks += 1;

    this->errors = (RuntimeError**) realloc(this->errors, this->totalWaterTanks * sizeof(RuntimeError*));
    this->waterTanks = (WaterTank**) realloc(this->waterTanks, this->totalWaterTanks * sizeof(WaterTank*));
    this->waterTankNames = (String*) realloc(this->waterTankNames, this->totalWaterTanks * sizeof(String));

    this->errors[this->totalWaterTanks - 1] = NULL;
    this->waterTankNames[this->totalWaterTanks - 1] = name;
    this->waterTanks[this->totalWaterTanks - 1] = waterTank;
}

void Manager::unregisterWaterTank(String name) {
    long waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        throw WATER_TANK_NOT_FOUND;
    }
    for (unsigned int i = waterTankIndex + 1; i < this->totalWaterTanks; i++) {
        this->errors[i - 1] = this->errors[i];
        this->waterTanks[i - 1] = this->waterTanks[i];
        this->waterTankNames[i - 1] = this->waterTankNames[i];
    }

    this->totalWaterTanks -= 1;

    this->errors = (RuntimeError**) realloc(this->errors, this->totalWaterTanks * sizeof(RuntimeError*));
    this->waterTanks = (WaterTank**) realloc(this->waterTanks, this->totalWaterTanks * sizeof(WaterTank*));
    this->waterTankNames = (String*) realloc(this->waterTankNames, this->totalWaterTanks * sizeof(String));
}

void Manager::fillWaterTank(String name) {
    WaterTank* waterTank = this->getWanterTank(name);
    waterTank->fill(this->mode == MANUAL);
}

void Manager::stopFillingWaterTank(String name) {
    WaterTank* waterTank = this->getWanterTank(name);
    waterTank->stopFilling();
}

void Manager::loop() {
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        this->errors[i] = this->waterTanks[i]->loop();
    }
}

long Manager::getWaterTankIndex(String name) {
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        if (this->waterTankNames[i] == name) {
            return (long) i;
        }
    }
    return ITEM_NOT_FOUND;
}
