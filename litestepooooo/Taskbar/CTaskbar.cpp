#include "CTaskbar.h"
#include "../Logging/FLogger.h"
#include "../Utils.h"
#include "taskbarItemClasses.h"

// initialise the static member

bool CTaskbar::m_bClassRegistered = false;

/***************/
/* Constructor */
/***************/

CTaskbar::CTaskbar() : pmenu(nullptr)
{

    // GDI objects

    m_hFont = NULL;
    m_hClockFont = NULL;
    m_hWnd = NULL;

    barItem::mainbar = this;

    createPopupMenu();

    // Add bar item here
    m_barItemList.push_back(taskService.getTaskList());
    m_barItemList.push_back(trayService.getTrayList());
    m_barItemList.push_back(new clockBtn());
}

/**************/
/* Destructor */
/**************/

CTaskbar::~CTaskbar()
{

    cleanup();

    // clean up the window

    if(m_hWnd)
        DestroyWindow(m_hWnd);

}

/***********************/
/* Cleanup GDI objects */
/***********************/

void CTaskbar::cleanup()
{
    if(m_hBackBrush) {
        DeleteObject(m_hFont);
        DeleteObject(m_hClockFont);
        DeleteObject(m_hBorderPen);
        DeleteObject(m_hBackBrush);

        m_hFont = NULL;
        m_hClockFont = NULL;
        m_hBorderPen = NULL;
        m_hBackBrush = NULL;

        for(int i = 0; i < m_barItemList.size(); i++) {
            barItem* bi = m_barItemList.at(i);
            delete bi;
            bi = nullptr;
        }
        m_barItemList.clear();

        delete pmenu;
        pmenu = nullptr;
    }
}

/**********************/
/* Create GDI objects */
/**********************/

void CTaskbar::createObjects(void)
{
    LOGFONT lf;
    HDC hDC;
    TEXTMETRIC tm;

    // default font and size

    m_strFont = m_Style.font;
    m_FontSize = m_Style.task_fontSize;

    m_hBorderPen = CreatePen(PS_SOLID, m_Style.border_Width, m_Style.border_Width == 0 ? m_Style.windowBackgroundColor
                             : m_Style.borderColor);
    m_hBackBrush = CreateSolidBrush(m_Style.windowBackgroundColor);

    // create normal  font

    ZeroMemory(&lf, sizeof(lf));

    hDC = CreateICA("DISPLAY", NULL, NULL, NULL);
    lf.lfHeight = m_FontSize;

    lf.lfWeight = m_Style.fontWeight * 100; // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
    lstrcpyn(lf.lfFaceName, m_strFont.c_str(),
             sizeof(lf.lfFaceName) / sizeof(TCHAR));
    m_hFont = ::CreateFontIndirect(&lf);

    lf.lfHeight = m_Style.clock_fontSize;
    lf.lfWeight = 100;
    m_hClockFont = ::CreateFontIndirect(&lf);

    // get the font heights

    HGDIOBJ hOldFont;
    hOldFont = SelectObject(hDC, m_hFont);
    GetTextMetrics(hDC, &tm);

    hOldFont = SelectObject(hDC, m_hClockFont);
    GetTextMetrics(hDC, &tm);

    // finished with the DC

    DeleteDC(hDC);
}

void CTaskbar::calc_barItemLists()
{
    int taskzone_width, clock_width = 0;

    int trayWidth = 0;
    RECT mainbarRect;
    GetClientRect(m_hWnd, &mainbarRect);

    int xpos = m_Style.border_Width;
    trayItemList* tl = trayService.getTrayList();
    if(tl != NULL)
        trayWidth = 20 * tl->m_Items.size();
    // --- 1st pass ----------------------------------
    for(int i = 0; i < m_barItemList.size(); i++) {
        barItem* bi = m_barItemList.at(i);

        switch(bi->buttonType) {
            case M_CLOCK:
                clock_width = 50;
                xpos += clock_width;
                break;

            case M_TASKLIST:
                break;

            case M_TRAYLIST:
                xpos += trayWidth;
                break;
        }
    }

    // assign variable widths
    int rest_width = max(0, mainbarRect.right - xpos); // for the taskzone.

    int top = 0;

    // 2nd passsss

    taskzone_width = rest_width;
    FLogger::debug("Calc.... Tray width:   : %i    taskzone width: %i", trayWidth, taskzone_width);

    xpos = 0;
    int ypos = 0;
    for(int i = 0; i < m_barItemList.size(); i++) {

        barItem* bi = m_barItemList.at(i);

        int width = 0;

        switch(bi->buttonType) {
            case M_TRAYLIST:
                width = trayWidth;
                break;
            case M_CLOCK:
                width = 50;
                break;
            case M_TASKLIST:
                width = taskzone_width;

                break;
        }

        bi->calc_size(&xpos, ypos, width, mainbarRect.bottom, ypos - top);
    }
    return;
}

bool CTaskbar::create(int x, int y, HWND hWnd)
{
    // register the class
    if(!m_bClassRegistered) {
        if(!LB_Api::register_class(L"LiteBox_TaskBar", LB_Api::main_hinstance,windowProc, 0)) {
            MessageBoxA(NULL, "LiteBox_TaskBar register class failed." + __LINE__, ".", S_OK);
            return false;
        }

        m_bClassRegistered = true;
    }

    // create GDI objects
    createObjects();

    // get the screen dimensions and adjust x and y if necessary
    RECT rcScreen;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

    if(x + m_Style.taskbar_Width > rcScreen.right) {
        x = max(0, rcScreen.right - m_Style.taskbar_Width);
        m_Style.taskbar_Width = rcScreen.right;
    }

    // create the window
    m_hWnd = ::CreateWindowExA(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST, // prevent button appearing on taskbar
        "LiteBox_TaskBar", "",
        WS_POPUP | WS_VISIBLE, x, y, m_Style.taskbar_Width,
        m_Style.taskbar_Height + m_Style.border_Width, NULL, NULL,
        LB_Api::main_hinstance, this);
    // check created OK

    if(m_hWnd == NULL) {
        MessageBoxA(NULL, "LiteBox_TaskBar register class failed." + __LINE__, ".", S_OK);
        FLogger::error("LiteBox_TaskBar register class failed. %s", __LINE__);

        return false;
    }

    ShowWindow(m_hWnd, SW_SHOW);

    m_hWndCommand = hWnd;

    return true;
}

/*****************************/
/* Register the window class */
/*****************************/

// WM_PAINT


void CTaskbar::OnPaint(HWND hWnd, HDC hDC)
{
    calc_barItemLists();

    // draw each item
    for(int i = 0; i < m_barItemList.size(); i++) {
        barItem* it = m_barItemList.at(i);
        it->draw(hWnd, hDC);
    }
}

/*************************/
/* stub window procedure */
/*************************/

LRESULT CALLBACK CTaskbar::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam)
{

    // the usual window procedure...
    CTaskbar* pClass = ( (CTaskbar*)GetWindowLongPtr(hWnd, GWLP_USERDATA) );
    switch(uMsg) {
        case WM_CREATE:
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)( (LPCREATESTRUCT)lParam )->lpCreateParams);
            SetTimer(hWnd, 0, 2000, 0);
            break;
        case WM_ERASEBKGND:

            pClass->OnEraseBkgnd(hWnd, (HDC)wParam);
            break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
            pClass->OnMouseButton(hWnd, uMsg, wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:

            pClass->OnMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            BeginPaint(hWnd, &ps);

            pClass->OnPaint(hWnd, ps.hdc);
            // CTaskbar::p_rcPaint = &ps.rcPaint;

            EndPaint(hWnd, &ps);
        } break;


        case WM_TIMER:
        {
            if(wParam == 0) // check full screen app timer
            {
                bool hideWindow = LB_Api::isFullscreen(GetForegroundWindow());

                ShowWindow(hWnd, hideWindow ? SW_HIDE : SW_SHOW);

            }
        } break;
        case WM_CLOSE: // if the user alt f4's the taskbar, it then proceeds to gracefully close.
        {
            SendMessage(pClass->m_hWndCommand, EXIT_TASKBAR, 0, 0);
        }
        break;
        case WM_DESTROY:
        {
            pClass->OnDestroy(hWnd);
            break;
        }

    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool CTaskbar::createPopupMenu()
{
    pmenu = new PopupMenu;
    // set some of the colors
    pmenu->SetColor(PopupMenu::colorBorder, m_Style.border_Width == 0
                    ? m_Style.windowBackgroundColor
                    : m_Style.borderColor);
    pmenu->SetColor(PopupMenu::colorText, m_Style.unfocusedTextColor);
    pmenu->SetColor(PopupMenu::colorHighlightText, m_Style.focusedTextColor);
    pmenu->SetColor(PopupMenu::colorBackground, m_Style.windowBackgroundColor);
    pmenu->SetColor(PopupMenu::colorHighlight, m_Style.task_FocusColor);
    pmenu->SetFont(m_Style.font.c_str());
    pmenu->SetFontSize(m_Style.menu_fontSize);

    // create it

    pmenu->Create(LB_Api::main_hinstance);

    // add some items

    pmenu->AppendItem(PopupMenu::itemBold | PopupMenu::itemNotSelectable,
                      L"LiteBox", 0);
    pmenu->AppendItem(PopupMenu::itemSeparator, NULL, 0);
    pmenu->AppendItem(0, L"Exit LiteBox", 1);
    pmenu->AppendItem(0, L"Restart LiteBox", 2);
    pmenu->AppendItem(0, L"Open Style File", 3);

    return true;
}

/**************/
/* WM_DESTROY */
/**************/

void CTaskbar::OnDestroy(HWND hWnd)
{
    KillTimer(hWnd, 0);

    m_hWnd = NULL;
}

/****************/
/* WM_MOUSEMOVE */
/****************/

void CTaskbar::OnMouseMove(HWND hWnd, short x, short y)
{
    RECT rect;
    int newitem;
    POINT p;

    // get a DC and the window rectangle

    GetClientRect(hWnd, &rect);

    // shift tracking if we're over another

    p.x = x;
    p.y = y;
    ClientToScreen(hWnd, &p);

    // get new selected item, if any
    newitem = getItem(x, y, rect);

    if(newitem != -1)
        m_barItemList.at(newitem)->mouse_event(x, y, WM_MOUSEMOVE, 0);
}

/************************/
/* Get item at position */
/************************/

int CTaskbar::getItem(short x, short y, const RECT& rect)
{
    int newitem;
    std::vector<baritemlist*>::size_type i;

    if(x <= 1 || y <= 1 || x > rect.right || y > rect.bottom)
        newitem = -1;
    else {
        for(i = 0; i < m_barItemList.size(); i++) {
            if(x > 1 && x < m_barItemList.at(i)->itemRect.right)
                break;
        }

        newitem = i == m_barItemList.size() ? -1 : i;
    }

    return newitem;
}

/****************/
/* WM_LBUTTONUP */
/****************/

void CTaskbar::OnMouseButton(HWND hWnd, int message, WPARAM wparam, short x,
                             short y)
{
    RECT rect;

    if(message == WM_RBUTTONDOWN && (wparam & MK_SHIFT )) {

        POINT pos;
        GetCursorPos(&pos);

        switch(pmenu->Track(pos.x, pos.y, NULL, true)) {
            case 1:
                SendMessage(m_hWndCommand, EXIT_TASKBAR, 0, 0);
                return;

            case 2:
                SendMessage(m_hWndCommand, RESTART_TASKBAR, 0, 0);
                // todo
                return;
            case 3:
                SendMessage(m_hWndCommand, OPEN_STYLE_FILE, 0, 0);

                break;
        }

        if(pmenu != NULL)
            pmenu->hidePopUp();
        return;
    }

    GetClientRect(hWnd, &rect);
    if(checkOutsideWindow(rect, x, y))
        return;
    // shift tracking if we're over another
    POINT p;
    p.x = x;
    p.y = y;
    ClientToScreen(hWnd, &p);

    // get new selected item, if any

    int newitem = getItem(x, y, rect);

    if(newitem != -1)
        m_barItemList.at(newitem)->mouse_event(x, y, message, 0);
}

/*************************/
/* Check if outside window */
/*************************/
/// returns true if its outside
bool CTaskbar::checkOutsideWindow(const RECT& rect, short x, short y)
{
    if(x <= 2 || y <= 2 || x > rect.right || y > rect.bottom) {
        // is out
        return true;
    }
    // is in
    return false;
}

/*****************/
/* WM_ERASEBKGND */
/*****************/

void CTaskbar::OnEraseBkgnd(HWND hWnd, HDC hDC)
{
    RECT rect;
    HGDIOBJ hOldBrush, hOldPen = 0;

    GetClientRect(hWnd, &rect);
    hOldBrush = SelectObject(hDC, m_hBackBrush);
    hOldPen = SelectObject(hDC, m_hBorderPen);

    // draws border and the back of it
    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hOldPen);
}

/****************/
/* Find an item */
/****************/

barItem* CTaskbar::getItem(const UINT itemid, const bool bByPosition)
{
    std::vector<baritemlist*>::iterator it;

    if(bByPosition) {
        if(itemid >= m_barItemList.size())
            return NULL;

        return m_barItemList.at(itemid);
    }
    else {
        for(int i = 0; i < m_barItemList.size(); i++) {
            barItem* it = m_barItemList.at(i);
            if(it->m_ItemID == itemid)
                return &( *it );
        }
        return NULL;
    }
}
