#!/usr/bin/env python

import paho.mqtt.client as mqtt
import json
import time
import colorsys

userData = {
  "hasDoubleDot": False,
  "rainbowChange": True,
  "currentColor": 0,
  "currentMessage": "OK",
  "lastUpdate": 0,
}

def hsv2rgb(h,s,v):
    return tuple(round(i * 255) for i in colorsys.hsv_to_rgb(h,s,v))

def SendMessage(client, userData, msg):
  # Try Align Center
  x = 0
  if len(msg) < 5:
    x = (30 - len(msg) * 6) / 2

  r,g,b = hsv2rgb(userData["currentColor"], 1, 1)

  client.publish("%s_msg" % userData["device"], json.dumps({
    "msg": msg,
    "r": r,
    "g": g,
    "b": b,
    "x": x,
  }))

def SetBrightness(client, brightness):
  client.publish("%s_brightness" % userData["device"], "%s" %brightness)

def SetBackgroundBrightness(client, brightness):
  client.publish("%s_bgbrightness" % userData["device"], "%s" %brightness)

def OnAnnounce(client, userdata, msg):
  try:
    data = json.loads(str(msg.payload))
    if not "device" in userdata:
      print "Device %s found" %data["name"]
      userdata["device"] = data["name"]
      SendMessage(client, userdata, "OK")
      SetBrightness(client, 0.5)
      SetBackgroundBrightness(client, 0)

    userdata["lastUpdate"] = time.time()
  except Exception, e:
    print "Error: %s" % e

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT")
    client.subscribe("announce")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
  if msg.topic == "announce":
    OnAnnounce(client, userdata, msg)
  else:
    print "%s => %s" %(msg.topic, str(msg.payload))

client = mqtt.Client(userdata=userData)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("mqtt_user", password="----")
client.connect("192.168.0.100", 1883, 60)

lastChange = time.time()
lastUpdate = time.time()

run = True
while run:
    client.loop(timeout=0.01, max_packets=1)
    if "device" in userData:
      if time.time() - userData["lastUpdate"] > 2:
        print "Lost device %s" %userData["device"]
        userData.pop("device")
        continue

      if time.time() - lastChange >= 1:
        userData["currentMessage"] = time.strftime("%H:%M" if userData["hasDoubleDot"] else "%H %M")
        lastChange = time.time()
        userData["hasDoubleDot"] = not userData["hasDoubleDot"]

      if time.time() - lastUpdate > 0.1:
        SendMessage(client, userData, userData["currentMessage"])
        lastUpdate = time.time()
        userData["currentColor"] += 0.001
        if userData["currentColor"] == 1:
          userData["currentColor"] == 0

