#include "Arduino.h"
#include "ledcontroller.h"
#include "font.h"

#define MATRIX_W 30
#define MATRIX_H 10
#define NUM_LEDS (MATRIX_W * MATRIX_H)

// GPIO5 - D1
#define DATA_PIN 1

// Define the array of leds
CRGB leds[NUM_LEDS];

// Define Background
CRGB background[NUM_LEDS];

int state = STRING_DISPLAY;
char buffer[8192];
CRGB bufferColor;

int scrollX = 0;
int scrollY = 0;
int scrollSpeed = 6; // 10 px / s

uint8_t brightness = 255;
uint8_t bgBrightness = 255;

const int scrollFactor = 10;

long timeHolder = 0;

void SetupLeds() {
  buffer[0] = 0x00;
  Serial.println("Setting up LEDS...");

  Serial.println("Setting up Background");
  for (int i = 0; i < NUM_LEDS; i++) {
    background[i] = CRGB(2,2,2);
  }

  Serial.println("Initializing NeoPixel Array");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  Serial.println("Resetting to Background");
  ResetToBackground();

  Serial.println("Writing Boot");
  WriteStringAt(3, 0, "Boot", CRGB::Red);

  Serial.println("Display Boot Message");
  FastLED.show();

  Serial.println("Led Setup Done");
  timeHolder = millis();
  bufferColor = CRGB::White;
}

int StringDisplayLoop(int resetBg) {
  int buffLen = strlen(buffer);
  if (buffLen * font16_10_charWidth > MATRIX_W) { // Needs scroll
    if (millis() - timeHolder > (1000 / scrollFactor)) {
      scrollX -= scrollSpeed;
      if (scrollX < (-buffLen * font16_10_charWidth * scrollFactor)) {
        scrollX = buffLen * font16_10_charWidth * scrollFactor;
      }
      timeHolder = millis();
    }

    return WriteStringAt(scrollX / scrollFactor, scrollY / scrollFactor, buffer, bufferColor);
  } else {
    return WriteStringAt(scrollX / scrollFactor, scrollY / scrollFactor, buffer, bufferColor);
  }
}

void SetBackground(int x, int y, CRGB color) {
  if ( x < 0 || x >= MATRIX_W || y < 0 || y >= MATRIX_H) {
    return;
  }

  background[y*MATRIX_W+x] = color;
}

void SetMode(int mode) {
  state = mode;
  scrollX = 0;
  scrollY = 0;
  timeHolder = millis();
}

int BackgroundDisplayLoop() {
  ResetToBackground(); // TODO: Cache write
  return 1;
}

inline int BackgroundStringDisplayLoop() {
  int x = 0;
  x += BackgroundDisplayLoop();
  x += StringDisplayLoop(0);
  return x;
}

void LedPrintAt(int x, int y, const char *text, CRGB color) {
  strncpy(buffer, text, 8191);
  timeHolder = millis();
  scrollX = x * scrollFactor;
  scrollY = y * scrollFactor;
  bufferColor = color;
  buffer[8191] = 0x00;
  LedLoop();
}

void LedPrint(const char *text, CRGB color) {
  int len = strlen(text) * font16_10_charWidth; // Pixels
  // Try center it if less than width
  if (len < MATRIX_W) {
    scrollX = ((MATRIX_W - len) / 2) * scrollFactor; // Align center
  } else {
    scrollX = 0; // The text is bigger than display, it will scroll.
  }

  strncpy(buffer, text, 8191);
  timeHolder = millis();
  scrollY = 0;
  bufferColor = color;
  buffer[8191] = 0x00;
  LedLoop();
}

void LedLoop() {
  int changed = 0;
  switch (state) {
    case STRING_DISPLAY: changed = StringDisplayLoop(1); break;
    case BACKGROUND_ONLY: changed = BackgroundDisplayLoop(); break;
    case BACKGROUND_STRING_DISPLAY: changed = BackgroundStringDisplayLoop(); break;
  }

  if (changed) {
    FastLED.show();
  }
}

void ResetToBackground() {
  for (int i=0; i<NUM_LEDS; i++) {
    leds[i].r = scale8(background[i].r, bgBrightness);
    leds[i].g = scale8(background[i].g, bgBrightness);
    leds[i].b = scale8(background[i].b, bgBrightness);
  }
}

inline CRGB GetPixel(int x, int y) {
  return leds[y * MATRIX_W + x];
}

inline int SetPixel(int x, int y, CRGB color) {
  CRGB old = leds[y * MATRIX_W + x];
  color.r = scale8(color.r, brightness);
  color.g = scale8(color.g, brightness);
  color.b = scale8(color.b, brightness);

  leds[y * MATRIX_W + x] = color;
  return old != color;
}

int WriteStringAt(int x, int y, const char *str, CRGB fgColor, CRGB bgColor) {
  int length = strlen(str);
  int origX = x;
  int changedPixels = 0;
  for (int i = 0; i < length; i++) {
    if (str[i] == '\n') {
      y += font16_10_height;
      x = origX;
    } else {
      changedPixels += WriteCharAt(x, y, str[i], fgColor, bgColor);
      x += font16_10_charWidth;
    }
  }

  return changedPixels;
}

int WriteStringAt(int x, int y, const char *str, CRGB fgColor) {
  int length = strlen(str);
  int origX = x;
  int changedPixels = 0;
  for (int i = 0; i < length; i++) {
    if (str[i] == '\n') {
      y += font16_10_height;
      x = origX;
    } else {
      changedPixels += WriteCharAt(x, y, str[i], fgColor);
      x += font16_10_charWidth;
    }
  }

  return changedPixels;
}

int WriteCharAt(int x, int y, uint8_t charToPut, CRGB fgColor, CRGB bgColor) {
  const int h = font16_10_height;
  const int cw = font16_10_charsPerLine;

  int cx = (charToPut & 0xF);
  int cy = ((charToPut & 0xF0) >> 4) * h;
  int changedPixels = 0;

  for(int l = 0; l < h; l++) {
    int ledY = y + l;
    if (ledY >= MATRIX_H || ledY < 0) {
      continue; // Out of bounds
    }
    // Put all lines
    uint8_t line = font16_10[cy * cw + l * cw + cx];
    for (int b = 0; b < font16_10_charWidth; b++) {
      int ledX = x + b;
      if (ledX >= MATRIX_W || ledX < 0) {
        continue; // Out of bounds
      }
      changedPixels += SetPixel(ledX, ledY, (line & (1 << (7-b))) ? fgColor : bgColor);
    }
  }

  return changedPixels;
}

int WriteCharAt(int x, int y, uint8_t charToPut, CRGB fgColor) {
  const int h = font16_10_height;
  const int cw = font16_10_charsPerLine;

  int cx = (charToPut & 0xF);
  int cy = ((charToPut & 0xF0) >> 4) * h;
  int changedPixels = 0;

  for(int l = 0; l < h; l++) {
    int ledY = y + l;
    if (ledY >= MATRIX_H || ledY < 0) {
      continue; // Out of bounds
    }
    // Put all lines
    uint8_t line = font16_10[cy * cw + l * cw + cx];
    for (int b = 0; b < font16_10_charWidth; b++) {
      int ledX = x + b;
      if (ledX >= MATRIX_W || ledX < 0) {
        continue; // Out of bounds
      }
      if ((line & (1 << (7-b)))) {
        changedPixels += SetPixel(ledX, ledY, fgColor);
      }
    }
  }

  return changedPixels;
}

void SetBrightness(float v) {
  brightness = (uint8_t) (v * 255);
  Serial.print("New Brightness: ");
  Serial.println(brightness);
}

void SetBackgroundBrightness(float v) {
  bgBrightness = (uint8_t) (v * 255);
  Serial.print("New BG Brightness: ");
  Serial.println(bgBrightness);
}