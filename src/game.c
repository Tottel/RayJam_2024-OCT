#include "game.h"
#include "utils.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

void game_init(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight) {
    const float tileSize = screenHeight / (float)levelData->LevelHeight;
    gameData->TileSize = tileSize;

    game_restart(gameData, levelData);
}

void game_exit(GameData* gameData) {

}

void game_tick(GameData* gameData, const LevelData* levelData, float dt) { 
    gameData->Timer += dt;

    bool onGround[2] = { false };
    uint16_t groundY[2] = { 0 };
    
    bool againstWall = false; // if 1 char is against a wall, they are both stuck
    uint32_t wallX = 0;

    Rectangle playerRecs[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0], gameData->TileSize, gameData->TileSize },
                                (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1], gameData->TileSize, gameData->TileSize }};

    gameData->DebugRectangleCount = 0;

    // ground CollisionCheck char 1: Check for the 3 adjacing tiles directly underneath us
    if (!gameData->GoingUp[0]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize;
            int charY = gameData->PlayerPosY[0] / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY + 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

            Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize, gameData->TileSize, 5.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (CheckCollisionRecs(playerRecs[0], levelRect)) {
                onGround[0] = true;
                groundY[0] = checkY;
                break;
            }
        }
    }

    // ground CollisionCheck char 2: Check for the 3 adjacing tiles directly above us
    if (!gameData->GoingUp[1]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize;
            int charY = (gameData->PlayerPosY[1] + gameData->TileSize * 0.5f) / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY - 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

            Rectangle levelRect = { checkX * gameData->TileSize, (checkY * gameData->TileSize) + gameData->TileSize - 5.0f, gameData->TileSize, 5.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (CheckCollisionRecs(playerRecs[1], levelRect)) {
                //TraceLog(LOG_ALL, "char 2 on floor!");
                onGround[1] = true;
                groundY[1] = checkY;
                break;
            }
        }
    }
    
    // wall collision check char 1: check for 2 adjacing tiles to our right
    for (int offsetY = -1; offsetY < 2; offsetY++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[0] + gameData->TileSize*0.5f) / gameData->TileSize;

        int checkX = charX + 1;
        int checkY = charY + offsetY;

        if (checkY < 0) continue;

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        Rectangle levelRect = { (checkX * gameData->TileSize), checkY * gameData->TileSize + 2.0f, 5.0f, gameData->TileSize - 4.0f};

        if (CheckCollisionRecs(playerRecs[0], levelRect)) {
            againstWall = true;
            wallX = checkX;
            break;
        }
    }

    // wall collision check char 2: check for 2 adjacing tiles to our right
    for (int offsetY = 0; offsetY < 2; offsetY++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[1] + gameData->TileSize*0.5f) / gameData->TileSize;

        int checkX = charX + 1;
        int checkY = charY + offsetY;      

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        Rectangle levelRect = { (checkX * gameData->TileSize), checkY * gameData->TileSize + 2.0f, 5.0f, gameData->TileSize - 4.0f };

        if (CheckCollisionRecs(playerRecs[1], levelRect)) {
            againstWall = true;
            wallX = checkX;
            break;
        }
    }

    gameData->PlayerPosX += againstWall ? 0.0f : 200.0f * dt;

    for (int i = 0; i < 2; i++) {
        gameData->JumpVelocity[i] = onGround[i] ? 0 : (gameData->JumpVelocity[i] - (250.0f * dt));

        if (gameData->JumpVelocity[i] < -0.1f) {
            gameData->GoingUp[i] = false;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        for (int i = 0; i < 2; i++) {
            if (onGround[i]) {
                gameData->JumpVelocity[i] = 150.0f;
                gameData->GoingUp[i] = true;
                onGround[i] = false;
            }
        }
    }

    gameData->PlayerPosY[0] -= gameData->JumpVelocity[0] * dt;
    gameData->PlayerPosY[1] += gameData->JumpVelocity[1] * dt;

    if (onGround[0]) {
        gameData->PlayerPosY[0] = (groundY[0] - 1) * gameData->TileSize + 1.0f;
    }
    if (onGround[1]) {
        gameData->PlayerPosY[1] = (groundY[1] + 1) * gameData->TileSize - 1.0f;
    }
    if (againstWall) {
        gameData->PlayerPosX = (wallX - 1) * gameData->TileSize + 1.0f;
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
        DrawRectangleRec(gameData->DebugRectangles[i], gameData->DebugRectanglesColors[i]);
    }
}

void game_restart(GameData* gameData, const LevelData* levelData) {
    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case TILE_SPAWN_1:
                gameData->PlayerPosX = x * gameData->TileSize;
                gameData->PlayerPosY[0] = y * gameData->TileSize;
                break;
            case TILE_SPAWN_2:
                gameData->PlayerPosX = x * gameData->TileSize; // Should be the same as before..
                gameData->PlayerPosY[1] = y * gameData->TileSize;
                break;
            default:
                break;
            }
        }
    }

    gameData->JumpVelocity[0] = 0.0f;
    gameData->JumpVelocity[1] = 0.0f;

    gameData->JumpAcceleration[0] = 0.0f;
    gameData->JumpAcceleration[1] = 0.0f;

    gameData->GoingUp[0] = false;
    gameData->GoingUp[1] = false;

    gameData->Timer = 0.0f;
}