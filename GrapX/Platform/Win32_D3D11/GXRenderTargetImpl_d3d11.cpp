#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "GrapX/GXRenderTarget.h"

// 私有头文件
#include "Platform/Win32_D3D11/GTextureImpl_D3D11.h"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "Platform/Win32_D3D11/GXRenderTargetImpl_d3d11.h"
//#include <clPathFile.h>
#ifdef ENABLE_GRAPHICS_API_DX11
//#include <FreeImage.h>

namespace D3D11
{

  GXRenderTargetImpl::GXRenderTargetImpl(GXGraphics* pGraphics, GXINT nWidth, GXINT nHeight)
    : m_pGraphics(static_cast<GXGraphicsImpl*>(pGraphics))
    , m_pColorTexture(NULL)
    , m_pDepthStencilTexture(NULL)
    , m_nWidth(nWidth)
    , m_nHeight(nHeight)
  {
  }

  GXRenderTargetImpl::~GXRenderTargetImpl()
  {
    SAFE_RELEASE(m_pColorTexture);
    SAFE_RELEASE(m_pDepthStencilTexture);
  }

  GXHRESULT GXRenderTargetImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GXRenderTargetImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      if(m_pColorTexture) {
        m_pGraphics->UnregisterResource(this);
      }
      delete this;
      return GX_OK;
    }

    return nRefCount;
  }

  GXHRESULT GXRenderTargetImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    return GX_OK;
  }

  GXBOOL GXRenderTargetImpl::GetRatio(GXSizeRatio* pWidth, GXSizeRatio* pHeight)
  {
    *pWidth  = m_nWidth < 0 ? static_cast<GXSizeRatio>(m_nWidth) : GXSizeRatio::Undefined;
    *pHeight = m_nHeight < 0 ? static_cast<GXSizeRatio>(m_nHeight) : GXSizeRatio::Undefined;
    return TRUE;
  }

  GXSIZE* GXRenderTargetImpl::GetDimension(GXSIZE* pDimension)
  {
    return m_pColorTexture->GetDimension(pDimension);
  }

  GXHRESULT GXRenderTargetImpl::GetColorTexture(GTexture** ppColorTexture, GXResUsage eUsage)
  {
    if(eUsage == GXResUsage::Default)
    {
      *ppColorTexture = m_pColorTexture;
      return m_pColorTexture->AddRef();
    }
    // TODO: else if(eUsage == GXResUsage::Read)
    return GX_FAIL;
  }

  GTexture* GXRenderTargetImpl::GetColorTextureUnsafe(GXResUsage eUsage)
  {
    return m_pColorTexture;
  }

  GXHRESULT GXRenderTargetImpl::GetDepthStencilTexture(GTexture** ppDepthStencilTexture)
  {
    if(m_pDepthStencilTexture)
    {
      *ppDepthStencilTexture = m_pDepthStencilTexture;
      return m_pDepthStencilTexture->AddRef();
    }
    *ppDepthStencilTexture = NULL;
    return GX_FAIL;
  }

  GXBOOL GXRenderTargetImpl::StretchRect(GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter)
  {
    // TODO: ...
    CLBREAK;
    return FALSE;
  }

  GXBOOL GXRenderTargetImpl::SaveToFile(GXLPCWSTR szFilePath, GXLPCSTR pImageFormat)
  {
    // TODO: ...
    CLBREAK;
    return FALSE;
  }

  GXBOOL GXRenderTargetImpl::SaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat)
  {
    // TODO: ...
    CLBREAK;
    return FALSE;
  }

  GXBOOL GXRenderTargetImpl::Initialize(GXFormat eColorFormat, GXFormat eDepthStencilFormat)
  {
    GXUINT nWidth  = m_nWidth;
    GXUINT nHeight = m_nHeight;
    if(m_nWidth < 0 || m_nHeight < 0)
    {
      GXGRAPHICSDEVICE_DESC sDesc;
      m_pGraphics->GetDesc(&sDesc);
      nWidth  = SizeRatioToDimension(m_nWidth, sDesc.BackBufferWidth, 0);
      nHeight = SizeRatioToDimension(m_nHeight, sDesc.BackBufferHeight, 0);
    }

    m_pColorTexture = new GTextureImpl_RenderTarget(m_pGraphics, eColorFormat, nWidth, nHeight);
    if(InlIsFailedToNewObject(m_pColorTexture)) {
      return FALSE;
    }

    if(_CL_NOT_(m_pColorTexture->InitRenderTexture())) {
      SAFE_RELEASE(m_pColorTexture);
      return FALSE;
    }

    if(eDepthStencilFormat != Format_Unknown) {
      m_pDepthStencilTexture = new GTextureImpl_DepthStencil(m_pGraphics, eDepthStencilFormat, nWidth, nHeight);

      if(InlIsFailedToNewObject(m_pDepthStencilTexture)) {
        return FALSE;
      }

      if(_CL_NOT_(m_pDepthStencilTexture->InitDepthStencil())) {
        SAFE_RELEASE(m_pColorTexture);
        SAFE_RELEASE(m_pDepthStencilTexture);
        return FALSE;
      }
    }

    return TRUE;
  }

  GTextureImpl_RenderTarget* GXRenderTargetImpl::IntGetColorTextureUnsafe()
  {
    return m_pColorTexture;
  }

  GTextureImpl_DepthStencil* GXRenderTargetImpl::IntGetDepthStencilTextureUnsafe()
  {
    return m_pDepthStencilTexture;
  }

} // namespace D3D11

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)