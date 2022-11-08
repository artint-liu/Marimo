//#define D3D9_LOW_DEBUG
//#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_CANVAS_D3D9_
#define _GXGRAPHICS_INLINE_RENDERTARGET_D3D9_
#define _GXGRAPHICS_INLINE_TEXTURE_D3D9_
#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GRegion.h"
//#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.h"
//#include "GrapX/GStateBlock.h"
#include "GrapX/GTexture.h"
//#include "GrapX/GXGraphics.h"
//#include "GrapX/GXCanvas.h"
#include "GrapX/GXRenderTarget.h"
//#include "GrapX/GXFont.h"
//#include "GrapX/GXCanvas3D.h"
//#include "GrapX/MOLogger.h"


// 私有头文件
#include <clPathFile.h>
//#include "Platform/Win32_D3D9/GStateBlock_d3d9.h"
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
//#include "Canvas/GXEffectImpl.h"
//#include "Console.h"
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStock.h>

#include "GrapX/GXKernel.h"
#include "GrapX/GXUser.h"
#include <GDI/RegionFunc.h>
#include <GDI/GRegionImpl.h>
//#include "Platform/Win32_D3D9/GTextureImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GTexture3DImpl_d3d9.h"

#include "Canvas/GXImageImpl.h"
//#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.h"

//#include <ft2build.h>
//#include <freetype/freetype.h>
//#include <freetype/ftglyph.h>
//#include <Canvas/GFTFontImpl.h>
//#include <GDI/GXShaderMgr.h>
#include <FreeImage.h>
#include "clImage.h"

#include "GrapX/gxError.h"
#ifdef ENABLE_DirectXTex
# include "third_party/DirectXTex/DirectXTex.h"
//# pragma comment(lib, "DirectXTex.lib")
#endif

namespace GrapXToDX11
{
  DXGI_FORMAT FormatFrom(GXFormat eFormat);
  GXFormat    FormatFrom(DXGI_FORMAT eFormat);
} // namespace GrapXToDX11

//////////////////////////////////////////////////////////////////////////
//
// namespace MarimoVerifier
// 
namespace MarimoVerifier
{
  namespace Texture
  {
    //static GXUINT c_nMaxTextureSize = 65536;  // 这个真不知道各个平台的限制, 先按一个最大可能来定义吧!
    GXBOOL CreateFromFileParam(
      GXLPCSTR  szPrefix,
      GXUINT    Width, 
      GXUINT    Height, 
      GXUINT    Depth, 
      GXUINT    MipLevels, 
      GXFormat  Format, 
      GXDWORD   ResUsage, 
      GXDWORD   Filter, 
      GXDWORD   MipFilter)
    {
      const GXFormatCategory eCate = Format == GXFMT_UNKNOWN 
        ? GXFMTCATE_OTHER : GetGraphicsFormatCategory(Format);
      if(eCate == GXFMTCATE_DEPTHSTENCIL) {
        CLOG_ERROR("%s can not create depth-stencil from file.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(Format != GXFMT_UNKNOWN && eCate != GXFMTCATE_COLOR && eCate != GXFMTCATE_COMPRESSEDCOLOR)
      {
        CLOG_ERROR("%s \"Format\" is out of range.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET))
      {
        CLOG_ERROR("%s Can't create as render target.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      return TRUE;
    }

    GXBOOL CreateParam(
      GXLPCSTR  szPrefix,
      GXUINT    Width, 
      GXUINT    Height, 
      GXUINT    Depth, 
      GXUINT    MipLevels, 
      GXFormat  Format, 
      GXDWORD   ResUsage)
    {
      //const LPCSTR szPrefix = "CreateTextureFromFile Error:";

      // 纹理尺寸是可以为负的, 这样会与屏幕保持一个比例
      if(Width == 0 || Height == 0)
      {
        CLOG_ERROR("%s Invalid texture size.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(Format == GXFMT_UNKNOWN)
      {
        CLOG_ERROR("%s Texture format must be specified.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      const GXFormatCategory eCate = GetGraphicsFormatCategory(Format);
      if(eCate == GXFMTCATE_DEPTHSTENCIL) {
        if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET)) {
          CLOG_ERROR("%s Can not specify \"GXRU_TEX_RENDERTARGET\" in depth-stencil texture.\n", szPrefix);
          ASSERT(0);
          return FALSE;
        }
      }

      if(Format != GXFMT_UNKNOWN && (eCate != GXFMTCATE_COLOR && eCate != GXFMTCATE_DEPTHSTENCIL))
      {
        CLOG_ERROR("%s \"Format\" is out of range.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }
      return TRUE;
    }

  } // namespace Texture
} // namespace MarimoVerifier

namespace GrapX
{
  GXBOOL GXDLLAPI Texture::EncodeToMemory(clstd::MemBuffer* pBuffer, GXLPCVOID pBitsData, GXFormat format,
    GXUINT width, GXUINT height, GXUINT cbPitch, GXLPCSTR szImageFormat, GXBOOL bVertFlip)
  {
    clStringA strFormat = szImageFormat;
    GXBOOL bval = TRUE;
    strFormat.MakeUpper();
#ifdef ENABLE_DirectXTex
    if (strFormat == "DDS" || strFormat == "DXT")
    {
      DirectX::Image image;
      DirectX::Blob blob;
      //D3D11_TEXTURE2D_DESC desc;
      //m_pD3D11Texture->GetDesc(&desc);
      image.width = width;
      image.height = height;
      image.format = GrapXToDX11::FormatFrom(format);
      image.rowPitch = cbPitch;
      image.slicePitch = cbPitch * height;
      image.pixels = (uint8_t*)pBitsData;

      if (SUCCEEDED(DirectX::SaveToDDSMemory(image, DirectX::DDS_FLAGS_NONE, blob)))
      {
        pBuffer->Resize(0, FALSE);
        pBuffer->Append(blob.GetBufferPointer(), blob.GetBufferSize());
      }
      else
      {
        bval = FALSE;
      }
    }
    else
#endif
    {
      GXUINT bpp = GetBytesOfGraphicsFormat(format);

      //GXLPVOID pBitsData = mapped.pBits;
      //GXINT nSourcePitch = mapped.Pitch;
      clstd::Image temp_image;
      FREE_IMAGE_TYPE fit = FIT_BITMAP;
      switch (format)
      {
      case Format_R8G8B8A8:
        temp_image.Set(width, height, "RGBA", 8, pBitsData, cbPitch);
        break;
      case Format_B8G8R8X8:
        temp_image.Set(width, height, "BGRX", 8, pBitsData, cbPitch);
        break;
      //case Format_B8G8R8:
      //  temp_image.Set(width, height, "BGRX", 8, pBitsData, cbPitch);
      //  break;
      case Format_R8:
        temp_image.Set(width, height, "R", 8, pBitsData, cbPitch);
        break;
      case Format_R8G8:
        temp_image.Set(width, height, "RG", 8, pBitsData, cbPitch);
        break;
      case Format_R32G32B32A32_Float:
        fit = FIT_RGBAF;
        break;
      case Format_R32:
        fit = FIT_FLOAT;
        break;
      default:
        CLBREAK;
      }

      if (temp_image.GetDataSize() > 0)
      {
        temp_image.SetFormat("BGRA");
        pBitsData = temp_image.GetLine(0);
        cbPitch = temp_image.GetPitch();
        bpp = temp_image.GetChannels();
      }

      FIBITMAP* fibmp = (fit == FIT_BITMAP)
        ? FreeImage_Allocate(width, height, bpp * 8)
        : FreeImage_AllocateT(fit, width, height, bpp * 8);
      BYTE* pDest = FreeImage_GetBits(fibmp);
      GXUINT nDestPitch = FreeImage_GetPitch(fibmp);

      if (bVertFlip)
      {
        pDest += nDestPitch * (height - 1);
        for (GXUINT y = 0; y < height; y++)
        {
          memcpy(pDest, pBitsData, clMin(nDestPitch, cbPitch));

          pDest -= nDestPitch;
          pBitsData = reinterpret_cast<GXLPVOID>(reinterpret_cast<size_t>(pBitsData) + cbPitch);
        }
      }
      else
      {
        for (GXUINT y = 0; y < height; y++)
        {
          memcpy(pDest, pBitsData, clMin(nDestPitch, cbPitch));

          pDest += nDestPitch;
          pBitsData = reinterpret_cast<GXLPVOID>(reinterpret_cast<size_t>(pBitsData) + cbPitch);
        }
      }

      FREE_IMAGE_FORMAT fi_format = FIF_UNKNOWN;

      if (strFormat == "PNG") {
        fi_format = FIF_PNG;
      }
      else if (strFormat == "JPEG" || strFormat == "JPG") {
        fi_format = FIF_JPEG;
      }
      else if (strFormat == "TIF" || strFormat == "TIFF") {
        fi_format = FIF_TIFF;
      }
      else if (strFormat == "TGA") {
        fi_format = FIF_TARGA;
      }
      else if (strFormat == "BMP") {
        fi_format = FIF_BMP;
      }
      else if (strFormat == "EXR") {
        fi_format = FIF_EXR;
      }

      if (fi_format != FIF_UNKNOWN)
      {
        FIMEMORY* fimemory = FreeImage_OpenMemory();
        if (FreeImage_SaveToMemory(fi_format, fibmp, fimemory))
        {
          BYTE *pData;
          DWORD size_in_bytes;
          if (FreeImage_AcquireMemory(fimemory, &pData, &size_in_bytes))
          {
            pBuffer->Resize(0, FALSE);
            pBuffer->Append(pData, size_in_bytes);
          }
          else
          {
            bval = FALSE;
          }
        }
        else
        {
          bval = FALSE;
        }
        FreeImage_CloseMemory(fimemory);
      }
      else
      {
        bval = FALSE;
      }

      FreeImage_Unload(fibmp);
    }
    return bval;
  }

  GXFormat GXDLLAPI Texture::DecodeToMemory(DECODE_TEXTURE_DESC* pDecodeDesc, GXLPCVOID pBitsData, GXUINT cbData, GXBOOL bVertFlip)
  {
    FIMEMORY* fi_mem = FreeImage_OpenMemory((BYTE*)pBitsData, (DWORD)cbData);
    FREE_IMAGE_FORMAT fi_fmt = FreeImage_GetFileTypeFromMemory(fi_mem);    
    GXFormat format = GXFormat::Format_Unknown;

    if (fi_fmt == FIF_UNKNOWN) {
      return GXFormat::Format_Unknown;
    }
#ifdef ENABLE_DirectXTex
    else if (fi_fmt == FIF_DDS)
    {
      DirectX::ScratchImage image;
      if(SUCCEEDED(DirectX::LoadFromDDSMemory(pBitsData, cbData, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, NULL, image)))
      {
        const DirectX::TexMetadata& meta = image.GetMetadata();
        format = GrapXToDX11::FormatFrom(meta.format);
        if (format != GXFormat::Format_Unknown)
        {
          GXLPCSTR szOrder = GetFormatChannelOrder(format);
          if(szOrder)
          {
            int nChannelDepth = GetBitsOfGraphicsFormat(format) / (int)clstd::strlenT(szOrder);

            ASSERT(clstd::strlenT(szOrder) <= 4);
            pDecodeDesc->pImageDesc->ptr = NULL;
            pDecodeDesc->pImageDesc->width = (int)image.GetImages()->width;
            pDecodeDesc->pImageDesc->height = (int)image.GetImages()->height * (int)meta.arraySize;
            pDecodeDesc->pImageDesc->channel = (int)clstd::strlenT(szOrder);
            pDecodeDesc->pImageDesc->pitch = (int)image.GetImages()->rowPitch;
            pDecodeDesc->pImageDesc->depth = nChannelDepth;
            clstd::strcpynT((GXLPSTR)pDecodeDesc->pImageDesc->format.name, szOrder, 4);

            pDecodeDesc->pBuffer->Resize(0, FALSE);
            if (format == Format_BC2 || format == Format_BC3)
            {
              pDecodeDesc->pBuffer->Append(image.GetPixels(), image.GetPixelsSize());
              pDecodeDesc->nMipLevels = (int)image.GetImageCount();
            }
            else
            {
              pDecodeDesc->pBuffer->Append(image.GetPixels(), image.GetPixelsSize());
            }
            pDecodeDesc->pImageDesc->ptr = pDecodeDesc->pBuffer->GetPtr();
          }
        }
      }
      return format;
    }
#endif

    FIBITMAP* fibmp = FreeImage_LoadFromMemory(fi_fmt, fi_mem);

    // FIXME:
    // 没有处理64位图像的地方
    // 没有检查图片格式

    //GXUINT nDIBSize = FreeImage_GetDIBSize(fibmp);
    //GXUINT nMemSize = FreeImage_GetMemorySize(fibmp);
    const GXUINT bpp = FreeImage_GetBPP(fibmp);
    const char* szFormat = NULL; // 均没验证顺序正确性
    int channel_depth = 0;
    if (bpp == 24)
    {
      CLBREAK;
      //format = Format_B8G8R8;
      //szFormat = "BGR";
      //channel_depth = 8;
    }
    else if (bpp == 32)
    {
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
      format = GXFMT_A8B8G8R8;
#else
      format = GXFMT_A8R8G8B8;
#endif
      szFormat = "RGBA";
      channel_depth = 8;
  }
    else if (bpp == 96)
    {
      format = Format_R32G32B32_Float;
      szFormat = "RGB";
      channel_depth = 32;
    }
    else if (bpp == 128)
    {
      format = Format_R32G32B32A32_Float;
      szFormat = "RGBA";
      channel_depth = 32;
    }
    else {
      CLBREAK;
    }

    if (bVertFlip) {
      FreeImage_FlipVertical(fibmp);
    }


    //if (FreeImage_GetWidth(fibmp) == FreeImage_GetHeight(fibmp) * 6)
    //{
    //  bval = CreateTextureCube(ppTexture, NULL, FreeImage_GetHeight(fibmp),
    //    format, eUsage, MipLevels, FreeImage_GetBits(fibmp), FreeImage_GetPitch(fibmp));

    //  // 有名字的要注册一下
    //  if (bval && szName)
    //  {
    //    m_ResMgr.Unregister(*ppTexture); // TODO: 暂时这么写吧，创建的核心功能还是得提到IntCreate中去
    //    m_ResMgr.Register(&rs, *ppTexture);
    //  }
    //}
    ASSERT((format == GXFormat::Format_Unknown && szFormat == NULL && channel_depth == 0) ||
      (format != GXFormat::Format_Unknown && szFormat != NULL && channel_depth != 0));

    if(format != GXFormat::Format_Unknown)
    {
      //pImage->Set(FreeImage_GetWidth(fibmp), FreeImage_GetHeight(fibmp), szFormat, channel_depth, FreeImage_GetBits(fibmp), FreeImage_GetPitch(fibmp));

      pDecodeDesc->pImageDesc->ptr = NULL;
      pDecodeDesc->pImageDesc->width = FreeImage_GetWidth(fibmp);
      pDecodeDesc->pImageDesc->height = FreeImage_GetHeight(fibmp);
      pDecodeDesc->pImageDesc->channel = (int)clstd::strlenT(szFormat);
      pDecodeDesc->pImageDesc->pitch = FreeImage_GetPitch(fibmp);
      pDecodeDesc->pImageDesc->depth = channel_depth;
      clstd::strcpynT((GXLPSTR)pDecodeDesc->pImageDesc->format.name, szFormat, 4);

      pDecodeDesc->pBuffer->Resize(0, FALSE);
      pDecodeDesc->pBuffer->Append(FreeImage_GetBits(fibmp), pDecodeDesc->pImageDesc->height * pDecodeDesc->pImageDesc->pitch);
      pDecodeDesc->pImageDesc->ptr = pDecodeDesc->pBuffer->GetPtr();
    }

    FreeImage_Unload(fibmp);
    FreeImage_CloseMemory(fi_mem);
    return format;
  }

} // namespace GrapX
