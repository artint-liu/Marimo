#ifndef _GRAP_X_UI_BUTTON_H_
#define _GRAP_X_UI_BUTTON_H_

class GXSprite;
class CMODialog;
class CMODlgItem;
class CMOButtonReceiver;

//typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC0v)();
//typedef GXLRESULT (CGXToolbarDelegate::*LPBTNCMD)();

class GAMEENGINE_API CMOButton : public CMODlgItem
{
private:
  CMOButtonReceiver*   m_pReceiver;
public:
public:
  CMOButton();
  CMOButton(GXHWND hWnd);
  virtual ~CMOButton();
public:
  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;
  //GXBOOL  SetSpriteFile (GXLPCWSTR lpSpriteFile);
  //GXBOOL  SetSpriteObj  (GSprite* pSprite);
  //GXINT   AddButtons    (const GXTBBUTTON* pButtons, int nButtons);
  //GXVOID  AutoSize      ();
  //GXBOOL  SetBitmapSize (int cx, int cy);
  //GXBOOL  SetButtonSize (int cx, int cy);
  //GXBOOL  SetCommandProc(LPCOMMANDPROC pCmd);
#ifdef REFACTOR_GXFC
#else
  void          SetReceiver   (CMOButtonReceiver* pReceiver);

  virtual CGXReceiver*  GetReceiver   ();
  virtual void          SetReceiver   (CGXReceiver* pReceiver);
  virtual GXLRESULT     InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT     Invoke        (CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // 处理WM_COMMAND
#endif // #ifdef REFACTOR_GXFC
};

class GAMEENGINE_API CMOButtonReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify  (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC
  virtual GXVOID    OnBtnClicked  (CMOButton* pSender);
};
//typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_UI_TOOLBAR_H_