typedef struct tagGXCOLORSCHEME {
  GXDWORD            dwSize;
  GXCOLORREF         clrBtnHighlight;       // highlight color
  GXCOLORREF         clrBtnShadow;          // shadow color
} GXCOLORSCHEME, *GXLPCOLORSCHEME;

#define GXCCM_SETCOLORSCHEME      (GXCCM_FIRST + 2) // lParam is color scheme
#define GXCCM_GETCOLORSCHEME      (GXCCM_FIRST + 3) // fills in COLORSCHEME pointed to by lParam
#define GXCCM_GETDROPTARGET       (GXCCM_FIRST + 4)
#define GXCCM_SETUNICODEFORMAT    (GXCCM_FIRST + 5)
#define GXCCM_GETUNICODEFORMAT    (GXCCM_FIRST + 6)

#define COMCTL32_VERSION  6

#define GXCCM_SETVERSION          (GXCCM_FIRST + 0x7)
#define GXCCM_GETVERSION          (GXCCM_FIRST + 0x8)
#define GXCCM_SETNOTIFYWINDOW     (GXCCM_FIRST + 0x9) // wParam == hwndParent.

#define GXCCM_SETWINDOWTHEME      (GXCCM_FIRST + 0xb)
#define GXCCM_DPISCALE            (GXCCM_FIRST + 0xc) // wParam == Awareness

#define CCS_TOP                 0x00000001L
#define CCS_NOMOVEY             0x00000002L
#define CCS_BOTTOM              0x00000003L
#define CCS_NORESIZE            0x00000004L
#define CCS_NOPARENTALIGN       0x00000008L
#define CCS_ADJUSTABLE          0x00000020L
#define CCS_NODIVIDER           0x00000040L
#define CCS_VERT                0x00000080L
#define CCS_LEFT                (CCS_VERT | CCS_TOP)
#define CCS_RIGHT               (CCS_VERT | CCS_BOTTOM)
#define CCS_NOMOVEX             (CCS_VERT | CCS_NOMOVEY)

//typedef struct {
//  GXNMHDR hdr;
//  GXWORD fwKeys;            // Specifies which keys are down when this notification is send
//  GXRECT rcParent;          // Contains Parent Window Rect
//  int  iDir;              // Scrolling Direction
//  int  iXpos;             // Horizontal scroll position
//  int  iYpos;             // Vertical scroll position
//  int  iScroll;           // [in/out] Amount to scroll
//}GXNMPGSCROLL, *GXLPNMPGSCROLL;


typedef struct __tagGXNMTBINITCUSTOMIZE
{
  GXNMHDR hdr;
  GXHWND hwndDialog;
} GXNMTBINITCUSTOMIZE, *GXLPNMTBINITCUSTOMIZE;

typedef struct  __tagGXNMTBWRAPHOTITEM
{
  GXNMHDR hdr;
  GXINT idNew;
  GXINT iDirection; /* left is -1, right is 1 */
  GXDWORD dwReason; /* HICF_* */
} GXNMTBWRAPHOTITEM, *LPGXNMTBWRAPHOTITEM;

typedef GXHANDLE GXHKEY;


#define GXPGM_FORWARDMOUSE        (GXPGM_FIRST + 3)
#define GXPager_ForwardMouse(hwnd, bForward) \
  (void)GXSNDMSG((hwnd), GXPGM_FORWARDMOUSE, (GXWPARAM)(bForward), 0)


#define GXTP_BUTTON          1
#define GXTP_DROPDOWNBUTTON      2
#define GXTP_SPLITBUTTON      3
#define GXTP_SPLITBUTTONDROPDOWN  4
#define GXTP_SEPARATOR        5
#define GXTP_SEPARATORVERT      6

#define GXTS_NORMAL    1
#define GXTS_HOT      2
#define GXTS_PRESSED    3
#define GXTS_DISABLED    4
#define GXTS_CHECKED    5
#define GXTS_HOTCHECKED  6

#define TBSTYLE_EX_UNDOC1               0x00000004 /* similar to TBSTYLE_WRAPABLE */

#define GXDCX_USESTYLE 0x00010000

#define IDS_SEPARATOR      1024

/* Toolbar imagelist bitmaps */
#define IDB_STD_SMALL       120
#define IDB_STD_LARGE       121
#define IDB_VIEW_SMALL      124
#define IDB_VIEW_LARGE      125
#define IDB_HIST_SMALL      130
#define IDB_HIST_LARGE      131
#define IDC_MOVEBUTTON                    1

#define GXTBN_FIRST         (0U-700U)       // toolbar
#define GXTBN_LAST          (0U-720U)

//====== TOOLBAR CONTROL ======================================================
#ifndef NOTOOLBAR

#define TOOLBARCLASSNAMEW       L"ToolbarWindow32"
#define TOOLBARCLASSNAMEA       "ToolbarWindow32"

#ifdef  _UNICODE
#define TOOLBARCLASSNAME        TOOLBARCLASSNAMEW
#else
#define TOOLBARCLASSNAME        TOOLBARCLASSNAMEA
#endif

struct GXTBBUTTON {
  GXINT       iBitmap;
  GXINT_PTR   idCommand;
  GXBYTE      fsState;
  GXBYTE      fsStyle;
#ifdef _WIN64
  GXBYTE      bReserved[6];          // padding for alignment
#elif defined(_WIN32)
  GXBYTE      bReserved[2];          // padding for alignment
#endif
  GXDWORD_PTR dwData;
  GXINT_PTR   iString;
};

typedef GXTBBUTTON*    GXLPTBBUTTON;
typedef GXTBBUTTON*    LPGXTBBUTTON;

typedef const GXTBBUTTON *GXLPCTBBUTTON;


typedef struct _GXCOLORMAP {
  GXCOLORREF from;
  GXCOLORREF to;
} GXCOLORMAP, *GXLPCOLORMAP;

GXHWND GXDLLAPI gxCreateToolbarEx(GXHWND hwnd, GXDWORD ws, GXUINT wID, int nBitmaps,
                       GXHINSTANCE hBMInst, GXUINT_PTR wBMID, GXLPCTBBUTTON lpButtons,
                       int iNumButtons, int dxButton, int dyButton,
                       int dxBitmap, int dyBitmap, GXUINT uStructSize);

//GXHBITMAP GXDLLAPI gxCreateMappedBitmap(GXHINSTANCE hInstance, GXINT_PTR idBitmap,
//                         GXUINT wFlags, GXLPCOLORMAP lpColorMap,
//                         int iNumMaps);

#define CMB_MASKED              0x02
#define TBSTATE_CHECKED         0x01
#define TBSTATE_PRESSED         0x02
#define TBSTATE_ENABLED         0x04
#define TBSTATE_HIDDEN          0x08
#define TBSTATE_INDETERMINATE   0x10
#define TBSTATE_WRAP            0x20
#define TBSTATE_ELLIPSES        0x40
#define TBSTATE_MARKED          0x80

#define TBSTYLE_BUTTON          0x0000  // obsolete; use BTNS_BUTTON instead
#define TBSTYLE_SEP             0x0001  // obsolete; use BTNS_SEP instead
#define TBSTYLE_CHECK           0x0002  // obsolete; use BTNS_CHECK instead
#define TBSTYLE_GROUP           0x0004  // obsolete; use BTNS_GROUP instead
#define TBSTYLE_CHECKGROUP      (TBSTYLE_GROUP | TBSTYLE_CHECK)     // obsolete; use BTNS_CHECKGROUP instead
#define TBSTYLE_DROPDOWN        0x0008  // obsolete; use BTNS_DROPDOWN instead
#define TBSTYLE_AUTOSIZE        0x0010  // obsolete; use BTNS_AUTOSIZE instead
#define TBSTYLE_NOPREFIX        0x0020  // obsolete; use BTNS_NOPREFIX instead

#define TBSTYLE_TOOLTIPS        0x0100
#define TBSTYLE_WRAPABLE        0x0200
#define TBSTYLE_ALTDRAG         0x0400
#define TBSTYLE_FLAT            0x0800
#define TBSTYLE_LIST            0x1000
#define TBSTYLE_CUSTOMERASE     0x2000
#define TBSTYLE_REGISTERDROP    0x4000
#define TBSTYLE_TRANSPARENT     0x8000
#define TBSTYLE_EX_DRAWDDARROWS 0x00000001

#define BTNS_BUTTON         TBSTYLE_BUTTON      // 0x0000
#define BTNS_SEP            TBSTYLE_SEP         // 0x0001
#define BTNS_CHECK          TBSTYLE_CHECK       // 0x0002
#define BTNS_GROUP          TBSTYLE_GROUP       // 0x0004
#define BTNS_CHECKGROUP     TBSTYLE_CHECKGROUP  // (TBSTYLE_GROUP | TBSTYLE_CHECK)
#define BTNS_DROPDOWN       TBSTYLE_DROPDOWN    // 0x0008
#define BTNS_AUTOSIZE       TBSTYLE_AUTOSIZE    // 0x0010; automatically calculate the cx of the button
#define BTNS_NOPREFIX       TBSTYLE_NOPREFIX    // 0x0020; this button should not have accel prefix
#define BTNS_SHOWTEXT       0x0040              // ignored unless TBSTYLE_EX_MIXEDBUTTONS is set
#define BTNS_WHOLEDROPDOWN  0x0080          // draw drop-down arrow, but without split arrow section

#define TBSTYLE_EX_MIXEDBUTTONS             0x00000008
#define TBSTYLE_EX_HIDECLIPPEDBUTTONS       0x00000010  // don't show partially obscured buttons


#define TBSTYLE_EX_DOUBLEBUFFER             0x00000080 // Double Buffer the toolbar

// Custom Draw Structure
typedef struct _GXNMTBCUSTOMDRAW {
  GXNMCUSTOMDRAW nmcd;
  GXHBRUSH hbrMonoDither;
  GXHBRUSH hbrLines;                // For drawing lines on buttons
  GXHPEN hpenLines;                 // For drawing lines on buttons

  GXCOLORREF clrText;               // Color of text
  GXCOLORREF clrMark;               // Color of text bk when marked. (only if TBSTATE_MARKED)
  GXCOLORREF clrTextHighlight;      // Color of text when highlighted
  GXCOLORREF clrBtnFace;            // Background of the button
  GXCOLORREF clrBtnHighlight;       // 3D highlight
  GXCOLORREF clrHighlightHotTrack;  // In conjunction with fHighlightHotTrack
  // will cause button to highlight like a menu
  GXRECT rcText;                    // Rect for text

  GXINT nStringBkMode;
  GXINT nHLStringBkMode;
  GXINT iListGap;
} GXNMTBCUSTOMDRAW, * LPGXNMTBCUSTOMDRAW, * GXLPNMTBCUSTOMDRAW;

// Toolbar custom draw return flags
#define TBCDRF_NOEDGES              0x00010000  // Don't draw button edges
#define TBCDRF_HILITEHOTTRACK       0x00020000  // Use color of the button bk when hottracked
#define TBCDRF_NOOFFSET             0x00040000  // Don't offset button if pressed
#define TBCDRF_NOMARK               0x00080000  // Don't draw default highlight of image/text for TBSTATE_MARKED
#define TBCDRF_NOETCHEDEFFECT       0x00100000  // Don't draw etched effect for disabled items

#define TBCDRF_BLENDICON            0x00200000  // Use ILD_BLEND50 on the icon image
#define TBCDRF_NOBACKGROUND         0x00400000  // Use ILD_BLEND50 on the icon image


#define GXTB_ENABLEBUTTON           (GXWM_USER + 1)
#define GXTB_CHECKBUTTON            (GXWM_USER + 2)
#define GXTB_PRESSBUTTON            (GXWM_USER + 3)
#define GXTB_HIDEBUTTON             (GXWM_USER + 4)
#define GXTB_INDETERMINATE          (GXWM_USER + 5)
#define GXTB_MARKBUTTON             (GXWM_USER + 6)
#define GXTB_ISBUTTONENABLED        (GXWM_USER + 9)
#define GXTB_ISBUTTONCHECKED        (GXWM_USER + 10)
#define GXTB_ISBUTTONPRESSED        (GXWM_USER + 11)
#define GXTB_ISBUTTONHIDDEN         (GXWM_USER + 12)
#define GXTB_ISBUTTONINDETERMINATE  (GXWM_USER + 13)
#define GXTB_ISBUTTONHIGHLIGHTED    (GXWM_USER + 14)
#define GXTB_SETSTATE               (GXWM_USER + 17)
#define GXTB_GETSTATE               (GXWM_USER + 18)
#define GXTB_ADDBITMAP              (GXWM_USER + 19)

typedef struct tagGXTBADDBITMAP {
  GXHINSTANCE       hInst;
  GXUINT_PTR        nID;
} GXTBADDBITMAP, *LPGXTBADDBITMAP, *GXLPTBADDBITMAP;

#define GXHINST_COMMCTRL        ((GXHINSTANCE)-1)
#define IDB_STD_SMALL_COLOR     0
#define IDB_STD_LARGE_COLOR     1
#define IDB_VIEW_SMALL_COLOR    4
#define IDB_VIEW_LARGE_COLOR    5
#define IDB_HIST_SMALL_COLOR    8
#define IDB_HIST_LARGE_COLOR    9

// icon indexes for standard bitmap

#define STD_CUT                 0
#define STD_COPY                1
#define STD_PASTE               2
#define STD_UNDO                3
#define STD_REDOW               4
#define STD_DELETE              5
#define STD_FILENEW             6
#define STD_FILEOPEN            7
#define STD_FILESAVE            8
#define STD_PRINTPRE            9
#define STD_PROPERTIES          10
#define STD_HELP                11
#define STD_FIND                12
#define STD_REPLACE             13
#define STD_PRINT               14

// icon indexes for standard view bitmap

#define VIEW_LARGEICONS         0
#define VIEW_SMALLICONS         1
#define VIEW_LIST               2
#define VIEW_DETAILS            3
#define VIEW_SORTNAME           4
#define VIEW_SORTSIZE           5
#define VIEW_SORTDATE           6
#define VIEW_SORTTYPE           7
#define VIEW_PARENTFOLDER       8
#define VIEW_NETCONNECT         9
#define VIEW_NETDISCONNECT      10
#define VIEW_NEWFOLDER          11
#define VIEW_VIEWMENU           12

#define HIST_BACK               0
#define HIST_FORWARD            1
#define HIST_FAVORITES          2
#define HIST_ADDTOFAVORITES     3
#define HIST_VIEWTREE           4


#define GXTB_ADDBUTTONSA          (GXWM_USER + 20)
#define GXTB_INSERTBUTTONA        (GXWM_USER + 21)

#define GXTB_DELETEBUTTON         (GXWM_USER + 22)
#define GXTB_GETBUTTON            (GXWM_USER + 23)
#define GXTB_BUTTONCOUNT          (GXWM_USER + 24)
#define GXTB_COMMANDTOINDEX       (GXWM_USER + 25)


typedef struct tagGXTBSAVEPARAMSA {
  GXHKEY hkr;
  GXLPCSTR pszSubKey;
  GXLPCSTR pszValueName;
} GXTBSAVEPARAMSA, *LPGXTBSAVEPARAMSA;

typedef struct tagGXTBSAVEPARAMSW {
  GXHKEY hkr;
  GXLPCWSTR pszSubKey;
  GXLPCWSTR pszValueName;
} GXTBSAVEPARAMSW, *LPGXTBSAVEPARAMSW;

#define TBSAVEPARAMS            TBSAVEPARAMSW
#define LPTBSAVEPARAMS          LPTBSAVEPARAMSW


#define GXTB_SAVERESTOREA         (GXWM_USER + 26)
#define GXTB_SAVERESTOREW         (GXWM_USER + 76)
#define GXTB_CUSTOMIZE            (GXWM_USER + 27)
#define GXTB_ADDSTRINGA           (GXWM_USER + 28)
#define GXTB_ADDSTRINGW           (GXWM_USER + 77)
#define GXTB_GETITEMRECT          (GXWM_USER + 29)
#define GXTB_BUTTONSTRUCTSIZE     (GXWM_USER + 30)
#define GXTB_SETBUTTONSIZE        (GXWM_USER + 31)
#define GXTB_SETBITMAPSIZE        (GXWM_USER + 32)
#define GXTB_AUTOSIZE             (GXWM_USER + 33)
#define GXTB_GETTOOLTIPS          (GXWM_USER + 35)
#define GXTB_SETTOOLTIPS          (GXWM_USER + 36)
#define GXTB_SETPARENT            (GXWM_USER + 37)
#define GXTB_SETROWS              (GXWM_USER + 39)
#define GXTB_GETROWS              (GXWM_USER + 40)
#define GXTB_SETCMDID             (GXWM_USER + 42)
#define GXTB_CHANGEBITMAP         (GXWM_USER + 43)
#define GXTB_GETBITMAP            (GXWM_USER + 44)
#define GXTB_GETBUTTONTEXTA       (GXWM_USER + 45)
#define GXTB_GETBUTTONTEXTW       (GXWM_USER + 75)
#define GXTB_REPLACEBITMAP        (GXWM_USER + 46)
#define GXTB_SETINDENT            (GXWM_USER + 47)
#define GXTB_SETIMAGELIST         (GXWM_USER + 48)
#define GXTB_GETIMAGELIST         (GXWM_USER + 49)
#define GXTB_LOADIMAGES           (GXWM_USER + 50)
#define GXTB_GETRECT              (GXWM_USER + 51) // wParam is the Cmd instead of index
#define GXTB_SETHOTIMAGELIST      (GXWM_USER + 52)
#define GXTB_GETHOTIMAGELIST      (GXWM_USER + 53)
#define GXTB_SETDISABLEDIMAGELIST (GXWM_USER + 54)
#define GXTB_GETDISABLEDIMAGELIST (GXWM_USER + 55)
#define GXTB_SETSTYLE             (GXWM_USER + 56)
#define GXTB_GETSTYLE             (GXWM_USER + 57)
#define GXTB_GETBUTTONSIZE        (GXWM_USER + 58)
#define GXTB_SETBUTTONWIDTH       (GXWM_USER + 59)
#define GXTB_SETMAXTEXTROWS       (GXWM_USER + 60)
#define GXTB_GETTEXTROWS          (GXWM_USER + 61)

#ifdef _UNICODE
#define GXTB_GETBUTTONTEXT        GXTB_GETBUTTONTEXTW
#define GXTB_SAVERESTORE          GXTB_SAVERESTOREW
#define GXTB_ADDSTRING            GXTB_ADDSTRINGW
#else
#define GXTB_GETBUTTONTEXT        GXTB_GETBUTTONTEXTA
#define GXTB_SAVERESTORE          GXTB_SAVERESTOREA
#define GXTB_ADDSTRING            GXTB_ADDSTRINGA
#endif

#define GXTB_GETOBJECT            (GXWM_USER + 62)  // wParam == IID, lParam void **ppv
#define GXTB_GETHOTITEM           (GXWM_USER + 71)
#define GXTB_SETHOTITEM           (GXWM_USER + 72)  // wParam == iHotItem
#define GXTB_SETANCHORHIGHLIGHT   (GXWM_USER + 73)  // wParam == TRUE/FALSE
#define GXTB_GETANCHORHIGHLIGHT   (GXWM_USER + 74)
#define GXTB_MAPACCELERATORA      (GXWM_USER + 78)  // wParam == ch, lParam int * pidBtn

typedef struct {
  int     iButton;
  GXDWORD dwFlags;
} GXTBINSERTMARK, *LPGXTBINSERTMARK, *GXLPTBINSERTMARK;
#define GXTBIMHT_AFTER      0x00000001 // TRUE = insert After iButton, otherwise before
#define GXTBIMHT_BACKGROUND 0x00000002 // TRUE iff missed buttons completely

#define GXTB_GETINSERTMARK        (GXWM_USER + 79)  // lParam == LPTBINSERTMARK
#define GXTB_SETINSERTMARK        (GXWM_USER + 80)  // lParam == LPTBINSERTMARK
#define GXTB_INSERTMARKHITTEST    (GXWM_USER + 81)  // wParam == LPGXPOINT lParam == LPTBINSERTMARK
#define GXTB_MOVEBUTTON           (GXWM_USER + 82)
#define GXTB_GETMAXSIZE           (GXWM_USER + 83)  // lParam == GXLPSIZE
#define GXTB_SETEXTENDEDSTYLE     (GXWM_USER + 84)  // For TBSTYLE_EX_*
#define GXTB_GETEXTENDEDSTYLE     (GXWM_USER + 85)  // For TBSTYLE_EX_*
#define GXTB_GETPADDING           (GXWM_USER + 86)
#define GXTB_SETPADDING           (GXWM_USER + 87)
#define GXTB_SETINSERTMARKCOLOR   (GXWM_USER + 88)
#define GXTB_GETINSERTMARKCOLOR   (GXWM_USER + 89)

#define GXTB_SETCOLORSCHEME       GXCCM_SETCOLORSCHEME  // lParam is color scheme
#define GXTB_GETCOLORSCHEME       GXCCM_GETCOLORSCHEME      // fills in COLORSCHEME pointed to by lParam

#define GXTB_SETUNICODEFORMAT     GXCCM_SETUNICODEFORMAT
#define GXTB_GETUNICODEFORMAT     GXCCM_GETUNICODEFORMAT

#define GXTB_MAPACCELERATORW      (GXWM_USER + 90)  // wParam == ch, lParam int * pidBtn
#ifdef _UNICODE
#define GXTB_MAPACCELERATOR       GXTB_MAPACCELERATORW
#else
#define GXTB_MAPACCELERATOR       GXTB_MAPACCELERATORA
#endif


typedef struct {
  GXHINSTANCE       hInstOld;
  GXUINT_PTR        nIDOld;
  GXHINSTANCE       hInstNew;
  GXUINT_PTR        nIDNew;
  int               nButtons;
} GXTBREPLACEBITMAP, *LPGXTBREPLACEBITMAP, *GXLPTBREPLACEBITMAP;


#define TBBF_LARGE              0x0001

#define GXTB_GETBITMAPFLAGS       (GXWM_USER + 41)


#define GXTBIF_IMAGE              0x00000001
#define GXTBIF_TEXT               0x00000002
#define GXTBIF_STATE              0x00000004
#define GXTBIF_STYLE              0x00000008
#define GXTBIF_LPARAM             0x00000010
#define GXTBIF_COMMAND            0x00000020
#define GXTBIF_SIZE               0x00000040

#define GXTBIF_BYINDEX            0x80000000 // this specifies that the wparam in Get/SetButtonInfo is an index, not id


typedef struct {
  GXUINT cbSize;
  GXDWORD dwMask;
  int  idCommand;
  int  iImage;
  GXBYTE fsState;
  GXBYTE fsStyle;
  GXWORD cx;
  GXDWORD_PTR lParam;
  GXLPSTR pszText;
  int  cchText;
} GXTBBUTTONINFOA, *GXLPTBBUTTONINFOA, *LPGXTBBUTTONINFOA;

typedef struct {
  GXUINT      cbSize;
  GXDWORD     dwMask;
  int         idCommand;
  int         iImage;
  GXBYTE      fsState;
  GXBYTE      fsStyle;
  GXWORD      cx;
  GXDWORD_PTR lParam;
  GXLPWSTR    pszText;
  int         cchText;
} GXTBBUTTONINFOW, *GXLPTBBUTTONINFOW, *LPGXTBBUTTONINFOW;

#ifdef _UNICODE
#define TBBUTTONINFO TBBUTTONINFOW
#define LPTBBUTTONINFO LPTBBUTTONINFOW
#else
#define TBBUTTONINFO TBBUTTONINFOA
#define LPTBBUTTONINFO LPTBBUTTONINFOA
#endif


// BUTTONINFO APIs do NOT support the string pool.
#define GXTB_GETBUTTONINFOW        (GXWM_USER + 63)
#define GXTB_SETBUTTONINFOW        (GXWM_USER + 64)
#define GXTB_GETBUTTONINFOA        (GXWM_USER + 65)
#define GXTB_SETBUTTONINFOA        (GXWM_USER + 66)
#ifdef _UNICODE
#define GXTB_GETBUTTONINFO        GXTB_GETBUTTONINFOW
#define GXTB_SETBUTTONINFO        GXTB_SETBUTTONINFOW
#else
#define GXTB_GETBUTTONINFO        GXTB_GETBUTTONINFOA
#define GXTB_SETBUTTONINFO        GXTB_SETBUTTONINFOA
#endif


#define GXTB_INSERTBUTTONW        (GXWM_USER + 67)
#define GXTB_ADDBUTTONSW          (GXWM_USER + 68)

#define GXTB_HITTEST              (GXWM_USER + 69)

// New post Win95/NT4 for InsertButton and AddButton.  if iString member
// is a pointer to a string, it will be handled as a string like listview
// (although LPSTR_TEXTCALLBACK is not supported).
#ifdef _UNICODE
#define GXTB_INSERTBUTTON         GXTB_INSERTBUTTONW
#define GXTB_ADDBUTTONS           GXTB_ADDBUTTONSW
#else
#define GXTB_INSERTBUTTON         GXTB_INSERTBUTTONA
#define GXTB_ADDBUTTONS           GXTB_ADDBUTTONSA
#endif

#define GXTB_SETDRAWTEXTFLAGS     (GXWM_USER + 70)  // wParam == mask lParam == bit values
#define GXTB_GETSTRINGW           (GXWM_USER + 91)
#define GXTB_GETSTRINGA           (GXWM_USER + 92)
#ifdef _UNICODE
#define GXTB_GETSTRING            GXTB_GETSTRINGW
#else
#define GXTB_GETSTRING            GXTB_GETSTRINGA
#endif

// GXUI µÄ Toolbar ÏûÏ¢
#define GXUITB_SETSPRITEFILE    (GXWM_USER + 101) // lParam sprite file
#define GXUITB_SETSPRITEOBJ     (GXWM_USER + 102) // lParam sprite Object

#define TBMF_PAD                0x00000001
#define TBMF_BARPAD             0x00000002
#define TBMF_BUTTONSPACING      0x00000004

typedef struct {
  GXUINT cbSize;
  GXDWORD dwMask;

  int cxPad;        // PAD
  int cyPad;
  int cxBarPad;     // BARPAD
  int cyBarPad;
  int cxButtonSpacing;   // BUTTONSPACING
  int cyButtonSpacing;
} GXTBMETRICS, *GXLPTBMETRICS, *LPGXTBMETRICS;

#define GXTB_GETMETRICS           (GXWM_USER + 101)
#define GXTB_SETMETRICS           (GXWM_USER + 102)


#define GXTB_SETWINDOWTHEME       GXCCM_SETWINDOWTHEME

#define GXTBN_GETBUTTONINFOA      (GXTBN_FIRST-0)
#define GXTBN_BEGINDRAG           (GXTBN_FIRST-1)
#define GXTBN_ENDDRAG             (GXTBN_FIRST-2)
#define GXTBN_BEGINADJUST         (GXTBN_FIRST-3)
#define GXTBN_ENDADJUST           (GXTBN_FIRST-4)
#define GXTBN_RESET               (GXTBN_FIRST-5)
#define GXTBN_QUERYINSERT         (GXTBN_FIRST-6)
#define GXTBN_QUERYDELETE         (GXTBN_FIRST-7)
#define GXTBN_TOOLBARCHANGE       (GXTBN_FIRST-8)
#define GXTBN_CUSTHELP            (GXTBN_FIRST-9)
#define GXTBN_DROPDOWN            (GXTBN_FIRST - 10)

#define GXTBN_GETOBJECT           (GXTBN_FIRST - 12)

// Structure for TBN_HOTITEMCHANGE notification
//
typedef struct tagGXNMTBHOTITEM
{
  GXNMHDR   hdr;
  int     idOld;
  int     idNew;
  GXDWORD   dwFlags;           // HICF_*
} GXNMTBHOTITEM, *GXLPNMTBHOTITEM, *LPGXNMTBHOTITEM;

// Hot item change flags
#define HICF_OTHER          0x00000000
#define HICF_MOUSE          0x00000001          // Triggered by mouse
#define HICF_ARROWKEYS      0x00000002          // Triggered by arrow keys
#define HICF_ACCELERATOR    0x00000004          // Triggered by accelerator
#define HICF_DUPACCEL       0x00000008          // This accelerator is not unique
#define HICF_ENTERING       0x00000010          // idOld is invalid
#define HICF_LEAVING        0x00000020          // idNew is invalid
#define HICF_RESELECT       0x00000040          // hot item reselected
#define HICF_LMOUSE         0x00000080          // left mouse button selected
#define HICF_TOGGLEDROPDOWN 0x00000100          // Toggle button's dropdown state


#define GXTBN_HOTITEMCHANGE       (GXTBN_FIRST - 13)
#define GXTBN_DRAGOUT             (GXTBN_FIRST - 14) // this is sent when the user clicks down on a button then drags off the button
#define GXTBN_DELETINGBUTTON      (GXTBN_FIRST - 15) // uses TBNOTIFY
#define GXTBN_GETDISPINFOA        (GXTBN_FIRST - 16) // This is sent when the  toolbar needs  some display information
#define GXTBN_GETDISPINFOW        (GXTBN_FIRST - 17) // This is sent when the  toolbar needs  some display information
#define GXTBN_GETINFOTIPA         (GXTBN_FIRST - 18)
#define GXTBN_GETINFOTIPW         (GXTBN_FIRST - 19)
#define GXTBN_GETBUTTONINFOW      (GXTBN_FIRST - 20)
#define GXTBN_RESTORE             (GXTBN_FIRST - 21)
#define GXTBN_SAVE                (GXTBN_FIRST - 22)
#define GXTBN_INITCUSTOMIZE       (GXTBN_FIRST - 23)
#define GXTBN_WRAPHOTITEM         (GXTBN_FIRST-24) /* this is undocumented and the name is a guess */
#define GXTBNRF_HIDEHELP       0x00000001
#define GXTBNRF_ENDCUSTOMIZE   0x00000002




typedef struct tagGXNMTBSAVE
{
  GXNMHDR hdr;
  GXDWORD* pData;
  GXDWORD* pCurrent;
  GXUINT cbData;
  int iItem;
  int cButtons;
  GXTBBUTTON tbButton;
} GXNMTBSAVE, *GXLPNMTBSAVE, *LPGXNMTBSAVE;

typedef struct tagGXNMTBRESTORE
{
  GXNMHDR hdr;
  GXDWORD* pData;
  GXDWORD* pCurrent;
  GXUINT cbData;
  GXINT iItem;
  GXINT cButtons;
  GXINT cbBytesPerRecord;
  GXTBBUTTON tbButton;
} GXNMTBRESTORE, *GXLPNMTBRESTORE, *LPGXNMTBRESTORE;

typedef struct tagGXNMTBGETINFOTIPA
{
  GXNMHDR hdr;
  GXLPSTR pszText;
  int cchTextMax;
  int iItem;
  GXLPARAM lParam;
} GXNMTBGETINFOTIPA, *GXLPNMTBGETINFOTIPA, *LPGXNMTBGETINFOTIPA;

typedef struct tagGXNMTBGETINFOTIPW
{
  GXNMHDR hdr;
  GXLPWSTR pszText;
  int cchTextMax;
  int iItem;
  GXLPARAM lParam;
} GXNMTBGETINFOTIPW, *GXLPNMTBGETINFOTIPW, *LPGXNMTBGETINFOTIPW;

#ifdef _UNICODE
#define TBN_GETINFOTIP          TBN_GETINFOTIPW
#define NMTBGETINFOTIP          NMTBGETINFOTIPW
#define LPNMTBGETINFOTIP        LPNMTBGETINFOTIPW
#else
#define TBN_GETINFOTIP          TBN_GETINFOTIPA
#define NMTBGETINFOTIP          NMTBGETINFOTIPA
#define LPNMTBGETINFOTIP        LPNMTBGETINFOTIPA
#endif

#define TBNF_IMAGE              0x00000001
#define TBNF_TEXT               0x00000002
#define TBNF_DI_SETITEM         0x10000000

typedef struct {
  GXNMHDR  hdr;
  GXDWORD dwMask;     // [in] Specifies the values requested .[out] Client ask the data to be set for future use
  int idCommand;    // [in] id of button we're requesting info for
  GXDWORD_PTR lParam;  // [in] lParam of button
  int iImage;       // [out] image index
  GXLPSTR pszText;    // [out] new text for item
  int cchText;      // [in] size of buffer pointed to by pszText
} GXNMTBDISPINFOA, *GXLPNMTBDISPINFOA, *LPGXNMTBDISPINFOA;

typedef struct {
  GXNMHDR hdr;
  GXDWORD dwMask;      //[in] Specifies the values requested .[out] Client ask the data to be set for future use
  GXINT idCommand;    // [in] id of button we're requesting info for
  GXDWORD_PTR lParam;  // [in] lParam of button
  GXINT iImage;       // [out] image index
  GXLPWSTR pszText;   // [out] new text for item
  GXINT cchText;      // [in] size of buffer pointed to by pszText
} GXNMTBDISPINFOW, *GXLPNMTBDISPINFOW, *LPGXNMTBDISPINFOW;



#ifdef _UNICODE
#define TBN_GETDISPINFO       TBN_GETDISPINFOW
#define NMTBDISPINFO          NMTBDISPINFOW
#define LPNMTBDISPINFO        LPNMTBDISPINFOW
#else
#define TBN_GETDISPINFO       TBN_GETDISPINFOA
#define NMTBDISPINFO          NMTBDISPINFOA
#define LPNMTBDISPINFO        LPNMTBDISPINFOA
#endif

// Return codes for TBN_DROPDOWN
#define TBDDRET_DEFAULT         0
#define TBDDRET_NODEFAULT       1
#define TBDDRET_TREATPRESSED    2       // Treat as a standard press button




#ifdef _UNICODE
#define TBN_GETBUTTONINFO       TBN_GETBUTTONINFOW
#else
#define TBN_GETBUTTONINFO       TBN_GETBUTTONINFOA
#endif

#define TBNOTIFYA NMTOOLBARA
#define TBNOTIFYW NMTOOLBARW
#define LPTBNOTIFYA LPNMTOOLBARA
#define LPTBNOTIFYW LPNMTOOLBARW

#define TBNOTIFY       NMTOOLBAR
#define LPTBNOTIFY     LPNMTOOLBAR

typedef struct tagGXNMTOOLBARA {
  GXNMHDR    hdr;
  GXINT     iItem;
  GXTBBUTTON tbButton;
  GXINT      cchText;
  GXLPSTR    pszText;
  GXRECT     rcButton;
} GXNMTOOLBARA, *GXLPNMTOOLBARA, *LPGXNMTOOLBARA;


typedef struct tagGXNMTOOLBARW {
  GXNMHDR    hdr;
  GXINT      iItem;
  GXTBBUTTON tbButton;
  GXINT      cchText;
  GXLPWSTR   pszText;
  GXRECT     rcButton;
} GXNMTOOLBARW, *GXLPNMTOOLBARW, *LPGXNMTOOLBARW;


#ifdef _UNICODE
#define NMTOOLBAR               NMTOOLBARW
#define LPNMTOOLBAR             LPNMTOOLBARW
#else
#define NMTOOLBAR               NMTOOLBARA
#define LPNMTOOLBAR             LPNMTOOLBARA
#endif



#endif      // NOTOOLBAR