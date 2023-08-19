#include "WindowQueryHelpers.h"



// Tries to find the CoreWindow class in a UWP application as thats the main class thats shown when not cloaked to my understanding..
BOOL CALLBACK ProcessWindow(HWND hwnd, LPARAM lparam) {
	bool* foundTHEchild = reinterpret_cast<bool*>(lparam);
	char windowClass[256] = {};
	GetClassNameA(hwnd, windowClass, sizeof(windowClass));

	if (strcmp(windowClass, "Windows.UI.Core.CoreWindow") == 0) {
		*foundTHEchild = true;
	}

	return true;
}

// https://stackoverflow.com/questions/32149880/how-to-identify-windows-10-background-store-processes-that-have-non-displayed-wi
// https://github.com/valinet/ExplorerPatcher/issues/266
// /https://forum.rehips.com/index.php?topic=9599.0
// https://github.com/valinet/ExplorerPatcher/issues/161
// https://github.com/valinet/sws/blob/55f2ba472b2fcfe9419e5aef88a5b0a4141030f1/SimpleWindowSwitcher/sws_WindowHelpers.c  872
// uwp apps are.. complicated *sigh*..
bool WindowQueryHelper::inspectWindows10AppWindow(HWND hWnd, bool cloakTest = true) {
	bool foundTHEchild = false;
	//Sleep(2000);
	EnumChildWindows(hWnd, ProcessWindow, reinterpret_cast<LPARAM>(&foundTHEchild));

	// Windows.UI.Core.CoreWindow
	// ApplicationFrame
	// 
	if (foundTHEchild == true) {
		//outputDebugStringA("--working--\n");
		return true;

	}
	return false;
}


char __fastcall _sws_IsOwnerToolWindow(HWND hWnd) {
	char v1; // bl
	HWND v2; // rsi
	HWND Window; // rdi
	HWND v4; // rbp
	HWND v5; // rax
	HWND v6; // rcx

	v1 = 0;
	v2 = hWnd;
	Window = GetWindow(hWnd, 4u);
	while ((GetWindowLongPtrW(v2, -20) & 0x40000) == 0 && Window) {
		v4 = v2;
		v2 = Window;
		v5 = GetWindow(Window, 4u);
		v6 = Window;
		Window = v5;
		if ((GetWindowLongPtrW(v6, -20) & 0x80) == 0x80) {
			if ((GetWindowLongPtrW(v4, -20) & 0x10000) == 0 || Window)
				return 1;

			return v1;
		}
	}

	return v1;
}

__int64 __fastcall WindowQueryHelper::IsTaskWindow(HWND a2) {
	HWND v5; // rdi
	unsigned int v6; // ebx
	int WindowLongPtrW; // ebp
	HWND Window; // rsi
	char v9; // si
	char v10; // al
	char v11; // r12
	BOOL v12; // bp
	HWND i; // rcx
	int v14; // eax
	HWND v15; // rax
	HWND v16; // rsi
	RECT Rect; // [rsp+20h] [rbp-38h] BYREF
	RECT rc; // [rsp+30h] [rbp-28h] BYREF

	v5 = a2;
	v6 = 0;
	if (IsWindow(a2)) {
		WindowLongPtrW = GetWindowLongPtrW(v5, -20);
		Window = GetWindow(v5, 4u);
		if (!IsWindow(Window)
			//|| !IsWindowEnabled(Window)
			|| (GetWindowRect(Window, &Rect), !IsWindowVisible(Window))
			|| (v9 = 1, IsRectEmpty(&Rect))) {
			v9 = 0;
		}

		if ((WindowLongPtrW & 0x8000000) != 0 || (v10 = 0, (WindowLongPtrW & 0x80u) != 0))
			v10 = 1;

		v11 = v10;
		v12 = (WindowLongPtrW & 0x40000i64) != 0;
		if (v12)
			v11 = 0;

		GetWindowRect(v5, &rc);
		if (IsWindowVisible(v5)
			&& !IsRectEmpty(&rc)
			//&& IsWindowEnabled(v5)
			&& !v11
			&& (v12 || !v9 && !_sws_IsOwnerToolWindow(v5))
			&& !_GhostWindowFromHungWindow(v5)) {
			for (i = v5; ; i = v16) {
				v15 = GetWindow(i, 4u);
				v16 = v15;
				if (!v15)
					break;

				v14 = GetWindowLongPtrW(v15, -20);
				if ((v14 & 0x40000i64) == 0 && ((v14 & 0x80u) != 0 || (v14 & 0x8000000) != 0))
					break;

				if (!IsWindowVisible(v16) || _GhostWindowFromHungWindow(v16))
					break;

				v5 = v16;
			}

			return 1;
		}
	}

	return v6;
}

BOOL ShouldTreatShellManagedWindowAsNotShellManaged(HWND hWnd) {
	return (GetPropW(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") != NULL);
}

// https://github.com/valinet/ExplorerPatcher/issues/161 Thank you valinet <3 <3 for your time researching into windows api to allow people to extend microsofts operating system 
bool WindowQueryHelper::isAppWindow(HWND hwnd, bool checkWin10) {
	// This identifies whether a window is a shell frame and includes those
	// A shell frame corresponds to, as far as I can tell, the frame of a UWP app
	// and we want those in the Alt-Tab list
	// Bugfix: Exclude hung shell frame (immersive) UWP windows, as we already include
	// ghost app windows in their place already
	if (_IsShellFrameWindow(hwnd) && !_GhostWindowFromHungWindow(hwnd)) {
		if (inspectWindows10AppWindow(hwnd, checkWin10))
			return true;

	}
	// Next, we need to check whether the window is shell managed and exclude it if so
	// Shell managed windows, as far as I can tell, represent all immersive UI the
	// Windows shell might present the user with, like: Start menu, Search (Win+Q),
	// notifications, taskbars etc
	if (_IsShellManagedWindow(hwnd) && !ShouldTreatShellManagedWindowAsNotShellManaged(hwnd)) {
		return FALSE;
	}

	// Lastly, this check works with the remaining classic window and determines if it is a
	// "task window" and only includes it in Alt-Tab if so; this check is taken fromf	ffffffffffffffff
	// "AltTab.dll" in Windows 7 and this is how that OS decided to include a window in its
	// window switcher
	return IsTaskWindow(hwnd);
}

void winEventProc(
	HWINEVENTHOOK hWinEventHook,
	DWORD event,
	HWND hwnd,
	LONG idObject,
	LONG idChild,
	DWORD idEventThread,
	DWORD dwmsEventTime
) {
	// SHOULD REMOVE THE LITERAL..
	if ((event == EVENT_OBJECT_UNCLOAKED) && hwnd && idObject == OBJID_WINDOW && _IsShellFrameWindow(hwnd) && !_GhostWindowFromHungWindow(hwnd)) {
		SendMessage(FindWindowW((L"LiteBoxDesktopWindoww"), NULL), WM_ShellHook, WM_USER + 44, (LPARAM)hwnd); // sends the UWP message to the SHELLHOOK proc wit WM_WINDOWCREATED
	}

}

/// ---- 


BOOL WindowQueryHelper::initWinHelper() {


	if (!SetWinEventHook(
		EVENT_MIN,
		EVENT_MAX,
		NULL,
		winEventProc,
		0,
		0,
		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
	)) {
		//outputDebugString(L"Failed to hook to win EVENTS\n");
		return false;
	}

	return true;
}
