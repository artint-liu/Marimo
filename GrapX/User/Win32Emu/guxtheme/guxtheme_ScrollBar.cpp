#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXSprite.H>

// 私有头文件
#include <User/Win32Emu/GUXTheme.hxx>
#include <User/Win32Emu/guxtheme/ScrollBar_ID_SBP.h>

unsigned long s_SpriteIdxMap_AB[] =
{
  17,
  ID_ABS_UPNORMAL,    //GXABS_UPNORMAL    1
  ID_ABS_UPHOT,      //GXABS_UPHOT      2
  ID_ABS_UPPRESSED,    //GXABS_UPPRESSED    3
  ID_ABS_UPDISABLED,    //GXABS_UPDISABLED    4
  ID_ABS_DOWNNORMAL,    //GXABS_DOWNNORMAL    5
  ID_ABS_DOWNHOT,      //GXABS_DOWNHOT      6
  ID_ABS_DOWNPRESSED,    //GXABS_DOWNPRESSED    7
  ID_ABS_DOWNDISABLED,  //GXABS_DOWNDISABLED  8
  ID_ABS_LEFTNORMAL,    //GXABS_LEFTNORMAL    9
  ID_ABS_LEFTHOT,      //GXABS_LEFTHOT      10
  ID_ABS_LEFTPRESSED,    //GXABS_LEFTPRESSED    11
  ID_ABS_LEFTDISABLED,  //GXABS_LEFTDISABLED  12
  ID_ABS_RIGHTNORMAL,    //GXABS_RIGHTNORMAL    13
  ID_ABS_RIGHTHOT,    //GXABS_RIGHTHOT    14
  ID_ABS_RIGHTPRESSED,  //GXABS_RIGHTPRESSED  15
  ID_ABS_RIGHTDISABLED,  //GXABS_RIGHTDISABLED  16
};

DRAWTHEMEBKG(DrawThemeBkg_SBP_ArrowBtn)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_AB[0]);
  s_pUxThemeSprite_ScrollBar->PaintModule(
    /*GXGDI_DC_PTR(hdc)->*/pCanvas, s_SpriteIdxMap_AB[iStateId], 
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}






unsigned long s_SpriteIdxMap_SCRBS_H_THUMB[] =
{
  4,
  ID_ABS_THUMB_H_NORMAL_1,    //GXSCRBS_NORMAL  1
  ID_ABS_THUMB_H_HOT_PRESSED_1,  //GXSCRBS_HOT    2
  ID_ABS_THUMB_H_HOT_PRESSED_1,  //GXSCRBS_PRESSED  3
  ID_ABS_THUMB_H_DISABLE_1,    //GXSCRBS_DISABLED  4
};

DRAWTHEMEBKG(DrawThemeBkg_SBP_ThumbBtnHorz)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_SCRBS_H_THUMB[0]);
  s_pUxThemeSprite_ScrollBar->PaintModule3H(
    /*GXGDI_DC_PTR(hdc)->*/pCanvas, 
    s_SpriteIdxMap_SCRBS_H_THUMB[iStateId],
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}

unsigned long s_SpriteIdxMap_SCRBS_V_THUMB[] =
{
  4,
  ID_ABS_THUMB_V_NORMAL_1,    //GXSCRBS_NORMAL  1
  ID_ABS_THUMB_V_HOT_PRESSED_1,  //GXSCRBS_HOT    2
  ID_ABS_THUMB_V_HOT_PRESSED_1,  //GXSCRBS_PRESSED  3
  ID_ABS_THUMB_V_DISABLE_1,    //GXSCRBS_DISABLED  4
};

DRAWTHEMEBKG(DrawThemeBkg_SBP_ThumbBtnVert)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_SCRBS_V_THUMB[0]);
  s_pUxThemeSprite_ScrollBar->PaintModule3V(
    /*GXGDI_DC_PTR(hdc)->*/pCanvas, 
    s_SpriteIdxMap_SCRBS_V_THUMB[iStateId],
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}

unsigned long s_SpriteIdxMap_SCRBS_H_TRACK[] =
{
  4,
  ID_ABS_TRACK_H_DHN,    //GXSCRBS_NORMAL  1
  ID_ABS_TRACK_H_DHN,    //GXSCRBS_HOT    2
  ID_ABS_TRACK_H_PRESSED,  //GXSCRBS_PRESSED  3
  ID_ABS_TRACK_H_DHN,    //GXSCRBS_DISABLED  4
};

DRAWTHEMEBKG(DrawThemeBkg_SBP_TrackHorz)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_SCRBS_H_TRACK[0]);
  s_pUxThemeSprite_ScrollBar->PaintModule(
    /*GXGDI_DC_PTR(hdc)->*/pCanvas, 
    s_SpriteIdxMap_SCRBS_H_TRACK[iStateId],
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}


unsigned long s_SpriteIdxMap_SCRBS_V_TRACK[] =
{
  4,
  ID_ABS_TRACK_V_DHN,    //GXSCRBS_NORMAL  1
  ID_ABS_TRACK_V_DHN,    //GXSCRBS_HOT    2
  ID_ABS_TRACK_V_PRESSED,  //GXSCRBS_PRESSED  3
  ID_ABS_TRACK_V_DHN,    //GXSCRBS_DISABLED  4
};
DRAWTHEMEBKG(DrawThemeBkg_SBP_TrackVert)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_SCRBS_V_TRACK[0]);
  s_pUxThemeSprite_ScrollBar->PaintModule(
    /*GXGDI_DC_PTR(hdc)->*/pCanvas, 
    s_SpriteIdxMap_SCRBS_V_TRACK[iStateId],
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}

DrawThemeBkg DRB_SBP_Table[] = {
  (DrawThemeBkg)10,
  DrawThemeBkg_SBP_ArrowBtn,      //#define GXSBP_ARROWBTN    1
  DrawThemeBkg_SBP_ThumbBtnHorz,    //#define GXSBP_THUMBBTNHORZ  2
  DrawThemeBkg_SBP_ThumbBtnVert,    //#define GXSBP_THUMBBTNVERT  3
  DrawThemeBkg_SBP_TrackHorz,      //#define GXSBP_LOWERTRACKHORZ  4
  DrawThemeBkg_SBP_TrackHorz,      //#define GXSBP_UPPERTRACKHORZ  5
  DrawThemeBkg_SBP_TrackVert,      //#define GXSBP_LOWERTRACKVERT  6
  DrawThemeBkg_SBP_TrackVert,      //#define GXSBP_UPPERTRACKVERT  7
  NULL,                //#define GXSBP_GRIPPERHORZ    8
  NULL,                //#define GXSBP_GRIPPERVERT    9
  NULL,                //#define GXSBP_SIZEBOX      10
};
#endif // _DEV_DISABLE_UI_CODE