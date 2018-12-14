#define INFOTIPSIZE 1024

#define TTN_FIRST               (0U-520U)       // tooltips
#define TTN_LAST                (0U-549U)

#define IDI_TT_INFO_SM      1001
#define IDI_TT_WARN_SM      1001
#define IDI_TT_ERROR_SM      1001
//////////////////////////////////////////////////////////////////////////
//
// 以下从 Visual Studio 的 Commctrl.h 复制过来的 ToolTips 控件相关定义
//
//#define NOTOOLTIPS
#ifndef NOTOOLTIPS

//#ifdef _WIN32

#define GXTOOLTIPS_CLASSW         _CLTEXT("tooltips_class32")
#define GXTOOLTIPS_CLASSA         "tooltips_class32"

#ifdef _UNICODE
#define GXTOOLTIPS_CLASS          GXTOOLTIPS_CLASSW
#else
#define GXTOOLTIPS_CLASS          GXTOOLTIPS_CLASSA
#endif

//#else
//#define TOOLTIPS_CLASS          "tooltips_class"
//#endif

#define GXLPTOOLINFOA   GXLPTTTOOLINFOA
#define GXLPTOOLINFOW   GXLPTTTOOLINFOW
#define GXTOOLINFOA       GXTTTOOLINFOA
#define GXTOOLINFOW       GXTTTOOLINFOW

#define GXLPTOOLINFO    GXLPTTTOOLINFO
#define GXTOOLINFO        GXTTTOOLINFO

#define GXTTTOOLINFOA_V1_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOA, lpszText)
#define GXTTTOOLINFOW_V1_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOW, lpszText)
#define GXTTTOOLINFOA_V2_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOA, lParam)
#define GXTTTOOLINFOW_V2_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOW, lParam)
#define GXTTTOOLINFOA_V3_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOA, lpReserved)
#define GXTTTOOLINFOW_V3_SIZE GXCCSIZEOF_STRUCT(GXTTTOOLINFOW, lpReserved)


typedef struct tagGXTOOLINFOA {
  GXUINT cbSize;
  GXUINT uFlags;
  GXHWND hwnd;
  GXUINT_PTR uId;
  GXRECT rect;
  GXHINSTANCE hinst;
  GXLPSTR lpszText;

  GXLPARAM lParam;

  void *lpReserved;
} GXTTTOOLINFOA, *GXLPTTTOOLINFOA, *LPGXTTTOOLINFOA;

typedef struct tagGXTOOLINFOW {
  GXUINT cbSize;
  GXUINT uFlags;
  GXHWND hwnd;
  GXUINT_PTR uId;
  GXRECT rect;
  GXHINSTANCE hinst;
  GXLPWSTR lpszText;

  GXLPARAM lParam;


  void *lpReserved;

} GXTTTOOLINFOW, *GXLPTTTOOLINFOW, *LPGXTTTOOLINFOW;

#ifdef _UNICODE
#define TTTOOLINFO              TTTOOLINFOW
#define PTOOLINFO               PTOOLINFOW
#define LPTTTOOLINFO            LPTTTOOLINFOW
#define TTTOOLINFO_V1_SIZE TTTOOLINFOW_V1_SIZE
#else
#define PTOOLINFO               PTOOLINFOA
#define TTTOOLINFO              TTTOOLINFOA
#define LPTTTOOLINFO            LPTTTOOLINFOA
#define TTTOOLINFO_V1_SIZE TTTOOLINFOA_V1_SIZE
#endif

// begin_r_commctrl

#define TTS_ALWAYSTIP           0x01
#define TTS_NOPREFIX            0x02
#define TTS_NOANIMATE           0x10
#define TTS_NOFADE              0x20
#define TTS_BALLOON             0x40
#define TTS_CLOSE               0x80

// end_r_commctrl

#define TTF_IDISHWND            0x0001

// Use this to center around trackpoint in trackmode
// -OR- to center around tool in normal mode.
// Use TTF_ABSOLUTE to place the tip exactly at the track coords when
// in tracking mode.  TTF_ABSOLUTE can be used in conjunction with TTF_CENTERTIP
// to center the tip absolutely about the track point.

#define TTF_CENTERTIP           0x0002
#define TTF_RTLREADING          0x0004
#define TTF_SUBCLASS            0x0010
#define TTF_TRACK               0x0020
#define TTF_ABSOLUTE            0x0080
#define TTF_TRANSPARENT         0x0100
#define TTF_PARSELINKS          0x1000
#define TTF_DI_SETITEM          0x8000       // valid only on the TTN_NEEDTEXT callback


#define TTDT_AUTOMATIC          0
#define TTDT_RESHOW             1
#define TTDT_AUTOPOP            2
#define TTDT_INITIAL            3

// ToolTip Icons possible wParam values for TTM_SETTITLE message
#define TTI_NONE                0
#define TTI_INFO                1
#define TTI_WARNING             2
#define TTI_ERROR               3
// values larger thant TTI_ERROR are assumed to be an HICON value

// Tool Tip Messages
#define GXTTM_ACTIVATE            (GXWM_USER + 1)
#define GXTTM_SETDELAYTIME        (GXWM_USER + 3)
#define GXTTM_ADDTOOLA            (GXWM_USER + 4)
#define GXTTM_ADDTOOLW            (GXWM_USER + 50)
#define GXTTM_DELTOOLA            (GXWM_USER + 5)
#define GXTTM_DELTOOLW            (GXWM_USER + 51)
#define GXTTM_NEWTOOLRECTA        (GXWM_USER + 6)
#define GXTTM_NEWTOOLRECTW        (GXWM_USER + 52)
#define GXTTM_RELAYEVENT          (GXWM_USER + 7)

#define GXTTM_GETTOOLINFOA        (GXWM_USER + 8)
#define GXTTM_GETTOOLINFOW        (GXWM_USER + 53)

#define GXTTM_SETTOOLINFOA        (GXWM_USER + 9)
#define GXTTM_SETTOOLINFOW        (GXWM_USER + 54)

#define GXTTM_HITTESTA            (GXWM_USER +10)
#define GXTTM_HITTESTW            (GXWM_USER +55)
#define GXTTM_GETTEXTA            (GXWM_USER +11)
#define GXTTM_GETTEXTW            (GXWM_USER +56)
#define GXTTM_UPDATETIPTEXTA      (GXWM_USER +12)
#define GXTTM_UPDATETIPTEXTW      (GXWM_USER +57)
#define GXTTM_GETTOOLCOUNT        (GXWM_USER +13)
#define GXTTM_ENUMTOOLSA          (GXWM_USER +14)
#define GXTTM_ENUMTOOLSW          (GXWM_USER +58)
#define GXTTM_GETCURRENTTOOLA     (GXWM_USER + 15)
#define GXTTM_GETCURRENTTOOLW     (GXWM_USER + 59)
#define GXTTM_WINDOWFROMPOINT     (GXWM_USER + 16)
#define GXTTM_TRACKACTIVATE       (GXWM_USER + 17)  // wParam = TRUE/FALSE start end  lparam = LPTOOLINFO
#define GXTTM_TRACKPOSITION       (GXWM_USER + 18)  // lParam = dwPos
#define GXTTM_SETTIPBKCOLOR       (GXWM_USER + 19)
#define GXTTM_SETTIPTEXTCOLOR     (GXWM_USER + 20)
#define GXTTM_GETDELAYTIME        (GXWM_USER + 21)
#define GXTTM_GETTIPBKCOLOR       (GXWM_USER + 22)
#define GXTTM_GETTIPTEXTCOLOR     (GXWM_USER + 23)
#define GXTTM_SETMAXTIPWIDTH      (GXWM_USER + 24)
#define GXTTM_GETMAXTIPWIDTH      (GXWM_USER + 25)
#define GXTTM_SETMARGIN           (GXWM_USER + 26)  // lParam = lprc
#define GXTTM_GETMARGIN           (GXWM_USER + 27)  // lParam = lprc
#define GXTTM_POP                 (GXWM_USER + 28)

#define GXTTM_UPDATE              (GXWM_USER + 29)

#define GXTTM_GETBUBBLESIZE       (GXWM_USER + 30)
#define GXTTM_ADJUSTRECT          (GXWM_USER + 31)
#define GXTTM_SETTITLEA           (GXWM_USER + 32)  // wParam = TTI_*, lParam = char* szTitle
#define GXTTM_SETTITLEW           (GXWM_USER + 33)  // wParam = TTI_*, lParam = wchar* szTitle

#define GXTTM_POPUP               (GXWM_USER + 34)
#define GXTTM_GETTITLE            (GXWM_USER + 35) // wParam = 0, lParam = TTGETTITLE*

typedef struct _GXTTGETTITLE
{
  GXDWORD dwSize;
  GXUINT uTitleBitmap;
  GXUINT cch;
  GXWCHAR* pszTitle;
} GXTTGETTITLE, *GXPTTGETTITLE;

#ifdef _UNICODE
#define GXTTM_ADDTOOL             GXTTM_ADDTOOLW
#define GXTTM_DELTOOL             GXTTM_DELTOOLW
#define GXTTM_NEWTOOLRECT         GXTTM_NEWTOOLRECTW
#define GXTTM_GETTOOLINFO         GXTTM_GETTOOLINFOW
#define GXTTM_SETTOOLINFO         GXTTM_SETTOOLINFOW
#define GXTTM_HITTEST             GXTTM_HITTESTW
#define GXTTM_GETTEXT             GXTTM_GETTEXTW
#define GXTTM_UPDATETIPTEXT       GXTTM_UPDATETIPTEXTW
#define GXTTM_ENUMTOOLS           GXTTM_ENUMTOOLSW
#define GXTTM_GETCURRENTTOOL      GXTTM_GETCURRENTTOOLW
#define GXTTM_SETTITLE            GXTTM_SETTITLEW
#else
#define GXTTM_ADDTOOL             GXTTM_ADDTOOLA
#define GXTTM_DELTOOL             GXTTM_DELTOOLA
#define GXTTM_NEWTOOLRECT         GXTTM_NEWTOOLRECTA
#define GXTTM_GETTOOLINFO         GXTTM_GETTOOLINFOA
#define GXTTM_SETTOOLINFO         GXTTM_SETTOOLINFOA
#define GXTTM_HITTEST             GXTTM_HITTESTA
#define GXTTM_GETTEXT             GXTTM_GETTEXTA
#define GXTTM_UPDATETIPTEXT       GXTTM_UPDATETIPTEXTA
#define GXTTM_ENUMTOOLS           GXTTM_ENUMTOOLSA
#define GXTTM_GETCURRENTTOOL      GXTTM_GETCURRENTTOOLA
#define GXTTM_SETTITLE            GXTTM_SETTITLEA
#endif

#define GXTTM_SETWINDOWTHEME      GXCCM_SETWINDOWTHEME


#define LPHITTESTINFOW    LPTTHITTESTINFOW
#define LPHITTESTINFOA    LPTTHITTESTINFOA

#define LPHITTESTINFO     LPTTHITTESTINFO

typedef struct _GXTT_HITTESTINFOA {
  GXHWND hwnd;
  GXPOINT pt;
  GXTTTOOLINFOA ti;
} GXTTHITTESTINFOA, *GXLPTTHITTESTINFOA, *LPGXTTHITTESTINFOA;

typedef struct _GXTT_HITTESTINFOW {
  GXHWND hwnd;
  GXPOINT pt;
  GXTTTOOLINFOW ti;
} GXTTHITTESTINFOW, *GXLPTTHITTESTINFOW, *LPGXTTHITTESTINFOW;

#ifdef _UNICODE
#define TTHITTESTINFO           TTHITTESTINFOW
#define LPTTHITTESTINFO         LPTTHITTESTINFOW
#else
#define TTHITTESTINFO           TTHITTESTINFOA
#define LPTTHITTESTINFO         LPTTHITTESTINFOA
#endif

#define TTN_GETDISPINFOA        (TTN_FIRST - 0)
#define TTN_GETDISPINFOW        (TTN_FIRST - 10)
#define TTN_SHOW                (TTN_FIRST - 1)
#define TTN_POP                 (TTN_FIRST - 2)
#define TTN_LINKCLICK           (TTN_FIRST - 3)

#ifdef _UNICODE
#define TTN_GETDISPINFO         TTN_GETDISPINFOW
#else
#define TTN_GETDISPINFO         TTN_GETDISPINFOA
#endif

#define TTN_NEEDTEXT            TTN_GETDISPINFO
#define TTN_NEEDTEXTA           TTN_GETDISPINFOA
#define TTN_NEEDTEXTW           TTN_GETDISPINFOW

#define TOOLTIPTEXTW NMTTDISPINFOW
#define TOOLTIPTEXTA NMTTDISPINFOA
#define LPTOOLTIPTEXTA LPNMTTDISPINFOA
#define LPTOOLTIPTEXTW LPNMTTDISPINFOW

#define TOOLTIPTEXT    NMTTDISPINFO
#define LPTOOLTIPTEXT  LPNMTTDISPINFO

#define NMTTDISPINFOA_V1_SIZE CCSIZEOF_STRUCT(NMTTDISPINFOA, uFlags)
#define NMTTDISPINFOW_V1_SIZE CCSIZEOF_STRUCT(NMTTDISPINFOW, uFlags)

typedef struct tagGXNMTTDISPINFOA {
  GXNMHDR hdr;
  GXLPSTR lpszText;
  char szText[80];
  GXHINSTANCE hinst;
  GXUINT uFlags;
  GXLPARAM lParam;
} GXNMTTDISPINFOA, *GXLPNMTTDISPINFOA, *LPGXNMTTDISPINFOA;

typedef struct tagGXNMTTDISPINFOW {
  GXNMHDR hdr;
  GXLPWSTR lpszText;
  GXWCHAR szText[80];
  GXHINSTANCE hinst;
  GXUINT uFlags;
  GXLPARAM lParam;
} GXNMTTDISPINFOW, *GXLPNMTTDISPINFOW, *LPGXNMTTDISPINFOW;

#ifdef _UNICODE
#define NMTTDISPINFO            NMTTDISPINFOW
#define LPNMTTDISPINFO          LPNMTTDISPINFOW
#define NMTTDISPINFO_V1_SIZE NMTTDISPINFOW_V1_SIZE
#else
#define NMTTDISPINFO            NMTTDISPINFOA
#define LPNMTTDISPINFO          LPNMTTDISPINFOA
#define NMTTDISPINFO_V1_SIZE NMTTDISPINFOA_V1_SIZE
#endif

#endif      // NOTOOLTIPS