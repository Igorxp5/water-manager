#include "IOInterface.h"

#include <Arduino.h>
#include "Utils.h"

int ITEM_NOT_FOUND = -1;

IOInterface** IOInterface::ios = NULL;
unsigned int* IOInterface::ioPins = NULL;
unsigned int IOInterface::totalIos = 0;

IOInterface::IOInterface(unsigned int pin, IOMode mode) {
    this->pin = pin;
    this->mode = mode;

	int ioIndex = IOInterface::getIndex(pin);
	if (ioIndex != ITEM_NOT_FOUND) {
		delete IOInterface::ios[ioIndex];
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
    int ioIndex = IOInterface::getIndex(pin);
	if (ioIndex != ITEM_NOT_FOUND) {
		return IOInterface::ios[ioIndex];
	}
	return NULL;
}

void IOInterface::remove(unsigned int pin) {
    int ioIndex = IOInterface::getIndex(pin);
	if (ioIndex == ITEM_NOT_FOUND) {
		//throw PIN_NOT_FOUND
	} else {
		delete IOInterface::ios[ioIndex];

		for (unsigned int i = ioIndex + 1; i < IOInterface::totalIos; i++) {
			IOInterface::ios[i - 1] = IOInterface::ios[i];
			IOInterface::ioPins[i - 1] = IOInterface::ioPins[i];
		}

		IOInterface::totalIos -= 1;

		IOInterface::ios = (IOInterface**) realloc(IOInterface::ios, IOInterface::totalIos * sizeof(IOInterface*));
		IOInterface::ioPins = (unsigned int*) realloc(IOInterface::ioPins, IOInterface::totalIos * sizeof(unsigned int));
	}

}

void IOInterface::removeAll() {
	while (IOInterface::totalIos > 0) {
		IOInterface::remove(IOInterface::ioPins[0]);
	}
}

int IOInterface::getIndex(unsigned int pin) {
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

unsigned int AnalogicIO::getPin() {
	return this->pin;
}

DigitalIO::DigitalIO(unsigned int pin, IOMode mode) : IOInterface(pin, mode) {
	pinMode(pin, (mode == READ_ONLY) ? INPUT : OUTPUT);
}

unsigned int DigitalIO::read() {
	return (unsigned int) digitalRead(this->pin);
}

unsigned int DigitalIO::getPin() {
	return this->pin;
}

void DigitalIO::write(unsigned int w) {
	return digitalWrite(this->pin, w);
}

#ifdef TEST
TestIO::TestIO(unsigned int pin) : IOInterface(pin, READ_WRITE) {

}

unsigned int TestIO::read() {
	return this->value;
}

unsigned int TestIO::getPin() {
	return this->pin;
}

void TestIO::write(unsigned int w) {
    this->value = w;
}
#endif
