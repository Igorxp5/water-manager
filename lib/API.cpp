#include "API.h"

#include "WaterTank.h"
#include "VolumeReader.h"
#include "OperationMode.h"
#include "IOInterface.h"

const long ITEM_NOT_FOUND = -1;

API::API() {
    this->manager = new Manager();
}

void API::createWaterSource(String name, short pin) {
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterSource* waterSource = new WaterSource(io);
    this->manager->registerWaterSource(name, waterSource);
}

void API::createWaterSource(String name, short pin, String waterTankName) {
    IOInterface* io = this->getOrCreateIO(pin, DIGITAL, READ_ONLY);
    WaterTank* waterTank = this->manager->getWaterTank(waterTankName);
    WaterSource* waterSource = new WaterSource(io, waterTank);
    this->manager->registerWaterSource(name, waterSource);
}

void API::createWaterTank(String name, short volumeReaderPin, double volumeFactor, double pressureFactor) {
    IOInterface* io = this->getOrCreateIO(volumeReaderPin, ANALOGIC, READ_ONLY);
    VolumeReader* volumeReader = new VolumeReader(io, pressureFactor, volumeFactor); 
    WaterTank* waterTank = new WaterTank(volumeReader);

    this->manager->registerWaterTank(name, waterTank);
}

void API::createWaterTank(String name, short volumeReaderPin, double volumeFactor, double pressureFactor, String waterSourceName) {
    IOInterface* io = this->getOrCreateIO(volumeReaderPin, ANALOGIC, READ_ONLY);
    WaterSource* waterSource = this->getWaterSource(waterSourceName);

    VolumeReader* volumeReader = new VolumeReader(io, pressureFactor, volumeFactor); 
    WaterTank* waterTank = new WaterTank(volumeReader, waterSource);

    this->manager->registerWaterTank(name, waterTank);
}

void API::setWaterTankMinimumVolume(String name, double minimum) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    waterTank->minimumVolume = minimum;
}

void API::setWaterTankMaxVolume(String name, double max) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    waterTank->maxVolume = max;
}

void API::setWaterZeroVolume(String name, double pressure) {
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

void API::enableWaterSource(String name) {
    this->manager->setWaterSourceState(name, true);
}

void API::disableWaterSource(String name) {
    this->manager->setWaterSourceState(name, false);
}

unsigned int API::getWaterSourceList(String* list) {
    return this->manager->getWaterSourceNames(list);
}

unsigned int API::getWaterTankList(String* list) {
    return this->manager->getWaterTankNames(list);
}

double API::getWaterTankVolume(String name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    return waterTank->getVolume();
}

double API::getWaterTankPressure(String name) {
    WaterTank* waterTank = this->manager->getWaterTank(name);
    return waterTank->getPressure();
}

bool API::getWaterSourceState(String name) {
    WaterSource* waterSource = this->manager->getWaterSource(name);
    return waterSource->isEnabled();
}

void API::removeWaterSource(String name) {
    this->manager->unregisterWaterSource(name);
}

void API::removeWaterTank(String name) {
    this->manager->unregisterWaterTank(name);
}

void API::reset() {
    delete this->manager;
    this->manager = new Manager();
}

unsigned int API::getError(RuntimeError** list) {
    return this->manager->getErrors(list);
}

IOInterface* API::getOrCreateIO(unsigned pin, IOType type, IOMode mode=READ_ONLY) {
    IOInterface* io = IOInterface::get(pin);
    if (io == NULL) {
        if (type == DIGITAL) {
            io = new DigitalIO(pin, mode);
        } else {
            io = new AnalogicIO(pin, mode);
        }
    }
    return io;
}
