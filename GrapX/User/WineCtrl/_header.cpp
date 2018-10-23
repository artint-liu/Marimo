#ifndef _DEV_DISABLE_UI_CODE
/*
*  Header control
*
*  Copyright 1998 Eric Kohl
*  Copyright 2000 Eric Kohl for CodeWeavers
*  Copyright 2003 Maxime Bellenge
*  Copyright 2006 Mikolaj Zalewski
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
*  TODO:
*   - Imagelist support (completed?)
*   - Hottrack support (completed?)
*   - Filters support (HDS_FILTER, HDI_FILTER, HDM_*FILTER*, HDN_*FILTER*)
*   - New Windows Vista features
*/

//#include <stdarg.h>
//#include <stdlib.h>
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
//#include "imagelist.h"
//#include "tmschema.h"
//#include "uxtheme.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(header);
#include <GrapX.h>
#include "GrapX/gUxtheme.h"
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXImm.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.h>
#include <User/Win32Emu/GXCommCtrl.h>

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

typedef struct __tagHEADER_ITEM
{
  GXINT     cxy;
  GXHBITMAP hbm;
  GXLPWSTR    pszText;
  GXINT     fmt;
  GXLPARAM    lParam;
  GXINT     iImage;
  GXINT     iOrder;    /* see documentation of HD_ITEM */

  GXBOOL    bDown;    /* is item pressed? (used for drawing) */
  GXRECT    rect;    /* bounding rectangle of the item */
  GXDWORD   callbackMask;       /* HDI_* flags for items that are callback */
} HEADER_ITEM;


typedef struct __tagHEADER_INFO
{
  GXHWND      hwndNotify;  /* Owner window to send notifications to */
  GXINT       nNotifyFormat;  /* format used for WM_NOTIFY messages */
  GXUINT      uNumItem;    /* number of items (columns) */
  GXINT       nHeight;    /* height of the header (pixels) */
  GXHFONT     hFont;    /* handle to the current font */
  GXHCURSOR   hcurArrow;  /* handle to the arrow cursor */
  GXHCURSOR   hcurDivider;  /* handle to a cursor (used over dividers) <-|-> */
  GXHCURSOR   hcurDivopen;  /* handle to a cursor (used over dividers) <-||-> */
  GXBOOL      bCaptured;  /* Is the mouse captured? */
  GXBOOL      bPressed;    /* Is a header item pressed (down)? */
  GXBOOL      bDragging;        /* Are we dragging an item? */
  GXBOOL      bTracking;  /* Is in tracking mode? */
  GXPOINT     ptLButtonDown;    /* The point where the left button was pressed */
  GXINT       iMoveItem;  /* index of tracked item. (Tracking mode) */
  GXINT       xTrackOffset;  /* distance between the right side of the tracked item and the cursor */
  GXINT       xOldTrack;  /* track offset (see above) after the last WM_MOUSEMOVE */
  GXINT       iHotItem;    /* index of hot item (cursor is over this item) */
  GXINT       iHotDivider;      /* index of the hot divider (used while dragging an item or by HDM_SETHOTDIVIDER) */
  GXINT       iMargin;          /* width of the margin that surrounds a bitmap */

  GXHIMAGELIST  himl;    /* handle to an image list (may be 0) */
  HEADER_ITEM *items;    /* pointer to array of HEADER_ITEM's */
  GXINT         *order;         /* array of item IDs indexed by order */
  GXBOOL  bRectsValid;  /* validity flag for bounding rectangles */
} HEADER_INFO;


#define VERT_BORDER     4
#define DIVIDER_WIDTH  10
#define HOT_DIVIDER_WIDTH 2
#define MAX_HEADER_TEXT_LEN 260
#define HDN_UNICODE_OFFSET 20
#define HDN_FIRST_UNICODE (HDN_FIRST-HDN_UNICODE_OFFSET)

#define HDI_SUPPORTED_FIELDS (HDI_WIDTH|HDI_TEXT|HDI_FORMAT|HDI_LPARAM|HDI_BITMAP|HDI_IMAGE|HDI_ORDER)
#define HDI_UNSUPPORTED_FIELDS (HDI_FILTER)
#define HDI_UNKNOWN_FIELDS (~(HDI_SUPPORTED_FIELDS|HDI_UNSUPPORTED_FIELDS|HDI_DI_SETITEM))
#define HDI_COMCTL32_4_0_FIELDS (HDI_WIDTH|HDI_TEXT|HDI_FORMAT|HDI_LPARAM|HDI_BITMAP)

#define HEADER_GetInfoPtr(hwnd) ((HEADER_INFO *)gxGetWindowLongPtrW(hwnd,0))

static GXBOOL HEADER_PrepareCallbackItems(GXHWND hwnd, GXINT iItem, GXINT reqMask);
static void HEADER_FreeCallbackItems(HEADER_ITEM *lpItem);
static GXLRESULT HEADER_SendNotify(GXHWND hwnd, GXUINT code, GXNMHDR *hdr);
static GXLRESULT HEADER_SendCtrlCustomDraw(GXHWND hwnd, GXDWORD dwDrawStage, GXHDC hdc, const GXRECT *rect);

static const GXWCHAR themeClass[] = {'H','e','a','d','e','r',0};

static void HEADER_StoreHDItemInHeader(HEADER_ITEM *lpItem, GXUINT mask, const GXHDITEMW *phdi, GXBOOL fUnicode)
{
  if (mask & HDI_UNSUPPORTED_FIELDS)
    FIXME("unsupported header fields %x\n", (mask & HDI_UNSUPPORTED_FIELDS));

  if (mask & HDI_BITMAP)
    lpItem->hbm = phdi->hbm;

  if (mask & HDI_FORMAT)
    lpItem->fmt = phdi->fmt;

  if (mask & HDI_LPARAM)
    lpItem->lParam = phdi->lParam;

  if (mask & HDI_WIDTH)
    lpItem->cxy = phdi->cxy;

  if (mask & HDI_IMAGE) 
  {
    lpItem->iImage = phdi->iImage;
    if (phdi->iImage == GXI_IMAGECALLBACK)
      lpItem->callbackMask |= HDI_IMAGE;
    else
      lpItem->callbackMask &= ~HDI_IMAGE;
  }

  if (mask & HDI_TEXT)
  {
    Free(lpItem->pszText);
    lpItem->pszText = NULL;

    if (phdi->pszText != GXLPSTR_TEXTCALLBACKW) /* covers != TEXTCALLBACKA too */
    {
      static const GXWCHAR emptyString[] = {0};

      GXLPCWSTR pszText = (phdi->pszText != NULL ? phdi->pszText : emptyString);
      if (fUnicode)
        gxStr_SetPtrW(&lpItem->pszText, pszText);
      else
        gxStr_SetPtrAtoW(&lpItem->pszText, (GXLPCSTR)pszText);
      lpItem->callbackMask &= ~HDI_TEXT;
    }
    else
    {
      lpItem->pszText = NULL;
      lpItem->callbackMask |= HDI_TEXT;
    }  
  }
}

static inline GXLRESULT
HEADER_IndexToOrder (GXHWND hwnd, GXINT iItem)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem = &infoPtr->items[iItem];
  return lpItem->iOrder;
}


static GXINT
HEADER_OrderToIndex(GXHWND hwnd, GXWPARAM wParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXINT iorder = (GXINT)wParam;

  if ((iorder <0) || iorder >= infoPtr->uNumItem)
    return iorder;
  return infoPtr->order[iorder];
}

static void
HEADER_ChangeItemOrder(const HEADER_INFO *infoPtr, GXINT iItem, GXINT iNewOrder)
{
  HEADER_ITEM *lpItem = &infoPtr->items[iItem];
  GXINT i, nMin, nMax;

  TRACE("%d: %d->%d\n", iItem, lpItem->iOrder, iNewOrder);
  if (lpItem->iOrder < iNewOrder)
  {
    memmove(&infoPtr->order[lpItem->iOrder],
      &infoPtr->order[lpItem->iOrder + 1],
      (iNewOrder - lpItem->iOrder) * sizeof(GXINT));
  }
  if (iNewOrder < lpItem->iOrder)
  {
    memmove(&infoPtr->order[iNewOrder + 1],
      &infoPtr->order[iNewOrder],
      (lpItem->iOrder - iNewOrder) * sizeof(GXINT));
  }
  infoPtr->order[iNewOrder] = iItem;
  nMin = min(lpItem->iOrder, iNewOrder);
  nMax = max(lpItem->iOrder, iNewOrder);
  for (i = nMin; i <= nMax; i++)
    infoPtr->items[infoPtr->order[i]].iOrder = i;
}

/* Note: if iItem is the last item then this function returns infoPtr->uNumItem */
static GXINT
HEADER_NextItem(GXHWND hwnd, GXINT iItem)
{
  return HEADER_OrderToIndex(hwnd, HEADER_IndexToOrder(hwnd, iItem)+1);
}

static GXINT
HEADER_PrevItem(GXHWND hwnd, GXINT iItem)
{
  return HEADER_OrderToIndex(hwnd, HEADER_IndexToOrder(hwnd, iItem)-1);
}

static void
HEADER_SetItemBounds (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *phdi;
  GXRECT rect;
  unsigned int i;
  int x;

  infoPtr->bRectsValid = TRUE;

  if (infoPtr->uNumItem == 0)
    return;

  gxGetClientRect (hwnd, &rect);

  x = rect.left;
  for (i = 0; i < infoPtr->uNumItem; i++) {
    phdi = &infoPtr->items[HEADER_OrderToIndex(hwnd,i)];
    phdi->rect.top = rect.top;
    phdi->rect.bottom = rect.bottom;
    phdi->rect.left = x;
    phdi->rect.right = phdi->rect.left + ((phdi->cxy>0)?phdi->cxy:0);
    x = phdi->rect.right;
  }
}

static GXLRESULT
HEADER_Size (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  infoPtr->bRectsValid = FALSE;

  return 0;
}

static void HEADER_GetHotDividerRect(GXHWND hwnd, const HEADER_INFO *infoPtr, GXRECT *r)
{
  GXINT iDivider = infoPtr->iHotDivider;
  if (infoPtr->uNumItem > 0)
  {
    HEADER_ITEM *lpItem;

    if (iDivider < infoPtr->uNumItem)
    {
      lpItem = &infoPtr->items[iDivider];
      r->left  = lpItem->rect.left - HOT_DIVIDER_WIDTH/2;
      r->right = lpItem->rect.left + HOT_DIVIDER_WIDTH/2;
    }
    else
    {
      lpItem = &infoPtr->items[HEADER_OrderToIndex(hwnd, infoPtr->uNumItem-1)];
      r->left  = lpItem->rect.right - HOT_DIVIDER_WIDTH/2;
      r->right = lpItem->rect.right + HOT_DIVIDER_WIDTH/2;
    }
    r->top    = lpItem->rect.top;
    r->bottom = lpItem->rect.bottom;
  }
  else
  {
    GXRECT clientRect;
    gxGetClientRect(hwnd, &clientRect);
    *r = clientRect;
    r->right = r->left + HOT_DIVIDER_WIDTH/2;
  }
}


static GXINT
HEADER_DrawItem (GXHWND hwnd, GXHDC hdc, GXINT iItem, GXBOOL bHotTrack, GXLRESULT lCDFlags)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *phdi = &infoPtr->items[iItem];
  GXRECT r;
  GXINT  oldBkMode;
  GXHTHEME theme = gxGetWindowTheme (hwnd);
  GXNMCUSTOMDRAW nmcd;

  TRACE("DrawItem(iItem %d bHotTrack %d unicode flag %d)\n", iItem, bHotTrack, (infoPtr->nNotifyFormat == GXNFR_UNICODE));

  r = phdi->rect;
  if (r.right - r.left == 0)
    return phdi->rect.right;

  /* Set the colors before sending NM_CUSTOMDRAW so that it can change them */
  gxSetTextColor(hdc, (bHotTrack && !theme) ? GXCOLOR_HIGHLIGHT : GXCOLOR_BTNTEXT);
  gxSetBkColor(hdc, gxGetSysColor(GXCOLOR_3DFACE));

  if (lCDFlags & CDRF_NOTIFYITEMDRAW && !(phdi->fmt & HDF_OWNERDRAW))
  {
    GXLRESULT lCDItemFlags;

    nmcd.dwDrawStage  = CDDS_PREPAINT | CDDS_ITEM;
    nmcd.hdc          = hdc;
    nmcd.dwItemSpec   = iItem;
    nmcd.rc           = r;
    nmcd.uItemState   = phdi->bDown ? CDIS_SELECTED : 0;
    nmcd.lItemlParam  = phdi->lParam;

    lCDItemFlags = HEADER_SendNotify(hwnd, GXNM_CUSTOMDRAW, (GXNMHDR *)&nmcd);
    if (lCDItemFlags & CDRF_SKIPDEFAULT)
      return phdi->rect.right;
  }

  if (theme != NULL) {
    int state = (phdi->bDown) ? HIS_PRESSED :
      (bHotTrack ? HIS_HOT : HIS_NORMAL);
    gxDrawThemeBackground (theme, hdc, HP_HEADERITEM, state,
      &r, NULL);
    gxGetThemeBackgroundContentRect (theme, hdc, HP_HEADERITEM, state,
      &r, &r);
  }
  else {
    GXHBRUSH hbr;

    if (gxGetWindowLongW (hwnd, GXGWL_STYLE) & HDS_BUTTONS) {
      if (phdi->bDown) {
        gxDrawEdge (hdc, &r, GXBDR_RAISEDOUTER,
          GXBF_RECT | GXBF_FLAT | GXBF_MIDDLE | GXBF_ADJUST);
      }
      else
        gxDrawEdge (hdc, &r, GXEDGE_RAISED,
        GXBF_RECT | GXBF_SOFT | GXBF_MIDDLE | GXBF_ADJUST);
    }
    else
      gxDrawEdge (hdc, &r, GXEDGE_ETCHED, GXBF_BOTTOM | GXBF_RIGHT | GXBF_ADJUST);

    hbr = gxCreateSolidBrush(gxGetBkColor(hdc));
    gxFillRect(hdc, &r, hbr);
    gxDeleteObject(hbr);
  }
  if (phdi->bDown) {
    r.left += 2;
    r.top  += 2;
  }

  if (phdi->fmt & HDF_OWNERDRAW) {
    GXDRAWITEMSTRUCT dis;

    dis.CtlType    = GXODT_HEADER;
    dis.CtlID      = gxGetWindowLongPtrW (hwnd, GXGWLP_ID);
    dis.itemID     = iItem;
    dis.itemAction = GXODA_DRAWENTIRE;
    dis.itemState  = phdi->bDown ? GXODS_SELECTED : 0;
    dis.hwndItem   = hwnd;
    dis.hDC        = hdc;
    dis.rcItem     = phdi->rect;
    dis.itemData   = phdi->lParam;
    oldBkMode = gxSetBkMode(hdc, GXTRANSPARENT);
    gxSendMessageW (infoPtr->hwndNotify, GXWM_DRAWITEM,
      (GXWPARAM)dis.CtlID, (GXLPARAM)&dis);
    if (oldBkMode != GXTRANSPARENT)
      gxSetBkMode(hdc, oldBkMode);
  }
  else {
    GXUINT rw, rh, /* width and height of r */
      *x = NULL, *w = NULL; /* x and width of the pic (bmp or img) which is part of cnt */
    /* cnt,txt,img,bmp */
    GXUINT cx, tx, ix, bx,
      cw, tw, iw, bw;
    GXBITMAP bmp;

    HEADER_PrepareCallbackItems(hwnd, iItem, HDI_TEXT|HDI_IMAGE);
    cw = tw = iw = bw = 0;
    rw = r.right - r.left;
    rh = r.bottom - r.top;

    if (phdi->fmt & HDF_STRING) {
      GXRECT textRect;

      gxSetRectEmpty(&textRect);
      gxDrawTextW (hdc, phdi->pszText, -1,
        &textRect, GXDT_LEFT|GXDT_VCENTER|GXDT_SINGLELINE|GXDT_CALCRECT);
      cw = textRect.right - textRect.left + 2 * infoPtr->iMargin;
    }

    if ((phdi->fmt & HDF_IMAGE) && (infoPtr->himl)) {
      iw = infoPtr->himl->cx + 2 * infoPtr->iMargin;
      x = &ix;
      w = &iw;
    }

    if ((phdi->fmt & HDF_BITMAP) && (phdi->hbm)) {
      gxGetObjectW (phdi->hbm, sizeof(GXBITMAP), (GXLPVOID)&bmp);
      bw = bmp.bmWidth + 2 * infoPtr->iMargin;
      if (!iw) {
        x = &bx;
        w = &bw;
      }
    }

    if (bw || iw)
      cw += *w; 

    /* align cx using the unclipped cw */
    if ((phdi->fmt & HDF_JUSTIFYMASK) == HDF_LEFT)
      cx = r.left;
    else if ((phdi->fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
      cx = r.left + rw / 2 - cw / 2;
    else /* HDF_RIGHT */
      cx = r.right - cw;

    /* clip cx & cw */
    if (cx < r.left)
      cx = r.left;
    if (cx + cw > r.right)
      cw = r.right - cx;

    tx = cx + infoPtr->iMargin;
    /* since cw might have changed we have to recalculate tw */
    tw = cw - infoPtr->iMargin * 2;

    if (iw || bw) {
      tw -= *w;
      if (phdi->fmt & HDF_BITMAP_ON_RIGHT) {
        /* put pic behind text */
        *x = cx + tw + infoPtr->iMargin * 3;
      } else {
        *x = cx + infoPtr->iMargin;
        /* move text behind pic */
        tx += *w;
      }
    }

    if (iw && bw) {
      /* since we're done with the layout we can
      now calculate the position of bmp which
      has no influence on alignment and layout
      because of img */
      if ((phdi->fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
        bx = cx - bw + infoPtr->iMargin;
      else
        bx = cx + cw + infoPtr->iMargin;
    }

    if (iw || bw) {
      GXHDC hClipDC = gxGetDC(hwnd);
      GXHRGN hClipRgn = gxCreateRectRgn(r.left, r.top, r.right, r.bottom);
      gxSelectClipRgn(hClipDC, hClipRgn);

      if (bw) {
        GXHDC hdcBitmap = gxCreateCompatibleDC (hClipDC);
        gxSelectObject (hdcBitmap, phdi->hbm);
        gxBitBlt (hClipDC, bx, r.top + ((GXINT)rh - bmp.bmHeight) / 2, 
          bmp.bmWidth, bmp.bmHeight, hdcBitmap, 0, 0, GXSRCCOPY);
        gxDeleteDC (hdcBitmap);
      }

      if (iw) {
        gxImageList_DrawEx (infoPtr->himl, phdi->iImage, hClipDC, 
          ix, r.top + ((GXINT)rh - infoPtr->himl->cy) / 2,
          infoPtr->himl->cx, infoPtr->himl->cy, CLR_DEFAULT, CLR_DEFAULT, 0);
      }

      gxDeleteObject(hClipRgn);
      gxReleaseDC(hwnd, hClipDC);
    }

    if (((phdi->fmt & HDF_STRING)
      || (!(phdi->fmt & (HDF_OWNERDRAW|HDF_STRING|HDF_BITMAP|
      HDF_BITMAP_ON_RIGHT|HDF_IMAGE)))) /* no explicit format specified? */
      && (phdi->pszText)) {
        oldBkMode = gxSetBkMode(hdc, GXTRANSPARENT);
        r.left  = tx;
        r.right = tx + tw;
        gxDrawTextW (hdc, phdi->pszText, -1,
          &r, GXDT_LEFT|GXDT_END_ELLIPSIS|GXDT_VCENTER|GXDT_SINGLELINE);
        if (oldBkMode != GXTRANSPARENT)
          gxSetBkMode(hdc, oldBkMode);
    }
    HEADER_FreeCallbackItems(phdi);
  }/*Ownerdrawn*/

  return phdi->rect.right;
}

static void
HEADER_DrawHotDivider(GXHWND hwnd, GXHDC hdc)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXHBRUSH brush;
  GXRECT r;

  HEADER_GetHotDividerRect(hwnd, infoPtr, &r);
  brush = gxCreateSolidBrush(gxGetSysColor(GXCOLOR_HIGHLIGHT));
  gxFillRect(hdc, &r, brush);
  gxDeleteObject(brush);
}

static void
HEADER_Refresh (GXHWND hwnd, GXHDC hdc)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXHFONT hFont, hOldFont;
  GXRECT rect, rcRest;
  GXHBRUSH hbrBk;
  GXUINT i;
  GXINT x;
  GXLRESULT lCDFlags;
  GXHTHEME theme = gxGetWindowTheme (hwnd);

  if (!infoPtr->bRectsValid)
    HEADER_SetItemBounds(hwnd);

  /* get rect for the bar, adjusted for the border */
  gxGetClientRect (hwnd, &rect);
  lCDFlags = HEADER_SendCtrlCustomDraw(hwnd, CDDS_PREPAINT, hdc, &rect);

  if (infoPtr->bDragging)
    gxImageList_DragShowNolock(FALSE);

  hFont = infoPtr->hFont ? infoPtr->hFont : (GXHFONT)gxGetStockObject (GXSYSTEM_FONT);
  hOldFont = (GXHFONT)gxSelectObject (hdc, hFont);

  /* draw Background */
  if (infoPtr->uNumItem == 0 && theme == NULL) {
    hbrBk = gxGetSysColorBrush(GXCOLOR_3DFACE);
    gxFillRect(hdc, &rect, hbrBk);
  }

  x = rect.left;
  for (i = 0; x <= rect.right && i < infoPtr->uNumItem; i++) {
    int idx = HEADER_OrderToIndex(hwnd,i);
    if (gxRectVisible(hdc, &infoPtr->items[idx].rect))
      HEADER_DrawItem(hwnd, hdc, idx, infoPtr->iHotItem == idx, lCDFlags);
    x = infoPtr->items[idx].rect.right;
  }

  rcRest = rect;
  rcRest.left = x;
  if ((x <= rect.right) && gxRectVisible(hdc, &rcRest) && (infoPtr->uNumItem > 0)) {
    if (theme != NULL) {
      gxDrawThemeBackground(theme, hdc, HP_HEADERITEM, HIS_NORMAL, &rcRest, NULL);
    }
    else {
      if (gxGetWindowLongW (hwnd, GXGWL_STYLE) & HDS_BUTTONS)
        gxDrawEdge (hdc, &rcRest, GXEDGE_RAISED, GXBF_TOP|GXBF_LEFT|GXBF_BOTTOM|GXBF_SOFT|GXBF_MIDDLE);
      else
        gxDrawEdge (hdc, &rcRest, GXEDGE_ETCHED, GXBF_BOTTOM|GXBF_MIDDLE);
    }
  }

  if (infoPtr->iHotDivider != -1)
    HEADER_DrawHotDivider(hwnd, hdc);

  if (infoPtr->bDragging)
    gxImageList_DragShowNolock(TRUE);
  gxSelectObject (hdc, hOldFont);

  if (lCDFlags & CDRF_NOTIFYPOSTPAINT)
    HEADER_SendCtrlCustomDraw(hwnd, CDDS_POSTPAINT, hdc, &rect);
}


static void
HEADER_RefreshItem (GXHWND hwnd, GXINT iItem)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  if (!infoPtr->bRectsValid)
    HEADER_SetItemBounds(hwnd);

  gxInvalidateRect(hwnd, &infoPtr->items[iItem].rect, FALSE);
}


static void
HEADER_InternalHitTest (GXHWND hwnd, const GXPOINT *lpPt, GXUINT *pFlags, GXINT *pItem)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXRECT rect, rcTest;
  GXUINT iCount;
  GXINT width;
  GXBOOL bNoWidth;

  gxGetClientRect (hwnd, &rect);

  *pFlags = 0;
  bNoWidth = FALSE;
  if (gxPtInRect (&rect, *lpPt))
  {
    if (infoPtr->uNumItem == 0) {
      *pFlags |= HHT_NOWHERE;
      *pItem = 1;
      TRACE("NOWHERE\n");
      return;
    }
    else {
      /* somewhere inside */
      for (iCount = 0; iCount < infoPtr->uNumItem; iCount++) {
        rect = infoPtr->items[iCount].rect;
        width = rect.right - rect.left;
        if (width == 0) {
          bNoWidth = TRUE;
          continue;
        }
        if (gxPtInRect (&rect, *lpPt)) {
          if (width <= 2 * DIVIDER_WIDTH) {
            *pFlags |= HHT_ONHEADER;
            *pItem = iCount;
            TRACE("ON HEADER %d\n", iCount);
            return;
          }
          if (HEADER_IndexToOrder(hwnd, iCount) > 0) {
            rcTest = rect;
            rcTest.right = rcTest.left + DIVIDER_WIDTH;
            if (gxPtInRect (&rcTest, *lpPt)) {
              if (bNoWidth) {
                *pFlags |= HHT_ONDIVOPEN;
                *pItem = HEADER_PrevItem(hwnd, iCount);
                TRACE("ON DIVOPEN %d\n", *pItem);
                return;
              }
              else {
                *pFlags |= HHT_ONDIVIDER;
                *pItem = HEADER_PrevItem(hwnd, iCount);
                TRACE("ON DIVIDER %d\n", *pItem);
                return;
              }
            }
          }
          rcTest = rect;
          rcTest.left = rcTest.right - DIVIDER_WIDTH;
          if (gxPtInRect (&rcTest, *lpPt)) {
            *pFlags |= HHT_ONDIVIDER;
            *pItem = iCount;
            TRACE("ON DIVIDER %d\n", *pItem);
            return;
          }

          *pFlags |= HHT_ONHEADER;
          *pItem = iCount;
          TRACE("ON HEADER %d\n", iCount);
          return;
        }
      }

      /* check for last divider part (on nowhere) */
      rect = infoPtr->items[infoPtr->uNumItem-1].rect;
      rect.left = rect.right;
      rect.right += DIVIDER_WIDTH;
      if (gxPtInRect (&rect, *lpPt)) {
        if (bNoWidth) {
          *pFlags |= HHT_ONDIVOPEN;
          *pItem = infoPtr->uNumItem - 1;
          TRACE("ON DIVOPEN %d\n", *pItem);
          return;
        }
        else {
          *pFlags |= HHT_ONDIVIDER;
          *pItem = infoPtr->uNumItem-1;
          TRACE("ON DIVIDER %d\n", *pItem);
          return;
        }
      }

      *pFlags |= HHT_NOWHERE;
      *pItem = 1;
      TRACE("NOWHERE\n");
      return;
    }
  }
  else {
    if (lpPt->x < rect.left) {
      TRACE("TO LEFT\n");
      *pFlags |= HHT_TOLEFT;
    }
    else if (lpPt->x > rect.right) {
      TRACE("TO RIGHT\n");
      *pFlags |= HHT_TORIGHT;
    }

    if (lpPt->y < rect.top) {
      TRACE("ABOVE\n");
      *pFlags |= HHT_ABOVE;
    }
    else if (lpPt->y > rect.bottom) {
      TRACE("BELOW\n");
      *pFlags |= HHT_BELOW;
    }
  }

  *pItem = 1;
  TRACE("flags=0x%X\n", *pFlags);
  return;
}


static void
HEADER_DrawTrackLine (GXHWND hwnd, GXHDC hdc, GXINT x)
{
  GXRECT rect;
  GXHPEN hOldPen;
  GXINT  oldRop;

  gxGetClientRect (hwnd, &rect);

  hOldPen = (GXHPEN)gxSelectObject (hdc, gxGetStockObject (GXBLACK_PEN));
  oldRop = gxSetROP2 (hdc, GXR2_XORPEN);
  gxMoveToEx (hdc, x, rect.top, NULL);
  gxLineTo (hdc, x, rect.bottom);
  gxSetROP2 (hdc, oldRop);
  gxSelectObject (hdc, hOldPen);
}

/***
* DESCRIPTION:
* Convert a HDITEM into the correct format (ANSI/Unicode) to send it in a notify
*
* PARAMETER(S):
* [I] infoPtr : the header that wants to send the notify
* [O] dest : The buffer to store the HDITEM for notify. It may be set to a HDITEMA of GXHDITEMW
* [I] src  : The source HDITEM. It may be a HDITEMA or GXHDITEMW
* [I] fSourceUnicode : is src a GXHDITEMW or HDITEMA
* [O] ppvScratch : a pointer to a scratch buffer that needs to be freed after
*                  the HDITEM is no longer in use or NULL if none was needed
* 
* NOTE: We depend on HDITEMA and GXHDITEMW having the same structure
*/
static void HEADER_CopyHDItemForNotify(const HEADER_INFO *infoPtr, GXHDITEMW *dest,
                     const GXHDITEMW *src, GXBOOL fSourceUnicode, GXLPVOID *ppvScratch)
{
  *ppvScratch = NULL;
  *dest = *src;

  if (src->mask & HDI_TEXT && src->pszText != GXLPSTR_TEXTCALLBACKW) /* covers TEXTCALLBACKA as well */
  {
    if (fSourceUnicode && infoPtr->nNotifyFormat != GXNFR_UNICODE)
    {
      dest->pszText = NULL;
      gxStr_SetPtrWtoA((GXLPSTR *)&dest->pszText, src->pszText);
      *ppvScratch = dest->pszText;
    }

    if (!fSourceUnicode && infoPtr->nNotifyFormat == GXNFR_UNICODE)
    {
      dest->pszText = NULL;
      gxStr_SetPtrAtoW(&dest->pszText, (GXLPSTR)src->pszText);
      *ppvScratch = dest->pszText;
    }
  }
}

static GXUINT HEADER_NotifyCodeWtoA(GXUINT code)
{
  /* we use the fact that all the unicode messages are in HDN_FIRST_UNICODE..HDN_LAST*/
  if (code >= HDN_LAST && code <= HDN_FIRST_UNICODE)
    return code + HDN_UNICODE_OFFSET;
  else
    return code;
}

static GXLRESULT
HEADER_SendNotify(GXHWND hwnd, GXUINT code, GXNMHDR *nmhdr)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  nmhdr->hwndFrom = hwnd;
  nmhdr->idFrom   = gxGetWindowLongPtrW (hwnd, GXGWLP_ID);
  nmhdr->code     = code;

  return gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY,
    nmhdr->idFrom, (GXLPARAM)nmhdr);
}

static GXBOOL
HEADER_SendSimpleNotify (GXHWND hwnd, GXUINT code)
{
  GXNMHDR nmhdr;
  return (GXBOOL)HEADER_SendNotify(hwnd, code, &nmhdr);
}

static GXLRESULT
HEADER_SendCtrlCustomDraw(GXHWND hwnd, GXDWORD dwDrawStage, GXHDC hdc, const GXRECT *rect)
{
  GXNMCUSTOMDRAW nm;
  nm.dwDrawStage = dwDrawStage;
  nm.hdc = hdc;
  nm.rc = *rect;
  nm.dwItemSpec = 0;
  nm.uItemState = 0;
  nm.lItemlParam = 0;

  return HEADER_SendNotify(hwnd, GXNM_CUSTOMDRAW, (GXNMHDR *)&nm);
}

static GXBOOL
HEADER_SendNotifyWithHDItemT(GXHWND hwnd, GXUINT code, GXINT iItem, GXHDITEMW *lpItem)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXNMHEADERW nmhdr;

  if (infoPtr->nNotifyFormat != GXNFR_UNICODE)
    code = HEADER_NotifyCodeWtoA(code);
  nmhdr.iItem = iItem;
  nmhdr.iButton = 0;
  nmhdr.pitem = lpItem;

  return (GXBOOL)HEADER_SendNotify(hwnd, code, (GXNMHDR *)&nmhdr);
}

static GXBOOL
HEADER_SendNotifyWithIntFieldT(GXHWND hwnd, GXUINT code, GXINT iItem, GXINT mask, GXINT iValue)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXHDITEMW nmitem;

  /* copying only the iValue should be ok but to make the code more robust we copy everything */
  nmitem.cxy = infoPtr->items[iItem].cxy;
  nmitem.hbm = infoPtr->items[iItem].hbm;
  nmitem.pszText = NULL;
  nmitem.cchTextMax = 0;
  nmitem.fmt = infoPtr->items[iItem].fmt;
  nmitem.lParam = infoPtr->items[iItem].lParam;
  nmitem.iOrder = infoPtr->items[iItem].iOrder;
  nmitem.iImage = infoPtr->items[iItem].iImage;

  nmitem.mask = mask;
  switch (mask)
  {
  case HDI_WIDTH:
    nmitem.cxy = iValue;
    break;
  case HDI_ORDER:
    nmitem.iOrder = iValue;
    break;
  default:
    ERR("invalid mask value 0x%x\n", iValue);
  }

  return HEADER_SendNotifyWithHDItemT(hwnd, code, iItem, &nmitem);
}

/**
* Prepare callback items
*   depends on GXNMHDDISPINFOW having same structure as NMHDDISPINFOA 
*   (so we handle the two cases only doing a specific cast for pszText).
* Checks if any of the required field are callback. If there are sends a 
* NMHDISPINFO notify to retrieve these items. The items are stored in the
* HEADER_ITEM pszText and iImage fields. They should be freed with
* HEADER_FreeCallbackItems.
*
* @param hwnd : hwnd header container handler
* @param iItem : the header item id
* @param reqMask : required fields. If any of them is callback this function will fetch it
*
* @return TRUE on success, else FALSE
*/
static GXBOOL
HEADER_PrepareCallbackItems(GXHWND hwnd, GXINT iItem, GXINT reqMask)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem = &infoPtr->items[iItem];
  GXDWORD mask = reqMask & lpItem->callbackMask;
  GXNMHDDISPINFOW dispInfo;
  void *pvBuffer = NULL;

  if (mask == 0)
    return TRUE;
  if (mask&HDI_TEXT && lpItem->pszText != NULL)
  {
    ERR("(): function called without a call to FreeCallbackItems\n");
    Free(lpItem->pszText);
    lpItem->pszText = NULL;
  }

  memset(&dispInfo, 0, sizeof(GXNMHDDISPINFOW));
  dispInfo.hdr.hwndFrom = hwnd;
  dispInfo.hdr.idFrom   = gxGetWindowLongPtrW (hwnd, GXGWLP_ID);
  if (infoPtr->nNotifyFormat == GXNFR_UNICODE)
  {
    dispInfo.hdr.code = HDN_GETDISPINFOW;
    if (mask & HDI_TEXT)
      pvBuffer = Alloc(MAX_HEADER_TEXT_LEN * sizeof(GXWCHAR));
  }
  else
  {
    dispInfo.hdr.code = HDN_GETDISPINFOA;
    if (mask & HDI_TEXT)
      pvBuffer = Alloc(MAX_HEADER_TEXT_LEN * sizeof(GXCHAR));
  }
  dispInfo.pszText      = (GXLPWSTR)pvBuffer;
  dispInfo.cchTextMax   = (pvBuffer!=NULL?MAX_HEADER_TEXT_LEN:0);
  dispInfo.iItem        = iItem;
  dispInfo.mask         = mask;
  dispInfo.lParam       = lpItem->lParam;

  TRACE("Sending HDN_GETDISPINFO%c\n", infoPtr->nNotifyFormat == GXNFR_UNICODE?'W':'A');
  gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY, dispInfo.hdr.idFrom, (GXLPARAM)&dispInfo);

  //TRACE("gxSendMessage returns(mask:0x%x,str:%s,lParam:%p)\n", 
  //      dispInfo.mask,
  //      (infoPtr->nNotifyFormat == GXNFR_UNICODE ? debugstr_w(dispInfo.pszText) : (GXLPSTR) dispInfo.pszText),
  //      (void*) dispInfo.lParam);

  if (mask & HDI_IMAGE)
    lpItem->iImage = dispInfo.iImage;
  if (mask & HDI_TEXT)
  {
    if (infoPtr->nNotifyFormat == GXNFR_UNICODE)
    {
      lpItem->pszText = (GXLPWSTR)pvBuffer;

      /* the user might have used his own buffer */
      if (dispInfo.pszText != lpItem->pszText)
        gxStr_GetPtrW(dispInfo.pszText, lpItem->pszText, MAX_HEADER_TEXT_LEN);
    }
    else
    {
      gxStr_SetPtrAtoW(&lpItem->pszText, (GXLPSTR)dispInfo.pszText);
      Free(pvBuffer);
    }
  }

  if (dispInfo.mask & HDI_DI_SETITEM) 
  {
    /* make the items permanent */
    lpItem->callbackMask &= ~dispInfo.mask;
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Free the items that might be allocated with HEADER_PrepareCallbackItems
*
* PARAMETER(S):
* [I] lpItem : the item to free the data
*
*/
static void
HEADER_FreeCallbackItems(HEADER_ITEM *lpItem)
{
  if (lpItem->callbackMask&HDI_TEXT)
  {
    Free(lpItem->pszText);
    lpItem->pszText = NULL;
  }

  if (lpItem->callbackMask&HDI_IMAGE)
    lpItem->iImage = GXI_IMAGECALLBACK;
}

static GXLRESULT
HEADER_CreateDragImage (GXHWND hwnd, GXWPARAM wParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr(hwnd);
  HEADER_ITEM *lpItem;
  GXHIMAGELIST himl;
  GXHBITMAP hMemory, hOldBitmap;
  GXLRESULT lCDFlags;
  GXRECT rc;
  GXHDC hMemoryDC;
  GXHDC hDeviceDC;
  int height, width;
  GXHFONT hFont;

  if (wParam >= infoPtr->uNumItem)
    return FALSE;

  if (!infoPtr->bRectsValid)
    HEADER_SetItemBounds(hwnd);

  lpItem = &infoPtr->items[wParam];
  width = lpItem->rect.right - lpItem->rect.left;
  height = lpItem->rect.bottom - lpItem->rect.top;

  hDeviceDC = gxGetDC(NULL);
  hMemoryDC = gxCreateCompatibleDC(hDeviceDC);
  hMemory = gxCreateCompatibleBitmap(hDeviceDC, width, height);
  gxReleaseDC(NULL, hDeviceDC);
  hOldBitmap = (GXHBITMAP)gxSelectObject(hMemoryDC, hMemory);
  gxSetViewportOrgEx(hMemoryDC, -lpItem->rect.left, -lpItem->rect.top, NULL);
  hFont = infoPtr->hFont ? infoPtr->hFont : (GXHFONT)gxGetStockObject(GXSYSTEM_FONT);
  gxSelectObject(hMemoryDC, hFont);

  gxGetClientRect(hwnd, &rc);
  lCDFlags = HEADER_SendCtrlCustomDraw(hwnd, CDDS_PREPAINT, hMemoryDC, &rc);
  HEADER_DrawItem(hwnd, hMemoryDC, wParam, FALSE, lCDFlags);
  if (lCDFlags & CDRF_NOTIFYPOSTPAINT)
    HEADER_SendCtrlCustomDraw(hwnd, CDDS_POSTPAINT, hMemoryDC, &rc);

  hMemory = (GXHBITMAP)gxSelectObject(hMemoryDC, hOldBitmap);
  gxDeleteDC(hMemoryDC);

  if (hMemory == NULL)    /* if anything failed */
    return FALSE;

  himl = gxImageList_Create(width, height, GXILC_COLORDDB, 1, 1);
  gxImageList_Add(himl, hMemory, NULL);
  gxDeleteObject(hMemory);
  return (GXLRESULT)himl;
}

static GXLRESULT
HEADER_SetHotDivider(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr(hwnd);
  GXINT iDivider;
  GXRECT r;

  if (wParam)
  {
    GXPOINT pt;
    GXUINT flags;
    pt.x = (GXINT)(GXSHORT)GXLOWORD(lParam);
    pt.y = 0;
    HEADER_InternalHitTest (hwnd, &pt, &flags, &iDivider);

    if (flags & HHT_TOLEFT)
      iDivider = 0;
    else if (flags & HHT_NOWHERE || flags & HHT_TORIGHT)
      iDivider = infoPtr->uNumItem;
    else
    {
      HEADER_ITEM *lpItem = &infoPtr->items[iDivider];
      if (pt.x > (lpItem->rect.left+lpItem->rect.right)/2)
        iDivider = HEADER_NextItem(hwnd, iDivider);
    }
  }
  else
    iDivider = (GXINT)lParam;

  /* Note; wParam==FALSE, lParam==-1 is valid and is used to clear the hot divider */
  if (iDivider<-1 || iDivider>(int)infoPtr->uNumItem)
    return iDivider;

  if (iDivider != infoPtr->iHotDivider)
  {
    if (infoPtr->iHotDivider != -1)
    {
      HEADER_GetHotDividerRect(hwnd, infoPtr, &r);
      gxInvalidateRect(hwnd, &r, FALSE);
    }
    infoPtr->iHotDivider = iDivider;
    if (iDivider != -1)
    {
      HEADER_GetHotDividerRect(hwnd, infoPtr, &r);
      gxInvalidateRect(hwnd, &r, FALSE);
    }
  }
  return iDivider;
}

static GXLRESULT
HEADER_DeleteItem (GXHWND hwnd, GXWPARAM wParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr(hwnd);
  GXINT iItem = (GXINT)wParam;
  GXINT iOrder;
  GXUINT i;

  TRACE("[iItem=%d]\n", iItem);

  if ((iItem < 0) || (iItem >= (GXINT)infoPtr->uNumItem))
    return FALSE;

  for (i = 0; i < infoPtr->uNumItem; i++)
    TRACE("%d: order=%d, iOrder=%d, ->iOrder=%d\n", i, infoPtr->order[i], infoPtr->items[i].iOrder, infoPtr->items[infoPtr->order[i]].iOrder);

  iOrder = infoPtr->items[iItem].iOrder;
  Free(infoPtr->items[iItem].pszText);

  infoPtr->uNumItem--;
  memmove(&infoPtr->items[iItem], &infoPtr->items[iItem + 1],
    (infoPtr->uNumItem - iItem) * sizeof(HEADER_ITEM));
  memmove(&infoPtr->order[iOrder], &infoPtr->order[iOrder + 1],
    (infoPtr->uNumItem - iOrder) * sizeof(GXINT));
  infoPtr->items = (HEADER_ITEM *)ReAlloc(infoPtr->items, sizeof(HEADER_ITEM) * infoPtr->uNumItem);
  infoPtr->order = (GXINT*)ReAlloc(infoPtr->order, sizeof(GXINT) * infoPtr->uNumItem);

  /* Correct the orders */
  for (i = 0; i < infoPtr->uNumItem; i++)
  {
    if (infoPtr->order[i] > iItem)
      infoPtr->order[i]--;
    if (i >= iOrder)
      infoPtr->items[infoPtr->order[i]].iOrder = i;
  }
  for (i = 0; i < infoPtr->uNumItem; i++)
    TRACE("%d: order=%d, iOrder=%d, ->iOrder=%d\n", i, infoPtr->order[i], infoPtr->items[i].iOrder, infoPtr->items[infoPtr->order[i]].iOrder);

  HEADER_SetItemBounds (hwnd);
  gxInvalidateRect(hwnd, NULL, FALSE);

  return TRUE;
}


static GXLRESULT
HEADER_GetImageList (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  return (GXLRESULT)infoPtr->himl;
}


static GXLRESULT
HEADER_GetItemT (GXHWND hwnd, GXINT nItem, GXLPHDITEMW phdi, GXBOOL bUnicode)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem;
  GXUINT mask;

  if (!phdi)
    return FALSE;

  TRACE("[nItem=%d]\n", nItem);

  mask = phdi->mask;
  if (mask == 0)
    return TRUE;

  if ((nItem < 0) || (nItem >= (GXINT)infoPtr->uNumItem))
    return FALSE;

  if (mask & HDI_UNKNOWN_FIELDS)
  {
    TRACE("mask %x contains unknown fields. Using only comctl32 4.0 fields\n", mask);
    mask &= HDI_COMCTL32_4_0_FIELDS;
  }

  lpItem = &infoPtr->items[nItem];
  HEADER_PrepareCallbackItems(hwnd, nItem, mask);

  if (mask & HDI_BITMAP)
    phdi->hbm = lpItem->hbm;

  if (mask & HDI_FORMAT)
    phdi->fmt = lpItem->fmt;

  if (mask & HDI_WIDTH)
    phdi->cxy = lpItem->cxy;

  if (mask & HDI_LPARAM)
    phdi->lParam = lpItem->lParam;

  if (mask & HDI_IMAGE) 
    phdi->iImage = lpItem->iImage;

  if (mask & HDI_ORDER)
    phdi->iOrder = lpItem->iOrder;

  if (mask & HDI_TEXT)
  {
    if (bUnicode)
      gxStr_GetPtrW (lpItem->pszText, phdi->pszText, phdi->cchTextMax);
    else
      gxStr_GetPtrWtoA (lpItem->pszText, (GXLPSTR)phdi->pszText, phdi->cchTextMax);
  }

  HEADER_FreeCallbackItems(lpItem);
  return TRUE;
}


static inline GXLRESULT
HEADER_GetItemCount (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  return infoPtr->uNumItem;
}


static GXLRESULT
HEADER_GetItemRect (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXINT iItem = (GXINT)wParam;
  LPGXRECT lpRect = (LPGXRECT)lParam;

  if ((iItem < 0) || (iItem >= (GXINT)infoPtr->uNumItem))
    return FALSE;

  lpRect->left   = infoPtr->items[iItem].rect.left;
  lpRect->right  = infoPtr->items[iItem].rect.right;
  lpRect->top    = infoPtr->items[iItem].rect.top;
  lpRect->bottom = infoPtr->items[iItem].rect.bottom;

  return TRUE;
}


static GXLRESULT
HEADER_GetOrderArray(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLPINT order = (GXLPINT) lParam;
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  if ((unsigned int)wParam <infoPtr->uNumItem)
    return FALSE;

  memcpy(order, infoPtr->order, infoPtr->uNumItem * sizeof(GXINT));
  return TRUE;
}

static GXLRESULT
HEADER_SetOrderArray(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  int i;
  GXLPINT order = (GXLPINT) lParam;
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem;

  if ((unsigned int)wParam <infoPtr->uNumItem)
    return FALSE;
  memcpy(infoPtr->order, order, infoPtr->uNumItem * sizeof(GXINT));
  for (i=0; i<(int)wParam; i++)
  {
    lpItem = &infoPtr->items[*order++];
    lpItem->iOrder=i;
  }
  infoPtr->bRectsValid=0;
  gxInvalidateRect(hwnd, NULL, FALSE);
  return TRUE;
}

static inline GXLRESULT
HEADER_GetUnicodeFormat (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  return (infoPtr->nNotifyFormat == GXNFR_UNICODE);
}


static GXLRESULT
HEADER_HitTest (GXHWND hwnd, GXLPARAM lParam)
{
  GXLPHDHITTESTINFO phti = (GXLPHDHITTESTINFO)lParam;

  HEADER_InternalHitTest (hwnd, &phti->pt, &phti->flags, &phti->iItem);

  if (phti->flags == HHT_NOWHERE)
    return -1;
  else
    return phti->iItem;
}


static GXLRESULT
HEADER_InsertItemT (GXHWND hwnd, GXINT nItem, const GXHDITEMW *phdi, GXBOOL bUnicode)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem;
  GXINT       iOrder;
  GXUINT      i;
  GXUINT      copyMask;

  if ((phdi == NULL) || (nItem < 0) || (phdi->mask == 0))
    return -1;

  if (nItem > infoPtr->uNumItem)
    nItem = infoPtr->uNumItem;

  iOrder = (phdi->mask & HDI_ORDER) ? phdi->iOrder : nItem;
  if (iOrder < 0)
    iOrder = 0;
  else if (infoPtr->uNumItem < iOrder)
    iOrder = infoPtr->uNumItem;

  infoPtr->uNumItem++;
  infoPtr->items = (HEADER_ITEM *)ReAlloc(infoPtr->items, sizeof(HEADER_ITEM) * infoPtr->uNumItem);
  infoPtr->order = (GXINT*)ReAlloc(infoPtr->order, sizeof(GXINT) * infoPtr->uNumItem);

  /* make space for the new item */
  memmove(&infoPtr->items[nItem + 1], &infoPtr->items[nItem],
    (infoPtr->uNumItem - nItem - 1) * sizeof(HEADER_ITEM));
  memmove(&infoPtr->order[iOrder + 1], &infoPtr->order[iOrder],
    (infoPtr->uNumItem - iOrder - 1) * sizeof(GXINT));

  /* update the order array */
  infoPtr->order[iOrder] = nItem;
  for (i = 0; i < infoPtr->uNumItem; i++)
  {
    if (i != iOrder && infoPtr->order[i] >= nItem)
      infoPtr->order[i]++;
    infoPtr->items[infoPtr->order[i]].iOrder = i;
  }

  lpItem = &infoPtr->items[nItem];
  ZeroMemory(lpItem, sizeof(HEADER_ITEM));
  /* cxy, fmt and lParam are copied even if not in the HDITEM mask */
  copyMask = phdi->mask | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
  HEADER_StoreHDItemInHeader(lpItem, copyMask, phdi, bUnicode);
  lpItem->iOrder = iOrder;

  /* set automatically some format bits */
  if (phdi->mask & HDI_TEXT)
    lpItem->fmt |= HDF_STRING;
  else
    lpItem->fmt &= ~HDF_STRING;

  if (lpItem->hbm != NULL)
    lpItem->fmt |= HDF_BITMAP;
  else
    lpItem->fmt &= ~HDF_BITMAP;

  if (phdi->mask & HDI_IMAGE)
    lpItem->fmt |= HDF_IMAGE;

  HEADER_SetItemBounds (hwnd);
  gxInvalidateRect(hwnd, NULL, FALSE);

  return nItem;
}


static GXLRESULT
HEADER_Layout (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXLPHDLAYOUT lpLayout = (GXLPHDLAYOUT)lParam;

  lpLayout->pwpos->hwnd = hwnd;
  lpLayout->pwpos->hwndInsertAfter = 0;
  lpLayout->pwpos->x = lpLayout->prc->left;
  lpLayout->pwpos->y = lpLayout->prc->top;
  lpLayout->pwpos->cx = lpLayout->prc->right - lpLayout->prc->left;
  if (gxGetWindowLongW (hwnd, GXGWL_STYLE) & HDS_HIDDEN)
    lpLayout->pwpos->cy = 0;
  else {
    lpLayout->pwpos->cy = infoPtr->nHeight;
    lpLayout->prc->top += infoPtr->nHeight;
  }
  lpLayout->pwpos->flags = GXSWP_NOZORDER;

  TRACE("Layout x=%d y=%d cx=%d cy=%d\n",
    lpLayout->pwpos->x, lpLayout->pwpos->y,
    lpLayout->pwpos->cx, lpLayout->pwpos->cy);

  infoPtr->bRectsValid = FALSE;

  return TRUE;
}


static GXLRESULT
HEADER_SetImageList (GXHWND hwnd, GXHIMAGELIST himl)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXHIMAGELIST himlOld;

  TRACE("(himl %p)\n", himl);
  himlOld = infoPtr->himl;
  infoPtr->himl = himl;

  /* FIXME: Refresh needed??? */

  return (GXLRESULT)himlOld;
}


static GXLRESULT
HEADER_GetBitmapMargin(GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr(hwnd);

  return infoPtr->iMargin;
}

static GXLRESULT
HEADER_SetBitmapMargin(GXHWND hwnd, GXWPARAM wParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXINT oldMargin = infoPtr->iMargin;

  infoPtr->iMargin = (GXINT)wParam;

  return oldMargin;
}

static GXLRESULT
HEADER_SetItemT (GXHWND hwnd, GXINT nItem, const GXHDITEMW *phdi, GXBOOL bUnicode)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem;
  GXHDITEMW hdNotify;
  void *pvScratch;

  if (phdi == NULL)
    return FALSE;
  if ((nItem < 0) || (nItem >= (GXINT)infoPtr->uNumItem))
    return FALSE;

  TRACE("[nItem=%d]\n", nItem);

  HEADER_CopyHDItemForNotify(infoPtr, &hdNotify, phdi, bUnicode, &pvScratch);
  if (HEADER_SendNotifyWithHDItemT(hwnd, HDN_ITEMCHANGINGW, nItem, &hdNotify))
  {
    Free(pvScratch);
    return FALSE;
  }

  lpItem = &infoPtr->items[nItem];
  HEADER_StoreHDItemInHeader(lpItem, phdi->mask, phdi, bUnicode);

  if (phdi->mask & HDI_ORDER)
    if (phdi->iOrder >= 0 && phdi->iOrder < infoPtr->uNumItem)
      HEADER_ChangeItemOrder(infoPtr, nItem, phdi->iOrder);

  HEADER_SendNotifyWithHDItemT(hwnd, HDN_ITEMCHANGEDW, nItem, &hdNotify);

  HEADER_SetItemBounds (hwnd);

  gxInvalidateRect(hwnd, NULL, FALSE);

  Free(pvScratch);
  return TRUE;
}

static inline GXLRESULT
HEADER_SetUnicodeFormat (GXHWND hwnd, GXWPARAM wParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXBOOL bTemp = (infoPtr->nNotifyFormat == GXNFR_UNICODE);

  infoPtr->nNotifyFormat = ((GXBOOL)wParam ? GXNFR_UNICODE : GXNFR_ANSI);

  return bTemp;
}


static GXLRESULT
HEADER_Create (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr;
  GXTEXTMETRICW tm;
  GXHFONT hOldFont;
  GXHDC   hdc;

  infoPtr = (HEADER_INFO *)Alloc (sizeof(HEADER_INFO));
  gxSetWindowLongPtrW (hwnd, 0, (GXDWORD_PTR)infoPtr);

  infoPtr->hwndNotify = ((GXLPCREATESTRUCTW)lParam)->hwndParent;
  infoPtr->uNumItem = 0;
  infoPtr->hFont = 0;
  infoPtr->items = 0;
  infoPtr->order = 0;
  infoPtr->bRectsValid = FALSE;
  infoPtr->hcurArrow = gxLoadCursorW (0, (GXLPWSTR)GXIDC_ARROW);
  infoPtr->hcurDivider = gxLoadCursorW (COMCTL32_hModule, GXMAKERESOURCEW(GXIDC_DIVIDER));
  infoPtr->hcurDivopen = gxLoadCursorW (COMCTL32_hModule, GXMAKERESOURCEW(GXIDC_DIVIDEROPEN));
  infoPtr->bPressed  = FALSE;
  infoPtr->bTracking = FALSE;
  infoPtr->iMoveItem = 0;
  infoPtr->himl = 0;
  infoPtr->iHotItem = -1;
  infoPtr->iHotDivider = -1;
  infoPtr->iMargin = 3*gxGetSystemMetrics(GXSM_CXEDGE);
  infoPtr->nNotifyFormat =
    gxSendMessageW (infoPtr->hwndNotify, GXWM_NOTIFYFORMAT, (GXWPARAM)hwnd, GXNF_QUERY);

  hdc = gxGetDC (0);
  hOldFont = (GXHFONT)gxSelectObject (hdc, gxGetStockObject (GXSYSTEM_FONT));
  gxGetTextMetricsW (hdc, &tm);
  infoPtr->nHeight = tm.tmHeight + VERT_BORDER;
  gxSelectObject (hdc, hOldFont);
  gxReleaseDC (0, hdc);

  gxOpenThemeData(hwnd, themeClass);

  return 0;
}


static GXLRESULT
HEADER_Destroy (GXHWND hwnd)
{
  GXHTHEME theme = gxGetWindowTheme(hwnd);
  gxCloseThemeData(theme);
  return 0;
}

static GXLRESULT
HEADER_NCDestroy (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  HEADER_ITEM *lpItem;
  GXINT nItem;

  if (infoPtr->items) {
    lpItem = infoPtr->items;
    for (nItem = 0; nItem < infoPtr->uNumItem; nItem++, lpItem++) {
      Free(lpItem->pszText);
    }
    Free (infoPtr->items);
  }

  Free(infoPtr->order);

  if (infoPtr->himl)
    gxImageList_Destroy (infoPtr->himl);

  gxSetWindowLongPtrW (hwnd, 0, 0);
  Free (infoPtr);

  return 0;
}


static inline GXLRESULT
HEADER_GetFont (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  return (GXLRESULT)infoPtr->hFont;
}


static GXBOOL
HEADER_IsDragDistance(const HEADER_INFO *infoPtr, const GXPOINT *pt)
{
  /* Windows allows for a mouse movement before starting the drag. We use the
  * SM_CXDOUBLECLICK/SM_CYDOUBLECLICK as that distance.
  */
  return (abs(infoPtr->ptLButtonDown.x - pt->x)>gxGetSystemMetrics(GXSM_CXDOUBLECLK) ||
    abs(infoPtr->ptLButtonDown.y - pt->y)>gxGetSystemMetrics(GXSM_CYDOUBLECLK));
}

static GXLRESULT
HEADER_LButtonDblClk (GXHWND hwnd, GXLPARAM lParam)
{
  GXPOINT pt;
  GXUINT  flags;
  GXINT   nItem;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);
  HEADER_InternalHitTest (hwnd, &pt, &flags, &nItem);

  if ((gxGetWindowLongW (hwnd, GXGWL_STYLE) & HDS_BUTTONS) && (flags == HHT_ONHEADER))
    HEADER_SendNotifyWithHDItemT(hwnd, HDN_ITEMDBLCLICKW, nItem, NULL);
  else if ((flags == HHT_ONDIVIDER) || (flags == HHT_ONDIVOPEN))
    HEADER_SendNotifyWithHDItemT(hwnd, HDN_DIVIDERDBLCLICKW, nItem, NULL);

  return 0;
}


static GXLRESULT
HEADER_LButtonDown (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXDWORD dwStyle = gxGetWindowLongW (hwnd, GXGWL_STYLE);
  GXPOINT pt;
  GXUINT  flags;
  GXINT   nItem;
  GXHDC   hdc;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);
  HEADER_InternalHitTest (hwnd, &pt, &flags, &nItem);

  if ((dwStyle & HDS_BUTTONS) && (flags == HHT_ONHEADER)) {
    gxSetCapture (hwnd);
    infoPtr->bCaptured = TRUE;
    infoPtr->bPressed  = TRUE;
    infoPtr->bDragging = FALSE;
    infoPtr->iMoveItem = nItem;
    infoPtr->ptLButtonDown = pt;

    infoPtr->items[nItem].bDown = TRUE;

    /* Send WM_CUSTOMDRAW */
    hdc = gxGetDC (hwnd);
    HEADER_RefreshItem (hwnd, nItem);
    gxReleaseDC (hwnd, hdc);

    TRACE("Pressed item %d!\n", nItem);
  }
  else if ((flags == HHT_ONDIVIDER) || (flags == HHT_ONDIVOPEN)) {
    GXINT iCurrWidth = infoPtr->items[nItem].cxy;
    if (!HEADER_SendNotifyWithIntFieldT(hwnd, HDN_BEGINTRACKW, nItem, HDI_WIDTH, iCurrWidth))
    {
      gxSetCapture (hwnd);
      infoPtr->bCaptured = TRUE;
      infoPtr->bTracking = TRUE;
      infoPtr->iMoveItem = nItem;
      infoPtr->xTrackOffset = infoPtr->items[nItem].rect.right - pt.x;

      if (!(dwStyle & HDS_FULLDRAG)) {
        infoPtr->xOldTrack = infoPtr->items[nItem].rect.right;
        hdc = gxGetDC (hwnd);
        HEADER_DrawTrackLine (hwnd, hdc, infoPtr->xOldTrack);
        gxReleaseDC (hwnd, hdc);
      }

      TRACE("Begin tracking item %d!\n", nItem);
    }
  }

  return 0;
}


static GXLRESULT
HEADER_LButtonUp (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXDWORD dwStyle = gxGetWindowLongW (hwnd, GXGWL_STYLE);
  GXPOINT pt;
  GXUINT  flags;
  GXINT   nItem;
  GXHDC   hdc;

  pt.x = (GXINT)(GXSHORT)GXLOWORD(lParam);
  pt.y = (GXINT)(GXSHORT)GXHIWORD(lParam);
  HEADER_InternalHitTest (hwnd, &pt, &flags, &nItem);

  if (infoPtr->bPressed) {
    if (infoPtr->bDragging)
    {
      HEADER_ITEM *lpItem = &infoPtr->items[infoPtr->iMoveItem];
      GXINT iNewOrder;

      gxImageList_DragShowNolock(FALSE);
      gxImageList_EndDrag();
      lpItem->bDown=FALSE;

      if (infoPtr->iHotDivider == -1)
        iNewOrder = -1;
      else if (infoPtr->iHotDivider == infoPtr->uNumItem)
        iNewOrder = infoPtr->uNumItem-1;
      else
      {
        iNewOrder = HEADER_IndexToOrder(hwnd, infoPtr->iHotDivider);
        if (iNewOrder > lpItem->iOrder)
          iNewOrder--;
      }

      if (iNewOrder != -1 &&
        !HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ENDDRAG, infoPtr->iMoveItem, HDI_ORDER, iNewOrder))
      {
        HEADER_ChangeItemOrder(infoPtr, infoPtr->iMoveItem, iNewOrder);
        infoPtr->bRectsValid = FALSE;
        gxInvalidateRect(hwnd, NULL, FALSE);
      }
      else
        gxInvalidateRect(hwnd, &infoPtr->items[infoPtr->iMoveItem].rect, FALSE);

      HEADER_SetHotDivider(hwnd, FALSE, -1);
    }
    else if (!(dwStyle&HDS_DRAGDROP) || !HEADER_IsDragDistance(infoPtr, &pt))
    {
      infoPtr->items[infoPtr->iMoveItem].bDown = FALSE;
      hdc = gxGetDC (hwnd);
      HEADER_RefreshItem (hwnd, infoPtr->iMoveItem);
      gxReleaseDC (hwnd, hdc);

      HEADER_SendNotifyWithHDItemT(hwnd, HDN_ITEMCLICKW, infoPtr->iMoveItem, NULL);
    }

    TRACE("Released item %d!\n", infoPtr->iMoveItem);
    infoPtr->bPressed = FALSE;
  }
  else if (infoPtr->bTracking) {
    GXINT iNewWidth = pt.x - infoPtr->items[infoPtr->iMoveItem].rect.left + infoPtr->xTrackOffset;
    if (iNewWidth < 0)
      iNewWidth = 0;
    TRACE("End tracking item %d!\n", infoPtr->iMoveItem);
    infoPtr->bTracking = FALSE;

    HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ENDTRACKW, infoPtr->iMoveItem, HDI_WIDTH, iNewWidth);

    if (!(dwStyle & HDS_FULLDRAG)) {
      hdc = gxGetDC (hwnd);
      HEADER_DrawTrackLine (hwnd, hdc, infoPtr->xOldTrack);
      gxReleaseDC (hwnd, hdc);
    }

    if (!HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ITEMCHANGINGW, infoPtr->iMoveItem, HDI_WIDTH, iNewWidth))
    {
      infoPtr->items[infoPtr->iMoveItem].cxy = iNewWidth;
      HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ITEMCHANGEDW, infoPtr->iMoveItem, HDI_WIDTH, iNewWidth);
    }

    HEADER_SetItemBounds (hwnd);
    gxInvalidateRect(hwnd, NULL, TRUE);
  }

  if (infoPtr->bCaptured) {
    infoPtr->bCaptured = FALSE;
    gxReleaseCapture ();
    HEADER_SendSimpleNotify (hwnd, GXNM_RELEASEDCAPTURE);
  }

  return 0;
}


static GXLRESULT
HEADER_NotifyFormat (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);

  switch (lParam)
  {
  case GXNF_QUERY:
    return infoPtr->nNotifyFormat;

  case GXNF_REQUERY:
    infoPtr->nNotifyFormat =
      gxSendMessageW ((GXHWND)wParam, GXWM_NOTIFYFORMAT,
      (GXWPARAM)hwnd, (GXLPARAM)GXNF_QUERY);
    return infoPtr->nNotifyFormat;
  }

  return 0;
}

static GXLRESULT
HEADER_MouseLeave (GXHWND hwnd)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  /* Reset hot-tracked item when mouse leaves control. */
  GXINT oldHotItem = infoPtr->iHotItem;
  GXHDC hdc = gxGetDC (hwnd);

  infoPtr->iHotItem = -1;
  if (oldHotItem != -1) HEADER_RefreshItem (hwnd, oldHotItem);
  gxReleaseDC (hwnd, hdc);

  return 0;
}


static GXLRESULT
HEADER_MouseMove (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXDWORD dwStyle = gxGetWindowLongW (hwnd, GXGWL_STYLE);
  GXPOINT pt;
  GXUINT  flags;
  GXINT   nItem, nWidth;
  GXHDC   hdc;
  /* With theming, hottracking is always enabled */
  GXBOOL  hotTrackEnabled =
    ((dwStyle & HDS_BUTTONS) && (dwStyle & HDS_HOTTRACK))
    || (gxGetWindowTheme (hwnd) != NULL);
  GXINT oldHotItem = infoPtr->iHotItem;

  pt.x = (GXINT)(GXSHORT)GXLOWORD(lParam);
  pt.y = (GXINT)(GXSHORT)GXHIWORD(lParam);
  HEADER_InternalHitTest (hwnd, &pt, &flags, &nItem);

  if (hotTrackEnabled) {
    if (flags & (HHT_ONHEADER | HHT_ONDIVIDER | HHT_ONDIVOPEN))
      infoPtr->iHotItem = nItem;
    else
      infoPtr->iHotItem = -1;
  }

  if (infoPtr->bCaptured) {
    /* check if we should drag the header */
    if (infoPtr->bPressed && !infoPtr->bDragging && dwStyle&HDS_DRAGDROP
      && HEADER_IsDragDistance(infoPtr, &pt))
    {
      if (!HEADER_SendNotifyWithHDItemT(hwnd, HDN_BEGINDRAG, infoPtr->iMoveItem, NULL))
      {
        GXHIMAGELIST hDragItem = (GXHIMAGELIST)HEADER_CreateDragImage(hwnd, infoPtr->iMoveItem);
        if (hDragItem != NULL)
        {
          HEADER_ITEM *lpItem = &infoPtr->items[infoPtr->iMoveItem];
          TRACE("Starting item drag\n");
          gxImageList_BeginDrag(hDragItem, 0, pt.x - lpItem->rect.left, 0);
          gxImageList_DragShowNolock(TRUE);
          gxImageList_Destroy(hDragItem);
          infoPtr->bDragging = TRUE;
        }
      }
    }

    if (infoPtr->bDragging)
    {
      GXPOINT drag;
      drag.x = pt.x;
      drag.y = 0;
      gxClientToScreen(hwnd, &drag);
      gxImageList_DragMove(drag.x, drag.y);
      HEADER_SetHotDivider(hwnd, TRUE, lParam);
    }

    if (infoPtr->bPressed && !infoPtr->bDragging) {
      GXBOOL oldState = infoPtr->items[infoPtr->iMoveItem].bDown;
      if ((nItem == infoPtr->iMoveItem) && (flags == HHT_ONHEADER))
        infoPtr->items[infoPtr->iMoveItem].bDown = TRUE;
      else
        infoPtr->items[infoPtr->iMoveItem].bDown = FALSE;
      if (oldState != infoPtr->items[infoPtr->iMoveItem].bDown) {
        hdc = gxGetDC (hwnd);
        HEADER_RefreshItem (hwnd, infoPtr->iMoveItem);
        gxReleaseDC (hwnd, hdc);
      }

      TRACE("Moving pressed item %d!\n", infoPtr->iMoveItem);
    }
    else if (infoPtr->bTracking) {
      if (dwStyle & HDS_FULLDRAG) {
        HEADER_ITEM *lpItem = &infoPtr->items[infoPtr->iMoveItem];
        nWidth = pt.x - lpItem->rect.left + infoPtr->xTrackOffset;
        if (!HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ITEMCHANGINGW, infoPtr->iMoveItem, HDI_WIDTH, nWidth))
        {
          GXINT nOldWidth = lpItem->rect.right - lpItem->rect.left;
          GXRECT rcClient;
          GXRECT rcScroll;

          if (nWidth < 0) nWidth = 0;
          infoPtr->items[infoPtr->iMoveItem].cxy = nWidth;
          HEADER_SetItemBounds(hwnd);

          gxGetClientRect(hwnd, &rcClient);
          rcScroll = rcClient;
          rcScroll.left = lpItem->rect.left + nOldWidth;
          gxScrollWindowEx(hwnd, nWidth - nOldWidth, 0, &rcScroll, &rcClient, NULL, NULL, 0);
          gxInvalidateRect(hwnd, &lpItem->rect, FALSE);
          gxUpdateWindow(hwnd);

          HEADER_SendNotifyWithIntFieldT(hwnd, HDN_ITEMCHANGEDW, infoPtr->iMoveItem, HDI_WIDTH, nWidth);
        }
      }
      else {
        GXINT iTrackWidth;
        hdc = gxGetDC (hwnd);
        HEADER_DrawTrackLine (hwnd, hdc, infoPtr->xOldTrack);
        infoPtr->xOldTrack = pt.x + infoPtr->xTrackOffset;
        if (infoPtr->xOldTrack < infoPtr->items[infoPtr->iMoveItem].rect.left)
          infoPtr->xOldTrack = infoPtr->items[infoPtr->iMoveItem].rect.left;
        HEADER_DrawTrackLine (hwnd, hdc, infoPtr->xOldTrack);
        gxReleaseDC (hwnd, hdc);
        iTrackWidth = infoPtr->xOldTrack - infoPtr->items[infoPtr->iMoveItem].rect.left;
        /* FIXME: should stop tracking if HDN_TRACK returns TRUE */
        HEADER_SendNotifyWithIntFieldT(hwnd, HDN_TRACKW, infoPtr->iMoveItem, HDI_WIDTH, iTrackWidth);
      }

      TRACE("Tracking item %d!\n", infoPtr->iMoveItem);
    }
  }

  if (hotTrackEnabled) {
    GXTRACKMOUSEEVENT tme;
    if (oldHotItem != infoPtr->iHotItem && !infoPtr->bDragging) {
      hdc = gxGetDC (hwnd);
      if (oldHotItem != -1) HEADER_RefreshItem (hwnd, oldHotItem);
      if (infoPtr->iHotItem != -1) HEADER_RefreshItem (hwnd, infoPtr->iHotItem);
      gxReleaseDC (hwnd, hdc);
    }
    tme.cbSize = sizeof( tme );
    tme.dwFlags = GXTME_LEAVE;
    tme.hwndTrack = hwnd;
    gxTrackMouseEvent( &tme );
  }

  return 0;
}


static GXLRESULT
HEADER_Paint (GXHWND hwnd, GXWPARAM wParam)
{
  GXHDC hdc;
  GXPAINTSTRUCT ps;

  hdc = wParam==0 ? gxBeginPaint (hwnd, &ps) : (GXHDC)wParam;
  HEADER_Refresh (hwnd, hdc);
  if(!wParam)
    gxEndPaint (hwnd, &ps);
  return 0;
}


static GXLRESULT
HEADER_RButtonUp (GXHWND hwnd, GXLPARAM lParam)
{
  GXBOOL bRet;
  GXPOINT pt;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);

  /* Send a Notify message */
  bRet = HEADER_SendSimpleNotify (hwnd, GXNM_RCLICK);

  /* Change to screen coordinate for WM_CONTEXTMENU */
  gxClientToScreen(hwnd, &pt);

  /* Send a WM_CONTEXTMENU message in response to the RBUTTONUP */
  gxSendMessageW( hwnd, GXWM_CONTEXTMENU, (GXWPARAM) hwnd, GXMAKELPARAM(pt.x, pt.y));

  return bRet;
}


static GXLRESULT
HEADER_SetCursor (GXHWND hwnd, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXPOINT pt;
  GXUINT  flags;
  GXINT   nItem;

  TRACE("code=0x%X  id=0x%X\n", GXLOWORD(lParam), GXHIWORD(lParam));

  gxGetCursorPos (&pt);
  gxScreenToClient (hwnd, &pt);

  HEADER_InternalHitTest (hwnd, &pt, &flags, &nItem);

  if (flags == HHT_ONDIVIDER)
    gxSetCursor (infoPtr->hcurDivider);
  else if (flags == HHT_ONDIVOPEN)
    gxSetCursor (infoPtr->hcurDivopen);
  else
    gxSetCursor (infoPtr->hcurArrow);

  return 0;
}


static GXLRESULT
HEADER_SetFont (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  HEADER_INFO *infoPtr = HEADER_GetInfoPtr (hwnd);
  GXTEXTMETRICW tm;
  GXHFONT hFont, hOldFont;
  GXHDC hdc;

  infoPtr->hFont = (GXHFONT)wParam;

  hFont = infoPtr->hFont ? infoPtr->hFont : (GXHFONT)gxGetStockObject (GXSYSTEM_FONT);

  hdc = gxGetDC (0);
  hOldFont = (GXHFONT)gxSelectObject (hdc, hFont);
  gxGetTextMetricsW (hdc, &tm);
  infoPtr->nHeight = tm.tmHeight + VERT_BORDER;
  gxSelectObject (hdc, hOldFont);
  gxReleaseDC (0, hdc);

  infoPtr->bRectsValid = FALSE;

  if (lParam) {
    gxInvalidateRect(hwnd, NULL, FALSE);
  }

  return 0;
}

static GXLRESULT HEADER_SetRedraw(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  /* ignoring the gxInvalidateRect calls is handled by user32. But some apps expect
  * that we invalidate the header and this has to be done manually  */
  GXLRESULT ret;

  ret = gxDefWindowProcW(hwnd, GXWM_SETREDRAW, wParam, lParam);
  if (wParam)
    gxInvalidateRect(hwnd, NULL, TRUE);
  return ret;
}

/* Update the theme handle after a theme change */
static GXLRESULT HEADER_ThemeChanged(GXHWND hwnd)
{
  GXHTHEME theme = gxGetWindowTheme(hwnd);
  gxCloseThemeData(theme);
  gxOpenThemeData(hwnd, themeClass);
  gxInvalidateRect(hwnd, NULL, FALSE);
  return 0;
}


static GXLRESULT GXCALLBACK HEADER_WindowProc (GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACEW(_CLTEXT("hwnd=%p msg=%x wparam=%lx lParam=%lx\n"), hwnd, msg, wParam, lParam);
  if (!HEADER_GetInfoPtr (hwnd) && (msg != GXWM_CREATE))
    return gxDefWindowProcW (hwnd, msg, wParam, lParam);
  switch (msg) {
    /*  case HDM_CLEARFILTER: */

  case GXHDM_CREATEDRAGIMAGE:
    return HEADER_CreateDragImage (hwnd, wParam);

  case GXHDM_DELETEITEM:
    return HEADER_DeleteItem (hwnd, wParam);

    /*  case GXHDM_EDITFILTER: */

  case GXHDM_GETBITMAPMARGIN:
    return HEADER_GetBitmapMargin(hwnd);

  case GXHDM_GETIMAGELIST:
    return HEADER_GetImageList (hwnd);

  case GXHDM_GETITEMA:
  case GXHDM_GETITEMW:
    return HEADER_GetItemT (hwnd, (GXINT)wParam, (GXLPHDITEMW)lParam, msg == GXHDM_GETITEMW);

  case GXHDM_GETITEMCOUNT:
    return HEADER_GetItemCount (hwnd);

  case GXHDM_GETITEMRECT:
    return HEADER_GetItemRect (hwnd, wParam, lParam);

  case GXHDM_GETORDERARRAY:
    return HEADER_GetOrderArray(hwnd, wParam, lParam);

  case GXHDM_GETUNICODEFORMAT:
    return HEADER_GetUnicodeFormat (hwnd);

  case GXHDM_HITTEST:
    return HEADER_HitTest (hwnd, lParam);

  case GXHDM_INSERTITEMA:
  case GXHDM_INSERTITEMW:
    return HEADER_InsertItemT (hwnd, (GXINT)wParam, (GXLPHDITEMW)lParam, msg == GXHDM_INSERTITEMW);

  case GXHDM_LAYOUT:
    return HEADER_Layout (hwnd, lParam);

  case GXHDM_ORDERTOINDEX:
    return HEADER_OrderToIndex(hwnd, wParam);

  case GXHDM_SETBITMAPMARGIN:
    return HEADER_SetBitmapMargin(hwnd, wParam);

    /*  case GXHDM_SETFILTERCHANGETIMEOUT: */

  case GXHDM_SETHOTDIVIDER:
    return HEADER_SetHotDivider(hwnd, wParam, lParam);

  case GXHDM_SETIMAGELIST:
    return HEADER_SetImageList (hwnd, (GXHIMAGELIST)lParam);

  case GXHDM_SETITEMA:
  case GXHDM_SETITEMW:
    return HEADER_SetItemT (hwnd, (GXINT)wParam, (GXLPHDITEMW)lParam, msg == GXHDM_SETITEMW);

  case GXHDM_SETORDERARRAY:
    return HEADER_SetOrderArray(hwnd, wParam, lParam);

  case GXHDM_SETUNICODEFORMAT:
    return HEADER_SetUnicodeFormat (hwnd, wParam);

  case GXWM_CREATE:
    return HEADER_Create (hwnd, lParam);

  case GXWM_DESTROY:
    return HEADER_Destroy (hwnd);

  case GXWM_NCDESTROY:
    return HEADER_NCDestroy (hwnd);

  case GXWM_ERASEBKGND:
    return 1;

  case GXWM_GETDLGCODE:
    return GXDLGC_WANTTAB | GXDLGC_WANTARROWS;

  case GXWM_GETFONT:
    return HEADER_GetFont (hwnd);

  case GXWM_LBUTTONDBLCLK:
    return HEADER_LButtonDblClk (hwnd, lParam);

  case GXWM_LBUTTONDOWN:
    return HEADER_LButtonDown (hwnd, lParam);

  case GXWM_LBUTTONUP:
    return HEADER_LButtonUp (hwnd, lParam);

  case GXWM_MOUSELEAVE:
    return HEADER_MouseLeave (hwnd);

  case GXWM_MOUSEMOVE:
    return HEADER_MouseMove (hwnd, lParam);

  case GXWM_NOTIFYFORMAT:
    return HEADER_NotifyFormat (hwnd, wParam, lParam);

  case GXWM_SIZE:
    return HEADER_Size (hwnd);

  case GXWM_THEMECHANGED:
    return HEADER_ThemeChanged (hwnd);

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    return HEADER_Paint (hwnd, wParam);

  case GXWM_RBUTTONUP:
    return HEADER_RButtonUp (hwnd, lParam);

  case GXWM_SETCURSOR:
    return HEADER_SetCursor (hwnd, lParam);

  case GXWM_SETFONT:
    return HEADER_SetFont (hwnd, wParam, lParam);

  case GXWM_SETREDRAW:
    return HEADER_SetRedraw(hwnd, wParam, lParam);

  default:
    if ((msg >= GXWM_USER) && (msg < GXWM_APP) && !gxCOMCTL32_IsReflectedMessage(msg))
      ERR("unknown msg %04x wp=%04lx lp=%08lx\n",
      msg, wParam, lParam );
    return gxDefWindowProcW(hwnd, msg, wParam, lParam);
  }
}


GXVOID
HEADER_Register ()
{
  GXWNDCLASSEX wndClass;

  ZeroMemory (&wndClass, sizeof(GXWNDCLASSEX));
  wndClass.style         = GXCS_GLOBALCLASS | GXCS_DBLCLKS;
  wndClass.lpfnWndProc   = HEADER_WindowProc;
  wndClass.cbClsExtra    = 0;
  wndClass.cbWndExtra    = sizeof(HEADER_INFO *);
  wndClass.hCursor       = gxLoadCursorW (0, (GXLPWSTR)GXIDC_ARROW);
  wndClass.lpszClassName = GXWC_HEADERW;

  gxRegisterClassExW (&wndClass);
}


GXVOID
HEADER_Unregister ()
{
  gxUnregisterClassW (GXWC_HEADERW, NULL);
}
#endif // _DEV_DISABLE_UI_CODE