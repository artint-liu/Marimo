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
#include "clImage.h"

// 私有头文件
#include "Platform/Win32_D3D11/GTextureImpl_D3D11.h"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include "Platform/Win32_D3D11/GXRenderTargetImpl_d3d11.h"
//#include <clPathFile.h>
#ifdef ENABLE_GRAPHICS_API_DX11
#include <FreeImage.h>

#define _RENDERTARGET_TEMPL template<class _TargetInterfaceT>
#define _RENDERTARGET_IMPL RenderTargetBase<_TargetInterfaceT>

namespace GrapX
{
  namespace D3D11
  {
    GXBOOL CheckRenderTargetFormatSupport(ID3D11Device* pD3D11Device, GXFormat eColorFormat, GXFormat eDepthStencilFormat)
    {
      UINT uColorSupport;
      UINT uDepthStencilSupport;

      pD3D11Device->CheckFormatSupport(GrapXToDX11::FormatFrom(eColorFormat), &uColorSupport);
      if (TEST_FLAG_NOT(uColorSupport, D3D11_FORMAT_SUPPORT_RENDER_TARGET)) {
        CLOG_ERROR("RenderTarget: %s 格式不能作为渲染纹理", FormatToString(eColorFormat));
        return FALSE;
      }

      if (eDepthStencilFormat != Format_Unknown) {
        pD3D11Device->CheckFormatSupport(GrapXToDX11::FormatFrom(eDepthStencilFormat), &uDepthStencilSupport);
        if (TEST_FLAG_NOT(uDepthStencilSupport, D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)) {
          CLOG_ERROR("RenderTarget: %s 格式不能作为渲染模板深度缓冲", FormatToString(eDepthStencilFormat));
          return FALSE;
        }
      }
      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    _RENDERTARGET_TEMPL
      _RENDERTARGET_IMPL::RenderTargetBase(GraphicsImpl* pGraphics)
      : m_pGraphics(pGraphics)
    {
    }


    _RENDERTARGET_TEMPL
      _RENDERTARGET_IMPL::RenderTargetBase(GraphicsImpl* pGraphics, TextureImpl_RenderTarget* pColorTexture, TextureImpl_DepthStencil* pDepthTexture)
      : m_pGraphics(pGraphics)
      , m_pColorTexture(pColorTexture)
      , m_pDepthStencilTexture(pDepthTexture)
    {
      pColorTexture->AddRef();
      pDepthTexture->AddRef();
    }

    _RENDERTARGET_TEMPL
      _RENDERTARGET_IMPL::~RenderTargetBase()
    {
      SAFE_RELEASE(m_pColorTexture);
      SAFE_RELEASE(m_pDepthStencilTexture);
    }

    //_RENDERTARGET_TEMPL
    //GXBOOL _RENDERTARGET_IMPL::Initialize(GXDWORD dwResType, GXUINT width, GXUINT height, GXFormat eColorFormat, GXFormat eDepthStencilFormat)
    //{
    //  //
    //  // 检查
    //  //
    //  ID3D11Device* pD3D11Device = m_pGraphics->D3DGetDevice();
    //  ASSERT(m_pColorTexture == NULL);
    //  ASSERT(m_pDepthStencilTexture == NULL);
    //  
    //  if (CheckRenderTargetFormatSupport(pD3D11Device, eColorFormat, eDepthStencilFormat) == FALSE) {
    //    return FALSE;
    //  }

    //  m_pColorTexture = new _RenderTargetTextureTempl(m_pGraphics, eColorFormat, width, height);
    //  if (InlIsFailedToNewObject(m_pColorTexture)) {
    //    return FALSE;
    //  }

    //  if (_CL_NOT_(m_pColorTexture->InitRenderTexture(NULL))) {
    //    SAFE_RELEASE(m_pColorTexture);
    //    return FALSE;
    //  }

    //  if (eDepthStencilFormat != Format_Unknown) {
    //    return InitDepthStencil(eDepthStencilFormat, width, height);
    //  }
    //  return TRUE;
    //}

    _RENDERTARGET_TEMPL
    GXBOOL _RENDERTARGET_IMPL::InitDepthStencil(GXFormat eDepthStencilFormat, GXUINT nWidth, GXUINT nHeight)
    {
      m_pDepthStencilTexture = new TextureImpl_DepthStencil(m_pGraphics, eDepthStencilFormat, nWidth, nHeight);

      if (InlIsFailedToNewObject(m_pDepthStencilTexture)) {
        return FALSE;
      }

      if (_CL_NOT_(m_pDepthStencilTexture->InitDepthStencil())) {
        SAFE_RELEASE(m_pColorTexture);
        SAFE_RELEASE(m_pDepthStencilTexture);
        return FALSE;
      }
      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////

    RenderTargetImpl::RenderTargetImpl(Graphics* pGraphics, GXINT nWidth, GXINT nHeight)
      : RenderTargetBase(static_cast<GraphicsImpl*>(pGraphics))
      , m_nWidth(nWidth)
      , m_nHeight(nHeight)
    {
    }

    RenderTargetImpl::RenderTargetImpl(Graphics* pGraphics, GXINT nWidth, GXINT nHeight, TextureImpl_RenderTarget* pColorTexture, TextureImpl_DepthStencil* pDepthTexture)
      : RenderTargetBase(static_cast<GraphicsImpl*>(pGraphics), pColorTexture, pDepthTexture)
      , m_nWidth(nWidth)
      , m_nHeight(nHeight)
    {
    }

    RenderTargetImpl::~RenderTargetImpl()
    {
      SAFE_RELEASE(m_pReadBackTexture);
    }

    GXHRESULT RenderTargetImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT RenderTargetImpl::Release()
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

    GXHRESULT RenderTargetImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      return GX_OK;
    }

    GXBOOL RenderTargetImpl::GetRatio(GXSizeRatio* pWidth, GXSizeRatio* pHeight)
    {
      *pWidth = m_nWidth < 0 ? static_cast<GXSizeRatio>(m_nWidth) : GXSizeRatio::Undefined;
      *pHeight = m_nHeight < 0 ? static_cast<GXSizeRatio>(m_nHeight) : GXSizeRatio::Undefined;
      return TRUE;
    }

    GXSIZE* RenderTargetImpl::GetDimension(GXSIZE* pDimension)
    {
      return m_pColorTexture->GetDimension(pDimension);
    }

    GXHRESULT RenderTargetImpl::GetColorTexture(Texture** ppColorTexture, GXResUsage eUsage)
    {
      if(eUsage == GXResUsage::Default)
      {
        *ppColorTexture = m_pColorTexture;
        return m_pColorTexture->AddRef();
      }
      else if(eUsage == GXResUsage::Read)
      {
        TextureImpl_GPUReadBack* pReadBackTexture = NULL;
        if(IntCreateReadBackTexture(&pReadBackTexture))
        {
          ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
          pD3D11Context->CopyResource(pReadBackTexture->D3DTexture(), m_pColorTexture->D3DTexture());
          *ppColorTexture = pReadBackTexture;
          pReadBackTexture = NULL;
          return GX_OK;
        }
      }
      return GX_FAIL;
    }

    Texture* RenderTargetImpl::GetColorTextureUnsafe(GXResUsage eUsage)
    {
      if(eUsage == GXResUsage::Default)
      {
        return m_pColorTexture;
      }
      else if(eUsage == GXResUsage::Read)
      {
        if(m_pReadBackTexture || IntCreateReadBackTexture(&m_pReadBackTexture))
        {
          ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
          pD3D11Context->CopyResource(m_pReadBackTexture->D3DTexture(), m_pColorTexture->D3DTexture());
          return m_pReadBackTexture;
        }
      }
      return NULL;
    }

    GXHRESULT RenderTargetImpl::GetDepthStencilTexture(Texture** ppDepthStencilTexture)
    {
      if(m_pDepthStencilTexture)
      {
        *ppDepthStencilTexture = m_pDepthStencilTexture;
        return m_pDepthStencilTexture->AddRef();
      }
      *ppDepthStencilTexture = NULL;
      return GX_FAIL;
    }

    GXBOOL RenderTargetImpl::StretchRect(Texture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter)
    {
      // TODO: ...
      CLBREAK;
      return FALSE;
    }

    GXBOOL RenderTargetImpl::SaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip)
    {
      GXSIZE sDimension;
      GXFormat format = m_pColorTexture->GetFormat();
      m_pColorTexture->GetDimension(&sDimension);
      Texture* pReadBackTexture = NULL;

      GetColorTexture(&pReadBackTexture, GXResUsage::Read);
      GXBOOL bval = pReadBackTexture->SaveToMemory(pBuffer, pImageFormat, bVertFlip);

      SAFE_RELEASE(pReadBackTexture);
      return bval;
    }

    GXBOOL RenderTargetImpl::SaveToFile(GXLPCWSTR szFilePath, GXLPCSTR pImageFormat, GXBOOL bVertFlip)
    {
      clstd::MemBuffer buffer;
      if(SaveToMemory(&buffer, pImageFormat, bVertFlip))
      {
        clstd::File file;
        if(file.CreateAlways(szFilePath))
        {
          file.Write(buffer);
          return TRUE;
        }
      }
      return FALSE;
    }

    GXBOOL RenderTargetImpl::Initialize(GXFormat eColorFormat, GXFormat eDepthStencilFormat)
    {
      GXUINT nWidth = m_nWidth;
      GXUINT nHeight = m_nHeight;
      if(m_nWidth < 0 || m_nHeight < 0)
      {
        GXGRAPHICSDEVICE_DESC sDesc;
        m_pGraphics->GetDesc(&sDesc);
        nWidth = SizeRatioToDimension(m_nWidth, sDesc.BackBufferWidth, 0);
        nHeight = SizeRatioToDimension(m_nHeight, sDesc.BackBufferHeight, 0);
      }

      //
      // 检查
      //
      ID3D11Device* pD3D11Device = m_pGraphics->D3DGetDevice();
      ASSERT(m_pColorTexture == NULL);
      ASSERT(m_pDepthStencilTexture == NULL);

      if (CheckRenderTargetFormatSupport(pD3D11Device, eColorFormat, eDepthStencilFormat) == FALSE) {
        return FALSE;
      }

      m_pColorTexture = new TextureImpl_RenderTarget(m_pGraphics, RESTYPE_RENDERTEXTURE, eColorFormat, nWidth, nHeight);
      if (InlIsFailedToNewObject(m_pColorTexture)) {
        return FALSE;
      }

      if (_CL_NOT_(m_pColorTexture->InitRenderTexture(NULL))) {
        SAFE_RELEASE(m_pColorTexture);
        return FALSE;
      }

      if (eDepthStencilFormat != Format_Unknown) {
        return InitDepthStencil(eDepthStencilFormat, nWidth, nHeight);
      }
      return TRUE;
    }

    TextureImpl_RenderTarget* RenderTargetImpl::IntGetColorTextureUnsafe()
    {
      return m_pColorTexture;
    }

    TextureImpl_DepthStencil* RenderTargetImpl::IntGetDepthStencilTextureUnsafe()
    {
      return m_pDepthStencilTexture;
    }

    GXBOOL RenderTargetImpl::IntCreateReadBackTexture(TextureImpl_GPUReadBack** ppReadBackTex)
    {
      GXSIZE sDimension;
      GXFormat format = m_pColorTexture->GetFormat();
      m_pColorTexture->GetDimension(&sDimension);

      TextureImpl_GPUReadBack* pReadBackTexture = new TextureImpl_GPUReadBack(m_pGraphics, format, sDimension.cx, sDimension.cy);
      if(InlIsFailedToNewObject(pReadBackTexture)) {
        CLOG_ERROR("%s(%d): 无法创建纹理对象", __FUNCTION__, __LINE__);
        return FALSE;
      }

      D3D11_TEXTURE2D_DESC desc = {0};
      m_pColorTexture->D3DTexture()->GetDesc(&desc);

      //ASSERT(m_pColorTexture->m_dwResType == RESTYPE_RENDERTEXTURE || m_pColorTexture->m_dwResType == RESTYPE_CUBERENDERTARGET);
      if(_CL_NOT_(pReadBackTexture->InitReadBackTexture(desc.ArraySize)))
      {
        CLOG_ERROR("%s(%d): 初始化纹理失败", __FUNCTION__, __LINE__);
        SAFE_RELEASE(pReadBackTexture);
        return FALSE;
      }
      *ppReadBackTex = pReadBackTexture;
      return TRUE;
    }

    //GXBOOL RenderTargetImpl::InitDepthStencil(GXFormat eDepthStencilFormat, GXUINT nWidth, GXUINT nHeight)
    //{
    //  m_pDepthStencilTexture = new TextureImpl_DepthStencil(m_pGraphics, eDepthStencilFormat, nWidth, nHeight);

    //  if(InlIsFailedToNewObject(m_pDepthStencilTexture)) {
    //    return FALSE;
    //  }

    //  if(_CL_NOT_(m_pDepthStencilTexture->InitDepthStencil())) {
    //    SAFE_RELEASE(m_pColorTexture);
    //    SAFE_RELEASE(m_pDepthStencilTexture);
    //    return FALSE;
    //  }
    //  return TRUE;
    //}

    //////////////////////////////////////////////////////////////////////////

    GXHRESULT RenderTargetImpl_BackBuffer::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }

    RenderTargetImpl_BackBuffer::RenderTargetImpl_BackBuffer(Graphics* pGraphics)
      : RenderTargetImpl(pGraphics, GXINT(GXSizeRatio::Same), GXINT(GXSizeRatio::Same))
    {
    }

    GXBOOL RenderTargetImpl_BackBuffer::InitializeWithSwapChain(IDXGISwapChain* pD3D11SwapChain)
    {
      ID3D11Device* pD3D11Device = m_pGraphics->D3DGetDevice();
      ID3D11Texture2D* pBackBuffer = NULL;

      HRESULT hval = pD3D11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
      if(FAILED(hval)) {
        return FALSE;
      }

      m_pColorTexture = new TextureImpl_RenderTarget(m_pGraphics, RESTYPE_RENDERTEXTURE, Format_Unknown, 0, 0);
      if(InlIsFailedToNewObject(m_pColorTexture)) {
        return FALSE;
      }

      if(m_pColorTexture->InitRenderTexture(pBackBuffer) == FALSE) {
        return FALSE;
      }
      pBackBuffer->Release();

      GXSIZE sDimension;
      m_pColorTexture->GetDimension(&sDimension);

      // 初始化DepthStencil

      return InitDepthStencil(Format_D24S8, sDimension.cx, sDimension.cy);

      //hval = pD3D11Device->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
      //m_pDeviceOriginTex->SetTexture(pBackBuffer);
      //if(FAILED(hval))
      //  return hval;
      //// ==========
      //m_pCurRenderTargetView = m_pRenderTargetView;
      //m_pCurRenderTargetView->AddRef();

      //////////////////////////////////////////////////////////////////////////
      // Create depth stencil texture
      //D3D11_TEXTURE2D_DESC descDepth;
      //ZeroMemory(&descDepth, sizeof(descDepth));
      //descDepth.Width = rcWndClient.right;
      //descDepth.Height = rcWndClient.bottom;
      //descDepth.MipLevels = 1;
      //descDepth.ArraySize = 1;
      //descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
      //descDepth.SampleDesc.Count = 1;
      //descDepth.SampleDesc.Quality = 0;
      //descDepth.Usage = D3D11_USAGE_DEFAULT;
      //descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
      //descDepth.CPUAccessFlags = 0;
      //descDepth.MiscFlags = 0;
      //hval = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
      //if(FAILED(hval))
      //  return hval;

      //// Create the depth stencil view
      //D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
      //ZeroMemory(&descDSV, sizeof(descDSV));
      //descDSV.Format = descDepth.Format;
      //descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      //descDSV.Texture2D.MipSlice = 0;
      //hval = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
      //if(FAILED(hval))
      //  return hval;
    }

    //////////////////////////////////////////////////////////////////////////

    CubeRenderTargetImpl::CubeRenderTargetImpl(Graphics* pGraphics)
      : RenderTargetBase(static_cast<GraphicsImpl*>(pGraphics))
    {
    }

    CubeRenderTargetImpl::~CubeRenderTargetImpl()
    {
      SAFE_RELEASE(m_pReadBackTexture);
      for (int n = 0; n < countof(m_pCubeFace); n++)
      {
        SAFE_RELEASE(m_pCubeFace[n])
        SAFE_RELEASE(m_pRenderTargetFace[n]);
      }

    }

    GXHRESULT CubeRenderTargetImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT CubeRenderTargetImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if (nRefCount == 0)
      {
        if (m_pColorTexture) {
          m_pGraphics->UnregisterResource(this);
        }
        delete this;
        return GX_OK;
      }

      return nRefCount;
    }

    GXHRESULT CubeRenderTargetImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      return GX_OK;
    }

    GXBOOL CubeRenderTargetImpl::Initialize(GXUINT nSize, GXFormat eColorFormat, GXFormat eDepthStencilFormat)
    {
      //
      // 检查
      //
      ID3D11Device* pD3D11Device = m_pGraphics->D3DGetDevice();
      ASSERT(m_pColorTexture == NULL);
      ASSERT(m_pDepthStencilTexture == NULL);

      if (CheckRenderTargetFormatSupport(pD3D11Device, eColorFormat, eDepthStencilFormat) == FALSE) {
        return FALSE;
      }

      m_pColorTexture = new TextureImpl_RenderTarget(m_pGraphics, RESTYPE_CUBERENDERTARGET, eColorFormat, nSize, nSize);
      if (InlIsFailedToNewObject(m_pColorTexture)) {
        return FALSE;
      }

      if (_CL_NOT_(m_pColorTexture->InitRenderTexture(NULL))) {
        SAFE_RELEASE(m_pColorTexture);
        return FALSE;
      }

      if (eDepthStencilFormat != Format_Unknown) {
        if(_CL_NOT_(InitDepthStencil(eDepthStencilFormat, nSize, nSize))) {
          return FALSE;
        }
      }


      //GXBOOL bval = RenderTargetBase::Initialize(RESTYPE_CUBERENDERTARGET, nSize, nSize, eColorFormat, eDepthStencilFormat);
      //if(_CL_NOT_(bval)) {
      //  return bval;
      //}

      for(int n = 0; n < countof(m_pCubeFace); n++)
      {
        m_pCubeFace[n] = new // (m_CubeFaceTextureBuffer + sizeof(CubeFaceRenderTargetTextureImpl) * n)
          CubeFaceRenderTargetTextureImpl(m_pGraphics, RESTYPE_RENDERTEXTURE, eColorFormat, nSize, nSize);
        m_pCubeFace[n]->AddRef();
        m_pCubeFace[n]->InitRenderTexture(m_pColorTexture->D3DTexture(), n);

        m_pRenderTargetFace[n] = new // (m_CubeFaceRenderTargetBuffer + sizeof(RenderTargetImpl) * n)
          RenderTargetImpl(m_pGraphics, nSize, nSize, m_pCubeFace[n], m_pDepthStencilTexture);
        m_pRenderTargetFace[n]->AddRef();
      }
      return TRUE;
    }

    RenderTarget* CubeRenderTargetImpl::GetFaceUnsafe(Face face)
    {
      return static_cast<RenderTarget*>(m_pRenderTargetFace[(int)face]);
    }

    RenderTarget** CubeRenderTargetImpl::GetFacesUnsafe()
    {
      return reinterpret_cast<RenderTarget**>(m_pRenderTargetFace);
    }

  } // namespace D3D11
} // namespace GrapX

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)