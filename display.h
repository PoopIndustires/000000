/*
 * Display Management for ESP32-S3 Watch
 * SH8601 AMOLED Driver Interface
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include <TFT_eSPI.h>
#include <SPI.h>

// Display buffer for smooth animations
extern uint16_t* display_buffer;

// Initialize display system
bool initializeDisplay();

// Basic display operations
void clearDisplay();
void updateDisplay();
void setDisplayBrightness(int brightness);

// Drawing primitives
void drawPixel(int x, int y, uint16_t color);
void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
void drawRect(int x, int y, int w, int h, uint16_t color);
void fillRect(int x, int y, int w, int h, uint16_t color);
void drawCircle(int x, int y, int radius, uint16_t color);
void fillCircle(int x, int y, int radius, uint16_t color);
void drawRoundRect(int x, int y, int w, int h, int radius, uint16_t color);
void fillRoundRect(int x, int y, int w, int h, int radius, uint16_t color);

// Text rendering
void drawText(const char* text, int x, int y, uint16_t color, int size);
void drawCenteredText(const char* text, int x, int y, uint16_t color, int size);
int getTextWidth(const char* text, int size);
int getTextHeight(int size);

// Advanced graphics
void drawBitmap(int x, int y, int w, int h, const uint16_t* bitmap);
void drawSprite(int x, int y, int w, int h, const uint16_t* sprite);
void drawGradient(int x, int y, int w, int h, uint16_t color1, uint16_t color2, bool vertical);

// Apple Watch style elements
void drawProgressRing(int centerX, int centerY, int radius, float progress, uint16_t color, int thickness);
void drawActivityRing(int centerX, int centerY, int radius, float progress, uint16_t color, int thickness);
void drawComplication(int x, int y, int w, int h, const char* title, const char* value, uint16_t color);

// Animation support
void fadeIn(int duration);
void fadeOut(int duration);
void slideTransition(int direction, int duration);
void pushTransition(int direction, int duration);

// Screen capture for transitions
void captureScreen();
void restoreScreen();

// Font and icon management
void loadCustomFonts();
void loadIcons();

#endif // DISPLAY_H