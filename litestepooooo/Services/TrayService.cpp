#include "TrayService.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>

HWND findTrayToolbarWindow()
{
    HWND hWnd = NULL;

    if ((hWnd = FindWindow(("Shell_TrayWnd") , NULL)) != NULL) {
        if ((hWnd = FindWindowEx(hWnd , 0 , ("TrayNotifyWnd") , NULL)) != NULL) {
            if (hWnd = FindWindowEx(hWnd , 0 , ("SysPager") , NULL)) {
                hWnd = FindWindowEx(hWnd , 0 , ("ToolbarWindow32") , NULL);
            }
        }
    }

    return hWnd;
}

TrayService::TrayService()
{
    initTrayService();
}

TrayService::~TrayService()
{
}

trayItemList* TrayService::getTrayList()
{
    return trayBtnList;
}

void TrayService::initTrayService()
{
    trayBtnList = new trayItemList();
    std::vector< TrayEntryBtn::_TrayItem*> trayItems = getTrayItems();

    for (int i = 0; i < trayItems.size(); i++) {
        AppendTrayBtn(0 , trayItems.at(i)->sIconText , 1 , trayItems.at(i)->hWnd , trayItems.at(i)->hIcon , trayItems.at(i));
    }
}

std::vector<TrayEntryBtn::_TrayItem* > TrayService::getTrayItems()
{
    std::vector< TrayEntryBtn::_TrayItem*> trayItems;
    HWND toolBarhWnd = findTrayToolbarWindow();

    if (toolBarhWnd == NULL) {
        return trayItems;
    }

    int nBtnCount = ::SendMessage(toolBarhWnd , TB_BUTTONCOUNT , 0 , 0);
    if (nBtnCount < 1) {
        return trayItems;
    }

    DWORD dwPID = 0;
    GetWindowThreadProcessId(toolBarhWnd , &dwPID);
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE , false , (int)dwPID);
    if (!hProcess) {
        return trayItems;
    }
    //Apply for a piece of memory in the target process, put TBBUTTON information
    LPVOID pvAddress = VirtualAllocEx(hProcess , 0 , sizeof(TBBUTTON) , MEM_COMMIT , PAGE_READWRITE);
    for (int i = 0; i < nBtnCount; ++i) {
        TrayEntryBtn::_TrayItem* tray = new TrayEntryBtn::_TrayItem();
        SIZE_T nNumberOfBytesRead = 0;

        // Use ReadProcessMemory to read data from the process memory space: icon title, icon area
        ::SendMessage(toolBarhWnd , TB_GETBUTTON , i , (long)pvAddress);

        TBBUTTON bi = {};
        ReadProcessMemory(hProcess , (LPVOID)pvAddress , &bi , sizeof(TBBUTTON) , &nNumberOfBytesRead);
        ReadProcessMemory(hProcess , (LPCVOID)bi.dwData , (LPVOID)tray , sizeof(TrayEntryBtn::_TrayItem) , NULL);

        TCHAR szBtnText[_MAX_PATH] = { 0 };
        SendMessage(toolBarhWnd , TB_GETBUTTONTEXT , bi.idCommand , (LPARAM)(pvAddress));
        ReadProcessMemory(hProcess , (void*)(pvAddress) , szBtnText , _MAX_PATH * sizeof(TCHAR) , &nNumberOfBytesRead);

        //ReadProcessMemory(hProcess, (void*)(pvAddress), icon, _MAX_PATH * sizeof(HICON), &nNumberOfBytesRead);

        strcpy_s(tray->sIconText , _MAX_PATH , szBtnText);
        HICON hico = CopyIcon(tray->hIcon);
        tray->hIcon = hico;

        trayItems.push_back(tray);
    }
    VirtualFreeEx(hProcess , pvAddress , 0 , MEM_RELEASE);
    CloseHandle(hProcess);

    return trayItems;
}

bool TrayService::AppendTrayBtn(const DWORD dwFlags , LPCTSTR pszName , const UINT itemid , HWND appHwnd , HICON icon , TrayEntryBtn::_TrayItem* trayItem)
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
