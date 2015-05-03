#ifndef _LOG_MANAGER_H_
#define _LOG_MANAGER_H_

namespace LogMgr
{
  enum LogChannel
  {
    LC_D3D = 0,
    LC_GrapX,
    LC_Max,
  };
  bool Initialize();
  void Release();
  int Output(LogChannel lc, char *fmt, ...);
};

#endif // _LOG_MANAGER_H_