#ifndef _DEV_DISABLE_UI_CODE
#ifndef TOOLTIP
/*
* Tool tip control
*
* Copyright 1998, 1999 Eric Kohl
* Copyright 2004 Robert Shearman
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
* of Comctl32.dll version 6.0 on Sep. 08, 2004, by Robert Shearman.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features or bugs please note them below.
* 
* TODO:
*   - Custom draw support.
*   - Animation.
*   - Links.
*   - Messages:
*     o TTM_ADJUSTRECT
*     o TTM_GETTITLEA
*     o TTM_GETTTILEW
*     o TTM_POPUP
*   - Styles:
*     o TTS_NOANIMATE
*     o TTS_NOFADE
*     o TTS_CLOSE
*
* Testing:
*   - Run tests using Waite Group Windows95 API Bible Volume 2.
*     The second cdrom (chapter 3) contains executables activate.exe,
*     curtool.exe, deltool.exe, enumtools.exe, getinfo.exe, getiptxt.exe,
*     hittest.exe, needtext.exe, newrect.exe, updtext.exe and winfrpt.exe.
*
*   Timer logic.
*
* One important point to remember is that tools don't necessarily get
* a WM_MOUSEMOVE once the cursor leaves the tool, an example is when
* a tool sets TTF_IDISHWND (i.e. an entire window is a tool) because
* here WM_MOUSEMOVEs only get sent when the cursor is inside the
* client area.  Therefore the only reliable way to know that the
* cursor has left a tool is to keep a timer running and check the
* position every time it expires.  This is the role of timer
* ID_TIMERLEAVE.
*
*
* On entering a tool (detected in a relayed WM_MOUSEMOVE) we start
* ID_TIMERSHOW, if this times out and we're still in the tool we show
* the tip.  On showing a tip we start both ID_TIMERPOP and
* ID_TIMERLEAVE.  On hiding a tooltip we kill ID_TIMERPOP.
* ID_TIMERPOP is restarted on every relayed WM_MOUSEMOVE.  If
* ID_TIMERPOP expires the tool is hidden and ID_TIMERPOP is killed.
* ID_TIMERLEAVE remains running - this is important as we need to
* determine when the cursor leaves the tool.
*
* When ID_TIMERLEAVE expires or on a relayed WM_MOUSEMOVE if we're
* still in the tool do nothing (apart from restart ID_TIMERPOP if
* this is a WM_MOUSEMOVE) (ID_TIMERLEAVE remains running).  If we've
* left the tool and entered another one then hide the tip and start
* ID_TIMERSHOW with time ReshowTime and kill ID_TIMERLEAVE.  If we're
* outside all tools hide the tip and kill ID_TIMERLEAVE.  On Relayed
* mouse button messages hide the tip but leave ID_TIMERLEAVE running,
* this again will let us keep track of when the cursor leaves the
* tool.
*
*
* infoPtr->nTool is the tool the mouse was on on the last relayed MM
* or timer expiry or -1 if the mouse was not on a tool.
*
* infoPtr->nCurrentTool is the tool for which the tip is currently
* displaying text for or -1 if the tip is not shown.  Actually this
* will only ever be infoPtr-nTool or -1, so it could be changed to a
* GXBOOL.
*
*/

// 
// 2014/7/18 artint 修改
// 1. 合并 AddToolA 和 AddToolW， 并改为 struct 内的方法调用
//

//#include <stdarg.h>
//#include <string.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "wine/unicode.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "winnls.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(tooltips);

#include <GrapX.H>
#include "GrapX/gUxtheme.h"
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXImm.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.H>
#include <User/Win32Emu/GXCommCtrl.H>

#ifdef TRACE
#undef TRACE
#define TRACE
#endif // TRACE

#ifdef TRACEA
#undef TRACEA
#define TRACEA
#endif // TRACEA

#ifdef TRACEW
#undef TRACEW
#define TRACEW
#endif // TRACEW

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

static GXHICON hTooltipIcons[TTI_ERROR+1];

typedef struct
{
  GXUINT      uFlags;
  GXHWND      hwnd;
  GXBOOL      bNotifyUnicode;
  GXUINT_PTR  uId;
  GXRECT      rect;
  GXHINSTANCE hinst;
  GXLPWSTR    lpszText;
  GXLPARAM    lParam;
} TTTOOL_INFO;


typedef struct
{
  GXWCHAR    szTipText[INFOTIPSIZE];
  GXBOOL     bActive;
  GXBOOL     bTrackActive;
  GXUINT     m_uNumTools;
  GXCOLORREF clrBk;
  GXCOLORREF clrText;
  GXHFONT    hFont;
  GXHFONT    hTitleFont;
  GXINT      xTrackPos;
  GXINT      yTrackPos;
  GXINT      nMaxTipWidth;
  GXINT      nTool; /* tool that mouse was on on last relayed mouse move */
  GXINT      nCurrentTool;
  GXINT      nTrackTool;
  GXINT      nReshowTime;
  GXINT      nAutoPopTime;
  GXINT      nInitialTime;
  GXRECT     rcMargin;
  GXBOOL     bToolBelow;
  GXLPWSTR   pszTitle;
  GXHICON    hTitleIcon;

  TTTOOL_INFO *m_pTools;

  GXLRESULT AddToolA  (GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam);
  GXLRESULT AddToolW  (GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam);
  GXLRESULT Destroy   (GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam);
} TOOLTIPS_INFO;

#define ID_TIMERSHOW   1    /* show delay timer */
#define ID_TIMERPOP    2    /* auto pop timer */
#define ID_TIMERLEAVE  3    /* tool leave timer */


#define TOOLTIPS_GetInfoPtr(hWindow) ((TOOLTIPS_INFO *)gxGetWindowLongPtrW (hWindow, 0))

/* offsets from window edge to start of text */
#define NORMAL_TEXT_MARGIN 2
#define BALLOON_TEXT_MARGIN (NORMAL_TEXT_MARGIN+8)
/* value used for CreateRoundRectRgn that specifies how much
* each corner is curved */
#define BALLOON_ROUNDEDNESS 20
#define BALLOON_STEMHEIGHT 13
#define BALLOON_STEMWIDTH 10
#define BALLOON_STEMINDENT 20

#define BALLOON_ICON_TITLE_SPACING 8 /* horizontal spacing between icon and title */
#define BALLOON_TITLE_TEXT_SPACING 8 /* vertical spacing between icon/title and main text */
#define ICON_HEIGHT 16
#define ICON_WIDTH  16

static GXLRESULT GXCALLBACK
TOOLTIPS_SubclassProc (GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam, GXUINT_PTR uId, GXDWORD_PTR dwRef);


static inline GXUINT_PTR
TOOLTIPS_GetTitleIconIndex(GXHICON hIcon)
{
  GXUINT i;
  for (i = 0; i <= TTI_ERROR; i++)
    if (hTooltipIcons[i] == hIcon)
      return i;
  return (GXUINT_PTR)hIcon;
}

static void
TOOLTIPS_InitSystemSettings (TOOLTIPS_INFO *infoPtr)
{
  GXNONCLIENTMETRICSW nclm;

  infoPtr->clrBk   = gxGetSysColor (GXCOLOR_INFOBK);
  infoPtr->clrText = gxGetSysColor (GXCOLOR_INFOTEXT);

  gxDeleteObject (infoPtr->hFont);
  nclm.cbSize = sizeof(nclm);
  gxSystemParametersInfoW (SPI_GETNONCLIENTMETRICS, sizeof(nclm), &nclm, 0);
  infoPtr->hFont = gxCreateFontIndirectW ((const GXLOGFONTW*)&nclm.lfStatusFont);

  gxDeleteObject (infoPtr->hTitleFont);
  nclm.lfStatusFont.lfWeight = GXFW_BOLD;
  infoPtr->hTitleFont = gxCreateFontIndirectW ((const GXLOGFONTW*)&nclm.lfStatusFont);
}

static void
TOOLTIPS_Refresh (GXHWND hwnd, GXHDC hdc)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr(hwnd);
  GXRECT rc;
  GXINT oldBkMode;
  GXHFONT hOldFont;
  GXHBRUSH hBrush;
  GXUINT uFlags = GXDT_EXTERNALLEADING;
  GXHRGN hRgn = NULL;
  GXDWORD dwStyle = gxGetWindowLongW(hwnd, GXGWL_STYLE);

  if (infoPtr->nMaxTipWidth > -1)
    uFlags |= GXDT_WORDBREAK;
  if (gxGetWindowLongW (hwnd, GXGWL_STYLE) & TTS_NOPREFIX)
    uFlags |= GXDT_NOPREFIX;
  gxGetClientRect (hwnd, &rc);

  hBrush = gxCreateSolidBrush(infoPtr->clrBk);

  oldBkMode = gxSetBkMode (hdc, GXTRANSPARENT);
  gxSetTextColor (hdc, infoPtr->clrText);

  if (dwStyle & TTS_BALLOON)
  {
    /* create a region to store result into */
    hRgn = gxCreateRectRgn(0, 0, 0, 0);

    gxGetWindowRgn(hwnd, hRgn);

    /* fill the background */
    gxFillRgn(hdc, hRgn, hBrush);
    gxDeleteObject(hBrush);
    hBrush = NULL;
  }
  else
  {
    /* fill the background */
    gxFillRect(hdc, &rc, hBrush);
    gxDeleteObject(hBrush);
    hBrush = NULL;
  }

  if ((dwStyle & TTS_BALLOON) || infoPtr->pszTitle)
  {
    /* calculate text rectangle */
    rc.left   += (BALLOON_TEXT_MARGIN + infoPtr->rcMargin.left);
    rc.top    += (BALLOON_TEXT_MARGIN + infoPtr->rcMargin.top);
    rc.right  -= (BALLOON_TEXT_MARGIN + infoPtr->rcMargin.right);
    rc.bottom -= (BALLOON_TEXT_MARGIN + infoPtr->rcMargin.bottom);
    if(infoPtr->bToolBelow) rc.top += BALLOON_STEMHEIGHT;

    if (infoPtr->pszTitle)
    {
      GXRECT rcTitle(rc.left, rc.top, rc.right, rc.bottom);
      int height;
      GXBOOL icon_present;

      /* draw icon */
      icon_present = infoPtr->hTitleIcon && 
        gxDrawIconEx(hdc, rc.left, rc.top, infoPtr->hTitleIcon,
        ICON_WIDTH, ICON_HEIGHT, 0, NULL, GXDI_NORMAL);
      if (icon_present)
        rcTitle.left += ICON_WIDTH + BALLOON_ICON_TITLE_SPACING;

      rcTitle.bottom = rc.top + ICON_HEIGHT;

      /* draw title text */
      hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hTitleFont);
      height = gxDrawTextW(hdc, infoPtr->pszTitle, -1, &rcTitle, GXDT_BOTTOM | GXDT_SINGLELINE | GXDT_NOPREFIX);
      gxSelectObject (hdc, hOldFont);
      rc.top += height + BALLOON_TITLE_TEXT_SPACING;
    }
  }
  else
  {
    /* calculate text rectangle */
    rc.left   += (NORMAL_TEXT_MARGIN + infoPtr->rcMargin.left);
    rc.top    += (NORMAL_TEXT_MARGIN + infoPtr->rcMargin.top);
    rc.right  -= (NORMAL_TEXT_MARGIN + infoPtr->rcMargin.right);
    rc.bottom -= (NORMAL_TEXT_MARGIN + infoPtr->rcMargin.bottom);
  }

  /* draw text */
  hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hFont);
  gxDrawTextW (hdc, infoPtr->szTipText, -1, &rc, uFlags);
  /* be polite and reset the things we changed in the dc */
  gxSelectObject (hdc, hOldFont);
  gxSetBkMode (hdc, oldBkMode);

  if (dwStyle & TTS_BALLOON)
  {
    /* frame region because default window proc doesn't do it */
    GXINT width = gxGetSystemMetrics(GXSM_CXDLGFRAME) - gxGetSystemMetrics(GXSM_CXEDGE);
    GXINT height = gxGetSystemMetrics(GXSM_CYDLGFRAME) - gxGetSystemMetrics(GXSM_CYEDGE);

    hBrush = gxGetSysColorBrush(GXCOLOR_WINDOWFRAME);
    gxFrameRgn(hdc, hRgn, hBrush, width, height);
  }

  if (hRgn)
    gxDeleteObject(hRgn);
}

static void TOOLTIPS_GetDispInfoA(GXHWND hwnd, TOOLTIPS_INFO *infoPtr, TTTOOL_INFO *toolPtr)
{
  GXNMTTDISPINFOA ttnmdi;

  /* fill GXNMHDR struct */
  ZeroMemory (&ttnmdi, sizeof(GXNMTTDISPINFOA));
  ttnmdi.hdr.hwndFrom = hwnd;
  ttnmdi.hdr.idFrom = toolPtr->uId;
  ttnmdi.hdr.code = TTN_GETDISPINFOA; /* == TTN_NEEDTEXTA */
  ttnmdi.lpszText = (GXLPSTR)&ttnmdi.szText;
  ttnmdi.uFlags = toolPtr->uFlags;
  ttnmdi.lParam = toolPtr->lParam;

  TRACE("hdr.idFrom = %lx\n", ttnmdi.hdr.idFrom);
  gxSendMessageW(toolPtr->hwnd, GXWM_NOTIFY,
    (GXWPARAM)toolPtr->uId, (GXLPARAM)&ttnmdi);

  if (GXIS_INTRESOURCE(ttnmdi.lpszText)) {
    gxLoadStringW(ttnmdi.hinst, GXLOWORD(ttnmdi.lpszText),
      infoPtr->szTipText, INFOTIPSIZE);
    if (ttnmdi.uFlags & TTF_DI_SETITEM) {
      toolPtr->hinst = ttnmdi.hinst;
      toolPtr->lpszText = (GXLPWSTR)ttnmdi.lpszText;
    }
  }
  else if (ttnmdi.lpszText == 0) {
    infoPtr->szTipText[0] = '\0';
  }
  else if (ttnmdi.lpszText != GXLPSTR_TEXTCALLBACKA) {
    gxStr_GetPtrAtoW(ttnmdi.lpszText, infoPtr->szTipText, INFOTIPSIZE);
    if (ttnmdi.uFlags & TTF_DI_SETITEM) {
      toolPtr->hinst = 0;
      toolPtr->lpszText = NULL;
      gxStr_SetPtrW(&toolPtr->lpszText, infoPtr->szTipText);
    }
  }
  else {
    ERR("recursive text callback!\n");
    infoPtr->szTipText[0] = '\0';
  }

  /* no text available - try calling parent instead as per native */
  /* FIXME: Unsure if SETITEM should save the value or not        */
  if (infoPtr->szTipText[0] == 0x00) {

    gxSendMessageW(gxGetParent(toolPtr->hwnd), GXWM_NOTIFY,
      (GXWPARAM)toolPtr->uId, (GXLPARAM)&ttnmdi);

    if (GXIS_INTRESOURCE(ttnmdi.lpszText)) {
      gxLoadStringW(ttnmdi.hinst, GXLOWORD(ttnmdi.lpszText),
        infoPtr->szTipText, INFOTIPSIZE);
    } else if (ttnmdi.lpszText &&
      ttnmdi.lpszText != GXLPSTR_TEXTCALLBACKA) {
        gxStr_GetPtrAtoW(ttnmdi.lpszText, infoPtr->szTipText, INFOTIPSIZE);
    }
  }
}

static void TOOLTIPS_GetDispInfoW(GXHWND hwnd, TOOLTIPS_INFO *infoPtr, TTTOOL_INFO *toolPtr)
{
  GXNMTTDISPINFOW ttnmdi;

  /* fill GXNMHDR struct */
  ZeroMemory (&ttnmdi, sizeof(GXNMTTDISPINFOW));
  ttnmdi.hdr.hwndFrom = hwnd;
  ttnmdi.hdr.idFrom = toolPtr->uId;
  ttnmdi.hdr.code = TTN_GETDISPINFOW; /* == TTN_NEEDTEXTW */
  ttnmdi.lpszText = (GXLPWSTR)&ttnmdi.szText;
  ttnmdi.uFlags = toolPtr->uFlags;
  ttnmdi.lParam = toolPtr->lParam;

  TRACE("hdr.idFrom = %lx\n", ttnmdi.hdr.idFrom);
  gxSendMessageW(toolPtr->hwnd, GXWM_NOTIFY,
    (GXWPARAM)toolPtr->uId, (GXLPARAM)&ttnmdi);

  if (GXIS_INTRESOURCE(ttnmdi.lpszText)) {
    gxLoadStringW(ttnmdi.hinst, GXLOWORD(ttnmdi.lpszText),
      infoPtr->szTipText, INFOTIPSIZE);
    if (ttnmdi.uFlags & TTF_DI_SETITEM) {
      toolPtr->hinst = ttnmdi.hinst;
      toolPtr->lpszText = ttnmdi.lpszText;
    }
  }
  else if (ttnmdi.lpszText == 0) {
    infoPtr->szTipText[0] = '\0';
  }
  else if (ttnmdi.lpszText != GXLPSTR_TEXTCALLBACKW) {
    gxStr_GetPtrW(ttnmdi.lpszText, infoPtr->szTipText, INFOTIPSIZE);
    if (ttnmdi.uFlags & TTF_DI_SETITEM) {
      toolPtr->hinst = 0;
      toolPtr->lpszText = NULL;
      gxStr_SetPtrW(&toolPtr->lpszText, infoPtr->szTipText);
    }
  }
  else {
    ERR("recursive text callback!\n");
    infoPtr->szTipText[0] = '\0';
  }

  /* no text available - try calling parent instead as per native */
  /* FIXME: Unsure if SETITEM should save the value or not        */
  if (infoPtr->szTipText[0] == 0x00) {

    gxSendMessageW(gxGetParent(toolPtr->hwnd), GXWM_NOTIFY,
      (GXWPARAM)toolPtr->uId, (GXLPARAM)&ttnmdi);

    if (GXIS_INTRESOURCE(ttnmdi.lpszText)) {
      gxLoadStringW(ttnmdi.hinst, GXLOWORD(ttnmdi.lpszText),
        infoPtr->szTipText, INFOTIPSIZE);
    } else if (ttnmdi.lpszText &&
      ttnmdi.lpszText != GXLPSTR_TEXTCALLBACKW) {
        gxStr_GetPtrW(ttnmdi.lpszText, infoPtr->szTipText, INFOTIPSIZE);
    }
  }

}

static void
TOOLTIPS_GetTipText (GXHWND hwnd, TOOLTIPS_INFO *infoPtr, GXINT nTool)
{
  TTTOOL_INFO *toolPtr = &infoPtr->m_pTools[nTool];

  if (GXIS_INTRESOURCE(toolPtr->lpszText) && toolPtr->hinst) {
    /* load a resource */
    TRACE("load res string %p %x\n",
      toolPtr->hinst, GXLOWORD(toolPtr->lpszText));
    gxLoadStringW (toolPtr->hinst, GXLOWORD(toolPtr->lpszText),
      infoPtr->szTipText, INFOTIPSIZE);
  }
  else if (toolPtr->lpszText) {
    if (toolPtr->lpszText == GXLPSTR_TEXTCALLBACKW) {
      if (toolPtr->bNotifyUnicode)
        TOOLTIPS_GetDispInfoW(hwnd, infoPtr, toolPtr);
      else
        TOOLTIPS_GetDispInfoA(hwnd, infoPtr, toolPtr);
    }
    else {
      /* the item is a usual (unicode) text */
      GXSTRCPYN (infoPtr->szTipText, toolPtr->lpszText, INFOTIPSIZE);
    }
  }
  else {
    /* no text available */
    infoPtr->szTipText[0] = '\0';
  }

  TRACE("%s\n", debugstr_w(infoPtr->szTipText));
}


static void
TOOLTIPS_CalcTipSize (GXHWND hwnd, const TOOLTIPS_INFO *infoPtr, GXLPSIZE lpSize)
{
  GXHDC hdc;
  GXHFONT hOldFont;
  GXDWORD style = gxGetWindowLongW(hwnd, GXGWL_STYLE);
  GXUINT uFlags = GXDT_EXTERNALLEADING | GXDT_CALCRECT;
  GXRECT rc(0, 0, 0, 0);
  GXSIZE title = {0, 0};

  if (infoPtr->nMaxTipWidth > -1) {
    rc.right = infoPtr->nMaxTipWidth;
    uFlags |= GXDT_WORDBREAK;
  }
  if (style & TTS_NOPREFIX)
    uFlags |= GXDT_NOPREFIX;
  TRACE("%s\n", debugstr_w(infoPtr->szTipText));

  hdc = gxGetDC (hwnd);
  if (infoPtr->pszTitle)
  {
    GXRECT rcTitle(0, 0, 0, 0);
    TRACE("title %s\n", debugstr_w(infoPtr->pszTitle));
    if (infoPtr->hTitleIcon)
    {
      title.cx = ICON_WIDTH;
      title.cy = ICON_HEIGHT;
    }
    if (title.cx != 0) title.cx += BALLOON_ICON_TITLE_SPACING;
    hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hTitleFont);
    gxDrawTextW(hdc, infoPtr->pszTitle, -1, &rcTitle, GXDT_SINGLELINE | GXDT_NOPREFIX | GXDT_CALCRECT);
    gxSelectObject (hdc, hOldFont);
    title.cy = max(title.cy, rcTitle.bottom - rcTitle.top) + BALLOON_TITLE_TEXT_SPACING;
    title.cx += (rcTitle.right - rcTitle.left);
  }
  hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hFont);
  gxDrawTextW (hdc, infoPtr->szTipText, -1, &rc, uFlags);
  gxSelectObject (hdc, hOldFont);
  gxReleaseDC (hwnd, hdc);

  if ((style & TTS_BALLOON) || infoPtr->pszTitle)
  {
    lpSize->cx = max(rc.right - rc.left, title.cx) + 2*BALLOON_TEXT_MARGIN +
      infoPtr->rcMargin.left + infoPtr->rcMargin.right;
    lpSize->cy = title.cy + rc.bottom - rc.top + 2*BALLOON_TEXT_MARGIN +
      infoPtr->rcMargin.bottom + infoPtr->rcMargin.top +
      BALLOON_STEMHEIGHT;
  }
  else
  {
    lpSize->cx = rc.right - rc.left + 2*NORMAL_TEXT_MARGIN +
      infoPtr->rcMargin.left + infoPtr->rcMargin.right;
    lpSize->cy = rc.bottom - rc.top + 2*NORMAL_TEXT_MARGIN +
      infoPtr->rcMargin.bottom + infoPtr->rcMargin.top;
  }
}


static void
TOOLTIPS_Show (GXHWND hwnd, TOOLTIPS_INFO *infoPtr, GXBOOL track_activate)
{
  TTTOOL_INFO *toolPtr;
  GXHMONITOR monitor;
  GXMONITORINFO mon_info;
  GXRECT rect;
  GXSIZE size;
  GXNMHDR  hdr;
  int ptfx = 0;
  GXDWORD style = gxGetWindowLongW(hwnd, GXGWL_STYLE);
  GXINT nTool;

  if (track_activate)
  {
    if (infoPtr->nTrackTool == -1)
    {
      TRACE("invalid tracking tool (-1)!\n");
      return;
    }
    nTool = infoPtr->nTrackTool;
  }
  else
  {
    if (infoPtr->nTool == -1)
    {
      TRACE("invalid tool (-1)!\n");
      return;
    }
    nTool = infoPtr->nTool;
  }

  TRACE("Show tooltip pre %d! (%p)\n", nTool, hwnd);

  TOOLTIPS_GetTipText (hwnd, infoPtr, nTool);

  if (infoPtr->szTipText[0] == '\0')
    return;

  toolPtr = &infoPtr->m_pTools[nTool];

  if (!track_activate)
    infoPtr->nCurrentTool = infoPtr->nTool;

  TRACE("Show tooltip %d!\n", nTool);

  hdr.hwndFrom = hwnd;
  hdr.idFrom = toolPtr->uId;
  hdr.code = TTN_SHOW;
  gxSendMessageW (toolPtr->hwnd, GXWM_NOTIFY,
    (GXWPARAM)toolPtr->uId, (GXLPARAM)&hdr);

  TRACE("%s\n", debugstr_w(infoPtr->szTipText));

  TOOLTIPS_CalcTipSize (hwnd, infoPtr, &size);
  TRACE("size %d x %d\n", size.cx, size.cy);

  if (track_activate)
  {
    rect.left = infoPtr->xTrackPos;
    rect.top  = infoPtr->yTrackPos;
    ptfx = rect.left;

    if (toolPtr->uFlags & TTF_CENTERTIP)
    {
      rect.left -= (size.cx / 2);
      if (!(style & TTS_BALLOON))
        rect.top  -= (size.cy / 2);
    }
    infoPtr->bToolBelow = TRUE;

    if (!(toolPtr->uFlags & TTF_ABSOLUTE))
    {
      if (style & TTS_BALLOON)
        rect.left -= BALLOON_STEMINDENT;
      else
      {
        GXRECT rcTool;

        if (toolPtr->uFlags & TTF_IDISHWND)
          gxGetWindowRect ((GXHWND)toolPtr->uId, &rcTool);
        else
        {
          rcTool = toolPtr->rect;
          gxMapWindowPoints (toolPtr->hwnd, NULL, (LPGXPOINT)&rcTool, 2);
        }

        /* smart placement */
        if ((rect.left + size.cx > rcTool.left) && (rect.left < rcTool.right) &&
          (rect.top + size.cy > rcTool.top) && (rect.top < rcTool.bottom))
          rect.left = rcTool.right;
      }
    }
  }
  else
  {
    if (toolPtr->uFlags & TTF_CENTERTIP)
    {
      GXRECT rc;

      if (toolPtr->uFlags & TTF_IDISHWND)
        gxGetWindowRect ((GXHWND)toolPtr->uId, &rc);
      else {
        rc = toolPtr->rect;
        gxMapWindowPoints (toolPtr->hwnd, NULL, (LPGXPOINT)&rc, 2);
      }
      rect.left = (rc.left + rc.right - size.cx) / 2;
      if (style & TTS_BALLOON)
      {
        ptfx = rc.left + ((rc.right - rc.left) / 2);

        /* CENTERTIP ballon tooltips default to below the field
        * if they fit on the screen */
        if (rc.bottom + size.cy > gxGetSystemMetrics(GXSM_CYSCREEN))
        {
          rect.top = rc.top - size.cy;
          infoPtr->bToolBelow = FALSE;
        }
        else
        {
          infoPtr->bToolBelow = TRUE;
          rect.top = rc.bottom;
        }
        rect.left = max(0, rect.left - BALLOON_STEMINDENT);
      }
      else
      {
        rect.top  = rc.bottom + 2;
        infoPtr->bToolBelow = TRUE;
      }
    }
    else
    {
      gxGetCursorPos ((LPGXPOINT)&rect);
      if (style & TTS_BALLOON)
      {
        ptfx = rect.left;
        if(rect.top - size.cy >= 0)
        {
          rect.top -= size.cy;
          infoPtr->bToolBelow = FALSE;
        }
        else
        {
          infoPtr->bToolBelow = TRUE;
          rect.top += 20;
        }
        rect.left = max(0, rect.left - BALLOON_STEMINDENT);
      }
      else
      {
        rect.top += 20;
        infoPtr->bToolBelow = TRUE;
      }
    }
  }

  TRACE("pos %d - %d\n", rect.left, rect.top);

  rect.right = rect.left + size.cx;
  rect.bottom = rect.top + size.cy;

  /* check position */

  monitor = gxMonitorFromRect( &rect, GXMONITOR_DEFAULTTOPRIMARY );
  mon_info.cbSize = sizeof(mon_info);
  gxGetMonitorInfoW( monitor, &mon_info );

  if( rect.right > mon_info.rcWork.right ) {
    rect.left -= rect.right - mon_info.rcWork.right + 2;
    rect.right = mon_info.rcWork.right - 2;
  }
  if (rect.left < mon_info.rcWork.left) rect.left = mon_info.rcWork.left;

  if( rect.bottom > mon_info.rcWork.bottom ) {
    GXRECT rc;

    if (toolPtr->uFlags & TTF_IDISHWND)
      gxGetWindowRect ((GXHWND)toolPtr->uId, &rc);
    else {
      rc = toolPtr->rect;
      gxMapWindowPoints (toolPtr->hwnd, NULL, (LPGXPOINT)&rc, 2);
    }
    rect.bottom = rc.top - 2;
    rect.top = rect.bottom - size.cy;
  }

  gxAdjustWindowRectEx (&rect, gxGetWindowLongW (hwnd, GXGWL_STYLE),
    FALSE, gxGetWindowLongW (hwnd, GXGWL_EXSTYLE));

  if (style & TTS_BALLOON)
  {
    GXHRGN hRgn;
    GXHRGN hrStem;
    GXPOINT pts[3];

    ptfx -= rect.left;

    if(infoPtr->bToolBelow)
    {
      pts[0].x = ptfx;
      pts[0].y = 0;
      pts[1].x = max(BALLOON_STEMINDENT, ptfx - (BALLOON_STEMWIDTH / 2));
      pts[1].y = BALLOON_STEMHEIGHT;
      pts[2].x = pts[1].x + BALLOON_STEMWIDTH;
      pts[2].y = pts[1].y;
      if(pts[2].x > (rect.right - rect.left) - BALLOON_STEMINDENT)
      {
        pts[2].x = (rect.right - rect.left) - BALLOON_STEMINDENT;
        pts[1].x = pts[2].x - BALLOON_STEMWIDTH;
      }
    }
    else
    {
      pts[0].x = max(BALLOON_STEMINDENT, ptfx - (BALLOON_STEMWIDTH / 2));
      pts[0].y = (rect.bottom - rect.top) - BALLOON_STEMHEIGHT;
      pts[1].x = pts[0].x + BALLOON_STEMWIDTH;
      pts[1].y = pts[0].y;
      pts[2].x = ptfx;
      pts[2].y = (rect.bottom - rect.top);
      if(pts[1].x > (rect.right - rect.left) - BALLOON_STEMINDENT)
      {
        pts[1].x = (rect.right - rect.left) - BALLOON_STEMINDENT;
        pts[0].x = pts[1].x - BALLOON_STEMWIDTH;
      }
    }

    hrStem = gxCreatePolygonRgn(pts, sizeof(pts) / sizeof(pts[0]), GXALTERNATE);

    hRgn = gxCreateRoundRectRgn(0,
      (infoPtr->bToolBelow ? BALLOON_STEMHEIGHT : 0),
      rect.right - rect.left,
      (infoPtr->bToolBelow ? rect.bottom - rect.top : rect.bottom - rect.top - BALLOON_STEMHEIGHT),
      BALLOON_ROUNDEDNESS, BALLOON_ROUNDEDNESS);

    gxCombineRgn(hRgn, hRgn, hrStem, GXRGN_OR);
    gxDeleteObject(hrStem);

    gxSetWindowRgn(hwnd, hRgn, FALSE);
    /* we don't free the region handle as the system deletes it when 
    * it is no longer needed */
  }

  gxSetWindowPos (hwnd, GXHWND_TOP, rect.left, rect.top,
    rect.right - rect.left, rect.bottom - rect.top,
    GXSWP_SHOWWINDOW | GXSWP_NOACTIVATE);

  /* repaint the tooltip */
  gxInvalidateRect(hwnd, NULL, TRUE);
  gxUpdateWindow(hwnd);

  if (!track_activate)
  {
    gxSetTimer (hwnd, ID_TIMERPOP, infoPtr->nAutoPopTime, 0);
    TRACE("timer 2 started!\n");
    gxSetTimer (hwnd, ID_TIMERLEAVE, infoPtr->nReshowTime, 0);
    TRACE("timer 3 started!\n");
  }
}


static void
TOOLTIPS_Hide (GXHWND hwnd, TOOLTIPS_INFO *infoPtr)
{
  TTTOOL_INFO *toolPtr;
  GXNMHDR hdr;

  TRACE("Hide tooltip %d! (%p)\n", infoPtr->nCurrentTool, hwnd);

  if (infoPtr->nCurrentTool == -1)
    return;

  toolPtr = &infoPtr->m_pTools[infoPtr->nCurrentTool];
  gxKillTimer (hwnd, ID_TIMERPOP);

  hdr.hwndFrom = hwnd;
  hdr.idFrom = toolPtr->uId;
  hdr.code = TTN_POP;
  gxSendMessageW (toolPtr->hwnd, GXWM_NOTIFY,
    (GXWPARAM)toolPtr->uId, (GXLPARAM)&hdr);

  infoPtr->nCurrentTool = -1;

  gxSetWindowPos (hwnd, GXHWND_TOP, 0, 0, 0, 0,
    GXSWP_NOZORDER | GXSWP_HIDEWINDOW | GXSWP_NOACTIVATE);
}


static void
TOOLTIPS_TrackShow (GXHWND hwnd, TOOLTIPS_INFO *infoPtr)
{
  TOOLTIPS_Show(hwnd, infoPtr, TRUE);
}


static void
TOOLTIPS_TrackHide (GXHWND hwnd, const TOOLTIPS_INFO *infoPtr)
{
  TTTOOL_INFO *toolPtr;
  GXNMHDR hdr;

  TRACE("hide tracking tooltip %d\n", infoPtr->nTrackTool);

  if (infoPtr->nTrackTool == -1)
    return;

  toolPtr = &infoPtr->m_pTools[infoPtr->nTrackTool];

  hdr.hwndFrom = hwnd;
  hdr.idFrom = toolPtr->uId;
  hdr.code = TTN_POP;
  gxSendMessageW (toolPtr->hwnd, GXWM_NOTIFY,
    (GXWPARAM)toolPtr->uId, (GXLPARAM)&hdr);

  gxSetWindowPos (hwnd, GXHWND_TOP, 0, 0, 0, 0,
    GXSWP_NOZORDER | GXSWP_HIDEWINDOW | GXSWP_NOACTIVATE);
}


static GXINT
TOOLTIPS_GetToolFromInfoA (const TOOLTIPS_INFO *infoPtr, const GXTTTOOLINFOA *lpToolInfo)
{
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if (!(toolPtr->uFlags & TTF_IDISHWND) &&
      (lpToolInfo->hwnd == toolPtr->hwnd) &&
      (lpToolInfo->uId == toolPtr->uId))
      return nTool;
  }

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if ((toolPtr->uFlags & TTF_IDISHWND) &&
      (lpToolInfo->uId == toolPtr->uId))
      return nTool;
  }

  return -1;
}


static GXINT
TOOLTIPS_GetToolFromInfoW (const TOOLTIPS_INFO *infoPtr, const GXTTTOOLINFOW *lpToolInfo)
{
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if (!(toolPtr->uFlags & TTF_IDISHWND) &&
      (lpToolInfo->hwnd == toolPtr->hwnd) &&
      (lpToolInfo->uId == toolPtr->uId))
      return nTool;
  }

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if ((toolPtr->uFlags & TTF_IDISHWND) &&
      (lpToolInfo->uId == toolPtr->uId))
      return nTool;
  }

  return -1;
}


static GXINT
TOOLTIPS_GetToolFromPoint (const TOOLTIPS_INFO *infoPtr, GXHWND hwnd, const GXPOINT *lpPt)
{
  TTTOOL_INFO *toolPtr;
  GXINT  nTool;

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if (!(toolPtr->uFlags & TTF_IDISHWND)) {
      if (hwnd != toolPtr->hwnd)
        continue;
      if (!gxPtInRect (&toolPtr->rect, *lpPt))
        continue;
      return nTool;
    }
  }

  for (nTool = 0; nTool < infoPtr->m_uNumTools; nTool++) {
    toolPtr = &infoPtr->m_pTools[nTool];

    if (toolPtr->uFlags & TTF_IDISHWND) {
      if ((GXHWND)toolPtr->uId == hwnd)
        return nTool;
    }
  }

  return -1;
}


static GXBOOL
TOOLTIPS_IsWindowActive (GXHWND hwnd)
{
  GXHWND hwndActive = gxGetActiveWindow ();
  if (!hwndActive)
    return FALSE;
  if (hwndActive == hwnd)
    return TRUE;
  return gxIsChild (hwndActive, hwnd);
}


static GXINT
TOOLTIPS_CheckTool (GXHWND hwnd, GXBOOL bShowTest)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXPOINT pt;
  GXHWND hwndTool;
  GXINT nTool;

  gxGetCursorPos (&pt);
  hwndTool = (GXHWND)gxSendMessageW (hwnd, GXTTM_WINDOWFROMPOINT, 0, (GXLPARAM)&pt);
  if (hwndTool == 0)
    return -1;

  gxScreenToClient (hwndTool, &pt);
  nTool = TOOLTIPS_GetToolFromPoint (infoPtr, hwndTool, &pt);
  if (nTool == -1)
    return -1;

  if (!(gxGetWindowLongW (hwnd, GXGWL_STYLE) & TTS_ALWAYSTIP) && bShowTest) {
    if (!TOOLTIPS_IsWindowActive (gxGetWindow (hwnd, GXGW_OWNER)))
      return -1;
  }

  TRACE("tool %d\n", nTool);

  return nTool;
}


static GXLRESULT
TOOLTIPS_Activate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  infoPtr->bActive = (GXBOOL)wParam;

  if (infoPtr->bActive)
    TRACE("activate!\n");

  if (!(infoPtr->bActive) && (infoPtr->nCurrentTool != -1))
    TOOLTIPS_Hide (hwnd, infoPtr);

  return 0;
}


GXLRESULT TOOLTIPS_INFO::AddToolA (GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  GXTTTOOLINFOW ToolInfoW;

  clStringW strText = lpToolInfo->lpszText;

  ToolInfoW.cbSize      = sizeof(GXTTTOOLINFOW);
  ToolInfoW.uFlags      = lpToolInfo->uFlags;
  ToolInfoW.hwnd        = lpToolInfo->hwnd;
  ToolInfoW.uId         = lpToolInfo->uId;
  ToolInfoW.rect        = lpToolInfo->rect;
  ToolInfoW.hinst       = lpToolInfo->hinst;
  ToolInfoW.lpszText    = strText.GetBuffer();
  ToolInfoW.lParam      = lpToolInfo->lParam;
  ToolInfoW.lpReserved  = lpToolInfo->lpReserved;

  return AddToolW(hWnd, wParam, (GXLPARAM)&ToolInfoW);
}


GXLRESULT TOOLTIPS_INFO::AddToolW(GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam)
{
  //TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nResult;

  if (lpToolInfo == NULL) {
    return FALSE;
  }

  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE) {
    return FALSE;
  }

  TRACE("add tool (%p) %p %ld%s!\n",
    hWnd, lpToolInfo->hwnd, lpToolInfo->uId,
    (lpToolInfo->uFlags & TTF_IDISHWND) ? " TTF_IDISHWND" : "");

  if (m_uNumTools == 0) {
    m_pTools = (TTTOOL_INFO*)Alloc (sizeof(TTTOOL_INFO));
    toolPtr = m_pTools;
    TRACE("alloc infoPtr->tools:%p\n", m_pTools);
  }
  else {
    TTTOOL_INFO *oldTools = m_pTools;
    m_pTools = (TTTOOL_INFO*)Alloc(sizeof(TTTOOL_INFO) * (m_uNumTools + 1));
    memcpy (m_pTools, oldTools, m_uNumTools * sizeof(TTTOOL_INFO));
    Free (oldTools);
    toolPtr = &m_pTools[m_uNumTools];
    TRACE("realloc infoPtr->tools:%p\n", m_pTools);
  }

  m_uNumTools++;

  /* copy tool data */
  toolPtr->uFlags = lpToolInfo->uFlags;
  toolPtr->hwnd   = lpToolInfo->hwnd;
  toolPtr->uId    = lpToolInfo->uId;
  toolPtr->rect   = lpToolInfo->rect;
  toolPtr->hinst  = lpToolInfo->hinst;

  if (GXIS_INTRESOURCE(lpToolInfo->lpszText)) {
    TRACE("add string id %x\n", GXLOWORD(lpToolInfo->lpszText));
    toolPtr->lpszText = lpToolInfo->lpszText;
  }
  else if (lpToolInfo->lpszText) {
    if (lpToolInfo->lpszText == GXLPSTR_TEXTCALLBACKW) {
      TRACE("add CALLBACK!\n");
      toolPtr->lpszText = GXLPSTR_TEXTCALLBACKW;
    }
    else {
      GXINT len = GXSTRLEN (lpToolInfo->lpszText);
      TRACE("add text %s!\n",
        debugstr_w(lpToolInfo->lpszText));
      toolPtr->lpszText =  (GXLPWSTR)Alloc ((len + 1)*sizeof(GXWCHAR));
      GXSTRCPY (toolPtr->lpszText, lpToolInfo->lpszText);
    }
  }

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOW))
    toolPtr->lParam = lpToolInfo->lParam;

  /* install subclassing hook */
  if (toolPtr->uFlags & TTF_SUBCLASS) {
    if (toolPtr->uFlags & TTF_IDISHWND) {
      gxSetWindowSubclass((GXHWND)toolPtr->uId, TOOLTIPS_SubclassProc, 1, (GXDWORD_PTR)hWnd);
    }
    else {
      gxSetWindowSubclass(toolPtr->hwnd, TOOLTIPS_SubclassProc, 1, (GXDWORD_PTR)hWnd);
    }
    TRACE("subclassing installed!\n");
  }

  nResult = (GXINT) gxSendMessageW (toolPtr->hwnd, GXWM_NOTIFYFORMAT,
    (GXWPARAM)hWnd, (GXLPARAM)GXNF_QUERY);
  if (nResult == GXNFR_ANSI) {
    toolPtr->bNotifyUnicode = FALSE;
    TRACE(" -- WM_NOTIFYFORMAT returns: GXNFR_ANSI\n");
  } else if (nResult == GXNFR_UNICODE) {
    toolPtr->bNotifyUnicode = TRUE;
    TRACE(" -- WM_NOTIFYFORMAT returns: GXNFR_UNICODE\n");
  } else {
    TRACE (" -- WM_NOTIFYFORMAT returns: error!\n");
  }

  return TRUE;
}


static void
TOOLTIPS_DelToolCommon (GXHWND hwnd, TOOLTIPS_INFO *infoPtr, GXINT nTool)
{
  TTTOOL_INFO *toolPtr;

  TRACE("tool %d\n", nTool);

  if (nTool == -1)
    return;

  /* make sure the tooltip has disappeared before deleting it */
  TOOLTIPS_Hide(hwnd, infoPtr);

  /* delete text string */
  toolPtr = &infoPtr->m_pTools[nTool];
  if (toolPtr->lpszText) {
    if ( (toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW) &&
      !GXIS_INTRESOURCE(toolPtr->lpszText) )
      Free (toolPtr->lpszText);
  }

  /* remove subclassing */
  if (toolPtr->uFlags & TTF_SUBCLASS) {
    if (toolPtr->uFlags & TTF_IDISHWND) {
      gxRemoveWindowSubclass((GXHWND)toolPtr->uId, TOOLTIPS_SubclassProc, 1);
    }
    else {
      gxRemoveWindowSubclass(toolPtr->hwnd, TOOLTIPS_SubclassProc, 1);
    }
  }

  /* delete tool from tool list */
  if (infoPtr->m_uNumTools == 1) {
    Free (infoPtr->m_pTools);
    infoPtr->m_pTools = NULL;
  }
  else {
    TTTOOL_INFO *oldTools = infoPtr->m_pTools;
    infoPtr->m_pTools =
      (TTTOOL_INFO*)Alloc (sizeof(TTTOOL_INFO) * (infoPtr->m_uNumTools - 1));

    if (nTool > 0)
      memcpy (&infoPtr->m_pTools[0], &oldTools[0],
      nTool * sizeof(TTTOOL_INFO));

    if (nTool < infoPtr->m_uNumTools - 1)
      memcpy (&infoPtr->m_pTools[nTool], &oldTools[nTool + 1],
      (infoPtr->m_uNumTools - nTool - 1) * sizeof(TTTOOL_INFO));

    Free (oldTools);
  }

  /* update any indices affected by delete */

  /* destroying tool that mouse was on on last relayed mouse move */
  if (infoPtr->nTool == nTool)
    /* -1 means no current tool (0 means first tool) */
    infoPtr->nTool = -1;
  else if (infoPtr->nTool > nTool)
    infoPtr->nTool--;

  if (infoPtr->nTrackTool == nTool)
    /* -1 means no current tool (0 means first tool) */
    infoPtr->nTrackTool = -1;
  else if (infoPtr->nTrackTool > nTool)
    infoPtr->nTrackTool--;

  if (infoPtr->nCurrentTool == nTool)
    /* -1 means no current tool (0 means first tool) */
    infoPtr->nCurrentTool = -1;
  else if (infoPtr->nCurrentTool > nTool)
    infoPtr->nCurrentTool--;

  infoPtr->m_uNumTools--;
}

static GXLRESULT
TOOLTIPS_DelToolA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return 0;
  if (infoPtr->m_uNumTools == 0)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);

  TOOLTIPS_DelToolCommon (hwnd, infoPtr, nTool);

  return 0;
}


static GXLRESULT
TOOLTIPS_DelToolW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return 0;
  if (infoPtr->m_uNumTools == 0)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);

  TOOLTIPS_DelToolCommon (hwnd, infoPtr, nTool);

  return 0;
}


static GXLRESULT
TOOLTIPS_EnumToolsA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXUINT uIndex = (GXUINT)wParam;
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  TTTOOL_INFO *toolPtr;

  if (lpToolInfo == NULL)
    return FALSE;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return FALSE;
  if (uIndex >= infoPtr->m_uNumTools)
    return FALSE;

  TRACE("index=%u\n", uIndex);

  toolPtr = &infoPtr->m_pTools[uIndex];

  /* copy tool data */
  lpToolInfo->uFlags   = toolPtr->uFlags;
  lpToolInfo->hwnd     = toolPtr->hwnd;
  lpToolInfo->uId      = toolPtr->uId;
  lpToolInfo->rect     = toolPtr->rect;
  lpToolInfo->hinst    = toolPtr->hinst;
  /*    lpToolInfo->lpszText = toolPtr->lpszText; */
  lpToolInfo->lpszText = NULL;  /* FIXME */

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOA))
    lpToolInfo->lParam = toolPtr->lParam;

  return TRUE;
}


static GXLRESULT
TOOLTIPS_EnumToolsW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXUINT uIndex = (GXUINT)wParam;
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;

  if (lpToolInfo == NULL)
    return FALSE;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return FALSE;
  if (uIndex >= infoPtr->m_uNumTools)
    return FALSE;

  TRACE("index=%u\n", uIndex);

  toolPtr = &infoPtr->m_pTools[uIndex];

  /* copy tool data */
  lpToolInfo->uFlags   = toolPtr->uFlags;
  lpToolInfo->hwnd     = toolPtr->hwnd;
  lpToolInfo->uId      = toolPtr->uId;
  lpToolInfo->rect     = toolPtr->rect;
  lpToolInfo->hinst    = toolPtr->hinst;
  /*    lpToolInfo->lpszText = toolPtr->lpszText; */
  lpToolInfo->lpszText = NULL;  /* FIXME */

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOW))
    lpToolInfo->lParam = toolPtr->lParam;

  return TRUE;
}

static GXLRESULT
TOOLTIPS_GetBubbleSize (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  GXINT nTool;
  GXSIZE size;

  if (lpToolInfo == NULL)
    return FALSE;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  TRACE("tool %d\n", nTool);

  TOOLTIPS_CalcTipSize (hwnd, infoPtr, &size);
  TRACE("size %d x %d\n", size.cx, size.cy);

  return GXMAKELRESULT(size.cx, size.cy);
}

static GXLRESULT
TOOLTIPS_GetCurrentToolA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  TTTOOL_INFO *toolPtr;

  if (lpToolInfo) {
    if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
      return FALSE;

    if (infoPtr->nCurrentTool > -1) {
      toolPtr = &infoPtr->m_pTools[infoPtr->nCurrentTool];

      /* copy tool data */
      lpToolInfo->uFlags   = toolPtr->uFlags;
      lpToolInfo->rect     = toolPtr->rect;
      lpToolInfo->hinst    = toolPtr->hinst;
      /*      lpToolInfo->lpszText = toolPtr->lpszText; */
      lpToolInfo->lpszText = NULL;  /* FIXME */

      if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOA))
        lpToolInfo->lParam = toolPtr->lParam;

      return TRUE;
    }
    else
      return FALSE;
  }
  else
    return (infoPtr->nCurrentTool != -1);
}


static GXLRESULT
TOOLTIPS_GetCurrentToolW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;

  if (lpToolInfo) {
    if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
      return FALSE;

    if (infoPtr->nCurrentTool > -1) {
      toolPtr = &infoPtr->m_pTools[infoPtr->nCurrentTool];

      /* copy tool data */
      lpToolInfo->uFlags   = toolPtr->uFlags;
      lpToolInfo->rect     = toolPtr->rect;
      lpToolInfo->hinst    = toolPtr->hinst;
      /*      lpToolInfo->lpszText = toolPtr->lpszText; */
      lpToolInfo->lpszText = NULL;  /* FIXME */

      if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOW))
        lpToolInfo->lParam = toolPtr->lParam;

      return TRUE;
    }
    else
      return FALSE;
  }
  else
    return (infoPtr->nCurrentTool != -1);
}


static GXLRESULT
TOOLTIPS_GetDelayTime (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  switch (wParam) {
  case TTDT_RESHOW:
    return infoPtr->nReshowTime;

  case TTDT_AUTOPOP:
    return infoPtr->nAutoPopTime;

  case TTDT_INITIAL:
  case TTDT_AUTOMATIC: /* Apparently TTDT_AUTOMATIC returns TTDT_INITIAL */
    return infoPtr->nInitialTime;

  default:
    WARN("Invalid wParam %lx\n", wParam);
    break;
  }

  return -1;
}


static GXLRESULT
TOOLTIPS_GetMargin (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  LPGXRECT lpRect = (LPGXRECT)lParam;

  lpRect->left   = infoPtr->rcMargin.left;
  lpRect->right  = infoPtr->rcMargin.right;
  lpRect->bottom = infoPtr->rcMargin.bottom;
  lpRect->top    = infoPtr->rcMargin.top;

  return 0;
}


static inline GXLRESULT
TOOLTIPS_GetMaxTipWidth (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  return infoPtr->nMaxTipWidth;
}


static GXLRESULT
TOOLTIPS_GetTextA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  /* NB this API is broken, there is no way for the app to determine
  what size buffer it requires nor a way to specify how long the
  one it supplies is.  We'll assume it's up to INFOTIPSIZE */

  gxWideCharToMultiByte(GXCP_ACP, 0, infoPtr->m_pTools[nTool].lpszText, -1,
    lpToolInfo->lpszText, INFOTIPSIZE, NULL, NULL);

  return 0;
}


static GXLRESULT
TOOLTIPS_GetTextW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  GXSTRCPY (lpToolInfo->lpszText, infoPtr->m_pTools[nTool].lpszText);

  return 0;
}


static inline GXLRESULT
TOOLTIPS_GetTipBkColor (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  return infoPtr->clrBk;
}


static inline GXLRESULT
TOOLTIPS_GetTipTextColor (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  return infoPtr->clrText;
}


static inline GXLRESULT
TOOLTIPS_GetToolCount (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  return infoPtr->m_uNumTools;
}


static GXLRESULT
TOOLTIPS_GetToolInfoA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return FALSE;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return FALSE;
  if (infoPtr->m_uNumTools == 0)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);
  if (nTool == -1)
    return FALSE;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool data */
  lpToolInfo->uFlags   = toolPtr->uFlags;
  lpToolInfo->rect     = toolPtr->rect;
  lpToolInfo->hinst    = toolPtr->hinst;
  /*    lpToolInfo->lpszText = toolPtr->lpszText; */
  lpToolInfo->lpszText = NULL;  /* FIXME */

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOA))
    lpToolInfo->lParam = toolPtr->lParam;

  return TRUE;
}


static GXLRESULT
TOOLTIPS_GetToolInfoW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return FALSE;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return FALSE;
  if (infoPtr->m_uNumTools == 0)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);
  if (nTool == -1)
    return FALSE;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool data */
  lpToolInfo->uFlags   = toolPtr->uFlags;
  lpToolInfo->rect     = toolPtr->rect;
  lpToolInfo->hinst    = toolPtr->hinst;
  /*    lpToolInfo->lpszText = toolPtr->lpszText; */
  lpToolInfo->lpszText = NULL;  /* FIXME */

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOW))
    lpToolInfo->lParam = toolPtr->lParam;

  return TRUE;
}


static GXLRESULT
TOOLTIPS_HitTestA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTHITTESTINFOA lptthit = (GXLPTTHITTESTINFOA)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lptthit == 0)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromPoint (infoPtr, lptthit->hwnd, &lptthit->pt);
  if (nTool == -1)
    return FALSE;

  TRACE("tool %d!\n", nTool);

  /* copy tool data */
  if (lptthit->ti.cbSize >= sizeof(GXTTTOOLINFOA)) {
    toolPtr = &infoPtr->m_pTools[nTool];

    lptthit->ti.uFlags   = toolPtr->uFlags;
    lptthit->ti.hwnd     = toolPtr->hwnd;
    lptthit->ti.uId      = toolPtr->uId;
    lptthit->ti.rect     = toolPtr->rect;
    lptthit->ti.hinst    = toolPtr->hinst;
    /*  lptthit->ti.lpszText = toolPtr->lpszText; */
    lptthit->ti.lpszText = NULL;  /* FIXME */
    lptthit->ti.lParam   = toolPtr->lParam;
  }

  return TRUE;
}


static GXLRESULT
TOOLTIPS_HitTestW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTHITTESTINFOW lptthit = (GXLPTTHITTESTINFOW)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lptthit == 0)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromPoint (infoPtr, lptthit->hwnd, &lptthit->pt);
  if (nTool == -1)
    return FALSE;

  TRACE("tool %d!\n", nTool);

  /* copy tool data */
  if (lptthit->ti.cbSize >= sizeof(GXTTTOOLINFOW)) {
    toolPtr = &infoPtr->m_pTools[nTool];

    lptthit->ti.uFlags   = toolPtr->uFlags;
    lptthit->ti.hwnd     = toolPtr->hwnd;
    lptthit->ti.uId      = toolPtr->uId;
    lptthit->ti.rect     = toolPtr->rect;
    lptthit->ti.hinst    = toolPtr->hinst;
    /*  lptthit->ti.lpszText = toolPtr->lpszText; */
    lptthit->ti.lpszText = NULL;  /* FIXME */
    lptthit->ti.lParam   = toolPtr->lParam;
  }

  return TRUE;
}


static GXLRESULT
TOOLTIPS_NewToolRectA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpti = (GXLPTTTOOLINFOA)lParam;
  GXINT nTool;

  if (lpti == NULL)
    return 0;
  if (lpti->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpti);

  TRACE("nTool = %d, rect = %s\n", nTool, wine_dbgstr_rect(&lpti->rect));

  if (nTool == -1) return 0;

  infoPtr->m_pTools[nTool].rect = lpti->rect;

  return 0;
}


static GXLRESULT
TOOLTIPS_NewToolRectW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpti = (GXLPTTTOOLINFOW)lParam;
  GXINT nTool;

  if (lpti == NULL)
    return 0;
  if (lpti->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpti);

  TRACE("nTool = %d, rect = %s\n", nTool, wine_dbgstr_rect(&lpti->rect));

  if (nTool == -1) return 0;

  infoPtr->m_pTools[nTool].rect = lpti->rect;

  return 0;
}


static inline GXLRESULT
TOOLTIPS_Pop (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  TOOLTIPS_Hide (hwnd, infoPtr);

  return 0;
}


static GXLRESULT
TOOLTIPS_RelayEvent (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPMSG lpMsg = (GXLPMSG)lParam;
  GXPOINT pt;
  GXINT nOldTool;

  if (lParam == 0) {
    ERR("lpMsg == NULL!\n");
    return 0;
  }

  switch (lpMsg->message) {
  case GXWM_LBUTTONDOWN:
  case GXWM_LBUTTONUP:
  case GXWM_MBUTTONDOWN:
  case GXWM_MBUTTONUP:
  case GXWM_RBUTTONDOWN:
  case GXWM_RBUTTONUP:
    TOOLTIPS_Hide (hwnd, infoPtr);
    break;

  case GXWM_MOUSEMOVE:
    pt.x = (short)GXLOWORD(lpMsg->lParam);
    pt.y = (short)GXHIWORD(lpMsg->lParam);
    nOldTool = infoPtr->nTool;
    infoPtr->nTool = TOOLTIPS_GetToolFromPoint(infoPtr, (GXHWND)lpMsg->hwnd,
      &pt);
    TRACE("tool (%p) %d %d %d\n", hwnd, nOldTool,
      infoPtr->nTool, infoPtr->nCurrentTool);
    TRACE("WM_MOUSEMOVE (%p %d %d)\n", hwnd, pt.x, pt.y);

    if (infoPtr->nTool != nOldTool) {
      if(infoPtr->nTool == -1) { /* Moved out of all tools */
        TOOLTIPS_Hide(hwnd, infoPtr);
        gxKillTimer(hwnd, ID_TIMERLEAVE);
      } else if (nOldTool == -1) { /* Moved from outside */
        if(infoPtr->bActive) {
          gxSetTimer(hwnd, ID_TIMERSHOW, infoPtr->nInitialTime, 0);
          TRACE("timer 1 started!\n");
        }
      } else { /* Moved from one to another */
        TOOLTIPS_Hide (hwnd, infoPtr);
        gxKillTimer(hwnd, ID_TIMERLEAVE);
        if(infoPtr->bActive) {
          gxSetTimer (hwnd, ID_TIMERSHOW, infoPtr->nReshowTime, 0);
          TRACE("timer 1 started!\n");
        }
      }
    } else if(infoPtr->nCurrentTool != -1) { /* restart autopop */
      gxKillTimer(hwnd, ID_TIMERPOP);
      gxSetTimer(hwnd, ID_TIMERPOP, infoPtr->nAutoPopTime, 0);
      TRACE("timer 2 restarted\n");
    } else if(infoPtr->nTool != -1 && infoPtr->bActive) {
      /* previous show attempt didn't result in tooltip so try again */
      gxSetTimer(hwnd, ID_TIMERSHOW, infoPtr->nInitialTime, 0);
      TRACE("timer 1 started!\n");
    }
    break;
  }

  return 0;
}


static GXLRESULT
TOOLTIPS_SetDelayTime (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXINT nTime = (GXINT)GXLOWORD(lParam);

  switch (wParam) {
  case TTDT_AUTOMATIC:
    if (nTime <= 0)
      nTime = gxGetDoubleClickTime();
    infoPtr->nReshowTime    = nTime / 5;
    infoPtr->nAutoPopTime   = nTime * 10;
    infoPtr->nInitialTime   = nTime;
    break;

  case TTDT_RESHOW:
    if(nTime < 0)
      nTime = gxGetDoubleClickTime() / 5;
    infoPtr->nReshowTime = nTime;
    break;

  case TTDT_AUTOPOP:
    if(nTime < 0)
      nTime = gxGetDoubleClickTime() * 10;
    infoPtr->nAutoPopTime = nTime;
    break;

  case TTDT_INITIAL:
    if(nTime < 0)
      nTime = gxGetDoubleClickTime();
    infoPtr->nInitialTime = nTime;
    break;

  default:
    WARN("Invalid wParam %lx\n", wParam);
    break;
  }

  return 0;
}


static GXLRESULT
TOOLTIPS_SetMargin (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  LPGXRECT lpRect = (LPGXRECT)lParam;

  infoPtr->rcMargin.left   = lpRect->left;
  infoPtr->rcMargin.right  = lpRect->right;
  infoPtr->rcMargin.bottom = lpRect->bottom;
  infoPtr->rcMargin.top    = lpRect->top;

  return 0;
}


static inline GXLRESULT
TOOLTIPS_SetMaxTipWidth (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXINT nTemp = infoPtr->nMaxTipWidth;

  infoPtr->nMaxTipWidth = (GXINT)lParam;

  return nTemp;
}


static inline GXLRESULT
TOOLTIPS_SetTipBkColor (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  infoPtr->clrBk = (GXCOLORREF)wParam;

  return 0;
}


static inline GXLRESULT
TOOLTIPS_SetTipTextColor (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  infoPtr->clrText = (GXCOLORREF)wParam;

  return 0;
}


static GXLRESULT
TOOLTIPS_SetTitleA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPCSTR pszTitle = (GXLPCSTR)lParam;
  GXUINT_PTR uTitleIcon = (GXUINT_PTR)wParam;
  GXUINT size;

  TRACE("hwnd = %p, title = %s, icon = %p\n", hwnd, debugstr_a(pszTitle),
    (void*)uTitleIcon);

  Free(infoPtr->pszTitle);

  if (pszTitle)
  {
    size = sizeof(GXWCHAR)*gxMultiByteToWideChar(GXCP_ACP, 0, pszTitle, -1, NULL, 0);
    infoPtr->pszTitle = (GXLPWSTR)Alloc(size);
    if (!infoPtr->pszTitle)
      return FALSE;
    gxMultiByteToWideChar(GXCP_ACP, 0, pszTitle, -1, infoPtr->pszTitle, size/sizeof(GXWCHAR));
  }
  else
    infoPtr->pszTitle = NULL;

  if (uTitleIcon <= TTI_ERROR)
    infoPtr->hTitleIcon = hTooltipIcons[uTitleIcon];
  else
    infoPtr->hTitleIcon = gxCopyIcon((GXHICON)wParam);

  return TRUE;
}


static GXLRESULT
TOOLTIPS_SetTitleW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPCWSTR pszTitle = (GXLPCWSTR)lParam;
  GXUINT_PTR uTitleIcon = (GXUINT_PTR)wParam;
  GXUINT size;

  TRACE("hwnd = %p, title = %s, icon = %p\n", hwnd, debugstr_w(pszTitle),
    (void*)uTitleIcon);

  Free(infoPtr->pszTitle);

  if (pszTitle)
  {
    size = (GXSTRLEN(pszTitle)+1)*sizeof(GXWCHAR);
    infoPtr->pszTitle = (GXLPWSTR)Alloc(size);
    if (!infoPtr->pszTitle)
      return FALSE;
    memcpy(infoPtr->pszTitle, pszTitle, size);
  }
  else
    infoPtr->pszTitle = NULL;

  if (uTitleIcon <= TTI_ERROR)
    infoPtr->hTitleIcon = hTooltipIcons[uTitleIcon];
  else
    infoPtr->hTitleIcon = gxCopyIcon((GXHICON)wParam);

  TRACE("icon = %p\n", infoPtr->hTitleIcon);

  return TRUE;
}


static GXLRESULT
TOOLTIPS_SetToolInfoA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool data */
  toolPtr->uFlags = lpToolInfo->uFlags;
  toolPtr->hwnd   = lpToolInfo->hwnd;
  toolPtr->uId    = lpToolInfo->uId;
  toolPtr->rect   = lpToolInfo->rect;
  toolPtr->hinst  = lpToolInfo->hinst;

  if (GXIS_INTRESOURCE(lpToolInfo->lpszText)) {
    TRACE("set string id %x\n", GXLOWORD(lpToolInfo->lpszText));
    toolPtr->lpszText = (GXLPWSTR)lpToolInfo->lpszText;
  }
  else if (lpToolInfo->lpszText) {
    if (lpToolInfo->lpszText == GXLPSTR_TEXTCALLBACKA)
      toolPtr->lpszText = GXLPSTR_TEXTCALLBACKW;
    else {
      if ( (toolPtr->lpszText) &&
        !GXIS_INTRESOURCE(toolPtr->lpszText) ) {
          if( toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW)
            Free (toolPtr->lpszText);
          toolPtr->lpszText = NULL;
      }
      if (lpToolInfo->lpszText) {
        GXINT len = gxMultiByteToWideChar(GXCP_ACP, 0, lpToolInfo->lpszText,
          -1, NULL, 0);
        toolPtr->lpszText = (GXLPWSTR)Alloc (len * sizeof(GXWCHAR));
        gxMultiByteToWideChar(GXCP_ACP, 0, lpToolInfo->lpszText, -1,
          toolPtr->lpszText, len);
      }
    }
  }

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOA))
    toolPtr->lParam = lpToolInfo->lParam;

  return 0;
}


static GXLRESULT
TOOLTIPS_SetToolInfoW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return 0;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool data */
  toolPtr->uFlags = lpToolInfo->uFlags;
  toolPtr->hwnd   = lpToolInfo->hwnd;
  toolPtr->uId    = lpToolInfo->uId;
  toolPtr->rect   = lpToolInfo->rect;
  toolPtr->hinst  = lpToolInfo->hinst;

  if (GXIS_INTRESOURCE(lpToolInfo->lpszText)) {
    TRACE("set string id %x!\n", GXLOWORD(lpToolInfo->lpszText));
    toolPtr->lpszText = lpToolInfo->lpszText;
  }
  else {
    if (lpToolInfo->lpszText == GXLPSTR_TEXTCALLBACKW)
      toolPtr->lpszText = GXLPSTR_TEXTCALLBACKW;
    else {
      if ( (toolPtr->lpszText) &&
        !GXIS_INTRESOURCE(toolPtr->lpszText) ) {
          if( toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW)
            Free (toolPtr->lpszText);
          toolPtr->lpszText = NULL;
      }
      if (lpToolInfo->lpszText) {
        GXINT len = GXSTRLEN (lpToolInfo->lpszText);
        toolPtr->lpszText = (GXLPWSTR)Alloc ((len+1)*sizeof(GXWCHAR));
        GXSTRCPY (toolPtr->lpszText, lpToolInfo->lpszText);
      }
    }
  }

  if (lpToolInfo->cbSize >= sizeof(GXTTTOOLINFOW))
    toolPtr->lParam = lpToolInfo->lParam;

  if (infoPtr->nCurrentTool == nTool)
  {
    TOOLTIPS_GetTipText (hwnd, infoPtr, infoPtr->nCurrentTool);

    if (infoPtr->szTipText[0] == 0)
      TOOLTIPS_Hide(hwnd, infoPtr);
    else
      TOOLTIPS_Show (hwnd, infoPtr, FALSE);
  }

  return 0;
}


static GXLRESULT
TOOLTIPS_TrackActivate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  if ((GXBOOL)wParam) {
    GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;

    if (lpToolInfo == NULL)
      return 0;
    if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
      return FALSE;

    /* activate */
    infoPtr->nTrackTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);
    if (infoPtr->nTrackTool != -1) {
      TRACE("activated!\n");
      infoPtr->bTrackActive = TRUE;
      TOOLTIPS_TrackShow (hwnd, infoPtr);
    }
  }
  else {
    /* deactivate */
    TOOLTIPS_TrackHide (hwnd, infoPtr);

    infoPtr->bTrackActive = FALSE;
    infoPtr->nTrackTool = -1;

    TRACE("deactivated!\n");
  }

  return 0;
}


static GXLRESULT
TOOLTIPS_TrackPosition (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  infoPtr->xTrackPos = (GXINT)GXLOWORD(lParam);
  infoPtr->yTrackPos = (GXINT)GXHIWORD(lParam);

  if (infoPtr->bTrackActive) {
    TRACE("[%d %d]\n",
      infoPtr->xTrackPos, infoPtr->yTrackPos);

    TOOLTIPS_TrackShow (hwnd, infoPtr);
  }

  return 0;
}


static GXLRESULT
TOOLTIPS_Update (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  if (infoPtr->nCurrentTool != -1)
    gxUpdateWindow (hwnd);

  return 0;
}


static GXLRESULT
TOOLTIPS_UpdateTipTextA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOA lpToolInfo = (GXLPTTTOOLINFOA)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOA_V1_SIZE)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoA (infoPtr, lpToolInfo);
  if (nTool == -1) return 0;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool text */
  toolPtr->hinst  = lpToolInfo->hinst;

  if (GXIS_INTRESOURCE(lpToolInfo->lpszText)){
    toolPtr->lpszText = (GXLPWSTR)lpToolInfo->lpszText;
  }
  else if (lpToolInfo->lpszText) {
    if (lpToolInfo->lpszText == GXLPSTR_TEXTCALLBACKA)
      toolPtr->lpszText = GXLPSTR_TEXTCALLBACKW;
    else {
      if ( (toolPtr->lpszText) &&
        !GXIS_INTRESOURCE(toolPtr->lpszText) ) {
          if( toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW)
            Free (toolPtr->lpszText);
          toolPtr->lpszText = NULL;
      }
      if (lpToolInfo->lpszText) {
        GXINT len = gxMultiByteToWideChar(GXCP_ACP, 0, lpToolInfo->lpszText,
          -1, NULL, 0);
        toolPtr->lpszText = (GXLPWSTR)Alloc (len * sizeof(GXWCHAR));
        gxMultiByteToWideChar(GXCP_ACP, 0, lpToolInfo->lpszText, -1,
          toolPtr->lpszText, len);
      }
    }
  }

  if(infoPtr->nCurrentTool == -1) return 0;
  /* force repaint */
  if (infoPtr->bActive)
    TOOLTIPS_Show (hwnd, infoPtr, FALSE);
  else if (infoPtr->bTrackActive)
    TOOLTIPS_TrackShow (hwnd, infoPtr);

  return 0;
}


static GXLRESULT
TOOLTIPS_UpdateTipTextW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLPTTTOOLINFOW lpToolInfo = (GXLPTTTOOLINFOW)lParam;
  TTTOOL_INFO *toolPtr;
  GXINT nTool;

  if (lpToolInfo == NULL)
    return 0;
  if (lpToolInfo->cbSize < GXTTTOOLINFOW_V1_SIZE)
    return FALSE;

  nTool = TOOLTIPS_GetToolFromInfoW (infoPtr, lpToolInfo);
  if (nTool == -1)
    return 0;

  TRACE("tool %d\n", nTool);

  toolPtr = &infoPtr->m_pTools[nTool];

  /* copy tool text */
  toolPtr->hinst  = lpToolInfo->hinst;

  if (GXIS_INTRESOURCE(lpToolInfo->lpszText)){
    toolPtr->lpszText = lpToolInfo->lpszText;
  }
  else if (lpToolInfo->lpszText) {
    if (lpToolInfo->lpszText == GXLPSTR_TEXTCALLBACKW)
      toolPtr->lpszText = GXLPSTR_TEXTCALLBACKW;
    else {
      if ( (toolPtr->lpszText)  &&
        !GXIS_INTRESOURCE(toolPtr->lpszText) ) {
          if( toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW)
            Free (toolPtr->lpszText);
          toolPtr->lpszText = NULL;
      }
      if (lpToolInfo->lpszText) {
        GXINT len = GXSTRLEN (lpToolInfo->lpszText);
        toolPtr->lpszText = (GXLPWSTR)Alloc ((len+1)*sizeof(GXWCHAR));
        GXSTRCPY (toolPtr->lpszText, lpToolInfo->lpszText);
      }
    }
  }

  if(infoPtr->nCurrentTool == -1) return 0;
  /* force repaint */
  if (infoPtr->bActive)
    TOOLTIPS_Show (hwnd, infoPtr, FALSE);
  else if (infoPtr->bTrackActive)
    TOOLTIPS_Show (hwnd, infoPtr, TRUE);

  return 0;
}


static GXLRESULT
TOOLTIPS_WindowFromPoint (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  return (GXLRESULT)gxWindowFromPoint (((LPGXPOINT)lParam));
}



static GXLRESULT
TOOLTIPS_Create (GXHWND hwnd, const GXCREATESTRUCTW *lpcs)
{
  TOOLTIPS_INFO *infoPtr;

  /* allocate memory for info structure */
  infoPtr = (TOOLTIPS_INFO *)Alloc (sizeof(TOOLTIPS_INFO));
  gxSetWindowLongPtrW (hwnd, 0, (GXDWORD_PTR)infoPtr);

  /* initialize info structure */
  infoPtr->bActive = TRUE;
  infoPtr->bTrackActive = FALSE;

  infoPtr->nMaxTipWidth = -1;
  infoPtr->nTool = -1;
  infoPtr->nCurrentTool = -1;
  infoPtr->nTrackTool = -1;

  /* initialize colours and fonts */
  TOOLTIPS_InitSystemSettings(infoPtr);

  TOOLTIPS_SetDelayTime(hwnd, TTDT_AUTOMATIC, 0L);

  gxSetWindowPos (hwnd, GXHWND_TOP, 0, 0, 0, 0, GXSWP_NOZORDER | GXSWP_HIDEWINDOW | GXSWP_NOACTIVATE);

  return 0;
}


GXLRESULT TOOLTIPS_INFO::Destroy(GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TTTOOL_INFO *toolPtr;
  GXUINT i;

  /* free tools */
  if (m_pTools) {
    for (i = 0; i < m_uNumTools; i++) {
      toolPtr = &m_pTools[i];
      if (toolPtr->lpszText) {
        if ( (toolPtr->lpszText != GXLPSTR_TEXTCALLBACKW) &&
          !GXIS_INTRESOURCE(toolPtr->lpszText) )
        {
          Free (toolPtr->lpszText);
          toolPtr->lpszText = NULL;
        }
      }

      /* remove subclassing */
      if (toolPtr->uFlags & TTF_SUBCLASS) {
        gxRemoveWindowSubclass((toolPtr->uFlags & TTF_IDISHWND) 
          ? (GXHWND)toolPtr->uId 
          : toolPtr->hwnd, TOOLTIPS_SubclassProc, 1);
      }
    }
    Free (m_pTools);
  }

  /* free title string */
  Free (pszTitle);
  /* free title icon if not a standard one */
  if (TOOLTIPS_GetTitleIconIndex(hTitleIcon) > TTI_ERROR)
    gxDeleteObject(hTitleIcon);

  /* delete fonts */
  gxDeleteObject (hFont);
  gxDeleteObject (hTitleFont);

  /* free tool tips info data */
  Free (this);
  gxSetWindowLongPtrW(hWnd, 0, 0);
  return 0;
}


static GXLRESULT
TOOLTIPS_GetFont (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  return (GXLRESULT)infoPtr->hFont;
}


static GXLRESULT
TOOLTIPS_MouseMessage (GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  TOOLTIPS_Hide (hwnd, infoPtr);

  return 0;
}


static GXLRESULT
TOOLTIPS_NCCreate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXDWORD dwStyle = gxGetWindowLongW (hwnd, GXGWL_STYLE);
  GXDWORD dwExStyle = gxGetWindowLongW (hwnd, GXGWL_EXSTYLE);

  dwStyle &= ~(GXWS_CHILD | /*WS_MAXIMIZE |*/ GXWS_BORDER | GXWS_DLGFRAME);
  dwStyle |= (GXWS_POPUP | GXWS_BORDER | GXWS_CLIPSIBLINGS);

  /* WS_BORDER only draws a border round the window rect, not the
  * window region, therefore it is useless to us in balloon mode */
  if (dwStyle & TTS_BALLOON) dwStyle &= ~GXWS_BORDER;

  gxSetWindowLongW (hwnd, GXGWL_STYLE, dwStyle);

  dwExStyle |= GXWS_EX_TOOLWINDOW;
  gxSetWindowLongW (hwnd, GXGWL_EXSTYLE, dwExStyle);

  return TRUE;
}


static GXLRESULT
TOOLTIPS_NCHitTest (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXINT nTool = (infoPtr->bTrackActive) ? infoPtr->nTrackTool : infoPtr->nTool;

  TRACE(" nTool=%d\n", nTool);

  if ((nTool > -1) && (nTool < infoPtr->m_uNumTools)) {
    if (infoPtr->m_pTools[nTool].uFlags & TTF_TRANSPARENT) {
      TRACE("-- in transparent mode!\n");
      return GXHTTRANSPARENT;
    }
  }

  return gxDefWindowProcW (hwnd, GXWM_NCHITTEST, wParam, lParam);
}


static GXLRESULT
TOOLTIPS_NotifyFormat (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  FIXME ("hwnd=%p wParam=%lx lParam=%lx\n", hwnd, wParam, lParam);

  return 0;
}


static GXLRESULT
TOOLTIPS_Paint (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXHDC hdc;
  GXPAINTSTRUCT ps;

  hdc = (wParam == 0) ? gxBeginPaint (hwnd, &ps) : (GXHDC)wParam;
  TOOLTIPS_Refresh (hwnd, hdc);
  if (!wParam)
    gxEndPaint (hwnd, &ps);
  return 0;
}


static GXLRESULT
TOOLTIPS_SetFont (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLOGFONTW lf;

  if(!gxGetObjectW((GXHFONT)wParam, sizeof(lf), &lf))
    return 0;

  gxDeleteObject (infoPtr->hFont);
  infoPtr->hFont = gxCreateFontIndirectW((const GXLOGFONTW*)&lf);

  gxDeleteObject (infoPtr->hTitleFont);
  lf.lfWeight = GXFW_BOLD;
  infoPtr->hTitleFont = gxCreateFontIndirectW((const GXLOGFONTW*)&lf);

  if ((GXLOWORD(lParam)) & (infoPtr->nCurrentTool != -1)) {
    FIXME("full redraw needed!\n");
  }

  return 0;
}

/******************************************************************
* TOOLTIPS_GetTextLength
*
* This function is called when the tooltip receive a
* WM_GETTEXTLENGTH message.
* wParam : not used
* lParam : not used
*
* returns the length, in characters, of the tip text
*/
static GXLRESULT
TOOLTIPS_GetTextLength(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  return GXSTRLEN(infoPtr->szTipText);
}

/******************************************************************
* TOOLTIPS_OnWMGetText
*
* This function is called when the tooltip receive a
* WM_GETTEXT message.
* wParam : specifies the maximum number of characters to be copied
* lParam : is the pointer to the buffer that will receive
*          the tip text
*
* returns the number of characters copied
*/
static GXLRESULT
TOOLTIPS_OnWMGetText (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXLRESULT res;
  GXLPWSTR pszText = (GXLPWSTR)lParam;

  if(!infoPtr->szTipText || !wParam)
    return 0;

  res = min(GXSTRLEN(infoPtr->szTipText)+1, wParam);
  memcpy(pszText, infoPtr->szTipText, res*sizeof(GXWCHAR));
  pszText[res-1] = '\0';
  return res-1;
}

static GXLRESULT
TOOLTIPS_Timer (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);
  GXINT nOldTool;

  TRACE("timer %ld (%p) expired!\n", wParam, hwnd);

  switch (wParam) {
  case ID_TIMERSHOW:
    gxKillTimer (hwnd, ID_TIMERSHOW);
    nOldTool = infoPtr->nTool;
    if ((infoPtr->nTool = TOOLTIPS_CheckTool (hwnd, TRUE)) == nOldTool)
      TOOLTIPS_Show (hwnd, infoPtr, FALSE);
    break;

  case ID_TIMERPOP:
    TOOLTIPS_Hide (hwnd, infoPtr);
    break;

  case ID_TIMERLEAVE:
    nOldTool = infoPtr->nTool;
    infoPtr->nTool = TOOLTIPS_CheckTool (hwnd, FALSE);
    TRACE("tool (%p) %d %d %d\n", hwnd, nOldTool,
      infoPtr->nTool, infoPtr->nCurrentTool);
    if (infoPtr->nTool != nOldTool) {
      if(infoPtr->nTool == -1) { /* Moved out of all tools */
        TOOLTIPS_Hide(hwnd, infoPtr);
        gxKillTimer(hwnd, ID_TIMERLEAVE);
      } else if (nOldTool == -1) { /* Moved from outside */
        ERR("How did this happen?\n");
      } else { /* Moved from one to another */
        TOOLTIPS_Hide (hwnd, infoPtr);
        gxKillTimer(hwnd, ID_TIMERLEAVE);
        if(infoPtr->bActive) {
          gxSetTimer (hwnd, ID_TIMERSHOW, infoPtr->nReshowTime, 0);
          TRACE("timer 1 started!\n");
        }
      }
    }
    break;

  default:
    ERR("Unknown timer id %ld\n", wParam);
    break;
  }
  return 0;
}


static GXLRESULT
TOOLTIPS_WinIniChange (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr (hwnd);

  TOOLTIPS_InitSystemSettings (infoPtr);

  return 0;
}


static GXLRESULT GXCALLBACK
TOOLTIPS_SubclassProc (GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam, GXUINT_PTR uID, GXDWORD_PTR dwRef)
{
  GXMSG msg;

  switch(uMsg) {
  case GXWM_MOUSEMOVE:
  case GXWM_LBUTTONDOWN:
  case GXWM_LBUTTONUP:
  case GXWM_MBUTTONDOWN:
  case GXWM_MBUTTONUP:
  case GXWM_RBUTTONDOWN:
  case GXWM_RBUTTONUP:
    msg.hwnd = hwnd;
    msg.message = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;
    TOOLTIPS_RelayEvent((GXHWND)dwRef, 0, (GXLPARAM)&msg);
    break;

  default:
    break;
  }
  return gxDefSubclassProc(hwnd, uMsg, wParam, lParam);
}


static GXLRESULT GXCALLBACK
TOOLTIPS_WindowProc (GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("hwnd=%p msg=%x wparam=%lx lParam=%lx\n", hwnd, uMsg, wParam, lParam);
  TOOLTIPS_INFO *infoPtr = TOOLTIPS_GetInfoPtr(hwnd);

  if ( ! infoPtr && (uMsg != GXWM_CREATE) && (uMsg != GXWM_NCCREATE)) {
    return gxDefWindowProcW (hwnd, uMsg, wParam, lParam);
  }

  switch (uMsg)
  {
  case GXTTM_ACTIVATE:
    return TOOLTIPS_Activate (hwnd, wParam, lParam);

  case GXTTM_ADDTOOLA:
    return infoPtr->AddToolA(hwnd, wParam, lParam);

  case GXTTM_ADDTOOLW:
    return infoPtr->AddToolW(hwnd, wParam, lParam);

  case GXTTM_DELTOOLA:
    return TOOLTIPS_DelToolA (hwnd, wParam, lParam);

  case GXTTM_DELTOOLW:
    return TOOLTIPS_DelToolW (hwnd, wParam, lParam);

  case GXTTM_ENUMTOOLSA:
    return TOOLTIPS_EnumToolsA (hwnd, wParam, lParam);

  case GXTTM_ENUMTOOLSW:
    return TOOLTIPS_EnumToolsW (hwnd, wParam, lParam);

  case GXTTM_GETBUBBLESIZE:
    return TOOLTIPS_GetBubbleSize (hwnd, wParam, lParam);

  case GXTTM_GETCURRENTTOOLA:
    return TOOLTIPS_GetCurrentToolA (hwnd, wParam, lParam);

  case GXTTM_GETCURRENTTOOLW:
    return TOOLTIPS_GetCurrentToolW (hwnd, wParam, lParam);

  case GXTTM_GETDELAYTIME:
    return TOOLTIPS_GetDelayTime (hwnd, wParam, lParam);

  case GXTTM_GETMARGIN:
    return TOOLTIPS_GetMargin (hwnd, wParam, lParam);

  case GXTTM_GETMAXTIPWIDTH:
    return TOOLTIPS_GetMaxTipWidth (hwnd, wParam, lParam);

  case GXTTM_GETTEXTA:
    return TOOLTIPS_GetTextA (hwnd, wParam, lParam);

  case GXTTM_GETTEXTW:
    return TOOLTIPS_GetTextW (hwnd, wParam, lParam);

  case GXTTM_GETTIPBKCOLOR:
    return TOOLTIPS_GetTipBkColor (hwnd, wParam, lParam);

  case GXTTM_GETTIPTEXTCOLOR:
    return TOOLTIPS_GetTipTextColor (hwnd, wParam, lParam);

  case GXTTM_GETTOOLCOUNT:
    return TOOLTIPS_GetToolCount (hwnd, wParam, lParam);

  case GXTTM_GETTOOLINFOA:
    return TOOLTIPS_GetToolInfoA (hwnd, wParam, lParam);

  case GXTTM_GETTOOLINFOW:
    return TOOLTIPS_GetToolInfoW (hwnd, wParam, lParam);

  case GXTTM_HITTESTA:
    return TOOLTIPS_HitTestA (hwnd, wParam, lParam);

  case GXTTM_HITTESTW:
    return TOOLTIPS_HitTestW (hwnd, wParam, lParam);

  case GXTTM_NEWTOOLRECTA:
    return TOOLTIPS_NewToolRectA (hwnd, wParam, lParam);

  case GXTTM_NEWTOOLRECTW:
    return TOOLTIPS_NewToolRectW (hwnd, wParam, lParam);

  case GXTTM_POP:
    return TOOLTIPS_Pop (hwnd, wParam, lParam);

  case GXTTM_RELAYEVENT:
    return TOOLTIPS_RelayEvent (hwnd, wParam, lParam);

  case GXTTM_SETDELAYTIME:
    return TOOLTIPS_SetDelayTime (hwnd, wParam, lParam);

  case GXTTM_SETMARGIN:
    return TOOLTIPS_SetMargin (hwnd, wParam, lParam);

  case GXTTM_SETMAXTIPWIDTH:
    return TOOLTIPS_SetMaxTipWidth (hwnd, wParam, lParam);

  case GXTTM_SETTIPBKCOLOR:
    return TOOLTIPS_SetTipBkColor (hwnd, wParam, lParam);

  case GXTTM_SETTIPTEXTCOLOR:
    return TOOLTIPS_SetTipTextColor (hwnd, wParam, lParam);

  case GXTTM_SETTITLEA:
    return TOOLTIPS_SetTitleA (hwnd, wParam, lParam);

  case GXTTM_SETTITLEW:
    return TOOLTIPS_SetTitleW (hwnd, wParam, lParam);

  case GXTTM_SETTOOLINFOA:
    return TOOLTIPS_SetToolInfoA (hwnd, wParam, lParam);

  case GXTTM_SETTOOLINFOW:
    return TOOLTIPS_SetToolInfoW (hwnd, wParam, lParam);

  case GXTTM_TRACKACTIVATE:
    return TOOLTIPS_TrackActivate (hwnd, wParam, lParam);

  case GXTTM_TRACKPOSITION:
    return TOOLTIPS_TrackPosition (hwnd, wParam, lParam);

  case GXTTM_UPDATE:
    return TOOLTIPS_Update (hwnd, wParam, lParam);

  case GXTTM_UPDATETIPTEXTA:
    return TOOLTIPS_UpdateTipTextA (hwnd, wParam, lParam);

  case GXTTM_UPDATETIPTEXTW:
    return TOOLTIPS_UpdateTipTextW (hwnd, wParam, lParam);

  case GXTTM_WINDOWFROMPOINT:
    return TOOLTIPS_WindowFromPoint (hwnd, wParam, lParam);


  case GXWM_CREATE:
    return TOOLTIPS_Create (hwnd, (GXLPCREATESTRUCTW)lParam);

  case GXWM_DESTROY:
    return infoPtr->Destroy(hwnd, wParam, lParam);

  case GXWM_ERASEBKGND:
    /* we draw the background in WM_PAINT */
    return 0;

  case GXWM_GETFONT:
    return TOOLTIPS_GetFont (hwnd, wParam, lParam);

  case GXWM_GETTEXT:
    return TOOLTIPS_OnWMGetText (hwnd, wParam, lParam);

  case GXWM_GETTEXTLENGTH:
    return TOOLTIPS_GetTextLength (hwnd, wParam, lParam);

  case GXWM_LBUTTONDOWN:
  case GXWM_LBUTTONUP:
  case GXWM_MBUTTONDOWN:
  case GXWM_MBUTTONUP:
  case GXWM_RBUTTONDOWN:
  case GXWM_RBUTTONUP:
  case GXWM_MOUSEMOVE:
    return TOOLTIPS_MouseMessage (hwnd, uMsg, wParam, lParam);

  case GXWM_NCCREATE:
    return TOOLTIPS_NCCreate (hwnd, wParam, lParam);

  case GXWM_NCHITTEST:
    return TOOLTIPS_NCHitTest (hwnd, wParam, lParam);

  case GXWM_NOTIFYFORMAT:
    return TOOLTIPS_NotifyFormat (hwnd, wParam, lParam);

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    return TOOLTIPS_Paint (hwnd, wParam, lParam);

  case GXWM_SETFONT:
    return TOOLTIPS_SetFont (hwnd, wParam, lParam);

  case GXWM_TIMER:
    return TOOLTIPS_Timer (hwnd, wParam, lParam);

  case GXWM_WININICHANGE:
    return TOOLTIPS_WinIniChange (hwnd, wParam, lParam);

  default:
    if ((uMsg >= GXWM_USER) && (uMsg < GXWM_APP))
      ERR("unknown msg %04x wp=%08lx lp=%08lx\n",
      uMsg, wParam, lParam);
    return gxDefWindowProcW (hwnd, uMsg, wParam, lParam);
  }
}


GXVOID
TOOLTIPS_Register ()
{
  GXWNDCLASSEX wndClass;

  ZeroMemory (&wndClass, sizeof(GXWNDCLASSEX));
  wndClass.style         = GXCS_GLOBALCLASS | GXCS_DBLCLKS | GXCS_SAVEBITS;
  wndClass.lpfnWndProc   = TOOLTIPS_WindowProc;
  wndClass.cbClsExtra    = 0;
  wndClass.cbWndExtra    = sizeof(TOOLTIPS_INFO *);
  wndClass.hCursor       = gxLoadCursorW (0, (GXLPWSTR)GXIDC_ARROW);
  wndClass.hbrBackground = 0;
  wndClass.lpszClassName = TOOLTIPS_CLASSW;

  gxRegisterClassExW (&wndClass);

  hTooltipIcons[TTI_NONE] = NULL;
  hTooltipIcons[TTI_INFO] = (GXHICON)gxLoadImageW(COMCTL32_hModule,
    (GXLPCWSTR)GXMAKEINTRESOURCE(IDI_TT_INFO_SM), GXIMAGE_ICON, 0, 0, 0);
  hTooltipIcons[TTI_WARNING] = (GXHICON)gxLoadImageW(COMCTL32_hModule,
    (GXLPCWSTR)GXMAKEINTRESOURCE(IDI_TT_WARN_SM), GXIMAGE_ICON, 0, 0, 0);
  hTooltipIcons[TTI_ERROR] = (GXHICON)gxLoadImageW(COMCTL32_hModule,
    (GXLPCWSTR)GXMAKEINTRESOURCE(IDI_TT_ERROR_SM), GXIMAGE_ICON, 0, 0, 0);
}


GXVOID
TOOLTIPS_Unregister ()
{
  int i;
  for (i = TTI_INFO; i <= TTI_ERROR; i++)
    gxDestroyIcon(hTooltipIcons[i]);
  gxUnregisterClassW (TOOLTIPS_CLASSW, NULL);
}
#endif // TOOLTIP
#endif // _DEV_DISABLE_UI_CODE