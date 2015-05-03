#ifndef _DEV_DISABLE_UI_CODE
// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
#include <Include/GUnknown.H>
#include <Include/GResource.H>
#include <Include/GXGraphics.H>
#include <Include/GXSprite.H>

// ˽��ͷ�ļ�
#include <User/GXWindow.h>

#define FRAME_TOP_HEIGHT      35
#define FRAME_TOP_SPLIT_LEFT    32  // ��������
#define FRAME_TOP_SPLIT_RIGHT    119  // ��������

#define FRAME_BOTTOM_HEIGHT      21
#define FRAME_BOTTOM_SPLIT_LEFT    32  // ��������
#define FRAME_BOTTOM_SPLIT_RIGHT  32  // ��������

#define SPRITE_COMMON_TOPLEFT  0
#define SPRITE_COMMON_TOP    1
#define SPRITE_COMMON_TOPRIGHT  2

#define SPRITE_COMMON_LEFT    3
#define SPRITE_COMMON_RIGHT    4

#define SPRITE_COMMON_BOTTOMLEFT  5
#define SPRITE_COMMON_BOTTOM    6
#define SPRITE_COMMON_BOTTOMRIGHT  7


GXHRESULT _gxInitializeCommonSprite(GXGraphics* pGraphics)
{
  static GXLPCWSTR pwszCommonSpr[] = 
  {
    L"Vista\\AeroCommon.tga",
  };

  static REGN arrayAeroCommon[] = {
    // [0-7] �����Frame
    {0,0,32,35},{32,0,173,35},{205,0,119,35},    // ����������
    {0,36,18,218},{304,36,20,218},          // ���ұ߿�
    {0,255,32,21},{32,255,260,21},{292,255,32,21},  // ��������
    // [8-15]�Ǽ���״̬��Frame
    {324+0,0,32,35},{324+32,0,173,35},{324+205,0,119,35},    // ����������
    {324+0,36,18,218},{324+304,36,20,218},            // ���ұ߿�
    {324+0,254,32,21},{324+32,255,260,21},{324+292,255,32,21},  // ��������
    // [16-19]��С��
    {20,37,26,15},{46,37,26,15},{72,37,26,15},{98,37,26,15},
    // [20-23]���
    {20,52,26,15},{46,52,26,15},{72,52,26,15},{98,52,26,15},
    // [24-27]��ԭ
    {20,67,26,15},{46,67,26,15},{72,67,26,15},{98,67,26,15},
    // [28-31]�ر�
    {20,82,43,15},{63,82,43,15},{106,82,43,15},{149,82,43,15},
    // [32]��������
    {172,68,8,8},
    // [33]��С������󻯺ͻ�ԭ���ⷢ��
    {124,37,48,36},
    // [34]�رհ�ť���ⷢ��
    {172,37,59,31},
    // [35-46]CheckBox��ͼ��
    {20,97,13,13},{20,110,13,13},{20,123,13,13},{20,136,13,13},
    {20,149,13,13},{20,162,13,13},{20,175,13,13},{20,188,13,13},
    {20,201,13,13},{20,214,13,13},{20,227,13,13},{20,240,13,13},
    // [47-54]RadioButton��ͼ��
    {33,97,13,13},{33,110,13,13},{33,123,13,13},{33,136,13,13},
    {33,201,13,13},{33,162,13,13},{33,175,13,13},{33,188,13,13},
    // [55-63]����״̬�µ�Button
    {46,97,4,4},{50,97,20,4},{70,97,4,4},
    {46,101,4,15},{50,101,20,15},{70,101,4,15},
    {46,116,4,4},{50,116,20,4},{70,116,4,4},
    // [64-72]��������������Button
    {46+28,97,4,4},{50+28,97,20,4},{70+28,97,4,4},
    {46+28,101,4,15},{50+28,101,20,15},{70+28,101,4,15},
    {46+28,116,4,4},{50+28,116,20,4},{70+28,116,4,4},
    // [73-81]Disable��Button
    {46+56,97,4,4},{50+56,97,20,4},{70+56,97,4,4},
    {46+56,101,4,15},{50+56,101,20,15},{70+56,101,4,15},
    {46+56,116,4,4},{50+56,116,20,4},{70+56,116,4,4},
    // [82-90]��갴�µ�Button
    {46+84,97,4,4},{50+84,97,20,4},{70+84,97,4,4},
    {46+84,101,4,15},{50+84,101,20,15},{70+84,101,4,15},
    {46+84,116,4,4},{50+84,116,20,4},{70+84,116,4,4},
    // [91-99]��갴�µ�Button
    {46+112,97,4,4},{50+112,97,20,4},{70+112,97,4,4},
    {46+112,101,4,15},{50+112,101,20,15},{70+112,101,4,15},
    {46+112,116,4,4},{50+112,116,20,4},{70+112,116,4,4},
    // [100-107] GroupBox �õ��Ǹ����
    {231,37,6,4},{237,37,10,4},{247,37,6,4},
    {231,41,3,12},{250,41,3,12},
    {231,53,6,4},{237,53,10,4},{247,53,6,4},
    // [108-115]��������С��ť ��-��
    {47 + 17 * 0,122,17,16},{47 + 17 * 1,122,17,16},{47 + 17 * 2,122,17,16},{47 + 17 * 3,122,17,16},
    {47 + 17 * 4,122,17,16},{47 + 17 * 5,122,17,16},{47 + 17 * 6,122,17,16},{47 + 17 * 7,122,17,16},
    // [116-123]��������С��ť ��-��
    {48 + 17 * 0,138,16,17},{48 + 17 * 1,138,16,17},{48 + 17 * 2,138,16,17},{48 + 17 * 3,138,16,17},
    {48 + 17 * 4,138,16,17},{48 + 17 * 5,138,16,17},{48 + 17 * 6,138,16,17},{48 + 17 * 7,138,16,17},
    // [124-127]�������Ŀհ��� ��-��
    {47 + 17 * 0,156,17,16},{47 + 17 * 1,156,17,16},
    {48 + 17 * 2,155,16,17},{48 + 17 * 3,155,16,17},
    // [128-136]ˮƽ����������
    {116 + 22 * 0,156,4,16},{116 + 22 * 0 + 4,156,14,16},{116 + 22 * 0 + 18,156,4,16},
    {116 + 22 * 1,156,4,16},{116 + 22 * 1 + 4,156,14,16},{116 + 22 * 1 + 18,156,4,16},
    {116 + 22 * 2,156,4,16},{116 + 22 * 2 + 4,156,14,16},{116 + 22 * 2 + 18,156,4,16},
    // [137-145]��ֱ����������
    {48 + 17 * 0,173,16,3},{48 + 17 * 0,173 + 3,16,14},{48 + 17 * 0,173 + 17,16,3},
    {48 + 17 * 1,173,16,3},{48 + 17 * 1,173 + 3,16,14},{48 + 17 * 1,173 + 17,16,3},
    {48 + 17 * 2,173,16,3},{48 + 17 * 2,173 + 3,16,14},{48 + 17 * 2,173 + 17,16,3},
  };

  GXCreateSpriteFromFileW(pGraphics, L"AeroCommon.GSprite", &GXWnd::s_pCommonSpr);
  return GX_OK;
}
//////////////////////////////////////////////////////////////////////////
GXHRESULT _gxReleaseCommonSprite()
{
  SAFE_RELEASE(GXWnd::s_pCommonSpr);
  return GX_OK;
}
//////////////////////////////////////////////////////////////////////////
// ��ô���������������ĳߴ�
// ��������ĳߴ磬��ô����������򣬰����ⷢ������򣬲���������ͬ
GXVOID GXDLLAPI GXGetOverlappedRect(GXLPCRECT GXIN lpSrc, GXLPRECT GXOUT lpOverlappedRect)
{
  lpOverlappedRect->left    = lpSrc->left   /*- FRAME_NC_GLOW_LEFT*/;
  lpOverlappedRect->top     = lpSrc->top   /*- FRAME_NC_GLOW_TOP*/;
  lpOverlappedRect->right   = lpSrc->right   /*+ FRAME_NC_GLOW_RIGHT*/;
  lpOverlappedRect->bottom  = lpSrc->bottom /*+ FRAME_NC_GLOW_BOTTOM*/;
}
// ��ø�������
// �� gxGetOverlappedRect ��ͬ��������������width��height��������right��
// bottom��λ��
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