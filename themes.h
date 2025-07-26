/*
 * Theme Management for ESP32-S3 Watch
 * Luffy Gear 5, Sung Jin Woo, and Yugo Wakfu themes
 */

#ifndef THEMES_H
#define THEMES_H

#include "config.h"

// Theme color schemes
extern ThemeColors luffy_gear5_theme;
extern ThemeColors sung_jinwoo_theme;
extern ThemeColors yugo_wakfu_theme;

// Current active theme
extern ThemeColors* current_theme;

// Theme management functions
void initializeThemes();
void setTheme(ThemeType theme);
ThemeColors* getCurrentTheme();

// Theme-specific watch faces
void drawLuffyWatchFace();
void drawJinwooWatchFace();
void drawYugoWatchFace();
void drawSleepWatchFace();

// Theme-specific animations
void playLuffyAnimation();
void playJinwooAnimation();
void playYugoAnimation();

// Theme-specific UI elements
void drawThemeButton(int x, int y, int w, int h, const char* text, bool pressed);
void drawThemeProgressBar(int x, int y, int w, int h, float progress, const char* label);
void drawThemeNotification(const char* title, const char* message);

// Character-specific elements
void drawLuffyGear5Effects();
void drawJinwooShadows();
void drawYugoPortals();

// Activity ring themes
void drawLuffyActivityRings(int centerX, int centerY);
void drawJinwooActivityRings(int centerX, int centerY);
void drawYugoActivityRings(int centerX, int centerY);

#endif // THEMES_H