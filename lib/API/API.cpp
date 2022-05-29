#include "API.h"

#include "WaterTank.h"
#include "VolumeReader.h"
#include "OperationMode.h"
#include "IOInterface.h"
#include "Utils.h"

API::API() {
    this->manager = new Manager();
}

void API::createWaterSource(char* name, short pin) {
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterSource* waterSource = new WaterSource(io);
    this->manager->registerWaterSource(name, waterSource);
    if (Exception::hasException()) {
        delete waterSource;
        IOInterface::remove(pin);
    }
}

void API::createWaterSource(char* name, short pin, String waterTankName) {
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterTank* waterTank = this->manager->getWaterTank(waterTankName);
    WaterSource* waterSource = new WaterSource(io, waterTank);
    this->manager->registerWaterSource(name, waterSource);
}

void API::createWaterTank(String name, short volumeReaderPin, float volumeFactor, float pressureFactor) {
    IOInterface* io = this->getOrCreateIO(volumeReaderPin, ANALOGIC, READ_ONLY);
    VolumeReader* volumeReader = new VolumeReader(io, pressureFactor, volumeFactor); 
    WaterTank* waterTank = new WaterTank(volumeReader);

    this->manager->registerWaterTank(name, waterTank);
}

void API::createWaterTank(String name, short volumeReaderPin, float volumeFactor, float pressureFactor, String waterSourceName) {
    IOInterface* io = this->getOrCreateIO(volumeReaderPin, ANALOGIC, READ_ONLY);
    WaterSource* waterSource = this->getWaterSource(waterSourceName);

    VolumeReader* volumeReader = new VolumeReader(io, pressureFactor, volumeFactor); 
    WaterTank* waterTank = new WaterTank(volumeReader, waterSource);

    this->manager->registerWaterTank(name, waterTank);
}

void API::setWaterTankMinimumVolume(String name, float minimum) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    waterTank->minimumVolume = minimum;
}

void API::setWaterTankMaxVolume(String name, float max) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    waterTank->maxVolume = max;
}

void API::setWaterZeroVolume(String name, float pressure) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    waterTank->setZeroVolume(pressure);
}

void API::setAutomaticMode() {
    OperationMode mode = AUTOMATIC;
    this->manager->setOperationMode(mode);
}

void API::setManualMode() {
    OperationMode mode = MANUAL;
    this->manager->setOperationMode(mode);
}

void API::enableWaterSource(char* name) {
    this->manager->setWaterSourceState(name, true);
}

void API::disableWaterSource(char* name) {
    this->manager->setWaterSourceState(name, false);
}

char** API::getWaterSourceList() {
    return this->manager->getWaterSourceNames();
}

unsigned int API::getWaterTankList(String* list) {
    return this->manager->getWaterTankNames(list);
}

float API::getWaterTankVolume(String name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    return waterTank->getVolume();
}

float API::getWaterTankPressure(String name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    return waterTank->getPressure();
}

WaterSource* API::getWaterSource(char* name) {
    return this->manager->getWaterSource(name);
}

unsigned int API::getTotalWaterSources() {
    return this->manager->getTotalWaterSources();
}

void API::removeWaterSource(char* name) {
    WaterSource* waterSource = this->manager->unregisterWaterSource(name);
    if (waterSource != NULL) {
        delete waterSource;
    }
}

void API::removeWaterTank(String name) {
    WaterTank* waterTank = this->manager->unregisterWaterTank(name);
    delete waterTank;
}

void API::reset() {
    this->manager->reset();
    IOInterface::removeAll();
}

void API::loop() {
    this->manager->loop();
}

IOInterface* API::getOrCreateIO(unsigned pin, IOType type, IOMode mode) {
    IOInterface* io = IOInterface::get(pin);
    if (io == NULL) {
        #ifdef TEST
            io = new TestIO(pin);
        #else
        if (type == DIGITAL) {
            io = new DigitalIO(pin, mode);
        } else {
            io = new AnalogicIO(pin, mode);
        }
        #endif
    }
    return io;
}
