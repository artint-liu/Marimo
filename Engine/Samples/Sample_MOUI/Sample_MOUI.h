#pragma once

class CUniversalDialog;
class SampleApp_MOUI : public GXApp
{
  friend class MOUIStaff;
protected:
  MODataPool*       m_pBasicDataPool;
  CUniversalDialog* m_pDlgBasic;
  CUniversalDialog* m_pDlgUnitSelect;
  CMODialog*        m_pDlgConsole;

public:
  IStreamLogger*    m_pStreamLogger;
public:
  SampleApp_MOUI();

  virtual HRESULT OnCreate();
  virtual HRESULT OnDestroy();
  virtual HRESULT Render();
  virtual GXHRESULT KeyMessage(GXAPPKEYINFO* pKeyInfo);
};

