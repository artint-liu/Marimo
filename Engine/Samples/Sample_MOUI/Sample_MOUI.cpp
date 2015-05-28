#include <windows.h>
#include <tchar.h>
#include "Marimo.h"
#include "Engine/UniversalDialog.h"
#include "clpathfile.h"
#include "Sample_MOUI.h"

// Global Variables:
//HINSTANCE hInst;								// current instance

class SampleApp_MOUI : public GXApp
{
  MODataPool*       m_pBasicDataPool;
  CUniversalDialog* m_pDlgBasic;
public:
  SampleApp_MOUI()
    : m_pBasicDataPool(NULL)
    , m_pDlgBasic(NULL)
  {
  }

  virtual HRESULT OnCreate()
  {
    MODataPool::CompileFromFileW(&m_pBasicDataPool, "BasicPool", L"Test/UI/BaseDataPool.txt");
    m_pDlgBasic = new CUniversalDialog(L"Test/UI/DlgBasic.txt", NULL);
    m_pDlgBasic->CreateDlg();
    m_pDlgBasic->Show(TRUE);

    m_pBasicDataPool->WatchFor("fScale", [this](Marimo::LPCDATAIMPULSE pImpulse){
      (*m_pBasicDataPool)["fScaleAddition"] = pImpulse->sponsor->ToFloat() + 1.0f;
    });
    return GX_OK;
  }

  virtual HRESULT OnDestroy()
  {
    SAFE_DELETE(m_pDlgBasic);
    SAFE_RELEASE(m_pBasicDataPool);
    return GX_OK;
  }

  virtual HRESULT Render()
  {
    GXDWORD crColor = 0xff808080;
    m_pGraphics->Clear(0, NULL, GXCLEAR_TARGET, crColor, 1.0f, 0);
    
    if(GXSUCCEEDED(m_pGraphics->Begin()))
    {
      //Test1(rect);

      GXUIUpdateTimerEvent();
      GXRenderRootFrame();
      m_pGraphics->End();
      m_pGraphics->Present();
    }

    return GX_OK;
  }
};

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  clpathfile::LocalWorkingDirW(L"..");

  SampleApp_MOUI app;
  GXAPP_DESC sAppDesc = {0};
  sAppDesc.cbSize = sizeof(GXAPP_DESC);
  sAppDesc.lpName = L"Test Marimo Windows";
  sAppDesc.nWidth = 0;
  sAppDesc.nHeight = 0;
  sAppDesc.dwStyle = GXADS_SIZABLE;

  return (int)app.Go(&sAppDesc);
}
