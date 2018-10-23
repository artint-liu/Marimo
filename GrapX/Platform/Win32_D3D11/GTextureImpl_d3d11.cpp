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

// 私有头文件
#include "Platform/Win32_D3D11/GTextureImpl_D3D11.h"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_D3D11.h"
#include <clPathFile.h>
#ifdef ENABLE_GRAPHICS_API_DX11
#include <FreeImage.h>

namespace D3D11
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  GXHRESULT GTextureImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      SAFE_RELEASE(m_pTexRV);
      SAFE_RELEASE(m_pTexture);
      SAFE_RELEASE(m_pHelpTexture);
      break;
    case RC_ResetDevice:
      break;
    }
    return GX_OK;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GTextureImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GTextureImpl::Release()
  {
    clstd::ScopedLocker sl(m_pGraphics->GetLocker());
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      //OnDeviceEvent(DE_LostDevice);
      INVOKE_LOST_DEVICE;
      if(m_emType != CreationFailed) {
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

  GXBOOL GTextureImpl::SaveToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat)
  {
    if(szFileName == NULL)
      return FALSE;
    return FALSE;
    //D3DXIMAGE_FILEFORMAT d3diff = D3DXIFF_DDS;
    //if(IS_PTR(pszDestFormat))
    //{
    //  if(GXSTRCMPI(pszDestFormat, L"BMP") == 0)
    //    d3diff = D3DXIFF_BMP;
    //  else if(GXSTRCMPI(pszDestFormat, L"JPG") == 0)
    //    d3diff = D3DXIFF_JPG;
    //  else if(GXSTRCMPI(pszDestFormat, L"TGA") == 0)
    //    d3diff = D3DXIFF_TGA;
    //  else if(GXSTRCMPI(pszDestFormat, L"PNG") == 0)
    //    d3diff = D3DXIFF_PNG;
    //  else if(GXSTRCMPI(pszDestFormat, L"DDS") == 0)
    //    d3diff = D3DXIFF_DDS;
    //  else if(GXSTRCMPI(pszDestFormat, L"PPM") == 0)
    //    d3diff = D3DXIFF_PPM;
    //  else if(GXSTRCMPI(pszDestFormat, L"DIB") == 0)
    //    d3diff = D3DXIFF_DIB;
    //  else if(GXSTRCMPI(pszDestFormat, L"HDR") == 0)
    //    d3diff = D3DXIFF_HDR;
    //  else if(GXSTRCMPI(pszDestFormat, L"PFM") == 0)
    //    d3diff = D3DXIFF_PFM;

    //}
    //else if((GXUINT_PTR)pszDestFormat <= 8)
    //  d3diff = (D3DXIMAGE_FILEFORMAT)(GXUINT_PTR)pszDestFormat;
    //return GXSUCCEEDED(D3DXSaveTextureToFileW(pszFileName, d3diff, m_pTexture, NULL));
  }

  GTextureImpl::GTextureImpl(GXGraphics* pGraphics)
    : GTexBaseImplT   ((GXGraphicsImpl*)pGraphics)
    //, m_pGraphics     ((GXGraphicsImpl*)pGraphics)
    //, m_pTexture      (NULL)
    //, m_pTexRV        (NULL)
    //, m_pSurface      (NULL)
    , m_nWidthRatio   (0)
    , m_nHeightRatio  (0)
    , m_Format        (GXFMT_UNKNOWN)
    , m_nMipLevels    (0)
    , m_dwResUsage    (0)
    , m_emType        (Invalid)
    , m_pHelpTexture  (NULL)
  {

  }

  void GTextureImpl::CalcTextureActualDimension()
  {
    GXGRAPHICSDEVICE_DESC GrapDeviceDesc;
    m_pGraphics->GetDesc(&GrapDeviceDesc);

    GXDWORD dwFlags = TEST_FLAG(m_pGraphics->GetCaps(GXGRAPCAPS_TEXTURE), GXTEXTURECAPS_NONPOW2)
      ? NULL : TEXTURERATIO_POW2;

    //m_nWidth = (m_nWidthRatio < 0)
    //  ? TextureRatioToDimension((GXINT)m_nWidthRatio, GrapDeviceDesc.BackBufferWidth, dwFlags)
    //  : ((GXUINT)(GXINT)m_nWidthRatio);

    //m_nHeight = (m_nHeightRatio < 0)
    //  ? TextureRatioToDimension((GXINT)m_nHeightRatio, GrapDeviceDesc.BackBufferHeight, dwFlags)
    //  : ((GXUINT)(GXINT)m_nHeightRatio);

    m_nWidth  = TextureRatioToDimension((GXINT)m_nWidthRatio, GrapDeviceDesc.BackBufferWidth, dwFlags);
    m_nHeight = TextureRatioToDimension((GXINT)m_nHeightRatio, GrapDeviceDesc.BackBufferHeight, dwFlags);
  }


  GXBOOL GXDLLAPI GXSaveTextureToFileW(GXLPCWSTR pszFileName, GXLPCWSTR pszDestFormat, GTexture* pTexture)
  {
    if(pszFileName == NULL || pTexture == NULL)
      return FALSE;
    
    //D3DXIMAGE_FILEFORMAT d3diff = D3DXIFF_DDS;
    //if(IS_PTR(pszDestFormat))
    //{
    //  if(GXSTRCMPI(pszDestFormat, L"BMP") == 0)
    //    d3diff = D3DXIFF_BMP;
    //  else if(GXSTRCMPI(pszDestFormat, L"JPG") == 0)
    //    d3diff = D3DXIFF_JPG;
    //  else if(GXSTRCMPI(pszDestFormat, L"TGA") == 0)
    //    d3diff = D3DXIFF_TGA;
    //  else if(GXSTRCMPI(pszDestFormat, L"PNG") == 0)
    //    d3diff = D3DXIFF_PNG;
    //  else if(GXSTRCMPI(pszDestFormat, L"DDS") == 0)
    //    d3diff = D3DXIFF_DDS;
    //  else if(GXSTRCMPI(pszDestFormat, L"PPM") == 0)
    //    d3diff = D3DXIFF_PPM;
    //  else if(GXSTRCMPI(pszDestFormat, L"DIB") == 0)
    //    d3diff = D3DXIFF_DIB;
    //  else if(GXSTRCMPI(pszDestFormat, L"HDR") == 0)
    //    d3diff = D3DXIFF_HDR;
    //  else if(GXSTRCMPI(pszDestFormat, L"PFM") == 0)
    //    d3diff = D3DXIFF_PFM;

    //}
    //else if((GXUINT_PTR)pszDestFormat <= 8)
    //  d3diff = (D3DXIMAGE_FILEFORMAT)(GXUINT_PTR)pszDestFormat;
    //return GXSUCCEEDED(D3DXSaveTextureToFileW(pszFileName, d3diff, ((GTextureImpl*)pTexture)->D3DTexture(), NULL));
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  GXBOOL GTextureImpl::Clear(const GXLPRECT lpRect, GXCOLOR dwColor)
  {
    ID3D11DeviceContext* pContext = m_pGraphics->D3DGetDeviceContext();
    if(TEST_FLAG(m_dwResUsage, GXRU_TEST_MIGHTBE) || m_dwResUsage == GXRU_DEFAULT)
    {
      int nWidth;
      int nHeight;
      if(lpRect == NULL) {
        nWidth = m_nWidth;
        nHeight = m_nHeight;
      }
      else {
        nWidth = lpRect->right - lpRect->left;
        nHeight = lpRect->bottom - lpRect->top;
      }
      GXUINT cbFormat = GetBytesOfGraphicsFormat(m_Format);
      GXLPVOID pData = new GXBYTE[cbFormat * nWidth * nHeight];
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
         
      //ID3D11Texture2D* pTempTexture = IntCreateHelpTexture(nWidth, nHeight, pData);
      if(lpRect == NULL) {
        pContext->UpdateSubresource(m_pTexture, 0, NULL, pData, nWidth * cbFormat, NULL);
      }
      else {
        ASSERT(0); // FIXME: 不支持区域
      }

      
      //SAFE_RELEASE(pTempTexture);
      SAFE_DELETE_ARRAY(pData);
      return TRUE;
    }
    //GXHRESULT    hr;
    //GXRECT    rect;
    //if(lpRect == NULL)
    //{
    //  rect.left   = 0;
    //  rect.top    = 0;
    //  GetDimension((GXUINT*)&rect.right, (GXUINT*)&rect.bottom);
    //}
    //else
    //  rect = *lpRect;

    //DWORD Usage;
    //D3DPOOL Pool;
    //ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);
    //if(m_dwResUsage & D3DUSAGE_RENDERTARGET)
    //{
    //  hr = m_pGraphics->D3DGetDevice()->ColorFill(m_pSurface, (const RECT*)lpRect, (D3DCOLOR)dwColor);
    //  ASSERT(GXSUCCEEDED(hr));
    //  return GXSUCCEEDED(hr);
    //}
    //else if(Pool == D3DPOOL_SYSTEMMEM || Pool == D3DPOOL_MANAGED)
    //{
    //  D3DLOCKED_RECT  d3dlr;
    //  int x, y;
    //  hr = m_pSurface->LockRect(&d3dlr, (const RECT*)&rect, NULL);

    //  if(GXSUCCEEDED(hr))
    //  {
    //    switch(m_Format)
    //    {
    //    case D3DFMT_A8R8G8B8:
    //    case D3DFMT_X8R8G8B8:
    //      {
    //        GXDWORD* pBits = (GXDWORD*)d3dlr.pBits;
    //        for(y = 0; y < rect.bottom - rect.top; y++)
    //        {
    //          for(x = 0; x < rect.right - rect.left; x++)
    //          {
    //            pBits[x] = dwColor;
    //          }
    //          pBits += d3dlr.Pitch / sizeof(GXDWORD);
    //        }
    //      }
    //      break;
    //    default:  // 不支持的格式
    //      ASSERT(0);
    //    }

    //    m_pSurface->UnlockRect();
    //    return TRUE;
    //  }
    //  ASSERT(0);  // 不支持锁定
    //  return FALSE;
    //}
    //else 
    //{
    //  GTexture* pHelperTex;
    //  GXHRESULT hval = m_pGraphics->CreateTexture(&pHelperTex, rect.right, rect.bottom, 1, m_Format, GXRU_SYSTEMMEM);
    //  ASSERT(GXSUCCEEDED(hval));
    //  if(pHelperTex->Clear(lpRect, dwColor) == FALSE)
    //  {
    //    SAFE_RELEASE(pHelperTex);
    //    ASSERT(0);
    //    return FALSE;
    //  }
    //  if(CopyRect(pHelperTex, &rect) == FALSE)
    //  {
    //    SAFE_RELEASE(pHelperTex);
    //    ASSERT(0);
    //    return FALSE;
    //  }
    //  SAFE_RELEASE(pHelperTex);
    //  return TRUE;
    //}
    return FALSE;
    //HDC hdc = NULL;
    //hr = m_pSurface->GetDC(&hdc);
    //if(FAILED(hr) || hdc == NULL)
    //{
    //  ASSERT(0);
    //  return FALSE;
    //}
    //HBRUSH hBrush;
    //GXRECT rect;
    //hBrush = CreateSolidBrush((GXCOLORREF)dwColor);  // TODO: 这个颜色是不是反的？
    //
    //if(lpRect == NULL)
    //{
    //  rect.left   = 0;
    //  rect.top    = 0;
    //  rect.right  = m_nWidth;
    //  rect.bottom = m_nHeight;
    //}
    //else
    //  rect = *lpRect;
    //FillRect(hdc, (const RECT*)&rect, hBrush);

    //DeleteObject(hBrush);
    //m_pSurface->ReleaseDC(hdc);
    //return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  GTextureImpl::CREATETYPE  GTextureImpl::GetCreateType()
  {
    ASSERT(m_emType > Invalid && m_emType < LastType);
    return m_emType;
  }
  //GXBOOL GTextureImpl::CreateHelperSur()
  //{
  //  return FALSE;
  //}
  //GXBOOL GTextureImpl::FlushToHelperSur()
  //{
  //  return FALSE;
  //}
  //GXBOOL GTextureImpl::FlushToRTSurface()
  //{
  //  return FALSE;
  //}
  //////////////////////////////////////////////////////////////////////////
  GXBOOL GTextureImpl::CopyRect(GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination)
  {
    //RECT rect = {0,0};
    //if(lpRect == NULL)
    //{
    //  pSrc->GetDimension((GXUINT*)&rect.right, (GXUINT*)&rect.bottom);
    //}
    //else
    //{
    //  rect = *(RECT*)lpRect;
    //}

    //if(GXFAILED(D3DXLoadSurfaceFromSurface(
    //  m_pSurface, NULL, (const RECT*)&rect, ((GTextureImpl*)pSrc)->D3DSurface(),
    //  NULL, (const RECT*)&rect, D3DX_FILTER_POINT, NULL)))
    //{
    //  ASSERT(false);
    //  return FALSE;
    //}

    return TRUE;
  }

  GXBOOL GTextureImpl::StretchRect(GTexture* pSrc, const GXLPCRECT lpDestRect, const GXLPCRECT lpSrcRect, GXTextureFilterType eFilter)
  {
    HRESULT hval = GX_OK;
//    m_pGraphics->Enter();
//    GTextureImpl* pSrcImp = (GTextureImpl*)pSrc;
//#ifdef _DEBUG
//    GXUINT cxSrc, cySrc;
//    GXUINT cxDst, cyDst;
//    if(lpSrcRect != NULL)
//    {
//      pSrc->GetDimension(&cxSrc, &cySrc);
//      ASSERT(lpSrcRect->left >= 0 && lpSrcRect->top >= 0 &&
//        lpSrcRect->right <= (GXINT)cxSrc && lpSrcRect->bottom <= (GXINT)cySrc);
//    }
//    if(lpDestRect)
//    {
//      GetDimension(&cxDst, &cyDst);
//      ASSERT(lpDestRect->left >= 0 && lpDestRect->top >= 0 &&
//        lpDestRect->right <= (GXINT)cxDst && lpDestRect->bottom <= (GXINT)cyDst);
//    }
//#endif // _DEBUG
//    // StretchRect 不支持 Texture 到 Texture 对拷.
//    if( TEST_FLAG(m_dwResUsage, GXRU_TEX_RENDERTARGET) == 0 && 
//      TEST_FLAG(pSrcImp->m_dwResUsage, GXRU_TEX_RENDERTARGET) == 0 )
//    {
//      hval = D3DXLoadSurfaceFromSurface(m_pSurface, NULL, 
//        (RECT*)lpDestRect, pSrcImp->m_pSurface, NULL, (RECT*)lpSrcRect, eFilter, 0);
//      ASSERT(GXSUCCEEDED(hval));
//    }
//    else
//    {
//      hval = m_pGraphics->D3DGetDevice()->StretchRect(
//        ((GTextureImpl*)pSrc)->m_pSurface, (const RECT *)lpSrcRect, 
//        m_pSurface, (const RECT *)lpDestRect, (D3DTEXTUREFILTERTYPE)eFilter);
//      ASSERT(GXSUCCEEDED(hval));
//    }
//    m_pGraphics->Leave();
    return GXSUCCEEDED(hval);
  }
  ID3D11Texture2D* GTextureImpl::IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData)
  {
    ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
    ID3D11DeviceContext* pContext = m_pGraphics->D3DGetDeviceContext();

    ID3D11Texture2D* pHelpTexture = NULL;
    HRESULT hval = S_OK;

    ASSERT(nWidth > 0 && nWidth <= 16384 && nHeight > 0 && nHeight <= 16384);

    D3D11_TEXTURE2D_DESC TexDesc;
    InlSetZeroT(TexDesc);
    TexDesc.Width     = nWidth;
    TexDesc.Height    = nHeight;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format    = GrapXToDX11::FormatFrom(m_Format);
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage     = D3D11_USAGE_STAGING;
    TexDesc.BindFlags = 0;
    TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ;
    TexDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InlSetZeroT(InitData);
    InitData.pSysMem = pData;
    InitData.SysMemPitch = nWidth * GetBytesOfGraphicsFormat(m_Format);

    hval = pd3dDevice->CreateTexture2D( &TexDesc, pData == NULL ? NULL : &InitData, &pHelpTexture);

    if(FAILED(hval) || pHelpTexture == NULL)
    {
      CLOG_ERROR(__FUNCTION__" failed to create help texture.\n");
      return NULL;
    }

    // 下载资源
    pContext->CopyResource(pHelpTexture, m_pTexture);
    return pHelpTexture;
  }

  GXBOOL GTextureImpl::IntGetHelpTexture()
  {
    if(m_pHelpTexture == NULL) {
      m_pHelpTexture = IntCreateHelpTexture(m_nWidth, m_nHeight, NULL);
      if(m_pHelpTexture == NULL) {
        return FALSE;
      }
    }
    return TRUE;
  }

  GXBOOL GTextureImpl::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)// TODO: Flags 统一化, 现在没有GX方面的定义,这个值目前都和平台相关
  {
#ifdef _DEBUG
    if( ! (Flags == 0)) {
      CLOG_WARNING("Flags == 0 Failed.\n");
    }
#endif // #ifdef _DEBUG

    ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
    ID3D11DeviceContext* pContext = m_pGraphics->D3DGetDeviceContext();
    D3D11_MAPPED_SUBRESOURCE SubResource;
    HRESULT hval = S_OK;

    InlSetZeroT(SubResource);

    if(TEST_FLAG(m_dwResUsage, GXRU_TEST_FREQUENTLY))
    {     
      IntGetHelpTexture();
      hval = pContext->Map(m_pHelpTexture, 0, D3D11_MAP_WRITE, 0, &SubResource);

      //if(SUCCEEDED(hval)) {
      //  lpLockRect->pBits = SubResource.pData;
      //  lpLockRect->Pitch = SubResource.RowPitch;
      //  if(lpRect != NULL) {
      //    const GXUINT cbPixel = GetBytesOfGraphicsFormat(m_Format);
      //    const GXUINT nOffset = lpRect->top * SubResource.RowPitch + lpRect->left * cbPixel;

      //    lpLockRect->pBits = (GXBYTE*)lpLockRect->pBits + nOffset;
      //  }
      //  return TRUE;
      //}
    }
    else if(TEST_FLAG(m_dwResUsage, GXRU_SYSTEMMEM))
    {
      hval = pContext->Map(m_pTexture, 0, D3D11_MAP_WRITE, 0, &SubResource);
    }
    else {
      ASSERT(0); // FIXME: 没实现!
    }

    if(SUCCEEDED(hval)) {
      lpLockRect->pBits = SubResource.pData;
      lpLockRect->Pitch = SubResource.RowPitch;
      if(lpRect != NULL) {
        const GXUINT cbPixel = GetBytesOfGraphicsFormat(m_Format);
        const GXUINT nOffset = lpRect->top * SubResource.RowPitch + lpRect->left * cbPixel;

        lpLockRect->pBits = (GXBYTE*)lpLockRect->pBits + nOffset;
      }
      return TRUE;
    }
    return FALSE;
   
  }

  GXBOOL GTextureImpl::UnlockRect()
  {
    ID3D11DeviceContext* pContext = m_pGraphics->D3DGetDeviceContext();
    if(TEST_FLAG(m_dwResUsage, GXRU_TEST_FREQUENTLY))
    {
      if(m_pHelpTexture == NULL)
      {
        CLOG_ERROR(__FUNCTION__" failed to unlock texture.\n");
        return FALSE;
      }
      pContext->Unmap(m_pHelpTexture, 0);
      pContext->CopyResource(m_pTexture, m_pHelpTexture);
      return TRUE;
    }
    else if(TEST_FLAG(m_dwResUsage, GXRU_SYSTEMMEM))
    {
      pContext->Unmap(m_pTexture, 0);
      return TRUE;
    }

    //pContext->Unmap(m_pTexture, 0);
    //return GXSUCCEEDED(m_pSurface->UnlockRect());
    ASSERT(0); // FIXME: 没实现!
    return FALSE;
  }

  GXGraphics*  GTextureImpl::GetGraphicsUnsafe()
  {
    return m_pGraphics;
  }
  GXBOOL GTextureImpl::GetRatio(GXINT* pWidthRatio, GXINT* pHeightRatio)
  {
    if(pWidthRatio != NULL)
      *pWidthRatio = (GXINT)m_nWidthRatio;
    if(pHeightRatio != NULL)
      *pHeightRatio = (GXINT)m_nHeightRatio;
    return TRUE;
  }
  GXUINT GTextureImpl::GetWidth()
  {
    return m_nWidth;
  }
  GXUINT GTextureImpl::GetHeight()
  {
    return m_nHeight;
  }
  GXBOOL GTextureImpl::GetDimension(GXUINT* pWidth, GXUINT* pHeight)
  {
    if(pWidth != NULL || pHeight != NULL)
    {
      //GXGRAPHICSDEVICE_DESC GrapDeviceDesc;
      //m_pGraphics->GetDesc(&GrapDeviceDesc);

      if(pWidth != NULL)
        *pWidth = m_nWidth;
      if(pHeight != NULL)
        *pHeight = m_nHeight;

      return TRUE;
    }
    return FALSE;
    //if( (pWidth != NULL && ((GXINT)m_nWidth) < 0) ||
    //  (pHeight != NULL && ((GXINT)m_nHeight) < 0) )
    //{
    //  D3DPRESENT_PARAMETERS d3dpp;
    //  m_pGraphics->D3DGetPresentParam(&d3dpp);

    //  if(pWidth != NULL)
    //  {
    //    *pWidth = m_nWidth;
    //    if(((GXINT)m_nWidth) < 0)
    //      *pWidth = (GXUINT)((float)(-(GXINT)m_nWidth) / (float)TEXSIZE_FIXEDPOINT * d3dpp.BackBufferWidth);
    //  }

    //  if(pHeight != NULL)
    //  {
    //    *pHeight = m_nHeight;
    //    if(((GXINT)m_nHeight) < 0)
    //      *pHeight = (GXUINT)((float)(-(GXINT)m_nHeight) / (float)TEXSIZE_FIXEDPOINT * d3dpp.BackBufferHeight);
    //  }
    //}
    //else
    //{
    //  ASSERT(m_nWidth > 0);
    //  ASSERT(m_nHeight > 0);
    //  if(pWidth != NULL)
    //    *pWidth = m_nWidth;
    //  if(pHeight != NULL)
    //    *pHeight = m_nHeight;
    //}
    //return TRUE;
  }
  GXDWORD GTextureImpl::GetUsage()
  {
    return m_dwResUsage;
  }
  GXFormat GTextureImpl::GetFormat()
  {
    return m_Format;
  }

  GXVOID GTextureImpl::GenerateMipMaps()
  {
    //DWORD dwLevel = m_pTexture->GetLevelCount();
    //if(dwLevel > 1)
    //{
    //  LPDIRECT3DSURFACE9 pSrcSur = NULL;
    //  LPDIRECT3DSURFACE9 pDstSur = NULL;
    //  LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphics->D3DGetDevice();
    //  m_pTexture->GetSurfaceLevel(0, &pSrcSur);

    //  for(DWORD i = 1; i < dwLevel; i++)
    //  {
    //    m_pTexture->GetSurfaceLevel(i, &pDstSur);
    //    pd3dDevice->StretchRect(pSrcSur, NULL, pDstSur, NULL, D3DTEXF_LINEAR);
    //    pSrcSur->Release();
    //    pSrcSur = pDstSur;
    //  }
    //  pDstSur->Release();
    //}
  }

  //GXPool GTextureImpl::GetPool()
  //{
  //  return m_Pool;
  //}

  GXBOOL GTextureImpl::GetDesc(GXBITMAP*lpBitmap)
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
  //GXLRESULT GTextureFromUser::AddRef()
  //{
  //  m_uRefCount++;
  //  if(m_uRefCount == 1) {
  //    return OnDeviceEvent(DE_ResetDevice);
  //  }
  //  return m_uRefCount;
  //}
  
  GXHRESULT GTextureFromUser::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);

    if(pDesc->dwCmdCode == RC_ResetDevice)
    {
      //ASSERT(m_pTexRV == NULL);
      //ASSERT(m_pTexture == NULL);
      //ASSERT(m_pHelpTexture == NULL);
      //ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
      //CalcTextureActualDimension();

      //D3D11_TEXTURE2D_DESC TexDesc;
      //InlSetZeroT(TexDesc);
      //TexDesc.Width = m_nWidth;
      //TexDesc.Height = m_nHeight;
      //TexDesc.MipLevels = m_nMipLevels;
      //TexDesc.ArraySize = 1;
      //TexDesc.Format = GrapXToDX11::FormatFrom(m_Format);
      //TexDesc.SampleDesc.Count = 1;
      //TexDesc.SampleDesc.Quality = 0;
      //TexDesc.Usage = D3D11_USAGE_DEFAULT;
      //TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      //TexDesc.CPUAccessFlags = 0;
      //TexDesc.MiscFlags = 0;

      //GrapXToDX11::TextureDescFromResUsage(m_dwResUsage, &TexDesc);

      //HRESULT hval = pd3dDevice->CreateTexture2D( &TexDesc, NULL, &m_pTexture);
      //if(SUCCEEDED(hval) && TEST_FLAG(TexDesc.BindFlags, D3D11_BIND_SHADER_RESOURCE)) {
      //  hval = pd3dDevice->CreateShaderResourceView(m_pTexture, NULL, &m_pTexRV);
      //  if(SUCCEEDED(hval)) {
      //    return 0;
      //  }
      //}
      CLBREAK;
      return GTextureImpl::Invoke(pDesc);
    }
    else if(pDesc->dwCmdCode == RC_LostDevice)
    {
      return GTextureImpl::Invoke(pDesc);
    }
    return GX_OK;
  }
  GXBOOL GTextureFromUser::CopyRect(GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination)
  {   
    return TRUE;
  }

  GTextureFromUser::GTextureFromUser(GXGraphicsImpl* pGraphicsImpl) 
    : GTextureImpl(pGraphicsImpl)
  {
  }
  //GTextureFromUser::~GTextureFromUser()
  //{
  //}

  GXBOOL GTextureFromUser::Initialize(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    m_emType       = GTextureImpl::User;
    m_nWidth       = 0;
    m_nHeight      = 0;
    m_nMipLevels   = MipLevels;
    m_Format       = Format;
    m_dwResUsage   = ResUsage;
    m_nWidthRatio  = (GXWORD)WidthRatio;
    m_nHeightRatio = (GXWORD)HeightRatio;

    CalcTextureActualDimension();

    ASSERT(m_pTexRV == NULL);
    ASSERT(m_pTexture == NULL);
    ASSERT(m_pHelpTexture == NULL);
    ASSERT(m_nWidth >= 1 && m_nWidth < 16384 && m_nHeight >= 1 && m_nHeight < 16384);

    ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
    D3D11_TEXTURE2D_DESC TexDesc;
    InlSetZeroT(TexDesc);

    TexDesc.Width              = m_nWidth;
    TexDesc.Height             = m_nHeight;
    TexDesc.MipLevels          = m_nMipLevels;
    TexDesc.ArraySize          = 1;
    TexDesc.Format             = GrapXToDX11::FormatFrom(m_Format);
    TexDesc.SampleDesc.Count   = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage              = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    TexDesc.CPUAccessFlags     = 0;
    TexDesc.MiscFlags          = 0;

    GrapXToDX11::TextureDescFromResUsage(m_dwResUsage, &TexDesc);

    ASSERT(TexDesc.Width < 16384 && TexDesc.Height < 16384);
    HRESULT hval = pd3dDevice->CreateTexture2D( &TexDesc, NULL, &m_pTexture);
    if(SUCCEEDED(hval) && TEST_FLAG(TexDesc.BindFlags, D3D11_BIND_SHADER_RESOURCE)) {
      hval = pd3dDevice->CreateShaderResourceView(m_pTexture, NULL, &m_pTexRV);
      if(FAILED(hval)) {
        return FALSE;
      }
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL GTextureFromUserRT::Initialize(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    ASSERT(m_pRenderTargetView == NULL);
    ASSERT(m_pDepthStencil == NULL);
    ASSERT(m_pDepthStencilView == NULL);
    GXBOOL bval = FALSE;
    if(GTextureFromUser::Initialize(WidthRatio, HeightRatio, MipLevels, Format, ResUsage)) {
      ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
      D3D11_RENDER_TARGET_VIEW_DESC TarDesc;
      InlSetZeroT(TarDesc);

      TarDesc.Format = GrapXToDX11::FormatFrom(m_Format);
      TarDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

      HRESULT hval = pd3dDevice->CreateRenderTargetView(m_pTexture, &TarDesc, &m_pRenderTargetView);
      bval = GXSUCCEEDED(hval);



      // Create depth stencil texture
      D3D11_TEXTURE2D_DESC descDepth;
      InlSetZeroT(descDepth);
      descDepth.Width               = m_nWidth;
      descDepth.Height              = m_nHeight;
      descDepth.MipLevels           = 1;
      descDepth.ArraySize           = 1;
      descDepth.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
      descDepth.SampleDesc.Count    = 1;
      descDepth.SampleDesc.Quality  = 0;
      descDepth.Usage               = D3D11_USAGE_DEFAULT;
      descDepth.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
      descDepth.CPUAccessFlags      = 0;
      descDepth.MiscFlags           = 0;
      hval = pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
      if(FAILED(hval)) {
        return hval;
      }

      // Create the depth stencil view
      D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
      InlSetZeroT(descDSV);
      descDSV.Format              = descDepth.Format;
      descDSV.ViewDimension       = D3D11_DSV_DIMENSION_TEXTURE2D;
      descDSV.Texture2D.MipSlice  = 0;
      hval = pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
    }
    return bval;
  }

  GXBOOL GTextureFromUserRT::CopyRect(GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination)
  {   
    return FALSE;
  }

  GXHRESULT GTextureFromUserRT::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);

    if(pDesc->dwCmdCode == RC_ResetDevice)
    {
      CLBREAK;
    }
    else if(pDesc->dwCmdCode == RC_LostDevice)
    {
      SAFE_RELEASE(m_pRenderTargetView);
      return GTextureFromUser::Invoke(pDesc);
    }
    return GX_OK;
  }
  GTextureFromUserRT::GTextureFromUserRT(GXGraphicsImpl* pGraphicsImpl) 
    : GTextureFromUser(pGraphicsImpl)
    , m_pRenderTargetView(NULL)
    , m_pDepthStencil(NULL)
    , m_pDepthStencilView(NULL)
  {
  }


  GTextureFromUserRT::~GTextureFromUserRT()
  {
    SAFE_RELEASE(m_pRenderTargetView);
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pDepthStencilView);
  }

  //////////////////////////////////////////////////////////////////////////

  GTextureFromFile::GTextureFromFile(
    GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, 
    GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, 
    GXDWORD MipFilter, GXCOLORREF ColorKey, GXGraphics* pGraphics)
    : GTextureImpl(pGraphics)
  {
    m_strSrcFile = pSrcFile;
    m_nWidth     = Width;
    m_nHeight    = Height;
    m_nMipLevels = MipLevels;
    m_Format     = Format;
    m_dwResUsage = ResUsage;
    m_Filter     = Filter;
    m_MipFilter  = MipFilter;
    m_ColorKey   = ColorKey;
  }
  GTextureFromFile::~GTextureFromFile()
  {

  }

  //UINT GTextureFromFile::ConvertParamSizeToD3D(GXUINT nSize)
  //{
  //  if(nSize == GX_DEFAULT) {
  //    return D3DX_DEFAULT;
  //  }
  //  else if(nSize == GX_DEFAULT_NONPOW2) {
  //    return D3DX_DEFAULT_NONPOW2;
  //  }
  //  else return nSize;
  //}

  HRESULT GTextureFromFile::Create(LPGXIMAGEINFOX pSrcInfo)
  {
    ID3D11Device* pd3dDevice = m_pGraphics->D3DGetDevice();
    //CalcTextureActualDimension();

    //DirectX::Load
    D3D11_SHADER_RESOURCE_VIEW_DESC resview_desc;
    InlSetZeroT(resview_desc);


    //HRESULT hval = D3DX11CreateTextureFromFileW(pd3dDevice, 
    //  m_strSrcFile, NULL, NULL, (ID3D11Resource**)&m_pTexture, NULL);
    DirectX::ScratchImage image;
    DirectX::TexMetadata metadata;
    HRESULT hval = S_OK;
    if(clpathfile::CompareExtension(m_strSrcFile, _CLTEXT("dds")))
    {
      hval = DirectX::LoadFromDDSFile(reinterpret_cast<const wchar_t*>(m_strSrcFile.CStr()), DirectX::DDS_FLAGS_NONE, &metadata, image);

      if(GXFAILED(hval)) {
        return hval;
      }

      const DirectX::Image* pImage = image.GetImage(0, 0, 0);
      hval = DirectX::CreateTexture(pd3dDevice, pImage, 1, metadata, (ID3D11Resource**)&m_pTexture);
      resview_desc.Format = pImage->format;
    }
    //else if(clpathfile::CompareExtension(m_strSrcFile, _CLTEXT("tga")))
    //{
    //  hval = DirectX::LoadFromTGAFile(m_strSrcFile, &metadata, image);
    //}
    else
    {
      clstd::File file;
      if(file.OpenExisting(m_strSrcFile))
      {
        clstd::MemBuffer buffer;
        if(file.ReadToBuffer(&buffer) == FALSE) {
          return GX_ERROR_OUROFMEMORY;
        }

        FIMEMORY* fi_mem = FreeImage_OpenMemory((BYTE*)buffer.GetPtr(), buffer.GetSize());
        FREE_IMAGE_FORMAT fi_fmt = FreeImage_GetFileTypeFromMemory(fi_mem);
        if(fi_fmt == FIF_UNKNOWN) {
          return E_FAIL;
        }

        FIBITMAP* fibmp = FreeImage_LoadFromMemory(fi_fmt, fi_mem);

        D3D11_TEXTURE2D_DESC tex2d_desc;
        D3D11_SUBRESOURCE_DATA subres_data;
        InlSetZeroT(tex2d_desc);
        InlSetZeroT(subres_data);

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
        m_Format = GXFMT_A8B8G8R8;
#else
        m_Format = GXFMT_A8R8G8B8;
#endif

        tex2d_desc.Width          = FreeImage_GetWidth(fibmp);    // UINT 
        tex2d_desc.Height         = FreeImage_GetHeight(fibmp);   // UINT 
        tex2d_desc.MipLevels      = 1;// UINT 
        tex2d_desc.ArraySize      = 1;// UINT 
        tex2d_desc.Format         = GrapXToDX11::FormatFrom(m_Format); // DXGI_FORMAT
        tex2d_desc.SampleDesc.Count = 1; // DXGI_SAMPLE_DESC
        tex2d_desc.SampleDesc.Quality = 0; // DXGI_SAMPLE_DESC
        tex2d_desc.Usage          = D3D11_USAGE_DEFAULT; // D3D11_USAGE
        tex2d_desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;      // UINT
        tex2d_desc.CPUAccessFlags = 0; // UINT
        tex2d_desc.MiscFlags      = 0;      // UINT

        subres_data.pSysMem     = FreeImage_GetBits(fibmp);
        subres_data.SysMemPitch = FreeImage_GetPitch(fibmp);

        hval = pd3dDevice->CreateTexture2D(&tex2d_desc, &subres_data,&m_pTexture);

        FreeImage_Unload(fibmp);
        FreeImage_CloseMemory(fi_mem);

        resview_desc.Format = tex2d_desc.Format;
      }
      else
      {
        CLOG_ERRORW(_CLTEXT("Can not load texture file:%s"), m_strSrcFile.CStr());
      }
    }



    if(SUCCEEDED(hval)) {
      resview_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      resview_desc.Texture2D.MipLevels = 1;
      resview_desc.Texture2D.MostDetailedMip = 0;

      hval = pd3dDevice->CreateShaderResourceView(m_pTexture, &resview_desc, &m_pTexRV);
      if(SUCCEEDED(hval)) {
        D3D11_TEXTURE2D_DESC Tex2Desc;
        m_pTexture->GetDesc(&Tex2Desc);
        m_nWidth = Tex2Desc.Width;
        m_nHeight = Tex2Desc.Height;
        m_nWidthRatio = m_nWidth;
        m_nHeightRatio = m_nHeight;
        // FIXME: 补全
        return GX_OK;
      }
    }
    return hval;
      //D3DX11CreateTexture2DFrom( pd3dDevice, L"seafloor.dds", NULL, NULL, &g_pTextureRV, NULL );
    //D3DXIMAGE_INFO ImageInfo;
    //memset(&ImageInfo, 0, sizeof(D3DXIMAGE_INFO));

    //DWORD dwUsage;
    //D3DPOOL Pool;
    //ConvertTexResUsageToNative(m_dwResUsage, dwUsage, Pool);

    //UINT nPWidth = ConvertParamSizeToD3D(m_nWidth);
    //UINT nPHeight = ConvertParamSizeToD3D(m_nHeight);

    //HRESULT hval = D3DXCreateTextureFromFileEx(m_pGraphics->D3DGetDevice(), m_strSrcFile, 
    //  nPWidth, nPHeight, (GXUINT)m_nMipLevels, dwUsage, (D3DFORMAT)m_Format, Pool, 
    //  (GXDWORD)m_Filter, (GXDWORD)m_MipFilter, (D3DCOLOR)m_ColorKey, &ImageInfo, NULL, &m_pTexture);

    //if(SUCCEEDED(hval))
    //{
    //  m_nWidthRatio  = (GXSHORT)ImageInfo.Width;
    //  m_nHeightRatio = (GXSHORT)ImageInfo.Height;

    //  if(pSrcInfo != NULL)
    //  {
    //    pSrcInfo->Width     = ImageInfo.Width;
    //    pSrcInfo->Height    = ImageInfo.Height;
    //    pSrcInfo->Depth     = ImageInfo.Depth;
    //    pSrcInfo->MipLevels = ImageInfo.MipLevels;
    //    pSrcInfo->Format    = (GXGraphicsFormat)ImageInfo.Format;
    //  }

    //  if(GXSUCCEEDED(m_pTexture->GetSurfaceLevel(0, &m_pSurface)))
    //  {
    //    D3DSURFACE_DESC d3dsd;
    //    m_pSurface->GetDesc(&d3dsd);
    //    m_nWidth  = d3dsd.Width;
    //    m_nHeight = d3dsd.Height;
    //    m_Format = (GXFormat)d3dsd.Format;
    //    ASSERT(d3dsd.Pool == Pool &&
    //      d3dsd.Usage == dwUsage);
    //  }
    //}

    //return hval;
    //return FALSE;
  }
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  //GXBOOL GTextureOffscreenPlainSur::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)
  //{
  //  //D3DLOCKED_RECT d3dlr;
  //  //GXHRESULT hr;

  //  //ASSERT(m_pTexture == NULL);
  //  //hr = m_pSurface->LockRect(&d3dlr, (RECT*)lpRect, Flags);
  //  //lpLockRect->pBits = d3dlr.pBits;
  //  //lpLockRect->Pitch = d3dlr.Pitch;
  //  //ASSERT(GXSUCCEEDED(hr));
  //  //return GXSUCCEEDED(hr);
  //  return TRUE;
  //}
  //GXBOOL GTextureOffscreenPlainSur::UnlockRect()
  //{
  //  //GXHRESULT hr;
  //  //hr = m_pSurface->UnlockRect();
  //  //ASSERT(GXSUCCEEDED(hr));
  //  //return GXSUCCEEDED(hr);
  //  return TRUE;
  //}


  //GXBOOL GTextureOffscreenPlainSur::CopyRect(GTexture* pSrc)
  //{
  //  //LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
  //  //ASSERT(IS_PTR(pSrc));
  //  //if(((GTextureImpl*)pSrc)->GetCreateType() == User && pSrc->GetUsage() == D3DUSAGE_RENDERTARGET &&
  //  //  GXSUCCEEDED(lpd3dDevice->GetRenderTargetData(((GTextureImpl*)pSrc)->D3DSurface(), m_pSurface)))
  //  //  return TRUE;
  //  //ASSERT(FALSE);
  //  return FALSE;
  //}
  //GTextureOffscreenPlainSur::GTextureOffscreenPlainSur(GXGraphics* pGraphics)
  //  : GTextureImpl(pGraphics)
  //{
  //  m_pTexture = NULL;
  //}
  //GTextureOffscreenPlainSur::~GTextureOffscreenPlainSur()
  //{

  //}

} // namespace D3D11
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)