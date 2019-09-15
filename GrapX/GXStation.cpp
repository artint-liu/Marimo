// 全局头文件
#include "GrapX.h"
#include "GXApp.h"
#include "clPathFile.h"
#include "thread/clMessageThread.h"
#include "User/GrapX.Hxx"

//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXUser.h"
#include "GrapX/GXKernel.h"
#include "GrapX/MOLogger.h"
#include "GrapX/MOConsoleStaff.h"

#include "GrapX/Platform.h"
#include "GrapX/gxDevice.h"
#include "User/gxMessage.hxx"
//#include "clMessageThread.h"
#include "GXStation.h"
#include "User/GXWindow.h"
#include "User/Win32Emu/_dpa.h"
#include <User/DesktopWindowsMgr.h>
#include "Utility/ConsoleAssistant.h"

#include "clSchedule.h"

#ifndef _DEV_DISABLE_UI_CODE
namespace DlgXM
{
  GXVOID FinalizeDefinationTable();
}
#endif // #ifndef _DEV_DISABLE_UI_CODE

GXINT GXSTATION::Enter()
{
  //m_pStationLocker->Lock();
  //m_nStationEnterCount++;
  //pGraphics->Begin();

  //return m_nStationEnterCount;
  return 0;
}

GXBOOL GXSTATION::TryEnter()
{
  return TRUE;
  //if(m_pStationLocker->TryLock() == TRUE)
  //{
  //  m_nStationEnterCount++;
  //  //pGraphics->Begin();
  //  return TRUE;
  //}
  //return FALSE;
}

GXINT GXSTATION::Leave()
{
  ////pGraphics->End();
  //m_nStationEnterCount--;
  //m_pStationLocker->Unlock();

  //return m_nStationEnterCount;
  return 0;
}

GXLRESULT GXSTATION::AppRender()
{
  //GXGraphics* pGraphics = lpPlatform->m_pApp->GetGraphicsUnsafe();
  //Enter();
  //pGraphics->Enter();
  lpPlatform->m_pApp->Render();
  //pGraphics->Leave();
  //Leave();
  return 0;
}
//////////////////////////////////////////////////////////////////////////
GXHRESULT GXSTATION::Initialize()
{
  ConsoleAssistant* pBaseStaff = new ConsoleAssistant();
  MORegisterConsoleStaff(pBaseStaff);

  if( ! lpPlatform || GXFAILED(lpPlatform->QueryFeature(GXMAKEFOURCC('L','O','G','R'), (GXVOID**)&m_pLogger))) {
    ASSERT(m_pLogger == NULL);
    MOCreateFileLoggerW(&m_pLogger, _CLTEXT("station.log"), FALSE);
  }

  GXBOOL bresult;
  pInstApp = new GXINSTANCE;
  bresult = pInstApp->Initialize(this, (HINSTANCE)(GXLONG _w64)GetWindowLong(hBindWin32Wnd, GXGWL_HINSTANCE));
  if( ! bresult) {
    return GX_FAIL;
  }

  // 初始化桌面窗口
  lpDesktopWnd          = new GXWnd;
  lpDesktopWnd->m_hSelf = GXWND_HANDLE(lpDesktopWnd);

  // 初始化桌面窗口使用的类
  lpDesktopWnd->m_lpClsAtom = new GXWNDCLSATOM;
  lpDesktopWnd->m_lpClsAtom->nRefCount     = 1;
  lpDesktopWnd->m_lpClsAtom->lpfnWndProc   = NULL; 
  lpDesktopWnd->m_lpClsAtom->cbClsExtra    = 0;
  lpDesktopWnd->m_lpClsAtom->cbWndExtra    = 0; 
  lpDesktopWnd->m_lpClsAtom->hStation      = GXSTATION_HANDLE(this);
  lpDesktopWnd->m_lpClsAtom->hIcon         = NULL;
  lpDesktopWnd->m_lpClsAtom->hCursor       = NULL;
  lpDesktopWnd->m_lpClsAtom->hbrBackground = NULL;
  lpDesktopWnd->m_lpClsAtom->szMenuName[0] = '\0'; 
  GXSTRCPYN(lpDesktopWnd->m_lpClsAtom->szClassName, _CLTEXT("GrapX Desktop Class"), 
    sizeof(lpDesktopWnd->m_lpClsAtom->szClassName) / sizeof(lpDesktopWnd->m_lpClsAtom->szClassName[0]));

  if(pGraphics) {
    m_pDesktopWindowsMgr = new DesktopWindowsMgr(this);
  }

  hClassDPA = gxDPA_Create(8);

  InlSetZeroT(m_HotKeyChain);

#ifdef _CL_SYSTEM_WINDOWS
  FILTERKEYS filter_key = {sizeof(FILTERKEYS)};
  SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &filter_key, 0);
  m_stFilterKeys.cbSize      = sizeof(GXFILTERKEYS);
  m_stFilterKeys.dwFlags     = filter_key.dwFlags;
  m_stFilterKeys.iWaitMSec   = filter_key.iWaitMSec;
  m_stFilterKeys.iDelayMSec  = filter_key.iDelayMSec;
  m_stFilterKeys.iRepeatMSec = filter_key.iRepeatMSec;
  m_stFilterKeys.iBounceMSec = filter_key.iBounceMSec;
#else
  m_stFilterKeys.cbSize      = sizeof(GXFILTERKEYS);
  m_stFilterKeys.dwFlags	     = 114;
  m_stFilterKeys.iWaitMSec   = 1000;
  m_stFilterKeys.iDelayMSec  = 1000;
  m_stFilterKeys.iRepeatMSec = 500;
  m_stFilterKeys.iBounceMSec	 = 0;
#endif


#ifdef _ENABLE_STMT
  STMT::CreateTask(1024 * 1024, UITask, NULL);
#else

#ifdef  REFACTOR_TIMER
  m_pShcedule = new clstd::Schedule;
#endif

  return GX_OK;
}

GXHRESULT GXSTATION::StartUserThread()
{
# ifdef REFACTOR_SYSQUEUE
  //pStation->m_pSysMsg->Start();
  m_pMsgThread = new GXUIMsgThread(lpPlatform);
  m_pSysMsg = new GrapX::Internal::SystemMessage(lpPlatform, m_pMsgThread->GetMessageSignal());
  m_pMsgThread->Start();
# else
  pStation->m_pMsgThread = new GXUIMsgThread(this);
  pStation->m_pMsgThread->Start();
# endif
  //static_cast<MessageThread*>(MessageThread::CreateThread((CLTHREADCALLBACK)UITask, this));
#endif // #ifdef _ENABLE_STMT
  //#endif // _DEV_DISABLE_UI_CODE
  return GX_OK;
}

GXHRESULT GXSTATION::Finalize()
{
  GXLPWNDCLSATOM lpClsAtom = lpDesktopWnd->m_lpClsAtom;
  delete (lpDesktopWnd);
  delete (lpClsAtom);
  delete (m_pMsgThread);
#ifdef REFACTOR_SYSQUEUE
  delete (m_pSysMsg);
#endif // #ifdef REFACTOR_SYSQUEUE

#ifdef REFACTOR_TIMER
  SAFE_DELETE(m_pShcedule);
#endif

  if(hClassDPA)
  {
    GXINT nClsCount = gxDPA_GetPtrCount(hClassDPA);
    for(GXINT i = 0; i < nClsCount; i++)
    {
      GXLPVOID lpClsAtom = gxDPA_GetPtr(hClassDPA, i);
      delete lpClsAtom;
    }
    gxDPA_Destroy(hClassDPA);
  }

  

#ifndef _DEV_DISABLE_UI_CODE
  DlgXM::FinalizeDefinationTable();
#endif // #ifndef _DEV_DISABLE_UI_CODE

  if( ! m_NamedPool.empty()) {
    CLOG_WARNING("Named Datapool does not completely released.\n");
    CLBREAK;
    //for(NamedInterfaceDict::iterator it = m_NamedPool.begin();
    //  it != m_NamedPool.end(); ++it)
    //{
    //  
    //}
    m_NamedPool.clear();
  }

  for(CmdDict::iterator it = m_CommandDict.begin();
    it != m_CommandDict.end(); ++it) {
      it->second.pStaff->Release();
  }
  m_CommandDict.clear();
  SAFE_RELEASE(m_pLogger);

  return GX_OK;
}

clStringW GXSTATION::ConvertAbsPathW(GXLPCWSTR szPath)
{
  if(clpathfile::IsRelative(szPath)) {
    clStringW strAbs;
    return clpathfile::CombinePath(strAbs, lpPlatform->GetRootDir(), szPath);
  }
  return szPath;
}


GXBOOL GXSTATION::SetCursorPos(GXLPPOINT lpCursor)
{
  if(m_ptCursor.x == lpCursor->x && m_ptCursor.y == lpCursor->y)
    return FALSE;
  m_ptCursor = *lpCursor;
  return TRUE;
}

GXLRESULT GXSTATION::SetCursor(GXWPARAM wParam, GXLPARAM lParam)
{
#if defined(_WIN32) || defined(_WINDOWS)
  const DWORD nHittest = LOWORD(lParam);  // hit-test code 

  if(hCursor != NULL && nHittest == HTCLIENT) {
    ::SetCursor(hCursor);
    return 0;       
  }
  return DefWindowProc(hBindWin32Wnd, WM_SETCURSOR, wParam, lParam);

#else
  return 0;
#endif // defined(_WIN32) || defined(_WINDOWS)
}
#ifndef _DEV_DISABLE_UI_CODE
GXLRESULT GXSTATION::CleanupRecord(GXHWND hWnd)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  ASSERT(lpWnd != NULL);

  // 删除所有相关的计时器
  gxKillTimer(hWnd, 0);

  // 如果拥有光标, 则销毁
  if(SysCaret.hWnd == hWnd)
  {
    gxDestroyCaret();
  }

  m_pBtnDown = NULL;

  // 如果涉及到焦点或者捕获窗口则清除
  if(m_pMouseFocus == lpWnd)  {
    // 发送失去鼠标焦点信息
    gxSendMessageW(GXWND_HANDLE(m_pMouseFocus), GXWM_NCMOUSELEAVE, NULL, NULL);
    gxSendMessageW(GXWND_HANDLE(m_pMouseFocus), GXWM_MOUSELEAVE, NULL, NULL);
    m_pMouseFocus = NULL;
  }

  if(m_pKeyboardFocus == lpWnd)  {
    // 发送失去键盘焦点信息
    gxSetFocus(NULL);
  }
  if(m_pCapture == lpWnd)  {
    // 发送失去窗口捕获信息
    gxReleaseCapture();
  }

  // 如果是顶层窗口, 则要清除激活列表里的记录
  if(gxIsTopLevelWindow(hWnd) == TRUE)
  {
    auto& aWnds = m_aActiveWnds;

    // 如果是激活窗口,则先更换掉它,使其非激活
    if(GetActiveWnd() == lpWnd)
    {
      for(auto it = aWnds.rbegin(); it != aWnds.rend(); ++it)
      {
        const GXULONG uStyle = (*it)->m_uStyle;
        if((uStyle & (GXWS_DISABLED | GXWS_VISIBLE)) == GXWS_VISIBLE &&
          (*it) != lpWnd)
        {
          (*it)->SetActive();
          break;
        }
      }
    }
    CleanupActiveWnd(lpWnd);
  }

  return GX_OK;
}

GXLRESULT GXSTATION::CleanupActiveWnd(GXLPWND lpWnd)
{
  for(auto it = m_aActiveWnds.begin();
    it != m_aActiveWnds.end(); ++it)
  {
    if((*it) == lpWnd)
    {
      m_aActiveWnds.erase(it);
      break;
    }
  }
  return 0;
}

GXLPWND GXSTATION::GetActiveWnd()
{
  for(auto it = m_aActiveWnds.rbegin();
    it != m_aActiveWnds.rend(); ++it)
  {
    const GXULONG uStyle = (*it)->m_uStyle;
    // TODO: 不可见的窗口或者被禁用的窗口还会在队列尾端吗? 应该向后放了吧?
    if((uStyle & (GXWS_VISIBLE | GXWS_DISABLED)) == GXWS_VISIBLE)
      return *it;
  }
  return NULL;
}
#endif // #ifndef _DEV_DISABLE_UI_CODE


GXWndMsg GXSTATION::DoDoubleClick(GXWndMsg msg, GXLPWND lpWnd)
{
  if( TEST_FLAG_NOT(lpWnd->m_lpClsAtom->style, GXCS_DBLCLKS) || (
    msg != GXWM_NCLBUTTONDOWN && msg != GXWM_NCMBUTTONDOWN && msg != GXWM_NCRBUTTONDOWN &&
    msg != GXWM_LBUTTONDOWN && msg != GXWM_MBUTTONDOWN && msg != GXWM_RBUTTONDOWN )) {
      return msg;
  }

  const int msg_delta = 2;
  STATIC_ASSERT (
    (GXWM_LBUTTONDBLCLK - GXWM_LBUTTONDOWN) == msg_delta &&
    (GXWM_MBUTTONDBLCLK - GXWM_MBUTTONDOWN) == msg_delta &&
    (GXWM_RBUTTONDBLCLK - GXWM_RBUTTONDOWN) == msg_delta &&
    (GXWM_NCLBUTTONDBLCLK - GXWM_NCLBUTTONDOWN) == msg_delta &&
    (GXWM_NCMBUTTONDBLCLK - GXWM_NCMBUTTONDOWN) == msg_delta &&
    (GXWM_NCRBUTTONDBLCLK - GXWM_NCRBUTTONDOWN) == msg_delta );



  if(m_eDownMsg == GXWM_NULL) {
    ASSERT(m_pBtnDown == NULL);
    m_pBtnDown      = lpWnd;
    m_eDownMsg      = msg;
    m_dwBtnDownTime = gxGetTickCount();
  }
  else {
    if(m_eDownMsg == msg && m_pBtnDown == lpWnd)
    {
      const GXDWORD dwTick = gxGetTickCount();
      if((dwTick - m_dwBtnDownTime) < gxGetDoubleClickTime())
      {
        msg = (GXWndMsg)(msg + msg_delta);
        m_eDownMsg = GXWM_NULL;
        m_pBtnDown = NULL;
      }
      else {
        TRACE("%d\n", dwTick - m_dwBtnDownTime);
        m_dwBtnDownTime = dwTick;
      }
    }
    else {
      m_eDownMsg = msg;
      m_pBtnDown = lpWnd;
      m_dwBtnDownTime = gxGetTickCount();
    }
  }
  return msg;
}

GXUpdateRate GXSTATION::GetUpdateRate() const
{
  return m_eUpdateRate;
}

void GXSTATION::SetLazyUpdate()
{
  SET_FLAG(m_dwFlags, GXSTATIONSTATEFLAG_LAZYUPDATE);
}

GXBOOL GXSTATION::CheckLazyUpdate()
{
  if(TEST_FLAG(m_dwFlags, GXSTATIONSTATEFLAG_LAZYUPDATE))
  {
    RESET_FLAG(m_dwFlags, GXSTATIONSTATEFLAG_LAZYUPDATE);
    return TRUE;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GXSTATION::ConvMessageX(GXLPMSG lpMsg, MOUIMSG &UIMsg)
{
  lpMsg->hwnd = (GXHWND)UIMsg.handle;
  lpMsg->message = UIMsg.message;
  lpMsg->wParam = UIMsg.wParam;
  lpMsg->lParam = UIMsg.lParam;
  lpMsg->time = UIMsg.dwTime;
  lpMsg->pt.x = UIMsg.xPos;
  lpMsg->pt.y = UIMsg.yPos;
  if(UIMsg.message == -1 || UIMsg.message == GXWM_QUIT) {
    lpMsg->message = GXWM_QUIT;
    lpMsg->lParam  = (GXLPARAM)UIMsg.handle; // 退出代码存在MessageThread中的handle里面
    lpMsg->hwnd    = NULL;
    //m_pMsgThread->PostQuitMessage((u32_ptr)UIMsg.handle);
  //  return FALSE;
  }
  return TRUE;
}

GXBOOL GXSTATION::ConvMessageX(GXLPMSG lpMsg, GXSYSMSG &SysMsg)
{
  lpMsg->hwnd = NULL; // 考虑去掉 SysMsg.handle;
  lpMsg->message = SysMsg.message & (~GXSysMessage_Bits); // GXWndMsg, GXSysMessage
  gxGetCursorPos(&lpMsg->pt);
  if(SysMsg.message >= GXSysMessage_MouseFirst && SysMsg.message <= GXSysMessage_MouseLast)
  {
    lpMsg->lParam = GXMAKELPARAM(lpMsg->pt.x, lpMsg->pt.y);
  }
  else
  {
    lpMsg->lParam = SysMsg.lParam;
  }
  lpMsg->wParam = SysMsg.wParam;
  lpMsg->time = SysMsg.dwTime;
  //if(SysMsg.message == GXSysMessage_Quit)
  //{
  //  lpMsg->message = GXWM_QUIT;
  //  m_pMsgThread->PostQuitMessage((u32_ptr)SysMsg.wParam);
  //  return FALSE;
  //}
  return TRUE;
}

GXBOOL GXDLLAPI IntAnalyzeMessage(GXINOUT GXMSG* msg);
//////////////////////////////////////////////////////////////////////////
// 消息按照以下顺序（优先级）处理：
//   Sent messages 
//   Posted messages 
//   Input (hardware) messages and system internal events 
//   Sent messages (again) 
//   WM_PAINT messages 
//   WM_TIMER messages 
GXBOOL GXSTATION::IntPeekMessage(GXLPMSG lpMsg, GXHWND hWnd, GXBOOL bRemoveMessage)
{
  MOUIMSG PostMsg;
  GXSYSMSG SysMsg;

  // Send messages
  m_pMsgThread->ResponseSentMessage();

  // Posted messages
  if(m_pMsgThread->PeekMessage(&PostMsg, hWnd, NULL, NULL, bRemoveMessage ? GXPM_REMOVE : FALSE))
  {
    return ConvMessageX(lpMsg, PostMsg);
  }

  // Input messages
  if(m_pSysMsg->Peek(&SysMsg, NULL, NULL, bRemoveMessage))
  {
    ConvMessageX(lpMsg, SysMsg);

    GXUINT uOriginMessage = lpMsg->message;
    if(IntAnalyzeMessage(lpMsg)) {
      // App消息先于UI消息处理, 保证得到正确的hUIHoverWnd信息
      if(bRemoveMessage)
      {
        AppHandle(uOriginMessage, lpMsg->wParam, lpMsg->lParam);
      }
      return TRUE;
      //break;
    }

    //if( ! ConvMessageX(lpMsg, SysMsg))
    //{
    //  return FALSE;
    //}
  }

  // Send messages, again
  m_pMsgThread->ResponseSentMessage();
#ifdef REFACTOR_TIMER
  // WM_TIMER messages
  GXDWORD dwDelta = UpdateTimerLoop(lpMsg);
  if(dwDelta == 0) {
    gxGetCursorPos(&lpMsg->pt);
    return TRUE;
  }
#endif // REFACTOR_TIMER
  return FALSE;
}

#ifdef REFACTOR_TIMER
GXDWORD GXSTATION::UpdateTimerLoop(GXLPMSG msg)
{
  clstd::TIMER t;
  GXDWORD dwTick = gxGetTickCount();
  GXDWORD dwDeltaTime;

  do {
    dwDeltaTime = m_pShcedule->GetTimer(dwTick, &t);
    if(dwDeltaTime) {
      return dwDeltaTime;
    }
  } while(TEST_FLAG(t.flags, clstd::TIMER::TimerFlag_FirstTime));

  msg->hwnd    = (GXHWND)t.handle;
  msg->message = GXWM_TIMER;
  msg->wParam  = t.id;
  msg->lParam  = (GXLPARAM)t.proc;
  msg->time    = dwTick;
  msg->pt.x    = 0;
  msg->pt.y    = 0;

  ASSERT(dwDeltaTime == 0);
  return 0;
}
#endif // REFACTOR_TIMER

//////////////////////////////////////////////////////////////////////////
//#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
//GXSTATION::GXSTATION(HWND hWnd, IGXPlatform* lpPlatform)
//#else
//GXSTATION::GXSTATION(IGXPlatform* lpPlatform)
//#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
GXSTATION::GXSTATION(const GXCREATESTATION* lpCreateStation)
  : dwMagic               (GXSTATION_MAGIC)
  , m_dwFlags             (GXSTATIONSTATEFLAG_DRAWDEBUGMSG)
  , m_pMsgThread          (NULL)
  , m_pSysMsg             (NULL)
  , m_hConsole            (NULL)
  , lpPlatform            (lpCreateStation->lpPlatform)
  , dwUIThreadId          (NULL)
  , pGraphics             (lpPlatform ? lpPlatform->m_pApp->GetGraphicsUnsafe() : NULL)
  , m_uFrameCount         (0)
  //, m_pRichFXMgr          (NULL)
  , hClassDPA             (NULL)
  , pInstApp              (NULL)
  , m_pMouseFocus         (NULL)
  , m_pKeyboardFocus      (NULL)
  , m_pCapture            (NULL)
  , m_pStockObject        (NULL)
  , m_HotKeyChain         (NULL)
  , m_pDesktopWindowsMgr  (NULL)
  , pBackDownSampTexA     (NULL)
  , pBackDownSampTexB     (NULL)
  , m_pLogger             (NULL)
  , m_pBtnDown            (NULL)
  , m_eDownMsg            (GXWM_NULL)
  , m_dwBtnDownTime       (NULL)
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  , hBindWin32Wnd     (lpCreateStation->hWnd)
  , hCursor           (NULL)
  , dwDoubleClickTime     (GetDoubleClickTime())
#else
  , dwDoubleClickTime     (500) // Windows 默认值
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef  REFACTOR_TIMER
  , m_pShcedule           (NULL)
#else
  , m_pTimerChain         (NULL)
#endif
{
  GXGRAPHICSDEVICE_DESC sGraphDesc;

  if(pGraphics)
  {
    pGraphics->GetDesc(&sGraphDesc);

    nWidth                       = sGraphDesc.BackBufferWidth;
    nHeight                      = sGraphDesc.BackBufferHeight;
    MonitorInfo.rcMonitor.left   = 0;
    MonitorInfo.rcMonitor.top    = 0;
    MonitorInfo.rcMonitor.right  = sGraphDesc.BackBufferWidth;
    MonitorInfo.rcMonitor.bottom = sGraphDesc.BackBufferHeight;
    MonitorInfo.rcWork           = MonitorInfo.rcMonitor;
  }
  else
  {
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
    MONITORINFO mi = {sizeof(MONITORINFO)};
    HMONITOR hMonitor = MonitorFromWindow(lpCreateStation->hWnd, MONITOR_DEFAULTTOPRIMARY);

    GetMonitorInfo(hMonitor, &mi);

    nWidth                = mi.rcMonitor.right;
    nHeight               = mi.rcMonitor.bottom;
    MonitorInfo.rcMonitor = *(GXRECT*)&mi.rcMonitor;
    MonitorInfo.rcWork    = *(GXRECT*)&mi.rcWork;
#else
    CLOG_ERROR("can not get monitor info.");
#endif
  }

  const GXDWORD dwAppStyle = lpCreateStation->lpAppDesc ? lpCreateStation->lpAppDesc->dwStyle : 0;
  if(TEST_FLAGS_ALL(dwAppStyle, GXADS_REALTIME)) {
    m_eUpdateRate = UpdateRate_RealTime;
  } else if(TEST_FLAGS_ALL(dwAppStyle, GXADS_ACTIVE)) {
    m_eUpdateRate = UpdateRate_Active;
  } else if(TEST_FLAGS_ALL(dwAppStyle, GXADS_LAZY)) {
     m_eUpdateRate = UpdateRate_Lazy;
  }

  m_ptCursor.x = 0;
  m_ptCursor.y = 0;

  InlSetZeroT(SysCaret);
}

//////////////////////////////////////////////////////////////////////////

GXHRESULT GXDLLAPI MORegisterConsoleStaff(IConsoleStaff* pStaff)
{
  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
  LPCSTAFFCAPSDESC pDesc = pStaff->GetCapacity();
  
  typedef GXSTATION::CmdDict CmdDict;
  CmdDict& sCmdDict = lpStation->m_CommandDict;

  CONSOLECMD Cmd;

  for(int i = 0; pDesc[i].szTitle != NULL; i++)
  {
    CmdDict::iterator it = sCmdDict.find(pDesc[i].szTitle);
    if(it == sCmdDict.end()) {
      Cmd.nIndex = i;
      Cmd.pStaff = pStaff;
      pStaff->AddRef();
      sCmdDict[pDesc[i].szTitle] = Cmd;
    }
  }

  return GX_OK;
}

GXHRESULT GXDLLAPI MOUnregisterConsoleStaff(IConsoleStaff* pStaff)
{
  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
  LPCSTAFFCAPSDESC pDesc = pStaff->GetCapacity();

  typedef GXSTATION::CmdDict CmdDict;
  CmdDict& sCmdDict = lpStation->m_CommandDict;

  for(int i = 0; pDesc[i].szTitle != NULL; i++)
  {
    CmdDict::iterator it = sCmdDict.find(pDesc[i].szTitle);
    if(it != sCmdDict.end() && it->second.pStaff == pStaff && it->second.nIndex == i) {
      it->second.pStaff->AddRef();
      sCmdDict.erase(it);
    }
  }

  return GX_OK;
}

GXBOOL GXDLLAPI MOExecuteFileW(GXLPCWSTR szFilename)
{
  clstd::File file;
  if( ! file.OpenExisting(szFilename)) {
    CLOG_ERRORW(_CLTEXT("MOExecuteFileW: Can not open specify file(%s).\n"), szFilename);
    return FALSE;
  }

  clBuffer* buf;
  if(file.MapToBuffer(&buf))
  {
    GXLPCWSTR lpBegin = (GXLPCWSTR)buf->GetPtr();
    GXLPCWSTR lpEnd = (GXLPCWSTR)((GXLPBYTE)buf->GetPtr() + buf->GetSize());
    while(lpBegin < lpEnd)
    {
      GXLPCWSTR lpReturn = strchrW(lpBegin, L'\r');
      GXLPCWSTR lpNewline = strchrW(lpBegin, L'\n');
      GXLPCWSTR lpLineEnd = clMin(lpReturn, lpNewline);
      clStringW strCommandLine(lpBegin, (size_t)(lpLineEnd - lpBegin));
      MOExecuteConsoleCmdW(strCommandLine);
      lpBegin = lpLineEnd;
      while(*lpBegin == '\r' || *lpBegin == '\n') {
        lpBegin++;
      }
    }
    delete buf;
  }
  return TRUE;
}

GXBOOL GXDLLAPI MOExecuteBatchCmdW(GXLPCWSTR* szCommand, int nCount)
{
  typedef GXSTATION::CmdDict CmdDict;

  clStringArrayW aArgs;

  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
  CmdDict& sCmdDict = lpStation->m_CommandDict;

  for(int i = 0; i < nCount; i++)
  {
    if(szCommand[i] == NULL) {
      break;
    }
    clstd::ParseCommandLine<GXWCHAR>(clStringW(szCommand[i]), aArgs);
    if(aArgs.empty()) {
      continue;
    }

    CmdDict::iterator it = sCmdDict.find(clStringA(aArgs[0]));
    if(it != sCmdDict.end()) {
      it->second.pStaff->Execute(it->second.nIndex, &aArgs.front(), (int)aArgs.size());
    }
    aArgs.clear();
  }
  return TRUE;
}

GXBOOL GXDLLAPI MOExecuteConsoleCmdW(GXLPCWSTR szCommand)
{
  clStringArrayW aArgs;
  clstd::ParseCommandLine<GXWCHAR>(clStringW(szCommand), aArgs);

  if(aArgs.empty()) {
    return FALSE;
  }

  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();
  typedef GXSTATION::CmdDict CmdDict;
  CmdDict& sCmdDict = lpStation->m_CommandDict;

  CmdDict::iterator it = sCmdDict.find(clStringA(aArgs[0]));
  if(it != sCmdDict.end()) {
    it->second.pStaff->Execute(it->second.nIndex, &aArgs.front(), (int)aArgs.size());
    return TRUE;
  }
  else {
    MOLogW(_CLTEXT("\"%s\" is not a command.\r\n"), (GXLPCWSTR)aArgs[0]);
    return FALSE;
  }
}

GXBOOL GXDLLAPI MOExecuteConsoleCmdA(GXLPCSTR szCommand)
{
  clStringW strW = szCommand;
  return MOExecuteConsoleCmdW(strW);
}

GXBOOL GXDLLAPI MOGetConsoleCmdInfoA(GXLPCSTR szCommand, int* nCmdIdx, IConsoleStaff** ppStaff)
{
  GXLPSTATION lpStation = GrapX::Internal::GetStationPtr();

  typedef GXSTATION::CmdDict CmdDict;
  CmdDict& sCmdDict = lpStation->m_CommandDict;

  CmdDict::iterator it = sCmdDict.find(szCommand);
  if(it == sCmdDict.end()) {
    return FALSE;
  }

  if(nCmdIdx) {
    *nCmdIdx = it->second.nIndex;
  }

  if(ppStaff) {
    *ppStaff = it->second.pStaff;
    it->second.pStaff->AddRef();
  }

  return TRUE;
}