# ESP32-S3 Apple Watch Style Smartwatch

## Overview
This firmware creates an Apple Watch-inspired smartwatch for the ESP32-S3-Touch-AMOLED-1.8 development board with anime character theming featuring Luffy Gear 5, Sung Jin Woo, and Yugo from Wakfu.

## Hardware Requirements
- **Board**: ESP32-S3-Touch-AMOLED-1.8 Development Board
- **Display**: 1.8" AMOLED capacitive touch (368×448, SH8601 driver + FT3168 touch)
- **Sensors**: QMI8658 6-axis IMU, PCF85063 RTC
- **Power**: AXP2101 power management + 3.7V lithium battery
- **Storage**: MicroSD card (for music and PDF files)
- **Memory**: 16MB Flash + 8MB PSRAM

## Key Features

### 🎌 Anime Character Themes
- **Luffy Gear 5**: White/gold Sun God Nika theme with stretchy animations
- **Sung Jin Woo**: Dark purple Shadow Monarch theme with leveling system
- **Yugo Wakfu**: Teal/blue portal theme with Wakfu energy effects

### ⌚ Apple Watch-Style Interface
- Smooth animations and transitions
- Digital crown simulation via touch gestures
- Activity rings themed to characters
- App grid with icon layout
- Customizable complications

### 🎮 Gamified Step Tracking ("Urgent Quests")
- Character-specific daily quests
- Urgent time-limited challenges
- XP and leveling system
- Achievement notifications
- Progress tracking with anime references

### 🎵 Smart Music Player
- Auto-detects MP3 files from SD card
- Album art display
- Touch controls (play/pause/skip)
- Volume control via touch slider
- Supports /Music folder organization

### 📄 PDF Reader
- Auto-detects PDF files from SD card
- Touch navigation and zoom
- File browser integration
- Supports /Documents folder organization

### 📝 Notes Application
- Touch-based text input
- Save/load notes to SD card
- Quick access from app grid

### ⚡ Advanced Power Management
- AXP2101 integration for battery monitoring
- Smart sleep modes
- Charging animation
- Low battery warnings
- Power optimization

### 📱 Additional Features
- Real-time clock with alarms
- Weather display
- File browser
- Settings customization
- Touch gesture recognition
- Wrist raise detection

## File Structure
```
ESP32_Watch/
├── ESP32_Watch.ino          # Main application
├── config.h                 # Pin definitions & settings
├── display.h/.cpp          # AMOLED display management  
├── touch.h/.cpp            # Touch input handling
├── themes.h/.cpp           # Character theme system
├── sensors.h/.cpp          # IMU sensor integration
├── quests.h/.cpp           # Gamified quest system
├── apps.h                  # Application framework
├── music_app.cpp           # Music player implementation
├── filesystem.h/.cpp       # SD card file management
├── power.h                 # Power management
├── rtc.h                   # Real-time clock
└── ui.h                    # UI framework
```

## Arduino IDE Setup

### Required Libraries
```cpp
// Install these libraries through Arduino Library Manager:
#include <TFT_eSPI.h>        // Display driver
#include <Wire.h>            // I2C communication
#include <SD.h>              // SD card support
#include <FS.h>              // File system
#include <WiFi.h>            // WiFi connectivity
#include <BluetoothSerial.h> // Bluetooth support
```

### TFT_eSPI Configuration
Create or modify `TFT_eSPI/User_Setup.h`:
```cpp
#define USER_SETUP_INFO "ESP32_S3_AMOLED"

#define SH8601_DRIVER    // AMOLED display driver
#define TFT_WIDTH  368
#define TFT_HEIGHT 448

#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_CS   37
#define TFT_DC   38
#define TFT_RST  39
#define TFT_BL   40

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

#define SPI_FREQUENCY  40000000
```

## SD Card Setup

### Required Folders
Create these folders on your MicroSD card:
```
/Music/          # MP3 audio files
/Documents/      # PDF files  
/Notes/          # Text notes (auto-created)
/Cache/          # System cache (auto-created)
```

### Supported File Formats
- **Music**: MP3, WAV, M4A
- **Documents**: PDF
- **Notes**: TXT
- **Images**: JPG, PNG (for future enhancements)

## Usage Guide

### Basic Controls
- **Single Tap**: Select/activate
- **Double Tap**: Wake screen or go to home
- **Long Press**: Context menu or back
- **Swipe Up**: Open app grid
- **Swipe Down**: Notifications/control center
- **Swipe Left/Right**: Navigate between screens
- **Vertical Scroll**: Digital crown simulation

### Quest System
1. Daily quests generate automatically for each character theme
2. Complete step goals to earn XP and level up
3. Urgent quests appear with time limits for bonus rewards
4. Progress is tracked across all character themes

### Music Player
1. Copy MP3 files to `/Music/` folder on SD card
2. Open Music app from app grid
3. Use touch controls to play/pause/skip
4. Drag on volume bar to adjust volume
5. Files are automatically detected and loaded

### PDF Reader
1. Copy PDF files to `/Documents/` folder on SD card
2. Open File Browser or directly open PDF app
3. Tap to select and view PDFs
4. Swipe to navigate pages (basic implementation)

## Memory Optimization
The firmware is optimized for 16MB Flash + 8MB PSRAM:
- Display buffers allocated in PSRAM
- File arrays stored in PSRAM  
- Efficient graphics rendering with minimal memory footprint
- Smart caching system for SD card files
- Lazy loading of app resources

## Character Theme Details

### Luffy Gear 5 Theme
- **Colors**: White, gold, cream
- **Features**: Sun symbol, stretchy animations, "Nika" references
- **Activity Rings**: Stretch, Gear, Freedom
- **Quest Examples**: "Gear Second Rush", "Pirate King's Journey"

### Sung Jin Woo Theme  
- **Colors**: Purple, dark, silver, violet
- **Features**: Shadow effects, "ARISE" notifications, leveling system
- **Activity Rings**: Shadow Power, Monarch, Army
- **Quest Examples**: "Shadow Army March", "Monarch's Challenge"

### Yugo Wakfu Theme
- **Colors**: Teal, blue, light blue, energy cyan
- **Features**: Portal rings, geometric patterns, Wakfu energy
- **Activity Rings**: Wakfu Energy, Portal, Brotherhood  
- **Quest Examples**: "Portal Training", "Brotherhood Quest"

## Customization

### Adding New Themes
1. Define colors in `config.h`
2. Add theme structure in `themes.cpp`
3. Implement watch face in `themes.cpp`
4. Add quest templates in `quests.cpp`

### Adding New Apps
1. Define app structure in `apps.h`
2. Implement init/draw/touch functions
3. Register app in app array
4. Add icon and navigation

## Troubleshooting

### Display Issues
- Check TFT_eSPI configuration
- Verify pin connections
- Ensure adequate power supply

### SD Card Problems
- Format as FAT32
- Check file system initialization
- Verify pin connections
- Ensure adequate power

### Touch Not Responsive
- Check I2C connections
- Verify touch controller address
- Run touch calibration

### Battery Issues
- Check AXP2101 connections
- Monitor power consumption
- Ensure proper charging circuit

## Performance Notes
- Target 60 FPS for smooth animations
- Sensor updates at 10Hz for efficiency
- Display updates optimized for battery life
- File operations cached for responsiveness

## Future Enhancements
- Voice commands via microphone
- Heart rate monitoring (if hardware available)
- More character themes
- Advanced PDF rendering
- Music visualization
- Weather API integration
- Fitness tracking algorithms

## License
This project is open source. Feel free to modify and enhance for your specific needs.

---

**Note**: This firmware is designed specifically for the ESP32-S3-Touch-AMOLED-1.8 development board. Pin definitions and hardware-specific code may need adjustment for other boards.