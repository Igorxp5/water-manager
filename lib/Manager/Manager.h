#ifndef MANAGER_H
#define MANAGER_H

#include <Arduino.h>

#include "Exception.h"
#include "WaterTank.h"
#include "OperationMode.h"

class Manager
{
    public:
        Manager();

        OperationMode getMode();
        const RuntimeError** getErrors();
        WaterTank* getWaterTank(String name);
        WaterSource* getWaterSource(String name);
        unsigned int getWaterSourceNames(String* list);
        unsigned int getWaterTankNames(String* list);
        unsigned int getErrors(const RuntimeError** list);
        void setOperationMode(OperationMode mode);
        void setWaterSourceState(String name, bool enabled);
        void registerWaterSource(String name, WaterSource* waterSource);
        void unregisterWaterSource(String name);
        void registerWaterTank(String name, WaterTank* waterTank);
        void unregisterWaterTank(String name);
        void fillWaterTank(String name);
        void stopFillingWaterTank(String name);
        void loop();

    private:
        WaterTank** waterTanks = NULL;
        String* waterTankNames = NULL;
        WaterSource** waterSources = NULL;
        String* waterSourceNames = NULL;
        OperationMode mode = MANUAL;
        const RuntimeError** errors = NULL;
        unsigned int totalWaterTanks = 0;
        unsigned int totalWaterSources = 0;

        long getWaterTankIndex(String name);
        long getWaterSourceIndex(String name);
};

#endif
