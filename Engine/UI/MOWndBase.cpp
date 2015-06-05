#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"

CMOWnd::CMOWnd() 
  : m_hWnd      (NULL)
{
}

CMOWnd::CMOWnd(GXHWND hWnd) 
  : m_hWnd      (hWnd)
{
}

CMOWnd::CMOWnd(GXHINSTANCE hInstance, CMOWnd* pParentWnd)
  : m_hWnd      (NULL)
{
  CLBREAK; // 可能要去掉这个构造
}

CMOWnd::CMOWnd(CMOWnd* pWnd)
  : m_hWnd(pWnd == NULL ? NULL : pWnd->m_hWnd)
{
}

CMOWnd::~CMOWnd()
{
}

GXBOOL CMOWnd::Enable(GXBOOL bEnable)
{
  return gxEnableWindow(m_hWnd, bEnable);
}
GXBOOL CMOWnd::IsVisible()
{
  return gxIsWindowVisible(m_hWnd);
}

GXBOOL CMOWnd::Show(bool bShow)
{
  if(m_hWnd) {
    return gxShowWindow(m_hWnd, bShow == TRUE ? SW_SHOWNORMAL : SW_HIDE);
  }
  return FALSE;
}

GXBOOL CMOWnd::Show(int nCmdShow)
{
  return gxShowWindow(m_hWnd, nCmdShow);
}

//GXBOOL CMOWnd::SetText(GXLPCTSTR szString)
//{
//#ifdef _UNICODE
//  return SetTextW(szString);
//#else
//  return SetTextA(szString);
//#endif // #ifdef _UNICODE
//}

clStringW CMOWnd::GetTextW()
{
  clStringW str;
  int nLength = gxGetWindowTextLengthW(m_hWnd) + 1;
  gxGetWindowTextW(m_hWnd, str.GetBuffer(nLength), nLength);
  str.ReleaseBuffer();
  return str;
}

clStringA CMOWnd::GetTextA()
{
  clStringW str = GetTextW();
  return clStringA(str);
}

GXBOOL CMOWnd::SetTextW(GXLPCWSTR szString)
{
  return gxSetWindowTextW(m_hWnd, szString);
}

GXBOOL CMOWnd::SetTextA(GXLPCSTR szString)
{
  clStringW strText = szString;
  return gxSetWindowTextW(m_hWnd, strText);
}

int CMOWnd::GetTextLengthW()
{
  return gxGetWindowTextLengthW(m_hWnd);
}

GXLONG_PTR CMOWnd::SetLong(int index, GXLONG_PTR dwLong)
{
  if(index == GXGWL_COBJECT) {
    return 0;
  }
  return (GXLONG_PTR)gxSetWindowLongW(m_hWnd, index, (GXLONG_PTR)dwLong);
}

BOOL CMOWnd::GetClientRect(LPGXRECT lpRect)
{
  return gxGetClientRect(m_hWnd, lpRect);
}

UINT CMOWnd::SetTimer(UINT uID, UINT uElapse)
{
  return gxSetTimer(m_hWnd, uID, uElapse, NULL);
}

BOOL CMOWnd::KillTimer(UINT uID)
{
  return gxKillTimer(m_hWnd, uID);
}

GXLONG_PTR CMOWnd::GetLong(int index)
{
  return (GXLONG_PTR)gxGetWindowLongW(m_hWnd, index);
}

GXLRESULT CMOWndProcedure::OnSize(GXDWORD fwSizeType, GXSIZE& size)
{
  return 1L;
}

GXLRESULT CMOWndProcedure::OnPaint(GXPAINTSTRUCT* pps)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnDisplayChange(GXUINT uColorDepth,GXSIZE sizeScreen)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnCommand(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnCreate(GXLPCREATESTRUCT lpCreateParam)
{
  return 0;
}

GXBOOL CMOWndProcedure::OnNCCreate(GXLPCREATESTRUCT lpCreateParam)
{
  return TRUE;
}

GXLRESULT CMOWndProcedure::OnDestory()
{
  return 0L;
}

GXBOOL CMOWndProcedure::OnSetCursor(GXHWND hWnd, int nHittest, GXDWORD wMouseMsg)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnNotify(NMHDR* pnmhdr)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnTimer(UINT uID)
{
  return 1L;
}

GXLRESULT CMOWndProcedure::OnShow(GXBOOL bShow, int nStatus)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnChar(GXWCHAR chCharCode, GXLPARAM lKeyData)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnKeyDown(int nVirtKey, GXLPARAM lKeyData)
{
  return 0L;
}

GXLRESULT CMOWndProcedure::OnKeyUp(int nVirtKey, GXLPARAM lKeyData)
{
  return 0L;
}

GXVOID CMOWndProcedure::OnMouseMove(WPARAM fwKeys, POINT* ptMouse)
{  
}

GXBOOL CMOWndProcedure::OnSetCursor(HWND hWnd, int nHittest, GXWORD wMouseMsg)
{
  return FALSE;
}

GXLRESULT CMOWndProcedure::OnWindowPosChanging(GXWINDOWPOS* pWndPos)
{
  return -1;
}

CMOWndProcedure::CMOWndProcedure()
{
}

CMOWndProcedure::CMOWndProcedure( GXHWND hWnd )
  : CMOWnd(hWnd)
{
}

CMOWndProcedure::~CMOWndProcedure()
{
  ASSERT(m_hWnd == NULL || gxIsWindow(m_hWnd));
  if(m_hWnd)
  {
    CLOG_WARNING("Calling DestroyWindow() in "__FUNCTION__".\n");
    gxDestroyWindow(m_hWnd);
  }
}

//GXLRESULT CMOWnd::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
//{
//  return -1;
//}
//
//GXLRESULT CMOWnd::Invoke(CGXReceiver* pReceiver, GXNMHDR* pnmhdr)
//{
//  return 0L;
//}

GXBOOL CMOWnd::SetWindowPos(GXHWND hWndInsertAfter,int X,int Y,int cx,int cy,UINT uFlags)
{
  return gxSetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

GXBOOL CMOWnd::GetWindowRect(GXRECT* rect)
{
  return gxGetWindowRect(m_hWnd, rect);
}

GXBOOL CMOWnd::UpdateWindow()
{
  return gxUpdateWindow(m_hWnd);
}

GXBOOL CMOWnd::InvalidateRect(CONST GXRECT *lpRect, BOOL bErase)
{
  return gxInvalidateRect(m_hWnd, lpRect, bErase);
}

GXHWND CMOWnd::SetCapture()
{
  return gxSetCapture(Handle());
}

GXHWND CMOWnd::SetFocus()
{
  return gxSetFocus(m_hWnd);
}

GXHWND CMOWnd::SetParent(GXHWND hParent)
{
  return gxSetParent(m_hWnd, hParent);
}

GXHWND CMOWnd::SetParent(CMOWnd* pParent)
{
  return gxSetParent(m_hWnd, pParent->m_hWnd);
}

int CMOWnd::MessageBoxW(GXLPCWSTR lpText, GXLPCWSTR lpCaption, GXUINT uType)
{
  return gxMessageBoxW(m_hWnd, lpText, lpCaption, uType);
}
//////////////////////////////////////////////////////////////////////////
//CMOWnd& CMOWnd::operator=(CMOWnd* pWnd)
//{
//  return this->operator=(*pWnd);
//}
//
//CMOWnd& CMOWnd::operator=(CMOWnd& Wnd)
//{
//  ASSERT(GetClassNameCode() == Wnd.GetClassNameCode() /*&&
//    GetThisSizeOf() == Wnd.GetThisSizeOf()*/);
//
//  const size_t c_nVPtrSize = sizeof(void*);
//  memcpy((GXBYTE*)this + c_nVPtrSize, (GXBYTE*)&Wnd + c_nVPtrSize, GetThisSizeOf() - c_nVPtrSize);
//
//  //m_hInstance = Wnd.m_hInstance;
//  //m_hWnd = Wnd.m_hWnd;
//  //m_pParentWnd = Wnd.m_pParentWnd;
//
//  return *this;
//}

clStringW CMOWnd::GetClassName() const
{
  const int c_nNameLen = 64;
  ASSERT(m_hWnd != NULL);
  GXWCHAR szClassName[c_nNameLen] = {L'\0'};
  gxGetClassName(m_hWnd, szClassName, c_nNameLen);
  clStringW ClsName(szClassName);
  ClsName.MakeUpper();
  return ClsName;
}

//GXSIZE_T CMOWnd::GetThisSizeOf() const
//{
//  return sizeof(CMOWnd);
//}

GXDWORD CMOWnd::GetClassNameCode() const
{
  return GetClassName().GetHash();
}

GXBOOL CMOWnd::Destroy()
{
  if( ! this) {
    return -1;
  }
  GXBOOL result = gxDestroyWindow(m_hWnd);
  m_hWnd = NULL;
  return result;
}
