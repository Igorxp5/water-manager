#ifndef PERSISTER_H
#define PERSISTER_H

#include <Arduino.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_common.h>

#include "API.h"
#include "Exception.h"
#include "api.pb.h"


/*
The Water Manager persists the Water Sources and Water Tanks using the EEPROM. The data is persisted
using the same protobuf protocol used by Requests. See the format:

START EEPROM ADDRESS: 0x0000
DESCRIPTION                 |   OFFSET  |   Data Type   |   Data length (bytes)   |

Amount of requests          |   0       |   byte        |   1
EEPROM CRC                  |   1       |   ulong       |   4
Length Table                |   5       |   byte array  |   MAX_REQUESTS (20)    |
Request 1 Length            |   5       |   byte        |   2
...
Request 1                   |   25     |   byte        |   Variable length
...
*/

class Persister
{
    public:
        static byte getTotalRequests();
        static bool isAPIDataCorrupted();
        static Request readRequest(byte index);
        static void save(API* api);
        static void clearEEPROM();

    private:
        //We need 2 requests to create a water tank/water source fully (create and setActive requests).
        static const byte MAX_REQUESTS = (MAX_WATER_TANKS * 2) + (MAX_WATER_SOURCES * 2);

        static const unsigned int TOTAL_REQUESTS_OFFSET = 0;
        static const unsigned int CRC_OFFSET = TOTAL_REQUESTS_OFFSET + sizeof(byte);
        static const unsigned int LENGTH_TABLE_OFFSET = CRC_OFFSET + sizeof(unsigned long);
        static const unsigned int REQUESTS_START_OFFSET = LENGTH_TABLE_OFFSET + (MAX_REQUESTS * sizeof(byte));

        static unsigned long calculateCRC();
        static void readEPPROM(byte* dest, unsigned int offset, unsigned int size);
        static void writeRequest(Request* request, byte index);
        static byte getRequestLength(byte index);
        static unsigned int getRequestOffset(byte index);
        static void setTotalRequests(byte total);
        static void updateCRC();
        static unsigned int calculateWaterTankDependencyWeight(WaterTank* waterTank, WaterSource** waterSources, WaterTank** waterTanks, 
                                                               unsigned int totalWaterSources, unsigned int totalWaterTanks);
        static unsigned int calculateWaterSourceDependencyWeight(WaterSource* waterSource, WaterSource** waterSources, WaterTank** waterTanks, 
                                                                 unsigned int totalWaterSources, unsigned int totalWaterTanks);
};

#endif