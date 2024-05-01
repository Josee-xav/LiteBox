#include "Styling.h"
#include "iniParser.h"
#include <windef.h>
#include "colors.h"


//colors of stuff.
COLORREF _Stylizing::windowBackgroundColor = PALETTERGB(32, 31, 30);
COLORREF _Stylizing::task_FocusColor = PALETTERGB(59, 58, 57);
COLORREF _Stylizing::clock_Color = CLR_INVALID;
COLORREF _Stylizing::borderColor = CLR_INVALID;
COLORREF _Stylizing::unfocusedTextColor = PALETTERGB(255, 255, 255);
COLORREF _Stylizing::focusedTextColor = PALETTERGB(255, 255, 255);

// roundedness
bool _Stylizing::rectRoundedEdge_Clock = false;
bool _Stylizing::rectRoundedEdge_TaskButtons = false;
bool _Stylizing::rectRoundedEdge_Bar = false;

int _Stylizing::rectRoundedEdge_Clock_Width = 0;
int _Stylizing::rectRoundedEdge_Clock_Height = 0;
int _Stylizing::rectRoundedEdge_TaskButtons_Width = 0;
int _Stylizing::rectRoundedEdge_TaskButtons_Height = 0;
int _Stylizing::rectRoundedEdge_Bar_Width = 0;
int _Stylizing::rectRoundedEdge_Bar_Height = 0;

//WIDTHS TUFF
int _Stylizing::taskbar_Width = 1001;

int _Stylizing::border_Width = 0;

//height stff
int _Stylizing::taskbar_Height = 20;
int _Stylizing::task_iconSize = 20;
int _Stylizing::tray_iconSize = 20;
//===

//spacings
int _Stylizing::task_buttonSpacing = 0;
int _Stylizing::task_buttonTopSpacing = 0;

//font stuff
std::wstring _Stylizing::font = L"Arial";
int _Stylizing::task_fontSize = 20;
LONG _Stylizing::fontWeight = 1;
int _Stylizing::menu_fontSize = 15;
int _Stylizing::clock_fontSize = 15;

// bevels
int _Stylizing::task_BevelStyle = 0;
int _Stylizing::clock_BevelStyle = 0;

//misc
int _Stylizing::clock_TimeFormat = 0;
bool _Stylizing::tasks_IconOnly = false;

_Stylizing m_Style;

enum class style_data_types
{
    C_BOOL,
    C_INT,
    C_STR,
    C_COL,
    C_NULL,
};

enum STYLE_INDEX
{
    //COLORS OF STUFF.
    S_WINDOWBACKGROUNDCOLOR,
    S_TASK_FOCUSCOLOR,
    S_CLOCK_COLOR,
    S_BORDERCOLOR,
    S_UNFOCUSEDTEXTCOLOR,
    S_FOCUSEDTEXTCOLOR,

    // ROUNDEDNESS
    S_RECTROUNDEDEDGE_CLOCK,
    S_RECTROUNDEDEDGE_TASKBUTTONS,
    S_RECTROUNDEDEDGE_BAR,


    S_RECTROUNDEDEDGE_CLOCK_WIDTH ,
    S_RECTROUNDEDEDGE_CLOCK_HEIGHT ,
    S_RECTROUNDEDEDGE_TASKBUTTONS_WIDTH ,
    S_RECTROUNDEDEDGE_TASKBUTTONS_HEIGHT ,
    S_RECTROUNDEDEDGE_BAR_WIDTH ,
    S_RECTROUNDEDEDGE_BAR_HEIGHT ,




    S_TASK_ICONSIZE,
    S_TRAY_ICONSIZE,

    //WIDTHS TUFF
    S_TASKBAR_WIDTH,

    S_BORDER_WIDTH,

    //HEIGHT STFF
    S_TASKBAR_HEIGHT,

    //===

    //SPACINGS
    S_TASK_BUTTONSPACING,
    S_TASK_BUTTONTOPSPACING,

    //FONT STUFF
    S_FONT,
    S_TASK_FONTSIZE,
    S_MENU_FONTSIZE,
    S_CLOCK_FONTSIZE,
    S_FONTWEIGHT,

    // BEVELS
    S_TASK_BEVELSTYLE,
    S_CLOCK_BEVELSTYLE,

    //MISC
    S_CLOCK_TIMEFORMAT,
    S_TASKS_ICONONLY,
};

struct iniOptions
{
    style_data_types type;
    int sn;
    std::string key;
};

static const iniOptions StyleItems[] = {
    { style_data_types::C_COL, S_WINDOWBACKGROUNDCOLOR, "WINDOW_FRAME_COLOR"},
    { style_data_types::C_COL, S_TASK_FOCUSCOLOR , "TASK_FOCUS_COLOR"},
    { style_data_types::C_COL, S_BORDERCOLOR, "BORDER_COLOR"},
    { style_data_types::C_COL, S_UNFOCUSEDTEXTCOLOR, "UNFOCUSED_TEXT_COLOR"},
    { style_data_types::C_COL, S_FOCUSEDTEXTCOLOR, "FOCUSED_TEXT_COLOR"},
    { style_data_types::C_COL, S_CLOCK_COLOR, "CLOCK_COLOR"},

    //width
    { style_data_types::C_INT, S_BORDER_WIDTH, "BORDER_WIDTH"},
    { style_data_types::C_INT, S_TASKBAR_WIDTH, "TASKBAR_WIDTH"},
    //HEIGHT
    { style_data_types::C_INT, S_TASKBAR_HEIGHT, "TASKBAR_HEIGHT"},


    { style_data_types::C_INT, S_TASK_ICONSIZE, "TASK_ICONSIZE"},
    { style_data_types::C_INT, S_TRAY_ICONSIZE, "TRAY_ICONSIZE"},

    //roundness
    { style_data_types::C_BOOL, S_RECTROUNDEDEDGE_BAR, "ROUNDED_EDGE_BAR"},
    { style_data_types::C_BOOL, S_RECTROUNDEDEDGE_TASKBUTTONS, "ROUNDED_EDGE_TASKBUTTONS"},
    { style_data_types::C_BOOL, S_RECTROUNDEDEDGE_CLOCK, "ROUNDED_EDGE_CLOCK"},

    { style_data_types::C_INT, S_RECTROUNDEDEDGE_CLOCK_WIDTH , "ROUNDED_EDGE_CLOCK_WIDTH"},
    { style_data_types::C_INT, S_RECTROUNDEDEDGE_CLOCK_HEIGHT , "ROUNDED_EDGE_CLOCK_HEIGHT"},
    { style_data_types::C_INT, S_RECTROUNDEDEDGE_TASKBUTTONS_WIDTH , "ROUNDED_EDGE_TASKBUTTON_WIDTH"},
    { style_data_types::C_INT, S_RECTROUNDEDEDGE_TASKBUTTONS_HEIGHT , "ROUNDED_EDGE_TASKBUTTON_HEIGHT"},
    { style_data_types::C_INT, S_RECTROUNDEDEDGE_BAR_WIDTH , "ROUNDED_EDGE_BAR_WIDTH"},
    { style_data_types::C_INT, S_RECTROUNDEDEDGE_BAR_HEIGHT , "ROUNDED_EDGE_BAR_WIDTH"},


    //spacing

    { style_data_types::C_INT, S_TASK_BUTTONSPACING , "TASKBTN_SPACING"},
    { style_data_types::C_INT, S_TASK_BUTTONTOPSPACING , "TASKBTN_TOP_SPACING"},

    // font stuff
    { style_data_types::C_STR, S_FONT , "FONT_TYPE"},
    { style_data_types::C_INT, S_TASK_FONTSIZE , "TASK_FONT_SIZE"},
    { style_data_types::C_INT, S_MENU_FONTSIZE , "MENU_FONT_SIZE"},
    { style_data_types::C_INT, S_CLOCK_FONTSIZE , "CLOCK_FONT_SIZE"},
    { style_data_types::C_INT, S_FONTWEIGHT , "FONT_WEIGHT"},

    //bevel
    { style_data_types::C_INT, S_TASK_BEVELSTYLE, "TASK_BEVEL_STYLE"},
    { style_data_types::C_INT, S_CLOCK_BEVELSTYLE, "CLOCK_BEVEL_STYLE"},



    //misc
    { style_data_types::C_INT, S_CLOCK_TIMEFORMAT, "CLOCK_TIME_FORMAT"},
    { style_data_types::C_BOOL, S_TASKS_ICONONLY, "TASKS_ICONONLY"},

    { style_data_types::C_NULL, 0 }
};






















enum class STR2INTCONVERT_error { E_SUCCESS, E_OVERFLOW, E_UNDERFLOWW, E_INCONVERTIBLE };

bool readBool(std::wstring data)
{
    //case-insensitive
    if (!_wcsicmp(data.c_str(), L"true"))
        return true;
    if (!_wcsicmp(data.c_str(), L"false"))
        return false;

    return false; // default is false.
}


void* StyleStructPtr(int sn_index, _Stylizing* pStyle)
{
    switch (sn_index) {
    case  S_WINDOWBACKGROUNDCOLOR:
        return &pStyle->windowBackgroundColor;
        break;
    case  S_TASK_FOCUSCOLOR:
        return &pStyle->task_FocusColor;
        break;
    case  S_BORDERCOLOR:
        return &pStyle->borderColor;
        break;
    case  S_UNFOCUSEDTEXTCOLOR:
        return &pStyle->unfocusedTextColor;
        break;
    case  S_FOCUSEDTEXTCOLOR:
        return &pStyle->focusedTextColor;
        break;
    case  S_CLOCK_COLOR:
        return &pStyle->clock_Color;
        break;
        //===

        //width
    case  S_BORDER_WIDTH:
        return &pStyle->border_Width;
        break;
    case  S_TASKBAR_WIDTH:
        return &pStyle->taskbar_Width;
        break;
        //HEIGHT STUFF
    case  S_TASKBAR_HEIGHT:
        return &pStyle->taskbar_Height;
        break;

        //bevels
    case  S_TASK_BEVELSTYLE:
        return &pStyle->task_BevelStyle;
        break;
    case  S_CLOCK_BEVELSTYLE:
        return &pStyle->clock_BevelStyle;
        break;
        //roundedness
    case  S_RECTROUNDEDEDGE_CLOCK:
        return &pStyle->rectRoundedEdge_Clock;
        break;
    case  S_RECTROUNDEDEDGE_TASKBUTTONS:
        return &pStyle->rectRoundedEdge_TaskButtons;
        break;
    case  S_RECTROUNDEDEDGE_BAR:
        return &pStyle->rectRoundedEdge_Bar;
        break;

    case S_RECTROUNDEDEDGE_CLOCK_WIDTH:
        return &pStyle->rectRoundedEdge_Clock_Width;
        break;

    case S_RECTROUNDEDEDGE_CLOCK_HEIGHT: 
        return &pStyle->rectRoundedEdge_Clock_Height;
        break;

    case S_RECTROUNDEDEDGE_TASKBUTTONS_WIDTH:
        return &pStyle->rectRoundedEdge_TaskButtons_Width;
        break;

    case S_RECTROUNDEDEDGE_TASKBUTTONS_HEIGHT:
        return &pStyle->rectRoundedEdge_TaskButtons_Height;
        break;

    case S_RECTROUNDEDEDGE_BAR_WIDTH:
        return &pStyle->rectRoundedEdge_Bar_Width;
        break;

    case S_RECTROUNDEDEDGE_BAR_HEIGHT:
        return &pStyle->rectRoundedEdge_Bar_Height;
        break;

    case  S_TASK_ICONSIZE:
        return &pStyle->task_iconSize;
        break;
    case  S_TRAY_ICONSIZE:
        return &pStyle->tray_iconSize;
        break;


        //spacing
    case  S_TASK_BUTTONSPACING:
        return &pStyle->task_buttonSpacing;
        break;
    case  S_TASK_BUTTONTOPSPACING:
        return &pStyle->task_buttonTopSpacing;
        break;

        //FONT
    case  S_FONT:
        return &pStyle->font;
        break;
    case  S_MENU_FONTSIZE:
        return &pStyle->menu_fontSize;
        break;
    case  S_TASK_FONTSIZE:
        return &pStyle->task_fontSize;
        break;
    case  S_CLOCK_FONTSIZE:
        return &pStyle->clock_fontSize;
        break;

    case  S_FONTWEIGHT:
        return &pStyle->fontWeight;
        break;
        ///misc
    case  S_CLOCK_TIMEFORMAT:
        return &pStyle->clock_TimeFormat;
        break;
    case  S_TASKS_ICONONLY:
        return &pStyle->tasks_IconOnly;
        break;
    default: return NULL;
    }
}

void getStyleItem(ini::Section& sec, iniOptions item, _Stylizing* pStyle)
{

    std::wstring data = sec[item.key].GetValue<std::wstring>();

    // couldnt find a match.
    if (data == L"") {
        return;
    }

    switch (item.type) {
    case style_data_types::C_BOOL:
    {

        bool boolData = readBool(data);

        bool* v = (bool*)StyleStructPtr(item.sn, pStyle);
        *v = boolData;
    }
    break;
    case style_data_types::C_STR:
    {



        std::wstring* v = (std::wstring*)StyleStructPtr(item.sn, pStyle);
        *v = data;
    }

    break;
    case style_data_types::C_INT:
    {
        int intData;
        intData = std::stoi(data.c_str());

        //      prevents negative numbers.
        if (intData < 0) {
            intData = 0;
        }

        int* v = (int*)StyleStructPtr(item.sn, pStyle);
        *v = intData;
    }
    break;
    case style_data_types::C_COL:
    {
        COLORREF colorData = readColorFromString(data.c_str());
        if (CLR_INVALID == colorData)
            colorData = m_Style.task_FocusColor;

        COLORREF* v = (COLORREF*)StyleStructPtr(item.sn, pStyle);
        *v = colorData;
    }
    break;
    }
}



void readStyle(const char* styleFilePath)
{
    ini::IniFile ini(styleFilePath);
    const bool fileLoadStatus = ini.Load();

    std::string data;

    if (fileLoadStatus) {
        ini::Section& sec = ini.GetSection("settings");

        for (int i = 0; i < sizeof(StyleItems) / sizeof(StyleItems[0]); i++) {
            iniOptions item = StyleItems[i];

            getStyleItem(sec, item, &m_Style);

        }
    }
}
