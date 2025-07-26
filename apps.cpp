/*
 * Application Framework Implementation
 * Apple Watch-style app management for ESP32-S3 Watch
 */

#include "apps.h"
#include "display.h"
#include "themes.h"
#include "ui.h"
#include "games.h"
#include "music_app.cpp"
#include "quests.h"

// App registry
WatchApp registered_apps[] = {
  {APP_WATCHFACE, "Watch", "", COLOR_WHITE, true, nullptr, drawWatchFace, nullptr, nullptr},
  {APP_QUESTS, "Quests", "", LUFFY_GOLD, true, initializeQuests, drawQuestScreen, nullptr, nullptr},
  {APP_MUSIC, "Music", "", COLOR_BLUE, true, initMusicApp, drawMusicApp, handleMusicTouch, nullptr},
  {APP_GAMES, "Games", "", COLOR_GREEN, true, initializeGames, drawGameMenu, handleGameMenuTouch, nullptr},
  {APP_NOTES, "Notes", "", COLOR_YELLOW, true, initNotesApp, drawNotesApp, handleNotesTouch, nullptr},
  {APP_FILES, "Files", "", COLOR_ORANGE, true, initFileBrowserApp, drawFileBrowserApp, handleFileBrowserTouch, nullptr},
  {APP_PDF_READER, "PDF", "", COLOR_RED, true, initPDFReaderApp, drawPDFReaderApp, handlePDFReaderTouch, nullptr},
  {APP_SETTINGS, "Settings", "", COLOR_PURPLE, true, initSettingsApp, drawSettingsApp, handleSettingsTouch, nullptr},
  {APP_WEATHER, "Weather", "", COLOR_CYAN, true, initWeatherApp, drawWeatherApp, handleWeatherTouch, nullptr}
};

int num_registered_apps = sizeof(registered_apps) / sizeof(registered_apps[0]);
int current_app_index = 0;

void initializeApps() {
  Serial.println("Initializing applications...");
  
  // Initialize all apps
  for (int i = 0; i < num_registered_apps; i++) {
    if (registered_apps[i].init_func != nullptr) {
      registered_apps[i].init_func();
    }
  }
  
  Serial.println("Applications initialized successfully");
}

void launchApp(AppType app) {
  system_state.current_app = app;
  system_state.current_screen = SCREEN_APP_GRID;
  
  // Find and initialize the specific app
  for (int i = 0; i < num_registered_apps; i++) {
    if (registered_apps[i].type == app) {
      current_app_index = i;
      if (registered_apps[i].init_func != nullptr) {
        registered_apps[i].init_func();
      }
      break;
    }
  }
  
  Serial.println("Launched app: " + String((int)app));
}

void exitCurrentApp() {
  // Run cleanup function if available
  if (registered_apps[current_app_index].cleanup_func != nullptr) {
    registered_apps[current_app_index].cleanup_func();
  }
  
  // Return to watchface
  system_state.current_screen = SCREEN_WATCHFACE;
  system_state.current_app = APP_WATCHFACE;
}

void drawAppGrid() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Background
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Title
  drawNavigationBar("Apps", false);
  
  // App grid (3x3 layout)
  int apps_per_row = 3;
  int app_size = 60;
  int spacing = 20;
  int grid_width = apps_per_row * app_size + (apps_per_row - 1) * spacing;
  int start_x = (DISPLAY_WIDTH - grid_width) / 2;
  int start_y = 80;
  
  for (int i = 0; i < num_registered_apps && i < 9; i++) {
    int row = i / apps_per_row;
    int col = i % apps_per_row;
    
    int x = start_x + col * (app_size + spacing);
    int y = start_y + row * (app_size + spacing + 20);
    
    drawAppIcon(x, y, app_size, registered_apps[i]);
    
    // App name
    drawCenteredText(registered_apps[i].name.c_str(), x + app_size/2, y + app_size + 10, theme->text, 1);
  }
  
  // Instructions
  drawCenteredText("Tap app to launch", DISPLAY_WIDTH/2, start_y + 280, theme->secondary, 1);
  
  updateDisplay();
}

void drawAppIcon(int x, int y, int size, WatchApp& app) {
  ThemeColors* theme = getCurrentTheme();
  
  // App icon background
  fillRoundRect(x, y, size, size, size/8, app.icon_color);
  drawRoundRect(x, y, size, size, size/8, theme->accent);
  
  // Simple icon based on app type
  int center_x = x + size/2;
  int center_y = y + size/2;
  
  switch (app.type) {
    case APP_WATCHFACE:
      // Clock icon
      drawCircle(center_x, center_y, size/3, COLOR_BLACK);
      drawLine(center_x, center_y, center_x, center_y - size/4, COLOR_BLACK);
      drawLine(center_x, center_y, center_x + size/5, center_y, COLOR_BLACK);
      break;
      
    case APP_QUESTS:
      // Quest scroll icon
      fillRect(center_x - size/4, center_y - size/3, size/2, size/1.5, COLOR_BLACK);
      drawText("!", center_x - 3, center_y - 4, COLOR_RED, 1);
      break;
      
    case APP_MUSIC:
      // Music note icon
      fillCircle(center_x - size/6, center_y + size/6, size/8, COLOR_BLACK);
      fillRect(center_x + size/12, center_y - size/3, 3, size/2, COLOR_BLACK);
      break;
      
    case APP_GAMES:
      // Game controller icon
      fillRoundRect(center_x - size/4, center_y - size/6, size/2, size/3, size/12, COLOR_BLACK);
      fillCircle(center_x - size/8, center_y, 3, COLOR_RED);
      fillCircle(center_x + size/8, center_y, 3, COLOR_BLUE);
      break;
      
    case APP_NOTES:
      // Note icon
      fillRect(center_x - size/4, center_y - size/3, size/2, size/1.5, COLOR_BLACK);
      for (int i = 0; i < 3; i++) {
        drawLine(center_x - size/5, center_y - size/5 + i*6, center_x + size/5, center_y - size/5 + i*6, COLOR_WHITE);
      }
      break;
      
    case APP_FILES:
      // Folder icon
      fillRect(center_x - size/4, center_y - size/6, size/2, size/3, COLOR_BLACK);
      fillRect(center_x - size/3, center_y - size/4, size/6, size/8, COLOR_BLACK);
      break;
      
    case APP_SETTINGS:
      // Gear icon
      drawCircle(center_x, center_y, size/4, COLOR_BLACK);
      drawCircle(center_x, center_y, size/6, theme->background);
      for (int i = 0; i < 8; i++) {
        float angle = i * PI / 4;
        int x1 = center_x + (size/4 + 5) * cos(angle);
        int y1 = center_y + (size/4 + 5) * sin(angle);
        int x2 = center_x + (size/4 + 8) * cos(angle);
        int y2 = center_y + (size/4 + 8) * sin(angle);
        drawLine(x1, y1, x2, y2, COLOR_BLACK);
      }
      break;
      
    default:
      // Default app icon
      fillCircle(center_x, center_y, size/4, COLOR_BLACK);
      break;
  }
}

void handleAppGridTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  // Calculate which app was tapped
  int apps_per_row = 3;
  int app_size = 60;
  int spacing = 20;
  int grid_width = apps_per_row * app_size + (apps_per_row - 1) * spacing;
  int start_x = (DISPLAY_WIDTH - grid_width) / 2;
  int start_y = 80;
  
  for (int i = 0; i < num_registered_apps && i < 9; i++) {
    int row = i / apps_per_row;
    int col = i % apps_per_row;
    
    int x = start_x + col * (app_size + spacing);
    int y = start_y + row * (app_size + spacing + 20);
    
    if (gesture.x >= x && gesture.x <= x + app_size &&
        gesture.y >= y && gesture.y <= y + app_size) {
      
      launchApp(registered_apps[i].type);
      return;
    }
  }
}

void switchToApp(AppType app) {
  launchApp(app);
}

// ==================== NOTES APP ====================
void initNotesApp() {
  Serial.println("Notes app initialized");
}

void drawNotesApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("Notes", true);
  
  drawCenteredText("Simple Notes App", DISPLAY_WIDTH/2, 100, theme->text, 2);
  drawCenteredText("Current note:", DISPLAY_WIDTH/2, 140, theme->secondary, 1);
  
  // Note content area
  fillRoundRect(20, 160, DISPLAY_WIDTH - 40, 100, 8, theme->shadow);
  drawRoundRect(20, 160, DISPLAY_WIDTH - 40, 100, 8, theme->secondary);
  
  if (system_state.current_note.length() > 0) {
    drawText(system_state.current_note.c_str(), 30, 180, theme->text, 1);
  } else {
    drawText("No notes yet...", 30, 180, theme->secondary, 1);
  }
  
  // Buttons
  drawGameButton(20, 280, DISPLAY_WIDTH - 40, 40, "New Note", false);
  drawGameButton(20, 330, DISPLAY_WIDTH - 40, 40, "Save Note", false);
  drawGameButton(20, 380, DISPLAY_WIDTH - 40, 40, "Load Note", false);
  
  updateDisplay();
}

void handleNotesTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  // Simple note handling - in a real implementation this would have text input
  if (gesture.y >= 280 && gesture.y <= 320) {
    system_state.current_note = "Sample note created at " + String(millis());
  } else if (gesture.y >= 330 && gesture.y <= 370) {
    saveNote();
  } else if (gesture.y >= 380 && gesture.y <= 420) {
    loadNote();
  }
}

void saveNote() {
  // Save current note to file system
  writeTextFile("/notes.txt", system_state.current_note);
}

void loadNote() {
  // Load note from file system
  system_state.current_note = readTextFile("/notes.txt");
}

// ==================== FILE BROWSER APP ====================
void initFileBrowserApp() {
  Serial.println("File browser app initialized");
}

void drawFileBrowserApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("File Browser", true);
  
  drawCenteredText("SD Card Files", DISPLAY_WIDTH/2, 60, theme->text, 2);
  
  // File list (simplified)
  char file_info[100];
  sprintf(file_info, "Music Files: %d", total_music_files);
  drawText(file_info, 20, 100, theme->text, 1);
  
  sprintf(file_info, "PDF Files: %d", total_pdf_files);
  drawText(file_info, 20, 120, theme->text, 1);
  
  // Show some file names
  if (total_music_files > 0) {
    drawText("♪ " + music_files[0].title, 20, 150, theme->accent, 1);
  }
  
  if (total_pdf_files > 0) {
    drawText("📄 " + pdf_files[0].title, 20, 170, theme->accent, 1);
  }
  
  drawGameButton(20, 300, DISPLAY_WIDTH - 40, 40, "Refresh Files", false);
  
  updateDisplay();
}

void handleFileBrowserTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP && gesture.y >= 300 && gesture.y <= 340) {
    // Refresh file list
    total_music_files = scanMusicFiles(music_files, 100);
    total_pdf_files = scanPDFFiles(pdf_files, 50);
  }
}

// ==================== PDF READER APP ====================
void initPDFReaderApp() {
  Serial.println("PDF reader app initialized");
}

void drawPDFReaderApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("PDF Reader", true);
  
  drawCenteredText("PDF Viewer", DISPLAY_WIDTH/2, 100, theme->text, 2);
  
  if (total_pdf_files > 0) {
    drawCenteredText("Current PDF:", DISPLAY_WIDTH/2, 140, theme->secondary, 1);
    drawCenteredText(pdf_files[0].title, DISPLAY_WIDTH/2, 160, theme->text, 1);
    
    // PDF content area (placeholder)
    fillRoundRect(20, 180, DISPLAY_WIDTH - 40, 120, 8, COLOR_WHITE);
    drawRoundRect(20, 180, DISPLAY_WIDTH - 40, 120, 8, theme->secondary);
    drawCenteredText("PDF content would", DISPLAY_WIDTH/2, 220, COLOR_BLACK, 1);
    drawCenteredText("be displayed here", DISPLAY_WIDTH/2, 240, COLOR_BLACK, 1);
    
    drawGameButton(20, 320, 80, 30, "Prev", false);
    drawGameButton(120, 320, 80, 30, "Next", false);
    drawGameButton(220, 320, 80, 30, "Zoom", false);
  } else {
    drawCenteredText("No PDF files found", DISPLAY_WIDTH/2, 200, theme->secondary, 1);
    drawCenteredText("Add PDF files to SD card", DISPLAY_WIDTH/2, 220, theme->secondary, 1);
  }
  
  updateDisplay();
}

void handlePDFReaderTouch(TouchGesture& gesture) {
  // Basic PDF navigation would be implemented here
}

// ==================== SETTINGS APP ====================
void initSettingsApp() {
  Serial.println("Settings app initialized");
}

void drawSettingsApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("Settings", true);
  
  // Theme selection
  drawText("Theme:", 20, 80, theme->text, 1);
  drawGameButton(20, 100, 100, 30, "Luffy", system_state.current_theme == THEME_LUFFY_GEAR5);
  drawGameButton(130, 100, 100, 30, "Jin Woo", system_state.current_theme == THEME_SUNG_JINWOO);
  drawGameButton(240, 100, 100, 30, "Yugo", system_state.current_theme == THEME_YUGO_WAKFU);
  
  // Brightness
  drawText("Brightness: " + String(system_state.brightness) + "%", 20, 160, theme->text, 1);
  drawRect(20, 180, DISPLAY_WIDTH - 40, 20, theme->secondary);
  fillRect(20, 180, (DISPLAY_WIDTH - 40) * system_state.brightness / 100, 20, theme->accent);
  
  // Step goal
  drawText("Step Goal: " + String(system_state.step_goal), 20, 220, theme->text, 1);
  drawGameButton(20, 240, 80, 30, "5000", system_state.step_goal == 5000);
  drawGameButton(110, 240, 80, 30, "10000", system_state.step_goal == 10000);
  drawGameButton(200, 240, 80, 30, "15000", system_state.step_goal == 15000);
  
  // System info
  drawText("Battery: " + String(system_state.battery_percentage) + "%", 20, 300, theme->text, 1);
  drawText("Steps Today: " + String(system_state.steps_today), 20, 320, theme->text, 1);
  
  updateDisplay();
}

void handleSettingsTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  // Theme selection
  if (gesture.y >= 100 && gesture.y <= 130) {
    if (gesture.x >= 20 && gesture.x <= 120) {
      setTheme(THEME_LUFFY_GEAR5);
      system_state.current_theme = THEME_LUFFY_GEAR5;
    } else if (gesture.x >= 130 && gesture.x <= 230) {
      setTheme(THEME_SUNG_JINWOO);
      system_state.current_theme = THEME_SUNG_JINWOO;
    } else if (gesture.x >= 240 && gesture.x <= 340) {
      setTheme(THEME_YUGO_WAKFU);
      system_state.current_theme = THEME_YUGO_WAKFU;
    }
  }
  
  // Brightness control
  if (gesture.y >= 180 && gesture.y <= 200) {
    system_state.brightness = map(gesture.x, 20, DISPLAY_WIDTH - 20, 10, 100);
    setDisplayBrightness(system_state.brightness);
  }
  
  // Step goal selection
  if (gesture.y >= 240 && gesture.y <= 270) {
    if (gesture.x >= 20 && gesture.x <= 100) {
      system_state.step_goal = 5000;
    } else if (gesture.x >= 110 && gesture.x <= 190) {
      system_state.step_goal = 10000;
    } else if (gesture.x >= 200 && gesture.x <= 280) {
      system_state.step_goal = 15000;
    }
  }
}

// ==================== WEATHER APP ====================
void initWeatherApp() {
  Serial.println("Weather app initialized");
}

void drawWeatherApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("Weather", true);
  
  drawCenteredText("Weather Forecast", DISPLAY_WIDTH/2, 80, theme->text, 2);
  
  // Weather icon (sun)
  fillCircle(DISPLAY_WIDTH/2, 140, 25, COLOR_YELLOW);
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int x1 = DISPLAY_WIDTH/2 + 35 * cos(angle);
    int y1 = 140 + 35 * sin(angle);
    int x2 = DISPLAY_WIDTH/2 + 45 * cos(angle);
    int y2 = 140 + 45 * sin(angle);
    drawLine(x1, y1, x2, y2, COLOR_YELLOW);
  }
  
  // Temperature (simulated)
  drawCenteredText("24°C", DISPLAY_WIDTH/2, 200, theme->text, 3);
  drawCenteredText("Sunny", DISPLAY_WIDTH/2, 240, theme->secondary, 1);
  
  // Additional info
  drawText("Humidity: 65%", 20, 280, theme->text, 1);
  drawText("Wind: 12 km/h", 20, 300, theme->text, 1);
  drawText("Pressure: 1013 hPa", 20, 320, theme->text, 1);
  
  drawCenteredText("Tap to refresh", DISPLAY_WIDTH/2, 380, theme->secondary, 1);
  
  updateDisplay();
}

void handleWeatherTouch(TouchGesture& gesture) {
  // Weather refresh would be implemented here
  // Could connect to WiFi and fetch real weather data
}