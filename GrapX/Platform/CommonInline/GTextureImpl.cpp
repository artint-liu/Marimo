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
#include "GrapX/GXImage.h"
//#include "GrapX/GXFont.h"
//#include "GrapX/GXCanvas3D.h"
//#include "GrapX/MOLogger.h"

// 平台相关
//#include "GrapX/Platform.h"
//#include "GrapX/DataPool.h"
//#include "GrapX/DataPoolVariable.h"
//#include "Platform/Win32_XXX.h"
//#include "Platform/Win32_D3D9.h"
//#include "Platform/Win32_D3D9/GPrimitiveImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GShaderStubImpl_d3d9.h"
//#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.h"

// 私有头文件
#include <clPathFile.h>
//#include "Platform/Win32_D3D9/GStateBlock_d3d9.h"
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "Canvas/GXEffectImpl.h"
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

#include "GrapX/gxError.h"

//#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.h"
// Canvas3D用的
//#include "GrapX/GCamera.h"
//#include "GrapX/GrapVR.h"  // Canvas3D 用的
//#include "Canvas/GXMaterialImpl.h"
// </Canvas3D用的>


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
