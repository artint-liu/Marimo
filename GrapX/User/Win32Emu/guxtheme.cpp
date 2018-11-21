#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GXSprite.h>
#include "GrapX/GXCanvas.h"

// 私有头文件
#include <User/GXWindow.h>
#include <User/Win32Emu/guxTheme.hxx>

GXSprite *s_pUxThemeSprite_Window;
GXSprite *s_pUxThemeSprite_ScrollBar;
GXSprite *s_pUxThemeSprite_Button;


static GXTHEME g_UxTheme_Window = {{DRB_WP_Table}};
static GXTHEME g_UxTheme_ScrollBar = {{DRB_SBP_Table}};
static GXTHEME g_UxTheme_Button = {{DRB_BP_Table}};

GXHTHEME hTheme_Window    = GXTHEME_HANDLE(&g_UxTheme_Window);
GXHTHEME hTheme_ScrollBar = GXTHEME_HANDLE(&g_UxTheme_ScrollBar);
GXHTHEME hTheme_Button = GXTHEME_HANDLE(&g_UxTheme_Button);

//GRENDERSTATEBLOCK g_UxThemeRenderState_Window[] = {
//  {D3DRS_ALPHABLENDENABLE, FALSE},
//  {(D3DRENDERSTATETYPE)0,0},
//};
//extern GRENDERSTATEBLOCK s_OpaquePreBlend[];
//GSAMPSTATEBLOCK g_UxThemeSampState_Window[] = {
//  {D3DSAMP_MINFILTER, D3DTEXF_LINEAR},
//  {D3DSAMP_MAGFILTER, D3DTEXF_LINEAR},
//  {D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP},
//  {D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP},
//  {(D3DSAMPLERSTATETYPE)0,0},
//};

extern "C"
{
  GXHTHEME GXDLLAPI gxOpenThemeData(
                   GXHWND hwnd,
                   GXLPCWSTR pszClassList
                   )
  {
    if(GXSTRCMPI(pszClassList, _CLTEXT("WINDOW")) == 0)
    {
      GXWND_PTR(hwnd)->m_hTheme = hTheme_Window;
    }
    else return NULL;
    return GXWND_PTR(hwnd)->m_hTheme;
  }

  GXHTHEME GXDLLAPI gxGetWindowTheme(       
                  GXHWND hWnd
                  )
  {
    return GXWND_PTR(hWnd)->m_hTheme;
  }

  GXHRESULT GXDLLAPI gxCloseThemeData(  
                   GXHTHEME hTheme
                   )
  {
    return GX_OK;
  }


  GXBOOL GXDLLAPI gxIsThemeBackgroundPartiallyTransparent(  
    GXHTHEME GXHTHEME,
    int iPartId,
    int iStateId
    )
  {
    //TODO: ASSERT(false);
    return NULL;
  }

  GXHRESULT GXDLLAPI gxDrawThemeParentBackground(
    GXHWND hwnd,
    GXHDC hdc,
    const GXRECT *prc
    )
  {
    ASSERT(false);
    return NULL;
  }
  
  GXHRESULT GXDLLAPI GXDrawThemeBackground(
    GXHTHEME hTheme,
    GrapX::GXCanvas* canvas,
    int iPartId,
    int iStateId,
    const GXRECT *pRect,  // 两个端点的坐标
    const GXRECT *pClipRect
    )
  {
    if(iPartId == 0)
      return NULL;
    GXLPTHEME lpTheme = GXTHEME_PTR(hTheme);
    ASSERT((GXINT_PTR)lpTheme->pDrawThemeBkgFunc[0] > iPartId);
    DrawThemeBkg pDrawThemeBkg;
    pDrawThemeBkg = lpTheme->pDrawThemeBkgFunc[iPartId];
    if(pDrawThemeBkg) {
      return pDrawThemeBkg(canvas, iStateId, (GXLPCRECT)pRect, (GXLPCRECT)pClipRect);
    }
    //else ASSERT(false);
    return NULL;
  }

  GXHRESULT GXDLLAPI gxDrawThemeBackground(
                      GXHTHEME hTheme,
                      GXHDC hdc,
                      int iPartId,
                      int iStateId,
                      const GXRECT *pRect,
                      const GXRECT *pClipRect
                      )
  {
    //if(iPartId == 0)
    //  return NULL;
    //GXLPTHEME lpTheme = GXTHEME_PTR(hTheme);
    //ASSERT((GXINT_PTR)lpTheme->pDrawThemeBkgFunc[0] > iPartId);
    //DrawThemeBkg pDrawThemeBkg;
    //pDrawThemeBkg = lpTheme->pDrawThemeBkgFunc[iPartId];
    //if(pDrawThemeBkg) {
    //  return pDrawThemeBkg(GXGDI_DC_PTR(hdc)->pCanvas, iStateId, (GXLPCRECT)pRect, (GXLPCRECT)pClipRect);
  //  }
    ////else ASSERT(false);
    //return NULL;
    return GXDrawThemeBackground(hTheme, GXGDI_DC_PTR(hdc)->pCanvas, iPartId, iStateId, pRect, pClipRect);
  }

  GXHRESULT GXDLLAPI gxGetThemeBackgroundContentRect(GXHTHEME hTheme, GXHDC hdc, 
                        int iPartId, int iStateId,  const GXRECT *pBoundingRect, 
                        GXRECT *pContentRect)
  {
  //  ASSERT(false);
    return NULL;
  }
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  GXBOOL GXUXTHEME_Initialize(GrapX::Graphics* pGraphics)
  {
    // TODO: 抽取资源
    GXCreateSpriteFromFileW(pGraphics, _CLTEXT("elements/common.stock"),       &GXWnd::s_pCommonSpr);
    GXCreateSpriteFromFileW(pGraphics, _CLTEXT("elements/window_frame.stock"), &s_pUxThemeSprite_Window);
    GXCreateSpriteFromFileW(pGraphics, _CLTEXT("elements/scroll_bar.stock"),   &s_pUxThemeSprite_ScrollBar);
    GXCreateSpriteFromFileW(pGraphics, _CLTEXT("elements/button.stock"),       &s_pUxThemeSprite_Button);
    return TRUE;
  }

  GXVOID GXUXTHEME_Release()
  {
    SAFE_RELEASE(s_pUxThemeSprite_Button);
    SAFE_RELEASE(s_pUxThemeSprite_ScrollBar);
    SAFE_RELEASE(s_pUxThemeSprite_Window);
    SAFE_RELEASE(GXWnd::s_pCommonSpr);
  }
};

GXHRESULT GXDLLAPI GXDrawThemeBackground(
  GXHTHEME hTheme,
  GXWndCanvas& canvas,
  int iPartId,
  int iStateId,
  const GXRECT *pRect,  // 两个端点的坐标
  const GXRECT *pClipRect
  )
{
  return GXDrawThemeBackground(hTheme, canvas.GetCanvasUnsafe(), iPartId, iStateId, pRect, pClipRect);
}

#endif // _DEV_DISABLE_UI_CODE