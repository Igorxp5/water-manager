#include <Arduino.h>

#include "WaterTank.h"
#include "Exception.h"
#include "Utils.h"

WaterTank::WaterTank(VolumeReader* volumeReader, WaterSource* waterSource) {
    this->volumeReader = volumeReader;
    this->waterSource = waterSource;
}

float WaterTank::getVolume() {
    return this->volumeReader->getValue();
}

float WaterTank::getPressure() {
    return this->volumeReader->getPressureValue();
}

void WaterTank::setZeroVolume(float pressure) {
    this->volumeReader->setZeroValue(pressure);
}

void WaterTank::fill(bool force) {
    if (this->waterSource == NULL) {
        //throw CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE;
    }
    if (!force && this->getVolume() >= this->maxVolume) {
        //throw CANNOT_FILL_WATER_TANK_MAX_VOLUME;
    }
    this->lastChangingTime = 0;
    this->startFillingTime = millis();
    this->lastLoopTime = this->startFillingTime;
    this->waterSource->enable(force);
}

void WaterTank::stopFilling() {
    this->waterSource->disable();
}

const RuntimeError* WaterTank::loop() {
    if (this->waterSource != NULL && !this->waterSource->isEnabled()) {
        return NULL;
    }

    const RuntimeError* error = NULL;
    unsigned long currentTime = millis();

    if (currentTime < this->lastLoopTime) {
        //Long Overflow (millis has reseted)
        if (this->lastChangingTime == 0) {
            this->lastChangingTime = currentTime + (this->lastChangingTime - this->startFillingTime);
        }
        this->startFillingTime = currentTime;
    }

    float currentPressure = this->getPressure();
    if (abs(this->lastLoopPressure - currentPressure) >= CHANGING_TOLERANCE) {
        this->lastChangingTime = currentTime;
    } else {

        if (this->lastChangingTime != 0) {
            if (currentTime - this->lastChangingTime >= CHANGING_INTERVAL) {
                //Volume/Presure is not changing anymore
                error = WATER_TANK_STOPPED_TO_FILL;
            }
        } else if (currentTime - this->startFillingTime >= CHANGING_INTERVAL) {
            //Volume/Pressure didn't change since water tank was ordered to fill
            error = WATER_TANK_IS_NOT_FILLING;
        }
    }
    this->lastLoopPressure = currentPressure;

    if (this->waterSource != NULL) {
        if (this->getVolume() >= this->maxVolume) {
            this->waterSource->disable();
        } else if (this->getVolume() <= this->minimumVolume) {
            //try {
                this->fill(false);
            //} catch(RuntimeError* fillError) {
                //If the water tank source is empty, do not start filling this water tank
            //}
        }
    }

    this->lastLoopTime = millis();

    return error;
}


WaterSource::WaterSource(IOInterface* io) {
    this->io = io;
}

WaterSource::WaterSource(IOInterface* io, WaterTank* waterTank) {
    this->io = io;
    this->waterTank = waterTank;
}

void WaterSource::enable(bool force) {
    if (!force && this->waterTank != NULL && this->waterTank->getVolume() <= this->waterTank->minimumVolume) {
        //throw CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
    }
    this->io->write(HIGH);
}

void WaterSource::disable() {
    this->io->write(LOW);
}

bool WaterSource::isEnabled() {
    return this->io->read() == 1;
}

unsigned int WaterSource::getPin() {
    return this->io->getPin();
}
