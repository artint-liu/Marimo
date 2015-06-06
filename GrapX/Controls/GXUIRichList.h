#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GXUI_LIST_CUSTOM_H_
#define _GXUI_LIST_CUSTOM_H_

// ����Ӧ�ø���Ϊ ListCabinet

namespace DlgXM
{
  struct DLGBASICPARAM;
}
namespace GXUI
{
  ////////////////////////////////////////////////////////////////////////////

  struct CUSTOMIZEITEMSTAT
  {
    GXINT   nBottom;
    GXHWND  hItem;
    GXDWORD bSelected : 1;
    GXDWORD bValidate : 1;  // Item����Ч��־, ���Ϊ0��ʾҪ���´�Adapter��ȡ
    CUSTOMIZEITEMSTAT() : nBottom(-1), hItem(NULL), bSelected(0), bValidate(0){}
  };

  struct ITEMELEMENT   // item�е��Ӵ�����Ϣ
  {
    clStringW strName;
    clStringW strClass;
    GXDWORD   dwStyle;
    GXDWORD   dwExStyle;
  };

  typedef clvector<ITEMELEMENT> ItemElementArray;
  
  //typedef clvector<CUSTOMIZEITEMSTAT>  CustItemStatArray;

  class RichList : public List
  {
  public:
    typedef clvector<GXHWND>        GXHWndArray;
    typedef cllist<GXHWND>          WndHandleList;
    //typedef clvector<ITEMELEMENT>   ItemElementArray;
  private:
    clStringW         m_strTemplate;
    GXHWND            m_hPrototype;
    clStringArrayW    m_aElementName;   // Item �еĿؼ���
    WndHandleList     m_HandlesPool;    // Windows �����
    //GXSIZE_T          m_FirstItem;      // m_ItemHandles��һ����Ӧ��item����
    //WndHandleList     m_ItemHandles;
    MOVariable        m_VarList;
  private:
    static GXLRESULT GXCALLBACK CustomWndProc         (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    static GXBOOL    GXCALLBACK EnumChildProc         (GXHWND hWnd, GXLPARAM lParam);
    static GXBOOL    GXCALLBACK EnumAdapterChildProc  (GXHWND hWnd, GXLPARAM lParam);

  private:
    GXLRESULT SetupAdapter  ();
    GXHWND    GetItemWnd    (int item);
    GXHWND    CreateItemWnd (int item);
  public:
    GXINT     GetItemHeight       (GXINT nIdx) const;
    GXHWND    PlantCustItem       (int nIndex, GXLPCRECT lprect);
    void      Recycle             (int nIndex);
    int       Recycle             (int nBegin, int nDir); // ���� nBegin
    GXBOOL    UpdateCustItemText  (int nIndex, GXLPCRECT rcItem);
    GXLRESULT SetItemTemplate     (GXLPCWSTR szTemplate);

    virtual ListType  GetListType         ();
    virtual GXLRESULT Measure             (GXRegn* pRegn);
    virtual int       OnCreate            (GXCREATESTRUCT* pCreateParam);
    virtual GXLRESULT SetVariable         (MOVariable* pVariable);
    virtual GXINT     HitTest             (int fwKeys, int x, int y) const;
    virtual GXLRESULT OnPaint             (GXWndCanvas& canvas);
    virtual GXINT     VirGetItemHeight    (GXINT nIdx) const;
    virtual int       OnSize              (int cx, int cy);
    //virtual GXHRESULT OnKnock             (KNOCKACTION* pAction);
    virtual GXVOID    OnImpulse           (LPCDATAIMPULSE pImpulse);
    virtual GXBOOL    ReduceItemStat      (GXINT nCount);
    virtual void      DeleteItemStat      (GXINT nIndex);
    virtual GXBOOL    GetItemRect(int nItem, GXDWORD dwStyle, GXLPRECT lprc) const;

  public:
    RichList(GXLPCWSTR szIdName);
  };
} // namespace GXUI
#endif // _GXUI_LIST_CUSTOM_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE