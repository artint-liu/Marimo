#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GRegion.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXRenderTarget.h"
#include "GrapX/GXKernel.h"

// 私有头文件
#include "GrapX/GXUser.h"
#include "GXStation.h"
#include <User/GXWindow.h>
#include <User/DesktopWindowsMgr.h>
#include <User/WindowsSurface.h>


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXWindowsSurface::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GXWindowsSurface::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE


GXWindowsSurface::GXWindowsSurface(GXLPSTATION lpStation, GXHWND hWnd)
  : m_pRenderTar      (NULL)
  //, m_pDepthStencil    (NULL)
  , m_prgnUpdate      (NULL)
  , m_lpStation      (lpStation)
  , m_prgnWindows      (NULL)
  , m_bGenerateWindowsRgn  (FALSE)
  , m_hExclusiveWnd    (hWnd)
{
  //GXUINT nWidth, nHeight;
  GrapX::Graphics* pGraphics = m_lpStation->pGraphics;
  pGraphics->CreateRectRgn(&m_prgnUpdate, 0, 0, 0, 0);
  pGraphics->CreateRectRgn(&m_prgnWindows, 0, 0, 0, 0);
  //pGraphics->CreateTexture(&m_pDepthStencil, TEXSIZE_SAME, TEXSIZE_SAME, 1, NULL, GXFMT_D24S8, GXPOOL_DEFAULT);
  pGraphics->CreateRenderTarget(&m_pRenderTar, NULL, GXSizeRatio::Same, GXSizeRatio::Same,
    GXGraphicsFormat::Format_B8G8R8A8, GXGraphicsFormat::Format_D24S8);

  // 复位设备后整个页面要重绘，这里要设置范围
  GXSIZE sDimension;
  m_pRenderTar->GetDimension(&sDimension);
  gxSetRect(&rcScrUpdate, 0, 0, sDimension.cx, sDimension.cy);

  m_bLayered = gxGetWindowLong(hWnd, GXGWL_EXSTYLE) & GXWS_EX_LAYERED;
#ifdef ENABLE_DYNMAIC_EFFECT
  pKGrid = GXNEW CKinematicGrid(pWnd);
#endif // ENABLE_DYNMAIC_EFFECT
}

GXWindowsSurface::~GXWindowsSurface()
{
  m_hExclusiveWnd = NULL;
  m_lpStation = NULL;
  SAFE_RELEASE(m_prgnUpdate);
  //SAFE_RELEASE(m_pDepthStencil);
  SAFE_RELEASE(m_pRenderTar);
  SAFE_RELEASE(m_prgnWindows);
#ifdef ENABLE_DYNMAIC_EFFECT
  SAFE_DELETE(pKGrid);
#endif // ENABLE_DYNMAIC_EFFECT
}

RGNCOMPLEX GXWindowsSurface::InvalidateRegion(const GRegion* pRegion)
{
  m_lpStation->SetLazyUpdate();
  // TODO: 应该检查传入的区域是否超出了可以绘图的面积
  return m_prgnUpdate->Union(pRegion);
}

RGNCOMPLEX GXWindowsSurface::InvalidateRect(GXRECT* lpRect)
{
  GRegion* pRegion;
  GrapX::Graphics* pGraphics = m_lpStation->pGraphics;

  GXRECT rcUpdate(0);

  if(lpRect == NULL)
  {
    GXSIZE sDimension;
    m_pRenderTar->GetDimension(&sDimension);
    rcUpdate.right = sDimension.cx;
    rcUpdate.bottom = sDimension.cy;
  }
  else
    rcUpdate = *lpRect;

  pGraphics->CreateRectRgn(&pRegion, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom);
  const RGNCOMPLEX r = m_prgnUpdate->Union(pRegion);
  SAFE_RELEASE(pRegion);
  m_lpStation->SetLazyUpdate();
  return r;
}

GXBOOL GXWindowsSurface::SaveToFileW(GXLPCWSTR lpFilename, GXLPCSTR lpFormat)
{
  return m_pRenderTar->SaveToFile(lpFilename, lpFormat);
}

GXBOOL GXWindowsSurface::Scroll(int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate)
{
  GRegion* prgnUpdate = NULL;
  //m_pRenderTar->Scroll(dx, dy, lprcScroll, lprgnClip, &prgnUpdate);
  CLBREAK; // 上面这条没改

  // 将更新区添加到Surface中
  if(prgnUpdate != NULL)
    m_prgnUpdate->Union(prgnUpdate);

  if(lpprgnUpdate != NULL)
    *lpprgnUpdate = prgnUpdate;
  else
    SAFE_RELEASE(prgnUpdate);

  m_lpStation->SetLazyUpdate();
  return TRUE;
}

GXBOOL GXWindowsSurface::BitBltRect(GXWindowsSurface* pSrcSurface, int xDest, int yDest, LPGXCRECT lprcSource)
{
  GXRECT rcDest;
  rcDest.left   = lprcSource->left + xDest;
  rcDest.top    = lprcSource->top + yDest;
  rcDest.right  = lprcSource->right + xDest;
  rcDest.bottom = lprcSource->bottom + yDest;

  m_pRenderTar->StretchRect(pSrcSurface->m_pRenderTar->GetColorTextureUnsafe(GXResUsage::Default), &rcDest, lprcSource, GXTEXFILTER_POINT);
  return TRUE;
}

GXBOOL GXWindowsSurface::BitBltRegion(GXWindowsSurface* pSrcSurface, int xDest, int yDest, GRegion* lprgnSource)
{
  //return m_pRenderTar->BitBltRegion(pSrcSurface->m_pRenderTar, xDest, yDest, lprgnSource);
  CLBREAK; // 上面这条没改
  return FALSE;
}

int GXWindowsSurface::GenerateWindowsRgn(GXBOOL bDelay)
{
  if(m_hExclusiveWnd == NULL && bDelay == TRUE)
  {
    m_bGenerateWindowsRgn = TRUE;
    return 0;
  }

  int nCount = NULL;
  GRegion* prgnWindow = NULL;

  m_prgnWindows->SetEmpty();
  if(m_hExclusiveWnd != NULL)
  {
    GXLPWND lpWnd = GXWND_PTR(m_hExclusiveWnd);
    lpWnd->GetSystemRegion(GSR_ALLLAYERS | GSR_CLIPSIBLINGS | GSR_WINDOW, &prgnWindow);
    m_prgnWindows->Union(prgnWindow);
    SAFE_RELEASE(prgnWindow);
  }
  else
  {
    GXLPWND lpWnd = m_lpStation->lpDesktopWnd->m_pFirstChild;
    if(lpWnd != NULL) {
      GET_LAST_WINDOW(lpWnd);
      //while(lpWnd->m_pNextWnd != NULL)
      //  lpWnd = lpWnd->m_pNextWnd;
    }

    while(lpWnd != NULL)
    {
      if((lpWnd->m_uStyle & GXWS_VISIBLE) && 
        gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
      {
        if(lpWnd->m_pWinsSurface == this)
        {
          lpWnd->GetWindowRegion(&prgnWindow);
          m_prgnWindows->Union(prgnWindow);
          SAFE_RELEASE(prgnWindow);
        }
        else
        {
          lpWnd->GetWindowRegion(&prgnWindow);
          prgnWindow->Subtract(m_prgnWindows);
          lpWnd->m_pWinsSurface->m_prgnWindows->SetEmpty();
          lpWnd->m_pWinsSurface->m_prgnWindows->Union(prgnWindow);
          SAFE_RELEASE(prgnWindow);
        }
      }
      lpWnd = lpWnd->m_pPrevWnd;
      nCount++;
    }
  }
  m_bGenerateWindowsRgn = FALSE;
  return nCount;
}

GXHWND GXWindowsSurface::GetExclusiveWnd()
{
  return m_hExclusiveWnd;
}

GXHWND GXWindowsSurface::SetExclusiveWnd(GXHWND hWnd, GXDWORD dwFlags)
{
  GXHWND hOldWnd = m_hExclusiveWnd;
  m_hExclusiveWnd = hWnd;

  if((dwFlags & SEW_DONOTBLT) != NULL || hOldWnd == hWnd)
    return hOldWnd;

  GXWindowsSurface* pDesktopSurface = NULL;
  GXWindowsSurface* pOldWndOldSur = NULL;
  GXWindowsSurface* pNewWndOldSur = NULL;
  GRegion* prgnOldWnd = NULL;
  GRegion* prgnNewWnd = NULL;
  GRegion* prgnOldWndOldSurInvld = NULL;
  GRegion* prgnNewWndOldSurInvld = NULL;
  GXLPWND lpOldWnd = NULL;
  GXLPWND lpNewWnd = NULL;

  if(hOldWnd != NULL)
  {
    lpOldWnd = GXWND_PTR(hOldWnd);
    pDesktopSurface = m_lpStation->m_pDesktopWindowsMgr->GetSurface(DWM_GS_DESKTOP);


    ASSERT(lpOldWnd->m_pWinsSurface == this);
    ASSERT((lpOldWnd->m_uExStyle & GXWS_EX_LAYERED) == 0);

    if(lpOldWnd->m_pWinsSurface != pDesktopSurface)
    {
      pOldWndOldSur = lpOldWnd->m_pWinsSurface;

      // 把桌面Surface分配给这个窗口
      lpOldWnd->m_pWinsSurface = pDesktopSurface;
      pDesktopSurface->AddRef();

    }
    pDesktopSurface->GenerateWindowsRgn(TRUE);
  }

  if(hWnd != NULL)
  {
    lpNewWnd = GXWND_PTR(hWnd);
    ASSERT((lpNewWnd->m_uExStyle & GXWS_EX_LAYERED) == 0);

    pNewWndOldSur = lpNewWnd->m_pWinsSurface;
    lpNewWnd->m_pWinsSurface = this;
    AddRef();
  }

  if(lpOldWnd != NULL) {
    lpOldWnd->GetSystemRegion(GSR_CLIPSIBLINGS | GSR_WINDOW, &prgnOldWnd);
  }

  if(lpNewWnd != NULL) {
    lpNewWnd->GetSystemRegion(GSR_CLIPSIBLINGS | GSR_WINDOW, &prgnNewWnd);
  }

  if(pNewWndOldSur != NULL)
  {
    if(prgnOldWnd != NULL)
    {
      // 测试是否相交,不相交则省掉一次复制

      GXRECT rcNew, rcOld, rcResult;
      prgnNewWnd->GetBounding(&rcNew);
      prgnOldWnd->GetBounding(&rcOld);

      // 计算需要复制的无效区域
      if(pOldWndOldSur->m_prgnUpdate->GetComplexity() > RC_NULL && prgnOldWnd->GetComplexity() > RC_NULL) {
        prgnOldWndOldSurInvld = pOldWndOldSur->m_prgnUpdate->CreateIntersect(prgnOldWnd);
      }

      if(pNewWndOldSur->m_prgnUpdate->GetComplexity() > RC_NULL && prgnNewWnd->GetComplexity() > RC_NULL) {
        prgnNewWndOldSurInvld = pNewWndOldSur->m_prgnUpdate->CreateIntersect(prgnNewWnd);
      }

      if(gxIntersectRect(&rcResult, &rcNew, &rcOld) == 0)
      {

        pDesktopSurface->BitBltRegion(pOldWndOldSur, 0, 0, prgnOldWnd);
        BitBltRegion(pNewWndOldSur, 0, 0, prgnNewWnd);
      }
      else
      {
        //GXImage* pBackBufferImage = m_lpStation->pGraphics->GetBackBufferImg();
        //pBackBufferImage->BitBltRegion(pNewWndOldSur->m_pRenderTar, 0, 0, prgnNewWnd);
        //pDesktopSurface->BitBltRegion(pOldWndOldSur, 0, 0, prgnOldWnd);
        //m_pRenderTar->BitBltRegion(pBackBufferImage, 0, 0, prgnNewWnd);
        CLBREAK; // 上面这条没改
      }

      // 移动
      if(prgnOldWndOldSurInvld != NULL)
      {
        ASSERT(pDesktopSurface == pNewWndOldSur);
        pDesktopSurface->InvalidateRegion(prgnOldWndOldSurInvld);
        pOldWndOldSur->m_prgnUpdate->Subtract(prgnOldWndOldSurInvld);
      }

      if(prgnNewWndOldSurInvld != NULL)
      {
        InvalidateRegion(prgnNewWndOldSurInvld);
        //pNewWndOldSur->m_prgnUpdate->Subtract(prgnNewWndOldSurInvld);
      }

      // pNewWndOldSur要设置NewWnd的无效区，并排除OldWnd的有效区
      GRegion* prgnDesktopSurUpdate = prgnNewWnd->CreateSubtract(prgnOldWnd);
      pDesktopSurface->InvalidateRegion(prgnDesktopSurUpdate);
      SAFE_RELEASE(prgnDesktopSurUpdate);
    }
    else // 这个表面没有旧的独占窗口 (prgnOldWnd == NULL)
    {
      if(pNewWndOldSur->m_prgnUpdate->GetComplexity() > RC_NULL && 
        prgnNewWnd->GetComplexity() > RC_NULL)
        prgnNewWndOldSurInvld = pNewWndOldSur->m_prgnUpdate->CreateIntersect(prgnNewWnd);

      BitBltRegion(pNewWndOldSur, 0, 0, prgnNewWnd);

      if(prgnNewWndOldSurInvld != NULL)
      {
        InvalidateRegion(prgnNewWndOldSurInvld);
        //pNewWndOldSur->m_prgnUpdate->Subtract(prgnNewWndOldSurInvld);
      }

      // pNewWndOldSur要设置NewWnd的无效区
      pNewWndOldSur->InvalidateRegion(prgnNewWnd);
    }
  }
  else
  {
    if(prgnNewWnd != NULL)
      InvalidateRegion(prgnNewWnd);
    if(prgnOldWnd != NULL)
    {
      if(pOldWndOldSur->m_prgnUpdate->GetComplexity() > RC_NULL && 
        prgnOldWnd->GetComplexity() > RC_NULL)
        prgnOldWndOldSurInvld = pOldWndOldSur->m_prgnUpdate->CreateIntersect(prgnOldWnd);

      pDesktopSurface->BitBltRegion(pOldWndOldSur, 0, 0, prgnOldWnd);

      if(prgnOldWndOldSurInvld != NULL)
      {
        pDesktopSurface->InvalidateRegion(prgnOldWndOldSurInvld);
        pOldWndOldSur->m_prgnUpdate->Subtract(prgnOldWndOldSurInvld);
      }
    }
  }

  if(hOldWnd == NULL)
    GenerateWindowsRgn(TRUE);

  SAFE_RELEASE(prgnOldWndOldSurInvld);
  SAFE_RELEASE(prgnNewWndOldSurInvld);
  SAFE_RELEASE(prgnOldWnd);
  SAFE_RELEASE(prgnNewWnd);
  SAFE_RELEASE(pOldWndOldSur);
  SAFE_RELEASE(pNewWndOldSur);
  return hOldWnd;
}

// 返回值是 m_pUpdate 的状态
RGNCOMPLEX GXWindowsSurface::ValidateBlank()
{
  // 求出Windows之外的更新区, 这些区域因为没有窗口, 可以直接剪掉
  RGNCOMPLEX rc = RC_ERROR;
  if(m_hExclusiveWnd != NULL)
  {
    GXRECT rcUpdate;
    GXRECT rcWindow;
    m_prgnUpdate->GetBounding(&rcUpdate);
    gxGetWindowRect(m_hExclusiveWnd, &rcWindow);

    // 如果更新矩形在窗口矩形之内
    if(rcUpdate.left >= rcWindow.left && rcUpdate.top >= rcWindow.top &&
      rcUpdate.right <= rcWindow.right && rcUpdate.bottom >= rcWindow.bottom)
    {
      return m_prgnUpdate->GetComplexity();
    }
    GRegion* prgnWindow;
    GXWND_PTR(m_hExclusiveWnd)->GetWindowRegion(&prgnWindow);
    rc = m_prgnUpdate->Intersect(prgnWindow);
    SAFE_RELEASE(prgnWindow);
    return rc;
  }
  else
  {
    rc = m_prgnUpdate->Intersect(m_prgnWindows);
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////

#endif // _DEV_DISABLE_UI_CODE