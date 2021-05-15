#include <Arduino.h>

#include "PressureReader.h"

PressureReader::PressureReader(IOInterface* io, float pressureFactor) {
    this->io = io;
    this->pressureFactor = pressureFactor;
}

float PressureReader::getValue() {
	return this->io->read() * this->pressureFactor;
}
