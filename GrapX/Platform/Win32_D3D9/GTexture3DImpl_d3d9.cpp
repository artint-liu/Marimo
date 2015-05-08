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
#include "Platform/Win32_D3D9/GTexture3DImpl_d3d9.H"
#define _GXGRAPHICS_INLINE_TEXTURE_D3D9_
#include "Canvas/GXResourceMgr.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"

namespace D3D9
{
//#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

  //////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GTexture3DImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT GTexture3DImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0) {
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXBOOL GTexture3DImpl::Clear (GXCONST LPBOX lpRect, GXCOLOR dwColor)
  {
    CLBREAK;
    return FALSE;
  }

  GXUINT GTexture3DImpl::GetWidth()
  {
    return m_nWidth;
  }

  GXUINT GTexture3DImpl::GetHeight()
  {
    return m_nHeight;
  }

  GXUINT GTexture3DImpl::GetDepth()
  {
    return m_nDepth;
  }

  GXBOOL GTexture3DImpl::GetDimension(GXUINT* pWidth, GXUINT* pHeight, GXUINT* pDepth)
  {
    if(pWidth) {
      *pWidth = m_nWidth;
    }
    if(pHeight) {
      *pHeight = m_nHeight;
    }
    if(pDepth) {
      *pDepth = m_nDepth;
    }
    return TRUE;
  }

  GXDWORD GTexture3DImpl::GetUsage()
  {
    return m_dwResUsage;
  }

  GXFormat GTexture3DImpl::GetFormat()
  {
    return m_Format;
  }

  GXVOID GTexture3DImpl::GenerateMipMaps()
  {
    CLBREAK;
  }

  GXBOOL GTexture3DImpl::CopyBox(GTexture3D* pSrc, GXCONST LPBOX lprcSource, GXUINT x, GXUINT y, GXUINT z)
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL GTexture3DImpl::LockBox(LPLOCKEDBOX lpLockRect, GXCONST LPBOX lpBox, GXDWORD Flags)
  {
    CLBREAK;
    return FALSE;
  }

  GXBOOL GTexture3DImpl::UnlockBox()
  {
    CLBREAK;
    return FALSE;
  }

  GXGraphics* GTexture3DImpl::GetGraphicsUnsafe()
  {
    return m_pGraphicsImpl;
  }

  GXBOOL GTexture3DImpl::SaveToFileW(GXLPCWSTR szFileName, GXLPCSTR szDestFormat)
  {
    if(szFileName == NULL) {
      return FALSE;
    }
    D3DXIMAGE_FILEFORMAT d3diff;
    ConvertTextureFileSaveFormat(szDestFormat, &d3diff);
    return GXSUCCEEDED(D3DXSaveTextureToFileW(szFileName, d3diff, m_pTexture, NULL));
  }

  GTexture3DImpl::GTexture3DImpl( GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXGraphics* pGraphics ) : GTexBaseImplT   ((GXGraphicsImpl*)pGraphics)
    //: m_pGraphicsImpl ((GXGraphicsImpl*)pGraphics)
    , m_nWidth        (Width)
    , m_nHeight       (Height)
    , m_nDepth        (Depth)
    , m_nMipLevels    (MipLevels)
    , m_Format        (Format)
    , m_dwResUsage    (ResUsage)
    //, m_pTexture      (NULL)
  {

  }

  GTexture3DImpl::~GTexture3DImpl()
  {
    if(m_Format != GXFMT_UNKNOWN) {
      m_pGraphicsImpl->UnregisterResource(this);
    }
    SAFE_RELEASE(m_pTexture);
  }

  //////////////////////////////////////////////////////////////////////////
  GTexture3DFromFile::GTexture3DFromFile(GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, 
    GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXGraphics* pGraphics)
    : GTexture3DImpl(Width, Height, Depth, MipLevels, Format, ResUsage, pGraphics)
    , m_strSrcFile(pSrcFile)
    , m_Filter    (Filter)
    , m_MipFilter (MipFilter)
    , m_ColorKey  (ColorKey)
  {
  }

  GTexture3DFromFile::~GTexture3DFromFile()
  {
  }

  HRESULT GTexture3DFromFile::Create(LPGXIMAGEINFOX pSrcInfo)
  {
    D3DXIMAGE_INFO ImageInfo; // 文件的原始信息, 用来参考
    memset(&ImageInfo, 0, sizeof(D3DXIMAGE_INFO));

    DWORD dwUsage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, dwUsage, Pool);

    UINT nPWidth  = GTextureFromFile::ConvertParamSizeToD3D(m_nWidth);
    UINT nPHeight = GTextureFromFile::ConvertParamSizeToD3D(m_nHeight);
    UINT nPDepth  = GTextureFromFile::ConvertParamSizeToD3D(m_nDepth);

    HRESULT hval = D3DXCreateVolumeTextureFromFileEx(m_pGraphicsImpl->D3DGetDevice(), m_strSrcFile, 
      nPWidth, nPHeight, nPDepth, (GXUINT)m_nMipLevels, dwUsage, (D3DFORMAT)m_Format, Pool, 
      (GXDWORD)m_Filter, (GXDWORD)m_MipFilter, (D3DCOLOR)m_ColorKey, &ImageInfo, NULL, (LPDIRECT3DVOLUMETEXTURE9*)&m_pTexture);

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

      m_nWidth  = ImageInfo.Width;
      m_nHeight = ImageInfo.Height;
      m_nDepth  = ImageInfo.Depth;
      m_Format  = (GXFormat)ImageInfo.Format;
      m_nMipLevels = ImageInfo.MipLevels;

      //if(GXSUCCEEDED(m_pTexture->GetSurfaceLevel(0, &m_pSurface)))
      //{
      //  D3DSURFACE_DESC d3dsd;
      //  m_pSurface->GetDesc(&d3dsd);
      //  m_nWidth  = d3dsd.Width;
      //  m_nHeight = d3dsd.Height;
      //  m_Format = (GXFormat)d3dsd.Format;
      //  ASSERT(d3dsd.Pool == Pool &&
      //    (d3dsd.Usage == dwUsage || d3dsd.Usage == D3DUSAGE_DYNAMIC));
      //}
      //m_nWidthRatio  = (GXSHORT)m_nWidth;
      //m_nHeightRatio = (GXSHORT)m_nHeight;
    }
    else {
      m_Format = GXFMT_UNKNOWN; // 用来标志创建失败
    }
    return hval;
  }

  GXHRESULT GTexture3DFromFile::Invoke(GRESCRIPTDESC* pDesc)
  { 
    INVOKE_DESC_CHECK(pDesc);
    DWORD Usage;
    D3DPOOL Pool;
    ConvertTexResUsageToNative(m_dwResUsage, Usage, Pool);

    switch(pDesc->dwCmdCode)
    {
    case RC_ResetDevice:
      {
        //ASSERT((m_pTexture == NULL && m_pSurface == NULL) ||
        //  (m_pTexture != NULL && m_pSurface != NULL));

        if(Pool == D3DPOOL_SYSTEMMEM)
        {
          if(m_pTexture != NULL)
            return GX_OK;
        }

        //ASSERT(m_pTexture == NULL && m_pSurface == NULL);
        if(GXSUCCEEDED(Create(NULL))) {
          return GX_OK;
        }
        return GX_FAIL;
      }
      break;

    case RC_LostDevice:
      SAFE_RELEASE(m_pTexture);
      return GX_OK;
    }
    return GX_OK;
  }

} // namespace D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
