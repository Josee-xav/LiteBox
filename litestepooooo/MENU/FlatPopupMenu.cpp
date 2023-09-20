#include "FlatPopupMenu.h"

bool PopupMenu::m_bClassRegistered = false;

// stub window procedure

static LRESULT CALLBACK stubWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/***************/
/* Constructor */
/***************/

PopupMenu::PopupMenu()
{
    // default colors

    m_Colors[colorBorder] = GetSysColor(COLOR_MENUTEXT);
    m_Colors[colorBackground] = GetSysColor(COLOR_MENU);
    m_Colors[colorText] = GetSysColor(COLOR_MENUTEXT);
    m_Colors[colorGrayedText] = GetSysColor(COLOR_GRAYTEXT);
    m_Colors[colorHighlight] = GetSysColor(COLOR_HIGHLIGHT);
    m_Colors[colorHighlightText] = GetSysColor(COLOR_HIGHLIGHTTEXT);
    m_Colors[colorIconTransparent] = RGB(255, 0, 255);    // bright pink (mmm, nice...)
    m_Colors[colorLightShadow] = GetSysColor(COLOR_3DHILIGHT);
    m_Colors[colorDarkShadow] = GetSysColor(COLOR_3DSHADOW);

    // default font and size

    m_strFont = L"MS Sans Serif";
    m_FontSize = 8;

    // 400ms is the default submenu delay

    m_PopupDelay = 400;

    // GDI objects

    m_hFont = NULL;
    m_hBoldFont = NULL;
    m_hBorderPen = NULL;
    m_hBackBrush = NULL;
    m_hLightShadowPen = NULL;
    m_hDarkShadowPen = NULL;
    m_hBackPen = NULL;
    m_hSelectedBrush = NULL;
    m_hTextPen = NULL;
    m_hSelectedTextPen = NULL;
    m_hBitmap = NULL;

    // other internal variables

    m_bChild = false;
    m_pPrevious = NULL;
    m_State = stateInactive;
    m_hWnd = NULL;
}


/**************/
/* Destructor */
/**************/

PopupMenu::~PopupMenu()
{
    // if we're tracking, release it

    if (m_State == stateTrack)
        ReleaseCapture();

    // clean up the window

    if (m_hWnd)
        DestroyWindow(m_hWnd);

    // clean up the GDI objects

    Cleanup();
}


/***********************/
/* Cleanup GDI objects */
/***********************/

void PopupMenu::Cleanup(void)
{
    if (m_hFont) {
        DeleteObject(m_hFont);
        DeleteObject(m_hBoldFont);
        DeleteObject(m_hBorderPen);
        DeleteObject(m_hBackBrush);
        DeleteObject(m_hLightShadowPen);
        DeleteObject(m_hDarkShadowPen);
        DeleteObject(m_hBackPen);
        DeleteObject(m_hSelectedBrush);
        DeleteObject(m_hTextPen);
        DeleteObject(m_hSelectedTextPen);
        DeleteObject(m_hBitmap);

        m_hFont = NULL;
        m_hBoldFont = NULL;
        m_hBorderPen = NULL;
        m_hBackBrush = NULL;
        m_hLightShadowPen = NULL;
        m_hDarkShadowPen = NULL;
        m_hBackPen = NULL;
        m_hSelectedBrush = NULL;
        m_hTextPen = NULL;
        m_hSelectedTextPen = NULL;
        m_hBitmap = NULL;
    }
}


/**********************/
/* Create GDI objects */
/**********************/

void PopupMenu::CreateObjects(void)
{
    LOGFONT    lf;
    HDC        hDC;
    HGDIOBJ    hOldFont;
    TEXTMETRIC tm;

    // cleanup any previous

    Cleanup();

    // create normal menu font

    ZeroMemory(&lf, sizeof(lf));

    hDC = CreateICA("DISPLAY", NULL, NULL, NULL);
    lf.lfHeight = -MulDiv(m_FontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);

    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
    lstrcpyn(lf.lfFaceName, m_strFont.c_str(), sizeof(lf.lfFaceName) / sizeof(TCHAR));

    m_hFont = ::CreateFontIndirect(&lf);

    // create bold menu font

    lf.lfWeight = FW_BOLD;
    m_hBoldFont = ::CreateFontIndirect(&lf);

    // get the font heights

    hOldFont = SelectObject(hDC, m_hFont);
    GetTextMetrics(hDC, &tm);
    m_FontHeight = tm.tmHeight;

    SelectObject(hDC, m_hBoldFont);
    GetTextMetrics(hDC, &tm);
    m_BoldFontHeight = tm.tmHeight;
    SelectObject(hDC, hOldFont);

    // finished with the DC

    DeleteDC(hDC);
    DeleteObject(hOldFont);

    // create pens and brushes

    m_hBorderPen = CreatePen(PS_SOLID, 1, m_Colors[colorBorder]);
    m_hBackBrush = CreateSolidBrush(m_Colors[colorBackground]);
    m_hSelectedBrush = CreateSolidBrush(m_Colors[colorHighlight]);
    m_hLightShadowPen = CreatePen(PS_SOLID, 1, m_Colors[colorLightShadow]);
    m_hDarkShadowPen = CreatePen(PS_SOLID, 1, m_Colors[colorDarkShadow]);
    m_hBackPen = CreatePen(PS_SOLID, 1, m_Colors[colorBackground]);
    m_hTextPen = CreatePen(PS_SOLID, 1, m_Colors[colorText]);
    m_hSelectedTextPen = CreatePen(PS_SOLID, 1, m_Colors[colorHighlightText]);

    // create the bitmap

    if (m_BitmapID != (UINT)-1)
        m_hBitmap = LoadBitmap(m_hInstance, MAKEINTRESOURCE(m_BitmapID));

}


/*************/
/* Create it */
/*************/

bool PopupMenu::Create(HINSTANCE hInstance, const UINT bitmap_id)
{
    // save variables

    m_hInstance = hInstance;
    m_BitmapID = bitmap_id;

    // register the class

    if (!m_bClassRegistered) {
        if (!RegisterClass())
            return false;

        m_bClassRegistered = true;
    }

    // remove any previous items

    m_Items.clear();

    // create GDI objects

    CreateObjects();
    return true;
}


/*****************************/
/* Register the window class */
/*****************************/

bool PopupMenu::RegisterClass(void)
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = stubWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"fpmMenuClass";

    return ::RegisterClass(&wc) != FALSE;
}


/**********************/
/* Append a menu item */
/**********************/

bool PopupMenu::AppendItem(const DWORD dwFlags, LPCTSTR pszName, const UINT itemid, const int icon)
{
    CItem item;

    item.m_dwFlags = dwFlags;

    if (pszName)
        item.m_strName = pszName;

    item.m_ItemID = itemid;
    item.m_IconIndex = icon;
    item.m_pPopup = NULL;

    m_Items.push_back(item);
    return true;
}


/******************/
/* Append a popup */
/******************/

bool PopupMenu::AppendPopup(const DWORD dwFlags, LPCTSTR pszName, PopupMenu& popup, const int icon)
{
    CItem item;

    // the separator flag is not allowed

    if ((dwFlags & itemSeparator) != 0)
        return false;

    // add the item

    item.m_dwFlags = dwFlags;
    item.m_strName = pszName;
    item.m_ItemID = 0;
    item.m_IconIndex = icon;
    item.m_pPopup = &popup;

    popup.m_bChild = true;
    popup.m_pPrevious = this;

    m_Items.push_back(item);
    return true;
}


/******************/
/* Track the menu */
/******************/

UINT PopupMenu::Track(int x, int y, HWND hWnd, const bool bModal, const bool bPopup)
{
    HDC hDC;
    std::vector<CItem>::iterator it;
    SIZE size;
    int maxw, top;
    HGDIOBJ hOldFont;
    MSG msg;
    RECT rcScreen;

    // get an information context for the screen

    hDC = CreateICA("DISPLAY", NULL, NULL, NULL);

    // get the greatest item width

    maxw = 0;
    top = 3;

    for (it = m_Items.begin(); it != m_Items.end(); it++) {
        it->m_Top = top;

        if (!(it->m_dwFlags & itemSeparator)) {
            hOldFont = SelectObject(hDC, (it->m_dwFlags & itemBold) == 0 ? m_hFont : m_hBoldFont);
            GetTextExtentPoint32(hDC, it->m_strName.c_str(), it->m_strName.length(), &size);
            maxw = max(size.cx, maxw);
            SelectObject(hDC, hOldFont);

            top += 17;
            it->m_Height = 17;
        }
        else {
            top += 5;
            it->m_Height = 5;
        }


    }

    // finished with the DC

    DeleteDC(hDC);

    // get the screen dimensions and adjust x and y if necessary

    m_Width = 1 + 2 + 18 + 2 + maxw + 18 + 1;
    m_SelectedItem = -1;
    m_SelectedID = 0;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

    if (x + m_Width > rcScreen.right)
        x = max(0, rcScreen.right - m_Width);

    if (y + top + 2 > rcScreen.bottom)
        y = max(0, rcScreen.bottom - (top + 2));

    // create the window

    m_hWnd = ::CreateWindowExA(WS_EX_TOOLWINDOW,         // prevent button appearing on taskbar
        "fpmMenuClass",
        "",
        WS_POPUP | WS_VISIBLE,
        x,
        y,
        m_Width,
        top + 2,
        NULL,
        NULL,
        m_hInstance,
        this);

    // check created OK

    if (m_hWnd == NULL)
        return false;

    if (bPopup) {
        // already in the menu, no need to wait for any buttons
        m_bWaitLeftButton = false;
        m_bWaitRightButton = false;
        m_State = stateShow;
    }
    else {
        // enter tracking state

        SetCapture(m_hWnd);
        m_State = stateTrack;
        // we wait for any already-down buttons to come up

        m_bWaitLeftButton = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
        m_bWaitRightButton = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    }

    // remember modal setting and command window handle

    m_bModal = bModal;
    m_hWndCommand = hWnd;

    if (m_bModal) {
        // go into a modal loop

        while (GetMessage(&msg, NULL, 0, 0) == TRUE && IsWindow(m_hWnd)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return m_SelectedID;
    }

    return 0;
}

void PopupMenu::hidePopUp()
{
    if (m_hWnd)
        ShowWindow(m_hWnd, SW_HIDE);

}


/*************************/
/* stub window procedure */
/*************************/

static LRESULT CALLBACK stubWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    // the usual window procedure...
    PopupMenu* pClass = ((PopupMenu*)GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (uMsg) {
    case WM_CREATE:
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams); break;
        break;
    case WM_ERASEBKGND:

        pClass->OnEraseBkgnd(hWnd, (HDC)wParam);
        return TRUE;

    case WM_LBUTTONDOWN:

        pClass->OnLButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RBUTTONDOWN:

        pClass->OnLButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_LBUTTONUP:

        pClass->OnLButtonUp(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RBUTTONUP:

        pClass->OnRButtonUp(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MOUSEMOVE:

        pClass->OnMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_TIMER:

        pClass->OnTimer(hWnd, wParam);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint(hWnd, &ps);

        pClass->OnPaint(hWnd, ps.hdc);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        pClass->OnDestroy(hWnd);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



/************/
/* WM_TIMER */
/************/

void PopupMenu::OnTimer(HWND hWnd, unsigned short timerid)
{
    POINT p;

    // stop the timer

    KillTimer(hWnd, timerid);

    // show the popup menu

    if (m_SelectedItem != -1 && m_Items.at(m_SelectedItem).m_pPopup) {
        // get the display position

        p.x = m_Width - 10;
        p.y = m_Items.at(m_SelectedItem).m_Top - 3;
        ClientToScreen(hWnd, &p);

        // track it

        if (m_Items.at(m_SelectedItem).m_pPopup->Track(p.x, p.y, NULL, true, true))
            DestroyAll();
    }
}


/**********************/
/* Draw a single item */
/**********************/

void PopupMenu::DrawItem(HWND hWnd, HDC hDC, const int index, const CItem& item)
{
    bool    bSelected = index == m_SelectedItem;
    HGDIOBJ hOldPen, hOldBrush, hOldFont;
    POINT   p[3];
    int     i, x, y, h, nColor;
    RECT    rect;


    // test if a separator

    if ((item.m_dwFlags & itemSeparator) != 0) {
        DrawSeparator(hDC, item.m_Top);
        return;
    }

    // get the top left of the item area

    x = 3;
    y = item.m_Top;

    // is there an icon?

    if (item.m_IconIndex != -1) {
        // draw or erase the frame

        p[0].x = p[1].x = x;
        p[0].y = y + 16;
        p[1].y = p[2].y = item.m_Top;
        p[2].x = x + 17;
        hOldPen = SelectObject(hDC, bSelected ? m_hLightShadowPen : m_hBackPen);
        Polyline(hDC, p, 3);

        p[0].x = p[1].x = x + 17;
        p[0].y = item.m_Top;
        p[1].y = p[2].y = y + 16;
        p[2].x = x;
        SelectObject(hDC, bSelected ? m_hDarkShadowPen : m_hBackPen);
        Polyline(hDC, p, 3);
        SelectObject(hDC, hOldPen);

        // draw the bitmap

        DrawTransparentBitmap(hDC, item.m_IconIndex * 16, x + 1, item.m_Top + 1);

        // update co-ordinate

        x += 18;
    }

    // draw/clear the selection rectangle

    hOldBrush = SelectObject(hDC, bSelected ? m_hSelectedBrush : m_hBackBrush);
    hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));

    Rectangle(hDC, x, item.m_Top, m_Width - 2, item.m_Top + 18);

    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);

    // draw the item text

    if ((item.m_dwFlags & itemBold) != 0) {
        h = m_BoldFontHeight;
        hOldFont = SelectObject(hDC, m_hBoldFont);
    }
    else {
        h = m_FontHeight;
        hOldFont = SelectObject(hDC, m_hFont);
    }

    SetBkMode(hDC, TRANSPARENT);

    if (bSelected)
        nColor = colorHighlightText;
    else {
        if ((item.m_dwFlags & itemGrayed) != 0)
            nColor = colorGrayedText;
        else
            nColor = colorText;
    }

    rect.left = 1 + 2 + 18 + 2;
    rect.top = item.m_Top + ((17 - h) / 2);
    rect.bottom = rect.top + 1000;
    rect.right = rect.left + 1000;

    SetTextColor(hDC, m_Colors[nColor]);
    DrawText(hDC, item.m_strName.c_str(), item.m_strName.length(), &rect, DT_LEFT | DT_TOP | DT_NOCLIP);
    SelectObject(hDC, hOldFont);

    // draw a subitem arrow

    if (item.m_pPopup) {
        hOldPen = SelectObject(hDC, bSelected ? m_hSelectedTextPen : m_hTextPen);

        p[0].x = p[1].x = m_Width - 7;
        p[0].y = item.m_Top + 8;
        p[1].y = item.m_Top + 9;

        for (i = 0; i < 4; i++) {
            Polyline(hDC, p, 2);
            p[0].x--;
            p[1].x--;
            p[0].y--;
            p[1].y++;
        }

        SelectObject(hDC, hOldPen);
    }


    DeleteObject(hOldPen);
    DeleteObject(hOldBrush);
    DeleteObject(hOldFont);
}


/********************/
/* Draw a separator */
/********************/

void PopupMenu::DrawSeparator(HDC hDC, int top)
{
    POINT   p[2];
    HGDIOBJ hOldPen;

    p[0].x = 0;
    p[1].x = m_Width;
    p[0].y = p[1].y = top + 2;
    hOldPen = SelectObject(hDC, m_hBorderPen);
    Polyline(hDC, p, 2);
    SelectObject(hDC, hOldPen);

    DeleteObject(hOldPen);
}


/**************/
/* WM_DESTROY */
/**************/

void PopupMenu::OnDestroy(HWND hWnd)
{
    std::vector<CItem>::const_iterator it;
    // any active popups must also be destroyed

    for (it = m_Items.begin(); it != m_Items.end(); it++)
        if (it->m_pPopup && it->m_pPopup->m_State != stateInactive)
            DestroyWindow(it->m_pPopup->m_hWnd);

    if (m_State == stateTrack)
        ReleaseCapture();

    m_State = stateInactive;
    m_hWnd = NULL;


    if (!m_bModal && m_SelectedID && !m_bChild)
        PostMessage(m_hWndCommand, WM_COMMAND, m_SelectedID, 0);
}


/****************/
/* WM_MOUSEMOVE */
/****************/

void PopupMenu::OnMouseMove(HWND hWnd, short x, short y)
{
    RECT            rect;
    HDC             hDC;
    HWND            hPointWnd;
    int             item, newitem;
    POINT           p;
    TCHAR           szClass[100];

    // get a DC and the window rectangle

    GetClientRect(hWnd, &rect);
    hDC = GetDC(hWnd);

    // shift tracking if we're over another menu

    p.x = x;
    p.y = y;
    ClientToScreen(hWnd, &p);

    //!= NULL && GetClassName(hPointWnd, szClass, sizeof(szClass) / sizeof(TCHAR)) && !lstrcmp(szClass, "fpmMenuClass")

    if ((hPointWnd = WindowFromPoint(p)) != NULL && GetClassName(hPointWnd, szClass, sizeof(szClass) / sizeof(TCHAR)) && !lstrcmp(szClass, L"fpmMenuClass")) {
        PopupMenu* pClass = ((PopupMenu*)GetWindowLongPtr(hPointWnd, GWLP_USERDATA));
        if (pClass != this) {
            ReleaseCapture();
            m_State = stateShow;

            SetCapture(pClass->m_hWnd);
            pClass->m_State = stateTrack;

            return;
        }
    }

    // get new selected item, if any

    newitem = GetItem(x, y, rect);

    // remove previous selection?

    if (m_SelectedItem != -1 && newitem != m_SelectedItem) {
        // kill any ongoing timer
        KillTimer(m_hWnd, 1);

        //	// remove any popups associated with the last active item
        if (m_Items.at(m_SelectedItem).m_pPopup && m_Items.at(m_SelectedItem).m_pPopup->m_State != stateInactive)
            DestroyWindow(m_Items.at(m_SelectedItem).m_pPopup->m_hWnd);

        //	// deselect the old item
        item = m_SelectedItem;
        m_SelectedItem = -1;

        DrawItem(hWnd, hDC, item, m_Items.at(item));
    }

    // new selected item?
    if (newitem != -1 && m_SelectedItem != newitem) {

        // draw the new item
        m_SelectedItem = newitem;
        DrawItem(hWnd, hDC, m_SelectedItem, m_Items.at(m_SelectedItem));

        // start a timer if this is a popup
        if (m_Items.at(m_SelectedItem).m_pPopup)
            SetTimer(m_hWnd, 1, m_PopupDelay, NULL);
    }

    ReleaseDC(hWnd, hDC);
}


/************************/
/* Get item at position */
/************************/

int PopupMenu::GetItem(short x, short y, const RECT& rect)
{
    int newitem;
    std::vector<CItem>::size_type i;

    if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom)
        newitem = -1;
    else {
        for (i = 0; i < m_Items.size(); i++)
            if (y > m_Items.at(i).m_Top && y < m_Items.at(i).m_Top + m_Items.at(i).m_Height)
                break;

        newitem = i == m_Items.size() ? -1 : i;
        if (newitem != -1 && (m_Items.at(newitem).m_dwFlags & itemNotSelectable) != 0)
            newitem = -1;
    }

    return newitem;
}


/********************/
/* a button is down */
/********************/

void PopupMenu::ButtonDown(HWND hWnd, short x, short y, const bool bLeft)
{
    RECT rect;

    // check for click outside menu


    GetClientRect(hWnd, &rect);
    if (CheckOutsideMenu(hWnd, rect, x, y))
        return;
    // check for click on popup

    if (m_SelectedItem != -1 && m_Items.at(m_SelectedItem).m_pPopup && m_Items.at(m_SelectedItem).m_pPopup->m_State == stateInactive)
        OnTimer(hWnd, 1);
}


/****************/
/* WM_LBUTTONUP */
/****************/

void PopupMenu::OnLButtonUp(HWND hWnd, short x, short y)
{
    RECT rect;

    if (m_bWaitLeftButton) {
        m_bWaitLeftButton = false;
        return;
    }

    GetClientRect(hWnd, &rect);
    if (CheckOutsideMenu(hWnd, rect, x, y))
        return;

    if (m_SelectedItem != -1 && m_Items.at(m_SelectedItem).m_ItemID)
        SetReturn(m_Items.at(m_SelectedItem).m_ItemID);
}


/****************/
/* WM_RBUTTONUP */
/****************/

void PopupMenu::OnRButtonUp(HWND hWnd, short x, short y)
{
    RECT rect;

    if (m_bWaitRightButton) {
        m_bWaitRightButton = false;
        return;
    }

    GetClientRect(hWnd, &rect);
    if (CheckOutsideMenu(hWnd, rect, x, y))
        return;

    if (m_SelectedItem != -1 && m_Items.at(m_SelectedItem).m_ItemID)
        SetReturn(m_Items.at(m_SelectedItem).m_ItemID);
}


/************************/
/* Set the return value */
/************************/

void PopupMenu::SetReturn(const UINT id)
{
    PopupMenu* pMenu;

    // find the root

    for (pMenu = this; pMenu->m_pPrevious; pMenu = pMenu->m_pPrevious);
    pMenu->m_SelectedID = id;
    DestroyAll();
}


/*************************/
/* Check if outside menu */
/*************************/

bool PopupMenu::CheckOutsideMenu(HWND hWnd, const RECT& rect, short x, short y)
{
    if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom) {
        if (m_SelectedItem != -1)
            if (m_Items.at(m_SelectedItem).m_pPopup && m_Items.at(m_SelectedItem).m_pPopup->m_State != stateInactive) {
                return false;
            }
        // clicked outside the window

        m_SelectedItem = -1;
        m_SelectedID = 0;
        DestroyAll();

        return true;
    }

    return false;
}


/*****************/
/* WM_ERASEBKGND */
/*****************/

void PopupMenu::OnEraseBkgnd(HWND hWnd, HDC hDC)
{
    RECT    rect;
    HGDIOBJ hOldBrush, hOldPen;

    GetClientRect(hWnd, &rect);
    hOldBrush = SelectObject(hDC, m_hBackBrush);
    hOldPen = SelectObject(hDC, m_hBorderPen);

    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hOldPen);

    DeleteObject(hOldBrush);
    DeleteObject(hOldPen);

}


/***************************/
/* Assign item to src item */
/***************************/

void PopupMenu::CItem::Assign(const CItem& src)
{
    m_dwFlags = src.m_dwFlags;
    m_strName = src.m_strName;
    m_ItemID = src.m_ItemID;
    m_IconIndex = src.m_IconIndex;
    m_Top = src.m_Top;
    m_Height = src.m_Height;
    m_pPopup = src.m_pPopup;
}


/*****************************/
/* Draw a transparent bitmap */
/*****************************

This function is borrowed and slightly modified from an MSDN sample */

void PopupMenu::DrawTransparentBitmap(HDC hdc, const int xSrcOffset, const int xStart, const int yStart)
{
    BITMAP    bm;
    COLORREF  cColor;
    HBITMAP   bmAndBack, bmAndObject, bmAndMem, bmSave;
    HBITMAP   bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
    HDC       hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT     ptSize;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, m_hBitmap);      // Select the bitmap

    GetObject(m_hBitmap, sizeof(BITMAP), (LPSTR)&bm);
    ptSize.x = bm.bmWidth;                // Get width of bitmap
    ptSize.y = bm.bmHeight;               // Get height of bitmap
    DPtoLP(hdcTemp, &ptSize, 1);          // Convert from device
    // to logical points
// Create some DCs to hold temporary data

    hdcBack = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem = CreateCompatibleDC(hdc);
    hdcSave = CreateCompatibleDC(hdc);

    // Create a bitmap for each DC. DCs are required for a number of GDI functions

    // Monochrome DC

    bmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    // Monochrome DC

    bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    bmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    bmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    // Each DC must select a bitmap object to store pixel data

    bmBackOld = (HBITMAP)SelectObject(hdcBack, bmAndBack);
    bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
    bmMemOld = (HBITMAP)SelectObject(hdcMem, bmAndMem);
    bmSaveOld = (HBITMAP)SelectObject(hdcSave, bmSave);

    // Set proper mapping mode

    SetMapMode(hdcTemp, GetMapMode(hdc));

    // Save the bitmap sent here, because it will be overwritten.

    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC to the color.
    // contained in the parts of the bitmap that should be transparent

    cColor = SetBkColor(hdcTemp, m_Colors[colorIconTransparent]);

    // Create the object mask for the bitmap by performing a BitBlt
    // from the source bitmap to a monochrome bitmap.

    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC back to the original color

    SetBkColor(hdcTemp, cColor);

    // Create the inverse of the object mask

    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

    // Copy the background of the main DC to the destination

    BitBlt(hdcMem, xSrcOffset, 0, 16, 15, hdc, xStart, yStart, SRCCOPY);

    // Mask out the places where the bitmap will be placed

    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

    // Mask out the transparent colored pixels on the bitmap

    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

    // XOR the bitmap with the background on the destination DC

    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

    // Copy the destination to the screen

    BitBlt(hdc, xStart, yStart, 16, 15, hdcMem, xSrcOffset, 0, SRCCOPY);

    // Place the original bitmap back into the bitmap sent here

    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    // Delete the memory bitmaps

    DeleteObject(SelectObject(hdcBack, bmBackOld));
    DeleteObject(SelectObject(hdcObject, bmObjectOld));
    DeleteObject(SelectObject(hdcMem, bmMemOld));
    DeleteObject(SelectObject(hdcSave, bmSaveOld));
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    // Delete the memory DCs

    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}


/************************/
/* Change the menu text */
/************************/

bool PopupMenu::SetMenuItemText(const UINT itemid, LPCTSTR pszName, const bool bByPosition)
{
    CItem* pItem;

    // get the item

    if ((pItem = GetItem(itemid, bByPosition)) == NULL)
        return false;

    // adjust the name

    pItem->m_strName = pszName;
    return true;
}


/*************************/
/* Change the menu flags */
/*************************/

bool PopupMenu::SetMenuItemFlags(const UINT itemid, const DWORD dwFlags, const bool bByPosition)
{
    CItem* pItem;

    // get the item

    if ((pItem = GetItem(itemid, bByPosition)) == NULL)
        return false;

    // adjust the flags

    pItem->m_dwFlags = dwFlags;
    return true;
}


/**************************/
/* Get a menu item's text */
/**************************/

bool PopupMenu::GetString(const UINT itemid, LPTSTR pszText, const UINT cchText, const bool bByPosition)
{
    CItem* pItem;

    // get the item

    if ((pItem = GetItem(itemid, bByPosition)) == NULL)
        return false;

    if (pItem->m_strName.length() + 1 > cchText)
        return false;

    pItem->m_strName = pszText;
    return true;
}


/*************************/
/* Change the menu flags */
/*************************/

bool PopupMenu::SetMenuItemIcon(const UINT itemid, const int icon, const bool bByPosition)
{
    CItem* pItem;

    // get the item

    if ((pItem = GetItem(itemid, bByPosition)) == NULL)
        return false;

    // adjust the icon

    pItem->m_IconIndex = icon;
    return true;
}


/****************/
/* Find an item */
/****************/

PopupMenu::CItem* PopupMenu::GetItem(const UINT itemid, const bool bByPosition)
{
    std::vector<CItem>::iterator it;

    if (bByPosition) {
        if (itemid >= m_Items.size())
            return NULL;

        return &m_Items.at(itemid);
    }
    else {
        for (it = m_Items.begin(); it != m_Items.end(); it++)
            if (it->m_ItemID == itemid)
                return &(*it);

        return NULL;
    }
}
