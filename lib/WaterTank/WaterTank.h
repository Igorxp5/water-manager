#ifndef WATER_TANK_H
#define WATER_TANK_H

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
        float volumeFactor;
        float pressureFactor;
        float zeroVolumePressure;

        WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor);
        WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor, WaterSource* waterSource);

        float getVolume();
        float getPressure();
        unsigned int getPressureRawValue();
        unsigned int getPressureSensorPin();
        WaterSource* getWaterSource();
        void fill(bool force);
        bool isFilling();
        void stopFilling();
        void loop();

    protected:
        IOInterface* pressureSensor;
        WaterSource* waterSource = NULL;

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
        WaterTank* getWaterTank();
        unsigned int getPin();

    private:
        IOInterface* io;
};

#endif
