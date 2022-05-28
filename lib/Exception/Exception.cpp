#include "Exception.h"

const Exception* Exception::thrownException = NULL;

const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE = new InvalidRequest(
    "Cannot open water source when the water tank is under the minimum threshold");

const InvalidRequest* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE = new InvalidRequest(
    "Cannot fill a water tank without setting a water source for it");

const InvalidRequest* WATER_TANK_NOT_FOUND = new InvalidRequest(
    "Could not find a water tank with the name provided");
const InvalidRequest* WATER_TANK_ALREADY_REGISTERED = new InvalidRequest(
    "There is already a water tank with that name registered");

const RuntimeError* WATER_TANK_STOPPED_TO_FILL = new RuntimeError("The water tank has stopped to fill");
const RuntimeError* WATER_TANK_IS_NOT_FILLING = new RuntimeError("The water tank is not filling");

const InvalidRequest* WATER_SOURCE_NOT_FOUND = new InvalidRequest(
    "Could not find a water source with the name provided");
const InvalidRequest* WATER_SOURCE_ALREADY_REGISTERED = new InvalidRequest(
    "There is already a water source with that name registered");

const InvalidRequest* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC = new InvalidRequest(
    "Cannot handle a water source in auto mode");

const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME = new InvalidRequest(
    "Cannot open a water source, the underlying water tank is under the minimum threshold");

const InvalidRequest* CANNOT_FILL_WATER_TANK_MAX_VOLUME = new InvalidRequest(
    "Cannot fill the water tank, maximum threshold reached");

const InvalidRequest* MAX_WATER_SOURCES_ERROR = new InvalidRequest("Max of water sources reached");
const InvalidRequest* MAX_WATER_TANKS_ERROR = new InvalidRequest("Max of water tanks reached");

Exception::Exception(const char* message, unsigned int exceptionType) {
    this->message = message;
    this->exceptionType = exceptionType;
}

RuntimeError::RuntimeError(const char* message) : Exception(message, RUNTIME_ERROR_EXCEPTION_TYPE) {

}

InvalidRequest::InvalidRequest(const char* message) : Exception(message, INVALID_REQUEST_EXCEPTION_TYPE) {

}

const char* Exception::getMessage() {
	return this->message;
}

void Exception::throwException(const Exception* exception) {
	Exception::thrownException = exception;
}

const Exception* Exception::popException() {
	const Exception* exception = Exception::thrownException;
    Exception::thrownException = NULL;
    return exception;
}

bool Exception::hasException() {
    return Exception::thrownException != NULL;
}

unsigned int Exception::getExceptionType() {
    return this->exceptionType;
}
