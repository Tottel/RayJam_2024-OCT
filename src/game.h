#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "level_parser.h"
#include <stdbool.h>

#define MAX_ENEMIES 50

typedef struct Enemy {
	int PosX;
	int PosY;
	Vector2 Pos;
	float HitTimer; // Bigger than 0 means hit. And it counts down
	int HP;
	float PosOffsetTimer; 
} Enemy;

typedef struct GameData {
	bool NextLevel;
	bool RestartLevel;

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

	Enemy Enemies[MAX_ENEMIES];
	uint32_t EnemyCount;

	float BulletFireTimer;
	bool GunAtTop;
	Vector2 BulletPos[20];
	uint32_t BulletCount;

	float PortalPosX;
	float PortalPosY[2];

	Texture CharSheet[2];
	int CharFrameCount;

	Texture EnemySheet[2];
	Texture EnemyHitSheet[2];
	int EnemyFrameCount;
	float EnemyAnimationTimer;
	int EnemyAnimationIndex;

	Texture PortalSheet[2];
	int PortalFrameCount;
	float PortalAnimationTimer;
	int PortalAnimationIndex;

	Sound JumpSoundTop[3];
	Sound Portal;
	Sound Respawn;
} GameData;

void game_create(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight);
void game_init(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight);
void game_exit(GameData* gameData);
void game_tick(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight, float dt);
void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors);
void game_bladesaws_draw(GameData* gameData, Texture2D bladesaw, float dt);

void game_restart(GameData* gameData, const LevelData* levelData);

#endif