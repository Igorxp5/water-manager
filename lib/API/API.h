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
        void createWaterSource(char* name, short pin, char* waterTankName);
        void createWaterTank(char* name, short volumeReaderPin, float volumeFactor, float pressureFactor);
        void createWaterTank(char* name, short volumeReaderPin, float volumeFactor, float pressureFactor, char* waterSourceName);
        void setWaterTankMinimumVolume(char* name, float minimum);
        void setWaterTankMaxVolume(char* name, float max);
        void setWaterZeroVolume(char* name, float pressure);
        void setWaterTankVolumeFactor(char* name, float volumeFactor);
        void setWaterTankPressureFactor(char* name, float pressureFactor);
        void setOperationMode(byte mode);
        byte getOperationMode();
        void setWaterSourceState(char* name, bool enabled);
        WaterSource* getWaterSource(char* name);
        WaterTank* getWaterTank(char* name);
        char* getWaterSourceName(WaterSource* waterSource);
        char* getWaterTankName(WaterTank* waterTank);
        char** getWaterSourceList();
        char** getWaterTankList();
        float getWaterTankVolume(char* name);
        float getWaterTankPressure(char* name);
        unsigned int getTotalWaterSources();
        unsigned int getTotalWaterTanks();
        void removeWaterSource(char* name);
        void removeWaterTank(char* name);
        void fillWaterTank(char* name, bool enabled, bool force);
        void reset();
        void loop();

    private:
        Manager* manager = NULL;

        IOInterface* getOrCreateIO(unsigned pin, IOType type, IOMode mode=READ_ONLY);
};

#endif
