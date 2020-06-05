#include <ArduinoJson.h>
#include <MQTT.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "storage.h"
#include "netman.h"
#include "ledcontroller.h"

#define MQTTPORT 1883

#define MSGTOPIC "_msg"
#define BRIGHTNESSTOPIC "_brightness"
#define BGBRIGHTNESSTOPIC "_bgbrightness"
#define MODETOPIC "_mode"
#define TEXTCOLORTOPIC "_textcolor"
#define BGCOLORTOPIC "_bgcolor"
#define SCROLLSPEEDTOPIC "_scrollspeed"

MQTTClient client(2048);
WiFiClient net;

String msgTopic = MSGTOPIC;
String brightnessTopic = BRIGHTNESSTOPIC;
String bgBrightnessTopic = BGBRIGHTNESSTOPIC;
String modeTopic = MODETOPIC;
String textColorTopic = TEXTCOLORTOPIC;
String bgColorTopic = BGCOLORTOPIC;
String scrollSpeedTopic = SCROLLSPEEDTOPIC;

String deviceData;

DynamicJsonDocument inputDocument(2048);

void parseMessage(String message) {
  auto error = deserializeJson(inputDocument, message);
  if (!error) {
    uint8_t r = inputDocument["r"],
            g = inputDocument["g"],
            b = inputDocument["b"];

    int x = inputDocument["x"];
    int y = inputDocument["y"];
    int a = inputDocument["align"];

    String msg = inputDocument["msg"];
    CRGB color(r,g,b);

    Serial.print("Received \"");
    Serial.print(msg);
    Serial.print("\" with color ");
    Serial.println(color);
    if (!a && (x != 0 || y != 0)) {
      LedPrintAt(x, y, inputDocument["msg"].as<String>().c_str(), color);
    } else {
      LedPrint(inputDocument["msg"].as<String>().c_str(), color);
    }
  } else {
    Serial.print("Invalid JSON: ");
    Serial.println(error.c_str());
    Serial.println(message);
  }
}

void parseColor(String message) {
  auto error = deserializeJson(inputDocument, message);
  if (!error) {
    uint8_t r = inputDocument["r"],
            g = inputDocument["g"],
            b = inputDocument["b"];
    CRGB color = CRGB(r,g,b);
    LedPrintSetColor(color);
    Serial.print("Received text color");
    Serial.println(color);
  } else {
    Serial.print("Invalid JSON: ");
    Serial.println(error.c_str());
    Serial.println(message);
  }
}

void parseBGColor(String message) {
  auto error = deserializeJson(inputDocument, message);
  if (!error) {
    uint8_t r = inputDocument["r"],
            g = inputDocument["g"],
            b = inputDocument["b"];
    CRGB color = CRGB(r,g,b);
    SetBackgroundColor(color);
    Serial.print("Received background color");
    Serial.println(color);
  } else {
    Serial.print("Invalid JSON: ");
    Serial.println(error.c_str());
    Serial.println(message);
  }
}

void parseScrollSpeed(int speed) {
  if (speed > 80 || speed < 5) {
    return;
  }

  SetScrollSpeed(speed);
}

void messageReceived(String &topic, String &payload) {
  if (topic == msgTopic) {
    parseMessage(payload);
    return;
  }

  if (topic == brightnessTopic) {
    SetBrightness(payload.toFloat());
    return;
  }

  if (topic == bgBrightnessTopic) {
    SetBackgroundBrightness(payload.toFloat());
    return;
  }

  if (topic == modeTopic) {
    SetMode(payload.toInt());
    return;
  }

  if (topic == textColorTopic) {
    parseColor(payload);
    return;
  }

  if (topic == bgColorTopic) {
    parseBGColor(payload);
    return;
  }

  if (topic == scrollSpeedTopic) {
    parseScrollSpeed(payload.toInt());
    return;
  }

  Serial.print("Unknown topic: ");
  Serial.println(topic);
}

void Subscribe(String &topic) {
  topic = String(WiFi.getHostname()) + topic;
  client.subscribe(topic);

  Serial.print("Subscribing to topic ");
  Serial.println(topic);
}

void RefreshDeviceData() {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject root = jsonBuffer.to<JsonObject>();
  JsonObject network = jsonBuffer.createNestedObject("network");
  JsonArray functions = jsonBuffer.createNestedArray("functions");

  root["name"] = WiFi.getHostname();
  root["uid"] = String((long int)ESP.getEfuseMac(), HEX) + WiFi.macAddress();
  root["ssid"] = WiFi.SSID();

  network["ip"] = WiFi.localIP().toString();
  network["netmask"] = WiFi.subnetMask().toString();
  network["gateway"] = WiFi.gatewayIP().toString();
  network["rssi"] = WiFi.RSSI();

  JsonObject displayFunction = functions.createNestedObject();
  JsonObject brightnessFunction = functions.createNestedObject();
  JsonObject bgBrightnessFunction = functions.createNestedObject();
  JsonObject modeFunction = functions.createNestedObject();
  JsonObject textColorFunction = functions.createNestedObject();
  JsonObject bgColorFunction = functions.createNestedObject();
  JsonObject scrollSpeedFunction = functions.createNestedObject();

  displayFunction["type"] = "display";
  displayFunction["subtopic"] = MSGTOPIC;
  displayFunction["name"] = "Text Display";

  brightnessFunction["type"] = "float_variable";
  brightnessFunction["subtopic"] = BRIGHTNESSTOPIC;
  brightnessFunction["name"] = "Text Brightness";

  bgBrightnessFunction["type"] = "float_variable";
  bgBrightnessFunction["subtopic"] = BGBRIGHTNESSTOPIC;
  bgBrightnessFunction["name"] = "Background Brightness";

  modeFunction["type"] = "int_variable";
  modeFunction["subtopic"] = MODETOPIC;
  modeFunction["name"] = "Display Mode";

  textColorFunction["type"] = "json";
  textColorFunction["subtopic"] = TEXTCOLORTOPIC;
  textColorFunction["name"] = "Text Color";

  bgColorFunction["type"] = "json";
  bgColorFunction["subtopic"] = BGCOLORTOPIC;
  bgColorFunction["name"] = "Background Color";

  scrollSpeedFunction["type"] = "int";
  scrollSpeedFunction["subtopic"] = SCROLLSPEEDTOPIC;
  scrollSpeedFunction["name"] = "Scroll Speed";

  deviceData = "";
  serializeJson(root, deviceData);
}

void clientConnect() {
  String mqttUser = GetMQTTUser();
  String mqttPass = GetMQTTPass();
  Serial.println("Connecting...");
  LedPrint("MQTT Connecting...", CRGB::Yellow);
  while (!client.connect(WiFi.getHostname(), mqttUser.c_str(), mqttPass.c_str())) {
    Serial.print(".");
    delay(100);
    LedLoop();
    yield();
  }

  Serial.println();
  Serial.println("Connected!");
  LedPrint("MQTT", CRGB::Green);

  // build setup data
  RefreshDeviceData();

  // Reset
  msgTopic = String(MSGTOPIC);
  brightnessTopic = String(BRIGHTNESSTOPIC);
  bgBrightnessTopic = String(BGBRIGHTNESSTOPIC);
  modeTopic = String(MODETOPIC);
  textColorTopic = String(TEXTCOLORTOPIC);
  bgColorTopic = String(BGCOLORTOPIC);
  scrollSpeedTopic = String(SCROLLSPEEDTOPIC);

  // Subscribe
  Subscribe(msgTopic);
  Subscribe(brightnessTopic);
  Subscribe(bgBrightnessTopic);
  Subscribe(modeTopic);
  Subscribe(textColorTopic);
  Subscribe(bgColorTopic);
  Subscribe(scrollSpeedTopic);
}

void SetupMQTT() {
  String mqttHost = GetMQTTHost();
  Serial.println("Setting up MQTT");
  client.begin(mqttHost.c_str(), MQTTPORT, net);
  client.onMessage(messageReceived);

  clientConnect();

  Serial.println("Done");
}

long lastAnounce = 0;

void Announce() {
  client.publish("announce", deviceData);
}

void MQTTLoop() {
  client.loop();

  if (!client.connected()) {
    clientConnect();
  }

  if (millis() - lastAnounce > 1000) {
    lastAnounce = millis();
    RefreshDeviceData();
    Announce();
  }
}

