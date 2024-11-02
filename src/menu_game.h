#ifndef MENUGAME_H
#define MENUGAME_H
#include <raylib.h>
#include "level_parser.h"
#include <stdbool.h>

#include "game.h"

void game_menu_init(GameData* gameData, int screenWidth, int screenHeight);
void game_menu_tick(GameData* gameData, int screenWidth, int screenHeight, float dt);
void game_menu_draw(GameData* gameData, Color* gameColors);

#endif