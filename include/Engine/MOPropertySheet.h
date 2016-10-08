#ifndef _GRAP_X_UI_PROPERTY_SHEET_H_
#define _GRAP_X_UI_PROPERTY_SHEET_H_

class GXSprite;
class CMODialog;
class CMODlgItem;
class CMOPropertySheetReceiver;

//typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC0v)();
//typedef GXLRESULT (CGXToolbarDelegate::*LPBTNCMD)();

class GAMEENGINE_API CMOPropertySheet : public CMODlgItem
{
private:
  CMOPropertySheetReceiver*   m_pReceiver;
public:
public:
  CMOPropertySheet();
  CMOPropertySheet(GXHWND hWnd);
  virtual ~CMOPropertySheet();
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
  void          SetReceiver   (CMOPropertySheetReceiver* pReceiver);

  virtual CGXReceiver*  GetReceiver   ();
  virtual void          SetReceiver   (CGXReceiver* pReceiver);
  virtual GXLRESULT     InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT     InvokeReceiver(GXNMHDR* pnmhdr);

  virtual GXLRESULT     Invoke        (CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // 处理WM_COMMAND
  virtual GXLRESULT     Invoke        (CGXReceiver* pReceiver, GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  GXBOOL SetDataLayout  (const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink);
  GXBOOL UploadData     (const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast); // [nIDFirst, nIDLast]
  GXBOOL DownloadData   (const GXLPVOID* pBasePtrList, const PROPSHEET_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast); // [nIDFirst, nIDLast]
};

class GAMEENGINE_API CMOPropertySheetReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify  (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC
  virtual GXLRESULT OnValueChanging(CMOPropertySheet* pSender, GXLPCWSTR szName, const NM_PROPSHEETW* pItem);
  virtual GXLRESULT OnValueChanged(CMOPropertySheet* pSender, GXLPCWSTR szName, const NM_PROPSHEETW* pItem);
};
//typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_UI_PROPERTY_SHEET_H_