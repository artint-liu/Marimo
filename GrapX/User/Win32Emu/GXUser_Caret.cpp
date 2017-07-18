#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXCanvas.H>

// 私有头文件
#include <User/GXWindow.h>
#include "GrapX/GXUser.H"

extern GXLPSTATION g_pCurStation;
extern "C"
{
  GXBOOL GXDLLAPI gxCreateCaret(
    GXHWND hWnd,      // handle to owner window 
    GXHBITMAP hBitmap,    // handle to bitmap for caret shape
    GXINT nWidth,      // caret width 
    GXINT nHeight       // caret height 
    )
  {
#ifdef _UPDATE_WIN32_CARET
    PostMessage(g_pCurStation->hBindWin32Wnd, WM_GX_CREATECARET, 0, MAKELPARAM(nWidth, nHeight));
#endif // #ifdef _UPDATE_WIN32_CARET
    LPGXCARET lpCaret = &(GXHWND_STATION_PTR(hWnd)->SysCaret);
    lpCaret->hWnd      = hWnd;
    lpCaret->hTopLevel  = GXWND_PTR(hWnd)->GXGetTopLevel();
    lpCaret->hBitmap    = hBitmap;      ASSERT(hBitmap == NULL);
    lpCaret->regnCaret.width     = nWidth;
    lpCaret->regnCaret.height    = nHeight;
    lpCaret->flag      = GXCARET_AVAILABLE;
    lpCaret->nBlinkTime = 500;

    lpCaret->regnPrevShowing.left   = 0;
    lpCaret->regnPrevShowing.top    = 0;
    lpCaret->regnPrevShowing.width  = 0;
    lpCaret->regnPrevShowing.height = 0;

    return TRUE;
  }

  GXBOOL GXDLLAPI gxShowCaret(
    GXHWND hWnd     // handle of window with caret 
    )
  {
    GXLPSTATION lpStation = GXHWND_STATION_PTR(hWnd);
#ifdef _UPDATE_WIN32_CARET
    PostMessage(lpStation->hBindWin32Wnd, WM_GX_SHOWCARET, 1, 0);
#endif // #ifdef _UPDATE_WIN32_CARET
    LPGXCARET lpCaret = &(hWnd != NULL
      ? (lpStation->SysCaret)
      : GrapX::Internal::GetStationPtr()->SysCaret);

    if(lpCaret->flag & GXCARET_AVAILABLE)
    {
      lpCaret->flag |= GXCARET_VISIBLE;
      lpCaret->nBaseTime = gxGetTickCount();
      return TRUE;
    }
    return FALSE;
  }
  GXBOOL GXDLLAPI gxHideCaret(
    GXHWND hWnd     // handle to the window with the caret 
    )
  {
    if(hWnd == NULL)
      return FALSE;

    GXLPSTATION lpStation = GXHWND_STATION_PTR(hWnd);

#ifdef _UPDATE_WIN32_CARET
    PostMessage(lpStation->hBindWin32Wnd, WM_GX_SHOWCARET, 0, 0);
#endif // #ifdef _UPDATE_WIN32_CARET

    LPGXCARET lpCaret = &(lpStation->SysCaret);
    if(lpCaret->flag & GXCARET_AVAILABLE)
    {
      lpCaret->flag &= (~GXCARET_VISIBLE);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL GXDLLAPI gxSetCaretPos(
    GXINT X,    // horizontal position 
    GXINT Y     // vertical position 
    )
  {
#ifdef _UPDATE_WIN32_CARET
    //PostMessage(g_pCurStation->hBindWin32Wnd, WM_GX_SETCARETPOS, 0, MAKELPARAM(X, Y));
#endif // #ifdef _UPDATE_WIN32_CARET

    LPGXCARET lpCaret = &g_pCurStation->SysCaret;
    if(lpCaret->flag & GXCARET_AVAILABLE)
    {
      if((lpCaret->flag & GXCARET_BLINK) != 0)
      {
        lpCaret->regnPrevShowing.left   = lpCaret->regnCaret.left;
        lpCaret->regnPrevShowing.top    = lpCaret->regnCaret.top;
        lpCaret->regnPrevShowing.width  = lpCaret->regnCaret.width;
        lpCaret->regnPrevShowing.height = lpCaret->regnCaret.height;
      }
      else
      {
        lpCaret->regnPrevShowing.width  = 0;
        lpCaret->regnPrevShowing.height = 0;
      }
      lpCaret->regnCaret.left = X;
      lpCaret->regnCaret.top  = Y;
      lpCaret->nBaseTime = gxGetTickCount();
      lpCaret->flag &= (~GXCARET_BLINK);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL GXDLLAPI gxDestroyCaret()
  {
#ifdef _UPDATE_WIN32_CARET
    PostMessage(g_pCurStation->hBindWin32Wnd, WM_GX_DESTROYCARET, 0, 0);
#endif // #ifdef _UPDATE_WIN32_CARET

    LPGXCARET lpCaret = &g_pCurStation->SysCaret;
    if(lpCaret->flag & GXCARET_AVAILABLE)
    {
      lpCaret->hTopLevel = NULL;
      lpCaret->hWnd     = NULL;
      lpCaret->flag     = 0;
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL GXDLLAPI gxSetCaretBlinkTime(
    GXUINT uMSeconds   // blink time, in milliseconds 
    )
  {
    LPGXCARET lpCaret = &g_pCurStation->SysCaret;
    if(lpCaret->flag & GXCARET_AVAILABLE)
    {
      lpCaret->nBlinkTime = uMSeconds;
      return TRUE;
    }
    return FALSE;
  }

  //GXVOID GXWnd::ErasePrevCaret(GXWndCanvas *pCanvas)
  //{
  //  LPGXCARET lpCaret = &g_pCurStation->SysCaret;
  //
  //  if( lpCaret->regnPrevShowing.width  != 0 &&
  //    lpCaret->regnPrevShowing.height != 0)
  //  {
  //    pCanvas->InvertRect(
  //      lpCaret->regnPrevShowing.left,
  //      lpCaret->regnPrevShowing.top,
  //      lpCaret->regnPrevShowing.width,
  //      lpCaret->regnPrevShowing.height);
  //    lpCaret->regnPrevShowing.width  = 0;
  //    lpCaret->regnPrevShowing.height = 0;
  //  }
  //}
  GXBOOL GXCARET::IsVisible()
  {
    // TODO: 以后是否能把光标闪烁加入到RichFX中呢?
    return (flag & GXCARET_SHOWING) == GXCARET_SHOWING;
  }

  GXBOOL GXCARET::Tick()
  {
    // TODO: 以后是否能把光标闪烁加入到RichFX中呢?
    const GXDWORD dwUpdateFlag = (GXCARET_AVAILABLE | GXCARET_VISIBLE);
    if(
      (flag & dwUpdateFlag) == dwUpdateFlag &&
      ((gxGetTickCount() - nBaseTime) % (nBlinkTime << 1) < nBlinkTime)
      )
    {
      flag |= GXCARET_BLINK;
      return TRUE;
    }

    flag &= (~GXCARET_BLINK);
    return GX_OK;
  }

  GXHRESULT GXCARET::PaintCaret(GXCanvas* pCanvas)
  {
    // TODO: 以后是否能把光标闪烁加入到RichFX中呢?
    if(flag & GXCARET_BLINK)
    {
      LPGXWND lpWnd = GXWND_PTR(hWnd);
      CHECK_LPWND_VAILD(lpWnd);

      GXRECT rcClient;
      lpWnd->GetBoundingRect(FALSE, &rcClient);
      pCanvas->FillRectangle(
        rcClient.left + regnCaret.left,
        rcClient.top + regnCaret.top,
        regnCaret.width,
        regnCaret.height,
        0xff00ff00);
    }
    return GX_OK;
  }
}

#endif // _DEV_DISABLE_UI_CODE