# ESP32-S3 Watch Installation Guide

## Prerequisites

### Hardware Required
- ESP32-S3-Touch-AMOLED-1.8 Development Board
- MicroSD card (Class 10, 8GB+ recommended)
- USB-C cable for programming
- 3.7V lithium battery (optional but recommended)

### Software Required
- Arduino IDE 2.0+
- ESP32 Board Package
- Required Arduino Libraries

## Step-by-Step Installation

### 1. Arduino IDE Setup

1. **Install Arduino IDE 2.0+**
   - Download from: https://www.arduino.cc/en/software
   - Install and launch Arduino IDE

2. **Install ESP32 Board Package**
   - Go to File → Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Board Manager
   - Search for "esp32" and install "ESP32 by Espressif Systems"

3. **Select Board**
   - Go to Tools → Board → ESP32 Arduino
   - Select "ESP32S3 Dev Module"

### 2. Board Configuration

Configure the following settings in Arduino IDE:

- **Board**: "ESP32S3 Dev Module"
- **Upload Speed**: "921600"
- **USB Mode**: "Hardware CDC and JTAG"  
- **USB CDC On Boot**: "Enabled"
- **USB Firmware MSC On Boot**: "Disabled"
- **USB DFU On Boot**: "Disabled"
- **Upload Mode**: "UART0 / Hardware CDC"
- **CPU Frequency**: "240MHz (WiFi)"
- **Flash Mode**: "QIO 80MHz"
- **Flash Size**: "16MB (128Mb)"
- **Partition Scheme**: "16M Flash (3MB APP/9.9MB FATFS)"
- **Core Debug Level**: "None"
- **PSRAM**: "OPI PSRAM"

### 3. Install Required Libraries

Install these libraries through Arduino Library Manager (Tools → Manage Libraries):

```
TFT_eSPI (by Bodmer) - Version 2.5.0+
```

**Important**: Other libraries are included in the project files or use built-in ESP32 libraries.

### 4. TFT_eSPI Configuration

The most critical step is configuring TFT_eSPI for the AMOLED display.

**Option A: Modify User_Setup.h**

1. Navigate to your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\TFT_eSPI\`
   - Mac: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - Linux: `~/Arduino/libraries/TFT_eSPI/`

2. Open `User_Setup.h` and replace the contents with:

```cpp
// ESP32-S3 AMOLED Watch Configuration
#define USER_SETUP_ID 303

// Display driver
#define SH8601_DRIVER

// Display size
#define TFT_WIDTH  368
#define TFT_HEIGHT 448

// Pin definitions for ESP32-S3
#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_CS   37
#define TFT_DC   38
#define TFT_RST  39
#define TFT_BL   40

// Font loading
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

// SPI frequency
#define SPI_FREQUENCY  40000000

// Optional: Enable smooth fonts
#define SMOOTH_FONT
```

**Option B: Use User_Setup_Select.h**

1. Open `User_Setup_Select.h`
2. Comment out the default setup line:
   ```cpp
   //#include <User_Setup.h>
   ```
3. Add this line:
   ```cpp
   #include <User_Setups/Setup303_ESP32S3_AMOLED.h>
   ```

4. Create the file `User_Setups/Setup303_ESP32S3_AMOLED.h` with the configuration above.

### 5. Project File Structure

Organize your project files as follows:

```
ESP32_Watch/
├── ESP32_Watch.ino          # Main file (must match folder name!)
├── config.h
├── display.h
├── display.cpp
├── touch.h  
├── touch.cpp
├── themes.h
├── themes.cpp
├── sensors.h
├── sensors.cpp
├── quests.h
├── quests.cpp
├── games.h
├── games.cpp
├── apps.h
├── apps.cpp
├── music_app.cpp
├── filesystem.h
├── filesystem.cpp
├── power.h
├── power.cpp
├── rtc.h
├── rtc.cpp
├── ui.h
└── ui.cpp
```

**Critical**: The main `.ino` file MUST have the same name as the folder!

### 6. Hardware Connections

The ESP32-S3-Touch-AMOLED-1.8 board has integrated connections, but verify:

- **Display (SH8601)**: Connected via QSPI
- **Touch (FT3168)**: Connected via I2C
- **IMU (QMI8658)**: Connected via I2C  
- **RTC (PCF85063)**: Connected via I2C
- **Power (AXP2101)**: Connected via I2C
- **SD Card**: Connected via SPI

### 7. MicroSD Card Setup

1. **Format SD card as FAT32**

2. **Create folder structure**:
   ```
   /Music/          # Put MP3 files here
   /Documents/      # Put PDF files here
   /Notes/          # Will be created automatically
   /Cache/          # Will be created automatically
   ```

3. **Add sample files**:
   - Copy some MP3 files to `/Music/` folder
   - Copy some PDF files to `/Documents/` folder

### 8. Compilation and Upload

1. **Open the project**:
   - Open `ESP32_Watch.ino` in Arduino IDE
   - Verify all files are loaded in tabs

2. **Compile the code**:
   - Click "Verify" button (✓)
   - Fix any compilation errors

3. **Upload to board**:
   - Connect ESP32-S3 board via USB-C
   - Select correct COM port in Tools → Port
   - Click "Upload" button (→)

### 9. First Boot

After successful upload:

1. **Insert MicroSD card** (if not already inserted)
2. **Power on** the device
3. **Watch should display**:
   - Splash screen with loading animation
   - Default Luffy Gear 5 watch face
   - Touch should be responsive

### 10. Testing Features

Test each major feature:

1. **Touch Navigation**:
   - Swipe up from watch face to open app grid
   - Tap apps to launch them

2. **Theme Switching**:
   - Open Settings app
   - Tap different theme buttons (Luffy, Jin Woo, Yugo)

3. **Music Player**:
   - Ensure MP3 files are detected
   - Test play/pause controls

4. **Games**:
   - Open Games app
   - Try Battle Arena and Snake games

5. **Step Counter**:
   - Move the watch to test step detection
   - Check quest progress

## Troubleshooting

### Common Issues

**1. Compilation Errors**
- Verify all files are in the same folder
- Check TFT_eSPI configuration
- Ensure ESP32 board package is installed

**2. Display Not Working**
- Double-check TFT_eSPI pin configuration
- Verify SPI connections
- Check display power (TFT_BL pin)

**3. Touch Not Responsive**
- Verify I2C connections
- Check touch controller initialization
- Calibrate touch if needed

**4. SD Card Not Detected**
- Ensure SD card is FAT32 formatted
- Check SPI pin connections
- Verify SD card is inserted properly

**5. Upload Failures**
- Hold BOOT button while connecting USB
- Try different USB cable
- Check COM port selection
- Reduce upload speed to 115200

### Debug Output

Enable serial debugging:
1. Open Serial Monitor (Tools → Serial Monitor)
2. Set baud rate to 115200
3. Watch debug messages during operation

### Memory Issues

If running low on memory:
1. Reduce number of cached files
2. Optimize graphics buffers
3. Use PROGMEM for static data

### Performance Issues

If animations are slow:
1. Reduce display update frequency
2. Optimize drawing functions
3. Use hardware acceleration where possible

## Advanced Configuration

### Custom Themes

Create new character themes by:
1. Adding colors to `config.h`
2. Implementing theme structure in `themes.cpp`
3. Adding watch face drawing function

### Additional Games

Add new games by:
1. Implementing game logic in `games.cpp`
2. Adding game type to enum in `games.h`
3. Creating draw and touch handlers

### Hardware Modifications

For custom hardware:
1. Update pin definitions in `config.h`
2. Modify TFT_eSPI configuration
3. Adjust I2C addresses if different

## Support

If you encounter issues:
1. Check serial debug output
2. Verify hardware connections
3. Test individual components
4. Review this guide thoroughly

The firmware is designed to be modular and expandable. Each major component can be tested and debugged independently.

## Updates

To update the firmware:
1. Download new version
2. Backup your settings and files
3. Upload new firmware
4. Restore your custom settings

---

**Congratulations!** You now have a fully functional anime-themed Apple Watch-style smartwatch with games, step tracking, music player, and PDF reader capabilities!