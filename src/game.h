#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "level_parser.h"

typedef struct GameData {
	int ActiveColor;
	float Timer;
} GameData;

void game_init(GameData* game);
void game_exit(GameData* game);
void game_tick(GameData* gameData, LevelData* levelData, float dt);
void game_draw(GameData* gameData, LevelData* levelData, Color* gameColors, int screenWidth, int screenHeight);

#endif