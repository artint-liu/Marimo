#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GX_USER_H_
#define _GX_USER_H_

#define POPUPMENU_CLASS_ATOM L"#32768"    // PopupMenu
#define DESKTOP_CLASS_ATOM   L"#32769"    // Desktop
#define DIALOG_CLASS_ATOM    L"#32770"    // Dialog
#define DIALOG_CLASS_ATOM_EX L"#32780"    // Dialog Ex 从文件创建的对话框类
#define WINSWITCH_CLASS_ATOM L"#32771"    // WinSwitch
#define ICONTITLE_CLASS_ATOM L"#32772"    // IconTitle

#define GXWE_MENUW                POPUPMENU_CLASS_ATOM
#define GXWE_BUTTONW              _T("Button")
#define GXWE_EDITW                _T("Edit")
#define GXWE_EDITW_1_3_30         _T("Edit_1_3_30")
#define GXWE_LISTBOXW             _T("ListBox")
#define GXWE_SCROLLBARW           _T("ScrollBar")
#define GXWE_STATICW              _T("Static")
#define GXWE_LISTVIEWW            _T("SysListView32")
#define GXWE_TREEVIEWW            _T("SysTreeView32")
#define GXUICLASSNAME_EDIT        _T("GXUIEdit")       // 这个是从Wine的Edit控件修改来的
#define GXUICLASSNAME_EDIT_1_9_4  _T("GXUIEdit_1.9.4")       // 这个是从Wine的Edit控件修改来的
#define GXUICLASSNAME_STATIC      _T("GXUIStatic")
#define GXUICLASSNAME_SLIDER      _T("GXUISlider")
#define GXUICLASSNAME_BUTTON      _T("GXUIButton")
#define GXUICLASSNAME_LIST        _T("GXUIList")
#define GXUICLASSNAME_RICHLIST    _T("GXUIRichList")
#define GXUICLASSNAME_TOOLBAR     _T("GXUIToolbar")
#define GXUICLASSNAME_PROPSHEET   _T("UIEXTPROPSHEET")
#define GXUICLASSNAME_PROPLIST    _T("UIEXTPROPLIST")
#define GXWC_DIALOGW              DIALOG_CLASS_ATOM
#define GXWC_DIALOGEXW            DIALOG_CLASS_ATOM_EX

/*
 * Predefined Resource Types
 */
#define GXRT_CURSOR           GXMAKEINTRESOURCE(1)
#define GXRT_BITMAP           GXMAKEINTRESOURCE(2)
#define GXRT_ICON             GXMAKEINTRESOURCE(3)
#define GXRT_MENU             GXMAKEINTRESOURCE(4)
#define GXRT_DIALOG           GXMAKEINTRESOURCE(5)
#define GXRT_STRING           GXMAKEINTRESOURCE(6)
#define GXRT_FONTDIR          GXMAKEINTRESOURCE(7)
#define GXRT_FONT             GXMAKEINTRESOURCE(8)
#define GXRT_ACCELERATOR      GXMAKEINTRESOURCE(9)
#define GXRT_RCDATA           GXMAKEINTRESOURCE(10)
#define GXRT_MESSAGETABLE     GXMAKEINTRESOURCE(11)
#define GXRT_VERSION          GXMAKEINTRESOURCE(16)
#define GXRT_ANICURSOR        GXMAKEINTRESOURCE(21)
#define GXRT_ANIICON          GXMAKEINTRESOURCE(22)
#define GXDIFFERENCE          11
#define GXRT_GROUP_CURSOR     GXMAKEINTRESOURCE((GXULONG_PTR)(GXRT_CURSOR) + GXDIFFERENCE)
#define GXRT_GROUP_ICON       GXMAKEINTRESOURCE((GXULONG_PTR)(GXRT_ICON) + GXDIFFERENCE)

/*
 * Get/SetWindowWord/Long offsets for use with WC_DIALOG windows
 */
#define GXDWL_MSGRESULT   0
#define GXDWL_DLGPROC     4
#define GXDWL_USER        8
#define GXDWL_DLGLOG      12

#define GXDWLP_MSGRESULT  0
#define GXDWLP_DLGPROC    GXDWLP_MSGRESULT + sizeof(GXLRESULT)
#define GXDWLP_USER       GXDWLP_DLGPROC + sizeof(GXDLGPROC)


/* WM_PRINT flags */
#define GXPRF_CHECKVISIBLE    0x00000001L
#define GXPRF_NONCLIENT       0x00000002L
#define GXPRF_CLIENT          0x00000004L
#define GXPRF_ERASEBKGND      0x00000008L
#define GXPRF_CHILDREN        0x00000010L
#define GXPRF_OWNED           0x00000020L

/*
 * GetSystemMetrics() codes
 */
#define GXSM_CXSCREEN                     0
#define GXSM_CYSCREEN                     1
#define GXSM_CXVSCROLL                    2
#define GXSM_CYHSCROLL                    3
#define GXSM_CYCAPTION                    4
#define GXSM_CXBORDER                     5
#define GXSM_CYBORDER                     6
#define GXSM_CXDLGFRAME                   7
#define GXSM_CYDLGFRAME                   8
#define GXSM_CYVTHUMB                     9
#define GXSM_CXHTHUMB                     10
#define GXSM_CXICON                       11
#define GXSM_CYICON                       12
#define GXSM_CXCURSOR                     13
#define GXSM_CYCURSOR                     14
#define GXSM_CYMENU                       15
#define GXSM_CXFULLSCREEN                 16
#define GXSM_CYFULLSCREEN                 17
#define GXSM_CYKANJIWINDOW                18
#define GXSM_MOUSEPRESENT                 19
#define GXSM_CYVSCROLL                    20
#define GXSM_CXHSCROLL                    21
#define GXSM_DEBUG                        22
#define GXSM_SWAPBUTTON                   23
#define GXSM_RESERVED1                    24
#define GXSM_RESERVED2                    25
#define GXSM_RESERVED3                    26
#define GXSM_RESERVED4                    27
#define GXSM_CXMIN                        28
#define GXSM_CYMIN                        29
#define GXSM_CXSIZE                       30
#define GXSM_CYSIZE                       31
#define GXSM_CXFRAME                      32
#define GXSM_CYFRAME                      33
#define GXSM_CXMINTRACK                   34
#define GXSM_CYMINTRACK                   35
#define GXSM_CXDOUBLECLK                  36
#define GXSM_CYDOUBLECLK                  37
#define GXSM_CXICONSPACING                38
#define GXSM_CYICONSPACING                39
#define GXSM_MENUDROPALIGNMENT            40
#define GXSM_PENWINDOWS                   41
#define GXSM_DBCSENABLED                  42
#define GXSM_CMOUSEBUTTONS                43
#define GXSM_CXFIXEDFRAME                 GXSM_CXDLGFRAME  /* ;win40 name change */
#define GXSM_CYFIXEDFRAME                 GXSM_CYDLGFRAME  /* ;win40 name change */
#define GXSM_CXSIZEFRAME                  GXSM_CXFRAME     /* ;win40 name change */
#define GXSM_CYSIZEFRAME                  GXSM_CYFRAME     /* ;win40 name change */
#define GXSM_SECURE                       44
#define GXSM_CXEDGE                       45
#define GXSM_CYEDGE                       46
#define GXSM_CXMINSPACING                 47
#define GXSM_CYMINSPACING                 48
#define GXSM_CXSMICON                     49
#define GXSM_CYSMICON                     50
#define GXSM_CYSMCAPTION                  51
#define GXSM_CXSMSIZE                     52
#define GXSM_CYSMSIZE                     53
#define GXSM_CXMENUSIZE                   54
#define GXSM_CYMENUSIZE                   55
#define GXSM_ARRANGE                      56
#define GXSM_CXMINIMIZED                  57
#define GXSM_CYMINIMIZED                  58
#define GXSM_CXMAXTRACK                   59
#define GXSM_CYMAXTRACK                   60
#define GXSM_CXMAXIMIZED                  61
#define GXSM_CYMAXIMIZED                  62
#define GXSM_NETWORK                      63
#define GXSM_CLEANBOOT                    67
#define GXSM_CXDRAG                       68
#define GXSM_CYDRAG                       69
#define GXSM_SHOWSOUNDS                   70
#define GXSM_CXMENUCHECK                  71   /* Use instead of GetMenuCheckMarkDimensions()! */
#define GXSM_CYMENUCHECK                  72
#define GXSM_SLOWMACHINE                  73
#define GXSM_MIDEASTENABLED               74
#define GXSM_MOUSEWHEELPRESENT            75
#define GXSM_XVIRTUALSCREEN               76
#define GXSM_YVIRTUALSCREEN               77
#define GXSM_CXVIRTUALSCREEN              78
#define GXSM_CYVIRTUALSCREEN              79
#define GXSM_CMONITORS                    80
#define GXSM_SAMEDISPLAYFORMAT            81
#define GXSM_IMMENABLED                   82
#define GXSM_CXFOCUSBORDER                83
#define GXSM_CYFOCUSBORDER                84
#define GXSM_TABLETPC                     86
#define GXSM_MEDIACENTER                  87
#define GXSM_STARTER                      88
#define GXSM_SERVERR2                     89
#define GXSM_MOUSEHORIZONTALWHEELPRESENT  91
#define GXSM_CXPADDEDBORDER               92
#define GXSM_DIGITIZER                    94
#define GXSM_MAXIMUMTOUCHES               95
#define GXSM_CMETRICS                     97
#define GXSM_REMOTESESSION                0x1000
#define GXSM_SHUTTINGDOWN                 0x2000
#define GXSM_REMOTECONTROL                0x2001
#define GXSM_CARETBLINKINGENABLED         0x2002


/*
 * Color Types
 */
#define GXCTLCOLOR_MSGBOX         0
#define GXCTLCOLOR_EDIT           1
#define GXCTLCOLOR_LISTBOX        2
#define GXCTLCOLOR_BTN            3
#define GXCTLCOLOR_DLG            4
#define GXCTLCOLOR_SCROLLBAR      5
#define GXCTLCOLOR_STATIC         6
#define GXCTLCOLOR_MAX            7

#define GXCOLOR_SCROLLBAR               0
#define GXCOLOR_BACKGROUND              1
#define GXCOLOR_ACTIVECAPTION           2
#define GXCOLOR_INACTIVECAPTION         3
#define GXCOLOR_MENU                    4
#define GXCOLOR_WINDOW                  5
#define GXCOLOR_WINDOWFRAME             6
#define GXCOLOR_MENUTEXT                7
#define GXCOLOR_WINDOWTEXT              8
#define GXCOLOR_CAPTIONTEXT             9
#define GXCOLOR_ACTIVEBORDER            10
#define GXCOLOR_INACTIVEBORDER          11
#define GXCOLOR_APPWORKSPACE            12
#define GXCOLOR_HIGHLIGHT               13
#define GXCOLOR_HIGHLIGHTTEXT           14
#define GXCOLOR_BTNFACE                 15
#define GXCOLOR_BTNSHADOW               16
#define GXCOLOR_GRAYTEXT                17
#define GXCOLOR_BTNTEXT                 18
#define GXCOLOR_INACTIVECAPTIONTEXT     19
#define GXCOLOR_BTNHIGHLIGHT            20
#define GXCOLOR_3DDKSHADOW              21
#define GXCOLOR_3DLIGHT                 22
#define GXCOLOR_INFOTEXT                23
#define GXCOLOR_INFOBK                  24
#define GXCOLOR_HOTLIGHT                26
#define GXCOLOR_GRADIENTACTIVECAPTION   27
#define GXCOLOR_GRADIENTINACTIVECAPTION 28
#define GXCOLOR_MENUHILIGHT             29
#define GXCOLOR_MENUBAR                 30
#define GXCOLOR_DESKTOP                 GXCOLOR_BACKGROUND
#define GXCOLOR_3DFACE                  GXCOLOR_BTNFACE
#define GXCOLOR_3DSHADOW                GXCOLOR_BTNSHADOW
#define GXCOLOR_3DHIGHLIGHT             GXCOLOR_BTNHIGHLIGHT
#define GXCOLOR_3DHILIGHT               GXCOLOR_BTNHIGHLIGHT
#define GXCOLOR_BTNHILIGHT              GXCOLOR_BTNHIGHLIGHT

#define GXDI_MASK         0x0001
#define GXDI_IMAGE        0x0002
#define GXDI_NORMAL       0x0003
#define GXDI_COMPAT       0x0004
#define GXDI_DEFAULTSIZE  0x0008
#define GXDI_NOMIRROR     0x0010

//
// gxExtTextOutW
//
#define GXETO_OPAQUE                   0x0002
#define GXETO_CLIPPED                  0x0004
#define GXETO_GLYPH_INDEX              0x0010
#define GXETO_RTLREADING               0x0080
#define GXETO_NUMERICSLOCAL            0x0400
#define GXETO_NUMERICSLATIN            0x0800
#define GXETO_IGNORELANGUAGE           0x1000
#define GXETO_PDY                      0x2000
#define GXETO_REVERSE_INDEX_MAP        0x10000

/* flags for DrawFrameControl */

#define GXDFC_CAPTION             1
#define GXDFC_MENU                2
#define GXDFC_SCROLL              3
#define GXDFC_BUTTON              4
#define GXDFC_POPUPMENU           5

#define GXDFCS_CAPTIONCLOSE         0x0000
#define GXDFCS_CAPTIONMIN           0x0001
#define GXDFCS_CAPTIONMAX           0x0002
#define GXDFCS_CAPTIONRESTORE       0x0003
#define GXDFCS_CAPTIONHELP          0x0004

#define GXDFCS_MENUARROW            0x0000
#define GXDFCS_MENUCHECK            0x0001
#define GXDFCS_MENUBULLET           0x0002
#define GXDFCS_MENUARROWRIGHT       0x0004
#define GXDFCS_SCROLLUP             0x0000
#define GXDFCS_SCROLLDOWN           0x0001
#define GXDFCS_SCROLLLEFT           0x0002
#define GXDFCS_SCROLLRIGHT          0x0003
#define GXDFCS_SCROLLCOMBOBOX       0x0005
#define GXDFCS_SCROLLSIZEGRIP       0x0008
#define GXDFCS_SCROLLSIZEGRIPRIGHT  0x0010

#define GXDFCS_BUTTONCHECK          0x0000
#define GXDFCS_BUTTONRADIOIMAGE     0x0001
#define GXDFCS_BUTTONRADIOMASK      0x0002
#define GXDFCS_BUTTONRADIO          0x0004
#define GXDFCS_BUTTON3STATE         0x0008
#define GXDFCS_BUTTONPUSH           0x0010

#define GXDFCS_INACTIVE             0x0100
#define GXDFCS_PUSHED               0x0200
#define GXDFCS_CHECKED              0x0400

#define GXDFCS_TRANSPARENT          0x0800
#define GXDFCS_HOT                  0x1000

#define GXDFCS_ADJUSTRECT           0x2000
#define GXDFCS_FLAT                 0x4000
#define GXDFCS_MONO                 0x8000


#define GXSW_SCROLLCHILDREN   0x0001  /* Scroll children within *lprcScroll. */
#define GXSW_INVALIDATE       0x0002  /* Invalidate after scrolling */
#define GXSW_ERASE            0x0004  /* If SW_INVALIDATE, don't send WM_ERASEBACKGROUND */
#define GXSW_SMOOTHSCROLL     0x0010  /* Use smooth scrolling */

#define GXNFR_ANSI                             1
#define GXNFR_UNICODE                          2
#define GXNF_QUERY                             3
#define GXNF_REQUERY                           4

/*
 * Static Control Constants
 */
#define GXSS_LEFT             0x00000000L
#define GXSS_CENTER           0x00000001L
#define GXSS_RIGHT            0x00000002L
#define GXSS_ICON             0x00000003L
#define GXSS_BLACKRECT        0x00000004L
#define GXSS_GRAYRECT         0x00000005L
#define GXSS_WHITERECT        0x00000006L
#define GXSS_BLACKFRAME       0x00000007L
#define GXSS_GRAYFRAME        0x00000008L
#define GXSS_WHITEFRAME       0x00000009L
#define GXSS_USERITEM         0x0000000AL
#define GXSS_SIMPLE           0x0000000BL
#define GXSS_LEFTNOWORDWRAP   0x0000000CL
#define GXSS_OWNERDRAW        0x0000000DL
#define GXSS_BITMAP           0x0000000EL
#define GXSS_ENHMETAFILE      0x0000000FL
#define GXSS_ETCHEDHORZ       0x00000010L
#define GXSS_ETCHEDVERT       0x00000011L
#define GXSS_ETCHEDFRAME      0x00000012L
#define GXSS_TYPEMASK         0x0000001FL
#define GXSS_REALSIZECONTROL  0x00000040L
#define GXSS_NOPREFIX         0x00000080L /* Don't do "&" character translation */
#define GXSS_NOTIFY           0x00000100L
#define GXSS_CENTERIMAGE      0x00000200L
#define GXSS_RIGHTJUST        0x00000400L
#define GXSS_REALSIZEIMAGE    0x00000800L
#define GXSS_SUNKEN           0x00001000L
#define GXSS_EDITCONTROL      0x00002000L
#define GXSS_ENDELLIPSIS      0x00004000L
#define GXSS_PATHELLIPSIS     0x00008000L
#define GXSS_WORDELLIPSIS     0x0000C000L
#define GXSS_ELLIPSISMASK     0x0000C000L

//
// Static Control Notification Mesages
//
#define GXSTN_CLICKED         0
#define GXSTN_DBLCLK          1
#define GXSTN_ENABLE          2
#define GXSTN_DISABLE         3
#define GXSTM_MSGMAX          0x0174

//
// List Box Creation struct
//

//struct LISTBOXCREATIONPARAMW
//{
//  GXUINT    cbSize;
//  GXLPCWSTR szTemplate;
//};
//
//typedef LISTBOXCREATIONPARAMW LISTBOXCREATIONPARAM;

/*
 * Listbox Return Values
 */
#define GXLB_OKAY             0
#define GXLB_ERR              (-1)
#define GXLB_ERRSPACE         (-2)

/*
 * Listbox Notification Codes
 */
#define GXLBN_ERRSPACE        (-2)
#define GXLBN_SELCHANGE       1
#define GXLBN_DBLCLK          2
#define GXLBN_SELCANCEL       3
#define GXLBN_SETFOCUS        4
#define GXLBN_KILLFOCUS       5

// FIXME: 这个枚举值似乎会和其他WM_NOTIFY消息重复了
#define GXLBN_CUSTCTRLCMD     6 // GXUI 扩展: WM_NOTIFY消息, 自定义列表的控件消息，这个是透传控件的WM_COMMAND消息， 结构体:GXNMCUSTLISTCTRLCMD
#define GXLBN_CREATEADAPTER   7 // GXUI 扩展: WM_NOTIFY消息, 创建适配器通知,结构体:GXNMLISTADAPTER
#define GXLBN_ADAPTERCHANGED  8 // GXUI 扩展: WM_NOTIFY消息, 适配器被重新设置,结构体:GXNMLISTADAPTER


//
// GXLB_SETCOLOR
//
#define GXLBSC_BACKGROUND     0
#define GXLBSC_TEXT           1
#define GXLBSC_HIGHTLIGHT     2
#define GXLBSC_HIGHTLIGHTTEXT 3

//
// Listbox Styles
//
#define GXLBS_NOTIFY            0x0001L
#define GXLBS_SORT              0x0002L
#define GXLBS_NOREDRAW          0x0004L
#define GXLBS_MULTIPLESEL       0x0008L
#define GXLBS_OWNERDRAWFIXED    0x0010L
#define GXLBS_OWNERDRAWVARIABLE 0x0020L
#define GXLBS_HASSTRINGS        0x0040L
#define GXLBS_USETABSTOPS       0x0080L
#define GXLBS_NOINTEGRALHEIGHT  0x0100L
#define GXLBS_MULTICOLUMN       0x0200L
#define GXLBS_WANTKEYBOARDINPUT 0x0400L
#define GXLBS_EXTENDEDSEL       0x0800L
#define GXLBS_DISABLENOSCROLL   0x1000L
#define GXLBS_NODATA            0x2000L
#define GXLBS_NOSEL             0x4000L
#define GXLBS_COMBOBOX          0x8000L
#define GXLBS_LTRSCROLLED       0x0040L // left to right scrolled 与GXLBS_HASSTRINGS标志重合，因为rich list(customized list box)没有这个属性
#define GXLBS_STANDARD          (GXLBS_NOTIFY | GXLBS_SORT | GXWS_VSCROLL | GXWS_BORDER)

/*
 * Combo Box styles
 */
#define GXCBS_SIMPLE            0x0001L
#define GXCBS_DROPDOWN          0x0002L
#define GXCBS_DROPDOWNLIST      0x0003L
#define GXCBS_OWNERDRAWFIXED    0x0010L
#define GXCBS_OWNERDRAWVARIABLE 0x0020L
#define GXCBS_AUTOHSCROLL       0x0040L
#define GXCBS_OEMCONVERT        0x0080L
#define GXCBS_SORT              0x0100L
#define GXCBS_HASSTRINGS        0x0200L
#define GXCBS_NOINTEGRALHEIGHT  0x0400L
#define GXCBS_DISABLENOSCROLL   0x0800L
#define GXCBS_UPPERCASE         0x2000L
#define GXCBS_LOWERCASE         0x4000L

/*
 * Edit Control Styles
 */
#define GXES_LEFT             0x0000L
#define GXES_CENTER           0x0001L
#define GXES_RIGHT            0x0002L
#define GXES_MULTILINE        0x0004L
#define GXES_UPPERCASE        0x0008L
#define GXES_LOWERCASE        0x0010L
#define GXES_PASSWORD         0x0020L
#define GXES_AUTOVSCROLL      0x0040L
#define GXES_AUTOHSCROLL      0x0080L
#define GXES_NOHIDESEL        0x0100L
#define GXES_OEMCONVERT       0x0400L
#define GXES_READONLY         0x0800L
#define GXES_WANTRETURN       0x1000L
#define GXES_NUMBER           0x2000L

/*
 * EDITWORDBREAKPROC code values
 */
#define GXWB_LEFT            0
#define GXWB_RIGHT           1
#define GXWB_ISDELIMITER     2



/*
 * Edit Control Notification Codes
 */
#define GXEN_SETFOCUS         0x0100
#define GXEN_KILLFOCUS        0x0200
#define GXEN_CHANGE           0x0300
#define GXEN_UPDATE           0x0400
#define GXEN_ERRSPACE         0x0500
#define GXEN_MAXTEXT          0x0501
#define GXEN_HSCROLL          0x0601
#define GXEN_VSCROLL          0x0602

#define GXEN_ALIGN_LTR_EC     0x0700
#define GXEN_ALIGN_RTL_EC     0x0701

#define GXEC_LEFTMARGIN       0x0001
#define GXEC_RIGHTMARGIN      0x0002
#define GXEC_USEFONTINFO      0xffff


typedef int (GXCALLBACK* GXEDITWORDBREAKPROCA)(GXLPSTR lpch, int ichCurrent, int cch, int code);
typedef int (GXCALLBACK* GXEDITWORDBREAKPROCW)(GXLPWSTR lpch, int ichCurrent, int cch, int code);

//#ifdef _UNICODE
//#define GXEDITWORDBREAKPROC GXEDITWORDBREAKPROCW
//#else
//#define GXEDITWORDBREAKPROC GXEDITWORDBREAKPROCA
//#endif // #ifdef _UNICODE

/*
 * Button Control Styles
 */
#define GXBS_PUSHBUTTON       0x00000000L
#define GXBS_DEFPUSHBUTTON    0x00000001L
#define GXBS_CHECKBOX         0x00000002L
#define GXBS_AUTOCHECKBOX     0x00000003L
#define GXBS_RADIOBUTTON      0x00000004L
#define GXBS_3STATE           0x00000005L
#define GXBS_AUTO3STATE       0x00000006L
#define GXBS_GROUPBOX         0x00000007L
#define GXBS_USERBUTTON       0x00000008L
#define GXBS_AUTORADIOBUTTON  0x00000009L
#define GXBS_PUSHBOX          0x0000000AL
#define GXBS_OWNERDRAW        0x0000000BL
#define GXBS_TYPEMASK         0x0000000FL
#define GXBS_LEFTTEXT         0x00000020L
#define GXBS_TEXT             0x00000000L
#define GXBS_ICON             0x00000040L
#define GXBS_BITMAP           0x00000080L
#define GXBS_LEFT             0x00000100L
#define GXBS_RIGHT            0x00000200L
#define GXBS_CENTER           0x00000300L
#define GXBS_TOP              0x00000400L
#define GXBS_BOTTOM           0x00000800L
#define GXBS_VCENTER          0x00000C00L
#define GXBS_PUSHLIKE         0x00001000L
#define GXBS_MULTILINE        0x00002000L
#define GXBS_NOTIFY           0x00004000L
#define GXBS_FLAT             0x00008000L
#define GXBS_RIGHTBUTTON      GXBS_LEFTTEXT


#define GXBST_UNCHECKED      0x0000
#define GXBST_CHECKED        0x0001
#define GXBST_INDETERMINATE  0x0002
#define GXBST_PUSHED         0x0004
#define GXBST_FOCUS          0x0008

/*
 * User Button Notification Codes
 */
#define GXBN_CLICKED          0
#define GXBN_PAINT            1
#define GXBN_HILITE           2
#define GXBN_UNHILITE         3
#define GXBN_DISABLE          4
#define GXBN_DOUBLECLICKED    5
#define GXBN_PUSHED           GXBN_HILITE
#define GXBN_UNPUSHED         GXBN_UNHILITE
#define GXBN_DBLCLK           GXBN_DOUBLECLICKED
#define GXBN_SETFOCUS         6
#define GXBN_KILLFOCUS        7

/*
 * Menu flags for Add/Check/EnableMenuItem()
 */
#define GXMF_INSERT           0x00000000L
#define GXMF_CHANGE           0x00000080L
#define GXMF_APPEND           0x00000100L
#define GXMF_DELETE           0x00000200L
#define GXMF_REMOVE           0x00001000L

#define GXMF_BYCOMMAND        0x00000000L
#define GXMF_BYPOSITION       0x00000400L

#define GXMF_SEPARATOR        0x00000800L

#define GXMF_ENABLED          0x00000000L
#define GXMF_GRAYED           0x00000001L
#define GXMF_DISABLED         0x00000002L

#define GXMF_UNCHECKED        0x00000000L
#define GXMF_CHECKED          0x00000008L
#define GXMF_USECHECKBITMAPS  0x00000200L

#define GXMF_STRING           0x00000000L
#define GXMF_BITMAP           0x00000004L
#define GXMF_OWNERDRAW        0x00000100L

#define GXMF_POPUP            0x00000010L
#define GXMF_MENUBARBREAK     0x00000020L
#define GXMF_MENUBREAK        0x00000040L

#define GXMF_UNHILITE         0x00000000L
#define GXMF_HILITE           0x00000080L

#define GXMF_DEFAULT          0x00001000L
#define GXMF_SYSMENU          0x00002000L
#define GXMF_HELP             0x00004000L
#define GXMF_RIGHTJUSTIFY     0x00004000L

#define GXMF_MOUSESELECT      0x00008000L
#define GXMF_END              0x00000080L  /* Obsolete -- only used by old RES files */


#define GXMFT_STRING          GXMF_STRING
#define GXMFT_BITMAP          GXMF_BITMAP
#define GXMFT_MENUBARBREAK    GXMF_MENUBARBREAK
#define GXMFT_MENUBREAK       GXMF_MENUBREAK
#define GXMFT_OWNERDRAW       GXMF_OWNERDRAW
#define GXMFT_RADIOCHECK      0x00000200L
#define GXMFT_SEPARATOR       GXMF_SEPARATOR
#define GXMFT_RIGHTORDER      0x00002000L
#define GXMFT_RIGHTJUSTIFY    GXMF_RIGHTJUSTIFY

/* Menu flags for Add/Check/EnableMenuItem() */
#define GXMFS_GRAYED          0x00000003L
#define GXMFS_DISABLED        GXMFS_GRAYED
#define GXMFS_CHECKED         GXMF_CHECKED
#define GXMFS_HILITE          GXMF_HILITE
#define GXMFS_ENABLED         GXMF_ENABLED
#define GXMFS_UNCHECKED       GXMF_UNCHECKED
#define GXMFS_UNHILITE        GXMF_UNHILITE
#define GXMFS_DEFAULT         GXMF_DEFAULT

/*
 * System Menu Command Values
 */
#define GXSC_SIZE         0xF000
#define GXSC_MOVE         0xF010
#define GXSC_MINIMIZE     0xF020
#define GXSC_MAXIMIZE     0xF030
#define GXSC_NEXTWINDOW   0xF040
#define GXSC_PREVWINDOW   0xF050
#define GXSC_CLOSE        0xF060
#define GXSC_VSCROLL      0xF070
#define GXSC_HSCROLL      0xF080
#define GXSC_MOUSEMENU    0xF090
#define GXSC_KEYMENU      0xF100
#define GXSC_ARRANGE      0xF110
#define GXSC_RESTORE      0xF120
#define GXSC_TASKLIST     0xF130
#define GXSC_SCREENSAVE   0xF140
#define GXSC_HOTKEY       0xF150
#define GXSC_DEFAULT      0xF160
#define GXSC_MONITORPOWER 0xF170
#define GXSC_CONTEXTHELP  0xF180
#define GXSC_SEPARATOR    0xF00F

#define GXSCF_ISSECURE    0x00000001

#define GXGET_SC_WPARAM(wParam) ((int)wParam & 0xFFF0)

#define GXMNS_NOCHECK         0x80000000
#define GXMNS_MODELESS        0x40000000
#define GXMNS_DRAGDROP        0x20000000
#define GXMNS_AUTODISMISS     0x10000000
#define GXMNS_NOTIFYBYPOS     0x08000000
#define GXMNS_CHECKORBMP      0x04000000

#define GXMIM_MAXHEIGHT               0x00000001
#define GXMIM_BACKGROUND              0x00000002
#define GXMIM_HELPID                  0x00000004
#define GXMIM_MENUDATA                0x00000008
#define GXMIM_STYLE                   0x00000010
#define GXMIM_APPLYTOSUBMENUS         0x80000000

#define GXMIIM_STATE          0x00000001
#define GXMIIM_ID             0x00000002
#define GXMIIM_SUBMENU        0x00000004
#define GXMIIM_CHECKMARKS     0x00000008
#define GXMIIM_TYPE           0x00000010
#define GXMIIM_DATA           0x00000020

#define GXMIIM_STRING         0x00000040
#define GXMIIM_BITMAP         0x00000080
#define GXMIIM_FTYPE          0x00000100

#define GXHBMMENU_CALLBACK            ((GXHBITMAP) -1)
#define GXHBMMENU_SYSTEM              ((GXHBITMAP)  1)
#define GXHBMMENU_MBAR_RESTORE        ((GXHBITMAP)  2)
#define GXHBMMENU_MBAR_MINIMIZE       ((GXHBITMAP)  3)
#define GXHBMMENU_MBAR_CLOSE          ((GXHBITMAP)  5)
#define GXHBMMENU_MBAR_CLOSE_D        ((GXHBITMAP)  6)
#define GXHBMMENU_MBAR_MINIMIZE_D     ((GXHBITMAP)  7)
#define GXHBMMENU_POPUP_CLOSE         ((GXHBITMAP)  8)
#define GXHBMMENU_POPUP_RESTORE       ((GXHBITMAP)  9)
#define GXHBMMENU_POPUP_MAXIMIZE      ((GXHBITMAP) 10)
#define GXHBMMENU_POPUP_MINIMIZE      ((GXHBITMAP) 11)

/*
 * Flags for TrackPopupMenu
 */
#define GXTPM_LEFTBUTTON      0x0000L
#define GXTPM_RIGHTBUTTON     0x0002L
#define GXTPM_LEFTALIGN       0x0000L
#define GXTPM_CENTERALIGN     0x0004L
#define GXTPM_RIGHTALIGN      0x0008L
#define GXTPM_TOPALIGN        0x0000L
#define GXTPM_VCENTERALIGN    0x0010L
#define GXTPM_BOTTOMALIGN     0x0020L

#define GXTPM_HORIZONTAL      0x0000L     /* Horz alignment matters more */
#define GXTPM_VERTICAL        0x0040L     /* Vert alignment matters more */
#define GXTPM_NONOTIFY        0x0080L     /* Don't send any notification msgs */
#define GXTPM_RETURNCMD       0x0100L
#define GXTPM_RECURSE         0x0001L
#define GXTPM_HORPOSANIMATION 0x0400L
#define GXTPM_HORNEGANIMATION 0x0800L
#define GXTPM_VERPOSANIMATION 0x1000L
#define GXTPM_VERNEGANIMATION 0x2000L
#define GXTPM_NOANIMATION     0x4000L
#define GXTPM_LAYOUTRTL       0x8000L
#define GXTPM_WORKAREA        0x10000L

/*
 * Obsolete names
 */
#define GXSC_ICON         GXSC_MINIMIZE
#define GXSC_ZOOM         GXSC_MAXIMIZE


typedef struct GXMENUINFO
{
  GXDWORD       cbSize;
  GXDWORD       fMask;
  GXDWORD       dwStyle;
  GXUINT        cyMax;
  GXHBRUSH      hbrBack;
  GXDWORD       dwContextHelpID;
  GXULONG_PTR   dwMenuData;
}*GXLPMENUINFO, *LPGXMENUINFO;
typedef GXMENUINFO GXCONST *GXLPCMENUINFO;

/*
 * Menubar information
 */
struct GXMENUBARINFO
{
    GXDWORD cbSize;
    GXRECT  rcBar;          // rect of bar, popup, item
    GXHMENU hMenu;         // real menu handle of bar, popup
    GXHWND  hwndMenu;       // hwnd of item submenu if one
    GXBOOL  fBarFocused:1;  // bar, popup has the focus
    GXBOOL  fFocused:1;     // item has the focus
};
typedef GXMENUBARINFO *GXLPMENUBARINFO;
typedef GXMENUBARINFO *LPGXMENUBARINFO;

struct GXTPMPARAMS
{
  GXUINT    cbSize;     /* Size of structure */
  GXRECT    rcExclude;  /* Screen coordinates of rectangle to exclude when positioning */
};
typedef GXTPMPARAMS *GXLPTPMPARAMS;
typedef GXTPMPARAMS *LPGXTPMPARAMS;


// GetMenuDefaultItem() 参数
#define GXGMDI_USEDISABLED    0x0001L
#define GXGMDI_GOINTOPOPUPS   0x0002L

/*
 * Dialog Codes
 */
#define GXDLGC_WANTARROWS       0x0001      /* Control wants arrow keys         */
#define GXDLGC_WANTTAB          0x0002      /* Control wants tab keys           */
#define GXDLGC_WANTALLKEYS      0x0004      /* Control wants all keys           */
#define GXDLGC_WANTMESSAGE      0x0004      /* Pass message to control          */
#define GXDLGC_HASSETSEL        0x0008      /* Understands EM_SETSEL message    */
#define GXDLGC_DEFPUSHBUTTON    0x0010      /* Default pushbutton               */
#define GXDLGC_UNDEFPUSHBUTTON  0x0020      /* Non-default pushbutton           */
#define GXDLGC_RADIOBUTTON      0x0040      /* Radio button                     */
#define GXDLGC_WANTCHARS        0x0080      /* Want WM_CHAR messages            */
#define GXDLGC_STATIC           0x0100      /* Static item: don't include       */
#define GXDLGC_BUTTON           0x2000      /* Button item: can be checked      */


/*
 * PeekMessage() Options
 */
#define GXPM_NOREMOVE           0x0000
#define GXPM_REMOVE             0x0001
#define GXPM_NOYIELD            0x0002
//#define GXPM_QS_INPUT         (QS_INPUT << 16)
//#define GXPM_QS_POSTMESSAGE   ((QS_POSTMESSAGE | QS_HOTKEY | QS_TIMER) << 16)
//#define GXPM_QS_PAINT         (QS_PAINT << 16)
//#define GXPM_QS_SENDMESSAGE   (QS_SENDMESSAGE << 16)

/*
 * WH_MSGFILTER Filter Proc Codes
 */
#define GXMSGF_DIALOGBOX      0
#define GXMSGF_MESSAGEBOX     1
#define GXMSGF_MENU           2
#define GXMSGF_SCROLLBAR      5
#define GXMSGF_NEXTWINDOW     6
#define GXMSGF_MAX            8                       // unused
#define GXMSGF_USER           4096

/*
 * WM_MOUSEACTIVATE Return Codes
 */
#define GXMA_ACTIVATE         1
#define GXMA_ACTIVATEANDEAT   2
#define GXMA_NOACTIVATE       3
#define GXMA_NOACTIVATEANDEAT 4

/*
 * The "real" ancestor window
 */
#define     GXGA_PARENT       1
#define     GXGA_ROOT         2
#define     GXGA_ROOTOWNER    3

/*
 * Parameter for SystemParametersInfo.
 */

#define SPI_GETBEEP                         0x0001
#define SPI_SETBEEP                         0x0002
#define SPI_GETMOUSE                        0x0003
#define SPI_SETMOUSE                        0x0004
#define SPI_GETBORDER                       0x0005
#define SPI_SETBORDER                       0x0006
#define SPI_GETKEYBOARDSPEED                0x000A
#define SPI_SETKEYBOARDSPEED                0x000B
#define SPI_LANGDRIVER                      0x000C
#define SPI_ICONHORIZONTALSPACING           0x000D
#define SPI_GETSCREENSAVETIMEOUT            0x000E
#define SPI_SETSCREENSAVETIMEOUT            0x000F
#define SPI_GETSCREENSAVEACTIVE             0x0010
#define SPI_SETSCREENSAVEACTIVE             0x0011
#define SPI_GETGRIDGRANULARITY              0x0012
#define SPI_SETGRIDGRANULARITY              0x0013
#define SPI_SETDESKWALLPAPER                0x0014
#define SPI_SETDESKPATTERN                  0x0015
#define SPI_GETKEYBOARDDELAY                0x0016
#define SPI_SETKEYBOARDDELAY                0x0017
#define SPI_ICONVERTICALSPACING             0x0018
#define SPI_GETICONTITLEWRAP                0x0019
#define SPI_SETICONTITLEWRAP                0x001A
#define SPI_GETMENUDROPALIGNMENT            0x001B
#define SPI_SETMENUDROPALIGNMENT            0x001C
#define SPI_SETDOUBLECLKWIDTH               0x001D
#define SPI_SETDOUBLECLKHEIGHT              0x001E
#define SPI_GETICONTITLELOGFONT             0x001F
#define SPI_SETDOUBLECLICKTIME              0x0020
#define SPI_SETMOUSEBUTTONSWAP              0x0021
#define SPI_SETICONTITLELOGFONT             0x0022
#define SPI_GETFASTTASKSWITCH               0x0023
#define SPI_SETFASTTASKSWITCH               0x0024

#define SPI_SETDRAGFULLWINDOWS              0x0025
#define SPI_GETDRAGFULLWINDOWS              0x0026
#define SPI_GETNONCLIENTMETRICS             0x0029
#define SPI_SETNONCLIENTMETRICS             0x002A
#define SPI_GETMINIMIZEDMETRICS             0x002B
#define SPI_SETMINIMIZEDMETRICS             0x002C
#define SPI_GETICONMETRICS                  0x002D
#define SPI_SETICONMETRICS                  0x002E
#define SPI_SETWORKAREA                     0x002F
#define SPI_GETWORKAREA                     0x0030
#define SPI_SETPENWINDOWS                   0x0031

#define SPI_GETHIGHCONTRAST                 0x0042
#define SPI_SETHIGHCONTRAST                 0x0043
#define SPI_GETKEYBOARDPREF                 0x0044
#define SPI_SETKEYBOARDPREF                 0x0045
#define SPI_GETSCREENREADER                 0x0046
#define SPI_SETSCREENREADER                 0x0047
#define SPI_GETANIMATION                    0x0048
#define SPI_SETANIMATION                    0x0049
#define SPI_GETFONTSMOOTHING                0x004A
#define SPI_SETFONTSMOOTHING                0x004B
#define SPI_SETDRAGWIDTH                    0x004C
#define SPI_SETDRAGHEIGHT                   0x004D
#define SPI_SETHANDHELD                     0x004E
#define SPI_GETLOWPOWERTIMEOUT              0x004F
#define SPI_GETPOWEROFFTIMEOUT              0x0050
#define SPI_SETLOWPOWERTIMEOUT              0x0051
#define SPI_SETPOWEROFFTIMEOUT              0x0052
#define SPI_GETLOWPOWERACTIVE               0x0053
#define SPI_GETPOWEROFFACTIVE               0x0054
#define SPI_SETLOWPOWERACTIVE               0x0055
#define SPI_SETPOWEROFFACTIVE               0x0056
#define SPI_SETCURSORS                      0x0057
#define SPI_SETICONS                        0x0058
#define SPI_GETDEFAULTINPUTLANG             0x0059
#define SPI_SETDEFAULTINPUTLANG             0x005A
#define SPI_SETLANGTOGGLE                   0x005B
#define SPI_GETWINDOWSEXTENSION             0x005C
#define SPI_SETMOUSETRAILS                  0x005D
#define SPI_GETMOUSETRAILS                  0x005E
#define SPI_SETSCREENSAVERRUNNING           0x0061
#define SPI_SCREENSAVERRUNNING              SPI_SETSCREENSAVERRUNNING

#define SPI_GETFILTERKEYS                   0x0032
#define SPI_SETFILTERKEYS                   0x0033
#define SPI_GETTOGGLEKEYS                   0x0034
#define SPI_SETTOGGLEKEYS                   0x0035
#define SPI_GETMOUSEKEYS                    0x0036
#define SPI_SETMOUSEKEYS                    0x0037
#define SPI_GETSHOWSOUNDS                   0x0038
#define SPI_SETSHOWSOUNDS                   0x0039
#define SPI_GETSTICKYKEYS                   0x003A
#define SPI_SETSTICKYKEYS                   0x003B
#define SPI_GETACCESSTIMEOUT                0x003C
#define SPI_SETACCESSTIMEOUT                0x003D

#define SPI_GETSERIALKEYS                   0x003E
#define SPI_SETSERIALKEYS                   0x003F

#define SPI_GETSOUNDSENTRY                  0x0040
#define SPI_SETSOUNDSENTRY                  0x0041

#define SPI_GETSNAPTODEFBUTTON              0x005F
#define SPI_SETSNAPTODEFBUTTON              0x0060
#define SPI_GETMOUSEHOVERWIDTH              0x0062
#define SPI_SETMOUSEHOVERWIDTH              0x0063
#define SPI_GETMOUSEHOVERHEIGHT             0x0064
#define SPI_SETMOUSEHOVERHEIGHT             0x0065
#define SPI_GETMOUSEHOVERTIME               0x0066
#define SPI_SETMOUSEHOVERTIME               0x0067
#define SPI_GETWHEELSCROLLLINES             0x0068
#define SPI_SETWHEELSCROLLLINES             0x0069
#define SPI_GETMENUSHOWDELAY                0x006A
#define SPI_SETMENUSHOWDELAY                0x006B

#define SPI_GETWHEELSCROLLCHARS             0x006C
#define SPI_SETWHEELSCROLLCHARS             0x006D

#define SPI_GETSHOWIMEUI                    0x006E
#define SPI_SETSHOWIMEUI                    0x006F

#define SPI_GETMOUSESPEED                   0x0070
#define SPI_SETMOUSESPEED                   0x0071
#define SPI_GETSCREENSAVERRUNNING           0x0072
#define SPI_GETDESKWALLPAPER                0x0073

#define SPI_GETAUDIODESCRIPTION             0x0074
#define SPI_SETAUDIODESCRIPTION             0x0075

#define SPI_GETSCREENSAVESECURE             0x0076
#define SPI_SETSCREENSAVESECURE             0x0077

#define SPI_GETHUNGAPPTIMEOUT               0x0078
#define SPI_SETHUNGAPPTIMEOUT               0x0079
#define SPI_GETWAITTOKILLTIMEOUT            0x007A
#define SPI_SETWAITTOKILLTIMEOUT            0x007B
#define SPI_GETWAITTOKILLSERVICETIMEOUT     0x007C
#define SPI_SETWAITTOKILLSERVICETIMEOUT     0x007D
#define SPI_GETMOUSEDOCKTHRESHOLD           0x007E
#define SPI_SETMOUSEDOCKTHRESHOLD           0x007F
#define SPI_GETPENDOCKTHRESHOLD             0x0080
#define SPI_SETPENDOCKTHRESHOLD             0x0081
#define SPI_GETWINARRANGING                 0x0082
#define SPI_SETWINARRANGING                 0x0083
#define SPI_GETMOUSEDRAGOUTTHRESHOLD        0x0084
#define SPI_SETMOUSEDRAGOUTTHRESHOLD        0x0085
#define SPI_GETPENDRAGOUTTHRESHOLD          0x0086
#define SPI_SETPENDRAGOUTTHRESHOLD          0x0087
#define SPI_GETMOUSESIDEMOVETHRESHOLD       0x0088
#define SPI_SETMOUSESIDEMOVETHRESHOLD       0x0089
#define SPI_GETPENSIDEMOVETHRESHOLD         0x008A
#define SPI_SETPENSIDEMOVETHRESHOLD         0x008B
#define SPI_GETDRAGFROMMAXIMIZE             0x008C
#define SPI_SETDRAGFROMMAXIMIZE             0x008D
#define SPI_GETSNAPSIZING                   0x008E
#define SPI_SETSNAPSIZING                   0x008F
#define SPI_GETDOCKMOVING                   0x0090
#define SPI_SETDOCKMOVING                   0x0091

#define SPI_GETACTIVEWINDOWTRACKING         0x1000
#define SPI_SETACTIVEWINDOWTRACKING         0x1001
#define SPI_GETMENUANIMATION                0x1002
#define SPI_SETMENUANIMATION                0x1003
#define SPI_GETCOMBOBOXANIMATION            0x1004
#define SPI_SETCOMBOBOXANIMATION            0x1005
#define SPI_GETLISTBOXSMOOTHSCROLLING       0x1006
#define SPI_SETLISTBOXSMOOTHSCROLLING       0x1007
#define SPI_GETGRADIENTCAPTIONS             0x1008
#define SPI_SETGRADIENTCAPTIONS             0x1009
#define SPI_GETKEYBOARDCUES                 0x100A
#define SPI_SETKEYBOARDCUES                 0x100B
#define SPI_GETMENUUNDERLINES               SPI_GETKEYBOARDCUES
#define SPI_SETMENUUNDERLINES               SPI_SETKEYBOARDCUES
#define SPI_GETACTIVEWNDTRKZORDER           0x100C
#define SPI_SETACTIVEWNDTRKZORDER           0x100D
#define SPI_GETHOTTRACKING                  0x100E
#define SPI_SETHOTTRACKING                  0x100F
#define SPI_GETMENUFADE                     0x1012
#define SPI_SETMENUFADE                     0x1013
#define SPI_GETSELECTIONFADE                0x1014
#define SPI_SETSELECTIONFADE                0x1015
#define SPI_GETTOOLTIPANIMATION             0x1016
#define SPI_SETTOOLTIPANIMATION             0x1017
#define SPI_GETTOOLTIPFADE                  0x1018
#define SPI_SETTOOLTIPFADE                  0x1019
#define SPI_GETCURSORSHADOW                 0x101A
#define SPI_SETCURSORSHADOW                 0x101B
#define SPI_GETMOUSESONAR                   0x101C
#define SPI_SETMOUSESONAR                   0x101D
#define SPI_GETMOUSECLICKLOCK               0x101E
#define SPI_SETMOUSECLICKLOCK               0x101F
#define SPI_GETMOUSEVANISH                  0x1020
#define SPI_SETMOUSEVANISH                  0x1021
#define SPI_GETFLATMENU                     0x1022
#define SPI_SETFLATMENU                     0x1023
#define SPI_GETDROPSHADOW                   0x1024
#define SPI_SETDROPSHADOW                   0x1025
#define SPI_GETBLOCKSENDINPUTRESETS         0x1026
#define SPI_SETBLOCKSENDINPUTRESETS         0x1027

#define SPI_GETUIEFFECTS                    0x103E
#define SPI_SETUIEFFECTS                    0x103F

#define SPI_GETDISABLEOVERLAPPEDCONTENT     0x1040
#define SPI_SETDISABLEOVERLAPPEDCONTENT     0x1041
#define SPI_GETCLIENTAREAANIMATION          0x1042
#define SPI_SETCLIENTAREAANIMATION          0x1043
#define SPI_GETCLEARTYPE                    0x1048
#define SPI_SETCLEARTYPE                    0x1049
#define SPI_GETSPEECHRECOGNITION            0x104A
#define SPI_SETSPEECHRECOGNITION            0x104B

#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#define SPI_GETACTIVEWNDTRKTIMEOUT          0x2002
#define SPI_SETACTIVEWNDTRKTIMEOUT          0x2003
#define SPI_GETFOREGROUNDFLASHCOUNT         0x2004
#define SPI_SETFOREGROUNDFLASHCOUNT         0x2005
#define SPI_GETCARETWIDTH                   0x2006
#define SPI_SETCARETWIDTH                   0x2007

#define SPI_GETMOUSECLICKLOCKTIME           0x2008
#define SPI_SETMOUSECLICKLOCKTIME           0x2009
#define SPI_GETFONTSMOOTHINGTYPE            0x200A
#define SPI_SETFONTSMOOTHINGTYPE            0x200B

/* constants for SPI_GETFONTSMOOTHINGTYPE and SPI_SETFONTSMOOTHINGTYPE: */
#define FE_FONTSMOOTHINGSTANDARD            0x0001
#define FE_FONTSMOOTHINGCLEARTYPE           0x0002

#define SPI_GETFONTSMOOTHINGCONTRAST        0x200C
#define SPI_SETFONTSMOOTHINGCONTRAST        0x200D

#define SPI_GETFOCUSBORDERWIDTH             0x200E
#define SPI_SETFOCUSBORDERWIDTH             0x200F
#define SPI_GETFOCUSBORDERHEIGHT            0x2010
#define SPI_SETFOCUSBORDERHEIGHT            0x2011

#define SPI_GETFONTSMOOTHINGORIENTATION     0x2012
#define SPI_SETFONTSMOOTHINGORIENTATION     0x2013

/* constants for SPI_GETFONTSMOOTHINGORIENTATION and SPI_SETFONTSMOOTHINGORIENTATION: */
#define FE_FONTSMOOTHINGORIENTATIONBGR   0x0000
#define FE_FONTSMOOTHINGORIENTATIONRGB   0x0001

#define SPI_GETMINIMUMHITRADIUS             0x2014
#define SPI_SETMINIMUMHITRADIUS             0x2015
#define SPI_GETMESSAGEDURATION              0x2016
#define SPI_SETMESSAGEDURATION              0x2017

/*
 * Flags
 */
#define SPIF_UPDATEINIFILE            0x0001
#define SPIF_SENDWININICHANGE         0x0002
#define SPIF_SENDCHANGE               SPIF_SENDWININICHANGE

/*
 * Standard Cursor IDs
 */
#define GXIDC_ARROW           GXMAKEINTRESOURCE(32512)
#define GXIDC_IBEAM           GXMAKEINTRESOURCE(32513)
#define GXIDC_WAIT            GXMAKEINTRESOURCE(32514)
#define GXIDC_CROSS           GXMAKEINTRESOURCE(32515)
#define GXIDC_UPARROW         GXMAKEINTRESOURCE(32516)
#define GXIDC_SIZE            GXMAKEINTRESOURCE(32640)  /* OBSOLETE: use IDC_SIZEALL */
#define GXIDC_ICON            GXMAKEINTRESOURCE(32641)  /* OBSOLETE: use IDC_ARROW */
#define GXIDC_SIZENWSE        GXMAKEINTRESOURCE(32642)
#define GXIDC_SIZENESW        GXMAKEINTRESOURCE(32643)
#define GXIDC_SIZEWE          GXMAKEINTRESOURCE(32644)
#define GXIDC_SIZENS          GXMAKEINTRESOURCE(32645)
#define GXIDC_SIZEALL         GXMAKEINTRESOURCE(32646)
#define GXIDC_NO              GXMAKEINTRESOURCE(32648) /*not in win3.1 */
#define GXIDC_HAND            GXMAKEINTRESOURCE(32649)
#define GXIDC_APPSTARTING     GXMAKEINTRESOURCE(32650) /*not in win3.1 */
#define GXIDC_HELP            GXMAKEINTRESOURCE(32651)

#define GXIMAGE_BITMAP        0
#define GXIMAGE_ICON          1
#define GXIMAGE_CURSOR        2

#define GXMONITOR_DEFAULTTONULL       0x00000000
#define GXMONITOR_DEFAULTTOPRIMARY    0x00000001
#define GXMONITOR_DEFAULTTONEAREST    0x00000002

/* Monolithic state-drawing routine */
/* Image type */
#define GXDST_COMPLEX     0x0000
#define GXDST_TEXT        0x0001
#define GXDST_PREFIXTEXT  0x0002
#define GXDST_ICON        0x0003
#define GXDST_BITMAP      0x0004

/* State type */
#define GXDSS_NORMAL      0x0000
#define GXDSS_UNION       0x0010  /* Gray string appearance */
#define GXDSS_DISABLED    0x0020
#define GXDSS_MONO        0x0080
#define GXDSS_HIDEPREFIX  0x0200
#define GXDSS_PREFIXONLY  0x0400
#define GXDSS_RIGHT       0x8000


#define GXIMAGE_BITMAP        0
#define GXIMAGE_ICON          1
#define GXIMAGE_CURSOR        2
#define GXIMAGE_ENHMETAFILE   3

#define GXLR_DEFAULTCOLOR     0x00000000
#define GXLR_MONOCHROME       0x00000001
#define GXLR_COLOR            0x00000002
#define GXLR_COPYRETURNORG    0x00000004
#define GXLR_COPYDELETEORG    0x00000008
#define GXLR_LOADFROMFILE     0x00000010
#define GXLR_LOADTRANSPARENT  0x00000020
#define GXLR_DEFAULTSIZE      0x00000040
#define GXLR_VGACOLOR         0x00000080
#define GXLR_LOADMAP3DCOLORS  0x00001000
#define GXLR_CREATEDIBSECTION 0x00002000
#define GXLR_COPYFROMRESOURCE 0x00004000
#define GXLR_SHARED           0x00008000

/*
 * RedrawWindow() flags
 */
#define GXRDW_INVALIDATE          0x0001
#define GXRDW_INTERNALPAINT       0x0002
#define GXRDW_ERASE               0x0004

#define GXRDW_VALIDATE            0x0008
#define GXRDW_NOINTERNALPAINT     0x0010
#define GXRDW_NOERASE             0x0020

#define GXRDW_NOCHILDREN          0x0040
#define GXRDW_ALLCHILDREN         0x0080

#define GXRDW_UPDATENOW           0x0100
#define GXRDW_ERASENOW            0x0200

#define GXRDW_FRAME               0x0400
#define GXRDW_NOFRAME             0x0800

//////////////////////////////////////////////////////////////////////////
/* 3D border styles */
#define GXBDR_RAISEDOUTER 0x0001
#define GXBDR_SUNKENOUTER 0x0002
#define GXBDR_RAISEDINNER 0x0004
#define GXBDR_SUNKENINNER 0x0008

#define GXBDR_OUTER       (GXBDR_RAISEDOUTER | GXBDR_SUNKENOUTER)
#define GXBDR_INNER       (GXBDR_RAISEDINNER | GXBDR_SUNKENINNER)
#define GXBDR_RAISED      (GXBDR_RAISEDOUTER | GXBDR_RAISEDINNER)
#define GXBDR_SUNKEN      (GXBDR_SUNKENOUTER | GXBDR_SUNKENINNER)


#define GXEDGE_RAISED     (GXBDR_RAISEDOUTER | GXBDR_RAISEDINNER)
#define GXEDGE_SUNKEN     (GXBDR_SUNKENOUTER | GXBDR_SUNKENINNER)
#define GXEDGE_ETCHED     (GXBDR_SUNKENOUTER | GXBDR_RAISEDINNER)
#define GXEDGE_BUMP       (GXBDR_RAISEDOUTER | GXBDR_SUNKENINNER)

/* Border flags */
#define GXBF_LEFT         0x0001
#define GXBF_TOP          0x0002
#define GXBF_RIGHT        0x0004
#define GXBF_BOTTOM       0x0008

#define GXBF_TOPLEFT      (GXBF_TOP | GXBF_LEFT)
#define GXBF_TOPRIGHT     (GXBF_TOP | GXBF_RIGHT)
#define GXBF_BOTTOMLEFT   (GXBF_BOTTOM | GXBF_LEFT)
#define GXBF_BOTTOMRIGHT  (GXBF_BOTTOM | GXBF_RIGHT)
#define GXBF_RECT         (GXBF_LEFT | GXBF_TOP | GXBF_RIGHT | GXBF_BOTTOM)

#define GXBF_DIAGONAL     0x0010

// For diagonal lines, the BF_RECT flags specify the end point of the
// vector bounded by the rectangle parameter.
#define GXBF_DIAGONAL_ENDTOPRIGHT     (GXBF_DIAGONAL | GXBF_TOP | GXBF_RIGHT)
#define GXBF_DIAGONAL_ENDTOPLEFT      (GXBF_DIAGONAL | GXBF_TOP | GXBF_LEFT)
#define GXBF_DIAGONAL_ENDBOTTOMLEFT   (GXBF_DIAGONAL | GXBF_BOTTOM | GXBF_LEFT)
#define GXBF_DIAGONAL_ENDBOTTOMRIGHT  (GXBF_DIAGONAL | GXBF_BOTTOM | GXBF_RIGHT)


#define GXBF_MIDDLE       0x0800  /* Fill in the middle */
#define GXBF_SOFT         0x1000  /* For softer buttons */
#define GXBF_ADJUST       0x2000  /* Calculate the space left over */
#define GXBF_FLAT         0x4000  /* For flat rather than 3D borders */
#define GXBF_MONO         0x8000  /* For monochrome borders */

#define GXMONITORINFOF_PRIMARY        0x00000001

/*
 * Defines for the fVirt field of the Accelerator table structure.
 */
#define GXFVIRTKEY  TRUE          /* Assumed to be == TRUE */
#define GXFNOINVERT 0x02
#define GXFSHIFT    0x04
#define GXFCONTROL  0x08
#define GXFALT      0x10


/*
 * Predefined Clipboard Formats
 */
#define GXCF_TEXT             1
#define GXCF_BITMAP           2
#define GXCF_METAFILEPICT     3
#define GXCF_SYLK             4
#define GXCF_DIF              5
#define GXCF_TIFF             6
#define GXCF_OEMTEXT          7
#define GXCF_DIB              8
#define GXCF_PALETTE          9
#define GXCF_PENDATA          10
#define GXCF_RIFF             11
#define GXCF_WAVE             12
#define GXCF_UNICODETEXT      13
#define GXCF_ENHMETAFILE      14
#define GXCF_HDROP            15
#define GXCF_LOCALE           16
#define GXCF_DIBV5            17
#define GXCF_MAX              18
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

typedef struct _GXICONINFO {
  GXBOOL    fIcon;
  GXDWORD   xHotspot;
  GXDWORD   yHotspot;
  GXHBITMAP hbmMask;
  GXHBITMAP hbmColor;
} GXICONINFO;
typedef GXICONINFO *GXPICONINFO;

#define GXHELPINFO_WINDOW    0x0001
#define GXHELPINFO_MENUITEM  0x0002

struct GXHELPINFO      /* Structure pointed to by lParam of WM_HELP */
{
  GXUINT      cbSize;             /* Size in bytes of this struct  */
  int         iContextType;       /* Either HELPINFO_WINDOW or HELPINFO_MENUITEM */
  int         iCtrlId;            /* Control Id or a Menu item Id. */
  GXHANDLE    hItemHandle;        /* hWnd of control or hMenu.     */
  GXDWORD_PTR dwContextId;        /* Context Id associated with this item */
  GXPOINT     MousePos;           /* Mouse Position in screen co-ordinates */
};

typedef GXHELPINFO *GXLPHELPINFO;
typedef GXHELPINFO *LPGXHELPINFO;


#define GXOFN_SIMPLEBROWSER  0x00000001

struct GXOPENFILENAMEW {
  GXDWORD        lStructSize;         // 数组长度
  GXHWND         hwndOwner;
  //GXHINSTANCE  hInstance;
  GXLPCWSTR      lpstrFilter;
  //GXLPWSTR     lpstrCustomFilter;
  GXDWORD        nMaxCustFilter;
  //GXDWORD      nFilterIndex;
  GXLPWSTR       lpstrFile;
  GXDWORD        nMaxFile;
  GXLPWSTR       lpstrFileTitle;
  GXDWORD        nMaxFileTitle;
  GXLPCWSTR      lpstrInitialDir;
  GXLPCWSTR      lpstrTitle;
  GXDWORD        Flags;
  GXWORD         nFileOffset;
  GXWORD         nFileExtension;
  GXLPCWSTR      lpstrDefExt;
  //GXLPARAM     lCustData;
  //GXLPOFNHOOKPROC lpfnHook;
  //GXLPCWSTR    lpTemplateName;
};
typedef GXOPENFILENAMEW*       GXLPOPENFILENAMEW;


//////////////////////////////////////////////////////////////////////////
//
// GXUI Controls
//

#define GXUISS_TYPE_MASK    0x0000000f
#define GXUISS_TYPE_LABEL   0x00000001
#define GXUISS_TYPE_SPRITE  0x00000002
#define GXUISS_TYPE_RECT    0x00000003

#define GXUISS_LEFT         0x00000000
#define GXUISS_TOP          0x00000000
#define GXUISS_CENTER       0x00000010
#define GXUISS_RIGHT        0x00000020
#define GXUISS_VCENTER      0x00000040
#define GXUISS_BOTTOM       0x00000080
#define GXUISS_NOPREFIX     0x00000100
#define GXUISS_SINGLELINE   0x00000200
#define GXUISS_WORDBREAK    0x00000400
#define GXUISS_SIMPLE       0x00000800  // 简单字符串, 无视上边所有特性
#define GXUISS_NOTIFY       0x00001000
#define GXUISS_CONTRAST     0x00002000
#define GXUISS_EXPANDTABS   0x00004000


//
// GXUI Slider Style
//
#define GXUISLDS_HORZ         0x00000000  // 水平条
#define GXUISLDS_VERT         0x00000001  // 竖直条
#define GXUISLDS_TRACKMINOR   0x00000002  // 第二进度条跟随Handle位置
#define GXUISLDS_DISCRETE     0x00000004  // 对齐离散位置
#define GXUISLDS_SCALING      0x00000008  // 根据尺寸缩放
#define GXUISLDS_THUMB        0x00000010  // 单击进度条时调整Handle位置  
#define GXUISLDS_NOTIFY       0x00000020  // 通知父窗体
#define GXUISLDS_FLOAT        0x00000040  // 浮点数值



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
  GXBOOL      GXDLLAPI gxGetIconInfo            (GXHICON,GXPICONINFO);
  GXHICON     GXDLLAPI gxCopyIcon               (GXHICON hIcon);
  GXHANDLE    GXDLLAPI gxLoadImageW             (GXHINSTANCE hInstance, GXLPCWSTR lpszName, GXUINT uType, GXINT cxDesired, GXINT cyDesired, GXUINT fuLoad);
  GXHICON     GXDLLAPI gxLoadIconW              (GXHINSTANCE hInstance, GXLPCWSTR lpIconName);
  GXHANDLE    GXDLLAPI gxLoadImageA             (GXHINSTANCE hInstance, GXLPCSTR lpszName, GXUINT uType, GXINT cxDesired, GXINT cyDesired, GXUINT fuLoad);
  GXHICON     GXDLLAPI gxLoadIconA              (GXHINSTANCE hInstance, GXLPCSTR lpIconName);

  int         GXDLLAPI gxLoadStringW            (GXHINSTANCE hInstance,GXUINT uID,GXLPWSTR lpBuffer,int nBufferMax);
  GXHWND      GXDLLAPI gxGetParent              (GXHWND);
  GXBOOL      GXDLLAPI gxIsWindowEnabled        (GXHWND);
  GXINT       GXDLLAPI gxFillRect               (GXHDC,GXCONST GXRECT*,GXHBRUSH);  
  GXINT       GXDLLAPI gxDrawTextW              (GXHDC hDC, GXLPCWSTR lpString, int nCount, GXLPRECT lpRect, GXUINT uFormat);
  GXINT       GXDLLAPI gxDrawTextA              (GXHDC hDC, GXLPCSTR lpString, int nCount, GXLPRECT lpRect, GXUINT uFormat);
  GXINT       GXDLLAPI gxFrameRect              (GXHDC,GXCONST GXRECT *,GXHBRUSH);  
  GXBOOL      GXDLLAPI gxDrawIconEx             (GXHDC,GXINT,GXINT,GXHICON,GXINT,GXINT,GXUINT,GXHBRUSH,GXUINT);  
  GXBOOL      GXDLLAPI gxDrawEdge               (GXHDC,GXLPRECT,GXUINT,GXUINT);

  GXINT       GXDLLAPI gxReleaseDC              (GXHWND,GXHDC);
  GXBOOL      GXDLLAPI gxRedrawWindow           (GXHWND hWnd, GXLPCRECT lprcUpdate, GXHRGN hrgnUpdate, GXUINT flags);
  GXDWORD     GXDLLAPI gxGetSysColor            (GXINT nIndex);
  GXHBRUSH    GXDLLAPI gxGetCtrlBrush           (GXINT nIndex);
  GXHWND      GXDLLAPI gxWindowFromPoint        (GXPOINT *lpPoint);
  GXHWND      GXDLLAPI gxChildWindowFromPoint   (GXHWND hWndParent,GXPOINT Point);

  GXUINT      GXDLLAPI gxGetDoubleClickTime     ();
  GXBOOL      GXDLLAPI gxSetDoubleClickTime     (GXUINT dwTime);

  GXBOOL      GXDLLAPI gxEnableWindow           (GXHWND hWnd,GXBOOL bEnable);

  GXBOOL      GXDLLAPI gxShowWindow             (GXHWND hWnd,int nCmdShow);
  GXBOOL      GXDLLAPI gxClientToScreen         (GXHWND,GXLPPOINT);
  GXBOOL      GXDLLAPI gxScreenToClient         (GXHWND hWnd, GXLPPOINT lpPoint);
  GXBOOL      GXDLLAPI gxDragDetect             (GXHWND hWnd, GXLPPOINT lpPoint);

  GXHWND      GXDLLAPI gxGetCapture             ();
  GXINT       GXDLLAPI gxSetScrollInfo          (GXHWND,GXINT,GXLPSCROLLINFO,GXBOOL);
  GXBOOL      GXDLLAPI gxGetScrollInfo          (GXHWND,GXINT,GXLPSCROLLINFO);
  GXINT       GXDLLAPI gxGetScrollPos           (GXHWND,GXINT);
  int         GXDLLAPI gxSetScrollPos           (GXHWND hWnd,int nBar,int nPos,GXBOOL bRedraw);
  GXBOOL      GXDLLAPI gxShowScrollBar          (GXHWND hWnd,int wBar,GXBOOL bShow);

  int         GXDLLAPI gxSetWindowRgn           (GXHWND hWnd,GXHRGN hRgn,GXBOOL bRedraw);
  GXHRGN      GXDLLAPI gxCreatePolygonRgn       (GXCONST GXPOINT *lppt,int cPoints,int fnPolyFillMode);
  GXHRGN      GXDLLAPI gxCreateRoundRectRgn     (int nLeftRect,int nTopRect,int nRightRect,int nBottomRect,int nWidthEllipse,int nHeightEllipse);
  int         GXDLLAPI gxGetWindowRgn           (GXHWND hWnd,GXHRGN hRgn);
  GXBOOL      GXDLLAPI gxFillRgn                (GXHDC hdc,GXHRGN hrgn,GXHBRUSH hbr);
  GXBOOL      GXDLLAPI gxFrameRgn               (GXHDC hdc,GXHRGN hrgn,GXHBRUSH hbr,int nWidth,int nHeight);

  //GXVOID    GXDLLAPI GXGetClip                (GXHWND hWnd, GXBOOL bClient, GXLPRECT lprcOut);
  GXBOOL      GXDLLAPI gxGetWindowRect          (GXHWND hWnd, GXLPRECT lpRect);
  GXBOOL      GXDLLAPI GXInvalidateWindowRgn    (GXHWND hWnd, GRegion* pRegion, GXBOOL bErase); // pRegion是window空间的
  GXBOOL      GXDLLAPI GXInvalidateWindowRect   (GXHWND hWnd, GXLPCRECT lpRect, GXBOOL bErase);
  GXBOOL      GXDLLAPI gxInvalidateRect         (GXHWND hWnd, GXLPCRECT lpRect, GXBOOL bErase);
  GXBOOL      GXDLLAPI gxInvalidateRgn          (GXHWND hWnd, GXHRGN hRegion, GXBOOL bErase);

  GXBOOL      GXDLLAPI gxValidateRect           (GXHWND hWnd, GXLPCRECT lpRect);
  GXBOOL      GXDLLAPI gxGetUpdateRect          (GXHWND hWnd, GXLPRECT lpRect, GXBOOL bErase);

  GXBOOL      GXDLLAPI gxIsTopLevelWindow       (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxIsWindow               (GXHWND);
  GXBOOL      GXDLLAPI gxIsChild                (GXHWND hWndParent,GXHWND hWnd);
  GXBOOL      GXDLLAPI gxIsWindowVisible        (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxReleaseCapture         ();
  GXHWND      GXDLLAPI gxSetCapture             (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxGetClientRect          (GXHWND hWnd,GXLPRECT lpRect);
  GXINT       GXDLLAPI gxInternalGetWindowText  (GXHWND hWnd, GXLPWSTR lpString, int nMaxCount);
  GXHWND      GXDLLAPI gxSetFocus               (GXHWND hWnd);
  GXHWND      GXDLLAPI gxGetFocus               ();

  GXBOOL      GXDLLAPI gxMoveWindow             (GXHWND hWnd, int X, int Y, int nWidth, int nHeight, GXBOOL bRepaint);
  GXBOOL      GXDLLAPI gxSetWindowPos           (GXHWND hWnd, GXHWND hWndInsertAfter, GXINT X, GXINT Y, GXINT cx,  GXINT cy,  GXUINT uFlags);
  GXHDWP      GXDLLAPI gxBeginDeferWindowPos    (int nNumWindows);
  GXHDWP      GXDLLAPI gxDeferWindowPos         (GXHDWP hWinPosInfo, GXHWND hWnd, GXHWND hWndInsertAfter, int x, int y, int cx, int cy, GXUINT uFlags);
  GXBOOL      GXDLLAPI gxEndDeferWindowPos      (GXHDWP hWinPosInfo);
  int         GXDLLAPI gxGetDlgCtrlID           (GXHWND hwndCtl);
  GXUINT      GXDLLAPI gxRegisterWindowMessageW (GXLPCWSTR lpString);
  GXBOOL      GXDLLAPI gxEnumChildWindows       (GXHWND hWndParent,GXWNDENUMPROC lpEnumFunc,GXLPARAM lParam);
  GXHWND      GXDLLAPI gxFindWindow             (GXLPCWSTR lpClassName, GXLPCWSTR lpWindowName);
  GXHWND      GXDLLAPI gxFindWindowEx           (GXHWND hwndParent, GXHWND hwndChildAfter, GXLPCWSTR lpszClass, GXLPCWSTR lpszWindow);

  GXUINT      GXDLLAPI gxSetTimer               (GXHWND, GXUINT nIDEvent, GXUINT uElapse, GXTIMERPROC);
  GXBOOL      GXDLLAPI gxKillTimer              (GXHWND, GXUINT nIDEvent);

  GXHBRUSH    GXDLLAPI gxGetSysColorBrush       (GXINT nIndex);
  GXBOOL      GXDLLAPI gxDrawFocusRect          (GXHDC,GXCONST GXRECT *);
  GXBOOL      GXDLLAPI gxDrawIcon               (GXHDC hDC,int X,int Y,GXHICON hIcon);
  GXLONG      GXDLLAPI gxTabbedTextOutW         (GXHDC,GXINT,GXINT,GXLPCWSTR,GXINT,GXINT,GXLPINT,GXINT);
  GXDWORD     GXDLLAPI gxGetTabbedTextExtentW   (GXHDC,GXLPCWSTR,GXINT,GXINT,GXINT*);


  GXBOOL      GXDLLAPI gxCreateCaret            (GXHWND,GXHBITMAP,GXINT,GXINT);
  GXBOOL      GXDLLAPI gxShowCaret              (GXHWND);
  GXBOOL      GXDLLAPI gxHideCaret              (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxSetCaretPos            (GXINT,GXINT);
  GXBOOL      GXDLLAPI gxDestroyCaret           ();
  GXBOOL      GXDLLAPI gxSetCaretBlinkTime      (GXUINT);

  GXBOOL      GXDLLAPI gxInsertMenuW            (GXHMENU hMenu, GXUINT pos, GXUINT flags, GXUINT_PTR id, GXLPCWSTR str );
  GXBOOL      GXDLLAPI gxAppendMenuW            (GXHMENU hMenu, GXUINT flags, GXUINT_PTR id, GXLPCWSTR data);
  GXBOOL      GXDLLAPI gxAppendMenuA            (GXHMENU hMenu, GXUINT flags, GXUINT_PTR id, GXLPCSTR data);

  GXBOOL      GXDLLAPI gxDeleteMenu             (GXHMENU hMenu, GXUINT nPos, GXUINT wFlags );
  GXHMENU     GXDLLAPI gxLoadMenuW              (GXHINSTANCE instance, GXLPCWSTR name );
  GXBOOL      GXDLLAPI gxRemoveMenu             (GXHMENU hMenu, GXUINT nPos, GXUINT wFlags );
  GXBOOL      GXDLLAPI gxModifyMenuW            (GXHMENU hMenu, GXUINT pos, GXUINT flags,GXUINT_PTR id, GXLPCWSTR str );
  GXBOOL      GXDLLAPI gxIsMenu                 (GXHMENU hmenu);
  GXBOOL      GXDLLAPI gxDestroyMenu            (GXHMENU hMenu );
  GXHMENU     GXDLLAPI gxGetMenu                (GXHWND hWnd );
  GXBOOL      GXDLLAPI gxSetMenuDefaultItem     (GXHMENU hmenu, GXUINT uItem, GXUINT bypos);
  GXBOOL      GXDLLAPI gxInsertMenuItemW        (GXHMENU hMenu, GXUINT uItem, GXBOOL bypos,const GXMENUITEMINFOW *lpmii);
  GXHMENU     GXDLLAPI gxLoadMenuIndirectW      (GXLPCVOID lpTemplate);
  GXHMENU     GXDLLAPI gxCreateMenu             ();
  GXHMENU     GXDLLAPI gxCreatePopupMenu        ();
  GXHMENU     GXDLLAPI gxGetSubMenu             (GXHMENU hMenu, GXINT nPos );
  GXBOOL      GXDLLAPI gxEnableMenuItem         (GXHMENU hMenu, GXUINT wItemID, GXUINT wFlags );
  GXBOOL      GXDLLAPI gxTrackPopupMenu         (GXHMENU hMenu, GXUINT wFlags, GXINT x, GXINT y,GXINT nReserved, GXHWND hWnd, const GXRECT *lpRect );

  GXBOOL      GXDLLAPI gxEmptyClipboard             ();
  GXBOOL      GXDLLAPI gxOpenClipboard              (GXHWND hWnd);
  GXHANDLE    GXDLLAPI gxSetClipboardData           (GXUINT,GXHANDLE);
  GXHANDLE    GXDLLAPI gxGetClipboardData           (GXUINT);
  GXBOOL      GXDLLAPI gxCloseClipboard             ();
  GXBOOL      GXDLLAPI gxIsClipboardFormatAvailable (GXUINT format);

  GXHWND      GXDLLAPI gxGetDlgItem             (GXHWND hDlg, GXINT nIDCtrl);
  GXHWND      GXDLLAPI GXGetDlgItemByName       (GXHWND hWnd, GXLPCWSTR szName);
  GXHWND      GXDLLAPI gxGetNextDlgGroupItem    (GXHWND hDlg, GXHWND hCtl,GXBOOL bPrevious);
  GXBOOL      GXDLLAPI gxCheckRadioButton       (GXHWND hDlg,  int nIDFirstButton,  int nIDLastButton, int nIDCheckButton);

  GXHBITMAP   GXDLLAPI gxLoadBitmapW            (GXHINSTANCE hInstance,GXLPCWSTR lpBitmapName);
  GXHBITMAP   GXDLLAPI gxLoadBitmapA            (GXHINSTANCE hInstance,GXLPCSTR lpBitmapName);

  GXBOOL      GXDLLAPI gxDrawStateW             (GXHDC hdc,GXHBRUSH hbr,GXDRAWSTATEPROC lpOutputFunc,GXLPARAM lData,GXWPARAM wData,int x,int y,int cx,int cy,GXUINT fuFlags);  
  GXBOOL      GXDLLAPI gxDrawFrameControl       (GXHDC hdc,GXLPRECT lprc,GXUINT uType,GXUINT uState);  
  GXHANDLE    GXDLLAPI gxCopyImage              (GXHANDLE hImage,GXUINT uType,int cxDesired,int cyDesired,GXUINT fuFlags);

  GXLRESULT   GXDLLAPI gxDefWindowProcW         (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  GXLRESULT   GXDLLAPI gxDefWindowProcA         (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);

  GXHWND      GXDLLAPI gxGetActiveWindow        ();
  GXHWND      GXDLLAPI gxSetActiveWindow        (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxSetForegroundWindow    (GXHWND hWnd);
  GXHWND      GXDLLAPI gxGetWindow              (GXHWND hWnd,GXUINT uCmd);
  GXLONG_PTR  GXDLLAPI gxGetWindowLongW         (GXHWND hWnd,GXINT nIndex);
  GXLONG_PTR  GXDLLAPI gxSetWindowLongW         (GXHWND hWnd,GXINT nIndex, GXLONG_PTR dwNewLong);
  GXLRESULT   GXDLLAPI gxSendMessageW           (GXHWND hWnd,GXUINT Msg,GXWPARAM wParam,GXLPARAM lParam);
  GXBOOL      GXDLLAPI gxGetMessage             (GXLPMSG lpMsg,GXHWND hWnd);
  GXHWND      GXDLLAPI gxSetParent              (GXHWND hWndChild, GXHWND hWndNewParent);

  GXLRESULT   GXDLLAPI gxSendDlgItemMessageW    (GXHWND hDlg, int nIDDlgItem,GXUINT Msg,GXWPARAM wParam,GXLPARAM lParam);
  GXBOOL      GXDLLAPI gxEndDialog              (GXHWND hDlg, int nResult);

  GXHDC       GXDLLAPI gxBeginPaint             (GXHWND hwnd, GXLPPAINTSTRUCT lpPaint);
  GXBOOL      GXDLLAPI gxEndPaint               (GXHWND, GXCONST GXPAINTSTRUCT*);
  GXHDC       GXDLLAPI gxGetDC                  (GXHWND hWnd);
  GXHDC       GXDLLAPI gxGetDCEx                (GXHWND hWnd, GXHRGN hrgn, GXDWORD dwFlags);
  GXHDC       GXDLLAPI gxGetWindowDC            (GXHWND hWnd);

  GXLRESULT   GXDLLAPI gxDispatchMessageW       (const GXMSG *lpmsg);
  void        GXDLLAPI gxPostQuitMessage        (int nExitCode);
  GXBOOL      GXDLLAPI gxPostMessageW           (GXHWND hWnd, GXUINT Msg, GXWPARAM wParam, GXLPARAM lParam);
  GXBOOL      GXDLLAPI gxPeekMessageW           (GXLPMSG lpMsg,GXHWND hWnd,GXUINT wMsgFilterMin,GXUINT wMsgFilterMax,GXUINT wRemoveMsg);
  GXBOOL      GXDLLAPI gxTranslateMessage       (GXCONST GXMSG *lpMsg);
  GXBOOL      GXDLLAPI gxWaitMessage            ();

  GXBOOL      GXDLLAPI gxScrollWindow           (GXHWND hWnd, int XAmount,int YAmount,GXCONST GXRECT *lpRect,GXCONST GXRECT *lpClipRect);
  GXINT       GXDLLAPI gxScrollWindowEx         (GXHWND hWnd, GXINT dx, GXINT dy, GXCONST GXRECT *prcScroll, GXCONST GXRECT * prcClip, GXHRGN hrgnUpdate, GXLPRECT prcUpdate, GXUINT flags);
  GXBOOL      GXDLLAPI gxScrollDC               (GXHDC hDC, int dx, int dy, const GXRECT *lprcScroll, const GXRECT *lprcClip, GXHRGN hrgnUpdate, LPGXRECT lprcUpdate);

  GXBOOL      GXDLLAPI gxIsIconic               (GXHWND hWnd);
  GXBOOL      GXDLLAPI gxIsZoomed               (GXHWND hWnd);

  GXBOOL      GXDLLAPI gxDestroyWindow          (GXHWND hWnd);
  GXHWND      GXDLLAPI gxGetAncestor            (GXHWND hwnd,GXUINT gaFlags);

  GXBOOL      GXDLLAPI gxSetWindowTextW         (GXHWND hWnd,GXLPCWSTR lpString);
  GXINT       GXDLLAPI gxGetWindowTextW         (GXHWND hWnd,GXLPWSTR lpString,int nMaxCount);
  int         GXDLLAPI gxGetWindowTextA         (GXHWND hWnd,GXCHAR* lpString,int nMaxCount);
  int         GXDLLAPI gxGetWindowTextLengthW   (GXHWND hWnd);
  int         GXDLLAPI gxGetWindowTextLengthA   (GXHWND hWnd);
  GXDWORD     GXDLLAPI gxGetWindowThreadProcessId (GXHWND hWnd, GXLPDWORD lpdwProcessId);
  GXDWORD     GXDLLAPI gxRegisterClassExW       (GXCONST GXWNDCLASSEX *lpwcx);
  int         GXDLLAPI gxGetClassNameW          (GXHWND hWnd,GXLPWSTR lpClassName,int nMaxCount);
  GXBOOL      GXDLLAPI gxUnregisterClassW       (GXLPCWSTR lpClassName,GXHINSTANCE hInstance);
  GXDWORD     GXDLLAPI gxGetClassInfoExW        (GXHINSTANCE hinst,GXLPCWSTR lpszClass,GXLPWNDCLASSEX lpwcx);

  GXLONG_PTR  GXDLLAPI gxGetClassLongW          (GXHWND hWnd,int nIndex);
  GXBOOL      GXDLLAPI gxCallMsgFilterW         (GXLPMSG lpMsg,int nCode);
  int         GXDLLAPI gxMapWindowPoints        (GXHWND hWndFrom,GXHWND hWndTo,GXLPPOINT lpPoints,GXUINT cPoints);
  GXBOOL      GXDLLAPI gxUpdateWindow           (GXHWND hWnd);
  GXLONG      GXDLLAPI gxGdiGetCharDimensions   (GXHDC hdc, GXLPTEXTMETRICW lptm,GXLONG*height);
  GXHMONITOR  GXDLLAPI gxMonitorFromPoint       (GXPOINT pt,GXDWORD dwFlags);
  GXHMONITOR  GXDLLAPI gxMonitorFromRect        (GXLPCRECT lprc,GXDWORD dwFlags);
  GXBOOL      GXDLLAPI gxGetMonitorInfoW        (GXHMONITOR hMonitor, GXLPMONITORINFO lpmi);
  GXHWND      GXDLLAPI gxGetDesktopWindow       ();
  GXBOOL      GXDLLAPI gxDestroyIcon            (GXHICON hIcon);
  GXHICON     GXDLLAPI gxCreateIconIndirect     (GXPICONINFO piconinfo);
  int         GXDLLAPI gxGetSystemMetrics       (int nIndex);
  GXBOOL      GXDLLAPI gxSystemParametersInfoW  (GXUINT uiAction, GXUINT uiParam, GXLPVOID pvParam, GXUINT fWinIni);

  GXLRESULT   GXDLLAPI gxCallWindowProcW        (GXWNDPROC lpPrevWndFunc,GXHWND hWnd,GXUINT Msg,GXWPARAM wParam,GXLPARAM lParam);

  GXLONG      GXDLLAPI gxGetDialogBaseUnits     ();
  GXHWND      GXDLLAPI gxCreateDialogParamW     (GXHINSTANCE hInstance, GXLPCWSTR lpTemplate, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);
  int         GXDLLAPI gxDialogBoxParamW        (GXHINSTANCE _hInstance,GXLPCWSTR lpTemplateName,GXHWND hWndParent,GXDLGPROC lpDialogFunc,GXLPARAM dwInitParam);
  GXHWND      GXDLLAPI gxCreateWindowExW        (GXDWORD dwExStyle,GXLPCWSTR lpClassName,GXLPCWSTR lpWindowName,GXDWORD dwStyle, GXINT x,GXINT y,GXINT nWidth,GXINT nHeight,GXHWND hWndParent,GXHMENU hMenu,GXHINSTANCE hInstance,GXLPVOID lpParam);
  int         GXDLLAPI gxMessageBoxW            (GXHWND hWnd, GXLPCWSTR lpText, GXLPCWSTR lpCaption, GXUINT uType);	


  GXHWND      GXDLLAPI gxCreateWindowW          (GXLPCWSTR lpClassName, GXLPCWSTR lpWindowName, GXDWORD dwStyle, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPVOID lpParam);
    
  GXBOOL      GXDLLAPI gxGetCursorPos           (GXLPPOINT lpPoint);
  GXBOOL      GXDLLAPI gxSetCursorPos           (int X, int Y);
  GXHCURSOR   GXDLLAPI gxSetCursor              (GXHCURSOR hCursor);
  int         GXDLLAPI gxShowCursor             (GXBOOL bShow);
  GXHCURSOR   GXDLLAPI gxLoadCursorW            (GXHINSTANCE hInstance, GXLPCWSTR lpCursorName);
  GXHCURSOR   GXDLLAPI gxLoadCursorA            (GXHINSTANCE hInstance, GXLPCSTR lpCursorName);

  GXBOOL      GXDLLAPI gxTrackMouseEvent        (GXLPTRACKMOUSEEVENT lpEventTrack);
  GXBOOL      GXDLLAPI gxIsWindowUnicode        (GXHWND hWnd);

  GXBOOL      GXDLLAPI gxSetPropW               (GXHWND hWnd,GXLPCWSTR lpString,GXHANDLE hData);
  GXHANDLE    GXDLLAPI gxGetPropW               (GXHWND hWnd,GXLPCWSTR lpString);
  GXHANDLE    GXDLLAPI gxRemovePropW            (GXHWND hWnd,GXLPCWSTR lpString);

  GXBOOL      GXDLLAPI gxAdjustWindowRectEx     (GXLPRECT lpRect, GXDWORD dwStyle, GXBOOL bMenu, GXDWORD dwExStyle);

  GXSHORT     GXDLLAPI gxGetAsyncKeyState       (int vKey);
  GXSHORT     GXDLLAPI gxGetKeyState            (int nVirtKey);

  GXLONG      GXDLLAPI gxGetMessageTime         ();
  GXDWORD     GXDLLAPI gxGetMessagePos          ();
  GXBOOL      GXDLLAPI gxMessageBeep            (GXUINT uType);
  GXBOOL      GXDLLAPI gxFlashWindow            (GXHWND hWnd, GXBOOL bInvert);
  GXDWORD     GXDLLAPI gxCharUpperBuffW         (GXLPWSTR lpsz, GXDWORD cchLength);
  GXDWORD     GXDLLAPI gxCharLowerBuffW         (GXLPWSTR lpsz, GXDWORD cchLength);

  int         GXDLLAPI gxCopyAcceleratorTableW  (GXHACCEL hAccelSrc, LPACCEL lpAccelDst, int cAccelEntries);
  GXINT       GXDLLAPI gxTranslateAcceleratorA  (GXHWND hWnd, GXHACCEL hAccel, GXLPMSG msg);
  GXINT       GXDLLAPI gxTranslateAcceleratorW  (GXHWND hWnd, GXHACCEL hAccel, GXLPMSG msg);

#ifdef __cplusplus
}
#endif // __cplusplus
//#ifdef _WIN32
//GXHICON    GXDLLAPI GXCursorToIcon      (HCURSOR hCursor);
//#endif

#define gxLoadImage             gxLoadImageW
#define gxLoadCursor            gxLoadCursorW
#define gxGetWindowLong         gxGetWindowLongW
#define gxSetWindowLong         gxSetWindowLongW
#define gxGetWindowLongPtrW     gxGetWindowLongW
#define gxSetWindowLongPtrW     gxSetWindowLongW
#define gxSetWindowLongPtrA     gxSetWindowLongW
#define gxGetClassLongPtrW      gxGetClassLongW
#define gxSendMessage           gxSendMessageW
#define gxSendMessageA          gxSendMessageW
#define gxGetTextExtentPoint    gxGetTextExtentPointW
#define gxTabbedTextOut         gxTabbedTextOutW
#define gxDefWindowProc         gxDefWindowProcW
#define gxCallWindowProc        gxCallWindowProcW
#define gxCallWindowProcA       gxCallWindowProcW
#define gxSetWindowText         gxSetWindowTextW
#define gxGetClassName          gxGetClassNameW
#define gxCreateWindow          gxCreateWindowW
#define gxCreateWindowEx        gxCreateWindowExW
#define gxRegisterClassEx       gxRegisterClassExW
#define gxGetClassInfoEx        gxGetClassInfoExW
#define gxPostMessage           gxPostMessageW
#define gxGetWindowTextLength   gxGetWindowTextLengthW
#define gxDrawText              gxDrawTextW

#endif // _GX_USER_H_
#endif // _DEV_DISABLE_UI_CODE

#define GXDCX_WINDOW           0x00000001L
#define GXDCX_CACHE            0x00000002L
#define GXDCX_NORESETATTRS     0x00000004L
#define GXDCX_CLIPCHILDREN     0x00000008L
#define GXDCX_CLIPSIBLINGS     0x00000010L
#define GXDCX_PARENTCLIP       0x00000020L
#define GXDCX_EXCLUDERGN       0x00000040L
#define GXDCX_INTERSECTRGN     0x00000080L
#define GXDCX_EXCLUDEUPDATE    0x00000100L
#define GXDCX_INTERSECTUPDATE  0x00000200L
#define GXDCX_LOCKWINDOWUPDATE 0x00000400L
#define GXDCX_VALIDATE         0x00200000L

//
// DrawText() Format Flags
//
#define GXDT_TOP                      0x00000000
#define GXDT_LEFT                     0x00000000
#define GXDT_CENTER                   0x00000001
#define GXDT_RIGHT                    0x00000002
#define GXDT_VCENTER                  0x00000004
#define GXDT_BOTTOM                   0x00000008
#define GXDT_WORDBREAK                0x00000010
#define GXDT_SINGLELINE               0x00000020
#define GXDT_EXPANDTABS               0x00000040
#define GXDT_TABSTOP                  0x00000080
#define GXDT_NOCLIP                   0x00000100
#define GXDT_EXTERNALLEADING          0x00000200
#define GXDT_CALCRECT                 0x00000400
#define GXDT_NOPREFIX                 0x00000800
#define GXDT_INTERNAL                 0x00001000
#define GXDT_EDITCONTROL              0x00002000
#define GXDT_PATH_ELLIPSIS            0x00004000
#define GXDT_END_ELLIPSIS             0x00008000
#define GXDT_MODIFYSTRING             0x00010000
#define GXDT_RTLREADING               0x00020000
#define GXDT_WORD_ELLIPSIS            0x00040000
#define GXDT_NOFULLWIDTHCHARBREAK     0x00080000
#define GXDT_HIDEPREFIX               0x00100000
#define GXDT_PREFIXONLY               0x00200000

//
//  Code Page Default Values.
//
#define GXCP_ACP                    0           // default to ANSI code page
//#define GXCP_OEMCP                  1           // default to OEM  code page
//#define GXCP_MACCP                  2           // default to MAC  code page
//#define GXCP_THREAD_ACP             3           // current thread's ANSI code page
//#define GXCP_SYMBOL                 42          // SYMBOL translations
//#define GXCP_UTF7                   65000       // UTF-7 translation
//#define GXCP_UTF8                   65001       // UTF-8 translation

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GXBOOL      GXDLLAPI gxUnionRect        (GXLPRECT lpOut, GXLPCRECT lprc1, GXLPCRECT lprc2);
GXBOOL      GXDLLAPI gxUnionRegn        (GXLPREGN lpOut, GXLPCREGN lprg1, GXLPCREGN lprg2);
GXBOOL      GXDLLAPI gxIntersectRect    (GXLPRECT,GXCONST GXRECT *,GXCONST GXRECT *);
GXBOOL      GXDLLAPI gxIsRectEmpty      (GXCONST GXRECT *lprc);
GXBOOL      GXDLLAPI gxIsRegnEmpty      (GXCONST GXREGN *lprg);
GXBOOL      GXDLLAPI gxSetRect          (GXLPRECT lprc, GXINT xLeft, GXINT yTop, GXINT xRight, GXINT yBottom);
GXBOOL      GXDLLAPI gxSetRegn          (GXLPREGN lprg, GXINT xLeft, GXINT yTop, GXINT xWidth, GXINT yHeight);
GXBOOL      GXDLLAPI gxCopyRect         (GXLPRECT,GXCONST GXRECT *);
GXBOOL      GXDLLAPI gxPtInRect         (GXCONST GXRECT *lprc,GXPOINT pt);
GXBOOL      GXDLLAPI gxPtInRegn         (GXCONST GXREGN *lpgn,GXPOINT pt);
GXBOOL      GXDLLAPI gxEqualRect        (GXCONST GXRECT *lprc1,GXCONST GXRECT *lprc2);
GXBOOL      GXDLLAPI gxOffsetRect       (GXLPRECT lprc,GXINT dx,GXINT dy);
GXBOOL      GXDLLAPI gxInflateRect      (GXLPRECT lprc,GXINT dx,GXINT dy);
GXVOID      GXDLLAPI gxRegnToRect       (GXLPRECT lprc, GXCONST GXLPREGN lpregn);
GXVOID      GXDLLAPI gxRectToRegn       (GXLPREGN lpregn, GXCONST GXLPRECT lprc);
GXBOOL      GXDLLAPI gxSetRectEmpty     (GXLPRECT lprc);
GXBOOL      GXDLLAPI gxSetRegnEmpty     (GXLPREGN lprc);

LPGXREGN    GXDLLAPI gxLerpRegn         (LPGXREGN lprg, LPGXREGN lprg1, LPGXREGN lprg2, float fLerp);


int         GXDLLAPI gxMultiByteToWideChar      (GXUINT CodePage, GXDWORD dwFlags, GXLPCSTR lpMultiByteStr, int cchMultiByte, GXLPWSTR lpWideCharStr, int cchWideChar);
int         GXDLLAPI gxWideCharToMultiByte      (GXUINT CodePage, GXDWORD dwFlags, GXLPCWSTR lpWideCharStr, int cchWideChar, GXLPSTR lpMultiByteStr, int cchMultiByte, GXLPCSTR lpDefaultChar, GXBOOL* lpUsedDefaultChar);
GXDWORD     GXDLLAPI gxGetTickCount             ();
u64         GXDLLAPI gxGetTickCount64           ();

GXBOOL      GXDLLAPI gxGetSaveFileNameW         (GXLPOPENFILENAMEW lpOFN);
GXBOOL      GXDLLAPI gxGetOpenFileNameW         (GXLPOPENFILENAMEW lpOFN);

#ifdef __cplusplus
}
#endif // __cplusplus

LPGXGRAPHICS  GXDLLAPI GXGetGraphics      (GXHWND hWnd);
LPGXWNDCANVAS GXDLLAPI GXGetWndCanvas     (GXHDC hdc);