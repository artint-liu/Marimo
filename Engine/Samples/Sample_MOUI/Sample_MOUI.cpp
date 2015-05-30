#include <windows.h>
#include <tchar.h>
#include "Marimo.h"
#include "GrapX/MOConsoleStaff.h"
#include "Engine/UniversalDialog.h"
#include "clpathfile.h"
#include "Sample_MOUI.h"
#include "MOUIStaff.h"

//////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  SampleApp_MOUI app;

  // m_pStreamLogger ��Ϊ����App�����������ڶ����ã�����ҪԤ�ȴ���
  MOCreateMemoryStreamLoggerW(&app.m_pStreamLogger, 32768, FALSE);

  clpathfile::LocalWorkingDirW(L"..");

  GXAPP_DESC sAppDesc = {0};
  sAppDesc.cbSize = sizeof(GXAPP_DESC);
  sAppDesc.lpName = L"Test Marimo Windows";
  sAppDesc.nWidth = 0;
  sAppDesc.nHeight = 0;
  sAppDesc.dwStyle = GXADS_SIZABLE;
  sAppDesc.pLogger = app.m_pStreamLogger;

  int result = (int)app.Go(&sAppDesc);
  SAFE_RELEASE(app.m_pStreamLogger);
  return result;
}

//////////////////////////////////////////////////////////////////////////

SampleApp_MOUI::SampleApp_MOUI() : m_pBasicDataPool(NULL)
  , m_pDlgConsole(NULL)
  , m_pDlgBasic(NULL)
  , m_pDlgUnitSelect(NULL)
  , m_pStreamLogger(NULL)
{
}

HRESULT SampleApp_MOUI::OnCreate()
{
  //
  // ��������̨UI
  //
  m_pDlgConsole = MOCreateConsoleDlg(m_pStreamLogger);

  //
  // ����App�Լ�ʹ�õĿ���ִ̨�ж���
  //
  MOUIStaff* pStaff = new MOUIStaff(this);
  if( ! InlCheckNewAndIncReference(pStaff)) {
    return GX_FAIL;
  }
  MORegisterConsoleStaff(pStaff);
  SAFE_RELEASE(pStaff); // ע����������ʹ�ã���������������ͷŵ�

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

  // ����������������İ�ť����
  (*m_pBasicDataPool)["button[0]"] = "ButtonName";

  //
  // ͨ�������޸�֪ͨ����ʼ���󶨡�fInvScaling���Ļ����ؼ���״̬
  //
  MOVariable var;
  m_pBasicDataPool->QueryByExpression("fScaling", &var);
  var.Impulse(Marimo::DATACT_Change);
  return GX_OK;
}

HRESULT SampleApp_MOUI::OnDestroy()
{
  SAFE_DELETE(m_pDlgBasic);
  SAFE_DELETE(m_pDlgUnitSelect);
  SAFE_DELETE(m_pDlgConsole);
  SAFE_RELEASE(m_pBasicDataPool);
  return GX_OK;
}

GXHRESULT SampleApp_MOUI::KeyMessage( GXAPPKEYINFO* pKeyInfo )
{
  switch(pKeyInfo->dwAction)
  {
  case GXWM_KEYDOWN:
    //m_pKeyboardStaff->OnKeyDown(pKeyInfo->dwKey);
    switch(pKeyInfo->dwKey)
    {
    case VK_OEM_3: // "`"
      ////m_pGraphics->SwitchConsole();
      MOExecuteConsoleCmdW(L"console");
      //m_pDlgConsole->Show( ! m_pDlgConsole->IsVisible()); // ��Ϊ"console"����
    }
  }
  return GX_OK;
}

HRESULT SampleApp_MOUI::Render()
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
