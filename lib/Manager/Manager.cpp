#include <Arduino.h>

#include "Manager.h"
#include "Utils.h"

const int ITEM_NOT_FOUND = -1;

Manager::Manager() {

}

OperationMode Manager::getOperationMode() {
    return this->mode;
}

WaterTank* Manager::getWaterTank(String name) {
    int waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        //throw WATER_TANK_NOT_FOUND;
    }
    return this->waterTanks[waterTankIndex];
}

WaterSource* Manager::getWaterSource(char* name) {
    WaterSource* waterSource = NULL;
    int waterSourceIndex = this->getWaterSourceIndex(name);
    if (waterSourceIndex == ITEM_NOT_FOUND) {
        Exception::throwException(WATER_SOURCE_NOT_FOUND);
    } else {
        waterSource = this->waterSources[waterSourceIndex];
    }
    return waterSource;
}

char** Manager::getWaterSourceNames() {
    char** list = (char**) malloc(this->totalWaterSources * sizeof(char*));
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        list[i] = this->waterSourceNames[i];
    }
    return list;
}

unsigned int Manager::getWaterTankNames(String* list) {
    list = (String*) realloc(list, this->totalWaterTanks * sizeof(String));
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        list[i] = *this->waterTankNames[i];
    }
    return this->totalWaterTanks;
}

unsigned int Manager::getTotalWaterSources() {
    return this->totalWaterSources;
}

void Manager::setOperationMode(OperationMode mode) {
    this->mode = mode;
}

void Manager::setWaterSourceState(char* name, bool enabled) {
    WaterSource* waterSource = this->getWaterSource(name);
    if (!Exception::hasException()) {
        if (this->mode == AUTO) {
            return Exception::throwException(CANNOT_HANDLE_WATER_SOURCE_IN_AUTO);
        }
        if (enabled) {
            waterSource->enable();
        } else {
            waterSource->disable();
        }
    }
}

void Manager::registerWaterSource(char* name, WaterSource* waterSource) {
    if (this->isWaterSourceRegistered(name)) {
        return Exception::throwException(WATER_SOURCE_ALREADY_REGISTERED);
    } else if(this->totalWaterSources + 1 > MAX_WATER_SOURCES) {
        return Exception::throwException(MAX_WATER_SOURCES_ERROR);
    } else {
        this->totalWaterSources += 1;

        char* waterSourceName = new char[MAX_NAME_LENGTH + 1];
        strcpy(waterSourceName, name);
        this->waterSources[this->totalWaterSources - 1] = waterSource;
        this->waterSourceNames[this->totalWaterSources - 1] = waterSourceName;
    }
}

WaterSource* Manager::unregisterWaterSource(char* name) {
    WaterSource* waterSource = NULL;

    if (!this->isWaterSourceRegistered(name)) {
        Exception::throwException(WATER_SOURCE_NOT_FOUND);
    } else {
        int waterSourceIndex = this->getWaterSourceIndex(name);

        waterSource = this->waterSources[waterSourceIndex];
        char* waterSourceName = this->waterSourceNames[waterSourceIndex];

        for (unsigned int i = waterSourceIndex + 1; i < this->totalWaterSources; i++) {
            this->waterSources[i - 1] = this->waterSources[i];
            this->waterSourceNames[i - 1] = this->waterSourceNames[i];
        }

        this->totalWaterSources -= 1;

        delete waterSourceName;
    }
    return waterSource;
}

bool Manager::isWaterSourceRegistered(char* name) {
    return this->getWaterSourceIndex(name) != ITEM_NOT_FOUND;
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
    if (this->mode == AUTO) {
        for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
            this->errors[i] = this->waterTanks[i]->loop();
        }
    }
}

void Manager::reset() {
    this->setOperationMode(MANUAL);

    unsigned int total = this->totalWaterSources;
    for (unsigned int i = 0; i < total; i++) {
        //turn off all water sources when resetting
        this->setWaterSourceState(this->waterSourceNames[0], false);
        //remove the first water source from the list in each iteration
        delete this->unregisterWaterSource(this->waterSourceNames[0]);
    }
    total = this->totalWaterTanks;
    for (unsigned int j = 0; j < total; j++) {
        //TODO
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

int Manager::getWaterSourceIndex(char* name) {
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        if (strcmp(this->waterSourceNames[i], name) == 0) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}
