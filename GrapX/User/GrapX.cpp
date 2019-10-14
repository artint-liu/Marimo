// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GRegion.h>

// 私有头文件
#include "clPathFile.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXUser.h"
#include "GrapX/gxError.h"
#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
#include <vld.h>
#endif // #if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma warning( disable : 4996 ) // disable deprecated warning 

using namespace clstd;

#ifdef _WINDOWS
//GXVOID _WinVerifyFailure(GXCHAR *pszSrcFile,GXINT nLine, GXDWORD dwErrorNum)
//{
//  GXCHAR  buffer[1024];
//  PCHAR  pszCaption = NULL;
//  GXLPSTR  lpBuffer;
//  FormatMessageA(
//    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//    NULL,
//    dwErrorNum,
//    LANG_NEUTRAL,
//    (GXLPSTR)&lpBuffer,
//    0,
//    NULL
//    );
//  wsprintfA(
//    buffer,
//    "Win32 API Error(Code %d):%s",
//    dwErrorNum,
//    lpBuffer        
//    );
//  OutputDebugStringA("\n====================== ");
//  OutputDebugStringA(pszCaption);
//  OutputDebugStringA(" ======================\n");
//  OutputDebugStringA(buffer);
//  MessageBoxA(NULL, buffer, pszCaption, MB_OK | MB_ICONERROR);
//}
//void _gxTrace(char *fmt, ...)
//{
//
//  char out[1024];
//  va_list body;
//  va_start(body, fmt);
//  vsprintf(out, fmt, body);
//  va_end(body);
//
//  OutputDebugStringA(out);
//}
#else
GXUINT strlenW(GXLPCWSTR str)
{
  const GXWCHAR *s = str;
  while (*s) s++;
  return s - str;
}
GXLPWSTR strcpyW(GXLPWSTR lpString1,GXLPCWSTR lpString2)
{
  GXLPWSTR lpDest = lpString1;
  GXLPWSTR lpSrc  = (GXLPWSTR)lpString2;
  while(*lpSrc){*lpDest = *lpSrc; lpDest++; lpSrc++;}
  *lpDest = 0;
  return lpString1;
}

//static int strncmpW( const GXWCHAR *str1, const GXWCHAR *str2, int n )
//{
//  if (n <= 0) return 0;
//  while ((--n > 0) && *str1 && (*str1 == *str2)) { str1++; str2++; }
//  return *str1 - *str2;
//}


#endif

#ifndef _DEV_DISABLE_UI_CODE

//////////////////////////////////////////////////////////////////////////
//
// Win32 GDI Emulator Object 管理相关函数
//
//#include <map>
//#include <vector>
typedef clmap<void*, void*>  GXBitmapResMap;
typedef clvector<void*>    GXFontResMap;
GXBOOL GXDLLAPI _gxDeleteObject(GXHGDIOBJ);
GXVOID gxCOMCTL32_RefreshSysColors();

static GXBitmapResMap  *s_pGXBitmapResMap;    // 用于储存 从应用程序内部读取的资源指针，避免多次创建对象
//static GXFontResMap    *s_pGXFontResMap;

//GXFont*  g_pSystemFont = NULL;

// GetStockObject 使用的常量
GXHFONT g_hSystemFont = NULL;  // SYSTEM_FONT

GXGDIBRUSH g_BlackBrush;    // BLACK_BRUSH
GXGDIBRUSH g_DarkGrayBrush;    // DKGRAY_BRUSH
GXGDIBRUSH g_GrayBrush;      // GRAY_BRUSH
GXGDIBRUSH g_LightGrayBrush;  // LTGRAY_BRUSH
GXGDIBRUSH g_NullBrush;      // NULL_BRUSH
GXGDIBRUSH g_WhiteBrush;    // WHITE_BRUSH
GXGDIBRUSH g_Brush_00FFFFFF;
GXGDIBRUSH g_Brush_80FFFFFF;
GXGDIPEN g_BlackPen;      // BLACK_PEN
GXGDIPEN g_WhitePen;      // WHITE_PEN
GXGDIPEN g_NullPen;        // NULL_PEN


GXCOLORREF crSysColor[31] = {
  0xFFD4D0C8,    // #define GXCOLOR_SCROLLBAR         0        
  0xFF004E98,    // #define GXCOLOR_BACKGROUND        1        
  0xFF0054E3,    // #define GXCOLOR_ACTIVECAPTION     2        
  0xFF7A96DF,    // #define GXCOLOR_INACTIVECAPTION   3        
  0x80FFFFFF,    // #define GXCOLOR_MENU              4        
  0xFFFFFFFF,    // #define GXCOLOR_WINDOW            5        
  0xFF000000,    // #define GXCOLOR_WINDOWFRAME       6        
  0xFF000000,    // #define GXCOLOR_MENUTEXT          7        
  0xFF000000,    // #define GXCOLOR_WINDOWTEXT        8        
  0xFFFFFFFF,    // #define GXCOLOR_CAPTIONTEXT       9        
  0xFFD4D0C8,    // #define GXCOLOR_ACTIVEBORDER      10       
  0xFFD4D0C8,    // #define GXCOLOR_INACTIVEBORDER    11       
  0xFF808080,    // #define GXCOLOR_APPWORKSPACE      12       
  0xFF316AC5,    // #define GXCOLOR_HIGHLIGHT         13       
  0xFFFFFFFF,    // #define GXCOLOR_HIGHLIGHTTEXT     14       
  0xFFECE9D8,    // #define GXCOLOR_BTNFACE           15       
  0xFFACA899,    // #define GXCOLOR_BTNSHADOW         16       
  0xFFACA899,    // #define GXCOLOR_GRAYTEXT          17       
  0xFF000000,    // #define GXCOLOR_BTNTEXT           18       
  0xFFD8E4F8,    // #define GXCOLOR_INACTIVECAPTIONTEXT 19     
  0xFFFFFFFF,    // #define GXCOLOR_BTNHIGHLIGHT      20       
  0xFF716F64,    // #define GXCOLOR_3DDKSHADOW        21       
  0xFFF1EFE2,    // #define GXCOLOR_3DLIGHT           22       
  0xFF000000,    // #define GXCOLOR_INFOTEXT          23       
  0xFFFFFFE1,    // #define GXCOLOR_INFOBK            24       
  0xFFB5B5B5,    // Unknown               25       
  0xFF000080,    // #define GXCOLOR_HOTLIGHT          26       
  0xFF3D95FF,    // #define GXCOLOR_GRADIENTACTIVECAPTION 27   
  0xFF9DB9EB,    // #define GXCOLOR_GRADIENTINACTIVECAPTION 28 
  0x80316AC5,    // #define GXCOLOR_MENUHILIGHT       29       
  0x80ECE9D8,    // #define GXCOLOR_MENUBAR           30       
};
GXGDIBRUSH stSysColorBrush[32];

long g_SystemMetrics[] = {
  -1/*1920*/,    //( 0) SM_CXSCREEN  屏幕宽度
  -1/*1200*/,    //( 1) SM_CYSCREEN  屏幕高度
  17,      //( 2) SM_CXVSCROLL
  17,      //( 3) SM_CYHSCROLL
  30,      //( 4) SM_CYCAPTION  标题区
  1,      //( 5) SM_CXBORDER
  1,      //( 6) SM_CYBORDER
  6,      //( 7) SM_CXDLGFRAME or SM_CXFIXEDFRAME  水平边框的高度
  6,      //( 8) SM_CYDLGFRAME or SM_CYFIXEDFRAME 竖直边框的宽度
  17,      //( 9) SM_CYVTHUMB
  17,      //(10) SM_CXHTHUMB
  32,      //(11) SM_CXICON
  32,      //(12) SM_CYICON
  32,      //(13) SM_CXCURSOR
  32,      //(14) SM_CYCURSOR
  20,      //(15) SM_CYMENU
  -1/*1920*/,    //(16) SM_CXFULLSCREEN
  -1/*1148*/,    //(17) SM_CYFULLSCREEN
  0,      //(18) SM_CYKANJIWINDOW
  1,      //(19) SM_MOUSEPRESENT
  17,      //(20) SM_CYVSCROLL
  17,      //(21) SM_CXHSCROLL
  0,      //(22) SM_DEBUG
  0,      //(23) SM_SWAPBUTTON
  0,      //(24) SM_RESERVED1
  0,      //(25) SM_RESERVED2
  0,      //(26) SM_RESERVED3
  0,      //(27) SM_RESERVED4
  119,    //(28) SM_CXMIN
  30,      //(29) SM_CYMIN
  21,      //(30) SM_CXSIZE
  21,      //(31) SM_CYSIZE
  6,      //(32) SM_CXFRAME or SM_CXSIZEFRAME 水平边框的高度
  6,      //(33) SM_CYFRAME or SM_CYSIZEFRAME 竖直边框的宽度
  119,    //(34) SM_CXMINTRACK
  30,      //(35) SM_CYMINTRACK
  4,      //(36) SM_CXDOUBLECLK
  4,      //(37) SM_CYDOUBLECLK
  75,      //(38) SM_CXICONSPACING
  75,      //(39) SM_CYICONSPACING
  0,      //(40) SM_MENUDROPALIGNMENT
  0,      //(41) SM_PENWINDOWS
  1,      //(42) SM_DBCSENABLED
  3,      //(43) SM_CMOUSEBUTTONS
  0,      //(44) SM_SECURE
  2,      //(45) SM_CXEDGE
  2,      //(46) SM_CYEDGE
  160,    //(47) SM_CXMINSPACING
  27,      //(48) SM_CYMINSPACING
  16,      //(49) SM_CXSMICON
  16,      //(50) SM_CYSMICON
  20,      //(51) SM_CYSMCAPTION
  19,      //(52) SM_CXSMSIZE
  19,      //(53) SM_CYSMSIZE
  19,      //(54) SM_CXMENUSIZE
  19,      //(55) SM_CYMENUSIZE
  8,      //(56) SM_ARRANGE
  160,    //(57) SM_CXMINIMIZED
  27,      //(58) SM_CYMINIMIZED
  -1/*1932*/,    //(59) SM_CXMAXTRACK
  -1/*1212*/,    //(60) SM_CYMAXTRACK
  -1/*1928*/,    //(61) SM_CXMAXIMIZED
  -1/*1178*/,    //(62) SM_CYMAXIMIZED
  3,      //(63) SM_NETWORK
  -1,      //     SM_NOTUSED64
  -1,      //     SM_NOTUSED65
  -1,      //     SM_NOTUSED66
  0,      //(67) SM_CLEANBOOT
  4,      //(68) SM_CXDRAG
  4,      //(69) SM_CYDRAG
  0,      //(70) SM_SHOWSOUNDS
  17,      //(71) SM_CXMENUCHECK
  17,      //(72) SM_CYMENUCHECK
  0,      //(73) SM_SLOWMACHINE
  0,      //(74) SM_MIDEASTENABLED
  1,      //(75) SM_MOUSEWHEELPRESENT
  0,      //(76) SM_XVIRTUALSCREEN
  0,      //(77) SM_YVIRTUALSCREEN
  -1/*1920*/,    //(78) SM_CXVIRTUALSCREEN
  -1/*1200*/,    //(79) SM_CYVIRTUALSCREEN
  1,      //(80) SM_CMONITORS
  1,      //(81) SM_SAMEDISPLAYFORMAT
  1,      //(82) SM_IMMENABLED
  1,      //(83) SM_CXFOCUSBORDER
  1,      //(84) SM_CYFOCUSBORDER
  -1,      //     SM_NOTUSED85
  0,      //(86) SM_TABLETPC
  0,      //(87) SM_MEDIACENTER
  0,      //(88) SM_STARTER
  0,      //(89) SM_SERVERR2
};

GXBOOL GXWin32APIEmu::InitializeStatic()
{
  int i;

  // 创建位图储存列表
  s_pGXBitmapResMap = new GXBitmapResMap;

  // 创建画刷常量
  for(i = 0; i < 31; i++)
  {
    stSysColorBrush[i].emObjType = GXGDIOBJ_BRUSH;
    stSysColorBrush[i].uStyle   = GXBS_SOLID;
    stSysColorBrush[i].crColor   = crSysColor[i];
    stSysColorBrush[i].hHatch   = NULL;
  }
  gxCOMCTL32_RefreshSysColors();
  // 创建字体列表
  //s_pGXFontResMap = new GXFontResMap;

  // 创建系统默认字体
  GXLOGFONTW LogFont;
  memset(&LogFont, 0, sizeof(GXLOGFONTW));
  LogFont.lfHeight = 12;
  GXSTRCPYN(LogFont.lfFaceName, DEFAULT_FONT_NAMEW, GXLF_FACESIZE);
  g_hSystemFont = gxCreateFontIndirectW(&LogFont);

  // 创建系统默认的 Brush 和 Pen
  g_BlackBrush.emObjType = GXGDIOBJ_BRUSH;
  g_BlackBrush.pDC       = NULL;
  g_BlackBrush.uStyle    = GXBS_SOLID;
  g_BlackBrush.crColor   = 0xFF000000;
  g_BlackBrush.hHatch    = NULL;

  g_DarkGrayBrush   = g_BlackBrush;
  g_GrayBrush       = g_BlackBrush;
  g_LightGrayBrush  = g_BlackBrush;
  g_NullBrush       = g_BlackBrush;
  g_WhiteBrush      = g_BlackBrush;
  g_Brush_00FFFFFF  = g_BlackBrush;
  g_Brush_80FFFFFF  = g_BlackBrush;

  g_DarkGrayBrush.crColor   = 0xFF404040;
  g_GrayBrush.crColor       = 0xFF808080;
  g_LightGrayBrush.crColor  = 0xFFC0C0C0;
  g_NullBrush.crColor       = 0x00000000;
  g_WhiteBrush.crColor      = 0xFFFFFFFF;
  g_Brush_00FFFFFF.crColor  = 0x00FFFFFF;
  g_Brush_80FFFFFF.crColor  = 0x80FFFFFF;

  g_BlackPen.emObjType  = GXGDIOBJ_PEN;
  g_BlackPen.pDC        = NULL;
  g_BlackPen.uStyle     = GXPS_SOLID;
  g_BlackPen.uWidth     = 1;
  g_BlackPen.crColor    = 0xFF000000;

  g_WhitePen = g_BlackPen;
  g_NullPen  = g_BlackPen;
  g_WhitePen.crColor = 0xFFFFFFFF;
  g_NullPen.crColor  = 0x00000000;

  return TRUE;
}
GXVOID GXWin32APIEmu::SetScreen(long nWidth, long nHeight)
{
  g_SystemMetrics[GXSM_CXSCREEN] = nWidth;
  g_SystemMetrics[GXSM_CYSCREEN] = nHeight;
  g_SystemMetrics[GXSM_CXFULLSCREEN] = nWidth;
  g_SystemMetrics[GXSM_CYFULLSCREEN] = nHeight;
  g_SystemMetrics[GXSM_CXVIRTUALSCREEN] = nWidth;
  g_SystemMetrics[GXSM_CYVIRTUALSCREEN] = nHeight;

//#ifdef ENABLE_AERO
//  V(D3DXCreateTexture(
//    g_pd3dDevice, 
//    (GXUINT)(nWidth  / 2.0f), 
//    (GXUINT)(nHeight / 2.0f), 
//    1, 
//    D3DUSAGE_RENDERTARGET,
//    D3DFMT_A8R8G8B8,
//    D3DPOOL_DEFAULT,
//    &g_pCurStation->pBackDownSampTex));
//
//  g_pCurStation->pBackDownSampTex->GetSurfaceLevel(0, &g_pCurStation->pBackDownSampSur);
//#endif // ENABLE_AERO
}

GXBOOL GXWin32APIEmu::ReleaseStatic()
{
  if(s_pGXBitmapResMap != NULL)
  {
    for(GXBitmapResMap::iterator it = s_pGXBitmapResMap->begin();
      it != s_pGXBitmapResMap->end(); it++)
    {
      _gxDeleteObject((GXHGDIOBJ)it->second);
    }
    SAFE_DELETE(s_pGXBitmapResMap);
  }

  _gxDeleteObject(g_hSystemFont);
  //if(s_pGXFontResMap != NULL)
  //{
  //  for(GXFontResMap::iterator it = s_pGXFontResMap->begin();
  //    it != s_pGXFontResMap->end(); it++)
  //  {
  //    _gxDeleteObject((GXHGDIOBJ)*it);
  //  }
  //  SAFE_DELETE(s_pGXFontResMap);
  //}

  return TRUE;
}

GXVOID* GXWin32APIEmu::GetGXGdiObj(GXGDIOBJTYPE emObjType, GXVOID* uResCode)
{
  if(emObjType == GXGDIOBJ_BITMAP)
  {
    // uResCode 是资源名
    // TODO: uResCode可能是id或字符串，字符串可能是静态地址或动态地址
    // 动态地址时会出现多个实例
    GXBitmapResMap::iterator it = s_pGXBitmapResMap->find(uResCode);
    if(it == s_pGXBitmapResMap->end())
      return NULL;
    return it->second;
  }
  CLBREAK;
  //else if(emObjType == GXGDIOBJ_FONT)
  //{
  //  // uResCode 是内存地址
  //  for(GXFontResMap::iterator it = s_pGXFontResMap->begin();
  //    it != s_pGXFontResMap->end(); it++)
  //  {
  //    if(*it == uResCode)
  //      // FIXME: 测试这个代码,编译有警告,返回局部地址
  //      return (GXLPVOID)&it;
  //  }
  //  return NULL;
  //}
  return NULL;
}

GXVOID GXWin32APIEmu::MapGXGdiObj(GXGDIOBJTYPE emObjType, GXVOID* uResCode, GXLPVOID lpGdiObj)
{
  if(emObjType == GXGDIOBJ_BITMAP)
  {
    (*s_pGXBitmapResMap)[uResCode] = lpGdiObj;
  }
  CLBREAK;
  //else if(emObjType == GXGDIOBJ_FONT)
  //{
  //  (*s_pGXFontResMap).push_back(uResCode);
  //}
}

GXBOOL GXWin32APIEmu::EraseGXGdiObj(GXGDIOBJTYPE emObjType, GXLPVOID uResCode)
{
  //if(emObjType == GXGDIOBJ_FONT)
  //{
  //  // uResCode 是内存地址
  //  for(GXFontResMap::iterator it = s_pGXFontResMap->begin();
  //    it != s_pGXFontResMap->end(); it++)
  //  {
  //    if(*it == uResCode)
  //    {
  //      s_pGXFontResMap->erase(it);
  //      return TRUE;
  //    }
  //  }
  //}
  CLBREAK;
  return FALSE;
}
#endif // _DEV_DISABLE_UI_CODE

GXUINT GetBytesOfGraphicsFormat(GXFormat eFmt)
{
  switch(eFmt)
  {
  case GXFMT_A32B32G32R32F:
  case Format_R32G32B32A32_Float:
    return 16;

  case Format_R32G32B32_Float:
    return 12;

  case GXFMT_A16B16G16R16:
  //case GXFMT_Q16W16V16U16:
  case GXFMT_A16B16G16R16F:
  case GXFMT_G32R32F:
    return 8;

  case Format_BC1:
    return 1; // 4x4 pixel， 实际应该是0.5

  case Format_BC2:
  case Format_BC3:
    return 1; // 4x4 pixel

  case GXFMT_X8R8G8B8:
  case GXFMT_A8R8G8B8:
  case GXFMT_A8B8G8R8:
  //case GXFMT_A8R8G8B8:
  //case GXFMT_X8R8G8B8:
  case GXFMT_A2B10G10R10:
  case GXFMT_A2R10G10B10:
  //case GXFMT_A8B8G8R8:
  case GXFMT_X8B8G8R8:
  case GXFMT_V16U16:
  case GXFMT_A2W10V10U10:
  //case GXFMT_R8G8_B8G8:
  //case GXFMT_G8R8_G8B8:
  case GXFMT_R32F:  
  case GXFMT_D32:
  case GXFMT_D24S8:
  case GXFMT_D24X8:
  case GXFMT_G16R16:
  case GXFMT_X8L8V8U8:
  case GXFMT_Q8W8V8U8:
  case GXFMT_D32F_LOCKABLE:
  case GXFMT_D24FS8:
  case GXFMT_D24X4S4:
  case GXFMT_D32_LOCKABLE:
  case GXFMT_INDEX32:
  case GXFMT_G16R16F:
  case Format_B8G8R8A8:
  case Format_B8G8R8X8:
  case Format_R8G8B8A8:
  case Format_R16G16:
  case Format_R32:
  //case Format_R8G8B8X8:
  case Format_D32:
  case Format_D24S8:
  case Format_D24X8:
  case Format_Index32:
    return 4;
  
  case GXFMT_R8G8B8:
  //case Format_B8G8R8:
    return 3;
  
  case GXFMT_R5G6B5:
  case GXFMT_X1R5G5B5:
  case GXFMT_A1R5G5B5:
  case GXFMT_A4R4G4B4:
  case GXFMT_A8P8:
  case GXFMT_A8L8:
  case GXFMT_A8R3G3B2:
  case GXFMT_X4R4G4B4:
  case GXFMT_V8U8:
  case GXFMT_L6V5U5:
  case GXFMT_D16_LOCKABLE:
  case GXFMT_D15S1:
  case GXFMT_D16:
  case GXFMT_L16:
  case GXFMT_INDEX16:
  case GXFMT_R16F:
  case Format_D16:
  case Format_Index16:
  case Format_R8G8:
  case Format_R16:
    return 2;
  
  case GXFMT_A8:
  case GXFMT_R3G3B2:
  //case GXFMT_A8:
  case GXFMT_P8:
  case GXFMT_L8:
  case GXFMT_A4L4:
  case GXFMT_S8_LOCKABLE:
  case Format_R8:
  case Format_A8:
    return 1;

  default:
    ASSERT(0);
  }
  return 0;

  //case GXFMT_UYVY:
  //case GXFMT_YUY2:
  //case GXFMT_DXT1:
  //case GXFMT_DXT2:
  //case GXFMT_DXT3:
  //case GXFMT_DXT4:
  //case GXFMT_DXT5:
  //case GXFMT_VERTEXDATA:
  //case GXFMT_MULTI2_ARGB8:
  //case GXFMT_CxV8U8:
  //case GXFMT_A1:
  //case GXFMT_A2B10G10R10_XR_BIAS:
  //case GXFMT_BINARYBUFFER:
}

GXLPCSTR FormatToString(GXFormat eFormat)
{
#define CASE_TO_STRING(x) case x: return #x
  switch (eFormat)
  {
    CASE_TO_STRING(Format_Unknown);
    CASE_TO_STRING(Format_B8G8R8A8);
    CASE_TO_STRING(Format_B8G8R8X8);
    //CASE_TO_STRING(Format_B8G8R8);
    CASE_TO_STRING(Format_R8G8B8A8);
    CASE_TO_STRING(Format_R8G8);
    CASE_TO_STRING(Format_R16G16);
    CASE_TO_STRING(Format_R8);
    CASE_TO_STRING(Format_R16);
    CASE_TO_STRING(Format_R32);
    CASE_TO_STRING(Format_A8);
    CASE_TO_STRING(Format_R32G32B32A32_Float);
    CASE_TO_STRING(Format_R32G32B32_Float);
    CASE_TO_STRING(Format_D32);
    CASE_TO_STRING(Format_D16);
    CASE_TO_STRING(Format_D24S8);
    CASE_TO_STRING(Format_D24X8);
    CASE_TO_STRING(Format_Index16);
    CASE_TO_STRING(Format_Index32);
    CASE_TO_STRING(Format_BC1);
    CASE_TO_STRING(Format_BC2);
    CASE_TO_STRING(Format_BC3);
    CASE_TO_STRING(GXFMT_R8G8B8);
    CASE_TO_STRING(GXFMT_A8R8G8B8);
    CASE_TO_STRING(GXFMT_X8R8G8B8);
    CASE_TO_STRING(GXFMT_R5G6B5);
    CASE_TO_STRING(GXFMT_X1R5G5B5);
    CASE_TO_STRING(GXFMT_A1R5G5B5);
    CASE_TO_STRING(GXFMT_A4R4G4B4);
    CASE_TO_STRING(GXFMT_R3G3B2);
    CASE_TO_STRING(GXFMT_A8);
    CASE_TO_STRING(GXFMT_A8R3G3B2);
    CASE_TO_STRING(GXFMT_X4R4G4B4);
    CASE_TO_STRING(GXFMT_A2B10G10R10);
    CASE_TO_STRING(GXFMT_A8B8G8R8);
    CASE_TO_STRING(GXFMT_X8B8G8R8);
    CASE_TO_STRING(GXFMT_G16R16);
    CASE_TO_STRING(GXFMT_A2R10G10B10);
    CASE_TO_STRING(GXFMT_A16B16G16R16);

    CASE_TO_STRING(GXFMT_A8P8);
    CASE_TO_STRING(GXFMT_P8);
    CASE_TO_STRING(GXFMT_L8);
    CASE_TO_STRING(GXFMT_A8L8);
    CASE_TO_STRING(GXFMT_A4L4);

    CASE_TO_STRING(GXFMT_V8U8);
    CASE_TO_STRING(GXFMT_L6V5U5);
    CASE_TO_STRING(GXFMT_X8L8V8U8);
    CASE_TO_STRING(GXFMT_Q8W8V8U8);
    CASE_TO_STRING(GXFMT_V16U16);
    CASE_TO_STRING(GXFMT_A2W10V10U10);
    CASE_TO_STRING(GXFMT_DXT1);
    CASE_TO_STRING(GXFMT_DXT2);
    CASE_TO_STRING(GXFMT_DXT3);
    CASE_TO_STRING(GXFMT_DXT4);
    CASE_TO_STRING(GXFMT_DXT5);
    CASE_TO_STRING(GXFMT_D16_LOCKABLE);
    CASE_TO_STRING(GXFMT_D32);
    CASE_TO_STRING(GXFMT_D15S1);
    CASE_TO_STRING(GXFMT_D24S8);
    CASE_TO_STRING(GXFMT_D24X8);
    CASE_TO_STRING(GXFMT_D24X4S4);
    CASE_TO_STRING(GXFMT_D16);
    CASE_TO_STRING(GXFMT_D32F_LOCKABLE);
    CASE_TO_STRING(GXFMT_D24FS8);
    CASE_TO_STRING(GXFMT_D32_LOCKABLE);
    CASE_TO_STRING(GXFMT_S8_LOCKABLE);
    CASE_TO_STRING(GXFMT_L16);
    CASE_TO_STRING(GXFMT_VERTEXDATA);
    CASE_TO_STRING(GXFMT_INDEX16);
    CASE_TO_STRING(GXFMT_INDEX32);
    CASE_TO_STRING(GXFMT_R16F);
    CASE_TO_STRING(GXFMT_G16R16F);
    CASE_TO_STRING(GXFMT_A16B16G16R16F);
    CASE_TO_STRING(GXFMT_R32F);
    CASE_TO_STRING(GXFMT_G32R32F);
    CASE_TO_STRING(GXFMT_A32B32G32R32F);
    CASE_TO_STRING(GXFMT_CxV8U8);
    CASE_TO_STRING(GXFMT_A1);

  default:
    break;
  }
  return "<error format>";
#undef CASE_TO_STRING
}

GXDLL GXLPCSTR GetFormatChannelOrder(GXFormat eFormat)
{
  switch (eFormat)
  {
  case Format_R8G8B8A8: return "RGBA";
  case Format_B8G8R8X8: return "BGRX";
  case Format_R8:       return "R";
  case Format_R8G8:     return "RG";
  case Format_R32G32B32A32_Float:    return "RGBA"; // 不确定
  case Format_R32:      return "R";
  default:
    CLBREAK;
  }
  return NULL;
}

GXVOID GXDLLAPI MOCanonicalizeVertexDecl(LPGXVERTEXELEMENT pNewVertDecl, LPCGXVERTEXELEMENT pSrcVertDecl, GXBOOL bRelocalOffset)
{
  struct SORT_CONTEXT : public GXVERTEXELEMENT
  {
    GXBOOL SortCompare(SORT_CONTEXT& c) const {
      return Usage * 50 + UsageIndex > c.Usage * 50 + c.UsageIndex;
    }
    GXVOID SortSwap(SORT_CONTEXT& c) {
      SORT_CONTEXT t = c;
      c = *this;
      *this = t;
    }
  };
  GXUINT nCount = MOGetDeclCount(pSrcVertDecl);
  memcpy(pNewVertDecl, pSrcVertDecl, sizeof(GXVERTEXELEMENT) * (nCount + 1));

  QuickSort<SORT_CONTEXT, GXUINT>((SORT_CONTEXT*)pNewVertDecl, 0, nCount);
  if(bRelocalOffset) {
    // 重新计算Offset
    MORelocateVertexDecl(pNewVertDecl);
  }
}

GXVOID GXDLLAPI MORelocateVertexDecl(LPGXVERTEXELEMENT pVertDecl)
{
  GXUINT nOffset = 0;
  while((GXINT)pVertDecl->UsageIndex >= 0)
  {
    pVertDecl->Offset = nOffset;
    nOffset += MOGetDeclTypeSize(pVertDecl->Type);
    pVertDecl++;
  }
}

GXUINT GXDLLAPI MOGetDeclTypeSize(GXDeclType eType)
{
  switch(eType)
  {
  case GXDECLTYPE_FLOAT1:    // 1D float expanded to (value, 0., 0., 1.)
    return sizeof(float);
  case GXDECLTYPE_FLOAT2:    // 2D float expanded to (value, value, 0., 1.)
    return sizeof(float) * 2;
  case GXDECLTYPE_FLOAT3:    // 3D float expanded to (value, value, value, 1.)
    return sizeof(float) * 3;
  case GXDECLTYPE_FLOAT4:    // 4D float
    return sizeof(float) * 4;
  case GXDECLTYPE_D3DCOLOR:  // 4D packed unsigned bytes mapped to 0. to 1. range
    return sizeof(GXBYTE) * 4;
  case GXDECLTYPE_UBYTE4:    // 4D unsigned byte
  case GXDECLTYPE_UBYTE4N:   // Each of 4 bytes is normalized by dividing to 255.0
    return sizeof(GXBYTE) * 4;
  case GXDECLTYPE_SHORT2:    // 2D signed short expanded to (value, value, 0., 1.)
  case GXDECLTYPE_SHORT2N:   // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
  case GXDECLTYPE_USHORT2N:  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
    return sizeof(GXSHORT) * 2;
  case GXDECLTYPE_SHORT4:    // 4D signed short
  case GXDECLTYPE_SHORT4N:   // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
  case GXDECLTYPE_USHORT4N:    // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
    return sizeof(GXSHORT) * 4;

  case GXDECLTYPE_UDEC3:     // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
  case GXDECLTYPE_DEC3N:     // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
  case GXDECLTYPE_FLOAT16_2:// Two 16-bit floating point values, expanded to (value, value, 0, 1)
  case GXDECLTYPE_FLOAT16_4: // Four 16-bit floating point values
  case GXDECLTYPE_UNUSED:
  default:
    ASSERT(0);  // 不支持
  }
  return 0;
}

GXUINT GXDLLAPI MOGetDeclTypeDimension(GXDeclType eType)
{
  switch(eType)
  {
  case GXDECLTYPE_FLOAT1:    // 1D float expanded to (value, 0., 0., 1.)
    return 1;
  case GXDECLTYPE_FLOAT2:    // 2D float expanded to (value, value, 0., 1.)
    return 2;
  case GXDECLTYPE_FLOAT3:    // 3D float expanded to (value, value, value, 1.)
    return 3;
  case GXDECLTYPE_FLOAT4:    // 4D float
    return 4;
  case GXDECLTYPE_D3DCOLOR:  // 4D packed unsigned bytes mapped to 0. to 1. range
    return 4;
  case GXDECLTYPE_UBYTE4:    // 4D unsigned byte
  case GXDECLTYPE_UBYTE4N:   // Each of 4 bytes is normalized by dividing to 255.0
    return 4;
  case GXDECLTYPE_SHORT2:    // 2D signed short expanded to (value, value, 0., 1.)
  case GXDECLTYPE_SHORT2N:   // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
  case GXDECLTYPE_USHORT2N:  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
    return 2;
  case GXDECLTYPE_SHORT4:    // 4D signed short
  case GXDECLTYPE_SHORT4N:   // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
  case GXDECLTYPE_USHORT4N:    // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
    return 4;

  case GXDECLTYPE_UDEC3:     // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
  case GXDECLTYPE_DEC3N:     // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
  case GXDECLTYPE_FLOAT16_2:// Two 16-bit floating point values, expanded to (value, value, 0, 1)
  case GXDECLTYPE_FLOAT16_4: // Four 16-bit floating point values
  case GXDECLTYPE_UNUSED:
  default:
    ASSERT(0);  // 不支持
  }
  return 0;
}

GXFormatCategory GetGraphicsFormatCategory(GXFormat eFmt)
{
  switch(eFmt)
  {
  case GXFMT_R8G8B8:
  case GXFMT_A8R8G8B8:
  case GXFMT_X8R8G8B8:
  case GXFMT_R5G6B5:
  case GXFMT_X1R5G5B5:
  case GXFMT_A1R5G5B5:
  case GXFMT_A4R4G4B4:
  case GXFMT_R3G3B2:
  case GXFMT_A8:
  case GXFMT_A8R3G3B2:
  case GXFMT_X4R4G4B4:
  case GXFMT_A2B10G10R10:
  case GXFMT_A8B8G8R8:
  case GXFMT_X8B8G8R8:
  case GXFMT_G16R16:
  case GXFMT_A2R10G10B10:
  case GXFMT_A16B16G16R16:
  case GXFMT_A8P8:
  case GXFMT_P8:
  case GXFMT_L8:
  case GXFMT_A8L8:
  case GXFMT_A4L4:
  case GXFMT_V8U8:
  case GXFMT_L6V5U5:
  case GXFMT_X8L8V8U8:
  case GXFMT_Q8W8V8U8:
  case GXFMT_V16U16:
  case GXFMT_A2W10V10U10:
  //case GXFMT_UYVY:
  //case GXFMT_R8G8_B8G8:
  //case GXFMT_YUY2:
  //case GXFMT_G8R8_G8B8:
  case GXFMT_L16:
  case GXFMT_R16F:
  case GXFMT_G16R16F:
  case GXFMT_A16B16G16R16F:
  case GXFMT_R32F:
  case GXFMT_G32R32F:
  case GXFMT_A32B32G32R32F:
  case GXFMT_CxV8U8:
  case GXFMT_A1:
  //case GXFMT_A2B10G10R10_XR_BIAS:
    return GXFMTCATE_COLOR;

  case GXFMT_DXT1:
  case GXFMT_DXT2:
  case GXFMT_DXT3:
  case GXFMT_DXT4:
  case GXFMT_DXT5:
    return GXFMTCATE_COMPRESSEDCOLOR;

  case GXFMT_D16_LOCKABLE:
  case GXFMT_D32:
  case GXFMT_D15S1:
  case GXFMT_D24S8:
  case GXFMT_D24X8:
  case GXFMT_D24X4S4:
  case GXFMT_D16:
  case GXFMT_D32F_LOCKABLE:
  case GXFMT_D24FS8:
  case GXFMT_D32_LOCKABLE:
  case GXFMT_S8_LOCKABLE:
  case GXMAKEFOURCC('I','N','T','Z'):
    return GXFMTCATE_DEPTHSTENCIL;

  case GXFMT_VERTEXDATA:
  case GXFMT_INDEX16:
  case GXFMT_INDEX32:
    return GXFMTCATE_OTHER;

  default:
    CLBREAK;
    return GXFMTCATE_OTHER;
  }
}

GXUINT SizeRatioToDimension(GXINT nTexRatio, GXUINT nModel, GXDWORD dwFlags)
{
  if(((GXINT)nTexRatio) < 0) 
  {
    nTexRatio = (GXUINT)((GXFLOAT)(-nTexRatio) / (GXFLOAT)GXSizeRatio::FixedPoint * nModel);
  }

  if(TEST_FLAG(dwFlags, TEXTURERATIO_POW2))
  {
    nTexRatio = GetAdaptedSize(nTexRatio);
  }
  return nTexRatio;
}

GXBOOL IsPow2(GXINT nNum)
{
  return (nNum & (nNum - 1)) ? FALSE : TRUE;
}

GXINT GetAdaptedSize(GXINT nSize)
{
  return RoundupPowOfTwo<GXINT>(nSize);
}

// 从声明中获得需要的长度
GXUINT GXDLLAPI MOGetDeclVertexSize(LPCGXVERTEXELEMENT pVertDecl)
{
  GXUINT nOffset = 0;
  GXDeclType eType = GXDECLTYPE_UNUSED;
  while((GXINT)pVertDecl->UsageIndex >= 0)
  {
    if(nOffset < pVertDecl->Offset) {
      nOffset = pVertDecl->Offset;
      eType = pVertDecl->Type;
    }
    pVertDecl++;
  }
  return nOffset + MOGetDeclTypeSize(eType);
}

GXUINT GXDLLAPI MOGetDeclCount(LPCGXVERTEXELEMENT pVertDecl)
{
  return GetVertexDeclLength<GXUINT>(pVertDecl);
}

GXDWORD GXDLLAPI MOTestDeclElementFlags(LPCGXVERTEXELEMENT pVertDecl)
{
  GXDWORD dwFlags = 0;
  int nCount = 0;
  while((GXINT)pVertDecl[nCount].UsageIndex >= 0) {
    const GXVERTEXELEMENT& e = pVertDecl[nCount];
    if(e.Usage == GXDECLUSAGE_NORMAL) {
      dwFlags |= GXSHADERCAP_NORMAL;
    }
    else if(e.Usage == GXDECLUSAGE_COLOR && e.UsageIndex == 0) {
      dwFlags |= GXSHADERCAP_VCOLOR;
    }
    else if(e.Usage == GXDECLUSAGE_TEXCOORD) {
      switch(e.UsageIndex)
      {
      case 0: dwFlags |= GXSHADERCAP_TEXCOORD0; break;
      case 1: dwFlags |= GXSHADERCAP_TEXCOORD1; break;
      case 2: dwFlags |= GXSHADERCAP_TEXCOORD2; break;
      case 3: dwFlags |= GXSHADERCAP_TEXCOORD3; break;
      }      
    }
    nCount++;
  }
  return dwFlags;
}

void GXDLLAPI MOTestShaderCapsString(LPCGXVERTEXELEMENT pVertDecl, clStringA& strMacro)
{
  GXDWORD dwFlags = MOTestDeclElementFlags(pVertDecl);
  MOMakeShaderCapsString(dwFlags, strMacro);
}

void GXDLLAPI MOMakeShaderCapsString(GXDWORD dwCapsFlags, clStringA& strMacro)
{
  static GXLPCSTR aMacro[] = {
    "_INPUT_NORMAL_ELEMENT",
    "_INPUT_VCOLOR_ELEMENT",
    "_INPUT_TEXCOORD0_ELEMENT",
    "_INPUT_TEXCOORD1_ELEMENT",
    "_INPUT_TEXCOORD2_ELEMENT",
    "_INPUT_TEXCOORD3_ELEMENT",
  };

  // 要按顺序来
  STATIC_ASSERT(GXSHADERCAP_NORMAL    == 0x0004);
  STATIC_ASSERT(GXSHADERCAP_VCOLOR    == 0x0008);
  STATIC_ASSERT(GXSHADERCAP_TEXCOORD0 == 0x0010);
  STATIC_ASSERT(GXSHADERCAP_TEXCOORD1 == 0x0020);
  STATIC_ASSERT(GXSHADERCAP_TEXCOORD2 == 0x0040);
  STATIC_ASSERT(GXSHADERCAP_TEXCOORD3 == 0x0080);

  strMacro.Clear();
  for(int i = 0; i < 6; i++)
  {
    if(TEST_FLAG(dwCapsFlags, 4 << i)) {
      strMacro.Append(aMacro[i]);
      strMacro.Append(';');
    }
  }
}

// 获得指定的偏移
GXINT GXDLLAPI MOGetDeclOffset(
  GXIN LPCGXVERTEXELEMENT pVertDecl, 
  GXIN GXDeclUsage Usage, 
  GXIN GXUINT UsageIndex, 
  GXOUT LPGXVERTEXELEMENT lpDesc)
{
  while((GXINT)pVertDecl->UsageIndex >= 0)
  {
    if(pVertDecl->Usage == Usage &&
      pVertDecl->UsageIndex == UsageIndex) {
        if(lpDesc != NULL) {
          *lpDesc = *pVertDecl;
        }
        return pVertDecl->Offset;
    }
    pVertDecl++;
  }
  return -1;
}

template<typename _T>
inline void _MOConvertVertexFormat_CopyVertex(GXLPVOID lpDest, GXUINT nDestStride, GXLPVOID lpSrc, GXUINT nSrcStride, GXUINT nCount)
{
  for(GXUINT vi = 0; vi < nCount; vi++)
  {
    *((_T*)lpDest) = *((_T*)lpSrc);
    lpDest = ((GXLPBYTE)lpDest) + nDestStride;
    lpSrc = ((GXLPBYTE)lpSrc) + nSrcStride;
  }
}

GXBOOL GXDLLAPI MOConvertVertexFormat(
  GXLPVOID            lpDestVert, 
  GXLPCVERTEXELEMENT  lpDestDecl, 
  GXLPVOID            lpSrcVert, 
  GXLPCVERTEXELEMENT  lpSrcDecl, 
  GXUINT              nCount, 
  GXDWORD             dwFlags)
{
  // FIXME: 没完全实现!
  GXUINT nDestStride = MOGetDeclVertexSize(lpDestDecl);
  GXUINT nSrcStride  = MOGetDeclVertexSize(lpSrcDecl);

  for(int di = 0;; di++)
  {
    GXVERTEXELEMENT SrcElement;
    if((GXINT)lpDestDecl[di].UsageIndex < 0 || 
      lpDestDecl[di].Type < 0 || lpDestDecl[di].Type >= GXDECLTYPE_UNUSED) {
      break;
    }
    GXINT nOffset = MOGetDeclOffset(lpSrcDecl, lpDestDecl[di].Usage, lpDestDecl[di].UsageIndex, &SrcElement);
    if(nOffset < 0) {
      continue;
    }
    if(SrcElement.Type == lpDestDecl[di].Type)
    {
      switch(SrcElement.Type)
      {
      case GXDECLTYPE_FLOAT1:
        _MOConvertVertexFormat_CopyVertex<float>(
          (GXLPBYTE)lpDestVert + lpDestDecl[di].Offset, nDestStride,
          (GXLPBYTE)lpSrcVert + nOffset, nSrcStride, nCount);
        break;
      case GXDECLTYPE_FLOAT2:
        _MOConvertVertexFormat_CopyVertex<float2>(
          (GXLPBYTE)lpDestVert + lpDestDecl[di].Offset, nDestStride,
          (GXLPBYTE)lpSrcVert + nOffset, nSrcStride, nCount);
        break;
      case GXDECLTYPE_FLOAT3:
        _MOConvertVertexFormat_CopyVertex<float3>(
          (GXLPBYTE)lpDestVert + lpDestDecl[di].Offset, nDestStride,
          (GXLPBYTE)lpSrcVert + nOffset, nSrcStride, nCount);
        break;
      case GXDECLTYPE_FLOAT4:
        _MOConvertVertexFormat_CopyVertex<float4>(
          (GXLPBYTE)lpDestVert + lpDestDecl[di].Offset, nDestStride,
          (GXLPBYTE)lpSrcVert + nOffset, nSrcStride, nCount);
        break;
      case GXDECLTYPE_D3DCOLOR:
        _MOConvertVertexFormat_CopyVertex<GXDWORD>(
          (GXLPBYTE)lpDestVert + lpDestDecl[di].Offset, nDestStride,
          (GXLPBYTE)lpSrcVert + nOffset, nSrcStride, nCount);
        break;
      }
    }
  }
  return FALSE;
}


GXBOOL MOGenerateDeclarationCodes(DATALAYOUT* lpCommUniformDef, GXDWORD dwPlatfomCode, clBuffer** ppBuffer)
{
  if(lpCommUniformDef == NULL) {
    CLOG_ERROR(MOERROR_FMT_INVALIDPARAM, __FUNCTION__);
    return FALSE;
  }

  // 用于排序的结构体,不能有成员
  struct CONTEXT : public DATALAYOUT
  {
    GXBOOL SortCompare(CONTEXT& Stru) const {
      return uOffset > Stru.uOffset;
    }
    GXVOID SortSwap(CONTEXT& Stru) {
      CONTEXT t = Stru;
      Stru = *this;
      *this = t;
    }
  };

  STATIC_ASSERT(sizeof(CONTEXT) == sizeof(DATALAYOUT));

  typedef clvector<CONTEXT> ContextArray;
  ContextArray aContext;
  
  // 复制和检查字节对齐
  for(int i = 0;; i++)
  {
    if(lpCommUniformDef[i].pName == NULL) {
      break;
    }

    // 四字节对齐检查
    if(lpCommUniformDef[i].uOffset & 3) {
      CLOG_ERROR("%s Error: Must be aligned for 4 bytes.\n", __FUNCTION__);
      return FALSE;
    }
    aContext.push_back(*(CONTEXT*)&lpCommUniformDef[i]);
  }

  // 按照偏移排序
  QuickSort(&aContext.front(), 0, (int)aContext.size());

  GXUINT uOffset = 0;
  clStringA strDefine;
  clBuffer* pBuffer = new clBuffer();

  if(dwPlatfomCode == GXPLATFORM_X_OPENGLES2 ||
    dwPlatfomCode == GXPLATFORM_WIN32_OPENGL) {
      ASSERT(0); // 没实现
  }

  for(ContextArray::iterator it = aContext.begin();
    it != aContext.end(); ++it)
  {
    CONTEXT& c = *it;
    while(uOffset < c.uOffset) {
      strDefine.Format("float Unnamed%d;\r\n", uOffset);
      pBuffer->Append(strDefine.GetBuffer(), strDefine.GetLength());
      uOffset += sizeof(float);
    }
    ASSERT(uOffset == c.uOffset);
    switch(c.eType)
    {
    case GXUB_FLOAT:
      strDefine = "float";
      ASSERT(c.uSize == sizeof(float));
      break;
    case GXUB_FLOAT2:
      strDefine = "float2";
      ASSERT(c.uSize == sizeof(float2));
      break;
    case GXUB_FLOAT3:
      strDefine = "float3";
      ASSERT(c.uSize == sizeof(float3));
      break;
    case GXUB_FLOAT4:
      strDefine = "float4";
      ASSERT(c.uSize == sizeof(float4));
      break;
    case GXUB_MATRIX4:
      strDefine = "float4x4";
      ASSERT(c.uSize == sizeof(float4x4));
      break;
    default:
      ASSERT(0); // 不支持的类型
      break;
    }
    uOffset += c.uSize;
    strDefine.Append(' ');
    strDefine.Append(c.pName);
    strDefine.Append(";\r\n");
    pBuffer->Append(strDefine.GetBuffer(), strDefine.GetLength());
  }

  *ppBuffer = pBuffer;
  return TRUE;
}

void PixelFormat_A8R8G8B8ToA8B8G8R8(GXLPVOID lpDest, GXLPVOID lpSrc, GXUINT cbSize)
{
  //GXUINT uLoop = cbSize / sizeof(GXDWORD);
  GXBYTE* lpDestChannel = (GXBYTE*)lpDest;
  GXBYTE* lpSrcChannel = (GXBYTE*)lpSrc;
  for(GXUINT i = 0; i < cbSize; i += 4)
  {
    lpDestChannel[i    ] = lpSrcChannel[i + 2];
    lpDestChannel[i + 1] = lpSrcChannel[i + 1];
    lpDestChannel[i + 2] = lpSrcChannel[i    ];
    lpDestChannel[i + 3] = lpSrcChannel[i + 3];
  }
}


//////////////////////////////////////////////////////////////////////////
GXGDIOBJ::GXGDIOBJ(GXGDIOBJTYPE _emObjType, LPGXGDIDC _pDC)
  : emObjType(_emObjType)
  , pDC(_pDC)
{
}
GXGDIOBJ::GXGDIOBJ(GXGDIOBJTYPE _emObjType)
  : emObjType(_emObjType)
  , pDC(NULL)
{
}
GXGDIOBJ::GXGDIOBJ()
  : emObjType(GXGDIOBJ_NULL)
  , pDC(NULL)
{
}

GXGDIREGION::GXGDIREGION()
  : GXGDIOBJ(GXGDIOBJ_REGION)
  , lpRegion(NULL)
{
}
GXGDIREGION::GXGDIREGION(GRegion* pRegion)
  : GXGDIOBJ(GXGDIOBJ_REGION)
  , lpRegion(pRegion)
{
  lpRegion->AddRef();
  lpRegion->GetBounding(&rect);
}

GXGDIREGION::~GXGDIREGION()
{
  SAFE_RELEASE(lpRegion);
}
//////////////////////////////////////////////////////////////////////////
GXRSRC::GXRSRC(GXLPCSTR szFilename)
  : pBuffer(NULL)
  , strFilename(szFilename)
{
}

GXRSRC::~GXRSRC()
{
  SAFE_DELETE(pBuffer);
}

GXBOOL GXRSRC::Load()
{
  clFile file;
  if(file.OpenExisting(strFilename))
  {
    file.MapToBuffer(&pBuffer);
    return TRUE;
  }
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////
#if defined(_WIN32) || defined(_WINDOWS)
GXBOOL GXINSTANCE::Initialize(GXLPSTATION _lpStation, HINSTANCE hInst)
{
  lpStation = _lpStation;
  hInstance = hInst;

  GetModuleFileNameA(hInstance, strModuleName.GetBuffer(MAX_PATH), MAX_PATH);
  strModuleName.ReleaseBuffer();
  clpathfile::Canonicalize(strModuleName);

  clsize nFileExt = clpathfile::FindExtension(strModuleName);
  clsize nFileName = clpathfile::FindFileName(strModuleName);
  //strModuleName[nFileExt] = '\0';
  strRootDir = strModuleName.SubString(0, nFileName);
  strModuleName = strModuleName.SubString(nFileName, nFileExt - nFileName);
  return TRUE;
}
#else
GXBOOL GXINSTANCE::Initialize(GXLPSTATION _lpStation, GXLPSTR szModuleName)
{
  lpStation = _lpStation;
  //hInstance = hInst;

  return TRUE;
}
#endif // #if defined(_WIN32) || defined(_WINDOWS)

GXBOOL GXINSTANCE::Finalize()
{
  for(ResCodeDict::iterator it = sResCodeDict.begin();
    it != sResCodeDict.end(); ++it)
  {
    delete it->second;
  }
  sResCodeDict.clear();
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
#if defined(_WINDOWS) || defined(_WIN32)
STATIC_ASSERT(GXVK_BACK            == GXVK_BACK);
STATIC_ASSERT(GXVK_RETURN          == GXVK_RETURN);
STATIC_ASSERT(GXVK_SHIFT           == GXVK_SHIFT);
STATIC_ASSERT(GXVK_CONTROL         == GXVK_CONTROL);
STATIC_ASSERT(GXVK_MENU            == GXVK_MENU);
STATIC_ASSERT(GXVK_ESCAPE          == GXVK_ESCAPE);
STATIC_ASSERT(GXVK_SPACE           == GXVK_SPACE);
STATIC_ASSERT(GXVK_PRIOR           == GXVK_PRIOR);
STATIC_ASSERT(GXVK_NEXT            == GXVK_NEXT);
STATIC_ASSERT(GXVK_END             == GXVK_END);
STATIC_ASSERT(GXVK_HOME            == GXVK_HOME);
STATIC_ASSERT(GXVK_LEFT            == GXVK_LEFT);
STATIC_ASSERT(GXVK_UP              == GXVK_UP);
STATIC_ASSERT(GXVK_RIGHT           == GXVK_RIGHT);
STATIC_ASSERT(GXVK_DOWN            == GXVK_DOWN);
STATIC_ASSERT(GXVK_INSERT          == GXVK_INSERT);
STATIC_ASSERT(GXVK_DELETE          == GXVK_DELETE);
STATIC_ASSERT(GXVK_MULTIPLY        == GXVK_MULTIPLY);
STATIC_ASSERT(GXVK_ADD             == GXVK_ADD);
STATIC_ASSERT(GXVK_SUBTRACT        == GXVK_SUBTRACT);
STATIC_ASSERT(GXVK_F1              == GXVK_F1);
STATIC_ASSERT(GXVK_F4              == GXVK_F4);
STATIC_ASSERT(GXVK_LCONTROL        == GXVK_LCONTROL);
#endif // defined(_WINDOWS) || defined(_WIN32)

STATIC_ASSERT(sizeof(GXHWND) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHANDLE) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHWND) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHMENU) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHSTATION) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHINSTANCE) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHGDIOBJ) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHDC) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHBITMAP) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHPEN) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHFONT) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHRGN) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHBRUSH) == sizeof(GXLPVOID));
STATIC_ASSERT(sizeof(GXHICON) == sizeof(GXLPVOID));


//////////////////////////////////////////////////////////////////////////
// 全局头文件
// 标准接口
// 平台相关
// 私有头文件
