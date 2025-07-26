/*
 * Application Framework for ESP32-S3 Watch
 * Apple Watch-style app management
 */

#ifndef APPS_H
#define APPS_H

#include "config.h"

// App structure
struct WatchApp {
  AppType type;
  String name;
  String icon_file;
  uint16_t icon_color;
  bool installed;
  void (*init_func)();
  void (*draw_func)();
  void (*touch_func)(TouchGesture&);
  void (*cleanup_func)();
};

// Initialize app system
void initializeApps();

// App management
void launchApp(AppType app);
void exitCurrentApp();
void drawAppGrid();
void drawAppIcon(int x, int y, int size, WatchApp& app);

// App navigation
void handleAppGridTouch(TouchGesture& gesture);
void switchToApp(AppType app);

// Music Player App
void initMusicApp();
void drawMusicApp();
void handleMusicTouch(TouchGesture& gesture);
void playMusic();
void pauseMusic();
void nextTrack();
void previousTrack();

// Notes App  
void initNotesApp();
void drawNotesApp();
void handleNotesTouch(TouchGesture& gesture);
void saveNote();
void loadNote();

// File Browser App
void initFileBrowserApp();
void drawFileBrowserApp();
void handleFileBrowserTouch(TouchGesture& gesture);

// PDF Reader App
void initPDFReaderApp();
void drawPDFReaderApp();
void handlePDFReaderTouch(TouchGesture& gesture);

// Settings App
void initSettingsApp();
void drawSettingsApp();
void handleSettingsTouch(TouchGesture& gesture);

// Weather App
void initWeatherApp();
void drawWeatherApp();
void handleWeatherTouch(TouchGesture& gesture);

// App registry
extern WatchApp registered_apps[];
extern int num_registered_apps;
extern int current_app_index;

#endif // APPS_H