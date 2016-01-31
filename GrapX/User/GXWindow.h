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


//窗口之外外发光的距离
//#define FRAME_NC_GLOW_LEFT    0
//#define FRAME_NC_GLOW_TOP    0
//#define FRAME_NC_GLOW_RIGHT    0
//#define FRAME_NC_GLOW_BOTTOM  0

// 窗口之外边缘（不包括阴影）的距离
#define FRAME_NC_EDGE_CAPTION g_SystemMetrics[GXSM_CYCAPTION]
#define FRAME_NC_EDGE_LEFT    g_SystemMetrics[GXSM_CYFIXEDFRAME]
#define FRAME_NC_EDGE_TOP     g_SystemMetrics[GXSM_CXFIXEDFRAME]
#define FRAME_NC_EDGE_RIGHT   g_SystemMetrics[GXSM_CYFIXEDFRAME]
#define FRAME_NC_EDGE_BOTTOM  g_SystemMetrics[GXSM_CXFIXEDFRAME]

// 距离和
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

#define WIS_VISIBLE         0x00000010L   // WS_VISIBLE 会影响子窗口是否可见，WIS_VISIBLE标记当前窗口的实际状态

#define WIS_HASDESTROYWND   0x00000100L
#define WIS_DESTROYTHISWND  0x00000200L

#if 0
#define WIS_HASBEENDEL      0x00000400L // (Debug 模式) 已经被删除了
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
  // TODO: 工作站唯一
  //static GXDWORD      s_emCursorResult;
  static GXSprite    *  s_pCommonSpr;

  // 全局唯一
  static GXHICON      s_hCursorArrow;

#ifdef ENABLE_DYNMAIC_EFFECT
  static GXPOINT      s_ptPrevMousePos;
#endif
  GXHWND            m_hSelf;

  GXRECT            rectWindow;
  GXLPWND           m_pParent;
  GXLPWND           m_pFirstChild;
  GXLPWND           m_pPrevWnd;
  GXLPWND           m_pNextWnd;       // Next显示在this前面

  GXULONG           m_uStyle;
  GXULONG           m_uExStyle;
  GXULONG           m_uState;           // 内部使用的状态
  GXWCHAR*          m_pText;            // Frame 的文本信息
  GXHINSTANCE       m_hInstance;
  GXLPWNDCLSATOM    m_lpClsAtom;

  GXWNDPROC         m_lpWndProc;
  GXHMENU           m_pMenu;
  LPGXSCROLLBAR     m_lpVScrollBar;     // 如果允许，这个记录垂直滚动条的结构
  LPGXSCROLLBAR     m_lpHScrollBar;     // 水平滚动条的结构体

  GXHMENU           hSysMenu;           // 系统菜单
  GXLPVOID          m_dwUserData;
  GXHTHEME          m_hTheme;
  GXDWORD_PTR       m_CObj;             // C++ 对象
  GXLPVOID          m_pResponder;       // C++ 响应器
  //////////////////////////////////////////////////////////////////////////
  //
  // 只有TopLevel的Wnd才具有下面的数据结构
  // 其他子窗口都是NULL，或根据需要创建，
  // 这个类用于 D3Device 的渲染目标
  //
  GXWindowsSurface* m_pWinsSurface;
  //////////////////////////////////////////////////////////////////////////

  GRegion*          m_prgnUpdate;       // 自身的客户区的Update区域,没经过其他窗口裁剪; [屏幕坐标系]
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
  static  void      GetNonclientThickness (GXRECT* lpThickness, GXDWORD dwStyle, GXBOOL bMenu); // lpThickness 不是区域，而是上下左右Nonclient边缘厚度
  static  GXBOOL    AnalyzeMouseMoveMsg   (GXINOUT GXMSG* msg, GXLPPOINT pptMSWinClient);
          GXINT     gxGetWindowText       (GXLPWCHAR pwszString, GXINT nMaxCount);

          GXVOID    GetWindowRect         (GXRECT *lpRect) const;
          GXVOID    ClientToScreen        (GXLPPOINT lpPoints, GXUINT cPoints);
          GXVOID    ScreenToClient        (GXLPPOINT lpPoints, GXUINT cPoints);
          GXINT     MapWindowPoints       (GXLPWND lpWndFrom, GXLPPOINT lpPoints, GXUINT cPoints) const;


          // <这些是重构后比较规范的接口>
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
          int       GetSystemRegion       (GXDWORD dwFlags, GRegion** ppRegion);  // ppRegion 可以是一个现成的GRegion, 这样将重新指定, 也可以是一个NULL, 这样将会创建一个GRegion
          GXBOOL    GetBoundingRect       (GXBOOL bWindow, GXRECT* lprcOut);  // 返回的lprcOut是屏幕空间的 不论 bWindow 是否为 FALSE, 如果 lprcOut 和窗口区域一致就返回 TRUE
          LPGXWND   ChildWindowFromPoint  (LPGXPOINT lpPoint, GXLRESULT* lpHitTest, GXBOOL* bEnabled);
          void      SysUpdateWindow       (GRegion* prgnUpdate);
          void      UpdateWholeWindow     (GXWindowsSurface* pWinsSurface, GRegion* prgnPainted = NULL);
          //void      UpdateChildWindow     (GXWindowsSurface* pSurface, GXLPWND lpParent);
  GXWindowsSurface* GetTopSurface         ();
          void      SetLayeredWindowStyle (GXBOOL bEnable);
          GXBOOL    IsEnabled             () const;
          GXBOOL    IsVisible             () const;
          GXBOOL    ShowWindow            (int nCmdShow);
      // </这些是重构后比较规范的接口>

  static  GXBOOL    gxGetCursorPos        (GXLPPOINT pt);

          GXBOOL    InvalidateRect        (GXCONST GXRECT* lpRect, GXBOOL bErase);
          GXBOOL    InvalidateRgn         (GRegion* pRegion, GXBOOL bErase);
          GXHWND    GXGetTopLevel         ();

          GXBOOL    IsAncestorVisible     () const; // 检查祖先和自己的 WS_VISIBLE 属性

  inline  LPGXWND   GetDesktop            () const;

  GXVOID _GetSystemMinMaxInfo(GXLPMINMAXINFO lpmmi);
  GXVOID IntMoveChild(GXINT dx, GXINT dy);

  GXWnd();
  ~GXWnd();
private:
  void       SetClientUpdateRegion    (GRegion* prgnUpdate);  // 设置客户区更新,如果原来存在则合并
  GXLPWND    GetActiveOrder           (const GXWINDOWPOS* pWndPos) const;  // 根据窗口标志(WS_EX_TOPMOST)得到激活时插入的位置
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
// 参数: ppRegion    返回窗口的区域, 这个区域是窗口在屏幕坐标系的完整区域, 随窗口位置变化而偏移, 不会被屏幕区域或其他任何区域裁剪
// 返回: 区域的复杂度, 参考 RGNCOMPLEX 的定义

#endif // end of _FRAME_UI_BASIC_
#endif // _DEV_DISABLE_UI_CODE