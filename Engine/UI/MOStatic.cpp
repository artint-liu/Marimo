#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOStatic.h"

CMOStatic::CMOStatic()
  : CMODlgItem(NULL)
  , m_pDelegate(NULL)
{
}

CMOStatic::CMOStatic(GXHWND hWnd)
  : CMODlgItem(hWnd)
  , m_pDelegate(NULL)
{
}

CMOStatic::~CMOStatic()
{

}
GXSIZE_T CMOStatic::GetThisSizeOf() const
{
  return sizeof(CMOStatic);
}

clStringW CMOStatic::GetClassName() const
{
  clStringW ClsName(GXUICLASSNAME_TOOLBAR);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOStatic::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','S','T','C');
}

//GXBOOL CGXStatic::SetSpriteFile(GXLPCWSTR lpSpriteFile)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEFILE, NULL, (GXLPARAM)lpSpriteFile);
//}
//
//GXBOOL CGXStatic::SetSpriteObj(GSprite* pSprite)
//{
//  return gxSendMessage(m_hWnd, GXUITB_SETSPRITEOBJ, NULL, (GXLPARAM)pSprite);
//}
//
//GXINT CGXStatic::AddButtons(const GXTBBUTTON* pButtons, int nButtons)
//{
//  return gxSendMessage(m_hWnd, GXTB_ADDBUTTONS, nButtons, (GXLPARAM)pButtons);
//}
//
//GXVOID CGXStatic::AutoSize()
//{
//  gxSendMessage(m_hWnd, GXTB_AUTOSIZE, 0, 0);
//}
//
//GXBOOL CGXStatic::SetBitmapSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, GXTB_SETBITMAPSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXStatic::SetButtonSize(int cx, int cy)
//{
//  return gxSendMessage(m_hWnd, GXTB_SETBUTTONSIZE, NULL, GXMAKELONG(cx, cy));
//}
//
//GXBOOL CGXStatic::SetCommandProc(LPCOMMANDPROC pCmd)
//{
//  //m_pCmd = pCmd;
//  return TRUE;
//}

//void CGXStatic::SetDelegate(CGXStaticDelegate* pDelegate)
//{
//  CGXStatic* pToolbar = (CGXStatic*)GetLong(GXGWL_COBJECT);
//  ASSERT(pToolbar != NULL);
//  pToolbar->m_pDelegate = pDelegate;
//  m_pDelegate = NULL;
//}
//
GXLRESULT CMOStatic::Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  //CGXStaticReceiver* pStaticReceiver = TryCastReceiver(pReceiver, m_pReceiver);
  return 0;
}

GXBOOL CGXStaticSprite::SetSpriteByFilenameW(GXLPCWSTR szSpriteFile)
{
  GXHRESULT hval = gxSendMessage(m_hWnd, GXSSM_SETSPRITEBYFILENAMEW, NULL, (GXLPARAM)szSpriteFile);
  return (hval == 0);
}

GXBOOL CGXStaticSprite::SetSprite(GXSprite* pSprite)
{
  GXHRESULT hval = gxSendMessage(m_hWnd, GXSSM_SETSPRITE, NULL, (GXLPARAM)pSprite);
  return (hval == 0);
}

GXBOOL CGXStaticSprite::SetModuleByNameW(GXLPCWSTR szModuleName)
{
  GXHRESULT hval = gxSendMessage(m_hWnd, GXSSM_SETMODULEBYNAMEW, NULL, (GXLPARAM)szModuleName);
  return (hval == 0);
}

GXBOOL CGXStaticSprite::SetModuleByIndex(GXUINT nModuleIndex)
{
  GXHRESULT hval = gxSendMessage(m_hWnd, GXSSM_SETMODULEBYINDEX, (GXWPARAM)nModuleIndex, NULL);
  return (hval == 0);
}

//
//GXLRESULT CGXStaticDelegate::OnBtnClicked(CGXStatic* pSender, DWORD_PTR idButton)
//{
//  return 0;
//}