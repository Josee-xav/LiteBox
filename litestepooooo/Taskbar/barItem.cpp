#include "barItem.h"
#include "CTaskbar.h"

int barItem::s_id = 6696;
CTaskbar* barItem::mainbar = NULL;

/***************************/
/* Assign item to src item */
/***************************/
void barItem::Assign(const barItem& src)
{
    m_dwFlags = src.m_dwFlags;
    m_strName = src.m_strName;
    m_ItemID = src.m_ItemID;
    m_icon = src.m_icon;
    m_data = src.m_data;
    buttonType = src.buttonType;
}

// CItem constructor

barItem::barItem(int type) : itemRect{ 0,0,0,0 }, m_strName(L""), m_icon(NULL), m_ItemID(0), m_data(NULL), buttonType(type), m_dwFlags(NULL)
{
    m_ItemID = ++s_id;

}


// CItem copy constructor
barItem::barItem(const barItem& src)
{
    Assign(src);
}

barItem::~barItem()
{
}


// CItem = operator
barItem& barItem::operator=(const barItem& src)
{
    Assign(src);
    return *this;
}

int barItem::mouse_over(int mx, int my)
{
    return 0;
}

void barItem::mouse_event(int mx, int my, int message, unsigned flags)
{
}

void barItem::invalidate(int flag)
{
    /*RECT iitemRect;
    GetClientRect(mainbar->m_hWnd, &iitemRect);*/
    InvalidateRect(mainbar->m_hWnd, &itemRect, flag);

}

bool barItem::calc_size(int* px, int y, int w, int h, int m)
{
    int x = *px;
    bool f = false;

    if (itemRect.left != x)
        itemRect.left = x, f = true;
    x += w;
    if (itemRect.right != x)
        itemRect.right = x, f = true;
    *px = x;

    itemRect.top = y;
    itemRect.bottom = h;
    return true;
}

void barItem::draw(HWND hWnd, HDC hDC)
{
}
