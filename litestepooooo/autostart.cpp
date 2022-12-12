#include "autostart.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define APP_NAME L"mmm"

void setAutostart(int on, int elevate){
    // Open key
    HKEY key;
    int error = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL);
    if (error != ERROR_SUCCESS) {
        MessageBoxA(NULL , ("RegCreateKeyEx(HKEY_CURRENT_USER,'Software\\Microsoft\\Windows\\CurrentVersion\\Run') Error opening reg" + error), "error" , MB_OK);
        return;
    }
    if (on) {
        // Get path
        wchar_t path[MAX_PATH], value[MAX_PATH + 20];
        GetModuleFileName(NULL, path, ARRAY_SIZE(path));
        swprintf(value, ARRAY_SIZE(value), L"\"%s\"%s", path, (elevate ? L" -elevate" : L""));
        // Set autostart
        error = RegSetValueEx(key, APP_NAME, 0, REG_SZ, (LPBYTE)value, (wcslen(value) + 1) * sizeof(value[0]));
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL,"RegSetValueEx('APP_NAME'), LSetAutostart()" + error, "ERROR", MB_OK);

            return;
        }
    }
    else {
        // Remove
        error = RegDeleteValue(key, APP_NAME);
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL,"RegSetValueEx('APP_NAME'), LSetAutostart()" + error, "ERROR", MB_OK);
            return;
        }
    }
    // Close key
    RegCloseKey(key);
}