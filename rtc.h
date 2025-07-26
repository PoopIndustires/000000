/*
 * Real Time Clock Management for ESP32-S3 Watch
 * PCF85063 RTC Integration
 */

#ifndef RTC_H
#define RTC_H

#include "config.h"
#include <Wire.h>
#include <time.h>

// Time structure
struct WatchTime {
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
  int weekday;
};

// Alarm structure
struct Alarm {
  int hour;
  int minute;
  bool enabled;
  bool repeat_daily;
  String label;
  bool vibration;
  int snooze_minutes;
};

// Initialize RTC system
bool initializeRTC();

// Time functions
WatchTime getCurrentTime();
void setCurrentTime(WatchTime& time);
void syncTimeWithWiFi();
bool isValidTime(WatchTime& time);

// Time display functions
String formatTime(WatchTime& time, bool twelve_hour = false);
String formatDate(WatchTime& time);
String formatDateTime(WatchTime& time);

// Timezone management
void setTimezone(int offset_hours);
int getTimezone();
void handleDaylightSaving();

// Alarm functions
void setAlarm(int id, Alarm& alarm);
Alarm getAlarm(int id);
void enableAlarm(int id, bool enabled);
void checkAlarms();
void triggerAlarm(int id);
void snoozeAlarm(int id);
void dismissAlarm(int id);

// Timer functions
void startTimer(int minutes, String label = "");
void stopTimer();
void pauseTimer();
void resumeTimer();
int getTimerRemaining();

// Stopwatch functions
void startStopwatch();
void stopStopwatch();
void pauseStopwatch();
void resumeStopwatch();
void resetStopwatch();
unsigned long getStopwatchTime();

// Time-based automation
void checkTimeBasedEvents();
void setWakeUpTime(int hour, int minute);
void setSleepTime(int hour, int minute);

// RTC maintenance
void calibrateRTC();
bool isRTCBatteryLow();
void replaceRTCBattery();

#endif // RTC_H