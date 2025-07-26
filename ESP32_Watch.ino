/*
 * ESP32-S3 Apple Watch Style Smartwatch
 * Featuring Luffy Gear 5, Sung Jin Woo, and Yugo themes
 * 
 * Hardware: ESP32-S3-Touch-AMOLED-1.8
 * Display: 368x448 AMOLED with SH8601 driver + FT3168 touch
 * Memory: 16MB Flash + 8MB PSRAM
 * 
 * Features:
 * - Apple Watch-inspired interface
 * - Gamified step tracker with "urgent quests"
 * - Music player (auto-detect MP3)
 * - PDF reader (auto-detect PDF)
 * - Notes application
 * - Character-themed watch faces
 */

#include "config.h"
#include "display.h"
#include "touch.h"
#include "sensors.h"
#include "rtc.h"
#include "power.h"
#include "filesystem.h"
#include "ui.h"
#include "themes.h"
#include "apps.h"
#include "games.h"
#include "quests.h"

// Global system state
SystemState system_state;
unsigned long last_update = 0;
unsigned long last_sensor_update = 0;
unsigned long last_ui_update = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 Watch Starting...");
  
  // Initialize hardware components
  if (!initializePower()) {
    Serial.println("Power management failed to initialize!");
    return;
  }
  
  if (!initializeDisplay()) {
    Serial.println("Display failed to initialize!");
    return;
  }
  
  if (!initializeTouch()) {
    Serial.println("Touch controller failed to initialize!");
    return;
  }
  
  if (!initializeSensors()) {
    Serial.println("Sensors failed to initialize!");
    return;
  }
  
  if (!initializeRTC()) {
    Serial.println("RTC failed to initialize!");
    return;
  }
  
  if (!initializeFileSystem()) {
    Serial.println("File system failed to initialize!");
    return;
  }
  
  // Initialize UI system
  initializeUI();
  
  // Load user preferences
  loadUserSettings();
  
  // Set default theme (Luffy Gear 5)
  setTheme(THEME_LUFFY_GEAR5);
  
  // Show splash screen
  showSplashScreen();
  delay(2000);
  
  // Initialize apps
  initializeApps();
  
  // Go to watch face
  system_state.current_screen = SCREEN_WATCHFACE;
  system_state.sleep_timer = millis();
  
  Serial.println("ESP32-S3 Watch initialized successfully!");
}

void loop() {
  unsigned long current_time = millis();
  
  // Handle power management
  handlePowerManagement();
  
  // Update sensors (every 100ms)
  if (current_time - last_sensor_update >= 100) {
    updateSensors();
    last_sensor_update = current_time;
  }
  
  // Handle touch input
  TouchGesture gesture = handleTouchInput();
  if (gesture.event != TOUCH_NONE) {
    handleUITouch(gesture);
    
    // Handle specific screen touches
    switch (system_state.current_screen) {
      case SCREEN_WATCHFACE:
        if (gesture.event == TOUCH_SWIPE_UP) {
          system_state.current_screen = SCREEN_APP_GRID;
        }
        break;
      case SCREEN_APP_GRID:
        handleAppGridTouch(gesture);
        break;
      case SCREEN_QUESTS:
        // Quest touch handling would go here
        break;
      default:
        // Handle game touches when in games
        if (system_state.current_app == APP_GAMES) {
          if (current_game_session.current_game == GAME_BATTLE_ARENA) {
            handleBattleTouch(gesture);
          } else if (current_game_session.current_game == GAME_SHADOW_DUNGEON) {
            handleDungeonTouch(gesture);
          } else if (current_game_session.current_game == GAME_MINI_SNAKE) {
            handleSnakeTouch(gesture);
          } else if (current_game_session.state == GAME_MENU) {
            handleGameMenuTouch(gesture);
          }
        }
        break;
    }
    
    // Reset sleep timer on any touch
    system_state.sleep_timer = millis();
  }
  
  // Handle button input
  handleButtonInput();
  
  // Update UI (60 FPS target - every 16ms)
  if (current_time - last_ui_update >= 16) {
    updateUI();
    last_ui_update = current_time;
  }
  
  // Handle sleep mode
  handleSleepMode();
  
  // Small delay to prevent watchdog issues
  delay(1);
}

void showSplashScreen() {
  clearDisplay();
  
  // Show animated logo with current theme colors
  ThemeColors* theme = getCurrentTheme();
  
  // Draw centered logo
  drawCenteredText("ESP32-S3", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 40, theme->primary, 3);
  drawCenteredText("WATCH", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, theme->accent, 3);
  drawCenteredText("Loading...", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 40, theme->secondary, 1);
  
  // Animated loading bar
  int bar_width = 200;
  int bar_x = (DISPLAY_WIDTH - bar_width) / 2;
  int bar_y = DISPLAY_HEIGHT/2 + 70;
  
  for (int i = 0; i <= bar_width; i += 5) {
    drawRect(bar_x, bar_y, i, 4, theme->accent);
    updateDisplay();
    delay(20);
  }
}

void handlePowerManagement() {
  // Check battery level
  updateBatteryStatus();
  
  // Handle low battery warning
  if (system_state.battery_percentage < 10 && !system_state.low_battery_warning) {
    system_state.low_battery_warning = true;
    showLowBatteryWarning();
  }
  
  // Handle charging status
  if (system_state.is_charging && system_state.current_screen == SCREEN_SLEEP) {
    // Wake up when charging starts
    wakeFromSleep();
  }
}

void handleSleepMode() {
  // Auto sleep after inactivity
  if (millis() - system_state.sleep_timer > SLEEP_TIMEOUT && 
      system_state.current_screen != SCREEN_SLEEP) {
    goToSleep();
  }
}

void goToSleep() {
  system_state.current_screen = SCREEN_SLEEP;
  system_state.sleep_timer = millis();
  
  // Dim display but keep minimal watch face
  setDisplayBrightness(10);
  drawSleepWatchFace();
  
  // Reduce sensor polling
  system_state.low_power_mode = true;
}

void wakeFromSleep() {
  system_state.current_screen = SCREEN_WATCHFACE;
  system_state.sleep_timer = millis();
  system_state.low_power_mode = false;
  
  // Restore display brightness
  setDisplayBrightness(system_state.brightness);
  
  // Refresh display
  drawWatchFace();
}

void loadUserSettings() {
  // Load settings from EEPROM or SD card
  // Default values
  system_state.brightness = 80;
  system_state.current_theme = THEME_LUFFY_GEAR5;
  system_state.step_goal = 10000;
  system_state.wake_time = 7 * 60; // 7:00 AM
  system_state.sleep_time = 22 * 60; // 10:00 PM
  
  // Try to load from file system
  loadSettingsFromFile();
}

void saveUserSettings() {
  saveSettingsToFile();
}