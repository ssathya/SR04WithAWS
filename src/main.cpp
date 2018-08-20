//includes
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>

//defines
#define OnBoardLED 2

//variables
bool ledStatus;
String ssid;
String password;
//Used by application.
void BeginSPIFFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount file system");
        return;
    }
    Serial.println("File system is working");
}
void EndSPIFFS()
{
    Serial.println("Closing filesystem");
    SPIFFS.end();
}

void ReadWiFiConfig()
{
    if (!SPIFFS.exists("/WiFi.json"))
    {
        Serial.println("WiFi configuration not found; Nothing usful will be done!");
        return;
    }
    File f = SPIFFS.open("/WiFi.json", "r");
    if (!f)
    {
        Serial.println("WiFi configuration file not readable. End of story");
        return;
    }
    int fileSize = f.size();
    if (fileSize >= 400)
    {
        Serial.println("Did not expect WiFi.json larger than 400 bytes; End of stroy.");
        return;
    }
    Serial.printf("\nWifi.json file size = %d\n", fileSize);
    uint8_t buffer[400];
    int bytesRead = f.read(buffer, 400);
    buffer[bytesRead] = 0;
    String fileContent = (char *)buffer;
    DynamicJsonBuffer jsonBuffer(512);
    JsonObject &root = jsonBuffer.parse(fileContent, 2);
    ssid = (const char *)root["ssid"];
    password = (const char *)root["password"];
    //Serial.printf ("File values %s, %s\n", ssid.c_str(), password.c_str());
    return;
}
void SetupWiFiConnection()
{
    ReadWiFiConfig();
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
}
//Arduion methods
void setup()
{
    // put your setup code here, to run once:
    pinMode(OnBoardLED, OUTPUT);

    Serial.begin(115200);
    delay(2000);
    while (!Serial)
        ;
    Serial.println();
    BeginSPIFFS();
    SetupWiFiConnection();

    EndSPIFFS();
}

void loop()
{
    // put your main code here, to run repeatedly:
    ledStatus = ledStatus == true ? false : true;
    digitalWrite(OnBoardLED, ledStatus == true ? HIGH : LOW);
    delay(1000);
}