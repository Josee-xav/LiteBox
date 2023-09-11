#include "FLogger.h"
#include <Windows.h>
FILE* FLogger::logFile;

FLogger::FLogger()
{
    fopen_s(&logFile, LOG_FILE_NAME, "w") ;
}

FLogger::~FLogger()
{
    fclose(logFile);
}


