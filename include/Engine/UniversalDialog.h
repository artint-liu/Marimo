#ifndef _GAMEENGINE_UNIVERSAL_DIALOG_H_
#define _GAMEENGINE_UNIVERSAL_DIALOG_H_

class GAMEENGINE_API CUniversalDialog : public CMODialog, public CMOButtonReceiver, public CMOListBoxReceiver
{
protected:
  clStringW m_strTemplate;

  GXVOID IntExecuteVar(CMODlgItem* pSender, GXLPCWSTR szCmdFormat, ...);
public:
  CUniversalDialog(GXLPCWSTR idTemplate, CMOWnd* pParent)
    : CMODialog(NULL, idTemplate, pParent)
    , m_strTemplate(idTemplate)
  {
  }

  ~CUniversalDialog()
  {    
  }

  // Button
  virtual GXVOID OnBtnClicked             (CMOButton* pSender);

  // List box
  virtual GXVOID OnListBoxSelChange       (CMOListBox* pSender);
  virtual GXVOID OnListBoxSelCancel       (CMOListBox* pSender);

};

#endif // _GAMEENGINE_UNIVERSAL_DIALOG_H_