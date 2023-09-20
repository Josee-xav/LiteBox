#include "TrayService.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>
#include "../Utils.h"
#include "../Logging/FLogger.h"

HWND TrayService::hTrayWnd = NULL;
HINSTANCE TrayService::hinstTrayLib = NULL;
trayItemList* TrayService::trayBtnList = NULL;

trayHookDll_EntryFunc TrayService::th_libfunc = NULL;

TrayService::TrayService()
{
    trayBtnList = new trayItemList();
    initTrayService();
}

TrayService::~TrayService()
{
    if (hinstTrayLib != nullptr) {
        (th_libfunc)(NULL);
        FreeLibrary(hinstTrayLib);
    }
}

trayItemList* TrayService::getTrayList()
{
    return trayBtnList;
}

HWND TrayService::create_Tray_Child(HWND hwndParent, const wchar_t* class_name)
{
    LB_Api::register_class(class_name, LB_Api::main_hinstance, TrayNotifyWndProc, 0);

    return CreateWindow(
        class_name,
        NULL,
        WS_CHILD,
        0, 0, 0, 0,
        hwndParent,
        NULL,
        LB_Api::main_hinstance,
        NULL
    );

}
void TrayService::initTrayService()
{
    trayBtnList = new trayItemList();
    std::vector< TrayEntryBtn::_TrayItem*> trayItems = getTrayItems();

    for (int i = 0; i < trayItems.size(); i++) {
        AppendTrayBtn(0, trayItems.at(i)->sIconText, trayItems.at(i)->hWnd, trayItems.at(i)->hIcon, trayItems.at(i));
        FLogger::debug("LINE 52, appending tray btn via send message mthod. tray name:  %S", trayItems.at(i)->sIconText);
    }
    // ^ i feel like this is a better way as the tray hook doesnt seem to work 100% the time for me on windows 10
    // TODO mabye just hook shell_notify instead..... super autistic tho.


    FLogger::info("Tray Starting!", 0);

    std::wstring trayClassName = L"Shell_TrayWnd";

    hinstTrayLib = LoadLibrary(TEXT("TrayHook.dll"));


    if (hinstTrayLib != NULL) {
        th_libfunc = (trayHookDll_EntryFunc)GetProcAddress(hinstTrayLib, "EntryFunc");

        // If the function address is valid, call the function.

        if (NULL == th_libfunc) {
            FLogger::error("Hook failed.");
            return;
        }

    }
    else {
        MessageBoxA(NULL, "TRAYHOOK", "couldnt load dll.", S_OK);
        FLogger::error("Couldnt load dll .");
    }

    if (!LB_Api::register_class(trayClassName.c_str(), LB_Api::main_hinstance, trayProc, 0)) {
        FLogger::error("Registering tray class failed. " + __LINE__);
    }

    hTrayWnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        trayClassName.c_str(),
        NULL,
        WS_POPUP,
        0, 0, 0, 0,
        NULL,
        NULL,
        LB_Api::main_hinstance,
        this
    );

    if (hTrayWnd == NULL) {
        FLogger::error("Tray create window failed!!! " + __LINE__);
    }

    if (th_libfunc != NULL) {
        (th_libfunc)(hTrayWnd);
    }
    else {
        // Some programs want these child windows so they can
        // figure out the presence/location of the tray.
        create_Tray_Child(
            create_Tray_Child(hTrayWnd, L"TrayNotifyWnd"),
            L"TrayClockWClass");
    }

    SendNotifyMessage(HWND_BROADCAST, RegisterWindowMessage(L"TaskbarCreated"), 0, 0);
}

// RETURNS vector.empty when something has failed. otherwise it returns a nice vector full of notification area icons  
std::vector<TrayEntryBtn::_TrayItem* > TrayService::getTrayItems()
{
    std::vector< TrayEntryBtn::_TrayItem*> trayItems;
    HWND toolBarhWnd = LB_Api::findTrayToolbarWindow();

    if (toolBarhWnd == NULL) {
        return {};
    }

    int nBtnCount = ::SendMessage(toolBarhWnd, TB_BUTTONCOUNT, 0, 0);
    if (nBtnCount < 1) {
        return {};
    }

    DWORD dwPID = 0;
    GetWindowThreadProcessId(toolBarhWnd, &dwPID);
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ, false, (int)dwPID);
    if (!hProcess) {
        return {};
    }

    LPVOID pvAddress = VirtualAllocEx(hProcess, 0, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
    for (int i = 0; i < nBtnCount; ++i) {
        TrayEntryBtn::_TrayItem* tray = new TrayEntryBtn::_TrayItem();
        SIZE_T nNumberOfBytesRead = 0;

        // Use ReadProcessMemory to read data from the process memory space: icon title, icon area
        ::SendMessage(toolBarhWnd, TB_GETBUTTON, i, (long)pvAddress);

        TBBUTTON bi = {};
        ReadProcessMemory(hProcess, (LPVOID)pvAddress, &bi, sizeof(TBBUTTON), &nNumberOfBytesRead);
        ReadProcessMemory(hProcess, (LPCVOID)bi.dwData, (LPVOID)tray, sizeof(TrayEntryBtn::_TrayItem), NULL);

        TCHAR szBtnText[_MAX_PATH] = { 0 };
        SendMessage(toolBarhWnd, TB_GETBUTTONTEXT, bi.idCommand, (LPARAM)(pvAddress));
        ReadProcessMemory(hProcess, (void*)(pvAddress), szBtnText, _MAX_PATH * sizeof(TCHAR), &nNumberOfBytesRead);

        //ReadProcessMemory(hProcess, (void*)(pvAddress), icon, _MAX_PATH * sizeof(HICON), &nNumberOfBytesRead);

        wcscpy_s(tray->sIconText, _MAX_PATH, szBtnText);
        tray->hIcon = CopyIcon(tray->hIcon);

        if (tray->hIcon == NULL)
            continue;

        trayItems.push_back(tray);
    }
    VirtualFreeEx(hProcess, pvAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return trayItems;
}

bool TrayService::AppendTrayBtn(const DWORD dwFlags, LPCTSTR pszName, HWND appHwnd, HICON icon, TrayEntryBtn::_TrayItem* trayItem)
{
    TrayEntryBtn* item = new TrayEntryBtn();

    item->m_dwFlags = dwFlags;

    if (pszName)
        item->m_strName = pszName;

    item->m_icon = icon;
    item->m_data = (LPARAM)trayItem;
    //item.m_pPopup = NULL;

    trayBtnList->add(item);
    return true;
}
void TrayService::extractTrayData(NIDBB* nid, void* trayData)
{

    int size = -1;
#ifdef _WIN64
    size = ((NIDNT_32*)trayData)->cbSize;
    nid->hWnd = (HWND)((NIDNT_32*)trayData)->hWnd;
    nid->uID = ((NIDNT_32*)trayData)->uID;
    nid->uFlags = ((NIDNT_32*)trayData)->uFlags;
    nid->uCallbackMessage = ((NIDNT_32*)trayData)->uCallbackMessage;
    nid->hIcon = (HICON)((NIDNT_32*)trayData)->hIcon;
    if (size >= sizeof(NID2KW_32)) {
        nid->is_unicode = true;
        nid->pInfoFlags = &((NID2KW_32*)trayData)->dwInfoFlags;
        nid->pInfoTitle = &((NID2KW_32*)trayData)->szInfoTitle;
        nid->pVersion_Timeout = &((NID2KW_32*)trayData)->uVersion;
        nid->pInfo = &((NID2KW_32*)trayData)->szInfo;
        nid->pState = &((NID2KW_32*)trayData)->dwState;
        nid->pTip = &((NID2KW_32*)trayData)->szTip;
        if (size >= sizeof(NID2KW6_32))
            nid->pGuid = &((NID2KW6_32*)trayData)->guidItem;
    }
    else if (size >= sizeof(NID2K_32)) {
        nid->is_unicode = false;
        nid->pInfoFlags = &((NID2K_32*)trayData)->dwInfoFlags;
        nid->pInfoTitle = &((NID2K_32*)trayData)->szInfoTitle;
        nid->pVersion_Timeout = &((NID2K_32*)trayData)->uVersion;
        nid->pInfo = &((NID2K_32*)trayData)->szInfo;
        nid->pState = &((NID2K_32*)trayData)->dwState;
        nid->pTip = &((NID2K_32*)trayData)->szTip;
    }
    else {
        nid->is_unicode = (size == sizeof(NIDNT_32));
        nid->pTip = &((NIDNT_32*)trayData)->szTip;
    }
#else
    size = ((NIDNT*)trayData)->cbSize;
    nid->hWnd = ((NIDNT*)trayData)->hWnd;
    nid->uID = ((NIDNT*)trayData)->uID;
    nid->uFlags = ((NIDNT*)trayData)->uFlags;
    nid->uCallbackMessage = ((NIDNT*)trayData)->uCallbackMessage;
    nid->hIcon = ((NIDNT*)trayData)->hIcon;
    if (size >= sizeof(NID2KW)) {
        nid->is_unicode = true;
        nid->pInfoFlags = &((NID2KW*)trayData)->dwInfoFlags;
        nid->pInfoTitle = &((NID2KW*)trayData)->szInfoTitle;
        nid->pVersion_Timeout = &((NID2KW*)trayData)->uVersion;
        nid->pInfo = &((NID2KW*)trayData)->szInfo;
        nid->pState = &((NID2KW*)trayData)->dwState;
        nid->pTip = &((NID2KW*)trayData)->szTip;
        if (size >= sizeof(NID2KW6))
            nid->pGuid = &((NID2KW6*)trayData)->guidItem;
    }
    else if (size >= sizeof(NID2K)) {
        nid->is_unicode = false;
        nid->pInfoFlags = &((NID2K*)trayData)->dwInfoFlags;
        nid->pInfoTitle = &((NID2K*)trayData)->szInfoTitle;
        nid->pVersion_Timeout = &((NID2K*)trayData)->uVersion;
        nid->pInfo = &((NID2K*)trayData)->szInfo;
        nid->pState = &((NID2K*)trayData)->dwState;
        nid->pTip = &((NID2K*)trayData)->szTip;
    }
    else {
        nid->is_unicode = (size == sizeof(NIDNT));
        nid->pTip = &((NIDNT*)trayData)->szTip;
    }
#endif

}
#pragma pack(push,4)
typedef struct _SHELLTRAYDATA
{
    DWORD dwMagic; // e.g. 0x34753423;
    DWORD dwMessage;
    NOTIFYICONDATA iconData;
} SHELLTRAYDATA;
#pragma pack(pop)




void TrayService::trayEvent(void* data, unsigned size)
{
    NIDBB nid;
    memset(&nid, 0, sizeof nid);
    DWORD trayCommand = ((SHELLTRAYDATA*)data)->dwMessage;
    void* pData = &((SHELLTRAYDATA*)data)->iconData;

    extractTrayData(&nid, pData);
    std::wstring g = LB_Api::getWindowClassName(nid.hWnd);

    switch (trayCommand) {
    case NIM_ADD:
    {
        if (nid.pState[1] & NIS_HIDDEN)
            return;

        for (int i = 0; i < trayBtnList->m_Items.size(); i++) {
            barItem* item = trayBtnList->m_Items.at(i);
            TrayEntryBtn::TrayItem* ni = (TrayEntryBtn::_TrayItem*)item->m_data;
            if (ni->hWnd == nid.hWnd) {
                FLogger::info("Found a already existing tray icon, trying to be added again..   : %S", ni->sIconText);
                return;

            }
        }


        char tip[200];
        LB_Api::convert_string(tip, nid.pTip, 100, nid.is_unicode);


        TrayEntryBtn::_TrayItem* tray = new TrayEntryBtn::_TrayItem;
        wchar_t* ad = LB_Api::charToWChar(tip);
        wcscpy_s(tray->sIconText, _MAX_PATH, ad);
        delete[] ad;

        if (LB_Api::isIconValid(nid.hIcon) == FALSE)
            break;

        tray->hIcon = CopyIcon(nid.hIcon);
        tray->hWnd = nid.hWnd;
        tray->uCallbackMessage = nid.uCallbackMessage;

        tray->dwState = *nid.pState;


        tray->uID = nid.uID;
        FLogger::info("Adding a tray icon   : %S", tray->sIconText);
        AppendTrayBtn(0, tray->sIconText, tray->hWnd, tray->hIcon, tray);
        trayBtnList->invalidate(true);
    }
    break;
    case NIM_DELETE:
    {
        for (int i = 0; i < trayBtnList->m_Items.size(); i++) {
            barItem* item = trayBtnList->m_Items.at(i);
            TrayEntryBtn::TrayItem* ni = (TrayEntryBtn::_TrayItem*)item->m_data;
            if (ni->hWnd == nid.hWnd) {
                FLogger::debug("Removing a button..   : %S", ni->sIconText);
                trayBtnList->m_Items.erase(trayBtnList->m_Items.begin() + i);
                trayBtnList->invalidate(true);
                return;

            }
        }
    }
    break;
    }
}


LRESULT TrayService::trayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TrayService* pClass = ((TrayService*)GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (uMsg == WM_CREATE) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    }
    if (uMsg == WM_COPYDATA) {
        void* data;
        unsigned size;
        int id;

        data = ((COPYDATASTRUCT*)lParam)->lpData;
        size = ((COPYDATASTRUCT*)lParam)->cbData;
        id = ((COPYDATASTRUCT*)lParam)->dwData;

        if (size >= sizeof(DWORD)
            && ((SHELLTRAYDATA*)data)->dwMagic == 0x34753423) {
            if (id == 1) {
                pClass->trayEvent(data, size);
            }
            if (id == 3) {
                // t
            }
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT TrayService::TrayNotifyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, message, wParam, lParam);
}

