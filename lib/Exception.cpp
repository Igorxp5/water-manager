#include "Exception.h"

Exception::Exception(String message) {
    this->message = message;
}

String Exception::getMessage() {
	return this->message;
}

RuntimeError::RuntimeError(String message) : Exception(message) {

}
