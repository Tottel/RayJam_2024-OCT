#include "UISystem.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

inline bool IsPointInRectangle(Vector2 point, int rectPosX, int rectPosY, int rectWidth, int rectHeight) {
	return point.x >= rectPosX && point.x <= rectPosX + rectWidth && point.y >= rectPosY && point.y <= rectPosY + rectHeight;
}

void ui_exit(UIData* uiData) {
	// TODO Free text! 
	// Preferably from a big buffer
	//RL_FREE(uiData->RectanglesText)
}

void ui_tick(UIData* uiData) {
	Vector2 mousePos = GetMousePosition();
	bool isMouseReleased = IsMouseButtonReleased(0);

	for (int i = 0; i < uiData->ButtonsEnabledCount; ++i) {
		UIRectangle* rect = &uiData->Rectangles[uiData->ButtonsEnabled[i].RectIndex];
		UIText* text = &uiData->RectanglesText[uiData->ButtonsEnabled[i].RectIndex];

		if (IsPointInRectangle(mousePos, rect->PosX, rect->PosY, rect->Width, rect->Height)) {
			TraceLog(LOG_DEBUG, "IN RECT");
			bool isMouseDown = IsMouseButtonDown(0);

			rect->Color = isMouseDown ? uiData->ButtonsEnabled[i].ColorRectClick : uiData->ButtonsEnabled[i].ColorRectHover;
			text->Color = isMouseDown ? uiData->ButtonsEnabled[i].ColorTextClick : uiData->ButtonsEnabled[i].ColorTextHover;

			if (isMouseReleased) {
				uiData->ButtonsFunctionalities[i].button_clicked(uiData->ButtonsFunctionalities[i].Context);
			}
		}
		else {
			rect->Color = uiData->ButtonsEnabled[i].ColorRectDefault;
			text->Color = uiData->ButtonsEnabled[i].ColorTextDefault;
		}
	}
}

void ui_draw(UIData* uiData) {
	for (int i = 0; i < uiData->RectangleCount; ++i) {
		DrawRectangle(uiData->Rectangles[i].PosX, uiData->Rectangles[i].PosY, uiData->Rectangles[i].Width, uiData->Rectangles[i].Height, uiData->Rectangles[i].Color);
	}

	for (int i = 0; i < uiData->RectangleCount; ++i) {
		DrawText(uiData->RectanglesText[i].Text, uiData->RectanglesText[i].PosX, uiData->RectanglesText[i].PosY, uiData->RectanglesText[i].FontSize, uiData->RectanglesText[i].Color);
	}
}

uint16_t ui_add_rectangle_with_text(UIData* uiData, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height, Color rectColor, const char* text, int fontSize, UIAlignmentHorizontal alignHor, UIAlignmentVertical alignVer, Color textColor) {
	if (fontSize < 10) fontSize = 10;

	uiData->Rectangles[uiData->RectangleCount].PosX = posX;
	uiData->Rectangles[uiData->RectangleCount].PosY = posY;
	uiData->Rectangles[uiData->RectangleCount].Width = width;
	uiData->Rectangles[uiData->RectangleCount].Height = height;
	uiData->Rectangles[uiData->RectangleCount].Color = rectColor;

	uint16_t offsetX = 0;
	uint16_t offsetY = 0;

	if (alignHor > 0 || alignVer > 0) {
		Vector2 textDim = MeasureTextEx(GetFontDefault(), text, (float)fontSize, fontSize / 10.0f);

		offsetX = (uint16_t)(( width - textDim.x) * ALIGNMENT_TABLE[(int)alignHor]);
		offsetY = (uint16_t)((height - textDim.y) * ALIGNMENT_TABLE[(int)alignVer]);
	}

	uiData->RectanglesText[uiData->RectangleCount].Text = text;
	uiData->RectanglesText[uiData->RectangleCount].PosX = posX + offsetX;
	uiData->RectanglesText[uiData->RectangleCount].PosY = posY + offsetY;
	uiData->RectanglesText[uiData->RectangleCount].FontSize = fontSize;
	uiData->RectanglesText[uiData->RectangleCount].Color = textColor;

	uiData->RectangleCount += 1;

	return uiData->RectangleCount - 1;
}

uint16_t ui_add_button(UIData* uiData, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height, const char* text, UIStyleButton uiStyle, void(*button_clicked)(void*), void* context, bool enabled) {
	// TODO So much memory allocation.. Can we do this in a block of memory instead?
	char* copy = RL_MALLOC(strlen(text) + 1);
	strcpy(copy, text);
	
	uint16_t rectIndex = ui_add_rectangle_with_text(uiData, posX, posY, width, height, uiStyle.ColorRectDefault, copy, uiStyle.FontSize, uiStyle.TextAlignmentHorizontal, uiStyle.TextAlignmentVertical, uiStyle.ColorTextDefault);
	uint16_t uniqueHandle = uiData->ButtonsEnabledCount + uiData->ButtonsDisabledCount; // TODO May stop working when we allow removing buttons..

	uiData->ButtonsAll[uniqueHandle].RectIndex = rectIndex;
	uiData->ButtonsAll[uniqueHandle].ColorRectDefault = uiStyle.ColorRectDefault;
	uiData->ButtonsAll[uniqueHandle].ColorRectHover = uiStyle.ColorRectHover;
	uiData->ButtonsAll[uniqueHandle].ColorRectClick = uiStyle.ColorRectClick;
	uiData->ButtonsAll[uniqueHandle].ColorRectDisabled = uiStyle.ColorRectDisabled;
	uiData->ButtonsAll[uniqueHandle].ColorTextDefault = uiStyle.ColorTextDefault;
	uiData->ButtonsAll[uniqueHandle].ColorTextHover = uiStyle.ColorTextHover;
	uiData->ButtonsAll[uniqueHandle].ColorTextClick = uiStyle.ColorTextClick;
	uiData->ButtonsAll[uniqueHandle].ColorTextDisabled = uiStyle.ColorTextDisabled;
	uiData->ButtonsAll[uniqueHandle].button_clicked = button_clicked;
	uiData->ButtonsAll[uniqueHandle].Context = context;

	if (!enabled) {
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].RectIndex = rectIndex;
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].ColorRectDisabled = uiStyle.ColorRectDisabled;
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].ColorTextDisabled = uiStyle.ColorTextDisabled;

		uiData->Rectangles[rectIndex].Color = uiStyle.ColorRectDisabled;

		uiData->RectanglesText[rectIndex].Color = uiStyle.ColorTextDisabled;

		uiData->ButtonHandleValues[uniqueHandle].Enabled = false;
		uiData->ButtonHandleValues[uniqueHandle].ButtonID = uiData->ButtonsDisabledCount;

		uiData->ButtonsDisabledCount += 1;
	}
	else {
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].RectIndex = rectIndex;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectDefault = uiStyle.ColorRectDefault;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectHover = uiStyle.ColorRectHover;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectClick = uiStyle.ColorRectClick;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextDefault = uiStyle.ColorTextDefault;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextHover = uiStyle.ColorTextHover;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextClick = uiStyle.ColorTextClick;

		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].RectIndex = rectIndex;
		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].button_clicked = button_clicked;
		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].Context = context;

		uiData->ButtonHandleValues[uniqueHandle].Enabled = true;
		uiData->ButtonHandleValues[uniqueHandle].ButtonID = uiData->ButtonsEnabledCount;

		uiData->ButtonsEnabledCount += 1;
	}

	return uniqueHandle;
}

void ui_toggle_button(UIData* uiData, uint16_t buttonHandle, bool enabled) {
	UIButtonHandleValue buttonValue = uiData->ButtonHandleValues[buttonHandle];

	if (buttonValue.Enabled == enabled) return;

	if (enabled) {
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].RectIndex = uiData->ButtonsAll[buttonHandle].RectIndex;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectDefault = uiData->ButtonsAll[buttonHandle].ColorRectDefault;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectHover = uiData->ButtonsAll[buttonHandle].ColorRectHover;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorRectClick = uiData->ButtonsAll[buttonHandle].ColorRectClick;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextDefault = uiData->ButtonsAll[buttonHandle].ColorTextDefault;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextHover = uiData->ButtonsAll[buttonHandle].ColorTextHover;
		uiData->ButtonsEnabled[uiData->ButtonsEnabledCount].ColorTextClick = uiData->ButtonsAll[buttonHandle].ColorTextClick;

		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].RectIndex = uiData->ButtonsAll[buttonHandle].RectIndex;
		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].button_clicked = uiData->ButtonsAll[buttonHandle].button_clicked;
		uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount].Context = uiData->ButtonsAll[buttonHandle].Context;

		uiData->ButtonHandleValues[buttonHandle].Enabled = true;
		uiData->ButtonHandleValues[buttonHandle].ButtonID = uiData->ButtonsEnabledCount;

		uiData->ButtonsEnabledCount += 1;

		uiData->ButtonsDisabled[buttonValue.ButtonID] = uiData->ButtonsDisabled[uiData->ButtonsDisabledCount - 1];

		uiData->ButtonsDisabledCount -= 1;
	}
	else {
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].RectIndex = uiData->ButtonsAll[buttonHandle].RectIndex;
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].ColorRectDisabled = uiData->ButtonsAll[buttonHandle].ColorRectDisabled;
		uiData->ButtonsDisabled[uiData->ButtonsDisabledCount].ColorTextDisabled = uiData->ButtonsAll[buttonHandle].ColorTextDisabled;

		uint16_t rectIndex = uiData->ButtonsAll[buttonHandle].RectIndex;
		uiData->Rectangles[rectIndex].Color = uiData->ButtonsAll[buttonHandle].ColorRectDisabled;
		uiData->RectanglesText[rectIndex].Color = uiData->ButtonsAll[buttonHandle].ColorTextDisabled;

		uiData->ButtonHandleValues[buttonHandle].Enabled = false;
		uiData->ButtonHandleValues[buttonHandle].ButtonID = uiData->ButtonsDisabledCount;

		uiData->ButtonsDisabledCount += 1;

		uiData->ButtonsEnabled[buttonValue.ButtonID] = uiData->ButtonsEnabled[uiData->ButtonsEnabledCount - 1];
		uiData->ButtonsFunctionalities[buttonValue.ButtonID] = uiData->ButtonsFunctionalities[uiData->ButtonsEnabledCount - 1];

		uiData->ButtonsEnabledCount -= 1;
	}
}