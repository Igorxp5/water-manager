#include <Arduino.h>

#include "WaterTank.h"
#include "Exception.h"

WaterTank::WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor, WaterSource* waterSource) {
    this->pressureSensor = pressureSensor;
    this->volumeFactor = volumeFactor;
    this->pressureFactor = pressureFactor;
    this->waterSource = waterSource;
    this->zeroVolumePressure = 0;
    this->active = true;
    this->error = NULL;

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

bool WaterTank::canFill() {
    return this->waterSource != NULL && this->waterSource->canEnable() && this->active && this->getVolume() < this->maxVolume;
}

bool WaterTank::isActive() {
    return this->active;
}

void WaterTank::fill(bool force) {
    if (this->waterSource == NULL) {
        return Exception::throwException(&CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE);
    }
    if (!force && !this->active) {
        return Exception::throwException(&CANNOT_FILL_DEACTIVATED_WATER_TANK);
    }
    if (!force && this->getVolume() >= this->maxVolume) {
        return Exception::throwException(&CANNOT_FILL_WATER_TANK_MAX_VOLUME);
    }
    this->active = true;
    this->fillingTimer->startTimer();
    this->fillingCallsProtectionTimer->startTimer();
    this->pressureChangingTimer->stopTimer();
    this->waterSource->turnOn(force);
    this->lastLoopPressure = this->getPressure();
}

bool WaterTank::isFilling() {
    return this->waterSource->isTurnedOn();
}

void WaterTank::stopFilling() {
    if (this->waterSource != NULL) {
        this->waterSource->turnOff();
    }
}

void WaterTank::setActive(bool active) {
    this->active = active;
    if (!active) {
        this->stopFilling();
    }
}
 
void WaterTank::loop() {
    if (this->waterSource == NULL) {
        return;
    }

    if (this->active && this->waterSource->isTurnedOn()) {
        this->error = NULL;

        float currentPressure = this->getPressure();

        if (abs(this->lastLoopPressure - currentPressure) >= this->pressureChangingValue) {
            this->pressureChangingTimer->startTimer();
        } else {

            if (this->pressureChangingTimer->hasStarted()) {
                if (this->pressureChangingTimer->getElapsedTime() >= MAX_TIME_NOT_FILLING) {
                    this->error = &MAX_TIME_WATER_TANK_NOT_FILLING;
                    this->setActive(false);

                } else if (this->pressureChangingTimer->getElapsedTime() >= CHANGING_INTERVAL) {
                    //Volume/Presure is not changing anymore
                    this->error = &WATER_TANK_HAS_STOPPED_TO_FILL;
                }
            } else if (this->fillingTimer->getElapsedTime() >= MAX_TIME_NOT_FILLING) {
                this->error = &MAX_TIME_WATER_TANK_NOT_FILLING;
                this->setActive(false);

            } else if (this->fillingTimer->getElapsedTime() >= CHANGING_INTERVAL) {
                //Volume/Pressure didn't change since water tank was ordered to fill
                this->error = &WATER_TANK_IS_NOT_FILLING;
            }
        }
        this->lastLoopPressure = currentPressure;
    }

    if (this->fillingCallsProtectionTimer->getElapsedTime() > FILLING_CALLS_PROTECTION_TIME) {
        if (!this->canFill() && this->waterSource->isTurnedOn()) {
            this->waterSource->turnOff();
            this->fillingCallsProtectionTimer->startTimer();
        } else if ((this->canFill() && this->getVolume() <= this->minimumVolume) && !this->waterSource->isTurnedOn()) {
            this->fill(false);
            this->fillingCallsProtectionTimer->startTimer();
        }
    }

    if (this->error != NULL) {
        Exception::throwException(this->error);
    }
}

WaterSource::WaterSource(IOInterface* io, WaterTank* waterTank) {
    this->io = io;
    this->waterTank = waterTank;
    this->active = true;
}

WaterSource::WaterSource(IOInterface* io) : WaterSource(io, NULL) {

}

void WaterSource::turnOn(bool force) {
    if (!force && !this->active) {
        return Exception::throwException(&CANNOT_TURN_ON_DEACTIVATED_WATER_SOURCE);
    }
    if (!force && !this->canEnable()) {
        return Exception::throwException(&CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME);
    }
    this->io->write(HIGH);
}

void WaterSource::turnOff() {
    this->io->write(LOW);
}

bool WaterSource::isTurnedOn() {
    return this->io->read() == 1;
}

unsigned int WaterSource::getPin() {
    return this->io->getPin();
}

WaterTank* WaterSource::getWaterTank() {
    return this->waterTank;
}

bool WaterSource::canEnable() {
    return this->active && (this->waterTank == NULL || this->waterTank->getVolume() > this->waterTank->minimumVolume);
}

bool WaterSource::isActive() {
    return this->active;
}

void WaterSource::setActive(bool active) {
    this->active = active;
    if (!active) {
        this->turnOff();
    }
}
