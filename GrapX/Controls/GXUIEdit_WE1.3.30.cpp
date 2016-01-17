// *
// *  Edit control
// *
// *  Copyright  David W. Metcalfe, 1994
// *  Copyright  William Magro, 1995, 1996
// *  Copyright  Frans van Dorsselaer, 1996, 1997
// *
// *
// * This library is free software; you can redistribute it and/or
// * modify it under the terms of the GNU Lesser General Public
// * License as published by the Free Software Foundation; either
// * version 2.1 of the License, or (at your option) any later version.
// *
// * This library is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// * Lesser General Public License for more details.
// *
// * You should have received a copy of the GNU Lesser General Public
// * License along with this library; if not, write to the Free Software
// * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
// *
// * NOTES
// *
// * This code was audited for completeness against the documented features
// * of Comctl32.dll version 6.0 on Oct. 8, 2004, by Dimitrie O. Paun.
// * 
// * Unless otherwise noted, we believe this code to be complete, as per
// * the specification mentioned above.
// * If you discover missing features, or bugs, please note them below.
// *
// * TODO:
// *   - EDITBALLOONTIP structure
// *   - EM_GETCUEBANNER/Edit_GetCueBannerText
// *   - EM_HIDEBALLOONTIP/Edit_HideBalloonTip
// *   - EM_SETCUEBANNER/Edit_SetCueBannerText
// *   - EM_SHOWBALLOONTIP/Edit_ShowBalloonTip
// *   - EM_GETIMESTATUS, EM_SETIMESTATUS
// *   - EN_ALIGN_LTR_EC
// *   - EN_ALIGN_RTL_EC
// *   - ES_OEMCONVERT
// *
// *

// Artint: 这个修改自1.3.30版Wine项目的Edit空间代码

#include <GrapX.H>
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GrapX/GXKernel.H"
#include "GrapX/GXImm.h"
#include "GrapX/GResource.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/GXCanvas.H"
#include "GXUICtrlBase.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.H>

#undef TRACE
#undef TRACE_

#define TRACE
#define TRACE_(_TYPE)  TRACE
#define SPY_GetMsgName(_X, _Y)  ""


#define BUFLIMIT_INITIAL      30000   // initial buffer size
#define GROWLENGTH            32      // buffers granularity in bytes: must be power of 2
#define ROUND_TO_GROW(size)   (((size) + (GROWLENGTH - 1)) & ~(GROWLENGTH - 1))
#define HSCROLL_FRACTION      3       // scroll window by 1/3 width

//
//  extra flags for EDITSTATE.flags field
//
#define EF_MODIFIED       0x0001  // text has been modified
#define EF_FOCUSED        0x0002  // we have input focus
#define EF_UPDATE         0x0004  // notify parent of changed state
#define EF_VSCROLL_TRACK  0x0008  // don't SetScrollPos() since we are tracking the thumb
#define EF_HSCROLL_TRACK  0x0010  // don't SetScrollPos() since we are tracking the thumb
#define EF_AFTER_WRAP     0x0080  // the caret is displayed after the last character of a
// wrapped line, instead of in front of the next character
#define EF_USE_SOFTBRK    0x0100  // Enable soft breaks in text.
#define EF_APP_HAS_HANDLE 0x0200  // Set when an app sends EM_[G|S]ETHANDLE.  We are in sole control of
//       the text buffer if this is clear.
#define EF_DIALOGMODE     0x0400  // Indicates that we are inside a dialog window

using namespace Marimo;

typedef enum
{
  END_0 = 0,     // line ends with terminating '\0' character
  END_WRAP,      // line is wrapped
  END_HARD,      // line ends with a hard return '\r\n'
  END_SOFT,      // line ends with a soft return '\r\r\n'
  END_RICH       // line ends with a single '\n'
} LINE_END;

typedef struct tagLINEDEF {
  GXINT     length;       // bruto length of a line in bytes
  size_t    net_length;   // netto length of a line in visible characters
  LINE_END  ending;
  GXINT     width;        // width of the line in pixels
  size_t    index;        // line index into the buffer
  struct tagLINEDEF *next;
} LINEDEF;

struct EDITSTATE
{
  GXBOOL      is_unicode;         // how the control was created
  GXLPWSTR    m_pText;            // the actual contents of the control
  GXUINT      m_nTextLength;      // cached length of text buffer (in WCHARs) - use get_text_length() to retrieve
  GXUINT      buffer_size;        // the size of the buffer in characters
  GXUINT      buffer_limit;       // the maximum size to which the buffer may grow in characters
  GXHFONT     m_hFont;            // NULL means standard system font
  GXINT       x_offset;           // scroll offset  for multi lines this is in pixels
                                  // for single lines it's in characters
  GXCOLORREF  m_crText;
  GXCOLORREF  m_crBack;
  GXINT       line_height;        // height of a screen line in pixels
  GXINT       char_width;         // average character width in pixels
  GXDWORD     m_dwStyle;          // sane version of wnd->dwStyle
  GXWORD      flags;              // flags that are not in es->style or wnd->flags (EF_XXX)
  GXINT       undo_insert_count;  // number of characters inserted in sequence
  GXUINT      undo_position;      // character index of the insertion and deletion
  GXLPWSTR    undo_text;          // deleted text
  GXUINT      undo_buffer_size;   // size of the deleted text buffer
  GXINT       selection_start;    // == selection_end if no selection
  GXINT       selection_end;      // == current caret position
  GXWCHAR     password_char;      // == 0 if no password char, and for multi line controls
  GXINT       left_margin;        // in pixels
  GXINT       right_margin;       // in pixels
  GXRECT      format_rect;
  GXINT       text_width;         // width of the widest line in pixels for multi line controls
                                  // and just line width for single line controls 
  GXINT       region_posx;        // Position of cursor relative to region:
  GXINT       region_posy;        // -1: to left, 0: within, 1: to right
  void*       word_break_proc;    // 32-bit word break proc: ANSI or Unicode
  GXINT       line_count;         // number of lines
  GXINT       y_offset;           // scroll offset in number of lines
  GXBOOL      bCaptureState;      // flag indicating whether mouse was captured
  GXBOOL      bEnableState;       // flag keeping the enable state
  GXHWND      hwndSelf;           // the our window handle
  GXHWND      hwndParent;         // Handle of parent for sending EN_* messages.
                                  // Even if parent will change, EN_* messages
                                  // should be sent to the first parent.
  GXHWND      hwndListBox;        // handle of ComboBox's listbox or NULL
  MOVariable  m_VarText;

  //
  //  only for multi line controls
  //
  GXINT       lock_count;         // amount of re-entries in the EditWndProc
  GXINT       tabs_count;
  GXLPINT     tabs;
  LINEDEF*    first_line_def;     // linked list of (soft) linebreaks
  GXHLOCAL    hloc32W;            // our unicode local memory block
  GXHLOCAL    hloc32A;            // alias for ANSI control receiving EM_GETHANDLE
                                  // or EM_SETHANDLE
  //
  // IME Data
  //
  GXUINT      composition_len;    // length of composition, 0 == no composition
  int         composition_start;  // the character position for the composition

  static GXINT  WordBreakProc(GXLPWSTR s, GXINT index, GXINT count, GXINT action);
  static GXLRESULT WM_NCCreate(GXHWND hwnd, GXLPCREATESTRUCTW lpcs, GXBOOL unicode);

  inline void   text_buffer_changed();
  inline GXBOOL IsInsideDialog();
  inline GXUINT get_text_length();
  inline GXINT  get_vertical_line_count();
  inline void   EM_EmptyUndoBuffer();
  inline GXBOOL EM_CanUndo();
  inline void   WM_Clear();
  inline void   WM_Cut();

  void      AdjustFormatRect();
  GXINT     CallWordBreakProc(GXINT start, GXINT index, GXINT count, GXINT action);
  GXBOOL    MakeUndoFit(GXUINT size);
  void      CalcLineWidth_SL();
  void      BuildLineDefs_ML(GXINT istart, GXINT iend, GXINT delta, GXHRGN hrgn);
  GXLPWSTR  GetPasswordPointer_SL();
  void      GetLineRect(GXINT line, GXINT scol, GXINT ecol, GXLPRECT rc);
  void      PaintLine(GXHDC dc, GXINT line, GXBOOL rev);
  GXINT     PaintText(GXHDC dc, GXINT x, GXINT y, GXINT line, GXINT col, GXINT count, GXBOOL rev);
  void      UpdateText(const GXRECT *rc, GXBOOL bErase);
  void      UpdateTextRegion(GXHRGN hrgn, GXBOOL bErase);
  void      UpdateScrollInfo();
  GXBOOL    MakeFit(GXUINT size);
  void      LockBuffer();
  void      UnlockBuffer(GXBOOL force);
  void      InvalidateText(GXINT start, GXINT end);
  GXHBRUSH  NotifyCtlColor(GXHDC hdc);
  void      SetRectNP(const GXRECT *rc);
  void      SetCaretPos(GXINT pos, GXBOOL after_wrap);
  void      MoveWordForward(GXBOOL extend);
  void      MoveWordBackward(GXBOOL extend);
  void      MoveForward(GXBOOL extend);
  void      MoveBackward(GXBOOL extend);
  void      MoveUp_ML(GXBOOL extend);
  void      MoveDown_ML(GXBOOL extend);
  void      MoveHome(GXBOOL extend, GXBOOL ctrl);
  void      MoveEnd(GXBOOL extend, GXBOOL ctrl);
  void      MovePageUp_ML(GXBOOL extend);
  void      MovePageDown_ML(GXBOOL extend);
  GXINT     CharFromPos(GXINT x, GXINT y, GXLPBOOL after_wrap);
  void      ConfinePoint(GXLPINT x, GXLPINT y);
  GXBOOL    CheckCombo(GXUINT msg, GXINT key);
  void      GetCompositionStr(GXHIMC hIMC, GXLPARAM CompFlag);
  void      GetResultStr(GXHIMC hIMC);
  void      ImeComposition(GXHWND hwnd, GXLPARAM CompFlag);
  GXBOOL    SolveDefinition (const GXDefinitionArrayW& aDefinitions);


  GXLRESULT EM_CharFromPos(GXINT x, GXINT y);
  GXLRESULT EM_GetSel(GXLPUINT start, GXLPUINT end);
  GXLRESULT EM_Scroll(GXINT action);
  GXBOOL    EM_LineScroll(GXINT dx, GXINT dy);
  void      EM_ScrollCaret();
  void      EM_SetSel(GXUINT start, GXUINT end, GXBOOL after_wrap);
  GXINT     EM_LineIndex(GXINT line);
  GXINT     EM_LineFromChar(GXINT index);
  GXLRESULT EM_PosFromChar(GXINT index, GXBOOL after_wrap);
  GXINT     EM_LineLength(GXINT index);
  GXBOOL    EM_LineScroll_internal(GXINT dx, GXINT dy);
  void      EM_ReplaceSel(GXBOOL can_undo, GXLPCWSTR lpsz_replace, GXBOOL send_update, GXBOOL honor_limit);
  GXINT     EM_GetLine(GXINT line, GXLPWSTR dst, GXBOOL unicode);
  void      EM_SetMargins(GXINT action, GXWORD left, GXWORD right, GXBOOL repaint);
  GXBOOL    EM_Undo();
  GXBOOL    EM_FmtLines(GXBOOL add_eol);
  void      EM_SetHandle(GXHLOCAL hloc);
  void      EM_SetLimitText(GXUINT limit);
  void      EM_SetPasswordChar(GXWCHAR c);
  GXBOOL    EM_SetTabStops(GXINT count, const GXINT *tabs);
  void      EM_SetWordBreakProc(void *wbp);
  GXHLOCAL  EM_GetHandle();
  GXLRESULT EM_GetThumb();

  GXLRESULT WM_Create(GXLPCWSTR name);
  GXLRESULT WM_NCDestroy();
  void      WM_SetFont(GXHFONT font, GXBOOL redraw);
  GXLRESULT WM_MouseMove(GXINT x, GXINT y);
  void      WM_Size(GXUINT action, GXINT width, GXINT height);
  GXLRESULT WM_Char(GXWCHAR c);
  void      WM_Command(GXINT code, GXINT id, GXHWND control);
  void      WM_ContextMenu(GXINT x, GXINT y);
  GXINT     WM_GetText(GXINT count, GXLPWSTR dst, GXBOOL unicode) const;
  GXLRESULT WM_KeyDown(GXINT key);
  GXLRESULT WM_LButtonDown(GXDWORD keys, GXINT x, GXINT y);
  GXLRESULT WM_LButtonUp();
  GXLRESULT WM_MButtonDown();
  GXLRESULT WM_LButtonDblClk();
  GXLRESULT WM_HScroll(GXINT action, GXINT pos);
  void      WM_SetFocus();
  GXLRESULT WM_KillFocus();
  GXLRESULT WM_SysKeyDown(GXINT key, GXDWORD key_data);
  void      WM_Timer();
  void      WM_SetText(GXLPCWSTR text, GXBOOL unicode);
  GXLRESULT WM_VScroll(GXINT action, GXINT pos);
  GXLRESULT WM_StyleChanged (GXWPARAM which, const STYLESTRUCT *style);
  void      WM_Paint(GXHDC hdc);
  void      WM_Copy();
  void      WM_Paste();

  void      ML_InvalidateText(GXINT start, GXINT end);
  void      SL_InvalidateText(GXINT start, GXINT end);

  GXLRESULT SetVariable (MOVariable* pVariable);
  GXHRESULT OnKnock     (KNOCKACTION* pKnock);
  GXVOID    OnImpulse   (LPCDATAIMPULSE pImpulse);

};


#define SWAP_GXUINT32(x,y) do { GXUINT temp = (GXUINT)(x); (x) = (GXUINT)(y); (y) = temp; } while(0)
#define ORDER_GXUINT(x,y) do { if ((GXUINT)(y) < (GXUINT)(x)) SWAP_GXUINT32((x),(y)); } while(0)

/* used for disabled or read-only edit control */
#define EDIT_NOTIFY_PARENT(wNotifyCode) \
  do \
{ /* Notify parent which has created this edit control */ \
  TRACE("notification " #wNotifyCode " sent to hwnd=%p\n", hwndParent); \
  gxSendMessageW(hwndParent, WM_COMMAND, \
  MAKEWPARAM(gxGetWindowLongPtrW((hwndSelf),GWLP_ID), wNotifyCode), \
  (GXLPARAM)(hwndSelf)); \
} while(0)

static const GXWCHAR empty_stringW[] = {0};

/*********************************************************************
*
*  EM_CANUNDO
*
*/
inline GXBOOL EDITSTATE::EM_CanUndo()
{
  return (undo_insert_count || GXSTRLEN(undo_text));
}


/*********************************************************************
*
*  EM_EMPTYUNDOBUFFER
*
*/
inline void EDITSTATE::EM_EmptyUndoBuffer()
{
  undo_insert_count = 0;
  *undo_text = '\0';
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
static GXDWORD get_app_version(void)
{
  return 0x40000;
  //   static GXDWORD version;
  //   if (!version)
  //   {
  //       GXDWORD dwEmulatedVersion;
  //       GXOSVERSIONINFOW info;
  //       GXDWORD dwProcVersion = gxGetProcessVersion(0);

  //info.dwOSVersionInfoSize = sizeof(GXOSVERSIONINFOW);
  //       gxGetVersionExW( &info );
  //       dwEmulatedVersion = MAKELONG( info.dwMinorVersion, info.dwMajorVersion );
  //       /* FIXME: this may not be 100% correct; see discussion on the
  //        * wine developer list in Nov 1999 */
  //       version = dwProcVersion < dwEmulatedVersion ? dwProcVersion : dwEmulatedVersion;
  //   }
  //   return version;
}

GXHBRUSH EDITSTATE::NotifyCtlColor(GXHDC hdc)
{
  GXHBRUSH hBrush;
  GXUINT msg;

  if ( get_app_version() >= 0x40000 && (!bEnableState || (m_dwStyle & GXES_READONLY)))
    msg = GXWM_CTLCOLORSTATIC;
  else
    msg = GXWM_CTLCOLOREDIT;

  /* why do we notify to hwndParent, and we send this one to gxGetParent()? */
  hBrush = (GXHBRUSH)gxSendMessageW(gxGetParent(hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)hwndSelf);
  if (!hBrush)
    hBrush = (GXHBRUSH)gxDefWindowProcW(gxGetParent(hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)hwndSelf);
  return hBrush;
}


/*********************************************************************
*
*  EDIT_WordBreakProc
*
*  Find the beginning of words.
*  Note:  unlike the specs for a WordBreakProc, this function only
*    allows to be called without linebreaks between s[0] up to
*    s[count - 1].  Remember it is only called
*    internally, so we can decide this for ourselves.
*
*/
GXINT EDITSTATE::WordBreakProc(GXLPWSTR s, GXINT index, GXINT count, GXINT action)
{
  GXINT ret = 0;

  TRACE("s=%p, index=%d, count=%d, action=%d\n", s, index, count, action);

  if(!s) return 0;

  switch (action) {
  case GXWB_LEFT:
    if (!count)
      break;
    if (index)
      index--;
    if (s[index] == ' ') {
      while (index && (s[index] == ' '))
        index--;
      if (index) {
        while (index && (s[index] != ' '))
          index--;
        if (s[index] == ' ')
          index++;
      }
    } else {
      while (index && (s[index] != ' '))
        index--;
      if (s[index] == ' ')
        index++;
    }
    ret = index;
    break;
  case GXWB_RIGHT:
    if (!count)
      break;
    if (index)
      index--;
    if (s[index] == ' ')
      while ((index < count) && (s[index] == ' ')) index++;
    else {
      while (s[index] && (s[index] != ' ') && (index < count))
        index++;
      while ((s[index] == ' ') && (index < count)) index++;
    }
    ret = index;
    break;
  case GXWB_ISDELIMITER:
    ret = (s[index] == ' ');
    break;
  default:
    ERR("unknown action code, please report !\n");
    break;
  }
  return ret;
}


/*********************************************************************
*
*  EDIT_CallWordBreakProc
*
*  Call appropriate WordBreakProc (internal or external).
*
*  Note: The "start" argument should always be an index referring
*    to es->text.  The actual wordbreak proc might be
*    16 bit, so we can't always pass any 32 bit LPSTR.
*    Hence we assume that es->text is the buffer that holds
*    the string under examination (we can decide this for ourselves).
*
*/
GXINT EDITSTATE::CallWordBreakProc(GXINT start, GXINT index, GXINT count, GXINT action)
{
  GXINT ret;

  if (word_break_proc)
  {
    if(is_unicode)
    {
      GXEDITWORDBREAKPROCW wbpW = (GXEDITWORDBREAKPROCW)word_break_proc;

      TRACE_(relay)("(UNICODE wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n",
        word_break_proc, debugstr_wn(m_pText + start, count), index, count, action);
      ret = wbpW(m_pText + start, index, count, action);
    }
    else
    {
      EDITWORDBREAKPROCA wbpA = (EDITWORDBREAKPROCA)word_break_proc;
      GXINT countA;
      GXCHAR* textA;

      countA = gxWideCharToMultiByte(CP_ACP, 0, m_pText + start, count, NULL, 0, NULL, NULL);
      textA = (GXCHAR*)gxHeapAlloc(gxGetProcessHeap(), 0, countA);
      gxWideCharToMultiByte(CP_ACP, 0, m_pText + start, count, textA, countA, NULL, NULL);
      TRACE_(relay)("(ANSI wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n",
        word_break_proc, debugstr_an(textA, countA), index, countA, action);
      ret = wbpA(textA, index, countA, action);
      gxHeapFree(gxGetProcessHeap(), 0, textA);
    }
  }
  else
    ret = WordBreakProc(m_pText + start, index, count, action);

  return ret;
}

/*********************************************************************
*
*  EDIT_BuildLineDefs_ML
*
*  Build linked list of text lines.
*  Lines can end with '\0' (last line), a character (if it is wrapped),
*  a soft return '\r\r\n' or a hard return '\r\n'
*
*/
void EDITSTATE::BuildLineDefs_ML(GXINT istart, GXINT iend, GXINT delta, GXHRGN hrgn)
{
  GXHDC dc;
  GXHFONT old_font = 0;
  GXLPWSTR current_position, cp;
  GXINT fw;
  LINEDEF *current_line;
  LINEDEF *previous_line;
  LINEDEF *start_line;
  size_t line_index = 0, nstart_line = 0, nstart_index = 0;
  GXINT l_line_count = line_count;
  size_t orig_net_length;
  GXRECT rc;

  if (istart == iend && delta == 0)
    return;

  dc = gxGetDC(hwndSelf);
  if (m_hFont)
    old_font = (GXHFONT)gxSelectObject(dc, m_hFont);

  previous_line = NULL;
  current_line = first_line_def;

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
    gxReleaseDC(hwndSelf, dc);
    return;
  }

  /* Remember start of modifications in order to calculate update region */
  nstart_line = line_index;
  nstart_index = current_line->index;

  /* We must start to reformat from the previous line since the modifications
  * may have caused the line to wrap upwards. */
  if (!(m_dwStyle & GXES_AUTOHSCROLL) && line_index > 0)
  {
    line_index--;
    current_line = previous_line;
  }
  start_line = current_line;

  fw = format_rect.right - format_rect.left;
  current_position = m_pText + current_line->index;
  do {
    if (current_line != start_line)
    {
      if ( ! current_line || (current_line->index + (size_t)delta) > (size_t)(current_position - m_pText))
      {
        /* The buffer has been expanded, create a new line and
        insert it into the link list */
        LINEDEF *new_line = (LINEDEF*)gxHeapAlloc(gxGetProcessHeap(), 0, sizeof(LINEDEF));
        new_line->next = previous_line->next;
        previous_line->next = new_line;
        current_line = new_line;
        line_count++;
      }
      else if (current_line->index + (size_t)delta < (size_t)(current_position - m_pText))
      {
        /* The previous line merged with this line so we delete this extra entry */
        previous_line->next = current_line->next;
        gxHeapFree(gxGetProcessHeap(), 0, current_line);
        current_line = previous_line->next;
        line_count--;
        continue;
      }
      else /* current_line->index + delta == current_position */
      {
        if (current_position - m_pText > iend)
          break; /* We reached end of line modifications */
        /* else recalculate this line */
      }
    }

    current_line->index = current_position - m_pText;
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

    /* Calculate line width */
    current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
      current_position, (GXINT)current_line->net_length,
      tabs_count, tabs));

    /* FIXME: check here for lines that are too wide even in AUTOHSCROLL (> 32767 ???) */
    if (!(m_dwStyle & ES_AUTOHSCROLL)) {
      if (current_line->width > fw) {
        GXINT next = 0;
        GXINT prev;
        do {
          prev = next;
          next = CallWordBreakProc((GXINT)(current_position - m_pText),
            prev + 1, (GXINT)current_line->net_length, GXWB_RIGHT);
          current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
            current_position, next, tabs_count, tabs));
        } while (current_line->width <= fw);
        if (!prev) { /* Didn't find a line break so force a break */
          next = 0;
          do {
            prev = next;
            next++;
            current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
              current_position, next, tabs_count, tabs));
          } while (current_line->width <= fw);
          if (!prev)
            prev = 1;
        }

        /* If the first line we are calculating, wrapped before istart, we must
        * adjust istart in order for this to be reflected in the update region. */
        if (current_line->index == nstart_index && (size_t)istart > current_line->index + (size_t)prev)
          istart = (GXINT)(current_line->index + prev);
        /* else if we are updating the previous line before the first line we
        * are re-calculating and it expanded */
        else if (current_line == start_line &&
          current_line->index != nstart_index && orig_net_length < prev)
        {
          /* Line expanded due to an upwards line wrap so we must partially include
          * previous line in update region */
          nstart_line = line_index;
          nstart_index = current_line->index;
          istart = (GXINT)(current_line->index + orig_net_length);
        }

        current_line->net_length = prev;
        current_line->ending = END_WRAP;
        current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc, current_position,
          (GXINT)current_line->net_length, tabs_count, tabs));
      }
      else if (current_line == start_line &&
        current_line->index != nstart_index &&
        orig_net_length < current_line->net_length) {
          /* The previous line expanded but it's still not as wide as the client rect */
          /* The expansion is due to an upwards line wrap so we must partially include
          it in the update region */
          nstart_line = line_index;
          nstart_index = current_line->index;
          istart = (GXINT)(current_line->index + orig_net_length);
      }
    }


    /* Adjust length to include line termination */
    switch (current_line->ending) {
    case END_SOFT:
      current_line->length = (GXINT)(current_line->net_length + 3);
      break;
    case END_RICH:
      current_line->length = (GXINT)(current_line->net_length + 1);
      break;
    case END_HARD:
      current_line->length = (GXINT)(current_line->net_length + 2);
      break;
    case END_WRAP:
    case END_0:
      current_line->length = (GXINT)current_line->net_length;
      break;
    }
    text_width = max(text_width, current_line->width);
    current_position += current_line->length;
    previous_line = current_line;
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
      gxHeapFree(gxGetProcessHeap(), 0, current_line);
      current_line = pnext;
      line_count--;
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
    rc.top = (GXLONG)(format_rect.top + nstart_line * line_height -
      (y_offset * line_height)); /* Adjust for vertical scrollbar */
    rc.bottom = rc.top + line_height;
    if ((m_dwStyle & ES_CENTER) || (m_dwStyle & ES_RIGHT))
      rc.left = format_rect.left;
    else
      rc.left = format_rect.left + (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
      m_pText + nstart_index, (GXINT)(istart - nstart_index),
      tabs_count, tabs)) - x_offset; /* Adjust for horz scroll */
    rc.right = format_rect.right;
    gxSetRectRgn(hrgn, rc.left, rc.top, rc.right, rc.bottom);

    rc.top = rc.bottom;
    rc.left = format_rect.left;
    rc.right = format_rect.right;
    /*
    * If lines were added or removed we must re-paint the remainder of the
    * lines since the remaining lines were either shifted up or down.
    */
    if (l_line_count < line_count) /* We added lines */
      rc.bottom = line_count * line_height;
    else if (l_line_count > line_count) /* We removed lines */
      rc.bottom = l_line_count * line_height;
    else
      rc.bottom = (GXLONG)(line_index * line_height);
    rc.bottom += format_rect.top;
    rc.bottom -= (y_offset * line_height); /* Adjust for vertical scrollbar */
    tmphrgn = gxCreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
    gxCombineRgn(hrgn, hrgn, tmphrgn, RGN_OR);
    gxDeleteObject(tmphrgn);
  }

  if (m_hFont) {
    gxSelectObject(dc, old_font);
  }

  gxReleaseDC(hwndSelf, dc);
}

inline GXUINT EDITSTATE::get_text_length()
{
  if(m_nTextLength == (GXUINT)-1)
    m_nTextLength = GXSTRLEN(m_pText);
  return m_nTextLength;
}

/*********************************************************************
*
*  EDIT_GetPasswordPointer_SL
*
*  note: caller should free the (optionally) allocated buffer
*
*/
GXLPWSTR EDITSTATE::GetPasswordPointer_SL()
{
  if (m_dwStyle & GXES_PASSWORD) {
    GXINT len = get_text_length();
    GXLPWSTR text = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (len + 1) * sizeof(GXWCHAR));
    text[len] = '\0';
    while(len) text[--len] = password_char;
    return text;
  } else
    return m_pText;
}


/*********************************************************************
*
*  EDIT_CalcLineWidth_SL
*
*/
void EDITSTATE::CalcLineWidth_SL()
{
  GXSIZE size;
  GXLPWSTR text;
  GXHDC dc;
  GXHFONT old_font = 0;

  text = GetPasswordPointer_SL();

  dc = gxGetDC(hwndSelf);
  if (m_hFont)
    old_font = (GXHFONT)gxSelectObject(dc, m_hFont);

  gxGetTextExtentPoint32W(dc, text, GXSTRLEN(text), &size);

  if (m_hFont)
    gxSelectObject(dc, old_font);
  gxReleaseDC(hwndSelf, dc);

  if (m_dwStyle & ES_PASSWORD)
    gxHeapFree(gxGetProcessHeap(), 0, text);

  text_width = size.cx;
}

/*********************************************************************
*
*  EDIT_CharFromPos
*
*  Beware: This is not the function called on EM_CHARFROMPOS
*    The position _can_ be outside the formatting / client
*    rectangle
*    The return value is only the character index
*
*/
GXINT EDITSTATE::CharFromPos(GXINT x, GXINT y, GXLPBOOL after_wrap)
{
  GXINT index;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXINT x_high = 0, x_low = 0;

  if (m_dwStyle & GXES_MULTILINE) {
    GXINT line = (y - format_rect.top) / line_height + y_offset;
    GXINT line_index = 0;
    LINEDEF *line_def = first_line_def;
    GXINT low, high;
    while ((line > 0) && line_def->next) {
      line_index += line_def->length;
      line_def = line_def->next;
      line--;
    }
    x += x_offset - format_rect.left;
    if (m_dwStyle & GXES_RIGHT)
      x -= (format_rect.right - format_rect.left) - line_def->width;
    else if (m_dwStyle & GXES_CENTER)
      x -= ((format_rect.right - format_rect.left) - line_def->width) / 2;
    if (x >= line_def->width) {
      if (after_wrap)
        *after_wrap = (line_def->ending == END_WRAP);
      return (GXINT)(line_index + line_def->net_length);
    }
    if (x <= 0) {
      if (after_wrap)
        *after_wrap = FALSE;
      return line_index;
    }
    dc = gxGetDC(hwndSelf);
    if (m_hFont)
      old_font = (GXHFONT)gxSelectObject(dc, m_hFont);
    low = line_index;
    high = (GXINT)(line_index + line_def->net_length + 1);
    while (low < high - 1)
    {
      GXINT mid = (low + high) / 2;
      GXINT x_now = GXLOWORD(gxGetTabbedTextExtentW(dc, m_pText + line_index, mid - line_index, tabs_count, tabs));
      if (x_now > x) {
        high = mid;
        x_high = x_now;
      } else {
        low = mid;
        x_low = x_now;
      }
    }
    if (abs(x_high - x) + 1 <= abs(x_low - x))
      index = high;
    else
      index = low;

    if (after_wrap)
      *after_wrap = ((index == line_index + line_def->net_length) &&
      (line_def->ending == END_WRAP));
  } else {
    GXLPWSTR text;
    GXSIZE size;
    if (after_wrap)
      *after_wrap = FALSE;
    x -= format_rect.left;
    if (!x)
      return x_offset;

    if (!x_offset)
    {
      GXINT indent = (format_rect.right - format_rect.left) - text_width;
      if (m_dwStyle & GXES_RIGHT)
        x -= indent;
      else if (m_dwStyle & GXES_CENTER)
        x -= indent / 2;
    }

    text = GetPasswordPointer_SL();
    dc = gxGetDC(hwndSelf);
    if (m_hFont)
      old_font = (GXHFONT)gxSelectObject(dc, m_hFont);
    if (x < 0)
    {
      GXINT low = 0;
      GXINT high = x_offset;
      while (low < high - 1)
      {
        GXINT mid = (low + high) / 2;
        gxGetTextExtentPoint32W( dc, text + mid,
          x_offset - mid, &size );
        if (size.cx > -x) {
          low = mid;
          x_low = size.cx;
        } else {
          high = mid;
          x_high = size.cx;
        }
      }
      if (abs(x_high + x) <= abs(x_low + x) + 1)
        index = high;
      else
        index = low;
    }
    else
    {
      GXINT low = x_offset;
      GXINT high = get_text_length() + 1;
      while (low < high - 1)
      {
        GXINT mid = (low + high) / 2;
        gxGetTextExtentPoint32W( dc, text + x_offset,
          mid - x_offset, &size );
        if (size.cx > x) {
          high = mid;
          x_high = size.cx;
        } else {
          low = mid;
          x_low = size.cx;
        }
      }
      if (abs(x_high - x) <= abs(x_low - x) + 1)
        index = high;
      else
        index = low;
    }
    if (m_dwStyle & GXES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }
  if (m_hFont)
    gxSelectObject(dc, old_font);
  gxReleaseDC(hwndSelf, dc);
  return index;
}


/*********************************************************************
*
*  EDIT_ConfinePoint
*
*  adjusts the point to be within the formatting rectangle
*  (so CharFromPos returns the nearest _visible_ character)
*
*/
void EDITSTATE::ConfinePoint(GXLPINT x, GXLPINT y)
{
  *x = min(max(*x, format_rect.left), format_rect.right - 1);
  *y = min(max(*y, format_rect.top), format_rect.bottom - 1);
}


/*********************************************************************
*
*  EM_LINEFROMCHAR
*
*/
GXINT EDITSTATE::EM_LineFromChar(GXINT index)
{
  GXINT line;
  LINEDEF *line_def;

  if (!(m_dwStyle & GXES_MULTILINE))
    return 0;
  if (index > (GXINT)get_text_length())
    return line_count - 1;
  if (index == -1)
    index = min(selection_start, selection_end);

  line = 0;
  line_def = first_line_def;
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
GXINT EDITSTATE::EM_LineIndex(GXINT line)
{
  GXINT line_index;
  const LINEDEF *line_def;

  if (!(m_dwStyle & GXES_MULTILINE))
    return 0;
  if (line >= line_count)
    return -1;

  line_index = 0;
  line_def = first_line_def;
  if (line == -1) {
    GXINT index = selection_end - line_def->length;
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
GXINT EDITSTATE::EM_LineLength(GXINT index)
{
  LINEDEF *line_def;

  if (!(m_dwStyle & GXES_MULTILINE))
    return get_text_length();

  if (index == -1) {
    /* get the number of remaining non-selected chars of selected lines */
    INT32 l; /* line number */
    INT32 li; /* index of first char in line */
    INT32 count;
    l = EM_LineFromChar(selection_start);
    /* # chars before start of selection area */
    count = selection_start - EM_LineIndex(l);
    l = EM_LineFromChar(selection_end);
    /* # chars after end of selection */
    li = EM_LineIndex(l);
    count += li + EM_LineLength(li) - selection_end;
    return count;
  }
  line_def = first_line_def;
  index -= line_def->length;
  while ((index >= 0) && line_def->next) {
    line_def = line_def->next;
    index -= line_def->length;
  }
  return (GXINT)line_def->net_length;
}


/*********************************************************************
*
*  EM_POSFROMCHAR
*
*/
GXLRESULT EDITSTATE::EM_PosFromChar(GXINT index, GXBOOL after_wrap)
{
  GXINT len = get_text_length();
  GXINT l;
  GXINT li;
  GXINT x;
  GXINT y = 0;
  GXINT w;
  GXINT lw = 0;
  GXINT ll = 0;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXSIZE size;
  LINEDEF *line_def;

  index = min(index, len);
  dc = gxGetDC(hwndSelf);
  if (m_hFont)
    old_font = (GXHFONT)gxSelectObject(dc, m_hFont);
  if (m_dwStyle & GXES_MULTILINE) {
    l = EM_LineFromChar(index);
    y = (l - y_offset) * line_height;
    li = EM_LineIndex(l);
    if (after_wrap && (li == index) && l) {
      GXINT l2 = l - 1;
      line_def = first_line_def;
      while (l2) {
        line_def = line_def->next;
        l2--;
      }
      if (line_def->ending == END_WRAP) {
        l--;
        y -= line_height;
        li = EM_LineIndex(l);
      }
    }

    line_def = first_line_def;
    while (line_def->index != li)
      line_def = line_def->next;

    ll = (GXINT)line_def->net_length;
    lw = line_def->width;

    w = format_rect.right - format_rect.left;
    if (m_dwStyle & GXES_RIGHT)
    {
      x = GXLOWORD(gxGetTabbedTextExtentW(dc, m_pText + li + (index - li), ll - (index - li),
        tabs_count, tabs)) - x_offset;
      x = w - x;
    }
    else if (m_dwStyle & GXES_CENTER)
    {
      x = GXLOWORD(gxGetTabbedTextExtentW(dc, m_pText + li, index - li,
        tabs_count, tabs)) - x_offset;
      x += (w - lw) / 2;
    }
    else /* ES_LEFT */
    {
      x = GXLOWORD(gxGetTabbedTextExtentW(dc, m_pText + li, index - li,
        tabs_count, tabs)) - x_offset;
    }
  } else {
    GXLPWSTR text = GetPasswordPointer_SL();
    if (index < x_offset) {
      gxGetTextExtentPoint32W(dc, text + index,
        x_offset - index, &size);
      x = -size.cx;
    } else {
      gxGetTextExtentPoint32W(dc, text + x_offset,
        index - x_offset, &size);
      x = size.cx;

      if (!x_offset && (m_dwStyle & (ES_RIGHT | ES_CENTER)))
      {
        w = format_rect.right - format_rect.left;
        if (w > text_width)
        {
          if (m_dwStyle & ES_RIGHT)
            x += w - text_width;
          else if (m_dwStyle & ES_CENTER)
            x += (w - text_width) / 2;
        }
      }
    }
    y = 0;
    if (m_dwStyle & ES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }
  x += format_rect.left;
  y += format_rect.top;
  if (m_hFont)
    gxSelectObject(dc, old_font);
  gxReleaseDC(hwndSelf, dc);
  return MAKELONG((INT16)x, (INT16)y);
}


/*********************************************************************
*
*  EDIT_GetLineRect
*
*  Calculates the bounding rectangle for a line from a starting
*  column to an ending column.
*
*/
void EDITSTATE::GetLineRect(GXINT line, GXINT scol, GXINT ecol, GXLPRECT rc)
{
  GXINT line_index = EM_LineIndex(line);

  if (m_dwStyle & ES_MULTILINE)
    rc->top = format_rect.top + (line - y_offset) * line_height;
  else
    rc->top = format_rect.top;
  rc->bottom = rc->top + line_height;
  rc->left = (scol == 0) ? format_rect.left : (short)GXLOWORD(EM_PosFromChar( line_index + scol, TRUE));
  rc->right = (ecol == -1) ? format_rect.right : (short)GXLOWORD(EM_PosFromChar(line_index + ecol, TRUE));
}


inline void EDITSTATE::text_buffer_changed()
{
  m_nTextLength = (GXUINT)-1;
}

/*********************************************************************
* EDIT_LockBuffer
*
*/
void EDITSTATE::LockBuffer()
{
  if (!m_pText) {

    if(!hloc32W) return;

    if(hloc32A)
    {
      GXCHAR *textA = (GXCHAR*)gxLocalLock(hloc32A);
      GXHLOCAL hloc32W_new;
      GXUINT countW_new = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
      if(countW_new > buffer_size + 1)
      {
        GXUINT alloc_size = ROUND_TO_GROW(countW_new * sizeof(GXWCHAR));
        TRACE("Resizing 32-bit UNICODE buffer from %d+1 to %d WCHARs\n", buffer_size, countW_new);
        hloc32W_new = gxLocalReAlloc(hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if(hloc32W_new)
        {
          hloc32W = hloc32W_new;
          buffer_size = (GXUINT)gxLocalSize(hloc32W_new)/sizeof(GXWCHAR) - 1;
          TRACE("Real new size %d+1 WCHARs\n", buffer_size);
        }
        else
          WARN("FAILED! Will synchronize partially\n");
      }
      m_pText = (GXLPWSTR)gxLocalLock(hloc32W);
      gxMultiByteToWideChar(CP_ACP, 0, textA, -1, m_pText, buffer_size + 1);
      gxLocalUnlock(hloc32A);
    }
    else m_pText = (GXLPWSTR)gxLocalLock(hloc32W);
  }
  if(flags & EF_APP_HAS_HANDLE) text_buffer_changed();
  lock_count++;
}


/*********************************************************************
*
*  EDIT_UnlockBuffer
*
*/
void EDITSTATE::UnlockBuffer(GXBOOL force)
{
  /* Edit window might be already destroyed */
  if(!gxIsWindow(hwndSelf))
  {
    WARN("edit hwnd %p already destroyed\n", hwndSelf);
    return;
  }

  if (!lock_count) {
    ERR("lock_count == 0 ... please report\n");
    return;
  }
  if (!m_pText) {
    ERR("text == 0 ... please report\n");
    return;
  }

  if (force || (lock_count == 1)) {
    if (hloc32W) {
      GXUINT countA = 0;
      GXUINT countW = get_text_length() + 1;

      if(hloc32A)
      {
        GXUINT countA_new = gxWideCharToMultiByte(CP_ACP, 0, m_pText, countW, NULL, 0, NULL, NULL);
        TRACE("Synchronizing with 32-bit ANSI buffer\n");
        TRACE("%d WCHARs translated to %d bytes\n", countW, countA_new);
        countA = (GXUINT)LocalSize(hloc32A);
        if(countA_new > countA)
        {
          GXHLOCAL hloc32A_new;
          GXUINT alloc_size = ROUND_TO_GROW(countA_new);
          TRACE("Resizing 32-bit ANSI buffer from %d to %d bytes\n", countA, alloc_size);
          hloc32A_new = gxLocalReAlloc(hloc32A, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
          if(hloc32A_new)
          {
            hloc32A = hloc32A_new;
            countA = (GXUINT)LocalSize(hloc32A_new);
            TRACE("Real new size %d bytes\n", countA);
          }
          else
            WARN("FAILED! Will synchronize partially\n");
        }
        gxWideCharToMultiByte(CP_ACP, 0, m_pText, countW,
          (GXLPSTR)gxLocalLock(hloc32A), countA, NULL, NULL);
        gxLocalUnlock(hloc32A);
      }

      gxLocalUnlock(hloc32W);
      m_pText = NULL;
    }
    else {
      ERR("no buffer ... please report\n");
      return;
    }
  }
  lock_count--;
}


/*********************************************************************
*
*  EDIT_MakeFit
*
* Try to fit size + 1 characters in the buffer.
*/
GXBOOL EDITSTATE::MakeFit(GXUINT size)
{
  GXHLOCAL hNew32W;

  if (size <= buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1 characters\n", size);

  /* Force edit to unlock it's buffer. text now NULL */
  UnlockBuffer(TRUE);

  if (hloc32W) {
    GXUINT alloc_size = ROUND_TO_GROW((size + 1) * sizeof(GXWCHAR));
    if ((hNew32W = gxLocalReAlloc(hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT))) {
      TRACE("Old 32 bit handle %p, new handle %p\n", hloc32W, hNew32W);
      hloc32W = hNew32W;
      buffer_size = (GXUINT)gxLocalSize(hNew32W)/sizeof(GXWCHAR) - 1;
    }
  }

  LockBuffer();

  if (buffer_size < size) {
    WARN("FAILED !  We now have %d+1\n", buffer_size);
    EDIT_NOTIFY_PARENT(EN_ERRSPACE);
    return FALSE;
  } else {
    TRACE("We now have %d+1\n", buffer_size);
    return TRUE;
  }
}


/*********************************************************************
*
*  EDIT_MakeUndoFit
*
*  Try to fit size + 1 bytes in the undo buffer.
*
*/
GXBOOL EDITSTATE::MakeUndoFit(GXUINT size)
{
  GXUINT alloc_size;

  if (size <= undo_buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1\n", size);

  alloc_size = ROUND_TO_GROW((size + 1) * sizeof(GXWCHAR));
  if ((undo_text = (GXLPWSTR)gxHeapReAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, undo_text, alloc_size))) {
    undo_buffer_size = alloc_size/sizeof(GXWCHAR) - 1;
    return TRUE;
  }
  else
  {
    WARN("FAILED !  We now have %d+1\n", undo_buffer_size);
    return FALSE;
  }
}


/*********************************************************************
*
*  EDIT_UpdateTextRegion
*
*/
void EDITSTATE::UpdateTextRegion(GXHRGN hrgn, GXBOOL bErase)
{
  if (flags & EF_UPDATE) {
    flags &= ~EF_UPDATE;
    EDIT_NOTIFY_PARENT(EN_UPDATE);
  }
  gxInvalidateRgn(hwndSelf, hrgn, bErase);
}


/*********************************************************************
*
*  EDIT_UpdateText
*
*/
void EDITSTATE::UpdateText(const GXRECT *rc, GXBOOL bErase)
{
  if (flags & EF_UPDATE) {
    flags &= ~EF_UPDATE;
    EDIT_NOTIFY_PARENT(EN_UPDATE);
  }
  gxInvalidateRect(hwndSelf, rc, bErase);
}

/*********************************************************************
*
*  EDIT_SL_InvalidateText
*
*  Called from EDIT_InvalidateText().
*  Does the job for single-line controls only.
*
*/
void EDITSTATE::SL_InvalidateText(GXINT start, GXINT end)
{
  GXRECT line_rect;
  GXRECT rc;

  GetLineRect(0, start, end, &line_rect);
  if (gxIntersectRect(&rc, &line_rect, &format_rect))
    UpdateText(&rc, TRUE);
}


inline GXINT EDITSTATE::get_vertical_line_count()
{
  GXINT vlc = (format_rect.bottom - format_rect.top) / line_height;
  return max(1,vlc);
}

/*********************************************************************
*
*  EDIT_ML_InvalidateText
*
*  Called from EDIT_InvalidateText().
*  Does the job for multi-line controls only.
*
*/
void EDITSTATE::ML_InvalidateText(GXINT start, GXINT end)
{
  GXINT vlc = get_vertical_line_count();
  GXINT sl = EM_LineFromChar(start);
  GXINT el = EM_LineFromChar(end);
  GXINT sc;
  GXINT ec;
  GXRECT rc1;
  GXRECT rcWnd;
  GXRECT rcLine;
  GXRECT rcUpdate;
  GXINT l;

  if ((el < y_offset) || (sl > y_offset + vlc))
    return;

  sc = start - EM_LineIndex(sl);
  ec = end - EM_LineIndex(el);
  if (sl < y_offset) {
    sl = y_offset;
    sc = 0;
  }
  if (el > y_offset + vlc) {
    el = y_offset + vlc;
    ec = EM_LineLength(EM_LineIndex(el));
  }
  gxGetClientRect(hwndSelf, &rc1);
  gxIntersectRect(&rcWnd, &rc1, &format_rect);
  if (sl == el) {
    GetLineRect(sl, sc, ec, &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      UpdateText(&rcUpdate, TRUE);
  } else {
    GetLineRect(sl, sc, EM_LineLength(EM_LineIndex(sl)), &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      UpdateText(&rcUpdate, TRUE);
    for (l = sl + 1 ; l < el ; l++) {
      GetLineRect(l, 0,
        EM_LineLength(EM_LineIndex(l)), &rcLine);
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
*  EDIT_InvalidateText
*
*  Invalidate the text from offset start up to, but not including,
*  offset end.  Useful for (re)painting the selection.
*  Regions outside the linewidth are not invalidated.
*  end == -1 means end == TextLength.
*  start and end need not be ordered.
*
*/
void EDITSTATE::InvalidateText(GXINT start, GXINT end)
{
  if (end == start)
    return;

  if (end == -1)
    end = get_text_length();

  if (end < start) {
    GXINT tmp = start;
    start = end;
    end = tmp;
  }

  if (m_dwStyle & ES_MULTILINE)
    ML_InvalidateText(start, end);
  else
    SL_InvalidateText(start, end);
}


/*********************************************************************
*
*  EDIT_EM_SetSel
*
*  note:  unlike the specs say: the order of start and end
*    _is_ preserved in Windows.  (i.e. start can be > end)
*    In other words: this handler is OK
*
*/
void EDITSTATE::EM_SetSel(GXUINT start, GXUINT end, GXBOOL after_wrap)
{
  GXUINT old_start = selection_start;
  GXUINT old_end = selection_end;
  GXUINT len = get_text_length();

  if (start == (GXUINT)-1) {
    start = selection_end;
    end = selection_end;
  } else {
    start = min(start, len);
    end = min(end, len);
  }
  selection_start = start;
  selection_end = end;
  if (after_wrap)
    flags |= EF_AFTER_WRAP;
  else
    flags &= ~EF_AFTER_WRAP;
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
  ORDER_GXUINT(end, old_end);
  ORDER_GXUINT(start, old_start);
  ORDER_GXUINT(old_start, old_end);
  ORDER_GXUINT(start, end);
  /* Note that at this point 'end' and 'old_start' are not in order, but
  * start is definitely the min. and old_end is definitely the max. */
  if (end != old_start)
  {
    /*
    * One can also do
    *          ORDER_GXUINT32(end, old_start);
    *          EDIT_InvalidateText(es, start, end);
    *          EDIT_InvalidateText(es, old_start, old_end);
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
  else InvalidateText(start, old_end);
}


/*********************************************************************
*
*  EDIT_UpdateScrollInfo
*
*/
void EDITSTATE::UpdateScrollInfo()
{
  if ((m_dwStyle & GXWS_VSCROLL) && !(flags & EF_VSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize  = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin    = 0;
    si.nMax    = line_count - 1;
    si.nPage  = (format_rect.bottom - format_rect.top) / line_height;
    si.nPos    = y_offset;
    TRACE("SB_VERT, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n",
      si.nMin, si.nMax, si.nPage, si.nPos);
    gxSetScrollInfo(hwndSelf, SB_VERT, &si, TRUE);
  }

  if ((m_dwStyle & GXWS_HSCROLL) && !(flags & EF_HSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize  = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin    = 0;
    si.nMax    = text_width - 1;
    si.nPage  = format_rect.right - format_rect.left;
    si.nPos    = x_offset;
    TRACE("SB_HORZ, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n",
      si.nMin, si.nMax, si.nPage, si.nPos);
    gxSetScrollInfo(hwndSelf, SB_HORZ, &si, TRUE);
  }
}


/*********************************************************************
*
*  EDIT_EM_LineScroll_internal
*
*  Version of EDIT_EM_LineScroll for internal use.
*  It doesn't refuse if ES_MULTILINE is set and assumes that
*  dx is in pixels, dy - in lines.
*
*/
GXBOOL EDITSTATE::EM_LineScroll_internal(GXINT dx, GXINT dy)
{
  GXINT nyoff;
  GXINT x_offset_in_pixels;
  GXINT lines_per_page = (format_rect.bottom - format_rect.top) /
    line_height;

  if (m_dwStyle & GXES_MULTILINE)
  {
    x_offset_in_pixels = x_offset;
  }
  else
  {
    dy = 0;
    x_offset_in_pixels = (short)GXLOWORD(EM_PosFromChar(x_offset, FALSE));
  }

  if (-dx > x_offset_in_pixels)
    dx = -x_offset_in_pixels;
  if (dx > text_width - x_offset_in_pixels)
    dx = text_width - x_offset_in_pixels;
  nyoff = max(0, y_offset + dy);
  if (nyoff >= line_count - lines_per_page)
    nyoff = max(0, line_count - lines_per_page);
  dy = (y_offset - nyoff) * line_height;
  if (dx || dy) {
    GXRECT rc1;
    GXRECT rc;

    y_offset = nyoff;
    if(m_dwStyle & GXES_MULTILINE)
      x_offset += dx;
    else
      x_offset += dx / char_width;

    gxGetClientRect(hwndSelf, &rc1);
    gxIntersectRect(&rc, &rc1, &format_rect);
    gxScrollWindowEx(hwndSelf, -dx, dy,
      NULL, &rc, NULL, NULL, SW_INVALIDATE);
    /* force scroll info update */
    UpdateScrollInfo();
  }
  if (dx && !(flags & EF_HSCROLL_TRACK)) {
    EDIT_NOTIFY_PARENT(EN_HSCROLL);
  }
  if (dy && !(flags & EF_VSCROLL_TRACK)) {
    EDIT_NOTIFY_PARENT(EN_VSCROLL);
  }
  return TRUE;
}

/*********************************************************************
*
*  EM_LINESCROLL
*
*  NOTE: dx is in average character widths, dy - in lines;
*
*/
GXBOOL EDITSTATE::EM_LineScroll(GXINT dx, GXINT dy)
{
  if (!(m_dwStyle & GXES_MULTILINE))
    return FALSE;

  dx *= char_width;
  return EM_LineScroll_internal(dx, dy);
}


/*********************************************************************
*
*  EM_SCROLL
*
*/
GXLRESULT EDITSTATE::EM_Scroll(GXINT action)
{
  GXINT dy;

  if (!(m_dwStyle & GXES_MULTILINE))
    return (GXLRESULT)FALSE;

  dy = 0;

  switch (action) {
  case SB_LINEUP:
    if (y_offset)
      dy = -1;
    break;
  case SB_LINEDOWN:
    if (y_offset < line_count - 1)
      dy = 1;
    break;
  case SB_PAGEUP:
    if (y_offset)
      dy = -(format_rect.bottom - format_rect.top) / line_height;
    break;
  case SB_PAGEDOWN:
    if (y_offset < line_count - 1)
      dy = (format_rect.bottom - format_rect.top) / line_height;
    break;
  default:
    return (GXLRESULT)FALSE;
  }
  if (dy) {
    GXINT vlc = get_vertical_line_count();
    /* check if we are going to move too far */
    if(y_offset + dy > line_count - vlc)
      dy = max(line_count - vlc, 0) - y_offset;

    /* Notification is done in EDIT_EM_LineScroll */
    if(dy) {
      EM_LineScroll(0, dy);
      return MAKELONG(dy, TRUE);
    }

  }
  return (GXLRESULT)FALSE;
}


/*********************************************************************
*
*  EDIT_SetCaretPos
*
*/
void EDITSTATE::SetCaretPos(GXINT pos, GXBOOL after_wrap)
{
  GXLRESULT res = EM_PosFromChar( pos, after_wrap);
  TRACE("%d - %dx%d\n", pos, (short)GXLOWORD(res), (short)GXHIWORD(res));
  gxSetCaretPos((short)GXLOWORD(res), (short)GXHIWORD(res));
}


/*********************************************************************
*
*  EM_SCROLLCARET
*
*/
void EDITSTATE::EM_ScrollCaret()
{
  if (m_dwStyle & GXES_MULTILINE) {
    GXINT l;
    GXINT vlc;
    GXINT ww;
    GXINT cw = char_width;
    GXINT x;
    GXINT dy = 0;
    GXINT dx = 0;

    l = EM_LineFromChar(selection_end);
    x = (short)GXLOWORD(EM_PosFromChar( selection_end, flags & EF_AFTER_WRAP));
    vlc = get_vertical_line_count();
    if (l >= y_offset + vlc)
      dy = l - vlc + 1 - y_offset;
    if (l < y_offset)
      dy = l - y_offset;
    ww = format_rect.right - format_rect.left;
    if (x < format_rect.left)
      dx = x - format_rect.left - ww / HSCROLL_FRACTION / cw * cw;
    if (x > format_rect.right)
      dx = x - format_rect.left - (HSCROLL_FRACTION - 1) * ww / HSCROLL_FRACTION / cw * cw;
    if (dy || dx || (y_offset && (line_count - y_offset < vlc)))
    {
      /* check if we are going to move too far */
      if(x_offset + dx + ww > text_width)
        dx = text_width - ww - x_offset;
      if(dx || dy || (y_offset && (line_count - y_offset < vlc)))
        EM_LineScroll_internal(dx, dy);
    }
  } else {
    GXINT x;
    GXINT goal;
    GXINT format_width;

    x = (short)GXLOWORD(EM_PosFromChar( selection_end, FALSE));
    format_width = format_rect.right - format_rect.left;
    if (x < format_rect.left) {
      goal = format_rect.left + format_width / HSCROLL_FRACTION;
      do {
        x_offset--;
        x = (short)GXLOWORD(EM_PosFromChar( selection_end, FALSE));
      } while ((x < goal) && x_offset);
      /* FIXME: use ScrollWindow() somehow to improve performance */
      UpdateText(NULL, TRUE);
    } else if (x > format_rect.right) {
      GXINT x_last;
      GXINT len = get_text_length();
      goal = format_rect.right - format_width / HSCROLL_FRACTION;
      do {
        x_offset++;
        x = (short)GXLOWORD(EM_PosFromChar( selection_end, FALSE));
        x_last = (short)GXLOWORD(EM_PosFromChar( len, FALSE));
      } while ((x > goal) && (x_last > format_rect.right));
      /* FIXME: use ScrollWindow() somehow to improve performance */
      UpdateText(NULL, TRUE);
    }
  }

  if(flags & EF_FOCUSED)
    SetCaretPos(selection_end, flags & EF_AFTER_WRAP);
}


/*********************************************************************
*
*  EDIT_MoveBackward
*
*/
void EDITSTATE::MoveBackward(GXBOOL extend)
{
  GXINT e = selection_end;

  if (e) {
    e--;
    if ((m_dwStyle & GXES_MULTILINE) && e &&
      (m_pText[e - 1] == '\r') && (m_pText[e] == '\n')) {
        e--;
        if (e && (m_pText[e - 1] == '\r'))
          e--;
    }
  }
  EM_SetSel(extend ? selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveDown_ML
*
*  Only for multi line controls
*  Move the caret one line down, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MoveDown_ML(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXBOOL after_wrap = (flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar( e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = CharFromPos(x, y + line_height, &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveEnd
*
*/
void EDITSTATE::MoveEnd(GXBOOL extend, GXBOOL ctrl)
{
  GXBOOL after_wrap = FALSE;
  GXINT e;

  /* Pass a high value in x to make sure of receiving the end of the line */
  if (!ctrl && (m_dwStyle & GXES_MULTILINE))
    e = CharFromPos(0x3fffffff,
    GXHIWORD(EM_PosFromChar( selection_end, flags & EF_AFTER_WRAP)), &after_wrap);
  else
    e = get_text_length();
  EM_SetSel(extend ? selection_start : e, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveForward
*
*/
void EDITSTATE::MoveForward(GXBOOL extend)
{
  GXINT e = selection_end;

  if (m_pText[e]) {
    e++;
    if ((m_dwStyle & GXES_MULTILINE) && (m_pText[e - 1] == '\r')) {
      if (m_pText[e] == '\n')
        e++;
      else if ((m_pText[e] == '\r') && (m_pText[e + 1] == '\n'))
        e += 2;
    }
  }
  EM_SetSel(extend ? selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveHome
*
*  Home key: move to beginning of line.
*
*/
void EDITSTATE::MoveHome(GXBOOL extend, GXBOOL ctrl)
{
  GXINT e;

  /* Pass the x_offset in x to make sure of receiving the first position of the line */
  if (!ctrl && (m_dwStyle & GXES_MULTILINE))
    e = CharFromPos(-x_offset,
    GXHIWORD(EM_PosFromChar( selection_end, flags & EF_AFTER_WRAP)), NULL);
  else
    e = 0;
  EM_SetSel(extend ? selection_start : e, e, FALSE);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MovePageDown_ML
*
*  Only for multi line controls
*  Move the caret one page down, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MovePageDown_ML(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXBOOL after_wrap = (flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar( e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = CharFromPos(x, y + (format_rect.bottom - format_rect.top),
    &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MovePageUp_ML
*
*  Only for multi line controls
*  Move the caret one page up, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MovePageUp_ML(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXBOOL after_wrap = (flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar( e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = CharFromPos(x,
    y - (format_rect.bottom - format_rect.top),
    &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveUp_ML
*
*  Only for multi line controls
*  Move the caret one line up, on a column with the nearest
*  x coordinate on the screen (might be a different column).
*
*/
void EDITSTATE::MoveUp_ML(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXBOOL after_wrap = (flags & EF_AFTER_WRAP);
  GXLRESULT pos = EM_PosFromChar( e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = CharFromPos(x, y - line_height, &after_wrap);
  if (!extend)
    s = e;
  EM_SetSel(s, e, after_wrap);
  EM_ScrollCaret();
}


/*********************************************************************
*
*  EDIT_MoveWordBackward
*
*/
void EDITSTATE::MoveWordBackward(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXINT l;
  GXINT ll;
  GXINT li;

  l = EM_LineFromChar(e);
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
*  EDIT_MoveWordForward
*
*/
void EDITSTATE::MoveWordForward(GXBOOL extend)
{
  GXINT s = selection_start;
  GXINT e = selection_end;
  GXINT l;
  GXINT ll;
  GXINT li;

  l = EM_LineFromChar(e);
  ll = EM_LineLength(e);
  li = EM_LineIndex(l);
  if (e - li == ll) {
    if ((m_dwStyle & GXES_MULTILINE) && (l != line_count - 1))
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
*  EDIT_PaintText
*
*/
GXINT EDITSTATE::PaintText(GXHDC dc, GXINT x, GXINT y, GXINT line, GXINT col, GXINT count, GXBOOL rev)
{
  GXCOLORREF BkColor;
  GXCOLORREF TextColor;
  GXLOGFONTW underline_font;
  GXHFONT hUnderline = 0;
  GXHFONT old_font = 0;
  GXINT ret;
  GXINT li;
  GXINT BkMode;
  GXSIZE size;

  if (!count)
    return 0;
  BkMode = gxGetBkMode(dc);
  BkColor = gxGetBkColor(dc);
  TextColor = gxGetTextColor(dc);
  if (rev) {
    if (composition_len == 0)
    {
      gxSetBkColor(dc, gxGetSysColor(COLOR_HIGHLIGHT));
      gxSetTextColor(dc, gxGetSysColor(COLOR_HIGHLIGHTTEXT));
      gxSetBkMode(dc, GXOPAQUE);
    }
    else
    {
      GXHFONT current = (GXHFONT)gxGetCurrentObject(dc, GXOBJ_FONT);
      gxGetObjectW(current,sizeof(LOGFONTW), &underline_font);
      underline_font.lfUnderline = TRUE;
      hUnderline = (GXHFONT)gxCreateFontIndirectW(&underline_font);
      old_font = (GXHFONT)gxSelectObject(dc,hUnderline);
    }
  }

  if((! rev || composition_len != 0) && TEST_FLAG(m_crText, 0xff000000)) {
    gxSetTextColor(dc, m_crText);
  }

  li = EM_LineIndex(line);
  if (m_dwStyle & GXES_MULTILINE) {
    ret = (GXINT)GXLOWORD(gxTabbedTextOutW(dc, x, y, m_pText + li + col, count,
      tabs_count, tabs, format_rect.left - x_offset));
  } else {
    GXLPWSTR text = GetPasswordPointer_SL();
    gxTextOutW(dc, x, y, text + li + col, count);
    gxGetTextExtentPoint32W(dc, text + li + col, count, &size);
    ret = size.cx;
    if (m_dwStyle & ES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }

  if (rev) {
    if (composition_len == 0)
    {
      gxSetBkColor(dc, BkColor);
      gxSetTextColor(dc, TextColor);
      gxSetBkMode( dc, BkMode);
    }
    else
    {
      if (old_font)
        gxSelectObject(dc,old_font);
      if (hUnderline)
        gxDeleteObject(hUnderline);
    }
  }
  return ret;
}


/*********************************************************************
*
*  EDIT_PaintLine
*
*/
void EDITSTATE::PaintLine(GXHDC dc, GXINT line, GXBOOL rev)
{
  GXINT s;
  GXINT e;
  GXINT li;
  GXINT ll;
  GXINT x;
  GXINT y;
  GXLRESULT pos;

  if (m_dwStyle & GXES_MULTILINE) {
    GXINT vlc = get_vertical_line_count();

    if ((line < y_offset) || (line > y_offset + vlc) || (line >= line_count))
      return;
  } else if (line)
    return;

  TRACE("line=%d\n", line);

  pos = EM_PosFromChar(EM_LineIndex(line), FALSE);
  x = (short)GXLOWORD(pos);
  y = (short)GXHIWORD(pos);
  li = EM_LineIndex(line);
  ll = EM_LineLength(li);
  s = min(selection_start, selection_end);
  e = max(selection_start, selection_end);
  s = min(li + ll, max(li, s));
  e = min(li + ll, max(li, e));
  if (rev && (s != e) && ((flags & EF_FOCUSED) || (m_dwStyle & ES_NOHIDESEL))) {
    x += PaintText(dc, x, y, line, 0, s - li, FALSE);
    x += PaintText(dc, x, y, line, s - li, e - s, TRUE);
    x += PaintText(dc, x, y, line, e - li, li + ll - e, FALSE);
  } else {
    x += PaintText(dc, x, y, line, 0, ll, FALSE);
  }
}


/*********************************************************************
*
*  EDIT_AdjustFormatRect
*
*  Adjusts the format rectangle for the current font and the
*  current client rectangle.
*
*/
void EDITSTATE::AdjustFormatRect()
{
  GXRECT ClientRect;

  format_rect.right = max(format_rect.right, format_rect.left + char_width);
  if (m_dwStyle & GXES_MULTILINE)
  {
    GXINT fw, vlc, max_x_offset, max_y_offset;

    vlc = get_vertical_line_count();
    format_rect.bottom = format_rect.top + vlc * line_height;

    /* correct x_offset */
    fw = format_rect.right - format_rect.left;
    max_x_offset = text_width - fw;
    if(max_x_offset < 0) max_x_offset = 0;
    if(x_offset > max_x_offset)
      x_offset = max_x_offset;

    /* correct y_offset */
    max_y_offset = line_count - vlc;
    if(max_y_offset < 0) max_y_offset = 0;
    if(y_offset > max_y_offset)
      y_offset = max_y_offset;

    /* force scroll info update */
    UpdateScrollInfo();
  }
  else
    /* Windows doesn't care to fix text placement for SL controls */
    format_rect.bottom = format_rect.top + line_height;

  /* Always stay within the client area */
  gxGetClientRect(hwndSelf, &ClientRect);
  format_rect.bottom = min(format_rect.bottom, ClientRect.bottom);

  if ((m_dwStyle & GXES_MULTILINE) && !(m_dwStyle & ES_AUTOHSCROLL))
    BuildLineDefs_ML(0, get_text_length(), 0, NULL);

  SetCaretPos(selection_end, flags & EF_AFTER_WRAP);
}


/*********************************************************************
*
*  EDIT_SetRectNP
*
*  note:  this is not (exactly) the handler called on EM_SETRECTNP
*    it is also used to set the rect of a single line control
*
*/
void EDITSTATE::SetRectNP(const GXRECT *rc)
{
  LONG_PTR ExStyle;
  GXINT bw, bh;
  ExStyle = gxGetWindowLongPtrW(hwndSelf, GWL_EXSTYLE);

  gxCopyRect(&format_rect, rc);

  if (ExStyle & GXWS_EX_CLIENTEDGE) {
    format_rect.left++;
    format_rect.right--;

    if (format_rect.bottom - format_rect.top
      >= line_height + 2)
    {
      format_rect.top++;
      format_rect.bottom--;
    }
  }
  else if (m_dwStyle & GXWS_BORDER) {
    bw = GetSystemMetrics(SM_CXBORDER) + 1;
    bh = GetSystemMetrics(SM_CYBORDER) + 1;
    format_rect.left += bw;
    format_rect.right -= bw;
    if (format_rect.bottom - format_rect.top
      >= line_height + 2 * bh)
    {
      format_rect.top += bh;
      format_rect.bottom -= bh;
    }
  }

  format_rect.left += left_margin;
  format_rect.right -= right_margin;
  AdjustFormatRect();
}


/*********************************************************************
*
*  EM_CHARFROMPOS
*
*      returns line number (not index) in high-order word of result.
*      NB : Q137805 is unclear about this. GXPOINT * pointer in lParam apply
*      to Richedit, not to the edit control. Original documentation is valid.
*  FIXME: do the specs mean to return -1 if outside client area or
*    if outside formatting rectangle ???
*
*/
GXLRESULT EDITSTATE::EM_CharFromPos(GXINT x, GXINT y)
{
  GXPOINT pt;
  GXRECT rc;
  GXINT index;

  pt.x = x;
  pt.y = y;
  gxGetClientRect(hwndSelf, &rc);
  if (!gxPtInRect(&rc, pt))
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
  flags &= ~EF_USE_SOFTBRK;
  if (add_eol) {
    flags |= EF_USE_SOFTBRK;
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

  if (!(m_dwStyle & GXES_MULTILINE))
    return 0;

  if(is_unicode)
    hLocal = hloc32W;
  else
  {
    if(!hloc32A)
    {
      GXCHAR *textA;
      GXUINT countA, alloc_size;
      TRACE("Allocating 32-bit ANSI alias buffer\n");
      countA = gxWideCharToMultiByte(CP_ACP, 0, m_pText, -1, NULL, 0, NULL, NULL);
      alloc_size = ROUND_TO_GROW(countA);
      if(!(hloc32A = (GXHLOCAL)gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
      {
        ERR("Could not allocate %d bytes for 32-bit ANSI alias buffer\n", alloc_size);
        return 0;
      }
      textA = (GXCHAR*)gxLocalLock(hloc32A);
      gxWideCharToMultiByte(CP_ACP, 0, m_pText, -1, textA, countA, NULL, NULL);
      gxLocalUnlock(hloc32A);
    }
    hLocal = hloc32A;
  }

  flags |= EF_APP_HAS_HANDLE;
  TRACE("Returning %p, LocalSize() = %ld\n", hLocal, LocalSize(hLocal));
  return hLocal;
}


/*********************************************************************
*
*  EM_GETLINE
*
*/
GXINT EDITSTATE::EM_GetLine(GXINT line, GXLPWSTR dst, GXBOOL unicode)
{
  GXLPWSTR src;
  GXINT line_len, dst_len;
  GXINT i;

  if (m_dwStyle & GXES_MULTILINE) {
    if (line >= line_count)
      return 0;
  } else
    line = 0;
  i = EM_LineIndex(line);
  src = m_pText + i;
  line_len = EM_LineLength(i);
  dst_len = *(GXWORD *)dst;
  if(unicode)
  {
    if(dst_len <= line_len)
    {
      memcpy(dst, src, dst_len * sizeof(GXWCHAR));
      return dst_len;
    }
    else /* Append 0 if enough space */
    {
      memcpy(dst, src, line_len * sizeof(GXWCHAR));
      dst[line_len] = 0;
      return line_len;
    }
  }
  else
  {
    GXINT ret = gxWideCharToMultiByte(CP_ACP, 0, src, line_len, (GXLPSTR)dst, dst_len, NULL, NULL);
    if(!ret && line_len) /* Insufficient buffer size */
      return dst_len;
    if(ret < dst_len) /* Append 0 if enough space */
      ((GXLPSTR)dst)[ret] = 0;
    return ret;
  }
}


/*********************************************************************
*
*  EM_GETSEL
*
*/
GXLRESULT EDITSTATE::EM_GetSel(GXLPUINT start, GXLPUINT end)
{
  GXUINT s = selection_start;
  GXUINT e = selection_end;

  ORDER_GXUINT(s, e);
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
void EDITSTATE::EM_ReplaceSel(GXBOOL can_undo, GXLPCWSTR lpsz_replace, GXBOOL send_update, GXBOOL honor_limit)
{
  GXUINT strl = GXSTRLEN(lpsz_replace);
  GXUINT tl = get_text_length();
  GXUINT utl;
  GXUINT s;
  GXUINT e;
  GXUINT i;
  GXUINT size;
  GXLPWSTR p;
  GXHRGN hrgn = 0;
  GXLPWSTR buf = NULL;
  GXUINT bufl = 0;

  TRACE("%s, can_undo %d, send_update %d\n",
    debugstr_w(lpsz_replace), can_undo, send_update);

  s = selection_start;
  e = selection_end;

  if ((s == e) && !strl)
    return;

  ORDER_GXUINT(s, e);

  size = tl - (e - s) + strl;
  if (!size)
    text_width = 0;

  /* Issue the EN_MAXTEXT notification and continue with replacing text
  * such that buffer limit is honored. */
  if ((honor_limit) && (size > buffer_limit)) {
    EDIT_NOTIFY_PARENT(EN_MAXTEXT);
    /* Buffer limit can be smaller than the actual length of text in combobox */
    if (buffer_limit < (tl - (e-s)))
      strl = 0;
    else
      strl = buffer_limit - (tl - (e-s));
  }

  if (!MakeFit(tl - (e - s) + strl))
    return;

  if (e != s) {
    /* there is something to be deleted */
    TRACE("deleting stuff.\n");
    bufl = e - s;
    buf = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (bufl + 1) * sizeof(GXWCHAR));
    if (!buf) return;
    memcpy(buf, m_pText + s, bufl * sizeof(GXWCHAR));
    buf[bufl] = 0; /* ensure 0 termination */
    /* now delete */
    strcpyW(m_pText + s, m_pText + e);
    text_buffer_changed();
  }
  if (strl) {
    /* there is an insertion */
    tl = get_text_length();
    TRACE("inserting stuff (tl %d, strl %d, selstart %d (%s), text %s)\n", tl, strl, s, debugstr_w(m_pText + s), debugstr_w(m_pText));
    for (p = m_pText + tl ; p >= m_pText + s ; p--)
      p[strl] = p[0];
    for (i = 0 , p = m_pText + s ; i < strl ; i++)
      p[i] = lpsz_replace[i];
    if(m_dwStyle & ES_UPPERCASE)
      CharUpperBuffW(p, strl);
    else if(m_dwStyle & ES_LOWERCASE)
      CharLowerBuffW(p, strl);
    text_buffer_changed();
  }
  if (m_dwStyle & GXES_MULTILINE)
  {
    GXINT st = min(selection_start, selection_end);
    GXINT vlc = get_vertical_line_count();

    hrgn = gxCreateRectRgn(0, 0, 0, 0);
    BuildLineDefs_ML(st, st + strl, strl - abs(selection_end - selection_start), hrgn);
    /* if text is too long undo all changes */
    if (honor_limit && !(m_dwStyle & ES_AUTOVSCROLL) && (line_count > vlc)) {
      if (strl)
        strcpyW(m_pText + e, m_pText + e + strl);
      if (e != s)
        for (i = 0 , p = m_pText ; i < e - s ; i++)
          p[i + s] = buf[i];
      text_buffer_changed();
      BuildLineDefs_ML(s, e, abs(selection_end - selection_start) - strl, hrgn);
      strl = 0;
      e = s;
      //hrgn = gxCreateRectRgn(0, 0, 0, 0);
      EDIT_NOTIFY_PARENT(EN_MAXTEXT);
    }
  }
  else {
    GXINT fw = format_rect.right - format_rect.left;
    CalcLineWidth_SL();
    /* remove chars that don't fit */
    if (honor_limit && !(m_dwStyle & ES_AUTOHSCROLL) && (text_width > fw)) {
      while ((text_width > fw) && s + strl >= s) {
        strcpyW(m_pText + s + strl - 1, m_pText + s + strl);
        strl--;
        CalcLineWidth_SL();
      }
      text_buffer_changed();
      EDIT_NOTIFY_PARENT(EN_MAXTEXT);
    }
  }

  if (e != s) {
    if (can_undo) {
      utl = GXSTRLEN(undo_text);
      if (!undo_insert_count && (*undo_text && (s == undo_position))) {
        /* undo-buffer is extended to the right */
        MakeUndoFit(utl + e - s);
        memcpy(undo_text + utl, buf, (e - s)*sizeof(GXWCHAR));
        (undo_text + utl)[e - s] = 0; /* ensure 0 termination */
      } else if (!undo_insert_count && (*undo_text && (e == undo_position))) {
        /* undo-buffer is extended to the left */
        MakeUndoFit(utl + e - s);
        for (p = undo_text + utl ; p >= undo_text ; p--)
          p[e - s] = p[0];
        for (i = 0 , p = undo_text ; i < e - s ; i++)
          p[i] = buf[i];
        undo_position = s;
      } else {
        /* new undo-buffer */
        MakeUndoFit(e - s);
        memcpy(undo_text, buf, (e - s)*sizeof(GXWCHAR));
        undo_text[e - s] = 0; /* ensure 0 termination */
        undo_position = s;
      }
      /* any deletion makes the old insertion-undo invalid */
      undo_insert_count = 0;
    } else
      EM_EmptyUndoBuffer();
  }
  if (strl) {
    if (can_undo) {
      if ((s == undo_position) ||
        ((undo_insert_count) &&
        (s == undo_position + undo_insert_count)))
        /*
        * insertion is new and at delete position or
        * an extension to either left or right
        */
        undo_insert_count += strl;
      else {
        /* new insertion undo */
        undo_position = s;
        undo_insert_count = strl;
        /* new insertion makes old delete-buffer invalid */
        *undo_text = '\0';
      }
    } else
      EM_EmptyUndoBuffer();
  }

  if (bufl) {
    gxHeapFree(gxGetProcessHeap(), 0, buf);
  }

  s += strl;

  /* If text has been deleted and we're right or center aligned then scroll rightward */
  if (m_dwStyle & (ES_RIGHT | ES_CENTER))
  {
    GXINT delta = strl - abs(selection_end - selection_start);

    if (delta < 0 && x_offset)
    {
      if (abs(delta) > x_offset)
        x_offset = 0;
      else
        x_offset += delta;
    }
  }

  EM_SetSel(s, s, FALSE);
  flags |= EF_MODIFIED;
  if (send_update) flags |= EF_UPDATE;
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


  if(m_VarText.IsValid()) {
    m_VarText.ParseW(m_pText, 0);
  }

  if(send_update || (flags & EF_UPDATE))
  {
    flags &= ~EF_UPDATE;
    EDIT_NOTIFY_PARENT(EN_CHANGE);
  }
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
  if (!(m_dwStyle & GXES_MULTILINE))
    return;

  if (!hloc) {
    WARN("called with NULL handle\n");
    return;
  }

  UnlockBuffer(TRUE);

  if(is_unicode)
  {
    if(hloc32A)
    {
      gxLocalFree(hloc32A);
      hloc32A = NULL;
    }
    hloc32W = hloc;
  }
  else
  {
    GXINT countW, countA;
    GXHLOCAL hloc32W_new;
    GXWCHAR *textW;
    GXCHAR *textA;

    countA = (GXINT)gxLocalSize(hloc);
    textA = (GXCHAR*)gxLocalLock(hloc);
    countW = gxMultiByteToWideChar(CP_ACP, 0, textA, countA, NULL, 0);
    if(!(hloc32W_new = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, countW * sizeof(GXWCHAR))))
    {
      ERR("Could not allocate new unicode buffer\n");
      return;
    }
    textW = (GXWCHAR*)gxLocalLock(hloc32W_new);
    gxMultiByteToWideChar(CP_ACP, 0, textA, countA, textW, countW);
    gxLocalUnlock(hloc32W_new);
    gxLocalUnlock(hloc);

    if(hloc32W)
      gxLocalFree(hloc32W);

    hloc32W = hloc32W_new;
    hloc32A = hloc;
  }

  buffer_size = (GXUINT)gxLocalSize(hloc32W)/sizeof(GXWCHAR) - 1;

  flags |= EF_APP_HAS_HANDLE;
  LockBuffer();

  x_offset = y_offset = 0;
  selection_start = selection_end = 0;
  EM_EmptyUndoBuffer();
  flags &= ~EF_MODIFIED;
  flags &= ~EF_UPDATE;
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
  if (!(m_dwStyle & GXES_MULTILINE)) limit = min(limit, 0x7ffffffe);
  buffer_limit = limit;
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
static int calc_min_set_margin_size(GXHDC dc, GXINT left, GXINT right)
{
  GXWCHAR magic_string[] = {'\'','*','*','\'', 0};
  GXSIZE sz;

  gxGetTextExtentPointW(dc, magic_string, sizeof(magic_string)/sizeof(GXWCHAR) - 1, &sz);
  return sz.cx + left + right;
}

void EDITSTATE::EM_SetMargins(GXINT action, GXWORD left, GXWORD right, GXBOOL repaint)
{
  GXTEXTMETRICW tm;
  GXINT default_left_margin  = 0; /* in pixels */
  GXINT default_right_margin = 0; /* in pixels */

  /* Set the default margins depending on the font */
  if (m_hFont && (left == EC_USEFONTINFO || right == EC_USEFONTINFO)) {
    GXHDC dc = gxGetDC(hwndSelf);
    GXHFONT old_font = (GXHFONT)gxSelectObject(dc, m_hFont);
    gxGetTextMetricsW(dc, &tm);
    /* The default margins are only non zero for TrueType or Vector fonts */
    if (tm.tmPitchAndFamily & ( TMPF_VECTOR | TMPF_TRUETYPE )) {
      int min_size;
      GXRECT rc;
      /* This must be calculated more exactly! But how? */
      default_left_margin = tm.tmAveCharWidth / 2;
      default_right_margin = tm.tmAveCharWidth / 2;
      min_size = calc_min_set_margin_size(dc, default_left_margin, default_right_margin);
      gxGetClientRect(hwndSelf, &rc);
      if(rc.right - rc.left < min_size) {
        default_left_margin = left_margin;
        default_right_margin = right_margin;
      }
    }
    gxSelectObject(dc, old_font);
    gxReleaseDC(hwndSelf, dc);
  }

  if (action & EC_LEFTMARGIN) {
    format_rect.left -= left_margin;
    if (left != EC_USEFONTINFO)
      left_margin = left;
    else
      left_margin = default_left_margin;
    format_rect.left += left_margin;
  }

  if (action & EC_RIGHTMARGIN) {
    format_rect.right += right_margin;
    if (right != EC_USEFONTINFO)
      right_margin = right;
    else
      right_margin = default_right_margin;
    format_rect.right -= right_margin;
  }

  if (action & (EC_LEFTMARGIN | EC_RIGHTMARGIN)) {
    AdjustFormatRect();
    if (repaint) UpdateText(NULL, TRUE);
  }

  TRACE("left=%d, right=%d\n", left_margin, right_margin);
}


/*********************************************************************
*
*  EM_SETPASSWORDCHAR
*
*/
void EDITSTATE::EM_SetPasswordChar(GXWCHAR c)
{
  GXLONG dwStyle;

  if (m_dwStyle & GXES_MULTILINE)
    return;

  if (password_char == c)
    return;

  dwStyle = (GXLONG)gxGetWindowLongW( hwndSelf, GWL_STYLE );
  password_char = c;
  if (c) {
    gxSetWindowLongW( hwndSelf, GWL_STYLE, dwStyle | ES_PASSWORD );
    m_dwStyle |= ES_PASSWORD;
  } else {
    gxSetWindowLongW( hwndSelf, GWL_STYLE, dwStyle & ~ES_PASSWORD );
    m_dwStyle &= ~ES_PASSWORD;
  }
  UpdateText(NULL, TRUE);
}


/*********************************************************************
*
*  EM_SETTABSTOPS
*
*/
GXBOOL EDITSTATE::EM_SetTabStops(GXINT count, const GXINT* _tabs)
{
  if (!(m_dwStyle & GXES_MULTILINE))
    return FALSE;
  gxHeapFree(gxGetProcessHeap(), 0, tabs);
  tabs_count = count;
  if (!count)
    tabs = NULL;
  else {
    tabs = (GXLPINT)gxHeapAlloc(gxGetProcessHeap(), 0, count * sizeof(GXINT));
    memcpy(tabs, _tabs, count * sizeof(GXINT));
  }
  return TRUE;
}



/*********************************************************************
*
*  EM_SETWORDBREAKPROC
*
*/
void EDITSTATE::EM_SetWordBreakProc(void *wbp)
{
  if (word_break_proc == wbp)
    return;

  word_break_proc = wbp;

  if ((m_dwStyle & GXES_MULTILINE) && !(m_dwStyle & ES_AUTOHSCROLL)) {
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
  GXINT ulength;
  GXLPWSTR utext;

  /* As per MSDN spec, for a single-line edit control,
  the return value is always TRUE */
  if( m_dwStyle & ES_READONLY )
    return !(m_dwStyle & GXES_MULTILINE);

  ulength = GXSTRLEN(undo_text);

  utext = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (ulength + 1) * sizeof(GXWCHAR));

  strcpyW(utext, undo_text);

  TRACE("before UNDO:insertion length = %d, deletion buffer = %s\n",
    undo_insert_count, debugstr_w(utext));

  EM_SetSel(undo_position, undo_position + undo_insert_count, FALSE);
  EM_EmptyUndoBuffer();
  EM_ReplaceSel(TRUE, utext, TRUE, TRUE);
  EM_SetSel(undo_position, undo_position + undo_insert_count, FALSE);

  /* send the notification after the selection start and end are set */
  EDIT_NOTIFY_PARENT(EN_CHANGE);
  EM_ScrollCaret();
  gxHeapFree(gxGetProcessHeap(), 0, utext);

  TRACE("after UNDO:insertion length = %d, deletion buffer = %s\n",
    undo_insert_count, debugstr_w(undo_text));
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
  return (flags & EF_DIALOGMODE);
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
  if(m_dwStyle & ES_READONLY)
    return;

  gxOpenClipboard(hwndSelf);
  if ((hsrc = (GXHGLOBAL)gxGetClipboardData(CF_UNICODETEXT))) {
    src = (GXLPWSTR)gxGlobalLock(hsrc);
    EM_ReplaceSel(TRUE, src, TRUE, TRUE);
    gxGlobalUnlock((GXHLOCAL)hsrc);
  }
  else if (m_dwStyle & ES_PASSWORD) {
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
  GXINT s = min(selection_start, selection_end);
  GXINT e = max(selection_start, selection_end);
  GXHGLOBAL hdst;
  GXLPWSTR dst;
  GXDWORD len;

  if (e == s) return;

  len = e - s;
  hdst = gxGlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(GXWCHAR));
  dst = (GXLPWSTR)gxGlobalLock(hdst);
  memcpy(dst, m_pText + s, len * sizeof(GXWCHAR));
  dst[len] = 0; /* ensure 0 termination */
  TRACE("%s\n", debugstr_w(dst));
  gxGlobalUnlock(hdst);
  gxOpenClipboard(hwndSelf);
  gxEmptyClipboard();
  gxSetClipboardData(CF_UNICODETEXT, (GXHANDLE)hdst);
  gxCloseClipboard();
}


/*********************************************************************
*
*  WM_CLEAR
*
*/
inline void EDITSTATE::WM_Clear()
{
  /* Protect read-only edit control from modification */
  if(m_dwStyle & ES_READONLY)
    return;

  EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
}


/*********************************************************************
*
*  WM_CUT
*
*/
inline void EDITSTATE::WM_Cut()
{
  WM_Copy();
  WM_Clear();
}


/*********************************************************************
*
*  WM_CHAR
*
*/
GXLRESULT EDITSTATE::WM_Char(GXWCHAR c)
{
  GXBOOL control = GetKeyState(VK_CONTROL) & 0x8000;

  switch (c)
  {
  case '\r':
    /* If it's not a multiline edit box, it would be ignored below.
    * For multiline edit without ES_WANTRETURN, we have to make a
    * special case.
    */
    if ((m_dwStyle & GXES_MULTILINE) && !(m_dwStyle & ES_WANTRETURN))
      if (IsInsideDialog())
        break;
  case '\n':
    if (m_dwStyle & GXES_MULTILINE) {
      if (m_dwStyle & ES_READONLY) {
        MoveHome(FALSE, FALSE);
        MoveDown_ML(FALSE);
      } else {
        static const GXWCHAR cr_lfW[] = {'\r','\n',0};
        EM_ReplaceSel(TRUE, cr_lfW, TRUE, TRUE);
      }
    }
    break;
  case '\t':
    if ((m_dwStyle & GXES_MULTILINE) && !(m_dwStyle & ES_READONLY))
    {
      static const GXWCHAR tabW[] = {'\t',0};
      if (IsInsideDialog())
        break;
      EM_ReplaceSel(TRUE, tabW, TRUE, TRUE);
    }
    break;
  case VK_BACK:
    if (!(m_dwStyle & ES_READONLY) && !control) {
      if (selection_start != selection_end)
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
    if (!(m_dwStyle & ES_PASSWORD))
      gxSendMessageW(hwndSelf, WM_COPY, 0, 0);
    break;
  case 0x16: /* ^V */
    if (!(m_dwStyle & ES_READONLY))
      gxSendMessageW(hwndSelf, WM_PASTE, 0, 0);
    break;
  case 0x18: /* ^X */
    if (!((m_dwStyle & ES_READONLY) || (m_dwStyle & ES_PASSWORD)))
      gxSendMessageW(hwndSelf, WM_CUT, 0, 0);
    break;
  case 0x1A: /* ^Z */
    if (!(m_dwStyle & ES_READONLY))
      gxSendMessageW(hwndSelf, WM_UNDO, 0, 0);
    break;

  default:
    /*If Edit control style is ES_NUMBER allow users to key in only numeric values*/
    if( (m_dwStyle & ES_NUMBER) && !( c >= '0' && c <= '9') )
      break;

    if (!(m_dwStyle & ES_READONLY) && (c >= ' ') && (c != 127)) {
      GXWCHAR str[2];
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
*  WM_COMMAND
*
*/
void EDITSTATE::WM_Command(GXINT code, GXINT id, GXHWND control)
{
  if (code || control)
    return;

  switch (id) {
  case GXEM_UNDO:
    gxSendMessageW(hwndSelf, GXWM_UNDO, 0, 0);
    break;
  case GXWM_CUT:
    gxSendMessageW(hwndSelf, GXWM_CUT, 0, 0);
    break;
  case GXWM_COPY:
    gxSendMessageW(hwndSelf, GXWM_COPY, 0, 0);
    break;
  case GXWM_PASTE:
    gxSendMessageW(hwndSelf, GXWM_PASTE, 0, 0);
    break;
  case GXWM_CLEAR:
    gxSendMessageW(hwndSelf, GXWM_CLEAR, 0, 0);
    break;
  case GXEM_SETSEL:
    EM_SetSel(0, (GXUINT)-1, FALSE);
    EM_ScrollCaret();
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
*    (as we do in EDIT_WM_Command()).
*
*/
void EDITSTATE::WM_ContextMenu(GXINT x, GXINT y)
{
  GXHMENU menu = gxLoadMenuW(user32_module, L"EDITMENU");
  GXHMENU popup = gxGetSubMenu(menu, 0);
  GXUINT start = selection_start;
  GXUINT end = selection_end;

  ORDER_GXUINT(start, end);

  /* undo */
  gxEnableMenuItem(popup, 0, MF_BYPOSITION | (EM_CanUndo() && !(m_dwStyle & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* cut */
  gxEnableMenuItem(popup, 2, MF_BYPOSITION | ((end - start) && !(m_dwStyle & ES_PASSWORD) && !(m_dwStyle & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* copy */
  gxEnableMenuItem(popup, 3, MF_BYPOSITION | ((end - start) && !(m_dwStyle & ES_PASSWORD) ? MF_ENABLED : MF_GRAYED));
  /* paste */
  gxEnableMenuItem(popup, 4, MF_BYPOSITION | (IsClipboardFormatAvailable(CF_UNICODETEXT) && !(m_dwStyle & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* delete */
  gxEnableMenuItem(popup, 5, MF_BYPOSITION | ((end - start) && !(m_dwStyle & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* select all */
  gxEnableMenuItem(popup, 7, MF_BYPOSITION | (start || (end != get_text_length()) ? MF_ENABLED : MF_GRAYED));

  if (x == -1 && y == -1) /* passed via VK_APPS press/release */
  {
    GXRECT rc;
    /* Windows places the menu at the edit's center in this case */
    //WIN_GetRectangles( hwndSelf, COORDS_SCREEN, NULL, &rc );
    gxGetWindowRect(hwndSelf, &rc);
    x = rc.left + (rc.right - rc.left) / 2;
    y = rc.top + (rc.bottom - rc.top) / 2;
  }

  if (!(flags & EF_FOCUSED))
    gxSetFocus(hwndSelf);

  gxTrackPopupMenu(popup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hwndSelf, NULL);
  gxDestroyMenu(menu);
}


/*********************************************************************
*
*  WM_GETTEXT
*
*/
GXINT EDITSTATE::WM_GetText(GXINT count, GXLPWSTR dst, GXBOOL unicode) const
{
  if(!count) return 0;

  if(unicode)
  {
    lstrcpynW(dst, m_pText, count);
    return GXSTRLEN(dst);
  }
  else
  {
    GXLPSTR textA = (GXLPSTR)dst;
    if (!gxWideCharToMultiByte(CP_ACP, 0, m_pText, -1, textA, count, NULL, NULL))
      textA[count - 1] = 0; /* ensure 0 termination */
    return (GXINT)strlen(textA);
  }
}

/*********************************************************************
*
*  EDIT_CheckCombo
*
*/
GXBOOL EDITSTATE::CheckCombo(GXUINT msg, GXINT key)
{
  GXHWND hLBox = hwndListBox;
  GXHWND hCombo;
  GXBOOL bDropped;
  int  nEUI;

  if (!hLBox)
    return FALSE;

  hCombo   = gxGetParent(hwndSelf);
  bDropped = TRUE;
  nEUI     = 0;

  TRACE_(combo)("[%p]: handling msg %x (%x)\n", hwndSelf, msg, key);

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
      gxSendMessageW(hCombo, CB_SHOWDROPDOWN, bDropped ? FALSE : TRUE, 0);
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
GXLRESULT EDITSTATE::WM_KeyDown(GXINT key)
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
    if ((m_dwStyle & GXES_MULTILINE) && (key == VK_UP))
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
    if ((m_dwStyle & GXES_MULTILINE) && (key == VK_DOWN))
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
    if (m_dwStyle & GXES_MULTILINE)
      MovePageUp_ML(shift);
    else
      CheckCombo(WM_KEYDOWN, key);
    break;
  case VK_NEXT:
    if (m_dwStyle & GXES_MULTILINE)
      MovePageDown_ML(shift);
    else
      CheckCombo(WM_KEYDOWN, key);
    break;
  case VK_DELETE:
    if (!(m_dwStyle & ES_READONLY) && !(shift && control)) {
      if (selection_start != selection_end) {
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
      if (!(m_dwStyle & ES_READONLY))
        WM_Paste();
    } else if (control)
      WM_Copy();
    break;
  case VK_RETURN:
    /* If the edit doesn't want the return send a message to the default object */
    if( ! (m_dwStyle & GXES_MULTILINE) || ! (m_dwStyle & ES_WANTRETURN))
    {
      GXDWORD dw;

      if ( ! IsInsideDialog()) { 
        break;
      }

      if (control) {
        break;
      }

      dw = (GXDWORD)gxSendMessageW(hwndParent, DM_GETDEFID, 0, 0);
      if (GXHIWORD(dw) == DC_HASDEFID)
      {
        GXHWND hwDefCtrl = gxGetDlgItem(hwndParent, GXLOWORD(dw));
        if (hwDefCtrl)
        {
          gxSendMessageW(hwndParent, WM_NEXTDLGCTL, (GXWPARAM)hwDefCtrl, TRUE);
          gxPostMessageW(hwDefCtrl, WM_KEYDOWN, VK_RETURN, 0);
        }
      }
    }
    break;
  case VK_ESCAPE:
    if ((m_dwStyle & GXES_MULTILINE) && IsInsideDialog())
      gxPostMessageW(hwndParent, WM_CLOSE, 0, 0);
    break;
  case VK_TAB:
    if ((m_dwStyle & GXES_MULTILINE) && IsInsideDialog())
      gxSendMessageW(hwndParent, WM_NEXTDLGCTL, shift, 0);
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
  flags &= ~EF_FOCUSED;
  gxDestroyCaret();
  if(!(m_dwStyle & ES_NOHIDESEL))
    InvalidateText(selection_start, selection_end);
  EDIT_NOTIFY_PARENT(EN_KILLFOCUS);
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
  GXINT s;
  GXINT e = selection_end;
  GXINT l;
  GXINT li;
  GXINT ll;

  bCaptureState = TRUE;
  gxSetCapture(hwndSelf);

  l = EM_LineFromChar(e);
  li = EM_LineIndex(l);
  ll = EM_LineLength(e);
  s = li + CallWordBreakProc(li, e - li, ll, WB_LEFT);
  e = li + CallWordBreakProc(li, e - li, ll, WB_RIGHT);
  EM_SetSel(s, e, FALSE);
  EM_ScrollCaret();
  region_posx = region_posy = 0;
  gxSetTimer(hwndSelf, 0, 100, NULL);
  return 0;
}


/*********************************************************************
*
*  WM_LBUTTONDOWN
*
*/
GXLRESULT EDITSTATE::WM_LButtonDown(GXDWORD keys, GXINT x, GXINT y)
{
  GXINT e;
  GXBOOL after_wrap;

  bCaptureState = TRUE;
  gxSetCapture(hwndSelf);
  ConfinePoint(&x, &y);
  e = CharFromPos(x, y, &after_wrap);
  EM_SetSel((keys & MK_SHIFT) ? selection_start : e, e, after_wrap);
  EM_ScrollCaret();
  region_posx = region_posy = 0;
  gxSetTimer(hwndSelf, 0, 100, NULL);

  if (!(flags & EF_FOCUSED))
    gxSetFocus(hwndSelf);

  return 0;
}


/*********************************************************************
*
*  WM_LBUTTONUP
*
*/
GXLRESULT EDITSTATE::WM_LButtonUp()
{
  if (bCaptureState) {
    gxKillTimer(hwndSelf, 0);
    if (gxGetCapture() == hwndSelf) gxReleaseCapture();
  }
  bCaptureState = FALSE;
  return 0;
}


/*********************************************************************
*
*  WM_MBUTTONDOWN
*
*/
GXLRESULT EDITSTATE::WM_MButtonDown()
{
  gxSendMessageW(hwndSelf, WM_PASTE, 0, 0);
  return 0;
}


/*********************************************************************
*
*  WM_MOUSEMOVE
*
*/
GXLRESULT EDITSTATE::WM_MouseMove(GXINT x, GXINT y)
{
  GXINT e;
  GXBOOL after_wrap;
  GXINT prex, prey;

  /* If the mouse has been captured by process other than the edit control itself,
  * the windows edit controls will not select the strings with mouse move.
  */
  if (!bCaptureState || gxGetCapture() != hwndSelf)
    return 0;

  /*
  *  FIXME: gotta do some scrolling if outside client
  *    area.  Maybe reset the timer ?
  */
  prex = x; prey = y;
  ConfinePoint(&x, &y);
  region_posx = (prex < x) ? -1 : ((prex > x) ? 1 : 0);
  region_posy = (prey < y) ? -1 : ((prey > y) ? 1 : 0);
  e = CharFromPos(x, y, &after_wrap);
  EM_SetSel(selection_start, e, after_wrap);
  SetCaretPos(selection_end, flags & EF_AFTER_WRAP);
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
  GXINT i;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXRECT rc;
  GXRECT rcClient;
  GXRECT rcLine;
  GXRECT rcRgn;
  GXHBRUSH brush = NULL;
  GXHBRUSH old_brush;
  GXINT bw, bh;
  GXBOOL rev = bEnableState &&
    ((flags & EF_FOCUSED) ||
    (m_dwStyle & ES_NOHIDESEL));
  dc = hdc ? hdc : gxBeginPaint(hwndSelf, &ps);

  gxGetClientRect(hwndSelf, &rcClient);

  /* get the background brush */
  if(TEST_FLAG(m_crBack, 0xff000000)) {
    GXSetBkColor(dc, 0);
  }
  else {
    brush = NotifyCtlColor(dc);
  }

  /* paint the border and the background */
  gxIntersectClipRect(dc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

  if(m_dwStyle & GXWS_BORDER) {
    bw = gxGetSystemMetrics(GXSM_CXBORDER);
    bh = gxGetSystemMetrics(GXSM_CYBORDER);
    rc = rcClient;
    if(m_dwStyle & GXES_MULTILINE) {
      if(m_dwStyle & GXWS_HSCROLL) rc.bottom+=bh;
      if(m_dwStyle & GXWS_VSCROLL) rc.right+=bw;
    }

    /* Draw the frame. Same code as in nonclient.c */
    old_brush = (GXHBRUSH)gxSelectObject(dc, gxGetSysColorBrush(GXCOLOR_WINDOWFRAME));
    gxPatBlt(dc, rc.left, rc.top, rc.right - rc.left, bh, GXPATCOPY);
    gxPatBlt(dc, rc.left, rc.top, bw, rc.bottom - rc.top, GXPATCOPY);
    gxPatBlt(dc, rc.left, rc.bottom - 1, rc.right - rc.left, -bw, GXPATCOPY);
    gxPatBlt(dc, rc.right - 1, rc.top, -bw, rc.bottom - rc.top, GXPATCOPY);
    gxSelectObject(dc, old_brush);

    /* Keep the border clean */
    gxIntersectClipRect(dc, rc.left+bw, rc.top+bh,
      max(rc.right-bw, rc.left+bw), max(rc.bottom-bh, rc.top+bh));
  }

  gxGetClipBox(dc, &rc);

  if(brush) {
    gxFillRect(dc, &rc, brush);
  }
  else if(TEST_FLAG(m_crBack, 0xff000000)) {
    LPGXWNDCANVAS pCanvas = GXGetWndCanvas(dc);
    pCanvas->FillRect(&rc, m_crBack);
  }

  gxIntersectClipRect(dc, format_rect.left, format_rect.top, format_rect.right, format_rect.bottom);
  if (m_dwStyle & GXES_MULTILINE) {
    rc = rcClient;
    gxIntersectClipRect(dc, rc.left, rc.top, rc.right, rc.bottom);
  }
  if (m_hFont) {
    old_font = (GXHFONT)gxSelectObject(dc, m_hFont);
  }

  if (!bEnableState) {
    gxSetTextColor(dc, gxGetSysColor(GXCOLOR_GRAYTEXT));
  }

  gxGetClipBox(dc, &rcRgn);
  if (m_dwStyle & GXES_MULTILINE) {
    GXINT vlc = get_vertical_line_count();
    for (i = y_offset ; i <= min(y_offset + vlc, y_offset + line_count - 1) ; i++) {
      GetLineRect(i, 0, -1, &rcLine);
      if (gxIntersectRect(&rc, &rcRgn, &rcLine))
        PaintLine(dc, i, rev);
    }
  }
  else {
    GetLineRect(0, 0, -1, &rcLine);
    if(gxIntersectRect(&rc, &rcRgn, &rcLine)) {
      PaintLine(dc, 0, rev);
    }
  }
  if(m_hFont) {
    gxSelectObject(dc, old_font);
  }

  if (!hdc) {
    gxEndPaint(hwndSelf, &ps);
  }
}


/*********************************************************************
*
*  WM_SETFOCUS
*
*/
void EDITSTATE::WM_SetFocus()
{
  flags |= EF_FOCUSED;

  if (!(m_dwStyle & GXES_NOHIDESEL))
    InvalidateText(selection_start, selection_end);

  /* single line edit updates itself */
  if (!(m_dwStyle & GXES_MULTILINE))
  {
    GXHDC hdc = gxGetDC(hwndSelf);
    WM_Paint(hdc);
    gxReleaseDC(hwndSelf, hdc);
  }

  gxCreateCaret(hwndSelf, 0, 1, line_height);
  SetCaretPos(selection_end, flags & EF_AFTER_WRAP);
  gxShowCaret(hwndSelf);
  EDIT_NOTIFY_PARENT(EN_SETFOCUS);
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

  m_hFont = font;
  dc = gxGetDC(hwndSelf);
  if (font)
    old_font = (GXHFONT)gxSelectObject(dc, font);
  gxGetTextMetricsW(dc, &tm);
  line_height = tm.tmHeight;
  char_width = tm.tmAveCharWidth;
  if (font)
    gxSelectObject(dc, old_font);
  gxReleaseDC(hwndSelf, dc);

  /* Reset the format rect and the margins */
  gxGetClientRect(hwndSelf, &clientRect);
  SetRectNP(&clientRect);
  EM_SetMargins(EC_LEFTMARGIN | EC_RIGHTMARGIN,
    EC_USEFONTINFO, EC_USEFONTINFO, FALSE);

  if (m_dwStyle & GXES_MULTILINE)
    BuildLineDefs_ML(0, get_text_length(), 0, NULL);
  else
    CalcLineWidth_SL();

  if (redraw)
    UpdateText(NULL, TRUE);

  if (flags & EF_FOCUSED) {
    gxDestroyCaret();
    gxCreateCaret(hwndSelf, 0, 1, line_height);
    SetCaretPos(selection_end, flags & EF_AFTER_WRAP);
    gxShowCaret(hwndSelf);
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
void EDITSTATE::WM_SetText(GXLPCWSTR text, GXBOOL unicode)
{
  GXLPWSTR textW = NULL;
  if (!unicode && text)
  {
    LPCSTR textA = (LPCSTR)text;
    GXINT countW = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
    textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, countW * sizeof(GXWCHAR));
    if (textW)
      gxMultiByteToWideChar(CP_ACP, 0, textA, -1, textW, countW);
    text = textW;
  }

  if (flags & EF_UPDATE)
    /* fixed this bug once; complain if we see it about to happen again. */
    ERR("SetSel may generate UPDATE message whose handler may reset "
    "selection.\n");

  EM_SetSel(0, (GXUINT)-1, FALSE);
  if (text) 
  {
    TRACE("%s\n", debugstr_w(text));
    EM_ReplaceSel(FALSE, text, FALSE, FALSE);
    if(!unicode)
      gxHeapFree(gxGetProcessHeap(), 0, textW);
  } 
  else 
  {
    TRACE("<NULL>\n");
    EM_ReplaceSel(FALSE, empty_stringW, FALSE, FALSE);
  }
  x_offset = 0;
  flags &= ~EF_MODIFIED;
  EM_SetSel(0, 0, FALSE);

  /* Send the notification after the selection start and end have been set
  * edit control doesn't send notification on WM_SETTEXT
  * if it is multiline, or it is part of combobox
  */
  if( !((m_dwStyle & GXES_MULTILINE) || hwndListBox))
  {
    EDIT_NOTIFY_PARENT(EN_UPDATE);
    EDIT_NOTIFY_PARENT(EN_CHANGE);
  }
  EM_ScrollCaret();
  UpdateScrollInfo();    
}


/*********************************************************************
*
*  WM_SIZE
*
*/
void EDITSTATE::WM_Size(GXUINT action, GXINT width, GXINT height)
{
  if ((action == SIZE_MAXIMIZED) || (action == SIZE_RESTORED)) {
    GXRECT rc;
    TRACE("width = %d, height = %d\n", width, height);
    gxSetRect(&rc, 0, 0, width, height);
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
* See also EDIT_WM_NCCreate
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
GXLRESULT EDITSTATE::WM_StyleChanged(GXWPARAM which, const STYLESTRUCT* pStyle)
{
  if (GWL_STYLE == which) {
    GXDWORD style_change_mask;
    GXDWORD new_style;
    /* Only a subset of changes can be applied after the control
    * has been created.
    */
    style_change_mask = ES_UPPERCASE | ES_LOWERCASE |
      ES_NUMBER;
    if (m_dwStyle & GXES_MULTILINE)
      style_change_mask |= ES_WANTRETURN;

    new_style = pStyle->styleNew & style_change_mask;

    /* Number overrides lowercase overrides uppercase (at least it
    * does in Win95).  However I'll bet that ES_NUMBER would be
    * invalid under Win 3.1.
    */
    if (new_style & ES_NUMBER) {
      ; /* do not override the ES_NUMBER */
    }  else if (new_style & ES_LOWERCASE) {
      new_style &= ~ES_UPPERCASE;
    }

    m_dwStyle = (m_dwStyle & ~style_change_mask) | new_style;
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
GXLRESULT EDITSTATE::WM_SysKeyDown(GXINT key, GXDWORD key_data)
{
  if ((key == VK_BACK) && (key_data & 0x2000)) {
    if (EM_CanUndo())
      EM_Undo();
    return 0;
  } else if (key == VK_UP || key == VK_DOWN) {
    if (CheckCombo(WM_SYSKEYDOWN, key))
      return 0;
  }
  return gxDefWindowProcW(hwndSelf, WM_SYSKEYDOWN, key, key_data);
}


/*********************************************************************
*
*  WM_TIMER
*
*/
void EDITSTATE::WM_Timer()
{
  if (region_posx < 0) {
    MoveBackward(TRUE);
  } else if (region_posx > 0) {
    MoveForward(TRUE);
  }
  /*
  *  FIXME: gotta do some vertical scrolling here, like
  *    EDIT_EM_LineScroll(hwnd, 0, 1);
  */
}

/*********************************************************************
*
*  WM_HSCROLL
*
*/
GXLRESULT EDITSTATE::WM_HScroll(GXINT action, GXINT pos)
{
  GXINT dx;
  GXINT fw;

  if (!(m_dwStyle & GXES_MULTILINE))
    return 0;

  if (!(m_dwStyle & ES_AUTOHSCROLL))
    return 0;

  dx = 0;
  fw = format_rect.right - format_rect.left;
  switch (action) {
  case SB_LINELEFT:
    TRACE("SB_LINELEFT\n");
    if (x_offset)
      dx = -char_width;
    break;
  case SB_LINERIGHT:
    TRACE("SB_LINERIGHT\n");
    if (x_offset < text_width)
      dx = char_width;
    break;
  case SB_PAGELEFT:
    TRACE("SB_PAGELEFT\n");
    if (x_offset)
      dx = -fw / HSCROLL_FRACTION / char_width * char_width;
    break;
  case SB_PAGERIGHT:
    TRACE("SB_PAGERIGHT\n");
    if (x_offset < text_width)
      dx = fw / HSCROLL_FRACTION / char_width * char_width;
    break;
  case SB_LEFT:
    TRACE("SB_LEFT\n");
    if (x_offset)
      dx = -x_offset;
    break;
  case SB_RIGHT:
    TRACE("SB_RIGHT\n");
    if (x_offset < text_width)
      dx = text_width - x_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    flags |= EF_HSCROLL_TRACK;
    if(m_dwStyle & GXWS_HSCROLL)
      dx = pos - x_offset;
    else
    {
      GXINT fw, new_x;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      /* Assume default scroll range 0-100 */
      fw = format_rect.right - format_rect.left;
      new_x = pos * (text_width - fw) / 100;
      dx = text_width ? (new_x - x_offset) : 0;
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    flags &= ~EF_HSCROLL_TRACK;
    if(gxGetWindowLongW( hwndSelf, GWL_STYLE ) & GXWS_HSCROLL)
      dx = pos - x_offset;
    else
    {
      GXINT fw, new_x;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      /* Assume default scroll range 0-100 */
      fw = format_rect.right - format_rect.left;
      new_x = pos * (text_width - fw) / 100;
      dx = text_width ? (new_x - x_offset) : 0;
    }
    if (!dx) {
      /* force scroll info update */
      UpdateScrollInfo();
      EDIT_NOTIFY_PARENT(EN_HSCROLL);
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
      if(gxGetWindowLongW( hwndSelf, GWL_STYLE ) & GXWS_HSCROLL)
        ret = gxGetScrollPos(hwndSelf, SB_HORZ);
      else
      {
        /* Assume default scroll range 0-100 */
        GXINT fw = format_rect.right - format_rect.left;
        ret = text_width ? x_offset * 100 / (text_width - fw) : 0;
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
    GXINT fw = format_rect.right - format_rect.left;
    /* check if we are going to move too far */
    if(x_offset + dx + fw > text_width)
      dx = text_width - fw - x_offset;
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
GXLRESULT EDITSTATE::WM_VScroll(GXINT action, GXINT pos)
{
  GXINT dy;

  if (!(m_dwStyle & GXES_MULTILINE))
    return 0;

  if (!(m_dwStyle & ES_AUTOVSCROLL))
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
    dy = -y_offset;
    break;
  case SB_BOTTOM:
    TRACE("SB_BOTTOM\n");
    dy = line_count - 1 - y_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    flags |= EF_VSCROLL_TRACK;
    if(m_dwStyle & GXWS_VSCROLL)
      dy = pos - y_offset;
    else
    {
      /* Assume default scroll range 0-100 */
      GXINT vlc, new_y;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      vlc = get_vertical_line_count();
      new_y = pos * (line_count - vlc) / 100;
      dy = line_count ? (new_y - y_offset) : 0;
      TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
        line_count, y_offset, pos, dy);
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    flags &= ~EF_VSCROLL_TRACK;
    if(m_dwStyle & GXWS_VSCROLL)
      dy = pos - y_offset;
    else
    {
      /* Assume default scroll range 0-100 */
      GXINT vlc, new_y;
      /* Sanity check */
      if(pos < 0 || pos > 100) return 0;
      vlc = get_vertical_line_count();
      new_y = pos * (line_count - vlc) / 100;
      dy = line_count ? (new_y - y_offset) : 0;
      TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
        line_count, y_offset, pos, dy);
    }
    if (!dy)
    {
      /* force scroll info update */
      UpdateScrollInfo();
      EDIT_NOTIFY_PARENT(EN_VSCROLL);
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
      if(gxGetWindowLongW( hwndSelf, GWL_STYLE ) & GXWS_VSCROLL)
        ret = gxGetScrollPos(hwndSelf, SB_VERT);
      else
      {
        /* Assume default scroll range 0-100 */
        GXINT vlc = get_vertical_line_count();
        ret = line_count ? y_offset * 100 / (line_count - vlc) : 0;
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
  return MAKELONG(WM_VScroll(EM_GETTHUMB, 0),
    WM_HScroll(EM_GETTHUMB, 0));
}


/********************************************************************
* 
* The Following code is to handle inline editing from IMEs
*/

void EDITSTATE::GetCompositionStr(GXHIMC hIMC, GXLPARAM CompFlag)
{
  GXLONG buflen;
  GXLPWSTR lpCompStr = NULL;
  GXLPSTR lpCompStrAttr = NULL;
  GXDWORD dwBufLenAttr;

  buflen = gxImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);

  if (buflen < 0)
  {
    return;
  }

  lpCompStr = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(),0,buflen + sizeof(GXWCHAR));
  if (!lpCompStr)
  {
    ERR("Unable to allocate IME CompositionString\n");
    return;
  }

  if (buflen)
    gxImmGetCompositionStringW(hIMC, GCS_COMPSTR, lpCompStr, buflen);
  lpCompStr[buflen/sizeof(GXWCHAR)] = 0;

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
      lpCompStrAttr = (GXLPSTR)gxHeapAlloc(gxGetProcessHeap(),0,dwBufLenAttr+1);
      if (!lpCompStrAttr)
      {
        ERR("Unable to allocate IME Attribute String\n");
        gxHeapFree(gxGetProcessHeap(),0,lpCompStr);
        return;
      }
      gxImmGetCompositionStringW(hIMC,GCS_COMPATTR, lpCompStrAttr, 
        dwBufLenAttr);
      lpCompStrAttr[dwBufLenAttr] = 0;
    }
    else
      lpCompStrAttr = NULL;
  }

  /* check for change in composition start */
  if (selection_end < composition_start)
    composition_start = selection_end;

  /* replace existing selection string */
  selection_start = composition_start;

  if (composition_len > 0)
    selection_end = composition_start + composition_len;
  else
    selection_end = selection_start;

  EM_ReplaceSel(FALSE, lpCompStr, TRUE, TRUE);
  composition_len = abs(composition_start - selection_end);

  selection_start = composition_start;
  selection_end = selection_start + composition_len;

  gxHeapFree(gxGetProcessHeap(),0,lpCompStrAttr);
  gxHeapFree(gxGetProcessHeap(),0,lpCompStr);
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

  lpResultStr = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(),0, buflen+sizeof(GXWCHAR));
  if (!lpResultStr)
  {
    ERR("Unable to alloc buffer for IME string\n");
    return;
  }

  gxImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpResultStr, buflen);
  lpResultStr[buflen/sizeof(GXWCHAR)] = 0;

  /* check for change in composition start */
  if (selection_end < composition_start)
    composition_start = selection_end;

  selection_start = composition_start;
  selection_end = composition_start + composition_len;
  EM_ReplaceSel(TRUE, lpResultStr, TRUE, TRUE);
  composition_start = selection_end;
  composition_len = 0;

  gxHeapFree(gxGetProcessHeap(),0,lpResultStr);
}

void EDITSTATE::ImeComposition(GXHWND hwnd, GXLPARAM CompFlag)
{
  GXHIMC hIMC;
  int cursor;

  if (composition_len == 0 && selection_start != selection_end)
  {
    EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
    composition_start = selection_end;
  }

  hIMC = gxImmGetContext(hwnd);
  if (!hIMC)
    return;

  if (CompFlag & GCS_RESULTSTR)
    GetResultStr(hIMC);
  if (CompFlag & GCS_COMPSTR)
    GetCompositionStr(hIMC, CompFlag);
  cursor = gxImmGetCompositionStringW(hIMC, GCS_CURSORPOS, 0, 0);
  gxImmReleaseContext(hwnd, hIMC);
  SetCaretPos(selection_start + cursor, flags & EF_AFTER_WRAP);
}


/*********************************************************************
*
*  WM_NCCREATE
*
* See also EDIT_WM_StyleChanged
*/
GXLRESULT EDITSTATE::WM_NCCreate(GXHWND hwnd, GXLPCREATESTRUCTW lpcs, GXBOOL unicode)
{
  EDITSTATE *es;
  GXUINT alloc_size;

  TRACE("Creating %s edit control, style = %08x\n",
    unicode ? "Unicode" : "ANSI", lpcs->style);

  if (!(es = (EDITSTATE*)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, sizeof(*es))))
    return FALSE;
  gxSetWindowLongPtrW( hwnd, 0, (GXLONG_PTR)es );

  /*
  *      Note: since the EDITSTATE has not been fully initialized yet,
  *            we can't use any API calls that may send
  *            WM_XXX messages before WM_NCCREATE is completed.
  */

  es->is_unicode = unicode;
  es->m_dwStyle = lpcs->style;

  es->bEnableState = !(es->m_dwStyle & GXWS_DISABLED);

  es->m_crText = 0;
  es->m_crBack = 0;

  es->hwndSelf = hwnd;
  /* Save parent, which will be notified by EN_* messages */
  es->hwndParent = lpcs->hwndParent;

  //if (es->style & ES_COMBO)
  //   es->hwndListBox = GetDlgItem(es->hwndParent, ID_CB_LISTBOX);

  /* FIXME: should we handle changes to GXWS_EX_RIGHT style after creation? */
  if (lpcs->dwExStyle & GXWS_EX_RIGHT) es->m_dwStyle |= ES_RIGHT;

  /* Number overrides lowercase overrides uppercase (at least it
  * does in Win95).  However I'll bet that ES_NUMBER would be
  * invalid under Win 3.1.
  */
  if (es->m_dwStyle & ES_NUMBER) {
    ; /* do not override the ES_NUMBER */
  }  else if (es->m_dwStyle & ES_LOWERCASE) {
    es->m_dwStyle &= ~ES_UPPERCASE;
  }
  if (es->m_dwStyle & GXES_MULTILINE) {
    es->buffer_limit = BUFLIMIT_INITIAL;
    if (es->m_dwStyle & GXWS_VSCROLL)
      es->m_dwStyle |= ES_AUTOVSCROLL;
    if (es->m_dwStyle & GXWS_HSCROLL)
      es->m_dwStyle |= ES_AUTOHSCROLL;
    es->m_dwStyle &= ~ES_PASSWORD;
    if ((es->m_dwStyle & ES_CENTER) || (es->m_dwStyle & ES_RIGHT)) {
      /* Confirmed - RIGHT overrides CENTER */
      if (es->m_dwStyle & ES_RIGHT)
        es->m_dwStyle &= ~ES_CENTER;
      es->m_dwStyle &= ~GXWS_HSCROLL;
      es->m_dwStyle &= ~ES_AUTOHSCROLL;
    }
  } else {
    es->buffer_limit = BUFLIMIT_INITIAL;
    if ((es->m_dwStyle & ES_RIGHT) && (es->m_dwStyle & ES_CENTER))
      es->m_dwStyle &= ~ES_CENTER;
    es->m_dwStyle &= ~GXWS_HSCROLL;
    es->m_dwStyle &= ~GXWS_VSCROLL;
    if (es->m_dwStyle & ES_PASSWORD)
      es->password_char = '*';
  }

  alloc_size = ROUND_TO_GROW((es->buffer_size + 1) * sizeof(GXWCHAR));
  if(!(es->hloc32W = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
    goto cleanup;
  es->buffer_size = (GXUINT)gxLocalSize(es->hloc32W)/sizeof(GXWCHAR) - 1;

  if (!(es->undo_text = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, (es->buffer_size + 1) * sizeof(GXWCHAR))))
    goto cleanup;
  es->undo_buffer_size = es->buffer_size;

  if (es->m_dwStyle & GXES_MULTILINE)
    if (!(es->first_line_def = (LINEDEF*)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, sizeof(LINEDEF))))
      goto cleanup;
  es->line_count = 1;

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
  if (lpcs->dwExStyle & GXWS_EX_CLIENTEDGE)
    es->m_dwStyle &= ~GXWS_BORDER;
  else if (es->m_dwStyle & GXWS_BORDER)
    gxSetWindowLongW(hwnd, GWL_STYLE, es->m_dwStyle & ~GXWS_BORDER);

  return TRUE;

cleanup:
  gxSetWindowLongPtrW(es->hwndSelf, 0, 0);
  gxHeapFree(gxGetProcessHeap(), 0, es->first_line_def);
  gxHeapFree(gxGetProcessHeap(), 0, es->undo_text);
  if (es->hloc32W) gxLocalFree(es->hloc32W);
  gxHeapFree(gxGetProcessHeap(), 0, es);
  return FALSE;
}


/*********************************************************************
*
*  WM_CREATE
*
*/
GXLRESULT EDITSTATE::WM_Create(GXLPCWSTR name)
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
  gxGetClientRect(hwndSelf, &clientRect);
  SetRectNP(&clientRect);

  if (name && *name) {
    EM_ReplaceSel(FALSE, name, FALSE, FALSE);
    /* if we insert text to the editline, the text scrolls out
    * of the window, as the caret is placed after the insert
    * pos normally; thus we reset selection... to 0 and
    * update caret
    */
    selection_start = selection_end = 0;
    /* Adobe Photoshop does NOT like this. and MSDN says that EN_CHANGE
    * Messages are only to be sent when the USER does something to
    * change the contents. So I am removing this EN_CHANGE
    *
    * EDIT_NOTIFY_PARENT(EN_CHANGE);
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
  m_VarText.Free();

  if(m_hFont) {
    gxDeleteObject(m_hFont);
    m_hFont = NULL;
  }

  if (hloc32W) {
    gxLocalFree(hloc32W);
  }
  if (hloc32A) {
    gxLocalFree(hloc32A);
  }
  pc = first_line_def;
  while (pc)
  {
    pp = pc->next;
    gxHeapFree(gxGetProcessHeap(), 0, pc);
    pc = pp;
  }

  gxSetWindowLongPtrW(hwndSelf, 0, 0 );
  gxHeapFree(gxGetProcessHeap(), 0, undo_text);
  gxHeapFree(gxGetProcessHeap(), 0, this);

  return 0;
}

GXLRESULT EDITSTATE::SetVariable(MOVariable* pVariable)
{
  if(GXUI::CtrlBase::SetDataVariable(hwndSelf, m_VarText, pVariable))
  {
    WM_SetText(pVariable->ToStringW(), TRUE);
  }
  return 0;
}

//GXHRESULT EDITSTATE::OnKnock(KNOCKACTION* pKnock)
//{
//  ASSERT(m_VarText.IsValid());
//#ifdef ENABLE_DATAPOOL_WATCHER
//  if(pKnock->pSponsor != &m_VarText && m_VarText.GetName() == pKnock->Name)// &&
//    //! m_VarText.InlGetDataPool()->IsKnocking(&m_VarText))
//  {
//    WM_SetText(m_VarText.ToStringW(), TRUE);
//    //gxInvalidateRect(hwndSelf, NULL, FALSE);
//  }
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
//  return 0;
//}

GXVOID EDITSTATE::OnImpulse(LPCDATAIMPULSE pImpulse)
{
  CLBREAK;
}

GXBOOL EDITSTATE::SolveDefinition( const GXDefinitionArrayW& aDefinitions )
{
  for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
    it != aDefinitions.end(); ++it)
  {
    if(it->Name == L"BackColor") {
      m_crBack = DlgXM::GetColorFromMarkW(it->Value);
    }
    else if(it->Name == L"TextColor") {
      m_crText = DlgXM::GetColorFromMarkW(it->Value);
    }
  }
 
  return TRUE;
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
static GXLRESULT EditWndProc_common( GXHWND hwnd, GXUINT msg, GXWPARAM wParam, GXLPARAM lParam, GXBOOL unicode )
{
  EDITSTATE* es = (EDITSTATE *)gxGetWindowLongPtrW( hwnd, 0 );
  EDITSTATE* pThis = es; // 只是为了使用pThis这个名字
  GXLRESULT result = 0;

  TRACE("hwnd=%p msg=%x (%s) wparam=%lx lparam=%lx\n", hwnd, msg, SPY_GetMsgName(msg, hwnd), wParam, lParam);

  if (!es && msg != GXWM_NCCREATE)
    return DefWindowProcT(hwnd, msg, wParam, lParam, unicode);

  if (es && (msg != WM_NCDESTROY)) es->LockBuffer();

  switch (msg) {
  case GXEM_GETSEL:
    result = es->EM_GetSel((GXLPUINT)wParam, (GXLPUINT)lParam);
    break;

  case GXEM_SETSEL:
    es->EM_SetSel((GXUINT)wParam, (GXUINT)lParam, FALSE);
    es->EM_ScrollCaret();
    result = 1;
    break;

  case GXEM_GETRECT:
    if (lParam)
      gxCopyRect((GXLPRECT)lParam, &es->format_rect);
    break;

  case GXEM_SETRECT:
    if ((es->m_dwStyle & GXES_MULTILINE) && lParam) {
      es->SetRectNP((GXLPRECT)lParam);
      es->UpdateText(NULL, TRUE);
    }
    break;

  case GXEM_SETRECTNP:
    if ((es->m_dwStyle & GXES_MULTILINE) && lParam)
      es->SetRectNP((GXLPRECT)lParam);
    break;

  case GXEM_SCROLL:
    result = es->EM_Scroll((GXINT)wParam);
    break;

  case GXEM_LINESCROLL:
    result = (GXLRESULT)es->EM_LineScroll((GXINT)wParam, (GXINT)lParam);
    break;

  case GXEM_SCROLLCARET:
    es->EM_ScrollCaret();
    result = 1;
    break;

  case GXEM_GETMODIFY:
    result = ((es->flags & EF_MODIFIED) != 0);
    break;

  case GXEM_SETMODIFY:
    if (wParam)
      es->flags |= EF_MODIFIED;
    else
      es->flags &= ~(EF_MODIFIED | EF_UPDATE);  /* reset pending updates */
    break;

  case GXEM_GETLINECOUNT:
    result = (es->m_dwStyle & GXES_MULTILINE) ? es->line_count : 1;
    break;

  case GXEM_LINEINDEX:
    result = (GXLRESULT)es->EM_LineIndex((GXINT)wParam);
    break;

  case GXEM_SETHANDLE:
    es->EM_SetHandle((GXHLOCAL)wParam);
    break;

  case GXEM_GETHANDLE:
    result = (GXLRESULT)es->EM_GetHandle();
    break;

  case GXEM_GETTHUMB:
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

  case GXWM_SOLVEDEFINITION:
    es->SolveDefinition(*(reinterpret_cast<GXDefinitionArrayW*>(lParam)));
    break;

  case GXEM_LINELENGTH:
    result = (GXLRESULT)es->EM_LineLength((GXINT)wParam);
    break;

  case GXEM_REPLACESEL:
    {
      GXLPWSTR textW;

      if(unicode)
        textW = (GXLPWSTR)lParam;
      else
      {
        GXLPSTR textA = (GXLPSTR)lParam;
        GXINT countW = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
        if (!(textW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, countW * sizeof(GXWCHAR)))) break;
        gxMultiByteToWideChar(CP_ACP, 0, textA, -1, textW, countW);
      }

      es->EM_ReplaceSel((GXBOOL)wParam, textW, TRUE, TRUE);
      result = 1;

      if(!unicode)
        gxHeapFree(gxGetProcessHeap(), 0, textW);
      break;
    }

  case GXEM_GETLINE:
    result = (GXLRESULT)es->EM_GetLine((GXINT)wParam, (GXLPWSTR)lParam, unicode);
    break;

  case GXEM_SETLIMITTEXT:
    es->EM_SetLimitText((GXUINT)wParam);
    break;

  case GXEM_CANUNDO:
    result = (GXLRESULT)es->EM_CanUndo();
    break;

  case GXEM_UNDO:
  case GXWM_UNDO:
    result = (GXLRESULT)es->EM_Undo();
    break;

  case GXEM_FMTLINES:
    result = (GXLRESULT)es->EM_FmtLines((GXBOOL)wParam);
    break;

  case GXEM_LINEFROMCHAR:
    result = (GXLRESULT)es->EM_LineFromChar((GXINT)wParam);
    break;

  case GXEM_SETTABSTOPS:
    result = (GXLRESULT)es->EM_SetTabStops((GXINT)wParam, (LPINT)lParam);
    break;

  case GXEM_SETPASSWORDCHAR:
    {
      GXWCHAR charW = 0;

      if(unicode)
        charW = (GXWCHAR)wParam;
      else
      {
        GXCHAR charA = (GXCHAR)wParam;
        gxMultiByteToWideChar(CP_ACP, 0, &charA, 1, &charW, 1);
      }

      es->EM_SetPasswordChar(charW);
      break;
    }

  case GXEM_EMPTYUNDOBUFFER:
    es->EM_EmptyUndoBuffer();
    break;

  case GXEM_GETFIRSTVISIBLELINE:
    result = (es->m_dwStyle & GXES_MULTILINE) ? es->y_offset : es->x_offset;
    break;

  case GXEM_SETREADONLY:
    {
      GXDWORD old_style = es->m_dwStyle;

      if (wParam) {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) | ES_READONLY );
        es->m_dwStyle |= ES_READONLY;
      } else {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) & ~ES_READONLY );
        es->m_dwStyle &= ~ES_READONLY;
      }

      if (old_style ^ es->m_dwStyle)
        gxInvalidateRect(es->hwndSelf, NULL, TRUE);

      result = 1;
      break;
    }

  case GXEM_SETWORDBREAKPROC:
    es->EM_SetWordBreakProc((void *)lParam);
    break;

  case GXEM_GETWORDBREAKPROC:
    result = (GXLRESULT)es->word_break_proc;
    break;

  case GXEM_GETPASSWORDCHAR:
    {
      if(unicode)
        result = es->password_char;
      else
      {
        GXWCHAR charW = es->password_char;
        GXCHAR charA = 0;
        gxWideCharToMultiByte(CP_ACP, 0, &charW, 1, &charA, 1, NULL, NULL);
        result = charA;
      }
      break;
    }

  case GXEM_SETMARGINS:
    es->EM_SetMargins((GXINT)wParam, GXLOWORD(lParam), GXHIWORD(lParam), TRUE);
    break;

  case GXEM_GETMARGINS:
    result = MAKELONG(es->left_margin, es->right_margin);
    break;

  case GXEM_GETLIMITTEXT:
    result = es->buffer_limit;
    break;

  case GXEM_POSFROMCHAR:
    if ((GXINT)wParam >= es->get_text_length()) result = -1;
    else result = es->EM_PosFromChar( (GXINT)wParam, FALSE);
    break;

  case GXEM_CHARFROMPOS:
    result = es->EM_CharFromPos((short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

    /* End of the EM_ messages which were in numerical order; what order
    * are these in?  vaguely alphabetical?
    */

  case GXWM_NCCREATE:
    result = EDITSTATE::WM_NCCreate(hwnd, (GXLPCREATESTRUCTW)lParam, unicode);
    break;

  case GXWM_NCDESTROY:
    result = es->WM_NCDestroy();
    es = NULL;
    break;

  case GXWM_GETDLGCODE:
    result = DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;

    if (es->m_dwStyle & GXES_MULTILINE)
      result |= DLGC_WANTALLKEYS;

    if (lParam)
    {
      es->flags |= EF_DIALOGMODE;

      if (((LPMSG)lParam)->message == WM_KEYDOWN)
      {
        int vk = (int)((LPMSG)lParam)->wParam;

        if (es->hwndListBox) {
          if (vk == VK_RETURN || vk == VK_ESCAPE) {
            if (gxSendMessageW(gxGetParent(hwnd), CB_GETDROPPEDSTATE, 0, 0)) {
              result |= DLGC_WANTMESSAGE;
            }
          }
        }
      }
    }
    break;

  case GXWM_IME_CHAR:
    if (!unicode)
    {
      GXWCHAR charW;
      GXCHAR  strng[2];

      strng[0] = (GXCHAR)(wParam >> 8);
      strng[1] = (GXCHAR)(wParam & 0xff);
      if (strng[0]) gxMultiByteToWideChar(CP_ACP, 0, strng, 2, &charW, 1);
      else gxMultiByteToWideChar(CP_ACP, 0, &strng[1], 1, &charW, 1);
      result = es->WM_Char(charW);
      break;
    }
    /* fall through */
  case GXWM_CHAR:
    {
      GXWCHAR charW;

      if(unicode)
        charW = (GXWCHAR)wParam;
      else
      {
        GXCHAR charA = (GXCHAR)wParam;
        gxMultiByteToWideChar(CP_ACP, 0, &charA, 1, &charW, 1);
      }

      if (es->hwndListBox)
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
          es->WM_Char((GXWCHAR)(wParam >> 10) + 0xd800);
          es->WM_Char((wParam & 0x03ff) + 0xdc00);
        }
        else {
          es->WM_Char((GXWCHAR)wParam);
        }
      }
      return 0;
    }
    break;

  case GXWM_CLEAR:
    es->WM_Clear();
    break;

  case GXWM_COMMAND:
    es->WM_Command(GXHIWORD(wParam), GXLOWORD(wParam), (GXHWND)lParam);
    break;

  case GXWM_CONTEXTMENU:
    es->WM_ContextMenu((short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_COPY:
    es->WM_Copy();
    break;

  case GXWM_CREATE:
    if(unicode)
      result = es->WM_Create(((GXLPCREATESTRUCTW)lParam)->lpszName);
    else
    {
      LPCSTR nameA = ((GXLPCREATESTRUCTA)lParam)->lpszName;
      GXLPWSTR nameW = NULL;
      if(nameA)
      {
        GXINT countW = gxMultiByteToWideChar(CP_ACP, 0, nameA, -1, NULL, 0);
        if((nameW = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, countW * sizeof(GXWCHAR))))
          gxMultiByteToWideChar(CP_ACP, 0, nameA, -1, nameW, countW);
      }
      result = es->WM_Create(nameW);
      gxHeapFree(gxGetProcessHeap(), 0, nameW);
    }
    break;

  case GXWM_CUT:
    es->WM_Cut();
    break;

  case GXWM_ENABLE:
    es->bEnableState = (GXBOOL) wParam;
    es->UpdateText(NULL, TRUE);
    break;

  case GXWM_ERASEBKGND:
    /* we do the proper erase in EDIT_WM_Paint */
    result = 1;
    break;

  case GXWM_GETFONT:
    result = (GXLRESULT)es->m_hFont;
    break;

  case GXWM_GETTEXT:
    result = (GXLRESULT)es->WM_GetText((GXINT)wParam, (GXLPWSTR)lParam, unicode);
    break;

  case GXWM_GETTEXTLENGTH:
    if (unicode) result = es->get_text_length();
    else result = gxWideCharToMultiByte( CP_ACP, 0, es->m_pText, es->get_text_length(),
      NULL, 0, NULL, NULL );
    break;

  case GXWM_HSCROLL:
    result = es->WM_HScroll(GXLOWORD(wParam), (short)GXHIWORD(wParam));
    break;

  case GXWM_KEYDOWN:
    result = es->WM_KeyDown((GXINT)wParam);
    break;

  case GXWM_KILLFOCUS:
    result = es->WM_KillFocus();
    break;

  case GXWM_LBUTTONDBLCLK:
    result = es->WM_LButtonDblClk();
    break;

  case GXWM_LBUTTONDOWN:
    result = es->WM_LButtonDown((GXDWORD)wParam, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_LBUTTONUP:
    result = es->WM_LButtonUp();
    break;

  case GXWM_MBUTTONDOWN:
    result = es->WM_MButtonDown();
    break;

  case GXWM_MOUSEMOVE:
    result = es->WM_MouseMove((short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    es->WM_Paint((GXHDC)wParam);
    break;

  case GXWM_PASTE:
    es->WM_Paste();
    break;

  case GXWM_SETFOCUS:
    es->WM_SetFocus();
    break;

  case GXWM_SETFONT:
    es->WM_SetFont((GXHFONT)wParam, GXLOWORD(lParam) != 0);
    break;

  case GXWM_SETREDRAW:
    /* FIXME: actually set an internal flag and behave accordingly */
    break;

  case GXWM_SETTEXT:
    es->WM_SetText((GXLPCWSTR)lParam, unicode);
    result = TRUE;
    break;

  case GXWM_SIZE:
    es->WM_Size((GXUINT)wParam, GXLOWORD(lParam), GXHIWORD(lParam));
    break;

  case GXWM_STYLECHANGED:
    result = es->WM_StyleChanged(wParam, (const STYLESTRUCT *)lParam);
    break;

  case GXWM_STYLECHANGING:
    result = 0; /* See EDIT_WM_StyleChanged */
    break;

  case GXWM_SYSKEYDOWN:
    result = es->WM_SysKeyDown((GXINT)wParam, (GXDWORD)lParam);
    break;

  case GXWM_TIMER:
    es->WM_Timer();
    break;

  case GXWM_VSCROLL:
    result = es->WM_VScroll(GXLOWORD(wParam), (short)GXHIWORD(wParam));
    break;

  case GXWM_MOUSEWHEEL:
    {
      int gcWheelDelta = 0;
      GXUINT pulScrollLines = 3;
      SystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0, &pulScrollLines, 0);

      if (wParam & (MK_SHIFT | MK_CONTROL)) {
        result = gxDefWindowProcW(hwnd, msg, wParam, lParam);
        break;
      }
      gcWheelDelta -= GET_WHEEL_DELTA_WPARAM(wParam);
      if (abs(gcWheelDelta) >= WHEEL_DELTA && pulScrollLines)
      {
        int cLineScroll= (int) min((GXUINT) es->line_count, pulScrollLines);
        cLineScroll *= (gcWheelDelta / WHEEL_DELTA);
        result = es->EM_LineScroll(0, cLineScroll);
      }
    }
    break;


    /* IME messages to make the edit control IME aware */
  case GXWM_IME_SETCONTEXT:
    break;

  case GXWM_IME_STARTCOMPOSITION:
    es->composition_start = es->selection_end;
    es->composition_len = 0;
    break;

  case GXWM_IME_COMPOSITION:
    es->ImeComposition(hwnd, lParam);
    break;

  case GXWM_IME_ENDCOMPOSITION:
    if (es->composition_len > 0)
    {
      es->EM_ReplaceSel(TRUE, empty_stringW, TRUE, TRUE);
      es->selection_end = es->selection_start;
      es->composition_len= 0;
    }
    break;

  case GXWM_IME_COMPOSITIONFULL:
    break;

  case GXWM_IME_SELECT:
    break;

  case GXWM_IME_CONTROL:
    break;

  case GXWM_DATAPOOLOPERATION:
    if(wParam == DPO_SETVARIABLE) {
      return pThis->SetVariable((MOVariable*)lParam);
    }
    else return -1;

  case GXWM_IMPULSE:
    {
      pThis->OnImpulse((LPCDATAIMPULSE)lParam);
      //pThis->OnKnock((KNOCKACTION*)lParam);
    }
    break;

  default:
    result = DefWindowProcT(hwnd, msg, wParam, lParam, unicode);
    break;
  }

  if (gxIsWindow(hwnd) && es) es->UnlockBuffer(FALSE);

  TRACE("hwnd=%p msg=%x (%s) -- 0x%08lx\n", hwnd, msg, SPY_GetMsgName(msg, hwnd), result);

  return result;
}


//*********************************************************************
// edit class descriptor
//

static GXLRESULT __stdcall EditWndProcW_1_3_30(GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  return EditWndProc_common(hWnd, (GXUINT)uMsg, wParam, lParam, TRUE);
}

GXWNDCLASSEX WndClassEx_GXUIEdit_1_3_30 = { sizeof(GXWNDCLASSEX), GXCS_CLASSDC, EditWndProcW_1_3_30, 0L, sizeof(EDITSTATE *),
  (GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_IBEAM), NULL, NULL,
  GXUICLASSNAME_EDIT, NULL };
