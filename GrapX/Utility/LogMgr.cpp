#ifndef _DEV_DISABLE_UI_CODE
#if defined(_WIN32) || defined(_WINDOWS)
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "LogMgr.H"

static HANDLE ahLog[LogMgr::LC_Max];
static char* aLogName[] = {
  "Log\\D3dInfo.log",
  "Log\\grapx.log",
};

bool LogMgr::Initialize()
{
  CreateDirectoryW(L"Log", NULL);
  for(int i = 0; i < LC_Max; i++) {
    ahLog[i] = CreateFileA(aLogName[i], GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE  , NULL);
  }
    //fopen_s(&afpLog[i], aLogName[i], "wt");
  return true;
}

void LogMgr::Release()
{
  for(int i = 0; i < LC_Max; i++)
  {
    Output((LogChannel)i, "日志文件结尾！\n");
    CloseHandle(ahLog[i]);
  }
    //fclose(afpLog[i]);
}

int LogMgr::Output(LogChannel lc, char *fmt, ...)
{
  char buffer[1024];
  // TODO: 改为clString
  va_list val;
  va_start(val, fmt);
  int Result = vsnprintf_s ( buffer, sizeof(buffer), _TRUNCATE, fmt, val);
  va_end(val);

  DWORD dwNumReadWrite;
  WriteFile(ahLog[lc], buffer, Result, &dwNumReadWrite, NULL);
  return Result;
}
#endif // #if defined(_WIN32) || defined(_WINDOWS)
#endif // _DEV_DISABLE_UI_CODE