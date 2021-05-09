#ifndef PRESSURE_READER_H
#define PRESSURE_READER_H

#include "IOInterface.h"

class PressureReader
{
    public:
        PressureReader(IOInterface* io, double pressureFactor);

        virtual double getValue();

    protected:
        IOInterface* io;
        double pressureFactor;
};

#endif
