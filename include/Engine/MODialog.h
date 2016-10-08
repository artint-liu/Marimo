#ifndef _GRAP_X_WND_CLASS_H_
#define _GRAP_X_WND_CLASS_H_

typedef GXLRESULT (CMOWnd::*LPCOMMANDPROC)(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
typedef struct __tagGXCMDMAP
{
  GXINT      wID;
  LPCOMMANDPROC  lpCmdProc;
}GXCMDMAP, *LPGXCMDMAP;

#define GX_DECLARE_COMMAND_MAP()            \
protected:                                  \
  static const LPGXCMDMAP GetThisCmdMap();  \
  virtual const LPGXCMDMAP GetCmdMap();     \

#define GX_BEGIN_COMMAND_MAP(theClass)      \
  const LPGXCMDMAP theClass::GetCmdMap() {  \
  return theClass::GetThisCmdMap();         \
}                                           \
  const LPGXCMDMAP theClass::GetThisCmdMap()\
{                                           \
  typedef theClass TheClass;                \
  static const GXCMDMAP CommandMap[] = {    \

#define GX_END_COMMAND_MAP()                \
{0, NULL}};                                 \
  return (LPGXCMDMAP)CommandMap;            \
}

#define GX_ON_COMMAND(ID, CMDPROC)            \
{(GXINT)ID,(LPCOMMANDPROC)&TheClass::CMDPROC},

class CMOWnd;
class CWEListBox;
class CMODlgItem;
class GAMEENGINE_API CMODialog : public CMOWndProcedure//, public CGXReceiver
{
  GX_DECLARE_COMMAND_MAP()
private:
  struct CGXDLGLOG
  {
    GXLPARAM   lParam;
    CMODialog* pThis;
  };

  GXLPCWSTR   m_idDlgTemplate;
  CMOWnd      m_Parent;
  GXHINSTANCE m_hInstance;
private:
  static GXLRESULT  GXCALLBACK DialogProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
#ifdef REFACTOR_GXFC
#else
  static GXBOOL     GXCALLBACK EnumChildProc_RemoveBindCls(GXHWND hWnd, GXLPARAM lParam);
#endif // #ifdef REFACTOR_GXFC


  GXHWND            CreateDlg       (GXHINSTANCE hInst, LPCWSTR idTemplate, CMOWnd& pParentWnd, GXLPARAM lParam);
  int               DlgBox          (GXHINSTANCE hInst, LPCWSTR idTemplate, CMOWnd& pParentWnd, GXLPARAM lParam);
protected:
#ifdef REFACTOR_GXFC
#else
  CMODlgItem*   CreateDelegateReceiver(GXHWND hControl, CGXReceiver* lpReceiver);
  virtual CMODlgItem* CreateCObject (GXHWND hDlgItem);
#endif // #ifdef REFACTOR_GXFC
public:
  GXHWND            CreateDlg       (GXLPARAM lParam = NULL);
  int               DlgBox          (GXLPARAM lParam = NULL);

  //template <class _TDlgItemCls>
  //_TDlgItemCls      GetDlgItemT     (GXINT idCtrl);
  //CGXDlgItemBase    GetDlgItem      (GXINT idCtrl);
  //GXBOOL            GetDlgItem__      (GXLPCWSTR idCtrl, CGXDlgItemBase& DlgItem);
  //GXBOOL            GetDlgItem__      (int idCtrl, CGXDlgItemBase& DlgItem);
#ifdef REFACTOR_GXFC
#else
  template <class _TWndImplCls>
  _TWndImplCls*     WndObjFromHandle(GXHWND hWnd);
#endif // #ifdef REFACTOR_GXFC
  GXBOOL            GetDlgItem      (GXLPCWSTR idControl, CMOWnd& DlgItem);
  GXBOOL            EndDialog       (GXINT_PTR lResult);
  GXBOOL            GetClientRect   (LPGXRECT);
  GXBOOL            CheckRadioButton(int nIDFirstButton, int nIDLastButton, int nIDCheckButton);

  static  GXHWND    FindDialogByName(GXLPCWSTR szName);

  inline  LPCWSTR   GetTemplate       ();
  virtual GXVOID    OnInitDialog      (){};
  virtual GXLRESULT OnInitDialog      (GXHWND hWnd, GXLPARAM lParam){return 0L;};
  virtual GXVOID    OnClose           (){};
  virtual GXLRESULT OnPaint           (GXPAINTSTRUCT* pps);
  virtual GXLRESULT OnDisplayChange   (GXUINT uColorDepth, GXSIZE sizeScreen);
  virtual GXLRESULT OnCommand         (GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl);
  virtual GXBOOL    OnNCCreate        (GXLPCREATESTRUCT lpCreateParam) { return TRUE; };
  virtual GXLRESULT OnCreate          (GXLPCREATESTRUCT lpCreateParam) { return 0; };
  virtual GXLRESULT OnDestory         () { return 0L; }
  virtual GXLRESULT OnNotify          (GXNMHDR* pnmhdr);
  virtual GXLRESULT OnTimer           (UINT uID);
  virtual GXVOID    OnMouseMove       (GXWPARAM fwKeys, GXPOINT* ptMouse);
  virtual GXBOOL    OnSetCursor       (GXHWND hWnd, int nHittest, GXDWORD wMouseMsg);
  virtual GXLRESULT OnUserMessage     (GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  virtual GXLRESULT OnAppMessage      (GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  virtual GXDWORD   GetClassNameCode  () const;
#ifdef REFACTOR_GXFC
  virtual CGXReceiver* TryCastReceiver (GXHWND hDlgItem); // 尝试映射成为DlgItem的接收器
#endif // #ifdef REFACTOR_GXFC
  //virtual LRESULT 
public:
  CMODialog(){}
  //CMODialog(GXHINSTANCE hInst, LPCWSTR idTemplate, GXHWND hParent, GXLPARAM lParam);
  CMODialog(GXHINSTANCE hInstance, GXLPCWSTR idTemplate, CMOWnd* pParentWnd);
  virtual ~CMODialog();
};
//////////////////////////////////////////////////////////////////////////
//
// inline function
//
inline LPCWSTR CMODialog::GetTemplate()
{
  return m_idDlgTemplate;
}

typedef CMODialog *LPGXDIALOG, *PGXDIALOG;

#endif //  _GRAP_X_WND_CLASS_H_