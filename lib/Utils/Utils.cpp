#include "Utils.h"

void Utils::sendDebugResponse(String message) {
    Serial.write((byte) 3); //Debug message type
    Serial.println(message);
    Serial.flush();
}

void Utils::sendDebugResponse(char* message) {
    Serial.write((byte) 3); //Debug message type
    Serial.println(message);
    Serial.flush();
}
