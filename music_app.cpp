/*
 * Music Player Application
 * Auto-detects MP3 files from SD card
 */

#include "apps.h"
#include "filesystem.h"
#include "display.h"
#include "themes.h"
#include "ui.h"

// Music player state
struct MusicPlayerState {
  bool is_playing;
  int current_track;
  int volume;
  int progress_seconds;
  bool shuffle;
  bool repeat;
  String current_title;
  String current_artist;
} music_state;

void initMusicApp() {
  music_state.is_playing = false;
  music_state.current_track = 0;
  music_state.volume = 50;
  music_state.progress_seconds = 0;
  music_state.shuffle = false;
  music_state.repeat = false;
  
  // Scan for MP3 files
  total_music_files = scanMusicFiles(music_files, 100);
  
  if (total_music_files > 0) {
    music_state.current_title = music_files[0].title;
    music_state.current_artist = music_files[0].artist;
  }
}

void drawMusicApp() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Background
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Navigation bar
  drawNavigationBar("Music", true);
  
  // Album art area (placeholder)
  int art_size = 120;
  int art_x = (DISPLAY_WIDTH - art_size) / 2;
  int art_y = 60;
  
  drawRoundRect(art_x, art_y, art_size, art_size, 8, theme->accent);
  fillRoundRect(art_x + 2, art_y + 2, art_size - 4, art_size - 4, 6, theme->primary);
  
  // Music note icon (simple)
  fillCircle(art_x + art_size/2 - 10, art_y + art_size/2, 8, theme->background);
  fillRect(art_x + art_size/2 + 5, art_y + art_size/2 - 15, 3, 20, theme->background);
  
  // Track info
  int info_y = art_y + art_size + 20;
  
  if (total_music_files > 0 && music_state.current_track < total_music_files) {
    drawCenteredText(music_files[music_state.current_track].title.c_str(), 
                    DISPLAY_WIDTH/2, info_y, theme->text, 2);
    drawCenteredText(music_files[music_state.current_track].artist.c_str(), 
                    DISPLAY_WIDTH/2, info_y + 25, theme->secondary, 1);
  } else {
    drawCenteredText("No Music Found", DISPLAY_WIDTH/2, info_y, theme->text, 2);
    drawCenteredText("Add MP3 files to SD card", DISPLAY_WIDTH/2, info_y + 25, theme->secondary, 1);
  }
  
  // Progress bar
  int progress_y = info_y + 60;
  int progress_width = DISPLAY_WIDTH - 40;
  int progress_x = 20;
  
  drawRect(progress_x, progress_y, progress_width, 4, theme->secondary);
  
  if (total_music_files > 0) {
    int progress_pixels = (progress_width * music_state.progress_seconds) / 
                         (music_files[music_state.current_track].duration > 0 ? 
                          music_files[music_state.current_track].duration : 180);
    fillRect(progress_x, progress_y, progress_pixels, 4, theme->accent);
  }
  
  // Time labels
  char current_time[8], total_time[8];
  int current_min = music_state.progress_seconds / 60;
  int current_sec = music_state.progress_seconds % 60;
  sprintf(current_time, "%d:%02d", current_min, current_sec);
  
  int total_duration = total_music_files > 0 ? music_files[music_state.current_track].duration : 0;
  int total_min = total_duration / 60;
  int total_sec = total_duration % 60;
  sprintf(total_time, "%d:%02d", total_min, total_sec);
  
  drawText(current_time, progress_x, progress_y + 10, theme->secondary, 1);
  drawText(total_time, DISPLAY_WIDTH - progress_x - getTextWidth(total_time, 1), 
           progress_y + 10, theme->secondary, 1);
  
  // Control buttons
  int button_y = progress_y + 40;
  int button_size = 40;
  int button_spacing = 60;
  int center_x = DISPLAY_WIDTH / 2;
  
  // Previous button
  int prev_x = center_x - button_spacing - button_size/2;
  drawCircle(prev_x, button_y, button_size/2, theme->secondary);
  drawText("<<", prev_x - 8, button_y - 4, theme->text, 1);
  
  // Play/Pause button
  drawCircle(center_x, button_y, button_size/2, theme->accent);
  if (music_state.is_playing) {
    fillRect(center_x - 6, button_y - 8, 4, 16, theme->background);
    fillRect(center_x + 2, button_y - 8, 4, 16, theme->background);
  } else {
    // Draw play triangle
    for (int i = 0; i < 12; i++) {
      drawLine(center_x - 6 + i/2, button_y - 8 + i, 
               center_x - 6 + i/2, button_y + 8 - i, theme->background);
    }
  }
  
  // Next button
  int next_x = center_x + button_spacing - button_size/2;
  drawCircle(next_x, button_y, button_size/2, theme->secondary);
  drawText(">>", next_x - 8, button_y - 4, theme->text, 1);
  
  // Volume control
  int volume_y = button_y + 60;
  drawText("Volume:", 20, volume_y, theme->secondary, 1);
  
  int volume_bar_x = 80;
  int volume_bar_width = DISPLAY_WIDTH - 100;
  drawRect(volume_bar_x, volume_y + 5, volume_bar_width, 8, theme->secondary);
  fillRect(volume_bar_x, volume_y + 5, 
           (volume_bar_width * music_state.volume) / 100, 8, theme->accent);
  
  // Track list info
  char track_info[50];
  sprintf(track_info, "Track %d of %d", music_state.current_track + 1, total_music_files);
  drawCenteredText(track_info, DISPLAY_WIDTH/2, volume_y + 30, theme->secondary, 1);
  
  updateDisplay();
}

void handleMusicTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  // Handle button touches
  int button_y = 300; // Approximate button Y position
  int center_x = DISPLAY_WIDTH / 2;
  int button_size = 40;
  int button_spacing = 60;
  
  // Check play/pause button
  if (abs(gesture.x - center_x) <= button_size/2 && 
      abs(gesture.y - button_y) <= button_size/2) {
    if (music_state.is_playing) {
      pauseMusic();
    } else {
      playMusic();
    }
    return;
  }
  
  // Check previous button
  int prev_x = center_x - button_spacing - button_size/2;
  if (abs(gesture.x - prev_x) <= button_size/2 && 
      abs(gesture.y - button_y) <= button_size/2) {
    previousTrack();
    return;
  }
  
  // Check next button
  int next_x = center_x + button_spacing - button_size/2;
  if (abs(gesture.x - next_x) <= button_size/2 && 
      abs(gesture.y - button_y) <= button_size/2) {
    nextTrack();
    return;
  }
  
  // Handle volume control
  int volume_y = button_y + 60;
  int volume_bar_x = 80;
  int volume_bar_width = DISPLAY_WIDTH - 100;
  
  if (gesture.x >= volume_bar_x && gesture.x <= volume_bar_x + volume_bar_width &&
      abs(gesture.y - (volume_y + 5)) <= 10) {
    music_state.volume = map(gesture.x - volume_bar_x, 0, volume_bar_width, 0, 100);
    // Update actual volume here
  }
}

void playMusic() {
  if (total_music_files == 0) return;
  
  music_state.is_playing = true;
  // Start actual audio playback here
  Serial.println("Playing: " + music_files[music_state.current_track].title);
}

void pauseMusic() {
  music_state.is_playing = false;
  // Pause actual audio playback here
  Serial.println("Music paused");
}

void nextTrack() {
  if (total_music_files == 0) return;
  
  music_state.current_track++;
  if (music_state.current_track >= total_music_files) {
    music_state.current_track = 0;
  }
  music_state.progress_seconds = 0;
  
  if (music_state.is_playing) {
    playMusic();
  }
}

void previousTrack() {
  if (total_music_files == 0) return;
  
  music_state.current_track--;
  if (music_state.current_track < 0) {
    music_state.current_track = total_music_files - 1;
  }
  music_state.progress_seconds = 0;
  
  if (music_state.is_playing) {
    playMusic();
  }
}