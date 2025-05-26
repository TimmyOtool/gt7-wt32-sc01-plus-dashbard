
#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <conf_WT32SCO1-Plus.h>
// #include <conf_JC3248W535EN.h>

#include "lap/lap.h"
#include <Arduino.h>
#include <map>
#include <vector>
#include <iostream>

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <FS.h>

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

int currentPage = 1;
bool forceUpdate = false;

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
	IPAddress ip;
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
	float fuelCapacity = 0;

	float boost = 0;
	int brake = 0;	  // max 255
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

	uint16_t touchX, touchY;

	using PageFunc = std::function<void()>;

	std::vector<PageFunc> pages;

public:
	Display()
	{
		pages.push_back([this]()
						{ drawPageDashboard1(forceUpdate); });
		pages.push_back([this]()
						{ drawPageCarInfo(); });
		pages.push_back([this]()
						{ drawPageTableLaps(); });
		pages.push_back([this]()
						{ drawCreditPage(forceUpdate); });
		pages.push_back([this]()
						{ drawDebugPage(); });
	}
	void setup()
	{
		tft.init();
		tft.setRotation(1);
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
		// TODO used calculated fuelUsed
		lap l(lapNum, packetContent.packetContent.lastLaptime, fuel, usedFuel);
		laps.push_back(l);
	}

	void read()
	{
		speed = ((int)((packetContent.packetContent.speed) * 3.6));
		gear = packetContent.packetContent.gears & 0b00001111;
		if (atoi(gear.c_str()) == 0)
		{
			gear = "R";
		}
		if (atoi(gear.c_str()) == -1)
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
		fuelCapacity = packetContent.packetContent.fuelCapacity;
		lapCount = packetContent.packetContent.lapCount;
		boost = packetContent.packetContent.boost - 1;
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

	String convertTime(int32_t toConvert)
	{
		int minutes = toConvert / 60000;
		int seconds = (toConvert % 60000) / 1000;
		int milliseconds = toConvert % 1000;

		char formattedTime[12];
		sprintf(formattedTime, "%02d:%02d:%03d", minutes, seconds, milliseconds);
		return String(formattedTime);
	}

	void loop()
	{
		static int lastPage = currentPage;
		if (currentPage != lastPage)
		{
			tft.fillScreen(TFT_BLACK);
			forceUpdate = true;
			lastPage = currentPage;
		}
		int pageIdx = currentPage - 1;
		if (pageIdx >= 0 && pageIdx < pages.size())
		{
			pages[pageIdx]();
		}

		forceUpdate = false;
		unsigned long currentP = millis();
		if (tft.getTouch(&touchX, &touchY))
		{
			if (currentP - previousP >= intervalP)
			{
				previousP = currentP;
				updateLaps = true;
				if (checkButton(X_CENTER - 100, 70, 180, 50) && currentPage == 4)
				{
					tft.fillScreen(TFT_BLACK);
					tft.drawCenterString("Rebooting...", X_CENTER, Y_CENTER, &fonts::DejaVu12);
					sleep(1);
					tft.fillScreen(TFT_BLACK);
					ESP.restart();
				}
				if (checkButton(X_CENTER - 100, Y_CENTER + 70, 200, 50) && currentPage == 4)
				{
					WiFi.mode(WIFI_STA);
					WiFi.disconnect();
					WiFiManager wm;
					wm.resetSettings();
					tft.fillScreen(TFT_BLACK);
					tft.drawCenterString("Reset...", X_CENTER, Y_CENTER, &fonts::DejaVu12);
					sleep(1);
					tft.fillScreen(TFT_BLACK);
					ESP.restart();
				}
				if (touchX <= X_CENTER / 2)
				{
					tft.clear();
					currentPage--;
				}
				else if (touchX >= X_CENTER * 1.5)
				{
				
					tft.clear();
					currentPage++;
				}
				else
				{
					if (currentPage == 3)
					{
						laps.clear();
						tft.clear();
						tft.fillScreen(TFT_RED);
						tft.drawCenterString("Laps cleared", X_CENTER, Y_CENTER, &fonts::DejaVu12);
						sleep(1);
						tft.clear();
						updateLaps = true;
					}
				}
				if (currentPage > pages.size())
				{
					currentPage = 1;
				}
				else if (currentPage < 1)
				{
					currentPage = pages.size();
				}
			}
		}
	}

	void idle() {}

	// draw a button whith a label
	void drawButton(int x, int y, int w, int h, String label, uint16_t color = TFT_WHITE)
	{
		tft.fillRoundRect(x, y, w, h, 5, color);
		tft.setTextColor(TFT_WHITE);
		tft.drawCenterString(label, x + (w / 2), y + (h / 2), &fonts::DejaVu12);
	}
	// check if click on a button
	bool checkButton(int x, int y, int w, int h)
	{
		if (touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h)
		{
			return true;
		}
		return false;
	}

	void drawDebugPage()
	{
		if (forceUpdate)
		{
			tft.fillScreen(TFT_BLACK);
			tft.setTextColor(TFT_WHITE);
			// draw all Wifi info
			tft.drawString("SSID: " + String(WiFi.SSID()), 10, 10, &fonts::DejaVu12);
			tft.drawString("IP: " + String(WiFi.localIP().toString()), 10, 30, &fonts::DejaVu12);
			tft.drawString("Signal: " + String(WiFi.RSSI()), 10, 50, &fonts::DejaVu12);
			tft.drawString("Channel: " + String(WiFi.channel()), 10, 70, &fonts::DejaVu12);
			tft.drawString("MAC: " + String(WiFi.macAddress()), 10, 90, &fonts::DejaVu12);
			tft.drawString("Subnet: " + String(WiFi.subnetMask().toString()), 10, 130, &fonts::DejaVu12);
			tft.drawString("DNS: " + String(WiFi.dnsIP().toString()), 10, 150, &fonts::DejaVu12);
			tft.drawString("Mode: " + String(WiFi.getMode()), 10, 170, &fonts::DejaVu12);
			tft.drawString("Status: " + String(WiFi.status()), 10, 190, &fonts::DejaVu12);
			tft.drawString("Hostname: " + String(WiFi.getHostname()), 10, 210, &fonts::DejaVu12);

			tft.drawString("PS5 ip: " + String(ip.toString()), 10, 250, &fonts::DejaVu12);
		}
	}

	void drawCreditPage(bool forceUpdate = false)
	{
		if (forceUpdate)
		{
			tft.fillScreen(TFT_BLACK);
			tft.setTextColor(TFT_WHITE);
			drawButton(X_CENTER - 100, 70, 180, 50, "Reboot", TFT_DARKGRAY);
			tft.drawCenterString("GT7 Dashboard", X_CENTER, Y_CENTER - 20, &fonts::DejaVu12);
			tft.drawCenterString("by", X_CENTER, Y_CENTER + 10, &fonts::DejaVu12);
			tft.drawCenterString("BSR_Melinm", X_CENTER, Y_CENTER + 30, &fonts::DejaVu12);
			tft.drawCenterString("2025", X_CENTER, Y_CENTER + 50, &fonts::DejaVu12);
			drawButton(X_CENTER - 100, Y_CENTER + 70, 180, 50, "Reset Settings", TFT_RED);
		}
	}
	void drawPageCarInfo()
	{

		// tyre temp and radius
		//  supension height
		//  maxCalcSpeed
		drawCell(COL[0], ROW[0], String(fuel), "fuelLevel", "fuel Level", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[1], ROW[0], String(waterTemp), "waterTemp", "water Temp", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[4], ROW[0], String(BodyHeight), "BodyHeight", "Body Height", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[0], ROW[1], String(oilPressure), "oilPressure", "oil Pressure", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[1], ROW[1], String(oilTemp), "oilTemp", "oil Temp", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[4], ROW[1], String(maxSpeed), "maxSpeed", "max Speed", "center", TFT_WHITE, 4, forceUpdate);

		drawCell(COL[3], ROW[2], String(minAlertRPM), "minAlertRPM", "Alert RPM", "center", TFT_WHITE, 4, forceUpdate);
		drawCell(COL[4], ROW[2], String(maxAlertRPM), "maxAlertRPM", "MaxAlert RPM", "center", TFT_WHITE, 4, forceUpdate);

		drawCell(COL[0], ROW[3], String(tyreTemperatureFrontLeft) + "    " + String(tyreRadiusFrontLeft) + "    " + String(suspHeightFrontLeft), "tyreTemperatureFrontLeft", "FL", "left", tyreTemperatureFrontLeft < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[3], ROW[3], String(tyreTemperatureFrontRight) + "    " + String(tyreRadiusFrontRight) + "    " + String(suspHeightFrontRight), "tyreTemperatureFrontRight", "FR", "left", tyreTemperatureFrontRight < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[0], ROW[4], String(tyreTemperatureRearLeft) + "    " + String(tyreRadiusRearLeft) + "    " + String(suspHeightRearLeft), "tyreTemperatureRearLeft", "RL", "left", tyreTemperatureRearLeft < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
		drawCell(COL[3], ROW[4], String(tyreTemperatureRearRight) + "    " + String(tyreRadiusRearRight) + "    " + String(suspHeightRearRight), "tyreTemperatureRearRight", "RR", "left", tyreTemperatureRearRight < tireAlertTemp ? TFT_CYAN : TFT_RED, 4, forceUpdate);
	}

	void drawPageTableLaps()
	{
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

	void drawPageDashboard1(bool forceUpdate = false)
	{
		// lap
		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawString("LAP", SCREEN_WIDTH - 140, 25, &fonts::DejaVu18);
		drawDataString(SCREEN_WIDTH - 20, 21, "lap", String(lapCount) + "/" + String(totalLaps), "right", TFT_BLACK, TFT_WHITE, &fonts::DejaVu24);

		// pos
		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawString("POS", SCREEN_WIDTH - 140, 70, &fonts::DejaVu18);
		drawDataString(SCREEN_WIDTH - 20, 65, "position", String(RaceStartPosition) + "/" + String(preRaceNumCars), "right", TFT_BLACK, TFT_WHITE, &fonts::DejaVu24);

		// rpm
		int couleurRpm = TFT_GREEN;
		if (engineRpm > (minAlertRPM - 100) && engineRpm <= minAlertRPM)
		{
			couleurRpm = TFT_BLUE;
		}
		if (engineRpm >= minAlertRPM)
		{
			couleurRpm = TFT_ORANGE;
		}
		if (engineRpm >= maxAlertRPM)
		{
			couleurRpm = TFT_RED;
		}
		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawString("RPM", SCREEN_WIDTH - 140, 110, &fonts::DejaVu18);
		drawDataString(SCREEN_WIDTH - 20, 106, "rpm", String(engineRpm), "right", TFT_BLACK, couleurRpm, &fonts::DejaVu24);
		// tft.setTextColor(TFT_WHITE);

		// gear
		tft.setTextSize(2);
		drawDataString(X_CENTER, 20, "gear", gear, "center", TFT_BLACK, TFT_WHITE, &fonts::DejaVu72);
		tft.setTextSize(1);

		// speed
		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawCenterString("SPEED", X_CENTER, 150, &fonts::DejaVu18);
		drawDataString(X_CENTER, 170, "speed", String(speed), "center", TFT_BLACK, TFT_GREEN, &fonts::DejaVu24);

		// // currenttime
		// tft.setTextColor(TFT_WHITE);
		// tft.drawRightString("CURRENT TIME", SCREEN_WIDTH - 20, 148, &fonts::DejaVu18);
		// drawDataString(SCREEN_WIDTH - 20, 173, "currentLapTime", currentLapTime, "right", TFT_BLACK, TFT_GREEN);

		// lasttime
		tft.setTextColor(TFT_WHITE);
		tft.drawRightString("LAST TIME", SCREEN_WIDTH - 20, 205, &fonts::DejaVu18);
		drawDataString(SCREEN_WIDTH - 20, 230, "lastLapTime", lastLapTime, "right");

		// besttime
		tft.setTextColor(TFT_GREEN);
		tft.drawRightString("BEST TIME", SCREEN_WIDTH - 20, 265, &fonts::DejaVu18);
		drawDataString(SCREEN_WIDTH - 20, 290, "bestLapTime", bestLapTime, "right");

		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawCenterString("FRONT", 60, 20, &fonts::DejaVu18);
		tft.setTextColor(TFT_WHITE);
		tft.setTextColor(TFT_GREEN);
		tft.drawString("L", 20, 40, &fonts::DejaVu18);
		tft.drawRightString("R", 130, 40, &fonts::DejaVu18);
		tft.setTextColor(TFT_WHITE);
		tft.fillRoundRect(20, 60, 50, 100, 5, getTyreColor(tyreTemperatureFrontLeft));
		tft.fillRoundRect(80, 60, 50, 100, 5, getTyreColor(tyreTemperatureFrontRight));
		drawDataString(45, 140, "tyreTemperatureFrontLeft1", String(tyreTemperatureFrontLeft) + " C", "center", TFT_BLUE, TFT_WHITE, &fonts::DejaVu12);
		// tft.drawCenterString(String(tyreTemperatureFrontLeft) + "C", 45, 140, &fonts::DejaVu12);
		drawDataString(105, 140, "tyreTemperatureFrontRight1", String(tyreTemperatureFrontRight) + " C", "center", TFT_BLUE, TFT_WHITE, &fonts::DejaVu12);
		// tft.drawCenterString(String(tyreTemperatureFrontRight) + "C", 105, 140, &fonts::DejaVu12);

		tft.setTextColor(TFT_LIGHTGREY);
		tft.drawCenterString("REAR", 60, 180, &fonts::DejaVu18);
		tft.setTextColor(TFT_WHITE);
		tft.setTextColor(TFT_GREEN);
		tft.drawString("L", 20, 200, &fonts::DejaVu18);
		tft.drawRightString("R", 130, 200, &fonts::DejaVu18);
		tft.setTextColor(TFT_WHITE);
		tft.fillRoundRect(20, 220, 50, 100, 5, getTyreColor(tyreTemperatureRearLeft));
		tft.fillRoundRect(80, 220, 50, 100, 5, getTyreColor(tyreTemperatureRearRight));
		drawDataString(45, 300, "tyreTemperatureRearLeft1", String(tyreTemperatureRearLeft) + " C", "center", TFT_BLUE, TFT_WHITE, &fonts::DejaVu12);
		// tft.drawCenterString(String(tyreTemperatureRearLeft)+"C",45,300,&fonts::DejaVu12);
		drawDataString(105, 300, "tyreTemperatureRearRight1", String(tyreTemperatureRearRight) + " C", "center", TFT_BLUE, TFT_WHITE, &fonts::DejaVu12);
		// tft.drawCenterString(String(tyreTemperatureRearRight)+"C",105,300,&fonts::DejaVu12);

		// FUEL
		tft.drawRightString("Fuel", 200, 260, &fonts::DejaVu18);
		tft.setTextColor(TFT_GREEN);
		drawDataString(310, 262, "fuel1", String(fuel) + "/" + String(fuelCapacity), "right", TFT_BLACK, TFT_GREEN, &fonts::DejaVu12);
		tft.setTextColor(TFT_WHITE);
		tft.fillRoundRect(170, 285, 130, 25, 5, TFT_LIGHTGREY);
		int fuelWidth = getPercentageWidth(fuel, fuelCapacity, 130);
		tft.setTextColor(TFT_BLACK);
		if (fuelWidth >= 0 && fuelWidth <= 130)
		{
			tft.fillRoundRect(170, 285, 130 - fuelWidth, 25, 5, TFT_GREEN);
		}
		drawDataString(250, 290, "fuel", String(fuel) + "%", "center", TFT_WHITE, TFT_BLACK, &fonts::DejaVu18);
		tft.setTextColor(TFT_WHITE);

		// NONE
		tft.fillRoundRect(145, 30, 15, 110, 5, TFT_LIGHTGREY);
		// BRAKE
		tft.fillRoundRect(165, 30, 15, 110, 5, TFT_LIGHTGREY);
		int brakeWidth = getPercentageWidth(brake, 255, 110);
		tft.fillRoundRect(165, 30 + brakeWidth, 15, 110 - brakeWidth, 5, TFT_RED);

		// THROTTLE
		tft.fillRoundRect(SCREEN_WIDTH - 185, 30, 20, 110, 5, TFT_LIGHTGREY);
		int throttleWidth = getPercentageWidth(throttle, 255, 110);
		tft.fillRoundRect(SCREEN_WIDTH - 185, 30 + throttleWidth, 20, 110 - throttleWidth, 5, TFT_GREEN);

		// BOOST
		if (dataChange("boost", String(boost)))
		{
			tft.fillSmoothCircle(SCREEN_WIDTH - 160, calculatePosition(prevData["boost"].toFloat()), 5, TFT_BLACK);
			prevData["boost"] = String(boost);
		}

		tft.fillRect(SCREEN_WIDTH - 160, 30, 5, 110, TFT_LIGHTGREY);
		tft.fillRect(SCREEN_WIDTH - 160, 30, 10, 3, TFT_LIGHTGREY);
		tft.fillRect(SCREEN_WIDTH - 160, 66, 10, 3, TFT_LIGHTGREY);
		tft.fillRect(SCREEN_WIDTH - 160, 101, 10, 3, TFT_LIGHTGREY);
		tft.fillRect(SCREEN_WIDTH - 160, 137, 10, 3, TFT_LIGHTGREY);

		tft.fillSmoothCircle(SCREEN_WIDTH - 160, calculatePosition(boost), 5, TFT_RED);

		// RPM
		tft.fillRoundRect(100, 5, 270, 10, 5, TFT_LIGHTGREY);
		int rpmWidth = getPercentageWidth(engineRpm, maxAlertRPM, 270);
		if (rpmWidth < 0 || rpmWidth > 270)
		{
			rpmWidth = 0;
		}

		tft.fillRoundRect(100, 5, 270 - rpmWidth, 10, 5, couleurRpm);
	}

	int calculatePosition(float boost)
	{
		// Points de référence
		float x[] = {-1, 0, 1, 2};	  // Boost values
		int y[] = {30, 66, 103, 137}; // Corresponding positions

		// Vérifier si boost est en dehors des limites
		if (boost <= x[0])
			return y[0];
		if (boost >= x[3])
			return y[3];

		// Trouver les deux points entre lesquels boost se situe
		for (int i = 0; i < 3; i++)
		{
			if (boost >= x[i] && boost <= x[i + 1])
			{
				// Interpolation linéaire
				return y[i] + (boost - x[i]) * (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
			}
		}

		return 0; // Valeur par défaut (ne devrait jamais arriver)
	}

	int getTyreColor(int value)
	{
		if (value > tireAlertTemp && value < (tireAlertTemp + 50))
		{
			return TFT_ORANGE;
		}
		if (value >= (tireAlertTemp + 50))
		{
			return TFT_RED;
		}
		return TFT_BLUE;
	}

	int getPercentageWidth(int value, int max, int width)
	{
		double percent = (static_cast<double>(value) / max);
		return width - (width * percent);
	}

	boolean dataChange(String name, String value)
	{
		if (prevData[name] != value || forceUpdate)
		{
			return true;
		}
		return false;
	}

	void drawDataString(int32_t x, int32_t y, String name, String text, String align = "center", int32_t colorClear = TFT_BLACK, int32_t color = TFT_WHITE, const lgfx::v1::IFont *font = &fonts::DejaVu24)
	{
		if (dataChange(name, text))
		{
			tft.setTextColor(colorClear);
			if (align == "left")
				tft.drawString(String(prevData[name]), x, y, font);
			else if (align == "right")
				tft.drawRightString(String(prevData[name]), x, y, font);
			else
				tft.drawCenterString(String(prevData[name]), x, y, font);
		}
		tft.setTextColor(color);
		if (align == "left")
			tft.drawString(text, x, y, font);
		else if (align == "right")
			tft.drawRightString(text, x, y, font);
		else
			tft.drawCenterString(text, x, y, font);
		prevData[name] = text;
	}

	void drawTable()
	{
		tft.setTextColor(TFT_WHITE);
		int h = 32;
		tft.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5, TFT_WHITE);
		for (int i = h; i <= SCREEN_HEIGHT; i = i + h)
		{
			tft.drawLine(0, i, SCREEN_WIDTH, i);
		}
		tft.drawLine(COL[1], 0, COL[1], SCREEN_HEIGHT);
		tft.drawLine(COL[3], 0, COL[3], SCREEN_HEIGHT);
		tft.drawLine(COL[4], 0, COL[4], SCREEN_HEIGHT);

		tft.drawString("Lap Number", COL[0] + HALF_CELL_WIDTH / 3, 6, &fonts::Font2);
		tft.drawString("Lap Time", COL[1] + HALF_CELL_WIDTH, 6, &fonts::Font2);
		tft.drawString("Fuel ", COL[3] + HALF_CELL_WIDTH / 2, 6, &fonts::Font2);
		tft.drawString("Fuel Lap", COL[4] + HALF_CELL_WIDTH / 3, 6, &fonts::Font2);
	}

	boolean isDrawGearRpmRedRec()
	{
		if (engineRpm >= maxAlertRPM)
		{
			return true;
		}
		return false;
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
