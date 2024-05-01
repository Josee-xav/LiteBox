#pragma once

#include <vector>
#include <Windows.h>
#include <string>
#include "barItem.h"
#include "../Services/TaskService.h"
#include "../Services/TrayService.h"
#include "../Styling.h"

#include "../MENU/FlatPopupMenu.h"
#define GWL_USERDATA -21
#define RESTART_TASKBAR (WM_USER+1)
#define EXIT_TASKBAR (WM_USER+2)
#define OPEN_STYLE_FILE (WM_USER+3)

class CTaskbar
{
public:
    TaskService taskService;
    TrayService trayService;
    PopupMenu* pmenu;

    // menu item class
    static bool        m_bClassRegistered;          // once-per-app window class registration flag

    std::vector<barItem*> m_barItemList;                     // items in this taskbar
    std::wstring        m_strFont;                   // font name for items
    int                m_FontSize;                  // font point-size for items



    HWND               m_hWnd;   //taskbar hwnd
    HWND               m_hWndCommand; // command window hwnd
    // GDI objects

    HFONT             m_hFont;
    HFONT              m_hClockFont;
    HPEN m_hBorderPen;
    HBRUSH m_hBackBrush;



private:

protected:
    void cleanup();
    void createObjects();
    bool checkOutsideWindow(const RECT& rect, short x, short y);
    int getItem(short x, short y, const RECT& rect);
    barItem* getItem(const UINT itemid, const bool bByPosition);

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool createPopupMenu();
public:
    CTaskbar();
    ~CTaskbar();

    void SetFont(std::wstring pszFont);
    void SetFontSize(const int size);

    bool create(int x, int y, HWND hWnd);

    void calc_barItemLists();

    void drawItems(HWND hWnd, HDC hDC);
    void drawBar(HWND hWnd, HDC hDC);
    void OnMouseButton(HWND hWnd, int message, WPARAM wparam, short x, short y);
    void OnDestroy(HWND hWnd);
    void OnMouseMove(HWND hWnd, short x, short y);
};




// Set the font

inline void CTaskbar::SetFont(std::wstring pszFont)
{
    m_strFont = pszFont;
}


// Set the font size

inline void CTaskbar::SetFontSize(const int size)
{
    m_FontSize = size;
}



