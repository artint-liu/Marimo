#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/WEListBox.H"

CWEListBox::CWEListBox() : CMODlgItem(NULL)
{
}

CWEListBox::CWEListBox(GXHWND hWnd) : CMODlgItem(hWnd)
{
}

CWEListBox::~CWEListBox()
{
}

GXSIZE_T CWEListBox::GetThisSizeOf() const
{
  return sizeof(CWEListBox);
}

clStringW CWEListBox::GetClassName() const
{
  clStringW ClsName(GXWC_LISTBOXW);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CWEListBox::GetClassNameCode() const
{
  return GXMAKEFOURCC('W','L','S','T');
}


GXINT CWEListBox::AddString(GXLPWSTR lpString)
{
  return (GXINT)(GXINT_PTR)gxSendMessageW(m_hWnd, LB_ADDSTRING, NULL, (GXLPARAM)lpString);
}

GXINT CWEListBox::AddString(GXLPWSTR lpString, GXLPARAM lParam)
{
  return (GXINT)(GXINT_PTR)gxSendMessageW(m_hWnd, LB_SETITEMDATA,
    (GXWPARAM)gxSendMessageW(m_hWnd, LB_ADDSTRING, NULL, (GXLPARAM)lpString), lParam );
}

GXINT CWEListBox::DeleteString(GXINT nIndex)
{
  return (GXINT)gxSendMessageW(m_hWnd, LB_DELETESTRING, nIndex, NULL);
}

GXINT CWEListBox::GetCount()
{
  return (GXINT)gxSendMessageW(m_hWnd, LB_GETCOUNT, NULL, NULL);
}

//CWEListBox* CWEListBox::GetDlgItem(GXHWND hParent, GXINT idCtrl)
//{
//  GXHWND hDlgItem = gxGetDlgItem(hParent, idCtrl);
//  CWEListBox* pItem = (CWEListBox*)gxGetWindowLong(hDlgItem, GWL_USERDATA);
//  ASSERT(pItem == NULL || (pItem != NULL && pItem->m_hWnd == hDlgItem));
//
//  if(pItem == NULL)
//  {
//    pItem = new CWEListBox(hDlgItem);
//    gxSetWindowLong(hDlgItem, GWL_USERDATA, (GXLONG_PTR)pItem);
//  }
//  return pItem;
//}
