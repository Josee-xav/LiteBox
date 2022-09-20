#include "ShellHookingFunctionality.h"

pHungWindowFromGhostWindow _HungWindowFromGhostWindow;
pGhostWindowFromHungWindow _GhostWindowFromHungWindow;
pIsShellManagedWindow _IsShellManagedWindow;
pIsShellManagedWindow _IsShellFrameWindow;

unsigned WM_ShellHook = 0;
HINSTANCE _hUser32 = 0;

// registers the Shell32
typedef void (WINAPI* pRegisterShellHook)(HWND , DWORD);
pRegisterShellHook _pRegisterShellHook;

static const char* rtl_libs[] =
{
    "SHELL32",
    NULL
};

void init_shellHookFuncs(void)
{

	_pRegisterShellHook = (pRegisterShellHook)GetProcAddress(LoadLibraryA("SHELL32") , (char*)0xB5);

}

void exit_shellHookFuncs(void)
{
    const char** p = rtl_libs;
    do FreeLibrary(GetModuleHandleA(*p)); while (*++p);
}



void register_shellhook(HWND hwnd)
{
    if (_pRegisterShellHook) {
        _pRegisterShellHook(NULL , TRUE);
        _pRegisterShellHook(hwnd , 3);
        WM_ShellHook = RegisterWindowMessage(L"SHELLHOOK");
    }
}

void unregister_shellhook(HWND hwnd)
{
    if (_pRegisterShellHook) {
        _pRegisterShellHook(hwnd , 0);
    }
}



BOOL getShellFunctions() {

	if(!_hUser32) {
		_hUser32 = LoadLibraryW(L"user32.dll");
		if(!_hUser32) {
			OutputDebugStringA("ERRORORRRR IN INIT HELPER");
			return false;
		}
	}

	if(!_IsShellManagedWindow) {
		_IsShellManagedWindow = (pIsShellManagedWindow)GetProcAddress(_hUser32, (LPCSTR)2574);
		if(!_IsShellManagedWindow) {
			OutputDebugStringA("ERRORORRRR IN INIT HELPER");
			return false;
		}
	}
	if(!_IsShellFrameWindow) {
		_IsShellFrameWindow = (pIsShellManagedWindow)GetProcAddress(_hUser32, (LPCSTR)2573);
		if(!_IsShellFrameWindow) {
			OutputDebugStringA("ERRORORRRR IN INIT HELPER");
			return false;
		}
	}
	if(!_GhostWindowFromHungWindow) {
		_GhostWindowFromHungWindow = (pGhostWindowFromHungWindow)GetProcAddress(_hUser32, "GhostWindowFromHungWindow");
		if(!_GhostWindowFromHungWindow) {
			OutputDebugStringA("ERRORORRRR IN INIT HELPER");
			return false;
		}

	}
	if(!_HungWindowFromGhostWindow) {
		_HungWindowFromGhostWindow = (pHungWindowFromGhostWindow)GetProcAddress(_hUser32, "HungWindowFromGhostWindow");
		if(!_HungWindowFromGhostWindow) {
			OutputDebugStringA("ERRORORRRR IN INIT HELPER");
			return false;
		}
	}

	return TRUE;
}

void initShellHook(HWND hwnd)
{

    init_shellHookFuncs();
	getShellFunctions();
	WindowQueryHelper::initWinHelper();
    register_shellhook(hwnd);
    
    
}

void exitShellHook(HWND hwnd)
{
    exit_shellHookFuncs();
    unregister_shellhook(hwnd);
}