#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <string>

enum class IconSizes
{
    icon_small ,
    icon_big
};

namespace ShellApi {

    bool executeShell(HWND hwnd , const char* verb , const char* file , const char* args , const char* dir , int showCmds , int flags);

    //Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    std::string getLastErrorAsString();

    void restartExplorerWindow();

    std::string getWindowTitle(HWND hwnd);

    HBITMAP getBitmapFromHicon(HICON icon);

    HICON getHICONFromHWND(HWND hwnd , IconSizes iconsize);

    RECT getPrimaryScreenRes();

    void setWorkArea(int height);

    std::string getWindowClassName(HWND hwnd);

    std::string getExePath();

    int messageBox(int uType , HINSTANCE instance , std::string title , std::string text);

    void showExplorer();
    void hideExplorer();
};