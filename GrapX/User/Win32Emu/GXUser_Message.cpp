#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXCanvas.H>

// 私有头文件
#include <GXStation.H>
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>
#include "GrapX/GXUser.H"
#include "GrapX/GXKernel.H"
#include <User/GXWindow.h>
#include <GrapX/gxDevice.H>

#include "thread/clMessageThread.h"
#include "User/gxMessage.hxx"


GXBOOL IntMakeActionMessage(
  GXHWND    hWnd, 
  GXLPWND   lpWnd,
  GXINOUT GXMSG* msg, 
  GXWndMsg  message,
  GXWndMsg  msgNClient,   // 非客户区的消息
  GXWPARAM  wParam, 
  GXLPARAM  lParam, 
  GXBOOL    bActiveWnd)   // 以后需要标志扩展,可以考虑共用"SWP_*"或"SW_*" 标志
{
  ASSERT(hWnd != NULL);
  ASSERT(hWnd == GXWND_HANDLE(lpWnd));
  STATIC_ASSERT(GXWM_LBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCLBUTTONDOWN);
  STATIC_ASSERT(GXWM_MBUTTONDOWN - GXWM_MBUTTONDOWN + GXWM_NCMBUTTONDOWN == GXWM_NCMBUTTONDOWN);
  STATIC_ASSERT(GXWM_RBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCRBUTTONDOWN);
  STATIC_ASSERT(GXWM_LBUTTONUP - GXWM_LBUTTONUP + GXWM_NCLBUTTONUP == GXWM_NCLBUTTONUP);
  STATIC_ASSERT(GXWM_MBUTTONUP - GXWM_MBUTTONUP + GXWM_NCMBUTTONUP == GXWM_NCMBUTTONUP);
  STATIC_ASSERT(GXWM_RBUTTONUP - GXWM_LBUTTONUP + GXWM_NCLBUTTONUP == GXWM_NCRBUTTONUP);

  GXPOINT pt;
  GXPOINT ptClient;
  GXDWORD ht = HTERROR;


  pt.x = GXGET_X_LPARAM(lParam);
  pt.y = GXGET_Y_LPARAM(lParam);

  if(lpWnd && ! lpWnd->IsEnabled()) {
    ASSERT(ht == GXHTERROR);
    gxSendMessageW(hWnd, GXWM_SETCURSOR,(GXWPARAM)hWnd, GXMAKELPARAM(ht, message));
    return FALSE;
  }

  if(bActiveWnd) {
    lpWnd->SetActive();
  }

  ht = (GXDWORD)gxSendMessageW(hWnd, GXWM_NCHITTEST, wParam, GXMAKELPARAM(pt.x, pt.y));
  gxSendMessageW(hWnd, GXWM_SETCURSOR,(GXWPARAM)hWnd, GXMAKELPARAM(ht, message));

  if(ht != GXHTCLIENT && ht != GXHTNOWHERE)
  {
    //STATIC_ASSERT(GXWM_LBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCLBUTTONDOWN);
    //STATIC_ASSERT(GXWM_RBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCRBUTTONDOWN);
    GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);

    msg->hwnd    = hWnd;
    msg->message = lpStation->DoDoubleClick(msgNClient, lpWnd);//message - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN;
    msg->wParam  = ht;
    msg->lParam  = GXMAKELPARAM(pt.x, pt.y);
  }
  else
  {
    ptClient = pt;
    lpWnd->ScreenToClient(&ptClient, 1);

    ASSERT(message == GXWM_LBUTTONDOWN || message == GXWM_MBUTTONDOWN || message == GXWM_RBUTTONDOWN ||
      message == GXWM_LBUTTONUP || message == GXWM_MBUTTONUP || message == GXWM_RBUTTONUP);

    GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);
    msg->message = lpStation->DoDoubleClick((GXWndMsg)msg->message, lpWnd);
    msg->hwnd    = hWnd;
    msg->lParam  = GXMAKELPARAM(ptClient.x, ptClient.y);
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 返回值:  TRUE 后续窗口处理函数可以继续处理该消息
//      FALSE 后续窗口处理函数需要考虑到是否会改变该函数的处理结果
GXBOOL GXDLLAPI IntAnalyzeMessage(GXINOUT GXMSG* msg)
{
  GXPOINT      pt;
  GXPOINT      ptClient;

  const GXLPSTATION pStation        = GXSTATION_PTR(GXUIGetStation());
  const GXLPWND     pMouseFocus     = pStation->m_pMouseFocus;
  const GXLPWND     pCapture        = pStation->m_pCapture;
  const GXHWND      hMouseFocus     = GXWND_HANDLE(pMouseFocus);
  const GXHWND      hKeyboardFocus  = GXWND_HANDLE(pStation->m_pKeyboardFocus);
  const GXHWND      hCapture        = GXWND_HANDLE(pCapture);

  GXUINT message   = msg->message;
  GXWPARAM& wParam = msg->wParam;
  GXLPARAM& lParam = msg->lParam;

  //if(pMouseFocus && ((msg->message >= GXWM_KEYFIRST && msg->message <= GXWM_KEYLAST) ||
  //  (msg->message >= GXWM_MOUSEFIRST && msg->message <= GXWM_MOUSELAST)) && 
  //  ( ! pMouseFocus->IsEnabled())) {
  //    return FALSE;
  //}
  GXDWORD ht = GXHTERROR;
  GXBOOL  bRet = TRUE; // 这个初始值关系到后面的消息处理逻辑

  switch(message)
  {  
  case GXWM_MOUSEMOVE:
    pt.x = GXGET_X_LPARAM(lParam);
    pt.y = GXGET_Y_LPARAM(lParam);

    if(hCapture == NULL)
    {
      return GXWnd::AnalyzeMouseMoveMsg(msg, &pt);
    }
    else // s_pCaptureFrame != NULL
    {
      ASSERT(pCapture->IsEnabled());
      ASSERT((pCapture->m_uState & WIS_DESTROYTHISWND) == NULL);
      //ASSERT(pMouseFocus == pCapture);

      ptClient = pt;
      pStation->m_pMouseFocus = pCapture;

      ht = (GXDWORD)gxSendMessageW(hMouseFocus, GXWM_NCHITTEST, wParam, GXMAKELPARAM(pt.x, pt.y));
      gxSendMessageW(hMouseFocus, GXWM_SETCURSOR,(GXWPARAM)hMouseFocus, GXMAKELPARAM(ht, message));

      // Capture 时不发送 GXWM_SETCURSOR 消息！
      pMouseFocus->ScreenToClient(&ptClient, 1);

      msg->hwnd = hMouseFocus;
      msg->lParam = GXMAKELPARAM(ptClient.x, ptClient.y);
    }
    break;

  case GXWM_LBUTTONDOWN:
  case GXWM_MBUTTONDOWN:
  case GXWM_RBUTTONDOWN:
    {
      const GXWndMsg msgNoClient = (GXWndMsg)(message - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN);

      if(hCapture != NULL) {
        bRet = IntMakeActionMessage(hCapture, pCapture, msg, (GXWndMsg)message, msgNoClient, wParam, lParam, TRUE);
      }
      else if(hMouseFocus != NULL) {
        bRet = IntMakeActionMessage(hMouseFocus, pMouseFocus, msg, (GXWndMsg)message, msgNoClient, wParam, lParam, TRUE);
      }

      if( ! bRet) {
        return bRet;
      }
    }
    break;

  case GXWM_LBUTTONUP:
  case GXWM_MBUTTONUP:
  case GXWM_RBUTTONUP:
    {
      const GXWndMsg msgNoClient = (GXWndMsg)(message - GXWM_LBUTTONUP + GXWM_NCLBUTTONUP);

      if(hCapture != NULL) {
        bRet = IntMakeActionMessage(hCapture, pCapture, msg, (GXWndMsg)message, msgNoClient, wParam, lParam, FALSE);
      }
      else if(hMouseFocus != NULL) {
        bRet = IntMakeActionMessage(hMouseFocus, pMouseFocus, msg, (GXWndMsg)message, msgNoClient, wParam, lParam, FALSE);
      }

      if( ! bRet) {
        return bRet;
      }
    }
    break;

  case GXWM_MOUSEWHEEL:
    {
      //if(hMouseFocus != NULL)
      //  gxSendMessage(hMouseFocus, message, wParam, lParam);
      if(pMouseFocus && ! pMouseFocus->IsEnabled()) {
        return FALSE;
      }

      msg->hwnd = hMouseFocus;
    }
    break;

  case GXWM_CLOSE:
//#ifdef REFACTOR_SYSQUEUE
//    GXUIPostSysMessage(GXSysMessage_Quit, NULL, NULL);
//#else
    GXUIPostRootMessage(NULL, GXWM_QUIT, NULL, NULL);
//#endif // #ifdef REFACTOR_SYSQUEUE
    return FALSE;

  case GXWM_KEYDOWN:
    {
      LPGXHOTKEY lpHotKey = (LPGXHOTKEY)&pStation->m_HotKeyChain;
      while(lpHotKey != NULL)
      {
        if(lpHotKey->vk == wParam && gxIsWindowEnabled(lpHotKey->hWnd))
        {
          gxSendMessage(lpHotKey->hWnd, GXWM_HOTKEY, lpHotKey->id, GXMAKEWPARAM(lpHotKey->fsModifiers, lpHotKey->vk));
        }
        lpHotKey = lpHotKey->lpNext;
      }
    }
    // 没有 break !!!
  case GXWM_KEYUP:
  case GXWM_CHAR:
    {
      //if(hKeyboardFocus != NULL)
      //  return (GXBOOL)(gxSendMessage(hKeyboardFocus, message, wParam, lParam) != FALSE);

      if(pStation->m_pKeyboardFocus && ! pStation->m_pKeyboardFocus->IsEnabled()) {
        return FALSE;
      }

      msg->hwnd = hKeyboardFocus;
    }
    break;

  case GXWM_NCMOUSEMOVE:
    break;
    //case GXWM_CREATE:
    //  //s_hBindMSWnd = hWnd;
    //  break;
  }

  return TRUE;
}

inline void InlSetUIMSG(MOUIMSG& Msg, GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  GXPOINT ptCursor;
  gxGetCursorPos(&ptCursor);

  Msg.handle = hWnd;
  Msg.message = message;
  Msg.wParam = wParam;
  Msg.lParam = lParam;
  Msg.dwTime = gxGetTickCount();
  Msg.xPos = ptCursor.x;
  Msg.yPos = ptCursor.y;
}

//#ifdef REFACTOR_SYSQUEUE
//#else

clstd::Signal* GXUIMsgThread::GetMessageSignal()
{
  return &m_signal;
}

GXLRESULT GXUIMsgThread::SendRemoteMessage( GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam )
{
  MOUIMSG msg;
  GXLRESULT result = 0;
  InlSetUIMSG(msg, hWnd, message, wParam, lParam);

  ASSERT(gxGetCurrentThreadId() != m_dwThreadId);

  clstd::ScopedLocker __locker(m_RemoteLocker);

  m_pRemoteResult = &result;

  // 注意这里没有锁，消息处理线程根据m_pRemoteMsg有效性来处理
  // 所以m_pRemoteMsg要在所有工作准备好以后再设置
  m_pRemoteMsg = &msg;
  m_signal.Set();
  m_RemoteSignal.Wait();
  return result;
}

GXBOOL GXUIMsgThread::ResponseSentMessage() // 其它线程Send过来的消息
{
  if(_CL_NOT_(m_pRemoteMsg)) {
    return FALSE;
  }

  const GXLPWND lpWnd = GXWND_PTR(m_pRemoteMsg->handle);
  if(lpWnd && lpWnd->m_lpWndProc) {
    *m_pRemoteResult = lpWnd->m_lpWndProc((GXHWND)m_pRemoteMsg->handle, m_pRemoteMsg->message, 
      m_pRemoteMsg->wParam, m_pRemoteMsg->lParam);
  }
  else {
    *m_pRemoteResult= 0;
  }
  
  m_pRemoteResult = NULL;
  m_pRemoteMsg = NULL;
  m_RemoteSignal.Set();

  return TRUE;
}
//#endif // #ifdef REFACTOR_SYSQUEUE
//////////////////////////////////////////////////////////////////////////
namespace GrapX
{
  namespace Internal
  {
    GXLRESULT BroadcastToplevelWindowsMessage(GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
    {
      GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
      GXLPWND lpWnd = lpStation->lpDesktopWnd->m_pFirstChild;
      while(lpWnd) {
        gxSendMessage(lpWnd->m_hSelf, message, wParam, lParam);
        lpWnd = lpWnd->m_pNextWnd;
      }
      return 0l;
    }
  } // namespace Internal
} // namespace GrapX


//************************************
// Method:    gxSendMessageW
// Returns:   GXLRESULT
// Qualifier:
// Parameter: GXHWND hWnd
// Parameter: GXUINT Msg
// Parameter: GXWPARAM wParam
// Parameter: GXLPARAM lParam
//************************************
GXLRESULT GXDLLAPI gxSendMessageW(GXHWND hWnd, GXUINT Msg, GXWPARAM wParam, GXLPARAM lParam)
{
  //return hWnd->gxSendMessageW(Msg, wParam, lParam);
#ifdef ENABLE_SEH
  __try{
    return GXWND_PTR(hWnd)->m_lpWndProc(hWnd, Msg, wParam, lParam);
  }__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH){
  ASSERT(hWnd == NULL);
  return 0;
  }
#else
  if(hWnd == GXHWND_BROADCAST) {
    return GrapX::Internal::BroadcastToplevelWindowsMessage(Msg, wParam, lParam);
  }

  const GXLPWND lpWnd = GXWND_PTR(hWnd);  // FIXME: 其他线程访问这个Wnd的同时如果Wnd被释放有可能会出错
  if(lpWnd != NULL)
  {

#ifdef _DEBUG
    ASSERT(TEST_FLAG(lpWnd->m_uState, WIS_DESTROYTHISWND) == 0 ||
      Msg == GXWM_DESTROY || Msg == GXWM_NCDESTROY ||
      Msg == GXWM_KILLFOCUS || Msg == GXWM_NCMOUSELEAVE ||
      Msg == GXWM_MOUSELEAVE || Msg == GXWM_NOTIFY );
# ifdef WIS_HASBEENDEL
    ASSERT(TEST_FLAG(lpWnd->m_uState, WIS_HASBEENDEL) == 0);
# endif // WIS_HASBEENDEL
#endif // _DEBUG

    // TODO: Desktop 增加 ClassAtom 后使用 GXLPWND_STATION_PTR(lpWnd);
    GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd); // GXSTATION_PTR(GXUIGetStation()); 
    if(lpStation->dwUIThreadId == gxGetCurrentThreadId()) {
      return lpWnd->m_lpWndProc(hWnd, Msg, wParam, lParam);
    }
    else {
      return lpStation->m_pMsgThread->SendRemoteMessage(hWnd, Msg, wParam, lParam);
    }
  }
  return 0L;
#endif // ENABLE_SEH
}

//************************************
// Method:    gxSendDlgItemMessageW
// Returns:   GXLONG
// Qualifier:
// Parameter: GXHWND hDlg
// Parameter: int nIDDlgItem
// Parameter: GXUINT Msg
// Parameter: GXWPARAM wParam
// Parameter: GXLPARAM lParam
//************************************
GXLRESULT GXDLLAPI gxSendDlgItemMessageW(
  GXHWND hDlg,      // handle of dialog box
  int nIDDlgItem,   // identifier of control
  GXUINT Msg,       // message to send
  GXWPARAM wParam,  // first message parameter
  GXLPARAM lParam   // second message parameter
  )
{
  GXHWND hWnd = gxGetDlgItem(hDlg, nIDDlgItem);
  return hWnd == 0 ? 0 : gxSendMessage(hWnd, Msg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
//
// 分派消息
// 在消息队列中取出一条消息，如果是消息GrapX所在的Win32窗口消息，则调用解析
// 函数进行消息分派，如果是GrapX的窗口消息，则直接发送到相应窗口
//
GXLRESULT GXDLLAPI gxDispatchMessageW( const GXMSG *lpmsg )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());

  if((GXINT)lpmsg->message <= GXWM_NULL) {
    if(lpStation->m_pCapture) {
      gxPostMessageW(NULL, GXWM_CANCELMODE, NULL, NULL);
    }
    return FALSE;
  }
  else if(lpmsg->hwnd == NULL) {
    return FALSE;
  }

  GXLRESULT rval;
  //lpStation->Enter();

#if defined(_WIN32) || defined(_WINDOWS)  
  ASSERT((GXLPVOID)lpmsg->hwnd != (GXLPVOID)lpStation->hBindWin32Wnd);
#endif // #if defined(_WIN32) || defined(_WINDOWS)  

  //if(lpmsg->hwnd == NULL/*lpStation->hBindWin32Wnd*/)
  //  rval = IntAnalyzeMessage(lpmsg);
  //else
  rval = gxSendMessageW(((GXHWND)lpmsg->hwnd), lpmsg->message, lpmsg->wParam, lpmsg->lParam);

  //lpStation->Leave();
  return rval;
}

//************************************
// Method:    gxPostQuitMessage
// Returns:   void
// Qualifier:
// Parameter: int nExitCode
//************************************
void GXDLLAPI gxPostQuitMessage(int nExitCode)
{
  GXSTATION* pStation = GXSTATION_PTR(GXUIGetStation());
#if defined(_WIN32) || defined(_WINDOWS)
  PostMessage(pStation->hBindWin32Wnd, WM_CLOSE, NULL, NULL);
#else
  pStation->m_pMsgThread->PostQuitMessage(nExitCode);
#endif // #if defined(_WIN32) || defined(_WINDOWS)
}

//************************************
// Method:    gxPostMessageW
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXHWND hWnd
// Parameter: GXUINT Msg
// Parameter: GXWPARAM wParam
// Parameter: GXLPARAM lParam
//************************************
GXBOOL GXDLLAPI gxPostMessageW(
  GXHWND hWnd,      // handle of destination window
  GXUINT Msg,       // message to post 
  GXWPARAM wParam,  // first message parameter
  GXLPARAM lParam   // second message parameter
  )
{
  GXSTATION* pStation = GXSTATION_PTR(GXUIGetStation());
  MOUIMSG ThreadMsg;

  InlSetUIMSG(ThreadMsg, hWnd, Msg, wParam, lParam);

  GXBOOL bval = pStation->m_pMsgThread->PostMessage(&ThreadMsg); // FIXME: 退出时pStation->m_pMsgThread可能会为NULL
  return bval;
}

//************************************
// Method:    gxPeekMessageW
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXLPMSG lpMsg
// Parameter: GXHWND hWnd
// Parameter: GXUINT wMsgFilterMin
// Parameter: GXUINT wMsgFilterMax
// Parameter: GXUINT wRemoveMsg
//************************************
GXBOOL GXDLLAPI gxPeekMessageW(
  GXLPMSG lpMsg,        // pointer to structure for message
  GXHWND hWnd,          // handle to window
  GXUINT wMsgFilterMin, // first message
  GXUINT wMsgFilterMax, // last message
  GXUINT wRemoveMsg     // removal flags
  )
{
  // FIXME: PeekMessage 也能从队列中取出 hWnd 子窗口的消息
  MOUIMSG ThreadMsg;
  GXMSG msg;
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());

NEXT_PEEK:
  lpStation->m_pMsgThread->ResponseSentMessage();
  GXBOOL bVal = lpStation->m_pMsgThread->PeekMessage(&ThreadMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  if( ! bVal) {
    return bVal;
  }

  if(lpMsg == NULL) {
    lpMsg = &msg;
  }

  lpMsg->hwnd    = (GXHWND)ThreadMsg.handle;
  lpMsg->message = ThreadMsg.message;
  lpMsg->wParam  = ThreadMsg.wParam;
  lpMsg->lParam  = ThreadMsg.lParam;
  lpMsg->time    = ThreadMsg.dwTime;
  lpMsg->pt.x    = ThreadMsg.xPos;
  lpMsg->pt.y    = ThreadMsg.yPos;
  GXBOOL bval = IntAnalyzeMessage(lpMsg);
  if( ! bval) {
    if(TEST_FLAG_NOT(wRemoveMsg, GXPM_REMOVE)) {
      lpStation->m_pMsgThread->PeekMessage(&ThreadMsg, hWnd, wMsgFilterMin, wMsgFilterMax, GXPM_REMOVE);
    }
    goto NEXT_PEEK;
  }

  return bVal;
}

GXBOOL GXDLLAPI IntGetMessageX(GXLPMSG lpMsg, MOUIMSG &ThreadMsg, GXSTATION* lpStation)
{
  lpMsg->hwnd    = (GXHWND)ThreadMsg.handle;
  lpMsg->message = ThreadMsg.message;
  lpMsg->wParam  = ThreadMsg.wParam;
  lpMsg->lParam  = ThreadMsg.lParam;
  lpMsg->time    = ThreadMsg.dwTime;
  lpMsg->pt.x    = ThreadMsg.xPos;
  lpMsg->pt.y    = ThreadMsg.yPos;
  if(ThreadMsg.message == -1 || ThreadMsg.message == GXWM_QUIT) {
    lpMsg->message = GXWM_QUIT;
    lpStation->m_pMsgThread->PostQuitMessage((u32_ptr)ThreadMsg.handle);
    return FALSE;
  }
  return TRUE;
}

//************************************
// Method:    gxGetMessage
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXLPMSG lpMsg
// Parameter: GXHWND hWnd
//************************************
// 消息按照以下顺序（优先级）处理：
//   Sent messages 
//   Posted messages 
//   Input (hardware) messages and system internal events 
//   Sent messages (again) 
//   WM_PAINT messages 
//   WM_TIMER messages 
//
GXBOOL GXDLLAPI gxGetMessage(
  GXLPMSG lpMsg,
  GXHWND  hWnd)
{
  GXSTATION* lpStation = GXSTATION_PTR(GXUIGetStation());

#ifdef REFACTOR_SYSQUEUE
  if(lpStation->GetUpdateRate() == UpdateRate_Lazy)
  {
    CLBREAK;
  }
  else
  {
    while(_CL_NOT_(lpStation->IntPeekMessage(lpMsg, hWnd, TRUE)))
    {
      lpStation->AppRender();
    }

    if(lpMsg->message == GXWM_QUIT) {
      lpStation->m_pMsgThread->PostQuitMessage((u32_ptr)lpMsg->lParam);
      return FALSE;
    }
  }
  return TRUE;

#else
  MOUIMSG ThreadMsg;
  if(lpStation->GetUpdateRate() == UpdateRate_Lazy)
  {
    while(1)
    {
      // 在消息到来前，尽可能清理掉所有重绘
      while(lpStation->CheckLazyUpdate()) {
        lpStation->AppRender();
        if(lpStation->m_pMsgThread->PeekMessage(&ThreadMsg, hWnd, NULL, NULL, GXPM_REMOVE)) {
          break;
        }
      }

      lpStation->m_pMsgThread->ResponseSentMessage();
      lpStation->m_pMsgThread->GetMessage(&ThreadMsg);
      
      // 系统发送的重绘
      if(ThreadMsg.message == WM_PAINT)
      {
        lpStation->AppRender();
        continue;
      }

      if(!IntGetMessageX(lpMsg, ThreadMsg, lpStation)) {
        return FALSE;
      }

      GXUINT uOriginMessage = lpMsg->message;
      if(IntAnalyzeMessage(lpMsg)) {
        // App消息先于UI消息处理, 保证得到正确的hUIHoverWnd信息
        lpStation->AppHandle(uOriginMessage, lpMsg->wParam, lpMsg->lParam);
      }
      
      // 消息处理之后产生的重绘
      if(lpStation->CheckLazyUpdate()) {
        lpStation->AppRender();
      }

      break;
    }
  }
  else
  {
    while(1)
    {
      lpStation->m_pMsgThread->ResponseSentMessage();

      if(lpStation->m_pMsgThread->PeekMessage(&ThreadMsg, hWnd, NULL, NULL, GXPM_REMOVE))
      {
        if( ! IntGetMessageX(lpMsg, ThreadMsg, lpStation)) {
          return FALSE;
        }

        GXUINT uOriginMessage = lpMsg->message;
        if(IntAnalyzeMessage(lpMsg)) {
          // App消息先于UI消息处理, 保证得到正确的hUIHoverWnd信息
          lpStation->AppHandle(uOriginMessage, lpMsg->wParam, lpMsg->lParam);
          break;
        }

      }
      else {
        lpStation->AppRender();
      }
    }
  }
#endif
  return TRUE;
}

//************************************
// Method:    gxWaitMessage
// Returns:   GXBOOL
// Qualifier:
//************************************
GXBOOL GXDLLAPI gxWaitMessage()
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  if(lpStation->GetUpdateRate() == UpdateRate_Lazy)
  {
    return lpStation->m_pMsgThread->WaitMessage();
  }
  else
  {
    while(1)
    {
      GXBOOL bval = lpStation->m_pMsgThread->PeekMessage(NULL, NULL, NULL, NULL, GXPM_NOREMOVE);
      if(bval != 0)  // 实际上是消息队列里消息的数量
        return bval;
      lpStation->AppRender();
    }
  }
}

//************************************
// Method:    gxTranslateMessage
// Returns:   GXBOOL
// Qualifier:
// Parameter: const GXMSG * lpMsg
//************************************
GXBOOL GXDLLAPI gxTranslateMessage(
  const GXMSG *lpMsg   // address of structure with message
  )
{
  if(lpMsg->message == GXWM_KEYDOWN) {
    //gxPostMessageW(lpMsg->hwnd, GXWM_CHAR, );
  }
  return FALSE;
}

//************************************
// Method:    gxGetMessagePos
// Returns:   GXDWORD
// Qualifier:
//************************************
GXDWORD GXDLLAPI gxGetMessagePos()
{
#if defined(_WIN32) || defined(_WINDOWS)
  return GetMessagePos();
#else
  GXPOINT pt;
  gxGetCursorPos(&pt);
  return GXMAKELONG(pt.x, pt.y);
#endif // defined(_WIN32) || defined(_WINDOWS)
}

//************************************
// Method:    gxMessageBeep
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXUINT uType
//************************************
GXBOOL GXDLLAPI gxMessageBeep(
  GXUINT uType   // sound type  
  )
{
#if defined(_WIN32) || defined(_WINDOWS)
  return MessageBeep(uType);
#else
  return TRUE;
#endif // defined(_WIN32) || defined(_WINDOWS)
}

//************************************
// Method:    gxRegisterWindowMessageW
// Returns:   GXUINT 
// Qualifier:
// Parameter: GXLPCWSTR lpString
//************************************
GXUINT GXDLLAPI gxRegisterWindowMessageW(
  GXLPCWSTR lpString   // address of message string
  )
{
  ASSERT(FALSE);
  return NULL;
}

//************************************
// Method:    gxGetMessageTime
// Returns:   GXLONG
// Qualifier:
//************************************
GXLONG GXDLLAPI gxGetMessageTime()
{
#if defined(_WIN32) || defined(_WINDOWS)
  return GetMessageTime();
#else
  return gxGetTickCount();
#endif // defined(_WIN32) || defined(_WINDOWS)
}
#endif // #ifndef _DEV_DISABLE_UI_CODE