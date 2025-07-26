/*
 * File System Implementation
 * MicroSD card management for ESP32-S3 Watch
 */

#include "filesystem.h"

// Global file arrays in PSRAM
MusicFile* music_files = nullptr;
PDFFile* pdf_files = nullptr;
int total_music_files = 0;
int total_pdf_files = 0;

bool initializeFileSystem() {
  Serial.println("Initializing file system...");
  
  // Initialize SD card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }
  
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
  // Allocate file arrays in PSRAM
  music_files = (MusicFile*)ps_malloc(100 * sizeof(MusicFile));
  pdf_files = (PDFFile*)ps_malloc(50 * sizeof(PDFFile));
  
  if (!music_files || !pdf_files) {
    Serial.println("Failed to allocate file arrays!");
    return false;
  }
  
  // Initialize cache
  initializeCache();
  
  // Scan for files
  total_music_files = scanMusicFiles(music_files, 100);
  total_pdf_files = scanPDFFiles(pdf_files, 50);
  
  Serial.println("File system initialized successfully");
  Serial.printf("Found %d music files and %d PDF files\n", total_music_files, total_pdf_files);
  
  return true;
}

bool fileExists(const char* path) {
  return SD.exists(path);
}

size_t getFileSize(const char* path) {
  File file = SD.open(path);
  if (!file) return 0;
  
  size_t size = file.size();
  file.close();
  return size;
}

String readTextFile(const char* path) {
  File file = SD.open(path);
  if (!file) return "";
  
  String content = file.readString();
  file.close();
  return content;
}

bool writeTextFile(const char* path, const String& content) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) return false;
  
  file.print(content);
  file.close();
  return true;
}

bool deleteFile(const char* path) {
  return SD.remove(path);
}

bool listDirectory(const char* path, FileInfo files[], int max_files, int& count) {
  File root = SD.open(path);
  if (!root || !root.isDirectory()) {
    return false;
  }
  
  count = 0;
  File file = root.openNextFile();
  
  while (file && count < max_files) {
    files[count].filename = file.name();
    files[count].filepath = String(path) + "/" + file.name();
    files[count].size = file.size();
    files[count].is_directory = file.isDirectory();
    files[count].type = getFileType(file.name());
    
    count++;
    file = root.openNextFile();
  }
  
  root.close();
  return true;
}

int scanMusicFiles(MusicFile music_files[], int max_files) {
  File root = SD.open("/");
  if (!root) return 0;
  
  int count = 0;
  File file = root.openNextFile();
  
  while (file && count < max_files) {
    if (!file.isDirectory() && isAudioFile(file.name())) {
      music_files[count].filename = file.name();
      music_files[count].filepath = "/" + String(file.name());
      music_files[count].title = extractMP3Title(music_files[count].filepath.c_str());
      music_files[count].artist = extractMP3Artist(music_files[count].filepath.c_str());
      music_files[count].duration = 180; // Default 3 minutes
      music_files[count].has_metadata = false;
      
      count++;
    }
    file = root.openNextFile();
  }
  
  root.close();
  
  // Also scan Music folder if it exists
  File musicDir = SD.open("/Music");
  if (musicDir && musicDir.isDirectory()) {
    file = musicDir.openNextFile();
    
    while (file && count < max_files) {
      if (!file.isDirectory() && isAudioFile(file.name())) {
        music_files[count].filename = file.name();
        music_files[count].filepath = "/Music/" + String(file.name());
        music_files[count].title = extractMP3Title(music_files[count].filepath.c_str());
        music_files[count].artist = extractMP3Artist(music_files[count].filepath.c_str());
        music_files[count].duration = 180;
        music_files[count].has_metadata = false;
        
        count++;
      }
      file = musicDir.openNextFile();
    }
    
    musicDir.close();
  }
  
  return count;
}

int scanPDFFiles(PDFFile pdf_files[], int max_files) {
  File root = SD.open("/");
  if (!root) return 0;
  
  int count = 0;
  File file = root.openNextFile();
  
  while (file && count < max_files) {
    if (!file.isDirectory() && isPDFFile(file.name())) {
      pdf_files[count].filename = file.name();
      pdf_files[count].filepath = "/" + String(file.name());
      pdf_files[count].title = file.name();
      pdf_files[count].pages = 1; // Default
      pdf_files[count].file_size = file.size();
      
      count++;
    }
    file = root.openNextFile();
  }
  
  root.close();
  
  // Also scan Documents folder if it exists
  File docsDir = SD.open("/Documents");
  if (docsDir && docsDir.isDirectory()) {
    file = docsDir.openNextFile();
    
    while (file && count < max_files) {
      if (!file.isDirectory() && isPDFFile(file.name())) {
        pdf_files[count].filename = file.name();
        pdf_files[count].filepath = "/Documents/" + String(file.name());
        pdf_files[count].title = file.name();
        pdf_files[count].pages = 1;
        pdf_files[count].file_size = file.size();
        
        count++;
      }
      file = docsDir.openNextFile();
    }
    
    docsDir.close();
  }
  
  return count;
}

String extractMP3Title(const char* filepath) {
  // Simple filename-based title extraction
  String filename = String(filepath);
  int lastSlash = filename.lastIndexOf('/');
  int lastDot = filename.lastIndexOf('.');
  
  if (lastSlash >= 0) {
    filename = filename.substring(lastSlash + 1);
  }
  if (lastDot >= 0) {
    filename = filename.substring(0, lastDot);
  }
  
  // Replace underscores with spaces
  filename.replace("_", " ");
  
  return filename;
}

String extractMP3Artist(const char* filepath) {
  // Default artist - could be enhanced to read ID3 tags
  return "Unknown Artist";
}

FileType getFileType(const String& filename) {
  String ext = getFileExtension(filename);
  ext.toLowerCase();
  
  if (ext == "mp3" || ext == "wav" || ext == "m4a") {
    return FILE_MP3;
  } else if (ext == "pdf") {
    return FILE_PDF;
  } else if (ext == "txt") {
    return FILE_TXT;
  } else if (ext == "jpg" || ext == "jpeg") {
    return FILE_JPG;
  } else if (ext == "png") {
    return FILE_PNG;
  } else if (ext == "cfg" || ext == "ini") {
    return FILE_CONFIG;
  }
  
  return FILE_UNKNOWN;
}

bool isAudioFile(const String& filename) {
  FileType type = getFileType(filename);
  return type == FILE_MP3;
}

bool isPDFFile(const String& filename) {
  FileType type = getFileType(filename);
  return type == FILE_PDF;
}

String getFileExtension(const String& filename) {
  int lastDot = filename.lastIndexOf('.');
  if (lastDot >= 0) {
    return filename.substring(lastDot + 1);
  }
  return "";
}

String formatFileSize(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < 1024 * 1024) {
    return String(bytes / 1024) + " KB";
  } else {
    return String(bytes / (1024 * 1024)) + " MB";
  }
}

void saveSettingsToFile() {
  File settingsFile = SD.open("/settings.cfg", FILE_WRITE);
  if (!settingsFile) return;
  
  settingsFile.println("brightness=" + String(system_state.brightness));
  settingsFile.println("theme=" + String(system_state.current_theme));
  settingsFile.println("step_goal=" + String(system_state.step_goal));
  settingsFile.println("wake_time=" + String(system_state.wake_time));
  settingsFile.println("sleep_time=" + String(system_state.sleep_time));
  
  settingsFile.close();
}

void loadSettingsFromFile() {
  if (!fileExists("/settings.cfg")) return;
  
  String settings = readTextFile("/settings.cfg");
  
  // Parse settings (simple key=value format)
  // This is a simplified parser - a real implementation would be more robust
  if (settings.indexOf("brightness=") >= 0) {
    int start = settings.indexOf("brightness=") + 11;
    int end = settings.indexOf('\n', start);
    if (end < 0) end = settings.length();
    system_state.brightness = settings.substring(start, end).toInt();
  }
  
  // Similar parsing for other settings...
}

void initializeCache() {
  // Initialize file cache system
  // This would typically set up a cache in PSRAM for frequently accessed files
}

void clearCache() {
  // Clear file cache
}