#pragma once
#include "../Taskbar/taskbarItemClasses.h"

struct taskButtonData
{
    HWND hwnd;
    std::string wName;
    HICON icon;

    taskButtonData(HWND h = NULL , std::string n = "" , HICON i = NULL)
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
    void TaskWndProc(WPARAM wParam , LPARAM lparam);
private:
    taskItemList* taskList;
    barItem* activeBtn;

    void init_TaskButtons();

    bool removeBtn(HWND hwndToRemove);
    bool removeBtn(int index);

    bool AppendTaskBtn(const DWORD dwFlags , LPCTSTR pszName , const UINT itemid , HWND appHwnd , HICON icon);
    void updateActiveTask(HWND hwnd);
    // returns true if it was able to find the window in the list. false otherwise
    bool updateWindow(HWND hwnd);
};
