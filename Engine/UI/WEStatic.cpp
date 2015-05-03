#include "GameEngine.h"
#include "GXFCAgent.H"
//#include <PreDefine.H>
//#include <gxWinDef.H>
//#include <GrapX.H>
//#include <User\Win32Emu\GXUser.H>
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "WEStatic.H"

CWEStatic::CWEStatic(GXHWND hWnd) : CMODlgItem(hWnd)
{
}

CWEStatic::~CWEStatic()
{
}

//GXINT CWEStatic::AddString(GXLPWSTR lpString)
//{
//  return gxSendMessageW(m_hWnd, LB_ADDSTRING, NULL, (GXLPARAM)lpString);
//}
//
//GXINT CWEStatic::AddString(GXLPWSTR lpString, GXLPARAM lParam)
//{
//  return gxSendMessageW(m_hWnd, LB_SETITEMDATA,
//    gxSendMessageW(m_hWnd, LB_ADDSTRING, NULL, (GXLPARAM)lpString), lParam );
//}
//
//GXINT CWEStatic::DeleteString(GXINT nIndex)
//{
//  return gxSendMessageW(m_hWnd, LB_DELETESTRING, nIndex, NULL);
//}
//
//GXINT CWEStatic::GetCount()
//{
//  return gxSendMessageW(m_hWnd, LB_GETCOUNT, NULL, NULL);
//}
//
//CWEStatic* CWEStatic::GetDlgItem(GXHWND hParent, GXINT idCtrl)
//{
//  GXHWND hDlgItem = gxGetDlgItem(hParent, idCtrl);
//  CWEStatic* pItem = (CWEStatic*)gxGetWindowLong(hDlgItem, GWL_USERDATA);
//  ASSERT(pItem == NULL || (pItem != NULL && pItem->m_hWnd == hDlgItem));
//
//  if(pItem == NULL)
//  {
//    pItem = new CWEStatic(hDlgItem);
//    gxSetWindowLong(hDlgItem, GWL_USERDATA, (GXLONG_PTR)pItem);
//  }
//  return pItem;
//}
