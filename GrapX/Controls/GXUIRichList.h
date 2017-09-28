#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GXUI_LIST_CUSTOM_H_
#define _GXUI_LIST_CUSTOM_H_

// 觉得应该改名为 ListCabinet

namespace DlgXM
{
  struct DLGBASICPARAM;
}
namespace GXUI
{
  ////////////////////////////////////////////////////////////////////////////
  struct ITEMELEMENT   // item中的子窗口信息
  {
    clStringW strName;
    clStringW strClass;
    GXDWORD   dwStyle;
    GXDWORD   dwExStyle;
  };

  typedef clvector<ITEMELEMENT> ItemElementArray;
  
  class RichList : public List
  {
  public:
    typedef clvector<GXHWND>        GXHWndArray;
    typedef cllist<GXHWND>          WndHandleList;
  
  private:
    clStringW         m_strTemplate;
    GXHWND            m_hPrototype;
    clStringArrayW    m_aElementName;   // Item 中的控件名
    WndHandleList     m_HandlesPool;    // Windows 句柄池
    MOVariable        m_VarList;

  private:
    static GXLRESULT GXCALLBACK CustomWndProc         (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    static GXBOOL    GXCALLBACK EnumChildProc         (GXHWND hWnd, GXLPARAM lParam);
    static GXBOOL    GXCALLBACK EnumAdapterChildProc  (GXHWND hWnd, GXLPARAM lParam);

  private:
    GXLRESULT SetupAdapter  ();
    GXHWND    GetItemWnd    (GXSIZE_T item);
    GXHWND    CreateItemWnd (GXSIZE_T item);

  public:
    //GXINT     GetItemHeight       (GXINT nIdx) const;
    GXHWND    PlantCustItem       (GXSIZE_T nIndex, GXLPCRECT lprect);
    void      Recycle             (GXSIZE_T nIndex);
    int       Recycle             (GXINT nBegin, int nDir); // 包含 nBegin
    GXBOOL    UpdateCustItemText  (GXSIZE_T nIndex, GXLPCRECT rcItem);
    GXLRESULT SetItemTemplate     (GXLPCWSTR szTemplate);

    //virtual ListType  GetListType         ();
    virtual GXLRESULT Measure             (GXRegn* pRegn);
    virtual int       OnCreate            (GXCREATESTRUCT* pCreateParam);
    virtual GXLRESULT SetVariable         (MOVariable* pVariable);
    virtual GXINT     HitTest             (int fwKeys, int x, int y) const;
    virtual GXLRESULT OnPaint             (GXWndCanvas& canvas);
    //virtual GXINT     VirGetItemHeight    (GXINT nIdx) const;
    virtual int       OnSize              (int cx, int cy);
    //virtual GXHRESULT OnKnock             (KNOCKACTION* pAction);
    virtual GXVOID    OnImpulse           (LPCDATAIMPULSE pImpulse);
    //virtual GXBOOL    ReduceItemStat      (GXINT nCount);
    //virtual void      DeleteItemStat      (GXINT nIndex);
    virtual GXBOOL    GetItemRect(int nItem, GXDWORD dwStyle, GXLPRECT lprc) const;

  public:
    RichList(GXLPCWSTR szIdName);
  };
} // namespace GXUI
#endif // _GXUI_LIST_CUSTOM_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE