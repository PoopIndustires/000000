/*
 * Gamified Quest System for ESP32-S3 Watch
 * Anime-themed urgent quests for step tracking
 */

#ifndef QUESTS_H
#define QUESTS_H

#include "config.h"

// Quest difficulty levels
enum QuestDifficulty {
  QUEST_EASY,
  QUEST_MEDIUM,
  QUEST_HARD,
  QUEST_LEGENDARY
};

// Quest types
enum QuestType {
  QUEST_STEPS,
  QUEST_DISTANCE,
  QUEST_ACTIVE_TIME,
  QUEST_COMBO
};

// Character-specific quests
enum QuestCharacter {
  QUEST_LUFFY,
  QUEST_JINWOO,
  QUEST_YUGO
};

// Enhanced quest structure
struct QuestData {
  String title;
  String description;
  QuestType type;
  QuestCharacter character;
  QuestDifficulty difficulty;
  int target_value;
  int current_progress;
  int reward_points;
  unsigned long time_limit;
  unsigned long start_time;
  bool completed;
  bool urgent;
  bool daily;
};

// Initialize quest system
void initializeQuests();

// Quest management
void generateDailyQuests();
void generateUrgentQuest();
void updateQuestProgress();
bool completeQuest(int quest_id);

// Quest display
void drawQuestScreen();
void drawQuestCard(int x, int y, int w, int h, QuestData& quest);
void showQuestNotification(QuestData& quest);
void showQuestCompleted(QuestData& quest);

// Character-specific quests
QuestData generateLuffyQuest();
QuestData generateJinwooQuest();
QuestData generateYugoQuest();

// Quest rewards
void giveQuestReward(QuestData& quest);
void updatePlayerLevel();
int getPlayerLevel();
int getPlayerXP();

// Quest templates
extern QuestData luffy_quests[];
extern QuestData jinwoo_quests[];
extern QuestData yugo_quests[];

#endif // QUESTS_H