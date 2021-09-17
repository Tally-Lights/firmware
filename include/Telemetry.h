#ifndef _BATTERYHELPER_H_
#define _BATTERYHELPER_H_

#include <Arduino.h>
#include <M5StickC.h>
#include <WiFi.h>
#include "DrawStrings.h"

extern byte batteryPercentage;
extern int signalStrength;
extern bool isCharging;

double getBatteryVoltage();
int getBatteryPercentage(double volt);
void updateBatterySignalStrength(int backGroundColor, int samplesForAverage, int cameraNumber, WiFiClient client);
int calculateSignalStrength(int signalStrength);
void sendTelemetry(WiFiClient client, int cameraNumber);

#endif  // _BATTERYHELPER_H_