#pragma once
#include <vector>
#include "../Taskbar/taskbarItemClasses.h"

class TrayService
{
public:
    TrayService();
    ~TrayService();

    trayItemList* getTrayList();
private:
    trayItemList* trayBtnList;

    bool AppendTrayBtn(const DWORD dwFlags , LPCTSTR pszName , const UINT itemid , HWND appHwnd , HICON icon , TrayEntryBtn::_TrayItem* trayItem);

    void initTrayService();

    std::vector<TrayEntryBtn::_TrayItem* > getTrayItems();

};