#pragma once
#include "ShellHookingFunctionality.h"


namespace WindowQueryHelper
{
    BOOL initWinHelper();

    bool inspectWindows10AppWindow(HWND hWnd, bool cloakTest);
    bool isAppWindow(HWND hwnd, bool checkWin10 = true);
    __int64 __fastcall IsTaskWindow(HWND a2);

};