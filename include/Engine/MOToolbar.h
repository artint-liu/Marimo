#ifndef _GRAP_X_UI_TOOLBAR_H_
#define _GRAP_X_UI_TOOLBAR_H_

class GXSprite;
class CMODialog;
class CMODlgItem;
class CMOToolbarReceiver;

//typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC0v)();
//typedef GXLRESULT (CGXToolbarDelegate::*LPBTNCMD)();

class GAMEENGINE_API CMOToolbar : public CMODlgItem
{
private:
  CMOToolbarReceiver*   m_pReceiver;
  //LPCOMMANDPROC         m_pCmd;
public:
public:
  CMOToolbar();
  CMOToolbar(GXHWND hWnd);
  virtual ~CMOToolbar();
public:
  GXBOOL    SetSpriteFile       (GXLPCWSTR lpSpriteFile);
  GXBOOL    SetSpriteObj        (GXSprite* pSprite);
  GXINT     AddButtons          (const GXTBBUTTON* pButtons, int nButtons);
  GXVOID    AutoSize            ();
  GXBOOL    IsButtonChecked     (GXINT_PTR idCommand);
  GXBOOL    CheckButton         (GXINT_PTR idCommand, GXBOOL fCheck);
  GXBOOL    SetButtonBitmap     (GXINT_PTR idCommand, GXINT idBitmap);
  GXINT     GetButtonBitmap     (GXINT_PTR idCommand);
  GXBOOL    SetButtonBitmapByIndex (GXINT_PTR idCommand, GXINT idBitmap);
  GXINT     GetButtonBitmapByIndex (GXINT_PTR idCommand);
  GXBOOL    SetBitmapSize       (int cx, int cy);
  GXBOOL    SetButtonSize       (int cx, int cy);
  GXBOOL    HideButton          (GXINT_PTR idCommand, GXBOOL bHide);
  GXBOOL    SetCommandProc      (LPCOMMANDPROC pCmd);
  GXLPCWSTR GetIdentifierName   (GXINT nButtonID) const;  // 这个是扩展

#ifdef REFACTOR_GXFC
#else
  void      SetReceiver         (CMOToolbarReceiver* pReceiver);
  virtual CGXReceiver*  GetReceiver   ();
  virtual void        SetReceiver         (CGXReceiver* pReceiver);
#endif // #ifdef REFACTOR_GXFC

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;
  //CMOWnd& operator=(CMOWnd* pWnd);
  //CMOWnd& operator=(CMOWnd& Wnd);

#ifdef REFACTOR_GXFC
#else
  virtual GXLRESULT   InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT   Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // 处理WM_COMMAND
#endif // #ifdef REFACTOR_GXFC

};

class GAMEENGINE_API CMOToolbarReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify  (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  virtual GXLRESULT OnBtnClicked(CMOToolbar* pSender, GXLPCWSTR idButton);
};
typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_UI_TOOLBAR_H_