#ifndef GT7_H
#define GT7_H

#include "lap/lap.h"
#include <GT7UDPParser.h>
#include <map>
#include <vector>

class gt7
{
private:
    Packet packetContent;
    std::vector<lap> laps;
    int maxLapHistory = 10;
    int prevlap = 0;
    bool updateLaps = false;
    unsigned long dt_start = 0;
    // int lapNumber = 0;
    // int lapTime = 0;
    // int32_t bestLapTime = 0;
    // int32_t lastLapTime = 0;
    // int32_t lapStartTime = 0;
    // int32_t lapEndTime = 0;
    // int32_t lapTimeDiff = 0;
    // int32_t lapTimeDiff2 = 0;
    int engineRpm = 50;
    int maxAlertRPM = 90;
    int minAlertRPM = 70;
    String gear = "N";
    String prev_gear = "0";
    int speed = 0;
    String currentLapTime = "00:00.00";
    String lastLapTime = "00:00.00";
    String bestLapTime = "00.00.00";

    int tyreTemperatureFrontLeft = 0;
    int tyreTemperatureFrontRight = 0;
    int tyreTemperatureRearLeft = 0;
    int tyreTemperatureRearRight = 0;
    int lapCount = 0;
    int totalLaps = 0;
    int RaceStartPosition = 0;
    int preRaceNumCars = 0;
    float fuel = 0;
    int brake = 0;    // max 255
    int throttle = 0; // max 255
    int tireAlertTemp = 100;
    float tyreRadiusFrontLeft = 0;
    float tyreRadiusFrontRight = 0;
    float tyreRadiusRearLeft = 0;
    float tyreRadiusRearRight = 0;

    float suspHeightFrontLeft = 0;
    float suspHeightFrontRight = 0;
    float suspHeightRearLeft = 0;
    float suspHeightRearRight = 0;

    float oilPressure;
    float waterTemp;
    float oilTemp;
    float BodyHeight;
    float suspHeight[4];

    int maxSpeed = 0;

public:
    gt7();
    void saveLap(int lapNum);
    void read();
};

#endif