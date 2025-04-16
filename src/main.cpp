
#include <Arduino.h>
#include <WiFiManager.h>

#include <GT7UDPParser.h>
#include "display.h"
#include "console.h"

Display display;

IPAddress ip(0, 0, 0, 0);

unsigned long previousT = 0;
const long interval = 500;

GT7_UDP_Parser gt7Telem;
Packet packetContent;


void configModeCallback(WiFiManager *myWiFiManager)
{
  tft.clear();
  tft.println("Entered config mode");
  tft.println(WiFi.softAPIP());

  tft.println(myWiFiManager->getConfigPortalSSID());
}

void setup()
{
  display.setup();
  Serial.begin(115200);

  tft.clear();
  		if (SPIFFS.begin())
		{
			File gt7 = SPIFFS.open("/gt7.jpg", FILE_READ);
			tft.drawJpg(&gt7, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,0, 0, 1);
      gt7.close();
		}

  WiFiManager wm;
  // wm.resetSettings();
  wm.setAPCallback(configModeCallback);
  bool res;

  res = wm.autoConnect("Gt7-Dashboard");

  bool ps5ok = false;
  // read old ps5 ip
  IPAddress fromJson=readFs();
  IPAddress psFind;
  if (fromJson.toString().compareTo("0.0.0.0") != 0)
  {
    ps5ok = false;
    for(int i=0;i<10 && !ps5ok;i++){
      tft.println("check ps5 at"+fromJson.toString());
      ps5ok= checkConsole(fromJson);
      ip=fromJson;
      sleep(1);
    }
    
  }
  if (!ps5ok)
  {
    // searching ps5
    do
    {
      //tft.clear();
      tft.println("search gt7");
      psFind=discoverGT7();
    } while (psFind.toString().compareTo("0.0.0.0") == 0);
    // ps5found
     saveFS(psFind);
     ip=psFind;
  }


  //tft.pushImage(0, 0, 480, 320, image_data_480x320x16);
  tft.print("Waiting GT7 connexion at IP: ");
  tft.print(ip);
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


