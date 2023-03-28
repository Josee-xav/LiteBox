#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <string>
#include <ShellScalingApi.h>

enum class IconSizes
{
	icon_small,
	icon_big
};
// TODO make it nicer rather than all types of shit everywhere.
namespace LB_Api {
	extern HINSTANCE main_hinstance;
	bool isIconValid(HICON icon);

	bool IsWinShell(HWND hwnd);
	bool isFullscreen(HWND hwnd);
	bool executeShell(HWND hwnd, const wchar_t* verb, const wchar_t* file, const wchar_t* args, const wchar_t* dir, int showCmds, int flags);
	bool register_class(const wchar_t* classname, HINSTANCE hinstance, WNDPROC wndproc, int flags);
	void restartExplorerWindow();


	std::wstring getWindowTitle(HWND hwnd);
	HBITMAP getBitmapFromHicon(HICON icon);
	HICON getHICONFromHWND(HWND hwnd, IconSizes iconsize);

	//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
	std::string getLasterrorAsString();





	RECT getPrimaryScreenRes();

	void setWorkArea(int height);

	std::wstring getWindowClassName(HWND hwnd);

	std::wstring getLBExePath();


	void showExplorer();
	void hideExplorer();
	HWND findTrayToolbarWindow();



	bool convert_string(char* dest, const void* src, int nmax, bool is_unicode);
	wchar_t* charToWChar(const char* text);

};

