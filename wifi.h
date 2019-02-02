#include "Arduino.h"
#include <ArduinoOTA.h>

#ifndef __WIFI__
#define __WIFI__
void SetupWiFi();
static bool OnOTA;
#endif