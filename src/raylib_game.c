/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "utils.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 
#include <stddef.h>
#include <assert.h>

#include "game.h"
#include "UISystem.h"

void emscripten_loop(void);

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 1280;
static const int screenHeight = 720;

static const int TargetFPS = 60;

static RenderTexture2D target = { 0 };  // Render texture to render our game

static Color gameColors[8];

static GameData* gameData = NULL;
static UIData* uiData = NULL;

void OnButtonClicked(void* context) {

}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_ALL);         // Disable raylib trace log messages
#else
    SetTraceLogLevel(LOG_ALL);         
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam - Korneel Guns");
    
    // TODO: Load resources / Initialize variables at this point
    
    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    // Load palette
    {
        Image temp = LoadImage("resources/palettes/custodian.png");
        assert(temp.data != NULL);
        if (temp.data != NULL) {
            int count = temp.width;
            assert(count == 8);

            Color* colors = NULL;
            colors = LoadImageColors(temp);

            memcpy(gameColors, colors, 8 * sizeof(Color));

            UnloadImageColors(colors);
            UnloadImage(temp);
        }
    }

    uiData = RL_CALLOC(1, sizeof(UIData));
    gameData = RL_CALLOC(1, sizeof(GameData));

    game_init(gameData);

    ui_add_button(uiData, 100, 100, 100, 100, "Hi", UIStyleButtonMainMenu, OnButtonClicked, NULL, true);



#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    SetTargetFPS(TargetFPS);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        emscripten_loop();
    }
#endif



    game_exit(gameData);
    ui_exit(uiData);

    RL_FREE(gameData);
    RL_FREE(uiData);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void emscripten_loop(void) {
    const float dt = GetFrameTime();

    game_tick(gameData, dt);
    ui_tick(uiData);

    BeginDrawing();
    game_draw(target, gameData, gameColors, screenWidth, screenHeight);
    ui_draw(uiData);
    EndDrawing();
}