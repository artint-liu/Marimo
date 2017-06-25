//#define _GXGRAPHICS_INLINE_EFFECT_D3D9_
#define _GXGRAPHICS_INLINE_SHADER_D3D9_

// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
#include <GrapX/GResource.H>
#include <GrapX/GPrimitive.H>
#include <GrapX/GRegion.H>
#include <GrapX/GTexture.H>
#include <GrapX/GXCanvas.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXFont.H>
#include <GrapX/GXImage.H>
#include <GrapX/GShader.H>

// 平台相关
#include "Canvas/GXResourceMgr.h"
#include "GrapX/GXCanvas3D.h"

// 私有头文件
#include "GrapX/GXUser.H"
#include "User/WindowsSurface.h"
#include "User/DesktopWindowsMgr.h"
#include "User/GXWindow.h"

#ifndef _DEV_DISABLE_UI_CODE
GXLRESULT GXWndCanvas::Initialize(GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags)
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  GXLPSTATION lpStation    = NULL;
  GXWindowsSurface* lpSurface = NULL;
  GXRECT rect;
  GXREGN regn;

  if(lpWnd != NULL)
  {
    lpStation = GXLPWND_STATION_PTR(lpWnd);
    //lpStation->Enter();
    lpSurface = lpWnd->GetTopSurface();
    lpWnd->GetBoundingRect(dwFlags & GXDCX_WINDOW, &rect);

    if((dwFlags & (GXDCX_PARENTCLIP | GXDCX_CLIPCHILDREN | GXDCX_CLIPSIBLINGS)) != 0)
      lpWnd->GetSystemRegion(dwFlags, &m_pSystemRegion);
  }
  else
  {
    lpStation = IntGetStationPtr();
    //lpStation->Enter();
    lpSurface = lpStation->m_pDesktopWindowsMgr->GetSurface(DWM_GS_DESKTOP);
    gxSetRect(&rect, 0, 0, lpStation->nWidth, lpStation->nHeight);
  }

  m_lpStation = lpStation;
  //m_lpStation->Enter();
  m_lpStation->pGraphics->Begin();
  //m_lpStation->BeginGraphics();

  // TODO: 子窗口与父窗口相交时，这个rect其实是子窗口区域，
  // 在UpdateRegion()中才会调整为正确的裁剪区
  // 这里是不是要考虑下改为一次计算好呢？
  gxRectToRegn(&regn, &rect);
  m_pNative = lpStation->pGraphics->LockCanvas(lpSurface->m_pRenderTar, &regn, NULL);

  if(pRegion != NULL)
    m_pUpdateRegion = pRegion->Clone();

  UpdateRegion();
  m_pNative->SetRegion(m_pEffectiveRegion, TRUE);
  return 0;
}

GXWndCanvas::GXWndCanvas(GXHWND hWnd)
  : m_pNative          (NULL)
  , m_hWnd             (hWnd)
  , m_lpStation        (NULL)
  , m_pSystemRegion    (NULL)
  , m_pUpdateRegion    (NULL)
  , m_pClipRegion      (NULL)
  , m_pEffectiveRegion (NULL)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  Initialize(hWnd, lpWnd == NULL ? NULL : lpWnd->m_prgnUpdate, GXDCX_PARENTCLIP | GXDCX_CLIPSIBLINGS);
}

GXWndCanvas::GXWndCanvas(GXHWND hWnd, GRegion* pRegion)
  : m_pNative(NULL)
  , m_hWnd(hWnd)
  , m_lpStation(NULL)
  , m_pSystemRegion(NULL)
  , m_pUpdateRegion(NULL)
  , m_pClipRegion(NULL)
  , m_pEffectiveRegion(NULL)
{
  Initialize(hWnd, pRegion, GXDCX_PARENTCLIP | GXDCX_CLIPSIBLINGS);
}

GXWndCanvas::GXWndCanvas(GXHWND hWnd, GRegion* pRegion, GXDWORD dwFlags)
  : m_pNative(NULL)
  , m_hWnd(hWnd)
  , m_lpStation(NULL)
  , m_pSystemRegion(NULL)
  , m_pUpdateRegion(NULL)
  , m_pClipRegion(NULL)
  , m_pEffectiveRegion(NULL)
{
  Initialize(hWnd, pRegion, dwFlags);
}

GXWndCanvas::~GXWndCanvas()
{
  m_hWnd = NULL;
  //m_lpStation->EndGraphics();
  SAFE_RELEASE(m_pNative);
  m_lpStation->pGraphics->End();
  SAFE_RELEASE(m_pSystemRegion);
  SAFE_RELEASE(m_pUpdateRegion);
  SAFE_RELEASE(m_pClipRegion);
  SAFE_RELEASE(m_pEffectiveRegion);
  //m_lpStation->Leave();
  m_lpStation = NULL;
}

GXHRESULT GXWndCanvas::DrawImage(GXImage*pImage, const GXREGN *rcDest)
{
  return m_pNative->DrawImage(pImage, rcDest);
}

GXHRESULT GXWndCanvas::DrawImage(GXImage*pImage, const GXREGN *rcDest, const GXREGN *rcSrc)
{
  return m_pNative->DrawImage(pImage, rcDest, rcSrc);
}

GXHRESULT GXWndCanvas::DrawImage(GXImage*pImage, GXINT xPos, GXINT yPos, const GXREGN *rcSrc)
{
  return m_pNative->DrawImage(pImage, xPos, yPos, rcSrc);
}

GXINT GXWndCanvas::DrawTextW(GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText)
{
  return m_pNative->DrawTextW(pFTFont, lpString, nCount, lpRect, uFormat, crText);
}

GXINT GXWndCanvas::DrawGlowTextW(GXFont* pFTFont, GXLPCWSTR lpString,GXINT nCount,GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF Color, GXUINT uRadius)
{
  return m_pNative->DrawTextW(pFTFont, lpString, nCount, lpRect, uFormat, Color);
}

GXBOOL GXWndCanvas::TextOutW(GXFont* pFTFont, GXINT nXStart,GXINT nYStart,GXLPCWSTR lpString,GXINT cbString, GXCOLORREF crText)
{
  return m_pNative->TextOutW(pFTFont, nXStart, nYStart, lpString, cbString, crText);
}

GXVOID GXWndCanvas::DrawRect(GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color)
{
  m_pNative->DrawRectangle(xPos, yPos, nWidth, nHeight, Color);
}

GXVOID GXWndCanvas::FillRect(GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight, GXCOLORREF Color)
{
  m_pNative->FillRectangle(xPos, yPos, nWidth, nHeight, Color);
}

GXVOID GXWndCanvas::FillRect(GXLPRECT lprect, GXCOLORREF Color)
{
  m_pNative->FillRectangle(lprect->left, lprect->top, lprect->right - lprect->left, lprect->bottom - lprect->top, Color);
}

GXVOID GXWndCanvas::InvertRect(GXINT xPos, GXINT yPos, GXINT nWidth, GXINT nHeight)
{
  m_pNative->InvertRect(xPos, yPos, nWidth, nHeight);
}

GXVOID GXWndCanvas::SetPixel(GXINT xPos, GXINT yPos, GXCOLORREF Color)
{
  m_pNative->SetPixel(xPos, yPos, Color);
}

GXVOID GXWndCanvas::DrawLine(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF Color)
{
  m_pNative->DrawLine(left, top, right, bottom, Color);
}

GXBOOL GXWndCanvas::SetViewportOrg(GXINT x, GXINT y, GXLPPOINT lpPoint)
{
  return m_pNative->SetViewportOrg(x, y, lpPoint);
}

GXINT GXWndCanvas::GetClipBox(GXLPRECT lpRect)
{
  return m_pNative->GetClipBox(lpRect);
}

GXBOOL GXWndCanvas::GetPaintRect(GXLPRECT lpRect)
{
  LPGXWND lpWnd = GXWND_PTR(m_hWnd);
  m_pEffectiveRegion->GetBounding(lpRect);
  gxOffsetRect(lpRect, -lpWnd->rectWindow.left, -lpWnd->rectWindow.top);
  return TRUE;
}

GXCanvas* GXWndCanvas::GetCanvasUnsafe()
{
  return m_pNative;
}

void GXWndCanvas::EnableAlphaBlend(GXBOOL bEnable)
{

}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
GXVOID GXWndCanvas::GXGetRenderSurfaceExt(GXHWND IN hTopLevel, GXINT* OUT pWidth, GXINT* OUT pHeight)
{
  LPGXWND    lpWnd    = GXWND_PTR(hTopLevel);
  GXLPSTATION lpStation  = GXLPWND_STATION_PTR(lpWnd);

  if(lpWnd->m_uStyle & GXWS_THICKFRAME)
  {
    *pWidth  = lpStation->nWidth;
    *pHeight = lpStation->nHeight;
  }
  else
  {
    *pWidth  = (lpWnd->rectWindow.right - lpWnd->rectWindow.left);// + FRAME_NC_GLOW_LEFT + FRAME_NC_GLOW_RIGHT;
    *pHeight = (lpWnd->rectWindow.bottom - lpWnd->rectWindow.top);// + FRAME_NC_GLOW_TOP + FRAME_NC_GLOW_BOTTOM;
    clClamp((GXINT)0, (GXINT)(int _w64)lpStation->nWidth,  pWidth );
    clClamp((GXINT)0, (GXINT)(int _w64)lpStation->nHeight, pHeight);
  }
  //#ifdef _DEBUG
  //  D3DSURFACE_DESC d3ddesc;
  //  if( lpWnd->m_pCanvas != NULL && 
  //    lpWnd->m_pCanvas->m_pRenderTar->Get()->D3DSurface() != NULL)
  //  {
  //    lpWnd->m_pCanvas->m_pRenderTar->Get()->D3DSurface()->GetDesc(&d3ddesc);
  //    ASSERT(d3ddesc.Width == *pWidth && d3ddesc.Height == *pHeight);
  //  }
  //#endif // _DEBUG
}
//
//////////////////////////////////////////////////////////////////////////
// 获得窗口的渲染区域
// 在使用 RenderToTarget中，窗口的渲染区域不是他的坐标所在区域
// 计算方法是 TopLevel 按照 RenderTarget 的尺寸居中，子窗口在 
// TopLevel 上的偏移。
// 参数：hTopLevel 应该是 hChild 的最顶层窗口，函数中没有验证
// 在调用时要通过调用者保证
//
//TODO: 需要整合到Canvas类中
GXVOID GXWndCanvas::GXGetRenderingRect(GXHWND IN hTopLevel, GXHWND IN hChild, GXDWORD IN flags, GXLPRECT OUT lprcOut)
{
  //GXSIZE sizeRect;
  LPGXWND lpTopLevel = GXWND_PTR(hTopLevel);
  LPGXWND lpChild = GXWND_PTR(hChild);
  if( flags == GRR_CONVERTRENDERORG )
  {
    //gxOffsetRect(lprcOut, -lpTopLevel->rectWindow.left, -lpTopLevel->rectWindow.top);
  }
  else if( flags == GRR_CONVERTWNDORG )
  {
    gxOffsetRect(lprcOut, lpTopLevel->rectWindow.left, lpTopLevel->rectWindow.top);
  }
  else //( flags == GRR_GETRECT )
  {
    if(lpChild == NULL)
    {
      lprcOut->left = 0;
      lprcOut->top = 0;
      lprcOut->right = lpTopLevel->rectWindow.right - lpTopLevel->rectWindow.left;
      lprcOut->bottom = lpTopLevel->rectWindow.bottom - lpTopLevel->rectWindow.top;
      if(flags == GRR_CLIENT)
      {
        if(WINSTYLE_HASCAPTION(lpTopLevel->m_uStyle))
        {
          lprcOut->left += FRAME_NC_EDGE_LEFT;
          lprcOut->top += FRAME_NC_EDGE_CAPTION;
          lprcOut->right -= FRAME_NC_EDGE_RIGHT;
          lprcOut->bottom -= FRAME_NC_EDGE_BOTTOM;
        }
        if(lpTopLevel->m_uStyle & GXWS_VSCROLL)
          lprcOut->right -= g_SystemMetrics[GXSM_CXVSCROLL];
        if(lpTopLevel->m_uStyle & GXWS_HSCROLL)
          lprcOut->bottom -= g_SystemMetrics[SM_CYHSCROLL];
      }
    }
    else
    {
      //lprcOut->left   = lpChild->rectWindow.left   - lpTopLevel->rectWindow.left;
      //lprcOut->top    = lpChild->rectWindow.top    - lpTopLevel->rectWindow.top;
      //lprcOut->right  = lpChild->rectWindow.right  - lpTopLevel->rectWindow.left;
      //lprcOut->bottom = lpChild->rectWindow.bottom - lpTopLevel->rectWindow.top;

      lprcOut->left  = lpChild->rectWindow.left;
      lprcOut->top  = lpChild->rectWindow.top;
      lprcOut->right  = lpChild->rectWindow.right;
      lprcOut->bottom  = lpChild->rectWindow.bottom;

      if(flags == GRR_CLIENT)
      {
        if(WINSTYLE_HASCAPTION(lpChild->m_uStyle))
        {
          lprcOut->left += FRAME_NC_EDGE_LEFT;
          lprcOut->top += FRAME_NC_EDGE_CAPTION;
          lprcOut->right -= FRAME_NC_EDGE_RIGHT;
          lprcOut->bottom -= FRAME_NC_EDGE_BOTTOM;
        }
        if(lpChild->m_uStyle & GXWS_VSCROLL)
          lprcOut->right -= g_SystemMetrics[SM_CXVSCROLL];
        if(lpChild->m_uStyle & GXWS_HSCROLL)
          lprcOut->bottom -= g_SystemMetrics[SM_CYHSCROLL];
      }
    }
  }
  return;

}

GXINT GXWndCanvas::UpdateRegion()
{
  //m_pSystemRegion;    // Windows Manager 确定的 Region
  //m_pUpdateRegion;    // 需要更新的 Region
  //m_pClipRegion;    // 用户设定的 Region

  GRegion* pNewRegion = NULL;
  SAFE_RELEASE(m_pEffectiveRegion);

  // 根据三个 Region 计算出最后显示的 Region
  if(m_pSystemRegion != NULL)
  {
    if(m_pUpdateRegion != NULL)
    {
      pNewRegion = m_pSystemRegion->CreateIntersect(m_pUpdateRegion);
      if(m_pClipRegion != NULL)
        pNewRegion->Intersect(m_pClipRegion);
    }
    else
    {
      if(m_pClipRegion != NULL)
        pNewRegion = m_pSystemRegion->CreateIntersect(m_pClipRegion);
      else
        pNewRegion = m_pSystemRegion->Clone();
    }  
  }
  else
  {
    if(m_pUpdateRegion != NULL)
    {
      if(m_pClipRegion != NULL)
        pNewRegion = m_pUpdateRegion->CreateIntersect(m_pClipRegion);
      else
        pNewRegion = m_pUpdateRegion->Clone();
    }
    else
    {
      if(m_pClipRegion != NULL)
        pNewRegion = m_pClipRegion->Clone();
      else
      {
        ASSERT(pNewRegion == NULL);
        m_pEffectiveRegion = pNewRegion;
        return RC_ERROR;
      }
    }
  }
  m_pEffectiveRegion = pNewRegion;
  return m_pEffectiveRegion->GetComplexity();
}
//void GXWndCanvas::EndDevice()
//{
//}
#endif // _DEV_DISABLE_UI_CODE