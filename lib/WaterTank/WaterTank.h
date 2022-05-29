#ifndef WATER_TANK_H
#define WATER_TANK_H

#include "VolumeReader.h"
#include "IOInterface.h"
#include "Exception.h"

const float UNDEFINED_VOLUME = -1;
const unsigned long CHANGING_INTERVAL = 300000UL; // 5 minutes
const float CHANGING_TOLERANCE = 0.01;

class WaterSource;

class WaterTank
{
    public:
        float minimumVolume = 0;
        float maxVolume = 0;
        WaterSource* waterSource = NULL;

        WaterTank(VolumeReader* volumeReader, WaterSource* waterSource=NULL);

        float getVolume();
        float getPressure();
        void setZeroVolume(float pressure);
        void fill(bool force);
        void stopFilling();
        const RuntimeError* loop();

    protected:
        VolumeReader* volumeReader;
    
    private:
        unsigned long startFillingTime;
        unsigned long lastChangingTime;
        float lastLoopPressure;
        unsigned long lastLoopTime;
};

class WaterSource
{
    public:
        WaterTank* waterTank = NULL;

        WaterSource(IOInterface* io);
        WaterSource(IOInterface* io, WaterTank* waterTank);

        void enable(bool force=false);
        void disable();
        bool isEnabled();
        unsigned int getPin();

    private:
        IOInterface* io;
};

#endif
