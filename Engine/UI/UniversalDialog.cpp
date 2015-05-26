#include <GrapX.H>
#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/MOButton.h"
#include "Engine/MOEdit.H"
#include "GrapX/MOLogger.h"
#include "GrapX/GXKernel.H"

#include "Engine/UniversalDialog.h"

GXLRESULT CUniversalDialog::OnBtnClicked(CMOButton* pSender, GXLPCWSTR szBtnName)
{
  MOExecuteConsoleCmdW(szBtnName);
  return GX_OK;
}

extern "C"
{
  GAMEENGINE_API CMODialog* GXUICreateUniversalDialog(GXLPCWSTR szTemplate, CMOWnd* pParent)
  {
    return new CUniversalDialog(szTemplate, pParent);
  }
} // extern "C"