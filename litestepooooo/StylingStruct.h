#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace defaultStyle {
    const int TASKBAR_GENERAL_SPACING{ 3 };
    const int MAX_TASKWIDTH{ 100 };

    const int TASKBAR_HEIGHT{ 20 };
    const int TASKBAR_WIDTH{ 1001 };
    const int ICON_SIZE{ 20 };
    const COLORREF windowFrameColor{ PALETTERGB(32, 31, 30) };
    const COLORREF taskFocusColor{ PALETTERGB(59, 58, 57) };

    const COLORREF textColor{ PALETTERGB(255, 255, 255) };
};

// If you add a optionn on here then you also need to add to StyleItems in settings.cpp for the parser to get the data.
class StyleStruct
{
public:
    COLORREF windowBackgroundColor;
    COLORREF taskFocusColor;
    COLORREF clockColor;
    COLORREF borderColor;
    COLORREF unfocusedTextColor;
    COLORREF focusedTextColor;

    int marginWidth;

    int taskBevelStyle;
    int clockBevelStyle;

    int clockTimeFormat;

    bool rectRoundedEdgeClock;
    bool rectRoundedEdgeTaskButtons;


    int borderWidth;

    int taskbuttonSpacing;
    int taskbuttonTopSpacing;



    StyleStruct();

};