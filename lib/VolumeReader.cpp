#include <Arduino.h>

#include "VolumeReader.h"

VolumeReader::VolumeReader(unsigned int pin, double pressureFactor, double volumeFactor) : PressureReader(pin, pressureFactor) {
    this->volumeFactor = volumeFactor;
}

double VolumeReader::getValue() {
	return PressureReader::getValue() * this->volumeFactor;
}

double VolumeReader::getPressureValue() {
	return PressureReader::getValue();
}
