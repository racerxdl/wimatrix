#include "ledcontroller.h"
#include "wifi.h"
#include "netman.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  SetupLeds();
  delay(1000);

  SetMode(BACKGROUND_STRING_DISPLAY);
  LedPrint("Boot", CRGB::Green);
  delay(400);

  SetupWiFi();
  delay(400);

  SetupMQTT();
  delay(500);

  LedPrint("Ready", CRGB::Green);
  delay(1000);
  SetBrightness(0.2);
}

void loop() {
  LedLoop();
  yield();
  ArduinoOTA.handle();
  yield();
  MQTTLoop();
  delay(1);
}
