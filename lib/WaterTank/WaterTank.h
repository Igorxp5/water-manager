#ifndef WATER_TANK_H
#define WATER_TANK_H

#include "IOInterface.h"
#include "Exception.h"
#include "Clock.h"

const float UNDEFINED_VOLUME = -1;
const unsigned long CHANGING_INTERVAL = 300000UL; //5 minutes
const unsigned long FILLING_CALLS_PROTECTION_TIME = 60000UL;  //1 minute  
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
        ~WaterTank();

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
        Clock* fillingTimer;
        Clock* pressureChangingTimer;
        Clock* fillingCallsProtectionTimer;
        float lastLoopPressure;
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
        bool canEnable();
        WaterTank* getWaterTank();
        unsigned int getPin();

    private:
        IOInterface* io;
};

#endif
