// #define _WIN32_WINDOWS 0x0400
// #define _WIN32_WINNT  0x0400

#ifndef    _WINE_COMMON_DFEINE_FILE_
#define    _WINE_COMMON_DFEINE_FILE_

#define _WINE_MENU_1_3_20_

typedef GXUINT16        HANDLE16;
typedef HANDLE16        HLOCAL16;
typedef GXINT16*        GXLPINT16;
typedef GXUINT16        BOOL16;
#define HGLOBAL16      HGLOBAL
typedef GXINT16   (CALLBACK *EDITWORDBREAKPROC16)(GXLPSTR,GXINT16,GXINT16,GXINT16);
// #define TRACE
//#define FIXME  TRACE
//#define ERR    TRACE
//#define WARN  TRACE
#define FIXME(...)
#define ERR(...)
#define WARN(...)
#define LB_TRACE
#define ED_TRACE(...)
#define ST_TRACE
#define LocalFree16 gxLocalFree
#define LocalInit16 
#define LocalAlloc16 gxLocalAlloc
#define LocalFree16 gxLocalFree
#define LocalSize16 gxLocalSize
#define GlobalAlloc16 GlobalAlloc
#define LocalReAlloc16 gxLocalReAlloc
#define LocalSize16 gxLocalSize
#define LocalUnlock16 gxLocalUnlock
#define STACK16FRAME STACKFRAME
#define debugstr_w(x)    (x)
#define debugstr_a(x)    (x)
#define debugstr_wn(___TEXT, ___N)    (___TEXT)
#define debugstr_an(___TEXT, ___N)    (___TEXT)

// TODO: È¥µô!
#define strcpyW    lstrcpyW
#define strcmpW    lstrcmpW
#define strcatW    lstrcatW
#define strncmpiW  lstrncmpiW
#define strstrW    wcsstr

#define _gxTrackMouseEvent gxTrackMouseEvent
//#define strncmpiW ASSERT(FALSE)
#define HINSTANCE16 HANDLE16
//#define TRACE_ TRACE
#define GXEM_GETTHUMB16 GXEM_GETTHUMB
#define GXES_COMBO        0x00000200   /* Undocumented. Parent is a combobox */


#define COORDS_CLIENT 0
#define COORDS_WINDOW 1

#define CBF_DROPPED        0x0001
#define CBF_NOROLLUP      0x0004
#define CBF_SELCHANGE      0x0400
#define CBF_EUI          0x8000

//#define WM_SYSTIMER      0x0118
#define GXWM_SYSTIMER      GXWM_TIMER
#define GXWM_LBTRACKPOINT    0x0131
#define GXWM_BEGINDRAG      0x022C

#define GXWS_EX_DRAGDETECT    0x00000002L

/*
 * Combo Box messages
 */
#define GXCB_GETEDITSEL               0x0140
#define GXCB_LIMITTEXT                0x0141
#define GXCB_SETEDITSEL               0x0142
#define GXCB_ADDSTRING                0x0143
#define GXCB_DELETESTRING             0x0144
#define GXCB_DIR                      0x0145
#define GXCB_GETCOUNT                 0x0146
#define GXCB_GETCURSEL                0x0147
#define GXCB_GETLBTEXT                0x0148
#define GXCB_GETLBTEXTLEN             0x0149
#define GXCB_INSERTSTRING             0x014A
#define GXCB_RESETCONTENT             0x014B
#define GXCB_FINDSTRING               0x014C
#define GXCB_SELECTSTRING             0x014D
#define GXCB_SETCURSEL                0x014E
#define GXCB_SHOWDROPDOWN             0x014F
#define GXCB_GETITEMDATA              0x0150
#define GXCB_SETITEMDATA              0x0151
#define GXCB_GETDROPPEDCONTROLRECT    0x0152
#define GXCB_SETITEMHEIGHT            0x0153
#define GXCB_GETITEMHEIGHT            0x0154
#define GXCB_SETEXTENDEDUI            0x0155
#define GXCB_GETEXTENDEDUI            0x0156
#define GXCB_GETDROPPEDSTATE          0x0157
#define GXCB_FINDSTRINGEXACT          0x0158
#define GXCB_SETLOCALE                0x0159
#define GXCB_GETLOCALE                0x015A
#define GXCB_GETTOPINDEX              0x015b
#define GXCB_SETTOPINDEX              0x015c
#define GXCB_GETHORIZONTALEXTENT      0x015d
#define GXCB_SETHORIZONTALEXTENT      0x015e
#define GXCB_GETDROPPEDWIDTH          0x015f
#define GXCB_SETDROPPEDWIDTH          0x0160
#define GXCB_INITSTORAGE              0x0161
#define GXCB_MULTIPLEADDSTRING        0x0163
#define GXCB_GETCOMBOBOXINFO          0x0164
#define GXCB_MSGMAX                   0x015B

#define COMCTL32_hModule NULL
#pragma comment(lib, "imm32.lib")

/* combo state struct */
typedef struct
{
  GXHWND           self;
  GXHWND           owner;
  GXUINT           dwStyle;
  GXHWND           hWndEdit;
  GXHWND           hWndLBox;
  GXUINT           wState;
  GXHFONT          hFont;
  GXRECT           textRect;
  GXRECT           buttonRect;
  GXRECT           droppedRect;
  GXINT            droppedIndex;
  GXINT            fixedOwnerDrawHeight;
  GXINT            droppedWidth;   /* last two are not used unless set */
  GXINT            editHeight;     /* explicitly */
} GXHEADCOMBO,*GXLPHEADCOMBO;

/*
 * Combobox information
 */
typedef struct
{
    GXDWORD cbSize;
    GXRECT rcItem;
    GXRECT rcButton;
    GXDWORD stateButton;
    GXHWND hwndCombo;
    GXHWND hwndItem;
    GXHWND hwndList;
} GXCOMBOBOXINFO, *GXPCOMBOBOXINFO, *GXLPCOMBOBOXINFO;



typedef struct tagCURSORICONINFO
{
  GXPOINT   ptHotSpot;
  GXWORD    nWidth;
  GXWORD    nHeight;
  GXWORD    nWidthBytes;
  GXBYTE    bPlanes;
  GXBYTE    bBitsPerPixel;
} CURSORICONINFO;

//
//#define GXSCRIPT_UNDEFINED  0
//
///////   ScriptStringAnalyse
////
////
//#define GXSSA_PASSWORD         0x00000001  // Input string contains a single character to be duplicated iLength times
//#define GXSSA_TAB              0x00000002  // Expand tabs
//#define GXSSA_CLIP             0x00000004  // Clip string at iReqWidth
//#define GXSSA_FIT              0x00000008  // Justify string to iReqWidth
//#define GXSSA_DZWG             0x00000010  // Provide representation glyphs for control characters
//#define GXSSA_FALLBACK         0x00000020  // Use fallback fonts
//#define GXSSA_BREAK            0x00000040  // Return break flags (character and word stops)
//#define GXSSA_GLYPHS           0x00000080  // Generate glyphs, positions and attributes
//#define GXSSA_RTL              0x00000100  // Base embedding level 1
//#define GXSSA_GCP              0x00000200  // Return missing glyphs and LogCLust with GetCharacterPlacement conventions
//#define GXSSA_HOTKEY           0x00000400  // Replace '&' with underline on subsequent codepoint
//#define GXSSA_METAFILE         0x00000800  // Write items with ExtTextOutW Unicode calls, not glyphs
//#define GXSSA_LINK             0x00001000  // Apply FE font linking/association to non-complex text
//#define GXSSA_HIDEHOTKEY       0x00002000  // Remove first '&' from displayed string
//#define GXSSA_HOTKEYONLY       0x00002400  // Display underline only.
//
//#define GXSSA_FULLMEASURE      0x04000000  // Internal - calculate full width and out the number of chars can fit in iReqWidth.
//#define GXSSA_LPKANSIFALLBACK  0x08000000  // Internal - enable FallBack for all LPK Ansi calls Except BiDi hDC calls
//#define GXSSA_PIDX             0x10000000  // Internal
//#define GXSSA_LAYOUTRTL        0x20000000  // Internal - Used when DC is mirrored
//#define GXSSA_DONTGLYPH        0x40000000  // Internal - Used only by GDI during metafiling - Use ExtTextOutA for positioning
//#define GXSSA_NOKASHIDA        0x80000000  // Internal - Used by GCP to justify the non Arabic glyphs only.
//
///////   SCRIPT_TABDEF
////
////      Defines tabstop positions for ScriptStringAnalyse (ignored unless SSA_TAB passed)
////
//typedef struct tag_GXSCRIPT_TABDEF {
//  int   cTabStops;        // Number of entries in pTabStops array
//  int   iScale;           // Scale factor for pTabStops (see below)
//  int  *pTabStops;        // Pointer to array of one or more tab stops
//  int   iTabOrigin;       // Initial offset for tab stops (logical units)
//} GXSCRIPT_TABDEF;
//
//
///////   SCRIPT_STATE
////
////      The SCRIPT_STATE structure is used both to initialise the unicode
////      algorithm state as an input parameter to ScriptItemize, and is also
////      a component of each item analysis returned by ScriptItemize.
////
////
//typedef struct tag_GXSCRIPT_STATE {
//  GXWORD    uBidiLevel         :5;  // Unicode Bidi algorithm embedding level (0-16)
//  GXWORD    fOverrideDirection :1;  // Set when in LRO/RLO embedding
//  GXWORD    fInhibitSymSwap    :1;  // Set by U+206A (ISS), cleared by U+206B (ASS)
//  GXWORD    fCharShape         :1;  // Set by U+206D (AAFS), cleared by U+206C (IAFS)
//  GXWORD    fDigitSubstitute   :1;  // Set by U+206E (NADS), cleared by U+206F (NODS)
//  GXWORD    fInhibitLigate     :1;  // Equiv !GCP_Ligate, no Unicode control chars yet
//  GXWORD    fDisplayZWG        :1;  // Equiv GCP_DisplayZWG, no Unicode control characters yet
//  GXWORD    fArabicNumContext  :1;  // For EN->AN Unicode rule
//  GXWORD    fGcpClusters       :1;  // For Generating Backward Compatible GCP Clusters (legacy Apps)
//  GXWORD    fReserved          :1;
//  GXWORD    fEngineReserved    :2;  // For use by shaping engine
//} GXSCRIPT_STATE;
//
///////   SCRIPT_LOGATTR
////
////      The SCRIPT_LOGATTR structure describes attributes of logical
////      characters useful when editing and formatting text.
////
////      Note that for wordbreaking and linebreaking, if the first character of
////      the run passed in is not whitespace, the client needs to check whether
////      the last character of the previous run is whitespace to determine if
////      the first character of this run is the start of a word.
////
////
//typedef struct tag_GXSCRIPT_LOGATTR {
//  GXBYTE    fSoftBreak      :1;     // Potential linebreak point
//  GXBYTE    fWhiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
//  GXBYTE    fCharStop       :1;     // Valid cursor position (for left/right arrow)
//  GXBYTE    fWordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
//  GXBYTE    fInvalid        :1;     // Invalid character sequence
//  GXBYTE    fReserved       :3;
//} GXSCRIPT_LOGATTR;
//
//
///////   SCRIPT_ANALYSIS
////
////      Each analysed item is described by a SCRIPT_ANALYSIS structure.
////      It also includes a copy of the Unicode algorithm state (SCRIPT_STATE).
////
////
//typedef struct tag_GXSCRIPT_ANALYSIS {
//  GXWORD    eScript         :10;    // Shaping engine
//  GXWORD    fRTL            :1;     // Rendering direction
//  GXWORD    fLayoutRTL      :1;     // Set for GCP classes ARABIC/HEBREW and LOCALNUMBER
//  GXWORD    fLinkBefore     :1;     // Implies there was a ZWJ before this item
//  GXWORD    fLinkAfter      :1;     // Implies there is a ZWJ following this item.
//  GXWORD    fLogicalOrder   :1;     // Set by client as input to ScriptShape/Place
//  GXWORD    fNoGlyphIndex   :1;     // Generated by ScriptShape/Place - this item does not use glyph indices
//  GXSCRIPT_STATE s;
//} GXSCRIPT_ANALYSIS;



//WINUSERAPI GXUINT_PTR    GXDLLAPI SetSystemTimer(HWND,GXUINT_PTR,GXUINT,TIMERPROC);
//WINUSERAPI GXBOOL        GXDLLAPI KillSystemTimer(HWND,GXUINT_PTR);
#define SetSystemTimer gxSetTimer
#define KillSystemTimer gxKillTimer
#define gxGetWindowText gxGetWindowTextW
//#define gxGetWindowTextA gxGetWindowText
//#define  
//#define gxCreateWindowA gxCreateWindow
#if defined(_WIN32) || defined(_WINDOWS)
#define snprintf _snprintf
#endif
//#define wine_dbg_sprintf /*sprintf_s*/

#if defined(_WIN32) || defined(_WINDOWS) 
#define WEAssert ASSERT
#endif // defined(_WIN32) || defined(_WINDOWS) 

#define SYSCOLOR_GetPen(x)  (GXHGDIOBJ)gxGetSysColorBrush(x)

#define wine_dbgstr_rect(x)  "win_rect"
#define wine_dbgstr_point(x) "win_point"

#define WIN_GetFullHandle(h)  h
#define WIN_GetPtr(h)      GXWND_PTR(h)

__inline void *gxULongToHandle(
  const unsigned long h
  )
{
  return((void *) (GXUINT_PTR) h );
}

#define GET_WORD(ptr)  (*(const GXWORD *)(ptr))
#define GET_DWORD(ptr) (*(const GXDWORD *)(ptr))
#define toupperW(_Char)    ( (_Char)-'a'+'A' )
#define USER_HEAP_ALLOC(n) ((GXLPVOID)new GXBYTE[n])  //gxHeapAlloc(gxGetProcessHeap(),0,n)
#define USER_HEAP_FREE(p) (delete[] p)  //HeapFree(gxGetProcessHeap(),0,p)
#define USER_HEAP_LIN_ADDR(x) x

#define ERR_accel(...)
#define WARN_accel(...)
#define TRACE_accel(...)

#define ERR_(x)   ERR_##x
#define WARN_(x)  WARN_##x
#define TRACE_(x) TRACE_##x

#define WIN_ReleasePtr(_X)  {ASSERT(FALSE);}
#define GXWND_OTHER_PROCESS ((GXWnd*)1)  /* returned by WIN_GetPtr on unknown window handles */
#define GXWND_DESKTOP       ((GXWnd*)2)  /* returned by WIN_GetPtr on the desktop window */

#define GXOBM_CLOSE           32754  // OBM_CLOSE           
#define GXOBM_UPARROW         32753  // OBM_UPARROW         
#define GXOBM_DNARROW         32752  // OBM_DNARROW         
#define GXOBM_RGARROW         32751  // OBM_RGARROW         
#define GXOBM_LFARROW         32750  // OBM_LFARROW         
#define GXOBM_REDUCE          32749  // OBM_REDUCE          
#define GXOBM_ZOOM            32748  // OBM_ZOOM            
#define GXOBM_RESTORE         32747  // OBM_RESTORE         
#define GXOBM_REDUCED         32746  // OBM_REDUCED         
#define GXOBM_ZOOMD           32745  // OBM_ZOOMD           
#define GXOBM_RESTORED        32744  // OBM_RESTORED        
#define GXOBM_UPARROWD        32743  // OBM_UPARROWD        
#define GXOBM_DNARROWD        32742  // OBM_DNARROWD        
#define GXOBM_RGARROWD        32741  // OBM_RGARROWD        
#define GXOBM_LFARROWD        32740  // OBM_LFARROWD        
#define GXOBM_MNARROW         32739  // OBM_MNARROW         
#define GXOBM_COMBO           32738  // OBM_COMBO           
#define GXOBM_UPARROWI        32737  // OBM_UPARROWI        
#define GXOBM_DNARROWI        32736  // OBM_DNARROWI        
#define GXOBM_RGARROWI        32735  // OBM_RGARROWI        
#define GXOBM_LFARROWI        32734  // OBM_LFARROWI        

#define GXOBM_OLD_CLOSE       32767  // OBM_OLD_CLOSE       
#define GXOBM_SIZE            32766  // OBM_SIZE            
#define GXOBM_OLD_UPARROW     32765  // OBM_OLD_UPARROW     
#define GXOBM_OLD_DNARROW     32764  // OBM_OLD_DNARROW     
#define GXOBM_OLD_RGARROW     32763  // OBM_OLD_RGARROW     
#define GXOBM_OLD_LFARROW     32762  // OBM_OLD_LFARROW     
#define GXOBM_BTSIZE          32761  // OBM_BTSIZE          
#define GXOBM_CHECK           32760  // OBM_CHECK           
#define GXOBM_CHECKBOXES      32759  // OBM_CHECKBOXES      
#define GXOBM_BTNCORNERS      32758  // OBM_BTNCORNERS      
#define GXOBM_OLD_REDUCE      32757  // OBM_OLD_REDUCE      
#define GXOBM_OLD_ZOOM        32756  // OBM_OLD_ZOOM        
#define GXOBM_OLD_RESTORE     32755  // OBM_OLD_RESTORE     

GXLRESULT DEFWNDPROC_NcHitTest(GXHWND hWnd, GXINT xPos, GXINT yPos);
#define NC_DrawSysButton  TRACE("=== NC_DrawSysButton ===\n");
#define NC_GetSysPopupPos  TRACE("=== NC_GetSysPopupPos ===\n");
#define NC_HandleNCHitTest(_HWND, _PT)  DEFWNDPROC_NcHitTest(_HWND, _PT.x, _PT.y)//HTCLIENT; TRACE("=== NC_HandleNCHitTest ===\n");

#define HACCEL_16(x)  x
#define HBITMAP_32(x)  ((GXHBITMAP)x)
#define GXMAKERESOURCEW(x) L###x

#if defined(_WINDOWS) || defined(_WIN32)
extern HINSTANCE  g_hDLLModule;
#endif
#define user32_module GXGetInstance(GXINSTTYPE_GRAPX)

//extern GXWNDCLASSEX WndClassEx_MyEdit;


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef IDOK
#define IDOK                1
#endif

#ifndef IDCANCEL
#define IDCANCEL            2
#endif


#endif //_WINE_COMMON_DFEINE_FILE_