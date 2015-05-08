#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXSprite.H>
#include "GrapX/GXCanvas.H"

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
    if(GXSTRCMPI(pszClassList, L"WINDOW") == 0)
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
    GXCanvas* canvas,
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
  GXBOOL GXUXTHEME_Initialize(GXGraphics* pGraphics)
  {
    GXCreateSpriteFromFileW(pGraphics, L"Window.GSprite", &s_pUxThemeSprite_Window);
    //s_pUxThemeSprite_Window->SetSampStateRefPtr(g_UxThemeSampState_Window);
    //s_pUxThemeSprite_Window->SetRenderStateRefPtr(s_OpaquePreBlend);
    GXCreateSpriteFromFileW(pGraphics, L"ScrollBar.GSprite", &s_pUxThemeSprite_ScrollBar);
    GXCreateSpriteFromFileW(pGraphics, L"Button.GSprite", &s_pUxThemeSprite_Button);
    return TRUE;
  }

  GXVOID GXUXTHEME_Release()
  {
    SAFE_RELEASE(s_pUxThemeSprite_Button);
    SAFE_RELEASE(s_pUxThemeSprite_ScrollBar);
    SAFE_RELEASE(s_pUxThemeSprite_Window);
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