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
#include "menu_game.h"
#include "level_parser.h"
#include "UISystem.h"
#include "image_color_parser.h"

void app_loop(void);
void draw_parallax(void);
void go_to_next_level(void);

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
    SCREEN_GAMEPLAY_LEVEL_TRANSITION,
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

static int CurrentLevel = 0;
static float CurrentStateTimer = 0.0f;
static bool DoIntroSlide = false;

// menu state
static UIData* UIDataMenu = NULL;
static UIData* UIDataMenuInstructions = NULL;

// game intro state
static GameIntroSteps IntroSubState = { INTRO_SLIDE_1 }; 
static UIData* UIDataGameIntro = NULL;

// game state
static GameData* gameData = NULL;
static UIData* UIDataGame = NULL; 

static Texture2D BladeSaw;

static LevelData* levelData = NULL;

#if defined (_DEBUG)
static float slowMoMultiplier = 1.0f;
#endif

static Texture2D CavePar1;
static Texture2D CavePar2;
static Texture2D CavePar3;
 
static Texture2D WoodsPar1;
static Texture2D WoodsPar2;

static Sound MainTheme;

void OnPlayButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_GAMEPLAY_INTRO;
    IntroSubState = INTRO_SLIDE_1;
    CurrentStateTimer = 0.0f;
    CurrentLevel = 0;
    DoIntroSlide = true;

    go_to_next_level();
}

void OnHelpButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_MENU_INSTRUCTIONS;
    CurrentStateTimer = 0.0f;
}

void OnInstructionBackButtonClicked(void* context) {
    (void)context;

    CurrentState = SCREEN_MENU;
    CurrentStateTimer = 0.0f;
}

void OnInGameInstructionButtonClicked(void* context) {
    (void)context;

    IntroSubState = INTRO_SLIDE_2;
    CurrentStateTimer = 0.0f;
    DoIntroSlide = false;

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
    InitWindow(screenWidth, screenHeight, "Korneel Guns: Tethered (RayJam 2024)");
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
            SetTextureWrap(CavePar1, TEXTURE_WRAP_REPEAT);
            SetTextureWrap(CavePar2, TEXTURE_WRAP_REPEAT);
            SetTextureWrap(CavePar3, TEXTURE_WRAP_REPEAT);

            WoodsPar1 = load_and_convert_texture("resources/images/parallax/demon-woods/far.png", gameColors, 8);
            WoodsPar2 = load_and_convert_texture("resources/images/parallax/demon-woods/close.png", gameColors, 8);   
            SetTextureWrap(WoodsPar1, TEXTURE_WRAP_REPEAT);
            SetTextureWrap(WoodsPar2, TEXTURE_WRAP_REPEAT);
        }

        const uint16_t buttonWidth = 120;
        const uint16_t buttonHeight = 50;

        // menu state
        UIDataMenu = RL_CALLOC(1, sizeof(UIData));
        ui_add_rectangle_with_text(UIDataMenu, 70, 75, 660, 200, 4, "TETHERED", UIStyleTitleMainMenu);
        ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 140, buttonWidth, buttonHeight, "play", UIStyleButtonMainMenu, OnPlayButtonClicked, NULL, true);
        ui_add_button(UIDataMenu, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "help", UIStyleButtonMainMenu, OnHelpButtonClicked, NULL, true);

        // menu instructions state
        UIDataMenuInstructions = RL_CALLOC(1, sizeof(UIData));
        ui_add_rectangle_with_text(UIDataMenuInstructions, screenWidth / 2 - 300, screenHeight / 2 - 130, 600, 250, 0, "Instructions? Why would you need instructions?\n\nIt's so simple: You are two characters at the same time!\nAll you have to do is jump [SPACE] and they both jump.\n\nOkay, yes, you will also find a gun later on, and only one\nof your characters can have it. But don't worry!\nYou can pass it between them with [SHIFT].\n\nAnd you probably also want to know that you can\nfire that gun with [CTRL]..\n\nNow go!", UIStyleTextInstructions);
        ui_add_button(UIDataMenuInstructions, screenWidth / 2 - buttonWidth / 2, screenHeight - 80, buttonWidth, buttonHeight, "back", UIStyleButtonMainMenu, OnInstructionBackButtonClicked, NULL, true);

        // game intro state
        UIDataGameIntro = RL_CALLOC(1, sizeof(UIData));
        uint16_t rectWidth = 480;
        uint16_t rectHeight = 200;
        uint16_t rectPosX = screenWidth / 2 - 150;
        uint16_t rectPosY = screenHeight / 2 - 100;
        ui_add_rectangle(UIDataGameIntro, screenWidth / 2 - 155, screenHeight / 2 - 105, 490, 210, 1);
        ui_add_rectangle_with_text(UIDataGameIntro, rectPosX, rectPosY, rectWidth, rectHeight, 0, "You might be wondering:\n\nHow did I get here? Who am I? ...Who are we?\n\nThose are all great questions.\n\n... But I suggest you run.", UIStyleTextInGameInstructions);
        ui_add_button(UIDataGameIntro, rectPosX + rectWidth / 2 - buttonWidth/2, rectPosY + rectHeight + 20, buttonWidth, buttonHeight, "Yes", UIStyleButtonMainMenu, OnInGameInstructionButtonClicked, NULL, true);

        // game state
        gameData        = RL_CALLOC(1, sizeof(GameData));
        UIDataGame      = RL_CALLOC(1, sizeof(UIData));
        levelData       = RL_CALLOC(1, sizeof(LevelData));

        MainTheme = LoadSound("resources/music/relax_and_chill.mp3");
    }
    
    parse_level("resources/levels/level_1.txt", levelData); // Preload
    game_create(gameData, levelData, gameColors, screenWidth, screenHeight);
    game_menu_init(gameData, screenWidth, screenHeight);

    PlaySound(MainTheme);

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

    UnloadSound(MainTheme);

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
        game_menu_tick(gameData, screenWidth, screenHeight, dt);
        ui_tick(UIDataMenu);

        BeginDrawing();
        ClearBackground(gameColors[4]);
        ui_draw(UIDataMenu, gameColors);
        game_menu_draw(gameData, gameColors);
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
            //if (CurrentStateTimer > 0.1f) {
                CurrentState = SCREEN_GAMEPLAY;     
                CurrentStateTimer = -2.0f;

                //PlaySound(gameData->FirstSpawn);
            //}
            break;
        default:
            assert(false);
            break;
        }

        ui_tick(UIDataGameIntro);

        BeginDrawing();
        ClearBackground(gameColors[5]);

        draw_parallax();
        game_draw(gameData, levelData, gameColors);

        if (IntroSubState == INTRO_SLIDE_2 && CurrentStateTimer > 0.5f) {
            game_bladesaws_draw(gameData, BladeSaw, dt); 
        }

        ui_draw(UIDataGameIntro, gameColors); 

        if (DoIntroSlide && CurrentStateTimer < 0.5f) {
            DrawRectangle(CurrentStateTimer * screenWidth * 2.3f, 0, screenWidth, screenHeight, gameColors[0]);
        }

        EndDrawing();

    } break;
    case SCREEN_GAMEPLAY: {
#if defined(_DEBUG)
        if (IsKeyPressed(KEY_R)) {
            game_restart(gameData, levelData);
        }
#endif

        if (CurrentStateTimer > 0.5f) {
            game_tick(gameData, levelData, screenWidth, screenHeight, dt);
        }

        ui_tick(UIDataGame);

        BeginDrawing();
        ClearBackground(gameColors[5]);

        draw_parallax();
        game_draw(gameData, levelData, gameColors); 
        game_bladesaws_draw(gameData, BladeSaw, dt);
        ui_draw(UIDataGame, gameColors);

        if (DoIntroSlide && CurrentStateTimer < 0.5f) {
            DrawRectangle(CurrentStateTimer * screenWidth * 2.3f, 0, screenWidth, screenHeight, gameColors[0]);
        }

        //DrawFPS(10, 10);
        EndDrawing();

        if (gameData->NextLevel) {
            CurrentState = SCREEN_GAMEPLAY_LEVEL_TRANSITION;
            CurrentStateTimer = 0.0f;
        }

        if (gameData->RestartLevel) {
            gameData->RestartLevel = false;

            PlaySound(gameData->Respawn);

            CurrentStateTimer = 0.0f;

            game_restart(gameData, levelData);
        }

    } break;
    case SCREEN_GAMEPLAY_LEVEL_TRANSITION: {
        game_tick(gameData, levelData, screenWidth, screenHeight, dt);

        BeginDrawing();
        ClearBackground(gameColors[5]);
        draw_parallax();
        game_draw(gameData, levelData, gameColors);
        DrawRectangle(0, 0, CurrentStateTimer * screenWidth * 1.8f, screenHeight, gameColors[0]);
        //DrawFPS(10, 10);
        EndDrawing();

        if (CurrentStateTimer > 1.2f) {
            CurrentState = SCREEN_GAMEPLAY;
            CurrentStateTimer = 0.0f;
            gameData->NextLevel = false;
            DoIntroSlide = true;

            go_to_next_level();
        }

    } break;
    default:
        assert(false); // Should probably implement this state
        break;
    }

    CurrentStateTimer += dt;

    if (!IsSoundPlaying(MainTheme)) {
        PlaySound(MainTheme);
    }
}

void draw_parallax(void) {
    {
        // aspect ratio is ~4.35
        Rectangle dest = (Rectangle){ 0, 0, screenWidth, screenHeight / 2 };

        Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.1f, 60, 230 * 4.35f, 180 };
        DrawTexturePro(WoodsPar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

        Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.3f, 60, 230 * 4.35f, 180 };
        DrawTexturePro(WoodsPar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
    }

    {
        // aspect ratio is ~3.56
        Rectangle dest = (Rectangle){ 0, screenHeight / 2, screenWidth, screenHeight / 2 };

        Rectangle source1 = (Rectangle){ gameData->CameraPosX * 0.05f, 30, 1080 * 3.556f, 1080 };
        DrawTexturePro(CavePar1, source1, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

        Rectangle source2 = (Rectangle){ gameData->CameraPosX * 0.25f, 120, 830 * 3.556f, 830 };
        DrawTexturePro(CavePar2, source2, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);

        Rectangle source3 = (Rectangle){ gameData->CameraPosX * 0.9f, 70, 900 * 3.556f, 900 };
        DrawTexturePro(CavePar3, source3, dest, (Vector2) { 0, 0 }, 0.0f, WHITE);
    }
}

void go_to_next_level(void) {
    CurrentLevel += 1;
    switch(CurrentLevel) {
    case 1:
        assert(levelData != NULL);
        //parse_level("resources/levels/level_1.txt", levelData); // We have already pre-loaded it
        break;
    case 2:
        parse_level("resources/levels/level_2.txt", levelData);
        break;
    case 3:
        parse_level("resources/levels/level_3.txt", levelData);
        break;
    case 4:
        parse_level("resources/levels/level_4.txt", levelData);
        break;
    case 5:
        parse_level("resources/levels/level_5.txt", levelData);
        break;
    }
    
    game_restart(gameData, levelData);
}