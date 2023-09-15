#pragma once
#include "../Taskbar/taskbarItemClasses.h"
#include "../WinShellFunctionality/ShellHookingFunctionality.h"

#define UWP_APP_UNCLOAKED (WM_USER + 44)
struct taskButtonData
{
    HWND hwnd;
    std::wstring wName;
    HICON icon;

    taskButtonData(HWND h = NULL, std::wstring n = L"", HICON i = NULL)
    {
        hwnd = h;
        wName = n;
        icon = i;
    }
};


class TaskService
{
public:
    TaskService();
    ~TaskService();

    taskItemList* getTaskList();
    void TaskWndProc(WPARAM wParam, LPARAM lparam);
    void updateActiveTask(HWND hwnd, bool removeUnactive = FALSE);

    bool closeWindow(HWND hwnd);
private:
    taskItemList* taskList;
    barItem* activeBtn;

    void init_TaskButtons();

    bool removeBtn(HWND hwndToRemove);
    bool removeBtn(int index);

    bool AppendTaskBtn(const DWORD dwFlags, LPCTSTR pszName, const UINT itemid, HWND appHwnd, HICON icon);
    // returns true if it was able to find the window in the list. false otherwise
    bool updateWindow(HWND hwnd);

};
