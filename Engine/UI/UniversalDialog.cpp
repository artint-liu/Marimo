#include <GrapX.H>
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOButton.h"
#include "Engine/MOList.h"
#include "Engine/MOEdit.H"
#include "GrapX/MOLogger.h"
#include "GrapX/GXKernel.H"

#include "Engine/UniversalDialog.h"

GXVOID CUniversalDialog::OnBtnClicked(CMOButton* pSender)
{
  //MOExecuteConsoleCmdW(szBtnName);
  IntExecuteVar(pSender, NULL);
}

GXVOID CUniversalDialog::OnListBoxSelChange( CMOListBox* pSender )
{
  auto index = pSender->GetCurSel();
  IntExecuteVar(pSender, L"SelChange %d %d", index, pSender->IsSelected(index));
}

GXVOID CUniversalDialog::OnListBoxSelCancel( CMOListBox* pSender )
{
  IntExecuteVar(pSender, L"SelCancel %d", pSender->GetCurSel());
}

GXVOID CUniversalDialog::IntExecuteVar(CMODlgItem* pSender, GXLPCWSTR szCmdFormat, ...)
{
  clStringW str;
  GXLPCWSTR szName = pSender->GetIdentifierName();
  
  if(szName && szName[0])
  {
    if(szCmdFormat) {
      va_list  arglist;
      va_start(arglist, szCmdFormat);

      str.Append(szName);
      str.Append(' ');
      str.VarFormat(szCmdFormat, arglist);
      MOExecuteConsoleCmdW(str);
    }
    else {
      MOExecuteConsoleCmdW(szName);
    }
  }
}

extern "C"
{
  GAMEENGINE_API CMODialog* GXUICreateUniversalDialog(GXLPCWSTR szTemplate, CMOWnd* pParent)
  {
    return new CUniversalDialog(szTemplate, pParent);
  }
} // extern "C"