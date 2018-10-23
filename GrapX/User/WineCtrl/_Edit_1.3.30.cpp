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

#include <GrapX.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GXImm.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <GrapX/WineComm.h>

#undef TRACE_
#define TRACE_(_TYPE)  TRACEW
#define SPY_GetMsgName(_X, _Y)  ""
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
//#include "wine/unicode.h"
//#include "controls.h"
//#include "user_private.h"
//#include "wine/debug.h"
//
//WINE_DEFAULT_DEBUG_CHANNEL(edit);
//WINE_DECLARE_DEBUG_CHANNEL(combo);
//WINE_DECLARE_DEBUG_CHANNEL(relay);

#define BUFLIMIT_INITIAL    30000   /* initial buffer size */
#define GROWLENGTH    32  /* buffers granularity in bytes: must be power of 2 */
#define ROUND_TO_GROW(size)  (((size) + (GROWLENGTH - 1)) & ~(GROWLENGTH - 1))
#define HSCROLL_FRACTION  3  /* scroll window by 1/3 width */

/*
 *  extra flags for EDITSTATE.flags field
 */
#define EF_MODIFIED    0x0001  /* text has been modified */
#define EF_FOCUSED    0x0002  /* we have input focus */
#define EF_UPDATE    0x0004  /* notify parent of changed state */
#define EF_VSCROLL_TRACK  0x0008  /* don't SetScrollPos() since we are tracking the thumb */
#define EF_HSCROLL_TRACK  0x0010  /* don't SetScrollPos() since we are tracking the thumb */
#define EF_AFTER_WRAP    0x0080  /* the caret is displayed after the last character of a
             wrapped line, instead of in front of the next character */
#define EF_USE_SOFTBRK    0x0100  /* Enable soft breaks in text. */
#define EF_APP_HAS_HANDLE       0x0200  /* Set when an app sends EM_[G|S]ETHANDLE.  We are in sole control of
                                           the text buffer if this is clear. */
#define EF_DIALOGMODE           0x0400  /* Indicates that we are inside a dialog window */

typedef enum
 {
   END_0 = 0,     /* line ends with terminating '\0' character */
   END_WRAP,      /* line is wrapped */
   END_HARD,      /* line ends with a hard return '\r\n' */
   END_SOFT,      /* line ends with a soft return '\r\r\n' */
   END_RICH       /* line ends with a single '\n' */
 } LINE_END;

typedef struct tagLINEDEF {
  GXINT     length;       /* bruto length of a line in bytes */
  GXINT     net_length;   /* netto length of a line in visible characters */
  LINE_END  ending;
  GXINT     width;        /* width of the line in pixels */
  GXINT     index;        /* line index into the buffer */
  struct tagLINEDEF *next;
} LINEDEF;

typedef struct
{
  GXBOOL is_unicode;    /* how the control was created */
  GXLPWSTR text;      /* the actual contents of the control */
  GXUINT text_length;               /* cached length of text buffer (in WCHARs) - use get_text_length() to retrieve */
  GXUINT buffer_size;    /* the size of the buffer in characters */
  GXUINT buffer_limit;    /* the maximum size to which the buffer may grow in characters */
  GXHFONT font;      /* NULL means standard system font */
  GXINT x_offset;      /* scroll offset  for multi lines this is in pixels
                    for single lines it's in characters */
  GXINT line_height;    /* height of a screen line in pixels */
  GXINT char_width;      /* average character width in pixels */
  GXDWORD style;      /* sane version of wnd->dwStyle */
  GXWORD flags;      /* flags that are not in es->style or wnd->flags (EF_XXX) */
  GXINT undo_insert_count;    /* number of characters inserted in sequence */
  GXUINT undo_position;    /* character index of the insertion and deletion */
  GXLPWSTR undo_text;    /* deleted text */
  GXUINT undo_buffer_size;    /* size of the deleted text buffer */
  GXINT selection_start;    /* == selection_end if no selection */
  GXINT selection_end;    /* == current caret position */
  GXWCHAR password_char;    /* == 0 if no password char, and for multi line controls */
  GXINT left_margin;    /* in pixels */
  GXINT right_margin;    /* in pixels */
  GXRECT format_rect;
  GXINT text_width;      /* width of the widest line in pixels for multi line controls
                      and just line width for single line controls  */
  GXINT region_posx;    /* Position of cursor relative to region: */
  GXINT region_posy;    /* -1: to left, 0: within, 1: to right */
  void *word_break_proc;    /* 32-bit word break proc: ANSI or Unicode */
  GXINT line_count;      /* number of lines */
  GXINT y_offset;      /* scroll offset in number of lines */
  GXBOOL bCaptureState;     /* flag indicating whether mouse was captured */
  GXBOOL bEnableState;    /* flag keeping the enable state */
  GXHWND hwndSelf;      /* the our window handle */
  GXHWND hwndParent;    /* Handle of parent for sending EN_* messages.
                      Even if parent will change, EN_* messages
                      should be sent to the first parent. */
  GXHWND hwndListBox;    /* handle of ComboBox's listbox or NULL */

  /*
  *  only for multi line controls
  */
  GXINT lock_count;      /* amount of re-entries in the EditWndProc */
  GXINT tabs_count;
  GXLPINT tabs;
  LINEDEF *first_line_def;  /* linked list of (soft) linebreaks */
  GXHLOCAL hloc32W;      /* our unicode local memory block */
  GXHLOCAL hloc32A;      /* alias for ANSI control receiving EM_GETHANDLE
                      or EM_SETHANDLE */
  /*
  * IME Data
  */
  GXUINT composition_len;   /* length of composition, 0 == no composition */
  int composition_start;  /* the character position for the composition */
} EDITSTATE;


#define SWAP_GXUINT32(x,y) do { GXUINT temp = (GXUINT)(x); (x) = (GXUINT)(y); (y) = temp; } while(0)
#define ORDER_GXUINT(x,y) do { if ((GXUINT)(y) < (GXUINT)(x)) SWAP_GXUINT32((x),(y)); } while(0)

/* used for disabled or read-only edit control */
#define EDIT_NOTIFY_PARENT(es, wNotifyCode) \
  do \
  { /* Notify parent which has created this edit control */ \
      TRACE("notification " #wNotifyCode " sent to hwnd=%p\n", es->hwndParent); \
      gxSendMessageW(es->hwndParent, WM_COMMAND, \
         MAKEWPARAM(gxGetWindowLongPtrW((es->hwndSelf),GWLP_ID), wNotifyCode), \
         (GXLPARAM)(es->hwndSelf)); \
  } while(0)

static const GXWCHAR empty_stringW[] = {0};

/*********************************************************************
 *
 *  EM_CANUNDO
 *
 */
static inline GXBOOL EDIT_EM_CanUndo(const EDITSTATE *es)
{
  return (es->undo_insert_count || GXSTRLEN(es->undo_text));
}


/*********************************************************************
 *
 *  EM_EMPTYUNDOBUFFER
 *
 */
static inline void EDIT_EM_EmptyUndoBuffer(EDITSTATE *es)
{
  es->undo_insert_count = 0;
  *es->undo_text = '\0';
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

static GXHBRUSH EDIT_NotifyCtlColor(EDITSTATE *es, GXHDC hdc)
{
  GXHBRUSH hBrush;
  GXUINT msg;

  if ( get_app_version() >= 0x40000 && (!es->bEnableState || (es->style & ES_READONLY)))
    msg = GXWM_CTLCOLORSTATIC;
  else
    msg = GXWM_CTLCOLOREDIT;

  /* why do we notify to es->hwndParent, and we send this one to gxGetParent()? */
  hBrush = (GXHBRUSH)gxSendMessageW(gxGetParent(es->hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)es->hwndSelf);
  if (!hBrush)
    hBrush = (GXHBRUSH)gxDefWindowProcW(gxGetParent(es->hwndSelf), msg, (GXWPARAM)hdc, (GXLPARAM)es->hwndSelf);
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
static GXINT EDIT_WordBreakProc(GXLPWSTR s, GXINT index, GXINT count, GXINT action)
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
static GXINT EDIT_CallWordBreakProc(EDITSTATE *es, GXINT start, GXINT index, GXINT count, GXINT action)
{
  GXINT ret;

  if (es->word_break_proc)
  {
    if(es->is_unicode)
    {
      GXEDITWORDBREAKPROCW wbpW = (GXEDITWORDBREAKPROCW)es->word_break_proc;

      TRACE_(relay)(_CLTEXT("(UNICODE wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n"),
        es->word_break_proc, debugstr_wn(es->text + start, count), index, count, action);
      ret = wbpW(es->text + start, index, count, action);
    }
    else
    {
      EDITWORDBREAKPROCA wbpA = (EDITWORDBREAKPROCA)es->word_break_proc;
      GXINT countA;
      GXCHAR* textA;

      countA = gxWideCharToMultiByte(CP_ACP, 0, es->text + start, count, NULL, 0, NULL, NULL);
      textA = (GXCHAR*)gxHeapAlloc(gxGetProcessHeap(), 0, countA);
      gxWideCharToMultiByte(CP_ACP, 0, es->text + start, count, textA, countA, NULL, NULL);
      TRACE_(relay)(_CLTEXT("(ANSI wordbrk=%p,str=%s,idx=%d,cnt=%d,act=%d)\n"),
        es->word_break_proc, debugstr_an(textA, countA), index, countA, action);
      ret = wbpA(textA, index, countA, action);
      gxHeapFree(gxGetProcessHeap(), 0, textA);
    }
  }
  else
    ret = EDIT_WordBreakProc(es->text + start, index, count, action);

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
static void EDIT_BuildLineDefs_ML(EDITSTATE *es, GXINT istart, GXINT iend, GXINT delta, GXHRGN hrgn)
{
  GXHDC dc;
  GXHFONT old_font = 0;
  GXLPWSTR current_position, cp;
  GXINT fw;
  LINEDEF *current_line;
  LINEDEF *previous_line;
  LINEDEF *start_line;
  GXINT line_index = 0, nstart_line = 0, nstart_index = 0;
  GXINT line_count = es->line_count;
  GXINT orig_net_length;
  GXRECT rc;

  if (istart == iend && delta == 0)
    return;

  dc = gxGetDC(es->hwndSelf);
  if (es->font)
    old_font = (GXHFONT)gxSelectObject(dc, es->font);

  previous_line = NULL;
  current_line = es->first_line_def;

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
    gxReleaseDC(es->hwndSelf, dc);
    return;
  }

  /* Remember start of modifications in order to calculate update region */
  nstart_line = line_index;
  nstart_index = current_line->index;

  /* We must start to reformat from the previous line since the modifications
  * may have caused the line to wrap upwards. */
  if (!(es->style & GXES_AUTOHSCROLL) && line_index > 0)
  {
    line_index--;
    current_line = previous_line;
  }
  start_line = current_line;

  fw = es->format_rect.right - es->format_rect.left;
  current_position = es->text + current_line->index;
  do {
    if (current_line != start_line)
    {
      if (!current_line || current_line->index + delta > current_position - es->text)
      {
        /* The buffer has been expanded, create a new line and
        insert it into the link list */
        LINEDEF *new_line = (LINEDEF*)gxHeapAlloc(gxGetProcessHeap(), 0, sizeof(LINEDEF));
        new_line->next = previous_line->next;
        previous_line->next = new_line;
        current_line = new_line;
        es->line_count++;
      }
      else if (current_line->index + delta < current_position - es->text)
      {
        /* The previous line merged with this line so we delete this extra entry */
        previous_line->next = current_line->next;
        gxHeapFree(gxGetProcessHeap(), 0, current_line);
        current_line = previous_line->next;
        es->line_count--;
        continue;
      }
      else /* current_line->index + delta == current_position */
      {
        if (current_position - es->text > iend)
          break; /* We reached end of line modifications */
        /* else recalculate this line */
      }
    }

    current_line->index = (GXINT)(current_position - es->text);
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
      current_line->net_length = (GXINT)GXSTRLEN(current_position);
    } else if ((cp > current_position) && (*(cp - 1) == '\r')) {
      current_line->ending = END_SOFT;
      current_line->net_length = (GXINT)(cp - current_position - 1);
    } else if (*cp == '\n') {
      current_line->ending = END_RICH;
      current_line->net_length = (GXINT)(cp - current_position);
    } else {
      current_line->ending = END_HARD;
      current_line->net_length = (GXINT)(cp - current_position);
    }

    /* Calculate line width */
    current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
      current_position, current_line->net_length,
      es->tabs_count, es->tabs));

    /* FIXME: check here for lines that are too wide even in AUTOHSCROLL (> 32767 ???) */
    if (!(es->style & ES_AUTOHSCROLL)) {
      if (current_line->width > fw) {
        GXINT next = 0;
        GXINT prev;
        do {
          prev = next;
          next = EDIT_CallWordBreakProc(es, (GXINT)(current_position - es->text),
            (GXINT)(prev + 1), (GXINT)current_line->net_length, GXWB_RIGHT);
          current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
            current_position, next, es->tabs_count, es->tabs));
        } while (current_line->width <= fw);
        if (!prev) { /* Didn't find a line break so force a break */
          next = 0;
          do {
            prev = next;
            next++;
            current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
              current_position, next, es->tabs_count, es->tabs));
          } while (current_line->width <= fw);
          if (!prev)
            prev = 1;
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
        current_line->width = (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc, current_position,
          current_line->net_length, es->tabs_count, es->tabs));
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
    es->text_width = max(es->text_width, current_line->width);
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
      es->line_count--;
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
    rc.top = es->format_rect.top + nstart_line * es->line_height -
      (es->y_offset * es->line_height); /* Adjust for vertical scrollbar */
    rc.bottom = rc.top + es->line_height;
    if ((es->style & ES_CENTER) || (es->style & ES_RIGHT))
      rc.left = es->format_rect.left;
    else
      rc.left = es->format_rect.left + (GXINT)GXLOWORD(gxGetTabbedTextExtentW(dc,
      es->text + nstart_index, istart - nstart_index,
      es->tabs_count, es->tabs)) - es->x_offset; /* Adjust for horz scroll */
    rc.right = es->format_rect.right;
    gxSetRectRgn(hrgn, rc.left, rc.top, rc.right, rc.bottom);

    rc.top = rc.bottom;
    rc.left = es->format_rect.left;
    rc.right = es->format_rect.right;
    /*
    * If lines were added or removed we must re-paint the remainder of the
    * lines since the remaining lines were either shifted up or down.
    */
    if (line_count < es->line_count) /* We added lines */
      rc.bottom = es->line_count * es->line_height;
    else if (line_count > es->line_count) /* We removed lines */
      rc.bottom = line_count * es->line_height;
    else
      rc.bottom = line_index * es->line_height;
    rc.bottom += es->format_rect.top;
    rc.bottom -= (es->y_offset * es->line_height); /* Adjust for vertical scrollbar */
    tmphrgn = gxCreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
    gxCombineRgn(hrgn, hrgn, tmphrgn, RGN_OR);
    gxDeleteObject(tmphrgn);
  }

  if (es->font)
    gxSelectObject(dc, old_font);

  gxReleaseDC(es->hwndSelf, dc);
}


static inline GXUINT get_text_length(EDITSTATE *es)
{
    if(es->text_length == (GXUINT)-1)
        es->text_length = (GXUINT)GXSTRLEN(es->text);
    return es->text_length;
}

/*********************************************************************
 *
 *  EDIT_GetPasswordPointer_SL
 *
 *  note: caller should free the (optionally) allocated buffer
 *
 */
static GXLPWSTR EDIT_GetPasswordPointer_SL(EDITSTATE *es)
{
  if (es->style & GXES_PASSWORD) {
    GXINT len = get_text_length(es);
    GXLPWSTR text = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (len + 1) * sizeof(GXWCHAR));
    text[len] = '\0';
    while(len) text[--len] = es->password_char;
    return text;
  } else
    return es->text;
}


/*********************************************************************
 *
 *  EDIT_CalcLineWidth_SL
 *
 */
static void EDIT_CalcLineWidth_SL(EDITSTATE *es)
{
  GXSIZE size;
  GXLPWSTR text;
  GXHDC dc;
  GXHFONT old_font = 0;

  text = EDIT_GetPasswordPointer_SL(es);

  dc = gxGetDC(es->hwndSelf);
  if (es->font)
    old_font = (GXHFONT)gxSelectObject(dc, es->font);

  gxGetTextExtentPoint32W(dc, text, (GXINT)GXSTRLEN(text), &size);

  if (es->font)
    gxSelectObject(dc, old_font);
  gxReleaseDC(es->hwndSelf, dc);

  if (es->style & ES_PASSWORD)
    gxHeapFree(gxGetProcessHeap(), 0, text);

  es->text_width = size.cx;
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
static GXINT EDIT_CharFromPos(EDITSTATE *es, GXINT x, GXINT y, GXLPBOOL after_wrap)
{
  GXINT index;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXINT x_high = 0, x_low = 0;

  if (es->style & GXES_MULTILINE) {
    GXINT line = (y - es->format_rect.top) / es->line_height + es->y_offset;
    GXINT line_index = 0;
    LINEDEF *line_def = es->first_line_def;
    GXINT low, high;
    while ((line > 0) && line_def->next) {
      line_index += line_def->length;
      line_def = line_def->next;
      line--;
    }
    x += es->x_offset - es->format_rect.left;
    if (es->style & GXES_RIGHT)
      x -= (es->format_rect.right - es->format_rect.left) - line_def->width;
    else if (es->style & GXES_CENTER)
      x -= ((es->format_rect.right - es->format_rect.left) - line_def->width) / 2;
    if (x >= line_def->width) {
      if (after_wrap)
        *after_wrap = (line_def->ending == END_WRAP);
      return line_index + line_def->net_length;
    }
    if (x <= 0) {
      if (after_wrap)
        *after_wrap = FALSE;
      return line_index;
    }
    dc = gxGetDC(es->hwndSelf);
    if (es->font)
      old_font = (GXHFONT)gxSelectObject(dc, es->font);
    low = line_index;
    high = line_index + line_def->net_length + 1;
    while (low < high - 1)
    {
      GXINT mid = (low + high) / 2;
      GXINT x_now = GXLOWORD(gxGetTabbedTextExtentW(dc, es->text + line_index, mid - line_index, es->tabs_count, es->tabs));
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
    x -= es->format_rect.left;
    if (!x)
      return es->x_offset;

    if (!es->x_offset)
    {
      GXINT indent = (es->format_rect.right - es->format_rect.left) - es->text_width;
      if (es->style & GXES_RIGHT)
        x -= indent;
      else if (es->style & GXES_CENTER)
        x -= indent / 2;
    }

    text = EDIT_GetPasswordPointer_SL(es);
    dc = gxGetDC(es->hwndSelf);
    if (es->font)
      old_font = (GXHFONT)gxSelectObject(dc, es->font);
    if (x < 0)
    {
      GXINT low = 0;
      GXINT high = es->x_offset;
      while (low < high - 1)
      {
        GXINT mid = (low + high) / 2;
        gxGetTextExtentPoint32W( dc, text + mid,
          es->x_offset - mid, &size );
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
      GXINT low = es->x_offset;
      GXINT high = get_text_length(es) + 1;
      while (low < high - 1)
      {
        GXINT mid = (low + high) / 2;
        gxGetTextExtentPoint32W( dc, text + es->x_offset,
          mid - es->x_offset, &size );
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
    if (es->style & GXES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }
  if (es->font)
    gxSelectObject(dc, old_font);
  gxReleaseDC(es->hwndSelf, dc);
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
static void EDIT_ConfinePoint(const EDITSTATE *es, GXLPINT x, GXLPINT y)
{
  *x = min(max(*x, es->format_rect.left), es->format_rect.right - 1);
  *y = min(max(*y, es->format_rect.top), es->format_rect.bottom - 1);
}


/*********************************************************************
 *
 *  EM_LINEFROMCHAR
 *
 */
static GXINT EDIT_EM_LineFromChar(EDITSTATE *es, GXINT index)
{
  GXINT line;
  LINEDEF *line_def;

  if (!(es->style & GXES_MULTILINE))
    return 0;
  if (index > (GXINT)get_text_length(es))
    return es->line_count - 1;
  if (index == -1)
    index = min(es->selection_start, es->selection_end);

  line = 0;
  line_def = es->first_line_def;
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
static GXINT EDIT_EM_LineIndex(const EDITSTATE *es, GXINT line)
{
  GXINT line_index;
  const LINEDEF *line_def;

  if (!(es->style & GXES_MULTILINE))
    return 0;
  if (line >= es->line_count)
    return -1;

  line_index = 0;
  line_def = es->first_line_def;
  if (line == -1) {
    GXINT index = es->selection_end - line_def->length;
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
static GXINT EDIT_EM_LineLength(EDITSTATE *es, GXINT index)
{
  LINEDEF *line_def;

  if (!(es->style & GXES_MULTILINE))
    return get_text_length(es);

  if (index == -1) {
    /* get the number of remaining non-selected chars of selected lines */
    INT32 l; /* line number */
    INT32 li; /* index of first char in line */
    INT32 count;
    l = EDIT_EM_LineFromChar(es, es->selection_start);
    /* # chars before start of selection area */
    count = es->selection_start - EDIT_EM_LineIndex(es, l);
    l = EDIT_EM_LineFromChar(es, es->selection_end);
    /* # chars after end of selection */
    li = EDIT_EM_LineIndex(es, l);
    count += li + EDIT_EM_LineLength(es, li) - es->selection_end;
    return count;
  }
  line_def = es->first_line_def;
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
static GXLRESULT EDIT_EM_PosFromChar(EDITSTATE *es, GXINT index, GXBOOL after_wrap)
{
  GXINT len = get_text_length(es);
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
  dc = gxGetDC(es->hwndSelf);
  if (es->font)
    old_font = (GXHFONT)gxSelectObject(dc, es->font);
  if (es->style & GXES_MULTILINE) {
    l = EDIT_EM_LineFromChar(es, index);
    y = (l - es->y_offset) * es->line_height;
    li = EDIT_EM_LineIndex(es, l);
    if (after_wrap && (li == index) && l) {
      GXINT l2 = l - 1;
      line_def = es->first_line_def;
      while (l2) {
        line_def = line_def->next;
        l2--;
      }
      if (line_def->ending == END_WRAP) {
        l--;
        y -= es->line_height;
        li = EDIT_EM_LineIndex(es, l);
      }
    }

    line_def = es->first_line_def;
    while (line_def->index != li)
      line_def = line_def->next;

    ll = line_def->net_length;
    lw = line_def->width;

    w = es->format_rect.right - es->format_rect.left;
    if (es->style & GXES_RIGHT)
    {
      x = GXLOWORD(gxGetTabbedTextExtentW(dc, es->text + li + (index - li), ll - (index - li),
        es->tabs_count, es->tabs)) - es->x_offset;
      x = w - x;
    }
    else if (es->style & GXES_CENTER)
    {
      x = GXLOWORD(gxGetTabbedTextExtentW(dc, es->text + li, index - li,
        es->tabs_count, es->tabs)) - es->x_offset;
      x += (w - lw) / 2;
    }
    else /* ES_LEFT */
    {
        x = GXLOWORD(gxGetTabbedTextExtentW(dc, es->text + li, index - li,
        es->tabs_count, es->tabs)) - es->x_offset;
    }
  } else {
    GXLPWSTR text = EDIT_GetPasswordPointer_SL(es);
    if (index < es->x_offset) {
      gxGetTextExtentPoint32W(dc, text + index,
          es->x_offset - index, &size);
      x = -size.cx;
    } else {
      gxGetTextExtentPoint32W(dc, text + es->x_offset,
          index - es->x_offset, &size);
       x = size.cx;

      if (!es->x_offset && (es->style & (ES_RIGHT | ES_CENTER)))
      {
        w = es->format_rect.right - es->format_rect.left;
        if (w > es->text_width)
        {
          if (es->style & ES_RIGHT)
            x += w - es->text_width;
          else if (es->style & ES_CENTER)
            x += (w - es->text_width) / 2;
        }
      }
    }
    y = 0;
    if (es->style & ES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }
  x += es->format_rect.left;
  y += es->format_rect.top;
  if (es->font)
    gxSelectObject(dc, old_font);
  gxReleaseDC(es->hwndSelf, dc);
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
static void EDIT_GetLineRect(EDITSTATE *es, GXINT line, GXINT scol, GXINT ecol, GXLPRECT rc)
{
  GXINT line_index =  EDIT_EM_LineIndex(es, line);

  if (es->style & ES_MULTILINE)
    rc->top = es->format_rect.top + (line - es->y_offset) * es->line_height;
  else
    rc->top = es->format_rect.top;
  rc->bottom = rc->top + es->line_height;
  rc->left = (scol == 0) ? es->format_rect.left : (short)GXLOWORD(EDIT_EM_PosFromChar(es, line_index + scol, TRUE));
  rc->right = (ecol == -1) ? es->format_rect.right : (short)GXLOWORD(EDIT_EM_PosFromChar(es, line_index + ecol, TRUE));
}


static inline void text_buffer_changed(EDITSTATE *es)
{
    es->text_length = (GXUINT)-1;
}

/*********************************************************************
 * EDIT_LockBuffer
 *
 */
static void EDIT_LockBuffer(EDITSTATE *es)
{
  if (!es->text) {

    if(!es->hloc32W) return;

    if(es->hloc32A)
    {
      GXCHAR *textA = (GXCHAR*)gxLocalLock(es->hloc32A);
      GXHLOCAL hloc32W_new;
      GXUINT countW_new = gxMultiByteToWideChar(CP_ACP, 0, textA, -1, NULL, 0);
      if(countW_new > es->buffer_size + 1)
      {
        GXUINT alloc_size = ROUND_TO_GROW(countW_new * sizeof(GXWCHAR));
        TRACE("Resizing 32-bit UNICODE buffer from %d+1 to %d WCHARs\n", es->buffer_size, countW_new);
        hloc32W_new = gxLocalReAlloc(es->hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if(hloc32W_new)
        {
          es->hloc32W = hloc32W_new;
          es->buffer_size = (GXINT)gxLocalSize(hloc32W_new)/sizeof(GXWCHAR) - 1;
          TRACE("Real new size %d+1 WCHARs\n", es->buffer_size);
        }
        else
          WARN("FAILED! Will synchronize partially\n");
      }
      es->text = (GXLPWSTR)gxLocalLock(es->hloc32W);
      gxMultiByteToWideChar(CP_ACP, 0, textA, -1, es->text, es->buffer_size + 1);
      gxLocalUnlock(es->hloc32A);
    }
    else es->text = (GXLPWSTR)gxLocalLock(es->hloc32W);
  }
  if(es->flags & EF_APP_HAS_HANDLE) text_buffer_changed(es);
  es->lock_count++;
}


/*********************************************************************
 *
 *  EDIT_UnlockBuffer
 *
 */
static void EDIT_UnlockBuffer(EDITSTATE *es, GXBOOL force)
{

  /* Edit window might be already destroyed */
  if(!gxIsWindow(es->hwndSelf))
  {
    WARN("edit hwnd %p already destroyed\n", es->hwndSelf);
    return;
  }

  if (!es->lock_count) {
    ERR("lock_count == 0 ... please report\n");
    return;
  }
  if (!es->text) {
    ERR("es->text == 0 ... please report\n");
    return;
  }

  if (force || (es->lock_count == 1)) {
    if (es->hloc32W) {
      GXUINT countA = 0;
      GXUINT countW = get_text_length(es) + 1;

      if(es->hloc32A)
      {
        GXUINT countA_new = gxWideCharToMultiByte(CP_ACP, 0, es->text, countW, NULL, 0, NULL, NULL);
        TRACE("Synchronizing with 32-bit ANSI buffer\n");
        TRACE("%d WCHARs translated to %d bytes\n", countW, countA_new);
        countA = (GXINT)LocalSize(es->hloc32A);
        if(countA_new > countA)
        {
          GXHLOCAL hloc32A_new;
          GXUINT alloc_size = ROUND_TO_GROW(countA_new);
          TRACE("Resizing 32-bit ANSI buffer from %d to %d bytes\n", countA, alloc_size);
          hloc32A_new = gxLocalReAlloc(es->hloc32A, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT);
          if(hloc32A_new)
          {
            es->hloc32A = hloc32A_new;
            countA = (GXINT)LocalSize(hloc32A_new);
            TRACE("Real new size %d bytes\n", countA);
          }
          else
            WARN("FAILED! Will synchronize partially\n");
        }
        gxWideCharToMultiByte(CP_ACP, 0, es->text, countW,
          (GXLPSTR)gxLocalLock(es->hloc32A), countA, NULL, NULL);
        gxLocalUnlock(es->hloc32A);
      }

      gxLocalUnlock(es->hloc32W);
      es->text = NULL;
    }
    else {
      ERR("no buffer ... please report\n");
      return;
    }
  }
  es->lock_count--;
}


/*********************************************************************
 *
 *  EDIT_MakeFit
 *
 * Try to fit size + 1 characters in the buffer.
 */
static GXBOOL EDIT_MakeFit(EDITSTATE *es, GXUINT size)
{
  GXHLOCAL hNew32W;

  if (size <= es->buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1 characters\n", size);

  /* Force edit to unlock it's buffer. es->text now NULL */
  EDIT_UnlockBuffer(es, TRUE);

  if (es->hloc32W) {
      GXUINT alloc_size = ROUND_TO_GROW((size + 1) * sizeof(GXWCHAR));
      if ((hNew32W = gxLocalReAlloc(es->hloc32W, alloc_size, LMEM_MOVEABLE | LMEM_ZEROINIT))) {
    TRACE("Old 32 bit handle %p, new handle %p\n", es->hloc32W, hNew32W);
    es->hloc32W = hNew32W;
    es->buffer_size = (GXINT)gxLocalSize(hNew32W)/sizeof(GXWCHAR) - 1;
      }
  }

  EDIT_LockBuffer(es);

  if (es->buffer_size < size) {
    WARN("FAILED !  We now have %d+1\n", es->buffer_size);
    EDIT_NOTIFY_PARENT(es, EN_ERRSPACE);
    return FALSE;
  } else {
    TRACE("We now have %d+1\n", es->buffer_size);
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
static GXBOOL EDIT_MakeUndoFit(EDITSTATE *es, GXUINT size)
{
  GXUINT alloc_size;

  if (size <= es->undo_buffer_size)
    return TRUE;

  TRACE("trying to ReAlloc to %d+1\n", size);

  alloc_size = ROUND_TO_GROW((size + 1) * sizeof(GXWCHAR));
  if ((es->undo_text = (GXLPWSTR)gxHeapReAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, es->undo_text, alloc_size))) {
    es->undo_buffer_size = alloc_size/sizeof(GXWCHAR) - 1;
    return TRUE;
  }
  else
  {
    WARN("FAILED !  We now have %d+1\n", es->undo_buffer_size);
    return FALSE;
  }
}


/*********************************************************************
 *
 *  EDIT_UpdateTextRegion
 *
 */
static void EDIT_UpdateTextRegion(EDITSTATE *es, GXHRGN hrgn, GXBOOL bErase)
{
    if (es->flags & EF_UPDATE) {
        es->flags &= ~EF_UPDATE;
        EDIT_NOTIFY_PARENT(es, EN_UPDATE);
    }
    gxInvalidateRgn(es->hwndSelf, hrgn, bErase);
}


/*********************************************************************
 *
 *  EDIT_UpdateText
 *
 */
static void EDIT_UpdateText(EDITSTATE *es, const GXRECT *rc, GXBOOL bErase)
{
    if (es->flags & EF_UPDATE) {
        es->flags &= ~EF_UPDATE;
        EDIT_NOTIFY_PARENT(es, EN_UPDATE);
    }
    gxInvalidateRect(es->hwndSelf, rc, bErase);
}

/*********************************************************************
 *
 *  EDIT_SL_InvalidateText
 *
 *  Called from EDIT_InvalidateText().
 *  Does the job for single-line controls only.
 *
 */
static void EDIT_SL_InvalidateText(EDITSTATE *es, GXINT start, GXINT end)
{
  GXRECT line_rect;
  GXRECT rc;

  EDIT_GetLineRect(es, 0, start, end, &line_rect);
  if (gxIntersectRect(&rc, &line_rect, &es->format_rect))
    EDIT_UpdateText(es, &rc, TRUE);
}


static inline GXINT get_vertical_line_count(EDITSTATE *es)
{
  GXINT vlc = (es->format_rect.bottom - es->format_rect.top) / es->line_height;
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
static void EDIT_ML_InvalidateText(EDITSTATE *es, GXINT start, GXINT end)
{
  GXINT vlc = get_vertical_line_count(es);
  GXINT sl = EDIT_EM_LineFromChar(es, start);
  GXINT el = EDIT_EM_LineFromChar(es, end);
  GXINT sc;
  GXINT ec;
  GXRECT rc1;
  GXRECT rcWnd;
  GXRECT rcLine;
  GXRECT rcUpdate;
  GXINT l;

  if ((el < es->y_offset) || (sl > es->y_offset + vlc))
    return;

  sc = start - EDIT_EM_LineIndex(es, sl);
  ec = end - EDIT_EM_LineIndex(es, el);
  if (sl < es->y_offset) {
    sl = es->y_offset;
    sc = 0;
  }
  if (el > es->y_offset + vlc) {
    el = es->y_offset + vlc;
    ec = EDIT_EM_LineLength(es, EDIT_EM_LineIndex(es, el));
  }
  gxGetClientRect(es->hwndSelf, &rc1);
  gxIntersectRect(&rcWnd, &rc1, &es->format_rect);
  if (sl == el) {
    EDIT_GetLineRect(es, sl, sc, ec, &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      EDIT_UpdateText(es, &rcUpdate, TRUE);
  } else {
    EDIT_GetLineRect(es, sl, sc,
        EDIT_EM_LineLength(es,
          EDIT_EM_LineIndex(es, sl)),
        &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      EDIT_UpdateText(es, &rcUpdate, TRUE);
    for (l = sl + 1 ; l < el ; l++) {
      EDIT_GetLineRect(es, l, 0,
        EDIT_EM_LineLength(es,
          EDIT_EM_LineIndex(es, l)),
        &rcLine);
      if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
        EDIT_UpdateText(es, &rcUpdate, TRUE);
    }
    EDIT_GetLineRect(es, el, 0, ec, &rcLine);
    if (gxIntersectRect(&rcUpdate, &rcWnd, &rcLine))
      EDIT_UpdateText(es, &rcUpdate, TRUE);
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
static void EDIT_InvalidateText(EDITSTATE *es, GXINT start, GXINT end)
{
  if (end == start)
    return;

  if (end == -1)
    end = get_text_length(es);

  if (end < start) {
      GXINT tmp = start;
      start = end;
      end = tmp;
  }

  if (es->style & ES_MULTILINE)
    EDIT_ML_InvalidateText(es, start, end);
  else
    EDIT_SL_InvalidateText(es, start, end);
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
static void EDIT_EM_SetSel(EDITSTATE *es, GXUINT start, GXUINT end, GXBOOL after_wrap)
{
  GXUINT old_start = es->selection_start;
  GXUINT old_end = es->selection_end;
  GXUINT len = get_text_length(es);

  if (start == (GXUINT)-1) {
    start = es->selection_end;
    end = es->selection_end;
  } else {
    start = min(start, len);
    end = min(end, len);
  }
  es->selection_start = start;
  es->selection_end = end;
  if (after_wrap)
    es->flags |= EF_AFTER_WRAP;
  else
    es->flags &= ~EF_AFTER_WRAP;
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
                EDIT_InvalidateText(es, start, end);
                EDIT_InvalidateText(es, old_start, old_end);
            }
            else
            {
                EDIT_InvalidateText(es, start, old_start);
                EDIT_InvalidateText(es, end, old_end);
            }
  }
        else EDIT_InvalidateText(es, start, old_end);
}


/*********************************************************************
 *
 *  EDIT_UpdateScrollInfo
 *
 */
static void EDIT_UpdateScrollInfo(EDITSTATE *es)
{
  if ((es->style & GXWS_VSCROLL) && !(es->flags & EF_VSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize  = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin    = 0;
    si.nMax    = es->line_count - 1;
    si.nPage  = (es->format_rect.bottom - es->format_rect.top) / es->line_height;
    si.nPos    = es->y_offset;
    TRACE("SB_VERT, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n",
      si.nMin, si.nMax, si.nPage, si.nPos);
    gxSetScrollInfo(es->hwndSelf, SB_VERT, &si, TRUE);
  }

  if ((es->style & GXWS_HSCROLL) && !(es->flags & EF_HSCROLL_TRACK))
  {
    GXSCROLLINFO si;
    si.cbSize  = sizeof(GXSCROLLINFO);
    si.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin    = 0;
    si.nMax    = es->text_width - 1;
    si.nPage  = es->format_rect.right - es->format_rect.left;
    si.nPos    = es->x_offset;
    TRACE("SB_HORZ, nMin=%d, nMax=%d, nPage=%d, nPos=%d\n",
      si.nMin, si.nMax, si.nPage, si.nPos);
    gxSetScrollInfo(es->hwndSelf, SB_HORZ, &si, TRUE);
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
static GXBOOL EDIT_EM_LineScroll_internal(EDITSTATE *es, GXINT dx, GXINT dy)
{
  GXINT nyoff;
  GXINT x_offset_in_pixels;
  GXINT lines_per_page = (es->format_rect.bottom - es->format_rect.top) /
            es->line_height;

  if (es->style & GXES_MULTILINE)
  {
      x_offset_in_pixels = es->x_offset;
  }
  else
  {
      dy = 0;
      x_offset_in_pixels = (short)GXLOWORD(EDIT_EM_PosFromChar(es, es->x_offset, FALSE));
  }

  if (-dx > x_offset_in_pixels)
    dx = -x_offset_in_pixels;
  if (dx > es->text_width - x_offset_in_pixels)
    dx = es->text_width - x_offset_in_pixels;
  nyoff = max(0, es->y_offset + dy);
  if (nyoff >= es->line_count - lines_per_page)
    nyoff = max(0, es->line_count - lines_per_page);
  dy = (es->y_offset - nyoff) * es->line_height;
  if (dx || dy) {
    GXRECT rc1;
    GXRECT rc;

    es->y_offset = nyoff;
    if(es->style & GXES_MULTILINE)
        es->x_offset += dx;
    else
        es->x_offset += dx / es->char_width;

    gxGetClientRect(es->hwndSelf, &rc1);
    gxIntersectRect(&rc, &rc1, &es->format_rect);
    gxScrollWindowEx(es->hwndSelf, -dx, dy,
        NULL, &rc, NULL, NULL, SW_INVALIDATE);
    /* force scroll info update */
    EDIT_UpdateScrollInfo(es);
  }
  if (dx && !(es->flags & EF_HSCROLL_TRACK))
    EDIT_NOTIFY_PARENT(es, EN_HSCROLL);
  if (dy && !(es->flags & EF_VSCROLL_TRACK))
    EDIT_NOTIFY_PARENT(es, EN_VSCROLL);
  return TRUE;
}

/*********************************************************************
 *
 *  EM_LINESCROLL
 *
 *  NOTE: dx is in average character widths, dy - in lines;
 *
 */
static GXBOOL EDIT_EM_LineScroll(EDITSTATE *es, GXINT dx, GXINT dy)
{
  if (!(es->style & GXES_MULTILINE))
    return FALSE;

  dx *= es->char_width;
  return EDIT_EM_LineScroll_internal(es, dx, dy);
}


/*********************************************************************
 *
 *  EM_SCROLL
 *
 */
static GXLRESULT EDIT_EM_Scroll(EDITSTATE *es, GXINT action)
{
  GXINT dy;

  if (!(es->style & GXES_MULTILINE))
    return (GXLRESULT)FALSE;

  dy = 0;

  switch (action) {
  case SB_LINEUP:
    if (es->y_offset)
      dy = -1;
    break;
  case SB_LINEDOWN:
    if (es->y_offset < es->line_count - 1)
      dy = 1;
    break;
  case SB_PAGEUP:
    if (es->y_offset)
      dy = -(es->format_rect.bottom - es->format_rect.top) / es->line_height;
    break;
  case SB_PAGEDOWN:
    if (es->y_offset < es->line_count - 1)
      dy = (es->format_rect.bottom - es->format_rect.top) / es->line_height;
    break;
  default:
    return (GXLRESULT)FALSE;
  }
  if (dy) {
      GXINT vlc = get_vertical_line_count(es);
      /* check if we are going to move too far */
      if(es->y_offset + dy > es->line_count - vlc)
    dy = max(es->line_count - vlc, 0) - es->y_offset;

      /* Notification is done in EDIT_EM_LineScroll */
      if(dy) {
    EDIT_EM_LineScroll(es, 0, dy);
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
static void EDIT_SetCaretPos(EDITSTATE *es, GXINT pos,
           GXBOOL after_wrap)
{
  GXLRESULT res = EDIT_EM_PosFromChar(es, pos, after_wrap);
  TRACE("%d - %dx%d\n", pos, (short)GXLOWORD(res), (short)GXHIWORD(res));
  gxSetCaretPos((short)GXLOWORD(res), (short)GXHIWORD(res));
}


/*********************************************************************
 *
 *  EM_SCROLLCARET
 *
 */
static void EDIT_EM_ScrollCaret(EDITSTATE *es)
{
  if (es->style & GXES_MULTILINE) {
    GXINT l;
    GXINT vlc;
    GXINT ww;
    GXINT cw = es->char_width;
    GXINT x;
    GXINT dy = 0;
    GXINT dx = 0;

    l = EDIT_EM_LineFromChar(es, es->selection_end);
    x = (short)GXLOWORD(EDIT_EM_PosFromChar(es, es->selection_end, es->flags & EF_AFTER_WRAP));
    vlc = get_vertical_line_count(es);
    if (l >= es->y_offset + vlc)
      dy = l - vlc + 1 - es->y_offset;
    if (l < es->y_offset)
      dy = l - es->y_offset;
    ww = es->format_rect.right - es->format_rect.left;
    if (x < es->format_rect.left)
      dx = x - es->format_rect.left - ww / HSCROLL_FRACTION / cw * cw;
    if (x > es->format_rect.right)
      dx = x - es->format_rect.left - (HSCROLL_FRACTION - 1) * ww / HSCROLL_FRACTION / cw * cw;
    if (dy || dx || (es->y_offset && (es->line_count - es->y_offset < vlc)))
    {
        /* check if we are going to move too far */
        if(es->x_offset + dx + ww > es->text_width)
      dx = es->text_width - ww - es->x_offset;
        if(dx || dy || (es->y_offset && (es->line_count - es->y_offset < vlc)))
      EDIT_EM_LineScroll_internal(es, dx, dy);
    }
  } else {
    GXINT x;
    GXINT goal;
    GXINT format_width;

    x = (short)GXLOWORD(EDIT_EM_PosFromChar(es, es->selection_end, FALSE));
    format_width = es->format_rect.right - es->format_rect.left;
    if (x < es->format_rect.left) {
      goal = es->format_rect.left + format_width / HSCROLL_FRACTION;
      do {
        es->x_offset--;
        x = (short)GXLOWORD(EDIT_EM_PosFromChar(es, es->selection_end, FALSE));
      } while ((x < goal) && es->x_offset);
      /* FIXME: use ScrollWindow() somehow to improve performance */
      EDIT_UpdateText(es, NULL, TRUE);
    } else if (x > es->format_rect.right) {
      GXINT x_last;
      GXINT len = get_text_length(es);
      goal = es->format_rect.right - format_width / HSCROLL_FRACTION;
      do {
        es->x_offset++;
        x = (short)GXLOWORD(EDIT_EM_PosFromChar(es, es->selection_end, FALSE));
        x_last = (short)GXLOWORD(EDIT_EM_PosFromChar(es, len, FALSE));
      } while ((x > goal) && (x_last > es->format_rect.right));
      /* FIXME: use ScrollWindow() somehow to improve performance */
      EDIT_UpdateText(es, NULL, TRUE);
    }
  }

    if(es->flags & EF_FOCUSED)
  EDIT_SetCaretPos(es, es->selection_end, es->flags & EF_AFTER_WRAP);
}


/*********************************************************************
 *
 *  EDIT_MoveBackward
 *
 */
static void EDIT_MoveBackward(EDITSTATE *es, GXBOOL extend)
{
  GXINT e = es->selection_end;

  if (e) {
    e--;
    if ((es->style & GXES_MULTILINE) && e &&
        (es->text[e - 1] == '\r') && (es->text[e] == '\n')) {
      e--;
      if (e && (es->text[e - 1] == '\r'))
        e--;
    }
  }
  EDIT_EM_SetSel(es, extend ? es->selection_start : e, e, FALSE);
  EDIT_EM_ScrollCaret(es);
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
static void EDIT_MoveDown_ML(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXBOOL after_wrap = (es->flags & EF_AFTER_WRAP);
  GXLRESULT pos = EDIT_EM_PosFromChar(es, e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = EDIT_CharFromPos(es, x, y + es->line_height, &after_wrap);
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_MoveEnd
 *
 */
static void EDIT_MoveEnd(EDITSTATE *es, GXBOOL extend, GXBOOL ctrl)
{
  GXBOOL after_wrap = FALSE;
  GXINT e;

  /* Pass a high value in x to make sure of receiving the end of the line */
  if (!ctrl && (es->style & GXES_MULTILINE))
    e = EDIT_CharFromPos(es, 0x3fffffff,
      GXHIWORD(EDIT_EM_PosFromChar(es, es->selection_end, es->flags & EF_AFTER_WRAP)), &after_wrap);
  else
    e = get_text_length(es);
  EDIT_EM_SetSel(es, extend ? es->selection_start : e, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_MoveForward
 *
 */
static void EDIT_MoveForward(EDITSTATE *es, GXBOOL extend)
{
  GXINT e = es->selection_end;

  if (es->text[e]) {
    e++;
    if ((es->style & GXES_MULTILINE) && (es->text[e - 1] == '\r')) {
      if (es->text[e] == '\n')
        e++;
      else if ((es->text[e] == '\r') && (es->text[e + 1] == '\n'))
        e += 2;
    }
  }
  EDIT_EM_SetSel(es, extend ? es->selection_start : e, e, FALSE);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_MoveHome
 *
 *  Home key: move to beginning of line.
 *
 */
static void EDIT_MoveHome(EDITSTATE *es, GXBOOL extend, GXBOOL ctrl)
{
  GXINT e;

  /* Pass the x_offset in x to make sure of receiving the first position of the line */
  if (!ctrl && (es->style & GXES_MULTILINE))
    e = EDIT_CharFromPos(es, -es->x_offset,
      GXHIWORD(EDIT_EM_PosFromChar(es, es->selection_end, es->flags & EF_AFTER_WRAP)), NULL);
  else
    e = 0;
  EDIT_EM_SetSel(es, extend ? es->selection_start : e, e, FALSE);
  EDIT_EM_ScrollCaret(es);
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
static void EDIT_MovePageDown_ML(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXBOOL after_wrap = (es->flags & EF_AFTER_WRAP);
  GXLRESULT pos = EDIT_EM_PosFromChar(es, e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = EDIT_CharFromPos(es, x,
    y + (es->format_rect.bottom - es->format_rect.top),
    &after_wrap);
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
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
static void EDIT_MovePageUp_ML(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXBOOL after_wrap = (es->flags & EF_AFTER_WRAP);
  GXLRESULT pos = EDIT_EM_PosFromChar(es, e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = EDIT_CharFromPos(es, x,
    y - (es->format_rect.bottom - es->format_rect.top),
    &after_wrap);
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
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
static void EDIT_MoveUp_ML(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXBOOL after_wrap = (es->flags & EF_AFTER_WRAP);
  GXLRESULT pos = EDIT_EM_PosFromChar(es, e, after_wrap);
  GXINT x = (short)GXLOWORD(pos);
  GXINT y = (short)GXHIWORD(pos);

  e = EDIT_CharFromPos(es, x, y - es->line_height, &after_wrap);
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_MoveWordBackward
 *
 */
static void EDIT_MoveWordBackward(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXINT l;
  GXINT ll;
  GXINT li;

  l = EDIT_EM_LineFromChar(es, e);
  ll = EDIT_EM_LineLength(es, e);
  li = EDIT_EM_LineIndex(es, l);
  if (e - li == 0) {
    if (l) {
      li = EDIT_EM_LineIndex(es, l - 1);
      e = li + EDIT_EM_LineLength(es, li);
    }
  } else {
    e = li + EDIT_CallWordBreakProc(es, li, e - li, ll, WB_LEFT);
  }
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, FALSE);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_MoveWordForward
 *
 */
static void EDIT_MoveWordForward(EDITSTATE *es, GXBOOL extend)
{
  GXINT s = es->selection_start;
  GXINT e = es->selection_end;
  GXINT l;
  GXINT ll;
  GXINT li;

  l = EDIT_EM_LineFromChar(es, e);
  ll = EDIT_EM_LineLength(es, e);
  li = EDIT_EM_LineIndex(es, l);
  if (e - li == ll) {
    if ((es->style & GXES_MULTILINE) && (l != es->line_count - 1))
      e = EDIT_EM_LineIndex(es, l + 1);
  } else {
    e = li + EDIT_CallWordBreakProc(es,
        li, e - li + 1, ll, WB_RIGHT);
  }
  if (!extend)
    s = e;
  EDIT_EM_SetSel(es, s, e, FALSE);
  EDIT_EM_ScrollCaret(es);
}


/*********************************************************************
 *
 *  EDIT_PaintText
 *
 */
static GXINT EDIT_PaintText(EDITSTATE *es, GXHDC dc, GXINT x, GXINT y, GXINT line, GXINT col, GXINT count, GXBOOL rev)
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
    if (es->composition_len == 0)
    {
      gxSetBkColor(dc, gxGetSysColor(COLOR_HIGHLIGHT));
      gxSetTextColor(dc, gxGetSysColor(COLOR_HIGHLIGHTTEXT));
      gxSetBkMode( dc, GXOPAQUE);
    }
    else
    {
      GXHFONT current = (GXHFONT)gxGetCurrentObject(dc,OBJ_FONT);
      gxGetObjectW(current,sizeof(LOGFONTW),&underline_font);
      underline_font.lfUnderline = TRUE;
      hUnderline = (GXHFONT)gxCreateFontIndirectW(&underline_font);
      old_font = (GXHFONT)gxSelectObject(dc,hUnderline);
    }
  }
  li = EDIT_EM_LineIndex(es, line);
  if (es->style & GXES_MULTILINE) {
    ret = (GXINT)GXLOWORD(gxTabbedTextOutW(dc, x, y, es->text + li + col, count,
      es->tabs_count, es->tabs, es->format_rect.left - es->x_offset));
  } else {
    GXLPWSTR text = EDIT_GetPasswordPointer_SL(es);
    gxTextOutW(dc, x, y, text + li + col, count);
    gxGetTextExtentPoint32W(dc, text + li + col, count, &size);
    ret = size.cx;
    if (es->style & ES_PASSWORD)
      gxHeapFree(gxGetProcessHeap(), 0, text);
  }
  if (rev) {
    if (es->composition_len == 0)
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
static void EDIT_PaintLine(EDITSTATE *es, GXHDC dc, GXINT line, GXBOOL rev)
{
  GXINT s;
  GXINT e;
  GXINT li;
  GXINT ll;
  GXINT x;
  GXINT y;
  GXLRESULT pos;

  if (es->style & GXES_MULTILINE) {
    GXINT vlc = get_vertical_line_count(es);

    if ((line < es->y_offset) || (line > es->y_offset + vlc) || (line >= es->line_count))
      return;
  } else if (line)
    return;

  TRACE("line=%d\n", line);

  pos = EDIT_EM_PosFromChar(es, EDIT_EM_LineIndex(es, line), FALSE);
  x = (short)GXLOWORD(pos);
  y = (short)GXHIWORD(pos);
  li = EDIT_EM_LineIndex(es, line);
  ll = EDIT_EM_LineLength(es, li);
  s = min(es->selection_start, es->selection_end);
  e = max(es->selection_start, es->selection_end);
  s = min(li + ll, max(li, s));
  e = min(li + ll, max(li, e));
  if (rev && (s != e) &&
      ((es->flags & EF_FOCUSED) || (es->style & ES_NOHIDESEL))) {
    x += EDIT_PaintText(es, dc, x, y, line, 0, s - li, FALSE);
    x += EDIT_PaintText(es, dc, x, y, line, s - li, e - s, TRUE);
    x += EDIT_PaintText(es, dc, x, y, line, e - li, li + ll - e, FALSE);
  } else
    x += EDIT_PaintText(es, dc, x, y, line, 0, ll, FALSE);
}


/*********************************************************************
 *
 *  EDIT_AdjustFormatRect
 *
 *  Adjusts the format rectangle for the current font and the
 *  current client rectangle.
 *
 */
static void EDIT_AdjustFormatRect(EDITSTATE *es)
{
  GXRECT ClientRect;

  es->format_rect.right = max(es->format_rect.right, es->format_rect.left + es->char_width);
  if (es->style & GXES_MULTILINE)
  {
      GXINT fw, vlc, max_x_offset, max_y_offset;

      vlc = get_vertical_line_count(es);
      es->format_rect.bottom = es->format_rect.top + vlc * es->line_height;

      /* correct es->x_offset */
      fw = es->format_rect.right - es->format_rect.left;
      max_x_offset = es->text_width - fw;
      if(max_x_offset < 0) max_x_offset = 0;
      if(es->x_offset > max_x_offset)
    es->x_offset = max_x_offset;

      /* correct es->y_offset */
      max_y_offset = es->line_count - vlc;
      if(max_y_offset < 0) max_y_offset = 0;
      if(es->y_offset > max_y_offset)
    es->y_offset = max_y_offset;

      /* force scroll info update */
      EDIT_UpdateScrollInfo(es);
  }
  else
  /* Windows doesn't care to fix text placement for SL controls */
    es->format_rect.bottom = es->format_rect.top + es->line_height;

  /* Always stay within the client area */
  gxGetClientRect(es->hwndSelf, &ClientRect);
  es->format_rect.bottom = min(es->format_rect.bottom, ClientRect.bottom);

  if ((es->style & GXES_MULTILINE) && !(es->style & ES_AUTOHSCROLL))
    EDIT_BuildLineDefs_ML(es, 0, get_text_length(es), 0, NULL);
  
  EDIT_SetCaretPos(es, es->selection_end, es->flags & EF_AFTER_WRAP);
}


/*********************************************************************
 *
 *  EDIT_SetRectNP
 *
 *  note:  this is not (exactly) the handler called on EM_SETRECTNP
 *    it is also used to set the rect of a single line control
 *
 */
static void EDIT_SetRectNP(EDITSTATE *es, const GXRECT *rc)
{
  LONG_PTR ExStyle;
  GXINT bw, bh;
  ExStyle = gxGetWindowLongPtrW(es->hwndSelf, GWL_EXSTYLE);
  
  gxCopyRect(&es->format_rect, rc);
  
  if (ExStyle & GXWS_EX_CLIENTEDGE) {
    es->format_rect.left++;
    es->format_rect.right--;
    
    if (es->format_rect.bottom - es->format_rect.top
        >= es->line_height + 2)
    {
      es->format_rect.top++;
      es->format_rect.bottom--;
    }
  }
  else if (es->style & GXWS_BORDER) {
    bw = GetSystemMetrics(SM_CXBORDER) + 1;
    bh = GetSystemMetrics(SM_CYBORDER) + 1;
    es->format_rect.left += bw;
    es->format_rect.right -= bw;
    if (es->format_rect.bottom - es->format_rect.top
      >= es->line_height + 2 * bh)
    {
        es->format_rect.top += bh;
        es->format_rect.bottom -= bh;
    }
  }
  
  es->format_rect.left += es->left_margin;
  es->format_rect.right -= es->right_margin;
  EDIT_AdjustFormatRect(es);
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
static GXLRESULT EDIT_EM_CharFromPos(EDITSTATE *es, GXINT x, GXINT y)
{
  GXPOINT pt;
  GXRECT rc;
  GXINT index;

  pt.x = x;
  pt.y = y;
  gxGetClientRect(es->hwndSelf, &rc);
  if (!gxPtInRect(&rc, pt))
    return -1;

  index = EDIT_CharFromPos(es, x, y, NULL);
  return MAKELONG(index, EDIT_EM_LineFromChar(es, index));
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
static GXBOOL EDIT_EM_FmtLines(EDITSTATE *es, GXBOOL add_eol)
{
  es->flags &= ~EF_USE_SOFTBRK;
  if (add_eol) {
    es->flags |= EF_USE_SOFTBRK;
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
static GXHLOCAL EDIT_EM_GetHandle(EDITSTATE *es)
{
  GXHLOCAL hLocal;

  if (!(es->style & GXES_MULTILINE))
    return 0;

  if(es->is_unicode)
    hLocal = es->hloc32W;
  else
  {
    if(!es->hloc32A)
    {
      GXCHAR *textA;
      GXUINT countA, alloc_size;
      TRACE("Allocating 32-bit ANSI alias buffer\n");
      countA = gxWideCharToMultiByte(CP_ACP, 0, es->text, -1, NULL, 0, NULL, NULL);
      alloc_size = ROUND_TO_GROW(countA);
      if(!(es->hloc32A = (GXHLOCAL)gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
      {
        ERR("Could not allocate %d bytes for 32-bit ANSI alias buffer\n", alloc_size);
        return 0;
      }
      textA = (GXCHAR*)gxLocalLock(es->hloc32A);
      gxWideCharToMultiByte(CP_ACP, 0, es->text, -1, textA, countA, NULL, NULL);
      gxLocalUnlock(es->hloc32A);
    }
    hLocal = es->hloc32A;
  }

  es->flags |= EF_APP_HAS_HANDLE;
  TRACE("Returning %p, LocalSize() = %ld\n", hLocal, LocalSize(hLocal));
  return hLocal;
}


/*********************************************************************
 *
 *  EM_GETLINE
 *
 */
static GXINT EDIT_EM_GetLine(EDITSTATE *es, GXINT line, GXLPWSTR dst, GXBOOL unicode)
{
  GXLPWSTR src;
  GXINT line_len, dst_len;
  GXINT i;

  if (es->style & GXES_MULTILINE) {
    if (line >= es->line_count)
      return 0;
  } else
    line = 0;
  i = EDIT_EM_LineIndex(es, line);
  src = es->text + i;
  line_len = EDIT_EM_LineLength(es, i);
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
static GXLRESULT EDIT_EM_GetSel(const EDITSTATE *es, GXLPUINT start, GXLPUINT end)
{
  GXUINT s = es->selection_start;
  GXUINT e = es->selection_end;

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
static void EDIT_EM_ReplaceSel(EDITSTATE *es, GXBOOL can_undo, GXLPCWSTR lpsz_replace, GXBOOL send_update, GXBOOL honor_limit)
{
  GXUINT strl = (GXUINT)GXSTRLEN(lpsz_replace);
  GXUINT tl = get_text_length(es);
  GXUINT utl;
  GXUINT s;
  GXUINT e;
  GXUINT i;
  GXUINT size;
  GXLPWSTR p;
  GXHRGN hrgn = 0;
  GXLPWSTR buf = NULL;
  GXUINT bufl = 0;

  TRACEW(_CLTEXT("%s, can_undo %d, send_update %d\n"), debugstr_w(lpsz_replace), can_undo, send_update);

  s = es->selection_start;
  e = es->selection_end;

  if ((s == e) && !strl)
    return;

  ORDER_GXUINT(s, e);

  size = tl - (e - s) + strl;
  if (!size)
    es->text_width = 0;

  /* Issue the EN_MAXTEXT notification and continue with replacing text
   * such that buffer limit is honored. */
  if ((honor_limit) && (size > es->buffer_limit)) {
    EDIT_NOTIFY_PARENT(es, EN_MAXTEXT);
    /* Buffer limit can be smaller than the actual length of text in combobox */
    if (es->buffer_limit < (tl - (e-s)))
      strl = 0;
    else
      strl = es->buffer_limit - (tl - (e-s));
  }

  if (!EDIT_MakeFit(es, tl - (e - s) + strl))
    return;

  if (e != s) {
    /* there is something to be deleted */
    TRACE("deleting stuff.\n");
    bufl = e - s;
    buf = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (bufl + 1) * sizeof(GXWCHAR));
    if (!buf) return;
    memcpy(buf, es->text + s, bufl * sizeof(GXWCHAR));
    buf[bufl] = 0; /* ensure 0 termination */
    /* now delete */
    clstd::strcpyT(es->text + s, es->text + e);
    text_buffer_changed(es);
  }

  if (strl) {
    /* there is an insertion */
    tl = get_text_length(es);
    TRACEW(_CLTEXT("inserting stuff (tl %d, strl %d, selstart %d (%s), text %s)\n"), tl, strl, s, debugstr_w(es->text + s), debugstr_w(es->text));
    for (p = es->text + tl; p >= es->text + s; p--)
      p[strl] = p[0];
    for (i = 0, p = es->text + s; i < strl; i++)
      p[i] = lpsz_replace[i];
    if (es->style & ES_UPPERCASE)
      CharUpperBuffW(reinterpret_cast<LPWSTR>(p), strl);
    else if (es->style & ES_LOWERCASE)
      CharLowerBuffW(reinterpret_cast<LPWSTR>(p), strl);
    text_buffer_changed(es);
  }

  if (es->style & GXES_MULTILINE)
  {
    GXINT st = min(es->selection_start, es->selection_end);
    GXINT vlc = get_vertical_line_count(es);

    hrgn = gxCreateRectRgn(0, 0, 0, 0);
    EDIT_BuildLineDefs_ML(es, st, st + strl,
      strl - abs(es->selection_end - es->selection_start), hrgn);
    /* if text is too long undo all changes */
    if (honor_limit && !(es->style & ES_AUTOVSCROLL) && (es->line_count > vlc)) {
      if (strl)
        clstd::strcpyT(es->text + e, es->text + e + strl);
      if (e != s)
        for (i = 0, p = es->text; i < e - s; i++)
          p[i + s] = buf[i];
      text_buffer_changed(es);
      EDIT_BuildLineDefs_ML(es, s, e,
        abs(es->selection_end - es->selection_start) - strl, hrgn);
      strl = 0;
      e = s;
      hrgn = gxCreateRectRgn(0, 0, 0, 0);
      EDIT_NOTIFY_PARENT(es, EN_MAXTEXT);
    }
  }
  else {
    GXINT fw = es->format_rect.right - es->format_rect.left;
    EDIT_CalcLineWidth_SL(es);
    /* remove chars that don't fit */
    if (honor_limit && !(es->style & ES_AUTOHSCROLL) && (es->text_width > fw)) {
      while ((es->text_width > fw) && s + strl >= s) {
        clstd::strcpyT(es->text + s + strl - 1, es->text + s + strl);
        strl--;
        EDIT_CalcLineWidth_SL(es);
      }
                        text_buffer_changed(es);
      EDIT_NOTIFY_PARENT(es, EN_MAXTEXT);
    }
  }
  
  if (e != s) {
    if (can_undo) {
      utl = (GXUINT)GXSTRLEN(es->undo_text);
      if (!es->undo_insert_count && (*es->undo_text && (s == es->undo_position))) {
        /* undo-buffer is extended to the right */
        EDIT_MakeUndoFit(es, utl + e - s);
        memcpy(es->undo_text + utl, buf, (e - s)*sizeof(GXWCHAR));
        (es->undo_text + utl)[e - s] = 0; /* ensure 0 termination */
      } else if (!es->undo_insert_count && (*es->undo_text && (e == es->undo_position))) {
        /* undo-buffer is extended to the left */
        EDIT_MakeUndoFit(es, utl + e - s);
        for (p = es->undo_text + utl ; p >= es->undo_text ; p--)
          p[e - s] = p[0];
        for (i = 0 , p = es->undo_text ; i < e - s ; i++)
          p[i] = buf[i];
        es->undo_position = s;
      } else {
        /* new undo-buffer */
        EDIT_MakeUndoFit(es, e - s);
        memcpy(es->undo_text, buf, (e - s)*sizeof(GXWCHAR));
        es->undo_text[e - s] = 0; /* ensure 0 termination */
        es->undo_position = s;
      }
      /* any deletion makes the old insertion-undo invalid */
      es->undo_insert_count = 0;
    } else
      EDIT_EM_EmptyUndoBuffer(es);
  }
  if (strl) {
    if (can_undo) {
      if ((s == es->undo_position) ||
        ((es->undo_insert_count) &&
        (s == es->undo_position + es->undo_insert_count)))
        /*
         * insertion is new and at delete position or
         * an extension to either left or right
         */
        es->undo_insert_count += strl;
      else {
        /* new insertion undo */
        es->undo_position = s;
        es->undo_insert_count = strl;
        /* new insertion makes old delete-buffer invalid */
        *es->undo_text = '\0';
      }
    } else
      EDIT_EM_EmptyUndoBuffer(es);
  }

  if (bufl)
    gxHeapFree(gxGetProcessHeap(), 0, buf);
 
  s += strl;

  /* If text has been deleted and we're right or center aligned then scroll rightward */
  if (es->style & (ES_RIGHT | ES_CENTER))
  {
    GXINT delta = strl - abs(es->selection_end - es->selection_start);

    if (delta < 0 && es->x_offset)
    {
      if (abs(delta) > es->x_offset)
        es->x_offset = 0;
      else
        es->x_offset += delta;
    }
  }

  EDIT_EM_SetSel(es, s, s, FALSE);
  es->flags |= EF_MODIFIED;
  if (send_update) es->flags |= EF_UPDATE;
  if (hrgn)
  {
    EDIT_UpdateTextRegion(es, hrgn, TRUE);
    DeleteObject(hrgn);
  }
  else
            EDIT_UpdateText(es, NULL, TRUE);

  EDIT_EM_ScrollCaret(es);

  /* force scroll info update */
  EDIT_UpdateScrollInfo(es);


        if(send_update || (es->flags & EF_UPDATE))
  {
      es->flags &= ~EF_UPDATE;
      EDIT_NOTIFY_PARENT(es, EN_CHANGE);
  }
}


/*********************************************************************
 *
 *  EM_SETHANDLE
 *
 *  FIXME:  ES_LOWERCASE, ES_UPPERCASE, ES_OEMCONVERT, ES_NUMBER ???
 *
 */
static void EDIT_EM_SetHandle(EDITSTATE *es, GXHLOCAL hloc)
{
  if (!(es->style & GXES_MULTILINE))
    return;

  if (!hloc) {
    WARN("called with NULL handle\n");
    return;
  }

  EDIT_UnlockBuffer(es, TRUE);

  if(es->is_unicode)
  {
    if(es->hloc32A)
    {
      gxLocalFree(es->hloc32A);
      es->hloc32A = NULL;
    }
    es->hloc32W = hloc;
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

    if(es->hloc32W)
      gxLocalFree(es->hloc32W);

    es->hloc32W = hloc32W_new;
    es->hloc32A = hloc;
  }

  es->buffer_size = (GXUINT)gxLocalSize(es->hloc32W)/sizeof(GXWCHAR) - 1;

  es->flags |= EF_APP_HAS_HANDLE;
  EDIT_LockBuffer(es);

  es->x_offset = es->y_offset = 0;
  es->selection_start = es->selection_end = 0;
  EDIT_EM_EmptyUndoBuffer(es);
  es->flags &= ~EF_MODIFIED;
  es->flags &= ~EF_UPDATE;
  EDIT_BuildLineDefs_ML(es, 0, get_text_length(es), 0, NULL);
  EDIT_UpdateText(es, NULL, TRUE);
  EDIT_EM_ScrollCaret(es);
  /* force scroll info update */
  EDIT_UpdateScrollInfo(es);
}


/*********************************************************************
 *
 *  EM_SETLIMITTEXT
 *
 *  NOTE: this version currently implements WinNT limits
 *
 */
static void EDIT_EM_SetLimitText(EDITSTATE *es, GXUINT limit)
{
    if (!limit) limit = ~0u;
    if (!(es->style & GXES_MULTILINE)) limit = min(limit, 0x7ffffffe);
    es->buffer_limit = limit;
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

static void EDIT_EM_SetMargins(EDITSTATE *es, GXINT action,
             GXWORD left, GXWORD right, GXBOOL repaint)
{
  GXTEXTMETRICW tm;
  GXINT default_left_margin  = 0; /* in pixels */
  GXINT default_right_margin = 0; /* in pixels */

        /* Set the default margins depending on the font */
        if (es->font && (left == EC_USEFONTINFO || right == EC_USEFONTINFO)) {
            GXHDC dc = gxGetDC(es->hwndSelf);
            GXHFONT old_font = (GXHFONT)gxSelectObject(dc, es->font);
            gxGetTextMetricsW(dc, &tm);
            /* The default margins are only non zero for TrueType or Vector fonts */
            if (tm.tmPitchAndFamily & ( TMPF_VECTOR | TMPF_TRUETYPE )) {
                int min_size;
                GXRECT rc;
                /* This must be calculated more exactly! But how? */
                default_left_margin = tm.tmAveCharWidth / 2;
                default_right_margin = tm.tmAveCharWidth / 2;
                min_size = calc_min_set_margin_size(dc, default_left_margin, default_right_margin);
                gxGetClientRect(es->hwndSelf, &rc);
                if(rc.right - rc.left < min_size) {
                    default_left_margin = es->left_margin;
                    default_right_margin = es->right_margin;
                }
            }
            gxSelectObject(dc, old_font);
            gxReleaseDC(es->hwndSelf, dc);
        }

  if (action & EC_LEFTMARGIN) {
    es->format_rect.left -= es->left_margin;
    if (left != EC_USEFONTINFO)
      es->left_margin = left;
    else
      es->left_margin = default_left_margin;
    es->format_rect.left += es->left_margin;
  }

  if (action & EC_RIGHTMARGIN) {
    es->format_rect.right += es->right_margin;
    if (right != EC_USEFONTINFO)
       es->right_margin = right;
    else
      es->right_margin = default_right_margin;
    es->format_rect.right -= es->right_margin;
  }
  
  if (action & (EC_LEFTMARGIN | EC_RIGHTMARGIN)) {
    EDIT_AdjustFormatRect(es);
    if (repaint) EDIT_UpdateText(es, NULL, TRUE);
  }
  
  TRACE("left=%d, right=%d\n", es->left_margin, es->right_margin);
}


/*********************************************************************
 *
 *  EM_SETPASSWORDCHAR
 *
 */
static void EDIT_EM_SetPasswordChar(EDITSTATE *es, GXWCHAR c)
{
  GXLONG style;

  if (es->style & GXES_MULTILINE)
    return;

  if (es->password_char == c)
    return;

  style = (GXLONG)gxGetWindowLongW( es->hwndSelf, GWL_STYLE );
  es->password_char = c;
  if (c) {
    gxSetWindowLongW( es->hwndSelf, GWL_STYLE, style | ES_PASSWORD );
    es->style |= ES_PASSWORD;
  } else {
    gxSetWindowLongW( es->hwndSelf, GWL_STYLE, style & ~ES_PASSWORD );
    es->style &= ~ES_PASSWORD;
  }
  EDIT_UpdateText(es, NULL, TRUE);
}


/*********************************************************************
 *
 *  EM_SETTABSTOPS
 *
 */
static GXBOOL EDIT_EM_SetTabStops(EDITSTATE *es, GXINT count, const GXINT *tabs)
{
  if (!(es->style & GXES_MULTILINE))
    return FALSE;
        gxHeapFree(gxGetProcessHeap(), 0, es->tabs);
  es->tabs_count = count;
  if (!count)
    es->tabs = NULL;
  else {
    es->tabs = (GXLPINT)gxHeapAlloc(gxGetProcessHeap(), 0, count * sizeof(GXINT));
    memcpy(es->tabs, tabs, count * sizeof(GXINT));
  }
  return TRUE;
}


/*********************************************************************
 *
 *  EM_SETWORDBREAKPROC
 *
 */
static void EDIT_EM_SetWordBreakProc(EDITSTATE *es, void *wbp)
{
  if (es->word_break_proc == wbp)
    return;

  es->word_break_proc = wbp;

  if ((es->style & GXES_MULTILINE) && !(es->style & ES_AUTOHSCROLL)) {
    EDIT_BuildLineDefs_ML(es, 0, get_text_length(es), 0, NULL);
    EDIT_UpdateText(es, NULL, TRUE);
  }
}


/*********************************************************************
 *
 *  EM_UNDO / WM_UNDO
 *
 */
static GXBOOL EDIT_EM_Undo(EDITSTATE *es)
{
  GXINT ulength;
  GXLPWSTR utext;

  /* As per MSDN spec, for a single-line edit control,
     the return value is always TRUE */
  if( es->style & ES_READONLY )
            return !(es->style & GXES_MULTILINE);

  ulength = (GXINT)GXSTRLEN(es->undo_text);

  utext = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), 0, (ulength + 1) * sizeof(GXWCHAR));

  clstd::strcpyT(utext, es->undo_text);

  TRACEW(_CLTEXT("before UNDO:insertion length = %d, deletion buffer = %s\n"), es->undo_insert_count, debugstr_w(utext));

  EDIT_EM_SetSel(es, es->undo_position, es->undo_position + es->undo_insert_count, FALSE);
  EDIT_EM_EmptyUndoBuffer(es);
  EDIT_EM_ReplaceSel(es, TRUE, utext, TRUE, TRUE);
  EDIT_EM_SetSel(es, es->undo_position, es->undo_position + es->undo_insert_count, FALSE);
        /* send the notification after the selection start and end are set */
        EDIT_NOTIFY_PARENT(es, EN_CHANGE);
  EDIT_EM_ScrollCaret(es);
  gxHeapFree(gxGetProcessHeap(), 0, utext);

  TRACEW(_CLTEXT("after UNDO:insertion length = %d, deletion buffer = %s\n"), es->undo_insert_count, debugstr_w(es->undo_text));
  return TRUE;
}


/* Helper function for WM_CHAR
 *
 * According to an MSDN blog article titled "Just because you're a control
 * doesn't mean that you're necessarily inside a dialog box," multiline edit
 * controls without ES_WANTRETURN would attempt to detect whether it is inside
 * a dialog box or not.
 */
static inline GXBOOL EDIT_IsInsideDialog(EDITSTATE *es)
{
    return (es->flags & EF_DIALOGMODE);
}


/*********************************************************************
 *
 *  WM_PASTE
 *
 */
static void EDIT_WM_Paste(EDITSTATE *es)
{
  GXHGLOBAL hsrc;
  GXLPWSTR src;

  /* Protect read-only edit control from modification */
  if(es->style & ES_READONLY)
    return;

  gxOpenClipboard(es->hwndSelf);
  if ((hsrc = (GXHGLOBAL)gxGetClipboardData(CF_UNICODETEXT))) {
    src = (GXLPWSTR)gxGlobalLock(hsrc);
    EDIT_EM_ReplaceSel(es, TRUE, src, TRUE, TRUE);
    gxGlobalUnlock((GXHLOCAL)hsrc);
  }
  else if (es->style & ES_PASSWORD) {
    /* clear selected text in password edit box even with empty clipboard */
    EDIT_EM_ReplaceSel(es, TRUE, empty_stringW, TRUE, TRUE);
  }
  gxCloseClipboard();
}


/*********************************************************************
 *
 *  WM_COPY
 *
 */
static void EDIT_WM_Copy(EDITSTATE *es)
{
  GXINT s = min(es->selection_start, es->selection_end);
  GXINT e = max(es->selection_start, es->selection_end);
  GXHGLOBAL hdst;
  GXLPWSTR dst;
  GXDWORD len;

  if (e == s) return;

  len = e - s;
  hdst = gxGlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(GXWCHAR));
  dst = (GXLPWSTR)gxGlobalLock(hdst);
  memcpy(dst, es->text + s, len * sizeof(GXWCHAR));
  dst[len] = 0; /* ensure 0 termination */
  TRACEW(_CLTEXT("%s\n"), debugstr_w(dst));
  gxGlobalUnlock(hdst);
  gxOpenClipboard(es->hwndSelf);
  gxEmptyClipboard();
  gxSetClipboardData(CF_UNICODETEXT, (GXHANDLE)hdst);
  gxCloseClipboard();
}


/*********************************************************************
 *
 *  WM_CLEAR
 *
 */
static inline void EDIT_WM_Clear(EDITSTATE *es)
{
  /* Protect read-only edit control from modification */
  if(es->style & ES_READONLY)
      return;

  EDIT_EM_ReplaceSel(es, TRUE, empty_stringW, TRUE, TRUE);
}


/*********************************************************************
 *
 *  WM_CUT
 *
 */
static inline void EDIT_WM_Cut(EDITSTATE *es)
{
  EDIT_WM_Copy(es);
  EDIT_WM_Clear(es);
}


/*********************************************************************
 *
 *  WM_CHAR
 *
 */
static GXLRESULT EDIT_WM_Char(EDITSTATE *es, GXWCHAR c)
{
        GXBOOL control;

  control = GetKeyState(VK_CONTROL) & 0x8000;

  switch (c) {
  case '\r':
            /* If it's not a multiline edit box, it would be ignored below.
             * For multiline edit without ES_WANTRETURN, we have to make a
             * special case.
             */
            if ((es->style & GXES_MULTILINE) && !(es->style & ES_WANTRETURN))
                if (EDIT_IsInsideDialog(es))
                    break;
  case '\n':
    if (es->style & GXES_MULTILINE) {
      if (es->style & ES_READONLY) {
        EDIT_MoveHome(es, FALSE, FALSE);
        EDIT_MoveDown_ML(es, FALSE);
      } else {
        static const GXWCHAR cr_lfW[] = {'\r','\n',0};
        EDIT_EM_ReplaceSel(es, TRUE, cr_lfW, TRUE, TRUE);
      }
    }
    break;
  case '\t':
    if ((es->style & GXES_MULTILINE) && !(es->style & ES_READONLY))
    {
      static const GXWCHAR tabW[] = {'\t',0};
                        if (EDIT_IsInsideDialog(es))
                            break;
      EDIT_EM_ReplaceSel(es, TRUE, tabW, TRUE, TRUE);
    }
    break;
  case VK_BACK:
    if (!(es->style & ES_READONLY) && !control) {
      if (es->selection_start != es->selection_end)
        EDIT_WM_Clear(es);
      else {
        /* delete character left of caret */
        EDIT_EM_SetSel(es, (GXUINT)-1, 0, FALSE);
        EDIT_MoveBackward(es, TRUE);
        EDIT_WM_Clear(es);
      }
    }
    break;
  case 0x03: /* ^C */
    if (!(es->style & ES_PASSWORD))
        gxSendMessageW(es->hwndSelf, WM_COPY, 0, 0);
    break;
  case 0x16: /* ^V */
          if (!(es->style & ES_READONLY))
        gxSendMessageW(es->hwndSelf, WM_PASTE, 0, 0);
    break;
  case 0x18: /* ^X */
          if (!((es->style & ES_READONLY) || (es->style & ES_PASSWORD)))
        gxSendMessageW(es->hwndSelf, WM_CUT, 0, 0);
    break;
  case 0x1A: /* ^Z */
          if (!(es->style & ES_READONLY))
        gxSendMessageW(es->hwndSelf, WM_UNDO, 0, 0);
    break;

  default:
    /*If Edit control style is ES_NUMBER allow users to key in only numeric values*/
    if( (es->style & ES_NUMBER) && !( c >= '0' && c <= '9') )
      break;
      
    if (!(es->style & ES_READONLY) && (c >= ' ') && (c != 127)) {
      GXWCHAR str[2];
       str[0] = c;
       str[1] = '\0';
       EDIT_EM_ReplaceSel(es, TRUE, str, TRUE, TRUE);
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
static void EDIT_WM_Command(EDITSTATE *es, GXINT code, GXINT id, GXHWND control)
{
  if (code || control)
    return;

  switch (id) {
  case GXEM_UNDO:
    gxSendMessageW(es->hwndSelf, GXWM_UNDO, 0, 0);
    break;
  case GXWM_CUT:
    gxSendMessageW(es->hwndSelf, GXWM_CUT, 0, 0);
    break;
  case GXWM_COPY:
    gxSendMessageW(es->hwndSelf, GXWM_COPY, 0, 0);
    break;
  case GXWM_PASTE:
    gxSendMessageW(es->hwndSelf, GXWM_PASTE, 0, 0);
    break;
  case GXWM_CLEAR:
    gxSendMessageW(es->hwndSelf, GXWM_CLEAR, 0, 0);
    break;
  case GXEM_SETSEL:
    EDIT_EM_SetSel(es, 0, (GXUINT)-1, FALSE);
    EDIT_EM_ScrollCaret(es);
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
static void EDIT_WM_ContextMenu(EDITSTATE *es, GXINT x, GXINT y)
{
  GXHMENU menu = gxLoadMenuW(user32_module, _CLTEXT("EDITMENU"));
  GXHMENU popup = gxGetSubMenu(menu, 0);
  GXUINT start = es->selection_start;
  GXUINT end = es->selection_end;

  ORDER_GXUINT(start, end);

  /* undo */
  gxEnableMenuItem(popup, 0, MF_BYPOSITION | (EDIT_EM_CanUndo(es) && !(es->style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* cut */
  gxEnableMenuItem(popup, 2, MF_BYPOSITION | ((end - start) && !(es->style & ES_PASSWORD) && !(es->style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* copy */
  gxEnableMenuItem(popup, 3, MF_BYPOSITION | ((end - start) && !(es->style & ES_PASSWORD) ? MF_ENABLED : MF_GRAYED));
  /* paste */
  gxEnableMenuItem(popup, 4, MF_BYPOSITION | (IsClipboardFormatAvailable(CF_UNICODETEXT) && !(es->style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* delete */
  gxEnableMenuItem(popup, 5, MF_BYPOSITION | ((end - start) && !(es->style & ES_READONLY) ? MF_ENABLED : MF_GRAYED));
  /* select all */
  gxEnableMenuItem(popup, 7, MF_BYPOSITION | (start || (end != get_text_length(es)) ? MF_ENABLED : MF_GRAYED));

        if (x == -1 && y == -1) /* passed via VK_APPS press/release */
        {
            GXRECT rc;
            /* Windows places the menu at the edit's center in this case */
            //WIN_GetRectangles( es->hwndSelf, COORDS_SCREEN, NULL, &rc );
            gxGetWindowRect(es->hwndSelf, &rc);
            x = rc.left + (rc.right - rc.left) / 2;
            y = rc.top + (rc.bottom - rc.top) / 2;
        }

  if (!(es->flags & EF_FOCUSED))
            gxSetFocus(es->hwndSelf);

  gxTrackPopupMenu(popup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, es->hwndSelf, NULL);
  gxDestroyMenu(menu);
}


/*********************************************************************
 *
 *  WM_GETTEXT
 *
 */
static GXINT EDIT_WM_GetText(const EDITSTATE *es, GXINT count, GXLPWSTR dst, GXBOOL unicode)
{
  if(!count) return 0;

  if(unicode)
  {
    clstd::strcpynT(dst, es->text, count);
    return (GXINT)GXSTRLEN(dst);
  }
  else
  {
    GXLPSTR textA = (GXLPSTR)dst;
    if (!gxWideCharToMultiByte(CP_ACP, 0, es->text, -1, textA, count, NULL, NULL))
      textA[count - 1] = 0; /* ensure 0 termination */
    return (GXINT)strlen(textA);
  }
}

/*********************************************************************
 *
 *  EDIT_CheckCombo
 *
 */
static GXBOOL EDIT_CheckCombo(EDITSTATE *es, GXUINT msg, GXINT key)
{
   GXHWND hLBox = es->hwndListBox;
   GXHWND hCombo;
   GXBOOL bDropped;
   int  nEUI;

   if (!hLBox)
      return FALSE;

   hCombo   = gxGetParent(es->hwndSelf);
   bDropped = TRUE;
   nEUI     = 0;

   TRACE_(combo)(_CLTEXT("[%p]: handling msg %x (%x)\n"), es->hwndSelf, msg, key);

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
static GXLRESULT EDIT_WM_KeyDown(EDITSTATE *es, GXINT key)
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
    if (EDIT_CheckCombo(es, WM_KEYDOWN, key) || key == VK_F4)
      break;

    /* fall through */
  case VK_LEFT:
    if ((es->style & GXES_MULTILINE) && (key == VK_UP))
      EDIT_MoveUp_ML(es, shift);
    else
      if (control)
        EDIT_MoveWordBackward(es, shift);
      else
        EDIT_MoveBackward(es, shift);
    break;
  case VK_DOWN:
    if (EDIT_CheckCombo(es, WM_KEYDOWN, key))
      break;
    /* fall through */
  case VK_RIGHT:
    if ((es->style & GXES_MULTILINE) && (key == VK_DOWN))
      EDIT_MoveDown_ML(es, shift);
    else if (control)
      EDIT_MoveWordForward(es, shift);
    else
      EDIT_MoveForward(es, shift);
    break;
  case VK_HOME:
    EDIT_MoveHome(es, shift, control);
    break;
  case VK_END:
    EDIT_MoveEnd(es, shift, control);
    break;
  case VK_PRIOR:
    if (es->style & GXES_MULTILINE)
      EDIT_MovePageUp_ML(es, shift);
    else
      EDIT_CheckCombo(es, WM_KEYDOWN, key);
    break;
  case VK_NEXT:
    if (es->style & GXES_MULTILINE)
      EDIT_MovePageDown_ML(es, shift);
    else
      EDIT_CheckCombo(es, WM_KEYDOWN, key);
    break;
  case VK_DELETE:
    if (!(es->style & ES_READONLY) && !(shift && control)) {
      if (es->selection_start != es->selection_end) {
        if (shift)
          EDIT_WM_Cut(es);
        else
          EDIT_WM_Clear(es);
      } else {
        if (shift) {
          /* delete character left of caret */
          EDIT_EM_SetSel(es, (GXUINT)-1, 0, FALSE);
          EDIT_MoveBackward(es, TRUE);
          EDIT_WM_Clear(es);
        } else if (control) {
          /* delete to end of line */
          EDIT_EM_SetSel(es, (GXUINT)-1, 0, FALSE);
          EDIT_MoveEnd(es, TRUE, FALSE);
          EDIT_WM_Clear(es);
        } else {
          /* delete character right of caret */
          EDIT_EM_SetSel(es, (GXUINT)-1, 0, FALSE);
          EDIT_MoveForward(es, TRUE);
          EDIT_WM_Clear(es);
        }
      }
    }
    break;
  case VK_INSERT:
    if (shift) {
      if (!(es->style & ES_READONLY))
        EDIT_WM_Paste(es);
    } else if (control)
      EDIT_WM_Copy(es);
    break;
  case VK_RETURN:
      /* If the edit doesn't want the return send a message to the default object */
      if(!(es->style & GXES_MULTILINE) || !(es->style & ES_WANTRETURN))
      {
                GXDWORD dw;

                if (!EDIT_IsInsideDialog(es)) break;
                if (control) break;
                dw = (GXDWORD)gxSendMessageW(es->hwndParent, DM_GETDEFID, 0, 0);
                if (GXHIWORD(dw) == DC_HASDEFID)
                {
                    GXHWND hwDefCtrl = gxGetDlgItem(es->hwndParent, GXLOWORD(dw));
                    if (hwDefCtrl)
                    {
                        gxSendMessageW(es->hwndParent, WM_NEXTDLGCTL, (GXWPARAM)hwDefCtrl, TRUE);
                        gxPostMessageW(hwDefCtrl, WM_KEYDOWN, VK_RETURN, 0);
                    }
                }
      }
      break;
        case VK_ESCAPE:
            if ((es->style & GXES_MULTILINE) && EDIT_IsInsideDialog(es))
                gxPostMessageW(es->hwndParent, WM_CLOSE, 0, 0);
            break;
        case VK_TAB:
            if ((es->style & GXES_MULTILINE) && EDIT_IsInsideDialog(es))
                gxSendMessageW(es->hwndParent, WM_NEXTDLGCTL, shift, 0);
            break;
  }
        return TRUE;
}


/*********************************************************************
 *
 *  WM_KILLFOCUS
 *
 */
static GXLRESULT EDIT_WM_KillFocus(EDITSTATE *es)
{
  es->flags &= ~EF_FOCUSED;
  gxDestroyCaret();
  if(!(es->style & ES_NOHIDESEL))
    EDIT_InvalidateText(es, es->selection_start, es->selection_end);
  EDIT_NOTIFY_PARENT(es, EN_KILLFOCUS);
  return 0;
}


/*********************************************************************
 *
 *  WM_LBUTTONDBLCLK
 *
 *  The caret position has been set on the WM_LBUTTONDOWN message
 *
 */
static GXLRESULT EDIT_WM_LButtonDblClk(EDITSTATE *es)
{
  GXINT s;
  GXINT e = es->selection_end;
  GXINT l;
  GXINT li;
  GXINT ll;

  es->bCaptureState = TRUE;
  gxSetCapture(es->hwndSelf);

  l = EDIT_EM_LineFromChar(es, e);
  li = EDIT_EM_LineIndex(es, l);
  ll = EDIT_EM_LineLength(es, e);
  s = li + EDIT_CallWordBreakProc(es, li, e - li, ll, WB_LEFT);
  e = li + EDIT_CallWordBreakProc(es, li, e - li, ll, WB_RIGHT);
  EDIT_EM_SetSel(es, s, e, FALSE);
  EDIT_EM_ScrollCaret(es);
  es->region_posx = es->region_posy = 0;
  gxSetTimer(es->hwndSelf, 0, 100, NULL);
  return 0;
}


/*********************************************************************
 *
 *  WM_LBUTTONDOWN
 *
 */
static GXLRESULT EDIT_WM_LButtonDown(EDITSTATE *es, GXDWORD keys, GXINT x, GXINT y)
{
  GXINT e;
  GXBOOL after_wrap;

  es->bCaptureState = TRUE;
  gxSetCapture(es->hwndSelf);
  EDIT_ConfinePoint(es, &x, &y);
  e = EDIT_CharFromPos(es, x, y, &after_wrap);
  EDIT_EM_SetSel(es, (keys & MK_SHIFT) ? es->selection_start : e, e, after_wrap);
  EDIT_EM_ScrollCaret(es);
  es->region_posx = es->region_posy = 0;
  gxSetTimer(es->hwndSelf, 0, 100, NULL);

  if (!(es->flags & EF_FOCUSED))
            gxSetFocus(es->hwndSelf);

  return 0;
}


/*********************************************************************
 *
 *  WM_LBUTTONUP
 *
 */
static GXLRESULT EDIT_WM_LButtonUp(EDITSTATE *es)
{
  if (es->bCaptureState) {
    gxKillTimer(es->hwndSelf, 0);
    if (gxGetCapture() == es->hwndSelf) gxReleaseCapture();
  }
  es->bCaptureState = FALSE;
  return 0;
}


/*********************************************************************
 *
 *  WM_MBUTTONDOWN
 *
 */
static GXLRESULT EDIT_WM_MButtonDown(EDITSTATE *es)
{
    gxSendMessageW(es->hwndSelf, WM_PASTE, 0, 0);
    return 0;
}


/*********************************************************************
 *
 *  WM_MOUSEMOVE
 *
 */
static GXLRESULT EDIT_WM_MouseMove(EDITSTATE *es, GXINT x, GXINT y)
{
  GXINT e;
  GXBOOL after_wrap;
  GXINT prex, prey;

        /* If the mouse has been captured by process other than the edit control itself,
         * the windows edit controls will not select the strings with mouse move.
         */
        if (!es->bCaptureState || gxGetCapture() != es->hwndSelf)
    return 0;

  /*
   *  FIXME: gotta do some scrolling if outside client
   *    area.  Maybe reset the timer ?
   */
  prex = x; prey = y;
  EDIT_ConfinePoint(es, &x, &y);
  es->region_posx = (prex < x) ? -1 : ((prex > x) ? 1 : 0);
  es->region_posy = (prey < y) ? -1 : ((prey > y) ? 1 : 0);
  e = EDIT_CharFromPos(es, x, y, &after_wrap);
  EDIT_EM_SetSel(es, es->selection_start, e, after_wrap);
  EDIT_SetCaretPos(es,es->selection_end,es->flags & EF_AFTER_WRAP);
  return 0;
}


/*********************************************************************
 *
 *  WM_PAINT
 *
 */
static void EDIT_WM_Paint(EDITSTATE *es, GXHDC hdc)
{
  GXPAINTSTRUCT ps;
  GXINT i;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXRECT rc;
  GXRECT rcClient;
  GXRECT rcLine;
  GXRECT rcRgn;
  GXHBRUSH brush;
  GXHBRUSH old_brush;
  GXINT bw, bh;
  GXBOOL rev = es->bEnableState &&
    ((es->flags & EF_FOCUSED) ||
    (es->style & ES_NOHIDESEL));
  dc = hdc ? hdc : gxBeginPaint(es->hwndSelf, &ps);

  gxGetClientRect(es->hwndSelf, &rcClient);

  /* get the background brush */
  brush = EDIT_NotifyCtlColor(es, dc);

  /* paint the border and the background */
  gxIntersectClipRect(dc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

  if(es->style & GXWS_BORDER) {
    bw = gxGetSystemMetrics(GXSM_CXBORDER);
    bh = gxGetSystemMetrics(GXSM_CYBORDER);
    rc = rcClient;
    if(es->style & GXES_MULTILINE) {
      if(es->style & GXWS_HSCROLL) rc.bottom+=bh;
      if(es->style & GXWS_VSCROLL) rc.right+=bw;
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
  gxFillRect(dc, &rc, brush);

  gxIntersectClipRect(dc, es->format_rect.left,
    es->format_rect.top,
    es->format_rect.right,
    es->format_rect.bottom);
  if (es->style & GXES_MULTILINE) {
    rc = rcClient;
    gxIntersectClipRect(dc, rc.left, rc.top, rc.right, rc.bottom);
  }
  if (es->font)
    old_font = (GXHFONT)gxSelectObject(dc, es->font);

  if (!es->bEnableState)
    gxSetTextColor(dc, gxGetSysColor(GXCOLOR_GRAYTEXT));
  gxGetClipBox(dc, &rcRgn);
  if (es->style & GXES_MULTILINE) {
    GXINT vlc = get_vertical_line_count(es);
    for (i = es->y_offset ; i <= min(es->y_offset + vlc, es->y_offset + es->line_count - 1) ; i++) {
      EDIT_GetLineRect(es, i, 0, -1, &rcLine);
      if (gxIntersectRect(&rc, &rcRgn, &rcLine))
        EDIT_PaintLine(es, dc, i, rev);
    }
  } else {
    EDIT_GetLineRect(es, 0, 0, -1, &rcLine);
    if (gxIntersectRect(&rc, &rcRgn, &rcLine))
      EDIT_PaintLine(es, dc, 0, rev);
  }
  if (es->font)
    gxSelectObject(dc, old_font);

  if (!hdc)
    gxEndPaint(es->hwndSelf, &ps);
}


/*********************************************************************
 *
 *  WM_SETFOCUS
 *
 */
static void EDIT_WM_SetFocus(EDITSTATE *es)
{
  es->flags |= EF_FOCUSED;

        if (!(es->style & GXES_NOHIDESEL))
            EDIT_InvalidateText(es, es->selection_start, es->selection_end);

        /* single line edit updates itself */
        if (!(es->style & GXES_MULTILINE))
        {
            GXHDC hdc = gxGetDC(es->hwndSelf);
            EDIT_WM_Paint(es, hdc);
            gxReleaseDC(es->hwndSelf, hdc);
        }

  gxCreateCaret(es->hwndSelf, 0, 1, es->line_height);
  EDIT_SetCaretPos(es, es->selection_end,
       es->flags & EF_AFTER_WRAP);
  gxShowCaret(es->hwndSelf);
  EDIT_NOTIFY_PARENT(es, EN_SETFOCUS);
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
static void EDIT_WM_SetFont(EDITSTATE *es, GXHFONT font, GXBOOL redraw)
{
  GXTEXTMETRICW tm;
  GXHDC dc;
  GXHFONT old_font = 0;
  GXRECT clientRect;

  es->font = font;
  dc = gxGetDC(es->hwndSelf);
  if (font)
    old_font = (GXHFONT)gxSelectObject(dc, font);
  gxGetTextMetricsW(dc, &tm);
  es->line_height = tm.tmHeight;
  es->char_width = tm.tmAveCharWidth;
  if (font)
    gxSelectObject(dc, old_font);
  gxReleaseDC(es->hwndSelf, dc);
  
  /* Reset the format rect and the margins */
  gxGetClientRect(es->hwndSelf, &clientRect);
  EDIT_SetRectNP(es, &clientRect);
  EDIT_EM_SetMargins(es, EC_LEFTMARGIN | EC_RIGHTMARGIN,
         EC_USEFONTINFO, EC_USEFONTINFO, FALSE);

  if (es->style & GXES_MULTILINE)
    EDIT_BuildLineDefs_ML(es, 0, get_text_length(es), 0, NULL);
  else
      EDIT_CalcLineWidth_SL(es);

  if (redraw)
    EDIT_UpdateText(es, NULL, TRUE);
  if (es->flags & EF_FOCUSED) {
    gxDestroyCaret();
    gxCreateCaret(es->hwndSelf, 0, 1, es->line_height);
    EDIT_SetCaretPos(es, es->selection_end,
         es->flags & EF_AFTER_WRAP);
    gxShowCaret(es->hwndSelf);
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
static void EDIT_WM_SetText(EDITSTATE *es, GXLPCWSTR text, GXBOOL unicode)
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

    if (es->flags & EF_UPDATE) {
      /* fixed this bug once; complain if we see it about to happen again. */
      ERR("SetSel may generate UPDATE message whose handler may reset "
        "selection.\n");
    }

    EDIT_EM_SetSel(es, 0, (GXUINT)-1, FALSE);
    if (text) 
    {
  TRACEW(_CLTEXT("%s\n"), debugstr_w(text));
  EDIT_EM_ReplaceSel(es, FALSE, text, FALSE, FALSE);
  if(!unicode)
      gxHeapFree(gxGetProcessHeap(), 0, textW);
    } 
    else 
    {
  TRACEW(_CLTEXT("<NULL>\n"));
  EDIT_EM_ReplaceSel(es, FALSE, empty_stringW, FALSE, FALSE);
    }
    es->x_offset = 0;
    es->flags &= ~EF_MODIFIED;
    EDIT_EM_SetSel(es, 0, 0, FALSE);

    /* Send the notification after the selection start and end have been set
     * edit control doesn't send notification on WM_SETTEXT
     * if it is multiline, or it is part of combobox
     */
    if( !((es->style & GXES_MULTILINE) || es->hwndListBox))
    {
        EDIT_NOTIFY_PARENT(es, EN_UPDATE);
        EDIT_NOTIFY_PARENT(es, EN_CHANGE);
    }
    EDIT_EM_ScrollCaret(es);
    EDIT_UpdateScrollInfo(es);    
}


/*********************************************************************
 *
 *  WM_SIZE
 *
 */
static void EDIT_WM_Size(EDITSTATE *es, GXUINT action, GXINT width, GXINT height)
{
  if ((action == SIZE_MAXIMIZED) || (action == SIZE_RESTORED)) {
    GXRECT rc;
    TRACE("width = %d, height = %d\n", width, height);
    gxSetRect(&rc, 0, 0, width, height);
    EDIT_SetRectNP(es, &rc);
    EDIT_UpdateText(es, NULL, TRUE);
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
static GXLRESULT  EDIT_WM_StyleChanged ( EDITSTATE *es, GXWPARAM which, const STYLESTRUCT *style)
{
        if (GWL_STYLE == which) {
                GXDWORD style_change_mask;
                GXDWORD new_style;
                /* Only a subset of changes can be applied after the control
                 * has been created.
                 */
                style_change_mask = ES_UPPERCASE | ES_LOWERCASE |
                                    ES_NUMBER;
                if (es->style & GXES_MULTILINE)
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

                es->style = (es->style & ~style_change_mask) | new_style;
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
static GXLRESULT EDIT_WM_SysKeyDown(EDITSTATE *es, GXINT key, GXDWORD key_data)
{
  if ((key == VK_BACK) && (key_data & 0x2000)) {
    if (EDIT_EM_CanUndo(es))
      EDIT_EM_Undo(es);
    return 0;
  } else if (key == VK_UP || key == VK_DOWN) {
    if (EDIT_CheckCombo(es, WM_SYSKEYDOWN, key))
      return 0;
  }
  return gxDefWindowProcW(es->hwndSelf, WM_SYSKEYDOWN, key, key_data);
}


/*********************************************************************
 *
 *  WM_TIMER
 *
 */
static void EDIT_WM_Timer(EDITSTATE *es)
{
  if (es->region_posx < 0) {
    EDIT_MoveBackward(es, TRUE);
  } else if (es->region_posx > 0) {
    EDIT_MoveForward(es, TRUE);
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
static GXLRESULT EDIT_WM_HScroll(EDITSTATE *es, GXINT action, GXINT pos)
{
  GXINT dx;
  GXINT fw;

  if (!(es->style & GXES_MULTILINE))
    return 0;

  if (!(es->style & ES_AUTOHSCROLL))
    return 0;

  dx = 0;
  fw = es->format_rect.right - es->format_rect.left;
  switch (action) {
  case SB_LINELEFT:
    TRACE("SB_LINELEFT\n");
    if (es->x_offset)
      dx = -es->char_width;
    break;
  case SB_LINERIGHT:
    TRACE("SB_LINERIGHT\n");
    if (es->x_offset < es->text_width)
      dx = es->char_width;
    break;
  case SB_PAGELEFT:
    TRACE("SB_PAGELEFT\n");
    if (es->x_offset)
      dx = -fw / HSCROLL_FRACTION / es->char_width * es->char_width;
    break;
  case SB_PAGERIGHT:
    TRACE("SB_PAGERIGHT\n");
    if (es->x_offset < es->text_width)
      dx = fw / HSCROLL_FRACTION / es->char_width * es->char_width;
    break;
  case SB_LEFT:
    TRACE("SB_LEFT\n");
    if (es->x_offset)
      dx = -es->x_offset;
    break;
  case SB_RIGHT:
    TRACE("SB_RIGHT\n");
    if (es->x_offset < es->text_width)
      dx = es->text_width - es->x_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    es->flags |= EF_HSCROLL_TRACK;
    if(es->style & GXWS_HSCROLL)
        dx = pos - es->x_offset;
    else
    {
        GXINT fw, new_x;
        /* Sanity check */
        if(pos < 0 || pos > 100) return 0;
        /* Assume default scroll range 0-100 */
        fw = es->format_rect.right - es->format_rect.left;
        new_x = pos * (es->text_width - fw) / 100;
        dx = es->text_width ? (new_x - es->x_offset) : 0;
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    es->flags &= ~EF_HSCROLL_TRACK;
    if(gxGetWindowLongW( es->hwndSelf, GWL_STYLE ) & GXWS_HSCROLL)
        dx = pos - es->x_offset;
    else
    {
        GXINT fw, new_x;
        /* Sanity check */
        if(pos < 0 || pos > 100) return 0;
        /* Assume default scroll range 0-100 */
        fw = es->format_rect.right - es->format_rect.left;
        new_x = pos * (es->text_width - fw) / 100;
        dx = es->text_width ? (new_x - es->x_offset) : 0;
    }
    if (!dx) {
      /* force scroll info update */
      EDIT_UpdateScrollInfo(es);
      EDIT_NOTIFY_PARENT(es, EN_HSCROLL);
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
    if(gxGetWindowLongW( es->hwndSelf, GWL_STYLE ) & GXWS_HSCROLL)
        ret = gxGetScrollPos(es->hwndSelf, SB_HORZ);
    else
    {
        /* Assume default scroll range 0-100 */
        GXINT fw = es->format_rect.right - es->format_rect.left;
        ret = es->text_width ? es->x_offset * 100 / (es->text_width - fw) : 0;
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
      GXINT fw = es->format_rect.right - es->format_rect.left;
      /* check if we are going to move too far */
      if(es->x_offset + dx + fw > es->text_width)
    dx = es->text_width - fw - es->x_offset;
      if(dx)
    EDIT_EM_LineScroll_internal(es, dx, 0);
  }
  return 0;
}


/*********************************************************************
 *
 *  WM_VSCROLL
 *
 */
static GXLRESULT EDIT_WM_VScroll(EDITSTATE *es, GXINT action, GXINT pos)
{
  GXINT dy;

  if (!(es->style & GXES_MULTILINE))
    return 0;

  if (!(es->style & ES_AUTOVSCROLL))
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
    EDIT_EM_Scroll(es, action);
    return 0;
  case SB_TOP:
    TRACE("SB_TOP\n");
    dy = -es->y_offset;
    break;
  case SB_BOTTOM:
    TRACE("SB_BOTTOM\n");
    dy = es->line_count - 1 - es->y_offset;
    break;
  case SB_THUMBTRACK:
    TRACE("SB_THUMBTRACK %d\n", pos);
    es->flags |= EF_VSCROLL_TRACK;
    if(es->style & GXWS_VSCROLL)
        dy = pos - es->y_offset;
    else
    {
        /* Assume default scroll range 0-100 */
        GXINT vlc, new_y;
        /* Sanity check */
        if(pos < 0 || pos > 100) return 0;
        vlc = get_vertical_line_count(es);
        new_y = pos * (es->line_count - vlc) / 100;
        dy = es->line_count ? (new_y - es->y_offset) : 0;
        TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
          es->line_count, es->y_offset, pos, dy);
    }
    break;
  case SB_THUMBPOSITION:
    TRACE("SB_THUMBPOSITION %d\n", pos);
    es->flags &= ~EF_VSCROLL_TRACK;
    if(es->style & GXWS_VSCROLL)
        dy = pos - es->y_offset;
    else
    {
        /* Assume default scroll range 0-100 */
        GXINT vlc, new_y;
        /* Sanity check */
        if(pos < 0 || pos > 100) return 0;
        vlc = get_vertical_line_count(es);
        new_y = pos * (es->line_count - vlc) / 100;
        dy = es->line_count ? (new_y - es->y_offset) : 0;
        TRACE("line_count=%d, y_offset=%d, pos=%d, dy = %d\n",
          es->line_count, es->y_offset, pos, dy);
    }
    if (!dy)
    {
      /* force scroll info update */
      EDIT_UpdateScrollInfo(es);
      EDIT_NOTIFY_PARENT(es, EN_VSCROLL);
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
    if(gxGetWindowLongW( es->hwndSelf, GWL_STYLE ) & GXWS_VSCROLL)
        ret = gxGetScrollPos(es->hwndSelf, SB_VERT);
    else
    {
        /* Assume default scroll range 0-100 */
        GXINT vlc = get_vertical_line_count(es);
        ret = es->line_count ? es->y_offset * 100 / (es->line_count - vlc) : 0;
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
    EDIT_EM_LineScroll(es, 0, dy);
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
static GXLRESULT EDIT_EM_GetThumb(EDITSTATE *es)
{
  return MAKELONG(EDIT_WM_VScroll(es, EM_GETTHUMB, 0),
                        EDIT_WM_HScroll(es, EM_GETTHUMB, 0));
}


/********************************************************************
 * 
 * The Following code is to handle inline editing from IMEs
 */

static void EDIT_GetCompositionStr(GXHIMC hIMC, GXLPARAM CompFlag, EDITSTATE *es)
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
    if (es->selection_end < es->composition_start)
        es->composition_start = es->selection_end;
    
    /* replace existing selection string */
    es->selection_start = es->composition_start;

    if (es->composition_len > 0)
        es->selection_end = es->composition_start + es->composition_len;
    else
        es->selection_end = es->selection_start;

    EDIT_EM_ReplaceSel(es, FALSE, lpCompStr, TRUE, TRUE);
    es->composition_len = abs(es->composition_start - es->selection_end);

    es->selection_start = es->composition_start;
    es->selection_end = es->selection_start + es->composition_len;

    gxHeapFree(gxGetProcessHeap(),0,lpCompStrAttr);
    gxHeapFree(gxGetProcessHeap(),0,lpCompStr);
}

static void EDIT_GetResultStr(GXHIMC hIMC, EDITSTATE *es)
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
    if (es->selection_end < es->composition_start)
        es->composition_start = es->selection_end;

    es->selection_start = es->composition_start;
    es->selection_end = es->composition_start + es->composition_len;
    EDIT_EM_ReplaceSel(es, TRUE, lpResultStr, TRUE, TRUE);
    es->composition_start = es->selection_end;
    es->composition_len = 0;

    gxHeapFree(gxGetProcessHeap(),0,lpResultStr);
}

static void EDIT_ImeComposition(GXHWND hwnd, GXLPARAM CompFlag, EDITSTATE *es)
{
    GXHIMC hIMC;
    int cursor;

    if (es->composition_len == 0 && es->selection_start != es->selection_end)
    {
        EDIT_EM_ReplaceSel(es, TRUE, empty_stringW, TRUE, TRUE);
        es->composition_start = es->selection_end;
    }

    hIMC = gxImmGetContext(hwnd);
    if (!hIMC)
        return;

    if (CompFlag & GCS_RESULTSTR)
        EDIT_GetResultStr(hIMC, es);
    if (CompFlag & GCS_COMPSTR)
        EDIT_GetCompositionStr(hIMC, CompFlag, es);
    cursor = gxImmGetCompositionStringW(hIMC, GCS_CURSORPOS, 0, 0);
    gxImmReleaseContext(hwnd, hIMC);
    EDIT_SetCaretPos(es, es->selection_start + cursor, es->flags & EF_AFTER_WRAP);
}


/*********************************************************************
 *
 *  WM_NCCREATE
 *
 * See also EDIT_WM_StyleChanged
 */
static GXLRESULT EDIT_WM_NCCreate(GXHWND hwnd, GXLPCREATESTRUCTW lpcs, GXBOOL unicode)
{
  EDITSTATE *es;
  GXUINT alloc_size;

  TRACEW(_CLTEXT("Creating %s edit control, style = %08x\n"), unicode ? "Unicode" : "ANSI", lpcs->style);

  if (!(es = (EDITSTATE*)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, sizeof(*es))))
    return FALSE;
        gxSetWindowLongPtrW( hwnd, 0, (GXLONG_PTR)es );

       /*
        *      Note: since the EDITSTATE has not been fully initialized yet,
        *            we can't use any API calls that may send
        *            WM_XXX messages before WM_NCCREATE is completed.
        */

  es->is_unicode = unicode;
  es->style = lpcs->style;

        es->bEnableState = !(es->style & GXWS_DISABLED);

  es->hwndSelf = hwnd;
  /* Save parent, which will be notified by EN_* messages */
  es->hwndParent = lpcs->hwndParent;

  //if (es->style & ES_COMBO)
  //   es->hwndListBox = GetDlgItem(es->hwndParent, ID_CB_LISTBOX);

        /* FIXME: should we handle changes to GXWS_EX_RIGHT style after creation? */
        if (lpcs->dwExStyle & GXWS_EX_RIGHT) es->style |= ES_RIGHT;

        /* Number overrides lowercase overrides uppercase (at least it
         * does in Win95).  However I'll bet that ES_NUMBER would be
         * invalid under Win 3.1.
         */
        if (es->style & ES_NUMBER) {
                ; /* do not override the ES_NUMBER */
        }  else if (es->style & ES_LOWERCASE) {
                es->style &= ~ES_UPPERCASE;
        }
  if (es->style & GXES_MULTILINE) {
    es->buffer_limit = BUFLIMIT_INITIAL;
    if (es->style & GXWS_VSCROLL)
      es->style |= ES_AUTOVSCROLL;
    if (es->style & GXWS_HSCROLL)
      es->style |= ES_AUTOHSCROLL;
    es->style &= ~ES_PASSWORD;
    if ((es->style & ES_CENTER) || (es->style & ES_RIGHT)) {
                        /* Confirmed - RIGHT overrides CENTER */
      if (es->style & ES_RIGHT)
        es->style &= ~ES_CENTER;
      es->style &= ~GXWS_HSCROLL;
      es->style &= ~ES_AUTOHSCROLL;
    }
  } else {
    es->buffer_limit = BUFLIMIT_INITIAL;
    if ((es->style & ES_RIGHT) && (es->style & ES_CENTER))
      es->style &= ~ES_CENTER;
    es->style &= ~GXWS_HSCROLL;
    es->style &= ~GXWS_VSCROLL;
    if (es->style & ES_PASSWORD)
      es->password_char = '*';
  }

  alloc_size = ROUND_TO_GROW((es->buffer_size + 1) * sizeof(GXWCHAR));
  if(!(es->hloc32W = gxLocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, alloc_size)))
      goto cleanup;
  es->buffer_size = (GXUINT)gxLocalSize(es->hloc32W)/sizeof(GXWCHAR) - 1;

  if (!(es->undo_text = (GXLPWSTR)gxHeapAlloc(gxGetProcessHeap(), GXHEAP_ZERO_MEMORY, (es->buffer_size + 1) * sizeof(GXWCHAR))))
    goto cleanup;
  es->undo_buffer_size = es->buffer_size;

  if (es->style & GXES_MULTILINE)
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
    es->style &= ~GXWS_BORDER;
        else if (es->style & GXWS_BORDER)
    gxSetWindowLongW(hwnd, GWL_STYLE, es->style & ~GXWS_BORDER);

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
static GXLRESULT EDIT_WM_Create(EDITSTATE *es, GXLPCWSTR name)
{
  GXRECT clientRect;

  TRACEW(_CLTEXT("%s\n"), debugstr_w(name));
  /*
  *  To initialize some final structure members, we call some helper
  *  functions.  However, since the EDITSTATE is not consistent (i.e.
  *  not fully initialized), we should be very careful which
  *  functions can be called, and in what order.
  */
  EDIT_WM_SetFont(es, 0, FALSE);
  EDIT_EM_EmptyUndoBuffer(es);

  /* We need to calculate the format rect
  (applications may send EM_SETMARGINS before the control gets visible) */
  gxGetClientRect(es->hwndSelf, &clientRect);
  EDIT_SetRectNP(es, &clientRect);

  if (name && *name) {
    EDIT_EM_ReplaceSel(es, FALSE, name, FALSE, FALSE);
    /* if we insert text to the editline, the text scrolls out
    * of the window, as the caret is placed after the insert
    * pos normally; thus we reset es->selection... to 0 and
    * update caret
    */
    es->selection_start = es->selection_end = 0;
    /* Adobe Photoshop does NOT like this. and MSDN says that EN_CHANGE
    * Messages are only to be sent when the USER does something to
    * change the contents. So I am removing this EN_CHANGE
    *
    * EDIT_NOTIFY_PARENT(es, EN_CHANGE);
    */
    EDIT_EM_ScrollCaret(es);
  }
  /* force scroll info update */
  EDIT_UpdateScrollInfo(es);
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
static GXLRESULT EDIT_WM_NCDestroy(EDITSTATE *es)
{
  LINEDEF *pc, *pp;

  if (es->hloc32W) {
    gxLocalFree(es->hloc32W);
  }
  if (es->hloc32A) {
    gxLocalFree(es->hloc32A);
  }
  pc = es->first_line_def;
  while (pc)
  {
    pp = pc->next;
    gxHeapFree(gxGetProcessHeap(), 0, pc);
    pc = pp;
  }

  gxSetWindowLongPtrW( es->hwndSelf, 0, 0 );
  gxHeapFree(gxGetProcessHeap(), 0, es->undo_text);
  gxHeapFree(gxGetProcessHeap(), 0, es);

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
  EDITSTATE *es = (EDITSTATE *)gxGetWindowLongPtrW( hwnd, 0 );
  GXLRESULT result = 0;

  TRACEW(_CLTEXT("hwnd=%p msg=%x (%s) wparam=%lx lparam=%lx\n"), hwnd, msg, SPY_GetMsgName(msg, hwnd), wParam, lParam);

  if (!es && msg != GXWM_NCCREATE)
    return DefWindowProcT(hwnd, msg, wParam, lParam, unicode);

  if (es && (msg != WM_NCDESTROY)) EDIT_LockBuffer(es);

  switch (msg) {
  case GXEM_GETSEL:
    result = EDIT_EM_GetSel(es, (GXLPUINT)wParam, (GXLPUINT)lParam);
    break;

  case GXEM_SETSEL:
    EDIT_EM_SetSel(es, (GXUINT)wParam, (GXUINT)lParam, FALSE);
    EDIT_EM_ScrollCaret(es);
    result = 1;
    break;

  case GXEM_GETRECT:
    if (lParam)
      gxCopyRect((GXLPRECT)lParam, &es->format_rect);
    break;

  case GXEM_SETRECT:
    if ((es->style & GXES_MULTILINE) && lParam) {
      EDIT_SetRectNP(es, (GXLPRECT)lParam);
      EDIT_UpdateText(es, NULL, TRUE);
    }
    break;

  case GXEM_SETRECTNP:
    if ((es->style & GXES_MULTILINE) && lParam)
      EDIT_SetRectNP(es, (GXLPRECT)lParam);
    break;

  case GXEM_SCROLL:
    result = EDIT_EM_Scroll(es, (GXINT)wParam);
    break;

  case GXEM_LINESCROLL:
    result = (GXLRESULT)EDIT_EM_LineScroll(es, (GXINT)wParam, (GXINT)lParam);
    break;

  case GXEM_SCROLLCARET:
    EDIT_EM_ScrollCaret(es);
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
    result = (es->style & GXES_MULTILINE) ? es->line_count : 1;
    break;

  case GXEM_LINEINDEX:
    result = (GXLRESULT)EDIT_EM_LineIndex(es, (GXINT)wParam);
    break;

  case GXEM_SETHANDLE:
    EDIT_EM_SetHandle(es, (GXHLOCAL)wParam);
    break;

  case GXEM_GETHANDLE:
    result = (GXLRESULT)EDIT_EM_GetHandle(es);
    break;

  case GXEM_GETTHUMB:
    result = EDIT_EM_GetThumb(es);
    break;

    /* these messages missing from specs */
  case 0x00bf:
  case 0x00c0:
  case 0x00c3:
  case 0x00ca:
    FIXME("undocumented message 0x%x, please report\n", msg);
    result = gxDefWindowProcW(hwnd, msg, wParam, lParam);
    break;

  case GXEM_LINELENGTH:
    result = (GXLRESULT)EDIT_EM_LineLength(es, (GXINT)wParam);
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

      EDIT_EM_ReplaceSel(es, (GXBOOL)wParam, textW, TRUE, TRUE);
      result = 1;

      if(!unicode)
        gxHeapFree(gxGetProcessHeap(), 0, textW);
      break;
    }

  case GXEM_GETLINE:
    result = (GXLRESULT)EDIT_EM_GetLine(es, (GXINT)wParam, (GXLPWSTR)lParam, unicode);
    break;

  case GXEM_SETLIMITTEXT:
    EDIT_EM_SetLimitText(es, (GXUINT)wParam);
    break;

  case GXEM_CANUNDO:
    result = (GXLRESULT)EDIT_EM_CanUndo(es);
    break;

  case GXEM_UNDO:
  case GXWM_UNDO:
    result = (GXLRESULT)EDIT_EM_Undo(es);
    break;

  case GXEM_FMTLINES:
    result = (GXLRESULT)EDIT_EM_FmtLines(es, (GXBOOL)wParam);
    break;

  case GXEM_LINEFROMCHAR:
    result = (GXLRESULT)EDIT_EM_LineFromChar(es, (GXINT)wParam);
    break;

  case GXEM_SETTABSTOPS:
    result = (GXLRESULT)EDIT_EM_SetTabStops(es, (GXINT)wParam, (LPINT)lParam);
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

      EDIT_EM_SetPasswordChar(es, charW);
      break;
    }

  case GXEM_EMPTYUNDOBUFFER:
    EDIT_EM_EmptyUndoBuffer(es);
    break;

  case GXEM_GETFIRSTVISIBLELINE:
    result = (es->style & GXES_MULTILINE) ? es->y_offset : es->x_offset;
    break;

  case GXEM_SETREADONLY:
    {
      GXDWORD old_style = es->style;

      if (wParam) {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) | ES_READONLY );
        es->style |= ES_READONLY;
      } else {
        gxSetWindowLongW( hwnd, GWL_STYLE,
          gxGetWindowLongW( hwnd, GWL_STYLE ) & ~ES_READONLY );
        es->style &= ~ES_READONLY;
      }

      if (old_style ^ es->style)
        gxInvalidateRect(es->hwndSelf, NULL, TRUE);

      result = 1;
      break;
    }

  case GXEM_SETWORDBREAKPROC:
    EDIT_EM_SetWordBreakProc(es, (void *)lParam);
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
    EDIT_EM_SetMargins(es, (GXINT)wParam, GXLOWORD(lParam), GXHIWORD(lParam), TRUE);
    break;

  case GXEM_GETMARGINS:
    result = MAKELONG(es->left_margin, es->right_margin);
    break;

  case GXEM_GETLIMITTEXT:
    result = es->buffer_limit;
    break;

  case GXEM_POSFROMCHAR:
    if ((GXINT)wParam >= get_text_length(es)) result = -1;
    else result = EDIT_EM_PosFromChar(es, (GXINT)wParam, FALSE);
    break;

  case GXEM_CHARFROMPOS:
    result = EDIT_EM_CharFromPos(es, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

    /* End of the EM_ messages which were in numerical order; what order
    * are these in?  vaguely alphabetical?
    */

  case GXWM_NCCREATE:
    result = EDIT_WM_NCCreate(hwnd, (GXLPCREATESTRUCTW)lParam, unicode);
    break;

  case GXWM_NCDESTROY:
    result = EDIT_WM_NCDestroy(es);
    es = NULL;
    break;

  case GXWM_GETDLGCODE:
    result = DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;

    if (es->style & GXES_MULTILINE)
      result |= DLGC_WANTALLKEYS;

    if (lParam)
    {
      es->flags|=EF_DIALOGMODE;

      if (((LPMSG)lParam)->message == WM_KEYDOWN)
      {
        int vk = (int)((LPMSG)lParam)->wParam;

        if (es->hwndListBox)
        {
          if (vk == VK_RETURN || vk == VK_ESCAPE)
            if (gxSendMessageW(gxGetParent(hwnd), CB_GETDROPPEDSTATE, 0, 0))
              result |= DLGC_WANTMESSAGE;
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
      result = EDIT_WM_Char(es, charW);
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
      result = EDIT_WM_Char(es, charW);
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
          EDIT_WM_Char(es, (GXWCHAR)(wParam >> 10) + 0xd800);
          EDIT_WM_Char(es, (GXWCHAR)(wParam & 0x03ff) + 0xdc00);
        }
        else EDIT_WM_Char(es, (GXWCHAR)wParam);
      }
      return 0;
    }
    break;

  case GXWM_CLEAR:
    EDIT_WM_Clear(es);
    break;

  case GXWM_COMMAND:
    EDIT_WM_Command(es, GXHIWORD(wParam), GXLOWORD(wParam), (GXHWND)lParam);
    break;

  case GXWM_CONTEXTMENU:
    EDIT_WM_ContextMenu(es, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_COPY:
    EDIT_WM_Copy(es);
    break;

  case GXWM_CREATE:
    if(unicode)
      result = EDIT_WM_Create(es, ((GXLPCREATESTRUCTW)lParam)->lpszName);
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
      result = EDIT_WM_Create(es, nameW);
      gxHeapFree(gxGetProcessHeap(), 0, nameW);
    }
    break;

  case GXWM_CUT:
    EDIT_WM_Cut(es);
    break;

  case GXWM_ENABLE:
    es->bEnableState = (GXBOOL) wParam;
    EDIT_UpdateText(es, NULL, TRUE);
    break;

  case GXWM_ERASEBKGND:
    /* we do the proper erase in EDIT_WM_Paint */
    result = 1;
    break;

  case GXWM_GETFONT:
    result = (GXLRESULT)es->font;
    break;

  case GXWM_GETTEXT:
    result = (GXLRESULT)EDIT_WM_GetText(es, (GXINT)wParam, (GXLPWSTR)lParam, unicode);
    break;

  case GXWM_GETTEXTLENGTH:
    if (unicode) result = get_text_length(es);
    else result = gxWideCharToMultiByte( CP_ACP, 0, es->text, get_text_length(es),
      NULL, 0, NULL, NULL );
    break;

  case GXWM_HSCROLL:
    result = EDIT_WM_HScroll(es, GXLOWORD(wParam), (short)GXHIWORD(wParam));
    break;

  case GXWM_KEYDOWN:
    result = EDIT_WM_KeyDown(es, (GXINT)wParam);
    break;

  case GXWM_KILLFOCUS:
    result = EDIT_WM_KillFocus(es);
    break;

  case GXWM_LBUTTONDBLCLK:
    result = EDIT_WM_LButtonDblClk(es);
    break;

  case GXWM_LBUTTONDOWN:
    result = EDIT_WM_LButtonDown(es, (GXWORD)wParam, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_LBUTTONUP:
    result = EDIT_WM_LButtonUp(es);
    break;

  case GXWM_MBUTTONDOWN:
    result = EDIT_WM_MButtonDown(es);
    break;

  case GXWM_MOUSEMOVE:
    result = EDIT_WM_MouseMove(es, (short)GXLOWORD(lParam), (short)GXHIWORD(lParam));
    break;

  case GXWM_PRINTCLIENT:
  case GXWM_PAINT:
    EDIT_WM_Paint(es, (GXHDC)wParam);
    break;

  case GXWM_PASTE:
    EDIT_WM_Paste(es);
    break;

  case GXWM_SETFOCUS:
    EDIT_WM_SetFocus(es);
    break;

  case GXWM_SETFONT:
    EDIT_WM_SetFont(es, (GXHFONT)wParam, GXLOWORD(lParam) != 0);
    break;

  case GXWM_SETREDRAW:
    /* FIXME: actually set an internal flag and behave accordingly */
    break;

  case GXWM_SETTEXT:
    EDIT_WM_SetText(es, (GXLPCWSTR)lParam, unicode);
    result = TRUE;
    break;

  case GXWM_SIZE:
    EDIT_WM_Size(es, (GXUINT)wParam, GXLOWORD(lParam), GXHIWORD(lParam));
    break;

  case GXWM_STYLECHANGED:
    result = EDIT_WM_StyleChanged(es, wParam, (const STYLESTRUCT *)lParam);
    break;

  case GXWM_STYLECHANGING:
    result = 0; /* See EDIT_WM_StyleChanged */
    break;

  case GXWM_SYSKEYDOWN:
    result = EDIT_WM_SysKeyDown(es, (GXINT)wParam, (GXDWORD)lParam);
    break;

  case GXWM_TIMER:
    EDIT_WM_Timer(es);
    break;

  case GXWM_VSCROLL:
    result = EDIT_WM_VScroll(es, GXLOWORD(wParam), (short)GXHIWORD(wParam));
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
        result = EDIT_EM_LineScroll(es, 0, cLineScroll);
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
    EDIT_ImeComposition(hwnd, lParam, es);
    break;

  case GXWM_IME_ENDCOMPOSITION:
    if (es->composition_len > 0)
    {
      EDIT_EM_ReplaceSel(es, TRUE, empty_stringW, TRUE, TRUE);
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

  default:
    result = DefWindowProcT(hwnd, msg, wParam, lParam, unicode);
    break;
  }

  if (gxIsWindow(hwnd) && es) EDIT_UnlockBuffer(es, FALSE);

  TRACE("hwnd=%p msg=%x (%s) -- 0x%08lx\n", hwnd, msg, SPY_GetMsgName(msg, hwnd), result);

  return result;
}


/*********************************************************************
 * edit class descriptor
 */
//static const GXWCHAR editW[] = {'E','d','i','t',0};
//const struct builtin_class_descr EDIT_builtin_class =
//{
//    editW,                /* name */
//    CS_DBLCLKS | CS_PARENTDC,   /* style */
//    WINPROC_EDIT,         /* proc */
//#ifdef __i386__
//    sizeof(EDITSTATE *) + sizeof(GXWORD), /* extra */
//#else
//    sizeof(EDITSTATE *),  /* extra */
//#endif
//    IDC_IBEAM,            /* cursor */
//    0                     /* brush */
//};

GXLRESULT GXCALLBACK EditWndProcW_1_3_30(GXHWND hWnd, GXUINT uMsg, GXWPARAM wParam, GXLPARAM lParam)
{
  return EditWndProc_common(hWnd, (GXUINT)uMsg, wParam, lParam, TRUE);
}

GXWNDCLASSEX WndClassEx_MyEdit_1_3_30 = { sizeof(GXWNDCLASSEX), GXCS_CLASSDC, EditWndProcW_1_3_30, 0L, sizeof(EDITSTATE *),
  (GXHINSTANCE)gxGetModuleHandle(NULL), NULL, gxLoadCursor(NULL, (GXLPCWSTR)GXIDC_IBEAM), NULL, NULL,
  GXWE_EDITW_1_3_30, NULL };
