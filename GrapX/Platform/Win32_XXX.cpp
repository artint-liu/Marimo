// 全局头文件
#include "GrapX.H"
#include "User/GrapX.Hxx"

// 标准接口
#include "Include/GUnknown.H"
#include "Include/GXApp.H"
#include "Include/GXUser.H"
#include "Include/GResource.H"
#include "Include/GXGraphics.H"
#include "Include/MOLogger.h"

// 私有头文件
#include <clstd/User32Ex.H>
//#include "Console.h"
#include "gxDevice.H"
#include "Platform.h"
#include "Win32_XXX.h"

using namespace clstd;
void GXDestroyRootFrame();

inline void InlPrepareActionInfo(GXAPPACTIONINFO& Info, GXWPARAM wParam, LPARAM lParam, DWORD dwAction)
{
  Info.hUIHoverWnd = GXWND_HANDLE(IntGetStationPtr()->m_pMouseFocus);
  Info.dwAction    = dwAction;
  Info.Keys        = wParam;
  Info.Data        = 0;
  Info.ptCursor.x  = (GXLONG)((GXSHORT)(GXLOWORD(lParam)));
  Info.ptCursor.y  = (GXLONG)((GXSHORT)(GXHIWORD(lParam)));
}

//////////////////////////////////////////////////////////////////////////
IMOPlatform_Win32Base::IMOPlatform_Win32Base()
  : m_hInstance (NULL)
  , m_hWnd      (NULL)
  , m_pLogger   (NULL)
{
  // 获得当前路径
  GXWCHAR szWorkingPath[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, szWorkingPath);
  m_strRootDir = szWorkingPath;
}

GXHRESULT IMOPlatform_Win32Base::Finalize(GXINOUT GXGraphics** ppGraphics)
{
  SAFE_RELEASE(m_pLogger);
  m_aDropFiles.~clStringArrayW();
  return GX_OK;
}

//static int bTrack = 0;

LRESULT CALLBACK IMOPlatform_Win32Base::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  GXApp* pApp = (GXApp*)GetWindowLong(hWnd, 0);
  GXLPSTATION lpStation = NULL;
  GXGraphics* pGraphics = NULL;
  //if(bTrack && message != WM_USER + 57 && message != WM_USER + 58) {
  //  TRACE("message=%x\n", message);
  //}
  //if(message == WM_MOUSEMOVE && bTrack) {
  //  return 0;
  //}
#ifndef _DEV_DISABLE_UI_CODE
  if(pApp != NULL && message < WM_USER)  // 只派送系统消息
  {
    pGraphics = pApp->GetGraphicsUnsafe();
    if(pGraphics != NULL)
    {
      lpStation = IntGetStationPtr();  // 下面用到
      GXUIPostRootMessage(NULL, message, wParam, lParam);
    }
  }
#endif // _DEV_DISABLE_UI_CODE

  switch (message)
  {
  //case WM_USER + 57:
  //  bTrack = 1;
  //  break;
  //case WM_USER + 58:
  //  bTrack = 0;
  //  break;
#ifndef _DEV_DISABLE_UI_CODE
  case GXWM_SETCURSOR:
    if(lpStation != NULL)
      return lpStation->SetCursor(wParam, lParam);
    break;
    //return GXUISetCursor(wParam, lParam);
#endif // _DEV_DISABLE_UI_CODE
  case WM_LBUTTONDOWN:
    SetCapture(hWnd);
    break;
  case WM_LBUTTONUP:
    ReleaseCapture();
    break;
  case WM_CLOSE:
    {
      lpStation = IntGetStationPtr();
      lpStation->m_pMsgThread->PostQuitMessage(0);
      lpStation->m_pMsgThread->WaitThreadQuit(-1);
      SAFE_DELETE(lpStation->m_pMsgThread);
      DestroyWindow(hWnd);
    }
    break;
  case GXWM_EXITSIZEMOVE:
    {
      TRACE("GXWM_EXITSIZEMOVE\n");
      IntWin32ResizeWindow(hWnd, pApp);
      TRACE("GXWM_EXITSIZEMOVE ret\n");
    }
    break;

  case WM_SYSCOMMAND:
    {
      TRACE("WM_SYSCOMMAND DefWindowProc\n");
      LRESULT lval = DefWindowProc(hWnd, message, wParam, lParam);
      TRACE("WM_SYSCOMMAND DefWindowProc Ret\n");
      DWORD uCmdType = 0xfff0 & wParam;
      if(uCmdType == SC_MAXIMIZE || uCmdType == SC_RESTORE)
      {
        IntWin32ResizeWindow(hWnd, pApp);
      }
      return lval;
    }
  case WM_DROPFILES:
    {
      if(lpStation == NULL) {
        lpStation = IntGetStationPtr();
      }
      clStringArrayW& Strings = ((IMOPlatform_Win32Base*)lpStation->lpPlatform)->m_aDropFiles;
      GXWCHAR szFilename[MAX_PATH];
      int nFileCount = DragQueryFileW((HDROP)wParam, -1, NULL, 0);

      Strings.clear();
      for(int nFileIdx = 0; nFileIdx < nFileCount; nFileIdx++)
      {        
        DragQueryFileW((HDROP)wParam, nFileIdx, szFilename, MAX_PATH);
        Strings.push_back(szFilename);
      }
      DragFinish((HDROP)wParam);
      GXUIPostRootMessage(NULL, WM_DROPFILES, NULL, (GXLPARAM)&Strings);
    }
    break;
  case WM_SYSKEYDOWN:
    if(wParam == VK_RETURN)
    {
      IntWin32SwitchFullScreen(hWnd);
    }
    else {
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    break;
  case WM_CREATE:
    {
      DragAcceptFiles(hWnd, TRUE);
    }
    break;
  case GXWM_DESTROY:
    PostQuitMessage(0);
    break;
#ifdef _UPDATE_WIN32_CARET
  case WM_GX_CREATECARET:
    CreateCaret(hWnd, NULL, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));
    break;
  case WM_GX_DESTROYCARET:
    DestroyWindow(hWnd);
    break;
  case WM_GX_SHOWCARET:
    if((GXBOOL)lParam) {
      ShowCaret(hWnd);
    }
    else {
      HideCaret(hWnd);
    }
    break;
  case WM_GX_SETCARETPOS:
    SetCaretPos(GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));
    break;
#endif // #ifdef _UPDATE_WIN32_CARET
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

GXLRESULT IMOPlatform_Win32Base::AppHandle(GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  switch(message)
  {
  case WM_LBUTTONDOWN:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_LBUTTON);
      m_pApp->ActionBegin(&Info);
    }
    break;
  case WM_LBUTTONUP:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_LBUTTON);
      m_pApp->ActionEnd(&Info);
    }
    break;
  case WM_RBUTTONDOWN:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_RBUTTON);
      m_pApp->ActionBegin(&Info);
    }
    break;
  case WM_RBUTTONUP:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_RBUTTON);
      m_pApp->ActionEnd(&Info);
    }
    break;
  case WM_MBUTTONDOWN:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_MBUTTON);
      m_pApp->ActionBegin(&Info);
    }
    break;
  case WM_MBUTTONUP:
    {
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, wParam, lParam, GXMK_MBUTTON);
      m_pApp->ActionEnd(&Info);
    }
    break;
  case GXWM_MOUSEMOVE:
    {
      GXAPPACTIONINFO Info;
      DWORD dwKeys = wParam;  // 要保证 STATIC_ASSERT(MK_XXX == GXMK_XXX)

      InlPrepareActionInfo(Info, wParam, lParam, dwKeys);
      m_pApp->ActionMove(&Info);
    }
    break;
  case WM_MOUSEWHEEL:
    {      
      GXAPPACTIONINFO Info;

      InlPrepareActionInfo(Info, LOWORD(wParam), lParam, GXMK_WHEEL);
      Info.Data = (GXINT)(GXSHORT)HIWORD(wParam);
      ScreenToClient(m_hWnd, (LPPOINT)&Info.ptCursor);
      m_pApp->ActionMove(&Info);
    }
    break;
  case GXWM_KEYUP:
  case GXWM_KEYDOWN:
  case GXWM_CHAR:
    {
      if(wParam == '`')
      {
        GXUISwitchConsole();
        break;
      }
      GXAPPKEYINFO KeyInfo;
      KeyInfo.hUIFocusWnd = NULL;
      KeyInfo.dwAction    = message;
      KeyInfo.dwKey       = wParam;
      m_pApp->KeyMessage(&KeyInfo);
    }
    break;
  case WM_ACTIVATEAPP:
    {
      GXAPPACTIONINFO Info;
      Info.hUIHoverWnd = GXWND_HANDLE(IntGetStationPtr()->m_pMouseFocus);
      Info.dwAction    = GXMAKEFOURCC('A','T','V','P');
      Info.Keys        = wParam; // BOOL fActive
      Info.Data        = lParam; // DWORD dwThreadID
      Info.ptCursor.x  = 0;
      Info.ptCursor.y  = 0;
      m_pApp->ActionExtra(&Info);
    }
    break;
  case GXWM_DROPFILES:
    {
      // 这个消息会被解析成两个,一个是系统原始的,另一个是把文件名取出来后处理过的,我们要处理第二个
      if(lParam == NULL) {
        break;
      }
      GXAPPACTIONINFO Info;
      Info.hUIHoverWnd = GXWND_HANDLE(IntGetStationPtr()->m_pMouseFocus);
      Info.dwAction    = GXMAKEFOURCC('D','R','P','F');
      Info.Keys        = 0;
      Info.Data        = lParam;
      Info.ptCursor.x  = 0;
      Info.ptCursor.y  = 0;
      m_pApp->ActionExtra(&Info);
    }
    break;
  default:
    return -1;
  }
  return 0;
}

GXHRESULT IMOPlatform_Win32Base::MainLoop()
{
  MSG msg;
  while (1)
  {
    GXBOOL bval = GetMessage(&msg, NULL, 0, 0);
    if(bval <= 0)
      break;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return GX_OK;
}

GXHRESULT IMOPlatform_Win32Base::QueryFeature(GXDWORD dwFeatureCode, GXVOID** ppUnknown)
{
  switch(dwFeatureCode)
  {
  case GXMAKEFOURCC('W','D','P','F'): // Win32 Drop Files
    if(ppUnknown) {
      *ppUnknown = (GXVOID*)&m_aDropFiles;
    }
    return GX_OK;
  case GXMAKEFOURCC('H','W','N','D'):
    return (GXHRESULT)m_hWnd;
  case GXMAKEFOURCC('L','O','G','R'):
    *ppUnknown = m_pLogger;
    if(m_pLogger) {
      return m_pLogger->AddRef();
    }
    break;
  }
  return GX_FAIL;
}

GXLRESULT IMOPlatform_Win32Base::CreateWnd(GXLPWSTR lpClassName, WNDPROC pWndProc, GXAPP_DESC* pDesc, GXApp* pApp)
{
  WNDCLASSEX wcex;

  m_hInstance        = GetModuleHandle(NULL);
  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = GXCS_HREDRAW | GXCS_VREDRAW | CS_OWNDC;
  wcex.lpfnWndProc   = pWndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = sizeof(GXApp*);
  wcex.hInstance     = m_hInstance;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = lpClassName;
  wcex.hIconSm       = NULL;

  if(RegisterClassEx(&wcex) == 0)
  {
    return GX_FAIL;
  }

  const GXBOOL bSizeable = TEST_FLAG(pDesc->dwStyle, GXADS_SIZABLE);
  if(bSizeable && pDesc->nWidth == 0)
  {
    GXREGN regnNewWin;
    // 可调整窗口
    // 根据宽度决定是默认尺寸还是用户指定尺寸
    //if(pDesc->nWidth == 0) {
    gxSetRegn(&regnNewWin, GXCW_USEDEFAULT, 0, GXCW_USEDEFAULT, 0);
    //}
    //else {
    //  RECT rcWorkArea;
    //  SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcWorkArea, NULL);
    //  gxSetRegn(&regnNewWin, 
    //    rcWorkArea.left + (rcWorkArea.right - rcWorkArea.left - pDesc->nWidth) / 2, 
    //    rcWorkArea.top + (rcWorkArea.bottom - rcWorkArea.top - pDesc->nHeight) / 2, pDesc->nWidth, pDesc->nHeight);
    //}
    m_hWnd = CreateWindowEx(
      NULL, lpClassName, pDesc->lpName, WS_OVERLAPPEDWINDOW,
      regnNewWin.left, regnNewWin.top, regnNewWin.width, regnNewWin.height, NULL, NULL, 
      m_hInstance, NULL);
  }
  else
  {
    m_hWnd = CreateWindowEx(
      NULL, lpClassName, pDesc->lpName, bSizeable ? WS_OVERLAPPEDWINDOW : (WS_CAPTION | WS_SYSMENU),
      0, 0, 100, 100, NULL, NULL, m_hInstance, NULL);
    RECT rectWorkarea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkarea, 0);
    SetClientSize(m_hWnd, NULL, 
      (LONG)((rectWorkarea.right - rectWorkarea.left) - pDesc->nWidth) / 2 + rectWorkarea.left,
      (LONG)((rectWorkarea.bottom - rectWorkarea.top) - pDesc->nHeight) / 2 + rectWorkarea.top,
      pDesc->nWidth, pDesc->nHeight, NULL);
  }

  SetWindowLong(m_hWnd, 0, (GXLONG)pApp);

  if (m_hWnd == NULL)
  {
    return GX_FAIL;
  }

  return GX_OK;
}

GXLRESULT GXSTATION::AppHandle(GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  return (static_cast<IMOPlatform_Win32Base*>(lpPlatform))->
    AppHandle(message, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////////
i32 GXUIMsgThread::Run()
{
#ifndef _DEV_DISABLE_UI_CODE
  GXApp* pApp = (GXApp*)m_pPlatform->m_pApp;
  GXGraphics* pGraphics = pApp->GetGraphicsUnsafe();

  pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
  GXHRESULT hval = pApp->OnCreate();
  pGraphics->Activate(FALSE);

  if(GXFAILED(hval)) {
    return hval;
  }

  GXMSG gxmsg;
  while(gxGetMessage(&gxmsg, NULL))
  {
    // 在处理拖拽消息时, 这个函数可能要很久才返回
    gxDispatchMessageW(&gxmsg);
  }
#endif // _DEV_DISABLE_UI_CODE

  pApp->OnDestroy();

  // Station在UI线程内的清理
  GXDestroyRootFrame();
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
void ResolverMacroStringToD3DMacro(GXLPCSTR szMacros, GXDefinitionArray& aMacros)
{
  clStringArrayA aMacroStrings;
  GXDefinition m;
  ResolveString<clStringA, ch, clStringArrayA>(szMacros, ';', aMacroStrings);
  for(clStringArrayA::iterator it = aMacroStrings.begin();
    it != aMacroStrings.end(); ++it) {
      int nPos = it->Find('=');
      if(nPos < 0) {
        m.Name = *it;
        m.Value.Clear();
      }
      else {
        m.Name = it->Left(nPos);
        m.Value = it->Right(it->GetLength() - nPos - 1);
      }
      if(m.Name.IsNotEmpty()) {
        aMacros.push_back(m);
      }
  }
}
