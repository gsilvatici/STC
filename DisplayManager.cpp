#include "DisplayManager.h"

#include "AudioManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), cursorX(0), cursorY(0) {
}

void DisplayManager::initialize() {
  delay(250); // wait for the OLED to power up
  display.begin(I2C_ADDRESS, true);
  // display.setFont(Dialog_plain_8);

  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
}

void DisplayManager::drawText(const char* text) 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Display text
  display.setCursor(0, SCREEN_HEIGHT - 10);
  display.print(text);

  display.display();
}

void DisplayManager::drawInt(int num) 
{
  display.clearDisplay(); // Clear the display buffer
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Set cursor position to display the number
  display.setCursor(0, SCREEN_HEIGHT - 10);

  // Print the integer value on the display
  display.print(num);

  display.display(); // Show the buffer content on the display
}

void DisplayManager::print(const char* text) 
{
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Set cursor to the current position
  display.setCursor(cursorX, cursorY);
  display.print(text);
  
  // Update cursorX to move to the right of the printed text
  cursorX += display.getCursorX(); // Move cursor X to the end of the printed text

  // Check if the cursor has reached the end of the display width
  if (cursorX >= SCREEN_WIDTH) 
  {
    cursorX = 0; // Reset to start of the line
    cursorY += 10; // Move to the next line
  }

  // If the display is full, clear the screen and reset cursor
  if (cursorY >= SCREEN_HEIGHT) 
  {
    display.clearDisplay();
    cursorX = 0; // Reset to start of the line
    cursorY = 0; // Reset to top
  }

  display.display();
}

void DisplayManager::println(const char* text) 
{
  print(text); // Print the text

  cursorX = 0; // Reset cursor X
  cursorY += 10; // Move to the next line

  // If the display is full, clear the screen and reset cursor
  if (cursorY >= SCREEN_HEIGHT) 
  {
    display.clearDisplay();
    cursorX = 0; // Reset to start of the line
    cursorY = 0; // Reset to top
  }

  display.display();
}

void DisplayManager::drawHorizontalLine(const uint8_t x, const uint8_t y, const uint8_t length, bool draw)
{
  display.drawLine(x, y, x + length, y, SH110X_WHITE);

  if (draw)
    display.display();
}

void DisplayManager::drawBand(uint8_t band, uint8_t height, bool draw)
{  
    if (height >= SCREEN_HEIGHT)
      height = SCREEN_HEIGHT;
    int bandGap = 2;
    int bandPosX = (SCREEN_WIDTH/BANDS_COUNT)*band;
    int bandWidth = (SCREEN_WIDTH/BANDS_COUNT) - bandGap;
    for (int s = 0; s <= height; s++)
        drawHorizontalLine(bandPosX, SCREEN_HEIGHT - s, bandWidth, draw);

    if (draw)
        display.display();
}
