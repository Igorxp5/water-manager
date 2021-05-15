#include "IOInterface.h"

#include <Arduino.h>

unsigned long ITEM_NOT_FOUND = -1;

IOInterface** IOInterface::ios = NULL;
unsigned int* IOInterface::ioPins = 0;
unsigned int IOInterface::totalIos = 0;

IOInterface::IOInterface(unsigned int pin, IOMode mode) {
    this->pin = pin;
    this->mode = mode;

	unsigned int ioIndex = IOInterface::getIndex(pin);
	if (ioIndex == ITEM_NOT_FOUND) {
		IOInterface::ios[ioIndex] = this;
	} else {
		IOInterface::totalIos += 1;
		IOInterface::ios = (IOInterface**) realloc(IOInterface::ios, IOInterface::totalIos * sizeof(IOInterface*));
		IOInterface::ioPins = (unsigned int*) realloc(IOInterface::ioPins, IOInterface::totalIos * sizeof(unsigned int));
		IOInterface::ios[IOInterface::totalIos - 1] = this;
		IOInterface::ioPins[IOInterface::totalIos - 1] = pin;
	}
}

IOInterface* IOInterface::get(unsigned int pin) {
    unsigned int ioIndex = IOInterface::getIndex(pin);
	if (ioIndex != ITEM_NOT_FOUND) {
		return IOInterface::ios[ioIndex];
	}
	return NULL;
}

unsigned long IOInterface::getIndex(unsigned int pin) {
	for (unsigned int i = 0; i < IOInterface::totalIos; i++) {
        if (IOInterface::ioPins[i] == pin) {
            return i;
        }
    }
    return ITEM_NOT_FOUND;
}

AnalogicIO::AnalogicIO(unsigned int pin, IOMode mode) : IOInterface(pin, mode) {
	pinMode(pin, (mode == READ_ONLY) ? INPUT : OUTPUT);
}

unsigned int AnalogicIO::read() {
	return analogRead(this->pin);
}

DigitalIO::DigitalIO(unsigned int pin, IOMode mode) : IOInterface(pin, mode) {
	pinMode(pin, (mode == READ_ONLY) ? INPUT : OUTPUT);
}

unsigned int DigitalIO::read() {
	return (unsigned int) digitalRead(this->pin);
}

void DigitalIO::write(unsigned int w) {
	return digitalWrite(this->pin, w);
}

#ifdef TEST
unsigned int TestIO::read() {
	return this->value;
}

void TestIO::write(unsigned int w) {
    this->value = w;
}
#endif
