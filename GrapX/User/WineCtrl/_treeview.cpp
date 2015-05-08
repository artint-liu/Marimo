#ifndef _DEV_DISABLE_UI_CODE
/* Treeview control
*
* Copyright 1998 Eric Kohl <ekohl@abo.rhein-zeitung.de>
* Copyright 1998,1999 Alex Priem <alexp@sci.kun.nl>
* Copyright 1999 Sylvain St-Germain
* Copyright 2002 CodeWeavers, Aric Stewart
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
* Note that TREEVIEW_INFO * and GXHTREEITEM are the same thing.
*
* Note2: All items always! have valid (allocated) pszText field.
*      If item's text == GXLPSTR_TEXTCALLBACKA we allocate buffer
*      of size TEXT_CALLBACK_SIZE in DoSetItem.
*      We use callbackMask to keep track of fields to be updated.
*
* TODO:
*   missing notifications: NM_SETCURSOR, TVN_GETINFOTIP, TVN_KEYDOWN,
*      TVN_SETDISPINFO, TVN_SINGLEEXPAND
*
*   missing styles: TVS_FULLROWSELECT, TVS_INFOTIP, TVS_RTLREADING,
*
*   missing item styles: TVIS_CUT, TVIS_EXPANDPARTIAL
*
*   Make the insertion mark look right.
*   Scroll (instead of repaint) as much as possible.
*/

//#include "config.h"
//#include "wine/port.h"
//
//#include <assert.h>
//#include <ctype.h>
//#include <stdarg.h>
//#include <string.h>
//#include <limits.h>
//#include <stdlib.h>

#define NONAMELESSUNION
#define NONAMELESSSTRUCT
//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "winnls.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "uxtheme.h"
//#include "tmschema.h"
//#include "wine/unicode.h"
//#include "wine/debug.h"

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
#include <User/Win32Emu/_dpa.H>

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

/* internal structures */

typedef struct _GXTREEITEM *GXHTREEITEM;
typedef struct _GXTREEITEM    /* GXHTREEITEM is a _TREEINFO *. */
{
  GXUINT      callbackMask;
  GXUINT      state;
  GXUINT      stateMask;
  GXLPWSTR    pszText;
  int       cchTextMax;
  int       iImage;
  int       iSelectedImage;
  int       cChildren;
  GXLPARAM    lParam;
  int       iIntegral;      /* item height multiplier (1 is normal) */
  int       iLevel;         /* indentation level:0=root level */
  GXHTREEITEM parent;         /* handle to parent or 0 if at root */
  GXHTREEITEM firstChild;     /* handle to first child or 0 if no child */
  GXHTREEITEM lastChild;
  GXHTREEITEM prevSibling;    /* handle to prev item in list, 0 if first */
  GXHTREEITEM nextSibling;    /* handle to next item in list, 0 if last */
  GXRECT      rect;
  GXLONG      linesOffset;
  GXLONG      stateOffset;
  GXLONG      imageOffset;
  GXLONG      textOffset;
  GXLONG      textWidth;      /* horizontal text extent for pszText */
  GXLONG      visibleOrder;   /* visible ordering, 0 is first visible item */
} GXTREEVIEW_ITEM;

typedef struct tagGXTREEVIEW_INFO
{
  GXHWND          hwnd;
  GXHWND          hwndNotify;     /* Owner window to send notifications to */
  GXDWORD         dwStyle;
  GXHTREEITEM     root;
  GXUINT          uInternalStatus;
  GXINT           Timer;
  GXUINT          uNumItems;      /* number of valid TREEVIEW_ITEMs */
  GXINT           cdmode;         /* last custom draw setting */
  GXUINT          uScrollTime;  /* max. time for scrolling in milliseconds */
  GXBOOL          bRedraw;        /* if FALSE we validate but don't redraw in TREEVIEW_Paint() */

  GXUINT          uItemHeight;    /* item height */
  GXBOOL          bHeightSet;

  GXLONG          clientWidth;    /* width of control window */
  GXLONG          clientHeight;   /* height of control window */

  GXLONG          treeWidth;      /* width of visible tree items */
  GXLONG          treeHeight;     /* height of visible tree items */

  GXUINT          uIndent;        /* indentation in pixels */
  GXHTREEITEM     selectedItem;   /* handle to selected item or 0 if none */
  GXHTREEITEM     hotItem;        /* handle currently under cursor, 0 if none */
  GXHTREEITEM  focusedItem;    /* item that was under the cursor when WM_LBUTTONDOWN was received */

  GXHTREEITEM     firstVisible;   /* handle to first visible item */
  GXLONG          maxVisibleOrder;
  GXHTREEITEM     dropItem;       /* handle to item selected by drag cursor */
  GXHTREEITEM     insertMarkItem; /* item after which insertion mark is placed */
  GXBOOL          insertBeforeorAfter; /* flag used by GXTVM_SETINSERTMARK */
  GXHIMAGELIST    dragList;       /* Bitmap of dragged item */
  GXLONG          scrollX;
  GXCOLORREF      clrBk;
  GXCOLORREF      clrText;
  GXCOLORREF      clrLine;
  GXCOLORREF      clrInsertMark;
  GXHFONT         hFont;
  GXHFONT         hDefaultFont;
  GXHFONT         hBoldFont;
  GXHFONT         hUnderlineFont;
  GXHCURSOR       hcurHand;
  GXHWND          hwndToolTip;

  GXHWND          hwndEdit;
  GXWNDPROC       wpEditOrig;     /* orig window proc for subclassing edit */
  GXBOOL          bIgnoreEditKillFocus;
  GXBOOL          bLabelChanged;

  GXBOOL          bNtfUnicode;    /* TRUE if should send NOTIFY with W */
  GXHIMAGELIST    himlNormal;
  int           normalImageHeight;
  int           normalImageWidth;
  GXHIMAGELIST    himlState;
  int           stateImageHeight;
  int           stateImageWidth;
  GXHDPA          items;

  GXDWORD lastKeyPressTimestamp; /* Added */
  GXWPARAM charCode; /* Added */
  GXINT nSearchParamLength; /* Added */
  GXWCHAR szSearchParam[ MAX_PATH ]; /* Added */
} GXTREEVIEW_INFO;


/******** Defines that TREEVIEW_ProcessLetterKeys uses ****************/
#define KEY_DELAY       450

/* bitflags for infoPtr->uInternalStatus */

#define GXTV_HSCROLL   0x01    /* treeview too large to fit in window */
#define GXTV_VSCROLL   0x02  /* (horizontal/vertical) */
#define GXTV_LDRAG    0x04  /* Lbutton pushed to start drag */
#define GXTV_LDRAGGING  0x08  /* Lbutton pushed, mouse moved. */
#define GXTV_RDRAG    0x10  /* dito Rbutton */
#define GXTV_RDRAGGING  0x20

/* bitflags for infoPtr->timer */

#define GXTV_EDIT_TIMER    2
#define GXTV_EDIT_TIMER_SET 2


GXVOID TREEVIEW_Register ();
GXVOID TREEVIEW_Unregister ();


//WINE_DEFAULT_DEBUG_CHANNEL(treeview);


#define TEXT_CALLBACK_SIZE 260

#define TREEVIEW_LEFT_MARGIN 8

#define MINIMUM_INDENT 19

#define CALLBACK_MASK_ALL (GXTVIF_TEXT|GXTVIF_CHILDREN|GXTVIF_IMAGE|GXTVIF_SELECTEDIMAGE)

#define STATEIMAGEINDEX(x) (((x) >> 12) & 0x0f)
#define OVERLAYIMAGEINDEX(x) (((x) >> 8) & 0x0f)
#define ISVISIBLE(x)         ((x)->visibleOrder >= 0)


static const GXWCHAR themeClass[] = { 'T','r','e','e','v','i','e','w',0 };


typedef GXVOID (*TREEVIEW_ItemEnumFunc)(GXTREEVIEW_INFO *, GXTREEVIEW_ITEM *,GXLPVOID);

//static inline int lstrncmpiW(GXLPCWSTR s1, GXLPCWSTR s2, int n);
static GXVOID TREEVIEW_Invalidate(const GXTREEVIEW_INFO *, const GXTREEVIEW_ITEM *);

static GXLRESULT TREEVIEW_DoSelectItem(GXTREEVIEW_INFO *, GXINT, GXHTREEITEM, GXINT);
static GXVOID TREEVIEW_SetFirstVisible(GXTREEVIEW_INFO *, GXTREEVIEW_ITEM *, GXBOOL);
static GXLRESULT TREEVIEW_EnsureVisible(GXTREEVIEW_INFO *, GXHTREEITEM, GXBOOL);
static GXLRESULT TREEVIEW_RButtonUp(const GXTREEVIEW_INFO *, const GXPOINT *);
static GXLRESULT TREEVIEW_EndEditLabelNow(GXTREEVIEW_INFO *infoPtr, GXBOOL bCancel);
static GXVOID TREEVIEW_UpdateScrollBars(GXTREEVIEW_INFO *infoPtr);
static GXLRESULT TREEVIEW_HScroll(GXTREEVIEW_INFO *, GXWPARAM);
static GXINT TREEVIEW_NotifyFormat (GXTREEVIEW_INFO *infoPtr, GXHWND wParam, GXUINT lParam);


/* Random Utilities *****************************************************/

#ifndef NDEBUG
static inline void
TREEVIEW_VerifyTree(GXTREEVIEW_INFO *infoPtr)
{
  (void)infoPtr;
}
#else
/* The definition is at the end of the file. */
static void TREEVIEW_VerifyTree(GXTREEVIEW_INFO *infoPtr);
#endif

/* Returns the treeview private data if hwnd is a treeview.
* Otherwise returns an undefined value. */
static GXTREEVIEW_INFO *
TREEVIEW_GetInfoPtr(GXHWND hwnd)
{
  return (GXTREEVIEW_INFO *)gxGetWindowLongPtrW(hwnd, 0);
}

/* Don't call this. Nothing wants an item index. */
static inline int
TREEVIEW_GetItemIndex(const GXTREEVIEW_INFO *infoPtr, GXHTREEITEM handle)
{
  assert(infoPtr != NULL);

  return gxDPA_GetPtrIndex(infoPtr->items, handle);
}

/* Checks if item has changed and needs to be redrawn */
static inline GXBOOL item_changed (const GXTREEVIEW_ITEM *tiOld, const GXTREEVIEW_ITEM *tiNew,
                   const GXTVITEMEXW *tvChange)
{
  /* Number of children has changed */
  if ((tvChange->mask & GXTVIF_CHILDREN) && (tiOld->cChildren != tiNew->cChildren))
    return TRUE;

  /* Image has changed and it's not a callback */
  if ((tvChange->mask & GXTVIF_IMAGE) && (tiOld->iImage != tiNew->iImage) &&
    tiNew->iImage != GXI_IMAGECALLBACK)
    return TRUE;

  /* Selected image has changed and it's not a callback */
  if ((tvChange->mask & GXTVIF_SELECTEDIMAGE) && (tiOld->iSelectedImage != tiNew->iSelectedImage) &&
    tiNew->iSelectedImage != GXI_IMAGECALLBACK)
    return TRUE;

  /* Text has changed and it's not a callback */
  if ((tvChange->mask & GXTVIF_TEXT) && (tiOld->pszText != tiNew->pszText) &&
    tiNew->pszText != GXLPSTR_TEXTCALLBACKW)
    return TRUE;

  /* Indent has changed */
  if ((tvChange->mask & GXTVIF_INTEGRAL) && (tiOld->iIntegral != tiNew->iIntegral))
    return TRUE;

  /* Item state has changed */
  if ((tvChange->mask & GXTVIF_STATE) && ((tiOld->state ^ tiNew->state) & tvChange->stateMask ))
    return TRUE;

  return FALSE;
}

/***************************************************************************
* This method checks that handle is an item for this tree.
*/
static GXBOOL
TREEVIEW_ValidItem(const GXTREEVIEW_INFO *infoPtr, GXHTREEITEM handle)
{
  if (TREEVIEW_GetItemIndex(infoPtr, handle) == -1)
  {
    TRACE("invalid item %p\n", handle);
    return FALSE;
  }
  else
    return TRUE;
}

static GXHFONT
TREEVIEW_CreateBoldFont(GXHFONT hOrigFont)
{
  GXLOGFONTW font;

  gxGetObjectW(hOrigFont, sizeof(font), &font);
  font.lfWeight = GXFW_BOLD;
  return gxCreateFontIndirectW(&font);
}

static GXHFONT
TREEVIEW_CreateUnderlineFont(GXHFONT hOrigFont)
{
  GXLOGFONTW font;

  gxGetObjectW(hOrigFont, sizeof(font), &font);
  font.lfUnderline = TRUE;
  return gxCreateFontIndirectW(&font);
}

static inline GXHFONT
TREEVIEW_FontForItem(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *item)
{
  if ((infoPtr->dwStyle & TVS_TRACKSELECT) && (item == infoPtr->hotItem))
    return infoPtr->hUnderlineFont;
  if (item->state & TVIS_BOLD)
    return infoPtr->hBoldFont;
  return infoPtr->hFont;
}

/* for trace/debugging purposes only */
static const char *
TREEVIEW_ItemName(const GXTREEVIEW_ITEM *item)
{
  if (item == NULL) return "<null item>";
  if (item->pszText == GXLPSTR_TEXTCALLBACKW) return "<callback>";
  if (item->pszText == NULL) return "<null>";
  return (const char *)debugstr_w(item->pszText);
}

/* An item is not a child of itself. */
static GXBOOL
TREEVIEW_IsChildOf(const GXTREEVIEW_ITEM *parent, const GXTREEVIEW_ITEM *child)
{
  do
  {
    child = child->parent;
    if (child == parent) return TRUE;
  } while (child != NULL);

  return FALSE;
}


/* Tree Traversal *******************************************************/

/***************************************************************************
* This method returns the last expanded sibling or child child item
* of a tree node
*/
static GXTREEVIEW_ITEM *
TREEVIEW_GetLastListItem(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem)
{
  if (!wineItem)
    return NULL;

  while (wineItem->lastChild)
  {
    if (wineItem->state & TVIS_EXPANDED)
      wineItem = wineItem->lastChild;
    else
      break;
  }

  if (wineItem == infoPtr->root)
    return NULL;

  return wineItem;
}

/***************************************************************************
* This method returns the previous non-hidden item in the list not
* considering the tree hierarchy.
*/
static GXTREEVIEW_ITEM *
TREEVIEW_GetPrevListItem(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *tvItem)
{
  if (tvItem->prevSibling)
  {
    /* This item has a prevSibling, get the last item in the sibling's tree. */
    GXTREEVIEW_ITEM *upItem = tvItem->prevSibling;

    if ((upItem->state & TVIS_EXPANDED) && upItem->lastChild != NULL)
      return TREEVIEW_GetLastListItem(infoPtr, upItem->lastChild);
    else
      return upItem;
  }
  else
  {
    /* this item does not have a prevSibling, get the parent */
    return (tvItem->parent != infoPtr->root) ? tvItem->parent : NULL;
  }
}


/***************************************************************************
* This method returns the next physical item in the treeview not
* considering the tree hierarchy.
*/
static GXTREEVIEW_ITEM *
TREEVIEW_GetNextListItem(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *tvItem)
{
  assert(tvItem != NULL);

  /*
  * If this item has children and is expanded, return the first child
  */
  if ((tvItem->state & TVIS_EXPANDED) && tvItem->firstChild != NULL)
  {
    return tvItem->firstChild;
  }


  /*
  * try to get the sibling
  */
  if (tvItem->nextSibling)
    return tvItem->nextSibling;

  /*
  * Otherwise, get the parent's sibling.
  */
  while (tvItem->parent)
  {
    tvItem = tvItem->parent;

    if (tvItem->nextSibling)
      return tvItem->nextSibling;
  }

  return NULL;
}

/***************************************************************************
* This method returns the nth item starting at the given item.  It returns
* the last item (or first) we we run out of items.
*
* Will scroll backward if count is <0.
*             forward if count is >0.
*/
static GXTREEVIEW_ITEM *
TREEVIEW_GetListItem(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
           GXLONG count)
{
  GXTREEVIEW_ITEM *(*next_item)(const GXTREEVIEW_INFO *, const GXTREEVIEW_ITEM *);
  GXTREEVIEW_ITEM *previousItem;

  assert(wineItem != NULL);

  if (count > 0)
  {
    next_item = TREEVIEW_GetNextListItem;
  }
  else if (count < 0)
  {
    count = -count;
    next_item = TREEVIEW_GetPrevListItem;
  }
  else
    return wineItem;

  do
  {
    previousItem = wineItem;
    wineItem = next_item(infoPtr, wineItem);

  } while (--count && wineItem != NULL);


  return wineItem ? wineItem : previousItem;
}

/* Notifications ************************************************************/

static GXINT get_notifycode(const GXTREEVIEW_INFO *infoPtr, GXINT code)
{
  if (!infoPtr->bNtfUnicode) {
    switch (code) {
  case GXTVN_SELCHANGINGW:    return GXTVN_SELCHANGINGA;
  case GXTVN_SELCHANGEDW:    return GXTVN_SELCHANGEDA;
  case GXTVN_GETDISPINFOW:    return GXTVN_GETDISPINFOA;
  case GXTVN_SETDISPINFOW:    return GXTVN_SETDISPINFOA;
  case GXTVN_ITEMEXPANDINGW:  return GXTVN_ITEMEXPANDINGA;
  case GXTVN_ITEMEXPANDEDW:    return GXTVN_ITEMEXPANDEDA;
  case GXTVN_BEGINDRAGW:    return GXTVN_BEGINDRAGA;
  case GXTVN_BEGINRDRAGW:    return GXTVN_BEGINRDRAGA;
  case GXTVN_DELETEITEMW:    return GXTVN_DELETEITEMA;
  case GXTVN_BEGINLABELEDITW: return GXTVN_BEGINLABELEDITA;
  case GXTVN_ENDLABELEDITW:    return GXTVN_ENDLABELEDITA;
  case GXTVN_GETINFOTIPW:    return GXTVN_GETINFOTIPA;
    }
  }
  return code;
}

static GXLRESULT
TREEVIEW_SendRealNotify(const GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("wParam=%ld, lParam=%ld\n", wParam, lParam);
  return gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY, wParam, lParam);
}

static GXBOOL
TREEVIEW_SendSimpleNotify(const GXTREEVIEW_INFO *infoPtr, GXUINT code)
{
  GXNMHDR nmhdr;
  GXHWND hwnd = infoPtr->hwnd;

  TRACE("%d\n", code);
  nmhdr.hwndFrom = hwnd;
  nmhdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  nmhdr.code = get_notifycode(infoPtr, code);

  return (GXBOOL)TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)nmhdr.idFrom, (GXLPARAM)&nmhdr);
}

static GXVOID
TREEVIEW_TVItemFromItem(const GXTREEVIEW_INFO *infoPtr, GXUINT mask, GXTVITEMW *tvItem, GXTREEVIEW_ITEM *item)
{
  tvItem->mask = mask;
  tvItem->hItem = item;
  tvItem->state = item->state;
  tvItem->stateMask = 0;
  tvItem->iImage = item->iImage;
  tvItem->iSelectedImage = item->iSelectedImage;
  tvItem->cChildren = item->cChildren;
  tvItem->lParam = item->lParam;

  if(mask & GXTVIF_TEXT)
  {
    if (!infoPtr->bNtfUnicode)
    {
      tvItem->cchTextMax = gxWideCharToMultiByte( GXCP_ACP, 0, item->pszText, -1, NULL, 0, NULL, NULL );
      tvItem->pszText = (GXLPWSTR)Alloc (tvItem->cchTextMax);
      gxWideCharToMultiByte( GXCP_ACP, 0, item->pszText, -1, (GXLPSTR)tvItem->pszText, tvItem->cchTextMax, 0, 0 );
    }
    else
    {
      tvItem->cchTextMax = item->cchTextMax;
      tvItem->pszText = item->pszText;
    }
  }
  else
  {
    tvItem->cchTextMax = 0;
    tvItem->pszText = NULL;
  }
}

static GXBOOL
TREEVIEW_SendTreeviewNotify(const GXTREEVIEW_INFO *infoPtr, GXUINT code, GXUINT action,
              GXUINT mask, GXHTREEITEM oldItem, GXHTREEITEM newItem)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXNMTREEVIEWW nmhdr;
  GXBOOL ret;

  TRACE("code:%d action:%x olditem:%p newitem:%p\n",
    code, action, oldItem, newItem);

  ZeroMemory(&nmhdr, sizeof(GXNMTREEVIEWW));

  nmhdr.hdr.hwndFrom = hwnd;
  nmhdr.hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  nmhdr.hdr.code = get_notifycode(infoPtr, code);
  nmhdr.action = action;

  if (oldItem)
    TREEVIEW_TVItemFromItem(infoPtr, mask, &nmhdr.itemOld, oldItem);

  if (newItem)
    TREEVIEW_TVItemFromItem(infoPtr, mask, &nmhdr.itemNew, newItem);

  nmhdr.ptDrag.x = 0;
  nmhdr.ptDrag.y = 0;

  ret = (GXBOOL)TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)nmhdr.hdr.idFrom,
    (GXLPARAM)&nmhdr);
  if (!infoPtr->bNtfUnicode)
  {
    Free(nmhdr.itemOld.pszText);
    Free(nmhdr.itemNew.pszText);
  }
  return ret;
}

static GXBOOL
TREEVIEW_SendTreeviewDnDNotify(const GXTREEVIEW_INFO *infoPtr, GXUINT code,
                 GXHTREEITEM dragItem, GXPOINT pt)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXNMTREEVIEWW nmhdr;

  TRACE("code:%d dragitem:%p\n", code, dragItem);

  nmhdr.hdr.hwndFrom = hwnd;
  nmhdr.hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  nmhdr.hdr.code = get_notifycode(infoPtr, code);
  nmhdr.action = 0;
  nmhdr.itemNew.mask = GXTVIF_STATE | GXTVIF_PARAM | GXTVIF_HANDLE;
  nmhdr.itemNew.hItem = dragItem;
  nmhdr.itemNew.state = dragItem->state;
  nmhdr.itemNew.lParam = dragItem->lParam;

  nmhdr.ptDrag.x = pt.x;
  nmhdr.ptDrag.y = pt.y;

  return (GXBOOL)TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)nmhdr.hdr.idFrom,
    (GXLPARAM)&nmhdr);
}


static GXBOOL
TREEVIEW_SendCustomDrawNotify(const GXTREEVIEW_INFO *infoPtr, GXDWORD dwDrawStage,
                GXHDC hdc, GXRECT rc)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXNMTVCUSTOMDRAW nmcdhdr;
  GXLPNMCUSTOMDRAW nmcd;

  TRACE("drawstage:%x hdc:%p\n", dwDrawStage, hdc);

  nmcd = &nmcdhdr.nmcd;
  nmcd->hdr.hwndFrom = hwnd;
  nmcd->hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  nmcd->hdr.code = GXNM_CUSTOMDRAW;
  nmcd->dwDrawStage = dwDrawStage;
  nmcd->hdc = hdc;
  nmcd->rc = rc;
  nmcd->dwItemSpec = 0;
  nmcd->uItemState = 0;
  nmcd->lItemlParam = 0;
  nmcdhdr.clrText = infoPtr->clrText;
  nmcdhdr.clrTextBk = infoPtr->clrBk;
  nmcdhdr.iLevel = 0;

  return (GXBOOL)TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)nmcd->hdr.idFrom,
    (GXLPARAM)&nmcdhdr);
}



/* FIXME: need to find out when the flags in uItemState need to be set */

static GXBOOL
TREEVIEW_SendCustomDrawItemNotify(const GXTREEVIEW_INFO *infoPtr, GXHDC hdc,
                  GXTREEVIEW_ITEM *wineItem, GXUINT uItemDrawState,
                  GXNMTVCUSTOMDRAW *nmcdhdr)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXLPNMCUSTOMDRAW nmcd;
  GXDWORD dwDrawStage;
  GXDWORD_PTR dwItemSpec;
  GXUINT uItemState;
  GXINT retval;

  dwDrawStage = CDDS_ITEM | uItemDrawState;
  dwItemSpec = (GXDWORD_PTR)wineItem;
  uItemState = 0;
  if (wineItem->state & TVIS_SELECTED)
    uItemState |= CDIS_SELECTED;
  if (wineItem == infoPtr->selectedItem)
    uItemState |= CDIS_FOCUS;
  if (wineItem == infoPtr->hotItem)
    uItemState |= CDIS_HOT;

  nmcd = &nmcdhdr->nmcd;
  nmcd->hdr.hwndFrom = hwnd;
  nmcd->hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  nmcd->hdr.code = GXNM_CUSTOMDRAW;
  nmcd->dwDrawStage = dwDrawStage;
  nmcd->hdc = hdc;
  nmcd->rc = wineItem->rect;
  nmcd->dwItemSpec = dwItemSpec;
  nmcd->uItemState = uItemState;
  nmcd->lItemlParam = wineItem->lParam;
  nmcdhdr->iLevel = wineItem->iLevel;

  TRACE("drawstage:%x hdc:%p item:%lx, itemstate:%x, lItemlParam:%lx\n",
    nmcd->dwDrawStage, nmcd->hdc, nmcd->dwItemSpec,
    nmcd->uItemState, nmcd->lItemlParam);

  retval = TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)nmcd->hdr.idFrom,
    (GXLPARAM)nmcdhdr);

  return (GXBOOL)retval;
}

static GXBOOL
TREEVIEW_BeginLabelEditNotify(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *editItem)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXNMTVDISPINFOW tvdi;
  GXBOOL ret;

  tvdi.hdr.hwndFrom = hwnd;
  tvdi.hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  tvdi.hdr.code = get_notifycode(infoPtr, GXTVN_BEGINLABELEDITW);

  TREEVIEW_TVItemFromItem(infoPtr, GXTVIF_HANDLE | GXTVIF_STATE | GXTVIF_PARAM | GXTVIF_TEXT,
    &tvdi.item, editItem);

  ret = (GXBOOL)TREEVIEW_SendRealNotify(infoPtr, tvdi.hdr.idFrom, (GXLPARAM)&tvdi);

  if (!infoPtr->bNtfUnicode)
    Free(tvdi.item.pszText);

  return ret;
}

static void
TREEVIEW_UpdateDispInfo(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
            GXUINT mask)
{
  GXNMTVDISPINFOW callback;
  GXHWND hwnd = infoPtr->hwnd;

  TRACE("mask %x callbackMask %x\n", mask, wineItem->callbackMask);
  mask &= wineItem->callbackMask;

  if (mask == 0) return;

  callback.hdr.hwndFrom         = hwnd;
  callback.hdr.idFrom           = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  callback.hdr.code             = get_notifycode(infoPtr, GXTVN_GETDISPINFOW);

  /* 'state' always contains valid value, as well as 'lParam'.
  * All other parameters are uninitialized.
  */
  callback.item.pszText         = wineItem->pszText;
  callback.item.cchTextMax      = wineItem->cchTextMax;
  callback.item.mask            = mask;
  callback.item.hItem           = wineItem;
  callback.item.state           = wineItem->state;
  callback.item.lParam          = wineItem->lParam;

  /* If text is changed we need to recalculate textWidth */
  if (mask & GXTVIF_TEXT)
    wineItem->textWidth = 0;

  TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)callback.hdr.idFrom, (GXLPARAM)&callback);

  /* It may have changed due to a call to SetItem. */
  mask &= wineItem->callbackMask;

  if ((mask & GXTVIF_TEXT) && callback.item.pszText != wineItem->pszText)
  {
    /* Instead of copying text into our buffer user specified its own */
    if (!infoPtr->bNtfUnicode) {
      GXLPWSTR newText;
      int buflen;
      int len = gxMultiByteToWideChar( GXCP_ACP, 0,
        (GXLPSTR)callback.item.pszText, -1,
        NULL, 0);
      buflen = max((len)*sizeof(GXWCHAR), TEXT_CALLBACK_SIZE);
      newText = (GXLPWSTR)ReAlloc(wineItem->pszText, buflen);

      TRACE("returned str %s, len=%d, buflen=%d\n",
        debugstr_a((GXLPSTR)callback.item.pszText), len, buflen);

      if (newText)
      {
        wineItem->pszText = newText;
        gxMultiByteToWideChar( GXCP_ACP, 0,
          (GXLPSTR)callback.item.pszText, -1,
          wineItem->pszText, buflen/sizeof(GXWCHAR));
        wineItem->cchTextMax = buflen/sizeof(GXWCHAR);
      }
      /* If ReAlloc fails we have nothing to do, but keep original text */
    }
    else {
      int len = max(GXSTRLEN(callback.item.pszText) + 1,
        (int)TEXT_CALLBACK_SIZE);
      GXLPWSTR newText = (GXLPWSTR)ReAlloc(wineItem->pszText, len);

      TRACE("returned wstr %s, len=%d\n",
        debugstr_w(callback.item.pszText), len);

      if (newText)
      {
        wineItem->pszText = newText;
        GXSTRCPY(wineItem->pszText, callback.item.pszText);
        wineItem->cchTextMax = len;
      }
      /* If ReAlloc fails we have nothing to do, but keep original text */
    }
  }
  else if (mask & GXTVIF_TEXT) {
    /* User put text into our buffer, that is ok unless A string */
    if (!infoPtr->bNtfUnicode) {
      GXLPWSTR newText;
      GXLPWSTR oldText = NULL;
      int buflen;
      int len = gxMultiByteToWideChar( GXCP_ACP, 0,
        (GXLPSTR)callback.item.pszText, -1,
        NULL, 0);
      buflen = max((len)*sizeof(GXWCHAR), TEXT_CALLBACK_SIZE);
      newText = (GXLPWSTR)Alloc(buflen);

      TRACE("same buffer str %s, len=%d, buflen=%d\n",
        debugstr_a((GXLPSTR)callback.item.pszText), len, buflen);

      if (newText)
      {
        oldText = wineItem->pszText;
        wineItem->pszText = newText;
        gxMultiByteToWideChar( GXCP_ACP, 0,
          (GXLPSTR)callback.item.pszText, -1,
          wineItem->pszText, buflen/sizeof(GXWCHAR));
        wineItem->cchTextMax = buflen/sizeof(GXWCHAR);
        Free(oldText);
      }
    }
  }

  if (mask & GXTVIF_IMAGE)
    wineItem->iImage = callback.item.iImage;

  if (mask & GXTVIF_SELECTEDIMAGE)
    wineItem->iSelectedImage = callback.item.iSelectedImage;

  if (mask & GXTVIF_CHILDREN)
    wineItem->cChildren = callback.item.cChildren;

  /* These members are now permanently set. */
  if (callback.item.mask & TVIF_DI_SETITEM)
    wineItem->callbackMask &= ~callback.item.mask;
}

/***************************************************************************
* This function uses cChildren field to decide whether the item has
* children or not.
* Note: if this returns TRUE, the child items may not actually exist,
* they could be virtual.
*
* Just use wineItem->firstChild to check for physical children.
*/
static GXBOOL
TREEVIEW_HasChildren(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem)
{
  TREEVIEW_UpdateDispInfo(infoPtr, wineItem, GXTVIF_CHILDREN);

  return wineItem->cChildren > 0;
}


/* Item Position ********************************************************/

/* Compute linesOffset, stateOffset, imageOffset, textOffset of an item. */
static GXVOID
TREEVIEW_ComputeItemInternalMetrics(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  /* Same effect, different optimisation. */
#if 0
  GXBOOL lar = ((infoPtr->dwStyle & TVS_LINESATROOT)
    && (infoPtr->dwStyle & (TVS_HASLINES|TVS_HASBUTTONS)));
#else
  GXBOOL lar = ((infoPtr->dwStyle
    & (TVS_LINESATROOT|TVS_HASLINES|TVS_HASBUTTONS))
    > TVS_LINESATROOT);
#endif

  item->linesOffset = infoPtr->uIndent * (item->iLevel + lar - 1)
    - infoPtr->scrollX;
  item->stateOffset = item->linesOffset + infoPtr->uIndent;
  item->imageOffset = item->stateOffset
    + (STATEIMAGEINDEX(item->state) ? infoPtr->stateImageWidth : 0);
  item->textOffset  = item->imageOffset + infoPtr->normalImageWidth;
}

static GXVOID
TREEVIEW_ComputeTextWidth(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item, GXHDC hDC)
{
  GXHDC hdc;
  GXHFONT hOldFont=0;
  GXSIZE sz;

  /* DRAW's OM docker creates items like this */
  if (item->pszText == NULL)
  {
    item->textWidth = 0;
    return;
  }

  if (hDC != 0)
  {
    hdc = hDC;
  }
  else
  {
    hdc = gxGetDC(infoPtr->hwnd);
    hOldFont = (GXHFONT)gxSelectObject(hdc, TREEVIEW_FontForItem(infoPtr, item));
  }

  gxGetTextExtentPoint32W(hdc, item->pszText, GXSTRLEN(item->pszText), &sz);
  item->textWidth = sz.cx;

  if (hDC == 0)
  {
    gxSelectObject(hdc, hOldFont);
    gxReleaseDC(0, hdc);
  }
}

static GXVOID
TREEVIEW_ComputeItemRect(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  item->rect.top = infoPtr->uItemHeight *
    (item->visibleOrder - infoPtr->firstVisible->visibleOrder);

  item->rect.bottom = item->rect.top
    + infoPtr->uItemHeight * item->iIntegral - 1;

  item->rect.left = 0;
  item->rect.right = infoPtr->clientWidth;
}

/* We know that only items after start need their order updated. */
static void
TREEVIEW_RecalculateVisibleOrder(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *start)
{
  GXTREEVIEW_ITEM *item;
  int order;

  if (!start)
  {
    start = infoPtr->root->firstChild;
    order = 0;
  }
  else
    order = start->visibleOrder;

  for (item = start; item != NULL;
    item = TREEVIEW_GetNextListItem(infoPtr, item))
  {
    if (!ISVISIBLE(item) && order > 0)
      TREEVIEW_ComputeItemInternalMetrics(infoPtr, item);
    item->visibleOrder = order;
    order += item->iIntegral;
  }

  infoPtr->maxVisibleOrder = order;

  for (item = start; item != NULL;
    item = TREEVIEW_GetNextListItem(infoPtr, item))
  {
    TREEVIEW_ComputeItemRect(infoPtr, item);
  }
}


/* Update metrics of all items in selected subtree.
* root must be expanded
*/
static GXVOID
TREEVIEW_UpdateSubTree(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *root)
{
  GXTREEVIEW_ITEM *sibling;
  GXHDC hdc;
  GXHFONT hOldFont;

  if (!root->firstChild || !(root->state & TVIS_EXPANDED))
    return;

  root->state &= ~TVIS_EXPANDED;
  sibling = TREEVIEW_GetNextListItem(infoPtr, root);
  root->state |= TVIS_EXPANDED;

  hdc = gxGetDC(infoPtr->hwnd);
  hOldFont = (GXHFONT)gxSelectObject(hdc, infoPtr->hFont);

  for (; root != sibling;
    root = TREEVIEW_GetNextListItem(infoPtr, root))
  {
    TREEVIEW_ComputeItemInternalMetrics(infoPtr, root);

    if (root->callbackMask & GXTVIF_TEXT)
      TREEVIEW_UpdateDispInfo(infoPtr, root, GXTVIF_TEXT);

    if (root->textWidth == 0)
    {
      gxSelectObject(hdc, TREEVIEW_FontForItem(infoPtr, root));
      TREEVIEW_ComputeTextWidth(infoPtr, root, hdc);
    }
  }

  gxSelectObject(hdc, hOldFont);
  gxReleaseDC(infoPtr->hwnd, hdc);
}

/* Item Allocation **********************************************************/

static GXTREEVIEW_ITEM *
TREEVIEW_AllocateItem(const GXTREEVIEW_INFO *infoPtr)
{
  GXTREEVIEW_ITEM *newItem = (GXTREEVIEW_ITEM *)Alloc(sizeof(GXTREEVIEW_ITEM));

  if (!newItem)
    return NULL;

  /* I_IMAGENONE would make more sense but this is neither what is
  * documented (MSDN doesn't specify) nor what Windows actually does
  * (it sets it to zero)... and I can so imagine an application using
  * inc/dec to toggle the images. */
  newItem->iImage = 0;
  newItem->iSelectedImage = 0;

  if (gxDPA_InsertPtr(infoPtr->items, INT_MAX, newItem) == -1)
  {
    Free(newItem);
    return NULL;
  }

  return newItem;
}

/* Exact opposite of TREEVIEW_AllocateItem. In particular, it does not
* free item->pszText. */
static void
TREEVIEW_FreeItem(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  gxDPA_DeletePtr(infoPtr->items, gxDPA_GetPtrIndex(infoPtr->items, item));
  Free(item);
  if (infoPtr->selectedItem == item)
    infoPtr->selectedItem = NULL;
  if (infoPtr->hotItem == item)
    infoPtr->hotItem = NULL;
  if (infoPtr->focusedItem == item)
    infoPtr->focusedItem = NULL;
  if (infoPtr->firstVisible == item)
    infoPtr->firstVisible = NULL;
  if (infoPtr->dropItem == item)
    infoPtr->dropItem = NULL;
  if (infoPtr->insertMarkItem == item)
    infoPtr->insertMarkItem = NULL;
}


/* Item Insertion *******************************************************/

/***************************************************************************
* This method inserts newItem before sibling as a child of parent.
* sibling can be NULL, but only if parent has no children.
*/
static void
TREEVIEW_InsertBefore(GXTREEVIEW_ITEM *newItem, GXTREEVIEW_ITEM *sibling,
            GXTREEVIEW_ITEM *parent)
{
  assert(newItem != NULL);
  assert(parent != NULL);

  if (sibling != NULL)
  {
    assert(sibling->parent == parent);

    if (sibling->prevSibling != NULL)
      sibling->prevSibling->nextSibling = newItem;

    newItem->prevSibling = sibling->prevSibling;
    sibling->prevSibling = newItem;
  }
  else
    newItem->prevSibling = NULL;

  newItem->nextSibling = sibling;

  if (parent->firstChild == sibling)
    parent->firstChild = newItem;

  if (parent->lastChild == NULL)
    parent->lastChild = newItem;
}

/***************************************************************************
* This method inserts newItem after sibling as a child of parent.
* sibling can be NULL, but only if parent has no children.
*/
static void
TREEVIEW_InsertAfter(GXTREEVIEW_ITEM *newItem, GXTREEVIEW_ITEM *sibling,
           GXTREEVIEW_ITEM *parent)
{
  assert(newItem != NULL);
  assert(parent != NULL);

  if (sibling != NULL)
  {
    assert(sibling->parent == parent);

    if (sibling->nextSibling != NULL)
      sibling->nextSibling->prevSibling = newItem;

    newItem->nextSibling = sibling->nextSibling;
    sibling->nextSibling = newItem;
  }
  else
    newItem->nextSibling = NULL;

  newItem->prevSibling = sibling;

  if (parent->lastChild == sibling)
    parent->lastChild = newItem;

  if (parent->firstChild == NULL)
    parent->firstChild = newItem;
}

static GXBOOL
TREEVIEW_DoSetItemT(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
          const GXTVITEMEXW *tvItem, GXBOOL isW)
{
  GXUINT callbackClear = 0;
  GXUINT callbackSet = 0;

  TRACE("item %p\n", wineItem);
  /* Do this first in case it fails. */
  if (tvItem->mask & GXTVIF_TEXT)
  {
    wineItem->textWidth = 0; /* force width recalculation */
    if (tvItem->pszText != GXLPSTR_TEXTCALLBACKW) /* covers != TEXTCALLBACKA too */
    {
      int len;
      GXLPWSTR newText;
      if (isW)
        len = GXSTRLEN(tvItem->pszText) + 1;
      else
        len = gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPSTR)tvItem->pszText, -1, NULL, 0);

      newText  = (GXLPWSTR)ReAlloc(wineItem->pszText, len * sizeof(GXWCHAR));

      if (newText == NULL) return FALSE;

      callbackClear |= GXTVIF_TEXT;

      wineItem->pszText = newText;
      wineItem->cchTextMax = len;
      if (isW)
        GXSTRCPYN(wineItem->pszText, tvItem->pszText, len);
      else
        gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPSTR)tvItem->pszText, -1,
        wineItem->pszText, len);

      TRACE("setting text %s, item %p\n", debugstr_w(wineItem->pszText), wineItem);
    }
    else
    {
      callbackSet |= GXTVIF_TEXT;

      wineItem->pszText = (GXLPWSTR)ReAlloc(wineItem->pszText,
        TEXT_CALLBACK_SIZE * sizeof(GXWCHAR));
      wineItem->cchTextMax = TEXT_CALLBACK_SIZE;
      TRACE("setting callback, item %p\n", wineItem);
    }
  }

  if (tvItem->mask & GXTVIF_CHILDREN)
  {
    wineItem->cChildren = tvItem->cChildren;

    if (wineItem->cChildren == I_CHILDRENCALLBACK)
      callbackSet |= GXTVIF_CHILDREN;
    else
      callbackClear |= GXTVIF_CHILDREN;
  }

  if (tvItem->mask & GXTVIF_IMAGE)
  {
    wineItem->iImage = tvItem->iImage;

    if (wineItem->iImage == GXI_IMAGECALLBACK)
      callbackSet |= GXTVIF_IMAGE;
    else
      callbackClear |= GXTVIF_IMAGE;
  }

  if (tvItem->mask & GXTVIF_SELECTEDIMAGE)
  {
    wineItem->iSelectedImage = tvItem->iSelectedImage;

    if (wineItem->iSelectedImage == GXI_IMAGECALLBACK)
      callbackSet |= GXTVIF_SELECTEDIMAGE;
    else
      callbackClear |= GXTVIF_SELECTEDIMAGE;
  }

  if (tvItem->mask & GXTVIF_PARAM)
    wineItem->lParam = tvItem->lParam;

  /* If the application sets TVIF_INTEGRAL without
  * supplying a TVITEMEX structure, it's toast. */
  if (tvItem->mask & GXTVIF_INTEGRAL)
    wineItem->iIntegral = tvItem->iIntegral;

  if (tvItem->mask & GXTVIF_STATE)
  {
    TRACE("prevstate,state,mask:%x,%x,%x\n", wineItem->state, tvItem->state,
      tvItem->stateMask);
    wineItem->state &= ~tvItem->stateMask;
    wineItem->state |= (tvItem->state & tvItem->stateMask);
  }

  wineItem->callbackMask |= callbackSet;
  wineItem->callbackMask &= ~callbackClear;

  return TRUE;
}

/* Note that the new item is pre-zeroed. */
static GXLRESULT
TREEVIEW_InsertItemT(GXTREEVIEW_INFO *infoPtr, const GXTVINSERTSTRUCTW *ptdi, GXBOOL isW)
{
  const GXTVITEMEXW *tvItem = &ptdi->u.itemex;
  GXHTREEITEM insertAfter;
  GXTREEVIEW_ITEM *newItem, *parentItem;
  GXBOOL bTextUpdated = FALSE;

  if (ptdi->hParent == GXTVI_ROOT || ptdi->hParent == 0)
  {
    parentItem = infoPtr->root;
  }
  else
  {
    parentItem = ptdi->hParent;

    if (!TREEVIEW_ValidItem(infoPtr, parentItem))
    {
      WARN("invalid parent %p\n", parentItem);
      return (GXLRESULT)(GXHTREEITEM)NULL;
    }
  }

  insertAfter = ptdi->hInsertAfter;

  /* Validate this now for convenience. */
  switch ((GXDWORD_PTR)insertAfter)
  {
  case ULONG_GXTVI_FIRST:
  case ULONG_GXTVI_LAST:
  case ULONG_GXTVI_SORT:
    break;

  default:
    if (!TREEVIEW_ValidItem(infoPtr, insertAfter) ||
      insertAfter->parent != parentItem)
    {
      WARN("invalid insert after %p\n", insertAfter);
      insertAfter = GXTVI_LAST;
    }
  }

  // TRACE("parent %p position %p: %s\n", parentItem, insertAfter,
  //(tvItem->mask & TVIF_TEXT)
  //? ((tvItem->pszText == GXLPSTR_TEXTCALLBACKW) ? "<callback>"
  //   : (isW ? debugstr_w(tvItem->pszText) : debugstr_a((GXLPSTR)tvItem->pszText)))
  //: "<no label>");

  newItem = TREEVIEW_AllocateItem(infoPtr);
  if (newItem == NULL)
    return (GXLRESULT)(GXHTREEITEM)NULL;

  newItem->parent = parentItem;
  newItem->iIntegral = 1;

  if (!TREEVIEW_DoSetItemT(infoPtr, newItem, tvItem, isW))
    return (GXLRESULT)(GXHTREEITEM)NULL;

  /* After this point, nothing can fail. (Except for TVI_SORT.) */

  infoPtr->uNumItems++;

  switch ((GXDWORD_PTR)insertAfter)
  {
  case (GXDWORD_PTR)ULONG_GXTVI_FIRST:
    {
      GXTREEVIEW_ITEM *originalFirst = parentItem->firstChild;
      TREEVIEW_InsertBefore(newItem, parentItem->firstChild, parentItem);
      if (infoPtr->firstVisible == originalFirst)
        TREEVIEW_SetFirstVisible(infoPtr, newItem, TRUE);
    }
    break;

  case (GXDWORD_PTR)ULONG_GXTVI_LAST:
    TREEVIEW_InsertAfter(newItem, parentItem->lastChild, parentItem);
    break;

    /* hInsertAfter names a specific item we want to insert after */
  default:
    TREEVIEW_InsertAfter(newItem, insertAfter, insertAfter->parent);
    break;

  case (GXDWORD_PTR)ULONG_GXTVI_SORT:
    {
      GXTREEVIEW_ITEM *aChild;
      GXTREEVIEW_ITEM *previousChild = NULL;
      GXTREEVIEW_ITEM *originalFirst = parentItem->firstChild;
      GXBOOL bItemInserted = FALSE;

      aChild = parentItem->firstChild;

      bTextUpdated = TRUE;
      TREEVIEW_UpdateDispInfo(infoPtr, newItem, GXTVIF_TEXT);

      /* Iterate the parent children to see where we fit in */
      while (aChild != NULL)
      {
        GXINT comp;

        TREEVIEW_UpdateDispInfo(infoPtr, aChild, GXTVIF_TEXT);
        comp = GXSTRCMP(newItem->pszText, aChild->pszText);

        if (comp < 0)  /* we are smaller than the current one */
        {
          TREEVIEW_InsertBefore(newItem, aChild, parentItem);
          if (infoPtr->firstVisible == originalFirst &&
            aChild == originalFirst)
            TREEVIEW_SetFirstVisible(infoPtr, newItem, TRUE);
          bItemInserted = TRUE;
          break;
        }
        else if (comp > 0)  /* we are bigger than the current one */
        {
          previousChild = aChild;

          /* This will help us to exit if there is no more sibling */
          aChild = (aChild->nextSibling == 0)
            ? NULL
            : aChild->nextSibling;

          /* Look at the next item */
          continue;
        }
        else if (comp == 0)
        {
          /*
          * An item with this name is already existing, therefore,
          * we add after the one we found
          */
          TREEVIEW_InsertAfter(newItem, aChild, parentItem);
          bItemInserted = TRUE;
          break;
        }
      }

      /*
      * we reach the end of the child list and the item has not
      * yet been inserted, therefore, insert it after the last child.
      */
      if ((!bItemInserted) && (aChild == NULL))
        TREEVIEW_InsertAfter(newItem, previousChild, parentItem);

      break;
    }
  }


  TRACE("new item %p; parent %p, mask %x\n", newItem,
    newItem->parent, tvItem->mask);

  newItem->iLevel = newItem->parent->iLevel + 1;

  if (newItem->parent->cChildren == 0)
    newItem->parent->cChildren = 1;

  if (infoPtr->dwStyle & TVS_CHECKBOXES)
  {
    if (STATEIMAGEINDEX(newItem->state) == 0)
      newItem->state |= INDEXTOSTATEIMAGEMASK(1);
  }

  if (infoPtr->firstVisible == NULL)
    infoPtr->firstVisible = newItem;

  TREEVIEW_VerifyTree(infoPtr);

  if (parentItem == infoPtr->root ||
    (ISVISIBLE(parentItem) && parentItem->state & TVIS_EXPANDED))
  {
    GXTREEVIEW_ITEM *item;
    GXTREEVIEW_ITEM *prev = TREEVIEW_GetPrevListItem(infoPtr, newItem);

    TREEVIEW_RecalculateVisibleOrder(infoPtr, prev);
    TREEVIEW_ComputeItemInternalMetrics(infoPtr, newItem);

    if (!bTextUpdated)
      TREEVIEW_UpdateDispInfo(infoPtr, newItem, GXTVIF_TEXT);

    TREEVIEW_ComputeTextWidth(infoPtr, newItem, 0);
    TREEVIEW_UpdateScrollBars(infoPtr);
    /*
    * if the item was inserted in a visible part of the tree,
    * invalidate it, as well as those after it
    */
    for (item = newItem;
      item != NULL;
      item = TREEVIEW_GetNextListItem(infoPtr, item))
      TREEVIEW_Invalidate(infoPtr, item);
  }
  else
  {
    newItem->visibleOrder = -1;

    /* refresh treeview if newItem is the first item inserted under parentItem */
    if (ISVISIBLE(parentItem) && newItem->prevSibling == newItem->nextSibling)
    {
      /* parent got '+' - update it */
      TREEVIEW_Invalidate(infoPtr, parentItem);
    }
  }

  return (GXLRESULT)newItem;
}

/* Item Deletion ************************************************************/
static void
TREEVIEW_RemoveItem(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem);

static void
TREEVIEW_RemoveAllChildren(GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *parentItem)
{
  GXTREEVIEW_ITEM *kill = parentItem->firstChild;

  while (kill != NULL)
  {
    GXTREEVIEW_ITEM *next = kill->nextSibling;

    TREEVIEW_RemoveItem(infoPtr, kill);

    kill = next;
  }

  assert(parentItem->cChildren <= 0); /* I_CHILDRENCALLBACK or 0 */
  assert(parentItem->firstChild == NULL);
  assert(parentItem->lastChild == NULL);
}

static void
TREEVIEW_UnlinkItem(const GXTREEVIEW_ITEM *item)
{
  GXTREEVIEW_ITEM *parentItem = item->parent;

  assert(item != NULL);
  assert(item->parent != NULL); /* i.e. it must not be the root */

  if (parentItem->firstChild == item)
    parentItem->firstChild = item->nextSibling;

  if (parentItem->lastChild == item)
    parentItem->lastChild = item->prevSibling;

  if (parentItem->firstChild == NULL && parentItem->lastChild == NULL
    && parentItem->cChildren > 0)
    parentItem->cChildren = 0;

  if (item->prevSibling)
    item->prevSibling->nextSibling = item->nextSibling;

  if (item->nextSibling)
    item->nextSibling->prevSibling = item->prevSibling;
}

static void
TREEVIEW_RemoveItem(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem)
{
  TRACE("%p, (%s)\n", wineItem, TREEVIEW_ItemName(wineItem));

  TREEVIEW_SendTreeviewNotify(infoPtr, GXTVN_DELETEITEMW, TVC_UNKNOWN,
    GXTVIF_HANDLE | GXTVIF_PARAM, wineItem, 0);

  if (wineItem->firstChild)
    TREEVIEW_RemoveAllChildren(infoPtr, wineItem);

  TREEVIEW_UnlinkItem(wineItem);

  infoPtr->uNumItems--;

  if (wineItem->pszText != GXLPSTR_TEXTCALLBACKW)
    Free(wineItem->pszText);

  TREEVIEW_FreeItem(infoPtr, wineItem);
}


/* Empty out the tree. */
static void
TREEVIEW_RemoveTree(GXTREEVIEW_INFO *infoPtr)
{
  TREEVIEW_RemoveAllChildren(infoPtr, infoPtr->root);

  assert(infoPtr->uNumItems == 0);  /* root isn't counted in uNumItems */
}

static GXLRESULT
TREEVIEW_DeleteItem(GXTREEVIEW_INFO *infoPtr, GXHTREEITEM wineItem)
{
  GXTREEVIEW_ITEM *newSelection = NULL;
  GXTREEVIEW_ITEM *newFirstVisible = NULL;
  GXTREEVIEW_ITEM *parent, *prev = NULL;
  GXBOOL visible = FALSE;

  if (wineItem == GXTVI_ROOT)
  {
    TRACE("TVI_ROOT\n");
    parent = infoPtr->root;
    newSelection = NULL;
    visible = TRUE;
    TREEVIEW_RemoveTree(infoPtr);
  }
  else
  {
    if (!TREEVIEW_ValidItem(infoPtr, wineItem))
      return FALSE;

    TRACE("%p (%s)\n", wineItem, TREEVIEW_ItemName(wineItem));
    parent = wineItem->parent;

    if (ISVISIBLE(wineItem))
    {
      prev = TREEVIEW_GetPrevListItem(infoPtr, wineItem);
      visible = TRUE;
    }

    if (infoPtr->selectedItem != NULL
      && (wineItem == infoPtr->selectedItem
      || TREEVIEW_IsChildOf(wineItem, infoPtr->selectedItem)))
    {
      if (wineItem->nextSibling)
        newSelection = wineItem->nextSibling;
      else if (wineItem->parent != infoPtr->root)
        newSelection = wineItem->parent;
      else
        newSelection = wineItem->prevSibling;
      TRACE("newSelection = %p\n", newSelection);
    }

    if (infoPtr->firstVisible == wineItem)
    {
      if (wineItem->nextSibling)
        newFirstVisible = wineItem->nextSibling;
      else if (wineItem->prevSibling)
        newFirstVisible = wineItem->prevSibling;
      else if (wineItem->parent != infoPtr->root)
        newFirstVisible = wineItem->parent;
      TREEVIEW_SetFirstVisible(infoPtr, NULL, TRUE);
    }
    else
      newFirstVisible = infoPtr->firstVisible;

    TREEVIEW_RemoveItem(infoPtr, wineItem);
  }

  /* Don't change if somebody else already has (infoPtr->selectedItem is cleared by FreeItem). */
  if (!infoPtr->selectedItem && newSelection)
  {
    if (TREEVIEW_ValidItem(infoPtr, newSelection))
      TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, newSelection, TVC_UNKNOWN);
  }

  /* Validate insertMark dropItem.
  * hotItem ??? - used for comparison only.
  */
  if (!TREEVIEW_ValidItem(infoPtr, infoPtr->insertMarkItem))
    infoPtr->insertMarkItem = 0;

  if (!TREEVIEW_ValidItem(infoPtr, infoPtr->dropItem))
    infoPtr->dropItem = 0;

  if (!TREEVIEW_ValidItem(infoPtr, newFirstVisible))
    newFirstVisible = infoPtr->root->firstChild;

  TREEVIEW_VerifyTree(infoPtr);


  if (visible)
  {
    TREEVIEW_SetFirstVisible(infoPtr, newFirstVisible, TRUE);
    TREEVIEW_RecalculateVisibleOrder(infoPtr, prev);
    TREEVIEW_UpdateScrollBars(infoPtr);
    TREEVIEW_Invalidate(infoPtr, NULL);
  }
  else if (ISVISIBLE(parent) && !TREEVIEW_HasChildren(infoPtr, parent))
  {
    /* parent lost '+/-' - update it */
    TREEVIEW_Invalidate(infoPtr, parent);
  }

  return TRUE;
}


/* Get/Set Messages *********************************************************/
static GXLRESULT
TREEVIEW_SetRedraw(GXTREEVIEW_INFO* infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  if(wParam)
    infoPtr->bRedraw = TRUE;
  else
    infoPtr->bRedraw = FALSE;

  return 0;
}

static GXLRESULT
TREEVIEW_GetIndent(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return infoPtr->uIndent;
}

static GXLRESULT
TREEVIEW_SetIndent(GXTREEVIEW_INFO *infoPtr, GXUINT newIndent)
{
  TRACE("\n");

  if (newIndent < MINIMUM_INDENT)
    newIndent = MINIMUM_INDENT;

  if (infoPtr->uIndent != newIndent)
  {
    infoPtr->uIndent = newIndent;
    TREEVIEW_UpdateSubTree(infoPtr, infoPtr->root);
    TREEVIEW_UpdateScrollBars(infoPtr);
    TREEVIEW_Invalidate(infoPtr, NULL);
  }

  return 0;
}


static GXLRESULT
TREEVIEW_GetToolTips(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return (GXLRESULT)infoPtr->hwndToolTip;
}

static GXLRESULT
TREEVIEW_SetToolTips(GXTREEVIEW_INFO *infoPtr, GXHWND hwndTT)
{
  GXHWND prevToolTip;

  TRACE("\n");
  prevToolTip = infoPtr->hwndToolTip;
  infoPtr->hwndToolTip = hwndTT;

  return (GXLRESULT)prevToolTip;
}

static GXLRESULT
TREEVIEW_SetUnicodeFormat(GXTREEVIEW_INFO *infoPtr, GXBOOL fUnicode)
{
  GXBOOL rc = infoPtr->bNtfUnicode;
  infoPtr->bNtfUnicode = fUnicode;
  return rc;
}

static GXLRESULT
TREEVIEW_GetUnicodeFormat(const GXTREEVIEW_INFO *infoPtr)
{
  return infoPtr->bNtfUnicode;
}

static GXLRESULT
TREEVIEW_GetScrollTime(const GXTREEVIEW_INFO *infoPtr)
{
  return infoPtr->uScrollTime;
}

static GXLRESULT
TREEVIEW_SetScrollTime(GXTREEVIEW_INFO *infoPtr, GXUINT uScrollTime)
{
  GXUINT uOldScrollTime = infoPtr->uScrollTime;

  infoPtr->uScrollTime = min(uScrollTime, 100);

  return uOldScrollTime;
}


static GXLRESULT
TREEVIEW_GetImageList(const GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  TRACE("\n");

  switch (wParam)
  {
  case (GXWPARAM)TVSIL_NORMAL:
    return (GXLRESULT)infoPtr->himlNormal;

  case (GXWPARAM)TVSIL_STATE:
    return (GXLRESULT)infoPtr->himlState;

  default:
    return 0;
  }
}

#define TVHEIGHT_MIN         16
#define TVHEIGHT_FONT_ADJUST 3 /* 2 for focus border + 1 for margin some apps assume */

/* Compute the natural height for items. */
static GXUINT
TREEVIEW_NaturalHeight(const GXTREEVIEW_INFO *infoPtr)
{
  GXTEXTMETRICW tm;
  GXHDC hdc = gxGetDC(0);
  GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, infoPtr->hFont);
  GXUINT height;

  /* Height is the maximum of:
  * 16 (a hack because our fonts are tiny), and
  * The text height + border & margin, and
  * The size of the normal image list
  */
  gxGetTextMetricsW(hdc, &tm);
  gxSelectObject(hdc, hOldFont);
  gxReleaseDC(0, hdc);

  height = TVHEIGHT_MIN;
  if (height < tm.tmHeight + tm.tmExternalLeading + TVHEIGHT_FONT_ADJUST)
    height = tm.tmHeight + tm.tmExternalLeading + TVHEIGHT_FONT_ADJUST;
  if (height < infoPtr->normalImageHeight)
    height = infoPtr->normalImageHeight;

  /* Round down, unless we support odd ("non even") heights. */
  if (!(infoPtr->dwStyle & TVS_NONEVENHEIGHT))
    height &= ~1;

  return height;
}

static GXLRESULT
TREEVIEW_SetImageList(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXHIMAGELIST himlNew)
{
  GXHIMAGELIST himlOld = 0;
  int oldWidth  = infoPtr->normalImageWidth;
  int oldHeight = infoPtr->normalImageHeight;


  TRACE("%lx,%p\n", wParam, himlNew);

  switch (wParam)
  {
  case (GXWPARAM)TVSIL_NORMAL:
    himlOld = infoPtr->himlNormal;
    infoPtr->himlNormal = himlNew;

    if (himlNew != NULL)
      gxImageList_GetIconSize(himlNew, &infoPtr->normalImageWidth,
      &infoPtr->normalImageHeight);
    else
    {
      infoPtr->normalImageWidth = 0;
      infoPtr->normalImageHeight = 0;
    }

    break;

  case (GXWPARAM)TVSIL_STATE:
    himlOld = infoPtr->himlState;
    infoPtr->himlState = himlNew;

    if (himlNew != NULL)
      gxImageList_GetIconSize(himlNew, &infoPtr->stateImageWidth,
      &infoPtr->stateImageHeight);
    else
    {
      infoPtr->stateImageWidth = 0;
      infoPtr->stateImageHeight = 0;
    }

    break;
  }

  if (oldWidth != infoPtr->normalImageWidth ||
    oldHeight != infoPtr->normalImageHeight)
  {
    GXBOOL bRecalcVisible = FALSE;

    if (oldHeight != infoPtr->normalImageHeight &&
      !infoPtr->bHeightSet)
    {
      infoPtr->uItemHeight = TREEVIEW_NaturalHeight(infoPtr);
      bRecalcVisible = TRUE;
    }

    if (infoPtr->normalImageWidth > MINIMUM_INDENT &&
      infoPtr->normalImageWidth != infoPtr->uIndent)
    {
      infoPtr->uIndent = infoPtr->normalImageWidth;
      bRecalcVisible = TRUE;
    }

    if (bRecalcVisible)
      TREEVIEW_RecalculateVisibleOrder(infoPtr, NULL);

    TREEVIEW_UpdateSubTree(infoPtr, infoPtr->root);
    TREEVIEW_UpdateScrollBars(infoPtr);
  }

  TREEVIEW_Invalidate(infoPtr, NULL);

  return (GXLRESULT)himlOld;
}

static GXLRESULT
TREEVIEW_SetItemHeight(GXTREEVIEW_INFO *infoPtr, GXINT newHeight)
{
  GXINT prevHeight = infoPtr->uItemHeight;

  TRACE("%d\n", newHeight);
  if (newHeight == -1)
  {
    infoPtr->uItemHeight = TREEVIEW_NaturalHeight(infoPtr);
    infoPtr->bHeightSet = FALSE;
  }
  else
  {
    infoPtr->uItemHeight = newHeight;
    infoPtr->bHeightSet = TRUE;
  }

  /* Round down, unless we support odd ("non even") heights. */
  if (!(infoPtr->dwStyle & TVS_NONEVENHEIGHT))
    infoPtr->uItemHeight &= ~1;

  if (infoPtr->uItemHeight != prevHeight)
  {
    TREEVIEW_RecalculateVisibleOrder(infoPtr, NULL);
    TREEVIEW_UpdateScrollBars(infoPtr);
    TREEVIEW_Invalidate(infoPtr, NULL);
  }

  return prevHeight;
}

static GXLRESULT
TREEVIEW_GetItemHeight(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return infoPtr->uItemHeight;
}


static GXLRESULT
TREEVIEW_GetFont(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("%p\n", infoPtr->hFont);
  return (GXLRESULT)infoPtr->hFont;
}


static GXINT GXCALLBACK
TREEVIEW_ResetTextWidth(GXLPVOID pItem, GXLPVOID unused)
{
  (void)unused;

  ((GXTREEVIEW_ITEM *)pItem)->textWidth = 0;

  return 1;
}

static GXLRESULT
TREEVIEW_SetFont(GXTREEVIEW_INFO *infoPtr, GXHFONT hFont, GXBOOL bRedraw)
{
  GXUINT uHeight = infoPtr->uItemHeight;

  TRACE("%p %i\n", hFont, bRedraw);

  infoPtr->hFont = hFont ? hFont : infoPtr->hDefaultFont;

  gxDeleteObject(infoPtr->hBoldFont);
  infoPtr->hBoldFont = TREEVIEW_CreateBoldFont(infoPtr->hFont);
  infoPtr->hUnderlineFont = TREEVIEW_CreateUnderlineFont(infoPtr->hFont);

  if (!infoPtr->bHeightSet)
    infoPtr->uItemHeight = TREEVIEW_NaturalHeight(infoPtr);

  if (uHeight != infoPtr->uItemHeight)
    TREEVIEW_RecalculateVisibleOrder(infoPtr, NULL);

  gxDPA_EnumCallback(infoPtr->items, (GXPFNDPAENUMCALLBACK)TREEVIEW_ResetTextWidth, 0);

  TREEVIEW_UpdateSubTree(infoPtr, infoPtr->root);
  TREEVIEW_UpdateScrollBars(infoPtr);

  if (bRedraw)
    TREEVIEW_Invalidate(infoPtr, NULL);

  return 0;
}


static GXLRESULT
TREEVIEW_GetLineColor(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return (GXLRESULT)infoPtr->clrLine;
}

static GXLRESULT
TREEVIEW_SetLineColor(GXTREEVIEW_INFO *infoPtr, GXCOLORREF color)
{
  GXCOLORREF prevColor = infoPtr->clrLine;

  TRACE("\n");
  infoPtr->clrLine = color;
  return (GXLRESULT)prevColor;
}


static GXLRESULT
TREEVIEW_GetTextColor(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return (GXLRESULT)infoPtr->clrText;
}

static GXLRESULT
TREEVIEW_SetTextColor(GXTREEVIEW_INFO *infoPtr, GXCOLORREF color)
{
  GXCOLORREF prevColor = infoPtr->clrText;

  TRACE("\n");
  infoPtr->clrText = color;

  if (infoPtr->clrText != prevColor)
    TREEVIEW_Invalidate(infoPtr, NULL);

  return (GXLRESULT)prevColor;
}


static GXLRESULT
TREEVIEW_GetBkColor(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return (GXLRESULT)infoPtr->clrBk;
}

static GXLRESULT
TREEVIEW_SetBkColor(GXTREEVIEW_INFO *infoPtr, GXCOLORREF newColor)
{
  GXCOLORREF prevColor = infoPtr->clrBk;

  TRACE("\n");
  infoPtr->clrBk = newColor;

  if (newColor != prevColor)
    TREEVIEW_Invalidate(infoPtr, NULL);

  return (GXLRESULT)prevColor;
}


static GXLRESULT
TREEVIEW_GetInsertMarkColor(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");
  return (GXLRESULT)infoPtr->clrInsertMark;
}

static GXLRESULT
TREEVIEW_SetInsertMarkColor(GXTREEVIEW_INFO *infoPtr, GXCOLORREF color)
{
  GXCOLORREF prevColor = infoPtr->clrInsertMark;

  TRACE("%x\n", color);
  infoPtr->clrInsertMark = color;

  return (GXLRESULT)prevColor;
}


static GXLRESULT
TREEVIEW_SetInsertMark(GXTREEVIEW_INFO *infoPtr, GXBOOL wParam, GXHTREEITEM item)
{
  TRACE("%d %p\n", wParam, item);

  if (!TREEVIEW_ValidItem(infoPtr, item))
    return 0;

  infoPtr->insertBeforeorAfter = wParam;
  infoPtr->insertMarkItem = item;

  TREEVIEW_Invalidate(infoPtr, NULL);

  return 1;
}


/************************************************************************
* Some serious braindamage here. lParam is a pointer to both the
* input GXHTREEITEM and the output GXRECT.
*/
static GXLRESULT
TREEVIEW_GetItemRect(const GXTREEVIEW_INFO *infoPtr, GXBOOL fTextRect, LPGXRECT lpRect)
{
  GXTREEVIEW_ITEM *wineItem;
  const GXHTREEITEM *pItem = (GXHTREEITEM *)lpRect;

  TRACE("\n");
  /*
  * validate parameters
  */
  if (pItem == NULL)
    return FALSE;

  wineItem = *pItem;
  if (!TREEVIEW_ValidItem(infoPtr, wineItem) || !ISVISIBLE(wineItem))
    return FALSE;

  /*
  * If wParam is TRUE return the text size otherwise return
  * the whole item size
  */
  if (fTextRect)
  {
    /* Windows does not send TVN_GETDISPINFO here. */

    lpRect->top = wineItem->rect.top;
    lpRect->bottom = wineItem->rect.bottom;

    lpRect->left = wineItem->textOffset;
    if (!wineItem->textWidth)
      TREEVIEW_ComputeTextWidth(infoPtr, wineItem, 0);

    lpRect->right = wineItem->textOffset + wineItem->textWidth + 4;
  }
  else
  {
    *lpRect = wineItem->rect;
  }

  TRACE("%s [L:%d R:%d T:%d B:%d]\n", fTextRect ? "text" : "item",
    lpRect->left, lpRect->right, lpRect->top, lpRect->bottom);

  return TRUE;
}

static inline GXLRESULT
TREEVIEW_GetVisibleCount(const GXTREEVIEW_INFO *infoPtr)
{
  /* Suprise! This does not take integral height into account. */
  return infoPtr->clientHeight / infoPtr->uItemHeight;
}


static GXLRESULT
TREEVIEW_GetItemT(const GXTREEVIEW_INFO *infoPtr, GXLPTVITEMEXW tvItem, GXBOOL isW)
{
  GXTREEVIEW_ITEM *wineItem;

  wineItem = tvItem->hItem;
  if (!TREEVIEW_ValidItem(infoPtr, wineItem))
    return FALSE;

  TREEVIEW_UpdateDispInfo(infoPtr, wineItem, tvItem->mask);

  if (tvItem->mask & GXTVIF_CHILDREN)
  {
    if (wineItem->cChildren==I_CHILDRENCALLBACK)
      FIXME("I_CHILDRENCALLBACK not supported\n");
    tvItem->cChildren = wineItem->cChildren;
  }

  if (tvItem->mask & GXTVIF_HANDLE)
    tvItem->hItem = wineItem;

  if (tvItem->mask & GXTVIF_IMAGE)
    tvItem->iImage = wineItem->iImage;

  if (tvItem->mask & GXTVIF_INTEGRAL)
    tvItem->iIntegral = wineItem->iIntegral;

  /* undocumented: windows ignores TVIF_PARAM and
  * * always sets lParam
  */
  tvItem->lParam = wineItem->lParam;

  if (tvItem->mask & GXTVIF_SELECTEDIMAGE)
    tvItem->iSelectedImage = wineItem->iSelectedImage;

  if (tvItem->mask & GXTVIF_STATE)
    /* Careful here - Windows ignores the stateMask when you get the state
    That contradicts the documentation, but makes more common sense, masking
    retrieval in this way seems overkill */
    tvItem->state = wineItem->state;

  if (tvItem->mask & GXTVIF_TEXT)
  {
    if (isW)
    {
      if (wineItem->pszText == GXLPSTR_TEXTCALLBACKW)
      {
        tvItem->pszText = GXLPSTR_TEXTCALLBACKW;
        FIXME(" GetItem called with LPSTR_TEXTCALLBACK\n");
      }
      else
      {
        GXSTRCPYN(tvItem->pszText, wineItem->pszText, tvItem->cchTextMax);
      }
    }
    else
    {
      if (wineItem->pszText == GXLPSTR_TEXTCALLBACKW)
      {
        tvItem->pszText = (GXLPWSTR)GXLPSTR_TEXTCALLBACKA;
        FIXME(" GetItem called with LPSTR_TEXTCALLBACK\n");
      }
      else
      {
        gxWideCharToMultiByte(GXCP_ACP, 0, wineItem->pszText, -1,
          (GXLPSTR)tvItem->pszText, tvItem->cchTextMax, NULL, NULL);
      }
    }
  }
  TRACE("item <%p>, txt %p, img %p, mask %x\n",
    wineItem, tvItem->pszText, &tvItem->iImage, tvItem->mask);

  return TRUE;
}

/* Beware MSDN Library Visual Studio 6.0. It says -1 on failure, 0 on success,
* which is wrong. */
static GXLRESULT
TREEVIEW_SetItemT(GXTREEVIEW_INFO *infoPtr, const GXTVITEMEXW *tvItem, GXBOOL isW)
{
  GXTREEVIEW_ITEM *wineItem;
  GXTREEVIEW_ITEM originalItem;

  wineItem = tvItem->hItem;

  TRACE("item %d,mask %x\n", TREEVIEW_GetItemIndex(infoPtr, wineItem),
    tvItem->mask);

  if (!TREEVIEW_ValidItem(infoPtr, wineItem))
    return FALSE;

  /* store the orignal item values */
  originalItem = *wineItem;

  if (!TREEVIEW_DoSetItemT(infoPtr, wineItem, tvItem, isW))
    return FALSE;

  /* If the text or TVIS_BOLD was changed, and it is visible, recalculate. */
  if ((tvItem->mask & GXTVIF_TEXT
    || (tvItem->mask & GXTVIF_STATE && tvItem->stateMask & TVIS_BOLD))
    && ISVISIBLE(wineItem))
  {
    TREEVIEW_UpdateDispInfo(infoPtr, wineItem, GXTVIF_TEXT);
    TREEVIEW_ComputeTextWidth(infoPtr, wineItem, 0);
  }

  if (tvItem->mask != 0 && ISVISIBLE(wineItem))
  {
    /* The refresh updates everything, but we can't wait until then. */
    TREEVIEW_ComputeItemInternalMetrics(infoPtr, wineItem);

    /* if any of the item's values changed and it's not a callback, redraw the item */
    if (item_changed(&originalItem, wineItem, tvItem))
    {
      if (tvItem->mask & GXTVIF_INTEGRAL)
      {
        TREEVIEW_RecalculateVisibleOrder(infoPtr, wineItem);
        TREEVIEW_UpdateScrollBars(infoPtr);

        TREEVIEW_Invalidate(infoPtr, NULL);
      }
      else
      {
        TREEVIEW_UpdateScrollBars(infoPtr);
        TREEVIEW_Invalidate(infoPtr, wineItem);
      }
    }
  }

  return TRUE;
}

static GXLRESULT
TREEVIEW_GetItemState(const GXTREEVIEW_INFO *infoPtr, GXHTREEITEM wineItem, GXUINT mask)
{
  TRACE("\n");

  if (!wineItem || !TREEVIEW_ValidItem(infoPtr, wineItem))
    return 0;

  return (wineItem->state & mask);
}

static GXLRESULT
TREEVIEW_GetNextItem(const GXTREEVIEW_INFO *infoPtr, GXUINT which, GXHTREEITEM wineItem)
{
  GXTREEVIEW_ITEM *retval;

  retval = 0;

  /* handle all the global data here */
  switch (which)
  {
  case GXTVGN_CHILD:    /* Special case: child of 0 is root */
    if (wineItem)
      break;
    /* fall through */
  case GXTVGN_ROOT:
    retval = infoPtr->root->firstChild;
    break;

  case GXTVGN_CARET:
    retval = infoPtr->selectedItem;
    break;

  case GXTVGN_FIRSTVISIBLE:
    retval = infoPtr->firstVisible;
    break;

  case GXTVGN_DROPHILITE:
    retval = infoPtr->dropItem;
    break;

  case GXTVGN_LASTVISIBLE:
    retval = TREEVIEW_GetLastListItem(infoPtr, infoPtr->root);
    break;
  }

  if (retval)
  {
    TRACE("flags:%x, returns %p\n", which, retval);
    return (GXLRESULT)retval;
  }

  if (wineItem == GXTVI_ROOT) wineItem = infoPtr->root;

  if (!TREEVIEW_ValidItem(infoPtr, wineItem))
    return FALSE;

  switch (which)
  {
  case GXTVGN_NEXT:
    retval = wineItem->nextSibling;
    break;
  case GXTVGN_PREVIOUS:
    retval = wineItem->prevSibling;
    break;
  case GXTVGN_PARENT:
    retval = (wineItem->parent != infoPtr->root) ? wineItem->parent : NULL;
    break;
  case GXTVGN_CHILD:
    retval = wineItem->firstChild;
    break;
  case GXTVGN_NEXTVISIBLE:
    retval = TREEVIEW_GetNextListItem(infoPtr, wineItem);
    break;
  case GXTVGN_PREVIOUSVISIBLE:
    retval = TREEVIEW_GetPrevListItem(infoPtr, wineItem);
    break;
  default:
    TRACE("Unknown msg %x,item %p\n", which, wineItem);
    break;
  }

  TRACE("flags:%x, item %p;returns %p\n", which, wineItem, retval);
  return (GXLRESULT)retval;
}


static GXLRESULT
TREEVIEW_GetCount(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE(" %d\n", infoPtr->uNumItems);
  return (GXLRESULT)infoPtr->uNumItems;
}

static GXVOID
TREEVIEW_ToggleItemState(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  if (infoPtr->dwStyle & TVS_CHECKBOXES)
  {
    static const unsigned int state_table[] = { 0, 2, 1 };

    unsigned int state;

    state = STATEIMAGEINDEX(item->state);
    TRACE("state:%x\n", state);
    item->state &= ~TVIS_STATEIMAGEMASK;

    if (state < 3)
      state = state_table[state];

    item->state |= INDEXTOSTATEIMAGEMASK(state);

    TRACE("state:%x\n", state);
    TREEVIEW_Invalidate(infoPtr, item);
  }
}


/* Painting *************************************************************/

/* Draw the lines and expand button for an item. Also draws one section
* of the line from item's parent to item's parent's next sibling. */
static void
TREEVIEW_DrawItemLines(const GXTREEVIEW_INFO *infoPtr, GXHDC hdc, const GXTREEVIEW_ITEM *item)
{
  GXLONG centerx, centery;
  GXBOOL lar = ((infoPtr->dwStyle
    & (TVS_LINESATROOT|TVS_HASLINES|TVS_HASBUTTONS))
    > TVS_LINESATROOT);
  GXHBRUSH hbr, hbrOld;

  if (!lar && item->iLevel == 0)
    return;

  hbr    = gxCreateSolidBrush(infoPtr->clrBk);
  hbrOld = (GXHBRUSH)gxSelectObject(hdc, hbr);

  centerx = (item->linesOffset + item->stateOffset) / 2;
  centery = (item->rect.top + item->rect.bottom) / 2;

  if (infoPtr->dwStyle & TVS_HASLINES)
  {
    GXHPEN hOldPen, hNewPen;
    GXHTREEITEM parent;
    GXLOGBRUSH lb;

    /* Get a dotted grey pen */
    lb.lbStyle = GXBS_SOLID;
    lb.lbColor = infoPtr->clrLine;
    hNewPen = gxExtCreatePen(GXPS_COSMETIC|GXPS_ALTERNATE, 1, &lb, 0, NULL);
    hOldPen = (GXHPEN)gxSelectObject(hdc, hNewPen);

    /* Make sure the center is on a dot (using +2 instead
    * of +1 gives us pixel-by-pixel compat with native) */
    centery = (centery + 2) & ~1;

    gxMoveToEx(hdc, item->stateOffset, centery, NULL);
    gxLineTo(hdc, centerx - 1, centery);

    if (item->prevSibling || item->parent != infoPtr->root)
    {
      gxMoveToEx(hdc, centerx, item->rect.top, NULL);
      gxLineTo(hdc, centerx, centery);
    }

    if (item->nextSibling)
    {
      gxMoveToEx(hdc, centerx, centery, NULL);
      gxLineTo(hdc, centerx, item->rect.bottom + 1);
    }

    /* Draw the line from our parent to its next sibling. */
    parent = item->parent;
    while (parent != infoPtr->root)
    {
      int pcenterx = (parent->linesOffset + parent->stateOffset) / 2;

      if (parent->nextSibling
        /* skip top-levels unless TVS_LINESATROOT */
        && parent->stateOffset > parent->linesOffset)
      {
        gxMoveToEx(hdc, pcenterx, item->rect.top, NULL);
        gxLineTo(hdc, pcenterx, item->rect.bottom + 1);
      }

      parent = parent->parent;
    }

    gxSelectObject(hdc, hOldPen);
    gxDeleteObject(hNewPen);
  }

  /*
  * Display the (+/-) signs
  */

  if (infoPtr->dwStyle & TVS_HASBUTTONS)
  {
    if (item->cChildren)
    {
      GXHTHEME theme = gxGetWindowTheme(infoPtr->hwnd);
      if (theme)
      {
        GXRECT glyphRect = item->rect;
        glyphRect.left = item->linesOffset;
        glyphRect.right = item->stateOffset;
        gxDrawThemeBackground (theme, hdc, TVP_GLYPH,
          (item->state & TVIS_EXPANDED) ? GLPS_OPENED : GLPS_CLOSED,
          &glyphRect, NULL);
      }
      else
      {
        GXLONG height = item->rect.bottom - item->rect.top;
        GXLONG width  = item->stateOffset - item->linesOffset;
        GXLONG rectsize = min(height, width) / 4;
        /* plussize = ceil(rectsize * 3/4) */
        GXLONG plussize = (rectsize + 1) * 3 / 4;

        GXHPEN hNewPen  = gxCreatePen(GXPS_SOLID, 0, infoPtr->clrLine);
        GXHPEN hOldPen  = (GXHPEN)gxSelectObject(hdc, hNewPen);

        gxRectangle(hdc, centerx - rectsize - 1, centery - rectsize - 1,
          centerx + rectsize + 2, centery + rectsize + 2);

        gxSelectObject(hdc, hOldPen);
        gxDeleteObject(hNewPen);

        if (height < 18 || width < 18)
        {
          gxMoveToEx(hdc, centerx - plussize + 1, centery, NULL);
          gxLineTo(hdc, centerx + plussize, centery);

          if (!(item->state & TVIS_EXPANDED))
          {
            gxMoveToEx(hdc, centerx, centery - plussize + 1, NULL);
            gxLineTo(hdc, centerx, centery + plussize);
          }
        }
        else
        {
          gxRectangle(hdc, centerx - plussize + 1, centery - 1,
            centerx + plussize, centery + 2);

          if (!(item->state & TVIS_EXPANDED))
          {
            gxRectangle(hdc, centerx - 1, centery - plussize + 1,
              centerx + 2, centery + plussize);
            gxSetPixel(hdc, centerx - 1, centery, infoPtr->clrBk);
            gxSetPixel(hdc, centerx + 1, centery, infoPtr->clrBk);
          }
        }
      }
    }
  }
  gxSelectObject(hdc, hbrOld);
  gxDeleteObject(hbr);
}

static void
TREEVIEW_DrawItem(const GXTREEVIEW_INFO *infoPtr, GXHDC hdc, GXTREEVIEW_ITEM *wineItem)
{
  GXINT cditem;
  GXHFONT hOldFont;
  GXCOLORREF oldTextColor, oldTextBkColor;
  int centery;
  GXBOOL inFocus = (gxGetFocus() == infoPtr->hwnd);
  GXNMTVCUSTOMDRAW nmcdhdr;

  TREEVIEW_UpdateDispInfo(infoPtr, wineItem, CALLBACK_MASK_ALL);

  /* - If item is drop target or it is selected and window is in focus -
  * use blue background (GXCOLOR_HIGHLIGHT).
  * - If item is selected, window is not in focus, but it has style
  * TVS_SHOWSELALWAYS - use grey background (GXCOLOR_BTNFACE)
  * - Otherwise - use background color
  */
  if ((wineItem->state & TVIS_DROPHILITED) || ((wineItem == infoPtr->focusedItem) && !(wineItem->state & TVIS_SELECTED)) ||
    ((wineItem->state & TVIS_SELECTED) && (!infoPtr->focusedItem) &&
    (inFocus || (infoPtr->dwStyle & TVS_SHOWSELALWAYS))))
  {
    if ((wineItem->state & TVIS_DROPHILITED) || inFocus)
    {
      nmcdhdr.clrTextBk = gxGetSysColor(GXCOLOR_HIGHLIGHT);
      nmcdhdr.clrText   = gxGetSysColor(GXCOLOR_HIGHLIGHTTEXT);
    }
    else
    {
      nmcdhdr.clrTextBk = gxGetSysColor(GXCOLOR_BTNFACE);
      if (infoPtr->clrText == -1)
        nmcdhdr.clrText = gxGetSysColor(GXCOLOR_WINDOWTEXT);
      else
        nmcdhdr.clrText = infoPtr->clrText;
    }
  }
  else
  {
    nmcdhdr.clrTextBk = infoPtr->clrBk;
    if ((infoPtr->dwStyle & TVS_TRACKSELECT) && (wineItem == infoPtr->hotItem))
      nmcdhdr.clrText = comctl32_color.clrHighlight;
    else if (infoPtr->clrText == -1)
      nmcdhdr.clrText = gxGetSysColor(GXCOLOR_WINDOWTEXT);
    else
      nmcdhdr.clrText = infoPtr->clrText;
  }

  hOldFont = (GXHFONT)gxSelectObject(hdc, TREEVIEW_FontForItem(infoPtr, wineItem));

  /* The custom draw handler can query the text rectangle,
  * so get ready. */
  /* should already be known, set to 0 when changed */
  if (!wineItem->textWidth)
    TREEVIEW_ComputeTextWidth(infoPtr, wineItem, hdc);

  cditem = 0;

  if (infoPtr->cdmode & CDRF_NOTIFYITEMDRAW)
  {
    cditem = TREEVIEW_SendCustomDrawItemNotify
      (infoPtr, hdc, wineItem, CDDS_ITEMPREPAINT, &nmcdhdr);
    TRACE("prepaint:cditem-app returns 0x%x\n", cditem);

    if (cditem & CDRF_SKIPDEFAULT)
    {
      gxSelectObject(hdc, hOldFont);
      return;
    }
  }

  if (cditem & CDRF_NEWFONT)
    TREEVIEW_ComputeTextWidth(infoPtr, wineItem, hdc);

  TREEVIEW_DrawItemLines(infoPtr, hdc, wineItem);

  /* Set colors. Custom draw handler can change these so we do this after it. */
  oldTextColor = gxSetTextColor(hdc, nmcdhdr.clrText);
  oldTextBkColor = gxSetBkColor(hdc, nmcdhdr.clrTextBk);

  centery = (wineItem->rect.top + wineItem->rect.bottom) / 2;

  /*
  * Display the images associated with this item
  */
  {
    GXINT imageIndex;

    /* State images are displayed to the left of the Normal image
    * image number is in state; zero should be `display no image'.
    */
    imageIndex = STATEIMAGEINDEX(wineItem->state);

    if (infoPtr->himlState && imageIndex)
    {
      gxImageList_Draw(infoPtr->himlState, imageIndex, hdc,
        wineItem->stateOffset,
        centery - infoPtr->stateImageHeight / 2,
        ILD_NORMAL);
    }

    /* Now, draw the normal image; can be either selected or
    * non-selected image.
    */

    if ((wineItem->state & TVIS_SELECTED) && (wineItem->iSelectedImage >= 0))
    {
      /* The item is currently selected */
      imageIndex = wineItem->iSelectedImage;
    }
    else
    {
      /* The item is not selected */
      imageIndex = wineItem->iImage;
    }

    if (infoPtr->himlNormal)
    {
      int ovlIdx = wineItem->state & TVIS_OVERLAYMASK;

      gxImageList_Draw(infoPtr->himlNormal, imageIndex, hdc,
        wineItem->imageOffset,
        centery - infoPtr->normalImageHeight / 2,
        ILD_NORMAL | ovlIdx);
    }
  }


  /*
  * Display the text associated with this item
  */

  /* Don't paint item's text if it's being edited */
  if (!infoPtr->hwndEdit || (infoPtr->selectedItem != wineItem))
  {
    if (wineItem->pszText)
    {
      GXRECT rcText;

      rcText.top = wineItem->rect.top;
      rcText.bottom = wineItem->rect.bottom;
      rcText.left = wineItem->textOffset;
      rcText.right = rcText.left + wineItem->textWidth + 4;

      TRACE("drawing text %s at (%d,%d)-(%d,%d)\n",
        debugstr_w(wineItem->pszText),
        rcText.left, rcText.top, rcText.right, rcText.bottom);

      /* Draw it */
      gxExtTextOutW(hdc, rcText.left + 2, rcText.top + 1,
        GXETO_CLIPPED | GXETO_OPAQUE,
        &rcText,
        wineItem->pszText,
        GXSTRLEN(wineItem->pszText),
        NULL);

      /* Draw the box around the selected item */
      if ((wineItem == infoPtr->selectedItem) && inFocus)
      {
        gxDrawFocusRect(hdc,&rcText);
      }

    }
  }

  /* Draw insertion mark if necessary */

  if (infoPtr->insertMarkItem)
    TRACE("item:%d,mark:%p\n",
    TREEVIEW_GetItemIndex(infoPtr, wineItem),
    infoPtr->insertMarkItem);

  if (wineItem == infoPtr->insertMarkItem)
  {
    GXHPEN hNewPen, hOldPen;
    int offset;
    int left, right;

    hNewPen = gxCreatePen(GXPS_SOLID, 2, infoPtr->clrInsertMark);
    hOldPen = (GXHPEN)gxSelectObject(hdc, hNewPen);

    if (infoPtr->insertBeforeorAfter)
      offset = wineItem->rect.bottom - 1;
    else
      offset = wineItem->rect.top + 1;

    left = wineItem->textOffset - 2;
    right = wineItem->textOffset + wineItem->textWidth + 2;

    gxMoveToEx(hdc, left, offset - 3, NULL);
    gxLineTo(hdc, left, offset + 4);

    gxMoveToEx(hdc, left, offset, NULL);
    gxLineTo(hdc, right + 1, offset);

    gxMoveToEx(hdc, right, offset + 3, NULL);
    gxLineTo(hdc, right, offset - 4);

    gxSelectObject(hdc, hOldPen);
    gxDeleteObject(hNewPen);
  }

  if (cditem & CDRF_NOTIFYPOSTPAINT)
  {
    cditem = TREEVIEW_SendCustomDrawItemNotify
      (infoPtr, hdc, wineItem, CDDS_ITEMPOSTPAINT, &nmcdhdr);
    TRACE("postpaint:cditem-app returns 0x%x\n", cditem);
  }

  /* Restore the hdc state */
  gxSetTextColor(hdc, oldTextColor);
  gxSetBkColor(hdc, oldTextBkColor);
  gxSelectObject(hdc, hOldFont);
}

/* Computes treeHeight and treeWidth and updates the scroll bars.
*/
static void
TREEVIEW_UpdateScrollBars(GXTREEVIEW_INFO *infoPtr)
{
  GXTREEVIEW_ITEM *wineItem;
  GXHWND hwnd = infoPtr->hwnd;
  GXBOOL vert = FALSE;
  GXBOOL horz = FALSE;
  GXSCROLLINFO si;
  GXLONG scrollX = infoPtr->scrollX;

  infoPtr->treeWidth = 0;
  infoPtr->treeHeight = 0;

  /* We iterate through all visible items in order to get the tree height
  * and width */
  wineItem = infoPtr->root->firstChild;

  while (wineItem != NULL)
  {
    if (ISVISIBLE(wineItem))
    {
      /* actually we draw text at textOffset + 2 */
      if (2+wineItem->textOffset+wineItem->textWidth > infoPtr->treeWidth)
        infoPtr->treeWidth = wineItem->textOffset+wineItem->textWidth+2;

      /* This is scroll-adjusted, but we fix this below. */
      infoPtr->treeHeight = wineItem->rect.bottom;
    }

    wineItem = TREEVIEW_GetNextListItem(infoPtr, wineItem);
  }

  /* Fix the scroll adjusted treeHeight and treeWidth. */
  if (infoPtr->root->firstChild)
    infoPtr->treeHeight -= infoPtr->root->firstChild->rect.top;

  infoPtr->treeWidth += infoPtr->scrollX;

  if (infoPtr->dwStyle & TVS_NOSCROLL) return;

  /* Adding one scroll bar may take up enough space that it forces us
  * to add the other as well. */
  if (infoPtr->treeHeight > infoPtr->clientHeight)
  {
    vert = TRUE;

    if (infoPtr->treeWidth
  > infoPtr->clientWidth - gxGetSystemMetrics(GXSM_CXVSCROLL))
  horz = TRUE;
  }
  else if (infoPtr->treeWidth > infoPtr->clientWidth || infoPtr->scrollX > 0)
    horz = TRUE;

  if (!vert && horz && infoPtr->treeHeight
  > infoPtr->clientHeight - gxGetSystemMetrics(GXSM_CYVSCROLL))
  vert = TRUE;

  if (horz && (infoPtr->dwStyle & TVS_NOHSCROLL)) horz = FALSE;

  si.cbSize = sizeof(GXSCROLLINFO);
  si.fMask  = GXSIF_POS|GXSIF_RANGE|GXSIF_PAGE;
  si.nMin   = 0;

  if (vert)
  {
    si.nPage = TREEVIEW_GetVisibleCount(infoPtr);
    if ( si.nPage && NULL != infoPtr->firstVisible)
    {
      si.nPos  = infoPtr->firstVisible->visibleOrder;
      si.nMax  = infoPtr->maxVisibleOrder - 1;

      gxSetScrollInfo(hwnd, GXSB_VERT, &si, TRUE);

      if (!(infoPtr->uInternalStatus & GXTV_VSCROLL))
        gxShowScrollBar(hwnd, GXSB_VERT, TRUE);
      infoPtr->uInternalStatus |= GXTV_VSCROLL;
    }
    else
    {
      if (infoPtr->uInternalStatus & GXTV_VSCROLL)
        gxShowScrollBar(hwnd, GXSB_VERT, FALSE);
      infoPtr->uInternalStatus &= ~GXTV_VSCROLL;
    }
  }
  else
  {
    if (infoPtr->uInternalStatus & GXTV_VSCROLL)
      gxShowScrollBar(hwnd, GXSB_VERT, FALSE);
    infoPtr->uInternalStatus &= ~GXTV_VSCROLL;
  }

  if (horz)
  {
    si.nPage = infoPtr->clientWidth;
    si.nPos  = infoPtr->scrollX;
    si.nMax  = infoPtr->treeWidth - 1;

    if (si.nPos > si.nMax - max( si.nPage-1, 0 ))
    {
      si.nPos = si.nMax - max( si.nPage-1, 0 );
      scrollX = si.nPos;
    }

    if (!(infoPtr->uInternalStatus & GXTV_HSCROLL))
      gxShowScrollBar(hwnd, GXSB_HORZ, TRUE);
    infoPtr->uInternalStatus |= GXTV_HSCROLL;

    gxSetScrollInfo(hwnd, GXSB_HORZ, &si, TRUE);
    TREEVIEW_HScroll(infoPtr,
      GXMAKEWPARAM(GXSB_THUMBPOSITION, scrollX));
  }
  else
  {
    if (infoPtr->uInternalStatus & GXTV_HSCROLL)
      gxShowScrollBar(hwnd, GXSB_HORZ, FALSE);
    infoPtr->uInternalStatus &= ~GXTV_HSCROLL;

    scrollX = 0;
    if (infoPtr->scrollX != 0)
    {
      TREEVIEW_HScroll(infoPtr,
        GXMAKEWPARAM(GXSB_THUMBPOSITION, scrollX));
    }
  }

  if (!horz)
    infoPtr->uInternalStatus &= ~GXTV_HSCROLL;
}

/* CtrlSpy doesn't mention this, but CorelDRAW's object manager needs it. */
static GXLRESULT
TREEVIEW_EraseBackground(const GXTREEVIEW_INFO *infoPtr, GXHDC hDC)
{
  GXHBRUSH hBrush = gxCreateSolidBrush(infoPtr->clrBk);
  GXRECT rect;

  gxGetClientRect(infoPtr->hwnd, &rect);
  gxFillRect(hDC, &rect, hBrush);
  gxDeleteObject(hBrush);

  return 1;
}

static void
TREEVIEW_Refresh(GXTREEVIEW_INFO *infoPtr, GXHDC hdc, const GXRECT *rc)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXRECT rect = *rc;
  GXTREEVIEW_ITEM *wineItem;

  if (infoPtr->clientHeight == 0 || infoPtr->clientWidth == 0)
  {
    TRACE("empty window\n");
    return;
  }

  infoPtr->cdmode = TREEVIEW_SendCustomDrawNotify(infoPtr, CDDS_PREPAINT,
    hdc, rect);

  if (infoPtr->cdmode == CDRF_SKIPDEFAULT)
  {
    gxReleaseDC(hwnd, hdc);
    return;
  }

  for (wineItem = infoPtr->root->firstChild;
    wineItem != NULL;
    wineItem = TREEVIEW_GetNextListItem(infoPtr, wineItem))
  {
    if (ISVISIBLE(wineItem))
    {
      /* Avoid unneeded calculations */
      if (wineItem->rect.top > rect.bottom)
        break;
      if (wineItem->rect.bottom < rect.top)
        continue;

      TREEVIEW_DrawItem(infoPtr, hdc, wineItem);
    }
  }

  TREEVIEW_UpdateScrollBars(infoPtr);

  if (infoPtr->cdmode & CDRF_NOTIFYPOSTPAINT)
    infoPtr->cdmode =
    TREEVIEW_SendCustomDrawNotify(infoPtr, CDDS_POSTPAINT, hdc, rect);
}

static void
TREEVIEW_Invalidate(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *item)
{
  if (item != NULL)
    gxInvalidateRect(infoPtr->hwnd, &item->rect, TRUE);
  else
    gxInvalidateRect(infoPtr->hwnd, NULL, TRUE);
}

static GXLRESULT
TREEVIEW_Paint(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  GXHDC hdc;
  GXPAINTSTRUCT ps;
  GXRECT rc;

  TRACE("\n");

  if (wParam)
  {
    hdc = (GXHDC)wParam;
    gxGetClientRect(infoPtr->hwnd, &rc);        
    TREEVIEW_EraseBackground(infoPtr, hdc);
  }
  else
  {
    hdc = gxBeginPaint(infoPtr->hwnd, &ps);
    rc = ps.rcPaint;
  }

  if(infoPtr->bRedraw) /* WM_SETREDRAW sets bRedraw */
    TREEVIEW_Refresh(infoPtr, hdc, &rc);

  if (!wParam)
    gxEndPaint(infoPtr->hwnd, &ps);

  return 0;
}


/* Sorting **************************************************************/

/***************************************************************************
* Forward the GXDPA local callback to the treeview owner callback
*/
static GXINT GXAPI
TREEVIEW_CallBackCompare(const GXTREEVIEW_ITEM *first, const GXTREEVIEW_ITEM *second,
             const GXTVSORTCB *pCallBackSort)
{
  /* Forward the call to the client-defined callback */
  return pCallBackSort->lpfnCompare(first->lParam,
    second->lParam,
    pCallBackSort->lParam);
}

/***************************************************************************
* Treeview native sort routine: sort on item text.
*/
static GXINT GXAPI
TREEVIEW_SortOnName(GXTREEVIEW_ITEM *first, GXTREEVIEW_ITEM *second,
          const GXTREEVIEW_INFO *infoPtr)
{
  TREEVIEW_UpdateDispInfo(infoPtr, first, GXTVIF_TEXT);
  TREEVIEW_UpdateDispInfo(infoPtr, second, GXTVIF_TEXT);

  if(first->pszText && second->pszText)
    return GXSTRCMPI(first->pszText, second->pszText);
  else if(first->pszText)
    return -1;
  else if(second->pszText)
    return 1;
  else
    return 0;
}

/* Returns the number of physical children belonging to item. */
static GXINT
TREEVIEW_CountChildren(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *item)
{
  GXINT cChildren = 0;
  GXHTREEITEM hti;

  for (hti = item->firstChild; hti != NULL; hti = hti->nextSibling)
    cChildren++;

  return cChildren;
}

/* Returns a GXDPA containing a pointer to each physical child of item in
* sibling order. If item has no children, an empty GXDPA is returned. */
static GXHDPA
TREEVIEW_BuildChildDPA(const GXTREEVIEW_INFO *infoPtr, const GXTREEVIEW_ITEM *item)
{
  GXHTREEITEM child = item->firstChild;

  GXHDPA list = gxDPA_Create(8);
  if (list == 0) return NULL;

  for (child = item->firstChild; child != NULL; child = child->nextSibling)
  {
    if (gxDPA_InsertPtr(list, INT_MAX, child) == -1)
    {
      gxDPA_Destroy(list);
      return NULL;
    }
  }

  return list;
}

/***************************************************************************
* Setup the treeview structure with regards of the sort method
* and sort the children of the TV item specified in lParam
* fRecurse: currently unused. Should be zero.
* parent: if pSort!=NULL, should equal pSort->hParent.
*         otherwise, item which child items are to be sorted.
* pSort:  sort method info. if NULL, sort on item text.
*         if non-NULL, sort on item's lParam content, and let the
*         application decide what that means. See also GXTVM_SORTCHILDRENCB.
*/

static GXLRESULT
TREEVIEW_Sort(GXTREEVIEW_INFO *infoPtr, GXBOOL fRecurse, GXHTREEITEM parent,
        GXLPTVSORTCB pSort)
{
  GXINT cChildren;
  GXPFNDPACOMPARE pfnCompare;
  GXLPARAM lpCompare;

  /* undocumented feature: TVI_ROOT or NULL means `sort the whole tree' */
  if (parent == GXTVI_ROOT || parent == NULL)
    parent = infoPtr->root;

  /* Check for a valid handle to the parent item */
  if (!TREEVIEW_ValidItem(infoPtr, parent))
  {
    ERR("invalid item hParent=%p\n", parent);
    return FALSE;
  }

  if (pSort)
  {
    pfnCompare = (GXPFNDPACOMPARE)TREEVIEW_CallBackCompare;
    lpCompare = (GXLPARAM)pSort;
  }
  else
  {
    pfnCompare = (GXPFNDPACOMPARE)TREEVIEW_SortOnName;
    lpCompare = (GXLPARAM)infoPtr;
  }

  cChildren = TREEVIEW_CountChildren(infoPtr, parent);

  /* Make sure there is something to sort */
  if (cChildren > 1)
  {
    /* GXTREEVIEW_ITEM rechaining */
    GXINT count = 0;
    GXHTREEITEM item = 0;
    GXHTREEITEM nextItem = 0;
    GXHTREEITEM prevItem = 0;

    GXHDPA sortList = TREEVIEW_BuildChildDPA(infoPtr, parent);

    if (sortList == NULL)
      return FALSE;

    /* let GXDPA sort the list */
    gxDPA_Sort(sortList, pfnCompare, lpCompare);

    /* The order of GXDPA entries has been changed, so fixup the
    * nextSibling and prevSibling pointers. */

    item = (GXHTREEITEM)gxDPA_GetPtr(sortList, count++);
    while ((nextItem = (GXHTREEITEM)gxDPA_GetPtr(sortList, count++)) != NULL)
    {
      /* link the two current item toghether */
      item->nextSibling = nextItem;
      nextItem->prevSibling = item;

      if (prevItem == NULL)
      {
        /* this is the first item, update the parent */
        parent->firstChild = item;
        item->prevSibling = NULL;
      }
      else
      {
        /* fix the back chaining */
        item->prevSibling = prevItem;
      }

      /* get ready for the next one */
      prevItem = item;
      item = nextItem;
    }

    /* the last item is pointed to by item and never has a sibling */
    item->nextSibling = NULL;
    parent->lastChild = item;

    gxDPA_Destroy(sortList);

    TREEVIEW_VerifyTree(infoPtr);

    if (parent->state & TVIS_EXPANDED)
    {
      int visOrder = infoPtr->firstVisible->visibleOrder;

      if (parent == infoPtr->root)
        TREEVIEW_RecalculateVisibleOrder(infoPtr, NULL);
      else
        TREEVIEW_RecalculateVisibleOrder(infoPtr, parent);

      if (TREEVIEW_IsChildOf(parent, infoPtr->firstVisible))
      {
        GXTREEVIEW_ITEM *item;

        for (item = infoPtr->root->firstChild; item != NULL;
          item = TREEVIEW_GetNextListItem(infoPtr, item))
        {
          if (item->visibleOrder == visOrder)
            break;
        }

        if (!item) item = parent->firstChild;
        TREEVIEW_SetFirstVisible(infoPtr, item, FALSE);
      }

      TREEVIEW_Invalidate(infoPtr, NULL);
    }

    return TRUE;
  }
  return FALSE;
}


/***************************************************************************
* Setup the treeview structure with regards of the sort method
* and sort the children of the TV item specified in lParam
*/
static GXLRESULT
TREEVIEW_SortChildrenCB(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPTVSORTCB pSort)
{
  return TREEVIEW_Sort(infoPtr, wParam, pSort->hParent, pSort);
}


/***************************************************************************
* Sort the children of the TV item specified in lParam.
*/
static GXLRESULT
TREEVIEW_SortChildren(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  return TREEVIEW_Sort(infoPtr, (GXBOOL)wParam, (GXHTREEITEM)lParam, NULL);
}


/* Expansion/Collapse ***************************************************/

static GXBOOL
TREEVIEW_SendExpanding(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
             GXUINT action)
{
  return !TREEVIEW_SendTreeviewNotify(infoPtr, GXTVN_ITEMEXPANDINGW, action,
    GXTVIF_HANDLE | GXTVIF_STATE | GXTVIF_PARAM
    | GXTVIF_IMAGE | GXTVIF_SELECTEDIMAGE,
    0, wineItem);
}

static GXVOID
TREEVIEW_SendExpanded(const GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
            GXUINT action)
{
  TREEVIEW_SendTreeviewNotify(infoPtr, GXTVN_ITEMEXPANDEDW, action,
    GXTVIF_HANDLE | GXTVIF_STATE | GXTVIF_PARAM
    | GXTVIF_IMAGE | GXTVIF_SELECTEDIMAGE,
    0, wineItem);
}


/* This corresponds to GXTVM_EXPAND with TVE_COLLAPSE.
* bRemoveChildren corresponds to TVE_COLLAPSERESET. */
static GXBOOL
TREEVIEW_Collapse(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
          GXBOOL bRemoveChildren, GXBOOL bUser)
{
  GXUINT action = TVE_COLLAPSE | (bRemoveChildren ? TVE_COLLAPSERESET : 0);
  GXBOOL bSetSelection, bSetFirstVisible;
  GXRECT scrollRect;
  GXLONG scrollDist = 0;
  GXTREEVIEW_ITEM *nextItem = NULL, *tmpItem;

  TRACE("TVE_COLLAPSE %p %s\n", wineItem, TREEVIEW_ItemName(wineItem));

  if (!(wineItem->state & TVIS_EXPANDED))
    return FALSE;

  if (bUser || !(wineItem->state & TVIS_EXPANDEDONCE))
    TREEVIEW_SendExpanding(infoPtr, wineItem, action);

  if (wineItem->firstChild == NULL)
    return FALSE;

  wineItem->state &= ~TVIS_EXPANDED;

  if (bUser || !(wineItem->state & TVIS_EXPANDEDONCE))
    TREEVIEW_SendExpanded(infoPtr, wineItem, action);

  bSetSelection = (infoPtr->selectedItem != NULL
    && TREEVIEW_IsChildOf(wineItem, infoPtr->selectedItem));

  bSetFirstVisible = (infoPtr->firstVisible != NULL
    && TREEVIEW_IsChildOf(wineItem, infoPtr->firstVisible));

  tmpItem = wineItem;
  while (tmpItem)
  {
    if (tmpItem->nextSibling)
    {
      nextItem = tmpItem->nextSibling;
      break;
    }
    tmpItem = tmpItem->parent;
  }

  if (nextItem)
    scrollDist = nextItem->rect.top;

  if (bRemoveChildren)
  {
    GXINT old_cChildren = wineItem->cChildren;
    TRACE("TVE_COLLAPSERESET\n");
    wineItem->state &= ~TVIS_EXPANDEDONCE;
    TREEVIEW_RemoveAllChildren(infoPtr, wineItem);
    wineItem->cChildren = old_cChildren;
  }

  if (wineItem->firstChild)
  {
    GXTREEVIEW_ITEM *item, *sibling;

    sibling = TREEVIEW_GetNextListItem(infoPtr, wineItem);

    for (item = wineItem->firstChild; item != sibling;
      item = TREEVIEW_GetNextListItem(infoPtr, item))
    {
      item->visibleOrder = -1;
    }
  }

  TREEVIEW_RecalculateVisibleOrder(infoPtr, wineItem);

  if (nextItem)
    scrollDist = -(scrollDist - nextItem->rect.top);

  if (bSetSelection)
  {
    /* Don't call DoSelectItem, it sends notifications. */
    if (TREEVIEW_ValidItem(infoPtr, infoPtr->selectedItem))
      infoPtr->selectedItem->state &= ~TVIS_SELECTED;
    wineItem->state |= TVIS_SELECTED;
    infoPtr->selectedItem = wineItem;
  }

  TREEVIEW_UpdateScrollBars(infoPtr);

  scrollRect.left = 0;
  scrollRect.right = infoPtr->clientWidth;
  scrollRect.bottom = infoPtr->clientHeight;

  if (nextItem)
  {
    scrollRect.top = nextItem->rect.top;

    gxScrollWindowEx (infoPtr->hwnd, 0, scrollDist, &scrollRect, NULL,
      NULL, NULL, GXSW_ERASE | GXSW_INVALIDATE);
    TREEVIEW_Invalidate(infoPtr, wineItem);
  } else {
    scrollRect.top = wineItem->rect.top;
    gxInvalidateRect(infoPtr->hwnd, &scrollRect, TRUE);
  }

  TREEVIEW_SetFirstVisible(infoPtr,
    bSetFirstVisible ? wineItem : infoPtr->firstVisible,
    TRUE);

  return TRUE;
}

static GXBOOL
TREEVIEW_Expand(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem,
        GXBOOL bExpandPartial, GXBOOL bUser)
{
  GXLONG scrollDist;
  GXLONG orgNextTop = 0;
  GXRECT scrollRect;
  GXTREEVIEW_ITEM *nextItem, *tmpItem;

  TRACE("\n");

  if (wineItem->state & TVIS_EXPANDED)
    return TRUE;

  tmpItem = wineItem; nextItem = NULL;
  while (tmpItem)
  {
    if (tmpItem->nextSibling)
    {
      nextItem = tmpItem->nextSibling;
      break;
    }
    tmpItem = tmpItem->parent;
  }

  if (nextItem)
    orgNextTop = nextItem->rect.top;

  TRACE("TVE_EXPAND %p %s\n", wineItem, TREEVIEW_ItemName(wineItem));

  if (bUser || ((wineItem->cChildren != 0) &&
    !(wineItem->state & TVIS_EXPANDEDONCE)))
  {
    if (!TREEVIEW_SendExpanding(infoPtr, wineItem, TVE_EXPAND))
    {
      TRACE("  GXTVN_ITEMEXPANDING returned TRUE, exiting...\n");
      return FALSE;
    }

    if (!wineItem->firstChild)
      return FALSE;

    wineItem->state |= TVIS_EXPANDED;
    TREEVIEW_SendExpanded(infoPtr, wineItem, TVE_EXPAND);
    wineItem->state |= TVIS_EXPANDEDONCE;
  }
  else
  {
    if (!wineItem->firstChild)
      return FALSE;

    /* this item has already been expanded */
    wineItem->state |= TVIS_EXPANDED;
  }

  if (bExpandPartial)
    FIXME("TVE_EXPANDPARTIAL not implemented\n");

  TREEVIEW_RecalculateVisibleOrder(infoPtr, wineItem);
  TREEVIEW_UpdateSubTree(infoPtr, wineItem);
  TREEVIEW_UpdateScrollBars(infoPtr);

  scrollRect.left = 0;
  scrollRect.bottom = infoPtr->treeHeight;
  scrollRect.right = infoPtr->clientWidth;
  if (nextItem)
  {
    scrollDist = nextItem->rect.top - orgNextTop;
    scrollRect.top = orgNextTop;

    gxScrollWindowEx (infoPtr->hwnd, 0, scrollDist, &scrollRect, NULL,
      NULL, NULL, GXSW_ERASE | GXSW_INVALIDATE);
    TREEVIEW_Invalidate (infoPtr, wineItem);
  } else {
    scrollRect.top = wineItem->rect.top;
    gxInvalidateRect(infoPtr->hwnd, &scrollRect, FALSE);
  }

  /* Scroll up so that as many children as possible are visible.
  * This fails when expanding causes an HScroll bar to appear, but we
  * don't know that yet, so the last item is obscured. */
  if (wineItem->firstChild != NULL)
  {
    int nChildren = wineItem->lastChild->visibleOrder
      - wineItem->firstChild->visibleOrder + 1;

    int visible_pos = wineItem->visibleOrder
      - infoPtr->firstVisible->visibleOrder;

    int rows_below = TREEVIEW_GetVisibleCount(infoPtr) - visible_pos - 1;

    if (visible_pos > 0 && nChildren > rows_below)
    {
      int scroll = nChildren - rows_below;

      if (scroll > visible_pos)
        scroll = visible_pos;

      if (scroll > 0)
      {
        GXTREEVIEW_ITEM *newFirstVisible
          = TREEVIEW_GetListItem(infoPtr, infoPtr->firstVisible,
          scroll);


        TREEVIEW_SetFirstVisible(infoPtr, newFirstVisible, TRUE);
      }
    }
  }

  return TRUE;
}

static GXBOOL
TREEVIEW_Toggle(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *wineItem, GXBOOL bUser)
{
  TRACE("\n");

  if (wineItem->state & TVIS_EXPANDED)
    return TREEVIEW_Collapse(infoPtr, wineItem, FALSE, bUser);
  else
    return TREEVIEW_Expand(infoPtr, wineItem, FALSE, bUser);
}

static GXVOID
TREEVIEW_ExpandAll(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  TREEVIEW_Expand(infoPtr, item, FALSE, TRUE);

  for (item = item->firstChild; item != NULL; item = item->nextSibling)
  {
    if (TREEVIEW_HasChildren(infoPtr, item))
      TREEVIEW_ExpandAll(infoPtr, item);
  }
}

/* Note:If the specified item is the child of a collapsed parent item,
the parent's list of child items is (recursively) expanded to reveal the
specified item. This is mentioned for TREEVIEW_SelectItem; don't
know if it also applies here.
*/

static GXLRESULT
TREEVIEW_ExpandMsg(GXTREEVIEW_INFO *infoPtr, GXUINT flag, GXHTREEITEM wineItem)
{
  if (!TREEVIEW_ValidItem(infoPtr, wineItem))
    return 0;

  TRACE("For (%s) item:%d, flags %x, state:%d\n",
    TREEVIEW_ItemName(wineItem), flag,
    TREEVIEW_GetItemIndex(infoPtr, wineItem), wineItem->state);

  switch (flag & TVE_TOGGLE)
  {
  case TVE_COLLAPSE:
    return TREEVIEW_Collapse(infoPtr, wineItem, flag & TVE_COLLAPSERESET,
      FALSE);

  case TVE_EXPAND:
    return TREEVIEW_Expand(infoPtr, wineItem, flag & TVE_EXPANDPARTIAL,
      FALSE);

  case TVE_TOGGLE:
    return TREEVIEW_Toggle(infoPtr, wineItem, TRUE);

  default:
    return 0;
  }

#if 0
  TRACE("Exiting, Item %p state is now %d...\n", wineItem, wineItem->state);
#endif
}

/* Hit-Testing **********************************************************/

static GXTREEVIEW_ITEM *
TREEVIEW_HitTestPoint(const GXTREEVIEW_INFO *infoPtr, GXPOINT pt)
{
  GXTREEVIEW_ITEM *wineItem;
  GXLONG row;

  if (!infoPtr->firstVisible)
    return NULL;

  row = pt.y / infoPtr->uItemHeight + infoPtr->firstVisible->visibleOrder;

  for (wineItem = infoPtr->firstVisible; wineItem != NULL;
    wineItem = TREEVIEW_GetNextListItem(infoPtr, wineItem))
  {
    if (row >= wineItem->visibleOrder
      && row < wineItem->visibleOrder + wineItem->iIntegral)
      break;
  }

  return wineItem;
}

static GXLRESULT
TREEVIEW_HitTest(const GXTREEVIEW_INFO *infoPtr, GXLPTVHITTESTINFO lpht)
{
  GXTREEVIEW_ITEM *wineItem;
  GXRECT rect;
  GXUINT status;
  GXLONG x, y;

  lpht->hItem = 0;
  gxGetClientRect(infoPtr->hwnd, &rect);
  status = 0;
  x = lpht->pt.x;
  y = lpht->pt.y;

  if (x < rect.left)
  {
    status |= TVHT_TOLEFT;
  }
  else if (x > rect.right)
  {
    status |= TVHT_TORIGHT;
  }

  if (y < rect.top)
  {
    status |= TVHT_ABOVE;
  }
  else if (y > rect.bottom)
  {
    status |= TVHT_BELOW;
  }

  if (status)
  {
    lpht->flags = status;
    return (GXLRESULT)(GXHTREEITEM)NULL;
  }

  wineItem = TREEVIEW_HitTestPoint(infoPtr, lpht->pt);
  if (!wineItem)
  {
    lpht->flags = TVHT_NOWHERE;
    return (GXLRESULT)(GXHTREEITEM)NULL;
  }

  if (x >= wineItem->textOffset + wineItem->textWidth)
  {
    lpht->flags = TVHT_ONITEMRIGHT;
  }
  else if (x >= wineItem->textOffset)
  {
    lpht->flags = TVHT_ONITEMLABEL;
  }
  else if (x >= wineItem->imageOffset)
  {
    lpht->flags = TVHT_ONITEMICON;
  }
  else if (x >= wineItem->stateOffset)
  {
    lpht->flags = TVHT_ONITEMSTATEICON;
  }
  else if (x >= wineItem->linesOffset && infoPtr->dwStyle & TVS_HASBUTTONS)
  {
    lpht->flags = TVHT_ONITEMBUTTON;
  }
  else
  {
    lpht->flags = TVHT_ONITEMINDENT;
  }

  lpht->hItem = wineItem;
  TRACE("(%d,%d):result %x\n", lpht->pt.x, lpht->pt.y, lpht->flags);

  return (GXLRESULT)wineItem;
}

/* Item Label Editing ***************************************************/

static GXLRESULT
TREEVIEW_GetEditControl(const GXTREEVIEW_INFO *infoPtr)
{
  return (GXLRESULT)infoPtr->hwndEdit;
}

static GXLRESULT GXCALLBACK
TREEVIEW_Edit_SubclassProc(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  GXTREEVIEW_INFO *infoPtr = TREEVIEW_GetInfoPtr(gxGetParent(hwnd));
  GXBOOL bCancel = FALSE;
  GXLRESULT rc;

  switch (uMsg)
  {
  case GXWM_PAINT:
    TRACE("WM_PAINT start\n");
    rc = gxCallWindowProcW(infoPtr->wpEditOrig, hwnd, uMsg, wParam,
      lParam);
    TRACE("WM_PAINT done\n");
    return rc;

  case GXWM_KILLFOCUS:
    if (infoPtr->bIgnoreEditKillFocus)
      return TRUE;
    break;

  case GXWM_GETDLGCODE:
    return GXDLGC_WANTARROWS | GXDLGC_WANTALLKEYS;

  case GXWM_KEYDOWN:
    if (wParam == (GXWPARAM)GXVK_ESCAPE)
    {
      bCancel = TRUE;
      break;
    }
    else if (wParam == (GXWPARAM)GXVK_RETURN)
    {
      break;
    }

    /* fall through */
  default:
    return gxCallWindowProcW(infoPtr->wpEditOrig, hwnd, uMsg, wParam, lParam);
  }

  /* Processing TVN_ENDLABELEDIT message could kill the focus       */
  /* eg. Using a messagebox                                         */

  infoPtr->bIgnoreEditKillFocus = TRUE;
  TREEVIEW_EndEditLabelNow(infoPtr, bCancel || !infoPtr->bLabelChanged);
  infoPtr->bIgnoreEditKillFocus = FALSE;

  return 0;
}


/* should handle edit control messages here */

static GXLRESULT
TREEVIEW_Command(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("%lx %ld\n", wParam, lParam);

  switch (GXHIWORD(wParam))
  {
  case GXEN_UPDATE:
    {
      /*
      * Adjust the edit window size
      */
      GXWCHAR buffer[1024];
      GXTREEVIEW_ITEM *editItem = infoPtr->selectedItem;
      GXHDC hdc = gxGetDC(infoPtr->hwndEdit);
      GXSIZE sz;
      int len;
      GXHFONT hFont, hOldFont = 0;

      infoPtr->bLabelChanged = TRUE;

      len = gxGetWindowTextW(infoPtr->hwndEdit, buffer, sizeof(buffer) / sizeof(buffer[0]));

      /* Select font to get the right dimension of the string */
      hFont = (GXHFONT)gxSendMessageW(infoPtr->hwndEdit, GXWM_GETFONT, 0, 0);

      if (hFont != 0)
      {
        hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);
      }

      if (gxGetTextExtentPoint32W(hdc, buffer, GXSTRLEN(buffer), &sz))
      {
        GXTEXTMETRICW textMetric;

        /* Add Extra spacing for the next character */
        gxGetTextMetricsW(hdc, &textMetric);
        sz.cx += (textMetric.tmMaxCharWidth * 2);

        sz.cx = max(sz.cx, textMetric.tmMaxCharWidth * 3);
        sz.cx = min(sz.cx,
          infoPtr->clientWidth - editItem->textOffset + 2);

        gxSetWindowPos(infoPtr->hwndEdit,
          GXHWND_TOP,
          0,
          0,
          sz.cx,
          editItem->rect.bottom - editItem->rect.top + 3,
          GXSWP_NOMOVE | GXSWP_DRAWFRAME);
      }

      if (hFont != 0)
      {
        gxSelectObject(hdc, hOldFont);
      }

      gxReleaseDC(infoPtr->hwnd, hdc);
      break;
    }

  default:
    return gxSendMessageW(infoPtr->hwndNotify, GXWM_COMMAND, wParam, lParam);
  }

  return 0;
}

static GXHWND
TREEVIEW_EditLabel(GXTREEVIEW_INFO *infoPtr, GXHTREEITEM hItem)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXHWND hwndEdit;
  GXSIZE sz;
  GXTREEVIEW_ITEM *editItem = hItem;
  GXHINSTANCE hinst = (GXHINSTANCE)gxGetWindowLongPtrW(hwnd, GXGWLP_HINSTANCE);
  GXHDC hdc;
  GXHFONT hOldFont=0;
  GXTEXTMETRICW textMetric;
  static const GXWCHAR EditW[] = {'E','d','i','t',0};

  TRACE("%p %p\n", hwnd, hItem);
  if (!TREEVIEW_ValidItem(infoPtr, editItem))
    return NULL;

  if (infoPtr->hwndEdit)
    return infoPtr->hwndEdit;

  infoPtr->bLabelChanged = FALSE;

  /* Make sure that edit item is selected */
  TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, hItem, TVC_UNKNOWN);
  TREEVIEW_EnsureVisible(infoPtr, hItem, TRUE);

  TREEVIEW_UpdateDispInfo(infoPtr, editItem, GXTVIF_TEXT);

  hdc = gxGetDC(hwnd);
  /* Select the font to get appropriate metric dimensions */
  if (infoPtr->hFont != 0)
  {
    hOldFont = (GXHFONT)gxSelectObject(hdc, infoPtr->hFont);
  }

  /* Get string length in pixels */
  gxGetTextExtentPoint32W(hdc, editItem->pszText, GXSTRLEN(editItem->pszText),
    &sz);

  /* Add Extra spacing for the next character */
  gxGetTextMetricsW(hdc, &textMetric);
  sz.cx += (textMetric.tmMaxCharWidth * 2);

  sz.cx = max(sz.cx, textMetric.tmMaxCharWidth * 3);
  sz.cx = min(sz.cx, infoPtr->clientWidth - editItem->textOffset + 2);

  if (infoPtr->hFont != 0)
  {
    gxSelectObject(hdc, hOldFont);
  }

  gxReleaseDC(hwnd, hdc);
  hwndEdit = gxCreateWindowExW(GXWS_EX_LEFT,
    EditW,
    0,
    GXWS_CHILD | GXWS_BORDER | GXES_AUTOHSCROLL |
    GXWS_CLIPSIBLINGS | GXES_WANTRETURN |
    GXES_LEFT, editItem->textOffset - 2,
    editItem->rect.top - 1, sz.cx + 3,
    editItem->rect.bottom -
    editItem->rect.top + 3, hwnd, 0, hinst, 0);
  /* FIXME: (HMENU)IDTVEDIT,pcs->hInstance,0); */

  infoPtr->hwndEdit = hwndEdit;

  /* Get a 2D border. */
  gxSetWindowLongW(hwndEdit, GXGWL_EXSTYLE,
    gxGetWindowLongW(hwndEdit, GXGWL_EXSTYLE) & ~GXWS_EX_CLIENTEDGE);
  gxSetWindowLongW(hwndEdit, GXGWL_STYLE,
    gxGetWindowLongW(hwndEdit, GXGWL_STYLE) | GXWS_BORDER);

  gxSendMessageW(hwndEdit, GXWM_SETFONT,
    (GXWPARAM)TREEVIEW_FontForItem(infoPtr, editItem), FALSE);

  infoPtr->wpEditOrig = (GXWNDPROC)gxSetWindowLongPtrW(hwndEdit, GXGWLP_WNDPROC,
    (GXDWORD_PTR)
    TREEVIEW_Edit_SubclassProc);

  if (TREEVIEW_BeginLabelEditNotify(infoPtr, editItem))
  {
    gxDestroyWindow(hwndEdit);
    infoPtr->hwndEdit = 0;
    return NULL;
  }

  infoPtr->selectedItem = hItem;
  gxSetWindowTextW(hwndEdit, editItem->pszText);
  gxSetFocus(hwndEdit);
  gxSendMessageW(hwndEdit, GXEM_SETSEL, 0, -1);
  gxShowWindow(hwndEdit, GXSW_SHOW);

  return hwndEdit;
}


static GXLRESULT
TREEVIEW_EndEditLabelNow(GXTREEVIEW_INFO *infoPtr, GXBOOL bCancel)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXTREEVIEW_ITEM *editedItem = infoPtr->selectedItem;
  GXNMTVDISPINFOW tvdi;
  GXBOOL bCommit;
  GXWCHAR tmpText[1024] = { '\0' };
  GXWCHAR *newText = tmpText;
  int iLength = 0;

  if (!infoPtr->hwndEdit)
    return FALSE;

  tvdi.hdr.hwndFrom = hwnd;
  tvdi.hdr.idFrom = gxGetWindowLongPtrW(hwnd, GXGWLP_ID);
  tvdi.hdr.code = get_notifycode(infoPtr, GXTVN_ENDLABELEDITW);
  tvdi.item.mask = 0;
  tvdi.item.hItem = editedItem;
  tvdi.item.state = editedItem->state;
  tvdi.item.lParam = editedItem->lParam;

  if (!bCancel)
  {
    if (!infoPtr->bNtfUnicode)
      iLength = gxGetWindowTextA(infoPtr->hwndEdit, (GXLPSTR)tmpText, 1023);
    else
      iLength = gxGetWindowTextW(infoPtr->hwndEdit, tmpText, 1023);

    if (iLength >= 1023)
    {
      ERR("Insufficient space to retrieve new item label\n");
    }

    tvdi.item.mask = GXTVIF_TEXT;
    tvdi.item.pszText = tmpText;
    tvdi.item.cchTextMax = iLength + 1;
  }
  else
  {
    tvdi.item.pszText = NULL;
    tvdi.item.cchTextMax = 0;
  }

  bCommit = (GXBOOL)TREEVIEW_SendRealNotify(infoPtr,
    (GXWPARAM)tvdi.hdr.idFrom, (GXLPARAM)&tvdi);

  if (!bCancel && bCommit)  /* Apply the changes */
  {
    if (!infoPtr->bNtfUnicode)
    {
      GXDWORD len = gxMultiByteToWideChar( GXCP_ACP, 0, (GXLPSTR)tmpText, -1, NULL, 0 );
      newText = (GXWCHAR*)Alloc(len * sizeof(GXWCHAR));
      gxMultiByteToWideChar( GXCP_ACP, 0, (GXLPSTR)tmpText, -1, newText, len );
      iLength = len - 1;
    }

    if (GXSTRCMP(newText, editedItem->pszText) != 0)
    {
      GXWCHAR *ptr = (GXWCHAR *)ReAlloc(editedItem->pszText, sizeof(GXWCHAR)*(iLength + 1));
      if (ptr == NULL)
      {
        ERR("OutOfMemory, cannot allocate space for label\n");
        if(newText != tmpText) Free(newText);
        gxDestroyWindow(infoPtr->hwndEdit);
        infoPtr->hwndEdit = 0;
        return FALSE;
      }
      else
      {
        editedItem->pszText = ptr;
        editedItem->cchTextMax = iLength + 1;
        GXSTRCPY(editedItem->pszText, newText);
        TREEVIEW_ComputeTextWidth(infoPtr, editedItem, 0);
      }
    }
    if(newText != tmpText) Free(newText);
  }

  gxShowWindow(infoPtr->hwndEdit, GXSW_HIDE);
  gxDestroyWindow(infoPtr->hwndEdit);
  infoPtr->hwndEdit = 0;
  return TRUE;
}

static GXLRESULT
TREEVIEW_HandleTimer(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  if (wParam != GXTV_EDIT_TIMER)
  {
    ERR("got unknown timer\n");
    return 1;
  }

  gxKillTimer(infoPtr->hwnd, GXTV_EDIT_TIMER);
  infoPtr->Timer &= ~GXTV_EDIT_TIMER_SET;

  TREEVIEW_EditLabel(infoPtr, infoPtr->selectedItem);

  return 0;
}


/* Mouse Tracking/Drag **************************************************/

/***************************************************************************
* This is quite unusual piece of code, but that's how it's implemented in
* Windows.
*/
static GXLRESULT
TREEVIEW_TrackMouse(const GXTREEVIEW_INFO *infoPtr, GXPOINT pt)
{
  GXINT cxDrag = gxGetSystemMetrics(GXSM_CXDRAG);
  GXINT cyDrag = gxGetSystemMetrics(GXSM_CYDRAG);
  GXRECT r;
  GXMSG msg;

  r.top = pt.y - cyDrag;
  r.left = pt.x - cxDrag;
  r.bottom = pt.y + cyDrag;
  r.right = pt.x + cxDrag;

  gxSetCapture(infoPtr->hwnd);

  while (1)
  {
    if (gxPeekMessageW(&msg, 0, 0, 0, GXPM_REMOVE | GXPM_NOYIELD))
    {
      if (msg.message == GXWM_MOUSEMOVE)
      {
        pt.x = (short)GXLOWORD(msg.lParam);
        pt.y = (short)GXHIWORD(msg.lParam);
        if (gxPtInRect(&r, pt))
          continue;
        else
        {
          gxReleaseCapture();
          return 1;
        }
      }
      else if (msg.message >= GXWM_LBUTTONDOWN &&
        msg.message <= GXWM_RBUTTONDBLCLK)
      {
        if (msg.message == GXWM_RBUTTONUP)
          TREEVIEW_RButtonUp(infoPtr, &pt);
        break;
      }

      gxDispatchMessageW(&msg);
    }

    if (gxGetCapture() != infoPtr->hwnd)
      return 0;
  }

  gxReleaseCapture();
  return 0;
}


static GXLRESULT
TREEVIEW_LButtonDoubleClick(GXTREEVIEW_INFO *infoPtr, GXLPARAM lParam)
{
  GXTREEVIEW_ITEM *wineItem;
  GXTVHITTESTINFO hit;

  TRACE("\n");
  gxSetFocus(infoPtr->hwnd);

  if (infoPtr->Timer & GXTV_EDIT_TIMER_SET)
  {
    /* If there is pending 'edit label' event - kill it now */
    gxKillTimer(infoPtr->hwnd, GXTV_EDIT_TIMER);
  }

  hit.pt.x = (short)GXLOWORD(lParam);
  hit.pt.y = (short)GXHIWORD(lParam);

  wineItem = (GXTREEVIEW_ITEM *)TREEVIEW_HitTest(infoPtr, &hit);
  if (!wineItem)
    return 0;
  TRACE("item %d\n", TREEVIEW_GetItemIndex(infoPtr, wineItem));

  if (TREEVIEW_SendSimpleNotify(infoPtr, GXNM_DBLCLK) == FALSE)
  {        /* FIXME! */
    switch (hit.flags)
    {
    case TVHT_ONITEMRIGHT:
      /* FIXME: we should not have sent NM_DBLCLK in this case. */
      break;

    case TVHT_ONITEMINDENT:
      if (!(infoPtr->dwStyle & TVS_HASLINES))
      {
        break;
      }
      else
      {
        int level = hit.pt.x / infoPtr->uIndent;
        if (!(infoPtr->dwStyle & TVS_LINESATROOT)) level++;

        while (wineItem->iLevel > level)
        {
          wineItem = wineItem->parent;
        }

        /* fall through */
      }

    case TVHT_ONITEMLABEL:
    case TVHT_ONITEMICON:
    case TVHT_ONITEMBUTTON:
      TREEVIEW_Toggle(infoPtr, wineItem, TRUE);
      break;

    case TVHT_ONITEMSTATEICON:
      if (infoPtr->dwStyle & TVS_CHECKBOXES)
        TREEVIEW_ToggleItemState(infoPtr, wineItem);
      else
        TREEVIEW_Toggle(infoPtr, wineItem, TRUE);
      break;
    }
  }
  return TRUE;
}


static GXLRESULT
TREEVIEW_LButtonDown(GXTREEVIEW_INFO *infoPtr, GXLPARAM lParam)
{
  GXHWND hwnd = infoPtr->hwnd;
  GXTVHITTESTINFO ht;
  GXBOOL bTrack, bDoLabelEdit;
  GXHTREEITEM tempItem;

  /* If Edit control is active - kill it and return.
  * The best way to do it is to set focus to itself.
  * Edit control subclassed procedure will automatically call
  * EndEditLabelNow.
  */
  if (infoPtr->hwndEdit)
  {
    gxSetFocus(hwnd);
    return 0;
  }

  ht.pt.x = (short)GXLOWORD(lParam);
  ht.pt.y = (short)GXHIWORD(lParam);

  TREEVIEW_HitTest(infoPtr, &ht);
  TRACE("item %d\n", TREEVIEW_GetItemIndex(infoPtr, ht.hItem));

  /* update focusedItem and redraw both items */
  if(ht.hItem && (ht.flags & TVHT_ONITEM))
  {
    infoPtr->focusedItem = ht.hItem;
    gxInvalidateRect(hwnd, &ht.hItem->rect, TRUE);

    if(infoPtr->selectedItem)
      gxInvalidateRect(hwnd, &(infoPtr->selectedItem->rect), TRUE);
  }

  bTrack = (ht.flags & TVHT_ONITEM)
    && !(infoPtr->dwStyle & TVS_DISABLEDRAGDROP);

  /*
  * If the style allows editing and the node is already selected
  * and the click occurred on the item label...
  */
  bDoLabelEdit = (infoPtr->dwStyle & TVS_EDITLABELS) &&
    (ht.flags & TVHT_ONITEMLABEL) && (infoPtr->selectedItem == ht.hItem);

  /* Send NM_CLICK right away */
  if (!bTrack)
    if (TREEVIEW_SendSimpleNotify(infoPtr, GXNM_CLICK))
      goto setfocus;

  if (ht.flags & TVHT_ONITEMBUTTON)
  {
    TREEVIEW_Toggle(infoPtr, ht.hItem, TRUE);
    goto setfocus;
  }
  else if (bTrack)
  {   /* if TREEVIEW_TrackMouse == 1 dragging occurred and the cursor left the dragged item's rectangle */
    if (TREEVIEW_TrackMouse(infoPtr, ht.pt))
    {
      TREEVIEW_SendTreeviewDnDNotify(infoPtr, GXTVN_BEGINDRAGW, ht.hItem, ht.pt);
      infoPtr->dropItem = ht.hItem;

      /* clean up focusedItem as we dragged and won't select this item */
      if(infoPtr->focusedItem)
      {
        /* refresh the item that was focused */
        tempItem = infoPtr->focusedItem;
        infoPtr->focusedItem = 0;
        gxInvalidateRect(infoPtr->hwnd, &tempItem->rect, TRUE);

        /* refresh the selected item to return the filled background */
        gxInvalidateRect(infoPtr->hwnd, &(infoPtr->selectedItem->rect), TRUE);
      }

      return 0;
    }
  }

  if (bTrack && TREEVIEW_SendSimpleNotify(infoPtr, GXNM_CLICK))
    goto setfocus;

  if (bDoLabelEdit)
  {
    if (infoPtr->Timer & GXTV_EDIT_TIMER_SET)
      gxKillTimer(hwnd, GXTV_EDIT_TIMER);

    gxSetTimer(hwnd, GXTV_EDIT_TIMER, gxGetDoubleClickTime(), 0);
    infoPtr->Timer |= GXTV_EDIT_TIMER_SET;
  }
  else if (ht.flags & (TVHT_ONITEMICON|TVHT_ONITEMLABEL)) /* select the item if the hit was inside of the icon or text */
  {
    /*
    * if we are TVS_SINGLEEXPAND then we want this single click to
    * do a bunch of things.
    */
    if((infoPtr->dwStyle & TVS_SINGLEEXPAND) &&
      (infoPtr->hwndEdit == 0))
    {
      GXTREEVIEW_ITEM *SelItem;

      /*
      * Send the notification
      */
      TREEVIEW_SendTreeviewNotify(infoPtr, GXTVN_SINGLEEXPAND, TVC_UNKNOWN, GXTVIF_HANDLE | GXTVIF_PARAM, ht.hItem, 0);

      /*
      * Close the previous selection all the way to the root
      * as long as the new selection is not a child
      */
      if((infoPtr->selectedItem)
        && (infoPtr->selectedItem != ht.hItem))
      {
        GXBOOL closeit = TRUE;
        SelItem = ht.hItem;

        /* determine if the hitItem is a child of the currently selected item */
        while(closeit && SelItem && TREEVIEW_ValidItem(infoPtr, SelItem) && (SelItem != infoPtr->root))
        {
          closeit = (SelItem != infoPtr->selectedItem);
          SelItem = SelItem->parent;
        }

        if(closeit)
        {
          if(TREEVIEW_ValidItem(infoPtr, infoPtr->selectedItem))
            SelItem = infoPtr->selectedItem;

          while(SelItem && (SelItem != ht.hItem) && TREEVIEW_ValidItem(infoPtr, SelItem) && (SelItem != infoPtr->root))
          {
            TREEVIEW_Collapse(infoPtr, SelItem, FALSE, FALSE);
            SelItem = SelItem->parent;
          }
        }
      }

      /*
      * Expand the current item
      */
      TREEVIEW_Expand(infoPtr, ht.hItem, TVE_TOGGLE, FALSE);
    }

    /* Select the current item */
    TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, ht.hItem, TVC_BYMOUSE);
  }
  else if (ht.flags & TVHT_ONITEMSTATEICON)
  {
    /* TVS_CHECKBOXES requires us to toggle the current state */
    if (infoPtr->dwStyle & TVS_CHECKBOXES)
      TREEVIEW_ToggleItemState(infoPtr, ht.hItem);
  }

setfocus:
  gxSetFocus(hwnd);
  return 0;
}


static GXLRESULT
TREEVIEW_RButtonDown(GXTREEVIEW_INFO *infoPtr, GXLPARAM lParam)
{
  GXTVHITTESTINFO ht;

  if (infoPtr->hwndEdit)
  {
    gxSetFocus(infoPtr->hwnd);
    return 0;
  }

  ht.pt.x = (short)GXLOWORD(lParam);
  ht.pt.y = (short)GXHIWORD(lParam);

  TREEVIEW_HitTest(infoPtr, &ht);

  if (TREEVIEW_TrackMouse(infoPtr, ht.pt))
  {
    if (ht.hItem)
    {
      TREEVIEW_SendTreeviewDnDNotify(infoPtr, GXTVN_BEGINRDRAGW, ht.hItem, ht.pt);
      infoPtr->dropItem = ht.hItem;
    }
  }
  else
  {
    gxSetFocus(infoPtr->hwnd);
    TREEVIEW_SendSimpleNotify(infoPtr, GXNM_RCLICK);
  }

  return 0;
}

static GXLRESULT
TREEVIEW_RButtonUp(const GXTREEVIEW_INFO *infoPtr, const GXPOINT *pPt)
{
  GXTVHITTESTINFO ht;

  ht.pt = *pPt;

  TREEVIEW_HitTest(infoPtr, &ht);

  if (ht.hItem)
  {
    /* Change to screen coordinate for WM_CONTEXTMENU */
    gxClientToScreen(infoPtr->hwnd, &ht.pt);

    /* Send a WM_CONTEXTMENU message in response to the RBUTTONUP */
    gxSendMessageW(infoPtr->hwnd, GXWM_CONTEXTMENU,
      (GXWPARAM)infoPtr->hwnd, GXMAKELPARAM(ht.pt.x, ht.pt.y));
  }
  return 0;
}


static GXLRESULT
TREEVIEW_CreateDragImage(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  GXTREEVIEW_ITEM *dragItem = (GXHTREEITEM)lParam;
  GXINT cx, cy;
  GXHDC hdc, htopdc;
  GXHWND hwtop;
  GXHBITMAP hbmp, hOldbmp;
  GXSIZE size;
  GXRECT rc;
  GXHFONT hOldFont;

  TRACE("\n");

  if (!(infoPtr->himlNormal))
    return 0;

  if (!dragItem || !TREEVIEW_ValidItem(infoPtr, dragItem))
    return 0;

  TREEVIEW_UpdateDispInfo(infoPtr, dragItem, GXTVIF_TEXT);

  hwtop = gxGetDesktopWindow();
  htopdc = gxGetDC(hwtop);
  hdc = gxCreateCompatibleDC(htopdc);

  hOldFont = (GXHFONT)gxSelectObject(hdc, infoPtr->hFont);
  gxGetTextExtentPoint32W(hdc, dragItem->pszText, GXSTRLEN(dragItem->pszText),
    &size);
  TRACE("%d %d %s %d\n", size.cx, size.cy, debugstr_w(dragItem->pszText),
    GXSTRLEN(dragItem->pszText));
  hbmp = gxCreateCompatibleBitmap(htopdc, size.cx, size.cy);
  hOldbmp = (GXHBITMAP)gxSelectObject(hdc, hbmp);

  gxImageList_GetIconSize(infoPtr->himlNormal, &cx, &cy);
  size.cx += cx;
  if (cy > size.cy)
    size.cy = cy;

  infoPtr->dragList = gxImageList_Create(size.cx, size.cy, GXILC_COLOR, 10, 10);
  gxImageList_Draw(infoPtr->himlNormal, dragItem->iImage, hdc, 0, 0,
    ILD_NORMAL);

  /*
  gxImageList_GetImageInfo (infoPtr->himlNormal, dragItem->hItem, &iminfo);
  gxImageList_AddMasked (infoPtr->dragList, iminfo.hbmImage, CLR_DEFAULT);
  */

  /* draw item text */

  gxSetRect(&rc, cx, 0, size.cx, size.cy);
  gxDrawTextW(hdc, dragItem->pszText, GXSTRLEN(dragItem->pszText), &rc,
    GXDT_LEFT);
  gxSelectObject(hdc, hOldFont);
  gxSelectObject(hdc, hOldbmp);

  gxImageList_Add(infoPtr->dragList, hbmp, 0);

  gxDeleteDC(hdc);
  gxDeleteObject(hbmp);
  gxReleaseDC(hwtop, htopdc);

  return (GXLRESULT)infoPtr->dragList;
}

/* Selection ************************************************************/

static GXLRESULT
TREEVIEW_DoSelectItem(GXTREEVIEW_INFO *infoPtr, GXINT action, GXHTREEITEM newSelect,
            GXINT cause)
{
  GXTREEVIEW_ITEM *prevSelect;
  GXRECT rcFocused;

  assert(newSelect == NULL || TREEVIEW_ValidItem(infoPtr, newSelect));

  TRACE("Entering item %p (%s), flag %x, cause %x, state %d\n",
    newSelect, TREEVIEW_ItemName(newSelect), action, cause,
    newSelect ? newSelect->state : 0);

  /* reset and redraw focusedItem if focusedItem was set so we don't */
  /* have to worry about the previously focused item when we set a new one */
  if(infoPtr->focusedItem)
  {
    rcFocused = (infoPtr->focusedItem)->rect;
    infoPtr->focusedItem = 0;
    gxInvalidateRect(infoPtr->hwnd, &rcFocused, TRUE);
  }

  switch (action)
  {
  case GXTVGN_CARET:
    prevSelect = infoPtr->selectedItem;

    if (prevSelect == newSelect) {
      TREEVIEW_EnsureVisible(infoPtr, infoPtr->selectedItem, FALSE);
      break;
    }

    if (TREEVIEW_SendTreeviewNotify(infoPtr,
      GXTVN_SELCHANGINGW,
      cause,
      GXTVIF_TEXT | GXTVIF_HANDLE | GXTVIF_STATE | GXTVIF_PARAM,
      prevSelect,
      newSelect))
      return FALSE;

    if (prevSelect)
      prevSelect->state &= ~TVIS_SELECTED;
    if (newSelect)
      newSelect->state |= TVIS_SELECTED;

    infoPtr->selectedItem = newSelect;

    TREEVIEW_EnsureVisible(infoPtr, infoPtr->selectedItem, FALSE);

    if (prevSelect)
      TREEVIEW_Invalidate(infoPtr, prevSelect);
    if (newSelect)
      TREEVIEW_Invalidate(infoPtr, newSelect);

    TREEVIEW_SendTreeviewNotify(infoPtr,
      GXTVN_SELCHANGEDW,
      cause,
      GXTVIF_TEXT | GXTVIF_HANDLE | GXTVIF_STATE | GXTVIF_PARAM,
      prevSelect,
      newSelect);
    break;

  case GXTVGN_DROPHILITE:
    prevSelect = infoPtr->dropItem;

    if (prevSelect)
      prevSelect->state &= ~TVIS_DROPHILITED;

    infoPtr->dropItem = newSelect;

    if (newSelect)
      newSelect->state |= TVIS_DROPHILITED;

    TREEVIEW_Invalidate(infoPtr, prevSelect);
    TREEVIEW_Invalidate(infoPtr, newSelect);
    break;

  case GXTVGN_FIRSTVISIBLE:
    if (newSelect != NULL)
    {
      TREEVIEW_EnsureVisible(infoPtr, newSelect, FALSE);
      TREEVIEW_SetFirstVisible(infoPtr, newSelect, TRUE);
      TREEVIEW_Invalidate(infoPtr, NULL);
    }
    break;
  }

  TRACE("Leaving state %d\n", newSelect ? newSelect->state : 0);
  return TRUE;
}

/* FIXME: handle NM_KILLFOCUS etc */
static GXLRESULT
TREEVIEW_SelectItem(GXTREEVIEW_INFO *infoPtr, GXINT wParam, GXHTREEITEM item)
{
  if (item != NULL && !TREEVIEW_ValidItem(infoPtr, item))
    return FALSE;

  TRACE("%p (%s) %d\n", item, TREEVIEW_ItemName(item), wParam);

  if (!TREEVIEW_DoSelectItem(infoPtr, wParam, item, TVC_UNKNOWN))
    return FALSE;

  return TRUE;
}

/*************************************************************************
*    TREEVIEW_ProcessLetterKeys
*
*  Processes keyboard messages generated by pressing the letter keys
*  on the keyboard.
*  What this does is perform a case insensitive search from the
*  current position with the following quirks:
*  - If two chars or more are pressed in quick succession we search
*    for the corresponding string (e.g. 'abc').
*  - If there is a delay we wipe away the current search string and
*    restart with just that char.
*  - If the user keeps pressing the same character, whether slowly or
*    fast, so that the search string is entirely composed of this
*    character ('aaaaa' for instance), then we search for first item
*    that starting with that character.
*  - If the user types the above character in quick succession, then
*    we must also search for the corresponding string ('aaaaa'), and
*    go to that string if there is a match.
*
* RETURNS
*
*  Zero.
*
* BUGS
*
*  - The current implementation has a list of characters it will
*    accept and it ignores averything else. In particular it will
*    ignore accentuated characters which seems to match what
*    Windows does. But I'm not sure it makes sense to follow
*    Windows there.
*  - We don't sound a beep when the search fails.
*  - The search should start from the focused item, not from the selected
*    item. One reason for this is to allow for multiple selections in trees.
*    But currently infoPtr->focusedItem does not seem very usable.
*
* SEE ALSO
*
*  TREEVIEW_ProcessLetterKeys
*/
static GXINT TREEVIEW_ProcessLetterKeys(
                    GXHWND hwnd, /* handle to the window */
                    GXWPARAM charCode, /* the character code, the actual character */
                    GXLPARAM keyData /* key data */
                    )
{
  GXTREEVIEW_INFO *infoPtr;
  GXHTREEITEM nItem;
  GXHTREEITEM endidx,idx;
  GXTVITEMEXW item;
  GXWCHAR buffer[MAX_PATH];
  GXDWORD timestamp,elapsed;

  /* simple parameter checking */
  if (!hwnd || !charCode || !keyData)
    return 0;

  infoPtr=(GXTREEVIEW_INFO*)gxGetWindowLongPtrW(hwnd, 0);
  if (!infoPtr)
    return 0;

  /* only allow the valid WM_CHARs through */
  if (!isalnum(charCode) &&
    charCode != '.' && charCode != '`' && charCode != '!' &&
    charCode != '@' && charCode != '#' && charCode != '$' &&
    charCode != '%' && charCode != '^' && charCode != '&' &&
    charCode != '*' && charCode != '(' && charCode != ')' &&
    charCode != '-' && charCode != '_' && charCode != '+' &&
    charCode != '=' && charCode != '\\'&& charCode != ']' &&
    charCode != '}' && charCode != '[' && charCode != '{' &&
    charCode != '/' && charCode != '?' && charCode != '>' &&
    charCode != '<' && charCode != ',' && charCode != '~')
    return 0;

  /* compute how much time elapsed since last keypress */
  timestamp = gxGetTickCount();
  if (timestamp > infoPtr->lastKeyPressTimestamp) {
    elapsed=timestamp-infoPtr->lastKeyPressTimestamp;
  } else {
    elapsed=infoPtr->lastKeyPressTimestamp-timestamp;
  }

  /* update the search parameters */
  infoPtr->lastKeyPressTimestamp=timestamp;
  if (elapsed < KEY_DELAY) {
    if (infoPtr->nSearchParamLength < sizeof(infoPtr->szSearchParam) / sizeof(GXWCHAR)) {
      infoPtr->szSearchParam[infoPtr->nSearchParamLength++]=charCode;
    }
    if (infoPtr->charCode != charCode) {
      infoPtr->charCode=charCode=0;
    }
  } else {
    infoPtr->charCode=charCode;
    infoPtr->szSearchParam[0]=charCode;
    infoPtr->nSearchParamLength=1;
    /* Redundant with the 1 char string */
    charCode=0;
  }

  /* and search from the current position */
  nItem=NULL;
  if (infoPtr->selectedItem != NULL) {
    endidx=infoPtr->selectedItem;
    /* if looking for single character match,
    * then we must always move forward
    */
    if (infoPtr->nSearchParamLength == 1)
      idx=TREEVIEW_GetNextListItem(infoPtr,endidx);
    else
      idx=endidx;
  } else {
    endidx=NULL;
    idx=infoPtr->root->firstChild;
  }
  do {
    /* At the end point, sort out wrapping */
    if (idx == NULL) {

      /* If endidx is null, stop at the last item (ie top to bottom) */
      if (endidx == NULL)
        break;

      /* Otherwise, start again at the very beginning */
      idx=infoPtr->root->firstChild;

      /* But if we are stopping on the first child, end now! */
      if (idx == endidx) break;
    }

    /* get item */
    ZeroMemory(&item, sizeof(item));
    item.mask = GXTVIF_TEXT;
    item.hItem = idx;
    item.pszText = buffer;
    item.cchTextMax = sizeof(buffer);
    TREEVIEW_GetItemT( infoPtr, &item, TRUE );

    /* check for a match */
    if (strncmpiW(item.pszText,infoPtr->szSearchParam,infoPtr->nSearchParamLength) == 0) {
      nItem=idx;
      break;
    } else if ( (charCode != 0) && (nItem == NULL) &&
      (nItem != infoPtr->selectedItem) &&
      (strncmpiW(item.pszText,infoPtr->szSearchParam,1) == 0) ) {
        /* This would work but we must keep looking for a longer match */
        nItem=idx;
    }
    idx=TREEVIEW_GetNextListItem(infoPtr,idx);
  } while (idx != endidx);

  if (nItem != NULL) {
    if (TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, nItem, TVC_BYKEYBOARD)) {
      TREEVIEW_EnsureVisible(infoPtr, nItem, FALSE);
    }
  }

  return 0;
}

/* Scrolling ************************************************************/

static GXLRESULT
TREEVIEW_EnsureVisible(GXTREEVIEW_INFO *infoPtr, GXHTREEITEM item, GXBOOL bHScroll)
{
  int viscount;
  GXBOOL hasFirstVisible = infoPtr->firstVisible != NULL;
  GXHTREEITEM newFirstVisible = NULL;
  int visible_pos = -1;

  if (!TREEVIEW_ValidItem(infoPtr, item))
    return FALSE;

  if (!ISVISIBLE(item))
  {
    /* Expand parents as necessary. */
    GXHTREEITEM parent;

    /* see if we are trying to ensure that root is vislble */
    if((item != infoPtr->root) && TREEVIEW_ValidItem(infoPtr, item))
      parent = item->parent;
    else
      parent = item; /* this item is the topmost item */

    while (parent != infoPtr->root)
    {
      if (!(parent->state & TVIS_EXPANDED))
        TREEVIEW_Expand(infoPtr, parent, FALSE, FALSE);

      parent = parent->parent;
    }
  }

  viscount = TREEVIEW_GetVisibleCount(infoPtr);

  TRACE("%p (%s) %d - %d viscount(%d)\n", item, TREEVIEW_ItemName(item), item->visibleOrder,
    hasFirstVisible ? infoPtr->firstVisible->visibleOrder : -1, viscount);

  if (hasFirstVisible)
    visible_pos = item->visibleOrder - infoPtr->firstVisible->visibleOrder;

  if (visible_pos < 0)
  {
    /* item is before the start of the list: put it at the top. */
    newFirstVisible = item;
  }
  else if (visible_pos >= viscount
    /* Sometimes, before we are displayed, GVC is 0, causing us to
    * spuriously scroll up. */
    && visible_pos > 0 && !(infoPtr->dwStyle & TVS_NOSCROLL) )
  {
    /* item is past the end of the list. */
    int scroll = visible_pos - viscount;

    newFirstVisible = TREEVIEW_GetListItem(infoPtr, infoPtr->firstVisible,
      scroll + 1);
  }

  if (bHScroll)
  {
    /* Scroll window so item's text is visible as much as possible */
    /* Calculation of amount of extra space is taken from EditLabel code */
    GXINT pos, x;
    GXTEXTMETRICW textMetric;
    GXHDC hdc = gxGetWindowDC(infoPtr->hwnd);

    x = item->textWidth;

    gxGetTextMetricsW(hdc, &textMetric);
    gxReleaseDC(infoPtr->hwnd, hdc);

    x += (textMetric.tmMaxCharWidth * 2);
    x = max(x, textMetric.tmMaxCharWidth * 3);

    if (item->textOffset < 0)
      pos = item->textOffset;
    else if (item->textOffset + x > infoPtr->clientWidth)
    {
      if (x > infoPtr->clientWidth)
        pos = item->textOffset;
      else
        pos = item->textOffset + x - infoPtr->clientWidth;
    }
    else
      pos = 0;

    TREEVIEW_HScroll(infoPtr, GXMAKEWPARAM(GXSB_THUMBPOSITION, infoPtr->scrollX + pos));
  }

  if (newFirstVisible != NULL && newFirstVisible != infoPtr->firstVisible)
  {
    TREEVIEW_SetFirstVisible(infoPtr, newFirstVisible, TRUE);

    return TRUE;
  }

  return FALSE;
}

static GXVOID
TREEVIEW_SetFirstVisible(GXTREEVIEW_INFO *infoPtr,
             GXTREEVIEW_ITEM *newFirstVisible,
             GXBOOL bUpdateScrollPos)
{
  int gap_size;

  TRACE("%p: %s\n", newFirstVisible, TREEVIEW_ItemName(newFirstVisible));

  if (newFirstVisible != NULL)
  {
    /* Prevent an empty gap from appearing at the bottom... */
    gap_size = TREEVIEW_GetVisibleCount(infoPtr)
      - infoPtr->maxVisibleOrder + newFirstVisible->visibleOrder;

    if (gap_size > 0)
    {
      newFirstVisible = TREEVIEW_GetListItem(infoPtr, newFirstVisible,
        -gap_size);

      /* ... unless we just don't have enough items. */
      if (newFirstVisible == NULL)
        newFirstVisible = infoPtr->root->firstChild;
    }
  }

  if (infoPtr->firstVisible != newFirstVisible)
  {
    if (infoPtr->firstVisible == NULL || newFirstVisible == NULL)
    {
      infoPtr->firstVisible = newFirstVisible;
      TREEVIEW_Invalidate(infoPtr, NULL);
    }
    else
    {
      GXTREEVIEW_ITEM *item;
      int scroll = infoPtr->uItemHeight *
        (infoPtr->firstVisible->visibleOrder
        - newFirstVisible->visibleOrder);

      infoPtr->firstVisible = newFirstVisible;

      for (item = infoPtr->root->firstChild; item != NULL;
        item = TREEVIEW_GetNextListItem(infoPtr, item))
      {
        item->rect.top += scroll;
        item->rect.bottom += scroll;
      }

      if (bUpdateScrollPos)
        gxSetScrollPos(infoPtr->hwnd, GXSB_VERT,
        newFirstVisible->visibleOrder, TRUE);

      gxScrollWindowEx(infoPtr->hwnd, 0, scroll, NULL, NULL, NULL, NULL, GXSW_ERASE | GXSW_INVALIDATE);
    }
  }
}

/************************************************************************
* VScroll is always in units of visible items. i.e. we always have a
* visible item aligned to the top of the control. (Unless we have no
* items at all.)
*/
static GXLRESULT
TREEVIEW_VScroll(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  GXTREEVIEW_ITEM *oldFirstVisible = infoPtr->firstVisible;
  GXTREEVIEW_ITEM *newFirstVisible = NULL;

  int nScrollCode = GXLOWORD(wParam);

  TRACE("wp %lx\n", wParam);

  if (!(infoPtr->uInternalStatus & GXTV_VSCROLL))
    return 0;

  if (!oldFirstVisible)
  {
    assert(infoPtr->root->firstChild == NULL);
    return 0;
  }

  switch (nScrollCode)
  {
  case GXSB_TOP:
    newFirstVisible = infoPtr->root->firstChild;
    break;

  case GXSB_BOTTOM:
    newFirstVisible = TREEVIEW_GetLastListItem(infoPtr, infoPtr->root);
    break;

  case GXSB_LINEUP:
    newFirstVisible = TREEVIEW_GetPrevListItem(infoPtr, oldFirstVisible);
    break;

  case GXSB_LINEDOWN:
    newFirstVisible = TREEVIEW_GetNextListItem(infoPtr, oldFirstVisible);
    break;

  case GXSB_PAGEUP:
    newFirstVisible = TREEVIEW_GetListItem(infoPtr, oldFirstVisible,
      -max(1, TREEVIEW_GetVisibleCount(infoPtr)));
    break;

  case GXSB_PAGEDOWN:
    newFirstVisible = TREEVIEW_GetListItem(infoPtr, oldFirstVisible,
      max(1, TREEVIEW_GetVisibleCount(infoPtr)));
    break;

  case GXSB_THUMBTRACK:
  case GXSB_THUMBPOSITION:
    newFirstVisible = TREEVIEW_GetListItem(infoPtr,
      infoPtr->root->firstChild,
      (GXLONG)(GXSHORT)GXHIWORD(wParam));
    break;

  case GXSB_ENDSCROLL:
    return 0;
  }

  if (newFirstVisible != NULL)
  {
    if (newFirstVisible != oldFirstVisible)
      TREEVIEW_SetFirstVisible(infoPtr, newFirstVisible,
      nScrollCode != GXSB_THUMBTRACK);
    else if (nScrollCode == GXSB_THUMBPOSITION)
      gxSetScrollPos(infoPtr->hwnd, GXSB_VERT,
      newFirstVisible->visibleOrder, TRUE);
  }

  return 0;
}

static GXLRESULT
TREEVIEW_HScroll(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  int maxWidth;
  int scrollX = infoPtr->scrollX;
  int nScrollCode = GXLOWORD(wParam);

  TRACE("wp %lx\n", wParam);

  if (!(infoPtr->uInternalStatus & GXTV_HSCROLL))
    return FALSE;

  maxWidth = infoPtr->treeWidth - infoPtr->clientWidth;
  /* shall never occur */
  if (maxWidth <= 0)
  {
    scrollX = 0;
    goto scroll;
  }

  switch (nScrollCode)
  {
  case GXSB_LINELEFT:
    scrollX -= infoPtr->uItemHeight;
    break;
  case GXSB_LINERIGHT:
    scrollX += infoPtr->uItemHeight;
    break;
  case GXSB_PAGELEFT:
    scrollX -= infoPtr->clientWidth;
    break;
  case GXSB_PAGERIGHT:
    scrollX += infoPtr->clientWidth;
    break;

  case GXSB_THUMBTRACK:
  case GXSB_THUMBPOSITION:
    scrollX = (int)(GXSHORT)GXHIWORD(wParam);
    break;

  case GXSB_ENDSCROLL:
    return 0;
  }

  if (scrollX > maxWidth)
    scrollX = maxWidth;
  else if (scrollX < 0)
    scrollX = 0;

scroll:
  if (scrollX != infoPtr->scrollX)
  {
    GXTREEVIEW_ITEM *item;
    GXLONG scroll_pixels = infoPtr->scrollX - scrollX;

    for (item = infoPtr->root->firstChild; item != NULL;
      item = TREEVIEW_GetNextListItem(infoPtr, item))
    {
      item->linesOffset += scroll_pixels;
      item->stateOffset += scroll_pixels;
      item->imageOffset += scroll_pixels;
      item->textOffset  += scroll_pixels;
    }

    gxScrollWindow(infoPtr->hwnd, scroll_pixels, 0, NULL, NULL);
    infoPtr->scrollX = scrollX;
    gxUpdateWindow(infoPtr->hwnd);
  }

  if (nScrollCode != GXSB_THUMBTRACK)
    gxSetScrollPos(infoPtr->hwnd, GXSB_HORZ, scrollX, TRUE);

  return 0;
}

static GXLRESULT
TREEVIEW_MouseWheel(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  short gcWheelDelta;
  GXUINT pulScrollLines = 3;

  if (infoPtr->firstVisible == NULL)
    return TRUE;

  gxSystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &pulScrollLines, 0);

  gcWheelDelta = -(short)GXHIWORD(wParam);
  pulScrollLines *= (gcWheelDelta / GXWHEEL_DELTA);

  if (abs(gcWheelDelta) >= GXWHEEL_DELTA && pulScrollLines)
  {
    int newDy = infoPtr->firstVisible->visibleOrder + pulScrollLines;
    int maxDy = infoPtr->maxVisibleOrder;

    if (newDy > maxDy)
      newDy = maxDy;

    if (newDy < 0)
      newDy = 0;

    TREEVIEW_VScroll(infoPtr, GXMAKEWPARAM(GXSB_THUMBPOSITION, newDy));
  }
  return TRUE;
}

/* Create/Destroy *******************************************************/

static void
initialize_checkboxes(GXTREEVIEW_INFO *infoPtr)
{
  GXRECT rc;
  GXHBITMAP hbm, hbmOld;
  GXHDC hdc, hdcScreen;
  int nIndex;

  infoPtr->himlState = gxImageList_Create(16, 16, GXILC_COLOR | GXILC_MASK, 3, 0);

  hdcScreen = gxGetDC(0);

  hdc = gxCreateCompatibleDC(hdcScreen);
  hbm = gxCreateCompatibleBitmap(hdcScreen, 48, 16);
  hbmOld = (GXHBITMAP)gxSelectObject(hdc, hbm);

  gxSetRect(&rc, 0, 0, 48, 16);
  gxFillRect(hdc, &rc, (GXHBRUSH)(GXCOLOR_WINDOW+1));

  gxSetRect(&rc, 18, 2, 30, 14);
  gxDrawFrameControl(hdc, &rc, GXDFC_BUTTON,
    GXDFCS_BUTTONCHECK|GXDFCS_FLAT);

  gxSetRect(&rc, 34, 2, 46, 14);
  gxDrawFrameControl(hdc, &rc, GXDFC_BUTTON,
    GXDFCS_BUTTONCHECK|GXDFCS_FLAT|GXDFCS_CHECKED);

  gxSelectObject(hdc, hbmOld);
  nIndex = gxImageList_AddMasked(infoPtr->himlState, hbm,
    gxGetSysColor(GXCOLOR_WINDOW));
  TRACE("checkbox index %d\n", nIndex);

  gxDeleteObject(hbm);
  gxDeleteDC(hdc);
  gxReleaseDC(0, hdcScreen);

  infoPtr->stateImageWidth = 16;
  infoPtr->stateImageHeight = 16;
}

static GXLRESULT
TREEVIEW_Create(GXHWND hwnd, const GXCREATESTRUCTW *lpcs)
{
  GXRECT rcClient;
  GXTREEVIEW_INFO *infoPtr;
  GXLOGFONTW lf;

  TRACE("wnd %p, style %x\n", hwnd, gxGetWindowLongW(hwnd, GXGWL_STYLE));

  infoPtr = (GXTREEVIEW_INFO *)Alloc(sizeof(GXTREEVIEW_INFO));

  if (infoPtr == NULL)
  {
    ERR("could not allocate info memory!\n");
    return 0;
  }

  gxSetWindowLongPtrW(hwnd, 0, (GXDWORD_PTR)infoPtr);

  infoPtr->hwnd = hwnd;
  infoPtr->dwStyle = gxGetWindowLongW(hwnd, GXGWL_STYLE);
  infoPtr->Timer = 0;
  infoPtr->uNumItems = 0;
  infoPtr->cdmode = 0;
  infoPtr->uScrollTime = 300;  /* milliseconds */
  infoPtr->bRedraw = TRUE;

  gxGetClientRect(hwnd, &rcClient);

  /* No scroll bars yet. */
  infoPtr->clientWidth = rcClient.right;
  infoPtr->clientHeight = rcClient.bottom;
  infoPtr->uInternalStatus = 0;

  infoPtr->treeWidth = 0;
  infoPtr->treeHeight = 0;

  infoPtr->uIndent = MINIMUM_INDENT;
  infoPtr->selectedItem = 0;
  infoPtr->focusedItem = 0;
  infoPtr->hotItem = 0;
  infoPtr->firstVisible = 0;
  infoPtr->maxVisibleOrder = 0;
  infoPtr->dropItem = 0;
  infoPtr->insertMarkItem = 0;
  infoPtr->insertBeforeorAfter = 0;
  /* dragList */

  infoPtr->scrollX = 0;

  infoPtr->clrBk = gxGetSysColor(GXCOLOR_WINDOW);
  infoPtr->clrText = -1;  /* use system color */
  infoPtr->clrLine = GXRGB(128, 128, 128);
  infoPtr->clrInsertMark = gxGetSysColor(GXCOLOR_BTNTEXT);

  /* hwndToolTip */

  infoPtr->hwndEdit = 0;
  infoPtr->wpEditOrig = NULL;
  infoPtr->bIgnoreEditKillFocus = FALSE;
  infoPtr->bLabelChanged = FALSE;

  infoPtr->himlNormal = NULL;
  infoPtr->himlState = NULL;
  infoPtr->normalImageWidth = 0;
  infoPtr->normalImageHeight = 0;
  infoPtr->stateImageWidth = 0;
  infoPtr->stateImageHeight = 0;

  infoPtr->items = gxDPA_Create(16);

  gxSystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
  infoPtr->hFont = infoPtr->hDefaultFont = gxCreateFontIndirectW(&lf);
  infoPtr->hBoldFont = TREEVIEW_CreateBoldFont(infoPtr->hFont);
  infoPtr->hUnderlineFont = TREEVIEW_CreateUnderlineFont(infoPtr->hFont);
  infoPtr->hcurHand = gxLoadCursorW(NULL, (GXLPWSTR)GXIDC_HAND);

  infoPtr->uItemHeight = TREEVIEW_NaturalHeight(infoPtr);

  infoPtr->root = TREEVIEW_AllocateItem(infoPtr);
  infoPtr->root->state = TVIS_EXPANDED;
  infoPtr->root->iLevel = -1;
  infoPtr->root->visibleOrder = -1;

  infoPtr->hwndNotify = lpcs->hwndParent;
#if 0
  infoPtr->bTransparent = ( gxGetWindowLongW( hwnd, GXGWL_STYLE) & TBSTYLE_FLAT);
#endif

  infoPtr->hwndToolTip = 0;

  infoPtr->bNtfUnicode = gxIsWindowUnicode (hwnd);

  /* Determine what type of notify should be issued */
  /* sets infoPtr->bNtfUnicode */
  TREEVIEW_NotifyFormat(infoPtr, infoPtr->hwndNotify, GXNF_REQUERY);

  if (!(infoPtr->dwStyle & TVS_NOTOOLTIPS))
    infoPtr->hwndToolTip = gxCreateWindowExW(0, TOOLTIPS_CLASSW, NULL, GXWS_POPUP,
    GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT,
    hwnd, 0, 0, 0);

  if (infoPtr->dwStyle & TVS_CHECKBOXES)
    initialize_checkboxes(infoPtr);

  /* Make sure actual scrollbar state is consistent with uInternalStatus */
  gxShowScrollBar(hwnd, GXSB_VERT, FALSE);
  gxShowScrollBar(hwnd, GXSB_HORZ, FALSE);

  gxOpenThemeData (hwnd, themeClass);

  return 0;
}


static GXLRESULT
TREEVIEW_Destroy(GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");

  TREEVIEW_RemoveTree(infoPtr);
  Free(infoPtr->root);

  /* tool tip is automatically destroyed: we are its owner */

  /* Restore original wndproc */
  if (infoPtr->hwndEdit)
    gxSetWindowLongPtrW(infoPtr->hwndEdit, GXGWLP_WNDPROC,
    (GXDWORD_PTR)infoPtr->wpEditOrig);

  gxCloseThemeData (gxGetWindowTheme (infoPtr->hwnd));

  /* Deassociate treeview from the window before doing anything drastic. */
  gxSetWindowLongPtrW(infoPtr->hwnd, 0, (GXDWORD_PTR)NULL);

  gxDPA_Destroy(infoPtr->items);

  gxDeleteObject(infoPtr->hDefaultFont);
  gxDeleteObject(infoPtr->hBoldFont);
  gxDeleteObject(infoPtr->hUnderlineFont);
  Free(infoPtr);

  return 0;
}

/* Miscellaneous Messages ***********************************************/

static GXLRESULT
TREEVIEW_ScrollKeyDown(GXTREEVIEW_INFO *infoPtr, GXWPARAM key)
{
  static const struct
  {
    unsigned char code;
  }
  scroll[] =
  {
#define SCROLL_ENTRY(dir, code) { ((dir) << 7) | (code) }
    SCROLL_ENTRY(GXSB_VERT, GXSB_PAGEUP),  /* GXVK_PRIOR */
    SCROLL_ENTRY(GXSB_VERT, GXSB_PAGEDOWN),  /* GXVK_NEXT */
    SCROLL_ENTRY(GXSB_VERT, GXSB_BOTTOM),  /* GXVK_END */
    SCROLL_ENTRY(GXSB_VERT, GXSB_TOP),    /* GXVK_HOME */
    SCROLL_ENTRY(GXSB_HORZ, GXSB_LINEUP),  /* GXVK_LEFT */
    SCROLL_ENTRY(GXSB_VERT, GXSB_LINEUP),  /* GXVK_UP */
    SCROLL_ENTRY(GXSB_HORZ, GXSB_LINEDOWN),  /* GXVK_RIGHT */
    SCROLL_ENTRY(GXSB_VERT, GXSB_LINEDOWN)  /* GXVK_DOWN */
#undef SCROLL_ENTRY
  };

  if (key >= GXVK_PRIOR && key <= GXVK_DOWN)
  {
    unsigned char code = scroll[key - GXVK_PRIOR].code;

    (((code & (1 << 7)) == (GXSB_HORZ << 7))
      ? TREEVIEW_HScroll
      : TREEVIEW_VScroll)(infoPtr, code & 0x7F);
  }

  return 0;
}

/************************************************************************
*        TREEVIEW_KeyDown
*
* GXVK_UP       Move selection to the previous non-hidden item.
* GXVK_DOWN     Move selection to the next non-hidden item.
* VK_HOME     Move selection to the first item.
* VK_END      Move selection to the last item.
* GXVK_LEFT     If expanded then collapse, otherwise move to parent.
* GXVK_RIGHT    If collapsed then expand, otherwise move to first child.
* VK_ADD      Expand.
* VK_SUBTRACT Collapse.
* VK_MULTIPLY Expand all.
* VK_PRIOR    Move up GetVisibleCount items.
* VK_NEXT     Move down GetVisibleCount items.
* VK_BACK     Move to parent.
* CTRL-Left,Right,Up,Down,PgUp,PgDown,Home,End: Scroll without changing selection
*/
static GXLRESULT
TREEVIEW_KeyDown(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam)
{
  /* If it is non-NULL and different, it will be selected and visible. */
  GXTREEVIEW_ITEM *newSelection = NULL;

  GXTREEVIEW_ITEM *prevItem = infoPtr->selectedItem;

  TRACE("%lx\n", wParam);

  if (prevItem == NULL)
    return FALSE;

  if (gxGetAsyncKeyState(GXVK_CONTROL) & 0x8000)
    return TREEVIEW_ScrollKeyDown(infoPtr, wParam);

  switch (wParam)
  {
  case GXVK_UP:
    newSelection = TREEVIEW_GetPrevListItem(infoPtr, prevItem);
    if (!newSelection)
      newSelection = infoPtr->root->firstChild;
    break;

  case GXVK_DOWN:
    newSelection = TREEVIEW_GetNextListItem(infoPtr, prevItem);
    break;

  case GXVK_HOME:
    newSelection = infoPtr->root->firstChild;
    break;

  case GXVK_END:
    newSelection = TREEVIEW_GetLastListItem(infoPtr, infoPtr->root);
    break;

  case GXVK_LEFT:
    if (prevItem->state & TVIS_EXPANDED)
    {
      TREEVIEW_Collapse(infoPtr, prevItem, FALSE, TRUE);
    }
    else if (prevItem->parent != infoPtr->root)
    {
      newSelection = prevItem->parent;
    }
    break;

  case GXVK_RIGHT:
    if (TREEVIEW_HasChildren(infoPtr, prevItem))
    {
      if (!(prevItem->state & TVIS_EXPANDED))
        TREEVIEW_Expand(infoPtr, prevItem, FALSE, TRUE);
      else
      {
        newSelection = prevItem->firstChild;
      }
    }

    break;

  case GXVK_MULTIPLY:
    TREEVIEW_ExpandAll(infoPtr, prevItem);
    break;

  case GXVK_ADD:
    if (!(prevItem->state & TVIS_EXPANDED))
      TREEVIEW_Expand(infoPtr, prevItem, FALSE, TRUE);
    break;

  case GXVK_SUBTRACT:
    if (prevItem->state & TVIS_EXPANDED)
      TREEVIEW_Collapse(infoPtr, prevItem, FALSE, TRUE);
    break;

  case GXVK_PRIOR:
    newSelection
      = TREEVIEW_GetListItem(infoPtr, prevItem,
      -TREEVIEW_GetVisibleCount(infoPtr));
    break;

  case GXVK_NEXT:
    newSelection
      = TREEVIEW_GetListItem(infoPtr, prevItem,
      TREEVIEW_GetVisibleCount(infoPtr));
    break;

  case GXVK_BACK:
    newSelection = prevItem->parent;
    if (newSelection == infoPtr->root)
      newSelection = NULL;
    break;

  case GXVK_SPACE:
    if (infoPtr->dwStyle & TVS_CHECKBOXES)
      TREEVIEW_ToggleItemState(infoPtr, prevItem);
    break;
  }

  if (newSelection && newSelection != prevItem)
  {
    if (TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, newSelection,
      TVC_BYKEYBOARD))
    {
      TREEVIEW_EnsureVisible(infoPtr, newSelection, FALSE);
    }
  }

  return FALSE;
}

static GXLRESULT
TREEVIEW_MouseLeave (GXTREEVIEW_INFO * infoPtr)
{
  if (infoPtr->hotItem)
  {
    /* remove hot effect from item */
    gxInvalidateRect(infoPtr->hwnd, &infoPtr->hotItem->rect, TRUE);
    infoPtr->hotItem = NULL;
  }
  return 0;
}

static GXLRESULT
TREEVIEW_MouseMove (GXTREEVIEW_INFO * infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  GXPOINT pt;
  GXTRACKMOUSEEVENT trackinfo;
  GXTREEVIEW_ITEM * item;

  /* fill in the GXTRACKMOUSEEVENT struct */
  trackinfo.cbSize = sizeof(GXTRACKMOUSEEVENT);
  trackinfo.dwFlags = GXTME_QUERY;
  trackinfo.hwndTrack = infoPtr->hwnd;
  trackinfo.dwHoverTime = GXHOVER_DEFAULT;

  /* call _gxTrackMouseEvent to see if we are currently tracking for this hwnd */
  _gxTrackMouseEvent(&trackinfo);

  /* Make sure tracking is enabled so we receive a WM_MOUSELEAVE message */
  if(!(trackinfo.dwFlags & GXTME_LEAVE))
  {
    trackinfo.dwFlags = GXTME_LEAVE; /* notify upon leaving */

    /* call GXTRACKMOUSEEVENT so we receive a WM_MOUSELEAVE message */
    /* and can properly deactivate the hot item */
    _gxTrackMouseEvent(&trackinfo);
  }

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);

  item = TREEVIEW_HitTestPoint(infoPtr, pt);

  if (item != infoPtr->hotItem)
  {
    /* redraw old hot item */
    if (infoPtr->hotItem)
      gxInvalidateRect(infoPtr->hwnd, &infoPtr->hotItem->rect, TRUE);
    infoPtr->hotItem = item;
    /* redraw new hot item */
    if (infoPtr->hotItem)
      gxInvalidateRect(infoPtr->hwnd, &infoPtr->hotItem->rect, TRUE);
  }

  return 0;
}

/* Draw themed border */
static GXBOOL nc_paint (const GXTREEVIEW_INFO *infoPtr, GXHRGN region)
{
  GXHTHEME theme = gxGetWindowTheme (infoPtr->hwnd);
  GXHDC dc;
  GXRECT r;
  GXHRGN cliprgn;
  int cxEdge = gxGetSystemMetrics (GXSM_CXEDGE),
    cyEdge = gxGetSystemMetrics (GXSM_CYEDGE);

  if (!theme) return FALSE;

  gxGetWindowRect(infoPtr->hwnd, &r);

  cliprgn = gxCreateRectRgn (r.left + cxEdge, r.top + cyEdge,
    r.right - cxEdge, r.bottom - cyEdge);
  if (region != (GXHRGN)1)
    gxCombineRgn (cliprgn, cliprgn, region, GXRGN_AND);
  gxOffsetRect(&r, -r.left, -r.top);

  dc = gxGetDCEx(infoPtr->hwnd, region, GXDCX_WINDOW|GXDCX_INTERSECTRGN);
  gxOffsetRect(&r, -r.left, -r.top);

  if (gxIsThemeBackgroundPartiallyTransparent (theme, 0, 0))
    gxDrawThemeParentBackground(infoPtr->hwnd, dc, &r);
  gxDrawThemeBackground (theme, dc, 0, 0, &r, 0);
  gxReleaseDC(infoPtr->hwnd, dc);

  /* Call default proc to get the scrollbars etc. painted */
  gxDefWindowProcW (infoPtr->hwnd, GXWM_NCPAINT, (GXWPARAM)cliprgn, 0);

  // Patch
  gxDeleteObject(cliprgn);

  return TRUE;
}

static GXLRESULT
TREEVIEW_Notify(const GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLPNMHDR lpnmh = (GXLPNMHDR)lParam;

  if (lpnmh->code == PGN_CALCSIZE) {
    GXLPNMPGCALCSIZE lppgc = (GXLPNMPGCALCSIZE)lParam;

    if (lppgc->dwFlag == PGF_CALCWIDTH) {
      lppgc->iWidth = infoPtr->treeWidth;
      TRACE("got PGN_CALCSIZE, returning horz size = %d, client=%d\n",
        infoPtr->treeWidth, infoPtr->clientWidth);
    }
    else {
      lppgc->iHeight = infoPtr->treeHeight;
      TRACE("got PGN_CALCSIZE, returning vert size = %d, client=%d\n",
        infoPtr->treeHeight, infoPtr->clientHeight);
    }
    return 0;
  }
  return gxDefWindowProcW(infoPtr->hwnd, GXWM_NOTIFY, wParam, lParam);
}

static GXINT TREEVIEW_NotifyFormat (GXTREEVIEW_INFO *infoPtr, GXHWND hwndFrom, GXUINT nCommand)
{
  GXINT format;

  TRACE("(hwndFrom=%p, nCommand=%d)\n", hwndFrom, nCommand);

  if (nCommand != GXNF_REQUERY) return 0;

  format = gxSendMessageW(hwndFrom, GXWM_NOTIFYFORMAT, (GXWPARAM)infoPtr->hwnd, GXNF_QUERY);
  TRACE("format=%d\n", format);

  if (format != GXNFR_ANSI && format != GXNFR_UNICODE) return 0;

  infoPtr->bNtfUnicode = (format == GXNFR_UNICODE);

  return format;
}

static GXLRESULT
TREEVIEW_Size(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  if (wParam == GXSIZE_RESTORED)
  {
    infoPtr->clientWidth  = (short)GXLOWORD(lParam);
    infoPtr->clientHeight = (short)GXHIWORD(lParam);

    TREEVIEW_RecalculateVisibleOrder(infoPtr, NULL);
    TREEVIEW_SetFirstVisible(infoPtr, infoPtr->firstVisible, TRUE);
    TREEVIEW_UpdateScrollBars(infoPtr);
  }
  else
  {
    FIXME("WM_SIZE flag %lx %lx not handled\n", wParam, lParam);
  }

  TREEVIEW_Invalidate(infoPtr, NULL);
  return 0;
}

static GXLRESULT
TREEVIEW_StyleChanged(GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("(%lx %lx)\n", wParam, lParam);

  if (wParam == GXGWL_STYLE)
  {
    GXDWORD dwNewStyle = ((GXLPSTYLESTRUCT)lParam)->styleNew;

    if ((infoPtr->dwStyle ^ dwNewStyle) & TVS_CHECKBOXES)
    {
      if (dwNewStyle & TVS_CHECKBOXES)
      {
        initialize_checkboxes(infoPtr);
        TRACE("checkboxes enabled\n");
      }
      else
      {
        FIXME("tried to disable checkboxes\n");
      }
    }

    if ((infoPtr->dwStyle ^ dwNewStyle) & TVS_NOTOOLTIPS)
    {
      if (infoPtr->dwStyle & TVS_NOTOOLTIPS)
      {
        infoPtr->hwndToolTip = gxCOMCTL32_CreateToolTip(infoPtr->hwnd);
        TRACE("tooltips enabled\n");
      }
      else
      {
        gxDestroyWindow(infoPtr->hwndToolTip);
        infoPtr->hwndToolTip = 0;
        TRACE("tooltips disabled\n");
      }
    }

    infoPtr->dwStyle = dwNewStyle;
  }

  TREEVIEW_UpdateSubTree(infoPtr, infoPtr->root);
  TREEVIEW_UpdateScrollBars(infoPtr);
  TREEVIEW_Invalidate(infoPtr, NULL);

  return 0;
}

static GXLRESULT
TREEVIEW_SetCursor(const GXTREEVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  GXPOINT pt;
  GXTREEVIEW_ITEM * item;

  gxGetCursorPos(&pt);
  gxScreenToClient(infoPtr->hwnd, &pt);

  item = TREEVIEW_HitTestPoint(infoPtr, pt);

  /* FIXME: send NM_SETCURSOR */

  if (item && (infoPtr->dwStyle & TVS_TRACKSELECT))
  {
    gxSetCursor(infoPtr->hcurHand);
    return 0;
  }
  else
    return gxDefWindowProcW(infoPtr->hwnd, GXWM_SETCURSOR, wParam, lParam);
}

static GXLRESULT
TREEVIEW_SetFocus(GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");

  if (!infoPtr->selectedItem)
  {
    TREEVIEW_DoSelectItem(infoPtr, GXTVGN_CARET, infoPtr->firstVisible,
      TVC_UNKNOWN);
  }

  TREEVIEW_Invalidate(infoPtr, infoPtr->selectedItem);
  TREEVIEW_SendSimpleNotify(infoPtr, GXNM_SETFOCUS);
  return 0;
}

static GXLRESULT
TREEVIEW_KillFocus(const GXTREEVIEW_INFO *infoPtr)
{
  TRACE("\n");

  TREEVIEW_Invalidate(infoPtr, infoPtr->selectedItem);
  gxUpdateWindow(infoPtr->hwnd);
  TREEVIEW_SendSimpleNotify(infoPtr, GXNM_KILLFOCUS);
  return 0;
}

/* update theme after a WM_THEMECHANGED message */
static GXLRESULT theme_changed(const GXTREEVIEW_INFO *infoPtr)
{
  GXHTHEME theme = gxGetWindowTheme (infoPtr->hwnd);
  gxCloseThemeData (theme);
  gxOpenThemeData (infoPtr->hwnd, themeClass);
  return 0;
}


static GXLRESULT GXCALLBACK
TREEVIEW_WindowProc(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  GXTREEVIEW_INFO *infoPtr = TREEVIEW_GetInfoPtr(hwnd);

  TRACE("hwnd %p msg %04x wp=%08lx lp=%08lx\n", hwnd, uMsg, wParam, lParam);

  if (infoPtr) TREEVIEW_VerifyTree(infoPtr);
  else
  {
    if (uMsg == GXWM_CREATE)
      TREEVIEW_Create(hwnd, (GXLPCREATESTRUCTW)lParam);
    else
      goto def;
  }

  switch (uMsg)
  {
  case GXTVM_CREATEDRAGIMAGE:
    return TREEVIEW_CreateDragImage(infoPtr, wParam, lParam);

  case GXTVM_DELETEITEM:
    return TREEVIEW_DeleteItem(infoPtr, (GXHTREEITEM)lParam);

  case GXTVM_EDITLABELA:
    return (GXLRESULT)TREEVIEW_EditLabel(infoPtr, (GXHTREEITEM)lParam);

  case GXTVM_EDITLABELW:
    return (GXLRESULT)TREEVIEW_EditLabel(infoPtr, (GXHTREEITEM)lParam);

  case GXTVM_ENDEDITLABELNOW:
    return TREEVIEW_EndEditLabelNow(infoPtr, (GXBOOL)wParam);

  case GXTVM_ENSUREVISIBLE:
    return TREEVIEW_EnsureVisible(infoPtr, (GXHTREEITEM)lParam, TRUE);

  case GXTVM_EXPAND:
    return TREEVIEW_ExpandMsg(infoPtr, (GXUINT)wParam, (GXHTREEITEM)lParam);

  case GXTVM_GETBKCOLOR:
    return TREEVIEW_GetBkColor(infoPtr);

  case GXTVM_GETCOUNT:
    return TREEVIEW_GetCount(infoPtr);

  case GXTVM_GETEDITCONTROL:
    return TREEVIEW_GetEditControl(infoPtr);

  case GXTVM_GETIMAGELIST:
    return TREEVIEW_GetImageList(infoPtr, wParam);

  case GXTVM_GETINDENT:
    return TREEVIEW_GetIndent(infoPtr);

  case GXTVM_GETINSERTMARKCOLOR:
    return TREEVIEW_GetInsertMarkColor(infoPtr);

  case GXTVM_GETISEARCHSTRINGA:
    FIXME("Unimplemented msg TVM_GETISEARCHSTRINGA\n");
    return 0;

  case GXTVM_GETISEARCHSTRINGW:
    FIXME("Unimplemented msg TVM_GETISEARCHSTRINGW\n");
    return 0;

  case GXTVM_GETITEMA:
    return TREEVIEW_GetItemT(infoPtr, (GXLPTVITEMEXW)lParam, FALSE);

  case GXTVM_GETITEMW:
    return TREEVIEW_GetItemT(infoPtr, (GXLPTVITEMEXW)lParam, TRUE);

  case GXTVM_GETITEMHEIGHT:
    return TREEVIEW_GetItemHeight(infoPtr);

  case GXTVM_GETITEMRECT:
    return TREEVIEW_GetItemRect(infoPtr, (GXBOOL)wParam, (LPGXRECT)lParam);

  case GXTVM_GETITEMSTATE:
    return TREEVIEW_GetItemState(infoPtr, (GXHTREEITEM)wParam, (GXUINT)lParam);

  case GXTVM_GETLINECOLOR:
    return TREEVIEW_GetLineColor(infoPtr);

  case GXTVM_GETNEXTITEM:
    return TREEVIEW_GetNextItem(infoPtr, (GXUINT)wParam, (GXHTREEITEM)lParam);

  case GXTVM_GETSCROLLTIME:
    return TREEVIEW_GetScrollTime(infoPtr);

  case GXTVM_GETTEXTCOLOR:
    return TREEVIEW_GetTextColor(infoPtr);

  case GXTVM_GETTOOLTIPS:
    return TREEVIEW_GetToolTips(infoPtr);

  case GXTVM_GETUNICODEFORMAT:
    return TREEVIEW_GetUnicodeFormat(infoPtr);

  case GXTVM_GETVISIBLECOUNT:
    return TREEVIEW_GetVisibleCount(infoPtr);

  case GXTVM_HITTEST:
    return TREEVIEW_HitTest(infoPtr, (GXLPTVHITTESTINFO)lParam);

  case GXTVM_INSERTITEMA:
    return TREEVIEW_InsertItemT(infoPtr, (GXLPTVINSERTSTRUCTW)lParam, FALSE);

  case GXTVM_INSERTITEMW:
    return TREEVIEW_InsertItemT(infoPtr, (GXLPTVINSERTSTRUCTW)lParam, TRUE);

  case GXTVM_SELECTITEM:
    return TREEVIEW_SelectItem(infoPtr, (GXINT)wParam, (GXHTREEITEM)lParam);

  case GXTVM_SETBKCOLOR:
    return TREEVIEW_SetBkColor(infoPtr, (GXCOLORREF)lParam);

  case GXTVM_SETIMAGELIST:
    return TREEVIEW_SetImageList(infoPtr, wParam, (GXHIMAGELIST)lParam);

  case GXTVM_SETINDENT:
    return TREEVIEW_SetIndent(infoPtr, (GXUINT)wParam);

  case GXTVM_SETINSERTMARK:
    return TREEVIEW_SetInsertMark(infoPtr, (GXBOOL)wParam, (GXHTREEITEM)lParam);

  case GXTVM_SETINSERTMARKCOLOR:
    return TREEVIEW_SetInsertMarkColor(infoPtr, (GXCOLORREF)lParam);

  case GXTVM_SETITEMA:
    return TREEVIEW_SetItemT(infoPtr, (GXLPTVITEMEXW)lParam, FALSE);

  case GXTVM_SETITEMW:
    return TREEVIEW_SetItemT(infoPtr, (GXLPTVITEMEXW)lParam, TRUE);

  case GXTVM_SETLINECOLOR:
    return TREEVIEW_SetLineColor(infoPtr, (GXCOLORREF)lParam);

  case GXTVM_SETITEMHEIGHT:
    return TREEVIEW_SetItemHeight(infoPtr, (GXINT)(GXSHORT)wParam);

  case GXTVM_SETSCROLLTIME:
    return TREEVIEW_SetScrollTime(infoPtr, (GXUINT)wParam);

  case GXTVM_SETTEXTCOLOR:
    return TREEVIEW_SetTextColor(infoPtr, (GXCOLORREF)lParam);

  case GXTVM_SETTOOLTIPS:
    return TREEVIEW_SetToolTips(infoPtr, (GXHWND)wParam);

  case GXTVM_SETUNICODEFORMAT:
    return TREEVIEW_SetUnicodeFormat(infoPtr, (GXBOOL)wParam);

  case GXTVM_SORTCHILDREN:
    return TREEVIEW_SortChildren(infoPtr, wParam, lParam);

  case GXTVM_SORTCHILDRENCB:
    return TREEVIEW_SortChildrenCB(infoPtr, wParam, (GXLPTVSORTCB)lParam);

  case GXWM_CHAR:
    return TREEVIEW_ProcessLetterKeys( hwnd, wParam, lParam );

  case GXWM_COMMAND:
    return TREEVIEW_Command(infoPtr, wParam, lParam);

  case GXWM_DESTROY:
    return TREEVIEW_Destroy(infoPtr);

    /* WM_ENABLE */

  case GXWM_ERASEBKGND:
    return TREEVIEW_EraseBackground(infoPtr, (GXHDC)wParam);

  case GXWM_GETDLGCODE:
    return GXDLGC_WANTARROWS | GXDLGC_WANTCHARS;

  case GXWM_GETFONT:
    return TREEVIEW_GetFont(infoPtr);

  case GXWM_HSCROLL:
    return TREEVIEW_HScroll(infoPtr, wParam);

  case GXWM_KEYDOWN:
    return TREEVIEW_KeyDown(infoPtr, wParam);

  case GXWM_KILLFOCUS:
    return TREEVIEW_KillFocus(infoPtr);

  case GXWM_LBUTTONDBLCLK:
    return TREEVIEW_LButtonDoubleClick(infoPtr, lParam);

  case GXWM_LBUTTONDOWN:
    return TREEVIEW_LButtonDown(infoPtr, lParam);

    /* WM_MBUTTONDOWN */

  case GXWM_MOUSELEAVE:
    return TREEVIEW_MouseLeave(infoPtr);

  case GXWM_MOUSEMOVE:
    if (infoPtr->dwStyle & TVS_TRACKSELECT)
      return TREEVIEW_MouseMove(infoPtr, wParam, lParam);
    else
      return 0;

  case GXWM_NCLBUTTONDOWN:
    if (infoPtr->hwndEdit)
      gxSetFocus(infoPtr->hwnd);
    goto def;

  case GXWM_NCPAINT:
    if (nc_paint (infoPtr, (GXHRGN)wParam))
      return 0;
    goto def;

  case GXWM_NOTIFY:
    return TREEVIEW_Notify(infoPtr, wParam, lParam);

  case GXWM_NOTIFYFORMAT:
    return TREEVIEW_NotifyFormat(infoPtr, (GXHWND)wParam, (GXUINT)lParam);

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    return TREEVIEW_Paint(infoPtr, wParam);

  case GXWM_RBUTTONDOWN:
    return TREEVIEW_RButtonDown(infoPtr, lParam);

  case GXWM_SETCURSOR:
    return TREEVIEW_SetCursor(infoPtr, wParam, lParam);

  case GXWM_SETFOCUS:
    return TREEVIEW_SetFocus(infoPtr);

  case GXWM_SETFONT:
    return TREEVIEW_SetFont(infoPtr, (GXHFONT)wParam, (GXBOOL)lParam);

  case GXWM_SETREDRAW:
    return TREEVIEW_SetRedraw(infoPtr, wParam, lParam);

  case GXWM_SIZE:
    return TREEVIEW_Size(infoPtr, wParam, lParam);

  case GXWM_STYLECHANGED:
    return TREEVIEW_StyleChanged(infoPtr, wParam, lParam);

    /* WM_SYSCOLORCHANGE */

    /* WM_SYSKEYDOWN */

  case GXWM_TIMER:
    return TREEVIEW_HandleTimer(infoPtr, wParam);

  case GXWM_THEMECHANGED:
    return theme_changed (infoPtr);

  case GXWM_VSCROLL:
    return TREEVIEW_VScroll(infoPtr, wParam);

    /* WM_WININICHANGE */

  case GXWM_MOUSEWHEEL:
    if (wParam & (GXMK_SHIFT | GXMK_CONTROL))
      goto def;
    return TREEVIEW_MouseWheel(infoPtr, wParam);

  case GXWM_DRAWITEM:
    TRACE("drawItem\n");
    goto def;

  default:
    /* This mostly catches MFC and Delphi messages. :( */
    if ((uMsg >= GXWM_USER) && (uMsg < GXWM_APP))
      TRACE("Unknown msg %04x wp=%08lx lp=%08lx\n", uMsg, wParam, lParam);
def:
    return gxDefWindowProcW(hwnd, uMsg, wParam, lParam);
  }
}


/* Class Registration ***************************************************/

GXVOID
TREEVIEW_Register()
{
  GXWNDCLASSEX wndClass;

  TRACE("\n");

  ZeroMemory(&wndClass, sizeof(GXWNDCLASSEX));
  wndClass.style = GXCS_GLOBALCLASS | GXCS_DBLCLKS;
  wndClass.lpfnWndProc = TREEVIEW_WindowProc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = sizeof(GXTREEVIEW_INFO *);

  wndClass.hCursor = gxLoadCursorW(0, (GXLPWSTR)GXIDC_ARROW);
  wndClass.hbrBackground = 0;
  wndClass.lpszClassName = WC_TREEVIEWW;

  gxRegisterClassExW(&wndClass);
}


GXVOID
TREEVIEW_Unregister()
{
  gxUnregisterClassW(WC_TREEVIEWW, NULL);
}

//static inline int lstrncmpiW(GXLPCWSTR s1, GXLPCWSTR s2, int n)
//{
//  return GXSTRNCMPI(s1, s2, n);
//  //int res;
//
//  //n = min(min(n, GXSTRLEN(s1)), GXSTRLEN(s2));
//  //res = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, s1, n, s2, n);
//  //return res ? res - sizeof(GXWCHAR) : res;
//}

/* Tree Verification ****************************************************/

#ifdef NDEBUG
static inline void
TREEVIEW_VerifyChildren(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item);

static inline void TREEVIEW_VerifyItemCommon(GXTREEVIEW_INFO *infoPtr,
                       GXTREEVIEW_ITEM *item)
{
  assert(infoPtr != NULL);
  assert(item != NULL);

  /* both NULL, or both non-null */
  assert((item->firstChild == NULL) == (item->lastChild == NULL));

  assert(item->firstChild != item);
  assert(item->lastChild != item);

  if (item->firstChild)
  {
    assert(item->firstChild->parent == item);
    assert(item->firstChild->prevSibling == NULL);
  }

  if (item->lastChild)
  {
    assert(item->lastChild->parent == item);
    assert(item->lastChild->nextSibling == NULL);
  }

  assert(item->nextSibling != item);
  if (item->nextSibling)
  {
    assert(item->nextSibling->parent == item->parent);
    assert(item->nextSibling->prevSibling == item);
  }

  assert(item->prevSibling != item);
  if (item->prevSibling)
  {
    assert(item->prevSibling->parent == item->parent);
    assert(item->prevSibling->nextSibling == item);
  }
}

static inline void
TREEVIEW_VerifyItem(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  assert(item != NULL);

  assert(item->parent != NULL);
  assert(item->parent != item);
  assert(item->iLevel == item->parent->iLevel + 1);

  assert(gxDPA_GetPtrIndex(infoPtr->items, item) != -1);

  TREEVIEW_VerifyItemCommon(infoPtr, item);

  TREEVIEW_VerifyChildren(infoPtr, item);
}

static inline void
TREEVIEW_VerifyChildren(GXTREEVIEW_INFO *infoPtr, GXTREEVIEW_ITEM *item)
{
  GXTREEVIEW_ITEM *child;
  assert(item != NULL);

  for (child = item->firstChild; child != NULL; child = child->nextSibling)
    TREEVIEW_VerifyItem(infoPtr, child);
}

static inline void
TREEVIEW_VerifyRoot(GXTREEVIEW_INFO *infoPtr)
{
  GXTREEVIEW_ITEM *root = infoPtr->root;

  assert(root != NULL);
  assert(root->iLevel == -1);
  assert(root->parent == NULL);
  assert(root->prevSibling == NULL);

  TREEVIEW_VerifyItemCommon(infoPtr, root);

  TREEVIEW_VerifyChildren(infoPtr, root);
}

static void
TREEVIEW_VerifyTree(GXTREEVIEW_INFO *infoPtr)
{
  assert(infoPtr != NULL);

  TREEVIEW_VerifyRoot(infoPtr);
}
#endif

#endif // _DEV_DISABLE_UI_CODE