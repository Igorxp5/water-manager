#include "Exception.h"

const Exception* Exception::thrownException = NULL;
char Exception::thrownExceptionArg[MAX_ERROR_ARG_LENGTH + 1] = "";


Exception::Exception(const char* message, ErrorType exceptionType) {
    this->message = message;
    this->exceptionType = exceptionType;
}

Exception::Exception(const char* message) : Exception(message, GENERIC_ERROR) {

}

const char* Exception::getMessage() const {
	return this->message;
}

void Exception::throwException(const Exception* exception, char* arg) {
	Exception::thrownException = exception;
    if (arg != NULL) {
        strncpy(Exception::thrownExceptionArg, arg, MAX_ERROR_ARG_LENGTH);
    } else {
        Exception::clearExceptionArg();
    }
}

void Exception::throwException(const Exception* exception) {
	Exception::throwException(exception, NULL);
}

void Exception::clearException() {
    Exception::thrownException = NULL;
    Exception::clearExceptionArg();
}

const Exception* Exception::popException() {
	const Exception* exception = Exception::thrownException;
    Exception::thrownException = NULL;
    return exception;
}

char* Exception::popExceptionArg() {
    return Exception::thrownExceptionArg;
}

bool Exception::hasException() {
    return Exception::thrownException != NULL;
}

ErrorType Exception::getExceptionType() const {
    return this->exceptionType;
}

void Exception::clearExceptionArg() {
    Exception::thrownExceptionArg[0] = '\0';
}
