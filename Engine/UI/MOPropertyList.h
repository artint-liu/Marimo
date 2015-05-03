#ifndef _GRAP_X_UI_PROPERTY_LIST_H_
#define _GRAP_X_UI_PROPERTY_LIST_H_

class GXSprite;
class CMODialog;
class CMODlgItem;
class CMOPropertyListReceiver;

//typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC0v)();
//typedef GXLRESULT (CGXToolbarDelegate::*LPBTNCMD)();

class GAMEENGINE_API CMOPropertyList : public CMODlgItem
{
private:
  CMOPropertyListReceiver*   m_pReceiver;
public:
public:
  CMOPropertyList();
  CMOPropertyList(GXHWND hWnd);
  virtual ~CMOPropertyList();
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
  void          SetReceiver   (CMOPropertyListReceiver* pReceiver);

  virtual CGXReceiver*  GetReceiver   ();
  virtual void          SetReceiver   (CGXReceiver* pReceiver);
  virtual GXLRESULT     InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT     InvokeReceiver(GXNMHDR* pnmhdr);

  virtual GXLRESULT     Invoke        (CGXReceiver* pReceiver, GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // ¥¶¿ÌWM_COMMAND
  virtual GXLRESULT     Invoke        (CGXReceiver* pReceiver, GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC

  GXBOOL SetDataLayout  (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink);
  GXBOOL UploadData     (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast); // [nIDFirst, nIDLast]
  GXBOOL DownloadData   (const GXLPVOID* pBasePtrList, const PROPLIST_DATALINK* pDataLink, GXINT nIDFirst, GXINT nIDLast); // [nIDFirst, nIDLast]
};

class GAMEENGINE_API CMOPropertyListReceiver : public CGXReceiver
{
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXLRESULT InvokeNotify  (GXNMHDR* pnmhdr);
#endif // #ifdef REFACTOR_GXFC
  virtual GXLRESULT OnValueChanging(CMOPropertyList* pSender, GXLPCWSTR szName, const NM_PROPLISTW* pItem);
  virtual GXLRESULT OnValueChanged(CMOPropertyList* pSender, GXLPCWSTR szName, const NM_PROPLISTW* pItem);
};
//typedef CWEListBox *LPWELISTBOX, *PWELISTBOX;
#endif // _GRAP_X_UI_PROPERTY_LIST_H_