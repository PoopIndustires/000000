/*
 * Stopwatch & Timer Application
 * Complete stopwatch, timer, and alarm management
 */

#include "apps.h"
#include "display.h"
#include "themes.h"
#include "ui.h"
#include "rtc.h"

// Stopwatch app state
enum StopwatchMode {
  MODE_STOPWATCH,
  MODE_TIMER,
  MODE_ALARMS
};

struct StopwatchAppState {
  StopwatchMode current_mode;
  int timer_minutes;
  int timer_seconds;
  bool timer_running;
  unsigned long timer_start_time;
  int timer_total_seconds;
  int selected_alarm;
} stopwatch_state;

void initStopwatchTimerApp() {
  stopwatch_state.current_mode = MODE_STOPWATCH;
  stopwatch_state.timer_minutes = 5;
  stopwatch_state.timer_seconds = 0;
  stopwatch_state.timer_running = false;
  stopwatch_state.timer_total_seconds = 300; // 5 minutes default
  stopwatch_state.selected_alarm = 0;
  
  Serial.println("Stopwatch & Timer app initialized");
}

void drawStopwatchTimerApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Mode tabs
  drawGameButton(10, 50, 110, 30, "Stopwatch", stopwatch_state.current_mode == MODE_STOPWATCH);
  drawGameButton(130, 50, 110, 30, "Timer", stopwatch_state.current_mode == MODE_TIMER);
  drawGameButton(250, 50, 110, 30, "Alarms", stopwatch_state.current_mode == MODE_ALARMS);
  
  switch (stopwatch_state.current_mode) {
    case MODE_STOPWATCH:
      drawStopwatchMode();
      break;
    case MODE_TIMER:
      drawTimerMode();
      break;
    case MODE_ALARMS:
      drawAlarmsMode();
      break;
  }
  
  updateDisplay();
}

void drawStopwatchMode() {
  ThemeColors* theme = getCurrentTheme();
  drawNavigationBar("Stopwatch", true);
  
  // Get current stopwatch time
  unsigned long elapsed = getStopwatchTime();
  int minutes = (elapsed / 1000) / 60;
  int seconds = (elapsed / 1000) % 60;
  int milliseconds = (elapsed % 1000) / 10; // Show centiseconds
  
  // Large time display
  char time_str[20];
  sprintf(time_str, "%02d:%02d.%02d", minutes, seconds, milliseconds);
  
  // Calculate text position for centering
  int text_width = strlen(time_str) * 24; // Approximate width for size 4 font
  int text_x = (DISPLAY_WIDTH - text_width) / 2;
  drawText(time_str, text_x, 150, theme->accent, 4);
  
  // Control buttons
  bool is_running = stopwatch_running;
  
  if (is_running) {
    drawGameButton(50, 250, 100, 50, "PAUSE", false);
  } else {
    drawGameButton(50, 250, 100, 50, "START", false);
  }
  
  drawGameButton(200, 250, 100, 50, "RESET", false);
  
  // Lap times area (simplified)
  drawText("Lap Times:", 20, 320, theme->secondary, 1);
  if (elapsed > 0) {
    char lap_str[30];
    sprintf(lap_str, "Lap 1: %s", time_str);
    drawText(lap_str, 20, 340, theme->text, 1);
  }
  
  // Instructions
  drawCenteredText("Tap START to begin timing", DISPLAY_WIDTH/2, 380, theme->secondary, 1);
  if (is_running) {
    drawCenteredText("Running... tap PAUSE to stop", DISPLAY_WIDTH/2, 400, theme->secondary, 1);
  }
}

void drawTimerMode() {
  ThemeColors* theme = getCurrentTheme();
  drawNavigationBar("Timer", true);
  
  // Timer display
  int display_minutes, display_seconds;
  
  if (stopwatch_state.timer_running) {
    // Calculate remaining time
    unsigned long elapsed = (millis() - stopwatch_state.timer_start_time) / 1000;
    int remaining = stopwatch_state.timer_total_seconds - elapsed;
    
    if (remaining <= 0) {
      // Timer finished
      remaining = 0;
      stopwatch_state.timer_running = false;
      showTimerFinishedNotification();
    }
    
    display_minutes = remaining / 60;
    display_seconds = remaining % 60;
  } else {
    display_minutes = stopwatch_state.timer_minutes;
    display_seconds = stopwatch_state.timer_seconds;
  }
  
  // Large countdown display
  char time_str[20];
  sprintf(time_str, "%02d:%02d", display_minutes, display_seconds);
  
  int text_width = strlen(time_str) * 24;
  int text_x = (DISPLAY_WIDTH - text_width) / 2;
  
  uint16_t time_color = theme->accent;
  if (stopwatch_state.timer_running && display_minutes == 0 && display_seconds <= 10) {
    time_color = COLOR_RED; // Flash red in last 10 seconds
  }
  
  drawText(time_str, text_x, 150, time_color, 4);
  
  // Progress ring
  if (stopwatch_state.timer_running) {
    float progress = (float)(stopwatch_state.timer_total_seconds - display_minutes * 60 - display_seconds) / stopwatch_state.timer_total_seconds;
    drawProgressRing(DISPLAY_WIDTH/2, 120, 80, progress, theme->accent, 5);
  }
  
  // Time setting buttons (when not running)
  if (!stopwatch_state.timer_running) {
    drawGameButton(20, 220, 60, 30, "+1m", false);
    drawGameButton(90, 220, 60, 30, "+10s", false);
    drawGameButton(160, 220, 60, 30, "-10s", false);
    drawGameButton(230, 220, 60, 30, "-1m", false);
    
    drawCenteredText("Set timer duration", DISPLAY_WIDTH/2, 270, theme->secondary, 1);
  }
  
  // Control buttons
  if (stopwatch_state.timer_running) {
    drawGameButton(50, 300, 100, 50, "PAUSE", false);
    drawGameButton(200, 300, 100, 50, "STOP", false);
  } else {
    drawGameButton(125, 300, 100, 50, "START", false);
  }
  
  // Quick preset buttons
  drawGameButton(20, 380, 70, 30, "1min", false);
  drawGameButton(100, 380, 70, 30, "5min", false);
  drawGameButton(180, 380, 70, 30, "10min", false);
  drawGameButton(260, 380, 70, 30, "30min", false);
}

void drawAlarmsMode() {
  ThemeColors* theme = getCurrentTheme();
  drawNavigationBar("Alarms", true);
  
  // Show 3 alarms on screen
  for (int i = 0; i < 3; i++) {
    Alarm alarm = getAlarm(i);
    int y = 100 + i * 60;
    
    // Alarm card
    uint16_t card_color = alarm.enabled ? theme->accent : theme->shadow;
    fillRoundRect(20, y, DISPLAY_WIDTH - 40, 50, 8, card_color);
    drawRoundRect(20, y, DISPLAY_WIDTH - 40, 50, 8, theme->primary);
    
    // Alarm time
    char time_str[10];
    sprintf(time_str, "%02d:%02d", alarm.hour, alarm.minute);
    drawText(time_str, 30, y + 15, theme->background, 2);
    
    // Alarm label
    drawText(alarm.label.c_str(), 120, y + 10, theme->background, 1);
    
    // Days (simplified - daily)
    if (alarm.repeat_daily) {
      drawText("Daily", 120, y + 30, theme->background, 1);
    } else {
      drawText("Once", 120, y + 30, theme->background, 1);
    }
    
    // Enable/disable toggle
    drawText(alarm.enabled ? "ON" : "OFF", 280, y + 20, theme->background, 1);
  }
  
  // Add new alarm button
  drawGameButton(20, 300, DISPLAY_WIDTH - 40, 40, "Add New Alarm", false);
  
  // Instructions
  drawCenteredText("Tap alarm to edit", DISPLAY_WIDTH/2, 360, theme->secondary, 1);
  drawCenteredText("Swipe for more alarms", DISPLAY_WIDTH/2, 380, theme->secondary, 1);
}

void handleStopwatchTimerTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  // Mode tab selection
  if (gesture.y >= 50 && gesture.y <= 80) {
    if (gesture.x >= 10 && gesture.x <= 120) {
      stopwatch_state.current_mode = MODE_STOPWATCH;
    } else if (gesture.x >= 130 && gesture.x <= 240) {
      stopwatch_state.current_mode = MODE_TIMER;
    } else if (gesture.x >= 250 && gesture.x <= 360) {
      stopwatch_state.current_mode = MODE_ALARMS;
    }
    return;
  }
  
  switch (stopwatch_state.current_mode) {
    case MODE_STOPWATCH:
      handleStopwatchTouch(gesture);
      break;
    case MODE_TIMER:
      handleTimerTouch(gesture);
      break;
    case MODE_ALARMS:
      handleAlarmsTouch(gesture);
      break;
  }
}

void handleStopwatchTouch(TouchGesture& gesture) {
  // Start/Pause button
  if (gesture.y >= 250 && gesture.y <= 300) {
    if (gesture.x >= 50 && gesture.x <= 150) {
      if (stopwatch_running) {
        pauseStopwatch();
      } else {
        startStopwatch();
      }
    }
    // Reset button
    else if (gesture.x >= 200 && gesture.x <= 300) {
      resetStopwatch();
    }
  }
}

void handleTimerTouch(TouchGesture& gesture) {
  if (!stopwatch_state.timer_running) {
    // Time adjustment buttons
    if (gesture.y >= 220 && gesture.y <= 250) {
      if (gesture.x >= 20 && gesture.x <= 80) { // +1m
        stopwatch_state.timer_minutes++;
        if (stopwatch_state.timer_minutes > 59) stopwatch_state.timer_minutes = 59;
      } else if (gesture.x >= 90 && gesture.x <= 150) { // +10s
        stopwatch_state.timer_seconds += 10;
        if (stopwatch_state.timer_seconds >= 60) {
          stopwatch_state.timer_seconds -= 60;
          stopwatch_state.timer_minutes++;
        }
      } else if (gesture.x >= 160 && gesture.x <= 220) { // -10s
        stopwatch_state.timer_seconds -= 10;
        if (stopwatch_state.timer_seconds < 0) {
          stopwatch_state.timer_seconds += 60;
          if (stopwatch_state.timer_minutes > 0) stopwatch_state.timer_minutes--;
        }
      } else if (gesture.x >= 230 && gesture.x <= 290) { // -1m
        if (stopwatch_state.timer_minutes > 0) stopwatch_state.timer_minutes--;
      }
      
      // Update total seconds
      stopwatch_state.timer_total_seconds = stopwatch_state.timer_minutes * 60 + stopwatch_state.timer_seconds;
    }
    
    // Preset buttons
    if (gesture.y >= 380 && gesture.y <= 410) {
      if (gesture.x >= 20 && gesture.x <= 90) { // 1min
        stopwatch_state.timer_minutes = 1;
        stopwatch_state.timer_seconds = 0;
      } else if (gesture.x >= 100 && gesture.x <= 170) { // 5min
        stopwatch_state.timer_minutes = 5;
        stopwatch_state.timer_seconds = 0;
      } else if (gesture.x >= 180 && gesture.x <= 250) { // 10min
        stopwatch_state.timer_minutes = 10;
        stopwatch_state.timer_seconds = 0;
      } else if (gesture.x >= 260 && gesture.x <= 330) { // 30min
        stopwatch_state.timer_minutes = 30;
        stopwatch_state.timer_seconds = 0;
      }
      stopwatch_state.timer_total_seconds = stopwatch_state.timer_minutes * 60 + stopwatch_state.timer_seconds;
    }
  }
  
  // Control buttons
  if (gesture.y >= 300 && gesture.y <= 350) {
    if (stopwatch_state.timer_running) {
      if (gesture.x >= 50 && gesture.x <= 150) { // PAUSE
        stopwatch_state.timer_running = false;
      } else if (gesture.x >= 200 && gesture.x <= 300) { // STOP
        stopwatch_state.timer_running = false;
        stopwatch_state.timer_minutes = 5; // Reset to default
        stopwatch_state.timer_seconds = 0;
        stopwatch_state.timer_total_seconds = 300;
      }
    } else {
      if (gesture.x >= 125 && gesture.x <= 225) { // START
        startTimer(stopwatch_state.timer_minutes, "Custom Timer");
        stopwatch_state.timer_running = true;
        stopwatch_state.timer_start_time = millis();
      }
    }
  }
}

void handleAlarmsTouch(TouchGesture& gesture) {
  // Alarm cards
  for (int i = 0; i < 3; i++) {
    int y = 100 + i * 60;
    if (gesture.y >= y && gesture.y <= y + 50) {
      // Toggle alarm on/off
      if (gesture.x >= 280 && gesture.x <= 320) {
        Alarm alarm = getAlarm(i);
        alarm.enabled = !alarm.enabled;
        setAlarm(i, alarm);
      } else {
        // Edit alarm (simplified - just change time by 1 hour)
        Alarm alarm = getAlarm(i);
        alarm.hour = (alarm.hour + 1) % 24;
        setAlarm(i, alarm);
      }
      return;
    }
  }
  
  // Add new alarm button
  if (gesture.y >= 300 && gesture.y <= 340) {
    // Create new alarm at current time + 1 hour
    WatchTime current = getCurrentTime();
    Alarm new_alarm;
    new_alarm.hour = (current.hour + 1) % 24;
    new_alarm.minute = current.minute;
    new_alarm.enabled = true;
    new_alarm.repeat_daily = true;
    new_alarm.label = "New Alarm";
    new_alarm.vibration = true;
    new_alarm.snooze_minutes = 5;
    
    setAlarm(3, new_alarm); // Set to slot 3
  }
}

void showTimerFinishedNotification() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Timer finished screen
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->accent);
  
  // Bell icon
  drawCircle(DISPLAY_WIDTH/2, 150, 40, theme->background);
  fillCircle(DISPLAY_WIDTH/2, 150, 35, theme->background);
  fillCircle(DISPLAY_WIDTH/2, 190, 8, theme->background);
  
  drawCenteredText("TIMER FINISHED!", DISPLAY_WIDTH/2, 220, theme->background, 3);
  drawCenteredText("Time's up!", DISPLAY_WIDTH/2, 260, theme->background, 2);
  
  drawGameButton(50, 320, DISPLAY_WIDTH - 100, 50, "OK", false);
  
  updateDisplay();
  
  // This would trigger sound/vibration
  Serial.println("Timer finished - would play alarm sound");
}