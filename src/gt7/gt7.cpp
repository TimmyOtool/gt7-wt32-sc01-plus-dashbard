#include "gt7.h"

gt7::gt7()
{
}


String convertTime(int32_t toConvert)
{
    int minutes = toConvert / 60000;
    int seconds = (toConvert % 60000) / 1000;
    int milliseconds = toConvert % 1000;

    char formattedTime[12];
    sprintf(formattedTime, "%02d:%02d:%03d", minutes, seconds, milliseconds);
    return String(formattedTime);
}

void gt7::saveLap(int lapNum)
{
    if (laps.size() >= maxLapHistory)
    {
        laps.erase(laps.begin());
    }
    int size = laps.size();
    float prevFuel = 100;
    float usedFuel = 0;
    if (size > 0)
    {
        prevFuel = laps[size - 1].fuelLevel;
        usedFuel = prevFuel - fuel;
    }
    // TODO used calculated fuelUsed
    lap l(lapNum, packetContent.packetContent.lastLaptime, fuel, usedFuel);
    laps.push_back(l);
}

void gt7::read()
{
    speed = ((int)((packetContent.packetContent.speed) * 3.6));
    gear = packetContent.packetContent.gears & 0b00001111;
    if (atoi(gear.c_str()) == 0)
    {
        gear = "N";
    }
    lastLapTime = convertTime(packetContent.packetContent.lastLaptime);

    bestLapTime = convertTime(packetContent.packetContent.bestLaptime);
    // currentLapTime = convertTime(packetContent.packetContent.dayProgression);
    tyreTemperatureFrontLeft = packetContent.packetContent.tyreTemp[0];
    tyreTemperatureFrontRight = packetContent.packetContent.tyreTemp[1];
    tyreTemperatureRearLeft = packetContent.packetContent.tyreTemp[2];
    tyreTemperatureRearRight = packetContent.packetContent.tyreTemp[3];
    fuel = packetContent.packetContent.fuelLevel;
    lapCount = packetContent.packetContent.lapCount;
    oilPressure = packetContent.packetContent.oilPressure;
    oilTemp = packetContent.packetContent.oilTemp;
    waterTemp = packetContent.packetContent.waterTemp;
    BodyHeight = packetContent.packetContent.bodyHeight;

    tyreRadiusFrontLeft = packetContent.packetContent.tyreRadius[0];
    tyreRadiusFrontRight = packetContent.packetContent.tyreRadius[1];
    tyreRadiusRearLeft = packetContent.packetContent.tyreRadius[2];
    tyreRadiusRearRight = packetContent.packetContent.tyreRadius[3];

    suspHeightFrontLeft = packetContent.packetContent.suspHeight[0];
    suspHeightFrontRight = packetContent.packetContent.suspHeight[1];
    suspHeightRearLeft = packetContent.packetContent.suspHeight[2];
    suspHeightRearRight = packetContent.packetContent.suspHeight[3];
    maxSpeed = packetContent.packetContent.calcMaxSpeed;

    if (lapCount > 0)
    {
        unsigned long dt_now = millis();
        // its a new lap!
        if (lapCount != prevlap)
        {
            saveLap(prevlap);
            updateLaps = true;
            prevlap = lapCount;

            dt_start = dt_now;
        }

        currentLapTime = convertTime(dt_now - dt_start);
    }
    else
    {
        currentLapTime = "00:00:000";
    }

    totalLaps = packetContent.packetContent.totalLaps;

    RaceStartPosition = packetContent.packetContent.RaceStartPosition;
    preRaceNumCars = packetContent.packetContent.preRaceNumCars;

    engineRpm = packetContent.packetContent.EngineRPM;
    maxAlertRPM = packetContent.packetContent.maxAlertRPM;
    minAlertRPM = packetContent.packetContent.minAlertRPM;

    brake = packetContent.packetContent.brake;
    throttle = packetContent.packetContent.throttle;
}