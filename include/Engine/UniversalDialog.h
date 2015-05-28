#ifndef _GAMEENGINE_UNIVERSAL_DIALOG_H_
#define _GAMEENGINE_UNIVERSAL_DIALOG_H_

class GAMEENGINE_API CUniversalDialog : public CMODialog, public CMOButtonReceiver//, public CGXListBoxReceiver
{
protected:
  clStringW m_strTemplate;

public:
  CUniversalDialog(GXLPCWSTR idTemplate, CMOWnd* pParent)
    : CMODialog(NULL, idTemplate, pParent)
    , m_strTemplate(idTemplate)
  {
  }

  ~CUniversalDialog()
  {    
  }

  virtual GXLRESULT OnBtnClicked(CMOButton* pSender, GXLPCWSTR szBtnName);
};

#endif // _GAMEENGINE_UNIVERSAL_DIALOG_H_