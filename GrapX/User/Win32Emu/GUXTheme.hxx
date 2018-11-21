#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GX_THEME_INTERNAL_H_
#define _GX_THEME_INTERNAL_H_

#define DRAWTHEMEBKG(FUNNAME)  GXHRESULT FUNNAME(GrapX::GXCanvas* pCanvas, int iStateId, GXLPCRECT pRect, GXLPCRECT pClipRect)
typedef DRAWTHEMEBKG((*DrawThemeBkg));

typedef struct _tagGXTHEME
{
  DrawThemeBkg  *pDrawThemeBkgFunc;
}GXTHEME, *LPGXTHEME, *GXLPTHEME;

#define GXTHEME_PTR(_HTHEME)  ((LPGXTHEME)_HTHEME)
#define GXTHEME_HANDLE(_LPTHEME)  ((GXHTHEME)_LPTHEME)

extern GXSprite *s_pUxThemeSprite_Window;
extern GXSprite *s_pUxThemeSprite_ScrollBar;
extern GXSprite *s_pUxThemeSprite_Button;

extern DrawThemeBkg DRB_WP_Table[];  // Window Part
extern DrawThemeBkg DRB_SBP_Table[];  // ScrollBar Part
extern DrawThemeBkg DRB_BP_Table[];  // Button Part

#endif // _GX_THEME_INTERNAL_H_
#endif // _DEV_DISABLE_UI_CODE