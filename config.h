/*
   ESP32_FilamentDryBox	// Enter in Project Title
   Description:
   ESP32 Filament Dry Box utilizing an ESP32, a AHT20+BMP280 Temperature Humidity Air Pressure Module, and a PTC
   Notes:
   Contact Info:
   email - anthony.sleck@gmail.com
   web - https://github.com/3lkgrove/
   github - https://github.com/3lkgrove/
   Changelog:
   0.1 - new code
*/

#ifndef config_h
#define config_h

// system versioning
#define VER "0.1"
#define VER_BUILD "01282025"	// Enter in build date
#define email "3lkgrove@gmail.com"
#define firmwareLink "https://github.com/3lkgrove/"

// common includes
#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1309.h>
#include <EEPROM.h>
#include <PID_v1.h>
#include <WebServer.h>
#include <WiFi.h>

// device definitions
#define DEBUG 1      // Set to 0 to disable serial debugging; set to 1 to enable serial debugging

// debugging
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// heater pin
#define HEATER_PIN 26			// Set to heater control pin; prefer to have a mosfet or ssr to control power of the PTC

// OLED display
#define OLED_RESET -1
Adafruit_SSD1309 display(128, 64, &Wire, OLED_RESET);

// persistent memory addresses
#define EEPROM_SIZE 512
#define TEMP_SETPOINT_ADDR 0
#define WIFI_SSID_ADDR 50
#define WIFI_PASSWORD_ADDR 150

// PID variables
double Setpoint, Input, Output;
double Kp = 2, Ki = 5, Kd = 1; // adjust as needed
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// sensors
Adafruit_AHTX0 aht20;
Adafruit_BMP280 bmp280;

// web server
WebServer server(80);

// WiFi credentials
String ssid = "defaultSSID";
String password = "defaultPassword";

// wd timer
unsigned long startPostMillis;
const unsigned long periodPost = 300000; // the time in ms
#define WATCHDOG_TIMEOUT_S 15            // enter time in sec
hw_timer_t *watchDogTimer = NULL;

#endif