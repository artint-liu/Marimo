#include <Marimo.H>
#include "GrapX/MOConsoleStaff.h"

#include "MOUIStaff.h"
#include "Sample_MOUI.h"
#include "Engine/UniversalDialog.h"

#define SPRITE_FILE_FILTER L"marimo sprite(*.sprite;*.stock)\0*.sprite;*.stock\0all files(*.*)\0*.*\0"

// 这个表与命令表一一对应,因为控制台系统可以返回命令
// 在staff中的索引,使用索引可以更快执行命令
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
  {L"SlideAndLabel",    L"滑动控件演示"},
  {L"TestListBox",      L"ListBox演示"},
  {L"DictList",         L"单词表"},
  {L"NewWordList",      L"生词表"},
  {L"WordSelect",       L"选择单词"},
  {L"WordDiselect",     L"移除生词"},
  {L"browsefiles",      L"打开文件"},
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
    if(m_pMyApp->m_pDlgConsole) {
      m_pMyApp->m_pDlgConsole->Show( ! m_pMyApp->m_pDlgConsole->IsVisible());
    }
    break;

  case CMD_SlideAndLabel:
    if(m_pMyApp->m_pDlgBasic) {
      m_pMyApp->m_pDlgBasic->Show( ! m_pMyApp->m_pDlgBasic->IsVisible());
    }
    break;

  case CMD_TestListBox:
    if(m_pMyApp->m_pDlgList) {
      m_pMyApp->m_pDlgList->Show( ! m_pMyApp->m_pDlgList->IsVisible());
    }
    break;

  case CMD_DictList:
    if(argv[1] == "SelChange") {
      int index = argv[2].ToInteger();
      int selected = argv[3].ToInteger();
      TRACE("SelChange %d %d\n", index, selected);

      if(selected) {
        m_WordSel.insert(index);
      }
      else {
        m_WordSel.erase(index);
      }
    }
    else if(argv[1] == "SelCancel") {
      TRACE("SelCancel %d\n", argv[2].ToInteger());
    }
    break;

  case CMD_WordSelect:
    {
      MOVariable varList1;
      MOVariable varList2;
      m_pMyApp->m_pBasicDataPool->QueryByExpression("List1", &varList1);
      m_pMyApp->m_pBasicDataPool->QueryByExpression("List2", &varList2);

      // 这个要倒过来遍历，否则删除前面的后面会错位
      for(auto it = m_WordSel.rbegin(); it != m_WordSel.rend(); ++it)
      {
        varList2.NewBack().Set(varList1[*it]);
        varList1.Remove(*it);
      }
      m_WordSel.clear();
    }
    break;
  }

  if(argv[0] == L"browsefiles")
  {
    GXOPENFILENAMEW gxofn = {0};
    GXWCHAR szFilename[MAX_PATH] = {0};


    gxofn.lpstrFilter = SPRITE_FILE_FILTER;
    gxofn.lpstrFile = szFilename;
    gxofn.nMaxFile = MAX_PATH;
    gxofn.lpstrTitle = L"Open Sprite File";
    gxofn.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;

    gxGetOpenFileNameW(&gxofn);
  }
  return GX_OK;
}

