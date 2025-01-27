#include <FS.h>
#include <Arduino.h>
#include <WiFiManager.h>

#include <GT7UDPParser.h>
#include "display.h"

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <ArduinoJson.h>

Display display;

IPAddress ip(0, 0, 0, 0);
char ps5ipchar[16];
unsigned long previousT = 0;
const long interval = 500;

GT7_UDP_Parser gt7Telem;
Packet packetContent;

constexpr char heartbeatMsg = 'A';

IPAddress getBroadcastAddress(IPAddress ip, IPAddress subnet)
{
  IPAddress broadcast;
  for (int i = 0; i < 4; i++)
  {
    broadcast[i] = ip[i] | ~subnet[i];
  }
  return broadcast;
}

boolean discoverGT7()
{

  WiFiUDP udp;
  int localPort = 33740;

  int targetPort = 33739;
  int timeout = 200;
  IPAddress localIP = WiFi.localIP();
  IPAddress subnetMask = WiFi.subnetMask();
  IPAddress broadcastIP = getBroadcastAddress(localIP, subnetMask);

  udp.begin(localPort);
  int maxip = 255;
  for (int i = 1; i < maxip; i++)
  {
    IPAddress targetIP = localIP;
    targetIP[3] = i;

    udp.beginPacket(targetIP, targetPort);
    udp.write(heartbeatMsg);
    udp.endPacket();
    unsigned long startTime = millis();
    tft.clear();
    tft.setCursor(10, 10);
    tft.println("try: " + targetIP.toString());
    while (millis() - startTime < timeout)
    {
      int packetSize = udp.parsePacket();
      if (packetSize > 0)
      {
        char response[packetSize];
        udp.read(response, packetSize);
        response[packetSize] = '\0';
        tft.printf("PS5 Foung : %s\n", targetIP.toString().c_str());
        ip = targetIP;
        udp.stop();
        return true;
      }
    }
  }
  udp.stop();
  tft.println("PS5 not found");
  return false;
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  tft.clear();
  tft.println("Entered config mode");
  tft.println(WiFi.softAPIP());

  tft.println(myWiFiManager->getConfigPortalSSID());
}

void saveFS() {
 if (SPIFFS.begin())
  {
      File configFile = SPIFFS.open("/config.json", FILE_WRITE);
      DynamicJsonDocument json(1024);
     // serializeJson()
  }

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
          ip.fromString(String(ps5ipchar));
        }
        else
        {
          tft.println("failed to load json config");
        }
        configFile.close();
      }
    }
    else
    {
      // File configFile = SPIFFS.open("/config.json", "w");
      // DynamicJsonDocument json(1024);
      // configFile.w
    }
  }
  else
  {
    tft.println("failed to mount FS");
  }

  // sleep(1);
}
void setup()
{
  display.setup();
  tft.clear();
  tft.pushImage(0, 0, 480, 320, image_data_480x320x16);
  Serial.begin(115200);

  WiFiManager wm;
  // wm.resetSettings();
  wm.setAPCallback(configModeCallback);
  bool res;

  res = wm.autoConnect("Gt7-Dashboard");

  bool ps5ok = false;
  // read old ps5 ip
  readFs();
  if (ip.toString().compareTo("0.0.0.0") != 0)
  {
    // TODO try ps5 access;
    ps5ok = true;
  }
  if (!ps5ok)
  {
    // searching ps5
    do
    {
      tft.clear();
      tft.println("search gt7");
    } while (!discoverGT7());
    // ps5found
    saveFS();
  }

  tft.print("Waiting GT7 connexion at IP: ");
  tft.print(ip);
  sleep(2);
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
