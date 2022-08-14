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

    bool executeShell(HWND hwnd , const wchar_t* verb , const wchar_t* file , const wchar_t* args , const wchar_t* dir , int showCmds , int flags);

    //Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    std::string getLastErrorAsString();

    void restartExplorerWindow();

    std::wstring getWindowTitle(HWND hwnd);

    HBITMAP getBitmapFromHicon(HICON icon);

    HICON getHICONFromHWND(HWND hwnd , IconSizes iconsize);

    RECT getPrimaryScreenRes();

    void setWorkArea(int height);

    std::wstring getWindowClassName(HWND hwnd);

    std::wstring getExePath();

    int messageBox(int uType , HINSTANCE instance , std::wstring title , std::wstring text);

    void showExplorer();
    void hideExplorer();
};