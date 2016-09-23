//////////////////////////////////////////////////////////////////////////
//
// GRAPX 内部结构定义文件
// 包含了GX不对外使用的内部数据结构定义
// Liu.Chenglong

#ifndef _GRAPHICS_X_USER_INTERFACE_GLOBAL_DEFINE_
#define _GRAPHICS_X_USER_INTERFACE_GLOBAL_DEFINE_

//#include <vector>

//////////////////////////////////////////////////////////////////////////
//
// 类声明
//
class GXWnd;
class GXImage;
class GUnknown;
class GTexture;
class GXFont;
class GXCanvas;
class GXGraphics;
class GRegion;
class DesktopWindowsMgr;
class RichFXMgr;
class IGXPlatform;
class GXUIMsgThread;
//class clLocker;
class GXApp;
class GXConsole;
class ILogger;
class IConsoleStaff;

namespace GXUI
{
  class Layout;
}

struct STOCKOBJECT;

#define GXSTATION_MAGIC      ((GXDWORD)0x54535847)  // "GXST"
#define TRACE_UNACHIEVE(...)      //TRACE    // 跟踪未实现的函数
#define ENABLE_AERO

typedef struct __tagGXHOTKEY
{
  __tagGXHOTKEY*  lpNext;
  GXHWND          hWnd;
  GXINT           id;
  GXUINT          fsModifiers;
  GXUINT          vk;
}GXHOTKEY, *LPGXHOTKEY, *GXLPHOTKEY;

struct CONSOLECMD
{
  int nIndex;
  IConsoleStaff* pStaff;
};


//#define IS_IDENTIFY(_ID_)  (((GXDWORD_PTR)_ID_) <= 0xFFFF)
//#define IS_PTR(_PTR_)  (((GXDWORD_PTR)_PTR_) > 0xFFFF && ((GXDWORD_PTR)_PTR_) < 0x80000000)  // 32 bits mode
#define WINSTYLE_HASCAPTION(STYLE)  (((STYLE & GXWS_CAPTION)) == GXWS_CAPTION && (STYLE & GXWS_CHILD) == 0)
#define WINSTYLE_HASMENU(LPWND, LPROOT)  (LPWND->m_pMenu != NULL &&                \
  (LPWND->m_uStyle & GXWS_CHILD) == NULL &&          \
  (LPWND->m_pParent == NULL || LPWND->m_pParent == LPROOT))

#define GET_LAST_WINDOW(_LPWND)   while((_LPWND)->m_pNextWnd != NULL) { (_LPWND) = (_LPWND)->m_pNextWnd; }

//
// GRESCRIPTDESC 使用的内部命令
//
enum ResCmd
{
  RC_LostDevice         = GXMAKEFOURCC('L','S','T','D'),  // DX9 Lost D3DDevice
  RC_ResetDevice        = GXMAKEFOURCC('R','S','T','D'),  // DX9 Reset D3DDevice
  RC_ResizeDevice       = GXMAKEFOURCC('R','S','Z','D'),
  RC_CanvasUniformSize  = GXMAKEFOURCC('C','C','V','U'),  // Count of bytes for canvas uniform
  RC_MarkCanvsUniform   = GXMAKEFOURCC('M','C','V','U'),  // Mark canvas uniform
};

// TODO: 增加宏开关，可以使用索引的版本
// TODO: 去掉这个枚举，统一使用"RESTYPE_*"定义
enum RESOURCE_CATEGORY_CODE // TODO: 这个要改为外部声明
{
  RCC_FontA             = GXMAKEFOURCC('F','T','I','A'),
  RCC_Texture           = GXMAKEFOURCC('T','X','F','W'),
  RCC_TextureName       = GXMAKEFOURCC('T','E','X','N'),
  RCC_Image             = GXMAKEFOURCC('I','M','F','W'),
  RCC_SpriteW           = GXMAKEFOURCC('S','P','F','W'),
  RCC_Shader            = GXMAKEFOURCC('S','D','R','A'),
  RCC_MaterialDesc      = GXMAKEFOURCC('M','T','L','I'),
  RCC_VertexDecl        = GXMAKEFOURCC('V','T','X','E'),
  RCC_RasterizerState   = GXMAKEFOURCC('R','A','Z','T'),
  RCC_BlendState        = GXMAKEFOURCC('B','L','D','T'),
  RCC_DepthStencilState = GXMAKEFOURCC('D','P','S','T'),
  RCC_Canvas3D          = GXMAKEFOURCC('C','N','V','3'),
};



//////////////////////////////////////////////////////////////////////////
//
// GX Object
//
// GXDC Object 枚举
enum GXGDIOBJTYPE
{
  GXGDIOBJ_NULL   = 0,
  GXGDIOBJ_PEN    = 1    ,// OBJ_PEN
  GXGDIOBJ_BRUSH  = 2    ,// OBJ_BRUSH
  GXGDIOBJ_DC     = 3    ,// OBJ_DC
  //OBJ_METADC      ,// 4
  //OBJ_PAL          ,// 5
  GXGDIOBJ_FONT   = 6      ,// OBJ_FONT
  GXGDIOBJ_BITMAP = 7    ,// OBJ_BITMAP
  GXGDIOBJ_REGION = 8    ,// OBJ_REGION
  //OBJ_METAFILE        ,// 9
  GXGDIOBJ_MEMDC  = 9    ,// OBJ_MEMDC
  //OBJ_EXTPEN          ,// 11
  //OBJ_ENHMETADC       ,// 12
  //OBJ_ENHMETAFILE     ,// 13
  //OBJ_COLORSPACE      ,// 14
};

typedef struct __tagGXGDIDC *LPGXGDIDC;

struct GXGDIOBJ
{
  GXGDIOBJTYPE  emObjType;
  LPGXGDIDC    pDC;
  GXGDIOBJ(GXGDIOBJTYPE _emObjType, LPGXGDIDC _pDC);
  GXGDIOBJ(GXGDIOBJTYPE _emObjType);
  GXGDIOBJ();
};

// GX Pen
typedef struct __tagGXGDIPEN : GXGDIOBJ
{
  GXUINT      uStyle;
  GXUINT      uWidth;
  GXCOLORREF    crColor;
} GXGDIPEN;

// GX Brush
typedef struct __tagGXGDIBRUSH : GXGDIOBJ
{
  GXUINT      uStyle;
  GXCOLORREF  crColor;
  GXLONG_PTR  hHatch;
} GXGDIBRUSH;

// GX Font
typedef struct __tagGXGDIFONT : GXGDIOBJ
{
  GXFont*    lpFont;
} GXGDIFONT;

// GX Region
struct GXGDIREGION : GXGDIOBJ
{
  GXRECT    rect;
  GRegion*  lpRegion;
  GXGDIREGION    ();
  GXGDIREGION    (GRegion* pRegion);
  ~GXGDIREGION  ();
};

// GX Bitmap
typedef struct __tagGXGDIBITMAP : GXGDIOBJ
{
  GXLONG      bmWidth; 
  GXLONG      bmHeight; 
  GXLONG      bmWidthBytes; 
  GXWORD      bmPlanes; 
  GXWORD      bmBitsPixel; 
  GXLPVOID    bmBits; 
  GXImage*    pImage;    
}GXGDIBITMAP;

typedef GXGDIPEN  *LPGXGDIPEN, *GXLPGDIPEN;
typedef GXGDIBRUSH  *LPGXGDIBRUSH, *GXLPGDIBRUSH;
typedef GXGDIFONT  *LPGXGDIFONT, *GXLPGDIFONT;
typedef GXGDIREGION *LPGXGDIREGION,*GXLPGDIREGION;
typedef GXGDIBITMAP  *LPGXGDIBITMAP, *GXLPGDIBITMAP;


// GX Device Context
typedef struct __tagGXGDIDC : GXGDIOBJ
{
  GXWndCanvas*    pWndCanvas;    // 
  GXCanvas*       pCanvas;    
  LPGXGDIBITMAP   lpBitmap;      // 如果是MemDC，才不为0
  GXHWND          hBindWnd;      // 与这个DC绑定的窗口
  //GXUINT        uRefCount;
  //GXINT         wndOrgX;       // 窗口的原点,绘图函数指定的(0,0)将出现在(wndOrgX,wndOrgY)点
  //GXINT         wndOrgY;
  GXCOLORREF      crTextBack;
  GXCOLORREF      crText;
  GXDWORD         flag;
  LPGXGDIBRUSH    hBrush;
  LPGXGDIPEN      hPen;
  LPGXGDIFONT     hFont;
  GXPOINT         ptPen;
} GXGDIDC, *LPGXGDIDC, *GXLPGDIDC;

// GXDC flag
#define GXDCFLAG_MANUALBEGINSCENE  0x80000000L
#define GXDCFLAG_OPAQUEBKMODE    0x00000001L
#define GXDCFLAG_COMPATIBLE      0x00000002L

typedef struct __tagGXICON
{
  GXBOOL      fIcon;
  GXDWORD      xHotspot;
  GXDWORD      yHotspot;
  GXImage  *    pImgIcon;
//  HBITMAP hbmMask; 
//  HBITMAP hbmColor; 
}GXICON, *LPGXICON, *GXLPICON;

// GXDC Macro

#define GXGDIOBJ_TYPE(_GXHGDIOBJ)     (*((GXGDIOBJTYPE*)_GXHGDIOBJ))
#define GXGDIOBJ_PTR(_GXHGDIOBJ)      ((GXGDIOBJ*)  _GXHGDIOBJ)
#define GXGDI_DC_PTR(_GXHGDIOBJ)      ((GXGDIDC*)    _GXHGDIOBJ)
#define GXGDI_BITMAP_PTR(_GXHGDIOBJ)  ((GXGDIBITMAP*)  _GXHGDIOBJ)
#define GXGDI_BRUSH_PTR(_GXHGDIOBJ)   ((GXGDIBRUSH*)  _GXHGDIOBJ)
#define GXGDI_PEN_PTR(_GXHGDIOBJ)     ((GXGDIPEN*)  _GXHGDIOBJ)
#define GXGDI_FONT_PTR(_GXHGDIOBJ)    ((GXGDIFONT*)  _GXHGDIOBJ)
#define GXGDI_RGN_PTR(_GXHGDIOBJ)     ((GXGDIREGION*)  _GXHGDIOBJ)

#define GXGDI_DC_HANDLE(_PGXGDIOBJ)     ((GXHDC)_PGXGDIOBJ)
#define GXGDI_BRUSH_HANDLE(_PGXGDIOBJ)  ((GXHBRUSH)_PGXGDIOBJ)
#define GXGDI_BITMAP_HANDLE(_PGXGDIOBJ) ((GXHBITMAP)_PGXGDIOBJ)
#define GXGDI_PEN_HANDLE(_PGXGDIOBJ)    ((GXHPEN)_PGXGDIOBJ)
#define GXGDI_RGN_HANDLE(_PGXGDIOBJ)    ((GXHRGN)_PGXGDIOBJ)

namespace GXWin32APIEmu
{
  GXBOOL InitializeStatic();
  GXBOOL ReleaseStatic();
  GXVOID SetScreen(long nWidth, long nHeight);
  GXVOID* GetGXGdiObj(GXGDIOBJTYPE emObjType, GXLPVOID uResCode);
  GXVOID MapGXGdiObj(GXGDIOBJTYPE emObjType, GXLPVOID uResCode, GXLPVOID lpGdiObj);
  GXBOOL EraseGXGdiObj(GXGDIOBJTYPE emObjType, GXLPVOID uResCode);
}


//////////////////////////////////////////////////////////////////////////
struct GXCARET
{
  // TODO: 以后是否能把光标闪烁加入到RichFX中呢?
  GXHWND        hWnd;
  GXHWND        hTopLevel;
  GXHBITMAP      hBitmap;
  GXREGN        regnCaret;    // 相对于渲染区域的位置, 经由 hWnd 的位置计算得到
  GXREGN        regnPrevShowing;// 上一次显示的位置，如果SetCaretPos设置时没有显示则为空
  GXUINT        nBlinkTime;
  GXUINT        nBaseTime;    // 这个值用来保证一个细节问题，即光标被移动/显示时会立即显示
  GXDWORD        flag;
  GXBOOL  IsVisible  ();
  GXBOOL  Tick    ();        // 只在可用并且显示时更新就可以了
  GXHRESULT PaintCaret  (GXCanvas* pCanvas);
};

struct INTMEASURESTRING // 内部字符串测量结构
{
  GXFont*   pFont;
  GXLPCWSTR lpString;
  GXINT     cString;
  GXUINT    uFormat; // "GXDT_"标志

  // 设置GXDT_EXPANDTABS时才有效
  GXINT     nTabPositions;
  GXINT*    lpnTabStopPositions;
};

typedef GXCARET* LPGXCARET;

typedef struct __tagGXACCEL16
{
  GXBYTE   fVirt;
  GXWORD   key;
  GXWORD   cmd;
} GXACCEL16, *GXLPACCEL16, *LPGXACCEL16;

struct GXLOCALMEM_STRUCT
{
  GXDWORD   dwMagic;
  GXINT     nLockCount;
  GXSIZE_T  uSize;
};
#define LOCALMEM_MAGIC 0x12345678
#define GXLOCALMEM_HANDLE(LMEMPTR)  ((GXHLOCAL)LMEMPTR)
#define GXLOCALMEM_PTR(LMEMHANDLE)  ((GXLOCALMEM_STRUCT*)LMEMHANDLE)

// 类注册后储存用的数据结构
typedef struct __tagGXWNDCLSATOM
{
  GXINT       nRefCount;  // 有多少窗口引用了这个类
  GXUINT      style;
  GXWNDPROC   lpfnWndProc; 
  GXINT       cbClsExtra; 
  GXINT       cbWndExtra; 
//HANDLE      hInstance; 
  GXHSTATION  hStation;
  GXHICON     hIcon; 
  GXHCURSOR   hCursor; 
  GXHBRUSH    hbrBackground; 
  GXWCHAR     szMenuName[16]; 
  GXWCHAR     szClassName[32]; 
}GXWNDCLSATOM, *LPGXWNDCLSATOM, *GXLPWNDCLSATOM;

struct MOUIMSG
{
  void*     handle;
  u32       message;
  GXWPARAM  wParam;
  GXLPARAM  lParam;
  u32       dwTime;
  i32       xPos;
  i32       yPos;
};


#define GXCARET_AVAILABLE     0x80000000L        // 可用标志
#define GXCARET_VISIBLE       0x00000001L        // 是否被用户设置为显示
#define GXCARET_BLINK         0x00000002L        // 闪烁显示时如果可见，则设置此标志
#define GXCARET_SHOWING       (GXCARET_AVAILABLE | GXCARET_VISIBLE | GXCARET_VISIBLE)    // 能被渲染出来的所有检测标志
                
#if defined(_WIN32) && defined(_X86) && ! defined(__clang__)
#define ENABLE_ASSEMBLE
#endif // _WIN32

typedef GXWnd *LPGXWND, *GXLPWND;
typedef clvector<GXLPWND>  GXLPWND_ARRAY;
typedef cllist<GXLPWND>    GXLPWND_LIST;

#define GXWND_PTR(HANDLEWND)          ((HANDLEWND != NULL && ((LPGXWND)HANDLEWND)->m_hSelf == HANDLEWND) ? (LPGXWND)HANDLEWND : NULL)
#define GXWND_HANDLE(WNDPTR)          ((GXHWND)WNDPTR)

#define GXICON_PTR(ICONHANDLE)        ((LPGXICON)ICONHANDLE)
#define GXICON_HANDLE(ICONPTR)        ((GXHICON)ICONPTR)

#define GXSTATION_PTR(HDL)            ((GXLPSTATION)HDL)
#define GXSTATION_HANDLE(STPTR)       ((GXHSTATION)STPTR)
#define GXHWND_STATION_PTR(HANDLEWND) GXSTATION_PTR((GXWND_PTR(HANDLEWND)->m_lpClsAtom)->hStation)
#define GXLPWND_STATION_PTR(LPWND)    GXSTATION_PTR((LPWND->m_lpClsAtom)->hStation)

#define GXHINSTANCE_PTR(HINST)        ((GXLPINSTANCE)HINST)
#define GXINSTANCE_HANDLE(INST_PTR)   ((GXHINSTANCE)INST_PTR)

#define GXHRSRC_PTR(_HRSRC)           ((LPGXRSRC)_HRSRC)
#define GXRSRC_HANDLE(RSRC_PTR)       ((GXHRSRC)RSRC_PTR)

// GXSTATION 的标志
#define  GXST_DRAWDEBUGMSG      0x00000001
struct GXINSTANCE;
struct GXSTATION
{
  typedef clhash_map<clStringA, GUnknown*>  NamedInterfaceDict;
  typedef clhash_map<clStringA, CONSOLECMD> CmdDict;
  GXDWORD             dwMagic;
  GXDWORD             m_dwFlags;
  IGXPlatform*        lpPlatform;
  GXDWORD             dwUIThreadId;     // GXUI不支持多线程Wnd, 这里记录了UI线程ID, 用于校验
  GXMONITORINFO       MonitorInfo;
#ifdef _WIN32
  HWND                hBindWin32Wnd;    // gx 所属的系统窗口
  HCURSOR             hCursor;
#endif // _WIN32
  GXINSTANCE*         pInstDll;
  GXINSTANCE*         pInstApp;
  GXGraphics*         pGraphics;
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

#ifdef ENABLE_AERO
  GTexture*        pBackDownSampTexA;    // 玻璃效果的缓冲纹理 - A
  GTexture*        pBackDownSampTexB;    // 玻璃效果的缓冲纹理 - B
#endif // ENABLE_AERO
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXSTATION(HWND hWnd, IGXPlatform* lpPlatform);
#else
  GXSTATION(IGXPlatform* lpPlatform);
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXHRESULT    Initialize     ();
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
};
typedef GXSTATION*        GXLPSTATION;
typedef const GXSTATION*  GXLPCSTATION;
typedef GXSTATION* const  GXCLPSTATION;

struct GXRSRC
{
  clStringA  strFilename;
  clBuffer*  pBuffer;

  GXRSRC(GXLPCSTR szFilename);
  virtual ~GXRSRC();
  GXBOOL Load();
};

typedef GXRSRC* LPGXRSRC;
typedef GXRSRC* GXLPRSRC;


struct GXINSTANCE
{
  GXLPSTATION    lpStation;
#if defined(_WIN32) || defined(_WINDOWS)
  HINSTANCE    hInstance;
#endif // defined(_WIN32) || defined(_WINDOWS)
  clStringA    strRootDir;
  clStringA    strModuleName;  // 同时也作为路径
  typedef clhash_map<GXDWORD, GXRSRC*> ResCodeDict;
  ResCodeDict    sResCodeDict;
#if defined(_WIN32) || defined(_WINDOWS)
  GXBOOL Initialize(GXLPSTATION _lpStation, HINSTANCE hInst);
#else
  GXBOOL Initialize(GXLPSTATION _lpStation, GXLPSTR szModuleName);
#endif // defined(_WIN32) || defined(_WINDOWS)
  GXBOOL Finalize();
};

struct DLGLOG
{
  typedef clmap<clStringW, GXHWND>  NameToWndDict;

  GXUINT        cbSize;
  GXDLGPROC     pDlgProc; // 与DWL_PROC储存重复?
  clStringW     strName;
  NameToWndDict CtrlItemDict;
  GXUI::Layout* pLayout;
  GXLPARAM      lParam;   // CreateDialog()的lParam参数
 
  DLGLOG();
  virtual ~DLGLOG();
  GXBOOL AddItem(GXLPCWSTR szName, GXHWND hWnd);
  GXHWND GetItem(GXLPCWSTR szName) const;
  
};
typedef DLGLOG* LPDLGLOG;
typedef GXINSTANCE* GXLPINSTANCE;
typedef GXINSTANCE GXMODULE;
typedef GXMODULE* GXLPMODULE;

//extern "C" GXHSTATION GXDLLAPI GXUIGetStation();
GXLPSTATION IntGetStationPtr();
GXLPRECT _GlbLockStaticRects(GXUINT nCounts);
GXVOID _GlbUnlockStaticRects(GXLPRECT lpRects);
#if defined(_WIN32) || defined(_WINDOWS)
void    IntWin32ResizeWindow    (HWND hWnd, GXApp* pApp);
GXBOOL  IntWin32SwitchFullScreen(HWND hWnd);
GXBOOL  IntSetCursor            (GXWPARAM wParam, LPARAM lParam);
#endif // #if defined(_WIN32) || defined(_WINDOWS)

struct GXDEFERWNDPOS
{
  struct ELEMENT
  {
    GXHWND hWnd;
    GXHWND hWndInsertAfer;
    int x, y, cx, cy;
    GXDWORD dwFlags;
  };
  typedef clvector<ELEMENT> ElementArray;
  ElementArray m_aElements;
};

typedef GXDEFERWNDPOS*    GXLPDEFERWNDPOS;

#define GXDWP_PTR(_HANDLE)    ((GXLPDEFERWNDPOS)_HANDLE)
#define GXDWP_HANDLE(_PTR)    ((GXHDWP)_PTR)

// 引用的全局变量
extern long g_SystemMetrics[];

extern GXGDIBRUSH g_BlackBrush;
extern GXGDIBRUSH g_DarkGrayBrush;
extern GXGDIBRUSH g_GrayBrush;
extern GXGDIBRUSH g_LightGrayBrush;
extern GXGDIBRUSH g_NullBrush;
extern GXGDIBRUSH g_WhiteBrush;
extern GXGDIBRUSH g_Brush_80FFFFFF;
extern GXGDIBRUSH g_Brush_00FFFFFF;
extern GXGDIPEN g_BlackPen;
extern GXGDIPEN g_WhitePen;
extern GXGDIPEN g_NullPen;

#define DEFAULT_FONT_NAME "fonts\\wqy-microhei.ttc"
#define DEFAULT_FONT_NAMEW L"fonts\\wqy-microhei.ttc"

//
// menu ex template
//

struct GXMENUEX_TEMPLATE_HEADER{
  GXWORD  wVersion;
  GXWORD  wOffset;
  GXDWORD dwHelpId;
};

#pragma pack(push, 2)
struct GXMENUEX_TEMPLATE_ITEM_HEADER{
  //GXDWORD dwHelpId;   // Popup 才有这项
  GXDWORD dwType;
  GXDWORD dwState;
  GXDWORD menuId;
  GXWORD  bResInfo;
  // 后面接 GXWCHAR[] 数据
};
#pragma pack(pop)






template<typename _Ty>
inline _Ty GetVertexDeclLength(LPCGXVERTEXELEMENT lpVerticesDecl)
{
  _Ty nCount = 0;
  while((GXINT)lpVerticesDecl[nCount].UsageIndex >= 0)
    nCount++;
  return nCount;
}

// 这个是在测试标志时使用,所以是内部标志, 不能用来做创建时的标志
#define GXRU_TEST_READ  (GXRU_MIGHTBEREAD | GXRU_FREQUENTLYREAD)    // 用来测试读的标志
#define GXRU_TEST_WRITE (GXRU_MIGHTBEWRITE | GXRU_FREQUENTLYWRITE)  // 用来测试写的标志
#define GXRU_TEST_FREQUENTLY (GXRU_FREQUENTLYREAD | GXRU_FREQUENTLYWRITE)
#define GXRU_TEST_MIGHTBE    (GXRU_MIGHTBEREAD | GXRU_MIGHTBEWRITE)


#define WM_GX_RESETDEVICED3D9 (WM_USER + 100)
#ifdef _UPDATE_WIN32_CARET
#define WM_GX_CREATECARET     (WM_USER + 101)
#define WM_GX_DESTROYCARET    (WM_USER + 102)
#define WM_GX_SHOWCARET       (WM_USER + 103)
#define WM_GX_SETCARETPOS     (WM_USER + 104)
#endif // #ifdef _UPDATE_WIN32_CARET

namespace MarimoVerifier
{
  namespace Texture
  {
    GXBOOL CreateFromFileParam(GXLPCSTR szPrefix, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter);
    GXBOOL CreateParam(GXLPCSTR szPrefix, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
  }
} // namespace MarimoVerifier

typedef clvector<GXDefinition>      ParamArray;

struct MTLFILEPARAMDESC
{
  ParamArray      aUniforms;
  ParamArray      aStates;
  clStringArrayA  aBindPool;
};

#endif // _GRAPHICS_X_USER_INTERFACE_GLOBAL_DEFINE_