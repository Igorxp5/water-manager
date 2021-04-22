#include <Arduino.h>

#include "PressureReader.h"

PressureReader::PressureReader(unsigned int pin, double pressureFactor) {
    this->pin = pin;
    this->pressureFactor = pressureFactor;

    pinMode(pin, INPUT);
}

double PressureReader::getValue() {
	return analogRead(this->pin) * this->pressureFactor;
}
