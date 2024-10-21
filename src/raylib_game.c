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
    SCREEN_MENU,
    SCREEN_MENU_INSTRUCTIONS,
    SCREEN_GAMEPLAY, 
} GameScreen;

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const uint16_t screenWidth = 800;
static const uint16_t screenHeight = 450;

static const int TargetFPS = 60;

static RenderTexture2D target = { 0 };  // Render texture to render our game

static Color gameColors[8];

static GameScreen CurrentState = { SCREEN_MENU };
static GameScreen NextState = { SCREEN_MENU };

// menu state
static UIData* UIDataMenu = NULL;
static UIData* UIDataMenuInstructions = NULL;

// game state
static GameData* gameData = NULL;
static UIData* UIDataGame = NULL;

void OnPlayButtonClicked(void* context) {
    NextState = SCREEN_GAMEPLAY;
}

void OnHelpButtonClicked(void* context) {
    NextState = SCREEN_MENU_INSTRUCTIONS;
}

void OnInstructionBackButtonClicked(void* context) {
    NextState = SCREEN_MENU;
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

    const uint16_t buttonWidth = 120;
    const uint16_t buttonHeight = 50;

    // menu state
    UIDataMenu = RL_CALLOC(1, sizeof(UIData));
    ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 140, buttonWidth, buttonHeight, "play", UIStyleButtonMainMenu, OnPlayButtonClicked, NULL, true);
    ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "help", UIStyleButtonMainMenu, OnHelpButtonClicked, NULL, true);

    // menu instructions state
    UIDataMenuInstructions = RL_CALLOC(1, sizeof(UIData));
    ui_add_rectangle_with_text(UIDataMenuInstructions, screenWidth / 2 - 250, screenHeight / 2 - 100, 500, 200, 0, "This is an instruction", 20, ALIGN_HOR_LEFT, ALIGN_VER_CENTER, 4);
    ui_add_button(UIDataMenuInstructions, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "back", UIStyleButtonMainMenu, OnInstructionBackButtonClicked, NULL, true);

    // game state
    gameData = RL_CALLOC(1, sizeof(GameData));
    UIDataGame = RL_CALLOC(1, sizeof(UIData));

    game_init(gameData);

    //--------------------------------------------------------------------------------------
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    SetTargetFPS(TargetFPS);     // Set our game frames-per-second

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        emscripten_loop();
    }
#endif
    //--------------------------------------------------------------------------------------


    game_exit(gameData);
    ui_exit(UIDataGame);
    ui_exit(UIDataMenu);
    ui_exit(UIDataMenuInstructions);

    RL_FREE(gameData);
    RL_FREE(UIDataGame);
    RL_FREE(UIDataMenu);
    RL_FREE(UIDataMenuInstructions);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void emscripten_loop(void) {
    if (CurrentState != NextState) {
        CurrentState = NextState;
    }

    const float dt = GetFrameTime();

    switch (CurrentState) {
    case SCREEN_LOGO:
        break;
    case SCREEN_TITLE:
        break;
    case SCREEN_MENU: {
        ui_tick(UIDataMenu);

        BeginDrawing();
        ClearBackground(gameColors[4]);
        ui_draw(UIDataMenu, gameColors);
        EndDrawing();
    } break;
    case SCREEN_MENU_INSTRUCTIONS:
    {
        ui_tick(UIDataMenuInstructions);

        BeginDrawing();
        ClearBackground(gameColors[4]);
        ui_draw(UIDataMenuInstructions, gameColors);
        EndDrawing();
    } break;
    case SCREEN_GAMEPLAY: {
        game_tick(gameData, dt);
        ui_tick(UIDataGame);

        BeginDrawing();
        ClearBackground(gameColors[4]);
        game_draw(target, gameData, gameColors, screenWidth, screenHeight);
        ui_draw(UIDataGame, gameColors);
        EndDrawing();
    } break;
    default:
        break;
    }
}