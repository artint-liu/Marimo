#ifndef _DEV_DISABLE_UI_CODE
/*
* Static control
*
* Copyright  David W. Metcalfe, 1993
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*
* NOTES
*
* This code was audited for completeness against the documented features
* of Comctl32.dll version 6.0 on Oct. 4, 2004, by Dimitrie O. Paun.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
*
* Notes:
*   - Windows XP introduced new behavior: The background of centered
*     icons and bitmaps is painted differently. This is only done if
*     a manifest is present.
*     Because it has not yet been decided how to implement the two
*     different modes in Wine, only the Windows XP mode is implemented.
*   - Controls with SS_SIMPLE but without SS_NOPREFIX:
*     The text should not be changed. Windows doesn't clear the
*     client rectangle, so the new text must be larger than the old one.
*   - The SS_RIGHTJUST style is currently not implemented by Windows
*     (or it does something different than documented).
*
* TODO:
*   - Animated cursors
*/
// typedef unsigned short wchar_t;
// typedef wchar_t GXWCHAR;    // wc,   16-bit UNICODE character

//#include "stdafx.h"
#include <GrapX.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXKernel.h"
#include <stdarg.h>
#include <GrapX/WineComm.h>

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "wine/winuser16.h"
//#include "controls.h"
//#include "user_private.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(static);

static void STATIC_PaintOwnerDrawfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintTextfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintRectfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintIconfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintBitmapfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintEnhMetafn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
static void STATIC_PaintEtchedfn( GXHWND hwnd, GXHDC hdc, GXDWORD style );
GXLRESULT GXCALLBACK StaticWndProcA( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam );
GXLRESULT GXCALLBACK StaticWndProcW( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam );

static GXCOLORREF color_3dshadow, color_3ddkshadow, color_3dhighlight;

/* offsets for GetWindowLong for static private information */
#define HFONT_GWL_OFFSET    0
#define HICON_GWL_OFFSET    (sizeof(GXHFONT))
#define STATIC_EXTRA_BYTES  (HICON_GWL_OFFSET + sizeof(GXHICON))

typedef void (*pfPaint)( GXHWND hwnd, GXHDC hdc, GXDWORD style );

static const pfPaint staticPaintFunc[GXSS_TYPEMASK+1] =
{
  STATIC_PaintTextfn,      /* GXSS_LEFT */
  STATIC_PaintTextfn,      /* GXSS_CENTER */
  STATIC_PaintTextfn,      /* GXSS_RIGHT */
  STATIC_PaintIconfn,      /* GXSS_ICON */
  STATIC_PaintRectfn,      /* GXSS_BLACKRECT */
  STATIC_PaintRectfn,      /* GXSS_GRAYRECT */
  STATIC_PaintRectfn,      /* GXSS_WHITERECT */
  STATIC_PaintRectfn,      /* GXSS_BLACKFRAME */
  STATIC_PaintRectfn,      /* GXSS_GRAYFRAME */
  STATIC_PaintRectfn,      /* GXSS_WHITEFRAME */
  NULL,                    /* GXSS_USERITEM */
  STATIC_PaintTextfn,      /* GXSS_SIMPLE */
  STATIC_PaintTextfn,      /* GXSS_LEFTNOWORDWRAP */
  STATIC_PaintOwnerDrawfn, /* GXSS_OWNERDRAW */
  STATIC_PaintBitmapfn,    /* GXSS_BITMAP */
  STATIC_PaintEnhMetafn,   /* GXSS_ENHMETAFILE */
  STATIC_PaintEtchedfn,    /* GXSS_ETCHEDHORZ */
  STATIC_PaintEtchedfn,    /* GXSS_ETCHEDVERT */
  STATIC_PaintEtchedfn,    /* GXSS_ETCHEDFRAME */
};


/*********************************************************************
* static class descriptor
*/
static const GXWCHAR staticW[] = {'S','t','a','t','i','c',0};
//const struct builtin_class_descr STATIC_builtin_class =
//{
//    staticW,             /* name */
//    CS_DBLCLKS | CS_PARENTDC, /* style  */
//    StaticWndProcA,      /* procA */
//    StaticWndProcW,      /* procW */
//    STATIC_EXTRA_BYTES,  /* extra */
//    IDC_ARROW,           /* cursor */
//    0                    /* brush */
//};

GXWNDCLASSEX WndClassEx_MyStatic = { sizeof(GXWNDCLASSEX), GXCS_DBLCLKS | GXCS_PARENTDC, StaticWndProcW, 0L, STATIC_EXTRA_BYTES,
(GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_ARROW), NULL, NULL,
GXWE_STATICW, NULL };

static GXBOOL GetIconInfo2(GXHICON hIcon, CURSORICONINFO *lpIconInfo)
{
  GXICONINFO stInfo;
  GXBITMAP  stBmp;
  if(gxGetIconInfo(hIcon,&stInfo) == FALSE)
  {
    return FALSE;
  }
  else
  {
    if(gxGetObject((GXHGDIOBJ)stInfo.hbmColor, sizeof(stBmp), &stBmp) == 0)
      return FALSE;
    else
    {
      lpIconInfo->ptHotSpot.x = stInfo.xHotspot;
      lpIconInfo->ptHotSpot.y = stInfo.yHotspot;
      lpIconInfo->nWidth = (GXWORD)stBmp.bmHeight;
      lpIconInfo->nHeight = (GXWORD)stBmp.bmWidth;
      lpIconInfo->nWidthBytes = 0;
      lpIconInfo->bPlanes = (GXBYTE)stBmp.bmPlanes;
      lpIconInfo->bBitsPerPixel = (GXBYTE)stBmp.bmBitsPixel;
      return TRUE;
    }
  }
}

static void setup_clipping(GXHWND hwnd, GXHDC hdc, GXHRGN *orig)
{
  GXRECT rc;
  GXHRGN hrgn;

  /* Native control has always a clipping region set (this may be because
  * builtin controls uses CS_PARENTDC) and an application depends on it
  */
  hrgn = gxCreateRectRgn(0, 0, 1, 1);
  if (gxGetClipRgn(hdc, hrgn) != 1)
  {
    gxDeleteObject(hrgn);
    *orig = NULL;
  } else
    *orig = hrgn;

  gxGetClientRect(hwnd, &rc);
  gxDPtoLP(hdc, (GXPOINT *)&rc, 2);
  gxIntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
}

static void restore_clipping(GXHDC hdc, GXHRGN hrgn)
{
  gxSelectClipRgn(hdc, hrgn);
  if (hrgn != NULL)
    gxDeleteObject(hrgn);
}

/***********************************************************************
*           STATIC_SetIcon
*
* Set the icon for an GXSS_ICON control.
*/
static GXHICON STATIC_SetIcon( GXHWND hwnd, GXHICON hicon, GXDWORD style )
{
  GXHICON prevIcon;
  //CURSORICONINFO * info;
  CURSORICONINFO  info;

  if ((style & GXSS_TYPEMASK) != GXSS_ICON) return 0;
  //info = hicon?(CURSORICONINFO *) GlobalLock16(HICON_16(hicon)):NULL;
  //if (hicon && !info) {
  //    WARN("hicon != 0, but info == 0\n");
  //    return 0;
  //}
  if(GetIconInfo2(hicon, &info) == FALSE)
  {
    return 0;
  }
  prevIcon = (GXHICON)gxSetWindowLongPtrW( hwnd, HICON_GWL_OFFSET, (GXLONG_PTR)hicon );
  if (hicon && !(style & GXSS_CENTERIMAGE) && !(style & GXSS_REALSIZECONTROL))
  {
    /* Windows currently doesn't implement GXSS_RIGHTJUST */
    /*
    if ((style & GXSS_RIGHTJUST) != 0)
    {
    GXRECT wr;
    GetWindowRect(hwnd, &wr);
    SetWindowPos( hwnd, 0, wr.right - info->nWidth, wr.bottom - info->nHeight,
    info->nWidth, info->nHeight, GXSWP_NOACTIVATE | GXSWP_NOZORDER );
    }
    else */
    {
      gxSetWindowPos( hwnd, 0, 0, 0, info.nWidth, info.nHeight,
        GXSWP_NOACTIVATE | GXSWP_NOMOVE | GXSWP_NOZORDER );
    }
  }
  //if (info) GlobalUnlock16(HICON_16(hicon));
  return prevIcon;
}

/***********************************************************************
*           STATIC_SetBitmap
*
* Set the bitmap for an GXSS_BITMAP control.
*/
static GXHBITMAP STATIC_SetBitmap( GXHWND hwnd, GXHBITMAP hBitmap, GXDWORD style )
{
  GXHBITMAP hOldBitmap;

  if ((style & GXSS_TYPEMASK) != GXSS_BITMAP) return 0;
  if (hBitmap && gxGetObjectType(hBitmap) != GXOBJ_BITMAP) {
    WARN("hBitmap != 0, but it's not a bitmap\n");
    return 0;
  }
  hOldBitmap = (GXHBITMAP)gxSetWindowLongPtrW( hwnd, HICON_GWL_OFFSET, (GXLONG_PTR)hBitmap );
  if (hBitmap && !(style & GXSS_CENTERIMAGE) && !(style & GXSS_REALSIZECONTROL))
  {
    GXBITMAP bm;
    gxGetObjectW(hBitmap, sizeof(bm), &bm);
    /* Windows currently doesn't implement GXSS_RIGHTJUST */
    /*
    if ((style & GXSS_RIGHTJUST) != 0)
    {
    GXRECT wr;
    GetWindowRect(hwnd, &wr);
    SetWindowPos( hwnd, 0, wr.right - bm.bmWidth, wr.bottom - bm.bmHeight,
    bm.bmWidth, bm.bmHeight, GXSWP_NOACTIVATE | GXSWP_NOZORDER );
    }
    else */
    {
      gxSetWindowPos( hwnd, 0, 0, 0, bm.bmWidth, bm.bmHeight,
        GXSWP_NOACTIVATE | GXSWP_NOMOVE | GXSWP_NOZORDER );
    }

  }
  return hOldBitmap;
}

/***********************************************************************
*           STATIC_SetEnhMetaFile
*
* Set the enhanced metafile for an GXSS_ENHMETAFILE control.
*/
static GXHENHMETAFILE STATIC_SetEnhMetaFile( GXHWND hwnd, GXHENHMETAFILE hEnhMetaFile, GXDWORD style )
{
  if ((style & GXSS_TYPEMASK) != GXSS_ENHMETAFILE) return 0;
  if (hEnhMetaFile && gxGetObjectType(hEnhMetaFile) != GXOBJ_ENHMETAFILE) {
    WARN("hEnhMetaFile != 0, but it's not an enhanced metafile\n");
    return 0;
  }
  return (GXHENHMETAFILE)gxSetWindowLongPtrW( hwnd, HICON_GWL_OFFSET, (GXLONG_PTR)hEnhMetaFile );
}

/***********************************************************************
*           STATIC_GetImage
*
* Gets the bitmap for an GXSS_BITMAP control, the icon/cursor for an
* GXSS_ICON control or the enhanced metafile for an GXSS_ENHMETAFILE control.
*/
static GXHANDLE STATIC_GetImage( GXHWND hwnd, GXWPARAM wParam, GXDWORD style )
{
  switch(style & GXSS_TYPEMASK)
  {
  case GXSS_ICON:
    if ((wParam != GXIMAGE_ICON) &&
      (wParam != GXIMAGE_CURSOR)) return NULL;
    break;
  case GXSS_BITMAP:
    if (wParam != GXIMAGE_BITMAP) return NULL;
    break;
  case GXSS_ENHMETAFILE:
    if (wParam != GXIMAGE_ENHMETAFILE) return NULL;
    break;
  default:
    return NULL;
  }
  return (GXHANDLE)gxGetWindowLongPtrW( hwnd, HICON_GWL_OFFSET );
}

/***********************************************************************
*           STATIC_LoadIconA
*
* Load the icon for an GXSS_ICON control.
*/
static GXHICON STATIC_LoadIconA( GXHWND hwnd, GXLPCSTR name, GXDWORD style )
{
  GXHINSTANCE hInstance = (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GXGWLP_HINSTANCE );
  if ((style & GXSS_REALSIZEIMAGE) != 0)
  {
    return (GXHICON)gxLoadImageA(hInstance, name, GXIMAGE_ICON, 0, 0, GXLR_SHARED);
  }
  else
  {
    GXHICON hicon = gxLoadIconA( hInstance, name );
    if (!hicon) hicon = gxLoadCursorA( hInstance, name );
    if (!hicon) hicon = gxLoadIconA( 0, name );
    /* Windows doesn't try to load a standard cursor,
    probably because most IDs for standard cursors conflict
    with the IDs for standard icons anyway */
    return hicon;
  }
}

/***********************************************************************
*           STATIC_LoadIconW
*
* Load the icon for an GXSS_ICON control.
*/
static GXHICON STATIC_LoadIconW( GXHWND hwnd, GXLPCWSTR name, GXDWORD style )
{
  GXHINSTANCE hInstance = (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GXGWLP_HINSTANCE );
  if ((style & GXSS_REALSIZEIMAGE) != 0)
  {
    return (GXHICON)gxLoadImage(hInstance, name, GXIMAGE_ICON, 0, 0, GXLR_SHARED);
  }
  else
  {
    GXHICON hicon = gxLoadIconW( hInstance, name );
    if (!hicon) hicon = gxLoadCursor( hInstance, name );
    if (!hicon) hicon = gxLoadIconW( 0, name );
    /* Windows doesn't try to load a standard cursor,
    probably because most IDs for standard cursors conflict
    with the IDs for standard icons anyway */
    return hicon;
  }
}

/***********************************************************************
*           STATIC_LoadBitmapA
*
* Load the bitmap for an GXSS_BITMAP control.
*/
static GXHBITMAP STATIC_LoadBitmapA( GXHWND hwnd, GXLPCSTR name )
{
  GXHINSTANCE hInstance = (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GXGWLP_HINSTANCE );
  /* Windows doesn't try to load OEM Bitmaps (hInstance == NULL) */
  return gxLoadBitmapA( hInstance, name );
}

/***********************************************************************
*           STATIC_LoadBitmapW
*
* Load the bitmap for an GXSS_BITMAP control.
*/
static GXHBITMAP STATIC_LoadBitmapW( GXHWND hwnd, GXLPCWSTR name )
{
  GXHINSTANCE hInstance = (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GXGWLP_HINSTANCE );
  /* Windows doesn't try to load OEM Bitmaps (hInstance == NULL) */
  return gxLoadBitmapW( hInstance, name );
}

/***********************************************************************
*           STATIC_TryPaintFcn
*
* Try to immediately paint the control.
*/
static GXVOID STATIC_TryPaintFcn(GXHWND hwnd, GXLONG full_style)
{
  GXLONG style = full_style & GXSS_TYPEMASK;
  GXRECT rc;

  gxGetClientRect( hwnd, &rc );
  if (!gxIsRectEmpty((GXLPRECT)&rc) && gxIsWindowVisible(hwnd) && staticPaintFunc[style])
  {
    GXHDC hdc;
    GXHRGN hOrigClipping;

    hdc = gxGetDC( hwnd );
    setup_clipping(hwnd, hdc, &hOrigClipping);
    (staticPaintFunc[style])( hwnd, hdc, full_style );
    restore_clipping(hdc, hOrigClipping);
    gxReleaseDC( hwnd, hdc );
  }
}

static GXHBRUSH STATIC_SendWmCtlColorStatic(GXHWND hwnd, GXHDC hdc)
{
  GXHBRUSH hBrush;
  GXHWND parent = gxGetParent(hwnd);

  if (!parent) parent = hwnd;
  hBrush = (GXHBRUSH) gxSendMessageW( parent,
    GXWM_CTLCOLORSTATIC, (GXWPARAM)hdc, (GXLPARAM)hwnd );
  if (!hBrush) /* did the app forget to call DefWindowProc ? */
  {
    /* FIXME: DefWindowProc should return different colors if a
    manifest is present */
    hBrush = (GXHBRUSH)gxDefWindowProcW( parent, GXWM_CTLCOLORSTATIC,
      (GXWPARAM)hdc, (GXLPARAM)hwnd);
  }
  return hBrush;
}

static GXVOID STATIC_InitColours()
{
  color_3ddkshadow  = gxGetSysColor(GXCOLOR_3DDKSHADOW);
  color_3dshadow    = gxGetSysColor(GXCOLOR_3DSHADOW);
  color_3dhighlight = gxGetSysColor(GXCOLOR_3DHIGHLIGHT);
}

/***********************************************************************
*           hasTextStyle
*
* Tests if the control displays text.
*/
static GXBOOL hasTextStyle( GXDWORD style )
{
  switch(style & GXSS_TYPEMASK)
  {
  case GXSS_SIMPLE:
  case GXSS_LEFT:
  case GXSS_LEFTNOWORDWRAP:
  case GXSS_CENTER:
  case GXSS_RIGHT:
  case GXSS_OWNERDRAW:
    return TRUE;
  }

  return FALSE;
}

/***********************************************************************
*           StaticWndProc_common
*/
static GXLRESULT StaticWndProc_common( GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam,
                  GXLPARAM lParam, GXBOOL unicode )
{
  GXLRESULT lResult = 0;
  GXLONG full_style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXLONG style = full_style & GXSS_TYPEMASK;

  switch (uMsg)
  {
  case GXWM_CREATE:
    if (style < 0L || style > GXSS_TYPEMASK)
    {
      ERR("Unknown style 0x%02x\n", style );
      return -1;
    }
    STATIC_InitColours();
    break;

  case GXWM_NCDESTROY:
    if (style == GXSS_ICON) {
      /*
      * FIXME
      *           DestroyIcon32( STATIC_SetIcon( wndPtr, 0 ) );
      *
      * We don't want to do this yet because DestroyIcon32 is broken. If the icon
      * had already been loaded by the application the last thing we want to do is
      * GlobalFree16 the handle.
      */
      break;
    }
    else return unicode ? gxDefWindowProcW(hwnd, uMsg, wParam, lParam) :
      gxDefWindowProcA(hwnd, uMsg, wParam, lParam);

  case GXWM_ERASEBKGND:
    /* do all painting in WM_PAINT like Windows does */
    return 1;

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    {
      GXPAINTSTRUCT ps;
      GXHDC hdc = wParam ? (GXHDC)wParam : gxBeginPaint(hwnd, &ps);
      if (staticPaintFunc[style])
      {
        //GXHRGN hOrigClipping;
        //setup_clipping(hwnd, hdc, &hOrigClipping);
        (staticPaintFunc[style])( hwnd, hdc, full_style );
        //restore_clipping(hdc, hOrigClipping);
      }
      if (!wParam) gxEndPaint(hwnd, &ps);
    }
    break;

  case GXWM_ENABLE:
    STATIC_TryPaintFcn( hwnd, full_style );
    if (full_style & GXSS_NOTIFY) {
      if (wParam) {
        gxSendMessageW( gxGetParent(hwnd), GXWM_COMMAND,
          GXMAKEWPARAM( gxGetWindowLongPtrW(hwnd,GXGWLP_ID), GXSTN_ENABLE ), (GXLPARAM)hwnd);
      }
      else {
        gxSendMessageW( gxGetParent(hwnd), GXWM_COMMAND,
          GXMAKEWPARAM( gxGetWindowLongPtrW(hwnd,GXGWLP_ID), GXSTN_DISABLE ), (GXLPARAM)hwnd);
      }
    }
    break;

  case GXWM_SYSCOLORCHANGE:
    STATIC_InitColours();
    STATIC_TryPaintFcn( hwnd, full_style );
    break;

  case GXWM_NCCREATE:
    {
      GXLPCSTR textA;
      GXLPCWSTR textW;

      if (full_style & GXSS_SUNKEN)
        gxSetWindowLongW( hwnd, GXGWL_EXSTYLE,
        gxGetWindowLongW( hwnd, GXGWL_EXSTYLE ) | GXWS_EX_STATICEDGE );

      if(unicode)
      {
        textA = NULL;
        textW = ((GXLPCREATESTRUCTW)lParam)->lpszName;
      }
      else
      {
        textA = ((GXLPCREATESTRUCTA)lParam)->lpszName;
        textW = NULL;
      }

      switch (style) {
  case GXSS_ICON:
    {
      GXHICON hIcon;
      if(unicode)
        hIcon = STATIC_LoadIconW(hwnd, textW, full_style);
      else
        hIcon = STATIC_LoadIconA(hwnd, textA, full_style);
      STATIC_SetIcon(hwnd, hIcon, full_style);
    }
    break;
  case GXSS_BITMAP:
    {
      GXHBITMAP hBitmap;
      if(unicode)
        hBitmap = STATIC_LoadBitmapW(hwnd, textW);
      else
        hBitmap = STATIC_LoadBitmapA(hwnd, textA);
      STATIC_SetBitmap(hwnd, hBitmap, full_style);
    }
    break;
      }
      /* GXSS_ENHMETAFILE: Despite what MSDN says, Windows does not load
      the enhanced metafile that was specified as the window text. */
    }
    return unicode ? gxDefWindowProcW(hwnd, uMsg, wParam, lParam) :
      gxDefWindowProcA(hwnd, uMsg, wParam, lParam);

  case GXWM_SETTEXT:
    if (hasTextStyle( full_style ))
    {
      if (GXHIWORD(lParam))
      {
        if(unicode)
          lResult = gxDefWindowProcW( hwnd, uMsg, wParam, lParam );
        else
          lResult = gxDefWindowProcA( hwnd, uMsg, wParam, lParam );
        STATIC_TryPaintFcn( hwnd, full_style );
      }
    }
    break;

  case GXWM_SETFONT:
    if (hasTextStyle( full_style ))
    {
      gxSetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET, wParam );
      if (GXLOWORD(lParam))
        gxRedrawWindow( hwnd, NULL, 0, GXRDW_INVALIDATE | GXRDW_ERASE | GXRDW_UPDATENOW | GXRDW_ALLCHILDREN );
    }
    break;

  case GXWM_GETFONT:
    return gxGetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET );

  case GXWM_NCHITTEST:
    if (full_style & GXSS_NOTIFY)
      return GXHTCLIENT;
    else
      return GXHTTRANSPARENT;

  case GXWM_GETDLGCODE:
    return GXDLGC_STATIC;

  case GXWM_LBUTTONDOWN:
  case GXWM_NCLBUTTONDOWN:
    if (full_style & GXSS_NOTIFY)
      gxSendMessageW( gxGetParent(hwnd), GXWM_COMMAND,
      GXMAKEWPARAM( gxGetWindowLongPtrW(hwnd,GXGWLP_ID), GXSTN_CLICKED ), (GXLPARAM)hwnd);
    return 0;

  case GXWM_LBUTTONDBLCLK:
  case GXWM_NCLBUTTONDBLCLK:
    if (full_style & GXSS_NOTIFY)
      gxSendMessageW( gxGetParent(hwnd), GXWM_COMMAND,
      GXMAKEWPARAM( gxGetWindowLongPtrW(hwnd,GXGWLP_ID), GXSTN_DBLCLK ), (GXLPARAM)hwnd);
    return 0;

  case GXSTM_GETIMAGE:
    return (GXLRESULT)STATIC_GetImage( hwnd, wParam, full_style );

    //case STM_GETICON16:
  case GXSTM_GETICON:
    return (GXLRESULT)STATIC_GetImage( hwnd, GXIMAGE_ICON, full_style );

  case GXSTM_SETIMAGE:
    switch(wParam) {
  case GXIMAGE_BITMAP:
    lResult = (GXLRESULT)STATIC_SetBitmap( hwnd, (GXHBITMAP)lParam, full_style );
    break;
  case GXIMAGE_ENHMETAFILE:
    lResult = (GXLRESULT)STATIC_SetEnhMetaFile( hwnd, (GXHENHMETAFILE)lParam, full_style );
    break;
  case GXIMAGE_ICON:
  case GXIMAGE_CURSOR:
    lResult = (GXLRESULT)STATIC_SetIcon( hwnd, (GXHICON)lParam, full_style );
    break;
  default:
    FIXME("STM_SETIMAGE: Unhandled type %lx\n", wParam);
    break;
    }
    STATIC_TryPaintFcn( hwnd, full_style );
    break;

    //case STM_SETICON16:
  case GXSTM_SETICON:
    lResult = (GXLRESULT)STATIC_SetIcon( hwnd, (GXHICON)wParam, full_style );
    STATIC_TryPaintFcn( hwnd, full_style );
    break;

  default:
    return unicode ? gxDefWindowProcW(hwnd, uMsg, wParam, lParam) :
      gxDefWindowProcA(hwnd, uMsg, wParam, lParam);
  }
  return lResult;
}

/***********************************************************************
*           StaticWndProcA
*/
GXLRESULT GXCALLBACK StaticWndProcA( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam )
{
  //if (!gxIsWindow( hWnd )) return 0;
  return StaticWndProc_common(hWnd, uMsg, wParam, lParam, FALSE);
}

/***********************************************************************
*           StaticWndProcW
*/
GXLRESULT GXCALLBACK StaticWndProcW( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam )
{
  //if (!gxIsWindow( hWnd )) return 0;
  return StaticWndProc_common(hWnd, uMsg, wParam, lParam, TRUE);
}

static void STATIC_PaintOwnerDrawfn( GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXDRAWITEMSTRUCT dis;
  GXHFONT font, oldFont = NULL;
  GXUINT id = (GXUINT)gxGetWindowLongPtrW( hwnd, GXGWLP_ID );

  dis.CtlType    = GXODT_STATIC;
  dis.CtlID      = id;
  dis.itemID     = 0;
  dis.itemAction = GXODA_DRAWENTIRE;
  dis.itemState  = gxIsWindowEnabled(hwnd) ? 0 : GXODS_DISABLED;
  dis.hwndItem   = (GXHWND)hwnd;
  dis.hDC        = (GXHDC)hdc;
  dis.itemData   = 0;
  gxGetClientRect( hwnd, (GXLPRECT)&dis.rcItem );

  font = (GXHFONT)gxGetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET );
  if (font) oldFont = (GXHFONT)gxSelectObject( hdc, font );
  gxSendMessageW( gxGetParent(hwnd), GXWM_CTLCOLORSTATIC, (GXWPARAM)hdc, (GXLPARAM)hwnd );
  gxSendMessageW( gxGetParent(hwnd), GXWM_DRAWITEM, id, (GXLPARAM)&dis );
  if (font) gxSelectObject( hdc, oldFont );
}

static void STATIC_PaintTextfn( GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXRECT rc;
  GXHBRUSH hBrush;
  GXHFONT hFont, hOldFont = NULL;
  GXWORD wFormat;
  GXINT len, buf_size;
  GXWCHAR *text;

  gxGetClientRect( hwnd, &rc);

  switch (style & GXSS_TYPEMASK)
  {
  case GXSS_LEFT:
    wFormat = GXDT_LEFT | GXDT_EXPANDTABS | GXDT_WORDBREAK;
    break;

  case GXSS_CENTER:
    wFormat = GXDT_CENTER | GXDT_EXPANDTABS | GXDT_WORDBREAK;
    break;

  case GXSS_RIGHT:
    wFormat = GXDT_RIGHT | GXDT_EXPANDTABS | GXDT_WORDBREAK;
    break;

  case GXSS_SIMPLE:
    wFormat = GXDT_LEFT | GXDT_SINGLELINE;
    break;

  case GXSS_LEFTNOWORDWRAP:
    wFormat = GXDT_LEFT | GXDT_EXPANDTABS;
    break;

  default:
    return;
  }

  if (style & GXSS_NOPREFIX)
    wFormat |= GXDT_NOPREFIX;

  if ((style & GXSS_TYPEMASK) != GXSS_SIMPLE)
  {
    if (style & GXSS_CENTERIMAGE)
      wFormat |= GXDT_SINGLELINE | GXDT_VCENTER;
    if (style & GXSS_EDITCONTROL)
      wFormat |= GXDT_EDITCONTROL;
    if (style & GXSS_ENDELLIPSIS)
      wFormat |= GXDT_SINGLELINE | GXDT_END_ELLIPSIS;
    if (style & GXSS_PATHELLIPSIS)
      wFormat |= GXDT_SINGLELINE | GXDT_PATH_ELLIPSIS;
    if (style & GXSS_WORDELLIPSIS)
      wFormat |= GXDT_SINGLELINE | GXDT_WORD_ELLIPSIS;
  }

  if ((hFont = (GXHFONT)gxGetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET )))
    hOldFont = (GXHFONT)gxSelectObject( hdc, hFont );

  /* GXSS_SIMPLE controls: WM_CTLCOLORSTATIC is sent, but the returned
  brush is not used */
  hBrush = STATIC_SendWmCtlColorStatic(hwnd, hdc);

  if ((style & GXSS_TYPEMASK) != GXSS_SIMPLE)
  {
    gxFillRect( hdc, &rc, hBrush );
    if (!gxIsWindowEnabled(hwnd)) gxSetTextColor(hdc, gxGetSysColor(GXCOLOR_GRAYTEXT));
  }

  buf_size = 256;
  if (!(text = (GXWCHAR*)gxHeapAlloc( gxGetProcessHeap(), 0, buf_size * sizeof(GXWCHAR) )))
    goto no_TextOut;

  while ((len = gxInternalGetWindowText( hwnd, text, buf_size )) == buf_size - 1)
  {
    buf_size *= 2;
    if (!(text = (GXWCHAR*)gxHeapReAlloc( gxGetProcessHeap(), 0, text, buf_size * sizeof(GXWCHAR) )))
      goto no_TextOut;
  }

  if (!len) goto no_TextOut;

  if (((style & GXSS_TYPEMASK) == GXSS_SIMPLE) && (style & GXSS_NOPREFIX))
  {
    /* Windows uses the faster ExtTextOut() to draw the text and
    to paint the whole client rectangle with the text background
    color. Reference: "Static Controls" by Kyle Marsh, 1992 */
    gxExtTextOutW( hdc, rc.left, rc.top, GXETO_CLIPPED | GXETO_OPAQUE,
      &rc, text, len, NULL );
  }
  else
  {
    gxDrawTextW( hdc, text, -1, &rc, wFormat | GDT_GLOW);
  }

no_TextOut:
  gxHeapFree( gxGetProcessHeap(), 0, text );

  if (hFont)
    gxSelectObject( hdc, hOldFont );
}

static void STATIC_PaintRectfn( GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXRECT rc;
  GXHBRUSH hBrush;

  gxGetClientRect( hwnd, &rc);

  /* FIXME: send WM_CTLCOLORSTATIC */
  switch (style & GXSS_TYPEMASK)
  {
  case GXSS_BLACKRECT:
    hBrush = gxCreateSolidBrush(color_3ddkshadow);
    gxFillRect( hdc, &rc, hBrush );
    break;
  case GXSS_GRAYRECT:
    hBrush = gxCreateSolidBrush(color_3dshadow);
    gxFillRect( hdc, &rc, hBrush );
    break;
  case GXSS_WHITERECT:
    hBrush = gxCreateSolidBrush(color_3dhighlight);
    gxFillRect( hdc, &rc, hBrush );
    break;
  case GXSS_BLACKFRAME:
    hBrush = gxCreateSolidBrush(color_3ddkshadow);
    gxFrameRect( hdc, &rc, hBrush );
    break;
  case GXSS_GRAYFRAME:
    hBrush = gxCreateSolidBrush(color_3dshadow);
    gxFrameRect( hdc, &rc, hBrush );
    break;
  case GXSS_WHITEFRAME:
    hBrush = gxCreateSolidBrush(color_3dhighlight);
    gxFrameRect( hdc, &rc, hBrush );
    break;
  default:
    return;
  }
  gxDeleteObject( hBrush );
}


static void STATIC_PaintIconfn( GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXRECT rc, iconRect;
  GXHBRUSH hbrush;
  GXHICON hIcon;
  //CURSORICONINFO * info;
  CURSORICONINFO info;

  gxGetClientRect( hwnd, &rc );
  hbrush = STATIC_SendWmCtlColorStatic(hwnd, hdc);
  hIcon = (GXHICON)gxGetWindowLongPtrW( hwnd, HICON_GWL_OFFSET );
  //info = hIcon ? (CURSORICONINFO *)GlobalLock16(HICON_16(hIcon)) : NULL;

  if (!GetIconInfo2(hIcon, &info) || !hIcon)
  {
    gxFillRect(hdc, &rc, hbrush);
  }
  else
  {
    if (style & GXSS_CENTERIMAGE)
    {
      iconRect.left = (rc.right - rc.left) / 2 - info.nWidth / 2;
      iconRect.top = (rc.bottom - rc.top) / 2 - info.nHeight / 2;
      iconRect.right = iconRect.left + info.nWidth;
      iconRect.bottom = iconRect.top + info.nHeight;
    }
    else
      iconRect = rc;
    gxFillRect( hdc, &rc, hbrush );
    gxDrawIconEx( hdc, iconRect.left, iconRect.top, hIcon, iconRect.right - iconRect.left,
      iconRect.bottom - iconRect.top, 0, NULL, GXDI_NORMAL );
  }
  //if (info) GlobalUnlock16(HICON_16(hIcon));
}

static void STATIC_PaintBitmapfn(GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXHDC hMemDC;
  GXHBITMAP hBitmap, oldbitmap;
  GXHBRUSH hbrush;

  /* message is still sent, even if the returned brush is not used */
  hbrush = STATIC_SendWmCtlColorStatic(hwnd, hdc);

  if ((hBitmap = (GXHBITMAP)gxGetWindowLongPtrW( hwnd, HICON_GWL_OFFSET ))
    && (gxGetObjectType(hBitmap) == GXOBJ_BITMAP)
    && (hMemDC = gxCreateCompatibleDC( hdc )))
  {
    GXBITMAP bm;
    GXRECT rcClient;
    GXLOGBRUSH brush;

    gxGetObjectW(hBitmap, sizeof(bm), &bm);
    oldbitmap = (GXHBITMAP)gxSelectObject(hMemDC, hBitmap);

    /* Set the background color for monochrome bitmaps
    to the color of the background brush */
    if (gxGetObjectW( hbrush, sizeof(brush), &brush ))
    {
      if (brush.lbStyle == GXBS_SOLID)
        gxSetBkColor(hdc, brush.lbColor);
    }
    gxGetClientRect(hwnd, &rcClient);
    if (style & GXSS_CENTERIMAGE)
    {
      GXINT x, y;
      x = (rcClient.right - rcClient.left)/2 - bm.bmWidth/2;
      y = (rcClient.bottom - rcClient.top)/2 - bm.bmHeight/2;
      gxFillRect( hdc, &rcClient, hbrush );
      gxBitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0,
        GXSRCCOPY);
    }
    else
    {
      gxStretchBlt(hdc, 0, 0, rcClient.right - rcClient.left,
        rcClient.bottom - rcClient.top, hMemDC,
        0, 0, bm.bmWidth, bm.bmHeight, GXSRCCOPY);
    }
    gxSelectObject(hMemDC, oldbitmap);
    gxDeleteDC(hMemDC);
  }
  else
  {
    GXRECT rcClient;
    gxGetClientRect( hwnd, &rcClient );
    gxFillRect( hdc, &rcClient, hbrush );
  }
}


static void STATIC_PaintEnhMetafn(GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  //HENHMETAFILE hEnhMetaFile;
  //GXRECT rc;
  //GXHBRUSH hbrush;

  ASSERT(FALSE);

  //     gxGetClientRect(hwnd, &rc);
  //     hbrush = STATIC_SendWmCtlColorStatic(hwnd, hdc);
  //     gxFillRect(hdc, &rc, hbrush);
  //     if ((hEnhMetaFile = (HENHMETAFILE)gxGetWindowLongPtrW( hwnd, HICON_GWL_OFFSET )))
  //     {
  //         /* The control's current font is not selected into the
  //            device context! */
  //         if (gxGetObjectType(hEnhMetaFile) == OBJ_ENHMETAFILE)
  //             PlayEnhMetaFile(hdc, hEnhMetaFile, &rc);
  //     }

}


static void STATIC_PaintEtchedfn( GXHWND hwnd, GXHDC hdc, GXDWORD style )
{
  GXRECT rc;

  /* FIXME: sometimes (not always) sends WM_CTLCOLORSTATIC */
  gxGetClientRect( hwnd, &rc );
  switch (style & GXSS_TYPEMASK)
  {
  case GXSS_ETCHEDHORZ:
    gxDrawEdge(hdc,&rc,GXEDGE_ETCHED,GXBF_TOP|GXBF_BOTTOM);
    break;
  case GXSS_ETCHEDVERT:
    gxDrawEdge(hdc,&rc,GXEDGE_ETCHED,GXBF_LEFT|GXBF_RIGHT);
    break;
  case GXSS_ETCHEDFRAME:
    gxDrawEdge (hdc, &rc, GXEDGE_ETCHED, GXBF_RECT);
    break;
  }
}
#endif // _DEV_DISABLE_UI_CODE