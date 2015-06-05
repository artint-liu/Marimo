#ifndef _DEV_DISABLE_UI_CODE
/*
* Menu functions
*
* Copyright 1993 Martin Ayotte
* Copyright 1994 Alexandre Julliard
* Copyright 1997 Morten Welinder
* Copyright 2005 Maxime Belleng? * Copyright 2006 Phil Krylov
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
*/

/*
* Note: the style MF_MOUSESELECT is used to mark popup items that
* have been selected, i.e. their popup menu is currently displayed.
* This is probably not the meaning this style has in MS-Windows.
*
* Note 2: where there is a difference, these menu API's are according
* the behavior of Windows 2k and Windows XP. Known differences with
* Windows 9x/ME are documented in the comments, in case an application
* is found to depend on the old behavior.
* 
* TODO:
*    implements styles :
*        - MNS_AUTODISMISS
*        - MNS_DRAGDROP
*        - MNS_MODELESS
*/

//#include "config.h"
//#include "wine/port.h"
#include <GrapX.H>
#include <User/GrapX.Hxx>
#include <GXStation.H>
//#include <User/GXWindow.h>
#include "GrapX/GXUser.H"
#include "GrapX/GXKernel.h"
#include "GrapX/GXGDI.H"

#include <stdarg.h>
#include <string.h>

#include <GrapX/WineComm.H>
#include "GrapX/gxError.H"

#if !defined(_WINE_MENU_1_3_20_)
#define OEMRESOURCE

#pragma warning( disable : 4244 )  // _w64型数与普通类型的相互转换
#pragma warning( disable : 4018 )  // 无符号与有符号数比较

//#include "windef.h"
//#include "winbase.h"
//#include "wingdi.h"
//#include "winnls.h"
//#include "wine/winbase16.h"
//#include "wine/winuser16.h"
//#include "wownt32.h"
//#include "wine/server.h"
//#include "wine/unicode.h"
//#include "win.h"
//#include "controls.h"
//#include "user_private.h"
//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(menu);
//WINE_DECLARE_DEBUG_CHANNEL(accel);

/* internal popup menu window messages */

#define MM_SETMENUHANDLE  (GXWM_USER + 0)
#define MM_GETMENUHANDLE  (GXWM_USER + 1)

/* Menu item structure */
typedef struct {
  /* ----------- MENUITEMINFO Stuff ----------- */
  GXUINT fType;      /* Item type. */
  GXUINT fState;    /* Item state.  */
  GXUINT_PTR wID;    /* Item id.  */
  GXHMENU hSubMenu;    /* Pop-up menu.  */
  GXHBITMAP hCheckBit;    /* Bitmap when checked.  */
  GXHBITMAP hUnCheckBit;  /* Bitmap when unchecked.  */
  GXLPWSTR text;    /* Item text. */
  GXULONG_PTR dwItemData;  /* Application defined.  */
  GXLPWSTR dwTypeData;    /* depends on fMask */
  GXHBITMAP hbmpItem;    /* bitmap */
  /* ----------- Wine stuff ----------- */
  GXRECT      rect;    /* Item area (relative to menu window) */
  GXUINT      xTab;    /* X position of text after Tab */
  GXSIZE   bmpsize;             /* size needed for the HBMMENU_CALLBACK
                * bitmap */ 
} MENUITEM;

/* Popup menu structure */
typedef struct {
  GXWORD        wFlags;       /* Menu flags (MF_POPUP, MF_SYSMENU) */
  GXWORD        wMagic;       /* Magic number */
  GXWORD    Width;        /* Width of the whole menu */
  GXWORD    Height;       /* Height of the whole menu */
  GXUINT      nItems;       /* Number of items in the menu */
  GXHWND      hWnd;         /* Window containing the menu */
  MENUITEM    *items;       /* Array of menu items */
  GXUINT      FocusedItem;  /* Currently focused item */
  GXHWND    hWndOwner;    /* window receiving the messages for ownerdraw */
  GXBOOL    bTimeToHide;  /* Request hiding when receiving a second click in the top-level menu item */
  GXBOOL    bScrolling;   /* Scroll arrows are active */
  GXUINT    nScrollPos;   /* Current scroll position */
  GXUINT    nTotalHeight; /* Total height of menu items inside menu */
  /* ------------ MENUINFO members ------ */
  GXDWORD    dwStyle;  /* Extended menu style */
  GXUINT    cyMax;    /* max height of the whole menu, 0 is screen height */
  GXHBRUSH  hbrBack;  /* brush for menu background */
  GXDWORD    dwContextHelpID;
  GXDWORD    dwMenuData;  /* application defined value */
  GXHMENU    hSysMenuOwner;  /* Handle to the dummy sys menu holder */
  GXSIZE    maxBmpSize;     /* Maximum size of the bitmap items */
} POPUPMENU, *LPPOPUPMENU;

/* internal flags for menu tracking */

#define TF_ENDMENU              0x0001
#define TF_SUSPENDPOPUP         0x0002
#define TF_SKIPREMOVE      0x0004

typedef struct
{
  GXUINT  trackFlags;
  GXHMENU  hCurrentMenu; /* current submenu (can be equal to hTopMenu)*/
  GXHMENU  hTopMenu;     /* initial menu */
  GXHWND  hOwnerWnd;    /* where notifications are sent */
  GXPOINT  pt;
} MTRACKER;

#define MENU_MAGIC   0x554d  /* 'MU' */

#define ITEM_PREV    -1
#define ITEM_NEXT     1

/* Internal MENU_TrackMenu() flags */
#define GXTPM_INTERNAL    0xF0000000
#define GXTPM_ENTERIDLEEX     0x80000000    /* set owner window for GXWM_ENTERIDLE */
#define GXTPM_BUTTONDOWN    0x40000000    /* menu was clicked before tracking */
#define GXTPM_POPUPMENU           0x20000000              /* menu is a popup menu */

/* Space between 2 columns */
#define MENU_COL_SPACE 4

/*  top and bottom margins for popup menus */
#define MENU_TOP_MARGIN 3
#define MENU_BOTTOM_MARGIN 2

/* (other menu->FocusedItem values give the position of the focused item) */
#define NO_SELECTED_ITEM  0xffff

#define MENU_ITEM_TYPE(flags) \
  ((flags) & (GXMF_STRING | GXMF_BITMAP | GXMF_OWNERDRAW | GXMF_SEPARATOR))

/* macro to test that flags do not indicate bitmap, ownerdraw or separator */
#define IS_STRING_ITEM(flags) (MENU_ITEM_TYPE ((flags)) == GXMF_STRING)
#define IS_MAGIC_BITMAP(id)     ((id) && ((GXINT_PTR)(id) < 12) && ((GXINT_PTR)(id) >= -1))

#define IS_SYSTEM_MENU(menu)  \
  (!((menu)->wFlags & GXMF_POPUP) && ((menu)->wFlags & GXMF_SYSMENU))

#define MENUITEMINFO_TYPE_MASK \
  (GXMFT_STRING | GXMFT_BITMAP | GXMFT_OWNERDRAW | GXMFT_SEPARATOR | \
  GXMFT_MENUBARBREAK | GXMFT_MENUBREAK | GXMFT_RADIOCHECK | \
  GXMFT_RIGHTORDER | GXMFT_RIGHTJUSTIFY /* same as GXMF_HELP */ )
#define TYPE_MASK  (MENUITEMINFO_TYPE_MASK | GXMF_POPUP | GXMF_SYSMENU)
#define STATE_MASK (~TYPE_MASK)
#define MENUITEMINFO_STATE_MASK (STATE_MASK & ~(GXMF_BYPOSITION | GXMF_MOUSESELECT))

#define WIN_ALLOWED_MENU(style) ((style & (GXWS_CHILD | GXWS_POPUP)) != GXWS_CHILD)

static GXSIZE     menucharsize;
static GXUINT     ODitemheight; /* default owner drawn item height */      

/* Use global popup window because there's no way 2 menus can
* be tracked at the same time.  */
static GXHWND top_popup;

/* Flag set by EndMenu() to force an exit from menu tracking */
static GXBOOL fEndMenu = FALSE;

static GXLRESULT GXCALLBACK PopupMenuWndProc( GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam );

GXDWORD DrawMenuBarTemp(GXHWND hWnd, GXHDC hDC, LPGXRECT lprect, GXHMENU hMenu, GXHFONT hFont);

/*********************************************************************
* menu class descriptor
*/
//const struct builtin_class_descr MENU_builtin_class =
//{
//    (GXLPCWSTR)POPUPMENU_CLASS_ATOM,  /* name */
//    CS_DROPSHADOW | CS_SAVEBITS | CS_DBLCLKS,  /* style */
//    NULL,                          /* procA (winproc is Unicode only) */
//    PopupMenuWndProc,              /* procW */
//    sizeof(GXHMENU),                 /* extra */
//    IDC_ARROW,                     /* cursor */
//    (HBRUSH)(GXCOLOR_MENU+1)         /* brush */
//};
GXWNDCLASSEX WndClassEx_Menu = { sizeof(GXWNDCLASSEX), GXCS_DROPSHADOW | GXCS_SAVEBITS | GXCS_DBLCLKS, PopupMenuWndProc, 0L, sizeof(GXHMENU),
(GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_IBEAM), NULL, NULL,
GXWE_MENUW, NULL };

#define GXPOPUPMENU_CLASS_ATOM  (WndClassEx_Menu.lpszClassName)

/***********************************************************************
*           debug_print_menuitem
*
* Print a menuitem in readable form.
*/

#define debug_print_menuitem(pre, mp, post) //\
  //do { if (TRACE_ON(menu)) do_debug_print_menuitem(pre, mp, post); } while (0)

#define MENUOUT(text) \
  TRACE("%s%s", (count++ ? "," : ""), (text))

#define MENUFLAG(bit,text) \
  do { \
  if (flags & (bit)) { flags &= ~(bit); MENUOUT ((text)); } \
  } while (0)

static void do_debug_print_menuitem(const char *prefix, const MENUITEM *mp,
                  const char *postfix)
{
  static const char * const hbmmenus[] = { "HBMMENU_CALLBACK", "", "HBMMENU_SYSTEM",
    "HBMMENU_MBAR_RESTORE", "HBMMENU_MBAR_MINIMIZE", "UNKNOWN BITMAP", "HBMMENU_MBAR_CLOSE",
    "HBMMENU_MBAR_CLOSE_D", "HBMMENU_MBAR_MINIMIZE_D", "HBMMENU_POPUP_CLOSE",
    "HBMMENU_POPUP_RESTORE", "HBMMENU_POPUP_MAXIMIZE", "HBMMENU_POPUP_MINIMIZE"};
  TRACE("%s ", prefix);
  if (mp) {
    GXUINT flags = mp->fType;
    TRACE( "{ ID=0x%lx", mp->wID);
    if ( mp->hSubMenu)
      TRACE( ", Sub=%p", mp->hSubMenu);
    if (flags) {
      int count = 0;
      TRACE( ", fType=");
      MENUFLAG( GXMFT_SEPARATOR, "sep");
      MENUFLAG( GXMFT_OWNERDRAW, "own");
      MENUFLAG( GXMFT_BITMAP, "bit");
      MENUFLAG(GXMF_POPUP, "pop");
      MENUFLAG(GXMFT_MENUBARBREAK, "barbrk");
      MENUFLAG(GXMFT_MENUBREAK, "brk");
      MENUFLAG(GXMFT_RADIOCHECK, "radio");
      MENUFLAG(GXMFT_RIGHTORDER, "rorder");
      MENUFLAG(GXMF_SYSMENU, "sys");
      MENUFLAG(GXMFT_RIGHTJUSTIFY, "right");  /* same as GXMF_HELP */
      if (flags)
        TRACE( "+0x%x", flags);
    }
    flags = mp->fState;
    if (flags) {
      int count = 0;
      TRACE( ", State=");
      MENUFLAG(GXMFS_GRAYED, "grey");
      MENUFLAG(GXMFS_DEFAULT, "default");
      MENUFLAG(GXMFS_DISABLED, "dis");
      MENUFLAG(GXMFS_CHECKED, "check");
      MENUFLAG(GXMFS_HILITE, "hi");
      MENUFLAG(GXMF_USECHECKBITMAPS, "usebit");
      MENUFLAG(GXMF_MOUSESELECT, "mouse");
      if (flags)
        TRACE( "+0x%x", flags);
    }
    if (mp->hCheckBit)
      TRACE( ", Chk=%p", mp->hCheckBit);
    if (mp->hUnCheckBit)
      TRACE( ", Unc=%p", mp->hUnCheckBit);
    if (mp->text)
      TRACE( ", Text=%s", debugstr_w(mp->text));
    if (mp->dwItemData)
      TRACE( ", ItemData=0x%08lx", mp->dwItemData);
    if (mp->hbmpItem)
    {
      if( IS_MAGIC_BITMAP(mp->hbmpItem))
        TRACE( ", hbitmap=%s", hbmmenus[ (GXINT_PTR)mp->hbmpItem + 1]);
      else
        TRACE( ", hbitmap=%p", mp->hbmpItem);
    }
    TRACE( " }");
  } else
    TRACE( "NULL");
  TRACE(" %s\n", postfix);
}

#undef MENUOUT
#undef MENUFLAG


/***********************************************************************
*           MENU_GetMenu
*
* Validate the given menu handle and returns the menu structure pointer.
*/
static POPUPMENU *MENU_GetMenu(GXHMENU hMenu)
{
  POPUPMENU *menu = (POPUPMENU *)USER_HEAP_LIN_ADDR(hMenu);
  if (!menu || menu->wMagic != MENU_MAGIC)
  {
    WARN("invalid menu handle=%p, ptr=%p, magic=%x\n", hMenu, menu, menu? menu->wMagic:0);
    menu = NULL;
  }
  return menu;
}

/***********************************************************************
*           get_win_sys_menu
*
* Get the system menu of a window
*/
static GXHMENU get_win_sys_menu( GXHWND hWnd )
{
  GXHMENU ret = 0;
  GXWnd *win = WIN_GetPtr( hWnd );
  //Artint ??
  ASSERT(FALSE);
  //if (win && win != WND_OTHER_PROCESS && win != WND_DESKTOP)
  {
    ret = win->hSysMenu;
    //Artint ??
    ASSERT(FALSE);
    //WIN_ReleasePtr( win );
  }
  return ret;
}

/***********************************************************************
*           get_menu_font
*/
static GXHFONT get_menu_font( GXBOOL bold )
{
  static GXHFONT hMenuFont, hMenuFontBold;

  GXHFONT ret = bold ? hMenuFontBold : hMenuFont;

  if (!ret)
  {
    GXNONCLIENTMETRICSW ncm;
    GXHFONT prev;

    ncm.cbSize = sizeof(GXNONCLIENTMETRICSW);
    gxSystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(GXNONCLIENTMETRICSW), &ncm, 0);

    if (bold)
    {
      ncm.lfMenuFont.lfWeight += 300;
      if (ncm.lfMenuFont.lfWeight > 1000) ncm.lfMenuFont.lfWeight = 1000;
    }
    if (!(ret = gxCreateFontIndirectW((const GXLOGFONTW*)&ncm.lfMenuFont ))) return 0;
    prev = (GXHFONT)gxInterlockedCompareExchangePointer( (void **)(bold ? &hMenuFontBold : &hMenuFont),
      ret, NULL );
    if (prev)
    {
      /* another thread beat us to it */
      gxDeleteObject( ret );
      ret = prev;
    }
  }
  return ret;
}

/***********************************************************************
*           get_arrow_bitmap
*/
static GXHBITMAP get_arrow_bitmap()
{
  static GXHBITMAP arrow_bitmap;

  if (!arrow_bitmap) arrow_bitmap = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_MNARROW));
  return arrow_bitmap;
}

/***********************************************************************
*           get_down_arrow_bitmap
*/
static GXHBITMAP get_down_arrow_bitmap()
{
  static GXHBITMAP arrow_bitmap;

  if (!arrow_bitmap) arrow_bitmap = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_DNARROW));
  return arrow_bitmap;
}

/***********************************************************************
*           get_down_arrow_inactive_bitmap
*/
static GXHBITMAP get_down_arrow_inactive_bitmap()
{
  static GXHBITMAP arrow_bitmap;

  if (!arrow_bitmap) arrow_bitmap = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_DNARROWI));
  return arrow_bitmap;
}

/***********************************************************************
*           get_up_arrow_bitmap
*/
static GXHBITMAP get_up_arrow_bitmap()
{
  static GXHBITMAP arrow_bitmap;

  if (!arrow_bitmap) arrow_bitmap = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_UPARROW));
  return arrow_bitmap;
}

/***********************************************************************
*           get_up_arrow_inactive_bitmap
*/
static GXHBITMAP get_up_arrow_inactive_bitmap()
{
  static GXHBITMAP arrow_bitmap;

  if (!arrow_bitmap) arrow_bitmap = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_UPARROWI));
  return arrow_bitmap;
}

/***********************************************************************
*           MENU_CopySysPopup
*
* Return the default system menu.
*/
static GXHMENU MENU_CopySysPopup()
{
  static const GXWCHAR sysmenuW[] = {'S','Y','S','M','E','N','U',0};
  GXHMENU hMenu = gxLoadMenuW(user32_module, sysmenuW);

  if( hMenu ) {
    POPUPMENU* menu = MENU_GetMenu(hMenu);
    menu->wFlags |= GXMF_SYSMENU | GXMF_POPUP;
    gxSetMenuDefaultItem(hMenu, GXSC_CLOSE, FALSE);
  }
  else
    ERR("Unable to load default system menu\n" );

  TRACE("returning %p.\n", hMenu );

  return hMenu;
}


/**********************************************************************
*           MENU_GetSysMenu
*
* Create a copy of the system menu. System menu in Windows is
* a special menu bar with the single entry - system menu popup.
* This popup is presented to the outside world as a "system menu".
* However, the real system menu handle is sometimes seen in the
* GXWM_MENUSELECT parameters (and Word 6 likes it this way).
*/
static GXHMENU MENU_GetSysMenu( GXHWND hWnd, GXHMENU hPopupMenu )
{
  GXHMENU hMenu;

  TRACE("loading system menu, hWnd %p, hPopupMenu %p\n", hWnd, hPopupMenu);
  if ((hMenu = gxCreateMenu()))
  {
    POPUPMENU *menu = MENU_GetMenu(hMenu);
    menu->wFlags = GXMF_SYSMENU;
    menu->hWnd = WIN_GetFullHandle( hWnd );
    TRACE("hWnd %p (hMenu %p)\n", menu->hWnd, hMenu);

    if (!hPopupMenu)
      hPopupMenu = MENU_CopySysPopup();

    if (hPopupMenu)
    {
      if (gxGetClassLongW(hWnd, GXGCL_STYLE) & GXCS_NOCLOSE)
        gxDeleteMenu(hPopupMenu, GXSC_CLOSE, GXMF_BYCOMMAND);

      gxInsertMenuW( hMenu, -1, GXMF_SYSMENU | GXMF_POPUP | GXMF_BYPOSITION,
        (GXUINT_PTR)hPopupMenu, NULL );

      menu->items[0].fType = GXMF_SYSMENU | GXMF_POPUP;
      menu->items[0].fState = 0;
      if ((menu = MENU_GetMenu(hPopupMenu))) menu->wFlags |= GXMF_SYSMENU;

      TRACE("hMenu=%p (hPopup %p)\n", hMenu, hPopupMenu );
      return hMenu;
    }
    gxDestroyMenu( hMenu );
  }
  ERR("failed to load system menu!\n");
  return 0;
}


/***********************************************************************
*           MENU_InitSysMenuPopup
*
* Grey the appropriate items in System menu.
*/
static void MENU_InitSysMenuPopup( GXHMENU hmenu, GXDWORD style, GXDWORD clsStyle )
{
  GXBOOL gray;

  gray = !(style & GXWS_THICKFRAME) || (style & (GXWS_MAXIMIZE | GXWS_MINIMIZE));
  gxEnableMenuItem( hmenu, GXSC_SIZE, (gray ? GXMF_GRAYED : GXMF_ENABLED) );
  gray = ((style & GXWS_MAXIMIZE) != 0);
  gxEnableMenuItem( hmenu, GXSC_MOVE, (gray ? GXMF_GRAYED : GXMF_ENABLED) );
  gray = !(style & GXWS_MINIMIZEBOX) || (style & GXWS_MINIMIZE);
  gxEnableMenuItem( hmenu, GXSC_MINIMIZE, (gray ? GXMF_GRAYED : GXMF_ENABLED) );
  gray = !(style & GXWS_MAXIMIZEBOX) || (style & GXWS_MAXIMIZE);
  gxEnableMenuItem( hmenu, GXSC_MAXIMIZE, (gray ? GXMF_GRAYED : GXMF_ENABLED) );
  gray = !(style & (GXWS_MAXIMIZE | GXWS_MINIMIZE));
  gxEnableMenuItem( hmenu, GXSC_RESTORE, (gray ? GXMF_GRAYED : GXMF_ENABLED) );
  gray = (clsStyle & GXCS_NOCLOSE) != 0;

  /* The menu item must keep its state if it's disabled */
  if(gray)
    gxEnableMenuItem( hmenu, GXSC_CLOSE, GXMF_GRAYED);
}


/******************************************************************************
*
*   GXUINT  MENU_GetStartOfNextColumn(
*     GXHMENU  hMenu )
*
*****************************************************************************/

static GXUINT  MENU_GetStartOfNextColumn(
  GXHMENU  hMenu )
{
  POPUPMENU *menu = MENU_GetMenu(hMenu);
  GXUINT i;

  if(!menu)
    return NO_SELECTED_ITEM;

  i = menu->FocusedItem + 1;
  if( i == NO_SELECTED_ITEM )
    return i;

  for( ; i < menu->nItems; ++i ) {
    if (menu->items[i].fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK))
      return i;
  }

  return NO_SELECTED_ITEM;
}


/******************************************************************************
*
*   GXUINT  MENU_GetStartOfPrevColumn(
*     GXHMENU  hMenu )
*
*****************************************************************************/

static GXUINT  MENU_GetStartOfPrevColumn(
  GXHMENU  hMenu )
{
  POPUPMENU *menu = MENU_GetMenu(hMenu);
  GXUINT  i;

  if( !menu )
    return NO_SELECTED_ITEM;

  if( menu->FocusedItem == 0 || menu->FocusedItem == NO_SELECTED_ITEM )
    return NO_SELECTED_ITEM;

  /* Find the start of the column */

  for(i = menu->FocusedItem; i != 0 &&
    !(menu->items[i].fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK));
    --i); /* empty */

  if(i == 0)
    return NO_SELECTED_ITEM;

  for(--i; i != 0; --i) {
    if (menu->items[i].fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK))
      break;
  }

  TRACE("ret %d.\n", i );

  return i;
}



/***********************************************************************
*           MENU_FindItem
*
* Find a menu item. Return a pointer on the item, and modifies *hmenu
* in case the item was in a sub-menu.
*/
static MENUITEM *MENU_FindItem( GXHMENU *hmenu, GXUINT *nPos, GXUINT wFlags )
{
  POPUPMENU *menu;
  MENUITEM *fallback = NULL;
  GXUINT fallback_pos = 0;
  GXUINT i;

  if ((*hmenu == (GXHMENU)0xffff) || (!(menu = MENU_GetMenu(*hmenu)))) return NULL;
  if (wFlags & GXMF_BYPOSITION)
  {
    if (*nPos >= menu->nItems) return NULL;
    return &menu->items[*nPos];
  }
  else
  {
    MENUITEM *item = menu->items;
    for (i = 0; i < menu->nItems; i++, item++)
    {
      if (item->fType & GXMF_POPUP)
      {
        GXHMENU hsubmenu = item->hSubMenu;
        MENUITEM *subitem = MENU_FindItem( &hsubmenu, nPos, wFlags );
        if (subitem)
        {
          *hmenu = hsubmenu;
          return subitem;
        }
        else if (item->wID == *nPos)
        {
          /* fallback to this item if nothing else found */
          fallback_pos = i;
          fallback = item;
        }
      }
      else if (item->wID == *nPos)
      {
        *nPos = i;
        return item;
      }
    }
  }

  if (fallback)
    *nPos = fallback_pos;

  return fallback;
}

/***********************************************************************
*           MENU_FindSubMenu
*
* Find a Sub menu. Return the position of the submenu, and modifies
* *hmenu in case it is found in another sub-menu.
* If the submenu cannot be found, NO_SELECTED_ITEM is returned.
*/
GXUINT MENU_FindSubMenu( GXHMENU *hmenu, GXHMENU hSubTarget )
{
  POPUPMENU *menu;
  GXUINT i;
  MENUITEM *item;
  if (((*hmenu)==(GXHMENU)0xffff) ||
    (!(menu = MENU_GetMenu(*hmenu))))
    return NO_SELECTED_ITEM;
  item = menu->items;
  for (i = 0; i < menu->nItems; i++, item++) {
    if(!(item->fType & GXMF_POPUP)) continue;
    if (item->hSubMenu == hSubTarget) {
      return i;
    }
    else  {
      GXHMENU hsubmenu = item->hSubMenu;
      GXUINT pos = MENU_FindSubMenu( &hsubmenu, hSubTarget );
      if (pos != NO_SELECTED_ITEM) {
        *hmenu = hsubmenu;
        return pos;
      }
    }
  }
  return NO_SELECTED_ITEM;
}

/***********************************************************************
*           MENU_FreeItemData
*/
static void MENU_FreeItemData( MENUITEM* item )
{
  /* delete text */
  gxHeapFree( gxGetProcessHeap(), 0, item->text );
}

/***********************************************************************
*           MENU_AdjustMenuItemRect
*
* Adjust menu item gxRectangle according to scrolling state.
*/
static void
MENU_AdjustMenuItemRect(const POPUPMENU *menu, LPGXRECT rect)
{
  if (menu->bScrolling)
  {
    GXUINT arrow_bitmap_width, arrow_bitmap_height;
    GXBITMAP bmp;

    gxGetObjectW(get_up_arrow_bitmap(), sizeof(bmp), &bmp);
    arrow_bitmap_width = bmp.bmWidth;
    arrow_bitmap_height = bmp.bmHeight;
    rect->top += arrow_bitmap_height - menu->nScrollPos;
    rect->bottom += arrow_bitmap_height - menu->nScrollPos;
  }
}


/***********************************************************************
*           MENU_FindItemByCoords
*
* Find the item at the specified coordinates (screen coords). Does
* not work for child windows and therefore should not be called for
* an arbitrary system menu.
*/
static MENUITEM *MENU_FindItemByCoords( const POPUPMENU *menu,
                     GXPOINT pt, GXUINT *pos )
{
  MENUITEM *item;
  GXUINT i;
  GXRECT rect;

  if (!gxGetWindowRect(menu->hWnd, &rect)) return NULL;
  pt.x -= rect.left;
  pt.y -= rect.top;
  item = menu->items;
  for (i = 0; i < menu->nItems; i++, item++)
  {
    rect = item->rect;
    MENU_AdjustMenuItemRect(menu, &rect);
    if (gxPtInRect(&rect, pt))
    {
      if (pos) *pos = i;
      return item;
    }
  }
  return NULL;
}


/***********************************************************************
*           MENU_FindItemByKey
*
* Find the menu item selected by a key press.
* Return item id, -1 if none, -2 if we should close the menu.
*/
static GXUINT MENU_FindItemByKey( GXHWND hWndOwner, GXHMENU hmenu,
                 GXWCHAR key, GXBOOL forceMenuChar )
{
  TRACE("\tlooking for '%c' (0x%02x) in [%p]\n", (char)key, key, hmenu );

  if (!gxIsMenu( hmenu )) hmenu = gxGetSubMenu( get_win_sys_menu(hWndOwner), 0);

  if (hmenu)
  {
    POPUPMENU *menu = MENU_GetMenu( hmenu );
    MENUITEM *item = menu->items;
    GXLRESULT menuchar;

    if( !forceMenuChar )
    {
      GXUINT i;

      for (i = 0; i < menu->nItems; i++, item++)
      {
        if( item->text)
        {
          GXWCHAR *p = item->text - 2;
          do
          {
            p = strchrW (p + 2, '&');
          }
          while (p != NULL && p [1] == '&');
          if (p && (toupperW(p[1]) == toupperW(key))) return i;
        }
      }
    }
    menuchar = gxSendMessageW( hWndOwner, GXWM_MENUCHAR,
      GXMAKEWPARAM( key, menu->wFlags ), (GXLPARAM)hmenu );
    if (GXHIWORD(menuchar) == 2) return GXLOWORD(menuchar);
    if (GXHIWORD(menuchar) == 1) return (GXUINT)(-2);
  }
  return (GXUINT)(-1);
}


/***********************************************************************
*           MENU_GetBitmapItemSize
*
* Get the size of a bitmap item.
*/
static void MENU_GetBitmapItemSize( MENUITEM *lpitem, GXSIZE *size,
                   GXHWND hWndOwner)
{
  GXBITMAP bm;
  GXHBITMAP bmp = lpitem->hbmpItem;

  size->cx = size->cy = 0;

  /* check if there is a magic menu item associated with this item */
  switch( (GXINT_PTR) bmp )
  {
  case (GXINT_PTR)GXHBMMENU_CALLBACK:
    {
      GXMEASUREITEMSTRUCT measItem;
      measItem.CtlType = GXODT_MENU;
      measItem.CtlID = 0;
      measItem.itemID = lpitem->wID;
      measItem.itemWidth = lpitem->rect.right - lpitem->rect.left;
      measItem.itemHeight = lpitem->rect.bottom - lpitem->rect.top;
      measItem.itemData = lpitem->dwItemData;
      gxSendMessageW( hWndOwner, GXWM_MEASUREITEM, lpitem->wID, (GXLPARAM)&measItem);
      size->cx = measItem.itemWidth;
      size->cy = measItem.itemHeight;
      return;
    }
    break;
  case (GXINT_PTR)GXHBMMENU_SYSTEM:
    if (lpitem->dwItemData)
    {
      bmp = (GXHBITMAP)lpitem->dwItemData;
      break;
    }
    /* fall through */
  case (GXINT_PTR)GXHBMMENU_MBAR_RESTORE:
  case (GXINT_PTR)GXHBMMENU_MBAR_MINIMIZE:
  case (GXINT_PTR)GXHBMMENU_MBAR_MINIMIZE_D:
  case (GXINT_PTR)GXHBMMENU_MBAR_CLOSE:
  case (GXINT_PTR)GXHBMMENU_MBAR_CLOSE_D:
    size->cx = gxGetSystemMetrics( GXSM_CYMENU ) - 4;
    size->cy = size->cx;
    return;
  case (GXINT_PTR)GXHBMMENU_POPUP_CLOSE:
  case (GXINT_PTR)GXHBMMENU_POPUP_RESTORE:
  case (GXINT_PTR)GXHBMMENU_POPUP_MAXIMIZE:
  case (GXINT_PTR)GXHBMMENU_POPUP_MINIMIZE:
    FIXME("Magic %p not implemented\n", bmp );
    return;
  }
  if (gxGetObjectW(bmp, sizeof(bm), &bm ))
  {
    size->cx = bm.bmWidth;
    size->cy = bm.bmHeight;
  }
}

/***********************************************************************
*           MENU_DrawBitmapItem
*
* Draw a bitmap item.
*/
static void MENU_DrawBitmapItem( GXHDC hdc, MENUITEM *lpitem, const GXRECT *rect,
                GXHMENU hmenu, GXHWND hWndOwner, GXUINT odaction, GXBOOL menuBar)
{
  GXBITMAP bm;
  GXDWORD rop;
  GXHDC hdcMem;
  GXHBITMAP bmp;
  int w = rect->right - rect->left;
  int h = rect->bottom - rect->top;
  int bmp_xoffset = 0;
  int left, top;
  GXHBITMAP hbmToDraw = lpitem->hbmpItem;
  bmp = hbmToDraw;

  /* Check if there is a magic menu item associated with this item */
  if (IS_MAGIC_BITMAP(hbmToDraw))
  {
    GXUINT flags = 0;
    GXRECT r;

    switch((GXINT_PTR)hbmToDraw)
    {
    case (GXINT_PTR)GXHBMMENU_SYSTEM:
      if (lpitem->dwItemData)
      {
        bmp = (GXHBITMAP)lpitem->dwItemData;
        if (!gxGetObjectW( bmp, sizeof(bm), &bm )) return;
      }
      else
      {
        static GXHBITMAP hBmpSysMenu;

        if (!hBmpSysMenu) hBmpSysMenu = gxLoadBitmapW(0, GXMAKEINTRESOURCEW(GXOBM_CLOSE));
        bmp = hBmpSysMenu;
        if (!gxGetObjectW( bmp, sizeof(bm), &bm )) return;
        /* only use right half of the bitmap */
        bmp_xoffset = bm.bmWidth / 2;
        bm.bmWidth -= bmp_xoffset;
      }
      goto got_bitmap;
    case (GXINT_PTR)GXHBMMENU_MBAR_RESTORE:
      flags = GXDFCS_CAPTIONRESTORE;
      break;
    case (GXINT_PTR)GXHBMMENU_MBAR_MINIMIZE:
      flags = GXDFCS_CAPTIONMIN;
      break;
    case (GXINT_PTR)GXHBMMENU_MBAR_MINIMIZE_D:
      flags = GXDFCS_CAPTIONMIN | GXDFCS_INACTIVE;
      break;
    case (GXINT_PTR)GXHBMMENU_MBAR_CLOSE:
      flags = GXDFCS_CAPTIONCLOSE;
      break;
    case (GXINT_PTR)GXHBMMENU_MBAR_CLOSE_D:
      flags = GXDFCS_CAPTIONCLOSE | GXDFCS_INACTIVE;
      break;
    case (GXINT_PTR)GXHBMMENU_CALLBACK:
      {
        GXDRAWITEMSTRUCT drawItem;
        drawItem.CtlType = GXODT_MENU;
        drawItem.CtlID = 0;
        drawItem.itemID = lpitem->wID;
        drawItem.itemAction = odaction;
        drawItem.itemState = (lpitem->fState & GXMF_CHECKED)?GXODS_CHECKED:0;
        drawItem.itemState |= (lpitem->fState & GXMF_DEFAULT)?GXODS_DEFAULT:0;
        drawItem.itemState |= (lpitem->fState & GXMF_DISABLED)?GXODS_DISABLED:0;
        drawItem.itemState |= (lpitem->fState & GXMF_GRAYED)?GXODS_GRAYED|GXODS_DISABLED:0;
        drawItem.itemState |= (lpitem->fState & GXMF_HILITE)?GXODS_SELECTED:0;
        drawItem.hwndItem = (GXHWND)hmenu;
        drawItem.hDC = hdc;
        drawItem.itemData = lpitem->dwItemData;
        drawItem.rcItem = *rect;
        gxSendMessageW( hWndOwner, GXWM_DRAWITEM, 0, (GXLPARAM)&drawItem);
        return;
      }
      break;
    case (GXINT_PTR)GXHBMMENU_POPUP_CLOSE:
    case (GXINT_PTR)GXHBMMENU_POPUP_RESTORE:
    case (GXINT_PTR)GXHBMMENU_POPUP_MAXIMIZE:
    case (GXINT_PTR)GXHBMMENU_POPUP_MINIMIZE:
    default:
      FIXME("Magic %p not implemented\n", hbmToDraw);
      return;
    }
    r = *rect;
    gxInflateRect( &r, -1, -1 );
    if (lpitem->fState & GXMF_HILITE) flags |= GXDFCS_PUSHED;
    gxDrawFrameControl( hdc, &r, GXDFC_CAPTION, flags );
    return;
  }

  if (!bmp || !gxGetObjectW( bmp, sizeof(bm), &bm )) return;

got_bitmap:
  hdcMem = gxCreateCompatibleDC( hdc );
  gxSelectObject( hdcMem, bmp );

  /* handle fontsize > bitmap_height */
  top = (h>bm.bmHeight) ? rect->top+(h-bm.bmHeight)/2 : rect->top;
  left=rect->left;
  rop=((lpitem->fState & GXMF_HILITE) && !IS_MAGIC_BITMAP(hbmToDraw)) ? GXNOTSRCCOPY : GXSRCCOPY;
  if ((lpitem->fState & GXMF_HILITE) && lpitem->hbmpItem)
    gxSetBkColor(hdc, gxGetSysColor(GXCOLOR_HIGHLIGHT));
  gxBitBlt( hdc, left, top, w, h, hdcMem, bmp_xoffset, 0, rop );
  gxDeleteDC( hdcMem );
}


/***********************************************************************
*           MENU_CalcItemSize
*
* Calculate the size of the menu item and store it in lpitem->rect.
*/
static void MENU_CalcItemSize( GXHDC hdc, MENUITEM *lpitem, GXHWND hWndOwner,
                GXINT orgX, GXINT orgY, GXBOOL menuBar, POPUPMENU* lppop )
{
  GXWCHAR *p;
  GXUINT check_bitmap_width = gxGetSystemMetrics( GXSM_CXMENUCHECK );
  GXUINT arrow_bitmap_width;
  GXBITMAP bm;
  GXINT itemheight;

  TRACE("dc=%p owner=%p (%d,%d)\n", hdc, hWndOwner, orgX, orgY);
  //debug_print_menuitem("MENU_CalcItemSize: menuitem:", lpitem,
  //(menuBar ? " (MenuBar)" : ""));

  gxGetObjectW( get_arrow_bitmap(), sizeof(bm), &bm );
  arrow_bitmap_width = bm.bmWidth;

  /* not done in Menu_Init: GetDialogBaseUnits() breaks there */
  if( !menucharsize.cx ) {
    menucharsize.cx = gxGdiGetCharDimensions( hdc, NULL, &menucharsize.cy );
    /* Win95/98/ME will use menucharsize.cy here. Testing is possible
    * but it is unlikely an application will depend on that */
    ODitemheight = GXHIWORD( gxGetDialogBaseUnits());
  }

  gxSetRect( &lpitem->rect, orgX, orgY, orgX, orgY );

  if (lpitem->fType & GXMF_OWNERDRAW)
  {
    GXMEASUREITEMSTRUCT mis;
    mis.CtlType    = GXODT_MENU;
    mis.CtlID      = 0;
    mis.itemID     = lpitem->wID;
    mis.itemData   = lpitem->dwItemData;
    mis.itemHeight = ODitemheight;
    mis.itemWidth  = 0;
    gxSendMessageW( hWndOwner, GXWM_MEASUREITEM, 0, (GXLPARAM)&mis );
    /* Tests reveal that Windows ( Win95 thru WinXP) adds twice the average
    * width of a menufont character to the width of an owner-drawn menu. 
    */
    lpitem->rect.right += mis.itemWidth + 2 * menucharsize.cx;
    if (menuBar) {
      /* under at least win95 you seem to be given a standard
      height for the menu and the height value is ignored */
      lpitem->rect.bottom += gxGetSystemMetrics(GXSM_CYMENUSIZE);
    } else
      lpitem->rect.bottom += mis.itemHeight;

    TRACE("id=%04lx size=%dx%d\n",
      lpitem->wID, lpitem->rect.right-lpitem->rect.left,
      lpitem->rect.bottom-lpitem->rect.top);
    return;
  }

  if (lpitem->fType & GXMF_SEPARATOR)
  {
    lpitem->rect.bottom += gxGetSystemMetrics( GXSM_CYMENUSIZE)/2;
    if( !menuBar)
      lpitem->rect.right += arrow_bitmap_width + menucharsize.cx;
    return;
  }

  itemheight = 0;
  lpitem->xTab = 0;

  if (!menuBar) {
    if (lpitem->hbmpItem) {
      GXSIZE size;

      MENU_GetBitmapItemSize(lpitem, &size, hWndOwner);
      /* Keep the size of the bitmap in callback mode to be able
      * to draw it correctly */
      lpitem->bmpsize = size;
      lppop->maxBmpSize.cx = max( lppop->maxBmpSize.cx, size.cx);
      lppop->maxBmpSize.cy = max( lppop->maxBmpSize.cy, size.cy);
      lpitem->rect.right += size.cx + 2;
      itemheight = size.cy + 2;
    }
    if( !(lppop->dwStyle & GXMNS_NOCHECK))
      lpitem->rect.right += check_bitmap_width; 
    lpitem->rect.right += 4 + menucharsize.cx;
    lpitem->xTab = lpitem->rect.right;
    lpitem->rect.right += arrow_bitmap_width;
  } else if (lpitem->hbmpItem) { /* menuBar */
    GXSIZE size;

    MENU_GetBitmapItemSize( lpitem, &size, hWndOwner );
    lpitem->bmpsize = size;
    lpitem->rect.right  += size.cx;
    if( lpitem->text) lpitem->rect.right  += 2;
    itemheight = size.cy;
  }

  /* it must be a text item - unless it's the system menu */
  if (!(lpitem->fType & GXMF_SYSMENU) && lpitem->text) {
    GXHFONT hfontOld = NULL;
    GXRECT rc = lpitem->rect;
    GXLONG txtheight, txtwidth;

    if ( lpitem->fState & GXMFS_DEFAULT ) {
      hfontOld = (GXHFONT)gxSelectObject( hdc, get_menu_font(TRUE) );
    }
    if (menuBar) {
      txtheight = gxDrawTextW( hdc, lpitem->text, -1, &rc,
        GXDT_SINGLELINE|GXDT_CALCRECT); 
      lpitem->rect.right  += rc.right - rc.left;
      itemheight = max( max( itemheight, txtheight),
        gxGetSystemMetrics( GXSM_CYMENU) - 1);
      lpitem->rect.right +=  2 * menucharsize.cx;
    } else {
      if ((p = strchrW( lpitem->text, '\t' )) != NULL) {
        GXRECT tmprc = rc;
        GXLONG tmpheight;
        int n = (int)( p - lpitem->text);
        /* Item contains a tab (only meaningful in popup menus) */
        /* get text size before the tab */
        txtheight = gxDrawTextW( hdc, lpitem->text, n, &rc,
          GXDT_SINGLELINE|GXDT_CALCRECT);
        txtwidth = rc.right - rc.left;
        p += 1; /* advance past the Tab */
        /* get text size after the tab */
        tmpheight = gxDrawTextW( hdc, p, -1, &tmprc,
          GXDT_SINGLELINE|GXDT_CALCRECT);
        lpitem->xTab += txtwidth;
        txtheight = max( txtheight, tmpheight);
        txtwidth += menucharsize.cx + /* space for the tab */
          tmprc.right - tmprc.left; /* space for the short cut */
      } else {
        txtheight = gxDrawTextW( hdc, lpitem->text, -1, &rc,
          GXDT_SINGLELINE|GXDT_CALCRECT);
        txtwidth = rc.right - rc.left;
        lpitem->xTab += txtwidth;
      }
      lpitem->rect.right  += 2 + txtwidth;
      itemheight = max( itemheight,
        max( txtheight + 2, menucharsize.cy + 4));
    }
    if (hfontOld) gxSelectObject (hdc, hfontOld);
  } else if( menuBar) {
    itemheight = max( itemheight, gxGetSystemMetrics(GXSM_CYMENU)-1);
  }
  lpitem->rect.bottom += itemheight;
  TRACE("%s\n", wine_dbgstr_rect( &lpitem->rect));
}


/***********************************************************************
*           MENU_GetMaxPopupHeight
*/
static GXUINT
MENU_GetMaxPopupHeight(const POPUPMENU *lppop)
{
  if (lppop->cyMax)
    return lppop->cyMax;
  return gxGetSystemMetrics(GXSM_CYSCREEN) - gxGetSystemMetrics(GXSM_CYBORDER);
}


/***********************************************************************
*           MENU_PopupMenuCalcSize
*
* Calculate the size of a popup menu.
*/
static void MENU_PopupMenuCalcSize( LPPOPUPMENU lppop )
{
  MENUITEM *lpitem;
  GXHDC hdc;
  int start, i;
  int orgX, orgY, maxX, maxTab, maxTabWidth, maxHeight;

  lppop->Width = lppop->Height = 0;
  if (lppop->nItems == 0) return;
  hdc = gxGetDC( 0 );

  gxSelectObject( hdc, get_menu_font(FALSE));

  start = 0;
  maxX = 2 + 1;

  lppop->maxBmpSize.cx = 0;
  lppop->maxBmpSize.cy = 0;

  while (start < lppop->nItems)
  {
    lpitem = &lppop->items[start];
    orgX = maxX;
    if( lpitem->fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK))
      orgX += MENU_COL_SPACE; 
    orgY = MENU_TOP_MARGIN;

    maxTab = maxTabWidth = 0;
    /* Parse items until column break or end of menu */
    for (i = start; i < lppop->nItems; i++, lpitem++)
    {
      if ((i != start) &&
        (lpitem->fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK))) break;

      MENU_CalcItemSize( hdc, lpitem, lppop->hWndOwner, orgX, orgY, FALSE, lppop );
      maxX = max( maxX, lpitem->rect.right );
      orgY = lpitem->rect.bottom;
      if (IS_STRING_ITEM(lpitem->fType) && lpitem->xTab)
      {
        maxTab = max( maxTab, lpitem->xTab );
        maxTabWidth = max(maxTabWidth,lpitem->rect.right-lpitem->xTab);
      }
    }

    /* Finish the column (set all items to the largest width found) */
    maxX = max( maxX, maxTab + maxTabWidth );
    for (lpitem = &lppop->items[start]; start < i; start++, lpitem++)
    {
      lpitem->rect.right = maxX;
      if (IS_STRING_ITEM(lpitem->fType) && lpitem->xTab)
        lpitem->xTab = maxTab;

    }
    lppop->Height = max( lppop->Height, orgY );
  }

  lppop->Width  = maxX;

  /* space for 3d border */
  lppop->Height += MENU_BOTTOM_MARGIN;
  lppop->Width += 2;

  /* Adjust popup height if it exceeds maximum */
  maxHeight = MENU_GetMaxPopupHeight(lppop);
  lppop->nTotalHeight = lppop->Height - MENU_TOP_MARGIN;
  if (lppop->Height >= maxHeight)
  {
    lppop->Height = maxHeight;
    lppop->bScrolling = TRUE;
  }
  else
  {
    lppop->bScrolling = FALSE;
  }

  gxReleaseDC( 0, hdc );
}


/***********************************************************************
*           MENU_MenuBarCalcSize
*
* FIXME: Word 6 implements its own MDI and its own 'close window' bitmap
* height is off by 1 pixel which causes lengthy window relocations when
* active document window is maximized/restored.
*
* Calculate the size of the menu bar.
*/
static void MENU_MenuBarCalcSize( GXHDC hdc, LPGXRECT lprect,
                 LPPOPUPMENU lppop, GXHWND hWndOwner )
{
  MENUITEM *lpitem;
  int start, i, orgX, orgY, maxY, helpPos;

  if ((lprect == NULL) || (lppop == NULL)) return;
  if (lppop->nItems == 0) return;
  TRACE("lprect %p %s\n", lprect, wine_dbgstr_rect( lprect));
  lppop->Width  = lprect->right - lprect->left;
  lppop->Height = 0;
  maxY = lprect->top+1;
  start = 0;
  helpPos = -1;
  lppop->maxBmpSize.cx = 0;
  lppop->maxBmpSize.cy = 0;
  while (start < lppop->nItems)
  {
    lpitem = &lppop->items[start];
    orgX = lprect->left;
    orgY = maxY;

    /* Parse items until line break or end of menu */
    for (i = start; i < lppop->nItems; i++, lpitem++)
    {
      if ((helpPos == -1) && (lpitem->fType & GXMF_RIGHTJUSTIFY)) helpPos = i;
      if ((i != start) &&
        (lpitem->fType & (GXMF_MENUBREAK | GXMF_MENUBARBREAK))) break;

      TRACE("calling MENU_CalcItemSize org=(%d, %d)\n", orgX, orgY );
      debug_print_menuitem ("  item: ", lpitem, "");
      MENU_CalcItemSize( hdc, lpitem, hWndOwner, orgX, orgY, TRUE, lppop );

      if (lpitem->rect.right > lprect->right)
      {
        if (i != start) break;
        else lpitem->rect.right = lprect->right;
      }
      maxY = max( maxY, lpitem->rect.bottom );
      orgX = lpitem->rect.right;
    }

    /* Finish the line (set all items to the largest height found) */
    while (start < i) lppop->items[start++].rect.bottom = maxY;
  }

  lprect->bottom = maxY;
  lppop->Height = lprect->bottom - lprect->top;

  /* Flush right all items between the MF_RIGHTJUSTIFY and */
  /* the last item (if several lines, only move the last line) */
  lpitem = &lppop->items[lppop->nItems-1];
  orgY = lpitem->rect.top;
  orgX = lprect->right;
  for (i = lppop->nItems - 1; i >= helpPos; i--, lpitem--) {
    if ( (helpPos==-1) || (helpPos>i) )
      break;        /* done */
    if (lpitem->rect.top != orgY) break;  /* Other line */
    if (lpitem->rect.right >= orgX) break;  /* Too far right already */
    lpitem->rect.left += orgX - lpitem->rect.right;
    lpitem->rect.right = orgX;
    orgX = lpitem->rect.left;
  }
}


/***********************************************************************
*           MENU_DrawScrollArrows
*
* Draw scroll arrows.
*/
static void
MENU_DrawScrollArrows(const POPUPMENU *lppop, GXHDC hdc)
{
  GXHDC hdcMem = gxCreateCompatibleDC(hdc);
  GXHBITMAP hOrigBitmap;
  GXUINT arrow_bitmap_width, arrow_bitmap_height;
  GXBITMAP bmp;
  GXRECT rect;

  gxGetObjectW(get_down_arrow_bitmap(), sizeof(bmp), &bmp);
  arrow_bitmap_width = bmp.bmWidth;
  arrow_bitmap_height = bmp.bmHeight;


  if (lppop->nScrollPos)
    hOrigBitmap = (GXHBITMAP)gxSelectObject(hdcMem, get_up_arrow_bitmap());
  else
    hOrigBitmap = (GXHBITMAP)gxSelectObject(hdcMem, get_up_arrow_inactive_bitmap());
  rect.left = 0;
  rect.top = 0;
  rect.right = lppop->Width;
  rect.bottom = arrow_bitmap_height;
  gxFillRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_MENU));
  gxBitBlt(hdc, (lppop->Width - arrow_bitmap_width) / 2, 0,
    arrow_bitmap_width, arrow_bitmap_height, hdcMem, 0, 0, GXSRCCOPY);
  rect.top = lppop->Height - arrow_bitmap_height;
  rect.bottom = lppop->Height;
  gxFillRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_MENU));
  if (lppop->nScrollPos < lppop->nTotalHeight - (MENU_GetMaxPopupHeight(lppop) - 2 * arrow_bitmap_height))
    gxSelectObject(hdcMem, get_down_arrow_bitmap());
  else
    gxSelectObject(hdcMem, get_down_arrow_inactive_bitmap());
  gxBitBlt(hdc, (lppop->Width - arrow_bitmap_width) / 2,
    lppop->Height - arrow_bitmap_height,
    arrow_bitmap_width, arrow_bitmap_height, hdcMem, 0, 0, GXSRCCOPY);
  gxSelectObject(hdcMem, hOrigBitmap);
  gxDeleteDC(hdcMem);
}


/***********************************************************************
*           draw_popup_arrow
*
* Draws the popup-menu arrow.
*/
static void draw_popup_arrow( GXHDC hdc, GXRECT rect, GXUINT arrow_bitmap_width,
               GXUINT arrow_bitmap_height)
{
  GXHDC hdcMem = gxCreateCompatibleDC( hdc );
  GXHBITMAP hOrigBitmap;

  hOrigBitmap = (GXHBITMAP)gxSelectObject( hdcMem, get_arrow_bitmap() );
  gxBitBlt( hdc, rect.right - arrow_bitmap_width - 1,
    (rect.top + rect.bottom - arrow_bitmap_height) / 2,
    arrow_bitmap_width, arrow_bitmap_height,
    hdcMem, 0, 0, GXSRCCOPY );
  gxSelectObject( hdcMem, hOrigBitmap );
  gxDeleteDC( hdcMem );
}
/***********************************************************************
*           MENU_DrawMenuItem
*
* Draw a single menu item.
*/
static void MENU_DrawMenuItem( GXHWND hWnd, GXHMENU hmenu, GXHWND hWndOwner, GXHDC hdc, MENUITEM *lpitem,
                GXUINT height, GXBOOL menuBar, GXUINT odaction )
{
  GXRECT rect;
  GXBOOL flat_menu = FALSE;
  int bkgnd;
  GXUINT arrow_bitmap_width = 0, arrow_bitmap_height = 0;
  POPUPMENU *menu = MENU_GetMenu(hmenu);
  GXRECT bmprc;

  //debug_print_menuitem("MENU_DrawMenuItem: ", lpitem, "");

  if (!menuBar) {
    GXBITMAP bmp;
    gxGetObjectW( get_arrow_bitmap(), sizeof(bmp), &bmp );
    arrow_bitmap_width = bmp.bmWidth;
    arrow_bitmap_height = bmp.bmHeight;
  }

  if (lpitem->fType & GXMF_SYSMENU)
  {
    if( !gxIsIconic(hWnd) )
      NC_DrawSysButton( hWnd, hdc, lpitem->fState & (GXMF_HILITE | GXMF_MOUSESELECT) );
    return;
  }

  gxSystemParametersInfoW (SPI_GETFLATMENU, 0, &flat_menu, 0);
  bkgnd = (menuBar && flat_menu) ? GXCOLOR_MENUBAR : GXCOLOR_MENU;

  /* Setup colors */

  if (lpitem->fState & GXMF_HILITE)
  {
    if(menuBar && !flat_menu) {
      gxSetTextColor(hdc, gxGetSysColor(GXCOLOR_MENUTEXT));
      gxSetBkColor(hdc, gxGetSysColor(GXCOLOR_MENU));
    } else {
      if(lpitem->fState & GXMF_GRAYED)
        gxSetTextColor(hdc, gxGetSysColor(GXCOLOR_GRAYTEXT));
      else
        gxSetTextColor(hdc, gxGetSysColor(GXCOLOR_HIGHLIGHTTEXT));
      gxSetBkColor(hdc, gxGetSysColor(GXCOLOR_HIGHLIGHT));
    }
  }
  else
  {
    if (lpitem->fState & GXMF_GRAYED)
      gxSetTextColor( hdc, gxGetSysColor( GXCOLOR_GRAYTEXT ) );
    else
      gxSetTextColor( hdc, gxGetSysColor( GXCOLOR_MENUTEXT ) );
    gxSetBkColor( hdc, gxGetSysColor( bkgnd ) );
  }

  TRACE("rect=%s\n", wine_dbgstr_rect( &lpitem->rect));
  rect = lpitem->rect;
  MENU_AdjustMenuItemRect(MENU_GetMenu(hmenu), &rect);

  if (lpitem->fType & GXMF_OWNERDRAW)
  {
    /*
    ** Experimentation under Windows reveals that an owner-drawn
    ** menu is given the gxRectangle which includes the space it requested
    ** in its response to GXWM_MEASUREITEM _plus_ width for a checkmark
    ** and a popup-menu arrow.  This is the value of lpitem->rect.
    ** Windows will leave all drawing to the application except for
    ** the popup-menu arrow.  Windows always draws that itself, after
    ** the menu owner has finished drawing.
    */
    GXDRAWITEMSTRUCT dis;

    dis.CtlType   = GXODT_MENU;
    dis.CtlID     = 0;
    dis.itemID    = lpitem->wID;
    dis.itemData  = lpitem->dwItemData;
    dis.itemState = 0;
    if (lpitem->fState & GXMF_CHECKED) dis.itemState |= GXODS_CHECKED;
    if (lpitem->fState & GXMF_GRAYED)  dis.itemState |= GXODS_GRAYED|GXODS_DISABLED;
    if (lpitem->fState & GXMF_HILITE)  dis.itemState |= GXODS_SELECTED;
    dis.itemAction = odaction; /* GXODA_DRAWENTIRE | GXODA_SELECT | GXODA_FOCUS; */
    dis.hwndItem   = (GXHWND)hmenu;
    dis.hDC        = hdc;
    dis.rcItem     = rect;
    TRACE("Ownerdraw: owner=%p itemID=%d, itemState=%d, itemAction=%d, "
      "GXHWNDItem=%p, hdc=%p, rcItem=%s\n", hWndOwner,
      dis.itemID, dis.itemState, dis.itemAction, dis.hwndItem,
      dis.hDC, wine_dbgstr_rect( &dis.rcItem));
    gxSendMessageW( hWndOwner, GXWM_DRAWITEM, 0, (GXLPARAM)&dis );
    /* Draw the popup-menu arrow */
    if (lpitem->fType & GXMF_POPUP)
      draw_popup_arrow( hdc, rect, arrow_bitmap_width,
      arrow_bitmap_height);
    return;
  }

  if (menuBar && (lpitem->fType & GXMF_SEPARATOR)) return;

  if (lpitem->fState & GXMF_HILITE)
  {
    if (flat_menu)
    {
      gxInflateRect (&rect, -1, -1);
      gxFillRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_MENUHILIGHT));
      gxInflateRect (&rect, 1, 1);
      gxFrameRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_HIGHLIGHT));
    }
    else
    {
      if(menuBar)
        gxDrawEdge(hdc, &rect, GXBDR_SUNKENOUTER, GXBF_RECT);
      else
        gxFillRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_HIGHLIGHT));
    }
  }
  else
    gxFillRect( hdc, &rect, gxGetSysColorBrush(bkgnd) );

  gxSetBkMode( hdc, GXTRANSPARENT );

  /* vertical separator */
  if (!menuBar && (lpitem->fType & GXMF_MENUBARBREAK))
  {
    GXHPEN oldPen;
    GXRECT rc = rect;

    rc.left -= MENU_COL_SPACE / 2 + 1;
    rc.top = 3;
    rc.bottom = height - 3;
    if (flat_menu)
    {
      oldPen = (GXHPEN)gxSelectObject( hdc, SYSCOLOR_GetPen(GXCOLOR_BTNSHADOW) );
      gxMoveToEx( hdc, rc.left, rc.top, NULL );
      gxLineTo( hdc, rc.left, rc.bottom );
      gxSelectObject( hdc, oldPen );
    }
    else
      gxDrawEdge (hdc, &rc, GXEDGE_ETCHED, GXBF_LEFT);
  }

  /* horizontal separator */
  if (lpitem->fType & GXMF_SEPARATOR)
  {
    GXHPEN oldPen;
    GXRECT rc = rect;

    rc.left++;
    rc.right--;
    rc.top = ( rc.top + rc.bottom) / 2;
    if (flat_menu)
    {
      oldPen = (GXHPEN)gxSelectObject( hdc, SYSCOLOR_GetPen(GXCOLOR_BTNSHADOW) );
      gxMoveToEx( hdc, rc.left, rc.top, NULL );
      gxLineTo( hdc, rc.right, rc.top );
      gxSelectObject( hdc, oldPen );
    }
    else
      gxDrawEdge (hdc, &rc, GXEDGE_ETCHED, GXBF_TOP);
    return;
  }

  /* helper lines for debugging */
  /*  gxFrameRect(hdc, &rect, GetStockObject(BLACK_BRUSH));
  gxSelectObject( hdc, SYSCOLOR_GetPen(GXCOLOR_WINDOWFRAME) );
  gxMoveToEx( hdc, rect.left, (rect.top + rect.bottom)/2, NULL );
  gxLineTo( hdc, rect.right, (rect.top + rect.bottom)/2 );
  */

  if (lpitem->hbmpItem) {
    /* calculate the bitmap gxRectangle in coordinates relative
    * to the item gxRectangle */
    if( menuBar) {
      if( lpitem->hbmpItem == GXHBMMENU_CALLBACK)
        bmprc.left = 3;
      else 
        bmprc.left = lpitem->text ? menucharsize.cx : 0;          
    } else {
      bmprc.left = 4;
      if( !(menu->dwStyle & ( GXMNS_CHECKORBMP | GXMNS_NOCHECK)))
        bmprc.left += gxGetSystemMetrics( GXSM_CXMENUCHECK); 
    }
    bmprc.right =  bmprc.left + lpitem->bmpsize.cx;
    if( menuBar && !(lpitem->hbmpItem == GXHBMMENU_CALLBACK))
      bmprc.top = 0;
    else
      bmprc.top = (rect.bottom - rect.top -
      lpitem->bmpsize.cy) / 2; 
    bmprc.bottom =  bmprc.top + lpitem->bmpsize.cy;
  }

  if (!menuBar)
  {
    GXHBITMAP bm;
    GXINT y = rect.top + rect.bottom;
    GXRECT rc = rect;
    int checked = FALSE;
    GXUINT check_bitmap_width = gxGetSystemMetrics( GXSM_CXMENUCHECK );
    GXUINT check_bitmap_height = gxGetSystemMetrics( GXSM_CYMENUCHECK );
    /* Draw the check mark
    *
    * FIXME:
    * Custom checkmark bitmaps are monochrome but not always 1bpp.
    */
    if( !(menu->dwStyle & GXMNS_NOCHECK)) {
      bm = (lpitem->fState & GXMF_CHECKED) ? lpitem->hCheckBit :
        lpitem->hUnCheckBit;
    if (bm)  /* we have a custom bitmap */
    {
      GXHDC hdcMem = gxCreateCompatibleDC( hdc );

      gxSelectObject( hdcMem, bm );
      gxBitBlt( hdc, rc.left, (y - check_bitmap_height) / 2,
        check_bitmap_width, check_bitmap_height,
        hdcMem, 0, 0, GXSRCCOPY );
      gxDeleteDC( hdcMem );
      checked = TRUE;
    }
    else if (lpitem->fState & GXMF_CHECKED) /* standard bitmaps */
    {
      GXRECT r;
      GXHBITMAP bm = gxCreateBitmap( check_bitmap_width,
        check_bitmap_height, 1, 1, NULL );
      GXHDC hdcMem = gxCreateCompatibleDC( hdc );

      gxSelectObject( hdcMem, bm );
      gxSetRect( &r, 0, 0, check_bitmap_width, check_bitmap_height);
      gxDrawFrameControl( hdcMem, &r, GXDFC_MENU,
        (lpitem->fType & GXMFT_RADIOCHECK) ?
        GXDFCS_MENUBULLET : GXDFCS_MENUCHECK );
      gxBitBlt( hdc, rc.left, (y - r.bottom) / 2, r.right, r.bottom,
        hdcMem, 0, 0, GXSRCCOPY );
      gxDeleteDC( hdcMem );
      gxDeleteObject( bm );
      checked = TRUE;
    }
    }
    if( lpitem->hbmpItem &&
      !( checked && (menu->dwStyle & GXMNS_CHECKORBMP))) {
        GXPOINT origorg;
        /* some applications make this assumption on the DC's origin */
        gxSetViewportOrgEx( hdc, rect.left, rect.top, &origorg);
        MENU_DrawBitmapItem(hdc, lpitem, &bmprc, hmenu, hWndOwner,
          odaction, FALSE);
        gxSetViewportOrgEx( hdc, origorg.x, origorg.y, NULL);
    }
    /* Draw the popup-menu arrow */
    if (lpitem->fType & GXMF_POPUP)
      draw_popup_arrow( hdc, rect, arrow_bitmap_width,
      arrow_bitmap_height);
    rect.left += 4;
    if( !(menu->dwStyle & GXMNS_NOCHECK))
      rect.left += check_bitmap_width;
    rect.right -= arrow_bitmap_width;
  }
  else if( lpitem->hbmpItem)
  {   /* Draw the bitmap */
    GXPOINT origorg;

    gxSetViewportOrgEx( hdc, rect.left, rect.top, &origorg);
    MENU_DrawBitmapItem( hdc, lpitem, &bmprc, hmenu, hWndOwner,
      odaction, menuBar);
    gxSetViewportOrgEx( hdc, origorg.x, origorg.y, NULL);
  }
  /* process text if present */
  if (lpitem->text)
  {
    register int i;
    GXHFONT hfontOld = 0;

    GXUINT uFormat = (menuBar) ?
      GXDT_CENTER | GXDT_VCENTER | GXDT_SINGLELINE :
    GXDT_LEFT | GXDT_VCENTER | GXDT_SINGLELINE;

    if( !(menu->dwStyle & GXMNS_CHECKORBMP))
      rect.left += menu->maxBmpSize.cx;

    if ( lpitem->fState & GXMFS_DEFAULT )
    {
      hfontOld = (GXHFONT)gxSelectObject( hdc, get_menu_font(TRUE) );
    }

    if (menuBar) {
      if( lpitem->hbmpItem)
        rect.left += lpitem->bmpsize.cx;
      if( !(lpitem->hbmpItem == GXHBMMENU_CALLBACK))
        rect.left += menucharsize.cx;
      rect.right -= menucharsize.cx;
    }

    for (i = 0; lpitem->text[i]; i++)
      if ((lpitem->text[i] == '\t') || (lpitem->text[i] == '\b'))
        break;

    if(lpitem->fState & GXMF_GRAYED)
    {
      if (!(lpitem->fState & GXMF_HILITE) )
      {
        ++rect.left; ++rect.top; ++rect.right; ++rect.bottom;
        gxSetTextColor(hdc, gxGetSysColor(GXCOLOR_GRAYTEXT));
        gxDrawTextW( hdc, lpitem->text, i, &rect, uFormat );
        --rect.left; --rect.top; --rect.right; --rect.bottom;
      }
      gxSetTextColor(hdc, GXRGB(0x80, 0x80, 0x80));
    }

    gxDrawTextW( hdc, lpitem->text, i, &rect, uFormat);

    /* paint the shortcut text */
    if (!menuBar && lpitem->text[i])  /* There's a tab or flush-right char */
    {
      if (lpitem->text[i] == '\t')
      {
        rect.left = lpitem->xTab;
        uFormat = GXDT_LEFT | GXDT_VCENTER | GXDT_SINGLELINE;
      }
      else
      {
        rect.right = lpitem->xTab;
        uFormat = GXDT_RIGHT | GXDT_VCENTER | GXDT_SINGLELINE;
      }

      if(lpitem->fState & GXMF_GRAYED)
      {
        if (!(lpitem->fState & GXMF_HILITE) )
        {
          ++rect.left; ++rect.top; ++rect.right; ++rect.bottom;
          gxSetTextColor(hdc, GXRGB(0xff, 0xff, 0xff));
          gxDrawTextW( hdc, lpitem->text + i + 1, -1, &rect, uFormat );
          --rect.left; --rect.top; --rect.right; --rect.bottom;
        }
        gxSetTextColor(hdc, GXRGB(0x80, 0x80, 0x80));
      }
      gxDrawTextW( hdc, lpitem->text + i + 1, -1, &rect, uFormat );
    }

    if (hfontOld)
      gxSelectObject (hdc, hfontOld);
  }
}


/***********************************************************************
*           MENU_DrawPopupMenu
*
* Paint a popup menu.
*/
static void MENU_DrawPopupMenu( GXHWND hWnd, GXHDC hdc, GXHMENU hmenu )
{
  GXHBRUSH hPrevBrush = 0;
  GXRECT rect;

  TRACE("GXWND=%p dc=%p menu=%p\n", hWnd, hdc, hmenu);

  gxGetClientRect( hWnd, &rect );

  if((hPrevBrush = (GXHBRUSH)gxSelectObject( hdc, gxGetSysColorBrush(GXCOLOR_MENU) ))
    && (gxSelectObject( hdc, get_menu_font(FALSE))))
  {
    GXHPEN hPrevPen;

    gxRectangle( hdc, rect.left, rect.top, rect.right, rect.bottom );

    hPrevPen = (GXHPEN)gxSelectObject( hdc, gxGetStockObject( NULL_PEN ) );
    if( hPrevPen )
    {
      POPUPMENU *menu;
      GXBOOL flat_menu = FALSE;

      gxSystemParametersInfoW (SPI_GETFLATMENU, 0, &flat_menu, 0);
      if (flat_menu)
        gxFrameRect(hdc, &rect, gxGetSysColorBrush(GXCOLOR_BTNSHADOW));
      else
        gxDrawEdge (hdc, &rect, GXEDGE_RAISED, GXBF_RECT);

      if( (menu = MENU_GetMenu( hmenu )))
      {
        /* draw menu items */
        if( menu->nItems)
        {
          MENUITEM *item;
          GXUINT u;

          item = menu->items;
          for( u = menu->nItems; u > 0; u--, item++)
            MENU_DrawMenuItem( hWnd, hmenu, menu->hWndOwner, hdc,
            item, menu->Height, FALSE, GXODA_DRAWENTIRE );
        }
        /* draw scroll arrows */
        if (menu->bScrolling)
          MENU_DrawScrollArrows(menu, hdc);
      }
    } else
    {
      gxSelectObject( hdc, hPrevBrush );
    }
  }
}

/***********************************************************************
*           MENU_DrawMenuBar
*
* Paint a menu bar. Returns the height of the menu bar.
* called from [windows/nonclient.c]
*/
GXUINT MENU_DrawMenuBar( GXHDC hDC, LPGXRECT lprect, GXHWND hWnd,
            GXBOOL suppress_draw)
{
  LPPOPUPMENU lppop;
  GXHFONT hfontOld = 0;
  GXHMENU hMenu = gxGetMenu(hWnd);

  lppop = MENU_GetMenu( hMenu );
  if (lppop == NULL || lprect == NULL)
  {
    return gxGetSystemMetrics(GXSM_CYMENU);
  }

  if (suppress_draw)
  {
    hfontOld = (GXHFONT)gxSelectObject( hDC, get_menu_font(FALSE));

    if (lppop->Height == 0)
      MENU_MenuBarCalcSize(hDC, lprect, lppop, hWnd);

    lprect->bottom = lprect->top + lppop->Height;

    if (hfontOld) gxSelectObject( hDC, hfontOld);
    return lppop->Height;
  }
  else
    return DrawMenuBarTemp(hWnd, hDC, lprect, hMenu, NULL);
}


/***********************************************************************
*           MENU_ShowPopup
*
* Display a popup menu.
*/
static GXBOOL MENU_ShowPopup( GXHWND hWndOwner, GXHMENU hmenu, GXUINT id,
               GXINT x, GXINT y, GXINT xanchor, GXINT yanchor )
{
  POPUPMENU *menu;
  GXINT width, height;
  GXPOINT pt;
  GXHMONITOR monitor;
  GXMONITORINFO info;

  TRACE("owner=%p hmenu=%p id=0x%04x x=0x%04x y=0x%04x xa=0x%04x ya=0x%04x\n",
    hWndOwner, hmenu, id, x, y, xanchor, yanchor);

  if (!(menu = MENU_GetMenu( hmenu ))) return FALSE;
  if (menu->FocusedItem != NO_SELECTED_ITEM)
  {
    menu->items[menu->FocusedItem].fState &= ~(GXMF_HILITE|GXMF_MOUSESELECT);
    menu->FocusedItem = NO_SELECTED_ITEM;
  }

  /* store the owner for DrawItem */
  menu->hWndOwner = hWndOwner;

  menu->nScrollPos = 0;
  MENU_PopupMenuCalcSize( menu );

  /* adjust popup menu pos so that it fits within the desktop */

  width = menu->Width + gxGetSystemMetrics(GXSM_CXBORDER);
  height = menu->Height + gxGetSystemMetrics(GXSM_CYBORDER);

  /* FIXME: should use item rect */
  pt.x = x;
  pt.y = y;
  monitor = gxMonitorFromPoint( pt, GXMONITOR_DEFAULTTONEAREST );
  info.cbSize = sizeof(info);
  gxGetMonitorInfoW( monitor, &info );
  if( x + width > info.rcWork.right)
  {
    if( xanchor && x >= width - xanchor )
      x -= width - xanchor;

    if( x + width > info.rcWork.right)
      x = info.rcWork.right - width;
  }
  if( x < info.rcWork.left ) x = info.rcWork.left;

  if( y + height > info.rcWork.bottom)
  {
    if( yanchor && y >= height + yanchor )
      y -= height + yanchor;

    if( y + height > info.rcWork.bottom)
      y = info.rcWork.bottom - height;
  }
  if( y < info.rcWork.top ) y = info.rcWork.top;

  /* NOTE: In Windows, top menu popup is not owned. */
  menu->hWnd = gxCreateWindowExW( 0, GXPOPUPMENU_CLASS_ATOM, NULL,
    GXWS_POPUP, x, y, width, height,
    NULL/*hWndOwner*/, 0, (GXHINSTANCE)gxGetWindowLongPtrW(hWndOwner, GXGWLP_HINSTANCE),
    (GXLPVOID)hmenu );
  if( !menu->hWnd ) return FALSE;
  if (!top_popup) top_popup = menu->hWnd;

  /* Display the window */

  gxSetWindowPos( menu->hWnd, GXHWND_TOP, 0, 0, 0, 0,
    GXSWP_SHOWWINDOW | GXSWP_NOSIZE | GXSWP_NOMOVE | GXSWP_NOACTIVATE );
  gxUpdateWindow( menu->hWnd );
  return TRUE;
}


/***********************************************************************
*           MENU_EnsureMenuItemVisible
*/
static void
MENU_EnsureMenuItemVisible(LPPOPUPMENU lppop, GXUINT wIndex, GXHDC hdc)
{
  if (lppop->bScrolling)
  {
    MENUITEM *item = &lppop->items[wIndex];
    GXUINT nMaxHeight = MENU_GetMaxPopupHeight(lppop);
    GXUINT nOldPos = lppop->nScrollPos;
    GXRECT rc;
    GXUINT arrow_bitmap_height;
    GXBITMAP bmp;

    gxGetClientRect(lppop->hWnd, &rc);

    gxGetObjectW(get_down_arrow_bitmap(), sizeof(bmp), &bmp);
    arrow_bitmap_height = bmp.bmHeight;

    rc.top += arrow_bitmap_height;
    rc.bottom -= arrow_bitmap_height + MENU_BOTTOM_MARGIN;

    nMaxHeight -= gxGetSystemMetrics(GXSM_CYBORDER) + 2 * arrow_bitmap_height;
    if (item->rect.bottom > lppop->nScrollPos + nMaxHeight)
    {

      lppop->nScrollPos = item->rect.bottom - nMaxHeight;
      gxScrollWindow(lppop->hWnd, 0, nOldPos - lppop->nScrollPos, &rc, &rc);
      MENU_DrawScrollArrows(lppop, hdc);
    }
    else if (item->rect.top - MENU_TOP_MARGIN < lppop->nScrollPos)
    {
      lppop->nScrollPos = item->rect.top - MENU_TOP_MARGIN;
      gxScrollWindow(lppop->hWnd, 0, nOldPos - lppop->nScrollPos, &rc, &rc);
      MENU_DrawScrollArrows(lppop, hdc);
    }
  }
}


/***********************************************************************
*           MENU_SelectItem
*/
static void MENU_SelectItem( GXHWND hWndOwner, GXHMENU hmenu, GXUINT wIndex,
              GXBOOL sendMenuSelect, GXHMENU topmenu )
{
  LPPOPUPMENU lppop;
  GXHDC hdc;

  TRACE("owner=%p menu=%p index=0x%04x select=0x%04x\n", hWndOwner, hmenu, wIndex, sendMenuSelect);

  lppop = MENU_GetMenu( hmenu );
  if ((!lppop) || (!lppop->nItems) || (!lppop->hWnd)) return;

  if (lppop->FocusedItem == wIndex) return;
  if (lppop->wFlags & GXMF_POPUP) hdc = gxGetDC( lppop->hWnd );
  else hdc = gxGetDCEx( lppop->hWnd, 0, GXDCX_CACHE | GXDCX_WINDOW);
  if (!top_popup) top_popup = lppop->hWnd;

  gxSelectObject( hdc, get_menu_font(FALSE));

  /* Clear previous highlighted item */
  if (lppop->FocusedItem != NO_SELECTED_ITEM)
  {
    lppop->items[lppop->FocusedItem].fState &= ~(GXMF_HILITE|GXMF_MOUSESELECT);
    MENU_DrawMenuItem(lppop->hWnd, hmenu, hWndOwner, hdc,&lppop->items[lppop->FocusedItem],
      lppop->Height, !(lppop->wFlags & GXMF_POPUP),
      GXODA_SELECT );
  }

  /* Highlight new item (if any) */
  lppop->FocusedItem = wIndex;
  if (lppop->FocusedItem != NO_SELECTED_ITEM)
  {
    if(!(lppop->items[wIndex].fType & GXMF_SEPARATOR)) {
      lppop->items[wIndex].fState |= GXMF_HILITE;
      MENU_EnsureMenuItemVisible(lppop, wIndex, hdc);
      MENU_DrawMenuItem( lppop->hWnd, hmenu, hWndOwner, hdc,
        &lppop->items[wIndex], lppop->Height,
        !(lppop->wFlags & GXMF_POPUP), GXODA_SELECT );
    }
    if (sendMenuSelect)
    {
      MENUITEM *ip = &lppop->items[lppop->FocusedItem];
      gxSendMessageW( hWndOwner, GXWM_MENUSELECT,
        GXMAKELONG(ip->fType & GXMF_POPUP ? wIndex: ip->wID,
        ip->fType | ip->fState |
        (lppop->wFlags & GXMF_SYSMENU)), (GXLPARAM)hmenu);
    }
  }
  else if (sendMenuSelect) {
    if(topmenu){
      int pos;
      if((pos=MENU_FindSubMenu(&topmenu, hmenu))!=NO_SELECTED_ITEM){
        POPUPMENU *ptm = MENU_GetMenu( topmenu );
        MENUITEM *ip = &ptm->items[pos];
        gxSendMessageW( hWndOwner, GXWM_MENUSELECT, GXMAKELONG(pos,
          ip->fType | ip->fState |
          (ptm->wFlags & GXMF_SYSMENU)), (GXLPARAM)topmenu);
      }
    }
  }
  gxReleaseDC( lppop->hWnd, hdc );
}


/***********************************************************************
*           MENU_MoveSelection
*
* Moves currently selected item according to the offset parameter.
* If there is no selection then it should select the last item if
* offset is ITEM_PREV or the first item if offset is ITEM_NEXT.
*/
static void MENU_MoveSelection( GXHWND hWndOwner, GXHMENU hmenu, GXINT offset )
{
  GXINT i;
  POPUPMENU *menu;

  TRACE("hWnd=%p hmenu=%p off=0x%04x\n", hWndOwner, hmenu, offset);

  menu = MENU_GetMenu( hmenu );
  if ((!menu) || (!menu->items)) return;

  if ( menu->FocusedItem != NO_SELECTED_ITEM )
  {
    if( menu->nItems == 1 ) return; else
      for (i = menu->FocusedItem + offset ; i >= 0 && i < menu->nItems
        ; i += offset)
        if (!(menu->items[i].fType & GXMF_SEPARATOR))
        {
          MENU_SelectItem( hWndOwner, hmenu, i, TRUE, 0 );
          return;
        }
  }

  for ( i = (offset > 0) ? 0 : menu->nItems - 1;
    i >= 0 && i < menu->nItems ; i += offset)
    if (!(menu->items[i].fType & GXMF_SEPARATOR))
    {
      MENU_SelectItem( hWndOwner, hmenu, i, TRUE, 0 );
      return;
    }
}


/**********************************************************************
*         MENU_SetItemData
*
* Set an item's flags, id and text ptr. Called by InsertMenu() and
* ModifyMenu().
*/
static GXBOOL MENU_SetItemData( MENUITEM *item, GXUINT flags, GXUINT_PTR id,
                 GXLPCWSTR str )
{
  //debug_print_menuitem("MENU_SetItemData from: ", item, "");
  TRACE("flags=%x str=%p\n", flags, str);

  if (IS_STRING_ITEM(flags))
  {
    GXLPWSTR prevText = item->text;
    if (!str)
    {
      flags |= GXMF_SEPARATOR;
      item->text = NULL;
    }
    else
    {
      GXLPWSTR text;
      /* Item beginning with a backspace is a help item */
      if (*str == '\b')
      {
        flags |= GXMF_HELP;
        str++;
      }
      if (!(text = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (GXSTRLEN(str)+1) * sizeof(GXWCHAR) )))
        return FALSE;
      GXSTRCPY( text, str );
      item->text = text;
    }
    item->hbmpItem = NULL;
    gxHeapFree( gxGetProcessHeap(), 0, prevText );
  }
  else if(( flags & GXMFT_BITMAP)) {
    item->hbmpItem = HBITMAP_32(GXLOWORD(str));
    /* setting bitmap clears text */
    gxHeapFree( gxGetProcessHeap(), 0, item->text );
    item->text = NULL;
  }

  if (flags & GXMF_SEPARATOR) flags |= GXMF_GRAYED | GXMF_DISABLED;

  if (flags & GXMF_OWNERDRAW)
    item->dwItemData = (GXDWORD_PTR)str;
  else
    item->dwItemData = 0;

  if ((item->fType & GXMF_POPUP) && (flags & GXMF_POPUP) && (item->hSubMenu != (GXHMENU)id) )
    gxDestroyMenu( item->hSubMenu );   /* ModifyMenu() spec */

  if (flags & GXMF_POPUP)
  {
    POPUPMENU *menu = MENU_GetMenu((GXHMENU)id);
    if (menu) menu->wFlags |= GXMF_POPUP;
    else
    {
      item->wID = 0;
      item->hSubMenu = 0;
      item->fType = 0;
      item->fState = 0;
      return FALSE;
    }
  }

  item->wID = id;
  if (flags & GXMF_POPUP) item->hSubMenu = (GXHMENU)id;

  if ((item->fType & GXMF_POPUP) && !(flags & GXMF_POPUP) )
    flags |= GXMF_POPUP; /* keep popup */

  item->fType = flags & TYPE_MASK;
  /* MFS_DEFAULT is not accepted. MF_HILITE is not listed as a valid flag
  for ModifyMenu, but Windows accepts it */
  item->fState = flags & MENUITEMINFO_STATE_MASK & ~GXMFS_DEFAULT;

  /* Don't call SetRectEmpty here! */

  debug_print_menuitem("MENU_SetItemData to  : ", item, "");
  return TRUE;
}


/**********************************************************************
*         MENU_InsertItem
*
* Insert (allocate) a new item into a menu.
*/
static MENUITEM *MENU_InsertItem( GXHMENU hMenu, GXUINT pos, GXUINT flags )
{
  MENUITEM *newItems;
  POPUPMENU *menu;

  if (!(menu = MENU_GetMenu(hMenu)))
    return NULL;

  /* Find where to insert new item */

  if (flags & GXMF_BYPOSITION) {
    if (pos > menu->nItems)
      pos = menu->nItems;
  } else {
    if (!MENU_FindItem( &hMenu, &pos, flags ))
      pos = menu->nItems;
    else {
      if (!(menu = MENU_GetMenu( hMenu )))
        return NULL;
    }
  }

  /* Make sure that MDI system buttons stay on the right side.
  * Note: XP treats only bitmap handles 1 - 6 as "magic" ones
  * regardless of their id.
  */
  while (pos > 0 && (menu->items[pos - 1].fType & GXMFT_BITMAP) &&
    (GXINT_PTR)menu->items[pos - 1].hbmpItem >= (GXINT_PTR)GXHBMMENU_SYSTEM &&
    (GXINT_PTR)menu->items[pos - 1].hbmpItem <= (GXINT_PTR)GXHBMMENU_MBAR_CLOSE_D)
    pos--;

  TRACE("inserting at %u by pos %u\n", pos, flags & GXMF_BYPOSITION);

  /* Create new items array */

  newItems = (MENUITEM*)gxHeapAlloc( gxGetProcessHeap(), 0, sizeof(MENUITEM) * (menu->nItems+1) );
  if (!newItems)
  {
    WARN("allocation failed\n" );
    return NULL;
  }
  if (menu->nItems > 0)
  {
    /* Copy the old array into the new one */
    if (pos > 0) memcpy( newItems, menu->items, pos * sizeof(MENUITEM) );
    if (pos < menu->nItems) memcpy( &newItems[pos+1], &menu->items[pos],
      (menu->nItems-pos)*sizeof(MENUITEM) );
    gxHeapFree( gxGetProcessHeap(), 0, menu->items );
  }
  menu->items = newItems;
  menu->nItems++;
  memset( &newItems[pos], 0, sizeof(*newItems) );
  menu->Height = 0; /* force size recalculate */
  return &newItems[pos];
}


/**********************************************************************
*         MENU_ParseResource
*
* Parse a standard menu resource and add items to the menu.
* Return a pointer to the end of the resource.
*
* NOTE: flags is equivalent to the mtOption field
*/
static GXLPCSTR MENU_ParseResource( GXLPCSTR res, GXHMENU hMenu, GXBOOL unicode )
{
  GXWORD flags, id = 0;
  GXLPCSTR str;
  GXBOOL end_flag;

  do
  {
    flags = GET_WORD(res);
    end_flag = flags & GXMF_END;
    /* Remove GXMF_END because it has the same value as MF_HILITE */
    flags &= ~GXMF_END;
    res += sizeof(GXWORD);
    if (!(flags & GXMF_POPUP))
    {
      id = GET_WORD(res);
      res += sizeof(GXWORD);
    }
    str = res;
    if (!unicode) res += strlen(str) + 1;
    else res += (GXSTRLEN((GXLPCWSTR)str) + 1) * sizeof(GXWCHAR);
    if (flags & GXMF_POPUP)
    {
      GXHMENU hSubMenu = gxCreatePopupMenu();
      if (!hSubMenu) return NULL;
      if (!(res = MENU_ParseResource( res, hSubMenu, unicode )))
        return NULL;
      /*if (!unicode) gxAppendMenuA( hMenu, flags, (GXUINT_PTR)hSubMenu, str );
      else */gxAppendMenuW( hMenu, flags, (GXUINT_PTR)hSubMenu, (GXLPCWSTR)str );
    }
    else  /* Not a popup */
    {
      /*if (!unicode) gxAppendMenuA( hMenu, flags, id, *str ? str : NULL );
      else */gxAppendMenuW( hMenu, flags, id,
      *(GXLPCWSTR)str ? (GXLPCWSTR)str : NULL );
    }
  } while (!end_flag);
  return res;
}


/**********************************************************************
*         MENUEX_ParseResource
*
* Parse an extended menu resource and add items to the menu.
* Return a pointer to the end of the resource.
*/
static GXLPCSTR MENUEX_ParseResource( GXLPCSTR res, GXHMENU hMenu)
{
  GXWORD resinfo;
  do {
    GXMENUITEMINFOW mii;

    mii.cbSize = sizeof(mii);
    mii.fMask = GXMIIM_STATE | GXMIIM_ID | GXMIIM_TYPE;
    mii.fType = GET_DWORD(res);
    res += sizeof(GXDWORD);
    mii.fState = GET_DWORD(res);
    res += sizeof(GXDWORD);
    mii.wID = GET_DWORD(res);
    res += sizeof(GXDWORD);
    resinfo = GET_WORD(res); /* FIXME: for 16-bit apps this is a byte.  */
    res += sizeof(GXWORD);
    /* Align the text on a word boundary.  */
    res += (~((GXUINT_PTR)res - 1)) & 1;
    mii.dwTypeData = (GXLPWSTR) res;
    res += (1 + GXSTRLEN(mii.dwTypeData)) * sizeof(GXWCHAR);
    /* Align the following fields on a dword boundary.  */
    res += (~((GXUINT_PTR)res - 1)) & 3;

    TRACE("Menu item: [%08x,%08x,%04x,%04x,%s]\n",
      mii.fType, mii.fState, mii.wID, resinfo, debugstr_w(mii.dwTypeData));

    if (resinfo & 1) {  /* Pop-up? */
      /* GXDWORD helpid = GET_DWORD(res); FIXME: use this.  */
      res += sizeof(GXDWORD);
      mii.hSubMenu = gxCreatePopupMenu();
      if (!mii.hSubMenu)
        return NULL;
      if (!(res = MENUEX_ParseResource(res, mii.hSubMenu))) {
        gxDestroyMenu(mii.hSubMenu);
        return NULL;
      }
      mii.fMask |= GXMIIM_SUBMENU;
      mii.fType |= GXMF_POPUP;
    }
    else if(!*mii.dwTypeData && !(mii.fType & GXMF_SEPARATOR))
    {
      WARN("Converting NULL menu item %04x, type %04x to SEPARATOR\n",
        mii.wID, mii.fType);
      mii.fType |= GXMF_SEPARATOR;
    }
    gxInsertMenuItemW(hMenu, -1, GXMF_BYPOSITION, &mii);
  } while (!(resinfo & GXMF_END));
  return res;
}


/***********************************************************************
*           MENU_GetSubPopup
*
* Return the handle of the selected sub-popup menu (if any).
*/
static GXHMENU MENU_GetSubPopup( GXHMENU hmenu )
{
  POPUPMENU *menu;
  MENUITEM *item;

  menu = MENU_GetMenu( hmenu );

  if ((!menu) || (menu->FocusedItem == NO_SELECTED_ITEM)) return 0;

  item = &menu->items[menu->FocusedItem];
  if ((item->fType & GXMF_POPUP) && (item->fState & GXMF_MOUSESELECT))
    return item->hSubMenu;
  return 0;
}


/***********************************************************************
*           MENU_HideSubPopups
*
* Hide the sub-popup menus of this menu.
*/
static void MENU_HideSubPopups( GXHWND hWndOwner, GXHMENU hmenu,
                 GXBOOL sendMenuSelect )
{
  POPUPMENU *menu = MENU_GetMenu( hmenu );

  TRACE("owner=%p hmenu=%p 0x%04x\n", hWndOwner, hmenu, sendMenuSelect);

  if (menu && top_popup)
  {
    GXHMENU hsubmenu;
    POPUPMENU *submenu;
    MENUITEM *item;

    if (menu->FocusedItem != NO_SELECTED_ITEM)
    {
      item = &menu->items[menu->FocusedItem];
      if (!(item->fType & GXMF_POPUP) ||
        !(item->fState & GXMF_MOUSESELECT)) return;
      item->fState &= ~GXMF_MOUSESELECT;
      hsubmenu = item->hSubMenu;
    } else return;

    submenu = MENU_GetMenu( hsubmenu );
    MENU_HideSubPopups( hWndOwner, hsubmenu, FALSE );
    MENU_SelectItem( hWndOwner, hsubmenu, NO_SELECTED_ITEM, sendMenuSelect, 0 );
    gxDestroyWindow( submenu->hWnd );
    submenu->hWnd = 0;
  }
}


/***********************************************************************
*           MENU_ShowSubPopup
*
* Display the sub-menu of the selected item of this menu.
* Return the handle of the submenu, or hmenu if no submenu to display.
*/
static GXHMENU MENU_ShowSubPopup( GXHWND hWndOwner, GXHMENU hmenu,
                 GXBOOL selectFirst, GXUINT wFlags )
{
  GXRECT rect;
  POPUPMENU *menu;
  MENUITEM *item;
  GXHDC hdc;

  TRACE("owner=%p hmenu=%p 0x%04x\n", hWndOwner, hmenu, selectFirst);

  if (!(menu = MENU_GetMenu( hmenu ))) return hmenu;

  if (menu->FocusedItem == NO_SELECTED_ITEM) return hmenu;

  item = &menu->items[menu->FocusedItem];
  if (!(item->fType & GXMF_POPUP) || (item->fState & (GXMF_GRAYED | GXMF_DISABLED)))
    return hmenu;

  /* message must be sent before using item,
  because nearly everything may be changed by the application ! */

  /* Send GXWM_INITMENUPOPUP message only if TPM_NONOTIFY flag is not specified */
  if (!(wFlags & GXTPM_NONOTIFY))
    gxSendMessageW( hWndOwner, GXWM_INITMENUPOPUP, (GXWPARAM)item->hSubMenu,
    GXMAKELONG( menu->FocusedItem, IS_SYSTEM_MENU(menu) ));

  item = &menu->items[menu->FocusedItem];
  rect = item->rect;

  /* correct item if modified as a reaction to GXWM_INITMENUPOPUP message */
  if (!(item->fState & GXMF_HILITE))
  {
    if (menu->wFlags & GXMF_POPUP) hdc = gxGetDC( menu->hWnd );
    else hdc = gxGetDCEx( menu->hWnd, 0, GXDCX_CACHE | GXDCX_WINDOW);

    gxSelectObject( hdc, get_menu_font(FALSE));

    item->fState |= GXMF_HILITE;
    MENU_DrawMenuItem( menu->hWnd, hmenu, hWndOwner, hdc, item, menu->Height, !(menu->wFlags & GXMF_POPUP), GXODA_DRAWENTIRE );
    gxReleaseDC( menu->hWnd, hdc );
  }
  if (!item->rect.top && !item->rect.left && !item->rect.bottom && !item->rect.right)
    item->rect = rect;

  item->fState |= GXMF_MOUSESELECT;

  if (IS_SYSTEM_MENU(menu))
  {
    MENU_InitSysMenuPopup(item->hSubMenu,
      gxGetWindowLongW( menu->hWnd, GXGWL_STYLE ),
      gxGetClassLongW( menu->hWnd, GXGCL_STYLE));

    NC_GetSysPopupPos( menu->hWnd, &rect );
    rect.top = rect.bottom;
    rect.right = gxGetSystemMetrics(GXSM_CXSIZE);
    rect.bottom = gxGetSystemMetrics(GXSM_CYSIZE);
  }
  else
  {
    gxGetWindowRect( menu->hWnd, &rect );
    if (menu->wFlags & GXMF_POPUP)
    {
      GXRECT rc = item->rect;

      MENU_AdjustMenuItemRect(menu, &rc);

      /* The first item in the popup menu has to be at the
      same y position as the focused menu item */
      rect.left += rc.right - gxGetSystemMetrics(GXSM_CXBORDER);
      rect.top += rc.top - MENU_TOP_MARGIN;
      rect.right = rc.left - rc.right + gxGetSystemMetrics(GXSM_CXBORDER);
      rect.bottom = rc.top - rc.bottom - MENU_TOP_MARGIN
        - MENU_BOTTOM_MARGIN - gxGetSystemMetrics(GXSM_CYBORDER);
    }
    else
    {
      rect.left += item->rect.left;
      rect.top += item->rect.bottom;
      rect.right = item->rect.right - item->rect.left;
      rect.bottom = item->rect.bottom - item->rect.top;
    }
  }

  MENU_ShowPopup( hWndOwner, item->hSubMenu, menu->FocusedItem,
    rect.left, rect.top, rect.right, rect.bottom );
  if (selectFirst)
    MENU_MoveSelection( hWndOwner, item->hSubMenu, ITEM_NEXT );
  return item->hSubMenu;
}



/**********************************************************************
*         MENU_IsMenuActive
*/
GXHWND MENU_IsMenuActive()
{
  return top_popup;
}

/***********************************************************************
*           MENU_PtMenu
*
* Walks menu chain trying to find a menu pt maps to.
*/
static GXHMENU MENU_PtMenu( GXHMENU hMenu, GXPOINT pt )
{
  POPUPMENU *menu = MENU_GetMenu( hMenu );
  GXUINT item = menu->FocusedItem;
  GXHMENU ret;

  /* try subpopup first (if any) */
  ret = (item != NO_SELECTED_ITEM &&
    (menu->items[item].fType & GXMF_POPUP) &&
    (menu->items[item].fState & GXMF_MOUSESELECT))
    ? MENU_PtMenu(menu->items[item].hSubMenu, pt) : 0;

  if (!ret)  /* check the current window (avoiding GXWM_HITTEST) */
  {
    GXINT ht = NC_HandleNCHitTest( menu->hWnd, pt );
    if( menu->wFlags & GXMF_POPUP )
    {
      if (ht != GXHTNOWHERE && ht != GXHTERROR) ret = hMenu;
    }
    else if (ht == GXHTSYSMENU)
      ret = get_win_sys_menu( menu->hWnd );
    else if (ht == GXHTMENU)
      ret = gxGetMenu( menu->hWnd );
  }
  return ret;
}

/***********************************************************************
*           MENU_ExecFocusedItem
*
* Execute a menu item (for instance when user pressed Enter).
* Return the wID of the executed item. Otherwise, -1 indicating
* that no menu item was executed, -2 if a popup is shown;
* Have to receive the flags for the TrackPopupMenu options to avoid
* sending unwanted message.
*
*/
static GXINT MENU_ExecFocusedItem( MTRACKER* pmt, GXHMENU hMenu, GXUINT wFlags )
{
  MENUITEM *item;
  POPUPMENU *menu = MENU_GetMenu( hMenu );

  TRACE("%p hmenu=%p\n", pmt, hMenu);

  if (!menu || !menu->nItems ||
    (menu->FocusedItem == NO_SELECTED_ITEM)) return -1;

  item = &menu->items[menu->FocusedItem];

  TRACE("hMenu %p wID %08lx hSubMenu %p fType %04x\n", hMenu, item->wID, item->hSubMenu, item->fType);

  if (!(item->fType & GXMF_POPUP))
  {
    if (!(item->fState & (GXMF_GRAYED | GXMF_DISABLED)) && !(item->fType & GXMF_SEPARATOR))
    {
      /* If TPM_RETURNCMD is set you return the id, but
      do not send a message to the owner */
      if(!(wFlags & GXTPM_RETURNCMD))
      {
        if( menu->wFlags & GXMF_SYSMENU )
          gxPostMessageW( pmt->hOwnerWnd, GXWM_SYSCOMMAND, item->wID,
          GXMAKELPARAM((GXINT16)pmt->pt.x, (GXINT16)pmt->pt.y) );
        else
        {
          if (menu->dwStyle & GXMNS_NOTIFYBYPOS)
            gxPostMessageW( pmt->hOwnerWnd, GXWM_MENUCOMMAND, menu->FocusedItem,
            (GXLPARAM)hMenu);
          else
            gxPostMessageW( pmt->hOwnerWnd, GXWM_COMMAND, item->wID, 0 );
        }
      }
      return item->wID;
    }
  }
  else
  {
    pmt->hCurrentMenu = MENU_ShowSubPopup(pmt->hOwnerWnd, hMenu, TRUE, wFlags);
    return -2;
  }

  return -1;
}

/***********************************************************************
*           MENU_SwitchTracking
*
* Helper function for menu navigation routines.
*/

static void MENU_SwitchTracking( MTRACKER* pmt, GXHMENU hPtMenu, GXUINT id )
{
  POPUPMENU *ptmenu = MENU_GetMenu( hPtMenu );
  POPUPMENU *topmenu = MENU_GetMenu( pmt->hTopMenu );

  TRACE("%p hmenu=%p 0x%04x\n", pmt, hPtMenu, id);

  if( pmt->hTopMenu != hPtMenu &&
    !((ptmenu->wFlags | topmenu->wFlags) & GXMF_POPUP) )
  {
    /* both are top level menus (system and menu-bar) */
    MENU_HideSubPopups( pmt->hOwnerWnd, pmt->hTopMenu, FALSE );
    MENU_SelectItem( pmt->hOwnerWnd, pmt->hTopMenu, NO_SELECTED_ITEM, FALSE, 0 );
    pmt->hTopMenu = hPtMenu;
  }
  else MENU_HideSubPopups( pmt->hOwnerWnd, hPtMenu, FALSE );
  MENU_SelectItem( pmt->hOwnerWnd, hPtMenu, id, TRUE, 0 );
}

/***********************************************************************
*           MENU_ButtonDown
*
* Return TRUE if we can go on with menu tracking.
*/
static GXBOOL MENU_ButtonDown( MTRACKER* pmt, GXHMENU hPtMenu, GXUINT wFlags )
{
  TRACE("%p hPtMenu=%p\n", pmt, hPtMenu);

  if (hPtMenu)
  {
    GXUINT id = 0;
    POPUPMENU *ptmenu = MENU_GetMenu( hPtMenu );
    MENUITEM *item;

    if( IS_SYSTEM_MENU(ptmenu) )
      item = ptmenu->items;
    else
      item = MENU_FindItemByCoords( ptmenu, pmt->pt, &id );

    if( item )
    {
      if( ptmenu->FocusedItem != id )
        MENU_SwitchTracking( pmt, hPtMenu, id );

      /* If the popup menu is not already "popped" */
      if(!(item->fState & GXMF_MOUSESELECT ))
      {
        pmt->hCurrentMenu = MENU_ShowSubPopup( pmt->hOwnerWnd, hPtMenu, FALSE, wFlags );
      }

      return TRUE;
    }
    /* Else the click was on the menu bar, finish the tracking */
  }
  return FALSE;
}

/***********************************************************************
*           MENU_ButtonUp
*
* Return the value of MENU_ExecFocusedItem if
* the selected item was not a popup. Else open the popup.
* A -1 return value indicates that we go on with menu tracking.
*
*/
static GXINT MENU_ButtonUp( MTRACKER* pmt, GXHMENU hPtMenu, GXUINT wFlags)
{
  TRACE("%p hmenu=%p\n", pmt, hPtMenu);

  if (hPtMenu)
  {
    GXUINT id = 0;
    POPUPMENU *ptmenu = MENU_GetMenu( hPtMenu );
    MENUITEM *item;

    if( IS_SYSTEM_MENU(ptmenu) )
      item = ptmenu->items;
    else
      item = MENU_FindItemByCoords( ptmenu, pmt->pt, &id );

    if( item && (ptmenu->FocusedItem == id ))
    {
      debug_print_menuitem ("FocusedItem: ", item, "");

      if( !(item->fType & GXMF_POPUP) )
      {
        GXINT executedMenuId = MENU_ExecFocusedItem( pmt, hPtMenu, wFlags);
        if (executedMenuId == -1 || executedMenuId == -2) return -1;
        return executedMenuId;
      }

      /* If we are dealing with the top-level menu            */
      /* and this is a click on an already "popped" item:     */
      /* Stop the menu tracking and close the opened submenus */
      if((pmt->hTopMenu == hPtMenu) && ptmenu->bTimeToHide)
        return 0;
    }
    ptmenu->bTimeToHide = TRUE;
  }
  return -1;
}


/***********************************************************************
*           MENU_MouseMove
*
* Return TRUE if we can go on with menu tracking.
*/
static GXBOOL MENU_MouseMove( MTRACKER* pmt, GXHMENU hPtMenu, GXUINT wFlags )
{
  GXUINT id = NO_SELECTED_ITEM;
  POPUPMENU *ptmenu = NULL;

  if( hPtMenu )
  {
    ptmenu = MENU_GetMenu( hPtMenu );
    if( IS_SYSTEM_MENU(ptmenu) )
      id = 0;
    else
      MENU_FindItemByCoords( ptmenu, pmt->pt, &id );
  }

  if( id == NO_SELECTED_ITEM )
  {
    MENU_SelectItem( pmt->hOwnerWnd, pmt->hCurrentMenu,
      NO_SELECTED_ITEM, TRUE, pmt->hTopMenu);

  }
  else if( ptmenu->FocusedItem != id )
  {
    MENU_SwitchTracking( pmt, hPtMenu, id );
    pmt->hCurrentMenu = MENU_ShowSubPopup(pmt->hOwnerWnd, hPtMenu, FALSE, wFlags);
  }
  return TRUE;
}


/***********************************************************************
*           MENU_SetCapture
*/
static void MENU_SetCapture( GXHWND hWnd )
{
  GXHWND previous = 0;
  /*  Artint
  SERVER_START_REQ( set_capture_window )
  {
  req->handle = hWnd;
  req->flags  = CAPTURE_MENU;
  if (!wine_server_call_err( req ))
  {
  previous = reply->previous;
  hWnd = reply->full_handle;
  }
  }
  SERVER_END_REQ;
  */
  if (previous && previous != hWnd)
    gxSendMessageW( previous, GXWM_CAPTURECHANGED, 0, (GXLPARAM)hWnd );
}


/***********************************************************************
*           MENU_DoNextMenu
*
* NOTE: GXWM_NEXTMENU documented in Win32 is a bit different.
*/
static GXLRESULT MENU_DoNextMenu( MTRACKER* pmt, GXUINT vk )
{
  POPUPMENU *menu = MENU_GetMenu( pmt->hTopMenu );
  GXBOOL atEnd = FALSE;

  /* When skipping left, we need to do something special after the
  first menu.                                                  */
  if (vk == GXVK_LEFT && menu->FocusedItem == 0)
  {
    atEnd = TRUE;
  }
  /* When skipping right, for the non-system menu, we need to
  handle the last non-special menu item (ie skip any window
  icons such as MDI maximize, restore or close)             */
  else if ((vk == GXVK_RIGHT) && !IS_SYSTEM_MENU(menu))
  {
    int i = menu->FocusedItem + 1;
    while (i < menu->nItems) {
      if ((menu->items[i].wID >= GXSC_SIZE &&
        menu->items[i].wID <= GXSC_RESTORE)) {
          i++;
      } else break;
    }
    if (i == menu->nItems) {
      atEnd = TRUE;
    }
  }
  /* When skipping right, we need to cater for the system menu */
  else if ((vk == GXVK_RIGHT) && IS_SYSTEM_MENU(menu))
  {
    if (menu->FocusedItem == (menu->nItems - 1)) {
      atEnd = TRUE;
    }
  }

  if( atEnd )
  {
    GXMDINEXTMENU next_menu;
    GXHMENU hNewMenu;
    GXHWND  hNewWnd;
    GXUINT  id = 0;

    next_menu.hmenuIn = (IS_SYSTEM_MENU(menu)) ? gxGetSubMenu(pmt->hTopMenu,0) : pmt->hTopMenu;
    next_menu.hmenuNext = 0;
    next_menu.hwndNext = 0;
    // TODO: warning C4700: 使用了未初始化的局部变量“hNewWnd”
    gxSendMessageW( hNewWnd, GXWM_NEXTMENU, vk, (GXLPARAM)&next_menu );

    TRACE("%p [%p] -> %p [%p]\n",
      pmt->hCurrentMenu, hNewWnd, next_menu.hmenuNext, next_menu.hwndNext );

    if (!next_menu.hmenuNext || !next_menu.hwndNext)
    {
      GXDWORD style = gxGetWindowLongW( hNewWnd, GXGWL_STYLE );
      hNewWnd = hNewWnd;
      if( IS_SYSTEM_MENU(menu) )
      {
        /* switch to the menu bar */

        if(style & GXWS_CHILD || !(hNewMenu = gxGetMenu(hNewWnd))) return FALSE;

        if( vk == GXVK_LEFT )
        {
          menu = MENU_GetMenu( hNewMenu );
          id = menu->nItems - 1;

          /* Skip backwards over any system predefined icons,
          eg. MDI close, restore etc icons                 */
          while ((id > 0) &&
            (menu->items[id].wID >= GXSC_SIZE &&
            menu->items[id].wID <= GXSC_RESTORE)) id--;
        }
      }
      else if (style & GXWS_SYSMENU )
      {
        /* switch to the system menu */
        hNewMenu = get_win_sys_menu( hNewWnd );
      }
      else return FALSE;
    }
    else    /* application returned a new menu to switch to */
    {
      hNewMenu = next_menu.hmenuNext;
      hNewWnd = WIN_GetFullHandle( next_menu.hwndNext );

      if( gxIsMenu(hNewMenu) && gxIsWindow(hNewWnd) )
      {
        GXDWORD style = gxGetWindowLongW( hNewWnd, GXGWL_STYLE );

        if (style & GXWS_SYSMENU &&
          gxGetSubMenu(get_win_sys_menu(hNewWnd), 0) == hNewMenu )
        {
          /* get the real system menu */
          hNewMenu =  get_win_sys_menu(hNewWnd);
        }
        else if (style & GXWS_CHILD || gxGetMenu(hNewWnd) != hNewMenu )
        {
          /* FIXME: Not sure what to do here;
          * perhaps try to track hNewMenu as a popup? */

          TRACE(" -- got confused.\n");
          return FALSE;
        }
      }
      else return FALSE;
    }

    if( hNewMenu != pmt->hTopMenu )
    {
      MENU_SelectItem( hNewWnd, pmt->hTopMenu, NO_SELECTED_ITEM,
        FALSE, 0 );
      if( pmt->hCurrentMenu != pmt->hTopMenu )
        MENU_HideSubPopups( hNewWnd, pmt->hTopMenu, FALSE );
    }

    if( hNewWnd != hNewWnd )
    {
      hNewWnd = hNewWnd;
      MENU_SetCapture( hNewWnd );
    }

    pmt->hTopMenu = pmt->hCurrentMenu = hNewMenu; /* all subpopups are hidden */
    MENU_SelectItem( hNewWnd, pmt->hTopMenu, id, TRUE, 0 );

    return TRUE;
  }
  return FALSE;
}

/***********************************************************************
*           MENU_SuspendPopup
*
* The idea is not to show the popup if the next input message is
* going to hide it anyway.
*/
static GXBOOL MENU_SuspendPopup( MTRACKER* pmt, GXUINT16 uMsg )
{
  GXMSG msg;

  msg.hwnd = pmt->hOwnerWnd;

  gxPeekMessageW( &msg, 0, 0, 0, GXPM_NOYIELD | GXPM_REMOVE);
  pmt->trackFlags |= TF_SKIPREMOVE;

  switch( uMsg )
  {
  case GXWM_KEYDOWN:
    gxPeekMessageW( &msg, 0, 0, 0, GXPM_NOYIELD | GXPM_NOREMOVE);
    if( msg.message == GXWM_KEYUP || msg.message == GXWM_PAINT )
    {
      gxPeekMessageW( &msg, 0, 0, 0, GXPM_NOYIELD | GXPM_REMOVE);
      gxPeekMessageW( &msg, 0, 0, 0, GXPM_NOYIELD | GXPM_NOREMOVE);
      if( msg.message == GXWM_KEYDOWN &&
        (msg.wParam == GXVK_LEFT || msg.wParam == GXVK_RIGHT))
      {
        pmt->trackFlags |= TF_SUSPENDPOPUP;
        return TRUE;
      }
    }
    break;
  }

  /* failures go through this */
  pmt->trackFlags &= ~TF_SUSPENDPOPUP;
  return FALSE;
}

/***********************************************************************
*           MENU_KeyEscape
*
* Handle a GXVK_ESCAPE key event in a menu.
*/
static GXBOOL MENU_KeyEscape(MTRACKER* pmt, GXUINT wFlags)
{
  GXBOOL bEndMenu = TRUE;

  if (pmt->hCurrentMenu != pmt->hTopMenu)
  {
    POPUPMENU *menu = MENU_GetMenu(pmt->hCurrentMenu);

    if (menu->wFlags & GXMF_POPUP)
    {
      GXHMENU hmenutmp, hmenuprev;

      hmenuprev = hmenutmp = pmt->hTopMenu;

      /* close topmost popup */
      while (hmenutmp != pmt->hCurrentMenu)
      {
        hmenuprev = hmenutmp;
        hmenutmp = MENU_GetSubPopup( hmenuprev );
      }

      MENU_HideSubPopups( pmt->hOwnerWnd, hmenuprev, TRUE );
      pmt->hCurrentMenu = hmenuprev;
      bEndMenu = FALSE;
    }
  }

  return bEndMenu;
}

/***********************************************************************
*           MENU_KeyLeft
*
* Handle a GXVK_LEFT key event in a menu.
*/
static void MENU_KeyLeft( MTRACKER* pmt, GXUINT wFlags )
{
  POPUPMENU *menu;
  GXHMENU hmenutmp, hmenuprev;
  GXUINT  prevcol;

  hmenuprev = hmenutmp = pmt->hTopMenu;
  menu = MENU_GetMenu( hmenutmp );

  /* Try to move 1 column left (if possible) */
  if( (prevcol = MENU_GetStartOfPrevColumn( pmt->hCurrentMenu )) !=
    NO_SELECTED_ITEM ) {

      MENU_SelectItem( pmt->hOwnerWnd, pmt->hCurrentMenu,
        prevcol, TRUE, 0 );
      return;
  }

  /* close topmost popup */
  while (hmenutmp != pmt->hCurrentMenu)
  {
    hmenuprev = hmenutmp;
    hmenutmp = MENU_GetSubPopup( hmenuprev );
  }

  MENU_HideSubPopups( pmt->hOwnerWnd, hmenuprev, TRUE );
  pmt->hCurrentMenu = hmenuprev;

  if ( (hmenuprev == pmt->hTopMenu) && !(menu->wFlags & GXMF_POPUP) )
  {
    /* move menu bar selection if no more popups are left */

    if( !MENU_DoNextMenu( pmt, GXVK_LEFT) )
      MENU_MoveSelection( pmt->hOwnerWnd, pmt->hTopMenu, ITEM_PREV );

    if ( hmenuprev != hmenutmp || pmt->trackFlags & TF_SUSPENDPOPUP )
    {
      /* A sublevel menu was displayed - display the next one
      * unless there is another displacement coming up */

      if( !MENU_SuspendPopup( pmt, GXWM_KEYDOWN ) )
        pmt->hCurrentMenu = MENU_ShowSubPopup(pmt->hOwnerWnd,
        pmt->hTopMenu, TRUE, wFlags);
    }
  }
}


/***********************************************************************
*           MENU_KeyRight
*
* Handle a GXVK_RIGHT key event in a menu.
*/
static void MENU_KeyRight( MTRACKER* pmt, GXUINT wFlags )
{
  GXHMENU hmenutmp;
  POPUPMENU *menu = MENU_GetMenu( pmt->hTopMenu );
  GXUINT  nextcol;

  TRACE("MENU_KeyRight called, cur %p (%s), top %p (%s).\n",
    pmt->hCurrentMenu,
    debugstr_w((MENU_GetMenu(pmt->hCurrentMenu))->items[0].text),
    pmt->hTopMenu, debugstr_w(menu->items[0].text) );

  if ( (menu->wFlags & GXMF_POPUP) || (pmt->hCurrentMenu != pmt->hTopMenu))
  {
    /* If already displaying a popup, try to display sub-popup */

    hmenutmp = pmt->hCurrentMenu;
    pmt->hCurrentMenu = MENU_ShowSubPopup(pmt->hOwnerWnd, hmenutmp, TRUE, wFlags);

    /* if subpopup was displayed then we are done */
    if (hmenutmp != pmt->hCurrentMenu) return;
  }

  /* Check to see if there's another column */
  if( (nextcol = MENU_GetStartOfNextColumn( pmt->hCurrentMenu )) !=
    NO_SELECTED_ITEM ) {
      TRACE("Going to %d.\n", nextcol );
      MENU_SelectItem( pmt->hOwnerWnd, pmt->hCurrentMenu,
        nextcol, TRUE, 0 );
      return;
  }

  if (!(menu->wFlags & GXMF_POPUP))  /* menu bar tracking */
  {
    if( pmt->hCurrentMenu != pmt->hTopMenu )
    {
      MENU_HideSubPopups( pmt->hOwnerWnd, pmt->hTopMenu, FALSE );
      hmenutmp = pmt->hCurrentMenu = pmt->hTopMenu;
    } else hmenutmp = 0;

    /* try to move to the next item */
    if( !MENU_DoNextMenu( pmt, GXVK_RIGHT) )
      MENU_MoveSelection( pmt->hOwnerWnd, pmt->hTopMenu, ITEM_NEXT );

    if( hmenutmp || pmt->trackFlags & TF_SUSPENDPOPUP )
      if( !MENU_SuspendPopup(pmt, GXWM_KEYDOWN) )
        pmt->hCurrentMenu = MENU_ShowSubPopup(pmt->hOwnerWnd,
        pmt->hTopMenu, TRUE, wFlags);
  }
}

/***********************************************************************
*           MENU_TrackMenu
*
* Menu tracking code.
*/
static GXBOOL MENU_TrackMenu( GXHMENU hmenu, GXUINT wFlags, GXINT x, GXINT y,
               GXHWND hWnd, const GXRECT *lprect )
{
  GXMSG msg;
  POPUPMENU *menu;
  GXBOOL fRemove;
  GXINT executedMenuId = -1;
  MTRACKER mt;
  GXBOOL enterIdleSent = FALSE;

  mt.trackFlags = 0;
  mt.hCurrentMenu = hmenu;
  mt.hTopMenu = hmenu;
  mt.hOwnerWnd = WIN_GetFullHandle( hWnd );
  mt.pt.x = x;
  mt.pt.y = y;

  TRACE("hmenu=%p flags=0x%08x (%d,%d) hWnd=%p %s\n",
    hmenu, wFlags, x, y, hWnd, wine_dbgstr_rect( lprect));

  fEndMenu = FALSE;
  if (!(menu = MENU_GetMenu( hmenu )))
  {
    WARN("Invalid menu handle %p\n", hmenu);
    gxSetLastError(GXERROR_INVALID_MENU_HANDLE);
    return FALSE;
  }

  if (wFlags & GXTPM_BUTTONDOWN)
  {
    /* Get the result in order to start the tracking or not */
    fRemove = MENU_ButtonDown( &mt, hmenu, wFlags );
    fEndMenu = !fRemove;
  }

  if (wFlags & TF_ENDMENU) fEndMenu = TRUE;

  MENU_SetCapture( mt.hOwnerWnd );

  while (!fEndMenu)
  {
    menu = MENU_GetMenu( mt.hCurrentMenu );
    if (!menu) /* sometimes happens if I do a window manager close */
      break;

    /* we have to keep the message in the queue until it's
    * clear that menu loop is not over yet. */

    for (;;)
    {
      if (gxPeekMessageW( &msg, 0, 0, 0, GXPM_NOREMOVE ))
      {
        if (!gxCallMsgFilterW( &msg, GXMSGF_MENU )) break;
        /* remove the message from the queue */
        gxPeekMessageW( &msg, 0, msg.message, msg.message, GXPM_REMOVE );
      }
      else
      {
        if (!enterIdleSent)
        {
          GXHWND win = (wFlags & GXTPM_ENTERIDLEEX && menu->wFlags & GXMF_POPUP) ? menu->hWnd : 0;
          enterIdleSent = TRUE;
          gxSendMessageW( mt.hOwnerWnd, GXWM_ENTERIDLE, GXMSGF_MENU, (GXLPARAM)win );
        }
        gxWaitMessage();
      }
    }

    /* check if EndMenu() tried to cancel us, by posting this message */
    if(msg.message == GXWM_CANCELMODE)
    {
      /* we are now out of the loop */
      fEndMenu = TRUE;

      /* remove the message from the queue */
      gxPeekMessageW( &msg, 0, msg.message, msg.message, GXPM_REMOVE );

      /* break out of internal loop, ala ESCAPE */
      break;
    }

    gxTranslateMessage( &msg );
    mt.pt = msg.pt;

    if ( (msg.hwnd==menu->hWnd) || (msg.message != GXWM_TIMER) )
      enterIdleSent=FALSE;

    fRemove = FALSE;
    if ((msg.message >= GXWM_MOUSEFIRST) && (msg.message <= GXWM_MOUSELAST))
    {
      /*
      * Use the mouse coordinates in lParam instead of those in the MSG
      * struct to properly handle synthetic messages. They are already
      * in screen coordinates.
      */
      mt.pt.x = (short)GXLOWORD(msg.lParam);
      mt.pt.y = (short)GXHIWORD(msg.lParam);

      /* Find a menu for this mouse event */
      hmenu = MENU_PtMenu( mt.hTopMenu, mt.pt );

      switch(msg.message)
      {
        /* no GXWM_NC... messages in captured state */

      case GXWM_RBUTTONDBLCLK:
      case GXWM_RBUTTONDOWN:
        if (!(wFlags & GXTPM_RIGHTBUTTON)) break;
        /* fall through */
      case GXWM_LBUTTONDBLCLK:
      case GXWM_LBUTTONDOWN:
        /* If the message belongs to the menu, removes it from the queue */
        /* Else, end menu tracking */
        fRemove = MENU_ButtonDown( &mt, hmenu, wFlags );
        fEndMenu = !fRemove;
        break;

      case GXWM_RBUTTONUP:
        if (!(wFlags & GXTPM_RIGHTBUTTON)) break;
        /* fall through */
      case GXWM_LBUTTONUP:
        /* Check if a menu was selected by the mouse */
        if (hmenu)
        {
          executedMenuId = MENU_ButtonUp( &mt, hmenu, wFlags);
          TRACE("executedMenuId %d\n", executedMenuId);

          /* End the loop if executedMenuId is an item ID */
          /* or if the job was done (executedMenuId = 0). */
          fEndMenu = fRemove = (executedMenuId != -1);
        }
        /* No menu was selected by the mouse */
        /* if the function was called by TrackPopupMenu, continue
        with the menu tracking. If not, stop it */
        else
          fEndMenu = ((wFlags & GXTPM_POPUPMENU) ? FALSE : TRUE);

        break;

      case GXWM_MOUSEMOVE:
        /* the selected menu item must be changed every time */
        /* the mouse moves. */

        if (hmenu)
          fEndMenu |= !MENU_MouseMove( &mt, hmenu, wFlags );

      } /* switch(msg.message) - mouse */
    }
    else if ((msg.message >= GXWM_KEYFIRST) && (msg.message <= GXWM_KEYLAST))
    {
      fRemove = TRUE;  /* Keyboard messages are always removed */
      switch(msg.message)
      {
      case GXWM_KEYDOWN:
      case GXWM_SYSKEYDOWN:
        switch(msg.wParam)
        {
        case GXVK_MENU:
          fEndMenu = TRUE;
          break;

        case GXVK_HOME:
        case GXVK_END:
          MENU_SelectItem( mt.hOwnerWnd, mt.hCurrentMenu,
            NO_SELECTED_ITEM, FALSE, 0 );
          MENU_MoveSelection( mt.hOwnerWnd, mt.hCurrentMenu,
            (msg.wParam == GXVK_HOME)? ITEM_NEXT : ITEM_PREV );
          break;

        case GXVK_UP:
        case GXVK_DOWN: /* If on menu bar, pull-down the menu */

          menu = MENU_GetMenu( mt.hCurrentMenu );
          if (!(menu->wFlags & GXMF_POPUP))
            mt.hCurrentMenu = MENU_ShowSubPopup(mt.hOwnerWnd, mt.hTopMenu, TRUE, wFlags);
          else      /* otherwise try to move selection */
            MENU_MoveSelection( mt.hOwnerWnd, mt.hCurrentMenu, 
            (msg.wParam == GXVK_UP)? ITEM_PREV : ITEM_NEXT );
          break;

        case GXVK_LEFT:
          MENU_KeyLeft( &mt, wFlags );
          break;

        case GXVK_RIGHT:
          MENU_KeyRight( &mt, wFlags );
          break;

        case GXVK_ESCAPE:
          fEndMenu = MENU_KeyEscape(&mt, wFlags);
          break;

        case GXVK_F1:
          {
            GXHELPINFO hi;
            hi.cbSize = sizeof(GXHELPINFO);
            hi.iContextType = GXHELPINFO_MENUITEM;
            if (menu->FocusedItem == NO_SELECTED_ITEM)
              hi.iCtrlId = 0;
            else
              hi.iCtrlId = menu->items[menu->FocusedItem].wID;
            hi.hItemHandle = hmenu;
            hi.dwContextId = menu->dwContextHelpID;
            hi.MousePos = *(GXPOINT*)&msg.pt;
            gxSendMessageW(hWnd, GXWM_HELP, 0, (GXLPARAM)&hi);
            break;
          }

        default:
          break;
        }
        break;  /* GXWM_KEYDOWN */

      case GXWM_CHAR:
      case GXWM_SYSCHAR:
        {
          GXUINT  pos;

          if (msg.wParam == '\r' || msg.wParam == ' ')
          {
            executedMenuId = MENU_ExecFocusedItem(&mt,mt.hCurrentMenu, wFlags);
            fEndMenu = (executedMenuId != -2);

            break;
          }

          /* Hack to avoid control chars. */
          /* We will find a better way real soon... */
          if (msg.wParam < 32) break;

          pos = MENU_FindItemByKey( mt.hOwnerWnd, mt.hCurrentMenu,
            GXLOWORD(msg.wParam), FALSE );
          if (pos == (GXUINT)-2) fEndMenu = TRUE;
          else if (pos == (GXUINT)-1) gxMessageBeep(0);
          else
          {
            MENU_SelectItem( mt.hOwnerWnd, mt.hCurrentMenu, pos,
              TRUE, 0 );
            executedMenuId = MENU_ExecFocusedItem(&mt,mt.hCurrentMenu, wFlags);
            fEndMenu = (executedMenuId != -2);
          }
        }
        break;
      }  /* switch(msg.message) - kbd */
    }
    else
    {
      gxPeekMessageW( &msg, 0, msg.message, msg.message, GXPM_REMOVE );
      gxDispatchMessageW( &msg );
      continue;
    }

    if (!fEndMenu) fRemove = TRUE;

    /* finally remove message from the queue */

    if (fRemove && !(mt.trackFlags & TF_SKIPREMOVE) )
      gxPeekMessageW( &msg, 0, msg.message, msg.message, GXPM_REMOVE );
    else mt.trackFlags &= ~TF_SKIPREMOVE;
  }

  MENU_SetCapture(0);  /* release the capture */

  /* If dropdown is still painted and the close box is clicked on
  then the menu will be destroyed as part of the DispatchMessage above.
  This will then invalidate the menu handle in mt.hTopMenu. We should
  check for this first.  */
  if( gxIsMenu( mt.hTopMenu ) )
  {
    menu = MENU_GetMenu( mt.hTopMenu );

    if( gxIsWindow( mt.hOwnerWnd ) )
    {
      MENU_HideSubPopups( mt.hOwnerWnd, mt.hTopMenu, FALSE );

      if (menu && (menu->wFlags & GXMF_POPUP))
      {
        gxDestroyWindow( menu->hWnd );
        menu->hWnd = 0;
      }
      MENU_SelectItem( mt.hOwnerWnd, mt.hTopMenu, NO_SELECTED_ITEM, FALSE, 0 );
      gxSendMessageW( mt.hOwnerWnd, GXWM_MENUSELECT, GXMAKELONG(0,0xffff), 0 );
    }

    /* Reset the variable for hiding menu */
    if( menu ) menu->bTimeToHide = FALSE;
  }

  /* The return value is only used by TrackPopupMenu */
  if (!(wFlags & GXTPM_RETURNCMD)) return TRUE;
  if (executedMenuId == -1) executedMenuId = 0;
  return executedMenuId;
}

/***********************************************************************
*           MENU_InitTracking
*/
static GXBOOL MENU_InitTracking(GXHWND hWnd, GXHMENU hMenu, GXBOOL bPopup, GXUINT wFlags)
{
  POPUPMENU *menu;

  TRACE("hWnd=%p hmenu=%p\n", hWnd, hMenu);

  gxHideCaret(0);

  /* Send GXWM_ENTERMENULOOP and GXWM_INITMENU message only if TPM_NONOTIFY flag is not specified */
  if (!(wFlags & GXTPM_NONOTIFY))
    gxSendMessageW( hWnd, GXWM_ENTERMENULOOP, bPopup, 0 );

  gxSendMessageW( hWnd, GXWM_SETCURSOR, (GXWPARAM)hWnd, GXHTCAPTION );

  if (!(wFlags & GXTPM_NONOTIFY))
  {
    gxSendMessageW( hWnd, GXWM_INITMENU, (GXWPARAM)hMenu, 0 );
    /* If an app changed/recreated menu bar entries in GXWM_INITMENU
    * menu sizes will be recalculated once the menu created/shown.
    */
  }

  /* This makes the menus of applications built with Delphi work.
  * It also enables menus to be displayed in more than one window,
  * but there are some bugs left that need to be fixed in this case.
  */
  if ((menu = MENU_GetMenu( hMenu ))) menu->hWnd = hWnd;

  return TRUE;
}
/***********************************************************************
*           MENU_ExitTracking
*/
static GXBOOL MENU_ExitTracking(GXHWND hWnd)
{
  TRACE("hWnd=%p\n", hWnd);

  gxSendMessageW( hWnd, GXWM_EXITMENULOOP, 0, 0 );
  gxShowCaret(0);
  top_popup = 0;
  return TRUE;
}

/***********************************************************************
*           MENU_TrackMouseMenuBar
*
* Menu-bar tracking upon a mouse event. Called from NC_HandleSysCommand().
*/
void MENU_TrackMouseMenuBar( GXHWND hWnd, GXINT ht, GXPOINT pt )
{
  GXHMENU hMenu = (ht == GXHTSYSMENU) ? get_win_sys_menu( hWnd ) : gxGetMenu( hWnd );
  GXUINT wFlags = GXTPM_ENTERIDLEEX | GXTPM_BUTTONDOWN | GXTPM_LEFTALIGN | GXTPM_LEFTBUTTON;

  TRACE("GXWND=%p ht=0x%04x %s\n", hWnd, ht, wine_dbgstr_point( &pt));

  if (gxIsMenu(hMenu))
  {
    MENU_InitTracking( hWnd, hMenu, FALSE, wFlags );
    MENU_TrackMenu( hMenu, wFlags, pt.x, pt.y, hWnd, NULL );
    MENU_ExitTracking(hWnd);
  }
}


/***********************************************************************
*           MENU_TrackKbdMenuBar
*
* Menu-bar tracking upon a keyboard event. Called from NC_HandleSysCommand().
*/
void MENU_TrackKbdMenuBar( GXHWND hWnd, GXUINT wParam, GXWCHAR wChar)
{
  GXUINT uItem = NO_SELECTED_ITEM;
  GXHMENU hTrackMenu;
  GXUINT wFlags = GXTPM_ENTERIDLEEX | GXTPM_LEFTALIGN | GXTPM_LEFTBUTTON;

  TRACE("hWnd %p wParam 0x%04x wChar 0x%04x\n", hWnd, wParam, wChar);

  /* find window that has a menu */

  while (!WIN_ALLOWED_MENU(gxGetWindowLongW( hWnd, GXGWL_STYLE )))
    if (!(hWnd = gxGetAncestor( hWnd, GXGA_PARENT ))) return;

  /* check if we have to track a system menu */

  hTrackMenu = gxGetMenu( hWnd );
  if (!hTrackMenu || gxIsIconic(hWnd) || wChar == ' ' )
  {
    if (!(gxGetWindowLongW( hWnd, GXGWL_STYLE ) & GXWS_SYSMENU)) return;
    hTrackMenu = get_win_sys_menu( hWnd );
    uItem = 0;
    wParam |= GXHTSYSMENU; /* prevent item lookup */
  }

  if (!gxIsMenu( hTrackMenu )) return;

  MENU_InitTracking( hWnd, hTrackMenu, FALSE, wFlags );

  if( wChar && wChar != ' ' )
  {
    uItem = MENU_FindItemByKey( hWnd, hTrackMenu, wChar, (wParam & GXHTSYSMENU) );
    if ( uItem >= (GXUINT)(-2) )
    {
      if( uItem == (GXUINT)(-1) ) gxMessageBeep(0);
      /* schedule end of menu tracking */
      wFlags |= TF_ENDMENU;
      goto track_menu;
    }
  }

  MENU_SelectItem( hWnd, hTrackMenu, uItem, TRUE, 0 );

  if (wParam & GXHTSYSMENU && wChar != ' ')
  {
    /* prevent sysmenu activation for managed windows on Alt down/up */
    ASSERT(FALSE);
    //Artint ??
    //if (GetPropA( hWnd, "__wine_x11_managed" ))
    //    wFlags |= TF_ENDMENU; /* schedule end of menu tracking */
  }
  else
  {
    if( uItem == NO_SELECTED_ITEM )
      MENU_MoveSelection( hWnd, hTrackMenu, ITEM_NEXT );
    else
      gxPostMessageW( hWnd, GXWM_KEYDOWN, GXVK_DOWN, 0L );
  }

track_menu:
  MENU_TrackMenu( hTrackMenu, wFlags, 0, 0, hWnd, NULL );
  MENU_ExitTracking( hWnd );
}


/**********************************************************************
*           gxTrackPopupMenu   (USER32.@)
*
* Like the win32 API, the function return the command ID only if the
* flag TPM_RETURNCMD is on.
*
*/
GXBOOL GXDLLAPI gxTrackPopupMenu( GXHMENU hMenu, GXUINT wFlags, GXINT x, GXINT y,
                GXINT nReserved, GXHWND hWnd, const GXRECT *lpRect )
{
  GXBOOL ret = FALSE;

  TRACE("hmenu %p flags %04x (%d,%d) reserved %d hWnd %p rect %s\n",
    hMenu, wFlags, x, y, nReserved, hWnd, wine_dbgstr_rect(lpRect));

  MENU_InitTracking(hWnd, hMenu, TRUE, wFlags);

  /* Send GXWM_INITMENUPOPUP message only if TPM_NONOTIFY flag is not specified */
  if (!(wFlags & GXTPM_NONOTIFY))
    gxSendMessageW( hWnd, GXWM_INITMENUPOPUP, (GXWPARAM)hMenu, 0);

  if (MENU_ShowPopup( hWnd, hMenu, 0, x, y, 0, 0 ))
    ret = MENU_TrackMenu( hMenu, wFlags | GXTPM_POPUPMENU, 0, 0, hWnd, lpRect );
  MENU_ExitTracking(hWnd);

  return ret;
}

/**********************************************************************
*           gxTrackPopupMenuEx   (USER32.@)
*/
GXBOOL GXDLLAPI gxTrackPopupMenuEx( GXHMENU hMenu, GXUINT wFlags, GXINT x, GXINT y,
                GXHWND hWnd, GXLPTPMPARAMS lpTpm )
{
  FIXME("not fully implemented\n" );
  return gxTrackPopupMenu( hMenu, wFlags, x, y, 0, hWnd,
    lpTpm ? (GXRECT*)&lpTpm->rcExclude : NULL );
}

/***********************************************************************
*           PopupMenuWndProc
*
* NOTE: Windows has totally different (and undocumented) popup GXWNDproc.
*/
static GXLRESULT GXCALLBACK PopupMenuWndProc( GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam )
{
  TRACE("hWnd=%p msg=0x%04x wp=0x%04lx lp=0x%08lx\n", hWnd, message, wParam, lParam);

  switch(message)
  {
  case GXWM_CREATE:
    {
      GXCREATESTRUCTW *cs = (GXCREATESTRUCTW*)lParam;
      gxSetWindowLongPtrW( hWnd, 0, (GXLONG_PTR)cs->lpCreateParams );
      return 0;
    }

  case GXWM_MOUSEACTIVATE:  /* We don't want to be activated */
    return GXMA_NOACTIVATE;

  case GXWM_PAINT:
    {
      GXPAINTSTRUCT ps;
      gxBeginPaint( hWnd, &ps );
      MENU_DrawPopupMenu( hWnd, ps.hdc,
        (GXHMENU)gxGetWindowLongPtrW( hWnd, 0 ) );
      gxEndPaint( hWnd, &ps );
      return 0;
    }
    //case GXWM_ERASEBKGND:
    //    return 1;

  case GXWM_DESTROY:
    /* zero out global pointer in case resident popup window was destroyed. */
    if (hWnd == top_popup) top_popup = 0;
    break;

  case GXWM_SHOWWINDOW:

    if( wParam )
    {
      if (!gxGetWindowLongPtrW( hWnd, 0 )) ERR("no menu to display\n");
    }
    else
      gxSetWindowLongPtrW( hWnd, 0, 0 );
    break;

  case MM_SETMENUHANDLE:
    gxSetWindowLongPtrW( hWnd, 0, wParam );
    break;

  case MM_GETMENUHANDLE:
    return gxGetWindowLongPtrW( hWnd, 0 );

  default:
    return gxDefWindowProcW( hWnd, message, wParam, lParam );
  }
  return 0;
}


/***********************************************************************
*           MENU_GetMenuBarHeight
*
* Compute the size of the menu bar height. Used by NC_HandleNCCalcSize().
*/
GXUINT MENU_GetMenuBarHeight( GXHWND hWnd, GXUINT menubarWidth,
               GXINT orgX, GXINT orgY )
{
  GXHDC hdc;
  GXRECT rectBar;
  LPPOPUPMENU lppop;

  TRACE("GXHWND %p, width %d, at (%d, %d).\n", hWnd, menubarWidth, orgX, orgY );

  if (!(lppop = MENU_GetMenu( gxGetMenu(hWnd) ))) return 0;

  hdc = gxGetDCEx( hWnd, 0, GXDCX_CACHE | GXDCX_WINDOW );
  gxSelectObject( hdc, get_menu_font(FALSE));
  gxSetRect(&rectBar, orgX, orgY, orgX+menubarWidth, orgY+gxGetSystemMetrics(GXSM_CYMENU));
  MENU_MenuBarCalcSize( hdc, &rectBar, lppop, hWnd );
  gxReleaseDC( hWnd, hdc );
  return lppop->Height;
}


/*******************************************************************
*         ChangeMenuA    (USER32.@)
*/

//GXBOOL ChangeMenuA( GXHMENU hMenu, GXUINT pos, GXLPCSTR data,
//                             GXUINT id, GXUINT flags )
//{
//    TRACE("menu=%p pos=%d data=%p id=%08x flags=%08x\n", hMenu, pos, data, id, flags );
//    if (flags & MF_APPEND) return gxAppendMenuA( hMenu, flags & ~MF_APPEND,
//                                                 id, data );
//    if (flags & MF_DELETE) return DeleteMenu(hMenu, pos, flags & ~MF_DELETE);
//    if (flags & MF_CHANGE) return ModifyMenuA(hMenu, pos, flags & ~MF_CHANGE,
//                                                id, data );
//    if (flags & MF_REMOVE) return gxRemoveMenu( hMenu,
//                                              flags & MF_BYPOSITION ? pos : id,
//                                              flags & ~MF_REMOVE );
//    /* Default: MF_INSERT */
//    return InsertMenuA( hMenu, pos, flags, id, data );
//}


/*******************************************************************
*         ChangeMenuW    (USER32.@)
*/
GXBOOL GXDLLAPI ChangeMenuW( GXHMENU hMenu, GXUINT pos, GXLPCWSTR data,
             GXUINT id, GXUINT flags )
{
  TRACE("menu=%p pos=%d data=%p id=%08x flags=%08x\n", hMenu, pos, data, id, flags );
  if (flags & GXMF_APPEND) return gxAppendMenuW( hMenu, flags & ~GXMF_APPEND,
    id, data );
  if (flags & GXMF_DELETE) return gxDeleteMenu(hMenu, pos, flags & ~GXMF_DELETE);
  if (flags & GXMF_CHANGE) return gxModifyMenuW(hMenu, pos, flags & ~GXMF_CHANGE,
    id, data );
  if (flags & GXMF_REMOVE) return gxRemoveMenu( hMenu,
    flags & GXMF_BYPOSITION ? pos : id,
    flags & ~GXMF_REMOVE );
  /* Default: MF_INSERT */
  return gxInsertMenuW( hMenu, pos, flags, id, data );
}


/*******************************************************************
*         gxCheckMenuItem    (USER32.@)
*/
GXDWORD GXDLLAPI gxCheckMenuItem( GXHMENU hMenu, GXUINT id, GXUINT flags )
{
  MENUITEM *item;
  GXDWORD ret;

  if (!(item = MENU_FindItem( &hMenu, &id, flags ))) return -1;
  ret = item->fState & GXMF_CHECKED;
  if (flags & GXMF_CHECKED) item->fState |= GXMF_CHECKED;
  else item->fState &= ~GXMF_CHECKED;
  return ret;
}


/**********************************************************************
*         gxEnableMenuItem    (USER32.@)
*/
GXBOOL GXDLLAPI gxEnableMenuItem( GXHMENU hMenu, GXUINT wItemID, GXUINT wFlags )
{
  GXUINT    oldflags;
  MENUITEM *item;
  POPUPMENU *menu;

  TRACE("(%p, %04x, %04x) !\n", hMenu, wItemID, wFlags);

  /* Get the Popupmenu to access the owner menu */
  if (!(menu = MENU_GetMenu(hMenu)))
    return (GXUINT)-1;

  if (!(item = MENU_FindItem( &hMenu, &wItemID, wFlags )))
    return (GXUINT)-1;

  oldflags = item->fState & (GXMF_GRAYED | GXMF_DISABLED);
  item->fState ^= (oldflags ^ wFlags) & (GXMF_GRAYED | GXMF_DISABLED);

  /* If the close item in the system menu change update the close button */
  if((item->wID == GXSC_CLOSE) && (oldflags != wFlags))
  {
    if (menu->hSysMenuOwner != 0)
    {
      GXRECT rc;
      POPUPMENU* parentMenu;

      /* Get the parent menu to access*/
      if (!(parentMenu = MENU_GetMenu(menu->hSysMenuOwner)))
        return (GXUINT)-1;

      /* Refresh the frame to reflect the change */
      gxGetWindowRect(parentMenu->hWnd, &rc);
      gxMapWindowPoints(0, parentMenu->hWnd, (GXPOINT *)&rc, 2);
      rc.bottom = 0;
      gxRedrawWindow(parentMenu->hWnd, &rc, 0, GXRDW_FRAME | GXRDW_INVALIDATE | GXRDW_NOCHILDREN);
    }
  }

  return oldflags;
}


/*******************************************************************
*         GetMenuStringA    (USER32.@)
*/
//
//GXINT GetMenuStringA(
//  GXHMENU hMenu,  /* [in] menuhandle */
//  GXUINT wItemID,  /* [in] menu item (dep. on wFlags) */
//  GXLPSTR str,  /* [out] outbuffer. If NULL, func returns entry length*/
//  GXINT nMaxSiz,  /* [in] length of buffer. if 0, func returns entry len*/
//  GXUINT wFlags  /* [in] MF_ flags */
//) {
//    MENUITEM *item;
//
//    TRACE("menu=%p item=%04x ptr=%p len=%d flags=%04x\n", hMenu, wItemID, str, nMaxSiz, wFlags );
//    if (str && nMaxSiz) str[0] = '\0';
//    if (!(item = MENU_FindItem( &hMenu, &wItemID, wFlags ))) {
//        gxSetLastError( ERROR_MENU_ITEM_NOT_FOUND);
//        return 0;
//    }
//    if (!item->text) return 0;
//    if (!str || !nMaxSiz) return GXSTRLEN(item->text);
//    if (!WideCharToMultiByte( CP_ACP, 0, item->text, -1, str, nMaxSiz, NULL, NULL ))
//        str[nMaxSiz-1] = 0;
//    TRACE("returning %s\n", debugstr_a(str));
//    return strlen(str);
//}


/*******************************************************************
*         gxGetMenuStringW    (USER32.@)
*/
GXINT GXDLLAPI gxGetMenuStringW( GXHMENU hMenu, GXUINT wItemID,
               GXLPWSTR str, GXINT nMaxSiz, GXUINT wFlags )
{
  MENUITEM *item;

  TRACE("menu=%p item=%04x ptr=%p len=%d flags=%04x\n", hMenu, wItemID, str, nMaxSiz, wFlags );
  if (str && nMaxSiz) str[0] = '\0';
  if (!(item = MENU_FindItem( &hMenu, &wItemID, wFlags ))) {
    gxSetLastError( GXERROR_MENU_ITEM_NOT_FOUND);
    return 0;
  }
  if (!str || !nMaxSiz) return item->text ? (GXINT)GXSTRLEN(item->text) : 0;
  if( !(item->text)) {
    str[0] = 0;
    return 0;
  }
  GXSTRCPYN( str, item->text, nMaxSiz );
  TRACE("returning %s\n", debugstr_w(str));
  return (GXINT)GXSTRLEN(str);
}


/**********************************************************************
*         gxHiliteMenuItem    (USER32.@)
*/
GXBOOL GXDLLAPI gxHiliteMenuItem( GXHWND hWnd, GXHMENU hMenu, GXUINT wItemID,
                GXUINT wHilite )
{
  LPPOPUPMENU menu;
  TRACE("(%p, %p, %04x, %04x);\n", hWnd, hMenu, wItemID, wHilite);
  if (!MENU_FindItem( &hMenu, &wItemID, wHilite )) return FALSE;
  if (!(menu = MENU_GetMenu(hMenu))) return FALSE;
  if (menu->FocusedItem == wItemID) return TRUE;
  MENU_HideSubPopups( hWnd, hMenu, FALSE );
  MENU_SelectItem( hWnd, hMenu, wItemID, TRUE, 0 );
  return TRUE;
}


/**********************************************************************
*         gxGetMenuState    (USER32.@)
*/
GXUINT GXDLLAPI gxGetMenuState( GXHMENU hMenu, GXUINT wItemID, GXUINT wFlags )
{
  MENUITEM *item;
  TRACE("(menu=%p, id=%04x, flags=%04x);\n", hMenu, wItemID, wFlags);
  if (!(item = MENU_FindItem( &hMenu, &wItemID, wFlags ))) return -1;
  debug_print_menuitem ("  item: ", item, "");
  if (item->fType & GXMF_POPUP)
  {
    POPUPMENU *menu = MENU_GetMenu( item->hSubMenu );
    if (!menu) return -1;
    else return (menu->nItems << 8) | ((item->fState|item->fType) & 0xff);
  }
  else
  {
    /* We used to (from way back then) mask the result to 0xff.  */
    /* I don't know why and it seems wrong as the documented */
    /* return flag MF_SEPARATOR is outside that mask.  */
    return (item->fType | item->fState);
  }
}


/**********************************************************************
*         gxGetMenuItemCount    (USER32.@)
*/
GXINT GXDLLAPI gxGetMenuItemCount( GXHMENU hMenu )
{
  LPPOPUPMENU  menu = MENU_GetMenu(hMenu);
  if (!menu) return -1;
  TRACE("(%p) returning %d\n", hMenu, menu->nItems );
  return menu->nItems;
}


/**********************************************************************
*         gxGetMenuItemID    (USER32.@)
*/
GXUINT GXDLLAPI gxGetMenuItemID( GXHMENU hMenu, GXINT nPos )
{
  MENUITEM * lpmi;

  if (!(lpmi = MENU_FindItem(&hMenu,(GXUINT*)&nPos,GXMF_BYPOSITION))) return -1;
  if (lpmi->fType & GXMF_POPUP) return -1;
  return lpmi->wID;

}


/*******************************************************************
*         gxInsertMenuW    (USER32.@)
*/
GXBOOL GXDLLAPI gxInsertMenuW( GXHMENU hMenu, GXUINT pos, GXUINT flags,
               GXUINT_PTR id, GXLPCWSTR str )
{
  MENUITEM *item;

  if (IS_STRING_ITEM(flags) && str)
    TRACE("hMenu %p, pos %d, flags %08x, id %04lx, str %s\n",
    hMenu, pos, flags, id, debugstr_w(str) );
  else TRACE("hMenu %p, pos %d, flags %08x, id %04lx, str %p (not a string)\n",
    hMenu, pos, flags, id, str );

  if (!(item = MENU_InsertItem( hMenu, pos, flags ))) return FALSE;

  if (!(MENU_SetItemData( item, flags, id, str )))
  {
    gxRemoveMenu( hMenu, pos, flags );
    return FALSE;
  }

  item->hCheckBit = item->hUnCheckBit = 0;
  return TRUE;
}


/*******************************************************************
*         InsertMenuA    (USER32.@)
*/
/*
GXBOOL InsertMenuA( GXHMENU hMenu, GXUINT pos, GXUINT flags,
GXUINT_PTR id, GXLPCSTR str )
{
GXBOOL ret = FALSE;

if (IS_STRING_ITEM(flags) && str)
{
GXINT len = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );
GXLPWSTR newstr = gxHeapAlloc( gxGetProcessHeap(), 0, len * sizeof(GXWCHAR) );
if (newstr)
{
MultiByteToWideChar( CP_ACP, 0, str, -1, newstr, len );
ret = gxInsertMenuW( hMenu, pos, flags, id, newstr );
gxHeapFree( gxGetProcessHeap(), 0, newstr );
}
return ret;
}
else return gxInsertMenuW( hMenu, pos, flags, id, (GXLPCWSTR)str );
}
*/

/*******************************************************************
*         gxAppendMenuA    (USER32.@)
*/

GXBOOL GXDLLAPI gxAppendMenuA( GXHMENU hMenu, GXUINT flags,
               GXUINT_PTR id, GXLPCSTR data )
{
  ASSERT(FALSE);
  return FALSE;
  //Artint
  //return InsertMenuA( hMenu, -1, flags | MF_BYPOSITION, id, data );
}


/*******************************************************************
*         gxAppendMenuW    (USER32.@)
*/
GXBOOL GXDLLAPI gxAppendMenuW( GXHMENU hMenu, GXUINT flags,
               GXUINT_PTR id, GXLPCWSTR data )
{
  return gxInsertMenuW( hMenu, -1, flags | GXMF_BYPOSITION, id, data );
}


/**********************************************************************
*         gxRemoveMenu    (USER32.@)
*/
GXBOOL GXDLLAPI gxRemoveMenu( GXHMENU hMenu, GXUINT nPos, GXUINT wFlags )
{
  LPPOPUPMENU  menu;
  MENUITEM *item;

  TRACE("(menu=%p pos=%04x flags=%04x)\n",hMenu, nPos, wFlags);
  if (!(item = MENU_FindItem( &hMenu, &nPos, wFlags ))) return FALSE;
  if (!(menu = MENU_GetMenu(hMenu))) return FALSE;

  /* Remove item */

  MENU_FreeItemData( item );

  if (--menu->nItems == 0)
  {
    gxHeapFree( gxGetProcessHeap(), 0, menu->items );
    menu->items = NULL;
  }
  else
  {
    while(nPos < menu->nItems)
    {
      *item = *(item+1);
      item++;
      nPos++;
    }
    menu->items = (MENUITEM*)gxHeapReAlloc( gxGetProcessHeap(), 0, menu->items,
      menu->nItems * sizeof(MENUITEM) );
  }
  return TRUE;
}


/**********************************************************************
*         DeleteMenu    (USER32.@)
*/
GXBOOL GXDLLAPI gxDeleteMenu( GXHMENU hMenu, GXUINT nPos, GXUINT wFlags )
{
  MENUITEM *item = MENU_FindItem( &hMenu, &nPos, wFlags );
  if (!item) return FALSE;
  if (item->fType & GXMF_POPUP) gxDestroyMenu( item->hSubMenu );
  /* nPos is now the position of the item */
  gxRemoveMenu( hMenu, nPos, wFlags | GXMF_BYPOSITION );
  return TRUE;
}


/*******************************************************************
*         ModifyMenuW    (USER32.@)
*/
GXBOOL GXDLLAPI gxModifyMenuW( GXHMENU hMenu, GXUINT pos, GXUINT flags,
               GXUINT_PTR id, GXLPCWSTR str )
{
  MENUITEM *item;

  if (IS_STRING_ITEM(flags))
    TRACE("%p %d %04x %04lx %s\n", hMenu, pos, flags, id, debugstr_w(str) );
  else
    TRACE("%p %d %04x %04lx %p\n", hMenu, pos, flags, id, str );

  if (!(item = MENU_FindItem( &hMenu, &pos, flags ))) return FALSE;
  MENU_GetMenu(hMenu)->Height = 0; /* force size recalculate */
  return MENU_SetItemData( item, flags, id, str );
}


/*******************************************************************
*         ModifyMenuA    (USER32.@)
*/
/*
GXBOOL ModifyMenuA( GXHMENU hMenu, GXUINT pos, GXUINT flags,
GXUINT_PTR id, GXLPCSTR str )
{
GXBOOL ret = FALSE;

if (IS_STRING_ITEM(flags) && str)
{
GXINT len = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );
GXLPWSTR newstr = gxHeapAlloc( gxGetProcessHeap(), 0, len * sizeof(GXWCHAR) );
if (newstr)
{
MultiByteToWideChar( CP_ACP, 0, str, -1, newstr, len );
ret = ModifyMenuW( hMenu, pos, flags, id, newstr );
gxHeapFree( gxGetProcessHeap(), 0, newstr );
}
return ret;
}
else return ModifyMenuW( hMenu, pos, flags, id, (GXLPCWSTR)str );
}
*/

/**********************************************************************
*         gxCreatePopupMenu    (USER32.@)
*/
GXHMENU GXDLLAPI gxCreatePopupMenu()
{
  GXHMENU hmenu;
  POPUPMENU *menu;

  if (!(hmenu = gxCreateMenu())) return 0;
  menu = MENU_GetMenu( hmenu );
  menu->wFlags |= GXMF_POPUP;
  menu->bTimeToHide = FALSE;
  return hmenu;
}


/**********************************************************************
*         GetMenuCheckMarkDimensions    (USER.417)
*         GetMenuCheckMarkDimensions    (USER32.@)
*/
GXDWORD GXDLLAPI gxGetMenuCheckMarkDimensions()
{
  return GXMAKELONG( gxGetSystemMetrics(GXSM_CXMENUCHECK), gxGetSystemMetrics(GXSM_CYMENUCHECK) );
}


/**********************************************************************
*         SetMenuItemBitmaps    (USER32.@)
*/
GXBOOL GXDLLAPI gxSetMenuItemBitmaps( GXHMENU hMenu, GXUINT nPos, GXUINT wFlags,
                  GXHBITMAP hNewUnCheck, GXHBITMAP hNewCheck)
{
  MENUITEM *item;

  if (!(item = MENU_FindItem( &hMenu, &nPos, wFlags ))) return FALSE;

  if (!hNewCheck && !hNewUnCheck)
  {
    item->fState &= ~GXMF_USECHECKBITMAPS;
  }
  else  /* Install new bitmaps */
  {
    item->hCheckBit = hNewCheck;
    item->hUnCheckBit = hNewUnCheck;
    item->fState |= GXMF_USECHECKBITMAPS;
  }
  return TRUE;
}


/**********************************************************************
*         gxCreateMenu    (USER32.@)
*/
GXHMENU GXDLLAPI gxCreateMenu()
{
  GXHMENU hMenu;
  LPPOPUPMENU menu;
  if (!(hMenu = (GXHMENU)USER_HEAP_ALLOC( sizeof(POPUPMENU) ))) return 0;
  menu = (LPPOPUPMENU) USER_HEAP_LIN_ADDR(hMenu);

  ZeroMemory(menu, sizeof(POPUPMENU));
  menu->wMagic = MENU_MAGIC;
  menu->FocusedItem = NO_SELECTED_ITEM;
  menu->bTimeToHide = FALSE;

  TRACE("return %p\n", hMenu );

  return hMenu;
}


/**********************************************************************
*         gxDestroyMenu    (USER32.@)
*/
GXBOOL GXDLLAPI gxDestroyMenu( GXHMENU hMenu )
{
  LPPOPUPMENU lppop = MENU_GetMenu(hMenu);

  TRACE("(%p)\n", hMenu);


  if (!lppop) return FALSE;

  lppop->wMagic = 0;  /* Mark it as destroyed */

  /* gxDestroyMenu should not destroy system menu popup owner */
  if ((lppop->wFlags & (GXMF_POPUP | GXMF_SYSMENU)) == GXMF_POPUP && lppop->hWnd)
  {
    gxDestroyWindow( lppop->hWnd );
    lppop->hWnd = 0;
  }

  if (lppop->items) /* recursively destroy submenus */
  {
    int i;
    MENUITEM *item = lppop->items;
    for (i = lppop->nItems; i > 0; i--, item++)
    {
      if (item->fType & GXMF_POPUP) gxDestroyMenu(item->hSubMenu);
      MENU_FreeItemData( item );
    }
    gxHeapFree( gxGetProcessHeap(), 0, lppop->items );
  }
  USER_HEAP_FREE( hMenu );
  return TRUE;
}


/**********************************************************************
*         gxGetSystemMenu    (USER32.@)
*/
GXHMENU GXDLLAPI gxGetSystemMenu( GXHWND hWnd, GXBOOL bRevert )
{
  GXWnd *GXWNDPtr = WIN_GetPtr( hWnd );
  GXHMENU retvalue = 0;

  if (GXWNDPtr == GXWND_DESKTOP) return 0;
  if (GXWNDPtr == GXWND_OTHER_PROCESS)
  {
    if (gxIsWindow( hWnd )) FIXME( "not supported on other process window %p\n", hWnd );
  }
  else if (GXWNDPtr)
  {
    if (GXWNDPtr->hSysMenu && bRevert)
    {
      gxDestroyMenu(GXWNDPtr->hSysMenu);
      GXWNDPtr->hSysMenu = 0;
    }

    if(!GXWNDPtr->hSysMenu && (GXWNDPtr->m_uStyle & GXWS_SYSMENU) )
      GXWNDPtr->hSysMenu = MENU_GetSysMenu( hWnd, 0 );

    if( GXWNDPtr->hSysMenu )
    {
      POPUPMENU *menu;
      retvalue = gxGetSubMenu(GXWNDPtr->hSysMenu, 0);

      /* Store the dummy sysmenu handle to facilitate the refresh */
      /* of the close button if the SC_CLOSE item change */
      menu = MENU_GetMenu(retvalue);
      if ( menu )
        menu->hSysMenuOwner = GXWNDPtr->hSysMenu;
    }
    WIN_ReleasePtr( GXWNDPtr );
  }
  return bRevert ? 0 : retvalue;
}


/*******************************************************************
*         SetSystemMenu    (USER32.@)
*/
GXBOOL gxSetSystemMenu( GXHWND hWnd, GXHMENU hMenu )
{
  GXWnd *GXWNDPtr = WIN_GetPtr( hWnd );

  if (GXWNDPtr && GXWNDPtr != GXWND_OTHER_PROCESS && GXWNDPtr != GXWND_DESKTOP)
  {
    if (GXWNDPtr->hSysMenu) gxDestroyMenu( GXWNDPtr->hSysMenu );
    GXWNDPtr->hSysMenu = MENU_GetSysMenu( hWnd, hMenu );
    WIN_ReleasePtr( GXWNDPtr );
    return TRUE;
  }
  return FALSE;
}


/**********************************************************************
*         gxGetMenu    (USER32.@)
*/
GXHMENU GXDLLAPI gxGetMenu( GXHWND hWnd )
{
  GXHMENU retvalue = (GXHMENU)gxGetWindowLongPtrW( hWnd, GXGWLP_ID );
  TRACE("for %p returning %p\n", hWnd, retvalue);
  return retvalue;
}

/**********************************************************************
*         GetMenuBarInfo    (USER32.@)
*/
GXBOOL GXDLLAPI gxGetMenuBarInfo( GXHWND hWnd, GXLONG idObject, GXLONG idItem, GXLPMENUBARINFO pmbi )
{
  FIXME( "(%p,0x%08x,0x%08x,%p)\n", hWnd, idObject, idItem, pmbi );
  return FALSE;
}

/**********************************************************************
*         MENU_SetMenu
*
* Helper for gxSetMenu. Also called by WIN_CreateWindowEx to avoid the
* gxSetWindowPos call that would result if gxSetMenu were called directly.
*/
GXBOOL MENU_SetMenu( GXHWND hWnd, GXHMENU hMenu )
{
  TRACE("(%p, %p);\n", hWnd, hMenu);

  if (hMenu && !gxIsMenu(hMenu))
  {
    WARN("hMenu %p is not a menu handle\n", hMenu);
    return FALSE;
  }
  if (!WIN_ALLOWED_MENU(gxGetWindowLongW( hWnd, GXGWL_STYLE )))
    return FALSE;

  hWnd = WIN_GetFullHandle( hWnd );
  if (gxGetCapture() == hWnd) MENU_SetCapture(0);  /* release the capture */

  if (hMenu != 0)
  {
    LPPOPUPMENU lpmenu;

    if (!(lpmenu = MENU_GetMenu(hMenu))) return FALSE;

    lpmenu->hWnd = hWnd;
    lpmenu->Height = 0;  /* Make sure we recalculate the size */
  }
  gxSetWindowLongPtrW( hWnd, GXGWLP_ID, (GXLONG_PTR)hMenu );
  return TRUE;
}


/**********************************************************************
*         gxSetMenu    (USER32.@)
*/
GXBOOL gxSetMenu( GXHWND hWnd, GXHMENU hMenu )
{   
  if(!MENU_SetMenu(hWnd, hMenu))
    return FALSE;

  gxSetWindowPos( hWnd, 0, 0, 0, 0, 0, GXSWP_NOSIZE | GXSWP_NOMOVE |
    GXSWP_NOACTIVATE | GXSWP_NOZORDER | GXSWP_FRAMECHANGED );
  return TRUE;
}


/**********************************************************************
*         gxGetSubMenu    (USER32.@)
*/
GXHMENU GXDLLAPI gxGetSubMenu( GXHMENU hMenu, GXINT nPos )
{
  MENUITEM * lpmi;

  if (!(lpmi = MENU_FindItem(&hMenu,(GXUINT*)&nPos,GXMF_BYPOSITION))) return 0;
  if (!(lpmi->fType & GXMF_POPUP)) return 0;
  return lpmi->hSubMenu;
}


/**********************************************************************
*         DrawMenuBar    (USER32.@)
*/
GXBOOL GXDLLAPI gxDrawMenuBar( GXHWND hWnd )
{
  LPPOPUPMENU lppop;
  GXHMENU hMenu = gxGetMenu(hWnd);

  if (!WIN_ALLOWED_MENU(gxGetWindowLongW( hWnd, GXGWL_STYLE )))
    return FALSE;
  if (!hMenu || !(lppop = MENU_GetMenu( hMenu ))) return FALSE;

  lppop->Height = 0; /* Make sure we call MENU_MenuBarCalcSize */
  lppop->hWndOwner = hWnd;
  gxSetWindowPos( hWnd, 0, 0, 0, 0, 0, GXSWP_NOSIZE | GXSWP_NOMOVE |
    GXSWP_NOACTIVATE | GXSWP_NOZORDER | GXSWP_FRAMECHANGED );
  return TRUE;
}

/***********************************************************************
*           DrawMenuBarTemp   (USER32.@)
*
* UNDOCUMENTED !!
*
* called by W98SE desk.cpl Control Panel Applet
*
* Not 100% sure about the param names, but close.
*/
GXDWORD DrawMenuBarTemp(GXHWND hWnd, GXHDC hDC, LPGXRECT lprect, GXHMENU hMenu, GXHFONT hFont)
{
  LPPOPUPMENU lppop;
  GXUINT i,retvalue;
  GXHFONT hfontOld = 0;
  GXBOOL flat_menu = FALSE;

  gxSystemParametersInfoW (SPI_GETFLATMENU, 0, &flat_menu, 0);

  if (!hMenu)
    hMenu = gxGetMenu(hWnd);

  if (!hFont)
    hFont = get_menu_font(FALSE);

  lppop = MENU_GetMenu( hMenu );
  if (lppop == NULL || lprect == NULL)
  {
    retvalue = gxGetSystemMetrics(GXSM_CYMENU);
    goto END;
  }

  TRACE("(%p, %p, %p, %p, %p)\n", hWnd, hDC, lprect, hMenu, hFont);

  hfontOld = (GXHFONT)gxSelectObject( hDC, hFont);

  if (lppop->Height == 0)
    MENU_MenuBarCalcSize(hDC, lprect, lppop, hWnd);

  lprect->bottom = lprect->top + lppop->Height;

  gxFillRect(hDC, lprect, gxGetSysColorBrush(flat_menu ? GXCOLOR_MENUBAR : GXCOLOR_MENU) );

  gxSelectObject( hDC, SYSCOLOR_GetPen(GXCOLOR_3DFACE));
  gxMoveToEx( hDC, lprect->left, lprect->bottom, NULL );
  gxLineTo( hDC, lprect->right, lprect->bottom );

  if (lppop->nItems == 0)
  {
    retvalue = gxGetSystemMetrics(GXSM_CYMENU);
    goto END;
  }

  for (i = 0; i < lppop->nItems; i++)
  {
    MENU_DrawMenuItem( hWnd, hMenu, hWnd,
      hDC, &lppop->items[i], lppop->Height, TRUE, GXODA_DRAWENTIRE );
  }
  retvalue = lppop->Height;

END:
  if (hfontOld) gxSelectObject (hDC, hfontOld);
  return retvalue;
}

/***********************************************************************
*           EndMenu   (USER.187)
*           EndMenu   (USER32.@)
*/
void gxEndMenu()
{
  /* if we are in the menu code, and it is active */
  if (!fEndMenu && top_popup)
  {
    /* terminate the menu handling code */
    fEndMenu = TRUE;

    /* needs to be posted to wakeup the internal menu handler */
    /* which will now terminate the menu, in the event that */
    /* the main window was minimized, or lost focus, so we */
    /* don't end up with an orphaned menu */
    gxPostMessageW( top_popup, GXWM_CANCELMODE, 0, 0);
  }
}


/***********************************************************************
*           LookupMenuHandle   (USER.217)
*/
/*
HMENU16 LookupMenuHandle16( HMENU16 hmenu, INT16 id )
{
GXHMENU hmenu32 = HMENU_32(hmenu);
GXUINT id32 = id;
if (!MENU_FindItem( &hmenu32, &id32, MF_BYCOMMAND )) return 0;
else return HMENU_16(hmenu32);
}
*/


/**********************************************************************
*      LoadMenu    (USER.150)
*/
/*
HMENU16 LoadMenu16( HINSTANCE16 instance, GXLPCSTR name )
{
HRSRC16 hRsrc;
HGLOBAL16 handle;
HMENU16 hMenu;

if (GXHIWORD(name) && name[0] == '#') name = ULongToPtr(atoi( name + 1 ));
if (!name) return 0;

instance = GetExePtr( instance );
if (!(hRsrc = FindResource16( instance, name, (GXLPSTR)RT_MENU ))) return 0;
if (!(handle = LoadResource16( instance, hRsrc ))) return 0;
hMenu = LoadMenuIndirect16(LockResource16(handle));
FreeResource16( handle );
return hMenu;
}
*/


/*****************************************************************
*        LoadMenuA   (USER32.@)
*/
/*
GXHMENU LoadMenuA( HINSTANCE instance, GXLPCSTR name )
{
HRSRC hrsrc = FindResourceA( instance, name, (GXLPSTR)RT_MENU );
if (!hrsrc) return 0;
return LoadMenuIndirectA( (LPCVOID)LoadResource( instance, hrsrc ));
}
*/

/*****************************************************************
*        LoadMenuW   (USER32.@)
*/
GXHMENU GXDLLAPI gxLoadMenuW( GXHINSTANCE instance, GXLPCWSTR name )
{
  GXHRSRC hrsrc = gxFindResourceW( instance, name, (GXLPWSTR)GXRT_MENU );
  if (!hrsrc) return 0;
  return gxLoadMenuIndirectW( (const GXLPVOID)gxLoadResource( instance, hrsrc ));
}


/**********************************************************************
*      LoadMenuIndirect    (USER.220)
*/
/*
HMENU16 LoadMenuIndirect16( LPCVOID template )
{
GXHMENU hMenu;
GXWORD version, offset;
GXLPCSTR p = (GXLPCSTR)template;

TRACE("(%p)\n", template );
version = GET_WORD(p);
p += sizeof(GXWORD);
if (version)
{
WARN("version must be 0 for Win16\n" );
return 0;
}
offset = GET_WORD(p);
p += sizeof(GXWORD) + offset;
if (!(hMenu = gxCreateMenu())) return 0;
if (!MENU_ParseResource( p, hMenu, FALSE ))
{
gxDestroyMenu( hMenu );
return 0;
}
return HMENU_16(hMenu);
}
*/


/**********************************************************************
*      LoadMenuIndirectW    (USER32.@)
*/
GXHMENU GXDLLAPI gxLoadMenuIndirectW(GXLPCVOID lpTemplate)
{
  GXHMENU hMenu;
  GXWORD version, offset;
  GXLPCSTR p = (GXLPCSTR)lpTemplate;

  version = GET_WORD(p);
  p += sizeof(GXWORD);
  TRACE("%p, ver %d\n", lpTemplate, version );
  switch (version)
  {
  case 0: /* standard format is version of 0 */
    offset = GET_WORD(p);
    p += sizeof(GXWORD) + offset;
    if (!(hMenu = gxCreateMenu())) return 0;
    if (!MENU_ParseResource( p, hMenu, TRUE ))
    {
      gxDestroyMenu( hMenu );
      return 0;
    }
    return hMenu;
  case 1: /* extended format is version of 1 */
    offset = GET_WORD(p);
    p += sizeof(GXWORD) + offset;
    if (!(hMenu = gxCreateMenu())) return 0;
    if (!MENUEX_ParseResource( p, hMenu))
    {
      gxDestroyMenu( hMenu );
      return 0;
    }
    return hMenu;
  default:
    ERR("version %d not supported.\n", version);
    return 0;
  }
}


/**********************************************************************
*      LoadMenuIndirectA    (USER32.@)
*/
/*
GXHMENU LoadMenuIndirectA( LPCVOID template )
{
return LoadMenuIndirectW( template );
}
*/

/**********************************************************************
*    gxIsMenu    (USER32.@)
*/
GXBOOL GXDLLAPI gxIsMenu(GXHMENU hmenu)
{
  LPPOPUPMENU menu = MENU_GetMenu(hmenu);

  if (!menu)
  {
    gxSetLastError(GXERROR_INVALID_MENU_HANDLE);
    return FALSE;
  }
  return TRUE;
}

/**********************************************************************
*    GetMenuItemInfo_common
*/

static GXBOOL GetMenuItemInfo_common ( GXHMENU hmenu, GXUINT item, GXBOOL bypos,
                    LPGXMENUITEMINFOW lpmii, GXBOOL unicode)
{
  MENUITEM *menu = MENU_FindItem (&hmenu, &item, bypos ? GXMF_BYPOSITION : 0);

  debug_print_menuitem("GetMenuItemInfo_common: ", menu, "");

  if (!menu)
    return FALSE;

  if( lpmii->fMask & GXMIIM_TYPE) {
    if( lpmii->fMask & ( GXMIIM_STRING | GXMIIM_FTYPE | GXMIIM_BITMAP)) {
      WARN("invalid combination of fMask bits used\n");
      /* this does not happen on Win9x/ME */
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
    }
    lpmii->fType = menu->fType & MENUITEMINFO_TYPE_MASK;
    if( menu->hbmpItem) lpmii->fType |= GXMFT_BITMAP;
    lpmii->hbmpItem = menu->hbmpItem; /* not on Win9x/ME */
    if( lpmii->fType & GXMFT_BITMAP) {
      lpmii->dwTypeData = (GXLPWSTR) menu->hbmpItem;
      lpmii->cch = 0;
    } else if( lpmii->fType & (GXMFT_OWNERDRAW | GXMFT_SEPARATOR)) {
      /* this does not happen on Win9x/ME */
      lpmii->dwTypeData = 0;
      lpmii->cch = 0;
    }
  }

  /* copy the text string */
  if ((lpmii->fMask & (GXMIIM_TYPE|GXMIIM_STRING))) {
    if( !menu->text ) {
      if(lpmii->dwTypeData && lpmii->cch) {
        lpmii->cch = 0;
        if( unicode)
          *((GXWCHAR *)lpmii->dwTypeData) = 0;
        else
          *((GXCHAR *)lpmii->dwTypeData) = 0;
      }
    } else {
      int len;
      if (unicode)
      {
        len = (int)GXSTRLEN(menu->text);
        if(lpmii->dwTypeData && lpmii->cch)
          GXSTRCPYN(lpmii->dwTypeData, menu->text, lpmii->cch);
      }
      else
      {
        len = gxWideCharToMultiByte( GXCP_ACP, 0, menu->text, -1, NULL,
          0, NULL, NULL ) - 1;
        if(lpmii->dwTypeData && lpmii->cch)
          if (!gxWideCharToMultiByte( GXCP_ACP, 0, menu->text, -1,
            (GXLPSTR)lpmii->dwTypeData, lpmii->cch, NULL, NULL ))
            ((GXLPSTR)lpmii->dwTypeData)[lpmii->cch - 1] = 0;
      }
      /* if we've copied a substring we return its length */
      if(lpmii->dwTypeData && lpmii->cch)
        if (lpmii->cch <= len + 1)
          lpmii->cch--;
        else
          lpmii->cch = len;
      else {
        /* return length of string */
        /* not on Win9x/ME if fType & GXMFT_BITMAP */
        lpmii->cch = len;
      }
    }
  }

  if (lpmii->fMask & GXMIIM_FTYPE)
    lpmii->fType = menu->fType & MENUITEMINFO_TYPE_MASK;

  if (lpmii->fMask & GXMIIM_BITMAP)
    lpmii->hbmpItem = menu->hbmpItem;

  if (lpmii->fMask & GXMIIM_STATE)
    lpmii->fState = menu->fState & MENUITEMINFO_STATE_MASK;

  if (lpmii->fMask & GXMIIM_ID)
    lpmii->wID = menu->wID;

  if (lpmii->fMask & GXMIIM_SUBMENU)
    lpmii->hSubMenu = menu->hSubMenu;
  else {
    /* hSubMenu is always cleared 
    * (not on Win9x/ME ) */
    lpmii->hSubMenu = 0;
  }

  if (lpmii->fMask & GXMIIM_CHECKMARKS) {
    lpmii->hbmpChecked = menu->hCheckBit;
    lpmii->hbmpUnchecked = menu->hUnCheckBit;
  }
  if (lpmii->fMask & GXMIIM_DATA)
    lpmii->dwItemData = menu->dwItemData;

  return TRUE;
}

/**********************************************************************
*    GetMenuItemInfoA    (USER32.@)
*/
GXBOOL gxGetMenuItemInfoA( GXHMENU hmenu, GXUINT item, GXBOOL bypos,
              GXLPMENUITEMINFOA lpmii)
{
  GXBOOL ret;
  GXMENUITEMINFOA mii;
  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  mii.cbSize = sizeof( mii);
  ret = GetMenuItemInfo_common (hmenu, item, bypos,
    (GXLPMENUITEMINFOW)&mii, FALSE);
  mii.cbSize = lpmii->cbSize;
  memcpy( lpmii, &mii, mii.cbSize);
  return ret;
}

/**********************************************************************
*    GetMenuItemInfoW    (USER32.@)
*/
GXBOOL gxGetMenuItemInfoW( GXHMENU hmenu, GXUINT item, GXBOOL bypos,
              GXLPMENUITEMINFOW lpmii)
{
  GXBOOL ret;
  GXMENUITEMINFOW mii;
  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  mii.cbSize = sizeof( mii);
  ret = GetMenuItemInfo_common (hmenu, item, bypos, &mii, TRUE);
  mii.cbSize = lpmii->cbSize;
  memcpy( lpmii, &mii, mii.cbSize);
  return ret;
}


/* set a menu item text from a ASCII or Unicode string */
static inline void set_menu_item_text( MENUITEM *menu, GXLPCWSTR text, GXBOOL unicode )
{
  if (!text)
    menu->text = NULL;
  else if (unicode)
  {
    if ((menu->text = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, (GXSTRLEN(text)+1) * sizeof(GXWCHAR) )))
      GXSTRCPY( menu->text, text );
  }
  else
  {
    GXLPCSTR str = (GXLPCSTR)text;
    int len = gxMultiByteToWideChar( GXCP_ACP, 0, str, -1, NULL, 0 );
    if ((menu->text = (GXLPWSTR)gxHeapAlloc( gxGetProcessHeap(), 0, len * sizeof(GXWCHAR) )))
      gxMultiByteToWideChar( GXCP_ACP, 0, str, -1, menu->text, len );
  }
}


/**********************************************************************
*    SetMenuItemInfo_common
*/

static GXBOOL SetMenuItemInfo_common(MENUITEM * menu,
                   const GXMENUITEMINFOW *lpmii,
                   GXBOOL unicode)
{
  if (!menu) return FALSE;

  debug_print_menuitem("SetMenuItemInfo_common from: ", menu, "");

  if (lpmii->fMask & GXMIIM_TYPE ) {
    if( lpmii->fMask & ( GXMIIM_STRING | GXMIIM_FTYPE | GXMIIM_BITMAP)) {
      WARN("invalid combination of fMask bits used\n");
      /* this does not happen on Win9x/ME */
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
    }

    /* Remove the old type bits and replace them with the new ones */
    menu->fType &= ~MENUITEMINFO_TYPE_MASK;
    menu->fType |= lpmii->fType & MENUITEMINFO_TYPE_MASK;

    if (IS_STRING_ITEM(menu->fType)) {
      gxHeapFree(gxGetProcessHeap(), 0, menu->text);
      set_menu_item_text( menu, lpmii->dwTypeData, unicode );
    } else if( (menu->fType) & GXMFT_BITMAP)
      menu->hbmpItem = HBITMAP_32(GXLOWORD(lpmii->dwTypeData));
  }

  if (lpmii->fMask & GXMIIM_FTYPE ) {
    if(( lpmii->fType & GXMFT_BITMAP)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
    }
    menu->fType &= ~MENUITEMINFO_TYPE_MASK;
    menu->fType |= lpmii->fType & MENUITEMINFO_TYPE_MASK;
  }
  if (lpmii->fMask & GXMIIM_STRING ) {
    /* free the string when used */
    gxHeapFree(gxGetProcessHeap(), 0, menu->text);
    set_menu_item_text( menu, lpmii->dwTypeData, unicode );
  }

  if (lpmii->fMask & GXMIIM_STATE)
  {
    /* Other menu items having MFS_DEFAULT are not converted
    to normal items */
    menu->fState = lpmii->fState & MENUITEMINFO_STATE_MASK;
  }

  if (lpmii->fMask & GXMIIM_ID)
    menu->wID = lpmii->wID;

  if (lpmii->fMask & GXMIIM_SUBMENU) {
    menu->hSubMenu = lpmii->hSubMenu;
    if (menu->hSubMenu) {
      POPUPMENU *subMenu = MENU_GetMenu(menu->hSubMenu);
      if (subMenu) {
        subMenu->wFlags |= GXMF_POPUP;
        menu->fType |= GXMF_POPUP;
      }
      else {
        gxSetLastError( GXERROR_INVALID_PARAMETER);
        return FALSE;
      }
    }
    else
      menu->fType &= ~GXMF_POPUP;
  }

  if (lpmii->fMask & GXMIIM_CHECKMARKS)
  {
    menu->hCheckBit = lpmii->hbmpChecked;
    menu->hUnCheckBit = lpmii->hbmpUnchecked;
  }
  if (lpmii->fMask & GXMIIM_DATA)
    menu->dwItemData = lpmii->dwItemData;

  if (lpmii->fMask & GXMIIM_BITMAP)
    menu->hbmpItem = lpmii->hbmpItem;

  if( !menu->text && !(menu->fType & GXMFT_OWNERDRAW) && !menu->hbmpItem)
    menu->fType |= GXMFT_SEPARATOR;

  debug_print_menuitem("SetMenuItemInfo_common to : ", menu, "");
  return TRUE;
}

/**********************************************************************
*    SetMenuItemInfoA    (USER32.@)
*/
GXBOOL gxSetMenuItemInfoA(GXHMENU hmenu, GXUINT item, GXBOOL bypos,
              const GXMENUITEMINFOA *lpmii)
{
  GXMENUITEMINFOA mii;

  TRACE("hmenu %p, item %u, by pos %d, info %p\n", hmenu, item, bypos, lpmii);

  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  if( lpmii->cbSize != sizeof( mii)) {
    mii.cbSize = sizeof( mii);
    mii.hbmpItem = NULL;
  }
  return SetMenuItemInfo_common(MENU_FindItem(&hmenu, &item, bypos? GXMF_BYPOSITION : 0),
    (const GXMENUITEMINFOW *)&mii, FALSE);
}

/**********************************************************************
*    SetMenuItemInfoW    (USER32.@)
*/
GXBOOL gxSetMenuItemInfoW(GXHMENU hmenu, GXUINT item, GXBOOL bypos,
              const GXMENUITEMINFOW *lpmii)
{
  GXMENUITEMINFOW mii;

  TRACE("hmenu %p, item %u, by pos %d, info %p\n", hmenu, item, bypos, lpmii);

  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  if( lpmii->cbSize != sizeof( mii)) {
    mii.cbSize = sizeof( mii);
    mii.hbmpItem = NULL;
  }
  return SetMenuItemInfo_common(MENU_FindItem(&hmenu,
    &item, bypos? GXMF_BYPOSITION : 0), &mii, TRUE);
}

/**********************************************************************
*    SetMenuDefaultItem    (USER32.@)
*
*/
GXBOOL GXDLLAPI gxSetMenuDefaultItem(GXHMENU hmenu, GXUINT uItem, GXUINT bypos)
{
  GXUINT i;
  POPUPMENU *menu;
  MENUITEM *item;

  TRACE("(%p,%d,%d)\n", hmenu, uItem, bypos);

  if (!(menu = MENU_GetMenu(hmenu))) return FALSE;

  /* reset all default-item flags */
  item = menu->items;
  for (i = 0; i < menu->nItems; i++, item++)
  {
    item->fState &= ~GXMFS_DEFAULT;
  }

  /* no default item */
  if ( -1 == uItem)
  {
    return TRUE;
  }

  item = menu->items;
  if ( bypos )
  {
    if ( uItem >= menu->nItems ) return FALSE;
    item[uItem].fState |= GXMFS_DEFAULT;
    return TRUE;
  }
  else
  {
    for (i = 0; i < menu->nItems; i++, item++)
    {
      if (item->wID == uItem)
      {
        item->fState |= GXMFS_DEFAULT;
        return TRUE;
      }
    }

  }
  return FALSE;
}

/**********************************************************************
*    GetMenuDefaultItem    (USER32.@)
*/
GXUINT gxGetMenuDefaultItem(GXHMENU hmenu, GXUINT bypos, GXUINT flags)
{
  POPUPMENU *menu;
  MENUITEM * item;
  GXUINT i = 0;

  TRACE("(%p,%d,%d)\n", hmenu, bypos, flags);

  if (!(menu = MENU_GetMenu(hmenu))) return -1;

  /* find default item */
  item = menu->items;

  /* empty menu */
  if (! item) return -1;

  while ( !( item->fState & GXMFS_DEFAULT ) )
  {
    i++; item++;
    if  (i >= menu->nItems ) return -1;
  }

  /* default: don't return disabled items */
  if ( (!(GXGMDI_USEDISABLED & flags)) && (item->fState & GXMFS_DISABLED )) return -1;

  /* search rekursiv when needed */
  if ( (item->fType & GXMF_POPUP) &&  (flags & GXGMDI_GOINTOPOPUPS) )
  {
    GXUINT ret;
    ret = gxGetMenuDefaultItem( item->hSubMenu, bypos, flags );
    if ( -1 != ret ) return ret;

    /* when item not found in submenu, return the popup item */
  }
  return ( bypos ) ? i : item->wID;

}


/**********************************************************************
*    InsertMenuItemA    (USER32.@)
*/
GXBOOL gxInsertMenuItemA(GXHMENU hMenu, GXUINT uItem, GXBOOL bypos,
             const GXMENUITEMINFOA *lpmii)
{
  MENUITEM *item;
  GXMENUITEMINFOA mii;

  TRACE("hmenu %p, item %04x, by pos %d, info %p\n", hMenu, uItem, bypos, lpmii);

  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  if( lpmii->cbSize != sizeof( mii)) {
    mii.cbSize = sizeof( mii);
    mii.hbmpItem = NULL;
  }

  item = MENU_InsertItem(hMenu, uItem, bypos ? GXMF_BYPOSITION : 0 );
  return SetMenuItemInfo_common(item, (const GXMENUITEMINFOW *)&mii, FALSE);
}


/**********************************************************************
*    gxInsertMenuItemW    (USER32.@)
*/
GXBOOL GXDLLAPI gxInsertMenuItemW(GXHMENU hMenu, GXUINT uItem, GXBOOL bypos,
                 const GXMENUITEMINFOW *lpmii)
{
  MENUITEM *item;
  GXMENUITEMINFOW mii;

  TRACE("hmenu %p, item %04x, by pos %d, info %p\n", hMenu, uItem, bypos, lpmii);

  if( lpmii->cbSize != sizeof( mii) &&
    lpmii->cbSize != sizeof( mii) - sizeof ( mii.hbmpItem)) {
      gxSetLastError( GXERROR_INVALID_PARAMETER);
      return FALSE;
  }
  memcpy( &mii, lpmii, lpmii->cbSize);
  if( lpmii->cbSize != sizeof( mii)) {
    mii.cbSize = sizeof( mii);
    mii.hbmpItem = NULL;
  }

  item = MENU_InsertItem(hMenu, uItem, bypos ? GXMF_BYPOSITION : 0 );
  return SetMenuItemInfo_common(item, &mii, TRUE);
}

/**********************************************************************
*    CheckMenuRadioItem    (USER32.@)
*/

GXBOOL gxCheckMenuRadioItem(GXHMENU hMenu,
              GXUINT first, GXUINT last, GXUINT check,
              GXUINT bypos)
{
  GXBOOL done = FALSE;
  GXUINT i;
  MENUITEM *mi_first = NULL, *mi_check;
  GXHMENU m_first, m_check;

  for (i = first; i <= last; i++)
  {
    GXUINT pos = i;

    if (!mi_first)
    {
      m_first = hMenu;
      mi_first = MENU_FindItem(&m_first, &pos, bypos);
      if (!mi_first) continue;
      mi_check = mi_first;
      m_check = m_first;
    }
    else
    {
      m_check = hMenu;
      mi_check = MENU_FindItem(&m_check, &pos, bypos);
      if (!mi_check) continue;
    }

    if (m_first != m_check) continue;
    if (mi_check->fType == GXMFT_SEPARATOR) continue;

    if (i == check)
    {
      mi_check->fType |= GXMFT_RADIOCHECK;
      mi_check->fState |= GXMFS_CHECKED;
      done = TRUE;
    }
    else
    {
      /* MSDN is wrong, Windows does not remove GXMFT_RADIOCHECK */
      mi_check->fState &= ~GXMFS_CHECKED;
    }
  }

  return done;
}


/**********************************************************************
*    GetMenuItemRect    (USER32.@)
*
*      ATTENTION: Here, the returned values in rect are the screen
*                 coordinates of the item just like if the menu was
*                 always on the upper left side of the application.
*
*/
GXBOOL gxGetMenuItemRect (GXHWND hWnd, GXHMENU hMenu, GXUINT uItem,
              LPGXRECT rect)
{
  POPUPMENU *itemMenu;
  MENUITEM *item;
  GXHWND referenceGXHWND;

  TRACE("(%p,%p,%d,%p)\n", hWnd, hMenu, uItem, rect);

  item = MENU_FindItem (&hMenu, &uItem, GXMF_BYPOSITION);
  referenceGXHWND = hWnd;

  if(!hWnd)
  {
    itemMenu = MENU_GetMenu(hMenu);
    if (itemMenu == NULL)
      return FALSE;

    if(itemMenu->hWnd == 0)
      return FALSE;
    referenceGXHWND = itemMenu->hWnd;
  }

  if ((rect == NULL) || (item == NULL))
    return FALSE;

  *rect = item->rect;

  gxMapWindowPoints(referenceGXHWND, 0, (GXLPPOINT)rect, 2);

  return TRUE;
}


/**********************************************************************
*    SetMenuInfo    (USER32.@)
*
* FIXME
*  MIM_APPLYTOSUBMENUS
*  actually use the items to draw the menu
*/
GXBOOL gxSetMenuInfo (GXHMENU hMenu, GXLPCMENUINFO lpmi)
{
  POPUPMENU *menu;

  TRACE("(%p %p)\n", hMenu, lpmi);

  if (lpmi && (lpmi->cbSize==sizeof(GXMENUINFO)) && (menu = MENU_GetMenu(hMenu)))
  {

    if (lpmi->fMask & GXMIM_BACKGROUND)
      menu->hbrBack = lpmi->hbrBack;

    if (lpmi->fMask & GXMIM_HELPID)
      menu->dwContextHelpID = lpmi->dwContextHelpID;

    if (lpmi->fMask & GXMIM_MAXHEIGHT)
      menu->cyMax = lpmi->cyMax;

    if (lpmi->fMask & GXMIM_MENUDATA)
      menu->dwMenuData = lpmi->dwMenuData;

    if (lpmi->fMask & GXMIM_STYLE)
    {
      menu->dwStyle = lpmi->dwStyle;
      if (menu->dwStyle & GXMNS_AUTODISMISS) FIXME("MNS_AUTODISMISS unimplemented\n");
      if (menu->dwStyle & GXMNS_DRAGDROP) FIXME("MNS_DRAGDROP unimplemented\n");
      if (menu->dwStyle & GXMNS_MODELESS) FIXME("MNS_MODELESS unimplemented\n");
      if (menu->dwStyle & GXMNS_NOTIFYBYPOS) FIXME("MNS_NOTIFYBYPOS partially implemented\n");
    }

    return TRUE;
  }
  return FALSE;
}

/**********************************************************************
*    GetMenuInfo    (USER32.@)
*
*  NOTES
*  win98/NT5.0
*
*/
GXBOOL gxGetMenuInfo (GXHMENU hMenu, GXLPMENUINFO lpmi)
{   POPUPMENU *menu;

TRACE("(%p %p)\n", hMenu, lpmi);

if (lpmi && (menu = MENU_GetMenu(hMenu)))
{

  if (lpmi->fMask & GXMIM_BACKGROUND)
    lpmi->hbrBack = (GXHBRUSH)menu->hbrBack;

  if (lpmi->fMask & GXMIM_HELPID)
    lpmi->dwContextHelpID = menu->dwContextHelpID;

  if (lpmi->fMask & GXMIM_MAXHEIGHT)
    lpmi->cyMax = menu->cyMax;

  if (lpmi->fMask & GXMIM_MENUDATA)
    lpmi->dwMenuData = menu->dwMenuData;

  if (lpmi->fMask & GXMIM_STYLE)
    lpmi->dwStyle = menu->dwStyle;

  return TRUE;
}
return FALSE;
}


/**********************************************************************
*         SetMenuContextHelpId    (USER32.@)
*/
GXBOOL gxSetMenuContextHelpId( GXHMENU hMenu, GXDWORD dwContextHelpID)
{
  LPPOPUPMENU menu;

  TRACE("(%p 0x%08x)\n", hMenu, dwContextHelpID);

  if ((menu = MENU_GetMenu(hMenu)))
  {
    menu->dwContextHelpID = dwContextHelpID;
    return TRUE;
  }
  return FALSE;
}


/**********************************************************************
*         GetMenuContextHelpId    (USER32.@)
*/
GXDWORD gxGetMenuContextHelpId( GXHMENU hMenu )
{
  LPPOPUPMENU menu;

  TRACE("(%p)\n", hMenu);

  if ((menu = MENU_GetMenu(hMenu)))
  {
    return menu->dwContextHelpID;
  }
  return 0;
}

/**********************************************************************
*         MenuItemFromPoint    (USER32.@)
*/
GXINT gxMenuItemFromPoint(GXHWND hWnd, GXHMENU hMenu, GXPOINT ptScreen)
{
  POPUPMENU *menu = MENU_GetMenu(hMenu);
  GXUINT pos;

  /*FIXME: Do we have to handle hWnd here? */
  if (!menu) return -1;
  if (!MENU_FindItemByCoords(menu, ptScreen, &pos)) return -1;
  return pos;
}


/**********************************************************************
*           translate_accelerator
*/
static GXBOOL translate_accelerator( GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam,
                  GXBYTE fVirt, GXWORD key, GXWORD cmd )
{
  GXINT mask = 0;
  GXUINT mesg = 0;

  if (wParam != key) return FALSE;

  if (gxGetKeyState(GXVK_CONTROL) & 0x8000) mask |= GXFCONTROL;
  if (gxGetKeyState(GXVK_MENU) & 0x8000) mask |= GXFALT;
  if (gxGetKeyState(GXVK_SHIFT) & 0x8000) mask |= GXFSHIFT;

  if (message == GXWM_CHAR || message == GXWM_SYSCHAR)
  {
    if ( !(fVirt & GXFVIRTKEY) && (mask & GXFALT) == (fVirt & GXFALT) )
    {
      // Artint
      //TRACE_(accel)("found accel for GXWM_CHAR: ('%c')\n", GXLOWORD(wParam) & 0xff);
      goto found;
    }
  }
  else
  {
    if(fVirt & GXFVIRTKEY)
    {
      // Artint
      //TRACE_(accel)("found accel for virt_key %04lx (scan %04x)\n",
      //              wParam, 0xff & GXHIWORD(lParam));

      if(mask == (fVirt & (GXFSHIFT | GXFCONTROL | GXFALT))) goto found;
      // Artint
      //TRACE_(accel)(", but incorrect SHIFT/CTRL/ALT-state\n");
    }
    else
    {
      if (!(lParam & 0x01000000))  /* no special_key */
      {
        if ((fVirt & GXFALT) && (lParam & 0x20000000))
        {                              /* ^^ ALT pressed */
          // Artint
          //TRACE_(accel)("found accel for Alt-%c\n", GXLOWORD(wParam) & 0xff);
          goto found;
        }
      }
    }
  }
  return FALSE;

found:
  if (message == GXWM_KEYUP || message == GXWM_SYSKEYUP)
    mesg = 1;
  else
  {
    GXHMENU hMenu, hSubMenu, hSysMenu;
    GXUINT uSysStat = (GXUINT)-1, uStat = (GXUINT)-1, nPos;

    hMenu = (gxGetWindowLongW( hWnd, GXGWL_STYLE ) & GXWS_CHILD) ? 0 : gxGetMenu(hWnd);
    hSysMenu = get_win_sys_menu( hWnd );

    /* find menu item and ask application to initialize it */
    /* 1. in the system menu */
    hSubMenu = hSysMenu;
    nPos = cmd;
    if(MENU_FindItem(&hSubMenu, &nPos, GXMF_BYCOMMAND))
    {
      if (gxGetCapture())
        mesg = 2;
      if (!gxIsWindowEnabled(hWnd))
        mesg = 3;
      else
      {
        gxSendMessageW(hWnd, GXWM_INITMENU, (GXWPARAM)hSysMenu, 0L);
        if(hSubMenu != hSysMenu)
        {
          nPos = MENU_FindSubMenu(&hSysMenu, hSubMenu);
          // Artint
          //TRACE_(accel)("hSysMenu = %p, hSubMenu = %p, nPos = %d\n", hSysMenu, hSubMenu, nPos);
          gxSendMessageW(hWnd, GXWM_INITMENUPOPUP, (GXWPARAM)hSubMenu, GXMAKELPARAM(nPos, TRUE));
        }
        uSysStat = gxGetMenuState(gxGetSubMenu(hSysMenu, 0), cmd, GXMF_BYCOMMAND);
      }
    }
    else /* 2. in the window's menu */
    {
      hSubMenu = hMenu;
      nPos = cmd;
      if(MENU_FindItem(&hSubMenu, &nPos, GXMF_BYCOMMAND))
      {
        if (gxGetCapture())
          mesg = 2;
        if (!gxIsWindowEnabled(hWnd))
          mesg = 3;
        else
        {
          gxSendMessageW(hWnd, GXWM_INITMENU, (GXWPARAM)hMenu, 0L);
          if(hSubMenu != hMenu)
          {
            nPos = MENU_FindSubMenu(&hMenu, hSubMenu);
            // Artint
            //TRACE_(accel)("hMenu = %p, hSubMenu = %p, nPos = %d\n", hMenu, hSubMenu, nPos);
            gxSendMessageW(hWnd, GXWM_INITMENUPOPUP, (GXWPARAM)hSubMenu, GXMAKELPARAM(nPos, FALSE));
          }
          uStat = gxGetMenuState(hMenu, cmd, GXMF_BYCOMMAND);
        }
      }
    }

    if (mesg == 0)
    {
      if (uSysStat != (GXUINT)-1)
      {
        if (uSysStat & (GXMF_DISABLED|GXMF_GRAYED))
          mesg=4;
        else
          mesg=GXWM_SYSCOMMAND;
      }
      else
      {
        if (uStat != (GXUINT)-1)
        {
          if (gxIsIconic(hWnd))
            mesg=5;
          else
          {
            if (uStat & (GXMF_DISABLED|GXMF_GRAYED))
              mesg=6;
            else
              mesg=GXWM_COMMAND;
          }
        }
        else
          mesg=GXWM_COMMAND;
      }
    }
  }

  if( mesg==GXWM_COMMAND )
  {
    // Artint
    //TRACE_(accel)(", sending GXWM_COMMAND, wParam=%0x\n", 0x10000 | cmd);
    gxSendMessageW(hWnd, mesg, 0x10000 | cmd, 0L);
  }
  else if( mesg==GXWM_SYSCOMMAND )
  {
    // Artint
    //TRACE_(accel)(", sending GXWM_SYSCOMMAND, wParam=%0x\n", cmd);
    gxSendMessageW(hWnd, mesg, cmd, 0x00010000L);
  }
  else
  {
    /*  some reasons for NOT sending the GXWM_{SYS}COMMAND message:
    *   #0: unknown (please report!)
    *   #1: for GXWM_KEYUP,WM_SYSKEYUP
    *   #2: mouse is captured
    *   #3: window is disabled
    *   #4: it's a disabled system menu option
    *   #5: it's a menu option, but window is iconic
    *   #6: it's a menu option, but disabled
    */
    // Artint
    //TRACE_(accel)(", but won't send GXWM_{SYS}COMMAND, reason is #%d\n",mesg);
    ASSERT(mesg!=0);
    //if(mesg==0)
    //    ERR_(accel)(" unknown reason - please report!\n");
  }
  return TRUE;
}

/**********************************************************************
*      TranslateAcceleratorA     (USER32.@)
*      TranslateAccelerator      (USER32.@)
*/
/*
GXINT TranslateAcceleratorA( GXHWND hWnd, HACCEL hAccel, LPMSG msg )
{
//* YES, Accel16! *
LPACCEL16 lpAccelTbl;
int i;
GXWPARAM wParam;

if (!hWnd || !msg) return 0;

if (!hAccel || !(lpAccelTbl = (LPACCEL16) LockResource16(HACCEL_16(hAccel))))
{
WARN_(accel)("invalid accel handle=%p\n", hAccel);
return 0;
}

wParam = msg->wParam;

switch (msg->message)
{
case GXWM_KEYDOWN:
case GXWM_SYSKEYDOWN:
break;

case GXWM_CHAR:
case GXWM_SYSCHAR:
{
char ch = GXLOWORD(wParam);
GXWCHAR wch;
MultiByteToWideChar(CP_ACP, 0, &ch, 1, &wch, 1);
wParam = GXMAKEWPARAM(wch, GXHIWORD(wParam));
}
break;

default:
return 0;
}

TRACE_(accel)("hAccel %p, hWnd %p, msg->hWnd %p, msg->message %04x, wParam %08lx, lParam %08lx\n",
hAccel,hWnd,msg->hWnd,msg->message,msg->wParam,msg->lParam);
i = 0;
do
{
if (translate_accelerator( hWnd, msg->message, wParam, msg->lParam,
lpAccelTbl[i].fVirt, lpAccelTbl[i].key, lpAccelTbl[i].cmd))
return 1;
} while ((lpAccelTbl[i++].fVirt & 0x80) == 0);

return 0;
}
*/

/**********************************************************************
*      TranslateAcceleratorW     (USER32.@)
*/
GXINT GXDLLAPI gxTranslateAcceleratorW( GXHWND hWnd, GXHACCEL hAccel, GXLPMSG msg )
{
  /* YES, Accel16! */
  GXLPACCEL16 lpAccelTbl;
  int i;

  if (!hWnd || !msg) return 0;

  if (!hAccel || !(lpAccelTbl = (GXLPACCEL16) gxLockResource((GXHGLOBAL)hAccel)))
  {
    // Artint
    //WARN_(accel)("invalid accel handle=%p\n", hAccel);
    return 0;
  }

  switch (msg->message)
  {
  case GXWM_KEYDOWN:
  case GXWM_SYSKEYDOWN:
  case GXWM_CHAR:
  case GXWM_SYSCHAR:
    break;

  default:
    return 0;
  }

  // Artint
  //TRACE_(accel)("hAccel %p, hWnd %p, msg->hWnd %p, msg->message %04x, wParam %08lx, lParam %08lx\n",
  //              hAccel,hWnd,msg->hwnd,msg->message,msg->wParam,msg->lParam);
  i = 0;
  do
  {
    if (translate_accelerator( hWnd, msg->message, msg->wParam, msg->lParam,
      lpAccelTbl[i].fVirt, lpAccelTbl[i].key, lpAccelTbl[i].cmd))
      return 1;
  } while ((lpAccelTbl[i++].fVirt & 0x80) == 0);

  return 0;
}

GXUINT MENU_GetMenuBarHeightFast(GXHWND hWnd, GXUINT menubarWidth, GXINT orgX, GXINT orgY )
{
  LPPOPUPMENU lppop = MENU_GetMenu( gxGetMenu(hWnd) );
  if(lppop == NULL)
    return 0;
  if(lppop->Height > 0)
    return lppop->Height;
  return MENU_GetMenuBarHeight(hWnd, menubarWidth, orgX, orgY);
}
#endif // #if !defined(_WINE_MENU_1_3_20_)
#endif // _DEV_DISABLE_UI_CODE