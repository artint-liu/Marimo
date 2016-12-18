//////////////////////////////////////////////////////////////////////////
// GrapX.H

//////////////////////////////////////////////////////////////////////////
// 
// _GRAPX_DEF_HEADER_ 定义头文件
// 包含了公共借口所用的常量、结构、数据类型和宏的信息
// Liu.Chenglong
//

#ifndef _GRAPX_DEF_HEADER_
#define _GRAPX_DEF_HEADER_

#define REFACTOR_SHADER   // shader 重构宏

//////////////////////////////////////////////////////////////////////////
//
// 基础类型声明
//

#include <clstd.h>
#include <clString.H>
//#include <clFile.H>
//#include <clBuffer.H>
#include <thread/clThread.H>
#include <thread/clSignal.H>
//#include <thread/clMessageThread.h>
#include <clutility.h>
#include <clColorSpace.h>
//#include <smart/SmartRepository.h>
//#include <Socket/clSocket.h>

#include "GrapX/GBaseTypes.H"
#include <GrapX/GUnknown.h>
#include <GrapX/gxError.H>

#ifndef offsetof
#define offsetof(s,m)     (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif // offsetof
#define IS_IDENTIFY(_ID_)  (((GXDWORD_PTR)_ID_) <= 0xFFFF)
#ifdef _X64
#define IS_PTR(_PTR_)      (((GXDWORD_PTR)_PTR_) > 0xFFFF && ((GXDWORD_PTR)_PTR_) < 0x8000000000000000)
#else
#define IS_PTR(_PTR_)      (((GXDWORD_PTR)_PTR_) > 0xFFFF && ((GXDWORD_PTR)_PTR_) < 0x80000000)  // 32 bits mode
#endif // #ifdef _X64

#define GXSNDMSG ::gxSendMessage

namespace GXUI
{
  class IListDataAdapter;
} // namespace GXUI


namespace clstd
{
  class Buffer;
  class BufferBase;
  class SmartRepository;
  class StockA;
  class StockW;
} // namespace clstd

typedef clstd::Buffer           clBuffer;
typedef clstd::BufferBase       clBufferBase;
typedef clstd::StockA           clStockA;
typedef clstd::StockW           clStockW;
typedef clstd::SmartRepository  clSmartRepository;

//////////////////////////////////////////////////////////////////////////
//
// 基础数据结构
//
#include "GrapX/GRect.h"
typedef struct __tagGXPOINT
{
  GXLONG  x;
  GXLONG  y;
}GXPOINT, *GXLPPOINT, *LPGXPOINT;
typedef const GXPOINT GXCPOINT;
typedef const GXPOINT* GXLPCPOINT;

typedef struct __tagGXPOINTF
{
  GXFLOAT  x;
  GXFLOAT  y;
}GXPOINTF, *GXLPPOINTF, *LPGXPOINTF;

typedef struct __tagGXSIZE
{
  GXLONG  cx;
  GXLONG  cy;
}GXSIZE, *GXLPSIZE, *LPGXSIZE;
typedef const GXSIZE* GXLPCSIZE;

typedef struct __tagGXSIZEF
{
  GXFLOAT  cx;
  GXFLOAT  cy;
}GXSIZEF, *GXLPSIZEF, *LPGXSIZEF;

#ifdef REFACTOR_RECTREGN
typedef Marimo::RectT<GXLONG> GXRECT;
typedef Marimo::RectT<GXFLOAT> GXRECTF;
typedef GXRECT* GXLPRECT;
typedef GXRECT* LPGXRECT;
typedef const GXRECT* GXLPCRECT;
typedef const GXRECT* LPGXCRECT;
typedef GXRECTF* GXLPRECTF;
typedef GXRECTF* LPGXRECTF;
typedef const GXRECTF* GXLPCRECTF;
typedef const GXRECTF* LPGXCRECTF;
#else
template<typename _T>
struct RECTX
{
  _T left, top, right, bottom;
};
typedef RECTX<GXLONG> GXRECT;
typedef RECTX<GXFLOAT> GXRECTF;
typedef GXRECT* GXLPRECT;
typedef GXRECT* LPGXRECT;
typedef const GXRECT* GXLPCRECT;
typedef const GXRECT* LPGXCRECT;
typedef GXRECTF* GXLPRECTF;
typedef GXRECTF* LPGXRECTF;
typedef const GXRECTF* GXLPCRECTF;
typedef const GXRECTF* LPGXCRECTF;
#endif // #ifndef REFACTOR_RECTREGN


struct GXDEFINITION;
class GXDefinition   // string class
{
public:
  clStringA Name;
  clStringA Value;

  operator GXDEFINITION*()
  {
    return (GXDEFINITION*)this;
  }

  operator const GXDEFINITION*()
  {
    return (const GXDEFINITION*)this;
  }

  operator GXDEFINITION&()
  {
    return *(GXDEFINITION*)this;
  }
};

typedef clvector<GXDefinition> GXDefinitionArray;

// 由于 clString 的特殊性 GXDEFINITION_S 可以直接强制类型转换为 GXDEFINITION, 但是反之不可以!
struct GXDEFINITION // 改名 GXPARAMETER ?
{
  GXLPCSTR szName;
  GXLPCSTR szValue;
};

class GXDefinitionW   // string class
{
public:
  clStringW Name;
  clStringW Value;
};

typedef clvector<GXDefinitionW> GXDefinitionArrayW;

struct GXGUID{
  unsigned long  Data1;
  unsigned short Data2;
  unsigned short Data3;
  GXBYTE         Data4[8];
};

STATIC_ASSERT(sizeof(GXGUID) == 16);

//typedef struct __tagGXRECT
//{
//  GXLONG  left;
//  GXLONG  top;
//  GXLONG  right;
//  GXLONG  bottom;
//}GXRECT, *GXLPRECT, *LPGXRECT;
//typedef const GXLPRECT  GXLPCRECT;
//
//typedef struct __tagGXRECTF
//{
//  GXFLOAT  left;
//  GXFLOAT  top;
//  GXFLOAT  right;
//  GXFLOAT  bottom;
//}GXRECTF, *GXLPRECTF, *LPGXRECTF;
//typedef const GXLPRECTF  GXLPCRECTF;

//////////////////////////////////////////////////////////////////////////

class GXWnd;  // TODO: 放到内部文件中?
class GXGraphics;
class GShader;
class GXEffect;
class GTexture;
class GXCanvas;
class GRegion;
class CAeroShader;
class CSimpleShader;
class CBlurShader;
class GXFont;
//class CGraphics;


// 常量
#define GX_OK                 (0L)
#define GX_FAIL               (0x80004005L)
#define GX_ERROR_HANDLE       (0x80000006L)
#define GX_ERROR_OUTOFRANGE   (0x80000007L)
#define GX_ERROR_OUROFMEMORY  (0x80000008L)
#define GX_E_OPEN_FAILED      (0x8000006EL)

#define GXOUT
#define GXIN
#define GXINOUT
//extern int  g_screen_width;
//#define GXSCREEN_WIDTH    g_screen_width//854
//#define GXSCREEN_HEIGHT    480
//#define GXSCREEN_WIDTH_D2  (GXSCREEN_WIDTH / 2)
//#define GXSCREEN_HEIGHT_D2  (GXSCREEN_HEIGHT / 2)

#ifdef REFACTOR_RECTREGN
typedef Marimo::RegnT<GXLONG> REGN;
typedef REGN GXREGN;
typedef REGN* LPREGN;
typedef REGN* LPGXREGN;
typedef REGN* GXLPREGN;
typedef const REGN* GXLPCREGN;
typedef const REGN* LPGXCREGN;
typedef const REGN* LPCREGN;
#else
template<typename _T>
struct REGNX
{
  _T left, top, width, height;
};
typedef REGNX<GXLONG> REGN;
typedef REGN GXREGN;
typedef REGN* LPREGN;
typedef REGN* LPGXREGN;
typedef REGN* GXLPREGN;
typedef const REGN* GXLPCREGN;
typedef const REGN* LPGXCREGN;
typedef const REGN* LPCREGN;
#endif // #ifdef REFACTOR_RECTREGN

//typedef struct __tagREGN
//{
//  GXLONG    left;
//  GXLONG    top;
//  GXLONG    width;
//  GXLONG    height;
//} REGN, *LPREGN, GXREGN, *GXLPREGN, *LPGXREGN;
//typedef const LPREGN LPCREGN;

//struct GXRect : public GXRECT
//{
//  GXRect(GXLONG l, GXLONG t, GXLONG r, GXLONG b)
//  {
//    Set(l, t, r, b);
//  }
//
//  GXRect& Set(GXLONG l, GXLONG t, GXLONG r, GXLONG b)
//  {
//    left = l;
//    top = t;
//    right = r;
//    bottom = b;
//    return *this;
//  }
//
//  void Inflate(GXLONG dx, GXLONG dy)
//  {
//    gxInflateRect(this, dx, dy);
//  }
//};

template<typename _T>
class RegnX
{
public:
  union
  {
    struct
    {
      _T left, top, width, height;
    };
    struct  
    {
      _T x, y, w, h;
    };
  };
  RegnX(){}
  RegnX(_T v) : x(v), y(v), w(v), h(v){}
  RegnX(_T _x, _T _y, _T _w, _T _h) : x(_x),y(_y),w(_w),h(_h){}
  RegnX(const RegnX& rr) : x(rr.x),y(rr.y),w(rr.w),h(rr.h){}
  void Offset(_T dx, _T dy){ x += dx; y += dy;}
  void Empty()
  {
    x = y = w = h = 0;
  }
  void Set(int _x, int _y, int _w, int _h)
  {
    x = _x; y = _y;
    w = _w; h = _h;
  }
};

class Regn : public RegnX<GXLONG>
{
public:
  Regn(){}
  Regn(int v) : RegnX(v,v,v,v){}
  Regn(int _x, int _y, int _w, int _h) : RegnX(_x, _y, _w, _h){}
  Regn(const GXRECT& rect) : RegnX(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom) {}
  Regn& operator=(const GXREGN& regn)
  {
    x = regn.left;
    y = regn.top;
    w = regn.width;
    h = regn.height;
    return *this;
  }
  Regn& operator=(const GXRECT& rect)
  {
    x = rect.left;
    y = rect.top;
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;
    return *this;
  }
  operator const GXREGN*() const
  {
    return (const GXREGN*)this;
  }
  operator const GXREGN&() const
  {
    return *(this->operator const GXREGN *());
  }
};

class RegnF : public RegnX<GXFLOAT>
{
public:
  RegnF(int v) : RegnX((float)v,(float)v,(float)v,(float)v){}
  RegnF(int _x, int _y, int _w, int _h) : RegnX((float)_x, (float)_y, (float)_w, (float)_h){}
  RegnF(float v) : RegnX(v,v,v,v){}
  RegnF(float _x, float _y, float _w, float _h) : RegnX(_x, _y, _w, _h){}

  //operator const REGN* ()
  //{
  //  return (const REGN*)this;
  //}
};

typedef Regn GXRegn;
typedef RegnX<GXLONG>     RegnI;
//typedef RegnX<GXFLOAT>    RegnF;

//typedef void    *GXHBRUSH, *GXHBRUSH;
//////////////////////////////////////////////////////////////////////////
#define GXDECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
#define GXDECLARE_HANDLE_FROM(name,_base) struct name##__:public _base##__{int unused;}; typedef struct name##__ *name

GXDECLARE_HANDLE(GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHDWP, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHWND, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHMENU, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHSTATION, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHINSTANCE, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHGDIOBJ, GXHANDLE);
GXDECLARE_HANDLE_FROM(GXHDC, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHBITMAP, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHPEN, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHFONT, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHRGN, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHBRUSH, GXHGDIOBJ);
GXDECLARE_HANDLE_FROM(GXHICON, GXHGDIOBJ);

//typedef GXHWND      HGXWND;
//typedef GXHMENU     HGXMENU;
//typedef GXHSTATION  HGXSTATION;
//typedef GXHGDIOBJ   GXHGDIOBJ;
//typedef GXHBITMAP   HGXBITMAP;
//typedef GXLPVOID    HGXWND;
//typedef GXLPVOID    GXHWND;

//typedef GXLPVOID    HGXSTATION;
//typedef GXLPVOID    GXHSTATION;

//typedef GXLPVOID    HGXBITMAP;
//typedef GXLPVOID    GXHBITMAP;
GXDECLARE_HANDLE(GXHIMC);
GXDECLARE_HANDLE(GXHACCEL);
GXDECLARE_HANDLE_FROM(GXHENHMETAFILE, GXHGDIOBJ);
GXDECLARE_HANDLE(GXHLOCAL);
//GXDECLARE_HANDLE(GXHMODULE);
GXDECLARE_HANDLE(GXHRSRC);

#define GXHGLOBAL GXHLOCAL
typedef GXHINSTANCE GXHMODULE;
//typedef GXLPVOID    GXHIMC;
typedef GXDWORD     GXLCID;
//typedef GXLPVOID    GXHACCEL;
//typedef GXLPVOID    GXHENHMETAFILE;
//typedef GXLPVOID    GXHLOCAL;
//typedef GXLPVOID    GXHMODULE;
//typedef GXLPVOID    GXHRSRC;
//typedef GXLPVOID    GXHGLOBAL;

class GXWndCanvas;
typedef GXGraphics* LPGXGRAPHICS;
typedef GXWndCanvas* LPGXWNDCANVAS;


typedef struct __tagGXMSG
{
  GXHWND    hwnd;      // Windows Handle
  GXUINT    message;
  GXWPARAM  wParam;
  GXLPARAM  lParam;
  GXDWORD   time;
  GXPOINT   pt;
}GXMSG,*LPGXMSG, *GXLPMSG;

//////////////////////////////////////////////////////////////////////////

typedef GXDWORD CMenu;

//typedef GXVOID*       HGXINSTANCE;
//typedef GXHINSTANCE   HGXINSTANCE;

//typedef GXVOID*       GXHGDIOBJ;
//typedef GXHGDIOBJ     GXHGDIOBJ;

//typedef GXVOID*       HGXDC;
//typedef GXHDC         HGXDC;

typedef GXVOID*       GXHPALETTE;

//typedef void*         HGXICON;
//typedef GXHICON       HGXICON;

//typedef HGXICON       HGXCURSOR;
typedef GXHICON       GXHCURSOR;

//typedef void*         HGXPEN;
//typedef GXHPEN        HGXPEN;

typedef void*         HGXMONITOR;
typedef HGXMONITOR    GXHMONITOR;

//typedef void*         HGXFONT;
//typedef GXHFONT       HGXFONT;
//
//typedef void*         HGXRGN;
//typedef GXHRGN        HGXRGN;

//typedef void*         GXHANDLE;
//typedef GXLONG_PTR    HGXMENU, GXHMENU;

typedef struct _GXDPA *GXHDPA;


typedef GXBOOL        (GXCALLBACK* GXWNDENUMPROC)   (GXHWND, GXLPARAM);
typedef GXLRESULT     (GXCALLBACK* GXWNDPROC)       (GXHWND, GXUINT, GXWPARAM, GXLPARAM);
typedef GXINT_PTR     (GXCALLBACK* GXDLGPROC)       (GXHWND, GXUINT, GXWPARAM, GXLPARAM);
typedef GXVOID        (GXCALLBACK* GXTIMERPROC)     (GXHWND, GXUINT, GXUINT, GXDWORD);
typedef GXBOOL        (GXCALLBACK* GXDRAWSTATEPROC) (GXHDC hdc, GXLPARAM lData, GXWPARAM wData, GXINT cx, GXINT cy);

#define GXHWND_DESKTOP        ((GXHWND)0)
#define GXHWND_MESSAGE        ((GXHWND)-3)
#define GXCW_USEDEFAULT       ((int)0x80000000)
#define GXARGB(a,r,g,b)       ((GXCOLORREF)(((GXBYTE)(b)|((GXWORD)((GXBYTE)(g))<<8))|(((GXDWORD)(GXBYTE)(r))<<16)|(((GXDWORD)(GXBYTE)(a))<<24)))
#define GXABGR(a,r,g,b)       ((GXCOLORREF)(((GXBYTE)(r)|((GXWORD)((GXBYTE)(g))<<8))|(((GXDWORD)(GXBYTE)(b))<<16)|(((GXDWORD)(GXBYTE)(a))<<24)))
#define GXRGB(r,g,b)          ((GXCOLORREF)(((GXBYTE)(r)|((GXWORD)((GXBYTE)(g))<<8))|(((GXDWORD)(GXBYTE)(b))<<16)))

/* Background Modes */
#define GXTRANSPARENT         1
#define GXOPAQUE              2
#define GXBKMODE_LAST         2

//* PolyFill() Modes
#define GXALTERNATE           1
#define GXWINDING             2


// Ctl brush index
// 用于WM_CTLCOLOR_XXXX 消息返回 Brush 的索引
#define CTLBRUSH_MSGBOX         0
#define CTLBRUSH_EDIT           1
#define CTLBRUSH_LISTBOX        2
#define CTLBRUSH_BTN            3
#define CTLBRUSH_DLG            4
#define CTLBRUSH_SCROLLBAR      5
#define CTLBRUSH_STATIC         6
#define CTLBRUSH_MAX            7

//
// gxGetWindow 参数
//
#define GXGW_FIRSTCHILD   (256 + 0)
#define GXGW_PREVIOUS     (256 + 1)
#define GXGW_NEXT         (256 + 2)
#define GXGW_PARENT       (256 + 3)

//
// GXGetSystemRegion 参数
//
#define GSR_WINDOW        (GXDCX_WINDOW)        // 计算整个窗口
#define GSR_CLIPSIBLINGS  (GXDCX_CLIPSIBLINGS)  // 考虑对兄弟窗口的裁剪
#define GSR_CLIPCHILDREN  (GXDCX_CLIPCHILDREN)  // 考虑子窗口的裁剪
#define GSR_PARENTCLIP    (GXDCX_PARENTCLIP)    // 裁剪父窗口
#define GSR_ALLLAYERS     0x01000000            // 考虑所有的层
#define GSR_AVAILABLE     0x02000000            // 所有可用的区域, 即按窗口在全屏大小下的裁剪, 这个会无视GSR_WINDOW

//
// Owner Draw Itesm
//
#define GXODT_HEADER              100
#define GXODT_LISTVIEW            102

///*
// * Owner draw actions
// */
//#define GXODA_DRAWENTIRE  0x0001
//#define GXODA_SELECT      0x0002
//#define GXODA_FOCUS       0x0004

///*
// * Owner draw state
// */
//#define GXODS_SELECTED    0x0001
//#define GXODS_GRAYED      0x0002
//#define GXODS_DISABLED    0x0004
//#define GXODS_CHECKED     0x0008
//#define GXODS_FOCUS       0x0010
//#define GXODS_DEFAULT         0x0020
//#define GXODS_COMBOBOXEDIT    0x1000
//#define GXODS_HOTLIGHT        0x0040
//#define GXODS_INACTIVE        0x0080
//#define GXODS_NOACCEL         0x0100
//#define GXODS_NOFOCUSRECT     0x0200


//////////////////////////////////////////////////////////////////////////

struct GXMONITORINFO
{
  GXDWORD   cbSize;
  GXRECT    rcMonitor;
  GXRECT    rcWork;
  GXDWORD   dwFlags;
};
typedef GXMONITORINFO *GXLPMONITORINFO;

/*
 * Scroll Bar Constants
 */
#define GXSB_HORZ             0
#define GXSB_VERT             1
#define GXSB_CTL              2
#define GXSB_BOTH             3

/*
 * Scroll Bar Commands
 */
#define GXSB_LINEUP           0
#define GXSB_LINELEFT         0
#define GXSB_LINEDOWN         1
#define GXSB_LINERIGHT        1
#define GXSB_PAGEUP           2
#define GXSB_PAGELEFT         2
#define GXSB_PAGEDOWN         3
#define GXSB_PAGERIGHT        3
#define GXSB_THUMBPOSITION    4
#define GXSB_THUMBTRACK       5
#define GXSB_TOP              6
#define GXSB_LEFT             6
#define GXSB_BOTTOM           7
#define GXSB_RIGHT            7
#define GXSB_ENDSCROLL        8

//
// Scroll Information
//
#define GXSIF_RANGE           0x0001
#define GXSIF_PAGE            0x0002
#define GXSIF_POS             0x0004
#define GXSIF_DISABLENOSCROLL 0x0008
#define GXSIF_TRACKPOS        0x0010
#define GXSIF_ALL             (GXSIF_RANGE | GXSIF_PAGE | GXSIF_POS | GXSIF_TRACKPOS)


typedef struct __tagGXSCROLLBAR
{
  GXINT     m_nMin;
  GXINT     m_nMax;
  GXUINT    m_nPage;
  GXINT     m_nPos;
  GXINT     m_nTrackPos;
  GXDWORD   m_uFlag;
}GXSCROLLBAR, *LPGXSCROLLBAR;

typedef struct __tagGXPAINTSTRUCT {
  GXHDC         hdc;
  GXBOOL        fErase;
  GXRECT        rcPaint;
  GXBOOL        fRestore;
  GXBOOL        fIncUpdate;
  GXBYTE        rgbReserved[32];
} GXPAINTSTRUCT, *GXLPPAINTSTRUCT, *LPGXPAINTSTRUCT;

typedef struct __tagGXBITMAP {  // bm  
  GXLONG   bmType; 
  GXLONG   bmWidth; 
  GXLONG   bmHeight; 
  GXLONG   bmWidthBytes; 
  GXWORD   bmPlanes; 
  GXWORD   bmBitsPixel; 
  GXLPVOID bmBits; 
} GXBITMAP, *LPGXBITMAP, *GXLPBITMAP; 


//////////////////////////////////////////////////////////////////////////
// 常量的定义
//extern GXINT c_nGXVScrollBarWidth;
//extern GXINT c_nGXHScrollBarHeight;

//////////////////////////////////////////////////////////////////////////
// gxGetRenderingRect 参数用的标志
//#define GRR_CLIENT    0x00000001L    // 需要的是Client的渲染区域
#define GRR_GETRECT           0L
#define GRR_CLIENT            1L
#define GRR_CONVERTRENDERORG  0x100L
#define GRR_CONVERTWNDORG     0x200L

#define GDT_GLOW              0x80000000L


/* Font Weights */
#define GXFW_DONTCARE         0
#define GXFW_THIN             100
#define GXFW_EXTRALIGHT       200
#define GXFW_LIGHT            300
#define GXFW_NORMAL           400
#define GXFW_MEDIUM           500
#define GXFW_SEMIBOLD         600
#define GXFW_BOLD             700
#define GXFW_EXTRABOLD        800
#define GXFW_HEAVY            900

#define GXFW_ULTRALIGHT       GXFW_EXTRALIGHT
#define GXFW_REGULAR          GXFW_NORMAL
#define GXFW_DEMIBOLD         GXFW_SEMIBOLD
#define GXFW_ULTRABOLD        GXFW_EXTRABOLD
#define GXFW_BLACK            GXFW_HEAVY

/* tmPitchAndFamily flags */
#define GXTMPF_FIXED_PITCH    0x01
#define GXTMPF_VECTOR         0x02
#define GXTMPF_DEVICE         0x08
#define GXTMPF_TRUETYPE       0x04
//
// Font Create Desc
//
#define GXLF_FACESIZE         32
struct GXLOGFONTA
{
  GXLONG      lfHeight;
  GXLONG      lfWidth;
  GXLONG      lfEscapement;
  GXLONG      lfOrientation;
  GXLONG      lfWeight;
  GXBYTE      lfItalic;
  GXBYTE      lfUnderline;
  GXBYTE      lfStrikeOut;
  GXBYTE      lfCharSet;
  GXBYTE      lfOutPrecision;
  GXBYTE      lfClipPrecision;
  GXBYTE      lfQuality;
  GXBYTE      lfPitchAndFamily;
  GXCHAR      lfFaceName[GXLF_FACESIZE];
};
typedef GXLOGFONTA *GXLPLOGFONTA;
typedef GXLOGFONTA *LPGXLOGFONTA;

struct GXLOGFONTW
{
  GXLONG      lfHeight;
  GXLONG      lfWidth;
  GXLONG      lfEscapement;
  GXLONG      lfOrientation;
  GXLONG      lfWeight;
  GXBYTE      lfItalic;
  GXBYTE      lfUnderline;
  GXBYTE      lfStrikeOut;
  GXBYTE      lfCharSet;
  GXBYTE      lfOutPrecision;
  GXBYTE      lfClipPrecision;
  GXBYTE      lfQuality;
  GXBYTE      lfPitchAndFamily;
  GXWCHAR     lfFaceName[GXLF_FACESIZE];
};
typedef GXLOGFONTW *GXLPLOGFONTW;
typedef GXLOGFONTW *LPGXLOGFONTW;

#ifdef _UNICODE
typedef GXLOGFONTW GXLOGFONT;
typedef GXLPLOGFONTW GXLPLOGFONT;
typedef LPGXLOGFONTW LPGXLOGFONT;
#else
typedef GXLOGFONTA GXLOGFONT;
typedef GXLPLOGFONTA GXLPLOGFONT;
typedef LPGXLOGFONTA LPGXLOGFONT;
#endif // _UNICODE

typedef struct _GXABC {
  int     abcA;
  GXUINT    abcB;
  int     abcC;
} GXABC, *GXLPABC;

struct GXTEXTMETRICW
{
  GXLONG        tmHeight;
  //GXLONG        tmAscent;
  //GXLONG        tmDescent;
  //GXLONG        tmInternalLeading;
  GXLONG        tmExternalLeading;
  GXLONG        tmAveCharWidth;
  GXLONG        tmMaxCharWidth;
  //GXLONG        tmWeight;
  //GXLONG        tmOverhang;
  //GXLONG        tmDigitizedAspectX;
  //GXLONG        tmDigitizedAspectY;
  //GXWCHAR       tmFirstChar;
  //GXWCHAR       tmLastChar;
  //GXWCHAR       tmDefaultChar;
  //GXWCHAR       tmBreakChar;
  //GXBYTE        tmItalic;
  //GXBYTE        tmUnderlined;
  //GXBYTE        tmStruckOut;
  GXBYTE        tmPitchAndFamily;
  //GXBYTE        tmCharSet;
};
typedef GXTEXTMETRICW  *GXLPTEXTMETRICW;

struct GXNONCLIENTMETRICSW
{
  GXUINT      cbSize;
  int         iBorderWidth;
  int         iScrollWidth;
  int         iScrollHeight;
  int         iCaptionWidth;
  int         iCaptionHeight;
  GXLOGFONTW  lfCaptionFont;
  int         iSmCaptionWidth;
  int         iSmCaptionHeight;
  GXLOGFONTW  lfSmCaptionFont;
  int         iMenuWidth;
  int         iMenuHeight;
  GXLOGFONTW  lfMenuFont;
  GXLOGFONTW  lfStatusFont;
  GXLOGFONTW  lfMessageFont;
  int         iPaddedBorderWidth;
};
typedef GXNONCLIENTMETRICSW *GXLPNONCLIENTMETRICSW;
typedef GXNONCLIENTMETRICSW *LPGXNONCLIENTMETRICSW;

typedef struct tagGXPANOSE
{
  GXBYTE    bFamilyType;
  GXBYTE    bSerifStyle;
  GXBYTE    bWeight;
  GXBYTE    bProportion;
  GXBYTE    bContrast;
  GXBYTE    bStrokeVariation;
  GXBYTE    bArmStyle;
  GXBYTE    bLetterform;
  GXBYTE    bMidline;
  GXBYTE    bXHeight;
} GXPANOSE, *GXLPPANOSE;

typedef struct _GXOUTLINETEXTMETRICW {
  GXUINT    otmSize;
  GXTEXTMETRICW otmTextMetrics;
  GXBYTE    otmFiller;
  GXPANOSE  otmPanoseNumber;
  GXUINT    otmfsSelection;
  GXUINT    otmfsType;
  int    otmsCharSlopeRise;
  int    otmsCharSlopeRun;
  int    otmItalicAngle;
  GXUINT    otmEMSquare;
  int    otmAscent;
  int    otmDescent;
  GXUINT    otmLineGap;
  GXUINT    otmsCapEmHeight;
  GXUINT    otmsXHeight;
  GXRECT    otmrcFontBox;
  int    otmMacAscent;
  int    otmMacDescent;
  GXUINT    otmMacLineGap;
  GXUINT    otmusMinimumPPEM;
  GXPOINT   otmptSubscriptSize;
  GXPOINT   otmptSubscriptOffset;
  GXPOINT   otmptSuperscriptSize;
  GXPOINT   otmptSuperscriptOffset;
  GXUINT    otmsStrikeoutSize;
  int    otmsStrikeoutPosition;
  int    otmsUnderscoreSize;
  int    otmsUnderscorePosition;
  GXLPSTR    otmpFamilyName;
  GXLPSTR    otmpFaceName;
  GXLPSTR    otmpStyleName;
  GXLPSTR    otmpFullName;
} GXOUTLINETEXTMETRICW, *GXLPOUTLINETEXTMETRICW;

//
// WM_GETMINMAXINFO 消息 lParam 指向的数据结构
//
typedef struct __tagGXMINMAXINFO {
  GXPOINT    ptReserved;
  GXPOINT    ptMaxSize;
  GXPOINT    ptMaxPosition;
  GXPOINT    ptMinTrackSize;
  GXPOINT    ptMaxTrackSize;
} GXMINMAXINFO, *GXLPMINMAXINFO, *LPGXMINMAXINFO;

//
// WM_NOTIFY 消息 lParam 指向的数据结构
typedef struct __tagGXNMHDR
{
  GXHWND    hwndFrom;
  GXUINT    idFrom;
  GXUINT    code;         // NM_ code
}GXNMHDR;
typedef GXNMHDR *GXLPNMHDR, *LPGXNMHDR;

//
// WM_STYLECHANGED, GXWM_STYLECHANGING 消息 lParam 指向的数据结构
//
typedef struct __tagGXSTYLESTRUCT
{
  GXDWORD   styleOld;
  GXDWORD   styleNew;
} GXSTYLESTRUCT, *GXLPSTYLESTRUCT, *LPGXSTYLESTRUCT;

//
// gxCreateWindowEx 函数使用的数据结构
//
typedef struct __tagGXWNDCLASSEX {
  GXUINT        cbSize;
  GXUINT        style;
  GXWNDPROC     lpfnWndProc;
  GXINT         cbClsExtra;
  GXINT         cbWndExtra;
  GXHINSTANCE   hInstance;
  GXHICON       hIcon;
  GXHCURSOR     hCursor;
  GXHBRUSH      hbrBackground;
  GXLPCWSTR     lpszMenuName;
  GXLPCWSTR     lpszClassName;
  GXHICON       hIconSm;
} GXWNDCLASSEX, *LPGXWNDCLASSEX, *GXLPWNDCLASSEX;

typedef struct GXWINDOWPOS {
  GXHWND    hwnd;
  GXHWND    hwndInsertAfter;
  int       x;
  int       y;
  int       cx;
  int       cy;
  GXUINT    flags;
} *GXLPWINDOWPOS, *LPGXWINDOWPOS;

//typedef struct __tagGXDC
//{
//  //CGraphics  *    pGraphics;
//  GXHWND        hBindWnd;      // 与这个DC绑定的窗口
//  GXUINT        uRefCount;
//  GXINT        wndOrgX;      // 窗口的原点,绘图函数指定的(0,0)将出现在(wndOrgX,wndOrgY)点
//  GXINT        wndOrgY;
//  GXCOLORREF      crTextBack;
//  GXCOLORREF      crText;
//  GXDWORD        flag;
//  //LPGXBRUSH      hBrush;
//} GXDC, *LPGXDC,*GXHDC,*GXHDC;
struct GXLOGBRUSH
{
  GXUINT        lbStyle;
  GXCOLORREF    lbColor;
  GXULONG_PTR   lbHatch;
};
typedef GXLOGBRUSH *GXLPLOGBRUSH;



//
// WM_NCCREATE， WM_CREATE 消息 lParam 指向的数据结构
typedef struct __tagCREATESTRUCTA {
  GXLPVOID      lpCreateParams;
  GXHINSTANCE   hInstance;
  GXHMENU       hMenu;
  GXHWND        hwndParent;
  int           cy;
  int           cx;
  int           y;
  int           x;
  GXLONG        style;
  GXLPCSTR      lpszName;
  GXLPCSTR      lpszClass;
  GXDWORD       dwExStyle;
} GXCREATESTRUCTA, *GXLPCREATESTRUCTA, *LPGXCREATESTRUCTA;

typedef struct __tagGXCREATESTRUCTW{
  GXLPVOID      lpCreateParams;
  GXHINSTANCE   hInstance;
  GXHMENU       hMenu;
  GXHWND        hwndParent;
  GXINT         cy;
  GXINT         cx;
  GXINT         y;
  GXINT         x;
  GXLONG        style;
  GXLPCWSTR     lpszName;
  GXLPCWSTR     lpszClass;
  GXDWORD       dwExStyle;
} GXCREATESTRUCTW, *GXLPCREATESTRUCTW;
#ifdef _UNICODE
#define GXCREATESTRUCT    GXCREATESTRUCTW
#define GXLPCREATESTRUCT  GXLPCREATESTRUCTW
#define GXLPCCREATESTRUCT const GXCREATESTRUCTW*
#else
#define GXCREATESTRUCT    GXCREATESTRUCTA
#define GXLPCREATESTRUCT  GXLPCREATESTRUCTA
#define GXLPCCREATESTRUCT const GXCREATESTRUCTA*
#endif // _UNICODE

//
// 滚动条使用的相关结构
// 
typedef struct __tagGXSCROLLINFO
{
  GXUINT    cbSize;
  GXUINT    fMask;
  GXINT     nMin;
  GXINT     nMax;
  GXUINT    nPage;
  GXINT     nPos;
  GXINT     nTrackPos;
}   GXSCROLLINFO, *GXLPSCROLLINFO,*LPGXSCROLLINFO;
typedef GXSCROLLINFO const *GXLPCSCROLLINFO;
typedef GXSCROLLINFO const *LPGXCSCROLLINFO;


typedef struct __tagGXTRACKMOUSEEVENT {
  GXDWORD cbSize;
  GXDWORD dwFlags;
  GXHWND  hwndTrack;
  GXDWORD dwHoverTime;
} GXTRACKMOUSEEVENT, *GXLPTRACKMOUSEEVENT, *LPGXTRACKMOUSEEVENT;

/*
 * Owner draw control types
 */
#define GXODT_MENU        1
#define GXODT_LISTBOX     2
#define GXODT_COMBOBOX    3
#define GXODT_BUTTON      4
#define GXODT_STATIC      5

/*
 * Owner draw actions
 */
#define GXODA_DRAWENTIRE  0x0001
#define GXODA_SELECT      0x0002
#define GXODA_FOCUS       0x0004

/*
 * Owner draw state
 */
#define GXODS_SELECTED    0x0001
#define GXODS_GRAYED      0x0002
#define GXODS_DISABLED    0x0004
#define GXODS_CHECKED     0x0008
#define GXODS_FOCUS       0x0010
#define GXODS_DEFAULT         0x0020
#define GXODS_COMBOBOXEDIT    0x1000
#define GXODS_HOTLIGHT        0x0040
#define GXODS_INACTIVE        0x0080
#define GXODS_NOACCEL         0x0100
#define GXODS_NOFOCUSRECT     0x0200

/*
 * MEASUREITEMSTRUCT for ownerdraw
 */
typedef struct tagGXMEASUREITEMSTRUCT {
    GXUINT       CtlType;
    GXUINT       CtlID;
    GXUINT       itemID;
    GXUINT       itemWidth;
    GXUINT       itemHeight;
    GXULONG_PTR  itemData;
} GXMEASUREITEMSTRUCT, *LPGXMEASUREITEMSTRUCT, *GXLPMEASUREITEMSTRUCT;

/*
* DRAWITEMSTRUCT for ownerdraw
*/
typedef struct __tagGXDRAWITEMSTRUCT {
  GXUINT        CtlType;
  GXUINT        CtlID;
  GXUINT        itemID;
  GXUINT        itemAction;
  GXUINT        itemState;
  GXHWND        hwndItem;
  GXHDC         hDC;
  GXRECT      rcItem;
  GXULONG      itemData;
} GXDRAWITEMSTRUCT, *LPGXDRAWITEMSTRUCT, *GXLPDRAWITEMSTRUCT;

/*
 * DELETEITEMSTRUCT for ownerdraw
 */
typedef struct __tagGXDELETEITEMSTRUCT {
    GXUINT       CtlType;
    GXUINT       CtlID;
    GXUINT       itemID;
    GXHWND       hwndItem;
    GXULONG_PTR  itemData;
} GXDELETEITEMSTRUCT, *LPGXDELETEITEMSTRUCT, *GXLPDELETEITEMSTRUCT;

/*
 * COMPAREITEMSTUCT for ownerdraw sorting
 */
typedef struct __tagGXCOMPAREITEMSTRUCT {
    GXUINT        CtlType;
    GXUINT        CtlID;
    GXHWND        hwndItem;
    GXUINT        itemID1;
    GXULONG_PTR   itemData1;
    GXUINT        itemID2;
    GXULONG_PTR   itemData2;
    GXDWORD       dwLocaleId;
} GXCOMPAREITEMSTRUCT, *LPGXCOMPAREITEMSTRUCT, *GXLPCOMPAREITEMSTRUCT;

typedef struct __tagGXMENUITEMINFOW
{
  GXUINT     cbSize;
  GXUINT     fMask;
  GXUINT     fType;         // used if MIIM_TYPE (4.0) or MIIM_FTYPE (>4.0)
  GXUINT     fState;        // used if MIIM_STATE
  GXUINT     wID;           // used if MIIM_ID
  GXHMENU    hSubMenu;      // used if MIIM_SUBMENU
  GXHBITMAP  hbmpChecked;   // used if MIIM_CHECKMARKS
  GXHBITMAP  hbmpUnchecked; // used if MIIM_CHECKMARKS
  GXULONG    dwItemData;    // used if MIIM_DATA
  GXLPWSTR   dwTypeData;    // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
  GXUINT     cch;           // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
  GXHBITMAP  hbmpItem;      // used if MIIM_BITMAP
}GXMENUITEMINFOW, *GXLPMENUITEMINFOW, *LPGXMENUITEMINFOW;

typedef struct tagGXMENUITEMINFOA
{
  GXUINT     cbSize;
  GXUINT     fMask;
  GXUINT     fType;         // used if MIIM_TYPE (4.0) or MIIM_FTYPE (>4.0)
  GXUINT     fState;        // used if MIIM_STATE
  GXUINT     wID;           // used if MIIM_ID
  GXHMENU    hSubMenu;      // used if MIIM_SUBMENU
  GXHBITMAP  hbmpChecked;   // used if MIIM_CHECKMARKS
  GXHBITMAP  hbmpUnchecked; // used if MIIM_CHECKMARKS
  GXULONG_PTR dwItemData;   // used if MIIM_DATA
  GXLPSTR    dwTypeData;    // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
  GXUINT     cch;           // used if MIIM_TYPE (4.0) or MIIM_STRING (>4.0)
  GXHBITMAP  hbmpItem;      // used if MIIM_BITMAP
}GXMENUITEMINFOA, *GXLPMENUITEMINFOA, *LPGXMENUITEMINFOA;

typedef struct __tagGXRGBQUAD {
  GXBYTE    rgbBlue;
  GXBYTE    rgbGreen;
  GXBYTE    rgbRed;
  GXBYTE    rgbReserved;
} GXRGBQUAD;

typedef struct __tagGXMDINEXTMENU
{
  GXHMENU   hmenuIn;
  GXHMENU   hmenuNext;
  GXHWND    hwndNext;
} GXMDINEXTMENU, * GXLPMDINEXTMENU, * LPGXMDINEXTMENU;

typedef struct __tagGXBITMAPINFOHEADER{
  GXDWORD      biSize;
  GXLONG       biWidth;
  GXLONG       biHeight;
  GXWORD       biPlanes;
  GXWORD       biBitCount;
  GXDWORD      biCompression;
  GXDWORD      biSizeImage;
  GXLONG       biXPelsPerMeter;
  GXLONG       biYPelsPerMeter;
  GXDWORD      biClrUsed;
  GXDWORD      biClrImportant;
} GXBITMAPINFOHEADER, *GXLPBITMAPINFOHEADER, *LPGXBITMAPINFOHEADER;

typedef struct __tagGXBITMAPINFO {
  GXBITMAPINFOHEADER    bmiHeader;
  GXRGBQUAD             bmiColors[1];
} GXBITMAPINFO, *GXLPBITMAPINFO, *LPGXBITMAPINFO;

struct _GXIMAGELIST
{
  GXDWORD       magic;                  /* 00: 'SAMX' */
  GXINT         cCurImage;              /* 04: ImageCount */
  GXINT         cMaxImage;              /* 08: maximages */
  GXINT         cGrow;                  /* 0c: cGrow */
  GXINT         cx;                     /* 10: cx */
  GXINT         cy;                     /* 14: cy */
  GXDWORD       x4;
  GXUINT        flags;                  /* 1c: flags */
  GXCOLORREF    clrFg;                  /* 20: foreground color */
  GXCOLORREF    clrBk;                  /* 24: background color */


  GXHBITMAP     hbmImage;               /* 30: images Bitmap */
  GXHBITMAP     hbmMask;                /* 34: masks  Bitmap */
  GXHDC         hdcImage;               /* 38: images MemDC  */
  GXHDC         hdcMask;                /* 3C: masks  MemDC  */
  GXINT         nOvlIdx[15];            /* 40: overlay images index */

  /* not yet found out */
  GXHBRUSH  hbrBlend25;
  GXHBRUSH  hbrBlend50;
  GXINT     cInitial;
  GXUINT    uBitsPixel;
};

typedef struct _GXIMAGELIST *GXHIMAGELIST,*LPGXIMAGELIST, *GXLPIMAGELIST;

typedef struct _GXIMAGELISTDRAWPARAMS 
{
  GXDWORD       cbSize;
  GXHIMAGELIST  himl;
  int           i;
  GXHDC         hdcDst;
  int           x;
  int           y;
  int           cx;
  int           cy;
  int           xBitmap;        // x offest from the upperleft of bitmap
  int           yBitmap;        // y offset from the upperleft of bitmap
  GXCOLORREF    rgbBk;
  GXCOLORREF    rgbFg;
  GXUINT        fStyle;
  GXDWORD       dwRop;
  GXDWORD       fState;
  GXDWORD       Frame;
  GXCOLORREF    crEffect;
} GXIMAGELISTDRAWPARAMS, *GXLPIMAGELISTDRAWPARAMS, *LPGXIMAGELISTDRAWPARAMS;

struct GXIMAGEINFO
{
  GXHBITMAP hbmImage;
  GXHBITMAP hbmMask;
  int     Unused1;
  int     Unused2;
  GXRECT    rcImage;
};
typedef GXIMAGEINFO*  GXLPIMAGEINFO;
typedef GXIMAGEINFO*  LPGXIMAGEINFO;


struct GXNMCUSTOMDRAW
{
  GXNMHDR   hdr;
  GXDWORD   dwDrawStage;
  GXHDC     hdc;
  GXRECT    rc;
  GXDWORD   dwItemSpec;  // this is control specific, but it's how to specify an item.  valid only with CDDS_ITEM bit set
  GXUINT    uItemState;
  GXLPARAM  lItemlParam;
};
typedef GXNMCUSTOMDRAW*    LPGXNMCUSTOMDRAW;
typedef GXNMCUSTOMDRAW*    GXLPNMCUSTOMDRAW;


typedef struct __tagGXNMLVCUSTOMDRAW
{
  GXNMCUSTOMDRAW nmcd;
  GXCOLORREF clrText;
  GXCOLORREF clrTextBk;
  int iSubItem;
  GXDWORD dwItemType;

  // Item custom draw
  GXCOLORREF clrFace;
  int iIconEffect;
  int iIconPhase;
  int iPartId;
  int iStateId;

  // Group Custom Draw
  GXRECT rcText;
  GXUINT uAlign;      // Alignment. Use LVGA_HEADER_CENTER, LVGA_HEADER_RIGHT, LVGA_HEADER_LEFT
} GXNMLVCUSTOMDRAW, *LPGXNMLVCUSTOMDRAW, *GXLPNMLVCUSTOMDRAW;

//
// GXUI List Notify Message
//
struct GXNMCUSTLISTCTRLCMD
{
  GXNMHDR hdr;
  GXHWND  hTmplItemWnd; // 自定义列表项目的窗口句柄
  GXINT   nListItem;    // 项目在列表中的索引，开始基于0
  GXINT   nCommand;     // WM_COMMAND的GXHIWORD(wParam);
};

struct GXNMLISTADAPTER
{
  GXNMHDR hdr;
  GXHWND  hTemplateWnd;
  GXUI::IListDataAdapter* pAdapter;
};

//////////////////////////////////////////////////////////////////////////
//
// Common Control struct
//

#define CLR_DEFAULT             0xFF000000L

#define GXIDC_DIVIDER                     106
#define GXIDC_DIVIDEROPEN                 107

enum {
  HP_HEADERITEM = 1,
  HP_HEADERITEMLEFT = 2,
  HP_HEADERITEMRIGHT = 3,
  HP_HEADERSORTARROW = 4,
};
enum {
  HIS_NORMAL = 1,
  HIS_HOT = 2,
  HIS_PRESSED = 3,
};

struct GXLVHITTESTINFO
{
  GXPOINT pt;
  GXUINT flags;
  int iItem;
  int iSubItem;    // this is was NOT in win95.  valid only for GXLVM_SUBITEMHITTEST
};
typedef GXLVHITTESTINFO *GXLPLVHITTESTINFO, *LPGXLVHITTESTINFO;

typedef struct __tagGXLVKEYDOWN
{
  GXNMHDR hdr;
  GXWORD wVKey;
  GXUINT flags;
} GXNMLVKEYDOWN, *GXLPNMLVKEYDOWN, *LPGXNMLVKEYDOWN;


typedef struct __tagGXNMITEMACTIVATE
{
  GXNMHDR   hdr;
  int       iItem;
  int       iSubItem;
  GXUINT    uNewState;
  GXUINT    uOldState;
  GXUINT    uChanged;
  GXPOINT   ptAction;
  GXLPARAM  lParam;
  GXUINT    uKeyFlags;
} GXNMITEMACTIVATE, *GXLPNMITEMACTIVATE, *LPGXNMITEMACTIVATE;

// key flags stored in uKeyFlags
#define LVKF_ALT       0x0001
#define LVKF_CONTROL   0x0002
#define LVKF_SHIFT     0x0004


typedef struct __tagGXNMLVODSTATECHANGE
{
  GXNMHDR hdr;
  int iFrom;
  int iTo;
  GXUINT uNewState;
  GXUINT uOldState;
} GXNMLVODSTATECHANGE, *GXLPNMLVODSTATECHANGE, *LPGXNMLVODSTATECHANGE;

typedef struct __tagGXLVITEMW
{
  GXUINT    mask;
  int       iItem;
  int       iSubItem;
  GXUINT    state;
  GXUINT    stateMask;
  GXLPWSTR  pszText;
  int       cchTextMax;
  int       iImage;
  GXLPARAM  lParam;
  int       iIndent;
  int       iGroupId;
  GXUINT    cColumns; // tile view columns
  GXUINT*   puColumns;
} GXLVITEMW, *GXLPLVITEMW, *LPGXLVITEMW;

typedef struct __tagGXLVDISPINFOW {
  GXNMHDR hdr;
  GXLVITEMW item;
} GXNMLVDISPINFOW, *GXLPNMLVDISPINFOW, *LPGXNMLVDISPINFOW;

typedef struct __tagGXNMLISTVIEW
{
  GXNMHDR   hdr;
  int       iItem;
  int       iSubItem;
  GXUINT    uNewState;
  GXUINT    uOldState;
  GXUINT    uChanged;
  GXPOINT   ptAction;
  GXLPARAM  lParam;
} GXNMLISTVIEW, *GXLPNMLISTVIEW, *LPGXNMLISTVIEW;

typedef struct
{
  GXCOLORREF clrBtnHighlight;       /* GXCOLOR_BTNHIGHLIGHT                  */
  GXCOLORREF clrBtnShadow;          /* GXCOLOR_BTNSHADOW                     */
  GXCOLORREF clrBtnText;            /* GXCOLOR_BTNTEXT                       */
  GXCOLORREF clrBtnFace;            /* GXCOLOR_BTNFACE                       */
  GXCOLORREF clrHighlight;          /* GXCOLOR_HIGHLIGHT                     */
  GXCOLORREF clrHighlightText;      /* GXCOLOR_HIGHLIGHTTEXT                 */
  GXCOLORREF clrHotTrackingColor;   /* GXCOLOR_HOTLIGHT                      */
  GXCOLORREF clr3dHilight;          /* GXCOLOR_3DHILIGHT                     */
  GXCOLORREF clr3dShadow;           /* GXCOLOR_3DSHADOW                      */
  GXCOLORREF clr3dDkShadow;         /* GXCOLOR_3DDKSHADOW                    */
  GXCOLORREF clr3dFace;             /* GXCOLOR_3DFACE                        */
  GXCOLORREF clrWindow;             /* GXCOLOR_WINDOW                        */
  GXCOLORREF clrWindowText;         /* GXCOLOR_WINDOWTEXT                    */
  GXCOLORREF clrGrayText;           /* GXCOLOR_GREYTEXT                      */
  GXCOLORREF clrActiveCaption;      /* GXCOLOR_ACTIVECAPTION                 */
  GXCOLORREF clrInfoBk;             /* GXCOLOR_INFOBK                        */
  GXCOLORREF clrInfoText;           /* GXCOLOR_INFOTEXT                      */
} GXCOMCTL32_SysColor;

typedef struct __tagGXNMLVCACHEHINT
{
  GXNMHDR   hdr;
  int     iFrom;
  int     iTo;
} GXNMLVCACHEHINT, *GXLPNMLVCACHEHINT, *LPGXNMLVCACHEHINT;


//typedef struct __tagGXNMTOOLTIPSCREATED
//{
//  GXNMHDR hdr;
//  GXHWND hwndToolTips;
//} GXNMTOOLTIPSCREATED, * GXLPNMTOOLTIPSCREATED;

typedef struct _tagGXHD_ITEMW
{
  GXUINT    mask;
  int       cxy;
  GXLPWSTR  pszText;
  GXHBITMAP hbm;
  int       cchTextMax;
  int       fmt;
  GXLPARAM  lParam;
  int       iImage;        // index of bitmap in ImageList
  int       iOrder;
  GXUINT    type;           // [in] filter type (defined what pvFilter is a pointer to)
  void *    pvFilter;       // [in] fillter data see above
} GXHDITEMW, *GXLPHDITEMW, *LPGXHDITEMW;
#define GXHDITEM GXHDITEMW
#define GXHD_ITEM GXHDITEM


typedef struct __tagGXNMHEADERW
{
  GXNMHDR     hdr;
  int         iItem;
  int         iButton;
  GXHDITEMW*  pitem;
} GXNMHEADERW, *GXLPNMHEADERW, *LPGXNMHEADERW;

typedef int (GXCALLBACK *GXPFNLVCOMPARE)(GXLPARAM, GXLPARAM, GXLPARAM);

typedef struct __tagGXNMHDDISPINFOW
{
  GXNMHDR   hdr;
  int     iItem;
  GXUINT    mask;
  GXLPWSTR  pszText;
  int     cchTextMax;
  int     iImage;
  GXLPARAM  lParam;
} GXNMHDDISPINFOW, *GXLPNMHDDISPINFOW;

typedef struct __tagGXNMHDDISPINFOA
{
  GXNMHDR   hdr;
  int     iItem;
  GXUINT    mask;
  GXLPSTR   pszText;
  int     cchTextMax;
  int     iImage;
  GXLPARAM  lParam;
} GXNMHDDISPINFOA, *GXLPNMHDDISPINFOA;

//-----------------------------------------------------------
//typedef struct _GXHD_ITEMA
//{
//  GXUINT    mask;
//  int     cxy;
//  GXLPSTR   pszText;
//  GXHBITMAP hbm;
//  int     cchTextMax;
//  int     fmt;
//  GXLPARAM  lParam;
//  int     iImage;        // index of bitmap in ImageList
//  int     iOrder;        // where to draw this item
//
//  GXUINT    type;           // [in] filter type (defined what pvFilter is a pointer to)
//  void *  pvFilter;       // [in] fillter data see above
//
//} GXHDITEMA, *GXLPHDITEMA;
//
//typedef struct _GXHD_ITEMW
//{
//  GXUINT    mask;
//  int     cxy;
//  GXLPWSTR   pszText;
//  GXHBITMAP hbm;
//  int     cchTextMax;
//  int     fmt;
//  GXLPARAM  lParam;
//
//  int     iImage;        // index of bitmap in ImageList
//  int     iOrder;
//
//  GXUINT    type;           // [in] filter type (defined what pvFilter is a pointer to)
//  void *  pvFilter;       // [in] fillter data see above
//} GXHDITEMW, *GXLPHDITEMW;
//
//#define GXHDITEM GXHDITEMW
//#define GXLPHDITEM GXLPHDITEMW
//-----------------------------------------------------------
#define GXNMHDDISPINFO            GXNMHDDISPINFOW
#define GXLPNMHDDISPINFO          GXLPNMHDDISPINFOW
//-----------------------------------------------------------
struct GXHDLAYOUT
{
  GXRECT *prc;
  GXWINDOWPOS *pwpos;
};
typedef GXHDLAYOUT*    GXLPHDLAYOUT;
typedef GXHDLAYOUT*    LPGXHDLAYOUT;


//-----------------------------------------------------------
#define DPAS_SORTED             0x0001
#define DPAS_INSERTBEFORE       0x0002
#define DPAS_INSERTAFTER        0x0004

//-----------------------------------------------------------
#define GXHDM_FIRST               0x1200      // Header messages

#define HHT_NOWHERE             0x0001
#define HHT_ONHEADER            0x0002
#define HHT_ONDIVIDER           0x0004
#define HHT_ONDIVOPEN           0x0008

#define HHT_ONFILTER            0x0010
#define HHT_ONFILTERBUTTON      0x0020

#define HHT_ABOVE               0x0100
#define HHT_BELOW               0x0200
#define HHT_TORIGHT             0x0400
#define HHT_TOLEFT              0x0800

#define GXHDM_GETITEMCOUNT        (GXHDM_FIRST + 0)
#define gxHeader_GetItemCount(hwndHD) \
  (int)GXSNDMSG((hwndHD), GXHDM_GETITEMCOUNT, 0, 0L)


#define GXHDM_INSERTITEMA         (GXHDM_FIRST + 1)

typedef struct _GXHD_HITTESTINFO
{
  GXPOINT pt;
  GXUINT flags;
  int iItem;
} GXHDHITTESTINFO, *GXLPHDHITTESTINFO, *LPGXHDHITTESTINFO;

#define GXHDM_HITTEST             (GXHDM_FIRST + 6)

#define GXHDM_GETITEMRECT         (GXHDM_FIRST + 7)
#define gxHeader_GetItemRect(hwnd, iItem, lprc) \
  (GXBOOL)GXSNDMSG((hwnd), GXHDM_GETITEMRECT, (GXWPARAM)(iItem), (GXLPARAM)(lprc))

#define GXHDM_SETIMAGELIST        (GXHDM_FIRST + 8)
#define gxHeader_SetImageList(hwnd, himl) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXHDM_SETIMAGELIST, 0, (GXLPARAM)(himl))

#define GXHDM_GETIMAGELIST        (GXHDM_FIRST + 9)
#define gxHeader_GetImageList(hwnd) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXHDM_GETIMAGELIST, 0, 0)


#define GXHDM_ORDERTOINDEX        (GXHDM_FIRST + 15)
#define gxHeader_OrderToIndex(hwnd, i) \
  (int)GXSNDMSG((hwnd), GXHDM_ORDERTOINDEX, (GXWPARAM)(i), 0)

#define GXHDM_CREATEDRAGIMAGE     (GXHDM_FIRST + 16)  // wparam = which item (by index)
#define gxHeader_CreateDragImage(hwnd, i) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXHDM_CREATEDRAGIMAGE, (GXWPARAM)(i), 0)

#define GXHDM_GETORDERARRAY       (GXHDM_FIRST + 17)
#define gxHeader_GetOrderArray(hwnd, iCount, lpi) \
  (GXBOOL)GXSNDMSG((hwnd), GXHDM_GETORDERARRAY, (GXWPARAM)(iCount), (GXLPARAM)(lpi))

#define GXHDM_SETORDERARRAY       (GXHDM_FIRST + 18)
#define gxHeader_SetOrderArray(hwnd, iCount, lpi) \
  (GXBOOL)SNDMSG((hwnd), GXHDM_SETORDERARRAY, (GXWPARAM)(iCount), (GXLPARAM)(lpi))

#define GXHDM_SETHOTDIVIDER          (GXHDM_FIRST + 19)
#define gxHeader_SetHotDivider(hwnd, fPos, dw) \
  (int)GXSNDMSG((hwnd), GXHDM_SETHOTDIVIDER, (GXWPARAM)(fPos), (GXLPARAM)(dw))

#define GXHDM_SETBITMAPMARGIN          (GXHDM_FIRST + 20)
#define gxHeader_SetBitmapMargin(hwnd, iWidth) \
  (int)GXSNDMSG((hwnd), GXHDM_SETBITMAPMARGIN, (GXWPARAM)(iWidth), 0)

#define GXHDM_GETBITMAPMARGIN          (GXHDM_FIRST + 21)
#define gxHeader_GetBitmapMargin(hwnd) \
  (int)GXSNDMSG((hwnd), GXHDM_GETBITMAPMARGIN, 0, 0)

#define GXHDM_GETUNICODEFORMAT   GXCCM_GETUNICODEFORMAT
#define gxHeader_GetUnicodeFormat(hwnd)  \
  (GXBOOL)GXSNDMSG((hwnd), GXHDM_GETUNICODEFORMAT, 0, 0)

#define GXHDM_INSERTITEMW         (GXHDM_FIRST + 10)

#define GXHDM_INSERTITEM          GXHDM_INSERTITEMW

#define gxHeader_InsertItem(hwndHD, i, phdi) \
  (int)GXSNDMSG((hwndHD), GXHDM_INSERTITEM, (GXWPARAM)(int)(i), (GXLPARAM)(const GXHD_ITEM *)(phdi))
#define gxHeader_SetItemW(hwndHD, i, phdi) \
  (GXBOOL)GXSNDMSG((hwndHD), GXHDM_SETITEM, (GXWPARAM)(int)(i), (GXLPARAM)(const GXHD_ITEM *)(phdi))


#define GXHDM_DELETEITEM          (GXHDM_FIRST + 2)
#define gxHeader_DeleteItem(hwndHD, i) \
  (GXBOOL)GXSNDMSG((hwndHD), GXHDM_DELETEITEM, (GXWPARAM)(int)(i), 0L)


#define GXHDM_GETITEMA            (GXHDM_FIRST + 3)
#define GXHDM_GETITEMW            (GXHDM_FIRST + 11)

#define GXHDM_GETITEM             GXHDM_GETITEMW

#define gxHeader_GetItemW(hwndHD, i, phdi) \
  (GXBOOL)GXSNDMSG((hwndHD), GXHDM_GETITEM, (GXWPARAM)(int)(i), (GXLPARAM)(GXHD_ITEM *)(phdi))

#define GXHDM_SETUNICODEFORMAT   GXCCM_SETUNICODEFORMAT
#define gxHeader_SetUnicodeFormat(hwnd, fUnicode)  \
  (GXBOOL)GXSNDMSG((hwnd), GXHDM_SETUNICODEFORMAT, (GXWPARAM)(fUnicode), 0)

#define GXHDM_LAYOUT              (GXHDM_FIRST + 5)

#define GXHDM_SETITEMA            (GXHDM_FIRST + 4)
#define GXHDM_SETITEMW            (GXHDM_FIRST + 12)

#define GXHDM_SETITEM             GXHDM_SETITEMW
//-----------------------------------------------------------
#define HDN_FIRST               (0U-300U)       // header
#define HDN_LAST                (0U-399U)

#define HDN_ITEMCHANGINGA       (HDN_FIRST-0)
#define HDN_ITEMCHANGINGW       (HDN_FIRST-20)
#define HDN_ITEMCHANGEDA        (HDN_FIRST-1)
#define HDN_ITEMCHANGEDW        (HDN_FIRST-21)
#define HDN_ITEMCLICKA          (HDN_FIRST-2)
#define HDN_ITEMCLICKW          (HDN_FIRST-22)
#define HDN_ITEMDBLCLICKA       (HDN_FIRST-3)
#define HDN_ITEMDBLCLICKW       (HDN_FIRST-23)
#define HDN_DIVIDERDBLCLICKA    (HDN_FIRST-5)
#define HDN_DIVIDERDBLCLICKW    (HDN_FIRST-25)
#define HDN_BEGINTRACKA         (HDN_FIRST-6)
#define HDN_BEGINTRACKW         (HDN_FIRST-26)
#define HDN_ENDTRACKA           (HDN_FIRST-7)
#define HDN_ENDTRACKW           (HDN_FIRST-27)
#define HDN_TRACKA              (HDN_FIRST-8)
#define HDN_TRACKW              (HDN_FIRST-28)

#define HDN_GETDISPINFOA        (HDN_FIRST-9)
#define HDN_GETDISPINFOW        (HDN_FIRST-29)
#define HDN_BEGINDRAG           (HDN_FIRST-10)
#define HDN_ENDDRAG             (HDN_FIRST-11)


#define HDN_FILTERCHANGE        (HDN_FIRST-12)
#define HDN_FILTERBTNCLICK      (HDN_FIRST-13)


#define HDN_ITEMCHANGING         HDN_ITEMCHANGINGW
#define HDN_ITEMCHANGED          HDN_ITEMCHANGEDW
#define HDN_ITEMCLICK            HDN_ITEMCLICKW
#define HDN_ITEMDBLCLICK         HDN_ITEMDBLCLICKW
#define HDN_DIVIDERDBLCLICK      HDN_DIVIDERDBLCLICKW
#define HDN_BEGINTRACK           HDN_BEGINTRACKW
#define HDN_ENDTRACK             HDN_ENDTRACKW
#define HDN_TRACK                HDN_TRACKW

#define HDN_GETDISPINFO          HDN_GETDISPINFOW
//-----------------------------------------------------------
#define HDI_WIDTH               0x0001
#define HDI_HEIGHT              HDI_WIDTH
#define HDI_TEXT                0x0002
#define HDI_FORMAT              0x0004
#define HDI_LPARAM              0x0008
#define HDI_BITMAP              0x0010

#define HDI_IMAGE               0x0020
#define HDI_DI_SETITEM          0x0040
#define HDI_ORDER               0x0080

#define HDI_FILTER              0x0100
//-----------------------------------------------------------

#define HDF_LEFT                0x0000
#define HDF_RIGHT               0x0001
#define HDF_CENTER              0x0002
#define HDF_JUSTIFYMASK         0x0003
#define HDF_RTLREADING          0x0004

#define HDF_OWNERDRAW           0x8000
#define HDF_STRING              0x4000
#define HDF_BITMAP              0x2000

#define HDF_BITMAP_ON_RIGHT     0x1000
#define HDF_IMAGE               0x0800

#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200

//-----------------------------------------------------------
// begin_r_commctrl

#define HDS_HORZ                0x0000
#define HDS_BUTTONS             0x0002

#define HDS_HOTTRACK            0x0004

#define HDS_HIDDEN              0x0008


#define HDS_DRAGDROP            0x0040
#define HDS_FULLDRAG            0x0080

#define HDS_FILTERBAR           0x0100



#define HDS_FLAT                0x0200
// end_r_commctrl

//-----------------------------------------------------------

#define GXLVNI_ALL                0x0000
#define GXLVNI_FOCUSED            0x0001
#define GXLVNI_SELECTED           0x0002
#define GXLVNI_CUT                0x0004
#define GXLVNI_DROPHILITED        0x0008

#define GXLVNI_ABOVE              0x0100
#define GXLVNI_BELOW              0x0200
#define GXLVNI_TOLEFT             0x0400
#define GXLVNI_TORIGHT            0x0800

//-----------------------------------------------------------
#define LVFI_PARAM              0x0001
#define LVFI_STRING             0x0002
#define LVFI_PARTIAL            0x0008
#define LVFI_WRAP               0x0020
#define LVFI_NEARESTXY          0x0040

//-----------------------------------------------------------
#define LVCF_FMT                0x0001
#define LVCF_WIDTH              0x0002
#define LVCF_TEXT               0x0004
#define LVCF_SUBITEM            0x0008
#define LVCF_IMAGE              0x0010
#define LVCF_ORDER              0x0020
//-----------------------------------------------------------

#define GXLVIF_TEXT               0x0001
#define GXLVIF_IMAGE              0x0002
#define GXLVIF_PARAM              0x0004
#define GXLVIF_STATE              0x0008

#define GXLVIF_INDENT             0x0010
#define GXLVIF_NORECOMPUTE        0x0800

#define GXLVIF_GROUPID            0x0100
#define GXLVIF_COLUMNS            0x0200
#define GXLVIF_DI_SETITEM         0x1000
//-----------------------------------------------------------

#define LVIS_FOCUSED            0x0001
#define LVIS_SELECTED           0x0002
#define LVIS_CUT                0x0004
#define LVIS_DROPHILITED        0x0008
#define LVIS_GLOW               0x0010
#define LVIS_ACTIVATING         0x0020

#define LVIS_OVERLAYMASK        0x0F00
#define LVIS_STATEIMAGEMASK     0xF000

//-----------------------------------------------------------

#define LVIR_BOUNDS             0
#define LVIR_ICON               1
#define LVIR_LABEL              2
#define LVIR_SELECTBOUNDS       3


//-----------------------------------------------------------
#define LVCFMT_LEFT             0x0000
#define LVCFMT_RIGHT            0x0001
#define LVCFMT_CENTER           0x0002
#define LVCFMT_JUSTIFYMASK      0x0003

#define LVCFMT_IMAGE            0x0800
#define LVCFMT_BITMAP_ON_RIGHT  0x1000
#define LVCFMT_COL_HAS_IMAGES   0x8000

//-----------------------------------------------------------
#define LVSIL_NORMAL            0
#define LVSIL_SMALL             1
#define LVSIL_STATE             2
//-----------------------------------------------------------

#define LVN_FIRST               (0U-100U)       // listview
#define LVN_LAST                (0U-199U)

#define LVN_ITEMCHANGING        (LVN_FIRST-0)
#define LVN_ITEMCHANGED         (LVN_FIRST-1)
#define LVN_INSERTITEM          (LVN_FIRST-2)
#define LVN_DELETEITEM          (LVN_FIRST-3)
#define LVN_DELETEALLITEMS      (LVN_FIRST-4)
#define LVN_BEGINLABELEDITA     (LVN_FIRST-5)
#define LVN_BEGINLABELEDITW     (LVN_FIRST-75)
#define LVN_ENDLABELEDITA       (LVN_FIRST-6)
#define LVN_ENDLABELEDITW       (LVN_FIRST-76)
#define LVN_COLUMNCLICK         (LVN_FIRST-8)
#define LVN_BEGINDRAG           (LVN_FIRST-9)
#define LVN_BEGINRDRAG          (LVN_FIRST-11)

#define LVN_ODCACHEHINT         (LVN_FIRST-13)
#define LVN_ODFINDITEMA         (LVN_FIRST-52)
#define LVN_KEYDOWN             (LVN_FIRST-55)
#define LVN_ODFINDITEMW         (LVN_FIRST-79)

#define LVN_ITEMACTIVATE        (LVN_FIRST-14)
#define LVN_ODSTATECHANGED      (LVN_FIRST-15)

#define LVN_ODFINDITEM          LVN_ODFINDITEMW


#define LVN_HOTTRACK            (LVN_FIRST-21)

#define LVN_GETDISPINFOA        (LVN_FIRST-50)
#define LVN_GETDISPINFOW        (LVN_FIRST-77)
#define LVN_SETDISPINFOA        (LVN_FIRST-51)
#define LVN_SETDISPINFOW        (LVN_FIRST-78)

#define LVN_BEGINLABELEDIT      LVN_BEGINLABELEDITW
#define LVN_ENDLABELEDIT        LVN_ENDLABELEDITW
#define LVN_GETDISPINFO         LVN_GETDISPINFOW
#define LVN_SETDISPINFO         LVN_SETDISPINFOW

#define LVN_GETINFOTIPA          (LVN_FIRST-57)
#define LVN_GETINFOTIPW          (LVN_FIRST-58)
//-----------------------------------------------------------
#define GXLVM_FIRST               0x1000      // ListView messages

#define GXCCM_FIRST               0x2000      // Common control shared messages
#define GXCCM_SETUNICODEFORMAT    (GXCCM_FIRST + 5)
#define GXCCM_GETUNICODEFORMAT    (GXCCM_FIRST + 6)


#define GXLVM_SETUNICODEFORMAT     GXCCM_SETUNICODEFORMAT
#define gxListView_SetUnicodeFormat(hwnd, fUnicode)  \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETUNICODEFORMAT, (GXWPARAM)(fUnicode), 0)

#define GXLVM_GETUNICODEFORMAT     GXCCM_GETUNICODEFORMAT
#define gxListView_GetUnicodeFormat(hwnd)  \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETUNICODEFORMAT, 0, 0)


#define GXLVM_GETBKCOLOR          (GXLVM_FIRST + 0)
#define gxListView_GetBkColor(hwnd)  \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_GETBKCOLOR, 0, 0L)

#define GXLVM_SETBKCOLOR          (GXLVM_FIRST + 1)
#define gxListView_SetBkColor(hwnd, clrBk) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETBKCOLOR, 0, (GXLPARAM)(GXCOLORREF)(clrBk))

#define GXLVM_GETIMAGELIST        (GXLVM_FIRST + 2)
#define gxListView_GetImageList(hwnd, iImageList) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXLVM_GETIMAGELIST, (GXWPARAM)(GXINT)(iImageList), 0L)

#define LVSIL_NORMAL            0
#define LVSIL_SMALL             1
#define LVSIL_STATE             2

#define GXLVM_SETIMAGELIST        (GXLVM_FIRST + 3)
#define gxListView_SetImageList(hwnd, himl, iImageList) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXLVM_SETIMAGELIST, (GXWPARAM)(iImageList), (GXLPARAM)(GXHIMAGELIST)(himl))

#define GXLVM_GETITEMCOUNT        (GXLVM_FIRST + 4)
#define gxListView_GetItemCount(hwnd) \
  (int)GXSNDMSG((hwnd), GXLVM_GETITEMCOUNT, 0, 0L)


#define GXLVIF_TEXT               0x0001
#define GXLVIF_IMAGE              0x0002
#define GXLVIF_PARAM              0x0004
#define GXLVIF_STATE              0x0008

#define GXLVIF_INDENT             0x0010
#define GXLVIF_NORECOMPUTE        0x0800


#define GXLVIF_GROUPID            0x0100
#define GXLVIF_COLUMNS            0x0200


#define LVIS_FOCUSED            0x0001
#define LVIS_SELECTED           0x0002
#define LVIS_CUT                0x0004
#define LVIS_DROPHILITED        0x0008
#define LVIS_GLOW               0x0010
#define LVIS_ACTIVATING         0x0020

#define LVIS_OVERLAYMASK        0x0F00
#define LVIS_STATEIMAGEMASK     0xF000

#define INDEXTOSTATEIMAGEMASK(i) ((i) << 12)


#define I_INDENTCALLBACK        (-1)
//#define LV_ITEMA LVITEMA
//#define LV_ITEMW LVITEMW

//#define tagLVITEMA    _LV_ITEMA
//#define LVITEMA       LV_ITEMA
//#define tagLVITEMW    _LV_ITEMW
//#define LVITEMW       LV_ITEMW



#define I_GROUPIDCALLBACK   (-1)
#define I_GROUPIDNONE       (-2)

#define GXLV_ITEM GXLVITEMW

#define LVITEMA_V1_SIZE CCSIZEOF_STRUCT(LVITEMA, lParam)
#define LVITEMW_V1_SIZE CCSIZEOF_STRUCT(LVITEMW, lParam)

//-----------------------------------------------------------
#define WC_LISTVIEWA            "SysListView32"
#define WC_LISTVIEWW            L"SysListView32"
#define WC_LISTVIEW             WC_LISTVIEWW

#define WC_HEADERA              "SysHeader32"
#define WC_HEADERW              L"SysHeader32"
#define WC_HEADER               WC_HEADERW

//#define TOOLTIPS_CLASSW         L"tooltips_class32"
//#define TOOLTIPS_CLASSA         "tooltips_class32"
//#define TOOLTIPS_CLASS          TOOLTIPS_CLASSW
//-----------------------------------------------------------

//typedef struct tagLVITEMA
//{
//  GXUINT mask;
//  int iItem;
//  int iSubItem;
//  GXUINT state;
//  GXUINT stateMask;
//  GXLPSTR pszText;
//  int cchTextMax;
//  int iImage;
//  GXLPARAM lParam;
//
//  int iIndent;
//
//
//  int iGroupId;
//  GXUINT cColumns; // tile view columns
//  PUINT puColumns;
//
//} LVITEMA, *LPLVITEMA;

//typedef struct tagGXLVITEMW
//{
//  GXUINT mask;
//  int iItem;
//  int iSubItem;
//  GXUINT state;
//  GXUINT stateMask;
//  GXLPWSTR pszText;
//  int cchTextMax;
//  int iImage;
//  GXLPARAM lParam;
//
//  int iIndent;
//
//
//  int iGroupId;
//  GXUINT cColumns; // tile view columns
//  GXUINT* puColumns;
//
//} GXLVITEMW, *GXLPLVITEMW, *LPGXLVITEMW;



#define GXLVITEM    GXLVITEMW
#define GXLPLVITEM  GXLPLVITEMW
#define LVITEM_V1_SIZE LVITEMW_V1_SIZE

//#define LVITEM    LVITEMA
//#define LPLVITEM  LPLVITEMA
//#define LVITEM_V1_SIZE LVITEMA_V1_SIZE



#define GXLPSTR_TEXTCALLBACKW     ((GXLPWSTR)-1L)
#define GXLPSTR_TEXTCALLBACKA     ((GXLPSTR)-1L)

#define GXLPSTR_TEXTCALLBACK      LPSTR_TEXTCALLBACKW

//#define LPSTR_TEXTCALLBACK      LPSTR_TEXTCALLBACKA


#define GXI_IMAGECALLBACK         (-1)

#define GXI_IMAGENONE             (-2)



// For tileview
#define GXI_COLUMNSCALLBACK       ((GXUINT)-1)


#define GXLVM_GETITEMA            (GXLVM_FIRST + 5)
#define GXLVM_GETITEMW            (GXLVM_FIRST + 75)

#define GXLVM_GETITEM             GXLVM_GETITEMW

//#define GXLVM_GETITEM             GXLVM_GETITEMA


#define gxListView_GetItem(hwnd, pitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETITEM, 0, (GXLPARAM)(LV_ITEM *)(pitem))


#define GXLVM_SETITEMA            (GXLVM_FIRST + 6)
#define GXLVM_SETITEMW            (GXLVM_FIRST + 76)

#define GXLVM_SETITEM             GXLVM_SETITEMW

//#define GXLVM_SETITEM             GXLVM_SETITEMA


#define gxListView_SetItem(hwnd, pitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETITEM, 0, (GXLPARAM)(const LV_ITEM *)(pitem))


#define GXLVM_INSERTITEMA         (GXLVM_FIRST + 7)
#define GXLVM_INSERTITEMW         (GXLVM_FIRST + 77)

#define GXLVM_INSERTITEM          GXLVM_INSERTITEMW

//#define GXLVM_INSERTITEM          GXLVM_INSERTITEMA

#define gxListView_InsertItem(hwnd, pitem)   \
  (int)GXSNDMSG((hwnd), GXLVM_INSERTITEM, 0, (GXLPARAM)(const LV_ITEM *)(pitem))


#define GXLVM_DELETEITEM          (GXLVM_FIRST + 8)
#define gxListView_DeleteItem(hwnd, i) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_DELETEITEM, (GXWPARAM)(int)(i), 0L)


#define GXLVM_DELETEALLITEMS      (GXLVM_FIRST + 9)
#define gxListView_DeleteAllItems(hwnd) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_DELETEALLITEMS, 0, 0L)


#define GXLVM_GETCALLBACKMASK     (GXLVM_FIRST + 10)
#define gxListView_GetCallbackMask(hwnd) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETCALLBACKMASK, 0, 0)


#define GXLVM_SETCALLBACKMASK     (GXLVM_FIRST + 11)
#define gxListView_SetCallbackMask(hwnd, mask) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETCALLBACKMASK, (GXWPARAM)(GXUINT)(mask), 0)


#define GXLVNI_ALL                0x0000
#define GXLVNI_FOCUSED            0x0001
#define GXLVNI_SELECTED           0x0002
#define GXLVNI_CUT                0x0004
#define GXLVNI_DROPHILITED        0x0008

#define GXLVNI_ABOVE              0x0100
#define GXLVNI_BELOW              0x0200
#define GXLVNI_TOLEFT             0x0400
#define GXLVNI_TORIGHT            0x0800


#define GXLVM_GETNEXTITEM         (GXLVM_FIRST + 12)
#define gxListView_GetNextItem(hwnd, i, flags) \
  (int)GXSNDMSG((hwnd), GXLVM_GETNEXTITEM, (GXWPARAM)(int)(i), GXMAKELPARAM((flags), 0))


#define LVFI_PARAM              0x0001
#define LVFI_STRING             0x0002
#define LVFI_PARTIAL            0x0008
#define LVFI_WRAP               0x0020
#define LVFI_NEARESTXY          0x0040


#define GXLV_FINDINFOA    GXLVFINDINFOA
#define GXLV_FINDINFOW    GXLVFINDINFOW

#define tagGXLVFINDINFOA  _GXLV_FINDINFOA
#define    GXLVFINDINFOA   GXLV_FINDINFOA
#define tagGXLVFINDINFOW  _GXLV_FINDINFOW
#define    GXLVFINDINFOW   GXLV_FINDINFOW


#define GXLV_FINDINFO  GXLVFINDINFO

//typedef struct tagLVFINDINFOA
//{
//  GXUINT flags;
//  GXLPCSTR psz;
//  GXLPARAM lParam;
//  GXPOINT pt;
//  GXUINT vkDirection;
//} LVFINDINFOA, *LPFINDINFOA;

struct GXLVFINDINFOW
{
  GXUINT flags;
  GXLPCWSTR psz;
  GXLPARAM lParam;
  GXPOINT pt;
  GXUINT vkDirection;
};
typedef GXLVFINDINFOW *GXLPFINDINFOW;


#define  GXLVFINDINFO            GXLVFINDINFOW

//#define  GXLVFINDINFO            GXLVFINDINFOA


#define GXLVM_FINDITEMA           (GXLVM_FIRST + 13)
#define GXLVM_FINDITEMW           (GXLVM_FIRST + 83)

#define  GXLVM_FINDITEM           GXLVM_FINDITEMW

//#define  GXLVM_FINDITEM           GXLVM_FINDITEMA


#define gxListView_FindItem(hwnd, iStart, plvfi) \
  (int)GXSNDMSG((hwnd), GXLVM_FINDITEM, (GXWPARAM)(int)(iStart), (GXLPARAM)(const GXLV_FINDINFO *)(plvfi))

#define LVIR_BOUNDS             0
#define LVIR_ICON               1
#define LVIR_LABEL              2
#define LVIR_SELECTBOUNDS       3


#define GXLVM_GETITEMRECT         (GXLVM_FIRST + 14)
#define gxListView_GetItemRect(hwnd, i, prc, code) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETITEMRECT, (GXWPARAM)(int)(i), \
  ((prc) ? (((GXRECT *)(prc))->left = (code),(GXLPARAM)(GXRECT *)(prc)) : (GXLPARAM)(GXRECT *)NULL))


#define GXLVM_SETITEMPOSITION     (GXLVM_FIRST + 15)
#define gxListView_SetItemPosition(hwndLV, i, x, y) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_SETITEMPOSITION, (GXWPARAM)(int)(i), GXMAKELPARAM((x), (y)))


#define GXLVM_GETITEMPOSITION     (GXLVM_FIRST + 16)
#define gxListView_GetItemPosition(hwndLV, i, ppt) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_GETITEMPOSITION, (GXWPARAM)(int)(i), (GXLPARAM)(GXPOINT *)(ppt))


#define GXLVM_GETSTRINGWIDTHA     (GXLVM_FIRST + 17)
#define GXLVM_GETSTRINGWIDTHW     (GXLVM_FIRST + 87)

#define  GXLVM_GETSTRINGWIDTH     GXLVM_GETSTRINGWIDTHW

//#define  GXLVM_GETSTRINGWIDTH     GXLVM_GETSTRINGWIDTHA


#define gxListView_GetStringWidth(hwndLV, psz) \
  (int)GXSNDMSG((hwndLV), GXLVM_GETSTRINGWIDTH, 0, (GXLPARAM)(GXLPCWSTR)(psz))


#define GXLVHT_NOWHERE            0x0001
#define GXLVHT_ONITEMICON         0x0002
#define GXLVHT_ONITEMLABEL        0x0004
#define GXLVHT_ONITEMSTATEICON    0x0008
#define GXLVHT_ONITEM             (GXLVHT_ONITEMICON | GXLVHT_ONITEMLABEL | GXLVHT_ONITEMSTATEICON)

#define GXLVHT_ABOVE              0x0008
#define GXLVHT_BELOW              0x0010
#define GXLVHT_TORIGHT            0x0020
#define GXLVHT_TOLEFT             0x0040



#define LV_HITTESTINFO LVHITTESTINFO

#define tagLVHITTESTINFO  _LV_HITTESTINFO
#define    LVHITTESTINFO   LV_HITTESTINFO


#define LVHITTESTINFO_V1_SIZE CCSIZEOF_STRUCT(LVHITTESTINFO, iItem)

#define GXLVM_HITTEST             (GXLVM_FIRST + 18)
#define gxListView_HitTest(hwndLV, pinfo) \
  (int)GXSNDMSG((hwndLV), GXLVM_HITTEST, 0, (GXLPARAM)(LV_HITTESTINFO *)(pinfo))


#define GXLVM_ENSUREVISIBLE       (GXLVM_FIRST + 19)
#define gxListView_EnsureVisible(hwndLV, i, fPartialOK) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_ENSUREVISIBLE, (GXWPARAM)(int)(i), GXMAKELPARAM((fPartialOK), 0))


#define GXLVM_SCROLL              (GXLVM_FIRST + 20)
#define gxListView_Scroll(hwndLV, dx, dy) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_SCROLL, (GXWPARAM)(int)(dx), (GXLPARAM)(int)(dy))


#define GXLVM_REDRAWITEMS         (GXLVM_FIRST + 21)
#define gxListView_RedrawItems(hwndLV, iFirst, iLast) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_REDRAWITEMS, (GXWPARAM)(int)(iFirst), (GXLPARAM)(int)(iLast))


#define LVA_DEFAULT             0x0000
#define LVA_ALIGNLEFT           0x0001
#define LVA_ALIGNTOP            0x0002
#define LVA_SNAPTOGRID          0x0005


#define GXLVM_ARRANGE             (GXLVM_FIRST + 22)
#define gxListView_Arrange(hwndLV, code) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_ARRANGE, (GXWPARAM)(GXUINT)(code), 0L)


#define GXLVM_EDITLABELA          (GXLVM_FIRST + 23)
#define GXLVM_EDITLABELW          (GXLVM_FIRST + 118)

#define GXLVM_EDITLABEL           GXLVM_EDITLABELW

//#define GXLVM_EDITLABEL           GXLVM_EDITLABELA


#define gxListView_EditLabel(hwndLV, i) \
  (GXHWND)GXSNDMSG((hwndLV), GXLVM_EDITLABEL, (GXWPARAM)(int)(i), 0L)


#define GXLVM_GETEDITCONTROL      (GXLVM_FIRST + 24)
#define gxListView_GetEditControl(hwndLV) \
  (GXHWND)GXSNDMSG((hwndLV), GXLVM_GETEDITCONTROL, 0, 0L)



#define GXLV_COLUMNA      GXLVCOLUMNA
#define GXLV_COLUMNW      GXLVCOLUMNW

#define tagGXLVCOLUMNA    _GXLV_COLUMNA
#define    GXLVCOLUMNA     GXLV_COLUMNA
#define tagGXLVCOLUMNW    _GXLV_COLUMNW
#define    GXLVCOLUMNW     GXLV_COLUMNW


#define GXLV_COLUMN       GXLVCOLUMN

#define GXLVCOLUMNA_V1_SIZE GXCCSIZEOF_STRUCT(LVCOLUMNA, iSubItem)
#define GXLVCOLUMNW_V1_SIZE GXCCSIZEOF_STRUCT(LVCOLUMNW, iSubItem)
//
//typedef struct tagLVCOLUMNA
//{
//  GXUINT mask;
//  int fmt;
//  int cx;
//  GXLPSTR pszText;
//  int cchTextMax;
//  int iSubItem;
//
//  int iImage;
//  int iOrder;
//
//} LVCOLUMNA, *LPLVCOLUMNA;

struct GXLVCOLUMNW
{
  GXUINT mask;
  int fmt;
  int cx;
  GXLPWSTR pszText;
  int cchTextMax;
  int iSubItem;

  int iImage;
  int iOrder;
};

typedef GXLVCOLUMNW *GXLPLVCOLUMNW;


#define  GXLVCOLUMN               GXLVCOLUMNW
#define  GXLPLVCOLUMN             GXLPLVCOLUMNW
#define GXLVCOLUMN_V1_SIZE GXLVCOLUMNW_V1_SIZE

//#define  LVCOLUMN               LVCOLUMNA
//#define  LPLVCOLUMN             LPLVCOLUMNA
//#define LVCOLUMN_V1_SIZE LVCOLUMNA_V1_SIZE



#define LVCF_FMT                0x0001
#define LVCF_WIDTH              0x0002
#define LVCF_TEXT               0x0004
#define LVCF_SUBITEM            0x0008

#define LVCF_IMAGE              0x0010
#define LVCF_ORDER              0x0020


#define LVCFMT_LEFT             0x0000
#define LVCFMT_RIGHT            0x0001
#define LVCFMT_CENTER           0x0002
#define LVCFMT_JUSTIFYMASK      0x0003


#define LVCFMT_IMAGE            0x0800
#define LVCFMT_BITMAP_ON_RIGHT  0x1000
#define LVCFMT_COL_HAS_IMAGES   0x8000


#define GXLVM_GETCOLUMNA          (GXLVM_FIRST + 25)
#define GXLVM_GETCOLUMNW          (GXLVM_FIRST + 95)

#define  GXLVM_GETCOLUMN          GXLVM_GETCOLUMNW

//#define  GXLVM_GETCOLUMN          GXLVM_GETCOLUMNA


#define gxListView_GetColumn(hwnd, iCol, pcol) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETCOLUMN, (GXWPARAM)(int)(iCol), (GXLPARAM)(LV_COLUMN *)(pcol))


#define GXLVM_SETCOLUMNA          (GXLVM_FIRST + 26)
#define GXLVM_SETCOLUMNW          (GXLVM_FIRST + 96)

#define  GXLVM_SETCOLUMN          GXLVM_SETCOLUMNW

//#define  GXLVM_SETCOLUMN          GXLVM_SETCOLUMNA


#define gxListView_SetColumn(hwnd, iCol, pcol) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETCOLUMN, (GXWPARAM)(int)(iCol), (GXLPARAM)(const LV_COLUMN *)(pcol))


#define GXLVM_INSERTCOLUMNA       (GXLVM_FIRST + 27)
#define GXLVM_INSERTCOLUMNW       (GXLVM_FIRST + 97)

#   define  GXLVM_INSERTCOLUMN    GXLVM_INSERTCOLUMNW

//#   define  GXLVM_INSERTCOLUMN    GXLVM_INSERTCOLUMNA


#define gxListView_InsertColumn(hwnd, iCol, pcol) \
  (int)GXSNDMSG((hwnd), GXLVM_INSERTCOLUMN, (GXWPARAM)(int)(iCol), (GXLPARAM)(const LV_COLUMN *)(pcol))


#define GXLVM_DELETECOLUMN        (GXLVM_FIRST + 28)
#define gxListView_DeleteColumn(hwnd, iCol) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_DELETECOLUMN, (GXWPARAM)(int)(iCol), 0)


#define GXLVM_GETCOLUMNWIDTH      (GXLVM_FIRST + 29)
#define gxListView_GetColumnWidth(hwnd, iCol) \
  (int)GXSNDMSG((hwnd), GXLVM_GETCOLUMNWIDTH, (GXWPARAM)(int)(iCol), 0)


#define LVSCW_AUTOSIZE              -1
#define LVSCW_AUTOSIZE_USEHEADER    -2
#define GXLVM_SETCOLUMNWIDTH          (GXLVM_FIRST + 30)

#define gxListView_SetColumnWidth(hwnd, iCol, cx) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETCOLUMNWIDTH, (GXWPARAM)(int)(iCol), GXMAKELPARAM((cx), 0))


#define GXLVM_GETHEADER               (GXLVM_FIRST + 31)
#define gxListView_GetHeader(hwnd)\
  (GXHWND)GXSNDMSG((hwnd), GXLVM_GETHEADER, 0, 0L)


#define GXLVM_CREATEDRAGIMAGE     (GXLVM_FIRST + 33)
#define gxListView_CreateDragImage(hwnd, i, lpptUpLeft) \
  (GXHIMAGELIST)GXSNDMSG((hwnd), GXLVM_CREATEDRAGIMAGE, (GXWPARAM)(int)(i), (GXLPARAM)(GXLPPOINT)(lpptUpLeft))


#define GXLVM_GETVIEWRECT         (GXLVM_FIRST + 34)
#define gxListView_GetViewRect(hwnd, prc) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETVIEWRECT, 0, (GXLPARAM)(GXRECT *)(prc))


#define GXLVM_GETTEXTCOLOR        (GXLVM_FIRST + 35)
#define gxListView_GetTextColor(hwnd)  \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_GETTEXTCOLOR, 0, 0L)


#define GXLVM_SETTEXTCOLOR        (GXLVM_FIRST + 36)
#define gxListView_SetTextColor(hwnd, clrText) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETTEXTCOLOR, 0, (GXLPARAM)(GXCOLORREF)(clrText))


#define GXLVM_GETTEXTBKCOLOR      (GXLVM_FIRST + 37)
#define gxListView_GetTextBkColor(hwnd)  \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_GETTEXTBKCOLOR, 0, 0L)


#define GXLVM_SETTEXTBKCOLOR      (GXLVM_FIRST + 38)
#define gxListView_SetTextBkColor(hwnd, clrTextBk) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETTEXTBKCOLOR, 0, (GXLPARAM)(GXCOLORREF)(clrTextBk))


#define GXLVM_GETTOPINDEX         (GXLVM_FIRST + 39)
#define gxListView_GetTopIndex(hwndLV) \
  (int)GXSNDMSG((hwndLV), GXLVM_GETTOPINDEX, 0, 0)


#define GXLVM_GETCOUNTPERPAGE     (GXLVM_FIRST + 40)
#define gxListView_GetCountPerPage(hwndLV) \
  (int)GXSNDMSG((hwndLV), GXLVM_GETCOUNTPERPAGE, 0, 0)


#define GXLVM_GETORIGIN           (GXLVM_FIRST + 41)
#define gxListView_GetOrigin(hwndLV, ppt) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_GETORIGIN, (GXWPARAM)0, (GXLPARAM)(GXPOINT *)(ppt))


#define GXLVM_UPDATE              (GXLVM_FIRST + 42)
#define gxListView_Update(hwndLV, i) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_UPDATE, (GXWPARAM)(i), 0L)


#define GXLVM_SETITEMSTATE        (GXLVM_FIRST + 43)
#define gxListView_SetItemState(hwndLV, i, data, mask) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.stateMask = mask;\
  _ms_lvi.state = data;\
  GXSNDMSG((hwndLV), GXLVM_SETITEMSTATE, (GXWPARAM)(i), (GXLPARAM)(LV_ITEM *)&_ms_lvi);\
}


#define gxListView_SetCheckState(hwndLV, i, fCheck) \
  gxListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)


#define GXLVM_GETITEMSTATE        (GXLVM_FIRST + 44)
#define gxListView_GetItemState(hwndLV, i, mask) \
  (GXUINT)GXSNDMSG((hwndLV), GXLVM_GETITEMSTATE, (GXWPARAM)(i), (GXLPARAM)(mask))


#define gxListView_GetCheckState(hwndLV, i) \
  ((((GXUINT)(GXSNDMSG((hwndLV), GXLVM_GETITEMSTATE, (GXWPARAM)(i), LVIS_STATEIMAGEMASK))) >> 12) -1)


#define GXLVM_GETITEMTEXTA        (GXLVM_FIRST + 45)
#define GXLVM_GETITEMTEXTW        (GXLVM_FIRST + 115)


#define  GXLVM_GETITEMTEXT        GXLVM_GETITEMTEXTW

//#define  GXLVM_GETITEMTEXT        GXLVM_GETITEMTEXTA


#define gxListView_GetItemText(hwndLV, i, iSubItem_, pszText_, cchTextMax_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.cchTextMax = cchTextMax_;\
  _ms_lvi.pszText = pszText_;\
  GXSNDMSG((hwndLV), GXLVM_GETITEMTEXT, (GXWPARAM)(i), (GXLPARAM)(LV_ITEM *)&_ms_lvi);\
}


#define GXLVM_SETITEMTEXTA        (GXLVM_FIRST + 46)
#define GXLVM_SETITEMTEXTW        (GXLVM_FIRST + 116)


#define  GXLVM_SETITEMTEXT        GXLVM_SETITEMTEXTW

//#define  GXLVM_SETITEMTEXT        GXLVM_SETITEMTEXTA


#define gxListView_SetItemText(hwndLV, i, iSubItem_, pszText_) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.iSubItem = iSubItem_;\
  _ms_lvi.pszText = pszText_;\
  GXSNDMSG((hwndLV), GXLVM_SETITEMTEXT, (GXWPARAM)(i), (GXLPARAM)(LV_ITEM *)&_ms_lvi);\
}


// these flags only apply to LVS_OWNERDATA listviews in report or list mode
#define LVSICF_NOINVALIDATEALL  0x00000001
#define LVSICF_NOSCROLL         0x00000002


#define GXLVM_SETITEMCOUNT        (GXLVM_FIRST + 47)
#define gxListView_SetItemCount(hwndLV, cItems) \
  GXSNDMSG((hwndLV), GXLVM_SETITEMCOUNT, (GXWPARAM)(cItems), 0)


#define gxListView_SetItemCountEx(hwndLV, cItems, dwFlags) \
  GXSNDMSG((hwndLV), GXLVM_SETITEMCOUNT, (GXWPARAM)(cItems), (GXLPARAM)(dwFlags))


typedef int (GXCALLBACK *GXPFNLVCOMPARE)(GXLPARAM, GXLPARAM, GXLPARAM);


#define GXLVM_SORTITEMS           (GXLVM_FIRST + 48)
#define gxListView_SortItems(hwndLV, _pfnCompare, _lPrm) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_SORTITEMS, (GXWPARAM)(GXLPARAM)(_lPrm), \
  (GXLPARAM)(GXPFNLVCOMPARE)(_pfnCompare))


#define GXLVM_SETITEMPOSITION32   (GXLVM_FIRST + 49)
#define gxListView_SetItemPosition32(hwndLV, i, x0, y0) \
{   GXPOINT ptNewPos; \
  ptNewPos.x = x0; ptNewPos.y = y0; \
  GXSNDMSG((hwndLV), GXLVM_SETITEMPOSITION32, (GXWPARAM)(int)(i), (GXLPARAM)&ptNewPos); \
}


#define GXLVM_GETSELECTEDCOUNT    (GXLVM_FIRST + 50)
#define gxListView_GetSelectedCount(hwndLV) \
  (GXUINT)GXSNDMSG((hwndLV), GXLVM_GETSELECTEDCOUNT, 0, 0L)

#define GXLVM_GETITEMSPACING      (GXLVM_FIRST + 51)
#define gxListView_GetItemSpacing(hwndLV, fSmall) \
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_GETITEMSPACING, fSmall, 0L)


#define GXLVM_GETISEARCHSTRINGA   (GXLVM_FIRST + 52)
#define GXLVM_GETISEARCHSTRINGW   (GXLVM_FIRST + 117)


#define GXLVM_GETISEARCHSTRING    GXLVM_GETISEARCHSTRINGW

//#define GXLVM_GETISEARCHSTRING    GXLVM_GETISEARCHSTRINGA


#define gxListView_GetISearchString(hwndLV, lpsz) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_GETISEARCHSTRING, 0, (GXLPARAM)(LPTSTR)(lpsz))


#define GXLVM_SETICONSPACING      (GXLVM_FIRST + 53)
// -1 for cx and cy means we'll use the default (system settings)
// 0 for cx or cy means use the current setting (allows you to change just one param)
#define gxListView_SetIconSpacing(hwndLV, cx, cy) \
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_SETICONSPACING, 0, GXMAKELONG(cx,cy))


#define GXLVM_SETEXTENDEDLISTVIEWSTYLE (GXLVM_FIRST + 54)   // optional wParam == mask
#define gxListView_SetExtendedListViewStyle(hwndLV, dw)\
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)

#define gxListView_SetExtendedListViewStyleEx(hwndLV, dwMask, dw)\
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_SETEXTENDEDLISTVIEWSTYLE, dwMask, dw)


#define GXLVM_GETEXTENDEDLISTVIEWSTYLE (GXLVM_FIRST + 55)
#define gxListView_GetExtendedListViewStyle(hwndLV)\
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0)

#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_SUBITEMIMAGES    0x00000002
#define LVS_EX_CHECKBOXES       0x00000004
#define LVS_EX_TRACKSELECT      0x00000008
#define LVS_EX_HEADERDRAGDROP   0x00000010
#define LVS_EX_FULLROWSELECT    0x00000020 // applies to report mode only
#define LVS_EX_ONECLICKACTIVATE 0x00000040
#define LVS_EX_TWOCLICKACTIVATE 0x00000080

#define LVS_EX_FLATSB           0x00000100
#define LVS_EX_REGIONAL         0x00000200
#define LVS_EX_INFOTIP          0x00000400 // listview does InfoTips for you
#define LVS_EX_UNDERLINEHOT     0x00000800
#define LVS_EX_UNDERLINECOLD    0x00001000
#define LVS_EX_MULTIWORKAREAS   0x00002000


#define LVS_EX_LABELTIP         0x00004000 // listview unfolds partly hidden labels if it does not have infotip text
#define LVS_EX_BORDERSELECT     0x00008000 // border selection style instead of highlight
// End (_WIN32_IE >= 0x0500)

#define LVS_EX_DOUBLEBUFFER     0x00010000
#define LVS_EX_HIDELABELS       0x00020000
#define LVS_EX_SINGLEROW        0x00040000
#define LVS_EX_SNAPTOGRID       0x00080000  // Icons automatically snap to grid.
#define LVS_EX_SIMPLESELECT     0x00100000  // Also changes overlay rendering to top right for icon mode.


#define GXLVM_GETSUBITEMRECT      (GXLVM_FIRST + 56)
#define gxListView_GetSubItemRect(hwnd, iItem, iSubItem, code, prc) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETSUBITEMRECT, (GXWPARAM)(int)(iItem), \
  ((prc) ? ((((LPRECT)(prc))->top = iSubItem), (((LPRECT)(prc))->left = code), (GXLPARAM)(prc)) : (GXLPARAM)(LPRECT)NULL))

#define GXLVM_SUBITEMHITTEST      (GXLVM_FIRST + 57)
#define gxListView_SubItemHitTest(hwnd, plvhti) \
  (int)GXSNDMSG((hwnd), GXLVM_SUBITEMHITTEST, 0, (GXLPARAM)(LPLVHITTESTINFO)(plvhti))

#define GXLVM_SETCOLUMNORDERARRAY (GXLVM_FIRST + 58)
#define gxListView_SetColumnOrderArray(hwnd, iCount, pi) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETCOLUMNORDERARRAY, (GXWPARAM)(iCount), (GXLPARAM)(GXLPINT)(pi))

#define GXLVM_GETCOLUMNORDERARRAY (GXLVM_FIRST + 59)
#define gxListView_GetColumnOrderArray(hwnd, iCount, pi) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETCOLUMNORDERARRAY, (GXWPARAM)(iCount), (GXLPARAM)(GXLPINT)(pi))

#define GXLVM_SETHOTITEM  (GXLVM_FIRST + 60)
#define gxListView_SetHotItem(hwnd, i) \
  (int)GXSNDMSG((hwnd), GXLVM_SETHOTITEM, (GXWPARAM)(i), 0)

#define GXLVM_GETHOTITEM  (GXLVM_FIRST + 61)
#define gxListView_GetHotItem(hwnd) \
  (int)GXSNDMSG((hwnd), GXLVM_GETHOTITEM, 0, 0)

#define GXLVM_SETHOTCURSOR  (GXLVM_FIRST + 62)
#define gxListView_SetHotCursor(hwnd, hcur) \
  (HCURSOR)GXSNDMSG((hwnd), GXLVM_SETHOTCURSOR, 0, (GXLPARAM)(hcur))

#define GXLVM_GETHOTCURSOR  (GXLVM_FIRST + 63)
#define gxListView_GetHotCursor(hwnd) \
  (HCURSOR)GXSNDMSG((hwnd), GXLVM_GETHOTCURSOR, 0, 0)

#define GXLVM_APPROXIMATEVIEWRECT (GXLVM_FIRST + 64)
#define gxListView_ApproximateViewRect(hwnd, iWidth, iHeight, iCount) \
  (GXDWORD)GXSNDMSG((hwnd), GXLVM_APPROXIMATEVIEWRECT, iCount, GXMAKELPARAM(iWidth, iHeight))
// _WIN32_IE >= 0x0300



#define LV_MAX_WORKAREAS         16
#define GXLVM_SETWORKAREAS         (GXLVM_FIRST + 65)
#define gxListView_SetWorkAreas(hwnd, nWorkAreas, prc) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETWORKAREAS, (GXWPARAM)(int)(nWorkAreas), (GXLPARAM)(GXRECT *)(prc))

#define GXLVM_GETWORKAREAS        (GXLVM_FIRST + 70)
#define gxListView_GetWorkAreas(hwnd, nWorkAreas, prc) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETWORKAREAS, (GXWPARAM)(int)(nWorkAreas), (GXLPARAM)(GXRECT *)(prc))


#define GXLVM_GETNUMBEROFWORKAREAS  (GXLVM_FIRST + 73)
#define gxListView_GetNumberOfWorkAreas(hwnd, pnWorkAreas) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETNUMBEROFWORKAREAS, 0, (GXLPARAM)(GXUINT *)(pnWorkAreas))


#define GXLVM_GETSELECTIONMARK    (GXLVM_FIRST + 66)
#define gxListView_GetSelectionMark(hwnd) \
  (int)GXSNDMSG((hwnd), GXLVM_GETSELECTIONMARK, 0, 0)

#define GXLVM_SETSELECTIONMARK    (GXLVM_FIRST + 67)
#define gxListView_SetSelectionMark(hwnd, i) \
  (int)GXSNDMSG((hwnd), GXLVM_SETSELECTIONMARK, 0, (GXLPARAM)(i))

#define GXLVM_SETHOVERTIME        (GXLVM_FIRST + 71)
#define gxListView_SetHoverTime(hwndLV, dwHoverTimeMs)\
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_SETHOVERTIME, 0, (GXLPARAM)(dwHoverTimeMs))

#define GXLVM_GETHOVERTIME        (GXLVM_FIRST + 72)
#define gxListView_GetHoverTime(hwndLV)\
  (GXDWORD)GXSNDMSG((hwndLV), GXLVM_GETHOVERTIME, 0, 0)

#define GXLVM_SETTOOLTIPS       (GXLVM_FIRST + 74)
#define gxListView_SetToolTips(hwndLV, hwndNewHwnd)\
  (GXHWND)GXSNDMSG((hwndLV), GXLVM_SETTOOLTIPS, (GXWPARAM)(hwndNewHwnd), 0)

#define GXLVM_GETTOOLTIPS       (GXLVM_FIRST + 78)
#define gxListView_GetToolTips(hwndLV)\
  (GXHWND)GXSNDMSG((hwndLV), GXLVM_GETTOOLTIPS, 0, 0)


#define GXLVM_SORTITEMSEX          (GXLVM_FIRST + 81)
#define gxListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_SORTITEMSEX, (GXWPARAM)(GXLPARAM)(_lPrm), (GXLPARAM)(GXPFNLVCOMPARE)(_pfnCompare))

//typedef struct tagLVBKIMAGEA
//{
//  GXULONG ulFlags;              // LVBKIF_*
//  GXHBITMAP hbm;
//  GXLPSTR pszImage;
//  GXUINT cchImageMax;
//  int xOffsetPercent;
//  int yOffsetPercent;
//} LVBKIMAGEA, *LPLVBKIMAGEA;
struct GXLVBKIMAGEW
{
  GXULONG ulFlags;              // LVBKIF_*
  GXHBITMAP hbm;
  GXLPWSTR pszImage;
  GXUINT cchImageMax;
  int xOffsetPercent;
  int yOffsetPercent;
};
typedef GXLVBKIMAGEW *GXLPLVBKIMAGEW;

#define LVBKIF_SOURCE_NONE      0x00000000
#define LVBKIF_SOURCE_HBITMAP   0x00000001
#define LVBKIF_SOURCE_URL       0x00000002
#define LVBKIF_SOURCE_MASK      0x00000003
#define LVBKIF_STYLE_NORMAL     0x00000000
#define LVBKIF_STYLE_TILE       0x00000010
#define LVBKIF_STYLE_MASK       0x00000010

#define LVBKIF_FLAG_TILEOFFSET  0x00000100
#define LVBKIF_TYPE_WATERMARK   0x10000000


#define GXLVM_SETBKIMAGEA         (GXLVM_FIRST + 68)
#define GXLVM_SETBKIMAGEW         (GXLVM_FIRST + 138)
#define GXLVM_GETBKIMAGEA         (GXLVM_FIRST + 69)
#define GXLVM_GETBKIMAGEW         (GXLVM_FIRST + 139)


#define GXLVM_SETSELECTEDCOLUMN         (GXLVM_FIRST + 140)
#define gxListView_SetSelectedColumn(hwnd, iCol) \
  GXSNDMSG((hwnd), GXLVM_SETSELECTEDCOLUMN, (GXWPARAM)iCol, 0)

#define GXLVM_SETTILEWIDTH         (GXLVM_FIRST + 141)
#define gxListView_SetTileWidth(hwnd, cpWidth) \
  GXSNDMSG((hwnd), GXLVM_SETTILEWIDTH, (GXWPARAM)cpWidth, 0)

#define LV_VIEW_ICON        0x0000
#define LV_VIEW_DETAILS     0x0001
#define LV_VIEW_SMALLICON   0x0002
#define LV_VIEW_LIST        0x0003
#define LV_VIEW_TILE        0x0004
#define LV_VIEW_MAX         0x0004

#define GXLVM_SETVIEW         (GXLVM_FIRST + 142)
#define gxListView_SetView(hwnd, iView) \
  (GXDWORD)GXSNDMSG((hwnd), GXLVM_SETVIEW, (GXWPARAM)(GXDWORD)iView, 0)

#define GXLVM_GETVIEW         (GXLVM_FIRST + 143)
#define gxListView_GetView(hwnd) \
  (GXDWORD)GXSNDMSG((hwnd), GXLVM_GETVIEW, 0, 0)


#define LVGF_NONE           0x00000000
#define LVGF_HEADER         0x00000001
#define LVGF_FOOTER         0x00000002
#define LVGF_STATE          0x00000004
#define LVGF_ALIGN          0x00000008
#define LVGF_GROUPID        0x00000010

#define LVGS_NORMAL         0x00000000
#define LVGS_COLLAPSED      0x00000001
#define LVGS_HIDDEN         0x00000002

#define LVGA_HEADER_LEFT    0x00000001
#define LVGA_HEADER_CENTER  0x00000002
#define LVGA_HEADER_RIGHT   0x00000004  // Don't forget to validate exclusivity
#define LVGA_FOOTER_LEFT    0x00000008
#define LVGA_FOOTER_CENTER  0x00000010
#define LVGA_FOOTER_RIGHT   0x00000020  // Don't forget to validate exclusivity

struct GXLVGROUP
{
  GXUINT    cbSize;
  GXUINT    mask;
  GXLPWSTR  pszHeader;
  int       cchHeader;

  GXLPWSTR  pszFooter;
  int       cchFooter;

  int       iGroupId;

  GXUINT    stateMask;
  GXUINT    state;
  GXUINT    uAlign;
};
typedef GXLVGROUP *GXLPLVGROUP;


#define GXLVM_INSERTGROUP         (GXLVM_FIRST + 145)
#define gxListView_InsertGroup(hwnd, index, pgrp) \
  GXSNDMSG((hwnd), GXLVM_INSERTGROUP, (GXWPARAM)index, (GXLPARAM)pgrp)


#define GXLVM_SETGROUPINFO         (GXLVM_FIRST + 147)
#define gxListView_SetGroupInfo(hwnd, iGroupId, pgrp) \
  GXSNDMSG((hwnd), GXLVM_SETGROUPINFO, (GXWPARAM)iGroupId, (GXLPARAM)pgrp)


#define GXLVM_GETGROUPINFO         (GXLVM_FIRST + 149)
#define gxListView_GetGroupInfo(hwnd, iGroupId, pgrp) \
  GXSNDMSG((hwnd), GXLVM_GETGROUPINFO, (GXWPARAM)iGroupId, (GXLPARAM)pgrp)


#define GXLVM_REMOVEGROUP         (GXLVM_FIRST + 150)
#define gxListView_RemoveGroup(hwnd, iGroupId) \
  GXSNDMSG((hwnd), GXLVM_REMOVEGROUP, (GXWPARAM)iGroupId, 0)

#define GXLVM_MOVEGROUP         (GXLVM_FIRST + 151)
#define gxListView_MoveGroup(hwnd, iGroupId, toIndex) \
  GXSNDMSG((hwnd), GXLVM_MOVEGROUP, (GXWPARAM)iGroupId, (GXLPARAM)toIndex)

#define GXLVM_MOVEITEMTOGROUP            (GXLVM_FIRST + 154)
#define gxListView_MoveItemToGroup(hwnd, idItemFrom, idGroupTo) \
  GXSNDMSG((hwnd), GXLVM_MOVEITEMTOGROUP, (GXWPARAM)idItemFrom, (GXLPARAM)idGroupTo)


#define LVGMF_NONE          0x00000000
#define LVGMF_BORDERSIZE    0x00000001
#define LVGMF_BORDERCOLOR   0x00000002
#define LVGMF_TEXTCOLOR     0x00000004

struct GXLVGROUPMETRICS
{
  GXUINT cbSize;
  GXUINT mask;
  GXUINT Left;
  GXUINT Top;
  GXUINT Right;
  GXUINT Bottom;
  GXCOLORREF crLeft;
  GXCOLORREF crTop;
  GXCOLORREF crRight;
  GXCOLORREF crBottom;
  GXCOLORREF crHeader;
  GXCOLORREF crFooter;
};
typedef GXLVGROUPMETRICS *GXLPLVGROUPMETRICS;

#define GXLVM_SETGROUPMETRICS         (GXLVM_FIRST + 155)
#define gxListView_SetGroupMetrics(hwnd, pGroupMetrics) \
  GXSNDMSG((hwnd), GXLVM_SETGROUPMETRICS, 0, (GXLPARAM)pGroupMetrics)

#define GXLVM_GETGROUPMETRICS         (GXLVM_FIRST + 156)
#define gxListView_GetGroupMetrics(hwnd, pGroupMetrics) \
  GXSNDMSG((hwnd), GXLVM_GETGROUPMETRICS, 0, (GXLPARAM)pGroupMetrics)

#define GXLVM_ENABLEGROUPVIEW         (GXLVM_FIRST + 157)
#define gxListView_EnableGroupView(hwnd, fEnable) \
  GXSNDMSG((hwnd), GXLVM_ENABLEGROUPVIEW, (GXWPARAM)fEnable, 0)

typedef int (GXCALLBACK *PFNLVGROUPCOMPARE)(int, int, void *);

#define GXLVM_SORTGROUPS         (GXLVM_FIRST + 158)
#define gxListView_SortGroups(hwnd, _pfnGroupCompate, _plv) \
  GXSNDMSG((hwnd), GXLVM_SORTGROUPS, (GXWPARAM)_pfnGroupCompate, (GXLPARAM)_plv)

struct GXLVINSERTGROUPSORTED
{
  PFNLVGROUPCOMPARE pfnGroupCompare;
  void *pvData;
  GXLVGROUP lvGroup;
};
typedef GXLVINSERTGROUPSORTED *GXLPLVINSERTGROUPSORTED;

#define GXLVM_INSERTGROUPSORTED           (GXLVM_FIRST + 159)
#define gxListView_InsertGroupSorted(hwnd, structInsert) \
  GXSNDMSG((hwnd), GXLVM_INSERTGROUPSORTED, (GXWPARAM)structInsert, 0)

#define GXLVM_REMOVEALLGROUPS             (GXLVM_FIRST + 160)
#define gxListView_RemoveAllGroups(hwnd) \
  GXSNDMSG((hwnd), GXLVM_REMOVEALLGROUPS, 0, 0)

#define GXLVM_HASGROUP                    (GXLVM_FIRST + 161)
#define gxListView_HasGroup(hwnd, dwGroupId) \
  GXSNDMSG((hwnd), GXLVM_HASGROUP, dwGroupId, 0)

#define LVTVIF_AUTOSIZE       0x00000000
#define LVTVIF_FIXEDWIDTH     0x00000001
#define LVTVIF_FIXEDHEIGHT    0x00000002
#define LVTVIF_FIXEDSIZE      0x00000003

#define LVTVIM_TILESIZE       0x00000001
#define LVTVIM_COLUMNS        0x00000002
#define LVTVIM_LABELMARGIN    0x00000004

typedef struct tagGXLVTILEVIEWINFO
{
  GXUINT cbSize;
  GXDWORD dwMask;     //LVTVIM_*
  GXDWORD dwFlags;    //LVTVIF_*
  GXSIZE sizeTile;
  int cLines;
  GXRECT rcLabelMargin;
} GXLVTILEVIEWINFO, *GXLPLVTILEVIEWINFO;

typedef struct tagGXLVTILEINFO
{
  GXUINT cbSize;
  int iItem;
  GXUINT cColumns;
  GXUINT* puColumns;
} GXLVTILEINFO, *GXLPLVTILEINFO;

#define GXLVM_SETTILEVIEWINFO                 (GXLVM_FIRST + 162)
#define gxListView_SetTileViewInfo(hwnd, ptvi) \
  GXSNDMSG((hwnd), GXLVM_SETTILEVIEWINFO, 0, (GXLPARAM)ptvi)

#define GXLVM_GETTILEVIEWINFO                 (GXLVM_FIRST + 163)
#define gxListView_GetTileViewInfo(hwnd, ptvi) \
  GXSNDMSG((hwnd), GXLVM_GETTILEVIEWINFO, 0, (GXLPARAM)ptvi)

#define GXLVM_SETTILEINFO                     (GXLVM_FIRST + 164)
#define gxListView_SetTileInfo(hwnd, pti) \
  GXSNDMSG((hwnd), GXLVM_SETTILEINFO, 0, (GXLPARAM)pti)

#define GXLVM_GETTILEINFO                     (GXLVM_FIRST + 165)
#define gxListView_GetTileInfo(hwnd, pti) \
  GXSNDMSG((hwnd), GXLVM_GETTILEINFO, 0, (GXLPARAM)pti)

typedef struct 
{
  GXUINT  cbSize;
  GXDWORD dwFlags;
  int     iItem;
  GXDWORD dwReserved;
} GXLVINSERTMARK, *GXLPLVINSERTMARK;

#define LVIM_AFTER      0x00000001 // TRUE = insert After iItem, otherwise before

#define GXLVM_SETINSERTMARK                   (GXLVM_FIRST + 166)
#define gxListView_SetInsertMark(hwnd, lvim) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETINSERTMARK, (GXWPARAM) 0, (GXLPARAM) (lvim))

#define GXLVM_GETINSERTMARK                   (GXLVM_FIRST + 167)
#define gxListView_GetInsertMark(hwnd, lvim) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETINSERTMARK, (GXWPARAM) 0, (GXLPARAM) (lvim))

#define GXLVM_INSERTMARKHITTEST               (GXLVM_FIRST + 168)
#define gxListView_InsertMarkHitTest(hwnd, point, lvim) \
  (int)GXSNDMSG((hwnd), GXLVM_INSERTMARKHITTEST, (GXWPARAM)(GXLPPOINT)(point), (GXLPARAM)(LPLVINSERTMARK)(lvim))

#define GXLVM_GETINSERTMARKRECT               (GXLVM_FIRST + 169)
#define gxListView_GetInsertMarkRect(hwnd, rc) \
  (int)GXSNDMSG((hwnd), GXLVM_GETINSERTMARKRECT, (GXWPARAM)0, (GXLPARAM)(LPRECT)(rc))

#define GXLVM_SETINSERTMARKCOLOR                 (GXLVM_FIRST + 170)
#define gxListView_SetInsertMarkColor(hwnd, color) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_SETINSERTMARKCOLOR, (GXWPARAM)0, (GXLPARAM)(GXCOLORREF)(color))

#define GXLVM_GETINSERTMARKCOLOR                 (GXLVM_FIRST + 171)
#define gxListView_GetInsertMarkColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_GETINSERTMARKCOLOR, (GXWPARAM)0, (GXLPARAM)0)

typedef struct tagGXLVSETINFOTIP
{
  GXUINT cbSize;
  GXDWORD dwFlags;
  GXLPWSTR pszText;
  int iItem;
  int iSubItem;
} GXLVSETINFOTIP, *GXLPLVSETINFOTIP;

#define  GXLVM_SETINFOTIP         (GXLVM_FIRST + 173)

#define gxListView_SetInfoTip(hwndLV, plvInfoTip)\
  (GXBOOL)GXSNDMSG((hwndLV), GXLVM_SETINFOTIP, (GXWPARAM)0, (GXLPARAM)plvInfoTip)

#define GXLVM_GETSELECTEDCOLUMN   (GXLVM_FIRST + 174)
#define gxListView_GetSelectedColumn(hwnd) \
  (GXUINT)GXSNDMSG((hwnd), GXLVM_GETSELECTEDCOLUMN, 0, 0)


#define GXLVM_ISGROUPVIEWENABLED  (GXLVM_FIRST + 175)
#define gxListView_IsGroupViewEnabled(hwnd) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_ISGROUPVIEWENABLED, 0, 0)

#define GXLVM_GETOUTLINECOLOR     (GXLVM_FIRST + 176)
#define gxListView_GetOutlineColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_GETOUTLINECOLOR, 0, 0)

#define GXLVM_SETOUTLINECOLOR     (GXLVM_FIRST + 177)
#define gxListView_SetOutlineColor(hwnd, color) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXLVM_SETOUTLINECOLOR, (GXWPARAM)0, (GXLPARAM)(GXCOLORREF)(color))


#define GXLVM_CANCELEDITLABEL     (GXLVM_FIRST + 179)
#define gxListView_CancelEditLabel(hwnd) \
  ()GXSNDMSG((hwnd), GXLVM_CANCELEDITLABEL, (GXWPARAM)0, (GXLPARAM)0)


// These next to methods make it easy to identify an item that can be repositioned
// within listview. For example: Many developers use the lParam to store an identifier that is
// unique. Unfortunatly, in order to find this item, they have to iterate through all of the items
// in the listview. Listview will maintain a unique identifier.  The upper bound is the size of a GXDWORD.
#define GXLVM_MAPINDEXTOID     (GXLVM_FIRST + 180)
#define gxListView_MapIndexToID(hwnd, index) \
  (GXUINT)GXSNDMSG((hwnd), GXLVM_MAPINDEXTOID, (GXWPARAM)index, (GXLPARAM)0)

#define GXLVM_MAPIDTOINDEX     (GXLVM_FIRST + 181)
#define gxListView_MapIDToIndex(hwnd, id) \
  (GXUINT)GXSNDMSG((hwnd), GXLVM_MAPIDTOINDEX, (GXWPARAM)id, (GXLPARAM)0)






#define LVBKIMAGE               LVBKIMAGEW
#define LPLVBKIMAGE             LPLVBKIMAGEW
#define GXLVM_SETBKIMAGE          GXLVM_SETBKIMAGEW
#define GXLVM_GETBKIMAGE          GXLVM_GETBKIMAGEW

//#define LVBKIMAGE               LVBKIMAGEA
//#define LPLVBKIMAGE             LPLVBKIMAGEA
//#define GXLVM_SETBKIMAGE          GXLVM_SETBKIMAGEA
//#define GXLVM_GETBKIMAGE          GXLVM_GETBKIMAGEA



#define gxListView_SetBkImage(hwnd, plvbki) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_SETBKIMAGE, 0, (GXLPARAM)(plvbki))

#define gxListView_GetBkImage(hwnd, plvbki) \
  (GXBOOL)GXSNDMSG((hwnd), GXLVM_GETBKIMAGE, 0, (GXLPARAM)(plvbki))

//-----------------------------------------------------------
#define LVA_DEFAULT             0x0000
#define LVA_ALIGNLEFT           0x0001
#define LVA_ALIGNTOP            0x0002
#define LVA_SNAPTOGRID          0x0005

//-----------------------------------------------------------
#define LVS_ICON                0x0000
#define LVS_REPORT              0x0001
#define LVS_SMALLICON           0x0002
#define LVS_LIST                0x0003
#define LVS_TYPEMASK            0x0003
#define LVS_SINGLESEL           0x0004
#define LVS_SHOWSELALWAYS       0x0008
#define LVS_SORTASCENDING       0x0010
#define LVS_SORTDESCENDING      0x0020
#define LVS_SHAREIMAGELISTS     0x0040
#define LVS_NOLABELWRAP         0x0080
#define LVS_AUTOARRANGE         0x0100
#define LVS_EDITLABELS          0x0200

#define LVS_OWNERDATA           0x1000

#define LVS_NOSCROLL            0x2000

#define LVS_TYPESTYLEMASK       0xfc00

#define LVS_ALIGNTOP            0x0000
#define LVS_ALIGNLEFT           0x0800
#define LVS_ALIGNMASK           0x0c00

#define LVS_OWNERDRAWFIXED      0x0400
#define LVS_NOCOLUMNHEADER      0x4000
#define LVS_NOSORTHEADER        0x8000

//-----------------------------------------------------------
#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_SUBITEMIMAGES    0x00000002
#define LVS_EX_CHECKBOXES       0x00000004
#define LVS_EX_TRACKSELECT      0x00000008
#define LVS_EX_HEADERDRAGDROP   0x00000010
#define LVS_EX_FULLROWSELECT    0x00000020 // applies to report mode only
#define LVS_EX_ONECLICKACTIVATE 0x00000040
#define LVS_EX_TWOCLICKACTIVATE 0x00000080

#define LVS_EX_FLATSB           0x00000100
#define LVS_EX_REGIONAL         0x00000200
#define LVS_EX_INFOTIP          0x00000400 // listview does InfoTips for you
#define LVS_EX_UNDERLINEHOT     0x00000800
#define LVS_EX_UNDERLINECOLD    0x00001000
#define LVS_EX_MULTIWORKAREAS   0x00002000

#define LVS_EX_LABELTIP         0x00004000 // listview unfolds partly hidden labels if it does not have infotip text
#define LVS_EX_BORDERSELECT     0x00008000 // border selection style instead of highlight

#define LVS_EX_DOUBLEBUFFER     0x00010000
#define LVS_EX_HIDELABELS       0x00020000
#define LVS_EX_SINGLEROW        0x00040000
#define LVS_EX_SNAPTOGRID       0x00080000  // Icons automatically snap to grid.
#define LVS_EX_SIMPLESELECT     0x00100000  // Also changes overlay rendering to top right for icon mode.

//-----------------------------------------------------------
// custom draw return flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
#define CDRF_DODEFAULT          0x00000000
#define CDRF_NEWFONT            0x00000002
#define CDRF_SKIPDEFAULT        0x00000004


#define CDRF_NOTIFYPOSTPAINT    0x00000010
#define CDRF_NOTIFYITEMDRAW     0x00000020

#define CDRF_NOTIFYSUBITEMDRAW  0x00000020  // flags are the same, we can distinguish by context
#define CDRF_NOTIFYPOSTERASE    0x00000040

// drawstage flags
// values under 0x00010000 are reserved for global custom draw values.
// above that are for specific controls
#define CDDS_PREPAINT           0x00000001
#define CDDS_POSTPAINT          0x00000002
#define CDDS_PREERASE           0x00000003
#define CDDS_POSTERASE          0x00000004
// the 0x000010000 bit means it's individual item specific
#define CDDS_ITEM               0x00010000
#define CDDS_ITEMPREPAINT       (CDDS_ITEM | CDDS_PREPAINT)
#define CDDS_ITEMPOSTPAINT      (CDDS_ITEM | CDDS_POSTPAINT)
#define CDDS_ITEMPREERASE       (CDDS_ITEM | CDDS_PREERASE)
#define CDDS_ITEMPOSTERASE      (CDDS_ITEM | CDDS_POSTERASE)

#define CDDS_SUBITEM            0x00020000

// itemState flags
#define CDIS_SELECTED       0x0001
#define CDIS_GRAYED         0x0002
#define CDIS_DISABLED       0x0004
#define CDIS_CHECKED        0x0008
#define CDIS_FOCUS          0x0010
#define CDIS_DEFAULT        0x0020
#define CDIS_HOT            0x0040
#define CDIS_MARKED         0x0080
#define CDIS_INDETERMINATE  0x0100

#define CDIS_SHOWKEYBOARDCUES   0x0200
//-----------------------------------------------------------

#define ILD_NORMAL              0x00000000
#define ILD_TRANSPARENT         0x00000001
#define ILD_MASK                0x00000010
#define ILD_IMAGE               0x00000020
#if (_WIN32_IE >= 0x0300)
#define ILD_ROP                 0x00000040
#endif
#define ILD_BLEND25             0x00000002
#define ILD_BLEND50             0x00000004
#define ILD_OVERLAYMASK         0x00000F00
#define INDEXTOOVERLAYMASK(i)   ((i) << 8)
#define ILD_PRESERVEALPHA       0x00001000  // This preserves the alpha channel in dest
#define ILD_SCALE               0x00002000  // Causes the image to be scaled to cx, cy instead of clipped
#define ILD_DPISCALE            0x00004000

#define ILD_SELECTED            ILD_BLEND50
#define ILD_FOCUS               ILD_BLEND25
#define ILD_BLEND               ILD_BLEND50

//-----------------------------------------------------------
//#define LPSTR_TEXTCALLBACKW     ((GXLPWSTR)-1L)



//////////////////////////////////////////////////////////////////////////
//
// 引用的标准函数
//
#ifdef _WIN32
//#define GXSTRLEN           lstrlenW
//#define GXSTRCPY           clstd::strcpyT
//#define GXSTRCPYN          clstd::strcpyn
//#define GXSTRCMP           lstrcmpW
//#define GXSTRCMPI          lstrcmpiW
//#define GXMEMCPY           memcpy
#define GXMEMSET           memset
#else
//GXUINT strlenW(GXLPCWSTR str);
//GXLPWSTR strcpyW(GXLPWSTR,GXLPCWSTR);
#define GXMEMCPY           memcpy
#define GXMEMSET           memset
#define ZeroMemory(d, s)   memset(d, 0, s)
#endif

#define GXSTRCMP          clstd::strcmpT
#define GXSTRNCMP         clstd::strncmpT
#define GXSTRLEN          clstd::strlenT
#define GXSTRCPY          clstd::strcpyT
#define GXSTRCPYN         clstd::strcpynT
#define GXSTRCMPI         clstd::strcmpiT
#define GXSTRNCMPI        clstd::strncmpiT
#define GXATOI(_STR)      clstd::xtoi(10, _STR, -1)


//
// 消息定义
//
enum GXWndMsg
{
  GXWM_NULL                   = 0x0000,
  GXWM_CREATE                 = 0x0001,  // WM_CREATE
  GXWM_DESTROY                = 0x0002,  // WM_DESTROY
  GXWM_SIZE                   = 0x0005,  // WM_SIZE
  GXWM_SETFOCUS               = 0x0007,  // WM_SETFOCUS
  GXWM_KILLFOCUS              = 0x0008,  // WM_KILLFOCUS
  GXWM_ENABLE                 = 0x000A,  // WM_ENABLE
  GXWM_SETREDRAW              = 0x000B,
  GXWM_SETTEXT                = 0x000C,  // WM_SETTEXT
  GXWM_GETTEXT                = 0x000D,
  GXWM_GETTEXTLENGTH          = 0x000E,
  GXWM_PAINT                  = 0x000F,

  GXWM_CLOSE                  = 0x0010,
  GXWM_QUIT                   = 0x0012,
  GXWM_ERASEBKGND             = 0x0014,
  GXWM_SYSCOLORCHANGE         = 0x0015,
  GXWM_SHOWWINDOW             = 0x0018,  // WM_SHOWWINDOW
  GXWM_GETIDNAMEW             = 0x0019,  // Identifier Name
  GXWM_WININICHANGE           = 0x001A,
  GXWM_CANCELMODE             = 0x001F,

  GXWM_SETCURSOR              = 0x0020,  // WM_SETCURSOR
  GXWM_MOUSEACTIVATE          = 0x0021,
  GXWM_GETMINMAXINFO          = 0x0024,
  GXWM_DRAWITEM               = 0x002B,
  GXWM_MEASUREITEM            = 0x002C,
  GXWM_DELETEITEM             = 0x002D,
  GXWM_VKEYTOITEM             = 0x002E,
  GXWM_CHARTOITEM             = 0x002F,

  GXWM_SETFONT                = 0x0030,
  GXWM_GETFONT                = 0x0031,
  GXWM_COMPAREITEM            = 0x0039,

  GXWM_WINDOWPOSCHANGING      = 0x0046,
  GXWM_WINDOWPOSCHANGED       = 0x0047,
  GXWM_NOTIFY                 = 0x004E,  // WM_NOTIFY
  GXWM_HELP                   = 0x0053,
  //GXWM_KNOCK                  = 0x0054,  // wParam -> 0: KnockName; 1: KnockVar; 2: KnockPtr, lParam: Name/VarPtr/Ptr
  GXWM_IMPULSE                = 0x0054,
  GXWM_NOTIFYFORMAT           = 0x0055,

  GXWM_CONTEXTMENU            = 0x007B,
  GXWM_STYLECHANGING          = 0x007C,  // WM_STYLECHANGING
  GXWM_STYLECHANGED           = 0x007D,  // WM_STYLECHANGED
  GXWM_DISPLAYCHANGE          = 0x007E,
  GXWM_NCCREATE               = 0x0081,  // WM_NCCREATE
  GXWM_NCDESTROY              = 0x0082,  // WM_NCDESTROY
  GXWM_NCCALCSIZE             = 0x0083,
  GXWM_NCHITTEST              = 0x0084,  // WM_NCHITTEST
  GXWM_NCPAINT                = 0x0085,  // WM_NCPAINT
  GXWM_NCACTIVATE             = 0x0086,
  GXWM_GETDLGCODE             = 0x0087,
  GXWM_NCMOUSEMOVE            = 0x00A0,
  
  GXWM_NCLBUTTONDOWN          = 0x00A1,
  GXWM_NCLBUTTONUP            = 0x00A2,
  GXWM_NCLBUTTONDBLCLK        = 0x00A3,

  GXWM_NCRBUTTONDOWN          = 0x00A4,
  GXWM_NCRBUTTONUP            = 0x00A5,
  GXWM_NCRBUTTONDBLCLK        = 0x00A6,

  GXWM_NCMBUTTONDOWN          = 0x00A7,
  GXWM_NCMBUTTONUP            = 0x00A8,
  GXWM_NCMBUTTONDBLCLK        = 0x00A9,

  // Edit Control Messages
  GXEM_GETSEL                 = 0x00B0,
  GXEM_SETSEL                 = 0x00B1,
  GXEM_GETRECT                = 0x00B2,
  GXEM_SETRECT                = 0x00B3,
  GXEM_SETRECTNP              = 0x00B4,
  GXEM_SCROLL                 = 0x00B5,
  GXEM_LINESCROLL             = 0x00B6,
  GXEM_SCROLLCARET            = 0x00B7,
  GXEM_GETMODIFY              = 0x00B8,
  GXEM_SETMODIFY              = 0x00B9,
  GXEM_GETLINECOUNT           = 0x00BA,
  GXEM_LINEINDEX              = 0x00BB,
  GXEM_SETHANDLE              = 0x00BC,
  GXEM_GETHANDLE              = 0x00BD,
  GXEM_GETTHUMB               = 0x00BE,
  GXEM_LINELENGTH             = 0x00C1,
  GXEM_REPLACESEL             = 0x00C2,
  GXEM_GETLINE                = 0x00C4,
  GXEM_LIMITTEXT              = 0x00C5,
  GXEM_CANUNDO                = 0x00C6,
  GXEM_UNDO                   = 0x00C7,
  GXEM_FMTLINES               = 0x00C8,
  GXEM_LINEFROMCHAR           = 0x00C9,
  GXEM_SETTABSTOPS            = 0x00CB,
  GXEM_SETPASSWORDCHAR        = 0x00CC,
  GXEM_EMPTYUNDOBUFFER        = 0x00CD,
  GXEM_GETFIRSTVISIBLELINE    = 0x00CE,
  GXEM_SETREADONLY            = 0x00CF,
  GXEM_SETWORDBREAKPROC       = 0x00D0,
  GXEM_GETWORDBREAKPROC       = 0x00D1,
  GXEM_GETPASSWORDCHAR        = 0x00D2,
  GXEM_SETMARGINS             = 0x00D3,
  GXEM_GETMARGINS             = 0x00D4,
  GXEM_SETLIMITTEXT           = GXEM_LIMITTEXT,   /* ;win40 Name change */
  GXEM_GETLIMITTEXT           = 0x00D5,
  GXEM_POSFROMCHAR            = 0x00D6,
  GXEM_CHARFROMPOS            = 0x00D7,
  GXEM_SETIMESTATUS           = 0x00D8,
  GXEM_GETIMESTATUS           = 0x00D9,

  // Scrollbar messages
  // Sliderbar messages
  GXSBM_SETPOS                = 0x00E0,
  GXSBM_GETPOS                = 0x00E1,
  GXSBM_SETRANGE              = 0x00E2,
  GXSBM_SETRANGEREDRAW        = 0x00E6,
  GXSBM_GETRANGE              = 0x00E3,
  GXSBM_ENABLE_ARROWS         = 0x00E4,
  GXSBM_SETSCROLLINFO         = 0x00E9,
  GXSBM_GETSCROLLINFO         = 0x00EA,
  GXSBM_GETSCROLLBARINFO      = 0x00EB,

  // Button Control Messages
  GXBM_GETCHECK               = 0x00F0,
  GXBM_SETCHECK               = 0x00F1,
  GXBM_GETSTATE               = 0x00F2,
  GXBM_SETSTATE               = 0x00F3,
  GXBM_SETSTYLE               = 0x00F4,
  GXBM_CLICK                  = 0x00F5,
  GXBM_GETIMAGE               = 0x00F6,
  GXBM_SETIMAGE               = 0x00F7,
  GXBM_SETDONTCLICK           = 0x00F8,


  GXWM_KEYFIRST               = 0x0100,
  GXWM_KEYDOWN                = 0x0100,  // WM_KEYDOWN
  GXWM_KEYUP                  = 0x0101,  // WM_KEYUP
  GXWM_CHAR                   = 0x0102,  // WM_CHAR
  
  GXWM_SYSKEYDOWN             = 0x0104,
  GXWM_SYSKEYUP               = 0x0105,
  GXWM_SYSCHAR                = 0x0106,

  GXWM_KEYLAST                = 0x0109,

  GXWM_IME_STARTCOMPOSITION   = 0x010D,
  GXWM_IME_ENDCOMPOSITION     = 0x010E,
  GXWM_IME_COMPOSITION        = 0x010F,
  GXWM_INITDIALOG             = 0x0110,
  GXWM_COMMAND                = 0x0111,  // WM_COMMAND
  GXWM_SYSCOMMAND             = 0x0112,
  GXWM_TIMER                  = 0x0113,
  GXWM_HSCROLL                = 0x0114,
  GXWM_VSCROLL                = 0x0115,
  GXWM_INITMENU               = 0x0116,
  GXWM_INITMENUPOPUP          = 0x0117,

  GXWM_MENUSELECT             = 0x011F,
  GXWM_MENUCHAR               = 0x0120,
  GXWM_ENTERIDLE              = 0x0121,

  GXWM_MENUCOMMAND            = 0x0126,

  GXWM_CTLCOLOREDIT           = 0x0133,
  GXWM_CTLCOLORLISTBOX        = 0x0134,
  GXWM_CTLCOLORBTN            = 0x0135,
  GXWM_CTLCOLORSTATIC         = 0x0138,

  
  // Combo Box messages
  GXCB_GETEDITSEL             = 0x0140,
  GXCB_LIMITTEXT              = 0x0141,
  GXCB_SETEDITSEL             = 0x0142,
  GXCB_ADDSTRING              = 0x0143,
  GXCB_DELETESTRING           = 0x0144,
  GXCB_DIR                    = 0x0145,
  GXCB_GETCOUNT               = 0x0146,
  GXCB_GETCURSEL              = 0x0147,
  GXCB_GETLBTEXT              = 0x0148,
  GXCB_GETLBTEXTLEN           = 0x0149,
  GXCB_INSERTSTRING           = 0x014A,
  GXCB_RESETCONTENT           = 0x014B,
  GXCB_FINDSTRING             = 0x014C,
  GXCB_SELECTSTRING           = 0x014D,
  GXCB_SETCURSEL              = 0x014E,
  GXCB_SHOWDROPDOWN           = 0x014F,
  GXCB_GETITEMDATA            = 0x0150,
  GXCB_SETITEMDATA            = 0x0151,
  GXCB_GETDROPPEDCONTROLRECT  = 0x0152,
  GXCB_SETITEMHEIGHT          = 0x0153,
  GXCB_GETITEMHEIGHT          = 0x0154,
  GXCB_SETEXTENDEDUI          = 0x0155,
  GXCB_GETEXTENDEDUI          = 0x0156,
  GXCB_GETDROPPEDSTATE        = 0x0157,
  GXCB_FINDSTRINGEXACT        = 0x0158,
  GXCB_SETLOCALE              = 0x0159,
  GXCB_GETLOCALE              = 0x015A,
  GXCB_GETTOPINDEX            = 0x015b,
  GXCB_SETTOPINDEX            = 0x015c,
  GXCB_GETHORIZONTALEXTENT    = 0x015d,
  GXCB_SETHORIZONTALEXTENT    = 0x015e,
  GXCB_GETDROPPEDWIDTH        = 0x015f,
  GXCB_SETDROPPEDWIDTH        = 0x0160,
  GXCB_INITSTORAGE            = 0x0161,
//GXCB_MULTIPLEADDSTRING      = 0x0163, // not used
  GXCB_GETCOMBOBOXINFO        = 0x0164,
  GXCB_MSGMAX                 = 0x0165,


  // Static messages
  GXSTM_SETICON               = 0x0170,
  GXSTM_GETICON               = 0x0171,
  GXSTM_SETIMAGE              = 0x0172,
  GXSTM_GETIMAGE              = 0x0173,

  
  // Listbox messages
  GXLB_ADDSTRINGW             = 0x0180,
  GXLB_INSERTSTRINGW          = 0x0181,
  GXLB_DELETESTRING           = 0x0182,
  GXLB_SELITEMRANGEEX         = 0x0183,
  GXLB_RESETCONTENT           = 0x0184,
  GXLB_SETSEL                 = 0x0185,
  GXLB_SETCURSEL              = 0x0186,
  GXLB_GETSEL                 = 0x0187,
  GXLB_GETCURSEL              = 0x0188,
  GXLB_GETTEXT                = 0x0189,
  GXLB_GETTEXTLEN             = 0x018A,
  GXLB_GETCOUNT               = 0x018B,
  GXLB_SELECTSTRING           = 0x018C,
  GXLB_DIR                    = 0x018D,
  GXLB_GETTOPINDEX            = 0x018E,
  GXLB_FINDSTRING             = 0x018F,
  GXLB_GETSELCOUNT            = 0x0190,
  GXLB_GETSELITEMS            = 0x0191,
  GXLB_SETTABSTOPS            = 0x0192,
  GXLB_GETHORIZONTALEXTENT    = 0x0193,
  GXLB_SETHORIZONTALEXTENT    = 0x0194,
  GXLB_SETCOLUMNWIDTH         = 0x0195,
  GXLB_ADDFILE                = 0x0196,
  GXLB_SETTOPINDEX            = 0x0197,
  GXLB_GETITEMRECT            = 0x0198,
  GXLB_GETITEMDATA            = 0x0199,
  GXLB_SETITEMDATA            = 0x019A,
  GXLB_SELITEMRANGE           = 0x019B,
  GXLB_SETANCHORINDEX         = 0x019C,
  GXLB_GETANCHORINDEX         = 0x019D,
  GXLB_SETCARETINDEX          = 0x019E,
  GXLB_GETCARETINDEX          = 0x019F,
  GXLB_SETITEMHEIGHT          = 0x01A0,
  GXLB_GETITEMHEIGHT          = 0x01A1,
  GXLB_FINDSTRINGEXACT        = 0x01A2,
  GXLB_CARETON                = 0x01a3,  // 这个在Windows头文件中没定义
  GXLB_CARETOFF               = 0x01a4,  // 这个在Windows头文件中没定义
  GXLB_SETLOCALE              = 0x01A5,
  GXLB_GETLOCALE              = 0x01A6,
  GXLB_SETCOUNT               = 0x01A7,
  GXLB_INITSTORAGE            = 0x01A8,
  GXLB_ITEMFROMPOINT          = 0x01A9,
  GXLB_MULTIPLEADDSTRING      = 0x01B1,
  GXLB_GETLISTBOXINFO         = 0x01B2,
  GXLB_SETITEMTEMPLATE        = 0x01B3,  // GXUI扩展，设置RichList项目模板名
  GXLB_SETCOLOR               = 0x01C0,
  GXLB_SETCOLUMNSWIDTH        = 0x01C1,
  GXLB_GETCOLUMNSWIDTH        = 0x01C2,
  GXLB_MSGMAX                 = 0x01C3,

  
  GXWM_MOUSEFIRST             = 0x0200,
  GXWM_MOUSEMOVE              = 0x0200,  // WM_MOUSEMOVE
  GXWM_LBUTTONDOWN            = 0x0201,  // WM_LBUTTONDOWN
  GXWM_LBUTTONUP              = 0x0202,  // WM_LBUTTONUP
  GXWM_LBUTTONDBLCLK          = 0x0203,
  GXWM_RBUTTONDOWN            = 0x0204,  // WM_RBUTTONDOWN
  GXWM_RBUTTONUP              = 0x0205,  // WM_RBUTTONUP
  GXWM_RBUTTONDBLCLK          = 0x0206,
  GXWM_MBUTTONDOWN            = 0x0207,
  GXWM_MBUTTONUP              = 0x0208,
  GXWM_MBUTTONDBLCLK          = 0x0209,
  GXWM_MOUSEWHEEL             = 0x020A,
  GXWM_MOUSELAST              = 0x020E,

  GXWM_PARENTNOTIFY           = 0x0210,
  GXWM_ENTERMENULOOP          = 0x0211,
  GXWM_EXITMENULOOP           = 0x0212,
  GXWM_NEXTMENU               = 0x0213,
  GXWM_CAPTURECHANGED         = 0x0215,
  GXWM_ENTERSIZEMOVE          = 0x0231,  // WM_ENTERSIZEMOVE
  GXWM_EXITSIZEMOVE           = 0x0232,  // WM_EXITSIZEMOVE
  GXWM_DROPFILES              = 0x0233,
  GXWM_IME_SETCONTEXT         = 0x0281,
  GXWM_IME_CONTROL            = 0x0283,
  GXWM_IME_COMPOSITIONFULL    = 0x0284,
  GXWM_IME_SELECT             = 0x0285,
  GXWM_IME_CHAR               = 0x0286,
  GXWM_NCMOUSEHOVER           = 0x02A0,
  GXWM_MOUSEHOVER             = 0x02A1,  // WM_MOUSEHOVER
  GXWM_NCMOUSELEAVE           = 0x02A2,
  GXWM_MOUSELEAVE             = 0x02A3,  // WM_MOUSELEAVE

  //GXWM_SETCONTAINER           = GXWM_SETADAPTER,  // lParam = pContainerObj
  GXWM_CUT                    = 0x0300,
  GXWM_COPY                   = 0x0301,
  GXWM_PASTE                  = 0x0302,
  GXWM_CLEAR                  = 0x0303,
  GXWM_UNDO                   = 0x0304,
  GXWM_HOTKEY                 = 0x0312,  // WM_HOTKEY
  GXWM_PRINTCLIENT            = 0x0318,
  GXWM_THEMECHANGED           = 0x031A,

//GXWM_SETOPACITY             = 0x0300,  // GXUI不透明性
//GXWM_GETCLSID               = 0x0301,  // GXUI得到类识别代码
  GXWM_GXPRIVATEBEGIN         = 0x03AF,  // GXUI 私有消息开始
  GXWM_DATAPOOLOPERATION      = 0x03B0,  // wParam = (DataPoolOperation), 根据DataPoolOperation 解释 lParam 含义
  GXWM_SOLVEDEFINITION        = 0x03B1,
  GXWM_SETCOBJECT             = 0x03B0,  // 在这个消息里设置 GXGWL_COBJECT 对象属性
//GXWM_GETADAPTER             = 0x02B1,  // lParam = (DataAdapter**) and add reference count, you must call Release() when you no longer needed.
//GXWM_SETDATAPOOL            = 0x02B2,  // lParam = pDataPoolObj
//GXWM_FLUSHCANVAS            GXWM_GXPRIVATEBEGIN      // 将窗口缓存图像刷新到屏幕，这个消息将发送GXWM_CALCCANVASSIZE，GXWM_SETRENDERCANVASSTATE，GXWM_RENDERCANVAS
//GXWM_CALCCANVASSIZE         (GXWM_GXPRIVATEBEGIN + 1)  // 计算窗口刷新到屏幕的尺寸，GXLPARAM指向了一个尺寸信息
//GXWM_SETRENDERCANVASSTATE   (GXWM_GXPRIVATEBEGIN + 2)  // 设置渲染的状态， GXWPARAM为0时是开始渲染的状态，非0为结束渲染状态，GXLPARAM指向了一个尺寸信息
//GXWM_RENDERCANVAS           (GXWM_GXPRIVATEBEGIN + 3)  // 刷新屏幕的操作消息
//GXWM_LOSTD3DDEVICE          (GXWM_GXPRIVATEBEGIN + 4)  // D3D 设备丢失
//GXWM_RESETD3DDEVICE         (GXWM_GXPRIVATEBEGIN + 5)  // D3D 复位
  GXWM_GXPRIVATEEND           = 0x03FF,  // GXUI 私有消息结束
  GXWM_USER                   = 0x0400,  // WM_USER
  GXWM_APP                    = 0x8000,  // WM_APP
  GXWM_STRING                 = 0xC000,  // 0xC000 through 0xFFFF String messages for use by applications.
}; // enum GXWndMsg

// GXWM_DATAPOOLOPERATION wParam 参数
enum DataPoolOperation
{
  DPO_SETDATAPOOL = 1,  // lParam = (DataAdapter*)
  DPO_GETDATAPOOL = 2,  // lParam = (DataAdapter**) and add reference count, you must call Release() when you no longer needed.
  DPO_SETVARIABLE = 3,  // lParam = (Variable*)
  DPO_GETVARIABLE = 4,  // lParam = (Variable*)
  DPO_SETADAPTER  = 5,  // lParam = (Adapter*)
  DPO_GETADAPTER  = 6,  // lParam = (Adapter**) add reference count
};

/*
 * WM_ACTIVATE state values
 */
#define     GXWA_INACTIVE     0
#define     GXWA_ACTIVE       1
#define     GXWA_CLICKACTIVE  2


#define GXWHEEL_DELTA                     120
#define GXGET_WHEEL_DELTA_WPARAM(wParam)  ((short)GXHIWORD(wParam))
/* Setting to scroll one page for SPI_GET/SETWHEELSCROLLLINES */
#define GXWHEEL_PAGESCROLL                (UINT_MAX)

//-----------------------------------------------------------
#define GXNM_FIRST                (0U-  0U)       // generic to all controls
#define GXNM_LAST                 (0U- 99U)

#define GXNM_OUTOFMEMORY          (GXNM_FIRST-1)
#define GXNM_CLICK                (GXNM_FIRST-2)    // uses NMCLICK struct
#define GXNM_DBLCLK               (GXNM_FIRST-3)
#define GXNM_RETURN               (GXNM_FIRST-4)
#define GXNM_RCLICK               (GXNM_FIRST-5)    // uses NMCLICK struct
#define GXNM_RDBLCLK              (GXNM_FIRST-6)
#define GXNM_SETFOCUS             (GXNM_FIRST-7)
#define GXNM_KILLFOCUS            (GXNM_FIRST-8)

#define GXNM_CUSTOMDRAW           (GXNM_FIRST-12)
#define GXNM_HOVER                (GXNM_FIRST-13)

#define GXNM_NCHITTEST            (GXNM_FIRST-14)   // uses NMMOUSE struct
#define GXNM_KEYDOWN              (GXNM_FIRST-15)   // uses NMKEY struct
#define GXNM_RELEASEDCAPTURE      (GXNM_FIRST-16)
#define GXNM_SETCURSOR            (GXNM_FIRST-17)   // uses NMMOUSE struct
#define GXNM_CHAR                 (GXNM_FIRST-18)   // uses NMCHAR struct

#define GXNM_TOOLTIPSCREATED      (GXNM_FIRST-19)   // notify of when the tooltips window is create

#define GXNM_LDOWN                (GXNM_FIRST-20)
#define GXNM_RDOWN                (GXNM_FIRST-21)
#define GXNM_THEMECHANGED         (GXNM_FIRST-22)
//-----------------------------------------------------------

/*
 * Window Styles
 */
#define GXWS_OVERLAPPED       0x00000000L
#define GXWS_POPUP            0x80000000L
#define GXWS_CHILD            0x40000000L
#define GXWS_MINIMIZE         0x20000000L
#define GXWS_VISIBLE          0x10000000L
#define GXWS_DISABLED         0x08000000L
#define GXWS_CLIPSIBLINGS     0x04000000L
#define GXWS_CLIPCHILDREN     0x02000000L
#define GXWS_MAXIMIZE         0x01000000L
#define GXWS_CAPTION          0x00C00000L     /* GXWS_BORDER | WS_DLGFRAME  */
#define GXWS_BORDER           0x00800000L
#define GXWS_DLGFRAME         0x00400000L
#define GXWS_VSCROLL          0x00200000L
#define GXWS_HSCROLL          0x00100000L
#define GXWS_SYSMENU          0x00080000L
#define GXWS_THICKFRAME       0x00040000L
#define GXWS_GROUP            0x00020000L
#define GXWS_TABSTOP          0x00010000L

#define GXWS_MINIMIZEBOX      0x00020000L
#define GXWS_MAXIMIZEBOX      0x00010000L

//#define  GXWS_ACTIVECAPTION    0x0001    // GXWS_ACTIVECAPTION


#define GXWS_TILED            GXWS_OVERLAPPED
#define GXWS_ICONIC           GXWS_MINIMIZE
#define GXWS_SIZEBOX          GXWS_THICKFRAME
#define GXWS_TILEDWINDOW      GXWS_OVERLAPPEDWINDOW

/*
 * Common Window Styles
 */
#define GXWS_OVERLAPPEDWINDOW (GXWS_OVERLAPPED     | \
                               GXWS_CAPTION        | \
                               GXWS_SYSMENU        | \
                               GXWS_THICKFRAME     | \
                               GXWS_MINIMIZEBOX    | \
                               GXWS_MAXIMIZEBOX)

#define GXWS_POPUPWINDOW      (GXWS_POPUP          | \
                               GXWS_BORDER         | \
                               GXWS_SYSMENU)

#define GXWS_CHILDWINDOW      (GXWS_CHILD)

/*
 * Extended Window Styles
 */
#define GXWS_EX_DLGMODALFRAME     0x00000001L
#define GXWS_EX_IMPALPABLE        0x00000002L   // GrapX 专有, 看得见摸不着的
#define GXWS_EX_NOPARENTNOTIFY    0x00000004L
#define GXWS_EX_TOPMOST           0x00000008L
#define GXWS_EX_ACCEPTFILES       0x00000010L
#define GXWS_EX_TRANSPARENT       0x00000020L

#define GXWS_EX_MDICHILD          0x00000040L
#define GXWS_EX_TOOLWINDOW        0x00000080L
#define GXWS_EX_WINDOWEDGE        0x00000100L
#define GXWS_EX_CLIENTEDGE        0x00000200L
#define GXWS_EX_CONTEXTHELP       0x00000400L

#define GXWS_EX_RIGHT             0x00001000L
#define GXWS_EX_LEFT              0x00000000L
#define GXWS_EX_RTLREADING        0x00002000L
#define GXWS_EX_LTRREADING        0x00000000L
#define GXWS_EX_LEFTSCROLLBAR     0x00004000L
#define GXWS_EX_RIGHTSCROLLBAR    0x00000000L

#define GXWS_EX_CONTROLPARENT     0x00010000L
#define GXWS_EX_STATICEDGE        0x00020000L
#define GXWS_EX_APPWINDOW         0x00040000L


#define GXWS_EX_OVERLAPPEDWINDOW  (GXWS_EX_WINDOWEDGE | GXWS_EX_CLIENTEDGE)
#define GXWS_EX_PALETTEWINDOW     (GXWS_EX_WINDOWEDGE | GXWS_EX_TOOLWINDOW | GXWS_EX_TOPMOST)
#define GXWS_EX_LAYERED           0x00080000
#define GXWS_EX_NOINHERITLAYOUT   0x00100000L // Disable inheritence of mirroring by children
#define GXWS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring
#define GXWS_EX_COMPOSITED        0x02000000L
#define GXWS_EX_NOACTIVATE        0x08000000L

/*
 * Dialog Styles
 */
#define GXDS_ABSALIGN         0x01L
#define GXDS_SYSMODAL         0x02L
#define GXDS_LOCALEDIT        0x20L   /* Edit items get Local storage. */
#define GXDS_SETFONT          0x40L   /* User specified font for Dlg controls */
#define GXDS_MODALFRAME       0x80L   /* Can be combined with WS_CAPTION  */
#define GXDS_NOIDLEMSG        0x100L  /* WM_ENTERIDLE message will not be sent */
#define GXDS_SETFOREGROUND    0x200L  /* not in win3.1 */

#define GXDS_3DLOOK           0x0004L
#define GXDS_FIXEDSYS         0x0008L
#define GXDS_NOFAILCREATE     0x0010L
#define GXDS_CONTROL          0x0400L
#define GXDS_CENTER           0x0800L
#define GXDS_CENTERMOUSE      0x1000L
#define GXDS_CONTEXTHELP      0x2000L
#define GXDS_SHELLFONT        (GXDS_SETFONT | GXDS_FIXEDSYS)
#define GXDS_USEPIXELS        0x8000L

#define GXDM_GETDEFID         (GXWM_USER+0)
#define GXDM_SETDEFID         (GXWM_USER+1)
#define GXDM_REPOSITION       (GXWM_USER+2)
/*
 * Returned in HIWORD() of DM_GETDEFID result if msg is supported
 */
#define GXDC_HASDEFID         0x534B

/*
 * Class styles
 */
#define GXCS_VREDRAW          0x0001
#define GXCS_HREDRAW          0x0002
#define GXCS_DBLCLKS          0x0008
#define GXCS_OWNDC            0x0020
#define GXCS_CLASSDC          0x0040
#define GXCS_PARENTDC         0x0080
#define GXCS_NOCLOSE          0x0200
#define GXCS_SAVEBITS         0x0800
#define GXCS_BYTEALIGNCLIENT  0x1000
#define GXCS_BYTEALIGNWINDOW  0x2000
#define GXCS_GLOBALCLASS      0x4000
#define GXCS_IME              0x00010000
#define GXCS_DROPSHADOW       0x00020000




#define GXGWL_USERDATA           (-21)
#define GXGWL_EXSTYLE            (-20)  // GWL_EXSTYLE
#define GXGWL_STYLE              (-16)  // GWL_STYLE
#define GXGWL_ID                 (-12)  // GWL_ID
#define GXGWL_HINSTANCE          (-6)
#define GXGWL_WNDPROC            (-4)  // GWL_WNDPROC
#define GXGWL_COBJECT            (-1)  // TODO: 准备去掉这个
#define GXGWL_RESPONDER          (-2)  // C++ 用的响应器

#define GXGWLP_WNDPROC        (-4)
#define GXGWLP_HINSTANCE      (-6)
#define GXGWLP_HWNDPARENT     (-8)
#define GXGWLP_USERDATA       (-21)
#define GXGWLP_ID             (-12)

/*
 * Class field offsets for GetClassLong()
 */
//#define GXGCL_MENUNAME        (-8)
//#define GXGCL_HBRBACKGROUND   (-10)
#define GXGCL_HCURSOR         (-12)
//#define GXGCL_HICON           (-14)
//#define GXGCL_HMODULE         (-16)
//#define GXGCL_CBWNDEXTRA      (-18)
//#define GXGCL_CBCLSEXTRA      (-20)
//#define GXGCL_WNDPROC         (-24)
#define GXGCL_STYLE           (-26)
//#define GXGCW_ATOM            (-32)
//#define GXGCL_HICONSM         (-34)

//#define GXGCLP_MENUNAME       (-8)
#define GXGCLP_HBRBACKGROUND  (-10)
//#define GXGCLP_HCURSOR        (-12)
//#define GXGCLP_HICON          (-14)
//#define GXGCLP_HMODULE        (-16)
//#define GXGCLP_WNDPROC        (-24)
//#define GXGCLP_HICONSM        (-34)



//
// GetWindow() Constants
//
#define GXGW_HWNDFIRST        0
#define GXGW_HWNDLAST         1
#define GXGW_HWNDNEXT         2
#define GXGW_HWNDPREV         3
#define GXGW_OWNER            4
#define GXGW_CHILD            5
#define GXGW_ENABLEDPOPUP     6
#define GXGW_MAX              6

//
// gxShowWindow() Commands
//
#define GXSW_HIDE             0      // SW_HIDE             
#define GXSW_SHOWNORMAL       1      // SW_SHOWNORMAL       
#define GXSW_NORMAL           1      // SW_NORMAL           
#define GXSW_SHOWMINIMIZED    2      // SW_SHOWMINIMIZED    
#define GXSW_SHOWMAXIMIZED    3      // SW_SHOWMAXIMIZED    
#define GXSW_MAXIMIZE         3      // SW_MAXIMIZE         
#define GXSW_SHOWNOACTIVATE   4      // SW_SHOWNOACTIVATE   
#define GXSW_SHOW             5      // SW_SHOW             
#define GXSW_MINIMIZE         6      // SW_MINIMIZE         
#define GXSW_SHOWMINNOACTIVE  7      // SW_SHOWMINNOACTIVE  
#define GXSW_SHOWNA           8      // SW_SHOWNA           
#define GXSW_RESTORE          9      // SW_RESTORE          
#define GXSW_SHOWDEFAULT      10    // SW_SHOWDEFAULT      
#define GXSW_FORCEMINIMIZE    11    // SW_FORCEMINIMIZE    
#define GXSW_MAX              11    // SW_MAX              

#define BYTE_ALIGN_4(x)      (((x) + 3) & (~3))
#define BYTE_ALIGN_32(x)    (((x) + 31) & (~31))
#define GXT(__STR)        ((GXWCHAR*)L##__STR)
#define GXMAKELONG(l, h)    ((GXLONG)(((GXWORD)((GXDWORD)(l) & 0xffff)) | ((GXDWORD)((GXWORD)((GXDWORD)(h) & 0xffff))) << 16))
#define GXMAKEWPARAM(l, h)      ((GXWPARAM)(GXDWORD)GXMAKELONG(l, h))
#define GXMAKELPARAM(l, h)      ((GXLPARAM)(GXDWORD)GXMAKELONG(l, h))
#define GXMAKELRESULT(l, h)     ((GXLRESULT)(GXDWORD)GXMAKELONG(l, h))
#define GXLOWORD(l)        ((GXWORD)((GXDWORD)(l) & 0xffff))
#define GXHIWORD(l)        ((GXWORD)((GXDWORD)(l) >> 16))
#define GXLOBYTE(w)        ((GXBYTE)((GXDWORD)(w) & 0xff))
#define GXHIBYTE(w)        ((GXBYTE)((GXDWORD)(w) >> 8))

#define GXGET_X_LPARAM(l)  ((GXINT)(GXSHORT)((l) & 0xffff))
#define GXGET_Y_LPARAM(l)  ((GXINT)(GXSHORT)((l) >> 16))

#define GXMAKEFOURCC(ch0, ch1, ch2, ch3) CLMAKEFOURCC(ch0, ch1, ch2, ch3)

#define GXIS_INTRESOURCE(_r) ((((GXULONG_PTR)(_r)) >> 16) == 0)
#define GXMAKEINTRESOURCEA(i) ((GXLPSTR)((GXULONG_PTR)((GXWORD)(i))))
#define GXMAKEINTRESOURCEW(i) ((GXLPWSTR)((GXULONG_PTR)((GXWORD)(i))))
#ifdef _UNICODE
#define GXMAKEINTRESOURCE  GXMAKEINTRESOURCEW
#else
#define GXMAKEINTRESOURCE  GXMAKEINTRESOURCEA
#endif // !_UNICODE


/*
 * SetWindowPos Flags
 */
#define GXSWP_NOSIZE          0x0001
#define GXSWP_NOMOVE          0x0002
#define GXSWP_NOZORDER        0x0004
#define GXSWP_NOREDRAW        0x0008
#define GXSWP_NOACTIVATE      0x0010
#define GXSWP_FRAMECHANGED    0x0020  /* The frame changed: send WM_NCCALCSIZE */
#define GXSWP_SHOWWINDOW      0x0040
#define GXSWP_HIDEWINDOW      0x0080
#define GXSWP_NOCOPYBITS      0x0100
#define GXSWP_NOOWNERZORDER   0x0200  /* Don't do owner Z ordering */
#define GXSWP_NOSENDCHANGING  0x0400  /* Don't send WM_WINDOWPOSCHANGING */
#define GXSWP_DRAWFRAME       GXSWP_FRAMECHANGED
#define GXSWP_NOREPOSITION    GXSWP_NOOWNERZORDER
#define GXSWP_DEFERERASE      0x2000
#define GXSWP_ASYNCWINDOWPOS  0x4000

#define GXHWND_TOP        ((GXHWND)0)   // HWND_TOP
#define GXHWND_BOTTOM     ((GXHWND)1)   // HWND_BOTTOM
#define GXHWND_TOPMOST    ((GXHWND)-1)  // HWND_TOPMOST
#define GXHWND_NOTOPMOST  ((GXHWND)-2)  // HWND_NOTOPMOST
#define GXHWND_BROADCAST  ((GXHWND)0xffff)

//
// Hit Test
//
#define GXHTERROR             (-2)
#define GXHTTRANSPARENT       (-1)
#define GXHTNOWHERE           0
#define GXHTCLIENT            1
#define GXHTCAPTION           2
#define GXHTSYSMENU           3
#define GXHTGROWBOX           4
#define GXHTSIZE              GXHTGROWBOX
#define GXHTMENU              5
#define GXHTHSCROLL           6
#define GXHTVSCROLL           7
#define GXHTMINBUTTON         8
#define GXHTMAXBUTTON         9
#define GXHTLEFT              10
#define GXHTRIGHT             11
#define GXHTTOP               12
#define GXHTTOPLEFT           13
#define GXHTTOPRIGHT          14
#define GXHTBOTTOM            15
#define GXHTBOTTOMLEFT        16
#define GXHTBOTTOMRIGHT       17
#define GXHTBORDER            18
#define GXHTREDUCE            GXHTMINBUTTON
#define GXHTZOOM              GXHTMAXBUTTON
#define GXHTSIZEFIRST         GXHTLEFT
#define GXHTSIZELAST          GXHTBOTTOMRIGHT
#define GXHTOBJECT            19
#define GXHTCLOSE             20
#define GXHTHELP              21


//
// GXWM_SIZE
//
#define GXSIZE_RESTORED       0
#define GXSIZE_MINIMIZED      1
#define GXSIZE_MAXIMIZED      2
#define GXSIZE_MAXSHOW        3
#define GXSIZE_MAXHIDE        4

//
// GXWM_COMMAND
//
#define GXBN_CLICKED        0  // BN_CLICKED
#define GXBN_MOUSEHOVER     1  // BN_CLICKED
#define GXBN_MOUSELEAVE     2  // BN_CLICKED
#define GXBN_FADEOUT        3  // 

//
// GXWM_KEYDOWN
// GXWM_KEYUP
//
#define GXVK_BACK           0x08
#define GXVK_TAB            0x09
#define GXVK_RETURN         0x0D
#define GXVK_SHIFT          0x10
#define GXVK_CONTROL        0x11
#define GXVK_MENU           0x12
#define GXVK_ESCAPE         0x1B  // VK_ESCAPE
#define GXVK_SPACE          0x20  // VK_SPACE
#define GXVK_PRIOR          0x21
#define GXVK_NEXT           0x22
#define GXVK_END            0x23
#define GXVK_HOME           0x24
#define GXVK_LEFT           0x25  // VK_LEFT
#define GXVK_UP             0x26  // VK_UP
#define GXVK_RIGHT          0x27  // VK_RIGHT
#define GXVK_DOWN           0x28  // VK_DOWN
#define GXVK_INSERT         0x2D
#define GXVK_DELETE         0x2E
#define GXVK_NUMPAD0        0x60
#define GXVK_NUMPAD1        0x61
#define GXVK_NUMPAD2        0x62
#define GXVK_NUMPAD3        0x63
#define GXVK_NUMPAD4        0x64
#define GXVK_NUMPAD5        0x65
#define GXVK_NUMPAD6        0x66
#define GXVK_NUMPAD7        0x67
#define GXVK_NUMPAD8        0x68
#define GXVK_NUMPAD9        0x69
#define GXVK_MULTIPLY       0x6A
#define GXVK_ADD            0x6B
#define GXVK_SUBTRACT       0x6D
#define GXVK_F1             0x70
#define GXVK_F2             0x71
#define GXVK_F3             0x72
#define GXVK_F4             0x73
#define GXVK_F5             0x74
#define GXVK_F6             0x75
#define GXVK_F7             0x76
#define GXVK_F8             0x77
#define GXVK_F9             0x78
#define GXVK_F10            0x79
#define GXVK_F11            0x7A
#define GXVK_F12            0x7B
#define GXVK_F13            0x7C
#define GXVK_F14            0x7D
#define GXVK_F15            0x7E
#define GXVK_F16            0x7F
#define GXVK_F17            0x80
#define GXVK_F18            0x81
#define GXVK_F19            0x82
#define GXVK_F20            0x83
#define GXVK_F21            0x84
#define GXVK_F22            0x85
#define GXVK_F23            0x86
#define GXVK_F24            0x87
#define GXVK_LSHIFT         0xA0
#define GXVK_RSHIFT         0xA1
#define GXVK_LCONTROL       0xA2
#define GXVK_RCONTROL       0xA3


/*
 * 0xB8 - 0xB9 : reserved
 */

#define GXVK_OEM_1          0xBA   // ';:' for US
#define GXVK_OEM_PLUS       0xBB   // '+' any country
#define GXVK_OEM_COMMA      0xBC   // ',' any country
#define GXVK_OEM_MINUS      0xBD   // '-' any country
#define GXVK_OEM_PERIOD     0xBE   // '.' any country
#define GXVK_OEM_2          0xBF   // '/?' for US
#define GXVK_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define GXVK_OEM_4          0xDB  //  '[{' for US
#define GXVK_OEM_5          0xDC  //  '\|' for US
#define GXVK_OEM_6          0xDD  //  ']}' for US
#define GXVK_OEM_7          0xDE  //  ''"' for US
#define GXVK_OEM_8          0xDF

//
// Keys
// 
#define GXMK_LBUTTON          0x0001
#define GXMK_RBUTTON          0x0002
#define GXMK_SHIFT            0x0004
#define GXMK_CONTROL          0x0008
#define GXMK_MBUTTON          0x0010
#define GXMK_WHEEL            0x0100

#define GXWS_DISABLED         0x08000000L
//#define GXWS_EX_TOPMOST       0x00000008L
//#define GXWS_EX_TRANSPARENT   0x00000020L  // WS_EX_TRANSPARENT

#define GXTME_HOVER       0x00000001
#define GXTME_LEAVE       0x00000002
#define GXTME_NONCLIENT   0x00000010
#define GXTME_QUERY       0x40000000
#define GXTME_CANCEL      0x80000000
#define GXHOVER_DEFAULT   0xFFFFFFFF

//
// MessageBox
//
#define GXMB_OK                       0x00000000L
#define GXMB_OKCANCEL                 0x00000001L
#define GXMB_ABORTRETRYIGNORE         0x00000002L
#define GXMB_YESNOCANCEL              0x00000003L
#define GXMB_YESNO                    0x00000004L
#define GXMB_RETRYCANCEL              0x00000005L


//#ifdef _WINDOWS
//GXVOID _WinVerifyFailure(GXCHAR *pszSrcFile,GXINT nLine, GXDWORD dwErrorNum);
//void _gxTrace(char *fmt, ...);
////#define VERIFY(v)  if(!(v))  _WinVerifyFailure(__FILE__,__LINE__, GetLastError())
//#define GXTRACE    _gxTrace
//#else
//#define VERIFY(v)  v
//#define GXTRACE
//#endif
#ifndef GXCCSIZEOF_STRUCT
#define GXCCSIZEOF_STRUCT(structname, member)  (((int)((GXLPBYTE)(&((structname*)0)->member) - ((GXLPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif


//====== Generic WM_NOTIFY notification structures ============================

typedef struct tagGXNMTOOLTIPSCREATED
{
  GXNMHDR hdr;
  GXHWND hwndToolTips;
} GXNMTOOLTIPSCREATED, * GXLPNMTOOLTIPSCREATED;
typedef struct tagGXNMMOUSE {
  GXNMHDR   hdr;
  GXDWORD_PTR dwItemSpec;
  GXDWORD_PTR dwItemData;
  GXPOINT   pt;
  GXLPARAM  dwHitInfo; // any specifics about where on the item or control the mouse is
} GXNMMOUSE, *GXLPNMMOUSE;

typedef GXNMMOUSE GXNMCLICK;
typedef GXLPNMMOUSE GXLPNMCLICK;

// Generic structure to request an object of a specific type.

typedef struct tagGXNMOBJECTNOTIFY {
  GXNMHDR   hdr;
  int     iItem;
#ifdef __IID_DEFINED__
  const IID *piid;
#else
  const void *piid;
#endif
  void *pObject;
  GXHRESULT hResult;
  GXDWORD dwFlags;    // control specific flags (hints as to where in iItem it hit)
} GXNMOBJECTNOTIFY, *GXLPNMOBJECTNOTIFY;

// Generic structure for a key

typedef struct tagGXNMKEY
{
  GXNMHDR hdr;
  GXUINT  nVKey;
  GXUINT  uFlags;
} GXNMKEY, *GXLPNMKEY;

// Generic structure for a character

typedef struct tagGXNMCHAR {
  GXNMHDR   hdr;
  GXUINT    ch;
  GXDWORD   dwItemPrev;     // Item previously selected
  GXDWORD   dwItemNext;     // Item to be selected
} GXNMCHAR, *GXLPNMCHAR;

// GXWM_CALCCANVASSIZE 结构
typedef struct __tagCANVASSIZEINFO{
  GXSIZE    sizeRenderingSur;//GXINT nWidth, nHeight;
  GXREGN    rgDest;
  GXRECT    rcRenderingSrc;
  GXCanvas*  pCanvas;
  GXEffect*  pEffect;
}CANVASSIZEINFO, *LPCANVASSIZEINFO;

//
// Canvas Common Constants
//
struct GXCANVASCOMMCONST
{
  float4x4  matWVProj; // TODO: 计划改名为Transform
  float4    colorMul;
  float4    colorAdd;
};

struct GXPALETTEENTRY {
  GXBYTE        peRed;
  GXBYTE        peGreen;
  GXBYTE        peBlue;
  GXBYTE        peFlags;
};
typedef GXPALETTEENTRY *GXLPPALETTEENTRY;
typedef GXPALETTEENTRY *LPGXPALETTEENTRY;


enum GXRenderStateType {
  //GXRS_ZENABLE                 = 7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
  GXRS_FILLMODE                  = 8,    /* D3DFILLMODE */
  GXRS_SHADEMODE                 = 9,    /* D3DSHADEMODE */
  //GXRS_ZWRITEENABLE            = 14,   /* TRUE to enable z writes */
  GXRS_ALPHATESTENABLE           = 15,   /* TRUE to enable alpha tests */
  GXRS_LASTPIXEL                 = 16,   /* TRUE for last-pixel on lines */
  //GXRS_SRCBLEND                = 19,   /* D3DBLEND */
  //GXRS_DESTBLEND               = 20,   /* D3DBLEND */
  GXRS_CULLMODE                  = 22,   /* D3DCULL */
  //GXRS_ZFUNC                   = 23,   /* GXCompareFunc */
  GXRS_ALPHAREF                  = 24,   /* D3DFIXED */
  GXRS_ALPHAFUNC                 = 25,   /* GXCompareFunc */
  GXRS_DITHERENABLE              = 26,   /* TRUE to enable dithering */
  //GXRS_ALPHABLENDENABLE        = 27,   /* TRUE to enable alpha blending */
  GXRS_FOGENABLE                 = 28,   /* TRUE to enable fog blending */
  GXRS_SPECULARENABLE            = 29,   /* TRUE to enable specular */
  GXRS_FOGCOLOR                  = 34,   /* D3DCOLOR */
  GXRS_FOGTABLEMODE              = 35,   /* D3DFOGMODE */
  GXRS_FOGSTART                  = 36,   /* Fog start (for both vertex and pixel fog) */
  GXRS_FOGEND                    = 37,   /* Fog end      */
  GXRS_FOGDENSITY                = 38,   /* Fog density  */
  GXRS_RANGEFOGENABLE            = 48,   /* Enables range-based fog */
  //GXRS_STENCILENABLE           = 52,   /* GXBOOL enable/disable stenciling */
  //GXRS_STENCILFAIL             = 53,   /* D3DSTENCILOP to do if stencil test fails */
  //GXRS_STENCILZFAIL            = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
  //GXRS_STENCILPASS             = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
  //GXRS_STENCILFUNC             = 56,   /* GXCompareFunc fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
  //GXRS_STENCILREF              = 57,   /* Reference value used in stencil test */
  //GXRS_STENCILMASK             = 58,   /* Mask value used in stencil test */
  //GXRS_STENCILWRITEMASK        = 59,   /* Write mask applied to values written to stencil buffer */
  GXRS_TEXTUREFACTOR             = 60,   /* D3DCOLOR used for multi-texture blend */
  GXRS_WRAP0                     = 128,  /* wrap for 1st texture coord. set */
  GXRS_WRAP1                     = 129,  /* wrap for 2nd texture coord. set */
  GXRS_WRAP2                     = 130,  /* wrap for 3rd texture coord. set */
  GXRS_WRAP3                     = 131,  /* wrap for 4th texture coord. set */
  GXRS_WRAP4                     = 132,  /* wrap for 5th texture coord. set */
  GXRS_WRAP5                     = 133,  /* wrap for 6th texture coord. set */
  GXRS_WRAP6                     = 134,  /* wrap for 7th texture coord. set */
  GXRS_WRAP7                     = 135,  /* wrap for 8th texture coord. set */
  GXRS_CLIPPING                  = 136,
  GXRS_LIGHTING                  = 137,
  GXRS_AMBIENT                   = 139,
  GXRS_FOGVERTEXMODE             = 140,
  GXRS_COLORVERTEX               = 141,
  GXRS_LOCALVIEWER               = 142,
  GXRS_NORMALIZENORMALS          = 143,
  GXRS_DIFFUSEMATERIALSOURCE     = 145,
  GXRS_SPECULARMATERIALSOURCE    = 146,
  GXRS_AMBIENTMATERIALSOURCE     = 147,
  GXRS_EMISSIVEMATERIALSOURCE    = 148,
  GXRS_VERTEXBLEND               = 151,
  GXRS_CLIPPLANEENABLE           = 152,
  GXRS_POINTSIZE                 = 154,   /* float point size */
  GXRS_POINTSIZE_MIN             = 155,   /* float point size min threshold */
  GXRS_POINTSPRITEENABLE         = 156,   /* GXBOOL point texture coord control */
  GXRS_POINTSCALEENABLE          = 157,   /* GXBOOL point size scale enable */
  GXRS_POINTSCALE_A              = 158,   /* float point attenuation A value */
  GXRS_POINTSCALE_B              = 159,   /* float point attenuation B value */
  GXRS_POINTSCALE_C              = 160,   /* float point attenuation C value */
  GXRS_MULTISAMPLEANTIALIAS      = 161,  // GXBOOL - set to do FSAA with multisample buffer
  GXRS_MULTISAMPLEMASK           = 162,  // GXDWORD - per-sample enable/disable
  GXRS_PATCHEDGESTYLE            = 163,  // Sets whether patch edges will use float style tessellation
  GXRS_DEBUGMONITORTOKEN         = 165,  // DEBUG ONLY - token to debug monitor
  GXRS_POINTSIZE_MAX             = 166,   /* float point size max threshold */
  GXRS_INDEXEDVERTEXBLENDENABLE  = 167,
  GXRS_COLORWRITEENABLE          = 168,  // per-channel write enable
  GXRS_TWEENFACTOR               = 170,   // float tween factor
  //GXRS_BLENDOP                 = 171,   // D3DBLENDOP setting
  GXRS_POSITIONDEGREE            = 172,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
  GXRS_NORMALDEGREE              = 173,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
  GXRS_SCISSORTESTENABLE         = 174,
  GXRS_SLOPESCALEDEPTHBIAS       = 175,
  GXRS_ANTIALIASEDLINEENABLE     = 176,
  GXRS_MINTESSELLATIONLEVEL      = 178,
  GXRS_MAXTESSELLATIONLEVEL      = 179,
  GXRS_ADAPTIVETESS_X            = 180,
  GXRS_ADAPTIVETESS_Y            = 181,
  GXRS_ADAPTIVETESS_Z            = 182,
  GXRS_ADAPTIVETESS_W            = 183,
  GXRS_ENABLEADAPTIVETESSELLATION = 184,
  GXRS_TWOSIDEDSTENCILMODE       = 185,   /* GXBOOL enable/disable 2 sided stenciling */
  //GXRS_CCW_STENCILFAIL         = 186,   /* D3DSTENCILOP to do if ccw stencil test fails */
  //GXRS_CCW_STENCILZFAIL        = 187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
  //GXRS_CCW_STENCILPASS         = 188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
  //GXRS_CCW_STENCILFUNC         = 189,   /* GXCompareFunc fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
  GXRS_COLORWRITEENABLE1         = 190,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
  GXRS_COLORWRITEENABLE2         = 191,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
  GXRS_COLORWRITEENABLE3         = 192,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
  GXRS_BLENDFACTOR               = 193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
  GXRS_SRGBWRITEENABLE           = 194,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
  GXRS_DEPTHBIAS                 = 195,
  GXRS_WRAP8                     = 198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
  GXRS_WRAP9                     = 199,
  GXRS_WRAP10                    = 200,
  GXRS_WRAP11                    = 201,
  GXRS_WRAP12                    = 202,
  GXRS_WRAP13                    = 203,
  GXRS_WRAP14                    = 204,
  GXRS_WRAP15                    = 205,
  //GXRS_SEPARATEALPHABLENDENABLE= 206,  /* TRUE to enable a separate blending function for the alpha channel */
  //GXRS_SRCBLENDALPHA           = 207,  /* SRC blend factor for the alpha channel when GXRS_SEPARATEDESTALPHAENABLE is TRUE */
  //GXRS_DESTBLENDALPHA          = 208,  /* DST blend factor for the alpha channel when GXRS_SEPARATEDESTALPHAENABLE is TRUE */
  //GXRS_BLENDOPALPHA            = 209,  /* Blending operation for the alpha channel when GXRS_SEPARATEDESTALPHAENABLE is TRUE */


  GXRS_FORCE_DWORD               = 0x7fffffff, /* force 32-bit size enum */
};

// 已经不再使用这个, 参考 GXSAMPLERDESC
//enum GXSamplerStateType
//{
//    GXSAMP_ADDRESSU       = 1,  /* D3DTEXTUREADDRESS for U coordinate */
//    GXSAMP_ADDRESSV       = 2,  /* D3DTEXTUREADDRESS for V coordinate */
//    GXSAMP_ADDRESSW       = 3,  /* D3DTEXTUREADDRESS for W coordinate */
//    GXSAMP_BORDERCOLOR    = 4,  /* D3DCOLOR */
//    GXSAMP_MAGFILTER      = 5,  /* D3DTEXTUREFILTER filter to use for magnification */
//    GXSAMP_MINFILTER      = 6,  /* D3DTEXTUREFILTER filter to use for minification */
//    GXSAMP_MIPFILTER      = 7,  /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
//    GXSAMP_MIPMAPLODBIAS  = 8,  /* float Mipmap LOD bias */
//    GXSAMP_MAXMIPLEVEL    = 9,  /* GXDWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
//    GXSAMP_MAXANISOTROPY  = 10, /* GXDWORD maximum anisotropy */
//    GXSAMP_SRGBTEXTURE    = 11, /* Default = 0 (which means Gamma 1.0,
//                                   no correction required.) else correct for
//                                   Gamma = 2.2 */
//    GXSAMP_ELEMENTINDEX   = 12, /* When multi-element texture is assigned to sampler, this
//                                    indicates which element index to use.  Default = 0.  */
//    GXSAMP_DMAPOFFSET     = 13, /* Offset in vertices in the pre-sampled displacement map.
//                                    Only valid for D3DDMAPSAMPLER sampler  */
//    GXSAMP_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */
//};

enum GXGraphicsFormat
{
  GXFMT_UNKNOWN              =  0,

  GXFMT_R8G8B8               = 20,
  GXFMT_A8R8G8B8             = 21,
  GXFMT_X8R8G8B8             = 22,
  GXFMT_R5G6B5               = 23,
  GXFMT_X1R5G5B5             = 24,
  GXFMT_A1R5G5B5             = 25,
  GXFMT_A4R4G4B4             = 26,
  GXFMT_R3G3B2               = 27,
  GXFMT_A8                   = 28,
  GXFMT_A8R3G3B2             = 29,
  GXFMT_X4R4G4B4             = 30,
  GXFMT_A2B10G10R10          = 31,
  GXFMT_A8B8G8R8             = 32,
  GXFMT_X8B8G8R8             = 33,
  GXFMT_G16R16               = 34,
  GXFMT_A2R10G10B10          = 35,
  GXFMT_A16B16G16R16         = 36,

  GXFMT_A8P8                 = 40,
  GXFMT_P8                   = 41,

  GXFMT_L8                   = 50,
  GXFMT_A8L8                 = 51,
  GXFMT_A4L4                 = 52,

  GXFMT_V8U8                 = 60,
  GXFMT_L6V5U5               = 61,
  GXFMT_X8L8V8U8             = 62,
  GXFMT_Q8W8V8U8             = 63,
  GXFMT_V16U16               = 64,
  GXFMT_A2W10V10U10          = 67,

  //GXFMT_UYVY               = GXMAKEFOURCC('U', 'Y', 'V', 'Y'),
  //GXFMT_R8G8_B8G8          = GXMAKEFOURCC('R', 'G', 'B', 'G'),
  //GXFMT_YUY2               = GXMAKEFOURCC('Y', 'U', 'Y', '2'),
  //GXFMT_G8R8_G8B8          = GXMAKEFOURCC('G', 'R', 'G', 'B'),
  GXFMT_DXT1                 = GXMAKEFOURCC('D', 'X', 'T', '1'),
  GXFMT_DXT2                 = GXMAKEFOURCC('D', 'X', 'T', '2'),
  GXFMT_DXT3                 = GXMAKEFOURCC('D', 'X', 'T', '3'),
  GXFMT_DXT4                 = GXMAKEFOURCC('D', 'X', 'T', '4'),
  GXFMT_DXT5                 = GXMAKEFOURCC('D', 'X', 'T', '5'),

  GXFMT_D16_LOCKABLE         = 70,
  GXFMT_D32                  = 71,
  GXFMT_D15S1                = 73,
  GXFMT_D24S8                = 75,
  GXFMT_D24X8                = 77,
  GXFMT_D24X4S4              = 79,
  GXFMT_D16                  = 80,

  GXFMT_D32F_LOCKABLE        = 82,
  GXFMT_D24FS8               = 83,

  /* Z-Stencil formats valid for CPU access */
  GXFMT_D32_LOCKABLE         = 84,
  GXFMT_S8_LOCKABLE          = 85,

  GXFMT_L16                  = 81,

  GXFMT_VERTEXDATA           =100,
  GXFMT_INDEX16              =101,
  GXFMT_INDEX32              =102,

  //GXFMT_Q16W16V16U16       =110,

  //GXFMT_MULTI2_ARGB8       = GXMAKEFOURCC('M','E','T','1'),

  // Floating point surface formats

  // s10e5 formats (16-bits per channel)
  GXFMT_R16F                 = 111,
  GXFMT_G16R16F              = 112,
  GXFMT_A16B16G16R16F        = 113,

  // IEEE s23e8 formats (32-bits per channel)
  GXFMT_R32F                 = 114,
  GXFMT_G32R32F              = 115,
  GXFMT_A32B32G32R32F        = 116,

  GXFMT_CxV8U8               = 117,

  // Monochrome 1 bit per pixel format
  GXFMT_A1                   = 118,

  // 2.8 biased fixed point
  //GXFMT_A2B10G10R10_XR_BIAS= 119,


  // Binary format indicating that the data has no inherent type
  //GXFMT_BINARYBUFFER       = 199,

  GXFMT_FORCE_DWORD          =0x7fffffff
};
typedef GXGraphicsFormat GXFormat;

struct GXIMAGEINFOX
{
  GXUINT              Width;
  GXUINT              Height;
  GXUINT              Depth;
  GXUINT              MipLevels;
  GXGraphicsFormat    Format;
  //D3DRESOURCETYPE      ResourceType;
  //D3DXIMAGE_FILEFORMAT ImageFileFormat;
};
typedef GXIMAGEINFOX *LPGXIMAGEINFOX;
typedef GXIMAGEINFOX *GXLPIMAGEINFOX;

//
// 格式类别
//
enum GXFormatCategory
{
  GXFMTCATE_OTHER           = 0,
  GXFMTCATE_COLOR           = 1,
  GXFMTCATE_COMPRESSEDCOLOR = 2,
  GXFMTCATE_DEPTHSTENCIL    = 3,
};

//enum GXPool 
//{
//  GXPOOL_DEFAULT                 = 0,
//  GXPOOL_SYSTEMMEM               = 2,
//
//  GXPOOL_FORCE_DWORD             = 0x7fffffff
//};


#define GXLOCK_READONLY           0x00000010L
#define GXLOCK_DISCARD            0x00002000L
#define GXLOCK_NOOVERWRITE        0x00001000L
//#define GXLOCK_NOSYSLOCK          0x00000800L
//#define GXLOCK_DONOTWAIT          0x00004000L
//#define GXLOCK_NO_DIRTY_UPDATE     0x00008000L

//////////////////////////////////////////////////////////////////////////
//#define GXUSAGE_RENDERTARGET       (0x00000001L)
//#define GXUSAGE_DEPTHSTENCIL       (0x00000002L)
//#define GXUSAGE_DYNAMIC            (0x00000200L)
//
//#define GXUSAGE_AUTOGENMIPMAP      (0x00000400L)
//#define GXUSAGE_DMAP               (0x00004000L)
//
///* Usages for Vertex/Index buffers */
//#define GXUSAGE_WRITEONLY          (0x00000008L)
//#define GXUSAGE_SOFTWAREPROCESSING (0x00000010L)
//#define GXUSAGE_DONOTCLIP          (0x00000020L)
//#define GXUSAGE_POINTS             (0x00000040L)
//#define GXUSAGE_RTPATCHES          (0x00000080L)
//#define GXUSAGE_NPATCHES           (0x00000100L)

//////////////////////////////////////////////////////////////////////////
#define GXFILTER_NONE             (1 << 0)
#define GXFILTER_POINT            (2 << 0)
#define GXFILTER_LINEAR           (3 << 0)
#define GXFILTER_TRIANGLE         (4 << 0)
#define GXFILTER_BOX              (5 << 0)

#define GXFILTER_MIRROR_U         (1 << 16)
#define GXFILTER_MIRROR_V         (2 << 16)
#define GXFILTER_MIRROR_W         (4 << 16)
#define GXFILTER_MIRROR           (7 << 16)

#define GXFILTER_DITHER           (1 << 19)
#define GXFILTER_DITHER_DIFFUSION (2 << 19)

#define GXFILTER_SRGB_IN          (1 << 21)
#define GXFILTER_SRGB_OUT         (2 << 21)
#define GXFILTER_SRGB             (3 << 21)

enum GXFillMode {
  GXFILL_POINT               = 1,
  GXFILL_WIREFRAME           = 2,
  GXFILL_SOLID               = 3,
  GXFILL_FORCE_DWORD         = 0x7fffffff, /* force 32-bit size enum */
};

// Blend
enum GXBlend {
  GXBLEND_ZERO               = 1,
  GXBLEND_ONE                = 2,
  GXBLEND_SRCCOLOR           = 3,
  GXBLEND_INVSRCCOLOR        = 4,
  GXBLEND_SRCALPHA           = 5,
  GXBLEND_INVSRCALPHA        = 6,
  GXBLEND_DESTALPHA          = 7,
  GXBLEND_INVDESTALPHA       = 8,
  GXBLEND_DESTCOLOR          = 9,
  GXBLEND_INVDESTCOLOR       = 10,
  GXBLEND_SRCALPHASAT        = 11,
  GXBLEND_BOTHSRCALPHA       = 12,
  GXBLEND_BOTHINVSRCALPHA    = 13,
  GXBLEND_BLENDFACTOR        = 14, /* Only supported if GXPBLENDCAPS_BLENDFACTOR is on */
  GXBLEND_INVBLENDFACTOR     = 15, /* Only supported if GXPBLENDCAPS_BLENDFACTOR is on */
  GXBLEND_SRCCOLOR2          = 16,
  GXBLEND_INVSRCCOLOR2       = 17,
  GXBLEND_FORCE_DWORD        = 0x7fffffff, /* force 32-bit size enum */
};

// Blend Op
enum GXBlendOp {
  GXBLENDOP_ADD              = 1,
  GXBLENDOP_SUBTRACT         = 2,
  GXBLENDOP_REVSUBTRACT      = 3,
  GXBLENDOP_MIN              = 4,
  GXBLENDOP_MAX              = 5,
  GXBLENDOP_FORCE_DWORD      = 0x7fffffff, /* force 32-bit size enum */
};

enum GXStencilOp {
  GXSTENCILOP_KEEP           = 1,
  GXSTENCILOP_ZERO           = 2,
  GXSTENCILOP_REPLACE        = 3,
  GXSTENCILOP_INCRSAT        = 4,
  GXSTENCILOP_DECRSAT        = 5,
  GXSTENCILOP_INVERT         = 6,
  GXSTENCILOP_INCR           = 7,
  GXSTENCILOP_DECR           = 8,
  GXSTENCILOP_FORCE_DWORD    = 0x7fffffff, /* force 32-bit size enum */
};

enum GXTextureAddress {
  GXTADDRESS_WRAP            = 1,
  GXTADDRESS_MIRROR          = 2,
  GXTADDRESS_CLAMP           = 3,
  GXTADDRESS_BORDER          = 4,
  GXTADDRESS_MIRRORONCE      = 5,
  GXTADDRESS_FORCE_DWORD     = 0x7fffffff, /* force 32-bit size enum */
};

enum GXCullMode {
  GXCULL_NONE                = 1,
  GXCULL_CW                  = 2,
  GXCULL_CCW                 = 3,
  GXCULL_FORCE_DWORD         = 0x7fffffff, /* force 32-bit size enum */
};

enum GXCompareFunc{
  GXCMP_NEVER                = 1,
  GXCMP_LESS                 = 2,
  GXCMP_EQUAL                = 3,
  GXCMP_LESSEQUAL            = 4,
  GXCMP_GREATER              = 5,
  GXCMP_NOTEQUAL             = 6,
  GXCMP_GREATEREQUAL         = 7,
  GXCMP_ALWAYS               = 8,
  GXCMP_FORCE_DWORD          = 0x7fffffff, /* force 32-bit size enum */
};

enum GXPrimitiveType
{
  GXPT_POINTLIST             = 1,
  GXPT_LINELIST              = 2,
  GXPT_LINESTRIP             = 3,
  GXPT_TRIANGLELIST          = 4,
  GXPT_TRIANGLESTRIP         = 5,
  GXPT_TRIANGLEFAN           = 6,
};


enum GXTextureFilterType
{
  GXTEXFILTER_NONE            = 0,    // filtering disabled (valid for mip filter only)
  GXTEXFILTER_POINT           = 1,    // nearest
  GXTEXFILTER_LINEAR          = 2,    // linear interpolation
  GXTEXFILTER_ANISOTROPIC     = 3,    // anisotropic
  GXTEXFILTER_PYRAMIDALQUAD   = 6,    // 4-sample tent
  GXTEXFILTER_GAUSSIANQUAD    = 7,    // 4-sample gaussian
  GXTEXFILTER_FORCE_DWORD     = 0x7fffffff,   // force 32-bit size enum
};

#define GX_DEFAULT         0
#define GX_DEFAULT_NONPOW2 ((GXUINT)-1)
#define GX_FROM_FILE       ((GXUINT)-3)

#define GX_MAX_TEXTURE_STAGE    8

// 纹理尺寸比率,与屏幕相比
enum GXTextureRatio
{
  TEXSIZE_FIXEDPOINT  = 2048,
  TEXSIZE_OKTA        = -(TEXSIZE_FIXEDPOINT >> 3),    // 1/8 屏幕, 长(宽)
  TEXSIZE_QUARTER     = -(TEXSIZE_FIXEDPOINT >> 2),    // 1/4 屏幕, 长(宽)
  TEXSIZE_HALF        = -(TEXSIZE_FIXEDPOINT >> 1),    // 1/2 屏幕, 长(宽)
  TEXSIZE_SAME        = -TEXSIZE_FIXEDPOINT,        // 一倍 屏幕, 长(宽)
  TEXSIZE_DOUBLE      = -(TEXSIZE_FIXEDPOINT << 1),    // 两 屏幕, 长(宽)
  TEXSIZE_QUAD        = -(TEXSIZE_FIXEDPOINT << 2),    // 四倍 屏幕, 长(宽)
  TEXSIZE_OCTUPLE     = -(TEXSIZE_FIXEDPOINT << 3),    // 八倍 屏幕, 长(宽)

  FORCE_DWORD = 0x7fffffff,
};

// TextureRatioToDimension() 参数
#define TEXTURERATIO_POW2 0x00000001


//
// Graphics::Clear
//
#define GXCLEAR_TARGET  0x00000001L
#define GXCLEAR_DEPTH   0x00000002L
#define GXCLEAR_STENCIL 0x00000004L

//
// GXGraphics::ScrollTexture
//
struct SCROLLTEXTUREDESC
{
  GTexture* pOperationTex;  // 需要滚动的纹理
  GTexture* pTempTex;       // 临时纹理
  int       dx, dy;         // 偏移量
  LPGXCRECT lprcScroll;     // 滚动区域
  GRegion*  lprgnClip;      // 裁剪区
  GRegion** lpprgnUpdate;   // [输出]更新的区域
  LPGXRECT  lprcUpdate;     // [输出]更新的区域
};

//
// GXGraphics::GetDesc
//
struct GXGRAPHICSDEVICE_DESC
{
  size_t             cbSize;
  GXUINT             BackBufferWidth;
  GXUINT             BackBufferHeight;
  GXUINT             BackBufferCount;
  GXFormat           BackBufferFormat;
  GXFormat           DepthStencilFormat;
  GXUINT             RefreshRateInHz;  // FullScreen
  GXDWORD            dwFlags;
};

struct RENDER_STATISTICS
{
  GXLONG nShaderSwitch;
  GXLONG nDrawCount;
  GXLONG nTriangles;
  GXLONG nVertices;
};

enum GXGrapCapsCategory
{
  GXGRAPCAPS_TEXTURE,
};

#define GXTEXTURECAPS_NONPOW2 0x00000001


//
// Render State
//
struct GXRENDERSTATE
{
  GXRenderStateType   dwType;
  union{
    GXDWORD             dwValue;
    GXFLOAT             fValue;
  };
};

typedef GXRENDERSTATE* LPGXRENDERSTATE;
typedef GXRENDERSTATE* GXLPRENDERSTATE;
typedef const GXRENDERSTATE* GXLPCRENDERSTATE;

#define BEGIN_RENDERSTATE_BLOCK(STATENAME)  GXRENDERSTATE STATENAME[] = {
#define RENDERSTATE_BLOCK(RENDERSTATE, VAL)  {RENDERSTATE, VAL},
#define END_RENDERSTATE_BLOCK        {(GXRenderStateType)0,0}};

//
// Sampler State
//
#define BEGIN_SAMPLERSTATE_BLOCK(STATENAME)    GXSAMPSTATE STATENAME[] = {
#define SAMPLERSTATE_BLOCK(SAMPLERSTATE, VAL)  {SAMPLERSTATE, VAL},
#define END_SAMPLERSTATE_BLOCK          {(GXSamplerStateType)0,0}};

//
// GXResUsage flags
// 资源访问限定
//
#define GXRU_DEFAULT            0x0000   // 普通方式, 只在创建时写入
#define GXRU_MIGHTBEREAD        0x0001   // 偶尔会读, 低性能, 低内存占用, 只关心能够取得结果
#define GXRU_MIGHTBEWRITE       0x0002   // 偶尔会写, 性能内存同上
#define GXRU_FREQUENTLYREAD     0x0004   // 频繁读, 高性能, 高内存占用
#define GXRU_FREQUENTLYWRITE    0x0008   // 频繁写, 性能内存同上
#define GXRU_SYSTEMMEM          0x0010   // 创建在系统内存中,不能用于渲染

// 这几个不能同时用
#define GXRU_TEX_MASK           0x0f00
#define GXRU_TEX_RENDERTARGET   0x0100   // 渲染纹理, 这个标志只在创建纹理时有效
#define GXRU_TEX_DEPTHSTENCIL   0x0200   // 渲染用的深度+模板纹理, 这个标志只在创建纹理时有效

enum MtlLoadType
{
  MLT_REFERENCE = 0,
  MLT_CLONE = 1,
};
//
// GShader 拥有的属性配置 flags
//
// TODO[9]: 实现
// TODO[9]: _DECL_NORMAL
// TODO[9]: _DECL_VERTEXCOLOR
// TODO[9]: _DECL_TEXCOORD
// shader创建时会引用一个顶点声明的参考,用来确定顶点声明中是否使用了
// 法线,顶点颜色和纹理采样这些影响最终颜色的顶点元素,最终生成一个对应的宏
// 用来在Shader中识别.这样就避免了Shader使用了空的顶点元素而使最终颜色计算不正确的问题
enum GXShaderCapability // TODO: 改名为 trait
{
  GXSHADERCAP_VERTEX    = 0x0001,
  GXSHADERCAP_PIXEL     = 0x0002,
  GXSHADERCAP_NORMAL    = 0x0004,   // shader 支持法线相关的计算
  GXSHADERCAP_VCOLOR    = 0x0008,   // shader 支持顶点色相关的计算
  GXSHADERCAP_TEXCOORD0 = 0x0010,   // shader 支持第0层纹理坐标相关的计算, 主要是采样
  GXSHADERCAP_TEXCOORD1 = 0x0020,   // shader 支持第1层纹理
  GXSHADERCAP_TEXCOORD2 = 0x0040,   // shader 支持第2层纹理
  GXSHADERCAP_TEXCOORD3 = 0x0080,   // shader 支持第3层纹理

  // 暂时不想支持这两个,会增加编译复杂度,如果要实现单独分shader
  // 而且我也不认为这两个是语义的基本类型
  //GXSHADERCAP_TANGENT   = 0x0100,   // 
  //GXSHADERCAP_BINORMAL  = 0x0200,   // 
};

enum GXDeclUsage
{
  GXDECLUSAGE_POSITION     = 0,
  GXDECLUSAGE_BLENDWEIGHT  = 1,
  GXDECLUSAGE_BLENDINDICES = 2,
  GXDECLUSAGE_NORMAL       = 3,
  //GXDECLUSAGE_PSIZE      = 4,
  GXDECLUSAGE_TEXCOORD     = 5,
  GXDECLUSAGE_TANGENT      = 6,
  GXDECLUSAGE_BINORMAL     = 7,
  //GXDECLUSAGE_TESSFACTOR   = 8,
  GXDECLUSAGE_POSITIONT    = 9,
  GXDECLUSAGE_COLOR        = 10,
  //GXDECLUSAGE_FOG          = 11,
  //GXDECLUSAGE_DEPTH        = 12,
  //GXDECLUSAGE_SAMPLE       = 13,
};

// Vertex Format 的掩码
#define GXVF_POSITION     (1 << GXDECLUSAGE_POSITION)
#define GXVF_BLENDWEIGHT  (1 << GXDECLUSAGE_BLENDWEIGHT)
#define GXVF_BLENDINDICES (1 << GXDECLUSAGE_BLENDINDICES)
#define GXVF_NORMAL       (1 << GXDECLUSAGE_NORMAL)
#define GXVF_TEXCOORD     (1 << GXDECLUSAGE_TEXCOORD)
#define GXVF_TANGENT      (1 << GXDECLUSAGE_TANGENT)
#define GXVF_BINORMAL     (1 << GXDECLUSAGE_BINORMAL)
#define GXVF_POSITIONT    (1 << GXDECLUSAGE_POSITIONT)
#define GXVF_COLOR        (1 << GXDECLUSAGE_COLOR)

enum GXDeclMethod
{
  GXDECLMETHOD_DEFAULT = 0,
  //GXDECLMETHOD_PARTIALU,
  //GXDECLMETHOD_PARTIALV,
  //GXDECLMETHOD_CROSSUV,              // Normal
  //GXDECLMETHOD_UV,
  //GXDECLMETHOD_LOOKUP,               // Lookup a displacement map
  //GXDECLMETHOD_LOOKUPPRESAMPLED,     // Lookup a pre-sampled displacement map
};

// Declarations for _Type fields
//
enum GXDeclType
{
  GXDECLTYPE_FLOAT1    =  0,  // 1D float expanded to (value, 0., 0., 1.)
  GXDECLTYPE_FLOAT2    =  1,  // 2D float expanded to (value, value, 0., 1.)
  GXDECLTYPE_FLOAT3    =  2,  // 3D float expanded to (value, value, value, 1.)
  GXDECLTYPE_FLOAT4    =  3,  // 4D float
  GXDECLTYPE_D3DCOLOR  =  4,  // 4D packed unsigned bytes mapped to 0. to 1. range
  // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
  GXDECLTYPE_UBYTE4    =  5,  // 4D unsigned byte
  GXDECLTYPE_SHORT2    =  6,  // 2D signed short expanded to (value, value, 0., 1.)
  GXDECLTYPE_SHORT4    =  7,  // 4D signed short

  // The following types are valid only with vertex shaders >= 2.0


  GXDECLTYPE_UBYTE4N   =  8,  // Each of 4 bytes is normalized by dividing to 255.0
  GXDECLTYPE_SHORT2N   =  9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
  GXDECLTYPE_SHORT4N   = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
  GXDECLTYPE_USHORT2N  = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
  GXDECLTYPE_USHORT4N  = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
  GXDECLTYPE_UDEC3     = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
  GXDECLTYPE_DEC3N     = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
  GXDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
  GXDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values
  GXDECLTYPE_UNUSED    = 17,  // When the type field in a decl is unused.
};

enum RGNCOMPLEX  // Region's Complexity
{
  RC_ERROR    = 0,
  RC_NULL     = 1,
  RC_SIMPLE   = 2,
  RC_COMPLEX  = 3,
};

struct GXVERTEXELEMENT
{
  GXUINT        Offset;     // Offset in the stream in bytes
  GXDeclType    Type;       // Data type
  GXDeclMethod  Method;     // Processing method
  GXDeclUsage   Usage;      // Semantics
  GXUINT        UsageIndex; // Semantic index
};

// TODO: 各种type应该能合并成统一的定义
enum GXUniformType{
  GXUB_UNDEFINED,
  GXUB_FLOAT,
  GXUB_FLOAT2,
  GXUB_FLOAT3,
  GXUB_FLOAT4,
  GXUB_MATRIX4,
  GXUB_SAMPLER2D,
  GXUB_SAMPLER3D,
};

// TODO: 以后这个结构不直接用做绑定, 要用它创建一个对象再绑定
struct DATALAYOUT
{
  GXLPCSTR      pName;
  GXUINT        uOffset;
  GXUniformType eType;
  GXUINT        uSize;
};
typedef clvector<DATALAYOUT> DataLayoutArray;

struct MOSHADER_ELEMENT_SOURCE // 改名为 MOSHADERCOMPONENTFILE / MOSHADERSOURCECOMPONENT
{
  GXBOOL          bExtComposer;       // External 外部引用的合成器, 如果自己带有合成器则优先使用
  clStringA       strPreVS;           // Vertex预先处理
  clStringA       strVS;              // Vertex处理
  //clStringA       strVSExtra;       // Vertex附加处理
  clStringA       strPS;              // Pixel处理
  clStringA       strVSComposer;      // Vertex合成器
  clStringA       strPSComposer;      // Pixel合成器
  //clStringArrayA  aPSComponent;     // Pixel分量
  clStringA       strMacros;
};

struct MOSHADERBUFFERS
{
  // 用户不需要关心这里面是什么,可能是源代码,也可能是编译后数据
  // 要注意 clBuffer 的内存回收.
  GXUINT cbSize;
  clBuffer* pVertexShader;
  clBuffer* pPixelShader;
  //GXDefinitionArray aMacros;
};


struct CANVASUNIFORM
{
  clBuffer*         pUnusualBuf;
  GXCANVASCOMMCONST pCommon;
};

typedef DATALAYOUT*        LPDATALAYOUT;
typedef const DATALAYOUT*  LPCDATALAYOUT;

typedef GXVERTEXELEMENT*        LPGXVERTEXELEMENT;
typedef const GXVERTEXELEMENT*  LPCGXVERTEXELEMENT;
typedef GXVERTEXELEMENT*        GXLPVERTEXELEMENT;
typedef const GXVERTEXELEMENT*  GXLPCVERTEXELEMENT;
#define GXDECL_END() {0,GXDECLTYPE_UNUSED,(GXDeclMethod)0,(GXDeclUsage)0,-1}

struct GXVIEWPORT
{
  GXRegn  regn;
  float   fNear;
  float   fFar;

  GXVIEWPORT() : regn(0), fNear(0), fFar(0)
  {
  }

  GXVIEWPORT(int x, int y, int w, int h, float fNear, float fFar)
  {
    regn.x = x;
    regn.y = y;
    regn.w = w;
    regn.h = h;
    this->fNear = fNear;
    this->fFar = fFar;
  }
  GXVIEWPORT(GXLPCREGN lpRegn, float fNear, float fFar)
  {
    regn.x = lpRegn->left;
    regn.y = lpRegn->top;
    regn.w = lpRegn->width;
    regn.h = lpRegn->height;
    this->fNear = fNear;
    this->fFar  = fFar;
  }
};
typedef GXVIEWPORT*         GXLPVIEWPORT;
typedef const GXVIEWPORT*   GXLPCVIEWPORT;

struct GXBLENDDESC
{
  GXBOOL    BlendEnable;
  GXBlend   SrcBlend;
  GXBlend   DestBlend;
  GXBlendOp BlendOp;
  GXBOOL    SeparateAlphaBlend;
  GXBlend   SrcBlendAlpha;
  GXBlend   DestBlendAlpha;
  GXBlendOp BlendOpAlpha;
  GXUINT8   WriteMask;

  GXBLENDDESC()  // 默认参数
    : BlendEnable   (FALSE)
    , SrcBlend      (GXBLEND_ONE)
    , DestBlend     (GXBLEND_ZERO)
    , BlendOp       (GXBLENDOP_ADD)
    , SeparateAlphaBlend(FALSE)
    , SrcBlendAlpha (GXBLEND_ONE)
    , DestBlendAlpha(GXBLEND_ZERO)
    , BlendOpAlpha  (GXBLENDOP_ADD)
    , WriteMask     (0xf)
  {}

  GXBLENDDESC(
    GXBOOL bBlend,
    GXBlend eSrcBlend = GXBLEND_SRCALPHA,
    GXBlend eDestBlend = GXBLEND_INVSRCALPHA,
    GXBlendOp eBlendOp = GXBLENDOP_ADD)
    : BlendEnable   (bBlend)
    , SrcBlend      (eSrcBlend)
    , DestBlend     (eDestBlend)
    , BlendOp       (eBlendOp)
    , SeparateAlphaBlend(FALSE)
    , SrcBlendAlpha (GXBLEND_SRCALPHA)
    , DestBlendAlpha(GXBLEND_INVSRCALPHA)
    , BlendOpAlpha  (GXBLENDOP_ADD)
    , WriteMask     (0xf)
  {}  
};

struct GXDEPTHSTENCILOP
{
  GXStencilOp   StencilFailOp;
  GXStencilOp   StencilDepthFailOp;
  GXStencilOp   StencilPassOp;
  GXCompareFunc StencilFunc;

  GXDEPTHSTENCILOP()
    : StencilFailOp     (GXSTENCILOP_KEEP)
    , StencilDepthFailOp(GXSTENCILOP_KEEP)
    , StencilPassOp     (GXSTENCILOP_KEEP)
    , StencilFunc       (GXCMP_ALWAYS)
  {}
};

struct GXDEPTHSTENCILDESC
{
  GXBOOL            DepthEnable;
  GXDWORD           DepthWriteMask; //Depth Write Enable
  GXCompareFunc     DepthFunc;
  GXBOOL            StencilEnable;
  GXUINT8           StencilReadMask;
  GXUINT8           StencilWriteMask;
  GXDEPTHSTENCILOP  FrontFace;
  GXDEPTHSTENCILOP  BackFace;

  GXDEPTHSTENCILDESC(GXBOOL bDepth = FALSE, GXBOOL bStencil = FALSE)
    : DepthEnable     (bDepth)
    , DepthWriteMask  (bDepth)
    , DepthFunc       (bDepth ? GXCMP_LESSEQUAL : GXCMP_NEVER)
    , StencilEnable   (bStencil)
    , StencilReadMask (bStencil ? 0xff : 0)
    , StencilWriteMask(bStencil ? 0xff : 0)
  {}
};

struct GXRASTERIZERDESC
{
  GXUINT      cbSize;
  GXFillMode  FillMode;
  GXCullMode  CullMode;
  GXBOOL      FrontCounterClockwise;
  GXFLOAT     DepthBias;
  GXFLOAT     DepthBiasClamp;
  GXFLOAT     SlopeScaledDepthBias;
  GXBOOL      DepthClipEnable;
  GXBOOL      ScissorEnable;

  GXRASTERIZERDESC()
    : cbSize                (sizeof(GXRASTERIZERDESC))
    , FillMode              (GXFILL_SOLID)
    , CullMode              (GXCULL_CCW)
    , FrontCounterClockwise (FALSE)
    , DepthBias             (0.0f)
    , DepthBiasClamp        (0.0f)
    , SlopeScaledDepthBias  (0.0f)
    , DepthClipEnable       (TRUE)
    , ScissorEnable         (FALSE)
  {
  }
};

struct GXSAMPLERDESC {
  GXTextureAddress      AddressU;
  GXTextureAddress      AddressV;
  GXTextureAddress      AddressW;

  GXColor               BorderColor;

  GXTextureFilterType   MagFilter;
  GXTextureFilterType   MinFilter;
  GXTextureFilterType   MipFilter;

  GXSAMPLERDESC()
    : AddressU    (GXTADDRESS_WRAP)
    , AddressV    (GXTADDRESS_WRAP)
    , AddressW    (GXTADDRESS_WRAP)
    , BorderColor (0)
    , MagFilter   (GXTEXFILTER_POINT)
    , MinFilter   (GXTEXFILTER_POINT)
    , MipFilter   (GXTEXFILTER_POINT)
  {
  }
};

struct STOCKOBJECT
{
  GShader*  pAeroShader;
  GShader*  pBlurShader;
  GShader*  pSimpleShader;
  GXEffect* pAeroEffect;
  GXEffect* pBlurEffect;
  GXEffect* pSimpleEffect;
  GXFont*   pDefaultFont;
};
typedef STOCKOBJECT* LPSTOCKOBJECT;

//////////////////////////////////////////////////////////////////////////
//
// Property Sheet Defines
//
#define PSM_SETDATALAYOUT   (GXWM_USER + 101)
#define PSM_UPLOADDATA      (GXWM_USER + 102)
#define PSM_DOWNLOADDATA    (GXWM_USER + 103)

enum PROPERTYSHEETTYPE
{
  PST_UNKNOWN,
  PST_BOOLEAN,
  PST_INTEGER,
  PST_UINTEGER,
  PST_FLOAT,
  PST_STRING,
  PST_DESCRIBE,
  PST_LIST,
  PST_PROPSHEETPAGE,
  PST_COLOR,
  PST_IMAGEPATHW,
  PST_IMAGEPATHA,
  PST_BUTTON,
  PST_DIALOG,

#ifdef _UNICODE
  PST_IMAGEPATH = PST_IMAGEPATHW,
#else
  PST_IMAGEPATH = PST_IMAGEPATHA,
#endif // #ifdef _UNICODE
};

enum PROPERTYLISTTYPE
{
  PLT_UNKNOWN,
  PLT_BOOLEAN,
  PLT_INTEGER,
  PLT_UINTEGER,
  PLT_FLOAT,
  PLT_STRING,
  PLT_DESCRIBE,
  PLT_LIST,
  PLT_PROPLISTPAGE,
  PLT_COLOR,
  PLT_IMAGEPATHW,
  PLT_IMAGEPATHA,
  PLT_BUTTON,
  PLT_DIALOG,

#ifdef _UNICODE
  PLT_IMAGEPATH = PLT_IMAGEPATHW,
#else
  PLT_IMAGEPATH = PLT_IMAGEPATHA,
#endif // #ifdef _UNICODE
};



#define PROPSHEET_DATALINK_BEGIN(NAME)                              PROPSHEET_DATALINK NAME[] = {
#define PROPSHEET_DATALINK_END                                      {0,0,0,0,0,0},}
#define PROPSHEET_TEXT(_TEXT)                                       {_TEXT, -1, PST_DESCRIBE, 0, 0, 0}
#define PROPSHEET_STRING(_TEXT, ID, BASEADDRIDX, OFFSET)            {_TEXT, ID, PST_STRING, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPSHEET_COLOR(_TEXT, ID, BASEADDRIDX, OFFSET)             {_TEXT, ID, PST_COLOR, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPSHEET_LIST(_TEXT, ID, LIST, BASEADDRIDX, OFFSET)        {_TEXT, ID, PST_LIST, BASEADDRIDX, (size_t)OFFSET, LIST}
#define PROPSHEET_BOOL(_TEXT, ID, BASEADDRIDX, OFFSET)              {_TEXT, ID, PST_BOOLEAN, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPSHEET_INT(_TEXT, ID, BASEADDRIDX, OFFSET)               {_TEXT, ID, PST_INTEGER, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPSHEET_UINT(_TEXT, ID, BASEADDRIDX, OFFSET)              {_TEXT, ID, PST_UINTEGER, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPSHEET_IMAGEPATHW(_TEXT, ID, BASEADDRIDX, OFFSET)        {_TEXT, ID, PST_IMAGEPATHW, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPSHEET_IMAGEPATHA(_TEXT, ID, BASEADDRIDX, OFFSET)        {_TEXT, ID, PST_IMAGEPATHA, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPSHEET_FLOAT(_TEXT, ID, BASEADDRIDX, OFFSET, INCREASE)   {_TEXT, ID, PST_FLOAT, BASEADDRIDX, (size_t)OFFSET, 0, INCREASE}
#define PROPSHEET_PAGE(_TEXT, ID, PAGE)                             {_TEXT, ID, PST_PROPSHEETPAGE, 0, 0, PAGE}
#define PROPSHEET_DIALOG(TEMPLATE_ID, ID, DLG_PROC, DLG_PARAM)      {"",    ID, PST_DIALOG, TEMPLATE_ID, (size_t)DLG_PROC, DLG_PARAM}

#ifdef _UNICODE
#define PROPSHEET_IMAGEPATH(_TEXT, ID, BASEADDRIDX, OFFSET)         {_TEXT, ID, PST_IMAGEPATHW, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#else
#define PROPSHEET_IMAGEPATH(_TEXT, ID, BASEADDRIDX, OFFSET)         {_TEXT, ID, PST_IMAGEPATHA, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#endif // #ifdef _UNICODE

struct PROPSHEET_LISTBOX
{
  GXLPCSTR    lpName;
  GXLPARAM    lParam;
};

struct PROPSHEET_DATALINK
{
  GXLPCSTR   lpName;      // 标签名
  GXDWORD    dwId;
  GXDWORD    eType;       // 类型
  union{
    struct
    {
      GXINT      nBasePtrIdx;     // 基值的索引值
      size_t     pDestOffsetPtr;  // 偏移量
      void*      pValue;          // 值, LIST作为列表地址, PropSheetPage作为下一个Page的地址
      float      fIncrease;       // 作为FLOAT时,单击上下按钮的增量
    };
    struct
    {
      GXLPCWSTR  lpTemplate;
      GXDLGPROC  lpDlgProc;
      GXLPARAM   dwInitParam;
      GXDWORD    dwReserved;      // 保留
    };
  };
};

#define PROPLIST_DATALINK_BEGIN(NAME)                              PROPLIST_DATALINK NAME[] = {
#define PROPLIST_DATALINK_END                                      {0,0,0,0,0,0},}
#define PROPLIST_TEXT(_TEXT)                                       {_TEXT, -1, PLT_DESCRIBE, 0, 0, 0}
#define PROPLIST_STRING(_TEXT, ID, BASEADDRIDX, OFFSET)            {_TEXT, ID, PLT_STRING, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPLIST_COLOR(_TEXT, ID, BASEADDRIDX, OFFSET)             {_TEXT, ID, PLT_COLOR, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPLIST_LIST(_TEXT, ID, LIST, BASEADDRIDX, OFFSET)        {_TEXT, ID, PLT_LIST, BASEADDRIDX, (size_t)OFFSET, LIST}
#define PROPLIST_BOOL(_TEXT, ID, BASEADDRIDX, OFFSET)              {_TEXT, ID, PLT_BOOLEAN, BASEADDRIDX, (size_t)OFFSET, 0}
#define PROPLIST_INT(_TEXT, ID, BASEADDRIDX, OFFSET)               {_TEXT, ID, PLT_INTEGER, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPLIST_UINT(_TEXT, ID, BASEADDRIDX, OFFSET)              {_TEXT, ID, PLT_UINTEGER, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPLIST_IMAGEPATHW(_TEXT, ID, BASEADDRIDX, OFFSET)        {_TEXT, ID, PLT_IMAGEPATHW, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPLIST_IMAGEPATHA(_TEXT, ID, BASEADDRIDX, OFFSET)        {_TEXT, ID, PLT_IMAGEPATHA, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#define PROPLIST_FLOAT(_TEXT, ID, BASEADDRIDX, OFFSET, INCREASE)   {_TEXT, ID, PLT_FLOAT, BASEADDRIDX, (size_t)OFFSET, 0, INCREASE}
#define PROPLIST_PAGE(_TEXT, ID, PAGE)                             {_TEXT, ID, PLT_PROPLISTPAGE, 0, 0, PAGE}
#define PROPLIST_DIALOG(TEMPLATE_ID, ID, DLG_PROC, DLG_PARAM)      {"",    ID, PLT_DIALOG, TEMPLATE_ID, (size_t)DLG_PROC, DLG_PARAM}

#ifdef _UNICODE
#define PROPLIST_IMAGEPATH(_TEXT, ID, BASEADDRIDX, OFFSET)         {_TEXT, ID, PST_IMAGEPATHW, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#else
#define PROPLIST_IMAGEPATH(_TEXT, ID, BASEADDRIDX, OFFSET)         {_TEXT, ID, PST_IMAGEPATHA, BASEADDRIDX, (size_t)OFFSET, 0, 0}
#endif // #ifdef _UNICODE

struct PROPLIST_LISTBOX
{
  GXLPCSTR    lpName;
  GXLPARAM    lParam;
};

struct PROPLIST_DATALINK
{
  GXLPCSTR   lpName;      // 标签名
  GXDWORD    dwId;
  GXDWORD    eType;       // 类型
  union{
    struct
    {
      GXINT      nBasePtrIdx;     // 基值的索引值
      size_t     pDestOffsetPtr;  // 偏移量
      void*      pValue;          // 值, LIST作为列表地址, PropSheetPage作为下一个Page的地址
      float      fIncrease;       // 作为FLOAT时,单击上下按钮的增量
    };
    struct
    {
      GXLPCWSTR  lpTemplate;
      GXDLGPROC  lpDlgProc;
      GXLPARAM   dwInitParam;
      GXDWORD    dwReserved;      // 保留
    };
  };
};

struct PROPLIST_DATALAYOUT
{
  const GXLPVOID* pBasePtrList;
  const PROPLIST_DATALINK* pDataDesc;

  // 更新数据时才能用到
  GXINT nIDFirst;
  GXINT nIDLast;
};

// PropertySheet WM_NOTIFY 消息结构
struct NM_PROPLISTW
{
  GXNMHDR       nmhdr;

  PROPERTYLISTTYPE eType;
  GXDWORD       dwStyle;
  GXDWORD       dwId;
  GXUINT        nHeight;
  clStringW     strName;
  union
  {
    GXBOOL      bVal;
    GXINT       nVal;
    float       fVal;
    GXCOLORREF  crVal;
    GXHWND      hDlg;
  };
  clStringW     strVal;

  //union
  //{
  //  Page*         pChildPage;
  //  LBItemList*   pListBoxItem;
  //  GXLPVOID      pGeneral;    // 通用的,只用于指针判断
  float         fIncrease;  // 如果是浮点数的话
  //};
};


#define OFFSET_MEMBER(CLS, MEMB)  ((size_t)&(((CLS*)0)->MEMB))
#define cloffsetof(CLS, MEMB)  OFFSET_MEMBER(CLS, MEMB)

struct PROPSHEET_DATALAYOUT
{
  const GXLPVOID* pBasePtrList;
  const PROPSHEET_DATALINK* pDataDesc;

  // 更新数据时才能用到
  GXINT nIDFirst;
  GXINT nIDLast;
};

// PropertySheet WM_NOTIFY 消息结构
struct NM_PROPSHEETW
{
  GXNMHDR       nmhdr;

  PROPERTYSHEETTYPE eType;
  GXDWORD       dwStyle;
  GXDWORD       dwId;
  GXUINT        nHeight;
  clStringW     strName;
  union
  {
    GXBOOL      bVal;
    GXINT       nVal;
    float       fVal;
    GXCOLORREF  crVal;
    GXHWND      hDlg;
  };
  clStringW     strVal;

  //union
  //{
  //  Page*         pChildPage;
  //  LBItemList*   pListBoxItem;
  //  GXLPVOID      pGeneral;    // 通用的,只用于指针判断
  float         fIncrease;  // 如果是浮点数的话
  //};
};

//////////////////////////////////////////////////////////////////////////

#define gxRtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define gxRtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define gxRtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define gxRtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define gxRtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

//template<class _Ty>
//inline void InlSetZeroT(_Ty& t)
//{
//  memset(&t, 0, sizeof(t));
//}

template<class _TIUnknown>
inline GXBOOL InlCheckNewAndIncReference(_TIUnknown* pIUnknown) // TODO: 准备废掉这个，用InlIsFailedToNewObj代替，注意返回值相反
{
  if(pIUnknown == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return FALSE;
  }
  pIUnknown->AddRef();
  return TRUE;
}

template<class _TIUnknown>
inline GXBOOL InlIsFailedToNewObject(_TIUnknown* pIUnknown)
{
  if(pIUnknown == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return TRUE;
  }

  pIUnknown->AddRef();
  return FALSE;
}

template<class _TObj>
inline GXHRESULT InlSetNewObjectT(_TObj*& pObj, _TObj* pNew) // pNew 可以为空
{
  SAFE_RELEASE(pObj);
  pObj = pNew;
  if(pObj != NULL) {
    return pObj->AddRef();
  }
  return GX_OK;
}

template<class _TObj>
inline GXHRESULT InlSetNewObjectAlwaysT(_TObj*& pObj, _TObj* pNew) // 用户保证 pNew 是不为空的
{
  SAFE_RELEASE(pObj);
  pObj = pNew;
  return pObj->AddRef();
}

template<class _TInterface>
inline GXHRESULT InlGetSafeObjectT(_TInterface** ppInterface, _TInterface* pHolding)
{
  if(pHolding == NULL)
    return GX_FAIL;
  *ppInterface = pHolding;
  return pHolding->AddRef();
}


// TODO: 是不是以后要加上"Gph"前缀呢?
GXDLL GXINT         GetAdaptedSize        (GXINT nSize);
GXDLL GXUINT        GetBytesOfGraphicsFormat  (GXFormat eFmt);
GXDLL GXFormatCategory  GetGraphicsFormatCategory  (GXFormat eFmt);
GXDLL GXBOOL        IsPow2            (GXINT nNum);
GXDLL GXUINT        TextureRatioToDimension    (GXINT nTexRatio, GXUINT nModel, GXDWORD dwFlags);

// 内置的顶点格式定义
enum GXSysVertexDecl
{
  GXVD_P3F_C1D,
  GXVD_P3T2C4F,
  GXVD_P4T2C4F,
  GXVD_P3T2F_C1D,
  GXVD_P4T2F_C1D,
  GXVD_P3T2N3F_C1D,
  GXVD_P3T2N3F,   // GXVERTDECL_P3T2N3F  GXVERTEX_P3T2N3F
};

GXLPCVERTEXELEMENT GXDLLAPI MOGetSysVertexDecl     (GXSysVertexDecl eDecl);
GXDWORD GXDLLAPI MOGenerateVertexDeclCode (LPCGXVERTEXELEMENT pVertDecl);
GXVOID  GXDLLAPI MOCanonicalizeVertexDecl (LPGXVERTEXELEMENT pNewVertDecl, LPCGXVERTEXELEMENT pSrcVertDecl, GXBOOL bRelocalOffset);  // 新的 Offset 会改变
GXVOID  GXDLLAPI MORelocateVertexDecl     (LPGXVERTEXELEMENT pVertDecl);
GXUINT  GXDLLAPI MOGetDeclTypeSize        (GXDeclType eType); // 获得DeclType表达的字节数
GXUINT  GXDLLAPI MOGetDeclTypeDimension   (GXDeclType eType); // 获得维度数, float4就是4
GXUINT  GXDLLAPI MOGetDeclVertexSize      (LPCGXVERTEXELEMENT pVertDecl); // 从声明中获得顶点的长度
GXUINT  GXDLLAPI MOGetDeclCount           (LPCGXVERTEXELEMENT pVertDecl);
GXDWORD GXDLLAPI MOTestDeclElementFlags   (LPCGXVERTEXELEMENT pVertDecl); // 根据顶点声明测试Shader输入元素
void    GXDLLAPI MOTestShaderCapsString   (LPCGXVERTEXELEMENT pVertDecl, clStringA& strMacro);  // 根据顶点声明测试Shader输入元素,并生成Shader编译宏
void    GXDLLAPI MOMakeShaderCapsString   (GXDWORD dwCapsFlags, clStringA& strMacro);  // 根据标志生成Shader编译宏
GXINT   GXDLLAPI MOGetDeclOffset          (GXIN LPCGXVERTEXELEMENT pVertDecl, GXIN GXDeclUsage Usage, GXIN GXUINT UsageIndex, GXOUT LPGXVERTEXELEMENT lpDesc = NULL); // 获得指定的偏移
int     GXDLLAPI MOParseRenderQueue       (GXLPCSTR szName);

GXHRESULT GXDLLAPI MOCompileHLSL          (GXLPCWSTR szShaderDesc, GXLPCWSTR szResourceDir, GXLPCSTR szPlatformSect, MOSHADERBUFFERS* pBuffers);
#define MOGetDeclVertexStride MOGetDeclVertexSize

// TODO: 增加一个直接从Primitive转换的接口
GXBOOL GXDLLAPI MOConvertVertexFormat   (GXLPVOID lpDestVert, GXLPCVERTEXELEMENT lpDestDecl, GXLPVOID lpSrcVert, GXLPCVERTEXELEMENT lpSrcDecl, GXUINT nCount, GXDWORD dwFlags); // dwFlags 还没用上

GXLPCRENDERSTATE    GXDLLAPI MOGetDefaultRenderState(); // 基于D3D9的定义

// GetAdaptedSize
// 获得适配尺寸, 找到一个大于输入数值的2的幂的值

// GetBytesOfGraphicsFormat
// 获得像素格式所占的字节数

// GetGraphicsFormatCategory
// 获得像素格式的分类

// IsPow2
// 判断输入数值是否是2的幂数

// TextureRatioToDimension
// 纹理比例转换为实际尺寸

//////////////////////////////////////////////////////////////////////////
enum GXPlaformIdentity // TODO: 以后去掉这个, 使用FOURCC来表示
{
  GXPLATFORM_UNKNOWN          = 0,
  GXPLATFORM_WIN32_DIRECT3D9  = GXMAKEFOURCC('D','3','D','9'),
  GXPLATFORM_WIN32_DIRECT3D11 = GXMAKEFOURCC('D','X','1','1'),
  GXPLATFORM_WIN32_OPENGL     = GXMAKEFOURCC('W','O','G','L'),
  GXPLATFORM_X_OPENGLES2      = GXMAKEFOURCC('E','G','L','2'),
  //GXPLATFORM_X_OPENGLES1A,
};


// Platform id 与字符串互相转换的函数
extern "C"
{
  GXBOOL            GXDLLAPI MOPlatformEnumToStringW (GXPlaformIdentity ePlatform, GXLPWSTR szName, int nSize);
  GXPlaformIdentity GXDLLAPI MOPlatformStringToEnumW (GXLPCWSTR szName);
};

// Static Sprite Message
#define GXSSM_SETSPRITEBYFILENAMEW  (GXWM_USER + 20)  // lParam: 文件名
#define GXSSM_SETSPRITE             (GXWM_USER + 21)  // lParam: Sprite对象
#define GXSSM_SETMODULEBYNAMEW      (GXWM_USER + 22)  // lParam: Module名字
#define GXSSM_SETMODULEBYINDEX      (GXWM_USER + 23)  // wParam: 索引,统一索引（Module，Frame，Animation连续在一起）
//#define GXSSM_SETMODULEBYID         (GXWM_USER + 24)  // wParam: ID

#define GXSSM_GETSPRITEBYFILENAMEW  (GXWM_USER + 30)  // lParam: 文件名, wParam 缓冲大小
#define GXSSM_GETSPRITE             (GXWM_USER + 31)  // lParam: Sprite**对象, 用完要释放引用
#define GXSSM_GETMODULEBYNAMEW      (GXWM_USER + 32)  // lParam: Module名字, wParam 缓冲大小
#define GXSSM_GETMODULEBYINDEX      (GXWM_USER + 33)  // 返回值: 索引
//#define GXSSM_GETMODULEBYID         (GXWM_USER + 34)  // 返回值: ID


#include <GrapX/GXKernel.h>
#include <GrapX/GXUser.h>
#include <GrapX/GXGDI.h>

#include "GrapX/WineTreeViewDef.H"
#include "GrapX/WineToolTipsDef.H"
#include "GrapX/WineDragListDef.H"
#include "GrapX/WinePagerDef.H"
#include "GrapX/WineToolbarDef.H"
#include "GrapX/VertexDecl.H"

#endif // _GRAPX_DEF_HEADER_