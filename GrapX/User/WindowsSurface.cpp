#ifndef _DEV_DISABLE_UI_CODE
// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GRegion.H"
#include "Include/GTexture.H"
#include "Include/GXGraphics.H"
#include "Include/GXImage.H"
#include "Include/GXKernel.H"

// ˽��ͷ�ļ�
#include "Include/GXUser.H"
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
  , m_pDepthStencil    (NULL)
  , m_prgnUpdate      (NULL)
  , m_lpStation      (lpStation)
  , m_prgnWindows      (NULL)
  , m_bGenerateWindowsRgn  (FALSE)
  , m_hExclusiveWnd    (hWnd)
{
  GXUINT nWidth, nHeight;
  GXGraphics* pGraphics = m_lpStation->pGraphics;
  pGraphics->CreateRectRgn(&m_prgnUpdate, 0, 0, 0, 0);
  pGraphics->CreateRectRgn(&m_prgnWindows, 0, 0, 0, 0);
  //pGraphics->CreateTexture(&m_pDepthStencil, TEXSIZE_SAME, TEXSIZE_SAME, 1, NULL, GXFMT_D24S8, GXPOOL_DEFAULT);
  m_pRenderTar = pGraphics->CreateImage(TEXSIZE_SAME, TEXSIZE_SAME, GXFMT_A8R8G8B8, TRUE, NULL);

  // ��λ�豸������ҳ��Ҫ�ػ棬����Ҫ���÷�Χ
  m_pRenderTar->GetTextureUnsafe()->GetDimension(&nWidth, &nHeight);
  gxSetRect(&rcScrUpdate, 0, 0, nWidth, nHeight);

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
  SAFE_RELEASE(m_pDepthStencil);
  SAFE_RELEASE(m_pRenderTar);
  SAFE_RELEASE(m_prgnWindows);
#ifdef ENABLE_DYNMAIC_EFFECT
  SAFE_DELETE(pKGrid);
#endif // ENABLE_DYNMAIC_EFFECT
}

RGNCOMPLEX GXWindowsSurface::InvalidateRegion(const GRegion* pRegion)
{
  // TODO: Ӧ�ü�鴫��������Ƿ񳬳��˿��Ի�ͼ�����
  return m_prgnUpdate->Union(pRegion);
}

RGNCOMPLEX GXWindowsSurface::InvalidateRect(GXRECT* lpRect)
{
  GRegion* pRegion;
  GXGraphics* pGraphics = m_lpStation->pGraphics;

  GXRECT rcUpdate = {0, 0};

  if(lpRect == NULL)
    m_pRenderTar->GetTextureUnsafe()->GetDimension((GXUINT*)&rcUpdate.right, (GXUINT*)&rcUpdate.bottom);
  else
    rcUpdate = *lpRect;

  pGraphics->CreateRectRgn(&pRegion, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom);
  const RGNCOMPLEX r = m_prgnUpdate->Union(pRegion);
  SAFE_RELEASE(pRegion);
  return r;
}

GXBOOL GXWindowsSurface::SaveToFileW(GXLPCWSTR lpFilename, GXLPCSTR lpFormat)
{
  return m_pRenderTar->SaveToFileW(lpFilename, lpFormat);
}

GXBOOL GXWindowsSurface::Scroll(int dx, int dy, LPGXCRECT lprcScroll, GRegion* lprgnClip, GRegion** lpprgnUpdate)
{
  GRegion* prgnUpdate = NULL;
  m_pRenderTar->Scroll(dx, dy, lprcScroll, lprgnClip, &prgnUpdate);

  // �����������ӵ�Surface��
  if(prgnUpdate != NULL)
    m_prgnUpdate->Union(prgnUpdate);

  if(lpprgnUpdate != NULL)
    *lpprgnUpdate = prgnUpdate;
  else
    SAFE_RELEASE(prgnUpdate);
  return TRUE;
}

GXBOOL GXWindowsSurface::BitBltRect(GXWindowsSurface* pSrcSurface, int xDest, int yDest, LPGXCRECT lprcSource)
{
  GXRECT rcDest;
  rcDest.left   = lprcSource->left + xDest;
  rcDest.top    = lprcSource->top + yDest;
  rcDest.right  = lprcSource->right + xDest;
  rcDest.bottom = lprcSource->bottom + yDest;

  m_pRenderTar->GetTextureUnsafe()->StretchRect(pSrcSurface->m_pRenderTar->GetTextureUnsafe(), &rcDest, lprcSource, GXTEXFILTER_POINT);
  return TRUE;
}

GXBOOL GXWindowsSurface::BitBltRegion(GXWindowsSurface* pSrcSurface, int xDest, int yDest, GRegion* lprgnSource)
{
  return m_pRenderTar->BitBltRegion(pSrcSurface->m_pRenderTar, xDest, yDest, lprgnSource);
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

      // ������Surface������������
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
      // �����Ƿ��ཻ,���ཻ��ʡ��һ�θ���

      GXRECT rcNew, rcOld, rcResult;
      prgnNewWnd->GetBounding(&rcNew);
      prgnOldWnd->GetBounding(&rcOld);

      // ������Ҫ���Ƶ���Ч����
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
        GXImage* pBackBufferImage = m_lpStation->pGraphics->GetBackBufferImg();
        pBackBufferImage->BitBltRegion(pNewWndOldSur->m_pRenderTar, 0, 0, prgnNewWnd);
        pDesktopSurface->BitBltRegion(pOldWndOldSur, 0, 0, prgnOldWnd);
        m_pRenderTar->BitBltRegion(pBackBufferImage, 0, 0, prgnNewWnd);
      }

      // �ƶ�
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

      // pNewWndOldSurҪ����NewWnd����Ч�������ų�OldWnd����Ч��
      GRegion* prgnDesktopSurUpdate = prgnNewWnd->CreateSubtract(prgnOldWnd);
      pDesktopSurface->InvalidateRegion(prgnDesktopSurUpdate);
      SAFE_RELEASE(prgnDesktopSurUpdate);
    }
    else // �������û�оɵĶ�ռ���� (prgnOldWnd == NULL)
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

      // pNewWndOldSurҪ����NewWnd����Ч��
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

// ����ֵ�� m_pUpdate ��״̬
RGNCOMPLEX GXWindowsSurface::ValidateBlank()
{
  // ���Windows֮��ĸ�����, ��Щ������Ϊû�д���, ����ֱ�Ӽ���
  RGNCOMPLEX rc = RC_ERROR;
  if(m_hExclusiveWnd != NULL)
  {
    GXRECT rcUpdate;
    GXRECT rcWindow;
    m_prgnUpdate->GetBounding(&rcUpdate);
    gxGetWindowRect(m_hExclusiveWnd, &rcWindow);

    // ������¾����ڴ��ھ���֮��
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