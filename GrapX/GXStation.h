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
//    ��ι�ϵ
//
//////////////////////////////////////////////////////////////////////////
//
//  GXStation
//  {
//    Window
//    D3DDevice(D3DGraphics, Ψһ,���߶��Station����)
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
  GXDWORD             dwUIThreadId;     // GXUI��֧�ֶ��߳�Wnd, �����¼��UI�߳�ID, ����У��
  GXMONITORINFO       MonitorInfo;
#ifdef _WIN32
  HWND                hBindWin32Wnd;    // gx ������ϵͳ����
  HCURSOR             hCursor;
#endif // _WIN32
  GXINSTANCE*         pInstDll;
  GXINSTANCE*         pInstApp;
  GrapX::Graphics*    pGraphics;
  GXLPWND             lpDesktopWnd;
  //D3DPRESENT_PARAMETERS  d3dpp;
  GXUINT              nWidth;   // TODO: ��MonitorInfo�ظ�
  GXUINT              nHeight;  // TODO: 
  GXCARET             SysCaret;      // ϵͳʹ�õĹ��
  GXHDPA              hClassDPA;
  GXUINT              dwDoubleClickTime; // ���˫������Ӧʱ��

  GXLPWND             m_pMouseFocus;    // ֻ����TopLevel Frame
  GXLPWND             m_pKeyboardFocus;
  GXLPWND             m_pCapture;
  STOCKOBJECT*        m_pStockObject;

  // ��¼˫����Ϣ
  GXLPWND             m_pBtnDown;       // ��һ�ΰ��µĴ��ڣ�һ������CS_DBLCLKS����
  GXWndMsg            m_eDownMsg;       // Null, LBtn, MBtn, RBtn, NCLBtn, NCMBtn, NCRBtn
  GXDWORD             m_dwBtnDownTime;

  GXPOINT             m_ptCursor;      // ��������λ��
  GXVOID*             m_HotKeyChain;
  GXULONG             m_uFrameCount;    // UI��֡������
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
  GrapX::RenderTarget*       pBackDownSampTexA;    // ����Ч���Ļ������� - A
  GrapX::RenderTarget*       pBackDownSampTexB;    // ����Ч���Ļ������� - B
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
  GXLRESULT   SetCursor       (GXWPARAM wParam, GXLPARAM lParam); // Win32 ���ڴ���������
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

  void        SetLazyUpdate   (); // ����lazy update���
  GXBOOL      CheckLazyUpdate (); // ��鲢���lazy update

  GXBOOL      ConvMessageX    (GXLPMSG lpMsg, GXSYSMSG &SysMsg);
  GXBOOL      ConvMessageX    (GXLPMSG lpMsg, MOUIMSG  &PostMsg);
  GXBOOL      IntPeekMessage   (GXLPMSG lpMsg, GXHWND hWnd, GXBOOL bRemoveMessage);
#ifdef REFACTOR_TIMER
  GXDWORD     UpdateTimerLoop(GXLPMSG msg);
#endif // REFACTOR_TIMER
};

#endif // _GRAPX_STATION_