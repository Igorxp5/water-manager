#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

String command;
char b;
void loop() {
  if (Serial.available()){
    b = (char) Serial.read();
    if (b == 0) {
      Serial.print("GOT: ");
      Serial.println(command);
      command = "";
    } else {
      command += b;
    }
  }
}
