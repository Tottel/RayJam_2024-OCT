#ifndef UISYSTEM_H
#define UISYSTEM_H

#include <raylib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_RECTANGLES 8
#define MAX_BUTTONS 4

static const float ALIGNMENT_TABLE[3] = { 0.0f, 0.5f, 1.0f };

typedef enum UIAlignmentHorizontal {
    ALIGN_HOR_LEFT = 0,
    ALIGN_HOR_CENTER,
    ALIGN_HOR_RIGHT,
    ALIGN_HOR_COUNT
} UIAlignmentHorizontal;

typedef enum UIAlignmentVertical {
    ALIGN_VER_TOP = 0,
    ALIGN_VER_CENTER,
    ALIGN_VER_BOT,
    ALIGN_VER_COUNT
} UIAlignmentVertical;

typedef struct UIButtonHandleValue {
    bool Enabled; // Tells us to look in enabled or disabled buttons array
    uint16_t ButtonID; // Index into array
} UIButtonHandleValue;

typedef struct UIButton {
    uint16_t RectIndex;
    uint8_t ColorRectDefault;
    uint8_t ColorRectHover;
    uint8_t ColorRectClick;
    uint8_t ColorRectDisabled;
    uint8_t ColorTextDefault;
    uint8_t ColorTextHover;
    uint8_t ColorTextClick;
    uint8_t ColorTextDisabled;

    void (*button_clicked)(void*);
    void* Context;
    //int Data;
} UIButton;

typedef struct UIButtonEnabled {
    uint16_t RectIndex;
    uint8_t ColorRectDefault;
    uint8_t ColorRectHover;
    uint8_t ColorRectClick;
    uint8_t ColorTextDefault;
    uint8_t ColorTextHover;
    uint8_t ColorTextClick;
} UIButtonEnabled;

typedef struct UIButtonDisabled {
    uint16_t RectIndex;
    uint8_t ColorRectDisabled;
    uint8_t ColorTextDisabled;
} UIButtonDisabled;

typedef struct UIButtonClickFunctionality {
    uint16_t RectIndex;

    void (*button_clicked)(void*);
    void* Context;
    //int Data;
} UIButtonClickFunctionality;

typedef struct UIRectangle {
    uint16_t PosX;
    uint16_t PosY;
    uint16_t Width;
    uint16_t Height;
    uint8_t ColorIndex;
} UIRectangle;

typedef struct UIText {
    const char* Text;
    uint16_t PosX;
    uint16_t PosY;
    int FontSize; // INT OR FLOAT?!
    uint8_t ColorIndex;
} UIText;

typedef struct UIStyleText {
    UIAlignmentHorizontal TextAlignmentHorizontal;
    UIAlignmentVertical TextAlignmentVertical;
    int FontSize; // INT OR FLOAT?!

    uint8_t ColorTextDefault;
} UIStyleText;

typedef struct UIStyleButton {
    UIStyleText TextStyle;

    uint8_t ColorRectDefault;
    uint8_t ColorRectHover;
    uint8_t ColorRectClick;
    uint8_t ColorRectDisabled;
 
    uint8_t ColorTextHover;
    uint8_t ColorTextClick;
    uint8_t ColorTextDisabled;
} UIStyleButton;

static struct UIStyleText UIStyleTextInstructions = {
    ALIGN_HOR_LEFT,
    ALIGN_VER_TOP,
    20,
    4
};

static struct UIStyleText UIStyleTextInGameInstructions = {
    ALIGN_HOR_CENTER,
    ALIGN_VER_CENTER,
    20,
    4
};

static struct UIStyleButton UIStyleButtonMainMenu = {
    { ALIGN_HOR_CENTER, ALIGN_VER_CENTER, 20, 4 },
    0,
    1,
    2,
    3,

    5,
    6,
    7,
};

static struct UIStyleText UIStyleTitleMainMenu = {
    ALIGN_HOR_CENTER,
    ALIGN_VER_CENTER,
    110,
    1
};

typedef struct UIData {
	UIRectangle Rectangles[MAX_RECTANGLES];
    UIText RectanglesText[MAX_RECTANGLES];
    uint16_t RectangleCount;

    UIButton ButtonsAll[MAX_BUTTONS];
    UIButtonEnabled ButtonsEnabled[MAX_BUTTONS];
    UIButtonDisabled ButtonsDisabled[MAX_BUTTONS];
    UIButtonClickFunctionality ButtonsFunctionalities[MAX_BUTTONS];

    UIButtonHandleValue ButtonHandleValues[MAX_BUTTONS];

    uint16_t ButtonsEnabledCount;
    uint16_t ButtonsDisabledCount;
} UIData;

void ui_exit(UIData* uiData);
void ui_tick(UIData* uiData);
void ui_draw(UIData* uiData, Color* gameColors);

uint16_t ui_add_rectangle(UIData* uiData, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height, uint8_t rectColor);
uint16_t ui_add_rectangle_with_text(UIData* uiData, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height, uint8_t rectColor, const char* text, UIStyleText textStyle);
uint16_t ui_add_button(UIData* uiData, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height, const char* text, UIStyleButton uiStyle, void(*button_clicked)(void*), void* context, bool enabled);

void ui_remove_all(UIData* uiData);

void ui_toggle_button(UIData* uiData, uint16_t buttonHandle, bool enabled);

#endif