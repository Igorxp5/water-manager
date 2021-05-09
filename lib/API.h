#ifndef API_H
#define API_H

#include <Arduino.h>

#include "Manager.h"
#include "WaterSource.h"
#include "Exception.h"

class API
{
    public:
        API();

        void createWaterSource(String name, short pin);
        void createWaterSource(String name, short pin, String waterTankName);
        void createWaterTank(String name, short volumeReaderPin, double volumeFactor, double pressureFactor);
        void createWaterTank(String name, short volumeReaderPin, double volumeFactor, double pressureFactor, String waterSourceName);
        void setWaterTankMinimumVolume(String name, double minimum);
        void setWaterTankMaxVolume(String name, double max);
        void setWaterZeroVolume(String name, double pressure);
        void setAutomaticMode();
        void setManualMode();
        void enableWaterSource(String name);
        void disableWaterSource(String name);
        unsigned int getWaterSourceList(String* list);
        unsigned int getWaterTankList(String* list);
        double getWaterTankVolume(String name);
        double getWaterTankPressure(String name);
        bool getWaterSourceState(String name);
        void removeWaterSource(String name);
        void removeWaterTank(String name);
        void reset();
        unsigned int getError(RuntimeError** list);

    private:
        Manager* manager = NULL;

        WaterSource* getWaterSource(String name);
        long getWaterSourceIndex(String name);

        IOInterface* getOrCreateIO(unsigned pin, IOType type, IOMode mode=READ_ONLY);
};

#endif
