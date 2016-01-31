#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXImage.H>

// 私有头文件
#include <User/GXWindow.h>
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>

#include "thread/clMessageThread.h"
#include "User/gxMessage.hxx"

GXHICON GXDLLAPI GXCursorToIcon(GXHCURSOR hCursor);
//GXHWND GXGetParentClip(HGXWND hWnd, GXBOOL bClient, GXLPRECT lprcOut);

//GXDWORD            GXWnd::s_emCursorResult    = NULL;
GXSprite*    GXWnd::s_pCommonSpr      = NULL;

GXBOOL GXPaintVScrollBar(GXWndCanvas* pCanvas, LPGXSCROLLBAR lpScrollBar, GXRECT* lpRect);
GXBOOL GXPaintHScrollBar(GXWndCanvas* pCanvas, LPGXSCROLLBAR lpScrollBar, GXRECT* lpRect);
GXUINT MENU_GetMenuBarHeightFast( GXHWND hWnd, GXUINT menubarWidth,  GXINT orgX, GXINT orgY );
//GXVOID          *  GXWnd::s_HotKeyChain;
//GXLONG            GXWnd::s_uFrameCount = 0;
//#ifdef _ENABLE_STMT
//GXMSG            GXWnd::s_msg[64];
//GXUINT            GXWnd::s_idxMsgStackTop = 0;
//GXUINT            GXWnd::s_idxMsgStackBottom = 0;
//#endif // #ifdef _ENABLE_STMT
GXHICON            GXWnd::s_hCursorArrow;


#ifdef ENABLE_DYNMAIC_EFFECT
GXPOINT              GXWnd::s_ptPrevMousePos;
#endif

GXLRESULT    DEFWNDPROC_SetText        (GXHWND hWnd, GXLPCWSTR pszName);

GXWnd::GXWnd()
: m_hSelf       (NULL)
, m_pParent     (NULL)
, m_pFirstChild (NULL)
, m_pPrevWnd    (NULL)
, m_pNextWnd    (NULL)

, m_uStyle      (NULL)
, m_uExStyle    (NULL)
, m_uState      (NULL)
, m_pText       (NULL)
, m_hInstance   (NULL)
, m_lpClsAtom   (NULL)

, m_lpWndProc   (NULL)
, m_pMenu       (NULL)
, m_lpVScrollBar(NULL)
, m_lpHScrollBar(NULL)

, hSysMenu      (NULL)
, m_dwUserData  (NULL)
, m_hTheme      (NULL)
, m_CObj        (NULL)
, m_pResponder  (NULL)
, m_pWinsSurface(NULL)
, m_prgnUpdate  (NULL)
{
  gxSetRectEmpty(&rectWindow);
}

GXWnd::~GXWnd()
{
  if(m_lpClsAtom != NULL)
  {
    GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
    if(lpStation != NULL)
      lpStation->m_pDesktopWindowsMgr->ManageWindowSurface(m_hSelf, GXWM_DESTROY);
  }

  SAFE_RELEASE(m_pWinsSurface);
  SAFE_RELEASE(m_prgnUpdate);

  if(!IS_IDENTIFY(m_pText))
    SAFE_DELETE_ARRAY(m_pText);
  SAFE_DELETE(m_lpHScrollBar);
  SAFE_DELETE(m_lpVScrollBar);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GXHWND GXWnd::SetParent(GXHWND hParent)
{
  LPGXWND pParent = GXWND_PTR(hParent);
  LPGXWND pDesktop = GetDesktop();
  GXHWND hPrevParent = GXWND_HANDLE(m_pParent);
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  DesktopWindowsMgr* pDWM = lpStation->m_pDesktopWindowsMgr;

  if(pParent == NULL)
    pParent = pDesktop;

  // 如果结构内 m_pParent 为空, 则是处于 CreateWindow 的过程中
  // 否则其他情况 m_pParent 都不应该为空
  if(pParent == m_pParent) {
    return NULL;
  }

  //if(m_pParent != NULL) // CreateWindow 调用中不执行这个
  //{
  //  pParent->MapWindowPoints(m_pParent, (GXLPPOINT)&rectWindow, 2);
  //}

  GXUINT message = GXWM_NULL;

  // 断开原有的父对象
  if(m_pParent != NULL)
  {
    if(m_pParent == pDesktop)
    {
      message = GXWM_DESTROY;
      //pDWM->ManageWindowSurface(m_hSelf, GXWM_DESTROY);
      lpStation->CleanupActiveWnd(this);
    }
    if(m_pPrevWnd != NULL)
    {
      m_pPrevWnd->m_pNextWnd = m_pNextWnd;
    }
    else
    {
      ASSERT(m_pParent->m_pFirstChild == this);
      m_pParent->m_pFirstChild = m_pNextWnd;
    }
    if(m_pNextWnd != NULL)
    {
      m_pNextWnd->m_pPrevWnd = m_pPrevWnd;
    }
    m_pNextWnd = NULL;
    m_pPrevWnd = NULL;
    m_pParent = NULL;
  }

  // 设置结构中的父对象
  m_pParent = pParent;

  // 得到父对象的第一个子对象
  GXWnd *pChild = pParent->m_pFirstChild;

  // 对父对象没有子对象的处理
  if(pChild == NULL)
  {
    pParent->m_pFirstChild = this;
    if(pParent == pDesktop && m_pWinsSurface == NULL)
    {
      message = GXWM_CREATE;
      //pDWM->ManageWindowSurface(m_hSelf, GXWM_CREATE);
    }
    else if(pParent != pDesktop && m_pWinsSurface != NULL)
    {
      message = GXWM_DESTROY;
      //pDWM->ManageWindowSurface(m_hSelf, GXWM_DESTROY);
    }
    //return hPrevParent;
    goto MANAGE_SURFACE;
  }
  {
  // 父对象如果有子对象, 则插入到链的最后, 
  // 如果 TopLevel 窗口不具有 TopMost 属性, 则插到已有的 TopMost 窗口之前
  GXLPWND pInsert = NULL;
  if(pParent == pDesktop)
  {
    message = GXWM_CREATE;
    //pDWM->ManageWindowSurface(m_hSelf, GXWM_CREATE);

    if(TEST_FLAG(m_uExStyle, GXWS_EX_TOPMOST))
      pInsert = pChild->GetLastSibling();
    else
      pInsert = pChild->GetLastSiblingNoTopMost();
  }
  else
  {
    pInsert = pChild->GetLastSibling();
    ASSERT(pInsert != NULL);
    ASSERT(pInsert->m_pNextWnd == NULL);
  }

  if(pInsert->m_pNextWnd != NULL)
  {
    pInsert->m_pNextWnd->m_pPrevWnd = this;
    m_pNextWnd = pInsert->m_pNextWnd;
  }

  pInsert->m_pNextWnd = this;
  m_pPrevWnd = pInsert;
}
  //if(GetTopSurface() != NULL) // CreateWindow 时无法得到有效的 Surface
  //{
    //GXInvalidateWindow(m_hSelf, NULL, FALSE);  // 测试
  //}
MANAGE_SURFACE:
  pDWM->ManageWindowSurface(m_hSelf, message);
  if(message == GXWM_CREATE)
  {
    GRegion* prgnWindow;
    GetWindowRegion(&prgnWindow);
    pDWM->InvalidateWndRegion(NULL, prgnWindow, FALSE);
    SAFE_RELEASE(prgnWindow);
  }
  else if(message == GXWM_DESTROY)
  {
    GXInvalidateWindowRect(m_hSelf, NULL, FALSE);
  }
  return hPrevParent;
}
//////////////////////////////////////////////////////////////////////////
GXHRESULT GXWnd::CreateDesktop(GXLPSTATION lpStation)
{
  GXWnd* lpWnd = lpStation->lpDesktopWnd;

  lpWnd->rectWindow.left   = 0;
  lpWnd->rectWindow.top    = 0;
  lpWnd->rectWindow.right  = g_SystemMetrics[GXSM_CXSCREEN];
  lpWnd->rectWindow.bottom = g_SystemMetrics[GXSM_CYSCREEN];
  ASSERT(lpWnd->rectWindow.right  == lpStation->nWidth);
  ASSERT(lpWnd->rectWindow.bottom == lpStation->nHeight);
  lpWnd->m_lpWndProc = (GXWNDPROC)gxDefWindowProcW;

  InitializeSysCursor();
  return _gxInitializeCommonSprite(lpStation->pGraphics);
}

GXHRESULT GXWnd::DestroyDesktop(GXLPSTATION lpStation)
{
  ReleaseSysCursor();
  return _gxReleaseCommonSprite();
}
GXHRESULT GXWnd::InitializeSysCursor()
{
#if defined(_WIN32) || defined(_WINDOWS)
  //ICONINFO IconInfo;
  //GetIconInfo((HICON)IDC_ARROW, &IconInfo);
  s_hCursorArrow  = GXCursorToIcon(gxLoadCursorW(NULL, (GXLPCWSTR)IDC_ARROW));
  //s_hCursorNESW  = GXCursorToIcon(LoadCursorW(NULL, IDC_SIZENESW));
  //s_hCursorNS    = GXCursorToIcon(LoadCursorW(NULL, IDC_SIZENS));
  //s_hCursorNWSE  = GXCursorToIcon(LoadCursorW(NULL, IDC_SIZENWSE));
  //s_hCursorWE    = GXCursorToIcon(LoadCursorW(NULL, IDC_SIZEWE));
#endif // defined(_WIN32) || defined(_WINDOWS)
  return TRUE;
}

GXVOID GXWnd::ReleaseSysCursor()
{
  gxDestroyIcon(s_hCursorArrow);
  //gxDestroyIcon(s_hCursorNESW);
  //gxDestroyIcon(s_hCursorNS);
  //gxDestroyIcon(s_hCursorNWSE);
  //gxDestroyIcon(s_hCursorWE);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GXBOOL GXWnd::AnalyzeMouseMoveMsg(GXINOUT GXMSG* msg, GXLPPOINT pptMSWinClient)
{
  GXWPARAM   fwKeys = (msg == 0) ? 0 : msg->wParam;
  GXLPWND    pNowFocus = NULL;
  GXPOINT    ptClient;

  GXCLPSTATION  lpStation = IntGetStationPtr();
  GXHWND        hMouseFocus = GXWND_HANDLE(lpStation->m_pMouseFocus);
  GXBOOL        bEnabled;
  GXLRESULT     ht;

  if(pptMSWinClient == NULL) {
    pptMSWinClient = &lpStation->m_ptCursor;
  }

  if(lpStation->lpDesktopWnd->m_pFirstChild != NULL) {
    pNowFocus = lpStation->lpDesktopWnd->ChildWindowFromPoint(pptMSWinClient, &ht, &bEnabled);
  }

  ptClient = *pptMSWinClient;

  if(pNowFocus == NULL) {
    IntSetCursor(NULL, GXMAKELPARAM(GXHTNOWHERE, NULL));
  }

  // 如果鼠标所在的焦点窗口改变了
  // Disabled 窗口能够正常接收盘旋和离开消息
  if(pNowFocus != lpStation->m_pMouseFocus)
  {
    GXHWND hNowFocus = GXWND_HANDLE(pNowFocus);
    if(hMouseFocus != NULL)
    {
      gxSendMessageW(hMouseFocus, GXWM_NCMOUSELEAVE, NULL, NULL);
      gxSendMessageW(hMouseFocus, GXWM_MOUSELEAVE, NULL, NULL);

      //lpStation->m_pMouseFocus = pNowFocus;
      if(hNowFocus != NULL)
      {
        gxSendMessageW(hNowFocus, GXWM_NCMOUSEHOVER, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
        gxSendMessageW(hNowFocus, GXWM_MOUSEHOVER, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
      }
    }
    else
    {
      // 如果 hMouseFocus == NULL
      // 在这个条件下 pNowFocus 一定不为 NULL 所以不用判断。
      //lpStation->m_pMouseFocus = pNowFocus;
      gxSendMessageW(hNowFocus, GXWM_NCMOUSEHOVER, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
      gxSendMessageW(hNowFocus, GXWM_MOUSEHOVER, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
    }

    lpStation->m_pMouseFocus = pNowFocus;
    hMouseFocus = hNowFocus;
  }

  ASSERT(lpStation->m_pMouseFocus == pNowFocus);

  if(hMouseFocus != NULL)
  {
    // TODO: 现在非客户区也会受到MouseMove消息,要改成NCMouseMove
    gxSendMessageW(hMouseFocus, GXWM_SETCURSOR, (GXWPARAM)hMouseFocus, GXMAKELPARAM(ht, GXWM_MOUSEMOVE));

    if( ! bEnabled) {
      return FALSE;
    }

    if(ht == GXHTCLIENT)
    {
      pNowFocus->ScreenToClient(&ptClient, 1);
      if(msg == NULL) {
        gxSendMessageW(hMouseFocus, GXWM_MOUSEMOVE, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
      }
      else {
        msg->hwnd    = hMouseFocus;
        msg->message = GXWM_MOUSEMOVE;
        msg->lParam  = GXMAKELPARAM(ptClient.x, ptClient.y);
        ASSERT(msg->wParam == fwKeys);
      }
    }
    else // Not Client
    {
      if(msg == NULL) {
        gxSendMessageW(hMouseFocus, GXWM_NCMOUSEMOVE, fwKeys, GXMAKELPARAM(ptClient.x, ptClient.y));
      }
      else {
        msg->hwnd    = hMouseFocus;
        msg->message = GXWM_NCMOUSEMOVE;
        msg->lParam  = GXMAKELPARAM(ptClient.x, ptClient.y);
        ASSERT(msg->wParam == fwKeys);
      }
    }

  }
  return TRUE;
}
extern "C" GXBOOL GXDLLAPI GXUIPostRootMessage(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
//#ifdef _ENABLE_STMT
//  GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].hwnd    = hWnd;
//  GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].message  = message;
//  GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].wParam    = wParam;
//  GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].lParam    = lParam;
//  GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].time    = gxGetTickCount();
//  GetCursorPos((LPPOINT)&GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].pt);
//  ScreenToClient(hWnd, (LPPOINT)&GXWnd::s_msg[GXWnd::s_idxMsgStackBottom].pt);
//
//  GXWnd::s_idxMsgStackBottom++;
//  GXWnd::s_idxMsgStackBottom &= 63;
//
//  STMT::SetTaskEvent(STMT::hDefHandle);
//  STMT::YieldTask();
//#else
  GXLPSTATION lpStation = IntGetStationPtr();
  MOUIMSG ThreadMsg;
  GXPOINT ptCursor;
  if(message == GXWM_MOUSEMOVE)
  {
    ptCursor.x = (GXLONG)(GXSHORT)GXLOWORD(lParam);
    ptCursor.y = (GXLONG)(GXSHORT)GXHIWORD(lParam);
    lpStation->SetCursorPos(&ptCursor);
  }
  else
    gxGetCursorPos(&ptCursor);
#if defined(_WIN32) || defined(_WINDOWS)
  ASSERT((GXLPVOID)hWnd != (GXLPVOID)lpStation->hBindWin32Wnd);
#endif // defined(_WIN32) || defined(_WINDOWS)

  ThreadMsg.handle  = hWnd;
  ThreadMsg.message = message;
  ThreadMsg.wParam  = wParam;
  ThreadMsg.lParam  = lParam;
  ThreadMsg.dwTime  = gxGetTickCount();
  ThreadMsg.xPos    = ptCursor.x;
  ThreadMsg.yPos    = ptCursor.y;

  if(lpStation->m_pMsgThread == NULL)
    return FALSE;
  lpStation->m_pMsgThread->PostMessage(&ThreadMsg);
//#endif // #ifdef _ENABLE_STMT
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//GXLPWND GXWnd::GetActive()
//{
//  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
//  for(GXLPWND_ARRAY::reverse_iterator it = lpStation->m_aActiveWnds.rbegin();
//    it != lpStation->m_aActiveWnds.rend(); ++it)
//  {
//    const GXULONG uStyle = (*it)->m_uStyle;
//    // TODO: 不可见的窗口或者被禁用的窗口还会在队列尾端吗? 应该向后放了吧?
//    if((uStyle & (GXWS_VISIBLE | GXWS_DISABLED)) == GXWS_VISIBLE)
//      return *it;
//  }
//  return NULL;
//}

GXLPWND GXWnd::GetActiveOrder(const GXWINDOWPOS* pWndPos) const
{
  GXLPWND lpWnd = GXLPWND_STATION_PTR(this)->lpDesktopWnd->m_pFirstChild;

  if(lpWnd == NULL)
    return NULL;

  if(pWndPos->hwndInsertAfter == NULL)  // GXHWND_TOP
  {
    if((m_uExStyle & GXWS_EX_TOPMOST) == 0)
    {
       return lpWnd->GetLastSiblingNoTopMost();
    }
    else
    {
      while(1)
      {
        if(lpWnd->m_pNextWnd == NULL)
          return lpWnd;
        lpWnd = lpWnd->m_pNextWnd;
      }
    }
  }
  else if(pWndPos->hwndInsertAfter == GXHWND_BOTTOM)
  {
    return NULL;
  }
  else if(pWndPos->hwndInsertAfter == GXHWND_TOP ||
    pWndPos->hwndInsertAfter == GXHWND_TOPMOST)
  {
    // 暂不支持
    return NULL;
  }
  else if(CHECK_HWND_VAILD(pWndPos->hwndInsertAfter))
  {
    GXLPWND lpInsertAfter = GXWND_PTR(pWndPos->hwndInsertAfter);
    
    // 出现这个断言的问题是调用者没有处理这个情况.顶层窗口插入到非顶层之后将失去顶层属性
    ASSERT(TEST_FLAG(m_uExStyle, GXWS_EX_TOPMOST) &&
      TEST_FLAG(lpInsertAfter->m_uExStyle, GXWS_EX_TOPMOST) == FALSE);

    if(TEST_FLAG(m_uExStyle, GXWS_EX_TOPMOST) == 0)
    {
      if(TEST_FLAG(lpInsertAfter->m_uExStyle, GXWS_EX_TOPMOST) == 0)
      {
        return lpInsertAfter->m_pPrevWnd;
      }
      else  // 非顶层Wnd插入到顶层Wnd后
      {
        return lpWnd->GetLastSiblingNoTopMost();
      }
    }
    else
    {
      return lpInsertAfter->m_pPrevWnd;
    }
  }
  return NULL;
}

GXLPWND GXWnd::GetLastSiblingNoTopMost()
{
  GXLPWND lpWnd = this;
  while(1)
  {
    if(lpWnd->m_pNextWnd == NULL || 
      (lpWnd->m_pNextWnd->m_uExStyle & GXWS_EX_TOPMOST))  // 截至到顶层窗口
    {
      return lpWnd;
    }
    lpWnd = lpWnd->m_pNextWnd;
  }
}

GXLPWND GXWnd::GetLastSibling()
{
  GXLPWND lpWnd = this;
  while(lpWnd != NULL)
  {
    if(lpWnd->m_pNextWnd == NULL) {
      return lpWnd;
    }
    lpWnd = lpWnd->m_pNextWnd;
  }
  return NULL;
}

GXLPWND GXWnd::SetActive()
{
  //TRACEW(L"SetActive %x(%s)\n", this, m_pText != NULL ? m_pText : L"");
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  LPGXWND lpDesktop = lpStation->lpDesktopWnd;

  lpStation->Enter();
  if(gxIsTopLevelWindow(m_hSelf) == FALSE)
  {
    LPGXWND lpTop = GXWND_PTR(GXGetTopLevel());
    lpStation->Leave();
    return lpTop->SetActive();
  }

  GXWINDOWPOS WndPos;
  WndPos.hwnd = m_hSelf;
  WndPos.hwndInsertAfter = NULL;
  WndPos.x     = rectWindow.left;
  WndPos.y     = rectWindow.top;
  WndPos.cx    = rectWindow.right - rectWindow.left;
  WndPos.cy    = rectWindow.bottom - rectWindow.top;
  WndPos.flags = NULL;
  gxSendMessage(WndPos.hwnd, GXWM_WINDOWPOSCHANGING, NULL, (GXLPARAM)&WndPos);

  // 不能放在自己之后
  if(WndPos.hwndInsertAfter == WndPos.hwnd) {
    WndPos.hwndInsertAfter = NULL;
  }

  GXWnd* pInsert = GetActiveOrder(&WndPos);   // this 放在窗口链 pInsert 的后面
  GXWnd* pActive = lpStation->GetActiveWnd(); // 旧的激活窗口

  //// 如果已经是顶层窗口，则返回
  //if(pInsert == this)
  //{
  //  // TODO: 发送激活/非激活消息
  //  goto UPDATE_ORDER;
  //}

  // 如果是激活窗口,则返回
  if(pActive == this)
  {
    lpStation->Leave();
    return pActive;
  }

  ASSERT(pInsert == NULL || pInsert->m_pWinsSurface != NULL);
  ASSERT(m_pWinsSurface != NULL);

  DWM_ACTIVEWINDOWS sActiveWindows;

  sActiveWindows.lpInactiveWnd = pActive;
  sActiveWindows.lpInsertWnd   = pInsert;
  sActiveWindows.lpActiveWnd   = this;
  sActiveWindows.lpGenFirst    = NULL;
  sActiveWindows.prgnAfter     = NULL;
  sActiveWindows.prgnBefore    = NULL;

  if(pInsert != NULL)
  {
    lpStation->m_pDesktopWindowsMgr->ActiveWindows(GXWA_INACTIVE, &sActiveWindows);
    if(pInsert != this)
    {
      if(m_pPrevWnd == NULL)
      {
        lpDesktop->m_pFirstChild = m_pNextWnd;
        m_pNextWnd->m_pPrevWnd = NULL;
      }

      if(m_pPrevWnd != NULL)
        m_pPrevWnd->m_pNextWnd = m_pNextWnd;
      if(m_pNextWnd != NULL)
        m_pNextWnd->m_pPrevWnd = m_pPrevWnd;

      if(pInsert->m_pNextWnd != NULL)
      {
        m_pNextWnd = pInsert->m_pNextWnd;
        pInsert->m_pNextWnd->m_pPrevWnd = this;
        pInsert->m_pNextWnd = this;
        m_pPrevWnd = pInsert;
      }
      else// if(pForeground->m_pNextFrame == NULL)
      {
        pInsert->m_pNextWnd = this;
        m_pPrevWnd = pInsert;
        m_pNextWnd = NULL;
      }
    }

    lpStation->m_pDesktopWindowsMgr->ActiveWindows(GXWA_ACTIVE, &sActiveWindows);
  }

//UPDATE_ORDER:
  for(auto it = lpStation->m_aActiveWnds.begin();
    it != lpStation->m_aActiveWnds.end(); ++it) {
    if((*it) == this)
    {
      lpStation->m_aActiveWnds.erase(it);
      break;
    }
  }

  lpStation->m_aActiveWnds.push_back(this);
  lpStation->Leave();
  return pActive;
}

//////////////////////////////////////////////////////////////////////////
GXLPWND GXWnd::GetForeground()
{
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  return lpStation->GetActiveWnd();
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXWnd::SetForeground()
{
  GXLPWND lpWnd = SetActive();
  return lpWnd != this;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXWnd::GetBoundingRect(GXBOOL bWindow, GXRECT* lprcOut)
{
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  *lprcOut = rectWindow;
  GXBOOL bval = TRUE;
  // TODO: 用GetNonclientThickness替代
  // 求客户区的裁剪
  if(bWindow == FALSE)
  {
    if(WINSTYLE_HASCAPTION(m_uStyle))
    {
      lprcOut->left   += FRAME_NC_EDGE_LEFT;
      lprcOut->top    += FRAME_NC_EDGE_CAPTION;
      lprcOut->right  -= FRAME_NC_EDGE_RIGHT;
      lprcOut->bottom -= FRAME_NC_EDGE_BOTTOM;
      bval = FALSE;
    }
    if(WINSTYLE_HASMENU(this, lpStation->lpDesktopWnd))
    {
      const GXUINT nHeight = MENU_GetMenuBarHeightFast(
        m_hSelf, lprcOut->right - lprcOut->left, 
        lprcOut->left - rectWindow.left, lprcOut->top - rectWindow.top);
      lprcOut->top += nHeight;
      bval = FALSE;
    }
    if(m_uStyle & GXWS_VSCROLL) {
      lprcOut->right -= g_SystemMetrics[GXSM_CXVSCROLL];
      bval = FALSE;
    }
    if(m_uStyle & GXWS_HSCROLL) {
      lprcOut->bottom -= g_SystemMetrics[GXSM_CYHSCROLL];
      bval = FALSE;
    }
  }
  return bval;
}

void GXWnd::GetNonclientThickness(GXRECT* lpThickness, GXDWORD dwStyle, GXBOOL bMenu)
{
  lpThickness->left   = 0;
  lpThickness->top    = 0;
  lpThickness->right  = 0;
  lpThickness->bottom = 0;

  if(WINSTYLE_HASCAPTION(dwStyle))
  {
    lpThickness->left   = FRAME_NC_EDGE_LEFT;
    lpThickness->top    = FRAME_NC_EDGE_CAPTION;
    lpThickness->right  = FRAME_NC_EDGE_RIGHT;
    lpThickness->bottom = FRAME_NC_EDGE_BOTTOM;
  }

  if(bMenu) {
    lpThickness->top += g_SystemMetrics[GXSM_CYMENU];
  }

  if(TEST_FLAG(dwStyle, GXWS_VSCROLL)) {
    lpThickness->right += g_SystemMetrics[GXSM_CXVSCROLL];
  }

  if(TEST_FLAG(dwStyle, GXWS_HSCROLL)) {
    lpThickness->bottom -= g_SystemMetrics[GXSM_CYHSCROLL];
  }
}

GXVOID GXWnd::GetWindowRect(GXRECT *lpRect) const
{
  *lpRect = rectWindow;
}

GXVOID GXWnd::ClientToScreen(GXLPPOINT lpPoints, GXUINT cPoints)
{
  // 目前将整个窗口(gxWin)当作客户区
  // 他的标题栏和边框都在客户区坐标内
  //GXWnd *pParent = m_pParent;
  //while(pParent != NULL)
  //{
  CHECK_LPWND_VAILD(this);
  GXLONG x = 0, y = 0;
  if(WINSTYLE_HASCAPTION(m_uStyle))
  {
    x = FRAME_NC_EDGE_LEFT;
    y = FRAME_NC_EDGE_CAPTION;
  }
  if(WINSTYLE_HASMENU(this, GXLPWND_STATION_PTR(this)->lpDesktopWnd))
  {
    const GXUINT nHeight = MENU_GetMenuBarHeightFast(
      m_hSelf, rectWindow.right - rectWindow.left, x, y);
    y += nHeight;
  }
  for(GXUINT i = 0; i < cPoints; i++)
  {
    lpPoints[i].x += (x + rectWindow.left);
    lpPoints[i].y += (y + rectWindow.top);
  }
}

GXVOID GXWnd::ScreenToClient(GXLPPOINT lpPoints, GXUINT cPoints)
{
  // 目前将整个窗口当作客户区
  // 目前将整个窗口(gxWin)当作客户区
  // 他的标题栏和边框都在客户区坐标内

  CHECK_LPWND_VAILD(this);
  for(GXUINT i = 0; i < cPoints; i++)
  {
    if(WINSTYLE_HASCAPTION(m_uStyle))
    {
      lpPoints[i].x -= FRAME_NC_EDGE_LEFT;
      lpPoints[i].y  -= FRAME_NC_EDGE_CAPTION;
    }

    lpPoints[i].x -= rectWindow.left;
    lpPoints[i].y -= rectWindow.top;
  }
}

GXINT GXWnd::MapWindowPoints(
  GXLPWND lpWndFrom,    // handle of window to be mapped from 
  GXLPPOINT lpPoints,   // address of structure array with points to map 
  GXUINT cPoints        // number of structures in array 
  ) const
{
  lpWndFrom = GXWND_HANDLE(lpWndFrom) == GXHWND_DESKTOP ? GetDesktop() : lpWndFrom;

  GXPOINT  ptDelta;
  ptDelta.x = lpWndFrom->rectWindow.left - rectWindow.left;
  ptDelta.y = lpWndFrom->rectWindow.top  - rectWindow.top;

  for(GXUINT i = 0;i < cPoints; i++)
  {
    lpPoints[i].x += ptDelta.x;
    lpPoints[i].y += ptDelta.y;
  }
  return GXMAKELONG(ptDelta.x, ptDelta.y);
}
//////////////////////////////////////////////////////////////////////////
GXVOID GXWnd::_GetSystemMinMaxInfo(GXLPMINMAXINFO lpmmi)
{
  GXMEMSET(lpmmi, 0, sizeof (GXMINMAXINFO));
  lpmmi->ptMaxSize.x    = g_SystemMetrics[GXSM_CXSCREEN];
  lpmmi->ptMaxSize.y    = g_SystemMetrics[GXSM_CYSCREEN];
  lpmmi->ptMaxTrackSize.x = g_SystemMetrics[GXSM_CXSCREEN];
  lpmmi->ptMaxTrackSize.y = g_SystemMetrics[GXSM_CYSCREEN];
  //ASSERT(lpmmi->ptMaxSize.x    == CD3DGraphics::s_d3dpp.BackBufferWidth);
  //ASSERT(lpmmi->ptMaxSize.y    == CD3DGraphics::s_d3dpp.BackBufferHeight);
  //ASSERT(lpmmi->ptMaxTrackSize.x  == CD3DGraphics::s_d3dpp.BackBufferWidth);
  //ASSERT(lpmmi->ptMaxTrackSize.y  == CD3DGraphics::s_d3dpp.BackBufferHeight);
  lpmmi->ptMinTrackSize.x = 220;
  lpmmi->ptMinTrackSize.y = FRAME_NC_EDGE_CAPTION + FRAME_NC_EDGE_BOTTOM;
}
//////////////////////////////////////////////////////////////////////////
GXVOID GXWnd::IntMoveChild(GXINT dx, GXINT dy)
{
  GXLPWND lpWnd = m_pFirstChild;
  while(lpWnd != NULL)
  {
    gxOffsetRect(&lpWnd->rectWindow, dx, dy);

    if(lpWnd->m_pFirstChild != NULL)
      lpWnd->IntMoveChild(dx, dy);
    lpWnd = lpWnd->m_pNextWnd;
  }
}

GXINT GXWnd::Scroll(GXINT dx, GXINT dy, GXCONST GXRECT *prcScroll, GXCONST GXRECT * prcClip, GXHRGN hrgnUpdate, GXLPRECT prcUpdate, GXUINT flags)
{
  SCROLLTEXTUREDESC stdesc;

  if((dx == 0 && dy == 0) || ! IsVisible()) {
    return NULLREGION;
  }

  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  GRegion* prgnClip = NULL;
  GRegion* prgnUpdate = NULL;
  GXRECT rcClient;
  GXRECT rcScrollClient;
  //GXRECT rcClipClient;

  GetBoundingRect(FALSE, &rcClient);
  rcScrollClient = rcClient;
  GetSystemRegion(GSR_PARENTCLIP|GSR_CLIPSIBLINGS, &prgnClip);
  //rcClipClient = rcClient;

  // 用户有指定裁剪区
  if(prcClip != NULL)
  {
    GXRECT rcUserClip = *prcClip;
    gxOffsetRect(&rcUserClip, rcClient.left, rcClient.top);
    GRegion* prgnUserClip = NULL;

    lpStation->pGraphics->CreateRectRgn(&prgnUserClip, 
      rcUserClip.left, rcUserClip.top, rcUserClip.right, rcUserClip.bottom);

    prgnClip->Intersect(prgnUserClip);

    SAFE_RELEASE(prgnUserClip);
    //gxIntersectRect(&rcClipClient, &rcClipClient, &rcClient);
  }

  //lpStation->pGraphics->CreateRectRgn(&prgnClip, 
  //  rcClipClient.left, rcClipClient.top,
  //  rcClipClient.right, rcClipClient.bottom);

  if(prcScroll != NULL)
  {
    rcScrollClient = *prcScroll;
    gxOffsetRect(&rcScrollClient, rcClient.left, rcClient.top);
    gxIntersectRect(&rcScrollClient, &rcScrollClient, &rcClient);
  }

  // TODO: 将来改用DefWindowPos
  // 滚动子窗体
  if(TEST_FLAG(flags, GXSW_SCROLLCHILDREN))
  {
    GXLPWND lpChildWnd = m_pFirstChild;
    GXRECT rcResult;

    // 滚动子窗体, 根据 prcScroll
    if(prcScroll != NULL)
    {
      while(lpChildWnd != NULL)
      {
        if(gxIntersectRect(&rcResult, &lpChildWnd->rectWindow, &rcScrollClient))
        {
          gxOffsetRect(&lpChildWnd->rectWindow, dx, dy);
          if(lpChildWnd->m_pFirstChild) {
            lpChildWnd->IntMoveChild(dx, dy);
          }
        }
        lpChildWnd = lpChildWnd->m_pNextWnd;
      }
    }
    else  // 没指定Scroll区域, 滚动所有子窗体
    {
      while(lpChildWnd != NULL)
      {
        gxOffsetRect(&lpChildWnd->rectWindow, dx, dy);
        if(lpChildWnd->m_pFirstChild) {
          lpChildWnd->IntMoveChild(dx, dy);
        }
        lpChildWnd = lpChildWnd->m_pNextWnd;
      }
    }
  }

  GXWindowsSurface* pSurface = GetTopSurface();

  // 如果滚动区内含有脏区域,则减掉它
  if(m_prgnUpdate != NULL) {
    prgnClip->Subtract(m_prgnUpdate);
  }
  if(pSurface->m_prgnUpdate != NULL) {
    prgnClip->Subtract(pSurface->m_prgnUpdate);
  }
  // </减掉脏区域>

  stdesc.pOperationTex = pSurface->m_pRenderTar->GetTextureUnsafe();
  stdesc.pTempTex      = NULL;
  stdesc.dx            = dx; 
  stdesc.dy            = dy;
  stdesc.lprcScroll    = &rcScrollClient;
  stdesc.lprgnClip     = prgnClip;
  stdesc.lpprgnUpdate  = &prgnUpdate;
  stdesc.lprcUpdate    = prcUpdate;

  RGNCOMPLEX rc = RC_NULL;
  if(lpStation->pGraphics->ScrollTexture(&stdesc))
  {
    ASSERT(prgnUpdate != NULL);
    rc = prgnUpdate->GetComplexity();

    if(rc != RC_NULL && TEST_FLAG(flags, GXSW_INVALIDATE)) {
      pSurface->InvalidateRegion(prgnUpdate);
    }

    if(hrgnUpdate != NULL)
    {
      GXGDIREGION* pgdirgnUpdate = GXGDI_RGN_PTR(hrgnUpdate);
      SAFE_RELEASE(pgdirgnUpdate->lpRegion);
      pgdirgnUpdate->lpRegion = prgnUpdate;
    }
    else
    {
      SAFE_RELEASE(prgnUpdate);
    }
  }

  SAFE_RELEASE(prgnClip);
  return (int)rc;
}

GXBOOL GXWnd::SetPos(GXHWND hWndInsertAfter, int x, int y, int cx, int cy, GXUINT uFlags)
{
  ASSERT(hWndInsertAfter == NULL || hWndInsertAfter == GXHWND_TOPMOST);

  if(hWndInsertAfter == GXHWND_TOPMOST)
  {
    SET_FLAG(m_uExStyle, GXWS_EX_TOPMOST);
  }
  //GXLPWND lpWnd = GXWND_PTR(hWnd);
  //LPGXWND lpDesktop = GXLPWND_STATION_PTR(lpWnd)->lpRootFrame;
  CHECK_LPWND_VAILD(lpWnd);

  // 支持列表
  ASSERT((uFlags & (~(
    GXSWP_NOMOVE|
    GXSWP_NOSIZE|
    GXSWP_NOREDRAW|
    GXSWP_HIDEWINDOW|
    GXSWP_SHOWWINDOW|
    GXSWP_NOZORDER|
    GXSWP_DRAWFRAME|  // 没实现
    GXSWP_NOACTIVATE))) == 0);

  GXREGN rgWindow;
  GXBOOL bNeedMoveSize = FALSE;
  gxRectToRegn(&rgWindow, &rectWindow);

  // 移动窗口
  if( ! TEST_FLAG(uFlags, GXSWP_NOMOVE))
  {
    rgWindow.left = x;
    rgWindow.top  = y;
    bNeedMoveSize = TRUE;
  }

  // 改变尺寸
  if( ! TEST_FLAG(uFlags, GXSWP_NOSIZE))
  {
    rgWindow.width  = cx;
    rgWindow.height = cy;
    bNeedMoveSize = TRUE;
  }

  if(TEST_FLAG(uFlags, GXSWP_SHOWWINDOW))
  {
    gxShowWindow(m_hSelf, TEST_FLAG(uFlags, GXSWP_NOACTIVATE) 
      ? GXSW_SHOWNOACTIVATE
      : GXSW_SHOWNORMAL);
  }
  else if(TEST_FLAG(uFlags, GXSWP_HIDEWINDOW))
  {
    gxShowWindow(m_hSelf, GXSWP_HIDEWINDOW);
  }

  if(bNeedMoveSize)
  {
    const GXBOOL bRedraw = (TEST_FLAG(uFlags, GXSWP_NOREDRAW) == FALSE);
    Move(rgWindow.left, rgWindow.top, rgWindow.width, rgWindow.height, bRedraw);
  }

  return TRUE;
}

GXBOOL GXWnd::Move(int x, int y, int cx, int cy, GXBOOL bRepaint)
{
  // 坐标转换
  GXPOINT ptOrg = {x, y};
  m_pParent->ClientToScreen(&ptOrg, 1);
  x = ptOrg.x;
  y = ptOrg.y;

  const int dx = x - rectWindow.left;
  const int dy = y - rectWindow.top;
  const int dw = cx - (rectWindow.right - rectWindow.left);
  const int dh = cy - (rectWindow.bottom - rectWindow.top);

  GXBOOL bRedraw = bRepaint && (m_uStyle & GXWS_VISIBLE);

  if(dw != 0 || dh != 0)
  {
    if(bRedraw)
    {
      Size(x, y, cx, cy);
    }
    else {
      gxSetRect(&rectWindow, x, y, x + cx, y + cy);
      IntMoveChild(dx, dy);
    }
  }
  else if(dx != 0 || dy != 0)
  {
    if(bRedraw)
    {
      MoveOnly(x, y);
    }
    else {
      gxOffsetRect(&rectWindow, dx, dy);
      IntMoveChild(dx, dy);
    }
  }
  return TRUE;
}

GXBOOL GXWnd::Size(GXINT x, GXINT y, GXINT nWidth, GXINT nHeight)
{
  GXWindowsSurface* pSurface = GetTopSurface();
  ASSERT(pSurface != NULL);

  GRegion* pBeforeRegion = NULL;
  GRegion* pAfterRegion = NULL;
  const GXDWORD dwFlags = GSR_PARENTCLIP|GSR_CLIPSIBLINGS|GSR_WINDOW;
  GXBOOL bSavePrevRgn = (rectWindow.right - rectWindow.left) > nWidth ||
    (rectWindow.bottom - rectWindow.top) > nHeight;

  const int dx = x - rectWindow.left;
  const int dy = y - rectWindow.top;
  if(dx != 0 || dy != 0) {
    IntMoveChild(dx, dy);
  }

  if(bSavePrevRgn) {
    GetSystemRegion(dwFlags, &pBeforeRegion);
  }

  gxSetRect(&rectWindow, x, y, x + nWidth, y + nHeight);

  // WM_SIZE Message
  {
    GXRECT rcClient;
    gxGetClientRect(m_hSelf, &rcClient);
    gxSendMessageW(m_hSelf, GXWM_SIZE, 0, (GXLPARAM)GXMAKELPARAM(rcClient.right, rcClient.bottom));
  }

  GetSystemRegion(dwFlags, &pAfterRegion);

  if(bSavePrevRgn) {
    // FIXME: 这里这么写会导致Window缩小时也重绘整个窗口
    pAfterRegion->Union(pBeforeRegion);
  }

  pSurface->InvalidateRegion(pAfterRegion);

  SAFE_RELEASE(pBeforeRegion);
  SAFE_RELEASE(pAfterRegion);
  pSurface->GenerateWindowsRgn(TRUE);
  return TRUE;
}

GXBOOL GXWnd::MoveOnly(GXINT x, GXINT y)
{
  GXWindowsSurface* pSurface = GetTopSurface();
  ASSERT(pSurface != NULL);

  const int dx = x - rectWindow.left;
  const int dy = y - rectWindow.top;
  const GXRECT rcOldWin = rectWindow;

  IntMoveChild(dx, dy);

  // TODO: 独占模式下不需要计算裁剪区域
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  GRegion* prgnOldUpdate = NULL;    // 没有及时更新的区域
  GRegion* prgnClip = NULL;
  GRegion* prgnBefore = NULL;
  GRegion* prgnAfter = NULL;

  // 计算没有及时更新的区域
  GetSystemRegion(GSR_PARENTCLIP|GSR_CLIPSIBLINGS|GSR_WINDOW, &prgnOldUpdate);
  RGNCOMPLEX rgncpxOld = prgnOldUpdate->Intersect(pSurface->m_prgnUpdate);

  // 得到窗口的所有可用区域
  GetSystemRegion(GSR_PARENTCLIP|GSR_CLIPSIBLINGS|GSR_AVAILABLE, &prgnClip);

  // 得到移动前后的区域
  GetWindowRegion(&prgnBefore);
  gxOffsetRect(&rectWindow, dx, dy);
  GetWindowRegion(&prgnAfter);

  prgnAfter->Union(prgnBefore);
  prgnClip->Intersect(prgnAfter);

  if(rgncpxOld != RC_NULL)
  {
    prgnOldUpdate->Offset(dx, dy);
    rgncpxOld = prgnOldUpdate->Intersect(prgnClip);
  }

  pSurface->Scroll(dx, dy, &rcOldWin, prgnClip, NULL);

  if(rgncpxOld != RC_NULL)
    pSurface->InvalidateRegion(prgnOldUpdate);
  pSurface->GenerateWindowsRgn(TRUE);
  SAFE_RELEASE(prgnBefore);
  SAFE_RELEASE(prgnAfter);
  SAFE_RELEASE(prgnOldUpdate);
  SAFE_RELEASE(prgnClip);

  return TRUE;
}

//void GXWnd::SysUpdateWindow(GRegion* prgnUpdate)
//{
//  CHECK_LPWND_VAILD(this);
//
//  GRegion* prgnWindow = NULL;
//  GRegion* prgnNClient = NULL;
//  GRegion* prgnClient = NULL;
//  GXRECT rcClient;
//
//  GetWindowRegion(&prgnWindow);
//
//  // prgnClip 应该包括了父窗口的裁剪
//  RGNCOMPLEX cpx = prgnWindow->Intersect(prgnUpdate);
//  if(cpx == RC_NULL) {
//    return;
//  }
//
//  if(GetBoundingRect(FALSE, &rcClient)) {
//    // 客户区占满整个窗口
//    prgnClient = prgnWindow;
//    prgnClient->AddRef();
//  }
//  else {
//    // 存在非客户区
//    GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
//    lpStation->pGraphics->CreateRectRgn(&prgnClient, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
//    prgnNClient = prgnWindow->CreateSubtract(prgnClient);
//    prgnClient->Intersect(prgnWindow);// 其实Subtract也可以
//  }
//
//  GXLPWND lpChildWnd = m_pFirstChild;
//
//  if(lpChildWnd == NULL)
//  {
//    // 更新非客户区
//    if(prgnNClient != NULL && ( ! prgnNClient->IsEmpty())) {
//      GXGDIREGION gdiRegion(prgnNClient);
//      gxSendMessageW(m_hSelf, GXWM_NCPAINT, (GXWPARAM)GXGDI_RGN_HANDLE(&gdiRegion), NULL);
//    }
//
//    // 更新客户区
//    if( ! prgnClient->IsEmpty()) {
//      if(m_prgnUpdate == NULL)
//      {
//        m_prgnUpdate = prgnClient;
//        prgnClient->AddRef();
//      }
//      else
//      {
//        m_prgnUpdate->Union(prgnClient);
//        InlSetNewObjectT(prgnClient, m_prgnUpdate);
//      }
//      gxSendMessageW(m_hSelf, GXWM_PAINT, 0, 0);
//    }
//
//    // 减掉
//    prgnUpdate->Subtract(prgnWindow);
//
//    SAFE_RELEASE(prgnClient);
//    SAFE_RELEASE(prgnNClient);
//    SAFE_RELEASE(prgnWindow);
//
//    return;
//  }
//
//  while(lpChildWnd != NULL)
//  {
//    if((lpChildWnd->m_uStyle & GXWS_VISIBLE) &&
//      gxIsRectEmpty(&lpChildWnd->rectWindow) == FALSE)
//    {
//      //lpWnd->UpdateWholeWindow(pSurface);
//    }
//    lpChildWnd = lpChildWnd->m_pNextFrame;
//  }
//
//}

void GXWnd::SetClientUpdateRegion(GRegion* prgnUpdate)
{
  if(m_prgnUpdate == NULL){
    m_prgnUpdate = prgnUpdate;
    prgnUpdate->AddRef();
  }
  else {
    m_prgnUpdate->Union(prgnUpdate);
  }
}

void GXWnd::UpdateWholeWindow(GXWindowsSurface* pWinsSurface, GRegion* prgnPainted)
{
  // TODO: 要好好考虑下 不应该是用GetSystemRegion这个通用函数,应该使用一个深度优化的高效计算方法
  CHECK_LPWND_VAILD(this);
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  GRegion* prgnCurPainted = NULL;
  if(prgnPainted != NULL) {
    prgnCurPainted = prgnPainted;
    prgnCurPainted->AddRef();
  }
  else {
    lpStation->pGraphics->CreateRectRgn(&prgnCurPainted, 0, 0, 0, 0);
  }

  GRegion* prgnWindow;
  // 窗口全部区域
  RGNCOMPLEX r = (RGNCOMPLEX)GetSystemRegion(GSR_PARENTCLIP|GSR_CLIPSIBLINGS|GSR_WINDOW, &prgnWindow);

  if(r != RC_NULL)
  {
    GRegion* pClientRgn = NULL;
    // 需要更新区域
    //if(TEST_FLAG_NOT(m_uExStyle, GXWS_EX_TRANSPARENT))
    r = prgnWindow->Intersect(pWinsSurface->m_prgnUpdate);
    if(r != RC_NULL)
    {
      GXRECT rcClient;
      GXHWND hPaintWnd = m_hSelf;

      GRegion* pNClientRgn = NULL;

      // 分开计算客户区和非客户区
      if(GetBoundingRect(FALSE, &rcClient)) { // 客户区占满整个窗口
        pClientRgn = prgnWindow;
        pClientRgn->AddRef();
      }
      else { // 有非客户区
        lpStation->pGraphics->CreateRectRgn(&pClientRgn, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
        r = pClientRgn->Intersect(prgnWindow);
        pNClientRgn = prgnWindow->CreateSubtract(pClientRgn);
      }

      // 客户区 - 标准
      if(r != RC_NULL) {
        SetClientUpdateRegion(pClientRgn);
        gxSendMessageW(hPaintWnd, GXWM_PAINT, 0, 0);
      }
      
      // 非客户区
      if(pNClientRgn != NULL && ( ! pNClientRgn->IsEmpty())) {
        GXGDIREGION gdiRegion(pNClientRgn);
        gxSendMessageW(hPaintWnd, GXWM_NCPAINT, (GXWPARAM)GXGDI_RGN_HANDLE(&gdiRegion), NULL);
        //pWinsSurface->m_prgnUpdate->Subtract(pNClientRgn);
      }

      // 子窗口
      if(m_pFirstChild != NULL) {
        GXLPWND lpChild = m_pFirstChild;
        do 
        {
          if((lpChild->m_uStyle & GXWS_VISIBLE) && 
            gxIsRectEmpty(&lpChild->rectWindow) == FALSE) {
            lpChild->UpdateWholeWindow(pWinsSurface, prgnCurPainted);
          }

          lpChild = lpChild->m_pNextWnd;
        } while (lpChild != NULL);
      }

      SAFE_RELEASE(pNClientRgn);

      // 外部函数是否想获得已更新区域
      // 如果不想就直接在Update里减掉,否则先存起来
      if(prgnPainted == NULL) {
        pWinsSurface->m_prgnUpdate->Subtract(prgnWindow);
      }
      else {
        prgnCurPainted->Union(prgnWindow);
      }
    }
    ASSERT(m_prgnUpdate == NULL); // 经过WM_PAINT,这个应该被清空了, 如果不为空,查找原因,或者在pWinsSurface->m_prgnUpdate加回去?
    //if(m_prgnUpdate == NULL && pClientRgn != NULL)
    //  pWinsSurface->m_prgnUpdate->Subtract(pClientRgn);
    SAFE_RELEASE(pClientRgn);
  }
  SAFE_RELEASE(prgnCurPainted);
  SAFE_RELEASE(prgnWindow);
}

GXBOOL GXWnd::InvalidateRect(GXCONST GXRECT* lpRect, GXBOOL bErase)
{
  GXRECT rcUpdate;
  GetBoundingRect(FALSE, &rcUpdate);
  gxOffsetRect(&rcUpdate, -rcUpdate.left, -rcUpdate.top);

  if(lpRect != NULL)
  {
    GXRECT rcUser = *lpRect;
    if(gxIntersectRect(&rcUpdate, &rcUpdate, lpRect) == 0)
      return FALSE;
  }

  // FIXME: rcUpdate是clinet space的，但是GXInvalidateWindowRect参数应该是window space的
  return GXInvalidateWindowRect(m_hSelf, &rcUpdate, bErase);
}

GXBOOL GXWnd::InvalidateRgn(GRegion* pRegion, GXBOOL bErase)
{
  GXRECT sThickness;
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);

  GetNonclientThickness(&sThickness, m_uStyle, WINSTYLE_HASMENU(this, lpStation->lpDesktopWnd));

  if(sThickness.left == 0 && sThickness.top == 0) {
    return GXInvalidateWindowRgn(m_hSelf, pRegion, bErase);
  }
  else {
    GRegion* pWinRegion = pRegion->Clone();
    pWinRegion->Offset(-sThickness.left, -sThickness.top);
    GXBOOL bresult = GXInvalidateWindowRgn(m_hSelf, pWinRegion, bErase);
    pWinRegion->Release();
    return bresult;
  }

  //NOT_IMPLEMENT_FUNC_MAKER;
  //InvalidateRect(NULL, bErase); // 这是替代函数
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GXDLLAPI gxIsTopLevelWindow(GXHWND hWnd)
{
  LPGXWND lpDesktop = IntGetStationPtr()->lpDesktopWnd;
  return ((GXWND_PTR(hWnd)->m_pParent == NULL) || (GXWND_PTR(hWnd)->m_pParent == lpDesktop));
}
//////////////////////////////////////////////////////////////////////////
GXHWND GXWnd::GXGetTopLevel()
{
  GXLPWND lpWnd = this;
  GXWnd *pParent = m_pParent;
  LPGXWND lpDesktop = GXLPWND_STATION_PTR(lpWnd)->lpDesktopWnd;

  while(pParent != NULL && pParent != lpDesktop)
  {
    lpWnd = pParent;
    pParent = lpWnd->m_pParent;
  }
  return GXWND_HANDLE(lpWnd);
}

int GXWnd::GetWindowRegion(GRegion** ppRegion)
{
  if(FALSE && gxIsTopLevelWindow(m_hSelf) && (m_uStyle & GXWS_CAPTION))
  {
    // 亮点!
    GXLPWND_STATION_PTR(this)->pGraphics->CreateRoundRectRgn(ppRegion, 
      rectWindow, 4, 4);
    return GXCOMPLEXREGION;
  }
  else
  {
    GXLPWND_STATION_PTR(this)->pGraphics->CreateRectRgn(ppRegion, 
      rectWindow.left, rectWindow.top, rectWindow.right, rectWindow.bottom);
    return GXSIMPLEREGION;
  }
}

int GXWnd::GetSystemRegion(GXDWORD dwFlags, GRegion** ppRegion)
{
  // TODO: 如果是TopLevel 窗口,会裁剪两边兄弟窗口
  GXLPSTATION pStation  = GXLPWND_STATION_PTR(this);
  LPGXWND lpDesktop    = pStation->lpDesktopWnd;
  GRegion* pRegion    = *ppRegion;
  GRegion* pClipRegion  = NULL;
  LPGXWND lpSiblings    = m_pNextWnd;

  // 获得窗口自己的裁剪区
  GXRECT rcClip;
  if(dwFlags & GSR_AVAILABLE)
  {
    gxSetRect(&rcClip, 0, 0, pStation->nWidth, pStation->nHeight);
    pStation->pGraphics->CreateRectRgn(&pRegion, 
      rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
  }
  else 
  {  
    GXRECT rcStation;
    gxSetRect(&rcStation, 0, 0, pStation->nWidth, pStation->nHeight);
    GetBoundingRect(dwFlags & GSR_WINDOW, &rcClip);

    if(TEST_FLAG_NOT(dwFlags, GSR_WINDOW))
    {
      gxIntersectRect(&rcClip, &rcClip, &rcStation);
      pStation->pGraphics->CreateRectRgn(&pRegion, 
        rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
    }
    else
    {
      GRegion* prgnStation = NULL;
      GetWindowRegion(&pRegion);

      // 如果窗口区域超过屏幕, 则进行裁剪
      // rcClip区域应该与pRegion包围盒相等!!!
      if(rcClip.left < 0 || rcClip.top < 0 ||
        rcClip.right > rcStation.right || rcClip.bottom > rcStation.bottom)
      {
        pStation->pGraphics->CreateRectRgn(&prgnStation, 
          rcStation.left, rcStation.top, rcStation.right, rcStation.bottom);

        pRegion->Intersect(prgnStation);
        pRegion->GetBounding(&rcClip);

        SAFE_RELEASE(prgnStation);
      }
    }
  }

  *ppRegion = pRegion;

  RGNCOMPLEX rc = RC_SIMPLE;

  // 已经是空的了,返回
  if(gxIsRectEmpty(&rcClip))
    return NULLREGION;

  // 裁剪所有父窗口
  if(dwFlags & GSR_PARENTCLIP)
  {
    LPGXWND pCanvasWnd = this;
    while( pCanvasWnd->m_pWinsSurface == NULL && 
      gxIsTopLevelWindow(GXWND_HANDLE(pCanvasWnd)) == FALSE)
    {
      pCanvasWnd = pCanvasWnd->m_pParent;
      pCanvasWnd->GetBoundingRect(FALSE, &rcClip);
      //GXGetClip(GXWND_HANDLE(pCanvasWnd), FALSE, &rcClip);
      pStation->pGraphics->CreateRectRgn(&pClipRegion, 
        rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);

      rc = pRegion->Intersect(pClipRegion);
      SAFE_RELEASE(pClipRegion);

      if(rc == RC_NULL) 
        return NULLREGION;
    }

    // 裁剪父窗口的兄弟窗口
    GXWindowsSurface* pWinsSurface = pCanvasWnd->m_pWinsSurface;
    pCanvasWnd = pCanvasWnd->m_pNextWnd;

    // 如果TopLevel窗口的兄弟窗口和自己的兄弟窗口一致,则说明自己是TopLevel,跳过这个
    if(lpSiblings != pCanvasWnd)
    {
      while(pCanvasWnd != NULL)
      {
        if((pCanvasWnd->m_uStyle & GXWS_VISIBLE) &&
          ((dwFlags & GSR_ALLLAYERS) != 0 || pWinsSurface == pCanvasWnd->m_pWinsSurface))
        {
          pCanvasWnd->GetWindowRegion(&pClipRegion);

          rc = pRegion->Subtract(pClipRegion);
          SAFE_RELEASE(pClipRegion);

          if(rc == RC_NULL) 
            return NULLREGION;
        }
        pCanvasWnd = pCanvasWnd->m_pNextWnd;
      }
    }
  }

  if(dwFlags & GSR_CLIPSIBLINGS)
  {
    LPGXWND lpWnd = m_pNextWnd;
    GXRECT rcTest;  // 用来保存测试两个窗口是否相交的结果, 后面应该不会再用到这个东东

    if(TEST_FLAG_NOT(dwFlags, GSR_ALLLAYERS) &&
      gxIsTopLevelWindow(m_hSelf))
    {      
      while(lpWnd != NULL)
      {
        if(TEST_FLAG(lpWnd->m_uStyle, GXWS_VISIBLE) && 
          TEST_FLAG_NOT(lpWnd->m_uExStyle, GXWS_EX_TRANSPARENT) &&
          m_pWinsSurface == lpWnd->m_pWinsSurface &&
          gxIntersectRect(&rcTest, &rectWindow, &lpWnd->rectWindow))
        {
          lpWnd->GetWindowRegion(&pClipRegion);

          rc = pRegion->Subtract(pClipRegion);
          SAFE_RELEASE(pClipRegion);

          if(rc == RC_NULL) 
            return NULLREGION;
        }
        lpWnd = lpWnd->m_pNextWnd;
      }
    }
    else
    {
      while(lpWnd != NULL)
      {
        if(TEST_FLAG(lpWnd->m_uStyle, GXWS_VISIBLE) &&
          TEST_FLAG_NOT(lpWnd->m_uExStyle, GXWS_EX_TRANSPARENT) &&
          gxIntersectRect(&rcTest, &rectWindow, &lpWnd->rectWindow))
        {
          lpWnd->GetWindowRegion(&pClipRegion);

          rc = pRegion->Subtract(pClipRegion);
          SAFE_RELEASE(pClipRegion);

          if(rc == RC_NULL) 
            return NULLREGION;
        }
        lpWnd = lpWnd->m_pNextWnd;
      }
    }
  }

  if(dwFlags & GSR_CLIPCHILDREN)
  {
    LPGXWND lpWnd = m_pFirstChild;
    while(lpWnd != NULL)
    {
      if(lpWnd->m_uStyle & GXWS_VISIBLE)
      {
        lpWnd->GetWindowRegion(&pClipRegion);

        rc = pRegion->Subtract(pClipRegion);
        SAFE_RELEASE(pClipRegion);

        if(rc == RC_NULL) 
          return NULLREGION;
      }
      lpWnd = lpWnd->m_pNextWnd;
    }
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////
// 取当前Frame所使用的渲染目标
GXWindowsSurface* GXWnd::GetTopSurface()
{
  GXWnd* lpWnd = this;
  while( lpWnd->m_pWinsSurface == NULL )
  {
    if(gxIsTopLevelWindow(GXWND_HANDLE(lpWnd)))
      break;
    lpWnd = lpWnd->m_pParent;
  }
  return lpWnd->m_pWinsSurface;
}

void GXWnd::SetLayeredWindowStyle(GXBOOL bEnable)
{
  GXBOOL bLayered = (m_uExStyle & GXWS_EX_LAYERED) != 0;

  if(bLayered == bEnable)
    return;

  m_uExStyle = bEnable 
    ? m_uExStyle | GXWS_EX_LAYERED
    : m_uExStyle & (~GXWS_EX_LAYERED);

  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);

  m_pWinsSurface->SetExclusiveWnd(NULL, SEW_DONOTBLT);
  SAFE_RELEASE(m_pWinsSurface);

  lpStation->m_pDesktopWindowsMgr->AllocSurface(m_hSelf);
  if(m_pWinsSurface != NULL)
  {
    m_pWinsSurface->GenerateWindowsRgn(TRUE);
    lpStation->m_pDesktopWindowsMgr->InvalidateWndRegion(m_hSelf, NULL, FALSE);
  }
}

GXBOOL GXWnd::IsEnabled() const
{
  const GXWnd* lpWnd = this;
  ASSERT(lpWnd != NULL);

  do {
    if(TEST_FLAG(lpWnd->m_uStyle, GXWS_DISABLED)) {
      return FALSE;
    }
    lpWnd = lpWnd->m_pParent;
  } while(lpWnd != NULL);

  return TRUE;
}

GXBOOL GXWnd::IsVisible() const
{
  //
  // 如果 m_uStyle 没有 WS_VISIBLE 则 m_uState 一定没有 WIS_VISIBLE 属性
  //
  ASSERT((TEST_FLAG_NOT(m_uStyle, GXWS_VISIBLE) && TEST_FLAG_NOT(m_uState, WIS_VISIBLE)) ||
    TEST_FLAG(m_uStyle, GXWS_VISIBLE));

  return TEST_FLAG(m_uState, WIS_VISIBLE);
}

GXBOOL GXWnd::IsAncestorVisible() const
{
  if((m_uStyle & GXWS_VISIBLE) == 0 ||    // 隐藏窗口
    (m_uState & WIS_DESTROYTHISWND) != 0 ) {  // 准备销毁的窗口
    return FALSE;
  }

  // 判断父窗口可见性
  GXLPWND lpPWnd = m_pParent;
  LPGXWND lpDesktop = IntGetStationPtr()->lpDesktopWnd;

  while (lpPWnd != lpDesktop && lpPWnd != NULL)
  {
    CHECK_LPWND_VAILD(lpPWnd);

    if( (lpPWnd->m_uStyle & GXWS_VISIBLE) == 0 ||
      (lpPWnd->m_uState & WIS_DESTROYTHISWND) != 0 )
      return FALSE;
    lpPWnd = lpPWnd->m_pParent;
  }
  return TRUE;
}

void GXWnd::SetVisibleStateRecursive(GXBOOL bVisible)
{
  UPDATE_FLAG(m_uState, WIS_VISIBLE, bVisible);
  
  GXLPWND lpChild = m_pFirstChild;
  
  if( ! lpChild) {
    return;
  }
  
  do{
    const GXBOOL bChildVisible = TEST_FLAG(lpChild->m_uStyle, GXWS_VISIBLE) && bVisible;
  
    if(lpChild->m_pFirstChild) {
      lpChild->SetVisibleStateRecursive(bChildVisible);
    }
    else {
      UPDATE_FLAG(lpChild->m_uState, WIS_VISIBLE, bChildVisible);
    }
    lpChild = lpChild->m_pNextWnd;
  } while (lpChild);
}

GXBOOL GXWnd::ShowWindow( int nCmdShow )
{
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(this);
  GXBOOL      bActive   = TRUE;
  GXBOOL      bShow     = TRUE;

  //CHECK_LPWND_VAILD(lpWnd);

  GXLPWND pWndLost;
  switch(nCmdShow)
  {
  case GXSW_HIDE:        //Hides the window and activates another window.

    // 如果原来就是隐藏状态， 直接返回
    if(TEST_FLAG_NOT(m_uStyle, GXWS_VISIBLE)) {
      ASSERT(TEST_FLAG_NOT(m_uState, WIS_VISIBLE));
      return TRUE;
    }

    RESET_FLAG(m_uStyle, GXWS_VISIBLE);
    SetVisibleStateRecursive(FALSE);

    gxSendMessage(m_hSelf, GXWM_SHOWWINDOW, FALSE, NULL);
    pWndLost = lpStation->m_pMouseFocus;
    while(pWndLost != NULL)
    {
      if(pWndLost == this)
      {
        // 发送失去鼠标焦点信息
        gxSendMessageW(GXWND_HANDLE(lpStation->m_pMouseFocus), GXWM_NCMOUSELEAVE, NULL, NULL);
        gxSendMessageW(GXWND_HANDLE(lpStation->m_pMouseFocus), GXWM_MOUSELEAVE, NULL, NULL);
        lpStation->m_pMouseFocus = NULL;
      }
      pWndLost = pWndLost->m_pParent;
    }
    pWndLost = lpStation->m_pKeyboardFocus;
    while(pWndLost != NULL)
    {
      if(pWndLost == this)
      {
        // 发送失去键盘焦点信息
        gxSetFocus(NULL);
      }
      pWndLost = pWndLost->m_pParent;
    }
    pWndLost = lpStation->m_pCapture;
    while(pWndLost != NULL)
    {
      if(pWndLost == this)
      {
        // 发送失去窗口捕获信息
        gxReleaseCapture();
      }
      pWndLost = pWndLost->m_pParent;
    }
    bActive = FALSE;
    bShow = FALSE;
    break;

  case GXSW_SHOW:             //Activates the window and displays it in its current size and position. 
  case GXSW_SHOWDEFAULT:      //Sets the show state based on the GXSW_ flag specified in the STARTUPINFO structure passed to the CreateProcess function by the program that started the application. 
  case GXSW_SHOWNORMAL:       //Activates and displays a window. If the window is minimized or maximized, Windows restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
    bShow = TRUE;
    bActive = TRUE;
    break;

  case GXSW_SHOWNOACTIVATE:   //Displays a window in its most recent size and position. The active window remains active.
  case GXSW_SHOWNA:           //Displays the window in its current state. The active window remains active.
    bActive = FALSE;
    break;

  case GXSW_SHOWMINNOACTIVE:  //Displays the window as a minimized window. The active window remains active.
  case GXSW_SHOWMAXIMIZED:    //Activates the window and displays it as a maximized window.
    //GXSW_MAXIMIZE:          //Maximizes the specified window.
  case GXSW_SHOWMINIMIZED:    //Activates the window and displays it as a minimized window.
  case GXSW_RESTORE:          //Activates and displays the window. If the window is minimized or maximized, Windows restores it to its original size and position. An application should specify this flag when restoring a minimized window.
  case GXSW_MINIMIZE:         //Minimizes the specified window and activates the next top-level window in the Z order.
    // 不支持这些
    ASSERT(FALSE);
    return FALSE;
  }

  if(bActive == TRUE && gxIsTopLevelWindow(m_hSelf) == TRUE)
    SetActive();

  if(bShow == TRUE)
  {
    // 此时 m_uStyle 可能有 GXWS_VISIBLE 标志
    if(TEST_FLAG_NOT(m_uStyle, GXWS_VISIBLE) || TEST_FLAG_NOT(m_uState, WIS_VISIBLE)) {
      //ASSERT(TEST_FLAG_NOT(m_uState, WIS_VISIBLE));
      SET_FLAG(m_uStyle, GXWS_VISIBLE);
      
      // 祖先是显示状态，更新 WIS_VISIBLE     
      const GXBOOL bAncestorVisible = IsAncestorVisible();
      if(bAncestorVisible) {
        SetVisibleStateRecursive(TRUE);
      }
    }

    GXRECT rect;
    GXGetOverlappedRect(&rectWindow, &rect/*pCanvasTar->rcUpdate*/);
    gxSendMessage(m_hSelf, GXWM_SHOWWINDOW, TRUE, NULL);
    gxOffsetRect(&rect, -rect.left, -rect.top);
    GXInvalidateWindowRect(m_hSelf, &rect, FALSE);
  }

  if(lpStation->m_pCapture == NULL)
  {
    AnalyzeMouseMoveMsg(NULL, NULL);
  }
  GXWindowsSurface* pSurface = GetTopSurface();
  GRegion* pRegion = NULL;
  GetSystemRegion(GSR_PARENTCLIP|GSR_CLIPSIBLINGS|GSR_WINDOW, &pRegion);
  pSurface->InvalidateRegion(pRegion);
  pSurface->GenerateWindowsRgn(TRUE);
  SAFE_RELEASE(pRegion);

  return TRUE;
}


#ifdef _WINDOWS
GXBOOL GXWnd::gxGetCursorPos(GXLPPOINT pt)
{
  return (GXBOOL)(GetCursorPos((LPPOINT)pt) == TRUE);
}
#else
GXBOOL GXWnd::gxGetCursorPos(GXLPPOINT pt)
{
  return FALSE;
}

#endif
#endif // _DEV_DISABLE_UI_CODE