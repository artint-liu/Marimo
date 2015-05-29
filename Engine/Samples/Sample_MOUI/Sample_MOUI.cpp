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
    // �����ȴ��� DataPool������������UI�����ҵ�������
    //
    MODataPool::CompileFromFileW(&m_pBasicDataPool, "BasicPool", L"Test/UI/BaseDataPool.txt");

    //
    // �����Ի���
    //
    // ��������Ի������DataPool֪ͨ������Ҫдר�д��룬����ʹ����ͨ�öԻ�����
    m_pDlgBasic = new CUniversalDialog(L"Test/UI/DlgBasic.txt", NULL);
    m_pDlgBasic->CreateDlg();
    m_pDlgBasic->Show(TRUE);

    m_pDlgUnitSelect = new CUniversalDialog(L"Test/UI/DlgUnitSelect.txt", NULL);
    m_pDlgUnitSelect->CreateDlg();
    m_pDlgUnitSelect->Show(TRUE);

    //
    // �������� WatchFor ��ͬ����ʵ���� "fInvScaling <=> fScaling" ��˫���޸�
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
    // ͨ�������޸�֪ͨ����ʼ���󶨡�fInvScaling���Ļ����ؼ���״̬
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
