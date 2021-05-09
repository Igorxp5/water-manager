#include <Arduino.h>

#include "WaterSource.h"
#include "Exception.h"

WaterSource::WaterSource(IOInterface* io) {
    this->io = io;
}

WaterSource::WaterSource(IOInterface* io, WaterTank* waterTank) {
    this->io = io;
    this->waterTank = waterTank;
}

void WaterSource::enable(bool force=false) {
    if (!force && this->waterTank != NULL && this->waterTank->getVolume() <= this->waterTank->minimumVolume) {
        throw CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
    }
    this->io->write(HIGH);
}

void WaterSource::disable() {
    this->io->write(LOW);
}
