#include "Utils.h"

std::vector<HWND> explorerHwnds;
HINSTANCE LB_Api::main_hinstance = NULL;
#pragma warning(disable : 4996)


std::wstring LB_Api::getWindowTitle(HWND hwnd)
{
    wchar_t windowTitle[100];
    if(GetWindowText(hwnd, windowTitle, 100))
        return windowTitle;
    else
        return LB_Api::getWindowClassName(hwnd);
}

HBITMAP LB_Api::getBitmapFromHicon(HICON icon)
{
    HICON copyHicon = CopyIcon(icon);
    ICONINFO iconinfo;
    GetIconInfo(copyHicon, &iconinfo);

    return iconinfo.hbmColor;
}

/*****************************/
/* Draw a transparent bitmap */
/*****************************

This function is borrowed and slightly modified from an MSDN sample */


HICON LB_Api::getHICONFromHWND(HWND hwnd, IconSizes iconsize)
{
    HICON hIco = NULL;

    SendMessageTimeout(hwnd, WM_GETICON, iconsize == IconSizes::icon_big ? ICON_BIG : ICON_SMALL, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, (DWORD_PTR*)&hIco);
    if(NULL == hIco) {
        hIco = (HICON)GetClassLong(hwnd, iconsize == IconSizes::icon_big ? GCLP_HICON : GCLP_HICONSM);
        if(NULL == hIco) {
            SendMessageTimeout(hwnd, WM_GETICON, iconsize == IconSizes::icon_big ? ICON_SMALL : ICON_BIG, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, (DWORD_PTR*)&hIco);
            if(NULL == hIco) {
                hIco = (HICON)GetClassLong(hwnd, iconsize == IconSizes::icon_big ? GCLP_HICONSM : GCLP_HICON);

            }
        }
    }
    if(hIco == NULL)
        hIco = CopyIcon(LoadIcon(NULL, IDI_ASTERISK));

    return hIco;
}

RECT LB_Api::getPrimaryScreenRes()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    return RECT{ 0, 0, screenWidth, screenHeight };
}

#define RUN_SHOWerrorS  0
#define RUN_NOerrorS    1
#define RUN_WAIT        2
#define RUN_HIDDEN      4
#define RUN_NOARGS      8
#define RUN_NOSUBST    16
#define RUN_ISPIDL     32
#define RUN_WINDIR     64

bool LB_Api::IsWinShell(HWND hwnd)
{
    std::wstring className = LB_Api::getWindowClassName(hwnd);
    if(className == L"Shell_TrayWnd" || className == L"DV2ControlHost" || className == L"MsgrIMEWindowClass" || className == L"SysShadow" || className == L"Button"
        || className == L"Windows.UI.Core.CoreWindow" || className == L"Frame Alternate Owner" || className == L"MultitaskingViewFrame" || className == L"Progman" || className == L"WorkerW") {
        return true;
    }

    if(hwnd == GetShellWindow() || hwnd == GetDesktopWindow())
        return true;

    return false;
}

bool LB_Api::isFullscreen(HWND hwnd)
{
    if(LB_Api::IsWinShell(hwnd))
        return false;

    HMONITOR hwndMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    if(!hwndMon)
        return false;
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if(!GetMonitorInfo(hwndMon, &mi))
        return false;

    RECT appBounds = { 0 };
    GetWindowRect(hwnd, &appBounds);

    if(EqualRect(&appBounds, &mi.rcMonitor)) {
        return true;
    }

    return false;
}

// THE
//TODO NO USE?
bool LB_Api::executeShell(HWND hwnd, const wchar_t* verb, const wchar_t* file, const wchar_t* args, const wchar_t* dir, int showCmds, int flags)
{
    SHELLEXECUTEINFO sei;

    if(NULL == dir || 0 == dir[0]) {
        std::wstring temp = LB_Api::getLBExePath().c_str();
        dir = temp.c_str();
    }

    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = hwnd;
    sei.lpVerb = verb;
    sei.lpParameters = args;
    sei.lpDirectory = dir;
    sei.nShow = showCmds;

    if(flags & RUN_ISPIDL) {
        sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
        sei.lpIDList = (void*)file;
    }
    else {
        sei.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;
        sei.lpFile = file;
        if(NULL == file || 0 == file[0]) {
            /*if (0 == (flags & RUN_NOerrorS)) {
                char msg[200];
                MessageBox(MB_OK , NLS2("$error_Execute$" ,
                           "error: Could not execute: %s\n(%s)") ,
                           file && file[0] ? file : NLS1("<empty>") ,
                           win_error(msg , sizeof msg));
            }*/

            return false;
        }
    }

    if(ShellExecuteEx(&sei))
        return TRUE;

    /* if (0 == (flags & RUN_NOerrorS)) {
         char msg[200];
         BBMessageBox(MB_OK , NLS2("$error_Execute$" ,
                      "error: Could not execute: %s\n(%s)") ,
                      file && file[0] ? file : NLS1("<empty>") ,
                      win_error(msg , sizeof msg));
     }*/

    return false;
}

bool LB_Api::register_class(const wchar_t* classname, HINSTANCE hinstance, WNDPROC wndproc, int flags)
{
    WNDCLASS wc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = classname;

    return ::RegisterClass(&wc) != FALSE;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string LB_Api::getLasterrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

// TODO: sometimes doesnt work.... fking windows 10.
void LB_Api::setWorkArea(int height)
{
    //setOldDesktopWorkArea();
    RECT primaryScreenRect = getPrimaryScreenRes();

    primaryScreenRect.bottom = primaryScreenRect.bottom - height;
    bool ret = SystemParametersInfo(SPI_SETWORKAREA, 0, &primaryScreenRect, 0);

}



std::wstring LB_Api::getWindowClassName(HWND hwnd)
{
    wchar_t windowClass[256] = {};
    if(GetClassName(hwnd, windowClass, sizeof(windowClass)))
        return windowClass;
    else
        return L"error";
}

std::wstring LB_Api::getLBExePath()
{

    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);

}

void hideWindow(HWND hwnd)
{
    if(hwnd && ( ShowWindow(hwnd, SW_HIDE) ))
        explorerHwnds.push_back(hwnd);
}

BOOL CALLBACK EnumExplorerWindowsProc(HWND hwnd, LPARAM lParam)
{
    wchar_t temp[32];
    if(GetClassName(hwnd, temp, 32)
        && ( 0 == wcscmp(temp, L"BaseBar")
       || 0 == wcscmp(temp, L"Button")
       ))
        hideWindow(hwnd);
    return TRUE;
}

void LB_Api::hideExplorer()
{
    HWND hw = FindWindow(L"Shell_TrayWnd", NULL);
    if(hw) {
        hideWindow(hw);
        EnumWindows((WNDENUMPROC)EnumExplorerWindowsProc, 0);
    }
}

void LB_Api::restartExplorerWindow()
{
    DWORD dwPID;
    HWND hSysTray = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);
    GetWindowThreadProcessId(hSysTray, &dwPID);
    HANDLE explorerHandle = OpenProcess(PROCESS_TERMINATE, FALSE, dwPID);

    if(explorerHandle) {
        TerminateProcess(explorerHandle, 0);
    }
    ShellExecute(NULL, NULL, L"explorer.exe", NULL, NULL, SW_HIDE);
}

wchar_t* LB_Api::charToWChar(const char* text)
{
    const size_t size = strlen(text) + 1;
    wchar_t* wText = new wchar_t[size];
    mbstowcs(wText, text, size);//TODO
    return wText;
}

// true when icon is valid 
bool LB_Api::isIconValid(HICON icon)
{
    ICONINFO iconInfo;
    BOOL success = GetIconInfo(icon, &iconInfo);

    bool ret = ( success != false || iconInfo.hbmColor != NULL || iconInfo.hbmMask != NULL );

    DeleteObject(iconInfo.hbmMask);
    DeleteObject(iconInfo.hbmColor);
    return ret;
}

void LB_Api::showExplorer()
{
    if(explorerHwnds.size() == 0)
        restartExplorerWindow();
    else
        for(int i = 0; i < explorerHwnds.size(); i++)
            ShowWindow(explorerHwnds.at(i), SW_SHOW);

    explorerHwnds.clear();
}

HWND LB_Api::findTrayToolbarWindow()
{
    HWND hWnd = NULL;

    if(( hWnd = FindWindow(( L"Shell_TrayWnd" ), NULL) ) != NULL) {
        if(( hWnd = FindWindowEx(hWnd, 0, ( L"TrayNotifyWnd" ), NULL) ) != NULL) {
            if(hWnd = FindWindowEx(hWnd, 0, ( L"SysPager" ), NULL)) {
                hWnd = FindWindowEx(hWnd, 0, ( L"ToolbarWindow32" ), NULL);
            }
        }
    }

    return hWnd;
}

int wideToMulti(const WCHAR* src, char* str, int len)
{
    int x, n;
    for(x = -1;;) {
        n = WideCharToMultiByte(
            CP_UTF8,
            0, src, x, str, len, NULL, NULL
        );
        if(n)
            return n;
        if(x < 0)
            x = len;
        if(--x == 0)
            break;
    }
    str[0] = 0;
    return 0;
}
bool LB_Api::convert_string(char* dest, const void* src, int nmax, bool is_unicode)
{
    char buffer[256];
    if(is_unicode) {
        wideToMulti((const WCHAR*)src, buffer, sizeof buffer); // TODO
        src = buffer;
    }
    if(strcmp(dest, (const char*)src)) {
        strcpy_s(dest, nmax, (const char*)src);
        return true;
    }
    return false;
}