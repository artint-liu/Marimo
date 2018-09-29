#ifndef _DEV_DISABLE_UI_CODE
/*
* Drag List control
*
* Copyright 1999 Eric Kohl
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
* of Comctl32.dll version 6.0 on Mar. 10, 2004, by Robert Shearman.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features or bugs please note them below.
* 
*/
//
//#include <stdarg.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "winnls.h"
//#include "commctrl.h"
//#include "comctl32.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(commctrl);

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
#include "res/resource.h"

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

#define DRAGLIST_SUBCLASSID     0
#define DRAGLIST_SCROLLPERIOD 200
#define DRAGLIST_TIMERID      666

/* properties relating to IDI_DRAGICON */
#define DRAGICON_HOTSPOT_X 17
#define DRAGICON_HOTSPOT_Y  7
#define DRAGICON_HEIGHT    32

/* internal Wine specific data for the drag list control */
typedef struct _DRAGLISTDATA
{
  /* are we currently in dragging mode? */
  GXBOOL dragging;

  /* cursor to use as determined by DL_DRAGGING notification.
  * NOTE: as we use gxLoadCursor we don't have to use DeleteCursor
  * when we are finished with it */
  GXHCURSOR cursor;

  /* optimisation so that we don't have to load the cursor
  * all of the time whilst dragging */
  GXLRESULT last_dragging_response;

  /* prevents flicker with drawing drag arrow */
  GXRECT last_drag_icon_rect;
} DRAGLISTDATA;

GXUINT uDragListMessage = 0; /* registered window message code */
static GXDWORD dwLastScrollTime = 0;
static GXHICON hDragArrow = NULL;

/***********************************************************************
*    DragList_Notify (internal)
*
* Sends notification messages to the parent control. Note that it
* does not use GXWM_NOTIFY like the rest of the controls, but a registered
* window message.
*/
static GXLRESULT DragList_Notify(GXHWND hwndLB, GXUINT uNotification)
{
  GXDRAGLISTINFO dli;
  dli.hWnd = hwndLB;
  dli.uNotification = uNotification;
  gxGetCursorPos(&dli.ptCursor);
  return gxSendMessageW(gxGetParent(hwndLB), uDragListMessage, gxGetDlgCtrlID(hwndLB), (GXLPARAM)&dli);
}

/* cleans up after dragging */
static void DragList_EndDrag(GXHWND hwnd, DRAGLISTDATA * data)
{
  gxKillTimer(hwnd, DRAGLIST_TIMERID);
  gxReleaseCapture();
  /* clear any drag insert icon present */
  gxInvalidateRect(gxGetParent(hwnd), &data->last_drag_icon_rect, TRUE);
  /* clear data for next use */
  memset(data, 0, sizeof(*data));
}

/***********************************************************************
*    DragList_SubclassWindowProc (internal)
*
* Handles certain messages to enable dragging for the ListBox and forwards
* the rest to the ListBox.
*/
static GXLRESULT CALLBACK
DragList_SubclassWindowProc(GXHWND hwnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam, GXUINT_PTR uIdSubclass, GXDWORD_PTR dwRefData)
{
  DRAGLISTDATA * data = (DRAGLISTDATA*)dwRefData;
  switch (uMsg)
  {
  case GXWM_LBUTTONDOWN:
    gxSetFocus(hwnd);
    data->dragging = (GXBOOL)DragList_Notify(hwnd, GXDL_BEGINDRAG);
    if (data->dragging)
    {
      gxSetCapture(hwnd);
      gxSetTimer(hwnd, DRAGLIST_TIMERID, DRAGLIST_SCROLLPERIOD, NULL);
    }
    /* note that we don't absorb this message to let the list box
    * do its thing (normally selecting an item) */
    break;

  case GXWM_KEYDOWN:
  case GXWM_RBUTTONDOWN:
    /* user cancelled drag by either right clicking or
    * by pressing the escape key */
    if ((data->dragging) &&
      ((uMsg == GXWM_RBUTTONDOWN) || (wParam == GXVK_ESCAPE)))
    {
      /* clean up and absorb message */
      DragList_EndDrag(hwnd, data);
      DragList_Notify(hwnd, GXDL_CANCELDRAG);
      return 0;
    }
    break;

  case GXWM_MOUSEMOVE:
  case GXWM_TIMER:
    if (data->dragging)
    {
      GXLRESULT cursor = DragList_Notify(hwnd, GXDL_DRAGGING);
      /* optimisation so that we don't have to load the cursor
      * all of the time whilst dragging */
      if (data->last_dragging_response != cursor)
      {
        switch (cursor)
        {
        case GXDL_STOPCURSOR:
          data->cursor = gxLoadCursorW(NULL, (GXLPCWSTR)GXIDC_NO);
          gxSetCursor(data->cursor);
          break;
        case GXDL_COPYCURSOR:
          data->cursor = gxLoadCursorW(COMCTL32_hModule, (GXLPCWSTR)IDC_COPY);
          gxSetCursor(data->cursor);
          break;
        case GXDL_MOVECURSOR:
          data->cursor = gxLoadCursorW(NULL, (GXLPCWSTR)GXIDC_ARROW);
          gxSetCursor(data->cursor);
          break;
        }
        data->last_dragging_response = cursor;
      }
      /* don't pass this message on to List Box */
      return 0;
    }
    break;

  case GXWM_LBUTTONUP:
    if (data->dragging)
    {
      DragList_EndDrag(hwnd, data);
      DragList_Notify(hwnd, GXDL_DROPPED);
    }
    break;

  case GXWM_GETDLGCODE:
    /* tell dialog boxes that we want to receive GXWM_KEYDOWN events
    * for keys like GXVK_ESCAPE */
    if (data->dragging)
      return GXDLGC_WANTALLKEYS;
    break;
  case GXWM_NCDESTROY:
    gxRemoveWindowSubclass(hwnd, DragList_SubclassWindowProc, DRAGLIST_SUBCLASSID);
    Free(data);
    break;
  }
  return gxDefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/***********************************************************************
*    MakeDragList (COMCTL32.13)
*
* Makes a normal ListBox into a DragList by subclassing it.
*
* RETURNS
*      Success: Non-zero
*      Failure: Zero
*/
GXBOOL GXDLLAPI gxMakeDragList (GXHWND hwndLB)
{
  DRAGLISTDATA *data = (DRAGLISTDATA*)Alloc(sizeof(DRAGLISTDATA));

  TRACE("(%p)\n", hwndLB);

  if (!uDragListMessage)
    uDragListMessage = gxRegisterWindowMessageW(GXDRAGLISTMSGSTRING);

  return gxSetWindowSubclass(hwndLB, DragList_SubclassWindowProc, DRAGLIST_SUBCLASSID, (GXDWORD_PTR)data);
}

/***********************************************************************
*    DrawInsert (COMCTL32.15)
*
* Draws insert arrow by the side of the ListBox item in the parent window.
*
* RETURNS
*      Nothing.
*/
GXVOID GXDLLAPI gxDrawInsert (GXHWND hwndParent, GXHWND hwndLB, GXINT nItem)
{
  GXRECT rcItem, rcListBox, rcDragIcon;
  GXHDC hdc;
  DRAGLISTDATA * data;

  TRACE("(%p %p %d)\n", hwndParent, hwndLB, nItem);

  if (!hDragArrow)
    hDragArrow = gxLoadIconW(COMCTL32_hModule, (GXLPCWSTR)IDI_DRAGARROW);

  if (GXLB_ERR == gxSendMessageW(hwndLB, GXLB_GETITEMRECT, nItem, (GXLPARAM)&rcItem))
    return;

  if (!gxGetWindowRect(hwndLB, &rcListBox))
    return;

  /* convert item rect to parent co-ordinates */
  if (!gxMapWindowPoints(hwndLB, hwndParent, (LPGXPOINT)&rcItem, 2))
    return;

  /* convert list box rect to parent co-ordinates */
  if (!gxMapWindowPoints(GXHWND_DESKTOP, hwndParent, (LPGXPOINT)&rcListBox, 2))
    return;

  rcDragIcon.left = rcListBox.left - DRAGICON_HOTSPOT_X;
  rcDragIcon.top = rcItem.top - DRAGICON_HOTSPOT_Y;
  rcDragIcon.right = rcListBox.left;
  rcDragIcon.bottom = rcDragIcon.top + DRAGICON_HEIGHT;

  if (!gxGetWindowSubclass(hwndLB, DragList_SubclassWindowProc, DRAGLIST_SUBCLASSID, (GXDWORD_PTR*)&data))
    return;

  if (nItem < 0)
    gxSetRectEmpty(&rcDragIcon);

  /* prevent flicker by only redrawing when necessary */
  if (!gxEqualRect(&rcDragIcon, &data->last_drag_icon_rect))
  {
    /* get rid of any previous inserts drawn */
    gxRedrawWindow(hwndParent, &data->last_drag_icon_rect, NULL,
      GXRDW_INTERNALPAINT | GXRDW_ERASE | GXRDW_INVALIDATE | GXRDW_UPDATENOW);

    gxCopyRect(&data->last_drag_icon_rect, &rcDragIcon);

    if (nItem >= 0)
    {
      hdc = gxGetDC(hwndParent);

      gxDrawIcon(hdc, (int)rcDragIcon.left, (int)rcDragIcon.top, hDragArrow);

      gxReleaseDC(hwndParent, hdc);
    }
  }
}

/***********************************************************************
*    LBItemFromPt (COMCTL32.14)
*
* Gets the index of the ListBox item under the specified point,
* scrolling if bAutoScroll is TRUE and pt is outside of the ListBox.
*
* RETURNS
*      The ListBox item ID if pt is over a list item or -1 otherwise.
*/
GXINT GXDLLAPI gxLBItemFromPt (GXHWND hwndLB, GXPOINT pt, GXBOOL bAutoScroll)
{
  GXRECT rcClient;
  GXINT nIndex;
  GXDWORD dwScrollTime;

  TRACE("(%p %ld x %ld %s)\n", hwndLB, pt.x, pt.y, bAutoScroll ? "TRUE" : "FALSE");

  gxScreenToClient (hwndLB, &pt);
  gxGetClientRect (hwndLB, &rcClient);
  nIndex = (GXINT)gxSendMessageW (hwndLB, GXLB_GETTOPINDEX, 0, 0);

  if (gxPtInRect (&rcClient, pt))
  {
    /* point is inside -- get the item index */
    while (TRUE)
    {
      if (gxSendMessageW (hwndLB, GXLB_GETITEMRECT, nIndex, (GXLPARAM)&rcClient) == GXLB_ERR)
        return -1;

      if (gxPtInRect (&rcClient, pt))
        return nIndex;

      nIndex++;
    }
  }
  else
  {
    /* point is outside */
    if (!bAutoScroll)
      return -1;

    if ((pt.x > rcClient.right) || (pt.x < rcClient.left))
      return -1;

    if (pt.y < 0)
      nIndex--;
    else
      nIndex++;

    dwScrollTime = gxGetTickCount ();

    if ((dwScrollTime - dwLastScrollTime) < DRAGLIST_SCROLLPERIOD)
      return -1;

    dwLastScrollTime = dwScrollTime;

    gxSendMessageW (hwndLB, GXLB_SETTOPINDEX, (GXWPARAM)nIndex, 0);
  }

  return -1;
}

#endif // _DEV_DISABLE_UI_CODE