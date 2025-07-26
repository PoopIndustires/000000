/*
 * User Interface Implementation
 * Apple Watch-inspired UI components for ESP32-S3 Watch
 */

#include "ui.h"
#include "display.h"
#include "themes.h"
#include "apps.h"

// UI state variables
static ScreenType current_ui_screen = SCREEN_WATCHFACE;
static bool loading_spinner_active = false;
static String loading_message = "";

void initializeUI() {
  Serial.println("Initializing UI system...");
  current_ui_screen = SCREEN_WATCHFACE;
  loading_spinner_active = false;
  Serial.println("UI system initialized");
}

void setCurrentScreen(ScreenType screen) {
  current_ui_screen = screen;
  system_state.current_screen = screen;
}

ScreenType getCurrentScreen() {
  return current_ui_screen;
}

void drawCurrentScreen() {
  switch (current_ui_screen) {
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
    case SCREEN_PDF_READER:
      drawPDFReaderApp();
      break;
    case SCREEN_FILE_BROWSER:
      drawFileBrowserApp();
      break;
    case SCREEN_SLEEP:
      drawSleepWatchFace();
      break;
    case SCREEN_CHARGING:
      showChargingAnimation();
      break;
    default:
      drawWatchFace();
      break;
  }
}

void drawButton(UIComponent& button) {
  ThemeColors* theme = getCurrentTheme();
  
  uint16_t bg_color = button.pressed ? theme->accent : theme->shadow;
  uint16_t border_color = button.enabled ? theme->primary : theme->secondary;
  uint16_t text_color = button.pressed ? theme->background : theme->text;
  
  if (!button.visible) return;
  
  fillRoundRect(button.x, button.y, button.width, button.height, 6, bg_color);
  drawRoundRect(button.x, button.y, button.width, button.height, 6, border_color);
  
  // Center text
  int text_x = button.x + (button.width - getTextWidth(button.text.c_str(), 1)) / 2;
  int text_y = button.y + (button.height - 8) / 2;
  drawText(button.text.c_str(), text_x, text_y, text_color, 1);
}

void drawLabel(UIComponent& label) {
  if (!label.visible) return;
  
  drawText(label.text.c_str(), label.x, label.y, label.color, 1);
}

void drawProgressBar(UIComponent& progress) {
  ThemeColors* theme = getCurrentTheme();
  
  if (!progress.visible) return;
  
  // Background
  fillRoundRect(progress.x, progress.y, progress.width, progress.height, 3, theme->shadow);
  drawRoundRect(progress.x, progress.y, progress.width, progress.height, 3, theme->secondary);
  
  // Progress fill (assuming progress value is stored in some way)
  // This is a simplified implementation
  int fill_width = progress.width * 0.5; // 50% for example
  fillRoundRect(progress.x + 1, progress.y + 1, fill_width - 2, progress.height - 2, 2, progress.color);
}

void drawSlider(UIComponent& slider) {
  ThemeColors* theme = getCurrentTheme();
  
  if (!slider.visible) return;
  
  // Slider track
  int track_y = slider.y + slider.height / 2 - 2;
  fillRoundRect(slider.x, track_y, slider.width, 4, 2, theme->shadow);
  
  // Slider handle (simplified - at center for now)
  int handle_x = slider.x + slider.width / 2;
  fillCircle(handle_x, slider.y + slider.height / 2, 8, slider.color);
  drawCircle(handle_x, slider.y + slider.height / 2, 8, theme->primary);
}

void drawToggle(UIComponent& toggle) {
  ThemeColors* theme = getCurrentTheme();
  
  if (!toggle.visible) return;
  
  uint16_t bg_color = toggle.pressed ? theme->accent : theme->shadow;
  uint16_t handle_color = toggle.pressed ? theme->background : theme->secondary;
  
  // Toggle background
  fillRoundRect(toggle.x, toggle.y, toggle.width, toggle.height, toggle.height / 2, bg_color);
  
  // Toggle handle
  int handle_x = toggle.pressed ? toggle.x + toggle.width - toggle.height / 2 - 4 : toggle.x + toggle.height / 2 + 4;
  fillCircle(handle_x, toggle.y + toggle.height / 2, toggle.height / 2 - 4, handle_color);
}

void handleUITouch(TouchGesture& gesture) {
  // Delegate touch handling based on current screen
  switch (current_ui_screen) {
    case SCREEN_APP_GRID:
      handleAppGridTouch(gesture);
      break;
    case SCREEN_MUSIC:
      handleMusicTouch(gesture);
      break;
    case SCREEN_QUESTS:
      // Quest screen touch handling would go here
      break;
    case SCREEN_SETTINGS:
      handleSettingsTouch(gesture);
      break;
    default:
      break;
  }
}

bool isTouchInComponent(TouchGesture& gesture, UIComponent& component) {
  return (gesture.x >= component.x && 
          gesture.x <= component.x + component.width &&
          gesture.y >= component.y && 
          gesture.y <= component.y + component.height);
}

void handleButtonPress(UIComponent& button) {
  if (button.callback != nullptr) {
    button.callback();
  }
}

void animateScreenTransition(ScreenType from, ScreenType to) {
  // Simple slide transition
  slideTransition(1, 300); // Slide left
  setCurrentScreen(to);
}

void animateButtonPress(UIComponent& button) {
  // Simple button press animation
  button.pressed = true;
  drawButton(button);
  updateDisplay();
  delay(100);
  button.pressed = false;
  drawButton(button);
  updateDisplay();
}

void drawNavigationBar(const char* title, bool back_button) {
  ThemeColors* theme = getCurrentTheme();
  
  // Navigation bar background
  fillRect(0, 0, DISPLAY_WIDTH, 40, theme->primary);
  
  // Back button
  if (back_button) {
    drawText("<", 10, 15, theme->background, 2);
  }
  
  // Title
  drawCenteredText(title, DISPLAY_WIDTH/2, 15, theme->background, 1);
  
  // Separator line
  drawLine(0, 40, DISPLAY_WIDTH, 40, theme->secondary);
}

void drawStatusBar() {
  ThemeColors* theme = getCurrentTheme();
  
  // Time (top left)
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char time_str[6];
  sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  drawText(time_str, 10, 5, theme->text, 1);
  
  // Battery (top right)
  char battery_str[10];
  sprintf(battery_str, "%d%%", system_state.battery_percentage);
  int battery_x = DISPLAY_WIDTH - getTextWidth(battery_str, 1) - 10;
  drawText(battery_str, battery_x, 5, theme->text, 1);
  
  // Battery icon
  int icon_x = battery_x - 25;
  drawRect(icon_x, 7, 20, 10, theme->text);
  drawRect(icon_x + 20, 9, 2, 6, theme->text);
  
  // Fill based on battery level
  int fill_width = 18 * system_state.battery_percentage / 100;
  fillRect(icon_x + 1, 8, fill_width, 8, theme->accent);
}

void drawNotificationDot(int x, int y, uint16_t color) {
  fillCircle(x, y, 3, color);
}

void drawListItem(int x, int y, int width, const char* title, const char* subtitle, uint16_t color) {
  ThemeColors* theme = getCurrentTheme();
  
  // Item background
  fillRoundRect(x, y, width, 50, 8, theme->shadow);
  drawRoundRect(x, y, width, 50, 8, color);
  
  // Title
  drawText(title, x + 10, y + 10, theme->text, 1);
  
  // Subtitle
  if (subtitle != nullptr) {
    drawText(subtitle, x + 10, y + 30, theme->secondary, 1);
  }
  
  // Arrow indicator
  drawText(">", x + width - 20, y + 20, color, 1);
}

void showAlert(const char* title, const char* message) {
  ThemeColors* theme = getCurrentTheme();
  
  // Semi-transparent overlay
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->shadow);
  
  // Alert box
  int alert_w = DISPLAY_WIDTH - 40;
  int alert_h = 120;
  int alert_x = 20;
  int alert_y = (DISPLAY_HEIGHT - alert_h) / 2;
  
  fillRoundRect(alert_x, alert_y, alert_w, alert_h, 12, theme->background);
  drawRoundRect(alert_x, alert_y, alert_w, alert_h, 12, theme->accent);
  
  // Title
  drawCenteredText(title, DISPLAY_WIDTH/2, alert_y + 30, theme->text, 2);
  
  // Message
  drawCenteredText(message, DISPLAY_WIDTH/2, alert_y + 60, theme->secondary, 1);
  
  // OK button
  drawGameButton(alert_x + 20, alert_y + alert_h - 40, alert_w - 40, 30, "OK", false);
  
  updateDisplay();
}

void showLoadingSpinner(const char* message) {
  loading_spinner_active = true;
  loading_message = message;
  
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Loading message
  drawCenteredText(message, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 40, theme->text, 1);
  
  // Simple loading animation
  for (int i = 0; i < 8; i++) {
    float angle = (2 * PI * i) / 8;
    int x = DISPLAY_WIDTH/2 + 20 * cos(angle);
    int y = DISPLAY_HEIGHT/2 + 20 * sin(angle);
    
    uint16_t dot_color = (i < 3) ? theme->accent : theme->secondary;
    fillCircle(x, y, 3, dot_color);
  }
  
  updateDisplay();
}

void hideLoadingSpinner() {
  loading_spinner_active = false;
  loading_message = "";
}

void drawDigitalCrown(int x, int y, int value) {
  ThemeColors* theme = getCurrentTheme();
  
  // Crown circle
  drawCircle(x, y, 15, theme->secondary);
  fillCircle(x, y, 12, theme->shadow);
  
  // Crown ridges
  for (int i = 0; i < 8; i++) {
    float angle = (2 * PI * i) / 8;
    int x1 = x + 10 * cos(angle);
    int y1 = y + 10 * sin(angle);
    int x2 = x + 14 * cos(angle);
    int y2 = y + 14 * sin(angle);
    drawLine(x1, y1, x2, y2, theme->secondary);
  }
  
  // Value indicator
  float value_angle = (2 * PI * value) / 100 - PI/2;
  int indicator_x = x + 8 * cos(value_angle);
  int indicator_y = y + 8 * sin(value_angle);
  fillCircle(indicator_x, indicator_y, 2, theme->accent);
}

void drawComplicationSlot(int x, int y, int w, int h, const char* data, uint16_t color) {
  ThemeColors* theme = getCurrentTheme();
  
  // Complication background
  fillRoundRect(x, y, w, h, 6, theme->shadow);
  drawRoundRect(x, y, w, h, 6, color);
  
  // Data text
  drawCenteredText(data, x + w/2, y + h/2, theme->text, 1);
}

void updateComplications() {
  // Update watch face complications with current data
  // This would be called periodically to refresh displayed information
}