#ifndef PRESSURE_READER_H
#define PRESSURE_READER_H

#include "IOInterface.h"

class PressureReader
{
    public:
        PressureReader(IOInterface* io, float pressureFactor);

        virtual float getValue();

    protected:
        IOInterface* io;
        float pressureFactor;
};

#endif
