#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "ledcontroller.h"
#include "wifi.h"

#define WifiSSID "WIFI"
#define HOSTNAME "WIMATRIX"

String ssid     = WifiSSID;
String password = "1234567890";

void SetupWiFi() {
  OnOTA = false;
  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  Serial.println("Setting up WiFi");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);

  LedPrint("WiFi (" WifiSSID ")", CRGB::Red);

  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    LedLoop();
    delay(10);
  }


  if (WiFi.SSID() != ssid || WiFi.psk() != password) {
    Serial.println("WiFi config changed.");
    // ... Try to connect to WiFi station.
    WiFi.begin(ssid.c_str(), password.c_str());

    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());
  } else {
    WiFi.begin();
  }

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(100);
    Serial.write('.');
    LedLoop();
    yield();
  }

  Serial.println();

  // Check connection
  if (WiFi.status() == WL_CONNECTED) {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    LedPrint("WiFi (" WifiSSID ")", CRGB::Green);
    delay(1500);
  } else {
    LedPrint("WiFi", CRGB::Yellow);
    Serial.println("Can not connect to WiFi station. Go into AP mode.");

    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP("WIMATRIX", "1234567890");

    delay(1500);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    LedPrint(WiFi.softAPIP().toString().c_str(), CRGB::Yellow);
  }

  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());

  ArduinoOTA.onStart([]() {
    OnOTA = true;
    Serial.println("OTA Update Start");
    LedPrint("OTA", CRGB::Yellow);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA Update End");
    LedPrint("OTA", CRGB::Green);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    String progressStr;
    progressStr = String((progress / (total / 100))) + String("%");
    Serial.print("Progress: ");
    Serial.println(progressStr);
    LedPrint(progressStr.c_str(), CRGB::Yellow);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    LedPrint("OTA", CRGB::Red);
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Start Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connection failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Error");
    else if (error == OTA_END_ERROR) Serial.println("End Fail");
  });

  ArduinoOTA.begin();
}

