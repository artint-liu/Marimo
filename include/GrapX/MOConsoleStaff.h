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
  GXSTDINTERFACE(LPCSTAFFCAPSDESC GetCapacity () const); // ���Commandʵ���б�Commandȫ�ֱ���Ψһ
  //GXSTDINTERFACE(GXHRESULT        Execute     (int nCmdIndex, const clStringArray* pCmdList)); // ִ������nCmdIndex��Commandʵ���б��е��������������ٶ�λ, pCmdList�п���ΪNULL
  GXSTDINTERFACE(GXHRESULT        Execute     (int nCmdIndex, const clStringW* argv, int argc)); // ִ������nCmdIndex��Commandʵ���б��е��������������ٶ�λ, pCmdList�п���ΪNULL
  //GXSTDINTERFACE(const clStringA& GetName     ()); // ��ǰStaff�����֣�Commandȫ�ֱ���Ψһ
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