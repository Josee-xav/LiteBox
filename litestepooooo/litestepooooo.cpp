#include <Windows.h>
#include "Taskbar/CTaskbar.h"
#include "WinShellFunctionality/UsefulShellApi.h"
#include "WinShellFunctionality/ShellHookingFunctionality.h"
#include <CommCtrl.h>
#include <dwmapi.h>
#include "Styling.h"
#include "colors.h"
#include "Services/TaskService.h"
#include <stdlib.h>
#pragma warning(disable : 4996)
#define SC_RESTORE      0xF120
#define SC_MINIMIZE     0xF020


std::wstring windowClassName{ L"LiteBoxDesktopWindoww" };

CTaskbar* taskbar;
HWND hDesktopWnd = NULL;
HINSTANCE main_hInstance = NULL;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
void setAutostart(int on , int elevate)
{
    // Open key
    HKEY key;
    int error = RegCreateKeyEx(HKEY_CURRENT_USER , L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" , 0 , NULL , 0 , KEY_SET_VALUE , NULL , &key , NULL);
    if (error != ERROR_SUCCESS) {
        MessageBoxA(NULL , ("RegCreateKeyEx(HKEY_CURRENT_USER,'Software\\Microsoft\\Windows\\CurrentVersion\\Run') Error opening reg" + error) , "error" , MB_OK);
        return;
    }
    if (on) {
        // Get path
        wchar_t path[MAX_PATH] , value[MAX_PATH + 20];
        GetModuleFileName(NULL , path , ARRAY_SIZE(path));
        swprintf(value , ARRAY_SIZE(value) , L"\"%s\"%s" , path , (elevate ? L" -elevate" : L""));
        // Set autostart
        error = RegSetValueEx(key , windowClassName.c_str() , 0 , REG_SZ , (LPBYTE)value , (wcslen(value) + 1) * sizeof(value[0]));
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL , "RegSetValueEx('APP_NAME'), LSetAutostart()" + error , "ERROR" , MB_OK);

            return;
        }
    }
    else {
        // Remove
        error = RegDeleteValue(key , windowClassName.c_str());
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL , "RegSetValueEx('APP_NAME'), LSetAutostart()" + error , "ERROR" , MB_OK);
            return;
        }
    }
    // Close key
    RegCloseKey(key);
}

BOOL isRunning()
{

    HANDLE h = CreateMutexA(NULL , TRUE , (LPCSTR)windowClassName.c_str());

    if (h) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            return TRUE;
        }
    }
    else {
        return TRUE;
    }
    return FALSE;
}


// the "desktop" window for litebox.  i was override the desktop but decided that i didnt want to at the end.
LRESULT CALLBACK WndProcParent(HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance ,
                     HINSTANCE hPrevInstance ,
                     LPSTR     lpCmdLine ,
                     int       nCmdShow)
{
    MSG msg;
    WNDCLASSEX wc;

    main_hInstance = hInstance;

    // Check if the taskbar is already running NULL);
    if (isRunning()) // if the HWND is not null then that means that it found the proccess
    {
        if (ShellApi::messageBox(MB_OK , main_hInstance , L"Error!" , L"Application is already running!\nClose all of the other instances and try again."))
            return 0;
    }

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProcParent;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL , IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = windowClassName.c_str();

    wc.hIconSm = LoadIcon(NULL , IDI_APPLICATION);


    if (!RegisterClassEx(&wc))
        MessageBoxA(NULL , "errrorrr register" , "error" , MB_OK);

    HWND hWnd = CreateWindowEx(
        0 ,
        wc.lpszClassName ,
        NULL ,
        NULL ,
        0 , 0 , 0 , 0 ,
        HWND_MESSAGE/*GetDesktopWindow()*/ ,
        NULL ,
        NULL ,
        NULL
    );

    setAutostart(1 , 0);
    ShowWindow(hWnd , nCmdShow);

    while (GetMessage(&msg , NULL , 0 , 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (taskbar != NULL) {
        delete taskbar;
        taskbar = nullptr;
    }

    return msg.wParam;
}

std::wstring getStylePath()
{
    return (ShellApi::getExePath() + L"\\defaultStyle.ini");
}

void edit_file(int id)
{
    switch (id) {
        case 0: // style file. id.
        {
            char buffer[2 * MAX_PATH];
            sprintf(buffer , "\"%s\" \"%s\"" , "notepad.exe" , getStylePath().c_str());
            ShellApi::executeShell(NULL , NULL , L"notepad.exe" , getStylePath().c_str() , ShellApi::getExePath().c_str() , 1 , 0);
        }
        break;


    }
}

void runTaskbar()
{
    StyleStruct mStyle;
    std::wstring buf = getStylePath();
    std::string str(buf.begin() , buf.end());
    readStyle(str.c_str() , &mStyle);


    // create it
    RECT monitorSize = ShellApi::getPrimaryScreenRes();

    ShellApi::setWorkArea((defaultStyle::TASKBAR_HEIGHT + mStyle.marginWidth + mStyle.borderWidth + 2));
    ShellApi::hideExplorer();// hides the taskbar

    taskbar = new CTaskbar();

    taskbar->create((monitorSize.right / 2) - (defaultStyle::TASKBAR_WIDTH / 2) , monitorSize.bottom - (defaultStyle::TASKBAR_HEIGHT + mStyle.borderWidth + mStyle.marginWidth) , hDesktopWnd , main_hInstance , mStyle);
}

LRESULT CALLBACK WndProcParent(HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam)
{
    switch (msg) {
        case WM_CREATE:
        {
            hDesktopWnd = hwnd;
            initShellHook(hDesktopWnd);

            runTaskbar();
        }
        break;
        case RESTART_TASKBAR:
        {
            if (taskbar != NULL) {
                delete taskbar;
                taskbar = nullptr;
            }
            runTaskbar();
        }
        break;
        case OPEN_STYLE_FILE:
        {
            edit_file(0);
        }
        break;
        case EXIT_TASKBAR:
        case WM_HOTKEY:
        {
            SendMessage(hwnd , WM_CLOSE , NULL , NULL);
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd , &ps);

            FillRect(hdc , &ps.rcPaint , (HBRUSH)(COLOR_WINDOW + 1));

            EndPaint(hwnd , &ps);
        }
        break;
        case WM_DESTROY:
        {
            exitShellHook(hwnd);
            /*ShellApi::restartExplorerWindow();*/
            ShellApi::showExplorer();

            PostQuitMessage(0);
        }
        break;
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        }
        break;

        default:
        {
            if (msg == WM_ShellHook) {
                if (taskbar != nullptr)
                    taskbar->taskService.TaskWndProc(wParam , lParam);
            }
        }
    }
    return DefWindowProc(hwnd , msg , wParam , lParam);
}
