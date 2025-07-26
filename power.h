/*
 * Power Management for ESP32-S3 Watch
 * AXP2101 Power Management IC Integration
 */

#ifndef POWER_H
#define POWER_H

#include "config.h"
#include <Wire.h>

// Power states
enum PowerState {
  POWER_ACTIVE,
  POWER_IDLE,
  POWER_SLEEP,
  POWER_DEEP_SLEEP,
  POWER_CHARGING
};

// Battery info structure
struct BatteryInfo {
  int percentage;
  int voltage_mv;
  int current_ma;
  int temperature;
  bool is_charging;
  bool is_plugged;
  unsigned long charge_time_remaining;
  unsigned long estimated_runtime;
};

// Power management functions
bool initializePower();

// Battery monitoring
BatteryInfo updateBatteryStatus();
int getBatteryPercentage();
int getBatteryVoltage();
bool isCharging();
bool isPluggedIn();

// Power state management
void setPowerState(PowerState state);
PowerState getCurrentPowerState();
void enterSleepMode();
void enterDeepSleepMode();
void wakeFromSleep();

// Display power management
void setDisplayPower(bool on);
void setDisplayBrightness(int brightness);
void enableAutoSleep(bool enabled);

// Component power control
void setPowerRail(int rail, bool enabled);
void setWiFiPower(bool enabled);
void setBluetoothPower(bool enabled);
void setSensorPower(bool enabled);
void setAudioPower(bool enabled);

// Low power optimizations
void enableLowPowerMode();
void disableLowPowerMode();
void optimizeForBatteryLife();

// Charging management
void handleChargingState();
void showChargingAnimation();
void updateChargingStatus();

// Power warnings and notifications
void showLowBatteryWarning();
void showCriticalBatteryWarning();
void showChargingNotification();

// Power usage monitoring
void logPowerUsage();
int getEstimatedRuntime();
void generatePowerReport();

#endif // POWER_H