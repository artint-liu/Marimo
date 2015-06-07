#ifndef _MOUI_STAFF_H_
#define _MOUI_STAFF_H_

class SampleApp_MOUI;

class MOUIStaff : public IConsoleStaff
{
  SampleApp_MOUI* m_pMyApp;
  typedef clset<int> SelectionSet;

  SelectionSet  m_WordSel;
public:
  MOUIStaff(SampleApp_MOUI* pApp);
  virtual ~MOUIStaff();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT        AddRef      ();
  virtual GXHRESULT        Release     ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual LPCSTAFFCAPSDESC GetCapacity () const;
  virtual GXHRESULT        Execute     (int nCmdIndex, const clStringW* argv, int argc);
};

#endif // _MOUI_STAFF_H_