#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOButton.h"

CMOButton::CMOButton()
  : CMODlgItem(NULL)
  , m_pReceiver(NULL)
{
}

CMOButton::CMOButton(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pReceiver(NULL)
{
}

CMOButton::~CMOButton()
{

}

GXSIZE_T CMOButton::GetThisSizeOf() const
{
  return sizeof(CMOButton);
}

clStringW CMOButton::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_BUTTON);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOButton::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','B','T','N');
}

//
//GXBOOL CMOButton::SetSpriteFile(GXLPCWSTR lpSpriteFile)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEFILE, NULL, (GXLPARAM)lpSpriteFile);
//}
//
//GXBOOL CMOButton::SetSpriteObj(GSprite* pSprite)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEOBJ, NULL, (GXLPARAM)pSprite);
//}
//
//GXINT CMOButton::AddButtons(const GXTBBUTTON* pButtons, int nButtons)
//{
//  return gxSendMessage(m_hWnd, TB_ADDBUTTONS, nButtons, (GXLPARAM)pButtons);
//}
//
//GXVOID CMOButton::AutoSize()
//{
//  gxSendMessage(m_hWnd, TB_AUTOSIZE, 0, 0);
//}
//
//GXBOOL CMOButton::SetBitmapSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBITMAPSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CMOButton::SetButtonSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, TB_SETBUTTONSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CMOButton::SetCommandProc(LPCOMMANDPROC pCmd)
//{
//  //m_pCmd = pCmd;
//  return TRUE;
//}

#ifdef REFACTOR_GXFC
#else
void CMOButton::SetReceiver(CMOButtonReceiver* pReceiver)
{
  CMOButton* pButton = (CMOButton*)GetCObject();
  ASSERT(pButton != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pButton->m_pReceiver = pReceiver;
}

CGXReceiver* CMOButton::GetReceiver()
{
  CMOButton* pButton = (CMOButton*)GetCObject();
  return pButton->m_pReceiver;
}

void CMOButton::SetReceiver(CGXReceiver* pReceiver)
{
  SetReceiver(TryCastReceiver<CMOButtonReceiver>((CMOButtonReceiver*)pReceiver, NULL));
}

GXLRESULT CMOButton::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return Invoke(m_pReceiver, nNotifyCode, wID, hwndCtrl);
}

GXLRESULT CMOButton::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CMOButtonReceiver* pBtnReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(nNotifyCode == GXBN_CLICKED && pBtnReceiver != NULL)
  {
    return pBtnReceiver->OnBtnClicked(this, GetIdentifierName());
  }
  return 0;
}
#endif // #ifdef REFACTOR_GXFC

GXVOID CMOButtonReceiver::OnBtnClicked(CMOButton* pSender)
{
}

#ifdef REFACTOR_GXFC
GXLRESULT CMOButtonReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  if(nNotifyCode == GXBN_CLICKED)
  {
    CMOButton sButton(hwndCtrl);
    OnBtnClicked(&sButton);
  }
  return 0;
}

GXLRESULT CMOButtonReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  CLBREAK;
  return 0;
}
#endif // #ifdef REFACTOR_GXFC