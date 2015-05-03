#ifndef _MARIMO_CONSOLE_ASSISTANT_H_
#define _MARIMO_CONSOLE_ASSISTANT_H_

class ConsoleAssistant : public IConsoleStaff
{
  static STAFFCAPSDESC s_aStaffDesc[];
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT        AddRef      ();
  virtual GXHRESULT        Release     ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual LPCSTAFFCAPSDESC GetCapacity () const;
  virtual GXHRESULT        Execute     (int nCmdIndex, const clStringW* argv, int argc);

  ConsoleAssistant();
  ~ConsoleAssistant();

private:
};

#endif // #ifndef _MARIMO_CONSOLE_ASSISTANT_H_