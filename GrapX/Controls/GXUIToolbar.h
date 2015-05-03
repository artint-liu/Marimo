#ifndef _GXUI_TOOLBAR_H_
#define _GXUI_TOOLBAR_H_

namespace GXUI
{
  class Toolbar : public CtrlBase
  {
  public:
    struct TOOLBARBUTTON
    {
      int         nSprite;
      GXLPCWSTR   idCommand;
      GXBYTE      fsState;    // 支持 TBSTATE_CHECKED, TBSTATE_HIDDEN
      GXBYTE      fsStyle;    // 支持 BTNS_SEP, BTNS_CHECK, BTNS_GROUP, BTNS_SHOWTEXT
      GXWORD      wLeft;
      GXWORD      wWidth;
      GXDWORD_PTR dwData;
      clStringW   strText;

      //int GetWidth();
    };
    typedef clvector<TOOLBARBUTTON> ToolBtnArray;

  protected:
    GXSprite*     m_pSprite;
    ToolBtnArray  m_aButtons;
    GXSIZE        m_ButtonSize;
    GXSIZE        m_BitmapSize;
    int           m_nMaxBtnWidth;   // 如果显示文字的话, 这个宽度限制按钮的最大宽度
    int           m_nCurItem;
    //int           m_nPressdItem;
    //GXCOLOR       m_crHover;
    GXColor32     m_crHover;
    GXColor32     m_crPressd;
    GXColor32     m_crChecked;
    GXColor32     m_crText;
    GXHWND        m_hToolTip;

    // 将来会弃用这个
    GXDWORD       m_bIndexedCommand : 1;  // GXWM_COMAND 发送的是ButtonId还是ButtonIndex,
                                          // 如果ToolbarButton的id是字符串,则必定是IndexedCommand

    //int       m_nNormal;
    //int       m_nHover;
    //int       m_nPressed;
    //int       m_nDisabled;
    //int       m_nDefault;
    //GXDWORD   m_dwState;
  protected:
    static  GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    virtual GXLRESULT            OnPaint        (GXWndCanvas& canvas);
    virtual GXLRESULT            Destroy        ();
    virtual GXBOOL               SolveDefinition (const GXDefinitionArrayW& aDefinitions);

    int       GetBtnWidth       (GXWndCanvas* pCanvas, const TOOLBARBUTTON* pTBBtn);
    GXLPCWSTR GetIdNameW        (GXINT Id);
    int       IntOnMouseMove    (int nFocusItem, int x, int y);  // 只处理按钮按下的情况
    int       IntOnLButtonUp    (int nFocusItem);
    void      InitToolTips      (GXHWND hWnd);
  public:
    Toolbar(GXLPCWSTR szIdName);
    static Toolbar* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static Toolbar* Create  (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
    virtual GXLRESULT Measure(GXRegn* pRegn);
  public:
    //GXHRESULT      SetFont (LPCWSTR szFontName, int nFontSize);
    GXBOOL SetSpriteFile(GXLPCWSTR szSpriteFile);
    GXBOOL SetSpriteObj(GXSprite* pSprite);
    GXINT AddButton(const GXTBBUTTON* pTBButton);
    GXBOOL HideButton(GXINT_PTR idButton, GXBOOL bHide);

    void  GetItemRect         (ToolBtnArray::iterator it, GXRECT* rect);
    void  GetItemRect         (int nBtn, GXRECT* rect);
    int   HitTestItem         (int x, int y);
    int   OnMouseMove         (int fwKeys, int x, int y);
    int   OnLButtonDown       (int fwKeys, int x, int y);
    //int   OnLButtonUp         (int fwKeys, int x, int y);
    void  ClearItem           ();
    void  AutoSize            ();
    void  DrawItem            (GXWndCanvas& canvas, int nItem, GXColor32 clr, int ox, int oy, int& x, int& y);
    void  DrawItem            (GXWndCanvas& canvas, int nItem, GXColor32 clr, GXBOOL bErase);
    int   GetButtonIndex      (GXINT_PTR idCommand) const;
    int   GetCheckedGroupButton(int idCommand) const;
    int   GetCheckedGroupButtonByIndex(int nIndex) const;
    GXVOID CalcButtonSize     (int nStart);
    GXBOOL SetButtonSize      (int cx, int cy);
    GXBOOL SetBitmapSize      (int cx, int cy);
    GXBOOL GetButtonInfo      (GXINT_PTR idCommand, GXTBBUTTONINFOW* pInfo) const;
    GXBOOL SetButtonInfo      (GXINT_PTR idCommand, const GXTBBUTTONINFOW* pInfo);
    GXBOOL IsButtonChecked    (int idCommand);
    GXBOOL CheckButton        (int idCommand, GXBOOL fCheck);
    GXBOOL CheckButtonByIndex (int nIndex, GXBOOL fCheck);
  };

  class ToolbarObjRef
  {
  private:
    Toolbar* m_pToolbar;

  public:
    ToolbarObjRef(Toolbar* pToolbar) 
      : m_pToolbar(pToolbar)
    {
      // WM_NCCREATE 时是NULL
      if(pToolbar) {
        pToolbar->Lock();
      }
    }

    ~ToolbarObjRef()
    {
      if(m_pToolbar) {
        m_pToolbar->Unlock();
      }
    }

    Toolbar* operator->()
    {
      return m_pToolbar;
    }

    operator CtrlBase*()
    {
      return m_pToolbar;
    }
  };
} // namespace GXUI

#endif // _GXUI_TOOLBAR_H_