#pragma once
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "WindowQueryHelpers.h"


typedef struct {
	int nAttribute;
	PVOID pData;
	ULONG ulDataSize;
} WINCOMPATTRDATA;
typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
extern pSetWindowCompositionAttribute _SetWindowCompositionAttribute;

typedef BOOL(WINAPI* pIsShellManagedWindow)(HWND);
extern pIsShellManagedWindow _IsShellManagedWindow;
extern pIsShellManagedWindow _IsShellFrameWindow;

typedef HWND(WINAPI* pHungWindowFromGhostWindow)(HWND);
extern pHungWindowFromGhostWindow _HungWindowFromGhostWindow;
typedef HWND(WINAPI* pGhostWindowFromHungWindow)(HWND);
extern pGhostWindowFromHungWindow _GhostWindowFromHungWindow;


extern unsigned WM_ShellHook;
extern HINSTANCE _hUser32 ;

void initShellFuncs(HWND hwnd);
void exitShellFuncs(HWND hwnd);