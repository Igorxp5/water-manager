#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

class Exception
{
    public:
        Exception(const char* message, unsigned int exceptionType);

        const char* getMessage();
        unsigned int getExceptionType();

        static void throwException(const Exception* exception);
        static const Exception* popException();
        static bool hasException();
    
    private:
        static const Exception* thrownException;
        const char* message;
        unsigned int exceptionType;
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

//Exceptions types
const unsigned int RUNTIME_ERROR_EXCEPTION_TYPE = 1;
const unsigned int INVALID_REQUEST_EXCEPTION_TYPE = 2;

// Exceptions
extern const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE;
extern const InvalidRequest* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE;

extern const InvalidRequest* WATER_TANK_NOT_FOUND;
extern const InvalidRequest* WATER_TANK_ALREADY_REGISTERED;

extern const RuntimeError* WATER_TANK_STOPPED_TO_FILL;
extern const RuntimeError* WATER_TANK_IS_NOT_FILLING;

extern const InvalidRequest* WATER_SOURCE_NOT_FOUND;
extern const InvalidRequest* WATER_SOURCE_ALREADY_REGISTERED;

extern const InvalidRequest* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC;
extern const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
extern const InvalidRequest* CANNOT_FILL_WATER_TANK_MAX_VOLUME;

extern const InvalidRequest* MAX_WATER_SOURCES_ERROR;
extern const InvalidRequest* MAX_WATER_TANKS_ERROR;

#endif
