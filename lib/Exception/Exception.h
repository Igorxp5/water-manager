#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

const byte MAX_ERROR_LENGTH = 100;
const byte MAX_ERROR_ARG_LENGTH = 20;

enum ErrorType {
    GENERIC_ERROR, RUNTIME_ERROR, INVALID_REQUEST
};

class Exception
{
    public:
        Exception(const char* message);
        Exception(const char* message, ErrorType exceptionType);

        const char* getMessage() const;
        ErrorType getExceptionType() const;

        static void throwException(const Exception* exception);
        static void throwException(const Exception* exception, char* arg);
        static const Exception* popException();
        static char* popExceptionArg();
        static bool hasException();
        static void clearException();
    
    private:
        const char* message;
        ErrorType exceptionType;

        static char thrownExceptionArg[MAX_ERROR_ARG_LENGTH + 1];
        static const Exception* thrownException;
        
        static void clearExceptionArg();
};

const Exception CANNOT_ENABLE_WATER_SOURCE = Exception(
    "Cannot open water source when the water tank is under the minimum threshold", INVALID_REQUEST);

const Exception CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE = Exception(
    "Cannot fill a water tank without setting a water source for it", INVALID_REQUEST);

const Exception WATER_TANK_NOT_FOUND = Exception(
    "Could not find a water tank with the name provided", INVALID_REQUEST);
const Exception WATER_TANK_ALREADY_REGISTERED = Exception(
    "There is already a water tank with that name registered", INVALID_REQUEST);

const Exception WATER_TANK_HAS_STOPPED_TO_FILL = Exception("The water tank has stopped to fill", RUNTIME_ERROR);
const Exception WATER_TANK_IS_NOT_FILLING = Exception("The water tank is not filling", RUNTIME_ERROR);
const Exception MAX_TIME_WATER_TANK_NOT_FILLING = Exception("The water tank deactivated. It has stopped to fill", RUNTIME_ERROR);

const Exception CANNOT_FILL_DEACTIVATED_WATER_TANK = Exception(
    "Cannot fill a deactivated water tank", INVALID_REQUEST);
const Exception CANNOT_TURN_ON_DEACTIVATED_WATER_SOURCE = Exception(
    "Cannot turn on a deactivated water source", INVALID_REQUEST);

const Exception WATER_SOURCE_NOT_FOUND = Exception(
    "Could not find a water source with the name provided", INVALID_REQUEST);
const Exception WATER_SOURCE_ALREADY_REGISTERED = Exception(
    "There is already a water source with that name registered", INVALID_REQUEST);

const Exception CANNOT_HANDLE_WATER_SOURCE_IN_AUTO = Exception(
    "Cannot handle a water source in auto mode", INVALID_REQUEST);

const Exception CANNOT_HANDLE_WATER_TANK_IN_AUTO = Exception(
    "Cannot handle a water tank in auto mode", INVALID_REQUEST);

const Exception CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME = Exception(
    "Cannot open a water source, the underlying water tank is under the minimum threshold", INVALID_REQUEST);

const Exception CANNOT_FILL_WATER_TANK_MAX_VOLUME = Exception(
    "Cannot fill the water tank, maximum threshold reached", INVALID_REQUEST);

const Exception MAX_WATER_SOURCES_ERROR = Exception("Max of water sources reached", INVALID_REQUEST);
const Exception MAX_WATER_TANKS_ERROR = Exception("Max of water tanks reached", INVALID_REQUEST);

const Exception INVALID_OPERATION_MODE = Exception("Invalid operation mode", INVALID_REQUEST);

const Exception CANNOT_REMOVE_WATER_SOURCE_DEPENDENCY = Exception(
    "Cannot remove the water source, there is a water tank dependent of it", INVALID_REQUEST);
const Exception CANNOT_REMOVE_WATER_TANK_DEPENDENCY = Exception(
    "Cannot remove the water tank, there is a water source dependent of it", INVALID_REQUEST);

const Exception PIN_NOT_FOUND = Exception("Pin is not defined in an IOInterface object", INVALID_REQUEST);

const Exception RESOURCE_NAME_EMPTY = Exception("Cannot create a resource with an empty name", INVALID_REQUEST);

#endif
