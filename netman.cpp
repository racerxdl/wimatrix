#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <MQTT.h>

#include "netman.h"
#include "ledcontroller.h"


#define MQTTIP "10.10.5.115"
#define MQTTPORT 1883
#define MQTTUSER "mqtt_user"
#define MQTTPASS "mqtt_pass"

#define MSGTOPIC "_msg"
#define BRIGHTNESSTOPIC "_brightness"
#define BGBRIGHTNESSTOPIC "_bgbrightness"

MQTTClient client(2048);
WiFiClient net;

String msgTopic = MSGTOPIC;
String brightnessTopic = BRIGHTNESSTOPIC;
String bgBrightnessTopic = BGBRIGHTNESSTOPIC;

String deviceData;

void parseMessage(String message) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(message);
  if (obj.success()) {
    uint8_t r = obj["r"], g = obj["g"], b = obj["b"];
    int x = obj["x"];
    int y = obj["y"];
    int a = obj["align"];

    String msg = obj["msg"];
    CRGB color(r,g,b);

    Serial.print("Received \"");
    Serial.print(msg);
    Serial.print("\" with color ");
    Serial.println(color);
    if (!a && (x != 0 || y != 0)) {
      LedPrintAt(x, y, obj["msg"].as<String>().c_str(), color);
    } else {
      LedPrint(obj["msg"].as<String>().c_str(), color);
    }
  } else {
    Serial.print("Invalid JSON: ");
    Serial.println(message);
  }
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

  Serial.print("Unknown topic: ");
  Serial.println(topic);
}

void Subscribe(String &topic) {
  topic = String(WiFi.hostname()) + topic;
  client.subscribe(topic);

  Serial.print("Subscribing to topic ");
  Serial.println(topic);
}

void RefreshDeviceData() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject &network = jsonBuffer.createObject();

  root["name"] = WiFi.hostname();
  root["uid"] = String(ESP.getChipId(), HEX) + WiFi.macAddress();
  root["ssid"] = WiFi.SSID();

  network["ip"] = WiFi.localIP().toString();
  network["netmask"] = WiFi.subnetMask().toString();
  network["gateway"] = WiFi.gatewayIP().toString();
  network["rssi"] = WiFi.RSSI();

  root["network"] = network;

  deviceData = "";
  root.printTo(deviceData);
}

void clientConnect() {
  Serial.println("Connecting...");
  LedPrint("MQTT Connecting...", CRGB::Yellow);
  while (!client.connect(WiFi.hostname().c_str(), MQTTUSER, MQTTPASS)) {
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

  // Subscribe
  Subscribe(msgTopic);
  Subscribe(brightnessTopic);
  Subscribe(bgBrightnessTopic);
}

void SetupMQTT() {
  Serial.println("Setting up MQTT");
  client.begin(MQTTIP, MQTTPORT, net);
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

