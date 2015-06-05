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
    GXINT   nItemHeight;  // 返回值小于0则使用List自己的高度
    GXDWORD bSelected : 1;
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
    const static GXUINT IDT_SCROLLUP   = 1000;  // 向上滚动动画的Timer
    const static GXUINT IDT_SCROLLDOWN = 1001;  // 向下滚动动画的Timer
    const static GXUINT IDT_WHEELCHECK = 1002;  // 

    enum ListType
    {
      LT_Simple,
      LT_Custom,
    };
  protected:
    IListDataAdapter* m_pAdapter;
    GXSIZE_T          m_nTopIndex;
    GXINT             m_nColumnCount;
    GXINT             m_nColumnWidth;
    GXINT             m_nLastSelected;
    GXINT             m_nScrolled;      // Item 滚动过去的数量,稳定时<=0
    GXINT             m_nPrevScrolled;  // 拖动开始时的Scrolled
    GXINT             m_nItemHeight;    // TODO: 这个看看是否能去掉
    GXCOLORREF        m_crBackground;
    GXCOLORREF        m_crText;
    GXCOLORREF        m_crHightlight;
    GXCOLORREF        m_crHightlightText;
    GXDWORD           m_bShowScrollBar : 1;   // 这个滚动条只用于显示, 不响应操作
    GXDWORD           m_bShowButtonBox : 1;   // TODO: 以后改成List的Style

  public:
    List(GXLPCWSTR szIdName);
  public:
    static GXLRESULT GXCALLBACK WndProc        (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
    //static List* Create  (GXDWORD dwExStyle, GXLPCWSTR lpWindowName, GXDWORD dwStyle, const GXRegn* pRegn, GXHWND hWndParent, GXHMENU hMenu, GXHINSTANCE hInstance, GXLPCWSTR szIdName, GXLPVOID lpParam);
    static List* Create     (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);
    static List* CreateRich (GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions);

    GXINT   AddStringW          (GXLPCWSTR lpString);
    GXINT   GetStringW          (GXSIZE_T nIndex, clStringW& str);
    GXINT   DeleteString        (GXSIZE_T nIndex);
    GXVOID  ResetContent        ();
    GXINT   GetCount            ();
    GXINT   SetItemHeight       (GXINT nNewHeight);
    GXCOLORREF SetColor         (GXUINT nType, GXCOLORREF color);
    
    //GXINT   GetStringLength     (GXINT nIndex);
    GXBOOL  CheckEndScrollAnim  (GXUINT nIDTimer, bool bForced);
    void    UpdateScrollBarRect (GXDWORD dwStyle, LPGXCRECT lprcClient = NULL);
    GXBOOL  IsEmpty             () const;

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
    //virtual GXHRESULT OnKnock             (KNOCKACTION* pAction) GXPURE;
    virtual GXVOID    OnImpulse           (LPCDATAIMPULSE pImpulse) GXPURE;
    virtual GXINT     GetCurSel           () GXPURE;
    virtual int       SetCurSel           (int nIndex) GXPURE;
    virtual GXBOOL    SetAdapter          (IListDataAdapter* pAdapter) GXPURE;
    virtual GXBOOL    GetAdapter          (IListDataAdapter** ppAdapter) GXPURE;
    virtual void      SetScrolledVal      (GXINT nScrolled) GXPURE;
    virtual GXINT     VirGetItemHeight    (GXINT nIdx) const GXPURE;
    virtual GXBOOL    EndScroll           () GXPURE;
    virtual GXBOOL    GetItemRect         (int nItem, GXDWORD dwStyle, GXLPRECT lprc) const GXPURE;
    virtual GXINT     HitTest             (int fwKeys, int x, int y) const GXPURE;
    virtual GXLRESULT SetItemTemplate     (GXLPCWSTR szTemplate) GXPURE;

    void DrawScrollBar( GXWndCanvas& canvas, LPGXCRECT lprcClient, GXINT nLastBottom, GXSIZE_T count, GXDWORD dwStyle ) const;
  };

  //////////////////////////////////////////////////////////////////////////

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
    //GXINT   HitTest             (int fwKeys, int x, int y) const;
    GXBOOL  IsItemSelected      (GXINT nItem) const;
    GXINT   GetCurSel           ();
    GXBOOL  SelectItem          (GXINT nItem, GXBOOL bSelected, GXBOOL bNotify);
    //GXINT   GetCurSel           ();
    void    SetScrolledVal      (GXINT nScrolled);
    GXBOOL  SyncItemStatCount   ();
    GXBOOL  UpdateTopIndex      ();
    GXBOOL  UpdateTopIndex      (GXDWORD dwStyle);
    GXBOOL  UpdateItemStat      (GXINT nBegin, GXINT nEnd); // 区间[nBeing, nEnd]

    virtual GXBOOL    EndScroll           ();
    virtual int       OnLButtonDown       (int fwKeys, int x, int y);
    virtual int       OnLButtonUp         (int fwKeys, int x, int y);
    //virtual GXHRESULT OnKnock             (KNOCKACTION* pAction);
    virtual GXVOID    OnImpulse           (LPCDATAIMPULSE pImpulse);
    virtual int       SetCurSel           (int nIndex);
    virtual GXBOOL    SetAdapter          (IListDataAdapter* pAdapter);
    virtual GXBOOL    GetAdapter          (IListDataAdapter** ppAdapter);
    virtual GXBOOL    ReduceItemStat      (GXINT nCount);
    virtual void      DeleteItemStat      (GXINT nIndex);
  };
  
  struct ITEMSTAT
  {
    GXINT   nBottom;
    GXDWORD bSelected : 1;
    GXDWORD bValidate : 1;  // Item的有效标志, 如果为0表示要重新从Adapter获取
    ITEMSTAT() : nBottom(-1), bSelected(0), bValidate(0){}
  };
 
  //////////////////////////////////////////////////////////////////////////

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



  //_LISTT_TEMPL
  //  void _LISTT_IMPL::DrawScrollBar(LPGXCRECT lprcClient, GXDWORD dwStyle, GXWndCanvas& canvas)
  //{
  //  if(IS_LEFTTORIGHT(dwStyle))
  //  {
  //    CLBREAK; // 不知道这里该干什么
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

      // 终止滚动的动画.
      if( ! EndScroll() && m_bShowScrollBar)
      {
        // 这个是为了把滚动条区域清空成底色
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
    // gxReleaseCapture 里会释放鼠标指针,并重新测试鼠标所在窗口
    // 然后发送WM_MOUSEMOVE消息, 而List的MouseMove消息有可能又会发送一个LButtonUp消息, 这一点要注意
    //if(m_bLBtnDown == FALSE) {
    //  return 0;
    //}
    //m_bShowScrollBar = FALSE;

    //// 这个是为了把滚动条区域清空成底色
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
    GXINT nMinScroll; // 就是最后一个项目完全显示之后Topmost的那个值

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
      // 这个需要放在IsFixedHeight()前面, 因为此分支是 IsFixedHeight()==TRUE 的子集
      const GXINT nModCount = m_pAdapter->GetCount() + (m_nColumnCount - 1);
      nMinScroll = nClientSize - (nModCount / m_nColumnCount * nItemSize);
    }
    //else if(m_pAdapter->IsFixedHeight()) {
    //  nMinScroll = clMin(0, nClientSize - m_pAdapter->GetCount() * nItemSize);
    //}
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
      nMinScroll = clMin(nMinScroll, 0); // 所有的Item加起来仍小于List的Height，要按照顶部对齐

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
    GXVOID _LISTT_IMPL::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    MOVariable varArray = m_pAdapter->GetVariable();
    //ASSERT(varArray.IsSamePool(pImpulse->pDataPool));
    CLBREAK;

    if(pImpulse->reason == Marimo::DATACT_Change)
    {
      CLBREAK;
      const GXINT index = pImpulse->index;
        //(GXINT)((GXINT_PTR)pImpulse->ptr - (GXINT_PTR)varArray.GetPtr()) / (varArray.GetSize()/varArray.GetLength());
      ASSERT(index >= 0);
      UpdateItemStat(index, index);
    }
    else if(pImpulse->reason == Marimo::DATACT_Insert)
    {
      //ASSERT(pAction->idx < m_pAdapter->GetCount());
      SyncItemStatCount();

      //CLBREAK; // 重新搞定!
      // 判断是否在中间插入
      if(pImpulse->index != m_pAdapter->GetCount() - 1)
      {
        UpdateItemStat(pImpulse->index, -1);
      }
      //UpdateItemStat(0, -1);
    }
    else if(pImpulse->reason == Marimo::DATACT_Deleting)
    {
      auto index = pImpulse->index;
      ASSERT(index >= -1); // -1时删除所有Item
      ASSERT(m_pAdapter->GetCount() == m_aItemStat.size());
      if(index == -1) {
        // 删除所有
        m_aItemStat.clear();
      }
      else {
        ASSERT(index >= 0);
        DeleteItemStat(index);
      }
      ASSERT(m_pAdapter->GetCount() > (GXINT)m_aItemStat.size());
    }
    else if(pImpulse->reason == Marimo::DATACT_Deleted)
    {
      auto index = pImpulse->index;
      ASSERT(m_pAdapter->GetCount() == m_aItemStat.size());
      ASSERT(index >= -1); // -1时删除所有Item
      if(index >= 0)
      {
        UpdateItemStat(index, -1);
      }
      ASSERT(m_pAdapter->GetCount() == m_aItemStat.size());
    }
    else
    {
      ASSERT(0);
    }

    Invalidate(FALSE);
    //return 0;
  }

  _LISTT_TEMPL
  int _LISTT_IMPL::SetCurSel(int nIndex)
  {
    const size_t nCount = m_pAdapter->GetCount();
    if(nIndex < 0 || nIndex >= (int)nCount) {
      return GXLB_ERR;
    }

    // 根据文档,用户设置不发送通知消息
    if( ! SelectItem(nIndex, TRUE, FALSE)) {
      return GXLB_ERR;
    }
    return nIndex;
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::SetAdapter(IListDataAdapter* pAdapter)
  {
    //MODataPool* pDataPool = NULL;

    if(m_pAdapter == pAdapter) {
      return FALSE;
    }

    //if(m_pAdapter)
    //{
    //  if(GXSUCCEEDED(m_pAdapter->GetDataPool(&pDataPool)))
    //  {
    //    pDataPool->Ignore(&m_pAdapter->GetVariable(), m_hWnd);
    //    pDataPool->Release();
    //    pDataPool = NULL;
    //  }

    //  m_pAdapter->Release();
    //  m_pAdapter = NULL;
    //}
    SAFE_DELETE(m_pAdapter);

    m_pAdapter = pAdapter;

    if(m_pAdapter) {
      m_pAdapter->AddRef();

      SyncItemStatCount();

      // 适配器改变通知
      GXNMLISTADAPTER sCreateAdapter;
      sCreateAdapter.hdr.hwndFrom = m_hWnd;
      sCreateAdapter.hdr.idFrom   = 0;
      sCreateAdapter.hdr.code     = GXLBN_ADAPTERCHANGED;
      sCreateAdapter.hTemplateWnd = NULL;
      sCreateAdapter.pAdapter     = pAdapter;

      //if(GXSUCCEEDED(pAdapter->GetDataPool(&pDataPool)))
      //{
      //  pDataPool->Watch(&pAdapter->GetVariable(), m_hWnd);
      //  pDataPool->Release();
      //  pDataPool = NULL;
      //}

      gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);    

      InvalidateRect(NULL, FALSE);
    }
    else {
      SyncItemStatCount();
    }
    return TRUE;
  }

  _LISTT_TEMPL
  GXBOOL _LISTT_IMPL::GetAdapter(IListDataAdapter** ppAdapter)
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
    const GXUINT nAdapterCount = m_pAdapter ? m_pAdapter->GetCount() : 0;
    if(nAdapterCount == 0) {
      m_aItemStat.clear();
      return TRUE;
    }

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
    if(m_nLastSelected >= 0 && m_nLastSelected < (int)m_aItemStat.size())
    {
      return m_nLastSelected;
    }
    return -1;
    //return m_aItemStat[m_nLastSelected].bSelected ? m_nLastSelected : -1;
  }

  _LISTT_TEMPL
    GXBOOL _LISTT_IMPL::SelectItem(GXINT nItem, GXBOOL bSelected, GXBOOL bNotify)
  {
    ASSERT(m_pAdapter->GetCount() == m_aItemStat.size());
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    if(TEST_FLAG(dwStyle, GXLBS_NOSEL)) {
      return FALSE;
    }

    bNotify = bNotify && TEST_FLAG(dwStyle, GXLBS_NOTIFY);

    //GXBOOL bSelChange = bNotify && bSelected;
    //GXBOOL bSelCancel = bNotify && ( ! bSelected);

    m_aItemStat[nItem].bSelected = bSelected;

    // 单选时撤销上一次的选择
    if(m_nLastSelected >= 0 && m_nLastSelected != nItem && 
      TEST_FLAG(dwStyle, GXLBS_MULTIPLESEL) == 0)
    {
      m_aItemStat[m_nLastSelected].bSelected = FALSE;
    }

    m_nLastSelected = nItem;

    if(bNotify) {
      NotifyParent((nItem == -1) ? GXLBN_SELCANCEL : GXLBN_SELCHANGE);
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

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)/* || m_pAdapter->IsFixedHeight()*/)
    {
      m_nTopIndex = nScrolled / VirGetItemHeight(0) * m_nColumnCount;
      clClamp((GXSIZE_T)0, m_pAdapter->GetCount(), &m_nTopIndex);
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
    ASSERT(m_pAdapter->GetCount() == m_aItemStat.size());
    const GXINT nItemCount = m_pAdapter->GetCount();

    nBegin = clMax(nBegin, 0);
    ASSERT(nBegin < nItemCount);

    if(nEnd < 0) {
      nEnd = nItemCount - 1;
    }
    ASSERT(nEnd < nItemCount);

    // 上一项的Bottom
    GXINT nPrevBottom = nBegin == 0 ? 0 : m_aItemStat[nBegin - 1].nBottom;
    //const GXBOOL bFixedHeight = m_pAdapter->IsFixedHeight();
    //const GXINT nItemHeight = bFixedHeight ? VirGetItemHeight(0) : -1;
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      int nVirHeightMax = 0;
      for(GXINT i = nBegin; i <= nEnd; i++)
      {
        _ITEMSTAT& sis = m_aItemStat[i];
        const int nEndOfRow = (i + 1) % m_nColumnCount;

        /*if(bFixedHeight) {
          sis.nBottom = nPrevBottom + nItemHeight;

          if(nEndOfRow == 0) {
            nPrevBottom = sis.nBottom;
          }
        }
        else */{
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
        /*if(bFixedHeight) {
          sis.nBottom = nPrevBottom + nItemHeight;
        }
        else */{
          sis.nBottom = nPrevBottom + VirGetItemHeight(i);
        }

        sis.bValidate = TRUE;
        nPrevBottom = sis.nBottom;
      }
    }
    return TRUE;
  }

} // namespace GXUI
#endif // _GXUI_LIST_H_
#endif // #ifndef _DEV_DISABLE_UI_CODE