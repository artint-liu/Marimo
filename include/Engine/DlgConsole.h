#ifndef _MARIMO_DIALOG_CONSOLE_H_
#define _MARIMO_DIALOG_CONSOLE_H_
class IStreamLogger;
class GAMEENGINE_API CDlgConsole : public CMODialog, public CMOButtonReceiver//, public CGXListBoxReceiver
{
  //GX_DECLARE_COMMAND_MAP()
  //CGXListBox        m_ItemList;
  //clStringW         m_strRootDir;
  //CMOButton         m_BtnEnter;
  //CMOButton         m_BtnRename;
  //CMOButton         m_BtnDelete;
  typedef clist<clStringW>  CmdCache;
  static GXLONG_PTR   s_OldWndProc;
  static const int    s_nMaxCmdCache = 16;
  CMOEdit             m_Output;
  CMOEdit             m_Input;
  CmdCache            m_listCmd;
public:
  //clStringW         m_strOpType;
  //clStringW         m_strMapName;
  //clStringW         m_strMapSeed;
  static LPCWSTR idTemplate; // = {L"@Resource\\console.dlg.txt"};

public:
  CDlgConsole()
    : CMODialog(NULL, idTemplate, NULL)
    //, m_strRootDir(szRootDir)
  {
  }
  ~CDlgConsole()
  {    
  }

  void OutputW(GXLPCWSTR szString);
  void OutputA(GXLPCSTR szString);
  void AttachLogger(IStreamLogger* pLogger);

  virtual GXVOID OnInitDialog();
  virtual GXLRESULT OnDestory();
  virtual GXLRESULT OnShow(GXBOOL bShow, int nStatus);
  //virtual GXLRESULT OnListBoxClicked(CGXListBox* pSender, GXLPCWSTR szBtnName);
  //virtual GXLRESULT OnListBoxSelChange(CGXListBox* pSender);
  //virtual GXLRESULT OnListBoxSelCancel (CGXListBox* pSender);
  //virtual GXLRESULT OnBtnClicked(CMOButton* pSender, GXLPCWSTR szBtnName);
  virtual GXVOID OnBtnClicked(CMOButton* pSender);
private:
  void ScrollToPreviousCommand();
  static GXLRESULT GXCALLBACK EditWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  static GXINT GXCALLBACK LogStreamProc(GXLPVOID szText, GXBOOL bUnicode, GXLPARAM lParam);
};

#endif // #ifndef _MARIMO_DIALOG_CONSOLE_H_