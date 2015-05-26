#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/WETreeView.H"

CWETreeView::CWETreeView(GXHWND hWnd) 
  : CMODlgItem  (hWnd)
  , m_pReceiver  (NULL)
{
}

CWETreeView::~CWETreeView()
{
}

//GXINT CWETreeView::GetCount()
//{
//  return (GXINT)gxSendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0);
//}

//CWETreeView* CWETreeView::GetDlgItem(GXHWND hParent, GXINT idCtrl)
//{
//  GXHWND hDlgItem = gxGetDlgItem(hParent, idCtrl);
//  CWETreeView* pItem = (CWETreeView*)gxGetWindowLong(hDlgItem, GWL_USERDATA);
//  ASSERT(pItem == NULL || (pItem != NULL && pItem->m_hWnd == hDlgItem));
//
//  if(pItem == NULL)
//  {
//    pItem = new CWETreeView(hDlgItem);
//    gxSetWindowLong(hDlgItem, GWL_USERDATA, (GXLONG_PTR)pItem);
//  }
//  return pItem;
//}
GXHTREEITEM CWETreeView::InsertItem(GXHTREEITEM hParent, GXHTREEITEM hInsertAfter, GXLPCWSTR lpText, GXLPARAM lParam)
{
  GXTV_INSERTSTRUCT tvi;
  tvi.hParent = hParent;
  tvi.hInsertAfter = hInsertAfter;
  tvi.u.item.mask = GXTVIF_TEXT | GXTVIF_PARAM;
  tvi.u.item.pszText = (LPWSTR)lpText;
  tvi.u.item.lParam = lParam;
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_INSERTITEM, 0, (GXLPARAM)&tvi);
}
GXHTREEITEM CWETreeView::InsertItem(GXHTREEITEM hParent, GXHTREEITEM hInsertAfter, int iImage, int iSelectImage, GXLPCWSTR lpText, GXLPARAM lParam)
{
  GXTV_INSERTSTRUCT tvi;
  tvi.hParent = hParent;
  tvi.hInsertAfter = hInsertAfter;
  tvi.u.item.mask = GXTVIF_IMAGE | GXTVIF_SELECTEDIMAGE | GXTVIF_TEXT | GXTVIF_PARAM;
  tvi.u.item.pszText = (LPWSTR)lpText;
  tvi.u.item.iImage = iImage;
  tvi.u.item.iSelectedImage = iSelectImage;
  tvi.u.item.lParam = lParam;
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_INSERTITEM, 0, (GXLPARAM)&tvi);
}
GXHTREEITEM CWETreeView::GetParent(GXHTREEITEM hItem)
{
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_GETNEXTITEM, GXTVGN_PARENT, (GXLPARAM)hItem);
}
GXHTREEITEM CWETreeView::GetChild(GXHTREEITEM hItem)
{
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_GETNEXTITEM, GXTVGN_CHILD, (GXLPARAM)hItem);
}
GXHTREEITEM CWETreeView::GetSibling(GXHTREEITEM hItem)
{
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_GETNEXTITEM, GXTVGN_NEXT, (GXLPARAM)hItem);
}
GXHTREEITEM CWETreeView::GetNextItem(GXHTREEITEM hItem, CWETreeView::GetNext eNext)
{
  return (GXHTREEITEM)gxSendMessage(m_hWnd, GXTVM_GETNEXTITEM, eNext, (GXLPARAM)hItem);
}

BOOL CWETreeView::GetItemText(GXHTREEITEM hItem, WCHAR* buffer, int nLength)
{
  GXTVITEM tvi;
  tvi.hItem = hItem;
  tvi.mask = GXTVIF_TEXT;
  tvi.pszText = buffer;
  tvi.cchTextMax = nLength;
  return (BOOL)gxSendMessage(m_hWnd, GXTVM_GETITEM, 0, (GXLPARAM)&tvi);
}
BOOL CWETreeView::GetItemParam(GXHTREEITEM hItem, GXLPARAM* plParam)
{
  GXTVITEM tvi;
  tvi.hItem = hItem;
  tvi.mask = GXTVIF_PARAM;
  GXBOOL bRet = (GXBOOL)gxSendMessage(m_hWnd, GXTVM_GETITEM, 0, (GXLPARAM)&tvi);
  *plParam = tvi.lParam;
  return bRet;
}
BOOL CWETreeView::SetItemParam(GXHTREEITEM hItem, GXLPARAM lParam)
{
  GXTVITEM tvi;
  tvi.hItem = hItem;
  tvi.mask = GXTVIF_PARAM;
  tvi.lParam = lParam;
  return (BOOL)(gxSendMessage(m_hWnd, GXTVM_SETITEM, 0, (GXLPARAM)&tvi) == 0);
}
//////////////////////////////////////////////////////////////////////////

//GXINT CWETreeView::InsertColumn(GXLPWSTR lpText, int fmt, int cx, int iSub)
//{
//  LV_COLUMN lvc;
//  lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
//  lvc.fmt = fmt;
//  lvc.cx = cx;
//  lvc.pszText = lpText;
//  lvc.cchTextMax = 0;
//  lvc.iSubItem = 0;  // 插入到哪个iSub是下面参数指定的，不是这个
//  lvc.iImage = 0;
//  lvc.iOrder = 0;
//  return (GXINT)gxSendMessage(m_hWnd, LVM_INSERTCOLUMN, iSub, (GXLPARAM)&lvc);
//}
//GXINT CWETreeView::InsertItem(int iItem, GXLPWSTR lpText, GXLPARAM lParam)
//{
//  LV_ITEM lvi;
//  lvi.mask = LVIF_TEXT | LVIF_PARAM;
//  lvi.iItem = iItem;
//  lvi.iSubItem = 0;
//  lvi.pszText = lpText;
//  lvi.iImage = NULL;
//  lvi.lParam = lParam;
//  return (GXINT)gxSendMessage(m_hWnd, LVM_INSERTITEM, 0, (GXLPARAM)&lvi);
//}
//
//GXINT CWETreeView::InsertItem(int iItem, GXLPWSTR lpText)
//{
//  LV_ITEM lvi;
//  lvi.mask = LVIF_TEXT | LVIF_PARAM;
//  lvi.iItem = iItem;
//  lvi.iSubItem = 0;
//  lvi.pszText = lpText;
//  lvi.iImage = NULL;
//  lvi.lParam = NULL;
//  return (GXINT)gxSendMessage(m_hWnd, LVM_INSERTITEM, 0, (GXLPARAM)&lvi);
//}
//
//GXBOOL CWETreeView::SetItem(GXINT iItem, int iSub, GXLPWSTR lpText)
//{
//  LV_ITEM lvi;
//  lvi.mask = LVIF_TEXT;
//  lvi.iSubItem = iSub;
//  lvi.iItem = (int)iItem;
//  lvi.pszText = lpText;
//  return (GXBOOL)gxSendMessage(m_hWnd, LVM_SETITEM, 0, (GXLPARAM)&lvi);
//}

#ifdef REFACTOR_GXFC
#else
CGXReceiver* CWETreeView::GetReceiver()
{
  CWETreeView* pTreeView = (CWETreeView*)GetCObject();
  return pTreeView->m_pReceiver;
}

void CWETreeView::SetReceiver(CGXReceiver* pReceiver)
{
  CWETreeView* pTreeView = (CWETreeView*)GetCObject();
  ASSERT(pTreeView != NULL);
  m_pReceiver = NULL;// 这个要放在前面,避免this和CObject是相同的
  pTreeView->m_pReceiver = TryCastReceiver(pReceiver, pTreeView->m_pReceiver);
}

GXLRESULT CWETreeView::InvokeReceiver(NMHDR* pnmhdr)
{
  return Invoke(m_pReceiver, pnmhdr);
}

GXLRESULT CWETreeView::Invoke(CGXReceiver* pReceiver, NMHDR* pnmhdr)
{
  CWETreeViewReceiver* pTreeViewReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(pTreeViewReceiver == NULL)
    return 0L;
  
  GXNMTREEVIEW* pnmtv = (GXNMTREEVIEW*)pnmhdr;
  switch(pnmhdr->code)
  {
  case GXTVN_ITEMEXPANDING:
    return pTreeViewReceiver->ItemExpanding(this,
      (CWETreeView::Expand)pnmtv->action, &pnmtv->itemNew);
  case GXTVN_ITEMEXPANDED:
    pTreeViewReceiver->ItemExpanded(this,
      (CWETreeView::Expand)pnmtv->action, &pnmtv->itemNew);
    break;
  case GXTVN_SELCHANGED:
    pTreeViewReceiver->SelChanged(this, (CWETreeView::Change)pnmtv->action,
      &pnmtv->itemOld, &pnmtv->itemNew);
    break;
  case GXTVN_SELCHANGING:
    return pTreeViewReceiver->SelChanging(this, (CWETreeView::Change)pnmtv->action,
      &pnmtv->itemOld, &pnmtv->itemNew);
  }
  return 0L;
}
#endif // #ifdef REFACTOR_GXFC

void CWETreeViewReceiver::DeleteItem(CWETreeView* pTreeView)
{

}
void CWETreeViewReceiver::GetDispInfo(CWETreeView* pTreeView)
{

}
void CWETreeViewReceiver::ItemExpanded(CWETreeView* pTreeView, CWETreeView::Expand eAction, GXTV_ITEM* tvi)
{

}
BOOL CWETreeViewReceiver::ItemExpanding(CWETreeView* pTreeView, CWETreeView::Expand eAction, GXTV_ITEM* tvi)
{
  return FALSE;
}
void CWETreeViewReceiver::KeyDown(CWETreeView* pTreeView)
{

}
void CWETreeViewReceiver::SelChanged(CWETreeView* pTreeView, CWETreeView::Change eAction, GXTV_ITEM* tviPrev, GXTV_ITEM* tviNew)
{

}
BOOL CWETreeViewReceiver::SelChanging(CWETreeView* pTreeView, CWETreeView::Change eAction, GXTV_ITEM* tviPrev, GXTV_ITEM* tviNew)
{
  return FALSE;
}
void CWETreeViewReceiver::SetDispInfo(CWETreeView* pTreeView)
{

}

#ifdef REFACTOR_GXFC
GXLRESULT CWETreeViewReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  CLBREAK;
  return 0;
}

GXLRESULT CWETreeViewReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  //CWETreeViewReceiver* pTreeViewReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(pTreeViewReceiver == NULL)
  //  return 0L;

  if( ! gxIsWindow(pnmhdr->hwndFrom)) {
    return -1;
  }

  GXNMTREEVIEW* pnmtv = (GXNMTREEVIEW*)pnmhdr;
  CWETreeView TreeViewWnd(pnmhdr->hwndFrom);

  switch(pnmhdr->code)
  {
  case GXTVN_ITEMEXPANDING:
    return ItemExpanding(&TreeViewWnd, (CWETreeView::Expand)pnmtv->action, &pnmtv->itemNew);
  case GXTVN_ITEMEXPANDED:
    ItemExpanded(&TreeViewWnd, (CWETreeView::Expand)pnmtv->action, &pnmtv->itemNew);
    break;
  case GXTVN_SELCHANGED:
    SelChanged(&TreeViewWnd, (CWETreeView::Change)pnmtv->action, &pnmtv->itemOld, &pnmtv->itemNew);
    break;
  case GXTVN_SELCHANGING:
    return SelChanging(&TreeViewWnd, (CWETreeView::Change)pnmtv->action, &pnmtv->itemOld, &pnmtv->itemNew);
  }
  return 0L;
}
#endif // #ifdef REFACTOR_GXFC