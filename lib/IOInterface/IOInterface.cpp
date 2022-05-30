#include "IOInterface.h"

#include <Arduino.h>
#include "Utils.h"

int ITEM_NOT_FOUND = -1;

IOInterface** IOInterface::ios = NULL;
unsigned int* IOInterface::ioPins = NULL;
unsigned int IOInterface::totalIos = 0;

IOInterface::IOInterface(unsigned int pin, IOMode mode, IOType type) {
    this->pin = pin;
    this->mode = mode;
	this->type = type;

	#ifndef TEST
	pinMode(pin, (mode == READ_ONLY) ? INPUT : OUTPUT);
	#endif

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

unsigned int IOInterface::read() {
	#ifndef TEST
	if (type == ANALOGIC) {
		return analogRead(this->pin);
	} else if (type == DIGITAL) {
		return (unsigned int) digitalRead(this->pin);
	}
	#else
	return this->value;
	#endif
}

void IOInterface::write(unsigned int value) {
	#ifndef TEST
	if (type == ANALOGIC) {
		analogWrite(this->pin, value);
	} else if (type == DIGITAL) {
		digitalWrite(this->pin, value);
	}
	#else
	this->value = value;
	#endif
}


unsigned int IOInterface::getPin() {
	return this->pin;
}
