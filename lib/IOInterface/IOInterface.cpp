#include "IOInterface.h"

#include "Exception.h"

#include <Arduino.h>

int ITEM_NOT_FOUND = -1;

IOInterface** IOInterface::ios = NULL;
unsigned int* IOInterface::ioPins = NULL;
unsigned int IOInterface::totalIos = 0;

#ifdef TEST
IOSource IOInterface::source = VIRTUAL;
#endif

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
		return Exception::throwException(&PIN_NOT_FOUND);
	}
	delete IOInterface::ios[ioIndex];

	for (unsigned int i = ioIndex + 1; i < IOInterface::totalIos; i++) {
		IOInterface::ios[i - 1] = IOInterface::ios[i];
		IOInterface::ioPins[i - 1] = IOInterface::ioPins[i];
	}

	IOInterface::totalIos -= 1;

	IOInterface::ios = (IOInterface**) realloc(IOInterface::ios, IOInterface::totalIos * sizeof(IOInterface*));
	IOInterface::ioPins = (unsigned int*) realloc(IOInterface::ioPins, IOInterface::totalIos * sizeof(unsigned int));
}


void IOInterface::removeAll() {
	for (unsigned int i = 0; i < IOInterface::totalIos; i++) {
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
	if (type == ANALOGIC) {
		#ifndef TEST
		return analogRead(this->pin);
		#else
		if (IOInterface::source == PHYSICAL) {
			return analogRead(this->pin);
		} else {
			return this->value;
		}
		#endif
	} else if (type == DIGITAL) {
		#ifndef TEST
		return (unsigned int) digitalRead(this->pin);
		#else
		if (IOInterface::source == PHYSICAL) {
			return (unsigned int) digitalRead(this->pin);
		} else {
			return this->value;
		}
		#endif
	}
	return 0;
}

void IOInterface::write(unsigned int value) {
	if (type == ANALOGIC) {
		#ifndef TEST
		analogWrite(this->pin, value);
		#else
		if (IOInterface::source == PHYSICAL) {
			analogWrite(this->pin, value);
		} else {
			this->value = value;
		}
		#endif
	} else if (type == DIGITAL) {
		#ifndef TEST
		digitalWrite(this->pin, value);
		#else
		if (IOInterface::source == PHYSICAL) {
			digitalWrite(this->pin, value);
		} else {
			this->value = value;
		}
		#endif
	}
}


unsigned int IOInterface::getPin() {
	return this->pin;
}
