#include "ledcontroller.h"
#include "wifi.h"
#include "netman.h"
#include "storage.h"

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Initializing Storage");
  InitStorage();

  // Uncomment to configure
  // SaveWifiSSID("----");
  // SaveWifiPassword("----");
  // SaveHostname("WIMATRIX2");
  // SaveMQTTHost("192.168.0.100");
  // SaveMQTTUser("mqtt_user");
  // SaveMQTTPass("---");
  // SaveOTAPassword("---");

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
  SetBrightness(0.01);
  delay(1000);
  SetMode(MODE_CLOCK);
}

int schedCount = 0;

void loop() {
  ArduinoOTA.handle();
  if (!InOTA()) {
    WiFiLoop();
    MQTTLoop();
    if (schedCount % 4) {
      LedLoop();
      yield();
    }
  }

  schedCount++;
  if (schedCount == 10) {
    schedCount = 0;
  }
}
