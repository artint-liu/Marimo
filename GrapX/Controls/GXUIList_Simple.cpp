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

#include "GrapX/GXUser.H"
#include "GrapX/GXGDI.H"
#include "GXUICtrlBase.h"
#include "GXUIList.h"
#include "GXUIList_Simple.h"
#include "GrapX/GXCanvas.H"
#include "GrapX/gxDevice.H"
#include "ListDataAdapter.h"

namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////
  SimpleList::SimpleList(GXLPCWSTR szIdName)
    : List(szIdName, FALSE)
  {
  }

  GXLRESULT SimpleList::Measure(GXRegn* pRegn)
  {
    pRegn->Set(0, 0, 128, 128);
    return 0;
  }

  //GXINT SimpleList::VirGetItemHeight(GXINT nIdx) const
  //{
  //  return GetItemHeight(nIdx);
  //}

  //GXINT SimpleList::GetItemHeight(GXINT nIdx) const
  //{
  //  //LISTBOXITEMSTAT sStat;
  //  //const GXINT nItemHeight = m_pAdapter->GetItemHeight(nIdx);
  //  //if(nItemHeight < 0) {
  //    return m_nItemHeight;
  //  //}
  //  //return nItemHeight;
  //}

  GXINT SimpleList::HitTest(int fwKeys, int x, int y) const
  {
    if(m_aItems.empty()) {
      return -1;
    }

    ItemStatusArray::const_iterator it = m_aItems.begin() + m_nTopIndex;
    ASSERT(it->nBottom > -m_nScrolled);
    auto nItem = m_nTopIndex;
    y -= m_nScrolled;
    for(; it != m_aItems.end(); ++it, ++nItem)
    {
      if(y < it->nBottom) {
        return (GXINT)nItem;
      }
    }
    return -1;
  }

  //List::ListType SimpleList::GetListType()
  //{
  //  return LT_Simple;
  //}

  GXLRESULT SimpleList::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    const GXDWORD dwStyle = (GXDWORD)gxGetWindowLong(m_hWnd, GXGWL_STYLE);


    if(m_crBackground & 0xff000000) {
      canvas.FillRect(rect.left, rect.top, rect.right, rect.bottom, m_crBackground);
    }

    // 如果背景透明度小于0.5，则启用高对比方案
    GXBOOL bContrast = (m_crBackground >> 24) < 0x80;

    if(m_aItems.empty()) {
      return 0;
    }

    //const GXBOOL bFixedHeight = TRUE;//m_pAdapter->IsFixedHeight();
    auto nCount = m_pAdapter->GetCount();
    //GXINT nHeight = 0;
    //if(bFixedHeight) {
    //  nHeight = GetItemHeight(0);
    //}
    if(nCount > (GXINT)m_aItems.size())
    {
      return 0;
    }
    //GXINT nItemTop = m_nScrolled + m_aItems[m_nTopIndex].nBottom/* - nHeight*/;
    //GXINT nItemTop;//    = m_nScrolled + (m_nTopIndex == 0 ? 0 : m_aItems[m_nTopIndex - 1].nBottom);
    //GXINT nItemBottom;// = m_nScrolled + m_aItems[m_nTopIndex].nBottom;

    //clStringW strItem;
    //GXRECT  rcItem;

    IListDataAdapter::GETSTRW ItemStrDesc;
    GXRECT& rcItem = ItemStrDesc.rect;
    ItemStrDesc.item = (GXINT)m_nTopIndex;

    if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) {

      // m_nTopIndex 要指向某一列的第一行,但是这个断言在自定义列高度时不一定准确
      //ASSERT(m_nTopIndex == 0 || m_aItems[m_nTopIndex - 1].nBottom >= m_aItems[m_nTopIndex].nBottom);

      //nItemTop      = 0;
      rcItem.left   = m_nScrolled >= 0 ? m_nScrolled : (m_nScrolled % m_nColumnWidth);
      rcItem.top    = 0;
      rcItem.right  = rcItem.left + m_nColumnWidth;
      rcItem.bottom = 0;

      if(rcItem.left > rect.right) {
        // 如果超出显示区域, 索引设为无效
        ItemStrDesc.item = (GXINT)nCount;
      }

      rcItem.left += (m_bShowButtonBox ? CHECKBOX_SIZE + 2 : 0);
    }
    else {
      rcItem.top    = m_nScrolled + (m_nTopIndex == 0 ? 0 : m_aItems[m_nTopIndex - 1].nBottom);
      rcItem.left   = m_bShowButtonBox ? CHECKBOX_SIZE + 2 : 0;
      rcItem.right  = rect.right;
      //rcItem.bottom = m_nScrolled + m_aItems[m_nTopIndex].nBottom;

      if(rcItem.top > rect.bottom) {
        // 如果超出显示区域, 索引设为无效
        ItemStrDesc.item = (GXINT)nCount;
      }
    }

    //TRACE("===================\n");

    for(; ItemStrDesc.item < nCount; ++ItemStrDesc.item)
    {
      ItemStrDesc.name    = NULL;
      ItemStrDesc.element = 0;

      const ITEMSTATUS& ItemStatus = m_aItems[ItemStrDesc.item];
      GXColor32 crText = m_crText;

      if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) {
        if(rcItem.bottom >= ItemStatus.nBottom) {
          rcItem.top = 0;
          rcItem.left += m_nColumnWidth;
          rcItem.right += m_nColumnWidth;
        }
        rcItem.bottom = ItemStatus.nBottom;
      }
      else {
        rcItem.bottom = m_nScrolled + ItemStatus.nBottom;
      }

      //gxSetRect(&rcItem, , nItemTop, rect.right, nItemBottom);

      if(m_pAdapter->GetStringW(&ItemStrDesc))
      {
        auto nStrLen = ItemStrDesc.sString.GetLength();
        //TRACEW(L"%s(%d,%d,%d,%d)\n", ItemStrDesc.sString,
        //  rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);

        if(m_bShowButtonBox) {
          GXRECT rcBtnBox  = {1, rcItem.top, CHECKBOX_SIZE + 2, rcItem.bottom};
          GXDWORD dwState = DFCS_BUTTONRADIO;
          if(gxGetWindowLong(m_hWnd, GXGWL_STYLE) & GXLBS_MULTIPLESEL) {
            dwState = GXDFCS_BUTTONCHECK;
          }
          if(ItemStatus.bSelected) {
            dwState |= GXDFCS_CHECKED;
          }
          canvas.DrawFrameControl(&rcBtnBox, GXDFC_BUTTON, dwState);
        }
        else if(ItemStatus.bSelected) {  // 绘制选择项目的底色
          crText = m_crHightlightText;
          canvas.FillRect(rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, m_crHightlight);
        }

        if(m_aColumns.empty()) {
          if(bContrast) {
            GXRECT rcContrast = {rcItem.left + 1, rcItem.top + 1, rcItem.right + 1, rcItem.bottom + 1};
            canvas.DrawTextW(m_pFont, ItemStrDesc.sString, (GXINT)nStrLen, &rcContrast, DT_SINGLELINE|DT_VCENTER, crText.color ^ 0xFFFFFF);
          }
          canvas.DrawTextW(m_pFont, ItemStrDesc.sString, (GXINT)nStrLen, &rcItem, DT_SINGLELINE|DT_VCENTER, crText.color);
        }
        else {
          DrawTextWithColumnsW(canvas, m_pFont, ItemStrDesc.sString, (GXINT)nStrLen, &rcItem, 
            DT_SINGLELINE|DT_VCENTER, crText.color, bContrast ? crText.color ^ 0xFFFFFF : 0);
        }
      }

      if(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)) {
        if(rcItem.left > rect.right) {
          break;
        }
      }
      else {
        if(rcItem.top >= rect.bottom) {
          break;
        }
      }

      rcItem.top = rcItem.bottom;
    }

    if(m_bShowScrollBar) {
      DrawScrollBar(canvas, &rect, m_aItems.back().nBottom, m_aItems.size(), dwStyle);
    }

    return 0;
  }

  GXINT SimpleList::DrawTextWithColumnsW(GXWndCanvas& canvas, GXFont* pFTFont, 
    GXLPCWSTR lpString, GXINT nCount,
    GXLPRECT lpRect,GXUINT uFormat, 
    GXCOLORREF crText, GXCOLORREF crContrast)
  {
    ASSERT( ! m_aColumns.empty());
    GXBOOL bEscapeChar = FALSE;
    GXRECT rect = *lpRect;
    GXRECT rcContrast;
    IntArray::iterator it = m_aColumns.begin();
    rect.right = rect.left + *it;
    int n = 0;
    for(int i = 0; i < nCount; i++)
    {
      switch(lpString[i])
      {
      case '\\':
        bEscapeChar = TRUE;
        break;
      case '\t':
        if( ! bEscapeChar) {
          ++it;
          if(it == m_aColumns.end()) {
            goto LAST_STR;
          }
          if(crContrast) {
            rcContrast.left   = rect.left + 1;
            rcContrast.top    = rect.top + 1;
            rcContrast.right  = rect.right + 1;
            rcContrast.bottom = rect.bottom + 1;
            canvas.DrawTextW(pFTFont, &lpString[n], i - n, &rcContrast, uFormat, crContrast);
          }
          canvas.DrawTextW(pFTFont, &lpString[n], i - n, &rect, uFormat, crText);
          n = i + 1;
          rect.left = rect.right;
          rect.right += *it;
          if(rect.left >= lpRect->right) {
            return 0;
          }
        }
        else {
          GXRECT rcSize = rect;
          canvas.DrawTextW(pFTFont, &lpString[n], i - n - 1, &rcSize, uFormat | GXDT_CALCRECT, crText);

          if(crContrast)
          {
            rcContrast.left   = rect.left + 1;
            rcContrast.top    = rect.top + 1;
            rcContrast.right  = rect.right + 1;
            rcContrast.bottom = rect.bottom + 1;
            canvas.DrawTextW(pFTFont, &lpString[n], i - n - 1, &rcContrast, uFormat, crContrast);
          }
          canvas.DrawTextW(pFTFont, &lpString[n], i - n - 1, &rect, uFormat, crText);
          rect.left += (rcSize.right - rcSize.left);
          n = i;
          bEscapeChar = FALSE;
        }
        break;
      default:
        bEscapeChar = FALSE;
        break;
      }
    }

LAST_STR:
    rect.right = lpRect->right;

    if(crContrast) {
      rcContrast.left   = rect.left + 1;
      rcContrast.top    = rect.top + 1;
      rcContrast.right  = rect.right + 1;
      rcContrast.bottom = rect.bottom + 1;
      canvas.DrawTextW(pFTFont, &lpString[n], nCount - n, &rcContrast, uFormat, crContrast);
    }

    canvas.DrawTextW(pFTFont, &lpString[n], nCount - n, &rect, uFormat, crText);
    return 0;
  }

  int SimpleList::OnSize(int cx, int cy)
  {
    m_nColumnWidth = cx / 2;
    return 0;
  }

  GXUINT SimpleList::SetColumnsWidth(GXLPCWSTR szString)
  {
    clStringArrayW aInteger;
    clvector<GXUINT> aWidth;
    clstd::ResolveString<clStringW>(szString, ',', aInteger);
    if(aInteger.empty()) {
      return 0;
    }

    for(clStringArrayW::iterator it = aInteger.begin();
      it != aInteger.end(); ++it) {
      GXUINT nWidth = clstd::xtou(*it, 10);
      aWidth.push_back(nWidth);
    }
    return SetColumnsWidth(&aWidth.front(), (GXUINT)aWidth.size());
  }

  GXUINT SimpleList::SetColumnsWidth( const GXUINT* pColumns, GXUINT nCount )
  {
    m_aColumns.clear();
    if( ! pColumns || ! nCount) {
      return 0;
    }
    
    m_aColumns.reserve(nCount);
    for(GXUINT i = 0; i < nCount; i++)
    {
      m_aColumns.push_back(pColumns[i]);
    }

    return (GXUINT)m_aColumns.size();
  }

  GXUINT SimpleList::GetColumnsWidth( GXUINT* pColumns, GXUINT nCount )
  {
    if( ! pColumns || ! nCount) {
      return (GXUINT)m_aColumns.size();
    }

    GXUINT i = 0;
    for(IntArray::iterator it = m_aColumns.begin();
      it != m_aColumns.end() && i < nCount; ++it, ++i) {
        pColumns[i] = *it;
    }
    return i;
  }

  GXBOOL SimpleList::GetItemRect( int nItem, GXDWORD dwStyle, GXLPRECT lprc ) const
  {
    CLBREAK; // 没测试过
    ASSERT(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)); // 暂不支持多列
    if(nItem > 0) {
      lprc->top = m_aItems[nItem - 1].nBottom + m_nScrolled;
    }
    else {
      lprc->top = m_nScrolled;
    }
    lprc->bottom = m_aItems[nItem].nBottom + m_nScrolled;
    lprc->left= 0;
    lprc->right = m_nColumnWidth;
    return TRUE;
  }

  int SimpleList::OnCreate( GXCREATESTRUCT* pCreateParam )
  {
    int result = List::OnCreate(pCreateParam);
    if(result < 0) {
      return result;
    }

    // FIXME: 这个不应该放在Create创建,不能在对话框的WM_INITDIALOG消息前产生GXWM_NOTIFY消息
    GXNMLISTADAPTER sCreateAdapter;
    sCreateAdapter.hdr.hwndFrom = m_hWnd;
    sCreateAdapter.hdr.idFrom   = 0;
    sCreateAdapter.hdr.code     = GXLBN_CREATEADAPTER;
    sCreateAdapter.hTemplateWnd = NULL;
    sCreateAdapter.pAdapter     = NULL;

    result = (int)gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);
    if(result < 0) {
      return result;
    }

    if(sCreateAdapter.pAdapter)
    {
      SetAdapter(sCreateAdapter.pAdapter);
      SAFE_RELEASE(sCreateAdapter.pAdapter);
      return result;
    }
    
    //DefaultListDataAdapter* pListAdapter = new DefaultListDataAdapter(m_hWnd);
    //if( ! InlCheckNewAndIncReference(pListAdapter)) {
    //  return -1;
    //}

    //if( ! pListAdapter->Initialize()) {
    //  CLBREAK;
    //  SAFE_RELEASE(pListAdapter);
    //  return -1;
    //}
    //SetAdapter(pListAdapter);
    //SAFE_RELEASE(pListAdapter);
    return result;
  }

  GXLRESULT SimpleList::SetItemTemplate( GXLPCWSTR szTemplate )
  {
    CLBREAK; // 不应该走到这里
    return 0;
  }

  GXLRESULT SimpleList::SetVariable( MOVariable* pVariable )
  {
    CDefListDataAdapter* pListAdapter = new CDefListDataAdapter(m_hWnd);

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

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE