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
//#include "Include/guxtheme.h"
#include "clStringSet.h"

#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GXUICtrlBase.h"
#include "GXUIList.h"
#include "GXUIList_Custom.h"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXKernel.H"
#include "GrapX/gxDevice.H"
#include "RichListAdapter.h"

#define SCROLLBAR_WIDTH 10
#define CHECKBOX_SIZE 10
#define ITEM_LINE_POS(_ITEM, _SIZE)  (m_nScrolled + _ITEM / m_nColumnCount * _SIZE) // LEFTTORIGHT 时是Left位置，否则是Top位置
#define ITEM_COLUMN_POS(_ITEM, _SIZE)  (_ITEM % m_nColumnCount * _SIZE)             // LEFTTORIGHT 时是Top位置，否则是Left位置
using namespace clstd;

GXHWND gxIntCreateDialogFromFileW(GXHINSTANCE  hInstance, GXLPCWSTR lpFilename, GXLPCWSTR lpDlgName, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);
//#ifdef _DEBUG
//GXDWORD g_nDbg = 0;
//#endif // _DEBUG
namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////
  GXLRESULT GXCALLBACK CustomizeList::CustomWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    switch(message)
    {
    case GXWM_ERASEBKGND:
      {
        GXHWND hParent = gxGetParent(hWnd);
        GXRECT rect;
        GXBOOL bSelected = FALSE;
        CustomizeList* pList = (CustomizeList*)gxGetWindowLong(hParent, 0);
        const int nMyIndex = gxGetWindowLong(hWnd, GXGWL_USERDATA);

        gxGetClientRect(hWnd, &rect);
        if(pList != NULL)
        {
          // 如果是多列的话,只有最右一列才减掉滚动条区域
          //if(pList->m_bShowScrollBar && (((nMyIndex + 1) % pList->m_nColumnCount) == 0)) {
          //  rect.right -= SCROLLBAR_WIDTH;
          //}
          bSelected = pList->m_aItemStat[nMyIndex].bSelected;
        }

        GXHDC hdc = (GXHDC)wParam;
        LPGXWNDCANVAS canvas = GXGetWndCanvas(hdc);
        canvas->FillRect(&rect, gxGetSysColor(bSelected ? GXCOLOR_HIGHLIGHT : GXCOLOR_WINDOW));
        //gxFillRect(hdc, &rect, (GXHBRUSH)gxGetStockObject(bSelected ? GXGRAY_BRUSH : GXWHITE_BRUSH));

//#ifdef _DEBUG
//        if(g_nDbg == 1)
//        {
//          GXRECT rcUpdate;
//          gxGetUpdateRect(hWnd, &rcUpdate, FALSE);
//          gxFillRect(hdc, &rect, (GXHBRUSH)gxGetStockObject(GXGRAY_BRUSH));
//          //canvas.GetPaintRect(&rcUpdate);
//          //canvas.FillRect(rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom, 0xff00ff00);
//        }
//        g_nDbg ^= 1;
//#endif // _DEBUG

      }
      break;
    case GXWM_COMMAND:
      {
        // 将子控件 COMMAND 消息转换为 NOTIFY 消息
        GXHWND hParent = gxGetWindow(hWnd, GXGW_PARENT);
        if(hParent) {
          GXNMCUSTLISTCTRLCMD UIList;
          UIList.hdr.hwndFrom = hParent;  // List Box Handle
          UIList.hdr.idFrom   = GXLOWORD(wParam);
          UIList.hdr.code     = GXLBN_CUSTCTRLCMD;
          UIList.nCommand     = GXHIWORD(wParam);
          UIList.nListItem    = gxGetWindowLong(hWnd, GXGWL_USERDATA);
          UIList.hTmplItemWnd = (GXHWND)lParam;

          gxSendMessage(hParent, GXWM_NOTIFY, UIList.hdr.idFrom, (GXLPARAM)&UIList);
        }
      }
      break;
    //case GXWM_LBUTTONDOWN:
    //  CLBREAK;
    //  break;
    case GXWM_NCHITTEST:
      {
        //GXLRESULT lval = gxDefWindowProc(hWnd, message, wParam, lParam);
        GXPOINT pt = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        GXHWND hChild = gxChildWindowFromPoint(hWnd, pt);
        gxSetWindowLong(hWnd, GXDWL_MSGRESULT, hChild == NULL ? GXHTNOWHERE : GXHTCLIENT);  //FIXME: 直接返回 GXHTCLIENT 可能不对
      }
      break;
    default:
      return FALSE;
    }
    return TRUE;
  }

  GXBOOL GXCALLBACK CustomizeList::EnumChildProc(GXHWND hWnd, GXLPARAM lParam)
  {
    CustomizeList* pThis = (CustomizeList*)lParam;
    GXLPCWSTR szName = (GXLPCWSTR)gxSendMessage(hWnd, GXWM_GETIDNAMEW, 0, 0);

    if(szName != NULL && szName[0] != '\0') {
      pThis->m_aElementName.push_back(szName);
    }
    return TRUE;
  }

  GXBOOL GXCALLBACK CustomizeList::EnumAdapterChildProc(GXHWND hWnd, GXLPARAM lParam)
  {
    ItemElementArray* pItemElements = (ItemElementArray*)lParam;
    ITEMELEMENT sElement;
    GXWCHAR szClassName[128];
    CustomizeList* pThis = (CustomizeList*)lParam;
    gxGetClassName(hWnd, szClassName, sizeof(szClassName) / sizeof(szClassName[0]));

    GXLPCWSTR szCtrlName = (GXLPCWSTR)gxSendMessage(hWnd, GXWM_GETIDNAMEW, 0, 0);
    if(szCtrlName == NULL || szCtrlName[0] == '\0') {
      return TRUE;
    }

    sElement.strName   = szCtrlName;
    sElement.dwStyle   = gxGetWindowLong(hWnd, GXGWL_STYLE);
    sElement.dwExStyle = gxGetWindowLong(hWnd, GXGWL_EXSTYLE);
    sElement.strClass  = szClassName;
    pItemElements->push_back(sElement);

    //pThis->m_aItemElement.push_back(sElement);
    return TRUE;
  }

  GXINT CustomizeList::GetItemHeight(GXINT nIdx) const
  {
    const GXINT nItemHeight = m_pAdapter->GetItemHeight(nIdx);
    if(nItemHeight < 0) {
      GXRECT rect;
      gxGetWindowRect(m_hPrototype, &rect);
      return rect.bottom - rect.top;
    }
    return nItemHeight;
  }

  GXHWND CustomizeList::PlantCustItem(int nIndex, GXLPCRECT lprect)
  {
    GXHWND hWnd = NULL;
    if(m_aItemStat[nIndex].hItem == NULL)
    {
      if(m_aWndPool.size() == 0)
      {
        m_strTemplate.Find(L':');
        clStringArrayW aString;
        ResolveString(m_strTemplate, L':', aString);
        if(aString.size() != 2) {
          return NULL;
        }
        hWnd = gxIntCreateDialogFromFileW(NULL, aString[0], aString[1], m_hWnd, (GXDLGPROC)CustomWndProc, NULL);
        gxShowWindow(hWnd, GXSW_SHOW);
      }
      else {
        hWnd = m_aWndPool.back();
        m_aWndPool.pop_back();
      }
      m_aItemStat[nIndex].hItem = hWnd;
      gxSetWindowPos(hWnd, NULL, lprect->left, lprect->top, lprect->right - lprect->left, lprect->bottom - lprect->top, GXSWP_NOSIZE|GXSWP_SHOWWINDOW|GXSWP_NOACTIVATE);
      gxSetWindowLong(hWnd, GXGWL_USERDATA, nIndex);
      UpdateCustItemText(nIndex, lprect);
    }
    else {
      hWnd = m_aItemStat[nIndex].hItem;
    }
    ASSERT(gxIsWindow(hWnd));
    return hWnd;
  }

  void CustomizeList::Recycle(int nIndex)
  {
    ASSERT(m_aItemStat[nIndex].hItem != NULL)

      // 回收后需要隐藏,因为删除Item并且Items不足一页时会有多余ItemWnd
    gxShowWindow(m_aItemStat[nIndex].hItem, GXSW_HIDE);

    m_aWndPool.push_back(m_aItemStat[nIndex].hItem);
    m_aItemStat[nIndex].hItem = NULL;
  }

  int CustomizeList::Recycle(int nBegin, int nDir)
  {
    ASSERT(nDir == -1 || nDir == 1);
    ASSERT(nBegin >= -1);
    int nEnd = -1;
    int nCount = 0;
    if(nDir == 1) {
      nEnd = (int)m_aItemStat.size();
    }

    while(nBegin != nEnd)
    {
      if(m_aItemStat[nBegin].hItem != NULL) {
        Recycle(nBegin);
        nCount++;
      }
      else if(nCount) { // 防止第一个hItem为NULL时返回
        break;
      }
      nBegin += nDir;
    }
    return nCount;
  }

  GXBOOL CustomizeList::UpdateCustItemText(int nIndex, GXLPCRECT rcItem)
  {
    //clStringW strText;
    GXHWND hItem = m_aItemStat[nIndex].hItem;
    ASSERT(hItem != NULL);
    ListDataAdapter::GETSTRW ItemStrDesc;
    for(clStringArrayW::iterator it = m_aElementName.begin();
      it != m_aElementName.end(); ++it)
    {
      GXHWND hElement = GXGetDlgItemByName(hItem, *it);
      if(hElement != NULL) {
        ItemStrDesc.nIdx     = nIndex;
        ItemStrDesc.nElement = it - m_aElementName.begin();
        ItemStrDesc.hItemWnd = hElement;
        ItemStrDesc.szName   = *it;
        ItemStrDesc.rect     = *rcItem;
        if(m_pAdapter->GetStringW(&ItemStrDesc) && ( ! ItemStrDesc.VarString.IsValid() || 
          gxSendMessage(hElement, GXWM_DATAPOOLOPERATION, DPO_SETVARIABLE, (GXLPARAM)&ItemStrDesc.VarString) < 0))
        {
          gxSetWindowTextW(hElement, ItemStrDesc.sString);
        }
        ItemStrDesc.sString.Clear();
        ItemStrDesc.VarString.Free();
      }
    }
    return TRUE;
  }

  List::ListType CustomizeList::GetListType()
  {
    return LT_Custom;
  }

  GXLRESULT CustomizeList::Measure(GXRegn* pRegn)
  {
    if(m_hPrototype == NULL)
    {
      pRegn->Set(0, 0, 128, 128);
    }
    else {
      GXRECT rect;
      gxGetWindowRect(m_hPrototype, &rect);
      pRegn->Set(0, 0, rect.right - rect.left, (rect.bottom - rect.top) * 5);
    }
    return 0;
  }

  int CustomizeList::OnCreate(GXCREATESTRUCT* pCreateParam)
  {
    int result = List::OnCreate(pCreateParam);
    if(result < 0) {
      return result;
    }
    return 0;
  }

  GXINT CustomizeList::HitTest(int fwKeys, int x, int y) const
  {
    if(m_aItemStat.empty()) {
      return -1;
    }

    GXRECT rcItem;
    GXPOINT pt = {x, y};
    const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    ItemStatArray::const_iterator it = m_aItemStat.begin() + m_nTopIndex;

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      for(; it < m_aItemStat.end(); ++it)
      {
        int nItem = it - m_aItemStat.begin();
        GetItemRect(nItem, dwStyle, &rcItem);
        if(gxPtInRect(&rcItem, pt)) {
          return nItem;
        }
      }
    }
    else
    {
      GXRECT rcClient;
      gxGetClientRect(m_hWnd, &rcClient);

      // 扩展的边
      const int nExtEdge = (IS_LEFTTORIGHT(dwStyle) 
        ? offsetof(GXRECT, bottom)
        : offsetof(GXRECT, right)) / sizeof(GXLONG);

      STATIC_ASSERT(sizeof(rcClient.bottom) == sizeof(GXLONG)); // 保证GXRECT与GXLONG类型长度一致

      for(; it < m_aItemStat.end(); ++it)
      {
        int nItem = it - m_aItemStat.begin();
        GetItemRect(nItem, dwStyle, &rcItem);
        ((GXLONG*)&rcItem)[nExtEdge] = ((GXLONG*)&rcClient)[nExtEdge];
        if(gxPtInRect(&rcItem, pt)) {
          return nItem;
        }
      }
    }

    return -1;
  }


  int CustomizeList::OnLButtonDown(int fwKeys, int x, int y)
  {
    return ListTemplate<CUSTOMIZEITEMSTAT>::OnLButtonDown(fwKeys, x, y);
  }

  GXLRESULT CustomizeList::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    // 如果没有条目, 填充背景区域
    if(m_aItemStat.size() == 0) {
      canvas.FillRect(rect.left, rect.top, rect.right, rect.bottom, 0xffffffff);
      return 0;
    }

    GXRECT rcGap; // 左上的终止位置，右下的起始位置
    const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);

    GXBOOL bFixedHeight = m_pAdapter->IsFixedHeight();
    GXRECT rcTemplProto;

    gxGetWindowRect(m_hPrototype, &rcTemplProto);
    const int TemplProtoWidth = rcTemplProto.right - rcTemplProto.left;
    const int TemplProtoHeight = rcTemplProto.bottom - rcTemplProto.top;

    GXINT nHeight = GetItemHeight(0);
    GXINT nCount = m_pAdapter->GetItemCount();
    GXINT nPassPos = 0; // 用于DialogTemplate位置的累加计数

    ASSERT(m_nTopIndex >= 0 && m_nTopIndex < nCount);

    // 填充Item右侧空白区, 如果有的话
    //int nRightCap = m_nColumnCount * TemplProtoWidth - SCROLLBAR_WIDTH;
    //if(nRightCap < rect.right) {
    //  canvas.FillRect(nRightCap, rect.top, rect.right - nRightCap, rect.bottom, 0xffffffff);
    //}
    if(IS_LEFTTORIGHT(dwStyle)) {
      rcGap.left = clMax(m_nScrolled, 0);
      rcGap.top = 0;      
      rcGap.bottom = m_nColumnCount * TemplProtoHeight/* - SCROLLBAR_WIDTH*/;
    }
    else {
      rcGap.left = 0;
      rcGap.top = clMax(m_nScrolled, 0);
      rcGap.right = m_nColumnCount * TemplProtoWidth/* - SCROLLBAR_WIDTH*/;
    }

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) 
    {
      if(IS_LEFTTORIGHT(dwStyle)) {
        //nPassPos = m_nScrolled + m_nTopIndex / m_nColumnCount * TemplProtoWidth;
        nPassPos = ITEM_LINE_POS(m_nTopIndex, TemplProtoWidth);
        rcGap.right = nPassPos + TemplProtoWidth;  // 后面累加
      }
      else {
        //nPassPos = m_nScrolled + m_nTopIndex / m_nColumnCount * nHeight;
        nPassPos = ITEM_LINE_POS(m_nTopIndex, nHeight);
        rcGap.bottom = nPassPos + TemplProtoHeight;  // 后面累加
      }
    }
    else if(bFixedHeight) {
      if(IS_LEFTTORIGHT(dwStyle)) {
        nPassPos = m_nScrolled + m_nTopIndex * TemplProtoWidth;
      }
      else {
        nPassPos = m_nScrolled + m_aItemStat[m_nTopIndex].nBottom - nHeight;
      }
    }
    else {
      ASSERT(0); // FIXME: 起始(与 nHeight 相关)位置计算不对, 要修正
      nPassPos = m_nScrolled + m_aItemStat[m_nTopIndex].nBottom - nHeight;
    }


    clStringW strItem;
    GXRECT  rcItem = {0,0,0,0};
    int i = m_nTopIndex;
    Recycle(i - 1, -1);
    for(; i < nCount; i++)
    {
      //GXINT nStrLen = m_pAdapter->GetStringW(i, NULL, strItem);
      CUSTOMIZEITEMSTAT& ItemStat = m_aItemStat[i];
      GXColor32 crText = 0xff000000;  // TODO: 用系统设置代替
      if(ItemStat.bSelected) {        // 绘制选择项目的底色
        crText = 0xffffffff;
      }
      
      if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
      {
        if(IS_LEFTTORIGHT(dwStyle))
        {
          const int yi = (i % m_nColumnCount);
          rcItem.left   = nPassPos;
          rcItem.top    = yi * TemplProtoHeight;
          rcItem.right  = nPassPos + TemplProtoWidth;
          rcItem.bottom = rcItem.top + TemplProtoHeight;

          PlantCustItem(i, &rcItem);

          if(yi + 1 == m_nColumnCount) {
            nPassPos = rcGap.right;
            rcGap.right += TemplProtoWidth;
          }
        }
        else
        {
          const int xi = (i % m_nColumnCount);
          rcItem.left   = xi * TemplProtoWidth;
          rcItem.top    = nPassPos;
          rcItem.right  = rcItem.left + TemplProtoWidth;
          rcItem.bottom = nPassPos + TemplProtoHeight;

          PlantCustItem(i, &rcItem);

          if(xi + 1 == m_nColumnCount) {
            nPassPos = rcGap.bottom; 
            rcGap.bottom += TemplProtoHeight;
          }
        }
      }
      else {
        //CLBREAK; // 修改后目前这个不支持

        if(IS_LEFTTORIGHT(dwStyle))
        {
          gxSetRect(&rcItem, nPassPos, 0, nPassPos + TemplProtoWidth, TemplProtoHeight);
          nPassPos += TemplProtoWidth;
          rcGap.right = nPassPos;
        }
        else
        {
          if( ! bFixedHeight) {
            nHeight = GetItemHeight(i);
          }
          gxSetRect(&rcItem, 0, nPassPos, TemplProtoWidth, nPassPos + nHeight);
          nPassPos += nHeight;
          rcGap.bottom = nPassPos;

          // 如果模板的宽度不足以填充整个客户区，这里用颜色填充
          if(TemplProtoWidth < rect.right) {
            canvas.FillRect(TemplProtoWidth, rcItem.top, rect.right - TemplProtoWidth, 
              rcItem.bottom - rcItem.top, gxGetSysColor(ItemStat.bSelected ? GXCOLOR_HIGHLIGHT : GXCOLOR_WINDOW));
          }
        }
        PlantCustItem(i, &rcItem);
      }


      if((IS_LEFTTORIGHT(dwStyle) && nPassPos >= rect.right) ||
        ( ! IS_LEFTTORIGHT(dwStyle) && nPassPos >= rect.bottom))
      {
        i++;  // 回收从这个开始
        break;
      }
    } // for each item


    // 填充最后一个Item之后一行的空白
    // 如果这个Item恰好在行尾，那么调整rcGap.bottom或rcGap.right
    if(IS_LEFTTORIGHT(dwStyle)) {
      ASSERT(TEST_FLAG_NOT(dwStyle, GXLBS_MULTICOLUMN) || nPassPos < rcGap.right);
      if(rcItem.bottom < rcGap.bottom) {        
        canvas.FillRect(nPassPos, rcItem.bottom, rcGap.right - nPassPos, rcGap.bottom - rcItem.bottom, 0xffffffff);
      }
      else {
        rcGap.right -= TemplProtoWidth;
      }
    }
    else {
      ASSERT(TEST_FLAG_NOT(dwStyle, GXLBS_MULTICOLUMN) || nPassPos < rcGap.bottom);
      if(rcItem.right < rcGap.right) {
        canvas.FillRect(rcItem.right, nPassPos, rcGap.right - rcItem.right, rcGap.bottom - nPassPos, 0xffffffff);
      }
      else {
        // 这个为什么要减掉一个高度，这样会导致最后一个条目
        // Template Window之外绘制的高亮填充被“底边空白填充”覆盖掉
        //rcGap.bottom -= TemplProtoHeight;
      }
    }//*/

    //////////////////////////////////////////////////////////////////////////
    if(rcGap.left) {
      canvas.FillRect(0, rcGap.top, rcGap.left, rcGap.bottom - rcGap.top, 0xffffffff);
    }

    if(rcGap.top) {
      canvas.FillRect(0, 0, rect.right, rcGap.top, 0xffffffff);
    }

    // 如果宽度不足，会用上面逐条判断是否在选择高亮状态下的方法填充
    //if(rcGap.right < rect.right) {
    //  canvas.FillRect(rcGap.right, rcGap.top, rect.right - rcGap.right, rcGap.bottom - rcGap.top, 0xffffffff);
    //}

    // 底边空白填充
    if(rcGap.bottom < rect.bottom) {
      canvas.FillRect(0, rcGap.bottom, rect.right, rect.bottom - rcGap.bottom, 0xffffffff);
    }
    //////////////////////////////////////////////////////////////////////////
    if(m_bShowScrollBar) {
      DrawScrollBar(&rect, dwStyle, canvas);
    }
    Recycle(i, 1);
    return 0;
  }

  int CustomizeList::OnSize(int cx, int cy)
  {
    const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);
    GXINT nPrevColumn = m_nColumnCount;
    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN))
    {
      GXRECT rcItemTemplate;
      if(m_hPrototype != NULL)
      {
        gxGetWindowRect(m_hPrototype, &rcItemTemplate);

        if(IS_LEFTTORIGHT(dwStyle)) {
          const int nItemSize = rcItemTemplate.bottom - rcItemTemplate.top;
          m_nColumnCount = nItemSize <= 0 ? 1 : cy / nItemSize;
        }
        else {
          const int nItemSize = rcItemTemplate.right - rcItemTemplate.left;
          m_nColumnCount = nItemSize <= 0 ? 1 : cx / nItemSize;
        }

        m_nColumnCount = clMax(m_nColumnCount, 1);
      }

      if(nPrevColumn != m_nColumnCount)
      {
        Recycle(0, 1);
        if(m_pAdapter) {
          UpdateTopIndex(dwStyle);
          EndScroll();
        }
      }
    }
    else {
      m_nColumnCount = 1;
    }
    return 0;
  }

  //GXHRESULT CustomizeList::OnKnock(KNOCKACTION* pAction)
  GXVOID CustomizeList::OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    CLBREAK;
    ListTemplate<CUSTOMIZEITEMSTAT>::OnImpulse(pImpulse);
    //if(hval == 0)
    {
      if(pImpulse->reason == Marimo::DATACT_Deleting)
      {
        MOVariable varArray = m_pAdapter->GetVariable();
        const int index = pImpulse->index; //((GXINT_PTR)pAction->ptr - (GXINT_PTR)varArray.GetPtr()) / (varArray.GetSize()/varArray.GetLength());
        ASSERT(index >= -1);
        Recycle(clMax(index, 0), 1);
      }
    }
    //return hval;
  }

  GXBOOL CustomizeList::ReduceItemStat(GXINT nCount)
  {
    Recycle(nCount, 1);
    m_aItemStat.erase(m_aItemStat.begin() + nCount, m_aItemStat.end());
    return TRUE;
  }

  GXINT CustomizeList::VirGetItemHeight(GXINT nIdx) const
  {
    return GetItemHeight(nIdx);
  }

  GXBOOL CustomizeList::GetItemRect(int nItem, GXDWORD dwStyle, GXLPRECT lprc) const
  {
    GXRECT rcItem;
    gxGetWindowRect(m_hPrototype, &rcItem);
    rcItem.right -= rcItem.left;
    rcItem.bottom -= rcItem.top;

    if(IS_LEFTTORIGHT(dwStyle)) {
      lprc->left = ITEM_LINE_POS(nItem, rcItem.right);
      lprc->top  = ITEM_COLUMN_POS(nItem, rcItem.bottom);
    }
    else {
      lprc->left = ITEM_COLUMN_POS(nItem, rcItem.right);
      lprc->top  = ITEM_LINE_POS(nItem,rcItem.bottom);
    }
    lprc->right = lprc->left + rcItem.right;
    lprc->bottom = lprc->top + rcItem.bottom;
    return TRUE;
  }

  CustomizeList::CustomizeList(GXLPCWSTR szIdName)
    : ListTemplate  (szIdName)
    , m_hPrototype  (NULL)
  {
  }

  void CustomizeList::DeleteItemStat( GXINT nIndex )
  {
    if(m_aItemStat[nIndex].hItem != NULL) {
      Recycle(nIndex);
    }
    ListTemplate<CUSTOMIZEITEMSTAT>::DeleteItemStat(nIndex);
  }

  GXLRESULT CustomizeList::SetItemTemplate( GXLPCWSTR szTemplate )
  {
    GXLRESULT result = 0;
    m_strTemplate = szTemplate;

    m_strTemplate.Find(L':');
    clvector<clStringW> aString;
    ResolveString(m_strTemplate, L':', aString);
    if(aString.size() == 2) {
      GXRECT rect;
      m_hPrototype = gxIntCreateDialogFromFileW(NULL, aString[0], aString[1], m_hWnd, (GXDLGPROC)CustomWndProc, NULL);
      gxGetClientRect(m_hPrototype, &rect);
      m_nColumnWidth = rect.right - rect.left;

      GXNMLISTADAPTER sCreateAdapter;
      sCreateAdapter.hdr.hwndFrom = m_hWnd;
      sCreateAdapter.hdr.idFrom   = 0;
      sCreateAdapter.hdr.code     = GXLBN_CREATEADAPTER;
      sCreateAdapter.hTemplateWnd = m_hPrototype;
      sCreateAdapter.pAdapter     = NULL;

      result = gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);
      if(result < 0) {
        return result;
      }

      if(sCreateAdapter.pAdapter)
      {
        SetAdapter(sCreateAdapter.pAdapter);
        SAFE_RELEASE(sCreateAdapter.pAdapter);
        m_aElementName.clear();
        gxEnumChildWindows(m_hPrototype, EnumChildProc, (GXLPARAM)this);
      }
      else
      {
        m_aElementName.clear();
        ItemElementArray aItemElements;
        if( ! gxEnumChildWindows(m_hPrototype, EnumAdapterChildProc, (GXLPARAM)&aItemElements)) {
          return -1;
        }

        CDefCustListAdapter* pAdapter = new CDefCustListAdapter(m_hWnd);
        if( ! InlCheckNewAndIncReference(pAdapter)) {
          return -1;
        }

        if( ! pAdapter->Initialize(aItemElements)) {
          SAFE_RELEASE(pAdapter);
          return -1;
        }

        for(ItemElementArray::iterator it = aItemElements.begin();
          it != aItemElements.end(); ++it) {
            if(it->strName.IsNotEmpty()) {
              m_aElementName.push_back(it->strName);
            }
        }

        SetAdapter(pAdapter);
        SAFE_RELEASE(pAdapter);
      }
      // 更新每行/列中Item的数量
      GXRECT rcClient;
      gxGetClientRect(m_hWnd, &rcClient);
      OnSize(rcClient.right, rcClient.bottom);
    }
    return result;
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE