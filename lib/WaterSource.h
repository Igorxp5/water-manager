#ifndef WATER_SOURCE_H
#define WATER_SOURCE_H

#include "WaterTank.h"
#include "IOInterface.h"

class WaterSource
{
    public:
        WaterTank* waterTank = NULL;

        WaterSource(IOInterface* io);
        WaterSource(IOInterface* io, WaterTank* waterTank);

        void enable(bool force=false);
        void disable();
        bool isEnabled();

    private:
        IOInterface* io;
        bool enabled;
};

#endif
