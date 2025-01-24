// File: simpleserialprint.ino
#include <FS.h>
#include <Arduino.h>
// #include <WiFi.h>
#include <WiFiManager.h>

#include <GT7UDPParser.h>
#include "display.h"

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>

Display display;

String defaultIp="192.168.0.59";
IPAddress ip; // Insert your PS4/5 IP address here
char ps5ipchar[16];

unsigned long previousT = 0;
const long interval = 500;
bool shouldSaveConfig = false;

GT7_UDP_Parser gt7Telem;
Packet packetContent;

WiFiManagerParameter ps5ip("ps5ip", "playstation5 ip", defaultIp.c_str(), 16);

void configModeCallback(WiFiManager *myWiFiManager)
{
  tft.clear();
  tft.println("Entered config mode");
  tft.println(WiFi.softAPIP());

  tft.println(myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback()
{
  tft.println("Should save config");
  shouldSaveConfig = true;
}

void readFs()
{

  if (SPIFFS.begin())
  {
    tft.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      // file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if (!deserializeError)
        {
          strcpy(ps5ipchar, json["ps5ip"]);
          
        }
        else
        {
          tft.println("failed to load json config");
        }
        configFile.close();
      }
    }else {
      //createEmptyCOnfigFIle();
    }
  }
  else
  {
    tft.println("failed to mount FS");
  }

  String theip=String(ps5ipchar);
  ip.fromString(theip);
  //sleep(1);

}

void setup()
{
  display.setup();
  tft.clear();
    tft.pushImage(0, 0, 480, 320, image_data_480x320x16);
  Serial.begin(115200);
  readFs();
  WiFiManager wm;
  //wm.resetSettings();
  wm.setAPCallback(configModeCallback);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.addParameter(&ps5ip);
  bool res;

  res = wm.autoConnect("Gt7-Dashboard");
  // ip.fromString(ps5ip.getValue());
  //sleep(2);
  tft.print("Waiting GT7 connexion at IP: ");
  tft.print(ip);
  sleep(1);
  gt7Telem.begin(ip);
  gt7Telem.sendHeartbeat();
  tft.clear();
}

void loop()
{
  unsigned long currentT = millis();
  packetContent = gt7Telem.readData();
  display.packetContent = packetContent;
  display.read();
  display.loop();

  if (currentT - previousT >= interval)
  {
    previousT = currentT;
    gt7Telem.sendHeartbeat();
  }
}
