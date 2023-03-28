#include "FLogger.h"

#include "../Utils.h"

FLogger::FLogger() : file(nullptr) {
    wchar_t buf[255];
    swprintf(buf, 255,L"%s\\%s", LB_Api::getLBExePath().c_str(), LOG_FILE_NAME);
    errno_t err = _wfopen_s(&file, buf, L"a"); // i know.. opening and closing files is a fairly costy operation. :/
    if (file == nullptr || err != 0)
    {
#ifdef _DEBUG
        MessageBoxA(NULL, "FLOGGER FILE FAILED", ".", S_OK);
#endif // DEBUG


    }
}

FLogger::~FLogger() {
    if (file != nullptr) {
        std::fclose(file);
        file = nullptr;
    }
}