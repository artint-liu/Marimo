/*
* Combo controls
*
* Copyright 1997 Alex Korobka
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
* TODO:
*   - ComboBox_[GS]etMinVisible()
*   - CB_GETMINVISIBLE, CB_SETMINVISIBLE
*   - CB_SETTOPINDEX
*/

#include <GrapX.H>
#include "Include/GXUser.H"
#include "Include/GXGDI.H"
#include "Include/GXImm.h"
#include "Include/GXKernel.H"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <User/WineComm.H>

#define OEMRESOURCE

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "winuser.h"
//#include "wine/unicode.h"
//#include "user_private.h"
//#include "win.h"
//#include "controls.h"
//#include "winternl.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(combo);

/* bits in the dwKeyData */
#define KEYDATA_ALT             0x2000
#define KEYDATA_PREVSTATE       0x4000

/* combo box */
#define ID_CB_LISTBOX           1000
#define ID_CB_EDIT              1001

/* internal flags */
#define CBF_DROPPED             0x0001
#define CBF_BUTTONDOWN          0x0002
#define CBF_NOROLLUP            0x0004
#define CBF_MEASUREITEM         0x0008
#define CBF_FOCUSED             0x0010
#define CBF_CAPTURE             0x0020
#define CBF_EDIT                0x0040
#define CBF_NORESIZE            0x0080
#define CBF_NOTIFY              0x0100
#define CBF_NOREDRAW            0x0200
#define CBF_SELCHANGE           0x0400
#define CBF_NOEDITNOTIFY        0x1000
#define CBF_NOLBSELECT          0x2000  /* do not change current selection */
#define CBF_BEENFOCUSED         0x4000  /* has it ever had focus           */
#define CBF_EUI                 0x8000


/*
* Additional combo box definitions
*/

#define CB_NOTIFY( lphc, code ) \
  (gxSendMessageW((lphc)->owner, WM_COMMAND, \
  MAKEWPARAM(gxGetWindowLongPtrW((lphc)->self,GWLP_ID), (code)), (LPARAM)(lphc)->self))

#define CB_DISABLED( lphc )   (!gxIsWindowEnabled((lphc)->self))
#define CB_OWNERDRAWN( lphc ) ((lphc)->dwStyle & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))
#define CB_HASSTRINGS( lphc ) ((lphc)->dwStyle & CBS_HASSTRINGS)
#define CB_HWND( lphc )       ((lphc)->self)
#define CB_GETTYPE( lphc )    ((lphc)->dwStyle & (CBS_DROPDOWNLIST))

#define ISWIN31 (LOWORD(GetVersion()) == 0x0a03)

/*
* Drawing globals
*/
static GXHBITMAP 	hComboBmp = 0;
static GXUINT	CBitHeight, CBitWidth;

/*
* Look and feel dependent "constants"
*/

#define COMBO_YBORDERGAP         5
#define COMBO_XBORDERSIZE()      2
#define COMBO_YBORDERSIZE()      2
#define COMBO_EDITBUTTONSPACE()  0
#define EDIT_CONTROL_PADDING()   1

/*********************************************************************
* combo class descriptor
*/
static const GXWCHAR comboboxW[] = {'C','o','m','b','o','B','o','x',0};
//const GXWNDCLASSEX COMBO_builtin_class =
//{
//    comboboxW,            /* name */
//    GXCS_PARENTDC | GXCS_DBLCLKS | GXCS_HREDRAW | GXCS_VREDRAW, /* style  */
//    WINPROC_COMBO,        /* proc */
//    sizeof(HEADCOMBO *),  /* extra */
//    IDC_ARROW,            /* cursor */
//    0                     /* brush */
//};


/***********************************************************************
*           set_control_clipping
*
* Set clipping for a builtin control that uses CS_PARENTDC.
* Return the previous clip region if any.
*/
GXHRGN set_control_clipping( GXHDC hdc, const GXRECT *rect )
{
  GXRECT rc = *rect;
  GXHRGN hrgn = gxCreateRectRgn( 0, 0, 0, 0 );

  if (gxGetClipRgn( hdc, hrgn ) != 1)
  {
    gxDeleteObject( hrgn );
    hrgn = 0;
  }
  gxDPtoLP( hdc, (GXPOINT *)&rc, 2 );
  if (gxGetLayout( hdc ) & LAYOUT_RTL)  /* compensate for the shifting done by IntersectClipRect */
  {
    rc.left++;
    rc.right++;
  }
  gxIntersectClipRect( hdc, rc.left, rc.top, rc.right, rc.bottom );
  return hrgn;
}

/***********************************************************************
*           COMBO_Init
*
* Load combo button bitmap.
*/
static GXBOOL COMBO_Init(void)
{
  GXHDC		hDC;

  if( hComboBmp ) return TRUE;
  if( (hDC = gxCreateCompatibleDC(0)) )
  {
    GXBOOL	bRet = FALSE;
    if( (hComboBmp = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_COMBO))) )
    {
      GXBITMAP      bm;
      GXHBITMAP     hPrevB;
      GXRECT        r;

      gxGetObjectW( hComboBmp, sizeof(bm), &bm );
      CBitHeight = bm.bmHeight;
      CBitWidth  = bm.bmWidth;

      TRACE("combo bitmap [%i,%i]\n", CBitWidth, CBitHeight );

      hPrevB = (GXHBITMAP)gxSelectObject( hDC, hComboBmp);
      gxSetRect( &r, 0, 0, CBitWidth, CBitHeight );
      gxInvertRect( hDC, &r );
      gxSelectObject( hDC, hPrevB );
      bRet = TRUE;
    }
    gxDeleteDC( hDC );
    return bRet;
  }
  return FALSE;
}

/***********************************************************************
*           COMBO_NCCreate
*/
static LRESULT COMBO_NCCreate(GXHWND hwnd, LONG style)
{
  GXLPHEADCOMBO lphc;

  if (COMBO_Init() && (lphc = (GXLPHEADCOMBO)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, sizeof(GXHEADCOMBO))) )
  {
    lphc->self = hwnd;
    gxSetWindowLongPtrW( hwnd, 0, (LONG_PTR)lphc );

    /* some braindead apps do try to use scrollbar/border flags */

    lphc->dwStyle = style & ~(GXWS_BORDER | WS_HSCROLL | GXWS_VSCROLL);
    gxSetWindowLongW( hwnd, GWL_STYLE, style & ~(GXWS_BORDER | WS_HSCROLL | GXWS_VSCROLL) );

    /*
    * We also have to remove the client edge style to make sure
    * we don't end-up with a non client area.
    */
    gxSetWindowLongW( hwnd, GWL_EXSTYLE,
      gxGetWindowLongW( hwnd, GWL_EXSTYLE ) & ~WS_EX_CLIENTEDGE );

    if( !(style & (GXCBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) )
      lphc->dwStyle |= CBS_HASSTRINGS;
    if( !(gxGetWindowLongW( hwnd, GWL_EXSTYLE ) & WS_EX_NOPARENTNOTIFY) )
      lphc->wState |= CBF_NOTIFY;

    TRACE("[%p], style = %08x\n", lphc, lphc->dwStyle );
    return TRUE;
  }
  return FALSE;
}

/***********************************************************************
*           COMBO_NCDestroy
*/
static LRESULT COMBO_NCDestroy( GXLPHEADCOMBO lphc )
{

  if( lphc )
  {
    TRACE("[%p]: freeing storage\n", lphc->self);

    if( (CB_GETTYPE(lphc) != CBS_SIMPLE) && lphc->hWndLBox )
      gxDestroyWindow( lphc->hWndLBox );

    gxSetWindowLongPtrW( lphc->self, 0, 0 );
    gxHeapFree( gxGetProcessHeap(), 0, lphc );
  }
  return 0;
}

/***********************************************************************
*           CBGetTextAreaHeight
*
* This method will calculate the height of the text area of the
* combobox.
* The height of the text area is set in two ways.
* It can be set explicitly through a combobox message or through a
* WM_MEASUREITEM callback.
* If this is not the case, the height is set to font height + 4px
* This height was determined through experimentation.
* CBCalcPlacement will add 2*COMBO_YBORDERSIZE pixels for the border
*/
static GXINT CBGetTextAreaHeight(
  GXHWND        hwnd,
  GXLPHEADCOMBO lphc)
{
  GXINT iTextItemHeight;

  if( lphc->editHeight ) /* explicitly set height */
  {
    iTextItemHeight = lphc->editHeight;
  }
  else
  {
    GXTEXTMETRICW tm;
    GXHDC       hDC = gxGetDC(hwnd);
    GXHFONT       hPrevFont = 0;
    GXINT         baseUnitY;

    if (lphc->hFont)
      hPrevFont = (GXHFONT)gxSelectObject( hDC, lphc->hFont );

    gxGetTextMetricsW(hDC, &tm);

    baseUnitY = tm.tmHeight;

    if( hPrevFont )
      gxSelectObject( hDC, hPrevFont );

    gxReleaseDC(hwnd, hDC);

    iTextItemHeight = baseUnitY + 4;
  }

  /*
  * Check the ownerdraw case if we haven't asked the parent the size
  * of the item yet.
  */
  if ( CB_OWNERDRAWN(lphc) &&
    (lphc->wState & CBF_MEASUREITEM) )
  {
    MEASUREITEMSTRUCT measureItem;
    GXRECT              clientRect;
    GXINT               originalItemHeight = iTextItemHeight;
    GXUINT id = (GXUINT)gxGetWindowLongPtrW( lphc->self, GWLP_ID );

    /*
    * We use the client rect for the width of the item.
    */
    gxGetClientRect(hwnd, &clientRect);

    lphc->wState &= ~CBF_MEASUREITEM;

    /*
    * Send a first one to measure the size of the text area
    */
    measureItem.CtlType    = GXODT_COMBOBOX;
    measureItem.CtlID      = id;
    measureItem.itemID     = -1;
    measureItem.itemWidth  = clientRect.right;
    measureItem.itemHeight = iTextItemHeight - 6; /* ownerdrawn cb is taller */
    measureItem.itemData   = 0;
    gxSendMessageW(lphc->owner, WM_MEASUREITEM, id, (LPARAM)&measureItem);
    iTextItemHeight = 6 + measureItem.itemHeight;

    /*
    * Send a second one in the case of a fixed ownerdraw list to calculate the
    * size of the list items. (we basically do this on behalf of the listbox)
    */
    if (lphc->dwStyle & GXCBS_OWNERDRAWFIXED)
    {
      measureItem.CtlType    = GXODT_COMBOBOX;
      measureItem.CtlID      = id;
      measureItem.itemID     = 0;
      measureItem.itemWidth  = clientRect.right;
      measureItem.itemHeight = originalItemHeight;
      measureItem.itemData   = 0;
      gxSendMessageW(lphc->owner, WM_MEASUREITEM, id, (LPARAM)&measureItem);
      lphc->fixedOwnerDrawHeight = measureItem.itemHeight;
    }

    /*
    * Keep the size for the next time
    */
    lphc->editHeight = iTextItemHeight;
  }

  return iTextItemHeight;
}

/***********************************************************************
*           CBForceDummyResize
*
* The dummy resize is used for listboxes that have a popup to trigger
* a re-arranging of the contents of the combobox and the recalculation
* of the size of the "real" control window.
*/
static void CBForceDummyResize(
  GXLPHEADCOMBO lphc)
{
  GXRECT windowRect;
  int newComboHeight;

  newComboHeight = CBGetTextAreaHeight(lphc->self,lphc) + 2*COMBO_YBORDERSIZE();

  gxGetWindowRect(lphc->self, &windowRect);

  /*
  * We have to be careful, resizing a combobox also has the meaning that the
  * dropped rect will be resized. In this case, we want to trigger a resize
  * to recalculate layout but we don't want to change the dropped rectangle
  * So, we pass the height of text area of control as the height.
  * this will cancel-out in the processing of the WM_WINDOWPOSCHANGING
  * message.
  */
  gxSetWindowPos( lphc->self,
    NULL,
    0, 0,
    windowRect.right  - windowRect.left,
    newComboHeight,
    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}

/***********************************************************************
*           CBCalcPlacement
*
* Set up component coordinates given valid lphc->RectCombo.
*/
static void CBCalcPlacement(
  GXHWND        hwnd,
  GXLPHEADCOMBO lphc,
  GXLPRECT      lprEdit,
  GXLPRECT      lprButton,
  GXLPRECT      lprLB)
{
  /*
  * Again, start with the client rectangle.
  */
  gxGetClientRect(hwnd, lprEdit);

  /*
  * Remove the borders
  */
  gxInflateRect(lprEdit, -COMBO_XBORDERSIZE(), -COMBO_YBORDERSIZE());

  /*
  * Chop off the bottom part to fit with the height of the text area.
  */
  lprEdit->bottom = lprEdit->top + CBGetTextAreaHeight(hwnd, lphc);

  /*
  * The button starts the same vertical position as the text area.
  */
  gxCopyRect(lprButton, lprEdit);

  /*
  * If the combobox is "simple" there is no button.
  */
  if( CB_GETTYPE(lphc) == CBS_SIMPLE )
    lprButton->left = lprButton->right = lprButton->bottom = 0;
  else
  {
    /*
    * Let's assume the combobox button is the same width as the
    * scrollbar button.
    * size the button horizontally and cut-off the text area.
    */
    lprButton->left = lprButton->right - GetSystemMetrics(SM_CXVSCROLL);
    lprEdit->right  = lprButton->left;
  }

  /*
  * In the case of a dropdown, there is an additional spacing between the
  * text area and the button.
  */
  if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
  {
    lprEdit->right -= COMBO_EDITBUTTONSPACE();
  }

  /*
  * If we have an edit control, we space it away from the borders slightly.
  */
  if (CB_GETTYPE(lphc) != CBS_DROPDOWNLIST)
  {
    gxInflateRect(lprEdit, -EDIT_CONTROL_PADDING(), -EDIT_CONTROL_PADDING());
  }

  /*
  * Adjust the size of the listbox popup.
  */
  if( CB_GETTYPE(lphc) == CBS_SIMPLE )
  {
    /*
    * Use the client rectangle to initialize the listbox rectangle
    */
    gxGetClientRect(hwnd, lprLB);

    /*
    * Then, chop-off the top part.
    */
    lprLB->top = lprEdit->bottom + COMBO_YBORDERSIZE();
  }
  else
  {
    /*
    * Make sure the dropped width is as large as the combobox itself.
    */
    if (lphc->droppedWidth < (lprButton->right + COMBO_XBORDERSIZE()))
    {
      lprLB->right  = lprLB->left + (lprButton->right + COMBO_XBORDERSIZE());

      /*
      * In the case of a dropdown, the popup listbox is offset to the right.
      * so, we want to make sure it's flush with the right side of the
      * combobox
      */
      if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
        lprLB->right -= COMBO_EDITBUTTONSPACE();
    }
    else
      lprLB->right = lprLB->left + lphc->droppedWidth;
  }

  /* don't allow negative window width */
  if (lprEdit->right < lprEdit->left)
    lprEdit->right = lprEdit->left;

  TRACE("\ttext\t= (%s)\n", wine_dbgstr_rect(lprEdit));

  TRACE("\tbutton\t= (%s)\n", wine_dbgstr_rect(lprButton));

  TRACE("\tlbox\t= (%s)\n", wine_dbgstr_rect(lprLB));
}

/***********************************************************************
*           CBGetDroppedControlRect
*/
static void CBGetDroppedControlRect( GXLPHEADCOMBO lphc, GXLPRECT lpRect)
{
  /* In windows, CB_GETDROPPEDCONTROLRECT returns the upper left corner
  of the combo box and the lower right corner of the listbox */

  gxGetWindowRect(lphc->self, lpRect);

  lpRect->right =  lpRect->left + lphc->droppedRect.right - lphc->droppedRect.left;
  lpRect->bottom = lpRect->top + lphc->droppedRect.bottom - lphc->droppedRect.top;

}

/***********************************************************************
*           COMBO_Create
*/
static LRESULT COMBO_Create( GXHWND hwnd, GXLPHEADCOMBO lphc, GXHWND hwndParent, LONG style,
  GXBOOL unicode )
{
  static const GXWCHAR clbName[] = {'C','o','m','b','o','L','B','o','x',0};
  static const GXWCHAR editName[] = {'E','d','i','t',0};

  if( !CB_GETTYPE(lphc) ) lphc->dwStyle |= CBS_SIMPLE;
  if( CB_GETTYPE(lphc) != CBS_DROPDOWNLIST ) lphc->wState |= CBF_EDIT;

  lphc->owner = hwndParent;

  /*
  * The item height and dropped width are not set when the control
  * is created.
  */
  lphc->droppedWidth = lphc->editHeight = 0;

  /*
  * The first time we go through, we want to measure the ownerdraw item
  */
  lphc->wState |= CBF_MEASUREITEM;

  /* M$ IE 3.01 actually creates (and rapidly destroys) an ownerless combobox */

  if( lphc->owner || !(style & WS_VISIBLE) )
  {
    GXUINT lbeStyle   = 0;
    GXUINT lbeExStyle = 0;

    /*
    * Initialize the dropped rect to the size of the client area of the
    * control and then, force all the areas of the combobox to be
    * recalculated.
    */
    gxGetClientRect( hwnd, &lphc->droppedRect );
    CBCalcPlacement(hwnd, lphc, &lphc->textRect, &lphc->buttonRect, &lphc->droppedRect );

    /*
    * Adjust the position of the popup listbox if it's necessary
    */
    if ( CB_GETTYPE(lphc) != CBS_SIMPLE )
    {
      lphc->droppedRect.top   = lphc->textRect.bottom + COMBO_YBORDERSIZE();

      /*
      * If it's a dropdown, the listbox is offset
      */
      if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
        lphc->droppedRect.left += COMBO_EDITBUTTONSPACE();

      if (lphc->droppedRect.bottom < lphc->droppedRect.top)
        lphc->droppedRect.bottom = lphc->droppedRect.top;
      if (lphc->droppedRect.right < lphc->droppedRect.left)
        lphc->droppedRect.right = lphc->droppedRect.left;
      gxMapWindowPoints( hwnd, 0, (GXLPPOINT)&lphc->droppedRect, 2 );
    }

    /* create listbox popup */

    lbeStyle = (LBS_NOTIFY | LBS_COMBOBOX | GXWS_BORDER | WS_CLIPSIBLINGS | WS_CHILD) |
      (style & (GXWS_VSCROLL | GXCBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE));

    if( lphc->dwStyle & CBS_SORT )
      lbeStyle |= LBS_SORT;
    if( lphc->dwStyle & CBS_HASSTRINGS )
      lbeStyle |= LBS_HASSTRINGS;
    if( lphc->dwStyle & CBS_NOINTEGRALHEIGHT )
      lbeStyle |= LBS_NOINTEGRALHEIGHT;
    if( lphc->dwStyle & CBS_DISABLENOSCROLL )
      lbeStyle |= LBS_DISABLENOSCROLL;

    if( CB_GETTYPE(lphc) == CBS_SIMPLE ) 	/* child listbox */
    {
      lbeStyle |= WS_VISIBLE;

      /*
      * In win 95 look n feel, the listbox in the simple combobox has
      * the WS_EXCLIENTEDGE style instead of the WS_BORDER style.
      */
      lbeStyle   &= ~GXWS_BORDER;
      lbeExStyle |= WS_EX_CLIENTEDGE;
    }
    else
    {
      lbeExStyle |= (WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
    }

    if (unicode) {
      lphc->hWndLBox = gxCreateWindowExW(lbeExStyle, clbName, NULL, lbeStyle,
        lphc->droppedRect.left,
        lphc->droppedRect.top,
        lphc->droppedRect.right - lphc->droppedRect.left,
        lphc->droppedRect.bottom - lphc->droppedRect.top,
        hwnd, (GXHMENU)ID_CB_LISTBOX,
        (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), lphc );
    }
    else {
      CLBREAK;
      //lphc->hWndLBox = gxCreateWindowExA(lbeExStyle, "ComboLBox", NULL, lbeStyle,
      //                                 lphc->droppedRect.left,
      //                                 lphc->droppedRect.top,
      //                                 lphc->droppedRect.right - lphc->droppedRect.left,
      //                                 lphc->droppedRect.bottom - lphc->droppedRect.top,
      //                                 hwnd, (HMENU)ID_CB_LISTBOX,
      //                                 (HINSTANCE)gxGetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), lphc );
    }

    if( lphc->hWndLBox )
    {
      GXBOOL	bEdit = TRUE;
      lbeStyle = GXWS_CHILD | GXWS_VISIBLE | GXES_NOHIDESEL | GXES_LEFT | GXES_COMBO;

      if( lphc->wState & CBF_EDIT )
      {
        if( lphc->dwStyle & CBS_OEMCONVERT )
          lbeStyle |= ES_OEMCONVERT;
        if( lphc->dwStyle & CBS_AUTOHSCROLL )
          lbeStyle |= ES_AUTOHSCROLL;
        if( lphc->dwStyle & CBS_LOWERCASE )
          lbeStyle |= ES_LOWERCASE;
        else if( lphc->dwStyle & CBS_UPPERCASE )
          lbeStyle |= ES_UPPERCASE;

        if (!gxIsWindowEnabled(hwnd)) lbeStyle |= WS_DISABLED;

        if (unicode)
        {
          lphc->hWndEdit = gxCreateWindowExW(0, editName, NULL, lbeStyle,
            lphc->textRect.left, lphc->textRect.top,
            lphc->textRect.right - lphc->textRect.left,
            lphc->textRect.bottom - lphc->textRect.top,
            hwnd, (GXHMENU)ID_CB_EDIT,
            (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), NULL );
        }
        else
        {
          CLBREAK;
          //lphc->hWndEdit = gxCreateWindowExA(0, "Edit", NULL, lbeStyle,
          //  lphc->textRect.left, lphc->textRect.top,
          //  lphc->textRect.right - lphc->textRect.left,
          //  lphc->textRect.bottom - lphc->textRect.top,
          //  hwnd, (GXHMENU)ID_CB_EDIT,
          //  (GXHINSTANCE)gxGetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), NULL );
        }

        if( !lphc->hWndEdit )
          bEdit = FALSE;
      }

      if( bEdit )
      {
        if( CB_GETTYPE(lphc) != CBS_SIMPLE )
        {
          /* Now do the trick with parent */
          gxSetParent(lphc->hWndLBox, GXHWND_DESKTOP);
          /*
          * If the combo is a dropdown, we must resize the control
          * to fit only the text area and button. To do this,
          * we send a dummy resize and the WM_WINDOWPOSCHANGING message
          * will take care of setting the height for us.
          */
          CBForceDummyResize(lphc);
        }

        TRACE("init done\n");
        return 0;
      }
      ERR("edit control failure.\n");
    } else ERR("listbox failure.\n");
  } else ERR("no owner for visible combo.\n");

  /* CreateWindow() will send WM_NCDESTROY to cleanup */

  return -1;
}

/***********************************************************************
*           CBPaintButton
*
* Paint combo button (normal, pressed, and disabled states).
*/
static void CBPaintButton( GXLPHEADCOMBO lphc, GXHDC hdc, GXRECT rectButton)
{
  GXUINT buttonState = DFCS_SCROLLCOMBOBOX;

  if( lphc->wState & CBF_NOREDRAW )
    return;


  if (lphc->wState & CBF_BUTTONDOWN)
    buttonState |= DFCS_PUSHED;

  if (CB_DISABLED(lphc))
    buttonState |= DFCS_INACTIVE;

  gxDrawFrameControl(hdc, &rectButton, DFC_SCROLL, buttonState);
}

/***********************************************************************
*           CBPaintText
*
* Paint CBS_DROPDOWNLIST text field / update edit control contents.
*/
static void CBPaintText(
  GXLPHEADCOMBO lphc,
  GXHDC         hdc,
  GXRECT        rectEdit)
{
  GXINT	id, size = 0;
  GXLPWSTR pText = NULL;

  if( lphc->wState & CBF_NOREDRAW ) return;

  TRACE("\n");

  /* follow Windows combobox that sends a bunch of text
  * inquiries to its listbox while processing WM_PAINT. */

  if( (id = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0) ) != LB_ERR )
  {
    size = gxSendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, id, 0);
    if (size == LB_ERR)
      FIXME("LB_ERR probably not handled yet\n");
    if( (pText = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (size + 1) * sizeof(GXWCHAR))) )
    {
      /* size from LB_GETTEXTLEN may be too large, from LB_GETTEXT is accurate */
      size=gxSendMessageW(lphc->hWndLBox, LB_GETTEXT, id, (LPARAM)pText);
      pText[size] = '\0';	/* just in case */
    } else return;
  }
  else
    if( !CB_OWNERDRAWN(lphc) )
      return;

  if( lphc->wState & CBF_EDIT )
  {
    static const GXWCHAR empty_stringW[] = { 0 };
    if( CB_HASSTRINGS(lphc) ) gxSetWindowTextW( lphc->hWndEdit, pText ? pText : empty_stringW );
    if( lphc->wState & CBF_FOCUSED )
      gxSendMessageW(lphc->hWndEdit, EM_SETSEL, 0, -1);
  }
  else /* paint text field ourselves */
  {
    GXUINT	itemState = ODS_COMBOBOXEDIT;
    GXHFONT	hPrevFont = (lphc->hFont) ? (GXHFONT)gxSelectObject(hdc, lphc->hFont) : 0;

    /*
    * Give ourselves some space.
    */
    gxInflateRect( &rectEdit, -1, -1 );

    if( CB_OWNERDRAWN(lphc) )
    {
      GXDRAWITEMSTRUCT dis;
      GXHRGN           clipRegion;
      GXUINT ctlid = (GXUINT)gxGetWindowLongPtrW( lphc->self, GWLP_ID );

      /* setup state for DRAWITEM message. Owner will highlight */
      if ( (lphc->wState & CBF_FOCUSED) &&
        !(lphc->wState & CBF_DROPPED) )
        itemState |= ODS_SELECTED | ODS_FOCUS;

      if (!gxIsWindowEnabled(lphc->self)) itemState |= ODS_DISABLED;

      dis.CtlType	    = GXODT_COMBOBOX;
      dis.CtlID	      = ctlid;
      dis.hwndItem	  = lphc->self;
      dis.itemAction	= ODA_DRAWENTIRE;
      dis.itemID	    = id;
      dis.itemState	  = itemState;
      dis.hDC		      = hdc;
      dis.rcItem	    = rectEdit;
      dis.itemData    = gxSendMessageW(lphc->hWndLBox, LB_GETITEMDATA, id, 0);

      /*
      * Clip the DC and have the parent draw the item.
      */
      clipRegion = set_control_clipping( hdc, &rectEdit );

      gxSendMessageW(lphc->owner, GXWM_DRAWITEM, ctlid, (LPARAM)&dis );

      gxSelectClipRgn( hdc, clipRegion );
      if (clipRegion) gxDeleteObject( clipRegion );
    }
    else
    {
      static const GXWCHAR empty_stringW[] = { 0 };

      if ( (lphc->wState & CBF_FOCUSED) &&
        !(lphc->wState & CBF_DROPPED) ) {

          /* highlight */
          gxFillRect( hdc, &rectEdit, gxGetSysColorBrush(COLOR_HIGHLIGHT) );
          gxSetBkColor( hdc, gxGetSysColor( COLOR_HIGHLIGHT ) );
          gxSetTextColor( hdc, gxGetSysColor( COLOR_HIGHLIGHTTEXT ) );
      }

      gxExtTextOutW( hdc,
        rectEdit.left + 1,
        rectEdit.top + 1,
        ETO_OPAQUE | ETO_CLIPPED,
        &rectEdit,
        pText ? pText : empty_stringW , size, NULL );

      if(lphc->wState & CBF_FOCUSED && !(lphc->wState & CBF_DROPPED))
        gxDrawFocusRect( hdc, &rectEdit );
    }

    if( hPrevFont )
      gxSelectObject(hdc, hPrevFont );
  }
  gxHeapFree( gxGetProcessHeap(), 0, pText );
}

/***********************************************************************
*           CBPaintBorder
*/
static void CBPaintBorder(
  GXHWND            hwnd,
  const GXHEADCOMBO *lphc,
  GXHDC             hdc)
{
  GXRECT clientRect;

  if (CB_GETTYPE(lphc) != CBS_SIMPLE)
  {
    gxGetClientRect(hwnd, &clientRect);
  }
  else
  {
    gxCopyRect(&clientRect, &lphc->textRect);

    gxInflateRect(&clientRect, EDIT_CONTROL_PADDING(), EDIT_CONTROL_PADDING());
    gxInflateRect(&clientRect, COMBO_XBORDERSIZE(), COMBO_YBORDERSIZE());
  }

  gxDrawEdge(hdc, &clientRect, EDGE_SUNKEN, BF_RECT);
}

/***********************************************************************
*           COMBO_PrepareColors
*
* This method will sent the appropriate WM_CTLCOLOR message to
* prepare and setup the colors for the combo's DC.
*
* It also returns the brush to use for the background.
*/
static GXHBRUSH COMBO_PrepareColors(
  GXLPHEADCOMBO lphc,
  GXHDC         hDC)
{
  GXHBRUSH  hBkgBrush;

  /*
  * Get the background brush for this control.
  */
  if (CB_DISABLED(lphc))
  {
    hBkgBrush = (GXHBRUSH)gxSendMessageW(lphc->owner, WM_CTLCOLORSTATIC,
      (WPARAM)hDC, (LPARAM)lphc->self );

    /*
    * We have to change the text color since WM_CTLCOLORSTATIC will
    * set it to the "enabled" color. This is the same behavior as the
    * edit control
    */
    gxSetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
  }
  else
  {
    /* FIXME: In which cases WM_CTLCOLORLISTBOX should be sent? */
    hBkgBrush = (GXHBRUSH)gxSendMessageW(lphc->owner, WM_CTLCOLOREDIT,
      (WPARAM)hDC, (LPARAM)lphc->self );
  }

  /*
  * Catch errors.
  */
  if( !hBkgBrush )
    hBkgBrush = gxGetSysColorBrush(COLOR_WINDOW);

  return hBkgBrush;
}


/***********************************************************************
*           COMBO_Paint
*/
static LRESULT COMBO_Paint(GXLPHEADCOMBO lphc, GXHDC hParamDC)
{
  GXPAINTSTRUCT ps;
  GXHDC 	hDC;

  hDC = (hParamDC) ? hParamDC
    : gxBeginPaint( lphc->self, &ps);

  TRACE("hdc=%p\n", hDC);

  if( hDC && !(lphc->wState & CBF_NOREDRAW) )
  {
    GXHBRUSH	hPrevBrush, hBkgBrush;

    /*
    * Retrieve the background brush and select it in the
    * DC.
    */
    hBkgBrush = COMBO_PrepareColors(lphc, hDC);

    hPrevBrush = (GXHBRUSH)gxSelectObject( hDC, hBkgBrush );
    if (!(lphc->wState & CBF_EDIT))
      gxFillRect(hDC, &lphc->textRect, hBkgBrush);

    /*
    * In non 3.1 look, there is a sunken border on the combobox
    */
    CBPaintBorder(lphc->self, lphc, hDC);

    if( !gxIsRectEmpty(&lphc->buttonRect) )
    {
      CBPaintButton(lphc, hDC, lphc->buttonRect);
    }

    /* paint the edit control padding area */
    if (CB_GETTYPE(lphc) != CBS_DROPDOWNLIST)
    {
      GXRECT rPadEdit = lphc->textRect;

      gxInflateRect(&rPadEdit, EDIT_CONTROL_PADDING(), EDIT_CONTROL_PADDING());

      gxFrameRect( hDC, &rPadEdit, gxGetSysColorBrush(COLOR_WINDOW) );
    }

    if( !(lphc->wState & CBF_EDIT) )
      CBPaintText( lphc, hDC, lphc->textRect);

    if( hPrevBrush )
      gxSelectObject( hDC, hPrevBrush );
  }

  if( !hParamDC )
    gxEndPaint(lphc->self, &ps);

  return 0;
}

/***********************************************************************
*           CBUpdateLBox
*
* Select listbox entry according to the contents of the edit control.
*/
static GXINT CBUpdateLBox( GXLPHEADCOMBO lphc, GXBOOL bSelect )
{
  GXINT	length, idx;
  GXLPWSTR pText = NULL;

  idx = LB_ERR;
  length = gxSendMessageW( lphc->hWndEdit, WM_GETTEXTLENGTH, 0, 0 );

  if( length > 0 )
    pText = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (length + 1) * sizeof(GXWCHAR));

  TRACE("\t edit text length %i\n", length );

  if( pText )
  {
    gxGetWindowTextW( lphc->hWndEdit, pText, length + 1);
    idx = gxSendMessageW(lphc->hWndLBox, LB_FINDSTRING, -1, (LPARAM)pText);
    gxHeapFree( gxGetProcessHeap(), 0, pText );
  }

  gxSendMessageW(lphc->hWndLBox, LB_SETCURSEL, bSelect ? idx : -1, 0);

  /* probably superfluous but Windows sends this too */
  gxSendMessageW(lphc->hWndLBox, LB_SETCARETINDEX, idx < 0 ? 0 : idx, 0);
  gxSendMessageW(lphc->hWndLBox, LB_SETTOPINDEX, idx < 0 ? 0 : idx, 0);

  return idx;
}

/***********************************************************************
*           CBUpdateEdit
*
* Copy a listbox entry to the edit control.
*/
static void CBUpdateEdit( GXLPHEADCOMBO lphc , GXINT index )
{
  GXINT	length;
  GXLPWSTR pText = NULL;
  static const GXWCHAR empty_stringW[] = { 0 };

  TRACE("\t %i\n", index );

  if( index >= 0 ) /* got an entry */
  {
    length = gxSendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, index, 0);
    if( length != LB_ERR)
    {
      if( (pText = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (length + 1) * sizeof(GXWCHAR))) )
      {
        gxSendMessageW(lphc->hWndLBox, LB_GETTEXT, index, (LPARAM)pText);
      }
    }
  }

  if( CB_HASSTRINGS(lphc) )
  {
    lphc->wState |= (CBF_NOEDITNOTIFY | CBF_NOLBSELECT);
    gxSendMessageW(lphc->hWndEdit, WM_SETTEXT, 0, pText ? (LPARAM)pText : (LPARAM)empty_stringW);
    lphc->wState &= ~(CBF_NOEDITNOTIFY | CBF_NOLBSELECT);
  }

  if( lphc->wState & CBF_FOCUSED )
    gxSendMessageW(lphc->hWndEdit, EM_SETSEL, 0, -1);

  gxHeapFree( gxGetProcessHeap(), 0, pText );
}

/***********************************************************************
*           CBDropDown
*
* Show listbox popup.
*/
static void CBDropDown( GXLPHEADCOMBO lphc )
{
  GXHMONITOR monitor;
  GXMONITORINFO mon_info;
  GXRECT rect,r;
  int nItems = 0;
  int nDroppedHeight;

  TRACE("[%p]: drop down\n", lphc->self);

  CB_NOTIFY( lphc, CBN_DROPDOWN );

  /* set selection */

  lphc->wState |= CBF_DROPPED;
  if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
  {
    lphc->droppedIndex = CBUpdateLBox( lphc, TRUE );

    /* Update edit only if item is in the list */
    if( !(lphc->wState & CBF_CAPTURE) && lphc->droppedIndex >= 0)
      CBUpdateEdit( lphc, lphc->droppedIndex );
  }
  else
  {
    lphc->droppedIndex = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);

    gxSendMessageW(lphc->hWndLBox, LB_SETTOPINDEX,
      lphc->droppedIndex == LB_ERR ? 0 : lphc->droppedIndex, 0);
    gxSendMessageW(lphc->hWndLBox, GXLB_CARETON, 0, 0);
  }

  /* now set popup position */
  gxGetWindowRect( lphc->self, &rect );

  /*
  * If it's a dropdown, the listbox is offset
  */
  if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
    rect.left += COMBO_EDITBUTTONSPACE();

  /* if the dropped height is greater than the total height of the dropped
  items list, then force the drop down list height to be the total height
  of the items in the dropped list */

  /* And Remove any extra space (Best Fit) */
  nDroppedHeight = lphc->droppedRect.bottom - lphc->droppedRect.top;
  /* if listbox length has been set directly by its handle */
  gxGetWindowRect(lphc->hWndLBox, &r);
  if (nDroppedHeight < r.bottom - r.top)
    nDroppedHeight = r.bottom - r.top;
  nItems = (int)gxSendMessageW(lphc->hWndLBox, LB_GETCOUNT, 0, 0);

  if (nItems > 0)
  {
    int nHeight;
    int nIHeight;

    nIHeight = (int)gxSendMessageW(lphc->hWndLBox, LB_GETITEMHEIGHT, 0, 0);

    nHeight = nIHeight*nItems;

    if (nHeight < nDroppedHeight - COMBO_YBORDERSIZE())
      nDroppedHeight = nHeight + COMBO_YBORDERSIZE();

    if (nDroppedHeight < nHeight)
    {
      if (nItems < 5)
        nDroppedHeight = (nItems+1)*nIHeight;
      else if (nDroppedHeight < 6*nIHeight)
        nDroppedHeight = 6*nIHeight;
    }
  }

  r.left = rect.left;
  r.top = rect.bottom;
  r.right = r.left + lphc->droppedRect.right - lphc->droppedRect.left;
  r.bottom = r.top + nDroppedHeight;

  /*If height of dropped rectangle gets beyond a screen size it should go up, otherwise down.*/
  monitor = gxMonitorFromRect( &rect, MONITOR_DEFAULTTOPRIMARY );
  mon_info.cbSize = sizeof(mon_info);
  gxGetMonitorInfoW( monitor, &mon_info );

  if (r.bottom > mon_info.rcWork.bottom)
  {
    r.top = max( rect.top - nDroppedHeight, mon_info.rcWork.top );
    r.bottom = min( r.top + nDroppedHeight, mon_info.rcWork.bottom );
  }

  gxSetWindowPos( lphc->hWndLBox, GXHWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top,
    SWP_NOACTIVATE | SWP_SHOWWINDOW );


  if( !(lphc->wState & CBF_NOREDRAW) )
    gxRedrawWindow( lphc->self, NULL, 0, RDW_INVALIDATE |
    RDW_ERASE | RDW_UPDATENOW | RDW_NOCHILDREN );

  gxEnableWindow( lphc->hWndLBox, TRUE );
  if (gxGetCapture() != lphc->self)
    gxSetCapture(lphc->hWndLBox);
}

/***********************************************************************
*           CBRollUp
*
* Hide listbox popup.
*/
static void CBRollUp( GXLPHEADCOMBO lphc, GXBOOL ok, GXBOOL bButton )
{
  GXHWND	hWnd = lphc->self;

  TRACE("[%p]: sel ok? [%i] dropped? [%i]\n",
    lphc->self, ok, (GXINT)(lphc->wState & CBF_DROPPED));

  CB_NOTIFY( lphc, (ok) ? CBN_SELENDOK : CBN_SELENDCANCEL );

  if( gxIsWindow( hWnd ) && CB_GETTYPE(lphc) != CBS_SIMPLE )
  {

    if( lphc->wState & CBF_DROPPED )
    {
      GXRECT	rect;

      lphc->wState &= ~CBF_DROPPED;
      gxShowWindow( lphc->hWndLBox, SW_HIDE );

      if(gxGetCapture() == lphc->hWndLBox)
      {
        gxReleaseCapture();
      }

      if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
      {
        rect = lphc->buttonRect;
      }
      else
      {
        if( bButton )
        {
          gxUnionRect( &rect,
            &lphc->buttonRect,
            &lphc->textRect);
        }
        else
          rect = lphc->textRect;

        bButton = TRUE;
      }

      if( bButton && !(lphc->wState & CBF_NOREDRAW) )
        gxRedrawWindow( hWnd, &rect, 0, RDW_INVALIDATE |
        RDW_ERASE | RDW_UPDATENOW | RDW_NOCHILDREN );
      CB_NOTIFY( lphc, CBN_CLOSEUP );
    }
  }
}

/***********************************************************************
*           COMBO_FlipListbox
*
* Used by the ComboLBox to show/hide itself in response to VK_F4, etc...
*/
GXBOOL COMBO_FlipListbox( GXLPHEADCOMBO lphc, GXBOOL ok, GXBOOL bRedrawButton )
{
  if( lphc->wState & CBF_DROPPED )
  {
    CBRollUp( lphc, ok, bRedrawButton );
    return FALSE;
  }

  CBDropDown( lphc );
  return TRUE;
}

/***********************************************************************
*           CBRepaintButton
*/
static void CBRepaintButton( GXLPHEADCOMBO lphc )
{
  gxInvalidateRect(lphc->self, &lphc->buttonRect, TRUE);
  gxUpdateWindow(lphc->self);
}

/***********************************************************************
*           COMBO_SetFocus
*/
static void COMBO_SetFocus( GXLPHEADCOMBO lphc )
{
  if( !(lphc->wState & CBF_FOCUSED) )
  {
    if( CB_GETTYPE(lphc) == CBS_DROPDOWNLIST )
      gxSendMessageW(lphc->hWndLBox, GXLB_CARETON, 0, 0);

    /* This is wrong. Message sequences seem to indicate that this
    is set *after* the notify. */
    /* lphc->wState |= CBF_FOCUSED;  */

    if( !(lphc->wState & CBF_EDIT) )
      gxInvalidateRect(lphc->self, &lphc->textRect, TRUE);

    CB_NOTIFY( lphc, CBN_SETFOCUS );
    lphc->wState |= CBF_FOCUSED;
  }
}

/***********************************************************************
*           COMBO_KillFocus
*/
static void COMBO_KillFocus( GXLPHEADCOMBO lphc )
{
  GXHWND	hWnd = lphc->self;

  if( lphc->wState & CBF_FOCUSED )
  {
    CBRollUp( lphc, FALSE, TRUE );
    if( gxIsWindow( hWnd ) )
    {
      if( CB_GETTYPE(lphc) == CBS_DROPDOWNLIST )
        gxSendMessageW(lphc->hWndLBox, GXLB_CARETOFF, 0, 0);

      lphc->wState &= ~CBF_FOCUSED;

      /* redraw text */
      if( !(lphc->wState & CBF_EDIT) )
        gxInvalidateRect(lphc->self, &lphc->textRect, TRUE);

      CB_NOTIFY( lphc, CBN_KILLFOCUS );
    }
  }
}

/***********************************************************************
*           COMBO_Command
*/
static LRESULT COMBO_Command( GXLPHEADCOMBO lphc, WPARAM wParam, GXHWND hWnd )
{
  if ( lphc->wState & CBF_EDIT && lphc->hWndEdit == hWnd )
  {
    /* ">> 8" makes gcc generate jump-table instead of cmp ladder */

    switch( HIWORD(wParam) >> 8 )
    {
    case (EN_SETFOCUS >> 8):

      TRACE("[%p]: edit [%p] got focus\n", lphc->self, lphc->hWndEdit );

      COMBO_SetFocus( lphc );
      break;

    case (EN_KILLFOCUS >> 8):

      TRACE("[%p]: edit [%p] lost focus\n", lphc->self, lphc->hWndEdit );

      /* NOTE: it seems that Windows' edit control sends an
      * undocumented message WM_USER + 0x1B instead of this
      * notification (only when it happens to be a part of
      * the combo). ?? - AK.
      */

      COMBO_KillFocus( lphc );
      break;


    case (EN_CHANGE >> 8):
      /*
      * In some circumstances (when the selection of the combobox
      * is changed for example) we don't want the EN_CHANGE notification
      * to be forwarded to the parent of the combobox. This code
      * checks a flag that is set in these occasions and ignores the
      * notification.
      */
      if (lphc->wState & CBF_NOLBSELECT)
      {
        lphc->wState &= ~CBF_NOLBSELECT;
      }
      else
      {
        CBUpdateLBox( lphc, lphc->wState & CBF_DROPPED );
      }

      if (!(lphc->wState & CBF_NOEDITNOTIFY))
        CB_NOTIFY( lphc, CBN_EDITCHANGE );
      break;

    case (EN_UPDATE >> 8):
      if (!(lphc->wState & CBF_NOEDITNOTIFY))
        CB_NOTIFY( lphc, CBN_EDITUPDATE );
      break;

    case (EN_ERRSPACE >> 8):
      CB_NOTIFY( lphc, CBN_ERRSPACE );
    }
  }
  else if( lphc->hWndLBox == hWnd )
  {
    switch( (short)HIWORD(wParam) )
    {
    case LBN_ERRSPACE:
      CB_NOTIFY( lphc, CBN_ERRSPACE );
      break;

    case LBN_DBLCLK:
      CB_NOTIFY( lphc, CBN_DBLCLK );
      break;

    case LBN_SELCHANGE:
    case LBN_SELCANCEL:

      TRACE("[%p]: lbox selection change [%x]\n", lphc->self, lphc->wState );

      /* do not roll up if selection is being tracked
      * by arrow keys in the dropdown listbox */
      if (!(lphc->wState & CBF_NOROLLUP))
      {
        CBRollUp( lphc, (HIWORD(wParam) == LBN_SELCHANGE), TRUE );
      }
      else lphc->wState &= ~CBF_NOROLLUP;

      CB_NOTIFY( lphc, CBN_SELCHANGE );

      if( HIWORD(wParam) == LBN_SELCHANGE)
      {
        if( lphc->wState & CBF_EDIT )
        {
          GXINT index = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
          lphc->wState |= CBF_NOLBSELECT;
          CBUpdateEdit( lphc, index );
          /* select text in edit, as Windows does */
          gxSendMessageW(lphc->hWndEdit, EM_SETSEL, 0, -1);
        }
        else
        {
          gxInvalidateRect(lphc->self, &lphc->textRect, TRUE);
          gxUpdateWindow(lphc->self);
        }
      }
      break;

    case LBN_SETFOCUS:
    case LBN_KILLFOCUS:
      /* nothing to do here since ComboLBox always resets the focus to its
      * combo/edit counterpart */
      break;
    }
  }
  return 0;
}

/***********************************************************************
*           COMBO_ItemOp
*
* Fixup an ownerdrawn item operation and pass it up to the combobox owner.
*/
static LRESULT COMBO_ItemOp( GXLPHEADCOMBO lphc, GXUINT msg, LPARAM lParam )
{
  GXHWND hWnd = lphc->self;
  GXUINT id = (GXUINT)gxGetWindowLongPtrW( hWnd, GWLP_ID );

  TRACE("[%p]: ownerdraw op %04x\n", lphc->self, msg );

  switch( msg )
  {
  case WM_DELETEITEM:
    {
      GXDELETEITEMSTRUCT *lpIS = (GXDELETEITEMSTRUCT *)lParam;
      lpIS->CtlType  = GXODT_COMBOBOX;
      lpIS->CtlID    = id;
      lpIS->hwndItem = hWnd;
      break;
    }
  case WM_DRAWITEM:
    {
      GXDRAWITEMSTRUCT *lpIS = (GXDRAWITEMSTRUCT *)lParam;
      lpIS->CtlType  = GXODT_COMBOBOX;
      lpIS->CtlID    = id;
      lpIS->hwndItem = hWnd;
      break;
    }
  case WM_COMPAREITEM:
    {
      GXCOMPAREITEMSTRUCT *lpIS = (GXCOMPAREITEMSTRUCT *)lParam;
      lpIS->CtlType  = GXODT_COMBOBOX;
      lpIS->CtlID    = id;
      lpIS->hwndItem = hWnd;
      break;
    }
  case WM_MEASUREITEM:
    {
      MEASUREITEMSTRUCT *lpIS = (MEASUREITEMSTRUCT *)lParam;
      lpIS->CtlType  = GXODT_COMBOBOX;
      lpIS->CtlID    = id;
      break;
    }
  }
  return gxSendMessageW(lphc->owner, msg, id, lParam);
}


/***********************************************************************
*           COMBO_GetTextW
*/
static LRESULT COMBO_GetTextW( GXLPHEADCOMBO lphc, GXINT count, GXLPWSTR buf )
{
  GXINT length;

  if( lphc->wState & CBF_EDIT )
    return gxSendMessageW( lphc->hWndEdit, WM_GETTEXT, count, (LPARAM)buf );

  /* get it from the listbox */

  if (!count || !buf) return 0;
  if( lphc->hWndLBox )
  {
    GXINT idx = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
    if (idx == LB_ERR) goto error;
    length = gxSendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, idx, 0 );
    if (length == LB_ERR) goto error;

    /* 'length' is without the terminating character */
    if (length >= count)
    {
      GXLPWSTR lpBuffer = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (length + 1) * sizeof(GXWCHAR));
      if (!lpBuffer) goto error;
      length = gxSendMessageW(lphc->hWndLBox, LB_GETTEXT, idx, (LPARAM)lpBuffer);

      /* truncate if buffer is too short */
      if (length != LB_ERR)
      {
        lstrcpynW( buf, lpBuffer, count );
        length = count;
      }
      gxHeapFree( gxGetProcessHeap(), 0, lpBuffer );
    }
    else length = gxSendMessageW(lphc->hWndLBox, LB_GETTEXT, idx, (LPARAM)buf);

    if (length == LB_ERR) return 0;
    return length;
  }

error:  /* error - truncate string, return zero */
  buf[0] = 0;
  return 0;
}


/***********************************************************************
*           COMBO_GetTextA
*
* NOTE! LB_GETTEXT does not count terminating \0, WM_GETTEXT does.
*       also LB_GETTEXT might return values < 0, WM_GETTEXT doesn't.
*/
static LRESULT COMBO_GetTextA( GXLPHEADCOMBO lphc, GXINT count, GXLPSTR buf )
{
  GXINT length;

  if( lphc->wState & CBF_EDIT )
    return gxSendMessageA( lphc->hWndEdit, WM_GETTEXT, count, (LPARAM)buf );

  /* get it from the listbox */

  if (!count || !buf) return 0;
  if( lphc->hWndLBox )
  {
    GXINT idx = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
    if (idx == LB_ERR) goto error;
    length = gxSendMessageA(lphc->hWndLBox, LB_GETTEXTLEN, idx, 0 );
    if (length == LB_ERR) goto error;

    /* 'length' is without the terminating character */
    if (length >= count)
    {
      GXLPSTR lpBuffer = (GXLPSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (length + 1) );
      if (!lpBuffer) goto error;
      length = gxSendMessageA(lphc->hWndLBox, LB_GETTEXT, idx, (LPARAM)lpBuffer);

      /* truncate if buffer is too short */
      if (length != LB_ERR)
      {
        lstrcpynA( buf, lpBuffer, count );
        length = count;
      }
      gxHeapFree( gxGetProcessHeap(), 0, lpBuffer );
    }
    else length = gxSendMessageA(lphc->hWndLBox, LB_GETTEXT, idx, (LPARAM)buf);

    if (length == LB_ERR) return 0;
    return length;
  }

error:  /* error - truncate string, return zero */
  buf[0] = 0;
  return 0;
}


/***********************************************************************
*           CBResetPos
*
* This function sets window positions according to the updated
* component placement struct.
*/
static void CBResetPos(
  GXLPHEADCOMBO lphc,
  const GXRECT  *rectEdit,
  const GXRECT  *rectLB,
  GXBOOL        bRedraw)
{
  GXBOOL	bDrop = (CB_GETTYPE(lphc) != CBS_SIMPLE);

  /* NOTE: logs sometimes have WM_LBUTTONUP before a cascade of
  * sizing messages */

  if( lphc->wState & CBF_EDIT )
    gxSetWindowPos( lphc->hWndEdit, 0,
    rectEdit->left, rectEdit->top,
    rectEdit->right - rectEdit->left,
    rectEdit->bottom - rectEdit->top,
    SWP_NOZORDER | SWP_NOACTIVATE | ((bDrop) ? SWP_NOREDRAW : 0) );

  gxSetWindowPos( lphc->hWndLBox, 0,
    rectLB->left, rectLB->top,
    rectLB->right - rectLB->left,
    rectLB->bottom - rectLB->top,
    SWP_NOACTIVATE | SWP_NOZORDER | ((bDrop) ? SWP_NOREDRAW : 0) );

  if( bDrop )
  {
    if( lphc->wState & CBF_DROPPED )
    {
      lphc->wState &= ~CBF_DROPPED;
      gxShowWindow( lphc->hWndLBox, SW_HIDE );
    }

    if( bRedraw && !(lphc->wState & CBF_NOREDRAW) )
      gxRedrawWindow( lphc->self, NULL, 0,
      RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
  }
}


/***********************************************************************
*           COMBO_Size
*/
static void COMBO_Size( GXLPHEADCOMBO lphc )
{
  /*
  * Those controls are always the same height. So we have to make sure
  * they are not resized to another value.
  */
  if( CB_GETTYPE(lphc) != CBS_SIMPLE )
  {
    int newComboHeight, curComboHeight, curComboWidth;
    GXRECT rc;

    gxGetWindowRect(lphc->self, &rc);
    curComboHeight = rc.bottom - rc.top;
    curComboWidth = rc.right - rc.left;
    newComboHeight = CBGetTextAreaHeight(lphc->self, lphc) + 2*COMBO_YBORDERSIZE();

    /*
    * Resizing a combobox has another side effect, it resizes the dropped
    * rectangle as well. However, it does it only if the new height for the
    * combobox is more than the height it should have. In other words,
    * if the application resizing the combobox only had the intention to resize
    * the actual control, for example, to do the layout of a dialog that is
    * resized, the height of the dropdown is not changed.
    */
    if( curComboHeight > newComboHeight )
    {
      TRACE("oldComboHeight=%d, newComboHeight=%d, oldDropBottom=%d, oldDropTop=%d\n",
        curComboHeight, newComboHeight, lphc->droppedRect.bottom,
        lphc->droppedRect.top);
      lphc->droppedRect.bottom = lphc->droppedRect.top + curComboHeight - newComboHeight;
    }
    /*
    * Restore original height
    */
    if( curComboHeight != newComboHeight )
      gxSetWindowPos(lphc->self, 0, 0, 0, curComboWidth, newComboHeight,
      SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW);
  }

  CBCalcPlacement(lphc->self,
    lphc,
    &lphc->textRect,
    &lphc->buttonRect,
    &lphc->droppedRect);

  CBResetPos( lphc, &lphc->textRect, &lphc->droppedRect, TRUE );
}


/***********************************************************************
*           COMBO_Font
*/
static void COMBO_Font( GXLPHEADCOMBO lphc, GXHFONT hFont, GXBOOL bRedraw )
{
  /*
  * Set the font
  */
  lphc->hFont = hFont;

  /*
  * Propagate to owned windows.
  */
  if( lphc->wState & CBF_EDIT )
    gxSendMessageW(lphc->hWndEdit, WM_SETFONT, (WPARAM)hFont, bRedraw);
  gxSendMessageW(lphc->hWndLBox, WM_SETFONT, (WPARAM)hFont, bRedraw);

  /*
  * Redo the layout of the control.
  */
  if ( CB_GETTYPE(lphc) == CBS_SIMPLE)
  {
    CBCalcPlacement(lphc->self,
      lphc,
      &lphc->textRect,
      &lphc->buttonRect,
      &lphc->droppedRect);

    CBResetPos( lphc, &lphc->textRect, &lphc->droppedRect, TRUE );
  }
  else
  {
    CBForceDummyResize(lphc);
  }
}


/***********************************************************************
*           COMBO_SetItemHeight
*/
static LRESULT COMBO_SetItemHeight( GXLPHEADCOMBO lphc, GXINT index, GXINT height )
{
  LRESULT	lRet = CB_ERR;

  if( index == -1 ) /* set text field height */
  {
    if( height < 32768 )
    {
      lphc->editHeight = height + 2;  /* Is the 2 for 2*EDIT_CONTROL_PADDING? */

      /*
      * Redo the layout of the control.
      */
      if ( CB_GETTYPE(lphc) == CBS_SIMPLE)
      {
        CBCalcPlacement(lphc->self,
          lphc,
          &lphc->textRect,
          &lphc->buttonRect,
          &lphc->droppedRect);

        CBResetPos( lphc, &lphc->textRect, &lphc->droppedRect, TRUE );
      }
      else
      {
        CBForceDummyResize(lphc);
      }

      lRet = height;
    }
  }
  else if ( CB_OWNERDRAWN(lphc) )	/* set listbox item height */
    lRet = gxSendMessageW(lphc->hWndLBox, LB_SETITEMHEIGHT, index, height);
  return lRet;
}

/***********************************************************************
*           COMBO_SelectString
*/
static LRESULT COMBO_SelectString( GXLPHEADCOMBO lphc, GXINT start, LPARAM pText, GXBOOL unicode )
{
  GXINT index = unicode ? gxSendMessageW(lphc->hWndLBox, LB_SELECTSTRING, start, pText) :
    gxSendMessageA(lphc->hWndLBox, LB_SELECTSTRING, start, pText);
if( index >= 0 )
{
  if( lphc->wState & CBF_EDIT )
    CBUpdateEdit( lphc, index );
  else
  {
    gxInvalidateRect(lphc->self, &lphc->textRect, TRUE);
  }
}
return (LRESULT)index;
}

/***********************************************************************
*           COMBO_LButtonDown
*/
static void COMBO_LButtonDown( GXLPHEADCOMBO lphc, LPARAM lParam )
{
  GXPOINT     pt;
  GXBOOL      bButton;
  GXHWND      hWnd = lphc->self;

  pt.x = (short)LOWORD(lParam);
  pt.y = (short)HIWORD(lParam);
  bButton = gxPtInRect(&lphc->buttonRect, pt);

  if( (CB_GETTYPE(lphc) == CBS_DROPDOWNLIST) ||
    (bButton && (CB_GETTYPE(lphc) == CBS_DROPDOWN)) )
  {
    lphc->wState |= CBF_BUTTONDOWN;
    if( lphc->wState & CBF_DROPPED )
    {
      /* got a click to cancel selection */

      lphc->wState &= ~CBF_BUTTONDOWN;
      CBRollUp( lphc, TRUE, FALSE );
      if( ! gxIsWindow( hWnd ) ) return;

      if( lphc->wState & CBF_CAPTURE )
      {
        lphc->wState &= ~CBF_CAPTURE;
        gxReleaseCapture();
      }
    }
    else
    {
      /* drop down the listbox and start tracking */

      lphc->wState |= CBF_CAPTURE;
      gxSetCapture( hWnd );
      CBDropDown( lphc );
    }
    if( bButton ) CBRepaintButton( lphc );
  }
}

/***********************************************************************
*           COMBO_LButtonUp
*
* Release capture and stop tracking if needed.
*/
static void COMBO_LButtonUp( GXLPHEADCOMBO lphc )
{
  if( lphc->wState & CBF_CAPTURE )
  {
    lphc->wState &= ~CBF_CAPTURE;
    if( CB_GETTYPE(lphc) == CBS_DROPDOWN )
    {
      GXINT index = CBUpdateLBox( lphc, TRUE );
      /* Update edit only if item is in the list */
      if(index >= 0)
      {
        lphc->wState |= CBF_NOLBSELECT;
        CBUpdateEdit( lphc, index );
        lphc->wState &= ~CBF_NOLBSELECT;
      }
    }
    gxReleaseCapture();
    gxSetCapture(lphc->hWndLBox);
  }

  if( lphc->wState & CBF_BUTTONDOWN )
  {
    lphc->wState &= ~CBF_BUTTONDOWN;
    CBRepaintButton( lphc );
  }
}

/***********************************************************************
*           COMBO_MouseMove
*
* Two things to do - track combo button and release capture when
* pointer goes into the listbox.
*/
static void COMBO_MouseMove( GXLPHEADCOMBO lphc, WPARAM wParam, LPARAM lParam )
{
  GXPOINT  pt;
  GXRECT   lbRect;

  pt.x = (short)LOWORD(lParam);
  pt.y = (short)HIWORD(lParam);

  if( lphc->wState & CBF_BUTTONDOWN )
  {
    GXBOOL bButton;

    bButton = gxPtInRect(&lphc->buttonRect, pt);

    if( !bButton )
    {
      lphc->wState &= ~CBF_BUTTONDOWN;
      CBRepaintButton( lphc );
    }
  }

  gxGetClientRect( lphc->hWndLBox, &lbRect );
  gxMapWindowPoints( lphc->self, lphc->hWndLBox, &pt, 1 );
  if( gxPtInRect(&lbRect, pt) )
  {
    lphc->wState &= ~CBF_CAPTURE;
    gxReleaseCapture();
    if( CB_GETTYPE(lphc) == CBS_DROPDOWN ) CBUpdateLBox( lphc, TRUE );

    /* hand over pointer tracking */
    gxSendMessageW(lphc->hWndLBox, WM_LBUTTONDOWN, wParam, lParam);
  }
}

static LRESULT COMBO_GetComboBoxInfo(const GXHEADCOMBO *lphc, GXCOMBOBOXINFO *pcbi)
{
  if (!pcbi || (pcbi->cbSize < sizeof(GXCOMBOBOXINFO)))
    return FALSE;

  pcbi->rcItem = lphc->textRect;
  pcbi->rcButton = lphc->buttonRect;
  pcbi->stateButton = 0;
  if (lphc->wState & CBF_BUTTONDOWN)
    pcbi->stateButton |= STATE_SYSTEM_PRESSED;
  if (gxIsRectEmpty(&lphc->buttonRect))
    pcbi->stateButton |= STATE_SYSTEM_INVISIBLE;
  pcbi->hwndCombo = lphc->self;
  pcbi->hwndItem = lphc->hWndEdit;
  pcbi->hwndList = lphc->hWndLBox;
  return TRUE;
}

static char *strdupA(LPCSTR str)
{
  char *ret;
  DWORD len;

  if(!str) return NULL;

  len = strlen(str);
  ret = (char*)gxHeapAlloc(gxGetProcessHeap(), 0, len + 1);
  memcpy(ret, str, len + 1);
  return ret;
}

/***********************************************************************
*           ComboWndProc_common
*/
LRESULT ComboWndProc_common( GXHWND hwnd, GXUINT message, WPARAM wParam, LPARAM lParam, GXBOOL unicode )
{
  GXLPHEADCOMBO lphc = (GXLPHEADCOMBO)gxGetWindowLongPtrW( hwnd, 0 );

  //TRACE("[%p]: msg %s wp %08lx lp %08lx\n",
  //      hwnd, SPY_GetMsgName(message, hwnd), wParam, lParam );

  if (!gxIsWindow(hwnd)) return 0;

  if( lphc || message == WM_NCCREATE )
    switch(message)
  {

    /* System messages */

    case WM_NCCREATE:
      {
        LONG style = unicode ? ((GXLPCREATESTRUCTW)lParam)->style :
          ((GXLPCREATESTRUCTA)lParam)->style;
      return COMBO_NCCreate(hwnd, style);
      }
    case WM_NCDESTROY:
      COMBO_NCDestroy(lphc);
      break;/* -> DefWindowProc */

    case WM_CREATE:
      {
        GXHWND hwndParent;
        LONG style;
        if(unicode)
        {
          hwndParent = ((GXLPCREATESTRUCTW)lParam)->hwndParent;
          style = ((GXLPCREATESTRUCTW)lParam)->style;
        }
        else
        {
          hwndParent = ((GXLPCREATESTRUCTA)lParam)->hwndParent;
          style = ((GXLPCREATESTRUCTA)lParam)->style;
        }
        return COMBO_Create(hwnd, lphc, hwndParent, style, unicode);
      }

    case WM_PRINTCLIENT:
      /* Fallthrough */
    case WM_PAINT:
      /* wParam may contain a valid HDC! */
      return  COMBO_Paint(lphc, (GXHDC)wParam);

    case WM_ERASEBKGND:
      /* do all painting in WM_PAINT like Windows does */
      return 1;

    case WM_GETDLGCODE:
      {
        LRESULT result = DLGC_WANTARROWS | DLGC_WANTCHARS;
        if (lParam && (((LPMSG)lParam)->message == WM_KEYDOWN))
        {
          int vk = (int)((LPMSG)lParam)->wParam;

          if ((vk == VK_RETURN || vk == VK_ESCAPE) && (lphc->wState & CBF_DROPPED))
            result |= DLGC_WANTMESSAGE;
        }
        return  result;
      }
    case WM_SIZE:
      if( lphc->hWndLBox &&
        !(lphc->wState & CBF_NORESIZE) ) COMBO_Size( lphc );
      return  TRUE;
    case WM_SETFONT:
      COMBO_Font( lphc, (GXHFONT)wParam, (GXBOOL)lParam );
      return  TRUE;
    case WM_GETFONT:
      return  (LRESULT)lphc->hFont;
    case WM_SETFOCUS:
      if( lphc->wState & CBF_EDIT ) {
        gxSetFocus( lphc->hWndEdit );
        /* The first time focus is received, select all the text */
        if( !(lphc->wState & CBF_BEENFOCUSED) ) {
          gxSendMessageW(lphc->hWndEdit, EM_SETSEL, 0, -1);
          lphc->wState |= CBF_BEENFOCUSED;
        }
      }
      else
        COMBO_SetFocus( lphc );
      return  TRUE;
    case WM_KILLFOCUS:
      {
        GXHWND hwndFocus = WIN_GetFullHandle( (GXHWND)wParam );
        if( !hwndFocus ||
          (hwndFocus != lphc->hWndEdit && hwndFocus != lphc->hWndLBox ))
          COMBO_KillFocus( lphc );
        return  TRUE;
      }
    case WM_COMMAND:
      return  COMBO_Command( lphc, wParam, WIN_GetFullHandle( (GXHWND)lParam ) );
    case WM_GETTEXT:
      return unicode ? COMBO_GetTextW( lphc, wParam, (GXLPWSTR)lParam )
        : COMBO_GetTextA( lphc, wParam, (GXLPSTR)lParam );
    case WM_SETTEXT:
    case WM_GETTEXTLENGTH:
    case WM_CLEAR:
      if ((message == WM_GETTEXTLENGTH) && !ISWIN31 && !(lphc->wState & CBF_EDIT))
      {
        int j = gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
        if (j == -1) return 0;
        return unicode ? gxSendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, j, 0) :
          gxSendMessageA(lphc->hWndLBox, LB_GETTEXTLEN, j, 0);
      }
      else if( lphc->wState & CBF_EDIT )
      {
        LRESULT ret;
        lphc->wState |= CBF_NOEDITNOTIFY;
        ret = unicode ? gxSendMessageW(lphc->hWndEdit, message, wParam, lParam) :
          gxSendMessageA(lphc->hWndEdit, message, wParam, lParam);
        lphc->wState &= ~CBF_NOEDITNOTIFY;
        return ret;
      }
      else return CB_ERR;
    case WM_CUT:
    case WM_PASTE:
    case WM_COPY:
      if( lphc->wState & CBF_EDIT )
      {
        return unicode ? gxSendMessageW(lphc->hWndEdit, message, wParam, lParam) :
          gxSendMessageA(lphc->hWndEdit, message, wParam, lParam);
      }
      else return  CB_ERR;

    case WM_DRAWITEM:
    case WM_DELETEITEM:
    case WM_COMPAREITEM:
    case WM_MEASUREITEM:
      return COMBO_ItemOp(lphc, message, lParam);
    case WM_ENABLE:
      if( lphc->wState & CBF_EDIT )
        gxEnableWindow( lphc->hWndEdit, (GXBOOL)wParam );
      gxEnableWindow( lphc->hWndLBox, (GXBOOL)wParam );

      /* Force the control to repaint when the enabled state changes. */
      gxInvalidateRect(lphc->self, NULL, TRUE);
      return  TRUE;
    case WM_SETREDRAW:
      if( wParam )
        lphc->wState &= ~CBF_NOREDRAW;
      else
        lphc->wState |= CBF_NOREDRAW;

      if( lphc->wState & CBF_EDIT )
        gxSendMessageW(lphc->hWndEdit, message, wParam, lParam);
      gxSendMessageW(lphc->hWndLBox, message, wParam, lParam);
      return  0;
    case WM_SYSKEYDOWN:
      if( KEYDATA_ALT & HIWORD(lParam) )
        if( wParam == VK_UP || wParam == VK_DOWN )
          COMBO_FlipListbox( lphc, FALSE, FALSE );
      return  0;

    case WM_KEYDOWN:
      if ((wParam == VK_RETURN || wParam == VK_ESCAPE) &&
        (lphc->wState & CBF_DROPPED))
      {
        CBRollUp( lphc, wParam == VK_RETURN, FALSE );
        return TRUE;
      }
      else if ((wParam == VK_F4) && !(lphc->wState & CBF_EUI))
      {
        COMBO_FlipListbox( lphc, FALSE, FALSE );
        return TRUE;
      }
      /* fall through */
    case WM_CHAR:
    case WM_IME_CHAR:
      {
        GXHWND hwndTarget;

        if( lphc->wState & CBF_EDIT )
          hwndTarget = lphc->hWndEdit;
        else
          hwndTarget = lphc->hWndLBox;

        return unicode ? gxSendMessageW(hwndTarget, message, wParam, lParam) :
          gxSendMessageA(hwndTarget, message, wParam, lParam);
      }
    case WM_LBUTTONDOWN:
      if( !(lphc->wState & CBF_FOCUSED) ) gxSetFocus( lphc->self );
      if( lphc->wState & CBF_FOCUSED ) COMBO_LButtonDown( lphc, lParam );
      return  TRUE;
    case WM_LBUTTONUP:
      COMBO_LButtonUp( lphc );
      return  TRUE;
    case WM_MOUSEMOVE:
      if( lphc->wState & CBF_CAPTURE )
        COMBO_MouseMove( lphc, wParam, lParam );
      return  TRUE;

    case WM_MOUSEWHEEL:
      if (wParam & (MK_SHIFT | MK_CONTROL))
        return unicode ? gxDefWindowProcW(hwnd, message, wParam, lParam) :
        gxDefWindowProcA(hwnd, message, wParam, lParam);

      if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) return gxSendMessageW(hwnd, WM_KEYDOWN, VK_UP, 0);
      if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) return gxSendMessageW(hwnd, WM_KEYDOWN, VK_DOWN, 0);
      return TRUE;

      /* Combo messages */

    case GXCB_ADDSTRING:
      if( unicode )
      {
        if( lphc->dwStyle & CBS_LOWERCASE )
          CharLowerW((GXLPWSTR)lParam);
        else if( lphc->dwStyle & CBS_UPPERCASE )
          CharUpperW((GXLPWSTR)lParam);
        return gxSendMessageW(lphc->hWndLBox, LB_ADDSTRING, 0, lParam);
      }
      else /* unlike the unicode version, the ansi version does not overwrite
           the string if converting case */
      {
        char *string = NULL;
        LRESULT ret;
        if( lphc->dwStyle & CBS_LOWERCASE )
        {
          string = strdupA((GXLPSTR)lParam);
          CharLowerA(string);
        }

        else if( lphc->dwStyle & CBS_UPPERCASE )
        {
          string = strdupA((GXLPSTR)lParam);
          CharUpperA(string);
        }

        ret = gxSendMessageA(lphc->hWndLBox, LB_ADDSTRING, 0, string ? (LPARAM)string : lParam);
        gxHeapFree(gxGetProcessHeap(), 0, string);
        return ret;
      }
    case GXCB_INSERTSTRING:
      if( unicode )
      {
        if( lphc->dwStyle & CBS_LOWERCASE )
          CharLowerW((GXLPWSTR)lParam);
        else if( lphc->dwStyle & CBS_UPPERCASE )
          CharUpperW((GXLPWSTR)lParam);
        return gxSendMessageW(lphc->hWndLBox, LB_INSERTSTRING, wParam, lParam);
      }
      else
      {
        if( lphc->dwStyle & CBS_LOWERCASE )
          CharLowerA((GXLPSTR)lParam);
        else if( lphc->dwStyle & CBS_UPPERCASE )
          CharUpperA((GXLPSTR)lParam);

        return gxSendMessageA(lphc->hWndLBox, LB_INSERTSTRING, wParam, lParam);
      }
    case GXCB_DELETESTRING:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_DELETESTRING, wParam, 0) :
        gxSendMessageA(lphc->hWndLBox, LB_DELETESTRING, wParam, 0);
    case GXCB_SELECTSTRING:
      return COMBO_SelectString(lphc, (GXINT)wParam, lParam, unicode);
    case GXCB_FINDSTRING:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_FINDSTRING, wParam, lParam) :
        gxSendMessageA(lphc->hWndLBox, LB_FINDSTRING, wParam, lParam);
    case GXCB_FINDSTRINGEXACT:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_FINDSTRINGEXACT, wParam, lParam) :
        gxSendMessageA(lphc->hWndLBox, LB_FINDSTRINGEXACT, wParam, lParam);
    case GXCB_SETITEMHEIGHT:
      return  COMBO_SetItemHeight( lphc, (GXINT)wParam, (GXINT)lParam);
    case GXCB_GETITEMHEIGHT:
      if( (GXINT)wParam >= 0 )	/* listbox item */
        return gxSendMessageW(lphc->hWndLBox, LB_GETITEMHEIGHT, wParam, 0);
      return  CBGetTextAreaHeight(hwnd, lphc);
    case GXCB_RESETCONTENT:
      gxSendMessageW(lphc->hWndLBox, LB_RESETCONTENT, 0, 0);
      if( (lphc->wState & CBF_EDIT) && CB_HASSTRINGS(lphc) )
      {
        static const GXWCHAR empty_stringW[] = { 0 };
        gxSendMessageW(lphc->hWndEdit, WM_SETTEXT, 0, (LPARAM)empty_stringW);
      }
      else
        gxInvalidateRect(lphc->self, NULL, TRUE);
      return  TRUE;
    case GXCB_INITSTORAGE:
      return gxSendMessageW(lphc->hWndLBox, LB_INITSTORAGE, wParam, lParam);
    case GXCB_GETHORIZONTALEXTENT:
      return gxSendMessageW(lphc->hWndLBox, LB_GETHORIZONTALEXTENT, 0, 0);
    case GXCB_SETHORIZONTALEXTENT:
      return gxSendMessageW(lphc->hWndLBox, LB_SETHORIZONTALEXTENT, wParam, 0);
    case GXCB_GETTOPINDEX:
      return gxSendMessageW(lphc->hWndLBox, LB_GETTOPINDEX, 0, 0);
    case GXCB_GETLOCALE:
      return gxSendMessageW(lphc->hWndLBox, LB_GETLOCALE, 0, 0);
    case GXCB_SETLOCALE:
      return gxSendMessageW(lphc->hWndLBox, LB_SETLOCALE, wParam, 0);
    case GXCB_SETDROPPEDWIDTH:
      if( (CB_GETTYPE(lphc) == CBS_SIMPLE) ||
        (GXINT)wParam >= 32768 )
        return CB_ERR;
      /* new value must be higher than combobox width */
      if((GXINT)wParam >= lphc->droppedRect.right - lphc->droppedRect.left)
        lphc->droppedWidth = wParam;
      else if(wParam)
        lphc->droppedWidth = 0;

      /* recalculate the combobox area */
      CBCalcPlacement(hwnd, lphc, &lphc->textRect, &lphc->buttonRect, &lphc->droppedRect );

      /* fall through */
    case GXCB_GETDROPPEDWIDTH:
      if( lphc->droppedWidth )
        return  lphc->droppedWidth;
      return  lphc->droppedRect.right - lphc->droppedRect.left;
    case GXCB_GETDROPPEDCONTROLRECT:
      if( lParam ) CBGetDroppedControlRect(lphc, (GXLPRECT)lParam );
      return  CB_OKAY;
    case GXCB_GETDROPPEDSTATE:
      return (lphc->wState & CBF_DROPPED) != 0;
    case GXCB_DIR:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_DIR, wParam, lParam) :
        gxSendMessageA(lphc->hWndLBox, LB_DIR, wParam, lParam);

    case GXCB_SHOWDROPDOWN:
      if( CB_GETTYPE(lphc) != CBS_SIMPLE )
      {
        if( wParam )
        {
          if( !(lphc->wState & CBF_DROPPED) )
            CBDropDown( lphc );
        }
        else
          if( lphc->wState & CBF_DROPPED )
            CBRollUp( lphc, FALSE, TRUE );
      }
      return  TRUE;
    case GXCB_GETCOUNT:
      return gxSendMessageW(lphc->hWndLBox, LB_GETCOUNT, 0, 0);
    case GXCB_GETCURSEL:
      return gxSendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
    case GXCB_SETCURSEL:
      lParam = gxSendMessageW(lphc->hWndLBox, LB_SETCURSEL, wParam, 0);
      if( lParam >= 0 )
        gxSendMessageW(lphc->hWndLBox, LB_SETTOPINDEX, wParam, 0);

      /* no LBN_SELCHANGE in this case, update manually */
      if( lphc->wState & CBF_EDIT )
        CBUpdateEdit( lphc, (GXINT)wParam );
      else
        gxInvalidateRect(lphc->self, &lphc->textRect, TRUE);
      lphc->wState &= ~CBF_SELCHANGE;
      return  lParam;
    case GXCB_GETLBTEXT:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_GETTEXT, wParam, lParam) :
        gxSendMessageA(lphc->hWndLBox, LB_GETTEXT, wParam, lParam);
    case GXCB_GETLBTEXTLEN:
      return unicode ? gxSendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, wParam, 0) :
        gxSendMessageA(lphc->hWndLBox, LB_GETTEXTLEN, wParam, 0);
    case GXCB_GETITEMDATA:
      return gxSendMessageW(lphc->hWndLBox, LB_GETITEMDATA, wParam, 0);
    case GXCB_SETITEMDATA:
      return gxSendMessageW(lphc->hWndLBox, LB_SETITEMDATA, wParam, lParam);
    case GXCB_GETEDITSEL:
      /* Edit checks passed parameters itself */
      if( lphc->wState & CBF_EDIT )
        return gxSendMessageW(lphc->hWndEdit, EM_GETSEL, wParam, lParam);
      return  CB_ERR;
    case GXCB_SETEDITSEL:
      if( lphc->wState & CBF_EDIT )
        return gxSendMessageW(lphc->hWndEdit, EM_SETSEL,
        (GXINT)(SHORT)LOWORD(lParam), (GXINT)(SHORT)HIWORD(lParam) );
      return  CB_ERR;
    case GXCB_SETEXTENDEDUI:
      if( CB_GETTYPE(lphc) == CBS_SIMPLE )
        return  CB_ERR;
      if( wParam )
        lphc->wState |= CBF_EUI;
      else lphc->wState &= ~CBF_EUI;
      return  CB_OKAY;
    case GXCB_GETEXTENDEDUI:
      return (lphc->wState & CBF_EUI) != 0;
    case GXCB_GETCOMBOBOXINFO:
      return COMBO_GetComboBoxInfo(lphc, (GXCOMBOBOXINFO *)lParam);
    case GXCB_LIMITTEXT:
      if( lphc->wState & CBF_EDIT )
        return gxSendMessageW(lphc->hWndEdit, EM_LIMITTEXT, wParam, lParam);
      return  TRUE;
    default:
      if (message >= WM_USER)
        WARN("unknown msg WM_USER+%04x wp=%04lx lp=%08lx\n",
        message - WM_USER, wParam, lParam );
      break;
  }
  return unicode ? gxDefWindowProcW(hwnd, message, wParam, lParam) :
    gxDefWindowProcA(hwnd, message, wParam, lParam);
}

/*************************************************************************
*           GetComboBoxInfo   (USER32.@)
*/
GXBOOL WINAPI GetComboBoxInfo(GXHWND hwndCombo,      /* [in] handle to combo box */
  PCOMBOBOXINFO pcbi   /* [in/out] combo box information */)
{
  TRACE("(%p, %p)\n", hwndCombo, pcbi);
  return gxSendMessageW(hwndCombo, CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
}
