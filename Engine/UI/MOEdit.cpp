#include "GrapX.h"
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOEdit.H"

CMOEdit::CMOEdit() : CMODlgItem(NULL)
{
}

CMOEdit::CMOEdit(GXHWND hWnd) : CMODlgItem(hWnd)
{
}

CMOEdit::~CMOEdit()
{
}

GXSIZE_T CMOEdit::GetThisSizeOf() const
{
  return sizeof(CMOEdit);
}

clStringW CMOEdit::GetClassName() const
{
  clStringW ClsName(GXWE_EDITW);
  ClsName.MakeUpper();
  return ClsName;
}

GXDWORD CMOEdit::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','E','D','T');
}

void CMOEdit::SetSelect( GXINT nStart, GXINT nEnd )
{
  gxSendMessage(m_hWnd, GXEM_SETSEL, nStart, nEnd);
}

void CMOEdit::ReplaceSelectW( GXLPCWSTR szReplace, GXBOOL bCanUndo )
{
  gxSendMessage(m_hWnd, GXEM_REPLACESEL, bCanUndo, (GXLPARAM)szReplace);
}

void CMOEdit::ReplaceSelectA( GXLPCSTR szReplace, GXBOOL bCanUndo )
{
  clStringW str(szReplace);
  gxSendMessage(m_hWnd, GXEM_REPLACESEL, bCanUndo, (GXLPARAM)(GXLPCWSTR)str);
}

GXUINT CMOEdit::SetLimitText( GXUINT nMaxLength )
{
  return gxSendMessage(m_hWnd, GXEM_SETLIMITTEXT, (GXWPARAM)nMaxLength, 0);
}

GXUINT CMOEdit::GetLimitText()
{
  return gxSendMessage(m_hWnd, GXEM_GETLIMITTEXT, 0, 0);
}
