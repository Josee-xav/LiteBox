#include "TaskService.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <dwmapi.h>
#include <vector>
#include "../UsefulShellApi.h"



HWND getLastVisibleActivePopUpOfWindow(HWND window)
{
    HWND lastWindow = GetLastActivePopup(window);

    if (IsWindowVisible(lastWindow))
        return lastWindow;

    if (lastWindow == window)
        return (HWND)NULL;
    return getLastVisibleActivePopUpOfWindow(lastWindow);
}


bool inspectNormalWindow(HWND hWnd , int currentProcessId , std::wstring className)
{

    if (className == L"Shell_TrayWnd" || className == L"DV2ControlHost" || className == L"MsgrIMEWindowClass" || className == L"SysShadow" || className == L"Button"
        || className == L"Windows.UI.Core.CoreWindow" || className == L"Frame Alternate Owner" || className == L"MultitaskingViewFrame") {
        return false;
    }


    wchar_t text[200];
    GetWindowText(hWnd , text , 200);
    std::wstring windowText(text);

    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hWnd , &lpdwProcessId);
    if (lpdwProcessId == currentProcessId)
        return false;

    return true;
}

bool foundTHEchild = false;
BOOL CALLBACK ProcessWindow(HWND hwnd , LPARAM lparam)
{
    DWORD childProcessId;
    GetWindowThreadProcessId(hwnd , &childProcessId);
    if (lparam != childProcessId) {
        wchar_t buffer[200] = L"";
        GetClassName(hwnd , buffer , 200);
        std::wstring childClassName(buffer);


        GetWindowText(hwnd , buffer , 200);
        std::wstring windowText(buffer);
        if (windowText != L"") {
            if (childClassName == L"MicrosoftEdge")
                return false;

            if (childClassName == L"MicrosoftEdgeCP") {
                if (windowText == L"CoreInput") {
                    foundTHEchild = true;
                    return true;
                }

            }

        }
    }

    return true;
}

bool inspectWindows10AppWindow(HWND hWnd , bool cloakTest = true)
{
    // check if windows is not cloaked
    if (cloakTest) {
        const int DWMWA_CLOAKED = 14;
        int cloaked;
        DwmGetWindowAttribute(hWnd , DWMWA_CLOAKED , &cloaked , sizeof(int));
        if (cloaked != 0)
            return false;
    }

    foundTHEchild = false;
    DWORD processId;
    GetWindowThreadProcessId(hWnd , &processId);

    EnumChildWindows(hWnd , ProcessWindow , processId);

    if (!foundTHEchild)// NEVERUSED always true
    {
        wchar_t text[200];
        GetWindowText(hWnd , text , 200);
        if (text != L"") {
            return true;
        }
    }
    return false;
}

// improved by Clodio Pontes with the help of stackoverflow and a huge thanks to Christian Rondeau on github.
bool isAppWindow(HWND hwnd , bool checkWin10 = true)
{
    if (hwnd == GetShellWindow())
        return false;

    // Start at the root owner
    HWND root = GetAncestor(hwnd , GA_ROOTOWNER);

    if (getLastVisibleActivePopUpOfWindow(root) != hwnd)
        return false;

    wchar_t name[200] = L"";
    GetClassName(hwnd , name , 200);
    std::wstring className(name);

    bool ret;
    if (className == L"ApplicationFrameWindow")
        ret = inspectWindows10AppWindow(hwnd , checkWin10);
    else
        ret = inspectNormalWindow(hwnd , GetCurrentProcessId() , className);

    wchar_t text[200];
    GetWindowText(hwnd , text , 200);

    // had to add this because when this function is used in 
    if (checkWin10)
        if (wcslen(text) == 0)
            return false;

    return ret;
}



BOOL CALLBACK initTaskProc(HWND hwnd , LPARAM lParam)
{
    std::vector<taskButtonData*>* windowsVec = reinterpret_cast<std::vector<taskButtonData*>*> (lParam);

    // checks if the app is a window and checks if the hwnd is not the taskbar
    if (isAppWindow(hwnd)) {
        std::wstring wName = ShellApi::getWindowTitle(hwnd);
        HICON icon = ShellApi::getHICONFromHWND(hwnd , IconSizes::icon_small);
        windowsVec->push_back(new taskButtonData(hwnd , wName , icon));
    }
    return true;
}

TaskService::TaskService() : activeBtn(nullptr)
{
    init_TaskButtons();
}

TaskService::~TaskService()
{
    activeBtn = NULL;
}

void TaskService::init_TaskButtons()
{
    taskList = new taskItemList();
    std::vector<taskButtonData*> taskbuttonDataVec;
    EnumWindows(initTaskProc , reinterpret_cast<LPARAM>(&taskbuttonDataVec));

    for (int i = 0; i < taskbuttonDataVec.size(); i++) {
        AppendTaskBtn(0 , taskbuttonDataVec.at(i)->wName.c_str() , 1 , taskbuttonDataVec.at(i)->hwnd , taskbuttonDataVec.at(i)->icon);
    }
}

bool TaskService::AppendTaskBtn(const DWORD dwFlags , LPCTSTR pszName , const UINT itemid , HWND appHwnd , HICON icon)
{
    taskEntryBtn* item = new taskEntryBtn();

    item->m_dwFlags = dwFlags;

    if (pszName)
        item->m_strName = pszName;

    item->m_icon = icon;
    item->m_data = (LPARAM)appHwnd;
    //item.m_pPopup = NULL;

    taskList->add(item);
    return true;
}

bool TaskService::removeBtn(HWND hwndToRemove)
{
    for (int i = 0; i < taskList->m_Items.size(); i++) {
        barItem* item = taskList->m_Items[i];

        if ((HWND)item->m_data == hwndToRemove) {

            taskList->m_Items.erase(taskList->m_Items.begin() + i);
            taskList->invalidate(true);
            return true;
        }
    }
    return false;
}

bool TaskService::removeBtn(int index)
{
    taskList->m_Items.erase(taskList->m_Items.begin() + index);
    taskList->invalidate(true);
    return true;
}

void TaskService::updateActiveTask(HWND hwnd , bool removeUnactive)
{
    // Checks if the activeBtn has already been set to that active window.  
    if (activeBtn != nullptr)
        if (hwnd == (HWND)activeBtn->m_data && activeBtn->m_dwFlags == taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG)
            return;

    if (activeBtn != nullptr) {
        activeBtn->m_dwFlags = 0;
        activeBtn->invalidate(true);
    }

    if (removeUnactive == true) {
        return;
    }

    for (int i = 0; i < taskList->m_Items.size(); i++) {
        barItem* taskbtn = taskList->m_Items.at(i);

        if ((HWND)taskbtn->m_data == hwnd) {
            HICON hico = ShellApi::getHICONFromHWND(hwnd , IconSizes::icon_big);

            if (hico != taskbtn->m_icon)
                taskbtn->m_icon = hico;

            taskbtn->m_dwFlags = taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG;
            activeBtn = taskbtn;
            activeBtn->invalidate(true);
            return;
        }

    }
}

bool TaskService::updateWindow(HWND hwnd)
{
    for (int i = 0; i < taskList->m_Items.size(); i++) {
        barItem* item = taskList->m_Items.at(i);

        // Checks if the task button exists
        if (hwnd == (HWND)item->m_data) {

            HICON hico = ShellApi::getHICONFromHWND(hwnd , IconSizes::icon_big);

            // updates icon if needed.
            if (hico != item->m_icon)
                item->m_icon = hico;


            // updates tooltip
            std::wstring windowText = ShellApi::getWindowTitle(hwnd);

            // if its not "ERROR" then it updates the button else it removes the button.
            if (windowText != L"ERROR")
                item->m_strName = windowText;
            else {
                OutputDebugStringA("\nremoved a button. \n");
                removeBtn(i);
            }

            taskList->invalidate(false);
            return true;
        }
    }
    return false;
}

taskItemList* TaskService::getTaskList()
{
    return taskList;
}


void TaskService::TaskWndProc(WPARAM wParam , LPARAM lparam)
{
    HWND winTaskApplication = (HWND)lparam;

    switch ((int)wParam & 0x7FFF) {
        case HSHELL_WINDOWCREATED:
        {
            std::wstring windowTitle = ShellApi::getWindowTitle(winTaskApplication);
            OutputDebugString((windowTitle + L" WINDOW CREATEDD\n").c_str()); // for debugging....

            if (isAppWindow(winTaskApplication , false)) // checks if the hwnd is not NULL and checks if the hwnd is actually a app window
            {
                AppendTaskBtn(0 , windowTitle.c_str() , 1 , winTaskApplication , ShellApi::getHICONFromHWND(winTaskApplication , IconSizes::icon_small));
                taskList->invalidate(true);
            }
        }
        break;
        case HSHELL_WINDOWDESTROYED:
        {
            removeBtn(winTaskApplication);
        }
        break;
        case HSHELL_WINDOWACTIVATED:
            if (winTaskApplication) {
                updateActiveTask(winTaskApplication);
            }
            break;
        case  HSHELL_REDRAW: // Used to update the title when the window title has been changed
            if (winTaskApplication) {
                bool ret = updateWindow(winTaskApplication);

                //if (ret == false && isAppWindow(hwnd)) {
                //    std::string windowTitle = ShellApi::getWindowTitle(hwnd);
                //    OutputDebugStringA((windowTitle + " redDAWW\n").c_str()); // for debugging....

                //    AppendTaskBtn(0 , windowTitle.c_str() , 1 , hwnd , ShellApi::getHICONFromHWND(hwnd , IconSizes::icon_small));
                //    taskList->invalidate(true);
                //}
            }
            break;


    }

}

