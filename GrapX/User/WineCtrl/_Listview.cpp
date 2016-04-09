#ifndef _DEV_DISABLE_UI_CODE
/*
* Listview control
*
* Copyright 1998, 1999 Eric Kohl
* Copyright 1999 Luc Tourangeau
* Copyright 2000 Jason Mawdsley
* Copyright 2001 CodeWeavers Inc.
* Copyright 2002 Dimitrie O. Paun
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
* of Comctl32.dll version 6.0 on May. 20, 2005, by James Hawkins.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
* 
* TODO:
*
* Default Message Processing
*   -- EN_KILLFOCUS should be handled in WM_COMMAND
*   -- WM_CREATE: create the icon and small icon image lists at this point only if
*      the LVS_SHAREIMAGELISTS style is not specified.
*   -- WM_ERASEBKGND: forward this message to the parent window if the bkgnd
*      color is GXCLR_NONE.
*   -- WM_WINDOWPOSCHANGED: arrange the list items if the current view is icon
*      or small icon and the LVS_AUTOARRANGE style is specified.
*   -- WM_TIMER
*   -- WM_WININICHANGE
*
* Features
*   -- Hot item handling, mouse hovering
*   -- Workareas support
*   -- Tilemode support
*   -- Groups support
*
* Bugs
*   -- Expand large item in ICON mode when the cursor is flying over the icon or text.
*   -- Support CustomDraw options for _WIN32_IE >= 0x560 (see GXNMLVCUSTOMDRAW docs).
*   -- LVA_SNAPTOGRID not implemented
*   -- LISTVIEW_ApproximateViewRect partially implemented
*   -- LISTVIEW_[GS]etColumnOrderArray stubs
*   -- LISTVIEW_SetColumnWidth ignores header images & bitmap
*   -- LISTVIEW_SetIconSpacing is incomplete
*   -- LISTVIEW_SortItems is broken
*   -- LISTVIEW_StyleChanged doesn't handle some changes too well
*
* Speedups
*   -- LISTVIEW_GetNextItem needs to be rewritten. It is currently
*      linear in the number of items in the list, and this is
*      unacceptable for large lists.
*   -- in sorted mode, LISTVIEW_InsertItemT sorts the array,
*      instead of inserting in the right spot
*   -- we should keep an ordered array of coordinates in iconic mode
*      this would allow to frame items (iterator_frameditems),
*      and find nearest item (LVFI_NEARESTXY) a lot more efficiently
*
* Flags
*   -- GXLVIF_COLUMNS
*   -- GXLVIF_GROUPID
*   -- GXLVIF_NORECOMPUTE
*
* States
*   -- LVIS_ACTIVATING (not currently supported by comctl32.dll version 6.0)
*   -- LVIS_CUT
*   -- LVIS_DROPHILITED
*   -- LVIS_OVERLAYMASK
*
* Styles
*   -- LVS_NOLABELWRAP
*   -- LVS_NOSCROLL (see Q137520)
*   -- LVS_SORTASCENDING, LVS_SORTDESCENDING
*   -- LVS_ALIGNTOP
*   -- LVS_TYPESTYLEMASK
*
* Extended Styles
*   -- LVS_EX_BORDERSELECT
*   -- LVS_EX_FLATSB
*   -- LVS_EX_HEADERDRAGDROP
*   -- LVS_EX_INFOTIP
*   -- LVS_EX_LABELTIP
*   -- LVS_EX_MULTIWORKAREAS
*   -- LVS_EX_ONECLICKACTIVATE
*   -- LVS_EX_REGIONAL
*   -- LVS_EX_SIMPLESELECT
*   -- LVS_EX_TRACKSELECT
*   -- LVS_EX_TWOCLICKACTIVATE
*   -- LVS_EX_UNDERLINECOLD
*   -- LVS_EX_UNDERLINEHOT
*   
* Notifications:
*   -- LVN_BEGINSCROLL, LVN_ENDSCROLL
*   -- LVN_GETINFOTIP
*   -- LVN_HOTTRACK
*   -- LVN_MARQUEEBEGIN
*   -- LVN_ODFINDITEM
*   -- LVN_SETDISPINFO
*   -- NM_HOVER
*   -- LVN_BEGINRDRAG
*
* Messages:
*   -- GXLVM_CANCELEDITLABEL
*   -- GXLVM_ENABLEGROUPVIEW
*   -- GXLVM_GETBKIMAGE, GXLVM_SETBKIMAGE
*   -- GXLVM_GETGROUPINFO, GXLVM_SETGROUPINFO
*   -- GXLVM_GETGROUPMETRICS, GXLVM_SETGROUPMETRICS
*   -- GXLVM_GETINSERTMARK, GXLVM_SETINSERTMARK
*   -- GXLVM_GETINSERTMARKCOLOR, GXLVM_SETINSERTMARKCOLOR
*   -- GXLVM_GETINSERTMARKRECT
*   -- GXLVM_GETNUMBEROFWORKAREAS
*   -- GXLVM_GETOUTLINECOLOR, GXLVM_SETOUTLINECOLOR
*   -- GXLVM_GETSELECTEDCOLUMN, GXLVM_SETSELECTEDCOLUMN
*   -- GXLVM_GETISEARCHSTRINGW, GXLVM_GETISEARCHSTRINGA
*   -- GXLVM_GETTILEINFO, GXLVM_SETTILEINFO
*   -- GXLVM_GETTILEVIEWINFO, GXLVM_SETTILEVIEWINFO
*   -- GXLVM_GETUNICODEFORMAT, GXLVM_SETUNICODEFORMAT
*   -- GXLVM_GETVIEW, GXLVM_SETVIEW
*   -- GXLVM_GETWORKAREAS, GXLVM_SETWORKAREAS
*   -- GXLVM_HASGROUP, GXLVM_INSERTGROUP, GXLVM_REMOVEGROUP, GXLVM_REMOVEALLGROUPS
*   -- GXLVM_INSERTGROUPSORTED
*   -- GXLVM_INSERTMARKHITTEST
*   -- GXLVM_ISGROUPVIEWENABLED
*   -- GXLVM_MAPIDTOINDEX, GXLVM_MAPINDEXTOID
*   -- GXLVM_MOVEGROUP
*   -- GXLVM_MOVEITEMTOGROUP
*   -- GXLVM_SETINFOTIP
*   -- GXLVM_SETTILEWIDTH
*   -- GXLVM_SORTGROUPS
*   -- GXLVM_SORTITEMSEX
*
* Macros:
*   -- gxListView_GetCheckSate, gxListView_SetCheckState
*   -- gxListView_GetHoverTime, gxListView_SetHoverTime
*   -- gxListView_GetISearchString
*   -- gxListView_GetNumberOfWorkAreas
*   -- gxListView_GetOrigin
*   -- gxListView_GetTextBkColor
*   -- gxListView_GetUnicodeFormat, gxListView_SetUnicodeFormat
*   -- gxListView_GetWorkAreas, gxListView_SetWorkAreas
*   -- gxListView_SortItemsEx
*
* Functions:
*   -- LVGroupComparE
*
* Known differences in message stream from native control (not known if
* these differences cause problems):
*   GXLVM_INSERTITEM issues GXLVM_SETITEMSTATE and GXLVM_SETITEM in certain cases.
*   GXLVM_SETITEM does not always issue LVN_ITEMCHANGING/LVN_ITEMCHANGED.
*   WM_CREATE does not issue WM_QUERYUISTATE and associated registry
*     processing for "USEDOUBLECLICKTIME".
*/


//#include "config.h"
//#include "wine/port.h"
//
//#include <assert.h>
//#include <ctype.h>
//#include <string.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <stdio.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "winnt.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "winnls.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "uxtheme.h"
//
//#include "wine/debug.h"
//#include "wine/unicode.h"

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

#ifndef assert
#define assert ASSERT
#endif // #ifndef assert

#define _CRT_SECURE_NO_WARNINGS

#include <GrapX.H>
#include <User/GrapXDef.H>
#include "GrapX/gUxtheme.h"
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXImm.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <GrapX/WineComm.H>
#include <User/Win32Emu/GXCommCtrl.H>
#include <User/Win32Emu/_dpa.H>

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

//#undef TRACE
//#define TRACE

//WINE_DEFAULT_DEBUG_CHANNEL(listview);


/* make sure you set this to 0 for production use! */
#define DEBUG_RANGES 1

typedef struct tagCOLUMN_INFO
{
  GXRECT rcHeader;  /* tracks the header's rectangle */
  int fmt;    /* same as LVCOLUMN.fmt */
} COLUMN_INFO;

typedef struct tagITEMHDR
{
  GXLPWSTR pszText;
  GXINT iImage;
} ITEMHDR, *LPITEMHDR;

typedef struct tagSUBITEM_INFO
{
  ITEMHDR hdr;
  GXINT iSubItem;
} SUBITEM_INFO;

typedef struct tagITEM_INFO
{
  ITEMHDR hdr;
  GXUINT state;
  GXLPARAM lParam;
  GXINT iIndent;
} ITEM_INFO;

typedef struct tagRANGE
{
  GXINT lower;
  GXINT upper;
} RANGE;

typedef struct tagRANGES
{
  GXHDPA hdpa;
} *RANGES;

typedef struct tagITERATOR
{
  GXINT nItem;
  GXINT nSpecial;
  RANGE range;
  RANGES ranges;
  GXINT index;
} ITERATOR;

typedef struct tagDELAYED_ITEM_EDIT
{
  GXBOOL fEnabled;
  GXINT iItem;
} DELAYED_ITEM_EDIT;

typedef struct tagLISTVIEW_INFO
{
  GXHWND hwndSelf;
  GXHBRUSH hBkBrush;
  GXCOLORREF clrBk;
  GXCOLORREF clrText;
  GXCOLORREF clrTextBk;
  GXHIMAGELIST himlNormal;
  GXHIMAGELIST himlSmall;
  GXHIMAGELIST himlState;
  GXBOOL bLButtonDown;
  GXBOOL bRButtonDown;
  GXBOOL bDragging;
  GXPOINT ptClickPos;         /* point where the user clicked */ 
  GXBOOL bNoItemMetrics;    /* flags if item metrics are not yet computed */
  GXINT nItemHeight;
  GXINT nItemWidth;
  RANGES selectionRanges;
  GXINT nSelectionMark;
  GXINT nHotItem;
  GXSHORT notifyFormat;
  GXHWND hwndNotify;
  GXRECT rcList;                 /* This rectangle is really the window
                   * client rectangle possibly reduced by the 
                   * horizontal scroll bar and/or header - see 
                   * LISTVIEW_UpdateSize. This rectangle offset
                   * by the LISTVIEW_GetOrigin value is in
                   * client coordinates   */
  GXSIZE iconSize;
  GXSIZE iconSpacing;
  GXSIZE iconStateSize;
  GXUINT uCallbackMask;
  GXHWND hwndHeader;
  GXHCURSOR hHotCursor;
  GXHFONT hDefaultFont;
  GXHFONT hFont;
  GXINT ntmHeight;    /* Some cached metrics of the font used */
  GXINT ntmMaxCharWidth;    /* by the listview to draw items */
  GXINT nEllipsisWidth;
  GXBOOL bRedraw;      /* Turns on/off repaints & invalidations */
  GXBOOL bAutoarrange;    /* Autoarrange flag when NOT in LVS_AUTOARRANGE */
  GXBOOL bFocus;
  GXBOOL bDoChangeNotify;         /* send change notification messages? */
  GXINT nFocusedItem;
  GXRECT rcFocus;
  GXDWORD dwStyle;    /* the cached window GXGWL_STYLE */
  GXDWORD dwLvExStyle;    /* extended listview style */
  GXINT nItemCount;    /* the number of items in the list */
  GXHDPA hdpaItems;               /* array ITEM_INFO pointers */
  GXHDPA hdpaPosX;    /* maintains the (X, Y) coordinates of the */
  GXHDPA hdpaPosY;    /* items in LVS_ICON, and LVS_SMALLICON modes */
  GXHDPA hdpaColumns;    /* array of COLUMN_INFO pointers */
  GXPOINT currIconPos;    /* this is the position next icon will be placed */
  GXPFNLVCOMPARE pfnCompare;
  GXLPARAM lParamSort;
  GXHWND hwndEdit;
  GXWNDPROC EditWndProc;
  GXINT nEditLabelItem;
  GXDWORD dwHoverTime;
  GXHWND hwndToolTip;

  GXDWORD cditemmode;             /* Keep the custom draw flags for an item/row */

  GXDWORD lastKeyPressTimestamp;
  GXWPARAM charCode;
  GXINT nSearchParamLength;
  GXWCHAR szSearchParam[ MAX_PATH ];
  GXBOOL bIsDrawing;
  GXINT nMeasureItemHeight;
  GXINT xTrackLine;               /* The x coefficient of the track line or -1 if none */
  DELAYED_ITEM_EDIT itemEdit;   /* Pointer to this structure will be the timer ID */
} LISTVIEW_INFO;

/*
* constants
*/
/* How many we debug buffer to allocate */
#define DEBUG_BUFFERS 20
/* The size of a single debug bbuffer */
#define DEBUG_BUFFER_SIZE 256

/* Internal interface to LISTVIEW_HScroll and LISTVIEW_VScroll */
#define SB_INTERNAL      -1

/* maximum size of a label */
#define DISP_TEXT_SIZE 512

/* padding for items in list and small icon display modes */
#define WIDTH_PADDING 12

/* padding for items in list, report and small icon display modes */
#define HEIGHT_PADDING 1

/* offset of items in report display mode */
#define REPORT_MARGINX 2

/* padding for icon in large icon display mode
*   ICON_TOP_PADDING_NOTHITABLE - space between top of box and area
*                                 that HITTEST will see.
*   ICON_TOP_PADDING_HITABLE - spacing between above and icon.
*   ICON_TOP_PADDING - sum of the two above.
*   ICON_BOTTOM_PADDING - between bottom of icon and top of text
*   LABEL_HOR_PADDING - between text and sides of box
*   LABEL_VERT_PADDING - between bottom of text and end of box
*
*   ICON_LR_PADDING - additional width above icon size.
*   ICON_LR_HALF - half of the above value
*/
#define ICON_TOP_PADDING_NOTHITABLE  2
#define ICON_TOP_PADDING_HITABLE     2
#define ICON_TOP_PADDING (ICON_TOP_PADDING_NOTHITABLE + ICON_TOP_PADDING_HITABLE)
#define ICON_BOTTOM_PADDING          4
#define LABEL_HOR_PADDING            5
#define LABEL_VERT_PADDING           7
#define ICON_LR_PADDING              16
#define ICON_LR_HALF                 (ICON_LR_PADDING/2)

/* default label width for items in list and small icon display modes */
#define DEFAULT_LABEL_WIDTH 40

/* default column width for items in list display mode */
#define DEFAULT_COLUMN_WIDTH 128

/* Size of "line" scroll for V & H scrolls */
#define LISTVIEW_SCROLL_ICON_LINE_SIZE 37

/* Padding between image and label */
#define IMAGE_PADDING  2

/* Padding behind the label */
#define TRAILING_LABEL_PADDING  12
#define TRAILING_HEADER_PADDING  11

/* Border for the icon caption */
#define CAPTION_BORDER  2

/* Standard DrawText flags */
#define LV_ML_DT_FLAGS  (GXDT_TOP | GXDT_NOPREFIX | GXDT_EDITCONTROL | GXDT_CENTER | GXDT_WORDBREAK | GXDT_WORD_ELLIPSIS | GXDT_END_ELLIPSIS)
#define LV_FL_DT_FLAGS  (GXDT_TOP | GXDT_NOPREFIX | GXDT_EDITCONTROL | GXDT_CENTER | GXDT_WORDBREAK | GXDT_NOCLIP)
#define LV_SL_DT_FLAGS  (GXDT_VCENTER | GXDT_NOPREFIX | GXDT_EDITCONTROL | GXDT_SINGLELINE | GXDT_WORD_ELLIPSIS | GXDT_END_ELLIPSIS)

/* Image index from state */
#define STATEIMAGEINDEX(x) (((x) & LVIS_STATEIMAGEMASK) >> 12)

/* The time in milliseconds to reset the search in the list */
#define KEY_DELAY       450

/* Dump the LISTVIEW_INFO structure to the debug channel */
#define LISTVIEW_DUMP(iP) do { \
  TRACE("hwndSelf=%p, clrBk=0x%06x, clrText=0x%06x, clrTextBk=0x%06x, ItemHeight=%d, ItemWidth=%d, Style=0x%08x\n", \
  iP->hwndSelf, iP->clrBk, iP->clrText, iP->clrTextBk, \
  iP->nItemHeight, iP->nItemWidth, iP->dwStyle); \
  TRACE("hwndSelf=%p, himlNor=%p, himlSml=%p, himlState=%p, Focused=%d, Hot=%d, exStyle=0x%08x, Focus=%d\n", \
  iP->hwndSelf, iP->himlNormal, iP->himlSmall, iP->himlState, \
  iP->nFocusedItem, iP->nHotItem, iP->dwLvExStyle, iP->bFocus ); \
  TRACE("hwndSelf=%p, ntmH=%d, icSz.cx=%d, icSz.cy=%d, icSp.cx=%d, icSp.cy=%d, notifyFmt=%d\n", \
  iP->hwndSelf, iP->ntmHeight, iP->iconSize.cx, iP->iconSize.cy, \
  iP->iconSpacing.cx, iP->iconSpacing.cy, iP->notifyFormat); \
  TRACE("hwndSelf=%p, rcList=%s\n", iP->hwndSelf, wine_dbgstr_rect(&iP->rcList)); \
} while(0)

static const GXWCHAR themeClass[] = {'L','i','s','t','V','i','e','w',0};

/*
* forward declarations
*/
static GXBOOL LISTVIEW_GetItemT(const LISTVIEW_INFO *, GXLPLVITEMW, GXBOOL);
static void LISTVIEW_GetItemBox(const LISTVIEW_INFO *, GXINT, LPGXRECT);
static void LISTVIEW_GetItemOrigin(const LISTVIEW_INFO *, GXINT, LPGXPOINT);
static GXBOOL LISTVIEW_GetItemPosition(const LISTVIEW_INFO *, GXINT, LPGXPOINT);
static GXBOOL LISTVIEW_GetItemRect(const LISTVIEW_INFO *, GXINT, LPGXRECT);
static GXINT LISTVIEW_GetLabelWidth(const LISTVIEW_INFO *, GXINT);
static void LISTVIEW_GetOrigin(const LISTVIEW_INFO *, LPGXPOINT);
static GXBOOL LISTVIEW_GetViewRect(const LISTVIEW_INFO *, LPGXRECT);
static void LISTVIEW_SetGroupSelection(LISTVIEW_INFO *, GXINT);
static GXBOOL LISTVIEW_SetItemT(LISTVIEW_INFO *, GXLVITEMW *, GXBOOL);
static void LISTVIEW_UpdateScroll(const LISTVIEW_INFO *);
static void LISTVIEW_SetSelection(LISTVIEW_INFO *, GXINT);
static void LISTVIEW_UpdateSize(LISTVIEW_INFO *);
static GXHWND LISTVIEW_EditLabelT(LISTVIEW_INFO *, GXINT, GXBOOL);
static GXLRESULT LISTVIEW_Command(const LISTVIEW_INFO *, GXWPARAM, GXLPARAM);
static GXBOOL LISTVIEW_SortItems(LISTVIEW_INFO *, GXPFNLVCOMPARE, GXLPARAM);
static GXINT LISTVIEW_GetStringWidthT(const LISTVIEW_INFO *, GXLPCWSTR, GXBOOL);
static GXBOOL LISTVIEW_KeySelection(LISTVIEW_INFO *, GXINT);
static GXUINT LISTVIEW_GetItemState(const LISTVIEW_INFO *, GXINT, GXUINT);
static GXBOOL LISTVIEW_SetItemState(LISTVIEW_INFO *, GXINT, const GXLVITEMW *);
static GXLRESULT LISTVIEW_VScroll(LISTVIEW_INFO *, GXINT, GXINT, GXHWND);
static GXLRESULT LISTVIEW_HScroll(LISTVIEW_INFO *, GXINT, GXINT, GXHWND);
static GXINT LISTVIEW_GetTopIndex(const LISTVIEW_INFO *);
static GXBOOL LISTVIEW_EnsureVisible(LISTVIEW_INFO *, GXINT, GXBOOL);
static GXHWND CreateEditLabelT(LISTVIEW_INFO *, GXLPCWSTR, GXDWORD, GXINT, GXINT, GXINT, GXINT, GXBOOL);
static GXHIMAGELIST LISTVIEW_SetImageList(LISTVIEW_INFO *, GXINT, GXHIMAGELIST);
static GXINT LISTVIEW_HitTest(const LISTVIEW_INFO *, GXLPLVHITTESTINFO, GXBOOL, GXBOOL);
static GXINT LISTVIEW_FindItemW(const LISTVIEW_INFO *infoPtr, GXINT nStart, const GXLVFINDINFOW *lpFindInfo);

char *wine_dbg_sprintf(char *fmt, ...)
{

  static char buffer[1024];
  va_list val;
  va_start(val, fmt);
  vsprintf(buffer, fmt, val);     
  va_end(val);

  return buffer;
}

/******** Text handling functions *************************************/

/* A text pointer is either NULL, LPSTR_TEXTCALLBACK, or points to a
* text string. The string may be ANSI or Unicode, in which case
* the boolean isW tells us the type of the string.
*
* The name of the function tell what type of strings it expects:
*   W: Unicode, T: ANSI/Unicode - function of isW
*/

static inline GXBOOL is_textW(GXLPCWSTR text)
{
  return text != NULL && text != GXLPSTR_TEXTCALLBACKW;
}

static inline GXBOOL is_textT(GXLPCWSTR text, GXBOOL isW)
{
  /* we can ignore isW since GXLPSTR_TEXTCALLBACKW == LPSTR_TEXTCALLBACKA */
  return is_textW(text);
}

static inline int textlenT(GXLPCWSTR text, GXBOOL isW)
{
  return !is_textT(text, isW) ? 0 :
    isW ? GXSTRLEN(text) : GXSTRLEN((GXLPCSTR)text);
}

static inline void textcpynT(GXLPWSTR dest, GXBOOL isDestW, GXLPCWSTR src, GXBOOL isSrcW, GXINT max)
{
  if (isDestW)
    if (isSrcW) GXSTRCPYN(dest, src, max);
    else gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPCSTR)src, -1, dest, max);
  else
    if (isSrcW) gxWideCharToMultiByte(GXCP_ACP, 0, src, -1, (GXLPSTR)dest, max, NULL, NULL);
    else GXSTRCPYN((GXLPSTR)dest, (GXLPCSTR)src, max);
}

static inline GXLPWSTR textdupTtoW(GXLPCWSTR text, GXBOOL isW)
{
  GXLPWSTR wstr = (GXLPWSTR)text;

  if (!isW && is_textT(text, isW))
  {
    GXINT len = gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPCSTR)text, -1, NULL, 0);
    wstr = (GXLPWSTR)Alloc(len * sizeof(GXWCHAR));
    if (wstr) gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPCSTR)text, -1, wstr, len);
  }
  TRACEW(L"   wstr=%s\n", text == GXLPSTR_TEXTCALLBACKW ?  L"(callback)" : debugstr_w(wstr));
  return wstr;
}

static inline void textfreeT(GXLPWSTR wstr, GXBOOL isW)
{
  if (!isW && is_textT(wstr, isW)) Free (wstr);
}

/*
* dest is a pointer to a Unicode string
* src is a pointer to a string (Unicode if isW, ANSI if !isW)
*/
static GXBOOL textsetptrT(GXLPWSTR *dest, GXLPCWSTR src, GXBOOL isW)
{
  GXBOOL bResult = TRUE;

  if (src == GXLPSTR_TEXTCALLBACKW)
  {
    if (is_textW(*dest)) Free(*dest);
    *dest = GXLPSTR_TEXTCALLBACKW;
  }
  else
  {
    GXLPWSTR pszText = textdupTtoW(src, isW);
    if (*dest == GXLPSTR_TEXTCALLBACKW) *dest = NULL;
    bResult = gxStr_SetPtrW(dest, pszText);
    textfreeT(pszText, isW);
  }
  return bResult;
}

/*
* compares a Unicode to a Unicode/ANSI text string
*/
static inline int textcmpWT(GXLPCWSTR aw, GXLPCWSTR bt, GXBOOL isW)
{
  if (!aw) return bt ? -1 : 0;
  if (!bt) return aw ? 1 : 0;
  if (aw == GXLPSTR_TEXTCALLBACKW)
    return bt == GXLPSTR_TEXTCALLBACKW ? 0 : -1;
  if (bt != GXLPSTR_TEXTCALLBACKW)
  {
    GXLPWSTR bw = textdupTtoW(bt, isW);
    int r = bw ? GXSTRCMP(aw, bw) : 1;
    textfreeT(bw, isW);
    return r;
  }      

  return 1;
}

//static inline int lstrncmpiW(GXLPCWSTR s1, GXLPCWSTR s2, int n)
//{
//  //int res;
//
//  n = min(min(n, GXSTRLEN(s1)), GXSTRLEN(s2));
//  //res = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, s1, n, s2, n);
//  //return res ? res - sizeof(GXWCHAR) : res;
//  return GXSTRNCMPI(s1, s2, n);
//}

/******** Debugging functions *****************************************/

static inline GXLPCSTR debugtext_t(GXLPCWSTR text, GXBOOL isW)
{
  if (text == GXLPSTR_TEXTCALLBACKW) return "(callback)";
  return isW ? (GXLPCSTR)debugstr_w(text) : debugstr_a((GXLPCSTR)text);
}

static inline GXLPCSTR debugtext_tn(GXLPCWSTR text, GXBOOL isW, GXINT n)
{
  if (text == GXLPSTR_TEXTCALLBACKW) return "(callback)";
  n = min(textlenT(text, isW), n);
  return isW ? debugstr_wn((GXLPCSTR)text, n) : debugstr_an((GXLPCSTR)text, n);
}

static char* debug_getbuf()
{
  static int index = 0;
  static char buffers[DEBUG_BUFFERS][DEBUG_BUFFER_SIZE];
  return buffers[index++ % DEBUG_BUFFERS];
}

static inline const char* debugrange(const RANGE *lprng)
{
  if (!lprng) return "(null)";
  return wine_dbg_sprintf("[%d, %d]", lprng->lower, lprng->upper);
}

static const char* debugscrollinfo(const GXSCROLLINFO *pScrollInfo)
{
  char* buf = debug_getbuf(), *text = buf;
  int len, size = DEBUG_BUFFER_SIZE;

  if (pScrollInfo == NULL) return "(null)";
  len = snprintf(buf, size, "{cbSize=%d, ", pScrollInfo->cbSize);
  if (len == -1) goto end; buf += len; size -= len;
  if (pScrollInfo->fMask & GXSIF_RANGE)
    len = snprintf(buf, size, "nMin=%d, nMax=%d, ", pScrollInfo->nMin, pScrollInfo->nMax);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (pScrollInfo->fMask & GXSIF_PAGE)
    len = snprintf(buf, size, "nPage=%u, ", pScrollInfo->nPage);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (pScrollInfo->fMask & GXSIF_POS)
    len = snprintf(buf, size, "nPos=%d, ", pScrollInfo->nPos);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (pScrollInfo->fMask & GXSIF_TRACKPOS)
    len = snprintf(buf, size, "nTrackPos=%d, ", pScrollInfo->nTrackPos);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  goto undo;
end:
  buf = text + strlen(text);
undo:
  if (buf - text > 2) { buf[-2] = '}'; buf[-1] = 0; }
  return text;
} 

static const char* debugnmlistview(const GXNMLISTVIEW *plvnm)
{
  if (!plvnm) return "(null)";
  return wine_dbg_sprintf("iItem=%d, iSubItem=%d, uNewState=0x%x,"
    " uOldState=0x%x, uChanged=0x%x, ptAction=%s, lParam=%ld",
    plvnm->iItem, plvnm->iSubItem, plvnm->uNewState, plvnm->uOldState,
    plvnm->uChanged, wine_dbgstr_point(&plvnm->ptAction), plvnm->lParam);
}

static const char* debuglvitem_t(const GXLVITEMW *lpLVItem, GXBOOL isW)
{
  char* buf = debug_getbuf(), *text = buf;
  int len, size = DEBUG_BUFFER_SIZE;

  if (lpLVItem == NULL) return "(null)";
  len = snprintf(buf, size, "{iItem=%d, iSubItem=%d, ", lpLVItem->iItem, lpLVItem->iSubItem);
  if (len == -1) goto end; buf += len; size -= len;
  if (lpLVItem->mask & GXLVIF_STATE)
    len = snprintf(buf, size, "state=%x, stateMask=%x, ", lpLVItem->state, lpLVItem->stateMask);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpLVItem->mask & GXLVIF_TEXT)
    len = snprintf(buf, size, "pszText=%s, cchTextMax=%d, ", debugtext_tn(lpLVItem->pszText, isW, 80), lpLVItem->cchTextMax);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpLVItem->mask & GXLVIF_IMAGE)
    len = snprintf(buf, size, "iImage=%d, ", lpLVItem->iImage);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpLVItem->mask & GXLVIF_PARAM)
    len = snprintf(buf, size, "lParam=%lx, ", lpLVItem->lParam);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpLVItem->mask & GXLVIF_INDENT)
    len = snprintf(buf, size, "iIndent=%d, ", lpLVItem->iIndent);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  goto undo;
end:
  buf = text + strlen(text);
undo:
  if (buf - text > 2) { buf[-2] = '}'; buf[-1] = 0; }
  return text;
}

static const char* debuglvcolumn_t(const GXLVCOLUMNW *lpColumn, GXBOOL isW)
{
  char* buf = debug_getbuf(), *text = buf;
  int len, size = DEBUG_BUFFER_SIZE;

  if (lpColumn == NULL) return "(null)";
  len = snprintf(buf, size, "{");
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_SUBITEM)
    len = snprintf(buf, size, "iSubItem=%d, ",  lpColumn->iSubItem);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_FMT)
    len = snprintf(buf, size, "fmt=%x, ", lpColumn->fmt);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_WIDTH)
    len = snprintf(buf, size, "cx=%d, ", lpColumn->cx);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_TEXT)
    len = snprintf(buf, size, "pszText=%s, cchTextMax=%d, ", debugtext_tn(lpColumn->pszText, isW, 80), lpColumn->cchTextMax);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_IMAGE)
    len = snprintf(buf, size, "iImage=%d, ", lpColumn->iImage);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  if (lpColumn->mask & LVCF_ORDER)
    len = snprintf(buf, size, "iOrder=%d, ", lpColumn->iOrder);
  else len = 0;
  if (len == -1) goto end; buf += len; size -= len;
  goto undo;
end:
  buf = text + strlen(text);
undo:
  if (buf - text > 2) { buf[-2] = '}'; buf[-1] = 0; }
  return text;
}

static const char* debuglvhittestinfo(const GXLVHITTESTINFO *lpht)
{
  if (!lpht) return "(null)";

  return wine_dbg_sprintf("{pt=%s, flags=0x%x, iItem=%d, iSubItem=%d}",
    wine_dbgstr_point(&lpht->pt), lpht->flags, lpht->iItem, lpht->iSubItem);
}

/* Return the corresponding text for a given scroll value */
static inline GXLPCSTR debugscrollcode(int nScrollCode)
{
  switch(nScrollCode)
  {
  case GXSB_LINELEFT: return "GXSB_LINELEFT";
  case GXSB_LINERIGHT: return "GXSB_LINERIGHT";
  case GXSB_PAGELEFT: return "GXSB_PAGELEFT";
  case GXSB_PAGERIGHT: return "GXSB_PAGERIGHT";
  case GXSB_THUMBPOSITION: return "GXSB_THUMBPOSITION";
  case GXSB_THUMBTRACK: return "GXSB_THUMBTRACK";
  case GXSB_ENDSCROLL: return "GXSB_ENDSCROLL";
  case SB_INTERNAL: return "SB_INTERNAL";
  default: return "unknown";
  }
}


/******** Notification functions i************************************/

static GXLRESULT notify_forward_header(const LISTVIEW_INFO *infoPtr, const GXNMHEADERW *lpnmh)
{
  return gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY,
    (GXWPARAM)lpnmh->hdr.idFrom, (GXLPARAM)lpnmh);
}

static GXLRESULT notify_hdr(const LISTVIEW_INFO *infoPtr, GXINT code, GXLPNMHDR pnmh)
{
  GXLRESULT result;

  TRACE("(code=%d)\n", code);

  pnmh->hwndFrom = infoPtr->hwndSelf;
  pnmh->idFrom = gxGetWindowLongPtrW(infoPtr->hwndSelf, GXGWLP_ID);
  pnmh->code = code;
  result = gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFY, pnmh->idFrom, (GXLPARAM)pnmh);

  TRACE("  <= %ld\n", result);

  return result;
}

static inline GXBOOL notify(const LISTVIEW_INFO *infoPtr, GXINT code)
{
  GXNMHDR nmh;
  GXHWND hwnd = infoPtr->hwndSelf;
  notify_hdr(infoPtr, code, &nmh);
  return gxIsWindow(hwnd);
}

static inline void notify_itemactivate(const LISTVIEW_INFO *infoPtr, const GXLVHITTESTINFO *htInfo)
{
  GXNMITEMACTIVATE nmia;
  GXLVITEMW item;

  if (htInfo) {
    nmia.uNewState = 0;
    nmia.uOldState = 0;
    nmia.uChanged  = 0;
    nmia.uKeyFlags = 0;

    item.mask = GXLVIF_PARAM|GXLVIF_STATE;
    item.iItem = htInfo->iItem;
    item.iSubItem = 0;
    if (LISTVIEW_GetItemT(infoPtr, &item, TRUE)) {
      nmia.lParam = item.lParam;
      nmia.uOldState = item.state;
      nmia.uNewState = item.state | LVIS_ACTIVATING;
      nmia.uChanged  = GXLVIF_STATE;
    }

    nmia.iItem = htInfo->iItem;
    nmia.iSubItem = htInfo->iSubItem;
    nmia.ptAction = htInfo->pt;     

    if (gxGetKeyState(GXVK_SHIFT) & 0x8000) nmia.uKeyFlags |= LVKF_SHIFT;
    if (gxGetKeyState(GXVK_CONTROL) & 0x8000) nmia.uKeyFlags |= LVKF_CONTROL;
    if (gxGetKeyState(GXVK_MENU) & 0x8000) nmia.uKeyFlags |= LVKF_ALT;
  }
  notify_hdr(infoPtr, LVN_ITEMACTIVATE, (GXLPNMHDR)&nmia);
}

static inline GXLRESULT notify_listview(const LISTVIEW_INFO *infoPtr, GXINT code, GXLPNMLISTVIEW plvnm)
{
  TRACE("(code=%d, plvnm=%s)\n", code, debugnmlistview(plvnm));
  return notify_hdr(infoPtr, code, (GXLPNMHDR)plvnm);
}

static GXBOOL notify_click(const LISTVIEW_INFO *infoPtr, GXINT code, const GXLVHITTESTINFO *lvht)
{
  GXNMLISTVIEW nmlv;
  GXLVITEMW item;
  GXHWND hwnd = infoPtr->hwndSelf;

  TRACE("code=%d, lvht=%s\n", code, debuglvhittestinfo(lvht)); 
  ZeroMemory(&nmlv, sizeof(nmlv));
  nmlv.iItem = lvht->iItem;
  nmlv.iSubItem = lvht->iSubItem;
  nmlv.ptAction = lvht->pt;
  item.mask = GXLVIF_PARAM;
  item.iItem = lvht->iItem;
  item.iSubItem = 0;
  if (LISTVIEW_GetItemT(infoPtr, &item, TRUE)) nmlv.lParam = item.lParam;
  notify_listview(infoPtr, code, &nmlv);
  return gxIsWindow(hwnd);
}

static GXBOOL notify_deleteitem(const LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXNMLISTVIEW nmlv;
  GXLVITEMW item;
  GXHWND hwnd = infoPtr->hwndSelf;

  ZeroMemory(&nmlv, sizeof (GXNMLISTVIEW));
  nmlv.iItem = nItem;
  item.mask = GXLVIF_PARAM;
  item.iItem = nItem;
  item.iSubItem = 0;
  if (LISTVIEW_GetItemT(infoPtr, &item, TRUE)) nmlv.lParam = item.lParam;
  notify_listview(infoPtr, LVN_DELETEITEM, &nmlv);
  return gxIsWindow(hwnd);
}

static int get_ansi_notification(GXUINT unicodeNotificationCode)
{
  switch (unicodeNotificationCode)
  {
  case LVN_BEGINLABELEDITW: return LVN_BEGINLABELEDITA;
  case LVN_ENDLABELEDITW: return LVN_ENDLABELEDITA;
  case LVN_GETDISPINFOW: return LVN_GETDISPINFOA;
  case LVN_SETDISPINFOW: return LVN_SETDISPINFOA;
  case LVN_ODFINDITEMW: return LVN_ODFINDITEMA;
  case LVN_GETINFOTIPW: return LVN_GETINFOTIPA;
  }
  ERR("unknown notification %x\n", unicodeNotificationCode);
  WEAssert(FALSE);
  return 0;
}

/*
Send notification. depends on dispinfoW having same
structure as dispinfoA.
infoPtr : listview struct
notificationCode : *Unicode* notification code
pdi : dispinfo structure (can be unicode or ansi)
isW : TRUE if dispinfo is Unicode
*/
static GXBOOL notify_dispinfoT(const LISTVIEW_INFO *infoPtr, GXUINT notificationCode, LPGXNMLVDISPINFOW pdi, GXBOOL isW)
{
  GXBOOL bResult = FALSE;
  GXBOOL convertToAnsi = FALSE, convertToUnicode = FALSE;
  GXINT cchTempBufMax = 0, savCchTextMax = 0;
  GXUINT realNotifCode;
  GXLPWSTR pszTempBuf = NULL, savPszText = NULL;

  if ((pdi->item.mask & GXLVIF_TEXT) && is_textT(pdi->item.pszText, isW))
  {
    convertToAnsi = (isW && infoPtr->notifyFormat == GXNFR_ANSI);
    convertToUnicode = (!isW && infoPtr->notifyFormat == GXNFR_UNICODE);
  }

  if (convertToAnsi || convertToUnicode)
  {
    if (notificationCode != LVN_GETDISPINFOW)
    {
      cchTempBufMax = convertToUnicode ?
        gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPCSTR)pdi->item.pszText, -1, NULL, 0):
      gxWideCharToMultiByte(GXCP_ACP, 0, pdi->item.pszText, -1, NULL, 0, NULL, NULL);
    }
    else
    {
      cchTempBufMax = pdi->item.cchTextMax;
      *pdi->item.pszText = 0; /* make sure we don't process garbage */
    }

    pszTempBuf = (GXLPWSTR)Alloc( (convertToUnicode ? sizeof(GXWCHAR) : sizeof(GXCHAR)) * cchTempBufMax);
    if (!pszTempBuf) return FALSE;

    if (convertToUnicode)
      gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPCSTR)pdi->item.pszText, -1,
      pszTempBuf, cchTempBufMax);
    else
      gxWideCharToMultiByte(GXCP_ACP, 0, pdi->item.pszText, -1, (GXLPSTR) pszTempBuf,
      cchTempBufMax, NULL, NULL);

    savCchTextMax = pdi->item.cchTextMax;
    savPszText = pdi->item.pszText;
    pdi->item.pszText = pszTempBuf;
    pdi->item.cchTextMax = cchTempBufMax;
  }

  if (infoPtr->notifyFormat == GXNFR_ANSI)
    realNotifCode = get_ansi_notification(notificationCode);
  else
    realNotifCode = notificationCode;
  TRACE(" pdi->item=%s\n", debuglvitem_t(&pdi->item, infoPtr->notifyFormat != GXNFR_ANSI));
  bResult = notify_hdr(infoPtr, realNotifCode, &pdi->hdr);

  if (convertToUnicode || convertToAnsi)
  {
    if (convertToUnicode) /* note : pointer can be changed by app ! */
      gxWideCharToMultiByte(GXCP_ACP, 0, pdi->item.pszText, -1, (GXLPSTR) savPszText,
      savCchTextMax, NULL, NULL);
    else
      gxMultiByteToWideChar(GXCP_ACP, 0, (GXLPSTR) pdi->item.pszText, -1,
      savPszText, savCchTextMax);
    pdi->item.pszText = savPszText; /* restores our buffer */
    pdi->item.cchTextMax = savCchTextMax;
    Free (pszTempBuf);
  }
  return bResult;
}

static void customdraw_fill(GXNMLVCUSTOMDRAW *lpnmlvcd, const LISTVIEW_INFO *infoPtr, GXHDC hdc,
              const GXRECT *rcBounds, const GXLVITEMW *lplvItem)
{
  ZeroMemory(lpnmlvcd, sizeof(GXNMLVCUSTOMDRAW));
  lpnmlvcd->nmcd.hdc = hdc;
  lpnmlvcd->nmcd.rc = *rcBounds;
  lpnmlvcd->clrTextBk = infoPtr->clrTextBk;
  lpnmlvcd->clrText   = infoPtr->clrText;
  if (!lplvItem) return;
  lpnmlvcd->nmcd.dwItemSpec = lplvItem->iItem + 1;
  lpnmlvcd->iSubItem = lplvItem->iSubItem;
  if (lplvItem->state & LVIS_SELECTED) lpnmlvcd->nmcd.uItemState |= CDIS_SELECTED;
  if (lplvItem->state & LVIS_FOCUSED) lpnmlvcd->nmcd.uItemState |= CDIS_FOCUS;
  if (lplvItem->iItem == infoPtr->nHotItem) lpnmlvcd->nmcd.uItemState |= CDIS_HOT;
  lpnmlvcd->nmcd.lItemlParam = lplvItem->lParam;
}

static inline GXDWORD notify_customdraw (const LISTVIEW_INFO *infoPtr, GXDWORD dwDrawStage, GXNMLVCUSTOMDRAW *lpnmlvcd)
{
  GXBOOL isForItem = (lpnmlvcd->nmcd.dwItemSpec != 0);
  GXDWORD result;

  lpnmlvcd->nmcd.dwDrawStage = dwDrawStage;
  if (isForItem) lpnmlvcd->nmcd.dwDrawStage |= CDDS_ITEM; 
  if (lpnmlvcd->iSubItem) lpnmlvcd->nmcd.dwDrawStage |= CDDS_SUBITEM;
  if (isForItem) lpnmlvcd->nmcd.dwItemSpec--;
  result = notify_hdr(infoPtr, GXNM_CUSTOMDRAW, &lpnmlvcd->nmcd.hdr);
  if (isForItem) lpnmlvcd->nmcd.dwItemSpec++;
  return result;
}

static void prepaint_setup (const LISTVIEW_INFO *infoPtr, GXHDC hdc, GXNMLVCUSTOMDRAW *lpnmlvcd, GXBOOL SubItem)
{
  if (lpnmlvcd->clrTextBk == GXCLR_DEFAULT)
    lpnmlvcd->clrTextBk = comctl32_color.clrWindow;
  if (lpnmlvcd->clrText == GXCLR_DEFAULT)
    lpnmlvcd->clrText = comctl32_color.clrWindowText;

  /* apparently, for selected items, we have to override the returned values */
  if (!SubItem)
  {
    if (lpnmlvcd->nmcd.uItemState & CDIS_SELECTED)
    {
      if (infoPtr->bFocus)
      {
        lpnmlvcd->clrTextBk = comctl32_color.clrHighlight;
        lpnmlvcd->clrText   = comctl32_color.clrHighlightText;
      }
      else if (infoPtr->dwStyle & LVS_SHOWSELALWAYS)
      {
        lpnmlvcd->clrTextBk = comctl32_color.clr3dFace;
        lpnmlvcd->clrText   = comctl32_color.clrBtnText;
      }
    }
  }

  /* Set the text attributes */
  if (lpnmlvcd->clrTextBk != GXCLR_NONE)
  {
    gxSetBkMode(hdc, GXOPAQUE);
    gxSetBkColor(hdc,lpnmlvcd->clrTextBk);
  }
  else
    gxSetBkMode(hdc, GXTRANSPARENT);
  gxSetTextColor(hdc, lpnmlvcd->clrText);
}

static inline GXDWORD notify_postpaint (const LISTVIEW_INFO *infoPtr, GXNMLVCUSTOMDRAW *lpnmlvcd)
{
  return notify_customdraw(infoPtr, CDDS_POSTPAINT, lpnmlvcd);
}

/******** Item iterator functions **********************************/

static RANGES ranges_create(int count);
static void ranges_destroy(RANGES ranges);
static GXBOOL ranges_add(RANGES ranges, RANGE range);
static GXBOOL ranges_del(RANGES ranges, RANGE range);
static void ranges_dump(RANGES ranges);

static inline GXBOOL ranges_additem(RANGES ranges, GXINT nItem)
{
  RANGE range = { nItem, nItem + 1 };

  return ranges_add(ranges, range);
}

static inline GXBOOL ranges_delitem(RANGES ranges, GXINT nItem)
{
  RANGE range = { nItem, nItem + 1 };

  return ranges_del(ranges, range);
}

/***
* ITERATOR DOCUMENTATION
*
* The iterator functions allow for easy, and convenient iteration
* over items of interest in the list. Typically, you create a
* iterator, use it, and destroy it, as such:
*   ITERATOR i;
*
*   iterator_xxxitems(&i, ...);
*   while (iterator_{prev,next}(&i)
*   {
*       //code which uses i.nItem
*   }
*   iterator_destroy(&i);
*
*   where xxx is either: framed, or visible.
* Note that it is important that the code destroys the iterator
* after it's done with it, as the creation of the iterator may
* allocate memory, which thus needs to be freed.
* 
* You can iterate both forwards, and backwards through the list,
* by using iterator_next or iterator_prev respectively.
* 
* Lower numbered items are draw on top of higher number items in
* LVS_ICON, and LVS_SMALLICON (which are the only modes where
* items may overlap). So, to test items, you should use
*    iterator_next
* which lists the items top to bottom (in Z-order).
* For drawing items, you should use
*    iterator_prev
* which lists the items bottom to top (in Z-order).
* If you keep iterating over the items after the end-of-items
* marker (-1) is returned, the iterator will start from the
* beginning. Typically, you don't need to test for -1,
* because iterator_{next,prev} will return TRUE if more items
* are to be iterated over, or FALSE otherwise.
*
* Note: the iterator is defined to be bidirectional. That is,
*       any number of prev followed by any number of next, or
*       five versa, should leave the iterator at the same item:
*           prev * n, next * n = next * n, prev * n
*
* The iterator has a notion of an out-of-order, special item,
* which sits at the start of the list. This is used in
* LVS_ICON, and LVS_SMALLICON mode to handle the focused item,
* which needs to be first, as it may overlap other items.
*           
* The code is a bit messy because we have:
*   - a special item to deal with
*   - simple range, or composite range
*   - empty range.
* If you find bugs, or want to add features, please make sure you
* always check/modify *both* iterator_prev, and iterator_next.
*/

/****
* This function iterates through the items in increasing order,
* but prefixed by the special item, then -1. That is:
*    special, 1, 2, 3, ..., n, -1.
* Each item is listed only once.
*/
static inline GXBOOL iterator_next(ITERATOR* i)
{
  if (i->nItem == -1)
  {
    i->nItem = i->nSpecial;
    if (i->nItem != -1) return TRUE;
  }
  if (i->nItem == i->nSpecial)
  {
    if (i->ranges) i->index = 0;
    goto pickarange;
  }

  i->nItem++;
testitem:
  if (i->nItem == i->nSpecial) i->nItem++;
  if (i->nItem < i->range.upper) return TRUE;

pickarange:
  if (i->ranges)
  {
    if (i->index < gxDPA_GetPtrCount(i->ranges->hdpa))
      i->range = *(RANGE*)gxDPA_GetPtr(i->ranges->hdpa, i->index++);
    else goto end;
  }
  else if (i->nItem >= i->range.upper) goto end;

  i->nItem = i->range.lower;
  if (i->nItem >= 0) goto testitem;
end:
  i->nItem = -1;
  return FALSE;
}

/****
* This function iterates through the items in decreasing order,
* followed by the special item, then -1. That is:
*    n, n-1, ..., 3, 2, 1, special, -1.
* Each item is listed only once.
*/
static inline GXBOOL iterator_prev(ITERATOR* i)
{
  GXBOOL start = FALSE;

  if (i->nItem == -1)
  {
    start = TRUE;
    if (i->ranges) i->index = gxDPA_GetPtrCount(i->ranges->hdpa);
    goto pickarange;
  }
  if (i->nItem == i->nSpecial)
  {
    i->nItem = -1;
    return FALSE;
  }

testitem:
  i->nItem--;
  if (i->nItem == i->nSpecial) i->nItem--;
  if (i->nItem >= i->range.lower) return TRUE;

pickarange:
  if (i->ranges)
  {
    if (i->index > 0)
      i->range = *(RANGE*)gxDPA_GetPtr(i->ranges->hdpa, --i->index);
    else goto end;
  }
  else if (!start && i->nItem < i->range.lower) goto end;

  i->nItem = i->range.upper;
  if (i->nItem > 0) goto testitem;
end:
  return (i->nItem = i->nSpecial) != -1;
}

static RANGE iterator_range(const ITERATOR *i)
{
  RANGE range;

  if (!i->ranges) return i->range;

  if (gxDPA_GetPtrCount(i->ranges->hdpa) > 0)
  {
    range.lower = (*(RANGE*)gxDPA_GetPtr(i->ranges->hdpa, 0)).lower;
    range.upper = (*(RANGE*)gxDPA_GetPtr(i->ranges->hdpa, gxDPA_GetPtrCount(i->ranges->hdpa) - 1)).upper;
  }
  else range.lower = range.upper = 0;

  return range;
}

/***
* Releases resources associated with this ierator.
*/
static inline void iterator_destroy(const ITERATOR *i)
{
  ranges_destroy(i->ranges);
}

/***
* Create an empty iterator.
*/
static inline GXBOOL iterator_empty(ITERATOR* i)
{
  ZeroMemory(i, sizeof(*i));
  i->nItem = i->nSpecial = i->range.lower = i->range.upper = -1;
  return TRUE;
}

/***
* Create an iterator over a range.
*/
static inline GXBOOL iterator_rangeitems(ITERATOR* i, RANGE range)
{
  iterator_empty(i);
  i->range = range;
  return TRUE;
}

/***
* Create an iterator over a bunch of ranges.
* Please note that the iterator will take ownership of the ranges,
* and will free them upon destruction.
*/
static inline GXBOOL iterator_rangesitems(ITERATOR* i, RANGES ranges)
{
  iterator_empty(i);
  i->ranges = ranges;
  return TRUE;
}

/***
* Creates an iterator over the items which intersect lprc.
*/
static GXBOOL iterator_frameditems(ITERATOR* i, const LISTVIEW_INFO* infoPtr, const GXRECT *lprc)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXRECT frame = *lprc, rcItem, rcTemp;
  GXPOINT Origin;

  /* in case we fail, we want to return an empty iterator */
  if (!iterator_empty(i)) return FALSE;

  LISTVIEW_GetOrigin(infoPtr, &Origin);

  TRACE("(lprc=%s)\n", wine_dbgstr_rect(lprc));
  gxOffsetRect(&frame, -Origin.x, -Origin.y);

  if (uView == LVS_ICON || uView == LVS_SMALLICON)
  {
    GXINT nItem;

    if (uView == LVS_ICON && infoPtr->nFocusedItem != -1)
    {
      LISTVIEW_GetItemBox(infoPtr, infoPtr->nFocusedItem, &rcItem);
      if (gxIntersectRect(&rcTemp, &rcItem, lprc))
        i->nSpecial = infoPtr->nFocusedItem;
    }
    if (!(iterator_rangesitems(i, ranges_create(50)))) return FALSE;
    /* to do better here, we need to have PosX, and PosY sorted */
    TRACE("building icon ranges:\n");
    for (nItem = 0; nItem < infoPtr->nItemCount; nItem++)
    {
      rcItem.left = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosX, nItem);
      rcItem.top = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosY, nItem);
      rcItem.right = rcItem.left + infoPtr->nItemWidth;
      rcItem.bottom = rcItem.top + infoPtr->nItemHeight;
      if (gxIntersectRect(&rcTemp, &rcItem, &frame))
        ranges_additem(i->ranges, nItem);
    }
    return TRUE;
  }
  else if (uView == LVS_REPORT)
  {
    RANGE range;

    if (frame.left >= infoPtr->nItemWidth) return TRUE;
    if (frame.top >= infoPtr->nItemHeight * infoPtr->nItemCount) return TRUE;

    range.lower = max(frame.top / infoPtr->nItemHeight, 0);
    range.upper = min((frame.bottom - 1) / infoPtr->nItemHeight, infoPtr->nItemCount - 1) + 1;
    if (range.upper <= range.lower) return TRUE;
    if (!iterator_rangeitems(i, range)) return FALSE;
    TRACE("    report=%s\n", debugrange(&i->range));
  }
  else
  {
    GXINT nPerCol = max((infoPtr->rcList.bottom - infoPtr->rcList.top) / infoPtr->nItemHeight, 1);
    GXINT nFirstRow = max(frame.top / infoPtr->nItemHeight, 0);
    GXINT nLastRow = min((frame.bottom - 1) / infoPtr->nItemHeight, nPerCol - 1);
    GXINT nFirstCol = max(frame.left / infoPtr->nItemWidth, 0);
    GXINT nLastCol = min((frame.right - 1) / infoPtr->nItemWidth, (infoPtr->nItemCount + nPerCol - 1) / nPerCol);
    GXINT lower = nFirstCol * nPerCol + nFirstRow;
    RANGE item_range;
    GXINT nCol;

    TRACE("nPerCol=%d, nFirstRow=%d, nLastRow=%d, nFirstCol=%d, nLastCol=%d, lower=%d\n",
      nPerCol, nFirstRow, nLastRow, nFirstCol, nLastCol, lower);

    if (nLastCol < nFirstCol || nLastRow < nFirstRow) return TRUE;

    if (!(iterator_rangesitems(i, ranges_create(nLastCol - nFirstCol + 1)))) return FALSE;
    TRACE("building list ranges:\n");
    for (nCol = nFirstCol; nCol <= nLastCol; nCol++)
    {
      item_range.lower = nCol * nPerCol + nFirstRow;
      if(item_range.lower >= infoPtr->nItemCount) break;
      item_range.upper = min(nCol * nPerCol + nLastRow + 1, infoPtr->nItemCount);
      TRACE("   list=%s\n", debugrange(&item_range));
      ranges_add(i->ranges, item_range);
    }
  }

  return TRUE;
}

/***
* Creates an iterator over the items which intersect the visible region of hdc.
*/
static GXBOOL iterator_visibleitems(ITERATOR *i, const LISTVIEW_INFO *infoPtr, GXHDC  hdc)
{
  GXPOINT Origin, Position;
  GXRECT rcItem, rcClip;
  GXINT rgntype;

  rgntype = gxGetClipBox(hdc, &rcClip);
  if (rgntype == NULLREGION) return iterator_empty(i);
  if (!iterator_frameditems(i, infoPtr, &rcClip)) return FALSE;
  if (rgntype == GXSIMPLEREGION) return TRUE;

  /* first deal with the special item */
  if (i->nSpecial != -1)
  {
    LISTVIEW_GetItemBox(infoPtr, i->nSpecial, &rcItem);
    if (!gxRectVisible(hdc, &rcItem)) i->nSpecial = -1;
  }

  /* if we can't deal with the region, we'll just go with the simple range */
  LISTVIEW_GetOrigin(infoPtr, &Origin);
  TRACE("building visible range:\n");
  if (!i->ranges && i->range.lower < i->range.upper)
  {
    if (!(i->ranges = ranges_create(50))) return TRUE;
    if (!ranges_add(i->ranges, i->range))
    {
      ranges_destroy(i->ranges);
      i->ranges = 0;
      return TRUE;
    }
  }

  /* now delete the invisible items from the list */
  while(iterator_next(i))
  {
    LISTVIEW_GetItemOrigin(infoPtr, i->nItem, &Position);
    rcItem.left = Position.x + Origin.x;
    rcItem.top = Position.y + Origin.y;
    rcItem.right = rcItem.left + infoPtr->nItemWidth;
    rcItem.bottom = rcItem.top + infoPtr->nItemHeight;
    if (!gxRectVisible(hdc, &rcItem))
      ranges_delitem(i->ranges, i->nItem);
  }
  /* the iterator should restart on the next iterator_next */
  TRACE("done\n");

  return TRUE;
}

/******** Misc helper functions ************************************/

static inline GXLRESULT CallWindowProcT(GXWNDPROC proc, GXHWND hwnd, GXUINT uMsg,
                    GXWPARAM wParam, GXLPARAM lParam, GXBOOL isW)
{
  if (isW) return gxCallWindowProcW(proc, hwnd, uMsg, wParam, lParam);
  else return gxCallWindowProcA(proc, hwnd, uMsg, wParam, lParam);
}

static inline GXBOOL is_autoarrange(const LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;

  return ((infoPtr->dwStyle & LVS_AUTOARRANGE) || infoPtr->bAutoarrange) &&
    (uView == LVS_ICON || uView == LVS_SMALLICON);
}

static void toggle_checkbox_state(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXDWORD state = STATEIMAGEINDEX(LISTVIEW_GetItemState(infoPtr, nItem, LVIS_STATEIMAGEMASK));
  if(state == 1 || state == 2)
  {
    GXLVITEMW lvitem;
    state ^= 3;
    lvitem.state = INDEXTOSTATEIMAGEMASK(state);
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    LISTVIEW_SetItemState(infoPtr, nItem, &lvitem);
  }
}

/******** Internal API functions ************************************/

static inline COLUMN_INFO * LISTVIEW_GetColumnInfo(const LISTVIEW_INFO *infoPtr, GXINT nSubItem)
{
  static COLUMN_INFO mainItem;

  if (nSubItem == 0 && gxDPA_GetPtrCount(infoPtr->hdpaColumns) == 0) return &mainItem;
  WEAssert (nSubItem >= 0 && nSubItem < gxDPA_GetPtrCount(infoPtr->hdpaColumns));
  return (COLUMN_INFO *)gxDPA_GetPtr(infoPtr->hdpaColumns, nSubItem);
}

static inline void LISTVIEW_GetHeaderRect(const LISTVIEW_INFO *infoPtr, GXINT nSubItem, LPGXRECT lprc)
{
  *lprc = LISTVIEW_GetColumnInfo(infoPtr, nSubItem)->rcHeader;
}

static inline GXBOOL LISTVIEW_GetItemW(const LISTVIEW_INFO *infoPtr, GXLPLVITEMW lpLVItem)
{
  return LISTVIEW_GetItemT(infoPtr, lpLVItem, TRUE);
}

/* Listview invalidation functions: use _only_ these functions to invalidate */

static inline GXBOOL is_redrawing(const LISTVIEW_INFO *infoPtr)
{
  return infoPtr->bRedraw;
}

static inline void LISTVIEW_InvalidateRect(const LISTVIEW_INFO *infoPtr, const GXRECT* rect)
{
  if(!is_redrawing(infoPtr)) return; 
  TRACE(" invalidating rect=%s\n", wine_dbgstr_rect(rect));
  gxInvalidateRect(infoPtr->hwndSelf, rect, TRUE);
}

static inline void LISTVIEW_InvalidateItem(const LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXRECT rcBox;

  if(!is_redrawing(infoPtr)) return; 
  LISTVIEW_GetItemBox(infoPtr, nItem, &rcBox);
  LISTVIEW_InvalidateRect(infoPtr, &rcBox);
}

static inline void LISTVIEW_InvalidateSubItem(const LISTVIEW_INFO *infoPtr, GXINT nItem, GXINT nSubItem)
{
  GXPOINT Origin, Position;
  GXRECT rcBox;

  if(!is_redrawing(infoPtr)) return; 
  WEAssert ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_REPORT);
  LISTVIEW_GetOrigin(infoPtr, &Origin);
  LISTVIEW_GetItemOrigin(infoPtr, nItem, &Position);
  LISTVIEW_GetHeaderRect(infoPtr, nSubItem, &rcBox);
  rcBox.top = 0;
  rcBox.bottom = infoPtr->nItemHeight;
  gxOffsetRect(&rcBox, Origin.x + Position.x, Origin.y + Position.y);
  LISTVIEW_InvalidateRect(infoPtr, &rcBox);
}

static inline void LISTVIEW_InvalidateList(const LISTVIEW_INFO *infoPtr)
{
  LISTVIEW_InvalidateRect(infoPtr, NULL);
}

static inline void LISTVIEW_InvalidateColumn(const LISTVIEW_INFO *infoPtr, GXINT nColumn)
{
  GXRECT rcCol;

  if(!is_redrawing(infoPtr)) return; 
  LISTVIEW_GetHeaderRect(infoPtr, nColumn, &rcCol);
  rcCol.top = infoPtr->rcList.top;
  rcCol.bottom = infoPtr->rcList.bottom;
  LISTVIEW_InvalidateRect(infoPtr, &rcCol);
}

/***
* DESCRIPTION:
* Retrieves the number of items that can fit vertically in the client area.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Number of items per row.
*/
static inline GXINT LISTVIEW_GetCountPerRow(const LISTVIEW_INFO *infoPtr)
{
  GXINT nListWidth = infoPtr->rcList.right - infoPtr->rcList.left;

  return max(nListWidth/infoPtr->nItemWidth, 1);
}

/***
* DESCRIPTION:
* Retrieves the number of items that can fit horizontally in the client
* area.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Number of items per column.
*/
static inline GXINT LISTVIEW_GetCountPerColumn(const LISTVIEW_INFO *infoPtr)
{
  GXINT nListHeight = infoPtr->rcList.bottom - infoPtr->rcList.top;

  return max(nListHeight / infoPtr->nItemHeight, 1);
}


/*************************************************************************
*    LISTVIEW_ProcessLetterKeys
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
* PARAMETERS
*   [I] hwnd : handle to the window
*   [I] charCode : the character code, the actual character
*   [I] keyData : key data
*
* RETURNS
*
*  Zero.
*
* BUGS
*
*  - The current implementation has a list of characters it will
*    accept and it ignores everything else. In particular it will
*    ignore accentuated characters which seems to match what
*    Windows does. But I'm not sure it makes sense to follow
*    Windows there.
*  - We don't sound a beep when the search fails.
*
* SEE ALSO
*
*  TREEVIEW_ProcessLetterKeys
*/
static GXINT LISTVIEW_ProcessLetterKeys(LISTVIEW_INFO *infoPtr, GXWPARAM charCode, GXLPARAM keyData)
{
  GXINT nItem;
  GXINT endidx,idx;
  GXLVITEMW item;
  GXWCHAR buffer[MAX_PATH];
  GXDWORD lastKeyPressTimestamp = infoPtr->lastKeyPressTimestamp;

  /* simple parameter checking */
  if (!charCode || !keyData) return 0;

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

  /* if there's one item or less, there is no where to go */
  if (infoPtr->nItemCount <= 1) return 0;

  /* update the search parameters */
  infoPtr->lastKeyPressTimestamp = gxGetTickCount();
  if (infoPtr->lastKeyPressTimestamp - lastKeyPressTimestamp < KEY_DELAY) {
    if (infoPtr->nSearchParamLength < MAX_PATH)
      infoPtr->szSearchParam[infoPtr->nSearchParamLength++]=charCode;
    if (infoPtr->charCode != charCode)
      infoPtr->charCode = charCode = 0;
  } else {
    infoPtr->charCode=charCode;
    infoPtr->szSearchParam[0]=charCode;
    infoPtr->nSearchParamLength=1;
    /* Redundant with the 1 char string */
    charCode=0;
  }

  /* and search from the current position */
  nItem=-1;
  if (infoPtr->nFocusedItem >= 0) {
    endidx=infoPtr->nFocusedItem;
    idx=endidx;
    /* if looking for single character match,
    * then we must always move forward
    */
    if (infoPtr->nSearchParamLength == 1)
      idx++;
  } else {
    endidx=infoPtr->nItemCount;
    idx=0;
  }
  do {
    if (idx == infoPtr->nItemCount) {
      if (endidx == infoPtr->nItemCount || endidx == 0)
        break;
      idx=0;
    }

    /* get item */
    item.mask = GXLVIF_TEXT;
    item.iItem = idx;
    item.iSubItem = 0;
    item.pszText = buffer;
    item.cchTextMax = MAX_PATH;
    if (!LISTVIEW_GetItemW(infoPtr, &item)) return 0;

    /* check for a match */
    if (lstrncmpiW(item.pszText,infoPtr->szSearchParam,infoPtr->nSearchParamLength) == 0) {
      nItem=idx;
      break;
    } else if ( (charCode != 0) && (nItem == -1) && (nItem != infoPtr->nFocusedItem) &&
      (lstrncmpiW(item.pszText,infoPtr->szSearchParam,1) == 0) ) {
        /* This would work but we must keep looking for a longer match */
        nItem=idx;
    }
    idx++;
  } while (idx != endidx);

  if (nItem != -1)
    LISTVIEW_KeySelection(infoPtr, nItem);

  return 0;
}

/*************************************************************************
* LISTVIEW_UpdateHeaderSize [Internal]
*
* Function to resize the header control
*
* PARAMS
* [I]  hwnd : handle to a window
* [I]  nNewScrollPos : scroll pos to set
*
* RETURNS
* None.
*/
static void LISTVIEW_UpdateHeaderSize(const LISTVIEW_INFO *infoPtr, GXINT nNewScrollPos)
{
  GXRECT winRect;
  GXPOINT point[2];

  TRACE("nNewScrollPos=%d\n", nNewScrollPos);

  gxGetWindowRect(infoPtr->hwndHeader, &winRect);
  point[0].x = winRect.left;
  point[0].y = winRect.top;
  point[1].x = winRect.right;
  point[1].y = winRect.bottom;

  gxMapWindowPoints(GXHWND_DESKTOP, infoPtr->hwndSelf, point, 2);
  point[0].x = -nNewScrollPos;
  point[1].x += nNewScrollPos;

  gxSetWindowPos(infoPtr->hwndHeader,0,
    point[0].x,point[0].y,point[1].x,point[1].y,
    (infoPtr->dwStyle & LVS_NOCOLUMNHEADER) ? GXSWP_HIDEWINDOW : GXSWP_SHOWWINDOW |
    GXSWP_NOZORDER | GXSWP_NOACTIVATE);
}

/***
* DESCRIPTION:
* Update the scrollbars. This functions should be called whenever
* the content, size or view changes.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* None
*/
static void LISTVIEW_UpdateScroll(const LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXSCROLLINFO horzInfo, vertInfo;
  GXINT dx, dy;

  if ((infoPtr->dwStyle & LVS_NOSCROLL) || !is_redrawing(infoPtr)) return;

  ZeroMemory(&horzInfo, sizeof(GXSCROLLINFO));
  horzInfo.cbSize = sizeof(GXSCROLLINFO);
  horzInfo.nPage = infoPtr->rcList.right - infoPtr->rcList.left;

  /* for now, we'll set info.nMax to the _count_, and adjust it later */
  if (uView == LVS_LIST)
  {
    GXINT nPerCol = LISTVIEW_GetCountPerColumn(infoPtr);
    horzInfo.nMax = (infoPtr->nItemCount + nPerCol - 1) / nPerCol;

    /* scroll by at least one column per page */
    if(horzInfo.nPage < infoPtr->nItemWidth)
      horzInfo.nPage = infoPtr->nItemWidth;

    horzInfo.nPage /= infoPtr->nItemWidth;
  }
  else if (uView == LVS_REPORT)
  {
    horzInfo.nMax = infoPtr->nItemWidth;
  }
  else /* LVS_ICON, or LVS_SMALLICON */
  {
    GXRECT rcView;

    if (LISTVIEW_GetViewRect(infoPtr, &rcView)) horzInfo.nMax = rcView.right - rcView.left;
  }

  horzInfo.fMask = GXSIF_RANGE | GXSIF_PAGE;
  horzInfo.nMax = max(horzInfo.nMax - 1, 0);
  dx = gxGetScrollPos(infoPtr->hwndSelf, GXSB_HORZ);
  dx -= gxSetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &horzInfo, TRUE);
  TRACE("horzInfo=%s\n", debugscrollinfo(&horzInfo));

  /* Setting the horizontal scroll can change the listview size
  * (and potentially everything else) so we need to recompute
  * everything again for the vertical scroll
  */

  ZeroMemory(&vertInfo, sizeof(GXSCROLLINFO));
  vertInfo.cbSize = sizeof(GXSCROLLINFO);
  vertInfo.nPage = infoPtr->rcList.bottom - infoPtr->rcList.top;

  if (uView == LVS_REPORT)
  {
    vertInfo.nMax = infoPtr->nItemCount;

    /* scroll by at least one page */
    if(vertInfo.nPage < infoPtr->nItemHeight)
      vertInfo.nPage = infoPtr->nItemHeight;

    vertInfo.nPage /= infoPtr->nItemHeight;
  }
  else if (uView != LVS_LIST) /* LVS_ICON, or LVS_SMALLICON */
  {
    GXRECT rcView;

    if (LISTVIEW_GetViewRect(infoPtr, &rcView)) vertInfo.nMax = rcView.bottom - rcView.top;
  }

  vertInfo.fMask = GXSIF_RANGE | GXSIF_PAGE;
  vertInfo.nMax = max(vertInfo.nMax - 1, 0);
  dy = gxGetScrollPos(infoPtr->hwndSelf, GXSB_VERT);
  dy -= gxSetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &vertInfo, TRUE);
  TRACE("vertInfo=%s\n", debugscrollinfo(&vertInfo));

  /* Change of the range may have changed the scroll pos. If so move the content */
  if (dx != 0 || dy != 0)
  {
    GXRECT listRect;
    listRect = infoPtr->rcList;
    gxScrollWindowEx(infoPtr->hwndSelf, dx, dy, &listRect, &listRect, 0, 0,
      GXSW_ERASE | GXSW_INVALIDATE);
  }

  /* Update the Header Control */
  if (uView == LVS_REPORT)
  {
    horzInfo.fMask = GXSIF_POS;
    gxGetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &horzInfo);
    LISTVIEW_UpdateHeaderSize(infoPtr, horzInfo.nPos);
  }
}


/***
* DESCRIPTION:
* Shows/hides the focus rectangle. 
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] fShow : TRUE to show the focus, FALSE to hide it.
*
* RETURN:
* None
*/
static void LISTVIEW_ShowFocusRect(const LISTVIEW_INFO *infoPtr, GXBOOL fShow)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXHDC hdc;

  TRACE("fShow=%d, nItem=%d\n", fShow, infoPtr->nFocusedItem);

  if (infoPtr->nFocusedItem < 0) return;

  /* we need some gymnastics in ICON mode to handle large items */
  if ( (infoPtr->dwStyle & LVS_TYPEMASK) == LVS_ICON )
  {
    GXRECT rcBox;

    LISTVIEW_GetItemBox(infoPtr, infoPtr->nFocusedItem, &rcBox); 
    if ((rcBox.bottom - rcBox.top) > infoPtr->nItemHeight)
    {
      LISTVIEW_InvalidateRect(infoPtr, &rcBox);
      return;
    }
  }

  if (!(hdc = gxGetDC(infoPtr->hwndSelf))) return;

  /* for some reason, owner draw should work only in report mode */
  if ((infoPtr->dwStyle & LVS_OWNERDRAWFIXED) && (uView == LVS_REPORT))
  {
    GXDRAWITEMSTRUCT dis;
    GXLVITEMW item;

    GXHFONT hFont = infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont;
    GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);

    item.iItem = infoPtr->nFocusedItem;
    item.iSubItem = 0;
    item.mask = GXLVIF_PARAM;
    if (!LISTVIEW_GetItemW(infoPtr, &item)) goto done;

    ZeroMemory(&dis, sizeof(dis)); 
    dis.CtlType = GXODT_LISTVIEW;
    dis.CtlID = (GXUINT)gxGetWindowLongPtrW(infoPtr->hwndSelf, GXGWLP_ID);
    dis.itemID = item.iItem;
    dis.itemAction = GXODA_FOCUS;
    if (fShow) dis.itemState |= GXODS_FOCUS;
    dis.hwndItem = infoPtr->hwndSelf;
    dis.hDC = hdc;
    LISTVIEW_GetItemBox(infoPtr, dis.itemID, &dis.rcItem);
    dis.itemData = item.lParam;

    gxSendMessageW(infoPtr->hwndNotify, GXWM_DRAWITEM, dis.CtlID, (GXLPARAM)&dis);

    gxSelectObject(hdc, hOldFont);
  }
  else
  {
    gxDrawFocusRect(hdc, &infoPtr->rcFocus);
  }
done:
  gxReleaseDC(infoPtr->hwndSelf, hdc);
}

/***
* Invalidates all visible selected items.
*/
static void LISTVIEW_InvalidateSelectedItems(const LISTVIEW_INFO *infoPtr)
{
  ITERATOR i; 

  iterator_frameditems(&i, infoPtr, &infoPtr->rcList); 
  while(iterator_next(&i))
  {
    if (LISTVIEW_GetItemState(infoPtr, i.nItem, LVIS_SELECTED))
      LISTVIEW_InvalidateItem(infoPtr, i.nItem);
  }
  iterator_destroy(&i);
}


/***
* DESCRIPTION:            [INTERNAL]
* Computes an item's (left,top) corner, relative to rcView.
* That is, the position has NOT been made relative to the Origin.
* This is deliberate, to avoid computing the Origin over, and
* over again, when this function is called in a loop. Instead,
* one can factor the computation of the Origin before the loop,
* and offset the value returned by this function, on every iteration.
* 
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem  : item number
* [O] lpptOrig : item top, left corner
*
* RETURN:
*   None.
*/
static void LISTVIEW_GetItemOrigin(const LISTVIEW_INFO *infoPtr, GXINT nItem, LPGXPOINT lpptPosition)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;

  WEAssert(nItem >= 0 && nItem < infoPtr->nItemCount);

  if ((uView == LVS_SMALLICON) || (uView == LVS_ICON))
  {
    lpptPosition->x = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosX, nItem);
    lpptPosition->y = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosY, nItem);
  }
  else if (uView == LVS_LIST)
  {
    GXINT nCountPerColumn = LISTVIEW_GetCountPerColumn(infoPtr);
    lpptPosition->x = nItem / nCountPerColumn * infoPtr->nItemWidth;
    lpptPosition->y = nItem % nCountPerColumn * infoPtr->nItemHeight;
  }
  else /* LVS_REPORT */
  {
    lpptPosition->x = 0;
    lpptPosition->y = nItem * infoPtr->nItemHeight;
  }
}

/***
* DESCRIPTION:            [INTERNAL]
* Compute the rectangles of an item.  This is to localize all
* the computations in one place. If you are not interested in some
* of these values, simply pass in a NULL -- the function is smart
* enough to compute only what's necessary. The function computes
* the standard rectangles (BOUNDS, ICON, LABEL) plus a non-standard
* one, the BOX rectangle. This rectangle is very cheap to compute,
* and is guaranteed to contain all the other rectangles. Computing
* the ICON rect is also cheap, but all the others are potentially
* expensive. This gives an easy and effective optimization when
* searching (like point inclusion, or rectangle intersection):
* first test against the BOX, and if TRUE, test against the desired
* rectangle.
* If the function does not have all the necessary information
* to computed the requested rectangles, will crash with a
* failed assertion. This is done so we catch all programming
* errors, given that the function is called only from our code.
*
* We have the following 'special' meanings for a few fields:
*   * If LVIS_FOCUSED is set, we assume the item has the focus
*     This is important in ICON mode, where it might get a larger
*     then usual rectangle
*
* Please note that subitem support works only in REPORT mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] lpLVItem : item to compute the measures for
* [O] lprcBox : ptr to Box rectangle
*                Same as GXLVM_GETITEMRECT with LVIR_BOUNDS
* [0] lprcSelectBox : ptr to select box rectangle
*        Same as GXLVM_GETITEMRECT with LVIR_SELECTEDBOUNDS
* [O] lprcIcon : ptr to Icon rectangle
*                Same as GXLVM_GETITEMRECT with LVIR_ICON
* [O] lprcStateIcon: ptr to State Icon rectangle
* [O] lprcLabel : ptr to Label rectangle
*                Same as GXLVM_GETITEMRECT with LVIR_LABEL
*
* RETURN:
*   None.
*/
static void LISTVIEW_GetItemMetrics(const LISTVIEW_INFO *infoPtr, const GXLVITEMW *lpLVItem,
                  LPGXRECT lprcBox, LPGXRECT lprcSelectBox,
                  LPGXRECT lprcIcon, LPGXRECT lprcStateIcon, LPGXRECT lprcLabel)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXBOOL doSelectBox = FALSE, doIcon = FALSE, doLabel = FALSE, oversizedBox = FALSE;
  GXRECT Box, SelectBox, Icon, Label;
  COLUMN_INFO *lpColumnInfo = NULL;
  GXSIZE labelSize = { 0, 0 };

  TRACE("(lpLVItem=%s)\n", debuglvitem_t(lpLVItem, TRUE));

  /* Be smart and try to figure out the minimum we have to do */
  if (lpLVItem->iSubItem) {
    WEAssert(uView == LVS_REPORT);
  }
  if (uView == LVS_ICON && (lprcBox || lprcLabel))
  {
    WEAssert((lpLVItem->mask & GXLVIF_STATE) && (lpLVItem->stateMask & LVIS_FOCUSED));
    if (lpLVItem->state & LVIS_FOCUSED) oversizedBox = doLabel = TRUE;
  }
  if (lprcSelectBox) doSelectBox = TRUE;
  if (lprcLabel) doLabel = TRUE;
  if (doLabel || lprcIcon || lprcStateIcon) doIcon = TRUE;
  if (doSelectBox)
  {
    doIcon = TRUE;
    doLabel = TRUE;
  }

  /************************************************************/
  /* compute the box rectangle (it should be cheap to do)     */
  /************************************************************/
  if (lpLVItem->iSubItem || uView == LVS_REPORT)
    lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, lpLVItem->iSubItem);

  if (lpLVItem->iSubItem)    
  {
    Box = lpColumnInfo->rcHeader;
  }
  else
  {
    Box.left = 0;
    Box.right = infoPtr->nItemWidth;
  }
  Box.top = 0;
  Box.bottom = infoPtr->nItemHeight;

  /******************************************************************/
  /* compute ICON bounding box (ala GXLVM_GETITEMRECT) and STATEICON  */
  /******************************************************************/
  if (doIcon)
  {
    GXLONG state_width = 0;

    if (infoPtr->himlState && lpLVItem->iSubItem == 0)
      state_width = infoPtr->iconStateSize.cx;

    if (uView == LVS_ICON)
    {
      Icon.left   = Box.left + state_width;
      if (infoPtr->himlNormal)
        Icon.left += (infoPtr->nItemWidth - infoPtr->iconSize.cx - state_width) / 2;
      Icon.top    = Box.top + ICON_TOP_PADDING;
      Icon.right  = Icon.left;
      Icon.bottom = Icon.top;
      if (infoPtr->himlNormal)
      {
        Icon.right  += infoPtr->iconSize.cx;
        Icon.bottom += infoPtr->iconSize.cy;
      }
    }
    else /* LVS_SMALLICON, LVS_LIST or LVS_REPORT */
    {
      Icon.left   = Box.left + state_width;

      if (uView == LVS_REPORT)
        Icon.left += REPORT_MARGINX;

      Icon.top    = Box.top;
      Icon.right  = Icon.left;
      if (infoPtr->himlSmall &&
        (!lpColumnInfo || lpLVItem->iSubItem == 0 || (lpColumnInfo->fmt & LVCFMT_IMAGE) ||
        ((infoPtr->dwLvExStyle & LVS_EX_SUBITEMIMAGES) && lpLVItem->iImage != GXI_IMAGECALLBACK)))
        Icon.right += infoPtr->iconSize.cx;
      Icon.bottom = Icon.top + infoPtr->iconSize.cy;
    }
    if(lprcIcon) *lprcIcon = Icon;
    TRACE("    - icon=%s\n", wine_dbgstr_rect(&Icon));

    /* TODO: is this correct? */
    if (lprcStateIcon)
    {
      lprcStateIcon->left = Icon.left - state_width;
      lprcStateIcon->right = Icon.left;
      lprcStateIcon->top = Icon.top;
      lprcStateIcon->bottom = lprcStateIcon->top + infoPtr->iconSize.cy;
      TRACE("    - state icon=%s\n", wine_dbgstr_rect(lprcStateIcon));
    }
  }
  else Icon.right = 0;

  /************************************************************/
  /* compute LABEL bounding box (ala GXLVM_GETITEMRECT)         */
  /************************************************************/
  if (doLabel)
  {
    /* calculate how far to the right can the label stretch */
    Label.right = Box.right;
    if (uView == LVS_REPORT)
    {
      if (lpLVItem->iSubItem == 0) Label = lpColumnInfo->rcHeader;
    }

    if (lpLVItem->iSubItem || ((infoPtr->dwStyle & LVS_OWNERDRAWFIXED) && uView == LVS_REPORT))
    {
      labelSize.cx = infoPtr->nItemWidth;
      labelSize.cy = infoPtr->nItemHeight;
      goto calc_label;
    }

    /* we need the text in non owner draw mode */
    WEAssert(lpLVItem->mask & GXLVIF_TEXT);
    if (is_textT(lpLVItem->pszText, TRUE))
    {
      GXHFONT hFont = infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont;
      GXHDC hdc = gxGetDC(infoPtr->hwndSelf);
      GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);
      GXUINT uFormat;
      GXRECT rcText;

      /* compute rough rectangle where the label will go */
      gxSetRectEmpty(&rcText);
      rcText.right = infoPtr->nItemWidth - TRAILING_LABEL_PADDING;
      rcText.bottom = infoPtr->nItemHeight;
      if (uView == LVS_ICON) 
        rcText.bottom -= ICON_TOP_PADDING + infoPtr->iconSize.cy + ICON_BOTTOM_PADDING;

      /* now figure out the flags */
      if (uView == LVS_ICON)
        uFormat = oversizedBox ? LV_FL_DT_FLAGS : LV_ML_DT_FLAGS;
      else
        uFormat = LV_SL_DT_FLAGS;

      gxDrawTextW (hdc, lpLVItem->pszText, -1, &rcText, uFormat | GXDT_CALCRECT);

      labelSize.cx = min(rcText.right - rcText.left + TRAILING_LABEL_PADDING, infoPtr->nItemWidth);
      labelSize.cy = rcText.bottom - rcText.top;

      gxSelectObject(hdc, hOldFont);
      gxReleaseDC(infoPtr->hwndSelf, hdc);
    }

calc_label:
    if (uView == LVS_ICON)
    {
      Label.left = Box.left + (infoPtr->nItemWidth - labelSize.cx) / 2;
      Label.top  = Box.top + ICON_TOP_PADDING_HITABLE +
        infoPtr->iconSize.cy + ICON_BOTTOM_PADDING;
      Label.right = Label.left + labelSize.cx;
      Label.bottom = Label.top + infoPtr->nItemHeight;
      if (!oversizedBox && labelSize.cy > infoPtr->ntmHeight)
      {
        labelSize.cy = min(Box.bottom - Label.top, labelSize.cy);
        labelSize.cy /= infoPtr->ntmHeight;
        labelSize.cy = max(labelSize.cy, 1);
        labelSize.cy *= infoPtr->ntmHeight;
      }
      Label.bottom = Label.top + labelSize.cy + HEIGHT_PADDING;
    }
    else if (uView == LVS_REPORT)
    {
      Label.left = Icon.right;
      Label.top = Box.top;
      Label.right = lpColumnInfo->rcHeader.right;
      Label.bottom = Label.top + infoPtr->nItemHeight;
    }
    else /* LVS_SMALLICON, LVS_LIST or LVS_REPORT */
    {
      Label.left = Icon.right;
      Label.top = Box.top;
      Label.right = min(Label.left + labelSize.cx, Label.right);
      Label.bottom = Label.top + infoPtr->nItemHeight;
    }

    if (lprcLabel) *lprcLabel = Label;
    TRACE("    - label=%s\n", wine_dbgstr_rect(&Label));
  }

  /************************************************************/
  /* compute STATEICON bounding box                           */
  /************************************************************/
  if (doSelectBox)
  {
    if (uView == LVS_REPORT)
    {
      SelectBox.left = Icon.right; /* FIXME: should be Icon.left */
      SelectBox.top = Box.top;
      SelectBox.bottom = Box.bottom;
      if (lpLVItem->iSubItem == 0)
      {
        /* we need the indent in report mode */
        WEAssert(lpLVItem->mask & GXLVIF_INDENT);
        SelectBox.left += infoPtr->iconSize.cx * lpLVItem->iIndent;
      }
      SelectBox.right = min(SelectBox.left + labelSize.cx, Label.right);
    }
    else
    {
      gxUnionRect(&SelectBox, &Icon, &Label);
    }
    if (lprcSelectBox) *lprcSelectBox = SelectBox;
    TRACE("    - select box=%s\n", wine_dbgstr_rect(&SelectBox));
  }

  /* Fix the Box if necessary */
  if (lprcBox)
  {
    if (oversizedBox) gxUnionRect(lprcBox, &Box, &Label);
    else *lprcBox = Box;
  }
  TRACE("    - box=%s\n", wine_dbgstr_rect(&Box));
}

/***
* DESCRIPTION:            [INTERNAL]
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item number
* [O] lprcBox : ptr to Box rectangle
*
* RETURN:
*   None.
*/
static void LISTVIEW_GetItemBox(const LISTVIEW_INFO *infoPtr, GXINT nItem, LPGXRECT lprcBox)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  GXPOINT Position, Origin;
  GXLVITEMW lvItem;

  LISTVIEW_GetOrigin(infoPtr, &Origin);
  LISTVIEW_GetItemOrigin(infoPtr, nItem, &Position);

  /* Be smart and try to figure out the minimum we have to do */
  lvItem.mask = 0;
  if (uView == LVS_ICON && infoPtr->bFocus && LISTVIEW_GetItemState(infoPtr, nItem, LVIS_FOCUSED))
    lvItem.mask |= GXLVIF_TEXT;
  lvItem.iItem = nItem;
  lvItem.iSubItem = 0;
  lvItem.pszText = szDispText;
  lvItem.cchTextMax = DISP_TEXT_SIZE;
  if (lvItem.mask) LISTVIEW_GetItemW(infoPtr, &lvItem);
  if (uView == LVS_ICON)
  {
    lvItem.mask |= GXLVIF_STATE;
    lvItem.stateMask = LVIS_FOCUSED;
    lvItem.state = (lvItem.mask & GXLVIF_TEXT ? LVIS_FOCUSED : 0);
  }
  LISTVIEW_GetItemMetrics(infoPtr, &lvItem, lprcBox, 0, 0, 0, 0);

  gxOffsetRect(lprcBox, Position.x + Origin.x, Position.y + Origin.y);
}


/***
* DESCRIPTION:
* Returns the current icon position, and advances it along the top.
* The returned position is not offset by Origin.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lpPos : will get the current icon position
*
* RETURN:
* None
*/
static void LISTVIEW_NextIconPosTop(LISTVIEW_INFO *infoPtr, LPGXPOINT lpPos)
{
  GXINT nListWidth = infoPtr->rcList.right - infoPtr->rcList.left;

  *lpPos = infoPtr->currIconPos;

  infoPtr->currIconPos.x += infoPtr->nItemWidth;
  if (infoPtr->currIconPos.x + infoPtr->nItemWidth <= nListWidth) return;

  infoPtr->currIconPos.x  = 0;
  infoPtr->currIconPos.y += infoPtr->nItemHeight;
}


/***
* DESCRIPTION:
* Returns the current icon position, and advances it down the left edge.
* The returned position is not offset by Origin.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lpPos : will get the current icon position
*
* RETURN:
* None
*/
static void LISTVIEW_NextIconPosLeft(LISTVIEW_INFO *infoPtr, LPGXPOINT lpPos)
{
  GXINT nListHeight = infoPtr->rcList.bottom - infoPtr->rcList.top;

  *lpPos = infoPtr->currIconPos;

  infoPtr->currIconPos.y += infoPtr->nItemHeight;
  if (infoPtr->currIconPos.y + infoPtr->nItemHeight <= nListHeight) return;

  infoPtr->currIconPos.x += infoPtr->nItemWidth;
  infoPtr->currIconPos.y  = 0;
}


/***
* DESCRIPTION:
* Moves an icon to the specified position.
* It takes care of invalidating the item, etc.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : the item to move
* [I] lpPos : the new icon position
* [I] isNew : flags the item as being new
*
* RETURN:
*   Success: TRUE
*   Failure: FALSE
*/
static GXBOOL LISTVIEW_MoveIconTo(const LISTVIEW_INFO *infoPtr, GXINT nItem, const GXPOINT *lppt, GXBOOL isNew)
{
  GXPOINT old;

  if (!isNew)
  { 
    old.x = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosX, nItem);
    old.y = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosY, nItem);

    if (lppt->x == old.x && lppt->y == old.y) return TRUE;
    LISTVIEW_InvalidateItem(infoPtr, nItem);
  }

  /* Allocating a POINTER for every item is too resource intensive,
  * so we'll keep the (x,y) in different arrays */
  if (!gxDPA_SetPtr(infoPtr->hdpaPosX, nItem, (void *)(GXLONG_PTR)lppt->x)) return FALSE;
  if (!gxDPA_SetPtr(infoPtr->hdpaPosY, nItem, (void *)(GXLONG_PTR)lppt->y)) return FALSE;

  LISTVIEW_InvalidateItem(infoPtr, nItem);

  return TRUE;
}

/***
* DESCRIPTION:
* Arranges listview items in icon display mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nAlignCode : alignment code
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_Arrange(LISTVIEW_INFO *infoPtr, GXINT nAlignCode)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  void (*next_pos)(LISTVIEW_INFO *, LPGXPOINT);
  GXPOINT pos;
  GXINT i;

  if (uView != LVS_ICON && uView != LVS_SMALLICON) return FALSE;

  TRACE("nAlignCode=%d\n", nAlignCode);

  if (nAlignCode == LVA_DEFAULT)
  {
    if (infoPtr->dwStyle & LVS_ALIGNLEFT) nAlignCode = LVA_ALIGNLEFT;
    else nAlignCode = LVA_ALIGNTOP;
  }

  switch (nAlignCode)
  {
  case LVA_ALIGNLEFT:  next_pos = LISTVIEW_NextIconPosLeft; break;
  case LVA_ALIGNTOP:   next_pos = LISTVIEW_NextIconPosTop;  break;
  case LVA_SNAPTOGRID: next_pos = LISTVIEW_NextIconPosTop;  break; /* FIXME */
  default: return FALSE;
  }

  infoPtr->bAutoarrange = TRUE;
  infoPtr->currIconPos.x = infoPtr->currIconPos.y = 0;
  for (i = 0; i < infoPtr->nItemCount; i++)
  {
    next_pos(infoPtr, &pos);
    LISTVIEW_MoveIconTo(infoPtr, i, &pos, FALSE);
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Retrieves the bounding rectangle of all the items, not offset by Origin.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lprcView : bounding rectangle
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static void LISTVIEW_GetAreaRect(const LISTVIEW_INFO *infoPtr, LPGXRECT lprcView)
{
  GXINT i, x, y;

  gxSetRectEmpty(lprcView);

  switch (infoPtr->dwStyle & LVS_TYPEMASK)
  {
  case LVS_ICON:
  case LVS_SMALLICON:
    for (i = 0; i < infoPtr->nItemCount; i++)
    {
      x = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosX, i);
      y = (GXLONG_PTR)gxDPA_GetPtr(infoPtr->hdpaPosY, i);
      lprcView->right = max(lprcView->right, x);
      lprcView->bottom = max(lprcView->bottom, y);
    }
    if (infoPtr->nItemCount > 0)
    {
      lprcView->right += infoPtr->nItemWidth;
      lprcView->bottom += infoPtr->nItemHeight;
    }
    break;

  case LVS_LIST:
    y = LISTVIEW_GetCountPerColumn(infoPtr);
    x = infoPtr->nItemCount / y;
    if (infoPtr->nItemCount % y) x++;
    lprcView->right = x * infoPtr->nItemWidth;
    lprcView->bottom = y * infoPtr->nItemHeight;
    break;

  case LVS_REPORT:      
    lprcView->right = infoPtr->nItemWidth;
    lprcView->bottom = infoPtr->nItemCount * infoPtr->nItemHeight;
    break;
  }
}

/***
* DESCRIPTION:
* Retrieves the bounding rectangle of all the items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lprcView : bounding rectangle
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_GetViewRect(const LISTVIEW_INFO *infoPtr, LPGXRECT lprcView)
{
  GXPOINT ptOrigin;

  TRACE("(lprcView=%p)\n", lprcView);

  if (!lprcView) return FALSE;

  LISTVIEW_GetOrigin(infoPtr, &ptOrigin);
  LISTVIEW_GetAreaRect(infoPtr, lprcView); 
  gxOffsetRect(lprcView, ptOrigin.x, ptOrigin.y); 

  TRACE("lprcView=%s\n", wine_dbgstr_rect(lprcView));

  return TRUE;
}

/***
* DESCRIPTION:
* Retrieves the subitem pointer associated with the subitem index.
*
* PARAMETER(S):
* [I] hdpaSubItems : GXDPA handle for a specific item
* [I] nSubItem : index of subitem
*
* RETURN:
*   SUCCESS : subitem pointer
*   FAILURE : NULL
*/
static SUBITEM_INFO* LISTVIEW_GetSubItemPtr(GXHDPA hdpaSubItems, GXINT nSubItem)
{
  SUBITEM_INFO *lpSubItem;
  GXINT i;

  /* we should binary search here if need be */
  for (i = 1; i < gxDPA_GetPtrCount(hdpaSubItems); i++)
  {
    lpSubItem = (SUBITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, i);
    if (lpSubItem->iSubItem == nSubItem)
      return lpSubItem;
  }

  return NULL;
}


/***
* DESCRIPTION:
* Calculates the desired item width.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*  The desired item width.
*/
static GXINT LISTVIEW_CalculateItemWidth(const LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nItemWidth = 0;

  TRACE("uView=%d\n", uView);

  if (uView == LVS_ICON)
    nItemWidth = infoPtr->iconSpacing.cx;
  else if (uView == LVS_REPORT)
  {
    GXRECT rcHeader;

    if (gxDPA_GetPtrCount(infoPtr->hdpaColumns) > 0)
    {
      LISTVIEW_GetHeaderRect(infoPtr, gxDPA_GetPtrCount(infoPtr->hdpaColumns) - 1, &rcHeader);
      nItemWidth = rcHeader.right;
    }
  }
  else /* LVS_SMALLICON, or LVS_LIST */
  {
    GXINT i;

    for (i = 0; i < infoPtr->nItemCount; i++)
      nItemWidth = max(LISTVIEW_GetLabelWidth(infoPtr, i), nItemWidth);

    if (infoPtr->himlSmall) nItemWidth += infoPtr->iconSize.cx; 
    if (infoPtr->himlState) nItemWidth += infoPtr->iconStateSize.cx;

    nItemWidth = max(DEFAULT_COLUMN_WIDTH, nItemWidth + WIDTH_PADDING);
  }

  return max(nItemWidth, 1);
}

/***
* DESCRIPTION:
* Calculates the desired item height.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*  The desired item height.
*/
static GXINT LISTVIEW_CalculateItemHeight(const LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nItemHeight;

  TRACE("uView=%d\n", uView);

  if (uView == LVS_ICON)
    nItemHeight = infoPtr->iconSpacing.cy;
  else
  {
    nItemHeight = infoPtr->ntmHeight; 
    if (uView == LVS_REPORT && infoPtr->dwLvExStyle & LVS_EX_GRIDLINES)
      nItemHeight++;
    if (infoPtr->himlState)
      nItemHeight = max(nItemHeight, infoPtr->iconStateSize.cy);
    if (infoPtr->himlSmall)
      nItemHeight = max(nItemHeight, infoPtr->iconSize.cy);
    if (infoPtr->himlState || infoPtr->himlSmall)
      nItemHeight += HEIGHT_PADDING;
    if (infoPtr->nMeasureItemHeight > 0)
      nItemHeight = infoPtr->nMeasureItemHeight;
  }

  return max(nItemHeight, 1);
}

/***
* DESCRIPTION:
* Updates the width, and height of an item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*  None.
*/
static inline void LISTVIEW_UpdateItemSize(LISTVIEW_INFO *infoPtr)
{
  infoPtr->nItemWidth = LISTVIEW_CalculateItemWidth(infoPtr);
  infoPtr->nItemHeight = LISTVIEW_CalculateItemHeight(infoPtr);
}


/***
* DESCRIPTION:
* Retrieves and saves important text metrics info for the current
* Listview font.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
*/
static void LISTVIEW_SaveTextMetrics(LISTVIEW_INFO *infoPtr)
{
  GXHDC hdc = gxGetDC(infoPtr->hwndSelf);
  GXHFONT hFont = infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont;
  GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);
  GXTEXTMETRICW tm;
  GXSIZE sz;

  if (gxGetTextMetricsW(hdc, &tm))
  {
    infoPtr->ntmHeight = tm.tmHeight;
    infoPtr->ntmMaxCharWidth = tm.tmMaxCharWidth;
  }

  if (gxGetTextExtentPoint32W(hdc, L"...", 3, &sz))
    infoPtr->nEllipsisWidth = sz.cx;

  gxSelectObject(hdc, hOldFont);
  gxReleaseDC(infoPtr->hwndSelf, hdc);

  TRACE("tmHeight=%d\n", infoPtr->ntmHeight);
}

/***
* DESCRIPTION:
* A compare function for ranges
*
* PARAMETER(S)
* [I] range1 : pointer to range 1;
* [I] range2 : pointer to range 2;
* [I] flags : flags
*
* RETURNS:
* > 0 : if range 1 > range 2
* < 0 : if range 2 > range 1
* = 0 : if range intersects range 2
*/
static GXINT GXCALLBACK ranges_cmp(GXLPVOID range1, GXLPVOID range2, GXLPARAM flags)
{
  GXINT cmp;

  if (((RANGE*)range1)->upper <= ((RANGE*)range2)->lower) 
    cmp = -1;
  else if (((RANGE*)range2)->upper <= ((RANGE*)range1)->lower) 
    cmp = 1;
  else 
    cmp = 0;

  TRACE("range1=%s, range2=%s, cmp=%d\n", debugrange((RANGE*)range1), debugrange((RANGE*)range2), cmp);

  return cmp;
}

#if DEBUG_RANGES
#define ranges_check(ranges, desc) ranges_assert(ranges, desc, __FUNCTION__, __LINE__)
#else
#define ranges_check(ranges, desc) do { } while(0)
#endif

static void ranges_assert(RANGES ranges, GXLPCSTR desc, const char *func, int line)
{
  GXINT i;
  RANGE *prev, *curr;

  TRACE("*** Checking %s:%d:%s ***\n", func, line, desc);
  WEAssert (ranges);
  WEAssert (gxDPA_GetPtrCount(ranges->hdpa) >= 0);
  ranges_dump(ranges);
  prev = (RANGE *)gxDPA_GetPtr(ranges->hdpa, 0);
  if (gxDPA_GetPtrCount(ranges->hdpa) > 0) {
    WEAssert (prev->lower >= 0 && prev->lower < prev->upper);
  }
  for (i = 1; i < gxDPA_GetPtrCount(ranges->hdpa); i++)
  {
    curr = (RANGE *)gxDPA_GetPtr(ranges->hdpa, i);
    WEAssert (prev->upper <= curr->lower);
    WEAssert (curr->lower < curr->upper);
    prev = curr;
  }
  TRACE("--- Done checking---\n");
}

static RANGES ranges_create(int count)
{
  RANGES ranges = (RANGES)Alloc(sizeof(struct tagRANGES));
  if (!ranges) return NULL;
  ranges->hdpa = gxDPA_Create(count);
  if (ranges->hdpa) return ranges;
  Free(ranges);
  return NULL;
}

static void ranges_clear(RANGES ranges)
{
  GXINT i;

  for(i = 0; i < gxDPA_GetPtrCount(ranges->hdpa); i++)
    Free(gxDPA_GetPtr(ranges->hdpa, i));
  gxDPA_DeleteAllPtrs(ranges->hdpa);
}


static void ranges_destroy(RANGES ranges)
{
  if (!ranges) return;
  ranges_clear(ranges);
  gxDPA_Destroy(ranges->hdpa);
  Free(ranges);
}

static RANGES ranges_clone(RANGES ranges)
{
  RANGES clone;
  GXINT i;

  if (!(clone = ranges_create(gxDPA_GetPtrCount(ranges->hdpa)))) goto fail;

  for (i = 0; i < gxDPA_GetPtrCount(ranges->hdpa); i++)
  {
    RANGE *newrng = (RANGE*)Alloc(sizeof(RANGE));
    if (!newrng) goto fail;
    *newrng = *((RANGE*)gxDPA_GetPtr(ranges->hdpa, i));
    gxDPA_SetPtr(clone->hdpa, i, newrng);
  }
  return clone;

fail:
  TRACE ("clone failed\n");
  ranges_destroy(clone);
  return NULL;
}

static RANGES ranges_diff(RANGES ranges, RANGES sub)
{
  GXINT i;

  for (i = 0; i < gxDPA_GetPtrCount(sub->hdpa); i++)
    ranges_del(ranges, *((RANGE *)gxDPA_GetPtr(sub->hdpa, i)));

  return ranges;
}

static void ranges_dump(RANGES ranges)
{
  GXINT i;

  for (i = 0; i < gxDPA_GetPtrCount(ranges->hdpa); i++)
    TRACE("   %s\n", debugrange((const RANGE*)gxDPA_GetPtr(ranges->hdpa, i)));
}

static inline GXBOOL ranges_contain(RANGES ranges, GXINT nItem)
{
  RANGE srchrng = { nItem, nItem + 1 };

  TRACE("(nItem=%d)\n", nItem);
  ranges_check(ranges, "before contain");
  return gxDPA_Search(ranges->hdpa, &srchrng, 0, ranges_cmp, 0, DPAS_SORTED) != -1;
}

static GXINT ranges_itemcount(RANGES ranges)
{
  GXINT i, count = 0;

  for (i = 0; i < gxDPA_GetPtrCount(ranges->hdpa); i++)
  {
    RANGE *sel = (RANGE*)gxDPA_GetPtr(ranges->hdpa, i);
    count += sel->upper - sel->lower;
  }

  return count;
}

static GXBOOL ranges_shift(RANGES ranges, GXINT nItem, GXINT delta, GXINT nUpper)
{
  RANGE srchrng = { nItem, nItem + 1 }, *chkrng;
  GXINT index;

  index = gxDPA_Search(ranges->hdpa, &srchrng, 0, ranges_cmp, 0, DPAS_SORTED | DPAS_INSERTAFTER);
  if (index == -1) return TRUE;

  for (; index < gxDPA_GetPtrCount(ranges->hdpa); index++)
  {
    chkrng = (RANGE*)gxDPA_GetPtr(ranges->hdpa, index);
    if (chkrng->lower >= nItem)
      chkrng->lower = max(min(chkrng->lower + delta, nUpper - 1), 0);
    if (chkrng->upper > nItem)
      chkrng->upper = max(min(chkrng->upper + delta, nUpper), 0);
  }
  return TRUE;
}

static GXBOOL ranges_add(RANGES ranges, RANGE range)
{
  RANGE srcGXHRGN;
  GXINT index;

  TRACE("(%s)\n", debugrange(&range));
  ranges_check(ranges, "before add");

  /* try find overlapping regions first */
  srcGXHRGN.lower = range.lower - 1;
  srcGXHRGN.upper = range.upper + 1;
  index = gxDPA_Search(ranges->hdpa, &srcGXHRGN, 0, ranges_cmp, 0, DPAS_SORTED);

  if (index == -1)
  {
    RANGE *newrgn;

    TRACE("Adding new range\n");

    /* create the brand new range to insert */  
    newrgn = (RANGE *)Alloc(sizeof(RANGE));
    if(!newrgn) goto fail;
    *newrgn = range;

    /* figure out where to insert it */
    index = gxDPA_Search(ranges->hdpa, newrgn, 0, ranges_cmp, 0, DPAS_SORTED | DPAS_INSERTAFTER);
    TRACE("index=%d\n", index);
    if (index == -1) index = 0;

    /* and get it over with */
    if (gxDPA_InsertPtr(ranges->hdpa, index, newrgn) == -1)
    {
      Free(newrgn);
      goto fail;
    }
  }
  else
  {
    RANGE *chkrgn, *mrgrgn;
    GXINT fromindex, mergeindex;

    chkrgn = (RANGE*)gxDPA_GetPtr(ranges->hdpa, index);
    TRACE("Merge with %s @%d\n", debugrange(chkrgn), index);

    chkrgn->lower = min(range.lower, chkrgn->lower);
    chkrgn->upper = max(range.upper, chkrgn->upper);

    TRACE("New range %s @%d\n", debugrange(chkrgn), index);

    /* merge now common ranges */
    fromindex = 0;
    srcGXHRGN.lower = chkrgn->lower - 1;
    srcGXHRGN.upper = chkrgn->upper + 1;

    do
    {
      mergeindex = gxDPA_Search(ranges->hdpa, &srcGXHRGN, fromindex, ranges_cmp, 0, 0);
      if (mergeindex == -1) break;
      if (mergeindex == index) 
      {
        fromindex = index + 1;
        continue;
      }

      TRACE("Merge with index %i\n", mergeindex);

      mrgrgn = (RANGE*)gxDPA_GetPtr(ranges->hdpa, mergeindex);
      chkrgn->lower = min(chkrgn->lower, mrgrgn->lower);
      chkrgn->upper = max(chkrgn->upper, mrgrgn->upper);
      Free(mrgrgn);
      gxDPA_DeletePtr(ranges->hdpa, mergeindex);
      if (mergeindex < index) index --;
    } while(1);
  }

  ranges_check(ranges, "after add");
  return TRUE;

fail:
  ranges_check(ranges, "failed add");
  return FALSE;
}

static GXBOOL ranges_del(RANGES ranges, RANGE range)
{
  RANGE *chkrgn;
  GXINT index;

  TRACE("(%s)\n", debugrange(&range));
  ranges_check(ranges, "before del");

  /* we don't use DPAS_SORTED here, since we need *
  * to find the first overlapping range          */
  index = gxDPA_Search(ranges->hdpa, &range, 0, ranges_cmp, 0, 0);
  while(index != -1) 
  {
    chkrgn = (RANGE*)gxDPA_GetPtr(ranges->hdpa, index);

    TRACE("Matches range %s @%d\n", debugrange(chkrgn), index); 

    /* case 1: Same range */
    if ( (chkrgn->upper == range.upper) &&
      (chkrgn->lower == range.lower) )
    {
      Free(chkrgn);  // Try Fix: Artint
      gxDPA_DeletePtr(ranges->hdpa, index);
      break;
    }
    /* case 2: engulf */
    else if ( (chkrgn->upper <= range.upper) &&
      (chkrgn->lower >= range.lower) ) 
    {
      Free(chkrgn);  // Try Fix: Artint
      gxDPA_DeletePtr(ranges->hdpa, index);
    }
    /* case 3: overlap upper */
    else if ( (chkrgn->upper <= range.upper) &&
      (chkrgn->lower < range.lower) )
    {
      chkrgn->upper = range.lower;
    }
    /* case 4: overlap lower */
    else if ( (chkrgn->upper > range.upper) &&
      (chkrgn->lower >= range.lower) )
    {
      chkrgn->lower = range.upper;
      break;
    }
    /* case 5: fully internal */
    else
    {
      RANGE tmprgn = *chkrgn, *newrgn;

      if (!(newrgn = (RANGE*)Alloc(sizeof(RANGE)))) goto fail;
      newrgn->lower = chkrgn->lower;
      newrgn->upper = range.lower;
      chkrgn->lower = range.upper;
      if (gxDPA_InsertPtr(ranges->hdpa, index, newrgn) == -1)
      {
        Free(newrgn);
        goto fail;
      }
      chkrgn = &tmprgn;
      break;
    }

    index = gxDPA_Search(ranges->hdpa, &range, index, ranges_cmp, 0, 0);
  }

  ranges_check(ranges, "after del");
  return TRUE;

fail:
  ranges_check(ranges, "failed del");
  return FALSE;
}

/***
* DESCRIPTION:
* Removes all selection ranges
*
* Parameters(s):
* [I] infoPtr : valid pointer to the listview structure
* [I] toSkip : item range to skip removing the selection
*
* RETURNS:
*   SUCCESS : TRUE
*   FAILURE : TRUE
*/
static GXBOOL LISTVIEW_DeselectAllSkipItems(LISTVIEW_INFO *infoPtr, RANGES toSkip)
{
  GXLVITEMW lvItem;
  ITERATOR i;
  RANGES clone;

  TRACE("()\n");

  lvItem.state = 0;
  lvItem.stateMask = LVIS_SELECTED;

  /* need to clone the GXDPA because callbacks can change it */
  if (!(clone = ranges_clone(infoPtr->selectionRanges))) return FALSE;
  iterator_rangesitems(&i, ranges_diff(clone, toSkip));
  while(iterator_next(&i))
    LISTVIEW_SetItemState(infoPtr, i.nItem, &lvItem);
  /* note that the iterator destructor will free the cloned range */
  iterator_destroy(&i);

  return TRUE;
}

static inline GXBOOL LISTVIEW_DeselectAllSkipItem(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  RANGES toSkip;

  if (!(toSkip = ranges_create(1))) return FALSE;
  if (nItem != -1) ranges_additem(toSkip, nItem);
  LISTVIEW_DeselectAllSkipItems(infoPtr, toSkip);
  ranges_destroy(toSkip);
  return TRUE;
}

static inline GXBOOL LISTVIEW_DeselectAll(LISTVIEW_INFO *infoPtr)
{
  return LISTVIEW_DeselectAllSkipItem(infoPtr, -1);
}

/***
* DESCRIPTION:
* Retrieves the number of items that are marked as selected.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Number of items selected.
*/
static GXINT LISTVIEW_GetSelectedCount(const LISTVIEW_INFO *infoPtr)
{
  GXINT nSelectedCount = 0;

  if (infoPtr->uCallbackMask & LVIS_SELECTED)
  {
    GXINT i;
    for (i = 0; i < infoPtr->nItemCount; i++)
    {
      if (LISTVIEW_GetItemState(infoPtr, i, LVIS_SELECTED))
        nSelectedCount++;
    }
  }
  else
    nSelectedCount = ranges_itemcount(infoPtr->selectionRanges);

  TRACE("nSelectedCount=%d\n", nSelectedCount);
  return nSelectedCount;
}

/***
* DESCRIPTION:
* Manages the item focus.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
*   TRUE : focused item changed
*   FALSE : focused item has NOT changed
*/
static inline GXBOOL LISTVIEW_SetItemFocus(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXINT oldFocus = infoPtr->nFocusedItem;
  GXLVITEMW lvItem;

  if (nItem == infoPtr->nFocusedItem) return FALSE;

  lvItem.state =  nItem == -1 ? 0 : LVIS_FOCUSED;
  lvItem.stateMask = LVIS_FOCUSED;
  LISTVIEW_SetItemState(infoPtr, nItem == -1 ? infoPtr->nFocusedItem : nItem, &lvItem);

  return oldFocus != infoPtr->nFocusedItem;
}

/* Helper function for LISTVIEW_ShiftIndices *only* */
static GXINT shift_item(const LISTVIEW_INFO *infoPtr, GXINT nShiftItem, GXINT nItem, GXINT direction)
{
  if (nShiftItem < nItem) return nShiftItem;

  if (nShiftItem > nItem) return nShiftItem + direction;

  if (direction > 0) return nShiftItem + direction;

  return min(nShiftItem, infoPtr->nItemCount - 1);
}

/**
* DESCRIPTION:
* Updates the various indices after an item has been inserted or deleted.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] direction : Direction of shift, +1 or -1.
*
* RETURN:
* None
*/
static void LISTVIEW_ShiftIndices(LISTVIEW_INFO *infoPtr, GXINT nItem, GXINT direction)
{
  GXINT nNewFocus;
  GXBOOL bOldChange;

  /* temporarily disable change notification while shifting items */
  bOldChange = infoPtr->bDoChangeNotify;
  infoPtr->bDoChangeNotify = FALSE;

  TRACE("Shifting %iu, %i steps\n", nItem, direction);

  ranges_shift(infoPtr->selectionRanges, nItem, direction, infoPtr->nItemCount);

  WEAssert(abs(direction) == 1);

  infoPtr->nSelectionMark = shift_item(infoPtr, infoPtr->nSelectionMark, nItem, direction);

  nNewFocus = shift_item(infoPtr, infoPtr->nFocusedItem, nItem, direction);
  if (nNewFocus != infoPtr->nFocusedItem)
    LISTVIEW_SetItemFocus(infoPtr, nNewFocus);

  /* But we are not supposed to modify nHotItem! */

  infoPtr->bDoChangeNotify = bOldChange;
}


/**
* DESCRIPTION:
* Adds a block of selections.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
* Whether the window is still valid.
*/
static GXBOOL LISTVIEW_AddGroupSelection(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXINT nFirst = min(infoPtr->nSelectionMark, nItem);
  GXINT nLast = max(infoPtr->nSelectionMark, nItem);
  GXHWND hwndSelf = infoPtr->hwndSelf;
  GXNMLVODSTATECHANGE nmlv;
  GXLVITEMW item;
  GXBOOL bOldChange;
  GXINT i;

  /* Temporarily disable change notification
  * If the control is LVS_OWNERDATA, we need to send
  * only one LVN_ODSTATECHANGED notification.
  * See MSDN documentation for LVN_ITEMCHANGED.
  */
  bOldChange = infoPtr->bDoChangeNotify;
  if (infoPtr->dwStyle & LVS_OWNERDATA) infoPtr->bDoChangeNotify = FALSE;

  if (nFirst == -1) nFirst = nItem;

  item.state = LVIS_SELECTED;
  item.stateMask = LVIS_SELECTED;

  for (i = nFirst; i <= nLast; i++)
    LISTVIEW_SetItemState(infoPtr,i,&item);

  ZeroMemory(&nmlv, sizeof(nmlv));
  nmlv.iFrom = nFirst;
  nmlv.iTo = nLast;
  nmlv.uNewState = 0;
  nmlv.uOldState = item.state;

  notify_hdr(infoPtr, LVN_ODSTATECHANGED, (GXLPNMHDR)&nmlv);
  if (!gxIsWindow(hwndSelf))
    return FALSE;
  infoPtr->bDoChangeNotify = bOldChange;
  return TRUE;
}


/***
* DESCRIPTION:
* Sets a single group selection.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
* None
*/
static void LISTVIEW_SetGroupSelection(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  RANGES selection;
  GXLVITEMW item;
  ITERATOR i;
  GXBOOL bOldChange;

  if (!(selection = ranges_create(100))) return;

  item.state = LVIS_SELECTED; 
  item.stateMask = LVIS_SELECTED;

  if ((uView == LVS_LIST) || (uView == LVS_REPORT))
  {
    if (infoPtr->nSelectionMark == -1)
    {
      infoPtr->nSelectionMark = nItem;
      ranges_additem(selection, nItem);
    }
    else
    {
      RANGE sel;

      sel.lower = min(infoPtr->nSelectionMark, nItem);
      sel.upper = max(infoPtr->nSelectionMark, nItem) + 1;
      ranges_add(selection, sel);
    }
  }
  else
  {
    GXRECT rcItem, rcSel, rcSelMark;
    GXPOINT ptItem;

    rcItem.left = LVIR_BOUNDS;
    if (!LISTVIEW_GetItemRect(infoPtr, nItem, &rcItem)) return;
    rcSelMark.left = LVIR_BOUNDS;
    if (!LISTVIEW_GetItemRect(infoPtr, infoPtr->nSelectionMark, &rcSelMark)) return;
    gxUnionRect(&rcSel, &rcItem, &rcSelMark);
    iterator_frameditems(&i, infoPtr, &rcSel);
    while(iterator_next(&i))
    {
      LISTVIEW_GetItemPosition(infoPtr, i.nItem, &ptItem);
      if (gxPtInRect(&rcSel, ptItem)) ranges_additem(selection, i.nItem);
    }
    iterator_destroy(&i);
  }

  bOldChange = infoPtr->bDoChangeNotify;
  infoPtr->bDoChangeNotify = FALSE;

  LISTVIEW_DeselectAllSkipItems(infoPtr, selection);


  iterator_rangesitems(&i, selection);
  while(iterator_next(&i))
    LISTVIEW_SetItemState(infoPtr, i.nItem, &item);
  /* this will also destroy the selection */
  iterator_destroy(&i);

  infoPtr->bDoChangeNotify = bOldChange;

  LISTVIEW_SetItemFocus(infoPtr, nItem);
}

/***
* DESCRIPTION:
* Sets a single selection.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
* None
*/
static void LISTVIEW_SetSelection(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXLVITEMW lvItem;

  TRACE("nItem=%d\n", nItem);

  LISTVIEW_DeselectAllSkipItem(infoPtr, nItem);

  lvItem.state = LVIS_FOCUSED | LVIS_SELECTED;
  lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
  LISTVIEW_SetItemState(infoPtr, nItem, &lvItem);

  infoPtr->nSelectionMark = nItem;
}

/***
* DESCRIPTION:
* Set selection(s) with keyboard.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
*   SUCCESS : TRUE (needs to be repainted)
*   FAILURE : FALSE (nothing has changed)
*/
static GXBOOL LISTVIEW_KeySelection(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  /* FIXME: pass in the state */
  GXWORD wShift = GXHIWORD(gxGetKeyState(GXVK_SHIFT));
  GXWORD wCtrl = GXHIWORD(gxGetKeyState(GXVK_CONTROL));
  GXBOOL bResult = FALSE;

  TRACE("nItem=%d, wShift=%d, wCtrl=%d\n", nItem, wShift, wCtrl);
  if ((nItem >= 0) && (nItem < infoPtr->nItemCount))
  {
    if (infoPtr->dwStyle & LVS_SINGLESEL)
    {
      bResult = TRUE;
      LISTVIEW_SetSelection(infoPtr, nItem);
    }
    else
    {
      if (wShift)
      {
        bResult = TRUE;
        LISTVIEW_SetGroupSelection(infoPtr, nItem);
      }
      else if (wCtrl)
      {
        GXLVITEMW lvItem;
        lvItem.state = ~LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED);
        lvItem.stateMask = LVIS_SELECTED;
        LISTVIEW_SetItemState(infoPtr, nItem, &lvItem);

        if (lvItem.state & LVIS_SELECTED)
          infoPtr->nSelectionMark = nItem;

        bResult = LISTVIEW_SetItemFocus(infoPtr, nItem);
      }
      else
      {
        bResult = TRUE;
        LISTVIEW_SetSelection(infoPtr, nItem);
      }
    }
    LISTVIEW_EnsureVisible(infoPtr, nItem, FALSE);
  }

  gxUpdateWindow(infoPtr->hwndSelf); /* update client area */
  return bResult;
}

static GXBOOL LISTVIEW_GetItemAtPt(const LISTVIEW_INFO *infoPtr, GXLPLVITEMW lpLVItem, GXPOINT pt)
{
  GXLVHITTESTINFO lvHitTestInfo;

  ZeroMemory(&lvHitTestInfo, sizeof(lvHitTestInfo));
  lvHitTestInfo.pt.x = pt.x;
  lvHitTestInfo.pt.y = pt.y;

  LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, FALSE);

  lpLVItem->mask = GXLVIF_PARAM;
  lpLVItem->iItem = lvHitTestInfo.iItem;
  lpLVItem->iSubItem = 0;

  return LISTVIEW_GetItemT(infoPtr, lpLVItem, TRUE);
}

/***
* DESCRIPTION:
* Called when the mouse is being actively tracked and has hovered for a specified
* amount of time
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] fwKeys : key indicator
* [I] x,y : mouse position
*
* RETURN:
*   0 if the message was processed, non-zero if there was an error
*
* INFO:
* LVS_EX_TRACKSELECT: An item is automatically selected when the cursor remains
* over the item for a certain period of time.
*
*/
static GXLRESULT LISTVIEW_MouseHover(LISTVIEW_INFO *infoPtr, GXWORD fwKeys, GXINT x, GXINT y)
{
  if (infoPtr->dwLvExStyle & LVS_EX_TRACKSELECT)
  {
    GXLVITEMW item;
    GXPOINT pt;

    pt.x = x;
    pt.y = y;

    if (LISTVIEW_GetItemAtPt(infoPtr, &item, pt))
      LISTVIEW_SetSelection(infoPtr, item.iItem);
  }

  return 0;
}

/***
* DESCRIPTION:
* Called whenever WM_MOUSEMOVE is received.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] fwKeys : key indicator
* [I] x,y : mouse position
*
* RETURN:
*   0 if the message is processed, non-zero if there was an error
*/
static GXLRESULT LISTVIEW_MouseMove(LISTVIEW_INFO *infoPtr, GXWORD fwKeys, GXINT x, GXINT y)
{
  GXTRACKMOUSEEVENT trackinfo;

  if (!(fwKeys & GXMK_LBUTTON))
    infoPtr->bLButtonDown = FALSE;

  if (infoPtr->bLButtonDown)
  {
    GXMSG msg;
    GXBOOL skip = FALSE;
    /* Check to see if we got a WM_LBUTTONUP, and skip the gxDragDetect.
    * Otherwise, gxDragDetect will eat it.
    */
    if (gxPeekMessageW(&msg, 0, GXWM_MOUSEFIRST, GXWM_MOUSELAST, GXPM_NOREMOVE))
      if (msg.message == GXWM_LBUTTONUP)
        skip = TRUE;

    if (!skip && gxDragDetect(infoPtr->hwndSelf, &infoPtr->ptClickPos))
    {
      GXLVHITTESTINFO lvHitTestInfo;
      GXNMLISTVIEW nmlv;

      lvHitTestInfo.pt = infoPtr->ptClickPos;
      LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, TRUE);

      ZeroMemory(&nmlv, sizeof(nmlv));
      nmlv.iItem = lvHitTestInfo.iItem;
      nmlv.ptAction = infoPtr->ptClickPos;

      if (!infoPtr->bDragging)
      {
        notify_listview(infoPtr, LVN_BEGINDRAG, &nmlv);
        infoPtr->bDragging = TRUE;
      }

      return 0;
    }
  }
  else
    infoPtr->bLButtonDown = FALSE;

  /* see if we are supposed to be tracking mouse hovering */
  if(infoPtr->dwLvExStyle & LVS_EX_TRACKSELECT) {
    /* fill in the trackinfo struct */
    trackinfo.cbSize = sizeof(GXTRACKMOUSEEVENT);
    trackinfo.dwFlags = GXTME_QUERY;
    trackinfo.hwndTrack = infoPtr->hwndSelf;
    trackinfo.dwHoverTime = infoPtr->dwHoverTime;

    /* see if we are already tracking this hwnd */
    _gxTrackMouseEvent(&trackinfo);

    if(!(trackinfo.dwFlags & GXTME_HOVER)) {
      trackinfo.dwFlags = GXTME_HOVER;

      /* call GXTRACKMOUSEEVENT so we receive WM_MOUSEHOVER messages */
      _gxTrackMouseEvent(&trackinfo);
    }
  }

  return 0;
}


/***
* Tests whether the item is assignable to a list with style lStyle
*/
static inline GXBOOL is_assignable_item(const GXLVITEMW *lpLVItem, GXLONG lStyle)
{
  if ( (lpLVItem->mask & GXLVIF_TEXT) && 
    (lpLVItem->pszText == GXLPSTR_TEXTCALLBACKW) &&
    (lStyle & (LVS_SORTASCENDING | LVS_SORTDESCENDING)) ) return FALSE;

  return TRUE;
}


/***
* DESCRIPTION:
* Helper for LISTVIEW_SetItemT *only*: sets item attributes.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] lpLVItem : valid pointer to new item attributes
* [I] isNew : the item being set is being inserted
* [I] isW : TRUE if lpLVItem is Unicode, FALSE if it's ANSI
* [O] bChanged : will be set to TRUE if the item really changed
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL set_main_item(LISTVIEW_INFO *infoPtr, const GXLVITEMW *lpLVItem, GXBOOL isNew, GXBOOL isW, GXBOOL *bChanged)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  ITEM_INFO *lpItem;
  GXNMLISTVIEW nmlv;
  GXUINT uChanged = 0;
  GXLVITEMW item;

  TRACE("()\n");

  WEAssert(lpLVItem->iItem >= 0 && lpLVItem->iItem < infoPtr->nItemCount);

  if (lpLVItem->mask == 0) return TRUE;   

  if (infoPtr->dwStyle & LVS_OWNERDATA)
  {
    /* a virtual listview only stores selection and focus */
    if (lpLVItem->mask & ~GXLVIF_STATE)
      return FALSE;
    lpItem = NULL;
  }
  else
  {
    GXHDPA hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, lpLVItem->iItem);
    lpItem = (ITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, 0);
    WEAssert (lpItem);
  }

  /* we need to get the lParam and state of the item */
  item.iItem = lpLVItem->iItem;
  item.iSubItem = lpLVItem->iSubItem;
  item.mask = GXLVIF_STATE | GXLVIF_PARAM;
  item.stateMask = ~0;
  item.state = 0;
  item.lParam = 0;
  if (!isNew && !LISTVIEW_GetItemW(infoPtr, &item)) return FALSE;

  TRACE("oldState=%x, newState=%x\n", item.state, lpLVItem->state);
  /* determine what fields will change */    
  if ((lpLVItem->mask & GXLVIF_STATE) && ((item.state ^ lpLVItem->state) & lpLVItem->stateMask & ~infoPtr->uCallbackMask))
    uChanged |= GXLVIF_STATE;

  if ((lpLVItem->mask & GXLVIF_IMAGE) && (lpItem->hdr.iImage != lpLVItem->iImage))
    uChanged |= GXLVIF_IMAGE;

  if ((lpLVItem->mask & GXLVIF_PARAM) && (lpItem->lParam != lpLVItem->lParam))
    uChanged |= GXLVIF_PARAM;

  if ((lpLVItem->mask & GXLVIF_INDENT) && (lpItem->iIndent != lpLVItem->iIndent))
    uChanged |= GXLVIF_INDENT;

  if ((lpLVItem->mask & GXLVIF_TEXT) && textcmpWT(lpItem->hdr.pszText, lpLVItem->pszText, isW))
    uChanged |= GXLVIF_TEXT;

  TRACE("uChanged=0x%x\n", uChanged); 
  if (!uChanged) return TRUE;
  *bChanged = TRUE;

  ZeroMemory(&nmlv, sizeof(GXNMLISTVIEW));
  nmlv.iItem = lpLVItem->iItem;
  nmlv.uNewState = (item.state & ~lpLVItem->stateMask) | (lpLVItem->state & lpLVItem->stateMask);
  nmlv.uOldState = item.state;
  nmlv.uChanged = uChanged;
  nmlv.lParam = item.lParam;

  /* send LVN_ITEMCHANGING notification, if the item is not being inserted */
  /* and we are _NOT_ virtual (LVS_OWNERDATA), and change notifications */
  /* are enabled */
  if(lpItem && !isNew && infoPtr->bDoChangeNotify)
  {
    GXHWND hwndSelf = infoPtr->hwndSelf;

    if (notify_listview(infoPtr, LVN_ITEMCHANGING, &nmlv))
      return FALSE;
    if (!gxIsWindow(hwndSelf))
      return FALSE;
  }

  /* copy information */
  if (lpLVItem->mask & GXLVIF_TEXT)
    textsetptrT(&lpItem->hdr.pszText, lpLVItem->pszText, isW);

  if (lpLVItem->mask & GXLVIF_IMAGE)
    lpItem->hdr.iImage = lpLVItem->iImage;

  if (lpLVItem->mask & GXLVIF_PARAM)
    lpItem->lParam = lpLVItem->lParam;

  if (lpLVItem->mask & GXLVIF_INDENT)
    lpItem->iIndent = lpLVItem->iIndent;

  if (uChanged & GXLVIF_STATE)
  {
    if (lpItem && (lpLVItem->stateMask & ~infoPtr->uCallbackMask & ~(LVIS_FOCUSED | LVIS_SELECTED)))
    {
      lpItem->state &= ~lpLVItem->stateMask;
      lpItem->state |= (lpLVItem->state & lpLVItem->stateMask);
    }
    if (lpLVItem->state & lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_SELECTED)
    {
      if (infoPtr->dwStyle & LVS_SINGLESEL) LISTVIEW_DeselectAllSkipItem(infoPtr, lpLVItem->iItem);
      ranges_additem(infoPtr->selectionRanges, lpLVItem->iItem);
    }
    else if (lpLVItem->stateMask & LVIS_SELECTED)
      ranges_delitem(infoPtr->selectionRanges, lpLVItem->iItem);

    /* if we are asked to change focus, and we manage it, do it */
    if (lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_FOCUSED)
    {
      if (lpLVItem->state & LVIS_FOCUSED)
      {
        LISTVIEW_SetItemFocus(infoPtr, -1);
        infoPtr->nFocusedItem = lpLVItem->iItem;
        LISTVIEW_EnsureVisible(infoPtr, lpLVItem->iItem, uView == LVS_LIST);
      }
      else if (infoPtr->nFocusedItem == lpLVItem->iItem)
        infoPtr->nFocusedItem = -1;
    }
  }

  /* if we're inserting the item, we're done */
  if (isNew) return TRUE;

  /* send LVN_ITEMCHANGED notification */
  if (lpLVItem->mask & GXLVIF_PARAM) nmlv.lParam = lpLVItem->lParam;
  if (infoPtr->bDoChangeNotify) notify_listview(infoPtr, LVN_ITEMCHANGED, &nmlv);

  return TRUE;
}

/***
* DESCRIPTION:
* Helper for LISTVIEW_{Set,Insert}ItemT *only*: sets subitem attributes.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] lpLVItem : valid pointer to new subitem attributes
* [I] isW : TRUE if lpLVItem is Unicode, FALSE if it's ANSI
* [O] bChanged : will be set to TRUE if the item really changed
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL set_sub_item(const LISTVIEW_INFO *infoPtr, const GXLVITEMW *lpLVItem, GXBOOL isW, GXBOOL *bChanged)
{
  GXHDPA hdpaSubItems;
  SUBITEM_INFO *lpSubItem;

  /* we do not support subitems for virtual listviews */
  if (infoPtr->dwStyle & LVS_OWNERDATA) return FALSE;

  /* set subitem only if column is present */
  if (lpLVItem->iSubItem >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;

  /* First do some sanity checks */
  /* The GXLVIF_STATE flag is valid for subitems, but does not appear to be
  particularly useful. We currently do not actually do anything with
  the flag on subitems.
  */
  if (lpLVItem->mask & ~(GXLVIF_TEXT | GXLVIF_IMAGE | GXLVIF_STATE)) return FALSE;
  if (!(lpLVItem->mask & (GXLVIF_TEXT | GXLVIF_IMAGE | GXLVIF_STATE))) return TRUE;

  /* get the subitem structure, and create it if not there */
  hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, lpLVItem->iItem);
  WEAssert (hdpaSubItems);

  lpSubItem = LISTVIEW_GetSubItemPtr(hdpaSubItems, lpLVItem->iSubItem);
  if (!lpSubItem)
  {
    SUBITEM_INFO *tmpSubItem;
    GXINT i;

    lpSubItem = (SUBITEM_INFO *)Alloc(sizeof(SUBITEM_INFO));
    if (!lpSubItem) return FALSE;
    /* we could binary search here, if need be...*/
    for (i = 1; i < gxDPA_GetPtrCount(hdpaSubItems); i++)
    {
      tmpSubItem = (SUBITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, i);
      if (tmpSubItem->iSubItem > lpLVItem->iSubItem) break;
    }
    if (gxDPA_InsertPtr(hdpaSubItems, i, lpSubItem) == -1)
    {
      Free(lpSubItem);
      return FALSE;
    }
    lpSubItem->iSubItem = lpLVItem->iSubItem;
    lpSubItem->hdr.iImage = GXI_IMAGECALLBACK;
    *bChanged = TRUE;
  }

  if (lpLVItem->mask & GXLVIF_IMAGE)
    if (lpSubItem->hdr.iImage != lpLVItem->iImage)
    {
      lpSubItem->hdr.iImage = lpLVItem->iImage;
      *bChanged = TRUE;
    }

    if (lpLVItem->mask & GXLVIF_TEXT)
      if (lpSubItem->hdr.pszText != lpLVItem->pszText)
      {
        textsetptrT(&lpSubItem->hdr.pszText, lpLVItem->pszText, isW);
        *bChanged = TRUE;
      }

      return TRUE;
}

/***
* DESCRIPTION:
* Sets item attributes.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] lpLVItem : new item attributes
* [I] isW : TRUE if lpLVItem is Unicode, FALSE if it's ANSI
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetItemT(LISTVIEW_INFO *infoPtr, GXLVITEMW *lpLVItem, GXBOOL isW)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXHWND hwndSelf = infoPtr->hwndSelf;
  GXLPWSTR pszText = NULL;
  GXBOOL bResult, bChanged = FALSE;

  TRACE("(lpLVItem=%s, isW=%d)\n", debuglvitem_t(lpLVItem, isW), isW);

  if (!lpLVItem || lpLVItem->iItem < 0 || lpLVItem->iItem >= infoPtr->nItemCount)
    return FALSE;

  /* For efficiency, we transform the lpLVItem->pszText to Unicode here */
  if ((lpLVItem->mask & GXLVIF_TEXT) && is_textW(lpLVItem->pszText))
  {
    pszText = lpLVItem->pszText;
    lpLVItem->pszText = textdupTtoW(lpLVItem->pszText, isW);
  }

  /* actually set the fields */
  if (!is_assignable_item(lpLVItem, infoPtr->dwStyle)) return FALSE;

  if (lpLVItem->iSubItem)
    bResult = set_sub_item(infoPtr, lpLVItem, TRUE, &bChanged);
  else
    bResult = set_main_item(infoPtr, lpLVItem, FALSE, TRUE, &bChanged);
  if (!gxIsWindow(hwndSelf))
    return FALSE;

  /* redraw item, if necessary */
  if (bChanged && !infoPtr->bIsDrawing)
  {
    /* this little optimization eliminates some nasty flicker */
    if ( uView == LVS_REPORT && !(infoPtr->dwStyle & LVS_OWNERDRAWFIXED) &&
      !(infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT) &&
      lpLVItem->iSubItem > 0 && lpLVItem->iSubItem <= gxDPA_GetPtrCount(infoPtr->hdpaColumns) )
      LISTVIEW_InvalidateSubItem(infoPtr, lpLVItem->iItem, lpLVItem->iSubItem);
    else
      LISTVIEW_InvalidateItem(infoPtr, lpLVItem->iItem);
  }
  /* restore text */
  if (pszText)
  {
    textfreeT(lpLVItem->pszText, isW);
    lpLVItem->pszText = pszText;
  }

  return bResult;
}

/***
* DESCRIPTION:
* Retrieves the index of the item at coordinate (0, 0) of the client area.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* item index
*/
static GXINT LISTVIEW_GetTopIndex(const LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nItem = 0;
  GXSCROLLINFO scrollInfo;

  scrollInfo.cbSize = sizeof(GXSCROLLINFO);
  scrollInfo.fMask = GXSIF_POS;

  if (uView == LVS_LIST)
  {
    if (gxGetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &scrollInfo))
      nItem = scrollInfo.nPos * LISTVIEW_GetCountPerColumn(infoPtr);
  }
  else if (uView == LVS_REPORT)
  {
    if (gxGetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &scrollInfo))
      nItem = scrollInfo.nPos;
  } 
  else
  {
    if (gxGetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &scrollInfo))
      nItem = LISTVIEW_GetCountPerRow(infoPtr) * (scrollInfo.nPos / infoPtr->nItemHeight);
  }

  TRACE("nItem=%d\n", nItem);

  return nItem;
}


/***
* DESCRIPTION:
* Erases the background of the given rectangle
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] lprcBox : clipping rectangle
*
* RETURN:
*   Success: TRUE
*   Failure: FALSE
*/
static inline GXBOOL LISTVIEW_FillBkgnd(const LISTVIEW_INFO *infoPtr, GXHDC hdc, const GXRECT *lprcBox)
{
  if (!infoPtr->hBkBrush) return FALSE;

  TRACE("(hdc=%p, lprcBox=%s, hBkBrush=%p)\n", hdc, wine_dbgstr_rect(lprcBox), infoPtr->hBkBrush);

  return gxFillRect(hdc, lprcBox, infoPtr->hBkBrush);
}

/***
* DESCRIPTION:
* Draws an item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] nItem : item index
* [I] nSubItem : subitem index
* [I] pos : item position in client coordinates
* [I] cdmode : custom draw mode
*
* RETURN:
*   Success: TRUE
*   Failure: FALSE
*/
static GXBOOL LISTVIEW_DrawItem(LISTVIEW_INFO *infoPtr, GXHDC hdc, GXINT nItem, GXINT nSubItem, GXPOINT pos, GXDWORD cdmode)
{
  GXUINT uFormat, uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  static GXWCHAR szCallback[] = { '(', 'c', 'a', 'l', 'l', 'b', 'a', 'c', 'k', ')', 0 };
  GXDWORD cdsubitemmode = CDRF_DODEFAULT;
  LPGXRECT lprcFocus;
  GXRECT rcSelect, rcBox, rcIcon, rcLabel, rcStateIcon;
  GXNMLVCUSTOMDRAW nmlvcd;
  GXHIMAGELIST himl;
  GXLVITEMW lvItem;
  GXHFONT hOldFont;

  TRACE("(hdc=%p, nItem=%d, nSubItem=%d, pos=%s)\n", hdc, nItem, nSubItem, wine_dbgstr_point(&pos));

  /* get information needed for drawing the item */
  lvItem.mask = GXLVIF_TEXT | GXLVIF_IMAGE | GXLVIF_PARAM;
  if (nSubItem == 0) lvItem.mask |= GXLVIF_STATE;
  if (uView == LVS_REPORT) lvItem.mask |= GXLVIF_INDENT;
  lvItem.stateMask = LVIS_SELECTED | LVIS_FOCUSED | LVIS_STATEIMAGEMASK;
  lvItem.iItem = nItem;
  lvItem.iSubItem = nSubItem;
  lvItem.state = 0;
  lvItem.lParam = 0;
  lvItem.cchTextMax = DISP_TEXT_SIZE;
  lvItem.pszText = szDispText;
  if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) return FALSE;
  if (nSubItem > 0 && (infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT)) 
    lvItem.state = LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED);
  if (lvItem.pszText == GXLPSTR_TEXTCALLBACKW) lvItem.pszText = szCallback;
  TRACE("   lvItem=%s\n", debuglvitem_t(&lvItem, TRUE));

  /* now check if we need to update the focus rectangle */
  lprcFocus = infoPtr->bFocus && (lvItem.state & LVIS_FOCUSED) ? &infoPtr->rcFocus : 0;

  if (!lprcFocus) lvItem.state &= ~LVIS_FOCUSED;
  LISTVIEW_GetItemMetrics(infoPtr, &lvItem, &rcBox, &rcSelect, &rcIcon, &rcStateIcon, &rcLabel);
  gxOffsetRect(&rcBox, pos.x, pos.y);
  gxOffsetRect(&rcSelect, pos.x, pos.y);
  gxOffsetRect(&rcIcon, pos.x, pos.y);
  gxOffsetRect(&rcStateIcon, pos.x, pos.y);
  gxOffsetRect(&rcLabel, pos.x, pos.y);
  TRACE("    rcBox=%s, rcSelect=%s, rcIcon=%s. rcLabel=%s\n",
    wine_dbgstr_rect(&rcBox), wine_dbgstr_rect(&rcSelect),
    wine_dbgstr_rect(&rcIcon), wine_dbgstr_rect(&rcLabel));

  /* fill in the custom draw structure */
  customdraw_fill(&nmlvcd, infoPtr, hdc, &rcBox, &lvItem);

  hOldFont = (GXHFONT)gxGetCurrentObject(hdc, GXOBJ_FONT);
  if (nSubItem > 0) cdmode = infoPtr->cditemmode;
  if (cdmode & CDRF_SKIPDEFAULT) goto postpaint;
  if (cdmode & CDRF_NOTIFYITEMDRAW)
    cdsubitemmode = notify_customdraw(infoPtr, CDDS_PREPAINT, &nmlvcd);
  if (nSubItem == 0) infoPtr->cditemmode = cdsubitemmode;
  if (cdsubitemmode & CDRF_SKIPDEFAULT) goto postpaint;
  /* we have to send a CDDS_SUBITEM customdraw explicitly for subitem 0 */
  if (nSubItem == 0 && cdsubitemmode == CDRF_NOTIFYITEMDRAW)
  {
    cdsubitemmode = notify_customdraw(infoPtr, CDDS_SUBITEM | CDDS_ITEMPREPAINT, &nmlvcd);
    if (cdsubitemmode & CDRF_SKIPDEFAULT) goto postpaint;
  }
  if (nSubItem == 0 || (cdmode & CDRF_NOTIFYITEMDRAW))
    prepaint_setup(infoPtr, hdc, &nmlvcd, FALSE);
  else if ((infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT) == FALSE)
    prepaint_setup(infoPtr, hdc, &nmlvcd, TRUE);

  /* in full row select, subitems, will just use main item's colors */
  if (nSubItem && uView == LVS_REPORT && (infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT))
    nmlvcd.clrTextBk = GXCLR_NONE;

  /* state icons */
  if (infoPtr->himlState && STATEIMAGEINDEX(lvItem.state) && (nSubItem == 0))
  {
    GXUINT uStateImage = STATEIMAGEINDEX(lvItem.state);
    if (uStateImage)
    {
      TRACE("uStateImage=%d\n", uStateImage);
      gxImageList_Draw(infoPtr->himlState, uStateImage - 1, hdc,
        rcStateIcon.left, rcStateIcon.top, ILD_NORMAL);
    }
  }

  /* small icons */
  himl = (uView == LVS_ICON ? infoPtr->himlNormal : infoPtr->himlSmall);
  if (himl && lvItem.iImage >= 0 && !gxIsRectEmpty(&rcIcon))
  {
    TRACE("iImage=%d\n", lvItem.iImage);
    gxImageList_DrawEx(himl, lvItem.iImage, hdc, rcIcon.left, rcIcon.top,
      rcIcon.right - rcIcon.left, rcIcon.bottom - rcIcon.top, infoPtr->clrBk, GXCLR_DEFAULT,
      (lvItem.state & LVIS_SELECTED) && (infoPtr->bFocus) ? ILD_SELECTED : ILD_NORMAL);
  }

  /* Don't bother painting item being edited */
  if (infoPtr->hwndEdit && nItem == infoPtr->nEditLabelItem && nSubItem == 0) goto postpaint;

  /* FIXME: temporary hack */
  rcSelect.left = rcLabel.left;

  /* draw the selection background, if we're drawing the main item */
  if (nSubItem == 0)
  {
    /* in icon mode, the label rect is really what we want to draw the
    * background for */
    if (uView == LVS_ICON)
      rcSelect = rcLabel;

    if (uView == LVS_REPORT && (infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT))
      rcSelect.right = rcBox.right;

    if (nmlvcd.clrTextBk != GXCLR_NONE) 
      gxExtTextOutW(hdc, rcSelect.left, rcSelect.top, GXETO_OPAQUE, &rcSelect, 0, 0, 0);
    if(lprcFocus) *lprcFocus = rcSelect;
  }

  /* figure out the text drawing flags */
  uFormat = (uView == LVS_ICON ? (lprcFocus ? LV_FL_DT_FLAGS : LV_ML_DT_FLAGS) : LV_SL_DT_FLAGS);
  if (uView == LVS_ICON)
    uFormat = (lprcFocus ? LV_FL_DT_FLAGS : LV_ML_DT_FLAGS);
  else if (nSubItem)
  {
    switch (LISTVIEW_GetColumnInfo(infoPtr, nSubItem)->fmt & LVCFMT_JUSTIFYMASK)
    {
    case LVCFMT_RIGHT:  uFormat |= GXDT_RIGHT;  break;
    case LVCFMT_CENTER: uFormat |= GXDT_CENTER; break;
    default:            uFormat |= GXDT_LEFT;
    }
  }
  if (!(uFormat & (GXDT_RIGHT | GXDT_CENTER)))
  {
    if (himl && lvItem.iImage >= 0 && !gxIsRectEmpty(&rcIcon)) rcLabel.left += IMAGE_PADDING;
    else rcLabel.left += LABEL_HOR_PADDING;
  }
  else if (uFormat & GXDT_RIGHT) rcLabel.right -= LABEL_HOR_PADDING;

  /* for GRIDLINES reduce the bottom so the text formats correctly */
  if (uView == LVS_REPORT && infoPtr->dwLvExStyle & LVS_EX_GRIDLINES)
    rcLabel.bottom--;

  gxDrawTextW(hdc, lvItem.pszText, -1, &rcLabel, uFormat);

postpaint:
  if (cdsubitemmode & CDRF_NOTIFYPOSTPAINT)
    notify_postpaint(infoPtr, &nmlvcd);
  if (cdsubitemmode & CDRF_NEWFONT)
    gxSelectObject(hdc, hOldFont);
  return TRUE;
}

/***
* DESCRIPTION:
* Draws listview items when in owner draw mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
*
* RETURN:
* None
*/
static void LISTVIEW_RefreshOwnerDraw(const LISTVIEW_INFO *infoPtr, ITERATOR *i, GXHDC hdc, GXDWORD cdmode)
{
  GXUINT uID = (GXUINT)gxGetWindowLongPtrW(infoPtr->hwndSelf, GXGWLP_ID);
  GXDWORD cditemmode = CDRF_DODEFAULT;
  GXNMLVCUSTOMDRAW nmlvcd;
  GXPOINT Origin, Position;
  GXDRAWITEMSTRUCT dis;
  GXLVITEMW item;

  TRACE("()\n");

  ZeroMemory(&dis, sizeof(dis));

  /* Get scroll info once before loop */
  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* iterate through the invalidated rows */
  while(iterator_next(i))
  {
    item.iItem = i->nItem;
    item.iSubItem = 0;
    item.mask = GXLVIF_PARAM | GXLVIF_STATE;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    if (!LISTVIEW_GetItemW(infoPtr, &item)) continue;

    dis.CtlType = GXODT_LISTVIEW;
    dis.CtlID = uID;
    dis.itemID = item.iItem;
    dis.itemAction = GXODA_DRAWENTIRE;
    dis.itemState = 0;
    if (item.state & LVIS_SELECTED) dis.itemState |= GXODS_SELECTED;
    if (infoPtr->bFocus && (item.state & LVIS_FOCUSED)) dis.itemState |= GXODS_FOCUS;
    dis.hwndItem = infoPtr->hwndSelf;
    dis.hDC = hdc;
    LISTVIEW_GetItemOrigin(infoPtr, dis.itemID, &Position);
    dis.rcItem.left = Position.x + Origin.x;
    dis.rcItem.right = dis.rcItem.left + infoPtr->nItemWidth;
    dis.rcItem.top = Position.y + Origin.y;
    dis.rcItem.bottom = dis.rcItem.top + infoPtr->nItemHeight;
    dis.itemData = item.lParam;

    TRACE("item=%s, rcItem=%s\n", debuglvitem_t(&item, TRUE), wine_dbgstr_rect(&dis.rcItem));

    /*
    * Even if we do not send the CDRF_NOTIFYITEMDRAW we need to fill the nmlvcd
    * structure for the rest. of the paint cycle
    */
    customdraw_fill(&nmlvcd, infoPtr, hdc, &dis.rcItem, &item);
    if (cdmode & CDRF_NOTIFYITEMDRAW)
      cditemmode = notify_customdraw(infoPtr, CDDS_PREPAINT, &nmlvcd);

    if (!(cditemmode & CDRF_SKIPDEFAULT))
    {
      prepaint_setup (infoPtr, hdc, &nmlvcd, FALSE);
      gxSendMessageW(infoPtr->hwndNotify, GXWM_DRAWITEM, dis.CtlID, (GXLPARAM)&dis);
    }

    if (cditemmode & CDRF_NOTIFYPOSTPAINT)
      notify_postpaint(infoPtr, &nmlvcd);
  }
}

/***
* DESCRIPTION:
* Draws listview items when in report display mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] cdmode : custom draw mode
*
* RETURN:
* None
*/
static void LISTVIEW_RefreshReport(LISTVIEW_INFO *infoPtr, ITERATOR *i, GXHDC hdc, GXDWORD cdmode)
{
  GXINT rgntype;
  GXRECT rcClip, rcItem;
  GXPOINT Origin, Position;
  RANGE colRange;
  ITERATOR j;

  TRACE("()\n");

  /* figure out what to draw */
  rgntype = gxGetClipBox(hdc, &rcClip);
  if (rgntype == NULLREGION) return;

  /* Get scroll info once before loop */
  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* narrow down the columns we need to paint */
  for(colRange.lower = 0; colRange.lower < gxDPA_GetPtrCount(infoPtr->hdpaColumns); colRange.lower++)
  {
    LISTVIEW_GetHeaderRect(infoPtr, colRange.lower, &rcItem);
    if (rcItem.right + Origin.x >= rcClip.left) break;
  }
  for(colRange.upper = gxDPA_GetPtrCount(infoPtr->hdpaColumns); colRange.upper > 0; colRange.upper--)
  {
    LISTVIEW_GetHeaderRect(infoPtr, colRange.upper - 1, &rcItem);
    if (rcItem.left + Origin.x < rcClip.right) break;
  }
  iterator_rangeitems(&j, colRange);

  /* in full row select, we _have_ to draw the main item */
  if (infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT)
    j.nSpecial = 0;

  /* iterate through the invalidated rows */
  while(iterator_next(i))
  {
    /* iterate through the invalidated columns */
    while(iterator_next(&j))
    {
      LISTVIEW_GetItemOrigin(infoPtr, i->nItem, &Position);
      Position.x += Origin.x;
      Position.y += Origin.y;

      if (rgntype == GXCOMPLEXREGION && !((infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT) && j.nItem == 0))
      {
        LISTVIEW_GetHeaderRect(infoPtr, j.nItem, &rcItem);
        rcItem.top = 0;
        rcItem.bottom = infoPtr->nItemHeight;
        gxOffsetRect(&rcItem, Position.x, Position.y);
        if (!gxRectVisible(hdc, &rcItem)) continue;
      }

      LISTVIEW_DrawItem(infoPtr, hdc, i->nItem, j.nItem, Position, cdmode);
    }
  }
  iterator_destroy(&j);
}

/***
* DESCRIPTION:
* Draws the gridlines if necessary when in report display mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
*
* RETURN:
* None
*/
static void LISTVIEW_RefreshReportGrid(LISTVIEW_INFO *infoPtr, GXHDC hdc)
{
  GXINT rgntype;
  GXINT y, itemheight;
  GXHPEN hPen, hOldPen;
  GXRECT rcClip, rcItem;
  GXPOINT Origin;
  RANGE colRange;
  ITERATOR j;

  TRACE("()\n");

  /* figure out what to draw */
  rgntype = gxGetClipBox(hdc, &rcClip);
  if (rgntype == NULLREGION) return;

  /* Get scroll info once before loop */
  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* narrow down the columns we need to paint */
  for(colRange.lower = 0; colRange.lower < gxDPA_GetPtrCount(infoPtr->hdpaColumns); colRange.lower++)
  {
    LISTVIEW_GetHeaderRect(infoPtr, colRange.lower, &rcItem);
    if (rcItem.right + Origin.x >= rcClip.left) break;
  }
  for(colRange.upper = gxDPA_GetPtrCount(infoPtr->hdpaColumns); colRange.upper > 0; colRange.upper--)
  {
    LISTVIEW_GetHeaderRect(infoPtr, colRange.upper - 1, &rcItem);
    if (rcItem.left + Origin.x < rcClip.right) break;
  }
  iterator_rangeitems(&j, colRange);

  if ((hPen = gxCreatePen( GXPS_SOLID, 1, comctl32_color.clr3dFace )))
  {
    hOldPen = (GXHPEN)gxSelectObject ( hdc, (GXHGDIOBJ)hPen );

    /* draw the vertical lines for the columns */
    iterator_rangeitems(&j, colRange);
    while(iterator_next(&j))
    {
      LISTVIEW_GetHeaderRect(infoPtr, j.nItem, &rcItem);
      if (rcItem.left == 0) continue; /* skip first column */
      rcItem.left += Origin.x;
      rcItem.right += Origin.x;
      rcItem.top = infoPtr->rcList.top;
      rcItem.bottom = infoPtr->rcList.bottom;
      TRACE("vert col=%d, rcItem=%s\n", j.nItem, wine_dbgstr_rect(&rcItem));
      gxMoveToEx (hdc, rcItem.left, rcItem.top, NULL);
      gxLineTo (hdc, rcItem.left, rcItem.bottom);
    }
    iterator_destroy(&j);

    /* draw the horizontial lines for the rows */
    itemheight =  LISTVIEW_CalculateItemHeight(infoPtr);
    rcItem.left = infoPtr->rcList.left + Origin.x;
    rcItem.right = infoPtr->rcList.right + Origin.x;
    rcItem.bottom = rcItem.top = Origin.y - 1;
    gxMoveToEx(hdc, rcItem.left, rcItem.top, NULL);
    gxLineTo(hdc, rcItem.right, rcItem.top);
    for(y=itemheight-1+Origin.y; y<=infoPtr->rcList.bottom; y+=itemheight)
    {
      rcItem.bottom = rcItem.top = y;
      TRACE("horz rcItem=%s\n", wine_dbgstr_rect(&rcItem));
      gxMoveToEx (hdc, rcItem.left, rcItem.top, NULL);
      gxLineTo (hdc, rcItem.right, rcItem.top);
    }

    gxSelectObject( hdc, hOldPen );
    gxDeleteObject( hPen );
  }
}

/***
* DESCRIPTION:
* Draws listview items when in list display mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] cdmode : custom draw mode
*
* RETURN:
* None
*/
static void LISTVIEW_RefreshList(LISTVIEW_INFO *infoPtr, ITERATOR *i, GXHDC hdc, GXDWORD cdmode)
{
  GXPOINT Origin, Position;

  /* Get scroll info once before loop */
  LISTVIEW_GetOrigin(infoPtr, &Origin);

  while(iterator_prev(i))
  {
    LISTVIEW_GetItemOrigin(infoPtr, i->nItem, &Position);
    Position.x += Origin.x;
    Position.y += Origin.y;

    LISTVIEW_DrawItem(infoPtr, hdc, i->nItem, 0, Position, cdmode);
  }
}


/***
* DESCRIPTION:
* Draws listview items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] prcErase : rect to be erased before refresh (may be NULL)
*
* RETURN:
* NoneX
*/
static void LISTVIEW_Refresh(LISTVIEW_INFO *infoPtr, GXHDC hdc, const GXRECT *prcErase)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXCOLORREF oldTextColor = 0, oldBkColor = 0, oldClrTextBk, oldClrText;
  GXNMLVCUSTOMDRAW nmlvcd;
  GXHFONT hOldFont = 0;
  GXDWORD cdmode;
  GXINT oldBkMode = 0;
  GXRECT rcClient;
  ITERATOR i;
  GXHDC hdcOrig = hdc;
  GXHBITMAP hbmp = NULL;

  LISTVIEW_DUMP(infoPtr);

  if (infoPtr->dwLvExStyle & LVS_EX_DOUBLEBUFFER) {
    TRACE("double buffering\n");

    hdc = gxCreateCompatibleDC(hdcOrig);
    if (!hdc) {
      ERR("Failed to create DC for backbuffer\n");
      return;
    }
    hbmp = gxCreateCompatibleBitmap(hdcOrig, infoPtr->rcList.right,
      infoPtr->rcList.bottom);
    if (!hbmp) {
      ERR("Failed to create bitmap for backbuffer\n");
      gxDeleteDC(hdc);
      return;
    }

    gxSelectObject(hdc, hbmp);
    gxSelectObject(hdc, infoPtr->hFont);
  } else {
    /* Save dc values we're gonna trash while drawing
    * FIXME: Should be done in LISTVIEW_DrawItem() */
    hOldFont = (GXHFONT)gxSelectObject(hdc, infoPtr->hFont);
    oldBkMode = gxGetBkMode(hdc);
    oldBkColor = gxGetBkColor(hdc);
    oldTextColor = gxGetTextColor(hdc);
  }

  infoPtr->bIsDrawing = TRUE;

  if (prcErase) {
    LISTVIEW_FillBkgnd(infoPtr, hdc, prcErase);
  } else if (infoPtr->dwLvExStyle & LVS_EX_DOUBLEBUFFER) {
    /* If no erasing was done (usually because gxRedrawWindow was called
    * with RDW_INVALIDATE only) we need to copy the old contents into
    * the backbuffer before continuing. */
    gxBitBlt(hdc, infoPtr->rcList.left, infoPtr->rcList.top,
      infoPtr->rcList.right - infoPtr->rcList.left,
      infoPtr->rcList.bottom - infoPtr->rcList.top,
      hdcOrig, infoPtr->rcList.left, infoPtr->rcList.top, GXSRCCOPY);
  }

  /* FIXME: Shouldn't need to do this */
  oldClrTextBk = infoPtr->clrTextBk;
  oldClrText   = infoPtr->clrText;

  infoPtr->cditemmode = CDRF_DODEFAULT;

  gxGetClientRect(infoPtr->hwndSelf, &rcClient);
  customdraw_fill(&nmlvcd, infoPtr, hdc, &rcClient, 0);
  cdmode = notify_customdraw(infoPtr, CDDS_PREPAINT, &nmlvcd);
  if (cdmode & CDRF_SKIPDEFAULT) goto enddraw;
  prepaint_setup(infoPtr, hdc, &nmlvcd, FALSE);

  /* Use these colors to draw the items */
  infoPtr->clrTextBk = nmlvcd.clrTextBk;
  infoPtr->clrText = nmlvcd.clrText;

  /* nothing to draw */
  if(infoPtr->nItemCount == 0) goto enddraw;

  /* figure out what we need to draw */
  iterator_visibleitems(&i, infoPtr, hdc);

  /* send cache hint notification */
  if (infoPtr->dwStyle & LVS_OWNERDATA)
  {
    RANGE range = iterator_range(&i);
    GXNMLVCACHEHINT nmlv;

    ZeroMemory(&nmlv, sizeof(GXNMLVCACHEHINT));
    nmlv.iFrom = range.lower;
    nmlv.iTo   = range.upper - 1;
    notify_hdr(infoPtr, LVN_ODCACHEHINT, &nmlv.hdr);
  }

  if ((infoPtr->dwStyle & LVS_OWNERDRAWFIXED) && (uView == LVS_REPORT))
    LISTVIEW_RefreshOwnerDraw(infoPtr, &i, hdc, cdmode);
  else
  {
    if (uView == LVS_REPORT)
      LISTVIEW_RefreshReport(infoPtr, &i, hdc, cdmode);
    else /* LVS_LIST, LVS_ICON or LVS_SMALLICON */
      LISTVIEW_RefreshList(infoPtr, &i, hdc, cdmode);

    /* if we have a focus rect, draw it */
    if (infoPtr->bFocus)
      gxDrawFocusRect(hdc, &infoPtr->rcFocus);
  }
  iterator_destroy(&i);

enddraw:
  /* For LVS_EX_GRIDLINES go and draw lines */
  /*  This includes the case where there were *no* items */
  if ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_REPORT &&
    infoPtr->dwLvExStyle & LVS_EX_GRIDLINES)
    LISTVIEW_RefreshReportGrid(infoPtr, hdc);

  if (cdmode & CDRF_NOTIFYPOSTPAINT)
    notify_postpaint(infoPtr, &nmlvcd);

  infoPtr->clrTextBk = oldClrTextBk;
  infoPtr->clrText = oldClrText;

  if(hbmp) {
    gxBitBlt(hdcOrig, infoPtr->rcList.left, infoPtr->rcList.top,
      infoPtr->rcList.right - infoPtr->rcList.left,
      infoPtr->rcList.bottom - infoPtr->rcList.top,
      hdc, infoPtr->rcList.left, infoPtr->rcList.top, GXSRCCOPY);

    gxDeleteObject(hbmp);
    gxDeleteDC(hdc);
  } else {
    gxSelectObject(hdc, hOldFont);
    gxSetBkMode(hdc, oldBkMode);
    gxSetBkColor(hdc, oldBkColor);
    gxSetTextColor(hdc, oldTextColor);
  }

  infoPtr->bIsDrawing = FALSE;
}


/***
* DESCRIPTION:
* Calculates the approximate width and height of a given number of items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItemCount : number of items
* [I] wWidth : width
* [I] wHeight : height
*
* RETURN:
* Returns a GXDWORD. The width in the low word and the height in high word.
*/
static GXDWORD LISTVIEW_ApproximateViewRect(const LISTVIEW_INFO *infoPtr, GXINT nItemCount,
                      GXWORD wWidth, GXWORD wHeight)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nItemCountPerColumn = 1;
  GXINT nColumnCount = 0;
  GXDWORD dwViewRect = 0;

  if (nItemCount == -1)
    nItemCount = infoPtr->nItemCount;

  if (uView == LVS_LIST)
  {
    if (wHeight == 0xFFFF)
    {
      /* use current height */
      wHeight = infoPtr->rcList.bottom - infoPtr->rcList.top;
    }

    if (wHeight < infoPtr->nItemHeight)
      wHeight = infoPtr->nItemHeight;

    if (nItemCount > 0)
    {
      if (infoPtr->nItemHeight > 0)
      {
        nItemCountPerColumn = wHeight / infoPtr->nItemHeight;
        if (nItemCountPerColumn == 0)
          nItemCountPerColumn = 1;

        if (nItemCount % nItemCountPerColumn != 0)
          nColumnCount = nItemCount / nItemCountPerColumn;
        else
          nColumnCount = nItemCount / nItemCountPerColumn + 1;
      }
    }

    /* Microsoft padding magic */
    wHeight = nItemCountPerColumn * infoPtr->nItemHeight + 2;
    wWidth = nColumnCount * infoPtr->nItemWidth + 2;

    dwViewRect = GXMAKELONG(wWidth, wHeight);
  }
  else if (uView == LVS_REPORT)
  {
    GXRECT rcBox;

    if (infoPtr->nItemCount > 0)
    {
      LISTVIEW_GetItemBox(infoPtr, 0, &rcBox);
      wWidth = rcBox.right - rcBox.left;
      wHeight = (rcBox.bottom - rcBox.top) * nItemCount;
    }
    else
    {
      /* use current height and width */
      if (wHeight == 0xffff)
        wHeight = infoPtr->rcList.bottom - infoPtr->rcList.top;
      if (wWidth == 0xffff)
        wWidth = infoPtr->rcList.right - infoPtr->rcList.left;
    }

    dwViewRect = GXMAKELONG(wWidth, wHeight);
  }
  else if (uView == LVS_SMALLICON) {
    FIXME("uView == LVS_SMALLICON: not implemented\n");
  }
  else if (uView == LVS_ICON) {
    FIXME("uView == LVS_ICON: not implemented\n");
  }

  return dwViewRect;
}


/***
* DESCRIPTION:
* Create a drag image list for the specified item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] iItem   : index of item
* [O] lppt    : Upperr-left corner of the image
*
* RETURN:
* Returns a handle to the image list if successful, NULL otherwise.
*/
static GXHIMAGELIST LISTVIEW_CreateDragImage(LISTVIEW_INFO *infoPtr, GXINT iItem, LPGXPOINT lppt)
{
  GXRECT rcItem;
  GXSIZE size;
  GXPOINT pos;
  GXHDC hdc, hdcOrig;
  GXHBITMAP hbmp, hOldbmp;
  GXHIMAGELIST dragList = 0;
  TRACE("iItem=%d Count=%d\n", iItem, infoPtr->nItemCount);

  if (iItem < 0 || iItem >= infoPtr->nItemCount)
    return 0;

  rcItem.left = LVIR_BOUNDS;
  if (!LISTVIEW_GetItemRect(infoPtr, iItem, &rcItem))
    return 0;

  lppt->x = rcItem.left;
  lppt->y = rcItem.top;

  size.cx = rcItem.right - rcItem.left;
  size.cy = rcItem.bottom - rcItem.top;

  hdcOrig = gxGetDC(infoPtr->hwndSelf);
  hdc = gxCreateCompatibleDC(hdcOrig);
  hbmp = gxCreateCompatibleBitmap(hdcOrig, size.cx, size.cy);
  hOldbmp = (GXHBITMAP)gxSelectObject(hdc, hbmp);

  rcItem.left = rcItem.top = 0;
  rcItem.right = size.cx;
  rcItem.bottom = size.cy;
  gxFillRect(hdc, &rcItem, infoPtr->hBkBrush);

  pos.x = pos.y = 0;
  if (LISTVIEW_DrawItem(infoPtr, hdc, iItem, 0, pos, infoPtr->cditemmode))
  {
    dragList = gxImageList_Create(size.cx, size.cy, GXILC_COLOR, 10, 10);
    gxSelectObject(hdc, hOldbmp);
    gxImageList_Add(dragList, hbmp, 0);
  }
  else
    gxSelectObject(hdc, hOldbmp);

  gxDeleteObject(hbmp);
  gxDeleteDC(hdc);
  gxReleaseDC(infoPtr->hwndSelf, hdcOrig);

  TRACE("ret=%p\n", dragList);

  return dragList;
}


/***
* DESCRIPTION:
* Removes all listview items and subitems.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_DeleteAllItems(LISTVIEW_INFO *infoPtr, GXBOOL destroy)
{
  GXNMLISTVIEW nmlv;
  GXHDPA hdpaSubItems = NULL;
  GXBOOL bSuppress;
  ITEMHDR *hdrItem;
  GXINT i, j;

  TRACE("()\n");

  /* we do it directly, to avoid notifications */
  ranges_clear(infoPtr->selectionRanges);
  infoPtr->nSelectionMark = -1;
  infoPtr->nFocusedItem = -1;
  gxSetRectEmpty(&infoPtr->rcFocus);
  /* But we are supposed to leave nHotItem as is! */


  /* send LVN_DELETEALLITEMS notification */
  ZeroMemory(&nmlv, sizeof(GXNMLISTVIEW));
  nmlv.iItem = -1;
  bSuppress = notify_listview(infoPtr, LVN_DELETEALLITEMS, &nmlv);

  for (i = infoPtr->nItemCount - 1; i >= 0; i--)
  {
    /* send LVN_DELETEITEM notification, if not suppressed */
    if (!bSuppress) notify_deleteitem(infoPtr, i);
    if (!(infoPtr->dwStyle & LVS_OWNERDATA))
    {
      hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, i);
      for (j = 0; j < gxDPA_GetPtrCount(hdpaSubItems); j++)
      {
        hdrItem = (ITEMHDR *)gxDPA_GetPtr(hdpaSubItems, j);
        if (is_textW(hdrItem->pszText)) Free(hdrItem->pszText);
        Free(hdrItem);
      }
      gxDPA_Destroy(hdpaSubItems);
      gxDPA_DeletePtr(infoPtr->hdpaItems, i);
    }
    gxDPA_DeletePtr(infoPtr->hdpaPosX, i);
    gxDPA_DeletePtr(infoPtr->hdpaPosY, i);
    infoPtr->nItemCount --;
  }

  if (!destroy)
  {
    LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);
    LISTVIEW_UpdateScroll(infoPtr);
  }
  LISTVIEW_InvalidateList(infoPtr);

  return TRUE;
}

/***
* DESCRIPTION:
* Scrolls, and updates the columns, when a column is changing width.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn : column to scroll
* [I] dx : amount of scroll, in pixels
*
* RETURN:
*   None.
*/
static void LISTVIEW_ScrollColumns(LISTVIEW_INFO *infoPtr, GXINT nColumn, GXINT dx)
{
  COLUMN_INFO *lpColumnInfo;
  GXRECT rcOld, rcCol;
  GXPOINT ptOrigin;
  GXINT nCol;

  if (nColumn < 0 || gxDPA_GetPtrCount(infoPtr->hdpaColumns) < 1) return;
  lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, min(nColumn, gxDPA_GetPtrCount(infoPtr->hdpaColumns) - 1));
  rcCol = lpColumnInfo->rcHeader;
  if (nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns))
    rcCol.left = rcCol.right;

  /* adjust the other columns */
  for (nCol = nColumn; nCol < gxDPA_GetPtrCount(infoPtr->hdpaColumns); nCol++)
  {
    lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, nCol);
    lpColumnInfo->rcHeader.left += dx;
    lpColumnInfo->rcHeader.right += dx;
  }

  /* do not update screen if not in report mode */
  if (!is_redrawing(infoPtr) || (infoPtr->dwStyle & LVS_TYPEMASK) != LVS_REPORT) return;

  /* if we have a focus, we must first erase the focus rect */
  if (infoPtr->bFocus) LISTVIEW_ShowFocusRect(infoPtr, FALSE);

  /* Need to reset the item width when inserting a new column */
  infoPtr->nItemWidth += dx;

  LISTVIEW_UpdateScroll(infoPtr);
  LISTVIEW_GetOrigin(infoPtr, &ptOrigin);

  /* scroll to cover the deleted column, and invalidate for redraw */
  rcOld = infoPtr->rcList;
  rcOld.left = ptOrigin.x + rcCol.left + dx;
  gxScrollWindowEx(infoPtr->hwndSelf, dx, 0, &rcOld, &rcOld, 0, 0, GXSW_ERASE | GXSW_INVALIDATE);

  /* we can restore focus now */
  if (infoPtr->bFocus) LISTVIEW_ShowFocusRect(infoPtr, TRUE);
}

/***
* DESCRIPTION:
* Removes a column from the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn : column index
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_DeleteColumn(LISTVIEW_INFO *infoPtr, GXINT nColumn)
{
  GXRECT rcCol;

  TRACE("nColumn=%d\n", nColumn);

  if (nColumn < 0 || gxDPA_GetPtrCount(infoPtr->hdpaColumns) == 0
    || nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;

  /* While the MSDN specifically says that column zero should not be deleted,
  what actually happens is that the column itself is deleted but no items or subitems
  are removed.
  */

  LISTVIEW_GetHeaderRect(infoPtr, nColumn, &rcCol);

  if (!gxHeader_DeleteItem(infoPtr->hwndHeader, nColumn))
    return FALSE;

  Free(gxDPA_GetPtr(infoPtr->hdpaColumns, nColumn));
  gxDPA_DeletePtr(infoPtr->hdpaColumns, nColumn);

  if (!(infoPtr->dwStyle & LVS_OWNERDATA) && nColumn)
  {
    SUBITEM_INFO *lpSubItem, *lpDelItem;
    GXHDPA hdpaSubItems;
    GXINT nItem, nSubItem, i;

    for (nItem = 0; nItem < infoPtr->nItemCount; nItem++)
    {
      hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, nItem);
      nSubItem = 0;
      lpDelItem = 0;
      for (i = 1; i < gxDPA_GetPtrCount(hdpaSubItems); i++)
      {
        lpSubItem = (SUBITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, i);
        if (lpSubItem->iSubItem == nColumn)
        {
          nSubItem = i;
          lpDelItem = lpSubItem;
        }
        else if (lpSubItem->iSubItem > nColumn) 
        {
          lpSubItem->iSubItem--;
        }
      }

      /* if we found our subitem, zapp it */  
      if (nSubItem > 0)
      {
        /* free string */
        if (is_textW(lpDelItem->hdr.pszText))
          Free(lpDelItem->hdr.pszText);

        /* free item */
        Free(lpDelItem);

        /* free dpa memory */
        gxDPA_DeletePtr(hdpaSubItems, nSubItem);
      }
    }
  }

  /* update the other column info */
  LISTVIEW_UpdateItemSize(infoPtr);
  if(gxDPA_GetPtrCount(infoPtr->hdpaColumns) == 0)
    LISTVIEW_InvalidateList(infoPtr);
  else
    LISTVIEW_ScrollColumns(infoPtr, nColumn, -(rcCol.right - rcCol.left));

  return TRUE;
}

/***
* DESCRIPTION:
* Invalidates the listview after an item's insertion or deletion.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] dir : -1 if deleting, 1 if inserting
*
* RETURN:
*   None
*/
static void LISTVIEW_ScrollOnInsert(LISTVIEW_INFO *infoPtr, GXINT nItem, GXINT dir)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nPerCol, nItemCol, nItemRow;
  GXRECT rcScroll;
  GXPOINT Origin;

  /* if we don't refresh, what's the point of scrolling? */
  if (!is_redrawing(infoPtr)) return;

  WEAssert (abs(dir) == 1);

  /* arrange icons if autoarrange is on */
  if (is_autoarrange(infoPtr))
  {
    GXBOOL arrange = TRUE;
    if (dir < 0 && nItem >= infoPtr->nItemCount) arrange = FALSE;
    if (dir > 0 && nItem == infoPtr->nItemCount - 1) arrange = FALSE;
    if (arrange) LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);
  }

  /* scrollbars need updating */
  LISTVIEW_UpdateScroll(infoPtr);

  /* figure out the item's position */ 
  if (uView == LVS_REPORT)
    nPerCol = infoPtr->nItemCount + 1;
  else if (uView == LVS_LIST)
    nPerCol = LISTVIEW_GetCountPerColumn(infoPtr);
  else /* LVS_ICON, or LVS_SMALLICON */
    return;

  nItemCol = nItem / nPerCol;
  nItemRow = nItem % nPerCol;
  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* move the items below up a slot */
  rcScroll.left = nItemCol * infoPtr->nItemWidth;
  rcScroll.top = nItemRow * infoPtr->nItemHeight;
  rcScroll.right = rcScroll.left + infoPtr->nItemWidth;
  rcScroll.bottom = nPerCol * infoPtr->nItemHeight;
  gxOffsetRect(&rcScroll, Origin.x, Origin.y);
  TRACE("rcScroll=%s, dx=%d\n", wine_dbgstr_rect(&rcScroll), dir * infoPtr->nItemHeight);
  if (gxIntersectRect(&rcScroll, &rcScroll, &infoPtr->rcList))
  {
    TRACE("Scrolling rcScroll=%s, rcList=%s\n", wine_dbgstr_rect(&rcScroll), wine_dbgstr_rect(&infoPtr->rcList));
    gxScrollWindowEx(infoPtr->hwndSelf, 0, dir * infoPtr->nItemHeight, 
      &rcScroll, &rcScroll, 0, 0, GXSW_ERASE | GXSW_INVALIDATE);
  }

  /* report has only that column, so we're done */
  if (uView == LVS_REPORT) return;

  /* now for LISTs, we have to deal with the columns to the right */
  rcScroll.left = (nItemCol + 1) * infoPtr->nItemWidth;
  rcScroll.top = 0;
  rcScroll.right = (infoPtr->nItemCount / nPerCol + 1) * infoPtr->nItemWidth;
  rcScroll.bottom = nPerCol * infoPtr->nItemHeight;
  gxOffsetRect(&rcScroll, Origin.x, Origin.y);
  if (gxIntersectRect(&rcScroll, &rcScroll, &infoPtr->rcList))
    gxScrollWindowEx(infoPtr->hwndSelf, 0, dir * infoPtr->nItemHeight,
    &rcScroll, &rcScroll, 0, 0, GXSW_ERASE | GXSW_INVALIDATE);
}

/***
* DESCRIPTION:
* Removes an item from the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_DeleteItem(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXLVITEMW item;
  const GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  const GXBOOL is_icon = (uView == LVS_SMALLICON || uView == LVS_ICON);

  TRACE("(nItem=%d)\n", nItem);

  if (nItem < 0 || nItem >= infoPtr->nItemCount) return FALSE;

  /* remove selection, and focus */
  item.state = 0;
  item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
  LISTVIEW_SetItemState(infoPtr, nItem, &item);

  /* send LVN_DELETEITEM notification. */
  if (!notify_deleteitem(infoPtr, nItem)) return FALSE;

  /* we need to do this here, because we'll be deleting stuff */  
  if (is_icon)
    LISTVIEW_InvalidateItem(infoPtr, nItem);

  if (!(infoPtr->dwStyle & LVS_OWNERDATA))
  {
    GXHDPA hdpaSubItems;
    ITEMHDR *hdrItem;
    GXINT i;

    hdpaSubItems = (GXHDPA)gxDPA_DeletePtr(infoPtr->hdpaItems, nItem);  
    for (i = 0; i < gxDPA_GetPtrCount(hdpaSubItems); i++)
    {
      hdrItem = (ITEMHDR *)gxDPA_GetPtr(hdpaSubItems, i);
      if (is_textW(hdrItem->pszText)) Free(hdrItem->pszText);
      Free(hdrItem);
    }
    gxDPA_Destroy(hdpaSubItems);
  }

  if (is_icon)
  {
    gxDPA_DeletePtr(infoPtr->hdpaPosX, nItem);
    gxDPA_DeletePtr(infoPtr->hdpaPosY, nItem);
  }

  infoPtr->nItemCount--;
  LISTVIEW_ShiftIndices(infoPtr, nItem, -1);

  /* now is the invalidation fun */
  if (!is_icon)
    LISTVIEW_ScrollOnInsert(infoPtr, nItem, -1);
  return TRUE;
}


/***
* DESCRIPTION:
* Callback implementation for editlabel control
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] pszText : modified text
* [I] isW : TRUE if psxText is Unicode, FALSE if it's ANSI
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_EndEditLabelT(LISTVIEW_INFO *infoPtr, GXLPWSTR pszText, GXBOOL isW)
{
  GXHWND hwndSelf = infoPtr->hwndSelf;
  GXNMLVDISPINFOW dispInfo;

  TRACE("(pszText=%s, isW=%d)\n", debugtext_t(pszText, isW), isW);

  ZeroMemory(&dispInfo, sizeof(dispInfo));
  dispInfo.item.mask = GXLVIF_PARAM | GXLVIF_STATE;
  dispInfo.item.iItem = infoPtr->nEditLabelItem;
  dispInfo.item.iSubItem = 0;
  dispInfo.item.stateMask = ~0;
  if (!LISTVIEW_GetItemW(infoPtr, &dispInfo.item)) return FALSE;
  /* add the text from the edit in */
  dispInfo.item.mask |= GXLVIF_TEXT;
  dispInfo.item.pszText = pszText;
  dispInfo.item.cchTextMax = textlenT(pszText, isW);

  /* Do we need to update the Item Text */
  if (!notify_dispinfoT(infoPtr, LVN_ENDLABELEDITW, &dispInfo, isW)) return FALSE;
  if (!gxIsWindow(hwndSelf))
    return FALSE;
  if (!pszText) return TRUE;

  if (!(infoPtr->dwStyle & LVS_OWNERDATA))
  {
    GXHDPA hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, infoPtr->nEditLabelItem);
    ITEM_INFO* lpItem = (ITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, 0);
    if (lpItem && lpItem->hdr.pszText == GXLPSTR_TEXTCALLBACKW)
    {
      LISTVIEW_InvalidateItem(infoPtr, infoPtr->nEditLabelItem);
      return TRUE;
    }
  }

  ZeroMemory(&dispInfo, sizeof(dispInfo));
  dispInfo.item.mask = GXLVIF_TEXT;
  dispInfo.item.iItem = infoPtr->nEditLabelItem;
  dispInfo.item.iSubItem = 0;
  dispInfo.item.pszText = pszText;
  dispInfo.item.cchTextMax = textlenT(pszText, isW);
  return LISTVIEW_SetItemT(infoPtr, &dispInfo.item, isW);
}

/***
* DESCRIPTION:
* Begin in place editing of specified list view item
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] isW : TRUE if it's a Unicode req, FALSE if ASCII
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXHWND LISTVIEW_EditLabelT(LISTVIEW_INFO *infoPtr, GXINT nItem, GXBOOL isW)
{
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { 0 };
  GXNMLVDISPINFOW dispInfo;
  GXRECT rect;
  GXHWND hwndSelf = infoPtr->hwndSelf;

  TRACE("(nItem=%d, isW=%d)\n", nItem, isW);

  if (~infoPtr->dwStyle & LVS_EDITLABELS) return 0;
  if (nItem < 0 || nItem >= infoPtr->nItemCount) return 0;

  infoPtr->nEditLabelItem = nItem;

  /* Is the EditBox still there, if so remove it */
  if(infoPtr->hwndEdit != 0)
  {
    gxSetFocus(infoPtr->hwndSelf);
    infoPtr->hwndEdit = 0;
  }

  LISTVIEW_SetSelection(infoPtr, nItem);
  LISTVIEW_SetItemFocus(infoPtr, nItem);
  LISTVIEW_InvalidateItem(infoPtr, nItem);

  rect.left = LVIR_LABEL;
  if (!LISTVIEW_GetItemRect(infoPtr, nItem, &rect)) return 0;

  ZeroMemory(&dispInfo, sizeof(dispInfo));
  dispInfo.item.mask = GXLVIF_PARAM | GXLVIF_STATE | GXLVIF_TEXT;
  dispInfo.item.iItem = nItem;
  dispInfo.item.iSubItem = 0;
  dispInfo.item.stateMask = ~0;
  dispInfo.item.pszText = szDispText;
  dispInfo.item.cchTextMax = DISP_TEXT_SIZE;
  if (!LISTVIEW_GetItemT(infoPtr, &dispInfo.item, isW)) return 0;

  infoPtr->hwndEdit = CreateEditLabelT(infoPtr, dispInfo.item.pszText, GXWS_VISIBLE,
    rect.left-2, rect.top-1, 0, rect.bottom - rect.top+2, isW);
  if (!infoPtr->hwndEdit) return 0;

  if (notify_dispinfoT(infoPtr, LVN_BEGINLABELEDITW, &dispInfo, isW))
  {
    if (!gxIsWindow(hwndSelf))
      return 0;
    gxSendMessageW(infoPtr->hwndEdit, GXWM_CLOSE, 0, 0);
    infoPtr->hwndEdit = 0;
    return 0;
  }

  gxShowWindow(infoPtr->hwndEdit, GXSW_NORMAL);
  gxSetFocus(infoPtr->hwndEdit);
  gxSendMessageW(infoPtr->hwndEdit, GXEM_SETSEL, 0, -1);
  return infoPtr->hwndEdit;
}


/***
* DESCRIPTION:
* Ensures the specified item is visible, scrolling into view if necessary.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] bPartial : partially or entirely visible
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_EnsureVisible(LISTVIEW_INFO *infoPtr, GXINT nItem, GXBOOL bPartial)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nScrollPosHeight = 0;
  GXINT nScrollPosWidth = 0;
  GXINT nHorzAdjust = 0;
  GXINT nVertAdjust = 0;
  GXINT nHorzDiff = 0;
  GXINT nVertDiff = 0;
  GXRECT rcItem, rcTemp;

  rcItem.left = LVIR_BOUNDS;
  if (!LISTVIEW_GetItemRect(infoPtr, nItem, &rcItem)) return FALSE;

  if (bPartial && gxIntersectRect(&rcTemp, &infoPtr->rcList, &rcItem)) return TRUE;

  if (rcItem.left < infoPtr->rcList.left || rcItem.right > infoPtr->rcList.right)
  {
    /* scroll left/right, but in LVS_REPORT mode */
    if (uView == LVS_LIST)
      nScrollPosWidth = infoPtr->nItemWidth;
    else if ((uView == LVS_SMALLICON) || (uView == LVS_ICON))
      nScrollPosWidth = 1;

    if (rcItem.left < infoPtr->rcList.left)
    {
      nHorzAdjust = -1;
      if (uView != LVS_REPORT) nHorzDiff = rcItem.left - infoPtr->rcList.left;
    }
    else
    {
      nHorzAdjust = 1;
      if (uView != LVS_REPORT) nHorzDiff = rcItem.right - infoPtr->rcList.right;
    }
  }

  if (rcItem.top < infoPtr->rcList.top || rcItem.bottom > infoPtr->rcList.bottom)
  {
    /* scroll up/down, but not in LVS_LIST mode */
    if (uView == LVS_REPORT)
      nScrollPosHeight = infoPtr->nItemHeight;
    else if ((uView == LVS_ICON) || (uView == LVS_SMALLICON))
      nScrollPosHeight = 1;

    if (rcItem.top < infoPtr->rcList.top)
    {
      nVertAdjust = -1;
      if (uView != LVS_LIST) nVertDiff = rcItem.top - infoPtr->rcList.top;
    }
    else
    {
      nVertAdjust = 1;
      if (uView != LVS_LIST) nVertDiff = rcItem.bottom - infoPtr->rcList.bottom;
    }
  }

  if (!nScrollPosWidth && !nScrollPosHeight) return TRUE;

  if (nScrollPosWidth)
  {
    GXINT diff = nHorzDiff / nScrollPosWidth;
    if (nHorzDiff % nScrollPosWidth) diff += nHorzAdjust;
    LISTVIEW_HScroll(infoPtr, SB_INTERNAL, diff, 0);
  }

  if (nScrollPosHeight)
  {
    GXINT diff = nVertDiff / nScrollPosHeight;
    if (nVertDiff % nScrollPosHeight) diff += nVertAdjust;
    LISTVIEW_VScroll(infoPtr, SB_INTERNAL, diff, 0);
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Searches for an item with specific characteristics.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] nStart : base item index
* [I] lpFindInfo : item information to look for
*
* RETURN:
*   SUCCESS : index of item
*   FAILURE : -1
*/
static GXINT LISTVIEW_FindItemW(const LISTVIEW_INFO *infoPtr, GXINT nStart,
                const GXLVFINDINFOW *lpFindInfo)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  GXBOOL bWrap = FALSE, bNearest = FALSE;
  GXINT nItem = nStart + 1, nLast = infoPtr->nItemCount, nNearestItem = -1;
  GXULONG xdist, ydist, dist, mindist = 0x7fffffff;
  GXPOINT Position, Destination;
  GXLVITEMW lvItem;

  if (!lpFindInfo || nItem < 0) return -1;

  lvItem.mask = 0;
  if (lpFindInfo->flags & (LVFI_STRING | LVFI_PARTIAL))
  {
    lvItem.mask |= GXLVIF_TEXT;
    lvItem.pszText = szDispText;
    lvItem.cchTextMax = DISP_TEXT_SIZE;
  }

  if (lpFindInfo->flags & LVFI_WRAP)
    bWrap = TRUE;

  if ((lpFindInfo->flags & LVFI_NEARESTXY) && 
    (uView == LVS_ICON || uView ==LVS_SMALLICON))
  {
    GXPOINT Origin;
    GXRECT rcArea;

    LISTVIEW_GetOrigin(infoPtr, &Origin);
    Destination.x = lpFindInfo->pt.x - Origin.x;
    Destination.y = lpFindInfo->pt.y - Origin.y;
    switch(lpFindInfo->vkDirection)
    {
    case GXVK_DOWN:  Destination.y += infoPtr->nItemHeight; break;
    case GXVK_UP:    Destination.y -= infoPtr->nItemHeight; break;
    case GXVK_RIGHT: Destination.x += infoPtr->nItemWidth; break;
    case GXVK_LEFT:  Destination.x -= infoPtr->nItemWidth; break;
    case GXVK_HOME:  Destination.x = Destination.y = 0; break;
    case GXVK_NEXT:  Destination.y += infoPtr->rcList.bottom - infoPtr->rcList.top; break;
    case GXVK_PRIOR: Destination.y -= infoPtr->rcList.bottom - infoPtr->rcList.top; break;
    case GXVK_END:
      LISTVIEW_GetAreaRect(infoPtr, &rcArea);
      Destination.x = rcArea.right; 
      Destination.y = rcArea.bottom; 
      break;
    default: ERR("Unknown vkDirection=%d\n", lpFindInfo->vkDirection);
    }
    bNearest = TRUE;
  }
  else Destination.x = Destination.y = 0;

  /* if LVFI_PARAM is specified, all other flags are ignored */
  if (lpFindInfo->flags & LVFI_PARAM)
  {
    lvItem.mask |= GXLVIF_PARAM;
    bNearest = FALSE;
    lvItem.mask &= ~GXLVIF_TEXT;
  }

again:
  for (; nItem < nLast; nItem++)
  {
    lvItem.iItem = nItem;
    lvItem.iSubItem = 0;
    if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) continue;

    if (lvItem.mask & GXLVIF_PARAM)
    {
      if (lpFindInfo->lParam == lvItem.lParam)
        return nItem;
      else
        continue;
    }

    if (lvItem.mask & GXLVIF_TEXT)
    {
      if (lpFindInfo->flags & LVFI_PARTIAL)
      {
        if (strstrW(lvItem.pszText, lpFindInfo->psz) == NULL) continue;
      }
      else
      {
        if (GXSTRCMP(lvItem.pszText, lpFindInfo->psz) != 0) continue;
      }
    }

    if (!bNearest) return nItem;

    /* This is very inefficient. To do a good job here,
    * we need a sorted array of (x,y) item positions */
    LISTVIEW_GetItemOrigin(infoPtr, nItem, &Position);

    /* compute the distance^2 to the destination */
    xdist = Destination.x - Position.x;
    ydist = Destination.y - Position.y;
    dist = xdist * xdist + ydist * ydist;

    /* remember the distance, and item if it's closer */
    if (dist < mindist)
    {
      mindist = dist;
      nNearestItem = nItem;
    }
  }

  if (bWrap)
  {
    nItem = 0;
    nLast = min(nStart + 1, infoPtr->nItemCount);
    bWrap = FALSE;
    goto again;
  }

  return nNearestItem;
}

/***
* DESCRIPTION:
* Searches for an item with specific characteristics.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] nStart : base item index
* [I] lpFindInfo : item information to look for
*
* RETURN:
*   SUCCESS : index of item
*   FAILURE : -1
*/
//static GXINT LISTVIEW_FindItemA(const LISTVIEW_INFO *infoPtr, GXINT nStart,
//                              const LVFINDINFOA *lpFindInfo)
//{
//    GXBOOL hasText = lpFindInfo->flags & (LVFI_STRING | LVFI_PARTIAL);
//    GXLVFINDINFOW fiw;
//    GXINT res;
//    GXLPWSTR strW = NULL;
//
//    memcpy(&fiw, lpFindInfo, sizeof(fiw));
//    if (hasText) fiw.psz = strW = textdupTtoW((GXLPCWSTR)lpFindInfo->psz, FALSE);
//    res = LISTVIEW_FindItemW(infoPtr, nStart, &fiw);
//    textfreeT(strW, FALSE);
//    return res;
//}

/***
* DESCRIPTION:
* Retrieves the background image of the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lpBkImage : background image attributes
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
/* static GXBOOL LISTVIEW_GetBkImage(const LISTVIEW_INFO *infoPtr, LPLVBKIMAGE lpBkImage)   */
/* {   */
/*   FIXME (listview, "empty stub!\n"); */
/*   return FALSE;   */
/* }   */

/***
* DESCRIPTION:
* Retrieves column attributes.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn :  column index
* [IO] lpColumn : column information
* [I] isW : if TRUE, then lpColumn is a GXLPLVCOLUMNW
*           otherwise it is in fact a LPLVCOLUMNA
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_GetColumnT(const LISTVIEW_INFO *infoPtr, GXINT nColumn, GXLPLVCOLUMNW lpColumn, GXBOOL isW)
{
  COLUMN_INFO *lpColumnInfo;
  GXHDITEMW hdi;

  if (!lpColumn || nColumn < 0 || nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;
  lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, nColumn);

  /* initialize memory */
  ZeroMemory(&hdi, sizeof(hdi));

  if (lpColumn->mask & LVCF_TEXT)
  {
    hdi.mask |= HDI_TEXT;
    hdi.pszText = lpColumn->pszText;
    hdi.cchTextMax = lpColumn->cchTextMax;
  }

  if (lpColumn->mask & LVCF_IMAGE)
    hdi.mask |= HDI_IMAGE;

  if (lpColumn->mask & LVCF_ORDER)
    hdi.mask |= HDI_ORDER;

  if (lpColumn->mask & LVCF_SUBITEM)
    hdi.mask |= HDI_LPARAM;

  if (!gxSendMessageW(infoPtr->hwndHeader, isW ? GXHDM_GETITEMW : GXHDM_GETITEMA, nColumn, (GXLPARAM)&hdi)) return FALSE;

  if (lpColumn->mask & LVCF_FMT)
    lpColumn->fmt = lpColumnInfo->fmt;

  if (lpColumn->mask & LVCF_WIDTH)
    lpColumn->cx = lpColumnInfo->rcHeader.right - lpColumnInfo->rcHeader.left;

  if (lpColumn->mask & LVCF_IMAGE)
    lpColumn->iImage = hdi.iImage;

  if (lpColumn->mask & LVCF_ORDER)
    lpColumn->iOrder = hdi.iOrder;

  if (lpColumn->mask & LVCF_SUBITEM)
    lpColumn->iSubItem = hdi.lParam;

  return TRUE;
}


static GXBOOL LISTVIEW_GetColumnOrderArray(const LISTVIEW_INFO *infoPtr, GXINT iCount, GXLPINT lpiArray)
{
  GXINT i;

  if (!lpiArray)
    return FALSE;

  /* FIXME: little hack */
  for (i = 0; i < iCount; i++)
    lpiArray[i] = i;

  return TRUE;
}

/***
* DESCRIPTION:
* Retrieves the column width.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] int : column index
*
* RETURN:
*   SUCCESS : column width
*   FAILURE : zero
*/
static GXINT LISTVIEW_GetColumnWidth(const LISTVIEW_INFO *infoPtr, GXINT nColumn)
{
  GXINT nColumnWidth = 0;
  GXHDITEMW hdItem;

  TRACE("nColumn=%d\n", nColumn);

  /* we have a 'column' in LIST and REPORT mode only */
  switch(infoPtr->dwStyle & LVS_TYPEMASK)
  {
  case LVS_LIST:
    nColumnWidth = infoPtr->nItemWidth;
    break;
  case LVS_REPORT:
    /* We are not using LISTVIEW_GetHeaderRect as this data is updated only after a GXHDM_ITEMCHANGED.
    * There is an application that subclasses the listview, calls GXLVM_GETCOLUMNWIDTH in the
    * GXHDM_ITEMCHANGED handler and goes into infinite recursion if it receives old data.
    *
    * TODO: should we do the same in GXLVM_GETCOLUMN?
    */
    hdItem.mask = HDI_WIDTH;
    if (!gxSendMessageW(infoPtr->hwndHeader, GXHDM_GETITEMW, nColumn, (GXLPARAM)&hdItem))
    {
      WARN("(%p): GXHDM_GETITEMW failed for item %d\n", infoPtr->hwndSelf, nColumn);
      return 0;
    }
    nColumnWidth = hdItem.cxy;
    break;
  }

  TRACE("nColumnWidth=%d\n", nColumnWidth);
  return nColumnWidth;
}

/***
* DESCRIPTION:
* In list or report display mode, retrieves the number of items that can fit
* vertically in the visible area. In icon or small icon display mode,
* retrieves the total number of visible items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Number of fully visible items.
*/
static GXINT LISTVIEW_GetCountPerPage(const LISTVIEW_INFO *infoPtr)
{
  switch (infoPtr->dwStyle & LVS_TYPEMASK)
  {
  case LVS_ICON:
  case LVS_SMALLICON:
    return infoPtr->nItemCount;
  case LVS_REPORT:
    return LISTVIEW_GetCountPerColumn(infoPtr);
  case LVS_LIST:
    return LISTVIEW_GetCountPerRow(infoPtr) * LISTVIEW_GetCountPerColumn(infoPtr);
  }
  WEAssert(FALSE);
  return 0;
}

/***
* DESCRIPTION:
* Retrieves an image list handle.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nImageList : image list identifier
*
* RETURN:
*   SUCCESS : image list handle
*   FAILURE : NULL
*/
static GXHIMAGELIST LISTVIEW_GetImageList(const LISTVIEW_INFO *infoPtr, GXINT nImageList)
{
  switch (nImageList)
  {
  case LVSIL_NORMAL: return infoPtr->himlNormal;
  case LVSIL_SMALL: return infoPtr->himlSmall;
  case LVSIL_STATE: return infoPtr->himlState;
  }
  return NULL;
}

/* LISTVIEW_GetISearchString */

/***
* DESCRIPTION:
* Retrieves item attributes.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [IO] lpLVItem : item info
* [I] isW : if TRUE, then lpLVItem is a GXLPLVITEMW,
*           if FALSE, then lpLVItem is a LPLVITEMA.
*
* NOTE:
*   This is the internal 'GetItem' interface -- it tries to
*   be smart and avoid text copies, if possible, by modifying
*   lpLVItem->pszText to point to the text string. Please note
*   that this is not always possible (e.g. OWNERDATA), so on
*   entry you *must* supply valid values for pszText, and cchTextMax.
*   The only difference to the documented interface is that upon
*   return, you should use *only* the lpLVItem->pszText, rather than
*   the buffer pointer you provided on input. Most code already does
*   that, so it's not a problem.
*   For the two cases when the text must be copied (that is,
*   for GXLVM_GETITEM, and GXLVM_GETITEMTEXT), use LISTVIEW_GetItemExtT.
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_GetItemT(const LISTVIEW_INFO *infoPtr, GXLPLVITEMW lpLVItem, GXBOOL isW)
{
  ITEMHDR callbackHdr = { GXLPSTR_TEXTCALLBACKW, GXI_IMAGECALLBACK };
  GXNMLVDISPINFOW dispInfo;
  ITEM_INFO *lpItem;
  ITEMHDR* pItemHdr;
  GXHDPA hdpaSubItems;
  GXINT isubitem;

  TRACE("(lpLVItem=%s, isW=%d)\n", debuglvitem_t(lpLVItem, isW), isW);

  if (!lpLVItem || lpLVItem->iItem < 0 || lpLVItem->iItem >= infoPtr->nItemCount)
    return FALSE;

  if (lpLVItem->mask == 0) return TRUE;

  /* make a local copy */
  isubitem = lpLVItem->iSubItem;

  /* a quick optimization if all we're asked is the focus state
  * these queries are worth optimising since they are common,
  * and can be answered in constant time, without the heavy accesses */
  if ( (lpLVItem->mask == GXLVIF_STATE) && (lpLVItem->stateMask == LVIS_FOCUSED) &&
    !(infoPtr->uCallbackMask & LVIS_FOCUSED) )
  {
    lpLVItem->state = 0;
    if (infoPtr->nFocusedItem == lpLVItem->iItem)
      lpLVItem->state |= LVIS_FOCUSED;
    return TRUE;
  }

  ZeroMemory(&dispInfo, sizeof(dispInfo));

  /* if the app stores all the data, handle it separately */
  if (infoPtr->dwStyle & LVS_OWNERDATA)
  {
    dispInfo.item.state = 0;

    /* apparently, we should not callback for lParam in LVS_OWNERDATA */
    if ((lpLVItem->mask & ~(GXLVIF_STATE | GXLVIF_PARAM)) || infoPtr->uCallbackMask)
    {
      /* NOTE: copy only fields which we _know_ are initialized, some apps
      *       depend on the uninitialized fields being 0 */
      dispInfo.item.mask = lpLVItem->mask & ~GXLVIF_PARAM;
      dispInfo.item.iItem = lpLVItem->iItem;
      dispInfo.item.iSubItem = isubitem;
      if (lpLVItem->mask & GXLVIF_TEXT)
      {
        dispInfo.item.pszText = lpLVItem->pszText;
        dispInfo.item.cchTextMax = lpLVItem->cchTextMax;    
      }
      if (lpLVItem->mask & GXLVIF_STATE)
        dispInfo.item.stateMask = lpLVItem->stateMask & infoPtr->uCallbackMask;
      notify_dispinfoT(infoPtr, LVN_GETDISPINFOW, &dispInfo, isW);
      dispInfo.item.stateMask = lpLVItem->stateMask;
      if (lpLVItem->mask & (GXLVIF_GROUPID|GXLVIF_COLUMNS))
      {
        /* full size structure expected - _WIN32IE >= 0x560 */
        *lpLVItem = dispInfo.item;
      }
      else if (lpLVItem->mask & GXLVIF_INDENT)
      {
        /* indent member expected - _WIN32IE >= 0x300 */
        memcpy(lpLVItem, &dispInfo.item, offsetof( GXLVITEMW, iGroupId ));
      }
      else
      {
        /* minimal structure expected */
        memcpy(lpLVItem, &dispInfo.item, offsetof( GXLVITEMW, iIndent ));
      }
      TRACE("   getdispinfo(1):lpLVItem=%s\n", debuglvitem_t(lpLVItem, isW));
    }

    /* make sure lParam is zeroed out */
    if (lpLVItem->mask & GXLVIF_PARAM) lpLVItem->lParam = 0;

    /* we store only a little state, so if we're not asked, we're done */
    if (!(lpLVItem->mask & GXLVIF_STATE) || isubitem) return TRUE;

    /* if focus is handled by us, report it */
    if ( lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_FOCUSED ) 
    {
      lpLVItem->state &= ~LVIS_FOCUSED;
      if (infoPtr->nFocusedItem == lpLVItem->iItem)
        lpLVItem->state |= LVIS_FOCUSED;
    }

    /* and do the same for selection, if we handle it */
    if ( lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_SELECTED ) 
    {
      lpLVItem->state &= ~LVIS_SELECTED;
      if (ranges_contain(infoPtr->selectionRanges, lpLVItem->iItem))
        lpLVItem->state |= LVIS_SELECTED;
    }

    return TRUE;
  }

  /* find the item and subitem structures before we proceed */
  hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, lpLVItem->iItem);
  lpItem = (ITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, 0);
  WEAssert (lpItem);

  if (isubitem)
  {
    SUBITEM_INFO *lpSubItem = LISTVIEW_GetSubItemPtr(hdpaSubItems, isubitem);
    pItemHdr = lpSubItem ? &lpSubItem->hdr : &callbackHdr;
    if (!lpSubItem)
    {
      WARN(" iSubItem invalid (%08x), ignored.\n", isubitem);
      isubitem = 0;
    }
  }
  else
    pItemHdr = &lpItem->hdr;

  /* Do we need to query the state from the app? */
  if ((lpLVItem->mask & GXLVIF_STATE) && infoPtr->uCallbackMask && isubitem == 0)
  {
    dispInfo.item.mask |= GXLVIF_STATE;
    dispInfo.item.stateMask = infoPtr->uCallbackMask;
  }

  /* Do we need to enquire about the image? */
  if ((lpLVItem->mask & GXLVIF_IMAGE) && pItemHdr->iImage == GXI_IMAGECALLBACK &&
    (isubitem == 0 || (infoPtr->dwLvExStyle & LVS_EX_SUBITEMIMAGES)))
  {
    dispInfo.item.mask |= GXLVIF_IMAGE;
    dispInfo.item.iImage = GXI_IMAGECALLBACK;
  }

  /* Apps depend on calling back for text if it is NULL or GXLPSTR_TEXTCALLBACKW */
  if ((lpLVItem->mask & GXLVIF_TEXT) && !is_textW(pItemHdr->pszText))
  {
    dispInfo.item.mask |= GXLVIF_TEXT;
    dispInfo.item.pszText = lpLVItem->pszText;
    dispInfo.item.cchTextMax = lpLVItem->cchTextMax;
    if (dispInfo.item.pszText && dispInfo.item.cchTextMax > 0)
      *dispInfo.item.pszText = '\0';
  }

  /* If we don't have all the requested info, query the application */
  if (dispInfo.item.mask != 0)
  {
    dispInfo.item.iItem = lpLVItem->iItem;
    dispInfo.item.iSubItem = lpLVItem->iSubItem; /* yes: the original subitem */
    dispInfo.item.lParam = lpItem->lParam;
    notify_dispinfoT(infoPtr, LVN_GETDISPINFOW, &dispInfo, isW);
    TRACE("   getdispinfo(2):item=%s\n", debuglvitem_t(&dispInfo.item, isW));
  }

  /* we should not store values for subitems */
  if (isubitem) dispInfo.item.mask &= ~GXLVIF_DI_SETITEM;

  /* Now, handle the iImage field */
  if (dispInfo.item.mask & GXLVIF_IMAGE)
  {
    lpLVItem->iImage = dispInfo.item.iImage;
    if ((dispInfo.item.mask & GXLVIF_DI_SETITEM) && pItemHdr->iImage == GXI_IMAGECALLBACK)
      pItemHdr->iImage = dispInfo.item.iImage;
  }
  else if (lpLVItem->mask & GXLVIF_IMAGE)
  {
    if(isubitem == 0 || (infoPtr->dwLvExStyle & LVS_EX_SUBITEMIMAGES))
      lpLVItem->iImage = pItemHdr->iImage;
    else
      lpLVItem->iImage = 0;
  }

  /* The pszText field */
  if (dispInfo.item.mask & GXLVIF_TEXT)
  {
    if ((dispInfo.item.mask & GXLVIF_DI_SETITEM) && pItemHdr->pszText)
      textsetptrT(&pItemHdr->pszText, dispInfo.item.pszText, isW);

    lpLVItem->pszText = dispInfo.item.pszText;
  }
  else if (lpLVItem->mask & GXLVIF_TEXT)
  {
    if (isW) lpLVItem->pszText = pItemHdr->pszText;
    else textcpynT(lpLVItem->pszText, isW, pItemHdr->pszText, TRUE, lpLVItem->cchTextMax);
  }

  /* Next is the lParam field */
  if (dispInfo.item.mask & GXLVIF_PARAM)
  {
    lpLVItem->lParam = dispInfo.item.lParam;
    if ((dispInfo.item.mask & GXLVIF_DI_SETITEM))
      lpItem->lParam = dispInfo.item.lParam;
  }
  else if (lpLVItem->mask & GXLVIF_PARAM)
    lpLVItem->lParam = lpItem->lParam;

  /* if this is a subitem, we're done */
  if (isubitem) return TRUE;

  /* ... the state field (this one is different due to uCallbackmask) */
  if (lpLVItem->mask & GXLVIF_STATE)
  {
    lpLVItem->state = lpItem->state & lpLVItem->stateMask;
    if (dispInfo.item.mask & GXLVIF_STATE)
    {
      lpLVItem->state &= ~dispInfo.item.stateMask;
      lpLVItem->state |= (dispInfo.item.state & dispInfo.item.stateMask);
    }
    if ( lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_FOCUSED ) 
    {
      lpLVItem->state &= ~LVIS_FOCUSED;
      if (infoPtr->nFocusedItem == lpLVItem->iItem)
        lpLVItem->state |= LVIS_FOCUSED;
    }
    if ( lpLVItem->stateMask & ~infoPtr->uCallbackMask & LVIS_SELECTED ) 
    {
      lpLVItem->state &= ~LVIS_SELECTED;
      if (ranges_contain(infoPtr->selectionRanges, lpLVItem->iItem))
        lpLVItem->state |= LVIS_SELECTED;
    }      
  }

  /* and last, but not least, the indent field */
  if (lpLVItem->mask & GXLVIF_INDENT)
    lpLVItem->iIndent = lpItem->iIndent;

  return TRUE;
}

/***
* DESCRIPTION:
* Retrieves item attributes.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [IO] lpLVItem : item info
* [I] isW : if TRUE, then lpLVItem is a GXLPLVITEMW,
*           if FALSE, then lpLVItem is a LPLVITEMA.
*
* NOTE:
*   This is the external 'GetItem' interface -- it properly copies
*   the text in the provided buffer.
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_GetItemExtT(const LISTVIEW_INFO *infoPtr, GXLPLVITEMW lpLVItem, GXBOOL isW)
{
  GXLPWSTR pszText;
  GXBOOL bResult;

  if (!lpLVItem || lpLVItem->iItem < 0 || lpLVItem->iItem >= infoPtr->nItemCount)
    return FALSE;

  pszText = lpLVItem->pszText;
  bResult = LISTVIEW_GetItemT(infoPtr, lpLVItem, isW);
  if (bResult && lpLVItem->pszText != pszText)
    textcpynT(pszText, isW, lpLVItem->pszText, isW, lpLVItem->cchTextMax);
  lpLVItem->pszText = pszText;

  return bResult;
}


/***
* DESCRIPTION:
* Retrieves the position (upper-left) of the listview control item.
* Note that for LVS_ICON style, the upper-left is that of the icon
* and not the bounding box.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [O] lpptPosition : coordinate information
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_GetItemPosition(const LISTVIEW_INFO *infoPtr, GXINT nItem, LPGXPOINT lpptPosition)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXPOINT Origin;

  TRACE("(nItem=%d, lpptPosition=%p)\n", nItem, lpptPosition);

  if (!lpptPosition || nItem < 0 || nItem >= infoPtr->nItemCount) return FALSE;

  LISTVIEW_GetOrigin(infoPtr, &Origin);
  LISTVIEW_GetItemOrigin(infoPtr, nItem, lpptPosition);

  if (uView == LVS_ICON)
  {
    lpptPosition->x += (infoPtr->nItemWidth - infoPtr->iconSize.cx) / 2;
    lpptPosition->y += ICON_TOP_PADDING;
  }
  lpptPosition->x += Origin.x;
  lpptPosition->y += Origin.y;

  TRACE ("  lpptPosition=%s\n", wine_dbgstr_point(lpptPosition));
  return TRUE;
}


/***
* DESCRIPTION:
* Retrieves the bounding rectangle for a listview control item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [IO] lprc : bounding rectangle coordinates
*     lprc->left specifies the portion of the item for which the bounding
*     rectangle will be retrieved.
*
*     LVIR_BOUNDS Returns the bounding rectangle of the entire item,
*        including the icon and label.
*         *
*         * For LVS_ICON
*         * Experiment shows that native control returns:
*         *  width = min (48, length of text line)
*         *    .left = position.x - (width - iconsize.cx)/2
*         *    .right = .left + width
*         *  height = #lines of text * ntmHeight + icon height + 8
*         *    .top = position.y - 2
*         *    .bottom = .top + height
*         *  separation between items .y = itemSpacing.cy - height
*         *                           .x = itemSpacing.cx - width
*     LVIR_ICON Returns the bounding rectangle of the icon or small icon.
*         *
*         * For LVS_ICON
*         * Experiment shows that native control returns:
*         *  width = iconSize.cx + 16
*         *    .left = position.x - (width - iconsize.cx)/2
*         *    .right = .left + width
*         *  height = iconSize.cy + 4
*         *    .top = position.y - 2
*         *    .bottom = .top + height
*         *  separation between items .y = itemSpacing.cy - height
*         *                           .x = itemSpacing.cx - width
*     LVIR_LABEL Returns the bounding rectangle of the item text.
*         *
*         * For LVS_ICON
*         * Experiment shows that native control returns:
*         *  width = text length
*         *    .left = position.x - width/2
*         *    .right = .left + width
*         *  height = ntmH * linecount + 2
*         *    .top = position.y + iconSize.cy + 6
*         *    .bottom = .top + height
*         *  separation between items .y = itemSpacing.cy - height
*         *                           .x = itemSpacing.cx - width
*     LVIR_SELECTBOUNDS Returns the union of the LVIR_ICON and LVIR_LABEL
*  rectangles, but excludes columns in report view.
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*
* NOTES
*   Note that the bounding rectangle of the label in the LVS_ICON view depends
*   upon whether the window has the focus currently and on whether the item
*   is the one with the focus.  Ensure that the control's record of which
*   item has the focus agrees with the items' records.
*/
static GXBOOL LISTVIEW_GetItemRect(const LISTVIEW_INFO *infoPtr, GXINT nItem, LPGXRECT lprc)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  GXBOOL doLabel = TRUE, oversizedBox = FALSE;
  GXPOINT Position, Origin;
  GXLVITEMW lvItem;

  TRACE("(hwnd=%p, nItem=%d, lprc=%p)\n", infoPtr->hwndSelf, nItem, lprc);

  if (!lprc || nItem < 0 || nItem >= infoPtr->nItemCount) return FALSE;

  LISTVIEW_GetOrigin(infoPtr, &Origin);
  LISTVIEW_GetItemOrigin(infoPtr, nItem, &Position);

  /* Be smart and try to figure out the minimum we have to do */
  if (lprc->left == LVIR_ICON) doLabel = FALSE;
  if (uView == LVS_REPORT && lprc->left == LVIR_BOUNDS) doLabel = FALSE;
  if (uView == LVS_ICON && lprc->left != LVIR_ICON &&
    infoPtr->bFocus && LISTVIEW_GetItemState(infoPtr, nItem, LVIS_FOCUSED))
    oversizedBox = TRUE;

  /* get what we need from the item before hand, so we make
  * only one request. This can speed up things, if data
  * is stored on the app side */
  lvItem.mask = 0;
  if (uView == LVS_REPORT) lvItem.mask |= GXLVIF_INDENT;
  if (doLabel) lvItem.mask |= GXLVIF_TEXT;
  lvItem.iItem = nItem;
  lvItem.iSubItem = 0;
  lvItem.pszText = szDispText;
  lvItem.cchTextMax = DISP_TEXT_SIZE;
  if (lvItem.mask && !LISTVIEW_GetItemW(infoPtr, &lvItem)) return FALSE;
  /* we got the state already up, simulate it here, to avoid a reget */
  if (uView == LVS_ICON && (lprc->left != LVIR_ICON))
  {
    lvItem.mask |= GXLVIF_STATE;
    lvItem.stateMask = LVIS_FOCUSED;
    lvItem.state = (oversizedBox ? LVIS_FOCUSED : 0);
  }

  if (uView == LVS_REPORT && (infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT) && lprc->left == LVIR_SELECTBOUNDS)
    lprc->left = LVIR_BOUNDS;
  switch(lprc->left)
  {
  case LVIR_ICON:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, NULL, NULL, lprc, NULL, NULL);
    break;

  case LVIR_LABEL:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, NULL, NULL, NULL, NULL, lprc);
    break;

  case LVIR_BOUNDS:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, lprc, NULL, NULL, NULL, NULL);
    break;

  case LVIR_SELECTBOUNDS:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, NULL, lprc, NULL, NULL, NULL);
    break;

  default:
    WARN("Unknown value: %d\n", lprc->left);
    return FALSE;
  }

  gxOffsetRect(lprc, Position.x + Origin.x, Position.y + Origin.y);

  TRACE(" rect=%s\n", wine_dbgstr_rect(lprc));

  return TRUE;
}

/***
* DESCRIPTION:
* Retrieves the spacing between listview control items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [IO] lprc : rectangle to receive the output
*             on input, lprc->top = nSubItem
*                       lprc->left = LVIR_ICON | LVIR_BOUNDS | LVIR_LABEL
* 
* NOTE: for subItem = 0, we should return the bounds of the _entire_ item,
*       not only those of the first column.
*       Fortunately, LISTVIEW_GetItemMetrics does the right thing.
* 
* RETURN:
*     TRUE: success
*     FALSE: failure
*/
static GXBOOL LISTVIEW_GetSubItemRect(const LISTVIEW_INFO *infoPtr, GXINT nItem, LPGXRECT lprc)
{
  GXPOINT Position;
  GXLVITEMW lvItem;
  GXINT nColumn;

  if (!lprc) return FALSE;

  nColumn = lprc->top;

  TRACE("(nItem=%d, nSubItem=%d)\n", nItem, lprc->top);
  /* On WinNT, a subitem of '0' calls LISTVIEW_GetItemRect */
  if (lprc->top == 0)
    return LISTVIEW_GetItemRect(infoPtr, nItem, lprc);

  if ((infoPtr->dwStyle & LVS_TYPEMASK) != LVS_REPORT) return FALSE;

  if (!LISTVIEW_GetItemPosition(infoPtr, nItem, &Position)) return FALSE;

  if (nColumn < 0 || nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;

  lvItem.mask = 0;
  lvItem.iItem = nItem;
  lvItem.iSubItem = nColumn;

  if (lvItem.mask && !LISTVIEW_GetItemW(infoPtr, &lvItem)) return FALSE;
  switch(lprc->left)
  {
  case LVIR_ICON:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, NULL, NULL, lprc, NULL, NULL);
    break;

  case LVIR_LABEL:
  case LVIR_BOUNDS:
    LISTVIEW_GetItemMetrics(infoPtr, &lvItem, lprc, NULL, NULL, NULL, NULL);
    break;

  default:
    ERR("Unknown bounds=%d\n", lprc->left);
    return FALSE;
  }

  gxOffsetRect(lprc, Position.x, Position.y);
  return TRUE;
}


/***
* DESCRIPTION:
* Retrieves the width of a label.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*   SUCCESS : string width (in pixels)
*   FAILURE : zero
*/
static GXINT LISTVIEW_GetLabelWidth(const LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  GXLVITEMW lvItem;

  TRACE("(nItem=%d)\n", nItem);

  lvItem.mask = GXLVIF_TEXT;
  lvItem.iItem = nItem;
  lvItem.iSubItem = 0;
  lvItem.pszText = szDispText;
  lvItem.cchTextMax = DISP_TEXT_SIZE;
  if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) return 0;

  return LISTVIEW_GetStringWidthT(infoPtr, lvItem.pszText, TRUE);
}

/***
* DESCRIPTION:
* Retrieves the spacing between listview control items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] bSmall : flag for small or large icon
*
* RETURN:
* Horizontal + vertical spacing
*/
static GXLONG LISTVIEW_GetItemSpacing(const LISTVIEW_INFO *infoPtr, GXBOOL bSmall)
{
  GXLONG lResult;

  if (!bSmall)
  {
    lResult = GXMAKELONG(infoPtr->iconSpacing.cx, infoPtr->iconSpacing.cy);
  }
  else
  {
    if ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_ICON)
      lResult = GXMAKELONG(DEFAULT_COLUMN_WIDTH, gxGetSystemMetrics(GXSM_CXSMICON)+HEIGHT_PADDING);
    else
      lResult = GXMAKELONG(infoPtr->nItemWidth, infoPtr->nItemHeight);
  }
  return lResult;
}

/***
* DESCRIPTION:
* Retrieves the state of a listview control item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] uMask : state mask
*
* RETURN:
* State specified by the mask.
*/
static GXUINT LISTVIEW_GetItemState(const LISTVIEW_INFO *infoPtr, GXINT nItem, GXUINT uMask)
{
  GXLVITEMW lvItem;

  if (nItem < 0 || nItem >= infoPtr->nItemCount) return 0;

  lvItem.iItem = nItem;
  lvItem.iSubItem = 0;
  lvItem.mask = GXLVIF_STATE;
  lvItem.stateMask = uMask;
  if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) return 0;

  return lvItem.state & uMask;
}

/***
* DESCRIPTION:
* Retrieves the text of a listview control item or subitem.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] nItem : item index
* [IO] lpLVItem : item information
* [I] isW :  TRUE if lpLVItem is Unicode
*
* RETURN:
*   SUCCESS : string length
*   FAILURE : 0
*/
static GXINT LISTVIEW_GetItemTextT(const LISTVIEW_INFO *infoPtr, GXINT nItem, GXLPLVITEMW lpLVItem, GXBOOL isW)
{
  if (!lpLVItem || nItem < 0 || nItem >= infoPtr->nItemCount) return 0;

  lpLVItem->mask = GXLVIF_TEXT;
  lpLVItem->iItem = nItem;
  if (!LISTVIEW_GetItemExtT(infoPtr, lpLVItem, isW)) return 0;

  return textlenT(lpLVItem->pszText, isW);
}

/***
* DESCRIPTION:
* Searches for an item based on properties + relationships.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] uFlags : relationship flag
*
* RETURN:
*   SUCCESS : item index
*   FAILURE : -1
*/
static GXINT LISTVIEW_GetNextItem(const LISTVIEW_INFO *infoPtr, GXINT nItem, GXUINT uFlags)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXUINT uMask = 0;
  GXLVFINDINFOW lvFindInfo;
  GXINT nCountPerColumn;
  GXINT nCountPerRow;
  GXINT i;

  TRACE("nItem=%d, uFlags=%x, nItemCount=%d\n", nItem, uFlags, infoPtr->nItemCount);
  if (nItem < -1 || nItem >= infoPtr->nItemCount) return -1;

  ZeroMemory(&lvFindInfo, sizeof(lvFindInfo));

  if (uFlags & GXLVNI_CUT)
    uMask |= LVIS_CUT;

  if (uFlags & GXLVNI_DROPHILITED)
    uMask |= LVIS_DROPHILITED;

  if (uFlags & GXLVNI_FOCUSED)
    uMask |= LVIS_FOCUSED;

  if (uFlags & GXLVNI_SELECTED)
    uMask |= LVIS_SELECTED;

  /* if we're asked for the focused item, that's only one, 
  * so it's worth optimizing */
  if (uFlags & GXLVNI_FOCUSED)
  {
    if ((LISTVIEW_GetItemState(infoPtr, infoPtr->nFocusedItem, uMask) & uMask) != uMask) return -1;
    return (infoPtr->nFocusedItem == nItem) ? -1 : infoPtr->nFocusedItem;
  }

  if (uFlags & GXLVNI_ABOVE)
  {
    if ((uView == LVS_LIST) || (uView == LVS_REPORT))
    {
      while (nItem >= 0)
      {
        nItem--;
        if ((gxListView_GetItemState(infoPtr->hwndSelf, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
    else
    {
      /* Special case for autoarrange - move 'til the top of a list */
      if (is_autoarrange(infoPtr))
      {
        nCountPerRow = LISTVIEW_GetCountPerRow(infoPtr);
        while (nItem - nCountPerRow >= 0)
        {
          nItem -= nCountPerRow;
          if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
            return nItem;
        }
        return -1;
      }
      lvFindInfo.flags = LVFI_NEARESTXY;
      lvFindInfo.vkDirection = GXVK_UP;
      gxSendMessageW( infoPtr->hwndSelf, GXLVM_GETITEMPOSITION, nItem, (GXLPARAM)&lvFindInfo.pt );
      while ((nItem = gxListView_FindItem(infoPtr->hwndSelf, nItem, &lvFindInfo)) != -1)
      {
        if ((gxListView_GetItemState(infoPtr->hwndSelf, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
  }
  else if (uFlags & GXLVNI_BELOW)
  {
    if ((uView == LVS_LIST) || (uView == LVS_REPORT))
    {
      while (nItem < infoPtr->nItemCount)
      {
        nItem++;
        if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
    else
    {
      /* Special case for autoarrange - move 'til the bottom of a list */
      if (is_autoarrange(infoPtr))
      {
        nCountPerRow = LISTVIEW_GetCountPerRow(infoPtr);
        while (nItem + nCountPerRow < infoPtr->nItemCount )
        {
          nItem += nCountPerRow;
          if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
            return nItem;
        }
        return -1;
      }
      lvFindInfo.flags = LVFI_NEARESTXY;
      lvFindInfo.vkDirection = GXVK_DOWN;
      gxSendMessageW( infoPtr->hwndSelf, GXLVM_GETITEMPOSITION, nItem, (GXLPARAM)&lvFindInfo.pt );
      while ((nItem = gxListView_FindItem(infoPtr->hwndSelf, nItem, &lvFindInfo)) != -1)
      {
        if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
  }
  else if (uFlags & GXLVNI_TOLEFT)
  {
    if (uView == LVS_LIST)
    {
      nCountPerColumn = LISTVIEW_GetCountPerColumn(infoPtr);
      while (nItem - nCountPerColumn >= 0)
      {
        nItem -= nCountPerColumn;
        if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
    else if ((uView == LVS_SMALLICON) || (uView == LVS_ICON))
    {
      /* Special case for autoarrange - move 'ti the beginning of a row */
      if (is_autoarrange(infoPtr))
      {
        nCountPerRow = LISTVIEW_GetCountPerRow(infoPtr);
        while (nItem % nCountPerRow > 0)
        {
          nItem --;
          if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
            return nItem;
        }
        return -1;
      }
      lvFindInfo.flags = LVFI_NEARESTXY;
      lvFindInfo.vkDirection = GXVK_LEFT;
      gxSendMessageW( infoPtr->hwndSelf, GXLVM_GETITEMPOSITION, nItem, (GXLPARAM)&lvFindInfo.pt );
      while ((nItem = gxListView_FindItem(infoPtr->hwndSelf, nItem, &lvFindInfo)) != -1)
      {
        if ((gxListView_GetItemState(infoPtr->hwndSelf, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
  }
  else if (uFlags & GXLVNI_TORIGHT)
  {
    if (uView == LVS_LIST)
    {
      nCountPerColumn = LISTVIEW_GetCountPerColumn(infoPtr);
      while (nItem + nCountPerColumn < infoPtr->nItemCount)
      {
        nItem += nCountPerColumn;
        if ((gxListView_GetItemState(infoPtr->hwndSelf, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
    else if ((uView == LVS_SMALLICON) || (uView == LVS_ICON))
    {
      /* Special case for autoarrange - move 'til the end of a row */
      if (is_autoarrange(infoPtr))
      {
        nCountPerRow = LISTVIEW_GetCountPerRow(infoPtr);
        while (nItem % nCountPerRow < nCountPerRow - 1 )
        {
          nItem ++;
          if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
            return nItem;
        }
        return -1;
      }
      lvFindInfo.flags = LVFI_NEARESTXY;
      lvFindInfo.vkDirection = GXVK_RIGHT;
      gxSendMessageW( infoPtr->hwndSelf, GXLVM_GETITEMPOSITION, nItem, (GXLPARAM)&lvFindInfo.pt );
      while ((nItem = gxListView_FindItem(infoPtr->hwndSelf, nItem, &lvFindInfo)) != -1)
      {
        if ((LISTVIEW_GetItemState(infoPtr, nItem, uMask) & uMask) == uMask)
          return nItem;
      }
    }
  }
  else
  {
    nItem++;

    /* search by index */
    for (i = nItem; i < infoPtr->nItemCount; i++)
    {
      if ((LISTVIEW_GetItemState(infoPtr, i, uMask) & uMask) == uMask)
        return i;
    }
  }

  return -1;
}

/* LISTVIEW_GetNumberOfWorkAreas */

/***
* DESCRIPTION:
* Retrieves the origin coordinates when in icon or small icon display mode.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [O] lpptOrigin : coordinate information
*
* RETURN:
*   None.
*/
static void LISTVIEW_GetOrigin(const LISTVIEW_INFO *infoPtr, LPGXPOINT lpptOrigin)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nHorzPos = 0, nVertPos = 0;
  GXSCROLLINFO scrollInfo;

  scrollInfo.cbSize = sizeof(GXSCROLLINFO);    
  scrollInfo.fMask = GXSIF_POS;

  if (gxGetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &scrollInfo))
    nHorzPos = scrollInfo.nPos;
  if (gxGetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &scrollInfo))
    nVertPos = scrollInfo.nPos;

  TRACE("nHorzPos=%d, nVertPos=%d\n", nHorzPos, nVertPos);

  lpptOrigin->x = infoPtr->rcList.left;
  lpptOrigin->y = infoPtr->rcList.top;
  if (uView == LVS_LIST)
    nHorzPos *= infoPtr->nItemWidth;
  else if (uView == LVS_REPORT)
    nVertPos *= infoPtr->nItemHeight;

  lpptOrigin->x -= nHorzPos;
  lpptOrigin->y -= nVertPos;

  TRACE(" origin=%s\n", wine_dbgstr_point(lpptOrigin));
}

/***
* DESCRIPTION:
* Retrieves the width of a string.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] lpszText : text string to process
* [I] isW : TRUE if lpszText is Unicode, FALSE otherwise
*
* RETURN:
*   SUCCESS : string width (in pixels)
*   FAILURE : zero
*/
static GXINT LISTVIEW_GetStringWidthT(const LISTVIEW_INFO *infoPtr, GXLPCWSTR lpszText, GXBOOL isW)
{
  GXSIZE stringSize;

  stringSize.cx = 0;    
  if (is_textT(lpszText, isW))
  {
    GXHFONT hFont = infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont;
    GXHDC hdc = gxGetDC(infoPtr->hwndSelf);
    GXHFONT hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);

    if (isW)
      gxGetTextExtentPointW(hdc, lpszText, GXSTRLEN(lpszText), &stringSize);
    //else
    //GetTextExtentPointA(hdc, (GXLPCSTR)lpszText, lstrlenA((GXLPCSTR)lpszText), &stringSize);
    gxSelectObject(hdc, hOldFont);
    gxReleaseDC(infoPtr->hwndSelf, hdc);
  }
  return stringSize.cx;
}

/***
* DESCRIPTION:
* Determines which listview item is located at the specified position.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [IO] lpht : hit test information
* [I] subitem : fill out iSubItem.
* [I] select : return the index only if the hit selects the item
*
* NOTE:
* (mm 20001022): We must not allow iSubItem to be touched, for
* an app might pass only a structure with space up to iItem!
* (MS Office 97 does that for instance in the file open dialog)
* 
* RETURN:
*   SUCCESS : item index
*   FAILURE : -1
*/
static GXINT LISTVIEW_HitTest(const LISTVIEW_INFO *infoPtr, GXLPLVHITTESTINFO lpht, GXBOOL subitem, GXBOOL select)
{
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { '\0' };
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXRECT rcBox, rcBounds, rcState, rcIcon, rcLabel, rcSearch;
  GXPOINT Origin, Position, opt;
  GXLVITEMW lvItem;
  ITERATOR i;
  GXINT iItem;

  TRACE("(pt=%s, subitem=%d, select=%d)\n", wine_dbgstr_point(&lpht->pt), subitem, select);

  lpht->flags = 0;
  lpht->iItem = -1;
  if (subitem) lpht->iSubItem = 0;

  if (infoPtr->rcList.left > lpht->pt.x)
    lpht->flags |= GXLVHT_TOLEFT;
  else if (infoPtr->rcList.right < lpht->pt.x)
    lpht->flags |= GXLVHT_TORIGHT;

  if (infoPtr->rcList.top > lpht->pt.y)
    lpht->flags |= GXLVHT_ABOVE;
  else if (infoPtr->rcList.bottom < lpht->pt.y)
    lpht->flags |= GXLVHT_BELOW;

  TRACE("lpht->flags=0x%x\n", lpht->flags);
  if (lpht->flags) return -1;

  lpht->flags |= GXLVHT_NOWHERE;

  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* first deal with the large items */
  rcSearch.left = lpht->pt.x;
  rcSearch.top = lpht->pt.y;
  rcSearch.right = rcSearch.left + 1;
  rcSearch.bottom = rcSearch.top + 1;

  iterator_frameditems(&i, infoPtr, &rcSearch);
  iterator_next(&i); /* go to first item in the sequence */
  iItem = i.nItem;
  iterator_destroy(&i);

  TRACE("lpht->iItem=%d\n", iItem); 
  if (iItem == -1) return -1;

  lvItem.mask = GXLVIF_STATE | GXLVIF_TEXT;
  if (uView == LVS_REPORT) lvItem.mask |= GXLVIF_INDENT;
  lvItem.stateMask = LVIS_STATEIMAGEMASK;
  if (uView == LVS_ICON) lvItem.stateMask |= LVIS_FOCUSED;
  lvItem.iItem = iItem;
  lvItem.iSubItem = 0;
  lvItem.pszText = szDispText;
  lvItem.cchTextMax = DISP_TEXT_SIZE;
  if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) return -1;
  if (!infoPtr->bFocus) lvItem.state &= ~LVIS_FOCUSED;

  LISTVIEW_GetItemMetrics(infoPtr, &lvItem, &rcBox, NULL, &rcIcon, &rcState, &rcLabel);
  LISTVIEW_GetItemOrigin(infoPtr, iItem, &Position);
  opt.x = lpht->pt.x - Position.x - Origin.x;
  opt.y = lpht->pt.y - Position.y - Origin.y;

  if (uView == LVS_REPORT)
    rcBounds = rcBox;
  else
  {
    gxUnionRect(&rcBounds, &rcIcon, &rcLabel);
    gxUnionRect(&rcBounds, &rcBounds, &rcState);
  }
  TRACE("rcBounds=%s\n", wine_dbgstr_rect(&rcBounds));
  if (!gxPtInRect(&rcBounds, opt)) return -1;

  if (gxPtInRect(&rcIcon, opt))
    lpht->flags |= GXLVHT_ONITEMICON;
  else if (gxPtInRect(&rcLabel, opt))
    lpht->flags |= GXLVHT_ONITEMLABEL;
  else if (infoPtr->himlState && STATEIMAGEINDEX(lvItem.state) && gxPtInRect(&rcState, opt))
    lpht->flags |= GXLVHT_ONITEMSTATEICON;
  if (lpht->flags & GXLVHT_ONITEM)
    lpht->flags &= ~GXLVHT_NOWHERE;

  TRACE("lpht->flags=0x%x\n", lpht->flags); 
  if (uView == LVS_REPORT && subitem)
  {
    GXINT j;

    rcBounds.right = rcBounds.left;
    for (j = 0; j < gxDPA_GetPtrCount(infoPtr->hdpaColumns); j++)
    {
      rcBounds.left = rcBounds.right;
      rcBounds.right += LISTVIEW_GetColumnWidth(infoPtr, j);
      if (gxPtInRect(&rcBounds, opt))
      {
        lpht->iSubItem = j;
        break;
      }
    }
  }

  if (select && !(uView == LVS_REPORT &&
    ((infoPtr->dwLvExStyle & LVS_EX_FULLROWSELECT) ||
    (infoPtr->dwStyle & LVS_OWNERDRAWFIXED))))
  {
    if (uView == LVS_REPORT)
    {
      gxUnionRect(&rcBounds, &rcIcon, &rcLabel);
      gxUnionRect(&rcBounds, &rcBounds, &rcState);
    }
    if (!gxPtInRect(&rcBounds, opt)) iItem = -1;
  }
  return lpht->iItem = iItem;
}


/* LISTVIEW_InsertCompare:  callback routine for comparing pszText members of the LV_ITEMS
in a LISTVIEW on insert.  Passed to gxDPA_Sort in LISTVIEW_InsertItem.
This function should only be used for inserting items into a sorted list (GXLVM_INSERTITEM)
and not during the processing of a GXLVM_SORTITEMS message. Applications should provide
their own sort proc. when sending GXLVM_SORTITEMS.
*/
/* Platform SDK:
(remarks on LVITEM: GXLVM_INSERTITEM will insert the new item in the proper sort postion...
if:
LVS_SORTXXX must be specified,
LVS_OWNERDRAW is not set,
<item>.pszText is not LPSTR_TEXTCALLBACK.

(LVS_SORT* flags): "For the LVS_SORTASCENDING... styles, item indices
are sorted based on item text..."
*/
static GXINT GXCALLBACK LISTVIEW_InsertCompare(  GXLPVOID first, GXLPVOID second,  GXLPARAM lParam)
{
  ITEM_INFO* lv_first = (ITEM_INFO*) gxDPA_GetPtr( (GXHDPA)first, 0 );
  ITEM_INFO* lv_second = (ITEM_INFO*) gxDPA_GetPtr( (GXHDPA)second, 0 );
  GXINT cmpv = textcmpWT(lv_first->hdr.pszText, lv_second->hdr.pszText, TRUE); 

  /* if we're sorting descending, negate the return value */
  return (((const LISTVIEW_INFO *)lParam)->dwStyle & LVS_SORTDESCENDING) ? -cmpv : cmpv;
}

/***
* DESCRIPTION:
* Inserts a new item in the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] lpLVItem : item information
* [I] isW : TRUE if lpLVItem is Unicode, FALSE if it's ANSI
*
* RETURN:
*   SUCCESS : new item index
*   FAILURE : -1
*/
static GXINT LISTVIEW_InsertItemT(LISTVIEW_INFO *infoPtr, const GXLVITEMW *lpLVItem, GXBOOL isW)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nItem;
  GXHDPA hdpaSubItems;
  GXNMLISTVIEW nmlv;
  ITEM_INFO *lpItem;
  GXBOOL is_sorted, has_changed;
  GXLVITEMW item;
  GXHWND hwndSelf = infoPtr->hwndSelf;

  TRACE("(lpLVItem=%s, isW=%d)\n", debuglvitem_t(lpLVItem, isW), isW);

  if (infoPtr->dwStyle & LVS_OWNERDATA) return infoPtr->nItemCount++;

  /* make sure it's an item, and not a subitem; cannot insert a subitem */
  if (!lpLVItem || lpLVItem->iSubItem) return -1;

  if (!is_assignable_item(lpLVItem, infoPtr->dwStyle)) return -1;

  if (!(lpItem = (ITEM_INFO*)Alloc(sizeof(ITEM_INFO)))) return -1;

  /* insert item in listview control data structure */
  if ( !(hdpaSubItems = gxDPA_Create(8)) ) goto fail;
  if ( !gxDPA_SetPtr(hdpaSubItems, 0, lpItem) ){
    WEAssert (FALSE);
  }

  is_sorted = (infoPtr->dwStyle & (LVS_SORTASCENDING | LVS_SORTDESCENDING)) &&
    !(infoPtr->dwStyle & LVS_OWNERDRAWFIXED) && (GXLPSTR_TEXTCALLBACKW != lpLVItem->pszText);

  if (lpLVItem->iItem < 0 && !is_sorted) return -1;

  nItem = is_sorted ? infoPtr->nItemCount : min(lpLVItem->iItem, infoPtr->nItemCount);
  TRACE(" inserting at %d, sorted=%d, count=%d, iItem=%d\n", nItem, is_sorted, infoPtr->nItemCount, lpLVItem->iItem);
  nItem = gxDPA_InsertPtr( infoPtr->hdpaItems, nItem, hdpaSubItems );
  if (nItem == -1) goto fail;
  infoPtr->nItemCount++;

  /* shift indices first so they don't get tangled */
  LISTVIEW_ShiftIndices(infoPtr, nItem, 1);

  /* set the item attributes */
  if (lpLVItem->mask & (GXLVIF_GROUPID|GXLVIF_COLUMNS))
  {
    /* full size structure expected - _WIN32IE >= 0x560 */
    item = *lpLVItem;
  }
  else if (lpLVItem->mask & GXLVIF_INDENT)
  {
    /* indent member expected - _WIN32IE >= 0x300 */
    memcpy(&item, lpLVItem, offsetof( GXLVITEMW, iGroupId ));
  }
  else
  {
    /* minimal structure expected */
    memcpy(&item, lpLVItem, offsetof( GXLVITEMW, iIndent ));
  }
  item.iItem = nItem;
  if (infoPtr->dwLvExStyle & LVS_EX_CHECKBOXES)
  {
    item.mask |= GXLVIF_STATE;
    item.stateMask |= LVIS_STATEIMAGEMASK;
    item.state &= ~LVIS_STATEIMAGEMASK;
    item.state |= INDEXTOSTATEIMAGEMASK(1);
  }
  if (!set_main_item(infoPtr, &item, TRUE, isW, &has_changed)) goto undo;

  /* if we're sorted, sort the list, and update the index */
  if (is_sorted)
  {
    gxDPA_Sort( infoPtr->hdpaItems, LISTVIEW_InsertCompare, (GXLPARAM)infoPtr );
    nItem = gxDPA_GetPtrIndex( infoPtr->hdpaItems, hdpaSubItems );
    WEAssert(nItem != -1);
  }

  /* make room for the position, if we are in the right mode */
  if ((uView == LVS_SMALLICON) || (uView == LVS_ICON))
  {
    if (gxDPA_InsertPtr(infoPtr->hdpaPosX, nItem, 0) == -1)
      goto undo;
    if (gxDPA_InsertPtr(infoPtr->hdpaPosY, nItem, 0) == -1)
    {
      gxDPA_DeletePtr(infoPtr->hdpaPosX, nItem);
      goto undo;
    }
  }

  /* send LVN_INSERTITEM notification */
  ZeroMemory(&nmlv, sizeof(GXNMLISTVIEW));
  nmlv.iItem = nItem;
  nmlv.lParam = lpItem->lParam;
  notify_listview(infoPtr, LVN_INSERTITEM, &nmlv);
  if (!gxIsWindow(hwndSelf))
    return -1;

  /* align items (set position of each item) */
  if ((uView == LVS_SMALLICON || uView == LVS_ICON))
  {
    GXPOINT pt;

    if (infoPtr->dwStyle & LVS_ALIGNLEFT)
      LISTVIEW_NextIconPosLeft(infoPtr, &pt);
    else
      LISTVIEW_NextIconPosTop(infoPtr, &pt);

    LISTVIEW_MoveIconTo(infoPtr, nItem, &pt, TRUE);
  }

  /* now is the invalidation fun */
  LISTVIEW_ScrollOnInsert(infoPtr, nItem, 1);
  return nItem;

undo:
  LISTVIEW_ShiftIndices(infoPtr, nItem, -1);
  gxDPA_DeletePtr(infoPtr->hdpaItems, nItem);
  infoPtr->nItemCount--;
fail:
  gxDPA_DeletePtr(hdpaSubItems, 0);
  gxDPA_Destroy (hdpaSubItems);
  Free (lpItem);
  return -1;
}

/***
* DESCRIPTION:
* Redraws a range of items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nFirst : first item
* [I] nLast : last item
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_RedrawItems(const LISTVIEW_INFO *infoPtr, GXINT nFirst, GXINT nLast)
{
  GXINT i;

  if (nLast < nFirst || min(nFirst, nLast) < 0 || 
    max(nFirst, nLast) >= infoPtr->nItemCount)
    return FALSE;

  for (i = nFirst; i <= nLast; i++)
    LISTVIEW_InvalidateItem(infoPtr, i);

  return TRUE;
}

/***
* DESCRIPTION:
* Scroll the content of a listview.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] dx : horizontal scroll amount in pixels
* [I] dy : vertical scroll amount in pixels
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*
* COMMENTS:
*  If the control is in report mode (LVS_REPORT) the control can
*  be scrolled only in line increments. "dy" will be rounded to the
*  nearest number of pixels that are a whole line. Ex: if line height
*  is 16 and an 8 is passed, the list will be scrolled by 16. If a 7
*  is passed, then the scroll will be 0.  (per MSDN 7/2002)
*
*  For:  (per experimentation with native control and CSpy ListView)
*     LVS_ICON       dy=1 = 1 pixel  (vertical only)
*                    dx ignored
*     LVS_SMALLICON  dy=1 = 1 pixel  (vertical only)
*                    dx ignored
*     LVS_LIST       dx=1 = 1 column (horizontal only)
*                           but will only scroll 1 column per message
*                           no matter what the value.
*                    dy must be 0 or FALSE returned.
*     LVS_REPORT     dx=1 = 1 pixel
*                    dy=  see above
*
*/
static GXBOOL LISTVIEW_Scroll(LISTVIEW_INFO *infoPtr, GXINT dx, GXINT dy)
{
  switch(infoPtr->dwStyle & LVS_TYPEMASK) {
  case LVS_REPORT:
    dy += (dy < 0 ? -1 : 1) * infoPtr->nItemHeight/2;
    dy /= infoPtr->nItemHeight;
    break;
  case LVS_LIST:
    if (dy != 0) return FALSE;
    break;
  default: /* icon */
    dx = 0;
    break;
  }  

  if (dx != 0) LISTVIEW_HScroll(infoPtr, SB_INTERNAL, dx, 0);
  if (dy != 0) LISTVIEW_VScroll(infoPtr, SB_INTERNAL, dy, 0);

  return TRUE;
}

/***
* DESCRIPTION:
* Sets the background color.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] clrBk : background color
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetBkColor(LISTVIEW_INFO *infoPtr, GXCOLORREF clrBk)
{
  TRACE("(clrBk=%x)\n", clrBk);

  if(infoPtr->clrBk != clrBk) {
    if (infoPtr->clrBk != GXCLR_NONE) gxDeleteObject(infoPtr->hBkBrush);
    infoPtr->clrBk = clrBk;
    if (clrBk == GXCLR_NONE)
      infoPtr->hBkBrush = (GXHBRUSH)(GXDWORD_PTR)gxGetClassLongPtrW(infoPtr->hwndSelf, GXGCLP_HBRBACKGROUND);
    else
      infoPtr->hBkBrush = gxCreateSolidBrush(clrBk);
    LISTVIEW_InvalidateList(infoPtr);
  }

  return TRUE;
}

/* LISTVIEW_SetBkImage */

/*** Helper for {Insert,Set}ColumnT *only* */
static void column_fill_hditem(const LISTVIEW_INFO *infoPtr, GXHDITEMW *lphdi, GXINT nColumn,
                 const GXLVCOLUMNW *lpColumn, GXBOOL isW)
{
  if (lpColumn->mask & LVCF_FMT)
  {
    /* format member is valid */
    lphdi->mask |= HDI_FORMAT;

    /* set text alignment (leftmost column must be left-aligned) */
    if (nColumn == 0 || (lpColumn->fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
      lphdi->fmt |= HDF_LEFT;
    else if ((lpColumn->fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
      lphdi->fmt |= HDF_RIGHT;
    else if ((lpColumn->fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_CENTER)
      lphdi->fmt |= HDF_CENTER;

    if (lpColumn->fmt & LVCFMT_BITMAP_ON_RIGHT)
      lphdi->fmt |= HDF_BITMAP_ON_RIGHT;

    if (lpColumn->fmt & LVCFMT_COL_HAS_IMAGES)
    {
      lphdi->fmt |= HDF_IMAGE;
      lphdi->iImage = GXI_IMAGECALLBACK;
    }
  }

  if (lpColumn->mask & LVCF_WIDTH)
  {
    lphdi->mask |= HDI_WIDTH;
    if(lpColumn->cx == LVSCW_AUTOSIZE_USEHEADER)
    {
      /* make it fill the remainder of the controls width */
      GXRECT rcHeader;
      GXINT item_index;

      for(item_index = 0; item_index < (nColumn - 1); item_index++)
      {
        LISTVIEW_GetHeaderRect(infoPtr, item_index, &rcHeader);
        lphdi->cxy += rcHeader.right - rcHeader.left;
      }

      /* retrieve the layout of the header */
      gxGetClientRect(infoPtr->hwndSelf, &rcHeader);
      TRACE("start cxy=%d rcHeader=%s\n", lphdi->cxy, wine_dbgstr_rect(&rcHeader));

      lphdi->cxy = (rcHeader.right - rcHeader.left) - lphdi->cxy;
    }
    else
      lphdi->cxy = lpColumn->cx;
  }

  if (lpColumn->mask & LVCF_TEXT)
  {
    lphdi->mask |= HDI_TEXT | HDI_FORMAT;
    lphdi->fmt |= HDF_STRING;
    lphdi->pszText = lpColumn->pszText;
    lphdi->cchTextMax = textlenT(lpColumn->pszText, isW);
  }

  if (lpColumn->mask & LVCF_IMAGE)
  {
    lphdi->mask |= HDI_IMAGE;
    lphdi->iImage = lpColumn->iImage;
  }

  if (lpColumn->mask & LVCF_ORDER)
  {
    lphdi->mask |= HDI_ORDER;
    lphdi->iOrder = lpColumn->iOrder;
  }
}


/***
* DESCRIPTION:
* Inserts a new column.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn : column index
* [I] lpColumn : column information
* [I] isW : TRUE if lpColumn is Unicode, FALSE otherwise
*
* RETURN:
*   SUCCESS : new column index
*   FAILURE : -1
*/
static GXINT LISTVIEW_InsertColumnT(LISTVIEW_INFO *infoPtr, GXINT nColumn,
                  const GXLVCOLUMNW *lpColumn, GXBOOL isW)
{
  COLUMN_INFO *lpColumnInfo;
  GXINT nNewColumn;
  GXHDITEMW hdi;

  TRACE("(nColumn=%d, lpColumn=%s, isW=%d)\n", nColumn, debuglvcolumn_t(lpColumn, isW), isW);

  if (!lpColumn || nColumn < 0) return -1;
  nColumn = min(nColumn, gxDPA_GetPtrCount(infoPtr->hdpaColumns));

  ZeroMemory(&hdi, sizeof(GXHDITEMW));
  column_fill_hditem(infoPtr, &hdi, nColumn, lpColumn, isW);

  /*
  * A mask not including LVCF_WIDTH turns into a mask of width, width 10
  * (can be seen in SPY) otherwise column never gets added.
  */
  if (!(lpColumn->mask & LVCF_WIDTH)) {
    hdi.mask |= HDI_WIDTH;
    hdi.cxy = 10;
  }

  /*
  * when the iSubItem is available Windows copies it to the header lParam. It seems
  * to happen only in GXLVM_INSERTCOLUMN - not in GXLVM_SETCOLUMN
  */
  if (lpColumn->mask & LVCF_SUBITEM)
  {
    hdi.mask |= HDI_LPARAM;
    hdi.lParam = lpColumn->iSubItem;
  }

  /* insert item in header control */
  nNewColumn = gxSendMessageW(infoPtr->hwndHeader, 
    isW ? GXHDM_INSERTITEMW : GXHDM_INSERTITEMA,
    (GXWPARAM)nColumn, (GXLPARAM)&hdi);
  if (nNewColumn == -1) return -1;
  if (nNewColumn != nColumn) {
    ERR("nColumn=%d, nNewColumn=%d\n", nColumn, nNewColumn);
  }

  /* create our own column info */ 
  if (!(lpColumnInfo = (COLUMN_INFO*)Alloc(sizeof(COLUMN_INFO)))) {
    goto fail;
  }

  if (gxDPA_InsertPtr(infoPtr->hdpaColumns, nNewColumn, lpColumnInfo) == -1) {
    goto fail;
  }

  if (lpColumn->mask & LVCF_FMT) lpColumnInfo->fmt = lpColumn->fmt;
  if (!gxHeader_GetItemRect(infoPtr->hwndHeader, nNewColumn, &lpColumnInfo->rcHeader)) goto fail;

  /* now we have to actually adjust the data */
  if (!(infoPtr->dwStyle & LVS_OWNERDATA) && infoPtr->nItemCount > 0)
  {
    SUBITEM_INFO *lpSubItem;
    GXHDPA hdpaSubItems;
    GXINT nItem, i;

    for (nItem = 0; nItem < infoPtr->nItemCount; nItem++)
    {
      hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, nItem);
      for (i = 1; i < gxDPA_GetPtrCount(hdpaSubItems); i++)
      {
        lpSubItem = (SUBITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, i);
        if (lpSubItem->iSubItem >= nNewColumn)
          lpSubItem->iSubItem++;
      }
    }
  }

  /* make space for the new column */
  LISTVIEW_ScrollColumns(infoPtr, nNewColumn + 1, lpColumnInfo->rcHeader.right - lpColumnInfo->rcHeader.left);
  LISTVIEW_UpdateItemSize(infoPtr);

  return nNewColumn;

fail:
  if (nNewColumn != -1) gxSendMessageW(infoPtr->hwndHeader, GXHDM_DELETEITEM, nNewColumn, 0);
  if (lpColumnInfo)
  {
    gxDPA_DeletePtr(infoPtr->hdpaColumns, nNewColumn);
    Free(lpColumnInfo);
  }
  return -1;
}

/***
* DESCRIPTION:
* Sets the attributes of a header item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn : column index
* [I] lpColumn : column attributes
* [I] isW: if TRUE, then lpColumn is a GXLPLVCOLUMNW, else it is a LPLVCOLUMNA
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetColumnT(const LISTVIEW_INFO *infoPtr, GXINT nColumn,
                  const GXLVCOLUMNW *lpColumn, GXBOOL isW)
{
  GXHDITEMW hdi, hdiget;
  GXBOOL bResult;

  TRACE("(nColumn=%d, lpColumn=%s, isW=%d)\n", nColumn, debuglvcolumn_t(lpColumn, isW), isW);

  if (!lpColumn || nColumn < 0 || nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;

  ZeroMemory(&hdi, sizeof(GXHDITEMW));
  if (lpColumn->mask & LVCF_FMT)
  {
    hdi.mask |= HDI_FORMAT;
    hdiget.mask = HDI_FORMAT;
    if (gxHeader_GetItemW(infoPtr->hwndHeader, nColumn, &hdiget))
      hdi.fmt = hdiget.fmt & HDF_STRING;
  }
  column_fill_hditem(infoPtr, &hdi, nColumn, lpColumn, isW);

  /* set header item attributes */
  bResult = gxSendMessageW(infoPtr->hwndHeader, isW ? GXHDM_SETITEMW : GXHDM_SETITEMA, (GXWPARAM)nColumn, (GXLPARAM)&hdi);
  if (!bResult) return FALSE;

  if (lpColumn->mask & LVCF_FMT)
  {
    COLUMN_INFO *lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, nColumn);
    int oldFmt = lpColumnInfo->fmt;

    lpColumnInfo->fmt = lpColumn->fmt;
    if ((oldFmt ^ lpColumn->fmt) & (LVCFMT_JUSTIFYMASK | LVCFMT_IMAGE))
    {
      GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
      if (uView == LVS_REPORT) LISTVIEW_InvalidateColumn(infoPtr, nColumn);
    }
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Sets the column order array
*
* PARAMETERS:
* [I] infoPtr : valid pointer to the listview structure
* [I] iCount : number of elements in column order array
* [I] lpiArray : pointer to column order array
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetColumnOrderArray(const LISTVIEW_INFO *infoPtr, GXINT iCount, const GXINT *lpiArray)
{
  FIXME("iCount %d lpiArray %p\n", iCount, lpiArray);

  if (!lpiArray)
    return FALSE;

  return TRUE;
}


/***
* DESCRIPTION:
* Sets the width of a column
*
* PARAMETERS:
* [I] infoPtr : valid pointer to the listview structure
* [I] nColumn : column index
* [I] cx : column width
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetColumnWidth(LISTVIEW_INFO *infoPtr, GXINT nColumn, GXINT cx)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXWCHAR szDispText[DISP_TEXT_SIZE] = { 0 };
  GXINT max_cx = 0;
  GXHDITEMW hdi;

  TRACE("(nColumn=%d, cx=%d\n", nColumn, cx);

  /* set column width only if in report or list mode */
  if (uView != LVS_REPORT && uView != LVS_LIST) return FALSE;

  /* take care of invalid cx values */
  if(uView == LVS_REPORT && cx < -2) cx = LVSCW_AUTOSIZE;
  else if (uView == LVS_LIST && cx < 1) return FALSE;

  /* resize all columns if in LVS_LIST mode */
  if(uView == LVS_LIST) 
  {
    infoPtr->nItemWidth = cx;
    LISTVIEW_InvalidateList(infoPtr);
    return TRUE;
  }

  if (nColumn < 0 || nColumn >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return FALSE;

  if (cx == LVSCW_AUTOSIZE || (cx == LVSCW_AUTOSIZE_USEHEADER && nColumn < gxDPA_GetPtrCount(infoPtr->hdpaColumns) -1))
  {
    GXINT nLabelWidth;
    GXLVITEMW lvItem;

    lvItem.mask = GXLVIF_TEXT;  
    lvItem.iItem = 0;
    lvItem.iSubItem = nColumn;
    lvItem.pszText = szDispText;
    lvItem.cchTextMax = DISP_TEXT_SIZE;
    for (; lvItem.iItem < infoPtr->nItemCount; lvItem.iItem++)
    {
      if (!LISTVIEW_GetItemW(infoPtr, &lvItem)) continue;
      nLabelWidth = LISTVIEW_GetStringWidthT(infoPtr, lvItem.pszText, TRUE);
      if (max_cx < nLabelWidth) max_cx = nLabelWidth;
    }
    if (infoPtr->himlSmall && (nColumn == 0 || (LISTVIEW_GetColumnInfo(infoPtr, nColumn)->fmt & LVCFMT_IMAGE)))
      max_cx += infoPtr->iconSize.cx;
    max_cx += TRAILING_LABEL_PADDING;
  }

  /* autosize based on listview items width */
  if(cx == LVSCW_AUTOSIZE)
    cx = max_cx;
  else if(cx == LVSCW_AUTOSIZE_USEHEADER)
  {
    /* if iCol is the last column make it fill the remainder of the controls width */
    if(nColumn == gxDPA_GetPtrCount(infoPtr->hdpaColumns) - 1) 
    {
      GXRECT rcHeader;
      GXPOINT Origin;

      LISTVIEW_GetOrigin(infoPtr, &Origin);
      LISTVIEW_GetHeaderRect(infoPtr, nColumn, &rcHeader);

      cx = infoPtr->rcList.right - Origin.x - rcHeader.left;
    }
    else
    {
      /* Despite what the MS docs say, if this is not the last
      column, then MS resizes the column to the width of the
      largest text string in the column, including headers
      and items. This is different from LVSCW_AUTOSIZE in that
      LVSCW_AUTOSIZE ignores the header string length. */
      cx = 0;

      /* retrieve header text */
      hdi.mask = HDI_TEXT;
      hdi.cchTextMax = DISP_TEXT_SIZE;
      hdi.pszText = szDispText;
      if (gxHeader_GetItemW(infoPtr->hwndHeader, nColumn, &hdi))
      {
        GXHDC hdc = gxGetDC(infoPtr->hwndSelf);
        GXHFONT old_font = (GXHFONT)gxSelectObject(hdc, (GXHFONT)gxSendMessageW(infoPtr->hwndHeader, GXWM_GETFONT, 0, 0));
        GXSIZE size;

        if (gxGetTextExtentPoint32W(hdc, hdi.pszText, GXSTRLEN(hdi.pszText), &size))
          cx = size.cx + TRAILING_HEADER_PADDING;
        /* FIXME: Take into account the header image, if one is present */
        gxSelectObject(hdc, old_font);
        gxReleaseDC(infoPtr->hwndSelf, hdc);
      }
      cx = max (cx, max_cx);
    }
  }

  if (cx < 0) return FALSE;

  /* call header to update the column change */
  hdi.mask = HDI_WIDTH;
  hdi.cxy = cx;
  TRACE("hdi.cxy=%d\n", hdi.cxy);
  return gxHeader_SetItemW(infoPtr->hwndHeader, nColumn, &hdi);
}

/***
* Creates the checkbox imagelist.  Helper for LISTVIEW_SetExtendedListViewStyle
*
*/
static GXHIMAGELIST LISTVIEW_CreateCheckBoxIL(const LISTVIEW_INFO *infoPtr)
{
  GXHDC hdc_wnd, hdc;
  GXHBITMAP hbm_im, hbm_mask, hbm_orig;
  GXRECT rc;
  GXHBRUSH hbr_white = (GXHBRUSH)gxGetStockObject(GXWHITE_BRUSH);
  GXHBRUSH hbr_black = (GXHBRUSH)gxGetStockObject(GXBLACK_BRUSH);
  GXHIMAGELIST himl;

  himl = gxImageList_Create(gxGetSystemMetrics(GXSM_CXSMICON), gxGetSystemMetrics(GXSM_CYSMICON),
    GXILC_COLOR | GXILC_MASK, 2, 2);
  hdc_wnd = gxGetDC(infoPtr->hwndSelf);
  hdc = gxCreateCompatibleDC(hdc_wnd);
  hbm_im = gxCreateCompatibleBitmap(hdc_wnd, gxGetSystemMetrics(GXSM_CXSMICON), gxGetSystemMetrics(GXSM_CYSMICON));
  hbm_mask = gxCreateBitmap(gxGetSystemMetrics(GXSM_CXSMICON), gxGetSystemMetrics(GXSM_CYSMICON), 1, 1, NULL);
  gxReleaseDC(infoPtr->hwndSelf, hdc_wnd);

  rc.left = rc.top = 0;
  rc.right = gxGetSystemMetrics(GXSM_CXSMICON);
  rc.bottom = gxGetSystemMetrics(GXSM_CYSMICON);

  hbm_orig = (GXHBITMAP)gxSelectObject(hdc, hbm_mask);
  gxFillRect(hdc, &rc, hbr_white);
  gxInflateRect(&rc, -3, -3);
  gxFillRect(hdc, &rc, hbr_black);

  gxSelectObject(hdc, hbm_im);
  gxDrawFrameControl(hdc, &rc, GXDFC_BUTTON, GXDFCS_BUTTONCHECK | GXDFCS_MONO);
  gxSelectObject(hdc, hbm_orig);
  gxImageList_Add(himl, hbm_im, hbm_mask); 

  gxSelectObject(hdc, hbm_im);
  gxDrawFrameControl(hdc, &rc, GXDFC_BUTTON, GXDFCS_BUTTONCHECK | GXDFCS_MONO | GXDFCS_CHECKED);
  gxSelectObject(hdc, hbm_orig);
  gxImageList_Add(himl, hbm_im, hbm_mask);

  gxDeleteObject(hbm_mask);
  gxDeleteObject(hbm_im);
  gxDeleteDC(hdc);

  return himl;
}

/***
* DESCRIPTION:
* Sets the extended listview style.
*
* PARAMETERS:
* [I] infoPtr : valid pointer to the listview structure
* [I] dwMask : mask
* [I] dwStyle : style
*
* RETURN:
*   SUCCESS : previous style
*   FAILURE : 0
*/
static GXDWORD LISTVIEW_SetExtendedListViewStyle(LISTVIEW_INFO *infoPtr, GXDWORD dwMask, GXDWORD dwExStyle)
{
  GXDWORD dwOldExStyle = infoPtr->dwLvExStyle;

  /* set new style */
  if (dwMask)
    infoPtr->dwLvExStyle = (dwOldExStyle & ~dwMask) | (dwExStyle & dwMask);
  else
    infoPtr->dwLvExStyle = dwExStyle;

  if((infoPtr->dwLvExStyle ^ dwOldExStyle) & LVS_EX_CHECKBOXES)
  {
    GXHIMAGELIST himl = 0;
    if(infoPtr->dwLvExStyle & LVS_EX_CHECKBOXES)
    {
      GXLVITEMW item;
      item.mask = GXLVIF_STATE;
      item.stateMask = LVIS_STATEIMAGEMASK;
      item.state = INDEXTOSTATEIMAGEMASK(1);
      LISTVIEW_SetItemState(infoPtr, -1, &item);

      himl = LISTVIEW_CreateCheckBoxIL(infoPtr);
    }
    LISTVIEW_SetImageList(infoPtr, LVSIL_STATE, himl);
  }

  if((infoPtr->dwLvExStyle ^ dwOldExStyle) & LVS_EX_HEADERDRAGDROP)
  {
    GXDWORD dwStyle = gxGetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE);
    if (infoPtr->dwLvExStyle & LVS_EX_HEADERDRAGDROP)
      dwStyle |= HDS_DRAGDROP;
    else
      dwStyle &= ~HDS_DRAGDROP;
    gxSetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE, dwStyle);
  }

  /* GRIDLINES adds decoration at top so changes sizes */
  if((infoPtr->dwLvExStyle ^ dwOldExStyle) & LVS_EX_GRIDLINES)
  {
    LISTVIEW_UpdateSize(infoPtr);
  }


  LISTVIEW_InvalidateList(infoPtr);
  return dwOldExStyle;
}

/***
* DESCRIPTION:
* Sets the new hot cursor used during hot tracking and hover selection.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hCursor : the new hot cursor handle
*
* RETURN:
* Returns the previous hot cursor
*/
static GXHCURSOR LISTVIEW_SetHotCursor(LISTVIEW_INFO *infoPtr, GXHCURSOR hCursor)
{
  GXHCURSOR oldCursor = infoPtr->hHotCursor;

  infoPtr->hHotCursor = hCursor;

  return oldCursor;
}


/***
* DESCRIPTION:
* Sets the hot item index.
*
* PARAMETERS:
* [I] infoPtr : valid pointer to the listview structure
* [I] iIndex : index
*
* RETURN:
*   SUCCESS : previous hot item index
*   FAILURE : -1 (no hot item)
*/
static GXINT LISTVIEW_SetHotItem(LISTVIEW_INFO *infoPtr, GXINT iIndex)
{
  GXINT iOldIndex = infoPtr->nHotItem;

  infoPtr->nHotItem = iIndex;

  return iOldIndex;
}


/***
* DESCRIPTION:
* Sets the amount of time the cursor must hover over an item before it is selected.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] dwHoverTime : hover time, if -1 the hover time is set to the default
*
* RETURN:
* Returns the previous hover time
*/
static GXDWORD LISTVIEW_SetHoverTime(LISTVIEW_INFO *infoPtr, GXDWORD dwHoverTime)
{
  GXDWORD oldHoverTime = infoPtr->dwHoverTime;

  infoPtr->dwHoverTime = dwHoverTime;

  return oldHoverTime;
}

/***
* DESCRIPTION:
* Sets spacing for icons of LVS_ICON style.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] cx : horizontal spacing (-1 = system spacing, 0 = autosize)
* [I] cy : vertical spacing (-1 = system spacing, 0 = autosize)
*
* RETURN:
*   MAKELONG(oldcx, oldcy)
*/
static GXDWORD LISTVIEW_SetIconSpacing(LISTVIEW_INFO *infoPtr, GXINT cx, GXINT cy)
{
  GXDWORD oldspacing = GXMAKELONG(infoPtr->iconSpacing.cx, infoPtr->iconSpacing.cy);
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;

  TRACE("requested=(%d,%d)\n", cx, cy);

  /* this is supported only for LVS_ICON style */
  if (uView != LVS_ICON) return oldspacing;

  /* set to defaults, if instructed to */
  if (cx == -1) cx = gxGetSystemMetrics(GXSM_CXICONSPACING);
  if (cy == -1) cy = gxGetSystemMetrics(GXSM_CYICONSPACING);

  /* if 0 then compute width
  * FIXME: Should scan each item and determine max width of
  *        icon or label, then make that the width */
  if (cx == 0)
    cx = infoPtr->iconSpacing.cx;

  /* if 0 then compute height */
  if (cy == 0) 
    cy = infoPtr->iconSize.cy + 2 * infoPtr->ntmHeight +
    ICON_BOTTOM_PADDING + ICON_TOP_PADDING + LABEL_VERT_PADDING;


  infoPtr->iconSpacing.cx = cx;
  infoPtr->iconSpacing.cy = cy;

  TRACE("old=(%d,%d), new=(%d,%d), iconSize=(%d,%d), ntmH=%d\n",
    GXLOWORD(oldspacing), GXHIWORD(oldspacing), cx, cy, 
    infoPtr->iconSize.cx, infoPtr->iconSize.cy,
    infoPtr->ntmHeight);

  /* these depend on the iconSpacing */
  LISTVIEW_UpdateItemSize(infoPtr);

  return oldspacing;
}

static inline void set_icon_size(GXSIZE *size, GXHIMAGELIST himl, GXBOOL _small)
{
  GXINT cx, cy;

  if (himl && gxImageList_GetIconSize(himl, &cx, &cy))
  {
    size->cx = cx;
    size->cy = cy;
  }
  else
  {
    size->cx = gxGetSystemMetrics(_small ? GXSM_CXSMICON : GXSM_CXICON);
    size->cy = gxGetSystemMetrics(_small ? GXSM_CYSMICON : GXSM_CYICON);
  }
}

/***
* DESCRIPTION:
* Sets image lists.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nType : image list type
* [I] himl : image list handle
*
* RETURN:
*   SUCCESS : old image list
*   FAILURE : NULL
*/
static GXHIMAGELIST LISTVIEW_SetImageList(LISTVIEW_INFO *infoPtr, GXINT nType, GXHIMAGELIST himl)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT oldHeight = infoPtr->nItemHeight;
  GXHIMAGELIST himlOld = 0;

  TRACE("(nType=%d, himl=%p\n", nType, himl);

  switch (nType)
  {
  case LVSIL_NORMAL:
    himlOld = infoPtr->himlNormal;
    infoPtr->himlNormal = himl;
    if (uView == LVS_ICON) set_icon_size(&infoPtr->iconSize, himl, FALSE);
    LISTVIEW_SetIconSpacing(infoPtr, 0, 0);
    break;

  case LVSIL_SMALL:
    himlOld = infoPtr->himlSmall;
    infoPtr->himlSmall = himl;
    if (uView != LVS_ICON) set_icon_size(&infoPtr->iconSize, himl, TRUE);
    break;

  case LVSIL_STATE:
    himlOld = infoPtr->himlState;
    infoPtr->himlState = himl;
    set_icon_size(&infoPtr->iconStateSize, himl, TRUE);
    gxImageList_SetBkColor(infoPtr->himlState, GXCLR_NONE);
    break;

  default:
    ERR("Unknown icon type=%d\n", nType);
    return NULL;
  }

  infoPtr->nItemHeight = LISTVIEW_CalculateItemHeight(infoPtr);
  if (infoPtr->nItemHeight != oldHeight)
    LISTVIEW_UpdateScroll(infoPtr);

  return himlOld;
}

/***
* DESCRIPTION:
* Preallocates memory (does *not* set the actual count of items !)
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItems : item count (projected number of items to allocate)
* [I] dwFlags : update flags
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetItemCount(LISTVIEW_INFO *infoPtr, GXINT nItems, GXDWORD dwFlags)
{
  TRACE("(nItems=%d, dwFlags=%x)\n", nItems, dwFlags);

  if (infoPtr->dwStyle & LVS_OWNERDATA)
  {
    GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
    GXINT nOldCount = infoPtr->nItemCount;

    if (nItems < nOldCount)
    {
      RANGE range = { nItems, nOldCount };
      ranges_del(infoPtr->selectionRanges, range);
      if (infoPtr->nFocusedItem >= nItems)
      {
        infoPtr->nFocusedItem = -1;
        gxSetRectEmpty(&infoPtr->rcFocus);
      }
    }

    infoPtr->nItemCount = nItems;
    LISTVIEW_UpdateScroll(infoPtr);

    /* the flags are valid only in ownerdata report and list modes */
    if (uView == LVS_ICON || uView == LVS_SMALLICON) dwFlags = 0;

    if (!(dwFlags & LVSICF_NOSCROLL) && infoPtr->nFocusedItem != -1)
      LISTVIEW_EnsureVisible(infoPtr, infoPtr->nFocusedItem, FALSE);

    if (!(dwFlags & LVSICF_NOINVALIDATEALL))
      LISTVIEW_InvalidateList(infoPtr);
    else
    {
      GXINT nFrom, nTo;
      GXPOINT Origin;
      GXRECT rcErase;

      LISTVIEW_GetOrigin(infoPtr, &Origin);
      nFrom = min(nOldCount, nItems);
      nTo = max(nOldCount, nItems);

      if (uView == LVS_REPORT)
      {
        rcErase.left = 0;
        rcErase.top = nFrom * infoPtr->nItemHeight;
        rcErase.right = infoPtr->nItemWidth;
        rcErase.bottom = nTo * infoPtr->nItemHeight;
        gxOffsetRect(&rcErase, Origin.x, Origin.y);
        if (gxIntersectRect(&rcErase, &rcErase, &infoPtr->rcList))
          LISTVIEW_InvalidateRect(infoPtr, &rcErase);
      }
      else /* LVS_LIST */
      {
        GXINT nPerCol = LISTVIEW_GetCountPerColumn(infoPtr);

        rcErase.left = (nFrom / nPerCol) * infoPtr->nItemWidth;
        rcErase.top = (nFrom % nPerCol) * infoPtr->nItemHeight;
        rcErase.right = rcErase.left + infoPtr->nItemWidth;
        rcErase.bottom = nPerCol * infoPtr->nItemHeight;
        gxOffsetRect(&rcErase, Origin.x, Origin.y);
        if (gxIntersectRect(&rcErase, &rcErase, &infoPtr->rcList))
          LISTVIEW_InvalidateRect(infoPtr, &rcErase);

        rcErase.left = (nFrom / nPerCol + 1) * infoPtr->nItemWidth;
        rcErase.top = 0;
        rcErase.right = (nTo / nPerCol + 1) * infoPtr->nItemWidth;
        rcErase.bottom = nPerCol * infoPtr->nItemHeight;
        gxOffsetRect(&rcErase, Origin.x, Origin.y);
        if (gxIntersectRect(&rcErase, &rcErase, &infoPtr->rcList))
          LISTVIEW_InvalidateRect(infoPtr, &rcErase);
      }
    }
  }
  else
  {
    /* According to MSDN for non-LVS_OWNERDATA this is just
    * a performance issue. The control allocates its internal
    * data structures for the number of items specified. It
    * cuts down on the number of memory allocations. Therefore
    * we will just issue a WARN here
    */
    WARN("for non-ownerdata performance option not implemented.\n");
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Sets the position of an item.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] pt : coordinate
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetItemPosition(LISTVIEW_INFO *infoPtr, GXINT nItem, GXPOINT pt)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXPOINT Origin;

  TRACE("(nItem=%d, &pt=%s\n", nItem, wine_dbgstr_point(&pt));

  if (nItem < 0 || nItem >= infoPtr->nItemCount ||
    !(uView == LVS_ICON || uView == LVS_SMALLICON)) return FALSE;

  LISTVIEW_GetOrigin(infoPtr, &Origin);

  /* This point value seems to be an undocumented feature.
  * The best guess is that it means either at the origin, 
  * or at true beginning of the list. I will assume the origin. */
  if ((pt.x == -1) && (pt.y == -1))
    pt = Origin;

  if (uView == LVS_ICON)
  {
    pt.x -= (infoPtr->nItemWidth - infoPtr->iconSize.cx) / 2;
    pt.y -= ICON_TOP_PADDING;
  }
  pt.x -= Origin.x;
  pt.y -= Origin.y;

  infoPtr->bAutoarrange = FALSE;

  return LISTVIEW_MoveIconTo(infoPtr, nItem, &pt, FALSE);
}

/***
* DESCRIPTION:
* Sets the state of one or many items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
* [I] lpLVItem : item or subitem info
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetItemState(LISTVIEW_INFO *infoPtr, GXINT nItem, const GXLVITEMW *lpLVItem)
{
  GXBOOL bResult = TRUE;
  GXLVITEMW lvItem;

  lvItem.iItem = nItem;
  lvItem.iSubItem = 0;
  lvItem.mask = GXLVIF_STATE;
  lvItem.state = lpLVItem->state;
  lvItem.stateMask = lpLVItem->stateMask;
  TRACE("lvItem=%s\n", debuglvitem_t(&lvItem, TRUE));

  if (nItem == -1)
  {
    /* apply to all items */
    for (lvItem.iItem = 0; lvItem.iItem < infoPtr->nItemCount; lvItem.iItem++)
      if (!LISTVIEW_SetItemT(infoPtr, &lvItem, TRUE)) bResult = FALSE;
  }
  else
    bResult = LISTVIEW_SetItemT(infoPtr, &lvItem, TRUE);

  /*
  * Update selection mark
  *
  * Investigation on windows 2k showed that selection mark was updated
  * whenever a new selection was made, but if the selected item was
  * unselected it was not updated.
  *
  * we are probably still not 100% accurate, but this at least sets the
  * proper selection mark when it is needed
  */

  if (bResult && (lvItem.state & lvItem.stateMask & LVIS_SELECTED) &&
    (infoPtr->nSelectionMark == -1))
  {
    int i;
    for (i = 0; i < infoPtr->nItemCount; i++)
    {
      if (infoPtr->uCallbackMask & LVIS_SELECTED)
      {
        if (LISTVIEW_GetItemState(infoPtr, i, LVIS_SELECTED))
        {
          infoPtr->nSelectionMark = i;
          break;
        }
      }
      else if (ranges_contain(infoPtr->selectionRanges, i))
      {
        infoPtr->nSelectionMark = i;
        break;
      }
    }
  }

  return bResult;
}

/***
* DESCRIPTION:
* Sets the text of an item or subitem.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] nItem : item index
* [I] lpLVItem : item or subitem info
* [I] isW : TRUE if input is Unicode
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetItemTextT(LISTVIEW_INFO *infoPtr, GXINT nItem, const GXLVITEMW *lpLVItem, GXBOOL isW)
{
  GXLVITEMW lvItem;

  if (nItem < 0 && nItem >= infoPtr->nItemCount) return FALSE;

  lvItem.iItem = nItem;
  lvItem.iSubItem = lpLVItem->iSubItem;
  lvItem.mask = GXLVIF_TEXT;
  lvItem.pszText = lpLVItem->pszText;
  lvItem.cchTextMax = lpLVItem->cchTextMax;

  TRACE("(nItem=%d, lpLVItem=%s, isW=%d)\n", nItem, debuglvitem_t(&lvItem, isW), isW);

  return LISTVIEW_SetItemT(infoPtr, &lvItem, isW); 
}

/***
* DESCRIPTION:
* Set item index that marks the start of a multiple selection.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nIndex : index
*
* RETURN:
* Index number or -1 if there is no selection mark.
*/
static GXINT LISTVIEW_SetSelectionMark(LISTVIEW_INFO *infoPtr, GXINT nIndex)
{
  GXINT nOldIndex = infoPtr->nSelectionMark;

  TRACE("(nIndex=%d)\n", nIndex);

  infoPtr->nSelectionMark = nIndex;

  return nOldIndex;
}

/***
* DESCRIPTION:
* Sets the text background color.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] clrTextBk : text background color
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetTextBkColor(LISTVIEW_INFO *infoPtr, GXCOLORREF clrTextBk)
{
  TRACE("(clrTextBk=%x)\n", clrTextBk);

  if (infoPtr->clrTextBk != clrTextBk)
  {
    infoPtr->clrTextBk = clrTextBk;
    LISTVIEW_InvalidateList(infoPtr);
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Sets the text foreground color.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] clrText : text color
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SetTextColor (LISTVIEW_INFO *infoPtr, GXCOLORREF clrText)
{
  TRACE("(clrText=%x)\n", clrText);

  if (infoPtr->clrText != clrText)
  {
    infoPtr->clrText = clrText;
    LISTVIEW_InvalidateList(infoPtr);
  }

  return TRUE;
}

/***
* DESCRIPTION:
* Determines which listview item is located at the specified position.
*
* PARAMETER(S):
* [I] infoPtr        : valid pointer to the listview structure
* [I] hwndNewToolTip : handle to new ToolTip
*
* RETURN:
*   old tool tip
*/
static GXHWND LISTVIEW_SetToolTips( LISTVIEW_INFO *infoPtr, GXHWND hwndNewToolTip)
{
  GXHWND hwndOldToolTip = infoPtr->hwndToolTip;
  infoPtr->hwndToolTip = hwndNewToolTip;
  return hwndOldToolTip;
}

/*
* DESCRIPTION:
*   sets the Unicode character format flag for the control
* PARAMETER(S):
*    [I] infoPtr         :valid pointer to the listview structure
*    [I] fUnicode        :true to switch to UNICODE false to switch to ANSI
*
* RETURN:
*    Old Unicode Format
*/
static GXBOOL LISTVIEW_SetUnicodeFormat( LISTVIEW_INFO *infoPtr, GXBOOL fUnicode)
{
  GXBOOL rc = infoPtr->notifyFormat;
  infoPtr->notifyFormat = (fUnicode)?GXNFR_UNICODE:GXNFR_ANSI;
  return rc;
}

/* LISTVIEW_SetWorkAreas */

/***
* DESCRIPTION:
* Callback internally used by LISTVIEW_SortItems()
*
* PARAMETER(S):
* [I] first : pointer to first ITEM_INFO to compare
* [I] second : pointer to second ITEM_INFO to compare
* [I] lParam : GXHWND of control
*
* RETURN:
*   if first comes before second : negative
*   if first comes after second : positive
*   if first and second are equivalent : zero
*/
static GXINT GXCALLBACK LISTVIEW_CallBackCompare(GXLPVOID first, GXLPVOID second, GXLPARAM lParam)
{
  LISTVIEW_INFO *infoPtr = (LISTVIEW_INFO *)lParam;
  ITEM_INFO* lv_first = (ITEM_INFO*) gxDPA_GetPtr( (GXHDPA)first, 0 );
  ITEM_INFO* lv_second = (ITEM_INFO*) gxDPA_GetPtr( (GXHDPA)second, 0 );

  /* Forward the call to the client defined callback */
  return (infoPtr->pfnCompare)( lv_first->lParam , lv_second->lParam, infoPtr->lParamSort );
}

/***
* DESCRIPTION:
* Sorts the listview items.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] pfnCompare : application-defined value
* [I] lParamSort : pointer to comparison callback
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_SortItems(LISTVIEW_INFO *infoPtr, GXPFNLVCOMPARE pfnCompare, GXLPARAM lParamSort)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXHDPA hdpaSubItems;
  ITEM_INFO *lpItem;
  GXLPVOID selectionMarkItem;
  GXLVITEMW item;
  int i;

  TRACE("(pfnCompare=%p, lParamSort=%lx)\n", pfnCompare, lParamSort);

  if (infoPtr->dwStyle & LVS_OWNERDATA) return FALSE;

  if (!pfnCompare) return FALSE;
  if (!infoPtr->hdpaItems) return FALSE;

  /* if there are 0 or 1 items, there is no need to sort */
  if (infoPtr->nItemCount < 2) return TRUE;

  if (infoPtr->nFocusedItem >= 0)
  {
    hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, infoPtr->nFocusedItem);
    lpItem = (ITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, 0);
    if (lpItem) lpItem->state |= LVIS_FOCUSED;
  }
  /* FIXME: go thorugh selected items and mark them so in lpItem->state */
  /*        clear the lpItem->state for non-selected ones */
  /*        remove the selection ranges */

  infoPtr->pfnCompare = pfnCompare;
  infoPtr->lParamSort = lParamSort;
  gxDPA_Sort(infoPtr->hdpaItems, LISTVIEW_CallBackCompare, (GXLPARAM)infoPtr);

  /* Adjust selections and indices so that they are the way they should
  * be after the sort (otherwise, the list items move around, but
  * whatever is at the item's previous original position will be
  * selected instead)
  */
  selectionMarkItem=(infoPtr->nSelectionMark>=0)?gxDPA_GetPtr(infoPtr->hdpaItems, infoPtr->nSelectionMark):NULL;
  for (i=0; i < infoPtr->nItemCount; i++)
  {
    hdpaSubItems = (GXHDPA)gxDPA_GetPtr(infoPtr->hdpaItems, i);
    lpItem = (ITEM_INFO *)gxDPA_GetPtr(hdpaSubItems, 0);

    if (lpItem->state & LVIS_SELECTED)
    {
      item.state = LVIS_SELECTED;
      item.stateMask = LVIS_SELECTED;
      LISTVIEW_SetItemState(infoPtr, i, &item);
    }
    if (lpItem->state & LVIS_FOCUSED)
    {
      infoPtr->nFocusedItem = i;
      lpItem->state &= ~LVIS_FOCUSED;
    }
  }
  if (selectionMarkItem != NULL)
    infoPtr->nSelectionMark = gxDPA_GetPtrIndex(infoPtr->hdpaItems, selectionMarkItem);
  /* I believe nHotItem should be left alone, see LISTVIEW_ShiftIndices */

  /* refresh the display */
  if (uView != LVS_ICON && uView != LVS_SMALLICON)
    LISTVIEW_InvalidateList(infoPtr);

  return TRUE;
}

/***
* DESCRIPTION:
* Update theme handle after a theme change.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*   SUCCESS : 0
*   FAILURE : something else
*/
static GXLRESULT LISTVIEW_ThemeChanged(const LISTVIEW_INFO *infoPtr)
{
  GXHTHEME theme = gxGetWindowTheme(infoPtr->hwndSelf);
  gxCloseThemeData (theme);
  gxOpenThemeData (infoPtr->hwndSelf, themeClass);
  return 0;
}

/***
* DESCRIPTION:
* Updates an items or rearranges the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nItem : item index
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_Update(LISTVIEW_INFO *infoPtr, GXINT nItem)
{
  TRACE("(nItem=%d)\n", nItem);

  if (nItem < 0 || nItem >= infoPtr->nItemCount) return FALSE;

  /* rearrange with default alignment style */
  if (is_autoarrange(infoPtr))
    LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);
  else
    LISTVIEW_InvalidateItem(infoPtr, nItem);

  return TRUE;
}

/***
* DESCRIPTION:
* Draw the track line at the place defined in the infoPtr structure.
* The line is drawn with a XOR pen so drawing the line for the second time
* in the same place erases the line.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_DrawTrackLine(const LISTVIEW_INFO *infoPtr)
{
  GXHPEN hOldPen;
  GXHDC hdc;
  GXINT oldROP;

  if (infoPtr->xTrackLine == -1)
    return FALSE;

  if (!(hdc = gxGetDC(infoPtr->hwndSelf)))
    return FALSE;
  hOldPen = (GXHPEN)gxSelectObject(hdc, gxGetStockObject(GXBLACK_PEN));
  oldROP = gxSetROP2(hdc, GXR2_XORPEN);
  gxMoveToEx(hdc, infoPtr->xTrackLine, infoPtr->rcList.top, NULL);
  gxLineTo(hdc, infoPtr->xTrackLine, infoPtr->rcList.bottom);
  gxSetROP2(hdc, oldROP);
  gxSelectObject(hdc, hOldPen);
  gxReleaseDC(infoPtr->hwndSelf, hdc);
  return TRUE;
}

/***
* DESCRIPTION:
* Called when an edit control should be displayed. This function is called after
* we are sure that there was a single click - not a double click (this is a TIMERPROC).
*
* PARAMETER(S):
* [I] hwnd : Handle to the listview
* [I] uMsg : WM_TIMER (ignored)
* [I] idEvent : The timer ID interpreted as a pointer to a DELAYED_EDIT_ITEM struct
* [I] dwTimer : The elapsed time (ignored)
*
* RETURN:
*   None.
*/
static GXVOID GXCALLBACK LISTVIEW_DelayedEditItem(GXHWND hwnd, GXUINT uMsg, GXUINT idEvent, GXDWORD dwTime)
{
  DELAYED_ITEM_EDIT *editItem = (DELAYED_ITEM_EDIT *)idEvent;
  LISTVIEW_INFO *infoPtr = (LISTVIEW_INFO *)gxGetWindowLongPtrW(hwnd, 0);

  gxKillTimer(hwnd, idEvent);
  editItem->fEnabled = FALSE;
  /* check if the item is still selected */
  if (infoPtr->bFocus && LISTVIEW_GetItemState(infoPtr, editItem->iItem, LVIS_SELECTED))
    LISTVIEW_EditLabelT(infoPtr, editItem->iItem, TRUE);
}

/***
* DESCRIPTION:
* Creates the listview control - the WM_NCCREATE phase.
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] lpcs : the create parameters
*
* RETURN:
*   Success: TRUE
*   Failure: FALSE
*/
static GXLRESULT LISTVIEW_NCCreate(GXHWND hwnd, const GXCREATESTRUCTW *lpcs)
{
  LISTVIEW_INFO *infoPtr;
  GXLOGFONTW logFont;

  TRACE("(lpcs=%p)\n", lpcs);

  /* initialize info pointer */
  infoPtr = (LISTVIEW_INFO *)Alloc(sizeof(LISTVIEW_INFO));
  if (!infoPtr) return FALSE;

  gxSetWindowLongPtrW(hwnd, 0, (GXDWORD_PTR)infoPtr);

  infoPtr->hwndSelf = hwnd;
  infoPtr->dwStyle = lpcs->style;    /* Note: may be changed in WM_CREATE */
  /* determine the type of structures to use */
  infoPtr->hwndNotify = lpcs->hwndParent;
  /* infoPtr->notifyFormat will be filled in WM_CREATE */

  /* initialize color information  */
  infoPtr->clrBk = GXCLR_NONE;
  infoPtr->clrText = GXCLR_DEFAULT;
  infoPtr->clrTextBk = GXCLR_DEFAULT;
  infoPtr->hBkBrush = gxCreateSolidBrush(infoPtr->clrBk);
  LISTVIEW_SetBkColor(infoPtr, comctl32_color.clrWindow);

  /* set default values */
  infoPtr->nFocusedItem = -1;
  infoPtr->nSelectionMark = -1;
  infoPtr->nHotItem = -1;
  infoPtr->bRedraw = TRUE;
  infoPtr->bNoItemMetrics = TRUE;
  infoPtr->bDoChangeNotify = TRUE;
  infoPtr->iconSpacing.cx = gxGetSystemMetrics(GXSM_CXICONSPACING);
  infoPtr->iconSpacing.cy = gxGetSystemMetrics(GXSM_CYICONSPACING);
  infoPtr->nEditLabelItem = -1;
  infoPtr->dwHoverTime = -1; /* default system hover time */
  infoPtr->nMeasureItemHeight = 0;
  infoPtr->xTrackLine = -1;  /* no track line */
  infoPtr->itemEdit.fEnabled = FALSE;

  /* get default font (icon title) */
  gxSystemParametersInfoW(SPI_GETICONTITLELOGFONT, 0, &logFont, 0);
  infoPtr->hDefaultFont = gxCreateFontIndirectW(&logFont);
  infoPtr->hFont = infoPtr->hDefaultFont;
  LISTVIEW_SaveTextMetrics(infoPtr);

  /* allocate memory for the data structure */
  if (!(infoPtr->selectionRanges = ranges_create(10))) goto fail;
  if (!(infoPtr->hdpaItems = gxDPA_Create(10))) goto fail;
  if (!(infoPtr->hdpaPosX  = gxDPA_Create(10))) goto fail;
  if (!(infoPtr->hdpaPosY  = gxDPA_Create(10))) goto fail;
  if (!(infoPtr->hdpaColumns = gxDPA_Create(10))) goto fail;
  return TRUE;

fail:
  gxDestroyWindow(infoPtr->hwndHeader);
  ranges_destroy(infoPtr->selectionRanges);
  gxDPA_Destroy(infoPtr->hdpaItems);
  gxDPA_Destroy(infoPtr->hdpaPosX);
  gxDPA_Destroy(infoPtr->hdpaPosY);
  gxDPA_Destroy(infoPtr->hdpaColumns);
  Free(infoPtr);
  return FALSE;
}

/***
* DESCRIPTION:
* Creates the listview control - the WM_CREATE phase. Most of the data is
* already set up in LISTVIEW_NCCreate
*
* PARAMETER(S):
* [I] hwnd : window handle
* [I] lpcs : the create parameters
*
* RETURN:
*   Success: 0
*   Failure: -1
*/
static GXLRESULT LISTVIEW_Create(GXHWND hwnd, const GXCREATESTRUCTW *lpcs)
{
  LISTVIEW_INFO *infoPtr = (LISTVIEW_INFO *)gxGetWindowLongPtrW(hwnd, 0);
  GXUINT uView = lpcs->style & LVS_TYPEMASK;
  GXDWORD dFlags = GXWS_CHILD | HDS_HORZ | HDS_FULLDRAG | HDS_DRAGDROP;

  TRACE("(lpcs=%p)\n", lpcs);

  infoPtr->dwStyle = lpcs->style;
  infoPtr->notifyFormat = gxSendMessageW(infoPtr->hwndNotify, GXWM_NOTIFYFORMAT,
    (GXWPARAM)infoPtr->hwndSelf, (GXLPARAM)GXNF_QUERY);

  /* setup creation flags */
  dFlags |= (LVS_NOSORTHEADER & lpcs->style) ? 0 : HDS_BUTTONS;
  dFlags |= (LVS_NOCOLUMNHEADER & lpcs->style) ? HDS_HIDDEN : 0;

  /* create header */
  infoPtr->hwndHeader = gxCreateWindowW(WC_HEADERW, NULL, dFlags,
    0, 0, 0, 0, hwnd, NULL,
    lpcs->hInstance, NULL);
  if (!infoPtr->hwndHeader) return -1;

  /* set header unicode format */
  gxSendMessageW(infoPtr->hwndHeader, GXHDM_SETUNICODEFORMAT, (GXWPARAM)TRUE, (GXLPARAM)NULL);

  /* set header font */
  gxSendMessageW(infoPtr->hwndHeader, GXWM_SETFONT, (GXWPARAM)infoPtr->hFont, (GXLPARAM)TRUE);

  /* init item size to avoid division by 0 */
  LISTVIEW_UpdateItemSize (infoPtr);

  if (uView == LVS_REPORT)
  {
    if (!(LVS_NOCOLUMNHEADER & lpcs->style))
    {
      gxShowWindow(infoPtr->hwndHeader, GXSW_SHOWNORMAL);
    }
    LISTVIEW_UpdateSize(infoPtr);
    LISTVIEW_UpdateScroll(infoPtr);
  }

  gxOpenThemeData(hwnd, themeClass);

  /* initialize the icon sizes */
  set_icon_size(&infoPtr->iconSize, infoPtr->himlNormal, uView != LVS_ICON);
  set_icon_size(&infoPtr->iconStateSize, infoPtr->himlState, TRUE);
  return 0;
}

/***
* DESCRIPTION:
* Destroys the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*   Success: 0
*   Failure: -1
*/
static GXLRESULT LISTVIEW_Destroy(const LISTVIEW_INFO *infoPtr)
{
  while(LISTVIEW_DeleteColumn((LISTVIEW_INFO *)infoPtr, 0) != FALSE);

  GXHTHEME theme = gxGetWindowTheme(infoPtr->hwndSelf);
  gxCloseThemeData(theme);
  return 0;
}

/***
* DESCRIPTION:
* Enables the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] bEnable : specifies whether to enable or disable the window
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static GXBOOL LISTVIEW_Enable(const LISTVIEW_INFO *infoPtr, GXBOOL bEnable)
{
  if (infoPtr->dwStyle & LVS_OWNERDRAWFIXED)
    gxInvalidateRect(infoPtr->hwndSelf, NULL, TRUE);
  return TRUE;
}

/***
* DESCRIPTION:
* Erases the background of the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
*
* RETURN:
*   SUCCESS : TRUE
*   FAILURE : FALSE
*/
static inline GXBOOL LISTVIEW_EraseBkgnd(const LISTVIEW_INFO *infoPtr, GXHDC hdc)
{
  GXRECT rc;

  TRACE("(hdc=%p)\n", hdc);

  if (!gxGetClipBox(hdc, &rc)) return FALSE;

  /* for double buffered controls we need to do this during refresh */
  if (infoPtr->dwLvExStyle & LVS_EX_DOUBLEBUFFER) return FALSE;

  return LISTVIEW_FillBkgnd(infoPtr, hdc, &rc);
}


/***
* DESCRIPTION:
* Helper function for LISTVIEW_[HV]Scroll *only*.
* Performs vertical/horizontal scrolling by a give amount.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] dx : amount of horizontal scroll
* [I] dy : amount of vertical scroll
*/
static void scroll_list(LISTVIEW_INFO *infoPtr, GXINT dx, GXINT dy)
{
  /* now we can scroll the list */
  gxScrollWindowEx(infoPtr->hwndSelf, dx, dy, &infoPtr->rcList, 
    &infoPtr->rcList, 0, 0, GXSW_ERASE | GXSW_INVALIDATE);
  /* if we have focus, adjust rect */
  gxOffsetRect(&infoPtr->rcFocus, dx, dy);
  gxUpdateWindow(infoPtr->hwndSelf);
}

/***
* DESCRIPTION:
* Performs vertical scrolling.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nScrollCode : scroll code
* [I] nScrollDiff : units to scroll in SB_INTERNAL mode, 0 otherwise
* [I] hScrollWnd  : scrollbar control window handle
*
* RETURN:
* Zero
*
* NOTES:
*   GXSB_LINEUP/GXSB_LINEDOWN:
*        for LVS_ICON, LVS_SMALLICON is 37 by experiment
*        for LVS_REPORT is 1 line
*        for LVS_LIST cannot occur
*
*/
static GXLRESULT LISTVIEW_VScroll(LISTVIEW_INFO *infoPtr, GXINT nScrollCode, 
                  GXINT nScrollDiff, GXHWND hScrollWnd)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nOldScrollPos, nNewScrollPos;
  GXSCROLLINFO scrollInfo;
  GXBOOL is_an_icon;

  TRACE("(nScrollCode=%d(%s), nScrollDiff=%d)\n", nScrollCode, 
    debugscrollcode(nScrollCode), nScrollDiff);

  if (infoPtr->hwndEdit) gxSendMessageW(infoPtr->hwndEdit, GXWM_KILLFOCUS, 0, 0);

  scrollInfo.cbSize = sizeof(GXSCROLLINFO);
  scrollInfo.fMask = GXSIF_PAGE | GXSIF_POS | GXSIF_RANGE | GXSIF_TRACKPOS;

  is_an_icon = ((uView == LVS_ICON) || (uView == LVS_SMALLICON));

  if (!gxGetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &scrollInfo)) return 1;

  nOldScrollPos = scrollInfo.nPos;
  switch (nScrollCode)
  {
  case SB_INTERNAL:
    break;

  case GXSB_LINEUP:
    nScrollDiff = (is_an_icon) ? -LISTVIEW_SCROLL_ICON_LINE_SIZE : -1;
    break;

  case GXSB_LINEDOWN:
    nScrollDiff = (is_an_icon) ? LISTVIEW_SCROLL_ICON_LINE_SIZE : 1;
    break;

  case GXSB_PAGEUP:
    nScrollDiff = -(GXINT)scrollInfo.nPage;
    break;

  case GXSB_PAGEDOWN:
    nScrollDiff = scrollInfo.nPage;
    break;

  case GXSB_THUMBPOSITION:
  case GXSB_THUMBTRACK:
    nScrollDiff = scrollInfo.nTrackPos - scrollInfo.nPos;
    break;

  default:
    nScrollDiff = 0;
  }

  /* quit right away if pos isn't changing */
  if (nScrollDiff == 0) return 0;

  /* calculate new position, and handle overflows */
  nNewScrollPos = scrollInfo.nPos + nScrollDiff;
  if (nScrollDiff > 0) {
    if (nNewScrollPos < nOldScrollPos ||
      nNewScrollPos > scrollInfo.nMax)
      nNewScrollPos = scrollInfo.nMax;
  } else {
    if (nNewScrollPos > nOldScrollPos ||
      nNewScrollPos < scrollInfo.nMin)
      nNewScrollPos = scrollInfo.nMin;
  }

  /* set the new position, and reread in case it changed */
  scrollInfo.fMask = GXSIF_POS;
  scrollInfo.nPos = nNewScrollPos;
  nNewScrollPos = gxSetScrollInfo(infoPtr->hwndSelf, GXSB_VERT, &scrollInfo, TRUE);

  /* carry on only if it really changed */
  if (nNewScrollPos == nOldScrollPos) return 0;

  /* now adjust to client coordinates */
  nScrollDiff = nOldScrollPos - nNewScrollPos;
  if (uView == LVS_REPORT) nScrollDiff *= infoPtr->nItemHeight;

  /* and scroll the window */ 
  scroll_list(infoPtr, 0, nScrollDiff);

  return 0;
}

/***
* DESCRIPTION:
* Performs horizontal scrolling.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nScrollCode : scroll code
* [I] nScrollDiff : units to scroll in SB_INTERNAL mode, 0 otherwise
* [I] hScrollWnd  : scrollbar control window handle
*
* RETURN:
* Zero
*
* NOTES:
*   GXSB_LINELEFT/GXSB_LINERIGHT:
*        for LVS_ICON, LVS_SMALLICON  1 pixel
*        for LVS_REPORT is 1 pixel
*        for LVS_LIST  is 1 column --> which is a 1 because the
*                                      scroll is based on columns not pixels
*
*/
static GXLRESULT LISTVIEW_HScroll(LISTVIEW_INFO *infoPtr, GXINT nScrollCode,
                  GXINT nScrollDiff, GXHWND hScrollWnd)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT nOldScrollPos, nNewScrollPos;
  GXSCROLLINFO scrollInfo;

  TRACE("(nScrollCode=%d(%s), nScrollDiff=%d)\n", nScrollCode, 
    debugscrollcode(nScrollCode), nScrollDiff);

  if (infoPtr->hwndEdit) gxSendMessageW(infoPtr->hwndEdit, GXWM_KILLFOCUS, 0, 0);

  scrollInfo.cbSize = sizeof(GXSCROLLINFO);
  scrollInfo.fMask = GXSIF_PAGE | GXSIF_POS | GXSIF_RANGE | GXSIF_TRACKPOS;

  if (!gxGetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &scrollInfo)) return 1;

  nOldScrollPos = scrollInfo.nPos;

  switch (nScrollCode)
  {
  case SB_INTERNAL:
    break;

  case GXSB_LINELEFT:
    nScrollDiff = -1;
    break;

  case GXSB_LINERIGHT:
    nScrollDiff = 1;
    break;

  case GXSB_PAGELEFT:
    nScrollDiff = -(GXINT)scrollInfo.nPage;
    break;

  case GXSB_PAGERIGHT:
    nScrollDiff = scrollInfo.nPage;
    break;

  case GXSB_THUMBPOSITION:
  case GXSB_THUMBTRACK:
    nScrollDiff = scrollInfo.nTrackPos - scrollInfo.nPos;
    break;

  default:
    nScrollDiff = 0;
  }

  /* quit right away if pos isn't changing */
  if (nScrollDiff == 0) return 0;

  /* calculate new position, and handle overflows */
  nNewScrollPos = scrollInfo.nPos + nScrollDiff;
  if (nScrollDiff > 0) {
    if (nNewScrollPos < nOldScrollPos ||
      nNewScrollPos > scrollInfo.nMax)
      nNewScrollPos = scrollInfo.nMax;
  } else {
    if (nNewScrollPos > nOldScrollPos ||
      nNewScrollPos < scrollInfo.nMin)
      nNewScrollPos = scrollInfo.nMin;
  }

  /* set the new position, and reread in case it changed */
  scrollInfo.fMask = GXSIF_POS;
  scrollInfo.nPos = nNewScrollPos;
  nNewScrollPos = gxSetScrollInfo(infoPtr->hwndSelf, GXSB_HORZ, &scrollInfo, TRUE);

  /* carry on only if it really changed */
  if (nNewScrollPos == nOldScrollPos) return 0;

  if(uView == LVS_REPORT)
    LISTVIEW_UpdateHeaderSize(infoPtr, nNewScrollPos);

  /* now adjust to client coordinates */
  nScrollDiff = nOldScrollPos - nNewScrollPos;
  if (uView == LVS_LIST) nScrollDiff *= infoPtr->nItemWidth;

  /* and scroll the window */
  scroll_list(infoPtr, nScrollDiff, 0);

  return 0;
}

static GXLRESULT LISTVIEW_MouseWheel(LISTVIEW_INFO *infoPtr, GXINT wheelDelta)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
  GXINT gcWheelDelta = 0;
  GXINT pulScrollLines = 3;
  GXSCROLLINFO scrollInfo;

  TRACE("(wheelDelta=%d)\n", wheelDelta);

  gxSystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0, &pulScrollLines, 0);
  gcWheelDelta -= wheelDelta;

  scrollInfo.cbSize = sizeof(GXSCROLLINFO);
  scrollInfo.fMask = GXSIF_POS;

  switch(uView)
  {
  case LVS_ICON:
  case LVS_SMALLICON:
    /*
    *  listview should be scrolled by a multiple of 37 dependently on its dimension or its visible item number
    *  should be fixed in the future.
    */
    LISTVIEW_VScroll(infoPtr, SB_INTERNAL, (gcWheelDelta < 0) ?
      -LISTVIEW_SCROLL_ICON_LINE_SIZE : LISTVIEW_SCROLL_ICON_LINE_SIZE, 0);
    break;

  case LVS_REPORT:
    if (abs(gcWheelDelta) >= GXWHEEL_DELTA && pulScrollLines)
    {
      int cLineScroll = min(LISTVIEW_GetCountPerColumn(infoPtr), pulScrollLines);
      cLineScroll *= (gcWheelDelta / GXWHEEL_DELTA);
      LISTVIEW_VScroll(infoPtr, SB_INTERNAL, cLineScroll, 0);
    }
    break;

  case LVS_LIST:
    LISTVIEW_HScroll(infoPtr, (gcWheelDelta < 0) ? GXSB_LINELEFT : GXSB_LINERIGHT, 0, 0);
    break;
  }
  return 0;
}

/***
* DESCRIPTION:
* ???
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nVirtualKey : virtual key
* [I] lKeyData : key data
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_KeyDown(LISTVIEW_INFO *infoPtr, GXINT nVirtualKey, GXLONG lKeyData)
{
  GXUINT uView =  infoPtr->dwStyle & LVS_TYPEMASK;
  GXHWND hwndSelf = infoPtr->hwndSelf;
  GXINT nItem = -1;
  GXNMLVKEYDOWN nmKeyDown;

  TRACE("(nVirtualKey=%d, lKeyData=%d)\n", nVirtualKey, lKeyData);

  /* send LVN_KEYDOWN notification */
  nmKeyDown.wVKey = nVirtualKey;
  nmKeyDown.flags = 0;
  notify_hdr(infoPtr, LVN_KEYDOWN, &nmKeyDown.hdr);
  if (!gxIsWindow(hwndSelf))
    return 0;

  switch (nVirtualKey)
  {
  case GXVK_SPACE:
    nItem = infoPtr->nFocusedItem;
    if (infoPtr->dwLvExStyle & LVS_EX_CHECKBOXES)
      toggle_checkbox_state(infoPtr, infoPtr->nFocusedItem);
    break;

  case GXVK_RETURN:
    if ((infoPtr->nItemCount > 0) && (infoPtr->nFocusedItem != -1))
    {
      if (!notify(infoPtr, GXNM_RETURN)) return 0;
      if (!notify(infoPtr, LVN_ITEMACTIVATE)) return 0;
    }
    break;

  case GXVK_HOME:
    if (infoPtr->nItemCount > 0)
      nItem = 0;
    break;

  case GXVK_END:
    if (infoPtr->nItemCount > 0)
      nItem = infoPtr->nItemCount - 1;
    break;

  case GXVK_LEFT:
    nItem = gxListView_GetNextItem(infoPtr->hwndSelf, infoPtr->nFocusedItem, GXLVNI_TOLEFT);
    break;

  case GXVK_UP:
    nItem = gxListView_GetNextItem(infoPtr->hwndSelf, infoPtr->nFocusedItem, GXLVNI_ABOVE);
    break;

  case GXVK_RIGHT:
    nItem = gxListView_GetNextItem(infoPtr->hwndSelf, infoPtr->nFocusedItem, GXLVNI_TORIGHT);
    break;

  case GXVK_DOWN:
    nItem = gxListView_GetNextItem(infoPtr->hwndSelf, infoPtr->nFocusedItem, GXLVNI_BELOW);
    break;

  case GXVK_PRIOR:
    if (uView == LVS_REPORT)
    {
      GXINT topidx = LISTVIEW_GetTopIndex(infoPtr);
      if (infoPtr->nFocusedItem == topidx)
        nItem = topidx - LISTVIEW_GetCountPerColumn(infoPtr) + 1;
      else
        nItem = topidx;
    }
    else
      nItem = infoPtr->nFocusedItem - LISTVIEW_GetCountPerColumn(infoPtr)
      * LISTVIEW_GetCountPerRow(infoPtr);
    if(nItem < 0) nItem = 0;
    break;

  case GXVK_NEXT:
    if (uView == LVS_REPORT)
    {
      GXINT topidx = LISTVIEW_GetTopIndex(infoPtr);
      GXINT cnt = LISTVIEW_GetCountPerColumn(infoPtr);
      if (infoPtr->nFocusedItem == topidx + cnt - 1)
        nItem = infoPtr->nFocusedItem + cnt - 1;
      else
        nItem = topidx + cnt - 1;
    }
    else
      nItem = infoPtr->nFocusedItem + LISTVIEW_GetCountPerColumn(infoPtr)
      * LISTVIEW_GetCountPerRow(infoPtr);
    if(nItem >= infoPtr->nItemCount) nItem = infoPtr->nItemCount - 1;
    break;
  }

  if ((nItem != -1) && (nItem != infoPtr->nFocusedItem || nVirtualKey == GXVK_SPACE))
    LISTVIEW_KeySelection(infoPtr, nItem);

  return 0;
}

/***
* DESCRIPTION:
* Kills the focus.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_KillFocus(LISTVIEW_INFO *infoPtr)
{
  TRACE("()\n");

  /* if we did not have the focus, there's nothing to do */
  if (!infoPtr->bFocus) return 0;

  /* send NM_KILLFOCUS notification */
  if (!notify(infoPtr, GXNM_KILLFOCUS)) return 0;

  /* if we have a focus rectagle, get rid of it */
  LISTVIEW_ShowFocusRect(infoPtr, FALSE);

  /* set window focus flag */
  infoPtr->bFocus = FALSE;

  /* invalidate the selected items before resetting focus flag */
  LISTVIEW_InvalidateSelectedItems(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Processes double click messages (left mouse button).
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wKey : key flag
* [I] x,y : mouse coordinate
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_LButtonDblClk(LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO htInfo;

  TRACE("(key=%hu, X=%hu, Y=%hu)\n", wKey, x, y);

  /* Cancel the item edition if any */
  if (infoPtr->itemEdit.fEnabled)
  {
    gxKillTimer(infoPtr->hwndSelf, (GXUINT_PTR)&infoPtr->itemEdit);
    infoPtr->itemEdit.fEnabled = FALSE;
  }

  /* send NM_RELEASEDCAPTURE notification */
  if (!notify(infoPtr, GXNM_RELEASEDCAPTURE)) return 0;

  htInfo.pt.x = x;
  htInfo.pt.y = y;

  /* send NM_DBLCLK notification */
  LISTVIEW_HitTest(infoPtr, &htInfo, TRUE, FALSE);
  if (!notify_click(infoPtr, GXNM_DBLCLK, &htInfo)) return 0;

  /* To send the LVN_ITEMACTIVATE, it must be on an Item */
  if(htInfo.iItem != -1) notify_itemactivate(infoPtr,&htInfo);

  return 0;
}

/***
* DESCRIPTION:
* Processes mouse down messages (left mouse button).
*
* PARAMETERS:
*   infoPtr  [I ] valid pointer to the listview structure
*   wKey     [I ] key flag
*   x,y      [I ] mouse coordinate
*
* RETURN:
*   Zero
*/
static GXLRESULT LISTVIEW_LButtonDown(LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO lvHitTestInfo;
  static GXBOOL bGroupSelect = TRUE;
  GXBOOL bReceivedFocus = FALSE;
  GXPOINT pt = { x, y };
  GXINT nItem;

  TRACE("(key=%hu, X=%hu, Y=%hu)\n", wKey, x, y);

  /* send NM_RELEASEDCAPTURE notification */
  if (!notify(infoPtr, GXNM_RELEASEDCAPTURE)) return 0;

  if (!infoPtr->bFocus)
    bReceivedFocus = TRUE;

  /* set left button down flag and record the click position */
  infoPtr->bLButtonDown = TRUE;
  infoPtr->ptClickPos = pt;
  infoPtr->bDragging = FALSE;

  lvHitTestInfo.pt.x = x;
  lvHitTestInfo.pt.y = y;

  nItem = LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, TRUE);
  TRACE("at %s, nItem=%d\n", wine_dbgstr_point(&pt), nItem);
  infoPtr->nEditLabelItem = -1;
  if ((nItem >= 0) && (nItem < infoPtr->nItemCount))
  {
    if ((infoPtr->dwLvExStyle & LVS_EX_CHECKBOXES) && (lvHitTestInfo.flags & GXLVHT_ONITEMSTATEICON))
    {
      toggle_checkbox_state(infoPtr, nItem);
      return 0;
    }

    if (infoPtr->dwStyle & LVS_SINGLESEL)
    {
      if (LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED))
        infoPtr->nEditLabelItem = nItem;
      else
        LISTVIEW_SetSelection(infoPtr, nItem);
    }
    else
    {
      if ((wKey & GXMK_CONTROL) && (wKey & GXMK_SHIFT))
      {
        if (bGroupSelect)
        {
          if (!LISTVIEW_AddGroupSelection(infoPtr, nItem)) return 0;
          LISTVIEW_SetItemFocus(infoPtr, nItem);
          infoPtr->nSelectionMark = nItem;
        }
        else
        {
          GXLVITEMW item;

          item.state = LVIS_SELECTED | LVIS_FOCUSED;
          item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

          LISTVIEW_SetItemState(infoPtr,nItem,&item);
          infoPtr->nSelectionMark = nItem;
        }
      }
      else if (wKey & GXMK_CONTROL)
      {
        GXLVITEMW item;

        bGroupSelect = (LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED) == 0);

        item.state = (bGroupSelect ? LVIS_SELECTED : 0) | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        LISTVIEW_SetItemState(infoPtr, nItem, &item);
        infoPtr->nSelectionMark = nItem;
      }
      else  if (wKey & GXMK_SHIFT)
      {
        LISTVIEW_SetGroupSelection(infoPtr, nItem);
      }
      else
      {
        if (LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED))
          infoPtr->nEditLabelItem = nItem;

        /* set selection (clears other pre-existing selections) */
        LISTVIEW_SetSelection(infoPtr, nItem);
      }
    }
  }
  else
  {
    /* remove all selections */
    LISTVIEW_DeselectAll(infoPtr);
    gxReleaseCapture();
  }

  if (bReceivedFocus)
    infoPtr->nEditLabelItem = -1;

  return 0;
}

/***
* DESCRIPTION:
* Processes mouse up messages (left mouse button).
*
* PARAMETERS:
*   infoPtr [I ] valid pointer to the listview structure
*   wKey    [I ] key flag
*   x,y     [I ] mouse coordinate
*
* RETURN:
*   Zero
*/
static GXLRESULT LISTVIEW_LButtonUp(LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO lvHitTestInfo;

  TRACE("(key=%hu, X=%hu, Y=%hu)\n", wKey, x, y);

  if (!infoPtr->bLButtonDown) return 0;

  lvHitTestInfo.pt.x = x;
  lvHitTestInfo.pt.y = y;

  /* send NM_CLICK notification */
  LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, FALSE);
  if (!notify_click(infoPtr, GXNM_CLICK, &lvHitTestInfo)) return 0;

  /* set left button flag */
  infoPtr->bLButtonDown = FALSE;

  if (infoPtr->bDragging)
  {
    infoPtr->bDragging = FALSE;
    return 0;
  }

  /* if we clicked on a selected item, edit the label */
  if(lvHitTestInfo.iItem == infoPtr->nEditLabelItem && (lvHitTestInfo.flags & GXLVHT_ONITEMLABEL))
  {
    /* we want to make sure the user doesn't want to do a double click. So we will
    * delay the edit. WM_LBUTTONDBLCLICK will cancel the timer
    */
    infoPtr->itemEdit.fEnabled = TRUE;
    infoPtr->itemEdit.iItem = lvHitTestInfo.iItem;
    gxSetTimer(infoPtr->hwndSelf,
      (GXUINT_PTR)&infoPtr->itemEdit,
      gxGetDoubleClickTime(),
      LISTVIEW_DelayedEditItem);
  }

  if (!infoPtr->bFocus)
    gxSetFocus(infoPtr->hwndSelf);

  return 0;
}

/***
* DESCRIPTION:
* Destroys the listview control (called after WM_DESTROY).
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_NCDestroy(LISTVIEW_INFO *infoPtr)
{
  TRACE("()\n");

  /* delete all items */
  LISTVIEW_DeleteAllItems(infoPtr, TRUE);

  /* destroy data structure */
  gxDPA_Destroy(infoPtr->hdpaItems);
  gxDPA_Destroy(infoPtr->hdpaPosX);
  gxDPA_Destroy(infoPtr->hdpaPosY);
  gxDPA_Destroy(infoPtr->hdpaColumns);
  ranges_destroy(infoPtr->selectionRanges);

  /* destroy image lists */
  if (!(infoPtr->dwStyle & LVS_SHAREIMAGELISTS))
  {
    if (infoPtr->himlNormal)
      gxImageList_Destroy(infoPtr->himlNormal);
    if (infoPtr->himlSmall)
      gxImageList_Destroy(infoPtr->himlSmall);
    if (infoPtr->himlState)
      gxImageList_Destroy(infoPtr->himlState);
  }

  /* destroy font, bkgnd brush */
  infoPtr->hFont = 0;
  if (infoPtr->hDefaultFont) gxDeleteObject(infoPtr->hDefaultFont);
  //if (infoPtr->clrBk != GXCLR_NONE) 
  gxDeleteObject(infoPtr->hBkBrush);

  gxSetWindowLongPtrW(infoPtr->hwndSelf, 0, 0);

  /* free listview info pointer*/
  Free(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Handles notifications from header.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] nCtrlId : control identifier
* [I] lpnmh : notification information
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_HeaderNotification(LISTVIEW_INFO *infoPtr, const GXNMHEADERW *lpnmh)
{
  GXUINT uView =  infoPtr->dwStyle & LVS_TYPEMASK;
  GXHWND hwndSelf = infoPtr->hwndSelf;

  TRACE("(lpnmh=%p)\n", lpnmh);

  if (!lpnmh || lpnmh->iItem < 0 || lpnmh->iItem >= gxDPA_GetPtrCount(infoPtr->hdpaColumns)) return 0;

  switch (lpnmh->hdr.code)
  {    
  case HDN_TRACKW:
  case HDN_TRACKA:
    {
      COLUMN_INFO *lpColumnInfo;
      GXPOINT ptOrigin;
      GXINT x;

      if (!lpnmh->pitem || !(lpnmh->pitem->mask & HDI_WIDTH))
        break;

      /* remove the old line (if any) */
      LISTVIEW_DrawTrackLine(infoPtr);

      /* compute & draw the new line */
      lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, lpnmh->iItem);
      x = lpColumnInfo->rcHeader.left + lpnmh->pitem->cxy;
      LISTVIEW_GetOrigin(infoPtr, &ptOrigin);
      infoPtr->xTrackLine = x + ptOrigin.x;
      LISTVIEW_DrawTrackLine(infoPtr);
      break;
    }

  case HDN_ENDTRACKA:
  case HDN_ENDTRACKW:
    /* remove the track line (if any) */
    LISTVIEW_DrawTrackLine(infoPtr);
    infoPtr->xTrackLine = -1;
    break;

  case HDN_ENDDRAG:
    FIXME("Changing column order not implemented\n");
    return TRUE;

  case HDN_ITEMCHANGINGW:
  case HDN_ITEMCHANGINGA:
    return notify_forward_header(infoPtr, lpnmh);

  case HDN_ITEMCHANGEDW:
  case HDN_ITEMCHANGEDA:
    {
      COLUMN_INFO *lpColumnInfo;
      GXINT dx, cxy;

      notify_forward_header(infoPtr, lpnmh);
      if (!gxIsWindow(hwndSelf))
        break;

      if (!lpnmh->pitem || !(lpnmh->pitem->mask & HDI_WIDTH))
      {
        GXHDITEMW hdi;

        hdi.mask = HDI_WIDTH;
        if (!gxHeader_GetItemW(infoPtr->hwndHeader, lpnmh->iItem, &hdi)) return 0;
        cxy = hdi.cxy;
      }
      else
        cxy = lpnmh->pitem->cxy;

      /* determine how much we change since the last know position */
      lpColumnInfo = LISTVIEW_GetColumnInfo(infoPtr, lpnmh->iItem);
      dx = cxy - (lpColumnInfo->rcHeader.right - lpColumnInfo->rcHeader.left);
      if (dx != 0)
      {
        lpColumnInfo->rcHeader.right += dx;
        if (lpnmh->iItem + 1 < gxDPA_GetPtrCount(infoPtr->hdpaColumns))
          LISTVIEW_ScrollColumns(infoPtr, lpnmh->iItem + 1, dx);
        else
        {
          /* only needs to update the scrolls */
          infoPtr->nItemWidth += dx;
          LISTVIEW_UpdateScroll(infoPtr);
        }
        LISTVIEW_UpdateItemSize(infoPtr);
        if (uView == LVS_REPORT && is_redrawing(infoPtr))
        {
          GXPOINT ptOrigin;
          GXRECT rcCol = lpColumnInfo->rcHeader;

          LISTVIEW_GetOrigin(infoPtr, &ptOrigin);
          gxOffsetRect(&rcCol, ptOrigin.x, 0);

          rcCol.top = infoPtr->rcList.top;
          rcCol.bottom = infoPtr->rcList.bottom;

          /* resizing left-aligned columns leaves most of the left side untouched */
          if ((lpColumnInfo->fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
          {
            GXINT nMaxDirty = infoPtr->nEllipsisWidth + infoPtr->ntmMaxCharWidth;
            if (dx > 0)
              nMaxDirty += dx;
            rcCol.left = max (rcCol.left, rcCol.right - nMaxDirty);
          }

          /* when shrinking the last column clear the now unused field */
          if (lpnmh->iItem == gxDPA_GetPtrCount(infoPtr->hdpaColumns) - 1 && dx < 0)
            rcCol.right -= dx;

          LISTVIEW_InvalidateRect(infoPtr, &rcCol);
        }
      }
    }
    break;

  case HDN_ITEMCLICKW:
  case HDN_ITEMCLICKA:
    {
      /* Handle sorting by Header Column */
      GXNMLISTVIEW nmlv;

      ZeroMemory(&nmlv, sizeof(GXNMLISTVIEW));
      nmlv.iItem = -1;
      nmlv.iSubItem = lpnmh->iItem;
      notify_listview(infoPtr, LVN_COLUMNCLICK, &nmlv);
    }
    break;

  case HDN_DIVIDERDBLCLICKW:
  case HDN_DIVIDERDBLCLICKA:
    LISTVIEW_SetColumnWidth(infoPtr, lpnmh->iItem, LVSCW_AUTOSIZE);
    break;
  }

  return 0;
}

/***
* DESCRIPTION:
* Paint non-client area of control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structureof the sender
* [I] region : update region
*
* RETURN:
*  TRUE  - frame was painted
*  FALSE - call default window proc
*/
static GXBOOL LISTVIEW_NCPaint(const LISTVIEW_INFO *infoPtr, GXHRGN region)
{
  GXHTHEME theme = gxGetWindowTheme (infoPtr->hwndSelf);
  GXHDC dc;
  GXRECT r;
  GXHRGN cliprgn;
  int cxEdge = gxGetSystemMetrics (GXSM_CXEDGE),
    cyEdge = gxGetSystemMetrics (GXSM_CYEDGE);

  if (!theme) return FALSE;

  gxGetWindowRect(infoPtr->hwndSelf, &r);

  cliprgn = gxCreateRectRgn (r.left + cxEdge, r.top + cyEdge,
    r.right - cxEdge, r.bottom - cyEdge);
  if (region != (GXHRGN)1)
    gxCombineRgn (cliprgn, cliprgn, region, GXRGN_AND);
  gxOffsetRect(&r, -r.left, -r.top);

  dc = gxGetDCEx(infoPtr->hwndSelf, region, GXDCX_WINDOW|GXDCX_INTERSECTRGN);
  gxOffsetRect(&r, -r.left, -r.top);

  if (gxIsThemeBackgroundPartiallyTransparent (theme, 0, 0))
    gxDrawThemeParentBackground(infoPtr->hwndSelf, dc, &r);
  gxDrawThemeBackground (theme, dc, 0, 0, &r, 0);
  gxReleaseDC(infoPtr->hwndSelf, dc);

  /* Call default proc to get the scrollbars etc. painted */
  gxDefWindowProcW (infoPtr->hwndSelf, GXWM_NCPAINT, (GXWPARAM)cliprgn, 0);

  return TRUE;
}

/***
* DESCRIPTION:
* Determines the type of structure to use.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structureof the sender
* [I] hwndFrom : listview window handle
* [I] nCommand : command specifying the nature of the WM_NOTIFYFORMAT
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_NotifyFormat(LISTVIEW_INFO *infoPtr, GXHWND hwndFrom, GXINT nCommand)
{
  TRACE("(hwndFrom=%p, nCommand=%d)\n", hwndFrom, nCommand);

  if (nCommand == GXNF_REQUERY)
    infoPtr->notifyFormat = gxSendMessageW(hwndFrom, GXWM_NOTIFYFORMAT, (GXWPARAM)infoPtr->hwndSelf, GXNF_QUERY);

  return infoPtr->notifyFormat;
}

/***
* DESCRIPTION:
* Paints/Repaints the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_Paint(LISTVIEW_INFO *infoPtr, GXHDC hdc)
{
  TRACE("(hdc=%p)\n", hdc);

  if (infoPtr->bNoItemMetrics && infoPtr->nItemCount)
  {
    GXUINT uView =  infoPtr->dwStyle & LVS_TYPEMASK;

    infoPtr->bNoItemMetrics = FALSE;
    LISTVIEW_UpdateItemSize(infoPtr);
    if (uView == LVS_ICON || uView == LVS_SMALLICON)
      LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);
    LISTVIEW_UpdateScroll(infoPtr);
  }

  gxUpdateWindow(infoPtr->hwndHeader);

  if (hdc) 
    LISTVIEW_Refresh(infoPtr, hdc, NULL);
  else
  {
    GXPAINTSTRUCT ps;

    hdc = gxBeginPaint(infoPtr->hwndSelf, &ps);
    if (!hdc) return 1;
    LISTVIEW_Refresh(infoPtr, hdc, (const GXRECT*)(ps.fErase ? &ps.rcPaint : NULL));
    gxEndPaint(infoPtr->hwndSelf, &ps);
  }

  return 0;
}


/***
* DESCRIPTION:
* Paints/Repaints the listview control.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hdc : device context handle
* [I] options : drawing options
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_PrintClient(LISTVIEW_INFO *infoPtr, GXHDC hdc, GXDWORD options)
{
  FIXME("Partial Stub: (hdc=%p options=0x%08x)\n", hdc, options);

  if ((options & GXPRF_CHECKVISIBLE) && !gxIsWindowVisible(infoPtr->hwndSelf))
    return 0;

  if (options & GXPRF_ERASEBKGND)
    LISTVIEW_EraseBkgnd(infoPtr, hdc);

  if (options & GXPRF_CLIENT)
    LISTVIEW_Paint(infoPtr, hdc);

  return 0;
}


/***
* DESCRIPTION:
* Processes double click messages (right mouse button).
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wKey : key flag
* [I] x,y : mouse coordinate
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_RButtonDblClk(const LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO lvHitTestInfo;

  TRACE("(key=%hu,X=%hu,Y=%hu)\n", wKey, x, y);

  /* send NM_RELEASEDCAPTURE notification */
  if (!notify(infoPtr, GXNM_RELEASEDCAPTURE)) return 0;

  /* send NM_RDBLCLK notification */
  lvHitTestInfo.pt.x = x;
  lvHitTestInfo.pt.y = y;
  LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, FALSE);
  notify_click(infoPtr, GXNM_RDBLCLK, &lvHitTestInfo);

  return 0;
}

/***
* DESCRIPTION:
* Processes mouse down messages (right mouse button).
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wKey : key flag
* [I] x,y : mouse coordinate
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_RButtonDown(LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO lvHitTestInfo;
  GXINT nItem;

  TRACE("(key=%hu,X=%hu,Y=%hu)\n", wKey, x, y);

  /* send NM_RELEASEDCAPTURE notification */
  if (!notify(infoPtr, GXNM_RELEASEDCAPTURE)) return 0;

  /* make sure the listview control window has the focus */
  if (!infoPtr->bFocus) gxSetFocus(infoPtr->hwndSelf);

  /* set right button down flag */
  infoPtr->bRButtonDown = TRUE;

  /* determine the index of the selected item */
  lvHitTestInfo.pt.x = x;
  lvHitTestInfo.pt.y = y;
  nItem = LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, TRUE);

  if ((nItem >= 0) && (nItem < infoPtr->nItemCount))
  {
    LISTVIEW_SetItemFocus(infoPtr, nItem);
    if (!((wKey & GXMK_SHIFT) || (wKey & GXMK_CONTROL)) &&
      !LISTVIEW_GetItemState(infoPtr, nItem, LVIS_SELECTED))
      LISTVIEW_SetSelection(infoPtr, nItem);
  }
  else
  {
    LISTVIEW_DeselectAll(infoPtr);
  }

  return 0;
}

/***
* DESCRIPTION:
* Processes mouse up messages (right mouse button).
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wKey : key flag
* [I] x,y : mouse coordinate
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_RButtonUp(LISTVIEW_INFO *infoPtr, GXWORD wKey, GXINT x, GXINT y)
{
  GXLVHITTESTINFO lvHitTestInfo;
  GXPOINT pt;

  TRACE("(key=%hu,X=%hu,Y=%hu)\n", wKey, x, y);

  if (!infoPtr->bRButtonDown) return 0;

  /* set button flag */
  infoPtr->bRButtonDown = FALSE;

  /* Send NM_RClICK notification */
  lvHitTestInfo.pt.x = x;
  lvHitTestInfo.pt.y = y;
  LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, TRUE, FALSE);
  if (!notify_click(infoPtr, GXNM_RCLICK, &lvHitTestInfo)) return 0;

  /* Change to screen coordinate for WM_CONTEXTMENU */
  pt = lvHitTestInfo.pt;
  gxClientToScreen(infoPtr->hwndSelf, &pt);

  /* Send a WM_CONTEXTMENU message in response to the RBUTTONUP */
  gxSendMessageW(infoPtr->hwndSelf, GXWM_CONTEXTMENU,
    (GXWPARAM)infoPtr->hwndSelf, GXMAKELPARAM(pt.x, pt.y));

  return 0;
}


/***
* DESCRIPTION:
* Sets the cursor.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hwnd : window handle of window containing the cursor
* [I] nHittest : hit-test code
* [I] wMouseMsg : ideintifier of the mouse message
*
* RETURN:
* TRUE if cursor is set
* FALSE otherwise
*/
static GXBOOL LISTVIEW_SetCursor(const LISTVIEW_INFO *infoPtr, GXHWND hwnd, GXUINT nHittest, GXUINT wMouseMsg)
{
  GXLVHITTESTINFO lvHitTestInfo;

  if(!(infoPtr->dwLvExStyle & LVS_EX_TRACKSELECT)) return FALSE;

  if(!infoPtr->hHotCursor)  return FALSE;

  gxGetCursorPos(&lvHitTestInfo.pt);
  if (LISTVIEW_HitTest(infoPtr, &lvHitTestInfo, FALSE, FALSE) < 0) return FALSE;

  gxSetCursor(infoPtr->hHotCursor);

  return TRUE;
}

/***
* DESCRIPTION:
* Sets the focus.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] hwndLoseFocus : handle of previously focused window
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_SetFocus(LISTVIEW_INFO *infoPtr, GXHWND hwndLoseFocus)
{
  TRACE("(hwndLoseFocus=%p)\n", hwndLoseFocus);

  /* if we have the focus already, there's nothing to do */
  if (infoPtr->bFocus) return 0;

  /* send NM_SETFOCUS notification */
  if (!notify(infoPtr, GXNM_SETFOCUS)) return 0;

  /* set window focus flag */
  infoPtr->bFocus = TRUE;

  /* put the focus rect back on */
  LISTVIEW_ShowFocusRect(infoPtr, TRUE);

  /* redraw all visible selected items */
  LISTVIEW_InvalidateSelectedItems(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Sets the font.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] fRedraw : font handle
* [I] fRedraw : redraw flag
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_SetFont(LISTVIEW_INFO *infoPtr, GXHFONT hFont, GXWORD fRedraw)
{
  GXHFONT oldFont = infoPtr->hFont;

  TRACE("(hfont=%p,redraw=%hu)\n", hFont, fRedraw);

  infoPtr->hFont = hFont ? hFont : infoPtr->hDefaultFont;
  if (infoPtr->hFont == oldFont) return 0;

  LISTVIEW_SaveTextMetrics(infoPtr);

  if ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_REPORT)
  {
    gxSendMessageW(infoPtr->hwndHeader, GXWM_SETFONT, (GXWPARAM)hFont, GXMAKELPARAM(fRedraw, 0));
    LISTVIEW_UpdateSize(infoPtr);
    LISTVIEW_UpdateScroll(infoPtr);
  }

  if (fRedraw) LISTVIEW_InvalidateList(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Message handling for WM_SETREDRAW.
* For the Listview, it invalidates the entire window (the doc specifies otherwise)
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] bRedraw: state of redraw flag
*
* RETURN:
* DefWinProc return value
*/
static GXLRESULT LISTVIEW_SetRedraw(LISTVIEW_INFO *infoPtr, GXBOOL bRedraw)
{
  TRACE("infoPtr->bRedraw=%d, bRedraw=%d\n", infoPtr->bRedraw, bRedraw);

  /* we cannot use straight equality here because _any_ non-zero value is TRUE */
  if ((infoPtr->bRedraw && bRedraw) || (!infoPtr->bRedraw && !bRedraw)) return 0;

  infoPtr->bRedraw = bRedraw;

  if(!bRedraw) return 0;

  if (is_autoarrange(infoPtr))
    LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);
  LISTVIEW_UpdateScroll(infoPtr);

  /* despite what the WM_SETREDRAW docs says, apps expect us
  * to invalidate the listview here... stupid! */
  LISTVIEW_InvalidateList(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Resizes the listview control. This function processes WM_SIZE
* messages.  At this time, the width and height are not used.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] Width : new width
* [I] Height : new height
*
* RETURN:
* Zero
*/
static GXLRESULT LISTVIEW_Size(LISTVIEW_INFO *infoPtr, int Width, int Height)
{
  GXRECT rcOld = infoPtr->rcList;

  TRACE("(width=%d, height=%d)\n", Width, Height);

  LISTVIEW_UpdateSize(infoPtr);
  if (gxEqualRect(&rcOld, &infoPtr->rcList)) return 0;

  /* do not bother with display related stuff if we're not redrawing */ 
  if (!is_redrawing(infoPtr)) return 0;

  if (is_autoarrange(infoPtr)) 
    LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);

  LISTVIEW_UpdateScroll(infoPtr);

  /* refresh all only for lists whose height changed significantly */
  if ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_LIST && 
    (rcOld.bottom - rcOld.top) / infoPtr->nItemHeight !=
    (infoPtr->rcList.bottom - infoPtr->rcList.top) / infoPtr->nItemHeight)
    LISTVIEW_InvalidateList(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Sets the size information.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
*
* RETURN:
*  None
*/
static void LISTVIEW_UpdateSize(LISTVIEW_INFO *infoPtr)
{
  GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;

  TRACE("uView=%d, rcList(old)=%s\n", uView, wine_dbgstr_rect(&infoPtr->rcList));

  gxGetClientRect(infoPtr->hwndSelf, &infoPtr->rcList);

  if (uView == LVS_LIST)
  {
    /* Apparently the "LIST" style is supposed to have the same
    * number of items in a column even if there is no scroll bar.
    * Since if a scroll bar already exists then the bottom is already
    * reduced, only reduce if the scroll bar does not currently exist.
    * The "2" is there to mimic the native control. I think it may be
    * related to either padding or edges.  (GLA 7/2002)
    */
    if (!(gxGetWindowLongW(infoPtr->hwndSelf, GXGWL_STYLE) & GXWS_HSCROLL))
      infoPtr->rcList.bottom -= gxGetSystemMetrics(GXSM_CYHSCROLL);
    infoPtr->rcList.bottom = max (infoPtr->rcList.bottom - 2, 0);
  }
  else if (uView == LVS_REPORT)
  {
    GXHDLAYOUT hl;
    GXWINDOWPOS wp;

    hl.prc = &infoPtr->rcList;
    hl.pwpos = &wp;
    gxSendMessageW( infoPtr->hwndHeader, GXHDM_LAYOUT, 0, (GXLPARAM)&hl );
    TRACE("  wp.flags=0x%08x, wp=%d,%d (%dx%d)\n", wp.flags, wp.x, wp.y, wp.cx, wp.cy);
    gxSetWindowPos(wp.hwnd, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy,
      wp.flags | ((infoPtr->dwStyle & LVS_NOCOLUMNHEADER)
      ? GXSWP_HIDEWINDOW : GXSWP_SHOWWINDOW));
    TRACE("  after SWP wp=%d,%d (%dx%d)\n", wp.x, wp.y, wp.cx, wp.cy);

    infoPtr->rcList.top = max(wp.cy, 0);
    infoPtr->rcList.top += (infoPtr->dwLvExStyle & LVS_EX_GRIDLINES) ? 2 : 0;
  }

  TRACE("  rcList=%s\n", wine_dbgstr_rect(&infoPtr->rcList));
}

/***
* DESCRIPTION:
* Processes WM_STYLECHANGED messages.
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wStyleType : window style type (normal or extended)
* [I] lpss : window style information
*
* RETURN:
* Zero
*/
static GXINT LISTVIEW_StyleChanged(LISTVIEW_INFO *infoPtr, GXWPARAM wStyleType,
                   const GXSTYLESTRUCT *lpss)
{
  GXUINT uNewView = lpss->styleNew & LVS_TYPEMASK;
  GXUINT uOldView = lpss->styleOld & LVS_TYPEMASK;
  GXUINT style;

  TRACE("(styletype=%lx, styleOld=0x%08x, styleNew=0x%08x)\n",
    wStyleType, lpss->styleOld, lpss->styleNew);

  if (wStyleType != GXGWL_STYLE) return 0;

  /* FIXME: if LVS_NOSORTHEADER changed, update header */
  /*        what if LVS_OWNERDATA changed? */
  /*        or LVS_SINGLESEL */
  /*        or LVS_SORT{AS,DES}CENDING */

  infoPtr->dwStyle = lpss->styleNew;

  if (((lpss->styleOld & GXWS_HSCROLL) != 0)&&
    ((lpss->styleNew & GXWS_HSCROLL) == 0))
    gxShowScrollBar(infoPtr->hwndSelf, GXSB_HORZ, FALSE);

  if (((lpss->styleOld & GXWS_VSCROLL) != 0)&&
    ((lpss->styleNew & GXWS_VSCROLL) == 0))
    gxShowScrollBar(infoPtr->hwndSelf, GXSB_VERT, FALSE);

  if (uNewView != uOldView)
  {
    GXSIZE oldIconSize = infoPtr->iconSize;
    GXHIMAGELIST himl;

    gxSendMessageW(infoPtr->hwndEdit, GXWM_KILLFOCUS, 0, 0);
    gxShowWindow(infoPtr->hwndHeader, GXSW_HIDE);

    gxShowScrollBar(infoPtr->hwndSelf, GXSB_BOTH, FALSE);
    gxSetRectEmpty(&infoPtr->rcFocus);

    himl = (uNewView == LVS_ICON ? infoPtr->himlNormal : infoPtr->himlSmall);
    set_icon_size(&infoPtr->iconSize, himl, uNewView != LVS_ICON);

    if (uNewView == LVS_ICON)
    {
      if ((infoPtr->iconSize.cx != oldIconSize.cx) || (infoPtr->iconSize.cy != oldIconSize.cy))
      {
        TRACE("icon old size=(%d,%d), new size=(%d,%d)\n",
          oldIconSize.cx, oldIconSize.cy, infoPtr->iconSize.cx, infoPtr->iconSize.cy);
        LISTVIEW_SetIconSpacing(infoPtr, 0, 0);
      }
    }
    else if (uNewView == LVS_REPORT)
    {
      GXHDLAYOUT hl;
      GXWINDOWPOS wp;

      hl.prc = &infoPtr->rcList;
      hl.pwpos = &wp;
      gxSendMessageW( infoPtr->hwndHeader, GXHDM_LAYOUT, 0, (GXLPARAM)&hl );
      gxSetWindowPos(infoPtr->hwndHeader, infoPtr->hwndSelf, wp.x, wp.y, wp.cx, wp.cy,
        wp.flags | ((infoPtr->dwStyle & LVS_NOCOLUMNHEADER)
        ? GXSWP_HIDEWINDOW : GXSWP_SHOWWINDOW));
    }

    LISTVIEW_UpdateItemSize(infoPtr);
  }

  if (uNewView == LVS_REPORT)
  {
    if ((lpss->styleOld ^ lpss->styleNew) & LVS_NOCOLUMNHEADER)
    {
      if (lpss->styleNew & LVS_NOCOLUMNHEADER)
      {
        /* Turn off the header control */
        style = gxGetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE);
        TRACE("Hide header control, was 0x%08x\n", style);
        gxSetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE, style | HDS_HIDDEN);
      } else {
        /* Turn on the header control */
        if ((style = gxGetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE)) & HDS_HIDDEN)
        {
          TRACE("Show header control, was 0x%08x\n", style);
          gxSetWindowLongW(infoPtr->hwndHeader, GXGWL_STYLE, (style & ~HDS_HIDDEN) | GXWS_VISIBLE);
        }
      }
    }
  }

  if ( (uNewView == LVS_ICON || uNewView == LVS_SMALLICON) &&
    (uNewView != uOldView || ((lpss->styleNew ^ lpss->styleOld) & LVS_ALIGNMASK)) )
    LISTVIEW_Arrange(infoPtr, LVA_DEFAULT);

  /* update the size of the client area */
  LISTVIEW_UpdateSize(infoPtr);

  /* add scrollbars if needed */
  LISTVIEW_UpdateScroll(infoPtr);

  /* invalidate client area + erase background */
  LISTVIEW_InvalidateList(infoPtr);

  return 0;
}

/***
* DESCRIPTION:
* Window procedure of the listview control.
*
*/
static GXLRESULT GXCALLBACK
LISTVIEW_WindowProc(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  LISTVIEW_INFO *infoPtr = (LISTVIEW_INFO *)gxGetWindowLongPtrW(hwnd, 0);

  TRACE("(uMsg=%x wParam=%lx lParam=%lx)\n", uMsg, wParam, lParam);

  if (!infoPtr && (uMsg != GXWM_NCCREATE))
    return gxDefWindowProcW(hwnd, uMsg, wParam, lParam);

  switch (uMsg)
  {
  case GXLVM_APPROXIMATEVIEWRECT:
    return LISTVIEW_ApproximateViewRect(infoPtr, (GXINT)wParam,
      GXLOWORD(lParam), GXHIWORD(lParam));
  case GXLVM_ARRANGE:
    return LISTVIEW_Arrange(infoPtr, (GXINT)wParam);

    /* case GXLVM_CANCELEDITLABEL: */

  case GXLVM_CREATEDRAGIMAGE:
    return (GXLRESULT)LISTVIEW_CreateDragImage(infoPtr, (GXINT)wParam, (LPGXPOINT)lParam);

  case GXLVM_DELETEALLITEMS:
    return LISTVIEW_DeleteAllItems(infoPtr, FALSE);

  case GXLVM_DELETECOLUMN:
    return LISTVIEW_DeleteColumn(infoPtr, (GXINT)wParam);

  case GXLVM_DELETEITEM:
    return LISTVIEW_DeleteItem(infoPtr, (GXINT)wParam);

  case GXLVM_EDITLABELW:
    return (GXLRESULT)LISTVIEW_EditLabelT(infoPtr, (GXINT)wParam, TRUE);

  case GXLVM_EDITLABELA:
    return (GXLRESULT)LISTVIEW_EditLabelT(infoPtr, (GXINT)wParam, FALSE);

    /* case GXLVM_ENABLEGROUPVIEW: */

  case GXLVM_ENSUREVISIBLE:
    return LISTVIEW_EnsureVisible(infoPtr, (GXINT)wParam, (GXBOOL)lParam);

  case GXLVM_FINDITEMW:
    return LISTVIEW_FindItemW(infoPtr, (GXINT)wParam, (GXLVFINDINFOW*)lParam);

    //case GXLVM_FINDITEMA:
    //  return LISTVIEW_FindItemA(infoPtr, (GXINT)wParam, (LPLVFINDINFOA)lParam);

  case GXLVM_GETBKCOLOR:
    return infoPtr->clrBk;

    /* case GXLVM_GETBKIMAGE: */

  case GXLVM_GETCALLBACKMASK:
    return infoPtr->uCallbackMask;

    //case GXLVM_GETCOLUMNA:
    //  return LISTVIEW_GetColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, FALSE);

  case GXLVM_GETCOLUMNW:
    return LISTVIEW_GetColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, TRUE);

  case GXLVM_GETCOLUMNORDERARRAY:
    return LISTVIEW_GetColumnOrderArray(infoPtr, (GXINT)wParam, (GXLPINT)lParam);

  case GXLVM_GETCOLUMNWIDTH:
    return LISTVIEW_GetColumnWidth(infoPtr, (GXINT)wParam);

  case GXLVM_GETCOUNTPERPAGE:
    return LISTVIEW_GetCountPerPage(infoPtr);

  case GXLVM_GETEDITCONTROL:
    return (GXLRESULT)infoPtr->hwndEdit;

  case GXLVM_GETEXTENDEDLISTVIEWSTYLE:
    return infoPtr->dwLvExStyle;

    /* case GXLVM_GETGROUPINFO: */

    /* case GXLVM_GETGROUPMETRICS: */

  case GXLVM_GETHEADER:
    return (GXLRESULT)infoPtr->hwndHeader;

  case GXLVM_GETHOTCURSOR:
    return (GXLRESULT)infoPtr->hHotCursor;

  case GXLVM_GETHOTITEM:
    return infoPtr->nHotItem;

  case GXLVM_GETHOVERTIME:
    return infoPtr->dwHoverTime;

  case GXLVM_GETIMAGELIST:
    return (GXLRESULT)LISTVIEW_GetImageList(infoPtr, (GXINT)wParam);

    /* case GXLVM_GETINSERTMARK: */

    /* case GXLVM_GETINSERTMARKCOLOR: */

    /* case GXLVM_GETINSERTMARKRECT: */

  case GXLVM_GETISEARCHSTRINGA:
  case GXLVM_GETISEARCHSTRINGW:
    FIXME("GXLVM_GETISEARCHSTRING: unimplemented\n");
    return FALSE;

    //case GXLVM_GETITEMA:
    //  return LISTVIEW_GetItemExtT(infoPtr, (GXLPLVITEMW)lParam, FALSE);

  case GXLVM_GETITEMW:
    return LISTVIEW_GetItemExtT(infoPtr, (GXLPLVITEMW)lParam, TRUE);

  case GXLVM_GETITEMCOUNT:
    return infoPtr->nItemCount;

  case GXLVM_GETITEMPOSITION:
    return LISTVIEW_GetItemPosition(infoPtr, (GXINT)wParam, (LPGXPOINT)lParam);

  case GXLVM_GETITEMRECT:
    return LISTVIEW_GetItemRect(infoPtr, (GXINT)wParam, (LPGXRECT)lParam);

  case GXLVM_GETITEMSPACING:
    return LISTVIEW_GetItemSpacing(infoPtr, (GXBOOL)wParam);

  case GXLVM_GETITEMSTATE:
    return LISTVIEW_GetItemState(infoPtr, (GXINT)wParam, (GXUINT)lParam);

  case GXLVM_GETITEMTEXTA:
    return LISTVIEW_GetItemTextT(infoPtr, (GXINT)wParam, (GXLPLVITEMW)lParam, FALSE);

  case GXLVM_GETITEMTEXTW:
    return LISTVIEW_GetItemTextT(infoPtr, (GXINT)wParam, (GXLPLVITEMW)lParam, TRUE);

  case GXLVM_GETNEXTITEM:
    return LISTVIEW_GetNextItem(infoPtr, (GXINT)wParam, GXLOWORD(lParam));

  case GXLVM_GETNUMBEROFWORKAREAS:
    FIXME("GXLVM_GETNUMBEROFWORKAREAS: unimplemented\n");
    return 1;

  case GXLVM_GETORIGIN:
    if (!lParam) return FALSE;
    if ((infoPtr->dwStyle & LVS_TYPEMASK) == LVS_REPORT ||
      (infoPtr->dwStyle & LVS_TYPEMASK) == LVS_LIST) return FALSE;
    LISTVIEW_GetOrigin(infoPtr, (LPGXPOINT)lParam);
    return TRUE;

    /* case GXLVM_GETOUTLINECOLOR: */

    /* case GXLVM_GETSELECTEDCOLUMN: */

  case GXLVM_GETSELECTEDCOUNT:
    return LISTVIEW_GetSelectedCount(infoPtr);

  case GXLVM_GETSELECTIONMARK:
    return infoPtr->nSelectionMark;

  case GXLVM_GETSTRINGWIDTHA:
    return LISTVIEW_GetStringWidthT(infoPtr, (GXLPCWSTR)lParam, FALSE);

  case GXLVM_GETSTRINGWIDTHW:
    return LISTVIEW_GetStringWidthT(infoPtr, (GXLPCWSTR)lParam, TRUE);

  case GXLVM_GETSUBITEMRECT:
    return LISTVIEW_GetSubItemRect(infoPtr, (GXUINT)wParam, (LPGXRECT)lParam);

  case GXLVM_GETTEXTBKCOLOR:
    return infoPtr->clrTextBk;

  case GXLVM_GETTEXTCOLOR:
    return infoPtr->clrText;

    /* case GXLVM_GETTILEINFO: */

    /* case GXLVM_GETTILEVIEWINFO: */

  case GXLVM_GETTOOLTIPS:
    if( !infoPtr->hwndToolTip )
      infoPtr->hwndToolTip = gxCOMCTL32_CreateToolTip( hwnd );
    return (GXLRESULT)infoPtr->hwndToolTip;

  case GXLVM_GETTOPINDEX:
    return LISTVIEW_GetTopIndex(infoPtr);

  case GXLVM_GETUNICODEFORMAT:
    return (infoPtr->notifyFormat == GXNFR_UNICODE);

    /* case GXLVM_GETVIEW: */

  case GXLVM_GETVIEWRECT:
    return LISTVIEW_GetViewRect(infoPtr, (LPGXRECT)lParam);

  case GXLVM_GETWORKAREAS:
    FIXME("GXLVM_GETWORKAREAS: unimplemented\n");
    return FALSE;

    /* case GXLVM_HASGROUP: */

  case GXLVM_HITTEST:
    return LISTVIEW_HitTest(infoPtr, (GXLPLVHITTESTINFO)lParam, FALSE, FALSE);

  case GXLVM_INSERTCOLUMNA:
    return LISTVIEW_InsertColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, FALSE);

  case GXLVM_INSERTCOLUMNW:
    return LISTVIEW_InsertColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, TRUE);

    /* case GXLVM_INSERTGROUP: */

    /* case GXLVM_INSERTGROUPSORTED: */

  case GXLVM_INSERTITEMA:
    return LISTVIEW_InsertItemT(infoPtr, (GXLPLVITEMW)lParam, FALSE);

  case GXLVM_INSERTITEMW:
    return LISTVIEW_InsertItemT(infoPtr, (GXLPLVITEMW)lParam, TRUE);

    /* case GXLVM_INSERTMARKHITTEST: */

    /* case GXLVM_ISGROUPVIEWENABLED: */

    /* case GXLVM_MAPIDTOINDEX: */

    /* case GXLVM_MAPINDEXTOID: */

    /* case GXLVM_MOVEGROUP: */

    /* case GXLVM_MOVEITEMTOGROUP: */

  case GXLVM_REDRAWITEMS:
    return LISTVIEW_RedrawItems(infoPtr, (GXINT)wParam, (GXINT)lParam);

    /* case GXLVM_REMOVEALLGROUPS: */

    /* case GXLVM_REMOVEGROUP: */

  case GXLVM_SCROLL:
    return LISTVIEW_Scroll(infoPtr, (GXINT)wParam, (GXINT)lParam);

  case GXLVM_SETBKCOLOR:
    return LISTVIEW_SetBkColor(infoPtr, (GXCOLORREF)lParam);

    /* case GXLVM_SETBKIMAGE: */

  case GXLVM_SETCALLBACKMASK:
    infoPtr->uCallbackMask = (GXUINT)wParam;
    return TRUE;

  case GXLVM_SETCOLUMNA:
    return LISTVIEW_SetColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, FALSE);

  case GXLVM_SETCOLUMNW:
    return LISTVIEW_SetColumnT(infoPtr, (GXINT)wParam, (GXLPLVCOLUMNW)lParam, TRUE);

  case GXLVM_SETCOLUMNORDERARRAY:
    return LISTVIEW_SetColumnOrderArray(infoPtr, (GXINT)wParam, (GXLPINT)lParam);

  case GXLVM_SETCOLUMNWIDTH:
    return LISTVIEW_SetColumnWidth(infoPtr, (GXINT)wParam, (short)GXLOWORD(lParam));

  case GXLVM_SETEXTENDEDLISTVIEWSTYLE:
    return LISTVIEW_SetExtendedListViewStyle(infoPtr, (GXDWORD)wParam, (GXDWORD)lParam);

    /* case GXLVM_SETGROUPINFO: */

    /* case GXLVM_SETGROUPMETRICS: */

  case GXLVM_SETHOTCURSOR:
    return (GXLRESULT)LISTVIEW_SetHotCursor(infoPtr, (GXHCURSOR)lParam);

  case GXLVM_SETHOTITEM:
    return LISTVIEW_SetHotItem(infoPtr, (GXINT)wParam);

  case GXLVM_SETHOVERTIME:
    return LISTVIEW_SetHoverTime(infoPtr, (GXDWORD)wParam);

  case GXLVM_SETICONSPACING:
    return LISTVIEW_SetIconSpacing(infoPtr, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));

  case GXLVM_SETIMAGELIST:
    return (GXLRESULT)LISTVIEW_SetImageList(infoPtr, (GXINT)wParam, (GXHIMAGELIST)lParam);

    /* case GXLVM_SETINFOTIP: */

    /* case GXLVM_SETINSERTMARK: */

    /* case GXLVM_SETINSERTMARKCOLOR: */

  case GXLVM_SETITEMA:
    return LISTVIEW_SetItemT(infoPtr, (GXLPLVITEMW)lParam, FALSE);

  case GXLVM_SETITEMW:
    return LISTVIEW_SetItemT(infoPtr, (GXLPLVITEMW)lParam, TRUE);

  case GXLVM_SETITEMCOUNT:
    return LISTVIEW_SetItemCount(infoPtr, (GXINT)wParam, (GXDWORD)lParam);

  case GXLVM_SETITEMPOSITION:
    {
      GXPOINT pt;
      pt.x = (short)GXLOWORD(lParam);
      pt.y = (short)GXHIWORD(lParam);
      return LISTVIEW_SetItemPosition(infoPtr, (GXINT)wParam, pt);
    }

  case GXLVM_SETITEMPOSITION32:
    if (lParam == 0) return FALSE;
    return LISTVIEW_SetItemPosition(infoPtr, (GXINT)wParam, *((GXPOINT*)lParam));

  case GXLVM_SETITEMSTATE:
    return LISTVIEW_SetItemState(infoPtr, (GXINT)wParam, (GXLPLVITEMW)lParam);

  case GXLVM_SETITEMTEXTA:
    return LISTVIEW_SetItemTextT(infoPtr, (GXINT)wParam, (GXLPLVITEMW)lParam, FALSE);

  case GXLVM_SETITEMTEXTW:
    return LISTVIEW_SetItemTextT(infoPtr, (GXINT)wParam, (GXLPLVITEMW)lParam, TRUE);

    /* case GXLVM_SETOUTLINECOLOR: */

    /* case GXLVM_SETSELECTEDCOLUMN: */

  case GXLVM_SETSELECTIONMARK:
    return LISTVIEW_SetSelectionMark(infoPtr, (GXINT)lParam);

  case GXLVM_SETTEXTBKCOLOR:
    return LISTVIEW_SetTextBkColor(infoPtr, (GXCOLORREF)lParam);

  case GXLVM_SETTEXTCOLOR:
    return LISTVIEW_SetTextColor(infoPtr, (GXCOLORREF)lParam);

    /* case GXLVM_SETTILEINFO: */

    /* case GXLVM_SETTILEVIEWINFO: */

    /* case GXLVM_SETTILEWIDTH: */

  case GXLVM_SETTOOLTIPS:
    return (GXLRESULT)LISTVIEW_SetToolTips(infoPtr, (GXHWND)lParam);

  case GXLVM_SETUNICODEFORMAT:
    return LISTVIEW_SetUnicodeFormat(infoPtr, wParam);

    /* case GXLVM_SETVIEW: */

    /* case GXLVM_SETWORKAREAS: */

    /* case GXLVM_SORTGROUPS: */

  case GXLVM_SORTITEMS:
    return LISTVIEW_SortItems(infoPtr, (GXPFNLVCOMPARE)lParam, (GXLPARAM)wParam);

    /* GXLVM_SORTITEMSEX: */

  case GXLVM_SUBITEMHITTEST:
    return LISTVIEW_HitTest(infoPtr, (GXLPLVHITTESTINFO)lParam, TRUE, FALSE);

  case GXLVM_UPDATE:
    return LISTVIEW_Update(infoPtr, (GXINT)wParam);

  case GXWM_CHAR:
    return LISTVIEW_ProcessLetterKeys( infoPtr, wParam, lParam );

  case GXWM_COMMAND:
    return LISTVIEW_Command(infoPtr, wParam, lParam);

  case GXWM_NCCREATE:
    return LISTVIEW_NCCreate(hwnd, (GXLPCREATESTRUCTW)lParam);

  case GXWM_CREATE:
    return LISTVIEW_Create(hwnd, (GXLPCREATESTRUCTW)lParam);

  case GXWM_DESTROY:
    return LISTVIEW_Destroy(infoPtr);

  case GXWM_ENABLE:
    return LISTVIEW_Enable(infoPtr, (GXBOOL)wParam);

  case GXWM_ERASEBKGND:
    return LISTVIEW_EraseBkgnd(infoPtr, (GXHDC)wParam);

  case GXWM_GETDLGCODE:
    return GXDLGC_WANTCHARS | GXDLGC_WANTARROWS;

  case GXWM_GETFONT:
    return (GXLRESULT)infoPtr->hFont;

  case GXWM_HSCROLL:
    return LISTVIEW_HScroll(infoPtr, (GXINT)GXLOWORD(wParam), 0, (GXHWND)lParam);

  case GXWM_KEYDOWN:
    return LISTVIEW_KeyDown(infoPtr, (GXINT)wParam, (GXLONG)lParam);

  case GXWM_KILLFOCUS:
    return LISTVIEW_KillFocus(infoPtr);

  case GXWM_LBUTTONDBLCLK:
    return LISTVIEW_LButtonDblClk(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_LBUTTONDOWN:
    return LISTVIEW_LButtonDown(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_LBUTTONUP:
    return LISTVIEW_LButtonUp(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_MOUSEMOVE:
    return LISTVIEW_MouseMove (infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_MOUSEHOVER:
    return LISTVIEW_MouseHover(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_NCDESTROY:
    return LISTVIEW_NCDestroy(infoPtr);

  case GXWM_NCPAINT:
    if (LISTVIEW_NCPaint(infoPtr, (GXHRGN)wParam))
      return 0;
    goto fwd_msg;

  case GXWM_NOTIFY:
    if (lParam && ((GXLPNMHDR)lParam)->hwndFrom == infoPtr->hwndHeader)
      return LISTVIEW_HeaderNotification(infoPtr, (LPGXNMHEADERW)lParam);
    else return 0;

  case GXWM_NOTIFYFORMAT:
    return LISTVIEW_NotifyFormat(infoPtr, (GXHWND)wParam, (GXINT)lParam);

  case GXWM_PRINTCLIENT:
    return LISTVIEW_PrintClient(infoPtr, (GXHDC)wParam, (GXDWORD)lParam);

  case GXWM_PAINT:
    return LISTVIEW_Paint(infoPtr, (GXHDC)wParam);

  case GXWM_RBUTTONDBLCLK:
    return LISTVIEW_RButtonDblClk(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_RBUTTONDOWN:
    return LISTVIEW_RButtonDown(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_RBUTTONUP:
    return LISTVIEW_RButtonUp(infoPtr, (GXWORD)wParam, (GXSHORT)GXLOWORD(lParam), (GXSHORT)GXHIWORD(lParam));

  case GXWM_SETCURSOR:
    if(LISTVIEW_SetCursor(infoPtr, (GXHWND)wParam, GXLOWORD(lParam), GXHIWORD(lParam)))
      return TRUE;
    goto fwd_msg;

  case GXWM_SETFOCUS:
    return LISTVIEW_SetFocus(infoPtr, (GXHWND)wParam);

  case GXWM_SETFONT:
    return LISTVIEW_SetFont(infoPtr, (GXHFONT)wParam, (GXWORD)lParam);

  case GXWM_SETREDRAW:
    return LISTVIEW_SetRedraw(infoPtr, (GXBOOL)wParam);

  case GXWM_SIZE:
    return LISTVIEW_Size(infoPtr, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));

  case GXWM_STYLECHANGED:
    return LISTVIEW_StyleChanged(infoPtr, wParam, (GXLPSTYLESTRUCT)lParam);

  case GXWM_SYSCOLORCHANGE:
    gxCOMCTL32_RefreshSysColors();
    return 0;

    /*  case GXWM_TIMER: */
  case GXWM_THEMECHANGED:
    return LISTVIEW_ThemeChanged(infoPtr);

  case GXWM_VSCROLL:
    return LISTVIEW_VScroll(infoPtr, (GXINT)GXLOWORD(wParam), 0, (GXHWND)lParam);

  case GXWM_MOUSEWHEEL:
    if (wParam & (GXMK_SHIFT | GXMK_CONTROL))
      return gxDefWindowProcW(hwnd, uMsg, wParam, lParam);
    return LISTVIEW_MouseWheel(infoPtr, (short int)GXHIWORD(wParam));

  case GXWM_WINDOWPOSCHANGED:
    if (!(((GXWINDOWPOS *)lParam)->flags & GXSWP_NOSIZE)) 
    {
      GXUINT uView = infoPtr->dwStyle & LVS_TYPEMASK;
      gxSetWindowPos(infoPtr->hwndSelf, 0, 0, 0, 0, 0, GXSWP_FRAMECHANGED | GXSWP_NOACTIVATE |
        GXSWP_NOZORDER | GXSWP_NOMOVE | GXSWP_NOSIZE);

      if ((infoPtr->dwStyle & LVS_OWNERDRAWFIXED) && (uView == LVS_REPORT))
      {
        GXMEASUREITEMSTRUCT mis;
        mis.CtlType = GXODT_LISTVIEW;
        mis.CtlID = gxGetWindowLongPtrW(infoPtr->hwndSelf, GXGWLP_ID);
        mis.itemID = -1;
        mis.itemWidth = 0;
        mis.itemData = 0;
        mis.itemHeight= infoPtr->nItemHeight;
        gxSendMessageW(infoPtr->hwndNotify, GXWM_MEASUREITEM, mis.CtlID, (GXLPARAM)&mis);
        if (infoPtr->nItemHeight != max(mis.itemHeight, 1))
          infoPtr->nMeasureItemHeight = infoPtr->nItemHeight = max(mis.itemHeight, 1);
      }

      LISTVIEW_UpdateSize(infoPtr);
      LISTVIEW_UpdateScroll(infoPtr);
    }
    return gxDefWindowProcW(hwnd, uMsg, wParam, lParam);

    /*  case GXWM_WININICHANGE: */

  default:
    if ((uMsg >= GXWM_USER) && (uMsg < GXWM_APP) && !gxCOMCTL32_IsReflectedMessage(uMsg)) {
      ERR("unknown msg %04x wp=%08lx lp=%08lx\n", uMsg, wParam, lParam);
    }

fwd_msg:
    /* call default window procedure */
    return gxDefWindowProcW(hwnd, uMsg, wParam, lParam);
  }

}

/***
* DESCRIPTION:
* Registers the window class.
*
* PARAMETER(S):
* None
*
* RETURN:
* None
*/
void LISTVIEW_Register()
{
  GXWNDCLASSEX wndClass;

  ZeroMemory(&wndClass, sizeof(GXWNDCLASSEX));
  wndClass.cbSize = sizeof(GXWNDCLASSEX);
  wndClass.style = GXCS_GLOBALCLASS | GXCS_DBLCLKS;
  wndClass.lpfnWndProc = LISTVIEW_WindowProc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = sizeof(LISTVIEW_INFO *);
  wndClass.hCursor = gxLoadCursorW(0, (GXLPWSTR)GXIDC_ARROW);
  wndClass.hbrBackground = (GXHBRUSH)(GXCOLOR_WINDOW + 1);
  wndClass.lpszClassName = WC_LISTVIEWW;
  gxRegisterClassExW(&wndClass);
}

/***
* DESCRIPTION:
* Unregisters the window class.
*
* PARAMETER(S):
* None
*
* RETURN:
* None
*/
void LISTVIEW_Unregister()
{
  gxUnregisterClassW(WC_LISTVIEWW, NULL);
}

/***
* DESCRIPTION:
* Handle any WM_COMMAND messages
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] wParam : the first message parameter
* [I] lParam : the second message parameter
*
* RETURN:
*   Zero.
*/
static GXLRESULT LISTVIEW_Command(const LISTVIEW_INFO *infoPtr, GXWPARAM wParam, GXLPARAM lParam)
{
  switch (GXHIWORD(wParam))
  {
  case GXEN_UPDATE:
    {
      /*
      * Adjust the edit window size
      */
      GXWCHAR buffer[1024];
      GXHDC           hdc = gxGetDC(infoPtr->hwndEdit);
      GXHFONT         hFont, hOldFont = 0;
      GXRECT    rect;
      GXSIZE    sz;

      if (!infoPtr->hwndEdit || !hdc) return 0;
      gxGetWindowTextW(infoPtr->hwndEdit, buffer, sizeof(buffer)/sizeof(buffer[0]));
      gxGetWindowRect(infoPtr->hwndEdit, &rect);

      /* Select font to get the right dimension of the string */
      hFont = (GXHFONT)gxSendMessageW(infoPtr->hwndEdit, GXWM_GETFONT, 0, 0);
      if(hFont != 0)
      {
        hOldFont = (GXHFONT)gxSelectObject(hdc, hFont);
      }

      if (gxGetTextExtentPoint32W(hdc, buffer, GXSTRLEN(buffer), &sz))
      {
        GXTEXTMETRICW textMetric;

        /* Add Extra spacing for the next character */
        gxGetTextMetricsW(hdc, &textMetric);
        sz.cx += (textMetric.tmMaxCharWidth * 2);

        gxSetWindowPos (
          infoPtr->hwndEdit,
          GXHWND_TOP,
          0,
          0,
          sz.cx,
          rect.bottom - rect.top,
          GXSWP_DRAWFRAME|GXSWP_NOMOVE);
      }
      if(hFont != 0)
        gxSelectObject(hdc, hOldFont);

      gxReleaseDC(infoPtr->hwndEdit, hdc);

      break;
    }

  default:
    return gxSendMessageW (infoPtr->hwndNotify, GXWM_COMMAND, wParam, lParam);
  }

  return 0;
}


/***
* DESCRIPTION:
* Subclassed edit control windproc function
*
* PARAMETER(S):
* [I] hwnd : the edit window handle
* [I] uMsg : the message that is to be processed
* [I] wParam : first message parameter
* [I] lParam : second message parameter
* [I] isW : TRUE if input is Unicode
*
* RETURN:
*   Zero.
*/
static GXLRESULT EditLblWndProcT(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam, GXBOOL isW)
{
  LISTVIEW_INFO *infoPtr = (LISTVIEW_INFO *)gxGetWindowLongPtrW(gxGetParent(hwnd), 0);
  GXBOOL cancel = FALSE;

  //Artint
  ASSERT(isW == TRUE);

  TRACE("(hwnd=%p, uMsg=%x, wParam=%lx, lParam=%lx, isW=%d)\n", hwnd, uMsg, wParam, lParam, isW);

  switch (uMsg)
  {
  case GXWM_GETDLGCODE:
    return GXDLGC_WANTARROWS | GXDLGC_WANTALLKEYS;

  case GXWM_KILLFOCUS:
    break;

  case GXWM_DESTROY:
    {
      GXWNDPROC editProc = infoPtr->EditWndProc;
      infoPtr->EditWndProc = 0;
      gxSetWindowLongPtrW(hwnd, GXGWLP_WNDPROC, (GXDWORD_PTR)editProc);
      return CallWindowProcT(editProc, hwnd, uMsg, wParam, lParam, isW);
    }

  case GXWM_KEYDOWN:
    if (GXVK_ESCAPE == (GXINT)wParam)
    {
      cancel = TRUE;
      break;
    }
    else if (GXVK_RETURN == (GXINT)wParam)
      break;

  default:
    return CallWindowProcT(infoPtr->EditWndProc, hwnd, uMsg, wParam, lParam, isW);
  }

  /* kill the edit */
  if (infoPtr->hwndEdit)
  {
    GXLPWSTR buffer = NULL;

    infoPtr->hwndEdit = 0;
    if (!cancel)
    {
      GXDWORD len = isW ? gxGetWindowTextLengthW(hwnd) : gxGetWindowTextLengthA(hwnd);

      if (len)
      {
        if ( (buffer = (GXLPWSTR)Alloc((len+1) * (isW ? sizeof(GXWCHAR) : sizeof(GXCHAR)))) )
        {
          if (isW) gxGetWindowTextW(hwnd, buffer, len+1);
          else gxGetWindowTextA(hwnd, (GXCHAR*)buffer, len+1);
        }
      }
    }
    LISTVIEW_EndEditLabelT(infoPtr, buffer, isW);

    Free(buffer);
  }

  gxSendMessageW(hwnd, GXWM_CLOSE, 0, 0);
  return 0;
}

/***
* DESCRIPTION:
* Subclassed edit control Unicode windproc function
*
* PARAMETER(S):
* [I] hwnd : the edit window handle
* [I] uMsg : the message that is to be processed
* [I] wParam : first message parameter
* [I] lParam : second message parameter
*
* RETURN:
*/
static GXLRESULT GXCALLBACK EditLblWndProcW(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  return EditLblWndProcT(hwnd, uMsg, wParam, lParam, TRUE);
}

/***
* DESCRIPTION:
* Subclassed edit control ANSI windproc function
*
* PARAMETER(S):
* [I] hwnd : the edit window handle
* [I] uMsg : the message that is to be processed
* [I] wParam : first message parameter
* [I] lParam : second message parameter
*
* RETURN:
*/
static GXLRESULT GXCALLBACK EditLblWndProcA(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  return EditLblWndProcT(hwnd, uMsg, wParam, lParam, FALSE);
}

/***
* DESCRIPTION:
* Creates a subclassed edit control
*
* PARAMETER(S):
* [I] infoPtr : valid pointer to the listview structure
* [I] text : initial text for the edit
* [I] style : the window style
* [I] isW : TRUE if input is Unicode
*
* RETURN:
*/
static GXHWND CreateEditLabelT(LISTVIEW_INFO *infoPtr, GXLPCWSTR text, GXDWORD style,
                 GXINT x, GXINT y, GXINT width, GXINT height, GXBOOL isW)
{
  GXWCHAR editName[5] = { 'E', 'd', 'i', 't', '\0' };
  GXHWND hedit;
  GXSIZE sz;
  GXHDC hdc;
  GXHDC hOldFont=0;
  GXTEXTMETRICW textMetric;
  GXHINSTANCE hinst = (GXHINSTANCE)gxGetWindowLongPtrW(infoPtr->hwndSelf, GXGWLP_HINSTANCE);

  TRACE("(text=%s, ..., isW=%d)\n", debugtext_t(text, isW), isW);

  style |= GXWS_CHILDWINDOW|GXWS_CLIPSIBLINGS|GXES_LEFT|GXES_AUTOHSCROLL|GXWS_BORDER;
  hdc = gxGetDC(infoPtr->hwndSelf);

  /* Select the font to get appropriate metric dimensions */
  if(infoPtr->hFont != 0)
    hOldFont = (GXHDC)gxSelectObject(hdc, infoPtr->hFont);

  /*Get String Length in pixels */
  gxGetTextExtentPoint32W(hdc, text, GXSTRLEN(text), &sz);

  /*Add Extra spacing for the next character */
  gxGetTextMetricsW(hdc, &textMetric);
  sz.cx += (textMetric.tmMaxCharWidth * 2);

  if(infoPtr->hFont != 0)
    gxSelectObject(hdc, hOldFont);

  gxReleaseDC(infoPtr->hwndSelf, hdc);
  if (isW)
    hedit = gxCreateWindowW(editName, text, style, x, y, sz.cx, height, infoPtr->hwndSelf, 0, hinst, 0);
  //    else
  //  hedit = gxCreateWindowA("Edit", (GXLPCSTR)text, style, x, y, sz.cx, height, infoPtr->hwndSelf, 0, hinst, 0);

  if (!hedit) return 0;

  infoPtr->EditWndProc = (GXWNDPROC)
    (isW ? gxSetWindowLongPtrW(hedit, GXGWLP_WNDPROC, (GXDWORD_PTR)EditLblWndProcW) :
    gxSetWindowLongPtrA(hedit, GXGWLP_WNDPROC, (GXDWORD_PTR)EditLblWndProcA) );

  gxSendMessageW(hedit, GXWM_SETFONT, (GXWPARAM)infoPtr->hFont, FALSE);

  return hedit;
}
#endif // _DEV_DISABLE_UI_CODE