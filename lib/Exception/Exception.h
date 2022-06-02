#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

const byte MAX_ERROR_LENGTH = 100;

enum ErrorType {
    GENERIC_ERROR, RUNTIME_ERROR, INVALID_REQUEST
};

class Exception
{
    public:
        Exception(const char* message, ErrorType exceptionType);

        const char* getMessage();
        ErrorType getExceptionType();

        static void throwException(const Exception* exception);
        static void throwException(const Exception* exception, char* arg);
        static const Exception* popException();
        static char* popExceptionArg();
        static bool hasException();
        static void clearException();
    
    private:
        static const Exception* thrownException;
        static char* thrownExceptionArg;
        const char* message;
        ErrorType exceptionType;
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

extern const InvalidRequest* WATER_TANK_NOT_FOUND;
extern const InvalidRequest* WATER_TANK_ALREADY_REGISTERED;

extern const RuntimeError* WATER_TANK_HAS_STOPPED_TO_FILL;
extern const RuntimeError* WATER_TANK_IS_NOT_FILLING;

extern const InvalidRequest* WATER_SOURCE_NOT_FOUND;
extern const InvalidRequest* WATER_SOURCE_ALREADY_REGISTERED;

extern const InvalidRequest* CANNOT_HANDLE_WATER_SOURCE_IN_AUTO;
extern const InvalidRequest* CANNOT_HANDLE_WATER_TANK_IN_AUTO;
extern const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME;
extern const InvalidRequest* CANNOT_FILL_WATER_TANK_MAX_VOLUME;

extern const InvalidRequest* MAX_WATER_SOURCES_ERROR;
extern const InvalidRequest* MAX_WATER_TANKS_ERROR;

extern const InvalidRequest* INVALID_OPERATION_MODE;

extern const InvalidRequest* CANNOT_REMOVE_WATER_SOURCE_DEPENDENCY;
extern const InvalidRequest* CANNOT_REMOVE_WATER_TANK_DEPENDENCY;

extern const Exception* PIN_NOT_FOUND;

#endif
