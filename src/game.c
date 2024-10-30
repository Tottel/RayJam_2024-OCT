#include "game.h"
#include "utils.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include "image_color_parser.h"

static Texture Char1Sheet;
static Texture Char2Sheet;
static int CharFrameCount;
static float AnimationSpeed = 4.0f;

void game_init(GameData* gameData, const LevelData* levelData, Color* allowedColors, int screenWidth, int screenHeight) {
    const float tileSize = screenHeight / (float)levelData->LevelHeight;
    gameData->TileSize = tileSize;

    game_restart(gameData, levelData);

    Image tempChar1 = load_and_convert_image("resources/characters/goblin_run.png", allowedColors, 8);
    Image tempChar2 = load_and_convert_image("resources/characters/goblin_run.png", allowedColors, 8);
    
    CharFrameCount = 6; // LoadImageAnim returns the wrong value :(((

    ImageResize(&tempChar1, tileSize * CharFrameCount, tileSize);
    ImageResize(&tempChar2, tileSize * CharFrameCount, tileSize);
    ImageFlipVertical(&tempChar2);

    Char1Sheet = LoadTextureFromImage(tempChar1); 
    Char2Sheet = LoadTextureFromImage(tempChar2);

    UnloadImage(tempChar1);
    UnloadImage(tempChar2);

    // particles
    gameData->PSConnection = ParticleSystem_New();
    assert(gameData->PSConnection != NULL);

    gameData->PSTexture = load_and_convert_texture("resources/images/connection.png", allowedColors, 8);

    struct EmitterConfig emitterDown = {
        .direction = (Vector2){.x = 0, .y = 1.0f},
        .velocity = (FloatRange){.min = 80, .max = 100},
        .directionAngle = (FloatRange){.min = -1, .max = 1},
        .velocityAngle = (FloatRange){.min = -1, .max = 1},
        .size = (FloatRange){.min = 1.5f, 3.0f},
        .burst = (IntRange){.min = 100, .max = 400},
        .capacity = 200,
        .emissionRate = 40,
        .origin = (Vector2){.x = 0, .y = 0},
        .externalAcceleration = (Vector2){.x = 0, .y = 0},
        .Color = allowedColors[4],
        .age = (FloatRange){.min = 0.3f, .max = 0.5f},
        .haltTime = (FloatRange){1.5f, 3.00f},
        .blendMode = BLEND_ALPHA,
        .texture = gameData->PSTexture,

        //.particle_Decellerator = Particle_Decelerator_Sudden,
    };
    gameData->EmitterConnectionDown = Emitter_New(emitterDown);
    assert(gameData->EmitterConnectionDown != NULL);

    struct EmitterConfig emitterUp = {
        .direction = (Vector2){.x = 0, .y = -1.0f},
        .velocity = (FloatRange){.min = 80, .max = 100},
        .directionAngle = (FloatRange){.min = -1, .max = 1},
        .velocityAngle = (FloatRange){.min = -1, .max = 1},
        .size = (FloatRange){.min = 1.5f, 3.0f},
        .burst = (IntRange){.min = 100, .max = 400},
        .capacity = 200,
        .emissionRate = 40,
        .origin = (Vector2){.x = 0, .y = 0},
        .externalAcceleration = (Vector2){.x = 0, .y = 0},
        .Color = allowedColors[4],
        .age = (FloatRange){.min = 0.3f, .max = 0.5f},
        .haltTime = (FloatRange){1.5f, 3.00f},
        .blendMode = BLEND_ALPHA,
        .texture = gameData->PSTexture,

        //.particle_Decellerator = Particle_Decelerator_Sudden,
            };
    gameData->EmitterConnectionUp = Emitter_New(emitterUp);
    assert(gameData->EmitterConnectionUp != NULL);

    ParticleSystem_Register(gameData->PSConnection, gameData->EmitterConnectionDown);
    ParticleSystem_Register(gameData->PSConnection, gameData->EmitterConnectionUp);
    ParticleSystem_Start(gameData->PSConnection);
}

void game_exit(GameData* gameData) {
    UnloadTexture(Char1Sheet);
    UnloadTexture(gameData->PSTexture);

    ParticleSystem_CleanAndFree(gameData->PSConnection);
}

void game_tick(GameData* gameData, const LevelData* levelData, int screenWidth, int screenHeight, float dt) { 
    gameData->Timer += dt;

    for (int i = 0; i < 2; ++i) {
        gameData->AnimationTimer[i] += AnimationSpeed * dt;

        if (gameData->AnimationTimer[i] > 1.0f) {
            gameData->AnimationTimer[i] = 0.0f;
            gameData->AnimationRectIndex[i] += 1;

            if (gameData->AnimationRectIndex[i] > 6) {
                gameData->AnimationRectIndex[i] = 0;
            }
        }
    }

    bool onGround[2] = { false };
    uint16_t groundY[2] = { 0 };
    
    bool againstWall = false; // if 1 char is against a wall, they are both stuck
    uint32_t wallX = 0; 

    Rectangle playerRecsForGround[2] = { (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[0] + gameData->TileSize - 10.0f + 0.5f, gameData->TileSize, 10.0f },
                                         (Rectangle) { gameData->PlayerPosX, gameData->PlayerPosY[1] - 0.5f, gameData->TileSize, 10.0f }};

    Rectangle playerRecsForWalls[2] = { (Rectangle) { gameData->PlayerPosX + 0.5f + gameData->TileSize - 10.0f, gameData->PlayerPosY[0] + 5.0f, 10.0f, gameData->TileSize - 10.0f },
                                        (Rectangle) { gameData->PlayerPosX + 0.5f + gameData->TileSize - 10.0f, gameData->PlayerPosY[1] + 5.0f, 10.0f, gameData->TileSize - 10.0f }};

    gameData->DebugRectangleCount = 0;

    gameData->DebugRectangles[gameData->DebugRectangleCount] = playerRecsForGround[0];
    gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = GREEN;
    gameData->DebugRectangleCount += 1;

    gameData->DebugRectangles[gameData->DebugRectangleCount] = playerRecsForWalls[0];
    gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = GREEN;
    gameData->DebugRectangleCount += 1;

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

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

            if (CheckCollisionRecs(playerRecsForGround[0], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount-1] = RED;
                onGround[0] = true; 
                groundY[0] = checkY;
                //break;
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

            if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;


            if (CheckCollisionRecs(playerRecsForGround[1], levelRect)) {
                //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;
                //TraceLog(LOG_ALL, "char 2 on floor!");
                onGround[1] = true;
                groundY[1] = checkY;
                //break;
            }
        }
    }
    
    // wall collision check char 1: check for 2 adjacing tiles to our right
    for (int offsetY = -1; offsetY < 2; offsetY++) {
        int charX = gameData->PlayerPosX / gameData->TileSize;
        int charY = (gameData->PlayerPosY[0] + gameData->TileSize * 0.95f) / gameData->TileSize;

        int checkX = charX + 1;
        int checkY = charY + offsetY;

        if (checkY < 0) continue;

        Rectangle levelRect = { (checkX * gameData->TileSize), checkY * gameData->TileSize + 2.0f, 10.0f, gameData->TileSize - 10.0f};
        gameData->DebugRectangles[gameData->DebugRectangleCount] = levelRect;
        gameData->DebugRectanglesColors[gameData->DebugRectangleCount] = RAYWHITE;
        gameData->DebugRectangleCount += 1;

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        if (CheckCollisionRecs(playerRecsForWalls[0], levelRect)) {
            gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;

            againstWall = true;
            wallX = checkX;
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

        if (levelData->Tiles[checkX + (checkY * levelData->LevelWidth)] != TILE_FLOOR) continue;

        if (CheckCollisionRecs(playerRecsForWalls[1], levelRect)) {
            //gameData->DebugRectanglesColors[gameData->DebugRectangleCount - 1] = RED;

            againstWall = true;
            wallX = checkX;
            break;
        }
    }

    const float playerMoveSpeed = 300.0f;

    gameData->PlayerPosX += againstWall ? 0.0f : playerMoveSpeed * dt; 

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
        for (int i = 0; i < 2; i++) {
            if (onGround[i]) {
                gameData->JumpVelocity[i] = 150.0f;
                gameData->GoingUp[i] = true;
                gameData->JumpTimer[i] = 0.0f;
                onGround[i] = false;
            }
        }
    }

    if (IsKeyDown(KEY_SPACE)) {
        for (int i = 0; i < 2; i++) {
            if (!onGround[i] && gameData->JumpTimer[i] < 0.4f) {
                gameData->JumpVelocity[i] += 350.0f * dt;
            }
        }
    }

    gameData->PlayerPosY[0] -= gameData->JumpVelocity[0] * dt;
    gameData->PlayerPosY[1] += gameData->JumpVelocity[1] * dt;

    float camSpeed = playerMoveSpeed;

    float cameraLagDistance = gameData->PlayerPosX - gameData->CameraPosX;  

    float catchupMultiplier = 1.0f;
    if (cameraLagDistance < 120) {
        camSpeed -= cameraLagDistance / 15.0f;
    }

    if (againstWall) { 
        camSpeed -= 100.0f;
    }

    gameData->CameraPosX += camSpeed * dt;


    Emitter_Set_Origin(gameData->EmitterConnectionDown, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX + gameData->TileSize / 2, gameData->PlayerPosY[0] + gameData->TileSize });
    Emitter_Set_Origin(gameData->EmitterConnectionUp, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX + gameData->TileSize / 2, gameData->PlayerPosY[1] });

    ParticleSystem_Update(gameData->PSConnection, dt);

    if (gameData->PlayerPosX - gameData->CameraPosX < 15) {
        game_restart(gameData, levelData);
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
                DrawRectangle(x * tileSize - gameData->CameraPosX-1, y * tileSize-1, tileSize+2, tileSize+2, gameColors[0]);
                break;
            default:
                break;
            }  
        }
    }

    // Draw char 1
    //DrawRectangleV((Vector2){ gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[0] }, (Vector2){ tileSize, tileSize }, gameColors[1]);
    DrawTextureRec(Char1Sheet, (Rectangle) { gameData->TileSize * gameData->AnimationRectIndex[0], 0, gameData->TileSize, gameData->TileSize }, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[0] }, WHITE);

    // Draw char 2
    //DrawRectangleV((Vector2){ gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[1] }, (Vector2){ tileSize, tileSize }, gameColors[2]);
    DrawTextureRec(Char2Sheet, (Rectangle) { gameData->TileSize * gameData->AnimationRectIndex[1], 0, gameData->TileSize, gameData->TileSize }, (Vector2) { gameData->PlayerPosX - gameData->CameraPosX, gameData->PlayerPosY[1] }, WHITE);

    ParticleSystem_Draw(gameData->PSConnection);

#if defined (_DEBUG)
    for (int i = 0; i < gameData->DebugRectangleCount; ++i) {
        Rectangle rect = gameData->DebugRectangles[i];
        rect.x -= gameData->CameraPosX;

        //DrawRectangleRec(rect, gameData->DebugRectanglesColors[i]);
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
    gameData->JumpTimer[0] = 0.0f;
    gameData->JumpTimer[1] = 0.0f;

    gameData->GoingUp[0] = false;
    gameData->GoingUp[1] = false;

    gameData->CameraPosX = 0.0f;

    gameData->BladeSawTimer = 0.0f;
    gameData->BladeSawRectIndex = 0;

    gameData->Timer = 0.0f;
}