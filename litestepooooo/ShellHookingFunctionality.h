#pragma once
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern unsigned WM_ShellHook;

void initShellHook(HWND hwnd);
void exitShellHook(HWND hwnd);