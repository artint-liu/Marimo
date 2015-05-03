#ifndef _DEV_DISABLE_UI_CODE
#ifndef _GXUI_LIST_H_
#define _GXUI_LIST_H_

#define SCROLLBAR_WIDTH 10
#define CHECKBOX_SIZE 10

#define IS_LEFTTORIGHT(_STYLE)  TEST_FLAG(_STYLE, GXLBS_LTRSCROLLED)

namespace DlgXM
{
  struct DLGBASICPARAM;
}
namespace GXUI
{
  class IDataPool;
  class StringArrayDataPool;
  struct LISTBOXITEMSTAT
  {
    GXINT   nItemHeight;  // ����ֵС��0��ʹ��List�Լ��ĸ߶�
    GXDWORD bSelected : 1;
  };


  class DefaultListDataAdapter : public ListDataAdapter
  {
  public:
    //typedef clvector<GXHWND>  WndHandleArray;
  private:
    //StringArrayDataPool* m_pDataPool;
    MODataPool*   m_pDataPool;
    MOVariable    m_DynArray;
    //clStringA     m_strPoolName;
    clStringA     m_strArrayName;

  public:
    DefaultListDataAdapter(GXHWND hWnd);
    virtual ~DefaultListDataAdapter();

    GXBOOL Initialize();
    GXBOOL Initialize(MOVariable& Var);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef        ();
    virtual GXHRESULT   Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXBOOL      IsAutoKnock   () const;
    virtual GXINT       GetItemCount  () const;
    virtual GXINT       GetItemHeight (GXINT nIdx) const; 
    //virtual GXBOOL      GetItemStatistics (GXINT nIdx, LISTBOXITEMSTAT* pStatictist) const;
    virtual GXBOOL      IsFixedHeight () const;

    virtual GXINT       AddStringW    (GXLPCWSTR szName, GXLPCWSTR lpString);
    virtual GXBOOL      GetStringW    (GETSTRW* pItemStrDesc);
    virtual MOVariable  GetVariable   ();
    virtual GXHRESULT   GetDataPool   (MODataPool** ppDataPool);

    //virtual GXHRESULT Knock         (GXHWND hSender, DataAction eAction, GXLPVOID lpData, GXINT nIndex) const;
    //virtual GXHRESULT DispatchKnock (const DataOutlet* pSender) const;
  };
  //class StringArrayDataPool : public IArrayDataPool<clStringW>
  //{
  //public:
  //  StringArrayDataPool();
  //  virtual GXHRESULT AddRef  ();
  //  virtual GXHRESULT Release ();
  //};
  //GXLRESULT GXCALLBACK ButtonWndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  //////////////////////////////////////////////////////////////////////////
  class List : public CtrlBase
  {
  public:
    const static GXUINT IDT_SCROLLUP   = 1000;  // ���Ϲ���������Timer
    const static GXUINT IDT_SCROLLDOWN = 1001;  // ���¹���������Timer
    const static GXUINT IDT_WHEELCHECK = 1002;  // 

    enum ListType
    {
      LT_Simple,
      LT_Custom,
    };
  protected:
    ListDataAdapter*  m_pAdapter;
    GXINT             m_nTopIndex;
    GXINT             m_nColumnCount;
    GXINT             m_nColumnWidth;
    GXINT             n_nLastSelected;
    GXINT             m_nScrolled;      // Item ������ȥ������,�ȶ�ʱ<=0
    GXINT             m_nPrevScrolled;  // �϶���ʼʱ��Scrolled
    GXINT             m_nItemHeight;
    //GXWORD            m_xHit;
    //GXWORD            m_yHit;
    //GXDWORD           m_bDrag : 1;
    //GXDWORD           m_bLBtnDown : 1;
    GXCOLORREF        m_crBackground;
    GXCOLORREF        m_crText;
    GXCOLORREF        m_crHightlight;
    GXCOLORREF        m_crHightlightText;
    GXDWORD           m_bShowScrollBar : 1;   // ���������ֻ������ʾ, ����Ӧ����
    GXDWORD           m_bShowButtonBox : 1;   // TODO: �Ժ�ĳ�List��Style

  public:
    List(GXLPCWSTR szIdName);
  public:
    static GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    //static List* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static List* Create     (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
    static List* CreateRich (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);

    GXINT   AddStringW          (GXLPCWSTR lpString);
    GXINT   GetStringW          (GXINT nIndex, clStringW& str);
    GXINT   DeleteString        (GXINT nIndex);
    GXVOID  ResetContent        ();
    GXINT   GetCount            ();
    GXINT   SetItemHeight       (GXINT nNewHeight);
    GXCOLORREF SetColor         (GXUINT nType, GXCOLORREF color);
    
    //GXINT   GetStringLength     (GXINT nIndex);
    GXBOOL  CheckEndScrollAnim  (GXUINT nIDTimer, bool bForced);
    void    UpdateScrollBarRect (GXDWORD dwStyle, LPGXCRECT lprcClient = NULL);

    virtual int       OnCreate            (GXCREATESTRUCT* pCreateParam);
    virtual GXLRESULT Destroy             ();
    virtual GXBOOL    SolveDefinition     (const GXDefinitionArrayW& aDefinitions);
    virtual GXLRESULT SetVariable         (MOVariable* pVariable);
    virtual GXUINT    SetColumnsWidth     (GXLPCWSTR szString);
    virtual GXUINT    SetColumnsWidth     (const GXUINT* pColumns, GXUINT nCount);
    virtual GXUINT    GetColumnsWidth     (GXUINT* pColumns, GXUINT nCount);
    virtual int       OnLButtonDown       (int fwKeys, int x, int y) GXPURE;
    virtual int       OnLButtonUp         (int fwKeys, int x, int y) GXPURE;
    virtual int       OnMouseMove         (int fwKeys, int x, int y);
    virtual int       OnMouseWheel        (int fwKeys, int nDelta);
    virtual int       OnSize              (int cx, int cy) GXPURE;
    virtual int       OnTimer             (GXUINT nIDTimer);
    virtual GXHRESULT OnKnock             (KNOCKACTION* pAction) GXPURE;
    virtual GXINT     GetCurSel           () GXPURE;
    virtual int       SetCurSel           (int nIndex) GXPURE;
    virtual GXBOOL    SetAdapter          (ListDataAdapter* pAdapter) GXPURE;
    virtual GXBOOL    GetAdapter          (ListDataAdapter** ppAdapter) GXPURE;
    virtual void      SetScrolledVal      (GXINT nScrolled) GXPURE;
    virtual GXINT     VirGetItemHeight    (GXINT nIdx) const GXPURE;
    virtual GXBOOL    EndScroll           () GXPURE;
    virtual GXBOOL    GetItemRect         (int nItem, GXDWORD dwStyle, GXLPRECT lprc) const GXPURE;
    virtual GXINT     HitTest             (int fwKeys, int x, int y) const GXPURE;
    virtual GXLRESULT SetItemTemplate     (GXLPCWSTR szTemplate) GXPURE;
  };

  template<typename _ITEMSTAT>
  class ListTemplate : public List
  {
  public:
    typedef clvector<_ITEMSTAT>  ItemStatArray;
  protected:
    ItemStatArray     m_aItemStat;
  public:
    ListTemplate(GXLPCWSTR szIdName) : List(szIdName) {}
  public:
    void    DrawScrollBar       (LPGXCRECT lprcClient, GXDWORD dwStyle, GXWndCanvas& canvas);
    //GXINT   HitTest             (int fwKeys, int x, int y) const;
    GXBOOL  IsItemSelected      (GXINT nItem) const;
    GXINT   GetCurSel           ();
    GXBOOL  SelectItem          (GXINT nItem, GXBOOL bSelected, GXBOOL bNotify);
    void    SetScrolledVal      (GXINT nScrolled);
    GXBOOL  SyncItemStatCount   ();
    GXBOOL  UpdateTopIndex      ();
    GXBOOL  UpdateTopIndex      (GXDWORD dwStyle);
    GXBOOL  UpdateItemStat      (GXINT nBegin, GXINT nEnd); // ����[nBeing, nEnd]

    virtual GXBOOL    EndScroll           ();
    virtual int       OnLButtonDown       (int fwKeys, int x, int y);
    virtual int       OnLButtonUp         (int fwKeys, int x, int y);
    virtual GXHRESULT OnKnock             (KNOCKACTION* pAction);
    virtual int       SetCurSel           (int nIndex);
    virtual GXBOOL    SetAdapter          (ListDataAdapter* pAdapter);
    virtual GXBOOL    GetAdapter          (ListDataAdapter** ppAdapter);
    virtual GXBOOL    ReduceItemStat      (GXINT nCount);
    virtual void      DeleteItemStat      (GXINT nIndex);
  };
  
  struct ITEMSTAT
  {
    //GXINT   nTop;
    GXINT   nBottom;
    GXDWORD bSelected : 1;
    GXDWORD bValidate : 1;  // Item����Ч��־, ���Ϊ0��ʾҪ���´�Adapter��ȡ
    ITEMSTAT() : nBottom(-1), bSelected(0), bValidate(0){}
  };
 

  //class SimpleList : public ListTemplate<ITEMSTAT>
  //{
  //public:
  //  //typedef clvector<ITEMSTAT>  ItemStatArray;
  //protected:
  //  virtual GXLRESULT            OnPaint        (GXWndCanvas& canvas);

  //  GXINT     AddStringW          (GXLPCWSTR lpString);
  //  GXBOOL    CheckEndScrollAnim  (GXUINT nIDTimer, bool bForced);
  //  GXINT     HitTest             (int fwKeys, int x, int y) const;
  //  GXINT     GetItemHeight       (GXINT nIdx) const;
  //  GXBOOL    IsItemSelected      (GXINT nItem) const;
  //  GXBOOL    SelectItem          (GXINT nItem, GXBOOL bSelected);

  //  virtual ListType  GetListType         ();
  //  virtual GXINT     VirGetItemHeight    (GXINT nIdx) const;

  //public:
  //  SimpleList(GXLPCWSTR szIdName);
  //  virtual GXLRESULT Measure(GXRegn* pRegn);
  //public:
  //  GXBOOL SetSpriteModule(GXLPCWSTR szSpriteFile, GXLPCWSTR szNormal, GXLPCWSTR szHover, GXLPCWSTR szPressed, GXLPCWSTR szDisabled, GXLPCWSTR szDefault);
  //};
  //////////////////////////////////////////////////////////////////////////
#define _LISTT_TEMPL  template<typename _ITEMSTAT>
#define _LISTT_IMPL   ListTemplate<_ITEMSTAT>

  _LISTT_TEMPL
    void _LISTT_IMPL::DrawScrollBar(LPGXCRECT lprcClient, GXDWORD dwStyle, GXWndCanvas& canvas)
  {
    if(IS_LEFTTORIGHT(dwStyle))
    {
      GXINT nTotalWidth = (m_aItemStat.size() + (m_nColumnCount - 1)) / m_nColumnCount * m_nColumnWidth;
      ASSERT(nTotalWidth > 0);
      canvas.FillRect(
        (-m_nScrolled) * lprcClient->right / nTotalWidth,
        lprcClient->bottom - SCROLLBAR_WIDTH, 
        lprcClient->right * lprcClient->right / nTotalWidth,
        SCROLLBAR_WIDTH, 0x80000000);
    }
    else
    {
      GXINT nTotalHeight = m_aItemStat.back().nBottom;
      ASSERT(nTotalHeight > 0);
      canvas.FillRect(
        lprcClient->right - SCROLLBAR_WIDTH, 
        (-m_nScrolled) * lprcClient->bottom / nTotalHeight,
        SCROLLBAR_WIDTH,
        lprcClient->bottom * lprcClient->bottom / nTotalHeight, 0x80000000);
    }
  }

  //_LISTT_TEMPL
  //  GXINT _LISTT_IMPL::HitTest(int fwKeys, int x, int y) const
  //{
  //  if(m_aItemStat.empty()) {
  //    return -1;
  //  }
  //  const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);

  //  ItemStatArray::const_iterator it = m_aItemStat.begin() + m_nTopIndex;

  //  // FIXME: û�д���MULTICOLUMN���
  //  if(IS_LEFTTORIGHT(dwStyle))
  //  {
  //    CLBREAK; // ��֪������ø�ʲô
  //  }
  //  else
  //  {
  //    ASSERT(it->nBottom > -m_nScrolled);
  //    //GXINT nItem = m_nTopIndex;
  //    y -= m_nScrolled;
  //    for(; it != m_aItemStat.end(); ++it) {
  //      if(y < it->nBottom) {
  //        return it - m_aItemStat.begin();
  //      }
  //    }
  //  }
  //  return -1;
  //}

  _LISTT_TEMPL
  GXBOOL _LISTT_IMPL::IsItemSelected(GXINT nItem) const
  {
    return m_aItemStat[nItem].bSelected;
  }
  
  _LISTT_TEMPL
  int _LISTT_IMPL::OnLButtonDown(int fwKeys, int x, int y)
  {
    if( ! m_pAdapter) {
      return 0;
    }
    GXPOINT pt = {x, y};
    if(gxDragDetect(m_hWnd, &pt))
    {
      GXMSG msg;
      const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);
      gxSetCapture(m_hWnd);

      m_bShowScrollBar = TRUE;
      m_nPrevScrolled = m_nScrolled;

      while(gxGetMessage(&msg, NULL))
      {
        if(msg.message == GXWM_KEYDOWN && msg.wParam == VK_ESCAPE) {
          break;
        }
        else if(msg.message == GXWM_MOUSEMOVE)
        {
          if(IS_LEFTTORIGHT(dwStyle))
          {
            const GXINT nScrolled = m_nPrevScrolled + (GXGET_X_LPARAM(msg.lParam) - x);
            gxScrollWindow(m_hWnd, nScrolled - m_nScrolled, 0, NULL, NULL);
            SetScrolledVal(nScrolled);
          }
          else
          {
            const GXINT nScrolled = m_nPrevScrolled + GXGET_Y_LPARAM(msg.lParam) - y;
            gxScrollWindow(m_hWnd, 0, nScrolled - m_nScrolled, NULL, NULL);
            SetScrolledVal(nScrolled);
          }
          //GXLONG aDelta[2] = {
          //  GXGET_X_LPARAM(msg.lParam) - x,
          //  GXGET_Y_LPARAM(msg.lParam) - y};
          //int aAmount[2] = {0, 0};
          //const GXINT nScrolled = m_nPrevScrolled + aDelta[nDim];
          //aAmount[nDim] = nScrolled - m_nScrolled;
          //gxScrollWindow(m_hWnd, aAmount[0], aAmount[1], NULL, NULL);
          if(m_bShowScrollBar) {
            UpdateScrollBarRect(dwStyle);
          }
          //SetScrolledVal(nScrolled);
        }
        else if(msg.message == GXWM_LBUTTONUP || msg.message == GXWM_RBUTTONUP) {
          break;
        }
        else if(msg.message > GXWM_MOUSEFIRST && msg.message <= GXWM_MOUSELAST) {
          break;
        }

        gxTranslateMessage(&msg);
        gxDispatchMessageW(&msg);
      }
      gxReleaseCapture();

      // ��ֹ�����Ķ���.
      if( ! EndScroll() && m_bShowScrollBar)
      {
        // �����Ϊ�˰ѹ�����������ճɵ�ɫ
        UpdateScrollBarRect(dwStyle);
        m_bShowScrollBar = FALSE;
      }
    }
    else
    {
      GXINT nItem = HitTest(fwKeys, x, y);
      if(nItem >= 0)
      {
        SelectItem(nItem, !IsItemSelected(nItem), TRUE);
        Invalidate(FALSE);
      }
    }
    return 0;
  }

  _LISTT_TEMPL
  int _LISTT_IMPL::OnLButtonUp(int fwKeys, int x, int y)
  {
    //gxReleaseCapture();
    // gxReleaseCapture ����ͷ����ָ��,�����²���������ڴ���
    // Ȼ����WM_MOUSEMOVE��Ϣ, ��List��MouseMove��Ϣ�п����ֻᷢ��һ��LButtonUp��Ϣ, ��һ��Ҫע��
    //if(m_bLBtnDown == FALSE) {
    //  return 0;
    //}
    //m_bShowScrollBar = FALSE;

    //// �����Ϊ�˰ѹ�����������ճɵ�ɫ
    //if( ! m_bShowScrollBar)
    //{
    //  const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    //  UpdateScrollBarRect(dwStyle);
    //}
    //m_bLBtnDown = FALSE;
    //m_bDrag = FALSE;
    return 0;
  }

  _LISTT_TEMPL
  GXBOOL _LISTT_IMPL::EndScroll()
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    GXINT nMinScroll; // �������һ����Ŀ��ȫ��ʾ֮��Topmost���Ǹ�ֵ

    int nItemSize = 0;
    int nClientSize = 0;
    if(IS_LEFTTORIGHT(dwStyle))
    {
      nItemSize = m_nColumnWidth;
      nClientSize = rect.right;
    }
    else
    {
      nItemSize = VirGetItemHeight(0);
      nClientSize = rect.bottom;
    }
    
    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) {
      // �����Ҫ����IsFixedHeight()ǰ��, ��Ϊ�˷�֧�� IsFixedHeight()==TRUE ���Ӽ�
      const GXINT nModCount = m_pAdapter->GetItemCount() + (m_nColumnCount - 1);
      nMinScroll = nClientSize - (nModCount / m_nColumnCount * nItemSize);
    }
    else if(m_pAdapter->IsFixedHeight()) {
      nMinScroll = clMin(0, nClientSize - m_pAdapter->GetItemCount() * nItemSize);
    }
    else {
      nMinScroll = nClientSize - m_aItemStat.back().nBottom;
    }

    if(m_nScrolled > 0)
    {
      m_nPrevScrolled = 0;
      gxSetTimer(m_hWnd, IDT_SCROLLUP, 30, NULL);
      m_bShowScrollBar = TRUE;
    }
    else if(m_nScrolled < nMinScroll)
    {
      nMinScroll = clMin(nMinScroll, 0); // ���е�Item��������С��List��Height��Ҫ���ն�������

      m_nPrevScrolled = nMinScroll;
      gxSetTimer(m_hWnd, IDT_SCROLLDOWN, 30, NULL);
      m_bShowScrollBar = TRUE;
    }
    else {
      return FALSE;
    }
    return TRUE;
  }

  _LISTT_TEMPL
    GXHRESULT _LISTT_IMPL::OnKnock(Marimo::KNOCKACTION* pAction)
  {
    MOVariable varArray = m_pAdapter->GetVariable();
    ASSERT(varArray.IsSamePool(pAction->pDataPool));

    if(pAction->Action == Marimo::DATACT_Change)
    {
      const GXINT index = (GXINT)((GXINT_PTR)pAction->ptr - (GXINT_PTR)varArray.GetPtr()) / (varArray.GetSize()/varArray.GetLength());
      ASSERT(index >= 0);
      UpdateItemStat(index, index);
    }
    else if(pAction->Action == Marimo::DATACT_Insert)
    {
      //ASSERT(pAction->idx < m_pAdapter->GetItemCount());
      SyncItemStatCount();

      //CLBREAK; // ���¸㶨!
      // �ж��Ƿ����м����
      if(pAction->Index != m_pAdapter->GetItemCount() - 1)
      {
        UpdateItemStat(pAction->Index, -1);
      }
      //UpdateItemStat(0, -1);
    }
    else if(pAction->Action == Marimo::DATACT_Deleting)
    {
      auto index = pAction->Index;
      ASSERT(index >= -1); // -1ʱɾ������Item
      ASSERT(m_pAdapter->GetItemCount() == m_aItemStat.size());
      if(index == -1) {
        // ɾ������
        m_aItemStat.clear();
      }
      else {
        ASSERT(index >= 0);
        DeleteItemStat(index);
      }
      ASSERT(m_pAdapter->GetItemCount() > (GXINT)m_aItemStat.size());
    }
    else if(pAction->Action == Marimo::DATACT_Deleted)
    {
      auto index = pAction->Index;
      ASSERT(m_pAdapter->GetItemCount() == m_aItemStat.size());
      ASSERT(index >= -1); // -1ʱɾ������Item
      if(index >= 0)
      {
        UpdateItemStat(index, -1);
      }
      ASSERT(m_pAdapter->GetItemCount() == m_aItemStat.size());
    }
    else
    {
      ASSERT(0);
    }

    Invalidate(FALSE);

    //if(pReflectKnock->eAction == DA_Change)
    //{
    //  ASSERT(pReflectKnock->nIndex >= 0);
    //  UpdateItemStat(pReflectKnock->nIndex, pReflectKnock->nIndex);
    //}
    //else if(pReflectKnock->eAction == DA_Insert)
    //{
    //  ASSERT(pReflectKnock->nIndex < m_pAdapter->GetItemCount());
    //  SyncItemStatCount();

    //  // �ж��Ƿ����м����
    //  if(pReflectKnock->nIndex != m_pAdapter->GetItemCount() - 1)
    //  {
    //    UpdateItemStat(pReflectKnock->nIndex, -1);
    //  }
    //}
    //else if(pReflectKnock->eAction == DA_PotentialChange)
    //{

    //}
    //else
    //{
    //  ASSERT(0);
    //}
    return 0;
  }

  _LISTT_TEMPL
  int _LISTT_IMPL::SetCurSel(int nIndex)
  {
    const size_t nCount = m_pAdapter->GetItemCount();
    if(nIndex < 0 || nIndex >= (int)nCount) {
      return GXLB_ERR;
    }

    // �����ĵ�,�û����ò�����֪ͨ��Ϣ
    if( ! SelectItem(nIndex, TRUE, FALSE)) {
      return GXLB_ERR;
    }
    return nIndex;
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::SetAdapter(ListDataAdapter* pAdapter)
  {
    if(m_pAdapter == pAdapter) {
      return FALSE;
    }

    SAFE_RELEASE(m_pAdapter);
    m_pAdapter = pAdapter;
    m_pAdapter->AddRef();
    SyncItemStatCount();

    // �������ı�֪ͨ
    GXNMLISTADAPTER sCreateAdapter;
    sCreateAdapter.hdr.hwndFrom = m_hWnd;
    sCreateAdapter.hdr.idFrom   = 0;
    sCreateAdapter.hdr.code     = GXLBN_ADAPTERCHANGED;
    sCreateAdapter.hTemplateWnd = NULL;
    sCreateAdapter.pAdapter     = pAdapter;

    MODataPool* pDataPool = NULL;
    if(GXSUCCEEDED(pAdapter->GetDataPool(&pDataPool)))
    {
#ifdef ENABLE_DATAPOOL_WATCHER
      pDataPool->RegisterIdentify(STR_DATAPOOL_WATCHER_UI, (GXLPVOID)m_hWnd);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      pDataPool->Release();
      pDataPool = NULL;
    }

    gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);    

    InvalidateRect(NULL, FALSE);
    return TRUE;
  }

  _LISTT_TEMPL
  GXBOOL _LISTT_IMPL::GetAdapter(ListDataAdapter** ppAdapter)
  {
    *ppAdapter = m_pAdapter;
    if(m_pAdapter != NULL) {
      m_pAdapter->AddRef();
    }
    return TRUE;
  }

  _LISTT_TEMPL
  GXBOOL _LISTT_IMPL::ReduceItemStat(GXINT nCount)
  {
    m_aItemStat.erase(m_aItemStat.begin() + nCount, m_aItemStat.end());
    return TRUE;
  }

  _LISTT_TEMPL
  void _LISTT_IMPL::DeleteItemStat(GXINT nIndex)
  {
    m_aItemStat.erase(m_aItemStat.begin() + nIndex);
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::SyncItemStatCount()
  {
    const GXUINT nItemCount = (GXUINT)m_aItemStat.size();
    const GXUINT nAdapterCount = m_pAdapter->GetItemCount();
    if(nItemCount < nAdapterCount)
    {
      _ITEMSTAT sis;
      for(GXUINT i = nItemCount; i < nAdapterCount; i++) {
        m_aItemStat.push_back(sis);
      }
      return UpdateItemStat(nItemCount, nAdapterCount - 1);
    }
    else if(nItemCount > nAdapterCount)
    {
      //m_aItemStat.erase(m_aItemStat.begin() + nAdapterCount, m_aItemStat.end());
      //return TRUE;
      return ReduceItemStat(nAdapterCount);
    }
    return TRUE;
  }

  _LISTT_TEMPL
  GXINT _LISTT_IMPL::GetCurSel()
  {
    if(n_nLastSelected >= 0 && n_nLastSelected < (int)m_aItemStat.size() && 
      m_aItemStat[n_nLastSelected].bSelected)
    {
      return n_nLastSelected;
    }
    return -1;
    //return m_aItemStat[n_nLastSelected].bSelected ? n_nLastSelected : -1;
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::SelectItem(GXINT nItem, GXBOOL bSelected, GXBOOL bNotify)
  {
    ASSERT(m_pAdapter->GetItemCount() == m_aItemStat.size());
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    if(TEST_FLAG(dwStyle, GXLBS_NOSEL)) {
      return FALSE;
    }

    bNotify = bNotify && TEST_FLAG(dwStyle, GXLBS_NOTIFY);

    //GXBOOL bSelChange = bNotify && bSelected;
    //GXBOOL bSelCancel = bNotify && ( ! bSelected);

    m_aItemStat[nItem].bSelected = bSelected;

    // ��ѡʱ������һ�ε�ѡ��
    if(n_nLastSelected >= 0 && n_nLastSelected != nItem && 
      TEST_FLAG(dwStyle, GXLBS_MULTIPLESEL) == 0)
    {
      m_aItemStat[n_nLastSelected].bSelected = FALSE;
    }

    n_nLastSelected = nItem;

    if(bNotify) {
      if(bSelected) {
        NotifyParent(GXLBN_SELCHANGE);
      }
      else {
        NotifyParent(GXLBN_SELCANCEL);
      }
    }

    return TRUE;
  }

  _LISTT_TEMPL
    void _LISTT_IMPL::SetScrolledVal(GXINT nScrolled)
  {
    m_nScrolled = nScrolled;
    UpdateTopIndex();
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::UpdateTopIndex(GXDWORD dwStyle)
  {
    GXINT nScrolled = -m_nScrolled;
    GXINT nItem = 0;

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN) || m_pAdapter->IsFixedHeight())
    {
      m_nTopIndex = nScrolled / VirGetItemHeight(0) * m_nColumnCount;
      clClamp(0, m_pAdapter->GetItemCount(), &m_nTopIndex);
      return TRUE;
    }
    else {
      for(ItemStatArray::iterator it = m_aItemStat.begin();
        it != m_aItemStat.end(); ++it, ++nItem)
      {
        if(it->nBottom > nScrolled) {
          m_nTopIndex = nItem;
          return TRUE;
        }
      }
    }
    return FALSE;
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::UpdateTopIndex()
  {
    if( ! m_pAdapter) {
      return FALSE;
    }
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    return UpdateTopIndex(dwStyle);
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::UpdateItemStat(GXINT nBegin, GXINT nEnd)
  {
    ASSERT(m_pAdapter->GetItemCount() == m_aItemStat.size());
    const GXINT nItemCount = m_pAdapter->GetItemCount();

    nBegin = clMax(nBegin, 0);
    ASSERT(nBegin < nItemCount);

    if(nEnd < 0) {
      nEnd = nItemCount - 1;
    }
    ASSERT(nEnd < nItemCount);

    // ��һ���Bottom
    GXINT nPrevBottom = nBegin == 0 ? 0 : m_aItemStat[nBegin - 1].nBottom;
    const GXBOOL bFixedHeight = m_pAdapter->IsFixedHeight();
    const GXINT nItemHeight = bFixedHeight ? VirGetItemHeight(0) : -1;
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      int nVirHeightMax = 0;
      for(GXINT i = nBegin; i <= nEnd; i++)
      {
        _ITEMSTAT& sis = m_aItemStat[i];
        const int nEndOfRow = (i + 1) % m_nColumnCount;

        if(bFixedHeight) {
          sis.nBottom = nPrevBottom + nItemHeight;

          if(nEndOfRow == 0) {
            nPrevBottom = sis.nBottom;
          }
        }
        else {
          const GXINT nVirHeight = VirGetItemHeight(i);
          sis.nBottom = nPrevBottom + nVirHeight;
          nVirHeightMax = clMax(nVirHeight, nVirHeightMax);

          if(nVirHeightMax == 0) {
            nPrevBottom += nVirHeightMax;
            nVirHeightMax = 0;
          }
        }

        sis.bValidate = TRUE;
      }
    }
    else
    {
      for(GXINT i = nBegin; i <= nEnd; i++)
      {
        _ITEMSTAT& sis = m_aItemStat[i];
        if(bFixedHeight) {
          sis.nBottom = nPrevBottom + nItemHeight;
        }
        else {
          sis.nBottom = nPrevBottom + VirGetItemHeight(i);
        }

        sis.bValidate = TRUE;
        nPrevBottom = sis.nBottom;
      }
    }
    return TRUE;
  }

  //struct CUSTOMIZEITEMSTAT
  //{
  //  //GXINT   nTop;
  //  GXINT   nBottom;
  //  GXHWND  hItem;
  //  GXDWORD bSelected : 1;
  //  GXDWORD bValidate : 1;  // Item����Ч��־, ���Ϊ0��ʾҪ���´�Adapter��ȡ
  //  CUSTOMIZEITEMSTAT() : nBottom(-1), hItem(NULL), bSelected(0), bValidate(0){}
  //};
  //
  //typedef clvector<CUSTOMIZEITEMSTAT>  CustItemStatArray;

  //class CustomizeList : public ListTemplate<CUSTOMIZEITEMSTAT>
  //{
  //public:
  //  typedef clvector<GXHWND>    GXHWndArray;
  //private:
  //  clStringW         m_strTemplate;
  //  GXHWND            m_hPrototype;
  //  //CustItemStatArray m_aCustItemStat;
  //  clStringArray     m_aElementName;   // Item �еĿؼ���
  //  GXHWndArray       m_aWndPool;
  //private:
  //  static GXLRESULT GXCALLBACK CustomWndProc  (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  //  static GXBOOL    GXCALLBACK EnumChildProc  (GXHWND hWnd, GXLPARAM lParam);

  //public:
  //  GXINT     GetItemHeight       (GXINT nIdx) const;
  //  GXHWND    PlantCustItem       (int nIndex, GXLPCRECT lprect);
  //  int       Recycle             (int nBegin, int nDir); // ���� nBegin
  //  GXBOOL    UpdateCustItemText  (int nIndex);

  //  virtual ListType  GetListType         ();
  //  virtual GXLRESULT Measure(GXRegn* pRegn);
  //  virtual int       OnCreate            ();
  //  virtual GXLRESULT OnPaint             (GXWndCanvas& canvas);
  //  virtual GXINT     VirGetItemHeight    (GXINT nIdx) const;


  //public:
  //  CustomizeList(GXLPCWSTR szIdName, GXLPCWSTR szTemplate);
  //};
} // namespace GXUI
#endif // _GXUI_LIST_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE