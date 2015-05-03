#include "GameEngine.h"
#include "GXFCAgent.H"
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "GXPropertySheet.h"

CMOPropertySheet::CMOPropertySheet()
  : CMODlgItem(NULL)
  , m_pReceiver(NULL)
{
}

CMOPropertySheet::CMOPropertySheet(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pReceiver(NULL)
{
}

CMOPropertySheet::~CMOPropertySheet()
{

}

GXSIZE_T CMOPropertySheet::GetThisSizeOf() const
{
  return sizeof(CMOPropertySheet);
}

clStringW CMOPropertySheet::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_PROPSHEET);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOPropertySheet::GetClassNameCode() const
{
  return GXMAKEFOURCC('P','R','P','H');
}

//
//GXBOOL CGXPropertySheet::SetSpriteFile(GXLPCWSTR lpSpriteFile)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEFILE, NULL, (GXLPARAM)lpSpriteFile);
//}
//
//GXBOOL CGXPropertySheet::SetSpriteObj(GSprite* pSprite)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEOBJ, NULL, (GXLPARAM)pSprite);
//}
//
//GXINT CGXPropertySheet::AddButtons(const GXTBBUTTON* pButtons, int nButtons)
//{
//  return gxSendMessage(m_hWnd, TB_ADDBUTTONS, nButtons, (GXLPARAM)pButtons);
//}
//
//GXVOID CGXPropertySheet::AutoSize()
//{
//  gxSendMessage(m_hWnd, TB_AUTOSIZE, 0, 0);
//}
//
//GXBOOL CGXPropertySheet::SetBitmapSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBITMAPSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXPropertySheet::SetButtonSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBUTTONSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXPropertySheet::SetCommandProc(LPCOMMANDPROC pCmd)
//{
//  //m_pCmd = pCmd;
//  return TRUE;
//}

#ifdef REFACTOR_GXFC
#else
void CMOPropertySheet::SetReceiver(CMOPropertySheetReceiver* pReceiver)
{
  CMOPropertySheet* pButton = (CMOPropertySheet*)GetCObject();
  ASSERT(pButton != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pButton->m_pReceiver = pReceiver;
}

CGXReceiver* CMOPropertySheet::GetReceiver()
{
  CMOPropertySheet* pButton = (CMOPropertySheet*)GetCObject();
  return pButton->m_pReceiver;
}

void CMOPropertySheet::SetReceiver(CGXReceiver* pReceiver)
{
  SetReceiver(TryCastReceiver<CMOPropertySheetReceiver>((CMOPropertySheetReceiver*)pReceiver, NULL));
}

GXLRESULT CMOPropertySheet::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return Invoke(m_pReceiver, nNotifyCode, wID, hwndCtrl);
}

GXLRESULT CMOPropertySheet::InvokeReceiver( GXNMHDR* pnmhdr )
{
  return Invoke(m_pReceiver, pnmhdr);
}

GXLRESULT CMOPropertySheet::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CMOPropertySheetReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(nNotifyCode == GXBN_CLICKED && pBtnReceiver != NULL)
  //{
  //  return pBtnReceiver->OnBtnClicked(this, GetIdentifierName());
  //}
  return 0;
}

GXLRESULT CMOPropertySheet::Invoke(CGXReceiver* pReceiver, GXNMHDR* pnmhdr)
{
  CMOPropertySheetReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(pPropReceiver == 0) {
    return -1;
  }

  NM_PROPSHEETW* pItemDesc = (NM_PROPSHEETW*)pnmhdr;

  if(pnmhdr->code == GXNM_CLICK || pnmhdr->code == GXNM_HOVER) {
    pPropReceiver->OnValueChanging(this, pItemDesc->strName, pItemDesc);
  }
  else if(pnmhdr->code == GXNM_RETURN) {
    pPropReceiver->OnValueChanging(this, pItemDesc->strName, pItemDesc);
  }
  else {
    CLBREAK; // 增加对应处理
  }
  return 0;

}
#endif // #ifdef REFACTOR_GXFC

GXBOOL CMOPropertySheet::SetDataLayout( const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink )
{
  PROPSHEET_DATALAYOUT sLayout = {pBasePtrList, pDataLink, 0, 0};
  return gxSendMessage(m_hWnd, PSM_SETDATALAYOUT, NULL, (GXLPARAM)&sLayout);
}

GXBOOL CMOPropertySheet::UploadData( const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast )
{
  PROPSHEET_DATALAYOUT sLayout = {pBasePtrList, pDataLink, nIDFirst, nIDLast};
  return gxSendMessage(m_hWnd, PSM_UPLOADDATA, NULL, (GXLPARAM)&sLayout);
}

GXBOOL CMOPropertySheet::DownloadData( const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast )
{
  PROPSHEET_DATALAYOUT sLayout = {pBasePtrList, pDataLink, nIDFirst, nIDLast};
  return gxSendMessage(m_hWnd, PSM_DOWNLOADDATA, NULL, (GXLPARAM)&sLayout);
}

//////////////////////////////////////////////////////////////////////////

GXLRESULT CMOPropertySheetReceiver::OnValueChanging(CMOPropertySheet* pSender, GXLPCWSTR szName, const NM_PROPSHEETW* pItem)
{
  return 0;
}

GXLRESULT CMOPropertySheetReceiver::OnValueChanged(CMOPropertySheet* pSender, GXLPCWSTR szName, const NM_PROPSHEETW* pItem)
{
  return 0;
}
#ifdef REFACTOR_GXFC
GXLRESULT CMOPropertySheetReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  CLBREAK;
  return 0;
}

GXLRESULT CMOPropertySheetReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  //CGXPropertySheetReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(pPropReceiver == 0) {
  //  return -1;
  //}

  if( ! gxIsWindow(pnmhdr->hwndFrom)) {
    return -1;
  }

  CMOPropertySheet PropWnd(pnmhdr->hwndFrom);
  NM_PROPSHEETW* pItemDesc = (NM_PROPSHEETW*)pnmhdr;

  if(pnmhdr->code == GXNM_CLICK || pnmhdr->code == GXNM_HOVER) {
    OnValueChanging(&PropWnd, pItemDesc->strName, pItemDesc);
  }
  else if(pnmhdr->code == GXNM_RETURN) {
    OnValueChanging(&PropWnd, pItemDesc->strName, pItemDesc);
  }
  else {
    CLBREAK; // 增加对应处理
  }
  return 0;
}
#endif // #ifdef REFACTOR_GXFC