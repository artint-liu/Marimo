//#include <Marimo.H>
#include <GrapX.H>
//#include "GrapX/GUnknown.H"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOButton.h"
#include "Engine/MOEdit.H"
#include "GrapX/MOLogger.h"
#include "GrapX/GXKernel.H"

#include "Engine/DlgConsole.h"
//#include "grapx/Console.h"

LPCWSTR CDlgConsole::idTemplate = {L"file://ui/console.dlg.txt"};
GXLONG_PTR CDlgConsole::s_OldWndProc;
GXVOID CDlgConsole::OnInitDialog()
{
  GetDlgItem(L"Output", m_Output);
  GetDlgItem(L"Input", m_Input);
  OutputW(L"Begin of log.\n");
  OutputA("Begin of log(ANSI).\n");

  s_OldWndProc = m_Input.GetLong(GXGWL_WNDPROC);
  m_Input.SetLong(GXGWL_WNDPROC, (GXLONG_PTR)EditWndProc);
  m_Input.SetLong(GXGWL_USERDATA, (GXLONG_PTR)this);
}

GXLRESULT CDlgConsole::OnDestory()
{
  return CMODialog::OnDestory();
}

void CDlgConsole::OutputW(GXLPCWSTR szString)
{
  m_Output.SetSelect(-2, -1);
  m_Output.ReplaceSelectW(szString, FALSE);
}

void CDlgConsole::OutputA(GXLPCSTR szString)
{
  const GXUINT nLen = GXSTRLEN(szString);

  // 如果添加的字符串超过控件限制，则在开头清除两倍字符串长度的文本
  if(m_Output.GetTextLengthW() + nLen > m_Output.GetLimitText()) {
    m_Output.SetSelect(0, nLen * 2);
    m_Output.ReplaceSelectA("", FALSE);
  }

  m_Output.SetSelect(-2, -1);
  m_Output.ReplaceSelectA(szString, FALSE);
}

void CDlgConsole::AttachLogger( IStreamLogger* pLogger )
{
  pLogger->SetStreamProc(gxGetCurrentThreadId(), LogStreamProc, (GXLPARAM)this);
}

GXINT GXCALLBACK CDlgConsole::LogStreamProc( GXLPVOID szText, GXBOOL bUnicode, GXLPARAM lParam )
{
  CDlgConsole* pConsole = (CDlgConsole*)lParam;

  if(bUnicode) {
    pConsole->OutputW((GXLPCWSTR)szText);
  }
  else {
    pConsole->OutputA((GXLPCSTR)szText);
  }
  return 0;
}

GXVOID CDlgConsole::OnBtnClicked(CMOButton* pSender)
{
  clStringW str = m_Input.GetTextW();
  str.TrimLeft(' ');
  str.TrimRight(' ');
  if(str.IsEmpty())
  {
    MOLogW(L"]\r\n");
    return;
  }

  if(m_listCmd.empty() || str != m_listCmd.back())
  {
    m_listCmd.push_back(str);
    if(m_listCmd.size() > s_nMaxCmdCache) {
      m_listCmd.erase(m_listCmd.begin());
    }
  }

  MOLogW(L"]%s\r\n", str);
  MOExecuteConsoleCmdW(str);

  m_Input.SetTextW(L"");
}

GXLRESULT GXCALLBACK CDlgConsole::EditWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  if(message == GXWM_CHAR)
  {
    if(wParam == GXVK_RETURN)
    {
      CDlgConsole* pDlg = (CDlgConsole*)gxGetWindowLong(hWnd, GXGWL_USERDATA);
      if(pDlg)
      {
        pDlg->OnBtnClicked(NULL);
      }
    }
    else if(wParam == '`') // TODO: 这里目前屏蔽输入控制台符号，以后应该在GXApp中增加一个接口用来处理未获得UI焦点时的键盘消息
    {
      return 0L;
    }
  }
  else if(message == GXWM_KEYDOWN && wParam == GXVK_UP){
    CDlgConsole* pDlg = (CDlgConsole*)gxGetWindowLong(hWnd, GXGWL_USERDATA);
    pDlg->ScrollToPreviousCommand();
    return 0;
  }
  return gxCallWindowProc((GXWNDPROC)s_OldWndProc, hWnd, message, wParam, lParam);
}

void CDlgConsole::ScrollToPreviousCommand()
{
  if(m_listCmd.empty()) {
    return;
  }
  clStringW str = m_listCmd.back();
  m_Input.SetTextW(str);
  m_Input.SetSelect(str.GetLength(), str.GetLength() + 1);
  m_listCmd.pop_back();
  m_listCmd.push_front(str);
}

GXLRESULT CDlgConsole::OnShow(GXBOOL bShow, int nStatus)
{
  m_Input.SetFocus();
  return 0;
}

extern "C"
{
  GAMEENGINE_API CMODialog* MOCreateConsoleDlg(IStreamLogger* pLogger)
  {
    CDlgConsole* pDlgConsole = NULL;
    pDlgConsole = new CDlgConsole();
    if(pDlgConsole->CreateDlg()) {
      pDlgConsole->AttachLogger(pLogger);
      pDlgConsole->Show(FALSE);
    }

    return pDlgConsole;
  }
} // extern "C"