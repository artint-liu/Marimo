#ifndef _GRAP_X_UI_STATIC_H_
#define _GRAP_X_UI_STATIC_H_

class GXSprite;
class CMODialog;
class CMODlgItem;
class CGXToolbarDelegate;

//typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC0v)();
//typedef GXLRESULT (CGXToolbarDelegate::*LPBTNCMD)();

class GAMEENGINE_API CMOStatic : public CMODlgItem
{
private:
  CGXToolbarDelegate*   m_pDelegate;
  //LPCOMMANDPROC         m_pCmd;
public:
public:
  CMOStatic();
  CMOStatic(GXHWND hWnd);
  virtual ~CMOStatic();
public:
  //GXBOOL  SetSpriteFile (GXLPCWSTR lpSpriteFile);
  //GXBOOL  SetSpriteObj  (GSprite* pSprite);
  //GXINT   AddButtons    (const GXTBBUTTON* pButtons, int nButtons);
  //GXVOID  AutoSize      ();
  //GXBOOL  SetBitmapSize (int cx, int cy);
  //GXBOOL  SetButtonSize (int cx, int cy);
  //GXBOOL  SetCommandProc(LPCOMMANDPROC pCmd);
  //void    SetDelegate  (CGXToolbarDelegate* pDelegate);

  virtual GXSIZE_T    GetThisSizeOf       () const;
  virtual clStringW   GetClassName        () const;
  virtual GXDWORD     GetClassNameCode    () const;
  //CMOWnd& operator=(CMOWnd* pWnd);
  //CMOWnd& operator=(CMOWnd& Wnd);

  virtual GXLRESULT   Invoke(CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // 处理WM_COMMAND
};

class GAMEENGINE_API CGXStaticSprite : public CMOStatic
{
public:
  GXBOOL SetSpriteByFilenameW   (GXLPCWSTR szSpriteFile);
  GXBOOL SetSprite              (GXSprite* pSprite);
  GXBOOL SetModuleByNameW       (GXLPCWSTR szModuleName);
  GXBOOL SetModuleByIndex       (GXUINT nModuleIndex);
};

//class CGXToolbarDelegate : public CGXDelegate
//{
//public:
//  virtual GXLRESULT OnBtnClicked(CGXToolbar* pSender, DWORD_PTR idButton);
//};
typedef CMOStatic *LPGXSTATIC;
#endif // _GRAP_X_UI_TOOLBAR_H_