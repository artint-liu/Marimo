#define GXPGM_FIRST               0x1400      // Pager control messages

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
///  ====================== Pager Control =============================
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

#ifndef NOPAGESCROLLER

//Pager Class Name
#define WC_PAGESCROLLERW           L"SysPager"
#define WC_PAGESCROLLERA           "SysPager"

#ifdef _UNICODE
#define WC_PAGESCROLLER          WC_PAGESCROLLERW
#else
#define WC_PAGESCROLLER          WC_PAGESCROLLERA
#endif


//---------------------------------------------------------------------------------------
// Pager Control Styles
//---------------------------------------------------------------------------------------
// begin_r_commctrl

#define PGS_VERT                0x00000000
#define PGS_HORZ                0x00000001
#define PGS_AUTOSCROLL          0x00000002
#define PGS_DRAGNDROP           0x00000004

// end_r_commctrl


//---------------------------------------------------------------------------------------
// Pager Button State
//---------------------------------------------------------------------------------------
//The scroll can be in one of the following control State
#define  PGF_INVISIBLE   0      // Scroll button is not visible
#define  PGF_NORMAL      1      // Scroll button is in normal state
#define  PGF_GRAYED      2      // Scroll button is in grayed state
#define  PGF_DEPRESSED   4      // Scroll button is in depressed state
#define  PGF_HOT         8      // Scroll button is in hot state


// The following identifiers specifies the button control
#define PGB_TOPORLEFT       0
#define PGB_BOTTOMORRIGHT   1

//---------------------------------------------------------------------------------------
// Pager Control  Messages
//---------------------------------------------------------------------------------------
#define GXPGM_SETCHILD            (GXPGM_FIRST + 1)  // GXLPARAM == hwnd
#define GXPager_SetChild(hwnd, hwndChild) \
  (void)GXSNDMSG((hwnd), GXPGM_SETCHILD, 0, (GXLPARAM)(hwndChild))

#define GXPGM_RECALCSIZE          (GXPGM_FIRST + 2)
#define GXPager_RecalcSize(hwnd) \
  (void)GXSNDMSG((hwnd), GXPGM_RECALCSIZE, 0, 0)

#define GXPGM_FORWARDMOUSE        (GXPGM_FIRST + 3)
#define GXPager_ForwardMouse(hwnd, bForward) \
  (void)GXSNDMSG((hwnd), GXPGM_FORWARDMOUSE, (GXWPARAM)(bForward), 0)

#define GXPGM_SETBKCOLOR          (GXPGM_FIRST + 4)
#define GXPager_SetBkColor(hwnd, clr) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXPGM_SETBKCOLOR, 0, (GXLPARAM)(clr))

#define GXPGM_GETBKCOLOR          (GXPGM_FIRST + 5)
#define GXPager_GetBkColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXPGM_GETBKCOLOR, 0, 0)

#define GXPGM_SETBORDER          (GXPGM_FIRST + 6)
#define GXPager_SetBorder(hwnd, iBorder) \
  (int)GXSNDMSG((hwnd), GXPGM_SETBORDER, 0, (GXLPARAM)(iBorder))

#define GXPGM_GETBORDER          (GXPGM_FIRST + 7)
#define GXPager_GetBorder(hwnd) \
  (int)GXSNDMSG((hwnd), GXPGM_GETBORDER, 0, 0)

#define GXPGM_SETPOS              (GXPGM_FIRST + 8)
#define GXPager_SetPos(hwnd, iPos) \
  (int)GXSNDMSG((hwnd), GXPGM_SETPOS, 0, (GXLPARAM)(iPos))

#define GXPGM_GETPOS              (GXPGM_FIRST + 9)
#define GXPager_GetPos(hwnd) \
  (int)GXSNDMSG((hwnd), GXPGM_GETPOS, 0, 0)

#define GXPGM_SETBUTTONSIZE       (GXPGM_FIRST + 10)
#define GXPager_SetButtonSize(hwnd, iSize) \
  (int)GXSNDMSG((hwnd), GXPGM_SETBUTTONSIZE, 0, (GXLPARAM)(iSize))

#define GXPGM_GETBUTTONSIZE       (GXPGM_FIRST + 11)
#define GXPager_GetButtonSize(hwnd) \
  (int)GXSNDMSG((hwnd), GXPGM_GETBUTTONSIZE, 0,0)

#define GXPGM_GETBUTTONSTATE      (GXPGM_FIRST + 12)
#define GXPager_GetButtonState(hwnd, iButton) \
  (GXDWORD)GXSNDMSG((hwnd), GXPGM_GETBUTTONSTATE, 0, (GXLPARAM)(iButton))

#define GXPGM_GETDROPTARGET       GXCCM_GETDROPTARGET
#define GXPager_GetDropTarget(hwnd, ppdt) \
  (void)GXSNDMSG((hwnd), GXPGM_GETDROPTARGET, 0, (GXLPARAM)(ppdt))
//---------------------------------------------------------------------------------------
//Pager Control Notification Messages
//---------------------------------------------------------------------------------------


// PGN_SCROLL Notification Message

#define PGN_SCROLL          (PGN_FIRST-1)

#define PGF_SCROLLUP        1
#define PGF_SCROLLDOWN      2
#define PGF_SCROLLLEFT      4
#define PGF_SCROLLRIGHT     8


//Keys down
#define PGK_SHIFT           1
#define PGK_CONTROL         2
#define PGK_MENU            4


#ifdef _WIN32
#include <pshpack1.h>
#endif

// This structure is sent along with PGN_SCROLL notifications
typedef struct {
  GXNMHDR hdr;
  GXWORD fwKeys;            // Specifies which keys are down when this notification is send
  GXRECT rcParent;          // Contains Parent Window Rect
  int  iDir;              // Scrolling Direction
  int  iXpos;             // Horizontal scroll position
  int  iYpos;             // Vertical scroll position
  int  iScroll;           // [in/out] Amount to scroll
}GXNMPGSCROLL, *GXLPNMPGSCROLL;

#ifdef _WIN32
#include <poppack.h>
#endif

// PGN_CALCSIZE Notification Message

#define PGN_CALCSIZE        (PGN_FIRST-2)

#define PGF_CALCWIDTH       1
#define PGF_CALCHEIGHT      2

typedef struct {
  GXNMHDR   hdr;
  GXDWORD   dwFlag;
  int     iWidth;
  int     iHeight;
}GXNMPGCALCSIZE, *GXLPNMPGCALCSIZE;


// PGN_HOTITEMCHANGE Notification Message

#define PGN_HOTITEMCHANGE   (PGN_FIRST-3)

/* 
The PGN_HOTITEMCHANGE notification uses these notification
flags defined in TOOLBAR:

#define HICF_ENTERING       0x00000010          // idOld is invalid
#define HICF_LEAVING        0x00000020          // idNew is invalid
*/

// Structure for PGN_HOTITEMCHANGE notification
//
typedef struct tagGXNMPGHOTITEM
{
  GXNMHDR   hdr;
  int     idOld;
  int     idNew;
  GXDWORD   dwFlags;           // HICF_*
} GXNMPGHOTITEM, * GXLPNMPGHOTITEM;

#endif // NOPAGESCROLLER