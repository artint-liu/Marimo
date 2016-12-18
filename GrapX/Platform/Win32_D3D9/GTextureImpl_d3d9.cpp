#ifdef ENABLE_GRAPHICS_API_DX9
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GTexture.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXKernel.H"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"

// 私有头文件
#include "Platform/Win32_D3D9/GTextureImpl_d3d9.H"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D9_
#include "Canvas/GXResourceMgr.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"

namespace D3D9
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"


  void CalcCopyingRects(GTexture* pSrcTexture, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination, RECT& rcDest, RECT& rcSrc)
  {
    if(lprcSource == NULL) {
      rcSrc.left = 0;
      rcSrc.top = 0;
      pSrcTexture->GetDimension((GXUINT*)&rcSrc.right, (GXUINT*)&rcSrc.bottom);
    }
    else {
      rcSrc = *(LPRECT)lprcSource;
    }

    if(lpptDestination == NULL) {
      rcDest.left   = 0;
      rcDest.top    = 0;
      rcDest.right  = rcSrc.right - rcSrc.left;
      rcDest.bottom = rcSrc.bottom - rcSrc.top;
    }
    else {
      rcDest.left   = lpptDestination->x;
      rcDest.top    = lpptDestination->y;
      rcDest.right  = rcSrc.right - rcSrc.left + lpptDestination->x;
      rcDest.bottom = rcSrc.bottom - rcSrc.top + lpptDestination->y;
    }    
  }

  GXHRESULT GTextureImpl::CreateRes()
  {
    return GX_OK;
  }

  GXHRESULT GTextureImpl::DestroyRes()
  {
    SAFE_RELEASE(m_pSurface);
    SAFE_RELEASE(m_pTexture);
    return GX_OK;
  }

  GXHRESULT GTextureImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      SAFE_RELEASE(m_pSurface);
      SAFE_RELEASE(m_pTexture);
      break;
    case RC_ResetDevice:
      {
      }
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
    clstd::ScopedLocker sl(m_pGraphicsImpl->GetLocker());
    const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      //INVOKE_LOST_DEVICE;
      DestroyRes();
      if(m_emType != CreationFailed)
      {
        m_pGraphicsImpl->UnregisterResource(this);
      }
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXBOOL GTextureImpl::SaveToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat)
  {
    if(szFileName == NULL) {
      return FALSE;
    }

    D3DXIMAGE_FILEFORMAT d3diff;
    ConvertTextureFileSaveFormat(szDestFormat, &d3diff);
    return GXSUCCEEDED(D3DXSaveTextureToFileW(szFileName, d3diff, m_pTexture, NULL));
  }

  GTextureImpl::GTextureImpl(GXGraphics* pGraphics)
    : GTexBaseImplT  ((GXGraphicsImpl*)pGraphics)
    //, m_pGraphics    ((GXGraphicsImpl*)pGraphics)
    //, m_pTexture     (NULL)
    , m_pSurface     (NULL)
    , m_nWidthRatio  (0)
    , m_nHeightRatio (0)
  {
  }

  GTextureImpl::GTextureImpl(GXGraphics* pGraphics, CREATIONTYPE eCreateType,
    GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
    : GTexBaseImplT  ((GXGraphicsImpl*)pGraphics)
    //, m_pGraphics    ((GXGraphicsImpl*)pGraphics)
    , m_emType       (eCreateType)
    //, m_pTexture     (NULL)
    , m_pSurface     (NULL)
    , m_nWidth       (0)
    , m_nHeight      (0)
    , m_nMipLevels   (MipLevels)
    , m_Format       (Format)
    , m_dwResUsage   (ResUsage)
    , m_nWidthRatio  ((GXWORD)WidthRatio)
    , m_nHeightRatio ((GXWORD)HeightRatio)
  {
    if(GetGraphicsFormatCategory(m_Format) == GXFMTCATE_DEPTHSTENCIL) {
      m_dwResUsage |= GXRU_TEX_DEPTHSTENCIL;
    }
  }

  GXBOOL GXDLLAPI GXSaveTextureToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat, GTexture* pTexture)
  {
    if(szFileName == NULL || pTexture == NULL)
      return FALSE;
    
    D3DXIMAGE_FILEFORMAT d3diff;
    ConvertTextureFileSaveFormat(szDestFormat, &d3diff);
    return GXSUCCEEDED(D3DXSaveTextureToFileW(szFileName, d3diff, ((GTextureImpl*)pTexture)->D3DTexture(), NULL));    
  }
  //////////////////////////////////////////////////////////////////////////
  GXBOOL GTextureImpl::Clear(const GXLPRECT lpRect, GXCOLOR dwColor)
  {
    GXHRESULT hr;
    GXRECT    rect;
    if(lpRect == NULL)
    {
      rect.left = 0;
      rect.top  = 0;
      GetDimension((GXUINT*)&rect.right, (GXUINT*)&rect.bottom);
    }
    else
      rect = *lpRect;

    DWORD   Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);
    if(m_dwResUsage & D3DUSAGE_RENDERTARGET)
    {
      hr = m_pGraphicsImpl->D3DGetDevice()->ColorFill(m_pSurface, (const RECT*)lpRect, (D3DCOLOR)dwColor);
      ASSERT(GXSUCCEEDED(hr));
      return GXSUCCEEDED(hr);
    }
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
    //  GXHRESULT hval = m_pGraphics->CreateTexture(&pHelperTex, NULL, rect.right, rect.bottom, 1, m_Format, GXRU_SYSTEMMEM);
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
    //  hval = m_pTexture->AddDirtyRect((RECT*)&rect);
    //  SAFE_RELEASE(pHelperTex);
    //  return TRUE;
    //}
    return FALSE;
  }

  GXBOOL GTextureImpl::FillSystemMemSurface(LPDIRECT3DSURFACE9 pSurface, D3DFORMAT fmt, const GXLPRECT lpRect, GXCOLOR dwColor)
  {
#ifdef _DEBUG
    D3DSURFACE_DESC SurfaceDesc;
    pSurface->GetDesc(&SurfaceDesc);
#endif // #ifdef _DEBUG

    // 放到条件编译外面是为了验证 ASSERT 的内容不会被编译为代码
    ASSERT(SurfaceDesc.Pool == D3DPOOL_SYSTEMMEM);
    ASSERT(SurfaceDesc.Format == fmt);

    D3DLOCKED_RECT  d3dlr;
    int x, y;
    HRESULT hr = pSurface->LockRect(&d3dlr, (const RECT*)lpRect, NULL);

    if(GXSUCCEEDED(hr))
    {
      const int nWidth  = lpRect->right - lpRect->left;
      const int nHeight = lpRect->bottom - lpRect->top;
      switch(fmt)
      {
      case D3DFMT_A8R8G8B8:
      case D3DFMT_X8R8G8B8:
        {
          GXDWORD* pBits = (GXDWORD*)d3dlr.pBits;
          for(y = 0; y < nHeight; y++) {
            for(x = 0; x < nWidth; x++) {
              pBits[x] = dwColor;
            }
            pBits += d3dlr.Pitch / sizeof(GXDWORD);
          }
        }
        break;
      default:  // 不支持的格式
        ASSERT(0);
      }

      pSurface->UnlockRect();
      return TRUE;
    }

    ASSERT(0);  // 不支持锁定
    return FALSE;
  }
  //////////////////////////////////////////////////////////////////////////
  GTextureImpl::CREATIONTYPE  GTextureImpl::GetCreationType()
  {
    ASSERT(m_emType > Invalid && m_emType < LastType);
    return m_emType;
  }

  GXBOOL GTextureImpl::CopyRect(GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination)
  {
    ASSERT(GetGraphicsUnsafe() == pSrc->GetGraphicsUnsafe());

    RECT rcDest;
    RECT rcSrc;
    CalcCopyingRects(pSrc, lprcSource, lpptDestination, rcDest, rcSrc);
    //if(lprcSource == NULL) {
    //  pSrc->GetDimension((GXUINT*)&rcSrc.right, (GXUINT*)&rcSrc.bottom);
    //}
    //else {
    //  rcSrc = *(RECT*)lprcSource;
    //}

    //if(lpptDestination == NULL) {
    //  rcDest.left   = 0;
    //  rcDest.top    = 0;
    //  rcDest.right  = rcSrc.right - rcSrc.left;
    //  rcDest.bottom = rcSrc.bottom - rcSrc.top;
    //}
    //else {
    //  rcDest.left   = lpptDestination->x;
    //  rcDest.top    = lpptDestination->y;
    //  rcDest.right  = rcSrc.right - rcSrc.left + lpptDestination->x;
    //  rcDest.bottom = rcSrc.bottom - rcSrc.top + lpptDestination->y;
    //}    

    LPDIRECT3DSURFACE9 pSrcSurface = ((GTextureImpl*)pSrc)->D3DSurface();
#ifdef _DEBUG
    D3DSURFACE_DESC DestDesc;
    D3DSURFACE_DESC SrcDesc;
    InlSetZeroT(DestDesc);
    InlSetZeroT(SrcDesc);
    m_pSurface->GetDesc(&DestDesc);
    pSrcSurface->GetDesc(&SrcDesc);
#endif // #ifdef _DEBUG
    
    if(GXFAILED(D3DXLoadSurfaceFromSurface(
      m_pSurface, NULL, (const RECT*)&rcDest, pSrcSurface,
      NULL, (const RECT*)&rcSrc, D3DX_FILTER_POINT, NULL)))
    {
      ASSERT(false);
      return FALSE;
    }

    return TRUE;
  }

  GXBOOL GTextureImpl::StretchRect(GTexture* pSrc, const GXLPCRECT lpDestRect, const GXLPCRECT lpSrcRect, GXTextureFilterType eFilter)
  {
    HRESULT hval = GX_OK;
    m_pGraphicsImpl->Enter();
    GTextureImpl* pSrcImp = (GTextureImpl*)pSrc;
#ifdef _DEBUG
    GXUINT cxSrc, cySrc;
    GXUINT cxDst, cyDst;
    if(lpSrcRect != NULL)
    {
      pSrc->GetDimension(&cxSrc, &cySrc);
      ASSERT(lpSrcRect->left >= 0 && lpSrcRect->top >= 0 &&
        lpSrcRect->right <= (GXINT)cxSrc && lpSrcRect->bottom <= (GXINT)cySrc);
    }
    if(lpDestRect)
    {
      GetDimension(&cxDst, &cyDst);
      ASSERT(lpDestRect->left >= 0 && lpDestRect->top >= 0 &&
        lpDestRect->right <= (GXINT)cxDst && lpDestRect->bottom <= (GXINT)cyDst);
    }
#endif // _DEBUG
    // StretchRect 不支持 Texture 到 Texture 对拷.
    if( TEST_FLAG(m_dwResUsage, GXRU_TEX_RENDERTARGET) == 0 && 
      TEST_FLAG(pSrcImp->m_dwResUsage, GXRU_TEX_RENDERTARGET) == 0 )
    {
      hval = D3DXLoadSurfaceFromSurface(m_pSurface, NULL, 
        (RECT*)lpDestRect, pSrcImp->m_pSurface, NULL, (RECT*)lpSrcRect, eFilter, 0);
      ASSERT(GXSUCCEEDED(hval));
    }
    else
    {
      hval = m_pGraphicsImpl->D3DGetDevice()->StretchRect(
        ((GTextureImpl*)pSrc)->m_pSurface, (const RECT *)lpSrcRect, 
        m_pSurface, (const RECT *)lpDestRect, (D3DTEXTUREFILTERTYPE)eFilter);
      ASSERT(GXSUCCEEDED(hval));
    }
    m_pGraphicsImpl->Leave();
    return GXSUCCEEDED(hval);
  }

  GXBOOL GTextureImpl::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)
  {
    ASSERT(/*lpRect == 0 && */Flags == 0);

    D3DLOCKED_RECT d3dlr;
    if(GXSUCCEEDED(m_pSurface->LockRect(&d3dlr, (const RECT*)lpRect, Flags)))
    {
      lpLockRect->pBits = d3dlr.pBits;
      lpLockRect->Pitch = d3dlr.Pitch;
      return TRUE;
    }
    else
      ASSERT(0);
    return FALSE;
  }

  GXBOOL GTextureImpl::UnlockRect()
  {
    return GXSUCCEEDED(m_pSurface->UnlockRect());
  }

  GXGraphics* GTextureImpl::GetGraphicsUnsafe()
  {
    return m_pGraphicsImpl;
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
    DWORD dwLevel = m_pTexture->GetLevelCount();
    if(dwLevel > 1)
    {
      LPDIRECT3DTEXTURE9 pTexture = static_cast<LPDIRECT3DTEXTURE9>(m_pTexture);
      LPDIRECT3DSURFACE9 pSrcSur = NULL;
      LPDIRECT3DSURFACE9 pDstSur = NULL;
      LPDIRECT3DDEVICE9 pd3dDevice = m_pGraphicsImpl->D3DGetDevice();
      pTexture->GetSurfaceLevel(0, &pSrcSur);

      for(DWORD i = 1; i < dwLevel; i++)
      {
        pTexture->GetSurfaceLevel(i, &pDstSur);
        pd3dDevice->StretchRect(pSrcSur, NULL, pDstSur, NULL, D3DTEXF_LINEAR);
        pSrcSur->Release();
        pSrcSur = pDstSur;
      }
      pDstSur->Release();
    }
  }

  GXBOOL GTextureImpl::GetDesc(GXBITMAP*lpBitmap)
  {
    D3DSURFACE_DESC sd;
    if(FAILED(static_cast<LPDIRECT3DTEXTURE9>(m_pTexture)->GetLevelDesc(0, &sd)))
      return FALSE;

    lpBitmap->bmType       = 0;
    lpBitmap->bmWidth      = sd.Width;
    lpBitmap->bmHeight     = sd.Height;
    lpBitmap->bmWidthBytes = sd.Width;
    lpBitmap->bmPlanes     = 1;
    switch(sd.Format)
    {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
      lpBitmap->bmBitsPixel = 32;
      break;
    case D3DFMT_A8:
      lpBitmap->bmBitsPixel = 8;
      break;
    default:
      ASSERT(FALSE);
    }
    lpBitmap->bmBits = NULL;
    return TRUE;
  }
  //////////////////////////////////////////////////////////////////////////
  GXHRESULT GTextureFromUser::Release()
  {
    clstd::ScopedLocker sl(m_pGraphicsImpl->GetLocker());
    const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      //INVOKE_LOST_DEVICE;
      DestroyRes();

      // 创建纹理失败时m_pTexture和m_pSurface为空,这是还没有注册
      if(m_emType != CreationFailed) {
        m_pGraphicsImpl->UnregisterResource(this);
      }
      delete this;
      return GX_OK;
    }

    // TODO: 这里 GetRenderTargetData 是性能瓶颈,暂时跳过
    //return nRefCount;
    //if(m_pSysMemTexture != NULL && TEST_FLAGS_ALL(m_dwResUsage, GXRU_TEX_RENDERTARGET|GXRU_FREQUENTLYREAD)) {
    //  IntHoldRenderTarget();
    //}
    return nRefCount;
  }

  GXHRESULT GTextureFromUser::CreateRes()
  {
    DWORD dwUsage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, dwUsage, Pool);

    //if(GetGraphicsFormatCategory(m_Format) == GXFMTCATE_DEPTHSTENCIL) {
    //  dwUsage |= D3DUSAGE_DEPTHSTENCIL;
    //}

    ASSERT((m_pTexture == NULL && m_pSurface == NULL) ||
      (m_pTexture != NULL && m_pSurface != NULL));

    GXGRAPHICSDEVICE_DESC GrapDeviceDesc;
    m_pGraphicsImpl->GetDesc(&GrapDeviceDesc);

    GXDWORD dwFlags = TEST_FLAG(m_pGraphicsImpl->GetCaps(GXGRAPCAPS_TEXTURE), GXTEXTURECAPS_NONPOW2)
      ? NULL : TEXTURERATIO_POW2;

    m_nWidth = (m_nWidthRatio < 0)
      ? TextureRatioToDimension((GXINT)m_nWidthRatio, GrapDeviceDesc.BackBufferWidth, dwFlags)
      : ((GXUINT)(GXINT)m_nWidthRatio);

    m_nHeight = (m_nHeightRatio < 0)
      ? TextureRatioToDimension((GXINT)m_nHeightRatio, GrapDeviceDesc.BackBufferHeight, dwFlags)
      : ((GXUINT)(GXINT)m_nHeightRatio);

    if(Pool == D3DPOOL_SYSTEMMEM)
    {
      if(m_pTexture != NULL && m_pSurface != NULL)
        return GX_OK;
    }

    GXHRESULT hval = GX_FAIL;
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();

    ASSERT(m_pTexture == NULL); // 下面的代码忘了为什么这么写,这段断言测试是否还存在不等于0的情况
    if(m_pTexture != NULL || SUCCEEDED(D3DXCreateTexture(
      lpd3dDevice, (UINT)m_nWidth, (UINT)m_nHeight,
      (GXUINT)m_nMipLevels, (GXDWORD)dwUsage, (D3DFORMAT)m_Format, (D3DPOOL)Pool, (LPDIRECT3DTEXTURE9*)&m_pTexture)))
    {
      if(m_pSurface != NULL || SUCCEEDED(static_cast<LPDIRECT3DTEXTURE9>(m_pTexture)->GetSurfaceLevel(0, &m_pSurface)))
      {
        if(Pool == D3DPOOL_DEFAULT && (dwUsage & D3DUSAGE_RENDERTARGET))
        {
          lpd3dDevice->ColorFill(m_pSurface, NULL, 0);
          static_cast<LPDIRECT3DTEXTURE9>(m_pTexture)->AddDirtyRect(NULL);
        }
        hval = GTextureImpl::CreateRes();
      }
    }
    else
    {
      TRACE(__FUNCTION__ ": error when calling D3DXCreateTexture.\n");
    }

    if(m_pSysMemTexture == NULL && GXSUCCEEDED(hval))
    {
      IntCreateShaderTexture(lpd3dDevice);
      //if(GXSUCCEEDED(hval) && Pool != D3DPOOL_SYSTEMMEM &&
      //  TEST_FLAG_NOT(dwUsage, D3DUSAGE_DEPTHSTENCIL)) {
      //    hval = D3DXCreateTexture(lpd3dDevice, (UINT)m_nWidth, (UINT)m_nHeight, 
      //      (GXUINT)m_nMipLevels, NULL, (D3DFORMAT)m_Format, D3DPOOL_SYSTEMMEM, &m_pSysMemTexture);
      //}
    }
    V(hval);
    return hval;
  }
  
  GXBOOL GTextureFromUser::IntCreateShaderTexture(LPDIRECT3DDEVICE9 lpd3dDevice)
  {
    ASSERT(m_pSysMemTexture == NULL);
    if(TEST_FLAG_NOT(m_dwResUsage, GXRU_TEX_DEPTHSTENCIL|GXRU_SYSTEMMEM)) {
        HRESULT hr = D3DXCreateTexture(lpd3dDevice, (UINT)m_nWidth, (UINT)m_nHeight, 
          (GXUINT)m_nMipLevels, NULL, (D3DFORMAT)m_Format, D3DPOOL_SYSTEMMEM, &m_pSysMemTexture);
        return SUCCEEDED(hr);
    }
    return TRUE;
  }

  GXBOOL GTextureFromUser::IntIsNeedBackupRTTexture() const
  {
    return (m_pSysMemTexture != NULL && 
      TEST_FLAGS_ALL(m_dwResUsage, GXRU_TEX_RENDERTARGET | GXRU_FREQUENTLYREAD));
  }

  GXBOOL GTextureFromUser::IntHoldRenderTarget()
  {
    ASSERT(m_pSysMemTexture != NULL && TEST_FLAGS_ALL(m_dwResUsage, GXRU_TEX_RENDERTARGET|GXRU_FREQUENTLYREAD));

    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    LPDIRECT3DSURFACE9 pSystemMemSurface = NULL;
    m_pSysMemTexture->GetSurfaceLevel(0, &pSystemMemSurface);
    HRESULT hr = lpd3dDevice->GetRenderTargetData(m_pSurface, pSystemMemSurface);
#ifdef _DEBUG
    if(FAILED(hr)) {
      D3DSURFACE_DESC SysMemDesc;
      D3DSURFACE_DESC NativeDesc;
      pSystemMemSurface->GetDesc(&SysMemDesc);
      m_pSurface->GetDesc(&NativeDesc);
      CLBREAK;
      return FALSE;
    }
#endif // #ifdef _DEBUG
    SAFE_RELEASE(pSystemMemSurface);
    return TRUE;
  }

  GXHRESULT GTextureFromUser::DestroyRes()
  {
    SAFE_RELEASE(m_pSysMemTexture);
    return GTextureImpl::DestroyRes();
  }

  GXHRESULT GTextureFromUser::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    //DWORD dwUsage;
    //D3DPOOL Pool;
    //ConvertTexResUsageToNative(m_dwResUsage, dwUsage, Pool);
    GXHRESULT hval = GX_OK;
    switch(pDesc->dwCmdCode)
    {
    case RC_ResetDevice:
      {
        hval = CreateRes();
        if(GXSUCCEEDED(hval) && m_pSysMemTexture != NULL) {
          LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
          LPDIRECT3DSURFACE9 pSysMemSurface = NULL;
          //RECT rcSource;
          m_pSysMemTexture->GetSurfaceLevel(0, &pSysMemSurface);
          D3DSURFACE_DESC SurDesc;
          pSysMemSurface->GetDesc(&SurDesc);
          RECT rect = {0, 0, clMin(SurDesc.Width, m_nWidth), clMin(SurDesc.Height, m_nHeight)};

          //SetRect(&rcSource, 0, 0, SurDesc.Width, SurDesc.Height);
          HRESULT hr = lpd3dDevice->UpdateSurface(pSysMemSurface, &rect, m_pSurface, 0);
          V(hr);

          if(SurDesc.Width != m_nWidth || SurDesc.Height != m_nHeight)
          {
            // 尺寸可变纹理需要重新创建系统内存纹理
            // 并且将原来纹理内容复制到新纹理上
            SAFE_RELEASE(m_pSysMemTexture); // 不需要Texture, 只需要它的Surface足够, COM原理
            LPDIRECT3DSURFACE9 pNewSurface = NULL;
            IntCreateShaderTexture(lpd3dDevice);
            m_pSysMemTexture->GetSurfaceLevel(0, &pNewSurface);

            hr = D3DXLoadSurfaceFromSurface(pNewSurface, NULL, &rect, pSysMemSurface, NULL, &rect, D3DX_FILTER_POINT, 0);

            ASSERT(SUCCEEDED(hr));
            SAFE_RELEASE(pNewSurface);
          }
          SAFE_RELEASE(pSysMemSurface);
        }
      }
      break;
    case RC_LostDevice:
      return GTextureImpl::Invoke(pDesc);
    }
    return hval;
  }

  GXBOOL GTextureFromUser::Clear(const GXLPRECT lpRect, GXCOLOR dwColor)
  {
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    GXRECT    rect;

    if(lpRect == NULL)
    {
      rect.left = 0;
      rect.top  = 0;
      GetDimension((GXUINT*)&rect.right, (GXUINT*)&rect.bottom);
    }
    else {
      rect = *lpRect;
    }

    DWORD Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);


    if(Pool == D3DPOOL_SYSTEMMEM || Pool == D3DPOOL_MANAGED)
    {
      return GTextureImpl::FillSystemMemSurface(m_pSurface, (D3DFORMAT)m_Format, &rect, dwColor);
    }
    else 
    {
      if(m_pSysMemTexture != NULL)
      {
        LPDIRECT3DSURFACE9 pSystemMemSurface = NULL;
        m_pSysMemTexture->GetSurfaceLevel(0, &pSystemMemSurface);
        GTextureImpl::FillSystemMemSurface(pSystemMemSurface, (D3DFORMAT)m_Format, &rect, dwColor);
        lpd3dDevice->UpdateTexture(m_pSysMemTexture, m_pTexture);
        SAFE_RELEASE(pSystemMemSurface);
        return TRUE;
      }
      else
      {
        GTexture* pHelperTex;
        GXHRESULT hval = m_pGraphicsImpl->CreateTexture(&pHelperTex, NULL, rect.right, rect.bottom, 1, m_Format, GXRU_SYSTEMMEM);
        ASSERT(GXSUCCEEDED(hval));
        if(pHelperTex->Clear(lpRect, dwColor) == FALSE)
        {
          SAFE_RELEASE(pHelperTex);
          ASSERT(0);
          return FALSE;
        }
        if(CopyRect(pHelperTex, &rect, NULL) == FALSE)
        {
          SAFE_RELEASE(pHelperTex);
          ASSERT(0);
          return FALSE;
        }
        hval = static_cast<LPDIRECT3DTEXTURE9>(m_pTexture)->AddDirtyRect((RECT*)&rect);
        SAFE_RELEASE(pHelperTex);
        return TRUE;
      }
    }

    CLBREAK;
    return FALSE;
  }

  GXBOOL GTextureFromUser::CopyRect(GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination)
  {
    GXHRESULT hr = E_FAIL;
    GTextureImpl* pSrcImpl = static_cast<GTextureImpl*>(pSrc);
    CREATIONTYPE ctSrc = pSrcImpl->GetCreationType();
    LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
    /*if(ctSrc == OffscreenPlainSur)
    {
      hr =lpd3dDevice->UpdateSurface(pSrcImpl->D3DSurface(), NULL, m_pSurface, NULL);
    }
    else */if(ctSrc == User || ctSrc == File/* || ctSrc == FileEx*/)
    {
      DWORD dwSrcUsage;
      D3DPOOL SrcPool;// = pSrc->GetPool();//, poolDest = GetPool();
      ConvertTexResUsageToNative(pSrcImpl->GetUsage(), dwSrcUsage, SrcPool);

      DWORD dwDstUsage;
      D3DPOOL DstPool;
      ConvertTexResUsageToNative(m_dwResUsage, dwDstUsage, DstPool);

      switch (SrcPool)
      {
      case D3DPOOL_DEFAULT:
        if(DstPool == D3DPOOL_DEFAULT) {
          RECT rcDest, rcSrc;
          CalcCopyingRects(pSrc, lprcSource, lpptDestination, rcDest, rcSrc);

          if(m_pSysMemTexture) {
            LPDIRECT3DSURFACE9 pDestSurface;
            hr = m_pSysMemTexture->GetSurfaceLevel(0, &pDestSurface);
            if(SUCCEEDED(hr))
            {
              hr = D3DXLoadSurfaceFromSurface(pDestSurface, NULL, &rcDest, pSrcImpl->D3DSurface(), NULL, &rcSrc, D3DX_FILTER_POINT, 0);
              SAFE_RELEASE(pDestSurface);
            }

            if(FAILED(hr)) {
              return FALSE;
            }
          }

          hr = D3DXLoadSurfaceFromSurface(m_pSurface, NULL, (const RECT*)&rcDest, pSrcImpl->D3DSurface(),
            NULL, (const RECT*)&rcSrc, D3DX_FILTER_POINT, NULL);

          return SUCCEEDED(hr);
        }
        else if(DstPool == D3DPOOL_SYSTEMMEM) {
          ASSERT(m_pSysMemTexture == NULL);
          if(pSrcImpl->GetUsage() & GXRU_TEX_RENDERTARGET)
          {
            hr = lpd3dDevice->GetRenderTargetData(((GTextureImpl*)pSrc)->D3DSurface(), m_pSurface);
          }
        }
        break;
      case D3DPOOL_SYSTEMMEM:
        if(DstPool == D3DPOOL_DEFAULT) {
          hr = lpd3dDevice->UpdateSurface(pSrcImpl->D3DSurface(), (LPCRECT)lprcSource, m_pSurface, (const POINT*)lpptDestination);
        }
        break;
      default:
        CLBREAK;
        break;
      }
    }
    else
      return GTextureImpl::CopyRect(pSrc, lprcSource, lpptDestination);

    if(FAILED(hr))
    {
      ASSERT(0);
      return FALSE;
    }
    return TRUE;
  }

  GTextureFromUser::GTextureFromUser(GXGraphics* pGraphics, GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
    : GTextureImpl(pGraphics, GTextureImpl::User, WidthRatio, HeightRatio, MipLevels, Format, ResUsage)
    , m_pSysMemTexture(NULL)
  {
  }

  GTextureFromUser::~GTextureFromUser()
  {
  }

  GXBOOL GTextureFromUser::LockRect( LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags )
  {
    if(m_pSysMemTexture) {
#ifdef _DEBUG
      D3DSURFACE_DESC d3dDesc;
      m_pSysMemTexture->GetLevelDesc(0, &d3dDesc);
      ASSERT(d3dDesc.Pool == D3DPOOL_SYSTEMMEM);
#endif // #ifdef _DEBUG
      D3DLOCKED_RECT locked_rect;
      if(SUCCEEDED(m_pSysMemTexture->LockRect(0, &locked_rect, (const RECT*)lpRect, Flags)))
      {
        lpLockRect->pBits = locked_rect.pBits;
        lpLockRect->Pitch = locked_rect.Pitch;
        return TRUE;
      }
      // 这里不调用基类的LockRect，否则Unlock时可能会搞错Locked Texture。
      CLBREAK;
      return FALSE;
    }
    return GTextureImpl::LockRect(lpLockRect, lpRect, Flags);
  }

  GXBOOL GTextureFromUser::UnlockRect()
  {
    if(m_pSysMemTexture) {
      if(SUCCEEDED(m_pSysMemTexture->UnlockRect(0))) {
        LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphicsImpl->D3DGetDevice();
        return SUCCEEDED(lpd3dDevice->UpdateTexture(m_pSysMemTexture, m_pTexture));
      }
    }
    return GTextureImpl::UnlockRect();
  }

  //////////////////////////////////////////////////////////////////////////



  GXHRESULT GTextureFromFile::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);

    switch(pDesc->dwCmdCode)
    {
    case RC_ResetDevice:
      {
        ASSERT((m_pTexture == NULL && m_pSurface == NULL) ||
          (m_pTexture != NULL && m_pSurface != NULL));

        if(Pool == D3DPOOL_SYSTEMMEM)
        {
          if(m_pTexture != NULL && m_pSurface != NULL)
            return GX_OK;
        }

        ASSERT(m_pTexture == NULL && m_pSurface == NULL);
        if(GXSUCCEEDED(Create(NULL)))
        {
          return GTextureImpl::Invoke(pDesc);
        }
        return GX_FAIL;
      }
      break;
    case RC_LostDevice:
      return GTextureImpl::Invoke(pDesc);
    }
    return GX_OK;
  }

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
    //m_Pool       = Pool;
    m_Filter   = Filter;
    m_MipFilter  = MipFilter;
    m_ColorKey   = ColorKey;
    m_emType = GTextureImpl::File;
  }
  GTextureFromFile::~GTextureFromFile()
  {

  }

  UINT GTextureFromFile::ConvertParamSizeToD3D(GXUINT nSize)
  {
    if(nSize == GX_DEFAULT) {
      return D3DX_DEFAULT;
    }
    else if(nSize == GX_DEFAULT_NONPOW2) {
      return D3DX_DEFAULT_NONPOW2;
    }
    else return nSize;
  }

  HRESULT GTextureFromFile::Create(LPGXIMAGEINFOX pSrcInfo)
  {
    D3DXIMAGE_INFO ImageInfo; // 文件的原始信息, 用来参考
    memset(&ImageInfo, 0, sizeof(D3DXIMAGE_INFO));

    DWORD dwUsage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, dwUsage, Pool);

    UINT nPWidth = ConvertParamSizeToD3D(m_nWidth);
    UINT nPHeight = ConvertParamSizeToD3D(m_nHeight);

    HRESULT hval = D3DXCreateTextureFromFileEx(m_pGraphicsImpl->D3DGetDevice(), m_strSrcFile, 
      nPWidth, nPHeight, (GXUINT)m_nMipLevels, dwUsage, (D3DFORMAT)m_Format, Pool, 
      (GXDWORD)m_Filter, (GXDWORD)m_MipFilter, (D3DCOLOR)m_ColorKey, &ImageInfo, NULL, (LPDIRECT3DTEXTURE9*)&m_pTexture);

    if(SUCCEEDED(hval))
    {
      if(pSrcInfo != NULL)
      {
        pSrcInfo->Width     = ImageInfo.Width;
        pSrcInfo->Height    = ImageInfo.Height;
        pSrcInfo->Depth     = ImageInfo.Depth;
        pSrcInfo->MipLevels = ImageInfo.MipLevels;
        pSrcInfo->Format    = (GXGraphicsFormat)ImageInfo.Format; // FIXME: 这个不对!返回的是DXIFF格式
      }

      if(GXSUCCEEDED(static_cast<LPDIRECT3DTEXTURE9>(m_pTexture)->GetSurfaceLevel(0, &m_pSurface)))
      {
        D3DSURFACE_DESC d3dsd;
        m_pSurface->GetDesc(&d3dsd);
        m_nWidth  = d3dsd.Width;
        m_nHeight = d3dsd.Height;
        m_Format = (GXFormat)d3dsd.Format;
        ASSERT(d3dsd.Pool == Pool &&
          (d3dsd.Usage == dwUsage || d3dsd.Usage == D3DUSAGE_DYNAMIC));
      }
      m_nWidthRatio  = (GXSHORT)m_nWidth;
      m_nHeightRatio = (GXSHORT)m_nHeight;
    }
    //else
    //{
    //  m_emType = CreationFailed;
    //  CLOG_ERROR("CreateTextureFromFileEx: Can not create texture(%s).\n", clStringA(m_strSrcFile));
    //}
    return hval;
  }
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  //GXHRESULT GTextureOffscreenPlainSur::Invoke(GRESCRIPTDESC* pDesc)
  //{
  //  INVOKE_DESC_CHECK(pDesc);
  //  LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
  //  
  //  DWORD Usage;
  //  D3DPOOL Pool;
  //  ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);

  //  switch(pDesc->dwCmdCode)
  //  {
  //  case RC_ResetDevice:
  //    ASSERT(m_pTexture == NULL && m_pSurface == NULL);
  //    if(GXSUCCEEDED(lpd3dDevice->CreateOffscreenPlainSurface
  //      ((GXUINT)m_nWidth, (GXUINT)m_nHeight, (D3DFORMAT)m_Format, (D3DPOOL)Pool, &m_pSurface, NULL/*, m_dwUsage*/)))
  //    {
  //      return GTextureImpl::Invoke(pDesc);
  //    }
  //    return GX_FAIL;

  //  case RC_LostDevice:
  //    return GTextureImpl::Invoke(pDesc);
  //  case RC_ResizeDevice:
  //    break;
  //  }
  //  return GX_OK;

  //}

  //GXBOOL GTextureOffscreenPlainSur::LockRect(LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)
  //{
  //  D3DLOCKED_RECT d3dlr;
  //  GXHRESULT hr;

  //  ASSERT(m_pTexture == NULL);
  //  hr = m_pSurface->LockRect(&d3dlr, (RECT*)lpRect, Flags);
  //  lpLockRect->pBits = d3dlr.pBits;
  //  lpLockRect->Pitch = d3dlr.Pitch;
  //  ASSERT(GXSUCCEEDED(hr));
  //  return GXSUCCEEDED(hr);
  //}
  //GXBOOL GTextureOffscreenPlainSur::UnlockRect()
  //{
  //  GXHRESULT hr;
  //  hr = m_pSurface->UnlockRect();
  //  ASSERT(GXSUCCEEDED(hr));
  //  return GXSUCCEEDED(hr);
  //}


  //GXBOOL GTextureOffscreenPlainSur::CopyRect(GTexture* pSrc)
  //{
  //  LPDIRECT3DDEVICE9 lpd3dDevice = m_pGraphics->D3DGetDevice();
  //  ASSERT(IS_PTR(pSrc));
  //  if(((GTextureImpl*)pSrc)->GetCreateType() == User && pSrc->GetUsage() == D3DUSAGE_RENDERTARGET &&
  //    GXSUCCEEDED(lpd3dDevice->GetRenderTargetData(((GTextureImpl*)pSrc)->D3DSurface(), m_pSurface)))
  //    return TRUE;
  //  ASSERT(FALSE);
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

  GXHRESULT GTextureImpl_FromD3DSurface::Release()
  {
    clstd::ScopedLocker sl(m_pGraphicsImpl->GetLocker());
    const GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      INVOKE_LOST_DEVICE;
      m_pGraphicsImpl->UnregisterResource(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }

  GXHRESULT GTextureImpl_FromD3DSurface::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    DWORD Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      if(Pool != D3DPOOL_MANAGED && Pool != D3DPOOL_SYSTEMMEM)
      {
        m_pSurface = NULL;
      }

      return GTextureImpl::Invoke(pDesc);

    case RC_ResizeDevice:
      break;
    }
    return GX_OK;
  }

  GTextureImpl_FromD3DSurface::GTextureImpl_FromD3DSurface(GXGraphics* pGraphics, LPDIRECT3DSURFACE9 lpd3dSurface)
    : GTextureImpl(pGraphics)
  {
    D3DSURFACE_DESC d3dsd;
    if(lpd3dSurface == NULL)
    {
      m_pTexture = NULL;
      m_pSurface = NULL;
      return;
    }
    lpd3dSurface->GetDesc(&d3dsd);

    m_pTexture    = NULL;
    m_pSurface    = lpd3dSurface;
    m_emType      = D3DSurfaceRef;
    m_nWidth      = d3dsd.Width;
    m_nHeight     = d3dsd.Height;
    m_nMipLevels  = 1;
    m_Format      = (GXFormat)d3dsd.Format;
    ConvertNativeToTexResUsage(d3dsd.Usage, d3dsd.Pool, m_dwResUsage);
  }

  GTextureImpl_FromD3DSurface::~GTextureImpl_FromD3DSurface()
  {
  }
  //////////////////////////////////////////////////////////////////////////
} // namespace D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
