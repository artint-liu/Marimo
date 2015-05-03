#ifndef _DEV_DISABLE_UI_CODE
/*
 * Listbox controls
 *
 * Copyright 1996 Alexandre Julliard
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
 * of Comctl32.dll version 6.0 on Oct. 9, 2004, by Dimitrie O. Paun.
 * 
 * Unless otherwise noted, we believe this code to be complete, as per
 * the specification mentioned above.
 * If you discover missing features, or bugs, please note them below.
 *
 * TODO:
 *    - GetListBoxInfo()
 *    - LB_GETLISTBOXINFO
 *    - LBS_NODATA
 */
#include <GrapX.H>
#include "Include/GXUser.H"
#include "Include/GXGDI.H"
#include "Include/gxKernel.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <User/WineComm.H>
//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "wine/winuser16.h"
//#include "wine/unicode.h"
//#include "user_private.h"
//#include "controls.h"
//#include "wine/debug.h"
#include "gxError.H"
//WINE_DEFAULT_DEBUG_CHANNEL(listbox);

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较


/* Items array granularity */
#define LB_ARRAY_GRANULARITY 16

/* Scrolling timeout in ms */
#define LB_SCROLL_TIMEOUT 50

/* Listbox system timer id */
#define LB_TIMER_ID  2

/* flag listbox changed while setredraw false - internal style */
#define GXLBS_DISPLAYCHANGED 0x80000000

/* Item structure */
typedef struct
{
    GXLPWSTR    str;       /* Item text */
    GXBOOL      selected;  /* Is item selected? */
    GXUINT      height;    /* Item height (only for OWNERDRAWVARIABLE) */
    GXULONG_PTR data;      /* User data */
} GXLB_ITEMDATA;

/* Listbox structure */
typedef struct
{
    GXHWND        self;           /* Our own window handle */
    GXHWND        owner;          /* Owner window to send notifications to */
    GXUINT        style;          /* Window style */
    GXINT         width;          /* Window width */
    GXINT         height;         /* Window height */
    GXLB_ITEMDATA  *items;        /* Array of items */
    GXINT         nb_items;       /* Number of items */
    GXINT         top_item;       /* Top visible item */
    GXINT         selected_item;  /* Selected item */
    GXINT         focus_item;     /* Item that has the focus */
    GXINT         anchor_item;    /* Anchor item for extended selection */
    GXINT         item_height;    /* Default item height */
    GXINT         page_size;      /* Items per listbox page */
    GXINT         column_width;   /* Column width for multi-column listboxes */
    GXINT         horz_extent;    /* Horizontal extent (0 if no hscroll) */
    GXINT         horz_pos;       /* Horizontal position */
    GXINT         nb_tabs;        /* Number of tabs in array */
    GXINT        *tabs;           /* Array of tabs */
    GXINT         avg_char_width; /* Average width of characters */
    GXBOOL        caret_on;       /* Is caret on? */
    GXBOOL        captured;       /* Is mouse captured? */
    GXBOOL  in_focus;
    GXHFONT       font;           /* Current font */
    GXLCID          locale;       /* Current locale for string comparisons */
    GXLPHEADCOMBO   lphc;    /* ComboLBox */
} GXLB_DESCR;


#define IS_OWNERDRAW(descr) \
    ((descr)->style & (GXLBS_OWNERDRAWFIXED | GXLBS_OWNERDRAWVARIABLE))

#define HAS_STRINGS(descr) \
    (!IS_OWNERDRAW(descr) || ((descr)->style & GXLBS_HASSTRINGS))


#define IS_MULTISELECT(descr) \
    ((descr)->style & (GXLBS_MULTIPLESEL|GXLBS_EXTENDEDSEL) && \
     !((descr)->style & GXLBS_NOSEL))

#define SEND_NOTIFICATION(descr,code) \
    (gxSendMessageW( (descr)->owner, GXWM_COMMAND, \
     GXMAKEWPARAM( gxGetWindowLongPtrW((descr->self),GXGWLP_ID), (code)), (GXLPARAM)(descr->self) ))

#define ISWIN31 FALSE//(GXLOWORD(GetVersion()) == 0x0a03)

/* Current timer status */
typedef enum
{
    LB_TIMER_NONE,
    LB_TIMER_UP,
    LB_TIMER_LEFT,
    LB_TIMER_DOWN,
    LB_TIMER_RIGHT
} TIMER_DIRECTION;

static TIMER_DIRECTION LISTBOX_Timer = LB_TIMER_NONE;

GXLRESULT GXDLLAPI ListBoxWndProcA( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam );
GXLRESULT GXDLLAPI ListBoxWndProcW( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam );

static GXLRESULT LISTBOX_GetItemRect( const GXLB_DESCR *descr, GXINT index, GXRECT *rect );


GXWNDCLASSEX WndClassEx_MyListbox = { sizeof(GXWNDCLASSEX), GXCS_DBLCLKS, ListBoxWndProcW, 0L, sizeof(GXLB_DESCR *),
(GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPWSTR)GXIDC_ARROW), NULL, NULL,
GXWC_LISTBOXW, NULL };


/*********************************************************************
 * listbox class descriptor
 */
static const GXWCHAR listboxW[] = {'L','i','s','t','B','o','x',0};
//const struct builtin_class_descr LISTBOX_builtin_class =
//{
//    listboxW,             /* name */
//    CS_DBLCLKS /*| CS_PARENTDC*/,  /* style */
//    ListBoxWndProcA,      /* procA */
//    ListBoxWndProcW,      /* procW */
//    sizeof(GXLB_DESCR *),   /* extra */
//    IDC_ARROW,            /* cursor */
//    0                     /* brush */
//};


/*********************************************************************
 * combolbox class descriptor
 */
static const GXWCHAR combolboxW[] = {'C','o','m','b','o','L','B','o','x',0};
//const struct builtin_class_descr COMBOLBOX_builtin_class =
//{
//    combolboxW,           /* name */
//    CS_DBLCLKS | CS_SAVEBITS,  /* style */
//    ListBoxWndProcA,      /* procA */
//    ListBoxWndProcW,      /* procW */
//    sizeof(GXLB_DESCR *),   /* extra */
//    IDC_ARROW,            /* cursor */
//    0                     /* brush */
//};


GXVOID COMBO_FlipListbox(GXLPHEADCOMBO a, GXBOOL bFirst, GXBOOL bSecond)
{
  ASSERT(FALSE);
}



/* check whether app is a Win 3.1 app */
static inline GXBOOL is_old_app( GXLB_DESCR *descr )
{
  return FALSE;
    //return (GetExpWinVer16( gxGetWindowLongPtrW(descr->self, GXGWLP_HINSTANCE) ) & 0xFF00 ) == 0x0300;
}


/***********************************************************************
 *           LISTBOX_GetCurrentPageSize
 *
 * Return the current page size
 */
static GXINT LISTBOX_GetCurrentPageSize( const GXLB_DESCR *descr )
{
    GXINT i, height;
    if (!(descr->style & GXLBS_OWNERDRAWVARIABLE)) return descr->page_size;
    for (i = descr->top_item, height = 0; i < descr->nb_items; i++)
    {
        if ((height += descr->items[i].height) > descr->height) break;
    }
    if (i == descr->top_item) return 1;
    else return i - descr->top_item;
}


/***********************************************************************
 *           LISTBOX_GetMaxTopIndex
 *
 * Return the maximum possible index for the top of the listbox.
 */
static GXINT LISTBOX_GetMaxTopIndex( const GXLB_DESCR *descr )
{
    GXINT max, page;

    if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        page = descr->height;
        for (max = descr->nb_items - 1; max >= 0; max--)
            if ((page -= descr->items[max].height) < 0) break;
        if (max < descr->nb_items - 1) max++;
    }
    else if (descr->style & GXLBS_MULTICOLUMN)
    {
        if ((page = descr->width / descr->column_width) < 1) page = 1;
        max = (descr->nb_items + descr->page_size - 1) / descr->page_size;
        max = (max - page) * descr->page_size;
    }
    else
    {
        max = descr->nb_items - descr->page_size;
    }
    if (max < 0) max = 0;
    return max;
}


/***********************************************************************
 *           LISTBOX_UpdateScroll
 *
 * Update the scrollbars. Should be called whenever the content
 * of the listbox changes.
 */
static void LISTBOX_UpdateScroll( GXLB_DESCR *descr )
{
    GXSCROLLINFO info;

    /* Check the listbox scroll bar flags individually before we call
       SetScrollInfo otherwise when the listbox style is WS_HSCROLL and
       no WS_VSCROLL, we end up with an uninitialized, visible horizontal
       scroll bar when we do not need one.
    if (!(descr->style & WS_VSCROLL)) return;
    */

    /*   It is important that we check descr->style, and not wnd->dwStyle,
       for WS_VSCROLL, as the former is exactly the one passed in
       argument to CreateWindow.
         In Windows (and from now on in Wine :) a listbox created
       with such a style (no WS_SCROLL) does not update
       the scrollbar with listbox-related data, thus letting
       the programmer use it for his/her own purposes. */

    if (descr->style & GXLBS_NOREDRAW) return;
    info.cbSize = sizeof(info);

    if (descr->style & GXLBS_MULTICOLUMN)
    {
        info.nMin  = 0;
        info.nMax  = (descr->nb_items - 1) / descr->page_size;
        info.nPos  = descr->top_item / descr->page_size;
        info.nPage = descr->width / descr->column_width;
        if (info.nPage < 1) info.nPage = 1;
        info.fMask = GXSIF_RANGE | GXSIF_POS | GXSIF_PAGE;
        if (descr->style & GXLBS_DISABLENOSCROLL)
            info.fMask |= GXSIF_DISABLENOSCROLL;
        if (descr->style & GXWS_HSCROLL)
            gxSetScrollInfo( descr->self, GXSB_HORZ, &info, TRUE );
        info.nMax = 0;
        info.fMask = GXSIF_RANGE;
        if (descr->style & GXWS_VSCROLL)
            gxSetScrollInfo( descr->self, GXSB_VERT, &info, TRUE );
    }
    else
    {
        info.nMin  = 0;
        info.nMax  = descr->nb_items - 1;
        info.nPos  = descr->top_item;
        info.nPage = LISTBOX_GetCurrentPageSize( descr );
        info.fMask = GXSIF_RANGE | GXSIF_POS | GXSIF_PAGE;
        if (descr->style & GXLBS_DISABLENOSCROLL)
            info.fMask |= GXSIF_DISABLENOSCROLL;
        if (descr->style & GXWS_VSCROLL)
            gxSetScrollInfo( descr->self, GXSB_VERT, &info, TRUE );

        if (descr->horz_extent)
        {
            info.nMin  = 0;
            info.nMax  = descr->horz_extent - 1;
            info.nPos  = descr->horz_pos;
            info.nPage = descr->width;
            info.fMask = GXSIF_RANGE | GXSIF_POS | GXSIF_PAGE;
            if (descr->style & GXLBS_DISABLENOSCROLL)
                info.fMask |= GXSIF_DISABLENOSCROLL;
            if (descr->style & GXWS_HSCROLL)
                gxSetScrollInfo( descr->self, GXSB_HORZ, &info, TRUE );
        }
    }
}


/***********************************************************************
 *           LISTBOX_SetTopItem
 *
 * Set the top item of the listbox, scrolling up or down if necessary.
 */
static GXLRESULT LISTBOX_SetTopItem( GXLB_DESCR *descr, GXINT index, GXBOOL scroll )
{
    GXINT max = LISTBOX_GetMaxTopIndex( descr );

    LB_TRACE("setting top item %d, scroll %d\n", index, scroll);

    if (index > max) index = max;
    if (index < 0) index = 0;
    if (descr->style & GXLBS_MULTICOLUMN) index -= index % descr->page_size;
    if (descr->top_item == index) return GXLB_OKAY;
    if (descr->style & GXLBS_MULTICOLUMN)
    {
        GXINT diff = (descr->top_item - index) / descr->page_size * descr->column_width;
        if (scroll && (abs((int)diff) < descr->width))
            gxScrollWindowEx( descr->self, diff, 0, NULL, NULL, 0, NULL,
                              GXSW_INVALIDATE | GXSW_ERASE | GXSW_SCROLLCHILDREN );

        else
            scroll = FALSE;
    }
    else if (scroll)
    {
        GXINT diff;
        if (descr->style & GXLBS_OWNERDRAWVARIABLE)
        {
            GXINT i;
            diff = 0;
            if (index > descr->top_item)
            {
                for (i = index - 1; i >= descr->top_item; i--)
                    diff -= descr->items[i].height;
            }
            else
            {
                for (i = index; i < descr->top_item; i++)
                    diff += descr->items[i].height;
            }
        }
        else
            diff = (descr->top_item - index) * descr->item_height;

        if (abs((int)diff) < descr->height)
            gxScrollWindowEx( descr->self, 0, diff, NULL, NULL, 0, NULL,
                            GXSW_INVALIDATE | GXSW_ERASE | GXSW_SCROLLCHILDREN );
        else
            scroll = FALSE;
    }
    if (!scroll) gxInvalidateRect( descr->self, NULL, TRUE );
    descr->top_item = index;
    LISTBOX_UpdateScroll( descr );
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_UpdatePage
 *
 * Update the page size. Should be called when the size of
 * the client area or the item height changes.
 */
static void LISTBOX_UpdatePage( GXLB_DESCR *descr )
{
    GXINT page_size;

    if ((descr->item_height == 0) || (page_size = descr->height / descr->item_height) < 1)
                       page_size = 1;
    if (page_size == descr->page_size) return;
    descr->page_size = page_size;
    if (descr->style & GXLBS_MULTICOLUMN)
        gxInvalidateRect( descr->self, NULL, TRUE );
    LISTBOX_SetTopItem( descr, descr->top_item, FALSE );
}


/***********************************************************************
 *           LISTBOX_UpdateSize
 *
 * Update the size of the listbox. Should be called when the size of
 * the client area changes.
 */
static void LISTBOX_UpdateSize( GXLB_DESCR *descr )
{
    GXRECT rect;

    gxGetClientRect( descr->self, &rect );
    descr->width  = rect.right - rect.left;
    descr->height = rect.bottom - rect.top;
    if (!(descr->style & GXLBS_NOINTEGRALHEIGHT) && !(descr->style & GXLBS_OWNERDRAWVARIABLE))
    {
        GXINT remaining;
        GXRECT rect;

        gxGetWindowRect( descr->self, &rect );
        if(descr->item_height != 0)
            remaining = descr->height % descr->item_height;
        else
            remaining = 0;
        if ((descr->height > descr->item_height) && remaining)
        {
            if (is_old_app(descr))
            { /* give a margin for error to 16 bits programs - if we need
                 less than the height of the nonclient area, round to the
                 *next* number of items */
                GXINT ncheight = rect.bottom - rect.top - descr->height;
                if ((descr->item_height - remaining) <= ncheight)
                    remaining = remaining - descr->item_height;
            }
            LB_TRACE("[%p]: changing height %d -> %d\n",
                  descr->self, descr->height, descr->height - remaining );
            gxSetWindowPos( descr->self, 0, 0, 0, rect.right - rect.left,
                          rect.bottom - rect.top - remaining,
                          GXSWP_NOZORDER | GXSWP_NOACTIVATE | GXSWP_NOMOVE );
            return;
        }
    }
    LB_TRACE("[%p]: new size = %d,%d\n", descr->self, descr->width, descr->height );
    LISTBOX_UpdatePage( descr );
    LISTBOX_UpdateScroll( descr );

    /* Invalidate the focused item so it will be repainted correctly */
    if (LISTBOX_GetItemRect( descr, descr->focus_item, &rect ) == 1)
    {
        gxInvalidateRect( descr->self, &rect, FALSE );
    }
}


/***********************************************************************
 *           LISTBOX_GetItemRect
 *
 * Get the rectangle enclosing an item, in listbox client coordinates.
 * Return 1 if the rectangle is (partially) visible, 0 if hidden, -1 on error.
 */
static GXLRESULT LISTBOX_GetItemRect( const GXLB_DESCR *descr, GXINT index, GXRECT *rect )
{
    /* Index <= 0 is legal even on empty listboxes */
    if (index && (index >= descr->nb_items))
    {
        memset(rect, 0, sizeof(*rect));
        gxSetLastError(GXERROR_INVALID_INDEX);
        return GXLB_ERR;
    }
    gxSetRect( rect, 0, 0, descr->width, descr->height );
    if (descr->style & GXLBS_MULTICOLUMN)
    {
        GXINT col = (index / descr->page_size) -
                        (descr->top_item / descr->page_size);
        rect->left += col * descr->column_width;
        rect->right = rect->left + descr->column_width;
        rect->top += (index % descr->page_size) * descr->item_height;
        rect->bottom = rect->top + descr->item_height;
    }
    else if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        GXINT i;
        rect->right += descr->horz_pos;
        if ((index >= 0) && (index < descr->nb_items))
        {
            if (index < descr->top_item)
            {
                for (i = descr->top_item-1; i >= index; i--)
                    rect->top -= descr->items[i].height;
            }
            else
            {
                for (i = descr->top_item; i < index; i++)
                    rect->top += descr->items[i].height;
            }
            rect->bottom = rect->top + descr->items[index].height;

        }
    }
    else
    {
        rect->top += (index - descr->top_item) * descr->item_height;
        rect->bottom = rect->top + descr->item_height;
        rect->right += descr->horz_pos;
    }

    //LB_TRACE("item %d, rect %s\n", index, wine_dbgstr_rect(rect));

    return ((rect->left < descr->width) && (rect->right > 0) &&
            (rect->top < descr->height) && (rect->bottom > 0));
}


/***********************************************************************
 *           LISTBOX_GetItemFromPoint
 *
 * Return the item nearest from point (x,y) (in client coordinates).
 */
static GXINT LISTBOX_GetItemFromPoint( const GXLB_DESCR *descr, GXINT x, GXINT y )
{
    GXINT index = descr->top_item;

    if (!descr->nb_items) return -1;  /* No items */
    if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        GXINT pos = 0;
        if (y >= 0)
        {
            while (index < descr->nb_items)
            {
                if ((pos += descr->items[index].height) > y) break;
                index++;
            }
        }
        else
        {
            while (index > 0)
            {
                index--;
                if ((pos -= descr->items[index].height) <= y) break;
            }
        }
    }
    else if (descr->style & GXLBS_MULTICOLUMN)
    {
        if (y >= descr->item_height * descr->page_size) return -1;
        if (y >= 0) index += y / descr->item_height;
        if (x >= 0) index += (x / descr->column_width) * descr->page_size;
        else index -= (((x + 1) / descr->column_width) - 1) * descr->page_size;
    }
    else
    {
        index += (y / descr->item_height);
    }
    if (index < 0) return 0;
    if (index >= descr->nb_items) return -1;
    return index;
}


/***********************************************************************
 *           LISTBOX_PaintItem
 *
 * Paint an item.
 */
static void LISTBOX_PaintItem( GXLB_DESCR *descr, GXHDC hdc, const GXRECT *rect, 
             GXINT index, GXUINT action, GXBOOL ignoreFocus )
{
    GXLB_ITEMDATA *item = NULL;
    if (index < descr->nb_items) item = &descr->items[index];

    if (IS_OWNERDRAW(descr))
    {
        GXDRAWITEMSTRUCT dis;
        GXRECT r;
        GXHRGN hrgn;

  if (!item)
  {
      if (action == GXODA_FOCUS)
    gxDrawFocusRect( hdc, rect );
      else
          ERR("called with an out of bounds index %d(%d) in owner draw, Not good.\n",index,descr->nb_items);
      return;
  }

        /* some programs mess with the clipping region when
        drawing the item, *and* restore the previous region
        after they are done, so a region has better to exist
        else everything ends clipped */
        gxGetClientRect(descr->self, &r);
        hrgn = gxCreateRectRgnIndirect(&r);
        gxSelectClipRgn( hdc, hrgn);
        gxDeleteObject( hrgn );

        dis.CtlType      = GXODT_LISTBOX;
        dis.CtlID        = (GXUINT)gxGetWindowLongPtrW( descr->self, GXGWLP_ID );
        dis.hwndItem     = (GXHWND)descr->self;
        dis.itemAction   = action;
        dis.hDC          = (GXHDC)hdc;
        dis.itemID       = (GXUINT)index;
        dis.itemState    = 0;
        if (item->selected) dis.itemState |= GXODS_SELECTED;
        if (!ignoreFocus && (descr->focus_item == index) &&
            (descr->caret_on) &&
            (descr->in_focus)) dis.itemState |= GXODS_FOCUS;
        if (!gxIsWindowEnabled(descr->self)) dis.itemState |= GXODS_DISABLED;
        dis.itemData     = item->data;
        dis.rcItem.left  = (GXLONG)rect->left;
    dis.rcItem.top  = (GXLONG)rect->top;
    dis.rcItem.right = (GXLONG)rect->right;
    dis.rcItem.bottom  = (GXLONG)rect->bottom;
        //LB_TRACE("[%p]: drawitem %d (%s) action=%02x state=%02x rect=%d,%d-%d,%d\n",
        //      descr->self, index, item ? debugstr_w(item->str) : "", action,
        //      dis.itemState, rect->left, rect->top, rect->right, rect->bottom );
        gxSendMessageW(descr->owner, GXWM_DRAWITEM, dis.CtlID, (GXLPARAM)&dis);
    }
    else
    {
        GXCOLORREF oldText = 0, oldBk = 0;

        if (action == GXODA_FOCUS)
        {
            gxDrawFocusRect( hdc, rect );
            return;
        }
        if (item && item->selected)
        {
            oldBk = gxSetBkColor( hdc, gxGetSysColor( GXCOLOR_HIGHLIGHT ) );
            oldText = gxSetTextColor( hdc, gxGetSysColor(GXCOLOR_HIGHLIGHTTEXT));
        }

        //LB_TRACE("[%p]: painting %d (%s) action=%02x rect=%d,%d-%d,%d\n",
        //      descr->self, index, item ? debugstr_w(item->str) : "", action,
        //      rect->left, rect->top, rect->right, rect->bottom );
        if (!item)
            gxExtTextOutW( hdc, rect->left + 1, rect->top,
                           GXETO_OPAQUE | GXETO_CLIPPED, rect, NULL, 0, NULL );
        else if (!(descr->style & GXLBS_USETABSTOPS))
            gxExtTextOutW( hdc, rect->left + 1, rect->top,
                         GXETO_OPAQUE | GXETO_CLIPPED, rect, item->str,
                         GXSTRLEN(item->str), NULL );
        else
  {
      /* Output empty string to paint background in the full width. */
            gxExtTextOutW( hdc, rect->left + 1, rect->top,
                         GXETO_OPAQUE | GXETO_CLIPPED, rect, NULL, 0, NULL );
            gxTabbedTextOutW( hdc, rect->left + 1 , rect->top,
                            item->str, GXSTRLEN(item->str),
                            descr->nb_tabs, descr->tabs, 0);
  }
        if (item && item->selected)
        {
            gxSetBkColor( hdc, oldBk );
            gxSetTextColor( hdc, oldText );
        }
        if (!ignoreFocus && (descr->focus_item == index) &&
            (descr->caret_on) &&
            (descr->in_focus)) gxDrawFocusRect( hdc, rect );
    }
}


/***********************************************************************
 *           LISTBOX_SetRedraw
 *
 * Change the redraw flag.
 */
static void LISTBOX_SetRedraw( GXLB_DESCR *descr, GXBOOL on )
{
    if (on)
    {
        if (!(descr->style & GXLBS_NOREDRAW)) return;
        descr->style &= ~GXLBS_NOREDRAW;
        if (descr->style & GXLBS_DISPLAYCHANGED)
        {     /* page was changed while setredraw false, refresh automatically */
            gxInvalidateRect(descr->self, NULL, TRUE);
            if ((descr->top_item + descr->page_size) > descr->nb_items)
            {      /* reset top of page if less than number of items/page */
                descr->top_item = descr->nb_items - descr->page_size;
                if (descr->top_item < 0) descr->top_item = 0;
            }
            descr->style &= ~GXLBS_DISPLAYCHANGED;
        }
        LISTBOX_UpdateScroll( descr );
    }
    else descr->style |= GXLBS_NOREDRAW;
}


/***********************************************************************
 *           LISTBOX_RepaintItem
 *
 * Repaint a single item synchronously.
 */
static void LISTBOX_RepaintItem( GXLB_DESCR *descr, GXINT index, GXUINT action )
{
    GXHDC hdc;
    GXRECT rect;
    GXHFONT oldFont = 0;
    GXHBRUSH hbrush, oldBrush = 0;

    /* Do not repaint the item if the item is not visible */
    if (!gxIsWindowVisible(descr->self)) return;
    if (descr->style & GXLBS_NOREDRAW)
    {
        descr->style |= GXLBS_DISPLAYCHANGED;
        return;
    }
    if (LISTBOX_GetItemRect( descr, index, &rect ) != 1) return;
    if (!(hdc = gxGetDCEx( descr->self, 0, GXDCX_CACHE ))) return;
    if (descr->font) oldFont = (GXHFONT)gxSelectObject( hdc, descr->font );
    hbrush = (GXHBRUSH)gxSendMessageW( descr->owner, GXWM_CTLCOLORLISTBOX,
           (GXWPARAM)hdc, (GXLPARAM)descr->self );
    if (hbrush) oldBrush = (GXHBRUSH)gxSelectObject( hdc, hbrush );
    if (!gxIsWindowEnabled(descr->self))
        gxSetTextColor( hdc, gxGetSysColor( GXCOLOR_GRAYTEXT ) );
    gxSetWindowOrgEx( hdc, descr->horz_pos, 0, NULL );
    LISTBOX_PaintItem( descr, hdc, &rect, index, action, TRUE );
    if (oldFont) gxSelectObject( hdc, oldFont );
    if (oldBrush) gxSelectObject( hdc, oldBrush );
    gxReleaseDC( descr->self, hdc );
}


/***********************************************************************
 *           LISTBOX_DrawFocusRect
 */
static void LISTBOX_DrawFocusRect( GXLB_DESCR *descr, GXBOOL on )
{
    GXHDC hdc;
    GXRECT rect;
    GXHFONT oldFont = 0;

    /* Do not repaint the item if the item is not visible */
    if (!gxIsWindowVisible(descr->self)) return;

    if (descr->focus_item == -1) return;
    if (!descr->caret_on || !descr->in_focus) return;

    if (LISTBOX_GetItemRect( descr, descr->focus_item, &rect ) != 1) return;
    if (!(hdc = gxGetDCEx( descr->self, 0, GXDCX_CACHE ))) return;
    if (descr->font) oldFont = (GXHFONT)gxSelectObject( hdc, descr->font );
    if (!gxIsWindowEnabled(descr->self))
        gxSetTextColor( hdc, gxGetSysColor( GXCOLOR_GRAYTEXT ) );
    gxSetWindowOrgEx( hdc, descr->horz_pos, 0, NULL );
    LISTBOX_PaintItem( descr, hdc, &rect, descr->focus_item, GXODA_FOCUS, on ? FALSE : TRUE );
    if (oldFont) gxSelectObject( hdc, oldFont );
    gxReleaseDC( descr->self, hdc );
}


/***********************************************************************
 *           LISTBOX_InitStorage
 */
static GXLRESULT LISTBOX_InitStorage( GXLB_DESCR *descr, GXSIZE_T nb_items )
{
  GXLB_ITEMDATA *item;

  nb_items += LB_ARRAY_GRANULARITY - 1;
  nb_items -= (nb_items % LB_ARRAY_GRANULARITY);
  if (descr->items) {
    nb_items += gxHeapSize( gxGetProcessHeap(), 0, descr->items ) / sizeof(*item);
    item = (GXLB_ITEMDATA *)gxHeapReAlloc( gxGetProcessHeap(), 0, descr->items,
      nb_items * sizeof(GXLB_ITEMDATA));
  }
  else {
    item = (GXLB_ITEMDATA *)gxHeapAlloc( gxGetProcessHeap(), 0,
      nb_items * sizeof(GXLB_ITEMDATA));
  }

  if (!item)
  {
    SEND_NOTIFICATION( descr, GXLBN_ERRSPACE );
    return GXLB_ERRSPACE;
  }
  descr->items = item;
  return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_SetTabStops
 */
static GXBOOL LISTBOX_SetTabStops( GXLB_DESCR *descr, GXINT count, GXINT* tabs, GXBOOL short_ints )
{
    GXINT i;

    if (!(descr->style & GXLBS_USETABSTOPS))
    {
        gxSetLastError(GXERROR_LB_WITHOUT_TABSTOPS);
        return FALSE;
    }

    gxHeapFree( gxGetProcessHeap(), 0, descr->tabs );
    if (!(descr->nb_tabs = count))
    {
        descr->tabs = NULL;
        return TRUE;
    }
    if (!(descr->tabs = (GXINT*)gxHeapAlloc( gxGetProcessHeap(), 0,
                                            descr->nb_tabs * sizeof(GXINT) )))
        return FALSE;
    if (short_ints)
    {
        GXINT i;
        GXLPINT16 p = (GXLPINT16)tabs;

        LB_TRACE("[%p]: settabstops ", descr->self );
        for (i = 0; i < descr->nb_tabs; i++) {
      descr->tabs[i] = *p++<<1; /* FIXME */
            LB_TRACE("%hd ", descr->tabs[i]);
  }
        LB_TRACE("\n");
    }
    else memcpy( descr->tabs, tabs, descr->nb_tabs * sizeof(GXINT) );

    /* convert into "dialog units"*/
    for (i = 0; i < descr->nb_tabs; i++)
        descr->tabs[i] = (GXINT)gxMulDiv((int)descr->tabs[i], (int)descr->avg_char_width, 4);

    return TRUE;
}


/***********************************************************************
 *           LISTBOX_GetText
 */
static GXLRESULT LISTBOX_GetText( GXLB_DESCR *descr, GXINT index, GXLPWSTR buffer, GXBOOL unicode )
{
    if ((index < 0) || (index >= descr->nb_items))
    {
        gxSetLastError(GXERROR_INVALID_INDEX);
        return GXLB_ERR;
    }
    if (HAS_STRINGS(descr))
    {
        if (!buffer)
        {
            GXDWORD len = GXSTRLEN(descr->items[index].str);
            if( unicode )
                return len;
            return gxWideCharToMultiByte( GXCP_ACP, 0, descr->items[index].str, (int)len,
                                        NULL, 0, NULL, NULL );
        }

  LB_TRACE("index %d (0x%04x) %s\n", index, index, debugstr_w(descr->items[index].str));

        if(unicode)
        {
            GXSTRCPY( buffer, descr->items[index].str );
            return GXSTRLEN(buffer);
        }
        else
        {
            return gxWideCharToMultiByte(GXCP_ACP, 0, descr->items[index].str, -1, (GXLPSTR)buffer, 0x7FFFFFFF, NULL, NULL) - 1;
        }
    } else {
        if (buffer)
            *((GXLPDWORD)buffer)=*(GXLPDWORD)(&descr->items[index].data);
        return sizeof(GXDWORD);
    }
}

static inline GXINT LISTBOX_lstrcmpiW( GXLCID lcid, GXLPCWSTR str1, GXLPCWSTR str2 )
{
  return GXSTRCMPI(str1, str2);
     
    //GXINT ret = CompareStringW( lcid, NORM_IGNORECASE, str1, -1, str2, -1 );
    //if (ret == CSTR_LESS_THAN)
    //    return -1;
    //if (ret == CSTR_EQUAL)
    //    return 0;
    //if (ret == CSTR_GREATER_THAN)
    //    return 1;
    //return -1;
}

/***********************************************************************
 *           LISTBOX_FindStringPos
 *
 * Find the nearest string located before a given string in sort order.
 * If 'exact' is TRUE, return an error if we don't get an exact match.
 */
static GXINT LISTBOX_FindStringPos( GXLB_DESCR *descr, GXLPCWSTR str, GXBOOL exact )
{
    GXINT index, min, max, res = -1;

    if (!(descr->style & GXLBS_SORT)) return -1;  /* Add it at the end */
    min = 0;
    max = descr->nb_items;
    while (min != max)
    {
        index = (min + max) / 2;
        if (HAS_STRINGS(descr))
            res = LISTBOX_lstrcmpiW( descr->locale, str, descr->items[index].str);
        else
        {
            GXCOMPAREITEMSTRUCT cis;
            GXUINT id = (GXUINT)gxGetWindowLongPtrW( descr->self, GXGWLP_ID );

            cis.CtlType    = GXODT_LISTBOX;
            cis.CtlID      = id;
            cis.hwndItem   = (GXHWND)descr->self;
            /* note that some application (MetaStock) expects the second item
             * to be in the listbox */
            cis.itemID1    = -1;
            cis.itemData1  = (GXULONG_PTR)str;
            cis.itemID2    = (GXUINT)index;
            cis.itemData2  = descr->items[index].data;
            cis.dwLocaleId = descr->locale;
            res = gxSendMessageW( descr->owner, GXWM_COMPAREITEM, id, (GXLPARAM)&cis );
        }
        if (!res) return index;
        if (res < 0) max = index;
        else min = index + 1;
    }
    return exact ? -1 : max;
}


/***********************************************************************
 *           LISTBOX_FindFileStrPos
 *
 * Find the nearest string located before a given string in directory
 * sort order (i.e. first files, then directories, then drives).
 */
static GXINT LISTBOX_FindFileStrPos( GXLB_DESCR *descr, GXLPCWSTR str )
{
    GXINT min, max, res = -1;

    if (!HAS_STRINGS(descr))
        return LISTBOX_FindStringPos( descr, str, FALSE );
    min = 0;
    max = descr->nb_items;
    while (min != max)
    {
        GXINT index = (min + max) / 2;
        GXLPCWSTR p = descr->items[index].str;
        if (*p == '[')  /* drive or directory */
        {
            if (*str != '[') res = -1;
            else if (p[1] == '-')  /* drive */
            {
                if (str[1] == '-') res = str[2] - p[2];
                else res = -1;
            }
            else  /* directory */
            {
                if (str[1] == '-') res = 1;
                else res = LISTBOX_lstrcmpiW( descr->locale, str, p );
            }
        }
        else  /* filename */
        {
            if (*str == '[') res = 1;
            else res = LISTBOX_lstrcmpiW( descr->locale, str, p );
        }
        if (!res) return index;
        if (res < 0) max = index;
        else min = index + 1;
    }
    return max;
}


/***********************************************************************
 *           LISTBOX_FindString
 *
 * Find the item beginning with a given string.
 */
static GXINT LISTBOX_FindString( GXLB_DESCR *descr, GXINT start, GXLPCWSTR str, GXBOOL exact )
{
    GXINT i;
    GXLB_ITEMDATA *item;

    if (start >= descr->nb_items) start = -1;
    item = descr->items + start + 1;
    if (HAS_STRINGS(descr))
    {
        if (!str || ! str[0] ) return GXLB_ERR;
        if (exact)
        {
            for (i = start + 1; i < descr->nb_items; i++, item++)
                if (!LISTBOX_lstrcmpiW( descr->locale, str, item->str )) return i;
            for (i = 0, item = descr->items; i <= start; i++, item++)
                if (!LISTBOX_lstrcmpiW( descr->locale, str, item->str )) return i;
        }
        else
        {
 /* Special case for drives and directories: ignore prefix */
#define CHECK_DRIVE(item) \
    if ((item)->str[0] == '[') \
    { \
        if (!strncmpiW( str, (item)->str+1, len )) return i; \
        if (((item)->str[1] == '-') && !strncmpiW(str, (item)->str+2, len)) \
        return i; \
    }

            GXINT len = GXSTRLEN(str);
            for (i = start + 1; i < descr->nb_items; i++, item++)
            {
               if (!strncmpiW( str, item->str, len )) return i;
               CHECK_DRIVE(item);
            }
            for (i = 0, item = descr->items; i <= start; i++, item++)
            {
               if (!strncmpiW( str, item->str, len )) return i;
               CHECK_DRIVE(item);
            }
#undef CHECK_DRIVE
        }
    }
    else
    {
        if (exact && (descr->style & GXLBS_SORT))
            /* If sorted, use a GXWM_COMPAREITEM binary search */
            return LISTBOX_FindStringPos( descr, str, TRUE );

        /* Otherwise use a linear search */
        for (i = start + 1; i < descr->nb_items; i++, item++)
            if (item->data == (GXULONG_PTR)str) return i;
        for (i = 0, item = descr->items; i <= start; i++, item++)
            if (item->data == (GXULONG_PTR)str) return i;
    }
    return GXLB_ERR;
}


/***********************************************************************
 *           LISTBOX_GetSelCount
 */
static GXLRESULT LISTBOX_GetSelCount( const GXLB_DESCR *descr )
{
    GXINT i, count;
    const GXLB_ITEMDATA *item = descr->items;

    if (!(descr->style & GXLBS_MULTIPLESEL) ||
        (descr->style & GXLBS_NOSEL))
      return GXLB_ERR;
    for (i = count = 0; i < descr->nb_items; i++, item++)
        if (item->selected) count++;
    return count;
}


/***********************************************************************
 *           LISTBOX_GetSelItems16
 */
static GXLRESULT LISTBOX_GetSelItems16( const GXLB_DESCR *descr, GXINT16 max, GXLPINT16 array )
{
    GXINT i, count;
    const GXLB_ITEMDATA *item = descr->items;

    if (!(descr->style & GXLBS_MULTIPLESEL)) return GXLB_ERR;
    for (i = count = 0; (i < descr->nb_items) && (count < max); i++, item++)
        if (item->selected) array[count++] = (GXINT16)i;
    return count;
}


/***********************************************************************
 *           LISTBOX_GetSelItems
 */
static GXLRESULT LISTBOX_GetSelItems( const GXLB_DESCR *descr, GXINT max, GXINT* array )
{
    GXINT i, count;
    const GXLB_ITEMDATA *item = descr->items;

    if (!(descr->style & GXLBS_MULTIPLESEL)) return GXLB_ERR;
    for (i = count = 0; (i < descr->nb_items) && (count < max); i++, item++)
        if (item->selected) array[count++] = i;
    return count;
}


/***********************************************************************
 *           LISTBOX_Paint
 */
static GXLRESULT LISTBOX_Paint( GXLB_DESCR *descr, GXHDC hdc )
{
    GXINT i, col_pos = descr->page_size - 1;
    GXRECT rect;
    GXRECT focusRect = {-1, -1, -1, -1};
    GXHFONT oldFont = 0;
    GXHBRUSH hbrush, oldBrush = 0;

    if (descr->style & GXLBS_NOREDRAW) return 0;

    gxSetRect(&rect, 0, 0, descr->width, descr->height );
    if (descr->style & GXLBS_MULTICOLUMN)
        rect.right = rect.left + descr->column_width;
    else if (descr->horz_pos)
    {
        gxSetWindowOrgEx( hdc, descr->horz_pos, 0, NULL );
        rect.right += descr->horz_pos;
    }

    if (descr->font) oldFont = (GXHFONT)gxSelectObject( hdc, descr->font );
    hbrush = (GXHBRUSH)gxSendMessageW( descr->owner, GXWM_CTLCOLORLISTBOX,
              (GXWPARAM)hdc, (GXLPARAM)descr->self );
    if (hbrush) oldBrush = (GXHBRUSH)gxSelectObject( hdc, hbrush );
    if (!gxIsWindowEnabled(descr->self)) gxSetTextColor( hdc, gxGetSysColor( GXCOLOR_GRAYTEXT ) );

    if (!descr->nb_items && (descr->focus_item != -1) && descr->caret_on &&
        (descr->in_focus))
    {
        /* Special case for empty listbox: paint focus rect */
        rect.bottom = rect.top + descr->item_height;
        gxExtTextOutW( hdc, 0, 0, GXETO_OPAQUE | GXETO_CLIPPED,
                     &rect, NULL, 0, NULL );
        LISTBOX_PaintItem( descr, hdc, &rect, descr->focus_item, GXODA_FOCUS, FALSE );
        rect.top = rect.bottom;
    }

    /* Paint all the item, regarding the selection
       Focus state will be painted after  */

    for (i = descr->top_item; i < descr->nb_items; i++)
    {
        if (!(descr->style & GXLBS_OWNERDRAWVARIABLE))
            rect.bottom = rect.top + descr->item_height;
        else
            rect.bottom = rect.top + descr->items[i].height;

        if (i == descr->focus_item)
        {
      /* keep the focus rect, to paint the focus item after */
      focusRect.left = rect.left;
      focusRect.right = rect.right;
      focusRect.top = rect.top;
      focusRect.bottom = rect.bottom;
        }
        LISTBOX_PaintItem( descr, hdc, &rect, i, GXODA_DRAWENTIRE, TRUE );
        rect.top = rect.bottom;

        if ((descr->style & GXLBS_MULTICOLUMN) && !col_pos)
        {
            if (!IS_OWNERDRAW(descr))
            {
                /* Clear the bottom of the column */
                if (rect.top < descr->height)
                {
                    rect.bottom = descr->height;
                    gxExtTextOutW( hdc, 0, 0, GXETO_OPAQUE | GXETO_CLIPPED,
                                   &rect, NULL, 0, NULL );
                }
            }

            /* Go to the next column */
            rect.left += descr->column_width;
            rect.right += descr->column_width;
            rect.top = 0;
            col_pos = descr->page_size - 1;
        }
        else
        {
            col_pos--;
            if (rect.top >= descr->height) break;
        }
    }

    /* Paint the focus item now */
    if (focusRect.top != focusRect.bottom &&
        descr->caret_on && descr->in_focus)
        LISTBOX_PaintItem( descr, hdc, &focusRect, descr->focus_item, GXODA_FOCUS, FALSE );

    if (!IS_OWNERDRAW(descr))
    {
        /* Clear the remainder of the client area */
        if (rect.top < descr->height)
        {
            rect.bottom = descr->height;
            gxExtTextOutW( hdc, 0, 0, GXETO_OPAQUE | GXETO_CLIPPED,
                           &rect, NULL, 0, NULL );
        }
        if (rect.right < descr->width)
        {
            rect.left   = rect.right;
            rect.right  = descr->width;
            rect.top    = 0;
            rect.bottom = descr->height;
            gxExtTextOutW( hdc, 0, 0, GXETO_OPAQUE | GXETO_CLIPPED,
                           &rect, NULL, 0, NULL );
        }
    }
    if (oldFont) gxSelectObject( hdc, oldFont );
    if (oldBrush) gxSelectObject( hdc, oldBrush );
    return 0;
}


/***********************************************************************
 *           LISTBOX_InvalidateItems
 *
 * Invalidate all items from a given item. If the specified item is not
 * visible, nothing happens.
 */
static void LISTBOX_InvalidateItems( GXLB_DESCR *descr, GXINT index )
{
    GXRECT rect;

    if (LISTBOX_GetItemRect( descr, index, &rect ) == 1)
    {
        if (descr->style & GXLBS_NOREDRAW)
        {
            descr->style |= GXLBS_DISPLAYCHANGED;
            return;
        }
        rect.bottom = descr->height;
        gxInvalidateRect( descr->self, &rect, TRUE );
        if (descr->style & GXLBS_MULTICOLUMN)
        {
            /* Repaint the other columns */
            rect.left  = rect.right;
            rect.right = descr->width;
            rect.top   = 0;
            gxInvalidateRect( descr->self, &rect, TRUE );
        }
    }
}

static void LISTBOX_InvalidateItemRect( GXLB_DESCR *descr, GXINT index )
{
    GXRECT rect;

    if (LISTBOX_GetItemRect( descr, index, &rect ) == 1)
        gxInvalidateRect( descr->self, &rect, TRUE );
}

/***********************************************************************
 *           LISTBOX_GetItemHeight
 */
static GXLRESULT LISTBOX_GetItemHeight( const GXLB_DESCR *descr, GXINT index )
{
    if (descr->style & GXLBS_OWNERDRAWVARIABLE && descr->nb_items > 0)
    {
        if ((index < 0) || (index >= descr->nb_items))
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        return descr->items[index].height;
    }
    else return descr->item_height;
}


/***********************************************************************
 *           LISTBOX_SetItemHeight
 */
static GXLRESULT LISTBOX_SetItemHeight( GXLB_DESCR *descr, GXINT index, GXINT height, GXBOOL repaint )
{
    if (height > BYTE_MAX)
        return -1;

    if (!height) height = 1;

    if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        if ((index < 0) || (index >= descr->nb_items))
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        LB_TRACE("[%p]: item %d height = %d\n", descr->self, index, height );
        descr->items[index].height = (GXUINT)height;
        LISTBOX_UpdateScroll( descr );
  if (repaint)
      LISTBOX_InvalidateItems( descr, index );
    }
    else if (height != descr->item_height)
    {
        LB_TRACE("[%p]: new height = %d\n", descr->self, height );
        descr->item_height = height;
        LISTBOX_UpdatePage( descr );
        LISTBOX_UpdateScroll( descr );
  if (repaint)
      gxInvalidateRect( descr->self, 0, TRUE );
    }
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_SetHorizontalPos
 */
static void LISTBOX_SetHorizontalPos( GXLB_DESCR *descr, GXINT pos )
{
    GXINT diff;

    if (pos > descr->horz_extent - descr->width)
        pos = descr->horz_extent - descr->width;
    if (pos < 0) pos = 0;
    if (!(diff = descr->horz_pos - pos)) return;
    LB_TRACE("[%p]: new horz pos = %d\n", descr->self, pos );
    descr->horz_pos = pos;
    LISTBOX_UpdateScroll( descr );
    if (abs((int)diff) < descr->width)
    {
        GXRECT rect;
        /* Invalidate the focused item so it will be repainted correctly */
        if (LISTBOX_GetItemRect( descr, descr->focus_item, &rect ) == 1)
            gxInvalidateRect( descr->self, &rect, TRUE );
        gxScrollWindowEx( descr->self, diff, 0, NULL, NULL, 0, NULL,
                          GXSW_INVALIDATE | GXSW_ERASE | GXSW_SCROLLCHILDREN );
    }
    else
        gxInvalidateRect( descr->self, NULL, TRUE );
}


/***********************************************************************
 *           LISTBOX_SetHorizontalExtent
 */
static GXLRESULT LISTBOX_SetHorizontalExtent( GXLB_DESCR *descr, GXINT extent )
{
    if (!descr->horz_extent || (descr->style & GXLBS_MULTICOLUMN))
        return GXLB_OKAY;
    if (extent <= 0) extent = 1;
    if (extent == descr->horz_extent) return GXLB_OKAY;
    LB_TRACE("[%p]: new horz extent = %d\n", descr->self, extent );
    descr->horz_extent = extent;
    if (descr->horz_pos > extent - descr->width)
        LISTBOX_SetHorizontalPos( descr, extent - descr->width );
    else
        LISTBOX_UpdateScroll( descr );
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_SetColumnWidth
 */
static GXLRESULT LISTBOX_SetColumnWidth( GXLB_DESCR *descr, GXINT width)
{
    if (width == descr->column_width) return GXLB_OKAY;
    LB_TRACE("[%p]: new column width = %d\n", descr->self, width );
    descr->column_width = width;
    LISTBOX_UpdatePage( descr );
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_SetFont
 *
 * Returns the item height.
 */
static GXINT LISTBOX_SetFont( GXLB_DESCR *descr, GXHFONT font )
{
    GXHDC hdc;
    GXHFONT oldFont = 0;
    const GXWCHAR *alphabet = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    GXSIZE sz;

    descr->font = font;

    if (!(hdc = gxGetDCEx( descr->self, 0, GXDCX_CACHE )))
    {
        ERR("unable to get DC.\n" );
        return 16;
    }
    if (font) oldFont = (GXHFONT)gxSelectObject( hdc, font );
    gxGetTextExtentPointW( hdc, alphabet, 52, &sz);
    if (oldFont) gxSelectObject( hdc, oldFont );
    gxReleaseDC( descr->self, hdc );

    descr->avg_char_width = (sz.cx / 26 + 1) / 2;
    if (!IS_OWNERDRAW(descr))
        LISTBOX_SetItemHeight( descr, 0, sz.cy, FALSE );
    return sz.cy;
}


/***********************************************************************
 *           LISTBOX_MakeItemVisible
 *
 * Make sure that a given item is partially or fully visible.
 */
static void LISTBOX_MakeItemVisible( GXLB_DESCR *descr, GXINT index, GXBOOL fully )
{
    GXINT top;

    LB_TRACE("current top item %d, index %d, fully %d\n", descr->top_item, index, fully);

    if (index <= descr->top_item) top = index;
    else if (descr->style & GXLBS_MULTICOLUMN)
    {
        GXINT cols = descr->width;
        if (!fully) cols += descr->column_width - 1;
        if (cols >= descr->column_width) cols /= descr->column_width;
        else cols = 1;
        if (index < descr->top_item + (descr->page_size * cols)) return;
        top = index - descr->page_size * (cols - 1);
    }
    else if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        GXINT height = fully ? descr->items[index].height : 1;
        for (top = index; top > descr->top_item; top--)
            if ((height += descr->items[top-1].height) > descr->height) break;
    }
    else
    {
        if (index < descr->top_item + descr->page_size) return;
        if (!fully && (index == descr->top_item + descr->page_size) &&
            (descr->height > (descr->page_size * descr->item_height))) return;
        top = index - descr->page_size + 1;
    }
    LISTBOX_SetTopItem( descr, top, TRUE );
}

/***********************************************************************
 *           LISTBOX_SetCaretIndex
 *
 * NOTES
 *   index must be between 0 and descr->nb_items-1, or LB_ERR is returned.
 *
 */
static GXLRESULT LISTBOX_SetCaretIndex( GXLB_DESCR *descr, GXINT index, GXBOOL fully_visible )
{
    GXINT oldfocus = descr->focus_item;

    LB_TRACE("old focus %d, index %d\n", oldfocus, index);

    if (descr->style & GXLBS_NOSEL) return GXLB_ERR;
    if ((index < 0) || (index >= descr->nb_items)) return GXLB_ERR;
    if (index == oldfocus) return GXLB_OKAY;

    LISTBOX_DrawFocusRect( descr, FALSE );
    descr->focus_item = index;

    LISTBOX_MakeItemVisible( descr, index, fully_visible );
    LISTBOX_DrawFocusRect( descr, TRUE );

    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_SelectItemRange
 *
 * Select a range of items. Should only be used on a MULTIPLESEL listbox.
 */
static GXLRESULT LISTBOX_SelectItemRange( GXLB_DESCR *descr, GXINT first,
                                        GXINT last, GXBOOL on )
{
    GXINT i;

    /* A few sanity checks */

    if (descr->style & GXLBS_NOSEL) return GXLB_ERR;
    if (!(descr->style & GXLBS_MULTIPLESEL)) return GXLB_ERR;

    if (!descr->nb_items) return GXLB_OKAY;

    if (last == -1 || last >= descr->nb_items) last = descr->nb_items - 1;
    if (first < 0) first = 0;
    if (last < first) return GXLB_OKAY;

    if (on)  /* Turn selection on */
    {
        for (i = first; i <= last; i++)
        {
            if (descr->items[i].selected) continue;
            descr->items[i].selected = TRUE;
            LISTBOX_InvalidateItemRect(descr, i);
        }
    }
    else  /* Turn selection off */
    {
        for (i = first; i <= last; i++)
        {
            if (!descr->items[i].selected) continue;
            descr->items[i].selected = FALSE;
            LISTBOX_InvalidateItemRect(descr, i);
        }
    }
    return GXLB_OKAY;
}

/***********************************************************************
 *           LISTBOX_SetSelection
 */
static GXLRESULT LISTBOX_SetSelection( GXLB_DESCR *descr, GXINT index,
                                     GXBOOL on, GXBOOL send_notify )
{
    LB_TRACE( "cur_sel=%d index=%d notify=%s\n",
           descr->selected_item, index, send_notify ? "YES" : "NO" );

    if (descr->style & GXLBS_NOSEL)
    {
        descr->selected_item = index;
        return GXLB_ERR;
    }
    if ((index < -1) || (index >= descr->nb_items)) return GXLB_ERR;
    if (descr->style & GXLBS_MULTIPLESEL)
    {
        if (index == -1)  /* Select all items */
            return LISTBOX_SelectItemRange( descr, 0, descr->nb_items, on );
        else  /* Only one item */
            return LISTBOX_SelectItemRange( descr, index, index, on );
    }
    else
    {
        GXINT oldsel = descr->selected_item;
        if (index == oldsel) return GXLB_OKAY;
        if (oldsel != -1) descr->items[oldsel].selected = FALSE;
        if (index != -1) descr->items[index].selected = TRUE;
        if (oldsel != -1) LISTBOX_RepaintItem( descr, oldsel, GXODA_SELECT );
        descr->selected_item = index;
        if (index != -1) LISTBOX_RepaintItem( descr, index, GXODA_SELECT );
        if (send_notify && descr->nb_items) SEND_NOTIFICATION( descr,
                               (index != -1) ? GXLBN_SELCHANGE : GXLBN_SELCANCEL );
  else
      if( descr->lphc ) /* set selection change flag for parent combo */
    descr->lphc->wState |= CBF_SELCHANGE;
    }
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_MoveCaret
 *
 * Change the caret position and extend the selection to the new caret.
 */
static void LISTBOX_MoveCaret( GXLB_DESCR *descr, GXINT index, GXBOOL fully_visible )
{
    LB_TRACE("old focus %d, index %d\n", descr->focus_item, index);

    if ((index <  0) || (index >= descr->nb_items))
        return;

    /* Important, repaint needs to be done in this order if
       you want to mimic Windows behavior:
       1. Remove the focus and paint the item
       2. Remove the selection and paint the item(s)
       3. Set the selection and repaint the item(s)
       4. Set the focus to 'index' and repaint the item */

    /* 1. remove the focus and repaint the item */
    LISTBOX_DrawFocusRect( descr, FALSE );

    /* 2. then turn off the previous selection */
    /* 3. repaint the new selected item */
    if (descr->style & GXLBS_EXTENDEDSEL)
    {
        if (descr->anchor_item != -1)
        {
            GXINT first = min( index, descr->anchor_item );
            GXINT last  = max( index, descr->anchor_item );
            if (first > 0)
                LISTBOX_SelectItemRange( descr, 0, first - 1, FALSE );
            LISTBOX_SelectItemRange( descr, last + 1, -1, FALSE );
            LISTBOX_SelectItemRange( descr, first, last, TRUE );
        }
    }
    else if (!(descr->style & GXLBS_MULTIPLESEL))
    {
        /* Set selection to new caret item */
        LISTBOX_SetSelection( descr, index, TRUE, FALSE );
    }

    /* 4. repaint the new item with the focus */
    descr->focus_item = index;
    LISTBOX_MakeItemVisible( descr, index, fully_visible );
    LISTBOX_DrawFocusRect( descr, TRUE );
}


/***********************************************************************
 *           LISTBOX_InsertItem
 */
static GXLRESULT LISTBOX_InsertItem( GXLB_DESCR *descr, GXINT index,
                                   GXLPWSTR str, GXULONG_PTR data )
{
    GXLB_ITEMDATA *item;
    GXINT max_items;
    GXINT oldfocus = descr->focus_item;

    if (index == -1) index = descr->nb_items;
    else if ((index < 0) || (index > descr->nb_items)) return GXLB_ERR;
    if (!descr->items) max_items = 0;
    else max_items = (GXINT)gxHeapSize( gxGetProcessHeap(), 0, descr->items ) / sizeof(*item);
    if (descr->nb_items == max_items)
    {
        /* We need to grow the array */
        max_items += LB_ARRAY_GRANULARITY;
  if (descr->items)
          item = (GXLB_ITEMDATA *)gxHeapReAlloc( gxGetProcessHeap(), 0, descr->items,
                                  max_items * sizeof(GXLB_ITEMDATA) );
  else
      item = (GXLB_ITEMDATA *)gxHeapAlloc( gxGetProcessHeap(), 0,
                                  max_items * sizeof(GXLB_ITEMDATA) );
        if (!item)
        {
            SEND_NOTIFICATION( descr, GXLBN_ERRSPACE );
            return GXLB_ERRSPACE;
        }
        descr->items = item;
    }

    /* Insert the item structure */

    item = &descr->items[index];
    if (index < descr->nb_items)
        gxRtlMoveMemory( item + 1, item,
                       (descr->nb_items - index) * sizeof(GXLB_ITEMDATA) );
    item->str      = str;
    item->data     = data;
    item->height   = 0;
    item->selected = FALSE;
    descr->nb_items++;

    /* Get item height */

    if (descr->style & GXLBS_OWNERDRAWVARIABLE)
    {
        GXMEASUREITEMSTRUCT mis;
        GXUINT id = (GXUINT)gxGetWindowLongPtrW( descr->self, GXGWLP_ID );

        mis.CtlType    = GXODT_LISTBOX;
        mis.CtlID      = id;
        mis.itemID     = (GXUINT)index;
        mis.itemData   = descr->items[index].data;
        mis.itemHeight = (GXUINT)descr->item_height;
        gxSendMessageW( descr->owner, GXWM_MEASUREITEM, id, (GXLPARAM)&mis );
        item->height = mis.itemHeight ? mis.itemHeight : 1;
        //LB_TRACE("[%p]: measure item %d (%s) = %d\n",
        //      descr->self, index, str ? debugstr_w(str) : "", item->height );
    }

    /* Repaint the items */

    LISTBOX_UpdateScroll( descr );
    LISTBOX_InvalidateItems( descr, index );

    /* Move selection and focused item */
    /* If listbox was empty, set focus to the first item */
    if (descr->nb_items == 1)
         LISTBOX_SetCaretIndex( descr, 0, FALSE );
    /* single select don't change selection index in win31 */
    else if ((ISWIN31) && !(IS_MULTISELECT(descr)))
    {
        descr->selected_item++;
        LISTBOX_SetSelection( descr, descr->selected_item-1, TRUE, FALSE );
    }
    else
    {
        if (index <= descr->selected_item)
        {
            descr->selected_item++;
            descr->focus_item = oldfocus; /* focus not changed */
        }
    }
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_InsertString
 */
static GXLRESULT LISTBOX_InsertString( GXLB_DESCR *descr, GXINT index, GXLPCWSTR str )
{
    GXLPWSTR new_str = NULL;
    GXULONG_PTR data = 0;
    GXLRESULT ret;

    if (HAS_STRINGS(descr))
    {
        static const GXWCHAR empty_stringW[] = { 0 };
        if (!str) str = empty_stringW;
        if (!(new_str = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (GXSTRLEN(str) + 1) * sizeof(GXWCHAR) )))
        {
            SEND_NOTIFICATION( descr, GXLBN_ERRSPACE );
            return GXLB_ERRSPACE;
        }
        GXSTRCPY(new_str, str);
    }
    else data = (GXULONG_PTR)str;

    if (index == -1) index = descr->nb_items;
    if ((ret = LISTBOX_InsertItem( descr, index, new_str, data )) != 0)
    {
        gxHeapFree( gxGetProcessHeap(), 0, new_str );
        return ret;
    }

    //LB_TRACE("[%p]: added item %d %s\n",
    //      descr->self, index, HAS_STRINGS(descr) ? debugstr_w(new_str) : "" );
    return index;
}


/***********************************************************************
 *           LISTBOX_DeleteItem
 *
 * Delete the content of an item. 'index' must be a valid index.
 */
static void LISTBOX_DeleteItem( GXLB_DESCR *descr, GXINT index )
{
    /* Note: Win 3.1 only sends DELETEITEM on owner-draw items,
     *       while Win95 sends it for all items with user data.
     *       It's probably better to send it too often than not
     *       often enough, so this is what we do here.
     */
    if (IS_OWNERDRAW(descr) || descr->items[index].data)
    {
        GXDELETEITEMSTRUCT dis;
        GXUINT id = (GXUINT)gxGetWindowLongPtrW( descr->self, GXGWLP_ID );

        dis.CtlType  = GXODT_LISTBOX;
        dis.CtlID    = id;
        dis.itemID   = (GXUINT)index;
        dis.hwndItem = (GXHWND)descr->self;
        dis.itemData = descr->items[index].data;
        gxSendMessageW( descr->owner, GXWM_DELETEITEM, id, (GXLPARAM)&dis );
    }
    if (HAS_STRINGS(descr))
        gxHeapFree( gxGetProcessHeap(), 0, descr->items[index].str );
}


/***********************************************************************
 *           LISTBOX_RemoveItem
 *
 * Remove an item from the listbox and delete its content.
 */
static GXLRESULT LISTBOX_RemoveItem( GXLB_DESCR *descr, GXINT index )
{
    GXLB_ITEMDATA *item;
    GXINT max_items;

    if ((index < 0) || (index >= descr->nb_items)) return GXLB_ERR;

    /* We need to invalidate the original rect instead of the updated one. */
    LISTBOX_InvalidateItems( descr, index );

    LISTBOX_DeleteItem( descr, index );

    /* Remove the item */

    item = &descr->items[index];
    if (index < descr->nb_items-1)
        gxRtlMoveMemory( item, item + 1,
                       (descr->nb_items - index - 1) * sizeof(GXLB_ITEMDATA) );
    descr->nb_items--;
    if (descr->anchor_item == descr->nb_items) descr->anchor_item--;

    /* Shrink the item array if possible */

    max_items = (GXINT)gxHeapSize( gxGetProcessHeap(), 0, descr->items ) / sizeof(GXLB_ITEMDATA);
    if (descr->nb_items < max_items - 2*LB_ARRAY_GRANULARITY)
    {
        max_items -= LB_ARRAY_GRANULARITY;
        item = (GXLB_ITEMDATA *)gxHeapReAlloc( gxGetProcessHeap(), 0, descr->items,
                            max_items * sizeof(GXLB_ITEMDATA) );
        if (item) descr->items = item;
    }
    /* Repaint the items */

    LISTBOX_UpdateScroll( descr );
    /* if we removed the scrollbar, reset the top of the list
      (correct for owner-drawn ???) */
    if (descr->nb_items == descr->page_size)
        LISTBOX_SetTopItem( descr, 0, TRUE );

    /* Move selection and focused item */
    if (!IS_MULTISELECT(descr))
    {
        if (index == descr->selected_item)
            descr->selected_item = -1;
        else if (index < descr->selected_item)
        {
            descr->selected_item--;
            if (ISWIN31) /* win 31 do not change the selected item number */
               LISTBOX_SetSelection( descr, descr->selected_item + 1, TRUE, FALSE);
        }
    }

    if (descr->focus_item >= descr->nb_items)
    {
          descr->focus_item = descr->nb_items - 1;
          if (descr->focus_item < 0) descr->focus_item = 0;
    }
    return GXLB_OKAY;
}


/***********************************************************************
 *           LISTBOX_ResetContent
 */
static void LISTBOX_ResetContent( GXLB_DESCR *descr )
{
    GXINT i;

    for(i = descr->nb_items - 1; i>=0; i--) LISTBOX_DeleteItem( descr, i);
    gxHeapFree( gxGetProcessHeap(), 0, descr->items );
    descr->nb_items      = 0;
    descr->top_item      = 0;
    descr->selected_item = -1;
    descr->focus_item    = 0;
    descr->anchor_item   = -1;
    descr->items         = NULL;
}


/***********************************************************************
 *           LISTBOX_SetCount
 */
static GXLRESULT LISTBOX_SetCount( GXLB_DESCR *descr, GXINT count )
{
    GXLRESULT ret;

    if (HAS_STRINGS(descr))
    {
        gxSetLastError(GXERROR_SETCOUNT_ON_BAD_LB);
        return GXLB_ERR;
    }

    /* FIXME: this is far from optimal... */
    if (count > descr->nb_items)
    {
        while (count > descr->nb_items)
            if ((ret = LISTBOX_InsertString( descr, -1, 0 )) < 0)
                return ret;
    }
    else if (count < descr->nb_items)
    {
        while (count < descr->nb_items)
            if ((ret = LISTBOX_RemoveItem( descr, (descr->nb_items - 1) )) < 0)
                return ret;
    }
    return GXLB_OKAY;
}

#if defined(_WIN32) || defined(_WINDOWS)
/***********************************************************************
 *           LISTBOX_Directory
 */
static GXLRESULT LISTBOX_Directory( GXLB_DESCR *descr, GXUINT attrib,
                                  GXLPCWSTR filespec, GXBOOL long_names )
{
    GXHANDLE handle;
    GXLRESULT ret = GXLB_OKAY;
    GXWIN32_FIND_DATAW entry;
    GXINT pos;
    GXLRESULT maxinsert = GXLB_ERR;

    /* don't scan directory if we just want drives exclusively */
    if (attrib != (DDL_DRIVES | DDL_EXCLUSIVE)) {
        /* scan directory */
        if ((handle = gxFindFirstFileW(filespec, &entry)) == INVALID_HANDLE_VALUE)
        {
       int le = GetLastError();
            if ((le != ERROR_NO_MORE_FILES) && (le != ERROR_FILE_NOT_FOUND)) return GXLB_ERR;
        }
        else
        {
            do
            {
                GXWCHAR buffer[270];
                if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    static const GXWCHAR bracketW[]  = { ']',0 };
                    static const GXWCHAR dotW[] = { '.',0 };
                    if (!(attrib & DDL_DIRECTORY) ||
                        !GXSTRCMP( entry.cFileName, dotW )) continue;
                    buffer[0] = '[';
                    if (!long_names && entry.cAlternateFileName[0])
                        GXSTRCPY( buffer + 1, entry.cAlternateFileName );
                    else
                        GXSTRCPY( buffer + 1, entry.cFileName );
                    strcatW(buffer, bracketW);
                }
                else  /* not a directory */
                {
#define ATTRIBS (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | \
                 FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE)

                    if ((attrib & DDL_EXCLUSIVE) &&
                        ((attrib & ATTRIBS) != (entry.dwFileAttributes & ATTRIBS)))
                        continue;
#undef ATTRIBS
                    if (!long_names && entry.cAlternateFileName[0])
                        GXSTRCPY( buffer, entry.cAlternateFileName );
                    else
                        GXSTRCPY( buffer, entry.cFileName );
                }
                if (!long_names) CharLowerW( buffer );
                pos = LISTBOX_FindFileStrPos( descr, buffer );
                if ((ret = LISTBOX_InsertString( descr, pos, buffer )) < 0)
                    break;
                if (ret <= maxinsert) maxinsert++; else maxinsert = ret;
            } while (gxFindNextFileW( handle, &entry ));
            gxFindClose( handle );
        }
    }
    if (ret >= 0)
    {
        ret = maxinsert;

        /* scan drives */
        if (attrib & DDL_DRIVES)
        {
            GXWCHAR buffer[] = {'[','-','a','-',']',0};
            GXWCHAR root[] = {'A',':','\\',0};
            int drive;
            for (drive = 0; drive < 26; drive++, buffer[2]++, root[0]++)
            {
                if (GetDriveTypeW(root) <= DRIVE_NO_ROOT_DIR) continue;
                if ((ret = LISTBOX_InsertString( descr, -1, buffer )) < 0)
                    break;
            }
        }
    }
    return ret;
}
#endif // defined(_WIN32) || defined(_WINDOWS)

/***********************************************************************
 *           LISTBOX_HandleVScroll
 */
static GXLRESULT LISTBOX_HandleVScroll( GXLB_DESCR *descr, GXWORD scrollReq, GXWORD pos )
{
    GXSCROLLINFO info;

    if (descr->style & GXLBS_MULTICOLUMN) return 0;
    switch(scrollReq)
    {
    case GXSB_LINEUP:
        LISTBOX_SetTopItem( descr, descr->top_item - 1, TRUE );
        break;
    case GXSB_LINEDOWN:
        LISTBOX_SetTopItem( descr, descr->top_item + 1, TRUE );
        break;
    case GXSB_PAGEUP:
        LISTBOX_SetTopItem( descr, descr->top_item -
                            LISTBOX_GetCurrentPageSize( descr ), TRUE );
        break;
    case GXSB_PAGEDOWN:
        LISTBOX_SetTopItem( descr, descr->top_item +
                            LISTBOX_GetCurrentPageSize( descr ), TRUE );
        break;
    case GXSB_THUMBPOSITION:
        LISTBOX_SetTopItem( descr, pos, TRUE );
        break;
    case GXSB_THUMBTRACK:
        info.cbSize = sizeof(info);
        info.fMask = GXSIF_TRACKPOS;
        gxGetScrollInfo( descr->self, GXSB_VERT, &info );
        LISTBOX_SetTopItem( descr, info.nTrackPos, TRUE );
        break;
    case GXSB_TOP:
        LISTBOX_SetTopItem( descr, 0, TRUE );
        break;
    case GXSB_BOTTOM:
        LISTBOX_SetTopItem( descr, descr->nb_items, TRUE );
        break;
    }
    return 0;
}


/***********************************************************************
 *           LISTBOX_HandleHScroll
 */
static GXLRESULT LISTBOX_HandleHScroll( GXLB_DESCR *descr, GXWORD scrollReq, GXWORD pos )
{
    GXSCROLLINFO info;
    GXINT page;

    if (descr->style & GXLBS_MULTICOLUMN)
    {
        switch(scrollReq)
        {
        case GXSB_LINELEFT:
            LISTBOX_SetTopItem( descr, descr->top_item-descr->page_size,
                                TRUE );
            break;
        case GXSB_LINERIGHT:
            LISTBOX_SetTopItem( descr, descr->top_item+descr->page_size,
                                TRUE );
            break;
        case GXSB_PAGELEFT:
            page = descr->width / descr->column_width;
            if (page < 1) page = 1;
            LISTBOX_SetTopItem( descr,
                             descr->top_item - page * descr->page_size, TRUE );
            break;
        case GXSB_PAGERIGHT:
            page = descr->width / descr->column_width;
            if (page < 1) page = 1;
            LISTBOX_SetTopItem( descr,
                             descr->top_item + page * descr->page_size, TRUE );
            break;
        case GXSB_THUMBPOSITION:
            LISTBOX_SetTopItem( descr, pos*descr->page_size, TRUE );
            break;
        case GXSB_THUMBTRACK:
            info.cbSize = sizeof(info);
            info.fMask  = GXSIF_TRACKPOS;
            gxGetScrollInfo( descr->self, GXSB_VERT, &info );
            LISTBOX_SetTopItem( descr, info.nTrackPos*descr->page_size,
                                TRUE );
            break;
        case GXSB_LEFT:
            LISTBOX_SetTopItem( descr, 0, TRUE );
            break;
        case GXSB_RIGHT:
            LISTBOX_SetTopItem( descr, descr->nb_items, TRUE );
            break;
        }
    }
    else if (descr->horz_extent)
    {
        switch(scrollReq)
        {
        case GXSB_LINELEFT:
            LISTBOX_SetHorizontalPos( descr, descr->horz_pos - 1 );
            break;
        case GXSB_LINERIGHT:
            LISTBOX_SetHorizontalPos( descr, descr->horz_pos + 1 );
            break;
        case GXSB_PAGELEFT:
            LISTBOX_SetHorizontalPos( descr,
                                      descr->horz_pos - descr->width );
            break;
        case GXSB_PAGERIGHT:
            LISTBOX_SetHorizontalPos( descr,
                                      descr->horz_pos + descr->width );
            break;
        case GXSB_THUMBPOSITION:
            LISTBOX_SetHorizontalPos( descr, pos );
            break;
        case GXSB_THUMBTRACK:
            info.cbSize = sizeof(info);
            info.fMask = GXSIF_TRACKPOS;
            gxGetScrollInfo( descr->self, GXSB_HORZ, &info );
            LISTBOX_SetHorizontalPos( descr, info.nTrackPos );
            break;
        case GXSB_LEFT:
            LISTBOX_SetHorizontalPos( descr, 0 );
            break;
        case GXSB_RIGHT:
            LISTBOX_SetHorizontalPos( descr,
                                      descr->horz_extent - descr->width );
            break;
        }
    }
    return 0;
}

static GXLRESULT LISTBOX_HandleMouseWheel(GXLB_DESCR *descr, GXSHORT delta )
{
    short gcWheelDelta = 0;
    GXUINT pulScrollLines = 3;

    gxSystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0, &pulScrollLines, 0);

    gcWheelDelta -= delta;

    if (abs(gcWheelDelta) >= GXWHEEL_DELTA && pulScrollLines)
    {
        int cLineScroll = (int) min((GXUINT) descr->page_size, pulScrollLines);
        cLineScroll *= (gcWheelDelta / GXWHEEL_DELTA);
        LISTBOX_SetTopItem( descr, descr->top_item + cLineScroll, TRUE );
    }
    return 0;
}

/***********************************************************************
 *           LISTBOX_HandleLButtonDown
 */
static GXLRESULT LISTBOX_HandleLButtonDown( GXLB_DESCR *descr, GXDWORD keys, GXINT x, GXINT y )
{
    GXINT index = LISTBOX_GetItemFromPoint( descr, x, y );

    LB_TRACE("[%p]: lbuttondown %d,%d item %d, focus item %d\n",
          descr->self, x, y, index, descr->focus_item);

    if (!descr->caret_on && (descr->in_focus)) return 0;

    if (!descr->in_focus)
    {
        if( !descr->lphc ) gxSetFocus( descr->self );
        else gxSetFocus((GXHWND)((descr->lphc->hWndEdit) ? descr->lphc->hWndEdit : descr->lphc->self) );
    }

    if (index == -1) return 0;

    if (!descr->lphc)
    {
        if (descr->style & GXLBS_NOTIFY )
            gxSendMessageW( descr->owner, GXWM_LBTRACKPOINT, index,
                            GXMAKELPARAM( x, y ) );
    }

    descr->captured = TRUE;
    gxSetCapture( descr->self );

    if (descr->style & (GXLBS_EXTENDEDSEL | GXLBS_MULTIPLESEL))
    {
        /* we should perhaps make sure that all items are deselected
           FIXME: needed for !GXLBS_EXTENDEDSEL, too ?
           if (!(keys & (MK_SHIFT|MK_CONTROL)))
           LISTBOX_SetSelection( descr, -1, FALSE, FALSE);
        */

        if (!(keys & GXMK_SHIFT)) descr->anchor_item = index;
        if (keys & GXMK_CONTROL)
        {
            LISTBOX_SetCaretIndex( descr, index, FALSE );
            LISTBOX_SetSelection( descr, index,
                                  !descr->items[index].selected,
                                  (descr->style & GXLBS_NOTIFY) != 0);
        }
        else
        {
            LISTBOX_MoveCaret( descr, index, FALSE );

            if (descr->style & GXLBS_EXTENDEDSEL)
            {
                LISTBOX_SetSelection( descr, index,
                               descr->items[index].selected,
                              (descr->style & GXLBS_NOTIFY) != 0 );
            }
            else
            {
                LISTBOX_SetSelection( descr, index,
                               !descr->items[index].selected,
                              (descr->style & GXLBS_NOTIFY) != 0 );
            }
        }
    }
    else
    {
        descr->anchor_item = index;
        LISTBOX_MoveCaret( descr, index, FALSE );
        LISTBOX_SetSelection( descr, index,
                              TRUE, (descr->style & GXLBS_NOTIFY) != 0 );
    }

    if (!descr->lphc)
    {
        if (gxGetWindowLongW( descr->self, GXGWL_EXSTYLE ) & GXWS_EX_DRAGDETECT)
        {
            GXPOINT pt;

      pt.x = x;
      pt.y = y;

            if (gxDragDetect( descr->self, &pt ))
                gxSendMessageW( descr->owner, GXWM_BEGINDRAG, 0, 0 );
        }
    }
    return 0;
}


/*************************************************************************
 * LISTBOX_HandleLButtonDownCombo [Internal]
 *
 * Process LButtonDown message for the ComboListBox
 *
 * PARAMS
 *     pWnd       [I] The windows internal structure
 *     pDescr     [I] The ListBox internal structure
 *     keys       [I] Key Flag (GXWM_LBUTTONDOWN doc for more info)
 *     x          [I] X Mouse Coordinate
 *     y          [I] Y Mouse Coordinate
 *
 * RETURNS
 *     0 since we are processing the WM_LBUTTONDOWN Message
 *
 * NOTES
 *  This function is only to be used when a ListBox is a ComboListBox
 */

static GXLRESULT LISTBOX_HandleLButtonDownCombo( GXLB_DESCR *descr, GXUINT msg, GXDWORD keys, GXINT x, GXINT y)
{
    GXRECT clientRect, screenRect;
    GXPOINT mousePos;

    mousePos.x = x;
    mousePos.y = y;

    gxGetClientRect(descr->self, &clientRect);

    if(gxPtInRect(&clientRect, mousePos))
    {
       /* MousePos is in client, resume normal processing */
        if (msg == GXWM_LBUTTONDOWN)
        {
           descr->lphc->droppedIndex = descr->nb_items ? descr->selected_item : -1;
           return LISTBOX_HandleLButtonDown( descr, keys, x, y);
        }
        else if (descr->style & GXLBS_NOTIFY)
            SEND_NOTIFICATION( descr, GXLBN_DBLCLK );
    }
    else
    {
        GXPOINT screenMousePos;
        GXHWND hWndOldCapture;

        /* Check the Non-Client Area */
        screenMousePos = mousePos;
        hWndOldCapture = gxGetCapture();
        gxReleaseCapture();
        gxGetWindowRect(descr->self, &screenRect);
        gxClientToScreen(descr->self, &screenMousePos);

        if(!gxPtInRect(&screenRect, screenMousePos))
        {
            LISTBOX_SetCaretIndex( descr, descr->lphc->droppedIndex, FALSE );
            LISTBOX_SetSelection( descr, descr->lphc->droppedIndex, FALSE, FALSE );
            COMBO_FlipListbox( descr->lphc, FALSE, FALSE );
        }
        else
        {
            /* Check to see the NC is a scrollbar */
            GXINT nHitTestType=0;
            GXLONG style = gxGetWindowLongW( descr->self, GXGWL_STYLE );
            /* Check Vertical scroll bar */
            if (style & GXWS_VSCROLL)
            {
                clientRect.right += gxGetSystemMetrics(GXSM_CXVSCROLL);
                if (gxPtInRect( &clientRect, mousePos ))
                    nHitTestType = GXHTVSCROLL;
            }
              /* Check horizontal scroll bar */
            if (style & GXWS_HSCROLL)
            {
                clientRect.bottom += gxGetSystemMetrics(GXSM_CYHSCROLL);
                if (gxPtInRect( &clientRect, mousePos ))
                    nHitTestType = GXHTHSCROLL;
            }
            /* Windows sends this message when a scrollbar is clicked
             */

            if(nHitTestType != 0)
            {
                gxSendMessageW(descr->self, GXWM_NCLBUTTONDOWN, nHitTestType,
                             GXMAKELONG(screenMousePos.x, screenMousePos.y));
            }
            /* Resume the Capture after scrolling is complete
             */
            if(hWndOldCapture != 0)
                gxSetCapture(hWndOldCapture);
        }
    }
    return 0;
}

/***********************************************************************
 *           LISTBOX_HandleLButtonUp
 */
static GXLRESULT LISTBOX_HandleLButtonUp( GXLB_DESCR *descr )
{
    if (LISTBOX_Timer != LB_TIMER_NONE)
        KillSystemTimer( descr->self, LB_TIMER_ID );
    LISTBOX_Timer = LB_TIMER_NONE;
    if (descr->captured)
    {
        descr->captured = FALSE;
        if (gxGetCapture() == descr->self) gxReleaseCapture();
        if ((descr->style & GXLBS_NOTIFY) && descr->nb_items)
            SEND_NOTIFICATION( descr, GXLBN_SELCHANGE );
    }
    return 0;
}


/***********************************************************************
 *           LISTBOX_HandleTimer
 *
 * Handle scrolling upon a timer event.
 * Return TRUE if scrolling should continue.
 */
static GXLRESULT LISTBOX_HandleTimer( GXLB_DESCR *descr, GXINT index, TIMER_DIRECTION dir )
{
    switch(dir)
    {
    case LB_TIMER_UP:
        if (descr->top_item) index = descr->top_item - 1;
        else index = 0;
        break;
    case LB_TIMER_LEFT:
        if (descr->top_item) index -= descr->page_size;
        break;
    case LB_TIMER_DOWN:
        index = descr->top_item + LISTBOX_GetCurrentPageSize( descr );
        if (index == descr->focus_item) index++;
        if (index >= descr->nb_items) index = descr->nb_items - 1;
        break;
    case LB_TIMER_RIGHT:
        if (index + descr->page_size < descr->nb_items)
            index += descr->page_size;
        break;
    case LB_TIMER_NONE:
        break;
    }
    if (index == descr->focus_item) return FALSE;
    LISTBOX_MoveCaret( descr, index, FALSE );
    return TRUE;
}


/***********************************************************************
 *           LISTBOX_HandleSystemTimer
 *
 * WM_SYSTIMER handler.
 */
static GXLRESULT LISTBOX_HandleSystemTimer( GXLB_DESCR *descr )
{
    if (!LISTBOX_HandleTimer( descr, descr->focus_item, LISTBOX_Timer ))
    {
        KillSystemTimer( descr->self, LB_TIMER_ID );
        LISTBOX_Timer = LB_TIMER_NONE;
    }
    return 0;
}


/***********************************************************************
 *           LISTBOX_HandleMouseMove
 *
 * WM_MOUSEMOVE handler.
 */
static void LISTBOX_HandleMouseMove( GXLB_DESCR *descr,
                                     GXINT x, GXINT y )
{
    GXINT index;
    TIMER_DIRECTION dir = LB_TIMER_NONE;

    if (!descr->captured) return;

    if (descr->style & GXLBS_MULTICOLUMN)
    {
        if (y < 0) y = 0;
        else if (y >= descr->item_height * descr->page_size)
            y = descr->item_height * descr->page_size - 1;

        if (x < 0)
        {
            dir = LB_TIMER_LEFT;
            x = 0;
        }
        else if (x >= descr->width)
        {
            dir = LB_TIMER_RIGHT;
            x = descr->width - 1;
        }
    }
    else
    {
        if (y < 0) dir = LB_TIMER_UP;  /* above */
        else if (y >= descr->height) dir = LB_TIMER_DOWN;  /* below */
    }

    index = LISTBOX_GetItemFromPoint( descr, x, y );
    if (index == -1) index = descr->focus_item;
    if (!LISTBOX_HandleTimer( descr, index, dir )) dir = LB_TIMER_NONE;

    /* Start/stop the system timer */

    if (dir != LB_TIMER_NONE)
        SetSystemTimer( descr->self, LB_TIMER_ID, LB_SCROLL_TIMEOUT, NULL);
    else if (LISTBOX_Timer != LB_TIMER_NONE)
        KillSystemTimer( descr->self, LB_TIMER_ID );
    LISTBOX_Timer = dir;
}


/***********************************************************************
 *           LISTBOX_HandleKeyDown
 */
static GXLRESULT LISTBOX_HandleKeyDown( GXLB_DESCR *descr, GXDWORD key )
{
    GXINT caret = -1;
    GXBOOL bForceSelection = TRUE; /* select item pointed to by focus_item */
    if ((IS_MULTISELECT(descr)) || (descr->selected_item == descr->focus_item))
        bForceSelection = FALSE; /* only for single select list */

    if (descr->style & GXLBS_WANTKEYBOARDINPUT)
    {
        caret = gxSendMessageW( descr->owner, GXWM_VKEYTOITEM,
                                GXMAKEWPARAM(GXLOWORD(key), descr->focus_item),
                                (GXLPARAM)descr->self );
        if (caret == -2) return 0;
    }
    if (caret == -1) switch(key)
    {
    case GXVK_LEFT:
        if (descr->style & GXLBS_MULTICOLUMN)
        {
            bForceSelection = FALSE;
            if (descr->focus_item >= descr->page_size)
                caret = descr->focus_item - descr->page_size;
            break;
        }
        /* fall through */
    case GXVK_UP:
        caret = descr->focus_item - 1;
        if (caret < 0) caret = 0;
        break;
    case GXVK_RIGHT:
        if (descr->style & GXLBS_MULTICOLUMN)
        {
            bForceSelection = FALSE;
            if (descr->focus_item + descr->page_size < descr->nb_items)
                caret = descr->focus_item + descr->page_size;
            break;
        }
        /* fall through */
    case GXVK_DOWN:
        caret = descr->focus_item + 1;
        if (caret >= descr->nb_items) caret = descr->nb_items - 1;
        break;

    case GXVK_PRIOR:
        if (descr->style & GXLBS_MULTICOLUMN)
        {
            GXINT page = descr->width / descr->column_width;
            if (page < 1) page = 1;
            caret = descr->focus_item - (page * descr->page_size) + 1;
        }
        else caret = descr->focus_item-LISTBOX_GetCurrentPageSize(descr) + 1;
        if (caret < 0) caret = 0;
        break;
    case GXVK_NEXT:
        if (descr->style & GXLBS_MULTICOLUMN)
        {
            GXINT page = descr->width / descr->column_width;
            if (page < 1) page = 1;
            caret = descr->focus_item + (page * descr->page_size) - 1;
        }
        else caret = descr->focus_item + LISTBOX_GetCurrentPageSize(descr) - 1;
        if (caret >= descr->nb_items) caret = descr->nb_items - 1;
        break;
    case GXVK_HOME:
        caret = 0;
        break;
    case GXVK_END:
        caret = descr->nb_items - 1;
        break;
    case GXVK_SPACE:
        if (descr->style & GXLBS_EXTENDEDSEL) caret = descr->focus_item;
        else if (descr->style & GXLBS_MULTIPLESEL)
        {
            LISTBOX_SetSelection( descr, descr->focus_item,
                                  !descr->items[descr->focus_item].selected,
                                  (descr->style & GXLBS_NOTIFY) != 0 );
        }
        break;
    default:
        bForceSelection = FALSE;
    }
    if (bForceSelection) /* focused item is used instead of key */
        caret = descr->focus_item;
    if (caret >= 0)
    {
        if (((descr->style & GXLBS_EXTENDEDSEL) &&
            !(gxGetKeyState( GXVK_SHIFT ) & 0x8000)) ||
            !IS_MULTISELECT(descr))
            descr->anchor_item = caret;
        LISTBOX_MoveCaret( descr, caret, TRUE );

        if (descr->style & GXLBS_MULTIPLESEL)
            descr->selected_item = caret;
        else
            LISTBOX_SetSelection( descr, caret, TRUE, FALSE);
        if (descr->style & GXLBS_NOTIFY)
        {
            if( descr->lphc )
            {
                /* make sure that combo parent doesn't hide us */
                descr->lphc->wState |= CBF_NOROLLUP;
            }
            if (descr->nb_items) SEND_NOTIFICATION( descr, GXLBN_SELCHANGE );
        }
    }
    return 0;
}


/***********************************************************************
 *           LISTBOX_HandleChar
 */
static GXLRESULT LISTBOX_HandleChar( GXLB_DESCR *descr, GXWCHAR charW )
{
    GXINT caret = -1;
    GXWCHAR str[2];

    str[0] = charW;
    str[1] = '\0';

    if (descr->style & GXLBS_WANTKEYBOARDINPUT)
    {
        caret = gxSendMessageW( descr->owner, GXWM_CHARTOITEM,
                                GXMAKEWPARAM(charW, descr->focus_item),
                                (GXLPARAM)descr->self );
        if (caret == -2) return 0;
    }
    if (caret == -1)
        caret = LISTBOX_FindString( descr, descr->focus_item, str, FALSE);
    if (caret != -1)
    {
        if ((!IS_MULTISELECT(descr)) && descr->selected_item == -1)
           LISTBOX_SetSelection( descr, caret, TRUE, FALSE);
        LISTBOX_MoveCaret( descr, caret, TRUE );
        if ((descr->style & GXLBS_NOTIFY) && descr->nb_items)
            SEND_NOTIFICATION( descr, GXLBN_SELCHANGE );
    }
    return 0;
}


/***********************************************************************
 *           LISTBOX_Create
 */
static GXBOOL LISTBOX_Create( GXHWND hwnd, GXLPHEADCOMBO lphc )
{
    GXLB_DESCR *descr;
    GXMEASUREITEMSTRUCT mis;
    GXRECT rect;

    if (!(descr = (GXLB_DESCR*)gxHeapAlloc( gxGetProcessHeap(), 0, sizeof(*descr) )))
        return FALSE;

    gxGetClientRect( hwnd, &rect );
    descr->self          = hwnd;
    descr->owner         = gxGetParent( descr->self );
    descr->style         = (GXUINT)gxGetWindowLongW( descr->self, GXGWL_STYLE );
    descr->width         = rect.right - rect.left;
    descr->height        = rect.bottom - rect.top;
    descr->items         = NULL;
    descr->nb_items      = 0;
    descr->top_item      = 0;
    descr->selected_item = -1;
    descr->focus_item    = 0;
    descr->anchor_item   = -1;
    descr->item_height   = 1;
    descr->page_size     = 1;
    descr->column_width  = 150;
    descr->horz_extent   = (descr->style & GXWS_HSCROLL) ? 1 : 0;
    descr->horz_pos      = 0;
    descr->nb_tabs       = 0;
    descr->tabs          = NULL;
    descr->caret_on      = lphc ? FALSE : TRUE;
    if (descr->style & GXLBS_NOSEL) descr->caret_on = FALSE;
    descr->in_focus    = FALSE;
    descr->captured      = FALSE;
    descr->font          = 0;
    descr->locale        = gxGetUserDefaultLCID();
    descr->lphc     = lphc;

    if (is_old_app(descr) && ( descr->style & ( GXWS_VSCROLL | GXWS_HSCROLL ) ) )
    {
  /* Win95 document "List Box Differences" from MSDN:
     If a list box in a version 3.x application has either the
     WS_HSCROLL or WS_VSCROLL style, the list box receives both
     horizontal and vertical scroll bars.
  */
  descr->style |= GXWS_VSCROLL | GXWS_HSCROLL;
    }

    if( lphc )
    {
        LB_TRACE("[%p]: resetting owner %p -> %p\n", descr->self, descr->owner, lphc->self );
        descr->owner = (GXHWND)lphc->self;
    }

    gxSetWindowLongPtrW( descr->self, 0, (GXLONG_PTR)descr );

/*    if (wnd->dwExStyle & WS_EX_NOPARENTNOTIFY) descr->style &= ~GXLBS_NOTIFY;
 */
    if (descr->style & GXLBS_EXTENDEDSEL) descr->style |= GXLBS_MULTIPLESEL;
    if (descr->style & GXLBS_MULTICOLUMN) descr->style &= ~GXLBS_OWNERDRAWVARIABLE;
    if (descr->style & GXLBS_OWNERDRAWVARIABLE) descr->style |= GXLBS_NOINTEGRALHEIGHT;
    descr->item_height = LISTBOX_SetFont( descr, 0 );

    if (descr->style & GXLBS_OWNERDRAWFIXED)
    {
  if( descr->lphc && (descr->lphc->dwStyle & GXCBS_DROPDOWN))
  {
      /* WinWord gets VERY unhappy if we send WM_MEASUREITEM from here */
    descr->item_height = lphc->fixedOwnerDrawHeight;
  }
  else
  {
            GXUINT id = (GXUINT)gxGetWindowLongPtrW( descr->self, GXGWLP_ID );
            mis.CtlType    = GXODT_LISTBOX;
            mis.CtlID      = id;
            mis.itemID     = -1;
            mis.itemWidth  =  0;
            mis.itemData   =  0;
            mis.itemHeight = (GXUINT)descr->item_height;
            gxSendMessageW( descr->owner, GXWM_MEASUREITEM, id, (GXLPARAM)&mis );
            descr->item_height = mis.itemHeight ? mis.itemHeight : 1;
  }
    }

    LB_TRACE("owner: %p, style: %08x, width: %d, height: %d\n", descr->owner, descr->style, descr->width, descr->height);
    return TRUE;
}


/***********************************************************************
 *           LISTBOX_Destroy
 */
static GXBOOL LISTBOX_Destroy( GXLB_DESCR *descr )
{
    LISTBOX_ResetContent( descr );
    gxSetWindowLongPtrW( descr->self, 0, 0 );
    gxHeapFree( gxGetProcessHeap(), 0, descr );
    return TRUE;
}


/***********************************************************************
 *           ListBoxWndProc_common
 */
static GXLRESULT GXAPI ListBoxWndProc_common( GXHWND hwnd, GXUINT msg,
                                             GXWPARAM wParam, GXLPARAM lParam, GXBOOL unicode )
{
    GXLB_DESCR *descr = (GXLB_DESCR *)gxGetWindowLongPtrW( hwnd, 0 );
    GXLPHEADCOMBO lphc = 0;
    GXLRESULT ret;

    if (!descr)
    {
        if (!gxIsWindow(hwnd)) return 0;

        if (msg == GXWM_CREATE)
        {
      GXCREATESTRUCTW *lpcs = (GXCREATESTRUCTW *)lParam;
      if (lpcs->style & GXLBS_COMBOBOX) lphc = (GXLPHEADCOMBO)lpcs->lpCreateParams;
            if (!LISTBOX_Create( hwnd, lphc )) return -1;
            LB_TRACE("creating wnd=%p descr=%x\n", hwnd, gxGetWindowLongPtrW( hwnd, 0 ) );
            return 0;
        }
        /* Ignore all other messages before we get a WM_CREATE */
        return unicode ? gxDefWindowProcW( hwnd, msg, wParam, lParam ) :
                         gxDefWindowProcA( hwnd, msg, wParam, lParam );
    }
    if (descr->style & GXLBS_COMBOBOX) lphc = descr->lphc;

    //LB_TRACE("[%p]: msg %s wp %08lx lp %08lx\n",
    //      descr->self, SPY_GetMsgName(msg, descr->self), wParam, lParam );

    switch(msg)
    {
    //case LB_RESETCONTENT16:
    case GXLB_RESETCONTENT:
        LISTBOX_ResetContent( descr );
        LISTBOX_UpdateScroll( descr );
        gxInvalidateRect( descr->self, NULL, TRUE );
        return 0;

    //case LB_ADDSTRING16:
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_ADDSTRINGW:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        wParam = LISTBOX_FindStringPos( descr, textW, FALSE );
        ret = LISTBOX_InsertString( descr, wParam, textW );
        if (!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }

    //case GXLB_INSERTSTRING16:
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    wParam = (GXINT)(INT16)wParam;
    //    /* fall through */
    case GXLB_INSERTSTRINGW:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        ret = LISTBOX_InsertString( descr, wParam, textW );
        if(!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }

    //case GXLB_ADDFILE16:
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_ADDFILE:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        wParam = LISTBOX_FindFileStrPos( descr, textW );
        ret = LISTBOX_InsertString( descr, wParam, textW );
        if(!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }

    //case GXLB_DELETESTRING16:
    case GXLB_DELETESTRING:
        if (LISTBOX_RemoveItem( descr, wParam) != GXLB_ERR)
            return descr->nb_items;
        else
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }

//     case GXLB_GETITEMDATA16:
    case GXLB_GETITEMDATA:
        if (((GXINT)wParam < 0) || ((GXINT)wParam >= descr->nb_items))
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        return descr->items[wParam].data;

//     case GXLB_SETITEMDATA16:
    case GXLB_SETITEMDATA:
        if (((GXINT)wParam < 0) || ((GXINT)wParam >= descr->nb_items))
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        descr->items[wParam].data = lParam;
        /* undocumented: returns TRUE, not GXLB_OKAY (0) */
        return TRUE;

    //case GXLB_GETCOUNT16:
    case GXLB_GETCOUNT:
        return descr->nb_items;

    //case GXLB_GETTEXT16:
    //    lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_GETTEXT:
        return LISTBOX_GetText( descr, wParam, (GXLPWSTR)lParam, unicode );

    //case GXLB_GETTEXTLEN16:
    //    /* fall through */
    case GXLB_GETTEXTLEN:
        if ((GXINT)wParam >= descr->nb_items || (GXINT)wParam < 0)
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        if (!HAS_STRINGS(descr)) return sizeof(GXDWORD);
        if (unicode) return GXSTRLEN( descr->items[wParam].str );
        return gxWideCharToMultiByte( GXCP_ACP, 0, descr->items[wParam].str,
                                    GXSTRLEN(descr->items[wParam].str), NULL, 0, NULL, NULL );

    //case GXLB_GETCURSEL16:
    case GXLB_GETCURSEL:
        if (descr->nb_items == 0)
            return GXLB_ERR;
        if (!IS_MULTISELECT(descr))
            return descr->selected_item;
        if (descr->selected_item != -1)
            return descr->selected_item;
        return descr->focus_item;
        /* otherwise, if the user tries to move the selection with the    */
        /* arrow keys, we will give the application something to choke on */
    //case GXLB_GETTOPINDEX16:
    case GXLB_GETTOPINDEX:
        return descr->top_item;

    //case GXLB_GETITEMHEIGHT16:
    case GXLB_GETITEMHEIGHT:
        return LISTBOX_GetItemHeight( descr, wParam );

    //case GXLB_SETITEMHEIGHT16:
    //    lParam = GXLOWORD(lParam);
    //    /* fall through */
    case GXLB_SETITEMHEIGHT:
        return LISTBOX_SetItemHeight( descr, wParam, lParam, TRUE );

    case GXLB_ITEMFROMPOINT:
        {
            GXPOINT pt;
            GXRECT rect;
            GXINT index;
            GXBOOL hit = TRUE;

            /* The hiword of the return value is not a client area
               hittest as suggested by MSDN, but rather a hittest on
               the returned listbox item. */

            if(descr->nb_items == 0)
                return 0x1ffff;      /* win9x returns 0x10000, we copy winnt */

            pt.x = (short)GXLOWORD(lParam);
            pt.y = (short)GXHIWORD(lParam);

            gxSetRect(&rect, 0, 0, descr->width, descr->height);

            if(!gxPtInRect(&rect, pt))
            {
                pt.x = min(pt.x, rect.right - 1);
                pt.x = max(pt.x, 0);
                pt.y = min(pt.y, rect.bottom - 1);
                pt.y = max(pt.y, 0);
                hit = FALSE;
            }

            index = LISTBOX_GetItemFromPoint(descr, pt.x, pt.y);

            if(index == -1)
            {
                index = descr->nb_items - 1;
                hit = FALSE;
            }
            return GXMAKELONG(index, hit ? 0 : 1);
        }

    //case GXLB_SETCARETINDEX16:
    case GXLB_SETCARETINDEX:
        if ((!IS_MULTISELECT(descr)) && (descr->selected_item != -1)) return GXLB_ERR;
        if (LISTBOX_SetCaretIndex( descr, wParam, !lParam ) == GXLB_ERR)
            return GXLB_ERR;
        else if (ISWIN31)
             return wParam;
        else
             return GXLB_OKAY;

    //case GXLB_GETCARETINDEX16:
    case GXLB_GETCARETINDEX:
        return descr->focus_item;

    //case GXLB_SETTOPINDEX16:
    case GXLB_SETTOPINDEX:
        return LISTBOX_SetTopItem( descr, wParam, TRUE );

    //case GXLB_SETCOLUMNWIDTH16:
    case GXLB_SETCOLUMNWIDTH:
        return LISTBOX_SetColumnWidth( descr, wParam );

 //   case GXLB_GETITEMRECT16:
 //       {
 //           GXRECT rect;
 //           RECT16 *r16 = MapSL(lParam);
 //           ret = LISTBOX_GetItemRect( descr, (INT16)wParam, &rect );
 //           r16->left   = rect.left;
 //           r16->top    = rect.top;
 //           r16->right  = rect.right;
 //           r16->bottom = rect.bottom;
 //       }
  //return ret;

    case GXLB_GETITEMRECT:
        return LISTBOX_GetItemRect( descr, wParam, (GXRECT *)lParam );

    //case GXLB_FINDSTRING16:
    //    wParam = (GXINT)(INT16)wParam;
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_FINDSTRING:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        ret = LISTBOX_FindString( descr, wParam, textW, FALSE );
        if(!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }

    //case GXLB_FINDSTRINGEXACT16:
    //    wParam = (GXINT)(INT16)wParam;
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_FINDSTRINGEXACT:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        ret = LISTBOX_FindString( descr, wParam, textW, TRUE );
        if(!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }

    //case GXLB_SELECTSTRING16:
    //    wParam = (GXINT)(INT16)wParam;
    //    if (HAS_STRINGS(descr)) lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
    case GXLB_SELECTSTRING:
    {
        GXINT index;
        GXLPWSTR textW;

  if(HAS_STRINGS(descr))
//       GXLB_TRACE("GXLB_SELECTSTRING: %s\n", unicode ? debugstr_w((GXLPWSTR)lParam) :
//                  debugstr_a((GXLPSTR)lParam));
        if(unicode || !HAS_STRINGS(descr))
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        index = LISTBOX_FindString( descr, wParam, textW, FALSE );
        if(!unicode && HAS_STRINGS(descr))
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        if (index != GXLB_ERR)
  {
            LISTBOX_MoveCaret( descr, index, TRUE );
            LISTBOX_SetSelection( descr, index, TRUE, FALSE );
  }
        return index;
    }

    //case GXLB_GETSEL16:
    //    wParam = (GXINT)(INT16)wParam;
    //    /* fall through */
    case GXLB_GETSEL:
        if (((GXINT)wParam < 0) || ((GXINT)wParam >= descr->nb_items))
            return GXLB_ERR;
        return descr->items[wParam].selected;

    //case GXLB_SETSEL16:
    //    lParam = (GXINT)(INT16)lParam;
    //    /* fall through */
    case GXLB_SETSEL:
        return LISTBOX_SetSelection( descr, lParam, (GXBOOL)wParam, FALSE );

    //case GXLB_SETCURSEL16:
    //    wParam = (GXINT)(INT16)wParam;
    //    /* fall through */
    case GXLB_SETCURSEL:
        if (IS_MULTISELECT(descr)) return GXLB_ERR;
        LISTBOX_SetCaretIndex( descr, wParam, FALSE );
        ret = LISTBOX_SetSelection( descr, wParam, TRUE, FALSE );
  if (ret != GXLB_ERR) ret = descr->selected_item;
  return ret;

    //case GXLB_GETSELCOUNT16:
    case GXLB_GETSELCOUNT:
        return LISTBOX_GetSelCount( descr );

    //case GXLB_GETSELITEMS16:
    //    return LISTBOX_GetSelItems16( descr, wParam, (GXLPINT16)MapSL(lParam) );

    case GXLB_GETSELITEMS:
        return LISTBOX_GetSelItems( descr, wParam, (GXINT*)lParam );

    //case GXLB_SELITEMRANGE16:
    case GXLB_SELITEMRANGE:
        if (GXLOWORD(lParam) <= GXHIWORD(lParam))
            return LISTBOX_SelectItemRange( descr, GXLOWORD(lParam),
                                            GXHIWORD(lParam), (GXBOOL)wParam );
        else
            return LISTBOX_SelectItemRange( descr, GXHIWORD(lParam),
                                            GXLOWORD(lParam), (GXBOOL)wParam );

    //case GXLB_SELITEMRANGEEX16:
    case GXLB_SELITEMRANGEEX:
        if ((GXINT)lParam >= (GXINT)wParam)
            return LISTBOX_SelectItemRange( descr, wParam, lParam, TRUE );
        else
            return LISTBOX_SelectItemRange( descr, lParam, wParam, FALSE);

    //case GXLB_GETHORIZONTALEXTENT16:
    case GXLB_GETHORIZONTALEXTENT:
        return descr->horz_extent;

    //case GXLB_SETHORIZONTALEXTENT16:
    case GXLB_SETHORIZONTALEXTENT:
        return LISTBOX_SetHorizontalExtent( descr, wParam );

    //case GXLB_GETANCHORINDEX16:
    case GXLB_GETANCHORINDEX:
        return descr->anchor_item;

    //case GXLB_SETANCHORINDEX16:
    //    wParam = (GXINT)(INT16)wParam;
    //    /* fall through */
    case GXLB_SETANCHORINDEX:
        if (((GXINT)wParam < -1) || ((GXINT)wParam >= descr->nb_items))
        {
            gxSetLastError(GXERROR_INVALID_INDEX);
            return GXLB_ERR;
        }
        descr->anchor_item = (GXINT)wParam;
        return GXLB_OKAY;

    //case GXLB_DIR16:
    //    /* according to Win16 docs, DDL_DRIVES should make DDL_EXCLUSIVE
    //     * be set automatically (this is different in Win32) */
    //    if (wParam & DDL_DRIVES) wParam |= DDL_EXCLUSIVE;
    //        lParam = (GXLPARAM)MapSL(lParam);
    //    /* fall through */
#if defined(_WIN32) || defined(_WINDOWS)
    case GXLB_DIR:
    {
        GXINT ret;
        GXLPWSTR textW;
        if(unicode)
            textW = (GXLPWSTR)lParam;
        else
        {
            GXLPSTR textA = (GXLPSTR)lParam;
            GXINT countW = gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, NULL, 0);
            if((textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (int)countW * sizeof(GXWCHAR))))
                gxMultiByteToWideChar(GXCP_ACP, 0, textA, -1, textW, (int)countW);
        }
        ret = LISTBOX_Directory( descr, wParam, textW, msg == GXLB_DIR );
        if(!unicode)
            gxHeapFree(gxGetProcessHeap(), 0, textW);
        return ret;
    }
#endif // defined(_WIN32) || defined(_WINDOWS)
    case GXLB_GETLOCALE:
        return descr->locale;

    case GXLB_SETLOCALE:
    {
        GXLCID ret;
        if (!gxIsValidLocale((GXLCID)wParam, GXLCID_INSTALLED))
            return GXLB_ERR;
        ret = descr->locale;
        descr->locale = (GXLCID)wParam;
        return ret;
    }

    case GXLB_INITSTORAGE:
        return LISTBOX_InitStorage( descr, wParam );

    case GXLB_SETCOUNT:
        return LISTBOX_SetCount( descr, (GXINT)wParam );

    //case LB_SETTABSTOPS16:
    //    return LISTBOX_SetTabStops( descr, (GXINT)(INT16)wParam, MapSL(lParam), TRUE );

    case GXLB_SETTABSTOPS:
        return LISTBOX_SetTabStops( descr, wParam, (GXINT*)lParam, FALSE );

    //case GXLB_CARETON16:
    case GXLB_CARETON:
        if (descr->caret_on)
            return GXLB_OKAY;
        descr->caret_on = TRUE;
        if ((descr->focus_item != -1) && (descr->in_focus))
            LISTBOX_RepaintItem( descr, descr->focus_item, GXODA_FOCUS );
        return GXLB_OKAY;

    //case GXLB_CARETOFF16:
    case GXLB_CARETOFF:
        if (!descr->caret_on)
            return GXLB_OKAY;
        descr->caret_on = FALSE;
        if ((descr->focus_item != -1) && (descr->in_focus))
            LISTBOX_RepaintItem( descr, descr->focus_item, GXODA_FOCUS );
        return GXLB_OKAY;

    case GXLB_GETLISTBOXINFO:
        FIXME("GXLB_GETLISTBOXINFO: stub!\n");
        return 0;

    case GXWM_DESTROY:
        return LISTBOX_Destroy( descr );

    case GXWM_ENABLE:
        gxInvalidateRect( descr->self, NULL, TRUE );
        return 0;

    case GXWM_SETREDRAW:
        LISTBOX_SetRedraw( descr, wParam != 0 );
        return 0;

    case GXWM_GETDLGCODE:
        return GXDLGC_WANTARROWS | GXDLGC_WANTCHARS;

    case GXWM_PRINTCLIENT:
    case GXWM_PAINT:
        {
            GXPAINTSTRUCT ps;
            GXHDC hdc = ( wParam ) ? ((GXHDC)wParam) :  gxBeginPaint( descr->self, &ps );
            ret = LISTBOX_Paint( descr, hdc );
            if( !wParam ) gxEndPaint( descr->self, &ps );
        }
        return ret;
    case GXWM_SIZE:
        LISTBOX_UpdateSize( descr );
        return 0;
    case GXWM_GETFONT:
        return (GXLRESULT)descr->font;
    case GXWM_SETFONT:
        LISTBOX_SetFont( descr, (GXHFONT)wParam );
        if (lParam) gxInvalidateRect( descr->self, 0, TRUE );
        return 0;
    case GXWM_SETFOCUS:
        descr->in_focus = TRUE;
        descr->caret_on = TRUE;
        if (descr->focus_item != -1)
            LISTBOX_DrawFocusRect( descr, TRUE );
        SEND_NOTIFICATION( descr, GXLBN_SETFOCUS );
        return 0;
    case GXWM_KILLFOCUS:
        descr->in_focus = FALSE;
        if ((descr->focus_item != -1) && descr->caret_on)
            LISTBOX_RepaintItem( descr, descr->focus_item, GXODA_FOCUS );
        SEND_NOTIFICATION( descr, GXLBN_KILLFOCUS );
        return 0;
    case GXWM_HSCROLL:
        return LISTBOX_HandleHScroll( descr, GXLOWORD(wParam), GXHIWORD(wParam) );
    case GXWM_VSCROLL:
        return LISTBOX_HandleVScroll( descr, GXLOWORD(wParam), GXHIWORD(wParam) );
    case GXWM_MOUSEWHEEL:
        if (wParam & (GXMK_SHIFT | GXMK_CONTROL))
            return gxDefWindowProcW( descr->self, msg, wParam, lParam );
        return LISTBOX_HandleMouseWheel( descr, (GXSHORT)GXHIWORD(wParam) );
    case GXWM_LBUTTONDOWN:
  if (lphc)
            return LISTBOX_HandleLButtonDownCombo(descr, msg, wParam,
                                                  (GXINT16)GXLOWORD(lParam),
                                                  (GXINT16)GXHIWORD(lParam) );
        return LISTBOX_HandleLButtonDown( descr, wParam,
                                          (GXINT16)GXLOWORD(lParam),
                                          (GXINT16)GXHIWORD(lParam) );
    case GXWM_LBUTTONDBLCLK:
  if (lphc)
            return LISTBOX_HandleLButtonDownCombo(descr, msg, wParam,
                                                  (GXINT16)GXLOWORD(lParam),
                                                  (GXINT16)GXHIWORD(lParam) );
        if (descr->style & GXLBS_NOTIFY)
            SEND_NOTIFICATION( descr, GXLBN_DBLCLK );
        return 0;
    case GXWM_MOUSEMOVE:
        if ( lphc && ((lphc->dwStyle & GXCBS_DROPDOWNLIST) != GXCBS_SIMPLE) )
        {
            GXBOOL    captured = descr->captured;
            GXPOINT   mousePos;
            GXRECT    clientRect;

            mousePos.x = (GXINT16)GXLOWORD(lParam);
            mousePos.y = (GXINT16)GXHIWORD(lParam);

            /*
             * If we are in a dropdown combobox, we simulate that
             * the mouse is captured to show the tracking of the item.
             */
            if (gxGetClientRect(descr->self, &clientRect) && gxPtInRect( &clientRect, mousePos ))
                descr->captured = TRUE;

            LISTBOX_HandleMouseMove( descr, mousePos.x, mousePos.y);

            descr->captured = captured;
        } 
        else if (gxGetCapture() == descr->self)
        {
            LISTBOX_HandleMouseMove( descr, (GXINT16)GXLOWORD(lParam),
                                     (GXINT16)GXHIWORD(lParam) );
        }
        return 0;
    case GXWM_LBUTTONUP:
  if (lphc)
  {
            GXPOINT mousePos;
            GXRECT  clientRect;

            /*
             * If the mouse button "up" is not in the listbox,
             * we make sure there is no selection by re-selecting the
             * item that was selected when the listbox was made visible.
             */
            mousePos.x = (GXINT16)GXLOWORD(lParam);
            mousePos.y = (GXINT16)GXHIWORD(lParam);

            gxGetClientRect(descr->self, &clientRect);

            /*
             * When the user clicks outside the combobox and the focus
             * is lost, the owning combobox will send a fake buttonup with
             * 0xFFFFFFF as the mouse location, we must also revert the
             * selection to the original selection.
             */
            if ( (lParam == (GXLPARAM)-1) || (!gxPtInRect( &clientRect, mousePos )) )
                LISTBOX_MoveCaret( descr, lphc->droppedIndex, FALSE );
        }
        return LISTBOX_HandleLButtonUp( descr );
    case GXWM_KEYDOWN:
        if( lphc && (lphc->dwStyle & GXCBS_DROPDOWNLIST) != GXCBS_SIMPLE )
        {
            /* for some reason Windows makes it possible to
             * show/hide ComboLBox by sending it WM_KEYDOWNs */

            if( (!(lphc->wState & CBF_EUI) && wParam == GXVK_F4) ||
                ( (lphc->wState & CBF_EUI) && !(lphc->wState & CBF_DROPPED)
                  && (wParam == GXVK_DOWN || wParam == GXVK_UP)) )
            {
                COMBO_FlipListbox( lphc, FALSE, FALSE );
                return 0;
            }
        }
        return LISTBOX_HandleKeyDown( descr, wParam );
    case GXWM_CHAR:
    {
        GXWCHAR charW;
        if(unicode)
            charW = (GXWCHAR)wParam;
        else
        {
            GXCHAR charA = (GXCHAR)wParam;
            gxMultiByteToWideChar(GXCP_ACP, 0, &charA, 1, &charW, 1);
        }
        return LISTBOX_HandleChar( descr, charW );
    }
    case GXWM_SYSTIMER:
        return LISTBOX_HandleSystemTimer( descr );
    case GXWM_ERASEBKGND:
        if ((IS_OWNERDRAW(descr)) && !(descr->style & GXLBS_DISPLAYCHANGED))
        {
            GXRECT rect;
            GXHBRUSH hbrush = (GXHBRUSH)gxSendMessageW( descr->owner, GXWM_CTLCOLORLISTBOX,
                                              wParam, (GXLPARAM)descr->self );
      LB_TRACE("hbrush = %p\n", hbrush);
      if(!hbrush)
    hbrush = gxGetSysColorBrush(GXCOLOR_WINDOW);
      if(hbrush)
      {
    gxGetClientRect(descr->self, &rect);
    gxFillRect((GXHDC)wParam, &rect, hbrush);
      }
        }
        return 1;
    case GXWM_DROPFILES:
        if( lphc ) return 0;
        return unicode ? gxSendMessageW( descr->owner, msg, wParam, lParam ) :
                         gxSendMessageA( descr->owner, msg, wParam, lParam );

    case GXWM_NCDESTROY:
        if( lphc && (lphc->dwStyle & GXCBS_DROPDOWNLIST) != GXCBS_SIMPLE )
            lphc->hWndLBox = 0;
        break;

    case GXWM_NCACTIVATE:
        if (lphc) return 0;
  break;

    default:
        if ((msg >= GXWM_USER) && (msg < 0xc000))
            WARN("[%p]: unknown msg %04x wp %08lx lp %08lx\n",
                 hwnd, msg, wParam, lParam );
    }

    return unicode ? gxDefWindowProcW( hwnd, msg, wParam, lParam ) :
                     gxDefWindowProcA( hwnd, msg, wParam, lParam );
}

/***********************************************************************
 *           ListBoxWndProcA
 */
GXLRESULT GXDLLAPI ListBoxWndProcA( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam )
{
    return ListBoxWndProc_common( hwnd, msg, wParam, lParam, FALSE );
}

/***********************************************************************
 *           ListBoxWndProcW
 */
GXLRESULT GXDLLAPI ListBoxWndProcW( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam )
{
    return ListBoxWndProc_common( hwnd, msg, wParam, lParam, TRUE );
}
#endif // _DEV_DISABLE_UI_CODE