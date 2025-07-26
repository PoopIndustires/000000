/*
 * User Interface Framework for ESP32-S3 Watch
 * Apple Watch-inspired UI components
 */

#ifndef UI_H
#define UI_H

#include "config.h"
#include "touch.h"

// UI Component types
enum UIComponentType {
  UI_BUTTON,
  UI_LABEL,
  UI_SLIDER,
  UI_TOGGLE,
  UI_LIST,
  UI_PROGRESS_BAR,
  UI_ACTIVITY_RING
};

// UI Component structure
struct UIComponent {
  UIComponentType type;
  int x, y, width, height;
  String text;
  uint16_t color;
  bool visible;
  bool enabled;
  bool pressed;
  void (*callback)(void);
};

// Screen layout structure
struct ScreenLayout {
  UIComponent* components;
  int component_count;
  void (*draw_function)(void);
  void (*touch_handler)(TouchGesture&);
};

// Initialize UI system
void initializeUI();

// Screen management
void setCurrentScreen(ScreenType screen);
ScreenType getCurrentScreen();
void drawCurrentScreen();

// Component drawing
void drawButton(UIComponent& button);
void drawLabel(UIComponent& label);
void drawSlider(UIComponent& slider);
void drawToggle(UIComponent& toggle);
void drawProgressBar(UIComponent& progress);

// Touch handling
void handleUITouch(TouchGesture& gesture);
bool isTouchInComponent(TouchGesture& gesture, UIComponent& component);
void handleButtonPress(UIComponent& button);

// Animation system
void animateScreenTransition(ScreenType from, ScreenType to);
void animateButtonPress(UIComponent& button);
void animateSlideIn(int direction);

// Apple Watch-style UI elements
void drawDigitalCrown(int x, int y, int value);
void drawNavigationBar(const char* title, bool back_button = true);
void drawStatusBar();
void drawNotificationDot(int x, int y, uint16_t color);

// List view components
void drawListItem(int x, int y, int width, const char* title, const char* subtitle, uint16_t color);
void handleListScroll(TouchGesture& gesture, int& scroll_offset);

// Modal dialogs
void showAlert(const char* title, const char* message);
void showConfirm(const char* title, const char* message, void (*yes_callback)(void));
void showInput(const char* title, String& input_text);

// Loading indicators
void showLoadingSpinner(const char* message);
void hideLoadingSpinner();
void updateLoadingProgress(int percentage);

// Watch face complications
void drawComplicationSlot(int x, int y, int w, int h, const char* data, uint16_t color);
void updateComplications();

#endif // UI_H