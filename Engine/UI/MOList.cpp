#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOList.h"
//#include "GrapX/GUnknown.H"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "GrapX/DataInfrastructure.H"

CMOListBox::CMOListBox()
  : CMODlgItem(NULL)
  , m_pReceiver(NULL)
{
}

CMOListBox::CMOListBox(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pReceiver(NULL)
{
}

CMOListBox::~CMOListBox()
{

}
GXSIZE_T CMOListBox::GetThisSizeOf() const
{
  return sizeof(CMOListBox);
}

clStringW CMOListBox::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_LIST);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOListBox::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','L','B','X');
}

GXHRESULT CMOListBox::GetDataAdapter( GXUI::IListDataAdapter** ppAdapter )
{
  return gxSendMessage(m_hWnd, GXWM_DATAPOOLOPERATION, 
    (GXWPARAM)DPO_GETADAPTER, (GXLPARAM)ppAdapter);
}

GXHRESULT CMOListBox::GetDataPool(GXUI::MODataPool** ppDataPool)
{
  GXUI::IListDataAdapter* pAdapter;
  gxSendMessage(m_hWnd, GXWM_DATAPOOLOPERATION, 
    (GXWPARAM)DPO_GETADAPTER, (GXLPARAM)&pAdapter);
  GXHRESULT hval = pAdapter->GetDataPool(ppDataPool);
  pAdapter->Release();
  return hval;
}

GXHRESULT CMOListBox::GetArrayElement(GXUI::MOVariable* pVariable)
{
  GXUI::IListDataAdapter* pAdapter = NULL;
  //GXUI::MODataPool* pDataPool;
  GXHRESULT hval = gxSendMessage(m_hWnd, GXWM_DATAPOOLOPERATION, 
    (GXWPARAM)DPO_GETADAPTER, (GXLPARAM)&pAdapter);
  //GXHRESULT hval = pAdapter->GetDataPool(&pDataPool);
  *pVariable = pAdapter->GetVariable();
  pAdapter->Release();
  return hval;
}

GXINT CMOListBox::AddStringW(GXLPCWSTR lpString)
{
  return gxSendMessage(m_hWnd, GXLB_ADDSTRINGW, 0, (GXLPARAM)lpString);
}

GXINT CMOListBox::AddStringA(GXLPCSTR lpString)
{
  clStringW str = lpString;
  return AddStringW(str);
}

GXINT CMOListBox::GetStringLength(GXINT nIndex)
{
  return gxSendMessage(m_hWnd, GXLB_GETTEXTLEN, nIndex, NULL);
}

GXLRESULT CMOListBox::GetStringW(GXINT nIndex, clStringW& str)
{
  GXINT nLen = GetStringLength(nIndex);
  if(nLen <= 0) {
    return 0;
  }
  WCHAR* pBuf = str.GetBuffer(nLen + 1);
  GXLRESULT lr = gxSendMessage(m_hWnd, GXLB_GETTEXT, nIndex, (GXLPARAM)pBuf);
  str.ReleaseBuffer();
  return lr;
}

GXLRESULT CMOListBox::GetStringA(GXINT nIndex, clStringA& str)
{
  clStringW wstr;
  GXLRESULT lr = GetStringW(nIndex, wstr);
  str = wstr;
  return lr;
}

GXINT CMOListBox::DeleteString(GXINT nIndex)
{
  return gxSendMessage(m_hWnd, GXLB_DELETESTRING, nIndex, NULL);
}

void CMOListBox::ResetContent()
{
  gxSendMessage(m_hWnd, GXLB_RESETCONTENT, NULL, NULL);
}

GXINT CMOListBox::GetCount() const
{
  return gxSendMessage(m_hWnd, GXLB_GETCOUNT, NULL, NULL);
}

GXINT CMOListBox::GetCurSel() const
{
  return gxSendMessage(m_hWnd, GXLB_GETCURSEL, NULL, NULL);
}

GXHRESULT CMOListBox::SetAdapter(GXUI::IListDataAdapter* pAdapter)
{
  return gxSendMessage(m_hWnd, GXWM_DATAPOOLOPERATION, (GXWPARAM)DPO_SETADAPTER, (GXLPARAM)pAdapter);
}

#ifdef REFACTOR_GXFC
#else
GXLRESULT CMOListBox::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return Invoke(m_pReceiver, nNotifyCode, wID, hwndCtrl);
}

GXLRESULT CMOListBox::InvokeReceiver(GXNMHDR* pnmhdr)
{
  return Invoke(m_pReceiver, pnmhdr);
}

GXLRESULT CMOListBox::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CMOListBoxReceiver* pListBoxReceiver = TryCastReceiver(pReceiver, m_pReceiver);

  if(pListBoxReceiver == NULL) {
    return -1;
  }

  switch(nNotifyCode)
  {
  case GXLBN_SELCANCEL:
    pListBoxReceiver->OnListBoxSelCancel(this);
    break;
  case GXLBN_SELCHANGE:
    pListBoxReceiver->OnListBoxSelChange(this);
    break;
  case GXLBN_ERRSPACE:
    pListBoxReceiver->OnListBoxErrorSpace(this);
    break;
  case GXLBN_DBLCLK:
    pListBoxReceiver->OnListBoxDoubleClicked(this);
    break;
  case GXLBN_SETFOCUS:
    pListBoxReceiver->OnListBoxSetFocus(this);
    break;
  case GXLBN_KILLFOCUS:
    pListBoxReceiver->OnListBoxKillFocus(this);
    break;
  }
  return 0;
}

GXLRESULT CMOListBox::Invoke(CGXReceiver* pReceiver, GXNMHDR* pnmhdr)
{
  CMOListBoxReceiver* pListBoxReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(pListBoxReceiver == 0) {
    return -1;
  }
  if(pnmhdr->code == GXLBN_CUSTCTRLCMD)
  {
    GXNMCUSTLISTCTRLCMD* pUIList = (GXNMCUSTLISTCTRLCMD*)pnmhdr;
    pListBoxReceiver->OnListBoxCtrlCommand(this, pUIList->hTmplItemWnd, pUIList->nListItem, pUIList->nCommand);
  }
  return 0;
}

void CMOListBox::SetReceiver(CMOListBoxReceiver* pReceiver)
{
  CMOListBox* pListBox = (CMOListBox*)GetCObject();
  ASSERT(pListBox != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pListBox->m_pReceiver = pReceiver;
}

CGXReceiver* CMOListBox::GetReceiver()
{
  CMOListBox* pListBox = (CMOListBox*)GetCObject();
  return pListBox->m_pReceiver;
}

void CMOListBox::SetReceiver(CGXReceiver* pReceiver)
{
  SetReceiver(TryCastReceiver<CMOListBoxReceiver>(pReceiver, NULL));
}
#endif // #ifdef REFACTOR_GXFC

GXCOLORREF CMOListBox::SetColor( GXUINT nType, GXCOLORREF color )
{
  return (GXCOLORREF)gxSendMessage(m_hWnd, GXLB_SETCOLOR, (GXWPARAM)nType, (GXLPARAM)color);
}

GXUINT CMOListBox::SetColumnsWidth( const GXUINT* pColumns, GXUINT nCount )
{
  return (GXUINT)gxSendMessage(m_hWnd, GXLB_SETCOLUMNSWIDTH, (GXWPARAM)nCount, (GXLPARAM)pColumns);
}

GXUINT CMOListBox::GetColumnsWidth( GXUINT* pColumns, GXUINT nCount )
{
  return (GXUINT)gxSendMessage(m_hWnd, GXLB_GETCOLUMNSWIDTH, (GXWPARAM)nCount, (GXLPARAM)pColumns);
}

GXBOOL CMOListBox::IsSelected( GXSIZE_T index ) const
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXLB_GETSEL, (GXWPARAM)index, 0);
}

#ifdef REFACTOR_GXFC
//////////////////////////////////////////////////////////////////////////

GXLRESULT CMOListBoxReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  //CGXListBoxReceiver* pListBoxReceiver = TryCastReceiver(pReceiver, m_pReceiver);

  //if(pListBoxReceiver == NULL) {
  //  return -1;
  //}
  CMOListBox ListBoxWnd(hwndCtrl);
  switch(nNotifyCode)
  {
  case GXLBN_SELCANCEL:
    OnListBoxSelCancel(&ListBoxWnd);
    break;
  case GXLBN_SELCHANGE:
    OnListBoxSelChange(&ListBoxWnd);
    break;
  case GXLBN_ERRSPACE:
    OnListBoxErrorSpace(&ListBoxWnd);
    break;
  case GXLBN_DBLCLK:
    OnListBoxDoubleClicked(&ListBoxWnd);
    break;
  case GXLBN_SETFOCUS:
    OnListBoxSetFocus(&ListBoxWnd);
    break;
  case GXLBN_KILLFOCUS:
    OnListBoxKillFocus(&ListBoxWnd);
    break;
  }
  return 0;
}

GXLRESULT CMOListBoxReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  //CGXListBoxReceiver* pListBoxReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  //if(pListBoxReceiver == 0) {
  //  return -1;
  //}
  if( ! gxIsWindow(pnmhdr->hwndFrom)) {
    return -1;
  }

  CMOListBox ListBoxWnd(pnmhdr->hwndFrom);
  if(pnmhdr->code == GXLBN_CUSTCTRLCMD)
  {
    GXNMCUSTLISTCTRLCMD* pUIList = (GXNMCUSTLISTCTRLCMD*)pnmhdr;
    OnListBoxCtrlCommand(&ListBoxWnd, pUIList->hTmplItemWnd, pUIList->nListItem, pUIList->nCommand);
  }
  return 0;
}
#endif // #ifdef REFACTOR_GXFC