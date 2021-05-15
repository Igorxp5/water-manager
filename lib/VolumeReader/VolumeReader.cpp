#include <Arduino.h>

#include "VolumeReader.h"

VolumeReader::VolumeReader(IOInterface* io, float pressureFactor, float volumeFactor) : PressureReader(io, pressureFactor) {
    this->volumeFactor = volumeFactor;
}

float VolumeReader::getValue() {
	return PressureReader::getValue() * this->volumeFactor;
}

float VolumeReader::getPressureValue() {
	return PressureReader::getValue();
}
