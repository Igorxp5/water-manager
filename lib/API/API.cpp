#include "API.h"

#include "WaterTank.h"
#include "OperationMode.h"
#include "IOInterface.h"

API::API() {
    this->manager = new Manager();
}

void API::createWaterSource(char* name, short pin) {
    if (this->manager->isWaterSourceRegistered(name)) {
        return Exception::throwException(&WATER_SOURCE_ALREADY_REGISTERED);
    }
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterSource* waterSource = new WaterSource(io);
    this->manager->registerWaterSource(name, waterSource);
}

void API::createWaterSource(char* name, short pin, char* waterTankName) {
    if (this->manager->isWaterSourceRegistered(name)) {
        return Exception::throwException(&WATER_SOURCE_ALREADY_REGISTERED);
    } else if (!this->manager->isWaterTankRegistered(waterTankName)) {
        return Exception::throwException(&WATER_TANK_NOT_FOUND);
    }
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterTank* waterTank = this->manager->getWaterTank(waterTankName);
    WaterSource* waterSource = new WaterSource(io, waterTank);
    this->manager->registerWaterSource(name, waterSource);
}

void API::createWaterTank(char* name, short pressureSensorPin, float volumeFactor, float pressureFactor, float pressureChangingValue) {
    if (this->manager->isWaterTankRegistered(name)) {
        return Exception::throwException(&WATER_TANK_ALREADY_REGISTERED);
    }
    IOInterface* pressureSensor = this->getOrCreateIO(pressureSensorPin, ANALOGIC, READ_ONLY);
    WaterTank* waterTank = new WaterTank(pressureSensor, volumeFactor, pressureFactor);
    waterTank->pressureChangingValue = pressureChangingValue;
    this->manager->registerWaterTank(name, waterTank);
}

void API::createWaterTank(char* name, short pressureSensorPin, float volumeFactor, float pressureFactor, float pressureChangingValue, char* waterSourceName) {
    if (this->manager->isWaterTankRegistered(name)) {
        return Exception::throwException(&WATER_TANK_ALREADY_REGISTERED);
    } else if (!this->manager->isWaterSourceRegistered(waterSourceName)) {
        return Exception::throwException(&WATER_SOURCE_NOT_FOUND);
    }
    IOInterface* pressureSensor = this->getOrCreateIO(pressureSensorPin, ANALOGIC, READ_ONLY);
    WaterSource* waterSource = this->getWaterSource(waterSourceName);

    WaterTank* waterTank = new WaterTank(pressureSensor, volumeFactor, pressureFactor, waterSource);
    waterTank->pressureChangingValue = pressureChangingValue;
    this->manager->registerWaterTank(name, waterTank);
}

void API::setWaterTankMinimumVolume(char* name, float minimum) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->minimumVolume = minimum;
    }
}

void API::setWaterTankMaxVolume(char* name, float max) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->maxVolume = max;
    }
}

void API::setWaterZeroVolume(char* name, float pressure) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->zeroVolumePressure = pressure;
    }
}

void API::setWaterTankVolumeFactor(char* name, float volumeFactor) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->volumeFactor = volumeFactor;
    }
}

void API::setWaterTankPressureFactor(char* name, float pressureFactor) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->pressureFactor = pressureFactor;
    }
}

void API::setWaterTankPressureChangingValue(char* name, float pressureChangingValue) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->pressureChangingValue = pressureChangingValue;
    }
}

void API::setWaterTankActive(char* name, bool active) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        waterTank->setActive(active);
    }
}

void API::setOperationMode(byte mode) {
    if (mode == 0) {
        this->manager->setOperationMode(MANUAL);
    } else if (mode == 1) {
        this->manager->setOperationMode(AUTO);
    } else {
        Exception::throwException(&INVALID_OPERATION_MODE);
    }
}

byte API::getOperationMode() {
    return (byte) this->manager->getOperationMode();
}

void API::setWaterSourceState(char* name, bool enabled, bool force) {
    this->manager->setWaterSourceState(name, enabled, force);
}

void API::setWaterSourceActive(char* name, bool active) {
    WaterSource* waterSource = this->manager->getWaterSource(name);
    if (waterSource != NULL) {
        waterSource->setActive(active);
    }
}

char** API::getWaterSourceList() {
    return this->manager->getWaterSourceNames();
}

char** API::getWaterTankList() {
    return this->manager->getWaterTankNames();
}

float API::getWaterTankVolume(char* name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        return waterTank->getVolume();
    }
    return 0;
}

float API::getWaterTankPressure(char* name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    if (waterTank != NULL) {
        return waterTank->getPressure();
    }
    return 0;
}

WaterSource* API::getWaterSource(char* name) {
    return this->manager->getWaterSource(name);
}

WaterTank* API::getWaterTank(char* name) {
    return this->manager->getWaterTank(name);
}

char* API::getWaterSourceName(WaterSource* waterSource) {
    return this->manager->getWaterSourceName(waterSource);
}

char* API::getWaterTankName(WaterTank* waterTank) {
    return this->manager->getWaterTankName(waterTank);
}

unsigned int API::getTotalWaterSources() {
    return this->manager->getTotalWaterSources();
}

unsigned int API::getTotalWaterTanks() {
    return this->manager->getTotalWaterTanks();
}

void API::removeWaterSource(char* name) {
    WaterSource* waterSource = this->manager->unregisterWaterSource(name);
    if (waterSource != NULL) {
        delete waterSource;
    }
}

void API::removeWaterTank(char* name) {
    WaterTank* waterTank = this->manager->unregisterWaterTank(name);
    if (waterTank != NULL) {
        delete waterTank;
    }
}

void API::fillWaterTank(char* name, bool enabled, bool force) {
    if (enabled) {
        this->manager->fillWaterTank(name, force);
    } else {
        this->manager->stopFillingWaterTank(name);
    }
}

void API::reset() {
    delete this->manager;
    this->manager = new Manager();
}

void API::loop() {
    this->manager->loop();
}

IOInterface* API::getOrCreateIO(unsigned pin, IOType type, IOMode mode) {
    IOInterface* io = IOInterface::get(pin);
    if (io == NULL) {
        io = new IOInterface(pin, mode, type);
    }
    return io;
}
