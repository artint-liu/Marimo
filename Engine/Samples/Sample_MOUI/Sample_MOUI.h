#pragma once

class CUniversalDialog;
class SampleApp_MOUI : public GXApp
{
  friend class MOUIStaff;
protected:
  MODataPool*       m_pBasicDataPool;
  CUniversalDialog* m_pDlgBasic;
  CUniversalDialog* m_pDlgList;
  CUniversalDialog* m_pDlgRichList;
  CUniversalDialog* m_pDlgUnitSelect;
  CUniversalDialog* m_pDlgPropertyList;
  CMODialog*        m_pDlgConsole;

public:
  IStreamLogger*    m_pStreamLogger;
public:
  SampleApp_MOUI();

  virtual HRESULT OnCreate();
  virtual HRESULT OnDestroy();
  virtual HRESULT Render();
  virtual GXHRESULT KeyMessage(GXAPPKEYINFO* pKeyInfo);
private:
  void InitDataPool();
};

