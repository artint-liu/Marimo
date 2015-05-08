#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXCanvas.H>

// 平台相关
// 私有头文件
#include "ScrollBar.H"
#include "Utility/AeroCommon.H"
#include "GrapX/GXUser.H"
#include "GrapX/guxtheme.h"

extern GXHTHEME hTheme_ScrollBar;

GXVOID GXGetScrollBarDrawingInfo(LPGXSCROLLBARDRAWINGINFO lpsbdi, LPGXSCROLLBAR lpScrollBar, GXUINT uRange)
{
  lpsbdi->nTotalPage = uRange - (GX_SCROLLBAR_BUTTON_WIDTH << 1);

  if(lpScrollBar->m_nMax > lpScrollBar->m_nMin)
  {
    lpsbdi->nPageSize = ((lpScrollBar->m_nPage * lpsbdi->nTotalPage << 2) / (lpScrollBar->m_nMax - lpScrollBar->m_nMin + 1) + 2) >> 2;
    lpsbdi->nTrackPos = ((lpScrollBar->m_nPos  * lpsbdi->nTotalPage << 2) / (lpScrollBar->m_nMax - lpScrollBar->m_nMin + 1) + 2) >> 2;
  }
  else
  {
    lpsbdi->nPageSize = 0;
    lpsbdi->nTrackPos = 0;
  }
  if(lpsbdi->nPageSize < GX_SCROLLBAR_MINPAGE_SIZE)
    lpsbdi->nPageSize = GX_SCROLLBAR_MINPAGE_SIZE;
}
GXBOOL GXPaintHScrollBar(GXWndCanvas*pCanvas, LPGXSCROLLBAR lpScrollBar, GXRECT* lpRect, GXDWORD uFlag)
{
  GXSCROLLBARDRAWINGINFO psbdi;
  GXINT nCurFocus = 0;
  if(!(lpScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
    nCurFocus = 1;

  //static GXINT s_ScrollModuleList[][7] = {
  //  {
  //    IDCOMMON_HSCROLL_DISABLE_BTN_LEFT,
  //    IDCOMMON_HSCROLL_DISABLE_SHAFT,
  //    -1,-1,-1,
  //    IDCOMMON_HSCROLL_DISABLE_BTN_RIGHT
  //  },
  //  {
  //    IDCOMMON_HSCROLL_NORMAL_BTN_LEFT,
  //    IDCOMMON_HSCROLL_NORMAL_SHAFT,
  //    IDCOMMON_HSCROLL_NORMAL_BOX_LEFT,
  //    IDCOMMON_HSCROLL_NORMAL_BOX,
  //    IDCOMMON_HSCROLL_NORMAL_BOX_RIGHT,
  //    IDCOMMON_HSCROLL_NORMAL_BTN_RIGHT
  //  },
  //};

  GXGDIDC dc;
  dc.emObjType = GXGDIOBJ_DC;
  dc.pCanvas = pCanvas->GetCanvasUnsafe();

  static int s_Flag2StateMap[][4] = {
    {GXABS_LEFTNORMAL, GXABS_RIGHTNORMAL, GXSCRBS_NORMAL, GXSCRBS_NORMAL},
    {GXABS_LEFTHOT, GXABS_RIGHTHOT, GXSCRBS_NORMAL, GXSCRBS_NORMAL},
  };

  GXRECT rect;
  gxSetRect(&rect,   
    lpRect->left, 
    lpRect->top, 
    lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH, 
    lpRect->bottom);
  gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_ARROWBTN, s_Flag2StateMap[uFlag][0], &rect, NULL);

  gxSetRect(&rect,   
    lpRect->right - GX_SCROLLBAR_BUTTON_WIDTH, 
    lpRect->top, 
    lpRect->right, 
    lpRect->bottom);
  gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_ARROWBTN, s_Flag2StateMap[uFlag][1], &rect, NULL);

  GXGetScrollBarDrawingInfo(&psbdi, lpScrollBar, lpRect->right - lpRect->left);

  if(!(lpScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
  {
    gxSetRect(&rect, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH,
      lpRect->top, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH + psbdi.nTrackPos,
      lpRect->bottom);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_LOWERTRACKHORZ, s_Flag2StateMap[uFlag][2], &rect, NULL);


    gxSetRect(&rect, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH + psbdi.nTrackPos + psbdi.nPageSize,
      lpRect->top, 
      lpRect->right - GX_SCROLLBAR_BUTTON_WIDTH,
      lpRect->bottom);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_UPPERTRACKHORZ, s_Flag2StateMap[uFlag][2], &rect, NULL);

    gxSetRect(&rect, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH + psbdi.nTrackPos,
      lpRect->top, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH + psbdi.nTrackPos + psbdi.nPageSize,
      lpRect->bottom);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_THUMBBTNHORZ, s_Flag2StateMap[uFlag][3], &rect, NULL);
  }
  else
  {
    gxSetRect(&rect, 
      lpRect->left + GX_SCROLLBAR_BUTTON_WIDTH,
      lpRect->top, 
      lpRect->right - GX_SCROLLBAR_BUTTON_WIDTH,
      lpRect->bottom);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_LOWERTRACKHORZ, GXSCRBS_DISABLED, &rect, NULL);
  }

  return TRUE;
}

GXBOOL GXPaintVScrollBar(GXWndCanvas*pCanvas, LPGXSCROLLBAR lpScrollBar, GXRECT* lpRect, GXDWORD uFlag)
{
  GXSCROLLBARDRAWINGINFO psbdi;
  GXINT nCurFocus = 0;
  if(!(lpScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
    nCurFocus = 1;
  //ASSERT(lpScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL);
  static GXINT s_ScrollModuleList[][7] = {
    {
      IDCOMMON_VSCROLL_DISABLE_BTN_LEFT,
      IDCOMMON_VSCROLL_DISABLE_SHAFT,
      -1,-1,-1,
      IDCOMMON_VSCROLL_DISABLE_BTN_RIGHT
    },
    {
      IDCOMMON_VSCROLL_NORMAL_BTN_LEFT,
      IDCOMMON_VSCROLL_NORMAL_SHAFT,
      IDCOMMON_VSCROLL_NORMAL_BOX_TOP,
      IDCOMMON_VSCROLL_NORMAL_BOX,
      IDCOMMON_VSCROLL_NORMAL_BOX_BOTTOM,
      IDCOMMON_VSCROLL_NORMAL_BTN_RIGHT
    },
  };
  GXGDIDC dc;
  dc.emObjType = GXGDIOBJ_DC;
  dc.pCanvas = pCanvas->GetCanvasUnsafe();

  GXRECT rect;
  gxSetRect(&rect,   
    lpRect->left, 
    lpRect->top, 
    lpRect->right, 
    lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT);
  gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_ARROWBTN, GXABS_UPNORMAL, &rect, NULL);

  gxSetRect(&rect,   
    lpRect->left, 
    lpRect->bottom - GX_SCROLLBAR_BUTTON_HEIGHT, 
    lpRect->right, 
    lpRect->bottom);
  gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_ARROWBTN, GXABS_DOWNNORMAL, &rect, NULL);

  GXGetScrollBarDrawingInfo(&psbdi, lpScrollBar, lpRect->bottom - lpRect->top);

  if(!(lpScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
  {
    gxSetRect(&rect, 
      lpRect->left, 
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT,
      lpRect->right,
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT + psbdi.nTrackPos);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_LOWERTRACKVERT, GXSCRBS_NORMAL, &rect, NULL);


    gxSetRect(&rect, 
      lpRect->left, 
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT + psbdi.nTrackPos + psbdi.nPageSize,
      lpRect->right,
      lpRect->bottom - GX_SCROLLBAR_BUTTON_HEIGHT);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_UPPERTRACKVERT, GXSCRBS_NORMAL, &rect, NULL);

    gxSetRect(&rect, 
      lpRect->left, 
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT + psbdi.nTrackPos,
      lpRect->right,
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT + psbdi.nTrackPos + psbdi.nPageSize);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_THUMBBTNVERT, GXSCRBS_NORMAL, &rect, NULL);
  }
  else
  {
    gxSetRect(&rect, 
      lpRect->left, 
      lpRect->top + GX_SCROLLBAR_BUTTON_HEIGHT,
      lpRect->right,
      lpRect->bottom - GX_SCROLLBAR_BUTTON_HEIGHT);
    gxDrawThemeBackground(hTheme_ScrollBar, GXGDI_DC_HANDLE(&dc), GXSBP_LOWERTRACKVERT, GXSCRBS_DISABLED, &rect, NULL);
  }

  return TRUE;
}
#endif // _DEV_DISABLE_UI_CODE