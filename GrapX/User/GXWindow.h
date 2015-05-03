#ifndef _DEV_DISABLE_UI_CODE
#ifndef _FRAME_UI_BASIC_
#define _FRAME_UI_BASIC_


//#define UNPACKPARAM(des16l, des16h, src32)    \
//  __asm  mov    eax, src32          \
//  __asm  movsx  ecx, ax            \
//  __asm  sar    eax, 16            \
//  __asm  mov    des16h, eax          \
//  __asm  mov    des16l, ecx

class CGraphics;
class GXWnd;
class CSprite;
class GXWindowsSurface;
class GRegion;


GXVOID GXDLLAPI GXGetOverlappedRect(GXLPCRECT IN, GXLPRECT OUT);
//GXVOID GXDLLAPI GXGetOverlappedRegion(LPCREGN IN, LPREGN OUT);

GXVOID _GXDrawFrameEdge(GXWndCanvas*, GXLPRECT, GXDWORD, GXDWORD, GXDWORD);
GXVOID _GXDrawBackground(GXWndCanvas*, GXLPRECT);
GXHRESULT _gxInitializeCommonSprite(GXGraphics* pGraphics);
GXHRESULT _gxReleaseCommonSprite();


//����֮���ⷢ��ľ���
//#define FRAME_NC_GLOW_LEFT    0
//#define FRAME_NC_GLOW_TOP    0
//#define FRAME_NC_GLOW_RIGHT    0
//#define FRAME_NC_GLOW_BOTTOM  0

// ����֮���Ե����������Ӱ���ľ���
#define FRAME_NC_EDGE_CAPTION g_SystemMetrics[GXSM_CYCAPTION]
#define FRAME_NC_EDGE_LEFT    g_SystemMetrics[GXSM_CYFIXEDFRAME]
#define FRAME_NC_EDGE_TOP     g_SystemMetrics[GXSM_CXFIXEDFRAME]
#define FRAME_NC_EDGE_RIGHT   g_SystemMetrics[GXSM_CYFIXEDFRAME]
#define FRAME_NC_EDGE_BOTTOM  g_SystemMetrics[GXSM_CXFIXEDFRAME]

// �����
//#define FRAME_NC_LEFT      (FRAME_NC_GLOW_LEFT    + FRAME_NC_EDGE_LEFT  )
//#define FRAME_NC_TOP      (FRAME_NC_GLOW_TOP    + FRAME_NC_EDGE_TOP    )
//#define FRAME_NC_RIGHT      (FRAME_NC_GLOW_RIGHT  + FRAME_NC_EDGE_RIGHT  )
//#define FRAME_NC_BOTTOM      (FRAME_NC_GLOW_BOTTOM  + FRAME_NC_EDGE_BOTTOM  )




// Window Internal State
#define WIS_ENDDIALOG       0x40000000L
#define WIS_ISDIALOG        0x20000000L
#define WIS_ISDIALOGEX      0x10000000L
//#define WIS_CLOSE           0x00000002L
//#define WIS_MINBUTTON       0x00000004L
//#define WIS_MAXBUTTON       0x00000008L
//#define WIS_RESTOREBUTTON   0x00000010L
//#define WIS_MASK            0x0000001FL

#define WIS_VISIBLE         0x00000010L   // WS_VISIBLE ��Ӱ���Ӵ����Ƿ�ɼ���WIS_VISIBLE��ǵ�ǰ���ڵ�ʵ��״̬

#define WIS_HASDESTROYWND   0x00000100L
#define WIS_DESTROYTHISWND  0x00000200L

#if 0
#define WIS_HASBEENDEL      0x00000400L // (Debug ģʽ) �Ѿ���ɾ����
#endif

#if defined(_DEBUG) && defined(WIS_HASBEENDEL)
# define CHECK_LPWND_VAILD(_LPWND)    ASSERT(TEST_FLAG_NOT(_LPWND->m_uState, WIS_HASBEENDEL))
#else
# define CHECK_LPWND_VAILD(_LPWND)
#endif // defined(_DEBUG) && defined(WIS_HASBEENDEL)

# define CHECK_HWND_VAILD(_HWND)  IS_PTR(_HWND)


GXVOID GXDestroyRootFrame();
//GXHRESULT GXDLLAPI GXRenderRootFrame();
typedef GXHANDLE GXHTHEME;
class GXSprite;
class GXWnd
{
public:
  // TODO: ����վΨһ
  //static GXDWORD      s_emCursorResult;
  static GXSprite    *  s_pCommonSpr;

  // ȫ��Ψһ
  static GXHICON      s_hCursorArrow;

#ifdef ENABLE_DYNMAIC_EFFECT
  static GXPOINT      s_ptPrevMousePos;
#endif
  GXHWND            m_hSelf;

  GXRECT            rectWindow;
  GXLPWND           m_pParent;
  GXLPWND           m_pFirstChild;
  GXLPWND           m_pPrevWnd;
  GXLPWND           m_pNextWnd;       // Next��ʾ��thisǰ��

  GXULONG           m_uStyle;
  GXULONG           m_uExStyle;
  GXULONG           m_uState;           // �ڲ�ʹ�õ�״̬
  GXWCHAR*          m_pText;            // Frame ���ı���Ϣ
  GXHINSTANCE       m_hInstance;
  GXLPWNDCLSATOM    m_lpClsAtom;

  GXWNDPROC         m_lpWndProc;
  GXHMENU           m_pMenu;
  LPGXSCROLLBAR     m_lpVScrollBar;     // ������������¼��ֱ�������Ľṹ
  LPGXSCROLLBAR     m_lpHScrollBar;     // ˮƽ�������Ľṹ��

  GXHMENU           hSysMenu;           // ϵͳ�˵�
  GXLPVOID          m_dwUserData;
  GXHTHEME          m_hTheme;
  GXDWORD_PTR       m_CObj;             // C++ ����
  GXLPVOID          m_pResponder;       // C++ ��Ӧ��
  //////////////////////////////////////////////////////////////////////////
  //
  // ֻ��TopLevel��Wnd�ž�����������ݽṹ
  // �����Ӵ��ڶ���NULL���������Ҫ������
  // ��������� D3Device ����ȾĿ��
  //
  GXWindowsSurface* m_pWinsSurface;
  //////////////////////////////////////////////////////////////////////////

  GRegion*          m_prgnUpdate;       // ����Ŀͻ�����Update����,û�����������ڲü�; [��Ļ����ϵ]
//#ifdef _ENABLE_STMT
//  static GXUINT  s_idxMsgStackTop;
//  static GXUINT  s_idxMsgStackBottom;
//  static GXMSG  s_msg[64];
//#endif // #ifdef _ENABLE_STMT
  static  GXHRESULT CreateDesktop         (GXLPSTATION lpStation);
  static  GXHRESULT DestroyDesktop        (GXLPSTATION lpStation);

  static  GXHRESULT InitializeSysCursor   ();
  static  GXVOID    ReleaseSysCursor      ();

  static  GXVOID    OutDebugMsg           ();

          GXHWND    SetParent             (GXHWND hParent);
  static  void      GetNonclientThickness (GXRECT* lpThickness, GXDWORD dwStyle, GXBOOL bMenu); // lpThickness �������򣬶�����������Nonclient��Ե���
  static  GXBOOL    AnalyzeMouseMoveMsg   (GXINOUT GXMSG* msg, GXLPPOINT pptMSWinClient);
          GXINT     gxGetWindowText       (GXLPWCHAR pwszString, GXINT nMaxCount);

          GXVOID    GetWindowRect         (GXRECT *lpRect) const;
          GXVOID    ClientToScreen        (GXLPPOINT lpPoints, GXUINT cPoints);
          GXVOID    ScreenToClient        (GXLPPOINT lpPoints, GXUINT cPoints);
          GXINT     MapWindowPoints       (GXLPWND lpWndFrom, GXLPPOINT lpPoints, GXUINT cPoints) const;


          // <��Щ���ع���ȽϹ淶�Ľӿ�>
          GXLPWND   SetActive             ();
          //GXLPWND   GetActive             ();

          GXLPWND   GetForeground         ();
          GXBOOL    SetForeground         ();

          GXINT     Scroll                (GXINT dx, GXINT dy, GXCONST GXRECT *prcScroll, GXCONST GXRECT * prcClip, GXHRGN hrgnUpdate, GXLPRECT prcUpdate, GXUINT flags);
          GXBOOL    SetPos                (GXHWND hWndInsertAfter, int x, int y, int cx, int cy, GXUINT uFlags);
          GXBOOL    Move                  (int x, int y, int cx, int cy, GXBOOL bRepaint);
          GXBOOL    Size                  (GXINT x, GXINT y, GXINT nWidth, GXINT nHeight);
          GXBOOL    MoveOnly              (GXINT x, GXINT y);
          //GXVOID    MoveWindow            (GXINT x, GXINT y, GXINT nWidth, GXINT nHeight, GXBOOL bRepaint);

          int       GetWindowRegion       (GRegion** ppRegion);
          int       GetSystemRegion       (GXDWORD dwFlags, GRegion** ppRegion);  // ppRegion ������һ���ֳɵ�GRegion, ����������ָ��, Ҳ������һ��NULL, �������ᴴ��һ��GRegion
          GXBOOL    GetBoundingRect       (GXBOOL bWindow, GXRECT* lprcOut);  // ���ص�lprcOut����Ļ�ռ�� ���� bWindow �Ƿ�Ϊ FALSE, ��� lprcOut �ʹ�������һ�¾ͷ��� TRUE
          LPGXWND   ChildWindowFromPoint  (LPGXPOINT lpPoint, GXLRESULT* lpHitTest, GXBOOL* bEnabled);
          void      SysUpdateWindow       (GRegion* prgnUpdate);
          void      UpdateWholeWindow     (GXWindowsSurface* pWinsSurface, GRegion* prgnPainted = NULL);
          //void      UpdateChildWindow     (GXWindowsSurface* pSurface, GXLPWND lpParent);
  GXWindowsSurface* GetTopSurface         ();
          void      SetLayeredWindowStyle (GXBOOL bEnable);
          GXBOOL    IsEnabled             () const;
          GXBOOL    IsVisible             () const;
          GXBOOL    ShowWindow            (int nCmdShow);
      // </��Щ���ع���ȽϹ淶�Ľӿ�>

  static  GXBOOL    gxGetCursorPos        (GXLPPOINT pt);

          GXBOOL    InvalidateRect        (GXCONST GXRECT* lpRect, GXBOOL bErase);
          GXBOOL    InvalidateRgn         (GRegion* pRegion, GXBOOL bErase);
          GXHWND    GXGetTopLevel         ();

          GXBOOL    IsAncestorVisible     () const; // ������Ⱥ��Լ��� WS_VISIBLE ����

  inline  LPGXWND   GetDesktop            () const;

  GXVOID _GetSystemMinMaxInfo(GXLPMINMAXINFO lpmmi);
  GXVOID IntMoveChild(GXINT dx, GXINT dy);

  GXWnd();
  ~GXWnd();
private:
  void       SetClientUpdateRegion    (GRegion* prgnUpdate);  // ���ÿͻ�������,���ԭ��������ϲ�
  GXLPWND    GetActiveOrder           (const GXWINDOWPOS* pWndPos) const;  // ���ݴ��ڱ�־(WS_EX_TOPMOST)�õ�����ʱ�����λ��
  GXLPWND    GetLastSiblingNoTopMost  ();
  GXLPWND    GetLastSibling           ();
  void       SetVisibleStateRecursive (GXBOOL bVisible);

};

//////////////////////////////////////////////////////////////////////////
//
// Inline function
//
inline LPGXWND GXWnd::GetDesktop() const
{
  return GXLPWND_STATION_PTR(this)->lpDesktopWnd;
}

//////////////////////////////////////////////////////////////////////////
// int  GXWnd::GetWindowRegion(GRegion** ppRegion);
// ����: ppRegion    ���ش��ڵ�����, ��������Ǵ�������Ļ����ϵ����������, �洰��λ�ñ仯��ƫ��, ���ᱻ��Ļ����������κ�����ü�
// ����: ����ĸ��Ӷ�, �ο� RGNCOMPLEX �Ķ���

#endif // end of _FRAME_UI_BASIC_
#endif // _DEV_DISABLE_UI_CODE