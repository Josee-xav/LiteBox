#include <Windows.h>
#include <windowsx.h>
#include "Taskbar/CTaskbar.h"
#include "UsefulShellApi.h"
#include "ShellHookingFunctionality.h"
#include <CommCtrl.h>
#include <dwmapi.h>
#include "StylingStruct.h"

#include "colors.h"
#include "Services/TaskService.h"
#include "settings.h"
#pragma warning(disable : 4996)
#define SC_RESTORE      0xF120
#define SC_MINIMIZE     0xF020


CTaskbar* taskbar;
HWND hDesktopWnd = NULL;
HINSTANCE main_hInstance = NULL;


LRESULT CALLBACK WndProcParent(HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance ,
                     HINSTANCE hPrevInstance ,
                     LPSTR     lpCmdLine ,
                     int       nCmdShow)
{
    MSG msg;
    WNDCLASSEX wc;
    std::string windowClassName{ "desktopWindoww" };

    main_hInstance = hInstance;

    // Check if the taskbar is already running
    HWND checkOldhwnd = FindWindow(windowClassName.c_str() , NULL);
    if (checkOldhwnd) // if the HWND is not null then that means that it found the proccess
    {
        if (ShellApi::messageBox(MB_OK , main_hInstance , "Error!" , "Application is already running!\nClose all of the other instances and try again."))
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
        WS_EX_TOOLWINDOW ,
        wc.lpszClassName ,
        NULL ,
        WS_POPUP ,
        0 , 0 , 0 , 0 ,
        NULL/*GetDesktopWindow()*/ ,
        NULL ,
        NULL ,
        NULL
    );

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

std::string getStylePath()
{
    return (ShellApi::getExePath() + "\\defaultStyle.ini");
}

void edit_file(int id)
{
    switch (id) {
        case 0: // style file. id.
        {
            char buffer[2 * MAX_PATH];
            sprintf(buffer , "\"%s\" \"%s\"" , "notepad.exe" , getStylePath());
            ShellApi::executeShell(NULL , NULL , "notepad.exe" , getStylePath().c_str() , ShellApi::getExePath().c_str() , 1 , 0);
        }
        break;


    }
}

void runTaskbar()
{
    StyleStruct mStyle;
    readStyle(getStylePath().c_str() , &mStyle);


    // create it
    RECT monitorSize = ShellApi::getPrimaryScreenRes();

    ShellApi::setWorkArea((defaultStyle::TASKBAR_HEIGHT + mStyle.marginWidth + mStyle.borderWidth));
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
            ShellApi::restartExplorerWindow();

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
