#include "GameEngine.h"
#include "GXFCAgent.H"
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "WEEdit.H"

CWEEdit::CWEEdit() : CMODlgItem(NULL)
{
}

CWEEdit::CWEEdit(GXHWND hWnd) : CMODlgItem(hWnd)
{
}

CWEEdit::~CWEEdit()
{
}

GXSIZE_T CWEEdit::GetThisSizeOf() const
{
  return sizeof(CWEEdit);
}

clStringW CWEEdit::GetClassName() const
{
  clStringW ClsName(GXWC_EDITW);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CWEEdit::GetClassNameCode() const
{
  return GXMAKEFOURCC('W','E','D','T');
}

void CWEEdit::SetSelect( GXINT nStart, GXINT nEnd )
{
  gxSendMessage(m_hWnd, GXEM_SETSEL, nStart, nEnd);
}

void CWEEdit::ReplaceSelectW( GXLPCWSTR szReplace, GXBOOL bCanUndo )
{
  gxSendMessage(m_hWnd, GXEM_REPLACESEL, bCanUndo, (GXLPARAM)szReplace);
}
