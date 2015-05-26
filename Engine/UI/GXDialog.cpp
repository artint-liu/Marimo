#include "GrapX.h"

#include "Engine.h"
#include "Engine/GXFCAgent.H"
#include "Engine/MOWndBase.H"
#include "Engine/MODialog.H"
#include "Engine/MODlgItemBase.H"
#include "Engine/WEStatic.H"
#include "Engine/WEListBox.H"
#include "Engine/WEListView.H"
#include "Engine/WETreeView.H"
#include "Engine/WEEdit.H"
#include "Engine/MOEdit.H"
#include "Engine/MOButton.h"
#include "Engine/MOStatic.h"
#include "Engine/MOList.h"
#include "Engine/MOToolbar.h"
#include "Engine/MOPropertySheet.h"
#include "Engine/MOPropertyList.h"

CMODialog::~CMODialog()
{
  // 这个里面不能做任何产生这个窗体消息的工作
  // 如果产生了窗口消息，会转到这个类的处理程序中，而在析构函数中
  // 虚函数表已经被修改回基类的，此时会导致无法调用继承类的虚函数
}

//CMODialog::CMODialog(GXHINSTANCE hInst, LPCWSTR idTemplate, GXHWND hParent, GXLPARAM lParam) : CMOWnd()
//{
//  // 注意: 使用这个函数将无法调用继承类中的 virtual 函数
//  CreateDlg(hInst, idTemplate, hParent, lParam);
//}

CMODialog::CMODialog(GXHINSTANCE hInstance, GXLPCWSTR idTemplate, CMOWnd* pParentWnd)
  : m_Parent(pParentWnd)
  , m_hInstance(hInstance)
  , m_idDlgTemplate(idTemplate)
{
}

GXHWND CMODialog::CreateDlg(GXHINSTANCE hInst, LPCWSTR idTemplate, CMOWnd& rParentWnd, GXLPARAM lParam)
{
  CGXDLGLOG sDlgLog = {lParam, this};
  return gxCreateDialogParamW(hInst, (LPCWSTR)(GXINT_PTR)idTemplate, rParentWnd.Handle(), (GXDLGPROC)DialogProc, (GXLPARAM)&sDlgLog);
}

GXHWND CMODialog::CreateDlg(GXLPARAM lParam)
{
  return CreateDlg(GetInstance(), m_idDlgTemplate, GetParent(), lParam);
}

int CMODialog::DlgBox(GXHINSTANCE hInst, LPCWSTR idTemplate, CMOWnd& rParentWnd, GXLPARAM lParam)
{
  ASSERT(m_hWnd == NULL);

  GXWPARAM aParam[2];
  aParam[0] = lParam;
  aParam[1] = (GXLPARAM)this;
  int nResult = gxDialogBoxParamW(hInst, (LPCWSTR)(GXINT_PTR)idTemplate, rParentWnd.Handle(), (GXDLGPROC)DialogProc, (GXLPARAM)&aParam);

  ASSERT(m_hWnd != NULL && ! gxIsWindow(m_hWnd));
  m_hWnd = NULL;
  return nResult;
}

int CMODialog::DlgBox(GXLPARAM lParam)
{
  return DlgBox(GetInstance(), m_idDlgTemplate, GetParent(), lParam);
}

GXLRESULT CMODialog::DialogProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  CMODialog *pDlgThis = (CMODialog*)GetCObjectFromHandle(hWnd);

  // 根据win7 windows dialog运行测试发现DlgProc不会收到
  // WM_NCCREATE和WM_CREATE消息, 模态和非模态都不会收到
  ASSERT(message != WM_NCCREATE && message != WM_CREATE);
  
  ASSERT(pDlgThis != NULL || message == GXWM_SETCOBJECT);

  //if(pDlgThis != NULL || message == WM_NCCREATE || message == WM_CREATE || message == WM_INITDIALOG)
  if(pDlgThis != NULL || message == GXWM_SETCOBJECT)
  {
    switch(message)
    {
    case WM_PAINT:
      {
        GXPAINTSTRUCT ps;
        gxBeginPaint(hWnd, &ps);
        LRESULT lr = pDlgThis->OnPaint(&ps);
        gxEndPaint(hWnd, &ps);
        return lr;
      }
    case WM_MOUSEMOVE:
      {
        GXPOINT ptMouse;
        ptMouse.x = (GXLONG)(short)GXLOWORD(lParam);
        ptMouse.y = (GXLONG)(short)GXHIWORD(lParam);
        pDlgThis->OnMouseMove(wParam, &ptMouse);
      }
      break;
    case WM_SETCURSOR:
      {
        GXHWND hWnd = (GXHWND)wParam;       // handle of window with cursor 
        int nHittest = LOWORD(lParam);  // hit-test code 
        GXDWORD wMouseMsg = HIWORD(lParam); // mouse-message identifier 
        return pDlgThis->OnSetCursor(hWnd, nHittest, wMouseMsg);
      }
      break;

    case GXWM_CHAR:
      pDlgThis->OnChar((GXWCHAR)wParam, lParam);
      break;

    case GXWM_KEYDOWN:
      pDlgThis->OnKeyDown((int)wParam, lParam);
      break;

    case GXWM_KEYUP:
      pDlgThis->OnKeyUp((int)wParam, lParam);
      break;

    case GXWM_NCDESTROY:
      {
#ifdef REFACTOR_GXFC
        //pDlgThis->m_hWnd = NULL;
#else
        GXHWND hWnd = pDlgThis->m_hWnd;
        if(hWnd != NULL) {
          gxEnumChildWindows(hWnd, (GXWNDENUMPROC)EnumChildProc_RemoveBindCls, (GXLPARAM)pDlgThis);
          gxSetWindowLongW(hWnd, GXGWL_COBJECT, NULL);
          pDlgThis->m_hWnd = NULL;
        }
#endif // #ifdef REFACTOR_GXFC

        //TRACE("CMODialog::DialogProc - WM_NCDESTROY\n");
      }
      break;

    case GXWM_SETCOBJECT: // Marimo 专有消息
      {
        CGXDLGLOG& sDlgLog = *(CGXDLGLOG*)lParam;
        pDlgThis = sDlgLog.pThis;
        pDlgThis->m_hWnd = hWnd;
        gxSetWindowLongW(hWnd, GXGWL_COBJECT, (GXLONG_PTR)(CMOWnd*)pDlgThis);
      }
      break;

    case GXWM_INITDIALOG:
      {
        CGXDLGLOG& sDlgLog = *(CGXDLGLOG*)lParam;
        //pDlgThis = sDlgLog.pThis;
        //pDlgThis->m_hWnd = hWnd;
        //gxSetWindowLongW(hWnd, GXGWL_COBJECT, (GXLONG_PTR)(CMOWnd*)pDlgThis);

        pDlgThis->OnInitDialog();
        return pDlgThis->OnInitDialog(NULL, sDlgLog.lParam);
      }

    case WM_SHOWWINDOW:
      return pDlgThis->OnShow((GXBOOL)wParam, (int)lParam);

    case WM_TIMER:
      return pDlgThis->OnTimer((UINT)wParam);

    case WM_SIZE:
      {
        GXSIZE size = {GXLOWORD(lParam), GXHIWORD(lParam)};
        return pDlgThis->OnSize(wParam, size);
      }
    case WM_NOTIFY:
      return pDlgThis->OnNotify((GXLPNMHDR)lParam);

    case WM_CLOSE:
      pDlgThis->OnClose();
      break;

    case GXWM_DISPLAYCHANGE:
      {
        GXSIZE sizeScreen;
        sizeScreen.cx = GXLOWORD(lParam);
        sizeScreen.cy = GXHIWORD(lParam);
        return (GXLRESULT)pDlgThis->OnDisplayChange((GXUINT)wParam, sizeScreen);
      }
    case GXWM_WINDOWPOSCHANGING:
      return pDlgThis->OnWindowPosChanging((GXWINDOWPOS*)lParam);
    case GXWM_COMMAND:
      return pDlgThis->OnCommand(GXHIWORD(wParam), GXLOWORD(wParam), (GXHWND)lParam);
    default:
      if(message >= GXWM_USER && message < GXWM_APP) {
        return pDlgThis->OnUserMessage(message, wParam, lParam);
      }
      else if(message >= GXWM_APP && message < GXWM_SETCURSOR) {
        return pDlgThis->OnAppMessage(message, wParam, lParam);
      }
      //case WM_DESTROY:
      //  return pDlgThis->OnDestory();
    }
    // TODO: 统一GXUI与Win32消息,去掉这个预编译
  }
//#ifdef _GXUI
//  return gxDefWindowProc(hWnd, message, wParam, lParam);
//#else
  return 0;
//#endif // _GXUI
}

#ifdef REFACTOR_GXFC
#else
template <class _TWndImplCls>
_TWndImplCls* CMODialog::WndObjFromHandle(GXHWND hWnd)  // TODO: 应该放在WndBase类里
{
  if(hWnd != NULL)
  {
    _TWndImplCls* pItem = (_TWndImplCls*)GetCObjectFromHandle(hWnd);
    ASSERT(pItem == NULL);  // 只能有一个 CGXWnd 副本!
    //ASSERT(pItem == NULL || (pItem != NULL && pItem->Handle() == hWnd));

    if(pItem == NULL)
    {
      pItem = new _TWndImplCls(hWnd);
      gxSetWindowLong(hWnd, GXGWL_COBJECT, (GXLONG_PTR)pItem);
    }
    return pItem;
  }
  return NULL;
}

CMODlgItem* CMODialog::CreateCObject(GXHWND hDlgItem)
{
  GXWCHAR szClassName[64];
  gxGetClassNameW(hDlgItem, szClassName, 64);

  if(GXSTRCMPI(szClassName, GXWC_LISTBOXW) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWEListBox>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXWC_LISTVIEWW) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWEListView>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXWC_TREEVIEWW) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWETreeView>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXWC_STATICW) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWEStatic>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXWC_EDITW) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWEEdit>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXWC_EDITW_1_3_30) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CWEEdit>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_EDIT) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOEdit>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_BUTTON) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOButton>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_LIST) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOListBox>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_TOOLBAR) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOToolbar>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_STATIC) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOStatic>(hDlgItem);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_PROPSHEET) == 0)
    return (GXLPDLGITEMBASE)WndObjFromHandle<CMOPropertySheet>(hDlgItem);
  return NULL;
}
#endif // #ifdef REFACTOR_GXFC

//
GXBOOL CMODialog::GetDlgItem(GXLPCWSTR idControl, CMOWnd& DlgItem)
{
  GXHWND hDlgItem = NULL;

  if(IS_IDENTIFY(idControl)) {
    hDlgItem = gxGetDlgItem(m_hWnd, (GXINT)idControl);
  }
  else {
    hDlgItem = GXGetDlgItemByName(m_hWnd, idControl);
  }

  CMOWnd* pCObject = (CMOWnd*)GetCObjectFromHandle(hDlgItem);

  // 只能有一个 CGXWnd 副本!
  if(pCObject == NULL) {
    new(&DlgItem) CMOWnd(hDlgItem);
    gxSetWindowLong(hDlgItem, GXGWL_COBJECT, (GXLONG_PTR)&DlgItem);
    return TRUE;
  }

  return FALSE;
}

GXBOOL CMODialog::EndDialog(GXINT_PTR lResult)
{
  return gxEndDialog(m_hWnd, (int)lResult);
}

GXBOOL CMODialog::GetClientRect(LPGXRECT lpRect)
{
  return gxGetClientRect(m_hWnd, lpRect);
}

GXBOOL CMODialog::CheckRadioButton(int nIDFirstButton, int nIDLastButton, int nIDCheckButton)
{
  return gxCheckRadioButton(m_hWnd, nIDFirstButton, nIDLastButton, nIDCheckButton);
}

GXHWND CMODialog::FindDialogByName(GXLPCWSTR szName)
{
  GXHWND hWnd = NULL;
  while(1)
  {
    hWnd = gxFindWindowEx(NULL, hWnd, GXWC_DIALOGEXW, NULL);
    if(hWnd == NULL) {
      break;
    }
    if(GXSTRCMP((GXLPCWSTR)gxSendMessage(hWnd, GXWM_GETIDNAMEW, NULL, NULL), szName) == 0) {
      return hWnd;
    }
  };
  return NULL;
}

#ifdef REFACTOR_GXFC
#else
GXBOOL CMODialog::EnumChildProc_RemoveBindCls(GXHWND hWnd, GXLPARAM lParam)
{
  CMODlgItem* lpItem = (CMODlgItem*)gxSetWindowLong(hWnd, GXGWL_COBJECT, NULL);
  //ASSERT(lpItem == NULL || (lpItem != NULL && lpItem->Handle() == hWnd));
  //if(lpItem && lpItem->GetClassNameCode() != GXMAKEFOURCC('X','D','L','G')) {
  //  lpItem->Destroy();
  //  SAFE_DELETE(lpItem);
  //}
  return TRUE;
}
#endif // #ifdef REFACTOR_GXFC

GX_BEGIN_COMMAND_MAP(CMODialog)
GX_END_COMMAND_MAP()

#ifdef REFACTOR_GXFC
#else
GXLPDLGITEMBASE CMODialog::CreateDelegateReceiver(GXHWND hControl, CGXReceiver* lpReceiver)
{
  GXLPDLGITEMBASE lpDlgItemBase = NULL;
  if(lpReceiver != NULL)
  {
    lpDlgItemBase = CreateCObject(hControl);

    if(lpDlgItemBase != NULL) {
      lpDlgItemBase->SetReceiver(lpReceiver);
    }
  }
  return lpDlgItemBase;
}
#endif // #ifdef REFACTOR_GXFC

GXLRESULT CMODialog::OnCommand(GXINT nNotifyCode, GXINT wID, GXHWND hwndCtrl)
{
  CGXReceiver* pReceiver = GetResponder(hwndCtrl);
#ifdef REFACTOR_GXFC
  if(pReceiver == RECEIVER_NONE) {
    return 0L;
  }
  else if(pReceiver == NULL) {
    pReceiver = TryCastReceiver(hwndCtrl);

    // 如果pReceiver为空，这个函数将会替换为RECEIVER_NONE
    SetResponder(hwndCtrl, pReceiver);
  }
  
  if(pReceiver != NULL) {
    return pReceiver->InvokeCommand(nNotifyCode, wID, hwndCtrl);
  }
#else
  LPGXCMDMAP lpCmdMap = (LPGXCMDMAP)GetCmdMap();
  CMODlgItem* pDlgItem = (CMODlgItem*)GetCObjectFromHandle(hwndCtrl);
  if(pDlgItem != NULL)
  {
    CGXReceiver* pReceiver = pDlgItem->GetReceiver();
    if(pReceiver == NULL) {
      pDlgItem->SetReceiver(static_cast<CGXReceiver*>(this));
    }
  }
  else
  {
    pDlgItem = CreateDelegateReceiver(hwndCtrl, static_cast<CGXReceiver*>(this));
    if(pDlgItem == NULL) {
      return -1;
    }
  }

  if(pDlgItem->InvokeReceiver(nNotifyCode, wID, hwndCtrl) < 0)
  {
    while(lpCmdMap->lpCmdProc != NULL)
    {
      if(lpCmdMap->wID == wID)
        return (this->*(lpCmdMap->lpCmdProc))(nNotifyCode, wID, hwndCtrl);
      lpCmdMap++;
    }
  }
#endif // #ifdef REFACTOR_GXFC
  return 0L;
}

GXLRESULT CMODialog::OnNotify(GXNMHDR* pnmhdr)
{
  if(pnmhdr->hwndFrom == NULL) {
    return -1;
  }
  
  CGXReceiver* pReceiver = GetResponder(pnmhdr->hwndFrom);
#ifdef REFACTOR_GXFC
  if(pReceiver == RECEIVER_NONE) {
    return 0L;
  }
  else if(pReceiver == NULL) {
    pReceiver = TryCastReceiver(pnmhdr->hwndFrom);

    // 如果pReceiver为空，这个函数将会替换为RECEIVER_NONE
    SetResponder(pnmhdr->hwndFrom, pReceiver);
  }

  if(pReceiver != NULL) {
    return pReceiver->InvokeNotify(pnmhdr);
  }
  return 0;
#else
  CMODlgItem* pDlgItemBase = (CMODlgItem*)
    GetCObjectFromHandle(pnmhdr->hwndFrom);

  if(pDlgItemBase != NULL) {
    return pDlgItemBase->InvokeReceiver(pnmhdr);
  }
  // TODO: 按照 OnCommand 方式实现
  //else {
  //  return pDlgItemBase->Invoke(this, pnmhdr);
  //}
  return 0L;
#endif // #ifdef REFACTOR_GXFC
}

GXLRESULT CMODialog::OnTimer(UINT uID)
{
  return CMOWndProcedure::OnTimer(uID);
}

GXLRESULT CMODialog::OnDisplayChange(GXUINT uColorDepth, GXSIZE sizeScreen)
{
  return gxDefWindowProcW(m_hWnd, WM_DISPLAYCHANGE, uColorDepth, GXMAKELPARAM(sizeScreen.cx, sizeScreen.cy));
}

GXLRESULT CMODialog::OnPaint(GXPAINTSTRUCT* pps)
{
  //return gxValidateRect(m_hWnd, NULL);
  return TRUE;
}

GXVOID CMODialog::OnMouseMove(GXWPARAM fwKeys, GXPOINT* ptMouse)
{
}

GXBOOL CMODialog::OnSetCursor(GXHWND hWnd, int nHittest, GXDWORD wMouseMsg)
{
  return FALSE;
}

GXDWORD CMODialog::GetClassNameCode() const
{
  return GXMAKEFOURCC('X','D','L','G');
}

GXLRESULT CMODialog::OnUserMessage( GXUINT message, GXWPARAM wParam, GXLPARAM lParam )
{
  return 0;
}

GXLRESULT CMODialog::OnAppMessage( GXUINT message, GXWPARAM wParam, GXLPARAM lParam )
{
  return 0;
}

#ifdef REFACTOR_GXFC
CGXReceiver* CMODialog::TryCastReceiver( GXHWND hDlgItem )
{
  GXWCHAR szClassName[64];
  gxGetClassNameW(hDlgItem, szClassName, 64);

  //if(GXSTRCMPI(szClassName, GXWC_LISTBOXW) == 0)
  //  CLBREAK // 没实现    return dynamic_cast<CWEListBoxReceiver*>(this);
  //else if(GXSTRCMPI(szClassName, GXWC_LISTVIEWW) == 0)
  //  CLBREAK // 没实现 return dynamic_cast<CWEListViewReceiver*>(this);
  //else 
    if(GXSTRCMPI(szClassName, GXWC_TREEVIEWW) == 0)
    return dynamic_cast<CWETreeViewReceiver*>(this);
  //else if(GXSTRCMPI(szClassName, GXWC_STATICW) == 0)
  //  CLBREAK // 没实现 return dynamic_cast<CWEStaticReceiver*>(this);
  //else if(GXSTRCMPI(szClassName, GXWC_EDITW) == 0)
  //  CLBREAK // 没实现 return dynamic_cast<CWEEditReceiver*>(this);
  //else if(GXSTRCMPI(szClassName, GXWC_EDITW_1_3_30) == 0)
  //  CLBREAK // 没实现 return dynamic_cast<CWEEditReceiver*>(this);
  //else if(GXSTRCMPI(szClassName, GXUICLASSNAME_EDIT) == 0)
  //  CLBREAK // 没实现 return dynamic_cast<CGXEditReceiver*>(this);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_BUTTON) == 0) {
    return dynamic_cast<CMOButtonReceiver*>(this); }
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_LIST) == 0) {
    return dynamic_cast<CMOListBoxReceiver*>(this); }
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_RICHLIST) == 0) {
    return dynamic_cast<CMOListBoxReceiver*>(this); }
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_TOOLBAR) == 0) {
    return dynamic_cast<CMOToolbarReceiver*>(this); }
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_STATIC) == 0) {
    CLBREAK; }  // 没实现 return dynamic_cast<CGXStaticReceiver*>(this);
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_PROPSHEET) == 0) {
    return dynamic_cast<CMOPropertySheetReceiver*>(this); }
  else if(GXSTRCMPI(szClassName, GXUICLASSNAME_PROPLIST) == 0) {
    return dynamic_cast<CMOPropertyListReceiver*>(this); }
  return NULL;
}
#endif // #ifdef REFACTOR_GXFC
