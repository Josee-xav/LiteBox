#pragma once
#include "barItem.h"
#include <vector>
#include "../StylingStruct.h"

enum E_BarTimers
{
    CLOCK_TIMER = 2 ,
    LANGUAGE_TIMER = 3 ,
    LABEL_TIMER = 4 ,
    TASK_RISE_TIMER = 5 ,
    TASKLOCK_TIMER = 6 ,
    GESTURE_TIMER = 7 ,
    CHECK_FULLSCREEN_TIMER = 8
};

// possible bar items
enum BARITEMS
{
    M_EOS = 0 ,
    M_NEWLINE ,

    M_CLOCK ,
    M_TASK ,
    M_TRAY ,

    // list classes
    M_BARLIST = 256 ,
    M_TASKLIST ,
    M_TRAYLIST
};


namespace taskbarItemFlags {
    enum
    {
        itemSeparator = 0x1 ,                            // not valid with anything else
        itemNotSelectable = 0x2 ,                        // cannot select this item
        itemBold = 0x4 ,                                 // display in bold face text
        itemGrayed = 0x8 ,                                // display grayed out
        M_TASKBUTTONACTIVE_FLAG = 0x9
    };
}

class taskEntryBtn : public barItem
{
public:
    //-----------------------------
    taskEntryBtn();

    //-----------------------------
    ~taskEntryBtn() override;

    //-----------------------------
    void draw(HWND hWnd , HDC hDC) override; // NEVERUSED override


    void mouse_event(int mx , int my , int message , unsigned flags) override;
};

enum class clockFormat
{
    format_24hour ,
    format_12hour ,
};

class clockBtn : public barItem
{
private:
    static bool classRegistered;          // once-per-app window class registration flag
    HWND messageWindow;
    std::wstring convert24hourTo12HourTime(int time24Hour , int time24minute);
    void updateClock(clockFormat format);
    static LRESULT CALLBACK timerProc(HWND hwnd , UINT msg , WPARAM wParam , LPARAM lParam);

public:
    //-----------------------------
    clockBtn();

    //-----------------------------
    ~clockBtn() override;

    void createClockTimer();

    //-----------------------------
    void draw(HWND hWnd , HDC hDC) override;

    void mouse_event(int mx , int my , int message , unsigned flags) override;
};

class TrayEntryBtn : public barItem
{
public:

    typedef struct _TrayItem
    {
        HWND hWnd;
        UINT uID;
        UINT uCallbackMessage;
        DWORD dwState;
        UINT uVersion;
        HICON hIcon;
        ULONG  uIconDemoteTimerID;
        DWORD dwUserPref;
        DWORD dwLastSoundTime;
        TCHAR	sExeName[MAX_PATH];
        TCHAR	sIconText[MAX_PATH];
        UINT uNumSeconds;
        GUID guidItem;
    } TrayItem;


    void trayMouseDown(int message);
    void trayMouseUp(int message);
    void trayMouseDOUBLECLICKDown(int message);


    //-----------------------------
    TrayEntryBtn();

    //-----------------------------
    ~TrayEntryBtn() override;

    //-----------------------------
    void draw(HWND hWnd , HDC hDC) override; // NEVERUSED override

    void mouse_event(int mx , int my , int message , unsigned flags) override;
};



// LISTS

class baritemlist : public barItem
{
protected:
    // caculates the items in the actual baritemlist. 
    virtual bool calc_itemsSizes();

public:
    std::vector<barItem*> m_Items;                     // items in this menu

    //-----------------------------
    baritemlist(int type);

    //-----------------------------
    ~baritemlist() override;

    int getItem(short x , short y , const RECT& rect);

    // adds a bar item to the list.
    virtual void add(barItem* entry);

    virtual void remove(barItem* entry);

    // clears the list.
    virtual void clear(void);

    void draw(HWND hWnd , HDC hDC) override;

    // checks every bar item to see if it was the one which the user clicked on.
    void mouse_event(int mx , int my , int message , unsigned flags) override;

    // calc size of the baritemlist as a whole and its items in it, automatically.
    bool calc_size(int* px , int y , int w , int h , int m) override;
};

class taskItemList : public baritemlist
{
private:
    bool calc_itemsSizes() override;

public:

    taskItemList();

    void remove(barItem* entry) override;

};

class trayItemList : public baritemlist
{
public:
    trayItemList();


};