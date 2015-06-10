#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GXUI_LIST_SIMPLE_H_
#define _GXUI_LIST_SIMPLE_H_

#define SCROLLBAR_WIDTH 10
#define CHECKBOX_SIZE 10

namespace DlgXM
{
  struct DLGBASICPARAM;
}
namespace GXUI
{
  class IDataPool;
  class StringArrayDataPool;
  


  class SimpleList : public List
  {
  public:
    //typedef clvector<ITEMSTAT>  ItemStatArray;
    typedef clvector<GXUINT>    IntArray;
    IntArray      m_aColumns;
  protected:
    virtual GXLRESULT            OnPaint        (GXWndCanvas& canvas);

    GXINT     AddStringW          (GXLPCWSTR lpString);
    GXBOOL    CheckEndScrollAnim  (GXUINT nIDTimer, bool bForced);
    GXINT     HitTest             (int fwKeys, int x, int y) const;
    //GXINT     GetItemHeight       (GXINT nIdx) const;
    GXBOOL    IsItemSelected      (GXINT nItem) const;
    GXBOOL    SelectItem          (GXINT nItem, GXBOOL bSelected);
    GXINT     DrawTextWithColumnsW(GXWndCanvas& canvas, GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount,
                                    GXLPRECT lpRect,GXUINT uFormat, GXCOLORREF crText, GXCOLORREF crContrast);

    virtual GXUINT    SetColumnsWidth     (GXLPCWSTR szString);
    virtual GXUINT    SetColumnsWidth     (const GXUINT* pColumn, GXUINT nCount);
    virtual GXUINT    GetColumnsWidth     (GXUINT* pColumn, GXUINT nCount);

    virtual int       OnCreate            (GXCREATESTRUCT* pCreateParam);
    virtual GXLRESULT SetVariable         (MOVariable* pVariable);
    //virtual ListType  GetListType         ();
    //virtual GXINT     VirGetItemHeight    (GXINT nIdx) const;
    virtual int       OnSize              (int cx, int cy);
    //virtual int       OnLButtonDown       (int fwKeys, int x, int y);
    virtual GXBOOL    GetItemRect         (int nItem, GXDWORD dwStyle, GXLPRECT lprc) const;
    virtual GXLRESULT SetItemTemplate     (GXLPCWSTR szTemplate);

  public:
    SimpleList(GXLPCWSTR szIdName);
    virtual GXLRESULT Measure(GXRegn* pRegn);
  public:
    GXBOOL SetSpriteModule(GXLPCWSTR szSpriteFile, GXLPCWSTR szNormal, GXLPCWSTR szHover, GXLPCWSTR szPressed, GXLPCWSTR szDisabled, GXLPCWSTR szDefault);
  };
  //////////////////////////////////////////////////////////////////////////
} // namespace GXUI
#endif // _GXUI_LIST_SIMPLE_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE