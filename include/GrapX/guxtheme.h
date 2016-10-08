#ifndef _GX_THEME_EMU_H_
#define _GX_THEME_EMU_H_

typedef GXHANDLE GXHTHEME;

#include "user/Win32Emu/GUXTheme.inl"

class GXGraphics;
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
  GXHTHEME GXDLLAPI gxOpenThemeData(
    GXHWND hwnd,
    GXLPCWSTR pszClassList
    );

  GXHTHEME GXDLLAPI gxGetWindowTheme(       
    GXHWND hWnd
    );

  GXHRESULT GXDLLAPI gxCloseThemeData(  
    GXHTHEME hTheme
    );

  GXHRESULT GXDLLAPI gxDrawThemeParentBackground(
    GXHWND hwnd,
    GXHDC hdc,
    const GXRECT *prc
    );

  GXHRESULT GXDLLAPI gxDrawThemeBackground(
    GXHTHEME hTheme,
    GXHDC hdc,
    int iPartId,
    int iStateId,
    const GXRECT *pRect,  // 两个端点的坐标
    const GXRECT *pClipRect
    );
  
  GXHRESULT GXDLLAPI GXDrawThemeBackground(
    GXHTHEME hTheme,
    GXCanvas* canvas,
    int iPartId,
    int iStateId,
    const GXRECT *pRect,  // 两个端点的坐标
    const GXRECT *pClipRect
    );

  GXBOOL GXDLLAPI gxIsThemeBackgroundPartiallyTransparent(  
    GXHTHEME hTheme,
    int iPartId,
    int iStateId
    );
  GXHRESULT GXDLLAPI gxGetThemeBackgroundContentRect(GXHTHEME hTheme, GXHDC hdc, 
    int iPartId, int iStateId,  const GXRECT *pBoundingRect, 
    GXRECT *pContentRect);

  GXBOOL GXUXTHEME_Initialize(GXGraphics* pGraphics);
  GXVOID GXUXTHEME_Release();
#ifdef __cplusplus
}
#endif // __cplusplus

GXHRESULT GXDLLAPI GXDrawThemeBackground(
  GXHTHEME hTheme,
  GXWndCanvas& canvas,
  int iPartId,
  int iStateId,
  const GXRECT *pRect,  // 两个端点的坐标
  const GXRECT *pClipRect
  );



#endif // _GX_THEME_EMU_H_