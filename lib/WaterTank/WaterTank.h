#ifndef WATER_TANK_H
#define WATER_TANK_H

#include "IOInterface.h"
#include "Exception.h"
#include "Clock.h"

const float UNDEFINED_VOLUME = -1;
const unsigned long CHANGING_INTERVAL = 300000UL; //5 minutes
const unsigned long FILLING_CALLS_PROTECTION_TIME = 60000UL;  //1 minute
const unsigned long MAX_TIME_NOT_FILLING = 600000UL; //10 minutes

class WaterSource;

class WaterTank
{
    public:
        float minimumVolume = 0;
        float maxVolume = 0;
        float volumeFactor;
        float pressureFactor;
        float zeroVolumePressure;
        float pressureChangingValue = 0.2;

        WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor);
        WaterTank(IOInterface* pressureSensor, float volumeFactor, float pressureFactor, WaterSource* waterSource);
        ~WaterTank();

        float getVolume();
        float getPressure();
        bool canFill();
        bool isActive();
        unsigned int getPressureRawValue();
        unsigned int getPressureSensorPin();
        WaterSource* getWaterSource();
        void fill(bool force);
        bool isFilling();
        void stopFilling();
        void setActive(bool active);
        void loop();

    protected:
        IOInterface* pressureSensor;
        WaterSource* waterSource = NULL;

    private:
    bool active;
        Clock* fillingTimer;
        Clock* pressureChangingTimer;
        Clock* fillingCallsProtectionTimer;
        float lastLoopPressure;
        const Exception* error;
};

class WaterSource
{
    public:
        WaterTank* waterTank = NULL;

        WaterSource(IOInterface* io);
        WaterSource(IOInterface* io, WaterTank* waterTank);

        void turnOn(bool force=false);
        void turnOff();
        bool isTurnedOn();
        bool isActive();
        bool canEnable();
        void setActive(bool active);
        WaterTank* getWaterTank();
        unsigned int getPin();

    private:
        IOInterface* io;
        bool active;
};

#endif
