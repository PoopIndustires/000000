/*
 * Touch Input Management for ESP32-S3 Watch
 * FT3168 Capacitive Touch Controller Interface
 */

#ifndef TOUCH_H
#define TOUCH_H

#include "config.h"
#include <Wire.h>

// Touch event types
enum TouchEvent {
  TOUCH_NONE,
  TOUCH_PRESS,
  TOUCH_RELEASE,
  TOUCH_MOVE,
  TOUCH_SWIPE_UP,
  TOUCH_SWIPE_DOWN,
  TOUCH_SWIPE_LEFT,
  TOUCH_SWIPE_RIGHT,
  TOUCH_TAP,
  TOUCH_DOUBLE_TAP,
  TOUCH_LONG_PRESS
};

// Touch gesture structure
struct TouchGesture {
  TouchEvent event;
  int x, y;
  int start_x, start_y;
  int end_x, end_y;
  unsigned long timestamp;
  unsigned long duration;
  bool is_valid;
};

// Initialize touch system
bool initializeTouch();

// Touch input handling
TouchGesture handleTouchInput();
bool isTouchPressed();
void getTouchPosition(int& x, int& y);

// Gesture recognition
TouchEvent recognizeGesture(int start_x, int start_y, int end_x, int end_y, unsigned long duration);
bool isSwipeGesture(int start_x, int start_y, int end_x, int end_y);
bool isTapGesture(unsigned long duration, int movement);

// Touch calibration
void calibrateTouch();
void saveTouchCalibration();
void loadTouchCalibration();

// Digital crown simulation
int getDigitalCrownValue();
void resetDigitalCrown();

#endif // TOUCH_H