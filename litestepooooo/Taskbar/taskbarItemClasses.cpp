#include "taskbarItemClasses.h"
#include "CTaskbar.h"

#include "../UsefulShellApi.h"
#include "../drawingApi.h"
#include "../Services/TrayService.h"
#pragma warning(disable : 4996)

// FOR DEBUGGING.
//void drawwww(barItem* r , HDC hDC)
//{
//    HGDIOBJ hOldBrush , hOldPen = 0;
//
//    HPEN m_hBorderPen = CreatePen(PS_SOLID , 1 , RGB(0 , 255 , 0));
//
//    hOldBrush = SelectObject(hDC , r->mainbar->m_hBackBrush);
//    hOldPen = SelectObject(hDC , m_hBorderPen);
//
//    // draws border and the back of it
//    Rectangle(hDC , r->itemRect.left , r->itemRect.top , r->itemRect.right , r->itemRect.bottom);
//
//    SelectObject(hDC , hOldBrush);
//    SelectObject(hDC , hOldPen);
//
//
//    DeleteObject(m_hBorderPen);
//}



//////////////////

taskEntryBtn::taskEntryBtn() : barItem(M_TASK)
{
}

taskEntryBtn::~taskEntryBtn()
{
}

void taskEntryBtn::draw(HWND hWnd , HDC hDC)
{
    bool    bSelected = (m_dwFlags == taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG);
    HGDIOBJ hOldPen , hOldBrush , hOldFont;
    int     h{ 0 };
    COLORREF color;

    RECT drawingRect;
    int margin = mainbar->m_Style.taskbuttonTopSpacing;//Margins create extra space around an element
    drawingRect.left = itemRect.left;
    drawingRect.top = margin;
    drawingRect.right = m_Xpos + itemRect.right;
    drawingRect.bottom = itemRect.bottom - (margin);


    HBRUSH taskBrush = CreateSolidBrush(bSelected ? mainbar->m_Style.taskFocusColor : mainbar->m_Style.windowBackgroundColor);
    HPEN taskPen = CreatePen(PS_SOLID , 1 , mainbar->m_Style.borderColor);

    // draw/clear the selection rectangle
    hOldBrush = SelectObject(hDC , taskBrush);
    hOldPen = SelectObject(hDC , mainbar->m_Style.taskBevelStyle == 1 ? taskPen : GetStockObject(NULL_PEN)); // allows the pen to make a bevel for us so its easier when its rounded for example.

    if (mainbar->m_Style.rectRoundedEdgeTaskButtons != true)
        Rectangle(hDC , drawingRect.left , drawingRect.top , drawingRect.right , drawingRect.bottom);
    else
        RoundRect(hDC , drawingRect.left , drawingRect.top , drawingRect.right , drawingRect.bottom , 5 , 5);

    //mainbar->m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
    if (mainbar->m_Style.taskBevelStyle != 0 && mainbar->m_Style.taskBevelStyle != 1)
        DrawingApi::drawBevel(hDC , mainbar->m_Style.borderColor , &drawingRect , mainbar->m_Style.taskBevelStyle , BF_RECT);

    SelectObject(hDC , hOldPen);
    SelectObject(hDC , hOldBrush);

    // is there an icon?
    if (m_icon != NULL) {
        drawingRect.left += 3; // spacing

        int iconSize = ((drawingRect.bottom - drawingRect.top) - 4);

        DrawIconEx(
            hDC ,
            drawingRect.left ,
            ((drawingRect.bottom + drawingRect.top) / 2) - (iconSize / 2) ,
            m_icon ,
            iconSize ,
            iconSize ,
            0 , NULL , DI_NORMAL);

    }



    // add spacing for text beside the icon
    if (m_icon != NULL)
        drawingRect.left += (drawingRect.bottom - drawingRect.top); // add the width of the icon for the basically the x cord for the text.

    if ((m_dwFlags & taskbarItemFlags::itemBold) != 0) {
        h = mainbar->m_BoldFontHeight;
        hOldFont = SelectObject(hDC , mainbar->m_hBoldFont);
    }
    else {
        h = mainbar->m_FontHeight;
        hOldFont = SelectObject(hDC , mainbar->m_hFont);
    }
    SetBkMode(hDC , TRANSPARENT);

    if (bSelected)
        color = mainbar->m_Style.focusedTextColor;
    else
        color = mainbar->m_Style.unfocusedTextColor;

    SetTextColor(hDC , color);
    DrawText(hDC , m_strName.c_str() , -1 , &drawingRect , DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
    SelectObject(hDC , hOldFont);

    DeleteObject(taskBrush);
    DeleteObject(taskPen);
}


void taskEntryBtn::mouse_event(int mx , int my , int message , unsigned flags)
{
    HWND taskAppHwnd = (HWND)m_data;
    DWORD_PTR dwResult = 0;

    if (message == WM_LBUTTONDOWN) {
        SetForegroundWindow(taskAppHwnd);
        // sends WM_SYSCOMMAND the parm SC_RESTORE which is kinda like right clicking the taskbutton on the normal windows shell and clicking open or something
        SendMessageTimeout(taskAppHwnd , WM_SYSCOMMAND , SC_RESTORE , 0 , 2 , 500 , &dwResult);
    }
    else if (message == WM_RBUTTONDOWN) {
        SendMessageTimeout(taskAppHwnd , WM_SYSCOMMAND , SC_MINIMIZE , 0 , 2 , 500 , &dwResult);

    }
}

/// <summary>
/// TRAY ENTRY BTN
/// </summary>
/// <param name="bar"></param>

void TrayEntryBtn::trayMouseDown(int message)
{
    TrayItem* ni = (_TrayItem*)m_data;

    OutputDebugStringA(ShellApi::getWindowClassName(ni->hWnd).c_str());

    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd , &procId);
    AllowSetForegroundWindow(procId);
    if (message == WM_RBUTTONDOWN)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , WM_RBUTTONDOWN | (ni->uID << 16));
    else if (message == WM_LBUTTONDOWN)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , WM_LBUTTONDOWN | (ni->uID << 16));
}

void TrayEntryBtn::trayMouseUp(int message)
{
    TrayItem* ni = (_TrayItem*)m_data;

    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd , &procId);
    AllowSetForegroundWindow(procId);
    if (message == WM_LBUTTONUP)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , NIN_SELECT | (ni->uID << 16));
    else if (message == WM_RBUTTONUP)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , WM_CONTEXTMENU | (ni->uID << 16));

}

void TrayEntryBtn::trayMouseDOUBLECLICKDown(int message)
{
    _TrayItem* ni = (_TrayItem*)m_data;

    DWORD procId = 0;
    GetWindowThreadProcessId(ni->hWnd , &procId);
    if (message == WM_LBUTTONDBLCLK)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , WM_LBUTTONDBLCLK | (ni->uID << 16));
    else if (message == WM_RBUTTONDBLCLK)
        SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , WM_RBUTTONDBLCLK | (ni->uID << 16));
}

TrayEntryBtn::TrayEntryBtn() : barItem(M_TRAY)
{
}

TrayEntryBtn::~TrayEntryBtn()
{
}

void TrayEntryBtn::draw(HWND hWnd , HDC hDC)
{
    HGDIOBJ hOldPen , hOldBrush;
    int      h{ 0 };

    RECT drawingRect;
    int y = mainbar->m_Style.borderWidth;
    drawingRect.left = itemRect.left;
    drawingRect.top = y;
    drawingRect.right = m_Xpos + itemRect.right;
    drawingRect.bottom = itemRect.bottom - (y);

    // draw/clear the selection rectangle
    HBRUSH brush = CreateSolidBrush(mainbar->m_Style.windowBackgroundColor);
    hOldBrush = SelectObject(hDC , brush);
    hOldPen = SelectObject(hDC , GetStockObject(NULL_PEN));

    Rectangle(hDC , drawingRect.left , drawingRect.top , drawingRect.right , drawingRect.bottom);
    // is there an icon?
    if (m_icon != NULL) {
        int ypos = (itemRect.bottom / 2) - (16 / 2);

        DrawIconEx(
            hDC ,
            drawingRect.left ,
            ypos , // centers the icon
            m_icon ,
            16 ,
            16 ,
            0 , NULL , DI_NORMAL);

    }

    SelectObject(hDC , hOldPen);
    SelectObject(hDC , hOldBrush);
    DeleteObject(brush);
}

void TrayEntryBtn::mouse_event(int mx , int my , int message , unsigned flags)
{
    switch (message) {
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            trayMouseUp(message);
            break;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            trayMouseDown(message);
        }
        break;
        //case WM_RBUTTONDBLCLK:
        //case WM_LBUTTONDBLCLK:
        //    trayMouseDOUBLECLICKDown(message);
        //    break;
    }
}

// BARITEMLIST



int baritemlist::getItem(short x , short y , const RECT& rect)
{

    int newitem;
    std::vector<barItem*>::size_type i;

    if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom)
        newitem = -1;
    else {
        for (i = 0; i < m_Items.size(); i++) {
            if (x > m_Items[i]->m_Xpos && x < m_Items[i]->m_Xpos + m_Items[i]->itemRect.right)
                break;
        }

        newitem = i == m_Items.size() ? -1 : i;
        if (newitem != -1 && (m_Items[newitem]->m_dwFlags & taskbarItemFlags::itemNotSelectable) != 0)
            newitem = -1;
    }

    return newitem;

}
bool baritemlist::calc_itemsSizes()
{
    int ts = m_Items.size();

    if (0 == ts)
        return false;

    int b = defaultStyle::TASKBAR_GENERAL_SPACING;
    int w = itemRect.right - itemRect.left + b;
    int h = itemRect.bottom - itemRect.top;
    int xpos = itemRect.left;

    int is = h + b;
    int min_width = is / 2;
    int max_width = max(w * defaultStyle::ICON_SIZE / 100 , is);
    if (w / ts >= max_width)
        w = ts * max_width;

    int n = 0;
    for (int i = 0; i < m_Items.size(); i++) {
        int left , right;
        {
            left = xpos + w * n / ts;
            right = xpos + w * (n + 1) / ts - b;
            if (right - left < min_width)
                right = left + min_width;
        }

        if (right > itemRect.right)
            break;

        m_Items[i]->calc_size(&left , itemRect.top , right - left , h , 0);
        ++n;
    }
    return TRUE;
}
baritemlist::baritemlist(int type) : barItem(type)
{

}

baritemlist::~baritemlist()
{
    clear();
}

void baritemlist::add(barItem* entry)
{
    m_Items.push_back(entry);
}

void baritemlist::remove(barItem* entry)
{

}

void baritemlist::clear(void)
{
    for (int i = 0; i < m_Items.size(); i++) {
        delete m_Items.at(i);
    }
    m_Items.clear();
}


void baritemlist::draw(HWND hWnd , HDC hDC)
{
    //drawwww(this , hDC);
    for (int i = 0; i < m_Items.size(); i++) {
        m_Items.at(i)->draw(hWnd , hDC);
    }
    //InvalidateRect(hWnd, NULL, FALSE);
}

void baritemlist::mouse_event(int mx , int my , int message , unsigned flags)
{
    RECT            rect;
    HDC             hDC;
    int              newitem;
    POINT           p;

    // get a DC and the window rectangle
    HWND mainbarHwnd = mainbar->m_hWnd;
    GetClientRect(mainbarHwnd , &rect);
    hDC = GetDC(mainbarHwnd);

    // shift tracking if we're over another menu

    p.x = mx;
    p.y = my;
    ClientToScreen(mainbarHwnd , &p);

    // get new selected item, if any
    newitem = getItem(mx , my , rect);

    switch (message) {
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        {
            if (newitem != -1)
                m_Items[newitem]->mouse_event(mx , my , message , 0);
        }
        break;

    }
}

bool baritemlist::calc_size(int* px , int y , int w , int h , int m)
{
    int x = *px;
    bool f = false;

    if (itemRect.left != x)
        itemRect.left = x , f = true;
    x += w;
    if (itemRect.right != x)
        itemRect.right = x , f = true;
    *px = x;

    itemRect.top = y;
    itemRect.bottom = h;

    calc_itemsSizes();

    return true;
}


bool taskItemList::calc_itemsSizes()
{
    int ts = m_Items.size();

    if (0 == ts)
        return false;

    int b = 1 + mainbar->m_Style.taskbuttonSpacing;
    int w = itemRect.right - itemRect.left;
    int h = itemRect.bottom - itemRect.top;
    int xpos = itemRect.left + b;

    int is = h + b;
    int min_width = is / 2;
    int max_width = max(w * defaultStyle::MAX_TASKWIDTH / 100 , is);
    if (w / ts >= max_width)
        w = ts * max_width;

    int n = 0;
    for (int i = 0; i < m_Items.size(); i++) {
        int left , right;
        {
            left = xpos + w * n / ts;
            right = xpos + w * (n + 1) / ts - b;
            if (right - left < min_width)
                right = left + min_width;
        }

        if (right > itemRect.right)
            break;

        m_Items[i]->calc_size(&left , itemRect.top , right - left , h , 0);
        ++n;
    }
    return TRUE;
}

taskItemList::taskItemList() : baritemlist(M_TASKLIST)
{
}

void taskItemList::remove(barItem* entry)
{
    for (int i = 0; i < m_Items.size(); i++) {
        barItem* item = m_Items[i];

        if ((HWND)item->m_data == (HWND)entry->m_data) {

            m_Items.erase(m_Items.begin() + i);
            invalidate(true);

        }
    }
}


trayItemList::trayItemList() : baritemlist(M_TRAYLIST)
{
}

std::string clockBtn::convert24hourTo12HourTime(int time24Hour , int time24minute)
{
    char buff[20];
    switch (time24Hour) {
        case 12:
            sprintf(buff , "%02d:%02d PM" , time24Hour , time24minute);
            break;
        case 00:
            sprintf(buff , "%02d:%02d AM" , time24Hour + 12 , time24minute);
            break;
        default:
        {
            if (time24Hour > 12) {
                sprintf(buff , "%02d:%02d PM" , time24Hour - 12 , time24minute);

            }
            else {
                sprintf(buff , "%02d:%02d AM" , time24Hour , time24minute);

            }
            break;
        }
        break;
    }
    std::string ret = buff;
    return ret;
}

void clockBtn::updateClock(clockFormat format)
{
    std::string formatTime;

    // gets the date and time 
    SYSTEMTIME dateAndTime;
    GetLocalTime(&dateAndTime);
    char buff[20];

    if (format == clockFormat::format_12hour) {
        formatTime = convert24hourTo12HourTime(dateAndTime.wHour , dateAndTime.wMinute);;
        if (formatTime != m_strName) {// so it doesnt set time when its not necessary
            m_strName = formatTime;
            invalidate(1);
        }
    }
    else {
        sprintf(buff , "%02d:%02d" , dateAndTime.wHour , dateAndTime.wMinute);

        formatTime = buff;
        if (formatTime != m_strName) {// so it doesnt set time when its not necessary
            m_strName = formatTime;
            invalidate(1);
        }
    }
}

LRESULT clockBtn::timerProc(HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam)
{
    switch (msg) {

        case WM_TIMER:
        {
            ((clockBtn*)GetWindowLongPtr(hwnd , GWLP_USERDATA))->updateClock(static_cast<clockFormat>(mainbar->m_Style.clockTimeFormat));

            SYSTEMTIME time;
            GetLocalTime(&time);
            SetTimer(mainbar->m_hWnd , 333 , 1100 - time.wMilliseconds , 0);
        }
        break;
        case WM_DESTROY:
        {

        }
    }

    return DefWindowProc(hwnd , msg , wParam , lParam);
}

bool clockBtn::classRegistered = false;

clockBtn::clockBtn() : barItem(M_CLOCK)
{
    setTimer();
}

clockBtn::~clockBtn()
{
    DestroyWindow(messageWindow);
}

void clockBtn::setTimer()
{
    if (!classRegistered) {
        WNDCLASS wc = { };

        wc.lpfnWndProc = timerProc;
        wc.hInstance = mainbar->m_hInstance;
        wc.lpszClassName = "clockproo";

        if (!RegisterClass(&wc)) {
            return;
        }
        classRegistered = true;
    }

    // Create the window.

    messageWindow = CreateWindow("clockproo" , 0 , 0 , 0 , 0 , 0 , 0 , HWND_MESSAGE , 0 , 0 , 0);
    SetWindowLongPtr(messageWindow , GWLP_USERDATA , (LONG_PTR)this); // sets the GWLP_USERDATA to the object so we can get it easily via the HWND 

    SYSTEMTIME lt;
    GetLocalTime(&lt);
    SetTimer(messageWindow , CLOCK_TIMER , 0 , 0);
}

void clockBtn::draw(HWND hWnd , HDC hDC)
{
    HGDIOBJ hOldPen , hOldBrush , hOldFont;
    int      h{ 0 };
    COLORREF color;

    RECT drawingRect;
    int y = mainbar->m_Style.borderWidth;
    drawingRect.left = itemRect.left;
    drawingRect.top = y;
    drawingRect.right = m_Xpos + itemRect.right;
    drawingRect.bottom = itemRect.bottom - (y);

    // draw/clear the selection rectangle
    HBRUSH background = CreateSolidBrush(mainbar->m_Style.clockColor != CLR_INVALID ? mainbar->m_Style.clockColor : mainbar->m_Style.windowBackgroundColor);
    HPEN borderPen = CreatePen(PS_SOLID , 1 , mainbar->m_Style.borderColor);

    hOldBrush = SelectObject(hDC , background);
    hOldPen = SelectObject(hDC , mainbar->m_Style.clockBevelStyle == 1 ? borderPen : GetStockObject(NULL_PEN)); // allows the pen to make a bevel for us so its easier when its rounded for example.

    if (mainbar->m_Style.rectRoundedEdgeClock != true)
        Rectangle(hDC , drawingRect.left , drawingRect.top , drawingRect.right , drawingRect.bottom);
    else
        RoundRect(hDC , drawingRect.left , drawingRect.top , drawingRect.right , drawingRect.bottom , 4 , 4);

    //mainbar->m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
    if (mainbar->m_Style.clockBevelStyle != 0 && mainbar->m_Style.clockBevelStyle != 1)
        DrawingApi::drawBevel(hDC , mainbar->m_Style.borderColor , &drawingRect , mainbar->m_Style.clockBevelStyle , BF_RECT);

    SelectObject(hDC , hOldPen);
    SelectObject(hDC , hOldBrush);

    if ((m_dwFlags & taskbarItemFlags::itemBold) != 0) {
        h = mainbar->m_BoldFontHeight;
        hOldFont = SelectObject(hDC , mainbar->m_hBoldFont);
    }
    else {
        h = mainbar->m_FontHeight;
        hOldFont = SelectObject(hDC , mainbar->m_hFont);
    }
    SetBkMode(hDC , TRANSPARENT);

    color = mainbar->m_Style.focusedTextColor;

    SetTextColor(hDC , color);
    DrawText(hDC , m_strName.c_str() , -1 , &drawingRect , DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
    SelectObject(hDC , hOldFont);

    DeleteObject(background);
    DeleteObject(borderPen);
}

void clockBtn::mouse_event(int mx , int my , int message , unsigned flags)
{
}
