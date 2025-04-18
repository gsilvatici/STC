// DisplayManager.h
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>

// Defines
#define I2C_ADDRESS 0x3c // I2C addr 0x3C for SH1106
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   // QT-PY / XIAO

class DisplayManager {
public:
  DisplayManager();
  void initialize();
  void drawText(const char* text);
  void drawInt(int num);

  void print(const char* text);
  void println(const char* text);
  void drawBand(uint8_t band, uint8_t height, bool draw = false);
  void drawHorizontalLine(const uint8_t x, const uint8_t y, const uint8_t length, bool draw = false);
  void draw() { display.display(); }
  void clear() { display.clearDisplay(); }

  Adafruit_SH1106G display;

private:

  uint8_t cursorX;
  uint8_t cursorY;
};

#endif // DISPLAY_MANAGER_H
