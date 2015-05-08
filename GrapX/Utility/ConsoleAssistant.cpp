// 全局头文件
#include "GrapX.H"
#include "User/GrapX.Hxx"

//#include "GrapX/GUnknown.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"
#include "GrapX/MOLogger.h"
#include "GrapX/MOConsoleStaff.h"

#include "Utility/ConsoleAssistant.h"

STAFFCAPSDESC ConsoleAssistant::s_aStaffDesc[] =
{
  {L"ver", L"show version"},
  {L"help", L"show help"},
  {L"exit", L"Exit process"},
  {L"quit", L"Exit process"},
  {NULL},
};

ConsoleAssistant::ConsoleAssistant()
{
}

ConsoleAssistant::~ConsoleAssistant()
{
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT ConsoleAssistant::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT ConsoleAssistant::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0) {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

LPCSTAFFCAPSDESC ConsoleAssistant::GetCapacity() const
{
  return s_aStaffDesc;
}

GXHRESULT ConsoleAssistant::Execute(int nCmdIndex, const clStringW* argv, int argc)
{
  ASSERT(GXSTRCMP<GXWCHAR>(s_aStaffDesc[nCmdIndex].szTitle, argv[0]) == 0);
  GXLPSTATION lpStation = IntGetStationPtr();
  switch(nCmdIndex)
  {
  case 0: // ver
    lpStation->m_pLogger->OutputA("Marimo Engine v0.0.1\r\n");
    break;
  case 1: // help
    {
      typedef GXSTATION::CmdDict CmdDict;

      int nWidth = 0;
      for(CmdDict::iterator it = lpStation->m_CommandDict.begin();
        it != lpStation->m_CommandDict.end(); ++it) {
          LPCSTAFFCAPSDESC pCapsDesc = it->second.pStaff->GetCapacity();
          nWidth = clMax(GXSTRLEN<GXCHAR>(it->first), nWidth);
      }

      for(CmdDict::iterator it = lpStation->m_CommandDict.begin();
        it != lpStation->m_CommandDict.end(); ++it) {
          LPCSTAFFCAPSDESC pCapsDesc = it->second.pStaff->GetCapacity();
          lpStation->m_pLogger->OutputFormatW(L"%*s %s\r\n", -nWidth, clStringW(it->first), pCapsDesc[it->second.nIndex].szDesc);
      }
    }
    break;
  case 2:
  case 3:
    gxPostQuitMessage(0);
    break;
  }
  return GX_OK;
}
