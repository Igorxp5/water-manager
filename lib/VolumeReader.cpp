#include <Arduino.h>

#include "VolumeReader.h"

VolumeReader::VolumeReader(IOInterface* io, double pressureFactor, double volumeFactor) : PressureReader(io, pressureFactor) {
    this->volumeFactor = volumeFactor;
}

double VolumeReader::getValue() {
	return PressureReader::getValue() * this->volumeFactor;
}

double VolumeReader::getPressureValue() {
	return PressureReader::getValue();
}
