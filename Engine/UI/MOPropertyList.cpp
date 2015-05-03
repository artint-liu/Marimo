#include "GameEngine.h"
#include "GXFCAgent.H"
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "MOPropertyList.h"

CMOPropertyList::CMOPropertyList()
  : CMODlgItem(NULL)
  , m_pReceiver(NULL)
{
}

CMOPropertyList::CMOPropertyList(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pReceiver(NULL)
{
}

CMOPropertyList::~CMOPropertyList()
{

}

GXSIZE_T CMOPropertyList::GetThisSizeOf() const
{
  return sizeof(CMOPropertyList);
}

clStringW CMOPropertyList::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_PROPLIST);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOPropertyList::GetClassNameCode() const
{
  return GXMAKEFOURCC('P','R','P','H');
}

//
//GXBOOL CGXPropertyList::SetSpriteFile(GXLPCWSTR lpSpriteFile)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEFILE, NULL, (GXLPARAM)lpSpriteFile);
//}
//
//GXBOOL CGXPropertyList::SetSpriteObj(GSprite* pSprite)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEOBJ, NULL, (GXLPARAM)pSprite);
//}
//
//GXINT CGXPropertyList::AddButtons(const GXTBBUTTON* pButtons, int nButtons)
//{
//  return gxSendMessage(m_hWnd, TB_ADDBUTTONS, nButtons, (GXLPARAM)pButtons);
//}
//
//GXVOID CGXPropertyList::AutoSize()
//{
//  gxSendMessage(m_hWnd, TB_AUTOSIZE, 0, 0);
//}
//
//GXBOOL CGXPropertyList::SetBitmapSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBITMAPSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXPropertyList::SetButtonSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBUTTONSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXPropertyList::SetCommandProc(LPCOMMANDPROC pCmd)
//{
//  //m_pCmd = pCmd;
//  return TRUE;
//}

#ifdef REFACTOR_GXFC
#else
void CMOPropertyList::SetReceiver(CMOPropertyListReceiver* pReceiver)
{
  CMOPropertyList* pButton = (CMOPropertyList*)GetCObject();
  ASSERT(pButton != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pButton->m_pReceiver = pReceiver;
}

CGXReceiver* CMOPropertyList::GetReceiver()
{
  CMOPropertyList* pButton = (CMOPropertyList*)GetCObject();
  return pButton->m_pReceiver;
}

void CMOPropertyList::SetReceiver(CGXReceiver* pReceiver)
{
  SetReceiver(TryCastReceiver<CMOPropertyListReceiver>((CMOPropertyListReceiver*)pReceiver, NULL));
}

GXLRESULT CMOPropertyList::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return Invoke(m_pReceiver, nNotifyCode, wID, hwndCtrl);
}

GXLRESULT CMOPropertyList::InvokeReceiver( GXNMHDR* pnmhdr )
{
  return Invoke(m_pReceiver, pnmhdr);
}

GXLRESULT CMOPropertyList::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CMOPropertyListReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(nNotifyCode == GXBN_CLICKED && pBtnReceiver != NULL)
  //{
  //  return pBtnReceiver->OnBtnClicked(this, GetIdentifierName());
  //}
  return 0;
}

GXLRESULT CMOPropertyList::Invoke(CGXReceiver* pReceiver, GXNMHDR* pnmhdr)
{
  CMOPropertyListReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(pPropReceiver == 0) {
    return -1;
  }

  NM_PROPListW* pItemDesc = (NM_PROPListW*)pnmhdr;

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

GXBOOL CMOPropertyList::SetDataLayout( const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink )
{
  PROPLIST_DATALAYOUT sLayout = {pBasePtrList, pDataLink, 0, 0};
  return gxSendMessage(m_hWnd, PSM_SETDATALAYOUT, NULL, (GXLPARAM)&sLayout);
}

GXBOOL CMOPropertyList::UploadData( const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast )
{
  PROPLIST_DATALAYOUT sLayout = {pBasePtrList, pDataLink, nIDFirst, nIDLast};
  return gxSendMessage(m_hWnd, PSM_UPLOADDATA, NULL, (GXLPARAM)&sLayout);
}

GXBOOL CMOPropertyList::DownloadData( const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast )
{
  PROPLIST_DATALAYOUT sLayout = {pBasePtrList, pDataLink, nIDFirst, nIDLast};
  return gxSendMessage(m_hWnd, PSM_DOWNLOADDATA, NULL, (GXLPARAM)&sLayout);
}

//////////////////////////////////////////////////////////////////////////

GXLRESULT CMOPropertyListReceiver::OnValueChanging(CMOPropertyList* pSender, GXLPCWSTR szName, const NM_PROPLISTW* pItem)
{
  return 0;
}

GXLRESULT CMOPropertyListReceiver::OnValueChanged(CMOPropertyList* pSender, GXLPCWSTR szName, const NM_PROPLISTW* pItem)
{
  return 0;
}
#ifdef REFACTOR_GXFC
GXLRESULT CMOPropertyListReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  CLBREAK;
  return 0;
}

GXLRESULT CMOPropertyListReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  //CGXPropertyListReceiver* pPropReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(pPropReceiver == 0) {
  //  return -1;
  //}

  if( ! gxIsWindow(pnmhdr->hwndFrom)) {
    return -1;
  }

  CMOPropertyList PropWnd(pnmhdr->hwndFrom);
  NM_PROPLISTW* pItemDesc = (NM_PROPLISTW*)pnmhdr;

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