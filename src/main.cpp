#include <Arduino.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb_common.h>

#include "API.h"
#include "IOInterface.h"
#include "api.pb.c"

#ifdef TEST
#include "test.pb.c"
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

void sendErrorResponse(const char* error) {
    response.error = true;
    response.has_message = true;
    response.message.which_value = Value_stringValue_tag;
    strcpy(response.message.value.stringValue, error);
    sendResponse();
}

void sendOkResponse(unsigned int requestId) {
    response.id = requestId;
    sendResponse();
}

void handleAPIRequest() {
    sendOkResponse(request.id);
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

void sendErrorTestResponse(const char* error) {
    testResponse.error = true;
    testResponse.has_message = true;
    testResponse.message.which_value = Value_stringValue_tag;
    strcpy(testResponse.message.value.stringValue, error);
    sendTestResponse();
}

void sendOkTestResponse(unsigned int requestId) {
    testResponse.id = requestId;
    sendTestResponse();
}

void handleTestRequest() {
    if (testRequest.which_message == _TestRequest_createIO_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.createIO.pin);
        if (io == NULL) {
            io = new TestIO(testRequest.message.createIO.pin);    
            sendOkTestResponse(testRequest.id);
        } else {
            testResponse.id = testRequest.id;
            sendErrorTestResponse("TestIO already set with that pin");
        }
    } else if (testRequest.which_message == _TestRequest_setIOValue_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.setIOValue.pin);
        if (io == NULL) {
            testResponse.id = testRequest.id;
            sendErrorTestResponse("TestIO with that pin does not exist");
        } else {
            io->write(testRequest.message.setIOValue.value);
            sendOkTestResponse(testRequest.id);
        }
    } else if (testRequest.which_message == _TestRequest_getIOValue_tag) {
        IOInterface* io = IOInterface::get(testRequest.message.getIOValue.pin);
        if (io == NULL) {
            testResponse.id = testRequest.id;
            sendErrorTestResponse("TestIO with that pin does not exist");
        } else {
            testResponse.has_message = true;
            testResponse.message.which_value = _TestResponseValue_intValue_tag;
            testResponse.message.value.intValue = io->read(); 
            sendOkTestResponse(testRequest.id);
        }
    } else if (testRequest.which_message == _TestRequest_clearIOs_tag) {
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
                sendErrorResponse("Failed to decode the request");
            } else {
                handleAPIRequest();
            }
        }
        #ifdef TEST
        else if (messageType == 2) {
            if(!pb_decode(&requestStream, _TestRequest_fields, &testRequest)) {
                sendErrorTestResponse("Failed to decode the request");
            } else {
                handleTestRequest();
            }
        }
        #endif

        freeRequestBuffer();
        freeResponseBuffer();
    } else if (messageType != 0 && millis() - lastReadTime >= READ_TIMEOUT) {
        sendErrorResponse("Truncated message received");
        freeRequestBuffer();
        freeResponseBuffer();
    }
  
    api->managerLoop();
}
