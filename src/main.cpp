#include <Arduino.h>
#include <pb_decode.h>
#include <pb_common.h>

#include "API.h"
#include "api.pb.c"

const unsigned int READ_TIMEOUT = 2500; //Miliseconds

API* api;
HardwareSerial* apiSerial = &Serial;

void setup() {
  apiSerial->begin(9600);
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
*/

byte* buffer;
byte messageType = 0;
unsigned int read = 0;
unsigned int messageLength = 0;

unsigned long lastReadTime;

Request request = Request_init_zero;
pb_istream_t stream;

void freeMessageBuffer() {
  read = 0;
  messageType = 0;
  messageLength = 0;
  free(buffer);
}

void loop() {
  if (apiSerial->available()) {
    if (messageType == 0) {
      messageType = apiSerial->read();
      lastReadTime = millis();
    } else if (buffer == NULL) {
      messageLength = (unsigned int) apiSerial->parseInt();
      buffer = (byte*) malloc(messageLength);
      lastReadTime = millis();
    } else if (read < messageLength) {
      buffer[read] = apiSerial->read();
      lastReadTime = millis();
      read += 1;
    }
  }
  
  if (read == messageLength) {
    stream = pb_istream_from_buffer(buffer, messageLength);
    if (messageType == 1) {
      if(!pb_decode(&stream, Request_fields, &request)) {
        //TODO: Handle Failed to decode message
      } else {
        
      }
    }
    #ifdef TEST
    else if (messageType == 2) {

    }
    #endif
    
    freeMessageBuffer();
  }
  
  if (lastReadTime > millis()) {
    //Long Overflow
    lastReadTime = millis(); //Give more time to read the data
  }
  
  if (millis() - lastReadTime >= READ_TIMEOUT) {
    freeMessageBuffer();
  }
  
  api->managerLoop();
}
