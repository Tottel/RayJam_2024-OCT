#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "level_parser.h"
#include <stdbool.h>

typedef struct Enemy {
	Vector2 Pos;
	float HitTimer; // Bigger than 0 means hit. And it counts down
	int HP;
	float PosOffsetTimer;
	//bool bobbingUp;
} Enemy;

typedef struct GameData {
	bool NextLevel;

	float PlayerPosX;
	float PlayerPosY[2];
	float JumpVelocity[2]; 
	float JumpAcceleration[2];
	float JumpTimer[2];
	bool GoingUp[2];

	float AnimationTimer[2];
	uint16_t AnimationRectIndex[2]; 

	float Timer;
	float TileSize;

	float CameraPosX;

	float BladeSawTimer;
	int BladeSawRectIndex;

	Enemy Enemies[15];
	uint32_t EnemyCount;

	float BulletFireTimer;
	
	Vector2 BulletPos[20];
	uint32_t BulletCount;

	Rectangle DebugRectangles[15];
	Color DebugRectanglesColors[15];
	int DebugRectangleCount;

	float PortalPosX;
	float PortalPosY[2];
} GameData;

void game_init(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight);
void game_exit(GameData* gameData);
void game_tick(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight, float dt);
void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors);
void game_bladesaws_draw(GameData* gameData, Texture2D bladesaw, float dt);

void game_restart(GameData* gameData, const LevelData* levelData);

#endif