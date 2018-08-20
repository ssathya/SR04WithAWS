//includes
#include <Arduino.h>
#include <ESP8266WiFi.h>

//defines
#define OnBoardLED 2

//variables
bool ledStatus;
void setup() {
    // put your setup code here, to run once:
    pinMode(OnBoardLED, OUTPUT);
}

void loop() {
    // put your main code here, to run repeatedly:
    ledStatus = ledStatus == true ? false : true;
    digitalWrite(OnBoardLED, ledStatus == true ? HIGH : LOW);
    delay(1000);
}