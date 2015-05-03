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
//#include "Include/guxtheme.h"

#include "Include/GXUser.H"
#include "Include/GXGDI.H"
#include "GXUICtrlBase.h"
#include "GXUIList.h"
#include "GXUIList_Simple.h"
#include "Include/GXCanvas.H"
#include "gxDevice.H"


//HGXWND gxIntCreateDialogFromFileW(GXHINSTANCE  hInstance, GXLPCWSTR lpFilename, GXLPCWSTR lpDlgName, GXHWND hParent, GXDLGPROC lpDialogFunc, GXLPARAM lParam);
//#ifdef _DEBUG
//GXDWORD g_nDbg = 0;
//#endif // _DEBUG
namespace GXUI
{
  //////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////
  SimpleList::SimpleList(GXLPCWSTR szIdName)
    : ListTemplate    (szIdName)
  {
  }

  GXLRESULT SimpleList::Measure(GXRegn* pRegn)
  {
    pRegn->Set(0, 0, 128, 128);
    return 0;
  }

  GXINT SimpleList::VirGetItemHeight(GXINT nIdx) const
  {
    return GetItemHeight(nIdx);
  }

  GXINT SimpleList::GetItemHeight(GXINT nIdx) const
  {
    //LISTBOXITEMSTAT sStat;
    const GXINT nItemHeight = m_pAdapter->GetItemHeight(nIdx);
    if(nItemHeight < 0) {
      return m_nItemHeight;
    }
    return nItemHeight;
  }

  GXINT SimpleList::HitTest(int fwKeys, int x, int y) const
  {
    if(m_aItemStat.empty()) {
      return -1;
    }

    ItemStatArray::const_iterator it = m_aItemStat.begin() + m_nTopIndex;
    ASSERT(it->nBottom > -m_nScrolled);
    GXINT nItem = m_nTopIndex;
    y -= m_nScrolled;
    for(; it != m_aItemStat.end(); ++it, ++nItem)
    {
      if(y < it->nBottom) {
        return nItem;
      }
    }
    return -1;
  }

  List::ListType SimpleList::GetListType()
  {
    return LT_Simple;
  }

  GXLRESULT SimpleList::OnPaint(GXWndCanvas& canvas)
  {
    GXRECT rect;
    gxGetClientRect(m_hWnd, &rect);

    const GXDWORD dwStyle = gxGetWindowLong(m_hWnd, GXGWL_STYLE);


    if(m_crBackground & 0xff000000) {
      canvas.FillRect(rect.left, rect.top, rect.right, rect.bottom, m_crBackground);
    }

    // �������͸����С��0.5�������ø߶Աȷ���
    GXBOOL bContrast = (m_crBackground >> 24) < 0x80;

    if(m_aItemStat.size() == 0) {
      return 0;
    }

    const GXBOOL bFixedHeight = m_pAdapter->IsFixedHeight();
    GXINT nCount = m_pAdapter->GetItemCount();
    GXINT nHeight = 0;
    if(bFixedHeight) {
      nHeight = GetItemHeight(0);
    }
    if(nCount > (GXINT)m_aItemStat.size())
    {
      return 0;
    }
    GXINT nAccHeight = m_nScrolled + m_aItemStat[m_nTopIndex].nBottom - nHeight;

    //clStringW strItem;
    //GXRECT  rcItem;

    ListDataAdapter::GETSTRW ItemStrDesc;
    GXRECT& rcItem = ItemStrDesc.rect;

    for(int i = m_nTopIndex; i < nCount; i++)
    {
      ItemStrDesc.nIdx = i;
      ItemStrDesc.szName = NULL;
      ItemStrDesc.nElement = 0;

      ITEMSTAT& ItemStat = m_aItemStat[i];
      GXColor32 crText = m_crText;

      if( ! bFixedHeight) {
        nHeight = GetItemHeight(i);
      }

      gxSetRect(&ItemStrDesc.rect, m_bShowButtonBox ? CHECKBOX_SIZE + 2 : 0, nAccHeight, rect.right, nAccHeight + nHeight);
      if(m_pAdapter->GetStringW(&ItemStrDesc))
      {
        GXINT nStrLen = ItemStrDesc.sString.GetLength();

        if(m_bShowButtonBox) {
          GXRECT rcBtnBox  = {1, rcItem.top, CHECKBOX_SIZE + 2, rcItem.bottom};
          GXDWORD dwState = DFCS_BUTTONRADIO;
          if(gxGetWindowLong(m_hWnd, GXGWL_STYLE) & GXLBS_MULTIPLESEL) {
            dwState = GXDFCS_BUTTONCHECK;
          }
          if(ItemStat.bSelected) {
            dwState |= GXDFCS_CHECKED;
          }
          canvas.DrawFrameControl(&rcBtnBox, GXDFC_BUTTON, dwState);
        }
        else if(ItemStat.bSelected) {  // ����ѡ����Ŀ�ĵ�ɫ
          crText = m_crHightlightText;
          canvas.FillRect(rcItem.left, rcItem.top, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, m_crHightlight);
        }

        if(m_aColumns.empty()) {
          if(bContrast) {
            GXRECT rcContrast = {rcItem.left + 1, rcItem.top + 1, rcItem.right + 1, rcItem.bottom + 1};
            canvas.DrawTextW(m_pFont, ItemStrDesc.sString, nStrLen, &rcContrast, DT_SINGLELINE|DT_VCENTER, crText.color ^ 0xFFFFFF);
          }
          canvas.DrawTextW(m_pFont, ItemStrDesc.sString, nStrLen, &rcItem, DT_SINGLELINE|DT_VCENTER, crText.color);
        }
        else {
          DrawTextWithColumnsW(canvas, m_pFont, ItemStrDesc.sString, nStrLen, &rcItem, 
            DT_SINGLELINE|DT_VCENTER, crText.color, bContrast ? crText.color ^ 0xFFFFFF : 0);
        }
      }

      nAccHeight += nHeight;
      if(nAccHeight >= rect.bottom)
        break;
    }

    if(m_bShowScrollBar) {
      DrawScrollBar(&rect, dwStyle, canvas);
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
    m_nColumnWidth = cx;
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
    return SetColumnsWidth(&aWidth.front(), aWidth.size());
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

    return m_aColumns.size();
  }

  GXUINT SimpleList::GetColumnsWidth( GXUINT* pColumns, GXUINT nCount )
  {
    if( ! pColumns || ! nCount) {
      return m_aColumns.size();
    }

    GXUINT i = 0;
    for(IntArray::iterator it = m_aColumns.begin();
      it != m_aColumns.end() && i < nCount; ++it, ++i) {
        pColumns[i] = *it;
    }
    return i;
  }

  int SimpleList::OnLButtonDown( int fwKeys, int x, int y )
  {
    return ListTemplate<ITEMSTAT>::OnLButtonDown(fwKeys, x, y);
  }

  GXBOOL SimpleList::GetItemRect( int nItem, GXDWORD dwStyle, GXLPRECT lprc ) const
  {
    CLBREAK; // û���Թ�
    ASSERT(TEST_FLAG(dwStyle, GXLBS_MULTICOLUMN)); // �ݲ�֧�ֶ���
    if(nItem > 0) {
      lprc->top = m_aItemStat[nItem - 1].nBottom + m_nScrolled;
    }
    else {
      lprc->top = m_nScrolled;
    }
    lprc->bottom = m_aItemStat[nItem].nBottom + m_nScrolled;
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

    // FIXME: �����Ӧ�÷���Create����,�����ڶԻ����WM_INITDIALOG��Ϣǰ����GXWM_NOTIFY��Ϣ
    GXNMLISTADAPTER sCreateAdapter;
    sCreateAdapter.hdr.hwndFrom = m_hWnd;
    sCreateAdapter.hdr.idFrom   = 0;
    sCreateAdapter.hdr.code     = GXLBN_CREATEADAPTER;
    sCreateAdapter.hTemplateWnd = NULL;
    sCreateAdapter.pAdapter     = NULL;

    result = gxSendMessage(m_hWnd, GXWM_NOTIFY, sCreateAdapter.hdr.idFrom, (GXLPARAM)&sCreateAdapter);
    if(result < 0) {
      return result;
    }

    if(sCreateAdapter.pAdapter)
    {
      SetAdapter(sCreateAdapter.pAdapter);
      SAFE_RELEASE(sCreateAdapter.pAdapter);
      return result;
    }
    
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
    return result;
  }

  GXLRESULT SimpleList::SetItemTemplate( GXLPCWSTR szTemplate )
  {
    CLBREAK; // ��Ӧ���ߵ�����
    return 0;
  }

} // namespace GXUI
#endif // #ifndef _DEV_DISABLE_UI_CODE