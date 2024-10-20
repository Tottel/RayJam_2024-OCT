#ifndef UISYSTEM_H
#define UISYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

#define MAX_RECTANGLES 100
#define MAX_BUTTONS 50

static const float ALIGNMENT_TABLE[3] = { 0.0f, 0.5f, 1.0f };

typedef enum UIAlignmentHorizontal {
    ALIGN_HOR_LEFT = 0,
    ALIGN_HOR_CENTER,
    ALIGN_HOR_RIGHT
} UIAlignmentHorizontal;

typedef enum UIAlignmentVertical {
    ALIGN_VER_TOP = 0,
    ALIGN_VER_CENTER,
    ALIGN_VER_BOT
} UIAlignmentVertical;

typedef struct UIButtonHandleValue {
    bool Enabled; // Tells us to look in enabled or disabled buttons array
    uint16_t ButtonID; // Index into array
} UIButtonHandleValue;

typedef struct UIButton {
    uint16_t RectIndex;
    Color ColorRectDefault;
    Color ColorRectHover;
    Color ColorRectClick;
    Color ColorRectDisabled;
    Color ColorTextDefault;
    Color ColorTextHover;
    Color ColorTextClick;
    Color ColorTextDisabled;

    void (*button_clicked)(void*, int);
    void* Context;
    int Data;
} UIButton;

typedef struct UIButtonEnabled {
    uint16_t RectIndex;
    Color ColorRectDefault;
    Color ColorRectHover;
    Color ColorRectClick;
    Color ColorTextDefault;
    Color ColorTextHover;
    Color ColorTextClick;
} UIButtonEnabled;

typedef struct UIButtonDisabled {
    uint16_t RectIndex;
    Color ColorRectDisabled;
    Color ColorTextDisabled;
} UIButtonDisabled;

typedef struct UIButtonClickFunctionality {
    uint16_t RectIndex;

    void (*button_clicked)(void*, int);
    void* Context;
    //int Data;
} UIButtonClickFunctionality;

typedef struct UIRectangle {
    uint16_t PosX;
    uint16_t PosY;
    uint16_t Width;
    uint16_t Height;
    Color Color;
} UIRectangle;

typedef struct UIText {
    const char* Text;
    uint16_t PosX;
    uint16_t PosY;
    float FontSize; // INT OR FLOAT?!
    Color Color;
} UIText;

typedef struct UIStyleButton {
    UIAlignmentHorizontal TextAlignmentHorizontal;
    UIAlignmentVertical TextAlignmentVertical;
    float FontSize; // INT OR FLOAT?!

    Color ColorRectDefault;
    Color ColorRectHover;
    Color ColorRectClick;
    Color ColorRectDisabled;

    Color ColorTextDefault;
    Color ColorTextHover;
    Color ColorTextClick;
    Color ColorTextDisabled;
} UIStyleButton;

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
//uint16_t ui_add_button(UIData* uiData, int posX, int posY, int width, int height, const char* text, UIStyleButton uiStyle, void(*button_clicked)(void*, int), void* context, bool enabled);


#endif