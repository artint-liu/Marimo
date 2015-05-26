#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
//#include <PreDefine.H>
//#include <gxWinDef.H>
//#include <GrapX.H>
//#include <User\Win32Emu\GXUser.H>
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/WEListView.H"

CWEListView::CWEListView(GXHWND hWnd) : CMODlgItem(hWnd)
{
}

CWEListView::~CWEListView()
{
}

GXINT CWEListView::GetCount()
{
  return (GXINT)gxSendMessage(m_hWnd, GXLVM_GETITEMCOUNT, 0, 0);
}

//CWEListView* CWEListView::GetDlgItem(GXHWND hParent, GXINT idCtrl)
//{
//  GXHWND hDlgItem = gxGetDlgItem(hParent, idCtrl);
//  CWEListView* pItem = (CWEListView*)gxGetWindowLong(hDlgItem, GWL_USERDATA);
//  ASSERT(pItem == NULL || (pItem != NULL && pItem->m_hWnd == hDlgItem));
//
//  if(pItem == NULL)
//  {
//    pItem = new CWEListView(hDlgItem);
//    gxSetWindowLong(hDlgItem, GWL_USERDATA, (GXLONG_PTR)pItem);
//  }
//  return pItem;
//}
//////////////////////////////////////////////////////////////////////////
BOOL CWEListView::GetItemText(int nItem, int nSubItem, LPWSTR lpText, int nLength)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_TEXT;
  lvi.iItem = nItem;
  lvi.iSubItem = nSubItem;
  lvi.pszText = lpText;
  lvi.cchTextMax = nLength;
  return (BOOL)gxSendMessage(m_hWnd, GXLVM_GETITEM, 0, (GXLPARAM)&lvi);
}
void CWEListView::DeleteAllItems()
{
  gxSendMessage(m_hWnd, GXLVM_DELETEALLITEMS, 0, 0);
}
GXINT CWEListView::InsertColumn(GXLPWSTR lpText, int fmt, int cx, int iSub)
{
  GXLV_COLUMN lvc;
  lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
  lvc.fmt = fmt;
  lvc.cx = cx;
  lvc.pszText = lpText;
  lvc.cchTextMax = 0;
  lvc.iSubItem = 0;  // 插入到哪个iSub是下面参数指定的，不是这个
  lvc.iImage = 0;
  lvc.iOrder = 0;
  return (GXINT)gxSendMessage(m_hWnd, GXLVM_INSERTCOLUMN, iSub, (GXLPARAM)&lvc);
}
GXINT CWEListView::InsertItem(int iItem, GXLPWSTR lpText, GXLPARAM lParam)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_TEXT | GXLVIF_PARAM;
  lvi.iItem = iItem;
  lvi.iSubItem = 0;
  lvi.pszText = lpText;
  lvi.iImage = NULL;
  lvi.lParam = lParam;
  return (GXINT)gxSendMessage(m_hWnd, GXLVM_INSERTITEM, 0, (GXLPARAM)&lvi);
}

GXINT CWEListView::InsertItem(int iItem, GXLPWSTR lpText)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_TEXT | GXLVIF_PARAM;
  lvi.iItem = iItem;
  lvi.iSubItem = 0;
  lvi.pszText = lpText;
  lvi.iImage = NULL;
  lvi.lParam = NULL;
  return (GXINT)gxSendMessage(m_hWnd, GXLVM_INSERTITEM, 0, (GXLPARAM)&lvi);
}
GXINT CWEListView::InsertItem(GXINT iItem, GXLPWSTR lpText, int iImage, GXLPARAM lParam)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_TEXT | GXLVIF_IMAGE | GXLVIF_PARAM;
  lvi.iItem = iItem;
  lvi.iSubItem = 0;
  lvi.pszText = lpText;
  lvi.iImage = iImage;
  lvi.lParam = NULL;
  return (GXINT)gxSendMessage(m_hWnd, GXLVM_INSERTITEM, 0, (GXLPARAM)&lvi);
}
GXHIMAGELIST CWEListView::SetImageList(GXHIMAGELIST hImageList, SetImageListType eType)
{
  return (GXHIMAGELIST)gxSendMessage(m_hWnd, GXLVM_SETIMAGELIST, (GXWPARAM)eType, (GXLPARAM)hImageList);
}
GXBOOL CWEListView::SetItemText(GXINT iItem, int iSub, GXLPWSTR lpText)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_TEXT;
  lvi.iItem = (int)iItem;
  lvi.iSubItem = iSub;
  lvi.pszText = lpText;
  return (GXBOOL)gxSendMessage(m_hWnd, GXLVM_SETITEM, 0, (GXLPARAM)&lvi);
}

GXBOOL CWEListView::SetItemImage(GXINT iItem, int iSub, int nImage)
{
  GXLV_ITEM lvi;
  lvi.mask = GXLVIF_IMAGE;
  lvi.iItem = (int)iItem;
  lvi.iSubItem = iSub;
  lvi.iImage = nImage;
  return (GXBOOL)gxSendMessage(m_hWnd, GXLVM_SETITEM, 0, (GXLPARAM)&lvi);
}