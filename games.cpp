/*
 * Games Framework Implementation
 * Anime-themed games for ESP32-S3 Watch
 */

#include "games.h"
#include "display.h"
#include "themes.h"
#include "ui.h"
#include <math.h>

// Global game session
GameSession current_game_session;

// Battle creatures database (anime-themed)
BattleCreature battle_creatures[] = {
  // Luffy-themed creatures
  {"Rubber Monkey", "Fighting", 25, 120, 120, 85, 65, 75, {"Gum-Gum Punch", "Gear Second", "Red Hawk", "King Kong Gun"}, {70, 90, 110, 150}, false, LUFFY_GOLD},
  {"Sea King", "Water", 30, 180, 180, 95, 80, 45, {"Tidal Wave", "Bite", "Whirlpool", "Tsunami"}, {80, 60, 85, 140}, false, COLOR_BLUE},
  {"Fire Fist", "Fire", 28, 140, 140, 105, 70, 85, {"Flame Punch", "Fire Wall", "Inferno", "Phoenix Fire"}, {75, 65, 100, 130}, false, COLOR_RED},
  
  // Jin Woo-themed creatures  
  {"Shadow Soldier", "Dark", 27, 100, 100, 90, 85, 95, {"Shadow Strike", "Dark Blade", "Shadow Army", "Arise"}, {65, 80, 120, 160}, false, JINWOO_PURPLE},
  {"Iron Golem", "Steel", 32, 200, 200, 110, 120, 35, {"Metal Punch", "Iron Defense", "Steel Slam", "Earthquake"}, {85, 50, 105, 120}, false, JINWOO_SILVER},
  {"Void Beast", "Psychic", 29, 130, 130, 100, 75, 90, {"Mind Crush", "Teleport", "Void Blast", "Shadow Portal"}, {90, 40, 115, 145}, false, JINWOO_VIOLET},
  
  // Yugo-themed creatures
  {"Portal Guardian", "Magic", 26, 110, 110, 80, 90, 100, {"Portal Punch", "Wakfu Blast", "Teleport", "Energy Beam"}, {70, 85, 45, 125}, false, YUGO_TEAL},
  {"Eliatrope Dragon", "Dragon", 35, 160, 160, 120, 95, 80, {"Dragon Claw", "Portal Breath", "Time Stop", "Ultimate Blast"}, {95, 110, 100, 170}, false, YUGO_ENERGY},
  {"Tofu Bird", "Flying", 20, 80, 80, 60, 50, 110, {"Quick Attack", "Wing Flap", "Tornado", "Aerial Ace"}, {55, 40, 75, 90}, false, YUGO_LIGHT_BLUE}
};

int num_battle_creatures = sizeof(battle_creatures) / sizeof(battle_creatures[0]);

void initializeGames() {
  Serial.println("Initializing games system...");
  
  // Initialize game session
  current_game_session.current_game = GAME_BATTLE_ARENA;
  current_game_session.state = GAME_MENU;
  current_game_session.score = 0;
  current_game_session.level = 1;
  current_game_session.lives = 3;
  current_game_session.sound_enabled = true;
  current_game_session.selected_move = 0;
  current_game_session.log_count = 0;
  
  // Load saved progress
  loadGameProgress();
  
  Serial.println("Games system initialized");
}

void launchGame(GameType game) {
  current_game_session.current_game = game;
  current_game_session.start_time = millis();
  
  switch (game) {
    case GAME_BATTLE_ARENA:
      initBattleArena();
      break;
    case GAME_SHADOW_DUNGEON:
      initShadowDungeon();
      break;
    case GAME_PIRATE_ADVENTURE:
      initPirateAdventure();
      break;
    case GAME_WAKFU_QUEST:
      initWakfuQuest();
      break;
    case GAME_MINI_SNAKE:
      initSnakeGame();
      break;
    case GAME_MEMORY_MATCH:
      initMemoryGame();
      break;
  }
}

void drawGameMenu() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  // Background
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  // Title
  drawNavigationBar("Games Arena", true);
  
  // Game buttons in grid layout
  int button_w = (DISPLAY_WIDTH - 30) / 2;
  int button_h = 50;
  int start_y = 80;
  
  // Row 1
  drawGameButton(10, start_y, button_w, button_h, "Battle Arena", current_game_session.current_game == GAME_BATTLE_ARENA);
  drawGameButton(20 + button_w, start_y, button_w, button_h, "Shadow Dungeon", current_game_session.current_game == GAME_SHADOW_DUNGEON);
  
  // Row 2  
  drawGameButton(10, start_y + 60, button_w, button_h, "Pirate Adventure", current_game_session.current_game == GAME_PIRATE_ADVENTURE);
  drawGameButton(20 + button_w, start_y + 60, button_w, button_h, "Wakfu Quest", current_game_session.current_game == GAME_WAKFU_QUEST);
  
  // Row 3
  drawGameButton(10, start_y + 120, button_w, button_h, "Snake Game", current_game_session.current_game == GAME_MINI_SNAKE);
  drawGameButton(20 + button_w, start_y + 120, button_w, button_h, "Memory Match", current_game_session.current_game == GAME_MEMORY_MATCH);
  
  // Game stats
  char stats_str[100];
  sprintf(stats_str, "Level %d | Score: %d | Games Played: %d", 
          current_game_session.level, current_game_session.score, current_game_session.level * 5);
  drawCenteredText(stats_str, DISPLAY_WIDTH/2, start_y + 200, theme->secondary, 1);
  
  // Instructions
  drawCenteredText("Tap game to play", DISPLAY_WIDTH/2, start_y + 230, theme->secondary, 1);
  drawCenteredText("Long press for info", DISPLAY_WIDTH/2, start_y + 250, theme->secondary, 1);
  
  updateDisplay();
}

void handleGameMenuTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  int button_w = (DISPLAY_WIDTH - 30) / 2;
  int button_h = 50;
  int start_y = 80;
  
  // Check which game button was tapped
  if (gesture.y >= start_y && gesture.y <= start_y + button_h) {
    if (gesture.x >= 10 && gesture.x <= 10 + button_w) {
      launchGame(GAME_BATTLE_ARENA);
    } else if (gesture.x >= 20 + button_w && gesture.x <= 20 + 2 * button_w) {
      launchGame(GAME_SHADOW_DUNGEON);
    }
  } else if (gesture.y >= start_y + 60 && gesture.y <= start_y + 60 + button_h) {
    if (gesture.x >= 10 && gesture.x <= 10 + button_w) {
      launchGame(GAME_PIRATE_ADVENTURE);
    } else if (gesture.x >= 20 + button_w && gesture.x <= 20 + 2 * button_w) {
      launchGame(GAME_WAKFU_QUEST);
    }
  } else if (gesture.y >= start_y + 120 && gesture.y <= start_y + 120 + button_h) {
    if (gesture.x >= 10 && gesture.x <= 10 + button_w) {
      launchGame(GAME_MINI_SNAKE);
    } else if (gesture.x >= 20 + button_w && gesture.x <= 20 + 2 * button_w) {
      launchGame(GAME_MEMORY_MATCH);
    }
  }
}

// ==================== BATTLE ARENA GAME ====================
void initBattleArena() {
  current_game_session.state = GAME_BATTLE_SELECT;
  current_game_session.selected_move = 0;
  current_game_session.log_count = 0;
  
  // Create player creature based on current theme
  switch (system_state.current_theme) {
    case THEME_LUFFY_GEAR5:
      current_game_session.player_creature = battle_creatures[0]; // Rubber Monkey
      break;
    case THEME_SUNG_JINWOO:
      current_game_session.player_creature = battle_creatures[3]; // Shadow Soldier
      break;
    case THEME_YUGO_WAKFU:
      current_game_session.player_creature = battle_creatures[6]; // Portal Guardian
      break;
  }
  
  current_game_session.player_creature.is_player = true;
  generateRandomEnemy();
  addBattleLog("A wild " + current_game_session.enemy_creature.name + " appeared!");
}

void drawBattleArena() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  
  if (current_game_session.state == GAME_BATTLE_SELECT) {
    // Draw creature selection screen
    drawCenteredText("Choose Your Fighter!", DISPLAY_WIDTH/2, 50, theme->accent, 2);
    
    // Show available creatures based on theme
    int start_index = 0;
    switch (system_state.current_theme) {
      case THEME_LUFFY_GEAR5: start_index = 0; break;
      case THEME_SUNG_JINWOO: start_index = 3; break;
      case THEME_YUGO_WAKFU: start_index = 6; break;
    }
    
    for (int i = 0; i < 3; i++) {
      BattleCreature& creature = battle_creatures[start_index + i];
      int y = 100 + i * 60;
      
      drawGameButton(20, y, DISPLAY_WIDTH - 40, 50, creature.name + " Lvl." + String(creature.level), false);
      
      // Stats preview
      char stats[50];
      sprintf(stats, "HP:%d ATK:%d DEF:%d SPD:%d", creature.max_hp, creature.attack, creature.defense, creature.speed);
      drawText(stats, 25, y + 30, theme->secondary, 1);
    }
    
    drawCenteredText("Tap to select fighter", DISPLAY_WIDTH/2, 400, theme->secondary, 1);
    
  } else if (current_game_session.state == GAME_BATTLE_FIGHT) {
    // Draw battle screen
    
    // Enemy creature (top)
    fillRoundRect(20, 20, DISPLAY_WIDTH - 40, 80, 8, theme->shadow);
    drawRoundRect(20, 20, DISPLAY_WIDTH - 40, 80, 8, current_game_session.enemy_creature.color);
    
    drawText(current_game_session.enemy_creature.name, 30, 35, current_game_session.enemy_creature.color, 1);
    drawText("Lvl." + String(current_game_session.enemy_creature.level), 30, 50, theme->secondary, 1);
    
    // Enemy health bar
    drawHealthBar(30, 70, DISPLAY_WIDTH - 80, 15, 
                 current_game_session.enemy_creature.hp, 
                 current_game_session.enemy_creature.max_hp, 
                 COLOR_RED);
    
    // Player creature (bottom)
    fillRoundRect(20, 120, DISPLAY_WIDTH - 40, 80, 8, theme->shadow);
    drawRoundRect(20, 120, DISPLAY_WIDTH - 40, 80, 8, current_game_session.player_creature.color);
    
    drawText(current_game_session.player_creature.name, 30, 135, current_game_session.player_creature.color, 1);
    drawText("Lvl." + String(current_game_session.player_creature.level), 30, 150, theme->secondary, 1);
    
    // Player health bar
    drawHealthBar(30, 170, DISPLAY_WIDTH - 80, 15, 
                 current_game_session.player_creature.hp, 
                 current_game_session.player_creature.max_hp, 
                 COLOR_GREEN);
    
    // Battle log
    int log_y = 210;
    for (int i = 0; i < current_game_session.log_count && i < 3; i++) {
      drawText(current_game_session.battle_log[i].c_str(), 10, log_y + i * 15, theme->text, 1);
    }
    
    // Move buttons
    int move_y = 280;
    for (int i = 0; i < 4; i++) {
      if (current_game_session.player_creature.moves[i] != "") {
        int x = (i % 2) * (DISPLAY_WIDTH / 2) + 10;
        int y = move_y + (i / 2) * 35;
        
        drawGameButton(x, y, DISPLAY_WIDTH / 2 - 20, 30, 
                      current_game_session.player_creature.moves[i], 
                      i == current_game_session.selected_move);
        
        // Power indicator
        char power_str[10];
        sprintf(power_str, "PWR:%d", current_game_session.player_creature.move_power[i]);
        drawText(power_str, x + 5, y + 20, theme->secondary, 1);
      }
    }
    
  } else if (current_game_session.state == GAME_BATTLE_RESULT) {
    // Draw battle result
    String result_text = (current_game_session.player_creature.hp > 0) ? "VICTORY!" : "DEFEAT!";
    uint16_t result_color = (current_game_session.player_creature.hp > 0) ? COLOR_GREEN : COLOR_RED;
    
    drawCenteredText(result_text, DISPLAY_WIDTH/2, 150, result_color, 3);
    
    if (current_game_session.player_creature.hp > 0) {
      drawCenteredText("You defeated " + current_game_session.enemy_creature.name + "!", 
                      DISPLAY_WIDTH/2, 200, theme->text, 1);
      drawCenteredText("Gained 100 XP!", DISPLAY_WIDTH/2, 220, theme->accent, 1);
    } else {
      drawCenteredText(current_game_session.enemy_creature.name + " wins!", 
                      DISPLAY_WIDTH/2, 200, theme->text, 1);
      drawCenteredText("Better luck next time!", DISPLAY_WIDTH/2, 220, theme->secondary, 1);
    }
    
    drawGameButton(50, 300, DISPLAY_WIDTH - 100, 40, "Battle Again", false);
    drawGameButton(50, 350, DISPLAY_WIDTH - 100, 40, "Back to Menu", false);
  }
  
  updateDisplay();
}

void handleBattleTouch(TouchGesture& gesture) {
  if (gesture.event != TOUCH_TAP) return;
  
  if (current_game_session.state == GAME_BATTLE_SELECT) {
    // Handle creature selection
    int start_index = 0;
    switch (system_state.current_theme) {
      case THEME_LUFFY_GEAR5: start_index = 0; break;
      case THEME_SUNG_JINWOO: start_index = 3; break;
      case THEME_YUGO_WAKFU: start_index = 6; break;
    }
    
    for (int i = 0; i < 3; i++) {
      int y = 100 + i * 60;
      if (gesture.y >= y && gesture.y <= y + 50) {
        current_game_session.player_creature = battle_creatures[start_index + i];
        current_game_session.player_creature.is_player = true;
        current_game_session.state = GAME_BATTLE_FIGHT;
        addBattleLog("Go, " + current_game_session.player_creature.name + "!");
        break;
      }
    }
    
  } else if (current_game_session.state == GAME_BATTLE_FIGHT) {
    // Handle move selection
    int move_y = 280;
    for (int i = 0; i < 4; i++) {
      if (current_game_session.player_creature.moves[i] != "") {
        int x = (i % 2) * (DISPLAY_WIDTH / 2) + 10;
        int y = move_y + (i / 2) * 35;
        
        if (gesture.x >= x && gesture.x <= x + DISPLAY_WIDTH / 2 - 20 &&
            gesture.y >= y && gesture.y <= y + 30) {
          selectMove(i);
          break;
        }
      }
    }
    
  } else if (current_game_session.state == GAME_BATTLE_RESULT) {
    // Handle result screen
    if (gesture.y >= 300 && gesture.y <= 340) {
      // Battle Again
      generateRandomEnemy();
      current_game_session.player_creature.hp = current_game_session.player_creature.max_hp;
      current_game_session.state = GAME_BATTLE_FIGHT;
      current_game_session.log_count = 0;
      addBattleLog("A new challenger approaches!");
      
    } else if (gesture.y >= 350 && gesture.y <= 390) {
      // Back to Menu
      current_game_session.state = GAME_MENU;
    }
  }
}

void selectMove(int move_index) {
  current_game_session.selected_move = move_index;
  executeBattleTurn();
}

void executeBattleTurn() {
  BattleCreature& player = current_game_session.player_creature;
  BattleCreature& enemy = current_game_session.enemy_creature;
  
  // Determine turn order based on speed
  bool player_first = player.speed >= enemy.speed;
  
  if (player_first) {
    // Player attacks first
    dealDamage(player, enemy, current_game_session.selected_move);
    if (enemy.hp > 0) {
      // Enemy counter-attacks
      int enemy_move = random(4);
      dealDamage(enemy, player, enemy_move);
    }
  } else {
    // Enemy attacks first
    int enemy_move = random(4);
    dealDamage(enemy, player, enemy_move);
    if (player.hp > 0) {
      // Player counter-attacks
      dealDamage(player, enemy, current_game_session.selected_move);
    }
  }
  
  checkBattleEnd();
}

void dealDamage(BattleCreature& attacker, BattleCreature& defender, int move_index) {
  if (move_index >= 4 || attacker.moves[move_index] == "") return;
  
  int damage = calculateDamage(attacker, defender, attacker.move_power[move_index]);
  defender.hp -= damage;
  if (defender.hp < 0) defender.hp = 0;
  
  String message = attacker.name + " used " + attacker.moves[move_index] + "!";
  addBattleLog(message);
  
  if (damage > 0) {
    message = "Dealt " + String(damage) + " damage!";
    addBattleLog(message);
  }
}

int calculateDamage(BattleCreature& attacker, BattleCreature& defender, int move_power) {
  // Simple damage calculation with some randomness
  float attack_ratio = (float)attacker.attack / defender.defense;
  float base_damage = move_power * attack_ratio * (attacker.level / 100.0 + 1.0);
  
  // Add some randomness (85-100% of calculated damage)
  float damage = base_damage * (0.85 + (random(16) / 100.0));
  
  return (int)damage;
}

void addBattleLog(String message) {
  if (current_game_session.log_count < 5) {
    current_game_session.battle_log[current_game_session.log_count] = message;
    current_game_session.log_count++;
  } else {
    // Shift messages up
    for (int i = 0; i < 4; i++) {
      current_game_session.battle_log[i] = current_game_session.battle_log[i + 1];
    }
    current_game_session.battle_log[4] = message;
  }
}

void checkBattleEnd() {
  if (current_game_session.player_creature.hp <= 0 || current_game_session.enemy_creature.hp <= 0) {
    current_game_session.state = GAME_BATTLE_RESULT;
    
    if (current_game_session.player_creature.hp > 0) {
      current_game_session.score += 100;
      addBattleLog("Victory!");
    } else {
      addBattleLog("Defeat!");
    }
  }
}

void generateRandomEnemy() {
  int enemy_index = random(num_battle_creatures);
  current_game_session.enemy_creature = battle_creatures[enemy_index];
  current_game_session.enemy_creature.is_player = false;
  
  // Slight level randomization
  current_game_session.enemy_creature.level += random(-3, 4);
  if (current_game_session.enemy_creature.level < 1) {
    current_game_session.enemy_creature.level = 1;
  }
}

// ==================== UTILITY FUNCTIONS ====================
void drawGameButton(int x, int y, int w, int h, String text, bool selected) {
  ThemeColors* theme = getCurrentTheme();
  
  uint16_t bg_color = selected ? theme->accent : theme->shadow;
  uint16_t border_color = selected ? theme->primary : theme->secondary;
  uint16_t text_color = selected ? theme->background : theme->text;
  
  fillRoundRect(x, y, w, h, 6, bg_color);
  drawRoundRect(x, y, w, h, 6, border_color);
  
  // Center text
  int text_x = x + (w - getTextWidth(text.c_str(), 1)) / 2;
  int text_y = y + (h - 8) / 2;
  drawText(text.c_str(), text_x, text_y, text_color, 1);
}

void drawHealthBar(int x, int y, int w, int h, int current_hp, int max_hp, uint16_t color) {
  ThemeColors* theme = getCurrentTheme();
  
  // Background
  fillRect(x, y, w, h, theme->shadow);
  drawRect(x, y, w, h, theme->secondary);
  
  // Health fill
  if (max_hp > 0) {
    int fill_w = (w - 2) * current_hp / max_hp;
    fillRect(x + 1, y + 1, fill_w, h - 2, color);
  }
  
  // HP text
  char hp_text[20];
  sprintf(hp_text, "%d/%d", current_hp, max_hp);
  int text_x = x + (w - getTextWidth(hp_text, 1)) / 2;
  drawText(hp_text, text_x, y + 2, COLOR_WHITE, 1);
}

void saveGameProgress() {
  // Save game progress to file system
  // Implementation would save scores, levels, unlocked features
}

void loadGameProgress() {
  // Load game progress from file system
  // Implementation would load saved data
}

// Placeholder implementations for other games
void initShadowDungeon() {
  current_game_session.state = GAME_PLAYING;
  current_game_session.player_x = 5;
  current_game_session.player_y = 5;
  current_game_session.map_level = 1;
  current_game_session.items_collected = 0;
}

void drawShadowDungeon() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_BLACK);
  drawNavigationBar("Shadow Dungeon", true);
  
  drawCenteredText("Jin Woo's Shadow Realm", DISPLAY_WIDTH/2, 100, JINWOO_PURPLE, 2);
  drawCenteredText("Level " + String(current_game_session.map_level), DISPLAY_WIDTH/2, 130, theme->text, 1);
  
  // Simple dungeon map representation
  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 8; y++) {
      int px = 50 + x * 25;
      int py = 160 + y * 25;
      
      if (x == current_game_session.player_x && y == current_game_session.player_y) {
        fillRect(px, py, 20, 20, JINWOO_PURPLE); // Player
      } else if (random(10) > 7) {
        fillRect(px, py, 20, 20, JINWOO_DARK); // Shadow
      } else {
        drawRect(px, py, 20, 20, theme->secondary); // Empty
      }
    }
  }
  
  drawCenteredText("Move with swipes", DISPLAY_WIDTH/2, 380, theme->secondary, 1);
  drawCenteredText("Shadows Defeated: " + String(current_game_session.items_collected), DISPLAY_WIDTH/2, 400, theme->text, 1);
  
  updateDisplay();
}

void handleDungeonTouch(TouchGesture& gesture) {
  switch (gesture.event) {
    case TOUCH_SWIPE_UP:
      movePlayer(0, -1);
      break;
    case TOUCH_SWIPE_DOWN:
      movePlayer(0, 1);
      break;
    case TOUCH_SWIPE_LEFT:
      movePlayer(-1, 0);
      break;
    case TOUCH_SWIPE_RIGHT:
      movePlayer(1, 0);
      break;
  }
}

void movePlayer(int dx, int dy) {
  current_game_session.player_x += dx;
  current_game_session.player_y += dy;
  
  // Boundary check
  if (current_game_session.player_x < 0) current_game_session.player_x = 0;
  if (current_game_session.player_x > 9) current_game_session.player_x = 9;
  if (current_game_session.player_y < 0) current_game_session.player_y = 0;
  if (current_game_session.player_y > 7) current_game_session.player_y = 7;
  
  // Random shadow encounter
  if (random(5) == 0) {
    current_game_session.items_collected++;
    addBattleLog("Shadow defeated!");
  }
}

// Placeholder implementations for other games (Snake, Memory, etc.)
void initSnakeGame() {
  current_game_session.state = GAME_PLAYING;
  current_game_session.snake_length = 3;
  current_game_session.direction = 0; // 0=up, 1=right, 2=down, 3=left
  current_game_session.score = 0;
  
  // Initialize snake position
  for (int i = 0; i < current_game_session.snake_length; i++) {
    current_game_session.snake_x[i] = 10;
    current_game_session.snake_y[i] = 10 + i;
  }
  
  generateFood();
}

void drawSnakeGame() {
  clearDisplay();
  ThemeColors* theme = getCurrentTheme();
  
  fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, theme->background);
  drawNavigationBar("Snake Game", true);
  
  // Game area
  int game_size = 20;
  int offset_x = 20;
  int offset_y = 60;
  
  // Draw snake
  for (int i = 0; i < current_game_session.snake_length; i++) {
    int x = offset_x + current_game_session.snake_x[i] * game_size;
    int y = offset_y + current_game_session.snake_y[i] * game_size;
    
    uint16_t snake_color = (i == 0) ? theme->accent : theme->primary;
    fillRect(x, y, game_size - 2, game_size - 2, snake_color);
  }
  
  // Draw food
  int food_x = offset_x + current_game_session.food_x * game_size;
  int food_y = offset_y + current_game_session.food_y * game_size;
  fillRect(food_x, food_y, game_size - 2, game_size - 2, COLOR_RED);
  
  // Score
  drawText("Score: " + String(current_game_session.score), 20, 400, theme->text, 1);
  drawText("Swipe to change direction", 20, 420, theme->secondary, 1);
  
  updateDisplay();
}

void handleSnakeTouch(TouchGesture& gesture) {
  switch (gesture.event) {
    case TOUCH_SWIPE_UP:
      if (current_game_session.direction != 2) current_game_session.direction = 0;
      break;
    case TOUCH_SWIPE_RIGHT:
      if (current_game_session.direction != 3) current_game_session.direction = 1;
      break;
    case TOUCH_SWIPE_DOWN:
      if (current_game_session.direction != 0) current_game_session.direction = 2;
      break;
    case TOUCH_SWIPE_LEFT:
      if (current_game_session.direction != 1) current_game_session.direction = 3;
      break;
  }
}

void generateFood() {
  current_game_session.food_x = random(15);
  current_game_session.food_y = random(12);
}

// Stub implementations for remaining games
void initPirateAdventure() { current_game_session.state = GAME_PLAYING; }
void drawPirateAdventure() { /* Implementation */ }
void handleAdventureTouch(TouchGesture& gesture) { /* Implementation */ }

void initWakfuQuest() { current_game_session.state = GAME_PLAYING; }
void drawWakfuQuest() { /* Implementation */ }
void handleWakfuTouch(TouchGesture& gesture) { /* Implementation */ }

void initMemoryGame() { current_game_session.state = GAME_PLAYING; }
void drawMemoryGame() { /* Implementation */ }
void handleMemoryTouch(TouchGesture& gesture) { /* Implementation */ }