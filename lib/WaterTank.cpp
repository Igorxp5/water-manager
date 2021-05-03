#include <Arduino.h>

#include "WaterTank.h"
#include "Exception.h"

WaterTank::WaterTank(VolumeReader* volumeReader, WaterSource* waterSource=NULL) {
    this->volumeReader = volumeReader;
    this->waterSource = waterSource;
}

double WaterTank::getVolume() {
    return this->volumeReader->getValue();
}

double WaterTank::getPressure() {
    return this->volumeReader->getPressureValue();
}

void WaterTank::setZeroVolume(double pressure) {
    this->volumeReader->setZeroValue(pressure);
}

void WaterTank::fill(bool force) {
    if (this->waterSource == NULL) {
        throw CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE;
    }
    this->lastChangingTime = 0;
    this->startFillingTime = millis();
    this->lastLoopTime = this->startFillingTime;
    this->waterSource->enable();
}

void WaterTank::stopFilling() {
    this->waterSource->disable();
}

RuntimeError* WaterTank::loop() {
    if (this->waterSource != NULL && !this->waterSource->isEnabled()) {
        return NULL;
    }

    RuntimeError* error = NULL;
    unsigned long currentTime = millis();

    if (currentTime < this->lastLoopTime) {
        //TODO: Long Overflow (millis has reseted)
        if (this->lastChangingTime == 0) {
            this->lastChangingTime = currentTime + (this->lastChangingTime - this->startFillingTime);
        }
        this->startFillingTime = currentTime;
    }

    double currentPressure = this->getPressure();
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
    this->lastLoopTime = millis();
    
    return error;
}