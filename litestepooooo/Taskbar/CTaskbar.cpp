#include "CTaskbar.h"
#include "taskbarItemClasses.h"
#include "../UsefulShellApi.h"
#include "../MENU/FlatPopupMenu.h"


// initialise the static member

bool CTaskbar::m_bClassRegistered = false;

/***************/
/* Constructor */
/***************/

CTaskbar::CTaskbar()
{

    // GDI objects

    m_hFont = NULL;
    m_hBoldFont = NULL;

    // other internal variables
    m_State = stateInactive;
    m_hWnd = NULL;

    barItem::mainbar = this;

    m_barItemList.push_back(taskService.getTaskList());
    m_barItemList.push_back(trayService.getTrayList());
    m_barItemList.push_back(new clockBtn());
}


/**************/
/* Destructor */
/**************/

CTaskbar::~CTaskbar()
{
    // if we're tracking, release it
    OutputDebugStringA("destructor ctaskabr\n");
    if (m_State == stateTrack)
        ReleaseCapture();

    // clean up the window

    if (m_hWnd)
        DestroyWindow(m_hWnd);

    // clean up the GDI objects

    cleanup();
}





/***********************/
/* Cleanup GDI objects */
/***********************/

void CTaskbar::cleanup(void)
{
    if (m_hFont) {
        DeleteObject(m_hFont);
        DeleteObject(m_hBoldFont);
        DeleteObject(m_hBorderPen);
        DeleteObject(m_hBackBrush);


        m_hFont = NULL;
        m_hBoldFont = NULL;
        m_hBorderPen = NULL;
        m_hBackBrush = NULL;



        for (int i = 0; i < m_barItemList.size(); i++) {
            barItem* bi = m_barItemList.at(i);
            delete bi;
            bi = nullptr;
        }
        m_barItemList.clear();
    }
}


/**********************/
/* Create GDI objects */
/**********************/

void CTaskbar::createObjects(void)
{
    LOGFONT    lf;
    HDC        hDC;
    TEXTMETRIC tm;

    // default font and size

    m_strFont = L"MS Sans Serif"; // TODO. could allow the user to change fonts but
    m_FontSize = 10; // TODO: make it so it automatically adjusts depending on the margin???


    // cleanup any previous

    cleanup();

    m_hBorderPen = CreatePen(PS_SOLID , m_Style.borderWidth , m_Style.borderWidth == 0 ? m_Style.windowBackgroundColor : m_Style.borderColor);
    m_hBackBrush = CreateSolidBrush(m_Style.windowBackgroundColor);

    // create normal  font

    ZeroMemory(&lf , sizeof(lf));


    hDC = CreateICA("DISPLAY", NULL, NULL, NULL);
    lf.lfHeight = -MulDiv(m_FontSize, GetDeviceCaps(hDC, LOGPIXELSY), 92);

    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
    lstrcpyn(lf.lfFaceName , m_strFont.c_str() , sizeof(lf.lfFaceName) / sizeof(TCHAR));
    //sort out?^^
    m_hFont = ::CreateFontIndirect(&lf);

    // create bold  font

    lf.lfWeight = FW_BOLD;
    m_hBoldFont = ::CreateFontIndirect(&lf);

    // get the font heights

    HGDIOBJ    hOldFont;
    hOldFont = SelectObject(hDC , m_hFont);
    GetTextMetrics(hDC , &tm);
    m_FontHeight = tm.tmHeight;

    SelectObject(hDC , m_hBoldFont);
    GetTextMetrics(hDC , &tm);
    m_BoldFontHeight = tm.tmHeight;
    SelectObject(hDC , hOldFont);

    // finished with the DC

    DeleteDC(hDC);


}



void CTaskbar::calc_barItemLists()
{
    int taskzone_width ,
        clock_width = 0;

    int trayWidth = 0;
    RECT mainbarRect;
    GetClientRect(m_hWnd , &mainbarRect);

    int xpos = m_Style.borderWidth;
    if (trayService.getTrayList() != NULL)
        trayWidth = defaultStyle::ICON_SIZE * trayService.getTrayList()->m_Items.size() + m_Style.borderWidth + defaultStyle::TASKBAR_GENERAL_SPACING;

    // --- 1st pass ----------------------------------
    for (int i = 0; i < m_barItemList.size(); i++) {
        barItem* bi = m_barItemList[i];

        switch (bi->buttonType) {
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
    int rest_width = max(0 , mainbarRect.right - xpos); // for the taskzone.

    int top = 0;
    int height = mainbarRect.bottom;

    // 2nd passsss

    taskzone_width = rest_width;

    xpos = 0;
    int ypos = 0;
    for (int i = 0; i < m_barItemList.size(); i++) {

        barItem* bi = m_barItemList[i];

        int width = 0;

        switch (bi->buttonType) {
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

        bi->calc_size(&xpos , ypos , width , mainbarRect.bottom , ypos - top);
    }
    return;
}

//RECT* CTaskbar::p_rcPaint;

bool CTaskbar::create(int x , int y , HWND hWnd , HINSTANCE hInstance , StyleStruct style , const UINT bitmap_id)
{
    // save variables
    m_hInstance = hInstance;
    m_BitmapID = bitmap_id;
    m_Style = style;


    // register the class
    if (!m_bClassRegistered) {
        if (!registerTaskbarClass())
            return false;

        m_bClassRegistered = true;
    }

    // create GDI objects
    createObjects();

    // get the screen dimensions and adjust x and y if necessary
    RECT rcScreen;
    SystemParametersInfo(SPI_GETWORKAREA , 0 , &rcScreen , 0);

    if (x + defaultStyle::TASKBAR_WIDTH > rcScreen.right)
        x = max(0 , rcScreen.right - defaultStyle::TASKBAR_WIDTH);

    // create the window
    m_hWnd = ::CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_TOPMOST ,         // prevent button appearing on taskbar
                               "taskbarClass" ,
                               "" ,
                               WS_POPUP | WS_VISIBLE ,
                               x ,
                               y ,
                               defaultStyle::TASKBAR_WIDTH ,
                               defaultStyle::TASKBAR_HEIGHT + m_Style.marginWidth + m_Style.borderWidth ,
                               NULL ,
                               NULL ,
                               m_hInstance ,
                               this);
    // check created OK

    if (m_hWnd == NULL)
        return false;

    ShowWindow(m_hWnd , SW_SHOW);
    // already in the , no need to wait for any buttons
    m_State = stateShow;

    m_hWndCommand = hWnd;

    return true;
}


/*****************************/
/* Register the window class */
/*****************************/

bool CTaskbar::registerTaskbarClass(void)
{
    WNDCLASS wc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL , IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"taskbarClass";

    return ::RegisterClass(&wc) != FALSE;
}


// WM_PAINT

void CTaskbar::OnPaint(HWND hWnd , HDC hDC)
{
    calc_barItemLists();

    // draw each item
    int pos = 0;
    for (int i = 0; i < m_barItemList.size(); i++) {
        barItem* it = m_barItemList[i];
        it->draw(hWnd , hDC);
    }
}


/*************************/
/* stub window procedure */
/*************************/

LRESULT CALLBACK CTaskbar::windowProc(HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam)
{

    // the usual window procedure...
    CTaskbar* pClass = ((CTaskbar*)GetWindowLongPtr(hWnd , GWLP_USERDATA));
    switch (uMsg) {
        case WM_CREATE:
            SetWindowLongPtr(hWnd , GWLP_USERDATA , (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetTimer(hWnd , 0 , 2000 , 0);
            break;
        case WM_ERASEBKGND:

            pClass->OnEraseBkgnd(hWnd , (HDC)wParam);
            return TRUE;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDBLCLK:
            pClass->OnMouseButton(hWnd , uMsg , wParam , LOWORD(lParam) , HIWORD(lParam));
            break;

        case WM_MOUSEMOVE:

            pClass->OnMouseMove(hWnd , LOWORD(lParam) , HIWORD(lParam));
            break;


        case WM_PAINT:
        {
            PAINTSTRUCT ps;

            BeginPaint(hWnd , &ps);

            pClass->OnPaint(hWnd , ps.hdc);
            //CTaskbar::p_rcPaint = &ps.rcPaint;

            EndPaint(hWnd , &ps);
        }
        break;

        case WM_TIMER:
        {
            if (wParam == 0) // check full screen app timer
            {
                bool hideWindow = ShellApi::isFullscreen(GetForegroundWindow());


                if (hideWindow == true) {
                    pClass->m_State = stateInactive;
                    ShowWindow(hWnd , SW_HIDE);
                }
                else if (pClass->m_State == stateInactive) {
                    pClass->m_State = stateShow;
                    ShowWindow(hWnd , SW_SHOW);

                }
            }
        }
        break;

        case WM_DESTROY:
            pClass->OnDestroy(hWnd);
            break;
    }

    return DefWindowProc(hWnd , uMsg , wParam , lParam);
}


/**************/
/* WM_DESTROY */
/**************/

void CTaskbar::OnDestroy(HWND hWnd)
{
    if (m_State == stateTrack)
        ReleaseCapture();

    m_State = stateInactive;
    m_hWnd = NULL;
}


/****************/
/* WM_MOUSEMOVE */
/****************/

void CTaskbar::OnMouseMove(HWND hWnd , short x , short y)
{
    RECT            rect;
    int              newitem;
    POINT           p;

    // get a DC and the window rectangle

    GetClientRect(hWnd , &rect);

    // shift tracking if we're over another 

    p.x = x;
    p.y = y;
    ClientToScreen(hWnd , &p);

    // get new selected item, if any
    newitem = getItem(x , y , rect);

    if (newitem != -1)
        m_barItemList[newitem]->mouse_event(x , y , WM_MOUSEMOVE , 0);

}


/************************/
/* Get item at position */
/************************/

int CTaskbar::getItem(short x , short y , const RECT& rect)
{
    int newitem;
    std::vector<baritemlist*>::size_type i;

    if (x <= 1 || y <= 1 || x > rect.right || y > rect.bottom)
        newitem = -1;
    else {
        for (i = 0; i < m_barItemList.size(); i++) {
            if (x > 1 && x <  m_barItemList[i]->itemRect.right)
                break;
        }

        newitem = i == m_barItemList.size() ? -1 : i;
    }

    return newitem;
}


/********************/
/* a button is down */
/********************/

void CTaskbar::ButtonDown(HWND hWnd , short x , short y , const bool bLeft)
{
    RECT rect;

    // check for click outside 
    GetClientRect(hWnd , &rect);
    if (checkOutsideWindow(rect , x , y))
        return;
}


/****************/
/* WM_LBUTTONUP */
/****************/

void CTaskbar::OnMouseButton(HWND hWnd , int message , WPARAM wparam , short x , short y)
{
    RECT rect;

    if (m_bWaitLeftButton) {
        m_bWaitLeftButton = false;
        return;
    }

    if (WM_RBUTTONUP && (wparam & MK_SHIFT)) {

        PopupMenu menu;
        // set some of the colors
        menu.SetColor(PopupMenu::colorBorder , m_Style.borderWidth == 0 ? m_Style.windowBackgroundColor : m_Style.borderColor);
        menu.SetColor(PopupMenu::colorText , m_Style.focusedTextColor);
        menu.SetColor(PopupMenu::colorBackground , m_Style.windowBackgroundColor);
        menu.SetColor(PopupMenu::colorHighlight , m_Style.taskFocusColor);
        menu.SetFontSize(m_Style.popupMenu_fontSize);

        // create it

        menu.Create(m_hInstance);

        // add some items

        menu.AppendItem(PopupMenu::itemBold | PopupMenu::itemNotSelectable , L"LiteBox" , 0);
        menu.AppendItem(PopupMenu::itemSeparator , NULL , 0);
        menu.AppendItem(0 , L"Exit LiteBox" , 1);
        menu.AppendItem(0 , L"Restart LiteBox" , 2);
        menu.AppendItem(0 , L"Open Style File" , 3);

        POINT pos;
        GetCursorPos(&pos);

        switch (menu.Track(pos.x , pos.y , NULL , true)) {
            case 1:
                SendMessage(m_hWndCommand , EXIT_TASKBAR , 0 , 0);
                break;

            case 2:
                SendMessage(m_hWndCommand , RESTART_TASKBAR , 0 , 0);
                //m_bCheck2 ^= true;
                break;
            case 3:
                SendMessage(m_hWndCommand , OPEN_STYLE_FILE , 0 , 0);
                //m_bCheck2 ^= true;
                break;
        }
        return;
    }

    GetClientRect(hWnd , &rect);
    if (checkOutsideWindow(rect , x , y))
        return;
    // shift tracking if we're over another  
    POINT           p;
    p.x = x;
    p.y = y;
    ClientToScreen(hWnd , &p);

    // get new selected item, if any

    int newitem = getItem(x , y , rect);

    if (newitem != -1)
        m_barItemList[newitem]->mouse_event(x , y , message , 0);
}

/*************************/
/* Check if outside window */
/*************************/
/// returns true if its outside 
bool CTaskbar::checkOutsideWindow(const RECT& rect , short x , short y)
{
    if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom) {
        // is out
        return true;
    }
    // is in 
    return false;
}


/*****************/
/* WM_ERASEBKGND */
/*****************/

void CTaskbar::OnEraseBkgnd(HWND hWnd , HDC hDC)
{
    RECT    rect;
    HGDIOBJ hOldBrush , hOldPen = 0;

    GetClientRect(hWnd , &rect);
    hOldBrush = SelectObject(hDC , m_hBackBrush);
    hOldPen = SelectObject(hDC , m_hBorderPen);

    // draws border and the back of it
    Rectangle(hDC , rect.left , rect.top , rect.right , rect.bottom);


    SelectObject(hDC , hOldBrush);
    SelectObject(hDC , hOldPen);


}

/****************/
/* Find an item */
/****************/

barItem* CTaskbar::getItem(const UINT itemid , const bool bByPosition)
{
    std::vector<baritemlist*>::iterator it;

    if (bByPosition) {
        if (itemid >= m_barItemList.size())
            return NULL;

        return m_barItemList[itemid];
    }
    else {
        for (int i = 0; i < m_barItemList.size(); i++) {
            barItem* it = m_barItemList[i];
            if (it->m_ItemID == itemid)
                return &(*it);
        }
        return NULL;
    }
}
