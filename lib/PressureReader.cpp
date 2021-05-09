#include <Arduino.h>

#include "PressureReader.h"

PressureReader::PressureReader(IOInterface* io, double pressureFactor) {
    this->io = io;
    this->pressureFactor = pressureFactor;
}

double PressureReader::getValue() {
	return this->io->read() * this->pressureFactor;
}
