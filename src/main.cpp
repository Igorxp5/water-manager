#include <Arduino.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_common.h>

#include "API.h"
#include "IOInterface.h"
#include "api.pb.c"

#include "Utils.h"

#ifdef TEST
#include "test.pb.c"
#include "MemoryFree.h"
#endif

const unsigned int READ_TIMEOUT = 2500; //Miliseconds

API* api;
HardwareSerial* apiSerial = &Serial;

void setup() {
    apiSerial->begin(9600);
    apiSerial->setTimeout(READ_TIMEOUT);
    api = new API();
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

byte* buffer;
byte responseBuffer[Response_size];
byte messageType = 0;
unsigned int read = 0;

byte messageLengthBuffer[2] = {0, 0};
byte messageLengthBufferReadIndex = 0;
unsigned int messageLength;

unsigned long lastReadTime;

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
    free(buffer);
    buffer = NULL;
}

void freeResponseBuffer() {
    response = Response_init_zero;

    #ifdef TEST
    testResponse = _TestResponse_init_zero;
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

void sendErrorResponse(unsigned int requestId, const char* error) {
    response.has_error = true;
    response.error = Response_Exception_EXCEPTION;
    response.has_message = true;
    response.message.has_value = true;
    response.message.value.which_content = PrimitiveValue_stringValue_tag;
    strcpy(response.message.value.content.stringValue, error);
    sendResponse();
}

void sendErrorResponse(unsigned int requestId, const RuntimeError* error) {
    response.has_error = true;
    response.error = Response_Exception_RUNTIME_ERROR;
    response.has_message = true;
    response.message.has_value = true;
    response.message.value.which_content = PrimitiveValue_stringValue_tag;
    const char* message = ((Exception*) error)->getMessage();
    strcpy(response.message.value.content.stringValue, message);
    sendResponse();
}

void sendErrorResponse(unsigned int requestId, const InvalidRequest* error) {
    response.has_error = true;
    response.error = Response_Exception_INVALID_REQUEST;
    response.has_message = true;
    response.message.has_value = true;
    response.message.value.which_content = PrimitiveValue_stringValue_tag;
    const char* message = ((Exception*) error)->getMessage();
    strcpy(response.message.value.content.stringValue, message);
    sendResponse();
}

void sendOkResponse(unsigned int requestId) {
    response.id = requestId;
    sendResponse();
}

void handleAPIRequest() {
    if (request.which_message == Request_createWaterSource_tag) {
        api->createWaterSource(request.message.createWaterSource.name, request.message.createWaterSource.pin);
        sendOkResponse(request.id);
    } else if (request.which_message == Request_getWaterSourceList_tag) {
        char** waterSourceList = api->getWaterSourceList();
        unsigned int totalWaterSources = api->getTotalWaterSources();
        response.has_message = true;
        response.message.listValue_count = totalWaterSources;
        PrimitiveValue value = PrimitiveValue_init_zero;
        for (unsigned int i = 0; i < totalWaterSources; i++) {
            value.which_content = PrimitiveValue_stringValue_tag;
            strcpy(value.content.stringValue, waterSourceList[i]);
            response.message.listValue[i] = value;
        }
        sendOkResponse(request.id);
        free(waterSourceList);
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
    strcpy(testResponse.message.value.stringValue, error);
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
            io = new TestIO(testRequest.message.createIO.pin);    
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
    } else if (testRequest.which_message == _TestRequest_memoryFree_tag) {
        sendOkTestResponse(testRequest.id, freeMemory());
    } else if (testRequest.which_message == _TestRequest_resetAPI_tag) {
        api->reset();
        IOInterface::removeAll();
        sendOkTestResponse(testRequest.id);
    }
}
#endif

void loop() {
    if (apiSerial->available()) {
        if (messageType == 0) {
            messageType = apiSerial->read();
        } else if (buffer == NULL) {
            messageLengthBuffer[messageLengthBufferReadIndex] = apiSerial->read();
            messageLengthBufferReadIndex += 1;
            if (messageLengthBufferReadIndex == 2) {
                messageLength = *((unsigned int*) &messageLengthBuffer);
                buffer = (byte*) malloc(messageLength);
            }
        } else if (read < messageLength) {
            buffer[read] = apiSerial->read();
            read += 1;
        }
        lastReadTime = millis();
    }

    if (lastReadTime > millis()) {
        //Long Overflow
        lastReadTime = millis(); //Give more time to read the data
    }

    if (read != 0 && read == messageLength) {
        requestStream = pb_istream_from_buffer(buffer, messageLength);
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

        freeRequestBuffer();
        freeResponseBuffer();
    } else if (messageType != 0 && millis() - lastReadTime >= READ_TIMEOUT) {
        sendErrorResponse(0, "Truncated message received");
        freeRequestBuffer();
        freeResponseBuffer();
    }
  
    api->managerLoop();

    //TODO: Check RuntimeErrors and send them
}
