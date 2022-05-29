#ifndef API_H
#define API_H

#include <Arduino.h>

#include "Manager.h"
#include "WaterTank.h"
#include "Exception.h"

class API
{
    public:
        API();

        void createWaterSource(char* name, short pin);
        void createWaterSource(char* name, short pin, String waterTankName);
        void createWaterTank(String name, short volumeReaderPin, float volumeFactor, float pressureFactor);
        void createWaterTank(String name, short volumeReaderPin, float volumeFactor, float pressureFactor, String waterSourceName);
        void setWaterTankMinimumVolume(String name, float minimum);
        void setWaterTankMaxVolume(String name, float max);
        void setWaterZeroVolume(String name, float pressure);
        void setAutomaticMode();
        void setManualMode();
        void enableWaterSource(char* name);
        void disableWaterSource(char* name);
        WaterSource* getWaterSource(char* name);
        char** getWaterSourceList();
        unsigned int getWaterTankList(String* list);
        float getWaterTankVolume(String name);
        float getWaterTankPressure(String name);
        unsigned int getTotalWaterSources();
        void removeWaterSource(char* name);
        void removeWaterTank(String name);
        void reset();
        void loop();

    private:
        Manager* manager = NULL;

        WaterSource* getWaterSource(String name);
        long getWaterSourceIndex(String name);

        IOInterface* getOrCreateIO(unsigned pin, IOType type, IOMode mode=READ_ONLY);
};

#endif
