// includes
#include "config.h"

void IRAM_ATTR watchDogInterrupt()
{
    debugln("reboot");
    ESP.restart();
}

void watchDogRefresh()
{
    timerWrite(watchDogTimer, 0); // reset timer
}

void startWatchdogTimer()
{
    debugln("Initializing Watchdog Timer!");
    watchDogTimer = timerBegin(2, 80, true);
    timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
    timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
    timerAlarmEnable(watchDogTimer);
    debugln("Watchdog timer Initialized!");
}

void saveToEEPROM(int address, String data)
{
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + data.length(), '\0'); // Null-terminator
  EEPROM.commit();
}

String readFromEEPROM(int address) {
  String data = "";
  char ch;
  for (int i = 0; (ch = EEPROM.read(address + i)) != '\0'; i++) {
    data += ch;
  }
  return data;
}

void handleRoot()
{
  server.send(200, "text/html", R"rawliteral(
    <html>
      <head><title>Thermostat Settings</title></head>
      <body>
        <h1>Thermostat Web Interface</h1>
        <form action="/setTemperature" method="POST">
          <label for="temp">Set Temperature (°C):</label>
          <input type="number" id="temp" name="temp" step="0.1" required>
          <button type="submit">Update</button>
        </form>
        <form action="/setWiFi" method="POST">
          <label for="ssid">WiFi SSID:</label>
          <input type="text" id="ssid" name="ssid" required>
          <br>
          <label for="password">WiFi Password:</label>
          <input type="password" id="password" name="password" required>
          <button type="submit">Update</button>
        </form>
      </body>
    </html>
  )rawliteral");
}

void handleSetTemperature()
{
  if (server.hasArg("temp")) {
    Setpoint = server.arg("temp").toDouble();
    saveToEEPROM(TEMP_SETPOINT_ADDR, String(Setpoint));
    server.send(200, "text/plain", "Temperature setpoint updated.");
  } else {
    server.send(400, "text/plain", "Invalid request.");
  }
}

void handleSetWiFi()
{
  if (server.hasArg("ssid") && server.hasArg("password")) {
    ssid = server.arg("ssid");
    password = server.arg("password");
    saveToEEPROM(WIFI_SSID_ADDR, ssid);
    saveToEEPROM(WIFI_PASSWORD_ADDR, password);
    server.send(200, "text/plain", "WiFi credentials updated. Rebooting...");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Invalid request.");
  }
}

void rootServer()
{
	// start web server
	server.on("/", handleRoot);
	server.on("/setTemperature", HTTP_POST, handleSetTemperature);
	server.on("/setWiFi", HTTP_POST, handleSetWiFi);
	server.begin();
}

void temperatureSensors()
{
	// get sensor data
	sensors_event_t humidity, temp;
	aht20.getEvent(&humidity, &temp);
	Input = temp.temperature;
}

void pidCalculations()
{
	// PID calculations
	myPID.Compute();
	analogWrite(HEATER_PIN, (int)Output);
}

void updateDisplay()
{
	// Update display
	display.clearDisplay();
	display.setCursor(0, 0);
	display.println("Thermostat");
	display.print("Temp: ");
	display.print(temp.temperature);
	display.println(" C");
	display.print("Hum: ");
	display.print(humidity.relative_humidity);
	display.println(" %");
	display.print("Set: ");
	display.print(Setpoint);
	display.println(" C");
	display.display();
}

void setup()
{
    // start serial
    Serial.begin(115200);
    delay(1000);
	
	// print sketch information
    debugln("Created by Anthony Sleck");
    debug("Email: ");
	debugln(email);
    debug("Version ");
    debugln(VER);
    debug("Build Code ");
    debugln(VER_BUILD);
    debug("Github: ");
    debugln(firmwareLink);
	
	// initialize EEPROM
	EEPROM.begin(EEPROM_SIZE);
	Setpoint = readFromEEPROM(TEMP_SETPOINT_ADDR).toDouble();
	ssid = readFromEEPROM(WIFI_SSID_ADDR);
	password = readFromEEPROM(WIFI_PASSWORD_ADDR);
	
	// initialize OLED display
	if (!display.begin(SSD1309_I2C_ADDRESS, OLED_RESET)) {
		debugln("SSD1309 allocation failed");
		for (;;);
	}
	display.clearDisplay();
	
	// initialize sensors
	if (!aht20.begin()) {
		debugln("AHT20 sensor not found");
	}
	if (!bmp280.begin()) {
		debugln("BMP280 sensor not found");
	}
	
	// initialize PID
	myPID.SetMode(AUTOMATIC);
	myPID.SetOutputLimits(0, 255);
	
	// connect WiFi
	WiFi.begin(ssid.c_str(), password.c_str());
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		debugln("Connecting to WiFi...");
	}
	debugln("Connected to WiFi");
	
	// start web server
	rootServer();
	
	// setup PTC heater
	pinMode(HEATER_PIN, OUTPUT);
		
	// start watchdog timer
    startWatchdogTimer();
}

void loop()
{
    // read sensor data
	temperatureSensors();
	
	// PID calculations
	pidCalculations();
	
	// update display
	updateDisplay();
	
	// handle web server
	server.handleClient();
	
	// reset watchdog timer
    watchDogRefresh();
}
