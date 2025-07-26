/*
 * Touch Input Implementation
 * FT3168 Capacitive Touch Controller
 */

#include "touch.h"
#include "config.h"

// Touch calibration data
struct TouchCalibration {
  int min_x, max_x;
  int min_y, max_y;
  bool calibrated;
} touch_cal;

// Touch tracking variables
static int last_touch_x = -1;
static int last_touch_y = -1;
static unsigned long touch_start_time = 0;
static bool touch_pressed = false;
static int digital_crown_value = 0;

bool initializeTouch() {
  Serial.println("Initializing touch controller...");
  
  // Initialize I2C for touch controller
  Wire1.begin(TOUCH_SDA, TOUCH_SCL);
  Wire1.setClock(400000); // 400kHz
  
  // Reset touch controller
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);
  
  // Configure interrupt pin
  pinMode(TOUCH_INT, INPUT_PULLUP);
  
  // Load touch calibration
  loadTouchCalibration();
  
  // Test touch controller communication
  Wire1.beginTransmission(0x38); // FT3168 I2C address
  if (Wire1.endTransmission() != 0) {
    Serial.println("Touch controller not found!");
    return false;
  }
  
  Serial.println("Touch controller initialized successfully");
  return true;
}

TouchGesture handleTouchInput() {
  TouchGesture gesture = {TOUCH_NONE, 0, 0, 0, 0, 0, 0, millis(), 0, false};
  
  // Check if touch interrupt is active
  if (digitalRead(TOUCH_INT) == HIGH) {
    return gesture; // No touch detected
  }
  
  // Read touch data from FT3168
  Wire1.beginTransmission(0x38);
  Wire1.write(0x00); // Status register
  Wire1.endTransmission();
  
  Wire1.requestFrom(0x38, 6);
  if (Wire1.available() < 6) {
    return gesture;
  }
  
  uint8_t status = Wire1.read();
  uint8_t touch_count = status & 0x0F;
  
  if (touch_count == 0) {
    // Touch released
    if (touch_pressed) {
      touch_pressed = false;
      unsigned long touch_duration = millis() - touch_start_time;
      
      gesture.event = TOUCH_RELEASE;
      gesture.x = last_touch_x;
      gesture.y = last_touch_y;
      gesture.start_x = last_touch_x;
      gesture.start_y = last_touch_y;
      gesture.end_x = last_touch_x;
      gesture.end_y = last_touch_y;
      gesture.duration = touch_duration;
      gesture.is_valid = true;
      
      // Determine gesture type based on duration
      if (touch_duration > 800) {
        gesture.event = TOUCH_LONG_PRESS;
      } else if (touch_duration < 200) {
        gesture.event = TOUCH_TAP;
      }
    }
    return gesture;
  }
  
  // Read touch coordinates
  uint8_t x_high = Wire1.read();
  uint8_t x_low = Wire1.read();
  uint8_t y_high = Wire1.read();
  uint8_t y_low = Wire1.read();
  uint8_t pressure = Wire1.read();
  
  int raw_x = ((x_high & 0x0F) << 8) | x_low;
  int raw_y = ((y_high & 0x0F) << 8) | y_low;
  
  // Apply calibration
  int touch_x = map(raw_x, touch_cal.min_x, touch_cal.max_x, 0, DISPLAY_WIDTH);
  int touch_y = map(raw_y, touch_cal.min_y, touch_cal.max_y, 0, DISPLAY_HEIGHT);
  
  // Constrain to display bounds
  touch_x = constrain(touch_x, 0, DISPLAY_WIDTH - 1);
  touch_y = constrain(touch_y, 0, DISPLAY_HEIGHT - 1);
  
  if (!touch_pressed) {
    // New touch started
    touch_pressed = true;
    touch_start_time = millis();
    last_touch_x = touch_x;
    last_touch_y = touch_y;
    
    gesture.event = TOUCH_PRESS;
    gesture.x = touch_x;
    gesture.y = touch_y;
    gesture.start_x = touch_x;
    gesture.start_y = touch_y;
    gesture.is_valid = true;
  } else {
    // Touch moved
    int dx = touch_x - last_touch_x;
    int dy = touch_y - last_touch_y;
    
    if (abs(dx) > 5 || abs(dy) > 5) {
      gesture.event = TOUCH_MOVE;
      gesture.x = touch_x;
      gesture.y = touch_y;
      gesture.start_x = last_touch_x;
      gesture.start_y = last_touch_y;
      gesture.end_x = touch_x;
      gesture.end_y = touch_y;
      gesture.is_valid = true;
      
      // Update digital crown simulation for vertical scrolling
      digital_crown_value += dy;
      
      // Check for swipe gestures
      unsigned long touch_duration = millis() - touch_start_time;
      if (touch_duration > 100 && touch_duration < 500) {
        if (abs(dx) > abs(dy) && abs(dx) > 50) {
          gesture.event = (dx > 0) ? TOUCH_SWIPE_RIGHT : TOUCH_SWIPE_LEFT;
        } else if (abs(dy) > abs(dx) && abs(dy) > 50) {
          gesture.event = (dy > 0) ? TOUCH_SWIPE_DOWN : TOUCH_SWIPE_UP;
        }
      }
      
      last_touch_x = touch_x;
      last_touch_y = touch_y;
    }
  }
  
  return gesture;
}

bool isTouchPressed() {
  return touch_pressed;
}

void getTouchPosition(int& x, int& y) {
  x = last_touch_x;
  y = last_touch_y;
}

TouchEvent recognizeGesture(int start_x, int start_y, int end_x, int end_y, unsigned long duration) {
  int dx = end_x - start_x;
  int dy = end_y - start_y;
  int distance = sqrt(dx*dx + dy*dy);
  
  // Long press
  if (duration > 800 && distance < 20) {
    return TOUCH_LONG_PRESS;
  }
  
  // Tap
  if (duration < 300 && distance < 20) {
    return TOUCH_TAP;
  }
  
  // Swipe gestures
  if (distance > 50 && duration < 500) {
    if (abs(dx) > abs(dy)) {
      return (dx > 0) ? TOUCH_SWIPE_RIGHT : TOUCH_SWIPE_LEFT;
    } else {
      return (dy > 0) ? TOUCH_SWIPE_DOWN : TOUCH_SWIPE_UP;
    }
  }
  
  return TOUCH_NONE;
}

bool isSwipeGesture(int start_x, int start_y, int end_x, int end_y) {
  int dx = abs(end_x - start_x);
  int dy = abs(end_y - start_y);
  return (dx > 50 || dy > 50) && (dx > 20 || dy > 20);
}

bool isTapGesture(unsigned long duration, int movement) {
  return duration < 300 && movement < 20;
}

void calibrateTouch() {
  // Touch calibration routine
  // This would involve touching corners and center points
  touch_cal.min_x = 100;
  touch_cal.max_x = 3900;
  touch_cal.min_y = 100;
  touch_cal.max_y = 3900;
  touch_cal.calibrated = true;
  saveTouchCalibration();
}

void saveTouchCalibration() {
  // Save calibration to EEPROM or file
  // Implementation would store calibration data persistently
}

void loadTouchCalibration() {
  // Load calibration from EEPROM or file
  // Default calibration values
  touch_cal.min_x = 100;
  touch_cal.max_x = 3900;
  touch_cal.min_y = 100;
  touch_cal.max_y = 3900;
  touch_cal.calibrated = true;
}

int getDigitalCrownValue() {
  return digital_crown_value;
}

void resetDigitalCrown() {
  digital_crown_value = 0;
}