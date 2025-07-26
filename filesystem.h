/*
 * File System Management for ESP32-S3 Watch
 * MicroSD card integration for music, PDFs, and data
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "config.h"
#include <SD.h>
#include <FS.h>

// File types
enum FileType {
  FILE_UNKNOWN,
  FILE_MP3,
  FILE_PDF,
  FILE_TXT,
  FILE_JPG,
  FILE_PNG,
  FILE_CONFIG
};

// File info structure
struct FileInfo {
  String filename;
  String filepath;
  FileType type;
  size_t size;
  time_t modified;
  bool is_directory;
};

// Music file structure
struct MusicFile {
  String filename;
  String filepath;
  String title;
  String artist;
  String album;
  int duration;
  bool has_metadata;
};

// PDF file structure  
struct PDFFile {
  String filename;
  String filepath;
  String title;
  int pages;
  size_t file_size;
};

// Initialize file system
bool initializeFileSystem();

// File operations
bool fileExists(const char* path);
size_t getFileSize(const char* path);
String readTextFile(const char* path);
bool writeTextFile(const char* path, const String& content);
bool deleteFile(const char* path);

// Directory operations
bool listDirectory(const char* path, FileInfo files[], int max_files, int& count);
bool createDirectory(const char* path);
bool removeDirectory(const char* path);

// Music file management
int scanMusicFiles(MusicFile music_files[], int max_files);
bool loadMusicMetadata(const char* filepath, MusicFile& music);
String extractMP3Title(const char* filepath);
String extractMP3Artist(const char* filepath);

// PDF file management
int scanPDFFiles(PDFFile pdf_files[], int max_files);
bool loadPDFInfo(const char* filepath, PDFFile& pdf);

// Settings file management
void saveSettingsToFile();
void loadSettingsFromFile();
void saveUserPreferences();
void loadUserPreferences();

// Cache management
void initializeCache();
void clearCache();
bool cacheFile(const char* filepath, uint8_t* buffer, size_t size);
bool getCachedFile(const char* filepath, uint8_t* buffer, size_t& size);

// File type detection
FileType getFileType(const String& filename);
bool isAudioFile(const String& filename);
bool isPDFFile(const String& filename);
bool isImageFile(const String& filename);

// Utility functions
String getFileExtension(const String& filename);
String formatFileSize(size_t bytes);
String formatFilePath(const String& path);

// File browser functionality
void drawFileBrowser(const char* current_path);
void handleFileBrowserNavigation(TouchGesture& gesture, const char* current_path);

// Global file arrays (in PSRAM)
extern MusicFile* music_files;
extern PDFFile* pdf_files;
extern int total_music_files;
extern int total_pdf_files;

#endif // FILESYSTEM_H