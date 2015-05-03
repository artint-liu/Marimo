#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/GXFont.H"
#include "Include/GXSprite.H"
#include "Include/GXGraphics.H"
#include "Include/DataPool.H"
#include "Include/DataPoolVariable.H"
#include "Include/DataInfrastructure.H"
#include "Smart/smartstream.h"

#include "Include/GXKernel.H"
#include "Include/GXUser.H"
#include "Include/GXGDI.H"

#include "GXUICtrlBase.h"
#include "GXUIList.h"
#include "GXUIList_Simple.h"
#include "GXUIList_Custom.h"
#include "Include/GXCanvas.H"
#include "gxDevice.H"


GXHWND gxIntCreateDialogFromFileW(GXHINSTANCE  hInstance, GXLPCWSTR lpFilename, GXLPCWSTR lpDlgName, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);
//#ifdef _DEBUG
//GXDWORD g_nDbg = 0;
//#endif // _DEBUG
namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////
  DefaultListDataAdapter::DefaultListDataAdapter(GXHWND hWnd)
    : ListDataAdapter (hWnd)
    , m_pDataPool     (NULL)
    , m_DynArray      ()
  {
  }

  DefaultListDataAdapter::~DefaultListDataAdapter()
  {
#ifdef ENABLE_DATAPOOL_WATCHER
    m_pDataPool->UnregisterIdentify(STR_DATAPOOL_WATCHER_UI, (GXLPVOID)m_hWnd);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    SAFE_RELEASE(m_pDataPool);
  }

  GXBOOL DefaultListDataAdapter::Initialize()
  {
    static
      Marimo::VARIABLE_DECLARATION s_DefaultListItem[] = {
        {"string", "name", 0, -1, 0},
        {NULL, NULL},
    };
    Marimo::DataPool::CreateDataPool(&m_pDataPool, NULL, NULL, s_DefaultListItem);
    m_strArrayName = "name";
    m_pDataPool->QueryByName(m_strArrayName, &m_DynArray);

#ifdef ENABLE_DATAPOOL_WATCHER
    m_pDataPool->CreateWatcher(STR_DATAPOOL_WATCHER_UI);
    m_pDataPool->RegisterIdentify(STR_DATAPOOL_WATCHER_UI, (GXLPVOID)m_hWnd);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

    return TRUE;
  }

  GXBOOL DefaultListDataAdapter::Initialize(MOVariable& Var)
  {
    m_strArrayName = Var.GetName();
    if(m_strArrayName.GetLength() == 0) {
      CLBREAK;
      return FALSE;
    }
    m_DynArray = Var;
    Var.GetPool(&m_pDataPool);

#ifdef ENABLE_DATAPOOL_WATCHER
    m_pDataPool->CreateWatcher(STR_DATAPOOL_WATCHER_UI);
    m_pDataPool->RegisterIdentify(STR_DATAPOOL_WATCHER_UI, (GXLPVOID)m_hWnd);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

    return TRUE;
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT DefaultListDataAdapter::AddRef()
  {
    const GXLONG nRef = gxInterlockedIncrement(&m_nRefCount);
    return nRef;
  }

  GXHRESULT DefaultListDataAdapter::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
    if(nRefCount == 0)
    {
      //m_pDataPool->UnregisterOutlet(this);
      delete this;
      return GX_OK;
    }
    return nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXBOOL DefaultListDataAdapter::IsAutoKnock() const
  {
#ifdef ENABLE_DATAPOOL_WATCHER
    if(m_pDataPool) {
      return m_pDataPool->IsAutoKnock();
    }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return FALSE;
  }

  GXINT DefaultListDataAdapter::GetItemCount() const
  {
    return m_DynArray.GetLength();
  }

  GXBOOL DefaultListDataAdapter::IsFixedHeight() const
  {
    return TRUE;
  }

  GXINT DefaultListDataAdapter::GetItemHeight(GXINT nIdx) const
  {
    return -1;
  }

  GXINT DefaultListDataAdapter::AddStringW(GXLPCWSTR szName, GXLPCWSTR lpString)
  {
    MOVariable newstr = m_DynArray.NewBack();
    newstr.Set(lpString);
#ifdef ENABLE_DATAPOOL_WATCHER
    if( ! m_pDataPool->IsAutoKnock()) {
      newstr.Impulse(Marimo::DATACT_Insert);
    }
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return m_DynArray.GetLength();
  }

  GXBOOL DefaultListDataAdapter::GetStringW(GETSTRW* pItemStrDesc)
  {
    pItemStrDesc->VarString = m_DynArray.IndexOf(pItemStrDesc->nIdx);
    pItemStrDesc->sString   = pItemStrDesc->VarString.ToStringW();
    return TRUE;
  }

  MOVariable DefaultListDataAdapter::GetVariable()
  {
    return m_DynArray;
  }

  GXHRESULT DefaultListDataAdapter::GetDataPool(MODataPool** ppDataPool)
  {
    *ppDataPool = m_pDataPool;
    if(m_pDataPool != NULL)
    {
      return m_pDataPool->AddRef();
    }
    return GX_FAIL;
  }

  //GXHRESULT DefaultListDataAdapter::Knock(GXHWND hSender, DataAction eAction, GXLPVOID lpData, GXINT nIndex) const
  //{
  //  if(m_pDataPool != NULL)
  //  {
  //    if(m_pDataPool->IsMember(lpData))
  //    {
  //      return m_pDataPool->Knock(this, eAction, lpData, nIndex);
  //    }
  //    ASSERT(0);
  //  }
  //  return GX_FAIL;
  //}


  //////////////////////////////////////////////////////////////////////////
  //StringArrayDataPool::StringArrayDataPool()
  //  : IArrayDataPool(sizeof(this), 8, 8)
  //{
  //}

  //GXHRESULT StringArrayDataPool::AddRef()
  //{
  //  return ++m_uRefCount;
  //}

  //GXHRESULT StringArrayDataPool::Release()
  //{
  //  if((--m_uRefCount) == 0)
  //  {
  //    delete this;
  //    return GX_OK;
  //  }
  //  return m_uRefCount;
  //}
  //////////////////////////////////////////////////////////////////////////
  int List::OnCreate(GXCREATESTRUCT* pCreateParam)
  {
    m_nColumnWidth = pCreateParam->cx;
    return 0;
  }

  GXLRESULT List::Destroy()
  {
    SAFE_RELEASE(m_pAdapter);
    return CtrlBase::Destroy();
  }

  GXBOOL List::SolveDefinition(const GXDefinitionArrayW& aDefinitions)
  {
    for(GXDefinitionArrayW::const_iterator it = aDefinitions.begin();
      it != aDefinitions.end(); ++it)
    {
      if(it->Name == L"DataPool") {
        SetDataPoolVariableW(it->Value);
      }
      else if(it->Name == L"ItemHeight") {
        SetItemHeight(it->Value.ToInteger());
      }
      else if(it->Name == L"BackColor") {
        SetColor(GXLBSC_BACKGROUND, DlgXM::GetColorFromMarkW(it->Value));
      }
      else if(it->Name == L"TextColor") {
        SetColor(GXLBSC_TEXT, DlgXM::GetColorFromMarkW(it->Value));
      }
      else if(it->Name == L"HightlightColor") {
        SetColor(GXLBSC_HIGHTLIGHT, DlgXM::GetColorFromMarkW(it->Value));
      }
      else if(it->Name == L"HightlightTextColor") {
        SetColor(GXLBSC_HIGHTLIGHTTEXT, DlgXM::GetColorFromMarkW(it->Value));
      }
      else if(it->Name == L"Columns") {
        SetColumnsWidth(it->Value);
      }
    }
    return TRUE;
  }

  GXLRESULT List::SetVariable(MOVariable* pVariable)
  {
    DefaultListDataAdapter* pListAdapter = new DefaultListDataAdapter(m_hWnd);

    if( ! InlCheckNewAndIncReference(pListAdapter)) {
      return GX_FAIL;
    }

    if( ! pListAdapter->Initialize(*pVariable)) {
      SAFE_RELEASE(pListAdapter);
      return GX_FAIL;
    }

    SetAdapter(pListAdapter);
    SAFE_RELEASE(pListAdapter);
    return GX_OK;
  }

  //int List::OnLButtonDown(int fwKeys, int x, int y)
  //{    
  //  //m_xHit = x;
  //  //m_yHit = y;
  //  //m_bLBtnDown = TRUE;
  //  //m_bDrag = FALSE;

  //}

  GXINT List::AddStringW(GXLPCWSTR lpString)
  {
    ASSERT(m_pAdapter != NULL);
    /*if(m_pAdapter == NULL)
    {
      DefaultListDataAdapter* pListAdapter = new DefaultListDataAdapter(m_hWnd);
      if( ! InlCheckNewAndIncReference(pListAdapter)) {
        return -1;
      }

      if( ! pListAdapter->Initialize()) {
        CLBREAK;
        SAFE_RELEASE(pListAdapter);
        return -1;
      }
      SetAdapter(pListAdapter);
      SAFE_RELEASE(pListAdapter);
    }*/

    const GXINT nval = m_pAdapter->AddStringW(NULL, lpString);
    Invalidate(FALSE);
    return nval;
  }

  GXINT List::GetStringW(GXINT nIndex, clStringW& str)
  {
    if(nIndex < 0 || nIndex >= m_pAdapter->GetItemCount()) {
      return -1;
    }
    ListDataAdapter::GETSTRW gs;
    gs.nIdx     = nIndex;
    gs.nElement = -1;
    gs.hItemWnd = NULL;
    gs.szName   = NULL;
    gxSetRectEmpty(&gs.rect);
    m_pAdapter->GetStringW(&gs);
    str = gs.sString;
    return (GXINT)gs.sString.GetLength();
  }

  GXINT List::DeleteString(GXINT nIndex)
  {
    if(nIndex < 0 || nIndex >= m_pAdapter->GetItemCount()) {
      return -1;
    }
    MOVariable VarArray = m_pAdapter->GetVariable();
    MOVariable VarElement = VarArray.IndexOf(nIndex);
#ifdef ENABLE_DATAPOOL_WATCHER
    if( ! m_pAdapter->IsAutoKnock()) {
      VarElement.Impulse(Marimo::DATACT_Deleting, nIndex);
      VarArray.Remove(nIndex);
      VarElement.Impulse(Marimo::DATACT_Deleted, nIndex);
    }
    else {
      VarArray.Remove(nIndex);
    }
#else
    VarArray.Remove(nIndex);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return nIndex;
  }

  GXVOID List::ResetContent()
  {
    if( ! m_pAdapter) {
      return;
    }
    MOVariable VarArray = m_pAdapter->GetVariable();

#ifdef ENABLE_DATAPOOL_WATCHER
    if( ! m_pAdapter->IsAutoKnock()) {
      VarArray.Impulse(Marimo::DATACT_Deleting, -1);
      VarArray.Remove(-1);
      VarArray.Impulse(Marimo::DATACT_Deleted, -1);
    }
    else {
      VarArray.Remove(-1);
    }
#else
    VarArray.Remove(-1);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    Invalidate(FALSE);
  }

  GXINT List::GetCount()
  {
    return m_pAdapter->GetItemCount();
  }

  GXINT List::SetItemHeight(GXINT nNewHeight)
  {
    GXINT nPrevHeight = m_nItemHeight;
    m_nItemHeight = nNewHeight;
    return nPrevHeight;
  }

  //GXINT List::GetStringLength(GXINT nIndex)
  //{
  //  ListDataAdapter::GETSTRW gs;
  //  gs.nIdx = nIndex;
  //  gs.hItemWnd = NULL;
  //  gs.szName = NULL;
  //  m_pAdapter->GetStringW(&gs);
  //  return gs.sString.GetLength();
  //}

  GXBOOL List::CheckEndScrollAnim(GXUINT nIDTimer, bool bForced)
  {
    ASSERT(nIDTimer == IDT_SCROLLUP || nIDTimer == IDT_SCROLLDOWN);
    //if(m_bLBtnDown || m_bDrag || bForced) 
    if(bForced) 
    {
      m_bShowScrollBar = FALSE;
      gxKillTimer(m_hWnd, nIDTimer);
      return TRUE;
    }
    return FALSE;
  }
  
  void List::UpdateScrollBarRect(GXDWORD dwStyle, LPGXCRECT lprcClient)
  {
    GXRECT rcClient;
    if(lprcClient == NULL) {
      gxGetClientRect(m_hWnd, &rcClient);
    }
    else {
      rcClient = *lprcClient;
    }

    if(IS_LEFTTORIGHT(dwStyle)) {
      rcClient.top = rcClient.bottom - SCROLLBAR_WIDTH;
    }
    else {
      rcClient.left = rcClient.right - SCROLLBAR_WIDTH;
    }
    InvalidateRect(&rcClient, FALSE);
  }

  int List::OnMouseMove(int fwKeys, int x, int y)
  {
    //if(TEST_FLAG(fwKeys, GXMK_LBUTTON) == FALSE && m_bLBtnDown)
    //{
    //  return OnLButtonUp(fwKeys, x, y);
    //}
    //else if(m_bLBtnDown)
    //{
    //  if(m_bDrag == TRUE || abs(y - m_yHit) > gxGetSystemMetrics(SM_CYDRAG))
    //  {
    //    const GXINT nScrolled = m_nPrevScrolled + y - m_yHit;
    //    gxScrollWindow(m_hWnd, 0, nScrolled - m_nScrolled, NULL, NULL);
    //    if(m_bShowScrollBar) {
    //      UpdateScrollBarRect();
    //    }
    //    SetScrolledVal(nScrolled);
    //    m_bDrag = TRUE;
    //  }
    //}
    return 0;
  }

  int List::OnMouseWheel(int fwKeys, int nDelta)
  {
    //const GXINT nScrolled = m_nPrevScrolled + nDelta / 2;
    const GXINT nScrolled = m_nScrolled + nDelta / 5;
    const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);

    if(IS_LEFTTORIGHT(dwStyle)) {
      gxScrollWindow(m_hWnd, nScrolled - m_nScrolled, 0, NULL, NULL);
    }
    else {
      gxScrollWindow(m_hWnd, 0, nScrolled - m_nScrolled, NULL, NULL);
    }
    if(m_bShowScrollBar) {
      UpdateScrollBarRect(dwStyle);
    }
    SetScrolledVal(nScrolled);

    gxSetTimer(m_hWnd, IDT_WHEELCHECK, 100, NULL);
    return 0;
  }

  int List::OnTimer(GXUINT nIDTimer)
  {
    TRACE("OnTimer\n");

    switch(nIDTimer)
    {
    case IDT_WHEELCHECK:
      {
        EndScroll();
        gxKillTimer(m_hWnd, IDT_WHEELCHECK);
      }
      break;
    case IDT_SCROLLUP:
    case IDT_SCROLLDOWN:
      {
        const GXINT nNewScroll = (m_nPrevScrolled * 3 + m_nScrolled * 5) >> 3;
        const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);
        CheckEndScrollAnim(nIDTimer, nNewScroll == m_nScrolled);

        //gxScrollWindow(m_hWnd, 0, nNewScroll - m_nScrolled, NULL, NULL);
        if(IS_LEFTTORIGHT(dwStyle)) {
          gxScrollWindow(m_hWnd, nNewScroll - m_nScrolled, 0, NULL, NULL);
        }
        else {
          gxScrollWindow(m_hWnd, 0, nNewScroll - m_nScrolled, NULL, NULL);
        }

        // 不管 m_bShowScrollBar 是否为真都要重绘滚动条区域
        UpdateScrollBarRect(dwStyle);
        SetScrolledVal(nNewScroll);
      }
      break;
    }
    return 0;
  }
  //////////////////////////////////////////////////////////////////////////

  //List* List::Create(
  //  GXDWORD       dwExStyle, 
  //  GXLPCWSTR     lpWindowName, 
  //  GXDWORD       dwStyle, 
  //  const GXRegn* pRegn, 
  //  GXHWND        hWndParent, 
  //  GXHMENU       hMenu, 
  //  GXHINSTANCE   hInstance, 
  //  GXLPCWSTR     szIdName,
  //  GXLPVOID      lpParam)
  //{

  //}

  List* List::Create(GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions)
  {
    GXDWORD dwStyle = GXWS_VISIBLE | GXWS_CHILD | pDlgParam->dwStyle;

    GXDWORD      dwExStyle = NULL;
    GXLPCWSTR    lpWindowName =  pDlgParam->strCaption;
    const GXRegn*pRegn =  &pDlgParam->regn;
    GXHWND       hWndParent =  hParent;
    GXHMENU      hMenu =  NULL;
    GXHINSTANCE  hInstance =  hInst;
    GXLPCWSTR    szIdName=  pDlgParam->strName;
    GXLPVOID     lpParam =  NULL;

    // 注册控件类
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_LIST);

    GXHWND hWnd = NULL;
    hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_LIST, lpWindowName, dwStyle, 
      pRegn->left, pRegn->top, pRegn->width, pRegn->height, hWndParent, NULL, hInstance, NULL);

    // 检查控件创建结果
    List* pList = (List*)gxGetWindowLongPtrW(hWnd, 0);

    if(pList && pList->m_hWnd && pDefinitions) {
      pList->SolveDefinition(*pDefinitions);
    }
    else {
      pList->Destroy();
      delete pList;
      pList = NULL;
    }

    return pList;
  }

  List* List::CreateRich(GXHINSTANCE hInst, GXHWND hParent, const DlgXM::DLGBASICPARAMW* pDlgParam, const GXDefinitionArrayW* pDefinitions)
  {
    GXDWORD dwStyle = GXWS_VISIBLE | GXWS_CHILD | pDlgParam->dwStyle;

    GXDWORD      dwExStyle = NULL;
    GXLPCWSTR    lpWindowName =  pDlgParam->strCaption;
    const GXRegn*pRegn =  &pDlgParam->regn;
    GXHWND       hWndParent =  hParent;
    GXHMENU      hMenu =  NULL;
    GXHINSTANCE  hInstance =  hInst;
    GXLPCWSTR    szIdName=  pDlgParam->strName;
    GXLPVOID     lpParam =  NULL;

    // 注册控件类
    TryRegisterClass(hInstance, WndProc, GXUICLASSNAME_RICHLIST);

    GXHWND hWnd = NULL;
    //LISTBOXCREATIONPARAM ListCreationParam;
    //ListCreationParam.cbSize = sizeof(LISTBOXCREATIONPARAM);
    //ListCreationParam.szTemplate = szTemplate;

    hWnd = gxCreateWindowEx(dwExStyle, GXUICLASSNAME_RICHLIST, lpWindowName, dwStyle, 
      pRegn->left, pRegn->top, pRegn->width, pRegn->height, hWndParent, NULL, hInstance, 
      NULL);

    // 检查控件创建结果
    List* pList = (List*)gxGetWindowLongPtrW(hWnd, 0);

    if(pList && pList->m_hWnd && pDefinitions) {
      pList->SolveDefinition(*pDefinitions);
    }
    else {
      pList->Destroy();
      delete pList;
      pList = NULL;
    }

    return pList;
  }
  //GXLRESULT SimpleList::Measure(GXRegn* pRegn)
  //{
  //  pRegn->Set(0, 0, 128, 128);
  //  return 0;
  //}

  GXLRESULT GXCALLBACK List::WndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    List* pThis = message == GXWM_NCCREATE ? NULL
      : (List*)CtrlBase::PreClassObj(hWnd, message, lParam);

    switch(message)
    {
    case GXLB_ADDSTRINGW:
      return (GXLRESULT)pThis->AddStringW((GXLPCWSTR)lParam);

    case GXLB_DELETESTRING:
      return (GXLRESULT)pThis->DeleteString((GXINT)wParam);

    case GXLB_GETTEXT:
      {
        clStringW str;
        GXLRESULT lr = pThis->GetStringW((GXINT)wParam, str);
        GXSTRCPY((GXWCHAR*)lParam, (const GXWCHAR*)str);
        return lr;
      }
      break;

    case GXLB_GETCURSEL:
      return (GXLRESULT)pThis->GetCurSel();

    case GXLB_GETCOUNT:
      return (GXLRESULT)pThis->GetCount();

    case GXLB_RESETCONTENT:
      pThis->ResetContent();
      return 0;

    case GXLB_SETCOLOR:
      return (GXLRESULT)pThis->SetColor((GXUINT)wParam, (GXCOLORREF)lParam);

    case GXLB_SETCOLUMNSWIDTH:
      return (GXLRESULT)pThis->SetColumnsWidth((const GXUINT*)lParam, (GXUINT)wParam);

    case GXLB_GETCOLUMNSWIDTH:
      return (GXLRESULT)pThis->GetColumnsWidth((GXUINT*)lParam, (GXUINT)wParam);

    case GXLB_GETTEXTLEN:
      {
        clStringW str;
        GXLRESULT lr = pThis->GetStringW((GXINT)wParam, str);
        return lr;
      }
      break;

    case GXLB_ITEMFROMPOINT:
      return pThis->HitTest(wParam, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));

    case GXLB_SETITEMTEMPLATE:
      return pThis->SetItemTemplate((GXLPCWSTR)lParam);


    case GXWM_DATAPOOLOPERATION:
      {
        switch((DataPoolOperation)wParam)
        {
        case DPO_SETADAPTER:
          pThis->SetAdapter((ListDataAdapter*)lParam);
          break;
        case DPO_GETADAPTER:
          pThis->GetAdapter((ListDataAdapter**)lParam);
          break;
        default:
          return -1;
        }
      }
      break;

    //case GXWM_GETADAPTER:
    //  pThis->GetAdapter((ListDataAdapter**)lParam);
    //  break;

    case GXWM_LBUTTONDOWN:
      {
        int x = GXGET_X_LPARAM(lParam);
        int y = GXGET_Y_LPARAM(lParam);
        pThis->OnLButtonDown((int)wParam, x, y);
      }
      return 0;

    case GXWM_MOUSEWHEEL:
      {
        return pThis->OnMouseWheel((int)GXLOWORD(wParam), (int)(short)GXHIWORD(wParam));
      }
      break;

    case GXWM_KNOCK:
      if(wParam == 0)
      {
        pThis->OnKnock((KNOCKACTION*)lParam);
        return 0;
      }
      return -1;

    case GXWM_LBUTTONUP:
      {
        int x = GXGET_X_LPARAM(lParam);
        int y = GXGET_Y_LPARAM(lParam);
        pThis->OnLButtonUp((int)wParam, x, y);
      }
      return 0;
    case GXWM_SIZE:
      {
        int cx = GXGET_X_LPARAM(lParam);
        int cy = GXGET_Y_LPARAM(lParam);
        return pThis->OnSize(cx, cy);
      }
    case GXWM_MOUSEMOVE:
      {
        int x = GXGET_X_LPARAM(lParam);
        int y = GXGET_Y_LPARAM(lParam);
        return pThis->OnMouseMove((int)wParam, x, y);
      }
    case GXWM_NOTIFY:
      {
        GXHWND hParent = gxGetWindow(hWnd, GXGW_PARENT);
        if(hParent) {
          gxSendMessage(hParent, message, wParam, lParam);
        }
      }
      break;
    case GXLB_SETCURSEL:
      {
        const int nIndex = (int)wParam;
        pThis->SetCurSel(nIndex);
      }
      break;
    case GXWM_TIMER:
      return pThis->OnTimer((int)wParam);

    case GXWM_NCCREATE:
      {
        List* pList = NULL;
        GXLPCREATESTRUCT lpcs = (GXLPCREATESTRUCT) lParam;
        //LISTBOXCREATIONPARAM* pListBoxParam = (LISTBOXCREATIONPARAM*)lpcs->lpCreateParams;
        GXLPCWSTR szIdName = IS_IDENTIFY(lpcs->hMenu) ? NULL : (GXLPCWSTR)lpcs->hMenu;

        if(GXSTRCMPI(lpcs->lpszClass, GXUICLASSNAME_RICHLIST) == 0) {
          pList = new CustomizeList(szIdName);
        }
        else {
          // 只可能是GXUICLASSNAME_LIST或GXUICLASSNAME_RICHLIST两种类
          ASSERT(GXSTRCMPI(lpcs->lpszClass, GXUICLASSNAME_LIST) == 0);
          pList = new SimpleList(szIdName);
        }

        //if(pListBoxParam == NULL || pListBoxParam->szTemplate == NULL || 
        //  GXSTRLEN(pListBoxParam->szTemplate) == 0)
        //{
        //  pList = new SimpleList(szIdName);
        //}
        //else {
        //  pList = new CustomizeList(szIdName, pListBoxParam->szTemplate);
        //}

        pList->m_hWnd = hWnd;
        gxSetWindowLong(hWnd, 0, (GXLONG_PTR)pList);
        return TRUE;
      }
    case GXWM_CREATE:
      {
        GXCREATESTRUCT* pCreateParam = (GXCREATESTRUCT*)lParam;
        return pThis->OnCreate(pCreateParam);
      }
    }

    return CtrlBase::DefWndProc(hWnd, message, wParam, lParam, pThis);
  }

  List::List( GXLPCWSTR szIdName )
    : CtrlBase(szIdName)
    , m_pAdapter      (NULL)
    , m_nTopIndex     (0)
    , m_nColumnCount  (1)
    , m_nColumnWidth  (1)
    , n_nLastSelected (-1)
    , m_nScrolled     (0)
    , m_nPrevScrolled (0)
    , m_nItemHeight   (-1)
    , m_bShowScrollBar(0)
    , m_bShowButtonBox(0)
    , m_crBackground  (gxGetSysColor(GXCOLOR_WINDOW))
    , m_crText        (gxGetSysColor(GXCOLOR_INFOTEXT))
    , m_crHightlight  (gxGetSysColor(GXCOLOR_HIGHLIGHT))
    , m_crHightlightText(gxGetSysColor(GXCOLOR_HIGHLIGHTTEXT))
  {
    m_pFont = GXUIGetStock()->pDefaultFont;
    m_pFont->AddRef();
    m_nItemHeight = m_pFont->GetHeight();
  }

  GXCOLORREF List::SetColor( GXUINT nType, GXCOLORREF color )
  {
    GXCOLORREF crPrev = 0;
    switch (nType)
    {
    case GXLBSC_BACKGROUND:     crPrev = m_crBackground;      m_crBackground = color;     break;
    case GXLBSC_TEXT:           crPrev = m_crText;            m_crText = color;           break;
    case GXLBSC_HIGHTLIGHT:     crPrev = m_crHightlight;      m_crHightlight = color;     break;
    case GXLBSC_HIGHTLIGHTTEXT: crPrev = m_crHightlightText;  m_crHightlightText = color; break;
    default: break;
    }
    return crPrev;
  }

  GXUINT List::SetColumnsWidth(GXLPCWSTR szString)
  {
    return 0;
  }

  GXUINT List::SetColumnsWidth( const GXUINT* pColumns, GXUINT nCount )
  {
    return 0;
  }

  GXUINT List::GetColumnsWidth( GXUINT* pColumns, GXUINT nCount )
  {
    return 0;
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE