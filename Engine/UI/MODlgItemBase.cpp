#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"

CMODlgItem::CMODlgItem(GXHWND hWnd) : CMOWnd(hWnd)
{
}

CMODlgItem::~CMODlgItem()
{
}

GXLPCWSTR CMODlgItem::GetIdentifierName() const
{
  return (GXLPCWSTR)gxSendMessage(m_hWnd, GXWM_GETIDNAMEW, NULL, NULL);
}

#ifdef REFACTOR_GXFC
#else
CGXReceiver* CMODlgItem::GetReceiver()
{
  return RECEIVER_REFUSED;
}

void CMODlgItem::SetReceiver(CGXReceiver* pReceiver)
{
}

GXLRESULT CMODlgItem::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return -1;
}

GXLRESULT CMODlgItem::InvokeReceiver(GXNMHDR* pnmhdr)
{
  return -1;
}
#endif // #ifdef REFACTOR_GXFC

//CMOWnd& CGXDlgItemBase::operator=(CGXDlgItemBase* pWnd)
//{
//  return this->operator=(*pWnd);
//}

//CMOWnd& CGXDlgItemBase::operator=(CGXDlgItemBase& Wnd)
//{
//  ASSERT(GetClassHashName() == Wnd.GetClassHashName() &&
//    GetThisSizeOf() == Wnd.GetThisSizeOf());
//
//  const size_t c_nVPtrSize = sizeof(void*);
//  memcpy(this + c_nVPtrSize, &Wnd + c_nVPtrSize, GetThisSizeOf() - c_nVPtrSize);
//
//  return *this;
//}
