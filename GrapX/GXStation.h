#ifndef _GRAPX_STATION_
#define _GRAPX_STATION_

namespace GrapX 
{
  class Graphics;
  class Texture;
}
class CAeroShader;
class CBlurShader;
class CSimpleShader;


//extern GXLPSTATION g_pCurStation;

//////////////////////////////////////////////////////////////////////////
//
//    层次关系
//
//////////////////////////////////////////////////////////////////////////
//
//  GXStation
//  {
//    Window
//    D3DDevice(D3DGraphics, 唯一,或者多个Station共享)
//    RootGXWindow
//    GXWindow(KeyFocus)
//    GXWindow(MouseFocus)
//    GXWindow(Capture)
//    GXInstance
//    {
//      GXWindow-s
//      {
//        GXWndCanvas
//        GXGDI
//      }
//    }
//    GXWindow(Prev KeyFocus)
//    GXWindow(Prev MouseFocus)
//    GXWindow(Prev Capture)
//  }
//  g_pCurStation


struct GXSTATION
{
  typedef clhash_map<clStringA, GUnknown*>  NamedInterfaceDict;
  typedef clhash_map<clStringA, CONSOLECMD> CmdDict;
  typedef GrapX::Internal::SystemMessage SystemMessage;
  GXDWORD             dwMagic;
  GXDWORD             m_dwFlags; // GXSTATIONSTATEFLAG_
  IGXPlatform*        lpPlatform;
  GXUpdateRate        m_eUpdateRate;
  GXDWORD             dwUIThreadId;     // GXUI不支持多线程Wnd, 这里记录了UI线程ID, 用于校验
  GXMONITORINFO       MonitorInfo;
#ifdef _WIN32
  HWND                hBindWin32Wnd;    // gx 所属的系统窗口
  HCURSOR             hCursor;
#endif // _WIN32
  GXINSTANCE*         pInstDll;
  GXINSTANCE*         pInstApp;
  GrapX::Graphics*    pGraphics;
  GXLPWND             lpDesktopWnd;
  //D3DPRESENT_PARAMETERS  d3dpp;
  GXUINT              nWidth;   // TODO: 和MonitorInfo重复
  GXUINT              nHeight;  // TODO: 
  GXCARET             SysCaret;      // 系统使用的光标
  GXHDPA              hClassDPA;
  GXUINT              dwDoubleClickTime; // 鼠标双击的响应时间

  GXLPWND             m_pMouseFocus;    // 只限于TopLevel Frame
  GXLPWND             m_pKeyboardFocus;
  GXLPWND             m_pCapture;
  STOCKOBJECT*        m_pStockObject;

  // 记录双击信息
  GXLPWND             m_pBtnDown;       // 第一次按下的窗口，一定带有CS_DBLCLKS属性
  GXWndMsg            m_eDownMsg;       // Null, LBtn, MBtn, RBtn, NCLBtn, NCMBtn, NCRBtn
  GXDWORD             m_dwBtnDownTime;

  GXPOINT             m_ptCursor;      // 储存的鼠标位置
  GXVOID*             m_HotKeyChain;
  GXULONG             m_uFrameCount;    // UI的帧计数器
  DesktopWindowsMgr*  m_pDesktopWindowsMgr;
  GXLPWND_LIST        m_aActiveWnds;
  //RichFXMgr*          m_pRichFXMgr;
  //GXDWORD             m_emCursorResult;

  GXHWND              m_hConsole;
  ILogger*            m_pLogger;
  NamedInterfaceDict  m_NamedPool;
  CmdDict             m_CommandDict;
  GXUIMsgThread*      m_pMsgThread;
  SystemMessage*      m_pSysMsg;

  GXFILTERKEYS        m_stFilterKeys;

#ifdef REFACTOR_TIMER
  clstd::Schedule*    m_pShcedule;
#else
  GXTIMERCHAIN*       m_pTimerChain;
#endif

#ifdef ENABLE_AERO
  GrapX::RenderTarget*       pBackDownSampTexA;    // 玻璃效果的缓冲纹理 - A
  GrapX::RenderTarget*       pBackDownSampTexB;    // 玻璃效果的缓冲纹理 - B
#endif // ENABLE_AERO
                                         //#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
                                         //  GXSTATION(HWND hWnd, IGXPlatform* lpPlatform);
                                         //#else
                                         //  GXSTATION(IGXPlatform* lpPlatform);
                                         //#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXSTATION(const GXCREATESTATION* lpCreateStation);
  GXHRESULT    Initialize     ();
  GXHRESULT    StartUserThread();
  GXHRESULT    Finalize       ();

  GXINT        Enter          ();
  GXBOOL       TryEnter       ();
  GXINT        Leave          ();

  GXBOOL      SetCursorPos    (GXLPPOINT lpCursor);
  GXLRESULT   SetCursor       (GXWPARAM wParam, GXLPARAM lParam); // Win32 窗口处理程序调用
  clStringW   ConvertAbsPathW (GXLPCWSTR szPath);

  //GXBOOL      RegisterNamedPool(GXLPCSTR szName, GXUI::Layout);
  //GXBOOL      RegisterNamedPool(GXLPCSTR szName);

  GXLRESULT    AppHandle      (GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  GXLRESULT    AppRender      ();
#ifndef _DEV_DISABLE_UI_CODE
  GXLRESULT   CleanupRecord   (GXHWND hWnd);
  GXLRESULT   CleanupActiveWnd(GXLPWND lpWnd);
  GXLPWND     GetActiveWnd();
#endif // #ifndef _DEV_DISABLE_UI_CODE
  GXWndMsg    DoDoubleClick   (GXWndMsg msg, GXLPWND lpWnd);
  //GXDWORD     GetAppDescStyle() const;
  GXUpdateRate GetUpdateRate() const;

  void        SetLazyUpdate   (); // 设置lazy update标记
  GXBOOL      CheckLazyUpdate (); // 检查并清空lazy update

  GXBOOL      ConvMessageX    (GXLPMSG lpMsg, GXSYSMSG &SysMsg);
  GXBOOL      ConvMessageX    (GXLPMSG lpMsg, MOUIMSG  &PostMsg);
  GXBOOL      IntPeekMessage   (GXLPMSG lpMsg, GXHWND hWnd, GXBOOL bRemoveMessage);
#ifdef REFACTOR_TIMER
  GXDWORD     UpdateTimerLoop(GXLPMSG msg);
#endif // REFACTOR_TIMER
};

#endif // _GRAPX_STATION_