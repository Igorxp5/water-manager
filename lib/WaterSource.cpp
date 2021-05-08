#include <Arduino.h>

#include "WaterSource.h"
#include "Exception.h"

WaterSource::WaterSource(unsigned int pin) {
    this->pin = pin;
    pinMode(pin, OUTPUT);
}

WaterSource::WaterSource(unsigned int pin, WaterTank* waterTank) {
    this->pin = pin;
    this->waterTank = waterTank;
    pinMode(pin, OUTPUT);
}

void WaterSource::enable(bool force=false) {
    if (!force && this->waterTank != NULL && this->waterTank->getVolume() <= this->waterTank->minimumVolume) {
        throw CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
    }
    digitalWrite(this->pin, HIGH);
}

void WaterSource::disable() {
    digitalWrite(this->pin, LOW);
}
