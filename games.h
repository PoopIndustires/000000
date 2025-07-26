/*
 * Games Framework for ESP32-S3 Watch
 * Text-based games including Pokemon Showdown style battles
 */

#ifndef GAMES_H
#define GAMES_H

#include "config.h"
#include "touch.h"

// Game types
enum GameType {
  GAME_BATTLE_ARENA,    // Pokemon Showdown style
  GAME_SHADOW_DUNGEON,  // Jin Woo themed RPG
  GAME_PIRATE_ADVENTURE, // Luffy themed adventure
  GAME_WAKFU_QUEST,     // Yugo themed puzzle
  GAME_MINI_SNAKE,      // Classic snake game
  GAME_MEMORY_MATCH     // Memory matching game
};

// Game states
enum GameState {
  GAME_MENU,
  GAME_PLAYING,
  GAME_PAUSED,
  GAME_OVER,
  GAME_WIN,
  GAME_BATTLE_SELECT,
  GAME_BATTLE_FIGHT,
  GAME_BATTLE_RESULT
};

// Battle system structures
struct BattleCreature {
  String name;
  String type;
  int level;
  int hp;
  int max_hp;
  int attack;
  int defense;
  int speed;
  String moves[4];
  int move_power[4];
  bool is_player;
  uint16_t color;
};

// Move structure
struct BattleMove {
  String name;
  String type;
  int power;
  int accuracy;
  String effect;
  int pp;
  int max_pp;
};

// Game session structure
struct GameSession {
  GameType current_game;
  GameState state;
  int score;
  int level;
  int lives;
  unsigned long start_time;
  bool sound_enabled;
  
  // Battle specific
  BattleCreature player_creature;
  BattleCreature enemy_creature;
  int selected_move;
  String battle_log[5];
  int log_count;
  
  // Adventure specific
  int player_x, player_y;
  int map_level;
  int items_collected;
  
  // Mini-game specific
  int snake_x[100], snake_y[100];
  int snake_length;
  int food_x, food_y;
  int direction;
};

// Initialize games system
void initializeGames();

// Game launcher
void launchGame(GameType game);
void drawGameMenu();
void handleGameMenuTouch(TouchGesture& gesture);

// Battle Arena Game (Pokemon Showdown style)
void initBattleArena();
void drawBattleArena();
void handleBattleTouch(TouchGesture& gesture);
void startBattle();
void selectMove(int move_index);
void executeBattleTurn();
void dealDamage(BattleCreature& attacker, BattleCreature& defender, int move_index);
void addBattleLog(String message);
void checkBattleEnd();
void generateRandomEnemy();

// Shadow Dungeon Game (Jin Woo themed)
void initShadowDungeon();
void drawShadowDungeon();
void handleDungeonTouch(TouchGesture& gesture);
void movePlayer(int dx, int dy);
void fightShadowMonster();
void levelUpPlayer();

// Pirate Adventure Game (Luffy themed)
void initPirateAdventure();
void drawPirateAdventure();
void handleAdventureTouch(TouchGesture& gesture);
void exploreIsland();
void findTreasure();

// Wakfu Quest Game (Yugo themed)
void initWakfuQuest();
void drawWakfuQuest();
void handleWakfuTouch(TouchGesture& gesture);
void solvePuzzle();
void openPortal();

// Mini Snake Game
void initSnakeGame();
void drawSnakeGame();
void handleSnakeTouch(TouchGesture& gesture);
void updateSnake();
void generateFood();
void checkSnakeCollision();

// Memory Match Game
void initMemoryGame();
void drawMemoryGame();
void handleMemoryTouch(TouchGesture& gesture);
void flipCard(int x, int y);
void checkMatch();

// Creature database for battles
extern BattleCreature battle_creatures[];
extern int num_battle_creatures;

// Game utilities
void drawGameButton(int x, int y, int w, int h, String text, bool selected);
void drawHealthBar(int x, int y, int w, int h, int current_hp, int max_hp, uint16_t color);
void drawProgressBar(int x, int y, int w, int h, float progress, uint16_t color);
void showGameNotification(String title, String message);
int calculateDamage(BattleCreature& attacker, BattleCreature& defender, int move_power);
String getTypeEffectiveness(String attack_type, String defend_type);

// Save/Load game progress
void saveGameProgress();
void loadGameProgress();

// Global game session
extern GameSession current_game_session;

#endif // GAMES_H