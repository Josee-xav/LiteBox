#include "settings.h"
#include "iniParser.h"
#include <windef.h>
#include "colors.h"
enum class STR2INTCONVERT_ERROR { E_SUCCESS , E_OVERFLOW , E_UNDERFLOWW , E_INCONVERTIBLE };

STR2INTCONVERT_ERROR stringToInt(int& i , char const* s , int base = 0)
{
    char* end;
    long  l;
    errno = 0;
    l = strtol(s , &end , base);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
        return STR2INTCONVERT_ERROR::E_OVERFLOW;
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
        return STR2INTCONVERT_ERROR::E_UNDERFLOWW;
    }
    if (*s == '\0' || *end != '\0') {
        return STR2INTCONVERT_ERROR::E_INCONVERTIBLE;
    }
    i = l;
    return STR2INTCONVERT_ERROR::E_SUCCESS;
}

bool readBool(std::string data)
{
    if (!_stricmp(data.c_str() , "true"))
        return true;
    if (!_stricmp(data.c_str() , "false"))
        return false;

    return false; // default is false.
}

enum class style_data_types
{
    C_BOOL ,
    C_INT ,
    C_STR ,
    C_COL ,
    C_NULL ,
};

enum STYLE_INDEX
{
    S_WINBACKGROUND_COLOR ,
    S_TASKFOCUS_COLOR ,
    S_BORDER_COLOR ,
    S_UNFOCUSEDTEXT_COLOR ,
    S_FOCUSEDTEXT_COLOR ,
    S_CLOCK_COLOR ,

    S_MARGINWIDTH ,
    S_BORDERWIDTH ,
    S_BEVELWIDTH ,
    S_TASK_BEVELSTYLE ,
    S_CLOCK_BEVELSTYLE ,
    S_CLOCK_TIMEFORMAT ,

    S_ROUNDED_CLOCK ,
    S_ROUNDED_TASKBUTTONS ,

    S_TASKBTN_SPACING ,
    S_TASKBTN_TOP_SPACING ,
    S_FONT_SIZE ,


    S_M_FONT_SIZE ,
};

struct items
{
    style_data_types type;
    int sn;
    std::string key;
};

static const struct items StyleItems[] = {
    { style_data_types::C_COL, S_WINBACKGROUND_COLOR, "WINDOW_FRAME_COLOR"},
    { style_data_types::C_COL, S_TASKFOCUS_COLOR , "TASK_FOCUS_COLOR"},
    { style_data_types::C_COL, S_BORDER_COLOR, "BORDER_COLOR"},
    { style_data_types::C_COL, S_UNFOCUSEDTEXT_COLOR, "UNFOCUSED_TEXT_COLOR"},
    { style_data_types::C_COL, S_FOCUSEDTEXT_COLOR, "FOCUSED_TEXT_COLOR"},
    { style_data_types::C_COL, S_CLOCK_COLOR, "CLOCK_COLOR"},

    { style_data_types::C_INT, S_MARGINWIDTH, "MARGIN_WIDTH"},
    { style_data_types::C_INT, S_BORDERWIDTH, "BORDER_WIDTH"},
    { style_data_types::C_INT, S_TASK_BEVELSTYLE, "TASK_BEVEL_STYLE"},
    { style_data_types::C_INT, S_CLOCK_BEVELSTYLE, "CLOCK_BEVEL_STYLE"},

    { style_data_types::C_INT, S_CLOCK_TIMEFORMAT, "CLOCK_TIME_FORMAT"},

    { style_data_types::C_BOOL, S_ROUNDED_TASKBUTTONS, "ROUNDED_EDGE_TASKBUTTONS"},
    { style_data_types::C_BOOL, S_ROUNDED_CLOCK, "ROUNDED_EDGE_CLOCK"},

    { style_data_types::C_INT, S_TASKBTN_SPACING , "TASKBTN_SPACING"},
    { style_data_types::C_INT, S_TASKBTN_TOP_SPACING , "TASKBTN_TOP_SPACING"},

    { style_data_types::C_INT, S_M_FONT_SIZE , "MENU_FONT_SIZE"},

    { style_data_types::C_NULL, 0 }
};


void* StyleStructPtr(int sn_index , StyleStruct* pStyle)
{
    switch (sn_index) {
        case  S_WINBACKGROUND_COLOR:
            return &pStyle->windowBackgroundColor;
            break;
        case  S_TASKFOCUS_COLOR:
            return &pStyle->taskFocusColor;
            break;
        case  S_BORDER_COLOR:
            return &pStyle->borderColor;
            break;
        case  S_UNFOCUSEDTEXT_COLOR:
            return &pStyle->unfocusedTextColor;
            break;
        case  S_FOCUSEDTEXT_COLOR:
            return &pStyle->focusedTextColor;
            break;
        case  S_MARGINWIDTH:
            return &pStyle->marginWidth;
            break;
        case  S_BORDERWIDTH:
            return &pStyle->borderWidth;
            break;
        case  S_TASK_BEVELSTYLE:
            return &pStyle->taskBevelStyle;
            break;
        case  S_CLOCK_BEVELSTYLE:
            return &pStyle->clockBevelStyle;
            break;
        case  S_CLOCK_TIMEFORMAT:
            return &pStyle->clockTimeFormat;
            break;
        case  S_ROUNDED_CLOCK:
            return &pStyle->rectRoundedEdgeClock;
            break;
        case  S_ROUNDED_TASKBUTTONS:
            return &pStyle->rectRoundedEdgeTaskButtons;
            break;
        case  S_CLOCK_COLOR:
            return &pStyle->clockColor;
            break;
        case  S_TASKBTN_SPACING:
            return &pStyle->taskbuttonSpacing;
            break;
        case  S_TASKBTN_TOP_SPACING:
            return &pStyle->taskbuttonTopSpacing;
            break;
        case  S_M_FONT_SIZE:
            return &pStyle->popupMenu_fontSize;
            break;
        default: return NULL;
    }
}

void getStyleItem(ini::Section& sec , items item , StyleStruct* pStyle)
{

    std::string data = sec[item.key].GetValue<std::string>();

    // couldnt find a match.
    if (data == "") {
        return;
    }

    switch (item.type) {
        case style_data_types::C_BOOL:
        {

            bool boolData = readBool(data);

            bool* v = (bool*)StyleStructPtr(item.sn , pStyle);
            *v = boolData;
        }
        break; case style_data_types::C_INT:
        {
            int intData;
            stringToInt(intData , data.c_str());

            //      prevents negative numbers.
            if (intData < 0) {
                intData = 0;
            }

            int* v = (int*)StyleStructPtr(item.sn , pStyle);
            *v = intData;
        }
        break;
        case style_data_types::C_COL:
        {
            COLORREF colorData = readColorFromString(data.c_str());
            if (CLR_INVALID == colorData)
                colorData = defaultStyle::taskFocusColor;

            COLORREF* v = (COLORREF*)StyleStructPtr(item.sn , pStyle);
            *v = colorData;
        }
        break;
    }
}


void readStyle(const char* styleFilePath , StyleStruct* pStyle)
{
    ini::IniFile ini(styleFilePath);
    bool fileLoadStatus = ini.Load();

    std::string data;

    if (fileLoadStatus) {
        ini::Section& sec = ini.GetSection("settings");

        for (int i = 0; i < sizeof(StyleItems) / sizeof(StyleItems[0]); i++) {
            items item = StyleItems[i];

            getStyleItem(sec , item , pStyle);

        }
    }
}