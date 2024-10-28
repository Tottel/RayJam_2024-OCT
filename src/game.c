#include "game.h"
#include "utils.h"

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

void game_init(GameData* game) {
    game->ActiveColor = 2;
}

void game_exit(GameData* gameData) {

}

void game_tick(GameData* gameData, LevelData* levelData, float dt) {
    gameData->Timer += dt;

    if (gameData->Timer >= 1.0f) {
        gameData->Timer = 0.0f;

        gameData->ActiveColor += 1;

        if (gameData->ActiveColor > 7) {
            gameData->ActiveColor = 0;
        }
    }
}

void game_draw(GameData* gameData, LevelData* levelData, Color* gameColors, int screenWidth, int screenHeight) {
    //BeginTextureMode(renderTarget);
    //ClearBackground(RAYWHITE);
    ////
    //////// TODO: Draw your game screen here
    //DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, gameColors[gameData->ActiveColor]);
    ////
    //EndTextureMode();

    // Render to screen (main framebuffer)

    //DrawCircle(200, 200, 200, gameColors[gameData->ActiveColor]);
    const float cubeSize = screenHeight / (float)levelData->LevelHeight;

    for (uint16_t y = 0; y < levelData->LevelHeight; y++) {
        for (uint32_t x = 0; x < levelData->LevelWidth; x++) {
            uint16_t tileType = levelData->Tiles[x + (y * levelData->LevelWidth)];

            switch (tileType) {
            case 0: // void
                break;
            case 1: // walls/floors 
                DrawRectangle(x * cubeSize, y * cubeSize, cubeSize, cubeSize, gameColors[0]);
                break;
            default:
                break;
            }  
        }
    }

    //DrawText("YAYAYAYAYAYAYA", 100, 100, 20, GREEN);
    //TraceLog(LOG_DEBUG, "drawing");

    // Draw render texture to screen, scaled if required
    //DrawTexturePro(renderTarget.texture, (Rectangle) { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height }, (Rectangle) { 0, 0, (float)renderTarget.texture.width, (float)renderTarget.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    //EndDrawing();
}