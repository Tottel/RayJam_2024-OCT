#include "game.h"

#include <assert.h>

void game_init(GameData* game) {
    
    // Palette
    {
        Image temp = LoadImage("resources/palettes/custodian.png");
        assert(temp.data != NULL);
        if (temp.data != NULL) {
            int count = 0;
            game->Palette_1 = LoadImagePalette(temp, 8, &count);
            assert(count == 8);
            UnloadImage(temp);
        }
    }
}

void game_clean(GameData* game) {
    UnloadImagePalette(game->Palette_1);
}

void game_update(GameData* gameData) {

}

void game_draw(RenderTexture2D renderTarget, GameData* gameData, int screenWidth, int screenHeight) {
    BeginTextureMode(renderTarget);
    ClearBackground(RAYWHITE);

    // TODO: Draw your game screen here
    DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, gameData->Palette_1[0]);

    EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(renderTarget.texture, (Rectangle) { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height }, (Rectangle) { 0, 0, (float)renderTarget.texture.width, (float)renderTarget.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
}