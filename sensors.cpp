/*
 * Sensor Management Implementation
 * QMI8658 6-Axis IMU for ESP32-S3 Watch
 */

#include "sensors.h"

// Sensor state variables
IMUData current_imu;
StepData step_data;
float step_threshold = MIN_STEP_THRESHOLD;
unsigned long last_step_check = 0;
bool step_detection_active = true;

bool initializeSensors() {
  Serial.println("Initializing sensors...");
  
  if (!initializeIMU()) {
    Serial.println("IMU initialization failed!");
    return false;
  }
  
  // Initialize step counter
  step_data.daily_steps = 0;
  step_data.calories_burned = 0;
  step_data.distance_km = 0.0;
  step_data.active_minutes = 0;
  step_data.last_step_time = 0;
  
  Serial.println("Sensors initialized successfully");
  return true;
}

bool initializeIMU() {
  // Initialize I2C for QMI8658
  Wire1.begin(IMU_SDA, IMU_SCL);
  Wire1.setClock(400000);
  
  // QMI8658 I2C address is typically 0x6A or 0x6B
  uint8_t imu_address = 0x6A;
  
  // Test communication
  Wire1.beginTransmission(imu_address);
  if (Wire1.endTransmission() != 0) {
    imu_address = 0x6B;
    Wire1.beginTransmission(imu_address);
    if (Wire1.endTransmission() != 0) {
      Serial.println("QMI8658 IMU not found!");
      return false;
    }
  }
  
  // Configure IMU settings
  // Enable accelerometer and gyroscope
  Wire1.beginTransmission(imu_address);
  Wire1.write(0x02); // CTRL2 register
  Wire1.write(0x05); // Enable accel and gyro
  Wire1.endTransmission();
  
  // Set accelerometer range to ±4g
  Wire1.beginTransmission(imu_address);
  Wire1.write(0x03); // CTRL3 register
  Wire1.write(0x04); // ±4g range, 100Hz ODR
  Wire1.endTransmission();
  
  // Set gyroscope range to ±512 dps
  Wire1.beginTransmission(imu_address);
  Wire1.write(0x04); // CTRL4 register
  Wire1.write(0x04); // ±512 dps, 100Hz ODR
  Wire1.endTransmission();
  
  Serial.println("QMI8658 IMU configured successfully");
  return true;
}

IMUData readIMU() {
  IMUData imu_data;
  uint8_t imu_address = 0x6A; // Assuming successful initialization
  
  // Read accelerometer data (6 bytes)
  Wire1.beginTransmission(imu_address);
  Wire1.write(0x35); // Accel X LSB register
  Wire1.endTransmission();
  
  Wire1.requestFrom(imu_address, 6);
  if (Wire1.available() >= 6) {
    int16_t ax = Wire1.read() | (Wire1.read() << 8);
    int16_t ay = Wire1.read() | (Wire1.read() << 8);
    int16_t az = Wire1.read() | (Wire1.read() << 8);
    
    // Convert to g-force (±4g range, 16-bit)
    imu_data.accel_x = ax / 8192.0;
    imu_data.accel_y = ay / 8192.0;
    imu_data.accel_z = az / 8192.0;
  }
  
  // Read gyroscope data (6 bytes)
  Wire1.beginTransmission(imu_address);
  Wire1.write(0x3B); // Gyro X LSB register
  Wire1.endTransmission();
  
  Wire1.requestFrom(imu_address, 6);
  if (Wire1.available() >= 6) {
    int16_t gx = Wire1.read() | (Wire1.read() << 8);
    int16_t gy = Wire1.read() | (Wire1.read() << 8);
    int16_t gz = Wire1.read() | (Wire1.read() << 8);
    
    // Convert to degrees per second (±512 dps range)
    imu_data.gyro_x = gx / 64.0;
    imu_data.gyro_y = gy / 64.0;
    imu_data.gyro_z = gz / 64.0;
  }
  
  imu_data.timestamp = millis();
  return imu_data;
}

void updateStepCounter() {
  if (!step_detection_active) return;
  
  unsigned long current_time = millis();
  if (current_time - last_step_check < 50) return; // Limit to 20Hz
  
  current_imu = readIMU();
  
  if (detectStep(current_imu)) {
    step_data.daily_steps++;
    step_data.last_step_time = current_time;
    
    // Calculate calories (rough approximation)
    step_data.calories_burned = step_data.daily_steps * 0.04; // ~0.04 cal per step
    
    // Calculate distance (rough approximation)
    step_data.distance_km = step_data.daily_steps * 0.0007; // ~70cm per step
  }
  
  last_step_check = current_time;
}

bool detectStep(IMUData& imu) {
  static float last_magnitude = 0;
  static unsigned long last_step = 0;
  static bool step_in_progress = false;
  
  // Calculate magnitude of acceleration
  float magnitude = sqrt(imu.accel_x * imu.accel_x + 
                        imu.accel_y * imu.accel_y + 
                        imu.accel_z * imu.accel_z);
  
  unsigned long current_time = millis();
  
  // Step detection algorithm
  // Look for peaks above threshold with proper timing
  if (magnitude > step_threshold && !step_in_progress && 
      current_time - last_step > STEP_TIME_WINDOW) {
    
    // Confirm it's a step by checking if it's followed by a valley
    if (last_magnitude < step_threshold * 0.8) {
      step_in_progress = true;
      last_step = current_time;
      last_magnitude = magnitude;
      return true;
    }
  }
  
  // Reset step detection state
  if (magnitude < step_threshold * 0.7) {
    step_in_progress = false;
  }
  
  last_magnitude = magnitude;
  return false;
}

void resetDailySteps() {
  step_data.daily_steps = 0;
  step_data.calories_burned = 0;
  step_data.distance_km = 0.0;
  step_data.active_minutes = 0;
}

int getDailySteps() {
  return step_data.daily_steps;
}

bool isMoving() {
  float magnitude = sqrt(current_imu.accel_x * current_imu.accel_x + 
                        current_imu.accel_y * current_imu.accel_y + 
                        current_imu.accel_z * current_imu.accel_z);
  
  return magnitude > 1.2; // Above 1.2g indicates movement
}

bool isRunning() {
  // Check for higher frequency and magnitude changes
  return isMoving() && abs(current_imu.gyro_y) > 50; // High angular velocity
}

String getCurrentActivity() {
  if (isRunning()) {
    return "Running";
  } else if (isMoving()) {
    return "Walking";
  } else {
    return "Stationary";
  }
}

bool detectWristRaise() {
  // Detect wrist raise gesture for screen wake
  static float baseline_z = -9.8; // Gravity when arm is down
  
  if (current_imu.accel_z > baseline_z + 3.0 && // Wrist raised
      abs(current_imu.gyro_x) > 30) { // Quick rotation
    return true;
  }
  return false;
}

bool detectWristFlick() {
  // Detect quick wrist flick for navigation
  return abs(current_imu.gyro_z) > 100; // Quick Z-axis rotation
}

bool detectDoubleTab() {
  // Detect double tap on watch face
  static unsigned long last_tap = 0;
  static int tap_count = 0;
  
  float magnitude = sqrt(current_imu.accel_x * current_imu.accel_x + 
                        current_imu.accel_y * current_imu.accel_y + 
                        current_imu.accel_z * current_imu.accel_z);
  
  if (magnitude > 15.0) { // Sharp acceleration spike
    unsigned long current_time = millis();
    
    if (current_time - last_tap < 500) {
      tap_count++;
    } else {
      tap_count = 1;
    }
    
    last_tap = current_time;
    
    if (tap_count >= 2) {
      tap_count = 0;
      return true;
    }
  }
  
  return false;
}

float getHeartRate() {
  // Simulate heart rate based on activity level
  if (isRunning()) {
    return 140 + random(-10, 10);
  } else if (isMoving()) {
    return 90 + random(-5, 5);
  } else {
    return 70 + random(-3, 3);
  }
}

void processSensorData() {
  updateStepCounter();
  updateActivityMetrics();
}

void updateActivityMetrics() {
  // Update activity ring progress and other metrics
  static unsigned long last_activity_update = 0;
  unsigned long current_time = millis();
  
  if (current_time - last_activity_update > 60000) { // Update every minute
    if (isMoving()) {
      step_data.active_minutes++;
    }
    last_activity_update = current_time;
  }
}

void calibrateIMU() {
  Serial.println("Calibrating IMU... Keep watch still for 5 seconds");
  
  // Calibration routine
  float accel_offset_x = 0, accel_offset_y = 0, accel_offset_z = 0;
  float gyro_offset_x = 0, gyro_offset_y = 0, gyro_offset_z = 0;
  int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    IMUData cal_data = readIMU();
    
    accel_offset_x += cal_data.accel_x;
    accel_offset_y += cal_data.accel_y;
    accel_offset_z += cal_data.accel_z - 1.0; // Account for gravity
    
    gyro_offset_x += cal_data.gyro_x;
    gyro_offset_y += cal_data.gyro_y;
    gyro_offset_z += cal_data.gyro_z;
    
    delay(50);
  }
  
  // Calculate offsets
  accel_offset_x /= samples;
  accel_offset_y /= samples;
  accel_offset_z /= samples;
  
  gyro_offset_x /= samples;
  gyro_offset_y /= samples;
  gyro_offset_z /= samples;
  
  // Store calibration values (would typically save to EEPROM)
  Serial.println("IMU calibration completed");
}