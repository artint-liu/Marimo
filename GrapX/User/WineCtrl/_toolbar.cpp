#ifndef _DEV_DISABLE_UI_CODE
#ifndef _TOOLBAR_
/*
* Toolbar control
*
* Copyright 1998,1999 Eric Kohl
* Copyright 2000 Eric Kohl for CodeWeavers
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
* of Comctl32.dll version 6.0 on Mar. 14, 2004, by Robert Shearman.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features or bugs please note them below.
* 
* TODO:
*   - Styles:
*     - TBSTYLE_REGISTERDROP
*     - TBSTYLE_EX_DOUBLEBUFFER
*   - Messages:
*     - TB_GETMETRICS
*     - TB_GETOBJECT
*     - TB_INSERTMARKHITTEST
*     - TB_SAVERESTORE
*     - TB_SETMETRICS
*     - WM_WININICHANGE
*   - Notifications:
*     - NM_CHAR
*     - GXTBN_GETOBJECT
*     - GXTBN_SAVE
*   - Button wrapping (under construction).
*   - Fix TB_SETROWS and Separators.
*   - iListGap custom draw support.
*
* Testing:
*   - Run tests using Waite Group Windows95 API Bible Volume 2.
*     The second cdrom contains executables addstr.exe, btncount.exe,
*     btnstate.exe, butstrsz.exe, chkbtn.exe, chngbmp.exe, customiz.exe,
*     enablebtn.exe, getbmp.exe, getbtn.exe, getflags.exe, hidebtn.exe,
*     indetbtn.exe, insbtn.exe, pressbtn.exe, setbtnsz.exe, setcmdid.exe,
*     setparnt.exe, setrows.exe, toolwnd.exe.
*   - Microsoft's controlspy examples.
*   - Charles Petzold's 'Programming Windows': gadgets.exe
*
*  Differences between MSDN and actual native control operation:
*   1. MSDN says: "TBSTYLE_LIST: Creates a flat toolbar with button text
*                  to the right of the bitmap. Otherwise, this style is
*                  identical to TBSTYLE_FLAT."
*      As implemented by both v4.71 and v5.80 of the native COMCTL32.DLL
*      you can create a TBSTYLE_LIST without TBSTYLE_FLAT and the result
*      is non-flat non-transparent buttons. Therefore TBSTYLE_LIST does
*      *not* imply TBSTYLE_FLAT as documented.  (GA 8/2001)
*
*/

//#include <stdarg.h>
//#include <string.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "winreg.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "wine/unicode.h"
//#include "winnls.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "uxtheme.h"
//#include "tmschema.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(toolbar);

#include <GrapX.h>
#include <User/GrapXDef.h>
#include "GrapX/gUxtheme.h"
#include "GrapX/GXUser.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXImm.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.h>
#include <User/Win32Emu/GXCommCtrl.h>
#include "res/resource.h"

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

#define DISABLE_CODE

GXBOOL GXDLLAPI gxMakeDragList (GXHWND hwndLB);
GXVOID GXDLLAPI gxDrawInsert (GXHWND hwndParent, GXHWND hwndLB, GXINT nItem);
GXINT GXDLLAPI gxLBItemFromPt (GXHWND hwndLB, GXPOINT pt, GXBOOL bAutoScroll);

static GXHCURSOR hCursorDrag = NULL;

typedef struct
{
  GXINT iBitmap;
  GXINT idCommand;
  GXBYTE  fsState;
  GXBYTE  fsStyle;
  GXBYTE  bHot;
  GXBYTE  bDropDownPressed;
  GXDWORD_PTR dwData;
  GXINT_PTR iString;
  GXINT nRow;
  GXRECT rect;
  GXINT cx; /* manually set size */
} TBUTTON_INFO;

typedef struct
{
  GXUINT nButtons;
  GXHINSTANCE hInst;
  GXUINT nID;
} TBITMAP_INFO;

typedef struct
{
  GXHIMAGELIST himl;
  GXINT id;
} IMLENTRY, *PIMLENTRY;

typedef struct
{
  GXDWORD    dwStructSize;    /* size of GXTBBUTTON struct */
  GXINT      nWidth;          /* width of the toolbar */
  GXRECT     client_rect;
  GXRECT     rcBound;         /* bounding rectangle */
  GXINT      nButtonHeight;
  GXINT      nButtonWidth;
  GXINT      nBitmapHeight;
  GXINT      nBitmapWidth;
  GXINT      nIndent;
  GXINT      nRows;           /* number of button rows */
  GXINT      nMaxTextRows;    /* maximum number of text rows */
  GXINT      cxMin;           /* minimum button width */
  GXINT      cxMax;           /* maximum button width */
  GXINT      nNumButtons;     /* number of buttons */
  GXINT      nNumBitmaps;     /* number of bitmaps */
  GXINT      nNumStrings;     /* number of strings */
  GXINT      nNumBitmapInfos;
  GXINT      nButtonDown;     /* toolbar button being pressed or -1 if none */
  GXINT      nButtonDrag;     /* toolbar button being dragged or -1 if none */
  GXINT      nOldHit;
  GXINT      nHotItem;        /* index of the "hot" item */
  GXSIZE     szPadding;       /* padding values around button */
  GXINT      iTopMargin;      /* the top margin */
  GXINT      iListGap;        /* default gap between text and image for toolbar with list style */
  GXHFONT    hDefaultFont;
  GXHFONT    hFont;           /* text font */
  GXHIMAGELIST himlInt;       /* image list created internally */
  PIMLENTRY *himlDef;       /* default image list array */
  GXINT       cimlDef;        /* default image list array count */
  PIMLENTRY *himlHot;       /* hot image list array */
  GXINT       cimlHot;        /* hot image list array count */
  PIMLENTRY *himlDis;       /* disabled image list array */
  GXINT       cimlDis;        /* disabled image list array count */
  GXHWND     hwndToolTip;     /* handle to tool tip control */
  GXHWND     hwndNotify;      /* handle to the window that gets notifications */
  GXHWND     hwndSelf;        /* my own handle */
  GXBOOL     bAnchor;         /* anchor highlight enabled */
  GXBOOL     bDoRedraw;       /* Redraw status */
  GXBOOL     bDragOutSent;    /* has GXTBN_DRAGOUT notification been sent for this drag? */
  GXBOOL     bUnicode;        /* Notifications are ASCII (FALSE) or Unicode (TRUE)? */
  GXBOOL     bCaptured;       /* mouse captured? */
  GXDWORD      dwStyle;       /* regular toolbar style */
  GXDWORD      dwExStyle;     /* extended toolbar style */
  GXDWORD      dwDTFlags;     /* DrawText flags */

  GXCOLORREF   clrInsertMark;   /* insert mark color */
  GXCOLORREF   clrBtnHighlight; /* color for Flat Separator */
  GXCOLORREF   clrBtnShadow;    /* color for Flag Separator */
  GXINT      iVersion;
  GXLPWSTR   pszTooltipText;    /* temporary store for a string > 80 characters
                  * for TTN_GETDISPINFOW notification */
  GXTBINSERTMARK  tbim;         /* info on insertion mark */
  TBUTTON_INFO *buttons;      /* pointer to button array */
  GXLPWSTR       *strings;      /* pointer to string array */
  TBITMAP_INFO *bitmaps;
} TOOLBAR_INFO, *PTOOLBAR_INFO;


/* used by customization dialog */
typedef struct
{
  PTOOLBAR_INFO tbInfo;
  GXHWND          tbHwnd;
} CUSTDLG_INFO, *PCUSTDLG_INFO;

typedef struct
{
  GXTBBUTTON btn;
  GXBOOL     bVirtual;
  GXBOOL     bRemovable;
  GXWCHAR    text[64];
} CUSTOMBUTTON, *PCUSTOMBUTTON;

typedef enum
{
  IMAGE_LIST_DEFAULT,
  IMAGE_LIST_HOT,
  IMAGE_LIST_DISABLED
} IMAGE_LIST_TYPE;

#define SEPARATOR_WIDTH    8
#define TOP_BORDER         2
#define BOTTOM_BORDER      2
#define DDARROW_WIDTH      11
#define ARROW_HEIGHT       3
#define INSERTMARK_WIDTH   2

#define DEFPAD_CX 7
#define DEFPAD_CY 6
#define DEFLISTGAP 4

/* vertical padding used in list mode when image is present */
#define LISTPAD_CY 9

/* how wide to treat the bitmap if it isn't present */
#define NONLIST_NOTEXT_OFFSET 2

#define TOOLBAR_NOWHERE (-1)

#define TOOLBAR_GetInfoPtr(hwnd) ((TOOLBAR_INFO *)gxGetWindowLongPtrW(hwnd,0))
#define TOOLBAR_HasText(x, y) (TOOLBAR_GetText(x, y) ? TRUE : FALSE)
#define TOOLBAR_HasDropDownArrows(exStyle) ((exStyle & TBSTYLE_EX_DRAWDDARROWS) ? TRUE : FALSE)

/* Used to find undocumented extended styles */
#define TBSTYLE_EX_ALL (TBSTYLE_EX_DRAWDDARROWS | \
  TBSTYLE_EX_UNDOC1 | \
  TBSTYLE_EX_MIXEDBUTTONS | \
  TBSTYLE_EX_DOUBLEBUFFER | \
  TBSTYLE_EX_HIDECLIPPEDBUTTONS)

/* all of the CCS_ styles */
#define COMMON_STYLES (CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM|CCS_NORESIZE| \
  CCS_NOPARENTALIGN|CCS_ADJUSTABLE|CCS_NODIVIDER|CCS_VERT)

#define GETIBITMAP(infoPtr, i) (infoPtr->iVersion >= 5 ? GXLOWORD(i) : i)
#define GETHIMLID(infoPtr, i) (infoPtr->iVersion >= 5 ? GXHIWORD(i) : 0)
#define GETDEFIMAGELIST(infoPtr, id) TOOLBAR_GetImageList(infoPtr->himlDef, infoPtr->cimlDef, id)
#define GETHOTIMAGELIST(infoPtr, id) TOOLBAR_GetImageList(infoPtr->himlHot, infoPtr->cimlHot, id)
#define GETDISIMAGELIST(infoPtr, id) TOOLBAR_GetImageList(infoPtr->himlDis, infoPtr->cimlDis, id)

static const GXWCHAR themeClass[] = { 'T','o','o','l','b','a','r',0 };

static GXBOOL TOOLBAR_GetButtonInfo(const TOOLBAR_INFO *infoPtr, GXNMTOOLBARW *nmtb);
static GXBOOL TOOLBAR_IsButtonRemovable(const TOOLBAR_INFO *infoPtr, GXINT iItem, PCUSTOMBUTTON btnInfo);
static GXHIMAGELIST TOOLBAR_GetImageList(const PIMLENTRY *pies, GXINT cies, GXINT id);
static PIMLENTRY TOOLBAR_GetImageListEntry(const PIMLENTRY *pies, GXINT cies, GXINT id);
static GXVOID TOOLBAR_DeleteImageList(PIMLENTRY **pies, GXINT *cies);
static GXHIMAGELIST TOOLBAR_InsertImageList(PIMLENTRY **pies, GXINT *cies, GXHIMAGELIST himl, GXINT id);
static GXLRESULT TOOLBAR_LButtonDown(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam);
static void TOOLBAR_SetHotItemEx (TOOLBAR_INFO *infoPtr, GXINT nHit, GXDWORD dwReason);
static void TOOLBAR_LayoutToolbar(GXHWND hwnd);
static GXLRESULT TOOLBAR_AutoSize(GXHWND hwnd);
static void TOOLBAR_CheckImageListIconSize(TOOLBAR_INFO *infoPtr);
static void TOOLBAR_TooltipAddTool(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *button);
static void TOOLBAR_TooltipSetRect(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *button);

static GXLRESULT
TOOLBAR_NotifyFormat(const TOOLBAR_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam);

static inline GXINT default_top_margin(const TOOLBAR_INFO *infoPtr)
{
  return (infoPtr->dwStyle & TBSTYLE_FLAT ? 0 : TOP_BORDER);
}

static GXLPWSTR
TOOLBAR_GetText(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *btnPtr)
{
  GXLPWSTR lpText = NULL;

  /* NOTE: iString == -1 is undocumented */
  if ((GXHIWORD(btnPtr->iString) != 0) && (btnPtr->iString != -1))
    lpText = (GXLPWSTR)btnPtr->iString;
  else if ((btnPtr->iString >= 0) && (btnPtr->iString < infoPtr->nNumStrings))
    lpText = infoPtr->strings[btnPtr->iString];

  return lpText;
}

static void
TOOLBAR_DumpTBButton(const GXTBBUTTON *tbb, GXBOOL fUnicode)
{
  //TRACE("TBBUTTON: id %d, bitmap=%d, state=%02x, style=%02x, data=%08lx, stringid=0x%08lx (%s)\n",
  //      tbb->idCommand,tbb->iBitmap, tbb->fsState, tbb->fsStyle, tbb->dwData, tbb->iString,
  //      (fUnicode ? wine_dbgstr_w((GXLPWSTR)tbb->iString) : wine_dbgstr_a((GXLPSTR)tbb->iString)));
}

static void
TOOLBAR_DumpButton(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *bP, GXINT btn_num)
{
  //   if (TRACE_ON(toolbar)){
  //       TRACE("button %d id %d, bitmap=%d, state=%02x, style=%02x, data=%08lx, stringid=0x%08lx\n",
  //             btn_num, bP->idCommand, GETIBITMAP(infoPtr, bP->iBitmap), 
  //             bP->fsState, bP->fsStyle, bP->dwData, bP->iString);
  //TRACE("string %s\n", debugstr_w(TOOLBAR_GetText(infoPtr,bP)));
  //       TRACE("button %d id %d, hot=%s, row=%d, rect=(%s)\n",
  //             btn_num, bP->idCommand, (bP->bHot) ? "TRUE":"FALSE", bP->nRow,
  //             wine_dbgstr_rect(&bP->rect));
  //   }
}


static void
TOOLBAR_DumpToolbar(const TOOLBAR_INFO *iP, GXINT line)
{
  //   if (TRACE_ON(toolbar)) {
  //GXINT i;

  //TRACE("toolbar %p at line %d, exStyle=%08x, buttons=%d, bitmaps=%d, strings=%d, style=%08x\n",
  //      iP->hwndSelf, line,
  //      iP->dwExStyle, iP->nNumButtons, iP->nNumBitmaps,
  //      iP->nNumStrings, iP->dwStyle);
  //TRACE("toolbar %p at line %d, himlInt=%p, himlDef=%p, himlHot=%p, himlDis=%p, redrawable=%s\n",
  //      iP->hwndSelf, line,
  //      iP->himlInt, iP->himlDef, iP->himlHot, iP->himlDis,
  //      (iP->bDoRedraw) ? "TRUE" : "FALSE");
  //  for(i=0; i<iP->nNumButtons; i++) {
  //           TOOLBAR_DumpButton(iP, &iP->buttons[i], i);
  //}
  //   }
}


/***********************************************************************
*     TOOLBAR_CheckStyle
*
* This function validates that the styles set are implemented and
* issues FIXME's warning of possible problems. In a perfect world this
* function should be null.
*/
static void
TOOLBAR_CheckStyle (GXHWND hwnd, GXDWORD dwStyle)
{
  if (dwStyle & TBSTYLE_REGISTERDROP)
    FIXME("[%p] TBSTYLE_REGISTERDROP not implemented\n", hwnd);
}


static GXINT
TOOLBAR_SendNotify (GXNMHDR *nmhdr, const TOOLBAR_INFO *infoPtr, GXUINT code)
{
  if(!gxIsWindow(infoPtr->hwndSelf))
    return 0;   /* we have just been destroyed */

  nmhdr->idFrom = gxGetDlgCtrlID (infoPtr->hwndSelf);
  nmhdr->hwndFrom = infoPtr->hwndSelf;
  nmhdr->code = code;

  TRACE("to window %p, code=%08x, %s\n", infoPtr->hwndNotify, code,
    (infoPtr->bUnicode) ? "via Unicode" : "via ANSI");

  return gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY, nmhdr->idFrom, (GXLPARAM)nmhdr);
}

/***********************************************************************
*     TOOLBAR_GetBitmapIndex
*
* This function returns the bitmap index associated with a button.
* If the button specifies GXI_IMAGECALLBACK, then the GXTBN_GETDISPINFO
* is issued to retrieve the index.
*/
static GXINT
TOOLBAR_GetBitmapIndex(const TOOLBAR_INFO *infoPtr, TBUTTON_INFO *btnPtr)
{
  GXINT ret = btnPtr->iBitmap;

  if (ret == GXI_IMAGECALLBACK)
  {
    /* issue GXTBN_GETDISPINFO */
    GXNMTBDISPINFOW nmgd;

    memset(&nmgd, 0, sizeof(nmgd));
    nmgd.idCommand = btnPtr->idCommand;
    nmgd.lParam = btnPtr->dwData;
    nmgd.dwMask = TBNF_IMAGE;
    nmgd.iImage = -1;
    /* Windows also send GXTBN_GETDISPINFOW even if the control is ANSI */
    TOOLBAR_SendNotify(&nmgd.hdr, infoPtr, GXTBN_GETDISPINFOW);
    if (nmgd.dwMask & TBNF_DI_SETITEM)
      btnPtr->iBitmap = nmgd.iImage;
    ret = nmgd.iImage;
    TRACE("GXTBN_GETDISPINFO returned bitmap id %d, mask=%08x, nNumBitmaps=%d\n",
      ret, nmgd.dwMask, infoPtr->nNumBitmaps);
  }

  if (ret != GXI_IMAGENONE)
    ret = GETIBITMAP(infoPtr, ret);

  return ret;
}


static GXBOOL
TOOLBAR_IsValidBitmapIndex(const TOOLBAR_INFO *infoPtr, GXINT index)
{
  GXHIMAGELIST himl;
  GXINT id = GETHIMLID(infoPtr, index);
  GXINT iBitmap = GETIBITMAP(infoPtr, index);

  if (((himl = GETDEFIMAGELIST(infoPtr, id)) &&
    iBitmap >= 0 && iBitmap < gxImageList_GetImageCount(himl)) ||
    (index == GXI_IMAGECALLBACK))
    return TRUE;
  else
    return FALSE;
}


static inline GXBOOL
TOOLBAR_IsValidImageList(const TOOLBAR_INFO *infoPtr, GXINT index)
{
  GXHIMAGELIST himl = GETDEFIMAGELIST(infoPtr, GETHIMLID(infoPtr, index));
  return (himl != NULL) && (gxImageList_GetImageCount(himl) > 0);
}


/***********************************************************************
*     TOOLBAR_GetImageListForDrawing
*
* This function validates the bitmap index (including GXI_IMAGECALLBACK
* functionality) and returns the corresponding image list.
*/
static GXHIMAGELIST
TOOLBAR_GetImageListForDrawing (const TOOLBAR_INFO *infoPtr, TBUTTON_INFO *btnPtr,
                IMAGE_LIST_TYPE imagelist, GXINT * index)
{
  GXHIMAGELIST himl;

  if (!TOOLBAR_IsValidBitmapIndex(infoPtr,btnPtr->iBitmap)) {
    if (btnPtr->iBitmap == GXI_IMAGENONE) return NULL;
    ERR("bitmap for ID %d, index %d is not valid, number of bitmaps in imagelist: %d\n",
      GXHIWORD(btnPtr->iBitmap), GXLOWORD(btnPtr->iBitmap), infoPtr->nNumBitmaps);
    return NULL;
  }

  if ((*index = TOOLBAR_GetBitmapIndex(infoPtr, btnPtr)) < 0) {
    if ((*index == GXI_IMAGECALLBACK) ||
      (*index == GXI_IMAGENONE)) return NULL;
    ERR("GXTBN_GETDISPINFO returned invalid index %d\n",
      *index);
    return NULL;
  }

  switch(imagelist)
  {
  case IMAGE_LIST_DEFAULT:
    himl = GETDEFIMAGELIST(infoPtr, GETHIMLID(infoPtr, btnPtr->iBitmap));
    break;
  case IMAGE_LIST_HOT:
    himl = GETHOTIMAGELIST(infoPtr, GETHIMLID(infoPtr, btnPtr->iBitmap));
    break;
  case IMAGE_LIST_DISABLED:
    himl = GETDISIMAGELIST(infoPtr, GETHIMLID(infoPtr, btnPtr->iBitmap));
    break;
  default:
    himl = NULL;
    FIXME("Shouldn't reach here\n");
  }

  if (!himl)
    TRACE("no image list\n");

  return himl;
}


static void
TOOLBAR_DrawFlatSeparator (const GXRECT *lpRect, GXHDC hdc, const TOOLBAR_INFO *infoPtr)
{
  GXRECT myrect;
  GXCOLORREF oldcolor, newcolor;

  myrect.left = (lpRect->left + lpRect->right) / 2 - 1;
  myrect.right = myrect.left + 1;
  myrect.top = lpRect->top + 2;
  myrect.bottom = lpRect->bottom - 2;

  newcolor = (infoPtr->clrBtnShadow == CLR_DEFAULT) ?
    comctl32_color.clrBtnShadow : infoPtr->clrBtnShadow;
  oldcolor = gxSetBkColor (hdc, newcolor);
  gxExtTextOutW (hdc, 0, 0, GXETO_OPAQUE, &myrect, 0, 0, 0);

  myrect.left = myrect.right;
  myrect.right = myrect.left + 1;

  newcolor = (infoPtr->clrBtnHighlight == CLR_DEFAULT) ?
    comctl32_color.clrBtnHighlight : infoPtr->clrBtnHighlight;
  gxSetBkColor (hdc, newcolor);
  gxExtTextOutW (hdc, 0, 0, GXETO_OPAQUE, &myrect, 0, 0, 0);

  gxSetBkColor (hdc, oldcolor);
}


/***********************************************************************
*     TOOLBAR_DrawDDFlatSeparator
*
* This function draws the separator that was flagged as BTNS_DROPDOWN.
* In this case, the separator is a pixel high line of GXCOLOR_BTNSHADOW,
* followed by a pixel high line of GXCOLOR_BTNHIGHLIGHT. These separators
* are horizontal as opposed to the vertical separators for not dropdown
* type.
*
* FIXME: It is possible that the height of each line is really SM_CYBORDER.
*/
static void
TOOLBAR_DrawDDFlatSeparator (const GXRECT *lpRect, GXHDC hdc,
               const TOOLBAR_INFO *infoPtr)
{
  GXRECT myrect;
  GXCOLORREF oldcolor, newcolor;

  myrect.left = lpRect->left;
  myrect.right = lpRect->right;
  myrect.top = lpRect->top + (lpRect->bottom - lpRect->top - 2)/2;
  myrect.bottom = myrect.top + 1;

  gxInflateRect (&myrect, -2, 0);

  TRACE("rect=(%s)\n", wine_dbgstr_rect(&myrect));

  newcolor = (infoPtr->clrBtnShadow == CLR_DEFAULT) ?
    comctl32_color.clrBtnShadow : infoPtr->clrBtnShadow;
  oldcolor = gxSetBkColor (hdc, newcolor);
  gxExtTextOutW (hdc, 0, 0, GXETO_OPAQUE, &myrect, 0, 0, 0);

  myrect.top = myrect.bottom;
  myrect.bottom = myrect.top + 1;

  newcolor = (infoPtr->clrBtnHighlight == CLR_DEFAULT) ?
    comctl32_color.clrBtnHighlight : infoPtr->clrBtnHighlight;
  gxSetBkColor (hdc, newcolor);
  gxExtTextOutW (hdc, 0, 0, GXETO_OPAQUE, &myrect, 0, 0, 0);

  gxSetBkColor (hdc, oldcolor);
}


static void
TOOLBAR_DrawArrow (GXHDC hdc, GXINT left, GXINT top, GXCOLORREF clr)
{
  GXINT x, y;
  GXHPEN hPen, hOldPen;

  if (!(hPen = gxCreatePen( GXPS_SOLID, 1, clr))) return;
  hOldPen = (GXHPEN)gxSelectObject ( hdc, hPen );
  x = left + 2;
  y = top;
  gxMoveToEx (hdc, x, y, NULL);
  gxLineTo (hdc, x+5, y++); x++;
  gxMoveToEx (hdc, x, y, NULL);
  gxLineTo (hdc, x+3, y++); x++;
  gxMoveToEx (hdc, x, y, NULL);
  gxLineTo (hdc, x+1, y);
  gxSelectObject( hdc, hOldPen );
  gxDeleteObject( hPen );
}

/*
* Draw the text string for this button.
* note: infoPtr->himlDis *SHOULD* be non-zero when infoPtr->himlDef
*   is non-zero, so we can simply check himlDef to see if we have
*      an image list
*/
static void
TOOLBAR_DrawString (const TOOLBAR_INFO *infoPtr, GXRECT *rcText, GXLPCWSTR lpText,
          const GXNMTBCUSTOMDRAW *tbcd, GXDWORD dwItemCDFlag)
{
  GXHDC hdc = tbcd->nmcd.hdc;
  GXHFONT  hOldFont = 0;
  GXCOLORREF clrOld = 0;
  GXCOLORREF clrOldBk = 0;
  GXINT oldBkMode = 0;
  GXUINT state = tbcd->nmcd.uItemState;

  /* draw text */
  if (lpText) {
    TRACE("string=%s rect=(%s)\n", debugstr_w(lpText),
      wine_dbgstr_rect(rcText));

    hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hFont);
    if ((state & CDIS_HOT) && (dwItemCDFlag & TBCDRF_HILITEHOTTRACK )) {
      clrOld = gxSetTextColor (hdc, tbcd->clrTextHighlight);
    }
    else if (state & CDIS_DISABLED) {
      clrOld = gxSetTextColor (hdc, tbcd->clrBtnHighlight);
      gxOffsetRect (rcText, 1, 1);
      gxDrawTextW (hdc, lpText, -1, rcText, infoPtr->dwDTFlags);
      gxSetTextColor (hdc, comctl32_color.clr3dShadow);
      gxOffsetRect (rcText, -1, -1);
    }
    else if (state & CDIS_INDETERMINATE) {
      clrOld = gxSetTextColor (hdc, comctl32_color.clr3dShadow);
    }
    else if ((state & CDIS_MARKED) && !(dwItemCDFlag & TBCDRF_NOMARK)) {
      clrOld = gxSetTextColor (hdc, tbcd->clrTextHighlight);
      clrOldBk = gxSetBkColor (hdc, tbcd->clrMark);
      oldBkMode = gxSetBkMode (hdc, tbcd->nHLStringBkMode);
    }
    else {
      clrOld = gxSetTextColor (hdc, tbcd->clrText);
    }

    gxDrawTextW (hdc, lpText, -1, rcText, infoPtr->dwDTFlags);
    gxSetTextColor (hdc, clrOld);
    if ((state & CDIS_MARKED) && !(dwItemCDFlag & TBCDRF_NOMARK))
    {
      gxSetBkColor (hdc, clrOldBk);
      gxSetBkMode (hdc, oldBkMode);
    }
    gxSelectObject (hdc, hOldFont);
  }
}


static void
TOOLBAR_DrawPattern (const GXRECT *lpRect, const GXNMTBCUSTOMDRAW *tbcd)
{
  GXHDC hdc = tbcd->nmcd.hdc;
  GXHBRUSH hbr = (GXHBRUSH)gxSelectObject (hdc, tbcd->hbrMonoDither);
  GXCOLORREF clrTextOld;
  GXCOLORREF clrBkOld;
  GXINT cx = lpRect->right - lpRect->left;
  GXINT cy = lpRect->bottom - lpRect->top;
  GXINT cxEdge = gxGetSystemMetrics(GXSM_CXEDGE);
  GXINT cyEdge = gxGetSystemMetrics(GXSM_CYEDGE);
  clrTextOld = gxSetTextColor(hdc, tbcd->clrBtnHighlight);
  clrBkOld = gxSetBkColor(hdc, tbcd->clrBtnFace);
  gxPatBlt (hdc, lpRect->left + cxEdge, lpRect->top + cyEdge,
    cx - (2 * cxEdge), cy - (2 * cyEdge), GXPATCOPY);
  gxSetBkColor(hdc, clrBkOld);
  gxSetTextColor(hdc, clrTextOld);
  gxSelectObject (hdc, hbr);
}


static void TOOLBAR_DrawMasked(GXHIMAGELIST himl, GXINT index, GXHDC hdc, GXINT x, GXINT y, GXUINT draw_flags)
{
  GXINT cx, cy;
  GXHBITMAP hbmMask, hbmImage;
  GXHDC hdcMask, hdcImage;

  gxImageList_GetIconSize(himl, &cx, &cy);

  /* Create src image */
  hdcImage = gxCreateCompatibleDC(hdc);
  hbmImage = gxCreateCompatibleBitmap(hdc, cx, cy);
  gxSelectObject(hdcImage, hbmImage);
  gxImageList_DrawEx(himl, index, hdcImage, 0, 0, cx, cy,
    GXRGB(0xff, 0xff, 0xff), GXRGB(0,0,0), draw_flags);

  /* Create Mask */
  hdcMask = gxCreateCompatibleDC(0);
  hbmMask = gxCreateBitmap(cx, cy, 1, 1, NULL);
  gxSelectObject(hdcMask, hbmMask);

  /* Remove the background and all white pixels */
  gxImageList_DrawEx(himl, index, hdcMask, 0, 0, cx, cy,
    GXRGB(0xff, 0xff, 0xff), GXRGB(0,0,0), ILD_MASK);
  gxSetBkColor(hdcImage, GXRGB(0xff, 0xff, 0xff));
  gxBitBlt(hdcMask, 0, 0, cx, cy, hdcImage, 0, 0, GXNOTSRCERASE);

  /* draw the new mask 'etched' to hdc */
  gxSetBkColor(hdc, GXRGB(255, 255, 255));
  gxSelectObject(hdc, gxGetSysColorBrush(GXCOLOR_3DHILIGHT));
  /* E20746 op code is (Dst ^ (Src & (Pat ^ Dst))) */
  gxBitBlt(hdc, x + 1, y + 1, cx, cy, hdcMask, 0, 0, 0xE20746);
  gxSelectObject(hdc, gxGetSysColorBrush(GXCOLOR_3DSHADOW));
  gxBitBlt(hdc, x, y, cx, cy, hdcMask, 0, 0, 0xE20746);

  /* Cleanup */
  gxDeleteObject(hbmImage);
  gxDeleteDC(hdcImage);
  gxDeleteObject (hbmMask);
  gxDeleteDC(hdcMask);
}


static GXUINT
TOOLBAR_TranslateState(const TBUTTON_INFO *btnPtr)
{
  GXUINT retstate = 0;

  retstate |= (btnPtr->fsState & TBSTATE_CHECKED) ? CDIS_CHECKED  : 0;
  retstate |= (btnPtr->fsState & TBSTATE_PRESSED) ? CDIS_SELECTED : 0;
  retstate |= (btnPtr->fsState & TBSTATE_ENABLED) ? 0 : CDIS_DISABLED;
  retstate |= (btnPtr->fsState & TBSTATE_MARKED ) ? CDIS_MARKED   : 0;
  retstate |= (btnPtr->bHot                     ) ? CDIS_HOT      : 0;
  retstate |= ((btnPtr->fsState & (TBSTATE_ENABLED|TBSTATE_INDETERMINATE)) == (TBSTATE_ENABLED|TBSTATE_INDETERMINATE)) ? CDIS_INDETERMINATE : 0;
  /* NOTE: we don't set CDIS_GRAYED, CDIS_FOCUS, CDIS_DEFAULT */
  return retstate;
}

/* draws the image on a toolbar button */
static void
TOOLBAR_DrawImage(const TOOLBAR_INFO *infoPtr, TBUTTON_INFO *btnPtr, GXINT left, GXINT top,
          const GXNMTBCUSTOMDRAW *tbcd, GXDWORD dwItemCDFlag)
{
  GXHIMAGELIST himl = NULL;
  GXBOOL draw_masked = FALSE;
  GXINT index;
  GXINT offset = 0;
  GXUINT draw_flags = ILD_TRANSPARENT;

  if (tbcd->nmcd.uItemState & (CDIS_DISABLED | CDIS_INDETERMINATE))
  {
    himl = TOOLBAR_GetImageListForDrawing(infoPtr, btnPtr, IMAGE_LIST_DISABLED, &index);
    if (!himl)
    {
      himl = TOOLBAR_GetImageListForDrawing(infoPtr, btnPtr, IMAGE_LIST_DEFAULT, &index);
      draw_masked = TRUE;
    }
  }
  else if (tbcd->nmcd.uItemState & CDIS_CHECKED ||
    ((tbcd->nmcd.uItemState & CDIS_HOT) 
    && ((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf))))
  {
    /* if hot, attempt to draw with hot image list, if fails, 
    use default image list */
    himl = TOOLBAR_GetImageListForDrawing(infoPtr, btnPtr, IMAGE_LIST_HOT, &index);
    if (!himl)
      himl = TOOLBAR_GetImageListForDrawing(infoPtr, btnPtr, IMAGE_LIST_DEFAULT, &index);
  }
  else
    himl = TOOLBAR_GetImageListForDrawing(infoPtr, btnPtr, IMAGE_LIST_DEFAULT, &index);

  if (!himl)
    return;

  if (!(dwItemCDFlag & TBCDRF_NOOFFSET) && 
    (tbcd->nmcd.uItemState & (CDIS_SELECTED | CDIS_CHECKED)))
    offset = 1;

  if (!(dwItemCDFlag & TBCDRF_NOMARK) &&
    (tbcd->nmcd.uItemState & CDIS_MARKED))
    draw_flags |= ILD_BLEND50;

  TRACE("drawing index=%d, himl=%p, left=%d, top=%d, offset=%d\n",
    index, himl, left, top, offset);

  if (draw_masked)
    TOOLBAR_DrawMasked (himl, index, tbcd->nmcd.hdc, left + offset, top + offset, draw_flags);
  else
    gxImageList_Draw (himl, index, tbcd->nmcd.hdc, left + offset, top + offset, draw_flags);
}

/* draws a blank frame for a toolbar button */
static void
TOOLBAR_DrawFrame(const TOOLBAR_INFO *infoPtr, const GXNMTBCUSTOMDRAW *tbcd, GXDWORD dwItemCDFlag)
{
  GXHDC hdc = tbcd->nmcd.hdc;
  GXRECT rc = tbcd->nmcd.rc;
  /* if the state is disabled or indeterminate then the button
  * cannot have an interactive look like pressed or hot */
  GXBOOL non_interactive_state = (tbcd->nmcd.uItemState & CDIS_DISABLED) ||
    (tbcd->nmcd.uItemState & CDIS_INDETERMINATE);
  GXBOOL pressed_look = !non_interactive_state &&
    ((tbcd->nmcd.uItemState & CDIS_SELECTED) || 
    (tbcd->nmcd.uItemState & CDIS_CHECKED));

  /* app don't want us to draw any edges */
  if (dwItemCDFlag & TBCDRF_NOEDGES)
    return;

  if (infoPtr->dwStyle & TBSTYLE_FLAT)
  {
    if (pressed_look)
      gxDrawEdge (hdc, &rc, GXBDR_SUNKENOUTER, GXBF_RECT);
    else if ((tbcd->nmcd.uItemState & CDIS_HOT) && !non_interactive_state)
      gxDrawEdge (hdc, &rc, GXBDR_RAISEDINNER, GXBF_RECT);
  }
  else
  {
    if (pressed_look)
      gxDrawEdge (hdc, &rc, GXEDGE_SUNKEN, GXBF_RECT | GXBF_MIDDLE);
    else
      gxDrawEdge (hdc, &rc, GXEDGE_RAISED,
      GXBF_SOFT | GXBF_RECT | GXBF_MIDDLE);
  }
}

static void
TOOLBAR_DrawSepDDArrow(const TOOLBAR_INFO *infoPtr, const GXNMTBCUSTOMDRAW *tbcd, GXRECT *rcArrow, GXBOOL bDropDownPressed, GXDWORD dwItemCDFlag)
{
  GXHDC hdc = tbcd->nmcd.hdc;
  GXINT offset = 0;
  GXBOOL pressed = bDropDownPressed ||
    (tbcd->nmcd.uItemState & (CDIS_SELECTED | CDIS_CHECKED));

  if (infoPtr->dwStyle & TBSTYLE_FLAT)
  {
    if (pressed)
      gxDrawEdge (hdc, rcArrow, GXBDR_SUNKENOUTER, GXBF_RECT);
    else if ( (tbcd->nmcd.uItemState & CDIS_HOT) &&
      !(tbcd->nmcd.uItemState & CDIS_DISABLED) &&
      !(tbcd->nmcd.uItemState & CDIS_INDETERMINATE))
      gxDrawEdge (hdc, rcArrow, GXBDR_RAISEDINNER, GXBF_RECT);
  }
  else
  {
    if (pressed)
      gxDrawEdge (hdc, rcArrow, GXEDGE_SUNKEN, GXBF_RECT | GXBF_MIDDLE);
    else
      gxDrawEdge (hdc, rcArrow, GXEDGE_RAISED,
      GXBF_SOFT | GXBF_RECT | GXBF_MIDDLE);
  }

  if (pressed)
    offset = (dwItemCDFlag & TBCDRF_NOOFFSET) ? 0 : 1;

  if (tbcd->nmcd.uItemState & (CDIS_DISABLED | CDIS_INDETERMINATE))
  {
    TOOLBAR_DrawArrow(hdc, rcArrow->left+1, rcArrow->top+1 + (rcArrow->bottom - rcArrow->top - ARROW_HEIGHT) / 2, comctl32_color.clrBtnHighlight);
    TOOLBAR_DrawArrow(hdc, rcArrow->left, rcArrow->top + (rcArrow->bottom - rcArrow->top - ARROW_HEIGHT) / 2, comctl32_color.clr3dShadow);
  }
  else
    TOOLBAR_DrawArrow(hdc, rcArrow->left + offset, rcArrow->top + offset + (rcArrow->bottom - rcArrow->top - ARROW_HEIGHT) / 2, comctl32_color.clrBtnText);
}

/* draws a complete toolbar button */
static void
TOOLBAR_DrawButton (GXHWND hwnd, TBUTTON_INFO *btnPtr, GXHDC hdc, GXDWORD dwBaseCustDraw)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD dwStyle = infoPtr->dwStyle;
  GXBOOL hasDropDownArrow = (TOOLBAR_HasDropDownArrows(infoPtr->dwExStyle) &&
    (btnPtr->fsStyle & BTNS_DROPDOWN)) ||
    (btnPtr->fsStyle & BTNS_WHOLEDROPDOWN);
  GXBOOL drawSepDropDownArrow = hasDropDownArrow && 
    (~btnPtr->fsStyle & BTNS_WHOLEDROPDOWN);
  GXRECT rc, rcArrow, rcBitmap, rcText;
  GXLPWSTR lpText = NULL;
  GXNMTBCUSTOMDRAW tbcd;
  GXDWORD ntfret;
  GXINT offset;
  GXINT oldBkMode;
  GXDWORD dwItemCustDraw;
  GXDWORD dwItemCDFlag;
  GXHTHEME theme = gxGetWindowTheme (hwnd);

  rc = btnPtr->rect;
  gxCopyRect (&rcArrow, &rc);

  /* separator - doesn't send NM_CUSTOMDRAW */
  if (btnPtr->fsStyle & BTNS_SEP) {
    if (theme)
    {
      gxDrawThemeBackground (theme, hdc, 
        (dwStyle & CCS_VERT) ? GXTP_SEPARATORVERT : GXTP_SEPARATOR, 0, 
        &rc, NULL);
    }
    else
      /* with the FLAT style, iBitmap is the width and has already */
      /* been taken into consideration in calculating the width    */
      /* so now we need to draw the vertical separator             */
      /* empirical tests show that iBitmap can/will be non-zero    */
      /* when drawing the vertical bar...      */
      if ((dwStyle & TBSTYLE_FLAT) /* && (btnPtr->iBitmap == 0) */) {
        if (btnPtr->fsStyle & BTNS_DROPDOWN)
          TOOLBAR_DrawDDFlatSeparator (&rc, hdc, infoPtr);
        else
          TOOLBAR_DrawFlatSeparator (&rc, hdc, infoPtr);
      }
      else if (btnPtr->fsStyle != BTNS_SEP) {
        FIXME("Draw some kind of separator: fsStyle=%x\n",
          btnPtr->fsStyle);
      }
      return;
  }

  /* get a pointer to the text */
  lpText = TOOLBAR_GetText(infoPtr, btnPtr);

  if (hasDropDownArrow)
  {
    GXINT right;

    if (dwStyle & TBSTYLE_FLAT)
      right = max(rc.left, rc.right - DDARROW_WIDTH);
    else
      right = max(rc.left, rc.right - DDARROW_WIDTH - 2);

    if (drawSepDropDownArrow)
      rc.right = right;

    rcArrow.left = right;
  }

  /* copy text & bitmap rects after adjusting for drop-down arrow
  * so that text & bitmap is centered in the rectangle not containing
  * the arrow */
  gxCopyRect(&rcText, &rc);
  gxCopyRect(&rcBitmap, &rc);

  /* Center the bitmap horizontally and vertically */
  if (dwStyle & TBSTYLE_LIST)
  {
    if (lpText &&
      infoPtr->nMaxTextRows > 0 &&
      (!(infoPtr->dwExStyle & TBSTYLE_EX_MIXEDBUTTONS) ||
      (btnPtr->fsStyle & BTNS_SHOWTEXT)) )
      rcBitmap.left += gxGetSystemMetrics(GXSM_CXEDGE) + infoPtr->szPadding.cx / 2;
    else
      rcBitmap.left += gxGetSystemMetrics(GXSM_CXEDGE) + infoPtr->iListGap / 2;
  }
  else
    rcBitmap.left += ((rc.right - rc.left) - infoPtr->nBitmapWidth) / 2;

  rcBitmap.top += infoPtr->szPadding.cy / 2;

  TRACE("iBitmap=%d, start=(%d,%d) w=%d, h=%d\n",
    btnPtr->iBitmap, rcBitmap.left, rcBitmap.top,
    infoPtr->nBitmapWidth, infoPtr->nBitmapHeight);
  TRACE("Text=%s\n", debugstr_w(lpText));
  TRACE("iListGap=%d, padding = { %d, %d }\n", infoPtr->iListGap, infoPtr->szPadding.cx, infoPtr->szPadding.cy);

  /* calculate text position */
  if (lpText)
  {
    rcText.left += gxGetSystemMetrics(GXSM_CXEDGE);
    rcText.right -= gxGetSystemMetrics(GXSM_CXEDGE);
    if (dwStyle & TBSTYLE_LIST)
    {
      rcText.left += infoPtr->nBitmapWidth + infoPtr->iListGap + 2;
    }
    else
    {
      if (gxImageList_GetImageCount(GETDEFIMAGELIST(infoPtr, 0)) > 0)
        rcText.top += infoPtr->szPadding.cy/2 + infoPtr->nBitmapHeight + 1;
      else
        rcText.top += infoPtr->szPadding.cy/2 + 2;
    }
  }

  /* Initialize fields in all cases, because we use these later
  * NOTE: applications can and do alter these to customize their
  * toolbars */
  ZeroMemory (&tbcd, sizeof(GXNMTBCUSTOMDRAW));
  tbcd.clrText = comctl32_color.clrBtnText;
  tbcd.clrTextHighlight = comctl32_color.clrHighlightText;
  tbcd.clrBtnFace = comctl32_color.clrBtnFace;
  tbcd.clrBtnHighlight = comctl32_color.clrBtnHighlight;
  tbcd.clrMark = comctl32_color.clrHighlight;
  tbcd.clrHighlightHotTrack = 0;
  tbcd.nStringBkMode = GXTRANSPARENT;
  tbcd.nHLStringBkMode = GXOPAQUE;
  /* MSDN says that this is the text rectangle.
  * But (why always a but) tracing of v5.7 of native shows
  * that this is really a *relative* rectangle based on the
  * the nmcd.rc. Also the left and top are always 0 ignoring
  * any bitmap that might be present. */
  tbcd.rcText.left = 0;
  tbcd.rcText.top = 0;
  tbcd.rcText.right = rcText.right - rc.left;
  tbcd.rcText.bottom = rcText.bottom - rc.top;
  tbcd.nmcd.uItemState = TOOLBAR_TranslateState(btnPtr);
  tbcd.nmcd.hdc = hdc;
  tbcd.nmcd.rc = rc;
  tbcd.hbrMonoDither = gxCOMCTL32_hPattern55AABrush;

  /* FIXME: what are these used for? */
  tbcd.hbrLines = 0;
  tbcd.hpenLines = 0;

  /* Issue Item Prepaint notify */
  dwItemCustDraw = 0;
  dwItemCDFlag = 0;
  if (dwBaseCustDraw & CDRF_NOTIFYITEMDRAW)
  {
    tbcd.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
    tbcd.nmcd.dwItemSpec = btnPtr->idCommand;
    tbcd.nmcd.lItemlParam = btnPtr->dwData;
    ntfret = TOOLBAR_SendNotify(&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
    /* reset these fields so the user can't alter the behaviour like native */
    tbcd.nmcd.hdc = hdc;
    tbcd.nmcd.rc = rc;

    dwItemCustDraw = ntfret & 0xffff;
    dwItemCDFlag = ntfret & 0xffff0000;
    if (dwItemCustDraw & CDRF_SKIPDEFAULT)
      return;
    /* save the only part of the rect that the user can change */
    rcText.right = tbcd.rcText.right + rc.left;
    rcText.bottom = tbcd.rcText.bottom + rc.top;
  }

  if (!(dwItemCDFlag & TBCDRF_NOOFFSET) &&
    (btnPtr->fsState & (TBSTATE_PRESSED | TBSTATE_CHECKED)))
    gxOffsetRect(&rcText, 1, 1);

  if (!(tbcd.nmcd.uItemState & CDIS_HOT) && 
    ((tbcd.nmcd.uItemState & CDIS_CHECKED) || (tbcd.nmcd.uItemState & CDIS_INDETERMINATE)))
    TOOLBAR_DrawPattern (&rc, &tbcd);

  if (((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf)) 
    && (tbcd.nmcd.uItemState & CDIS_HOT))
  {
    if ( dwItemCDFlag & TBCDRF_HILITEHOTTRACK )
    {
      GXCOLORREF oldclr;

      oldclr = gxSetBkColor(hdc, tbcd.clrHighlightHotTrack);
      gxExtTextOutW(hdc, 0, 0, GXETO_OPAQUE, &rc, NULL, 0, 0);
      if (hasDropDownArrow)
        gxExtTextOutW(hdc, 0, 0, GXETO_OPAQUE, &rcArrow, NULL, 0, 0);
      gxSetBkColor(hdc, oldclr);
    }
  }

  if (theme)
  {
    GXINT partId = drawSepDropDownArrow ? GXTP_SPLITBUTTON : GXTP_BUTTON;
    GXINT stateId = GXTS_NORMAL;

    if (tbcd.nmcd.uItemState & CDIS_DISABLED)
      stateId = GXTS_DISABLED;
    else if (tbcd.nmcd.uItemState & CDIS_SELECTED)
      stateId = GXTS_PRESSED;
    else if (tbcd.nmcd.uItemState & CDIS_CHECKED)
      stateId = (tbcd.nmcd.uItemState & CDIS_HOT) ? GXTS_HOTCHECKED : GXTS_HOT;
    else if ((tbcd.nmcd.uItemState & CDIS_HOT)
      || (drawSepDropDownArrow && btnPtr->bDropDownPressed))
      stateId = GXTS_HOT;

    gxDrawThemeBackground (theme, hdc, partId, stateId, &tbcd.nmcd.rc, NULL);
  }
  else
    TOOLBAR_DrawFrame(infoPtr, &tbcd, dwItemCDFlag);

  if (drawSepDropDownArrow)
  {
    if (theme)
    {
      GXINT stateId = GXTS_NORMAL;

      if (tbcd.nmcd.uItemState & CDIS_DISABLED)
        stateId = GXTS_DISABLED;
      else if (btnPtr->bDropDownPressed || (tbcd.nmcd.uItemState & CDIS_SELECTED))
        stateId = GXTS_PRESSED;
      else if (tbcd.nmcd.uItemState & CDIS_CHECKED)
        stateId = (tbcd.nmcd.uItemState & CDIS_HOT) ? GXTS_HOTCHECKED : GXTS_HOT;
      else if (tbcd.nmcd.uItemState & CDIS_HOT)
        stateId = GXTS_HOT;

      gxDrawThemeBackground (theme, hdc, GXTP_DROPDOWNBUTTON, stateId, &rcArrow, NULL);
      gxDrawThemeBackground (theme, hdc, GXTP_SPLITBUTTONDROPDOWN, stateId, &rcArrow, NULL);
    }
    else
      TOOLBAR_DrawSepDDArrow(infoPtr, &tbcd, &rcArrow, btnPtr->bDropDownPressed, dwItemCDFlag);
  }

  oldBkMode = gxSetBkMode (hdc, tbcd.nStringBkMode);
  if (!(infoPtr->dwExStyle & TBSTYLE_EX_MIXEDBUTTONS) || (btnPtr->fsStyle & BTNS_SHOWTEXT))
    TOOLBAR_DrawString (infoPtr, &rcText, lpText, &tbcd, dwItemCDFlag);
  gxSetBkMode (hdc, oldBkMode);

  TOOLBAR_DrawImage(infoPtr, btnPtr, rcBitmap.left, rcBitmap.top, &tbcd, dwItemCDFlag);

  if (hasDropDownArrow && !drawSepDropDownArrow)
  {
    if (tbcd.nmcd.uItemState & (CDIS_DISABLED | CDIS_INDETERMINATE))
    {
      TOOLBAR_DrawArrow(hdc, rcArrow.left+1, rcArrow.top+1 + (rcArrow.bottom - rcArrow.top - ARROW_HEIGHT) / 2, comctl32_color.clrBtnHighlight);
      TOOLBAR_DrawArrow(hdc, rcArrow.left, rcArrow.top + (rcArrow.bottom - rcArrow.top - ARROW_HEIGHT) / 2, comctl32_color.clr3dShadow);
    }
    else if (tbcd.nmcd.uItemState & (CDIS_SELECTED | CDIS_CHECKED))
    {
      offset = (dwItemCDFlag & TBCDRF_NOOFFSET) ? 0 : 1;
      TOOLBAR_DrawArrow(hdc, rcArrow.left + offset, rcArrow.top + offset + (rcArrow.bottom - rcArrow.top - ARROW_HEIGHT) / 2, comctl32_color.clrBtnText);
    }
    else
      TOOLBAR_DrawArrow(hdc, rcArrow.left, rcArrow.top + (rcArrow.bottom - rcArrow.top - ARROW_HEIGHT) / 2, comctl32_color.clrBtnText);
  }

  if (dwItemCustDraw & CDRF_NOTIFYPOSTPAINT)
  {
    tbcd.nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
    TOOLBAR_SendNotify(&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
  }

}


static void
TOOLBAR_Refresh (GXHWND hwnd, GXHDC hdc, const GXPAINTSTRUCT *ps)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT i;
  GXRECT rcTemp, rcClient;
  GXNMTBCUSTOMDRAW tbcd;
  GXDWORD ntfret;
  GXDWORD dwBaseCustDraw;

  /* the app has told us not to redraw the toolbar */
  if (!infoPtr->bDoRedraw)
    return;

  /* if imagelist belongs to the app, it can be changed
  by the app after setting it */
  if (GETDEFIMAGELIST(infoPtr, 0) != infoPtr->himlInt)
  {
    infoPtr->nNumBitmaps = 0;
    for (i = 0; i < infoPtr->cimlDef; i++)
      infoPtr->nNumBitmaps += gxImageList_GetImageCount(infoPtr->himlDef[i]->himl);
  }

  TOOLBAR_DumpToolbar (infoPtr, __LINE__);

  /* change the imagelist icon size if we manage the list and it is necessary */
  TOOLBAR_CheckImageListIconSize(infoPtr);

  /* Send initial notify */
  ZeroMemory (&tbcd, sizeof(GXNMTBCUSTOMDRAW));
  tbcd.nmcd.dwDrawStage = CDDS_PREPAINT;
  tbcd.nmcd.hdc = hdc;
  tbcd.nmcd.rc = ps->rcPaint;
  ntfret = TOOLBAR_SendNotify(&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
  dwBaseCustDraw = ntfret & 0xffff;

  gxGetClientRect(hwnd, &rcClient);

  /* redraw necessary buttons */
  btnPtr = infoPtr->buttons;
  for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++)
  {
    GXBOOL bDraw;
    if (!gxRectVisible(hdc, &btnPtr->rect))
      continue;
    if (infoPtr->dwExStyle & TBSTYLE_EX_HIDECLIPPEDBUTTONS)
    {
      gxIntersectRect(&rcTemp, &rcClient, &btnPtr->rect);
      bDraw = gxEqualRect(&rcTemp, &btnPtr->rect);
    }
    else
      bDraw = TRUE;
    bDraw &= gxIntersectRect(&rcTemp, &(ps->rcPaint), &(btnPtr->rect));
    bDraw = (btnPtr->fsState & TBSTATE_HIDDEN) ? FALSE : bDraw;
    if (bDraw)
      TOOLBAR_DrawButton(hwnd, btnPtr, hdc, dwBaseCustDraw);
  }

  /* draw insert mark if required */
  if (infoPtr->tbim.iButton != -1)
  {
    GXRECT rcButton = infoPtr->buttons[infoPtr->tbim.iButton].rect;
    GXRECT rcInsertMark;
    rcInsertMark.top = rcButton.top;
    rcInsertMark.bottom = rcButton.bottom;
    if (infoPtr->tbim.dwFlags & GXTBIMHT_AFTER)
      rcInsertMark.left = rcInsertMark.right = rcButton.right;
    else
      rcInsertMark.left = rcInsertMark.right = rcButton.left - INSERTMARK_WIDTH;
    gxCOMCTL32_DrawInsertMark(hdc, &rcInsertMark, infoPtr->clrInsertMark, FALSE);
  }

  if (dwBaseCustDraw & CDRF_NOTIFYPOSTPAINT)
  {
    ZeroMemory (&tbcd, sizeof(GXNMTBCUSTOMDRAW));
    tbcd.nmcd.dwDrawStage = CDDS_POSTPAINT;
    tbcd.nmcd.hdc = hdc;
    tbcd.nmcd.rc = ps->rcPaint;
    TOOLBAR_SendNotify(&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
  }
}

/***********************************************************************
*     TOOLBAR_MeasureString
*
* This function gets the width and height of a string in pixels. This
* is done first by using GetTextExtentPoint to get the basic width
* and height. The DrawText is called with GXDT_CALCRECT to get the exact
* width. The reason is because the text may have more than one "&" (or
* prefix characters as M$ likes to call them). The prefix character
* indicates where the underline goes, except for the string "&&" which
* is reduced to a single "&". GetTextExtentPoint does not process these
* only DrawText does. Note that the BTNS_NOPREFIX is handled here.
*/
static void
TOOLBAR_MeasureString(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *btnPtr,
            GXHDC hdc, GXLPSIZE lpSize)
{
  GXRECT myrect;

  lpSize->cx = 0;
  lpSize->cy = 0;

  if (infoPtr->nMaxTextRows > 0 &&
    !(btnPtr->fsState & TBSTATE_HIDDEN) &&
    (!(infoPtr->dwExStyle & TBSTYLE_EX_MIXEDBUTTONS) ||
    (btnPtr->fsStyle & BTNS_SHOWTEXT)) )
  {
    GXLPWSTR lpText = TOOLBAR_GetText(infoPtr, btnPtr);

    if(lpText != NULL) {
      /* first get size of all the text */
      gxGetTextExtentPoint32W (hdc, lpText, (int)GXSTRLEN(lpText), lpSize);

      /* feed above size into the rectangle for DrawText */
      myrect.left = myrect.top = 0;
      myrect.right = lpSize->cx;
      myrect.bottom = lpSize->cy;

      /* Use DrawText to get true size as drawn (less pesky "&") */
      gxDrawTextW (hdc, lpText, -1, &myrect, GXDT_VCENTER | GXDT_SINGLELINE |
        GXDT_CALCRECT | ((btnPtr->fsStyle & BTNS_NOPREFIX) ?
GXDT_NOPREFIX : 0));

      /* feed back to caller  */
      lpSize->cx = myrect.right;
      lpSize->cy = myrect.bottom;
    }
  }

  TRACE("string size %d x %d!\n", lpSize->cx, lpSize->cy);
}

/***********************************************************************
*     TOOLBAR_CalcStrings
*
* This function walks through each string and measures it and returns
* the largest height and width to caller.
*/
static void
TOOLBAR_CalcStrings (GXHWND hwnd, GXLPSIZE lpSize)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT i;
  GXSIZE sz;
  GXHDC hdc;
  GXHFONT hOldFont;

  lpSize->cx = 0;
  lpSize->cy = 0;

  if (infoPtr->nMaxTextRows == 0)
    return;

  hdc = gxGetDC (hwnd);
  hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hFont);

  if (infoPtr->nNumButtons == 0 && infoPtr->nNumStrings > 0)
  {
    GXTEXTMETRICW tm;

    gxGetTextMetricsW(hdc, &tm);
    lpSize->cy = tm.tmHeight;
  }

  btnPtr = infoPtr->buttons;
  for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++) {
    if(TOOLBAR_HasText(infoPtr, btnPtr))
    {
      TOOLBAR_MeasureString(infoPtr, btnPtr, hdc, &sz);
      if (sz.cx > lpSize->cx)
        lpSize->cx = sz.cx;
      if (sz.cy > lpSize->cy)
        lpSize->cy = sz.cy;
    }
  }

  gxSelectObject (hdc, hOldFont);
  gxReleaseDC (hwnd, hdc);

  TRACE("max string size %d x %d!\n", lpSize->cx, lpSize->cy);
}

/***********************************************************************
*     TOOLBAR_WrapToolbar
*
* This function walks through the buttons and separators in the
* toolbar, and sets the TBSTATE_WRAP flag only on those items where
* wrapping should occur based on the width of the toolbar window.
* It does *not* calculate button placement itself.  That task
* takes place in TOOLBAR_CalcToolbar. If the program wants to manage
* the toolbar wrapping on its own, it can use the TBSTYLE_WRAPABLE
* flag, and set the TBSTATE_WRAP flags manually on the appropriate items.
*
* Note: TBSTYLE_WRAPABLE or TBSTYLE_EX_UNDOC1 can be used also to allow
* vertical toolbar lists.
*/

static void
TOOLBAR_WrapToolbar( GXHWND hwnd, GXDWORD dwStyle )
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT x, cx, i, j;
  GXRECT rc;
  GXBOOL bButtonWrap;

  /*   When the toolbar window style is not TBSTYLE_WRAPABLE,  */
  /*  no layout is necessary. Applications may use this style */
  /*  to perform their own layout on the toolbar.     */
  if( !(dwStyle & TBSTYLE_WRAPABLE) &&
    !(infoPtr->dwExStyle & TBSTYLE_EX_UNDOC1) )  return;

  btnPtr = infoPtr->buttons;
  x  = infoPtr->nIndent;

  if (gxGetParent(hwnd))
  {
    /* this can get the parents width, to know how far we can extend
    * this toolbar.  We cannot use its height, as there may be multiple
    * toolbars in a rebar control
    */
    gxGetClientRect( gxGetParent(hwnd), &rc );
    infoPtr->nWidth = rc.right - rc.left;
  }
  else
  {
    gxGetWindowRect( hwnd, &rc );
    infoPtr->nWidth = rc.right - rc.left;
  }

  bButtonWrap = FALSE;

  TRACE("start ButtonWidth=%d, BitmapWidth=%d, nWidth=%d, nIndent=%d\n",
    infoPtr->nButtonWidth, infoPtr->nBitmapWidth, infoPtr->nWidth,
    infoPtr->nIndent);

  for (i = 0; i < infoPtr->nNumButtons; i++ )
  {
    btnPtr[i].fsState &= ~TBSTATE_WRAP;

    if (btnPtr[i].fsState & TBSTATE_HIDDEN)
      continue;

    /* UNDOCUMENTED: If a separator has a non zero bitmap index, */
    /* it is the actual width of the separator. This is used for */
    /* custom controls in toolbars.                              */
    /*                                                           */
    /* BTNS_DROPDOWN separators are treated as buttons for    */
    /* width.  - GA 8/01                                         */
    if ((btnPtr[i].fsStyle & BTNS_SEP) &&
      !(btnPtr[i].fsStyle & BTNS_DROPDOWN))
      cx = (btnPtr[i].iBitmap > 0) ?
      btnPtr[i].iBitmap : SEPARATOR_WIDTH;
    else
      cx = (btnPtr[i].cx) ? btnPtr[i].cx : infoPtr->nButtonWidth;

    /* Two or more adjacent separators form a separator group.   */
    /* The first separator in a group should be wrapped to the   */
    /* next row if the previous wrapping is on a button.       */
    if( bButtonWrap &&
      (btnPtr[i].fsStyle & BTNS_SEP) &&
      (i + 1 < infoPtr->nNumButtons ) &&
      (btnPtr[i + 1].fsStyle & BTNS_SEP) )
    {
      TRACE("wrap point 1 btn %d style %02x\n", i, btnPtr[i].fsStyle);
      btnPtr[i].fsState |= TBSTATE_WRAP;
      x = infoPtr->nIndent;
      i++;
      bButtonWrap = FALSE;
      continue;
    }

    /* The layout makes sure the bitmap is visible, but not the button. */
    /* Test added to also wrap after a button that starts a row but     */
    /* is bigger than the area.  - GA  8/01                             */
    if (( x + cx - (infoPtr->nButtonWidth - infoPtr->nBitmapWidth) / 2
      > infoPtr->nWidth ) ||
      ((x == infoPtr->nIndent) && (cx > infoPtr->nWidth)))
    {
      GXBOOL bFound = FALSE;

      /*   If the current button is a separator and not hidden,  */
      /*  go to the next until it reaches a non separator.      */
      /*  Wrap the last separator if it is before a button.     */
      while( ( ((btnPtr[i].fsStyle & BTNS_SEP) &&
        !(btnPtr[i].fsStyle & BTNS_DROPDOWN)) ||
        (btnPtr[i].fsState & TBSTATE_HIDDEN) ) &&
        i < infoPtr->nNumButtons )
      {
        i++;
        bFound = TRUE;
      }

      if( bFound && i < infoPtr->nNumButtons )
      {
        i--;
        TRACE("wrap point 2 btn %d style %02x, x=%d, cx=%d\n",
          i, btnPtr[i].fsStyle, x, cx);
        btnPtr[i].fsState |= TBSTATE_WRAP;
        x = infoPtr->nIndent;
        bButtonWrap = FALSE;
        continue;
      }
      else if ( i >= infoPtr->nNumButtons)
        break;

      /*   If the current button is not a separator, find the last  */
      /*  separator and wrap it.            */
      for ( j = i - 1; j >= 0  &&  !(btnPtr[j].fsState & TBSTATE_WRAP); j--)
      {
        if ((btnPtr[j].fsStyle & BTNS_SEP) &&
          !(btnPtr[j].fsState & TBSTATE_HIDDEN))
        {
          bFound = TRUE;
          i = j;
          TRACE("wrap point 3 btn %d style %02x, x=%d, cx=%d\n",
            i, btnPtr[i].fsStyle, x, cx);
          x = infoPtr->nIndent;
          btnPtr[j].fsState |= TBSTATE_WRAP;
          bButtonWrap = FALSE;
          break;
        }
      }

      /*   If no separator available for wrapping, wrap one of   */
      /*  non-hidden previous button.               */
      if (!bFound)
      {
        for ( j = i - 1;
          j >= 0 && !(btnPtr[j].fsState & TBSTATE_WRAP); j--)
        {
          if (btnPtr[j].fsState & TBSTATE_HIDDEN)
            continue;

          bFound = TRUE;
          i = j;
          TRACE("wrap point 4 btn %d style %02x, x=%d, cx=%d\n",
            i, btnPtr[i].fsStyle, x, cx);
          x = infoPtr->nIndent;
          btnPtr[j].fsState |= TBSTATE_WRAP;
          bButtonWrap = TRUE;
          break;
        }
      }

      /* If all above failed, wrap the current button. */
      if (!bFound)
      {
        TRACE("wrap point 5 btn %d style %02x, x=%d, cx=%d\n",
          i, btnPtr[i].fsStyle, x, cx);
        btnPtr[i].fsState |= TBSTATE_WRAP;
        x = infoPtr->nIndent;
        if (btnPtr[i].fsStyle & BTNS_SEP )
          bButtonWrap = FALSE;
        else
          bButtonWrap = TRUE;
      }
    }
    else {
      TRACE("wrap point 6 btn %d style %02x, x=%d, cx=%d\n",
        i, btnPtr[i].fsStyle, x, cx);
      x += cx;
    }
  }
}


/***********************************************************************
*     TOOLBAR_MeasureButton
*
* Calculates the width and height required for a button. Used in
* TOOLBAR_CalcToolbar to set the all-button width and height and also for
* the width of buttons that are autosized.
*
* Note that it would have been rather elegant to use one piece of code for
* both the laying out of the toolbar and for controlling where button parts
* are drawn, but the native control has inconsistencies between the two that
* prevent this from being effectively. These inconsistencies can be seen as
* artefacts where parts of the button appear outside of the bounding button
* rectangle.
*
* There are several cases for the calculation of the button dimensions and
* button part positioning:
*
* List
* ====
*
* With Bitmap:
*
* +--------------------------------------------------------+ ^
* |                    ^                     ^             | |
* |                    | pad.cy / 2          | centred     | |
* | pad.cx/2 + cxedge +--------------+     +------------+  | | DEFPAD_CY +
* |<----------------->| nBitmapWidth |     | Text       |  | | max(nBitmapHeight, szText.cy)
* |                   |<------------>|     |            |  | |
* |                   +--------------+     +------------+  | |
* |<-------------------------------------->|               | |
* |  cxedge + iListGap + nBitmapWidth + 2  |<----------->  | |
* |                                           szText.cx    | |
* +--------------------------------------------------------+ -
* <-------------------------------------------------------->
*  2*cxedge + nBitmapWidth + iListGap + szText.cx + pad.cx
*
* Without Bitmap (GXI_IMAGENONE):
*
* +-----------------------------------+ ^
* |                     ^             | |
* |                     | centred     | | LISTPAD_CY +
* |                   +------------+  | | szText.cy
* |                   | Text       |  | |
* |                   |            |  | |
* |                   +------------+  | |
* |<----------------->|               | |
* |      cxedge       |<----------->  | |
* |                      szText.cx    | |
* +-----------------------------------+ -
* <----------------------------------->
*          szText.cx + pad.cx
*
* Without text:
*
* +--------------------------------------+ ^
* |                       ^              | |
* |                       | padding.cy/2 | | DEFPAD_CY +
* |                     +------------+   | | nBitmapHeight
* |                     | Bitmap     |   | |
* |                     |            |   | |
* |                     +------------+   | |
* |<------------------->|                | |
* | cxedge + iListGap/2 |<----------->   | |
* |                       nBitmapWidth   | |
* +--------------------------------------+ -
* <-------------------------------------->
*     2*cxedge + nBitmapWidth + iListGap
*
* Non-List
* ========
*
* With bitmap:
*
* +-----------------------------------+ ^
* |                     ^             | |
* |                     | pad.cy / 2  | | nBitmapHeight +
* |                     -             | | szText.cy +
* |                   +------------+  | | DEFPAD_CY + 1
* |    centred        |   Bitmap   |  | |
* |<----------------->|            |  | |
* |                   +------------+  | |
* |                         ^         | |
* |                       1 |         | |
* |                         -         | |
* |     centred     +---------------+ | |
* |<--------------->|      Text     | | |
* |                 +---------------+ | |
* +-----------------------------------+ -
* <----------------------------------->
* pad.cx + max(nBitmapWidth, szText.cx)
*
* Without bitmaps (NULL imagelist or gxImageList_GetImageCount() = 0):
*
* +---------------------------------------+ ^
* |                     ^                 | |
* |                     | 2 + pad.cy / 2  | |
* |                     -                 | | szText.cy +
* |    centred      +-----------------+   | | pad.cy + 2
* |<--------------->|   Text          |   | |
* |                 +-----------------+   | |
* |                                       | |
* +---------------------------------------+ -
* <--------------------------------------->
*          2*cxedge + pad.cx + szText.cx
*
* Without text:
*   As for with bitmaps, but with szText.cx zero.
*/
static inline GXSIZE TOOLBAR_MeasureButton(const TOOLBAR_INFO *infoPtr, GXSIZE sizeString,
                       GXBOOL bHasBitmap, GXBOOL bValidImageList)
{
  GXSIZE sizeButton;
  if (infoPtr->dwStyle & TBSTYLE_LIST)
  {
    /* set button height from bitmap / text height... */
    sizeButton.cy = max((bHasBitmap ? infoPtr->nBitmapHeight : 0),
      sizeString.cy);

    /* ... add on the necessary padding */
    if (bValidImageList)
    {
      if (bHasBitmap)
        sizeButton.cy += DEFPAD_CY;
      else
        sizeButton.cy += LISTPAD_CY;
    }
    else
      sizeButton.cy += infoPtr->szPadding.cy;

    /* calculate button width */
    sizeButton.cx = 2*gxGetSystemMetrics(GXSM_CXEDGE) +
      infoPtr->nBitmapWidth + infoPtr->iListGap;
    if (sizeString.cx > 0)
      sizeButton.cx += sizeString.cx + infoPtr->szPadding.cx;

  }
  else
  {
    if (bHasBitmap)
    {
      sizeButton.cy = infoPtr->nBitmapHeight + DEFPAD_CY;
      if (sizeString.cy > 0)
        sizeButton.cy += 1 + sizeString.cy;
      sizeButton.cx = infoPtr->szPadding.cx +
        max(sizeString.cx, infoPtr->nBitmapWidth);
    }
    else
    {
      sizeButton.cy = sizeString.cy + infoPtr->szPadding.cy +
        NONLIST_NOTEXT_OFFSET;
      sizeButton.cx = infoPtr->szPadding.cx +
        max(2*gxGetSystemMetrics(GXSM_CXEDGE) + sizeString.cx, infoPtr->nBitmapWidth);
    }
  }
  return sizeButton;
}


/***********************************************************************
*     TOOLBAR_CalcToolbar
*
* This function calculates button and separator placement. It first
* calculates the button sizes, gets the toolbar window width and then
* calls TOOLBAR_WrapToolbar to determine which buttons we need to wrap
* on. It assigns a new location to each item and sends this location to
* the tooltip window if appropriate. Finally, it updates the rcBound
* rect and calculates the new required toolbar window height.
*/
static void
TOOLBAR_CalcToolbar (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXSIZE  sizeString, sizeButton;
  GXBOOL validImageList = FALSE;

  TOOLBAR_CalcStrings (hwnd, &sizeString);

  TOOLBAR_DumpToolbar (infoPtr, __LINE__);

  if (TOOLBAR_IsValidImageList(infoPtr, 0))
    validImageList = TRUE;
  sizeButton = TOOLBAR_MeasureButton(infoPtr, sizeString, TRUE, validImageList);
  infoPtr->nButtonWidth = sizeButton.cx;
  infoPtr->nButtonHeight = sizeButton.cy;
  infoPtr->iTopMargin = default_top_margin(infoPtr);

  if ( infoPtr->cxMin >= 0 && infoPtr->nButtonWidth < infoPtr->cxMin )
    infoPtr->nButtonWidth = infoPtr->cxMin;
  if ( infoPtr->cxMax > 0 && infoPtr->nButtonWidth > infoPtr->cxMax )
    infoPtr->nButtonWidth = infoPtr->cxMax;

  TOOLBAR_LayoutToolbar(hwnd);
}

static void
TOOLBAR_LayoutToolbar(GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  TBUTTON_INFO *btnPtr;
  GXSIZE sizeButton;
  GXINT i, nRows, nSepRows;
  GXINT x, y, cx, cy;
  GXBOOL bWrap;
  GXBOOL validImageList = TOOLBAR_IsValidImageList(infoPtr, 0);
  GXBOOL hasDropDownArrows = TOOLBAR_HasDropDownArrows(infoPtr->dwExStyle);

  TOOLBAR_WrapToolbar(hwnd, infoPtr->dwStyle);

  x  = infoPtr->nIndent;
  y  = infoPtr->iTopMargin;
  cx = infoPtr->nButtonWidth;
  cy = infoPtr->nButtonHeight;

  nRows = nSepRows = 0;

  infoPtr->rcBound.top = y;
  infoPtr->rcBound.left = x;
  infoPtr->rcBound.bottom = y + cy;
  infoPtr->rcBound.right = x;

  btnPtr = infoPtr->buttons;

  TRACE("cy=%d\n", cy);

  for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++ )
  {
    bWrap = FALSE;
    if (btnPtr->fsState & TBSTATE_HIDDEN)
    {
      gxSetRectEmpty (&btnPtr->rect);
      continue;
    }

    cy = infoPtr->nButtonHeight;

    /* UNDOCUMENTED: If a separator has a non zero bitmap index, */
    /* it is the actual width of the separator. This is used for */
    /* custom controls in toolbars.                              */
    if (btnPtr->fsStyle & BTNS_SEP) {
      if (btnPtr->fsStyle & BTNS_DROPDOWN) {
        cy = (btnPtr->iBitmap > 0) ?
          btnPtr->iBitmap : SEPARATOR_WIDTH;
        cx = infoPtr->nButtonWidth;
      }
      else
        cx = (btnPtr->iBitmap > 0) ?
        btnPtr->iBitmap : SEPARATOR_WIDTH;
    }
    else
    {
      if (btnPtr->cx)
        cx = btnPtr->cx;
      else if ((infoPtr->dwExStyle & TBSTYLE_EX_MIXEDBUTTONS) || 
        (btnPtr->fsStyle & BTNS_AUTOSIZE))
      {
        GXSIZE sz;
        GXHDC hdc;
        GXHFONT hOldFont;

        hdc = gxGetDC (hwnd);
        hOldFont = (GXHFONT)gxSelectObject (hdc, infoPtr->hFont);

        TOOLBAR_MeasureString(infoPtr, btnPtr, hdc, &sz);

        gxSelectObject (hdc, hOldFont);
        gxReleaseDC (hwnd, hdc);

        sizeButton = TOOLBAR_MeasureButton(infoPtr, sz,
          TOOLBAR_IsValidBitmapIndex(infoPtr, infoPtr->buttons[i].iBitmap),
          validImageList);
        cx = sizeButton.cx;
      }
      else
        cx = infoPtr->nButtonWidth;

      /* if size has been set manually then don't add on extra space
      * for the drop down arrow */
      if (!btnPtr->cx && hasDropDownArrows && 
        ((btnPtr->fsStyle & BTNS_DROPDOWN) || (btnPtr->fsStyle & BTNS_WHOLEDROPDOWN)))
        cx += DDARROW_WIDTH;
    }
    if (btnPtr->fsState & TBSTATE_WRAP )
      bWrap = TRUE;

    gxSetRect (&btnPtr->rect, x, y, x + cx, y + cy);

    if (infoPtr->rcBound.left > x)
      infoPtr->rcBound.left = x;
    if (infoPtr->rcBound.right < x + cx)
      infoPtr->rcBound.right = x + cx;
    if (infoPtr->rcBound.bottom < y + cy)
      infoPtr->rcBound.bottom = y + cy;

    TOOLBAR_TooltipSetRect(infoPtr, btnPtr);

    /* btnPtr->nRow is zero based. The space between the rows is   */
    /* also considered as a row.           */
    btnPtr->nRow = nRows + nSepRows;

    TRACE("button %d style=%x, bWrap=%d, nRows=%d, nSepRows=%d, btnrow=%d, (%d,%d)-(%d,%d)\n",
      i, btnPtr->fsStyle, bWrap, nRows, nSepRows, btnPtr->nRow,
      x, y, x+cx, y+cy);

    if( bWrap )
    {
      if ( !(btnPtr->fsStyle & BTNS_SEP) )
        y += cy;
      else
      {
        /* UNDOCUMENTED: If a separator has a non zero bitmap index, */
        /* it is the actual width of the separator. This is used for */
        /* custom controls in toolbars.            */
        if ( !(btnPtr->fsStyle & BTNS_DROPDOWN))
          y += cy + ( (btnPtr->iBitmap > 0 ) ?
          btnPtr->iBitmap : SEPARATOR_WIDTH) * 2 /3;
        else
          y += cy;

        /* nSepRows is used to calculate the extra height following  */
        /* the last row.               */
        nSepRows++;
      }
      x = infoPtr->nIndent;

      /* Increment row number unless this is the last button    */
      /* and it has Wrap set.                                   */
      if (i != infoPtr->nNumButtons-1)
        nRows++;
    }
    else
      x += cx;
  }

  /* infoPtr->nRows is the number of rows on the toolbar */
  infoPtr->nRows = nRows + nSepRows + 1;

  TRACE("toolbar button width %d\n", infoPtr->nButtonWidth);
}


static GXINT
TOOLBAR_InternalHitTest (GXHWND hwnd, const GXPOINT *lpPt)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT i;

  btnPtr = infoPtr->buttons;
  for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++) {
    if (btnPtr->fsState & TBSTATE_HIDDEN)
      continue;

    if (btnPtr->fsStyle & BTNS_SEP) {
      if (gxPtInRect (&btnPtr->rect, *lpPt)) {
        TRACE(" ON SEPARATOR %d!\n", i);
        return -i;
      }
    }
    else {
      if (gxPtInRect (&btnPtr->rect, *lpPt)) {
        TRACE(" ON BUTTON %d!\n", i);
        return i;
      }
    }
  }

  TRACE(" NOWHERE!\n");
  return TOOLBAR_NOWHERE;
}


/* worker for TB_ADDBUTTONS and TB_INSERTBUTTON */
static GXBOOL
TOOLBAR_InternalInsertButtonsT(TOOLBAR_INFO *infoPtr, GXINT iIndex, GXUINT nAddButtons, GXTBBUTTON *lpTbb, GXBOOL fUnicode)
{
  GXINT nOldButtons, nNewButtons, iButton;
  GXBOOL fHasString = FALSE;

  if (iIndex < 0)  /* iIndex can be negative, what means adding at the end */
    iIndex = infoPtr->nNumButtons;

  nOldButtons = infoPtr->nNumButtons;
  nNewButtons = nOldButtons + nAddButtons;

  infoPtr->buttons = (TBUTTON_INFO*)ReAlloc(infoPtr->buttons, sizeof(TBUTTON_INFO)*nNewButtons);
  memmove(&infoPtr->buttons[iIndex + nAddButtons], &infoPtr->buttons[iIndex],
    (nOldButtons - iIndex) * sizeof(TBUTTON_INFO));
  infoPtr->nNumButtons += nAddButtons;

  /* insert new buttons data */
  for (iButton = 0; iButton < (GXINT)nAddButtons; iButton++) {
    TBUTTON_INFO *btnPtr = &infoPtr->buttons[iIndex + iButton];

    TOOLBAR_DumpTBButton(lpTbb, fUnicode);

    ZeroMemory(btnPtr, sizeof(*btnPtr));
    btnPtr->iBitmap   = lpTbb[iButton].iBitmap;
    btnPtr->idCommand = (GXINT)lpTbb[iButton].idCommand;
    btnPtr->fsState   = lpTbb[iButton].fsState;
    btnPtr->fsStyle   = lpTbb[iButton].fsStyle;
    btnPtr->dwData    = lpTbb[iButton].dwData;
    if(GXHIWORD(lpTbb[iButton].iString) && lpTbb[iButton].iString != -1)
    {
      if (fUnicode)
        gxStr_SetPtrW((GXLPWSTR*)&btnPtr->iString, (GXLPWSTR)lpTbb[iButton].iString );
      else
        gxStr_SetPtrAtoW((GXLPWSTR*)&btnPtr->iString, (GXLPSTR)lpTbb[iButton].iString);
      fHasString = TRUE;
    }
    else
      btnPtr->iString   = lpTbb[iButton].iString;

    TOOLBAR_TooltipAddTool(infoPtr, btnPtr);
  }

  if (infoPtr->nNumStrings > 0 || fHasString)
    TOOLBAR_CalcToolbar(infoPtr->hwndSelf);
  else
    TOOLBAR_LayoutToolbar(infoPtr->hwndSelf);
  TOOLBAR_AutoSize(infoPtr->hwndSelf);

  TOOLBAR_DumpToolbar(infoPtr, __LINE__);
  gxInvalidateRect(infoPtr->hwndSelf, NULL, TRUE);
  return TRUE;
}


static GXINT
TOOLBAR_GetButtonIndex (const TOOLBAR_INFO *infoPtr, GXINT idCommand, GXBOOL CommandIsIndex)
{
  TBUTTON_INFO *btnPtr;
  GXINT i;

  if (CommandIsIndex) {
    TRACE("command is really index command=%d\n", idCommand);
    if (idCommand >= infoPtr->nNumButtons) return -1;
    return idCommand;
  }
  btnPtr = infoPtr->buttons;
  for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++) {
    if (btnPtr->idCommand == idCommand) {
      TRACE("command=%d index=%d\n", idCommand, i);
      return i;
    }
  }
  TRACE("no index found for command=%d\n", idCommand);
  return -1;
}


static GXINT
TOOLBAR_GetCheckedGroupButtonIndex (const TOOLBAR_INFO *infoPtr, GXINT nIndex)
{
  TBUTTON_INFO *btnPtr;
  GXINT nRunIndex;

  if ((nIndex < 0) || (nIndex > infoPtr->nNumButtons))
    return -1;

  /* check index button */
  btnPtr = &infoPtr->buttons[nIndex];
  if ((btnPtr->fsStyle & BTNS_CHECKGROUP) == BTNS_CHECKGROUP) {
    if (btnPtr->fsState & TBSTATE_CHECKED)
      return nIndex;
  }

  /* check previous buttons */
  nRunIndex = nIndex - 1;
  while (nRunIndex >= 0) {
    btnPtr = &infoPtr->buttons[nRunIndex];
    if ((btnPtr->fsStyle & BTNS_GROUP) == BTNS_GROUP) {
      if (btnPtr->fsState & TBSTATE_CHECKED)
        return nRunIndex;
    }
    else
      break;
    nRunIndex--;
  }

  /* check next buttons */
  nRunIndex = nIndex + 1;
  while (nRunIndex < infoPtr->nNumButtons) {
    btnPtr = &infoPtr->buttons[nRunIndex];
    if ((btnPtr->fsStyle & BTNS_GROUP) == BTNS_GROUP) {
      if (btnPtr->fsState & TBSTATE_CHECKED)
        return nRunIndex;
    }
    else
      break;
    nRunIndex++;
  }

  return -1;
}


static GXVOID
TOOLBAR_RelayEvent (GXHWND hwndTip, GXHWND hwndMsg, GXUINT uMsg,
          GXWPARAM wParam, GXLPARAM lParam)
{
  GXMSG msg;

  msg.hwnd = hwndMsg;
  msg.message = uMsg;
  msg.wParam = wParam;
  msg.lParam = lParam;
  msg.time = gxGetMessageTime ();
  msg.pt.x = (short)GXLOWORD(gxGetMessagePos ());
  msg.pt.y = (short)GXHIWORD(gxGetMessagePos ());

  gxSendMessageW (hwndTip, GXTTM_RELAYEVENT, 0, (GXLPARAM)&msg);
}

static void
TOOLBAR_TooltipAddTool(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *button)
{
  if (infoPtr->hwndToolTip && !(button->fsStyle & BTNS_SEP)) {
    GXTTTOOLINFOW ti;

    ZeroMemory(&ti, sizeof(GXTTTOOLINFOW));
    ti.cbSize   = sizeof (GXTTTOOLINFOW);
    ti.hwnd     = infoPtr->hwndSelf;
    ti.uId      = button->idCommand;
    ti.hinst    = 0;
    ti.lpszText = GXLPSTR_TEXTCALLBACKW;
    /* ti.lParam = random value from the stack? */

    gxSendMessageW(infoPtr->hwndToolTip, GXTTM_ADDTOOLW,
      0, (GXLPARAM)&ti);
  }
}

static void
TOOLBAR_TooltipDelTool(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *button)
{
  if ((infoPtr->hwndToolTip) && !(button->fsStyle & BTNS_SEP)) {
    GXTTTOOLINFOW ti;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize   = sizeof(ti);
    ti.hwnd     = infoPtr->hwndSelf;
    ti.uId      = button->idCommand;

    gxSendMessageW(infoPtr->hwndToolTip, GXTTM_DELTOOLW, 0, (GXLPARAM)&ti);
  }
}

static void TOOLBAR_TooltipSetRect(const TOOLBAR_INFO *infoPtr, const TBUTTON_INFO *button)
{
  /* Set the toolTip only for non-hidden, non-separator button */
  if (infoPtr->hwndToolTip && !(button->fsStyle & BTNS_SEP))
  {
    GXTTTOOLINFOW ti;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize = sizeof(ti);
    ti.hwnd = infoPtr->hwndSelf;
    ti.uId = button->idCommand;
    ti.rect = button->rect;
    gxSendMessageW(infoPtr->hwndToolTip, GXTTM_NEWTOOLRECTW, 0, (GXLPARAM)&ti);
  }
}

/* Creates the tooltip control */
static void
TOOLBAR_TooltipCreateControl(TOOLBAR_INFO *infoPtr)
{
  GXINT i;
  GXNMTOOLTIPSCREATED nmttc;

  infoPtr->hwndToolTip = gxCreateWindowExW(0, TOOLTIPS_CLASSW, NULL, GXWS_POPUP,
    GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT, GXCW_USEDEFAULT,
    infoPtr->hwndSelf, 0, 0, 0);

  if (!infoPtr->hwndToolTip)
    return;

  /* Send NM_TOOLTIPSCREATED notification */
  nmttc.hwndToolTips = infoPtr->hwndToolTip;
  TOOLBAR_SendNotify(&nmttc.hdr, infoPtr, GXNM_TOOLTIPSCREATED);

  for (i = 0; i < infoPtr->nNumButtons; i++)
  {
    TOOLBAR_TooltipAddTool(infoPtr, &infoPtr->buttons[i]);
    TOOLBAR_TooltipSetRect(infoPtr, &infoPtr->buttons[i]);
  }
}

/* keeps available button list box sorted by button id */
static void TOOLBAR_Cust_InsertAvailButton(GXHWND hwnd, PCUSTOMBUTTON btnInfoNew)
{
  GXINT i;
  GXINT count;
  PCUSTOMBUTTON btnInfo;
  GXHWND hwndAvail = gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX);

  TRACE("button %s, idCommand %d\n", debugstr_w(btnInfoNew->text), btnInfoNew->btn.idCommand);

  count = gxSendMessageW(hwndAvail, GXLB_GETCOUNT, 0, 0);

  /* position 0 is always separator */
  for (i = 1; i < count; i++)
  {
    btnInfo = (PCUSTOMBUTTON)gxSendMessageW(hwndAvail, GXLB_GETITEMDATA, i, 0);
    if (btnInfoNew->btn.idCommand < btnInfo->btn.idCommand)
    {
      i = gxSendMessageW(hwndAvail, GXLB_INSERTSTRINGW, i, 0);
      gxSendMessageW(hwndAvail, GXLB_SETITEMDATA, i, (GXLPARAM)btnInfoNew);
      return;
    }
  }
  /* id higher than all others add to end */
  i = gxSendMessageW(hwndAvail, GXLB_ADDSTRINGW, 0, 0);
  gxSendMessageW(hwndAvail, GXLB_SETITEMDATA, i, (GXLPARAM)btnInfoNew);
}

static void TOOLBAR_Cust_MoveButton(const CUSTDLG_INFO *custInfo, GXHWND hwnd, GXINT nIndexFrom, GXINT nIndexTo)
{
  GXNMTOOLBARW nmtb;

  TRACE("index from %d, index to %d\n", nIndexFrom, nIndexTo);

  if (nIndexFrom == nIndexTo)
    return;

  /* MSDN states that iItem is the index of the button, rather than the
  * command ID as used by every other NMTOOLBAR notification */
  nmtb.iItem = nIndexFrom;
  if (TOOLBAR_SendNotify(&nmtb.hdr, custInfo->tbInfo, GXTBN_QUERYINSERT))
  {
    PCUSTOMBUTTON btnInfo;
    GXNMHDR hdr;
    GXHWND hwndList = gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX);
    GXINT count = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);

    btnInfo = (PCUSTOMBUTTON)gxSendMessageW(hwndList, GXLB_GETITEMDATA, nIndexFrom, 0);

    gxSendMessageW(hwndList, GXLB_DELETESTRING, nIndexFrom, 0);
    gxSendMessageW(hwndList, GXLB_INSERTSTRINGW, nIndexTo, 0);
    gxSendMessageW(hwndList, GXLB_SETITEMDATA, nIndexTo, (GXLPARAM)btnInfo);
    gxSendMessageW(hwndList, GXLB_SETCURSEL, nIndexTo, 0);

    if (nIndexTo <= 0)
      gxEnableWindow(gxGetDlgItem(hwnd,IDC_MOVEUP_BTN), FALSE);
    else
      gxEnableWindow(gxGetDlgItem(hwnd,IDC_MOVEUP_BTN), TRUE);

    /* last item is always separator, so -2 instead of -1 */
    if (nIndexTo >= (count - 2))
      gxEnableWindow(gxGetDlgItem(hwnd,IDC_MOVEDN_BTN), FALSE);
    else
      gxEnableWindow(gxGetDlgItem(hwnd,IDC_MOVEDN_BTN), TRUE);

    gxSendMessageW(custInfo->tbHwnd, GXTB_DELETEBUTTON, nIndexFrom, 0);
    gxSendMessageW(custInfo->tbHwnd, GXTB_INSERTBUTTONW, nIndexTo, (GXLPARAM)&(btnInfo->btn));

    TOOLBAR_SendNotify(&hdr, custInfo->tbInfo, GXTBN_TOOLBARCHANGE);
  }
}

static void TOOLBAR_Cust_AddButton(const CUSTDLG_INFO *custInfo, GXHWND hwnd, GXINT nIndexAvail, GXINT nIndexTo)
{
  GXNMTOOLBARW nmtb;

  TRACE("Add: nIndexAvail %d, nIndexTo %d\n", nIndexAvail, nIndexTo);

  /* MSDN states that iItem is the index of the button, rather than the
  * command ID as used by every other NMTOOLBAR notification */
  nmtb.iItem = nIndexAvail;
  if (TOOLBAR_SendNotify(&nmtb.hdr, custInfo->tbInfo, GXTBN_QUERYINSERT))
  {
    PCUSTOMBUTTON btnInfo;
    GXNMHDR hdr;
    GXHWND hwndList = gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX);
    GXHWND hwndAvail = gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX);
    GXINT count = gxSendMessageW(hwndAvail, GXLB_GETCOUNT, 0, 0);

    btnInfo = (PCUSTOMBUTTON)gxSendMessageW(hwndAvail, GXLB_GETITEMDATA, nIndexAvail, 0);

    if (nIndexAvail != 0) /* index == 0 indicates separator */
    {
      /* remove from 'available buttons' list */
      gxSendMessageW(hwndAvail, GXLB_DELETESTRING, nIndexAvail, 0);
      if (nIndexAvail == count-1)
        gxSendMessageW(hwndAvail, GXLB_SETCURSEL, nIndexAvail-1 , 0);
      else
        gxSendMessageW(hwndAvail, GXLB_SETCURSEL, nIndexAvail , 0);
    }
    else
    {
      PCUSTOMBUTTON btnNew;

      /* duplicate 'separator' button */
      btnNew = (PCUSTOMBUTTON)Alloc(sizeof(CUSTOMBUTTON));
      *btnNew = *btnInfo;
      btnInfo = btnNew;
    }

    /* insert into 'toolbar button' list */
    gxSendMessageW(hwndList, GXLB_INSERTSTRINGW, nIndexTo, 0);
    gxSendMessageW(hwndList, GXLB_SETITEMDATA, nIndexTo, (GXLPARAM)btnInfo);

    gxSendMessageW(custInfo->tbHwnd, GXTB_INSERTBUTTONW, nIndexTo, (GXLPARAM)&(btnInfo->btn));

    TOOLBAR_SendNotify(&hdr, custInfo->tbInfo, GXTBN_TOOLBARCHANGE);
  }
}

static void TOOLBAR_Cust_RemoveButton(const CUSTDLG_INFO *custInfo, GXHWND hwnd, GXINT index)
{
  PCUSTOMBUTTON btnInfo;
  GXHWND hwndList = gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX);

  TRACE("Remove: index %d\n", index);

  btnInfo = (PCUSTOMBUTTON)gxSendMessageW(hwndList, GXLB_GETITEMDATA, index, 0);

  /* send GXTBN_QUERYDELETE notification */
  if (TOOLBAR_IsButtonRemovable(custInfo->tbInfo, index, btnInfo))
  {
    GXNMHDR hdr;

    gxSendMessageW(hwndList, GXLB_DELETESTRING, index, 0);
    gxSendMessageW(hwndList, GXLB_SETCURSEL, index , 0);

    gxSendMessageW(custInfo->tbHwnd, GXTB_DELETEBUTTON, index, 0);

    /* insert into 'available button' list */
    if (!(btnInfo->btn.fsStyle & BTNS_SEP))
      TOOLBAR_Cust_InsertAvailButton(hwnd, btnInfo);
    else
      Free(btnInfo);

    TOOLBAR_SendNotify(&hdr, custInfo->tbInfo, GXTBN_TOOLBARCHANGE);
  }
}

/* drag list notification function for toolbar buttons list box */
static GXLRESULT TOOLBAR_Cust_ToolbarDragListNotification(const CUSTDLG_INFO *custInfo, GXHWND hwnd,
                              const GXDRAGLISTINFO *pDLI)
{
  GXHWND hwndList = gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX);
  switch (pDLI->uNotification)
  {
  case GXDL_BEGINDRAG:
    {
      GXINT nCurrentItem = gxLBItemFromPt(hwndList, pDLI->ptCursor, TRUE);
      GXINT nCount = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);
      /* no dragging for last item (separator) */
      if (nCurrentItem >= (nCount - 1)) return FALSE;
      return TRUE;
    }
  case GXDL_DRAGGING:
    {
      GXINT nCurrentItem = gxLBItemFromPt(hwndList, pDLI->ptCursor, TRUE);
      GXINT nCount = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);
      /* no dragging past last item (separator) */
      if ((nCurrentItem >= 0) && (nCurrentItem < (nCount - 1)))
      {
        gxDrawInsert(hwnd, hwndList, nCurrentItem);
        /* FIXME: native uses "move button" cursor */
        return GXDL_COPYCURSOR;
      }

      /* not over toolbar buttons list */
      if (nCurrentItem < 0)
      {
        GXPOINT ptWindow = pDLI->ptCursor;
        GXHWND hwndListAvail = gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX);
        gxMapWindowPoints(NULL, hwnd, &ptWindow, 1);
        /* over available buttons list? */
        if (gxChildWindowFromPoint(hwnd, ptWindow) == hwndListAvail)
          /* FIXME: native uses "move button" cursor */
          return GXDL_COPYCURSOR;
      }
      /* clear drag arrow */
      gxDrawInsert(hwnd, hwndList, -1);
      return GXDL_STOPCURSOR;
    }
  case GXDL_DROPPED:
    {
      GXINT nIndexTo = gxLBItemFromPt(hwndList, pDLI->ptCursor, TRUE);
      GXINT nIndexFrom = gxSendMessageW(hwndList, GXLB_GETCURSEL, 0, 0);
      GXINT nCount = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);
      if ((nIndexTo >= 0) && (nIndexTo < (nCount - 1)))
      {
        /* clear drag arrow */
        gxDrawInsert(hwnd, hwndList, -1);
        /* move item */
        TOOLBAR_Cust_MoveButton(custInfo, hwnd, nIndexFrom, nIndexTo);
      }
      /* not over toolbar buttons list */
      if (nIndexTo < 0)
      {
        GXPOINT ptWindow = pDLI->ptCursor;
        GXHWND hwndListAvail = gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX);
        gxMapWindowPoints(NULL, hwnd, &ptWindow, 1);
        /* over available buttons list? */
        if (gxChildWindowFromPoint(hwnd, ptWindow) == hwndListAvail)
          TOOLBAR_Cust_RemoveButton(custInfo, hwnd, nIndexFrom);
      }
      break;
    }
  case GXDL_CANCELDRAG:
    /* Clear drag arrow */
    gxDrawInsert(hwnd, hwndList, -1);
    break;
  }

  return 0;
}

/* drag list notification function for available buttons list box */
static GXLRESULT TOOLBAR_Cust_AvailDragListNotification(const CUSTDLG_INFO *custInfo, GXHWND hwnd,
                            const GXDRAGLISTINFO *pDLI)
{
  GXHWND hwndList = gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX);
  switch (pDLI->uNotification)
  {
  case GXDL_BEGINDRAG:
    return TRUE;
  case GXDL_DRAGGING:
    {
      GXINT nCurrentItem = gxLBItemFromPt(hwndList, pDLI->ptCursor, TRUE);
      GXINT nCount = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);
      /* no dragging past last item (separator) */
      if ((nCurrentItem >= 0) && (nCurrentItem < nCount))
      {
        gxDrawInsert(hwnd, hwndList, nCurrentItem);
        /* FIXME: native uses "move button" cursor */
        return GXDL_COPYCURSOR;
      }

      /* not over toolbar buttons list */
      if (nCurrentItem < 0)
      {
        GXPOINT ptWindow = pDLI->ptCursor;
        GXHWND hwndListAvail = gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX);
        gxMapWindowPoints(NULL, hwnd, &ptWindow, 1);
        /* over available buttons list? */
        if (gxChildWindowFromPoint(hwnd, ptWindow) == hwndListAvail)
          /* FIXME: native uses "move button" cursor */
          return GXDL_COPYCURSOR;
      }
      /* clear drag arrow */
      gxDrawInsert(hwnd, hwndList, -1);
      return GXDL_STOPCURSOR;
    }
  case GXDL_DROPPED:
    {
      GXINT nIndexTo = gxLBItemFromPt(hwndList, pDLI->ptCursor, TRUE);
      GXINT nCount = gxSendMessageW(hwndList, GXLB_GETCOUNT, 0, 0);
      GXINT nIndexFrom = gxSendDlgItemMessageW(hwnd, IDC_AVAILBTN_LBOX, GXLB_GETCURSEL, 0, 0);
      if ((nIndexTo >= 0) && (nIndexTo < nCount))
      {
        /* clear drag arrow */
        gxDrawInsert(hwnd, hwndList, -1);
        /* add item */
        TOOLBAR_Cust_AddButton(custInfo, hwnd, nIndexFrom, nIndexTo);
      }
    }
  case GXDL_CANCELDRAG:
    /* Clear drag arrow */
    gxDrawInsert(hwnd, hwndList, -1);
    break;
  }
  return 0;
}

extern GXUINT uDragListMessage;

/***********************************************************************
* TOOLBAR_CustomizeDialogProc
* This function implements the toolbar customization dialog.
*/
static GXINT_PTR CALLBACK
TOOLBAR_CustomizeDialogProc(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  PCUSTDLG_INFO custInfo = (PCUSTDLG_INFO)gxGetWindowLongPtrW (hwnd, GXDWLP_USER);
  PCUSTOMBUTTON btnInfo;
  GXNMTOOLBARA nmtb;
  TOOLBAR_INFO *infoPtr = custInfo ? custInfo->tbInfo : NULL;

  switch (uMsg)
  {
  case GXWM_INITDIALOG:
    custInfo = (PCUSTDLG_INFO)lParam;
    gxSetWindowLongPtrW (hwnd, GXDWLP_USER, (GXLONG_PTR)custInfo);

    if (custInfo)
    {
      GXWCHAR Buffer[256];
      GXINT i = 0;
      GXINT index;
      GXNMTBINITCUSTOMIZE nmtbic;

      infoPtr = custInfo->tbInfo;

      /* send GXTBN_QUERYINSERT notification */
      nmtb.iItem = custInfo->tbInfo->nNumButtons;

      if (!TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_QUERYINSERT))
        return FALSE;

      nmtbic.hwndDialog = hwnd;
      /* Send GXTBN_INITCUSTOMIZE notification */
      if (TOOLBAR_SendNotify (&nmtbic.hdr, infoPtr, GXTBN_INITCUSTOMIZE) ==
        GXTBNRF_HIDEHELP)
      {
        TRACE("GXTBNRF_HIDEHELP requested\n");
        gxShowWindow(gxGetDlgItem(hwnd, IDC_HELP_BTN), GXSW_HIDE);
      }

      /* add items to 'toolbar buttons' list and check if removable */
      for (i = 0; i < custInfo->tbInfo->nNumButtons; i++)
      {
        btnInfo = (PCUSTOMBUTTON)Alloc(sizeof(CUSTOMBUTTON));
        memset (&btnInfo->btn, 0, sizeof(GXTBBUTTON));
        btnInfo->btn.fsStyle = BTNS_SEP;
        btnInfo->bVirtual = FALSE;
        gxLoadStringW (COMCTL32_hModule, IDS_SEPARATOR, btnInfo->text, 64);

        /* send GXTBN_QUERYDELETE notification */
        btnInfo->bRemovable = TOOLBAR_IsButtonRemovable(infoPtr, i, btnInfo);

        index = (GXINT)gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_ADDSTRINGW, 0, 0);
        gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETITEMDATA, index, (GXLPARAM)btnInfo);
      }

      gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETITEMHEIGHT, 0, infoPtr->nBitmapHeight + 8);

      /* insert separator button into 'available buttons' list */
      btnInfo = (PCUSTOMBUTTON)Alloc(sizeof(CUSTOMBUTTON));
      memset (&btnInfo->btn, 0, sizeof(GXTBBUTTON));
      btnInfo->btn.fsStyle = BTNS_SEP;
      btnInfo->bVirtual = FALSE;
      btnInfo->bRemovable = TRUE;
      gxLoadStringW (COMCTL32_hModule, IDS_SEPARATOR, btnInfo->text, 64);
      index = (GXINT)gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_ADDSTRINGW, 0, (GXLPARAM)btnInfo);
      gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_SETITEMDATA, index, (GXLPARAM)btnInfo);

      /* insert all buttons into dsa */
      for (i = 0;; i++)
      {
        /* send GXTBN_GETBUTTONINFO notification */
        GXNMTOOLBARW nmtb;
        nmtb.iItem = i;
        nmtb.pszText = Buffer;
        nmtb.cchText = 256;

        /* Clear previous button's text */
        ZeroMemory(nmtb.pszText, nmtb.cchText * sizeof(GXWCHAR));

        if (!TOOLBAR_GetButtonInfo(infoPtr, &nmtb))
          break;
#ifndef DISABLE_CODE
        TRACE("WM_INITDIALOG style: %x iItem(%d) idCommand(%d) iString(%ld) %s\n",
          nmtb.tbButton.fsStyle, i, 
          nmtb.tbButton.idCommand,
          nmtb.tbButton.iString,
          nmtb.tbButton.iString >= 0 ? debugstr_w(infoPtr->strings[nmtb.tbButton.iString])
          : "");
#endif // DISABLE_CODE
        /* insert button into the apropriate list */
        index = TOOLBAR_GetButtonIndex (custInfo->tbInfo, (GXINT)nmtb.tbButton.idCommand, FALSE);
        if (index == -1)
        {
          btnInfo = (PCUSTOMBUTTON)Alloc(sizeof(CUSTOMBUTTON));
          btnInfo->bVirtual = FALSE;
          btnInfo->bRemovable = TRUE;
        }
        else
        {
          btnInfo = (PCUSTOMBUTTON)(GXLONG_PTR)gxSendDlgItemMessageW (hwnd, 
            IDC_TOOLBARBTN_LBOX, GXLB_GETITEMDATA, index, 0);
        }

        btnInfo->btn = nmtb.tbButton;
        if (!(nmtb.tbButton.fsStyle & BTNS_SEP))
        {
          if (GXSTRLEN(nmtb.pszText))
            GXSTRCPY(btnInfo->text, nmtb.pszText);
          else if (nmtb.tbButton.iString >= 0 && 
            nmtb.tbButton.iString < infoPtr->nNumStrings)
          {
            GXSTRCPY(btnInfo->text, 
              infoPtr->strings[nmtb.tbButton.iString]);
          }
        }

        if (index == -1)
          TOOLBAR_Cust_InsertAvailButton(hwnd, btnInfo);
      }

      gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_SETITEMHEIGHT, 0, infoPtr->nBitmapHeight + 8);

      /* select first item in the 'available' list */
      gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_SETCURSEL, 0, 0);

      /* append 'virtual' separator button to the 'toolbar buttons' list */
      btnInfo = (PCUSTOMBUTTON)Alloc(sizeof(CUSTOMBUTTON));
      memset (&btnInfo->btn, 0, sizeof(GXTBBUTTON));
      btnInfo->btn.fsStyle = BTNS_SEP;
      btnInfo->bVirtual = TRUE;
      btnInfo->bRemovable = FALSE;
      gxLoadStringW (COMCTL32_hModule, IDS_SEPARATOR, btnInfo->text, 64);
      index = (GXINT)gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_ADDSTRINGW, 0, (GXLPARAM)btnInfo);
      gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETITEMDATA, index, (GXLPARAM)btnInfo);

      /* select last item in the 'toolbar' list */
      gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETCURSEL, index, 0);
      gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETTOPINDEX, index, 0);

      gxMakeDragList(gxGetDlgItem(hwnd, IDC_TOOLBARBTN_LBOX));
      gxMakeDragList(gxGetDlgItem(hwnd, IDC_AVAILBTN_LBOX));

      /* set focus and disable buttons */
      gxPostMessageW (hwnd, GXWM_USER, 0, 0);
    }
    return TRUE;

  case GXWM_USER:
    gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEUP_BTN), FALSE);
    gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEDN_BTN), FALSE);
    gxEnableWindow (gxGetDlgItem (hwnd,IDC_REMOVE_BTN), FALSE);
    gxSetFocus (gxGetDlgItem (hwnd, IDC_TOOLBARBTN_LBOX));
    return TRUE;

  case GXWM_CLOSE:
    gxEndDialog(hwnd, FALSE);
    return TRUE;

  case GXWM_COMMAND:
    switch (GXLOWORD(wParam))
    {
    case IDC_TOOLBARBTN_LBOX:
      if (GXHIWORD(wParam) == GXLBN_SELCHANGE)
      {
        PCUSTOMBUTTON btnInfo;
        GXNMTOOLBARA nmtb;
        GXINT count;
        GXINT index;

        count = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCOUNT, 0, 0);
        index = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCURSEL, 0, 0);

        /* send GXTBN_QUERYINSERT notification */
        nmtb.iItem = index;
        TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_QUERYINSERT);

        /* get list box item */
        btnInfo = (PCUSTOMBUTTON)(GXLONG_PTR)gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETITEMDATA, index, 0);

        if (index == (count - 1))
        {
          /* last item (virtual separator) */
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEUP_BTN), FALSE);
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEDN_BTN), FALSE);
        }
        else if (index == (count - 2))
        {
          /* second last item (last non-virtual item) */
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEUP_BTN), TRUE);
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEDN_BTN), FALSE);
        }
        else if (index == 0)
        {
          /* first item */
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEUP_BTN), FALSE);
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEDN_BTN), TRUE);
        }
        else
        {
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEUP_BTN), TRUE);
          gxEnableWindow (gxGetDlgItem (hwnd,IDC_MOVEDN_BTN), TRUE);
        }

        gxEnableWindow (gxGetDlgItem (hwnd,IDC_REMOVE_BTN), btnInfo->bRemovable);
      }
      break;

    case IDC_MOVEUP_BTN:
      {
        GXINT index = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCURSEL, 0, 0);
        TOOLBAR_Cust_MoveButton(custInfo, hwnd, index, index-1);
      }
      break;

    case IDC_MOVEDN_BTN: /* move down */
      {
        GXINT index = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCURSEL, 0, 0);
        TOOLBAR_Cust_MoveButton(custInfo, hwnd, index, index+1);
      }
      break;

    case IDC_REMOVE_BTN: /* remove button */
      {
        GXINT index = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCURSEL, 0, 0);

        if (GXLB_ERR == index)
          break;

        TOOLBAR_Cust_RemoveButton(custInfo, hwnd, index);
      }
      break;
    case IDC_HELP_BTN:
      TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_CUSTHELP);
      break;
    case IDC_RESET_BTN:
      TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_RESET);
      break;

    case IDOK: /* Add button */
      {
        GXINT index;
        GXINT indexto;

        index = gxSendDlgItemMessageW(hwnd, IDC_AVAILBTN_LBOX, GXLB_GETCURSEL, 0, 0);
        indexto = gxSendDlgItemMessageW(hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCURSEL, 0, 0);

        TOOLBAR_Cust_AddButton(custInfo, hwnd, index, indexto);
      }
      break;

    case IDCANCEL:
      gxEndDialog(hwnd, FALSE);
      break;
    }
    return TRUE;

  case GXWM_DESTROY:
    {
      GXINT count;
      GXINT i;

      /* delete items from 'toolbar buttons' listbox*/
      count = gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETCOUNT, 0, 0);
      for (i = 0; i < count; i++)
      {
        btnInfo = (PCUSTOMBUTTON)(GXLONG_PTR)gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_GETITEMDATA, i, 0);
        Free(btnInfo);
        gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_SETITEMDATA, 0, 0);
      }
      gxSendDlgItemMessageW (hwnd, IDC_TOOLBARBTN_LBOX, GXLB_RESETCONTENT, 0, 0);


      /* delete items from 'available buttons' listbox*/
      count = gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_GETCOUNT, 0, 0);
      for (i = 0; i < count; i++)
      {
        btnInfo = (PCUSTOMBUTTON)(GXLONG_PTR)gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_GETITEMDATA, i, 0);
        Free(btnInfo);
        gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_SETITEMDATA, i, 0);
      }
      gxSendDlgItemMessageW (hwnd, IDC_AVAILBTN_LBOX, GXLB_RESETCONTENT, 0, 0);
    }
    return TRUE;

  case GXWM_DRAWITEM:
    if (wParam == IDC_AVAILBTN_LBOX || wParam == IDC_TOOLBARBTN_LBOX)
    {
      GXLPDRAWITEMSTRUCT lpdis = (GXLPDRAWITEMSTRUCT)lParam;
      GXRECT rcButton;
      GXRECT rcText;
      GXHPEN hPen, hOldPen;
      GXHBRUSH hOldBrush;
      GXCOLORREF oldText = 0;
      GXCOLORREF oldBk = 0;

      /* get item data */
      btnInfo = (PCUSTOMBUTTON)(GXLONG_PTR)gxSendDlgItemMessageW (hwnd, wParam, GXLB_GETITEMDATA, (GXWPARAM)lpdis->itemID, 0);
      if (btnInfo == NULL)
      {
        FIXME("btnInfo invalid!\n");
        return TRUE;
      }

      /* set colors and select objects */
      oldBk = gxSetBkColor (lpdis->hDC, (lpdis->itemState & GXODS_FOCUS)?comctl32_color.clrHighlight:comctl32_color.clrWindow);
      if (btnInfo->bVirtual)
        oldText = gxSetTextColor (lpdis->hDC, comctl32_color.clrGrayText);
      else
        oldText = gxSetTextColor (lpdis->hDC, (lpdis->itemState & GXODS_FOCUS)?comctl32_color.clrHighlightText:comctl32_color.clrWindowText);
      hPen = gxCreatePen( GXPS_SOLID, 1,
        gxGetSysColor( (lpdis->itemState & GXODS_SELECTED)?GXCOLOR_HIGHLIGHT:GXCOLOR_WINDOW));
      hOldPen = (GXHPEN)gxSelectObject (lpdis->hDC, hPen );
      hOldBrush = (GXHBRUSH)gxSelectObject (lpdis->hDC, gxGetSysColorBrush ((lpdis->itemState & GXODS_FOCUS)?GXCOLOR_HIGHLIGHT:GXCOLOR_WINDOW));

      /* fill background rectangle */
      gxRectangle (lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top,
        lpdis->rcItem.right, lpdis->rcItem.bottom);

      /* calculate button and text rectangles */
      gxCopyRect (&rcButton, &lpdis->rcItem);
      gxInflateRect (&rcButton, -1, -1);
      gxCopyRect (&rcText, &rcButton);
      rcButton.right = rcButton.left + custInfo->tbInfo->nBitmapWidth + 6;
      rcText.left = rcButton.right + 2;

      /* draw focus rectangle */
      if (lpdis->itemState & GXODS_FOCUS)
        gxDrawFocusRect (lpdis->hDC, &lpdis->rcItem);

      /* draw button */
      if (!(infoPtr->dwStyle & TBSTYLE_FLAT))
        gxDrawEdge (lpdis->hDC, &rcButton, GXEDGE_RAISED, GXBF_RECT|GXBF_MIDDLE|GXBF_SOFT);

      /* draw image and text */
      if ((btnInfo->btn.fsStyle & BTNS_SEP) == 0) {
        GXHIMAGELIST himl = GETDEFIMAGELIST(infoPtr, GETHIMLID(infoPtr, 
          btnInfo->btn.iBitmap));
        gxImageList_Draw (himl, GETIBITMAP(infoPtr, btnInfo->btn.iBitmap), 
          lpdis->hDC, rcButton.left+3, rcButton.top+3, ILD_NORMAL);
      }
      gxDrawTextW (lpdis->hDC,  btnInfo->text, -1, &rcText,
        GXDT_LEFT | GXDT_VCENTER | GXDT_SINGLELINE);

      /* delete objects and reset colors */
      gxSelectObject (lpdis->hDC, hOldBrush);
      gxSelectObject (lpdis->hDC, hOldPen);
      gxSetBkColor (lpdis->hDC, oldBk);
      gxSetTextColor (lpdis->hDC, oldText);
      gxDeleteObject( hPen );
      return TRUE;
    }
    return FALSE;

  case GXWM_MEASUREITEM:
    if (wParam == IDC_AVAILBTN_LBOX || wParam == IDC_TOOLBARBTN_LBOX)
    {
      GXMEASUREITEMSTRUCT *lpmis = (GXMEASUREITEMSTRUCT*)lParam;

      lpmis->itemHeight = 15 + 8; /* default height */

      return TRUE;
    }
    return FALSE;

  default:
    if (uDragListMessage && (uMsg == uDragListMessage))
    {
      if (wParam == IDC_TOOLBARBTN_LBOX)
      {
        GXLRESULT res = TOOLBAR_Cust_ToolbarDragListNotification(
          custInfo, hwnd, (GXDRAGLISTINFO *)lParam);
        gxSetWindowLongPtrW(hwnd, GXDWLP_MSGRESULT, res);
        return TRUE;
      }
      else if (wParam == IDC_AVAILBTN_LBOX)
      {
        GXLRESULT res = TOOLBAR_Cust_AvailDragListNotification(
          custInfo, hwnd, (GXDRAGLISTINFO *)lParam);
        gxSetWindowLongPtrW(hwnd, GXDWLP_MSGRESULT, res);
        return TRUE;
      }
    }
    return FALSE;
  }
}

static GXBOOL
TOOLBAR_AddBitmapToImageList(TOOLBAR_INFO *infoPtr, GXHIMAGELIST himlDef, const TBITMAP_INFO *bitmap)
{
  GXHBITMAP hbmLoad;
  GXINT nCountBefore = gxImageList_GetImageCount(himlDef);
  GXINT nCountAfter;
  GXINT cxIcon, cyIcon;
  GXINT nAdded;
  GXINT nIndex;

  TRACE("adding hInst=%p nID=%d nButtons=%d\n", bitmap->hInst, bitmap->nID, bitmap->nButtons);
  /* Add bitmaps to the default image list */
  if (bitmap->hInst == NULL)         /* a handle was passed */
    hbmLoad = (GXHBITMAP)gxCopyImage((GXHANDLE)gxULongToHandle(bitmap->nID), GXIMAGE_BITMAP, 0, 0, 0);
  else
    hbmLoad = gxCreateMappedBitmap(bitmap->hInst, bitmap->nID, 0, NULL, 0);

  /* enlarge the bitmap if needed */
  gxImageList_GetIconSize(himlDef, &cxIcon, &cyIcon);
  if (bitmap->hInst != COMCTL32_hModule)
    gxCOMCTL32_EnsureBitmapSize(&hbmLoad, cxIcon*(GXINT)bitmap->nButtons, cyIcon, comctl32_color.clrBtnFace);

  nIndex = gxImageList_AddMasked(himlDef, hbmLoad, comctl32_color.clrBtnFace);
  gxDeleteObject(hbmLoad);
  if (nIndex == -1)
    return FALSE;

  nCountAfter = gxImageList_GetImageCount(himlDef);
  nAdded =  nCountAfter - nCountBefore;
  if (bitmap->nButtons == 0) /* wParam == 0 is special and means add only one image */
  {
    gxImageList_SetImageCount(himlDef, nCountBefore + 1);
  } else if (nAdded > (GXINT)bitmap->nButtons) {
    TRACE("Added more images than wParam: Previous image number %i added %i while wParam %i. Images in list %i\n",
      nCountBefore, nAdded, bitmap->nButtons, nCountAfter);
  }

  infoPtr->nNumBitmaps += nAdded;
  return TRUE;
}

static void
TOOLBAR_CheckImageListIconSize(TOOLBAR_INFO *infoPtr)
{
  GXHIMAGELIST himlDef;
  GXHIMAGELIST himlNew;
  GXINT cx, cy;
  GXINT i;

  himlDef = GETDEFIMAGELIST(infoPtr, 0);
  if (himlDef == NULL || himlDef != infoPtr->himlInt)
    return;
  if (!gxImageList_GetIconSize(himlDef, &cx, &cy))
    return;
  if (cx == infoPtr->nBitmapWidth && cy == infoPtr->nBitmapHeight)
    return;

  TRACE("Update icon size: %dx%d -> %dx%d\n",
    cx, cy, infoPtr->nBitmapWidth, infoPtr->nBitmapHeight);

  himlNew = gxImageList_Create(infoPtr->nBitmapWidth, infoPtr->nBitmapHeight,
    GXILC_COLORDDB|GXILC_MASK, 8, 2);
  for (i = 0; i < infoPtr->nNumBitmapInfos; i++)
    TOOLBAR_AddBitmapToImageList(infoPtr, himlNew, &infoPtr->bitmaps[i]);
  TOOLBAR_InsertImageList(&infoPtr->himlDef, &infoPtr->cimlDef, himlNew, 0);
  infoPtr->himlInt = himlNew;

  infoPtr->nNumBitmaps -= gxImageList_GetImageCount(himlDef);
  gxImageList_Destroy(himlDef);
}

/***********************************************************************
* TOOLBAR_AddBitmap:  Add the bitmaps to the default image list.
*
*/
static GXLRESULT
TOOLBAR_AddBitmap (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  LPGXTBADDBITMAP lpAddBmp = (LPGXTBADDBITMAP)lParam;
  TBITMAP_INFO info;
  GXINT iSumButtons, i;
  GXHIMAGELIST himlDef;

  TRACE("hwnd=%p wParam=%lx lParam=%lx\n", hwnd, wParam, lParam);
  if (!lpAddBmp)
    return -1;

  if (lpAddBmp->hInst == GXHINST_COMMCTRL)
  {
    info.hInst = COMCTL32_hModule;
    switch (lpAddBmp->nID)
    {
    case IDB_STD_SMALL_COLOR:
      info.nButtons = 15;
      info.nID = IDB_STD_SMALL;
      break;
    case IDB_STD_LARGE_COLOR:
      info.nButtons = 15;
      info.nID = IDB_STD_LARGE;
      break;
    case IDB_VIEW_SMALL_COLOR:
      info.nButtons = 12;
      info.nID = IDB_VIEW_SMALL;
      break;
    case IDB_VIEW_LARGE_COLOR:
      info.nButtons = 12;
      info.nID = IDB_VIEW_LARGE;
      break;
    case IDB_HIST_SMALL_COLOR:
      info.nButtons = 5;
      info.nID = IDB_HIST_SMALL;
      break;
    case IDB_HIST_LARGE_COLOR:
      info.nButtons = 5;
      info.nID = IDB_HIST_LARGE;
      break;
    default:
      return -1;
    }

    TRACE ("adding %d internal bitmaps!\n", info.nButtons);

    /* Windows resize all the buttons to the size of a newly added standard image */
    if (lpAddBmp->nID & 1)
    {
      /* large icons: 24x24. Will make the button 31x30 */
      gxSendMessageW (hwnd, GXTB_SETBITMAPSIZE, 0, GXMAKELPARAM(24, 24));
    }
    else
    {
      /* small icons: 16x16. Will make the buttons 23x22 */
      gxSendMessageW (hwnd, GXTB_SETBITMAPSIZE, 0, GXMAKELPARAM(16, 16));
    }

    TOOLBAR_CalcToolbar (hwnd);
  }
  else
  {
    info.nButtons = (GXINT)wParam;
    info.hInst = lpAddBmp->hInst;
    info.nID = lpAddBmp->nID;
    TRACE("adding %d bitmaps!\n", info.nButtons);
  }

  /* check if the bitmap is already loaded and compute iSumButtons */
  iSumButtons = 0;
  for (i = 0; i < infoPtr->nNumBitmapInfos; i++)
  {
    if (infoPtr->bitmaps[i].hInst == info.hInst &&
      infoPtr->bitmaps[i].nID == info.nID)
      return iSumButtons;
    iSumButtons += infoPtr->bitmaps[i].nButtons;
  }

  if (!infoPtr->cimlDef) {
    /* create new default image list */
    TRACE ("creating default image list!\n");

    himlDef = gxImageList_Create (infoPtr->nBitmapWidth, infoPtr->nBitmapHeight,
      GXILC_COLORDDB | GXILC_MASK, info.nButtons, 2);
    TOOLBAR_InsertImageList(&infoPtr->himlDef, &infoPtr->cimlDef, himlDef, 0);
    infoPtr->himlInt = himlDef;
  }
  else {
    himlDef = GETDEFIMAGELIST(infoPtr, 0);
  }

  if (!himlDef) {
    WARN("No default image list available\n");
    return -1;
  }

  if (!TOOLBAR_AddBitmapToImageList(infoPtr, himlDef, &info))
    return -1;

  TRACE("Number of bitmap infos: %d\n", infoPtr->nNumBitmapInfos);
  infoPtr->bitmaps = (TBITMAP_INFO*)ReAlloc(infoPtr->bitmaps, (infoPtr->nNumBitmapInfos + 1) * sizeof(TBITMAP_INFO));
  infoPtr->bitmaps[infoPtr->nNumBitmapInfos] = info;
  infoPtr->nNumBitmapInfos++;
  TRACE("Number of bitmap infos: %d\n", infoPtr->nNumBitmapInfos);

  gxInvalidateRect(hwnd, NULL, TRUE);
  return iSumButtons;
}


static GXLRESULT
TOOLBAR_AddButtonsT(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam, GXBOOL fUnicode)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBBUTTON lpTbb = (GXLPTBBUTTON)lParam;
  GXINT nAddButtons = (GXUINT)wParam;

  TRACE("adding %ld buttons (unicode=%d)!\n", wParam, fUnicode);

  return TOOLBAR_InternalInsertButtonsT(infoPtr, -1, nAddButtons, lpTbb, fUnicode);
}


static GXLRESULT
TOOLBAR_AddStringW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
#define MAX_RESOURCE_STRING_LENGTH 512
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXBOOL fFirstString = (infoPtr->nNumStrings == 0);
  GXINT nIndex = infoPtr->nNumStrings;

  if ((wParam) && (GXHIWORD(lParam) == 0)) {
    GXWCHAR szString[MAX_RESOURCE_STRING_LENGTH];
    GXWCHAR delimiter;
    GXWCHAR *next_delim;
    GXWCHAR *p;
    GXINT len;
    TRACE("adding string from resource!\n");

    len = gxLoadStringW ((GXHINSTANCE)wParam, (GXUINT)lParam,
      szString, MAX_RESOURCE_STRING_LENGTH);

    TRACE("len=%d %s\n", len, debugstr_w(szString));
    if (len == 0 || len == 1)
      return nIndex;

    TRACE("Delimiter: 0x%x\n", *szString);
    delimiter = *szString;
    p = szString + 1;

    while ((next_delim = strchrW(p, delimiter)) != NULL) {
      *next_delim = 0;
      if (next_delim + 1 >= szString + len)
      {
        /* this may happen if delimiter == '\0' or if the last char is a
        * delimiter (then it is ignored like the native does) */
        break;
      }

      infoPtr->strings = (GXLPWSTR*)ReAlloc(infoPtr->strings, sizeof(GXLPWSTR)*(infoPtr->nNumStrings+1));
      gxStr_SetPtrW(&infoPtr->strings[infoPtr->nNumStrings], p);
      infoPtr->nNumStrings++;

      p = next_delim + 1;
    }
  }
  else {
    GXLPWSTR p = (GXLPWSTR)lParam;
    GXINT len;

    if (p == NULL)
      return -1;
    TRACE("adding string(s) from array!\n");
    while (*p) {
      len = (int)GXSTRLEN(p);

      TRACE("len=%d %s\n", len, debugstr_w(p));
      infoPtr->strings = (GXLPWSTR*)ReAlloc(infoPtr->strings, sizeof(GXLPWSTR)*(infoPtr->nNumStrings+1));
      gxStr_SetPtrW (&infoPtr->strings[infoPtr->nNumStrings], p);
      infoPtr->nNumStrings++;

      p += (len+1);
    }
  }

  if (fFirstString)
    TOOLBAR_CalcToolbar(hwnd);
  return nIndex;
}


static GXLRESULT
TOOLBAR_AddStringA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXBOOL fFirstString = (infoPtr->nNumStrings == 0);
  GXLPSTR p;
  GXINT nIndex;
  GXINT len;

  if ((wParam) && (GXHIWORD(lParam) == 0))  /* load from resources */
    return TOOLBAR_AddStringW(hwnd, wParam, lParam);

  p = (GXLPSTR)lParam;
  if (p == NULL)
    return -1;

  TRACE("adding string(s) from array!\n");
  nIndex = infoPtr->nNumStrings;
  while (*p) {
    len = (GXINT)strlen (p);
    TRACE("len=%d \"%s\"\n", len, p);

    infoPtr->strings = (GXLPWSTR*)ReAlloc(infoPtr->strings, sizeof(GXLPWSTR)*(infoPtr->nNumStrings+1));
    gxStr_SetPtrAtoW(&infoPtr->strings[infoPtr->nNumStrings], p);
    infoPtr->nNumStrings++;

    p += (len+1);
  }

  if (fFirstString)
    TOOLBAR_CalcToolbar(hwnd);
  return nIndex;
}


static GXLRESULT
TOOLBAR_AutoSize (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXRECT parent_rect;
  GXHWND parent;
  GXINT  x, y;
  GXINT  cx, cy;

  TRACE("auto sizing, style=%x!\n", infoPtr->dwStyle);

  parent = gxGetParent (hwnd);

  if (!parent || !infoPtr->bDoRedraw)
    return 0;

  gxGetClientRect(parent, &parent_rect);

  x = parent_rect.left;
  y = parent_rect.top;

  TRACE("nRows: %d, infoPtr->nButtonHeight: %d\n", infoPtr->nRows, infoPtr->nButtonHeight);

  cy = TOP_BORDER + infoPtr->nRows * infoPtr->nButtonHeight + BOTTOM_BORDER;
  cx = parent_rect.right - parent_rect.left;

  if ((infoPtr->dwStyle & TBSTYLE_WRAPABLE) || (infoPtr->dwExStyle & TBSTYLE_EX_UNDOC1))
  {
    TOOLBAR_LayoutToolbar(hwnd);
    gxInvalidateRect( hwnd, NULL, TRUE );
  }

  if (!(infoPtr->dwStyle & CCS_NORESIZE))
  {
    GXRECT window_rect;
    GXUINT uPosFlags = GXSWP_NOZORDER;

    if ((infoPtr->dwStyle & CCS_BOTTOM) == CCS_NOMOVEY)
    {
      gxGetWindowRect(hwnd, &window_rect);
      gxScreenToClient(parent, (LPGXPOINT)&window_rect.left);
      y = window_rect.top;
    }
    if ((infoPtr->dwStyle & CCS_BOTTOM) == CCS_BOTTOM)
    {
      gxGetWindowRect(hwnd, &window_rect);
      y = parent_rect.bottom - ( window_rect.bottom - window_rect.top);
    }

    if (infoPtr->dwStyle & CCS_NOPARENTALIGN)
      uPosFlags |= GXSWP_NOMOVE;

    if (!(infoPtr->dwStyle & CCS_NODIVIDER))
      cy += gxGetSystemMetrics(GXSM_CYEDGE);

    if (infoPtr->dwStyle & GXWS_BORDER)
    {
      x = y = 1; /* FIXME: this looks wrong */
      cy += gxGetSystemMetrics(GXSM_CYEDGE);
      cx += gxGetSystemMetrics(GXSM_CXEDGE);
    }

    gxSetWindowPos(hwnd, NULL, x, y, cx, cy, uPosFlags);
  }

  return 0;
}


static GXLRESULT
TOOLBAR_ButtonCount (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return infoPtr->nNumButtons;
}


static GXLRESULT
TOOLBAR_ButtonStructSize (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  infoPtr->dwStructSize = (GXDWORD)wParam;

  return 0;
}


static GXLRESULT
TOOLBAR_ChangeBitmap (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;

  TRACE("button %ld, iBitmap now %d\n", wParam, GXLOWORD(lParam));

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  btnPtr->iBitmap = GXLOWORD(lParam);

  /* we HAVE to erase the background, the new bitmap could be */
  /* transparent */
  gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_CheckButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXINT nOldIndex = -1;
  GXBOOL bChecked = FALSE;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);

  TRACE("hwnd=%p, btn index=%d, lParam=0x%08lx\n", hwnd, nIndex, lParam);

  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];

  bChecked = (btnPtr->fsState & TBSTATE_CHECKED) ? TRUE : FALSE;

  if (GXLOWORD(lParam) == FALSE)
    btnPtr->fsState &= ~TBSTATE_CHECKED;
  else {
    if (btnPtr->fsStyle & BTNS_GROUP) {
      nOldIndex =
        TOOLBAR_GetCheckedGroupButtonIndex (infoPtr, nIndex);
      if (nOldIndex == nIndex)
        return 0;
      if (nOldIndex != -1)
        infoPtr->buttons[nOldIndex].fsState &= ~TBSTATE_CHECKED;
    }
    btnPtr->fsState |= TBSTATE_CHECKED;
  }

  if( bChecked != GXLOWORD(lParam) )
  {
    if (nOldIndex != -1)
      gxInvalidateRect(hwnd, &infoPtr->buttons[nOldIndex].rect, TRUE);
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
  }

  /* FIXME: Send a WM_NOTIFY?? */

  return TRUE;
}


static GXLRESULT
TOOLBAR_CommandToIndex (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
}


static GXLRESULT
TOOLBAR_Customize (GXHWND hwnd)
{
#ifndef DISABLE_CODE
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  CUSTDLG_INFO custInfo;
  GXLRESULT ret;
  LPCVOID dlgTemplate;
  HRSRC hRes;
  GXNMHDR nmhdr;

  custInfo.tbInfo = infoPtr;
  custInfo.tbHwnd = hwnd;

  /* send GXTBN_BEGINADJUST notification */
  TOOLBAR_SendNotify (&nmhdr, infoPtr, GXTBN_BEGINADJUST);

  if (!(hRes = gxFindResourceW (COMCTL32_hModule,
    GXMAKEINTRESOURCEW(IDD_TBCUSTOMIZE),
    (GXLPWSTR)RT_DIALOG)))
    return FALSE;

  if(!(dlgTemplate = LoadResource (COMCTL32_hModule, hRes)))
    return FALSE;

  ret = gxDialogBoxIndirectParamW ((GXHINSTANCE)gxGetWindowLongPtrW(hwnd, GXGWLP_HINSTANCE),
    dlgTemplate, hwnd, TOOLBAR_CustomizeDialogProc,
    (GXLPARAM)&custInfo);

  /* send GXTBN_ENDADJUST notification */
  TOOLBAR_SendNotify (&nmhdr, infoPtr, GXTBN_ENDADJUST);

  return ret;
#else
  ASSERT(false);
  return NULL;
#endif // DISABLE_CODE
}


static GXLRESULT
TOOLBAR_DeleteButton (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex = (GXINT)wParam;
  GXNMTOOLBARW nmtb;
  TBUTTON_INFO *btnPtr = &infoPtr->buttons[nIndex];

  if ((nIndex < 0) || (nIndex >= infoPtr->nNumButtons))
    return FALSE;

  memset(&nmtb, 0, sizeof(nmtb));
  nmtb.iItem = btnPtr->idCommand;
  nmtb.tbButton.iBitmap = btnPtr->iBitmap;
  nmtb.tbButton.idCommand = btnPtr->idCommand;
  nmtb.tbButton.fsState = btnPtr->fsState;
  nmtb.tbButton.fsStyle = btnPtr->fsStyle;
  nmtb.tbButton.dwData = btnPtr->dwData;
  nmtb.tbButton.iString = btnPtr->iString;
  TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_DELETINGBUTTON);

  TOOLBAR_TooltipDelTool(infoPtr, &infoPtr->buttons[nIndex]);

  if (infoPtr->nNumButtons == 1) {
    TRACE(" simple delete!\n");
    Free (infoPtr->buttons);
    infoPtr->buttons = NULL;
    infoPtr->nNumButtons = 0;
  }
  else {
    TBUTTON_INFO *oldButtons = infoPtr->buttons;
    TRACE("complex delete! [nIndex=%d]\n", nIndex);

    infoPtr->nNumButtons--;
    infoPtr->buttons = (TBUTTON_INFO*)Alloc (sizeof (TBUTTON_INFO) * infoPtr->nNumButtons);
    if (nIndex > 0) {
      memcpy (&infoPtr->buttons[0], &oldButtons[0],
        nIndex * sizeof(TBUTTON_INFO));
    }

    if (nIndex < infoPtr->nNumButtons) {
      memcpy (&infoPtr->buttons[nIndex], &oldButtons[nIndex+1],
        (infoPtr->nNumButtons - nIndex) * sizeof(TBUTTON_INFO));
    }

    Free (oldButtons);
  }

  TOOLBAR_LayoutToolbar(hwnd);

  gxInvalidateRect (hwnd, NULL, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_EnableButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXDWORD bState;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);

  TRACE("hwnd=%p, btn index=%ld, lParam=0x%08lx\n", hwnd, wParam, lParam);

  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];

  bState = btnPtr->fsState & TBSTATE_ENABLED;

  /* update the toolbar button state */
  if(GXLOWORD(lParam) == FALSE) {
    btnPtr->fsState &= ~(TBSTATE_ENABLED | TBSTATE_PRESSED);
  } else {
    btnPtr->fsState |= TBSTATE_ENABLED;
  }

  /* redraw the button only if the state of the button changed */
  if(bState != (btnPtr->fsState & TBSTATE_ENABLED))
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


static inline GXLRESULT
TOOLBAR_GetAnchorHighlight (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return infoPtr->bAnchor;
}


static GXLRESULT
TOOLBAR_GetBitmap (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return infoPtr->buttons[nIndex].iBitmap;
}


static inline GXLRESULT
TOOLBAR_GetBitmapFlags ()
{
  return (gxGetDeviceCaps (0, GXLOGPIXELSX) >= 120) ? TBBF_LARGE : 0;
}


static GXLRESULT
TOOLBAR_GetButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBBUTTON lpTbb = (GXLPTBBUTTON)lParam;
  GXINT nIndex = (GXINT)wParam;
  TBUTTON_INFO *btnPtr;

  if (lpTbb == NULL)
    return FALSE;

  if ((nIndex < 0) || (nIndex >= infoPtr->nNumButtons))
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  lpTbb->iBitmap   = btnPtr->iBitmap;
  lpTbb->idCommand = btnPtr->idCommand;
  lpTbb->fsState   = btnPtr->fsState;
  lpTbb->fsStyle   = btnPtr->fsStyle;
#if defined(_WIN32)
  lpTbb->bReserved[0] = 0;
  lpTbb->bReserved[1] = 0;
#endif // #if defined(_WIN32)
  lpTbb->dwData    = btnPtr->dwData;
  lpTbb->iString   = btnPtr->iString;

  return TRUE;
}


static GXLRESULT
TOOLBAR_GetButtonInfoT(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam, GXBOOL bUnicode)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  /* GXTBBUTTONINFOW and GXTBBUTTONINFOA have the same layout*/
  GXLPTBBUTTONINFOW lpTbInfo = (GXLPTBBUTTONINFOW)lParam;
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;

  if (lpTbInfo == NULL)
    return -1;

  /* MSDN documents a iImageLabel field added in Vista but it is not present in
  * the headers and tests shows that even with comctl 6 Vista accepts only the
  * original TBBUTTONINFO size
  */
  if (lpTbInfo->cbSize != sizeof(GXTBBUTTONINFOW))
  {
    WARN("Invalid button size\n");
    return -1;
  }

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam,
    lpTbInfo->dwMask & 0x80000000);
  if (nIndex == -1)
    return -1;

  if (!(btnPtr = &infoPtr->buttons[nIndex])) return -1;

  if (lpTbInfo->dwMask & GXTBIF_COMMAND)
    lpTbInfo->idCommand = btnPtr->idCommand;
  if (lpTbInfo->dwMask & GXTBIF_IMAGE)
    lpTbInfo->iImage = btnPtr->iBitmap;
  if (lpTbInfo->dwMask & GXTBIF_LPARAM)
    lpTbInfo->lParam = btnPtr->dwData;
  if (lpTbInfo->dwMask & GXTBIF_SIZE)
    lpTbInfo->cx = (GXWORD)(btnPtr->rect.right - btnPtr->rect.left);
  if (lpTbInfo->dwMask & GXTBIF_STATE)
    lpTbInfo->fsState = btnPtr->fsState;
  if (lpTbInfo->dwMask & GXTBIF_STYLE)
    lpTbInfo->fsStyle = btnPtr->fsStyle;
  if (lpTbInfo->dwMask & GXTBIF_TEXT) {
    /* TB_GETBUTTONINFO doesn't retrieve text from the string list, so we
    can't use TOOLBAR_GetText here */
    if (GXHIWORD(btnPtr->iString) && (btnPtr->iString != -1)) {
      GXLPWSTR lpText = (GXLPWSTR)btnPtr->iString;
      if (bUnicode)
        gxStr_GetPtrW(lpText, lpTbInfo->pszText, lpTbInfo->cchText);
      else
        gxStr_GetPtrWtoA(lpText, (GXLPSTR)lpTbInfo->pszText, lpTbInfo->cchText);
    } else
      lpTbInfo->pszText[0] = '\0';
  }
  return nIndex;
}


static GXLRESULT
TOOLBAR_GetButtonSize (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return GXMAKELONG((GXWORD)infoPtr->nButtonWidth,
    (GXWORD)infoPtr->nButtonHeight);
}


static GXLRESULT
TOOLBAR_GetButtonTextA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;
  GXLPWSTR lpText;

  if (lParam == 0)
    return -1;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  lpText = TOOLBAR_GetText(infoPtr,&infoPtr->buttons[nIndex]);

  return gxWideCharToMultiByte( GXCP_ACP, 0, lpText, -1,
    (GXLPSTR)lParam, 0x7fffffff, NULL, NULL ) - 1;
}


static GXLRESULT
TOOLBAR_GetButtonTextW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;
  GXLPWSTR lpText;
  GXLRESULT ret = 0;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  lpText = TOOLBAR_GetText(infoPtr,&infoPtr->buttons[nIndex]);

  if (lpText)
  {
    ret = GXSTRLEN (lpText);

    if (lParam)
      GXSTRCPY ((GXLPWSTR)lParam, lpText);
  }

  return ret;
}


static GXLRESULT
TOOLBAR_GetDisabledImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("hwnd=%p, wParam=%ld, lParam=0x%lx\n", hwnd, wParam, lParam);
  /* UNDOCUMENTED: wParam is actually the ID of the image list to return */
  return (GXLRESULT)GETDISIMAGELIST(TOOLBAR_GetInfoPtr (hwnd), wParam);
}


static inline GXLRESULT
TOOLBAR_GetExtendedStyle (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("\n");

  return infoPtr->dwExStyle;
}


static GXLRESULT
TOOLBAR_GetHotImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("hwnd=%p, wParam=%ld, lParam=0x%lx\n", hwnd, wParam, lParam);
  /* UNDOCUMENTED: wParam is actually the ID of the image list to return */
  return (GXLRESULT)GETHOTIMAGELIST(TOOLBAR_GetInfoPtr (hwnd), wParam);
}


static GXLRESULT
TOOLBAR_GetHotItem (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  if (!((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf)))
    return -1;

  if (infoPtr->nHotItem < 0)
    return -1;

  return (GXLRESULT)infoPtr->nHotItem;
}


static GXLRESULT
TOOLBAR_GetDefImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TRACE("hwnd=%p, wParam=%ld, lParam=0x%lx\n", hwnd, wParam, lParam);
  /* UNDOCUMENTED: wParam is actually the ID of the image list to return */
  return (GXLRESULT) GETDEFIMAGELIST(TOOLBAR_GetInfoPtr(hwnd), wParam);
}


static GXLRESULT
TOOLBAR_GetInsertMark (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXTBINSERTMARK *lptbim = (GXTBINSERTMARK*)lParam;

  TRACE("hwnd = %p, lptbim = %p\n", hwnd, lptbim);

  *lptbim = infoPtr->tbim;

  return 0;
}


static GXLRESULT
TOOLBAR_GetInsertMarkColor (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("hwnd = %p\n", hwnd);

  return (GXLRESULT)infoPtr->clrInsertMark;
}


static GXLRESULT
TOOLBAR_GetItemRect (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  LPGXRECT     lpRect;
  GXINT        nIndex;

  nIndex = (GXINT)wParam;
  btnPtr = &infoPtr->buttons[nIndex];
  if ((nIndex < 0) || (nIndex >= infoPtr->nNumButtons))
    return FALSE;
  lpRect = (LPGXRECT)lParam;
  if (lpRect == NULL)
    return FALSE;
  if (btnPtr->fsState & TBSTATE_HIDDEN)
    return FALSE;

  lpRect->left   = btnPtr->rect.left;
  lpRect->right  = btnPtr->rect.right;
  lpRect->bottom = btnPtr->rect.bottom;
  lpRect->top    = btnPtr->rect.top;

  return TRUE;
}


static GXLRESULT
TOOLBAR_GetMaxSize (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPSIZE lpSize = (GXLPSIZE)lParam;

  if (lpSize == NULL)
    return FALSE;

  lpSize->cx = infoPtr->rcBound.right - infoPtr->rcBound.left;
  lpSize->cy = infoPtr->rcBound.bottom - infoPtr->rcBound.top;

  TRACE("maximum size %d x %d\n",
    infoPtr->rcBound.right - infoPtr->rcBound.left,
    infoPtr->rcBound.bottom - infoPtr->rcBound.top);

  return TRUE;
}


/* << TOOLBAR_GetObject >> */


static GXLRESULT
TOOLBAR_GetPadding (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD oldPad;

  oldPad = GXMAKELONG(infoPtr->szPadding.cx, infoPtr->szPadding.cy);
  return (GXLRESULT) oldPad;
}


static GXLRESULT
TOOLBAR_GetRect (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  LPGXRECT     lpRect;
  GXINT        nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  btnPtr = &infoPtr->buttons[nIndex];
  if ((nIndex < 0) || (nIndex >= infoPtr->nNumButtons))
    return FALSE;
  lpRect = (LPGXRECT)lParam;
  if (lpRect == NULL)
    return FALSE;

  lpRect->left   = btnPtr->rect.left;
  lpRect->right  = btnPtr->rect.right;
  lpRect->bottom = btnPtr->rect.bottom;
  lpRect->top    = btnPtr->rect.top;

  return TRUE;
}


static GXLRESULT
TOOLBAR_GetRows (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return infoPtr->nRows;
}


static GXLRESULT
TOOLBAR_GetState (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return infoPtr->buttons[nIndex].fsState;
}


static GXLRESULT
TOOLBAR_GetStyle (GXHWND hwnd)
{
  return gxGetWindowLongW(hwnd, GXGWL_STYLE);
}


static GXLRESULT
TOOLBAR_GetTextRows (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return infoPtr->nMaxTextRows;
}


static GXLRESULT
TOOLBAR_GetToolTips (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  if ((infoPtr->dwStyle & TBSTYLE_TOOLTIPS) && (infoPtr->hwndToolTip == NULL))
    TOOLBAR_TooltipCreateControl(infoPtr);
  return (GXLRESULT)infoPtr->hwndToolTip;
}


static GXLRESULT
TOOLBAR_GetUnicodeFormat (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("%s hwnd=%p\n",
    infoPtr->bUnicode ? "TRUE" : "FALSE", hwnd);

  return infoPtr->bUnicode;
}


static inline GXLRESULT
TOOLBAR_GetVersion (GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  return infoPtr->iVersion;
}


static GXLRESULT
TOOLBAR_HideButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;

  TRACE("\n");

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  if (GXLOWORD(lParam) == FALSE)
    btnPtr->fsState &= ~TBSTATE_HIDDEN;
  else
    btnPtr->fsState |= TBSTATE_HIDDEN;

  TOOLBAR_LayoutToolbar (hwnd);

  gxInvalidateRect (hwnd, NULL, TRUE);

  return TRUE;
}


static inline GXLRESULT
TOOLBAR_HitTest (GXHWND hwnd, GXLPARAM lParam)
{
  return TOOLBAR_InternalHitTest (hwnd, (LPGXPOINT)lParam);
}


static GXLRESULT
TOOLBAR_Indeterminate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXDWORD oldState;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  oldState = btnPtr->fsState;
  if (GXLOWORD(lParam) == FALSE)
    btnPtr->fsState &= ~TBSTATE_INDETERMINATE;
  else
    btnPtr->fsState |= TBSTATE_INDETERMINATE;

  if(oldState != btnPtr->fsState)
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_InsertButtonT(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam, GXBOOL fUnicode)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBBUTTON lpTbb = (GXLPTBBUTTON)lParam;
  GXINT nIndex = (GXINT)wParam;

  if (lpTbb == NULL)
    return FALSE;

  if (nIndex == -1) {
    /* EPP: this seems to be an undocumented call (from my IE4)
    * I assume in that case that:
    * - index of insertion is at the end of existing buttons
    * I only see this happen with nIndex == -1, but it could have a special
    * meaning (like -nIndex (or ~nIndex) to get the real position of insertion).
    */
    nIndex = infoPtr->nNumButtons;

  } else if (nIndex < 0)
    return FALSE;

  TRACE("inserting button index=%d\n", nIndex);
  if (nIndex > infoPtr->nNumButtons) {
    nIndex = infoPtr->nNumButtons;
    TRACE("adjust index=%d\n", nIndex);
  }

  return TOOLBAR_InternalInsertButtonsT(infoPtr, nIndex, 1, lpTbb, fUnicode);
}

/* << TOOLBAR_InsertMarkHitTest >> */


static GXLRESULT
TOOLBAR_IsButtonChecked (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_CHECKED);
}


static GXLRESULT
TOOLBAR_IsButtonEnabled (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_ENABLED);
}


static GXLRESULT
TOOLBAR_IsButtonHidden (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_HIDDEN);
}


static GXLRESULT
TOOLBAR_IsButtonHighlighted (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_MARKED);
}


static GXLRESULT
TOOLBAR_IsButtonIndeterminate (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_INDETERMINATE);
}


static GXLRESULT
TOOLBAR_IsButtonPressed (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return -1;

  return (infoPtr->buttons[nIndex].fsState & TBSTATE_PRESSED);
}


static GXLRESULT
TOOLBAR_LoadImages (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXTBADDBITMAP tbab;
  tbab.hInst = (GXHINSTANCE)lParam;
  tbab.nID = wParam;

  TRACE("hwnd = %p, hInst = %p, nID = %lu\n", hwnd, tbab.hInst, tbab.nID);

  return TOOLBAR_AddBitmap(hwnd, 0, (GXLPARAM)&tbab);
}


static GXLRESULT
TOOLBAR_MapAccelerator (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXWCHAR wAccel = (GXWCHAR)wParam;
  GXUINT* pIDButton = (GXUINT*)lParam;
  GXWCHAR wszAccel[] = {'&',wAccel,0};
  GXINT i;

  TRACE("hwnd = %p, wAccel = %x(%s), pIDButton = %p\n",
    hwnd, wAccel, debugstr_wn(&wAccel,1), pIDButton);

  for (i = 0; i < infoPtr->nNumButtons; i++)
  {
    TBUTTON_INFO *btnPtr = infoPtr->buttons+i;
    if (!(btnPtr->fsStyle & BTNS_NOPREFIX) &&
      !(btnPtr->fsState & TBSTATE_HIDDEN))
    {
      GXINT iLen = (GXINT)GXSTRLEN(wszAccel);
      GXLPCWSTR lpszStr = TOOLBAR_GetText(infoPtr, btnPtr);

      if (!lpszStr)
        continue;

      while (*lpszStr)
      {
        if ((lpszStr[0] == '&') && (lpszStr[1] == '&'))
        {
          lpszStr += 2;
          continue;
        }
        if (!strncmpiW(lpszStr, wszAccel, iLen))
        {
          *pIDButton = btnPtr->idCommand;
          return TRUE;
        }
        lpszStr++;
      }
    }
  }
  return FALSE;
}


static GXLRESULT
TOOLBAR_MarkButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;
  GXDWORD oldState;
  TBUTTON_INFO *btnPtr;

  TRACE("hwnd = %p, wParam = %ld, lParam = 0x%08lx\n", hwnd, wParam, lParam);

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  oldState = btnPtr->fsState;

  if (GXLOWORD(lParam))
    btnPtr->fsState |= TBSTATE_MARKED;
  else
    btnPtr->fsState &= ~TBSTATE_MARKED;

  if(oldState != btnPtr->fsState)
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


/* fixes up an index of a button affected by a move */
static inline void TOOLBAR_MoveFixupIndex(GXINT* pIndex, GXINT nIndex, GXINT nMoveIndex, GXBOOL bMoveUp)
{
  if (bMoveUp)
  {
    if (*pIndex > nIndex && *pIndex <= nMoveIndex)
      (*pIndex)--;
    else if (*pIndex == nIndex)
      *pIndex = nMoveIndex;
  }
  else
  {
    if (*pIndex >= nMoveIndex && *pIndex < nIndex)
      (*pIndex)++;
    else if (*pIndex == nIndex)
      *pIndex = nMoveIndex;
  }
}


static GXLRESULT
TOOLBAR_MoveButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex;
  GXINT nCount;
  GXINT nMoveIndex = (GXINT)lParam;
  TBUTTON_INFO button;

  TRACE("hwnd=%p, wParam=%ld, lParam=%ld\n", hwnd, wParam, lParam);

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, TRUE);
  if ((nIndex == -1) || (nMoveIndex < 0))
    return FALSE;

  if (nMoveIndex > infoPtr->nNumButtons - 1)
    nMoveIndex = infoPtr->nNumButtons - 1;

  button = infoPtr->buttons[nIndex];

  /* move button right */
  if (nIndex < nMoveIndex)
  {
    nCount = nMoveIndex - nIndex;
    memmove(&infoPtr->buttons[nIndex], &infoPtr->buttons[nIndex+1], nCount*sizeof(TBUTTON_INFO));
    infoPtr->buttons[nMoveIndex] = button;

    TOOLBAR_MoveFixupIndex(&infoPtr->nButtonDown, nIndex, nMoveIndex, TRUE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nButtonDrag, nIndex, nMoveIndex, TRUE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nOldHit, nIndex, nMoveIndex, TRUE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nHotItem, nIndex, nMoveIndex, TRUE);
  }
  else if (nIndex > nMoveIndex) /* move button left */
  {
    nCount = nIndex - nMoveIndex;
    memmove(&infoPtr->buttons[nMoveIndex+1], &infoPtr->buttons[nMoveIndex], nCount*sizeof(TBUTTON_INFO));
    infoPtr->buttons[nMoveIndex] = button;

    TOOLBAR_MoveFixupIndex(&infoPtr->nButtonDown, nIndex, nMoveIndex, FALSE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nButtonDrag, nIndex, nMoveIndex, FALSE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nOldHit, nIndex, nMoveIndex, FALSE);
    TOOLBAR_MoveFixupIndex(&infoPtr->nHotItem, nIndex, nMoveIndex, FALSE);
  }

  TOOLBAR_LayoutToolbar(hwnd);
  TOOLBAR_AutoSize(hwnd);
  gxInvalidateRect(hwnd, NULL, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_PressButton (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXDWORD oldState;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  oldState = btnPtr->fsState;
  if (GXLOWORD(lParam) == FALSE)
    btnPtr->fsState &= ~TBSTATE_PRESSED;
  else
    btnPtr->fsState |= TBSTATE_PRESSED;

  if(oldState != btnPtr->fsState)
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}

/* FIXME: there might still be some confusion her between number of buttons
* and number of bitmaps */
static GXLRESULT
TOOLBAR_ReplaceBitmap (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBREPLACEBITMAP lpReplace = (GXLPTBREPLACEBITMAP) lParam;
  GXHBITMAP hBitmap;
  GXINT i = 0, nOldButtons = 0, pos = 0;
  GXINT nOldBitmaps, nNewBitmaps = 0;
  GXHIMAGELIST himlDef = 0;

  TRACE("hInstOld %p nIDOld %lx hInstNew %p nIDNew %lx nButtons %x\n",
    lpReplace->hInstOld, lpReplace->nIDOld, lpReplace->hInstNew, lpReplace->nIDNew,
    lpReplace->nButtons);

  if (lpReplace->hInstOld == GXHINST_COMMCTRL)
  {
    FIXME("changing standard bitmaps not implemented\n");
    return FALSE;
  }
  else if (lpReplace->hInstOld != 0)
    FIXME("resources not in the current module not implemented\n");

  TRACE("To be replaced hInstOld %p nIDOld %lx\n", lpReplace->hInstOld, lpReplace->nIDOld);
  for (i = 0; i < infoPtr->nNumBitmapInfos; i++) {
    TBITMAP_INFO *tbi = &infoPtr->bitmaps[i];
    TRACE("tbimapinfo %d hInstOld %p nIDOld %x\n", i, tbi->hInst, tbi->nID);
    if (tbi->hInst == lpReplace->hInstOld && tbi->nID == lpReplace->nIDOld)
    {
      TRACE("Found: nButtons %d hInst %p nID %x\n", tbi->nButtons, tbi->hInst, tbi->nID);
      nOldButtons = tbi->nButtons;
      tbi->nButtons = lpReplace->nButtons;
      tbi->hInst = lpReplace->hInstNew;
      tbi->nID = lpReplace->nIDNew;
      TRACE("tbimapinfo changed %d hInstOld %p nIDOld %x\n", i, tbi->hInst, tbi->nID);
      break;
    }
    pos += (GXINT)tbi->nButtons;
  }

  if (nOldButtons == 0)
  {
    WARN("No hinst/bitmap found! hInst %p nID %lx\n", lpReplace->hInstOld, lpReplace->nIDOld);
    return FALSE;
  }

  /* copy the bitmap before adding it as gxImageList_AddMasked modifies the
  * bitmap
  */
  if (lpReplace->hInstNew)
    hBitmap = gxLoadBitmapW(lpReplace->hInstNew,(GXLPWSTR)lpReplace->nIDNew);
  else
    hBitmap = (GXHBITMAP)gxCopyImage((GXHBITMAP)lpReplace->nIDNew, GXIMAGE_BITMAP, 0, 0, 0);

  himlDef = GETDEFIMAGELIST(infoPtr, 0); /* fixme: correct? */
  nOldBitmaps = gxImageList_GetImageCount(himlDef);

  /* gxImageList_Replace(GETDEFIMAGELIST(), pos, hBitmap, NULL); */

  for (i = pos + nOldBitmaps - 1; i >= pos; i--)
    gxImageList_Remove(himlDef, i);

  if (hBitmap)
  {
    gxImageList_AddMasked (himlDef, hBitmap, comctl32_color.clrBtnFace);
    nNewBitmaps = gxImageList_GetImageCount(himlDef);
    gxDeleteObject(hBitmap);
  }

  infoPtr->nNumBitmaps = infoPtr->nNumBitmaps - nOldBitmaps + nNewBitmaps;

  TRACE(" pos %d  %d old bitmaps replaced by %d new ones.\n",
    pos, nOldBitmaps, nNewBitmaps);

  gxInvalidateRect(hwnd, NULL, TRUE);
  return TRUE;
}


/* helper for TOOLBAR_SaveRestoreW */
static GXBOOL
TOOLBAR_Save(const GXTBSAVEPARAMSW *lpSave)
{
  FIXME("save to %s %s\n", debugstr_w(lpSave->pszSubKey),
    debugstr_w(lpSave->pszValueName));

  return FALSE;
}


/* helper for TOOLBAR_Restore */
static void
TOOLBAR_DeleteAllButtons(TOOLBAR_INFO *infoPtr)
{
  GXINT i;

  for (i = 0; i < infoPtr->nNumButtons; i++)
  {
    TOOLBAR_TooltipDelTool(infoPtr, &infoPtr->buttons[i]);
  }

  Free(infoPtr->buttons);
  infoPtr->buttons = NULL;
  infoPtr->nNumButtons = 0;
}


/* helper for TOOLBAR_SaveRestoreW */
static GXBOOL
TOOLBAR_Restore(TOOLBAR_INFO *infoPtr, const GXTBSAVEPARAMSW *lpSave)
{
  //GXLONG res;
  GXHKEY hkey = NULL;
  GXBOOL ret = FALSE;
  //GXDWORD dwType;
  GXDWORD dwSize = 0;
  //GXNMTBRESTORE nmtbr;

  ///* restore toolbar information */
  //TRACE("restore from %s %s\n", debugstr_w(lpSave->pszSubKey),
  //  debugstr_w(lpSave->pszValueName));

  //memset(&nmtbr, 0, sizeof(nmtbr));

  //res = RegOpenKeyExW((HKEY)lpSave->hkr, lpSave->pszSubKey, 0,
  //  KEY_QUERY_VALUE, (PHKEY)&hkey);
  //if (!res)
  //  res = RegQueryValueExW((HKEY)hkey, lpSave->pszValueName, NULL, &dwType,
  //  NULL, &dwSize);
  //if (!res && dwType != REG_BINARY)
  //  res = ERROR_FILE_NOT_FOUND;
  //if (!res)
  //{
  //  nmtbr.pData = (GXDWORD*)Alloc(dwSize);
  //  nmtbr.cbData = dwSize;
  //  if (!nmtbr.pData) res = ERROR_OUTOFMEMORY;
  //}
  //if (!res)
  //  res = RegQueryValueExW((HKEY)hkey, lpSave->pszValueName, NULL, &dwType,
  //  (GXLPBYTE)nmtbr.pData, &dwSize);
  //if (!res)
  //{
  //  nmtbr.pCurrent = nmtbr.pData;
  //  nmtbr.iItem = -1;
  //  nmtbr.cbBytesPerRecord = sizeof(GXDWORD);
  //  nmtbr.cButtons = nmtbr.cbData / nmtbr.cbBytesPerRecord;

  //  if (!TOOLBAR_SendNotify(&nmtbr.hdr, infoPtr, GXTBN_RESTORE))
  //  {
  //    GXINT i;

  //    /* remove all existing buttons as this function is designed to
  //    * restore the toolbar to a previously saved state */
  //    TOOLBAR_DeleteAllButtons(infoPtr);

  //    for (i = 0; i < nmtbr.cButtons; i++)
  //    {
  //      nmtbr.iItem = i;
  //      nmtbr.tbButton.iBitmap = -1;
  //      nmtbr.tbButton.fsState = 0;
  //      nmtbr.tbButton.fsStyle = 0;
  //      nmtbr.tbButton.idCommand = 0;
  //      if (*nmtbr.pCurrent == (GXDWORD)-1)
  //      {
  //        /* separator */
  //        nmtbr.tbButton.fsStyle = TBSTYLE_SEP;
  //        nmtbr.tbButton.iBitmap = SEPARATOR_WIDTH;
  //      }
  //      else if (*nmtbr.pCurrent == (GXDWORD)-2)
  //        /* hidden button */
  //        nmtbr.tbButton.fsState = TBSTATE_HIDDEN;
  //      else
  //        nmtbr.tbButton.idCommand = (GXINT)*nmtbr.pCurrent;

  //      nmtbr.pCurrent++;

  //      TOOLBAR_SendNotify(&nmtbr.hdr, infoPtr, GXTBN_RESTORE);

  //      /* can't contain real string as we don't know whether
  //      * the client put an ANSI or Unicode string in there */
  //      if (GXHIWORD(nmtbr.tbButton.iString))
  //        nmtbr.tbButton.iString = 0;

  //      TOOLBAR_InsertButtonT(infoPtr->hwndSelf, -1,
  //        (GXLPARAM)&nmtbr.tbButton, TRUE);
  //    }

  //    /* do legacy notifications */
  //    if (infoPtr->iVersion < 5)
  //    {
  //      /* FIXME: send GXTBN_BEGINADJUST */
  //      FIXME("send GXTBN_GETBUTTONINFO for each button\n");
  //      /* FIXME: send GXTBN_ENDADJUST */
  //    }

  //    /* remove all uninitialised buttons
  //    * note: loop backwards to avoid having to fixup i on a
  //    * delete */
  //    for (i = infoPtr->nNumButtons - 1; i >= 0; i--)
  //      if (infoPtr->buttons[i].iBitmap == -1)
  //        TOOLBAR_DeleteButton(infoPtr->hwndSelf, i);

  //    /* only indicate success if at least one button survived */
  //    if (infoPtr->nNumButtons > 0) ret = TRUE;
  //  }
  //}
  //Free (nmtbr.pData);
  //RegCloseKey((HKEY)hkey);

  return ret;
}


static GXLRESULT
TOOLBAR_SaveRestoreW (GXHWND hwnd, GXWPARAM wParam, const GXTBSAVEPARAMSW *lpSave)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  if (lpSave == NULL) return 0;

  if (wParam)
    return TOOLBAR_Save(lpSave);
  else
    return TOOLBAR_Restore(infoPtr, lpSave);
}


static GXLRESULT
TOOLBAR_SaveRestoreA (GXHWND hwnd, GXWPARAM wParam, const GXTBSAVEPARAMSA *lpSave)
{
  GXLPWSTR pszValueName = 0, pszSubKey = 0;
  GXTBSAVEPARAMSW SaveW;
  GXLRESULT result = 0;
  GXINT len;

  if (lpSave == NULL) return 0;

  len = gxMultiByteToWideChar(GXCP_ACP, 0, lpSave->pszSubKey, -1, NULL, 0);
  pszSubKey = (GXWCHAR*)Alloc(len * sizeof(GXWCHAR));
  if (pszSubKey) goto exit;
  gxMultiByteToWideChar(GXCP_ACP, 0, lpSave->pszSubKey, -1, pszSubKey, len);

  len = gxMultiByteToWideChar(GXCP_ACP, 0, lpSave->pszValueName, -1, NULL, 0);
  pszValueName = (GXWCHAR*)Alloc(len * sizeof(GXWCHAR));
  if (!pszValueName) goto exit;
  gxMultiByteToWideChar(GXCP_ACP, 0, lpSave->pszValueName, -1, pszValueName, len);

  SaveW.pszValueName = pszValueName;
  SaveW.pszSubKey = pszSubKey;
  SaveW.hkr = lpSave->hkr;
  result = TOOLBAR_SaveRestoreW(hwnd, wParam, &SaveW);

exit:
  Free (pszValueName);
  Free (pszSubKey);

  return result;
}


static GXLRESULT
TOOLBAR_SetAnchorHighlight (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXBOOL bOldAnchor = infoPtr->bAnchor;

  TRACE("hwnd=%p, bAnchor = %s\n", hwnd, wParam ? "TRUE" : "FALSE");

  infoPtr->bAnchor = (GXBOOL)wParam;

  /* Native does not remove the hot effect from an already hot button */

  return (GXLRESULT)bOldAnchor;
}


static GXLRESULT
TOOLBAR_SetBitmapSize (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXHIMAGELIST himlDef = GETDEFIMAGELIST(infoPtr, 0);
  short width = (short)GXLOWORD(lParam);
  short height = (short)GXHIWORD(lParam);

  TRACE("hwnd=%p, wParam=%ld, lParam=%ld\n", hwnd, wParam, lParam);

  if (wParam != 0)
    FIXME("wParam is %ld. Perhaps image list index?\n", wParam);

  /* 0 width or height is changed to 1 */
  if (width == 0)
    width = 1;
  if (height == 0)
    height = 1;

  if (infoPtr->nNumButtons > 0)
    TRACE("%d buttons, undoc change to bitmap size : %d-%d -> %d-%d\n",
    infoPtr->nNumButtons,
    infoPtr->nBitmapWidth, infoPtr->nBitmapHeight, width, height);

  if (width < -1 || height < -1)
  {
    /* Windows destroys the imagelist and seems to actually use negative
    * values to compute button sizes */
    FIXME("Negative bitmap sizes not supported (%d, %d)\n", width, height);
    return FALSE;
  }

  /* width or height of -1 means no change */
  if (width != -1)
    infoPtr->nBitmapWidth = width;
  if (height != -1)
    infoPtr->nBitmapHeight = height;

  if ((himlDef == infoPtr->himlInt) &&
    (gxImageList_GetImageCount(infoPtr->himlInt) == 0))
  {
    gxImageList_SetIconSize(infoPtr->himlInt, infoPtr->nBitmapWidth,
      infoPtr->nBitmapHeight);
  }

  TOOLBAR_CalcToolbar(hwnd);
  gxInvalidateRect(infoPtr->hwndSelf, NULL, FALSE);
  return TRUE;
}


static GXLRESULT
TOOLBAR_SetButtonInfoA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBBUTTONINFOA lptbbi = (GXLPTBBUTTONINFOA)lParam;
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXRECT oldBtnRect;

  if (lptbbi == NULL)
    return FALSE;
  if (lptbbi->cbSize < sizeof(GXTBBUTTONINFOA))
    return FALSE;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam,
    lptbbi->dwMask & 0x80000000);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  if (lptbbi->dwMask & GXTBIF_COMMAND)
    btnPtr->idCommand = lptbbi->idCommand;
  if (lptbbi->dwMask & GXTBIF_IMAGE)
    btnPtr->iBitmap = lptbbi->iImage;
  if (lptbbi->dwMask & GXTBIF_LPARAM)
    btnPtr->dwData = lptbbi->lParam;
  if (lptbbi->dwMask & GXTBIF_SIZE)
    btnPtr->cx = lptbbi->cx;
  if (lptbbi->dwMask & GXTBIF_STATE)
    btnPtr->fsState = lptbbi->fsState;
  if (lptbbi->dwMask & GXTBIF_STYLE)
    btnPtr->fsStyle = lptbbi->fsStyle;

  if ((lptbbi->dwMask & GXTBIF_TEXT) && ((GXINT_PTR)lptbbi->pszText != -1)) {
    if ((GXHIWORD(btnPtr->iString) == 0) || (btnPtr->iString == -1))
      /* iString is index, zero it to make Str_SetPtr succeed */
      btnPtr->iString=0;

    gxStr_SetPtrAtoW ((GXLPWSTR *)&btnPtr->iString, lptbbi->pszText);
  }

  /* save the button rect to see if we need to redraw the whole toolbar */
  oldBtnRect = btnPtr->rect;
  TOOLBAR_LayoutToolbar(hwnd);

  if (!gxEqualRect(&oldBtnRect, &btnPtr->rect))
    gxInvalidateRect(hwnd, NULL, TRUE);
  else
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetButtonInfoW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPTBBUTTONINFOW lptbbi = (GXLPTBBUTTONINFOW)lParam;
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;
  GXRECT oldBtnRect;

  if (lptbbi == NULL)
    return FALSE;
  if (lptbbi->cbSize < sizeof(GXTBBUTTONINFOW))
    return FALSE;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam,
    lptbbi->dwMask & 0x80000000);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];
  if (lptbbi->dwMask & GXTBIF_COMMAND)
    btnPtr->idCommand = lptbbi->idCommand;
  if (lptbbi->dwMask & GXTBIF_IMAGE)
    btnPtr->iBitmap = lptbbi->iImage;
  if (lptbbi->dwMask & GXTBIF_LPARAM)
    btnPtr->dwData = lptbbi->lParam;
  if (lptbbi->dwMask & GXTBIF_SIZE)
    btnPtr->cx = lptbbi->cx;
  if (lptbbi->dwMask & GXTBIF_STATE)
    btnPtr->fsState = lptbbi->fsState;
  if (lptbbi->dwMask & GXTBIF_STYLE)
    btnPtr->fsStyle = lptbbi->fsStyle;

  if ((lptbbi->dwMask & GXTBIF_TEXT) && ((GXINT_PTR)lptbbi->pszText != -1)) {
    if ((GXHIWORD(btnPtr->iString) == 0) || (btnPtr->iString == -1))
      /* iString is index, zero it to make Str_SetPtr succeed */
      btnPtr->iString=0;
    gxStr_SetPtrW ((GXLPWSTR *)&btnPtr->iString, lptbbi->pszText);
  }

  /* save the button rect to see if we need to redraw the whole toolbar */
  oldBtnRect = btnPtr->rect;
  TOOLBAR_LayoutToolbar(hwnd);

  if (!gxEqualRect(&oldBtnRect, &btnPtr->rect))
    gxInvalidateRect(hwnd, NULL, TRUE);
  else
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetButtonSize (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT cx = (short)GXLOWORD(lParam), cy = (short)GXHIWORD(lParam);

  if ((cx < 0) || (cy < 0))
  {
    ERR("invalid parameter 0x%08x\n", (GXDWORD)lParam);
    return FALSE;
  }

  TRACE("%p, cx = %d, cy = %d\n", hwnd, cx, cy);

  /* The documentation claims you can only change the button size before
  * any button has been added. But this is wrong.
  * WINZIP32.EXE (ver 8) calls this on one of its buttons after adding
  * it to the toolbar, and it checks that the return value is nonzero - mjm
  * Further testing shows that we must actually perform the change too.
  */
  /*
  * The documentation also does not mention that if 0 is supplied for
  * either size, the system changes it to the default of 24 wide and
  * 22 high. Demonstarted in ControlSpy Toolbar. GLA 3/02
  */
  if (cx == 0) cx = 24;
  if (cy == 0) cx = 22;

  cx = max(cx, infoPtr->szPadding.cx + infoPtr->nBitmapWidth);
  cy = max(cy, infoPtr->szPadding.cy + infoPtr->nBitmapHeight);

  infoPtr->nButtonWidth = cx;
  infoPtr->nButtonHeight = cy;

  infoPtr->iTopMargin = default_top_margin(infoPtr);
  TOOLBAR_LayoutToolbar(hwnd);
  return TRUE;
}


static GXLRESULT
TOOLBAR_SetButtonWidth (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  /* if setting to current values, ignore */
  if ((infoPtr->cxMin == (short)GXLOWORD(lParam)) &&
    (infoPtr->cxMax == (short)GXHIWORD(lParam))) {
      TRACE("matches current width, min=%d, max=%d, no recalc\n",
        infoPtr->cxMin, infoPtr->cxMax);
      return TRUE;
  }

  /* save new values */
  infoPtr->cxMin = (short)GXLOWORD(lParam);
  infoPtr->cxMax = (short)GXHIWORD(lParam);

  /* otherwise we need to recalc the toolbar and in some cases
  recalc the bounding rectangle (does DrawText w/ GXDT_CALCRECT
  which doesn't actually draw - GA). */
  TRACE("number of buttons %d, cx=%d, cy=%d, recalcing\n",
    infoPtr->nNumButtons, infoPtr->cxMin, infoPtr->cxMax);

  TOOLBAR_CalcToolbar (hwnd);

  gxInvalidateRect (hwnd, NULL, TRUE);

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetCmdId (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nIndex = (GXINT)wParam;

  if ((nIndex < 0) || (nIndex >= infoPtr->nNumButtons))
    return FALSE;

  infoPtr->buttons[nIndex].idCommand = (GXINT)lParam;

  if (infoPtr->hwndToolTip) {

    FIXME("change tool tip!\n");

  }

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetDisabledImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXHIMAGELIST himl = (GXHIMAGELIST)lParam;
  GXHIMAGELIST himlTemp;
  GXINT id = 0;

  if (infoPtr->iVersion >= 5)
    id = wParam;

  himlTemp = TOOLBAR_InsertImageList(&infoPtr->himlDis, 
    &infoPtr->cimlDis, himl, id);

  /* FIXME: redraw ? */

  return (GXLRESULT)himlTemp;
}


static GXLRESULT
TOOLBAR_SetDrawTextFlags (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD dwTemp;

  TRACE("hwnd = %p, dwMask = 0x%08x, dwDTFlags = 0x%08x\n", hwnd, (GXDWORD)wParam, (GXDWORD)lParam);

  dwTemp = infoPtr->dwDTFlags;
  infoPtr->dwDTFlags =
    (infoPtr->dwDTFlags & (GXDWORD)wParam) | (GXDWORD)lParam;

  return (GXLRESULT)dwTemp;
}

/* This function differs a bit from what MSDN says it does:
* 1. lParam contains extended style flags to OR with current style
*  (MSDN isn't clear on the OR bit)
* 2. wParam appears to contain extended style flags to be reset
*  (MSDN says that this parameter is reserved)
*/
static GXLRESULT
TOOLBAR_SetExtendedStyle (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD dwOldStyle;

  dwOldStyle = infoPtr->dwExStyle;
  infoPtr->dwExStyle = (GXDWORD)lParam;

  TRACE("new style 0x%08x\n", infoPtr->dwExStyle);

  if (infoPtr->dwExStyle & ~TBSTYLE_EX_ALL)
    FIXME("Unknown Toolbar Extended Style 0x%08x. Please report.\n",
    (infoPtr->dwExStyle & ~TBSTYLE_EX_ALL));

  if ((dwOldStyle ^ infoPtr->dwExStyle) & TBSTYLE_EX_MIXEDBUTTONS)
    TOOLBAR_CalcToolbar(hwnd);
  else
    TOOLBAR_LayoutToolbar(hwnd);

  TOOLBAR_AutoSize(hwnd);
  gxInvalidateRect(hwnd, NULL, TRUE);

  return (GXLRESULT)dwOldStyle;
}


static GXLRESULT
TOOLBAR_SetHotImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXHIMAGELIST himlTemp;
  GXHIMAGELIST himl = (GXHIMAGELIST)lParam;
  GXINT id = 0;

  if (infoPtr->iVersion >= 5)
    id = wParam;

  TRACE("hwnd = %p, himl = %p, id = %d\n", hwnd, himl, id);

  himlTemp = TOOLBAR_InsertImageList(&infoPtr->himlHot, 
    &infoPtr->cimlHot, himl, id);

  /* FIXME: redraw ? */

  return (GXLRESULT)himlTemp;
}


/* Makes previous hot button no longer hot, makes the specified
* button hot and sends appropriate notifications. dwReason is one or
* more HICF_ flags. Specify nHit < 0 to make no buttons hot.
* NOTE 1: this function does not validate nHit
* NOTE 2: the name of this function is completely made up and
* not based on any documentation from Microsoft. */
static void
TOOLBAR_SetHotItemEx (TOOLBAR_INFO *infoPtr, GXINT nHit, GXDWORD dwReason)
{
  if (infoPtr->nHotItem != nHit)
  {
    GXNMTBHOTITEM nmhotitem;
    TBUTTON_INFO *btnPtr = NULL, *oldBtnPtr = NULL;

    nmhotitem.dwFlags = dwReason;
    if(infoPtr->nHotItem >= 0)
    {
      oldBtnPtr = &infoPtr->buttons[infoPtr->nHotItem];
      nmhotitem.idOld = oldBtnPtr->idCommand;
    }
    else
    {
      nmhotitem.dwFlags |= HICF_ENTERING;
      nmhotitem.idOld = 0;
    }

    if (nHit >= 0)
    {
      btnPtr = &infoPtr->buttons[nHit];
      nmhotitem.idNew = btnPtr->idCommand;
    }
    else
    {
      nmhotitem.dwFlags |= HICF_LEAVING;
      nmhotitem.idNew = 0;
    }

    /* now change the hot and invalidate the old and new buttons - if the
    * parent agrees */
    if (!TOOLBAR_SendNotify(&nmhotitem.hdr, infoPtr, GXTBN_HOTITEMCHANGE))
    {
      if (oldBtnPtr) {
        oldBtnPtr->bHot = FALSE;
        gxInvalidateRect(infoPtr->hwndSelf, &oldBtnPtr->rect, TRUE);
      }
      /* setting disabled buttons as hot fails even if the notify contains the button id */
      if (btnPtr && (btnPtr->fsState & TBSTATE_ENABLED)) {
        btnPtr->bHot = TRUE;
        gxInvalidateRect(infoPtr->hwndSelf, &btnPtr->rect, TRUE);
        infoPtr->nHotItem = nHit;
      }
      else
        infoPtr->nHotItem = -1;            
    }
  }
}

static GXLRESULT
TOOLBAR_SetHotItem (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXINT nOldHotItem = infoPtr->nHotItem;

  TRACE("hwnd = %p, nHit = %d\n", hwnd, (GXINT)wParam);

  if ((GXINT)wParam >= infoPtr->nNumButtons)
    return infoPtr->nHotItem;

  if ((GXINT)wParam < 0)
    wParam = -1;

  /* NOTE: an application can still remove the hot item even if anchor
  * highlighting is enabled */

  TOOLBAR_SetHotItemEx(infoPtr, wParam, HICF_OTHER);

  if (nOldHotItem < 0)
    return -1;

  return (GXLRESULT)nOldHotItem;
}


static GXLRESULT
TOOLBAR_SetImageList (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXHIMAGELIST himlTemp;
  GXHIMAGELIST himl = (GXHIMAGELIST)lParam;
  GXINT oldButtonWidth = infoPtr->nButtonWidth;
  GXINT i, id = 0;

  if (infoPtr->iVersion >= 5)
    id = wParam;

  himlTemp = TOOLBAR_InsertImageList(&infoPtr->himlDef, 
    &infoPtr->cimlDef, himl, id);

  infoPtr->nNumBitmaps = 0;
  for (i = 0; i < infoPtr->cimlDef; i++)
    infoPtr->nNumBitmaps += gxImageList_GetImageCount(infoPtr->himlDef[i]->himl);

  if (!gxImageList_GetIconSize(himl, &infoPtr->nBitmapWidth,
    &infoPtr->nBitmapHeight))
  {
    infoPtr->nBitmapWidth = 1;
    infoPtr->nBitmapHeight = 1;
  }
  TOOLBAR_CalcToolbar(hwnd);
  if (infoPtr->nButtonWidth < oldButtonWidth)
    TOOLBAR_SetButtonSize(hwnd, GXMAKELONG(oldButtonWidth, infoPtr->nButtonHeight));

  TRACE("hwnd %p, new himl=%p, id = %d, count=%d, bitmap w=%d, h=%d\n",
    hwnd, infoPtr->himlDef, id, infoPtr->nNumBitmaps,
    infoPtr->nBitmapWidth, infoPtr->nBitmapHeight);

  gxInvalidateRect(hwnd, NULL, TRUE);

  return (GXLRESULT)himlTemp;
}


static GXLRESULT
TOOLBAR_SetIndent (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  infoPtr->nIndent = (GXINT)wParam;

  TRACE("\n");

  /* process only on indent changing */
  if(infoPtr->nIndent != (GXINT)wParam)
  {
    infoPtr->nIndent = (GXINT)wParam;
    TOOLBAR_CalcToolbar (hwnd);
    gxInvalidateRect(hwnd, NULL, FALSE);
  }

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetInsertMark (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXTBINSERTMARK *lptbim = (GXTBINSERTMARK*)lParam;

  TRACE("hwnd = %p, lptbim = { %d, 0x%08x}\n", hwnd, lptbim->iButton, lptbim->dwFlags);

  if ((lptbim->dwFlags & ~GXTBIMHT_AFTER) != 0)
  {
    FIXME("Unrecognized flag(s): 0x%08x\n", (lptbim->dwFlags & ~GXTBIMHT_AFTER));
    return 0;
  }

  if ((lptbim->iButton == -1) || 
    ((lptbim->iButton < infoPtr->nNumButtons) &&
    (lptbim->iButton >= 0)))
  {
    infoPtr->tbim = *lptbim;
    /* FIXME: don't need to update entire toolbar */
    gxInvalidateRect(hwnd, NULL, TRUE);
  }
  else
    ERR("Invalid button index %d\n", lptbim->iButton);

  return 0;
}


static GXLRESULT
TOOLBAR_SetInsertMarkColor (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  infoPtr->clrInsertMark = (GXCOLORREF)lParam;

  /* FIXME: don't need to update entire toolbar */
  gxInvalidateRect(hwnd, NULL, TRUE);

  return 0;
}


static GXLRESULT
TOOLBAR_SetMaxTextRows (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  infoPtr->nMaxTextRows = (GXINT)wParam;

  TOOLBAR_CalcToolbar(hwnd);
  return TRUE;
}


/* MSDN gives slightly wrong info on padding.
* 1. It is not only used on buttons with the BTNS_AUTOSIZE style
* 2. It is not used to create a blank area between the edge of the button
*    and the text or image if TBSTYLE_LIST is set. It is used to control
*    the gap between the image and text. 
* 3. It is not applied to both sides. If TBSTYLE_LIST is set it is used 
*    to control the bottom and right borders [with the border being
*    szPadding.cx - (gxGetSystemMetrics(SM_CXEDGE)+1)], otherwise the padding
*    is shared evenly on both sides of the button.
* See blueprints in comments above TOOLBAR_MeasureButton for more info.
*/
static GXLRESULT
TOOLBAR_SetPadding (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD  oldPad;

  oldPad = GXMAKELONG(infoPtr->szPadding.cx, infoPtr->szPadding.cy);
  infoPtr->szPadding.cx = min(GXLOWORD((GXDWORD)lParam), gxGetSystemMetrics(GXSM_CXEDGE));
  infoPtr->szPadding.cy = min(GXHIWORD((GXDWORD)lParam), gxGetSystemMetrics(GXSM_CYEDGE));
  TRACE("cx=%d, cy=%d\n",
    infoPtr->szPadding.cx, infoPtr->szPadding.cy);
  return (GXLRESULT) oldPad;
}


static GXLRESULT
TOOLBAR_SetParent (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXHWND hwndOldNotify;

  TRACE("\n");

  hwndOldNotify = infoPtr->hwndNotify;
  infoPtr->hwndNotify = (GXHWND)wParam;

  return (GXLRESULT)hwndOldNotify;
}


static GXLRESULT
TOOLBAR_SetRows (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  LPGXRECT lprc = (LPGXRECT)lParam;
  GXINT rows = GXLOWORD(wParam);
  GXBOOL bLarger = GXHIWORD(wParam);

  TRACE("\n");

  TRACE("Setting rows to %d (%d)\n", rows, bLarger);

  if(infoPtr->nRows != rows)
  {
    TBUTTON_INFO *btnPtr = infoPtr->buttons;
    GXINT curColumn = 0; /* Current column                      */
    GXINT curRow    = 0; /* Current row                         */
    GXINT hidden    = 0; /* Number of hidden buttons */
    GXINT seps      = 0; /* Number of separators     */
    GXINT idealWrap = 0; /* Ideal wrap point         */
    GXINT i;
    GXBOOL wrap;

    /*
    Calculate new size and wrap points - Under windows, setrows will
    change the dimensions if needed to show the number of requested
    rows (if CCS_NORESIZE is set), or will take up the whole window
    (if no CCS_NORESIZE).

    Basic algorithm - If N buttons, and y rows requested, each row
    contains N/y buttons.

    FIXME: Handling of separators not obvious from testing results
    FIXME: Take width of window into account?
    */

    /* Loop through the buttons one by one counting key items  */
    for (i = 0; i < infoPtr->nNumButtons; i++ )
    {
      btnPtr[i].fsState &= ~TBSTATE_WRAP;
      if (btnPtr[i].fsState & TBSTATE_HIDDEN)
        hidden++;
      else if (btnPtr[i].fsStyle & BTNS_SEP)
        seps++;
    }

    /* FIXME: Separators make this quite complex */
    if (seps) FIXME("Separators unhandled\n");

    /* Round up so more per line, i.e., less rows */
    idealWrap = (infoPtr->nNumButtons - hidden + (rows-1)) / rows;

    /* Calculate ideal wrap point if we are allowed to grow, but cannot
    achieve the requested number of rows. */
    if (bLarger && idealWrap > 1)
    {
      GXINT resRows = (infoPtr->nNumButtons + (idealWrap-1)) / idealWrap;
      GXINT moreRows = (infoPtr->nNumButtons + (idealWrap-2)) / (idealWrap-1);

      if (resRows < rows && moreRows > rows)
      {
        idealWrap--;
        TRACE("Changing idealWrap due to bLarger (now %d)\n", idealWrap);
      }
    }

    curColumn = curRow = 0;
    wrap = FALSE;
    TRACE("Trying to wrap at %d (%d,%d,%d)\n", idealWrap,
      infoPtr->nNumButtons, hidden, rows);

    for (i = 0; i < infoPtr->nNumButtons; i++ )
    {
      if (btnPtr[i].fsState & TBSTATE_HIDDEN)
        continue;

      /* Step on, wrap if necessary or flag next to wrap */
      if (!wrap) {
        curColumn++;
      } else {
        wrap = FALSE;
        curColumn = 1;
        curRow++;
      }

      if (curColumn > (idealWrap-1)) {
        wrap = TRUE;
        btnPtr[i].fsState |= TBSTATE_WRAP;
      }
    }

    TRACE("Result - %d rows\n", curRow + 1);

    /* recalculate toolbar */
    TOOLBAR_CalcToolbar (hwnd);

    /* Resize if necessary (Only if NORESIZE is set - odd, but basically
    if NORESIZE is NOT set, then the toolbar will always be resized to
    take up the whole window. With it set, sizing needs to be manual. */
    if (infoPtr->dwStyle & CCS_NORESIZE) {
      gxSetWindowPos(hwnd, NULL, 0, 0,
        infoPtr->rcBound.right - infoPtr->rcBound.left,
        infoPtr->rcBound.bottom - infoPtr->rcBound.top,
        GXSWP_NOMOVE);
    }

    /* repaint toolbar */
    gxInvalidateRect(hwnd, NULL, TRUE);
  }

  /* return bounding rectangle */
  if (lprc) {
    lprc->left   = infoPtr->rcBound.left;
    lprc->right  = infoPtr->rcBound.right;
    lprc->top    = infoPtr->rcBound.top;
    lprc->bottom = infoPtr->rcBound.bottom;
  }

  return 0;
}


static GXLRESULT
TOOLBAR_SetState (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXINT nIndex;

  nIndex = TOOLBAR_GetButtonIndex (infoPtr, (GXINT)wParam, FALSE);
  if (nIndex == -1)
    return FALSE;

  btnPtr = &infoPtr->buttons[nIndex];

  /* if hidden state has changed the invalidate entire window and recalc */
  if ((btnPtr->fsState & TBSTATE_HIDDEN) != (GXLOWORD(lParam) & TBSTATE_HIDDEN)) {
    btnPtr->fsState = GXLOWORD(lParam);
    TOOLBAR_CalcToolbar (hwnd);
    gxInvalidateRect(hwnd, 0, TRUE);
    return TRUE;
  }

  /* process state changing if current state doesn't match new state */
  if(btnPtr->fsState != GXLOWORD(lParam))
  {
    btnPtr->fsState = GXLOWORD(lParam);
    gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
  }

  return TRUE;
}


static GXLRESULT
TOOLBAR_SetStyle (GXHWND hwnd, GXLPARAM lParam)
{
  gxSetWindowLongW(hwnd, GXGWL_STYLE, lParam);

  return TRUE;
}


static inline GXLRESULT
TOOLBAR_SetToolTips (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("hwnd=%p, hwndTooltip=%p, lParam=0x%lx\n", hwnd, (GXHWND)wParam, lParam);

  infoPtr->hwndToolTip = (GXHWND)wParam;
  return 0;
}


static GXLRESULT
TOOLBAR_SetUnicodeFormat (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXBOOL bTemp;

  TRACE("%s hwnd=%p\n",
    ((GXBOOL)wParam) ? "TRUE" : "FALSE", hwnd);

  bTemp = infoPtr->bUnicode;
  infoPtr->bUnicode = (GXBOOL)wParam;

  return bTemp;
}


static GXLRESULT
TOOLBAR_GetColorScheme (GXHWND hwnd, GXLPCOLORSCHEME lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  lParam->clrBtnHighlight = (infoPtr->clrBtnHighlight == CLR_DEFAULT) ?
    comctl32_color.clrBtnHighlight :
  infoPtr->clrBtnHighlight;
  lParam->clrBtnShadow = (infoPtr->clrBtnShadow == CLR_DEFAULT) ?
    comctl32_color.clrBtnShadow : infoPtr->clrBtnShadow;
  return 1;
}


static GXLRESULT
TOOLBAR_SetColorScheme (GXHWND hwnd, const GXCOLORSCHEME *lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("new colors Hl=%x Shd=%x, old colors Hl=%x Shd=%x\n",
    lParam->clrBtnHighlight, lParam->clrBtnShadow,
    infoPtr->clrBtnHighlight, infoPtr->clrBtnShadow);

  infoPtr->clrBtnHighlight = lParam->clrBtnHighlight;
  infoPtr->clrBtnShadow = lParam->clrBtnShadow;
  gxInvalidateRect(hwnd, NULL, TRUE);
  return 0;
}


static GXLRESULT
TOOLBAR_SetVersion (GXHWND hwnd, GXINT iVersion)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT iOldVersion = infoPtr->iVersion;

  infoPtr->iVersion = iVersion;

  if (infoPtr->iVersion >= 5)
    TOOLBAR_SetUnicodeFormat(hwnd, TRUE);

  return iOldVersion;
}


static GXLRESULT
TOOLBAR_GetStringA (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXWORD iString = GXHIWORD(wParam);
  GXWORD buffersize = GXLOWORD(wParam);
  GXLPSTR str = (GXLPSTR)lParam;
  GXLRESULT ret = -1;

  TRACE("hwnd=%p, iString=%d, buffersize=%d, string=%p\n", hwnd, iString, buffersize, str);

  if (iString < infoPtr->nNumStrings)
  {
    ret = gxWideCharToMultiByte(GXCP_ACP, 0, infoPtr->strings[iString], -1, str, buffersize, NULL, NULL);
    ret--;

    TRACE("returning %s\n", debugstr_a(str));
  }
  else
    WARN("String index %d out of range (largest is %d)\n", iString, infoPtr->nNumStrings - 1);

  return ret;
}


static GXLRESULT
TOOLBAR_GetStringW (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXWORD iString = GXHIWORD(wParam);
  GXWORD len = GXLOWORD(wParam)/sizeof(GXWCHAR) - 1;
  GXLPWSTR str = (GXLPWSTR)lParam;
  GXLRESULT ret = -1;

  TRACE("hwnd=%p, iString=%d, buffersize=%d, string=%p\n", hwnd, iString, GXLOWORD(wParam), str);

  if (iString < infoPtr->nNumStrings)
  {
    len = min(len, (GXWORD)GXSTRLEN(infoPtr->strings[iString]));
    ret = (len+1)*sizeof(GXWCHAR);
    if (str)
    {
      memcpy(str, infoPtr->strings[iString], ret);
      str[len] = '\0';
    }
    ret = len;

    TRACE("returning %s\n", debugstr_w(str));
  }
  else
    WARN("String index %d out of range (largest is %d)\n", iString, infoPtr->nNumStrings - 1);

  return ret;
}

/* UNDOCUMENTED MESSAGE: This appears to set some kind of size. Perhaps it
* is the maximum size of the toolbar? */
static GXLRESULT TOOLBAR_Unkwn45D(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXSIZE * pSize = (GXSIZE*)lParam;
  FIXME("hwnd=%p, wParam=0x%08lx, size.cx=%d, size.cy=%d stub!\n", hwnd, wParam, pSize->cx, pSize->cy);
  return 0;
}


/* This is an extended version of the TB_SETHOTITEM message. It allows the
* caller to specify a reason why the hot item changed (rather than just the
* HICF_OTHER that TB_SETHOTITEM sends). */
static GXLRESULT
TOOLBAR_SetHotItem2 (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXINT nOldHotItem = infoPtr->nHotItem;

  TRACE("old item=%d, new item=%d, flags=%08x\n",
    nOldHotItem, infoPtr->nHotItem, (GXDWORD)lParam);

  if ((GXINT) wParam < 0 || (GXINT)wParam > infoPtr->nNumButtons)
    wParam = -1;

  /* NOTE: an application can still remove the hot item even if anchor
  * highlighting is enabled */

  TOOLBAR_SetHotItemEx(infoPtr, wParam, lParam);

  gxGetFocus();

  return (nOldHotItem < 0) ? -1 : (GXLRESULT)nOldHotItem;
}

/* Sets the toolbar global iListGap parameter which controls the amount of
* spacing between the image and the text of buttons for TBSTYLE_LIST
* toolbars. */
static GXLRESULT TOOLBAR_SetListGap(GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);

  TRACE("hwnd=%p iListGap=%ld\n", hwnd, wParam);

  infoPtr->iListGap = (GXINT)wParam;

  gxInvalidateRect(hwnd, NULL, TRUE);

  return 0;
}

/* Returns the number of maximum number of image lists associated with the
* various states. */
static GXLRESULT TOOLBAR_GetImageListCount(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);

  TRACE("hwnd=%p wParam %08lx lParam %08lx\n", hwnd, wParam, lParam);

  return max(infoPtr->cimlDef, max(infoPtr->cimlHot, infoPtr->cimlDis));
}

static GXLRESULT
TOOLBAR_GetIdealSize (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPSIZE lpsize = (GXLPSIZE)lParam;

  if (lpsize == NULL)
    return FALSE;

  /*
  * Testing shows the following:
  *   wParam    = 0 adjust cx value
  *             = 1 set cy value to max size.
  *   lParam    pointer to GXSIZE structure
  *
  */
  TRACE("wParam %ld, lParam 0x%08lx -> 0x%08x 0x%08x\n",
    wParam, lParam, lpsize->cx, lpsize->cy);

  switch(wParam) {
  case 0:
    if (lpsize->cx == -1) {
      /* **** this is wrong, native measures each button and sets it */
      lpsize->cx = infoPtr->rcBound.right - infoPtr->rcBound.left;
    }
    else if(GXHIWORD(lpsize->cx)) {
      GXRECT rc;
      GXHWND hwndParent = gxGetParent(hwnd);

      gxGetWindowRect(hwnd, &rc);
      gxMapWindowPoints(0, hwndParent, (LPGXPOINT)&rc, 2);
      TRACE("mapped to (%s)\n", wine_dbgstr_rect(&rc));
      lpsize->cx = max(rc.right-rc.left,
        infoPtr->rcBound.right - infoPtr->rcBound.left);
    }
    else {
      lpsize->cx = infoPtr->rcBound.right - infoPtr->rcBound.left;
    }
    break;
  case 1:
    lpsize->cy = infoPtr->rcBound.bottom - infoPtr->rcBound.top;
    break;
  default:
    FIXME("Unknown wParam %ld\n", wParam);
    return 0;
  }
  TRACE("set to -> 0x%08x 0x%08x\n",
    lpsize->cx, lpsize->cy);
  return 1;
}

static GXLRESULT TOOLBAR_Unkwn464(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  FIXME("hwnd=%p wParam %08lx lParam %08lx\n", hwnd, wParam, lParam);

  gxInvalidateRect(hwnd, NULL, TRUE);
  return 1;
}


static GXLRESULT
TOOLBAR_Create (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXDWORD dwStyle = gxGetWindowLongW (hwnd, GXGWL_STYLE);
  GXLOGFONTW logFont;

  TRACE("hwnd = %p\n", hwnd);

  /* initialize info structure */
  infoPtr->nButtonWidth = 23;
  infoPtr->nButtonHeight = 22;
  infoPtr->nBitmapHeight = 16;
  infoPtr->nBitmapWidth = 16;

  infoPtr->nMaxTextRows = 1;
  infoPtr->cxMin = -1;
  infoPtr->cxMax = -1;
  infoPtr->nNumBitmaps = 0;
  infoPtr->nNumStrings = 0;

  infoPtr->bCaptured = FALSE;
  infoPtr->nButtonDown = -1;
  infoPtr->nButtonDrag = -1;
  infoPtr->nOldHit = -1;
  infoPtr->nHotItem = -1;
  infoPtr->hwndNotify = ((GXLPCREATESTRUCTW)lParam)->hwndParent;
  infoPtr->dwDTFlags = (dwStyle & TBSTYLE_LIST) ? GXDT_LEFT | GXDT_VCENTER | GXDT_SINGLELINE | GXDT_END_ELLIPSIS: GXDT_CENTER | GXDT_END_ELLIPSIS;
  infoPtr->bAnchor = FALSE; /* no anchor highlighting */
  infoPtr->bDragOutSent = FALSE;
  infoPtr->iVersion = 0;
  infoPtr->hwndSelf = hwnd;
  infoPtr->bDoRedraw = TRUE;
  infoPtr->clrBtnHighlight = CLR_DEFAULT;
  infoPtr->clrBtnShadow = CLR_DEFAULT;
  infoPtr->szPadding.cx = DEFPAD_CX;
  infoPtr->szPadding.cy = DEFPAD_CY;
  infoPtr->iListGap = DEFLISTGAP;
  infoPtr->iTopMargin = default_top_margin(infoPtr);
  infoPtr->dwStyle = dwStyle;
  infoPtr->tbim.iButton = -1;
  gxGetClientRect(hwnd, &infoPtr->client_rect);
  infoPtr->bUnicode = infoPtr->hwndNotify && 
    (GXNFR_UNICODE == gxSendMessageW(hwnd, GXWM_NOTIFYFORMAT, (GXWPARAM)hwnd, (GXLPARAM)GXNF_REQUERY));
  infoPtr->hwndToolTip = NULL; /* if needed the tooltip control will be created after a WM_MOUSEMOVE */

  gxSystemParametersInfoW (SPI_GETICONTITLELOGFONT, 0, &logFont, 0);
  infoPtr->hFont = infoPtr->hDefaultFont = gxCreateFontIndirectW ((const GXLOGFONTW*)&logFont);

  gxOpenThemeData (hwnd, themeClass);

  TOOLBAR_CheckStyle (hwnd, dwStyle);

  return 0;
}


static GXLRESULT
TOOLBAR_Destroy (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  /* delete tooltip control */
  if (infoPtr->hwndToolTip)
    gxDestroyWindow (infoPtr->hwndToolTip);

  /* delete temporary buffer for tooltip text */
  Free (infoPtr->pszTooltipText);
  Free (infoPtr->bitmaps);            /* bitmaps list */

  /* delete button data */
  Free (infoPtr->buttons);

  /* delete strings */
  if (infoPtr->strings) {
    GXINT i;
    for (i = 0; i < infoPtr->nNumStrings; i++)
      Free (infoPtr->strings[i]);

    Free (infoPtr->strings);
  }

  /* destroy internal image list */
  if (infoPtr->himlInt)
    gxImageList_Destroy (infoPtr->himlInt);

  TOOLBAR_DeleteImageList(&infoPtr->himlDef, &infoPtr->cimlDef);
  TOOLBAR_DeleteImageList(&infoPtr->himlDis, &infoPtr->cimlDis);
  TOOLBAR_DeleteImageList(&infoPtr->himlHot, &infoPtr->cimlHot);

  /* delete default font */
  gxDeleteObject (infoPtr->hDefaultFont);

  gxCloseThemeData (gxGetWindowTheme (hwnd));

  /* free toolbar info data */
  Free (infoPtr);
  gxSetWindowLongPtrW (hwnd, 0, 0);

  return 0;
}


static GXLRESULT
TOOLBAR_EraseBackground (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXNMTBCUSTOMDRAW tbcd;
  GXINT ret = FALSE;
  GXDWORD ntfret;
  GXHTHEME theme = gxGetWindowTheme (hwnd);
  GXDWORD dwEraseCustDraw = 0;

  /* the app has told us not to redraw the toolbar */
  if (!infoPtr->bDoRedraw)
    return FALSE;

  if (infoPtr->dwStyle & TBSTYLE_CUSTOMERASE) {
    ZeroMemory (&tbcd, sizeof(GXNMTBCUSTOMDRAW));
    tbcd.nmcd.dwDrawStage = CDDS_PREERASE;
    tbcd.nmcd.hdc = (GXHDC)wParam;
    ntfret = TOOLBAR_SendNotify (&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
    dwEraseCustDraw = ntfret & 0xffff;

    /* FIXME: in general the return flags *can* be or'ed together */
    switch (dwEraseCustDraw)
    {
    case CDRF_DODEFAULT:
      break;
    case CDRF_SKIPDEFAULT:
      return TRUE;
    default:
      FIXME("[%p] response %d not handled to NM_CUSTOMDRAW (CDDS_PREERASE)\n",
        hwnd, ntfret);
    }
  }

  /* If the toolbar is "transparent" then pass the WM_ERASEBKGND up
  * to my parent for processing.
  */
  if (theme || (infoPtr->dwStyle & TBSTYLE_TRANSPARENT)) {
    GXPOINT pt, ptorig;
    GXHDC hdc = (GXHDC)wParam;
    GXHWND parent;

    pt.x = 0;
    pt.y = 0;
    parent = gxGetParent(hwnd);
    gxMapWindowPoints(hwnd, parent, &pt, 1);
    gxOffsetWindowOrgEx (hdc, pt.x, pt.y, &ptorig);
    ret = gxSendMessageW (parent, GXWM_ERASEBKGND, wParam, lParam);
    gxSetWindowOrgEx (hdc, ptorig.x, ptorig.y, 0);
  }
  if (!ret)
    ret = gxDefWindowProcW (hwnd, GXWM_ERASEBKGND, wParam, lParam);

  if (dwEraseCustDraw & CDRF_NOTIFYPOSTERASE) {
    ZeroMemory (&tbcd, sizeof(GXNMTBCUSTOMDRAW));
    tbcd.nmcd.dwDrawStage = CDDS_POSTERASE;
    tbcd.nmcd.hdc = (GXHDC)wParam;
    ntfret = TOOLBAR_SendNotify (&tbcd.nmcd.hdr, infoPtr, GXNM_CUSTOMDRAW);
    dwEraseCustDraw = ntfret & 0xffff;
    switch (dwEraseCustDraw)
    {
    case CDRF_DODEFAULT:
      break;
    case CDRF_SKIPDEFAULT:
      return TRUE;
    default:
      FIXME("[%p] response %d not handled to NM_CUSTOMDRAW (CDDS_POSTERASE)\n",
        hwnd, ntfret);
    }
  }
  return ret;
}


static GXLRESULT
TOOLBAR_GetFont (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  return (GXLRESULT)infoPtr->hFont;
}


static void
TOOLBAR_SetRelativeHotItem(TOOLBAR_INFO *infoPtr, GXINT iDirection, GXDWORD dwReason)
{
  GXINT i;
  GXINT nNewHotItem = infoPtr->nHotItem;

  for (i = 0; i < infoPtr->nNumButtons; i++)
  {
    /* did we wrap? */
    if ((nNewHotItem + iDirection < 0) ||
      (nNewHotItem + iDirection >= infoPtr->nNumButtons))
    {
      GXNMTBWRAPHOTITEM nmtbwhi;
      nmtbwhi.idNew = infoPtr->buttons[nNewHotItem].idCommand;
      nmtbwhi.iDirection  = iDirection;
      nmtbwhi.dwReason = dwReason;

      if (TOOLBAR_SendNotify(&nmtbwhi.hdr, infoPtr, GXTBN_WRAPHOTITEM))
        return;
    }

    nNewHotItem += iDirection;
    nNewHotItem = (nNewHotItem + infoPtr->nNumButtons) % infoPtr->nNumButtons;

    if ((infoPtr->buttons[nNewHotItem].fsState & TBSTATE_ENABLED) &&
      !(infoPtr->buttons[nNewHotItem].fsStyle & BTNS_SEP))
    {
      TOOLBAR_SetHotItemEx(infoPtr, nNewHotItem, dwReason);
      break;
    }
  }
}

static GXLRESULT
TOOLBAR_KeyDown (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXNMKEY nmkey;

  nmkey.nVKey = (GXUINT)wParam;
  nmkey.uFlags = GXHIWORD(lParam);

  if (TOOLBAR_SendNotify(&nmkey.hdr, infoPtr, GXNM_KEYDOWN))
    return gxDefWindowProcW(hwnd, GXWM_KEYDOWN, wParam, lParam);

  switch ((GXUINT)wParam)
  {
  case GXVK_LEFT:
  case GXVK_UP:
    TOOLBAR_SetRelativeHotItem(infoPtr, -1, HICF_ARROWKEYS);
    break;
  case GXVK_RIGHT:
  case GXVK_DOWN:
    TOOLBAR_SetRelativeHotItem(infoPtr, 1, HICF_ARROWKEYS);
    break;
  case GXVK_SPACE:
  case GXVK_RETURN:
    if ((infoPtr->nHotItem >= 0) &&
      (infoPtr->buttons[infoPtr->nHotItem].fsState & TBSTATE_ENABLED))
    {
      gxSendMessageW (infoPtr->hwndNotify, GXWM_COMMAND,
        GXMAKEWPARAM(infoPtr->buttons[infoPtr->nHotItem].idCommand, GXBN_CLICKED),
        (GXLPARAM)hwnd);
    }
    break;
  }

  return 0;
}


static GXLRESULT
TOOLBAR_LButtonDblClk (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXPOINT pt;
  GXINT   nHit;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);
  nHit = TOOLBAR_InternalHitTest (hwnd, &pt);

  if (nHit >= 0)
    TOOLBAR_LButtonDown (hwnd, wParam, lParam);
  else if (gxGetWindowLongW (hwnd, GXGWL_STYLE) & CCS_ADJUSTABLE)
    TOOLBAR_Customize (hwnd);

  return 0;
}


static GXLRESULT
TOOLBAR_LButtonDown (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXPOINT pt;
  GXINT   nHit;
  GXNMTOOLBARA nmtb;
  GXNMMOUSE GXNMMOUSE;
  GXBOOL bDragKeyPressed;

  TRACE("\n");

  if (infoPtr->dwStyle & TBSTYLE_ALTDRAG)
    bDragKeyPressed = (gxGetKeyState(GXVK_MENU) < 0);
  else
    bDragKeyPressed = (wParam & GXMK_SHIFT);

  if (infoPtr->hwndToolTip)
    TOOLBAR_RelayEvent (infoPtr->hwndToolTip, hwnd,
    GXWM_LBUTTONDOWN, wParam, lParam);

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);
  nHit = TOOLBAR_InternalHitTest (hwnd, &pt);

  btnPtr = &infoPtr->buttons[nHit];

  if ((nHit >= 0) && bDragKeyPressed && (infoPtr->dwStyle & CCS_ADJUSTABLE))
  {
    infoPtr->nButtonDrag = nHit;
    gxSetCapture (hwnd);

    /* If drag cursor has not been loaded, load it.
    * Note: it doesn't need to be freed */
    if (!hCursorDrag)
      hCursorDrag = gxLoadCursorW(COMCTL32_hModule, (GXLPCWSTR)IDC_MOVEBUTTON);
    gxSetCursor(hCursorDrag);
  }
  else if (nHit >= 0)
  {
    GXRECT arrowRect;
    infoPtr->nOldHit = nHit;

    gxCopyRect(&arrowRect, &btnPtr->rect);
    arrowRect.left = max(btnPtr->rect.left, btnPtr->rect.right - DDARROW_WIDTH);

    /* for EX_DRAWDDARROWS style,  click must be in the drop-down arrow rect */
    if ((btnPtr->fsState & TBSTATE_ENABLED) && 
      ((btnPtr->fsStyle & BTNS_WHOLEDROPDOWN) ||
      ((btnPtr->fsStyle & BTNS_DROPDOWN) &&
      ((TOOLBAR_HasDropDownArrows(infoPtr->dwExStyle) && gxPtInRect(&arrowRect, pt)) ||
      (!TOOLBAR_HasDropDownArrows(infoPtr->dwExStyle))))))
    {
      GXLRESULT res;

      /* draw in pressed state */
      if (btnPtr->fsStyle & BTNS_WHOLEDROPDOWN)
        btnPtr->fsState |= TBSTATE_PRESSED;
      else
        btnPtr->bDropDownPressed = TRUE;
      gxRedrawWindow(hwnd,&btnPtr->rect,0,
        GXRDW_ERASE|GXRDW_INVALIDATE|GXRDW_UPDATENOW);

      memset(&nmtb, 0, sizeof(nmtb));
      nmtb.iItem = btnPtr->idCommand;
      nmtb.rcButton = btnPtr->rect;
      res = TOOLBAR_SendNotify ((GXNMHDR *) &nmtb, infoPtr,
        GXTBN_DROPDOWN);
      TRACE("GXTBN_DROPDOWN responded with %ld\n", res);

      if (res != TBDDRET_TREATPRESSED)
      {
        GXMSG msg;

        /* redraw button in unpressed state */
        if (btnPtr->fsStyle & BTNS_WHOLEDROPDOWN)
          btnPtr->fsState &= ~TBSTATE_PRESSED;
        else
          btnPtr->bDropDownPressed = FALSE;
        gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);

        /* find and set hot item
        * NOTE: native doesn't do this, but that is a bug */
        gxGetCursorPos(&pt);
        gxScreenToClient(hwnd, &pt);
        nHit = TOOLBAR_InternalHitTest(hwnd, &pt);
        if (!infoPtr->bAnchor || (nHit >= 0))
          TOOLBAR_SetHotItemEx(infoPtr, nHit, HICF_MOUSE | HICF_LMOUSE);

        /* remove any left mouse button down or double-click messages
        * so that we can get a toggle effect on the button */
        while (gxPeekMessageW(&msg, hwnd, GXWM_LBUTTONDOWN, GXWM_LBUTTONDOWN, GXPM_REMOVE) ||
          gxPeekMessageW(&msg, hwnd, GXWM_LBUTTONDBLCLK, GXWM_LBUTTONDBLCLK, GXPM_REMOVE))
          ;

        return 0;
      }
      /* otherwise drop through and process as pushed */
    }
    infoPtr->bCaptured = TRUE;
    infoPtr->nButtonDown = nHit;
    infoPtr->bDragOutSent = FALSE;

    btnPtr->fsState |= TBSTATE_PRESSED;

    TOOLBAR_SetHotItemEx(infoPtr, nHit, HICF_MOUSE | HICF_LMOUSE);

    if (btnPtr->fsState & TBSTATE_ENABLED)
      gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
    gxUpdateWindow(hwnd);
    gxSetCapture (hwnd);
  }

  if (nHit >=0)
  {
    memset(&nmtb, 0, sizeof(nmtb));
    nmtb.iItem = btnPtr->idCommand;
    TOOLBAR_SendNotify((GXNMHDR *)&nmtb, infoPtr, GXTBN_BEGINDRAG);
  }

  GXNMMOUSE.dwHitInfo = nHit;

  /* !!! Undocumented - sends NM_LDOWN with the GXNMMOUSE structure. */
  if (nHit < 0)
    GXNMMOUSE.dwItemSpec = -1;
  else
  {
    GXNMMOUSE.dwItemSpec = infoPtr->buttons[GXNMMOUSE.dwHitInfo].idCommand;
    GXNMMOUSE.dwItemData = infoPtr->buttons[GXNMMOUSE.dwHitInfo].dwData;
  }

  gxClientToScreen(hwnd, &pt); 
  GXNMMOUSE.pt = pt;

  if (!TOOLBAR_SendNotify(&GXNMMOUSE.hdr, infoPtr, GXNM_LDOWN))
    return gxDefWindowProcW(hwnd, GXWM_LBUTTONDOWN, wParam, lParam);

  return 0;
}

static GXLRESULT
TOOLBAR_LButtonUp (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;
  GXPOINT pt;
  GXINT   nHit;
  GXINT   nOldIndex = -1;
  GXNMHDR hdr;
  GXNMMOUSE GXNMMOUSE;
  GXNMTOOLBARA nmtb;

  if (infoPtr->hwndToolTip)
    TOOLBAR_RelayEvent (infoPtr->hwndToolTip, hwnd,
    GXWM_LBUTTONUP, wParam, lParam);

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);
  nHit = TOOLBAR_InternalHitTest (hwnd, &pt);

  if (!infoPtr->bAnchor || (nHit >= 0))
    TOOLBAR_SetHotItemEx(infoPtr, nHit, HICF_MOUSE | HICF_LMOUSE);

  if (infoPtr->nButtonDrag >= 0) {
    GXRECT rcClient;
    GXNMHDR hdr;

    btnPtr = &infoPtr->buttons[infoPtr->nButtonDrag];
    gxReleaseCapture();
    /* reset cursor */
    gxSetCursor(gxLoadCursorW(NULL, (GXLPCWSTR)GXIDC_ARROW));

    gxGetClientRect(hwnd, &rcClient);
    if (gxPtInRect(&rcClient, pt))
    {
      GXINT nButton = -1;
      if (nHit >= 0)
        nButton = nHit;
      else if (nHit < -1)
        nButton = -nHit;
      else if ((nHit == -1) && gxPtInRect(&infoPtr->buttons[-nHit].rect, pt))
        nButton = -nHit;

      if (nButton == infoPtr->nButtonDrag)
      {
        /* if the button is moved sightly left and we have a
        * separator there then remove it */
        if (pt.x < (btnPtr->rect.left + (btnPtr->rect.right - btnPtr->rect.left)/2))
        {
          if ((nButton > 0) && (infoPtr->buttons[nButton-1].fsStyle & BTNS_SEP))
            TOOLBAR_DeleteButton(hwnd, nButton - 1);
        }
        else /* else insert a separator before the dragged button */
        {
          GXTBBUTTON tbb;
          memset(&tbb, 0, sizeof(tbb));
          tbb.fsStyle = BTNS_SEP;
          tbb.iString = -1;
          TOOLBAR_InsertButtonT(hwnd, nButton, (GXLPARAM)&tbb, TRUE);
        }
      }
      else
      {
        if (nButton == -1)
        {
          if ((infoPtr->nNumButtons > 0) && (pt.x < infoPtr->buttons[0].rect.left))
            TOOLBAR_MoveButton(hwnd, infoPtr->nButtonDrag, 0);
          else
            TOOLBAR_MoveButton(hwnd, infoPtr->nButtonDrag, infoPtr->nNumButtons);
        }
        else
          TOOLBAR_MoveButton(hwnd, infoPtr->nButtonDrag, nButton);
      }
    }
    else
    {
      TRACE("button %d dragged out of toolbar\n", infoPtr->nButtonDrag);
      TOOLBAR_DeleteButton(hwnd, (GXWPARAM)infoPtr->nButtonDrag);
    }

    /* button under cursor changed so need to re-set hot item */
    TOOLBAR_SetHotItemEx(infoPtr, nHit, HICF_MOUSE | HICF_LMOUSE);
    infoPtr->nButtonDrag = -1;

    TOOLBAR_SendNotify(&hdr, infoPtr, GXTBN_TOOLBARCHANGE);
  }
  else if (infoPtr->nButtonDown >= 0) {
    btnPtr = &infoPtr->buttons[infoPtr->nButtonDown];
    btnPtr->fsState &= ~TBSTATE_PRESSED;

    if (btnPtr->fsStyle & BTNS_CHECK) {
      if (btnPtr->fsStyle & BTNS_GROUP) {
        nOldIndex = TOOLBAR_GetCheckedGroupButtonIndex (infoPtr,
          nHit);
        if ((nOldIndex != nHit) &&
          (nOldIndex != -1))
          infoPtr->buttons[nOldIndex].fsState &= ~TBSTATE_CHECKED;
        btnPtr->fsState |= TBSTATE_CHECKED;
      }
      else {
        if (btnPtr->fsState & TBSTATE_CHECKED)
          btnPtr->fsState &= ~TBSTATE_CHECKED;
        else
          btnPtr->fsState |= TBSTATE_CHECKED;
      }
    }

    if (nOldIndex != -1)
      gxInvalidateRect(hwnd, &infoPtr->buttons[nOldIndex].rect, TRUE);

    /*
    * now we can gxReleaseCapture, which triggers CAPTURECHANGED msg,
    * that resets bCaptured and btn TBSTATE_PRESSED flags,
    * and obliterates nButtonDown and nOldHit (see TOOLBAR_CaptureChanged)
    */
    if ((infoPtr->bCaptured) && (infoPtr->nButtonDown >= 0))
      gxReleaseCapture ();
    infoPtr->nButtonDown = -1;

    /* Issue NM_RELEASEDCAPTURE to parent to let him know it is released */
    TOOLBAR_SendNotify (&hdr, infoPtr,
      GXNM_RELEASEDCAPTURE);

    /* native issues GXTBN_ENDDRAG here, if _LBUTTONDOWN issued the
    * GXTBN_BEGINDRAG
    */
    memset(&nmtb, 0, sizeof(nmtb));
    nmtb.iItem = btnPtr->idCommand;
    TOOLBAR_SendNotify ((GXNMHDR *) &nmtb, infoPtr,
      GXTBN_ENDDRAG);

    if (btnPtr->fsState & TBSTATE_ENABLED)
    {
      gxSendMessageW (infoPtr->hwndNotify, GXWM_COMMAND,
        GXMAKEWPARAM(infoPtr->buttons[nHit].idCommand, GXBN_CLICKED), (GXLPARAM)hwnd);

      /* In case we have just been destroyed... */
      if(!gxIsWindow(hwnd))
        return 0;
    }
  }

  /* !!! Undocumented - toolbar at 4.71 level and above sends
  * NM_CLICK with the GXNMMOUSE structure. */
  GXNMMOUSE.dwHitInfo = nHit;

  if (nHit < 0)
    GXNMMOUSE.dwItemSpec = -1;
  else
  {
    GXNMMOUSE.dwItemSpec = infoPtr->buttons[GXNMMOUSE.dwHitInfo].idCommand;
    GXNMMOUSE.dwItemData = infoPtr->buttons[GXNMMOUSE.dwHitInfo].dwData;
  }

  gxClientToScreen(hwnd, &pt); 
  GXNMMOUSE.pt = pt;

  if (!TOOLBAR_SendNotify((GXLPNMHDR)&GXNMMOUSE, infoPtr, GXNM_CLICK))
    return gxDefWindowProcW(hwnd, GXWM_LBUTTONUP, wParam, lParam);

  return 0;
}

static GXLRESULT
TOOLBAR_RButtonUp( GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXINT nHit;
  GXNMMOUSE GXNMMOUSE;
  GXPOINT pt;

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);

  nHit = TOOLBAR_InternalHitTest(hwnd, &pt);
  GXNMMOUSE.dwHitInfo = nHit;

  if (nHit < 0) {
    GXNMMOUSE.dwItemSpec = -1;
  } else {
    GXNMMOUSE.dwItemSpec = infoPtr->buttons[GXNMMOUSE.dwHitInfo].idCommand;
    GXNMMOUSE.dwItemData = infoPtr->buttons[GXNMMOUSE.dwHitInfo].dwData;
  }

  gxClientToScreen(hwnd, &pt); 
  GXNMMOUSE.pt = pt;

  if (!TOOLBAR_SendNotify((GXLPNMHDR)&GXNMMOUSE, infoPtr, GXNM_RCLICK))
    return gxDefWindowProcW(hwnd, GXWM_RBUTTONUP, wParam, lParam);

  return 0;
}

static GXLRESULT
TOOLBAR_RButtonDblClk( GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXNMHDR nmhdr;

  if (!TOOLBAR_SendNotify(&nmhdr, infoPtr, GXNM_RDBLCLK))
    return gxDefWindowProcW(hwnd, GXWM_RBUTTONDBLCLK, wParam, lParam);

  return 0;
}

static GXLRESULT
TOOLBAR_CaptureChanged(GXHWND hwnd)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  TBUTTON_INFO *btnPtr;

  infoPtr->bCaptured = FALSE;

  if (infoPtr->nButtonDown >= 0)
  {
    btnPtr = &infoPtr->buttons[infoPtr->nButtonDown];
    btnPtr->fsState &= ~TBSTATE_PRESSED;

    infoPtr->nOldHit = -1;

    if (btnPtr->fsState & TBSTATE_ENABLED)
      gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
  }
  return 0;
}

static GXLRESULT
TOOLBAR_MouseLeave (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  /* don't remove hot effects when in anchor highlighting mode or when a
  * drop-down button is pressed */
  if (infoPtr->nHotItem >= 0 && !infoPtr->bAnchor)
  {
    TBUTTON_INFO *hotBtnPtr = &infoPtr->buttons[infoPtr->nHotItem];
    if (!hotBtnPtr->bDropDownPressed)
      TOOLBAR_SetHotItemEx(infoPtr, TOOLBAR_NOWHERE, HICF_MOUSE);
  }

  if (infoPtr->nOldHit < 0)
    return TRUE;

  /* If the last button we were over is depressed then make it not */
  /* depressed and redraw it */
  if(infoPtr->nOldHit == infoPtr->nButtonDown)
  {
    TBUTTON_INFO *btnPtr;
    GXRECT rc1;

    btnPtr = &infoPtr->buttons[infoPtr->nButtonDown];

    btnPtr->fsState &= ~TBSTATE_PRESSED;

    rc1 = btnPtr->rect;
    gxInflateRect (&rc1, 1, 1);
    gxInvalidateRect (hwnd, &rc1, TRUE);
  }

  if (infoPtr->bCaptured && !infoPtr->bDragOutSent)
  {
    GXNMTOOLBARW nmt;
    ZeroMemory(&nmt, sizeof(nmt));
    nmt.iItem = infoPtr->buttons[infoPtr->nButtonDown].idCommand;
    TOOLBAR_SendNotify(&nmt.hdr, infoPtr, GXTBN_DRAGOUT);
    infoPtr->bDragOutSent = TRUE;
  }

  infoPtr->nOldHit = -1; /* reset the old hit index as we've left the toolbar */

  return TRUE;
}

static GXLRESULT
TOOLBAR_MouseMove (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXPOINT pt;
  GXTRACKMOUSEEVENT trackinfo;
  GXINT   nHit;
  TBUTTON_INFO *btnPtr;

  if ((infoPtr->dwStyle & TBSTYLE_TOOLTIPS) && (infoPtr->hwndToolTip == NULL))
    TOOLBAR_TooltipCreateControl(infoPtr);

  if ((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf)) {
    /* fill in the GXTRACKMOUSEEVENT struct */
    trackinfo.cbSize = sizeof(GXTRACKMOUSEEVENT);
    trackinfo.dwFlags = GXTME_QUERY;

    /* call _gxTrackMouseEvent to see if we are currently tracking for this hwnd */
    _gxTrackMouseEvent(&trackinfo);

    /* Make sure tracking is enabled so we receive a WM_MOUSELEAVE message */
    if(trackinfo.hwndTrack != hwnd || !(trackinfo.dwFlags & GXTME_LEAVE)) {
      trackinfo.dwFlags = GXTME_LEAVE; /* notify upon leaving */
      trackinfo.hwndTrack = hwnd;

      /* call GXTRACKMOUSEEVENT so we receive a WM_MOUSELEAVE message */
      /* and can properly deactivate the hot toolbar button */
      _gxTrackMouseEvent(&trackinfo);
    }
  }

  if (infoPtr->hwndToolTip)
    TOOLBAR_RelayEvent (infoPtr->hwndToolTip, hwnd,
    GXWM_MOUSEMOVE, wParam, lParam);

  pt.x = (short)GXLOWORD(lParam);
  pt.y = (short)GXHIWORD(lParam);

  nHit = TOOLBAR_InternalHitTest (hwnd, &pt);

  if (((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf)) 
    && (!infoPtr->bAnchor || (nHit >= 0)))
    TOOLBAR_SetHotItemEx(infoPtr, nHit, HICF_MOUSE);

  if (infoPtr->nOldHit != nHit)
  {
    if (infoPtr->bCaptured)
    {
      if (!infoPtr->bDragOutSent)
      {
        GXNMTOOLBARW nmt;
        ZeroMemory(&nmt, sizeof(nmt));
        nmt.iItem = infoPtr->buttons[infoPtr->nButtonDown].idCommand;
        TOOLBAR_SendNotify(&nmt.hdr, infoPtr, GXTBN_DRAGOUT);
        infoPtr->bDragOutSent = TRUE;
      }

      btnPtr = &infoPtr->buttons[infoPtr->nButtonDown];
      if (infoPtr->nOldHit == infoPtr->nButtonDown) {
        btnPtr->fsState &= ~TBSTATE_PRESSED;
        gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
      }
      else if (nHit == infoPtr->nButtonDown) {
        btnPtr->fsState |= TBSTATE_PRESSED;
        gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
      }
      infoPtr->nOldHit = nHit;
    }
  }

  return 0;
}


static inline GXLRESULT
TOOLBAR_NCActivate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  /*    if (wndPtr->dwStyle & CCS_NODIVIDER) */
  return gxDefWindowProcW (hwnd, GXWM_NCACTIVATE, wParam, lParam);
  /*    else */
  /*  return TOOLBAR_NCPaint (wndPtr, wParam, lParam); */
}


static inline GXLRESULT
TOOLBAR_NCCalcSize (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  if (!(gxGetWindowLongW(hwnd, GXGWL_STYLE) & CCS_NODIVIDER))
    ((LPGXRECT)lParam)->top += gxGetSystemMetrics(GXSM_CYEDGE);

  return gxDefWindowProcW (hwnd, GXWM_NCCALCSIZE, wParam, lParam);
}


static GXLRESULT
TOOLBAR_NCCreate (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr;
  GXLPCREATESTRUCTA cs = (GXLPCREATESTRUCTA)lParam;
  GXDWORD styleadd = 0;

  /* allocate memory for info structure */
  infoPtr = (TOOLBAR_INFO*)Alloc (sizeof(TOOLBAR_INFO));
  gxSetWindowLongPtrW (hwnd, 0, (GXLONG_PTR)infoPtr);

  /* paranoid!! */
  infoPtr->dwStructSize = sizeof(GXTBBUTTON);
  infoPtr->nRows = 1;
  infoPtr->nWidth = 0;

  /* fix instance handle, if the toolbar was created by CreateToolbarEx() */
  if (!gxGetWindowLongPtrW (hwnd, GXGWLP_HINSTANCE)) {
    GXHINSTANCE hInst = (GXHINSTANCE)gxGetWindowLongPtrW (gxGetParent (hwnd), GXGWLP_HINSTANCE);
    gxSetWindowLongPtrW (hwnd, GXGWLP_HINSTANCE, (GXLONG_PTR)hInst);
  }

  /* native control does:
  *    Get a lot of colors and brushes
  *    WM_NOTIFYFORMAT
  *    SystemParametersInfoW(0x1f, 0x3c, adr1, 0)
  *    gxCreateFontIndirectW(adr1)
  *    gxCreateBitmap(0x27, 0x24, 1, 1, 0)
  *    hdc = gxGetDC(toolbar)
  *    gxGetSystemMetrics(0x48)
  *    fnt2=CreateFontW(0xe, 0, 0, 0, 0x190, 0, 0, 0, 0, 2,
  *                     0, 0, 0, 0, "MARLETT")
  *    oldfnt = gxSelectObject(hdc, fnt2)
  *    GetCharWidthW(hdc, 0x36, 0x36, adr2)
  *    gxGetTextMetricsW(hdc, adr3)
  *    gxSelectObject(hdc, oldfnt)
  *    gxDeleteObject(fnt2)
  *    gxReleaseDC(hdc)
  *    gxInvalidateRect(toolbar, 0, 1)
  *    gxSetWindowLongW(toolbar, 0, addr)
  *    gxSetWindowLongW(toolbar, -16, xxx)  **sometimes**
  *                                          WM_STYLECHANGING
  *                             CallWinEx   old         new
  *                       ie 1  0x56000a4c  0x46000a4c  0x56008a4d
  *                       ie 2  0x4600094c  0x4600094c  0x4600894d
  *                       ie 3  0x56000b4c  0x46000b4c  0x56008b4d
  *                      rebar  0x50008844  0x40008844  0x50008845
  *                      pager  0x50000844  0x40000844  0x50008845
  *                    IC35mgr  0x5400084e  **nochange**
  *           on entry to _NCCREATE         0x5400084e
  *                    rowlist  0x5400004e  **nochange**
  *           on entry to _NCCREATE         0x5400004e
  *
  */

  /* I think the code below is a bug, but it is the way that the native
  * controls seem to work. The effect is that if the user of TBSTYLE_FLAT
  * forgets to specify TBSTYLE_TRANSPARENT but does specify either
  * CCS_TOP or CCS_BOTTOM (_NOMOVEY and _TOP), then the control
  * does *not* set TBSTYLE_TRANSPARENT even though it should!!!!
  * Somehow, the only cases of this seem to be MFC programs.
  *
  * Note also that the addition of _TRANSPARENT occurs *only* here. It
  * does not occur in the WM_STYLECHANGING routine.
  *    (Guy Albertelli   9/2001)
  *
  */
  if (((infoPtr->dwStyle & TBSTYLE_FLAT) || gxGetWindowTheme (infoPtr->hwndSelf)) 
    && !(cs->style & TBSTYLE_TRANSPARENT))
    styleadd |= TBSTYLE_TRANSPARENT;
  if (!(cs->style & (CCS_TOP | CCS_NOMOVEY))) {
    styleadd |= CCS_TOP;   /* default to top */
    gxSetWindowLongW (hwnd, GXGWL_STYLE, cs->style | styleadd);
  }

  return gxDefWindowProcW (hwnd, GXWM_NCCREATE, wParam, lParam);
}


static GXLRESULT
TOOLBAR_NCPaint (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  GXDWORD dwStyle = gxGetWindowLongW(hwnd, GXGWL_STYLE);
  GXRECT rcWindow;
  GXHDC hdc;

  if (dwStyle & GXWS_MINIMIZE)
    return 0; /* Nothing to do */

  gxDefWindowProcW (hwnd, GXWM_NCPAINT, wParam, lParam);

  if (!(hdc = gxGetDCEx (hwnd, 0, GXDCX_USESTYLE | GXDCX_WINDOW)))
    return 0;

  if (!(dwStyle & CCS_NODIVIDER))
  {
    gxGetWindowRect (hwnd, &rcWindow);
    gxOffsetRect (&rcWindow, -rcWindow.left, -rcWindow.top);
    if( dwStyle & GXWS_BORDER )
      gxOffsetRect (&rcWindow, 1, 1);
    gxDrawEdge (hdc, &rcWindow, GXEDGE_ETCHED, GXBF_TOP);
  }

  gxReleaseDC( hwnd, hdc );

  return 0;
}


/* handles requests from the tooltip control on what text to display */
static GXLRESULT TOOLBAR_TTGetDispInfo (TOOLBAR_INFO *infoPtr, GXNMTTDISPINFOW *lpnmtdi)
{
  GXINT index = TOOLBAR_GetButtonIndex(infoPtr, lpnmtdi->hdr.idFrom, FALSE);

  TRACE("button index = %d\n", index);

  Free (infoPtr->pszTooltipText);
  infoPtr->pszTooltipText = NULL;

  if (index < 0)
    return 0;

  if (infoPtr->bUnicode)
  {
    GXWCHAR wszBuffer[INFOTIPSIZE+1];
    GXNMTBGETINFOTIPW tbgit;
    unsigned int len; /* in chars */

    wszBuffer[0] = '\0';
    wszBuffer[INFOTIPSIZE] = '\0';

    tbgit.pszText = wszBuffer;
    tbgit.cchTextMax = INFOTIPSIZE;
    tbgit.iItem = lpnmtdi->hdr.idFrom;
    tbgit.lParam = infoPtr->buttons[index].dwData;

    TOOLBAR_SendNotify(&tbgit.hdr, infoPtr, GXTBN_GETINFOTIPW);

    TRACE("GXTBN_GETINFOTIPW - got string %s\n", debugstr_w(tbgit.pszText));

    len = (unsigned int)GXSTRLEN(tbgit.pszText);
    if (len > sizeof(lpnmtdi->szText)/sizeof(lpnmtdi->szText[0])-1)
    {
      /* need to allocate temporary buffer in infoPtr as there
      * isn't enough space in buffer passed to us by the
      * tooltip control */
      infoPtr->pszTooltipText = (GXLPWSTR)Alloc((len+1)*sizeof(GXWCHAR));
      if (infoPtr->pszTooltipText)
      {
        memcpy(infoPtr->pszTooltipText, tbgit.pszText, (len+1)*sizeof(GXWCHAR));
        lpnmtdi->lpszText = infoPtr->pszTooltipText;
        return 0;
      }
    }
    else if (len > 0)
    {
      memcpy(lpnmtdi->lpszText, tbgit.pszText, (len+1)*sizeof(GXWCHAR));
      return 0;
    }
  }
  else
  {
    GXCHAR szBuffer[INFOTIPSIZE+1];
    GXNMTBGETINFOTIPA tbgit;
    unsigned int len; /* in chars */

    szBuffer[0] = '\0';
    szBuffer[INFOTIPSIZE] = '\0';

    tbgit.pszText = szBuffer;
    tbgit.cchTextMax = INFOTIPSIZE;
    tbgit.iItem = lpnmtdi->hdr.idFrom;
    tbgit.lParam = infoPtr->buttons[index].dwData;

    TOOLBAR_SendNotify(&tbgit.hdr, infoPtr, GXTBN_GETINFOTIPA);

    TRACE("GXTBN_GETINFOTIPA - got string %s\n", debugstr_a(tbgit.pszText));

    len = gxMultiByteToWideChar(GXCP_ACP, 0, tbgit.pszText, -1, NULL, 0);
    if (len > sizeof(lpnmtdi->szText)/sizeof(lpnmtdi->szText[0]))
    {
      /* need to allocate temporary buffer in infoPtr as there
      * isn't enough space in buffer passed to us by the
      * tooltip control */
      infoPtr->pszTooltipText = (GXLPWSTR)Alloc(len*sizeof(GXWCHAR));
      if (infoPtr->pszTooltipText)
      {
        gxMultiByteToWideChar(GXCP_ACP, 0, tbgit.pszText, -1, infoPtr->pszTooltipText, len);
        lpnmtdi->lpszText = infoPtr->pszTooltipText;
        return 0;
      }
    }
    else if (tbgit.pszText[0])
    {
      gxMultiByteToWideChar(GXCP_ACP, 0, tbgit.pszText, -1,
        lpnmtdi->lpszText, sizeof(lpnmtdi->szText)/sizeof(lpnmtdi->szText[0]));
      return 0;
    }
  }

  /* if button has text, but it is not shown then automatically
  * use that text as tooltip */
  if ((infoPtr->dwExStyle & TBSTYLE_EX_MIXEDBUTTONS) &&
    !(infoPtr->buttons[index].fsStyle & BTNS_SHOWTEXT))
  {
    GXLPWSTR pszText = TOOLBAR_GetText(infoPtr, &infoPtr->buttons[index]);
    unsigned int len = pszText ? (unsigned int)GXSTRLEN(pszText) : 0;

    TRACE("using button hidden text %s\n", debugstr_w(pszText));

    if (len > sizeof(lpnmtdi->szText)/sizeof(lpnmtdi->szText[0])-1)
    {
      /* need to allocate temporary buffer in infoPtr as there
      * isn't enough space in buffer passed to us by the
      * tooltip control */
      infoPtr->pszTooltipText = (GXLPWSTR)Alloc((len+1)*sizeof(GXWCHAR));
      if (infoPtr->pszTooltipText)
      {
        memcpy(infoPtr->pszTooltipText, pszText, (len+1)*sizeof(GXWCHAR));
        lpnmtdi->lpszText = infoPtr->pszTooltipText;
        return 0;
      }
    }
    else if (len > 0)
    {
      memcpy(lpnmtdi->lpszText, pszText, (len+1)*sizeof(GXWCHAR));
      return 0;
    }
  }

  TRACE("Sending tooltip notification to %p\n", infoPtr->hwndNotify);

  /* last resort: send notification on to app */
  /* FIXME: find out what is really used here */
  return gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY, lpnmtdi->hdr.idFrom, (GXLPARAM)lpnmtdi);
}


static inline GXLRESULT
TOOLBAR_Notify (GXHWND hwnd, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXLPNMHDR lpnmh = (GXLPNMHDR)lParam;

  switch (lpnmh->code)
  {
  case PGN_CALCSIZE:
    {
      GXLPNMPGCALCSIZE lppgc = (GXLPNMPGCALCSIZE)lParam;

      if (lppgc->dwFlag == PGF_CALCWIDTH) {
        lppgc->iWidth = infoPtr->rcBound.right - infoPtr->rcBound.left;
        TRACE("processed PGN_CALCSIZE, returning horz size = %d\n",
          lppgc->iWidth);
      }
      else {
        lppgc->iHeight = infoPtr->rcBound.bottom - infoPtr->rcBound.top;
        TRACE("processed PGN_CALCSIZE, returning vert size = %d\n",
          lppgc->iHeight);
      }
      return 0;
    }

  case PGN_SCROLL:
    {
      GXLPNMPGSCROLL lppgs = (GXLPNMPGSCROLL)lParam;

      lppgs->iScroll = (lppgs->iDir & (PGF_SCROLLLEFT | PGF_SCROLLRIGHT)) ?
        infoPtr->nButtonWidth : infoPtr->nButtonHeight;
      TRACE("processed PGN_SCROLL, returning scroll=%d, dir=%d\n",
        lppgs->iScroll, lppgs->iDir);
      return 0;
    }

  case TTN_GETDISPINFOW:
    return TOOLBAR_TTGetDispInfo(infoPtr, (GXLPNMTTDISPINFOW)lParam);

  case TTN_GETDISPINFOA:
    FIXME("TTN_GETDISPINFOA - should not be received; please report\n");
    return 0;

  default:
    return 0;
  }
}


static GXLRESULT
TOOLBAR_NotifyFormat(const TOOLBAR_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  GXLRESULT format;

  TRACE("wParam = 0x%lx, lParam = 0x%08lx\n", wParam, lParam);

  if (lParam == GXNF_QUERY)
    return GXNFR_UNICODE;

  if (lParam == GXNF_REQUERY) {
    format = gxSendMessageW(infoPtr->hwndNotify,
      GXWM_NOTIFYFORMAT, (GXWPARAM)infoPtr->hwndSelf, GXNF_QUERY);
    if ((format != GXNFR_ANSI) && (format != GXNFR_UNICODE)) {
      ERR("wrong response to WM_NOTIFYFORMAT (%ld), assuming ANSI\n",
        format);
      format = GXNFR_ANSI;
    }
    return format;
  }
  return 0;
}


static GXLRESULT
TOOLBAR_Paint (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);
  GXHDC hdc;
  GXPAINTSTRUCT ps;

  /* fill ps.rcPaint with a default rect */
  ps.rcPaint = infoPtr->rcBound;

  hdc = wParam==0 ? gxBeginPaint(hwnd, &ps) : (GXHDC)wParam;

  TRACE("psrect=(%s)\n", wine_dbgstr_rect(&ps.rcPaint));

  TOOLBAR_Refresh (hwnd, hdc, &ps);
  if (!wParam) gxEndPaint (hwnd, &ps);

  return 0;
}


static GXLRESULT
TOOLBAR_SetFocus (GXHWND hwnd, GXWPARAM wParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("nHotItem = %d\n", infoPtr->nHotItem);

  /* make first item hot */
  if (infoPtr->nNumButtons > 0)
    TOOLBAR_SetHotItemEx(infoPtr, 0, HICF_OTHER);

  return 0;
}

static GXLRESULT
TOOLBAR_SetFont(GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr(hwnd);

  TRACE("font=%p redraw=%ld\n", (GXHFONT)wParam, lParam);

  if (wParam == 0)
    infoPtr->hFont = infoPtr->hDefaultFont;
  else
    infoPtr->hFont = (GXHFONT)wParam;

  TOOLBAR_CalcToolbar(hwnd);

  if (lParam)
    gxInvalidateRect(hwnd, NULL, TRUE);
  return 1;
}

static GXLRESULT
TOOLBAR_SetRedraw (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
/*****************************************************
*
* Function;
*  Handles the WM_SETREDRAW message.
*
* Documentation:
*  According to testing V4.71 of COMCTL32 returns the
*  *previous* status of the redraw flag (either 0 or 1)
*  instead of the MSDN documented value of 0 if handled.
*  (For laughs see the "consistency" with same function
*   in rebar.)
*
*****************************************************/
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);
  GXBOOL oldredraw = infoPtr->bDoRedraw;

  TRACE("set to %s\n",
    (wParam) ? "TRUE" : "FALSE");
  infoPtr->bDoRedraw = (GXBOOL) wParam;
  if (wParam) {
    gxInvalidateRect (infoPtr->hwndSelf, 0, TRUE);
  }
  return (oldredraw) ? 1 : 0;
}


static GXLRESULT
TOOLBAR_Size (GXHWND hwnd, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("sizing toolbar!\n");

  if (infoPtr->dwExStyle & TBSTYLE_EX_HIDECLIPPEDBUTTONS)
  {
    GXRECT delta_width, delta_height, client, dummy;
    GXDWORD min_x, max_x, min_y, max_y;
    TBUTTON_INFO *btnPtr;
    GXINT i;

    gxGetClientRect(hwnd, &client);
    if(client.right > infoPtr->client_rect.right)
    {
      min_x = infoPtr->client_rect.right;
      max_x = client.right;
    }
    else
    {
      max_x = infoPtr->client_rect.right;
      min_x = client.right;
    }
    if(client.bottom > infoPtr->client_rect.bottom)
    {
      min_y = infoPtr->client_rect.bottom;
      max_y = client.bottom;
    }
    else
    {
      max_y = infoPtr->client_rect.bottom;
      min_y = client.bottom;
    }

    gxSetRect(&delta_width, min_x, 0, max_x, min_y);
    gxSetRect(&delta_height, 0, min_y, max_x, max_y);

    TRACE("delta_width %s delta_height %s\n", wine_dbgstr_rect(&delta_width), wine_dbgstr_rect(&delta_height));
    btnPtr = infoPtr->buttons;
    for (i = 0; i < infoPtr->nNumButtons; i++, btnPtr++)
      if(gxIntersectRect(&dummy, &delta_width, &btnPtr->rect) ||
        gxIntersectRect(&dummy, &delta_height, &btnPtr->rect))
        gxInvalidateRect(hwnd, &btnPtr->rect, TRUE);
  }
  gxGetClientRect(hwnd, &infoPtr->client_rect);
  TOOLBAR_AutoSize(hwnd);
  return 0;
}


static GXLRESULT
TOOLBAR_StyleChanged (GXHWND hwnd, GXINT nType, const GXSTYLESTRUCT *lpStyle)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  if (nType == GXGWL_STYLE)
  {
    GXDWORD dwOldStyle = infoPtr->dwStyle;

    if (lpStyle->styleNew & TBSTYLE_LIST)
      infoPtr->dwDTFlags = GXDT_LEFT | GXDT_VCENTER | GXDT_SINGLELINE | GXDT_END_ELLIPSIS;
    else
      infoPtr->dwDTFlags = GXDT_CENTER | GXDT_END_ELLIPSIS;

    TOOLBAR_CheckStyle (hwnd, lpStyle->styleNew);

    TRACE("new style 0x%08x\n", lpStyle->styleNew);

    infoPtr->dwStyle = lpStyle->styleNew;

    if ((dwOldStyle ^ lpStyle->styleNew) & (TBSTYLE_WRAPABLE | CCS_VERT))
      TOOLBAR_LayoutToolbar(hwnd);

    /* only resize if one of the CCS_* styles was changed */
    if ((dwOldStyle ^ lpStyle->styleNew) & COMMON_STYLES)
    {
      TOOLBAR_AutoSize (hwnd);

      gxInvalidateRect(hwnd, NULL, TRUE);
    }
  }

  return 0;
}


static GXLRESULT
TOOLBAR_SysColorChange (GXHWND hwnd)
{
  gxCOMCTL32_RefreshSysColors();

  return 0;
}


/* update theme after a WM_THEMECHANGED message */
static GXLRESULT theme_changed (GXHWND hwnd)
{
  GXHTHEME theme = gxGetWindowTheme (hwnd);
  gxCloseThemeData (theme);
  gxOpenThemeData (hwnd, themeClass);
  return 0;
}


static GXLRESULT GXCALLBACK
ToolbarWindowProc (GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  TOOLBAR_INFO *infoPtr = TOOLBAR_GetInfoPtr (hwnd);

  TRACE("hwnd=%p msg=%x wparam=%lx lparam=%lx\n",
    hwnd, uMsg, /* SPY_GetMsgName(uMsg), */ wParam, lParam);

  if (!infoPtr && (uMsg != GXWM_NCCREATE))
    return gxDefWindowProcW( hwnd, uMsg, wParam, lParam );

  switch (uMsg)
  {
  case GXTB_ADDBITMAP:
    return TOOLBAR_AddBitmap (hwnd, wParam, lParam);

  case GXTB_ADDBUTTONSA:
    return TOOLBAR_AddButtonsT(hwnd, wParam, lParam, FALSE);

  case GXTB_ADDBUTTONSW:
    return TOOLBAR_AddButtonsT(hwnd, wParam, lParam, TRUE);

  case GXTB_ADDSTRINGA:
    return TOOLBAR_AddStringA (hwnd, wParam, lParam);

  case GXTB_ADDSTRINGW:
    return TOOLBAR_AddStringW (hwnd, wParam, lParam);

  case GXTB_AUTOSIZE:
    return TOOLBAR_AutoSize (hwnd);

  case GXTB_BUTTONCOUNT:
    return TOOLBAR_ButtonCount (hwnd);

  case GXTB_BUTTONSTRUCTSIZE:
    return TOOLBAR_ButtonStructSize (hwnd, wParam);

  case GXTB_CHANGEBITMAP:
    return TOOLBAR_ChangeBitmap (hwnd, wParam, lParam);

  case GXTB_CHECKBUTTON:
    return TOOLBAR_CheckButton (hwnd, wParam, lParam);

  case GXTB_COMMANDTOINDEX:
    return TOOLBAR_CommandToIndex (hwnd, wParam);

  case GXTB_CUSTOMIZE:
    return TOOLBAR_Customize (hwnd);

  case GXTB_DELETEBUTTON:
    return TOOLBAR_DeleteButton (hwnd, wParam);

  case GXTB_ENABLEBUTTON:
    return TOOLBAR_EnableButton (hwnd, wParam, lParam);

  case GXTB_GETANCHORHIGHLIGHT:
    return TOOLBAR_GetAnchorHighlight (hwnd);

  case GXTB_GETBITMAP:
    return TOOLBAR_GetBitmap (hwnd, wParam);

  case GXTB_GETBITMAPFLAGS:
    return TOOLBAR_GetBitmapFlags ();

  case GXTB_GETBUTTON:
    return TOOLBAR_GetButton (hwnd, wParam, lParam);

  case GXTB_GETBUTTONINFOA:
    return TOOLBAR_GetButtonInfoT(hwnd, wParam, lParam, FALSE);

  case GXTB_GETBUTTONINFOW:
    return TOOLBAR_GetButtonInfoT(hwnd, wParam, lParam, TRUE);

  case GXTB_GETBUTTONSIZE:
    return TOOLBAR_GetButtonSize (hwnd);

  case GXTB_GETBUTTONTEXTA:
    return TOOLBAR_GetButtonTextA (hwnd, wParam, lParam);

  case GXTB_GETBUTTONTEXTW:
    return TOOLBAR_GetButtonTextW (hwnd, wParam, lParam);

  case GXTB_GETDISABLEDIMAGELIST:
    return TOOLBAR_GetDisabledImageList (hwnd, wParam, lParam);

  case GXTB_GETEXTENDEDSTYLE:
    return TOOLBAR_GetExtendedStyle (hwnd);

  case GXTB_GETHOTIMAGELIST:
    return TOOLBAR_GetHotImageList (hwnd, wParam, lParam);

  case GXTB_GETHOTITEM:
    return TOOLBAR_GetHotItem (hwnd);

  case GXTB_GETIMAGELIST:
    return TOOLBAR_GetDefImageList (hwnd, wParam, lParam);

  case GXTB_GETINSERTMARK:
    return TOOLBAR_GetInsertMark (hwnd, lParam);

  case GXTB_GETINSERTMARKCOLOR:
    return TOOLBAR_GetInsertMarkColor (hwnd);

  case GXTB_GETITEMRECT:
    return TOOLBAR_GetItemRect (hwnd, wParam, lParam);

  case GXTB_GETMAXSIZE:
    return TOOLBAR_GetMaxSize (hwnd, lParam);

    /*  case TB_GETOBJECT:      */ /* 4.71 */

  case GXTB_GETPADDING:
    return TOOLBAR_GetPadding (hwnd);

  case GXTB_GETRECT:
    return TOOLBAR_GetRect (hwnd, wParam, lParam);

  case GXTB_GETROWS:
    return TOOLBAR_GetRows (hwnd);

  case GXTB_GETSTATE:
    return TOOLBAR_GetState (hwnd, wParam);

  case GXTB_GETSTRINGA:
    return TOOLBAR_GetStringA (hwnd, wParam, lParam);

  case GXTB_GETSTRINGW:
    return TOOLBAR_GetStringW (hwnd, wParam, lParam);

  case GXTB_GETSTYLE:
    return TOOLBAR_GetStyle (hwnd);

  case GXTB_GETTEXTROWS:
    return TOOLBAR_GetTextRows (hwnd);

  case GXTB_GETTOOLTIPS:
    return TOOLBAR_GetToolTips (hwnd);

  case GXTB_GETUNICODEFORMAT:
    return TOOLBAR_GetUnicodeFormat (hwnd);

  case GXTB_HIDEBUTTON:
    return TOOLBAR_HideButton (hwnd, wParam, lParam);

  case GXTB_HITTEST:
    return TOOLBAR_HitTest (hwnd, lParam);

  case GXTB_INDETERMINATE:
    return TOOLBAR_Indeterminate (hwnd, wParam, lParam);

  case GXTB_INSERTBUTTONA:
    return TOOLBAR_InsertButtonT(hwnd, wParam, lParam, FALSE);

  case GXTB_INSERTBUTTONW:
    return TOOLBAR_InsertButtonT(hwnd, wParam, lParam, TRUE);

    /*  case TB_INSERTMARKHITTEST:    */ /* 4.71 */

  case GXTB_ISBUTTONCHECKED:
    return TOOLBAR_IsButtonChecked (hwnd, wParam);

  case GXTB_ISBUTTONENABLED:
    return TOOLBAR_IsButtonEnabled (hwnd, wParam);

  case GXTB_ISBUTTONHIDDEN:
    return TOOLBAR_IsButtonHidden (hwnd, wParam);

  case GXTB_ISBUTTONHIGHLIGHTED:
    return TOOLBAR_IsButtonHighlighted (hwnd, wParam);

  case GXTB_ISBUTTONINDETERMINATE:
    return TOOLBAR_IsButtonIndeterminate (hwnd, wParam);

  case GXTB_ISBUTTONPRESSED:
    return TOOLBAR_IsButtonPressed (hwnd, wParam);

  case GXTB_LOADIMAGES:
    return TOOLBAR_LoadImages (hwnd, wParam, lParam);

  case GXTB_MAPACCELERATORA:
  case GXTB_MAPACCELERATORW:
    return TOOLBAR_MapAccelerator (hwnd, wParam, lParam);

  case GXTB_MARKBUTTON:
    return TOOLBAR_MarkButton (hwnd, wParam, lParam);

  case GXTB_MOVEBUTTON:
    return TOOLBAR_MoveButton (hwnd, wParam, lParam);

  case GXTB_PRESSBUTTON:
    return TOOLBAR_PressButton (hwnd, wParam, lParam);

  case GXTB_REPLACEBITMAP:
    return TOOLBAR_ReplaceBitmap (hwnd, lParam);

  case GXTB_SAVERESTOREA:
    return TOOLBAR_SaveRestoreA (hwnd, wParam, (LPGXTBSAVEPARAMSA)lParam);

  case GXTB_SAVERESTOREW:
    return TOOLBAR_SaveRestoreW (hwnd, wParam, (LPGXTBSAVEPARAMSW)lParam);

  case GXTB_SETANCHORHIGHLIGHT:
    return TOOLBAR_SetAnchorHighlight (hwnd, wParam);

  case GXTB_SETBITMAPSIZE:
    return TOOLBAR_SetBitmapSize (hwnd, wParam, lParam);

  case GXTB_SETBUTTONINFOA:
    return TOOLBAR_SetButtonInfoA (hwnd, wParam, lParam);

  case GXTB_SETBUTTONINFOW:
    return TOOLBAR_SetButtonInfoW (hwnd, wParam, lParam);

  case GXTB_SETBUTTONSIZE:
    return TOOLBAR_SetButtonSize (hwnd, lParam);

  case GXTB_SETBUTTONWIDTH:
    return TOOLBAR_SetButtonWidth (hwnd, lParam);

  case GXTB_SETCMDID:
    return TOOLBAR_SetCmdId (hwnd, wParam, lParam);

  case GXTB_SETDISABLEDIMAGELIST:
    return TOOLBAR_SetDisabledImageList (hwnd, wParam, lParam);

  case GXTB_SETDRAWTEXTFLAGS:
    return TOOLBAR_SetDrawTextFlags (hwnd, wParam, lParam);

  case GXTB_SETEXTENDEDSTYLE:
    return TOOLBAR_SetExtendedStyle (hwnd, wParam, lParam);

  case GXTB_SETHOTIMAGELIST:
    return TOOLBAR_SetHotImageList (hwnd, wParam, lParam);

  case GXTB_SETHOTITEM:
    return TOOLBAR_SetHotItem (hwnd, wParam);

  case GXTB_SETIMAGELIST:
    return TOOLBAR_SetImageList (hwnd, wParam, lParam);

  case GXTB_SETINDENT:
    return TOOLBAR_SetIndent (hwnd, wParam);

  case GXTB_SETINSERTMARK:
    return TOOLBAR_SetInsertMark (hwnd, lParam);

  case GXTB_SETINSERTMARKCOLOR:
    return TOOLBAR_SetInsertMarkColor (hwnd, lParam);

  case GXTB_SETMAXTEXTROWS:
    return TOOLBAR_SetMaxTextRows (hwnd, wParam);

  case GXTB_SETPADDING:
    return TOOLBAR_SetPadding (hwnd, lParam);

  case GXTB_SETPARENT:
    return TOOLBAR_SetParent (hwnd, wParam);

  case GXTB_SETROWS:
    return TOOLBAR_SetRows (hwnd, wParam, lParam);

  case GXTB_SETSTATE:
    return TOOLBAR_SetState (hwnd, wParam, lParam);

  case GXTB_SETSTYLE:
    return TOOLBAR_SetStyle (hwnd, lParam);

  case GXTB_SETTOOLTIPS:
    return TOOLBAR_SetToolTips (hwnd, wParam, lParam);

  case GXTB_SETUNICODEFORMAT:
    return TOOLBAR_SetUnicodeFormat (hwnd, wParam);
    /*
    case TB_UNKWN45D:
    return TOOLBAR_Unkwn45D(hwnd, wParam, lParam);

    case TB_SETHOTITEM2:
    return TOOLBAR_SetHotItem2 (hwnd, wParam, lParam);

    case TB_SETLISTGAP:
    return TOOLBAR_SetListGap(hwnd, wParam);

    case TB_GETIMAGELISTCOUNT:
    return TOOLBAR_GetImageListCount(hwnd, wParam, lParam);

    case TB_GETIDEALSIZE:
    return TOOLBAR_GetIdealSize (hwnd, wParam, lParam);

    case TB_UNKWN464:
    return TOOLBAR_Unkwn464(hwnd, wParam, lParam);
    */
    /* Common Control Messages */

    /*  case TB_GETCOLORSCHEME:      */ /* identical to CCM_ */
  case GXCCM_GETCOLORSCHEME:
    return TOOLBAR_GetColorScheme (hwnd, (GXLPCOLORSCHEME)lParam);

    /*  case TB_SETCOLORSCHEME:      */ /* identical to CCM_ */
  case GXCCM_SETCOLORSCHEME:
    return TOOLBAR_SetColorScheme (hwnd, (GXLPCOLORSCHEME)lParam);

  case GXCCM_GETVERSION:
    return TOOLBAR_GetVersion (hwnd);

  case GXCCM_SETVERSION:
    return TOOLBAR_SetVersion (hwnd, (GXINT)wParam);


    /*  case GXWM_CHAR: */

  case GXWM_CREATE:
    return TOOLBAR_Create (hwnd, lParam);

  case GXWM_DESTROY:
    return TOOLBAR_Destroy (hwnd, wParam, lParam);

  case GXWM_ERASEBKGND:
    return TOOLBAR_EraseBackground (hwnd, wParam, lParam);

  case GXWM_GETFONT:
    return TOOLBAR_GetFont (hwnd, wParam, lParam);

  case GXWM_KEYDOWN:
    return TOOLBAR_KeyDown (hwnd, wParam, lParam);

    /*  case GXWM_KILLFOCUS: */

  case GXWM_LBUTTONDBLCLK:
    return TOOLBAR_LButtonDblClk (hwnd, wParam, lParam);

  case GXWM_LBUTTONDOWN:
    return TOOLBAR_LButtonDown (hwnd, wParam, lParam);

  case GXWM_LBUTTONUP:
    return TOOLBAR_LButtonUp (hwnd, wParam, lParam);

  case GXWM_RBUTTONUP:
    return TOOLBAR_RButtonUp (hwnd, wParam, lParam);

  case GXWM_RBUTTONDBLCLK:
    return TOOLBAR_RButtonDblClk (hwnd, wParam, lParam);

  case GXWM_MOUSEMOVE:
    return TOOLBAR_MouseMove (hwnd, wParam, lParam);

  case GXWM_MOUSELEAVE:
    return TOOLBAR_MouseLeave (hwnd, wParam, lParam);

  case GXWM_CAPTURECHANGED:
    return TOOLBAR_CaptureChanged(hwnd);

  case GXWM_NCACTIVATE:
    return TOOLBAR_NCActivate (hwnd, wParam, lParam);

  case GXWM_NCCALCSIZE:
    return TOOLBAR_NCCalcSize (hwnd, wParam, lParam);

  case GXWM_NCCREATE:
    return TOOLBAR_NCCreate (hwnd, wParam, lParam);

  case GXWM_NCPAINT:
    return TOOLBAR_NCPaint (hwnd, wParam, lParam);

  case GXWM_NOTIFY:
    return TOOLBAR_Notify (hwnd, lParam);

  case GXWM_NOTIFYFORMAT:
    return TOOLBAR_NotifyFormat (infoPtr, wParam, lParam);

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    return TOOLBAR_Paint (hwnd, wParam);

  case GXWM_SETFOCUS:
    return TOOLBAR_SetFocus (hwnd, wParam);

  case GXWM_SETFONT:
    return TOOLBAR_SetFont(hwnd, wParam, lParam);

  case GXWM_SETREDRAW:
    return TOOLBAR_SetRedraw (hwnd, wParam, lParam);

  case GXWM_SIZE:
    return TOOLBAR_Size (hwnd, wParam, lParam);

  case GXWM_STYLECHANGED:
    return TOOLBAR_StyleChanged (hwnd, (GXINT)wParam, (GXLPSTYLESTRUCT)lParam);

  case GXWM_SYSCOLORCHANGE:
    return TOOLBAR_SysColorChange (hwnd);

  case GXWM_THEMECHANGED:
    return theme_changed (hwnd);

    /*  case GXWM_WININICHANGE: */

  case GXWM_CHARTOITEM:
  case GXWM_COMMAND:
  case GXWM_DRAWITEM:
  case GXWM_MEASUREITEM:
  case GXWM_VKEYTOITEM:
    return gxSendMessageW (infoPtr->hwndNotify, uMsg, wParam, lParam);

    /* We see this in Outlook Express 5.x and just does DefWindowProc */
  case GXPGM_FORWARDMOUSE:
    return gxDefWindowProcW (hwnd, uMsg, wParam, lParam);

  default:
    if ((uMsg >= GXWM_USER) && (uMsg < GXWM_APP) && !gxCOMCTL32_IsReflectedMessage(uMsg))
      ERR("unknown msg %04x wp=%08lx lp=%08lx\n",
      uMsg, wParam, lParam);
    return gxDefWindowProcW (hwnd, uMsg, wParam, lParam);
  }
}


GXVOID
TOOLBAR_Register ()
{
  GXWNDCLASSEX wndClass;

  ZeroMemory (&wndClass, sizeof(GXWNDCLASSEX));
  wndClass.style         = GXCS_GLOBALCLASS | GXCS_DBLCLKS;
  wndClass.lpfnWndProc   = ToolbarWindowProc;
  wndClass.cbClsExtra    = 0;
  wndClass.cbWndExtra    = sizeof(TOOLBAR_INFO *);
  wndClass.hCursor       = gxLoadCursorW (0, (GXLPWSTR)GXIDC_ARROW);
  wndClass.hbrBackground = (GXHBRUSH)(GXCOLOR_BTNFACE + 1);
  wndClass.lpszClassName = TOOLBARCLASSNAMEW;

  gxRegisterClassExW (&wndClass);
}


GXVOID
TOOLBAR_Unregister ()
{
  gxUnregisterClassW (TOOLBARCLASSNAMEW, NULL);
}

static GXHIMAGELIST TOOLBAR_InsertImageList(PIMLENTRY **pies, GXINT *cies, GXHIMAGELIST himl, GXINT id)
{
  GXHIMAGELIST himlold;
  PIMLENTRY c = NULL;

  /* Check if the entry already exists */
  c = TOOLBAR_GetImageListEntry(*pies, *cies, id);

  /* If this is a new entry we must create it and insert into the array */
  if (!c)
  {
    PIMLENTRY *pnies;

    c = (PIMLENTRY)Alloc(sizeof(IMLENTRY));
    c->id = id;

    pnies = (PIMLENTRY*)Alloc((*cies + 1) * sizeof(PIMLENTRY));
    memcpy(pnies, *pies, ((*cies) * sizeof(PIMLENTRY)));
    pnies[*cies] = c;
    (*cies)++;

    Free(*pies);
    *pies = pnies;
  }

  himlold = c->himl;
  c->himl = himl;

  return himlold;
}


static GXVOID TOOLBAR_DeleteImageList(PIMLENTRY **pies, GXINT *cies)
{
  GXINT i;

  for (i = 0; i < *cies; i++)
    Free((*pies)[i]);

  Free(*pies);

  *cies = 0;
  *pies = NULL;
}


static PIMLENTRY TOOLBAR_GetImageListEntry(const PIMLENTRY *pies, GXINT cies, GXINT id)
{
  PIMLENTRY c = NULL;

  if (pies != NULL)
  {
    GXINT i;

    for (i = 0; i < cies; i++)
    {
      if (pies[i]->id == id)
      {
        c = pies[i];
        break;
      }
    }
  }

  return c;
}


static GXHIMAGELIST TOOLBAR_GetImageList(const PIMLENTRY *pies, GXINT cies, GXINT id)
{
  GXHIMAGELIST himlDef = 0;
  PIMLENTRY pie = TOOLBAR_GetImageListEntry(pies, cies, id);

  if (pie)
    himlDef = pie->himl;

  return himlDef;
}


static GXBOOL TOOLBAR_GetButtonInfo(const TOOLBAR_INFO *infoPtr, GXNMTOOLBARW *nmtb)
{
  if (infoPtr->bUnicode)
    return TOOLBAR_SendNotify(&nmtb->hdr, infoPtr, GXTBN_GETBUTTONINFOW);
  else
  {
    GXCHAR Buffer[256];
    GXNMTOOLBARA nmtba;
    GXBOOL bRet = FALSE;

    nmtba.iItem = nmtb->iItem;
    nmtba.pszText = Buffer;
    nmtba.cchText = 256;
    ZeroMemory(nmtba.pszText, nmtba.cchText);

    if (TOOLBAR_SendNotify(&nmtba.hdr, infoPtr, GXTBN_GETBUTTONINFOA))
    {
      GXINT ccht = (GXINT)strlen(nmtba.pszText);
      if (ccht)
        gxMultiByteToWideChar(GXCP_ACP, 0, nmtba.pszText, -1,
        nmtb->pszText, nmtb->cchText);

      nmtb->tbButton = nmtba.tbButton;
      bRet = TRUE;
    }

    return bRet;
  }
}


static GXBOOL TOOLBAR_IsButtonRemovable(const TOOLBAR_INFO *infoPtr, GXINT iItem, PCUSTOMBUTTON btnInfo)
{
  GXNMTOOLBARW nmtb;

  /* MSDN states that iItem is the index of the button, rather than the
  * command ID as used by every other NMTOOLBAR notification */
  nmtb.iItem = iItem;
  memcpy(&nmtb.tbButton, &btnInfo->btn, sizeof(GXTBBUTTON));

  return TOOLBAR_SendNotify(&nmtb.hdr, infoPtr, GXTBN_QUERYDELETE);
}
#endif 
#endif // _DEV_DISABLE_UI_CODE