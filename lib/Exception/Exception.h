#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

class Exception
{
    public:
        Exception(const char* message);

        const char* getMessage();
    
    private:
        const char* message;
};

class RuntimeError: public Exception
{
    public:
        RuntimeError(const char* message);
};

class InvalidRequest: public Exception
{
    public:
        InvalidRequest(const char* message);
};


// Exceptions
extern const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE;
extern const InvalidRequest* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE;

extern const RuntimeError* WATER_TANK_NOT_FOUND;
extern const RuntimeError* WATER_TANK_ALREADY_REGISTERED;

extern const RuntimeError* WATER_TANK_STOPPED_TO_FILL;
extern const RuntimeError* WATER_TANK_IS_NOT_FILLING;

extern const RuntimeError* WATER_SOURCE_NOT_FOUND;
extern const RuntimeError* WATER_SOURCE_ALREADY_REGISTERED;

extern const InvalidRequest* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC;
extern const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
extern const InvalidRequest* CANNOT_FILL_WATER_TANK_MAX_VOLUME;

extern const InvalidRequest* MAX_LENGTH_ERROR;
extern const InvalidRequest* MAX_WATER_SOURCES_ERROR;
extern const InvalidRequest* MAX_WATER_TANKS_ERROR;

#endif
