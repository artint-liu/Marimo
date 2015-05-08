#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GRegion.H>

// 私有头文件
#include <clUtility.H>
#include <User/GXWindow.h>
#include <User/WindowsSurface.h>
#include "GrapX/gxuser.h"
extern "C"
{
  //gxGetScrollInfo
  GXBOOL GXDLLAPI gxGetScrollInfo(
    GXHWND hwnd,    // handle of window with scroll bar
    GXINT fnBar,      // scroll bar flag
    GXLPSCROLLINFO lpsi  // pointer to structure for scroll parameters
    )
  {
    //ASSERT(FALSE);
    GXLPWND lpWnd = GXWND_PTR(hwnd);
    ASSERT(fnBar == GXSB_VERT || fnBar == GXSB_HORZ);
    if(fnBar == GXSB_VERT || fnBar == GXSB_HORZ)
    {
      LPGXSCROLLBAR pFocusScroll;
      if(fnBar == GXSB_VERT)
        pFocusScroll = lpWnd->m_lpVScrollBar;
      else
        pFocusScroll = lpWnd->m_lpHScrollBar;
      if(pFocusScroll == NULL)
        return FALSE;
      if(lpsi->fMask & GXSIF_PAGE)
        lpsi->nPage = pFocusScroll->m_nPage;
      if(lpsi->fMask & GXSIF_POS)
        lpsi->nPos = pFocusScroll->m_nPos;
      if(lpsi->fMask & GXSIF_TRACKPOS)
        lpsi->nTrackPos = pFocusScroll->m_nTrackPos;
      if(lpsi->fMask & GXSIF_RANGE)
      {
        lpsi->nMin = pFocusScroll->m_nMin;
        lpsi->nMax = pFocusScroll->m_nMax;
      }
    }
    return TRUE;
  }
  //gxSetScrollInfo
  GXINT GXDLLAPI gxSetScrollInfo(
    GXHWND hwnd,    // handle of window with scroll bar
    GXINT fnBar,      // scroll bar flag
    GXLPSCROLLINFO lpsi,  // pointer to structure with scroll parameters
    GXBOOL fRedraw    // redraw flag
    )
  {
    LPGXSCROLLBAR lpScrollBar;
    GXLPWND lpWnd = GXWND_PTR(hwnd);
    ASSERT(fnBar == GXSB_VERT || fnBar == GXSB_HORZ);

    if(fnBar == GXSB_VERT)
    {
      if(lpWnd->m_lpVScrollBar == NULL)
      {
        lpWnd->m_uStyle |= GXWS_VSCROLL;
        lpWnd->m_lpVScrollBar = new GXSCROLLBAR;
        memset(lpWnd->m_lpVScrollBar, 0, sizeof(GXSCROLLBAR));
        lpWnd->m_lpVScrollBar->m_uFlag = NULL;
      }
      lpScrollBar = lpWnd->m_lpVScrollBar;  
    }
    else if(fnBar == GXSB_HORZ)
    {
      if(lpWnd->m_lpHScrollBar == NULL)
      {
        lpWnd->m_lpHScrollBar = new GXSCROLLBAR;
        lpWnd->m_uStyle |= GXWS_HSCROLL;
        memset(lpWnd->m_lpHScrollBar, 0, sizeof(GXSCROLLBAR));
        lpWnd->m_lpHScrollBar->m_uFlag = NULL;
      }
      lpScrollBar = lpWnd->m_lpHScrollBar;  
    }

    if(lpsi->fMask & GXSIF_PAGE)
    {
      lpScrollBar->m_nPage = lpsi->nPage;
      // The nPage member must specify a value from 0 to nMax - nMin +1. 
      clClamp((GXUINT)0, (GXUINT)(lpScrollBar->m_nMax - lpScrollBar->m_nMin + 1), (GXUINT*)&lpScrollBar->m_nPage);
    }
    if(lpsi->fMask & GXSIF_POS)
    {
      lpScrollBar->m_nPos = lpsi->nPos;
      // The nPos member must specify a value between nMin and nMax - max(nPage  - 1, 0).
      clClamp((GXUINT)lpScrollBar->m_nMin, 
        (GXUINT)lpScrollBar->m_nMax - clMax((GXUINT)lpScrollBar->m_nPage - 1,(GXUINT)0),
        (GXUINT*)&lpScrollBar->m_nPos);
    }
    if(lpsi->fMask & GXSIF_RANGE)
    {
      lpScrollBar->m_nMin = lpsi->nMin;
      lpScrollBar->m_nMax = lpsi->nMax;
    }
    if(lpsi->fMask & GXSIF_DISABLENOSCROLL || lpsi->nMax <= lpsi->nMin)
    {
      lpScrollBar->m_uFlag |= GXSIF_DISABLENOSCROLL;
    }
    else
    {
      lpScrollBar->m_uFlag &= (~GXSIF_DISABLENOSCROLL);
    }

    // 处理重绘
    if(fRedraw == TRUE)
    {
      GXRECT rcRedraw;

      lpWnd->GetBoundingRect(FALSE, &rcRedraw);
      if(fnBar == GXSB_VERT)
      {
        rcRedraw.left = rcRedraw.right;
        rcRedraw.right += g_SystemMetrics[GXSM_CXVSCROLL];
      }
      else if(fnBar == GXSB_HORZ)
      {
        rcRedraw.top = rcRedraw.bottom;
        rcRedraw.bottom += g_SystemMetrics[GXSM_CYHSCROLL];
      }
      else
        goto DONOT_REDRAW;

      GRegion* prgnUpdate;
      GXLPWND_STATION_PTR(lpWnd)->pGraphics->CreateRectRgn(&prgnUpdate, 
        rcRedraw.left, rcRedraw.top, rcRedraw.right, rcRedraw.bottom);
      lpWnd->GetTopSurface()->InvalidateRegion(prgnUpdate);
      SAFE_RELEASE(prgnUpdate);

DONOT_REDRAW:;
    }

    return lpScrollBar->m_nPos;
  }

  int GXDLLAPI gxSetScrollPos(
    GXHWND hWnd,  // handle of window with scroll bar
    int nBar,  // scroll bar flag
    int nPos,  // new position of scroll box
    GXBOOL bRedraw   // redraw flag
    )
  {
    ASSERT(FALSE);
    return NULL;
  }
  int GXDLLAPI gxGetScrollPos(
    GXHWND hWnd,    // handle of window with scroll bar
    int nBar       // scroll bar flags
    )
  {
    if(nBar == GXSB_HORZ && GXWND_PTR(hWnd)->m_lpHScrollBar != NULL)
      return GXWND_PTR(hWnd)->m_lpHScrollBar->m_nPos;
    else if(nBar == GXSB_VERT && GXWND_PTR(hWnd)->m_lpVScrollBar != NULL)
      return GXWND_PTR(hWnd)->m_lpVScrollBar->m_nPos;
    return 0L;
  }

  GXBOOL GXDLLAPI gxShowScrollBar(
    GXHWND hWnd,    // handle of window with scroll bar 
    int wBar,    // scroll bar flag
    GXBOOL bShow   // scroll bar visibility flag
    )
  {
    TRACE_UNACHIEVE("=== gxShowScrollBar ===\n");
    //ASSERT(FALSE);
    return NULL;
  }
}
#endif // _DEV_DISABLE_UI_CODE