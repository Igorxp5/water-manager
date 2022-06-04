#include "Persister.h"

#include <EEPROM.h>

byte Persister::getTotalRequests() {
    return EEPROM.read(Persister::TOTAL_REQUESTS_OFFSET);
}

Request Persister::readRequest(byte index) {
    Request request = Request_init_zero;
    byte requestBuffer[Request_size];
    pb_istream_t requestStream;

    byte requestLength = Persister::getRequestLength(index);
    Persister::readEPPROM(requestBuffer, Persister::getRequestOffset(index), requestLength);

    requestStream = pb_istream_from_buffer(requestBuffer, requestLength);
    if(!pb_decode(&requestStream, Request_fields, &request)) {
        Exception::throwException(&SAVE_CORRUPTED);
    }
    return request;
}

void Persister::readEPPROM(byte* dest, unsigned int offset, unsigned int size) {
    for(unsigned int e = offset, d = 0; d < size; e++, d++) {
        dest[d] = EEPROM.read(e);
    }
}

void Persister::save(API* api) {
    unsigned int totalWaterSources = api->getTotalWaterSources();
    unsigned int totalWaterTanks = api->getTotalWaterTanks();

    char** waterSourceNames = api->getWaterSourceList();
    char** waterTankNames = api->getWaterTankList();
    
    WaterSource** waterSources = new WaterSource*[totalWaterSources];
    WaterTank** waterTanks = new WaterTank*[totalWaterTanks];
    
    unsigned int i, j = 0;
    for(i = 0; i < totalWaterSources; i++) {
        waterSources[i] = api->getWaterSource(waterSourceNames[i]);
    }
    for(i = 0; i < totalWaterTanks; i++) {
        waterTanks[i] = api->getWaterTank(waterTankNames[i]);
    }

    //Calculating weights
    byte* waterSourceWeights = new byte[totalWaterSources] {};
    byte* waterTankWeights = new byte[totalWaterTanks] {};

    for (i = 0; i < totalWaterSources; i++) {
        waterSourceWeights[i] = Persister::calculateWaterSourceDependencyWeight(waterSources[i], waterSources, waterTanks, 
                                                                                totalWaterSources, totalWaterTanks);
    }
    for (i = 0; i < totalWaterTanks; i++) {
        waterTankWeights[i] = Persister::calculateWaterTankDependencyWeight(waterTanks[i], waterSources, waterTanks, 
                                                                            totalWaterSources, totalWaterTanks);
    }

    //Sort weights
    WaterSource* waterSource = NULL;
    WaterTank* waterTank = NULL;
    char* name = NULL;
    byte weight;
    unsigned int minWeightIndex;
    
    for (i = 0; i < totalWaterSources; i++) {
        minWeightIndex = i;
        for (j = 0; j < totalWaterSources; j++) {
            if (waterSourceWeights[j] < waterSourceWeights[minWeightIndex]) {
                minWeightIndex = j;
            }
        }
        weight = waterSourceWeights[i];
        waterSourceWeights[i] = waterSourceWeights[minWeightIndex];
        waterSourceWeights[minWeightIndex] = weight;
        
        waterSource = waterSources[i];
        waterSources[i] = waterSources[minWeightIndex];
        waterSources[minWeightIndex] = waterSource;

        name = waterSourceNames[i];
        waterSourceNames[i] = waterSourceNames[minWeightIndex];
        waterSourceNames[minWeightIndex] = name;
    }

    for (i = 0; i < totalWaterTanks; i++) {
        minWeightIndex = i;
        for (j = 0; j < totalWaterTanks; j++) {
            if (waterTankWeights[j] < waterTankWeights[minWeightIndex]) {
                minWeightIndex = j;
            }
        }
        weight = waterTankWeights[i];
        waterTankWeights[i] = waterTankWeights[minWeightIndex];
        waterTankWeights[minWeightIndex] = weight;
        
        waterTank = waterTanks[i];
        waterTanks[i] = waterTanks[minWeightIndex];
        waterTanks[minWeightIndex] = waterTank;

        name = waterTankNames[i];
        waterTankNames[i] = waterTankNames[minWeightIndex];
        waterTankNames[minWeightIndex] = name;
    }

    //Creating requests

    Request request = {};

    byte totalRequests = 0;

    i = 0;
    j = 0;
    bool isWaterSourceLower;
    while (i < totalWaterSources || j < totalWaterTanks) {
        if (i >= totalWaterSources) {
            isWaterSourceLower = false; 
        } else if (j >= totalWaterTanks) {
            isWaterSourceLower = true;
        } else {
            isWaterSourceLower = waterSourceWeights[i] < waterTankWeights[j];
        }
        if (isWaterSourceLower) {
            name = waterSourceNames[i];
            waterSource = waterSources[i];

            request = {};
            request.which_message = Request_createWaterSource_tag;
            request.message.createWaterSource.pin = waterSource->getPin();
            strncpy(request.message.createWaterSource.name, name, MAX_NAME_LENGTH);
            if (waterSource->getWaterTank() != NULL) {
                request.message.createWaterSource.has_waterTankName = true;
                strncpy(request.message.createWaterSource.waterTankName, api->getWaterTankName(waterSource->getWaterTank()), MAX_NAME_LENGTH);
            }
            
            Persister::writeRequest(&request, totalRequests);
            if (Exception::hasException()) {
                break;
            }
            totalRequests += 1;

            if (!waterSource->isActive()) {
                request = {};
                request.which_message = Request_setWaterSourceActive_tag;
                strncpy(request.message.setWaterSourceActive.waterSourceName, name, MAX_NAME_LENGTH);
                request.message.setWaterSourceActive.active = false;
                Persister::writeRequest(&request, totalRequests);
                if (Exception::hasException()) {
                    break;
                }
                totalRequests += 1;
            }
            i += 1;
        } else {
            name = waterTankNames[j];
            waterTank = waterTanks[j];

            request = {};
            request.which_message = Request_createWaterTank_tag;
            request.message.createWaterTank.pressureSensorPin = waterTank->getPressureSensorPin();
            request.message.createWaterTank.pressureFactor = waterTank->pressureFactor;
            request.message.createWaterTank.volumeFactor = waterTank->volumeFactor;
            strncpy(request.message.createWaterTank.name, name, MAX_NAME_LENGTH);
            if (waterSource->getWaterTank() != NULL) {
                request.message.createWaterTank.has_waterSourceName = true;
                strncpy(request.message.createWaterTank.waterSourceName, api->getWaterSourceName(waterTank->getWaterSource()), MAX_NAME_LENGTH);
            }
            Persister::writeRequest(&request, totalRequests);
            if (Exception::hasException()) {
                break;
            }
            totalRequests += 1;

            if (!waterTank->isActive()) {
                request = {};
                request.which_message = Request_setWaterTankActive_tag;
                strncpy(request.message.setWaterTankActive.waterTankName, name, MAX_NAME_LENGTH);
                request.message.setWaterTankActive.active = false;
                Persister::writeRequest(&request, totalRequests);
                if (Exception::hasException()) {
                    break;
                }
                totalRequests += 1;
            }

            j += 1;
        }
    }

    free(waterSourceNames);
    free(waterTankNames);

    delete[] waterSourceWeights;
    delete[] waterTankWeights;

    delete[] waterSources;
    delete[] waterTanks;

    Persister::setTotalRequests(totalRequests);
    Persister::updateCRC();
}

unsigned int Persister::calculateWaterTankDependencyWeight(WaterTank* waterTank, WaterSource** waterSources, WaterTank** waterTanks, 
                                                           unsigned int totalWaterSources, unsigned int totalWaterTanks) {
    //FIXME: This recursive function uses a lot of memory to perform its task
    unsigned int dependencies = 0;
    for (unsigned int i = 0; i < totalWaterSources; i++) {
        if(waterSources[i]->getWaterTank() == waterTank) {
            dependencies += Persister::calculateWaterSourceDependencyWeight(waterSources[i], waterSources, waterTanks,
                                                                            totalWaterSources, totalWaterTanks);
        }
    }
    return dependencies;
}

unsigned int Persister::calculateWaterSourceDependencyWeight(WaterSource* waterSource, WaterSource** waterSources, WaterTank** waterTanks, 
                                                             unsigned int totalWaterSources, unsigned int totalWaterTanks) {
    //FIXME: This recursive function uses a lot of memory to perform its task 
    unsigned int dependencies = 0;
    for (unsigned int i = 0; i < totalWaterTanks; i++) {
        if(waterTanks[i]->getWaterSource() == waterSource) {
            dependencies += Persister::calculateWaterTankDependencyWeight(waterTanks[i], waterSources, waterTanks,
                                                                          totalWaterSources, totalWaterTanks);
        }
    }
    return dependencies;
}

byte Persister::getRequestLength(byte index) {
    return EEPROM.read(LENGTH_TABLE_OFFSET + (index * sizeof(byte)));
}

unsigned int Persister::getRequestOffset(byte index) {
    unsigned int offset = REQUESTS_START_OFFSET;
    for (unsigned int i = 0; i < index; i++) {
        offset += Persister::getRequestLength(i);
    }
    return offset;
}

void Persister::writeRequest(Request* request, byte index) {
    byte requestBuffer[Request_size];
    pb_ostream_t requestStream;
    
    requestStream = pb_ostream_from_buffer(requestBuffer, Request_size);

    if(pb_encode(&requestStream, Request_fields, request)) {
        EEPROM.update(LENGTH_TABLE_OFFSET + index, (byte) requestStream.bytes_written);

        unsigned int offset = Persister::getRequestOffset(index);
        for (unsigned int i = 0; i < requestStream.bytes_written; i++) {
            EEPROM.update(offset + i, requestBuffer[i]);
        }
    } else {
        Exception::throwException(&FAILED_TO_SAVE);
    }
}

void Persister::setTotalRequests(byte total) {
    EEPROM.update(Persister::TOTAL_REQUESTS_OFFSET, total);
}

void Persister::clearEEPROM() {
    setTotalRequests(0);
}

void Persister::updateCRC() {
    unsigned long crc = calculateCRC();
    EEPROM.put(CRC_OFFSET, crc);
}

bool Persister::isAPIDataCorrupted() {
    byte totalRequests = Persister::getTotalRequests();
    if (totalRequests == 0) {
        return false;
    } else if (totalRequests > Persister::MAX_REQUESTS) {
        return true;
    }

    unsigned long headerCRC = 0;
    
    EEPROM.get(Persister::CRC_OFFSET, headerCRC);

    return headerCRC == calculateCRC();
}

unsigned long Persister::calculateCRC() {
    /***
    Written by Christopher Andrews.
    CRC algorithm generated by pycrc, MIT licence ( https://github.com/tpircher/pycrc ).
    ***/
   const unsigned long crcTable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
    byte totalRequests = Persister::getTotalRequests();

    if (totalRequests == 0) {
        return 0;
    }

    unsigned long crc =  ~0L;
    unsigned eepromLength = Persister::getRequestOffset(totalRequests - 1) + Persister::getRequestLength(totalRequests - 1);

    for (unsigned int i = 0; i < eepromLength; i++) {
        crc = crcTable[(crc ^ EEPROM[i]) & 0x0f] ^ (crc >> 4);
        crc = crcTable[(crc ^ (EEPROM[i] >> 4)) & 0x0f] ^ (crc >> 4);
        crc = ~crc;
    }

    return crc;
}
