/*
 * Display Management Implementation
 * SH8601 AMOLED Driver for ESP32-S3 Watch
 */

#include "display.h"
#include <math.h>

// TFT_eSPI instance
TFT_eSPI tft = TFT_eSPI();

// Display buffer for smooth operations
uint16_t* display_buffer = nullptr;
uint16_t* screen_capture = nullptr;

bool initializeDisplay() {
  Serial.println("Initializing AMOLED display...");
  
  // Allocate display buffer in PSRAM
  display_buffer = (uint16_t*)ps_malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
  screen_capture = (uint16_t*)ps_malloc(DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
  
  if (!display_buffer || !screen_capture) {
    Serial.println("Failed to allocate display buffers!");
    return false;
  }
  
  // Initialize TFT_eSPI
  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  tft.fillScreen(COLOR_BLACK);
  
  // Set default brightness
  setDisplayBrightness(80);
  
  Serial.println("AMOLED display initialized successfully");
  return true;
}

void clearDisplay() {
  tft.fillScreen(COLOR_BLACK);
  if (display_buffer) {
    memset(display_buffer, 0, DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
  }
}

void updateDisplay() {
  // Push buffer to display if using buffered mode
  if (display_buffer) {
    tft.pushImage(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, display_buffer);
  }
}

void setDisplayBrightness(int brightness) {
  // Control backlight via PWM
  int pwm_value = map(brightness, 0, 100, 0, 255);
  analogWrite(TFT_BL, pwm_value);
}

void drawPixel(int x, int y, uint16_t color) {
  if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT) {
    tft.drawPixel(x, y, color);
    if (display_buffer) {
      display_buffer[y * DISPLAY_WIDTH + x] = color;
    }
  }
}

void drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
  tft.drawLine(x0, y0, x1, y1, color);
}

void drawRect(int x, int y, int w, int h, uint16_t color) {
  tft.drawRect(x, y, w, h, color);
}

void fillRect(int x, int y, int w, int h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void drawCircle(int x, int y, int radius, uint16_t color) {
  tft.drawCircle(x, y, radius, color);
}

void fillCircle(int x, int y, int radius, uint16_t color) {
  tft.fillCircle(x, y, radius, color);
}

void drawRoundRect(int x, int y, int w, int h, int radius, uint16_t color) {
  tft.drawRoundRect(x, y, w, h, radius, color);
}

void fillRoundRect(int x, int y, int w, int h, int radius, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, radius, color);
}

void drawText(const char* text, int x, int y, uint16_t color, int size) {
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.setCursor(x, y);
  tft.print(text);
}

void drawCenteredText(const char* text, int x, int y, uint16_t color, int size) {
  tft.setTextColor(color);
  tft.setTextSize(size);
  
  int text_width = getTextWidth(text, size);
  int text_height = getTextHeight(size);
  
  tft.setCursor(x - text_width/2, y - text_height/2);
  tft.print(text);
}

int getTextWidth(const char* text, int size) {
  tft.setTextSize(size);
  return tft.textWidth(text);
}

int getTextHeight(int size) {
  return size * 8; // Basic font height approximation
}

void drawBitmap(int x, int y, int w, int h, const uint16_t* bitmap) {
  tft.pushImage(x, y, w, h, bitmap);
}

void drawSprite(int x, int y, int w, int h, const uint16_t* sprite) {
  // Draw sprite with transparency support
  for (int py = 0; py < h; py++) {
    for (int px = 0; px < w; px++) {
      uint16_t pixel = sprite[py * w + px];
      if (pixel != 0x0000) { // 0x0000 is transparent
        drawPixel(x + px, y + py, pixel);
      }
    }
  }
}

void drawGradient(int x, int y, int w, int h, uint16_t color1, uint16_t color2, bool vertical) {
  for (int i = 0; i < (vertical ? h : w); i++) {
    float ratio = (float)i / (vertical ? h : w);
    
    // Interpolate colors
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;
    
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;
    
    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;
    
    uint16_t color = (r << 11) | (g << 5) | b;
    
    if (vertical) {
      drawLine(x, y + i, x + w - 1, y + i, color);
    } else {
      drawLine(x + i, y, x + i, y + h - 1, color);
    }
  }
}

void drawProgressRing(int centerX, int centerY, int radius, float progress, uint16_t color, int thickness) {
  float start_angle = -PI/2; // Start at top
  float end_angle = start_angle + (2 * PI * progress);
  
  for (int t = 0; t < thickness; t++) {
    int r = radius - t;
    for (float angle = start_angle; angle <= end_angle; angle += 0.02) {
      int x = centerX + r * cos(angle);
      int y = centerY + r * sin(angle);
      drawPixel(x, y, color);
    }
  }
}

void drawActivityRing(int centerX, int centerY, int radius, float progress, uint16_t color, int thickness) {
  // Background ring (dimmed)
  uint16_t bg_color = color >> 2; // Dim the color
  for (int t = 0; t < thickness; t++) {
    int r = radius - t;
    for (float angle = 0; angle < 2 * PI; angle += 0.02) {
      int x = centerX + r * cos(angle);
      int y = centerY + r * sin(angle);
      drawPixel(x, y, bg_color);
    }
  }
  
  // Progress ring
  drawProgressRing(centerX, centerY, radius, progress, color, thickness);
}

void drawComplication(int x, int y, int w, int h, const char* title, const char* value, uint16_t color) {
  // Draw rounded background
  fillRoundRect(x, y, w, h, 8, color >> 3); // Dim background
  drawRoundRect(x, y, w, h, 8, color);
  
  // Draw title (smaller text)
  drawCenteredText(title, x + w/2, y + h/3, color, 1);
  
  // Draw value (larger text)
  drawCenteredText(value, x + w/2, y + 2*h/3, color, 2);
}

void fadeIn(int duration) {
  for (int brightness = 0; brightness <= 100; brightness += 5) {
    setDisplayBrightness(brightness);
    delay(duration / 20);
  }
}

void fadeOut(int duration) {
  for (int brightness = 100; brightness >= 0; brightness -= 5) {
    setDisplayBrightness(brightness);
    delay(duration / 20);
  }
}

void slideTransition(int direction, int duration) {
  // direction: 0=up, 1=down, 2=left, 3=right
  captureScreen();
  
  int steps = 20;
  int step_delay = duration / steps;
  
  for (int step = 0; step <= steps; step++) {
    clearDisplay();
    
    int offset = map(step, 0, steps, 0, (direction < 2 ? DISPLAY_HEIGHT : DISPLAY_WIDTH));
    
    // Draw old screen sliding out
    int old_x = 0, old_y = 0;
    switch (direction) {
      case 0: old_y = -offset; break; // up
      case 1: old_y = offset; break;  // down
      case 2: old_x = -offset; break; // left
      case 3: old_x = offset; break;  // right
    }
    
    if (old_x > -DISPLAY_WIDTH && old_x < DISPLAY_WIDTH && 
        old_y > -DISPLAY_HEIGHT && old_y < DISPLAY_HEIGHT) {
      tft.pushImage(old_x, old_y, DISPLAY_WIDTH, DISPLAY_HEIGHT, screen_capture);
    }
    
    delay(step_delay);
  }
}

void pushTransition(int direction, int duration) {
  // Apple Watch-style push transition
  slideTransition(direction, duration);
}

void captureScreen() {
  if (screen_capture && display_buffer) {
    memcpy(screen_capture, display_buffer, DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
  }
}

void restoreScreen() {
  if (screen_capture && display_buffer) {
    memcpy(display_buffer, screen_capture, DISPLAY_WIDTH * DISPLAY_HEIGHT * 2);
    updateDisplay();
  }
}

void loadCustomFonts() {
  // Load custom fonts for better typography
  // This would typically load from SPIFFS or SD card
}

void loadIcons() {
  // Load app icons and system icons
  // This would typically load from SPIFFS or SD card
}