
#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <conf_WT32SCO1-Plus.h>
//  dashboard + free deck grafica
#include "gt7.h"
#include "lap.h"
#include <Arduino.h>
#include <map>
#include <vector>
#include <iostream>

static LGFX tft;

static const int SCREEN_WIDTH = 480;
static const int SCREEN_HEIGHT = 320;
static const int X_CENTER = SCREEN_WIDTH / 2;
static const int Y_CENTER = SCREEN_HEIGHT / 2;
static const int ROWS = 5;
static const int COLS = 5;
static const int CELL_WIDTH = SCREEN_WIDTH / COLS;
static const int HALF_CELL_WIDTH = CELL_WIDTH / 2;
static const int CELL_HEIGHT = SCREEN_HEIGHT / ROWS;
static const int HALF_CELL_HEIGHT = CELL_HEIGHT / 2;
static const int COL[] = {0, CELL_WIDTH, CELL_WIDTH * 2, CELL_WIDTH * 3, CELL_WIDTH * 4, CELL_WIDTH * 6, CELL_WIDTH * 7};
static const int ROW[] = {0, CELL_HEIGHT, CELL_HEIGHT * 2, CELL_HEIGHT * 3, CELL_HEIGHT * 4, CELL_HEIGHT * 6, CELL_HEIGHT * 7};

std::map<String, String> prevData;
std::map<String, int32_t> prevColor;

int currentPage = 1; // Variabile per tenere traccia della pagina corrente
int totalPage = 4;
bool forceUpdate = false; // Variabile per forzare l'aggiornamento delle celle

unsigned long previousP = 0;
const long intervalP = 500;

unsigned long dt_start = 0;
int prevlap = 0;

int prev_rpmPercent = 0;
bool updateLaps = false;
std::vector<lap> laps;
int maxLapHistory = 9;

class Display
{
public:
	Packet packetContent;
	// Global variables
	int engineRpm = 50;
	// int prev_rpmPercent = 50;
	int maxAlertRPM = 90;
	int minAlertRPM = 70;
	String gear = "N";
	// std::string
	String prev_gear = "0";
	int speed = 0;
	String currentLapTime = "00:00.00";
	String lastLapTime = "00:00.00";
	String bestLapTime = "00.00.00";
	// String sessionBestLiveDeltaSeconds = "0.000";
	// String sessionBestLiveDeltaProgressSeconds = "0.00";

	int tyreTemperatureFrontLeft = 0;
	int tyreTemperatureFrontRight = 0;
	int tyreTemperatureRearLeft = 0;
	int tyreTemperatureRearRight = 0;
	int lapCount = 0;
	int totalLaps = 0;
	int RaceStartPosition = 0;
	int preRaceNumCars = 0;
	float fuel = 0;
	int brake = 0;	  // max 255
	int throttle = 0; // max 255
	int tireAlertTemp = 100;
	uint16_t touchX, touchY; // definisce i due interi per gestione touchscreen

public:
	void setup()
	{
		tft.init();
		tft.setRotation(3);
		tft.fillScreen(TFT_BLACK);
	}

	void saveLap(int lapNum)
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
		// TODO used calculated fuelSued
		lap l(lapNum, packetContent.packetContent.lastLaptime, fuel, usedFuel);
		laps.push_back(l);
	}

	// Called when new data is coming from computer
	void read()
	{
		speed = ((int)((packetContent.packetContent.speed) * 3.6));
		gear = packetContent.packetContent.gears & 0b00001111;
		if (atoi(gear.c_str()) == 0) // not working
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

	String convertTime(int32_t toConvert)
	{
		int minutes = toConvert / 60000;		  // Partie des minutes
		int seconds = (toConvert % 60000) / 1000; // Partie des secondes
		int milliseconds = toConvert % 1000;

		char formattedTime[12];
		sprintf(formattedTime, "%02d:%02d:%03d", minutes, seconds, milliseconds);
		return String(formattedTime);
	}
	// Called once per arduino loop, timing can't be predicted,
	// but it's called between each command sent to the arduino
	void loop()
	{
		static int lastPage = currentPage;
		if (currentPage != lastPage)
		{
			tft.fillScreen(TFT_BLACK);
			forceUpdate = true;
			lastPage = currentPage;
		}
		switch (currentPage)
		{
		case 1:
			drawPage1(forceUpdate);
			break;
		case 2:
			drawPage2();
			break;
		case 3:
			drawPage3();
			break;
		case 4:
			drawPage4();
			break;

		default:
			break;
		}

		forceUpdate = false;
		unsigned long currentP = millis();
		if (tft.getTouch(&touchX, &touchY))
		{
			if (currentP - previousP >= intervalP)
			{
				previousP = currentP;
				updateLaps = true;
				if (touchX <= X_CENTER)
				{
					currentPage--;
				}
				else
				{
					currentPage++;
				}
				if (currentPage > totalPage)
				{
					currentPage = 1;
				}
				else if (currentPage < 1)
				{
					currentPage = totalPage;
				}
			}
		}
	}

	void idle() {}

	void drawPage1(bool forceUpdate = false)
	{
		drawRpmMeter(0, 0, SCREEN_WIDTH, CELL_HEIGHT);
		drawGear(COL[2], COL[1]);
		drawBrakeOrThrottle(COL[3], ROW[1], CELL_WIDTH * 2, CELL_HEIGHT, throttle, "throttle", TFT_GREEN);
		drawBrakeOrThrottle(COL[3], ROW[2], CELL_WIDTH * 2, CELL_HEIGHT, brake, "brake", TFT_RED);

		drawCell(COL[0], ROW[1], bestLapTime, "bestLapTime", "Best Lap", "left", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[0], ROW[2], lastLapTime, "lastLapTime", "Last Lap", "left", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[0], ROW[3], currentLapTime, "currenLapTime", "Current Lap", "left", TFT_WHITE, 4, forceUpdate);

		drawCell(COL[2], ROW[3], String(speed), "speed", "Speed", "center", TFT_WHITE, 4, forceUpdate);

		// Fourth+Fifth Column (delta)
		//	drawCell(SCREEN_WIDTH, ROW[1], sessionBestLiveDeltaSeconds, "sessionBestLiveDeltaSeconds", "Delta", "right", sessionBestLiveDeltaSeconds.indexOf('-') >= 0 ? TFT_GREEN : TFT_RED, 4, forceUpdate);
		//	drawCell(SCREEN_WIDTH, ROW[2], sessionBestLiveDeltaProgressSeconds, "sessionBestLiveDeltaProgressSeconds", "Delta P", "right", sessionBestLiveDeltaProgressSeconds.indexOf('-') >= 0 ? TFT_GREEN : TFT_RED, 4, forceUpdate);

		drawCell(COL[0], ROW[4], String(lapCount) + "/" + String(totalLaps), "lapCount", "Lap", "center", TFT_YELLOW, 4, forceUpdate);
		drawCell(COL[1], ROW[4], String(RaceStartPosition) + "/" + String(preRaceNumCars), "position", "Position", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[2], ROW[4], String(fuel), "fuel", "Fuel", "center", TFT_MAGENTA, 4, forceUpdate);

		drawTyre();
	}

	void drawPage2() // pagina pulsanti
	{
		drawCell(COL[0], ROW[1], bestLapTime, "bestLapTime", "Best Lap", "left", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[0], ROW[2], lastLapTime, "lastLapTime", "Last Lap", "left", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[0], ROW[3], currentLapTime, "currenLapTime", "Current Lap", "left", TFT_WHITE, 4, forceUpdate);
		drawTyre();
	}

	void drawPage3()
	{
		drawRpmMeter(0, 0, SCREEN_WIDTH, CELL_HEIGHT);
		drawGear(COL[2], COL[1]);
		drawBrakeOrThrottle(COL[0], ROW[3], SCREEN_WIDTH, CELL_HEIGHT, throttle, "throttle", TFT_GREEN);
		drawBrakeOrThrottle(COL[0], ROW[4], SCREEN_WIDTH, CELL_HEIGHT, brake, "brake", TFT_RED);
	}

	void drawPage4()
	{
		// test
		// laps.clear();
		// for (int i = 0; i < maxLapHistory; i++)
		// {
		// 	lap p(i, 334 + i, i > 0 ? 100 - (i * 10) : 100, i > 0 ? laps[i - 1].fuelLevel - 10 : 0);
		// 	laps.push_back(p);
		// }

		if (updateLaps)
		{
			
			tft.clear();
			drawTable();
			static const int TABLEROW[] = {38, 72, 104, 136, 168, 200, 230, 265, 297};
			for (int i = 0; i < laps.size() && i < maxLapHistory; i++)
			{

				tft.drawString(String(laps[i].lapNumber), COL[0] + HALF_CELL_WIDTH / 2, TABLEROW[i], &fonts::FreeMono12pt7b);
				tft.drawString(convertTime(laps[i].lapTime), COL[1] + HALF_CELL_WIDTH, TABLEROW[i], &fonts::Font2);
				tft.drawString(String(laps[i].fuelLevel), COL[3] + HALF_CELL_WIDTH / 2, TABLEROW[i], &fonts::Font2);
				tft.drawString(String(laps[i].fuelUsed), COL[4] + HALF_CELL_WIDTH / 3, TABLEROW[i], &fonts::Font2);
			}
			updateLaps = false;
		}
	}

	void
	drawTable()
	{
		int h = 32;
		tft.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5, TFT_WHITE);
		for (int i = h; i <= SCREEN_HEIGHT; i = i + h)
		{
			tft.drawLine(0, i, SCREEN_WIDTH, i);
		}
		// tft.drawLine(COL[0], 0, COL[0], SCREEN_HEIGHT);
		tft.drawLine(COL[1], 0, COL[1], SCREEN_HEIGHT);
		tft.drawLine(COL[3], 0, COL[3], SCREEN_HEIGHT);
		tft.drawLine(COL[4], 0, COL[4], SCREEN_HEIGHT);

		tft.drawString("Lap Number", COL[0] + HALF_CELL_WIDTH / 3, 6, &fonts::Font2);
		tft.drawString("Lap Time", COL[1] + HALF_CELL_WIDTH, 6, &fonts::Font2);
		tft.drawString("Fuel ", COL[3] + HALF_CELL_WIDTH / 2, 6, &fonts::Font2);
		tft.drawString("Fuel Lap", COL[4] + HALF_CELL_WIDTH / 3, 6, &fonts::Font2);
	}

	void drawTyre()
	{
		drawCell(COL[3], ROW[3], String(tyreTemperatureFrontLeft), "tyreTemperatureFrontLeft", "FL", "center", tyreTemperatureFrontLeft < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[4], ROW[3], String(tyreTemperatureFrontRight), "tyreTemperatureFrontRight", "FR", "center", tyreTemperatureFrontRight < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[3], ROW[4], String(tyreTemperatureRearLeft), "tyreTemperatureRearLeft", "RL", "center", tyreTemperatureRearLeft < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[4], ROW[4], String(tyreTemperatureRearRight), "tyreTemperatureRearRight", "RR", "center", tyreTemperatureRearRight < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
	}

	boolean isDrawGearRpmRedRec()
	{
		if (engineRpm >= maxAlertRPM)
		{
			return true;
		}
		return false;
	}

	void drawGear(int32_t x, int32_t y)
	{
		// draw gear only when it changes
		if (gear != prev_gear)
		{
			tft.setTextColor(isDrawGearRpmRedRec() ? TFT_RED : TFT_YELLOW, TFT_BLACK);
			tft.setTextSize(8);
			tft.setTextDatum(MC_DATUM);
			tft.setCursor(x + 12, y + HALF_CELL_HEIGHT);
			tft.print(gear);
			tft.setTextSize(1);
			tft.setTextDatum(TL_DATUM);

			prev_gear = gear;
		}
	}

	void drawRpmMeter(int32_t x, int32_t y, int width, int height)
	{
		double percent = (static_cast<double>(engineRpm) / maxAlertRPM);
		int meterWidth = width * percent;
		int yPlusOne = y + 1;
		int innerWidth = width - meterWidth - 1;
		int innerHeight = height - 4;

		if (prev_rpmPercent != engineRpm)
		{
			int32_t color = TFT_BLACK;
			tft.fillRect(meterWidth, yPlusOne, width, innerHeight, color);

			if (isDrawGearRpmRedRec())
			{
				color = TFT_RED;
			}
			else if (engineRpm >= minAlertRPM)
			{
				color = TFT_ORANGE;
			}
			else
			{
				color = TFT_GREEN;
			}
			tft.fillRect(x, yPlusOne, meterWidth - 2, innerHeight, color);
			tft.setTextColor(TFT_BLACK, color);
			tft.drawString(String(engineRpm), x + (meterWidth / 2), y + 19, 4);
			prev_rpmPercent = engineRpm;
		}
	}

	void drawBrakeOrThrottle(int32_t x, int32_t y, int width, int height, int value, String name, int32_t color)
	{
		bool dataChanged = (prevData[name] != String(value)) || forceUpdate;
		if (dataChanged)
		{
			double percent = (static_cast<double>(value) / 255);
			int meterWidth = width * percent;
			int yPlusOne = y + 1;
			int innerWidth = width - meterWidth - 1;
			int innerHeight = height - 4;
			tft.fillRect(x, yPlusOne, width, innerHeight, TFT_BLACK);
			if (value > 0)
			{
				tft.fillRect(x, yPlusOne, meterWidth - 2, innerHeight, color);
				tft.setTextColor(TFT_BLACK, color);

				tft.drawString(String(value), x + (meterWidth / 2), y + 19, 4);
			}
			prevData[name] = String(value);
		}
	}

	void drawCell(int32_t x, int32_t y, String data, String id, String name = "Data", String align = "center", int32_t color = TFT_WHITE, int fontSize = 4, bool forceUpdate = false)
	{
		const static int titleHeight = 19;
		const static int hPadding = 5;
		const static int vPadding = 1;

		tft.setTextColor(color, TFT_BLACK);

		bool dataChanged = (prevData[id] != data) || forceUpdate;
		bool colorChanged = (prevColor[id] != color) || forceUpdate;

		if (dataChanged)
		{
			if (align == "left")
			{
				if (colorChanged)
					tft.drawRoundRect(x, y, CELL_WIDTH * 2 - 1, CELL_HEIGHT - 2, 5, color); // Rectangle
				if (colorChanged)
					tft.drawString(name, x + hPadding, y + vPadding, 2);	   // Title
				tft.drawString(data, x + hPadding, y + titleHeight, fontSize); // Data
			}
			else if (align == "right")
			{
				if (colorChanged)
					tft.drawRoundRect(x - (CELL_WIDTH * 2), y, CELL_WIDTH * 2 - 1, CELL_HEIGHT - 2, 5, color); // Rectangle
				if (colorChanged)
					tft.drawRightString(name, x - hPadding, y + vPadding, 2);		// Title
				tft.drawRightString(data, x - hPadding, y + titleHeight, fontSize); // Data
			}
			else // "center"
			{
				if (colorChanged)
					tft.drawRoundRect(x, y, CELL_WIDTH - 2, CELL_HEIGHT - 2, 5, color); // Rectangle
				if (colorChanged)
					tft.drawCentreString(name, x + HALF_CELL_WIDTH, y + vPadding, 2);		// Title
				tft.drawCentreString(data, x + HALF_CELL_WIDTH, y + titleHeight, fontSize); // Data
			}

			// Clean the previous speed if it was wider
			if (prevData[id].length() > data.length())
			{
				tft.setTextColor(TFT_BLACK, TFT_BLACK);
				if (align == "left")
				{
					tft.drawString(prevData[id], x + hPadding, y + titleHeight, fontSize);
				}
				else if (align == "right")
				{
					tft.drawRightString(prevData[id], x - hPadding, y + titleHeight, fontSize);
				}
				else
				{
					tft.drawCentreString(prevData[id], x + HALF_CELL_WIDTH, y + titleHeight, fontSize);
				}
			}

			prevData[id] = data;
			prevColor[id] = color;
		}
	}
};

#endif
