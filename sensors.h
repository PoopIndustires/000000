/*
 * Sensor Management for ESP32-S3 Watch
 * QMI8658 6-Axis IMU Integration
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include <Wire.h>

// IMU data structure
struct IMUData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float temperature;
  unsigned long timestamp;
};

// Step counter data
struct StepData {
  int daily_steps;
  int calories_burned;
  float distance_km;
  int active_minutes;
  unsigned long last_step_time;
};

// Initialize sensor systems
bool initializeSensors();

// IMU functions
bool initializeIMU();
IMUData readIMU();
void calibrateIMU();

// Step counting
void updateStepCounter();
bool detectStep(IMUData& imu);
void resetDailySteps();
int getDailySteps();

// Activity detection
bool isMoving();
bool isRunning();
String getCurrentActivity();

// Gesture recognition using IMU
bool detectWristRaise();
bool detectWristFlick();
bool detectDoubleTab();

// Heart rate simulation (for demonstration)
float getHeartRate();

// Sensor data processing
void processSensorData();
void updateActivityMetrics();

#endif // SENSORS_H