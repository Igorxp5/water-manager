#include <Arduino.h>

#include "WaterSource.h"

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
    digitalWrite(this->pin, HIGH);
}

void WaterSource::disable() {
    digitalWrite(this->pin, LOW);
}
