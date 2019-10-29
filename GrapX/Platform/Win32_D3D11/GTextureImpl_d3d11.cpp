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
//#include <FreeImage.h>
//#include "clImage.h"

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

    template<class _Interface>
    GXHRESULT TextureBaseImplT<_Interface>::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    template<class _Interface>
    GXHRESULT TextureBaseImplT<_Interface>::Release()
    {
      clstd::ScopedLocker sl(m_pGraphics->GetLocker());
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0)
      {
        //OnDeviceEvent(DE_LostDevice);
        INVOKE_LOST_DEVICE;
        if((m_pD3D11Texture && m_pD3D11ShaderView) &&
          (m_eResType == ResourceType::Texture2D || m_eResType == ResourceType::Texture3D || m_eResType == ResourceType::TextureCube)) {
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
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    TextureImpl::TextureImpl(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight, GXUINT nMipLevels, GXResUsage eResUsage)
      : TextureBaseImplT<Texture>(static_cast<GraphicsImpl*>(pGraphics), eFormat, nMipLevels, eResUsage)
      , m_pTextureData    (NULL)
      //, m_nMipLevels      (nMipLevels)
      //, m_Format          (eFormat)
      //, m_eResUsage       (eResUsage)
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
        if(_CL_NOT_(IntD3D11CreateResource(bRenderTarget, m_nWidth, m_nHeight, pInitData, nPitch))) {
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

    template<class _Interface>
    GXBOOL TextureBaseImplT<_Interface>::IntD3D11CreateResource(GXBOOL bRenderTarget, GXUINT nWidth, GXUINT nHeight, GXLPCVOID pInitData, GXUINT nPitch)
    {
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_TEXTURE2D_DESC TexDesc;
      InlSetZeroT(TexDesc);

      TexDesc.Width     = nWidth;
      TexDesc.Height    = nHeight;
      TexDesc.MipLevels = m_nMipLevels;
      TexDesc.Format    = GrapXToDX11::FormatFrom(m_Format);
      TexDesc.SampleDesc.Count = 1;
      TexDesc.SampleDesc.Quality = 0;

      if(m_eResType == ResourceType::TextureCube || m_eResType == ResourceType::CubeRenderTexture)
      {
        TexDesc.ArraySize = 6;
        TexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        TexDesc.Width = TexDesc.Height = clMin(nWidth, nHeight);
      }
      else
      {
        TexDesc.ArraySize = 1;
      }

      if(m_Format == Format_D32 || m_Format == Format_D16 ||
        m_Format == Format_D24S8 || m_Format == Format_D24X8)
      {
        TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.CPUAccessFlags = 0;
      }
      else if(bRenderTarget) {
        TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.CPUAccessFlags = 0;
      }
      else {
        TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        GrapXToDX11::TextureDescFromResUsage(&TexDesc, m_eResUsage, m_nMipLevels, (pInitData != NULL));
      }


      ASSERT(TexDesc.Width < 16384 && TexDesc.Height < 16384);

      HRESULT hval;
      D3D11_SUBRESOURCE_DATA TexInitData;

      if(pInitData == NULL || m_nMipLevels == 0)
      {
        hval = pd3dDevice->CreateTexture2D(&TexDesc, NULL, &m_pD3D11Texture);
      }
      else  if(m_nMipLevels == 1)
      {
        if(m_eResType == ResourceType::TextureCube)
        {
          D3D11_SUBRESOURCE_DATA aTexInitData[6] = { {0, nPitch, 0}, {0, nPitch, 0}, {0, nPitch, 0}, {0, nPitch, 0}, {0, nPitch, 0}, {0, nPitch, 0} };
          GXUINT nFacePitch = 0;
          if (nWidth == nHeight * 6)
          {
            nFacePitch = GetBytesOfGraphicsFormat(m_Format) * nHeight;
          }
          else if (nWidth * 6 == nHeight)
          {
            nFacePitch = nPitch * nWidth; // nWidth 是 face size
          }
          else {
            CLBREAK;
          }
          aTexInitData[0].pSysMem = pInitData;
          aTexInitData[1].pSysMem = (LPCVOID)((INT_PTR)pInitData + nFacePitch);
          aTexInitData[2].pSysMem = (LPCVOID)((INT_PTR)pInitData + nFacePitch * 2);
          aTexInitData[3].pSysMem = (LPCVOID)((INT_PTR)pInitData + nFacePitch * 3);
          aTexInitData[4].pSysMem = (LPCVOID)((INT_PTR)pInitData + nFacePitch * 4);
          aTexInitData[5].pSysMem = (LPCVOID)((INT_PTR)pInitData + nFacePitch * 5);

          hval = pd3dDevice->CreateTexture2D(&TexDesc, aTexInitData, &m_pD3D11Texture);
        }
        else
        {
          InlSetZeroT(TexInitData);
          TexInitData.pSysMem = pInitData;
          TexInitData.SysMemPitch = nPitch;
          hval = pd3dDevice->CreateTexture2D(&TexDesc, &TexInitData, &m_pD3D11Texture);
        }
      }
      else
      {
        if(m_eResType == ResourceType::TextureCube)
        {
          return FALSE; // 没实现Cube 的Mipmaps
        }

        clvector<D3D11_SUBRESOURCE_DATA> aInitData;
        aInitData.reserve(m_nMipLevels);
        InlSetZeroT(TexInitData);
        
        TexInitData.pSysMem = pInitData;
        TexInitData.SysMemPitch = nPitch;
        aInitData.push_back(TexInitData);
        GXUINT nNumOfDataLines = nHeight;

        if (m_Format == Format_BC2 || m_Format == Format_BC3)
        {
          nNumOfDataLines /= 4;
        }

        for(GXUINT i = 1; i < m_nMipLevels; i++)
        {
          TexInitData.pSysMem = reinterpret_cast<void*>(reinterpret_cast<size_t>(TexInitData.pSysMem) + TexInitData.SysMemPitch * nNumOfDataLines);
          nNumOfDataLines = clMax((GXUINT)1, nNumOfDataLines >> 1);
          TexInitData.SysMemPitch >>= 1;
          aInitData.push_back(TexInitData);
        }
        hval = pd3dDevice->CreateTexture2D(&TexDesc, &aInitData.front(), &m_pD3D11Texture);
      }


      //ASSERT(SUCCEEDED(hval)); // 临时
      if(FAILED(hval)) {
        return FALSE;
      }


      if(TEST_FLAG(TexDesc.BindFlags, D3D11_BIND_SHADER_RESOURCE)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC sResourceViewDesc = {TexDesc.Format, D3D11_SRV_DIMENSION_TEXTURECUBE};
        sResourceViewDesc.TextureCube.MipLevels = TexDesc.MipLevels;
        sResourceViewDesc.TextureCube.MostDetailedMip = 0;

        hval = pd3dDevice->CreateShaderResourceView(m_pD3D11Texture, (m_eResType == ResourceType::TextureCube) ? &sResourceViewDesc : NULL, &m_pD3D11ShaderView);
        if(m_nMipLevels == 0 && pInitData) {
          ASSERT(nPitch);
          ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
          pD3D11Context->UpdateSubresource(m_pD3D11Texture, 0, NULL, pInitData, nPitch, 0);
          pD3D11Context->GenerateMips(m_pD3D11ShaderView);
        }
        return SUCCEEDED(hval);
      }
      return TRUE;
    }

    GXUINT TextureImpl::GetMinPitchSize() const
    {
      return GetBytesOfGraphicsFormat(m_Format) * m_nWidth;
    }

    GXBOOL TextureImpl::IntSaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip)
    {
      //ASSERT(m_eResUsage == GXResUsage::SystemMem || m_eResUsage == GXResUsage::Read);
      GXBOOL bval = TRUE;
      Texture::MAPPED mapped;
      if (Map(&mapped, GXResMap::Read))
      {
        bval = EncodeToMemory(pBuffer, mapped.pBits, m_Format, m_nWidth, m_nHeight, mapped.Pitch, pImageFormat, bVertFlip);
        Unmap();
      }
      else
      {
        CLOG_ERROR("%s(%d): Unable map texture", __FILE__, __LINE__);
        CLBREAK;
      }
      return bval;
    }

    //////////////////////////////////////////////////////////////////////////
    GXBOOL TextureImpl::Clear(GXColor color)
    {
      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();

      if(m_eResUsage == GXResUsage::Write || m_eResUsage == GXResUsage::ReadWrite ||
        m_eResUsage == GXResUsage::SystemMem || m_eResUsage == GXResUsage::Default)
      {
        if(m_eResUsage == GXResUsage::Default)
        {
          D3D11_TEXTURE2D_DESC desc;
          m_pD3D11Texture->GetDesc(&desc);
          if(desc.Usage == D3D11_USAGE_IMMUTABLE) {
            return FALSE;
          }
        }

        int nWidth;
        int nHeight;
        nWidth = m_nWidth;
        nHeight = m_nHeight;
        GXUINT cbFormat = GetBytesOfGraphicsFormat(m_Format);
        GXLPVOID pData = new GXBYTE[cbFormat * m_nWidth * m_nHeight];
        // FIXME: 颜色通道顺序不对
        if(cbFormat == 4)
        {
          GXDWORD* pColor = (GXDWORD*)pData;
          GXCOLOR dwColor = color.ARGB();
          for(int i = nWidth * nHeight; i > 0; i--)
          {
            *pColor++ = dwColor;
          }
        }
        else if(cbFormat == 16)
        {
          GXColor* pColor = (GXColor*)pData;
          for(int i = nWidth * nHeight; i > 0; i--)
          {
            *pColor++ = color;
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
      ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
      D3D11_BOX box = {0, 0, 0, m_nWidth, m_nHeight, 1};
        // {UINT(lprcSource->left), UINT(lprcSource->top), 0, UINT(lprcSource->right), UINT(lprcSource->bottom), 1};
      UINT x = 0, y = 0;
      if(lpptDestination) {
        x = lpptDestination->x;
        y = lpptDestination->y;
      }

      if(lprcSource) {
        box.left = lprcSource->left;
        box.top = lprcSource->top;
        box.right = lprcSource->right;
        box.bottom = lprcSource->bottom;
      }

      if(pSrc->m_eResType == ResourceType::RenderTexture)
      {
        ASSERT(dynamic_cast<RenderTarget_TextureImpl*>(pSrc) != NULL);

        RenderTarget_TextureImpl* pRT = static_cast<RenderTarget_TextureImpl*>(pSrc);
        pD3D11Context->CopySubresourceRegion(
          m_pD3D11Texture, 0, x, y, 0,
          pRT->m_pD3D11Texture, pRT->m_nSlice, &box);
      }
      else
      {
        pD3D11Context->CopySubresourceRegion(
          m_pD3D11Texture, 0, x, y, 0,
          static_cast<TextureImpl*>(pSrc)->m_pD3D11Texture, 0, &box);
      }
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

    GXBOOL TextureImpl::SaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR szDestFormat, GXBOOL bVertFlip)
    {
      if (m_eResUsage == GXResUsage::SystemMem || m_eResUsage == GXResUsage::Read || m_eResUsage == GXResUsage::ReadWrite) {
        return IntSaveToMemory(pBuffer, szDestFormat, bVertFlip);
      }
      CLBREAK;
      return FALSE;
    }

    GXBOOL TextureImpl::SaveToFile(GXLPCWSTR szFileName, GXLPCSTR szDestFormat, GXBOOL bVertFlip)
    {
      clstd::MemBuffer buffer;
      if (SaveToMemory(&buffer, szDestFormat, bVertFlip))
      {
        clstd::File file;
        if (file.CreateAlways(szFileName))
        {
          file.Write(buffer);
          return TRUE;
        }
      }
      return FALSE;
    }

    GXSIZE* TextureImpl::GetDimension(GXSIZE* pDimension)
    {
      if(pDimension != NULL) {
        pDimension->cx = m_nWidth;
        pDimension->cy = m_nHeight;
      }
      return pDimension;
    }

    template<class _Interface>
    GXResUsage TextureBaseImplT<_Interface>::GetUsage()
    {
      return m_eResUsage;
    }

    template<class _Interface>
    GXFormat TextureBaseImplT<_Interface>::GetFormat()
    {
      return m_Format;
    }

    GXVOID TextureImpl::GenerateMipMaps()
    {
      m_pGraphics->D3DGetDeviceContext()->GenerateMips(m_pD3D11ShaderView);
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

    void TextureImpl::GetDesc(TEXTURE_DESC* pDesc)
    {
      D3D11_TEXTURE2D_DESC desc;
      m_pD3D11Texture->GetDesc(&desc);
      pDesc->Width      = m_nWidth;
      pDesc->Height     = m_nHeight;
      pDesc->MipLevels  = desc.MipLevels;
      pDesc->Format     = m_Format;
      pDesc->Usage      = m_eResUsage;
    }

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    RenderTarget_TextureImpl::RenderTarget_TextureImpl(Graphics* pGraphics/*, ResourceType dwResType*/, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Default)
    {
      //ASSERT(dwResType == ResourceType::RenderTexture || dwResType == ResourceType::CubeRenderTexture);
      m_eResType = ResourceType::RenderTexture;
    }

    RenderTarget_TextureImpl::~RenderTarget_TextureImpl()
    {
      SAFE_RELEASE(m_pD3D11RenderTargetView);
    }

    GXBOOL RenderTarget_TextureImpl::InitRenderTexture(ID3D11Texture2D* pD3D11Texture)
    {
      if(pD3D11Texture)
      {
        ASSERT(m_Format == Format_Unknown && m_nWidth == 0 && m_nHeight == 0);
        m_pD3D11Texture = pD3D11Texture;
        m_pD3D11Texture->AddRef();

        D3D11_TEXTURE2D_DESC desc;
        m_pD3D11Texture->GetDesc(&desc);
        //static_cast<GXFormat>(m_Format) = DX11ToGrapX();
        static_cast<GXUINT>(m_nWidth) = desc.Width;
        static_cast<GXUINT>(m_nHeight) = desc.Height;
      }
      else
      {
        if(_CL_NOT_(TextureImpl::InitTexture(TRUE, NULL, 0))) {
          return FALSE;
        }
      }

      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_RENDER_TARGET_VIEW_DESC TarDesc = { GrapXToDX11::FormatFrom(m_Format) };
      TarDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      HRESULT hval = pd3dDevice->CreateRenderTargetView(m_pD3D11Texture, &TarDesc, &m_pD3D11RenderTargetView);
      return SUCCEEDED(hval);
    }

    GXBOOL RenderTarget_TextureImpl::InitRenderTexture(ID3D11Texture2D* pD3D11Texture, int nFaceIndex)
    {
      m_pD3D11Texture = pD3D11Texture;
      pD3D11Texture->AddRef();
      D3D11_RENDER_TARGET_VIEW_DESC TarDesc = { GrapXToDX11::FormatFrom(m_Format) };
      //ASSERT(m_dwResType == RESTYPE_CUBERENDERTARGET);
      TarDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
      TarDesc.Texture2DArray.FirstArraySlice = m_nSlice = nFaceIndex;
      TarDesc.Texture2DArray.ArraySize = 1;
      
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
      HRESULT hval = pd3dDevice->CreateRenderTargetView(m_pD3D11Texture, &TarDesc, &m_pD3D11RenderTargetView);
      return SUCCEEDED(hval);
    }

    ID3D11RenderTargetView* RenderTarget_TextureImpl::D3DGetRenderTargetView() const
    {
      return m_pD3D11RenderTargetView;
    }

    GXBOOL RenderTarget_TextureImpl::CopyRect(Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource)
    {
      CLBREAK;
      return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
#pragma region Cube Render Target Texture

    CubeRenderTarget_TextureCubeImpl::CubeRenderTarget_TextureCubeImpl(Graphics* pGraphics, GXFormat eFormat, GXUINT nSize)
      : TextureCubeImpl(pGraphics, eFormat, nSize, 1, GXResUsage::Default)
    {
      //ASSERT(dwResType == ResourceType::RenderTexture || dwResType == ResourceType::CubeRenderTexture);
      m_eResType = ResourceType::CubeRenderTexture;
    }

    CubeRenderTarget_TextureCubeImpl::~CubeRenderTarget_TextureCubeImpl()
    {
      SAFE_RELEASE(m_pD3D11RenderTargetView);
    }

    GXBOOL CubeRenderTarget_TextureCubeImpl::InitRenderTexture()
    {
      if(_CL_NOT_(TextureCubeImpl::InitTexture(TRUE, m_nSize, m_nSize, NULL, 0))) {
        return FALSE;
      }

      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_RENDER_TARGET_VIEW_DESC TarDesc = { GrapXToDX11::FormatFrom(m_Format) };
      TarDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
      TarDesc.Texture2DArray.ArraySize = 6;

      HRESULT hval = pd3dDevice->CreateRenderTargetView(m_pD3D11Texture, &TarDesc, &m_pD3D11RenderTargetView);
      return SUCCEEDED(hval);
    }

    ID3D11RenderTargetView* CubeRenderTarget_TextureCubeImpl::D3DGetRenderTargetView() const
    {
      return m_pD3D11RenderTargetView;
    }

#pragma endregion

    //////////////////////////////////////////////////////////////////////////

    TextureImpl_DepthStencil::TextureImpl_DepthStencil(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Default)
      , m_pD3D11DepthStencilView(NULL)
    {
      m_eResType = ResourceType::DepthStenciltexture;
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

    TextureImpl_GPUReadBack::TextureImpl_GPUReadBack(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight)
      : TextureImpl(pGraphics, eFormat, nWidth, nHeight, 1, GXResUsage::Read)
    {
    }

    GXBOOL TextureImpl_GPUReadBack::InitReadBackTexture()
    {
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();

      D3D11_TEXTURE2D_DESC TexDesc;
      //D3D11_SUBRESOURCE_DATA TexInitData;
      InlSetZeroT(TexDesc);
      //InlSetZeroT(TexInitData);

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


    GXBOOL TextureImpl_GPUReadBack::Map(MAPPED* pMappedRect, GXResMap eResMap)
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

    GXBOOL TextureImpl_GPUReadBack::Unmap()
    {
      if(m_sMappedResource.pData)
      {
        ID3D11DeviceContext* pD3D11Context = m_pGraphics->D3DGetDeviceContext();
        pD3D11Context->Unmap(m_pD3D11Texture, 0);
        return TRUE;
      }
      return FALSE;
    }

    GXUINT CalculateNumOfMipLevels(GXUINT nWidth, GXUINT nHeight)
    {
      GXUINT nLevels = 0;
      while(nWidth > 0 && nHeight > 0)
      {
        nWidth >>= 1;
        nHeight >>= 1;
        nLevels++;
      }
      return nLevels;
    }

    //////////////////////////////////////////////////////////////////////////

    GXHRESULT TextureCubeImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      return 0;
    }

    GXUINT TextureCubeImpl::GetSize() const
    {
      return m_nSize;
    }


    TextureCubeImpl::TextureCubeImpl(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nSize, GXUINT nMipLevels, GXResUsage eResUsage)
      : TextureBaseImplT<TextureCube>(static_cast<GraphicsImpl*>(pGraphics), eFormat, nMipLevels, eResUsage)
      , m_nSize(nSize)
    {
      InlSetZeroT(m_sMappedResource);
    }

    TextureCubeImpl::~TextureCubeImpl()
    {
      SAFE_RELEASE(m_pD3D11Texture);
      SAFE_RELEASE(m_pD3D11ShaderView);
      SAFE_DELETE_ARRAY(m_pTextureData);
    }

    GXBOOL TextureCubeImpl::InitTexture(GXBOOL bRenderTarget, GXUINT nWidth, GXUINT nHeight, GXLPCVOID pInitData, GXUINT nPitch)
    {
      const GXUINT nMinPitch = GetMinPitchSize(nWidth);
      nPitch = clMax(nPitch, nMinPitch);

      if(m_eResUsage != GXResUsage::SystemMem)
      {
        if(_CL_NOT_(IntD3D11CreateResource(bRenderTarget, nWidth, nHeight, pInitData, nPitch))) {
          return FALSE;
        }
      }

      if(m_eResUsage == GXResUsage::Read || m_eResUsage == GXResUsage::ReadWrite || m_eResUsage == GXResUsage::SystemMem)
      {
        CLBREAK; // 没实现
        //const GXUINT nSize = nMinPitch * m_nSize;
        //m_pTextureData = new GXBYTE[nSize];
        //GXLPBYTE pDest = m_pTextureData;

        //if(pInitData)
        //{
        //  for(GXUINT y = 0; y < m_nSize; y++, pDest += nMinPitch) {
        //    memcpy(pDest, reinterpret_cast<GXLPVOID>((size_t)pInitData + nPitch * y), nMinPitch);
        //  }
        //}
      }

      return TRUE;
    }

    GXUINT TextureCubeImpl::GetMinPitchSize(GXUINT nWidth) const
    {
      return GetBytesOfGraphicsFormat(m_Format) * nWidth;
    }

    //GXBOOL TextureCubeImpl::IntSaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip)
    //{

    //}

  } // namespace D3D11
} // namespace GrapX
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)