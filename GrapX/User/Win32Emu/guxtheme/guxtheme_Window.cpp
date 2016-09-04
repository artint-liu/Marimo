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

DRAWTHEMEBKG(DrawThemeBkg_WP_Caption)
{
  //GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(GXCanvas::CM_SourceCopy);
  s_pUxThemeSprite_Window->PaintModule3H(/*GXGDI_DC_PTR(hdc)->*/pCanvas, 0, 
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}
DRAWTHEMEBKG(DrawThemeBkg_WP_FrameLeft)
{
  //GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(GXCanvas::CM_SourceCopy);
  s_pUxThemeSprite_Window->PaintModule(/*GXGDI_DC_PTR(hdc)->*/pCanvas, 3, 
    pRect->left, pRect->top, pRect->right, pRect->bottom);
  return 1L;
}
DRAWTHEMEBKG(DrawThemeBkg_WP_FrameRight)
{
  //GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(GXCanvas::CM_SourceCopy);
  s_pUxThemeSprite_Window->PaintModule(/*GXGDI_DC_PTR(hdc)->*/pCanvas, 5, 
    pRect->left, pRect->top, pRect->right, pRect->bottom);
  return 1L;
}
DRAWTHEMEBKG(DrawThemeBkg_WP_FrameBottom)
{
  //GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(GXCanvas::CM_SourceCopy);
  s_pUxThemeSprite_Window->PaintModule3H(/*GXGDI_DC_PTR(hdc)->*/pCanvas, 6, 
    pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
  return 1L;
}
DRAWTHEMEBKG(DrawThemeBkg_WP_Dialog)
{
  //LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  //lpDC->pCanvas->SetCompositingMode(GXCanvas::CM_SourceCopy);
  s_pUxThemeSprite_Window->PaintModule(/*GXGDI_DC_PTR(hdc)->*/pCanvas, 4, 
    pRect->left, pRect->top, pRect->right, pRect->bottom);
  return 1L;
}

DrawThemeBkg DRB_WP_Table[] =
{
  (DrawThemeBkg)37,
  DrawThemeBkg_WP_Caption,  //#define GXWP_CAPTION              1
  NULL,            //#define GXWP_SMALLCAPTION            2
  NULL,            //#define GXWP_MINCAPTION            3
  NULL,            //#define GXWP_SMALLMINCAPTION          4
  NULL,            //#define GXWP_MAXCAPTION            5
  NULL,            //#define GXWP_SMALLMAXCAPTION          6
  DrawThemeBkg_WP_FrameLeft,  //#define GXWP_FRAMELEFT            7
  DrawThemeBkg_WP_FrameRight, //#define GXWP_FRAMERIGHT            8
  DrawThemeBkg_WP_FrameBottom,//#define GXWP_FRAMEBOTTOM            9
  NULL,            //#define GXWP_SMALLFRAMELEFT          10
  NULL,            //#define GXWP_SMALLFRAMERIGHT          11
  NULL,            //#define GXWP_SMALLFRAMEBOTTOM          12
  NULL,            //#define GXWP_SYSBUTTON            13
  NULL,            //#define GXWP_MDISYSBUTTON            14
  NULL,            //#define GXWP_MINBUTTON            15
  NULL,            //#define GXWP_MDIMINBUTTON            16
  NULL,            //#define GXWP_MAXBUTTON            17
  NULL,            //#define GXWP_CLOSEBUTTON            18
  NULL,            //#define GXWP_SMALLCLOSEBUTTON          19
  NULL,            //#define GXWP_MDICLOSEBUTTON          20
  NULL,            //#define GXWP_RESTOREBUTTON          21
  NULL,            //#define GXWP_MDIRESTOREBUTTON          22
  NULL,            //#define GXWP_HELPBUTTON            23
  NULL,            //#define GXWP_MDIHELPBUTTON          24
  NULL,            //#define GXWP_HORZSCROLL            25
  NULL,            //#define GXWP_HORZTHUMB            26
  NULL,            //#define GXWP_VERTSCROLL            27
  NULL,            //#define GXWP_VERTTHUMB            28
  DrawThemeBkg_WP_Dialog,    //#define GXWP_DIALOG              29
  NULL,            //#define GXWP_CAPTIONSIZINGTEMPLATE      30
  NULL,            //#define GXWP_SMALLCAPTIONSIZINGTEMPLATE    31
  NULL,            //#define GXWP_FRAMELEFTSIZINGTEMPLATE      32
  NULL,            //#define GXWP_SMALLFRAMELEFTSIZINGTEMPLATE    33
  NULL,            //#define GXWP_FRAMERIGHTSIZINGTEMPLATE      34
  NULL,            //#define GXWP_SMALLFRAMERIGHTSIZINGTEMPLATE  35
  NULL,            //#define GXWP_FRAMEBOTTOMSIZINGTEMPLATE    36
  NULL,            //#define GXWP_SMALLFRAMEBOTTOMSIZINGTEMPLATE  37
};
#endif // _DEV_DISABLE_UI_CODE