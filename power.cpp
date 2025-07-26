/*
 * Power Management Implementation
 * AXP2101 Power Management IC for ESP32-S3 Watch
 */

#include "power.h"

// Power state variables
static PowerState current_power_state = POWER_ACTIVE;
static BatteryInfo current_battery_info;
static unsigned long last_battery_update = 0;

bool initializePower() {
  Serial.println("Initializing power management...");
  
  // Initialize I2C for AXP2101
  Wire1.begin(PWR_SDA, PWR_SCL);
  Wire1.setClock(400000);
  
  // AXP2101 I2C address is typically 0x34
  Wire1.beginTransmission(0x34);
  if (Wire1.endTransmission() != 0) {
    Serial.println("AXP2101 power management IC not found!");
    // Continue with simulated power management
  } else {
    Serial.println("AXP2101 power management IC found");
    
    // Configure AXP2101 power rails
    // Enable DCDC1 (3.3V for system)
    Wire1.beginTransmission(0x34);
    Wire1.write(0x10); // DCDC1 control register
    Wire1.write(0x80); // Enable DCDC1
    Wire1.endTransmission();
    
    // Enable LDO2 (Display power)
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12); // LDO2 control register
    Wire1.write(0x80); // Enable LDO2
    Wire1.endTransmission();
    
    // Configure battery charging
    Wire1.beginTransmission(0x34);
    Wire1.write(0x18); // Charging control register
    Wire1.write(0xC0); // Enable charging, 4.2V target
    Wire1.endTransmission();
  }
  
  // Initialize battery info
  current_battery_info.percentage = 80;
  current_battery_info.voltage_mv = 3800;
  current_battery_info.current_ma = 0;
  current_battery_info.temperature = 25;
  current_battery_info.is_charging = false;
  current_battery_info.is_plugged = false;
  current_battery_info.charge_time_remaining = 0;
  current_battery_info.estimated_runtime = 8 * 3600; // 8 hours
  
  system_state.battery_percentage = current_battery_info.percentage;
  system_state.is_charging = current_battery_info.is_charging;
  
  Serial.println("Power management initialized successfully");
  return true;
}

BatteryInfo updateBatteryStatus() {
  unsigned long current_time = millis();
  
  // Update battery status every 30 seconds
  if (current_time - last_battery_update < 30000) {
    return current_battery_info;
  }
  
  last_battery_update = current_time;
  
  // Read battery voltage from AXP2101
  Wire1.beginTransmission(0x34);
  Wire1.write(0x78); // Battery voltage register
  Wire1.endTransmission();
  
  Wire1.requestFrom(0x34, 2);
  if (Wire1.available() >= 2) {
    uint16_t voltage_raw = (Wire1.read() << 4) | (Wire1.read() & 0x0F);
    current_battery_info.voltage_mv = voltage_raw * 1.1; // Convert to mV
  } else {
    // Simulate battery discharge
    static int sim_battery = 80;
    if (current_time % 60000 == 0 && sim_battery > 0) { // Every minute
      sim_battery--;
    }
    current_battery_info.voltage_mv = 3200 + (sim_battery * 600 / 100); // 3.2V to 3.8V range
  }
  
  // Convert voltage to percentage (simplified)
  if (current_battery_info.voltage_mv >= 4100) {
    current_battery_info.percentage = 100;
  } else if (current_battery_info.voltage_mv >= 3900) {
    current_battery_info.percentage = 80 + (current_battery_info.voltage_mv - 3900) * 20 / 200;
  } else if (current_battery_info.voltage_mv >= 3700) {
    current_battery_info.percentage = 40 + (current_battery_info.voltage_mv - 3700) * 40 / 200;
  } else if (current_battery_info.voltage_mv >= 3400) {
    current_battery_info.percentage = 10 + (current_battery_info.voltage_mv - 3400) * 30 / 300;
  } else {
    current_battery_info.percentage = (current_battery_info.voltage_mv - 3200) * 10 / 200;
  }
  
  // Ensure percentage is within bounds
  if (current_battery_info.percentage > 100) current_battery_info.percentage = 100;
  if (current_battery_info.percentage < 0) current_battery_info.percentage = 0;
  
  // Check charging status
  Wire1.beginTransmission(0x34);
  Wire1.write(0x01); // Power status register
  Wire1.endTransmission();
  
  Wire1.requestFrom(0x34, 1);
  if (Wire1.available()) {
    uint8_t power_status = Wire1.read();
    current_battery_info.is_charging = (power_status & 0x04) != 0;
    current_battery_info.is_plugged = (power_status & 0x20) != 0;
  }
  
  // Calculate estimated runtime
  if (current_battery_info.is_charging) {
    current_battery_info.estimated_runtime = 0xFFFFFFFF; // Infinite while charging
    current_battery_info.charge_time_remaining = (100 - current_battery_info.percentage) * 60; // ~1% per minute
  } else {
    // Estimate based on current consumption and battery level
    int estimated_hours = current_battery_info.percentage * 12 / 100; // 12 hours at 100%
    current_battery_info.estimated_runtime = estimated_hours * 3600;
  }
  
  // Update system state
  system_state.battery_percentage = current_battery_info.percentage;
  system_state.is_charging = current_battery_info.is_charging;
  
  return current_battery_info;
}

int getBatteryPercentage() {
  return current_battery_info.percentage;
}

int getBatteryVoltage() {
  return current_battery_info.voltage_mv;
}

bool isCharging() {
  return current_battery_info.is_charging;
}

bool isPluggedIn() {
  return current_battery_info.is_plugged;
}

void setPowerState(PowerState state) {
  current_power_state = state;
  
  switch (state) {
    case POWER_ACTIVE:
      // Full power mode
      setDisplayPower(true);
      setSensorPower(true);
      break;
      
    case POWER_IDLE:
      // Reduce display brightness, keep sensors active
      setDisplayBrightness(30);
      break;
      
    case POWER_SLEEP:
      // Minimal display, reduced sensor polling
      setDisplayBrightness(5);
      break;
      
    case POWER_DEEP_SLEEP:
      // Display off, minimal sensors
      setDisplayPower(false);
      break;
      
    case POWER_CHARGING:
      // Display charging animation
      setDisplayPower(true);
      setDisplayBrightness(50);
      break;
  }
}

PowerState getCurrentPowerState() {
  return current_power_state;
}

void enterSleepMode() {
  setPowerState(POWER_SLEEP);
  system_state.low_power_mode = true;
}

void enterDeepSleepMode() {
  setPowerState(POWER_DEEP_SLEEP);
  
  // Configure wake sources
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // PWR button
  esp_sleep_enable_ext1_wakeup(GPIO_NUM_10, ESP_EXT1_WAKEUP_ANY_HIGH); // Touch interrupt
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void wakeFromSleep() {
  setPowerState(POWER_ACTIVE);
  system_state.low_power_mode = false;
  setDisplayBrightness(system_state.brightness);
}

void setDisplayPower(bool on) {
  if (on) {
    // Enable display power rail
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
  } else {
    // Disable display
    digitalWrite(TFT_BL, LOW);
  }
}

void setPowerRail(int rail, bool enabled) {
  // Control individual power rails via AXP2101
  uint8_t rail_reg = 0x10 + rail; // DCDC/LDO control registers start at 0x10
  uint8_t rail_value = enabled ? 0x80 : 0x00;
  
  Wire1.beginTransmission(0x34);
  Wire1.write(rail_reg);
  Wire1.write(rail_value);
  Wire1.endTransmission();
}

void setWiFiPower(bool enabled) {
  if (enabled) {
    WiFi.mode(WIFI_STA);
  } else {
    WiFi.mode(WIFI_OFF);
  }
}

void setBluetoothPower(bool enabled) {
  if (enabled) {
    // Enable Bluetooth
  } else {
    // Disable Bluetooth to save power
  }
}

void setSensorPower(bool enabled) {
  // Control sensor power rail
  setPowerRail(3, enabled); // Assuming sensors on LDO3
}

void setAudioPower(bool enabled) {
  // Control audio codec power
  setPowerRail(4, enabled); // Assuming audio on LDO4
}

void enableLowPowerMode() {
  // Reduce CPU frequency
  setCpuFrequencyMhz(80); // Reduce from 240MHz to 80MHz
  
  // Reduce peripheral clocks
  setSensorPower(false);
  setAudioPower(false);
  
  system_state.low_power_mode = true;
}

void disableLowPowerMode() {
  // Restore full CPU frequency
  setCpuFrequencyMhz(240);
  
  // Re-enable peripherals
  setSensorPower(true);
  setAudioPower(true);
  
  system_state.low_power_mode = false;
}

void optimizeForBatteryLife() {
  // Implement aggressive power saving
  enableLowPowerMode();
  setDisplayBrightness(20);
  setWiFiPower(false);
  setBluetoothPower(false);
}

void handleChargingState() {
  if (isCharging()) {
    if (current_power_state != POWER_CHARGING) {
      setPowerState(POWER_CHARGING);
      showChargingNotification();
    }
  } else {
    if (current_power_state == POWER_CHARGING) {
      setPowerState(POWER_ACTIVE);
    }
  }
}

void showChargingAnimation() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Charging animation
  static int charging_frame = 0;
  charging_frame = (charging_frame + 1) % 60;
  
  // Battery outline
  int battery_x = DISPLAY_WIDTH/2 - 30;
  int battery_y = DISPLAY_HEIGHT/2 - 40;
  
  drawRoundRect(battery_x, battery_y, 60, 80, 8, theme->primary);
  drawRect(battery_x + 20, battery_y - 8, 20, 8, theme->primary);
  
  // Animated fill
  int fill_height = (charging_frame % 30) * 76 / 30;
  fillRoundRect(battery_x + 2, battery_y + 78 - fill_height, 56, fill_height, 6, theme->accent);
  
  // Lightning bolt
  int bolt_x = DISPLAY_WIDTH/2;
  int bolt_y = DISPLAY_HEIGHT/2;
  
  // Simple lightning bolt shape
  drawLine(bolt_x - 8, bolt_y - 15, bolt_x + 5, bolt_y, COLOR_YELLOW);
  drawLine(bolt_x + 5, bolt_y, bolt_x - 8, bolt_y + 15, COLOR_YELLOW);
  drawLine(bolt_x - 3, bolt_y - 8, bolt_x + 8, bolt_y + 8, COLOR_YELLOW);
  
  // Charging text
  drawCenteredText("Charging", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 60, theme->text, 2);
  
  char battery_text[20];
  sprintf(battery_text, "%d%%", system_state.battery_percentage);
  drawCenteredText(battery_text, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 90, theme->accent, 1);
  
  updateDisplay();
}

void updateChargingStatus() {
  updateBatteryStatus();
  handleChargingState();
}

void showLowBatteryWarning() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Warning background
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_RED);
  
  // Battery icon (empty)
  int battery_x = DISPLAY_WIDTH/2 - 30;
  int battery_y = DISPLAY_HEIGHT/2 - 40;
  
  drawRoundRect(battery_x, battery_y, 60, 80, 8, COLOR_WHITE);
  drawRect(battery_x + 20, battery_y - 8, 20, 8, COLOR_WHITE);
  
  // Warning text
  drawCenteredText("LOW BATTERY", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 60, COLOR_WHITE, 2);
  drawCenteredText("Connect charger", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 90, COLOR_WHITE, 1);
  
  char battery_text[20];
  sprintf(battery_text, "%d%%", system_state.battery_percentage);
  drawCenteredText(battery_text, DISPLAY_WIDTH/2 - 15, DISPLAY_HEIGHT/2 - 5, COLOR_RED, 3);
  
  updateDisplay();
  delay(3000); // Show warning for 3 seconds
}

void showCriticalBatteryWarning() {
  showLowBatteryWarning();
  
  // After showing warning, enter deep sleep to preserve battery
  delay(1000);
  enterDeepSleepMode();
}

void showChargingNotification() {
  // Show brief charging notification
  ThemeColors* theme = getCurrentTheme();
  
  fillRoundRect(50, DISPLAY_HEIGHT/2 - 25, DISPLAY_WIDTH - 100, 50, 12, theme->accent);
  drawCenteredText("Charging Started", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, theme->background, 1);
  
  updateDisplay();
  delay(2000);
}

void logPowerUsage() {
  // Log power usage statistics
  Serial.println("Battery: " + String(current_battery_info.percentage) + "%");
  Serial.println("Voltage: " + String(current_battery_info.voltage_mv) + "mV");
  Serial.println("Charging: " + String(current_battery_info.is_charging ? "Yes" : "No"));
  Serial.println("Estimated runtime: " + String(current_battery_info.estimated_runtime / 3600) + "h");
}

int getEstimatedRuntime() {
  return current_battery_info.estimated_runtime;
}

void generatePowerReport() {
  logPowerUsage();
  
  // Additional power analysis
  Serial.println("Power State: " + String(current_power_state));
  Serial.println("Low Power Mode: " + String(system_state.low_power_mode ? "Enabled" : "Disabled"));
  Serial.println("Display Brightness: " + String(system_state.brightness) + "%");
}