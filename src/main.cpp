#include <Arduino.h>
#include <M5StickC.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <Telemetry.h>
#include "DrawStrings.h"
#include <HttpsOTAUpdate.h>
#include <HTTPClient.h>
#include "OTA.h"

// Color definitions (http://www.barth-dev.de/online/rgb565-color-picker/)
#define PREVIEWCOLOR 0x0400 // Green
#define LIVECOLOR 0xF800    // Red
#define LEDPIN 26           // Live LED pin
#define SERVERPORT 4423     // Server port number
#define MAXLIVESCENES 10    // Max number of concurrent live/preview scenes

// WiFi
WiFiClient client;
String ssid;
String password;

// Stores last camera number & last WiFi credentials
Preferences preferences;

// Current state of TCP server connection (defaults to true so just for the first time we simulate a disconnection)
bool connectedToServer = true;
bool serverDisconnected = false;
unsigned long lastServerCheck = 0;
// Store how long has passed from the last mDNS scan
unsigned long lastMDNSScan = 0;
// Current state of WiFi connection (defaults to true so just for the first time we simulate a disconnection)
bool connectedToWifi = true;
bool checkForUpdates = true;

int buttonAMillis = 0;

// Camera variables
int cameraNumber = 1;
int cameraSources = 4;
int backGroundColor = TFT_BLACK;
int foreGroundColor = TFT_WHITE;
int programSources[MAXLIVESCENES];
int previewSources[MAXLIVESCENES];

void connectToWiFi();
void receivedData();
void updateTallyScreen(bool force = false);
void updateCamera(int newCamera);
bool checkServerConnection();
IPAddress findServerIP();

void setup()
{
  // M5 initialization
  M5.begin();
  M5.MPU6886.Init();
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(9);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextDatum(MC_DATUM);

  // LED
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  // Get camera number from flash
  preferences.begin("tally", false);
  cameraNumber = preferences.getInt("camera", 1);

  // WiFi
  connectToWiFi();

  // First battery reading
  updateBatterySignalStrength(backGroundColor, 1, cameraNumber, client);
}

void loop()
{
  // Update button states
  M5.update();
  // Update battery
  updateBatterySignalStrength(backGroundColor, 5, cameraNumber, client);

  if (M5.BtnA.wasPressed())
  {
    buttonAMillis = millis();
  }
  if (M5.BtnA.isPressed() && buttonAMillis != 0 && buttonAMillis < millis() - 1000)
  {
    int newCameraNumber;
    newCameraNumber = cameraNumber + 1;
    if (newCameraNumber > cameraSources)
      newCameraNumber = 1;
    updateCamera(newCameraNumber);
    buttonAMillis = 0;
    Serial.println(newCameraNumber + "Ok");
  }

  while (Serial.available())
  { // Received new WiFi configuration
    String data = Serial.readStringUntil('\n');
    int start = 0;
    int end = data.indexOf(" ", start);
    String command = data.substring(start, end);
    Serial.println("Command: " + command);
    if (command == "wifiConfiguration")
    {
      start = end + 1;
      end = data.indexOf("+", start);
      String ssid = data.substring(start, end);
      start = end + 1;
      end = data.indexOf("\n", start);
      String password = data.substring(start, end);
      Serial.println(ssid + "+" + password);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      WiFi.disconnect();
      connectToWiFi();
    }
    Serial.flush();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    if (checkForUpdates) {
      Serial.println("Checking for updates");
      checkForUpdates = false;
      HTTPClient http;
      http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      http.begin("https://dmsoftware.altervista.org/version.txt", certificate);
      int httpCode = http.GET();
      String payload = http.getString();
      Serial.println(payload);
      if (httpCode == 200) { //Check for the returning code
        Serial.println(payload);
        if (payload != preferences.getString("version")) {
          HttpsOTA.onHttpEvent(HttpEvent);
          Serial.println("Starting OTA");
          drawUpdate(batteryPercentage, isCharging);

          HttpsOTA.begin("https://dmsoftware.altervista.org/firmware.bin", certificate);
          while(HttpsOTA.status() != 2) {
            delay(1000);
          }
          Serial.println("OTA done");
          esp_restart();
          preferences.putString("version", payload);
        }
      }
    }
    if (!connectedToWifi) {
      connectedToWifi = true;
    }
    if (checkServerConnection())
    { // If it is connected to TCP server
      if (serverDisconnected == true)
      { // If we have just connected send authentication password
        serverDisconnected = false;
        drawNumber(foreGroundColor, backGroundColor, cameraNumber, batteryPercentage, isCharging);
        byte signalStrength = calculateSignalStrength(WiFi.RSSI());
        char message[56];
        sprintf(message, "authenticate Lt$GS7wMoSPb44&TSm7efyf^Cy9SSPTm %i %u %i %u", cameraNumber, batteryPercentage, isCharging, signalStrength);
        client.println(message);
      }
      if (client.available())
      { // If there is data to read
        receivedData();
      }
    }
    else
    {
      if (serverDisconnected == false)
      { // Check if we were connected before (that means a disconnection occurred)
        serverDisconnected = true;
        backGroundColor = BLACK;
        MDNS.begin("tally");
        lastMDNSScan = 0;
        drawServerConnection(batteryPercentage, isCharging);
      }
      if (lastMDNSScan == 0 || millis() - lastMDNSScan > 2000)
      { // Do a mDNS scan every 2 seconds
        IPAddress serverIP = findServerIP();
        if (serverIP != 0)
        {
          client.connect(serverIP, SERVERPORT);
        }
        lastMDNSScan = millis();
      }
    }
  }
  else
  {
    if (connectedToWifi == true)
    { // Check if we were connected before (that means a disconnection occurred)
      backGroundColor = BLACK;
      connectToWiFi(); // Try to reconnect
    }
  }
}

void receivedData()
{
  String data = client.readStringUntil('\n');
  int start = 0;
  int end = data.indexOf(" ", start);
  String command = data.substring(start, end);
  if (command == "switcherUpdate")
  {
    // Update number of camera sources
    start = end + 1;
    end = data.indexOf(" ", start);
    int newCameraSources = data.substring(start, end).toInt();
    if (newCameraSources != cameraSources && newCameraSources != 0)
    {
      cameraSources = newCameraSources;
      if (cameraNumber > cameraSources)
      {
        updateCamera(1);
      }
    }
    else
    {
      // Reset both arrays
      memset(previewSources, 0, MAXLIVESCENES * sizeof(*previewSources));
      memset(programSources, 0, MAXLIVESCENES * sizeof(*programSources));
      // Program source numbers
      start = end + 1;
      end = data.indexOf(" ", start);
      int sources = data.substring(start, end).toInt();
      for (size_t i = 0; i < sources; i++)
      {
        start = end + 1;
        end = data.indexOf(" ", start);
        programSources[i] = data.substring(start, end).toInt();
      }
      // Preview source numbers
      start = end + 1;
      end = data.indexOf(" ", start);
      sources = data.substring(start, end).toInt();
      for (size_t i = 0; i < sources; i++)
      {
        start = end + 1;
        end = data.indexOf(" ", start);
        previewSources[i] = data.substring(start, end).toInt();
      }
      // Update the background/foreground color accordingly
      updateTallyScreen();
    }
  }
}

bool checkServerConnection()
{
  if (lastServerCheck == 0 || millis() - lastServerCheck > 1000)
  { // Do a server check every second
    lastServerCheck = millis();
    Serial.println("Server check");
    connectedToServer = client.connected();
    return connectedToServer;
  }
  else
  {
    return connectedToServer;
  }
}

void updateCamera(int newCamera)
{
  cameraNumber = newCamera;
  preferences.putInt("camera", newCamera);
  if (client.connected())
  {
    sendTelemetry(client, cameraNumber);
    updateTallyScreen(true);
  }
}

// Initiates WiFi connection with credentials stored in flash and updates screen information
void connectToWiFi()
{
  drawWiFiSearch(batteryPercentage, isCharging);
  ssid = preferences.getString("ssid");
  password = preferences.getString("password");
  Serial.println(ssid + " " + password);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); //Turn off wifi sleep in STA mode to improve response speed
  WiFi.begin(ssid.c_str(), password.c_str());
  connectedToWifi = false;
}

// Searches for the server IP using MDNS
IPAddress findServerIP()
{
  int n = MDNS.queryService("tallyServer", "tcp");
  Serial.println(n);
  if (n > 0)
  {
    return MDNS.IP(0);
  }
  return IPAddress();
}

void updateTallyScreen(bool force)
{
  bool program = false, preview = false;
  for (size_t i = 0; i < MAXLIVESCENES; i++)
  {
    if (programSources[i] == cameraNumber)
      program = true;
    if (previewSources[i] == cameraNumber)
      preview = true;
  }

  int oldBackgroud = backGroundColor;
  int oldForegroud = foreGroundColor;
  if (program || preview)
  {
    if (program && !preview)
    { // Only program
      digitalWrite(LEDPIN, HIGH);
      backGroundColor = LIVECOLOR;
      foreGroundColor = WHITE;
    }
    else if (!program && preview)
    { // Only preview
      digitalWrite(LEDPIN, LOW);
      backGroundColor = PREVIEWCOLOR;
      foreGroundColor = WHITE;
    }
    else if (program && preview)
    { // Program and preview
      digitalWrite(LEDPIN, HIGH);
      backGroundColor = LIVECOLOR;
      foreGroundColor = PREVIEWCOLOR;
    }
  }
  else
  { // None
    digitalWrite(LEDPIN, LOW);
    backGroundColor = BLACK;
    foreGroundColor = WHITE;
  }
  if (backGroundColor != oldBackgroud || oldForegroud != foreGroundColor || force == true)
  {
    drawNumber(foreGroundColor, backGroundColor, cameraNumber, batteryPercentage, isCharging);
  }
}