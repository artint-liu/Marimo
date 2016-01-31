#ifndef _UXEXT_PROPERTYLIST_H_
#define _UXEXT_PROPERTYLIST_H_

// 因为PropertySheet和Win32控件重名并且功能不同，所以改名为PropertyList

#define GXWM_DESTROYEDITCTL  GXWM_USER + 100
class GXImage;

namespace GXUIEXT
{
  namespace PropertyList
  {
    class Form;
    class Page;
    GXBOOL DrawUpDownControl(GXHDC hdc, GXLPRECT lpRect, int nBtnState);

    struct LISTBOXITEM
    {
      clStringW strName;
      GXLPARAM  lParam;
    };
    typedef clvector<LISTBOXITEM> LBItemList;


    struct ITEM
    {
      typedef PROPERTYSHEETTYPE TYPE;
      enum HITTEST
      {
        HT_ERROR,
        HT_NAME,
        HT_BLANK,
        HT_BOOL_CHECKBOX,   // T_BOOLEAN 的 CheckBox
        HT_PAGE_CHILD,      // T_PROPSHEETPAGE 的切换子页按钮
        HT_LIST_SELECT,
        HT_BUTTON,
        HT_COLOR,
        HT_EDIT,
        HT_SPINUP,
        HT_SPINDOWN,
        HT_SPINTRACK,
      };
      enum FLAGS : GXDWORD
      {
        F_DISABLE = 0x00008000,
        F_VISIBLE = 0x80000000,
      };
      TYPE       eType;
      GXDWORD    dwStyle;
      GXDWORD    dwId;
      GXUINT     nHeight;
      clStringW  strName;
      union
      {
        GXBOOL      bVal;
        GXINT       nVal;
        float       fVal;
        GXCOLORREF  crVal;
        GXHWND      hDlg;
      };
      clStringW     strVal;

      union
      {
        Page*         pChildPage;
        LBItemList*   pListBoxItem;
        GXImage*      pImage;
        GXLPVOID      pGeneral;    // 通用的,只用于指针判断
        float         fIncrease;
      };
      ITEM      ();
      //ITEM      (const ITEM& I);
      GXVOID  DrawAsButton    (GXHDC hdc, GXRECT* pRect, GXBOOL bPrushed) const;
      GXVOID  DrawAsCheckBox  (GXHDC hdc, GXRECT* pRect, GXUINT uState) const;
      GXVOID  DrawAsNumber    (GXHDC hdc, GXRECT* pRect, int nBtnState) const;
      HITTEST HitTest         (GXRECT* rcItem, GXINT nSplit, GXPOINT* ptHit);
      GXBOOL  EnableItem      (GXBOOL bEnable);
      GXBOOL  ShowItem        (GXBOOL bShow);
    };

    typedef clvector<ITEM>        ItemList;


    class Page
    {
      friend class Form;
    private:
      Form*       m_pPropSheet;
      Page*       m_pParent;
      ItemList    m_aItemList;
      GXINT       m_nTotalItemHeight;    // 全部Item的高度
      GXINT       m_nTopIndex;
      GXINT       m_nSplit;

    public:
      Page    (GXINT nSplit, Page* pParent = NULL);
      Page    (Form* pPropSheet, GXINT nSplit, Page* pParent = NULL);

      GXBOOL          Initialize        (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink);
      GXVOID          Release           ();

      GXBOOL          UploadData        (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast);
      GXBOOL          DownloadData      (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast);

      GXBOOL          UpdateHeightList  (GXHDC hdc);
      GXBOOL          UpdateHeightList  ();
      void            GetItemRect       (int nItem, GXRECT* rect) const;
      void            DrawItems         (GXHDC hdc) const;
      int             HitTest           (int xPos, int yPos);
      ItemList&       GetList           () { return m_aItemList; }
      GXINT           GetTotalItemHeight() const;
      Page*           GetParent         () const;
      GXINT           GetTopIndex       () const;
      GXINT           GetSplit          () const;
      GXINT           SetTopIndex       (INT nTopIndex);
      GXINT           FindByDlgHandle   (GXHWND hDlg) const;
      GXINT           FindById          (GXINT nStart, GXINT nId) const;
      ITEM*           FindItemById      (GXINT nId);
      Page*           FindPageById      (GXINT nId);
      GXINT           SetSplit          (GXINT nSplit);
    }; // class PropSheetPage

    class Form : public GXUI::CtrlBase
    {
      friend class Page;
    public:
      typedef clvector<GXLPVOID>    PtrArray;


    private:
      GXHBRUSH        m_hBrush[4];
      GXHPEN          m_hPen;
      GXHFONT         m_hFont;
      PtrArray        m_aBastPtr;
      Page            m_Root;
      Page*           m_pCurPage;
      GXPOINT         m_ptLButton;
      GXDWORD         m_fwKeys;
      GXINT           m_nPrevTopIndex;    // 左键单击时的m_uTopIndex值

      GXHWND          m_hEdit;
      GXINT           m_nEditing;         // 正在编辑的Item
      GXDWORD_PTR     m_dwOldEditWndProc;
      ITEM::HITTEST   m_eLBDown;
      GXHWND          m_hListBox;
      GXBOOL          m_bChangedNumber;   // 自从按下按键后是否改变了值  TODO: 考虑是否和m_SpinBtnDown合并
      GXBOOL          m_bShowScorollBar;
      int             m_xLeft;
      union
      {
        INT      nVal;
        float    fVal;
      }m_SpinBtnDown;    // Float/Int 控件左键按下时记录的初始值
    private:
      //static GXDWORD              RegisterClass       (GXHINSTANCE hInst);
      static GXLRESULT GXCALLBACK WndProc             (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
      static GXLRESULT GXCALLBACK EditStringWndProc   (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
      static GXLRESULT GXCALLBACK EditIntegerWndProc  (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
      static GXLRESULT GXCALLBACK EditFloatWndProc    (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
      static GXBOOL               Build               (LBItemList& aLBItem, const PROPSHEET_LISTBOX* pListBox);
      static GXINT                GetMaxPtrIndex      (const PROPLIST_DATALINK* pDataLayout);
      
      GXBOOL        PopupEditLabel    (int nItem, GXLPCRECT prc, GXLPCPOINT pt);  // 弹出Item用户输入编辑控件
      GXBOOL        PopupNextEditLabel(int nDir); // nDir查找防线，向上(-1)或是向下(1)
      GXBOOL        SubmitString      (GXBOOL bDelay);
      GXBOOL        SubmitList        (GXBOOL bDelay);
      GXBOOL        DestroyEditCtrl   ();
      GXBOOL        DestroyListCtrl   ();
      GXBOOL        SwitchPage        (Page* pPage);
      GXVOID        DrawScrollBar     (GXHDC hdc, const GXLPRECT lpClient, GXLPRECT prcOut = NULL);
      GXVOID        NotifyParent      (GXUINT code, ITEM& item);
      GXBOOL        DestroyChildDlg   (GXHWND hDlg);
      GXBOOL        CheckScrolled     ();

      GXVOID        Release           ();
    public:

      Form(GXLPCWSTR szName, GXLPCWSTR lpWindowName, GXDWORD dwStyle, GXDWORD dwExStyle, int x, int y, int nWidth, int nHeight, GXHWND hWndParent, GXHINSTANCE hInst);

      GXBOOL    Initialize    (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink);

      GXBOOL    UploadData    (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast);
      GXBOOL    DownloadData  (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast);

      GXBOOL    UpdateData    (const PROPLIST_DATALINK* pDataLink);
      void      LButtonDown   (GXWPARAM wParam, GXLPARAM lParam);
      void      LButtonUp     (GXWPARAM wParam, GXLPARAM lParam);
      void      OnTimer       (GXDWORD dwTiemrID);
      void      MouseMove     (GXWPARAM wParam, GXLPARAM lParam);
      GXLRESULT SetFont       (GXHFONT hFont);

      virtual GXLRESULT Destroy();
      virtual GXLRESULT OnPaint(GXWndCanvas& canvas);
      virtual GXLRESULT Measure(GXRegn* pRegn);

      Page*             FindPage    (GXINT nId);
      Page*             GetCurPage  ()  { return m_pCurPage; }
      inline GXHWND     Get         ()  { return m_hWnd; }

      static Form* Create(GXHINSTANCE hInst, GXHWND hParent, GXLPCWSTR szTemplate, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
    };
  } // namespace PropertyList
} // namespace GXUIEXT

#endif // _UXEXT_PROPERTYLIST_H_