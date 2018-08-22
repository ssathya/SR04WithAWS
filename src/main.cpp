//includes
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Ultrasonic.h>
#include <PubSubClient.h>

//defines
#define OnBoardLED 2
#define TRIG_PIN 16
#define ECHO_PIN 0
#define MillisecondsInADay 86400000

void callback(char *topic, byte *payload, unsigned int length);
//variables
bool ledStatus;
String ssid;
String password;
String output;
String publishTopic;
String readTopic;
String deviceName;

const char *AWS_endpoint = "a2oe8lf2wwvqnz.iot.us-east-2.amazonaws.com"; //MQTT broker ip
long lastMsg = 0;
long lastBlinkStatusChange = 0;

Ultrasonic us(TRIG_PIN, ECHO_PIN);
WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient);

//Used by application.
void BeginSPIFFS();
void EndSPIFFS();
void ReadWiFiConfig();
void SetupWiFiConnection();
void ProcessCertificatesAndKey();
void MeasureUltrasound();
void BlinkLED();
void ReconnectToAWS();
void CheckIfRebootNeeded();
//Arduion methods
void setup()
{
    //code for defaults; can be overwritten down below.
    publishTopic = "/distanceFromIoT";
    readTopic = "/distanceToIoT/#";
    deviceName = "NoNameYet";
    // put your setup code here, to run once:
    pinMode(OnBoardLED, OUTPUT);

    Serial.begin(115200);
    delay(2000);
    while (!Serial)
        ;
    Serial.println();
    BeginSPIFFS();
    SetupWiFiConnection();
    ProcessCertificatesAndKey();
    EndSPIFFS();
}

void loop()
{
    // put your main code here, to run repeatedly:
    MeasureUltrasound();
    BlinkLED();
    if (!client.loop())
    {
        ReconnectToAWS();
    }
    CheckIfRebootNeeded();
}