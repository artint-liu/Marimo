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
#include <User/Win32Emu/guxtheme/Button_ID_BP.H>

static unsigned long s_SpriteIdxMap_PBS[] =
{
  5,
  ID_BUTTON_PBS_NORMAL,  //#define    GXPBS_NORMAL 1
  ID_BUTTON_PBS_HOT,    //#define    GXPBS_HOT 2
  ID_BUTTON_PBS_PRESSED,  //#define    GXPBS_PRESSED 3
  ID_BUTTON_PBS_DISABLED,  //#define    GXPBS_DISABLED 4
  ID_BUTTON_PBS_DEFAULTED,//#define    GXPBS_DEFAULTED 5
};

DRAWTHEMEBKG(DrawThemeBkg_BP_PushButton)
{
  ASSERT((unsigned long)iStateId <= s_SpriteIdxMap_PBS[0]);
  s_pUxThemeSprite_Button->PaintModule3x3(/*GXGDI_DC_PTR(hdc)->*/pCanvas,
    s_SpriteIdxMap_PBS[iStateId], FALSE, pRect);
  return 1L;
}
//DRAWTHEMEBKG(DrawThemeBkg_WP_FrameLeft)
//{
//  s_pUxThemeSprite_Window->ApplyState(0);
//  s_pUxThemeSprite_Window->Paint(GXGDI_DC_PTR(hdc)->pCanvas, 3, 
//    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
//  return 1L;
//}
//DRAWTHEMEBKG(DrawThemeBkg_WP_FrameRight)
//{
//  s_pUxThemeSprite_Window->ApplyState(0);
//  s_pUxThemeSprite_Window->Paint(GXGDI_DC_PTR(hdc)->pCanvas, 5, 
//    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
//  return 1L;
//}
//DRAWTHEMEBKG(DrawThemeBkg_WP_FrameBottom)
//{
//  s_pUxThemeSprite_Window->ApplyState(0);
//  s_pUxThemeSprite_Window->Paint3H(GXGDI_DC_PTR(hdc)->pCanvas, 6, 
//    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
//  return 1L;
//}
//DRAWTHEMEBKG(DrawThemeBkg_WP_Dialog)
//{
//  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
//  //lpDC->pCanvas->FillRect(pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, 0xffffffff);
//  //gxRectangle(hdc, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
//  s_pUxThemeSprite_Window->ApplyState(0);
//  s_pUxThemeSprite_Window->Paint(GXGDI_DC_PTR(hdc)->pCanvas, 4, 
//    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
//  return 1L;
//}



DrawThemeBkg DRB_BP_Table[] =
{
  (DrawThemeBkg)5,
  DrawThemeBkg_BP_PushButton,  // #define GXBP_PUSHBUTTON  1
  NULL,            // #define GXBP_RADIOBUTTON  2
  NULL,            // #define GXBP_CHECKBOX  3
  NULL,            // #define GXBP_GROUPBOX  4
  NULL,            // #define GXBP_USERBUTTON  5
};
#endif // _DEV_DISABLE_UI_CODE