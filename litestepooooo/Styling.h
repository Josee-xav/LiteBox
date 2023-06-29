#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>


// If you add a optionn on here then you also need to add to StyleItems in settings.cpp for the parser to get the data.
struct _Stylizing
{
	//colors of stuff.
	static COLORREF windowBackgroundColor;
	static COLORREF task_FocusColor;
	static COLORREF clock_Color;
	static COLORREF borderColor;
	static COLORREF unfocusedTextColor;
	static COLORREF focusedTextColor;



	// roundedness
	static bool rectRoundedEdge_Clock;
	static bool rectRoundedEdge_TaskButtons;


	//WIDTHS TUFF
	static int taskbar_Width;

	static int border_Width;

	//height stff
	static int taskbar_Height;

	static int task_iconSize;
	static int tray_iconSize;

	//===

	//spacings
	static int task_buttonSpacing;
	static int task_buttonTopSpacing;

	//font stuff
	static std::wstring font;
	static int task_fontSize;
	static int clock_fontSize;
	static int menu_fontSize;
	static LONG fontWeight;

	// bevels
	static int task_BevelStyle;
	static int clock_BevelStyle;

	//misc
	static int clock_TimeFormat;
	static bool tasks_IconOnly;



};

//style
extern _Stylizing m_Style;

void readStyle(const char* styleFilePath);
