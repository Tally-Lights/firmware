#include "DrawStrings.h"
#include "Charging.h"

void drawWiFiSearch(int batteryPercentage, bool isCharging) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawString("Ricerca", M5.Lcd.width() / 2, 42, 4);
  M5.Lcd.drawString("WiFi", M5.Lcd.width() / 2, 66, 4);
  drawBattery(batteryPercentage, isCharging, BLACK);
}

void drawNumber(int foreGroundColor, int backGroundColor, int cameraNumber, int batteryPercentage, bool isCharging) {
  M5.Lcd.fillScreen(backGroundColor);
  M5.Lcd.setTextColor(foreGroundColor, backGroundColor);
  M5.Lcd.drawString(String(cameraNumber), M5.Lcd.width() / 2, (M5.Lcd.height() / 2) - 15, 8);
  M5.Lcd.setTextColor(WHITE, backGroundColor);
  drawBattery(batteryPercentage, isCharging, backGroundColor);
}

void drawBattery(int batteryPercentage, bool isCharging, int backGroundColor) {
  M5.Lcd.fillRect(103, 3, 52, 20, backGroundColor);
  M5.Lcd.drawRect(115, 3, 40, 20, WHITE);
  M5.Lcd.drawRect(155, 10, 2, 6, WHITE);
  M5.Lcd.setTextSize(1);  
  M5.Lcd.drawString(String(batteryPercentage)+"%", 135, 13, 2);
  if (isCharging) {
    M5.Lcd.drawBitmap(103, 5, 10, 18, chargingIcon, 0x0000);
  }
}

void drawServerConnection(int batteryPercentage, bool isCharging) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.drawString("Ricerca", M5.Lcd.width() / 2, 42, 4);
  M5.Lcd.drawString("switcher", M5.Lcd.width() / 2, 66, 4);
  drawBattery(batteryPercentage, isCharging, BLACK);
}

void drawUpdate(int batteryPercentage, bool isCharging) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.drawString("Aggiornamento", M5.Lcd.width() / 2, 44, 2);
  M5.Lcd.drawString("firmware in corso", M5.Lcd.width() / 2, 62, 2);
  drawBattery(batteryPercentage, isCharging, BLACK);
}