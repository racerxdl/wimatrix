#define FASTLED_INTERNAL
#include <FastLED.h>

#ifndef _LED_CONTROL_
#define _LED_CONTROL_
#define MATRIX_W 30
#define MATRIX_H 10
// How many leds in your strip?
#define NUM_LEDS (MATRIX_W * MATRIX_H)

// GPIO5 - D1
#define DATA_PIN 1


// Controller State
#define STRING_DISPLAY 0
#define BACKGROUND_ONLY 1
#define BACKGROUND_STRING_DISPLAY 2
#define MODE_CLOCK 3
#define BACKGROUND_MODE_CLOCK 4


// Methods
void SetupLeds();
void LedLoop();

void SetBrightness(float v);
void SetBackgroundBrightness(float v);
void Clear();

void ResetToBackground();

void SetBackground(int x, int y, CRGB color);
void SetMode(int mode);
void LedPrint(const char *text, CRGB color);
void LedPrintAt(int x, int y, const char *text, CRGB color);

int WriteCharAt(int x, int y, uint8_t charToPut, CRGB fgColor);
int WriteCharAt(int x, int y, uint8_t charToPut, CRGB fgColor, CRGB bgColor);

int WriteStringAt(int x, int y, const char *str, CRGB fgColor);
int WriteStringAt(int x, int y, const char *str, CRGB fgColor, CRGB bgColor);
#endif