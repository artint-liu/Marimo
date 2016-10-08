#ifndef _GRAPX_WND_FOUNDATION_CLASS_BASE_H_
#define _GRAPX_WND_FOUNDATION_CLASS_BASE_H_

class GXGraphics;
class CGXReceiver;
struct GXWINDOWPOS;

#define RECEIVER_NONE   ((CGXReceiver*)-1)
#define REFACTOR_GXFC

class GAMEENGINE_API CMOWnd
{
  // CMOWnd要带有虚函数表，不然继承类CMOWndProcedure做类型转换时会遇到很多问题。
  // 比如CMOWndProcedure*为NULL的对象转换到CMOWnd*之后this是0x00000004（32位下）

protected:
  GXHWND      m_hWnd;

public:
          GXBOOL      Enable        (GXBOOL bEnable);
          GXBOOL      IsVisible     ();
          GXBOOL      Show          (bool bShow);
          GXBOOL      Show          (int nCmdShow);
          GXBOOL      Destroy       ();

          clStringW   GetTextW      ();
          clStringA   GetTextA      ();
          GXBOOL      SetTextW      (GXLPCWSTR szString);
          GXBOOL      SetTextA      (GXLPCSTR  szString);
          int         GetTextLengthW();

          GXLONG_PTR  GetLong       (int index);
          GXLONG_PTR  SetLong       (int index, GXLONG_PTR dwLong);
          BOOL        GetClientRect (LPGXRECT lpRect);
          UINT        SetTimer      (UINT uID, UINT uElapse);
          BOOL        KillTimer     (UINT uID);
          GXBOOL      SetWindowPos  (GXHWND hWndInsertAfter,int X,int Y,int cx,int cy,UINT uFlags);
          GXBOOL      GetWindowRect (GXRECT* rect);
          GXBOOL      UpdateWindow  ();
          GXBOOL      InvalidateRect(CONST GXRECT *lpRect, BOOL bErase);
          GXHWND      SetCapture    ();
          GXHWND      SetFocus      ();
          GXHWND      SetParent     (GXHWND hParent);
          GXHWND      SetParent     (CMOWnd* pParent);
          int         MessageBoxW   (GXLPCWSTR lpText, GXLPCWSTR lpCaption, GXUINT uType);
  inline  CMOWnd*     GetCObject    ();
  inline  GXHWND      Handle        ();
  inline  CMOWnd      GetParent     ();
  inline  GXHINSTANCE GetInstance   ();
  inline  GXGraphics* GetGraphics   ();

  clStringW   GetClassName        () const;
  GXDWORD     GetClassNameCode    () const;

  //template<class _CReceiverT>
  //inline _CReceiverT* TryCastReceiver     (CGXReceiver* pReceiver, _CReceiverT* pClsRetained = NULL)
  //{
  //  // 这里忘了为啥要这样写了啊啊啊啊啊!
  //  // 好像是因为Dialog类中得到的DlgItem实际上不储存Receiver,
  //  // 真实的Receiver存在于HWND对象的内部COBJ的Receiver中.
  //  return (pReceiver == pClsRetained)
  //    ? pClsRetained : dynamic_cast<_CReceiverT*>(pReceiver);
  //}

  inline static CMOWnd* GetCObjectFromHandle (GXHWND hWnd);
  inline static CGXReceiver* GetResponder (GXHWND hWnd);
  inline static CGXReceiver* SetResponder (GXHWND hWnd, CGXReceiver* pReceiver);

public:
  CMOWnd();
  CMOWnd(CMOWnd* pWnd);
  CMOWnd(GXHWND hWnd);
  CMOWnd(GXHINSTANCE hInstance, CMOWnd* ParentWnd);
  virtual ~CMOWnd();
};


class GAMEENGINE_API CMOWndProcedure : public CMOWnd
{
public:
  virtual GXLRESULT   OnSize              (GXDWORD fwSizeType, GXSIZE& size);
  virtual GXLRESULT   OnPaint             (GXPAINTSTRUCT* pps);
  virtual GXLRESULT   OnDisplayChange     (GXUINT uColorDepth,GXSIZE sizeScreen);
  virtual GXLRESULT   OnCommand           (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXBOOL      OnNCCreate          (GXLPCREATESTRUCT lpCreateParam);
  virtual GXLRESULT   OnCreate            (GXLPCREATESTRUCT lpCreateParam);
  virtual GXLRESULT   OnDestory           ();
  virtual GXBOOL      OnSetCursor         (GXHWND hWnd, int nHittest, GXDWORD wMouseMsg);
  virtual GXLRESULT   OnNotify            (NMHDR* pnmhdr);
  virtual GXLRESULT   OnTimer             (UINT uID);
  virtual GXLRESULT   OnShow              (GXBOOL bShow, int nStatus);
  virtual GXLRESULT   OnChar              (GXWCHAR chCharCode, GXLPARAM lKeyData);
  virtual GXLRESULT   OnKeyDown           (int nVirtKey, GXLPARAM lKeyData);
  virtual GXLRESULT   OnKeyUp             (int nVirtKey, GXLPARAM lKeyData);
  virtual GXVOID      OnMouseMove         (WPARAM fwKeys, POINT* ptMouse);
  virtual GXBOOL      OnSetCursor         (HWND hWnd, int nHittest, GXWORD wMouseMsg);
  virtual GXLRESULT   OnWindowPosChanging (GXWINDOWPOS* pWndPos);

  //template<class _CReceiverT>
  //inline _CReceiverT* TryCastReceiver     (CGXReceiver* pReceiver, _CReceiverT* pClsRetained = NULL)
  //{
  //  // 这里忘了为啥要这样写了啊啊啊啊啊!
  //  // 好像是因为Dialog类中得到的DlgItem实际上不储存Receiver,
  //  // 真实的Receiver存在于HWND对象的内部COBJ的Receiver中.
  //  return (pReceiver == pClsRetained)
  //    ? pClsRetained : dynamic_cast<_CReceiverT*>(pReceiver);
  //}

public:
  CMOWndProcedure();
  CMOWndProcedure(GXHWND hWnd);
  //CMOWndProcedure(GXHINSTANCE hInstance, CMOWnd* ParentWnd);
  virtual ~CMOWndProcedure();
};

//////////////////////////////////////////////////////////////////////////
//
// inline function
//

inline CMOWnd* CMOWnd::GetCObjectFromHandle(GXHWND hWnd)
{
  return (CMOWnd*)gxGetWindowLongW(hWnd, GXGWL_COBJECT);
}

inline CGXReceiver* CMOWnd::GetResponder(GXHWND hWnd)
{
  return (CGXReceiver*)gxGetWindowLongW(hWnd, GXGWL_RESPONDER);
}

inline CGXReceiver* CMOWnd::SetResponder(GXHWND hWnd, CGXReceiver* pReceiver)
{
  return (CGXReceiver*)gxSetWindowLongW(hWnd, GXGWL_RESPONDER, (GXLONG_PTR)(pReceiver == NULL ? RECEIVER_NONE : pReceiver));
}

inline CMOWnd* CMOWnd::GetCObject()
{
  return GetCObjectFromHandle(m_hWnd);
}

inline GXHWND CMOWnd::Handle()
{
  return this ? m_hWnd : NULL;
}

inline CMOWnd CMOWnd::GetParent()
{
  GXHWND hParent = gxGetWindow(m_hWnd, GXGW_PARENT);
  return CMOWnd(hParent);
}

inline GXHINSTANCE CMOWnd::GetInstance()
{
  return (GXHINSTANCE)gxGetWindowLong(m_hWnd, GXGWL_HINSTANCE);
}

inline GXGraphics* CMOWnd::GetGraphics()
{
  return GXGetGraphics(m_hWnd);
}

//////////////////////////////////////////////////////////////////////////
//
// Receiver
//
class GAMEENGINE_API CGXReceiver
{
protected:
  virtual ~CGXReceiver(){}
public:
#ifdef REFACTOR_GXFC
  virtual GXLRESULT InvokeCommand  (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl) = 0;
  virtual GXLRESULT InvokeNotify   (GXNMHDR* pnmhdr) = 0;
#endif // #ifdef REFACTOR_GXFC
};

//////////////////////////////////////////////////////////////////////////
typedef CMOWnd *LPGXWNDBASE, *PGXWNDBASE;
typedef CMOWnd *GXLPWNDBASE;

#endif // _GRAPX_WND_FOUNDATION_CLASS_BASE_H_