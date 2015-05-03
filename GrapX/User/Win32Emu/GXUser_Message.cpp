#ifndef _DEV_DISABLE_UI_CODE
// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
#include <Include/GUnknown.H>
#include <Include/GResource.H>
#include <Include/GRegion.H>
#include <Include/GXGraphics.H>
#include <Include/GXCanvas.H>

// ˽��ͷ�ļ�
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>
#include "Include/GXUser.H"
#include "Include/GXKernel.H"
#include <User/GXWindow.h>
#include <GrapX/gxDevice.H>



GXBOOL IntMakeActionMessage(
  GXHWND    hWnd, 
  GXLPWND   lpWnd,
  GXINOUT GXMSG* msg, 
  GXWndMsg  message,
  GXWndMsg  msgNClient,      // �ǿͻ�������Ϣ
  GXWPARAM  wParam, 
  GXLPARAM  lParam, 
  GXBOOL    bActiveWnd) // �Ժ���Ҫ��־��չ,���Կ��ǹ���"SWP_*"��"SW_*" ��־
{
  ASSERT(hWnd != NULL);
  ASSERT(hWnd == GXWND_HANDLE(lpWnd));
  STATIC_ASSERT(GXWM_LBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCLBUTTONDOWN);
  STATIC_ASSERT(GXWM_RBUTTONDOWN - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN == GXWM_NCRBUTTONDOWN);
  STATIC_ASSERT(GXWM_LBUTTONUP - GXWM_LBUTTONUP + GXWM_NCLBUTTONUP == GXWM_NCLBUTTONUP);
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
    msg->hwnd    = hWnd;
    msg->message = msgNClient;//message - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN;
    msg->wParam  = ht;
    msg->lParam  = GXMAKELPARAM(pt.x, pt.y);
  }
  else
  {
    ptClient = pt;
    lpWnd->ScreenToClient(&ptClient, 1);

    msg->hwnd   = hWnd;
    msg->lParam = GXMAKELPARAM(ptClient.x, ptClient.y);
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ����ֵ:  TRUE �������ڴ����������Լ�����������Ϣ
//      FALSE �������ڴ���������Ҫ���ǵ��Ƿ��ı�ú����Ĵ������
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
  GXDWORD ht = HTERROR;
  GXBOOL  bRet = TRUE; // �����ʼֵ��ϵ���������Ϣ�����߼�

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

      // Capture ʱ������ GXWM_SETCURSOR ��Ϣ��
      pMouseFocus->ScreenToClient(&ptClient, 1);

      msg->hwnd = hMouseFocus;
      msg->lParam = GXMAKELPARAM(ptClient.x, ptClient.y);
    }
    break;

  case GXWM_RBUTTONDOWN:
  case GXWM_LBUTTONDOWN:
    {
      const GXWndMsg msgNoClient = (GXWndMsg)(message - GXWM_LBUTTONDOWN + GXWM_NCLBUTTONDOWN);

      if(hCapture != NULL)
      {
        bRet = IntMakeActionMessage(hCapture, pCapture, msg, (GXWndMsg)message, 
          msgNoClient, wParam, lParam, TRUE);
      }
      else if(hMouseFocus != NULL)
      {
        bRet = IntMakeActionMessage(hMouseFocus, pMouseFocus, msg, (GXWndMsg)message, 
          msgNoClient, wParam, lParam, TRUE);
      }

      if( ! bRet) {
        return bRet;
      }
    }
    break;

  case GXWM_RBUTTONUP:
  case GXWM_LBUTTONUP:
    {
      const GXWndMsg msgNoClient = (GXWndMsg)(message - GXWM_LBUTTONUP + GXWM_NCLBUTTONUP);

      if(hCapture != NULL)
      {
        bRet = IntMakeActionMessage(hCapture, pCapture, msg, (GXWndMsg)message, 
          msgNoClient, wParam, lParam, FALSE);
      }
      else if(hMouseFocus != NULL)
      {
        bRet = IntMakeActionMessage(hMouseFocus, pMouseFocus, msg, (GXWndMsg)message, 
          msgNoClient, wParam, lParam, FALSE);
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
    GXUIPostRootMessage(NULL, GXWM_QUIT, NULL, NULL);
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
    // û�� break !!!
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

GXLRESULT GXUIMsgThread::SendRemoteMessage( GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam )
{
  // TODO: ���û�н��й����߳�ͬʱ��UI�߳�SendMessage�Ĳ���
  MOUIMSG Msg;
  GXLRESULT result;
  InlSetUIMSG(Msg, hWnd, message, wParam, lParam);

  m_locker.Lock();
  while(m_pResult) {
    m_locker.Unlock();
    m_SendingWaiter.Wait();
    m_locker.Lock();
  }
  m_pSendingMsg = &Msg;
  m_pResult = &result;
  m_locker.Unlock();

  m_signal.Set();
  m_SendingWaiter.Wait();
  return result;
}

GXBOOL GXUIMsgThread::CallSentMessageProc()
{
  m_locker.Lock();
  if(m_pResult == NULL) {
    m_locker.Unlock();
    return FALSE;
  }

  const GXLPWND lpWnd = GXWND_PTR(m_pSendingMsg->handle);
  if(lpWnd && lpWnd->m_lpWndProc) {
    *m_pResult = lpWnd->m_lpWndProc((GXHWND)m_pSendingMsg->handle, m_pSendingMsg->message, 
      m_pSendingMsg->wParam, m_pSendingMsg->lParam);
  }
  else {
    *m_pResult = 0;
  }
  m_pResult = NULL;
  m_pSendingMsg = NULL;
  m_locker.Unlock();
  m_SendingWaiter.Set();
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
GXLRESULT IntBroadcastToplevelWindowsMessage(GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLPSTATION lpStation = IntGetStationPtr();
  GXLPWND lpWnd = lpStation->lpDesktopWnd->m_pFirstChild;
  while(lpWnd) {
    gxSendMessage(lpWnd->m_hSelf, message, wParam, lParam);
    lpWnd = lpWnd->m_pNextWnd;
  }
  return 0l;
}


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
    return IntBroadcastToplevelWindowsMessage(Msg, wParam, lParam);
  }

  const GXLPWND lpWnd = GXWND_PTR(hWnd);  // FIXME: �����̷߳������Wnd��ͬʱ���Wnd���ͷ��п��ܻ����
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

    // TODO: Desktop ���� ClassAtom ��ʹ�� GXLPWND_STATION_PTR(lpWnd);
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
// ������Ϣ
// ����Ϣ������ȡ��һ����Ϣ���������ϢGrapX���ڵ�Win32������Ϣ������ý���
// ����������Ϣ���ɣ������GrapX�Ĵ�����Ϣ����ֱ�ӷ��͵���Ӧ����
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
  //GXPOINT ptCursor;
  //gxGetCursorPos(&ptCursor);

  //ThreadMsg.handle = hWnd;
  //ThreadMsg.message = Msg;
  //ThreadMsg.wParam = wParam;
  //ThreadMsg.lParam = lParam;
  //ThreadMsg.dwTime = gxGetTickCount();
  //ThreadMsg.xPos = ptCursor.x;
  //ThreadMsg.yPos = ptCursor.y;
  InlSetUIMSG(ThreadMsg, hWnd, Msg, wParam, lParam);

  GXBOOL bval = pStation->m_pMsgThread->PostMessage(&ThreadMsg); // FIXME: �˳�ʱpStation->m_pMsgThread���ܻ�ΪNULL
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
  // FIXME: PeekMessage Ҳ�ܴӶ�����ȡ�� hWnd �Ӵ��ڵ���Ϣ
  MOUIMSG ThreadMsg;
  GXMSG msg;
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());

NEXT_PEEK:
  lpStation->m_pMsgThread->CallSentMessageProc();
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

//************************************
// Method:    gxGetMessage
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXLPMSG lpMsg
// Parameter: GXHWND hWnd
//************************************
// messages are processed in the following order: 
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
  MOUIMSG ThreadMsg;
  GXSTATION* lpStation = GXSTATION_PTR(GXUIGetStation());

  while(1)
  {
    lpStation->m_pMsgThread->CallSentMessageProc();

    if(lpStation->m_pMsgThread->PeekMessage(&ThreadMsg, hWnd, NULL, NULL, GXPM_REMOVE))
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

      GXUINT uOriginMessage = lpMsg->message;
      if(IntAnalyzeMessage(lpMsg)) {
        // App��Ϣ����UI��Ϣ����, ��֤�õ���ȷ��hUIHoverWnd��Ϣ
        lpStation->AppHandle(uOriginMessage, lpMsg->wParam, lpMsg->lParam);
        break;
      }
    }
    else
    {
      lpStation->AppRender();
    }
  }

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
  while(1)
  {
    GXBOOL bval = lpStation->m_pMsgThread->PeekMessage(NULL, NULL, NULL, NULL, GXPM_NOREMOVE);
    if(bval != 0)  // ʵ��������Ϣ��������Ϣ������
      return bval;
    lpStation->AppRender();
  }
}

//************************************
// Method:    gxTranslateMessage
// Returns:   GXBOOL
// Qualifier:
// Parameter: GXCONST GXMSG * lpMsg
//************************************
GXBOOL GXDLLAPI gxTranslateMessage(
  GXCONST GXMSG *lpMsg   // address of structure with message
  )
{
  TRACE_UNACHIEVE("=== gxTranslateMessage ===\n");
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