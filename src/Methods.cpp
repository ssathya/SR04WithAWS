//includes
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Ultrasonic.h>
#include <PubSubClient.h>
#include <string.h>

//defines
#define OnBoardLED 2
#define TRIG_PIN 16
#define ECHO_PIN 0
#define MillisecondsInADay 86400000

//variables
extern bool ledStatus;
extern String ssid;
extern String password;
extern String output;
extern String publishTopic;
extern String readTopic;
extern String deviceName;

extern const char *AWS_endpoint;
extern long lastMsg;
extern long lastBlinkStatusChange;

extern Ultrasonic us;
extern WiFiClientSecure espClient;
extern PubSubClient client;

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
    if (ssid == NULL || ssid.equals(""))
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
        f.close();
    }
    return;
}
void SetupWiFiConnection()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }
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

void ReconnectToAWS()
{
    while (!client.connected())
    {
        SetupWiFiConnection();
        Serial.println("Attempting MQTT Connection");
        if (client.connect("Distance"))
        {
            Serial.println("connected");
            client.publish(publishTopic.c_str(), output.c_str());
            client.subscribe(readTopic.c_str());
        }
        else
        {
            Serial.print("Failed to connect to AWS");
            Serial.println(client.state());
            Serial.println("Retrying after 5 seconds");
            delay(5000);
        }
    }
}

void ProcessCertificatesAndKey()
{
    File cert = SPIFFS.open("/cert.der", "r");
    if (!cert)
    {
        Serial.println("Failed to open cert file");
        return;
    }
    delay(1000);
    if (!espClient.loadCertificate(cert))
    {
        Serial.println("Certificate not loade");
        return;
    }
    File privateKey = SPIFFS.open("/private.der", "r");
    if (!privateKey)
    {
        Serial.println("Failed to open private file");
        return;
    }
    delay(1000);
    if (!espClient.loadPrivateKey(privateKey))
    {
        Serial.println("Private key not loade");
        return;
    }
    Serial.println("Keys loaded!");
    Serial.print("Heap: ");
    Serial.println(ESP.getFreeHeap());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    if (strstr(topic, "DeviceName"))
    {
        DynamicJsonBuffer jsonBuffer(512);
        JsonObject &root = jsonBuffer.parse((const char *)payload);
        if (root.success())
        {
            // /distanceToIoT/DeviceName
            /*
            {
                "deviceName": "FirstDevice"
            }
            */
           const char *a = root["deviceName"];
           Serial.printf("deviceName sent was %s\n", a);
            String outObj;
            root.prettyPrintTo(outObj);
            Serial.println(outObj);            
            
            deviceName = a;
        }
    }
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}
void MeasureUltrasound()
{
    long now = millis();
    if (now < lastMsg) //handle rollover
    {
        lastMsg = now;
    }
    if (now - lastMsg < 60000)
    {
        return;
    }
    lastMsg = now;
    Serial.println("Measuring values from sensor:hc-sr04");
    us.measure();
    float valuse_in_cm = us.get_cm();
    float value_in_inch = valuse_in_cm / 2.54;

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["distance"] = value_in_inch;
    root["deviceName"] = deviceName;
    output = "";
    root.printTo(output);
    ReconnectToAWS();
    if (!client.publish(publishTopic.c_str(), output.c_str()))
    {
        Serial.println("Publish returned false");
    }
    //Comment out this when not needed
    Serial.println(output);
}

void BlinkLED()
{
    long now = millis();
    if (now < lastBlinkStatusChange)
    {
        lastBlinkStatusChange = now;
    }
    if (now - lastBlinkStatusChange < 1000)
    {
        return;
    }
    lastBlinkStatusChange = now;
    ledStatus = ledStatus == true ? false : true;
    digitalWrite(OnBoardLED, ledStatus == true ? HIGH : LOW);
}
void CheckIfRebootNeeded()
{
    long now = millis();
    if (now >= MillisecondsInADay)
    {
        ESP.restart();
    }
}