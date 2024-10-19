#include "game.h"
#include "utils.h"

#include <assert.h>
#include <stddef.h>

void game_init(GameData* game) {
    // Palette
    {
        Image temp = LoadImage("resources/palettes/custodian.png");
        assert(temp.data != NULL);
        if (temp.data != NULL) {
            int count = temp.width;
            assert(count == 8);

            Color* palette = LoadImageColors(temp);

            for (int i = 0; i < count; ++i) {
                TraceLog(LOG_DEBUG, "Loading color: %u %u %u", palette[i].r, palette[i].g, palette[i].b);
                game->Palette_1[i] = palette[i];
                TraceLog(LOG_DEBUG, "Assigning color: %u %u %u", game->Palette_1[i].r, game->Palette_1[i].g, game->Palette_1[i].b);
            }
    
            UnloadImageColors(palette);
            UnloadImage(temp);
        }
    }
}

void game_update(GameData* gameData, float dt) {
    gameData->Timer += dt;

    if (gameData->Timer >= 1.0f) {
        gameData->Timer = 0.0f;

        gameData->ActiveColor += 1;
        TraceLog(LOG_DEBUG, "using color index: %i", gameData->ActiveColor);
        TraceLog(LOG_DEBUG, "Using color: %u %u %u", gameData->Palette_1[gameData->ActiveColor].r, gameData->Palette_1[gameData->ActiveColor].g, gameData->Palette_1[gameData->ActiveColor].b);

        if (gameData->ActiveColor > 7) {
            gameData->ActiveColor = 0;
        }
    }
}

void game_draw(RenderTexture2D renderTarget, GameData* gameData, int screenWidth, int screenHeight) {
    TraceLog(LOG_DEBUG, "Using color in draw: %u %u %u", gameData->Palette_1[gameData->ActiveColor].r, gameData->Palette_1[gameData->ActiveColor].g, gameData->Palette_1[gameData->ActiveColor].b);

    //BeginTextureMode(renderTarget);
    //ClearBackground(RAYWHITE);
    //
    ////// TODO: Draw your game screen here
    //DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, gameData->Palette_1[gameData->ActiveColor]);
    //
    //EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    //TraceLog(LOG_DEBUG, "drawing with color index: %i", gameData->ActiveColor);
    //TraceLog(LOG_DEBUG, "That is color: %u %u %u", gameData->Palette_1[gameData->ActiveColor].r, gameData->Palette_1[gameData->ActiveColor].g, gameData->Palette_1[gameData->ActiveColor].b);

    ClearBackground(gameData->Palette_1[gameData->ActiveColor]);

    //DrawCircle(200, 200, 200, gameData->Palette_1[gameData->ActiveColor]);

    DrawText("YAYAYAYAYAYAYA", 100, 100, 20, GREEN);
    //TraceLog(LOG_DEBUG, "drawing");

    // Draw render texture to screen, scaled if required
    //DrawTexturePro(renderTarget.texture, (Rectangle) { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height }, (Rectangle) { 0, 0, (float)renderTarget.texture.width, (float)renderTarget.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
}