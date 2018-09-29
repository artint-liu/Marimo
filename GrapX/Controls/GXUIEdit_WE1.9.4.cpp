#if 0
/*
*  Edit control
*
*  Copyright  David W. Metcalfe, 1994
*  Copyright  William Magro, 1995, 1996
*  Copyright  Frans van Dorsselaer, 1996, 1997
*
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
* of Comctl32.dll version 6.0 on Oct. 8, 2004, by Dimitrie O. Paun.
* 
* Unless otherwise noted, we believe this code to be complete, as per
* the specification mentioned above.
* If you discover missing features, or bugs, please note them below.
*
* TODO:
*   - EDITBALLOONTIP structure
*   - EM_GETCUEBANNER/Edit_GetCueBannerText
*   - EM_HIDEBALLOONTIP/Edit_HideBalloonTip
*   - EM_SETCUEBANNER/Edit_SetCueBannerText
*   - EM_SHOWBALLOONTIP/Edit_ShowBalloonTip
*   - EM_GETIMESTATUS, EM_SETIMESTATUS
*   - EN_ALIGN_LTR_EC
*   - EN_ALIGN_RTL_EC
*   - ES_OEMCONVERT
*
*/

// Artint: 这个修改自1.9.4版Wine项目的Edit控件代码
// 修改原则是(1)尽量不对原有的逻辑进行修改; (2)C代码C++类化; (3)内存创建改为new/delete;
// (4)声明和函数增加'GX'/'gx'前缀; (5)代码格式调整，主要是增删空格，制表符替换为空格，符合自己的代码规范

//#include "config.h"
//
//#include <stdarg.h>
//#include <string.h>
//#include <stdlib.h>
//
//#include "windef.h"
//#include "winbase.h"
//#include "winnt.h"
//#include "win.h"
//#include "imm.h"
//#include "usp10.h"
//#include "wine/unicode.h"
//#include "controls.h"
//#include "user_private.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(edit);
//WINE_DECLARE_DEBUG_CHANNEL(combo);
//WINE_DECLARE_DEBUG_CHANNEL(relay);

#include <GrapX.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GXImm.h"
#include "GrapX/GResource.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/GXCanvas.h"
#include "GXUICtrlBase.h"
#include "User/gxusp10.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.h>

#define BUFLIMIT_INITIAL    30000   /* initial buffer size */
#define GROWLENGTH    32  /* buffers granularity in bytes: must be power of 2 */
#define ROUND_TO_GROW(size)  (((size) + (GROWLENGTH - 1)) & ~(GROWLENGTH - 1))
#define HSCROLL_FRACTION  3  /* scroll window by 1/3 width */

#define TRACE_relay TRACEW
#define TRACE_combo TRACEW
//
//  extra flags for EDITSTATE.flags field
//
#define EF_MODIFIED       0x0001  // text has been modified
#define EF_FOCUSED        0x0002  // we have input focus
#define EF_UPDATE         0x0004  // notify parent of changed state
#define EF_VSCROLL_TRACK  0x0008  // don't SetScrollPos() since we are tracking the thumb
#define EF_HSCROLL_TRACK  0x0010  // don't SetScrollPos() since we are tracking the thumb
#define EF_AFTER_WRAP     0x0080  // the caret is displayed after the last character of a wrapped line, instead of in front of the next character
#define EF_USE_SOFTBRK    0x0100  // Enable soft breaks in text.
#define EF_DIALOGMODE     0x0200  // Indicates that we are inside a dialog window

enum LINE_END
{
  END_0 = 0,      // line ends with terminating '\0' character
  END_WRAP,       // line is wrapped
  END_HARD,       // line ends with a hard return '\r\n'
  END_SOFT,       // line ends with a soft return '\r\r\n'
  END_RICH        // line ends with a single '\n'
};

#define SCRIPT_UNDEFINED  0
typedef void* SCRIPT_STRING_ANALYSIS;
//struct GXSCRIPT_LOGATTR {
//  GXBYTE    fSoftBreak      :1;     // Potential linebreak point
//  GXBYTE    fWhiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
//  GXBYTE    fCharStop       :1;     // Valid cursor position (for left/right arrow)
//  GXBYTE    fWordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
//  GXBYTE    fInvalid        :1;     // Invalid character sequence
//  GXBYTE    fReserved       :3;
//};
//
//struct GXSCRIPT_STATE {
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
//};
//
//struct GXSCRIPT_ANALYSIS {
//  GXWORD    eScript         :10;    // Shaping engine
//  GXWORD    fRTL            :1;     // Rendering direction
//  GXWORD    fLayoutRTL      :1;     // Set for GCP classes ARABIC/HEBREW and LOCALNUMBER
//  GXWORD    fLinkBefore     :1;     // Implies there was a ZWJ before this item
//  GXWORD    fLinkAfter      :1;     // Implies there is a ZWJ following this item.
//  GXWORD    fLogicalOrder   :1;     // Set by client as input to ScriptShape/Place
//  GXWORD    fNoGlyphIndex   :1;     // Generated by ScriptShape/Place - this item does not use glyph indices
//  GXSCRIPT_STATE s;
//};
//

struct LINEDEF {
  INT length;                 // bruto length of a line in bytes
  INT net_length;             // netto length of a line in visible characters
  LINE_END ending;
  INT width;                  // width of the line in pixels
  INT index;                  // line index into the buffer
  SCRIPT_STRING_ANALYSIS ssa; // Uniscribe Data
  LINEDEF* next;

  inline void InvalidateUniscribeData_linedef();
};

struct EDITSTATE
{
  GXBOOL    m_is_unicode;           // how the control was created
  GXLPWSTR  m_text;                 // the actual contents of the control
  GXUINT    m_text_length;          // cached length of text buffer (in WCHARs) - use get_text_length() to retrieve
  GXUINT    m_buffer_size;          // the size of the buffer in characters
  GXUINT    m_buffer_limit;         // the maximum size to which the buffer may grow in characters
  GXHFONT   m_font;                 // NULL means standard system font
  GXINT     m_x_offset;             // scroll offset  for multi lines this is in pixels for single lines it's in characters
  GXINT     m_line_height;          // height of a screen line in pixels
  GXINT     m_char_width;           // average character width in pixels
  GXDWORD   m_style;                // sane version of wnd->dwStyle
  GXWORD    m_flags;                // flags that are not in m_style or wnd->flags (EF_XXX)
  GXINT     m_undo_insert_count;    // number of characters inserted in sequence
  GXUINT    m_undo_position;        // character index of the insertion and deletion
  GXLPWSTR  m_undo_text;            // deleted text
  GXUINT    m_undo_buffer_size;     // size of the deleted text buffer
  GXINT     m_selection_start;      // == selection_end if no selection
  GXINT     m_selection_end;        // == current caret position
  GXWCHAR   m_password_char;        // == 0 if no password char, and for multi line controls
  GXINT     m_left_margin;          // in pixels
  GXINT     m_right_margin;         // in pixels
  GXRECT    m_format_rect;
  GXINT     m_text_width;           // width of the widest line in pixels for multi line controls and just line width for single line controls 
  GXINT     m_region_posx;          // Position of cursor relative to region:
  GXINT     m_region_posy;          // -1: to left, 0: within, 1: to right
  void*     m_word_break_proc;      // 32-bit word break proc: ANSI or Unicode
  GXINT     m_line_count;           // number of lines
  GXINT     m_y_offset;             // scroll offset in number of lines
  GXBOOL    m_bCaptureState;        // flag indicating whether mouse was captured
  GXBOOL    m_bEnableState;         // flag keeping the enable state
  GXHWND    m_hwndSelf;             // the our window handle
  GXHWND    m_hwndParent;           // Handle of parent for sending EN_* messages. Even if parent will change, EN_* messages should be sent to the first parent.
  GXHWND    m_hwndListBox;          // handle of ComboBox's listbox or NULL
  GXINT     m_wheelDeltaRemainder;  // scroll wheel delta left over after scrolling whole lines
  //
  //  only for multi line controls
  //
  GXINT     m_lock_count;           // amount of re-entries in the EditWndProc
  GXINT     m_tabs_count;
  GXLPINT   m_tabs;
  LINEDEF*  m_first_line_def;       // linked list of (soft) linebreaks
  GXHLOCAL  m_hloc32W;              // our unicode local memory block
  GXHLOCAL  m_hloc32A;              // alias for ANSI control receiving EM_GETHANDLE or EM_SETHANDLE
  GXHLOCAL    m_hlocapp;              // The text buffer handle belongs to the app
  //
  // IME Data
  //
  GXUINT    m_composition_len;      // length of composition, 0 == no composition
  int       m_composition_start;    // the character position for the composition
  //
  // Uniscribe Data
  //
  GXSCRIPT_LOGATTR* m_logAttr;
  SCRIPT_STRING_ANALYSIS m_ssa; // Uniscribe Data for single line controls

  inline void InvalidateUniscribeData();
  inline GXUINT get_text_length();
  inline void text_buffer_changed();
  inline INT get_vertical_line_count();
  inline GXBOOL EM_CanUndo();
  inline void EM_EmptyUndoBuffer();
  //GXDWORD get_app_version(void);
  GXHBRUSH NotifyCtlColor(GXHDC hdc);
  void CalcLineWidth_SL();
  INT CharFromPos(INT x, INT y, LPBOOL after_wrap);
  void ConfinePoint(LPINT x, LPINT y) const;
  INT EM_LineFromChar(INT index);
  INT EM_LineIndex(INT line) const;
  INT EM_LineLength(INT index);
  GXLRESULT EM_PosFromChar(INT index, GXBOOL after_wrap);
  void GetLineRect(INT line, INT scol, INT ecol, GXLPRECT rc);
  void LockBuffer();
  void UnlockBuffer(GXBOOL force);
  GXBOOL MakeFit(GXUINT size);
  GXBOOL MakeUndoFit(GXUINT size);
  void UpdateTextRegion(GXHRGN hrgn, GXBOOL bErase);
  void UpdateText(const GXRECT *rc, GXBOOL bErase);
  void SL_InvalidateText(INT start, INT end);
  void ML_InvalidateText(INT start, INT end);
  void InvalidateText(INT start, INT end);
  void EM_SetSel(GXUINT start, GXUINT end, GXBOOL after_wrap);
  void UpdateScrollInfo();
  GXBOOL EM_LineScroll_internal(INT dx, INT dy);
  GXBOOL EM_LineScroll(INT dx, INT dy);
  GXLRESULT EM_Scroll(INT action);
  void SetCaretPos(INT pos, GXBOOL after_wrap);
  void EM_ScrollCaret();
  void MoveBackward(GXBOOL extend);
  void MoveDown_ML(GXBOOL extend);
  void MoveEnd(GXBOOL extend, GXBOOL ctrl);
  void MoveForward(GXBOOL extend);
  void MoveHome(GXBOOL extend, GXBOOL ctrl);
  void MovePageDown_ML(GXBOOL extend);
  void MovePageUp_ML(GXBOOL extend);
  void MoveUp_ML(GXBOOL extend);
  void MoveWordBackward(GXBOOL extend);
  void MoveWordForward(GXBOOL extend);
  INT PaintText(GXHDC dc, INT x, INT y, INT line, INT col, INT count, GXBOOL rev);
  void PaintLine(GXHDC dc, INT line, GXBOOL rev);
  void AdjustFormatRect();
  void SetRectNP(const GXRECT *rc);
  GXLRESULT EM_CharFromPos(INT x, INT y);
  GXBOOL EM_FmtLines(GXBOOL add_eol);
  GXHLOCAL EM_GetHandle();
  INT EM_GetLine(INT line, LPWSTR dst, GXBOOL unicode);
  GXLRESULT EM_GetSel(PUINT start, PUINT end) const;
  void EM_ReplaceSel(GXBOOL can_undo, LPCWSTR lpsz_replace, GXBOOL send_update, GXBOOL honor_limit);
  void EM_SetHandle(GXHLOCAL hloc);
  void EM_SetLimitText(GXUINT limit);
  //int calc_min_set_margin_size(GXHDC dc, INT left, INT right);
  void EM_SetMargins(INT action, WORD left, WORD right, GXBOOL repaint);
  void EM_SetPasswordChar(WCHAR c);
  GXBOOL EM_SetTabStops(INT count, const INT *tabs);
  GXBOOL EM_Undo();
  inline GXBOOL IsInsideDialog();
  void WM_Paste();
  void WM_Copy();
  inline void WM_Clear();
  inline void WM_Cut();
  GXLRESULT WM_Char(WCHAR c);
  void ContextMenuCommand(GXUINT id);
  void WM_ContextMenu(INT x, INT y);
  INT WM_GetText(INT count, LPWSTR dst, GXBOOL unicode) const;
  GXBOOL CheckCombo(GXUINT msg, INT key);
  GXLRESULT WM_KeyDown(INT key);
  GXLRESULT WM_KillFocus();
  GXLRESULT WM_LButtonDblClk();
  GXLRESULT WM_LButtonDown(GXDWORD keys, INT x, INT y);
  GXLRESULT WM_LButtonUp();
  GXLRESULT WM_MButtonDown();
  GXLRESULT WM_MouseMove(INT x, INT y);
  void WM_Paint(GXHDC hdc);
  void WM_SetFocus();
  void WM_SetFont(GXHFONT font, GXBOOL redraw);
  void WM_Size(GXUINT action);
  GXLRESULT  WM_StyleChanged(GXWPARAM which, const STYLESTRUCT *style);
  GXLRESULT WM_SysKeyDown(INT key, GXDWORD key_data);
  void WM_Timer();
  GXLRESULT WM_HScroll(INT action, INT pos);
  GXLRESULT WM_VScroll(INT action, INT pos);
  GXLRESULT EM_GetThumb();
  void GetCompositionStr(GXHIMC hIMC, GXLPARAM CompFlag);
  void GetResultStr(GXHIMC hIMC);
  void ImeComposition(GXHWND hwnd, GXLPARAM CompFlag);
  GXLRESULT WM_NCCreate(GXHWND hwnd, GXLPCREATESTRUCTW lpcs, GXBOOL unicode);
  GXLRESULT WM_Create(LPCWSTR name);
  GXLRESULT WM_NCDestroy();

  GXINT WordBreakProc(GXLPWSTR s, GXINT index, GXINT count, GXINT action);
  GXINT CallWordBreakProc(GXINT start, GXINT index, GXINT count, GXINT action);
  void EM_SetWordBreakProc(void *wbp);
  void BuildLineDefs_ML(INT istart, INT iend, INT delta, GXHRGN hrgn);
  void WM_SetText(LPCWSTR text, GXBOOL unicode);
  SCRIPT_STRING_ANALYSIS UpdateUniscribeData_linedef(GXHDC dc, LINEDEF *line_def);
  SCRIPT_STRING_ANALYSIS UpdateUniscribeData(GXHDC dc, INT line);

  EDITSTATE();
};


#define SWAP_UINT32(x,y) do { GXUINT temp = (GXUINT)(x); (x) = (GXUINT)(y); (y) = temp; } while(0)
#define ORDER_UINT(x,y) do { if ((GXUINT)(y) < (GXUINT)(x)) SWAP_UINT32((x),(y)); } while(0)

/* used for disabled or read-only edit control */
#define _NOTIFY_PARENT(wNotifyCode) \
  do \
{ /* Notify parent which has created this edit control */ \
  TRACE("notification " #wNotifyCode " sent to hwnd=%p\n", m_hwndParent); \
  gxSendMessageW(m_hwndParent, GXWM_COMMAND, \
  GXMAKEWPARAM(gxGetWindowLongPtrW((m_hwndSelf),GWLP_ID), wNotifyCode), \
  (GXLPARAM)(m_hwndSelf)); \
} while(0)

static const WCHAR empty_stringW[] = {0};
//static GXLRESULT EM_PosFromChar(EDITSTATE *INT index, GXBOOL after_wrap);

EDITSTATE::EDITSTATE() :
m_is_unicode            (0),
m_text                  (0),
m_text_length           (0),
m_buffer_size           (0),
m_buffer_limit          (0),
m_font                  (0),
m_x_offset              (0),
m_line_height           (0),
m_char_width            (0),
m_style                 (0),
m_flags                 (0),
m_undo_insert_count     (0),
m_undo_position         (0),
m_undo_text             (NULL),
m_undo_buffer_size      (0),
m_selection_start       (0),
m_selection_end         (0),
m_password_char         (0),
m_left_margin           (0),
m_right_margin          (0),
m_text_width            (0),
m_region_posx           (0),
m_region_posy           (0),
m_word_break_proc       (NULL),
m_line_count            (0),
m_y_offset              (0),
m_bCaptureState         (0),
m_bEnableState          (0),
m_hwndSelf              (0),
m_hwndParent            (0),
m_hwndListBox           (0),
m_wheelDeltaRemainder   (0),
m_lock_count            (0),
m_tabs_count            (0),
m_tabs                  (0),
m_first_line_def        (NULL),
m_hloc32W               (0),
m_hloc32A               (0),
m_hlocapp               (0),
m_composition_len       (0),
m_composition_start     (0),
m_logAttr               (0),
m_ssa                   (0)
{
  gxSetRectEmpty(&m_format_rect);
}

/*********************************************************************
*
*  EM_CANUNDO
*
*/
/*inline */GXBOOL EDITSTATE::EM_CanUndo()
{
  return (m_undo_insert_count || GXSTRLEN(m_undo_text));
}


/*********************************************************************
*
*  EM_EMPTYUNDOBUFFER
*
*/
void EDITSTATE::EM_EmptyUndoBuffer()
{
  m_undo_insert_count = 0;
  *m_undo_text = '\0';
}


/**********************************************************************
*         get_app_version
*
* Returns the window version in case Wine emulates a later version
* of windows than the application expects.
*
* In a number of cases when windows runs an application that was
* designed for an earlier windows version, windows reverts
* to "old" behaviour of that earlier version.
*
* An example is a disabled  edit control that needs to be painted.
* Old style behaviour is to send a WM_CTLCOLOREDIT message. This was
* changed in Win95, NT4.0 by a WM_CTLCOLORSTATIC message _only_ for
* applications with an expected version 0f 4.0 or higher.
*
*/
GXDWORD get_app_version(void)
{
  static GXDWORD version;
  if (!version)
  {
    GXDWORD dwEmulatedVersion;
    OSVERSIONINFOW info;
    GXDWORD dwProcVersion = GetProcessVersion(0);

    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    GetVersionExW( &info );
    dwEmulatedVersion = MAKELONG( info.dwMinorVersion, info.dwMajorVersion );
    /* FIXME: this may not be 100% correct; see discussion on the
    * wine developer list in Nov 1999 */
    version = dwProcVersion < dwEmulatedVersion ? dwProcVersion : dwEmulatedVersion;
  }
  return version;
}

GXHBRUSH EDITSTATE::NotifyCtlColor(GXHDC hdc)
{
  GXHBRUSH hbrush;
  GXUINT msg;

  if ( get_app_version() >= 0x40000 && (!m_bEnableState || (m_style & ES_READONLY)))
    msg = WM_CTLCOLORSTATIC;
  else
    msg = WM_CTLCOLOREDIT;

  /* why do we notify to m_hwndParent, and we send this one to gxGetParent()? */
  hbrush = (GXHBRUSH)gxSendMessageW(gxGetParent(m_hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)m_hwndSelf);
  if (!hbrush)
    hbrush = (GXHBRUSH)gxDefWindowProcW(gxGetParent(m_hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)m_hwndSelf);
  return hbrush;
}


GXUINT EDITSTATE::get_text_length()
{
  if(m_text_length == (GXUINT)-1)
    m_text_length = GXSTRLEN(m_text);
  return m_text_length;
}


/*********************************************************************
*
*  _WordBreakProc
*
*  Find the beginning of words.
*  Note:  unlike the specs for a WordBreakProc, this function can
*    only be called without linebreaks between s[0] up to
*    s[count - 1].  Remember it is only called
*    internally, so we can decide this for ourselves.
*    Additionally we will always be breaking the full string.
*
*/
GXINT EDITSTATE::WordBreakProc(GXLPWSTR s, GXINT index, GXINT count, GXINT action)
{
  GXINT ret = 0;

  TRACE("s=%p, index=%d, count=%d, action=%d\n", s, index, count, action);

  if(!s) return 0;

  if ( ! m_logAttr)
  {
    GXSCRIPT_ANALYSIS psa;

    memset(&psa, 0, sizeof(GXSCRIPT_ANALYSIS));
    psa.eScript = SCRIPT_UNDEFINED;

    m_logAttr = new GXSCRIPT_LOGATTR[get_text_length()]; // HeapAlloc(GetProcessHeap(), 0, sizeof(GXSCRIPT_LOGATTR) * get_text_length());
    gxScriptBreak(m_text, get_text_length(), &psa, m_logAttr);
  }

  switch (action) {
  case WB_LEFT:
    if (index)
      index--;
    while (index && ! m_logAttr[index].fSoftBreak)
      index--;
    ret = index;
    break;
  case WB_RIGHT:
    if (!count)
      break;
    while (s[index] && index < count && !m_logAttr[index].fSoftBreak)
      index++;
    ret = index;
    break;
  case WB_ISDELIMITER:
    ret = m_logAttr[index].fWhiteSpace;
    break;
  default:
    ERR("unknown action code, please report !\n");
    break;
  }
  return ret;
}


/*********************************************************************
*
*  _CallWordBreakProc
*
*  Call appropriate WordBreakProc (internal or external).
*
*  Note: The "start" argument should always be an index referring
*    to m_text.  The actual wordbreak proc might be
*    16 bit, so we can't always pass any 32 bit LPSTR.
*    Hence we assume that m_text is the buffer that holds
*    the string under examination (we can decide this for ourselves).
*
*/
GXINT EDITSTATE::CallWordBreakProc(GXINT start, GXINT index, GXINT count, GXINT action)
{
  GXINT ret;

  if (m_word_break_proc)
  {
    if(m_is_unicode)
    {
      EDITWORDBREAKPROCW wbpW = (EDITWORDBREAKPROCW)m_word_break_proc;

      TRACE_(relay)(L"(UNICODE wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n",
        m_word_break_proc, debugstr_wn(m_text + start, count), index, count, action);
      ret = wbpW(m_text + start, index, count, action);
    }
    else
    {
      EDITWORDBREAKPROCA wbpA = (EDITWORDBREAKPROCA)m_word_break_proc;
      INT countA;
      GXCHAR* textA;

      countA = WideCharToMultiByte(CP_ACP, 0, m_text + start, count, NULL, 0, NULL, NULL);
      textA = new CHAR[countA]; // HeapAlloc(GetProcessHeap(), 0, countA);
      gxWideCharToMultiByte(CP_ACP, 0, m_text + start, count, textA, countA, NULL, NULL);
      TRACE_(relay)(L"(ANSI wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n",
        m_word_break_proc, debugstr_an(textA, countA), index, countA, action);
      ret = wbpA(textA, index, countA, action);
      SAFE_DELETE_ARRAY(textA);
      //HeapFree(GetProcessHeap(), 0, textA);
    }
  }
  else
    ret = WordBreakProc(m_text, index+start, count+start, action) - start;

  return ret;
}

void LINEDEF::InvalidateUniscribeData_linedef()
{
  if (ssa)
  {
    gxScriptStringFree(&ssa);
    ssa = NULL;
  }
}

void EDITSTATE::InvalidateUniscribeData()
{
  LINEDEF *line_def = m_first_line_def;
  while (line_def)
  {
    line_def->InvalidateUniscribeData_linedef();
    line_def = line_def->next;
  }
  if (m_ssa)
  {
    gxScriptStringFree(&m_ssa);
    m_ssa = NULL;
  }
}

SCRIPT_STRING_ANALYSIS EDITSTATE::UpdateUniscribeData_linedef(GXHDC dc, LINEDEF *line_def)
{
  if (!line_def)
    return NULL;

  if (line_def->net_length && !line_def->ssa)
  {
    int index = line_def->index;
    GXHFONT old_font = NULL;
    GXHDC udc = dc;
    GXSCRIPT_TABDEF tabdef;
    HRESULT hr;

    if (!udc)
      udc = gxGetDC(m_hwndSelf);
    if (m_font)
      old_font = (GXHFONT)gxSelectObject(udc, m_font);

    tabdef.cTabStops = m_tabs_count;
    tabdef.iScale = 0;
    tabdef.pTabStops = m_tabs;
    tabdef.iTabOrigin = 0;

    hr = gxScriptStringAnalyse(udc, &m_text[index], line_def->net_length,
      (int)(1.5f * line_def->net_length + 16), -1, GXSSA_LINK | GXSSA_FALLBACK | GXSSA_GLYPHS | GXSSA_TAB, -1,
      NULL, NULL, NULL, &tabdef, NULL, &line_def->ssa);
    if (FAILED(hr))
    {
      WARN("ScriptStringAnalyse failed (%x)\n",hr);
      line_def->ssa = NULL;
    }

    if (m_font)
      gxSelectObject(udc, old_font);
    if (udc != dc)
      gxReleaseDC(m_hwndSelf, udc);
  }

  return line_def->ssa;
}

SCRIPT_STRING_ANALYSIS EDITSTATE::UpdateUniscribeData(GXHDC dc, INT line)
{
  LINEDEF *line_def;

  if ( ! (m_style & ES_MULTILINE))
  {
    if ( ! m_ssa)
    {
      INT length = get_text_length();
      GXHFONT old_font = NULL;
      GXHDC udc = dc;

      if (!udc)
        udc = gxGetDC(m_hwndSelf);
      if (m_font)
        old_font = (GXHFONT)gxSelectObject(udc, m_font);

      if (m_style & ES_PASSWORD)
        gxScriptStringAnalyse(udc, &m_password_char, length, (int)(1.5*length+16), -1, GXSSA_LINK|GXSSA_FALLBACK|GXSSA_GLYPHS|GXSSA_PASSWORD, -1, NULL, NULL, NULL, NULL, NULL, &m_ssa);
      else
        gxScriptStringAnalyse(udc, m_text, length, (int)(1.5*length+16), -1, GXSSA_LINK|GXSSA_FALLBACK|GXSSA_GLYPHS, -1, NULL, NULL, NULL, NULL, NULL, &m_ssa);

      if (m_font)
        gxSelectObject(udc, old_font);
      if (udc != dc)
        gxReleaseDC(m_hwndSelf, udc);
    }
    return m_ssa;
  }
  else
  {
    line_def = m_first_line_def;
    while (line_def && line)
    {
      line_def = line_def->next;
      line--;
    }

    return UpdateUniscribeData_linedef(dc,line_def);
  }
}

INT EDITSTATE::get_vertical_line_count()
{
  INT vlc = (m_format_rect.bottom - m_format_rect.top) / m_line_height;
  return max(1,vlc);
}

/*********************************************************************
*
*  _BuildLineDefs_ML
*
*  Build linked list of text lines.
*  Lines can end with '\0' (last line), a character (if it is wrapped),
*  a soft return '\r\r\n' or a hard return '\r\n'
*
*/
void EDITSTATE::BuildLineDefs_ML(INT istart, INT iend, INT delta, GXHRGN hrgn)
{
  LPWSTR current_position, cp;
  INT fw;
  LINEDEF *current_line;
  LINEDEF *previous_line;
  LINEDEF *start_line;
  INT line_index = 0, nstart_line, nstart_index;
  INT line_count = m_line_count;
  INT orig_net_length;
  GXRECT rc;
  INT vlc;

  if (istart == iend && delta == 0)
    return;

  previous_line = NULL;
  current_line = m_first_line_def;

  /* Find starting line. istart must lie inside an existing line or
  * at the end of buffer */
  do {
    if (istart < current_line->index + current_line->length ||
      current_line->ending == END_0)
      break;

    previous_line = current_line;
    current_line = current_line->next;
    line_index++;
  } while (current_line);

  if (!current_line) /* Error occurred start is not inside previous buffer */
  {
    FIXME(" modification occurred outside buffer\n");
    return;
  }

  /* Remember start of modifications in order to calculate update region */
  nstart_line = line_index;
  nstart_index = current_line->index;

  /* We must start to reformat from the previous line since the modifications
  * may have caused the line to wrap upwards. */
  if (!(m_style & ES_AUTOHSCROLL) && line_index > 0)
  {
    line_index--;
    current_line = previous_line;
  }
  start_line = current_line;

  fw = m_format_rect.right - m_format_rect.left;
  current_position = m_text + current_line->index;
  vlc = get_vertical_line_count();
  do {
    if (current_line != start_line)
    {
      if (!current_line || current_line->index + delta > current_position - m_text)
      {
        /* The buffer has been expanded, create a new line and
        insert it into the link list */
        LINEDEF* new_line = new LINEDEF;// HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LINEDEF));
        memset(new_line, 0, sizeof(LINEDEF));

        new_line->next = previous_line->next;
        previous_line->next = new_line;
        current_line = new_line;
        m_line_count++;
      }
      else if (current_line->index + delta < current_position - m_text)
      {
        /* The previous line merged with this line so we delete this extra entry */
        previous_line->next = current_line->next;
        //HeapFree(GetProcessHeap(), 0, current_line);
        SAFE_DELETE(current_line);
        current_line = previous_line->next;
        m_line_count--;
        continue;
      }
      else /* current_line->index + delta == current_position */
      {
        if (current_position - m_text > iend)
          break; /* We reached end of line modifications */
        /* else recalculate this line */
      }
    }

    current_line->index = current_position - m_text;
    orig_net_length = current_line->net_length;

    /* Find end of line */
    cp = current_position;
    while (*cp) {
      if (*cp == '\n') break;
      if ((*cp == '\r') && (*(cp + 1) == '\n'))
        break;
      cp++;
    }

    /* Mark type of line termination */
    if (!(*cp)) {
      current_line->ending = END_0;
      current_line->net_length = GXSTRLEN(current_position);
    } else if ((cp > current_position) && (*(cp - 1) == '\r')) {
      current_line->ending = END_SOFT;
      current_line->net_length = cp - current_position - 1;
    } else if (*cp == '\n') {
      current_line->ending = END_RICH;
      current_line->net_length = cp - current_position;
    } else {
      current_line->ending = END_HARD;
      current_line->net_length = cp - current_position;
    }

    if (current_line->net_length)
    {
      const GXSIZE* sz = NULL;
      current_line->InvalidateUniscribeData_linedef();
      UpdateUniscribeData_linedef(NULL, current_line);
      if (current_line->ssa)
      {
        sz = gxScriptString_pSize(current_line->ssa);
        /* Calculate line width */
        current_line->width = sz->cx;
      }
      else current_line->width = m_char_width * current_line->net_length;
    }
    else current_line->width = 0;

    /* FIXME: check here for lines that are too wide even in AUTOHSCROLL (> 32767 ???) */

    /* Line breaks just look back from the end and find the next break and try that. */

    if (!(m_style & ES_AUTOHSCROLL)) {
      if (current_line->width > fw && fw > m_char_width) {

        GXINT prev, next;
        int w;
        const GXSIZE* sz;
        float d;

        prev = current_line->net_length - 1;
        w = current_line->net_length;
        d = (float)current_line->width/(float)fw;
        if (d > 1.2f) d -= 0.2f;
        next = (GXINT)(prev / d);
        if (next >= prev) next = prev-1;
        do {
          prev = CallWordBreakProc(current_position - m_text,
            next, current_line->net_length, WB_LEFT);
          current_line->net_length = prev;
          current_line->InvalidateUniscribeData_linedef();
          UpdateUniscribeData_linedef(NULL, current_line);
          if (current_line->ssa)
            sz = gxScriptString_pSize(current_line->ssa);
          else sz = 0;
          if (sz)
            current_line->width = sz->cx;
          else
            prev = 0;
          next = prev - 1;
        } while (prev && current_line->width > fw);
        current_line->net_length = w;

        if (prev == 0) { /* Didn't find a line break so force a break */
          INT *piDx;
          const INT *count;

          current_line->InvalidateUniscribeData_linedef();
          UpdateUniscribeData_linedef(NULL, current_line);

          if (current_line->ssa)
          {
            count = gxScriptString_pcOutChars(current_line->ssa);
            piDx = new INT[*count]; // HeapAlloc(GetProcessHeap(),0,sizeof(INT) * (*count));
            gxScriptStringGetLogicalWidths(current_line->ssa,piDx);

            prev = current_line->net_length-1;
            do {
              current_line->width -= piDx[prev];
              prev--;
            } while ( prev > 0 && current_line->width > fw);
            if (prev<=0)
              prev = 1;
            SAFE_DELETE_ARRAY(piDx);
            //HeapFree(GetProcessHeap(),0,piDx);
          }
          else
            prev = (fw / m_char_width);
        }

        /* If the first line we are calculating, wrapped before istart, we must
        * adjust istart in order for this to be reflected in the update region. */
        if (current_line->index == nstart_index && istart > current_line->index + prev)
          istart = current_line->index + prev;
        /* else if we are updating the previous line before the first line we
        * are re-calculating and it expanded */
        else if (current_line == start_line &&
          current_line->index != nstart_index && orig_net_length < prev)
        {
          /* Line expanded due to an upwards line wrap so we must partially include
          * previous line in update region */
          nstart_line = line_index;
          nstart_index = current_line->index;
          istart = current_line->index + orig_net_length;
        }

        current_line->net_length = prev;
        current_line->ending = END_WRAP;

        if (current_line->net_length > 0)
        {
          UpdateUniscribeData_linedef(NULL, current_line);
          if (current_line->ssa)
          {
            sz = gxScriptString_pSize(current_line->ssa);
            current_line->width = sz->cx;
          }
          else
            current_line->width = 0;
        }
        else current_line->width = 0;
      }
      else if (current_line == start_line &&
        current_line->index != nstart_index &&
        orig_net_length < current_line->net_length) {
          /* The previous line expanded but it's still not as wide as the client rect */
          /* The expansion is due to an upwards line wrap so we must partially include
          it in the update region */
          nstart_line = line_index;
          nstart_index = current_line->index;
          istart = current_line->index + orig_net_length;
      }
    }


    /* Adjust length to include line termination */
    switch (current_line->ending) {
    case END_SOFT:
      current_line->length = current_line->net_length + 3;
      break;
    case END_RICH:
      current_line->length = current_line->net_length + 1;
      break;
    case END_HARD:
      current_line->length = current_line->net_length + 2;
      break;
    case END_WRAP:
    case END_0:
      current_line->length = current_line->net_length;
      break;
    }
    m_text_width = max(m_text_width, current_line->width);
    current_position += current_line->length;
    previous_line = current_line;

    /* Discard data for non-visible lines. It will be calculated as needed */
    if ((line_index < m_y_offset) || (line_index > m_y_offset + vlc))
      current_line->InvalidateUniscribeData_linedef();

    current_line = current_line->next;
    line_index++;
  } while (previous_line->ending != END_0);

  /* Finish adjusting line indexes by delta or remove hanging lines */
  if (previous_line->ending == END_0)
  {
    LINEDEF *pnext = NULL;

    previous_line->next = NULL;
    while (current_line)
    {
      pnext = current_line->next;
      current_line->InvalidateUniscribeData_linedef();
      SAFE_DELETE_ARRAY(current_line);
      //HeapFree(GetProcessHeap(), 0, current_line);
      current_line = pnext;
      m_line_count--;
    }
  }
  else if (delta != 0)
  {
    while (current_line)
    {
      current_line->index += delta;
      current_line = current_line->next;
    }
  }

  /* Calculate rest of modification rectangle */
  if (hrgn)
  {
    GXHRGN tmphrgn;
    /*
    * We calculate two rectangles. One for the first line which may have
    * an indent with respect to the format rect. The other is a format-width
    * rectangle that spans the rest of the lines that changed or moved.
    */
    rc.top = m_format_rect.top + nstart_line * m_line_height -
      (m_y_offset * m_line_height); /* Adjust for vertical scrollbar */
    rc.bottom = rc.top + m_line_height;
    if ((m_style & ES_CENTER) || (m_style & ES_RIGHT))
      rc.left = m_format_rect.left;
    else
      rc.left = GXLOWORD(EM_PosFromChar(nstart_index, FALSE));
    rc.right = m_format_rect.right;
    gxSetRectRgn(hrgn, rc.left, rc.top, rc.right, rc.bottom);

    rc.top = rc.bottom;
    rc.left = m_format_rect.left;
    rc.right = m_format_rect.right;
    /*
    * If lines were added or removed we must re-paint the remainder of the
    * lines since the remaining lines were either shifted up or down.
    */
    if (line_count < m_line_count) /* We added lines */
      rc.bottom = m_line_count * m_line_height;
    else if (line_count > m_line_count) /* We removed lines */
      rc.bottom = line_count * m_line_height;
    else
      rc.bottom = line_index * m_line_height;
    rc.bottom += m_format_rect.top;
    rc.bottom -= (m_y_offset * m_line_height); /* Adjust for vertical scrollbar */
    tmphrgn = gxCreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
    gxCombineRgn(hrgn, hrgn, tmphrgn, RGN_OR);
    gxDeleteObject(tmphrgn);
  }
}

/*********************************************************************
*
*  _CalcLineWidth_SL
*
*/
void EDITSTATE::CalcLineWidth_SL()
{
  UpdateUniscribeData(NULL, 0);
  if (m_ssa)
  {
    const GXSIZE *size;
    size = gxScriptString_pSize(m_ssa);
    m_text_width = size->cx;
  }
  else {
    m_text_width = 0;
  }
}

/*********************************************************************
*
*  _CharFromPos
*
*  Beware: This is not the function called on EM_CHARFROMPOS
*    The position _can_ be outside the formatting / client
*    rectangle
*    The return value is only the character index
*
*/
INT EDITSTATE::CharFromPos(INT x, INT y, LPBOOL after_wrap)
{
  INT index;

  if (m_style & ES_MULTILINE) {
    int trailing;
    INT line = (y - m_format_rect.top) / m_line_height + m_y_offset;
    INT line_index = 0;
    LINEDEF *line_def = m_first_line_def;
    UpdateUniscribeData(NULL, line);
    while ((line > 0) && line_def->next) {
      line_index += line_def->length;
      line_def = line_def->next;
      line--;
    }

    x += m_x_offset - m_format_rect.left;
    if (m_style & ES_RIGHT)
      x -= (m_format_rect.right - m_format_rect.left) - line_def->width;
    else if (m_style & ES_CENTER)
      x -= ((m_format_rect.right - m_format_rect.left) - line_def->width) / 2;
    if (x >= line_def->width) {
      if (after_wrap)
        *after_wrap = (line_def->ending == END_WRAP);
      return line_index + line_def->net_length;
    }
    if (x <= 0 || !line_def->ssa) {
      if (after_wrap)
        *after_wrap = FALSE;
      return line_index;
    }

    gxScriptStringXtoCP(line_def->ssa, x , &index, &trailing);
    if (trailing) index++;
    index += line_index;
    if (after_wrap)
      *after_wrap = ((index == line_index + line_def->net_length) &&
      (line_def->ending == END_WRAP));
  } else {
    INT xoff = 0;
    INT trailing;
    if (after_wrap)
      *after_wrap = FALSE;
    x -= m_format_rect.left;
    if (!x)
      return m_x_offset;

    if (!m_x_offset)
    {
      INT indent = (m_format_rect.right - m_format_rect.left) - m_text_width;
      if (m_style & ES_RIGHT)
        x -= indent;
      else if (m_style & ES_CENTER)
        x -= indent / 2;
    }

    UpdateUniscribeData(NULL, 0);
    if (m_x_offset)
    {
      if (m_ssa)
      {
        if (m_x_offset>= (GXINT)get_text_length())
        {
          const GXSIZE *size;
          size = gxScriptString_pSize(m_ssa);
          xoff = size->cx;
        }
        gxScriptStringCPtoX(m_ssa, m_x_offset, FALSE, &xoff);
      }
      else
        xoff = 0;
    }
    if (x < 0)
    {
      if (x + xoff > 0 || !m_ssa)
      {
        gxScriptStringXtoCP(m_ssa, x+xoff, &index, &trailing);
        if (trailing) index++;
      }
      else
        index = 0;
    }
    else
    {
      if (x)
      {
        const GXSIZE *size = NULL;
        if (m_ssa)
          size = gxScriptString_pSize(m_ssa);
        if (!size)
          index = 0;
        else if (x > size->cx)
          index = get_text_length();
        else if (m_ssa)
        {
          gxScriptStringXtoCP(m_ssa, x+xoff, &index, &trailing);
          if (trailing) index++;
        }
        else
          index = 0;
      }
      else
        index = m_x_offset;
    }
  }
  return index;
}


/*********************************************************************
*
*  _ConfinePoint
*
*  adjusts the point to be within the formatting rectangle
*  (so CharFromPos returns the nearest _visible_ character)
*
*/
void EDITSTATE::ConfinePoint(LPINT x, LPINT y) const
{
  *x = min(max(*x, m_format_rect.left), m_format_rect.right - 1);
  *y = min(max(*y, m_format_rect.top),  m_format_rect.bottom - 1);
}


/*********************************************************************
*
*  EM_LINEFROMCHAR
*
*/
INT EDITSTATE::EM_LineFromChar(INT index)
{
  INT line;
  LINEDEF *line_def;

  if (!(m_style & ES_MULTILINE))
    return 0;
  if (index > (INT)get_text_length())
    return m_line_count - 1;
  if (index == -1)
    index = min(m_selection_start, m_selection_end);

  line = 0;
  line_def = m_first_line_def;
  index -= line_def->length;
  while ((index >= 0) && line_def->next) {
    line++;
    line_def = line_def->next;
    index -= line_def->length;
  }
  return line;
}


/*********************************************************************
*
*  EM_LINEINDEX
*
*/
INT EDITSTATE::EM_LineIndex(INT line) const
{
  INT line_index;
  const LINEDEF *line_def;

  if (!(m_style & ES_MULTILINE))
    return 0;
  if (line >= m_line_count)
    return -1;

  line_index = 0;
  line_def = m_first_line_def;
  if (line == -1) {
    INT index = m_selection_end - line_def->length;
    while ((index >= 0) && line_def->next) {
      line_index += line_def->length;
      line_def = line_def->next;
      index -= line_def->length;
    }
  } else {
    while (line > 0) {
      line_index += line_def->length;
      line_def = line_def->next;
      line--;
    }
  }
  return line_index;
}


/*********************************************************************
*
*  EM_LINELENGTH
*
*/
INT EDITSTATE::EM_LineLength(INT index)
{
  LINEDEF *line_def;

  if (!(m_style & ES_MULTILINE))
    return get_text_length();

  if (index == -1) {
    /* get the number of remaining non-selected chars of selected lines */
    INT32 l; /* line number */
    INT32 li; /* index of first char in line */
    INT32 count;
    l = EM_LineFromChar(m_selection_start);
    /* # chars before start of selection area */
    count = m_selection_start - EM_LineIndex(l);
    l = EM_LineFromChar(m_selection_end);
    /* # chars after end of selection */
    li = EM_LineIndex(l);
    count += li + EM_LineLength(li) - m_selection_end;
    return count;
  }
  line_def = m_first_line_def;
  index -= line_def->length;
  while ((index >= 0) && line_def->next) {
    line_def = line_def->next;
    index -= line_def->length;
  }
  return line_def->net_length;
}


/*********************************************************************
*
*  EM_POSFROMCHAR
*
*/
GXLRESULT EDITSTATE::EM_PosFromChar(INT index, GXBOOL after_wrap)
{
  GXINT len = get_text_length();
  GXINT l;
  GXINT li;
  GXINT x = 0;
  GXINT y = 0;
  GXINT w;
  GXINT lw;
  LINEDEF* line_def;

  index = min(index, len);
  if (m_style & ES_MULTILINE) {
    l = EM_LineFromChar(index);
    UpdateUniscribeData(NULL, l);

    y = (l - m_y_offset) * m_line_height;
    li = EM_LineIndex(l);
    if (after_wrap && (li == index) && l) {
      INT l2 = l - 1;
      line_def = m_first_line_def;
      while (l2) {
        line_def = line_def->next;
        l2--;
      }
      if (line_def->ending == END_WRAP) {
        l--;
        y -= m_line_height;
        li = EM_LineIndex(l);
      }
    }

    line_def = m_first_line_def;
    while (line_def->index != li)
      line_def = line_def->next;

    lw = line_def->width;
    w = m_format_rect.right - m_format_rect.left;
    if (line_def->ssa)
    {
      gxScriptStringCPtoX(line_def->ssa, (index - 1) - li, TRUE, &x);
      x -= m_x_offset;
    }
    else
      x = m_x_offset;

    if (m_style & ES_RIGHT)
      x = w - (lw - x);
    else if (m_style & ES_CENTER)
      x += (w - lw) / 2;
  } else {
    INT xoff = 0;
    INT xi = 0;
    UpdateUniscribeData( NULL, 0);
    if (m_x_offset)
    {
      if (m_ssa)
      {
        if (m_x_offset >= (GXINT)get_text_length())
        {
          int leftover = m_x_offset - get_text_length();
          if (m_ssa)
          {
            const GXSIZE *size;
            size = gxScriptString_pSize(m_ssa);
            xoff = size->cx;
          }
          else
            xoff = 0;
          xoff += m_char_width * leftover;
        }
        else
          gxScriptStringCPtoX(m_ssa, m_x_offset, FALSE, &xoff);
      }
      else
        xoff = 0;
    }
    if (index)
    {
      if (index >= (GXINT)get_text_length())
      {
        if (m_ssa)
        {
          const GXSIZE *size;
          size = gxScriptString_pSize(m_ssa);
          xi = size->cx;
        }
        else
          xi = 0;
      }
      else if (m_ssa) {
        gxScriptStringCPtoX(m_ssa, index, FALSE, &xi);
      }
      else {
        xi = 0;
      }
    }
    x = xi - xoff;

    if (index >= m_x_offset) {
      if (!m_x_offset && (m_style & (ES_RIGHT | ES_CENTER)))
      {
        w = m_format_rect.right - m_format_rect.left;
        if (w > m_text_width)
        {
          if (m_style & ES_RIGHT)
            x += w - m_text_width;
          else if (m_style & ES_CENTER)
            x += (w - m_text_width) / 2;
        }
      }
    }
    y = 0;
  }
  x += m_format_rect.left;
  y += m_format_rect.top;
  return MAKELONG((INT16)x, (INT16)y);
}


/*********************************************************************
*
*  _GetLineRect
*
*  Calculates the bounding rectangle for a line from a starting
*  column to an ending column.
*
*/
void EDITSTATE::GetLineRect(INT line, INT scol, INT ecol, GXLPRECT rc)
{
  SCRIPT_STRING_ANALYSIS ssa;
  INT line_index = 0;
  INT pt1, pt2, pt3;

  if (m_style & ES_MULTILINE)
  {
    const LINEDEF *line_def = NULL;
    rc->top = m_format_rect.top + (line - m_y_offset) * m_line_height;
    if (line >= m_line_count)
      return;

    line_def = m_first_line_def;
    if (line == -1) {
      INT index = m_selection_end - line_def->length;
      while ((index >= 0) && line_def->next) {
        line_index += line_def->length;
        line_def = line_def->next;
        index -= line_def->length;
      }
    } else {
      while (line > 0) {
        line_index += line_def->length;
        line_def = line_def->next;
        line--;
      }
    }
    ssa = line_def->ssa;
  }
  else
  {
    line_index = 0;
    rc->top = m_format_rect.top;
    ssa = m_ssa;
  }

  rc->bottom = rc->top + m_line_height;
  pt1 = (scol == 0) ? m_format_rect.left : (short)LOWORD(EM_PosFromChar(line_index + scol, TRUE));
  pt2 = (ecol == -1) ? m_format_rect.right : (short)LOWORD(EM_PosFromChar(line_index + ecol, TRUE));
  if (ssa)
  {
    gxScriptStringCPtoX(ssa, scol, FALSE, &pt3);
    pt3+=m_format_rect.left;
  }
  else pt3 = pt1;
  rc->right = max(max(pt1 , pt2),pt3);
  rc->left = min(min(pt1, pt2),pt3);
}


void EDITSTATE::text_buffer_changed()
{
  m_text_length = (GXUINT)-1;

  //HeapFree( GetProcessHeap(), 0, m_logAttr );
  //m_logAttr = NULL;
  SAFE_DELETE_ARRAY(m_logAttr);
  InvalidateUniscribeData();
}

/*********************************************************************
* _LockBuffer
*
*/
void EDITSTATE::LockBuffer()
{
  if (m_hlocapp) return;

  if (!m_text) {

    if(!m_hloc32W) return;

    if(m_hloc32A)
    {
      GXCHAR* textA = (GXCHAR*)gxLocalLock(m_hloc32A);
      GXHLOCAL hloc32W_new;
      GXUINT countW_new = MultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
      if(countW_new > m_buffer_size + 1)
      {
        GXUINT alloc_size = ROUND_TO_GROW(countW_new * sizeof(WCHAR));
        TRACE("Resizing 32-bit UNICODE buffer from %d+1 to %d WCHARs\n", m_buffer_size, countW_new);
        hloc32W_new = gxLocalReAlloc(m_hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if(hloc32W_new)
        {
          m_hloc32W = hloc32W_new;
          m_buffer_size = LocalSize(hloc32W_new)/sizeof(WCHAR) - 1;
          TRACE("Real new size %d+1 WCHARs\n", m_buffer_size);
        }
        else
          WARN("FAILED! Will synchronize partially\n");
      }
      m_text = (GXWCHAR*)gxLocalLock(m_hloc32W);
      MultiByteToWideChar(CP_ACP, 0, textA, -1, m_text, m_buffer_size + 1);
      LocalUnlock(m_hloc32A);
    }
    else {
      m_text = (GXWCHAR*)gxLocalLock(m_hloc32W);
    }
  }
  m_lock_count++;
}


/*********************************************************************
*
*  _UnlockBuffer
*
*/
void EDITSTATE::UnlockBuffer(GXBOOL force)
{
  if (m_hlocapp) return;

  /* Edit window might be already destroyed */
  if( ! gxIsWindow(m_hwndSelf))
  {
    WARN("edit hwnd %p already destroyed\n", m_hwndSelf);
    return;
  }

  if (!m_lock_count) {
    ERR("lock_count == 0 ... please report\n");
    return;
  }
  if (!m_text) {
    ERR("m_text == 0 ... please report\n");
    return;
  }

  if (force || (m_lock_count == 1)) {
    if (m_hloc32W) {
      GXUINT countA = 0;
      GXUINT countW = get_text_length() + 1;

      if(m_hloc32A)
      {
        GXUINT countA_new = WideCharToMultiByte(CP_ACP, 0, m_text, countW, NULL, 0, NULL, NULL);
        TRACE("Synchronizing with 32-bit ANSI buffer\n");
        TRACE("%d WCHARs translated to %d bytes\n", countW, countA_new);
        countA = LocalSize(m_hloc32A);
        if(countA_new > countA)
        {
          GXHLOCAL hloc32A_new;
          GXUINT alloc_size = ROUND_TO_GROW(countA_new);
          TRACE("Resizing 32-bit ANSI buffer from %d to %d bytes\n", countA, alloc_size);
          hloc32A_new = gxLocalReAlloc(m_hloc32A, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
          if(hloc32A_new)
          {
            m_hloc32A = hloc32A_new;
            countA = LocalSize(hloc32A_new);
            TRACE("Real new size %d bytes\n", countA);
          }
          else
            WARN("FAILED! Will synchronize partially\n");
        }
        gxWideCharToMultiByte(CP_ACP, 0, m_text, countW, (GXLPSTR)gxLocalLock(m_hloc32A), countA, NULL, NULL);
        gxLocalUnlock(m_hloc32A);
      }

      LocalUnlock(m_hloc32W);
      m_text = NULL;
    }
    else {
      ERR("no buffer ... please report\n");
      return;
    }
  }
  m_lock_count--;
}


/*********************************************************************
*
*  _MakeFit
*
* Try to fit size + 1 characters in the buffer.
*/
GXBOOL EDITSTATE::MakeFit(GXUINT size)
{
  GXHLOCAL hNew32W;

  if (size <= m_buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1 characters\n", size);

  /* Force edit to unlock its buffer. m_text now NULL */
  UnlockBuffer(TRUE);

  if (m_hloc32W) {
    GXUINT alloc_size = ROUND_TO_GROW((size + 1) * sizeof(WCHAR));
    if ((hNew32W = gxLocalReAlloc(m_hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT))) {
      TRACE("Old 32 bit handle %p, new handle %p\n", m_hloc32W, hNew32W);
      m_hloc32W = hNew32W;
      m_buffer_size = LocalSize(hNew32W)/sizeof(WCHAR) - 1;
    }
  }

  LockBuffer();

  if (m_buffer_size < size) {
    WARN("FAILED !  We now have %d+1\n", m_buffer_size);
    _NOTIFY_PARENT(EN_ERRSPACE);
    return FALSE;
  } else {
    TRACE("We now have %d+1\n", m_buffer_size);
    return TRUE;
  }
}


/*********************************************************************
*
*  _MakeUndoFit
*
*  Try to fit size + 1 bytes in the undo buffer.
*
*/
GXBOOL EDITSTATE::MakeUndoFit(GXUINT size)
{
  GXUINT alloc_size;

  if (size <= m_undo_buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1\n", size);

  alloc_size = ROUND_TO_GROW((size + 1) * sizeof(WCHAR));
  if ((m_undo_text = (GXWCHAR*)gxHeapReAlloc(gxGetProcessHeap(), HEAP_ZERO_MEMORY, m_undo_text, alloc_size))) {
    m_undo_buffer_size = alloc_size/sizeof(WCHAR) - 1;
    return TRUE;
  }
  else
  {
    WARN("FAILED !  We now have %d+1\n", m_undo_buffer_size);
    return FALSE;
  }
}


/*********************************************************************
*
*  _UpdateTextRegion
*
*/
void EDITSTATE::UpdateTextRegion(GXHRGN hrgn, GXBOOL bErase)
{
  if (m_flags & EF_UPDATE) {
    m_flags &= ~EF_UPDATE;
    _NOTIFY_PARENT(EN_UPDATE);
  }
  gxInvalidateRgn(m_hwndSelf, hrgn, bErase);
}


/*********************************************************************
*
*  _UpdateText
*
*/
void EDITSTATE::UpdateText(const GXRECT *rc, GXBOOL bErase)
{
  if (m_flags & EF_UPDATE) {
    m_flags &= ~EF_UPDATE;
    _NOTIFY_PARENT(EN_UPDATE);
  }
  gxInvalidateRect(m_hwndSelf, rc, bErase);
}

/*********************************************************************
*
*  _SL_InvalidateText
*
*  Called from _InvalidateText().
*  Does the job for single-line controls only.
*
*/
void EDITSTATE::SL_InvalidateText(INT start, INT end)
{
  GXRECT line_rect;
  GXRECT rc;

  GetLineRect(0, start, end, &line_rect);
  if (gxIntersectRect(&rc, &line_rect, &m_format_rect))
    UpdateText(&rc, TRUE);
}

/*********************************************************************
*
*  _ML_InvalidateText
*
*  Called from _InvalidateText().
*  Does the job for multi-line controls only.
*
*/
void EDITSTATE::ML_InvalidateText(INT start, INT end)
{
  INT vlc = get_vertical_line_count();
  INT sl = EM_LineFromChar(start);
  INT el = EM_LineFromChar(end);
  INT sc;
  INT ec;
  GXRECT rc1;
  GXRECT rcWnd;
  GXRECT rcLine;
  GXRECT rcUpdate;
  INT l;

  if ((el < m_y_offset) || (sl > m_y_offset + vlc))
    return;

  sc = start - EM_LineIndex(sl);
  ec = end - EM_LineIndex(el);
  if (sl < m_y_offset) {
    sl = m_y_offset;
    sc = 0;
  }
  if (el > m_y_offset + vlc) {
    el = m_y_offset + vlc;
    ec = EM_LineLength(EM_LineIndex(el));
  }
  gxGetClientRect(m_hwndSelf, &rc1);
  gxIntersectRect(&rcWnd, &rc1, &m_format_rect);
  if (sl == el) {
    GetLineRect(sl, sc, ec, &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine)) {
      UpdateText(&rcUpdate, TRUE);
    }
  } else {
    GetLineRect(sl, sc,
      EM_LineLength(EM_LineIndex(sl)),
      &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      UpdateText(&rcUpdate, TRUE);
    for (l = sl + 1 ; l < el ; l++) {
      GetLineRect(l, 0, EM_LineLength(EM_LineIndex(l)),
        &rcLine);
      if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
        UpdateText(&rcUpdate, TRUE);
    }
    GetLineRect(el, 0, ec, &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      UpdateText(&rcUpdate, TRUE);
  }
}


/*********************************************************************
*
*  _InvalidateText
*
*  Invalidate the text from offset start up to, but not including,
*  offset end.  Useful for (re)painting the selection.
*  Regions outside the linewidth are not invalidated.
*  end == -1 means end == TextLength.
*  start and end need not be ordered.
*
*/
void EDITSTATE::InvalidateText(INT start, INT end)
{
  if (end == start)
    return;

  if (end == -1)
    end = get_text_length();

  if (end < start) {
    INT tmp = start;
    start = end;
    end = tmp;
  }

  if (m_style & ES_MULTILINE) {
    ML_InvalidateText(start, end);
  }
  else {
    SL_InvalidateText(start, end);
  }
}


/*********************************************************************
*
*  _EM_SetSel
*
*  note:  unlike the specs say: the order of start and end
*    _is_ preserved in Windows.  (i.e. start can be > end)
*    In other words: this handler is OK
*
*/
void EDITSTATE::EM_SetSel(GXUINT start, GXUINT end, GXBOOL after_wrap)
{
  GXUINT old_start = m_selection_start;
  GXUINT old_end = m_selection_end;
  GXUINT len = get_text_length();

  if (start == (GXUINT)-1) {
    start = m_selection_end;
    end = m_selection_end;
  } else {
    start = min(start, len);
    end = min(end, len);
  }
  m_selection_start = start;
  m_selection_end = end;
  if (after_wrap)
    m_flags |= EF_AFTER_WRAP;
  else
    m_flags &= ~EF_AFTER_WRAP;
  /* Compute the necessary invalidation region. */
  /* Note that we don't need to invalidate regions which have
  * "never" been selected, or those which are "still" selected.
  * In fact, every time we hit a selection boundary, we can
  * *toggle* whether we need to invalidate.  Thus we can optimize by
  * *sorting* the interval endpoints.  Let's assume that we sort them
  * in this order:
  *        start <= end <= old_start <= old_end
  * Knuth 5.3.1 (p 183) assures us that this can be done optimally
  * in 5 comparisons; i.e. it is impossible to do better than the
  * following: */
  ORDER_UINT(end, old_end);
  ORDER_UINT(start, old_start);
  ORDER_UINT(old_start, old_end);
  ORDER_UINT(start, end);
  /* Note that at this point 'end' and 'old_start' are not in order, but
  * start is definitely the min. and old_end is definitely the max. */
  if (end != old_start)
  {
    /*
    * One can also do
    *          ORDER_UINT32(end, old_start);
    *          _InvalidateText(start, end);
    *          _InvalidateText(old_start, old_end);
    * in place of the following if statement.
    * (That would complete the optimal five-comparison four-element sort.)
    */
    if (old_start > end )
    {
      InvalidateText(start, end);
      InvalidateText(old_start, old_end);
    }
    else
    {
      InvalidateText(start, old_start);
      InvalidateText(end, old_end);
    }
  }
  else {
    InvalidateText(start, old_end);
  }
}


/*********************************************************************
*
*  _UpdateScrollInfo
*
*/
void EDITSTATE::UpdateScrollInfo()
{
  if ((m_style & WS_VSCROLL) && !(m_flags & EF_VSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin   = 0;
    si.nMax   = m_line_count - 1;
    si.nPage  = (m_format_rect.bottom - m_format_rect.top) / m_line_height;
    si.nPos   = m_y_offset;

    TRACE("SB_VERT, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n", si.nMin, si.nMax, si.nPage, si.nPos);

    gxSetScrollInfo(m_hwndSelf, SB_VERT, &si, TRUE);
  }

  if ((m_style & WS_HSCROLL) && !(m_flags & EF_HSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize  = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin    = 0;
    si.nMax    = m_text_width - 1;
    si.nPage  = m_format_rect.right - m_format_rect.left;
    si.nPos    = m_x_offset;
    TRACE("SB_HORZ, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n",
      si.nMin, si.nMax, si.nPage, si.nPos);
    gxSetScrollInfo(m_hwndSelf, SB_HORZ, &si, TRUE);
  }
}


/*********************************************************************
*
*  _EM_LineScroll_internal
*
*  Version of _EM_LineScroll for internal use.
*  It doesn't refuse if ES_MULTILINE is set and assumes that
*  dx is in pixels, dy - in lines.
*
*/
GXBOOL EDITSTATE::EM_LineScroll_internal(INT dx, INT dy)
{
  INT nyoff;
  INT x_offset_in_pixels;
  INT lines_per_page = (m_format_rect.bottom - m_format_rect.top) /
    m_line_height;

  if (m_style & ES_MULTILINE)
  {
    x_offset_in_pixels = m_x_offset;
  }
  else
  {
    dy = 0;
    x_offset_in_pixels = (short)GXLOWORD(EM_PosFromChar(m_x_offset, FALSE));
  }

  if (-dx > x_offset_in_pixels)
    dx = -x_offset_in_pixels;
  if (dx > m_text_width - x_offset_in_pixels)
    dx = m_text_width - x_offset_in_pixels;
  nyoff = max(0, m_y_offset + dy);
  if (nyoff >= m_line_count - lines_per_page)
    nyoff = max(0, m_line_count - lines_per_page);
  dy = (m_y_offset - nyoff) * m_line_height;
  if (dx || dy) {
    GXRECT rc1;
    GXRECT rc;

    m_y_offset = nyoff;
    if(m_style & ES_MULTILINE)
      m_x_offset += dx;
    else
      m_x_offset += dx / m_char_width;

    gxGetClientRect(m_hwndSelf, &rc1);
    gxIntersectRect(&rc, &rc1, &m_format_rect);
    gxScrollWindowEx(m_hwndSelf, -dx, dy,
      NULL, &rc, NULL, NULL, SW_INVALIDATE);
    /* force scroll info update */
    UpdateScrollInfo();
  }
  if (dx && !(m_flags & EF_HSCROLL_TRACK))
    _NOTIFY_PARENT(EN_HSCROLL);
  if (dy && !(m_flags & EF_VSCROLL_TRACK))
    _NOTIFY_PARENT(EN_VSCROLL);
  return TRUE;
}

/*********************************************************************
*
*  EM_LINESCROLL
*
*  NOTE: dx is in average character widths, dy - in lines;
*
*/
GXBOOL EDITSTATE::EM_LineScroll(INT dx, INT dy)
{
  if (!(m_style & ES_MULTILINE))
    return FALSE;

  dx *= m_char_width;
  return EM_LineScroll_internal(dx, dy);
}


/*********************************************************************
*
*  EM_SCROLL
*
*/
GXLRESULT EDITSTATE::EM_Scroll(INT action)
{
  INT dy;

  if (!(m_style & ES_MULTILINE))
    return (GXLRESULT)FALSE;

  dy = 0;

  switch (action) {
  case SB_LINEUP:
    if (m_y_offset)
      dy = -1;
    break;
  case SB_LINEDOWN:
    if (m_y_offset < m_line_count - 1)
      dy = 1;
    break;
  case SB_PAGEUP:
    if (m_y_offset)
      dy = -(m_format_rect.bottom - m_format_rect.top) / m_line_height;
    break;
  case SB_PAGEDOWN:
    if (m_y_offset < m_line_count - 1)
      dy = (m_format_rect.bottom - m_format_rect.top) / m_line_height;
    break;
  default:
    return (GXLRESULT)FALSE;
  }
  if (dy) {
    GXINT vlc = get_vertical_line_count();
    /* check if we are going to move too far */
    if(m_y_offset + dy > m_line_count - vlc)
      dy = max(m_line_count - vlc, 0) - m_y_offset;

    /* Notification is done in _EM_LineScroll */
    if(dy) {
      EM_LineScroll(0, dy);
      return MAKELONG(dy, TRUE);
    }

  }
  return (GXLRESULT)FALSE;
}


/*********************************************************************
*
*  _SetCaretPos
*
*/
void EDITSTATE::SetCaretPos(INT pos, GXBOOL after_wrap)
{
  GXLRESULT res = EM_PosFromChar(pos, after_wrap);
  TRACE("%d - %dx%d\n", pos, (short)LOWORD(res), (short)HIWORD(res));
  SetCaretPos((short)LOWORD(res), (short)HIWORD(res));
}


/*********************************************************************
*
*  EM_SCROLLCARET
*
*/
void EDITSTATE::EM_ScrollCaret()
{
  if (m_style & ES_MULTILINE) {
    INT l;
    INT vlc;
    INT ww;
    INT cw = m_char_width;
    INT x;
    INT dy = 0;
    INT dx = 0;

    l = EM_LineFromChar(m_selection_end);
    x = (short)LOWORD(EM_PosFromChar(m_selection_end, m_flags & EF_AFTER_WRAP));
    vlc = get_vertical_line_count();
    if (l >= m_y_offset + vlc)
      dy = l - vlc + 1 - m_y_offset;
    if (l < m_y_offset)
      dy = l - m_y_offset;
    ww = m_format_rect.right - m_format_rect.left;
    if (x < m_format_rect.left)
      dx = x - m_format_rect.left - ww / HSCROLL_FRACTION / cw * cw;
    if (x > m_format_rect.right)
      dx = x - m_format_rect.left - (HSCROLL_FRACTION - 1) * ww / HSCROLL_FRACTION / cw * cw;
    if (dy || dx || (m_y_offset && (m_line_count - m_y_offset < vlc)))
    {
      /* check if we are going to move too far */
      if(m_x_offset + dx + ww > m_text_width)
        dx = m_text_width - ww - m_x_offset;
      if(dx || dy || (m_y_offset && (m_line_count - m_y_offset < vlc))) {
        EM_LineScroll_internal(dx, dy);
      }
    }
  } else {
    INT x;
    INT goal;
    INT format_width;

    x = (short)GXLOWORD(EM_PosFromChar(m_selection_end, FALSE));
    format_width = m_format_rect.right - m_format_rect.left;
    if (x < m_format_rect.left) {
      goal = m_format_rect.left + format_width / HSCROLL_FRACTION;
      do {
        m_x_offset--;
        x = (short)GXLOWORD(EM_PosFromChar(m_selection_end, FALSE));
      } while ((x < goal) && m_x_offset);
      /* FIXME: use ScrollWindow() somehow to improve performance */
      UpdateText(NULL, TRUE);
    } else if (x > m_format_rect.right) {
      INT x_last;
      INT len = get_text_length();
      goal = m_format_rect.right - format_width / HSCROLL_FRACTION;
      do {
        m_x_offset++;
        x = (short)LOWORD(EM_PosFromChar(m_selection_end, FALSE));
        x_last = (short)LOWORD(EM_PosFromChar(len, FALSE));
      } while ((x > goal) && (x_last > m_format_rect.right));
      /* FIXME: use ScrollWindow() somehow to improve performance */
      UpdateText(NULL, TRUE);
    }
  }

  if(m_flags & EF_FOCUSED) {
    SetCaretPos(m_selection_end, m_flags & EF_AFTER_WRAP);
  }
}


/*********************************************************************
*
*  _MoveBackward
*
*/
void EDITSTATE::MoveBackward(GXBOOL extend)
{
  INT e = m_selection_end;

  if (e) {
    e--;
    if ((m_style & ES_MULTILINE) && e &&
      (m_text[e - 1] == '\r') && (m_text[e] == '\n')) {
        e--;
        if (e && (m_text[e - 1] == '\r'))
          e--;
    }
  }
  EM_SetSel(extend ? m_selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveDown_ML
*
*  Only for multi line controls
*  Move the caret one line down, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MoveDown_ML(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  GXBOOL after_wrap = (m_flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar(e, after_wrap);
  INT x = (short)LOWORD(pos);
  INT y = (short)HIWORD(pos);

  e = CharFromPos(x, y + m_line_height, &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveEnd
*
*/
void EDITSTATE::MoveEnd(GXBOOL extend, GXBOOL ctrl)
{
  GXBOOL after_wrap = FALSE;
  INT e;

  /* Pass a high value in x to make sure of receiving the end of the line */
  if (!ctrl && (m_style & ES_MULTILINE))
    e = CharFromPos(0x3fffffff,
    HIWORD(EM_PosFromChar(m_selection_end, m_flags & EF_AFTER_WRAP)), &after_wrap);
  else
    e = get_text_length();
  EM_SetSel(extend ? m_selection_start : e, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveForward
*
*/
void EDITSTATE::MoveForward(GXBOOL extend)
{
  INT e = m_selection_end;

  if (m_text[e]) {
    e++;
    if ((m_style & ES_MULTILINE) && (m_text[e - 1] == '\r')) {
      if (m_text[e] == '\n')
        e++;
      else if ((m_text[e] == '\r') && (m_text[e + 1] == '\n'))
        e += 2;
    }
  }
  EM_SetSel(extend ? m_selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveHome
*
*  Home key: move to beginning of line.
*
*/
void EDITSTATE::MoveHome(GXBOOL extend, GXBOOL ctrl)
{
  INT e;

  /* Pass the x_offset in x to make sure of receiving the first position of the line */
  if (!ctrl && (m_style & ES_MULTILINE))
    e = CharFromPos(-m_x_offset,
    HIWORD(EM_PosFromChar(m_selection_end, m_flags & EF_AFTER_WRAP)), NULL);
  else
    e = 0;
  EM_SetSel(extend ? m_selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MovePageDown_ML
*
*  Only for multi line controls
*  Move the caret one page down, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MovePageDown_ML(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  GXBOOL after_wrap = (m_flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar(e, after_wrap);
  INT x = (short)LOWORD(pos);
  INT y = (short)HIWORD(pos);

  e = CharFromPos(x, y + (m_format_rect.bottom - m_format_rect.top),
    &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MovePageUp_ML
*
*  Only for multi line controls
*  Move the caret one page up, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MovePageUp_ML(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  GXBOOL after_wrap = (m_flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar(e, after_wrap);
  INT x = (short)GXLOWORD(pos);
  INT y = (short)GXHIWORD(pos);

  e = CharFromPos(x, y - (m_format_rect.bottom - m_format_rect.top), &after_wrap);
  if (!extend) {
    s = e;
  }
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveUp_ML
*
*  Only for multi line controls
*  Move the caret one line up, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MoveUp_ML(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  GXBOOL after_wrap = (m_flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar(e, after_wrap);
  INT x = (short)LOWORD(pos);
  INT y = (short)HIWORD(pos);

  e = CharFromPos(x, y - m_line_height, &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveWordBackward
*
*/
void EDITSTATE::MoveWordBackward(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  INT l;
  INT ll;
  INT li;

  l  = EM_LineFromChar(e);
  ll = EM_LineLength(e);
  li = EM_LineIndex(l);
  if (e - li == 0) {
    if (l) {
      li = EM_LineIndex(l - 1);
      e = li + EM_LineLength(li);
    }
  } else {
    e = li + CallWordBreakProc(li, e - li, ll, WB_LEFT);
  }
  if (!extend)
    s = e;
  EM_SetSel(s, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _MoveWordForward
*
*/
void EDITSTATE::MoveWordForward(GXBOOL extend)
{
  INT s = m_selection_start;
  INT e = m_selection_end;
  INT l;
  INT ll;
  INT li;

  l  = EM_LineFromChar(e);
  ll = EM_LineLength(e);
  li = EM_LineIndex(l);
  if (e - li == ll) {
    if ((m_style & ES_MULTILINE) && (l != m_line_count - 1))
      e = EM_LineIndex(l + 1);
  } else {
    e = li + CallWordBreakProc(li, e - li + 1, ll, WB_RIGHT);
  }
  if (!extend)
    s = e;
  EM_SetSel(s, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  _PaintText
*
*/
INT EDITSTATE::PaintText(GXHDC dc, INT x, INT y, INT line, INT col, INT count, GXBOOL rev)
{
  COLORREF BkColor;
  COLORREF TextColor;
  GXLOGFONTW underline_font;
  GXHFONT hUnderline = 0;
  GXHFONT old_font = 0;
  INT ret;
  INT li;
  INT BkMode;
  GXSIZE size;

  if (!count)
    return 0;
  BkMode = gxGetBkMode(dc);
  BkColor = gxGetBkColor(dc);
  TextColor = gxGetTextColor(dc);
  if (rev) {
    if (m_composition_len == 0)
    {
      gxSetBkColor(dc, gxGetSysColor(COLOR_HIGHLIGHT));
      gxSetTextColor(dc, gxGetSysColor(COLOR_HIGHLIGHTTEXT));
      gxSetBkMode( dc, OPAQUE);
    }
    else
    {
      GXHFONT current = (GXHFONT)gxGetCurrentObject(dc, OBJ_FONT);
      gxGetObjectW(current,sizeof(GXLOGFONTW),&underline_font);
      underline_font.lfUnderline = TRUE;
      hUnderline = (GXHFONT)gxCreateFontIndirectW(&underline_font);
      old_font = (GXHFONT)gxSelectObject(dc,hUnderline);
    }
  }
  li = EM_LineIndex(line);
  if (m_style & ES_MULTILINE) {
    ret = (INT)GXLOWORD(gxTabbedTextOutW(dc, x, y, m_text + li + col, count,
      m_tabs_count, m_tabs, m_format_rect.left - m_x_offset));
  } else {
    gxTextOutW(dc, x, y, m_text + li + col, count);
    gxGetTextExtentPoint32W(dc, m_text + li + col, count, &size);
    ret = size.cx;
  }
  if (rev) {
    if (m_composition_len == 0)
    {
      gxSetBkColor(dc, BkColor);
      gxSetTextColor(dc, TextColor);
      gxSetBkMode( dc, BkMode);
    }
    else
    {
      if (old_font) {
        gxSelectObject(dc,old_font);
      }
      if (hUnderline) {
        gxDeleteObject(hUnderline);
      }
    }
  }
  return ret;
}


/*********************************************************************
*
*  _PaintLine
*
*/
void EDITSTATE::PaintLine(GXHDC dc, INT line, GXBOOL rev)
{
  INT s = 0;
  INT e = 0;
  INT li = 0;
  INT ll = 0;
  INT x;
  INT y;
  GXLRESULT pos;
  SCRIPT_STRING_ANALYSIS ssa;

  if (m_style & ES_MULTILINE) {
    INT vlc = get_vertical_line_count();

    if ((line < m_y_offset) || (line > m_y_offset + vlc) || (line >= m_line_count))
      return;
  } else if (line)
    return;

  TRACE("line=%d\n", line);

  ssa = UpdateUniscribeData(dc, line);
  pos = EM_PosFromChar(EM_LineIndex(line), FALSE);
  x = (short)GXLOWORD(pos);
  y = (short)GXHIWORD(pos);

  if (m_style & ES_MULTILINE)
  {
    int line_idx = line;
    x =  -m_x_offset;
    if (m_style & ES_RIGHT || m_style & ES_CENTER)
    {
      LINEDEF *line_def = m_first_line_def;
      int w, lw;

      while (line_def && line_idx)
      {
        line_def = line_def->next;
        line_idx--;
      }
      w = m_format_rect.right - m_format_rect.left;
      lw = line_def->width;

      if (m_style & ES_RIGHT)
        x = w - (lw - x);
      else if (m_style & ES_CENTER)
        x += (w - lw) / 2;
    }
    x += m_format_rect.left;
  }

  if (rev)
  {
    li = EM_LineIndex(line);
    ll = EM_LineLength(li);
    s = min(m_selection_start, m_selection_end);
    e = max(m_selection_start, m_selection_end);
    s = min(li + ll, max(li, s));
    e = min(li + ll, max(li, e));
  }

  if (ssa)
    gxScriptStringOut(ssa, x, y, 0, &m_format_rect, s - li, e - li, FALSE);
  else if (rev && (s != e) &&
    ((m_flags & EF_FOCUSED) || (m_style & ES_NOHIDESEL))) {
      x += PaintText(dc, x, y, line, 0, s - li, FALSE);
      x += PaintText(dc, x, y, line, s - li, e - s, TRUE);
      x += PaintText(dc, x, y, line, e - li, li + ll - e, FALSE);
  } else
    x += PaintText(dc, x, y, line, 0, ll, FALSE);
}


/*********************************************************************
*
*  _AdjustFormatRect
*
*  Adjusts the format rectangle for the current font and the
*  current client rectangle.
*
*/
void EDITSTATE::AdjustFormatRect()
{
  GXRECT ClientRect;

  m_format_rect.right = max(m_format_rect.right, m_format_rect.left + m_char_width);
  if (m_style & ES_MULTILINE)
  {
    INT fw, vlc, max_x_offset, max_y_offset;

    vlc = get_vertical_line_count();
    m_format_rect.bottom = m_format_rect.top + vlc * m_line_height;

    /* correct m_x_offset */
    fw = m_format_rect.right - m_format_rect.left;
    max_x_offset = m_text_width - fw;
    if(max_x_offset < 0) max_x_offset = 0;
    if(m_x_offset > max_x_offset)
      m_x_offset = max_x_offset;

    /* correct m_y_offset */
    max_y_offset = m_line_count - vlc;
    if(max_y_offset < 0) max_y_offset = 0;
    if(m_y_offset > max_y_offset)
      m_y_offset = max_y_offset;

    /* force scroll info update */
    UpdateScrollInfo();
  }
  else
    /* Windows doesn't care to fix text placement for SL controls */
    m_format_rect.bottom = m_format_rect.top + m_line_height;

  /* Always stay within the client area */
  gxGetClientRect(m_hwndSelf, &ClientRect);
  m_format_rect.bottom = min(m_format_rect.bottom, ClientRect.bottom);

  if ((m_style & ES_MULTILINE) && !(m_style & ES_AUTOHSCROLL))
    BuildLineDefs_ML(0, get_text_length(), 0, NULL);

  SetCaretPos(m_selection_end, m_flags & EF_AFTER_WRAP);
}


/*********************************************************************
*
*  _SetRectNP
*
*  note:  this is not (exactly) the handler called on EM_SETRECTNP
*    it is also used to set the rect of a single line control
*
*/
void EDITSTATE::SetRectNP(const GXRECT *rc)
{
  LONG_PTR ExStyle;
  INT bw, bh;
  ExStyle = gxGetWindowLongPtrW(m_hwndSelf, GWL_EXSTYLE);

  gxCopyRect(&m_format_rect, rc);

  if (ExStyle & WS_EX_CLIENTEDGE) {
    m_format_rect.left++;
    m_format_rect.right--;

    if (m_format_rect.bottom - m_format_rect.top
      >= m_line_height + 2)
    {
      m_format_rect.top++;
      m_format_rect.bottom--;
    }
  }
  else if (m_style & WS_BORDER) {
    bw = GetSystemMetrics(SM_CXBORDER) + 1;
    bh = GetSystemMetrics(SM_CYBORDER) + 1;
    m_format_rect.left += bw;
    m_format_rect.right -= bw;
    if (m_format_rect.bottom - m_format_rect.top
      >= m_line_height + 2 * bh)
    {
      m_format_rect.top += bh;
      m_format_rect.bottom -= bh;
    }
  }

  m_format_rect.left += m_left_margin;
  m_format_rect.right -= m_right_margin;
  AdjustFormatRect();
}


/*********************************************************************
*
*  EM_CHARFROMPOS
*
*      returns line number (not index) in high-order word of result.
*      NB : Q137805 is unclear about this. POINT * pointer in lParam apply
*      to Richedit, not to the edit control. Original documentation is valid.
*  FIXME: do the specs mean to return -1 if outside client area or
*    if outside formatting rectangle ???
*
*/
GXLRESULT EDITSTATE::EM_CharFromPos(INT x, INT y)
{
  GXPOINT pt;
  GXRECT rc;
  INT index;

  pt.x = x;
  pt.y = y;
  gxGetClientRect(m_hwndSelf, &rc);
  if ( ! gxPtInRect(&rc, pt))
    return -1;

  index = CharFromPos(x, y, NULL);
  return MAKELONG(index, EM_LineFromChar(index));
}


/*********************************************************************
*
*  EM_FMTLINES
*
* Enable or disable soft breaks.
* 
* This means: insert or remove the soft linebreak character (\r\r\n).
* Take care to check if the text still fits the buffer after insertion.
* If not, notify with EN_ERRSPACE.
* 
*/
GXBOOL EDITSTATE::EM_FmtLines(GXBOOL add_eol)
{
  m_flags &= (~EF_USE_SOFTBRK);
  if (add_eol) {
    m_flags |= EF_USE_SOFTBRK;
    FIXME("soft break enabled, not implemented\n");
  }
  return add_eol;
}


/*********************************************************************
*
*  EM_GETHANDLE
*
*  Hopefully this won't fire back at us.
*  We always start with a fixed buffer in the local heap.
*  Despite of the documentation says that the local heap is used
*  only if DS_LOCALEDIT flag is set, NT and 2000 always allocate
*  buffer on the local heap.
*
*/
GXHLOCAL EDITSTATE::EM_GetHandle()
{
  GXHLOCAL hLocal;

  if (!(m_style & ES_MULTILINE))
    return 0;

  if(m_is_unicode)
    hLocal = m_hloc32W;
  else
  {
    if(!m_hloc32A)
    {
      CHAR *textA;
      GXUINT countA, alloc_size;
      TRACE("Allocating 32-bit ANSI alias buffer\n");
      countA = gxWideCharToMultiByte(CP_ACP, 0, m_text, -1, NULL, 0, NULL, NULL);
      alloc_size = ROUND_TO_GROW(countA);
      if(!(m_hloc32A = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
      {
        ERR("Could not allocate %d bytes for 32-bit ANSI alias buffer\n", alloc_size);
        return 0;
      }
      textA = (CHAR*)gxLocalLock(m_hloc32A);
      gxWideCharToMultiByte(CP_ACP, 0, m_text, -1, textA, countA, NULL, NULL);
      gxLocalUnlock(m_hloc32A);
    }
    hLocal = m_hloc32A;
  }

  UnlockBuffer(TRUE);

  /* The text buffer handle belongs to the app */
  m_hlocapp = hLocal;

  TRACE("Returning %p, LocalSize() = %ld\n", hLocal, LocalSize(hLocal));
  return hLocal;
}


/*********************************************************************
*
*  EM_GETLINE
*
*/
INT EDITSTATE::EM_GetLine(INT line, LPWSTR dst, GXBOOL unicode)
{
  LPWSTR src;
  INT line_len, dst_len;
  INT i;

  if (m_style & ES_MULTILINE) {
    if (line >= m_line_count)
      return 0;
  } else
    line = 0;
  i = EM_LineIndex(line);
  src = m_text + i;
  line_len = EM_LineLength(i);
  dst_len = *(WORD *)dst;
  if(unicode)
  {
    if(dst_len <= line_len)
    {
      memcpy(dst, src, dst_len * sizeof(WCHAR));
      return dst_len;
    }
    else /* Append 0 if enough space */
    {
      memcpy(dst, src, line_len * sizeof(WCHAR));
      dst[line_len] = 0;
      return line_len;
    }
  }
  else
  {
    INT ret = gxWideCharToMultiByte(CP_ACP, 0, src, line_len, (LPSTR)dst, dst_len, NULL, NULL);
    if(!ret && line_len) /* Insufficient buffer size */
      return dst_len;
    if(ret < dst_len) /* Append 0 if enough space */
      ((LPSTR)dst)[ret] = 0;
    return ret;
  }
}


/*********************************************************************
*
*  EM_GETSEL
*
*/
GXLRESULT EDITSTATE::EM_GetSel(PUINT start, PUINT end) const
{
  GXUINT s = m_selection_start;
  GXUINT e = m_selection_end;

  ORDER_UINT(s, e);
  if (start)
    *start = s;
  if (end)
    *end = e;
  return MAKELONG(s, e);
}


/*********************************************************************
*
*  EM_REPLACESEL
*
*  FIXME: handle ES_NUMBER and ES_OEMCONVERT here
*
*/
void EDITSTATE::EM_ReplaceSel(GXBOOL can_undo, LPCWSTR lpsz_replace, GXBOOL send_update, GXBOOL honor_limit)
{
  GXUINT strl = GXSTRLEN(lpsz_replace);
  GXUINT tl = get_text_length();
  GXUINT utl;
  GXUINT s;
  GXUINT e;
  GXUINT i;
  GXUINT size;
  LPWSTR p;
  GXHRGN hrgn = 0;
  LPWSTR buf = NULL;
  GXUINT bufl;

  TRACE("%s, can_undo %d, send_update %d\n",
    debugstr_w(lpsz_replace), can_undo, send_update);

  s = m_selection_start;
  e = m_selection_end;

  InvalidateUniscribeData();
  if ((s == e) && !strl)
    return;

  ORDER_UINT(s, e);

  size = tl - (e - s) + strl;
  if (!size)
    m_text_width = 0;

  /* Issue the EN_MAXTEXT notification and continue with replacing text
  * so that buffer limit is honored. */
  if ((honor_limit) && (size > m_buffer_limit)) {
    _NOTIFY_PARENT(EN_MAXTEXT);
    /* Buffer limit can be smaller than the actual length of text in combobox */
    if (m_buffer_limit < (tl - (e-s)))
      strl = 0;
    else
      strl = m_buffer_limit - (tl - (e-s));
  }

  if ( ! MakeFit(tl - (e - s) + strl))
    return;

  if (e != s) {
    /* there is something to be deleted */
    TRACE("deleting stuff.\n");
    bufl = e - s;
    buf = new WCHAR[bufl + 1]; // HeapAlloc(GetProcessHeap(), 0, (bufl + 1) * sizeof(WCHAR));
    if ( ! buf) {
      return;
    }
    memcpy(buf, m_text + s, bufl * sizeof(WCHAR));
    buf[bufl] = 0; /* ensure 0 termination */
    /* now delete */
    strcpyW(m_text + s, m_text + e);
    text_buffer_changed();
  }
  if (strl) {
    /* there is an insertion */
    tl = get_text_length();
    TRACE("inserting stuff (tl %d, strl %d, selstart %d (%s), text %s)\n", tl, strl, s, debugstr_w(m_text + s), debugstr_w(m_text));
    for (p = m_text + tl ; p >= m_text + s ; p--)
      p[strl] = p[0];
    for (i = 0 , p = m_text + s ; i < strl ; i++)
      p[i] = lpsz_replace[i];
    if(m_style & ES_UPPERCASE)
      CharUpperBuffW(p, strl);
    else if(m_style & ES_LOWERCASE)
      CharLowerBuffW(p, strl);
    text_buffer_changed();
  }
  if (m_style & ES_MULTILINE)
  {
    INT st = min(m_selection_start, m_selection_end);
    INT vlc = get_vertical_line_count();

    hrgn = gxCreateRectRgn(0, 0, 0, 0);
    BuildLineDefs_ML(st, st + strl,
      strl - abs(m_selection_end - m_selection_start), hrgn);
    /* if text is too long undo all changes */
    if (honor_limit && !(m_style & ES_AUTOVSCROLL) && (m_line_count > vlc)) {
      if (strl)
        strcpyW(m_text + e, m_text + e + strl);
      if (e != s)
        for (i = 0 , p = m_text ; i < e - s ; i++)
          p[i + s] = buf[i];
      text_buffer_changed();
      BuildLineDefs_ML(s, e, abs(m_selection_end - m_selection_start) - strl, hrgn);
      strl = 0;
      e = s;
      hrgn = gxCreateRectRgn(0, 0, 0, 0);
      _NOTIFY_PARENT(EN_MAXTEXT);
    }
  }
  else {
    INT fw = m_format_rect.right - m_format_rect.left;
    InvalidateUniscribeData();
    CalcLineWidth_SL();
    /* remove chars that don't fit */
    if (honor_limit && !(m_style & ES_AUTOHSCROLL) && (m_text_width > fw)) {
      while ((m_text_width > fw) && s + strl >= s) {
        strcpyW(m_text + s + strl - 1, m_text + s + strl);
        strl--;
        m_text_length = -1;
        InvalidateUniscribeData();
        CalcLineWidth_SL();
      }
      text_buffer_changed();
      _NOTIFY_PARENT(EN_MAXTEXT);
    }
  }

  if (e != s) {
    if (can_undo) {
      utl = GXSTRLEN(m_undo_text);
      if (!m_undo_insert_count && (*m_undo_text && (s == m_undo_position))) {
        /* undo-buffer is extended to the right */
        MakeUndoFit(utl + e - s);
        memcpy(m_undo_text + utl, buf, (e - s)*sizeof(WCHAR));
        (m_undo_text + utl)[e - s] = 0; /* ensure 0 termination */
      } else if (!m_undo_insert_count && (*m_undo_text && (e == m_undo_position))) {
        /* undo-buffer is extended to the left */
        MakeUndoFit(utl + e - s);
        for (p = m_undo_text + utl ; p >= m_undo_text ; p--)
          p[e - s] = p[0];
        for (i = 0 , p = m_undo_text ; i < e - s ; i++)
          p[i] = buf[i];
        m_undo_position = s;
      } else {
        /* new undo-buffer */
        MakeUndoFit(e - s);
        memcpy(m_undo_text, buf, (e - s)*sizeof(WCHAR));
        m_undo_text[e - s] = 0; /* ensure 0 termination */
        m_undo_position = s;
      }
      /* any deletion makes the old insertion-undo invalid */
      m_undo_insert_count = 0;
    } else
      EM_EmptyUndoBuffer();
  }
  if (strl) {
    if (can_undo) {
      if ((s == m_undo_position) ||
        ((m_undo_insert_count) &&
        (s == m_undo_position + m_undo_insert_count)))
        /*
        * insertion is new and at delete position or
        * an extension to either left or right
        */
        m_undo_insert_count += strl;
      else {
        /* new insertion undo */
        m_undo_position = s;
        m_undo_insert_count = strl;
        /* new insertion makes old delete-buffer invalid */
        *m_undo_text = '\0';
      }
    } else
      EM_EmptyUndoBuffer();
  }

  //HeapFree(GetProcessHeap(), 0, buf);
  SAFE_DELETE_ARRAY(buf);

  s += strl;

  /* If text has been deleted and we're right or center aligned then scroll rightward */
  if (m_style & (ES_RIGHT | ES_CENTER))
  {
    INT delta = strl - abs(m_selection_end - m_selection_start);

    if (delta < 0 && m_x_offset)
    {
      if (abs(delta) > m_x_offset)
        m_x_offset = 0;
      else
        m_x_offset += delta;
    }
  }

  EM_SetSel(s, s, FALSE);
  m_flags |= EF_MODIFIED;
  if (send_update) m_flags |= EF_UPDATE;
  if (hrgn)
  {
    UpdateTextRegion(hrgn, TRUE);
    gxDeleteObject(hrgn);
  }
  else
    UpdateText(NULL, TRUE);

  EM_ScrollCaret();

  /* force scroll info update */
  UpdateScrollInfo();


  if(send_update || (m_flags & EF_UPDATE))
  {
    m_flags &= ~EF_UPDATE;
    _NOTIFY_PARENT(EN_CHANGE);
  }
  InvalidateUniscribeData();
}


/*********************************************************************
*
*  EM_SETHANDLE
*
*  FIXME:  ES_LOWERCASE, ES_UPPERCASE, ES_OEMCONVERT, ES_NUMBER ???
*
*/
void EDITSTATE::EM_SetHandle(GXHLOCAL hloc)
{
  if (!(m_style & ES_MULTILINE))
    return;

  if (!hloc) {
    WARN("called with NULL handle\n");
    return;
  }

  UnlockBuffer(TRUE);

  if(m_is_unicode)
  {
    if(m_hloc32A)
    {
      LocalFree(m_hloc32A);
      m_hloc32A = NULL;
    }
    m_hloc32W = hloc;
  }
  else
  {
    INT countW, countA;
    GXHLOCAL hloc32W_new;
    WCHAR *textW;
    CHAR *textA;

    countA = gxLocalSize(hloc);
    textA = (GXCHAR*)gxLocalLock(hloc);
    countW = MultiByteToWideChar(CP_ACP, 0, textA, countA, NULL, 0);
    if(!(hloc32W_new = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, countW * sizeof(WCHAR))))
    {
      ERR("Could not allocate new unicode buffer\n");
      return;
    }
    textW = (GXWCHAR*)gxLocalLock(hloc32W_new);
    gxMultiByteToWideChar(CP_ACP, 0, textA, countA, textW, countW);
    gxLocalUnlock(hloc32W_new);
    gxLocalUnlock(hloc);

    if(m_hloc32W) {
      gxLocalFree(m_hloc32W);
    }

    m_hloc32W = hloc32W_new;
    m_hloc32A = hloc;
  }

  m_buffer_size = LocalSize(m_hloc32W)/sizeof(WCHAR) - 1;

  /* The text buffer handle belongs to the control */
  m_hlocapp = NULL;

  LockBuffer();
  text_buffer_changed();

  m_x_offset = m_y_offset = 0;
  m_selection_start = m_selection_end = 0;
  EM_EmptyUndoBuffer();
  m_flags &= ~EF_MODIFIED;
  m_flags &= ~EF_UPDATE;
  BuildLineDefs_ML(0, get_text_length(), 0, NULL);
  UpdateText(NULL, TRUE);
  EM_ScrollCaret();
  /* force scroll info update */
  UpdateScrollInfo();
}


/*********************************************************************
*
*  EM_SETLIMITTEXT
*
*  NOTE: this version currently implements WinNT limits
*
*/
void EDITSTATE::EM_SetLimitText(GXUINT limit)
{
  if (!limit) limit = ~0u;
  if (!(m_style & ES_MULTILINE)) limit = min(limit, 0x7ffffffe);
  m_buffer_limit = limit;
}


/*********************************************************************
*
*  EM_SETMARGINS
*
* EC_USEFONTINFO is used as a left or right value i.e. lParam and not as an
* action wParam despite what the docs say. EC_USEFONTINFO calculates the
* margin according to the textmetrics of the current font.
*
* FIXME - With TrueType or vector fonts EC_USEFONTINFO currently sets one third
* of the char's width as the margin, but this is not how Windows handles this.
* For all other fonts Windows sets the margins to zero.
*
* FIXME - When EC_USEFONTINFO is used the margins only change if the
* edit control is equal to or larger than a certain size.
* Interestingly if one subtracts both the left and right margins from
* this size one always seems to get an even number.  The extents of
* the (four character) string "'**'" match this quite closely, so
* we'll use this until we come up with a better idea.
*/
static int calc_min_set_margin_size(GXHDC dc, INT left, INT right)
{
  static const WCHAR magic_string[] = {'\'','*','*','\'', 0};
  GXSIZE sz;

  gxGetTextExtentPointW(dc, magic_string, sizeof(magic_string)/sizeof(WCHAR) - 1, &sz);
  return sz.cx + left + right;
}

void EDITSTATE::EM_SetMargins(INT action, WORD left, WORD right, GXBOOL repaint)
{
  GXTEXTMETRICW tm;
  INT default_left_margin  = 0; /* in pixels */
  INT default_right_margin = 0; /* in pixels */

  /* Set the default margins depending on the font */
  if (m_font && (left == EC_USEFONTINFO || right == EC_USEFONTINFO)) {
    GXHDC dc = gxGetDC(m_hwndSelf);
    GXHFONT old_font = (GXHFONT)gxSelectObject(dc, m_font);
    gxGetTextMetricsW(dc, &tm);
    /* The default margins are only non zero for TrueType or Vector fonts */
    if (tm.tmPitchAndFamily & ( TMPF_VECTOR | TMPF_TRUETYPE )) {
      int min_size;
      GXRECT rc;
      /* This must be calculated more exactly! But how? */
      default_left_margin = tm.tmAveCharWidth / 2;
      default_right_margin = tm.tmAveCharWidth / 2;
      min_size = calc_min_set_margin_size(dc, default_left_margin, default_right_margin);
      gxGetClientRect(m_hwndSelf, &rc);
      if ( ! gxIsRectEmpty(&rc) && (rc.right - rc.left < min_size)) {
        default_left_margin = m_left_margin;
        default_right_margin = m_right_margin;
      }
    }
    gxSelectObject(dc, old_font);
    gxReleaseDC(m_hwndSelf, dc);
  }

  if (action & EC_LEFTMARGIN) {
    m_format_rect.left -= m_left_margin;
    if (left != EC_USEFONTINFO)
      m_left_margin = left;
    else
      m_left_margin = default_left_margin;
    m_format_rect.left += m_left_margin;
  }

  if (action & EC_RIGHTMARGIN) {
    m_format_rect.right += m_right_margin;
    if (right != EC_USEFONTINFO)
      m_right_margin = right;
    else
      m_right_margin = default_right_margin;
    m_format_rect.right -= m_right_margin;
  }

  if (action & (EC_LEFTMARGIN | EC_RIGHTMARGIN)) {
    AdjustFormatRect();
    if (repaint) {
      UpdateText(NULL, TRUE);
    }
  }

  TRACE("left=%d, right=%d\n", m_left_margin, m_right_margin);
}


/*********************************************************************
*
*  EM_SETPASSWORDCHAR
*
*/
void EDITSTATE::EM_SetPasswordChar(WCHAR c)
{
  LONG style;

  if (m_style & ES_MULTILINE)
    return;

  if (m_password_char == c)
    return;

  style = gxGetWindowLongW(m_hwndSelf, GWL_STYLE);
  m_password_char = c;
  if (c) {
    gxSetWindowLongW(m_hwndSelf, GWL_STYLE, style | ES_PASSWORD);
    m_style |= ES_PASSWORD;
  } else {
    gxSetWindowLongW(m_hwndSelf, GWL_STYLE, style & ~ES_PASSWORD);
    m_style &= ~ES_PASSWORD;
  }
  InvalidateUniscribeData();
  UpdateText(NULL, TRUE);
}


/*********************************************************************
*
*  EM_SETTABSTOPS
*
*/
GXBOOL EDITSTATE::EM_SetTabStops(INT count, const INT *tabs)
{
  if (!(m_style & ES_MULTILINE))
    return FALSE;
  HeapFree(GetProcessHeap(), 0, m_tabs);
  m_tabs_count = count;
  if (!count)
    m_tabs = NULL;
  else {
    m_tabs = new GXINT[count]; // HeapAlloc(GetProcessHeap(), 0, count * sizeof(INT));
    memcpy(m_tabs, tabs, count * sizeof(INT));
  }
  InvalidateUniscribeData();
  return TRUE;
}


/*********************************************************************
*
*  EM_SETWORDBREAKPROC
*
*/
void EDITSTATE::EM_SetWordBreakProc(void *wbp)
{
  if (m_word_break_proc == wbp)
    return;

  m_word_break_proc = wbp;

  if ((m_style & ES_MULTILINE) && ! (m_style & ES_AUTOHSCROLL)) {
    BuildLineDefs_ML(0, get_text_length(), 0, NULL);
    UpdateText(NULL, TRUE);
  }
}


/*********************************************************************
*
*  EM_UNDO / WM_UNDO
*
*/
GXBOOL EDITSTATE::EM_Undo()
{
  INT ulength;
  LPWSTR utext;

  /* As per MSDN spec, for a single-line edit control,
  the return value is always TRUE */
  if( m_style & ES_READONLY )
    return !(m_style & ES_MULTILINE);

  ulength = GXSTRLEN(m_undo_text);

  //utext = gxHeapAlloc(gxGetProcessHeap(), 0, (ulength + 1) * sizeof(WCHAR));
  utext = new GXWCHAR[ulength + 1];

  strcpyW(utext, m_undo_text);

  TRACE("before UNDO:insertion length = %d, deletion buffer = %s\n",
    m_undo_insert_count, debugstr_w(utext));

  EM_SetSel(m_undo_position, m_undo_position + m_undo_insert_count, FALSE);
  EM_EmptyUndoBuffer();
  EM_ReplaceSel(TRUE, utext, TRUE, TRUE);
  EM_SetSel(m_undo_position, m_undo_position + m_undo_insert_count, FALSE);
  /* send the notification after the selection start and end are set */
  _NOTIFY_PARENT(EN_CHANGE);
  EM_ScrollCaret();
  SAFE_DELETE_ARRAY(utext);
  //gxHeapFree(GetProcessHeap(), 0, utext);

  TRACE("after UNDO:insertion length = %d, deletion buffer = %s\n",
    m_undo_insert_count, debugstr_w(m_undo_text));
  return TRUE;
}


/* Helper function for WM_CHAR
*
* According to an MSDN blog article titled "Just because you're a control
* doesn't mean that you're necessarily inside a dialog box," multiline edit
* controls without ES_WANTRETURN would attempt to detect whether it is inside
* a dialog box or not.
*/
inline GXBOOL EDITSTATE::IsInsideDialog()
{
  return (m_flags & EF_DIALOGMODE);
}


/*********************************************************************
*
*  WM_PASTE
*
*/
void EDITSTATE::WM_Paste()
{
  GXHGLOBAL hsrc;
  GXLPWSTR src;

  /* Protect read-only edit control from modification */
  if(m_style & ES_READONLY)
    return;

  gxOpenClipboard(m_hwndSelf);
  if ((hsrc = (GXHGLOBAL)gxGetClipboardData(CF_UNICODETEXT))) {
    src = (GXLPWSTR)gxGlobalLock(hsrc);
    EM_ReplaceSel(TRUE, src, TRUE, TRUE);
    gxGlobalUnlock(hsrc);
  }
  else if (m_style & ES_PASSWORD) {
    /* clear selected text in password edit box even with empty clipboard */
    EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
  }
  gxCloseClipboard();
}


/*********************************************************************
*
*  WM_COPY
*
*/
void EDITSTATE::WM_Copy()
{
  INT s = min(m_selection_start, m_selection_end);
  INT e = max(m_selection_start, m_selection_end);
  GXHGLOBAL hdst;
  GXLPWSTR dst;
  GXDWORD len;

  if (e == s) return;

  len = e - s;
  hdst = gxGlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
  dst = (GXLPWSTR)gxGlobalLock(hdst);
  memcpy(dst, m_text + s, len * sizeof(WCHAR));
  dst[len] = 0; /* ensure 0 termination */
  TRACE("%s\n", debugstr_w(dst));
  gxGlobalUnlock(hdst);
  gxOpenClipboard(m_hwndSelf);
  gxEmptyClipboard();
  gxSetClipboardData(CF_UNICODETEXT, (GXHANDLE)hdst);
  gxCloseClipboard();
}


/*********************************************************************
*
*  WM_CLEAR
*
*/
void EDITSTATE::WM_Clear()
{
  /* Protect read-only edit control from modification */
  if(m_style & ES_READONLY)
    return;

  EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
}


/*********************************************************************
*
*  WM_CUT
*
*/
void EDITSTATE::WM_Cut()
{
  WM_Copy();
  WM_Clear();
}


/*********************************************************************
*
*  WM_CHAR
*
*/
GXLRESULT EDITSTATE::WM_Char(WCHAR c)
{
  GXBOOL control;

  control = GetKeyState(VK_CONTROL) & 0x8000;

  switch (c) {
  case '\r':
    /* If it's not a multiline edit box, it would be ignored below.
    * For multiline edit without ES_WANTRETURN, we have to make a
    * special case.
    */
    if ((m_style & ES_MULTILINE) && !(m_style & ES_WANTRETURN))
      if (IsInsideDialog())
        break;
  case '\n':
    if (m_style & ES_MULTILINE) {
      if (m_style & ES_READONLY) {
        MoveHome(FALSE, FALSE);
        MoveDown_ML(FALSE);
      } else {
        static const WCHAR cr_lfW[] = {'\r','\n',0};
        EM_ReplaceSel(TRUE, cr_lfW, TRUE, TRUE);
      }
    }
    break;
  case '\t':
    if ((m_style & ES_MULTILINE) && !(m_style & ES_READONLY))
    {
      static const WCHAR tabW[] = {'\t',0};
      if (IsInsideDialog())
        break;
      EM_ReplaceSel(TRUE, tabW, TRUE, TRUE);
    }
    break;
  case VK_BACK:
    if (!(m_style & ES_READONLY) && !control) {
      if (m_selection_start != m_selection_end)
        WM_Clear();
      else {
        /* delete character left of caret */
        EM_SetSel((GXUINT)-1, 0, FALSE);
        MoveBackward(TRUE);
        WM_Clear();
      }
    }
    break;
  case 0x03: /* ^C */
    if (!(m_style & ES_PASSWORD))
      gxSendMessageW(m_hwndSelf, WM_COPY, 0, 0);
    break;
  case 0x16: /* ^V */
    if (!(m_style & ES_READONLY))
      gxSendMessageW(m_hwndSelf, WM_PASTE, 0, 0);
    break;
  case 0x18: /* ^X */
    if (!((m_style & ES_READONLY) || (m_style & ES_PASSWORD)))
      gxSendMessageW(m_hwndSelf, WM_CUT, 0, 0);
    break;
  case 0x1A: /* ^Z */
    if (!(m_style & ES_READONLY))
      gxSendMessageW(m_hwndSelf, WM_UNDO, 0, 0);
    break;

  default:
    /*If Edit control style is ES_NUMBER allow users to key in only numeric values*/
    if( (m_style & ES_NUMBER) && !( c >= '0' && c <= '9') )
      break;

    if (!(m_style & ES_READONLY) && (c >= ' ') && (c != 127)) {
      WCHAR str[2];
      str[0] = c;
      str[1] = '\0';
      EM_ReplaceSel(TRUE, str, TRUE, TRUE);
    }
    break;
  }
  return 1;
}


/*********************************************************************
*
*  _ContextMenuCommand
*
*/
void EDITSTATE::ContextMenuCommand(GXUINT id)
{
  switch (id) {
  case EM_UNDO:
    gxSendMessageW(m_hwndSelf, WM_UNDO, 0, 0);
    break;
  case WM_CUT:
    gxSendMessageW(m_hwndSelf, WM_CUT, 0, 0);
    break;
  case WM_COPY:
    gxSendMessageW(m_hwndSelf, WM_COPY, 0, 0);
    break;
  case WM_PASTE:
    gxSendMessageW(m_hwndSelf, WM_PASTE, 0, 0);
    break;
  case WM_CLEAR:
    gxSendMessageW(m_hwndSelf, WM_CLEAR, 0, 0);
    break;
  case EM_SETSEL:
    gxSendMessageW(m_hwndSelf, EM_SETSEL, 0, -1);
    break;
  default:
    ERR("unknown menu item, please report\n");
    break;
  }
}


/*********************************************************************
*
*  WM_CONTEXTMENU
*
*  Note: the resource files resource/sysres_??.rc cannot define a
*    single popup menu.  Hence we use a (dummy) menubar
*    containing the single popup menu as its first item.
*
*  FIXME: the message identifiers have been chosen arbitrarily,
*    hence we use MF_BYPOSITION.
*    We might as well use the "real" values (anybody knows ?)
*    The menu definition is in resources/sysres_??.rc.
*    Once these are OK, we better use MF_BYCOMMAND here
*    (as we do in _WM_Command()).
*
*/
void EDITSTATE::WM_ContextMenu(INT x, INT y)
{
  GXHMENU menu = gxLoadMenuA(user32_module, "EDITMENU");
  GXHMENU popup = gxGetSubMenu(menu, 0);
  GXUINT start = m_selection_start;
  GXUINT end = m_selection_end;
  GXUINT cmd;

  ORDER_UINT(start, end);

  /* undo */
  gxEnableMenuItem(popup, 0, MF_BYPOSITION | (EM_CanUndo() && !(m_style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* cut */
  gxEnableMenuItem(popup, 2, MF_BYPOSITION | ((end - start) && !(m_style & ES_PASSWORD) && !(m_style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* copy */
  gxEnableMenuItem(popup, 3, MF_BYPOSITION | ((end - start) && !(m_style & ES_PASSWORD) ? MF_ENABLED : MF_GRAYED));
  /* paste */
  gxEnableMenuItem(popup, 4, MF_BYPOSITION | (IsClipboardFormatAvailable(CF_UNICODETEXT) && !(m_style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* delete */
  gxEnableMenuItem(popup, 5, MF_BYPOSITION | ((end - start) && !(m_style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* select all */
  gxEnableMenuItem(popup, 7, MF_BYPOSITION | (start || (end != get_text_length()) ? MF_ENABLED : MF_GRAYED));

  if (x == -1 && y == -1) /* passed via VK_APPS press/release */
  {
    GXRECT rc;
    /* Windows places the menu at the edit's center in this case */
    WIN_GetRectangles( m_hwndSelf, COORDS_SCREEN, NULL, &rc );
    x = rc.left + (rc.right - rc.left) / 2;
    y = rc.top + (rc.bottom - rc.top) / 2;
  }

  if (!(m_flags & EF_FOCUSED))
    gxSetFocus(m_hwndSelf);

  cmd = gxTrackPopupMenu(popup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
    x, y, 0, m_hwndSelf, NULL);

  if (cmd)
    ContextMenuCommand(cmd);

  gxDestroyMenu(menu);
}


/*********************************************************************
*
*  WM_GETTEXT
*
*/
INT EDITSTATE::WM_GetText(INT count, LPWSTR dst, GXBOOL unicode) const
{
  if(!count) return 0;

  if(unicode)
  {
    lstrcpynW(dst, m_text, count);
    return GXSTRLEN(dst);
  }
  else
  {
    LPSTR textA = (LPSTR)dst;
    if (!WideCharToMultiByte(CP_ACP, 0, m_text, -1, textA, count, NULL, NULL))
      textA[count - 1] = 0; /* ensure 0 termination */
    return strlen(textA);
  }
}

/*********************************************************************
*
*  _CheckCombo
*
*/
GXBOOL EDITSTATE::CheckCombo(GXUINT msg, INT key)
{
  GXHWND hLBox = m_hwndListBox;
  GXHWND hCombo;
  GXBOOL bDropped;
  int  nEUI;

  if (!hLBox)
    return FALSE;

  hCombo   = gxGetParent(m_hwndSelf);
  bDropped = TRUE;
  nEUI     = 0;

  TRACE_(combo)(L"[%p]: handling msg %x (%x)\n", m_hwndSelf, msg, key);

  if (key == VK_UP || key == VK_DOWN)
  {
    if (gxSendMessageW(hCombo, CB_GETEXTENDEDUI, 0, 0))
      nEUI = 1;

    if (msg == WM_KEYDOWN || nEUI)
      bDropped = (GXBOOL)gxSendMessageW(hCombo, CB_GETDROPPEDSTATE, 0, 0);
  }

  switch (msg)
  {
  case WM_KEYDOWN:
    if (!bDropped && nEUI && (key == VK_UP || key == VK_DOWN))
    {
      /* make sure ComboLBox pops up */
      gxSendMessageW(hCombo, CB_SETEXTENDEDUI, FALSE, 0);
      key = VK_F4;
      nEUI = 2;
    }

    gxSendMessageW(hLBox, WM_KEYDOWN, key, 0);
    break;

  case WM_SYSKEYDOWN: /* Handle Alt+up/down arrows */
    if (nEUI)
      gxSendMessageW(hCombo, CB_SHOWDROPDOWN, !bDropped, 0);
    else
      gxSendMessageW(hLBox, WM_KEYDOWN, VK_F4, 0);
    break;
  }

  if(nEUI == 2)
    gxSendMessageW(hCombo, CB_SETEXTENDEDUI, TRUE, 0);

  return TRUE;
}


/*********************************************************************
*
*  WM_KEYDOWN
*
*  Handling of special keys that don't produce a WM_CHAR
*  (i.e. non-printable keys) & Backspace & Delete
*
*/
GXLRESULT EDITSTATE::WM_KeyDown(INT key)
{
  GXBOOL shift;
  GXBOOL control;

  if (GetKeyState(VK_MENU) & 0x8000)
    return 0;

  shift = GetKeyState(VK_SHIFT) & 0x8000;
  control = GetKeyState(VK_CONTROL) & 0x8000;

  switch (key) {
  case VK_F4:
  case VK_UP:
    if (CheckCombo(WM_KEYDOWN, key) || key == VK_F4)
      break;

    /* fall through */
  case VK_LEFT:
    if ((m_style & ES_MULTILINE) && (key == VK_UP))
      MoveUp_ML(shift);
    else
      if (control)
        MoveWordBackward(shift);
      else
        MoveBackward(shift);
    break;
  case VK_DOWN:
    if (CheckCombo(WM_KEYDOWN, key))
      break;
    /* fall through */
  case VK_RIGHT:
    if ((m_style & ES_MULTILINE) && (key == VK_DOWN))
      MoveDown_ML(shift);
    else if (control)
      MoveWordForward(shift);
    else
      MoveForward(shift);
    break;
  case VK_HOME:
    MoveHome(shift, control);
    break;
  case VK_END:
    MoveEnd(shift, control);
    break;
  case VK_PRIOR:
    if (m_style & ES_MULTILINE)
      MovePageUp_ML(shift);
    else
      CheckCombo(WM_KEYDOWN, key);
    break;
  case VK_NEXT:
    if (m_style & ES_MULTILINE)
      MovePageDown_ML(shift);
    else
      CheckCombo(WM_KEYDOWN, key);
    break;
  case VK_DELETE:
    if (!(m_style & ES_READONLY) && !(shift && control)) {
      if (m_selection_start != m_selection_end) {
        if (shift)
          WM_Cut();
        else
          WM_Clear();
      } else {
        if (shift) {
          /* delete character left of caret */
          EM_SetSel((GXUINT)-1, 0, FALSE);
          MoveBackward(TRUE);
          WM_Clear();
        } else if (control) {
          /* delete to end of line */
          EM_SetSel((GXUINT)-1, 0, FALSE);
          MoveEnd(TRUE, FALSE);
          WM_Clear();
        } else {
          /* delete character right of caret */
          EM_SetSel((GXUINT)-1, 0, FALSE);
          MoveForward(TRUE);
          WM_Clear();
        }
      }
    }
    break;
  case VK_INSERT:
    if (shift) {
      if (!(m_style & ES_READONLY))
        WM_Paste();
    } else if (control)
      WM_Copy();
    break;
  case VK_RETURN:
    /* If the edit doesn't want the return send a message to the default object */
    if(!(m_style & ES_MULTILINE) || !(m_style & ES_WANTRETURN))
    {
      GXDWORD dw;

      if ( ! IsInsideDialog()) break;
      if (control) break;
      dw = gxSendMessageW(m_hwndParent, DM_GETDEFID, 0, 0);
      if (HIWORD(dw) == DC_HASDEFID)
      {
        GXHWND hwDefCtrl = gxGetDlgItem(m_hwndParent, LOWORD(dw));
        if (hwDefCtrl)
        {
          gxSendMessageW(m_hwndParent, WM_NEXTDLGCTL, (GXWPARAM)hwDefCtrl, TRUE);
          gxPostMessageW(hwDefCtrl, WM_KEYDOWN, VK_RETURN, 0);
        }
      }
    }
    break;
  case VK_ESCAPE:
    if ((m_style & ES_MULTILINE) && IsInsideDialog())
      gxPostMessageW(m_hwndParent, WM_CLOSE, 0, 0);
    break;
  case VK_TAB:
    if ((m_style & ES_MULTILINE) && IsInsideDialog())
      gxSendMessageW(m_hwndParent, WM_NEXTDLGCTL, shift, 0);
    break;
  }
  return TRUE;
}


/*********************************************************************
*
*  WM_KILLFOCUS
*
*/
GXLRESULT EDITSTATE::WM_KillFocus()
{
  m_flags &= ~EF_FOCUSED;
  DestroyCaret();
  if(!(m_style & ES_NOHIDESEL))
    InvalidateText(m_selection_start, m_selection_end);
  _NOTIFY_PARENT(EN_KILLFOCUS);
  /* throw away left over scroll when we lose focus */
  m_wheelDeltaRemainder = 0;
  return 0;
}


/*********************************************************************
*
*  WM_LBUTTONDBLCLK
*
*  The caret position has been set on the WM_LBUTTONDOWN message
*
*/
GXLRESULT EDITSTATE::WM_LButtonDblClk()
{
  INT s;
  INT e = m_selection_end;
  INT l;
  INT li;
  INT ll;

  m_bCaptureState = TRUE;
  gxSetCapture(m_hwndSelf);

  l = EM_LineFromChar(e);
  li = EM_LineIndex(l);
  ll = EM_LineLength(e);
  s = li + CallWordBreakProc(li, e - li, ll, WB_LEFT);
  e = li + CallWordBreakProc(li, e - li, ll, WB_RIGHT);
  EM_SetSel(s, e, FALSE);
  EM_ScrollCaret();
  m_region_posx = m_region_posy = 0;
  gxSetTimer(m_hwndSelf, 0, 100, NULL);
  return 0;
}


/*********************************************************************
*
*  WM_LBUTTONDOWN
*
*/
GXLRESULT EDITSTATE::WM_LButtonDown(GXDWORD keys, INT x, INT y)
{
  INT e;
  GXBOOL after_wrap;

  m_bCaptureState = TRUE;
  gxSetCapture(m_hwndSelf);
  ConfinePoint(&x, &y);
  e = CharFromPos(x, y, &after_wrap);
  EM_SetSel((keys & MK_SHIFT) ? m_selection_start : e, e, after_wrap);
  EM_ScrollCaret();
  m_region_posx = m_region_posy = 0;
  gxSetTimer(m_hwndSelf, 0, 100, NULL);

  if (!(m_flags & EF_FOCUSED))
    gxSetFocus(m_hwndSelf);

  return 0;
}


/*********************************************************************
*
*  WM_LBUTTONUP
*
*/
GXLRESULT EDITSTATE::WM_LButtonUp()
{
  if (m_bCaptureState) {
    gxKillTimer(m_hwndSelf, 0);
    if (gxGetCapture() == m_hwndSelf) {
      gxReleaseCapture();
    }
  }
  m_bCaptureState = FALSE;
  return 0;
}


/*********************************************************************
*
*  WM_MBUTTONDOWN
*
*/
GXLRESULT EDITSTATE::WM_MButtonDown()
{
  gxSendMessageW(m_hwndSelf, WM_PASTE, 0, 0);
  return 0;
}


/*********************************************************************
*
*  WM_MOUSEMOVE
*
*/
GXLRESULT EDITSTATE::WM_MouseMove(INT x, INT y)
{
  INT e;
  GXBOOL after_wrap;
  INT prex, prey;

  /* If the mouse has been captured by process other than the edit control itself,
  * the windows edit controls will not select the strings with mouse move.
  */
  if (!m_bCaptureState || gxGetCapture() != m_hwndSelf)
    return 0;

  /*
  *  FIXME: gotta do some scrolling if outside client
  *    area.  Maybe reset the timer ?
  */
  prex = x; prey = y;
  ConfinePoint(&x, &y);
  m_region_posx = (prex < x) ? -1 : ((prex > x) ? 1 : 0);
  m_region_posy = (prey < y) ? -1 : ((prey > y) ? 1 : 0);
  e = CharFromPos(x, y, &after_wrap);
  EM_SetSel(m_selection_start, e, after_wrap);
  SetCaretPos(m_selection_end, m_flags & EF_AFTER_WRAP);
  return 0;
}


/*********************************************************************
*
*  WM_PAINT
*
*/
void EDITSTATE::WM_Paint(GXHDC hdc)
{
  GXPAINTSTRUCT ps;
  INT i;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXRECT rc;
  GXRECT rcClient;
  GXRECT rcLine;
  GXRECT rcRgn;
  GXHBRUSH brush;
  GXHBRUSH old_brush;
  INT bw, bh;
  GXBOOL rev = m_bEnableState &&
    ((m_flags & EF_FOCUSED) ||
    (m_style & ES_NOHIDESEL));
  dc = hdc ? hdc : gxBeginPaint(m_hwndSelf, &ps);

  /* The dc we use for calculating may not be the one we paint into.
  This is the safest action. */
  InvalidateUniscribeData();
  gxGetClientRect(m_hwndSelf, &rcClient);

  /* get the background brush */
  brush = NotifyCtlColor(dc);

  /* paint the border and the background */
  gxIntersectClipRect(dc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

  if(m_style & WS_BORDER) {
    bw = GetSystemMetrics(SM_CXBORDER);
    bh = GetSystemMetrics(SM_CYBORDER);
    rc = rcClient;
    if(m_style & ES_MULTILINE) {
      if(m_style & WS_HSCROLL) rc.bottom+=bh;
      if(m_style & WS_VSCROLL) rc.right+=bw;
    }

    /* Draw the frame. Same code as in nonclient.c */
    old_brush = (GXHBRUSH)gxSelectObject(dc, gxGetSysColorBrush(COLOR_WINDOWFRAME));
    gxPatBlt(dc, rc.left, rc.top, rc.right - rc.left, bh, PATCOPY);
    gxPatBlt(dc, rc.left, rc.top, bw, rc.bottom - rc.top, PATCOPY);
    gxPatBlt(dc, rc.left, rc.bottom - 1, rc.right - rc.left, -bw, PATCOPY);
    gxPatBlt(dc, rc.right - 1, rc.top, -bw, rc.bottom - rc.top, PATCOPY);
    gxSelectObject(dc, old_brush);

    /* Keep the border clean */
    gxIntersectClipRect(dc, rc.left+bw, rc.top+bh,
      max(rc.right-bw, rc.left+bw), max(rc.bottom-bh, rc.top+bh));
  }

  gxGetClipBox(dc, &rc);
  gxFillRect(dc, &rc, brush);

  gxIntersectClipRect(dc, m_format_rect.left,
    m_format_rect.top,
    m_format_rect.right,
    m_format_rect.bottom);
  if (m_style & ES_MULTILINE) {
    rc = rcClient;
    gxIntersectClipRect(dc, rc.left, rc.top, rc.right, rc.bottom);
  }
  if (m_font)
    old_font = (GXHFONT)gxSelectObject(dc, m_font);

  if (!m_bEnableState)
    gxSetTextColor(dc, GetSysColor(COLOR_GRAYTEXT));
  gxGetClipBox(dc, &rcRgn);
  if (m_style & ES_MULTILINE) {
    INT vlc = get_vertical_line_count();
    for (i = m_y_offset ; i <= min(m_y_offset + vlc, m_y_offset + m_line_count - 1) ; i++) {
      UpdateUniscribeData(dc, i);
      GetLineRect(i, 0, -1, &rcLine);
      if (gxIntersectRect(&rc, &rcRgn, &rcLine))
        PaintLine(dc, i, rev);
    }
  } else {
    UpdateUniscribeData(dc, 0);
    GetLineRect(0, 0, -1, &rcLine);
    if (gxIntersectRect(&rc, &rcRgn, &rcLine))
      PaintLine(dc, 0, rev);
  }
  if (m_font)
    gxSelectObject(dc, old_font);

  if (!hdc)
    gxEndPaint(m_hwndSelf, &ps);
}


/*********************************************************************
*
*  WM_SETFOCUS
*
*/
void EDITSTATE::WM_SetFocus()
{
  m_flags |= EF_FOCUSED;

  if (!(m_style & ES_NOHIDESEL))
    InvalidateText(m_selection_start, m_selection_end);

  /* single line edit updates itself */
  if (gxIsWindowVisible(m_hwndSelf) && !(m_style & ES_MULTILINE))
  {
    GXHDC hdc = gxGetDC(m_hwndSelf);
    WM_Paint(hdc);
    gxReleaseDC(m_hwndSelf, hdc);
  }

  gxCreateCaret(m_hwndSelf, 0, 1, m_line_height);
  SetCaretPos(m_selection_end, m_flags & EF_AFTER_WRAP);
  gxShowCaret(m_hwndSelf);
  _NOTIFY_PARENT(EN_SETFOCUS);
}


/*********************************************************************
*
*  WM_SETFONT
*
* With Win95 look the margins are set to default font value unless
* the system font (font == 0) is being set, in which case they are left
* unchanged.
*
*/
void EDITSTATE::WM_SetFont(GXHFONT font, GXBOOL redraw)
{
  GXTEXTMETRICW tm;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXRECT clientRect;

  m_font = font;
  InvalidateUniscribeData();
  dc = gxGetDC(m_hwndSelf);
  if (font) {
    old_font = (GXHFONT)gxSelectObject(dc, font);
  }
  gxGetTextMetricsW(dc, &tm);
  m_line_height = tm.tmHeight;
  m_char_width = tm.tmAveCharWidth;
  if (font) {
    gxSelectObject(dc, old_font);
  }
  gxReleaseDC(m_hwndSelf, dc);

  /* Reset the format rect and the margins */
  gxGetClientRect(m_hwndSelf, &clientRect);
  SetRectNP(&clientRect);
  EM_SetMargins(EC_LEFTMARGIN | EC_RIGHTMARGIN, EC_USEFONTINFO, EC_USEFONTINFO, FALSE);

  if (m_style & ES_MULTILINE) {
    BuildLineDefs_ML(0, get_text_length(), 0, NULL);
  }
  else {
    CalcLineWidth_SL();
  }

  if (redraw) {
    UpdateText(NULL, TRUE);
  }
  if (m_flags & EF_FOCUSED) {
    gxDestroyCaret();
    gxCreateCaret(m_hwndSelf, 0, 1, m_line_height);
    SetCaretPos(m_selection_end, m_flags & EF_AFTER_WRAP);
    gxShowCaret(m_hwndSelf);
  }
}


/*********************************************************************
*
*  WM_SETTEXT
*
* NOTES
*  For multiline controls (ES_MULTILINE), reception of WM_SETTEXT triggers:
*  The modified flag is reset. No notifications are sent.
*
*  For single-line controls, reception of WM_SETTEXT triggers:
*  The modified flag is reset. EN_UPDATE and EN_CHANGE notifications are sent.
*
*/
void EDITSTATE::WM_SetText(LPCWSTR text, GXBOOL unicode)
{
  LPWSTR textW = NULL;
  if (!unicode && text)
  {
    LPCSTR textA = (LPCSTR)text;
    INT countW = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
    textW = new GXWCHAR[countW]; // HeapAlloc(GetProcessHeap(), 0, countW * sizeof(WCHAR));
    if (textW) {
      gxMultiByteToWideChar(CP_ACP, 0, textA, -1, textW, countW);
    }
    text = textW;
  }

  if (m_flags & EF_UPDATE) {
    /* fixed this bug once; complain if we see it about to happen again. */
    ERR("SetSel may generate UPDATE message whose handler may reset selection.\n");
  }

  EM_SetSel(0, (GXUINT)-1, FALSE);
  if (text) 
  {
    TRACE("%s\n", debugstr_w(text));
    EM_ReplaceSel(FALSE, text, FALSE, FALSE);
    if(!unicode)
      HeapFree(GetProcessHeap(), 0, textW);
  } 
  else 
  {
    TRACE("<NULL>\n");
    EM_ReplaceSel(FALSE, empty_stringW, FALSE, FALSE);
  }
  m_x_offset = 0;
  m_flags &= ~EF_MODIFIED;
  EM_SetSel(0, 0, FALSE);

  /* Send the notification after the selection start and end have been set
  * edit control doesn't send notification on WM_SETTEXT
  * if it is multiline, or it is part of combobox
  */
  if( !((m_style & ES_MULTILINE) || m_hwndListBox))
  {
    _NOTIFY_PARENT(EN_UPDATE);
    _NOTIFY_PARENT(EN_CHANGE);
  }
  EM_ScrollCaret();
  UpdateScrollInfo();    
  InvalidateUniscribeData();
}


/*********************************************************************
*
*  WM_SIZE
*
*/
void EDITSTATE::WM_Size(GXUINT action)
{
  if ((action == SIZE_MAXIMIZED) || (action == SIZE_RESTORED)) {
    GXRECT rc;
    gxGetClientRect(m_hwndSelf, &rc);
    SetRectNP(&rc);
    UpdateText(NULL, TRUE);
  }
}


/*********************************************************************
*
*  WM_STYLECHANGED
*
* This message is sent by SetWindowLong on having changed either the Style
* or the extended style.
*
* We ensure that the window's version of the styles and the EDITSTATE's agree.
*
* See also _WM_NCCreate
*
* It appears that the Windows version of the edit control allows the style
* (as retrieved by GetWindowLong) to be any value and maintains an internal
* style variable which will generally be different.  In this function we
* update the internal style based on what changed in the externally visible
* style.
*
* Much of this content as based upon the MSDN, especially:
*  Platform SDK Documentation -> User Interface Services ->
*      Windows User Interface -> Edit Controls -> Edit Control Reference ->
*      Edit Control Styles
*/
GXLRESULT EDITSTATE::WM_StyleChanged(GXWPARAM which, const STYLESTRUCT *style)
{
  if (GWL_STYLE == which) {
    GXDWORD style_change_mask;
    GXDWORD new_style;
    /* Only a subset of changes can be applied after the control
    * has been created.
    */
    style_change_mask = ES_UPPERCASE | ES_LOWERCASE |
      ES_NUMBER;
    if (m_style & ES_MULTILINE)
      style_change_mask |= ES_WANTRETURN;

    new_style = style->styleNew & style_change_mask;

    /* Number overrides lowercase overrides uppercase (at least it
    * does in Win95).  However I'll bet that ES_NUMBER would be
    * invalid under Win 3.1.
    */
    if (new_style & ES_NUMBER) {
      ; /* do not override the ES_NUMBER */
    }  else if (new_style & ES_LOWERCASE) {
      new_style &= ~ES_UPPERCASE;
    }

    m_style = (m_style & ~style_change_mask) | new_style;
  } else if (GWL_EXSTYLE == which) {
    ; /* FIXME - what is needed here */
  } else {
    WARN ("Invalid style change %ld\n",which);
  }

  return 0;
}

/*********************************************************************
*
*  WM_SYSKEYDOWN
*
*/
GXLRESULT EDITSTATE::WM_SysKeyDown(INT key, GXDWORD key_data)
{
  if ((key == VK_BACK) && (key_data & 0x2000)) {
    if (EM_CanUndo())
      EM_Undo();
    return 0;
  } else if (key == VK_UP || key == VK_DOWN) {
    if (CheckCombo(WM_SYSKEYDOWN, key))
      return 0;
  }
  return gxDefWindowProcW(m_hwndSelf, WM_SYSKEYDOWN, key, key_data);
}


/*********************************************************************
*
*  WM_TIMER
*
*/
void EDITSTATE::WM_Timer()
{
  if (m_region_posx < 0) {
    MoveBackward(TRUE);
  } else if (m_region_posx > 0) {
    MoveForward(TRUE);
  }
  /*
  *  FIXME: gotta do some vertical scrolling here, like
  *    _EM_LineScroll(hwnd, 0, 1);
  */
}

/*********************************************************************
*
*  WM_HSCROLL
*
*/
GXLRESULT EDITSTATE::WM_HScroll(INT action, INT pos)
{
  INT dx;
  INT fw;

  if (!(m_style & ES_MULTILINE))
    return 0;

  if (!(m_style & ES_AUTOHSCROLL))
    return 0;

  dx = 0;
  fw = m_format_rect.right - m_format_rect.left;
  switch (action) {
  case SB_LINELEFT:
    TRACE("SB_LINELEFT\n");
    if (m_x_offset)
      dx = -m_char_width;
    break;
  case SB_LINERIGHT:
    TRACE("SB_LINERIGHT\n");
    if (m_x_offset < m_text_width)
      dx = m_char_width;
    break;
  case SB_PAGELEFT:
    TRACE("SB_PAGELEFT\n");
    if (m_x_offset)
      dx = -fw / HSCROLL_FRACTION / m_char_width * m_char_width;
    break;
  case SB_PAGERIGHT:
    TRACE("SB_PAGERIGHT\n");
    if (m_x_offset < m_text_width)
      dx = fw / HSCROLL_FRACTION / m_char_width * m_char_width;
    break;
  case SB_LEFT:
    TRACE("SB_LEFT\n");
    if (m_x_offset)
      dx = -m_x_offset;
    break;
  case SB_RIGHT:
    TRACE("SB_RIGHT\n");
    if (m_x_offset < m_text_width)
      dx = m_text_width - m_x_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    m_flags |= EF_HSCROLL_TRACK;
    if(m_style & WS_HSCROLL)
      dx = pos - m_x_offset;
    else
    {
      INT fw, new_x;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      /* Assume default scroll range 0-100 */
      fw = m_format_rect.right - m_format_rect.left;
      new_x = pos * (m_text_width - fw) / 100;
      dx = m_text_width ? (new_x - m_x_offset) : 0;
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    m_flags &= ~EF_HSCROLL_TRACK;
    if(gxGetWindowLongW( m_hwndSelf, GWL_STYLE ) & WS_HSCROLL)
      dx = pos - m_x_offset;
    else
    {
      INT fw, new_x;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      /* Assume default scroll range 0-100 */
      fw = m_format_rect.right - m_format_rect.left;
      new_x = pos * (m_text_width - fw) / 100;
      dx = m_text_width ? (new_x - m_x_offset) : 0;
    }
    if (!dx) {
      /* force scroll info update */
      UpdateScrollInfo();
      _NOTIFY_PARENT(EN_HSCROLL);
    }
    break;
  case SB_ENDSCROLL:
    TRACE("SB_ENDSCROLL\n");
    break;
    /*
    *  FIXME : the next two are undocumented !
    *  Are we doing the right thing ?
    *  At least Win 3.1 Notepad makes use of EM_GETTHUMB this way,
    *  although it's also a regular control message.
    */
  case EM_GETTHUMB: /* this one is used by NT notepad */
    {
      GXLRESULT ret;
      if(gxGetWindowLongW( m_hwndSelf, GWL_STYLE ) & WS_HSCROLL)
        ret = gxGetScrollPos(m_hwndSelf, SB_HORZ);
      else
      {
        /* Assume default scroll range 0-100 */
        INT fw = m_format_rect.right - m_format_rect.left;
        ret = m_text_width ? m_x_offset * 100 / (m_text_width - fw) : 0;
      }
      TRACE("EM_GETTHUMB: returning %ld\n", ret);
      return ret;
    }
  case EM_LINESCROLL:
    TRACE("EM_LINESCROLL16\n");
    dx = pos;
    break;

  default:
    ERR("undocumented WM_HSCROLL action %d (0x%04x), please report\n",
      action, action);
    return 0;
  }
  if (dx)
  {
    INT fw = m_format_rect.right - m_format_rect.left;
    /* check if we are going to move too far */
    if(m_x_offset + dx + fw > m_text_width)
      dx = m_text_width - fw - m_x_offset;
    if(dx)
      EM_LineScroll_internal(dx, 0);
  }
  return 0;
}


/*********************************************************************
*
*  WM_VSCROLL
*
*/
GXLRESULT EDITSTATE::WM_VScroll(INT action, INT pos)
{
  INT dy;

  if (!(m_style & ES_MULTILINE))
    return 0;

  if (!(m_style & ES_AUTOVSCROLL))
    return 0;

  dy = 0;
  switch (action) {
  case SB_LINEUP:
  case SB_LINEDOWN:
  case SB_PAGEUP:
  case SB_PAGEDOWN:
    TRACE("action %d (%s)\n", action, (action == SB_LINEUP ? "SB_LINEUP" :
      (action == SB_LINEDOWN ? "SB_LINEDOWN" :
      (action == SB_PAGEUP ? "SB_PAGEUP" :
      "SB_PAGEDOWN"))));
    EM_Scroll(action);
    return 0;
  case SB_TOP:
    TRACE("SB_TOP\n");
    dy = -m_y_offset;
    break;
  case SB_BOTTOM:
    TRACE("SB_BOTTOM\n");
    dy = m_line_count - 1 - m_y_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    m_flags |= EF_VSCROLL_TRACK;
    if(m_style & WS_VSCROLL)
      dy = pos - m_y_offset;
    else
    {
      /* Assume default scroll range 0-100 */
      INT vlc, new_y;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      vlc = get_vertical_line_count();
      new_y = pos * (m_line_count - vlc) / 100;
      dy = m_line_count ? (new_y - m_y_offset) : 0;
      TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
        m_line_count, m_y_offset, pos, dy);
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    m_flags &= ~EF_VSCROLL_TRACK;
    if(m_style & WS_VSCROLL)
      dy = pos - m_y_offset;
    else
    {
      /* Assume default scroll range 0-100 */
      INT vlc, new_y;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      vlc = get_vertical_line_count();
      new_y = pos * (m_line_count - vlc) / 100;
      dy = m_line_count ? (new_y - m_y_offset) : 0;
      TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
        m_line_count, m_y_offset, pos, dy);
    }
    if (!dy)
    {
      /* force scroll info update */
      UpdateScrollInfo();
      _NOTIFY_PARENT(EN_VSCROLL);
    }
    break;
  case SB_ENDSCROLL:
    TRACE("SB_ENDSCROLL\n");
    break;
    /*
    *  FIXME : the next two are undocumented !
    *  Are we doing the right thing ?
    *  At least Win 3.1 Notepad makes use of EM_GETTHUMB this way,
    *  although it's also a regular control message.
    */
  case EM_GETTHUMB: /* this one is used by NT notepad */
    {
      GXLRESULT ret;
      if(gxGetWindowLongW( m_hwndSelf, GWL_STYLE ) & WS_VSCROLL)
        ret = gxGetScrollPos(m_hwndSelf, SB_VERT);
      else
      {
        /* Assume default scroll range 0-100 */
        INT vlc = get_vertical_line_count();
        ret = m_line_count ? m_y_offset * 100 / (m_line_count - vlc) : 0;
      }
      TRACE("EM_GETTHUMB: returning %ld\n", ret);
      return ret;
    }
  case EM_LINESCROLL:
    TRACE("EM_LINESCROLL %d\n", pos);
    dy = pos;
    break;

  default:
    ERR("undocumented WM_VSCROLL action %d (0x%04x), please report\n",
      action, action);
    return 0;
  }
  if (dy)
    EM_LineScroll(0, dy);
  return 0;
}

/*********************************************************************
*
*  EM_GETTHUMB
*
*  FIXME: is this right ?  (or should it be only VSCROLL)
*  (and maybe only for edit controls that really have their
*  own scrollbars) (and maybe only for multiline controls ?)
*  All in all: very poorly documented
*
*/
GXLRESULT EDITSTATE::EM_GetThumb()
{
  return MAKELONG(WM_VScroll(EM_GETTHUMB, 0), WM_HScroll(EM_GETTHUMB, 0));
}


/********************************************************************
* 
* The Following code is to handle inline editing from IMEs
*/

void EDITSTATE::GetCompositionStr(GXHIMC hIMC, GXLPARAM CompFlag)
{
  GXLONG buflen;
  GXLPWSTR lpCompStr;
  GXLPSTR lpCompStrAttr = NULL;
  GXDWORD dwBufLenAttr;

  buflen = gxImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);

  if (buflen < 0)
  {
    return;
  }

  lpCompStr = new GXWCHAR[buflen / sizeof(WCHAR) + 1]; // HeapAlloc(GetProcessHeap(),0,buflen + sizeof(WCHAR));
  if (!lpCompStr)
  {
    ERR("Unable to allocate IME CompositionString\n");
    return;
  }

  if (buflen) {
    gxImmGetCompositionStringW(hIMC, GCS_COMPSTR, lpCompStr, buflen);
  }
  lpCompStr[buflen / sizeof(GXWCHAR)] = 0;

  if (CompFlag & GCS_COMPATTR)
  {
    /* 
    * We do not use the attributes yet. it would tell us what characters
    * are in transition and which are converted or decided upon
    */
    dwBufLenAttr = gxImmGetCompositionStringW(hIMC, GCS_COMPATTR, NULL, 0);
    if (dwBufLenAttr)
    {
      dwBufLenAttr ++;
      //lpCompStrAttr = HeapAlloc(GetProcessHeap(),0,dwBufLenAttr+1);
      lpCompStrAttr = new GXCHAR[dwBufLenAttr + 1];
      if ( ! lpCompStrAttr)
      {
        ERR("Unable to allocate IME Attribute String\n");
        //HeapFree(GetProcessHeap(),0,lpCompStr);
        SAFE_DELETE_ARRAY(lpCompStr);
        return;
      }
      gxImmGetCompositionStringW(hIMC,GCS_COMPATTR, lpCompStrAttr, dwBufLenAttr);
      lpCompStrAttr[dwBufLenAttr] = 0;
    }
  }

  /* check for change in composition start */
  if (m_selection_end < m_composition_start) {
    m_composition_start = m_selection_end;
  }

  /* replace existing selection string */
  m_selection_start = m_composition_start;

  if (m_composition_len > 0) {
    m_selection_end = m_composition_start + m_composition_len;
  }
  else {
    m_selection_end = m_selection_start;
  }

  EM_ReplaceSel(FALSE, lpCompStr, TRUE, TRUE);
  m_composition_len = abs(m_composition_start - m_selection_end);

  m_selection_start = m_composition_start;
  m_selection_end = m_selection_start + m_composition_len;

  //HeapFree(GetProcessHeap(), 0, lpCompStrAttr);
  //HeapFree(GetProcessHeap(), 0, lpCompStr);
  SAFE_DELETE_ARRAY(lpCompStrAttr);
  SAFE_DELETE_ARRAY(lpCompStr);
}

void EDITSTATE::GetResultStr(GXHIMC hIMC)
{
  GXLONG buflen;
  GXLPWSTR lpResultStr;

  buflen = gxImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);
  if (buflen <= 0)
  {
    return;
  }

  //lpResultStr = HeapAlloc(GetProcessHeap(),0, buflen+sizeof(WCHAR));
  lpResultStr = new GXWCHAR[buflen / sizeof(WCHAR) + 1];
  if ( ! lpResultStr)
  {
    ERR("Unable to alloc buffer for IME string\n");
    return;
  }

  gxImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpResultStr, buflen);
  lpResultStr[buflen / sizeof(WCHAR)] = 0;

  /* check for change in composition start */
  if (m_selection_end < m_composition_start)
    m_composition_start = m_selection_end;

  m_selection_start = m_composition_start;
  m_selection_end = m_composition_start + m_composition_len;
  EM_ReplaceSel(TRUE, lpResultStr, TRUE, TRUE);
  m_composition_start = m_selection_end;
  m_composition_len = 0;

  //HeapFree(GetProcessHeap(),0,lpResultStr);
  SAFE_DELETE_ARRAY(lpResultStr);
}

void EDITSTATE::ImeComposition(GXHWND hwnd, GXLPARAM CompFlag)
{
  GXHIMC hIMC;
  int cursor;

  if (m_composition_len == 0 && m_selection_start != m_selection_end)
  {
    EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
    m_composition_start = m_selection_end;
  }

  hIMC = gxImmGetContext(hwnd);
  if (!hIMC)
    return;

  if (CompFlag & GCS_RESULTSTR)
  {
    GetResultStr(hIMC);
    cursor = 0;
  }
  else
  {
    if (CompFlag & GCS_COMPSTR)
      GetCompositionStr(hIMC, CompFlag);
    cursor = gxImmGetCompositionStringW(hIMC, GCS_CURSORPOS, 0, 0);
  }
  gxImmReleaseContext(hwnd, hIMC);
  SetCaretPos(m_selection_start + cursor, m_flags & EF_AFTER_WRAP);
}


/*********************************************************************
*
*  WM_NCCREATE
*
* See also _WM_StyleChanged
*/
GXLRESULT EDITSTATE::WM_NCCreate(GXHWND hwnd, GXLPCREATESTRUCTW lpcs, GXBOOL unicode)
{
  EDITSTATE *es;
  GXUINT alloc_size;

  TRACE("Creating %s edit control, style = %08x\n",
    unicode ? "Unicode" : "ANSI", lpcs->style);

  //if (!(es = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*es))))
  //  return FALSE;
  es = new EDITSTATE;
  if ( ! es) {
    return FALSE;
  }
  gxSetWindowLongPtrW( hwnd, 0, (GXLONG_PTR)es );

  /*
  *      Note: since the EDITSTATE has not been fully initialized yet,
  *            we can't use any API calls that may send
  *            WM_XXX messages before WM_NCCREATE is completed.
  */

  m_is_unicode = unicode;
  m_style = lpcs->style;

  m_bEnableState = !(m_style & WS_DISABLED);

  m_hwndSelf = hwnd;
  /* Save parent, which will be notified by EN_* messages */
  m_hwndParent = lpcs->hwndParent;

  if (m_style & GXES_COMBO)
    m_hwndListBox = gxGetDlgItem(m_hwndParent, ID_CB_LISTBOX);

  /* FIXME: should we handle changes to WS_EX_RIGHT style after creation? */
  if (lpcs->dwExStyle & WS_EX_RIGHT) m_style |= ES_RIGHT;

  /* Number overrides lowercase overrides uppercase (at least it
  * does in Win95).  However I'll bet that ES_NUMBER would be
  * invalid under Win 3.1.
  */
  if (m_style & ES_NUMBER) {
    ; /* do not override the ES_NUMBER */
  }  else if (m_style & ES_LOWERCASE) {
    m_style &= ~ES_UPPERCASE;
  }
  if (m_style & ES_MULTILINE) {
    m_buffer_limit = BUFLIMIT_INITIAL;
    if (m_style & WS_VSCROLL)
      m_style |= ES_AUTOVSCROLL;
    if (m_style & WS_HSCROLL)
      m_style |= ES_AUTOHSCROLL;
    m_style &= ~ES_PASSWORD;
    if ((m_style & ES_CENTER) || (m_style & ES_RIGHT)) {
      /* Confirmed - RIGHT overrides CENTER */
      if (m_style & ES_RIGHT)
        m_style &= ~ES_CENTER;
      m_style &= ~WS_HSCROLL;
      m_style &= ~ES_AUTOHSCROLL;
    }
  } else {
    m_buffer_limit = BUFLIMIT_INITIAL;
    if ((m_style & ES_RIGHT) && (m_style & ES_CENTER))
      m_style &= ~ES_CENTER;
    m_style &= ~WS_HSCROLL;
    m_style &= ~WS_VSCROLL;
    if (m_style & ES_PASSWORD)
      m_password_char = '*';
  }

  alloc_size = ROUND_TO_GROW((m_buffer_size + 1) * sizeof(WCHAR));
  if(!(m_hloc32W = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
    goto cleanup;
  m_buffer_size = gxLocalSize(m_hloc32W)/sizeof(WCHAR) - 1;

  //if (!(m_undo_text = gxHeapAlloc(gxGetProcessHeap(), HEAP_ZERO_MEMORY, (m_buffer_size + 1) * sizeof(WCHAR))))
  m_undo_text = new WCHAR[m_buffer_size + 1];
  memset(m_undo_text, 0, (m_buffer_size + 1) * sizeof(WCHAR));
  if ( ! m_undo_text) {
    goto cleanup;
  }
  m_undo_buffer_size = m_buffer_size;

  if (m_style & ES_MULTILINE) {
    //m_first_line_def = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LINEDEF));
    m_first_line_def = new LINEDEF;
    memset(m_first_line_def, 0, sizeof(LINEDEF));
    if ( ! m_first_line_def) {
      goto cleanup;
    }
  }
  m_line_count = 1;

  /*
  * In Win95 look and feel, the WS_BORDER style is replaced by the
  * WS_EX_CLIENTEDGE style for the edit control. This gives the edit
  * control a nonclient area so we don't need to draw the border.
  * If WS_BORDER without WS_EX_CLIENTEDGE is specified we shouldn't have
  * a nonclient area and we should handle painting the border ourselves.
  *
  * When making modifications please ensure that the code still works
  * for edit controls created directly with style 0x50800000, exStyle 0
  * (which should have a single pixel border)
  */
  if (lpcs->dwExStyle & WS_EX_CLIENTEDGE)
    m_style &= ~WS_BORDER;
  else if (m_style & WS_BORDER)
    gxSetWindowLongW(hwnd, GWL_STYLE, m_style & ~WS_BORDER);

  return TRUE;

cleanup:
  gxSetWindowLongPtrW(m_hwndSelf, 0, 0);
  InvalidateUniscribeData();
  SAFE_DELETE(m_first_line_def);
  SAFE_DELETE_ARRAY(m_undo_text);
  //HeapFree(GetProcessHeap(), 0, m_first_line_def);
  //HeapFree(GetProcessHeap(), 0, m_undo_text);
  if (m_hloc32W) {
    gxLocalFree(m_hloc32W);
  }
  SAFE_DELETE_ARRAY(m_logAttr);
  SAFE_DELETE(es);
  //HeapFree(GetProcessHeap(), 0, m_logAttr);
  //HeapFree(GetProcessHeap(), 0, es);
  return FALSE;
}


/*********************************************************************
*
*  WM_CREATE
*
*/
GXLRESULT EDITSTATE::WM_Create(LPCWSTR name)
{
  GXRECT clientRect;

  TRACE("%s\n", debugstr_w(name));
  /*
  *  To initialize some final structure members, we call some helper
  *  functions.  However, since the EDITSTATE is not consistent (i.e.
  *  not fully initialized), we should be very careful which
  *  functions can be called, and in what order.
  */
  WM_SetFont(0, FALSE);
  EM_EmptyUndoBuffer();

  /* We need to calculate the format rect
  (applications may send EM_SETMARGINS before the control gets visible) */
  gxGetClientRect(m_hwndSelf, &clientRect);
  SetRectNP(&clientRect);

  if (name && *name) {
    EM_ReplaceSel(FALSE, name, FALSE, FALSE);
    /* if we insert text to the editline, the text scrolls out
    * of the window, as the caret is placed after the insert
    * pos normally; thus we reset m_selection... to 0 and
    * update caret
    */
    m_selection_start = m_selection_end = 0;
    /* Adobe Photoshop does NOT like this. and MSDN says that EN_CHANGE
    * Messages are only to be sent when the USER does something to
    * change the contents. So I am removing this EN_CHANGE
    *
    * _NOTIFY_PARENT(EN_CHANGE);
    */
    EM_ScrollCaret();
  }
  /* force scroll info update */
  UpdateScrollInfo();
  /* The rule seems to return 1 here for success */
  /* Power Builder masked edit controls will crash  */
  /* if not. */
  /* FIXME: is that in all cases so ? */
  return 1;
}


/*********************************************************************
*
*  WM_NCDESTROY
*
*/
GXLRESULT EDITSTATE::WM_NCDestroy()
{
  LINEDEF *pc, *pp;

  /* The app can own the text buffer handle */
  if (m_hloc32W && (m_hloc32W != m_hlocapp)) {
    LocalFree(m_hloc32W);
  }
  if (m_hloc32A && (m_hloc32A != m_hlocapp)) {
    LocalFree(m_hloc32A);
  }
  InvalidateUniscribeData();
  pc = m_first_line_def;
  while (pc)
  {
    pp = pc->next;
    HeapFree(GetProcessHeap(), 0, pc);
    pc = pp;
  }

  gxSetWindowLongPtrW( m_hwndSelf, 0, 0 );
  //HeapFree(GetProcessHeap(), 0, m_undo_text);
  //HeapFree(GetProcessHeap(), 0, es);
  SAFE_DELETE_ARRAY(m_undo_text);
  //SAFE_DELETE(es);

  return 0;
}


static inline GXLRESULT DefWindowProcT(GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam, GXBOOL unicode)
{
  if(unicode)
    return gxDefWindowProcW(hwnd, msg, wParam, lParam);
  else
    return gxDefWindowProcA(hwnd, msg, wParam, lParam);
}

/*********************************************************************
*
*  EditWndProc_common
*
*  The messages are in the order of the actual integer values
*  (which can be found in include/windows.h)
*/
GXLRESULT EditWndProc_common( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam, GXBOOL unicode )
{
  EDITSTATE* es = (EDITSTATE*)gxGetWindowLongPtrW( hwnd, 0 );
  GXLRESULT result = 0;

  TRACE("hwnd=%p msg=%x (%s) wparam=%lx lparam=%lx\n", hwnd, msg, SPY_GetMsgName(msg, hwnd), wParam, lParam);

  if (!es && msg != WM_NCCREATE)
    return DefWindowProcT(hwnd, msg, wParam, lParam, unicode);

  if (es && (msg != WM_NCDESTROY)) {
    es->LockBuffer();
  }

  switch (msg) {
  case EM_GETSEL:
    result = es->EM_GetSel((PUINT)wParam, (PUINT)lParam);
    break;

  case EM_SETSEL:
    es->EM_SetSel(wParam, lParam, FALSE);
    es->EM_ScrollCaret();
    result = 1;
    break;

  case EM_GETRECT:
    if (lParam)
      gxCopyRect((GXLPRECT)lParam, &es->m_format_rect);
    break;

  case EM_SETRECT:
    if ((es->m_style & ES_MULTILINE) && lParam) {
      es->SetRectNP((GXLPRECT)lParam);
      es->UpdateText(NULL, TRUE);
    }
    break;

  case EM_SETRECTNP:
    if ((es->m_style & ES_MULTILINE) && lParam)
      es->SetRectNP((GXLPRECT)lParam);
    break;

  case EM_SCROLL:
    result = es->EM_Scroll((INT)wParam);
    break;

  case EM_LINESCROLL:
    result = (GXLRESULT)es->EM_LineScroll((INT)wParam, (INT)lParam);
    break;

  case EM_SCROLLCARET:
    es->EM_ScrollCaret();
    result = 1;
    break;

  case EM_GETMODIFY:
    result = ((es->m_flags & EF_MODIFIED) != 0);
    break;

  case EM_SETMODIFY:
    if (wParam)
      es->m_flags |= EF_MODIFIED;
    else
      es->m_flags &= ~(EF_MODIFIED | EF_UPDATE);  /* reset pending updates */
    break;

  case EM_GETLINECOUNT:
    result = (es->m_style & ES_MULTILINE) ? es->m_line_count : 1;
    break;

  case EM_LINEINDEX:
    result = (GXLRESULT)es->EM_LineIndex((INT)wParam);
    break;

  case EM_SETHANDLE:
    es->EM_SetHandle((GXHLOCAL)wParam);
    break;

  case EM_GETHANDLE:
    result = (GXLRESULT)es->EM_GetHandle();
    break;

  case EM_GETTHUMB:
    result = es->EM_GetThumb();
    break;

    /* these messages missing from specs */
  case 0x00bf:
  case 0x00c0:
  case 0x00c3:
  case 0x00ca:
    FIXME("undocumented message 0x%x, please report\n", msg);
    result = gxDefWindowProcW(hwnd, msg, wParam, lParam);
    break;

  case EM_LINELENGTH:
    result = (GXLRESULT)es->EM_LineLength((INT)wParam);
    break;

  case EM_REPLACESEL:
    {
      LPWSTR textW;

      if(unicode)
        textW = (LPWSTR)lParam;
      else
      {
        LPSTR textA = (LPSTR)lParam;
        INT countW = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
        if (!(textW = (GXWCHAR*)gxHeapAlloc(gxGetProcessHeap(), 0, countW * sizeof(WCHAR)))) break;
        gxMultiByteToWideChar(CP_ACP, 0, textA, -1, textW, countW);
      }

      es->EM_ReplaceSel((GXBOOL)wParam, textW, TRUE, TRUE);
      result = 1;

      if(!unicode)
        gxHeapFree(gxGetProcessHeap(), 0, textW);
      break;
    }

  case EM_GETLINE:
    result = (GXLRESULT)es->EM_GetLine((INT)wParam, (LPWSTR)lParam, unicode);
    break;

  case EM_SETLIMITTEXT:
    es->EM_SetLimitText(wParam);
    break;

  case EM_CANUNDO:
    result = (GXLRESULT)es->EM_CanUndo();
    break;

  case EM_UNDO:
  case WM_UNDO:
    result = (GXLRESULT)es->EM_Undo();
    break;

  case EM_FMTLINES:
    result = (GXLRESULT)es->EM_FmtLines((GXBOOL)wParam);
    break;

  case EM_LINEFROMCHAR:
    result = (GXLRESULT)es->EM_LineFromChar((INT)wParam);
    break;

  case EM_SETTABSTOPS:
    result = (GXLRESULT)es->EM_SetTabStops((INT)wParam, (LPINT)lParam);
    break;

  case EM_SETPASSWORDCHAR:
    {
      WCHAR charW = 0;

      if(unicode)
        charW = (WCHAR)wParam;
      else
      {
        CHAR charA = wParam;
        gxMultiByteToWideChar(CP_ACP, 0, &charA, 1, &charW, 1);
      }

      es->EM_SetPasswordChar(charW);
      break;
    }

  case EM_EMPTYUNDOBUFFER:
    es->EM_EmptyUndoBuffer();
    break;

  case EM_GETFIRSTVISIBLELINE:
    result = (es->m_style & ES_MULTILINE) ? es->m_y_offset : es->m_x_offset;
    break;

  case EM_SETREADONLY:
    {
      GXDWORD old_style = es->m_style;

      if (wParam) {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) | ES_READONLY );
        es->m_style |= ES_READONLY;
      } else {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) & ~ES_READONLY );
        es->m_style &= ~ES_READONLY;
      }

      if (old_style ^ es->m_style)
        gxInvalidateRect(es->m_hwndSelf, NULL, TRUE);

      result = 1;
      break;
    }

  case EM_SETWORDBREAKPROC:
    es->EM_SetWordBreakProc((void *)lParam);
    break;

  case EM_GETWORDBREAKPROC:
    result = (GXLRESULT)es->m_word_break_proc;
    break;

  case EM_GETPASSWORDCHAR:
    {
      if(unicode)
        result = es->m_password_char;
      else
      {
        WCHAR charW = es->m_password_char;
        CHAR charA = 0;
        gxWideCharToMultiByte(CP_ACP, 0, &charW, 1, &charA, 1, NULL, NULL);
        result = charA;
      }
      break;
    }

  case EM_SETMARGINS:
    es->EM_SetMargins((INT)wParam, LOWORD(lParam), HIWORD(lParam), TRUE);
    break;

  case EM_GETMARGINS:
    result = MAKELONG(es->m_left_margin, es->m_right_margin);
    break;

  case EM_GETLIMITTEXT:
    result = es->m_buffer_limit;
    break;

  case EM_POSFROMCHAR:
    if ((INT)wParam >= es->get_text_length()) result = -1;
    else result = es->EM_PosFromChar((INT)wParam, FALSE);
    break;

  case EM_CHARFROMPOS:
    result = es->EM_CharFromPos((short)LOWORD(lParam), (short)HIWORD(lParam));
    break;

    /* End of the EM_ messages which were in numerical order; what order
    * are these in?  vaguely alphabetical?
    */

  case WM_NCCREATE:
    result = es->WM_NCCreate(hwnd, (GXLPCREATESTRUCTW)lParam, unicode);
    break;

  case WM_NCDESTROY:
    result = es->WM_NCDestroy();
    es = NULL;
    break;

  case WM_GETDLGCODE:
    result = DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;

    if (es->m_style & ES_MULTILINE)
      result |= DLGC_WANTALLKEYS;

    if (lParam)
    {
      es->m_flags|=EF_DIALOGMODE;

      if (((LPMSG)lParam)->message == WM_KEYDOWN)
      {
        int vk = (int)((LPMSG)lParam)->wParam;

        if (es->m_hwndListBox)
        {
          if (vk == VK_RETURN || vk == VK_ESCAPE)
            if (gxSendMessageW(gxGetParent(hwnd), CB_GETDROPPEDSTATE, 0, 0))
              result |= DLGC_WANTMESSAGE;
        }
      }
    }
    break;

  case WM_IME_CHAR:
    if (!unicode)
    {
      WCHAR charW;
      CHAR  strng[2];

      strng[0] = wParam >> 8;
      strng[1] = wParam & 0xff;
      if (strng[0]) gxMultiByteToWideChar(CP_ACP, 0, strng, 2, &charW, 1);
      else gxMultiByteToWideChar(CP_ACP, 0, &strng[1], 1, &charW, 1);
      result = es->WM_Char(charW);
      break;
    }
    /* fall through */
  case WM_CHAR:
    {
      WCHAR charW;

      if(unicode)
        charW = wParam;
      else
      {
        CHAR charA = wParam;
        gxMultiByteToWideChar(CP_ACP, 0, &charA, 1, &charW, 1);
      }

      if (es->m_hwndListBox)
      {
        if (charW == VK_RETURN || charW == VK_ESCAPE)
        {
          if (gxSendMessageW(gxGetParent(hwnd), CB_GETDROPPEDSTATE, 0, 0))
            gxSendMessageW(gxGetParent(hwnd), WM_KEYDOWN, charW, 0);
          break;
        }
      }
      result = es->WM_Char(charW);
      break;
    }

  case WM_UNICHAR:
    if (unicode)
    {
      if (wParam == UNICODE_NOCHAR) return TRUE;
      if (wParam <= 0x000fffff)
      {
        if(wParam > 0xffff) /* convert to surrogates */
        {
          wParam -= 0x10000;
          es->WM_Char((wParam >> 10) + 0xd800);
          es->WM_Char((wParam & 0x03ff) + 0xdc00);
        }
        else {
          es->WM_Char(wParam);
        }
      }
      return 0;
    }
    break;

  case WM_CLEAR:
    es->WM_Clear();
    break;

  case WM_CONTEXTMENU:
    es->WM_ContextMenu((short)LOWORD(lParam), (short)HIWORD(lParam));
    break;

  case WM_COPY:
    es->WM_Copy();
    break;

  case WM_CREATE:
    if(unicode)
      result = es->WM_Create(((LPCREATESTRUCTW)lParam)->lpszName);
    else
    {
      LPCSTR nameA = ((LPCREATESTRUCTA)lParam)->lpszName;
      LPWSTR nameW = NULL;
      if(nameA)
      {
        INT countW = gxMultiByteToWideChar(CP_ACP, 0, nameA, -1, NULL, 0);
        if((nameW = HeapAlloc(GetProcessHeap(), 0, countW * sizeof(WCHAR)))) {
          gxMultiByteToWideChar(CP_ACP, 0, nameA, -1, nameW, countW);
        }
      }
      result = es->WM_Create(nameW);
      HeapFree(GetProcessHeap(), 0, nameW);
    }
    break;

  case WM_CUT:
    es->WM_Cut();
    break;

  case WM_ENABLE:
    es->m_bEnableState = (GXBOOL) wParam;
    es->UpdateText(NULL, TRUE);
    break;

  case WM_ERASEBKGND:
    /* we do the proper erase in _WM_Paint */
    result = 1;
    break;

  case WM_GETFONT:
    result = (GXLRESULT)es->m_font;
    break;

  case WM_GETTEXT:
    result = (GXLRESULT)es->WM_GetText((INT)wParam, (LPWSTR)lParam, unicode);
    break;

  case WM_GETTEXTLENGTH:
    if (unicode) result = es->get_text_length();
    else result = gxWideCharToMultiByte( CP_ACP, 0, es->m_text, es->get_text_length(),
      NULL, 0, NULL, NULL );
    break;

  case WM_HSCROLL:
    result = es->WM_HScroll(LOWORD(wParam), (short)HIWORD(wParam));
    break;

  case WM_KEYDOWN:
    result = es->WM_KeyDown((INT)wParam);
    break;

  case WM_KILLFOCUS:
    result = es->WM_KillFocus();
    break;

  case WM_LBUTTONDBLCLK:
    result = es->WM_LButtonDblClk();
    break;

  case WM_LBUTTONDOWN:
    result = es->WM_LButtonDown(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
    break;

  case WM_LBUTTONUP:
    result = es->WM_LButtonUp();
    break;

  case WM_MBUTTONDOWN:
    result = es->WM_MButtonDown();
    break;

  case WM_MOUSEMOVE:
    result = es->WM_MouseMove((short)LOWORD(lParam), (short)HIWORD(lParam));
    break;

  case WM_PRINTCLIENT:
  case WM_PAINT:
    es->WM_Paint((GXHDC)wParam);
    break;

  case WM_PASTE:
    es->WM_Paste();
    break;

  case WM_SETFOCUS:
    es->WM_SetFocus();
    break;

  case WM_SETFONT:
    es->WM_SetFont((GXHFONT)wParam, LOWORD(lParam) != 0);
    break;

  case WM_SETREDRAW:
    /* FIXME: actually set an internal flag and behave accordingly */
    break;

  case WM_SETTEXT:
    es->WM_SetText((LPCWSTR)lParam, unicode);
    result = TRUE;
    break;

  case WM_SIZE:
    es->WM_Size((GXUINT)wParam);
    break;

  case WM_STYLECHANGED:
    result = es->WM_StyleChanged(wParam, (const STYLESTRUCT *)lParam);
    break;

  case WM_STYLECHANGING:
    result = 0; /* See _WM_StyleChanged */
    break;

  case WM_SYSKEYDOWN:
    result = es->WM_SysKeyDown((INT)wParam, (GXDWORD)lParam);
    break;

  case WM_TIMER:
    es->WM_Timer();
    break;

  case WM_VSCROLL:
    result = es->WM_VScroll(LOWORD(wParam), (short)HIWORD(wParam));
    break;

  case WM_MOUSEWHEEL:
    {
      int wheelDelta;
      GXUINT pulScrollLines = 3;
      gxSystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0, &pulScrollLines, 0);

      if (wParam & (MK_SHIFT | MK_CONTROL)) {
        result = gxDefWindowProcW(hwnd, msg, wParam, lParam);
        break;
      }
      wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
      /* if scrolling changes direction, ignore left overs */
      if ((wheelDelta < 0 && es->m_wheelDeltaRemainder < 0) ||
        (wheelDelta > 0 && es->m_wheelDeltaRemainder > 0))
        es->m_wheelDeltaRemainder += wheelDelta;
      else {
        es->m_wheelDeltaRemainder = wheelDelta;
      }
      if (es->m_wheelDeltaRemainder && pulScrollLines)
      {
        int cLineScroll;
        pulScrollLines = (int) min((GXUINT) m_line_count, pulScrollLines);
        cLineScroll = pulScrollLines * (float)es->m_wheelDeltaRemainder / WHEEL_DELTA;
        es->m_wheelDeltaRemainder -= WHEEL_DELTA * cLineScroll / (int)pulScrollLines;
        result = es->EM_LineScroll(0, -cLineScroll);
      }
    }
    break;


    /* IME messages to make the edit control IME aware */
  case WM_IME_SETCONTEXT:
    break;

  case WM_IME_STARTCOMPOSITION:
    es->m_composition_start = es->m_selection_end;
    es->m_composition_len = 0;
    break;

  case WM_IME_COMPOSITION:
    es->ImeComposition(hwnd, lParam);
    break;

  case WM_IME_ENDCOMPOSITION:
    if (es->m_composition_len > 0)
    {
      es->EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
      es->m_selection_end = es->m_selection_start;
      es->m_composition_len = 0;
    }
    break;

  case WM_IME_COMPOSITIONFULL:
    break;

  case WM_IME_SELECT:
    break;

  case WM_IME_CONTROL:
    break;

  case WM_IME_REQUEST:
    switch (wParam)
    {
    case IMR_QUERYCHARPOSITION:
      {
        GXLRESULT pos;
        GXIMECHARPOSITION *chpos = (GXIMECHARPOSITION *)lParam;

        pos = es->EM_PosFromChar(es->m_selection_start + chpos->dwCharPos, FALSE);
        chpos->pt.x = LOWORD(pos);
        chpos->pt.y = HIWORD(pos);
        chpos->cLineHeight = es->m_line_height;
        chpos->rcDocument = es->m_format_rect;
        gxMapWindowPoints(hwnd, 0, &chpos->pt, 1);
        gxMapWindowPoints(hwnd, 0, (GXPOINT*)&chpos->rcDocument, 2);
        result = 1;
        break;
      }
    }
    break;

  default:
    result = DefWindowProcT(hwnd, msg, wParam, lParam, unicode);
    break;
  }

  if (gxIsWindow(hwnd) && es) {
    es->UnlockBuffer(FALSE);
  }

  TRACE("hwnd=%p msg=%x (%s) -- 0x%08lx\n", hwnd, msg, SPY_GetMsgName(msg, hwnd), result);

  return result;
}


/*********************************************************************
* edit class descriptor
*/
static const GXWCHAR editW[] = {'E','d','i','t', 0};
//const struct builtin_class_descr _builtin_class =
//{
//  editW,                /* name */
//  CS_DBLCLKS | CS_PARENTDC,   /* style */
//  WINPROC_EDIT,         /* proc */
//#ifdef __i386__
//  sizeof(EDITSTATE *) + sizeof(WORD), /* extra */
//#else
//  sizeof(EDITSTATE *),  /* extra */
//#endif
//  IDC_IBEAM,            /* cursor */
//  0                     /* brush */
//};

static GXLRESULT __stdcall EditWndProcW_1_9_4(GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  return EditWndProc_common(hWnd, (GXUINT)uMsg, wParam, lParam, TRUE);
}

GXWNDCLASSEX WndClassEx_GXUIEdit_1_9_4 = { sizeof(GXWNDCLASSEX), GXCS_CLASSDC, EditWndProcW_1_9_4, 0L, sizeof(EDITSTATE *),
  (GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_IBEAM), NULL, NULL,
  GXUICLASSNAME_EDIT_1_9_4, NULL };

#endif // #if 0