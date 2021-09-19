#ifndef _DRAWSTRINGS_H_
#define _DRAWSTRINGS_H_

#include <Arduino.h>
#include <M5StickC.h>

void drawWiFiSearch(int batteryPercentage, bool isCharging);
void drawNumber(int foreGroundColor, int backGroundColor, int cameraNumber, int batteryPercentage, bool isCharging);
void drawBattery(int batteryPercentage, bool isCharging, int backGroundColor);
void drawServerConnection(int batteryPercentage, bool isCharging);
void drawUpdate(int batteryPercentage, bool isCharging);
#endif  // _DRAWSTRINGS_H_