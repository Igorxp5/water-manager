#ifndef MANAGER_H
#define MANAGER_H

#include <Arduino.h>

#include "Exception.h"
#include "WaterTank.h"
#include "OperationMode.h"

const unsigned int MAX_NAME_LENGTH = 20;
const unsigned int MAX_WATER_SOURCES = 10;
const unsigned int MAX_WATER_TANKS = 10;
const unsigned int ERROR_INTERVAL = 10 * 1000;

class Manager
{
    public:
        Manager();

        OperationMode getOperationMode();
        void setOperationMode(OperationMode mode);
        WaterTank* getWaterTank(char* name);
        WaterSource* getWaterSource(char* name);
        char* getWaterSourceName(WaterSource* waterSource);
        char* getWaterTankName(WaterTank* waterTank);
        char** getWaterSourceNames();
        char** getWaterTankNames();
        unsigned int getTotalWaterTanks();
        unsigned int getTotalWaterSources();
        void setWaterSourceState(char* name, bool enabled);
        void registerWaterSource(char* name, WaterSource* waterSource);
        void registerWaterTank(char* name, WaterTank* waterTank);
        bool isWaterSourceRegistered(char* name);
        bool isWaterTankRegistered(char* name);
        bool isWaterSourceDependency(char* name);
        bool isWaterTankDependency(char* name);
        WaterSource* unregisterWaterSource(char* name);
        WaterTank* unregisterWaterTank(char* name);
        void fillWaterTank(char* name);
        void stopFillingWaterTank(char* name);
        void loop();
        void reset();

    private:
        WaterTank* waterTanks[MAX_WATER_TANKS];
        char* waterTankNames[MAX_WATER_TANKS];
        WaterSource* waterSources[MAX_WATER_SOURCES];
        char* waterSourceNames[MAX_WATER_SOURCES];
        OperationMode mode = MANUAL;
        unsigned int totalWaterTanks = 0;
        unsigned int totalWaterSources = 0;
        unsigned long lastLoopTime = 0;
        unsigned int waterTankErrorIndex = 0;
        const RuntimeError* waterTanksLoopErrors[MAX_WATER_TANKS];

        int getWaterTankIndex(char* name);
        int getWaterSourceIndex(char* name);
};

#endif
