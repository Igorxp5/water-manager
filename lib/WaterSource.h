#ifndef WATER_SOURCE_H
#define WATER_SOURCE_H

#include "WaterTank.h"

class WaterSource
{
    public:
        WaterTank* waterTank = NULL;

        WaterSource(unsigned int pin);
        WaterSource(unsigned int pin, WaterTank* waterTank);

        void enable(bool force=false);
        void disable();
        bool isEnabled();

    private:
        unsigned int pin;
        bool enabled;
};

#endif
