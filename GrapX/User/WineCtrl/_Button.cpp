#ifndef _DEV_DISABLE_UI_CODE
/* File: button.c -- Button type widgets
*
* Copyright (C) 1993 Johannes Ruscheinski
* Copyright (C) 1993 David Metcalfe
* Copyright (C) 1994 Alexandre Julliard
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
* of Comctl32.dll version 6.0 on Oct. 3, 2004, by Dimitrie O. Paun.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
* 
* TODO
*  Styles
*  - GXBS_NOTIFY: is it complete?
*  - GXBS_RIGHTBUTTON: same as GXBS_LEFTTEXT
*  - GXBS_TYPEMASK
*
*  Messages
*  - WM_CHAR: Checks a (manual or automatic) check box on '+' or '=', clears it on '-' key.
*  - WM_SETFOCUS: For (manual or automatic) radio buttons, send the parent window BN_CLICKED
*  - WM_NCCREATE: Turns any GXBS_OWNERDRAW button into a GXBS_PUSHBUTTON button.
*  - WM_SYSKEYUP
*  - BCM_GETIDEALSIZE
*  - BCM_GETIMAGELIST
*  - BCM_GETTEXTMARGIN
*  - BCM_SETIMAGELIST
*  - BCM_SETTEXTMARGIN
*  
*  Notifications
*  - BCN_HOTITEMCHANGE
*  - BN_DISABLE
*  - BN_PUSHED/BN_HILITE
*  + BN_KILLFOCUS: is it OK?
*  - BN_PAINT
*  + BN_SETFOCUS: is it OK?
*  - BN_UNPUSHED/BN_UNHILITE
*  - NM_CUSTOMDRAW
*
*  Structures/Macros/Definitions
*  - BUTTON_IMAGELIST
*  - NMBCHOTITEM
*  - Button_GetIdealSize
*  - Button_GetImageList
*  - Button_GetTextMargin
*  - Button_SetImageList
*  - Button_SetTextMargin
*/
#include <GrapX.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXImm.h"
#include "GrapX/GXKernel.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.h>

#define OEMRESOURCE

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "wine/winuser16.h"
//#include "controls.h"
//#include "user_private.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(button);

/* gxGetWindowLong offsets for window extra information */
#define STATE_GWL_OFFSET  0
#define HFONT_GWL_OFFSET  (sizeof(GXLONG))
#define HIMAGE_GWL_OFFSET (HFONT_GWL_OFFSET+sizeof(GXHFONT))
#define NB_EXTRA_BYTES    (HIMAGE_GWL_OFFSET+sizeof(GXHANDLE))

/* Button state values */
#define BUTTON_UNCHECKED       0x00
#define BUTTON_CHECKED         0x01
#define BUTTON_3STATE          0x02
#define BUTTON_HIGHLIGHTED     0x04
#define BUTTON_HASFOCUS        0x08
#define BUTTON_NSTATES         0x0F
/* undocumented flags */
#define BUTTON_BTNPRESSED      0x40
#define BUTTON_UNKNOWN2        0x20
#define BUTTON_UNKNOWN3        0x10

#define BUTTON_NOTIFY_PARENT(hWnd, code) \
  do { /* Notify parent which has created this button control */ \
  TRACE("notification " #code " sent to hwnd=%d\n", gxGetParent(hWnd)); \
  gxSendMessageW(gxGetParent(hWnd), GXWM_COMMAND, \
  GXMAKEWPARAM(gxGetWindowLongPtrW((hWnd),GXGWLP_ID), (code)), \
  (GXLPARAM)(hWnd)); \
  } while(0)

static GXUINT BUTTON_CalcLabelRect( GXHWND hwnd, GXHDC hdc, GXRECT *rc );
static void PB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action );
static void CB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action );
static void GB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action );
static void UB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action );
static void OB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action );
static void BUTTON_CheckAutoRadioButton( GXHWND hwnd );
GXLRESULT GXDLLAPI ButtonWndProcA( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam );
GXLRESULT GXDLLAPI ButtonWndProcW( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam );

#define MAX_BTN_TYPE  12

static const GXWORD maxCheckState[MAX_BTN_TYPE] =
{
  BUTTON_UNCHECKED,   /* GXBS_PUSHBUTTON */
  BUTTON_UNCHECKED,   /* GXBS_DEFPUSHBUTTON */
  BUTTON_CHECKED,     /* GXBS_CHECKBOX */
  BUTTON_CHECKED,     /* GXBS_AUTOCHECKBOX */
  BUTTON_CHECKED,     /* GXBS_RADIOBUTTON */
  BUTTON_3STATE,      /* GXBS_3STATE */
  BUTTON_3STATE,      /* GXBS_AUTO3STATE */
  BUTTON_UNCHECKED,   /* GXBS_GROUPBOX */
  BUTTON_UNCHECKED,   /* GXBS_USERBUTTON */
  BUTTON_CHECKED,     /* GXBS_AUTORADIOBUTTON */
  BUTTON_UNCHECKED,   /* Not defined */
  BUTTON_UNCHECKED    /* GXBS_OWNERDRAW */
};

typedef void (*pfPaint)( GXHWND hwnd, GXHDC hdc, GXUINT action );

static const pfPaint btnPaintFunc[MAX_BTN_TYPE] =
{
  PB_Paint,    /* GXBS_PUSHBUTTON */
  PB_Paint,    /* GXBS_DEFPUSHBUTTON */
  CB_Paint,    /* GXBS_CHECKBOX */
  CB_Paint,    /* GXBS_AUTOCHECKBOX */
  CB_Paint,    /* GXBS_RADIOBUTTON */
  CB_Paint,    /* GXBS_3STATE */
  CB_Paint,    /* GXBS_AUTO3STATE */
  GB_Paint,    /* GXBS_GROUPBOX */
  UB_Paint,    /* GXBS_USERBUTTON */
  CB_Paint,    /* GXBS_AUTORADIOBUTTON */
  NULL,        /* Not defined */
  OB_Paint     /* GXBS_OWNERDRAW */
};

static GXHBITMAP hbitmapCheckBoxes = 0;
static GXWORD checkBoxWidth = 0, checkBoxHeight = 0;


/*********************************************************************
* button class descriptor
*/
static const GXWCHAR buttonW[] = {'B','u','t','t','o','n',0};
//const struct builtin_class_descr BUTTON_builtin_class =
//{
//    buttonW,             /* name */
//    GXCS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_PARENTDC, /* style  */
//    ButtonWndProcA,      /* procA */
//    ButtonWndProcW,      /* procW */
//    NB_EXTRA_BYTES,      /* extra */
//    IDC_ARROW,           /* cursor */
//    0                    /* brush */
//};

GXWNDCLASSEX WndClassEx_MyButton = { sizeof(GXWNDCLASSEX), GXCS_DBLCLKS, (GXWNDPROC)ButtonWndProcW, 0L, NB_EXTRA_BYTES,
(GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_ARROW), NULL, NULL,
GXWE_BUTTONW, NULL };

static inline GXLONG get_button_state( GXHWND hwnd )
{
  return (GXLONG)gxGetWindowLongW( hwnd, STATE_GWL_OFFSET );
}

static inline void set_button_state( GXHWND hwnd, GXLONG state )
{
  gxSetWindowLongW( hwnd, STATE_GWL_OFFSET, state );
}

static inline GXHFONT get_button_font( GXHWND hwnd )
{
  return (GXHFONT)gxGetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET );
}

static inline void set_button_font( GXHWND hwnd, GXHFONT font )
{
  gxSetWindowLongPtrW( hwnd, HFONT_GWL_OFFSET, (GXLONG_PTR)font );
}

static inline GXUINT get_button_type( GXLONG window_style )
{
  return (window_style & 0x0f);
}

/* paint a button of any type */
static inline void paint_button( GXHWND hwnd, GXLONG style, GXUINT action )
{
  if (btnPaintFunc[style] && gxIsWindowVisible(hwnd))
  {
    GXHDC hdc = gxGetDC( hwnd );
    btnPaintFunc[style]( hwnd, hdc, action );
    gxReleaseDC( hwnd, hdc );
  }
}

/* retrieve the button text; returned buffer must be freed by caller */
static inline GXWCHAR *get_button_text( GXHWND hwnd )
{
  GXINT len = 512;
  GXWCHAR *buffer = (GXWCHAR *)gxHeapAlloc( gxGetProcessHeap(), 0, (len + 1) * sizeof(GXWCHAR) );
  if (buffer) gxInternalGetWindowText( hwnd, buffer, len + 1 );
  return buffer;
}

/***********************************************************************
*           ButtonWndProc_common
*/
static GXLRESULT GXAPI ButtonWndProc_common(GXHWND hWnd, GXUINT uMsg,
                       GXWPARAM wParam, GXLPARAM lParam, GXBOOL unicode )
{
  GXRECT rect;
  GXPOINT pt;
  GXLONG style = gxGetWindowLongW( hWnd, GXGWL_STYLE );
  GXUINT btn_type = get_button_type( style );
  GXLONG state;
  GXHANDLE oldHbitmap;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);

  switch (uMsg)
  {
  case GXWM_GETDLGCODE:
    switch(btn_type)
    {
    case GXBS_USERBUTTON:
    case GXBS_PUSHBUTTON:      return GXDLGC_BUTTON | GXDLGC_UNDEFPUSHBUTTON;
    case GXBS_DEFPUSHBUTTON:   return GXDLGC_BUTTON | GXDLGC_DEFPUSHBUTTON;
    case GXBS_RADIOBUTTON:
    case GXBS_AUTORADIOBUTTON: return GXDLGC_BUTTON | GXDLGC_RADIOBUTTON;
    case GXBS_GROUPBOX:        return GXDLGC_STATIC;
    default:                 return GXDLGC_BUTTON;
    }

  case GXWM_ENABLE:
    paint_button( hWnd, btn_type, GXODA_DRAWENTIRE );
    break;

  case GXWM_CREATE:
    if (!hbitmapCheckBoxes)
    {
      GXBITMAP bmp;
      hbitmapCheckBoxes = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_CHECKBOXES));
      gxGetObjectW( hbitmapCheckBoxes, sizeof(bmp), &bmp );
      checkBoxWidth  = bmp.bmWidth / 4;
      checkBoxHeight = bmp.bmHeight / 3;
    }
    if (btn_type >= MAX_BTN_TYPE)
      return -1; /* abort */
    set_button_state( hWnd, BUTTON_UNCHECKED );
    return 0;

  case GXWM_ERASEBKGND:
    if (btn_type == GXBS_OWNERDRAW)
    {
      GXHDC hdc = (GXHDC)wParam;
      GXRECT rc;
      GXHBRUSH hBrush;
      GXHWND parent = gxGetParent(hWnd);
      if (!parent) parent = hWnd;
      hBrush = (GXHBRUSH)gxSendMessageW(parent, GXWM_CTLCOLORBTN, (GXWPARAM)hdc, (GXLPARAM)hWnd);
      if (!hBrush) /* did the app forget to call defwindowproc ? */
        hBrush = (GXHBRUSH)gxDefWindowProcW(parent, GXWM_CTLCOLORBTN,
        (GXWPARAM)hdc, (GXLPARAM)hWnd);
      gxGetClientRect(hWnd, &rc);
      gxFillRect(hdc, &rc, hBrush);
    }
    return 1;

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    if (btnPaintFunc[btn_type])
    {
      GXPAINTSTRUCT ps;
      GXHDC hdc = wParam ? (GXHDC)wParam : gxBeginPaint( hWnd, &ps );
      int nOldMode = gxSetBkMode( hdc, GXOPAQUE );
      (btnPaintFunc[btn_type])( hWnd, hdc, GXODA_DRAWENTIRE );
      gxSetBkMode(hdc, nOldMode); /*  reset painting mode */
      if( !wParam ) gxEndPaint( hWnd, &ps );
    }
    break;

  case GXWM_KEYDOWN:
    if (wParam == GXVK_SPACE)
    {
      gxSendMessageW( hWnd, GXBM_SETSTATE, TRUE, 0 );
      set_button_state( hWnd, get_button_state( hWnd ) | BUTTON_BTNPRESSED );
    }
    break;

  case GXWM_LBUTTONDBLCLK:
    if(style & GXBS_NOTIFY ||
      btn_type == GXBS_RADIOBUTTON ||
      btn_type == GXBS_USERBUTTON ||
      btn_type == GXBS_OWNERDRAW)
    {
      BUTTON_NOTIFY_PARENT(hWnd, GXBN_DOUBLECLICKED);
      break;
    }
    /* fall through */
  case GXWM_LBUTTONDOWN:
    gxSetCapture( hWnd );
    gxSetFocus( hWnd );
    set_button_state( hWnd, get_button_state( hWnd ) | BUTTON_BTNPRESSED );
    gxSendMessageW( hWnd, GXBM_SETSTATE, TRUE, 0 );
    break;

  case GXWM_KEYUP:
    if (wParam != GXVK_SPACE)
      break;
    /* fall through */
  case GXWM_LBUTTONUP:
    state = get_button_state( hWnd );
    if (!(state & BUTTON_BTNPRESSED)) break;
    state &= BUTTON_NSTATES;
    set_button_state( hWnd, state );
    if (!(state & BUTTON_HIGHLIGHTED))
    {
      gxReleaseCapture();
      break;
    }
    //MessageBoxW(0,0,0,0);
    gxSendMessageW( hWnd, GXBM_SETSTATE, FALSE, 0 );
    gxReleaseCapture();
    gxGetClientRect( hWnd, &rect );
    if (uMsg == GXWM_KEYUP || gxPtInRect( &rect, pt ))
    {
      state = get_button_state( hWnd );
      switch(btn_type)
      {
      case GXBS_AUTOCHECKBOX:
        gxSendMessageW( hWnd, GXBM_SETCHECK, !(state & BUTTON_CHECKED), 0 );
        break;
      case GXBS_AUTORADIOBUTTON:
        gxSendMessageW( hWnd, GXBM_SETCHECK, TRUE, 0 );
        break;
      case GXBS_AUTO3STATE:
        gxSendMessageW( hWnd, GXBM_SETCHECK,
          (state & BUTTON_3STATE) ? 0 : ((state & 3) + 1), 0 );
        break;
      }
      BUTTON_NOTIFY_PARENT(hWnd, GXBN_CLICKED);
    }
    break;

  case GXWM_CAPTURECHANGED:
    TRACE("GXWM_CAPTURECHANGED %d\n", hWnd);
    state = get_button_state( hWnd );
    if (state & BUTTON_BTNPRESSED)
    {
      state &= BUTTON_NSTATES;
      set_button_state( hWnd, state );
      if (state & BUTTON_HIGHLIGHTED) gxSendMessageW( hWnd, GXBM_SETSTATE, FALSE, 0 );
    }
    break;

  case GXWM_MOUSEMOVE:
    if ((wParam & GXMK_LBUTTON) && gxGetCapture() == hWnd)
    {
      gxGetClientRect( hWnd, &rect );
      gxSendMessageW( hWnd, GXBM_SETSTATE, gxPtInRect(&rect, pt), 0 );
    }
    break;

  case GXWM_SETTEXT:
    {
      /* Clear an old text here as Windows does */
      GXHDC hdc = gxGetDC(hWnd);
      GXHBRUSH hbrush;
      GXRECT client, rc;
      GXHWND parent = gxGetParent(hWnd);

      if (!parent) parent = hWnd;
      hbrush = (GXHBRUSH)gxSendMessageW(parent, GXWM_CTLCOLORSTATIC,
        (GXWPARAM)hdc, (GXLPARAM)hWnd);
      if (!hbrush) /* did the app forget to call gxDefWindowProc ? */
        hbrush = (GXHBRUSH)gxDefWindowProcW(parent, GXWM_CTLCOLORSTATIC,
        (GXWPARAM)hdc, (GXLPARAM)hWnd);

      gxGetClientRect(hWnd, &client);
      rc = client;
      BUTTON_CalcLabelRect(hWnd, hdc, &rc);
      /* Clip by client rect bounds */
      if (rc.right > client.right) rc.right = client.right;
      if (rc.bottom > client.bottom) rc.bottom = client.bottom;
      gxFillRect(hdc, &rc, hbrush);
      gxReleaseDC(hWnd, hdc);

      if (unicode) gxDefWindowProcW( hWnd, GXWM_SETTEXT, wParam, lParam );
      else gxDefWindowProcA( hWnd, GXWM_SETTEXT, wParam, lParam );
      if (btn_type == GXBS_GROUPBOX) /* Yes, only for GXBS_GROUPBOX */
        gxInvalidateRect( hWnd, NULL, TRUE );
      else
        paint_button( hWnd, btn_type, GXODA_DRAWENTIRE );
      return 1; /* success. FIXME: check text length */
    }

  case GXWM_SETFONT:
    set_button_font( hWnd, (GXHFONT)wParam );
    if (lParam) gxInvalidateRect(hWnd, NULL, TRUE);
    break;

  case GXWM_GETFONT:
    return (GXLRESULT)get_button_font( hWnd );

  case GXWM_SETFOCUS:
    TRACE("GXWM_SETFOCUS %d\n",hWnd);
    set_button_state( hWnd, get_button_state(hWnd) | BUTTON_HASFOCUS );
    paint_button( hWnd, btn_type, GXODA_FOCUS );
    if (style & GXBS_NOTIFY)
      BUTTON_NOTIFY_PARENT(hWnd, GXBN_SETFOCUS);
    break;

  case GXWM_KILLFOCUS:
    TRACE("GXWM_KILLFOCUS %d\n",hWnd);
    state = get_button_state( hWnd );
    set_button_state( hWnd, state & ~BUTTON_HASFOCUS );
    paint_button( hWnd, btn_type, GXODA_FOCUS );

    if ((state & BUTTON_BTNPRESSED) && gxGetCapture() == hWnd)
      gxReleaseCapture();
    if (style & GXBS_NOTIFY)
      BUTTON_NOTIFY_PARENT(hWnd, GXBN_KILLFOCUS);

    break;

  case GXWM_SYSCOLORCHANGE:
    gxInvalidateRect( hWnd, NULL, FALSE );
    break;

    //case BM_SETSTYLE16:
  case GXBM_SETSTYLE:
    if ((wParam & 0x0f) >= MAX_BTN_TYPE) break;
    btn_type = wParam & 0x0f;
    style = (style & ~0x0f) | btn_type;
    gxSetWindowLongW( hWnd, GXGWL_STYLE, style );

    /* Only redraw if lParam flag is set.*/
    if (lParam)
      paint_button( hWnd, btn_type, GXODA_DRAWENTIRE );

    break;

  case GXBM_CLICK:
    gxSendMessageW( hWnd, GXWM_LBUTTONDOWN, 0, 0 );
    gxSendMessageW( hWnd, GXWM_LBUTTONUP, 0, 0 );
    break;

  case GXBM_SETIMAGE:
    /* Check that image format matches button style */
    switch (style & (GXBS_BITMAP|GXBS_ICON))
    {
    case GXBS_BITMAP:
      if (wParam != GXIMAGE_BITMAP) return 0;
      break;
    case GXBS_ICON:
      if (wParam != GXIMAGE_ICON) return 0;
      break;
    default:
      return 0;
    }
    oldHbitmap = (GXHBITMAP)gxSetWindowLongPtrW( hWnd, HIMAGE_GWL_OFFSET, lParam );
    gxInvalidateRect( hWnd, NULL, FALSE );
    return (GXLRESULT)oldHbitmap;

  case GXBM_GETIMAGE:
    return gxGetWindowLongPtrW( hWnd, HIMAGE_GWL_OFFSET );

    //case BM_GETCHECK16:
  case GXBM_GETCHECK:
    return get_button_state( hWnd ) & 3;

    //case BM_SETCHECK16:
  case GXBM_SETCHECK:
    if (wParam > maxCheckState[btn_type]) wParam = maxCheckState[btn_type];
    state = get_button_state( hWnd );
    if ((btn_type == GXBS_RADIOBUTTON) || (btn_type == GXBS_AUTORADIOBUTTON))
    {
      if (wParam) style |= GXWS_TABSTOP;
      else style &= ~GXWS_TABSTOP;
      gxSetWindowLongW( hWnd, GXGWL_STYLE, style );
    }
    if ((state & 3) != wParam)
    {
      set_button_state( hWnd, (state & ~3) | wParam );
      paint_button( hWnd, btn_type, GXODA_SELECT );
    }
    if ((btn_type == GXBS_AUTORADIOBUTTON) && (wParam == BUTTON_CHECKED) && (style & GXWS_CHILD))
      BUTTON_CheckAutoRadioButton( hWnd );
    break;

    //case BM_GETSTATE16:
  case GXBM_GETSTATE:
    return get_button_state( hWnd );

    //case BM_SETSTATE16:
  case GXBM_SETSTATE:
    state = get_button_state( hWnd );
    if (wParam)
    {
      if (state & BUTTON_HIGHLIGHTED) break;
      set_button_state( hWnd, state | BUTTON_HIGHLIGHTED );
    }
    else
    {
      if (!(state & BUTTON_HIGHLIGHTED)) break;
      set_button_state( hWnd, state & ~BUTTON_HIGHLIGHTED );
    }
    paint_button( hWnd, btn_type, GXODA_SELECT );
    break;

  case GXWM_NCHITTEST:
    if(btn_type == GXBS_GROUPBOX) return GXHTTRANSPARENT;
    /* fall through */
  default:
    return unicode ? gxDefWindowProcW(hWnd, uMsg, wParam, lParam) :
      gxDefWindowProcA(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}

/***********************************************************************
*           ButtonWndProcW
* The button window procedure. This is just a wrapper which locks
* the passed GXHWND and calls the real window procedure (with a WND*
* pointer pointing to the locked windowstructure).
*/
GXLRESULT GXDLLAPI ButtonWndProcW( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam )
{
  if (!gxIsWindow( hWnd )) return 0;
  return ButtonWndProc_common( hWnd, uMsg, wParam, lParam, TRUE );
}


/***********************************************************************
*           ButtonWndProcA
*/
GXLRESULT GXDLLAPI ButtonWndProcA( GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam )
{
  if (!gxIsWindow( hWnd )) return 0;
  return ButtonWndProc_common( hWnd, uMsg, wParam, lParam, FALSE );
}


/**********************************************************************
* Convert button styles to flags used by DrawText.
* TODO: handle WS_EX_RIGHT extended style.
*/
static GXUINT BUTTON_BStoDT(GXDWORD style)
{
  GXUINT dtStyle = GXDT_NOCLIP;  /* We use gxSelectClipRgn to limit output */

  /* "Convert" pushlike buttons to pushbuttons */
  if (style & GXBS_PUSHLIKE)
    style &= ~0x0F;

  if (!(style & GXBS_MULTILINE))
    dtStyle |= GXDT_SINGLELINE;
  else
    dtStyle |= GXDT_WORDBREAK;

  switch (style & GXBS_CENTER)
  {
  case GXBS_LEFT:   /* GXDT_LEFT is 0 */    break;
  case GXBS_RIGHT:  dtStyle |= GXDT_RIGHT;  break;
  case GXBS_CENTER: dtStyle |= GXDT_CENTER; break;
  default:
    /* Pushbutton's text is centered by default */
    if (get_button_type(style) <= GXBS_DEFPUSHBUTTON) dtStyle |= GXDT_CENTER;
    /* all other flavours have left aligned text */
  }

  /* DrawText ignores vertical alignment for multiline text,
  * but we use these flags to align label manually.
  */
  if (get_button_type(style) != GXBS_GROUPBOX)
  {
    switch (style & GXBS_VCENTER)
    {
    case GXBS_TOP:     /* GXDT_TOP is 0 */      break;
    case GXBS_BOTTOM:  dtStyle |= GXDT_BOTTOM;  break;
    case GXBS_VCENTER: /* fall through */
    default:         dtStyle |= GXDT_VCENTER; break;
    }
  }
  else
    /* GroupBox's text is always single line and is top aligned. */
    dtStyle |= GXDT_SINGLELINE;

  return dtStyle;
}

/**********************************************************************
*       BUTTON_CalcLabelRect
*
*   Calculates label's rectangle depending on button style.
*
* Returns flags to be passed to DrawText.
* Calculated rectangle doesn't take into account button state
* (pushed, etc.). If there is nothing to draw (no text/image) output
* rectangle is empty, and return value is (GXUINT)-1.
*/
static GXUINT BUTTON_CalcLabelRect(GXHWND hwnd, GXHDC hdc, GXRECT *rc)
{
  GXLONG style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXWCHAR *text;
  GXICONINFO    iconInfo;
  GXBITMAP      bm;
  GXUINT        dtStyle = BUTTON_BStoDT(style);
  GXRECT        r = *rc;
  GXINT         n;

  /* Calculate label rectangle according to label type */
  switch (style & (GXBS_ICON|GXBS_BITMAP))
  {
  case GXBS_TEXT:
    if (!(text = get_button_text( hwnd ))) goto empty_rect;
    if (!text[0])
    {
      gxHeapFree( gxGetProcessHeap(), 0, text );
      goto empty_rect;
    }
    gxDrawTextW(hdc, text, -1, &r, dtStyle | GXDT_CALCRECT);
    gxHeapFree( gxGetProcessHeap(), 0, text );
    break;

  case GXBS_ICON:
    if (!gxGetIconInfo((GXHICON)gxGetWindowLongPtrW( hwnd, HIMAGE_GWL_OFFSET ), &iconInfo))
      goto empty_rect;

    gxGetObjectW (iconInfo.hbmColor, sizeof(GXBITMAP), &bm);

    r.right  = r.left + bm.bmWidth;
    r.bottom = r.top  + bm.bmHeight;

    gxDeleteObject(iconInfo.hbmColor);
    gxDeleteObject(iconInfo.hbmMask);
    break;

  case GXBS_BITMAP:
    if (!gxGetObjectW( (GXHGDIOBJ)gxGetWindowLongPtrW( hwnd, HIMAGE_GWL_OFFSET ), sizeof(GXBITMAP), &bm))
      goto empty_rect;

    r.right  = r.left + bm.bmWidth;
    r.bottom = r.top  + bm.bmHeight;
    break;

  default:
empty_rect:
    rc->right = r.left;
    rc->bottom = r.top;
    return (GXUINT)(GXLONG)-1;
  }

  /* Position label inside bounding rectangle according to
  * alignment flags. (calculated rect is always left-top aligned).
  * If label is aligned to any side - shift label in opposite
  * direction to leave extra space for focus rectangle.
  */
  switch (dtStyle & (GXDT_CENTER|GXDT_RIGHT))
  {
  case GXDT_LEFT:    r.left++;  r.right++;  break;
  case GXDT_CENTER:  n = r.right - r.left;
    r.left   = rc->left + ((rc->right - rc->left) - n) / 2;
    r.right  = r.left + n; break;
  case GXDT_RIGHT:   n = r.right - r.left;
    r.right  = rc->right - 1;
    r.left   = r.right - n;
    break;
  }

  switch (dtStyle & (GXDT_VCENTER|GXDT_BOTTOM))
  {
  case GXDT_TOP:     r.top++;  r.bottom++;  break;
  case GXDT_VCENTER: n = r.bottom - r.top;
    r.top    = rc->top + ((rc->bottom - rc->top) - n) / 2;
    r.bottom = r.top + n;  break;
  case GXDT_BOTTOM:  n = r.bottom - r.top;
    r.bottom = rc->bottom - 1;
    r.top    = r.bottom - n;
    break;
  }

  *rc = r;
  return dtStyle;
}


/**********************************************************************
*       BUTTON_DrawTextCallback
*
*   Callback function used by DrawStateW function.
*/
static GXBOOL CALLBACK BUTTON_DrawTextCallback(GXHDC hdc, GXLPARAM lp, GXWPARAM wp, GXINT cx, GXINT cy)
{
  GXRECT rc;
  rc.left = 0;
  rc.top = 0;
  rc.right = cx;
  rc.bottom = cy;

  gxDrawTextW(hdc, (GXLPCWSTR)lp, -1, &rc, (GXUINT)wp);
  return TRUE;
}


/**********************************************************************
*       BUTTON_DrawLabel
*
*   Common function for drawing button label.
*/
static void BUTTON_DrawLabel(GXHWND hwnd, GXHDC hdc, GXUINT dtFlags, const GXRECT *rc)
{
  GXDRAWSTATEPROC lpOutputProc = NULL;
  GXLPARAM lp;
  GXWPARAM wp = 0;
  GXHBRUSH hbr = 0;
  GXUINT flags = gxIsWindowEnabled(hwnd) ? GXDSS_NORMAL : GXDSS_DISABLED;
  GXLONG state = get_button_state( hwnd );
  GXLONG style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXWCHAR *text = NULL;

  /* FIXME: To draw disabled label in Win31 look-and-feel, we probably
  * must use DSS_MONO flag and GXCOLOR_GRAYTEXT brush (or maybe DSS_UNION).
  * I don't have Win31 on hand to verify that, so I leave it as is.
  */

  if ((style & GXBS_PUSHLIKE) && (state & BUTTON_3STATE))
  {
    hbr = gxGetSysColorBrush(GXCOLOR_GRAYTEXT);
    flags |= GXDSS_MONO;
  }

  switch (style & (GXBS_ICON|GXBS_BITMAP))
  {
  case GXBS_TEXT:
    /* DST_COMPLEX -- is 0 */
    lpOutputProc = BUTTON_DrawTextCallback;
    if (!(text = get_button_text( hwnd ))) return;
    lp = (GXLPARAM)text;
    wp = (GXWPARAM)dtFlags;
    break;

  case GXBS_ICON:
    flags |= GXDST_ICON;
    lp = gxGetWindowLongPtrW( hwnd, HIMAGE_GWL_OFFSET );
    break;

  case GXBS_BITMAP:
    flags |= GXDST_BITMAP;
    lp = gxGetWindowLongPtrW( hwnd, HIMAGE_GWL_OFFSET );
    break;

  default:
    return;
  }
  //Artint Patch
  GXUINT nt = get_button_type(style);
  if(nt == GXBS_AUTOCHECKBOX || nt == GXBS_CHECKBOX || nt == GXBS_RADIOBUTTON || nt == GXBS_AUTORADIOBUTTON)
    gxSetWindowOrgEx(hdc, -checkBoxWidth, 0, NULL);
  //</Artint Patch>
  gxDrawStateW(hdc, hbr, lpOutputProc, lp, wp | GDT_GLOW, rc->left, rc->top,
    rc->right - rc->left, rc->bottom - rc->top, flags);
  gxHeapFree( gxGetProcessHeap(), 0, text );
}

/**********************************************************************
*       Push Button Functions
*/
static void PB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action )
{
  GXRECT     rc, focus_rect, r;
  GXUINT     dtFlags, uState;
  GXHPEN     hOldPen;
  GXHBRUSH   hOldBrush;
  GXINT      oldBkMode;
  GXCOLORREF oldTxtColor;
  GXHFONT hFont;
  GXLONG state = get_button_state( hwnd );
  GXLONG style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXBOOL pushedState = (state & BUTTON_HIGHLIGHTED);
  GXHWND parent;

  gxGetClientRect( hwnd, &rc );

  /* Send GXWM_CTLCOLOR to allow changing the font (the colors are fixed) */
  if ((hFont = get_button_font( hwnd ))) gxSelectObject( hDC, hFont );
  parent = gxGetParent(hwnd);
  if (!parent) parent = hwnd;
  gxSendMessageW( parent, GXWM_CTLCOLORBTN, (GXWPARAM)hDC, (GXLPARAM)hwnd );
  hOldPen = (GXHPEN)gxSelectObject(hDC, SYSCOLOR_GetPen(GXCOLOR_WINDOWFRAME));
    hOldBrush =(GXHBRUSH)gxSelectObject(hDC,gxGetSysColorBrush(GXCOLOR_BTNFACE));
  oldBkMode = gxSetBkMode(hDC, GXTRANSPARENT);


  uState = GXDFCS_BUTTONPUSH | GXDFCS_ADJUSTRECT;

  if (get_button_type(style) == GXBS_DEFPUSHBUTTON)
  {
    // Artint Change for gx
    //      gxRectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    //gxInflateRect( &rc, -1, -1 );
    uState |= GXDFCS_HOT;
    // </Artint Change for gx>
  }

  if (style & GXBS_FLAT)
    uState |= GXDFCS_MONO;
  else if (pushedState)
  {
    if (get_button_type(style) == GXBS_DEFPUSHBUTTON )
      uState |= GXDFCS_FLAT;
    else
      uState |= GXDFCS_PUSHED;
  }

  if (state & (BUTTON_CHECKED | BUTTON_3STATE))
    uState |= GXDFCS_CHECKED;

  gxDrawFrameControl( hDC, &rc, GXDFC_BUTTON, uState );

  focus_rect = rc;

  /* draw button label */
  r = rc;
  dtFlags = BUTTON_CalcLabelRect(hwnd, hDC, &r);

  if (dtFlags == (GXUINT)-1L)
    goto cleanup;

  if (pushedState)
    gxOffsetRect(&r, 1, 1);

  gxIntersectClipRect(hDC, rc.left, rc.top, rc.right, rc.bottom);

  oldTxtColor = gxSetTextColor( hDC, gxGetSysColor(GXCOLOR_BTNTEXT) );

  BUTTON_DrawLabel(hwnd, hDC, dtFlags, &r);

  gxSetTextColor( hDC, oldTxtColor );

  if (state & BUTTON_HASFOCUS)
  {
    gxInflateRect( &focus_rect, -1, -1 );
    gxIntersectRect(&focus_rect, &focus_rect, &rc);
    gxDrawFocusRect( hDC, &focus_rect );
  }

cleanup:
  gxSelectObject( hDC, hOldPen );
  gxSelectObject( hDC, hOldBrush );
  gxSetBkMode(hDC, oldBkMode);
}

/**********************************************************************
*       Check Box & Radio Button Functions
*/

static void CB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action )
{
  GXRECT rbox, rtext, client;
  GXHBRUSH hBrush;
  int delta;
  GXUINT dtFlags;
  GXHFONT hFont;
  GXLONG state = get_button_state( hwnd );
  GXLONG style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXHWND parent;

  if (style & GXBS_PUSHLIKE)
  {
    PB_Paint( hwnd, hDC, action );
    return;
  }

  gxGetClientRect(hwnd, &client);
  rbox = rtext = client;

  if ((hFont = get_button_font( hwnd ))) gxSelectObject( hDC, hFont );

  parent = gxGetParent(hwnd);
  if (!parent) parent = hwnd;
  hBrush = (GXHBRUSH)gxSendMessageW(parent, GXWM_CTLCOLORSTATIC,
    (GXWPARAM)hDC, (GXLPARAM)hwnd);
  if (!hBrush) /* did the app forget to call defwindowproc ? */
    hBrush = (GXHBRUSH)gxDefWindowProcW(parent, GXWM_CTLCOLORSTATIC,
    (GXWPARAM)hDC, (GXLPARAM)hwnd );

  if (style & GXBS_LEFTTEXT)
  {
    /* magic +4 is what CTL3D expects */

    rtext.right -= checkBoxWidth + 4;
    rbox.left = rbox.right - checkBoxWidth;
  }
  else
  {
    rtext.left += checkBoxWidth + 4;
    rbox.right = checkBoxWidth;
  }

  /* Since GXWM_ERASEBKGND does nothing, first prepare background */
  if (action == GXODA_SELECT) gxFillRect( hDC, &rbox, hBrush );
  if (action == GXODA_DRAWENTIRE) gxFillRect( hDC, &client, hBrush );

  /* Draw label */
  client = rtext;
  dtFlags = BUTTON_CalcLabelRect(hwnd, hDC, &rtext);

  /* Only adjust rbox when rtext is valid */
  if (dtFlags != (GXUINT)-1L)
  {
    rbox.top = rtext.top;
    rbox.bottom = rtext.bottom;
  }

  /* Draw the check-box bitmap */
  if (action == GXODA_DRAWENTIRE || action == GXODA_SELECT)
  {
    GXUINT flags;

    if ((get_button_type(style) == GXBS_RADIOBUTTON) ||
      (get_button_type(style) == GXBS_AUTORADIOBUTTON)) flags = GXDFCS_BUTTONRADIO;
    else if (state & BUTTON_3STATE) flags = GXDFCS_BUTTON3STATE;
    else flags = GXDFCS_BUTTONCHECK;

    if (state & (BUTTON_CHECKED | BUTTON_3STATE)) flags |= GXDFCS_CHECKED;
    if (state & BUTTON_HIGHLIGHTED) flags |= GXDFCS_PUSHED;

    if (style & GXWS_DISABLED) flags |= GXDFCS_INACTIVE;

    /* rbox must have the correct height */
    delta = rbox.bottom - rbox.top - checkBoxHeight;

    if (style & GXBS_TOP) {
      if (delta > 0) {
        rbox.bottom = rbox.top + checkBoxHeight;
      } else { 
        rbox.top -= -delta/2 + 1;
        rbox.bottom = rbox.top + checkBoxHeight;
      }
    } else if (style & GXBS_BOTTOM) {
      if (delta > 0) {
        rbox.top = rbox.bottom - checkBoxHeight;
      } else {
        rbox.bottom += -delta/2 + 1;
        rbox.top = rbox.bottom - checkBoxHeight;
      }
    } else { /* Default */
      if (delta > 0) {
        int ofs = (delta / 2);
        rbox.bottom -= ofs + 1;
        rbox.top = rbox.bottom - checkBoxHeight;
      } else if (delta < 0) {
        int ofs = (-delta / 2);
        rbox.top -= ofs + 1;
        rbox.bottom = rbox.top + checkBoxHeight;
      }
    }

    gxDrawFrameControl( hDC, &rbox, GXDFC_BUTTON, flags );
  }

  if (dtFlags == (GXUINT)-1L) /* Noting to draw */
    return;

  gxIntersectClipRect(hDC, client.left, client.top, client.right, client.bottom);

  if (action == GXODA_DRAWENTIRE)
    BUTTON_DrawLabel(hwnd, hDC, dtFlags, &rtext);

  /* ... and focus */
  if ((action == GXODA_FOCUS) ||
    ((action == GXODA_DRAWENTIRE) && (state & BUTTON_HASFOCUS)))
  {
    rtext.left--;
    rtext.right++;
    gxIntersectRect(&rtext, &rtext, &client);
    gxDrawFocusRect( hDC, &rtext );
  }
}


/**********************************************************************
*       BUTTON_CheckAutoRadioButton
*
* hwnd is checked, uncheck every other auto radio button in group
*/
static void BUTTON_CheckAutoRadioButton( GXHWND hwnd )
{
  GXHWND parent, sibling, start;

  parent = gxGetParent(hwnd);
  /* make sure that starting control is not disabled or invisible */
  start = sibling = gxGetNextDlgGroupItem( parent, hwnd, TRUE );
  do
  {
    if (!sibling) break;
    if ((hwnd != sibling) &&
      ((gxGetWindowLongW( sibling, GXGWL_STYLE) & 0x0f) == GXBS_AUTORADIOBUTTON))
      gxSendMessageW( sibling, GXBM_SETCHECK, BUTTON_UNCHECKED, 0 );
    sibling = gxGetNextDlgGroupItem( parent, sibling, FALSE );
  } while (sibling != start);
}


/**********************************************************************
*       Group Box Functions
*/

static void GB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action )
{
  GXRECT rc, rcFrame;
  GXHBRUSH hbr;
  GXHFONT hFont;
  GXUINT dtFlags;
  GXTEXTMETRICW tm;
  GXLONG style = gxGetWindowLongW( hwnd, GXGWL_STYLE );
  GXHWND parent;

  if ((hFont = get_button_font( hwnd ))) gxSelectObject( hDC, hFont );
  /* GroupBox acts like static control, so it sends CTLCOLORSTATIC */
  parent = gxGetParent(hwnd);
  if (!parent) parent = hwnd;
  hbr = (GXHBRUSH)gxSendMessageW(parent, GXWM_CTLCOLORSTATIC, (GXWPARAM)hDC, (GXLPARAM)hwnd);
  if (!hbr) /* did the app forget to call defwindowproc ? */
    hbr = (GXHBRUSH)gxDefWindowProcW(parent, GXWM_CTLCOLORSTATIC,
    (GXWPARAM)hDC, (GXLPARAM)hwnd);

  gxGetClientRect( hwnd, &rc);
  rcFrame = rc;

  gxGetTextMetricsW (hDC, &tm);
  rcFrame.top += (tm.tmHeight / 2) - 1;
  gxDrawEdge(hDC, &rcFrame, GXEDGE_ETCHED, GXBF_RECT | ((style & GXBS_FLAT) ? GXBF_FLAT : 0));

  gxInflateRect(&rc, -7, 1);
  dtFlags = BUTTON_CalcLabelRect(hwnd, hDC, &rc);

  if (dtFlags == (GXUINT)-1L)
    return;

  /* Because buttons have CS_PARENTDC class style, there is a chance
  * that label will be drawn out of client rect.
  * But Windows doesn't clip label's rect, so do I.
  */

  /* There is 1-pixel marging at the left, right, and bottom */
  rc.left--; rc.right++; rc.bottom++;
  gxFillRect(hDC, &rc, hbr);
  rc.left++; rc.right--; rc.bottom--;

  BUTTON_DrawLabel(hwnd, hDC, dtFlags, &rc);
}


/**********************************************************************
*       User Button Functions
*/

static void UB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action )
{
  GXRECT rc;
  GXHBRUSH hBrush;
  GXHFONT hFont;
  GXLONG state = get_button_state( hwnd );
  GXHWND parent;

  if (action == GXODA_SELECT) return;

  gxGetClientRect( hwnd, &rc);

  if ((hFont = get_button_font( hwnd ))) gxSelectObject( hDC, hFont );

  parent = gxGetParent(hwnd);
  if (!parent) parent = hwnd;
  hBrush = (GXHBRUSH)gxSendMessageW(parent, GXWM_CTLCOLORBTN, (GXWPARAM)hDC, (GXLPARAM)hwnd);
  if (!hBrush) /* did the app forget to call defwindowproc ? */
    hBrush = (GXHBRUSH)gxDefWindowProcW(parent, GXWM_CTLCOLORBTN,
    (GXWPARAM)hDC, (GXLPARAM)hwnd);

  gxFillRect( hDC, &rc, hBrush );
  if ((action == GXODA_FOCUS) ||
    ((action == GXODA_DRAWENTIRE) && (state & BUTTON_HASFOCUS)))
    gxDrawFocusRect( hDC, &rc );
}


/**********************************************************************
*       Ownerdrawn Button Functions
*/

static void OB_Paint( GXHWND hwnd, GXHDC hDC, GXUINT action )
{
  GXLONG state = get_button_state( hwnd );
  GXDRAWITEMSTRUCT dis;
  GXHRGN clipRegion;
  GXRECT clipRect;
  GXLONG_PTR id = gxGetWindowLongPtrW( hwnd, GXGWLP_ID );
  GXHWND parent;
  GXHFONT hFont, hPrevFont = 0;

  dis.CtlType    = GXODT_BUTTON;
  dis.CtlID      = id;
  dis.itemID     = 0;
  dis.itemAction = action;
  dis.itemState  = ((state & BUTTON_HASFOCUS) ? GXODS_FOCUS : 0) |
    ((state & BUTTON_HIGHLIGHTED) ? GXODS_SELECTED : 0) |
    (gxIsWindowEnabled(hwnd) ? 0: GXODS_DISABLED);
  dis.hwndItem   = (GXHWND)hwnd;
  dis.hDC        = (GXHDC)hDC;
  dis.itemData   = 0;
  gxGetClientRect( hwnd, (GXRECT*)&dis.rcItem );

  clipRegion = gxCreateRectRgnIndirect(&dis.rcItem);
  if (gxGetClipRgn(hDC, clipRegion) != 1)
  {
    gxDeleteObject(clipRegion);
    clipRegion=NULL;
  }
  clipRect = *(GXRECT*)&dis.rcItem;
  gxDPtoLP(hDC, (GXLPPOINT) &clipRect, 2);
  gxIntersectClipRect(hDC, clipRect.left,  clipRect.top, clipRect.right, clipRect.bottom);

  if ((hFont = get_button_font( hwnd ))) hPrevFont = (GXHFONT)gxSelectObject( hDC, hFont );
  parent = gxGetParent(hwnd);
  if (!parent) parent = hwnd;
  gxSendMessageW( parent, GXWM_CTLCOLORBTN, (GXWPARAM)hDC, (GXLPARAM)hwnd );
  gxSendMessageW( gxGetParent(hwnd), GXWM_DRAWITEM, id, (GXLPARAM)&dis );
  if (hPrevFont) gxSelectObject(hDC, hPrevFont);
  gxSelectClipRgn(hDC, clipRegion);
}
#endif // _DEV_DISABLE_UI_CODE