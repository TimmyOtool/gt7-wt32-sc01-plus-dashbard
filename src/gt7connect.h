#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

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

IPAddress discoverGT7()
{
    IPAddress ip(0, 0, 0, 0);
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
        tft.fillRect(0,0, SCREEN_WIDTH, 30, TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.drawCenterString("try at "+targetIP.toString(), SCREEN_WIDTH/2, 10, &fonts::DejaVu12);
        while (millis() - startTime < timeout)
        {
            int packetSize = udp.parsePacket();
            if (packetSize > 0)
            {
                char response[packetSize];
                udp.read(response, packetSize);
                response[packetSize] = '\0';
                //tft.printf("PS5 Found : %s\n", targetIP.toString().c_str());
                udp.stop();
                return targetIP;
            }
        }
    }
    udp.stop();

    return ip;
}

bool checkGT7(IPAddress ip)
{
    WiFiUDP udp;
    int localPort = 33740;
    int targetPort = 33739;
    int timeout = 200;

    udp.begin(localPort);
    udp.beginPacket(ip, targetPort);
    udp.write(heartbeatMsg);
    udp.endPacket();
    unsigned long startTime = millis();
    while (millis() - startTime < timeout)
    {
        int packetSize = udp.parsePacket();
        if (packetSize > 0)
        {
            char response[packetSize];
            udp.read(response, packetSize);
            response[packetSize] = '\0';
            return true;
        }
    }
    return false;
}

void saveFS(IPAddress ip)
{
    if (SPIFFS.begin())
    {
        File configFile = SPIFFS.open("/config.json", FILE_WRITE);
        DynamicJsonDocument doc(1024);
        doc["ps5ip"] = ip.toString();
        serializeJson(doc, configFile);
        configFile.close();
    }
}

IPAddress readFs()
{
    IPAddress ip(0, 0, 0, 0);
    if (SPIFFS.begin())
    {
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
                    char ps5ipchar[16];
                    strcpy(ps5ipchar, json["ps5ip"]);
                    ip.fromString(String(ps5ipchar));
                    return ip;
                }
                configFile.close();
            }
        }
    }
    else
    {
        tft.println("failed to mount FS");
    }
    return ip;
}

#endif