#include <Windows.h>

#ifdef _MSC_VER
#pragma data_seg( ".shared" )
#pragma comment(linker, "/SECTION:.shared,RWS")
#define SHARED(T,X) T X = (T)0
#endif


SHARED(HHOOK, hTrayHook);
SHARED(HWND, hShellTrayWnd);
SHARED(HWND, bbTrayWnd);

#ifdef _MSC_VER
#pragma comment(linker, "/SECTION:.shared,RWS")
#pragma data_seg()
#endif

HINSTANCE hInstance;


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		hInstance = hDLLInst;
		//dbg_printf("Attached to %x", hDLLInst);
		break;

	case DLL_PROCESS_DETACH:
		//dbg_printf("unloaded.");
		break;
	}
	return TRUE;
}

static LRESULT CALLBACK WinTrayProc(int code, WPARAM wparam, LPARAM lparam)
{
	if (code >= 0)
	{
		CWPSTRUCT* cwps = (CWPSTRUCT*)lparam;
		if (cwps->hwnd == hShellTrayWnd
			&& cwps->message == WM_COPYDATA
			&& (((COPYDATASTRUCT*)cwps->lParam)->dwData == 1
				|| ((COPYDATASTRUCT*)cwps->lParam)->dwData == 0
				))
		{
			SendMessage(bbTrayWnd, WM_COPYDATA, cwps->wParam, cwps->lParam);
		}
	}
	return CallNextHookEx(hTrayHook, code, wparam, lparam);
}

extern "C" __declspec(dllexport) int EntryFunc(HWND trayWnd)
{
	if (trayWnd) {
		hShellTrayWnd = FindWindow(L"Shell_TrayWnd", NULL);
		bbTrayWnd = trayWnd;
		hTrayHook = SetWindowsHookEx(WH_CALLWNDPROC, WinTrayProc, hInstance, 0);
	}
	else {
		if (hTrayHook)
			UnhookWindowsHookEx(hTrayHook);
	}
	return 0;
}