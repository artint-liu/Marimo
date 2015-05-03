#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
#include <Include/GUnknown.H>
#include <Include/GResource.H>
#include <Include/GRegion.H>
#include <Include/GTexture.H>
#include <Include/GXGraphics.H>
#include <Include/GXCanvas.H>
#include <Include/GXImage.H>

// 私有头文件
#include <User/GXWindow.h>
#include "Include/GXUser.H"
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>

const GXFLOAT  c_flScaleWidth  = 2.0f;
const GXFLOAT  c_flScaleHeight = 2.0f;

//#define TRACE_SURACE_PAINTING

#define IS_TOPMOST(_WNDSURF) TEST_FLAG(gxGetWindowLongW((_WNDSURF)->m_hExclusiveWnd, GXGWL_EXSTYLE), GXWS_EX_TOPMOST)

DesktopWindowsMgr::DesktopWindowsMgr(GXLPSTATION lpStation)
  : m_lpStation        (lpStation)
  , m_pDesktopWindows  (NULL)
  , m_pActiveWndSur    (NULL)
  , m_dwFlags          (GXDWM_AERO)
{
  m_pDesktopWindows = new GXWindowsSurface(lpStation, NULL);
  m_pDesktopWindows->AddRef();

  if(TRUE)
  {
    m_pActiveWndSur = new GXWindowsSurface(lpStation, NULL);
    m_pActiveWndSur->AddRef();

    m_aExclusiveSurface.push_back(m_pActiveWndSur);
    m_pActiveWndSur->AddRef();
  }
}

DesktopWindowsMgr::~DesktopWindowsMgr()
{
  for(WinsSurface_Iterator it = m_aExclusiveSurface.begin();
    it != m_aExclusiveSurface.end(); ++it)
  {
    SAFE_RELEASE(*it);
  }
  SAFE_RELEASE(m_pDesktopWindows);
  SAFE_RELEASE(m_pActiveWndSur);
}

GXBOOL DesktopWindowsMgr::InvalidateWndRegion(GXHWND hWnd, const GRegion* prgnUpdate, GXBOOL bWndCoord)
{
  GXBOOL bRet = TRUE;
  if(hWnd == NULL)
  {
    if(prgnUpdate == NULL)
    {
      m_pDesktopWindows->InvalidateRect(NULL);
      for(WinsSurface_Iterator it = m_aExclusiveSurface.begin();
        it != m_aExclusiveSurface.end(); ++it)
      {
        (*it)->InvalidateRect(NULL);
      }
    }
    else
    {
      m_pDesktopWindows->InvalidateRegion(prgnUpdate);
      for(WinsSurface_Iterator it = m_aExclusiveSurface.begin();
        it != m_aExclusiveSurface.end(); ++it)
      {
        (*it)->InvalidateRegion(prgnUpdate);
      }
    }
  }
  else
  {
    GXWnd* lpWnd = GXWND_PTR(hWnd);
    RGNCOMPLEX rc = RC_NULL;
    GRegion* prgnWindow;
    lpWnd->GetWindowRegion(&prgnWindow);
    if(prgnUpdate != NULL)
    {
      if(bWndCoord == TRUE)
      {
        GRegion* pWndUpdate = prgnUpdate->Clone();
        pWndUpdate->Offset(lpWnd->rectWindow.left, lpWnd->rectWindow.top);
        rc = prgnWindow->Intersect(pWndUpdate);
        SAFE_RELEASE(pWndUpdate);
      }
      else
        rc = prgnWindow->Intersect(prgnUpdate);
    }
    else
      rc = prgnWindow->GetComplexity();

    if(rc == RC_SIMPLE || rc == RC_COMPLEX)
      bRet = lpWnd->m_pWinsSurface->InvalidateRegion(prgnWindow);
    SAFE_RELEASE(prgnWindow);
  }
  return bRet;
}

GXHRESULT DesktopWindowsMgr::ManageWindowSurface(GXHWND hWnd, GXUINT message)
{
  LPGXWND pWnd = GXWND_PTR(hWnd);

  if(message == GXWM_CREATE)
  {
    LPGXWND     lpDesktop = pWnd->GetDesktop();
    GXLPSTATION lpStation = GXLPWND_STATION_PTR(pWnd);
    GXGraphics*  pGraphics = lpStation->pGraphics;

    ASSERT(lpStation->m_pDesktopWindowsMgr == this);
    // TopLevel 窗口
    if(gxIsTopLevelWindow(hWnd) == FALSE)
      return FALSE;

    AllocSurface(hWnd);
    InvalidateWndRegion(hWnd, NULL, FALSE);
    ActiveSurface(pWnd->m_pWinsSurface);

    GRegion* prgnWindow = NULL;

    pWnd->GetWindowRegion(&prgnWindow);
    pWnd->m_pWinsSurface->m_prgnWindows->Union(prgnWindow);
    SAFE_RELEASE(prgnWindow);

    return GX_OK;
  }
  else if(message == GXWM_DESTROY)
  {
    if(pWnd->m_pWinsSurface != NULL)
    {
      GXLPSTATION lpStation = GXLPWND_STATION_PTR(pWnd);
      pWnd->m_pWinsSurface->SetExclusiveWnd(NULL, SEW_DONOTBLT);
      if(lpStation->m_pDesktopWindowsMgr->FreeSurface(pWnd->m_pWinsSurface) < 0)
      {
        // 没有释放成功说明是Desktop Surface
        GRegion* prgnWnd;
        pWnd->GetWindowRegion(&prgnWnd);
        pWnd->m_pWinsSurface->InvalidateRegion(prgnWnd);
        pWnd->m_pWinsSurface->GenerateWindowsRgn(TRUE);
        SAFE_RELEASE(prgnWnd);
      }
      pWnd->m_pWinsSurface->Release();
      pWnd->m_pWinsSurface = NULL;
    }
    return GX_OK;
  }
  return GX_FAIL;
}

GXHRESULT DesktopWindowsMgr::AllocSurface(GXHWND hWnd)
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);

  if(lpWnd->m_uExStyle & GXWS_EX_LAYERED)
  {
    GXWindowsSurface* pExclusiveSur = new GXWindowsSurface(m_lpStation, hWnd);
    
    // 为加入列表增加引用计数
    if(IS_TOPMOST(pExclusiveSur))
    {
      m_aExclusiveSurface.push_back(pExclusiveSur);
    }
    else
    {
      // 将非 TopMost Surface 插入到最后一个 TopMost 之前
      for(WinsSurface_Iterator it = m_aExclusiveSurface.end() - 1;
        it != m_aExclusiveSurface.begin(); --it)
      {
        if( ! IS_TOPMOST(*it)) {
          m_aExclusiveSurface.insert(it + 1, pExclusiveSur);
        }
      }
    }
    pExclusiveSur->AddRef();

    // 为返回值增加引用计数
    lpWnd->m_pWinsSurface = pExclusiveSur;
    return pExclusiveSur->AddRef();
  }

  if(m_pActiveWndSur != NULL)
  {
    m_pActiveWndSur->SetExclusiveWnd(hWnd, NULL);
    return 0;
  }

  lpWnd->m_pWinsSurface = m_pDesktopWindows;
  return m_pDesktopWindows->AddRef();
}

GXHRESULT DesktopWindowsMgr::FreeSurface(GXWindowsSurface* pWinsSurface)
{
  //*/
  WinsSurface_Iterator it = std::find(m_aExclusiveSurface.begin(), m_aExclusiveSurface.end(), pWinsSurface);
  if(it == m_aExclusiveSurface.end()) {
    return 0;
  }

  GXWindowsSurface* pSurface = *it;
  if(pSurface != m_pActiveWndSur) {
    m_aExclusiveSurface.erase(it);
  }
  else {
    pSurface->AddRef();  // 为了 Release() 返回引用计数
  }
  return pSurface->Release();
  /*/
  int nIndex = FindSurface(pWinsSurface);
  if(nIndex < 0)
    return (GXHRESULT)nIndex;

  GXWindowsSurface* pSurface = m_aExclusiveSurface[nIndex];
  if(pSurface != m_pActiveWndSur)
    m_aExclusiveSurface.erase(m_aExclusiveSurface.begin() + nIndex);
  else
    pSurface->AddRef();  // 为了 Release() 返回引用计数
  return pSurface->Release();
  //*/
}

//GXINT DesktopWindowsMgr::FindSurface(GXWindowsSurface* pWinsSurface)
//{
//  for(WinsSurfaceArray::iterator it = m_aExclusiveSurface.begin();
//    it != m_aExclusiveSurface.end(); ++it)
//  {
//    if(*it == pWinsSurface)
//    {
//      return it - m_aExclusiveSurface.begin();
//    }
//  }
//  return -1;
//}

//GXLPWND DesktopWindowsMgr::SendChildPaintMessage(GXWindowsSurface* pSurface, GXLPWND lpParent)
//{
//  GXLPWND lpWnd = lpParent->m_pFirstChild;
//
//  while(lpWnd != NULL)
//  {
//    if((lpWnd->m_uStyle & WS_VISIBLE) &&
//      gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
//    {
//      GRegion* pRegion = NULL;
//
//      GXWindowsSurface* pWinsSurface = pSurface == NULL 
//        ? lpWnd->m_pWinsSurface
//        : pSurface;
//      ASSERT(pWinsSurface != NULL);
//    }
//    lpWnd = lpWnd->m_pNextFrame;
//  }
//  return NULL;
//}

GXBOOL DesktopWindowsMgr::SendPaintMessage()
{
  if(m_pDesktopWindows->m_bGenerateWindowsRgn == TRUE)
    m_pDesktopWindows->GenerateWindowsRgn(FALSE);
#ifdef TRACE_SURACE_PAINTING
  static int g_nDbgCount = 0;
#endif // #ifdef TRACE_SURACE_PAINTING

  // 桌面通用表面的更新检查
  if(m_pDesktopWindows->m_prgnUpdate->GetComplexity() != RC_NULL &&
    m_pDesktopWindows->ValidateBlank() != RC_NULL)
  {
    GXLPWND lpWnd = m_lpStation->lpDesktopWnd->m_pFirstChild;
    while(lpWnd != NULL)
    {
      if((lpWnd->m_uStyle & GXWS_VISIBLE) &&
        lpWnd->m_pWinsSurface == m_pDesktopWindows &&
        gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
      {
        lpWnd->UpdateWholeWindow(lpWnd->m_pWinsSurface);

#ifdef TRACE_SURACE_PAINTING
        if(gxGetAsyncKeyState(GXVK_LSHIFT) & 0x8000) {
          clStringW str;
          str.Format(L"%05d_D.png", g_nDbgCount++);
          m_pDesktopWindows->SaveToFileW(str, "PNG");
        }
#endif // #ifdef TRACE_SURACE_PAINTING
      }
      lpWnd = lpWnd->m_pNextWnd;
    }
  }

  // 私有的独占表面
  for(WinsSurface_Iterator it = m_aExclusiveSurface.begin(); 
    it != m_aExclusiveSurface.end(); ++it)
  {
    GXWindowsSurface* pSurface = *it;
    GXLPWND lpWnd = GXWND_PTR(pSurface->m_hExclusiveWnd);

    if( lpWnd != NULL &&
      pSurface->m_prgnUpdate->GetComplexity() != RC_NULL && 
      pSurface->ValidateBlank() != RC_NULL)
    {
      if((lpWnd->m_uStyle & GXWS_VISIBLE) &&
        gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
      {
        lpWnd->UpdateWholeWindow(lpWnd->m_pWinsSurface);

#ifdef TRACE_SURACE_PAINTING
        if(gxGetAsyncKeyState(GXVK_LSHIFT) & 0x8000) {
          clStringW str;
          str.Format(L"%05d_E.png", g_nDbgCount++);
          pSurface->SaveToFileW(str, "PNG");
        }
#endif // #ifdef TRACE_SURACE_PAINTING

      }
    }
  }
  return TRUE;
}

GXBOOL DesktopWindowsMgr::Render(GXCanvas* pCanvas)
{
  GXCARET*      lpCaret        = &m_lpStation->SysCaret;
  LPGXWND        lpCaretTopWnd = NULL;
  GXWindowsSurface*  pCaretSurface = NULL;    // 光标所在的Surface
  if(lpCaret->Tick() == TRUE)
  {
    lpCaretTopWnd = GXWND_PTR(lpCaret->hTopLevel);
    if(lpCaretTopWnd != NULL)
      pCaretSurface = lpCaretTopWnd->m_pWinsSurface;
    else
      ASSERT(0);
  }
  // TODO: 根据窗口区域绘制纹理
  GXREGN regn = {0, 0};
  m_pDesktopWindows->m_pRenderTar->GetTextureUnsafe()->GetDimension((GXUINT*)&regn.width, (GXUINT*)&regn.height);

  if(TEST_FLAG(m_dwFlags, GXDWM_AERO))
  {
    // AERO 效果绘制窗口层
    m_lpStation->pBackDownSampTexA->StretchRect(
      m_lpStation->pGraphics->GetDeviceOriginTex(), NULL, NULL, GXTEXFILTER_LINEAR);

    static GXSAMPLERDESC SampDesc;
    SampDesc.MagFilter = GXTEXFILTER_LINEAR;
    SampDesc.MinFilter = GXTEXFILTER_LINEAR;

    pCanvas->SetSamplerState(0, &SampDesc);
    pCanvas->SetSamplerState(1, &SampDesc);
    pCanvas->SetParametersInfo(CPI_SETEXTTEXTURE, 1, m_lpStation->pBackDownSampTexA);
    pCanvas->SetEffect(m_lpStation->m_pStockObject->pAeroEffect);

    float2 vTexelKernel((float)(c_flScaleWidth / g_SystemMetrics[GXSM_CXSCREEN]),
      (float)(c_flScaleHeight / g_SystemMetrics[GXSM_CYSCREEN]));
    pCanvas->SetEffectUniformByName2f("TexelKernel", &vTexelKernel);

    if(m_pDesktopWindows->m_prgnWindows->IsEmpty() == FALSE)
    {
      pCanvas->SetRegion(m_pDesktopWindows->m_prgnWindows, TRUE);
      pCanvas->DrawTexture(m_pDesktopWindows->m_pRenderTar->GetTextureUnsafe(), 0, 0, &regn);

      if(m_pDesktopWindows == pCaretSurface)
      {
        GRegion* prgnCaret;
        lpCaretTopWnd->GetSystemRegion(GSR_CLIPSIBLINGS, &prgnCaret);
        pCanvas->SetRegion(prgnCaret, TRUE);
        pCanvas->SetEffect(m_lpStation->m_pStockObject->pSimpleEffect);
        lpCaret->PaintCaret(pCanvas);
        pCanvas->SetEffect(m_lpStation->m_pStockObject->pAeroEffect);
        SAFE_RELEASE(prgnCaret);
      }
    }
  }
  else if(m_pDesktopWindows->m_prgnWindows->IsEmpty() == FALSE)
  {
    pCanvas->SetRegion(m_pDesktopWindows->m_prgnWindows, TRUE);
    pCanvas->DrawTexture(m_pDesktopWindows->m_pRenderTar->GetTextureUnsafe(), 0, 0, &regn);
    //pCanvas->SetCompositingMode(TEST_FLAG(m_dwFlags, GXDWM_ALPHA) ? CM_SourceOver : CM_SourceCopy);

    if(m_pDesktopWindows == pCaretSurface)
    {
      GRegion* prgnCaret;
      lpCaretTopWnd->GetSystemRegion(GSR_CLIPSIBLINGS, &prgnCaret);
      pCanvas->SetRegion(prgnCaret, TRUE);
      lpCaret->PaintCaret(pCanvas);
      SAFE_RELEASE(prgnCaret);
    }
  }


  for(WinsSurface_Iterator it = m_aExclusiveSurface.begin(); 
    it != m_aExclusiveSurface.end(); ++it)
  {
    GXWindowsSurface* pSurface = *it;
    GXLPWND lpWnd = GXWND_PTR(pSurface->m_hExclusiveWnd);
    RGNCOMPLEX r = pSurface->m_prgnWindows->GetComplexity();

    if(lpWnd != NULL &&
      (lpWnd->m_uStyle & GXWS_VISIBLE) && r != RC_NULL &&
      gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
    {
      if(TEST_FLAG(m_dwFlags, GXDWM_AERO))
      {
        pCanvas->Flush();
        m_lpStation->pBackDownSampTexA->StretchRect(
          m_lpStation->pGraphics->GetDeviceOriginTex(), NULL, NULL, GXTEXFILTER_LINEAR);
      }
      else {
        pCanvas->SetCompositingMode(TEST_FLAG(m_dwFlags, GXDWM_ALPHA) ? CM_SourceOver : CM_SourceCopy);
      }

      pCanvas->SetRegion(pSurface->m_prgnWindows, TRUE);
      pCanvas->DrawTexture(pSurface->m_pRenderTar->GetTextureUnsafe(), 0, 0, &regn);
      if(pSurface == pCaretSurface)
      {
        pCanvas->SetRegion(pSurface->m_prgnWindows, TRUE);
        if(TEST_FLAG(m_dwFlags, GXDWM_AERO))
          pCanvas->SetEffect(m_lpStation->m_pStockObject->pSimpleEffect);
        lpCaret->PaintCaret(pCanvas);
        // 不恢复Effect是因为带有Caret的窗口只能是Top窗口,也就是最后一个画出来的窗口
      }
    }
  }
  return TRUE;
}

GXWindowsSurface* DesktopWindowsMgr::GetSurface(GXINT nSurface)
{
  switch(nSurface)
  {
  case DWM_GS_DESKTOP:
    return m_pDesktopWindows;
  case DWM_GS_ACTIVEWND:
    return m_pActiveWndSur;
  default:
    return m_aExclusiveSurface[nSurface];
  }
  return NULL;
}

void DesktopWindowsMgr::ActiveWindows(GXINT uActiveState, DWM_ACTIVEWINDOWS* p)
{
  const GXDWORD dwFlags = GSR_CLIPSIBLINGS | GSR_WINDOW;
  if(uActiveState == GXWA_INACTIVE)
  {
    p->lpActiveWnd->GetSystemRegion(dwFlags, &p->prgnBefore);
    p->lpGenFirst = p->lpActiveWnd->m_pNextWnd;
    if(p->lpGenFirst == NULL)
      p->lpGenFirst = p->lpInsertWnd;
  }
  else if(uActiveState == GXWA_ACTIVE)
  {
    GXBOOL bNeedSubtract = FALSE;
    // 将激活的窗口放入Active层
    if(m_pActiveWndSur != NULL)
    {
      GXBOOL bActiveLayered = (p->lpActiveWnd->m_uExStyle & GXWS_EX_LAYERED) != 0;
      GXBOOL bInactiveLayered = p->lpInactiveWnd == NULL 
        ? FALSE
        : (p->lpInactiveWnd->m_uExStyle & GXWS_EX_LAYERED) != 0;
      if(bActiveLayered == FALSE && bInactiveLayered == FALSE)
      {
        m_pActiveWndSur->SetExclusiveWnd(GXWND_HANDLE(p->lpActiveWnd), NULL);
        bNeedSubtract = TRUE;
      }
      else if(bInactiveLayered == FALSE)  // 一定是: bActiveLayered == TRUE
        m_pActiveWndSur->SetExclusiveWnd(NULL, NULL);
      else if(bActiveLayered == FALSE)  // 一定是: bInactiveLayered == TRUE
      {
        m_pActiveWndSur->SetExclusiveWnd(GXWND_HANDLE(p->lpActiveWnd), NULL);
        m_pDesktopWindows->GenerateWindowsRgn(TRUE);
        m_pDesktopWindows->InvalidateRegion(p->prgnBefore);
        bNeedSubtract = TRUE;
      }
      else // 一定是: bActiveLayered == TRUE && bInactiveLayered == TRUE
      {
        // 调整顺序就行, 啥也不做了...
      }

    }
    else
    {
      if(p->lpActiveWnd->m_pWinsSurface->GetExclusiveWnd() != NULL)
        p->lpActiveWnd->m_pWinsSurface->GenerateWindowsRgn(TRUE);
      else
        m_pDesktopWindows->GenerateWindowsRgn(TRUE);

      bNeedSubtract = TRUE;
    }
    
    // 计算激活窗口的无效区域, 变化后减变化前
    if(bNeedSubtract == TRUE)
    {
      p->lpActiveWnd->GetSystemRegion(dwFlags, &p->prgnAfter);
      if(p->prgnAfter->Subtract(p->prgnBefore) != RC_NULL)
        p->lpActiveWnd->m_pWinsSurface->InvalidateRegion(p->prgnAfter);
    }

    SAFE_RELEASE(p->prgnBefore);
    SAFE_RELEASE(p->prgnAfter);

    // 调整独占Surface的顺序
    ActiveSurface(p->lpActiveWnd->m_pWinsSurface);
  }
  else
    ASSERT(0);
}

GXBOOL DesktopWindowsMgr::ActiveSurface(GXWindowsSurface* pWndSurface)
{
  if(pWndSurface != m_pDesktopWindows && 
    m_aExclusiveSurface.back() != pWndSurface)
  {
    //GXINT nIndex = FindSurface(pWndSurface);
    WinsSurface_Iterator it = std::find(m_aExclusiveSurface.begin(), m_aExclusiveSurface.end(), pWndSurface);
    ASSERT(it != m_aExclusiveSurface.end());
    WinsSurface_Iterator itNext = it + 1;
    
    // TopMost 窗口直接加到队列尾部
    if(IS_TOPMOST(pWndSurface))
    {
      //GXWindowsSurface* pTopSurface = *it;//m_aExclusiveSurface[nIndex];
      //ASSERT(pTopSurface == pWndSurface);
      m_aExclusiveSurface.erase(it);
      m_aExclusiveSurface.push_back(pWndSurface);
    }
    else
    {
      // 非 TopMost 窗口逐渐移动到 TopMost 之前
      while(itNext != m_aExclusiveSurface.end())
      {
        if(IS_TOPMOST(*itNext)) {
          break;
        }
        
        GXWindowsSurface* pTemp = *it;
        *it = *itNext;
        *itNext = pTemp;

        it = itNext;
        itNext = it + 1;
      }
    }
    //ASSERT(nIndex >= 0);
    return TRUE;
  }
  return FALSE;
}
#endif // _DEV_DISABLE_UI_CODE