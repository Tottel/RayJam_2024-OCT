#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "level_parser.h"

typedef struct GameData {
	float PlayerPosX;
	float PlayerPosY[2];
	float JumpVelocity[2];
	float JumpAcceleration[2];

	float Timer;
	float TileSize;

	Rectangle DebugRectangles[10];
	int DebugRectangleCount;
} GameData;

void game_init(GameData* game, const LevelData* levelData, int screenWidth, int screenHeight);
void game_exit(GameData* game);
void game_tick(GameData* gameData, const LevelData* levelData, float dt);
void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors);

#endif