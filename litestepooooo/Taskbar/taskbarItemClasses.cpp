#include "taskbarItemClasses.h"
#include "CTaskbar.h"

#include "../Utils.h"
#include "drawingApi.h"
#include "../Services/TrayService.h"
#pragma warning(disable : 4996)

#include <algorithm>
#include "../Logging/YLogger.h"

//void drawwww(barItem* r, HDC hDC, COLORREF color)
//{
//	HGDIOBJ hOldBrush, hOldPen = 0;
//
//	HPEN m_hBorderPen = CreatePen(PS_SOLID, 1, color);
//	hOldBrush = SelectObject(hDC, r->mainbar->m_hBackBrush);
//	hOldPen = SelectObject(hDC, m_hBorderPen);
//
//	// draws border and the back of it
//	Rectangle(hDC, r->itemRect.left, r->itemRect.top, r->itemRect.right, r->itemRect.bottom);
//
//	SelectObject(hDC, hOldBrush);
//	SelectObject(hDC, hOldPen);
//
//
//	DeleteObject(m_hBorderPen);
//}

taskEntryBtn::taskEntryBtn() : barItem(M_TASK)
{
}

taskEntryBtn::~taskEntryBtn()
{
}

void taskEntryBtn::draw(HWND hWnd, HDC hDC)
{
	bool    bSelected = (m_dwFlags == taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG);
	HGDIOBJ hOldPen, hOldBrush, hOldFont;
	int     h{ 0 };
	COLORREF color;

	RECT drawingRect;

	drawingRect.left = itemRect.left;
	drawingRect.top = itemRect.top + m_Style.task_buttonTopSpacing;
	drawingRect.right = itemRect.right;
	drawingRect.bottom = itemRect.bottom - (itemRect.top + m_Style.task_buttonTopSpacing);


	HBRUSH taskBrush = CreateSolidBrush(bSelected ? m_Style.task_FocusColor : m_Style.windowBackgroundColor);
	HPEN taskPen = CreatePen(PS_SOLID, 1, m_Style.borderColor);

	// draw/clear the selection rectangle
	hOldBrush = SelectObject(hDC, taskBrush);
	hOldPen = SelectObject(hDC, m_Style.task_BevelStyle == 1 ? taskPen : GetStockObject(NULL_PEN)); // allows the pen to make a bevel for us so its easier when its rounded for example.

	if (m_Style.rectRoundedEdge_TaskButtons != true)
		Rectangle(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom);
	else
		RoundRect(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom, 5, 5);

	//m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
	if (m_Style.task_BevelStyle != 0 && m_Style.task_BevelStyle != 1)
		DrawingApi::drawBevel(hDC, m_Style.borderColor, &drawingRect, m_Style.task_BevelStyle, BF_RECT);

	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);

	// is there an icon?
	if (m_icon != NULL) {

		int iconSize = ((drawingRect.bottom - drawingRect.top) - 3);

		if (iconSize > (drawingRect.right - drawingRect.left)) { // make sure the icon is not bigger than the box.
			iconSize = (drawingRect.right - drawingRect.left) - 1;
		}

		DrawIconEx(
			hDC,
			drawingRect.left,
			((drawingRect.bottom + drawingRect.top) / 2) - (iconSize / 2),
			m_icon,
			iconSize,
			iconSize,
			0, NULL, DI_NORMAL);

	}



	// add spacing for text beside the icon
	if (m_icon != NULL)
		drawingRect.left += (drawingRect.bottom - drawingRect.top); // add the width of the icon for the basically the x cord for the text.

	hOldFont = SelectObject(hDC, mainbar->m_hFont);

	SetBkMode(hDC, TRANSPARENT);

	if (bSelected)
		color = m_Style.focusedTextColor;
	else
		color = m_Style.unfocusedTextColor;

	SetTextColor(hDC, color);
	DrawText(hDC, m_strName.c_str(), -1, &drawingRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX);
	SelectObject(hDC, hOldFont);

	DeleteObject(taskPen);
	DeleteObject(taskBrush);
}


void taskEntryBtn::mouse_event(int mx, int my, int message, unsigned flags)
{
	HWND taskAppHwnd = (HWND)m_data;
	DWORD_PTR dwResult = 0;

	if (message == WM_LBUTTONDOWN) {
		SetForegroundWindow(taskAppHwnd);
		// sends WM_SYSCOMMAND the parm SC_RESTORE which is kinda like right clicking the taskbutton on the normal windows shell and clicking open or something
		SendMessageTimeout(taskAppHwnd, WM_SYSCOMMAND, SC_RESTORE, 0, 2, 500, &dwResult);
		mainbar->taskService.updateActiveTask(taskAppHwnd);
	}
	else if (message == WM_RBUTTONDOWN) {
		SendMessageTimeout(taskAppHwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0, 2, 500, &dwResult);
		mainbar->taskService.updateActiveTask(NULL, true);
	}
}

/// <summary>
/// TRAY ENTRY BTN
/// </summary>
/// <param name="bar"></param>

void TrayEntryBtn::trayMouseDown(int message)
{
	TrayItem* ni = (_TrayItem*)m_data;
	DWORD procId = 0;
	GetWindowThreadProcessId(ni->hWnd, &procId);
	AllowSetForegroundWindow(procId);
	if (message == WM_RBUTTONDOWN)
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONDOWN | (ni->uID << 16));
	else if (message == WM_LBUTTONDOWN)
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_LBUTTONDOWN | (ni->uID << 16));
}

// only when the mouse is up is when the menu is actually opened. 
void TrayEntryBtn::trayMouseUp(int message)
{
	TrayItem* ni = (_TrayItem*)m_data;

	POINT ps;
	GetCursorPos(&ps);
	LPARAM lparam = MAKELPARAM(ps.x, ps.y);

	DWORD procId = 0;
	GetWindowThreadProcessId(ni->hWnd, &procId);
	AllowSetForegroundWindow(procId);
	if (message == WM_LBUTTONUP)
		//SendNotifyMessage(ni->hWnd , ni->uCallbackMessage , 0 , NIN_SELECT | (ni->uID << 16));
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, NIN_SELECT | (ni->uID << 16));
	else if (message == WM_RBUTTONUP) {
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONUP);
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, lparam, WM_CONTEXTMENU | (ni->uID << 16));
	}
}

void TrayEntryBtn::trayMouseDOUBLECLICKDown(int message)
{
	_TrayItem* ni = (_TrayItem*)m_data;

	DWORD procId = 0;
	GetWindowThreadProcessId(ni->hWnd, &procId);
	if (message == WM_LBUTTONDBLCLK)
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_LBUTTONDBLCLK | (ni->uID << 16));
	else if (message == WM_RBUTTONDBLCLK)
		SendNotifyMessage(ni->hWnd, ni->uCallbackMessage, 0, WM_RBUTTONDBLCLK | (ni->uID << 16));
}

TrayEntryBtn::TrayEntryBtn() : barItem(M_TRAY)
{
}

TrayEntryBtn::~TrayEntryBtn()
{
}

void TrayEntryBtn::draw(HWND hWnd, HDC hDC)
{
	const int trayIconSize = 16;//TODO

	HGDIOBJ hOldPen, hOldBrush;
	int      h{ 0 };
	RECT drawingRect;
	int y = m_Style.border_Width;
	drawingRect.left = itemRect.left;
	drawingRect.top = y;
	drawingRect.right = itemRect.right;
	drawingRect.bottom = itemRect.bottom - (y);

	// draw/clear the selection rectangle
	HBRUSH brush = CreateSolidBrush(m_Style.windowBackgroundColor);
	hOldBrush = SelectObject(hDC, brush);
	hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));

	Rectangle(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom);
	// is there an icon?
	int ypos = (itemRect.bottom / 2) - (trayIconSize / 2);

	if (LB_Api::isIconValid(m_icon) == false)
		YLogger::Error("ICON NOT VALID? TRAY STR : %s", m_strName);

	DrawIconEx(
		hDC,
		drawingRect.left,
		ypos, // centers the icon
		m_icon,
		trayIconSize,
		trayIconSize,
		0, NULL, DI_NORMAL);



	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);
	DeleteObject(brush);
}

void TrayEntryBtn::mouse_event(int mx, int my, int message, unsigned flags)
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
int baritemlist::getItem(short x, short y, const RECT& rect)
{

	int newitem;
	std::vector<barItem*>::size_type i;

	if (x <= 2 || y <= 2 || x > rect.right || y > rect.bottom)
		newitem = -1;
	else {
		for (i = 0; i < m_Items.size(); i++) {
			if (x > 2 && x < m_Items.at(i)->itemRect.right)
				break;
		}

		newitem = i == m_Items.size() ? -1 : i;
		if (newitem != -1 && (m_Items.at(newitem)->m_dwFlags & taskbarItemFlags::itemNotSelectable) != 0)
			newitem = -1;
	}

	return newitem;

}
bool baritemlist::calc_itemsSizes()
{
	return FALSE;
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


void baritemlist::draw(HWND hWnd, HDC hDC)
{
	for (int i = 0; i < m_Items.size(); i++) {
		m_Items.at(i)->draw(hWnd, hDC);
	}
}

void baritemlist::mouse_event(int mx, int my, int message, unsigned flags)
{
	RECT            rect;
	int              newitem;
	POINT           p;

	// get a DC and the window rectangle
	HWND mainbarHwnd = mainbar->m_hWnd;
	GetClientRect(mainbarHwnd, &rect);

	// shift tracking if we're over another menu

	p.x = mx;
	p.y = my;
	ClientToScreen(mainbarHwnd, &p);

	switch (message) {
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		// get new selected item, if any
		newitem = getItem(mx, my, rect);

		if (newitem != -1)
			m_Items.at(newitem)->mouse_event(mx, my, message, 0);
	}
	break;

	}
}

bool baritemlist::calc_size(int* px, int y, int w, int h, int m)
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

	calc_itemsSizes();

	return true;
}

// this function gives me a headache.
bool taskItemList::calc_itemsSizes()
{
	int amountOfTasks = m_Items.size();

	if (0 == amountOfTasks)
		return false;

	int b = 1 + m_Style.task_buttonSpacing;
	int max_width = itemRect.right - itemRect.left; // the max width of the task item list.
	int h = itemRect.bottom - itemRect.top;
	int xpos = itemRect.left + b;
	int y = itemRect.top + m_Style.border_Width;

	int is = h + b;
	int min_width = is / 2;

	int r = max(max_width / 100, is);
	if (r / amountOfTasks >= max_width || m_Style.tasks_IconOnly)
		max_width = amountOfTasks * r;


	int n = 0;
	for (int i = 0; i < m_Items.size(); i++) {
		int left, right;
		{
			left = xpos + max_width * n / amountOfTasks;
			right = xpos + max_width * (n + 1) / amountOfTasks - b;
			if (right - left < min_width)
				right = left + min_width;
		}

		if (right > itemRect.right - b)
			right -= b;


		m_Items.at(i)->calc_size(&left, y, right - left, h, 0);
		++n;
	}
	return TRUE;
}

taskItemList::taskItemList() : baritemlist(M_TASKLIST)
{
}

bool compareString(barItem* a, barItem* b)
{
	TCHAR           abuf[100];
	GetClassName((HWND)a->m_data, abuf, 100);
	TCHAR           bbuf[100];
	GetClassName((HWND)b->m_data, bbuf, 100);

	return std::wcscmp(abuf, bbuf) < 0;
}

void taskItemList::add(barItem* entry)
{
	m_Items.push_back(entry);
	std::sort(std::begin(m_Items), std::end(m_Items), compareString); // quick sort, O(N LOG N)
}

void taskItemList::remove(barItem* entry)
{

	for (int i = 0; i < m_Items.size(); i++) {
		barItem* item = m_Items.at(i);

		if ((HWND)item->m_data == (HWND)entry->m_data) {

			m_Items.erase(m_Items.begin() + i); // o(n)
			invalidate(true);

		}
	}
}


bool trayItemList::calc_itemsSizes()
{
	int amountOfTasks = m_Items.size();

	if (0 == amountOfTasks)
		return false;

	int max_width = itemRect.right - itemRect.left; // the max width of the task item list.
	int h = itemRect.bottom - itemRect.top;
	int xpos = itemRect.left;
	int y = itemRect.top;

	int is = h;

	max_width = amountOfTasks * 20;


	int n = 0;
	for (int i = 0; i < m_Items.size(); i++) {
		int left, right;
		{
			left = xpos + max_width * n / amountOfTasks;
			right = xpos + max_width * (n + 1) / amountOfTasks - 5;

		}




		m_Items.at(i)->calc_size(&left, y, right - left, h, 0);
		++n;
	}
	return TRUE;
}

void trayItemList::invalidate(int flag)
{
	InvalidateRect(mainbar->m_hWnd, NULL, flag); // tray item list may need to readjust the tasklist and the traylist. so might as welll refresh the whole bar.
}

trayItemList::trayItemList() : baritemlist(M_TRAYLIST)
{
}

std::wstring clockBtn::convert24hourTo12HourTime(int time24Hour, int time24minute)
{
	wchar_t  buff[20];
	switch (time24Hour) {
	case 12:
		swprintf(buff, 20, L"%02d:%02d PM", time24Hour, time24minute);
		break;
	case 00:
		swprintf(buff, 20, L"%02d:%02d AM", time24Hour + 12, time24minute);
		break;
	default:
	{
		if (time24Hour > 12) {
			swprintf(buff, 20, L"%02d:%02d PM", time24Hour - 12, time24minute);

		}
		else {
			swprintf(buff, 20, L"%02d:%02d AM", time24Hour, time24minute);

		}
		break;
	}
	break;
	}
	std::wstring ret = buff;
	return ret;
}

void clockBtn::updateClock(clockFormat format)
{
	std::wstring formatTime;

	// gets the date and time 
	SYSTEMTIME dateAndTime;
	GetLocalTime(&dateAndTime);

	if (format == clockFormat::format_12hour) {
		formatTime = convert24hourTo12HourTime(dateAndTime.wHour, dateAndTime.wMinute);
		if (formatTime != m_strName) {// so it doesnt set time when its not necessary
			m_strName = formatTime;
			invalidate(1);
		}
	}
	else {
		wchar_t buff[20];
		swprintf(buff, 20, L"%02d:%02d", dateAndTime.wHour, dateAndTime.wMinute);

		formatTime = buff;
		if (formatTime != m_strName) {// so it doesnt set time when its not necessary
			m_strName = formatTime;
			invalidate(1);
		}
	}
}

LRESULT clockBtn::timerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_TIMER:
	{
		((clockBtn*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->updateClock(static_cast<clockFormat>(m_Style.clock_TimeFormat));

		SYSTEMTIME time;
		GetLocalTime(&time);
		SetTimer(hwnd, 333, 1100 - time.wMilliseconds, 0);
	}
	break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool clockBtn::classRegistered = false;

clockBtn::clockBtn() : barItem(M_CLOCK)
{
	createClockTimer();
}

clockBtn::~clockBtn()
{
	DestroyWindow(messageWindow);
}

void clockBtn::createClockTimer()
{
	if (!classRegistered) {
		WNDCLASS wc = { };

		wc.lpfnWndProc = timerProc;
		wc.hInstance = LB_Api::main_hinstance;
		wc.lpszClassName = L"clockproo";

		if (!RegisterClass(&wc)) {
			return;
		}
		classRegistered = true;
	}

	// Create the window.

	messageWindow = CreateWindow(L"clockproo", 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
	SetWindowLongPtr(messageWindow, GWLP_USERDATA, (LONG_PTR)this); // sets the GWLP_USERDATA to the object so we can get it easily via the HWND 

	SYSTEMTIME lt;
	GetLocalTime(&lt);
	SetTimer(messageWindow, CLOCK_TIMER, 0, 0);
}

void clockBtn::draw(HWND hWnd, HDC hDC)
{
	HGDIOBJ hOldPen, hOldBrush, hOldFont;
	int      h{ 0 };
	COLORREF color;

	RECT drawingRect;
	int y = m_Style.border_Width;
	drawingRect.left = itemRect.left;
	drawingRect.top = y;
	drawingRect.right = itemRect.right;
	drawingRect.bottom = itemRect.bottom - (y);

	// draw/clear the selection rectangle
	HBRUSH background = CreateSolidBrush(m_Style.clock_Color != CLR_INVALID ? m_Style.clock_Color : m_Style.windowBackgroundColor);
	HPEN borderPen = CreatePen(PS_SOLID, 1, m_Style.borderColor);

	hOldBrush = SelectObject(hDC, background);
	hOldPen = SelectObject(hDC, m_Style.clock_BevelStyle == 1 ? borderPen : GetStockObject(NULL_PEN)); // allows the pen to make a bevel for us so its easier when its rounded for example.

	if (m_Style.rectRoundedEdge_Clock != true)
		Rectangle(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom);
	else
		RoundRect(hDC, drawingRect.left, drawingRect.top, drawingRect.right, drawingRect.bottom, 4, 4); // TODO

	//.m_Style.taskBevelStyle != 1 CUS it creates a flat looking bevel using the pen.
	if (m_Style.clock_BevelStyle != 0 && m_Style.clock_BevelStyle != 1)
		DrawingApi::drawBevel(hDC, m_Style.borderColor, &drawingRect, m_Style.clock_BevelStyle, BF_RECT);

	SelectObject(hDC, hOldPen);
	SelectObject(hDC, hOldBrush);


	hOldFont = SelectObject(hDC, mainbar->m_hSmallerFont);

	SetBkMode(hDC, TRANSPARENT);

	color = m_Style.unfocusedTextColor;



	SetTextColor(hDC, color);
	DrawText(hDC, m_strName.c_str(), -1, &drawingRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SelectObject(hDC, hOldFont);

	DeleteObject(background);
	DeleteObject(borderPen);
}

void clockBtn::mouse_event(int mx, int my, int message, unsigned flags)
{
}
