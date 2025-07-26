/*
 * Gamified Quest System Implementation
 * Anime-themed urgent quests for step tracking
 */

#include "quests.h"
#include "sensors.h"
#include "themes.h"
#include "display.h"
#include "ui.h"

// Quest data arrays
QuestData daily_quests[5];
QuestData urgent_quest;
int active_quest_count = 0;
int player_xp = 0;
int player_level = 1;

// Quest templates for each character
QuestData luffy_quest_templates[] = {
  {"Gear Second Rush", "Take 3000 steps to activate Gear Second!", QUEST_STEPS, QUEST_LUFFY, QUEST_EASY, 3000, 0, 150, 0, 0, false, false, false},
  {"Gomu Gomu Training", "Walk 5km like stretching your legs!", QUEST_DISTANCE, QUEST_LUFFY, QUEST_MEDIUM, 5000, 0, 300, 0, 0, false, false, false},
  {"Pirate King's Journey", "Complete 10,000 steps to become Pirate King!", QUEST_STEPS, QUEST_LUFFY, QUEST_HARD, 10000, 0, 500, 0, 0, false, false, true},
  {"Nika's Sun Dance", "Stay active for 60 minutes!", QUEST_ACTIVE_TIME, QUEST_LUFFY, QUEST_MEDIUM, 60, 0, 250, 0, 0, false, false, false}
};

QuestData jinwoo_quest_templates[] = {
  {"Shadow Army March", "Command your army with 2500 steps!", QUEST_STEPS, QUEST_JINWOO, QUEST_EASY, 2500, 0, 200, 0, 0, false, false, false},
  {"Level Up Training", "Gain power with 7500 steps!", QUEST_STEPS, QUEST_JINWOO, QUEST_MEDIUM, 7500, 0, 400, 0, 0, false, false, false},
  {"Monarch's Challenge", "Prove your strength with 12,000 steps!", QUEST_STEPS, QUEST_JINWOO, QUEST_HARD, 12000, 0, 600, 0, 0, false, false, true},
  {"Shadow Extraction", "Hunt for 45 minutes to extract shadows!", QUEST_ACTIVE_TIME, QUEST_JINWOO, QUEST_MEDIUM, 45, 0, 350, 0, 0, false, false, false}
};

QuestData yugo_quest_templates[] = {
  {"Portal Training", "Open portals with 3500 steps!", QUEST_STEPS, QUEST_YUGO, QUEST_EASY, 3500, 0, 175, 0, 0, false, false, false},
  {"Wakfu Gathering", "Collect Wakfu energy with 6km walk!", QUEST_DISTANCE, QUEST_YUGO, QUEST_MEDIUM, 6000, 0, 325, 0, 0, false, false, false},
  {"Brotherhood Quest", "Unite the brotherhood with 15,000 steps!", QUEST_STEPS, QUEST_YUGO, QUEST_LEGENDARY, 15000, 0, 800, 0, 0, false, false, true},
  {"Eliatrope Power", "Channel power for 90 minutes!", QUEST_ACTIVE_TIME, QUEST_YUGO, QUEST_HARD, 90, 0, 450, 0, 0, false, false, false}
};

void initializeQuests() {
  Serial.println("Initializing quest system...");
  
  // Generate initial daily quests
  generateDailyQuests();
  
  // Generate first urgent quest after 30 minutes
  urgent_quest.title = "";
  urgent_quest.urgent = false;
  
  Serial.println("Quest system initialized");
}

void generateDailyQuests() {
  active_quest_count = 0;
  
  // Generate one quest from each character
  daily_quests[0] = generateLuffyQuest();
  daily_quests[1] = generateJinwooQuest();
  daily_quests[2] = generateYugoQuest();
  active_quest_count = 3;
  
  // Mark all as daily quests
  for (int i = 0; i < active_quest_count; i++) {
    daily_quests[i].daily = true;
    daily_quests[i].start_time = millis();
  }
  
  Serial.println("Generated " + String(active_quest_count) + " daily quests");
}

void generateUrgentQuest() {
  // Generate random urgent quest
  int character = random(3);
  
  switch (character) {
    case 0:
      urgent_quest = generateLuffyQuest();
      break;
    case 1:
      urgent_quest = generateJinwooQuest();
      break;
    case 2:
      urgent_quest = generateYugoQuest();
      break;
  }
  
  urgent_quest.urgent = true;
  urgent_quest.daily = false;
  urgent_quest.time_limit = 30 * 60 * 1000; // 30 minutes
  urgent_quest.start_time = millis();
  urgent_quest.reward_points *= 2; // Double rewards for urgent quests
  
  showQuestNotification(urgent_quest);
  
  Serial.println("Generated urgent quest: " + urgent_quest.title);
}

QuestData generateLuffyQuest() {
  int template_index = random(4);
  QuestData quest = luffy_quest_templates[template_index];
  
  // Randomize target slightly
  quest.target_value += random(-500, 500);
  if (quest.target_value < 1000) quest.target_value = 1000;
  
  quest.current_progress = 0;
  quest.completed = false;
  
  return quest;
}

QuestData generateJinwooQuest() {
  int template_index = random(4);
  QuestData quest = jinwoo_quest_templates[template_index];
  
  // Randomize target slightly
  quest.target_value += random(-300, 300);
  if (quest.target_value < 800) quest.target_value = 800;
  
  quest.current_progress = 0;
  quest.completed = false;
  
  return quest;
}

QuestData generateYugoQuest() {
  int template_index = random(4);
  QuestData quest = yugo_quest_templates[template_index];
  
  // Randomize target slightly
  quest.target_value += random(-400, 400);
  if (quest.target_value < 1200) quest.target_value = 1200;
  
  quest.current_progress = 0;
  quest.completed = false;
  
  return quest;
}

void updateQuestProgress() {
  int current_steps = getDailySteps();
  
  // Update daily quests
  for (int i = 0; i < active_quest_count; i++) {
    if (daily_quests[i].completed) continue;
    
    switch (daily_quests[i].type) {
      case QUEST_STEPS:
        daily_quests[i].current_progress = current_steps;
        break;
      case QUEST_DISTANCE:
        // Convert steps to meters (rough approximation)
        daily_quests[i].current_progress = current_steps * 0.7; // ~70cm per step
        break;
      case QUEST_ACTIVE_TIME:
        // This would need to be tracked separately
        break;
    }
    
    // Check if quest completed
    if (daily_quests[i].current_progress >= daily_quests[i].target_value) {
      completeQuest(i);
    }
  }
  
  // Update urgent quest
  if (urgent_quest.urgent && !urgent_quest.completed) {
    switch (urgent_quest.type) {
      case QUEST_STEPS:
        urgent_quest.current_progress = current_steps;
        break;
      case QUEST_DISTANCE:
        urgent_quest.current_progress = current_steps * 0.7;
        break;
    }
    
    if (urgent_quest.current_progress >= urgent_quest.target_value) {
      giveQuestReward(urgent_quest);
      urgent_quest.completed = true;
      showQuestCompleted(urgent_quest);
    }
    
    // Check if urgent quest expired
    if (millis() - urgent_quest.start_time > urgent_quest.time_limit) {
      urgent_quest.urgent = false;
      Serial.println("Urgent quest expired!");
    }
  }
}

bool completeQuest(int quest_id) {
  if (quest_id >= active_quest_count) return false;
  
  daily_quests[quest_id].completed = true;
  giveQuestReward(daily_quests[quest_id]);
  showQuestCompleted(daily_quests[quest_id]);
  
  return true;
}

void giveQuestReward(QuestData& quest) {
  player_xp += quest.reward_points;
  updatePlayerLevel();
  
  Serial.println("Quest completed: " + quest.title);
  Serial.println("Rewarded " + String(quest.reward_points) + " XP");
}

void updatePlayerLevel() {
  int required_xp = player_level * 1000; // 1000 XP per level
  
  if (player_xp >= required_xp) {
    player_level++;
    player_xp -= required_xp;
    Serial.println("LEVEL UP! Now level " + String(player_level));
  }
}

int getPlayerLevel() {
  return player_level;
}

int getPlayerXP() {
  return player_xp;
}

void drawQuestScreen() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Background
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Title
  drawNavigationBar("Urgent Quests", true);
  
  // Player level and XP
  char level_str[30];
  sprintf(level_str, "Level %d - %d XP", player_level, player_xp);
  drawCenteredText(level_str, DISPLAY_WIDTH/2, 50, theme->accent, 1);
  
  int y_offset = 80;
  
  // Draw urgent quest if active
  if (urgent_quest.urgent && !urgent_quest.completed) {
    drawQuestCard(10, y_offset, DISPLAY_WIDTH-20, 80, urgent_quest);
    y_offset += 90;
  }
  
  // Draw daily quests
  for (int i = 0; i < active_quest_count && i < 3; i++) {
    drawQuestCard(10, y_offset, DISPLAY_WIDTH-20, 60, daily_quests[i]);
    y_offset += 70;
  }
  
  updateDisplay();
}

void drawQuestCard(int x, int y, int w, int h, QuestData& quest) {
  ThemeColors* theme = getCurrentTheme();
  
  // Get character colors
  uint16_t char_color;
  switch (quest.character) {
    case QUEST_LUFFY:
      char_color = LUFFY_GOLD;
      break;
    case QUEST_JINWOO:
      char_color = JINWOO_PURPLE;
      break;
    case QUEST_YUGO:
      char_color = YUGO_TEAL;
      break;
    default:
      char_color = theme->accent;
  }
  
  // Card background
  fillRoundRect(x, y, w, h, 8, quest.completed ? theme->secondary : theme->shadow);
  drawRoundRect(x, y, w, h, 8, char_color);
  
  // Title
  drawText(quest.title.c_str(), x + 10, y + 10, char_color, 1);
  
  // Progress bar
  int progress_w = w - 20;
  int progress_x = x + 10;
  int progress_y = y + h - 20;
  
  drawRect(progress_x, progress_y, progress_w, 6, theme->secondary);
  
  float progress = (float)quest.current_progress / quest.target_value;
  if (progress > 1.0) progress = 1.0;
  
  fillRect(progress_x, progress_y, progress_w * progress, 6, char_color);
  
  // Progress text
  char progress_str[50];
  sprintf(progress_str, "%d/%d", quest.current_progress, quest.target_value);
  drawText(progress_str, progress_x, progress_y - 15, theme->text, 1);
  
  // Urgent indicator
  if (quest.urgent) {
    fillCircle(x + w - 15, y + 15, 5, COLOR_RED);
  }
  
  // Completed indicator
  if (quest.completed) {
    fillCircle(x + w - 15, y + 15, 5, COLOR_GREEN);
  }
}

void showQuestNotification(QuestData& quest) {
  // Show popup notification for new quest
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(20, 100, DISPLAY_WIDTH-40, 200, theme->background);
  drawRect(20, 100, DISPLAY_WIDTH-40, 200, theme->accent);
  
  drawCenteredText("NEW URGENT QUEST!", DISPLAY_WIDTH/2, 130, theme->accent, 2);
  drawCenteredText(quest.title.c_str(), DISPLAY_WIDTH/2, 160, theme->text, 1);
  drawCenteredText(quest.description.c_str(), DISPLAY_WIDTH/2, 180, theme->secondary, 1);
  
  char reward_str[30];
  sprintf(reward_str, "Reward: %d XP", quest.reward_points);
  drawCenteredText(reward_str, DISPLAY_WIDTH/2, 220, theme->accent, 1);
  
  drawCenteredText("Tap to continue", DISPLAY_WIDTH/2, 260, theme->secondary, 1);
  
  updateDisplay();
  delay(3000); // Show for 3 seconds
}

void showQuestCompleted(QuestData& quest) {
  // Show completion animation
  ThemeColors* theme = getCurrentTheme();
  
  for (int i = 0; i < 3; i++) {
    clearDisplay();
    drawCenteredText("QUEST COMPLETED!", DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 40, theme->accent, 3);
    drawCenteredText(quest.title.c_str(), DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, theme->text, 2);
    
    char reward_str[30];
    sprintf(reward_str, "+%d XP", quest.reward_points);
    drawCenteredText(reward_str, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 40, theme->accent, 2);
    
    updateDisplay();
    delay(500);
    
    clearDisplay();
    updateDisplay();
    delay(200);
  }
}