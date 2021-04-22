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
        RuntimeError** getErrors();
        WaterTank* getWanterTank(String name);
        void setOperationMode(OperationMode mode);
        void registerWaterTank(String name, WaterTank* waterTank);
        void unregisterWaterTank(String name);
        void fillWaterTank(String name);
        void stopFillingWaterTank(String name);
        void loop();

    private:
        WaterTank** waterTanks = NULL;
        String* waterTankNames = NULL;
        OperationMode mode = MANUAL;
        RuntimeError** errors = NULL;
        unsigned int totalWaterTanks = 0;

        long getWaterTankIndex(String name);
};

#endif
