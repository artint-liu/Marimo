#define GXTVN_FIRST               ((GXINT)0U-(GXINT)400U)       // treeview
#define GXTVN_LAST                ((GXINT)0U-(GXINT)499U)
#define GXTV_FIRST                0x1100      // TreeView messages

enum 
{
  TVP_TREEITEM = 1,
  TVP_GLYPH = 2,
  TVP_BRANCH = 3,
};

enum{
  GLPS_CLOSED = 1,
  GLPS_OPENED = 2,
};

#define PGN_FIRST               (0U-900U)       // Pager Control
#define PGN_LAST                (0U-950U)
#define PGN_SCROLL          (PGN_FIRST-1)
#define PGN_CALCSIZE        (PGN_FIRST-2)

#define PGF_CALCWIDTH       1
#define PGF_CALCHEIGHT      2

//////////////////////////////////////////////////////////////////////////
//
// 以下从 Visual Studio 的 Commctrl.h 复制过来的 TreeView 控件相关定义
//

#ifndef NOTREEVIEW


#define GXWC_TREEVIEWA            "SysTreeView32"
#define GXWC_TREEVIEWW            _CLTEXT("SysTreeView32")

#ifdef _UNICODE
#define  GXWC_TREEVIEW            GXWC_TREEVIEWW
#else
#define  GXWC_TREEVIEW            GXWC_TREEVIEWA
#endif

// begin_r_commctrl

#define TVS_HASBUTTONS          0x0001
#define TVS_HASLINES            0x0002
#define TVS_LINESATROOT         0x0004
#define TVS_EDITLABELS          0x0008
#define TVS_DISABLEDRAGDROP     0x0010
#define TVS_SHOWSELALWAYS       0x0020

#define TVS_RTLREADING          0x0040

#define TVS_NOTOOLTIPS          0x0080
#define TVS_CHECKBOXES          0x0100
#define TVS_TRACKSELECT         0x0200

#define TVS_SINGLEEXPAND        0x0400
#define TVS_INFOTIP             0x0800
#define TVS_FULLROWSELECT       0x1000
#define TVS_NOSCROLL            0x2000
#define TVS_NONEVENHEIGHT       0x4000


#define TVS_NOHSCROLL           0x8000  // TVS_NOSCROLL overrides this



// end_r_commctrl

struct _GXTREEITEM;
typedef _GXTREEITEM *GXHTREEITEM;

#define GXTVIF_TEXT               0x0001
#define GXTVIF_IMAGE              0x0002
#define GXTVIF_PARAM              0x0004
#define GXTVIF_STATE              0x0008
#define GXTVIF_HANDLE             0x0010
#define GXTVIF_SELECTEDIMAGE      0x0020
#define GXTVIF_CHILDREN           0x0040

#define GXTVIF_INTEGRAL           0x0080

#define TVIS_SELECTED           0x0002
#define TVIS_CUT                0x0004
#define TVIS_DROPHILITED        0x0008
#define TVIS_BOLD               0x0010
#define TVIS_EXPANDED           0x0020
#define TVIS_EXPANDEDONCE       0x0040

#define TVIS_EXPANDPARTIAL      0x0080


#define TVIS_OVERLAYMASK        0x0F00
#define TVIS_STATEIMAGEMASK     0xF000
#define TVIS_USERMASK           0xF000

#define I_CHILDRENCALLBACK  (-1)

#define LPGXTV_ITEMW              LPGXTVITEMW
#define LPGXTV_ITEMA              LPGXTVITEMA
#define GXTV_ITEMW                GXTVITEMW
#define GXTV_ITEMA                GXTVITEMA

#define GXLPTV_ITEM               GXLPTVITEM
#define GXTV_ITEM                 GXTVITEM

typedef struct tagGXTVITEMA {
  GXUINT      mask;
  GXHTREEITEM hItem;
  GXUINT      state;
  GXUINT      stateMask;
  GXLPSTR     pszText;
  int       cchTextMax;
  int       iImage;
  int       iSelectedImage;
  int       cChildren;
  GXLPARAM    lParam;
} GXTVITEMA, *LPGXTVITEMA;

typedef struct tagGXTVITEMW {
  GXUINT      mask;
  GXHTREEITEM hItem;
  GXUINT      state;
  GXUINT      stateMask;
  GXLPWSTR    pszText;
  int       cchTextMax;
  int       iImage;
  int       iSelectedImage;
  int       cChildren;
  GXLPARAM    lParam;
} GXTVITEMW, *LPGXTVITEMW;


// only used for Get and Set messages.  no notifies
typedef struct tagGXTVITEMEXA {
  GXUINT      mask;
  GXHTREEITEM hItem;
  GXUINT      state;
  GXUINT      stateMask;
  GXLPSTR     pszText;
  int         cchTextMax;
  int         iImage;
  int         iSelectedImage;
  int         cChildren;
  GXLPARAM    lParam;
  int         iIntegral;
} GXTVITEMEXA, *GXLPTVITEMEXA, *LPGXTVITEMEXA;

// only used for Get and Set messages.  no notifies
typedef struct tagGXTVITEMEXW {
  GXUINT      mask;
  GXHTREEITEM hItem;
  GXUINT      state;
  GXUINT      stateMask;
  GXLPWSTR    pszText;
  int         cchTextMax;
  int         iImage;
  int         iSelectedImage;
  int         cChildren;
  GXLPARAM    lParam;
  int         iIntegral;
} GXTVITEMEXW, *GXLPTVITEMEXW, *LPGXTVITEMEXW;
#ifdef _UNICODE
typedef GXTVITEMEXW GXTVITEMEX;
typedef GXLPTVITEMEXW GXLPTVITEMEX;
#else
typedef GXTVITEMEXA GXTVITEMEX;
typedef GXLPTVITEMEXA GXLPTVITEMEX;
#endif // _UNICODE



#ifdef _UNICODE
#define  GXTVITEM               GXTVITEMW
#define  GXLPTVITEM             LPGXTVITEMW
#else
#define  GXTVITEM               GXTVITEMA
#define  GXLPTVITEM             LPGXTVITEMA
#endif


#define ULONG_GXTVI_ROOT                ((GXULONG_PTR)-0x10000)
#define ULONG_GXTVI_FIRST               ((GXULONG_PTR)-0x0FFFF)
#define ULONG_GXTVI_LAST                ((GXULONG_PTR)-0x0FFFE)
#define ULONG_GXTVI_SORT                ((GXULONG_PTR)-0x0FFFD)

#define GXTVI_ROOT                ((GXHTREEITEM)ULONG_GXTVI_ROOT)
#define GXTVI_FIRST               ((GXHTREEITEM)ULONG_GXTVI_FIRST)
#define GXTVI_LAST                ((GXHTREEITEM)ULONG_GXTVI_LAST)
#define GXTVI_SORT                ((GXHTREEITEM)ULONG_GXTVI_SORT)

#define LPTV_INSERTSTRUCTA      LPTVINSERTSTRUCTA
#define LPTV_INSERTSTRUCTW      LPTVINSERTSTRUCTW
#define GXTV_INSERTSTRUCTA        TVINSERTSTRUCTA
#define GXTV_INSERTSTRUCTW        TVINSERTSTRUCTW



#define TVINSERTSTRUCTA_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTA, item)
#define TVINSERTSTRUCTW_V1_SIZE CCSIZEOF_STRUCT(TVINSERTSTRUCTW, item)

typedef struct tagGXTVINSERTSTRUCTA {
  GXHTREEITEM hParent;
  GXHTREEITEM hInsertAfter;
  union
  {
    GXTVITEMEXA itemex;
    GXTV_ITEMA  item;
  } DUMMYUNIONNAME;
} GXTVINSERTSTRUCTA, *GXLPTVINSERTSTRUCTA;

typedef struct tagGXTVINSERTSTRUCTW {
  GXHTREEITEM hParent;
  GXHTREEITEM hInsertAfter;
  union
  {
    GXTVITEMEXW itemex;
    GXTV_ITEMW  item;
  }u /*DUMMYUNIONNAME*/;
} GXTVINSERTSTRUCTW, *GXLPTVINSERTSTRUCTW, *LPGXTVINSERTSTRUCTW;

#ifdef _UNICODE
#define  GXTVINSERTSTRUCT         GXTVINSERTSTRUCTW
#define  GXLPTVINSERTSTRUCT       GXLPTVINSERTSTRUCTW
#define GXTVINSERTSTRUCT_V1_SIZE GXTVINSERTSTRUCTW_V1_SIZE
#else
#define  GXTVINSERTSTRUCT         GXTVINSERTSTRUCTA
#define  GXLPTVINSERTSTRUCT       GXLPTVINSERTSTRUCTA
#define GXTVINSERTSTRUCT_V1_SIZE GXTVINSERTSTRUCTA_V1_SIZE
#endif

#define GXTV_INSERTSTRUCT         GXTVINSERTSTRUCT
#define GXLPTV_INSERTSTRUCT       GXLPTVINSERTSTRUCT


#define GXTVM_INSERTITEMA         (GXTV_FIRST + 0)
#define GXTVM_INSERTITEMW         (GXTV_FIRST + 50)
#ifdef _UNICODE
#define  GXTVM_INSERTITEM         GXTVM_INSERTITEMW
#else
#define  GXTVM_INSERTITEM         GXTVM_INSERTITEMA
#endif

#define GXTreeView_InsertItem(hwnd, lpis) \
  (GXHTREEITEM)GXSNDMSG((hwnd), GXTVM_INSERTITEM, 0, (GXLPARAM)(LPTV_INSERTSTRUCT)(lpis))


#define GXTVM_DELETEITEM          (GXTV_FIRST + 1)
#define GXTreeView_DeleteItem(hwnd, hitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_DELETEITEM, 0, (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTreeView_DeleteAllItems(hwnd) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_DELETEITEM, 0, (GXLPARAM)TVI_ROOT)


#define GXTVM_EXPAND              (GXTV_FIRST + 2)
#define GXTreeView_Expand(hwnd, hitem, code) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_EXPAND, (GXWPARAM)(code), (GXLPARAM)(GXHTREEITEM)(hitem))


#define TVE_COLLAPSE            0x0001
#define TVE_EXPAND              0x0002
#define TVE_TOGGLE              0x0003
#define TVE_EXPANDPARTIAL       0x4000
#define TVE_COLLAPSERESET       0x8000


#define GXTVM_GETITEMRECT         (GXTV_FIRST + 4)
#define GXTreeView_GetItemRect(hwnd, hitem, prc, code) \
  (*(GXHTREEITEM *)prc = (hitem), (GXBOOL)GXSNDMSG((hwnd), GXTVM_GETITEMRECT, (GXWPARAM)(code), (GXLPARAM)(RECT *)(prc)))


#define GXTVM_GETCOUNT            (GXTV_FIRST + 5)
#define GXTreeView_GetCount(hwnd) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_GETCOUNT, 0, 0)


#define GXTVM_GETINDENT           (GXTV_FIRST + 6)
#define GXTreeView_GetIndent(hwnd) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_GETINDENT, 0, 0)


#define GXTVM_SETINDENT           (GXTV_FIRST + 7)
#define GXTreeView_SetIndent(hwnd, indent) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SETINDENT, (GXWPARAM)(indent), 0)


#define GXTVM_GETIMAGELIST        (GXTV_FIRST + 8)
#define GXTreeView_GetImageList(hwnd, iImage) \
  (HIMAGELIST)GXSNDMSG((hwnd), GXTVM_GETIMAGELIST, iImage, 0)


#define TVSIL_NORMAL            0
#define TVSIL_STATE             2


#define GXTVM_SETIMAGELIST        (GXTV_FIRST + 9)
#define GXTreeView_SetImageList(hwnd, himl, iImage) \
  (HIMAGELIST)GXSNDMSG((hwnd), GXTVM_SETIMAGELIST, iImage, (GXLPARAM)(HIMAGELIST)(himl))


#define GXTVM_GETNEXTITEM         (GXTV_FIRST + 10)
#define GXTreeView_GetNextItem(hwnd, hitem, code) \
  (GXHTREEITEM)GXSNDMSG((hwnd), GXTVM_GETNEXTITEM, (GXWPARAM)(code), (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTVGN_ROOT               0x0000
#define GXTVGN_NEXT               0x0001
#define GXTVGN_PREVIOUS           0x0002
#define GXTVGN_PARENT             0x0003
#define GXTVGN_CHILD              0x0004
#define GXTVGN_FIRSTVISIBLE       0x0005
#define GXTVGN_NEXTVISIBLE        0x0006
#define GXTVGN_PREVIOUSVISIBLE    0x0007
#define GXTVGN_DROPHILITE         0x0008
#define GXTVGN_CARET              0x0009
#define GXTVGN_LASTVISIBLE        0x000A
#define GXTVGN_NEXTSELECTED       0x000B

#define TVSI_NOSINGLEEXPAND    0x8000 // Should not conflict with TVGN flags.


#define TVSI_NOSINGLEEXPAND    0x8000 // Should not conflict with TVGN flags.

#define GXTreeView_GetChild(hwnd, hitem)          GXTreeView_GetNextItem(hwnd, hitem, TVGN_CHILD)
#define GXTreeView_GetNextSibling(hwnd, hitem)    GXTreeView_GetNextItem(hwnd, hitem, TVGN_NEXT)
#define GXTreeView_GetPrevSibling(hwnd, hitem)    GXTreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUS)
#define GXTreeView_GetParent(hwnd, hitem)         GXTreeView_GetNextItem(hwnd, hitem, TVGN_PARENT)
#define GXTreeView_GetFirstVisible(hwnd)          GXTreeView_GetNextItem(hwnd, NULL,  TVGN_FIRSTVISIBLE)
#define GXTreeView_GetNextVisible(hwnd, hitem)    GXTreeView_GetNextItem(hwnd, hitem, TVGN_NEXTVISIBLE)
#define GXTreeView_GetPrevVisible(hwnd, hitem)    GXTreeView_GetNextItem(hwnd, hitem, TVGN_PREVIOUSVISIBLE)
#define GXTreeView_GetSelection(hwnd)             GXTreeView_GetNextItem(hwnd, NULL,  TVGN_CARET)
#define GXTreeView_GetDropHilight(hwnd)           GXTreeView_GetNextItem(hwnd, NULL,  TVGN_DROPHILITE)
#define GXTreeView_GetRoot(hwnd)                  GXTreeView_GetNextItem(hwnd, NULL,  TVGN_ROOT)
#define GXTreeView_GetLastVisible(hwnd)          GXTreeView_GetNextItem(hwnd, NULL,  TVGN_LASTVISIBLE)


#define GXTVM_SELECTITEM          (GXTV_FIRST + 11)
#define GXTreeView_Select(hwnd, hitem, code) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SELECTITEM, (GXWPARAM)(code), (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTreeView_SelectItem(hwnd, hitem)            GXTreeView_Select(hwnd, hitem, TVGN_CARET)
#define GXTreeView_SelectDropTarget(hwnd, hitem)      GXTreeView_Select(hwnd, hitem, TVGN_DROPHILITE)
#define GXTreeView_SelectSetFirstVisible(hwnd, hitem) GXTreeView_Select(hwnd, hitem, TVGN_FIRSTVISIBLE)

#define GXTVM_GETITEMA            (GXTV_FIRST + 12)
#define GXTVM_GETITEMW            (GXTV_FIRST + 62)

#ifdef _UNICODE
#define  GXTVM_GETITEM            GXTVM_GETITEMW
#else
#define  GXTVM_GETITEM            GXTVM_GETITEMA
#endif

#define GXTreeView_GetItem(hwnd, pitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_GETITEM, 0, (GXLPARAM)(GXTV_ITEM *)(pitem))


#define GXTVM_SETITEMA            (GXTV_FIRST + 13)
#define GXTVM_SETITEMW            (GXTV_FIRST + 63)

#ifdef _UNICODE
#define  GXTVM_SETITEM            GXTVM_SETITEMW
#else
#define  GXTVM_SETITEM            GXTVM_SETITEMA
#endif

#define GXTreeView_SetItem(hwnd, pitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SETITEM, 0, (GXLPARAM)(const GXTV_ITEM *)(pitem))


#define GXTVM_EDITLABELA          (GXTV_FIRST + 14)
#define GXTVM_EDITLABELW          (GXTV_FIRST + 65)
#ifdef _UNICODE
#define GXTVM_EDITLABEL           GXTVM_EDITLABELW
#else
#define GXTVM_EDITLABEL           GXTVM_EDITLABELA
#endif

#define GXTreeView_EditLabel(hwnd, hitem) \
  (GXHWND)GXSNDMSG((hwnd), GXTVM_EDITLABEL, 0, (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTVM_GETEDITCONTROL      (GXTV_FIRST + 15)
#define GXTreeView_GetEditControl(hwnd) \
  (GXHWND)GXSNDMSG((hwnd), GXTVM_GETEDITCONTROL, 0, 0)


#define GXTVM_GETVISIBLECOUNT     (GXTV_FIRST + 16)
#define GXTreeView_GetVisibleCount(hwnd) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_GETVISIBLECOUNT, 0, 0)


#define GXTVM_HITTEST             (GXTV_FIRST + 17)
#define GXTreeView_HitTest(hwnd, lpht) \
  (GXHTREEITEM)GXSNDMSG((hwnd), GXTVM_HITTEST, 0, (GXLPARAM)(LPTV_HITTESTINFO)(lpht))


#define GXLPTV_HITTESTINFO   GXLPTVHITTESTINFO
#define GXTV_HITTESTINFO     GXTVHITTESTINFO

typedef struct tagGXTVHITTESTINFO {
  GXPOINT       pt;
  GXUINT        flags;
  GXHTREEITEM   hItem;
} GXTVHITTESTINFO, *GXLPTVHITTESTINFO;

#define TVHT_NOWHERE            0x0001
#define TVHT_ONITEMICON         0x0002
#define TVHT_ONITEMLABEL        0x0004
#define TVHT_ONITEM             (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMSTATEICON)
#define TVHT_ONITEMINDENT       0x0008
#define TVHT_ONITEMBUTTON       0x0010
#define TVHT_ONITEMRIGHT        0x0020
#define TVHT_ONITEMSTATEICON    0x0040

#define TVHT_ABOVE              0x0100
#define TVHT_BELOW              0x0200
#define TVHT_TORIGHT            0x0400
#define TVHT_TOLEFT             0x0800


#define GXTVM_CREATEDRAGIMAGE     (GXTV_FIRST + 18)
#define GXTreeView_CreateDragImage(hwnd, hitem) \
  (HIMAGELIST)GXSNDMSG((hwnd), GXTVM_CREATEDRAGIMAGE, 0, (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTVM_SORTCHILDREN        (GXTV_FIRST + 19)
#define GXTreeView_SortChildren(hwnd, hitem, recurse) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SORTCHILDREN, (GXWPARAM)(recurse), (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTVM_ENSUREVISIBLE       (GXTV_FIRST + 20)
#define GXTreeView_EnsureVisible(hwnd, hitem) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_ENSUREVISIBLE, 0, (GXLPARAM)(GXHTREEITEM)(hitem))


#define GXTVM_SORTCHILDRENCB      (GXTV_FIRST + 21)
#define GXTreeView_SortChildrenCB(hwnd, psort, recurse) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SORTCHILDRENCB, (GXWPARAM)(recurse), \
  (GXLPARAM)(LPTV_SORTCB)(psort))


#define GXTVM_ENDEDITLABELNOW     (GXTV_FIRST + 22)
#define GXTreeView_EndEditLabelNow(hwnd, fCancel) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_ENDEDITLABELNOW, (GXWPARAM)(fCancel), 0)


#define GXTVM_GETISEARCHSTRINGA   (GXTV_FIRST + 23)
#define GXTVM_GETISEARCHSTRINGW   (GXTV_FIRST + 64)

#ifdef _UNICODE
#define GXTVM_GETISEARCHSTRING     GXTVM_GETISEARCHSTRINGW
#else
#define GXTVM_GETISEARCHSTRING     GXTVM_GETISEARCHSTRINGA
#endif

#define GXTVM_SETTOOLTIPS         (GXTV_FIRST + 24)
#define GXTreeView_SetToolTips(hwnd,  hwndTT) \
  (GXHWND)GXSNDMSG((hwnd), GXTVM_SETTOOLTIPS, (GXWPARAM)(hwndTT), 0)
#define GXTVM_GETTOOLTIPS         (GXTV_FIRST + 25)
#define GXTreeView_GetToolTips(hwnd) \
  (GXHWND)GXSNDMSG((hwnd), GXTVM_GETTOOLTIPS, 0, 0)

#define GXTreeView_GetISearchString(hwndTV, lpsz) \
  (GXBOOL)GXSNDMSG((hwndTV), GXTVM_GETISEARCHSTRING, 0, (GXLPARAM)(LPTSTR)(lpsz))

#define GXTVM_SETINSERTMARK       (GXTV_FIRST + 26)
#define GXTreeView_SetInsertMark(hwnd, hItem, fAfter) \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SETINSERTMARK, (GXWPARAM) (fAfter), (GXLPARAM) (hItem))

#define GXTVM_SETUNICODEFORMAT     GXCCM_SETUNICODEFORMAT
#define GXTreeView_SetUnicodeFormat(hwnd, fUnicode)  \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_SETUNICODEFORMAT, (GXWPARAM)(fUnicode), 0)

#define GXTVM_GETUNICODEFORMAT     GXCCM_GETUNICODEFORMAT
#define GXTreeView_GetUnicodeFormat(hwnd)  \
  (GXBOOL)GXSNDMSG((hwnd), GXTVM_GETUNICODEFORMAT, 0, 0)


#define GXTVM_SETITEMHEIGHT         (GXTV_FIRST + 27)
#define GXTreeView_SetItemHeight(hwnd,  iHeight) \
  (int)GXSNDMSG((hwnd), GXTVM_SETITEMHEIGHT, (GXWPARAM)(iHeight), 0)
#define GXTVM_GETITEMHEIGHT         (GXTV_FIRST + 28)
#define GXTreeView_GetItemHeight(hwnd) \
  (int)GXSNDMSG((hwnd), GXTVM_GETITEMHEIGHT, 0, 0)

#define GXTVM_SETBKCOLOR              (GXTV_FIRST + 29)
#define GXTreeView_SetBkColor(hwnd, clr) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_SETBKCOLOR, 0, (GXLPARAM)(clr))

#define GXTVM_SETTEXTCOLOR              (GXTV_FIRST + 30)
#define GXTreeView_SetTextColor(hwnd, clr) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_SETTEXTCOLOR, 0, (GXLPARAM)(clr))

#define GXTVM_GETBKCOLOR              (GXTV_FIRST + 31)
#define GXTreeView_GetBkColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_GETBKCOLOR, 0, 0)

#define GXTVM_GETTEXTCOLOR              (GXTV_FIRST + 32)
#define GXTreeView_GetTextColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_GETTEXTCOLOR, 0, 0)

#define GXTVM_SETSCROLLTIME              (GXTV_FIRST + 33)
#define GXTreeView_SetScrollTime(hwnd, uTime) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_SETSCROLLTIME, uTime, 0)

#define GXTVM_GETSCROLLTIME              (GXTV_FIRST + 34)
#define GXTreeView_GetScrollTime(hwnd) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_GETSCROLLTIME, 0, 0)


#define GXTVM_SETINSERTMARKCOLOR              (GXTV_FIRST + 37)
#define GXTreeView_SetInsertMarkColor(hwnd, clr) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_SETINSERTMARKCOLOR, 0, (GXLPARAM)(clr))
#define GXTVM_GETINSERTMARKCOLOR              (GXTV_FIRST + 38)
#define GXTreeView_GetInsertMarkColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_GETINSERTMARKCOLOR, 0, 0)


// tvm_?etitemstate only uses mask, state and stateMask.
// so unicode or ansi is irrelevant.
#define GXTreeView_SetItemState(hwndTV, hti, data, _mask) \
{ TVITEM _ms_TVi;\
  _ms_TVi.mask = TVIF_STATE; \
  _ms_TVi.hItem = hti; \
  _ms_TVi.stateMask = _mask;\
  _ms_TVi.state = data;\
  GXSNDMSG((hwndTV), GXTVM_SETITEM, 0, (GXLPARAM)(GXTV_ITEM *)&_ms_TVi);\
}

#define GXTreeView_SetCheckState(hwndTV, hti, fCheck) \
  GXTreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), TVIS_STATEIMAGEMASK)

#define GXTVM_GETITEMSTATE        (GXTV_FIRST + 39)
#define GXTreeView_GetItemState(hwndTV, hti, mask) \
  (GXUINT)GXSNDMSG((hwndTV), GXTVM_GETITEMSTATE, (GXWPARAM)(hti), (GXLPARAM)(mask))

#define GXTreeView_GetCheckState(hwndTV, hti) \
  ((((GXUINT)(GXSNDMSG((hwndTV), GXTVM_GETITEMSTATE, (GXWPARAM)(hti), TVIS_STATEIMAGEMASK))) >> 12) -1)


#define GXTVM_SETLINECOLOR            (GXTV_FIRST + 40)
#define GXTreeView_SetLineColor(hwnd, clr) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_SETLINECOLOR, 0, (GXLPARAM)(clr))

#define GXTVM_GETLINECOLOR            (GXTV_FIRST + 41)
#define GXTreeView_GetLineColor(hwnd) \
  (GXCOLORREF)GXSNDMSG((hwnd), GXTVM_GETLINECOLOR, 0, 0)


#if (_WIN32_WINNT >= 0x0501)
#define GXTVM_MAPACCIDTOHTREEITEM     (GXTV_FIRST + 42)
#define GXTreeView_MapAccIDToHTREEITEM(hwnd, id) \
  (GXHTREEITEM)GXSNDMSG((hwnd), GXTVM_MAPACCIDTOHTREEITEM, id, 0)

#define GXTVM_MAPHTREEITEMTOACCID     (GXTV_FIRST + 43)
#define GXTreeView_MapHTREEITEMToAccID(hwnd, htreeitem) \
  (GXUINT)GXSNDMSG((hwnd), GXTVM_MAPHTREEITEMTOACCID, (GXWPARAM)htreeitem, 0)


#endif

typedef int (GXCALLBACK *GXPFNTVCOMPARE)(GXLPARAM lParam1, GXLPARAM lParam2, GXLPARAM lParamSort);

#define LPTV_SORTCB    LPTVSORTCB
#define   GXTV_SORTCB      TVSORTCB

typedef struct tagGXTVSORTCB
{
  GXHTREEITEM       hParent;
  GXPFNTVCOMPARE    lpfnCompare;
  GXLPARAM          lParam;
} GXTVSORTCB, *GXLPTVSORTCB;


#define LPNM_TREEVIEWA          LPNMTREEVIEWA
#define LPNM_TREEVIEWW          LPNMTREEVIEWW
#define NM_TREEVIEWW            NMTREEVIEWW
#define NM_TREEVIEWA            NMTREEVIEWA

#define LPNM_TREEVIEW           LPNMTREEVIEW
#define NM_TREEVIEW             NMTREEVIEW

//struct GXNMTREEVIEWA {
//  GXNMHDR       hdr;
//  GXUINT        action;
//  GXTVITEMA    itemOld;
//  GXTVITEMA    itemNew;
//  GXPOINT       ptDrag;
//};
//typedef GXNMTREEVIEWA *GXLPNMTREEVIEWA, *LPGXNMTREEVIEWA;


struct GXNMTREEVIEWW{
  GXNMHDR    hdr;
  GXUINT     action;
  GXTVITEMW    itemOld;
  GXTVITEMW    itemNew;
  GXPOINT    ptDrag;
};
typedef GXNMTREEVIEWW GXNMTREEVIEW;
typedef GXNMTREEVIEWW *GXLPNMTREEVIEWW, *LPGXNMTREEVIEWW;
typedef GXNMTREEVIEW *GXLPNMTREEVIEW, *LPGXNMTREEVIEW;


#ifdef _UNICODE
#define  NMTREEVIEW             NMTREEVIEWW
#define  LPNMTREEVIEW           LPNMTREEVIEWW
#else
#define  NMTREEVIEW             NMTREEVIEWA
#define  LPNMTREEVIEW           LPNMTREEVIEWA
#endif


#define GXTVN_SELCHANGINGA        (GXTVN_FIRST-1)
#define GXTVN_SELCHANGINGW        (GXTVN_FIRST-50)
#define GXTVN_SELCHANGEDA         (GXTVN_FIRST-2)
#define GXTVN_SELCHANGEDW         (GXTVN_FIRST-51)

#define TVC_UNKNOWN             0x0000
#define TVC_BYMOUSE             0x0001
#define TVC_BYKEYBOARD          0x0002

#define GXTVN_GETDISPINFOA        (GXTVN_FIRST-3)
#define GXTVN_GETDISPINFOW        (GXTVN_FIRST-52)
#define GXTVN_SETDISPINFOA        (GXTVN_FIRST-4)
#define GXTVN_SETDISPINFOW        (GXTVN_FIRST-53)

#define TVIF_DI_SETITEM         0x1000

#define GXTV_DISPINFOA            NMTVDISPINFOA
#define GXTV_DISPINFOW            NMTVDISPINFOW

#define GXTV_DISPINFO             NMTVDISPINFO

typedef struct tagGXTVDISPINFOA {
  GXNMHDR hdr;
  GXTVITEMA item;
} GXNMTVDISPINFOA, *GXLPNMTVDISPINFOA;

typedef struct tagGXTVDISPINFOW {
  GXNMHDR hdr;
  GXTVITEMW item;
} GXNMTVDISPINFOW, *GXLPNMTVDISPINFOW;


#ifdef _UNICODE
#define NMTVDISPINFO            NMTVDISPINFOW
#define LPNMTVDISPINFO          LPNMTVDISPINFOW
#else
#define NMTVDISPINFO            NMTVDISPINFOA
#define LPNMTVDISPINFO          LPNMTVDISPINFOA
#endif

#define GXTVN_ITEMEXPANDINGA      (GXTVN_FIRST-5)
#define GXTVN_ITEMEXPANDINGW      (GXTVN_FIRST-54)
#define GXTVN_ITEMEXPANDEDA       (GXTVN_FIRST-6)
#define GXTVN_ITEMEXPANDEDW       (GXTVN_FIRST-55)
#define GXTVN_BEGINDRAGA          (GXTVN_FIRST-7)
#define GXTVN_BEGINDRAGW          (GXTVN_FIRST-56)
#define GXTVN_BEGINRDRAGA         (GXTVN_FIRST-8)
#define GXTVN_BEGINRDRAGW         (GXTVN_FIRST-57)
#define GXTVN_DELETEITEMA         (GXTVN_FIRST-9)
#define GXTVN_DELETEITEMW         (GXTVN_FIRST-58)
#define GXTVN_BEGINLABELEDITA     (GXTVN_FIRST-10)
#define GXTVN_BEGINLABELEDITW     (GXTVN_FIRST-59)
#define GXTVN_ENDLABELEDITA       (GXTVN_FIRST-11)
#define GXTVN_ENDLABELEDITW       (GXTVN_FIRST-60)
#define GXTVN_KEYDOWN             (GXTVN_FIRST-12)

#define GXTVN_GETINFOTIPA         (GXTVN_FIRST-13)
#define GXTVN_GETINFOTIPW         (GXTVN_FIRST-14)
#define GXTVN_SINGLEEXPAND        (GXTVN_FIRST-15)

#define TVNRET_DEFAULT          0
#define TVNRET_SKIPOLD          1
#define TVNRET_SKIPNEW          2



#define GXTV_KEYDOWN      NMTVKEYDOWN

#ifdef _WIN32
#include <pshpack1.h>
#endif

typedef struct tagGXTVKEYDOWN {
  GXNMHDR hdr;
  GXWORD wVKey;
  GXUINT flags;
} GXNMTVKEYDOWN, *GXLPNMTVKEYDOWN;

#ifdef _WIN32
#include <poppack.h>
#endif


#ifdef _UNICODE
#define GXTVN_SELCHANGING         GXTVN_SELCHANGINGW
#define GXTVN_SELCHANGED          GXTVN_SELCHANGEDW
#define GXTVN_GETDISPINFO         GXTVN_GETDISPINFOW
#define GXTVN_SETDISPINFO         GXTVN_SETDISPINFOW
#define GXTVN_ITEMEXPANDING       GXTVN_ITEMEXPANDINGW
#define GXTVN_ITEMEXPANDED        GXTVN_ITEMEXPANDEDW
#define GXTVN_BEGINDRAG           GXTVN_BEGINDRAGW
#define GXTVN_BEGINRDRAG          GXTVN_BEGINRDRAGW
#define GXTVN_DELETEITEM          GXTVN_DELETEITEMW
#define GXTVN_BEGINLABELEDIT      GXTVN_BEGINLABELEDITW
#define GXTVN_ENDLABELEDIT        GXTVN_ENDLABELEDITW
#else
#define GXTVN_SELCHANGING         GXTVN_SELCHANGINGA
#define GXTVN_SELCHANGED          GXTVN_SELCHANGEDA
#define GXTVN_GETDISPINFO         GXTVN_GETDISPINFOA
#define GXTVN_SETDISPINFO         GXTVN_SETDISPINFOA
#define GXTVN_ITEMEXPANDING       GXTVN_ITEMEXPANDINGA
#define GXTVN_ITEMEXPANDED        GXTVN_ITEMEXPANDEDA
#define GXTVN_BEGINDRAG           GXTVN_BEGINDRAGA
#define GXTVN_BEGINRDRAG          GXTVN_BEGINRDRAGA
#define GXTVN_DELETEITEM          GXTVN_DELETEITEMA
#define GXTVN_BEGINLABELEDIT      GXTVN_BEGINLABELEDITA
#define GXTVN_ENDLABELEDIT        GXTVN_ENDLABELEDITA
#endif

#define NMTVCUSTOMDRAW_V3_SIZE CCSIZEOF_STRUCT(NMTVCUSTOMDRAW, clrTextBk)

typedef struct tagGXNMTVCUSTOMDRAW
{
  GXNMCUSTOMDRAW nmcd;
  GXCOLORREF     clrText;
  GXCOLORREF     clrTextBk;
  int iLevel;
} GXNMTVCUSTOMDRAW, *GXLPNMTVCUSTOMDRAW, *LPGXNMTVCUSTOMDRAW;



// for tooltips

typedef struct tagGXNMTVGETINFOTIPA
{
  GXNMHDR hdr;
  GXLPSTR pszText;
  int cchTextMax;
  GXHTREEITEM hItem;
  GXLPARAM lParam;
} GXNMTVGETINFOTIPA, *GXLPNMTVGETINFOTIPA;

typedef struct tagGXNMTVGETINFOTIPW
{
  GXNMHDR hdr;
  GXLPWSTR pszText;
  int cchTextMax;
  GXHTREEITEM hItem;
  GXLPARAM lParam;
} GXNMTVGETINFOTIPW, *GXLPNMTVGETINFOTIPW;


#ifdef _UNICODE
#define GXTVN_GETINFOTIP          GXTVN_GETINFOTIPW
#define NMTVGETINFOTIP          NMTVGETINFOTIPW
#define LPNMTVGETINFOTIP        LPNMTVGETINFOTIPW
#else
#define GXTVN_GETINFOTIP          GXTVN_GETINFOTIPA
#define NMTVGETINFOTIP          NMTVGETINFOTIPA
#define LPNMTVGETINFOTIP        LPNMTVGETINFOTIPA
#endif

// treeview's customdraw return meaning don't draw images.  valid on CDRF_NOTIFYITEMPREPAINT
#define TVCDRF_NOIMAGES         0x00010000

#endif      // NOTREEVIEW