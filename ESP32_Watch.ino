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
    switch (system_state.current_screen) {
      case SCREEN_WATCHFACE:
        drawWatchFace();
        break;
      case SCREEN_APP_GRID:
        drawAppGrid();
        break;
      case SCREEN_MUSIC:
        drawMusicApp();
        break;
      case SCREEN_QUESTS:
        drawQuestScreen();
        break;
      case SCREEN_SETTINGS:
        drawSettingsApp();
        break;
      case SCREEN_NOTES:
        drawNotesApp();
        break;
      case SCREEN_FILE_BROWSER:
        drawFileBrowserApp();
        break;
      case SCREEN_PDF_READER:
        drawPDFReaderApp();
        break;
      default:
        // Handle game drawing
        if (system_state.current_app == APP_GAMES) {
          if (current_game_session.state == GAME_MENU) {
            drawGameMenu();
          } else if (current_game_session.current_game == GAME_BATTLE_ARENA) {
            drawBattleArena();
          } else if (current_game_session.current_game == GAME_SHADOW_DUNGEON) {
            drawShadowDungeon();
          } else if (current_game_session.current_game == GAME_MINI_SNAKE) {
            drawSnakeGame();
            updateSnake(); // Update snake game logic
          }
        } else {
          drawWatchFace();
        }
        break;
    }
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

// Watch face drawing functions
void drawWatchFace() {
  switch (system_state.current_theme) {
    case THEME_LUFFY_GEAR5:
      drawLuffyWatchFace();
      break;
    case THEME_SUNG_JINWOO:
      drawJinwooWatchFace();
      break;
    case THEME_YUGO_WAKFU:
      drawYugoWatchFace();
      break;
    default:
      drawLuffyWatchFace();
      break;
  }
}

// Button handling
void handleButtonInput() {
  static bool pwr_button_pressed = false;
  static bool boot_button_pressed = false;
  static unsigned long button_press_time = 0;
  
  // PWR button (GPIO 0)
  bool pwr_current = !digitalRead(BTN_PWR); // Active low
  if (pwr_current && !pwr_button_pressed) {
    pwr_button_pressed = true;
    button_press_time = millis();
  } else if (!pwr_current && pwr_button_pressed) {
    pwr_button_pressed = false;
    unsigned long press_duration = millis() - button_press_time;
    
    if (press_duration < 300) {
      // Short press - wake/sleep
      if (system_state.current_screen == SCREEN_SLEEP) {
        wakeFromSleep();
      } else {
        goToSleep();
      }
    } else if (press_duration > 2000) {
      // Long press - power off
      Serial.println("Power off requested");
      goToSleep();
    }
  }
  
  // BOOT button (GPIO 46) - Menu/Back
  bool boot_current = !digitalRead(BTN_BOOT); // Active low
  if (boot_current && !boot_button_pressed) {
    boot_button_pressed = true;
    button_press_time = millis();
  } else if (!boot_current && boot_button_pressed) {
    boot_button_pressed = false;
    unsigned long press_duration = millis() - button_press_time;
    
    if (press_duration < 300) {
      // Short press - back/menu
      if (system_state.current_screen == SCREEN_APP_GRID) {
        system_state.current_screen = SCREEN_WATCHFACE;
      } else if (system_state.current_screen != SCREEN_WATCHFACE) {
        exitCurrentApp();
      } else {
        system_state.current_screen = SCREEN_APP_GRID;
      }
    }
    
    // Reset sleep timer on button press
    system_state.sleep_timer = millis();
  }
}

// Snake game update function
void updateSnake() {
  static unsigned long last_snake_update = 0;
  unsigned long current_time = millis();
  
  if (current_game_session.state != GAME_PLAYING) return;
  if (current_time - last_snake_update < 200) return; // Snake speed
  
  last_snake_update = current_time;
  
  // Move snake head
  int new_head_x = current_game_session.snake_x[0];
  int new_head_y = current_game_session.snake_y[0];
  
  switch (current_game_session.direction) {
    case 0: new_head_y--; break; // Up
    case 1: new_head_x++; break; // Right
    case 2: new_head_y++; break; // Down
    case 3: new_head_x--; break; // Left
  }
  
  // Check boundaries
  if (new_head_x < 0 || new_head_x >= 15 || new_head_y < 0 || new_head_y >= 12) {
    current_game_session.state = GAME_OVER;
    return;
  }
  
  // Check self collision
  for (int i = 0; i < current_game_session.snake_length; i++) {
    if (current_game_session.snake_x[i] == new_head_x && 
        current_game_session.snake_y[i] == new_head_y) {
      current_game_session.state = GAME_OVER;
      return;
    }
  }
  
  // Move snake body
  for (int i = current_game_session.snake_length - 1; i > 0; i--) {
    current_game_session.snake_x[i] = current_game_session.snake_x[i - 1];
    current_game_session.snake_y[i] = current_game_session.snake_y[i - 1];
  }
  
  // Set new head position
  current_game_session.snake_x[0] = new_head_x;
  current_game_session.snake_y[0] = new_head_y;
  
  // Check food collision
  if (new_head_x == current_game_session.food_x && 
      new_head_y == current_game_session.food_y) {
    current_game_session.snake_length++;
    current_game_session.score += 10;
    generateFood();
  }
}