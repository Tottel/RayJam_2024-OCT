#include "game.h"
#include "utils.h"

#include "raymath.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "image_color_parser.h"

void game_create(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight) {
    const float tileSize = screenHeight / (float)levelData->LevelHeight;
    gameData->TileSize = tileSize;

    game_restart(gameData, levelData);

    // player chars
    Image tempChar = load_and_convert_image("resources/characters/goblin_run.png", allowedColors, 8);

    gameData->CharFrameCount = 6; // LoadImageAnim returns the wrong value :(((

    ImageResize(&tempChar, tileSize * gameData->CharFrameCount * 1.3f, tileSize * 1.3f);
    gameData->CharSheet[0] = LoadTextureFromImage(tempChar);

    ImageFlipVertical(&tempChar);
    gameData->CharSheet[1] = LoadTextureFromImage(tempChar);

    UnloadImage(tempChar);

    // enemies
    Image tempEnemy = load_and_convert_image("resources/characters/wachter_side.png", allowedColors, 8);
    Image tempHitEnemy = load_and_convert_image("resources/characters/wachter_side_hit.png", allowedColors, 8);
    gameData->EnemyFrameCount = 3;

    ImageResize(&tempEnemy, tileSize * gameData->EnemyFrameCount * 1.4f, tileSize * 1.4f);
    ImageResize(&tempHitEnemy, tileSize * gameData->EnemyFrameCount * 1.4f, tileSize * 1.4f);
    gameData->EnemySheet[0] = LoadTextureFromImage(tempEnemy);
    gameData->EnemyHitSheet[0] = LoadTextureFromImage(tempHitEnemy);

    ImageFlipVertical(&tempEnemy);
    ImageFlipVertical(&tempHitEnemy);
    gameData->EnemySheet[1] = LoadTextureFromImage(tempEnemy);
    gameData->EnemyHitSheet[1] = LoadTextureFromImage(tempHitEnemy);

    UnloadImage(tempEnemy);
    UnloadImage(tempHitEnemy);

    // portal
    Image tempPortal = LoadImage("resources/images/portal.png");
    gameData->PortalFrameCount = 8;

    ImageResize(&tempPortal, tileSize * gameData->PortalFrameCount * 2.2f, tileSize * 2.2f);
    ImageFlipHorizontal(&tempPortal);
    gameData->PortalSheet[0] = LoadTextureFromImage(tempPortal);

    ImageFlipVertical(&tempPortal);
    gameData->PortalSheet[1] = LoadTextureFromImage(tempPortal);

    UnloadImage(tempPortal);

    // sound
    gameData->JumpSoundTop[0] = LoadSound("resources/sound/hop_top_1.wav");
    gameData->JumpSoundTop[1] = LoadSound("resources/sound/hop_top_2.wav");
    gameData->JumpSoundTop[2] = LoadSound("resources/sound/hop_top_3.wav");

    gameData->Respawn = LoadSound("resources/sound/respawn.wav");
    gameData->Portal = LoadSound("resources/sound/portal.wav");
}

void game_init(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight) {
    const float tileSize = screenHeight / (float)levelData->LevelHeight;
    gameData->TileSize = tileSize;

    game_restart(gameData, levelData); 
}

void game_exit(GameData* gameData) {
    UnloadTexture(gameData->CharSheet[0]);
    UnloadTexture(gameData->CharSheet[1]);
    UnloadTexture(gameData->EnemySheet[0]);
    UnloadTexture(gameData->EnemySheet[1]);
    UnloadTexture(gameData->EnemyHitSheet[0]);
    UnloadTexture(gameData->EnemyHitSheet[1]);
    UnloadTexture(gameData->PortalSheet[0]);
    UnloadTexture(gameData->PortalSheet[1]);

    for (int i = 0; i < 3; ++i) {
        UnloadSound(gameData->JumpSoundTop[i]);
    }

    UnloadSound(gameData->Respawn);
    UnloadSound(gameData->Portal);
}

void game_tick(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight, float dt) {  
    gameData->Timer += dt;

    for (int i = 0; i < 2; ++i) {
        gameData->AnimationTimer[i] += 3.0f * dt;

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
    
    bool againstWall = false; // if 1 char is against a wall, they are both stuck

    bool againstCeiling[2] = { false };

    Rectangle playerRecsForGround[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0] + gameData->TileSize - 10.0f + 0.5f, gameData->TileSize, 10.0f },
                                         (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1] - 0.5f, gameData->TileSize, 10.0f }};

    Rectangle playerRecsForCeilings[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0] - 0.5f, gameData->TileSize, 10.0f },
                                           (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1] + gameData->TileSize - 10.0f + 0.5f, gameData->TileSize, 10.0f } };

    Rectangle playerRecsForWalls[2] = { (Rectangle) { gameData->PlayerPosX + 0.5f + gameData->TileSize - 10.0f, gameData->PlayerPosY[0] + 5.0f, 10.0f, gameData->TileSize - 10.0f },
                                        (Rectangle) { gameData->PlayerPosX + 0.5f + gameData->TileSize - 10.0f, gameData->PlayerPosY[1] + 5.0f, 10.0f, gameData->TileSize - 10.0f }};

    Rectangle playersRecsFull[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0], gameData->TileSize, gameData->TileSize },
                                     (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1], gameData->TileSize, gameData->TileSize } };
    
    gameData->DebugRectangleCount = 0;

    //gameData->DebugRectangles[gameData->DebugRectangleCount] = playerRecsForGround[0];
    //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = GREEN;
    //gameData->DebugRectangleCount += 1;
    //
    //gameData->DebugRectangles[gameData->DebugRectangleCount] = playerRecsForWalls[0];
    //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = GREEN;
    //gameData->DebugRectangleCount += 1;

    // ground CollisionCheck char 1: Check for the 3 adjacing tiles directly underneath us
    if (!gameData->GoingUp[0]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize;
            int charY = (gameData->PlayerPosY[0] + gameData->TileSize * 0.8f) / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY + 1; 

            Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize, gameData->TileSize, 10.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR &&
                levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

            if (CheckCollisionRecs(playerRecsForGround[0], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount-1] = RED;
                onGround[0] = true; 
                break;
            }
        }
    }

    // ground CollisionCheck char 2: Check for the 3 adjacing tiles directly above us
    if (!gameData->GoingUp[1]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize; 
            int charY = (gameData->PlayerPosY[1] + gameData->TileSize * 0.2f) / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY - 1;


            Rectangle levelRect = { checkX * gameData->TileSize, (checkY * gameData->TileSize) + gameData->TileSize - 10.0f, gameData->TileSize, 10.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR &&
                levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

            if (CheckCollisionRecs(playerRecsForGround[1], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;
                //TraceLog(LOG_ALL, "char 2 on floor!");
                onGround[1] = true;
                break;
            }
        }
    }

    // ceiling collision check char 1: check for the 3 tiles directly above us
    //if (gameData->GoingUp[0]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize;
            int charY = (gameData->PlayerPosY[0] + gameData->TileSize * 0.8f) / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY - 1;

            if (checkY < 0) { continue; }

            Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize + (gameData->TileSize - 10.0f), gameData->TileSize, 10.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

            if (CheckCollisionRecs(playerRecsForCeilings[0], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount-1] = RED;
                againstCeiling[0] = true;
                break;
            }
        }
    //}

    // ceiling collision check char 2: check for the 3 tiles directly below us
    //if (gameData->GoingUp[0]) {
        for (int offsetX = -1; offsetX < 2; offsetX++) {
            int charX = gameData->PlayerPosX / gameData->TileSize;
            int charY = (gameData->PlayerPosY[1] + gameData->TileSize * 0.2f) / gameData->TileSize;

            int checkX = charX + offsetX;
            int checkY = charY + 1;

            if (checkY >= levelData->LevelHeight) continue;

            Rectangle levelRect = { checkX * gameData->TileSize, checkY * gameData->TileSize, gameData->TileSize, 10.0f };
            //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
            //gameData->DebugRectangleCount += 1;

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

            if (CheckCollisionRecs(playerRecsForCeilings[1], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;
                againstCeiling[1] = true;
                break;
            }
        }
    //}

    // wall collision check char 1: check for 2 adjacing tiles to our right
    for (int offsetY = -1; offsetY < 2; offsetY++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[0] + gameData->TileSize * 0.95f) / gameData->TileSize;

        int checkX = charX + 1;
        int checkY = charY + offsetY;

        if (checkY < 0) continue;

        Rectangle levelRect = { (checkX * gameData->TileSize), checkY * gameData->TileSize + 2.0f, 10.0f, gameData->TileSize - 10.0f};
        //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
        //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
        //gameData->DebugRectangleCount += 1;

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

        if (CheckCollisionRecs(playerRecsForWalls[0], levelRect)) {
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;

            againstWall = true;
            break; 
        } 
    }

    // wall collision check char 2: check for 2 adjacing tiles to our right
    for (int offsetY = 0; offsetY < 2; offsetY++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[1] + gameData->TileSize*0.1f) / gameData->TileSize;

        int checkX = charX + 1;
        int checkY = charY + offsetY;

        Rectangle levelRect = { (checkX * gameData->TileSize), checkY * gameData->TileSize + 2.0f, 10.0f, gameData->TileSize - 10.0f };
        //gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
        //gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
        //gameData->DebugRectangleCount += 1;

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_PLATFORM) continue;

        if (CheckCollisionRecs(playerRecsForWalls[1], levelRect)) {
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;

            againstWall = true;
            break; 
        }
    }

    const float playerMoveSpeed = 300.0f;

    gameData->PlayerPosX += againstWall ? 0.0f : playerMoveSpeed * dt; 

    for (int i = 0; i < 2; i++) {

        if (againstCeiling[i]) {
            gameData->JumpVelocity[i] = -1.0f;
        }

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
            if (onGround[i] && !againstCeiling[i]) {
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

    float camSpeed = playerMoveSpeed;

    float cameraLagDistance = gameData->PlayerPosX - gameData->CameraPosX;

    if (cameraLagDistance < 120) {
        camSpeed -= cameraLagDistance / 15.0f;
    }

    if (againstWall) {
        camSpeed -= 100.0f;
    }

    gameData->CameraPosX += camSpeed * dt;

    for (int i = 0; i < gameData->EnemyCount; ++i) {
        if (gameData->Enemies[i].HitTimer > 0.0f) {
            gameData->Enemies[i].HitTimer -= dt;
        }
    }

    for (int i = 0; i < gameData->EnemyCount; ++i) {
        if (gameData->Enemies[i].HP <= 0) {
            memcpy(gameData->Enemies + i, gameData->Enemies + (gameData->EnemyCount - 1), sizeof(Enemy));

            gameData->EnemyCount -= 1;
        }
    }

    for (int i = 0; i < gameData->EnemyCount; ++i) {
        gameData->Enemies[i].PosOffsetTimer += dt;
    }

    // bullet stuff

    for (int i = 0; i < gameData->BulletCount; ++i) {
        gameData->BulletPos[i].x += (playerMoveSpeed + 500.0f) * dt;
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

                gameData->Enemies[enemyI].HP -= 1;
            }
        }
    }

    for (int enemyI = 0; enemyI < gameData->EnemyCount; ++enemyI) {
        Rectangle enemyRect = (Rectangle){ gameData->Enemies[enemyI].Pos.x, gameData->Enemies[enemyI].Pos.y, gameData->TileSize, gameData->TileSize };

        for (int j = 0; j < 2; j++) {
            if (CheckCollisionRecs(enemyRect, playersRecsFull[j])) {
                //game_restart(gameData, levelData);
                gameData->RestartLevel = true;
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

    if (gameData->PlayerPosX - gameData->CameraPosX < 15) {
        gameData->RestartLevel = true;
        //game_restart(gameData, levelData);
    }

    if (gameData->PlayerPosX >= gameData->PortalPosX) {
        gameData->NextLevel = true;

        if (!IsSoundPlaying(gameData->Portal)) {
            PlaySound(gameData->Portal);
        }
    }
}

void game_draw(GameData* gameData, const LevelData* levelData, Color* gameColors) {
    const float tileSize = gameData->TileSize;

    // Only render the tiles that are on the screen
    int xStart = gameData->CameraPosX / tileSize;
    int xEnd = xStart + (GetScreenWidth() / tileSize) + 2;

    // TODO Find the top platform at every X pos and add some random dithering on it to improve visibility
    // Draw level
    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = xStart; x < xEnd; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)]; 

            switch (tileType) {
            case TILE_PLATFORM:
            {
                bool isTop = false;
                bool high = false;
                if (y < levelData->LevelHeight / 2) {
                    isTop = y > 0 && levelData->Tiles[x + ((y - 1) * levelData->LevelWidth)] != TILE_PLATFORM;
                    high = true;
                }
                else {
                    isTop = y < levelData->LevelHeight - 1 && levelData->Tiles[x + ((y + 1) * levelData->LevelWidth)] != TILE_PLATFORM;
                }

                DrawRectangle(x * tileSize - gameData->CameraPosX - 1, y * tileSize - 1, tileSize + 2, tileSize + 2, gameColors[0]);

                if (isTop) {
                    for (int y2 = 0; y2 < tileSize / 7; ++y2) {
                        for (int x2 = 0; x2 < tileSize; ++x2) {
                            if ((x2 / 2 + y2) % 4 == 0) {
                                int posX = x * tileSize - gameData->CameraPosX - 1 + x2;
                                int posY = high ? (y * tileSize + y2) : (y * tileSize + (tileSize - tileSize / 7) + y2);
                                DrawPixel(posX, posY, gameColors[1]);
                            }
                        }
                    }

                    for (int y2 = tileSize / 7; y2 < tileSize / 4; ++y2) {
                        for (int x2 = 0; x2 < tileSize; ++x2) {
                            if ((x2 / 2 + y2) % 10 == 0) {
                                int posX = x * tileSize - gameData->CameraPosX - 1 + x2;
                                int posY = high ? (y * tileSize + y2) : ((y + 1) * tileSize - tileSize / 4 - tileSize / 7 + y2);
                                DrawPixel(posX, posY, gameColors[1]);
                            }
                        }
                    }
                }
            } break;
            default:
                break;
            }  
        }
    }

    // draw floor
    DrawRectangle(0, GetScreenHeight() / 2 - tileSize / 2, GetScreenWidth(), tileSize, gameColors[0]);

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
        bool isTop = gameData->Enemies[i].PosY < levelData->LevelHeight / 2;
        float offsetY = Lerp(0.0f, isTop ? -14.0f : 14.0f, (sinf(gameData->Enemies[i].PosOffsetTimer * 5.0f) + 2) / 2.0f);
        offsetY -= isTop ? 0.0f : gameData->TileSize / 2;

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
        int charStartX = (int)gameData->PlayerPosX - gameData->CameraPosX + tileSize / 2;
        int charYUp = (int)gameData->PlayerPosY[0] + tileSize;
        int charYDown = (int)gameData->PlayerPosY[1];
        for (int x = -2; x <= 2; x++) {
            for (int y = charYUp; y < charYDown; y++) {
                if ((x + y) % 3 == 0) {
                    DrawPixel(charStartX + x, y, gameColors[x+2]);
                }        
            }
        }
    }

#if defined (_DEBUG)
    for (int i = 0; i < gameData->DebugRectangleCount; ++i) {
        Rectangle rect = gameData->DebugRectangles[i];
        rect.x -= gameData->CameraPosX;

        DrawRectangleRec(rect, gameData->DebugRectanglesColors[i]);
    }
#endif
}

void game_bladesaws_draw(GameData* gameData, Texture2D bladesaw, float dt) {
    gameData->BladeSawTimer += dt;
    if (gameData->BladeSawTimer > 0.1f) {
        gameData->BladeSawTimer = 0.0f;
        gameData->BladeSawRectIndex += 1;
        if (gameData->BladeSawRectIndex >= 2) {
            gameData->BladeSawRectIndex = 0;
        }
    }

    Rectangle blades = (Rectangle){ gameData->BladeSawRectIndex * (bladesaw.width / 2), 0, bladesaw.width / 2, bladesaw.height };
    float startPosY = -bladesaw.height / 2;

    DrawTextureRec(bladesaw, blades, (Vector2) { 0, startPosY + bladesaw.height * 0 }, WHITE);
    DrawTextureRec(bladesaw, blades, (Vector2) { 0, startPosY + bladesaw.height * 1 }, WHITE);
    DrawTextureRec(bladesaw, blades, (Vector2) { 0, startPosY + bladesaw.height * 2 }, WHITE);
    DrawTextureRec(bladesaw, blades, (Vector2) { 0, startPosY + bladesaw.height * 3 }, WHITE);
    DrawTextureRec(bladesaw, blades, (Vector2) { 0, startPosY + bladesaw.height * 4 }, WHITE);
}

void game_restart(GameData* gameData, const LevelData* levelData) {
    gameData->NextLevel = false;

    gameData->JumpVelocity[0] = 0.0f;
    gameData->JumpVelocity[1] = 0.0f;
    gameData->JumpAcceleration[0] = 0.0f;
    gameData->JumpAcceleration[1] = 0.0f;
    gameData->JumpTimer[0] = 0.0f;
    gameData->JumpTimer[1] = 0.0f;

    gameData->GoingUp[0] = false;
    gameData->GoingUp[1] = false;

    gameData->AnimationTimer[0] = 0.0f;
    gameData->AnimationTimer[1] = 0.0f;
    gameData->AnimationRectIndex[0] = 0;
    gameData->AnimationRectIndex[1] = 0;

    gameData->CameraPosX = 0.0f;

    gameData->BladeSawTimer = 0.0f;
    gameData->BladeSawRectIndex = 0;

    gameData->EnemyCount = 0;

    for (int i = 0; i < 15; i++) {
        gameData->Enemies[i].HitTimer = 0.0f;
        gameData->Enemies[i].HP = 2;
        gameData->Enemies[i].PosOffsetTimer = 0.0f;
    }

    gameData->BulletFireTimer = 0.0f;
    gameData->BulletCount = 0;
    gameData->GunAtTop = true;

    gameData->Timer = 0.0f;
    
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
            case TILE_ENEMY:
                gameData->Enemies[gameData->EnemyCount].PosX = x;
                gameData->Enemies[gameData->EnemyCount].PosY = y;
                gameData->Enemies[gameData->EnemyCount].Pos = (Vector2){x * gameData->TileSize, y * gameData->TileSize};
                gameData->EnemyCount += 1;
                break;
            case TILE_PORTAL_1:
                gameData->PortalPosX = x * gameData->TileSize;
                gameData->PortalPosY[0] = y * gameData->TileSize;
                break;
            case TILE_PORTAL_2:
                gameData->PortalPosX = x * gameData->TileSize; // Should be the same as before..
                gameData->PortalPosY[1] = y * gameData->TileSize;;
                break;
            default:
                break;
            }
        }
    }
}