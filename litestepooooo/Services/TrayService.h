#pragma once
#include <vector>
#include "../Taskbar/taskbarItemClasses.h"

#include "../WinShellFunctionality/TrayShit.h"
typedef int ( *trayHookDll_EntryFunc )( HWND );


class TrayService
{
public:
    TrayService();
    ~TrayService();

    trayItemList* getTrayList();
    std::vector<TrayEntryBtn::_TrayItem* > getTrayItems();
    bool AppendTrayBtn(const DWORD dwFlags, LPCTSTR pszName, HWND appHwnd, HICON icon, TrayEntryBtn::_TrayItem* trayItem);
    static LRESULT CALLBACK trayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK TrayNotifyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void trayEvent(void* data, unsigned size);
    void extractTrayData(NIDBB* nid, void* trayData);
private:
    static trayItemList* trayBtnList;
    HWND create_Tray_Child(HWND hwndParent, const wchar_t* class_name);
    void initTrayService();

    static trayHookDll_EntryFunc th_libfunc;
    static HWND hTrayWnd;
    static HINSTANCE hinstTrayLib;

};