#include <Arduino.h>

#include "Manager.h"
#include "Utils.h"

const int ITEM_NOT_FOUND = -1;

Manager::Manager() {

}

OperationMode Manager::getMode() {
    return this->mode;
}

const RuntimeError** Manager::getErrors() {
    return this->errors;
}

WaterTank* Manager::getWaterTank(String name) {
    int waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        //throw WATER_TANK_NOT_FOUND;
    }
    return this->waterTanks[waterTankIndex];
}

WaterSource* Manager::getWaterSource(String name) {
    int waterSourceIndex = this->getWaterSourceIndex(name);
    if (waterSourceIndex == ITEM_NOT_FOUND) {
        //throw WATER_SOURCE_NOT_FOUND;
    }
    return this->waterSources[waterSourceIndex];
}

unsigned int Manager::getWaterSourceNames(String* list) {
    list = (String*) realloc(list, this->totalWaterSources * sizeof(String));
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        list[i] = *this->waterSourceNames[i];
    }
    return this->totalWaterSources;
}

unsigned int Manager::getWaterTankNames(String* list) {
    list = (String*) realloc(list, this->totalWaterTanks * sizeof(String));
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        list[i] = *this->waterTankNames[i];
    }
    return this->totalWaterTanks;
}

unsigned int Manager::getErrors(const RuntimeError** list) {
    list = (const RuntimeError**) realloc(list, this->totalWaterTanks * sizeof(const RuntimeError*));
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        list[i] = this->errors[i];
    }
    return this->totalWaterTanks;
}

void Manager::setOperationMode(OperationMode mode) {
    this->mode = mode;
}

void Manager::setWaterSourceState(String name, bool enabled) {
    WaterSource* waterSource = this->getWaterSource(name);
    if (this->mode == AUTOMATIC) {
        //throw CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC;
    }
    if (enabled) {
        waterSource->enable();
    } else {
        waterSource->disable();
    }
}

void Manager::registerWaterSource(String name, WaterSource* waterSource) {
    if (this->getWaterSourceIndex(name) != ITEM_NOT_FOUND) {
        //throw WATER_SOURCE_ALREADY_REGISTERED;
    } else {
        this->totalWaterSources += 1;

        this->waterSources = (WaterSource**) realloc(this->waterSources, this->totalWaterSources * sizeof(WaterSource*));
        this->waterSourceNames = (String**) realloc(this->waterSourceNames, this->totalWaterSources * sizeof(String*));

        this->waterSources[this->totalWaterSources - 1] = waterSource;
        this->waterSourceNames[this->totalWaterSources - 1] = new String(name);
    }
}

WaterSource* Manager::unregisterWaterSource(String name) {
    WaterSource* waterSource = NULL;

    int waterSourceIndex = this->getWaterSourceIndex(name);
    if (waterSourceIndex == ITEM_NOT_FOUND) {
        //throw WATER_SOURCE_NOT_FOUND;
    } else {
        waterSource = this->waterSources[waterSourceIndex];
        String* waterSourceName = this->waterSourceNames[waterSourceIndex];

        for (unsigned int i = waterSourceIndex + 1; i < this->totalWaterSources; i++) {
            this->waterSources[i - 1] = this->waterSources[i];
            this->waterSourceNames[i - 1] = this->waterSourceNames[i];
        }

        this->totalWaterSources -= 1;

        this->waterSources = (WaterSource**) realloc(this->waterSources, this->totalWaterSources * sizeof(WaterSource*));
        this->waterSourceNames = (String**) realloc(this->waterSourceNames, this->totalWaterSources * sizeof(String*));
        
        delete waterSourceName;
    }
    return waterSource;
}

void Manager::registerWaterTank(String name, WaterTank* waterTank) {
    if (this->getWaterTankIndex(name) != ITEM_NOT_FOUND) {
        //throw WATER_TANK_ALREADY_REGISTERED;
    } else {
        this->totalWaterTanks += 1;

        this->errors = (const RuntimeError**) realloc(this->errors, this->totalWaterTanks * sizeof(const RuntimeError*));
        this->waterTanks = (WaterTank**) realloc(this->waterTanks, this->totalWaterTanks * sizeof(WaterTank*));
        this->waterTankNames = (String**) realloc(this->waterTankNames, this->totalWaterTanks * sizeof(String*));

        this->errors[this->totalWaterTanks - 1] = NULL;
        this->waterTankNames[this->totalWaterTanks - 1] = new String(name);
        this->waterTanks[this->totalWaterTanks - 1] = waterTank;
    }
}

WaterTank* Manager::unregisterWaterTank(String name) {
    WaterTank* waterTank = NULL;
    int waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        //throw WATER_TANK_NOT_FOUND;
    } else {
        waterTank = this->waterTanks[waterTankIndex];
        String* waterTankName = this->waterTankNames[waterTankIndex];

        for (unsigned int i = waterTankIndex + 1; i < this->totalWaterTanks; i++) {
            this->errors[i - 1] = this->errors[i];
            this->waterTanks[i - 1] = this->waterTanks[i];
            this->waterTankNames[i - 1] = this->waterTankNames[i];
        }

        this->totalWaterTanks -= 1;

        this->errors = (const RuntimeError**) realloc(this->errors, this->totalWaterTanks * sizeof(const RuntimeError*));
        this->waterTanks = (WaterTank**) realloc(this->waterTanks, this->totalWaterTanks * sizeof(WaterTank*));
        this->waterTankNames = (String**) realloc(this->waterTankNames, this->totalWaterTanks * sizeof(String*));
        
        delete waterTankName;
    }
    return waterTank;
}

void Manager::fillWaterTank(String name) {
    WaterTank* waterTank = this->getWaterTank(name);
    waterTank->fill(this->mode == MANUAL);
}

void Manager::stopFillingWaterTank(String name) {
    WaterTank* waterTank = this->getWaterTank(name);
    waterTank->stopFilling();
}

void Manager::loop() {
    if (this->mode == AUTOMATIC) {
        for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
            this->errors[i] = this->waterTanks[i]->loop();
        }
    }
}

int Manager::getWaterTankIndex(String name) {
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        if (*this->waterTankNames[i] == name) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}

int Manager::getWaterSourceIndex(String name) {
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        if (*this->waterSourceNames[i] == name) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}
