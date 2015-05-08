#ifndef _GRAPX_DIALOG_ITEM_BASE_H_
#define _GRAPX_DIALOG_ITEM_BASE_H_

#define RECEIVER_REFUSED ((CGXReceiver*)-1)

class CMOWnd;
class CMODialog;
#ifdef REFACTOR_GXFC
class GAMEENGINE_API CMODlgItem : public CMOWnd
{
  friend class CMODialog;
public:
  //CMOWnd& operator=(CGXDlgItemBase* pWnd);
  //CMOWnd& operator=(CGXDlgItemBase& Wnd);

  GXLPCWSTR GetIdentifierName() const;
  CMODlgItem(GXHWND hWnd);
  ~CMODlgItem();
};

#else

class GAMEENGINE_API CMODlgItem : public CMOWnd
{
  friend class CMODialog;
public:
  //CMOWnd& operator=(CGXDlgItemBase* pWnd);
  //CMOWnd& operator=(CGXDlgItemBase& Wnd);

  GXLPCWSTR GetIdentifierName() const;
  CMODlgItem(GXHWND hWnd);
  ~CMODlgItem();

  virtual CGXReceiver*  GetReceiver  (); // ������Ч�� Receiver ���� NULL ���� RECEIVER_REFUSED
  virtual void          SetReceiver  (CGXReceiver* pReceiver);

  virtual GXLRESULT     InvokeReceiver(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);  // ����WM_COMMAND
  virtual GXLRESULT     InvokeReceiver(GXNMHDR* pnmhdr);  // ���� WM_NOTIFY
};
#endif // #ifdef REFACTOR_GXFC

typedef CMODlgItem *GXLPDLGITEMBASE, *LPGXDLGITEMBASE, *PGXDLGITEMBASE;

#endif // _GRAPX_DIALOG_ITEM_BASE_H_