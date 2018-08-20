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
void callback(char* topic, byte* payload, unsigned int length);
//variables
bool ledStatus;
String ssid;
String password;
String output;
const char* AWS_endpoint = "a2oe8lf2wwvqnz.iot.us-east-2.amazonaws.com"; //MQTT broker ip


Ultrasonic us(TRIG_PIN, ECHO_PIN);
WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); 

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
    f.close();
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
    File privateKey = SPIFFS.open("/private.der","r");
    if (!privateKey)
    {
        Serial.println("Failed to open private file");
        return;
    }
    delay(1000);
    if(!espClient.loadPrivateKey(privateKey))
    {
        Serial.println("Private key not loade");
        return; 
    }
    Serial.println("Keys loaded!");
    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

//Arduino Specific
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
    ProcessCertificatesAndKey();
    EndSPIFFS();
}
void MeasureUltrasound()
{
    us.measure();
    float valuse_in_cm = us.get_cm();
    float value_in_inch = valuse_in_cm / 2.54;
    //Serial.println(valuse_in_cm, 3);
    /*Serial.print("Value in Inch ");
    Serial.println(value_in_inch, 3);*/ 
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["distance"] = value_in_inch;    
    output = "";
    root.printTo(output);
    //Serial.println(output);
}

//Arduion methods
void loop()
{
    // put your main code here, to run repeatedly:
    MeasureUltrasound();
    
    ledStatus = ledStatus == true ? false : true;
    digitalWrite(OnBoardLED, ledStatus == true ? HIGH : LOW);
    delay(1000);

    //Comment out this when not needed
    Serial.println(output);
}