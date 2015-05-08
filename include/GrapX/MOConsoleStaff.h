#ifndef _MARIMO_CONSOLE_STAFF_H_
#define _MARIMO_CONSOLE_STAFF_H_

struct STAFFCAPSDESC
{
  GXLPCWSTR szTitle;
  GXLPCWSTR szDesc;
};
typedef STAFFCAPSDESC*        LPSTAFFCAPSDESC;
typedef const STAFFCAPSDESC*  LPCSTAFFCAPSDESC;

class IConsoleStaff : public GUnknown
{
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(GXHRESULT        AddRef      ());
  GXSTDINTERFACE(GXHRESULT        Release     ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(LPCSTAFFCAPSDESC GetCapacity () const); // 获得Command实现列表，Command全局必须唯一
  //GXSTDINTERFACE(GXHRESULT        Execute     (int nCmdIndex, const clStringArray* pCmdList)); // 执行命令nCmdIndex是Command实现列表中的索引，用来快速定位, pCmdList有可能为NULL
  GXSTDINTERFACE(GXHRESULT        Execute     (int nCmdIndex, const clStringW* argv, int argc)); // 执行命令nCmdIndex是Command实现列表中的索引，用来快速定位, pCmdList有可能为NULL
  //GXSTDINTERFACE(const clStringA& GetName     ()); // 当前Staff的名字，Command全局必须唯一
};

class MOKeyboardStaff : public IConsoleStaff
{
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(GXHRESULT        AddRef      ());
  GXSTDINTERFACE(GXHRESULT        Release     ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(LPCSTAFFCAPSDESC GetCapacity () const);
  GXSTDINTERFACE(GXHRESULT        Execute     (int nCmdIndex, const clStringW* argv, int argc));
  GXSTDINTERFACE(GXBOOL           OnKeyDown   (int nVirKey));
  GXSTDINTERFACE(GXBOOL           OnKeyUp     (int nVirKey));
  GXSTDINTERFACE(GXBOOL           SaveToFileW (GXLPCWSTR szFilename));
};

GXHRESULT GXDLLAPI MOCreateKeyboardStaff(MOKeyboardStaff** ppStaff);

#endif // #ifndef _MARIMO_CONSOLE_STAFF_H_