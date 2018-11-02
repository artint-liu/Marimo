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

  GXRenderTargetImpl::GXRenderTargetImpl(GXGraphics* pGraphics)
    : m_pGraphics(static_cast<GXGraphicsImpl*>(pGraphics))
    , m_pColorTexture(NULL)
    , m_pDepthStencilTexture(NULL)
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

  GXBOOL GXRenderTargetImpl::GetRatio(GXSizeRatio* pWidth, GXSizeRatio* pHeight)
  {
    // TODO: ...
    CLBREAK;
    return FALSE;
  }

  GXSIZE* GXRenderTargetImpl::GetDimension(GXSIZE* pDimension)
  {
    // TODO: ...
    CLBREAK;
    return pDimension;
  }

  GXHRESULT GXRenderTargetImpl::GetColorTexture(GTexture** ppColorTexture, GXResUsage eUsage)
  {
    // TODO: ...
    CLBREAK;
    return GX_FAIL;
  }

  GTexture* GXRenderTargetImpl::GetColorTextureUnsafe(GXResUsage eUsage)
  {
    // TODO: ...
    CLBREAK;
    return NULL;
  }

  GXHRESULT GXRenderTargetImpl::GetDepthStencilTexture(GTexture** ppDepthStencilTexture)
  {
    // TODO: ...
    CLBREAK;
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