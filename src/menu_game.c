#include "game.h"
#include "utils.h"

#include "raymath.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

void game_menu_init(GameData* gameData, int screenWidth, int screenHeight) {
    gameData->PlayerPosX = 101.0f;
    gameData->PlayerPosY[0] = 88.0f;
    gameData->PlayerPosY[1] = 206.0f;

    gameData->Enemies[0] = (Enemy){
        435, 80, (Vector2) { 435.0f, 80.0f },
        0.0f, 9999, (GetRandomValue(0, 1000) / 1000.0f)
    };

    gameData->Enemies[1] = (Enemy){
        590, 202, (Vector2) { 590.0f, 202.0f },
        0.0f, 9999, (GetRandomValue(0, 1000) / 1000.0f)
    };

    gameData->EnemyCount = 2;
}

void game_menu_tick(GameData* gameData, int screenWidth, int screenHeight, float dt) {  
    gameData->Timer += dt;

    for (int i = 0; i < 2; ++i) {
        gameData->AnimationTimer[i] += gameData->AnimationSpeed * dt;

        if (gameData->AnimationTimer[i] > 1.0f) {
            gameData->AnimationTimer[i] = 0.0f;
            gameData->AnimationRectIndex[i] += 1;

            if (gameData->AnimationRectIndex[i] > gameData->CharFrameCount-1) {
                gameData->AnimationRectIndex[i] = 0;
            }
        }
    }

    gameData->EnemyAnimationTimer += 2.0f * dt;

    if (gameData->EnemyAnimationTimer > 1.0f) {
        gameData->EnemyAnimationTimer = 0.0f;
        gameData->EnemyAnimationIndex += 1;

        if (gameData->EnemyAnimationIndex > gameData->EnemyFrameCount-1) {
            gameData->EnemyAnimationIndex = 0;
        }
    }

    gameData->PortalAnimationTimer += 5.0f * dt;

    if (gameData->PortalAnimationTimer > 1.0f) {
        gameData->PortalAnimationTimer = 0.0f;
        gameData->PortalAnimationIndex += 1;

        if (gameData->PortalAnimationIndex > gameData->PortalFrameCount-1) {
            gameData->PortalAnimationIndex = 0;
        }
    }

    bool onGround[2] = { false };

    if (!gameData->GoingUp[0]) {
        if (gameData->PlayerPosY[0] >= 88.0f) {
            onGround[0] = true; 
        }
    }

    if (!gameData->GoingUp[1]) {
        if (gameData->PlayerPosY[1] <= 206.0f) {
            onGround[1] = true;
        }
    }

    for (int i = 0; i < 2; i++) {
        gameData->JumpVelocity[i] = onGround[i] ? 0 : (gameData->JumpVelocity[i] - (400.0f * dt));

        if (gameData->JumpVelocity[i] < -0.1f) { 
            gameData->GoingUp[i] = false;
        }
    }

    for (int i = 0; i < 2; i++) {
        if (gameData->GoingUp[i]) {
            gameData->JumpTimer[i] += dt;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        bool jumped = false;

        for (int i = 0; i < 2; i++) {
            if (onGround[i]) {
                gameData->JumpVelocity[i] = 150.0f;
                gameData->GoingUp[i] = true;
                gameData->JumpTimer[i] = 0.0f;
                onGround[i] = false;
                
                jumped = true;
            }
        }

        if (jumped) {
            int randSound = GetRandomValue(0, 2);
            PlaySound(gameData->JumpSoundTop[randSound]);
        }
    }

    if (IsKeyDown(KEY_SPACE)) {
        for (int i = 0; i < 2; i++) {
            if (!onGround[i] && gameData->GoingUp[i] && gameData->JumpTimer[i] < 0.4f) {
                gameData->JumpVelocity[i] += 350.0f * dt;
            }
        }
    }

    gameData->PlayerPosY[0] -= gameData->JumpVelocity[0] * dt;
    gameData->PlayerPosY[1] += gameData->JumpVelocity[1] * dt;

    for (int i = 0; i < gameData->EnemyCount; ++i) {
        if (gameData->Enemies[i].HitTimer > 0.0f) {
            gameData->Enemies[i].HitTimer -= dt;
        }
    }

    for (int i = 0; i < gameData->EnemyCount; ++i) {
        gameData->Enemies[i].PosOffsetTimer += dt;
    }

    // bullet stuff

    for (int i = 0; i < gameData->BulletCount; ++i) {
        gameData->BulletPos[i].x += 500.0f * dt;
    }

    for (int i = 0; i < gameData->BulletCount; ++i) {
        if (gameData->BulletPos[i].x > gameData->CameraPosX + screenWidth) {
            gameData->BulletPos[i] = gameData->BulletPos[gameData->BulletCount - 1];
            gameData->BulletCount -= 1;
        }
    }
     
    for (int i = 0; i < gameData->BulletCount; ++i) { 
        for (int enemyI = 0; enemyI < gameData->EnemyCount; ++enemyI) {
            Rectangle enemyRect = (Rectangle){ gameData->Enemies[enemyI].Pos.x, gameData->Enemies[enemyI].Pos.y, gameData->TileSize, gameData->TileSize };

            if (CheckCollisionCircleRec(gameData->BulletPos[i], 5.0f, enemyRect)) {
                gameData->BulletPos[i] = gameData->BulletPos[gameData->BulletCount - 1];
                gameData->BulletCount -= 1;

                gameData->Enemies[enemyI].HitTimer = 0.2f; 
            }
        }
    }

    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        gameData->GunAtTop = !gameData->GunAtTop;
    }

    if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL)) {
        float posY = gameData->GunAtTop ? gameData->PlayerPosY[0] + gameData->TileSize / 2.0f : gameData->PlayerPosY[1] + gameData->TileSize / 2.0f;
        gameData->BulletPos[gameData->BulletCount] = (Vector2){ gameData->PlayerPosX + gameData->TileSize, posY };
        gameData->BulletCount += 1;
    }
}

void game_menu_draw(GameData* gameData, Color* gameColors) {
    // draw bullets
    float radius1 = 5.0f;
    float radius2 = 4.0f;
    float radius3 = 3.0f;
    float radius4 = 2.0f; 
    for (uint32_t i = 0; i < gameData->BulletCount; i++) {  
        DrawCircle(gameData->BulletPos[i].x + radius1 / 2 - gameData->CameraPosX, gameData->BulletPos[i].y + radius1 / 2, radius1, gameColors[1]);
        DrawCircle(gameData->BulletPos[i].x + radius2 / 2 - gameData->CameraPosX, gameData->BulletPos[i].y + radius2 / 2, radius2, gameColors[3]);
        DrawCircle(gameData->BulletPos[i].x + radius3 / 2 - gameData->CameraPosX, gameData->BulletPos[i].y + radius3 / 2, radius3, gameColors[5]);
        DrawCircle(gameData->BulletPos[i].x + radius4 / 2 - gameData->CameraPosX, gameData->BulletPos[i].y + radius4 / 2, radius4, gameColors[6]);
    }

    // draw enemies
    for (uint32_t i = 0; i < gameData->EnemyCount; i++) {
        bool isHit = gameData->Enemies[i].HitTimer > 0.01f;
        bool isTop = gameData->Enemies[i].PosY < 150.0f;
        float offsetY = Lerp(0.0f, isTop ? -8.0f : 8.0f, (sinf(gameData->Enemies[i].PosOffsetTimer * 3.0f) + 2) / 2.0f);
        Texture toUse = isTop ? (isHit ? gameData->EnemyHitSheet[0] : gameData->EnemySheet[0]) : (isHit ? gameData->EnemyHitSheet[1] : gameData->EnemySheet[1]);

        DrawTextureRec(toUse, (Rectangle) { gameData->TileSize * gameData->EnemyAnimationIndex * 1.4f, 0, gameData->EnemySheet[0].width / gameData->EnemyFrameCount, gameData->EnemySheet[0].height }, (Vector2) { gameData->Enemies[i].Pos.x - gameData->CameraPosX - 15.0f, gameData->Enemies[i].Pos.y + offsetY }, WHITE);
    }

    // draw portals
    DrawTextureRec(gameData->PortalSheet[0], (Rectangle) { gameData->TileSize* gameData->PortalAnimationIndex * 2.2f, 0, gameData->PortalSheet[0].width / gameData->PortalFrameCount, gameData->PortalSheet[0].height }, (Vector2) { gameData->PortalPosX - gameData->CameraPosX - 15.0f, gameData->PortalPosY[0] - 30.0f }, WHITE);
    DrawTextureRec(gameData->PortalSheet[1], (Rectangle) { gameData->TileSize* gameData->PortalAnimationIndex * 2.2f, 0, gameData->PortalSheet[1].width / gameData->PortalFrameCount, gameData->PortalSheet[1].height }, (Vector2) { gameData->PortalPosX - gameData->CameraPosX - 15.0f, gameData->PortalPosY[1] - 30.0f }, WHITE);

    // Draw char 1
    DrawTextureRec(gameData->CharSheet[0], (Rectangle) { gameData->TileSize* gameData->AnimationRectIndex[0] * 1.3f, 0, gameData->CharSheet[0].height, gameData->CharSheet[0].height }, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[0] - 8.0f }, WHITE);

    // Draw char 2
    DrawTextureRec(gameData->CharSheet[1], (Rectangle) { gameData->TileSize* gameData->AnimationRectIndex[1] * 1.3f, 0, gameData->CharSheet[1].height, gameData->CharSheet[1].height }, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[1] }, WHITE);

    // tether
    {
        int charStartX = (int)gameData->PlayerPosX + 23;
        int charYUp = (int)gameData->PlayerPosY[0] + 50;
        int charYDown = (int)gameData->PlayerPosY[1];
        for (int x = -2; x <= 2; x++) {
            for (int y = charYUp; y < charYDown; y++) {
                if ((x + y) % 3 == 0) {
                    DrawPixel(charStartX + x, y, gameColors[x+2]);
                }        
            }
        }
    }
}