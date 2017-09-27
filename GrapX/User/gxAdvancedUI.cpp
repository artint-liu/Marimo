#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXSprite.H>

// 私有头文件
#include <User/GXWindow.h>

#define FRAME_TOP_HEIGHT      35
#define FRAME_TOP_SPLIT_LEFT    32  // 从左向右
#define FRAME_TOP_SPLIT_RIGHT    119  // 从右向左

#define FRAME_BOTTOM_HEIGHT      21
#define FRAME_BOTTOM_SPLIT_LEFT    32  // 从左向右
#define FRAME_BOTTOM_SPLIT_RIGHT  32  // 从右向左

#define SPRITE_COMMON_TOPLEFT  0
#define SPRITE_COMMON_TOP    1
#define SPRITE_COMMON_TOPRIGHT  2

#define SPRITE_COMMON_LEFT    3
#define SPRITE_COMMON_RIGHT    4

#define SPRITE_COMMON_BOTTOMLEFT  5
#define SPRITE_COMMON_BOTTOM    6
#define SPRITE_COMMON_BOTTOMRIGHT  7


//GXHRESULT _gxInitializeCommonSprite(GXGraphics* pGraphics)
//{
//  GXCreateSpriteFromFileW(pGraphics, L"elements/common.stock", &GXWnd::s_pCommonSpr);
//  return GX_OK;
//}
//////////////////////////////////////////////////////////////////////////
//GXHRESULT _gxReleaseCommonSprite()
//{
//  SAFE_RELEASE(GXWnd::s_pCommonSpr);
//  return GX_OK;
//}
//////////////////////////////////////////////////////////////////////////
// 获得窗口整个覆盖区域的尺寸
// 根据输入的尺寸，获得窗口整个区域，包括外发光的区域，参数可以相同
GXVOID GXDLLAPI GXGetOverlappedRect(GXLPCRECT GXIN lpSrc, GXLPRECT GXOUT lpOverlappedRect)
{
  lpOverlappedRect->left    = lpSrc->left   /*- FRAME_NC_GLOW_LEFT*/;
  lpOverlappedRect->top     = lpSrc->top   /*- FRAME_NC_GLOW_TOP*/;
  lpOverlappedRect->right   = lpSrc->right   /*+ FRAME_NC_GLOW_RIGHT*/;
  lpOverlappedRect->bottom  = lpSrc->bottom /*+ FRAME_NC_GLOW_BOTTOM*/;
}
// 获得覆盖区域
// 与 gxGetOverlappedRect 不同的是这里计算的是width和height，而不是right和
// bottom的位置
//GXVOID GXDLLAPI GXGetOverlappedRegion(LPCREGN GXIN lpSrcRegn, LPREGN GXOUT lpOverlappedRegn)
//{
//  lpOverlappedRegn->left   = lpSrcRegn->left  /*- FRAME_NC_GLOW_LEFT*/;
//  lpOverlappedRegn->top   = lpSrcRegn->top  /*- FRAME_NC_GLOW_TOP*/;
//  lpOverlappedRegn->width   = lpSrcRegn->width  /*+ (FRAME_NC_GLOW_LEFT + FRAME_NC_GLOW_RIGHT )*/;
//  lpOverlappedRegn->height = lpSrcRegn->height /*+ (FRAME_NC_GLOW_TOP  + FRAME_NC_GLOW_BOTTOM)*/;
//}

//////////////////////////////////////////////////////////////////////////


#define SPRITE_BUTTON_TOPLEFT     55
#define SPRITE_BUTTON_TOP         56
#define SPRITE_BUTTON_TOPRIGHT    57
#define SPRITE_BUTTON_LEFT        58
#define SPRITE_BUTTON_MIDDLE      59
#define SPRITE_BUTTON_RIGHT       60
#define SPRITE_BUTTON_BOTTOMLEFT  61
#define SPRITE_BUTTON_BOTTOM      62
#define SPRITE_BUTTON_BOTTOMRIGHT 63



#define SPRITE_GROUPBOX_TOPLEFT     100
#define SPRITE_GROUPBOX_TOP         101
#define SPRITE_GROUPBOX_TOPRIGHT    102
#define SPRITE_GROUPBOX_LEFT        103
#define SPRITE_GROUPBOX_MIDDLE      -1
#define SPRITE_GROUPBOX_RIGHT       104
#define SPRITE_GROUPBOX_BOTTOMLEFT  105
#define SPRITE_GROUPBOX_BOTTOM      106
#define SPRITE_GROUPBOX_BOTTOMRIGHT 107

#endif // _DEV_DISABLE_UI_CODE