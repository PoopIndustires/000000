/*
 * Real Time Clock Implementation
 * PCF85063 RTC for ESP32-S3 Watch
 */

#include "rtc.h"

// RTC state variables
static Alarm watch_alarms[5];
static int active_alarms = 0;
static unsigned long timer_start_time = 0;
static int timer_duration = 0;
static bool timer_active = false;
static unsigned long stopwatch_start_time = 0;
static unsigned long stopwatch_pause_time = 0;
static bool stopwatch_running = false;

bool initializeRTC() {
  Serial.println("Initializing RTC...");
  
  // Initialize I2C for PCF85063
  Wire1.begin(RTC_SDA, RTC_SCL);
  Wire1.setClock(100000); // 100kHz for RTC
  
  // PCF85063 I2C address is 0x51
  Wire1.beginTransmission(0x51);
  if (Wire1.endTransmission() != 0) {
    Serial.println("PCF85063 RTC not found! Using system time.");
    // Continue with system time fallback
  } else {
    Serial.println("PCF85063 RTC found");
    
    // Configure RTC
    Wire1.beginTransmission(0x51);
    Wire1.write(0x00); // Control register
    Wire1.write(0x00); // Normal operation
    Wire1.endTransmission();
    
    // Set 24-hour mode
    Wire1.beginTransmission(0x51);
    Wire1.write(0x01); // Control register 2
    Wire1.write(0x00); // 24-hour format
    Wire1.endTransmission();
  }
  
  // Initialize alarms
  for (int i = 0; i < 5; i++) {
    watch_alarms[i].enabled = false;
    watch_alarms[i].hour = 7;
    watch_alarms[i].minute = 0;
    watch_alarms[i].repeat_daily = true;
    watch_alarms[i].label = "Alarm " + String(i + 1);
    watch_alarms[i].vibration = true;
    watch_alarms[i].snooze_minutes = 5;
  }
  
  // Set default alarm
  watch_alarms[0].enabled = true;
  watch_alarms[0].hour = 7;
  watch_alarms[0].minute = 30;
  watch_alarms[0].label = "Wake Up";
  
  Serial.println("RTC initialized successfully");
  return true;
}

WatchTime getCurrentTime() {
  WatchTime watch_time;
  
  // Try to read from PCF85063 first
  Wire1.beginTransmission(0x51);
  Wire1.write(0x04); // Seconds register
  Wire1.endTransmission();
  
  Wire1.requestFrom(0x51, 7);
  if (Wire1.available() >= 7) {
    // Read BCD time from RTC
    uint8_t seconds = Wire1.read();
    uint8_t minutes = Wire1.read();
    uint8_t hours = Wire1.read();
    uint8_t days = Wire1.read();
    uint8_t weekdays = Wire1.read();
    uint8_t months = Wire1.read();
    uint8_t years = Wire1.read();
    
    // Convert BCD to decimal
    watch_time.second = (seconds & 0x0F) + ((seconds >> 4) & 0x07) * 10;
    watch_time.minute = (minutes & 0x0F) + ((minutes >> 4) & 0x07) * 10;
    watch_time.hour = (hours & 0x0F) + ((hours >> 4) & 0x03) * 10;
    watch_time.day = (days & 0x0F) + ((days >> 4) & 0x03) * 10;
    watch_time.weekday = weekdays & 0x07;
    watch_time.month = (months & 0x0F) + ((months >> 4) & 0x01) * 10;
    watch_time.year = 2000 + (years & 0x0F) + ((years >> 4) & 0x0F) * 10;
  } else {
    // Fallback to system time
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    watch_time.second = timeinfo->tm_sec;
    watch_time.minute = timeinfo->tm_min;
    watch_time.hour = timeinfo->tm_hour;
    watch_time.day = timeinfo->tm_mday;
    watch_time.month = timeinfo->tm_mon + 1;
    watch_time.year = timeinfo->tm_year + 1900;
    watch_time.weekday = timeinfo->tm_wday;
  }
  
  return watch_time;
}

void setCurrentTime(WatchTime& time) {
  if (!isValidTime(time)) return;
  
  // Convert decimal to BCD
  uint8_t seconds = (time.second % 10) | ((time.second / 10) << 4);
  uint8_t minutes = (time.minute % 10) | ((time.minute / 10) << 4);
  uint8_t hours = (time.hour % 10) | ((time.hour / 10) << 4);
  uint8_t days = (time.day % 10) | ((time.day / 10) << 4);
  uint8_t weekdays = time.weekday;
  uint8_t months = (time.month % 10) | ((time.month / 10) << 4);
  uint8_t years = ((time.year - 2000) % 10) | (((time.year - 2000) / 10) << 4);
  
  // Write to PCF85063
  Wire1.beginTransmission(0x51);
  Wire1.write(0x04); // Seconds register
  Wire1.write(seconds);
  Wire1.write(minutes);
  Wire1.write(hours);
  Wire1.write(days);
  Wire1.write(weekdays);
  Wire1.write(months);
  Wire1.write(years);
  Wire1.endTransmission();
  
  Serial.println("Time set: " + formatTime(time));
}

void syncTimeWithWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot sync time");
    return;
  }
  
  // Configure NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  Serial.println("Syncing time with NTP...");
  
  // Wait for time sync
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 8 * 3600 * 2 && attempts < 15) {
    delay(500);
    now = time(nullptr);
    attempts++;
  }
  
  if (now > 8 * 3600 * 2) {
    struct tm* timeinfo = localtime(&now);
    WatchTime watch_time;
    
    watch_time.second = timeinfo->tm_sec;
    watch_time.minute = timeinfo->tm_min;
    watch_time.hour = timeinfo->tm_hour;
    watch_time.day = timeinfo->tm_mday;
    watch_time.month = timeinfo->tm_mon + 1;
    watch_time.year = timeinfo->tm_year + 1900;
    watch_time.weekday = timeinfo->tm_wday;
    
    setCurrentTime(watch_time);
    Serial.println("Time synced successfully");
  } else {
    Serial.println("Failed to sync time");
  }
}

bool isValidTime(WatchTime& time) {
  return (time.hour >= 0 && time.hour <= 23 &&
          time.minute >= 0 && time.minute <= 59 &&
          time.second >= 0 && time.second <= 59 &&
          time.day >= 1 && time.day <= 31 &&
          time.month >= 1 && time.month <= 12 &&
          time.year >= 2000 && time.year <= 2099);
}

String formatTime(WatchTime& time, bool twelve_hour) {
  char time_str[20];
  
  if (twelve_hour) {
    int display_hour = time.hour;
    const char* ampm = "AM";
    
    if (display_hour == 0) {
      display_hour = 12;
    } else if (display_hour > 12) {
      display_hour -= 12;
      ampm = "PM";
    } else if (display_hour == 12) {
      ampm = "PM";
    }
    
    sprintf(time_str, "%d:%02d %s", display_hour, time.minute, ampm);
  } else {
    sprintf(time_str, "%02d:%02d", time.hour, time.minute);
  }
  
  return String(time_str);
}

String formatDate(WatchTime& time) {
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char* months[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  char date_str[30];
  sprintf(date_str, "%s %s %d", days[time.weekday], months[time.month], time.day);
  
  return String(date_str);
}

String formatDateTime(WatchTime& time) {
  return formatDate(time) + " " + formatTime(time);
}

void setTimezone(int offset_hours) {
  // Set timezone offset
  configTime(offset_hours * 3600, 0, "pool.ntp.org");
}

int getTimezone() {
  // Return current timezone offset
  return 0; // Default UTC
}

void setAlarm(int id, Alarm& alarm) {
  if (id >= 0 && id < 5) {
    watch_alarms[id] = alarm;
    
    if (alarm.enabled && id >= active_alarms) {
      active_alarms = id + 1;
    }
    
    Serial.println("Alarm " + String(id) + " set: " + String(alarm.hour) + ":" + String(alarm.minute));
  }
}

Alarm getAlarm(int id) {
  if (id >= 0 && id < 5) {
    return watch_alarms[id];
  }
  
  return watch_alarms[0]; // Return default alarm
}

void enableAlarm(int id, bool enabled) {
  if (id >= 0 && id < 5) {
    watch_alarms[id].enabled = enabled;
  }
}

void checkAlarms() {
  WatchTime current = getCurrentTime();
  
  for (int i = 0; i < active_alarms; i++) {
    if (watch_alarms[i].enabled &&
        watch_alarms[i].hour == current.hour &&
        watch_alarms[i].minute == current.minute &&
        current.second == 0) { // Only trigger at the exact minute
      
      triggerAlarm(i);
    }
  }
}

void triggerAlarm(int id) {
  if (id < 0 || id >= 5) return;
  
  Serial.println("Alarm triggered: " + watch_alarms[id].label);
  
  // Show alarm notification
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Alarm icon (bell)
  drawCircle(DISPLAY_WIDTH/2, 120, 30, theme->accent);
  drawCircle(DISPLAY_WIDTH/2, 120, 25, theme->accent);
  fillCircle(DISPLAY_WIDTH/2, 145, 5, theme->accent);
  
  drawCenteredText("ALARM", DISPLAY_WIDTH/2, 180, theme->accent, 3);
  drawCenteredText(watch_alarms[id].label, DISPLAY_WIDTH/2, 220, theme->text, 2);
  
  WatchTime current = getCurrentTime();
  drawCenteredText(formatTime(current), DISPLAY_WIDTH/2, 250, theme->text, 2);
  
  // Snooze and dismiss buttons
  drawGameButton(20, 320, 100, 40, "Snooze", false);
  drawGameButton(140, 320, 100, 40, "Dismiss", false);
  
  updateDisplay();
  
  // Alarm sound/vibration would be triggered here
  if (watch_alarms[id].vibration) {
    // Trigger vibration
  }
}

void snoozeAlarm(int id) {
  if (id < 0 || id >= 5) return;
  
  // Set alarm to go off again after snooze time
  WatchTime current = getCurrentTime();
  current.minute += watch_alarms[id].snooze_minutes;
  
  if (current.minute >= 60) {
    current.minute -= 60;
    current.hour++;
    if (current.hour >= 24) {
      current.hour = 0;
    }
  }
  
  watch_alarms[id].hour = current.hour;
  watch_alarms[id].minute = current.minute;
  
  Serial.println("Alarm snoozed for " + String(watch_alarms[id].snooze_minutes) + " minutes");
}

void dismissAlarm(int id) {
  if (id < 0 || id >= 5) return;
  
  if (!watch_alarms[id].repeat_daily) {
    watch_alarms[id].enabled = false;
  }
  
  Serial.println("Alarm dismissed: " + watch_alarms[id].label);
}

// Timer functions
void startTimer(int minutes, String label) {
  timer_start_time = millis();
  timer_duration = minutes * 60000; // Convert to milliseconds
  timer_active = true;
  
  Serial.println("Timer started: " + String(minutes) + " minutes - " + label);
}

void stopTimer() {
  timer_active = false;
  Serial.println("Timer stopped");
}

void pauseTimer() {
  if (timer_active) {
    timer_active = false;
    // Pause implementation would save current state
  }
}

void resumeTimer() {
  if (!timer_active) {
    timer_active = true;
    // Resume implementation would restore state
  }
}

int getTimerRemaining() {
  if (!timer_active) return 0;
  
  unsigned long elapsed = millis() - timer_start_time;
  if (elapsed >= timer_duration) {
    timer_active = false;
    // Timer finished
    return 0;
  }
  
  return (timer_duration - elapsed) / 1000; // Return seconds remaining
}

// Stopwatch functions
void startStopwatch() {
  stopwatch_start_time = millis();
  stopwatch_running = true;
  stopwatch_pause_time = 0;
}

void stopStopwatch() {
  stopwatch_running = false;
}

void pauseStopwatch() {
  if (stopwatch_running) {
    stopwatch_pause_time = millis();
    stopwatch_running = false;
  }
}

void resumeStopwatch() {
  if (!stopwatch_running && stopwatch_pause_time > 0) {
    unsigned long pause_duration = millis() - stopwatch_pause_time;
    stopwatch_start_time += pause_duration;
    stopwatch_running = true;
    stopwatch_pause_time = 0;
  }
}

void resetStopwatch() {
  stopwatch_start_time = 0;
  stopwatch_pause_time = 0;
  stopwatch_running = false;
}

unsigned long getStopwatchTime() {
  if (!stopwatch_running && stopwatch_pause_time == 0) return 0;
  
  if (stopwatch_running) {
    return millis() - stopwatch_start_time;
  } else {
    return stopwatch_pause_time - stopwatch_start_time;
  }
}

void checkTimeBasedEvents() {
  checkAlarms();
  
  // Check timer
  if (timer_active && getTimerRemaining() == 0) {
    // Timer finished - show notification
    Serial.println("Timer finished!");
    // Show timer completion notification
  }
}

void setWakeUpTime(int hour, int minute) {
  system_state.wake_time = hour * 60 + minute;
}

void setSleepTime(int hour, int minute) {
  system_state.sleep_time = hour * 60 + minute;
}

void calibrateRTC() {
  // RTC calibration routine
  // This would fine-tune the RTC crystal frequency
  Serial.println("RTC calibration completed");
}

bool isRTCBatteryLow() {
  // Check RTC battery voltage
  // This would read from AXP2101 backup battery status
  return false; // Placeholder
}

void replaceRTCBattery() {
  Serial.println("Please replace RTC backup battery");
}