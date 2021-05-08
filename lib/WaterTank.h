#ifndef WATER_TANK_H
#define WATER_TANK_H

#include "VolumeReader.h"
#include "WaterSource.h"
#include "Exception.h"

const double UNDEFINED_VOLUME = -1;
const unsigned long CHANGING_INTERVAL = 5 * 60 * 1000; // 5 minutes
const double CHANGING_TOLERANCE = 0.01;

class WaterTank
{
    public:
        double minimumVolume = 0;
        double maxVolume = 0;
        WaterSource* waterSource = NULL;

        WaterTank(VolumeReader* volumeReader, WaterSource* waterSource=NULL);

        double getVolume();
        double getPressure();
        void setZeroVolume(double pressure);
        void fill(bool force);
        void stopFilling();
        RuntimeError* loop();

    protected:
        VolumeReader* volumeReader;
    
    private:
        unsigned long startFillingTime;
        unsigned long lastChangingTime;
        double lastLoopPressure;
        unsigned long lastLoopTime;
};

#endif
