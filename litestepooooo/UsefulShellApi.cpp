#include "UsefulShellApi.h"

std::vector<HWND> explorerHwnds;

int ShellApi::messageBox(int uType , HINSTANCE instance , std::wstring title , std::wstring text)
{
    MSGBOXPARAMS mb;
    ZeroMemory(&mb , sizeof mb); // "To avoid any undesired effects of optimizing compilers"
    mb.cbSize = sizeof mb;
    mb.hInstance = instance;
    //mp.hwndOwner = NULL;
    mb.lpszText = text.c_str();
    mb.lpszCaption = title.c_str();
    mb.dwStyle = uType | MB_SYSTEMMODAL | MB_SETFOREGROUND;
    MessageBeep(0);
    return MessageBoxIndirect(&mb);
}

std::wstring ShellApi::getWindowTitle(HWND hwnd)
{
    wchar_t windowTitle[100];
    if (GetWindowText(hwnd , windowTitle , 100))
        return windowTitle;
    else
        return L"ERROR";
}

HBITMAP ShellApi::getBitmapFromHicon(HICON icon)
{
    HICON copyHicon = CopyIcon(icon);
    ICONINFO iconinfo;
    GetIconInfo(copyHicon , &iconinfo);

    return iconinfo.hbmColor;
}

/*****************************/
/* Draw a transparent bitmap */
/*****************************

This function is borrowed and slightly modified from an MSDN sample */


HICON ShellApi::getHICONFromHWND(HWND hwnd , IconSizes iconsize)
{
    HICON hIco = NULL;
    HICON copyHicon = NULL;

    SendMessageTimeout(hwnd , WM_GETICON , iconsize == IconSizes::icon_big ? ICON_BIG : ICON_SMALL , 0 , SMTO_ABORTIFHUNG | SMTO_NORMAL , 500 , (DWORD_PTR*)&hIco);
    if (NULL == hIco) {
        hIco = (HICON)GetClassLong(hwnd , iconsize == IconSizes::icon_big ? GCLP_HICON : GCLP_HICONSM);
        if (NULL == hIco) {
            SendMessageTimeout(hwnd , WM_GETICON , iconsize == IconSizes::icon_big ? ICON_SMALL : ICON_BIG , 0 , SMTO_ABORTIFHUNG | SMTO_NORMAL , 500 , (DWORD_PTR*)&hIco);
            if (NULL == hIco) {
                hIco = (HICON)GetClassLong(hwnd , iconsize == IconSizes::icon_big ? GCLP_HICONSM : GCLP_HICON);

            }
        }
    }
    if (hIco == NULL)
        hIco = CopyIcon(LoadIcon(NULL , IDI_ASTERISK));

    return hIco;
}

RECT ShellApi::getPrimaryScreenRes()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    return RECT{ 0, 0, screenWidth, screenHeight };
}

#define RUN_SHOWERRORS  0
#define RUN_NOERRORS    1
#define RUN_WAIT        2
#define RUN_HIDDEN      4
#define RUN_NOARGS      8
#define RUN_NOSUBST    16
#define RUN_ISPIDL     32
#define RUN_WINDIR     64
bool ShellApi::executeShell(HWND hwnd , const wchar_t* verb , const wchar_t* file , const wchar_t* args , const wchar_t* dir , int showCmds , int flags)
{
    SHELLEXECUTEINFO sei;

    if (NULL == dir || 0 == dir[0]) {
        std::wstring temp = ShellApi::getExePath().c_str();
        dir = temp.c_str();
    }

    memset(&sei , 0 , sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = hwnd;
    sei.lpVerb = verb;
    sei.lpParameters = args;
    sei.lpDirectory = dir;
    sei.nShow = showCmds;

    if (flags & RUN_ISPIDL) {
        sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
        sei.lpIDList = (void*)file;
    }
    else {
        sei.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;
        sei.lpFile = file;
        if (NULL == file || 0 == file[0]) {
            /*if (0 == (flags & RUN_NOERRORS)) {
                char msg[200];
                MessageBox(MB_OK , NLS2("$Error_Execute$" ,
                           "Error: Could not execute: %s\n(%s)") ,
                           file && file[0] ? file : NLS1("<empty>") ,
                           win_error(msg , sizeof msg));
            }*/

            return false;
        }
    }

    if (ShellExecuteEx(&sei))
        return TRUE;

    /* if (0 == (flags & RUN_NOERRORS)) {
         char msg[200];
         BBMessageBox(MB_OK , NLS2("$Error_Execute$" ,
                      "Error: Could not execute: %s\n(%s)") ,
                      file && file[0] ? file : NLS1("<empty>") ,
                      win_error(msg , sizeof msg));
     }*/

    return false;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string ShellApi::getLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS ,
                                 NULL , errorMessageID , MAKELANGID(LANG_NEUTRAL , SUBLANG_DEFAULT) , (LPSTR)&messageBuffer , 0 , NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer , size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

// TODO: sometimes doesnt work.... idfk whyyyyy cus it works sometimes but other times it doesnt... fking windows 10.
void ShellApi::setWorkArea(int height)
{
    //setOldDesktopWorkArea();
    RECT primaryScreenRect = getPrimaryScreenRes();

    primaryScreenRect.bottom = primaryScreenRect.bottom - height;
    bool ret = SystemParametersInfo(SPI_SETWORKAREA , 0 , &primaryScreenRect , 0);

    if (ret) {
        OutputDebugStringA((getLastErrorAsString() + "\n").c_str());
    }
}



std::wstring ShellApi::getWindowClassName(HWND hwnd)
{
    wchar_t windowClass[256] = {};
    if (GetClassName(hwnd , windowClass , sizeof(windowClass)))
        return windowClass;
    else
        return L"CLASS ERROR";
}

std::wstring ShellApi::getExePath()
{

    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL , buffer , MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0 , pos);

}

void hideWindow(HWND hwnd)
{
    if (hwnd && (ShowWindow(hwnd , SW_HIDE)))
        explorerHwnds.push_back(hwnd);
}

BOOL CALLBACK EnumExplorerWindowsProc(HWND hwnd , LPARAM lParam)
{
    wchar_t temp[32];
    if (GetClassName(hwnd , temp , 32)
        && (0 == wcscmp(temp , L"BaseBar")
        || 0 == wcscmp(temp , L"Button")
        ))
        hideWindow(hwnd);
    return TRUE;
}

void ShellApi::hideExplorer()
{
    HWND hw = FindWindow(L"Shell_TrayWnd" , NULL);
    if (hw) {
        hideWindow(hw);
        EnumWindows((WNDENUMPROC)EnumExplorerWindowsProc , 0);
    }
}

void ShellApi::restartExplorerWindow()
{
    DWORD dwPID;
    HWND hSysTray = ::FindWindow(TEXT("Shell_TrayWnd") , NULL);
    GetWindowThreadProcessId(hSysTray , &dwPID);
    HANDLE explorerHandle = OpenProcess(PROCESS_TERMINATE , FALSE , dwPID);

    if (explorerHandle) {
        TerminateProcess(explorerHandle , 0);
    }
    ShellExecute(NULL , NULL , L"explorer.exe" , NULL , NULL , SW_HIDE);
}

void ShellApi::showExplorer()
{
    if (explorerHwnds.size() == 0)
        restartExplorerWindow();
    else
        for (int i = 0; i < explorerHwnds.size(); i++)
            ShowWindow(explorerHwnds.at(i) , SW_SHOW);

    explorerHwnds.clear();
}