/*
 * This file is part of Esp8266-ESP32AlexaRelaySwitch.
 * Copyright (C) 2023 Gaming12846
 *
 * Esp8266-ESP32AlexaRelaySwitch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Esp8266-ESP32AlexaRelaySwitch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//#define ESPALEXA_ASYNC            //Uncomment for async operation (can fix empty body issue)
//#define ESPALEXA_NO_SUBPAGE       //Disable /espalexa status page
//#define ESPALEXA_DEBUG            //Activate debug serial logging
//#define ESPALEXA_MAXDEVICES 15    //Set maximum devices add-able to Espalexa
#include <Espalexa.h>

// Wifi settings
const char* ssid = "...";
const char* password = "...";

// Relais pins
#define RELAI1 0
#define RELAI2 4

// Prototypes
bool connectWifi();

// Callback functions
// New callback type, contains device pointer
void switchChangedLight1(EspalexaDevice* dev);
void switchChangedLight2(EspalexaDevice* dev);

// Create the devices yourself
EspalexaDevice* light1;
EspalexaDevice* light2;

bool wifiConnected = false;

Espalexa espalexa;

void setup() {
  Serial.begin(115200);

  // Set pinmode for the relays
  pinMode(RELAI1, OUTPUT);
  pinMode(RELAI2, OUTPUT);

  // Default turn on lights
  digitalWrite(RELAI1, LOW);
  digitalWrite(RELAI2, LOW);

  // Initialise wifi connection
  wifiConnected = connectWifi();

  if (!wifiConnected) {
    while (1) {
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }

  // ------ Espalexa ------
  // Define your devices here
  light1 = new EspalexaDevice("Light1", switchChangedLight1, EspalexaDeviceType::onoff);  //Non-dimmable device
  espalexa.addDevice(light1);
  light1->setValue(127); //Default turn on lights
  light2 = new EspalexaDevice("Light2", switchChangedLight2, EspalexaDeviceType::onoff);  //Non-dimmable device
  espalexa.addDevice(light2);
  light2->setValue(127); //Default turn on lights

  espalexa.begin();

  // ------ ArduinoOTA ------
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("light");

  // No authentication by default
  ArduinoOTA.setPassword("light");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("ArduinoOTA ready");
}

void loop() {
  espalexa.loop();
  ArduinoOTA.handle();
  delay(1);
}

// Our callback functions
void switchChangedLight1(EspalexaDevice* d) {
  if (d == nullptr) return;  //this is good practice, but not required

  Serial.print(d->getName() + " changed to ");
  if (d->getValue()) {
    Serial.println("ON");
    digitalWrite(RELAI1, LOW);
  } else {
    Serial.println("OFF");
    digitalWrite(RELAI1, HIGH);
  }
}

void switchChangedLight2(EspalexaDevice* d) {
  if (d == nullptr) return;  //this is good practice, but not required

  Serial.print(d->getName() + " changed to ");
  if (d->getValue()) {
    Serial.println("ON");
    digitalWrite(RELAI2, LOW);
  } else {
    Serial.println("OFF");
    digitalWrite(RELAI2, HIGH);
  }
}

// Connect to wifi – returns true if successful or false if not
bool connectWifi() {
  bool state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  Serial.println("");
  if (state) {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Connection failed.");
  }
  return state;
}