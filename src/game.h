#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "level_parser.h"
#include <stdbool.h>

typedef struct GameData {
	float PlayerPosX;
	float PlayerPosY[2];
	float JumpVelocity[2];
	float JumpAcceleration[2];
	bool GoingUp[2];

	float Timer;
	float TileSize;

	float CameraPosX;
	float CameraSpeed;

	Rectangle DebugRectangles[15];
	Color DebugRectanglesColors[15];
	int DebugRectangleCount;
} GameData;

void game_init(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight);
void game_exit(GameData* gameData);
void game_tick(GameData* gameData, const LevelData* levelData, float dt);
void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors);

void game_restart(GameData* gameData, const LevelData* levelData);

#endif