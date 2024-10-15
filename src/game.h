#ifndef GAME_H
#define GAME_H
#include <raylib.h>

typedef struct GameData {
	// TODO this should not be a pointer, but an actual array in the game data
	Color* Palette_1; // We know there are 8 colors
} GameData;

void game_init(GameData* game);
void game_clean(GameData* game);
void game_update(GameData* gameData);
void game_draw(RenderTexture2D renderTarget, GameData* gameData, int screenWidth, int screenHeight);

#endif