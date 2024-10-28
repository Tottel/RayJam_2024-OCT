#include "game.h"
#include "utils.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

void game_init(GameData* game, const LevelData* levelData, int screenWidth, int screenHeight) {
    const float tileSize = screenHeight / (float)levelData->LevelHeight;
    game->TileSize = tileSize;

    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case TILE_SPAWN_1:
                game->PlayerPosX = x * tileSize;
                game->PlayerPosY[0] = y * tileSize;
                break;
            case TILE_SPAWN_2:
                game->PlayerPosX = x * tileSize; // Should be the same as before..
                game->PlayerPosY[1] = y * tileSize;
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

    gameData->PlayerPosX += 200.0f * dt;

    bool onGround[2] = { false };
    uint16_t groundLevel[2] = { 0 };
    bool againstWall[2] = { false };
    Rectangle playerRecs[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0], gameData->TileSize, gameData->TileSize },
                                (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1], gameData->TileSize, gameData->TileSize }};

    gameData->DebugRectangleCount = 0;

    // ground CollisionCheck char 1: Check for the 3 adjacing tiles directly underneath us
    for (int offsetX = -1; offsetX < 2; offsetX++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = gameData->PlayerPosY[0] / gameData->TileSize;

        int checkX = charX + offsetX;
        int checkY = charY + 1;

        if(levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize, gameData->TileSize, gameData->TileSize };
        //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
        //gameData->DebugRectangleCount += 1;

        if (CheckCollisionRecs(playerRecs[0], levelRect)) {
            onGround[0] = true;
            groundLevel[0] = checkY;
            break;
        }
    }

    // ground CollisionCheck char 2: Check for the 3 adjacing tiles directly above us
    for (int offsetX = -1; offsetX < 2; offsetX++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[1] + gameData->TileSize*0.5f) / gameData->TileSize;

        int checkX = charX + offsetX;
        int checkY = charY - 1; 

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize, gameData->TileSize, gameData->TileSize };
        //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
        //gameData->DebugRectangleCount += 1;

        if (CheckCollisionRecs(playerRecs[1], levelRect)) {
            //TraceLog(LOG_ALL, "char 2 on floor!");
            onGround[1] = true; 
            groundLevel[1] = checkY;
            break;
        }
    }

    // wall collision check char 1: check for the 3 adjacing tiles to our right
    for (int offsetY = -1; offsetY < 2; offsetY++) {

    }

    for (int i = 0; i < 2; i++) {
        gameData->JumpVelocity[i] = onGround[i] ? 0 : (gameData->JumpVelocity[i] - (250.0f * dt));
    }

    if (IsKeyPressed(KEY_SPACE)) {
        for (int i = 0; i < 2; i++) {
            gameData->JumpVelocity[i] = 150.0f;
        }
    }

    gameData->PlayerPosY[0] -= gameData->JumpVelocity[0] * dt;
    gameData->PlayerPosY[1] += gameData->JumpVelocity[1] * dt;

    if (onGround[0]) {
        gameData->PlayerPosY[0] = (groundLevel[0] - 1) * gameData->TileSize;
    }
    if (onGround[1]) {
        gameData->PlayerPosY[1] = (groundLevel[1] + 1) * gameData->TileSize;
    }
}

void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors) {
    const float tileSize = gameData->TileSize;

    // Draw level
    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case TILE_VOID:
                break;
            case TILE_FLOOR:
                DrawRectangle(x * tileSize, y * tileSize, tileSize, tileSize, gameColors[0]);
                break;
            default:
                break;
            }  
        }
    }

    // Draw char 1
    DrawRectangleV((Vector2){ gameData->PlayerPosX, gameData->PlayerPosY[0] }, (Vector2){ tileSize, tileSize }, gameColors[1]);

    // Draw char 2
    DrawRectangleV((Vector2){ gameData->PlayerPosX, gameData->PlayerPosY[1] }, (Vector2){ tileSize, tileSize }, gameColors[2]);

    for (int i = 0; i < gameData->DebugRectangleCount; ++i) {
        DrawRectangleRec(gameData->DebugRectangles[i], RAYWHITE);
    }
}