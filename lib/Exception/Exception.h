#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

class Exception
{
    public:
        Exception(const String message);

        String getMessage();
    
    private:
        String message;
};

class RuntimeError: public Exception
{
    public:
        RuntimeError(const String message);
};


// Exceptions
extern const Exception* CANNOT_ENABLE_WATER_SOURCE;
extern const Exception* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE;
extern const Exception* WATER_TANK_NOT_FOUND;
extern const Exception* WATER_TANK_ALREADY_REGISTERED;
extern const Exception* WATER_SOURCE_NOT_FOUND;
extern const Exception* WATER_SOURCE_ALREADY_REGISTERED;
extern const RuntimeError* WATER_TANK_STOPPED_TO_FILL;
extern const RuntimeError* WATER_TANK_IS_NOT_FILLING;
extern const RuntimeError* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC;
extern const RuntimeError* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
extern const RuntimeError* CANNOT_FILL_WATER_TANK_MAX_VOLUME;

#endif
