// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GRegion.h"
#include "GrapX/GXGraphics.h"

// 私有头文件
#include <GDI/RegionFunc.h>
#include <GDI/GRegionImpl.h>
#include "GXGraphicsBaseImpl.h"

namespace GrapX
{

  GraphicsBaseImpl::GraphicsBaseImpl()
    : m_pRgnAllocator(new GAllocator(NULL))
  {}

  GraphicsBaseImpl::~GraphicsBaseImpl()
  {
    SAFE_DELETE(m_pRgnAllocator);
  }

  GXHRESULT GraphicsBaseImpl::CreateRectRgn(GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom)
  {
    GXRECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;

    if(gxIsRectEmpty(&rect)) {
      *ppRegion = (GRegion*)GRegionImpl::CreateEmptyRgn(m_pRgnAllocator);
    }
    else {
      *ppRegion = (GRegion*)GRegionImpl::CreateRectRgn(m_pRgnAllocator, rect);
    }
    if(*ppRegion != NULL) {
      return GX_OK;
    }
    return GX_FAIL;
  }

  GXHRESULT GraphicsBaseImpl::CreateRectRgnIndirect(GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount)
  {
    *ppRegion = (GRegion*)GRegionImpl::CreateRectRgnIndirect(m_pRgnAllocator, lpRects, nCount);
    if(*ppRegion != NULL) {
      return GX_OK;
    }
    return GX_FAIL;
  }

  GXHRESULT GraphicsBaseImpl::CreateRoundRectRgn(GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse)
  {
    *ppRegion = (GRegion*)GRegionImpl::CreateRoundRectRgn(m_pRgnAllocator, rect, nWidthEllipse, nHeightEllipse);
    if(*ppRegion != NULL) {
      return GX_OK;
    }
    return GX_FAIL;
  }

} // namespace GrapX