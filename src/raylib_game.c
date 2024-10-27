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
#include "image_color_parser.h"

void app_loop(void);

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

static Color gameColors[8];

static GameScreen CurrentState = { SCREEN_MENU };

// menu state
static UIData* UIDataMenu = NULL;
static UIData* UIDataMenuInstructions = NULL;
static Texture2D UIInstructionTexture1;

// game state
static GameData* gameData = NULL;
static UIData* UIDataGame = NULL;

void OnPlayButtonClicked(void* context) {
    CurrentState = SCREEN_GAMEPLAY;
}

void OnHelpButtonClicked(void* context) {
    CurrentState = SCREEN_MENU_INSTRUCTIONS;
}

void OnInstructionBackButtonClicked(void* context) {
    CurrentState = SCREEN_MENU;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#else
    SetTraceLogLevel(LOG_ALL);         
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam - Korneel Guns");
    
    // Data/Resource initialization scope
    {
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

        UIInstructionTexture1 = load_and_convert_texture("resources/images/panda_fail.jpg", gameColors, 8);

        const uint16_t buttonWidth = 120;
        const uint16_t buttonHeight = 50;

        // menu state
        UIDataMenu = RL_CALLOC(1, sizeof(UIData));
        ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 140, buttonWidth, buttonHeight, "play", UIStyleButtonMainMenu, OnPlayButtonClicked, NULL, true);
        ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "help", UIStyleButtonMainMenu, OnHelpButtonClicked, NULL, true);

        // menu instructions state
        UIDataMenuInstructions = RL_CALLOC(1, sizeof(UIData));
        ui_add_rectangle_with_text(UIDataMenuInstructions, screenWidth / 2 - 300, screenHeight / 2 - 100, 600, 200, 0, "These are instructions. \nJust eh.. You know.. Do stuff.. \nWell, okay, how about YOU write instructions then?!", 20, ALIGN_HOR_LEFT, ALIGN_VER_TOP, 4);
        ui_add_button(UIDataMenuInstructions, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "back", UIStyleButtonMainMenu, OnInstructionBackButtonClicked, NULL, true);
        ui_add_rectangle_with_texture(UIDataMenuInstructions, 100, 45, 100, 75, 0, UIInstructionTexture1, true, ALIGN_HOR_LEFT, ALIGN_VER_TOP);
        ui_add_rectangle_with_texture(UIDataMenuInstructions, 210, 45, 100, 75, 0, UIInstructionTexture1, true, ALIGN_HOR_LEFT, ALIGN_VER_TOP);

        // game state
        gameData = RL_CALLOC(1, sizeof(GameData));
        UIDataGame = RL_CALLOC(1, sizeof(UIData));   
    }

    game_init(gameData);

    //--------------------------------------------------------------------------------------
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(app_loop, 0, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        app_loop();
    }
#endif
    //--------------------------------------------------------------------------------------

    game_exit(gameData);
    ui_exit(UIDataGame);
    ui_exit(UIDataMenu);
    ui_exit(UIDataMenuInstructions);

    UnloadTexture(UIInstructionTexture1);

    RL_FREE(gameData);
    RL_FREE(UIDataGame);
    RL_FREE(UIDataMenu);
    RL_FREE(UIDataMenuInstructions);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void app_loop(void) {
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
    case SCREEN_MENU_INSTRUCTIONS: {
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
        game_draw(gameData, gameColors, screenWidth, screenHeight);
        ui_draw(UIDataGame, gameColors);
        EndDrawing();
    } break;
    default:
        assert(false); // Should probably implement this state
        break;
    }
}