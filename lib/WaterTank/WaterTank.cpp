#include <Arduino.h>

#include "WaterTank.h"
#include "Exception.h"
#include "Utils.h"

WaterTank::WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor, WaterSource* waterSource) {
    this->pressureSensor = pressureSensor;
    this->volumeFactor = volumeFactor;
    this->pressureFactor = pressureFactor;
    this->waterSource = waterSource;
    this->zeroVolumePressure = 0;

    this->fillingTimer = new Clock();
    this->pressureChangingTimer = new Clock();
    this->fillingCallsProtectionTimer = new Clock();

    this->fillingCallsProtectionTimer->startTimer();
}

WaterTank::WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor) : WaterTank(pressureSensor, volumeFactor, pressureFactor, NULL){

}

WaterTank::~WaterTank() {
    delete this->fillingTimer;
    delete this->pressureChangingTimer;
    delete this->fillingCallsProtectionTimer;
}

float WaterTank::getVolume() {
    return max(0, (this->getPressure() * volumeFactor) - this->zeroVolumePressure);
}

float WaterTank::getPressure() {
    return this->pressureSensor->read() * pressureFactor;
}

unsigned int WaterTank::getPressureRawValue() {
    return this->pressureSensor->read();
}

unsigned int WaterTank::getPressureSensorPin() {
    return this->pressureSensor->getPin();
}

WaterSource* WaterTank::getWaterSource() {
    return this->waterSource;
}

void WaterTank::fill(bool force) {
    if (this->waterSource == NULL) {
        return Exception::throwException(CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE);
    }
    if (!force && this->getVolume() >= this->maxVolume) {
        return Exception::throwException(CANNOT_FILL_WATER_TANK_MAX_VOLUME);
    }
    this->fillingTimer->startTimer();
    this->fillingCallsProtectionTimer->startTimer();
    this->pressureChangingTimer->stopTimer();
    this->waterSource->enable(force);
}

bool WaterTank::isFilling() {
    return this->waterSource->isEnabled();
}

void WaterTank::stopFilling() {
    if (this->waterSource != NULL) {
        this->waterSource->disable();
    }
}

void WaterTank::loop() {
    if (this->waterSource == NULL) {
        return;
    }

    const RuntimeError* error = NULL;

    if (this->waterSource->isEnabled()) {
        float currentPressure = this->getPressure();

        if (abs(this->lastLoopPressure - currentPressure) >= CHANGING_TOLERANCE) {
            this->pressureChangingTimer->startTimer();
        } else {

            if (this->pressureChangingTimer->hasStarted()) {
                if (this->pressureChangingTimer->getElapsedTime() >= CHANGING_INTERVAL) {
                    //Volume/Presure is not changing anymore
                    error = WATER_TANK_HAS_STOPPED_TO_FILL;

                    this->pressureChangingTimer->startTimer();
                }
            } else if (this->fillingTimer->getElapsedTime() >= CHANGING_INTERVAL) {
                //Volume/Pressure didn't change since water tank was ordered to fill
                error = WATER_TANK_IS_NOT_FILLING;
            }
        }
        this->lastLoopPressure = currentPressure;
    }

    if (this->fillingCallsProtectionTimer->getElapsedTime() > FILLING_CALLS_PROTECTION_TIME) {
        if ((!this->waterSource->canEnable() || this->getVolume() >= this->maxVolume) && this->waterSource->isEnabled()) {
            this->waterSource->disable();
        } else if ((this->waterSource->canEnable() && this->getVolume() <= this->minimumVolume) && !this->waterSource->isEnabled()) {
            this->fill(false);
        }
        this->fillingCallsProtectionTimer->startTimer();
    }

    Exception::throwException(error);
}

WaterSource::WaterSource(IOInterface* io) : WaterSource(io, NULL) {

}

WaterSource::WaterSource(IOInterface* io, WaterTank* waterTank) {
    this->io = io;
    this->waterTank = waterTank;
}

void WaterSource::enable(bool force) {
    if (!force && !this->canEnable()) {
        Exception::throwException(CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME);
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

WaterTank* WaterSource::getWaterTank() {
    return this->waterTank;
}

bool WaterSource::canEnable() {
    return this->waterTank == NULL || this->waterTank->getVolume() > this->waterTank->minimumVolume;
}
