/*
 * Configuration file for ESP32-S3 Watch
 * Pin definitions, constants, and system settings
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== DISPLAY CONFIGURATION ====================
#define DISPLAY_WIDTH 368
#define DISPLAY_HEIGHT 448
#define DISPLAY_ROTATION 0

// SH8601 AMOLED Display Pins (QSPI)
#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_CS 37
#define TFT_DC 38
#define TFT_RST 39
#define TFT_BL 40

// ==================== TOUCH CONFIGURATION ====================
// FT3168 Capacitive Touch (I2C)
#define TOUCH_SDA 8
#define TOUCH_SCL 9
#define TOUCH_INT 10
#define TOUCH_RST 11

// ==================== SENSOR CONFIGURATION ====================
// QMI8658 6-Axis IMU (I2C)
#define IMU_SDA 41
#define IMU_SCL 42
#define IMU_INT 43

// ==================== RTC CONFIGURATION ====================
// PCF85063 RTC (I2C)
#define RTC_SDA 41  // Shared with IMU
#define RTC_SCL 42  // Shared with IMU
#define RTC_INT 44

// ==================== POWER CONFIGURATION ====================
// AXP2101 Power Management (I2C)
#define PWR_SDA 41  // Shared I2C bus
#define PWR_SCL 42  // Shared I2C bus
#define PWR_INT 45

// ==================== BUTTON CONFIGURATION ====================
#define BTN_PWR 0   // Power/Wake button
#define BTN_BOOT 46 // Boot/Menu button

// ==================== AUDIO CONFIGURATION ====================
#define AUDIO_SDA 47
#define AUDIO_SCL 48
#define MIC_DATA 4
#define MIC_CLK 5
#define SPEAKER_PIN 6

// ==================== SD CARD CONFIGURATION ====================
#define SD_MISO 13
#define SD_MOSI 14
#define SD_SCLK 15
#define SD_CS 16

// ==================== SYSTEM CONSTANTS ====================
#define SLEEP_TIMEOUT 30000    // 30 seconds
#define DEEP_SLEEP_TIMEOUT 300000  // 5 minutes
#define SENSOR_UPDATE_INTERVAL 100  // 100ms
#define UI_UPDATE_INTERVAL 16       // ~60 FPS

// Battery levels
#define BATTERY_LOW_THRESHOLD 15
#define BATTERY_CRITICAL_THRESHOLD 5

// Step counter
#define MIN_STEP_THRESHOLD 1.2     // G-force threshold
#define STEP_TIME_WINDOW 500       // 500ms between steps

// ==================== SCREEN DEFINITIONS ====================
enum ScreenType {
  SCREEN_SPLASH,
  SCREEN_WATCHFACE,
  SCREEN_APP_GRID,
  SCREEN_MUSIC,
  SCREEN_NOTES,
  SCREEN_QUESTS,
  SCREEN_SETTINGS,
  SCREEN_PDF_READER,
  SCREEN_FILE_BROWSER,
  SCREEN_SLEEP,
  SCREEN_CHARGING
};

// ==================== THEME DEFINITIONS ====================
enum ThemeType {
  THEME_LUFFY_GEAR5,
  THEME_SUNG_JINWOO,
  THEME_YUGO_WAKFU
};

// ==================== APP DEFINITIONS ====================
enum AppType {
  APP_WATCHFACE,
  APP_QUESTS,
  APP_MUSIC,
  APP_NOTES,
  APP_FILES,
  APP_SETTINGS,
  APP_PDF_READER,
  APP_WEATHER,
  APP_GAMES
};

// ==================== COLOR DEFINITIONS ====================
// 16-bit RGB565 color format
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_ORANGE    0xFD20
#define COLOR_PURPLE    0x8010
#define COLOR_CYAN      0x07FF
#define COLOR_PINK      0xF81F

// Luffy Gear 5 Colors
#define LUFFY_WHITE     0xFFFF
#define LUFFY_GOLD      0xFFE0
#define LUFFY_CREAM     0xF7DE
#define LUFFY_SHADOW    0x2104

// Sung Jin Woo Colors  
#define JINWOO_PURPLE   0x8010
#define JINWOO_DARK     0x2104
#define JINWOO_SILVER   0xC618
#define JINWOO_VIOLET   0xA015

// Yugo Wakfu Colors
#define YUGO_TEAL       0x0679
#define YUGO_BLUE       0x001F
#define YUGO_LIGHT_BLUE 0x867F
#define YUGO_ENERGY     0x07FF

// ==================== SYSTEM STATE STRUCTURE ====================
struct SystemState {
  ScreenType current_screen;
  ThemeType current_theme;
  AppType current_app;
  
  // Power management
  int battery_percentage;
  bool is_charging;
  bool low_battery_warning;
  bool low_power_mode;
  int brightness;
  
  // Time and sleep
  unsigned long sleep_timer;
  int wake_time;    // Minutes from midnight
  int sleep_time;   // Minutes from midnight
  
  // Sensors
  int steps_today;
  int step_goal;
  float heart_rate;
  
  // Touch and input
  bool touch_active;
  int touch_x, touch_y;
  bool button_pressed;
  
  // Apps
  int current_quest;
  bool music_playing;
  int current_song;
  String current_note;
  String current_pdf;
  
  // File system
  int total_mp3_files;
  int total_pdf_files;
};

// ==================== THEME COLOR STRUCTURE ====================
struct ThemeColors {
  uint16_t primary;
  uint16_t secondary;
  uint16_t accent;
  uint16_t background;
  uint16_t text;
  uint16_t shadow;
};

// ==================== QUEST STRUCTURE ====================
struct Quest {
  String title;
  String description;
  int target_steps;
  int reward_points;
  bool completed;
  int progress;
};

// ==================== MUSIC TRACK STRUCTURE ====================
struct MusicTrack {
  String filename;
  String title;
  String artist;
  int duration;
  bool has_artwork;
};

// ==================== GLOBAL VARIABLES ====================
extern SystemState system_state;

// ==================== UTILITY MACROS ====================
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define RGB565(r,g,b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#endif // CONFIG_H