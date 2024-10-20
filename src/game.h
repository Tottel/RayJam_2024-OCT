#ifndef GAME_H
#define GAME_H
#include <raylib.h>

typedef struct GameData {
	int ActiveColor;
	float Timer;
} GameData;

void game_init(GameData* game);
void game_exit(GameData* game);
void game_tick(GameData* gameData, float dt);
void game_draw(RenderTexture2D renderTarget, GameData* gameData, Color* gameColors, int screenWidth, int screenHeight);

#endif