#include "GameEngine.h"
#include "GXFCAgent.H"
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "GXToolbar.h"

CMOToolbar::CMOToolbar()
  : CMODlgItem(NULL)
  , m_pReceiver(NULL)
{
}

CMOToolbar::CMOToolbar(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pReceiver(NULL)
{
}

CMOToolbar::~CMOToolbar()
{

}
GXSIZE_T CMOToolbar::GetThisSizeOf() const
{
  return sizeof(CMOToolbar);
}

clStringW CMOToolbar::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_TOOLBAR);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOToolbar::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','T','B','R');
}

GXBOOL CMOToolbar::SetSpriteFile(GXLPCWSTR lpSpriteFile)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXUITB_SETSPRITEFILE, NULL, (GXLPARAM)lpSpriteFile);
}

GXBOOL CMOToolbar::SetSpriteObj(GXSprite* pSprite)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXUITB_SETSPRITEOBJ, NULL, (GXLPARAM)pSprite);
}

GXINT CMOToolbar::AddButtons(const GXTBBUTTON* pButtons, int nButtons)
{
  return (GXINT)gxSendMessage(m_hWnd, GXTB_ADDBUTTONS, nButtons, (GXLPARAM)pButtons);
}

GXVOID CMOToolbar::AutoSize()
{
  gxSendMessage(m_hWnd, GXTB_AUTOSIZE, 0, 0);
}

GXBOOL CMOToolbar::SetBitmapSize(int cx, int cy)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_SETBITMAPSIZE, NULL, GXMAKELONG(cx, cy));
}

GXBOOL CMOToolbar::SetButtonSize(int cx, int cy)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_SETBUTTONSIZE, NULL, GXMAKELONG(cx, cy));
}

GXBOOL CMOToolbar::SetCommandProc(LPCOMMANDPROC pCmd)
{
  //m_pCmd = pCmd;
  return TRUE;
}

#ifdef REFACTOR_GXFC
#else
void CMOToolbar::SetReceiver(CMOToolbarReceiver* pReceiver)
{
  CMOToolbar* pToolbar = (CMOToolbar*)GetCObject();
  ASSERT(pToolbar != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pToolbar->m_pReceiver = pReceiver;
}

CGXReceiver* CMOToolbar::GetReceiver()
{
  CMOToolbar* pToolbar = (CMOToolbar*)GetCObject();
  return pToolbar->m_pReceiver;
}

void CMOToolbar::SetReceiver(CGXReceiver* pReceiver)
{
  CMOToolbar* pToolbar = (CMOToolbar*)GetCObject();
  ASSERT(pToolbar != NULL);
  m_pReceiver = NULL; // 这个要放在前面,避免this和CObject是相同的
  pToolbar->m_pReceiver = TryCastReceiver(pReceiver, pToolbar->m_pReceiver);
}

GXLRESULT CMOToolbar::InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  return Invoke(m_pReceiver, nNotifyCode, wID, hwndCtrl);
}

GXLRESULT CMOToolbar::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CMOToolbarReceiver* pToolbarReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  if(nNotifyCode == GXBN_CLICKED && pToolbarReceiver != NULL)
  {
    GXLPCWSTR idButton = (GXLPCWSTR)gxSendMessage(m_hWnd, GXWM_GETIDNAMEW, wID, NULL);
    return pToolbarReceiver->OnBtnClicked(this, idButton);
  }
  return 0;
}
#endif // #ifdef REFACTOR_GXFC

GXBOOL CMOToolbar::IsButtonChecked(GXINT_PTR id)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_ISBUTTONCHECKED, id, NULL);
}

GXBOOL CMOToolbar::CheckButton(GXINT_PTR id, GXBOOL fCheck)
{
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_CHECKBUTTON, id, GXMAKELONG(fCheck, 0));
}

GXBOOL CMOToolbar::SetButtonBitmap(GXINT_PTR id, GXINT idBitmap)
{
  GXTBBUTTONINFOW Info = {sizeof(GXTBBUTTONINFOW), GXTBIF_IMAGE, 0, idBitmap, 0};
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_SETBUTTONINFOW, (GXWPARAM)id, (GXLPARAM)&Info);
}

GXINT CMOToolbar::GetButtonBitmap(GXINT_PTR id)
{
  GXTBBUTTONINFOW Info = {sizeof(GXTBBUTTONINFOW), GXTBIF_IMAGE, 0};
  if(gxSendMessage(m_hWnd, GXTB_GETBUTTONINFOW, (GXWPARAM)id, (GXLPARAM)&Info)) {
    return Info.iImage;
  }
  return -1;
}

GXBOOL CMOToolbar::SetButtonBitmapByIndex(GXINT_PTR id, GXINT idBitmap)
{
  GXTBBUTTONINFOW Info = {sizeof(GXTBBUTTONINFOW), GXTBIF_IMAGE|GXTBIF_BYINDEX, 0, idBitmap, 0};
  return (GXBOOL)gxSendMessage(m_hWnd, GXTB_SETBUTTONINFOW, (GXWPARAM)id, (GXLPARAM)&Info);
}

GXINT CMOToolbar::GetButtonBitmapByIndex(GXINT_PTR id)
{
  GXTBBUTTONINFOW Info = {sizeof(GXTBBUTTONINFOW), GXTBIF_IMAGE|GXTBIF_BYINDEX, 0};
  if(gxSendMessage(m_hWnd, GXTB_GETBUTTONINFOW, (GXWPARAM)id, (GXLPARAM)&Info)) {
    return Info.iImage;
  }
  return -1;
}

GXBOOL CMOToolbar::HideButton(GXINT_PTR idCommand, GXBOOL bHide)
{
  return gxSendMessage(m_hWnd, GXTB_HIDEBUTTON, (GXWPARAM)idCommand, (GXLPARAM)bHide);
}

GXLPCWSTR CMOToolbar::GetIdentifierName( GXINT nButtonID ) const
{
  return (GXLPCWSTR)gxSendMessage(m_hWnd, GXWM_GETIDNAMEW, nButtonID, NULL);
}

GXLRESULT CMOToolbarReceiver::OnBtnClicked( CMOToolbar* pSender, GXLPCWSTR idButton )
{
  return 0;
}
#ifdef REFACTOR_GXFC
GXLRESULT CMOToolbarReceiver::InvokeCommand( GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl )
{
  if(nNotifyCode == GXBN_CLICKED)
  {
    CMOToolbar ToolbarWnd(hwndCtrl);
    GXLPCWSTR idButton = ToolbarWnd.GetIdentifierName(wID);
    return OnBtnClicked(&ToolbarWnd, idButton);
  }
  return 0;
}

GXLRESULT CMOToolbarReceiver::InvokeNotify( GXNMHDR* pnmhdr )
{
  CLBREAK;
  return 0;
}
#endif // #ifdef REFACTOR_GXFC