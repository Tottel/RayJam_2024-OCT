#include "game.h"
#include "utils.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

void game_init(GameData* game, const LevelData* levelData, int screenWidth, int screenHeight) {
    const float cubeSize = screenHeight / (float)levelData->LevelHeight;

    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case TILE_SPAWN_1:
                game->PlayerPosX = x * cubeSize;
                game->PlayerPosY[0] = y * cubeSize;
                break;
            case TILE_SPAWN_2:
                game->PlayerPosX = x * cubeSize; // Should be the same as before..
                game->PlayerPosY[1] = y * cubeSize;
                break;
            default:
                break;
            }
        }
    }
}

void game_exit(GameData* gameData) {

}

void game_tick(GameData* gameData, const LevelData* levelData, float dt) {
    gameData->Timer += dt;
}

void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors, int screenWidth, int screenHeight) {
    const float cubeSize = screenHeight / (float)levelData->LevelHeight;

    // Draw level
    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case TILE_VOID:
                break;
            case TILE_FLOOR:
                DrawRectangle(x * cubeSize, y * cubeSize, cubeSize, cubeSize, gameColors[0]);
                break;
            default:
                break;
            }  
        }
    }

    // Draw char 1
    DrawRectangle(gameData->PlayerPosX, gameData->PlayerPosY[0], cubeSize, cubeSize, gameColors[1]);

    // Draw char 2
    DrawRectangle(gameData->PlayerPosX, gameData->PlayerPosY[1], cubeSize, cubeSize, gameColors[2]);

}