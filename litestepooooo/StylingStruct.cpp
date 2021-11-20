#include "StylingStruct.h"

StyleStruct::StyleStruct()
{
    windowBackgroundColor = defaultStyle::windowFrameColor;
    taskFocusColor = defaultStyle::taskFocusColor;
    clockColor = CLR_INVALID;
    borderColor = CLR_INVALID;
    unfocusedTextColor = defaultStyle::textColor;
    focusedTextColor = unfocusedTextColor;

    marginWidth = 0;

    taskBevelStyle = 0;
    clockBevelStyle = 0;

    clockTimeFormat = 0;

    rectRoundedEdgeClock = false;
    rectRoundedEdgeTaskButtons = false;


    borderWidth = 0;

    taskbuttonSpacing = 0;
    taskbuttonTopSpacing = 0;


}