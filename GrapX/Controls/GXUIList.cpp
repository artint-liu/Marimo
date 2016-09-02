#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"
#include "GrapX/GResource.H"
#include "GrapX/GXFont.H"
#include "GrapX/GXSprite.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataInfrastructure.H"
#include "Smart/smartstream.h"

#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"

#include "GXUICtrlBase.h"
#include "GXUIList.h"
#include "GXUIList_Simple.h"
#include "GXUIRichList.h"
#include "GrapX/GXCanvas.H"
#include "GrapX/gxDevice.H"
#include "ListDataAdapter.h"

GXHWND gxIntCreateDialogFromFileW(GXHINSTANCE  hInstance, GXLPCWSTR lpFilename, GXLPCWSTR lpDlgName, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);

#define IS_LEFTTORIGHT(_STYLE)  ((m_bRichList && TEST_FLAG(_STYLE, GXLBS_LTRSCROLLED)) || ( ! m_bRichList && TEST_FLAG(_STYLE, GXLBS_MULTICOLUMN)))

namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  int List::OnCreate(GXCREATESTRUCT* pCreateParam)
  {
    m_nColumnWidth = pCreateParam->cx;
    return 0;
  }

  GXLRESULT List::Destroy()
  {
    SetAdapter(NULL);
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

  GXSIZE_T List::AddStringW(GXLPCWSTR lpString)
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

    const GXSIZE_T nval = m_pAdapter->AddStringW(NULL, lpString);
    Invalidate(FALSE);
    return nval;
  }

  GXSIZE_T List::GetStringW(GXSIZE_T nIndex, clStringW& str)
  {
    if(/*nIndex < 0 || */nIndex >= m_pAdapter->GetCount()) {
      return -1;
    }
    IListDataAdapter::GETSTRW gs;
    gs.item     = (GXINT)nIndex;
    gs.element  = -1;
    gs.hItemWnd = NULL;
    gs.name     = NULL;
    //gxSetRectEmpty(&gs.rect);
    m_pAdapter->GetStringW(&gs);
    str = gs.sString;
    return (GXINT)gs.sString.GetLength();
  }

  GXSIZE_T List::DeleteString(GXSIZE_T nIndex)
  {
    if(/*nIndex < 0 || */nIndex >= m_pAdapter->GetCount()) {
      return -1;
    }
    MOVariable VarArray = m_pAdapter->GetVariable();
    MOVariable VarElement = VarArray.IndexOf(nIndex);
//#ifdef ENABLE_DATAPOOL_WATCHER
    /*if( ! m_pAdapter->IsAutoKnock()) {
    VarElement.Impulse(Marimo::DATACT_Deleting, nIndex);
    VarArray.Remove(nIndex);
    VarElement.Impulse(Marimo::DATACT_Deleted, nIndex);
    }
    else */{
      VarArray.Remove(nIndex);
    }
//#else
//    VarArray.Remove(nIndex);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    return nIndex;
  }

  GXVOID List::ResetContent()
  {
    if( ! m_pAdapter) {
      return;
    }
    MOVariable VarArray = m_pAdapter->GetVariable();

//#ifdef ENABLE_DATAPOOL_WATCHER
    /*if( ! m_pAdapter->IsAutoKnock()) {
    VarArray.Impulse(Marimo::DATACT_Deleting, -1);
    VarArray.Remove(-1);
    VarArray.Impulse(Marimo::DATACT_Deleted, -1);
    }
    else */{
      VarArray.Remove(-1);
    }
//#else
//    VarArray.Remove(-1);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    Invalidate(FALSE);
  }

  GXSIZE_T List::GetCount() const
  {
    return m_pAdapter->GetCount();
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

  //GXBOOL List::CheckEndScrollAnim(GXUINT nIDTimer, bool bForced)
  //{
  //  ASSERT(nIDTimer == IDT_SCROLLUP || nIDTimer == IDT_SCROLLDOWN);
  //  if(bForced) 
  //  {
  //    m_bShowScrollBar = FALSE;
  //    gxKillTimer(m_hWnd, nIDTimer);
  //    return TRUE;
  //  }
  //  return FALSE;
  //}

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
    return 0;
  }

  int List::OnMouseWheel(int fwKeys, int nDelta)
  {
    //const GXINT nScrolled = m_nPrevScrolled + nDelta / 2;
    const GXINT nScrolled = m_nScrolled + nDelta / 5;
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);

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
    //TRACE("OnTimer\n");

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
        GXINT nNewScroll = (m_nPrevScrolled * 3 + m_nScrolled * 5) >> 3;
        const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
        
        //if(abs(m_nPrevScrolled - nNewScroll) < 5)
        if(nNewScroll == m_nScrolled)
        {
          ASSERT(nIDTimer == IDT_SCROLLUP || nIDTimer == IDT_SCROLLDOWN);
          m_bShowScrollBar = FALSE;
          gxKillTimer(m_hWnd, nIDTimer);
          nNewScroll = m_nPrevScrolled;
        }
        //CheckEndScrollAnim(nIDTimer, abs(m_nPrevScrolled - nNewScroll) < 5);

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
      pRegn->left, pRegn->top, pRegn->width, pRegn->height, hWndParent, (GXHMENU)szIdName, hInstance, NULL);

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
      pRegn->left, pRegn->top, pRegn->width, pRegn->height, hWndParent, (GXHMENU)szIdName, hInstance, 
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

    case GXLB_GETSEL:
      return (GXLRESULT)pThis->IsSelected((GXSIZE_T)wParam);

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
      return (GXWPARAM)pThis->HitTest((int)wParam, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));

    case GXLB_SETITEMTEMPLATE:
      return pThis->SetItemTemplate((GXLPCWSTR)lParam);


    case GXWM_DATAPOOLOPERATION:
      {
        switch((DataPoolOperation)wParam)
        {
        case DPO_SETADAPTER:
          pThis->SetAdapter((IListDataAdapter*)lParam);
          break;
        case DPO_GETADAPTER:
          pThis->GetAdapter((IListDataAdapter**)lParam);
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

    case GXWM_IMPULSE:
      if(wParam == 0) {
        pThis->OnImpulse((Marimo::LPCDATAIMPULSE)lParam);
        return 0;
      }
      return 0;

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
          pList = new RichList(szIdName);
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

  List::List( GXLPCWSTR szIdName, GXDWORD bRichList )
    : CtrlBase(szIdName)
    , m_pAdapter      (NULL)
    , m_nTopIndex     (0)
    , m_nColumnCount  (1)
    , m_nColumnWidth  (1)
    , m_nLastSelected (-1)
    , m_nScrolled     (0)
    , m_nPrevScrolled (0)
    , m_nItemHeight   (-1)
    , m_bShowScrollBar(0)
    , m_bShowButtonBox(0)
    , m_bRichList     (bRichList)
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

  void List::DrawScrollBar( GXWndCanvas& canvas, LPGXCRECT lprcClient, GXINT nLastBottom, GXSIZE_T count, GXDWORD dwStyle ) const
  {
    if(IS_LEFTTORIGHT(dwStyle))
    {
      GXINT nTotalWidth = (GXINT)((count + (m_nColumnCount - 1)) / m_nColumnCount * m_nColumnWidth);
      ASSERT(nTotalWidth > 0);
      canvas.FillRect(
        (-m_nScrolled) * lprcClient->right / nTotalWidth,
        lprcClient->bottom - SCROLLBAR_WIDTH, 
        lprcClient->right * lprcClient->right / nTotalWidth,
        SCROLLBAR_WIDTH, 0x80000000);
    }
    else
    {
      GXINT nTotalHeight = nLastBottom;
      ASSERT(nTotalHeight > 0);
      canvas.FillRect(
        lprcClient->right - SCROLLBAR_WIDTH, 
        (-m_nScrolled) * lprcClient->bottom / nTotalHeight,
        SCROLLBAR_WIDTH,
        lprcClient->bottom * lprcClient->bottom / nTotalHeight, 0x80000000);
    }
  }

  GXBOOL List::IsItemSelected(GXINT nItem) const
  {
    return m_aItems[nItem].bSelected;
  }

  int List::OnLButtonDown(int fwKeys, int x, int y)
  {
    if( ! m_pAdapter) {
      return 0;
    }
    GXPOINT pt = {x, y};
    if(gxDragDetect(m_hWnd, &pt))
    {
      GXMSG msg;
      const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
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

  int List::OnLButtonUp(int fwKeys, int x, int y)
  {
    return 0;
  }

  GXBOOL List::EndScroll()
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    GXINT nMinScroll; // (pixel)就是最后一个项目完全显示之后Topmost的那个值

    if(m_bRichList)
    {
      if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) { // [复杂|多列]
        if(TEST_FLAG(dwStyle, GXLBS_LTRSCROLLED))
        {
          const GXINT nModCount = (GXINT)m_pAdapter->GetCount() + (m_nColumnCount - 1);
          nMinScroll = rect.right - (nModCount / m_nColumnCount * m_nColumnWidth);
        }
        else
        {
          const GXINT nModCount = (GXINT)m_pAdapter->GetCount() + (m_nColumnCount - 1);
          nMinScroll = rect.bottom - (nModCount / m_nColumnCount * m_nItemHeight);
        }
      }
      else { // [单列]
        if(TEST_FLAG(dwStyle, GXLBS_LTRSCROLLED)) {
          nMinScroll = (GXINT)(rect.right - m_aItems.size() * m_nColumnWidth);
        }
        else { // [单列|竖直滚动]
          nMinScroll = (GXINT)(rect.bottom - m_aItems.back().nBottom);
        }
      }
    }
    else // [简单List]
    {
      if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) {
        // FIXME: 这个假定高度一致
        nMinScroll = rect.right - m_nColumnCount * m_nColumnWidth;
      }
      else { // [简单|单列] [复杂|单列|竖直滚动]
        nMinScroll = rect.bottom - m_aItems.back().nBottom;
      }
    }



    if(m_nScrolled > 0)
    {
      m_nPrevScrolled = 0;
      gxSetTimer(m_hWnd, IDT_SCROLLUP, 30, NULL);
    }
    else if(m_nScrolled < nMinScroll)
    {
      m_nPrevScrolled = clMin(nMinScroll, 0); // 所有的Item加起来仍小于List的Height，要按照顶部对齐
      gxSetTimer(m_hWnd, IDT_SCROLLDOWN, 30, NULL);
    }
    else {
      return FALSE;
    }

    m_bShowScrollBar = TRUE;
    return TRUE;
  }

  GXVOID List::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    MOVariable varArray = m_pAdapter->GetVariable();
    //ASSERT(varArray.IsSamePool(pImpulse->pDataPool));
    //CLBREAK;

    if(pImpulse->reason == Marimo::DATACT_Change)
    {
      CLBREAK;
      ASSERT(m_pAdapter->GetCount() == m_aItems.size());
      //const GXINT index = pImpulse->index;
      //(GXINT)((GXINT_PTR)pImpulse->ptr - (GXINT_PTR)varArray.GetPtr()) / (varArray.GetSize()/varArray.GetLength());
      //ASSERT(index >= 0);
      //UpdateItemStatus(index, index);
    }
    else if(pImpulse->reason == Marimo::DATACT_Insert)
    {
      OnSyncInsert(pImpulse->index, pImpulse->count);
      ASSERT(m_pAdapter->GetCount() == m_aItems.size());
      //ASSERT(pAction->idx < m_pAdapter->GetCount());
      //SyncItemStatCount();

      ////CLBREAK; // 重新搞定!
      //// 判断是否在中间插入
      //if(pImpulse->index != m_pAdapter->GetCount() - 1)
      //{
      //  UpdateItemStatus(pImpulse->index, -1);
      //}
      //UpdateItemStat(0, -1);
    }
    else if(pImpulse->reason == Marimo::DATACT_Deleting)
    {
      //auto index = pImpulse->index;
      //ASSERT(index >= -1); // -1时删除所有Item
      //ASSERT(m_pAdapter->GetCount() == m_aItems.size());
      //if(index == -1) {
      //  // 删除所有
      //  m_aItems.clear();
      //}
      //else {
      //  ASSERT(index >= 0);
      //  DeleteItemStat(index);
      //}
      //ASSERT(m_pAdapter->GetCount() > (GXINT)m_aItems.size());
      //OnSyncRemove(pImpulse->index, pImpulse->count);
    }
    else if(pImpulse->reason == Marimo::DATACT_Deleted)
    {
      //auto index = pImpulse->index;
      //ASSERT(m_pAdapter->GetCount() == m_aItems.size());
      //ASSERT(index >= -1); // -1时删除所有Item
      //if(index >= 0)
      //{
      //  UpdateItemStatus(index, -1);
      //}
      OnSyncRemove(pImpulse->index, pImpulse->count);
      ASSERT(m_pAdapter->GetCount() == m_aItems.size());
    }
    else
    {
      CLBREAK;
    }

    Invalidate(FALSE);
  }

  int List::SetCurSel(int nIndex)
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

  GXBOOL List::SetAdapter(IListDataAdapter* pAdapter)
  {
    MODataPool* pDataPool = NULL;

    if(m_pAdapter == pAdapter) {
      return FALSE;
    }

    if(m_pAdapter) {
      if(GXSUCCEEDED(m_pAdapter->GetDataPool(&pDataPool)))
      {
        pDataPool->Ignore(&m_pAdapter->GetVariable(), m_hWnd);
        pDataPool->Release();
        pDataPool = NULL;
      }

      m_pAdapter->Release();
      m_pAdapter = NULL;
    }

    m_pAdapter = pAdapter;

    if(m_pAdapter) {
      m_pAdapter->AddRef();

      //SyncItemStatCount();
      m_aItems.clear();
      if(m_pAdapter->GetCount()) {
        OnSyncInsert(0, m_pAdapter->GetCount());
      }

      // 适配器改变通知
      GXNMLISTADAPTER sCreateAdapter;
      sCreateAdapter.hdr.hwndFrom = m_hWnd;
      sCreateAdapter.hdr.idFrom   = 0;
      sCreateAdapter.hdr.code     = GXLBN_ADAPTERCHANGED;
      sCreateAdapter.hTemplateWnd = NULL;
      sCreateAdapter.pAdapter     = pAdapter;

      if(GXSUCCEEDED(pAdapter->GetDataPool(&pDataPool))) {
        pDataPool->Watch(&pAdapter->GetVariable(), m_hWnd);
        pDataPool->Release();
        pDataPool = NULL;
      }

      gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);    

      InvalidateRect(NULL, FALSE);
    }
    else {
      m_aItems.clear();
    }
    return TRUE;
  }

  GXBOOL List::GetAdapter(IListDataAdapter** ppAdapter)
  {
    *ppAdapter = m_pAdapter;
    if(m_pAdapter != NULL) {
      m_pAdapter->AddRef();
    }
    return TRUE;
  }

  //GXBOOL List::ReduceItemStat(GXINT nCount)
  //{
  //  m_aItems.erase(m_aItems.begin() + nCount, m_aItems.end());
  //  return TRUE;
  //}

  //void List::DeleteItemStat(GXINT nIndex)
  //{
  //  m_aItems.erase(m_aItems.begin() + nIndex);
  //}

  //GXBOOL List::SyncItemStatCount()
  //{
  //  const GXUINT nItemCount = (GXUINT)m_aItems.size();
  //  const GXUINT nAdapterCount = m_pAdapter ? m_pAdapter->GetCount() : 0;
  //  if(nAdapterCount == 0) {
  //    m_aItems.clear();
  //    return TRUE;
  //  }

  //  if(nItemCount < nAdapterCount)
  //  {
  //    ITEMSTATUS sis;
  //    for(GXUINT i = nItemCount; i < nAdapterCount; i++) {
  //      m_aItems.push_back(sis);
  //    }
  //    return UpdateItemStatus(nItemCount, nAdapterCount - 1);
  //  }
  //  else if(nItemCount > nAdapterCount)
  //  {
  //    //m_aItems.erase(m_aItems.begin() + nAdapterCount, m_aItems.end());
  //    //return TRUE;
  //    return ReduceItemStat(nAdapterCount);
  //  }
  //  return TRUE;
  //}

  GXINT List::GetCurSel() const
  {
    if(m_nLastSelected >= 0 && m_nLastSelected < (int)m_aItems.size())
    {
      return m_nLastSelected;
    }
    return -1;
    //return m_aItems[m_nLastSelected].bSelected ? m_nLastSelected : -1;
  }

  GXBOOL List::SelectItem(GXINT nItem, GXBOOL bSelected, GXBOOL bNotify)
  {
    ASSERT(m_pAdapter->GetCount() == m_aItems.size());
    GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    if(TEST_FLAG(dwStyle, GXLBS_NOSEL)) {
      return FALSE;
    }

    bNotify = bNotify && TEST_FLAG(dwStyle, GXLBS_NOTIFY);

    //GXBOOL bSelChange = bNotify && bSelected;
    //GXBOOL bSelCancel = bNotify && ( ! bSelected);

    m_aItems[nItem].bSelected = bSelected;

    // 单选时撤销上一次的选择
    if(m_nLastSelected >= 0 && m_nLastSelected != nItem && 
      TEST_FLAG(dwStyle, GXLBS_MULTIPLESEL) == 0)
    {
      m_aItems[m_nLastSelected].bSelected = FALSE;
    }

    m_nLastSelected = nItem;

    if(bNotify) {
      NotifyParent((nItem == -1) ? GXLBN_SELCANCEL : GXLBN_SELCHANGE);
    }

    return TRUE;
  }

  void List::SetScrolledVal(GXINT nScrolled)
  {
    m_nScrolled = nScrolled;
    UpdateTopIndex();
  }

  GXBOOL List::UpdateTopIndex(GXDWORD dwStyle)
  {
    GXINT nScrolled = -m_nScrolled;
    GXINT nItem = 0;

    if( ! m_bRichList && TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      // FIXME: 这个假定高度一致
      GXRECT rect;
      gxGetClientRect(m_hWnd, &rect);
      int nColumn = (nScrolled / m_nColumnWidth);
      nItem = (nColumn == m_nColumnCount ? m_nColumnCount - 1 : nColumn) * (rect.bottom / m_nItemHeight);
      //TRACE("nColumn:%d\n", nColumn);
      m_nTopIndex = clClamp(0, (GXINT)m_pAdapter->GetCount() - 1, nItem);
      TRACE("m_nTopIndex:%d\n", m_nTopIndex);
      return TRUE;
    }
    else if(m_bRichList && TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)/* || m_pAdapter->IsFixedHeight()*/)
    {
      m_nTopIndex = nScrolled / GetItemHeight(0) * m_nColumnCount;
      clClamp((GXSIZE_T)0, m_pAdapter->GetCount() - 1, &m_nTopIndex);
      return TRUE;
    }
    else {
      for(ItemStatusArray::iterator it = m_aItems.begin();
        it != m_aItems.end(); ++it, ++nItem)
      {
        if(it->nBottom > nScrolled) {
          m_nTopIndex = nItem;
          return TRUE;
        }
      }
    }
    return FALSE;
  }

  GXBOOL List::UpdateTopIndex()
  {
    if( ! m_pAdapter) {
      return FALSE;
    }
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    return UpdateTopIndex(dwStyle);
  }

  //GXBOOL List::UpdateItemStatus(GXSIZE_T nBegin, GXSIZE_T nEnd)
  //{
  //  ASSERT(m_pAdapter->GetCount() == m_aItems.size());
  //  const GXSIZE_T nItemCount = m_pAdapter->GetCount();

  //  nBegin = clMax(nBegin, (GXSIZE_T)0);
  //  ASSERT(nBegin < nItemCount);

  //  if(nEnd < 0) {
  //    nEnd = nItemCount - 1;
  //  }
  //  ASSERT(nEnd < nItemCount);

  //  // 上一项的Bottom
  //  GXINT nPrevBottom = nBegin == 0 ? 0 : m_aItems[nBegin - 1].nBottom;
  //  //const GXBOOL bFixedHeight = m_pAdapter->IsFixedHeight();
  //  //const GXINT nItemHeight = bFixedHeight ? VirGetItemHeight(0) : -1;
  //  const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
  //  if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
  //  {
  //    int nVirHeightMax = 0;
  //    for(GXSIZE_T i = nBegin; i <= nEnd; i++)
  //    {
  //      ITEMSTATUS& sis = m_aItems[i];
  //      const int nEndOfRow = (i + 1) % m_nColumnCount;

  //      /*if(bFixedHeight) {
  //      sis.nBottom = nPrevBottom + nItemHeight;

  //      if(nEndOfRow == 0) {
  //      nPrevBottom = sis.nBottom;
  //      }
  //      }
  //      else */{
  //        const GXINT nVirHeight = GetItemHeight(i);
  //        sis.nBottom = nPrevBottom + nVirHeight;
  //        nVirHeightMax = clMax(nVirHeight, nVirHeightMax);

  //        if(nVirHeightMax == 0) {
  //          nPrevBottom += nVirHeightMax;
  //          nVirHeightMax = 0;
  //        }
  //      }

  //      sis.bValidate = TRUE;
  //    }
  //  }
  //  else
  //  {
  //    for(GXSIZE_T i = nBegin; i <= nEnd; i++)
  //    {
  //      ITEMSTATUS& sis = m_aItems[i];
  //      /*if(bFixedHeight) {
  //      sis.nBottom = nPrevBottom + nItemHeight;
  //      }
  //      else */{
  //        sis.nBottom = nPrevBottom + GetItemHeight(i);
  //      }

  //      sis.bValidate = TRUE;
  //      nPrevBottom = sis.nBottom;
  //    }
  //  }
  //  return TRUE;
  //}

  GXBOOL List::IsSelected( GXSIZE_T index ) const
  {
    if(index >= m_aItems.size()) {
      return FALSE;
    }
    return m_aItems[index].bSelected;
  }

  GXINT List::GetItemHeight(GXSIZE_T nIdx) const
  {
    return m_nItemHeight;
  }

  GXBOOL List::OnSyncInsert( GXSIZE_T begin, GXSIZE_T count )
  {
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    //ASSERT(TEST_FLAG_NOT(dwStyle, GXLBS_MULTICOLUMN));
    ASSERT(m_pAdapter->GetCount() == m_aItems.size() + count);
    ITEMSTATUS sItem;
    auto nPrevBottom = (begin == 0) ? 0 : m_aItems[begin - 1].nBottom;
    
    //m_aItems.insert(m_aItems.begin() + begin, count, sItem);
    //auto it = m_aItems.begin() + begin;
    //auto itEnd = (m_aItems.begin() + begin + count);

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      GXRECT rect;
      gxGetClientRect(m_hWnd, &rect);
      BottomToHeight(dwStyle, begin);

      sItem.nBottom = m_nItemHeight;
      m_aItems.insert(m_aItems.begin() + begin, count, sItem);

      HeightToBottom(dwStyle, rect, begin);
      ASSERT(m_aItems.size() < (size_t)m_nColumnCount || m_nColumnCount == DbgCalcColumnCount());
    }
    else
    {
      m_aItems.insert(m_aItems.begin() + begin, count, sItem);
      auto it = m_aItems.begin() + begin;
      auto itEnd = (m_aItems.begin() + begin + count);

      // 设置新增Item的bottom值
      for(; it != itEnd; ++it)
      {
        ASSERT(it->nBottom == -1);
        it->nBottom = nPrevBottom + m_nItemHeight;
        nPrevBottom = it->nBottom;
      }

      itEnd = m_aItems.end();
      nPrevBottom = (GXINT)(m_nItemHeight * count); // 这里开始 nPrevBottom 另作它用

      // 调整插入点之后的Item的bottom值
      for(; it != itEnd; ++it) {
        it->nBottom += nPrevBottom;
      }
    }
    return TRUE;
  }

  GXBOOL List::OnSyncRemove( GXSIZE_T begin, GXSIZE_T count )
  {
    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    //ASSERT(TEST_FLAG_NOT(dwStyle, GXLBS_MULTICOLUMN));
    ASSERT(m_pAdapter->GetCount() + count == m_aItems.size());

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      GXRECT rect;
      gxGetClientRect(m_hWnd, &rect);
      BottomToHeight(dwStyle, begin);

      auto it = m_aItems.begin() + begin;
      m_aItems.erase(it, it + count);

      HeightToBottom(dwStyle, rect, begin);
      ASSERT(m_nColumnCount == DbgCalcColumnCount());
    }
    else
    {
      auto nReduceHeight = m_aItems[begin + count - 1].nBottom - (begin == 0 ? 0 : m_aItems[begin - 1].nBottom);
      auto it = m_aItems.begin() + begin;
      m_aItems.erase(it, it + count);

      it = m_aItems.begin() + begin;
      for(; it != m_aItems.end(); ++it) {
        it->nBottom -= nReduceHeight;
      }
    }
    return TRUE;
  }

  void List::BottomToHeight( GXDWORD dwStyle, GXSIZE_T begin )
  {
    ASSERT(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)); // 只在多列模式下使用
    auto nPrevBottom = begin == 0 ? 0 : m_aItems[begin - 1].nBottom;
    for(auto it = m_aItems.begin() + begin; it != m_aItems.end(); ++it)
    {
      if(nPrevBottom < it->nBottom) {
        auto nBottom = it->nBottom;
        it->nBottom -= nPrevBottom;
        nPrevBottom = nBottom;
      }
      else {
        // 此时 it->nBottom = it->nBottom; 这个就是高度
        nPrevBottom = it->nBottom;
        --m_nColumnCount;
      }
    }
  }

  void List::HeightToBottom( GXDWORD dwStyle, const GXRECT& rcClient, GXSIZE_T begin )
  {
    ASSERT(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)); // 只在多列模式下使用
    auto nPrevBottom = begin == 0 ? 0 : m_aItems[begin - 1].nBottom;
    for(auto it = m_aItems.begin() + begin; it != m_aItems.end(); ++it)
    {
      const GXINT nItemHeight = it->nBottom;
      it->nBottom = nPrevBottom + nItemHeight;
      if(it->nBottom > rcClient.bottom) {
        it->nBottom = nItemHeight;
        ++m_nColumnCount;
      }
      nPrevBottom = it->nBottom;
    }    
  }

  GXINT List::DbgCalcColumnCount()
  {
    GXINT nColumnCount = 1;
    if(m_aItems.empty()) {
      return nColumnCount;
    }

    auto nPrevBottom = m_aItems.front().nBottom;
    for(auto it = m_aItems.begin() + 1; it != m_aItems.end(); ++it)
    {
      if(nPrevBottom >= it->nBottom) {
        nColumnCount++;
      }
      nPrevBottom = it->nBottom;
    }

    return nColumnCount;
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE