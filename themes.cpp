/*
 * Theme Management Implementation
 * Anime-inspired themes for ESP32-S3 Watch
 */

#include "themes.h"
#include "display.h"
#include <math.h>

// Luffy Gear 5 Theme (White/Gold Sun God Nika)
ThemeColors luffy_gear5_theme = {
  .primary = LUFFY_WHITE,
  .secondary = LUFFY_GOLD,
  .accent = LUFFY_CREAM,
  .background = COLOR_BLACK,
  .text = LUFFY_WHITE,
  .shadow = LUFFY_SHADOW
};

// Sung Jin Woo Theme (Shadow Monarch)
ThemeColors sung_jinwoo_theme = {
  .primary = JINWOO_PURPLE,
  .secondary = JINWOO_DARK,
  .accent = JINWOO_VIOLET,
  .background = COLOR_BLACK,
  .text = JINWOO_SILVER,
  .shadow = JINWOO_DARK
};

// Yugo Wakfu Theme (Portal Master)
ThemeColors yugo_wakfu_theme = {
  .primary = YUGO_TEAL,
  .secondary = YUGO_BLUE,
  .accent = YUGO_ENERGY,
  .background = COLOR_BLACK,
  .text = YUGO_LIGHT_BLUE,
  .shadow = YUGO_BLUE
};

ThemeColors* current_theme = &luffy_gear5_theme;

void initializeThemes() {
  current_theme = &luffy_gear5_theme;
}

void setTheme(ThemeType theme) {
  switch (theme) {
    case THEME_LUFFY_GEAR5:
      current_theme = &luffy_gear5_theme;
      break;
    case THEME_SUNG_JINWOO:
      current_theme = &sung_jinwoo_theme;
      break;
    case THEME_YUGO_WAKFU:
      current_theme = &yugo_wakfu_theme;
      break;
  }
}

ThemeColors* getCurrentTheme() {
  return current_theme;
}

void drawLuffyWatchFace() {
  clearDisplay();
  
  // Background gradient (black to cream)
  drawGradient(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, LUFFY_CREAM, false);
  
  // Main time display
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char time_str[6];
  sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Large time display with Gear 5 styling
  drawCenteredText(time_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 50, LUFFY_WHITE, 4);
  
  // Date
  char date_str[20];
  strftime(date_str, sizeof(date_str), "%a %b %d", timeinfo);
  drawCenteredText(date_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, LUFFY_GOLD, 1);
  
  // Activity rings (Luffy style)
  drawLuffyActivityRings(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 80);
  
  // Sun symbol (Nika reference)
  int sun_x = DISPLAY_WIDTH/2;
  int sun_y = DISPLAY_HEIGHT/2 + 150;
  fillCircle(sun_x, sun_y, 15, LUFFY_GOLD);
  
  // Sun rays
  for (int i = 0; i < 8; i++) {
    float angle = (2 * PI * i) / 8;
    int x1 = sun_x + 20 * cos(angle);
    int y1 = sun_y + 20 * sin(angle);
    int x2 = sun_x + 30 * cos(angle);
    int y2 = sun_y + 30 * sin(angle);
    drawLine(x1, y1, x2, y2, LUFFY_GOLD);
  }
  
  // Battery indicator
  int battery_x = DISPLAY_WIDTH - 50;
  int battery_y = 30;
  drawRect(battery_x, battery_y, 30, 15, LUFFY_WHITE);
  fillRect(battery_x + 2, battery_y + 2, 26 * system_state.battery_percentage / 100, 11, LUFFY_GOLD);
  
  updateDisplay();
}

void drawJinwooWatchFace() {
  clearDisplay();
  
  // Dark background with purple gradient
  drawGradient(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, JINWOO_DARK, true);
  
  // Time display
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char time_str[6];
  sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Glowing purple time
  drawCenteredText(time_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 50, JINWOO_PURPLE, 4);
  
  // "ARISE" text when new notification/quest
  if (system_state.current_quest > 0) {
    drawCenteredText("ARISE", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 100, JINWOO_VIOLET, 2);
  }
  
  // Level display (based on steps)
  char level_str[20];
  int level = system_state.steps_today / 1000 + 1;
  sprintf(level_str, "LVL %d", level);
  drawCenteredText(level_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 10, JINWOO_SILVER, 1);
  
  // Shadow army activity rings
  drawJinwooActivityRings(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 80);
  
  // Shadow effects
  drawJinwooShadows();
  
  updateDisplay();
}

void drawYugoWatchFace() {
  clearDisplay();
  
  // Energy portal background
  drawGradient(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK, YUGO_BLUE, false);
  
  // Time display
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char time_str[6];
  sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Wakfu-style time display
  drawCenteredText(time_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 50, YUGO_ENERGY, 4);
  
  // Portal rings
  for (int r = 30; r <= 90; r += 20) {
    drawCircle(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 80, r, YUGO_TEAL);
  }
  
  // Wakfu energy meter
  drawYugoActivityRings(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 80);
  
  // Portal effects
  drawYugoPortals();
  
  updateDisplay();
}

void drawSleepWatchFace() {
  clearDisplay();
  
  // Dim display for sleep mode
  ThemeColors* theme = getCurrentTheme();
  uint16_t dim_color = theme->primary >> 2; // Very dim
  
  // Just show time
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char time_str[6];
  sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  drawCenteredText(time_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, dim_color, 2);
  
  updateDisplay();
}

void drawLuffyActivityRings(int centerX, int centerY) {
  // Stretch activity ring (red)
  float stretch_progress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX - 40, centerY, 25, stretch_progress, COLOR_RED, 4);
  
  // Gear activity ring (gold)
  float gear_progress = 0.7; // Example progress
  drawActivityRing(centerX, centerY, 25, gear_progress, LUFFY_GOLD, 4);
  
  // Freedom ring (white)
  float freedom_progress = 0.5; // Example progress
  drawActivityRing(centerX + 40, centerY, 25, freedom_progress, LUFFY_WHITE, 4);
}

void drawJinwooActivityRings(int centerX, int centerY) {
  // Shadow power rings
  float shadow_progress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX - 40, centerY, 25, shadow_progress, JINWOO_PURPLE, 4);
  
  // Monarch ring
  float monarch_progress = 0.8;
  drawActivityRing(centerX, centerY, 25, monarch_progress, JINWOO_VIOLET, 4);
  
  // Army ring
  float army_progress = 0.6;
  drawActivityRing(centerX + 40, centerY, 25, army_progress, JINWOO_SILVER, 4);
}

void drawYugoActivityRings(int centerX, int centerY) {
  // Wakfu energy rings
  float wakfu_progress = (float)system_state.steps_today / system_state.step_goal;
  drawActivityRing(centerX - 40, centerY, 25, wakfu_progress, YUGO_ENERGY, 4);
  
  // Portal ring
  float portal_progress = 0.9;
  drawActivityRing(centerX, centerY, 25, portal_progress, YUGO_TEAL, 4);
  
  // Brotherhood ring
  float brotherhood_progress = 0.7;
  drawActivityRing(centerX + 40, centerY, 25, brotherhood_progress, YUGO_LIGHT_BLUE, 4);
}

void playLuffyAnimation() {
  // Stretchy bounce animation
  for (int i = 0; i < 10; i++) {
    // Animate stretch effect
    delay(100);
  }
}

void playJinwooAnimation() {
  // Shadow rising animation
  for (int i = 0; i < 20; i++) {
    // Animate shadows rising from bottom
    delay(50);
  }
}

void playYugoAnimation() {
  // Portal opening animation
  for (int r = 5; r <= 50; r += 5) {
    clearDisplay();
    drawCircle(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, r, YUGO_ENERGY);
    updateDisplay();
    delay(100);
  }
}