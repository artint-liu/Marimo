#include <GrapX.H>
#include "GUnknown.H"
#include "GameEngine.h"
#include "GXFCAgent.H"
#include "GXWndBase.H"
#include "GXDialog.H"
#include "GXDlgItemBase.H"
#include "GXButton.h"
#include "GXEdit.H"
#include "MOLogger.h"
#include "GXKernel.H"

#include "UniversalDialog.h"

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