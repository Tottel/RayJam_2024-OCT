#include "game.h"
#include "utils.h"

#include <assert.h>
#include <stddef.h>

void game_init(GameData* game) {
    game->ActiveColor = 2;
}

void game_exit(GameData* gameData) {

}

void game_tick(GameData* gameData, float dt) {
    gameData->Timer += dt;

    if (gameData->Timer >= 1.0f) {
        gameData->Timer = 0.0f;

        gameData->ActiveColor += 1;

        if (gameData->ActiveColor > 7) {
            gameData->ActiveColor = 0;
        }
    }
}

void game_draw(GameData* gameData, Color* gameColors, int screenWidth, int screenHeight) {
    //BeginTextureMode(renderTarget);
    //ClearBackground(RAYWHITE);
    ////
    //////// TODO: Draw your game screen here
    //DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, gameColors[gameData->ActiveColor]);
    ////
    //EndTextureMode();

    // Render to screen (main framebuffer)

    //DrawCircle(200, 200, 200, gameData->Palette_1[gameData->ActiveColor]);

    //DrawText("YAYAYAYAYAYAYA", 100, 100, 20, GREEN);
    //TraceLog(LOG_DEBUG, "drawing");

    // Draw render texture to screen, scaled if required
    //DrawTexturePro(renderTarget.texture, (Rectangle) { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height }, (Rectangle) { 0, 0, (float)renderTarget.texture.width, (float)renderTarget.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    //EndDrawing();
}