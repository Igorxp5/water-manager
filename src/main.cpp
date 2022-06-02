#include <Arduino.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_common.h>

#include "API.h"
#include "Clock.h"
#include "IOInterface.h"
#include "api.pb.c"

#include "Utils.h"

#ifdef TEST
#include "test.pb.c"
#include "MemoryFree.h"
#endif

const unsigned int READ_TIMEOUT = 2500; //Miliseconds

API* api;
Clock* readerTimer;
HardwareSerial* apiSerial = &Serial;

void setup() {
    apiSerial->begin(9600);
    apiSerial->setTimeout(READ_TIMEOUT);
    api = new API();
    readerTimer = new Clock();
}

/*
The Protobuf messages are transfered using following format:
[byte] messageType
[uint] messageLength
[variable-size] MESSAGE

messageType:
01: API.proto messages
02: Test.proto messages
03: Debug messages
*/

#ifdef TEST
const unsigned int MAX_MESSAGE_SIZE = max(Request_size, _TestRequest_size);
#else
const unsigned int MAX_MESSAGE_SIZE = Request_size;
#endif

byte requestBuffer[Request_size];
byte responseBuffer[Response_size];
byte messageType = 0;
unsigned int read = 0;

byte messageLengthBuffer[2] = {0, 0};
byte messageLengthBufferReadIndex = 0;
unsigned int messageLength;

Request request = Request_init_zero;
Response response = Response_init_zero;
pb_istream_t requestStream;
pb_ostream_t responseStream;

#ifdef TEST
byte testResponseBuffer[_TestResponse_size];
_TestRequest testRequest = _TestRequest_init_zero;
_TestResponse testResponse = _TestResponse_init_zero;
#endif

void freeRequestBuffer() {
    read = 0;
    messageType = 0;
    messageLength = 0;
    messageLengthBufferReadIndex = 0;
    request = {};
}

void freeResponseBuffer() {
    response = {};

    #ifdef TEST
    testResponse = {};
    #endif
}

void sendResponse() {
    responseStream = pb_ostream_from_buffer(responseBuffer, Response_size);

    if(!pb_encode(&responseStream, Response_fields, &response)) {
        //TODO: Handle failed to encode response
    } else {
        Serial.write((byte) 1); //API message type
        unsigned int responseLength = (long unsigned int) responseStream.bytes_written;
        Serial.write((byte*) &responseLength, sizeof(unsigned int));
        Serial.write(responseBuffer, responseLength);
        Serial.flush();
    }
}

void sendErrorResponse(unsigned int requestId, const char* error, char* arg) {
    response.id = requestId;
    response.which_content = Response_error_tag;
    strncpy(response.content.error.message, error, MAX_ERROR_LENGTH);
    if (arg != NULL) {
        strncpy(response.content.error.arg, arg, MAX_NAME_LENGTH);
    }
    sendResponse();
}

void sendErrorResponse(unsigned int requestId, const RuntimeError* error, char* arg) {
    response.content.error.type = Error_Exception_RUNTIME_ERROR;
    const char* message = ((Exception*) error)->getMessage();
    sendErrorResponse(requestId, message, arg);
    delete error;
}

void sendErrorResponse(unsigned int requestId, const RuntimeError* error) {
    sendErrorResponse(requestId, error, NULL);
}

void sendErrorResponse(unsigned int requestId, const char* error) {
    sendErrorResponse(requestId, error, NULL);
}

void sendErrorResponse(unsigned int requestId, const InvalidRequest* error) {
    response.content.error.type = Error_Exception_INVALID_REQUEST;
    const char* message = ((Exception*) error)->getMessage();
    sendErrorResponse(requestId, message);
}

void sendOkResponse(unsigned int requestId) {
    response.id = requestId;
    response.which_content = Response_message_tag;
    sendResponse();
}

void handleAPIRequest() {
    if (request.which_message == Request_createWaterSource_tag) {
        if (request.message.createWaterSource.has_waterTankName) {
            api->createWaterSource(request.message.createWaterSource.name, request.message.createWaterSource.pin, 
                                   request.message.createWaterSource.waterTankName);
        } else {
            api->createWaterSource(request.message.createWaterSource.name, request.message.createWaterSource.pin);
        }
    } else if (request.which_message == Request_getWaterSourceList_tag) {
        char** waterSourceList = api->getWaterSourceList();
        unsigned int totalWaterSources = api->getTotalWaterSources();
        response.content.message.listValue_count = totalWaterSources;
        PrimitiveValue value = PrimitiveValue_init_zero;
        for (unsigned int i = 0; i < totalWaterSources; i++) {
            value.which_content = PrimitiveValue_stringValue_tag;
            strncpy(value.content.stringValue, waterSourceList[i], MAX_NAME_LENGTH);
            response.content.message.listValue[i] = value;
        }
        free(waterSourceList);
    } else if (request.which_message == Request_removeWaterSource_tag) {
        api->removeWaterSource(request.message.removeWaterSource.waterSourceName);
    } else if (request.which_message == Request_setWaterSourceState_tag) {
        api->setWaterSourceState(request.message.setWaterSourceState.waterSourceName, request.message.setWaterSourceState.state);
    } else if (request.which_message == Request_getWaterSource_tag) {
        WaterSource* waterSource = api->getWaterSource(request.message.getWaterSource.waterSourceName);
        if (waterSource != NULL) {
            response.content.message.has_waterSource = true;
            WaterSourceState waterSourceState = WaterSourceState_init_zero;
            strncpy(waterSourceState.name, request.message.getWaterSource.waterSourceName, MAX_NAME_LENGTH);
            waterSourceState.pin = waterSource->getPin();
            waterSourceState.enabled = waterSource->isEnabled();
            if (waterSource->getWaterTank() != NULL) {
                waterSourceState.has_sourceWaterTank = true;
                strncpy(waterSourceState.sourceWaterTank, api->getWaterTankName(waterSource->getWaterTank()), MAX_NAME_LENGTH);
            }
            response.content.message.waterSource = waterSourceState;
        }
    } if (request.which_message == Request_createWaterTank_tag) {
        if (request.message.createWaterTank.has_waterSourceName) {
            api->createWaterTank(request.message.createWaterTank.name, request.message.createWaterTank.pressureSensorPin, 
                                 request.message.createWaterTank.volumeFactor, request.message.createWaterTank.pressureFactor,
                                 request.message.createWaterTank.waterSourceName);
        } else {
            api->createWaterTank(request.message.createWaterTank.name, request.message.createWaterTank.pressureSensorPin, 
                                 request.message.createWaterTank.volumeFactor, request.message.createWaterTank.pressureFactor);
        }
    } else if (request.which_message == Request_getWaterTankList_tag) {
        char** waterTankList = api->getWaterTankList();
        unsigned int totalWaterTanks = api->getTotalWaterTanks();
        response.content.message.listValue_count = totalWaterTanks;
        PrimitiveValue value = PrimitiveValue_init_zero;
        for (unsigned int i = 0; i < totalWaterTanks; i++) {
            value.which_content = PrimitiveValue_stringValue_tag;
            strncpy(value.content.stringValue, waterTankList[i], MAX_NAME_LENGTH);
            response.content.message.listValue[i] = value;
        }
        free(waterTankList);
    } else if (request.which_message == Request_removeWaterTank_tag) {
        api->removeWaterTank(request.message.removeWaterTank.waterTankName);
    } else if (request.which_message == Request_getWaterTank_tag) {
        WaterTank* waterTank = api->getWaterTank(request.message.getWaterTank.waterTankName);
        if (waterTank != NULL) {
            response.content.message.has_waterTank = true;
            WaterTankState waterTankState = WaterTankState_init_zero;
            strncpy(waterTankState.name, request.message.getWaterTank.waterTankName, MAX_NAME_LENGTH);
            waterTankState.pressureSensorPin = waterTank->getPressureSensorPin();
            waterTankState.filling = waterTank->isFilling();
            waterTankState.volumeFactor = waterTank->volumeFactor;
            waterTankState.pressureFactor = waterTank->pressureFactor;
            waterTankState.minimumVolume = waterTank->minimumVolume;
            waterTankState.maxVolume = waterTank->maxVolume;
            waterTankState.zeroVolumePressure = waterTank->zeroVolumePressure;
            waterTankState.rawPressureValue = waterTank->getPressureRawValue();
            waterTankState.pressure = waterTank->getPressure();
            waterTankState.volume = waterTank->getVolume();
            if (waterTank->getWaterSource() != NULL) {
                waterTankState.has_waterSource = true;
                strncpy(waterTankState.waterSource, api->getWaterSourceName(waterTank->getWaterSource()), MAX_NAME_LENGTH);
            }
            response.content.message.waterTank = waterTankState;
        }
    } else if (request.which_message == Request_setWaterTankMinimumVolume_tag) {
        api->setWaterTankMinimumVolume(request.message.setWaterTankMinimumVolume.waterTankName, request.message.setWaterTankMinimumVolume.value);
    } else if (request.which_message == Request_setWaterTankMaxVolume_tag) {
        api->setWaterTankMaxVolume(request.message.setWaterTankMaxVolume.waterTankName, request.message.setWaterTankMaxVolume.value);
    } else if (request.which_message == Request_setWaterTankZeroVolume_tag) {
        api->setWaterZeroVolume(request.message.setWaterTankZeroVolume.waterTankName, request.message.setWaterTankZeroVolume.value);
    } else if (request.which_message == Request_setWaterTankVolumeFactor_tag) {
        api->setWaterTankVolumeFactor(request.message.setWaterTankVolumeFactor.waterTankName, request.message.setWaterTankVolumeFactor.value);
    } else if (request.which_message == Request_setWaterTankPressureFactor_tag) {
        api->setWaterTankPressureFactor(request.message.setWaterTankPressureFactor.waterTankName, request.message.setWaterTankPressureFactor.value);
    } else if (request.which_message == Request_fillWaterTank_tag) {
        api->fillWaterTank(request.message.fillWaterTank.waterTankName, request.message.fillWaterTank.enabled, request.message.fillWaterTank.force);
    } else if (request.which_message == Request_setMode_tag) {
        api->setOperationMode(request.message.setMode.mode);
    } else if (request.which_message == Request_getMode_tag) {
        byte mode = api->getOperationMode();
        if (!Exception::hasException()) {
            response.content.message.has_value = true;
            response.content.message.value.which_content = PrimitiveValue_intValue_tag;
            response.content.message.value.content.intValue = mode;
        }
    } else if (request.which_message == Request_reset_tag) {
        api->reset();
    }

    if (!Exception::hasException()) {
        sendOkResponse(request.id);
    } else {
        Exception* exception = (Exception*) Exception::popException();
        if (exception->getExceptionType() == RUNTIME_ERROR) {
            sendErrorResponse(request.id, (const RuntimeError*) exception);
        } else if (exception->getExceptionType() == INVALID_REQUEST) {
            sendErrorResponse(request.id, (const InvalidRequest*) exception);
        }
    }
}

#ifdef TEST
void sendTestResponse() {
    responseStream = pb_ostream_from_buffer(testResponseBuffer, _TestResponse_size);

    if(!pb_encode(&responseStream, _TestResponse_fields, &testResponse)) {
        //TODO: Handle failed to encode response
    } else {
        Serial.write((byte) 2); //Test message type
        unsigned int responseLength = (long unsigned int) responseStream.bytes_written;
        Serial.write((byte*) &responseLength, sizeof(unsigned int));
        Serial.write(testResponseBuffer, responseLength);
        Serial.flush();
    }
}

void sendErrorTestResponse(unsigned int requestId, const char* error) {
    testResponse.id = requestId;
    testResponse.error = true;
    testResponse.has_message = true;
    testResponse.message.which_value = _TestResponseValue_stringValue_tag;
    strncpy(testResponse.message.value.stringValue, error, MAX_ERROR_LENGTH);
    sendTestResponse();
}

void sendOkTestResponse(unsigned int requestId) {
    testResponse.id = requestId;
    sendTestResponse();
}

void sendOkTestResponse(unsigned int requestId, int message) {
    testResponse.id = requestId;
    testResponse.has_message = true;
    testResponse.message.which_value = _TestResponseValue_intValue_tag;
    testResponse.message.value.intValue = message;
    sendTestResponse();
}

void handleTestRequest() {
    if (testRequest.which_message == _TestRequest_createIO_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.createIO.pin);
        if (io == NULL) {
            IOType type;
            if (testRequest.message.createIO.type == _TestCreateIO_IOType_DIGITAL) {
                type = DIGITAL;
            } else if (testRequest.message.createIO.type == _TestCreateIO_IOType_ANALOGIC) {
                type = ANALOGIC;
            }
            io = new IOInterface(testRequest.message.createIO.pin, READ_WRITE, type);
            sendOkTestResponse(testRequest.id);
        } else {
            sendErrorTestResponse(testRequest.id, "TestIO already set with that pin");
        }
    } else if (testRequest.which_message == _TestRequest_setIOValue_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.setIOValue.pin);
        if (io == NULL) {
            sendErrorTestResponse(testRequest.id, "TestIO with that pin does not exist");
        } else {
            io->write(testRequest.message.setIOValue.value);
            sendOkTestResponse(testRequest.id);
        }
    } else if (testRequest.which_message == _TestRequest_getIOValue_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.getIOValue.pin);
        if (io == NULL) {
            sendErrorTestResponse(testRequest.id, "TestIO with that pin does not exist");
        } else {
            testResponse.has_message = true;
            testResponse.message.which_value = _TestResponseValue_intValue_tag;
            testResponse.message.value.intValue = io->read(); 
            sendOkTestResponse(testRequest.id);
        }
    } else if (testRequest.which_message == _TestRequest_clearIOs_tag) {
        IOInterface::removeAll();
        sendOkTestResponse(testRequest.id);
    } else if (testRequest.which_message == _TestRequest_freeMemory_tag) {
        sendOkTestResponse(testRequest.id, freeMemory());
    } else if (testRequest.which_message == _TestRequest_setClockOffset_tag) {
        Clock::setClockOffset(testRequest.message.setClockOffset.value);
        sendOkTestResponse(testRequest.id);
    } else if (testRequest.which_message == _TestRequest_getMillis_tag) {
        testResponse.has_message = true;
        testResponse.message.which_value = _TestResponseValue_uintValue_tag;
        testResponse.message.value.uintValue = Clock::currentMillis(); 
        sendOkTestResponse(testRequest.id);
    } else if (testRequest.which_message == _TestRequest_resetClock_tag) {
        Clock::setClockOffset(0);
        sendOkTestResponse(testRequest.id);
    }
}
#endif

void loop() {
    if (apiSerial->available()) {
        if (messageType == 0) {
            messageType = apiSerial->read();
        } else if (messageLengthBufferReadIndex < 2) {
            messageLengthBuffer[messageLengthBufferReadIndex] = apiSerial->read();
            messageLengthBufferReadIndex += 1;
            if (messageLengthBufferReadIndex == 2) {
                messageLength = *((unsigned int*) &messageLengthBuffer);
                if (messageLength > MAX_MESSAGE_SIZE) {
                    sendErrorResponse(0, "Invalid message");
                    freeRequestBuffer();
                }
            }
        } else if (read < messageLength) {
            requestBuffer[read] = apiSerial->read();
            read += 1;
        }
        readerTimer->startTimer();
    }

    if (read != 0 && read == messageLength) {
        requestStream = pb_istream_from_buffer(requestBuffer, messageLength);
        if (messageType == 1) {
            if(!pb_decode(&requestStream, Request_fields, &request)) {
                sendErrorResponse(0, "Failed to decode the request");
            } else {
                handleAPIRequest();
            }
        }
        #ifdef TEST
        else if (messageType == 2) {
            if(!pb_decode(&requestStream, _TestRequest_fields, &testRequest)) {
                sendErrorTestResponse(0, "Failed to decode the request");
            } else {
                handleTestRequest();
            }
        }
        #endif
        else {
            sendErrorResponse(0, "Invalid message type");
        }

        freeRequestBuffer();
        freeResponseBuffer();
    } else if (messageType != 0 && readerTimer->getElapsedTime() >= READ_TIMEOUT) {
        sendErrorResponse(0, "Truncated message received");
        freeRequestBuffer();
        freeResponseBuffer();
    }
  
    api->loop();

    if (Exception::hasException()) {
        Exception* exception = (Exception*) Exception::popException();
        char* exceptionArg = Exception::popExceptionArg();
        sendErrorResponse(0, (const RuntimeError*) exception, exceptionArg);
    }
}
