#ifndef MANAGER_H
#define MANAGER_H

#include <Arduino.h>

#include "Exception.h"
#include "WaterTank.h"
#include "OperationMode.h"

const unsigned int MAX_LENGTH = 20;
const unsigned int MAX_WATER_SOURCES = 20;
const unsigned int MAX_WATER_TANKS = 20;

class Manager
{
    public:
        Manager();

        OperationMode getMode();
        const RuntimeError** getErrors();
        WaterTank* getWaterTank(String name);
        WaterSource* getWaterSource(char* name);
        char** getWaterSourceNames();
        unsigned int getWaterTankNames(String* list);
        unsigned int getErrors(const RuntimeError** list);
        unsigned int getTotalWaterSources();
        void setOperationMode(OperationMode mode);
        void setWaterSourceState(char* name, bool enabled);
        void registerWaterSource(char* name, WaterSource* waterSource);
        WaterSource* unregisterWaterSource(char* name);
        void registerWaterTank(String name, WaterTank* waterTank);
        WaterTank* unregisterWaterTank(String name);
        void fillWaterTank(String name);
        void stopFillingWaterTank(String name);
        void loop();
        void reset();

    private:
        WaterTank** waterTanks = NULL;
        String** waterTankNames = NULL;
        WaterSource* waterSources[MAX_WATER_SOURCES];
        char* waterSourceNames[MAX_WATER_SOURCES];
        OperationMode mode = MANUAL;
        const RuntimeError** errors = NULL;
        unsigned int totalWaterTanks = 0;
        unsigned int totalWaterSources = 0;

        int getWaterTankIndex(String name);
        int getWaterSourceIndex(char* name);
};

#endif
