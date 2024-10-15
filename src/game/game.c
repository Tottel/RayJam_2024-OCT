#include "game.h"

void game_update(void) {

}

void game_draw(RenderTexture2D renderTarget, int screenWidth, int screenHeight) {
    BeginTextureMode(renderTarget);
    ClearBackground(RAYWHITE);

    // TODO: Draw your game screen here
    DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, SKYBLUE);

    EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(renderTarget.texture, (Rectangle) { 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height }, (Rectangle) { 0, 0, (float)renderTarget.texture.width, (float)renderTarget.texture.height }, (Vector2) { 0, 0 }, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
}