#include <Marimo.H>
#include "GrapX/MOConsoleStaff.h"

#include "MOUIStaff.h"
#include "Sample_MOUI.h"
#include "Engine/UniversalDialog.h"

// ������������һһ��Ӧ,��Ϊ����̨ϵͳ���Է�������
// ��staff�е�����,ʹ���������Ը���ִ������
enum CMDLIST
{
  CMD_console,
  CMD_SlideAndLabel,
  CMD_TestListBox,
  CMD_DictList,
  CMD_NewWordList,
  CMD_WordSelect,
  CMD_WordDiselect,
};

static STAFFCAPSDESC s_aStaffDesc[] = {
  {L"console",          L"toggle console"},
  {L"SlideAndLabel",    L"�����ؼ���ʾ"},
  {L"TestListBox",      L"ListBox��ʾ"},
  {L"DictList",         L"���ʱ�"},
  {L"NewWordList",      L"���ʱ�"},
  {L"WordSelect",       L"ѡ�񵥴�"},
  {L"WordDiselect",     L"�Ƴ�����"},
  {NULL}
};

MOUIStaff::MOUIStaff(SampleApp_MOUI* pApp)
  : m_pMyApp(pApp)
{
}

MOUIStaff::~MOUIStaff()
{
}

GXHRESULT MOUIStaff::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT MOUIStaff::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0) {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}

LPCSTAFFCAPSDESC MOUIStaff::GetCapacity() const
{
  return s_aStaffDesc;
}

GXHRESULT MOUIStaff::Execute(int nCmdIndex, const clStringW* argv, int argc)
{
  switch(nCmdIndex)
  {
  case CMD_console:
    m_pMyApp->m_pDlgConsole->Show( ! m_pMyApp->m_pDlgConsole->IsVisible());
    break;

  case CMD_SlideAndLabel:
    m_pMyApp->m_pDlgBasic->Show( ! m_pMyApp->m_pDlgBasic->IsVisible());
    break;

  case CMD_TestListBox:
    m_pMyApp->m_pDlgList->Show( ! m_pMyApp->m_pDlgList->IsVisible());
    break;

  case CMD_DictList:
    if(argv[1] == "SelChange") {
      TRACE("SelChange %d\n", argv[2].ToInteger());
    }
    else if(argv[1] == "SelCancel") {
      TRACE("SelCancel %d\n", argv[2].ToInteger());
    }
    break;
  }
  return GX_OK;
}

