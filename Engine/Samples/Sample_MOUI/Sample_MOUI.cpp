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
  CUniversalDialog* m_pDlgUnitSelect;
public:
  SampleApp_MOUI()
    : m_pBasicDataPool(NULL)
    , m_pDlgBasic(NULL)
    , m_pDlgUnitSelect(NULL)
  {
  }

  virtual HRESULT OnCreate()
  {
    //
    // 必须先创建 DataPool，后来创建的UI才能找到它并绑定
    //
    MODataPool::CompileFromFileW(&m_pBasicDataPool, "BasicPool", L"Test/UI/BaseDataPool.txt");

    //
    // 创建对话框
    //
    // 由于这个对话框基于DataPool通知，不需要写专有代码，所以使用了通用对话框类
    m_pDlgBasic = new CUniversalDialog(L"Test/UI/DlgBasic.txt", NULL);
    m_pDlgBasic->CreateDlg();
    m_pDlgBasic->Show(TRUE);

    m_pDlgUnitSelect = new CUniversalDialog(L"Test/UI/DlgUnitSelect.txt", NULL);
    m_pDlgUnitSelect->CreateDlg();
    m_pDlgUnitSelect->Show(TRUE);

    //
    // 下面两个 WatchFor 共同作用实现了 "fInvScaling <=> fScaling" 的双向修改
    //
    // fScaling => fHalfScaling
    // fScaling => fInvScaling
    m_pBasicDataPool->WatchFor("fScaling", [this](Marimo::LPCDATAIMPULSE pImpulse){
      float fScaling = pImpulse->sponsor->ToFloat();
      (*m_pBasicDataPool)["fHalfScaling"] = fScaling * 0.5f;
      (*m_pBasicDataPool)["fInvScaling"] = 100.0f - fScaling;
    });

    // fInvScaling => fScaling
    m_pBasicDataPool->WatchFor("fInvScaling", [this](Marimo::LPCDATAIMPULSE pImpulse){
      float fScaling = pImpulse->sponsor->ToFloat();
      (*m_pBasicDataPool)["fScaling"] = 100.0f - fScaling;
    });

    (*m_pBasicDataPool)["button[0]"] = "ButtonName";

    //
    // 通过推送修改通知，初始化绑定“fInvScaling”的滑动控件的状态
    //
    MOVariable var;
    m_pBasicDataPool->QueryByExpression("fScaling", &var);
    var.Impulse(Marimo::DATACT_Change);
    return GX_OK;
  }

  virtual HRESULT OnDestroy()
  {
    SAFE_DELETE(m_pDlgBasic);
    SAFE_DELETE(m_pDlgUnitSelect);
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
