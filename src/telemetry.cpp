#include "Telemetry.h"

byte batteryPercentage;
int signalStrength;
unsigned long lastBatteryCheck = 0;
byte batterySamples = 0;
bool isCharging = false;
double voltageSum;
double strengthSum;

void updateBatterySignalStrength(int backGroundColor, int samplesForAverage, int cameraNumber, WiFiClient client)
{
  // Do a battery reading every second
  if (lastBatteryCheck == 0 || millis() - lastBatteryCheck > 1000)
  {
    voltageSum += getBatteryVoltage();
    lastBatteryCheck = millis();
    batterySamples++;
    strengthSum += calculateSignalStrength(WiFi.RSSI());

    // Average of battery and signal strength readings
    if (batterySamples == samplesForAverage) {
      double volt = voltageSum / samplesForAverage;
      voltageSum = 0;
      byte newBatteryPercentage = getBatteryPercentage(volt);
      if (newBatteryPercentage != batteryPercentage) {
        batteryPercentage = newBatteryPercentage;
      }
      if (M5.Axp.GetVBusVoltage() > 2) {
        isCharging = true;
      } else {
        isCharging = false;
      }
      drawBattery(batteryPercentage, isCharging, backGroundColor);
      signalStrength = calculateSignalStrength(strengthSum / samplesForAverage);
      strengthSum = 0;
      batterySamples = 0;
      sendTelemetry(client, cameraNumber);
    }
  }
}

void sendTelemetry(WiFiClient client, int cameraNumber) {
  if(client.connected()) {
    char message[56];
    sprintf(message, "telemetry %i %u %i %u", cameraNumber, batteryPercentage, isCharging ? 1 : 0, signalStrength);
    client.println(message);
  }
}

double getBatteryVoltage()
{
  return M5.Axp.GetBatVoltage();
}

int calculateSignalStrength(int signalStrength) {
  signalStrength > -65 ? signalStrength = 2 : signalStrength = 1 ;
  return signalStrength;
}

int getBatteryPercentage(double volt)
{
  if (volt < 3.30)
    return 0;
  if (volt < 3.47)
    return 5;
  if (volt < 3.55)
    return 10;
  if (volt < 3.57)
    return 15;
  if (volt < 3.61)
    return 20;
  if (volt < 3.63)
    return 25;
  if (volt < 3.65)
    return 30;
  if (volt < 3.67)
    return 35;
  if (volt < 3.69)
    return 40;
  if (volt < 3.70)
    return 45;
  if (volt < 3.72)
    return 50;
  if (volt < 3.76)
    return 55;
  if (volt < 3.78)
    return 60;
  if (volt < 3.81)
    return 65;
  if (volt < 3.83)
    return 70;
  if (volt < 3.87)
    return 75;
  if (volt < 3.9)
    return 80;
  if (volt < 3.94)
    return 85;
  if (volt < 3.98)
    return 90;
  if (volt < 4.03)
    return 95;
  else
    return 100;
}