#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>

// 私有头文件
#include <User/GXWindow.h>
extern "C" {
  GXHRGN GXDLLAPI gxCreatePolygonRgn(
    GXCONST GXPOINT *lppt,  // pointer to array of points 
    int cPoints,  // number of points in array 
    int fnPolyFillMode   // polygon-filling mode 
    )
  {
    ASSERT(false);
    return FALSE;
  }

  int GXDLLAPI gxSetWindowRgn(
    GXHWND hWnd,  // handle to window whose window region is to be set
    GXHRGN hRgn,  // handle to region 
    GXBOOL bRedraw  // window redraw flag 
    )
  {
    ASSERT(false);
    return FALSE;
  }

  GXHRGN GXDLLAPI gxCreateRoundRectRgn(
    int nLeftRect,  // x-coordinate of the region's  upper-left corner 
    int nTopRect,  // y-coordinate of the region's upper-left corner 
    int nRightRect,  // x-coordinate of the region's lower-right corner 
    int nBottomRect,  // y-coordinate of the region's lower-right corner 
    int nWidthEllipse,  // height of ellipse for rounded corners 
    int nHeightEllipse   // width of ellipse for rounded corners 
    )
  {
    ASSERT(false);
    return NULL;
  }

  int GXDLLAPI gxGetWindowRgn(
    GXHWND hWnd,  // handle to window whose window region is to be obtained
    GXHRGN hRgn  // handle to region that receives a copy of the window region
    )
  {
    GXGDIREGION* lpGDIRegion = GXGDI_RGN_PTR(hRgn);
    SAFE_RELEASE(lpGDIRegion->lpRegion);
    return GXWND_PTR(hWnd)->GetWindowRegion(&lpGDIRegion->lpRegion);
  }

  GXBOOL GXDLLAPI gxFillRgn(
    GXHDC hdc,      // handle to device context 
    GXHRGN hrgn,    // handle to region to be filled 
    GXHBRUSH hbr    // handle to brush used to fill the region  
    )
  {
    ASSERT(false);
    return NULL;
  }

  GXBOOL GXDLLAPI gxFrameRgn(
    GXHDC hdc,       // handle to device context 
    GXHRGN hrgn,     // handle to region to be framed 
    GXHBRUSH hbr,    // handle to brush used to draw border  
    int nWidth,       // width of region frame 
    int nHeight      // height of region frame 
    )
  {
    ASSERT(false);
    return NULL;
  }

}
#endif // _DEV_DISABLE_UI_CODE