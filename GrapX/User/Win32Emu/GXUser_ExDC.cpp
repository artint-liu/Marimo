#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXCanvas.H>
#include <GrapX/GXImage.H>
#include <GrapX/GXSprite.H>

// 私有头文件
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include <User/GXWindow.h>
#include "Utility/AeroCommon.H"
#include <User/WindowsSurface.h>
#include "GrapX/WineComm.H"
#include "GrapX/guxtheme.h"

extern GXHFONT g_hSystemFont;
//gxGetDCEx
//gxScrollWindowEx
//gxGetSysColorBrush
//gxDrawFocusRect
//gxTabbedTextOutW
//////////////////////////////////////////////////////////////////////////
//static GXDC hGetDCObject;
//static GXRECT  rcGetDCSaved;
//extern GXGDIFONT DefaultFont;
GXHDC GXDLLAPI gxGetDCEx(
      GXHWND hWnd,      // handle of window 
      GXHRGN hrgnClip,    // handle of clip region  
      GXDWORD flags       // device-context creation flags 
      )
{
  LPGXGDIDC pNew    = new GXGDIDC;
  pNew->emObjType   = GXGDIOBJ_DC;
  pNew->pWndCanvas  = new GXWndCanvas(hWnd, hrgnClip ? GXGDI_RGN_PTR(hrgnClip)->lpRegion : NULL, flags);
  pNew->pCanvas     = pNew->pWndCanvas->GetCanvasUnsafe();
  pNew->lpBitmap    = NULL;
  pNew->hBindWnd    = hWnd;
  pNew->crTextBack  = 0xFFFFFFFF;
  pNew->crText      = 0xFF000000; 
  pNew->flag        = GXDCFLAG_OPAQUEBKMODE;
  pNew->hBrush      = &g_WhiteBrush;
  pNew->hPen        = &g_BlackPen;
  pNew->hFont       = GXGDI_FONT_PTR(g_hSystemFont);
  pNew->ptPen.x     = 0;
  pNew->ptPen.y     = 0;
  return GXGDI_DC_HANDLE(pNew);
}

GXHDC GXDLLAPI gxGetWindowDC(
        GXHWND hWnd   // handle of window  
        )
{
  return gxGetDCEx(hWnd, NULL, GXDCX_CACHE|GXDCX_WINDOW|GXDCX_PARENTCLIP);
}

//////////////////////////////////////////////////////////////////////////
GXHDC GXDLLAPI gxGetDC(
        GXHWND hWnd   // handle of window  
        )
{
  const GXDWORD dwDCXFlags = GXDCX_CACHE|GXDCX_PARENTCLIP;
  return gxGetDCEx(hWnd, NULL, dwDCXFlags);
}

int GXDLLAPI gxReleaseDC(
        GXHWND hWnd,  // handle of window 
        GXHDC hDC     // handle of device context  
        )
{
  LPGXGDIDC pDC = GXGDI_DC_PTR(hDC);

  SAFE_DELETE(pDC->pWndCanvas);
  SAFE_DELETE(pDC);

  return TRUE;
}
int GXDLLAPI gxScrollWindowEx(
  GXHWND hWnd,                 // handle of window to scroll
  int dx,                      // amount of horizontal scrolling
  int dy,                      // amount of vertical scrolling
  GXCONST GXRECT *prcScroll,   // address of structure with scroll rectangle
  GXCONST GXRECT *prcClip,     // address of structure with clip rectangle
  GXHRGN hrgnUpdate,           // handle of update region
  GXLPRECT prcUpdate,          // address of structure for update rectangle
  GXUINT flags                 // scrolling flags
  )
{
  //SCROLLTEXTUREDESC stdesc;
  GXLPWND lpWnd = GXWND_PTR(hWnd);

  if(lpWnd == NULL || (dx == 0 && dy == 0)) {
    return NULLREGION;
  }

  return lpWnd->Scroll(dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
}
//////////////////////////////////////////////////////////////////////////
GXHBRUSH GXDLLAPI gxGetSysColorBrush(
            int nIndex      // system color index
            )
{
  ASSERT(nIndex < 31);
  extern GXGDIBRUSH stSysColorBrush[];
  return (GXHBRUSH)&stSysColorBrush[nIndex];
}

GXBOOL GXDLLAPI gxDrawFocusRect(
           GXHDC hDC,        // handle to device context
           GXCONST GXRECT *lprc      // pointer to structure for rectangle  
           )
{
  //ASSERT(FALSE);
  //TRACE_UNACHIEVE("=== gxDrawFocusRect ===\n");
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hDC);
  lpDC->pCanvas->InvertRect(lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
  lpDC->pCanvas->InvertRect(lprc->left + 2, lprc->top + 2, lprc->right - lprc->left - 4, lprc->bottom - lprc->top - 4);
  return FALSE;
}

GXLONG GXDLLAPI gxTabbedTextOutW(
  GXHDC hDC,                    // handle of device context 
  int X,                        // x-coordinate of starting position 
  int Y,                        // y-coordinate of starting position 
  GXLPCWSTR lpString,           // address of string 
  int nCount,                   // number of characters in string 
  int nTabPositions,            // number of tabs in array 
  GXLPINT lpnTabStopPositions,  // address of array for tab positions  
  int nTabOrigin                // x-coordinate for tab expansion 
  )
{
#ifdef _DEBUG
  if(nTabOrigin != 0) {
    TRACE("暂时不支持 nTabOrigin != 0\n");
  }
#endif // #ifdef _DEBUG
  GXLONG ret = gxGetTabbedTextExtentW(hDC, lpString, nCount, nTabPositions, lpnTabStopPositions);
  //GXRECT rect = {X, Y, 0, 0};
  GXGDIDC* pDC = GXGDI_DC_PTR(hDC);
  if(pDC)
  {
    GXCanvas* pCanvas = GXGDI_DC_PTR(hDC)->pCanvas;
    if(pCanvas && pDC->hFont)
    {
      GXFont* pFont = GXGDI_FONT_PTR(pDC->hFont)->lpFont;
      if(TEST_FLAG(pDC->crTextBack, 0xff000000)) {
        pCanvas->FillRectangle(X, Y, GXLOWORD(ret), GXHIWORD(ret), pDC->crTextBack);
      }

      if(pFont)
      {
        GXLONG ret2 = pCanvas->TabbedTextOutW(pFont, X, Y, lpString, nCount, 
          nTabPositions, lpnTabStopPositions, pDC->crText);
        //ASSERT(ret == ret2);
      }
    }
  }
  return ret;
}

//gxInvalidateRgn
GXBOOL GXDLLAPI gxInvalidateRgn(
           GXHWND hWnd,        // handle of window with changed update region  
           GXHRGN hRgn,        // handle of region to add
           GXBOOL bErase       // erase-background flag
           )
{
  GXWnd* pWnd = GXWND_PTR(hWnd);
  if(pWnd && pWnd->IsAncestorVisible()) {
    if(hRgn == NULL) {
      return pWnd->InvalidateRect(NULL, bErase);
    }
    else {
      GRegion* pRegion = GXGDI_RGN_PTR(hRgn)->lpRegion;
      return pWnd->InvalidateRgn(pRegion, bErase);
    }
  }
  return FALSE;
}

//gxGetTabbedTextExtentW
GXDWORD GXDLLAPI gxGetTabbedTextExtentW(
              GXHDC hDC,          // handle of device context 
              GXLPCWSTR lpString,        // address of character string 
              GXINT nCount,          // number of characters in string 
              GXINT nTabPositions,      // number of tab positions 
              GXINT* lpnTabStopPositions   // address of array of tab positions  
              )
{
  GXGDIDC* pDC = GXGDI_DC_PTR(hDC);
  if(pDC)
  {
    GXCanvas* pCanvas = pDC->pCanvas;
    if(pCanvas && pDC->hFont)
    {
      GXFont* pFont = GXGDI_FONT_PTR(pDC->hFont)->lpFont;
      return pCanvas->TabbedTextOutW(pFont, 0, 0, lpString, nCount, nTabPositions, lpnTabStopPositions, 0);
    }
  }
  return 0;
  //NOT_IMPLEMENT_FUNC_MAKER;
  //return 0;
  //return GetTabbedTextExtentW(GXGDI_DC_PTR(hDC)->pCanvas->m_hWin32DC, lpString, (int)nCount, (int)nTabPositions, lpnTabStopPositions);
}
int GXDLLAPI gxFillRect(
         GXHDC hDC,        // handle to device context 
         GXCONST GXRECT *lprc,    // pointer to structure with rectangle  
         GXHBRUSH hbr      // handle to brush 
         )
{
  //hDC->pGraphics->GetSimple_XYZ_COLOR_Shader()->Apply();
  //hDC->pGraphics->FillRectangle(hbr->crColor, lprc->left + hDC->wndOrgX, lprc->top + hDC->wndOrgY, lprc->right, lprc->bottom);
  //hDC->pGraphics->GetSimpleShader()->Apply();
  GXCOLORREF crBrush = GXGDI_BRUSH_PTR(hbr)->crColor;
  if((crBrush & 0xff000000) != 0)
    GXGDI_DC_PTR(hDC)->pCanvas->FillRectangle(lprc->left, lprc->top, lprc->right-lprc->left, lprc->bottom-lprc->top, crBrush);
  return TRUE;
}

GXDWORD GXDLLAPI gxGetSysColor(
          int nIndex   // display element
          )
{
  extern GXCOLORREF crSysColor[];
  //= {
  //  0,      // #define GXCOLOR_SCROLLBAR         0
  //  0,      // #define GXCOLOR_BACKGROUND        1
  //  0,      // #define GXCOLOR_ACTIVECAPTION     2
  //  0,      // #define GXCOLOR_INACTIVECAPTION   3
  //  0xffB0B0B0,  // #define GXCOLOR_MENU              4
  //  0,      // #define GXCOLOR_WINDOW            5
  //  0,      // #define GXCOLOR_WINDOWFRAME       6
  //  0xff000000,  // #define GXCOLOR_MENUTEXT          7
  //  0,      // #define GXCOLOR_WINDOWTEXT        8
  //  0,      // #define GXCOLOR_CAPTIONTEXT       9
  //  0,      // #define GXCOLOR_ACTIVEBORDER      10
  //  0,      // #define GXCOLOR_INACTIVEBORDER    11
  //  0,      // #define GXCOLOR_APPWORKSPACE      12
  //  0xFF0C1481,  // #define GXCOLOR_HIGHLIGHT         13
  //  0xFFfff897,  // #define GXCOLOR_HIGHLIGHTTEXT     14
  //  0,      // #define GXCOLOR_BTNFACE           15
  //  0x0099A8AC,  // #define GXCOLOR_BTNSHADOW         16
  //  0xFFA0A0A0,  // #define GXCOLOR_GRAYTEXT          17
  //  0,      // #define GXCOLOR_BTNTEXT           18
  //  0,      // #define GXCOLOR_INACTIVECAPTIONTEXT 19
  //  0xFFFFFFFF,  // #define GXCOLOR_BTNHIGHLIGHT      20
  //  0xFF646F71,  // #define GXCOLOR_3DDKSHADOW        21
  //  0,      // #define GXCOLOR_3DLIGHT           22
  //  0,      // #define GXCOLOR_INFOTEXT          23
  //  0,      // #define GXCOLOR_INFOBK            24
  //  0,      // Unknown               25
  //  0,      // #define GXCOLOR_HOTLIGHT          26
  //  0,      // #define GXCOLOR_GRADIENTACTIVECAPTION 27
  //  0,      // #define GXCOLOR_GRADIENTINACTIVECAPTION 28
  //  0,      // #define GXCOLOR_MENUHILIGHT       29
  //  0,      // #define GXCOLOR_MENUBAR           30
  //};

  //if(crSysColor[nIndex] == 0)
  //{
  //  GXDWORD crWinSys = GetSysColor(nIndex);
  //  ASSERT(crSysColor[nIndex] != 0);
  //}
  //ASSERT(nIndex != 4);
  ASSERT(nIndex < 31);
  return crSysColor[nIndex];
//   switch(nIndex)
//   {
//     
//   }
}

GXHBRUSH GXDLLAPI gxGetCtrlBrush(int nIndex)
{
  static LPGXGDIBRUSH apCtrlBrush[CTLBRUSH_MAX] =
  {
    &g_GrayBrush,         // #define CTLBRUSH_MSGBOX         0
    &g_WhiteBrush,        // #define CTLBRUSH_EDIT           1
    &g_Brush_80FFFFFF,    // #define CTLBRUSH_LISTBOX        2
    &g_NullBrush,         // #define CTLBRUSH_BTN            3
    &g_LightGrayBrush,    // #define CTLBRUSH_DLG            4
    &g_NullBrush,         // #define CTLBRUSH_SCROLLBAR      5
    &g_Brush_00FFFFFF,    // #define CTLBRUSH_STATIC         6
  };
  //static GXGDIBRUSH stCtlBrush[CTLBRUSH_MAX] = 
  //{
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0xff808080, NULL},      // #define CTLBRUSH_MSGBOX         0
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0xFFFFFFFF, NULL},      // #define CTLBRUSH_EDIT           1
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0x80FFFFFF, NULL},      // #define CTLBRUSH_LISTBOX        2
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0      , NULL},         // #define CTLBRUSH_BTN            3
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0xffC0C0C0, NULL},      // #define CTLBRUSH_DLG            4
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0      , NULL},         // #define CTLBRUSH_SCROLLBAR      5
  //  {GXGDIOBJ_BRUSH, GXBS_SOLID,0x00FFFFFF, NULL},      // #define CTLBRUSH_STATIC         6
  //};
  //ASSERT(stCtlBrush[nIndex].crColor != 0);
  return GXGDI_BRUSH_HANDLE(apCtrlBrush[nIndex]);
}

GXHBITMAP GXDLLAPI gxLoadBitmapA(
           GXHINSTANCE hInstance,  // handle of application instance 
           GXLPCSTR lpBitmapName   // address of bitmap resource name 
           )
{
  ASSERT(FALSE);
  return NULL;
}

GXHBITMAP GXDLLAPI gxLoadBitmapW(
            GXHINSTANCE hInstance,  // handle of application instance 
            GXLPCWSTR lpBitmapName   // address of bitmap resource name 
           )
{
  LPGXGDIBITMAP pBitmap;
  pBitmap = (LPGXGDIBITMAP)GXWin32APIEmu::GetGXGdiObj(GXGDIOBJ_BITMAP, (GXLPVOID)lpBitmapName);
  if(pBitmap == NULL)
  {
    GXLPSTATION lpStation = IntGetStationPtr();
    clStringW strFilename;

    pBitmap = new GXGDIBITMAP;
    memset(pBitmap, 0, sizeof(GXGDIBITMAP));
    pBitmap->emObjType = GXGDIOBJ_BITMAP;

    if(IS_IDENTIFY(lpBitmapName))
    {
      GXWCHAR* szBaseDir = L"Resource\\OBM\\";
      GXWCHAR* szOBMFile = NULL;
      switch((GXDWORD)lpBitmapName)
      {
      case GXOBM_CLOSE:
        break;
      case GXOBM_UPARROW:
        szOBMFile = L"obm_uparrow.png";
        break;
      case GXOBM_DNARROW:
        szOBMFile = L"obm_dnarrow.png";
        break;
      case GXOBM_RGARROW:
        break;
      case GXOBM_LFARROW:
        break;
      case GXOBM_REDUCE:
        break;
      case GXOBM_ZOOM:
        break;
      case GXOBM_RESTORE:
        break;
      case GXOBM_REDUCED:
        break;
      case GXOBM_ZOOMD:
        break;
      case GXOBM_RESTORED:
        break;
      case GXOBM_UPARROWD:
        break;
      case GXOBM_DNARROWD:
        break;
      case GXOBM_RGARROWD:
        break;
      case GXOBM_LFARROWD:
        break;
      case GXOBM_MNARROW:
        szOBMFile = L"obm_mnarrow.png";
        break;
      case GXOBM_COMBO:
        break;
      case GXOBM_UPARROWI:
        szOBMFile = L"obm_uparrowi.png";
        break;
      case GXOBM_DNARROWI:
        szOBMFile = L"obm_dnarrowi.png";
      case GXOBM_RGARROWI:
        break;
      case GXOBM_LFARROWI:
        break;
      case GXOBM_OLD_CLOSE:
        break;
      case GXOBM_SIZE:
        break;
      case GXOBM_OLD_UPARROW:
        break;
      case GXOBM_OLD_DNARROW:
        break;
      case GXOBM_OLD_RGARROW:
        break;
      case GXOBM_OLD_LFARROW:
        break;
      case GXOBM_BTSIZE:
        break;
      case GXOBM_CHECK:
        break;
      case GXOBM_CHECKBOXES:
        szOBMFile = L"obm_checkboxes.png";
        break;
      case GXOBM_BTNCORNERS:
        break;
      case GXOBM_OLD_REDUCE:
        break;
      case GXOBM_OLD_ZOOM:
        break;
      case GXOBM_OLD_RESTORE:
        break;
      default:
        ASSERT(0);
        break;
      }

      // [获得成就] 第一次实际使用 clString 哦!!
      if(szOBMFile != NULL)
        strFilename = clStringW(szBaseDir) + szOBMFile;
    }
    if(strFilename.GetLength() != 0)
      pBitmap->pImage = lpStation->pGraphics->CreateImageFromFile(strFilename);
    else
      pBitmap->pImage = lpStation->pGraphics->CreateImage(16, 16, GXFMT_A8, FALSE, NULL);
    GXWin32APIEmu::MapGXGdiObj(GXGDIOBJ_BITMAP, (GXLPVOID)lpBitmapName, pBitmap);
  }
  return GXGDI_BITMAP_HANDLE(pBitmap);
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDrawStateW(
              GXHDC hdc,  // handle to device context
              GXHBRUSH hbr,  // handle to brush
              GXDRAWSTATEPROC lpOutputFunc,  // pointer to callback function
              GXLPARAM lData,  // image information
              GXWPARAM wData,  // more image information
              int x,  // horizontal location of image
              int y,  // vertical location of image
              int cx,  // width of image
              int cy,  // height of image
              GXUINT fuFlags  // image type and state 
              )
{
  if(lpOutputFunc != NULL)
  {
    gxSetWindowOrgEx(hdc, -x, -y, NULL);
    lpOutputFunc(hdc, lData, wData, cx, cy);
    gxSetWindowOrgEx(hdc, 0, 0, NULL);
  }
  if(fuFlags == GXDSS_NORMAL)
  {
    return TRUE;
  }
  else if(fuFlags & GXDSS_DISABLED)
  {
    return TRUE;
  }
  TRACE_UNACHIEVE("=== gxDrawStateW ===\n");
  ASSERT(FALSE);
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDrawFrameControl(
              GXHDC hdc,   // handle to device context
              GXLPRECT lprc,  // pointer to bounding rectangle
              GXUINT uType,  // frame-control type
              GXUINT uState  // frame-control state
              )
{
  return GXGDI_DC_PTR(hdc)->pWndCanvas->DrawFrameControl(lprc, uType, uState);
}

GXHANDLE GXDLLAPI gxCopyImage(
         GXHANDLE hImage,   // handle to the image to copy
         GXUINT uType,    // type of image to copy
         int cxDesired,    // desired width of new image
         int cyDesired,    // desired height of new image
         GXUINT fuFlags    // copy flags
         )
{
  ASSERT(fuFlags == GXLR_COPYRETURNORG);
  if(uType == GXIMAGE_BITMAP)
  {
    LPGXGDIBITMAP lpOrgBitmap = GXGDI_BITMAP_PTR(hImage);
    LPGXGDIBITMAP lpNewBitmap;
    lpNewBitmap = new GXGDIBITMAP;
    lpNewBitmap->emObjType    = lpOrgBitmap->emObjType;
    lpNewBitmap->bmWidth    = lpOrgBitmap->bmWidth; 
    lpNewBitmap->bmHeight    = lpOrgBitmap->bmHeight; 
    lpNewBitmap->bmWidthBytes  = lpOrgBitmap->bmWidthBytes; 
    lpNewBitmap->bmPlanes    = lpOrgBitmap->bmPlanes; 
    lpNewBitmap->bmBitsPixel  = lpOrgBitmap->bmBitsPixel; 
    lpNewBitmap->bmBits      = lpOrgBitmap->bmBits; 
    ASSERT(lpOrgBitmap->bmBits == NULL);
    lpNewBitmap->pImage      = lpOrgBitmap->pImage->Clone();
    return (GXHANDLE)lpNewBitmap;
  }
  else
    ASSERT(false);
  return NULL;
}

GXBOOL GXDLLAPI gxDrawIcon(
        GXHDC hDC,    // handle to device context
        int X,        // x-coordinate of upper-left corner
        int Y,        // y-coordinate of upper-left corner
        GXHICON hIcon // handle to icon to draw
        )
{
  ASSERT(false);
  return false;
}

GXBOOL GXDLLAPI gxGetIconInfo(
               GXHICON hIcon,         // icon handle
               GXPICONINFO piconinfo  // address of icon structure
               )
{
  ASSERT(FALSE);
  return FALSE;
}

GXHICON GXDLLAPI gxCopyIcon(
             GXHICON hIcon   // handle to icon to copy
             )
{
  ASSERT(false);
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDrawIconEx(
              GXHDC hdc,            // handle to device context
              int xLeft,            // x-coordinate of upper left corner
              int yTop,            // y-coordinate of upper left corner
              GXHICON hIcon,          // handle to icon to draw
              int cxWidth,          // width of the icon
              int cyWidth,          // height of the icon
              GXUINT istepIfAniCur,        // index of frame in animated cursor
              GXHBRUSH hbrFlickerFreeDraw,  // handle to background brush
              GXUINT diFlags          // icon-drawing flags
              )
{
  ASSERT(FALSE);
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXHANDLE GXDLLAPI gxLoadImageW(
              GXHINSTANCE hinst,     // handle of the instance that contains the image
              GXLPCWSTR lpszName,    // name or identifier of image
              GXUINT uType,      // type of image
              int cxDesired,      // desired width
              int cyDesired,      // desired height
              GXUINT fuLoad      // load flags
              )
{
  //ASSERT(FALSE);
  TRACE_UNACHIEVE("=== gxLoadImageW ===\n");
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
GXHICON GXDLLAPI gxLoadIconW(
             GXHINSTANCE hInstance,    // handle of application instance
             GXLPCWSTR lpIconName     // icon-name string or icon resource identifier
             )
{
  ASSERT(FALSE);
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
GXHANDLE GXDLLAPI gxLoadImageA(GXHINSTANCE hInstance, GXLPCSTR lpszName, GXUINT uType, GXINT cxDesired, GXINT cyDesired, GXUINT fuLoad)
{
  TRACE_UNACHIEVE("=== gxLoadImageA ===\n");
  return NULL;
}

GXHICON GXDLLAPI gxLoadIconA(GXHINSTANCE hInstance, GXLPCSTR lpIconName)
{
  ASSERT(FALSE);
  return NULL;
}

//////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) || defined (_WINDOWS)
GXHICON GXDLLAPI GXCursorToIcon(GXHCURSOR hCursor)
{
  ICONINFO IconInfo;
  GXBITMAP BitmapMask;
  GXBITMAP BitmapColor;
  BITMAPINFO bmiMask;
  BITMAPINFO bmiColor;
  GXDWORD* lpColor = NULL;
  GXDWORD* lpMask = NULL;
  LPGXICON pIconRet = NULL;
  GXGraphics* pGraphics = IntGetStationPtr()->pGraphics;

  HDC hdc;

  if(GetIconInfo((HICON)hCursor, &IconInfo) == FALSE)
  {
    VERIFY(false);
    return NULL;
  }

  if(  (IconInfo.hbmMask != NULL && 
    GetObjectW(IconInfo.hbmMask, sizeof(BitmapMask), &BitmapMask) == 0) ||
    (IconInfo.hbmColor != NULL && 
    GetObjectW(IconInfo.hbmColor, sizeof(BitmapColor), &BitmapColor) == 0) )
  {
    VERIFY(false);
    goto FAILE_RET;
  }
  hdc = GetDC(GetDesktopWindow());
  if(hdc == NULL)
  {
    VERIFY(false);
    goto FAILE_RET;
  }

  if(IconInfo.hbmColor != NULL)
  {
    bmiColor.bmiHeader.biSize      = sizeof(bmiColor.bmiHeader);
    bmiColor.bmiHeader.biWidth      = BitmapColor.bmWidth; 
    bmiColor.bmiHeader.biHeight      = -BitmapColor.bmHeight; 
    bmiColor.bmiHeader.biPlanes      = 1; 
    bmiColor.bmiHeader.biBitCount    = 32; 
    bmiColor.bmiHeader.biCompression  = BI_RGB; 
    bmiColor.bmiHeader.biSizeImage    = 0; 
    bmiColor.bmiHeader.biXPelsPerMeter  = 0; 
    bmiColor.bmiHeader.biYPelsPerMeter  = 0; 
    bmiColor.bmiHeader.biClrUsed    = 0; 
    bmiColor.bmiHeader.biClrImportant  = 0;

    lpColor = new GXDWORD[BitmapColor.bmWidth * BitmapColor.bmHeight];
    if(lpColor == NULL)
      goto FAILE_RET0;
    if(GetDIBits(hdc, IconInfo.hbmColor, 0, -bmiColor.bmiHeader.biHeight, lpColor, &bmiColor, DIB_RGB_COLORS) == 0)
    {
      VERIFY(false);
      goto FAILE_RET1;
    }

  }

  if(IconInfo.hbmMask != NULL)
  {
    bmiMask.bmiHeader.biSize      = sizeof(bmiColor.bmiHeader);
    bmiMask.bmiHeader.biWidth      = BitmapMask.bmWidth; 
    bmiMask.bmiHeader.biHeight      = -BitmapMask.bmHeight; 
    bmiMask.bmiHeader.biPlanes      = 1; 
    bmiMask.bmiHeader.biBitCount    = 32; 
    bmiMask.bmiHeader.biCompression    = BI_RGB; 
    bmiMask.bmiHeader.biSizeImage    = 0; 
    bmiMask.bmiHeader.biXPelsPerMeter  = 0; 
    bmiMask.bmiHeader.biYPelsPerMeter  = 0; 
    bmiMask.bmiHeader.biClrUsed      = 0; 
    bmiMask.bmiHeader.biClrImportant  = 0;

    lpMask = new GXDWORD[BitmapMask.bmWidth * BitmapMask.bmHeight];
    if(lpMask == NULL)
      goto FAILE_RET1;
    if(GetDIBits(hdc, IconInfo.hbmMask, 0, -bmiMask.bmiHeader.biHeight, lpMask, &bmiMask, DIB_RGB_COLORS) == 0)
    {
      VERIFY(false);
      goto FAILE_RET2;
    }
  }
  pIconRet = new GXICON;
  if(pIconRet == NULL)
    goto FAILE_RET2;

  pIconRet->fIcon = TRUE;
  pIconRet->xHotspot = IconInfo.xHotspot;
  pIconRet->yHotspot = IconInfo.yHotspot;

  if(IconInfo.hbmMask != NULL && IconInfo.hbmColor == NULL)
  {
    size_t uDelta = BitmapMask.bmWidth * (BitmapMask.bmHeight >> 1);
    for(size_t i = 0; i < uDelta; i++)
    {
      lpMask[i] = (((~lpMask[i]) << 8) & 0xff000000) | (lpMask[uDelta + i]);
    }
    pIconRet->pImgIcon = pGraphics
      ->CreateImage(BitmapMask.bmWidth, BitmapMask.bmHeight >> 1, GXFMT_A8R8G8B8, FALSE, lpMask);

    //GXCreateImage(BitmapMask.bmWidth, 
    //  BitmapMask.bmHeight >> 1, lpMask, FALSE, &pIconRet->pImgIcon);
    if(pIconRet->pImgIcon == NULL)
    {
      SAFE_DELETE(pIconRet);
      goto FAILE_RET2;
    }
  }
  else if(IconInfo.hbmMask != NULL && IconInfo.hbmColor != NULL)
  {
    pIconRet->pImgIcon = pGraphics
      ->CreateImage(BitmapMask.bmWidth, BitmapMask.bmHeight, GXFMT_A8R8G8B8, FALSE, lpColor);
    //GXCreateImage(BitmapMask.bmWidth, 
    //  BitmapMask.bmHeight, lpColor, FALSE, &pIconRet->pImgIcon);
    if(pIconRet->pImgIcon == NULL)
    {
      SAFE_DELETE(pIconRet);
      goto FAILE_RET2;
    }
  }


FAILE_RET2:
  SAFE_DELETE_ARRAY(lpMask);
FAILE_RET1:
  SAFE_DELETE_ARRAY(lpColor);
FAILE_RET0:
  ReleaseDC(GetDesktopWindow(), hdc);
FAILE_RET:
  DeleteObject(IconInfo.hbmColor);
  DeleteObject(IconInfo.hbmMask);
  return GXICON_HANDLE(pIconRet);
}

#endif // #if defined (_WIN32) || defined (_WINDOWS)
//////////////////////////////////////////////////////////////////////////
//GXHCURSOR GXDLLAPI gxLoadCursor(
//               HINSTANCE hInstance,    // handle of application instance
//               LPCTSTR lpCursorName   // name string or cursor resource identifier  
//             )
//{
//  ASSERT(FALSE);
//  return NULL;
//}

GXBOOL GXWndCanvas::DrawFrameControl(GXLPRECT lprc,GXUINT uType,GXUINT uState)
{
  //ASSERT(FALSE);
  switch(uType)
  {
  case GXDFC_BUTTON:
    switch(uState & 0xff)
    {
    case GXDFCS_BUTTONPUSH:
      if(uState & GXDFCS_PUSHED)
        GXWnd::s_pCommonSpr->PaintModule3x3(m_pNative, IDCOMMON_BTN_PRESSED_TOPLEFT, FALSE, lprc);
      else if(uState & GXDFCS_FLAT)
        GXWnd::s_pCommonSpr->PaintModule3x3(m_pNative, IDCOMMON_BTN_OVER_TOPLEFT, FALSE, lprc);
      else if(uState & GXDFCS_HOT)
        GXWnd::s_pCommonSpr->PaintModule3x3(m_pNative, IDCOMMON_BTN_DEFAULT_TOPLEFT, FALSE, lprc);
      else
        GXWnd::s_pCommonSpr->PaintModule3x3(m_pNative, IDCOMMON_BTN_NORMAL_TOPLEFT, FALSE, lprc);
      uState &= (~(GXDFCS_ADJUSTRECT | GXDFCS_PUSHED | GXDFCS_FLAT | GXDFCS_HOT));
      ASSERT(uState == GXDFCS_BUTTONPUSH);
      //gxFillRect(hdc, lprc, gxGetCtlBrush(CTLBRUSH_MSGBOX));
      break;
    case GXDFCS_BUTTONCHECK:
      if((uState & 0xff00) == 0)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_CHECK_NORMAL, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else if(uState & GXDFCS_CHECKED)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_CHECK_SELECT, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else if(uState & GXDFCS_PUSHED)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_CHECK_PRESSED, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else
      {
        //gxFillRect(hdc, lprc, gxGetCtrlBrush(CTLBRUSH_EDIT));
        GXHBRUSH hBrush = gxGetCtrlBrush(CTLBRUSH_EDIT);
        GXGDIBRUSH sBrush;
        gxGetObject(hBrush, sizeof(GXGDIBRUSH), &sBrush);
        FillRect(lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, sBrush.crColor);

      }
      break;
    case GXDFCS_BUTTONRADIO:
      if((uState & 0xff00) == 0)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_RADIO_NORMAL, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else if(uState & GXDFCS_CHECKED)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_RADIO_SELECT, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else if(uState & GXDFCS_PUSHED)
        GXWnd::s_pCommonSpr->PaintModule(m_pNative, IDCOMMON_RADIO_PRESSED, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top);
      else
      {
        GXHBRUSH hBrush = gxGetCtrlBrush(CTLBRUSH_LISTBOX);
        GXGDIBRUSH sBrush;
        gxGetObject(hBrush, sizeof(GXGDIBRUSH), &sBrush);
        FillRect(lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, sBrush.crColor);
        //gxFillRect(hdc, lprc, gxGetCtrlBrush(CTLBRUSH_LISTBOX));
      }
      break;
    default:
      TRACE_UNACHIEVE("=== gxDrawFrameControl ===\n");
      ASSERT(FALSE);
    }
    break;
  case GXDFC_SCROLL:
    {
      extern GXHTHEME hTheme_Window;
      extern GXHTHEME hTheme_ScrollBar;
      extern GXHTHEME hTheme_Button;

      GXDrawThemeBackground(hTheme_ScrollBar, *this, GXSBP_THUMBBTNVERT, /*GXSCRBS_NORMAL*/GXSCRBS_PRESSED, lprc, NULL);
    }
    break;
  default:
    TRACE_UNACHIEVE("=== gxDrawFrameControl ===\n");
    ASSERT(FALSE);
    break;
  }
  return FALSE;
}
#endif // _DEV_DISABLE_UI_CODE