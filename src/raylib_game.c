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


// Game jam idea for the theme: "Connections"
// A platformer-style infinite-runner game with two characters vertically mirrored across the middle of the screen.
// The characters have to run (auto?) to the right in unison (horizontal movement is identical), but the characters need certain abilities/attributes that
// can only be active on 1 character at a time. 
// e.g.: jumping - shooting - ...
// Player has to press a keyboard-key to pass an ability to the other character before they can use it!
// - Certain buttons need to be shot at to open something for the other char?

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
#include "level_parser.h"
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
    SCREEN_GAMEPLAY_INTRO,
    SCREEN_GAMEPLAY, 
} GameScreen;

typedef enum {
    INTRO_SLIDE_1 = 0,
    INTRO_SLIDE_2
} GameIntroSteps;

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

// game intro state
static GameIntroSteps IntroSubState = { INTRO_SLIDE_1 };
static UIData* UIDataGameIntro = NULL;
static float IntroTimer = 0.0f;

// game state
static GameData* gameData = NULL;
static UIData* UIDataGame = NULL;

static Texture2D BladeSaw;

static LevelData* levelData = NULL;

static float slowMoMultiplier = 1.0f;

static Texture2D CavePar1;
static Texture2D CavePar2;
static Texture2D CavePar3;

static Texture2D WoodsPar1;
static Texture2D WoodsPar2;


void OnPlayButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_GAMEPLAY_INTRO;
    IntroSubState = INTRO_SLIDE_1;
    IntroTimer = 0.0f;
}

void OnHelpButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_MENU_INSTRUCTIONS;
}

void OnInstructionBackButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_MENU;
}

void OnInGameInstructionButtonClicked(void* context) {
    (void)context;

    IntroSubState = INTRO_SLIDE_2;
    IntroTimer = 0.0f;

    ui_remove_all(UIDataGameIntro);
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
    InitAudioDevice();
    
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

            BladeSaw = LoadTexture("resources/images/bladesaw.png");

            CavePar1 = load_and_convert_texture("resources/images/parallax/cave/2.png", gameColors, 8);
            CavePar2 = load_and_convert_texture("resources/images/parallax/cave/4.png", gameColors, 8);
            CavePar3 = load_and_convert_texture("resources/images/parallax/cave/7.png", gameColors, 8);
            SetTextureWrap(CavePar1, TEXTURE_WRAP_MIRROR_REPEAT);
            SetTextureWrap(CavePar2, TEXTURE_WRAP_MIRROR_REPEAT);
            SetTextureWrap(CavePar3, TEXTURE_WRAP_MIRROR_REPEAT);

            WoodsPar1 = load_and_convert_texture("resources/images/parallax/demon-woods/far.png", gameColors, 8);
            WoodsPar2 = load_and_convert_texture("resources/images/parallax/demon-woods/close.png", gameColors, 8);   
            SetTextureWrap(WoodsPar1, TEXTURE_WRAP_MIRROR_REPEAT);
            SetTextureWrap(WoodsPar2, TEXTURE_WRAP_MIRROR_REPEAT);
        }

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
        //ui_add_rectangle_with_texture(UIDataMenuInstructions, 100, 45, 100, 75, 0, UIInstructionTexture1, true, ALIGN_HOR_LEFT, ALIGN_VER_TOP);
        //ui_add_rectangle_with_texture(UIDataMenuInstructions, 210, 45, 100, 75, 0, UIInstructionTexture1, true, ALIGN_HOR_LEFT, ALIGN_VER_TOP);

        // game intro state
        UIDataGameIntro = RL_CALLOC(1, sizeof(UIData));
        uint16_t rectWidth = 480;
        uint16_t rectHeight = 200;
        uint16_t rectPosX = screenWidth / 2 - 150;
        uint16_t rectPosY = screenHeight / 2 - 100;
        ui_add_rectangle(UIDataGameIntro, screenWidth / 2 - 155, screenHeight / 2 - 105, 490, 210, 1);
        ui_add_rectangle_with_text(UIDataGameIntro, rectPosX, rectPosY, rectWidth, rectHeight, 0, "You might be wondering:\n\nHow did I get here? Who am I? ...Who are we?\n\nThose are all great questions.\n\n... But I suggest you run.", 20, ALIGN_HOR_CENTER, ALIGN_VER_CENTER, 4);
        ui_add_button(UIDataGameIntro, rectPosX + rectWidth / 2 - buttonWidth/2, rectPosY + rectHeight + 20, buttonWidth, buttonHeight, "Yes", UIStyleButtonMainMenu, OnInGameInstructionButtonClicked, NULL, true);

        // game state
        gameData        = RL_CALLOC(1, sizeof(GameData));
        UIDataGame      = RL_CALLOC(1, sizeof(UIData));     
        levelData       = RL_CALLOC(1, sizeof(LevelData));

        parse_level("resources/levels/level_1.txt", levelData);
    }

    game_init(gameData, levelData, gameColors, screenWidth, screenHeight);

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
  
    UnloadTexture(BladeSaw);
    UnloadTexture(CavePar1);
    UnloadTexture(CavePar2);
    UnloadTexture(CavePar3);
    UnloadTexture(WoodsPar1);
    UnloadTexture(WoodsPar2);

    game_exit(gameData);
    ui_exit(UIDataGame);
    ui_exit(UIDataGameIntro);
    ui_exit(UIDataMenu);
    ui_exit(UIDataMenuInstructions);

    RL_FREE(levelData);
    RL_FREE(gameData);
    RL_FREE(UIDataGame);
    RL_FREE(UIDataGameIntro);
    RL_FREE(UIDataMenu);
    RL_FREE(UIDataMenuInstructions);

    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void app_loop(void) {
    float dt = GetFrameTime();

    // hacky speedup
    dt *= 1.3f;

#if defined (_DEBUG)
    if (dt > 0.5f) dt = 0.5f;

    slowMoMultiplier += GetMouseWheelMove() * 0.05f;
    slowMoMultiplier -= IsKeyPressed(KEY_O) * 0.1f;
    slowMoMultiplier += IsKeyPressed(KEY_P) * 0.1f;

    dt *= slowMoMultiplier;
#endif

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
    case SCREEN_GAMEPLAY_INTRO: {
        switch (IntroSubState) {
        case INTRO_SLIDE_1:
            break;
        case INTRO_SLIDE_2:
            IntroTimer += dt;

            if (IntroTimer > 2.0f) {
                CurrentState = SCREEN_GAMEPLAY; 
            }
            break;
        default:
            assert(false);
            break;
        }

        ui_tick(UIDataGameIntro);

        BeginDrawing();
        ClearBackground(gameColors[5]);

        {
            // aspect ratio is ~4.35
            Rectangle dest = (Rectangle){ 0, 0, screenWidth, screenHeight / 2 };

            Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.01f, 60, 230 * 4.35f, 180 };
            DrawTexturePro(WoodsPar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.3f, 60, 230 * 4.35f, 180 };
            DrawTexturePro(WoodsPar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }

        {
            // aspect ratio is ~3.56
            Rectangle dest = (Rectangle){ 0, screenHeight / 2, screenWidth, screenHeight / 2 };
            
            Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.002f, 30, 1080 * 3.556f, 1080 };
            DrawTexturePro(CavePar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.07f, 120, 830 * 3.556f, 830 };
            DrawTexturePro(CavePar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source3 = (Rectangle){ gameData->CameraPosX * 0.9f, 70, 900 * 3.556f, 900 };
            DrawTexturePro(CavePar3, source3, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }

        game_draw(gameData, levelData, gameColors);

        if (IntroSubState == INTRO_SLIDE_2 && IntroTimer > 0.5f) {
            game_bladesaws_draw(gameData, BladeSaw, dt); 
        }

        ui_draw(UIDataGameIntro, gameColors); 
        EndDrawing();

    } break;
    case SCREEN_GAMEPLAY: {
#if defined(_DEBUG)
        if (IsKeyPressed(KEY_R)) {
            game_restart(gameData, levelData);
        }
#endif

        game_tick(gameData, levelData, screenWidth, screenHeight, dt);
        ui_tick(UIDataGame);

        BeginDrawing();
        ClearBackground(gameColors[5]);

        {
            // aspect ratio is ~4.35
            Rectangle dest = (Rectangle){ 0, 0, screenWidth, screenHeight / 2 };

            Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.01f, 60, 230 * 4.35f, 180 };
            DrawTexturePro(WoodsPar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.3f, 60, 230 * 4.35f, 180 };
            DrawTexturePro(WoodsPar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }

        {
            // aspect ratio is ~3.56
            Rectangle dest = (Rectangle){ 0, screenHeight / 2, screenWidth, screenHeight / 2 };

            Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.002f, 30, 1080 * 3.556f, 1080 };
            DrawTexturePro(CavePar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.07f, 120, 830 * 3.556f, 830 };
            DrawTexturePro(CavePar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

            Rectangle source3 = (Rectangle){ gameData->CameraPosX * 0.9f, 70, 900 * 3.556f, 900 };
            DrawTexturePro(CavePar3, source3, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
        }

        game_draw(gameData, levelData, gameColors);
        game_bladesaws_draw(gameData, BladeSaw, dt);
        ui_draw(UIDataGame, gameColors);
        DrawFPS(10, 10);
        EndDrawing();
    } break;
    default:
        assert(false); // Should probably implement this state
        break;
    }
}