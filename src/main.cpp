
#include <Arduino.h>
#include <WiFiManager.h>

#include <GT7UDPParser.h>
#include "display.h"
#include "gt7connect.h"

Display display;

IPAddress ip(0, 0, 0, 0);

unsigned long previousT = 0;
const long interval = 500;

GT7_UDP_Parser gt7Telem;
Packet packetContent;

void configModeCallback(WiFiManager *myWiFiManager)
{
  tft.clear();

  if (SPIFFS.begin())
  {
    File wifi = SPIFFS.open("/wifi.png", FILE_READ);
    tft.drawPng(&wifi, (SCREEN_WIDTH / 2) +30 , (SCREEN_HEIGHT / 2) - 50, 160, 90, 0, 0, 1);
    wifi.close();
  }

  tft.setCursor(0, (SCREEN_HEIGHT / 2 )-30);
  tft.setTextSize(2);
  tft.println("Entering config mode");
  tft.println(WiFi.softAPIP());

  tft.println(myWiFiManager->getConfigPortalSSID());
}

void setup()
{
  display.setup();
  Serial.begin(115200);
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  bool res;

  res = wm.autoConnect("Gt7-Dashboard");
  tft.clear();
  if (SPIFFS.begin())
  {
    File gt7 = SPIFFS.open("/gt7.jpg", FILE_READ);
    File bsr = SPIFFS.open("/LogoBsr.png", FILE_READ);
    //File azerty = SPIFFS.open("/azertyTV.png", FILE_READ);
    tft.drawJpg(&gt7, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1);
   // tft.drawPng(&azerty, 10 , 130, 150, 150, 0, 0, 1);
    tft.drawPng(&bsr, (SCREEN_WIDTH / 2) +40 , 130, 216, 216, 0, 0, 1);
    gt7.close();
    bsr.close();
    //azerty.close();

  }
  tft.setTextSize(1);
  tft.fillRect(0, 0, SCREEN_WIDTH, 30, TFT_RED);

  bool ps5ok = false;
  // read old ps5 ip
  IPAddress fromJson = readFs();
  IPAddress psFind;
  if (fromJson.toString().compareTo("0.0.0.0") != 0)
  {
    ps5ok = false; // set to true to bypass during dev
    for (int i = 0; i < 5 && !ps5ok; i++)
    {

      tft.fillRect(0, 0, SCREEN_WIDTH, 30, TFT_RED);
      tft.setTextColor(TFT_WHITE);
      tft.drawCenterString("check GT7 at " + fromJson.toString(), SCREEN_WIDTH / 2, 10, &fonts::DejaVu12);
      ps5ok = checkGT7(fromJson);
      ip = fromJson;
      sleep(1);
    }
  }
  if (!ps5ok)
  {
    tft.fillRect(0, 0, SCREEN_WIDTH, 30, TFT_RED);
      tft.setTextColor(TFT_WHITE);
      tft.drawCenterString("GT7 not found, start looking", SCREEN_WIDTH / 2, 10, &fonts::DejaVu12);
      sleep(1);
    do
    {
      psFind = discoverGT7();
    } while (psFind.toString().compareTo("0.0.0.0") == 0);
    // gt7found
    tft.fillRect(0, 0, SCREEN_WIDTH, 30, TFT_DARKGREEN);
    tft.setTextColor(TFT_BLACK);
    tft.drawCenterString("GT7 found at " + psFind.toString(), SCREEN_WIDTH / 2, 10, &fonts::DejaVu12);
    saveFS(psFind);
    ip = psFind;
    sleep(1);
  }

  tft.fillRect(0, 0, SCREEN_WIDTH, 30, TFT_DARKGREEN);
  tft.setTextColor(TFT_BLACK);
  tft.drawCenterString("Connected to GT7 at " + ip.toString(), SCREEN_WIDTH / 2, 10, &fonts::DejaVu12);
  sleep(3);
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
