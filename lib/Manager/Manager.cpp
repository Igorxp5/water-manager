#include <Arduino.h>

#include "Manager.h"
#include "Utils.h"

const int ITEM_NOT_FOUND = -1;

Manager::Manager() : waterTanksLoopErrors() {
    this->waterTanksErrorsTimer = new Clock();
    this->waterTanksErrorsTimer->startTimer();
}

Manager::~Manager() {
    delete this->waterTanksErrorsTimer;

    char* name;
    WaterTank* waterTank;
    WaterSource* waterSource;
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        name = this->waterSourceNames[i];
        waterSource = this->waterSources[i]; 
        delete[] name;
        delete waterSource;
    }

    for (unsigned int j = 0; j < this->totalWaterTanks; j++) {
        name = this->waterTankNames[j];
        waterTank = this->waterTanks[j]; 
        delete[] name;
        delete waterTank;
    }
    
    IOInterface::removeAll();
}

OperationMode Manager::getOperationMode() {
    return this->mode;
}

void Manager::setOperationMode(OperationMode mode) {
    this->mode = mode;
}

WaterTank* Manager::getWaterTank(char* name) {
    WaterTank* waterTank = NULL;
    int waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex == ITEM_NOT_FOUND) {
        Exception::throwException(&WATER_TANK_NOT_FOUND);
    } else {
        waterTank = this->waterTanks[waterTankIndex]; 
    }
    return waterTank;
}

WaterSource* Manager::getWaterSource(char* name) {
    WaterSource* waterSource = NULL;
    int waterSourceIndex = this->getWaterSourceIndex(name);
    if (waterSourceIndex == ITEM_NOT_FOUND) {
        Exception::throwException(&WATER_SOURCE_NOT_FOUND);
    } else {
        waterSource = this->waterSources[waterSourceIndex];
    }
    return waterSource;
}

char* Manager::getWaterSourceName(WaterSource* waterSource) {
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        if (this->waterSources[i] == waterSource) {
            return this->waterSourceNames[i];
        }
    }
    return NULL;
}

char* Manager::getWaterTankName(WaterTank* waterTank) {
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        if (this->waterTanks[i] == waterTank) {
            return this->waterTankNames[i];
        }
    }
    return NULL;
}

char** Manager::getWaterSourceNames() {
    char** list = (char**) malloc(this->totalWaterSources * sizeof(char*));
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        list[i] = this->waterSourceNames[i];
    }
    return list;
}

char** Manager::getWaterTankNames() {
    char** list = (char**) malloc(this->totalWaterTanks * sizeof(char*));
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        list[i] = this->waterTankNames[i];
    }
    return list;
}

unsigned int Manager::getTotalWaterSources() {
    return this->totalWaterSources;
}

unsigned int Manager::getTotalWaterTanks() {
    return this->totalWaterTanks;
}

void Manager::setWaterSourceState(char* name, bool enabled, bool force) {
    WaterSource* waterSource = this->getWaterSource(name);
    if (!Exception::hasException()) {
        if (this->mode == AUTO) {
            return Exception::throwException(&CANNOT_HANDLE_WATER_SOURCE_IN_AUTO);
        }
        if (enabled) {
            waterSource->turnOn(force);
        } else {
            waterSource->turnOff();
        }
    }
}

void Manager::setWaterSourceState(char* name, bool enabled) {
    return this->setWaterSourceState(name, enabled, false);
}

void Manager::registerWaterSource(char* name, WaterSource* waterSource) {
    if (this->isWaterSourceRegistered(name)) {
        return Exception::throwException(&WATER_SOURCE_ALREADY_REGISTERED);
    } else if(this->totalWaterSources + 1 > MAX_WATER_SOURCES) {
        return Exception::throwException(&MAX_WATER_SOURCES_ERROR);
    } else {
        this->totalWaterSources += 1;

        char* waterSourceName = new char[MAX_NAME_LENGTH + 1];
        strncpy(waterSourceName, name, MAX_NAME_LENGTH);
        this->waterSources[this->totalWaterSources - 1] = waterSource;
        this->waterSourceNames[this->totalWaterSources - 1] = waterSourceName;
    }
}

void Manager::registerWaterTank(char* name, WaterTank* waterTank) {
    if (this->isWaterTankRegistered(name)) {
        return Exception::throwException(&WATER_TANK_ALREADY_REGISTERED);
    } else if(this->totalWaterTanks + 1 > MAX_WATER_TANKS) {
        return Exception::throwException(&MAX_WATER_TANKS_ERROR);
    } else {
        this->totalWaterTanks += 1;

        char* waterTankName = new char[MAX_NAME_LENGTH + 1];
        strncpy(waterTankName, name, MAX_NAME_LENGTH);
        this->waterTanks[this->totalWaterTanks - 1] = waterTank;
        this->waterTankNames[this->totalWaterTanks - 1] = waterTankName;
    }
}

WaterSource* Manager::unregisterWaterSource(char* name) {
    WaterSource* waterSource = NULL;

    if (!this->isWaterSourceRegistered(name)) {
        Exception::throwException(&WATER_SOURCE_NOT_FOUND);
    } else if (this->isWaterSourceDependency(name)) {
        Exception::throwException(&CANNOT_REMOVE_WATER_SOURCE_DEPENDENCY);
    } else {
        int waterSourceIndex = this->getWaterSourceIndex(name);

        waterSource = this->waterSources[waterSourceIndex];
        char* waterSourceName = this->waterSourceNames[waterSourceIndex];

        for (unsigned int i = waterSourceIndex + 1; i < this->totalWaterSources; i++) {
            this->waterSources[i - 1] = this->waterSources[i];
            this->waterSourceNames[i - 1] = this->waterSourceNames[i];
        }

        this->totalWaterSources -= 1;

        delete[] waterSourceName;

        unsigned int pin = waterSource->getPin();
        if (!this->isIOInterfaceDependency(pin)) {
            IOInterface::remove(pin);
        }
    }
    return waterSource;
}

WaterTank* Manager::unregisterWaterTank(char* name) {
    WaterTank* waterTank = NULL;

    if (!this->isWaterTankRegistered(name)) {
        Exception::throwException(&WATER_TANK_NOT_FOUND);
    } else if (this->isWaterTankDependency(name)) {
        Exception::throwException(&CANNOT_REMOVE_WATER_TANK_DEPENDENCY);
    } else {
        int waterTankIndex = this->getWaterTankIndex(name);

        waterTank = this->waterTanks[waterTankIndex];
        char* waterTankName = this->waterTankNames[waterTankIndex];

        for (unsigned int i = waterTankIndex + 1; i < this->totalWaterTanks; i++) {
            this->waterTanks[i - 1] = this->waterTanks[i];
            this->waterTankNames[i - 1] = this->waterTankNames[i];
        }

        this->totalWaterTanks -= 1;

        delete[] waterTankName;

        unsigned int pin = waterTank->getPressureSensorPin();
        if (!this->isIOInterfaceDependency(pin)) {
            IOInterface::remove(pin);
        }
    }
    return waterTank;
}

bool Manager::isWaterSourceRegistered(char* name) {
    return this->getWaterSourceIndex(name) != ITEM_NOT_FOUND;
}

bool Manager::isWaterTankRegistered(char* name) {
    return this->getWaterTankIndex(name) != ITEM_NOT_FOUND;
}

bool Manager::isWaterSourceDependency(char* name) {
    int waterSourceIndex = this->getWaterSourceIndex(name);
    if (waterSourceIndex != ITEM_NOT_FOUND) {
        WaterSource* waterSource = this->waterSources[waterSourceIndex];
        for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
            if (this->waterTanks[i]->getWaterSource() == waterSource) {
                return true;
            }
        }
    }
    return false;
}

bool Manager::isWaterTankDependency(char* name) {
    int waterTankIndex = this->getWaterTankIndex(name);
    if (waterTankIndex != ITEM_NOT_FOUND) {
        WaterTank* waterTank = this->waterTanks[waterTankIndex];
        for (unsigned int i = 0; i < this->totalWaterSources; i++) {
            if (this->waterSources[i]->getWaterTank() == waterTank) {
                return true;
            }
        }
    }
    return false;
}

bool Manager::isIOInterfaceDependency(unsigned int pin) {
    IOInterface* io = IOInterface::get(pin);
    if (io == NULL) {
        return false;
    }
    return this->isIOInterfaceDependency(io); 
}

bool Manager::isIOInterfaceDependency(IOInterface* io) {
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        if (this->waterSources[i]->getPin() == io->getPin()) {
            return true;
        }
    }
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        if (this->waterTanks[i]->getPressureSensorPin() == io->getPin()) {
            return true;
        }
    }
    return false;
}

void Manager::fillWaterTank(char* name, bool force) {
    if (this->mode == AUTO) {
        return Exception::throwException(&CANNOT_HANDLE_WATER_TANK_IN_AUTO);
    }
    WaterTank* waterTank = this->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->fill(force);
    }
}

void Manager::stopFillingWaterTank(char* name) {
    if (this->mode == AUTO) {
        return Exception::throwException(&CANNOT_HANDLE_WATER_TANK_IN_AUTO);
    }
    WaterTank* waterTank = this->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->stopFilling();
    }
}

void Manager::loop() {
    if (this->mode == AUTO) {
        for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
            this->waterTanks[i]->loop();
            this->waterTanksLoopErrors[i] = Exception::popException();
        }
        if (this->totalWaterTanks > 0 && this->waterTanksErrorsTimer->getElapsedTime() >= ERROR_INTERVAL) {
            const Exception* error = NULL;
            char* waterTankName = this->waterTankNames[this->waterTankErrorIndex];
            int endIndex = max(0, this->waterTankErrorIndex - 1);
            do {
                error = this->waterTanksLoopErrors[this->waterTankErrorIndex];
                waterTankName = this->waterTankNames[this->waterTankErrorIndex];
                this->waterTankErrorIndex = (this->waterTankErrorIndex + 1) % this->totalWaterTanks;
            } while(error == NULL && this->waterTankErrorIndex != endIndex);
            if (error != NULL) {
                Exception::throwException(error, waterTankName);
            }
            this->waterTanksErrorsTimer->startTimer();
        }
    }
}

int Manager::getWaterTankIndex(char* name) {
    for (unsigned int i = 0; i < this->totalWaterTanks; i++) {
        if (strncmp(this->waterTankNames[i], name, MAX_NAME_LENGTH) == 0) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}

int Manager::getWaterSourceIndex(char* name) {
    for (unsigned int i = 0; i < this->totalWaterSources; i++) {
        if (strncmp(this->waterSourceNames[i], name, MAX_NAME_LENGTH) == 0) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}
