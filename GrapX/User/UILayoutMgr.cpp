#ifndef _DEV_DISABLE_UI_CODE
#include "GrapX.H"
#include "User/UILayoutMgr.h"
#include "GrapX/GXUser.H"

namespace GXUI
{
  BinaryPanel::BinaryPanel()
    : m_uPanel        (100)
    , m_dwItemPanel0  (IT_Empty)
    , m_dwItemPanel1  (IT_Empty)
    , m_dwStyle       (NULL)
    //, m_dwPanelType   (PT_Unknown)
    //, m_dwAlignType   (PAT_AlignFirst)
  {
    m_pChild[0] = 0;
    m_pChild[1] = 0;
  }

  BinaryPanel::~BinaryPanel()
  {
    if(m_dwItemPanel0 == IT_Panel) {
      SAFE_DELETE(m_pChild[0]);
    }
    if(m_dwItemPanel1 == IT_Panel) {
      SAFE_DELETE(m_pChild[1]);
    }
  }

  //PanelBase* BinaryPanel::SetPanelItem(int nPart, PanelBase::PanelMode eMode)
  //{
  //  SetType(nPart, PT_Node);
  //  if(eMode == PM_Horz ||
  //    eMode == PM_Vert)
  //  {
  //    ASSERT(m_pChild[nPart] == NULL);
  //    m_pChild[nPart] = new BinaryPanel(eMode);
  //    return m_pChild[nPart];
  //  }
  //  else if(eMode == PM_Quarter)
  //  {
  //    ASSERT(0);
  //  }
  //  return NULL;
  //}
  //void BinaryPanel::SetControlItem(int nPart, GXHWND hWnd)
  //{
  //  SetType(nPart, PT_Control);
  //  ASSERT(m_hWnd[nPart] == NULL);
  //  m_hWnd[nPart] = hWnd;
  //}

  GXLRESULT BinaryPanel::Initialize(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel)
  {
    const GXDWORD dwType = pPanel->dwStyle & LPS_TYPEMASK;
    if(dwType != LPS_HBINARYPANEL && dwType != LPS_VBINARYPANEL) {
      return -1;
    }
    m_dwStyle = pPanel->dwStyle;
    //m_dwPanelType = pPanel->eType;
    //m_dwAlignType = pPanel->eAlignType;
    m_rect = *lpRect;
    //ASSERT(m_dwAlignType >= 0 && m_dwAlignType < 4);

    int n = clMin(2, (int)pPanel->aPanels.size());
    for(int i = 0; i < n; i++)
    {
      const DLGPANEL& ChildPanel = pPanel->aPanels[i];
      const GXDWORD dwChildType = ChildPanel.dwStyle & LPS_TYPEMASK;

      // 计算分割区域
      GXRECT rcPart;
      CalcPanelFactor(lpRect, pPanel->fScale);
      CalcRect(lpRect, i, &rcPart);
      m_pChild[i] = Layout::CreatePanel(hWnd, &rcPart, &ChildPanel);
      if(dwChildType == LPS_WNDITEM) {
        if(m_pChild[i] != NULL) {
          SetItemType(i, IT_Item);
        }
      }
      else if(m_pChild[i] != NULL) {
        SetItemType(i, IT_Panel);
      }
      //CalcRect(lpRect, i, &rcPart);
      //const GXDWORD dwChildType = ChildPanel.dwStyle & LPS_TYPEMASK;
      //if(dwChildType == LPS_VBINARYPANEL || dwChildType == LPS_HBINARYPANEL) {
      //  m_pChild[i] = new BinaryPanel();
      //  m_pChild[i]->Initialize(hWnd, &rcPart, &ChildPanel);
      //  SetItemType(i, IT_Panel);
      //}
      //else if(dwChildType == LPS_WNDITEM) {
      //  m_hWnd[i] = GXGetDlgItemByName(hWnd, ChildPanel.strName.c_str());
      //  if(m_hWnd[i] != NULL) {
      //    SetItemType(i, IT_Item);
      //  }
      //}
    }
    return 0;
  }
  
  void BinaryPanel::OnDrag(GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam)
  {
    GXINT nStart;
    GXINT nEnd;
    //GXLPCWSTR idLoadCursor;
    GXLONG uPanel = 0;
    if(GetPanelType() == LPS_HBINARYPANEL)
    {
      uPanel = ptMouse->y - pptParam->y;
      nStart = m_rect.top;
      nEnd   = m_rect.bottom;
    }
    else if(GetPanelType() == LPS_VBINARYPANEL)
    {
      uPanel = ptMouse->x - pptParam->x;
      nStart = m_rect.left;
      nEnd   = m_rect.right;
    }
    else
      return;
    clClamp(nStart, nEnd, &uPanel);
    if(m_uPanel != uPanel)
    {
      m_uPanel = uPanel;
      OnSize(&m_rect);
    }
  }
  GXBOOL BinaryPanel::HitTest(GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult)
  {
    GXLONG nPos;
    GXLPCWSTR idLoadCursor;

    if(GetPanelType() == LPS_HBINARYPANEL)
    {
      nPos = ptMouse->y;
      idLoadCursor = (GXLPCWSTR)IDC_SIZENS;
    }
    else if(GetPanelType() == LPS_VBINARYPANEL)
    {
      nPos = ptMouse->x;
      idLoadCursor = (GXLPCWSTR)IDC_SIZEWE;
    }
    else
      return FALSE;

    if( ! IsFixed() && 
      nPos >= m_uPanel - c_nRange &&
      nPos <= m_uPanel + c_nRange )
    {
      pResult->ptParam.x =
      pResult->ptParam.y = nPos - m_uPanel;
      pResult->hCursor = gxLoadCursor(NULL, idLoadCursor);
      pResult->pPanel = this;
      return TRUE;
    }
    else if(m_dwItemPanel0 == IT_Panel && nPos < m_uPanel - c_nRange)
    {
      return m_pChild[0]->HitTest(fwKeys, ptMouse, pResult);
    }
    else if(m_dwItemPanel1 == IT_Panel && nPos > m_uPanel + c_nRange)
    {
      return m_pChild[1]->HitTest(fwKeys, ptMouse, pResult);
    }
    return FALSE;
  }

  GXLRESULT BinaryPanel::OnSize(GXRECT* rect)
  {
    // 向后一个对象对齐
    if(GetKeepIndex() == LPS_KEEPSECOND)
    {
      if(GetPanelType() == LPS_VBINARYPANEL) {
        m_uPanel = m_uPanel + rect->right - m_rect.right;
      }
      else if(GetPanelType() == LPS_HBINARYPANEL) {
        m_uPanel = m_uPanel + rect->bottom - m_rect.bottom;
      }
    }
    else
    {
      if(GetPanelType() == LPS_VBINARYPANEL) {
        m_uPanel = m_uPanel + rect->left - m_rect.left;
      }
      else if(GetPanelType() == LPS_HBINARYPANEL) {
        m_uPanel = m_uPanel + rect->top - m_rect.top;
      }
    }

    for(int i = 0; i < 2; i++)
    {
      GXRECT rcPart;
      CalcRect(rect, i, &rcPart);

      // 记录后与限制区域求交, 这样如果有区域被"挤"没了还能恢复
      m_rect = *rect;
      gxIntersectRect(&rcPart, &rcPart, rect);

      switch(GetItemType(i))
      {
      case IT_Empty:
          break;
      case IT_Item:
        gxSetWindowPos(m_hWnd[i], GXHWND_TOP, rcPart.left, rcPart.top, 
          rcPart.right - rcPart.left, rcPart.bottom - rcPart.top, GXSWP_DRAWFRAME|GXSWP_NOACTIVATE);
        //m_pWnd[i]->UpdateWindow();
        break;
      case IT_Panel:
        m_pChild[i]->OnSize(&rcPart);
        //ASSERT(0);
        break;
      }
    }
    return 0;
  }

  //////////////////////////////////////////////////////////////////////////
  BinaryPanel_KeepWeight::BinaryPanel_KeepWeight()
    : m_fWeight(0)
  {
  }

  BinaryPanel_KeepWeight::~BinaryPanel_KeepWeight()
  {
  }
  
  GXLRESULT BinaryPanel_KeepWeight::Initialize(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel)
  {
    m_fWeight = pPanel->fScale[0];
    return BinaryPanel::Initialize(hWnd, lpRect, pPanel);
  }

  void BinaryPanel_KeepWeight::OnDrag(GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam)
  {
    BinaryPanel::OnDrag(fwKeys, ptMouse, pptParam);
    if(GetPanelType() == LPS_VBINARYPANEL) {
      m_fWeight = (float)m_uPanel / (float)(m_rect.right - m_rect.left);
    }
    else if(GetPanelType() == LPS_HBINARYPANEL) {
      m_fWeight = (float)m_uPanel / (float)(m_rect.bottom - m_rect.top);
    }
  }

  GXLRESULT BinaryPanel_KeepWeight::OnSize(GXRECT* rect)
  {
    if(GetPanelType() == LPS_VBINARYPANEL) {
      m_uPanel = (GXLONG)((rect->right - rect->left) * m_fWeight) + rect->left;
    }
    else if(GetPanelType() == LPS_HBINARYPANEL) {
      m_uPanel = (GXLONG)((rect->bottom - rect->top) * m_fWeight) + rect->top;
    }
    return BinaryPanel::OnSize(rect);
  }

  //////////////////////////////////////////////////////////////////////////
  CrossPanel::CrossPanel()
    : m_dwItemPanel0  (IT_Empty)
    , m_dwItemPanel1  (IT_Empty)
    , m_dwItemPanel2  (IT_Empty)
    , m_dwItemPanel3  (IT_Empty)
    , m_dwStyle       (NULL)
    //, m_dwPanelType   (PT_Unknown)
    //, m_dwAlignType   (PAT_AlignFirst)
  {
    m_uPanel[0] = 100;
    m_uPanel[1] = 100;
    for(int i = 0; i < 4; i++) {
      m_pChild[i] = 0;
    }
  }

  CrossPanel::~CrossPanel()
  {
    if(m_dwItemPanel0 == IT_Panel) {
      SAFE_DELETE(m_pChild[0]);
    }
    if(m_dwItemPanel1 == IT_Panel) {
      SAFE_DELETE(m_pChild[1]);
    }
  }

  GXLRESULT CrossPanel::Initialize(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel)
  {
    const GXDWORD dwType = pPanel->dwStyle & LPS_TYPEMASK;
    if(dwType != LPS_CROSSPANEL) {
      return -1;
    }
    m_dwStyle = pPanel->dwStyle;
    //m_dwPanelType = pPanel->eType;
    //m_dwAlignType = pPanel->eAlignType;
    m_rect = *lpRect;
    //ASSERT(m_dwAlignType >= 0 && m_dwAlignType < 4);

    int n = clMin(4, (int)pPanel->aPanels.size());
    for(int i = 0; i < n; i++)
    {
      const DLGPANEL& ChildPanel = pPanel->aPanels[i];
      const GXDWORD dwChildType = ChildPanel.dwStyle & LPS_TYPEMASK;

      // 计算分割区域
      GXRECT rcPart;
      CalcPanelFactor(lpRect, pPanel->fScale);
      CalcRect(lpRect, i, &rcPart);
      m_pChild[i] = Layout::CreatePanel(hWnd, &rcPart, &ChildPanel);
      if(dwChildType == LPS_WNDITEM) {
        if(m_pChild[i] != NULL) {
          SetItemType(i, IT_Item);
        }
      }
      else if(m_pChild[i] != NULL) {
        SetItemType(i, IT_Panel);
      }
    }
    return 0;
  }

  void CrossPanel::OnDrag(GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam)
  {
    GXPOINT ptPanel;
    GXBOOL bChangeSize = FALSE;
    
    if(pptParam->x < INT_MAX)
    {
      ptPanel.x = ptMouse->x - pptParam->x;
      clClamp(m_rect.left, m_rect.right, &ptPanel.x);
      if(m_uPanel[0] != ptPanel.x)
      {
        m_uPanel[0] = ptPanel.x;
        bChangeSize = TRUE;
      }
    }

    if(pptParam->y < INT_MAX)
    {
      ptPanel.y = ptMouse->y - pptParam->y;
      clClamp(m_rect.top, m_rect.bottom, &ptPanel.y);

      if(m_uPanel[1] != ptPanel.y)
      {
        m_uPanel[1] = ptPanel.y;
        bChangeSize = TRUE;
      }
    }

    if(bChangeSize) {
      OnSize(&m_rect);
    }
  }
  GXBOOL CrossPanel::HitTest(GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult)
  {
    //GXLONG nPos;
    //GXLPCWSTR idLoadCursor;
    GXRECT rcCross;

    gxSetRect(&rcCross, m_uPanel[0] - c_nRange, m_uPanel[1] - c_nRange,
      m_uPanel[0] + c_nRange, m_uPanel[1] + c_nRange);

    //if(GetPanelType() == LPS_HBINARYPANEL)
    //{
    //  nPos = ptMouse->y;
    //  idLoadCursor = IDC_SIZENS;
    //}
    //else if(GetPanelType() == LPS_VBINARYPANEL)
    //{
    //  nPos = ptMouse->x;
    //  idLoadCursor = IDC_SIZEWE;
    //}
    //else
    //  return FALSE;

    if( ! IsFixed() )
    {
      pResult->hCursor = NULL;
      if(gxPtInRect(&rcCross, *ptMouse))
      {
        pResult->hCursor = gxLoadCursor(NULL, (GXLPCWSTR)IDC_SIZEALL);
        pResult->ptParam.x = ptMouse->x - m_uPanel[0];
        pResult->ptParam.y = ptMouse->y - m_uPanel[1];
      }
      else if(ptMouse->x >= rcCross.left && ptMouse->x <= rcCross.right)
      {
        pResult->hCursor = gxLoadCursor(NULL, (GXLPCWSTR)IDC_SIZEWE);
        pResult->ptParam.x = ptMouse->x - m_uPanel[0];
        pResult->ptParam.y = INT_MAX;
      }
      else if(ptMouse->y >= rcCross.top && ptMouse->y <= rcCross.bottom)
      {
        pResult->hCursor = gxLoadCursor(NULL, (GXLPCWSTR)IDC_SIZENS);
        pResult->ptParam.x = INT_MAX;
        pResult->ptParam.y = ptMouse->y - m_uPanel[1];
      }
      
      if(pResult->hCursor != NULL)
      {
        pResult->pPanel = this;
        return TRUE;
      }
    }

    if(ptMouse->x < rcCross.left)
    {
      if(ptMouse->y < rcCross.top)
      {
        if(m_dwItemPanel0 == IT_Panel) {
          return m_pChild[0]->HitTest(fwKeys, ptMouse, pResult);
        }
      }
      else if(ptMouse->y > rcCross.bottom)
      {
        if(m_dwItemPanel2 == IT_Panel) {
          return m_pChild[2]->HitTest(fwKeys, ptMouse, pResult);
        }
      }
    }
    else if(ptMouse->x > rcCross.right)
    {
      if(ptMouse->y < rcCross.top)
      {
        if(m_dwItemPanel1 == IT_Panel) {
          return m_pChild[1]->HitTest(fwKeys, ptMouse, pResult);
        }
      }
      else if(ptMouse->y > rcCross.bottom)
      {
        if(m_dwItemPanel3 == IT_Panel) {
          return m_pChild[3]->HitTest(fwKeys, ptMouse, pResult);
        }
      }
    }

    //else if(m_dwItemPanel0 == IT_Panel && nPos < m_uPanel - c_nRange)
    //{
    //  return m_pChild[0]->HitTest(fwKeys, ptMouse, pResult);
    //}
    //else if(m_dwItemPanel1 == IT_Panel && nPos > m_uPanel + c_nRange)
    //{
    //  return m_pChild[1]->HitTest(fwKeys, ptMouse, pResult);
    //}
    return FALSE;
  }

  GXLRESULT CrossPanel::OnSize(GXRECT* rect)
  {
    // 向后一个对象对齐
    if(GetKeepIndex() == LPS_KEEPSECOND)
    {
      m_uPanel[0] = m_uPanel[0] + rect->right - m_rect.right;
    }
    else if(GetKeepIndex() == LPS_KEEPTHIRD)
    {
      m_uPanel[1] = m_uPanel[1] + rect->bottom - m_rect.bottom;
    }
    else if(GetKeepIndex() == LPS_KEEPFOURTH)
    {
      m_uPanel[0] = m_uPanel[0] + rect->right - m_rect.right;
      m_uPanel[1] = m_uPanel[1] + rect->bottom - m_rect.bottom;
    }

    for(int i = 0; i < 4; i++)
    {
      GXRECT rcPart;
      CalcRect(rect, i, &rcPart);
      m_rect = *rect;
      gxIntersectRect(&rcPart, &rcPart, rect);
      switch(GetItemType(i))
      {
      case IT_Empty:
        break;
      case IT_Item:
        gxSetWindowPos(m_hWnd[i], GXHWND_TOP, rcPart.left, rcPart.top, 
          rcPart.right - rcPart.left, rcPart.bottom - rcPart.top, SWP_DRAWFRAME|SWP_NOACTIVATE);
        //m_pWnd[i]->UpdateWindow();
        break;
      case IT_Panel:
        m_pChild[i]->OnSize(&rcPart);
        //ASSERT(0);
        break;
      }
    }
    return 0;
  }



  LineWrapPanel::LineWrapPanel()
  {
  }

  LineWrapPanel::~LineWrapPanel()
  {
  }

  GXLRESULT LineWrapPanel::Initialize(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel)
  {
    m_aItems.reserve(pPanel->aPanels.size());
    for(DlgPanelArray::const_iterator it = pPanel->aPanels.begin();
      it != pPanel->aPanels.end(); ++it)
    {
      WNDITEM sItem;
      sItem.hWnd = GXGetDlgItemByName(hWnd, it->strName);
      sItem.dwStyle = it->dwStyle;
      m_aItems.push_back(sItem);
    }
    return GX_OK;
  }

  GXBOOL LineWrapPanel::HitTest(GXWPARAM fwKeys, GXPOINT* ptMouse, HITTESTRESULT* pResult)
  {
    return FALSE;
  }

  void LineWrapPanel::OnDrag(GXWPARAM fwKeys, GXPOINT* ptMouse, GXPOINT* pptParam)
  {
  }

  GXLRESULT LineWrapPanel::OnSize(GXRECT* rect)
  {
    typedef clvector<GXRECT> RectArray;
    //RectArray aRects;
    //aRects.reserve(m_aItems.size());

    GXPOINT cur = {rect->left, rect->top};
    int nMaxHeight = 0;
    for(WndItemArray::const_iterator it = m_aItems.begin();
      it != m_aItems.end(); ++it)
    {
      GXRECT rcWnd;
      if(gxGetWindowRect(it->hWnd, &rcWnd))
      {
        const auto nWidth = rcWnd.right - rcWnd.left;
        if(cur.x + nWidth > rect->right && nMaxHeight > 0)
        {
          cur.x = rect->left;
          cur.y += (nMaxHeight + c_nGap);
          nMaxHeight = 0;
        }
        gxSetWindowPos(it->hWnd, NULL, cur.x, cur.y, 0, 0, GXSWP_NOSIZE);
        cur.x += (nWidth + c_nGap);
        nMaxHeight = clMax(nMaxHeight, rcWnd.bottom - rcWnd.top);
        if(TEST_FLAG(it->dwStyle, LPS_INT_RETURN)) {
          cur.x = rect->right; // 占满宽度, 使强制换行
        }

        //aRects.push_back(rcWnd);
      }
    }

    return GX_OK;
  }


  //////////////////////////////////////////////////////////////////////////
  Layout::Layout(GXHWND hWndOwner)
    : m_hWndOwner   (hWndOwner)
    , m_pRoot       (NULL)
    , m_fwPrevKeys  (NULL)
    , m_pFocusPanel (NULL)
    , m_hCursor     (NULL)
  {
    //if(eRootMode == PanelBase::PM_Horz || eRootMode == PanelBase::PM_Vert)
    //{
    //  m_pRoot = new BinaryPanel(eRootMode);
    //}
    //RECT rect;
    //pParentWnd->GetClientRect(&rect);
    //m_hWnd = CreateWindow(L"Partition", NULL, WS_CHILD|WS_VISIBLE,
    //  0, 0, 0, 0, pParentWnd->Handle(), NULL, hInst, this);
  }
  Layout::~Layout()
  {
    SAFE_DELETE(m_pRoot);
  }


  GXLRESULT Layout::Initialize(const DLGPANEL* pPanel)
  {
    ASSERT(m_pRoot == NULL);
    GXRECT rect;
    gxGetClientRect(m_hWndOwner, &rect);
    m_pRoot = CreatePanel(m_hWndOwner, &rect, pPanel);
    //const GXDWORD dwType = pPanel->dwStyle & LPS_TYPEMASK;
    //if(dwType == LPS_HBINARYPANEL || dwType == LPS_VBINARYPANEL) {
    //  m_pRoot = new BinaryPanel;
    //}
    //GXRECT rect;
    //gxGetClientRect(m_hWndOwner, &rect);
    //GXHRESULT hval = m_pRoot->Initialize(m_hWndOwner, &rect, pPanel);
    if(m_pRoot != NULL) {
      GXSIZE size;
      size.cx = rect.right;
      size.cy = rect.bottom;
      return OnSize(NULL, size);
    }
    return -1;
  }

  GXLRESULT Layout::Finalize()
  {
    return 0;
  }

  //GXLRESULT GXCALLBACK Layout::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  //{
  //  Layout* pThis = (Layout*)GetWindowLong(hWnd, 0);
  //  switch(message)
  //  {
  //  case WM_SETCURSOR:
  //    {
  //      GXHWND hWnd = (GXHWND)wParam;       // handle of window with cursor 
  //      int nHittest = LOWORD(lParam);  // hit-test code 
  //      GXDWORD wMouseMsg = HIWORD(lParam); // mouse-message identifier 
  //      return pThis->OnSetCursor(hWnd, nHittest, wMouseMsg);
  //    }
  //    break;
  //  case WM_MOUSEMOVE:
  //    {
  //      POINT ptMouse;
  //      ptMouse.x = (LONG)(short)GXLOWORD(lParam);
  //      ptMouse.y = (LONG)(short)GXHIWORD(lParam);
  //      pThis->OnMouseMove(wParam, &ptMouse);
  //    }
  //    break;
  //  case WM_LBUTTONDOWN:
  //    {
  //      POINT ptMouse;
  //      ptMouse.x = (LONG)(short)GXLOWORD(lParam);
  //      ptMouse.y = (LONG)(short)GXHIWORD(lParam);
  //      pThis->OnLButtonDown(wParam, &ptMouse);
  //    }
  //    break;
  //  case WM_LBUTTONUP:
  //    {
  //      POINT ptMouse;
  //      ptMouse.x = (LONG)(short)GXLOWORD(lParam);
  //      ptMouse.y = (LONG)(short)GXHIWORD(lParam);
  //      pThis->OnLButtonUp(wParam, &ptMouse);
  //    }
  //    break;
  //  case WM_NCCREATE:
  //    {
  //      LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
  //      SetWindowLong(hWnd, 0, (LONG)lpcs->lpCreateParams);
  //    }
  //    return TRUE;
  //  case WM_SIZE:
  //    {
  //      GXSIZE size = {GXLOWORD(lParam), GXHIWORD(lParam)};
  //      return pThis->OnSize(wParam, size);
  //    }
  //    break;
  //  case WM_PAINT:
  //    ValidateRect(hWnd, NULL);
  //    break;
  //  default:
  //    return DefWindowProc(hWnd, message, wParam, lParam);
  //  }
  //  return 0;
  //}
  //HRESULT Layout::RegisterClass(HINSTANCE hInst)
  //{
  //  if(s_uRefCount == 0)
  //  {
  //    WNDCLASSEX wcex;

  //    wcex.cbSize = sizeof(WNDCLASSEX);

  //    wcex.style      = CS_HREDRAW | CS_VREDRAW;
  //    wcex.lpfnWndProc  = Layout::WndProc;
  //    wcex.cbClsExtra    = 0;
  //    wcex.cbWndExtra    = sizeof(Layout*);
  //    wcex.hInstance    = hInst;
  //    wcex.hIcon      = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TEST_WIN32_GXFC));
  //    wcex.hCursor    = LoadCursor(NULL, IDC_ARROW);
  //    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  //    wcex.lpszMenuName  = NULL;
  //    wcex.lpszClassName  = L"Partition";
  //    wcex.hIconSm    = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  //    RegisterClassEx(&wcex);
  //  }
  //  s_uRefCount++;
  //  return s_uRefCount;
  //}
  //HRESULT Layout::UnregisterClass()
  //{
  //  s_uRefCount--;
  //  if(s_uRefCount == 0)
  //  {
  //    //UnregisterClass();
  //    return S_OK;
  //  }
  //  return s_uRefCount;
  //}
  PanelBase* Layout::GetRoot()
  {
    return m_pRoot;
  }

  GXLRESULT Layout::OnSize(GXDWORD fwSizeType, GXSIZE& size)
  {
    GXRECT rect = {0, 0, size.cx, size.cy};
    return m_pRoot->OnSize(&rect);
    //return CMOWnd::OnSize(fwSizeType, size);
  }
  
  GXBOOL Layout::OnSetCursor(int nHittest)
  {
    //POINT ptScreen;
    //GetCursorPos(&ptScreen);
    //ScreenToClient(m_Wnd,)
    //PanelBase::HITTESTRESULT HitTestResult;
    //if(m_pRoot->HitTest(fwKeys, ptMouse, &HitTestResult) != FALSE)

    if(nHittest == HTCLIENT && m_hCursor != NULL)
    {
      gxSetCursor(m_hCursor);
      return TRUE;
    }
    return FALSE;
  }

  GXLRESULT Layout::OnLButtonDown(GXWPARAM fwKeys, GXPOINT* ptMouse)
  {
    PanelBase::HITTESTRESULT HitTestResult;
    if(m_pRoot->HitTest(fwKeys, ptMouse, &HitTestResult) != FALSE)
    {
      m_pFocusPanel = HitTestResult.pPanel;
    }
    gxSetCapture(m_hWndOwner);
    return 0;
  }

  GXLRESULT Layout::OnLButtonUp(GXWPARAM fwKeys, GXPOINT* ptMouse)
  {
    m_pFocusPanel = NULL;
    gxReleaseCapture();
    return 0;
  }

  //GXBOOL Layout::OnSetCursor(GXHWND hWnd, int nHittest, GXWORD wMouseMsg)
  //{
  //  if(m_hCursor != NULL)
  //    gxSetCursor(m_hCursor);
  //  return FALSE;
  //}
  
  GXVOID Layout::OnMouseMove(GXWPARAM fwKeys, GXPOINT* ptMouse)
  {
    if(m_pRoot != NULL)
    {
      //PanelBase::HITTESTRESULT HitTestResult;
      if(m_pFocusPanel != NULL)
      {
        if((fwKeys & MK_LBUTTON) != 0)
        {
          m_pFocusPanel->OnDrag(fwKeys, ptMouse, &m_ptParam);
        }
        else
          OnLButtonUp(fwKeys, ptMouse);
      }
      //else if(m_pRoot->HitTest(fwKeys, ptMouse, &HitTestResult) != FALSE)
      //{
      //  m_ptParam = HitTestResult.ptParam;
      //  m_hCursor = HitTestResult.hCursor;
      //}
      else
      {
        m_hCursor = NULL;
      }
    }
    m_fwPrevKeys = fwKeys;
  }

  GXBOOL Layout::OnHitTest(const GXPOINT* ptCursor)
  {
    if(m_pFocusPanel != NULL) {
      return FALSE;
    }

    GXPOINT ptClient = *ptCursor;
    PanelBase::HITTESTRESULT HitTestResult;
    gxScreenToClient(m_hWndOwner, &ptClient);

    if(m_pRoot->HitTest(NULL, &ptClient, &HitTestResult) != FALSE)
    {
      m_ptParam = HitTestResult.ptParam;
      m_hCursor = HitTestResult.hCursor;
      return TRUE;
    }
    return FALSE;
  }

  PanelBase* Layout::CreatePanel(GXHWND hWnd, GXLPCRECT lpRect, const DLGPANEL* pPanel)
  {
    const GXDWORD dwType = pPanel->dwStyle & LPS_TYPEMASK;
    const GXDWORD dwKeep = pPanel->dwStyle & LPS_KEEPMASK;
    PanelBase* pPanelObj = NULL;
    if(dwType == LPS_VBINARYPANEL || dwType == LPS_HBINARYPANEL) {
      if(dwKeep == LPS_KEEPWEIGHT) {
        pPanelObj = new BinaryPanel_KeepWeight();
      }
      else {
        pPanelObj = new BinaryPanel();
      }
      //BinaryPanel* pBinPanel = new BinaryPanel();
      //pBinPanel->Initialize(hWnd, lpRect, pPanel);
      //return pBinPanel;
    }
    else if(dwType == LPS_CROSSPANEL)
    {
      pPanelObj = new CrossPanel();

      //CrossPanel* pCrossPanel = new CrossPanel;
      //pCrossPanel->Initialize(hWnd, lpRect, pPanel);
      //return pCrossPanel;
    }
    else if(dwType == LPS_LINEWRAP)
    {
      pPanelObj = new LineWrapPanel();
    }
    else if(dwType == LPS_WNDITEM) {
      GXHWND hChildWnd = GXGetDlgItemByName(hWnd, pPanel->strName);
      return (PanelBase*)hChildWnd;
    }

    if(pPanelObj != NULL)
    {
      pPanelObj->Initialize(hWnd, lpRect, pPanel);
    }
    return pPanelObj;
  }
} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE