#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXKernel.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// 私有头文件
#include "Platform/Win32_D3D11/GTextureImpl_D3D11.h"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include <clPathFile.h>
#ifdef ENABLE_GRAPHICS_API_DX11
#include <FreeImage.h>

// Resource Usage |Default |Dynamic |Immutable |Staging
// GPU - Read     |yes     |yes     |yes       |yes¹
// GPU - Write    |yes     |        |          |yes¹
// CPU - Read     |        |        |          |yes¹
// CPU - Write    |        |yes     |          |yes¹
//
// 1 - GPU read or write of a resource with the D3D11_USAGE_STAGING usage is restricted to copy operations.
// You use ID3D11DeviceContext::CopySubresourceRegion and ID3D11DeviceContext::CopyResource for these copy
// operations. Also, because depth-stencil formats and multisample layouts are implementation details of a
// particular GPU design, the operating system can’t expose these formats and layouts to the CPU in general.
// Therefore, staging resources can't be a depth-stencil buffer or a multisampled render target.

namespace GrapX
{
  namespace D3D11
  {
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

    GXHRESULT TextureImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      INVOKE_DESC_CHECK(pDesc);

      switch(pDesc->dwCmdCode)
      {
      case RC_LostDevice:
        break;
      case RC_ResetDevice:
        break;
      }
      return GX_OK;
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT TextureImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT TextureImpl::Release()
    {
      clstd::ScopedLocker sl(m_pGraphics->GetLocker());
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        //OnDeviceEvent(DE_LostDevice);
        INVOKE_LOST_DEVICE;
        if(m_pD3D11Texture && m_pD3D11ShaderView) {
          m_pGraphics->UnregisterResource(this);
        }
        delete this;
        return GX_OK;
      }
      //else if(m_uRefCount == 1)
      //{
      //  return m_pGraphics->OldUnregisterResource(this);
      //}

      return nRefCount;
    }

    TextureImpl::TextureImpl(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight, GXUINT nMipLevels, GXResUsage eResUsage)
      : TextureBaseImplT<Texture>(static_cast<GraphicsImpl*>(pGraphics))
      , m_pTextureData    (NULL)
      , m_nMipLevels      (nMipLevels)
      , m_Format          (eFormat)
      , m_eResUsage       (eResUsage)
      , m_nWidth          (nWidth)
      , m_nHeight         (nHeight)
    {
      InlSetZeroT(m_sMappedResource);
    }

    TextureImpl::~TextureImpl()
    {
      SAFE_RELEASE(m_pD3D11Texture);
      SAFE_RELEASE(m_pD3D11ShaderView);
      SAFE_DELETE_ARRAY(m_pTextureData);
    }

    GXBOOL TextureImpl::InitTexture(GXBOOL bRenderTarget, GXLPCVOID pInitData, GXUINT nPitch)
    {
      const GXUINT nMinPitch = GetMinPitchSize();
      nPitch = clMax(nPitch, nMinPitch);

      if(m_eResUsage != GXResUsage::SystemMem)
      {
        if(_CL_NOT_(IntD3D11CreateResource(bRenderTarget, pInitData, nPitch))) {
          return FALSE;
        }
      }

      if(m_eResUsage == GXResUsage::Read || m_eResUsage == GXResUsage::ReadWrite || m_eResUsage == GXResUsage::SystemMem)
      {
        const GXUINT nSize = nMinPitch * m_nHeight;
        m_pTextureData = new GXBYTE[nSize];
        GXLPBYTE pDest = m_pTextureData;

        if(pInitData)
        {
          for(GXUINT y = 0; y < m_nHeight; y++, pDest += nMinPitch) {
            memcpy(pDest, reinterpret_cast<GXLPVOID>((size_t)pInitData + nPitch * y), nMinPitch);
          }
        }
      }

      return TRUE;
    }

    GXBOOL TextureImpl::IntD3D11CreateResource(GXBOOL bRenderTarget, GXLPCVOID pInitData, GXUINT nPitch)
    {
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_TEXTURE2D_DESC TexDesc;
      D3D11_SUBRESOURCE_DATA TexInitData;
      InlSetZeroT(TexDesc);
      InlSetZeroT(TexInitData);

      TexDesc.Width = m_nWidth;
      TexDesc.Height = m_nHeight;
      TexDesc.MipLevels = m_nMipLevels;
      TexDesc.ArraySize = 1;
      TexDesc.Format = GrapXToDX11::FormatFrom(m_Format);
      TexDesc.SampleDesc.Count = 1;
      TexDesc.SampleDesc.Quality = 0;

      if(m_Format == Format_D32 || m_Format == Format_D16 ||
        m_Format == Format_D24S8 || m_Format == Format_D24X8)
      {
        TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
      }
      else if(bRenderTarget) {
        TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
      }
      else {
        TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      }

      GrapXToDX11::TextureDescFromResUsage(&TexDesc, m_eResUsage, (pInitData != NULL));

      if(pInitData) {
        TexInitData.pSysMem = pInitData;
        TexInitData.SysMemPitch = nPitch;
      }

      ASSERT(TexDesc.Width < 16384 && TexDesc.Height < 16384);
      HRESULT hval = pd3dDevice->CreateTexture2D(&TexDesc, pInitData ? &TexInitData : NULL, &m_pD3D11Texture);
      //ASSERT(SUCCEEDED(hval)); // 临时
      if(FAILED(hval)) {
        return FALSE;
      }

      if(TEST_FLAG(TexDesc.BindFlags, D3D11_BIND_SHADER_RESOURCE)) {
        hval = pd3dDevice->CreateShaderResourceView(m_pD3D11Texture, NULL, &m_pD3D11ShaderView);
        return SUCCEEDED(hval);
      }
      return TRUE;
    }

    GXUINT TextureImpl::GetMinPitchSize() const
    {
      return GetBytesOfGraphicsFormat(m_Format) * m_nWidth;
    }

    //////////////////////////////////////////////////////////////////////////
    GXBOOL TextureImpl::Clear(GXCOLOR dwColor)
    {
      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
      if(m_eResUsage == GXResUsage::Write || m_eResUsage == GXResUsage::ReadWrite || m_eResUsage == GXResUsage::SystemMem)
      {
        int nWidth;
        int nHeight;
        //if(lpRect == NULL) {
        nWidth = m_nWidth;
        nHeight = m_nHeight;
        //}
        //else {
        //  nWidth = lpRect->right - lpRect->left;
        //  nHeight = lpRect->bottom - lpRect->top;
        //}
        GXUINT cbFormat = GetBytesOfGraphicsFormat(m_Format);
        GXLPVOID pData = new GXBYTE[cbFormat * m_nWidth * m_nHeight];
        if(cbFormat == 4)
        {
          GXDWORD* pColor = (GXDWORD*)pData;
          for(int i = nWidth * nHeight; i > 0; i--)
          {
            *pColor++ = dwColor;
          }
        }
        else {
          ASSERT(0);
          // 目前不支持
        }

        pD3D11Context->UpdateSubresource(m_pD3D11Texture, 0, NULL, pData, nWidth * cbFormat, NULL);

        //SAFE_RELEASE(pTempTexture);
        SAFE_DELETE_ARRAY(pData);
        return TRUE;
      }
      return FALSE;
    }

    //////////////////////////////////////////////////////////////////////////

    GXBOOL TextureImpl::CopyRect(Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource)
    {
      return TRUE;
    }

    GXBOOL TextureImpl::Map(MAPPED* pMappedRect, GXResMap eResMap)
    {
      // 不能嵌套Map/Unmap
      if(m_sMappedResource.pData) {
        return FALSE;
      }

      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
      D3D11_MAPPED_SUBRESOURCE SubResource;
      HRESULT hval = S_OK;
      InlSetZeroT(SubResource);

      if(m_eResUsage == GXResUsage::Default)
      {
        return FALSE;
      }
      else if(m_eResUsage == GXResUsage::Read && eResMap == GXResMap::Read)
      {
        pMappedRect->pBits = m_pTextureData;
        pMappedRect->Pitch = GetMinPitchSize();
      }
      else if(m_eResUsage == GXResUsage::Write && eResMap == GXResMap::Write)
      {
        if(FAILED(pD3D11Context->Map(m_pD3D11Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_sMappedResource)))
        {
          return FALSE;
        }
        pMappedRect->pBits = m_sMappedResource.pData;
        pMappedRect->Pitch = m_sMappedResource.RowPitch;
      }
      else if(m_eResUsage == GXResUsage::ReadWrite)
      {
        if(eResMap == GXResMap::Write)
        {
          pMappedRect->pBits = m_sMappedResource.pData;
          pMappedRect->Pitch = m_sMappedResource.RowPitch;
        }
        else if(eResMap == GXResMap::Read || eResMap == GXResMap::ReadWrite)
        {
          if(FAILED(pD3D11Context->Map(m_pD3D11Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_sMappedResource)))
          {
            return FALSE;
          }
          pMappedRect->pBits = m_pTextureData;
          pMappedRect->Pitch = GetMinPitchSize();
        }
        else {
          return FALSE;
        }
      }
      else if(m_eResUsage == GXResUsage::SystemMem)
      {
        pMappedRect->pBits = m_pTextureData;
        pMappedRect->Pitch = GetMinPitchSize();
      }
      else {
        return FALSE;
      }

      //if(pRect)
      //{
      //  pMappedRect->pBits = reinterpret_cast<GXLPVOID>(
      //    reinterpret_cast<size_t>(pMappedRect->pBits) + pMappedRect->Pitch * pRect->top + GetBytesOfGraphicsFormat(m_Format) * pRect->left);
      //}

      return TRUE;
    }

    GXBOOL TextureImpl::Unmap()
    {
      if(m_sMappedResource.pData)
      {
        ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
        if(m_pTextureData)
        {
          const GXUINT nMinPitch = GetMinPitchSize();
          for(GXUINT y = 0; y < m_nHeight; y++)
          {
            memcpy(static_cast<GXLPBYTE>(m_sMappedResource.pData) + y * m_sMappedResource.RowPitch,
              m_pTextureData + y * nMinPitch, m_sMappedResource.RowPitch);
          }
        }
        pD3D11Context->Unmap(m_pD3D11Texture, 0);
        InlSetZeroT(m_sMappedResource);
      }
      return TRUE;
    }

    GXBOOL TextureImpl::UpdateRect(GXLPCRECT prcDest, GXLPVOID pData, GXUINT nPitch)
    {
      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
      D3D11_BOX box;
      InlSetZeroT(box);

      if(prcDest)
      {
        box.left = prcDest->left;
        box.top = prcDest->top;
        box.right = prcDest->right;
        box.bottom = prcDest->bottom;
        box.back = 1;
      }

      pD3D11Context->UpdateSubresource(m_pD3D11Texture, 0, prcDest ? &box : NULL, pData, nPitch, 0);
      return TRUE;
    }

    Graphics*  TextureImpl::GetGraphicsUnsafe()
    {
      return m_pGraphics;
    }

    GXSIZE* TextureImpl::GetDimension(GXSIZE* pDimension)
    {
      if(pDimension != NULL) {
        pDimension->cx = m_nWidth;
        pDimension->cy = m_nHeight;
      }
      return pDimension;
    }

    GXResUsage TextureImpl::GetUsage()
    {
      return m_eResUsage;
    }

    GXFormat TextureImpl::GetFormat()
    {
      return m_Format;
    }

    GXVOID TextureImpl::GenerateMipMaps()
    {
    }

    GXBOOL TextureImpl::GetDesc(GXBITMAP*lpBitmap)
    {
      //D3DSURFACE_DESC sd;
      //if(FAILED(m_pTexture->GetLevelDesc(0, &sd)))
      //  return FALSE;

      //lpBitmap->bmType  = 0;
      //lpBitmap->bmWidth = sd.Width;
      //lpBitmap->bmHeight = sd.Height;
      //lpBitmap->bmWidthBytes = sd.Width;
      //lpBitmap->bmPlanes = 1;
      //switch(sd.Format)
      //{
      //case D3DFMT_A8R8G8B8:
      //case D3DFMT_X8R8G8B8:
      //  lpBitmap->bmBitsPixel = 32;
      //  break;
      //case D3DFMT_A8:
      //  lpBitmap->bmBitsPixel = 8;
      //  break;
      //default:
      //  ASSERT(FALSE);
      //}
      //lpBitmap->bmBits = NULL;
      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    TextureImpl_RenderTarget::TextureImpl_RenderTarget(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Read)
      , m_pD3D11RenderTargetView(NULL)
    {
    }

    TextureImpl_RenderTarget::~TextureImpl_RenderTarget()
    {
      SAFE_RELEASE(m_pD3D11RenderTargetView);
    }

    GXBOOL TextureImpl_RenderTarget::InitRenderTexture()
    {
      if(_CL_NOT_(TextureImpl::InitTexture(TRUE, NULL, 0))) {
        return FALSE;
      }

      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_RENDER_TARGET_VIEW_DESC TarDesc;
      InlSetZeroT(TarDesc);

      TarDesc.Format = GrapXToDX11::FormatFrom(m_Format);
      TarDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      HRESULT hval = pd3dDevice->CreateRenderTargetView(m_pD3D11Texture, &TarDesc, &m_pD3D11RenderTargetView);
      return SUCCEEDED(hval);
    }

    //////////////////////////////////////////////////////////////////////////

    TextureImpl_DepthStencil::TextureImpl_DepthStencil(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Read)
      , m_pD3D11DepthStencilView(NULL)
    {
    }

    TextureImpl_DepthStencil::~TextureImpl_DepthStencil()
    {
      SAFE_RELEASE(m_pD3D11DepthStencilView);
    }

    GXBOOL TextureImpl_DepthStencil::InitDepthStencil()
    {
      if(_CL_NOT_(TextureImpl::InitTexture(FALSE, NULL, 0))) {
        return FALSE;
      }

      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      // Create the depth stencil view
      D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
      InlSetZeroT(descDSV);
      descDSV.Format = GrapXToDX11::FormatFrom(m_Format);
      descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      //descDSV.Texture2D.MipSlice = 0;
      HRESULT hval = pd3dDevice->CreateDepthStencilView(m_pD3D11Texture, &descDSV, &m_pD3D11DepthStencilView);
      return SUCCEEDED(hval);
    }

    //////////////////////////////////////////////////////////////////////////

    textureImpl_GPUReadBack::textureImpl_GPUReadBack(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Read)
    {
    }

    GXBOOL textureImpl_GPUReadBack::InitReadBackTexture()
    {
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_TEXTURE2D_DESC TexDesc;
      D3D11_SUBRESOURCE_DATA TexInitData;
      InlSetZeroT(TexDesc);
      InlSetZeroT(TexInitData);

      TexDesc.Width = m_nWidth;
      TexDesc.Height = m_nHeight;
      TexDesc.MipLevels = m_nMipLevels;
      TexDesc.ArraySize = 1;
      TexDesc.Format = GrapXToDX11::FormatFrom(m_Format);
      TexDesc.SampleDesc.Count = 1;
      TexDesc.SampleDesc.Quality = 0;
      TexDesc.BindFlags = 0;
      TexDesc.Usage = D3D11_USAGE_STAGING;
      TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;


      HRESULT hval = pd3dDevice->CreateTexture2D(&TexDesc, NULL, &m_pD3D11Texture);

      return SUCCEEDED(hval);
    }


    GXBOOL textureImpl_GPUReadBack::Map(MAPPED* pMappedRect, GXResMap eResMap)
    {
      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
      if(FAILED(pD3D11Context->Map(m_pD3D11Texture, 0, D3D11_MAP_READ, 0, &m_sMappedResource)))
      {
        return FALSE;
      }

      pMappedRect->pBits = m_sMappedResource.pData;
      pMappedRect->Pitch = m_sMappedResource.RowPitch;
      return TRUE;
    }

    GXBOOL textureImpl_GPUReadBack::Unmap()
    {
      if(m_sMappedResource.pData)
      {
        ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
        pD3D11Context->Unmap(m_pD3D11Texture, 0);
        return TRUE;
      }
      return FALSE;
    }

  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)