#ifndef _DEV_DISABLE_UI_CODE
// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>
#include <GrapX/GXGraphics.H>
#include <GrapX/GXCanvas.H>

// ˽��ͷ�ļ�
#include <User/WindowsSurface.h>
#include <User/DesktopWindowsMgr.h>
#include "GrapX/GXUser.H"
#include "GrapX/GXKernel.H"
#include <User/GXWindow.h>
#include <GrapX/gxDevice.H>

extern GXSTATION g_gxStation;
GXLRESULT DEFWNDPROC_SetText      (GXHWND hWnd, GXLPCWSTR pszName);
GXUINT  MENU_GetMenuBarHeightFast (GXHWND hWnd, GXUINT menubarWidth,  GXINT orgX, GXINT orgY );
GXVOID  _gxDestroyMarkedWindow    (GXHWND);
GXVOID  _gxDestroyWindow          (GXHWND);
GXINT   gxIntDestroyChildWnd      (GXLPWND lpWnd);
void    IntRemoveAllProp          (GXHWND hWnd);
//////////////////////////////////////////////////////////////////////////
GXBOOL IntSendCreateMessage(GXHWND hWnd, GXHWND hParent, GXLPCWSTR szClassName, GXDWORD dwExStyle, GXDWORD dwStyle,
  int x, int y, int width, int height, GXHMENU hMenu, GXLPVOID lParam)
{
  GXCREATESTRUCT sCreateStruct = {0};
  GXLPWND lpWnd = GXWND_PTR(hWnd);

  sCreateStruct.lpszName       = lpWnd->m_pText;
  sCreateStruct.hMenu          = hMenu;
  sCreateStruct.hwndParent     = hParent;
  sCreateStruct.dwExStyle      = dwExStyle;
  sCreateStruct.style          = dwStyle;
  sCreateStruct.x              = x;
  sCreateStruct.y              = y;
  sCreateStruct.cx             = width;
  sCreateStruct.cy             = height;
  sCreateStruct.lpCreateParams = lParam;
  sCreateStruct.lpszClass      = szClassName;
  
  if(gxSendMessageW(hWnd, GXWM_NCCREATE, NULL, (GXLPARAM)&sCreateStruct) == FALSE)
    return FALSE;

  if((gxSendMessageW(hWnd, GXWM_CREATE, NULL, (GXLPARAM)&sCreateStruct) & 0x80000000) != NULL)
    return FALSE;

  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// û�в��������ж�̬���������ٴ���
GXBOOL GXDLLAPI gxDestroyWindow(
               GXHWND hWnd   // handle to window to destroy  
             )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL) {
    return FALSE;
  }

  if(TEST_FLAG(lpWnd->m_uState, WIS_DESTROYTHISWND)) {
    return TRUE;
  }

  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);

  // TODO: Active ��������ʱ WndsSurface �Ĵ���, Ҫ��������Active����
  // TODO: �� Active ��������ʱ WndsSurface �Ĵ���, ����DWM ��״

  gxIntDestroyChildWnd(lpWnd);

  GXLPWND lpParent = lpWnd->m_pParent;
  while(lpParent != NULL)
  {
    lpParent->m_uState |= WIS_HASDESTROYWND;
    lpParent = lpParent->m_pParent;
  }

  // �����ػ�����
  GXWindowsSurface* pSurface = lpWnd->GetTopSurface();
  if(pSurface != NULL)
  {
    GRegion* pRegion = NULL;
    lpWnd->GetSystemRegion(GSR_CLIPSIBLINGS|GSR_WINDOW, &pRegion);
    pSurface->InvalidateRegion(pRegion);
    pSurface->GenerateWindowsRgn(TRUE);
    SAFE_RELEASE(pRegion);
  }

  return TRUE;
}

GXINT gxIntDestroyChildWnd(GXLPWND lpWnd)
{
  ASSERT(lpWnd != NULL);
  GXMSG msg;
  GXHWND hWnd = GXWND_HANDLE(lpWnd);
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);

  // flush message queue
  while(gxPeekMessageW(&msg, hWnd, 0, 0, GXPM_REMOVE)) {
    gxDispatchMessageW(&msg);
  }

  // ��������������޸����ٱ�־���ɶ�����ˣ�
  //// Ҫ�����������ٱ�־
  //SET_FLAG(lpWnd->m_uState, WIS_DESTROYTHISWND | WIS_HASDESTROYWND);

  lpStation->CleanupRecord(hWnd);

  gxSendMessage(hWnd, GXWM_DESTROY, 0, 0);

  GXLPWND lpChild = lpWnd->m_pFirstChild;
  while(lpChild != NULL)
  {
    if(TEST_FLAG(lpChild->m_uState, WIS_DESTROYTHISWND) == FALSE) {
      gxIntDestroyChildWnd(lpChild);
    }
    lpChild = lpChild->m_pNextWnd;
  }

  gxSendMessage(hWnd, GXWM_NCDESTROY, 0, 0);

  // IsWindow �л��� WIS_DESTROYTHISWND �����־
  // ���յ� GXWM_DESTROY �� GXWM_NCDESTROY ʱ IsWindow ��Ȼ�� TRUE
  SET_FLAG(lpWnd->m_uState, WIS_DESTROYTHISWND | WIS_HASDESTROYWND);
  return 1;
}

GXVOID _gxDestroyMarkedWindow(GXHWND hWnd)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  lpWnd->m_uState &= (~WIS_HASDESTROYWND);
  if((lpWnd->m_uState & WIS_DESTROYTHISWND) != 0)
  {
    _gxDestroyWindow(hWnd);
    return;
  }
  GXLPWND pChild = lpWnd->m_pFirstChild;
  GXLPWND pNext;
  while(pChild != NULL)
  {
    pNext = pChild->m_pNextWnd;
    if((pChild->m_uState & WIS_HASDESTROYWND) != 0)
      _gxDestroyMarkedWindow(GXWND_HANDLE(pChild));
    pChild = pNext;
  }
}

GXVOID _gxDestroyWindow(GXHWND hWnd)
{
  GXLPWND pParent, lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL)
    return;

  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);
  LPGXWND lpDesktop = lpStation->lpDesktopWnd;

  //gxSendMessage(hWnd, GXWM_DESTROY, 0, 0);
  while(lpWnd->m_pFirstChild != NULL)
  {
    _gxDestroyWindow(GXWND_HANDLE(lpWnd->m_pFirstChild));
  }

  ////////////////////////////////////////////////////////////////////////////
  ////
  //// ���� Station �ļ�¼
  ////

  //// ����漰��������߲��񴰿������
  //if(lpStation->m_pMouseFocus == lpWnd)  {
  //  // ����ʧȥ��꽹����Ϣ
  //  gxSendMessageW(lpStation->m_pMouseFocus, GXWM_NCMOUSELEAVE, NULL, NULL);
  //  gxSendMessageW(lpStation->m_pMouseFocus, GXWM_MOUSELEAVE, NULL, NULL);
  //  lpStation->m_pMouseFocus = NULL;
  //}
  //if(lpStation->m_pKeyboardFocus == lpWnd)  {
  //  // ����ʧȥ���̽�����Ϣ
  //  gxSetFocus(NULL);
  //}
  //if(lpStation->m_pCapture == lpWnd)  {
  //  // ����ʧȥ���ڲ�����Ϣ
  //  gxReleaseCapture();
  //}

  //// ����Ƕ��㴰��, ��Ҫ��������б���ļ�¼
  //if(gxIsTopLevelWindow(hWnd) == TRUE)
  //{
  //  GXLPWND_ARRAY& aWnds = lpStation->m_aActiveWnds;

  //  // ����Ǽ����,���ȸ�������,ʹ��Ǽ���
  //  if(lpWnd->GetActive() == lpWnd)
  //  {
  //    for(GXLPWND_ARRAY::reverse_iterator it = aWnds.rbegin();
  //      it != aWnds.rend(); ++it)
  //    {
  //      const GXULONG uStyle = (*it)->m_uStyle;
  //      if((uStyle & (GXWS_DISABLED | GXWS_VISIBLE)) == GXWS_VISIBLE &&
  //        (*it) != lpWnd)
  //      {
  //        (*it)->SetActive();
  //        break;
  //      }
  //    }
  //  }

  //  for(GXLPWND_ARRAY::iterator it = aWnds.begin();
  //    it != aWnds.end(); ++it)
  //  {
  //    if((*it) == lpWnd)
  //    {
  //      aWnds.erase(it);
  //      break;
  //    }
  //  }
  //}
  //lpStation->CleanupRecord(hWnd);

  //gxSendMessage(hWnd, GXWM_NCDESTROY, 0, 0);

  if(lpWnd->m_pPrevWnd == NULL)
  {
    pParent = lpWnd->m_pParent;
    if(pParent == NULL)
      pParent = lpDesktop;
    pParent->m_pFirstChild = lpWnd->m_pNextWnd;
    if(lpWnd->m_pNextWnd != NULL)
    {
      lpWnd->m_pNextWnd->m_pPrevWnd = NULL;
    }
  }
  else
  {
    lpWnd->m_pPrevWnd->m_pNextWnd = lpWnd->m_pNextWnd;
    if(lpWnd->m_pNextWnd != NULL)
    {
      lpWnd->m_pNextWnd->m_pPrevWnd = lpWnd->m_pPrevWnd;
    }
  }
  if(TEST_FLAG(lpWnd->m_uStyle, GXWS_CHILD))
  {

  }
  else if(lpWnd->m_pMenu)
  {
    //if(IS_IDENTIFY(lpWnd->m_pMenu) == FALSE)
    gxDestroyMenu(lpWnd->m_pMenu);
  }
  lpWnd->m_lpClsAtom->nRefCount--;

#if defined(_DEBUG) && defined(WIS_HASBEENDEL)
  CHECK_LPWND_VAILD(lpWnd);
  SET_FLAG(lpWnd->m_uState, WIS_HASBEENDEL);
  lpWnd->m_pNextWnd = NULL;
  lpWnd->m_pPrevWnd = NULL;
  SET_FLAG(lpWnd->m_uStyle, GXWS_DISABLED);
  RESET_FLAG(lpWnd->m_uStyle, GXWS_VISIBLE);
#else
  SAFE_DELETE(lpWnd);
#endif // defined(_DEBUG) && defined(WIS_HASBEENDEL)
}

//////////////////////////////////////////////////////////////////////////
GXVOID GXDestroyRootFrame()
{
  GXLPSTATION lpStation = IntGetStationPtr();
  ASSERT(lpStation->dwUIThreadId == 0 || lpStation->dwUIThreadId == gxGetCurrentThreadId());

  LPGXWND lpDesktop = lpStation->lpDesktopWnd;
  if( ! lpDesktop) {
    return;
  }

  LPGXWND lpWnd = lpDesktop->m_pFirstChild;

  // ��Ǵ������ٱ�־,Ȼ��������,����Ϊ���ô�����ȷ�յ����ִ�����Ϣ
  while(lpWnd != NULL)
  {
    gxDestroyWindow(GXWND_HANDLE(lpWnd));
    lpWnd = lpWnd->m_pNextWnd;
  }
  _gxDestroyMarkedWindow(GXWND_HANDLE(lpDesktop));

  // TODO: ע���������ȼ�(UngisterHotKey)
}

//////////////////////////////////////////////////////////////////////////
extern "C" GXHRESULT GXDLLAPI GXRenderRootFrame()
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  lpStation->Enter();
  LPGXWND lpDesktop = lpStation->lpDesktopWnd;

  if((lpDesktop->m_uState & WIS_HASDESTROYWND) != 0)
    _gxDestroyMarkedWindow(GXWND_HANDLE(lpDesktop));

  GXCanvas* pCanvas = lpStation->pGraphics->LockCanvas(NULL, NULL, NULL);
  pCanvas->SetCompositingMode(CM_SourceOver);

  if(pCanvas != NULL)
  {
    if(lpStation->m_dwFlags & GXST_DRAWDEBUGMSG)
      GXDrawDebugMsg(GXSTATION_HANDLE(lpStation), pCanvas);

    lpStation->m_pDesktopWindowsMgr->SendPaintMessage();
    lpStation->m_pDesktopWindowsMgr->Render(pCanvas);
    pCanvas->Release();
    pCanvas = NULL;
  }
  
  lpStation->m_uFrameCount++;
  lpStation->Leave();
  return GX_OK;
}
//////////////////////////////////////////////////////////////////////////
GXHWND GXDLLAPI gxCreateWindowExW(GXDWORD dwExStyle,GXLPCWSTR lpClassName,GXLPCWSTR lpWindowName,GXDWORD dwStyle,
                 GXINT x,GXINT y,GXINT nWidth,GXINT nHeight,GXHWND hWndParent,GXHMENU hMenu,GXHINSTANCE hInstance,GXLPVOID lpParam)
{
  GXWnd*pNewWnd;
  GXPOINT ptTopLeft;
  GXWNDCLASSEX WndClassEx;

  if(TEST_FLAG(dwStyle, GXWS_CHILD) && hWndParent == NULL) {
    // ����ж��Ǹռӵģ���ֹ֮ǰд������������Ӹ��ж���������ǰ�Ĵ���д�������Ų�
    CLBREAK;
    return NULL;
  }

  GXDWORD lpClsAtom = gxGetClassInfoExW(NULL, lpClassName, &WndClassEx);
  if(lpClsAtom == NULL)
  {
    ASSERT(FALSE);
    return NULL;
  }


  // ����32λϵͳ, ����Ĵ���ռ����cbWndExtra
  // ����64λϵͳ����Ĵ���ռ���cbWndExtra��2��, ������֤����ռ�����Ա���8�ֽڵ�ָ��
  const GXSIZE_T cbWndClassSize = sizeof(GXWnd) + WndClassEx.cbWndExtra * (sizeof(void*) / sizeof(GXDWORD));

  // ����������Ͷ�������ݴ���ռ�
  pNewWnd = new(new GXBYTE[cbWndClassSize]) GXWnd;
  if( ! pNewWnd) {
    CLOG_ERROR(__FUNCTION__": Out of memory.\n");
    return NULL;
  }

  GXHWND hNewWnd = GXWND_HANDLE(pNewWnd);
  pNewWnd->m_hSelf = hNewWnd;

  GXMEMSET((GXLPBYTE)pNewWnd + sizeof(GXWnd), 0, cbWndClassSize - sizeof(GXWnd));
  pNewWnd->m_lpWndProc = WndClassEx.lpfnWndProc;
  pNewWnd->m_uStyle    = dwStyle;
  pNewWnd->m_uExStyle  = dwExStyle;
  pNewWnd->m_uState    = NULL;
  pNewWnd->m_lpClsAtom = (GXLPWNDCLSATOM)(GXDWORD_PTR)lpClsAtom;
  pNewWnd->m_lpClsAtom->nRefCount++;

  GXLPSTATION lpStation = GXLPWND_STATION_PTR(pNewWnd);
  GXDWORD dwThreadId = gxGetCurrentThreadId();
  if(lpStation->dwUIThreadId == NULL) {
    CLOG_WARNING(__FUNCTION__": User doesn't set UI thread Id, set to default.\n"
      "Call GXUIMakeCurrent() first in UI message processing thread.\n");
    lpStation->dwUIThreadId = dwThreadId;
  }
  else if(lpStation->dwUIThreadId != dwThreadId)
  {
    CLOG_ERROR(__FUNCTION__": GXUI can not create window out of UI thread.\n");
    CLBREAK;
    delete pNewWnd;
    return NULL;
  }
  lpStation->Enter();

  // ���ø�����
  ptTopLeft.x = x;
  ptTopLeft.y = y;
  pNewWnd->SetParent(hWndParent);
  if(hWndParent != NULL)
  {
    GXWND_PTR(hWndParent)->ClientToScreen(&ptTopLeft, 1);
  }

  // ���� Wnd �ߴ�
  pNewWnd->rectWindow.left    = ptTopLeft.x;
  pNewWnd->rectWindow.top     = ptTopLeft.y;
  pNewWnd->rectWindow.right   = ptTopLeft.x + nWidth;
  pNewWnd->rectWindow.bottom  = ptTopLeft.y + nHeight;

  if(lpWindowName != NULL) {
    // Win32 �ڴ��ڴ���ʱ�ǲ�����WM_SETTEXT��Ϣ��, ����ֱ������
    DEFWNDPROC_SetText(hNewWnd, lpWindowName);
    //gxSendMessage(hNewWnd, GXWM_SETTEXT, NULL, (GXLPARAM)lpWindowName);
  }

  if(hWndParent == NULL)
  {
    //ASSERT(IS_IDENTIFY(hMenu));
    //TODO: �������ڴ�й¶����������WineMenu������gxHeapAlloc�����ڴ棬����޷����
    //pNewWnd->m_pMenu = gxLoadMenuW(hInstance, (GXLPCWSTR)hMenu);
    pNewWnd->m_pMenu = hMenu;
  }
  else
  {
    //ASSERT(hMenu != NULL && dwStyle & WS_CHILD);
    pNewWnd->m_pMenu = hMenu;
  }
  // FIXME: WS_CHILDģʽ��, �ַ�����ʽ��Control IdӦ�õ��������ڴ汣��, ��Ӧ��ֱ������ָ��.

  if(IntSendCreateMessage(hNewWnd, hWndParent, lpClassName, dwExStyle, dwStyle, x, y, nWidth, nHeight, hMenu, lpParam) == FALSE)
  {
    ASSERT(0);  // �����ؼ���ʼ��ʧ��
    gxDestroyWindow(hNewWnd);
    lpStation->Leave();
    return NULL;
  }

  // WM_SIZE Message
  {
    GXRECT rcClient;
    gxGetClientRect(hNewWnd, &rcClient);
    gxSendMessage(hNewWnd, GXWM_SIZE, GXSIZE_RESTORED, GXMAKELPARAM(rcClient.right, rcClient.bottom));
  }

  //lpStation->m_pDesktopWindowsMgr->ManageWindowSurface(hNewWnd, GXWM_CREATE);

  if((dwStyle & GXWS_VISIBLE) != NULL)
  {
    gxShowWindow(hNewWnd, GXSW_SHOWNORMAL);
  }
  lpStation->Leave();
  return hNewWnd;
}
//////////////////////////////////////////////////////////////////////////
GXHWND GXDLLAPI gxCreateWindowW(GXLPCWSTR lpClassName,GXLPCWSTR lpWindowName,GXDWORD dwStyle,
               GXINT x,GXINT y,GXINT nWidth,GXINT nHeight,GXHWND hWndParent,GXHMENU hMenu,GXHINSTANCE hInstance,GXLPVOID lpParam)
{
  return gxCreateWindowExW(NULL, lpClassName, lpWindowName, dwStyle,
    x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxMoveWindow(
  GXHWND hWnd,  // handle of window
  int X,  // horizontal position
  int Y,  // vertical position
  int nWidth,  // width
  int nHeight,  // height
  GXBOOL bRepaint   // repaint flag
  )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL) {
    return FALSE;
  }
 
  return lpWnd->Move(X, Y, nWidth, nHeight, bRepaint);
}

GXBOOL GXDLLAPI gxSetWindowPos(
              GXHWND hWnd,        // handle of window
              GXHWND hWndInsertAfter,    // placement-order handle
              GXINT X,          // horizontal position
              GXINT Y,          // vertical position
              GXINT cx,          // width
              GXINT cy,          // height
              GXUINT uFlags        // window-positioning flags
              )
{
  GXHDWP hDWP = gxBeginDeferWindowPos(1);
  gxDeferWindowPos(hDWP, hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
  return gxEndDeferWindowPos(hDWP);
}

GXHDWP GXDLLAPI gxBeginDeferWindowPos(
  int nNumWindows   // number of windows
  )
{
  GXLPDEFERWNDPOS lpDeferWndPos = new GXDEFERWNDPOS;
  lpDeferWndPos->m_aElements.reserve(nNumWindows);
  return GXDWP_HANDLE(lpDeferWndPos);
}

GXHDWP GXDLLAPI gxDeferWindowPos(
  GXHDWP hWinPosInfo,     // handle to internal structure
  GXHWND hWnd,            // handle to window to position
  GXHWND hWndInsertAfter, // placement-order handle
  int x,                  // horizontal position
  int y,                  // vertical position
  int cx,                 // width
  int cy,                 // height
  GXUINT uFlags           // window-positioning flags
  )
{
  GXLPDEFERWNDPOS lpDeferWndPos = GXDWP_PTR(hWinPosInfo);
  if(lpDeferWndPos == NULL){
    goto FUNC_RET;
  }
  GXDEFERWNDPOS::ELEMENT Element;
  Element.hWnd = hWnd;
  Element.hWndInsertAfer = hWndInsertAfter;
  Element.x = x;
  Element.y = y;
  Element.cx = cx;
  Element.cy = cy;
  Element.dwFlags = uFlags;
  lpDeferWndPos->m_aElements.push_back(Element);
FUNC_RET:
  return hWinPosInfo;
}

GXBOOL GXDLLAPI gxEndDeferWindowPos(
  GXHDWP hWinPosInfo   // handle to internal structure
  )
{
  GXLPDEFERWNDPOS lpDeferWndPos = GXDWP_PTR(hWinPosInfo);
  if(lpDeferWndPos == NULL){
    return FALSE;
  }

  for(GXDEFERWNDPOS::ElementArray::iterator it = lpDeferWndPos->m_aElements.begin();
    it != lpDeferWndPos->m_aElements.end(); ++it)
  {
    GXLPWND lpWnd = GXWND_PTR(it->hWnd);
    lpWnd->SetPos(it->hWndInsertAfer, it->x, it->y, it->cx, it->cy, it->dwFlags);
  }

  SAFE_DELETE(lpDeferWndPos);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxSetWindowTextW(
                GXHWND hWnd,  // handle of window or control
                GXLPCWSTR lpString   // address of string
                )
{
  if(lpString == NULL)
    return FALSE;

  return (GXBOOL)(gxSendMessageW(hWnd, GXWM_SETTEXT, 0, (GXLPARAM)lpString) != FALSE);

}

GXINT GXDLLAPI gxGetWindowTextW(
               GXHWND hWnd,    // handle of window or control with text
               GXLPWSTR lpString,  // address of buffer for text
               int nMaxCount     // maximum number of characters to copy
               )
{
  return (GXINT)gxSendMessageW(hWnd, GXWM_GETTEXT, (GXWPARAM)nMaxCount, (GXLPARAM)lpString);
}

int GXDLLAPI gxGetWindowTextA(
               GXHWND hWnd,      // handle of window or control with text
               GXCHAR* lpString,  // address of buffer for text
               int nMaxCount     // maximum number of characters to copy
               )
{
  if(hWnd == NULL)
  {
    return 0;
  }
  GXWCHAR* lpStringW = new GXWCHAR[nMaxCount];
  GXINT nLength = gxGetWindowTextW(hWnd, lpStringW, nMaxCount);
  clStringA_traits::XStringToNative(lpString, nMaxCount, lpStringW, nMaxCount);
  SAFE_DELETE_ARRAY(lpStringW);
  return nLength;
}

int GXDLLAPI gxGetWindowTextLengthW(GXHWND hWnd)
{
  return (int)gxSendMessageW(hWnd, GXWM_GETTEXTLENGTH, 0, 0);
}

int  GXDLLAPI gxGetWindowTextLengthA(GXHWND hWnd)
{
  return gxGetWindowTextLengthW(hWnd);
}


int GXDLLAPI gxInternalGetWindowText(
                  GXHWND hWnd,
                  GXLPWSTR lpString,
                  int nMaxCount
                  )
{
  GXINT nLen = GXSTRLEN(GXWND_PTR(hWnd)->m_pText) + 1;
  nLen = nLen < nMaxCount ? nLen : nMaxCount;
  GXSTRCPYN(lpString, GXWND_PTR(hWnd)->m_pText, (int)nLen);
  return nLen;
}

GXDWORD GXDLLAPI gxGetWindowThreadProcessId (
  GXHWND hWnd,            // handle of window
  GXLPDWORD lpdwProcessId // address of variable for process identifier
  )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation()); 
  if(lpdwProcessId) {
    *lpdwProcessId = 0;
  }
  return lpStation->dwUIThreadId;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxGetClientRect(
               GXHWND hWnd,  // handle of window
               GXLPRECT lpRect   // address of structure for client coordinates
               )
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL)
  {
    gxSetRect(lpRect, 0, 0, 0, 0);
    return FALSE;
  }
  CHECK_LPWND_VAILD(lpWnd);

  lpRect->left    = 0;
  lpRect->top     = 0;
  lpRect->right   = lpWnd->rectWindow.right - lpWnd->rectWindow.left;
  lpRect->bottom  = lpWnd->rectWindow.bottom - lpWnd->rectWindow.top;

  GXINT xMenuOrg = 0;
  GXINT yMenuOrg = 0;

  if(WINSTYLE_HASCAPTION(lpWnd->m_uStyle))
  {
    lpRect->right  -= (FRAME_NC_EDGE_RIGHT + FRAME_NC_EDGE_LEFT);
    lpRect->bottom  -= (FRAME_NC_EDGE_BOTTOM + FRAME_NC_EDGE_CAPTION);

    xMenuOrg = FRAME_NC_EDGE_LEFT;
    yMenuOrg = FRAME_NC_EDGE_CAPTION;
  }
  if(WINSTYLE_HASMENU(lpWnd, GXLPWND_STATION_PTR(lpWnd)->lpDesktopWnd))
  {
    const GXUINT nHeight = MENU_GetMenuBarHeightFast(
      hWnd, lpRect->right, 
      xMenuOrg, yMenuOrg);
    lpRect->bottom -= nHeight;
  }
  if(lpWnd->m_uStyle & GXWS_VSCROLL)
    lpRect->right -= g_SystemMetrics[GXSM_CXVSCROLL];
  if(lpWnd->m_uStyle & GXWS_HSCROLL)
    lpRect->bottom -= g_SystemMetrics[GXSM_CYHSCROLL];
  return TRUE;
}

GXHWND GXDLLAPI gxGetActiveWindow()
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  return GXWND_HANDLE(lpStation->GetActiveWnd());
}

GXHWND GXDLLAPI gxSetActiveWindow(GXHWND hWnd)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL)
    return gxGetActiveWindow();

  return GXWND_HANDLE(lpWnd->SetActive());
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxSetForegroundWindow(
  GXHWND hWnd   // handle of window to bring to foreground
  )
{
  ASSERT(0);
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
GXHWND GXDLLAPI gxGetWindow(
             GXHWND hWnd,  // handle of original window
             GXUINT uCmd   // relationship flag
             )
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL) {
    return NULL;
  }

  LPGXWND lpRetWnd = NULL;
  CHECK_LPWND_VAILD(lpWnd);

  switch(uCmd)
  {
  case GXGW_FIRSTCHILD:
    lpRetWnd = lpWnd->m_pFirstChild;
    break;
  case GXGW_PREVIOUS:
    lpRetWnd = lpWnd->m_pPrevWnd;
    break;
  case GXGW_NEXT:
    lpRetWnd = lpWnd->m_pNextWnd;
    break;
  case GXGW_PARENT:
    lpRetWnd = lpWnd->m_pParent;
    break;
  case GXGW_OWNER:
    lpRetWnd = lpWnd->m_pParent;
    break;
  default:
    ASSERT(FALSE);
  }
  CHECK_LPWND_VAILD(lpRetWnd);
  return GXWND_HANDLE(lpRetWnd);
}
//////////////////////////////////////////////////////////////////////////
GXLONG_PTR GXDLLAPI gxGetWindowLongW(GXHWND hWnd, GXINT nIndex)
{
  if(gxIsWindow(hWnd) == FALSE) {
    return 0;
  }
  //TODO: Ҫ�ع���
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  CHECK_LPWND_VAILD(lpWnd);

#if defined(_X64)
  if(nIndex >= 0) {
    return *(GXLONG_PTR*)((GXLPBYTE)lpWnd + sizeof(GXWnd) + nIndex * 2);
  }
#else
  if(nIndex >= 0) {
    return *(GXLONG_PTR*)((GXLPBYTE)lpWnd + sizeof(GXWnd) + nIndex);
  }
#endif //

  switch(nIndex)
  {
  case GXGWL_WNDPROC:
    return (GXLONG_PTR)lpWnd->m_lpWndProc;
  case GXGWL_STYLE:
    return lpWnd->m_uStyle;
  case GXGWL_EXSTYLE:
    return lpWnd->m_uExStyle;
  case GXGWLP_ID:
    return (GXLONG_PTR)lpWnd->m_pMenu;
  case GXGWL_HINSTANCE:
    return (GXLONG_PTR)lpWnd->m_hInstance;
  case GXGWL_USERDATA: 
    return (GXLONG_PTR)lpWnd->m_dwUserData;
  case GXGWL_COBJECT:
    return (GXLONG_PTR)lpWnd->m_CObj;
  case GXGWL_RESPONDER:
    return (GXLONG_PTR)lpWnd->m_pResponder;
  default:
    ASSERT(FALSE);
  }
  return 0;

}
//////////////////////////////////////////////////////////////////////////

GXLONG_PTR GXDLLAPI gxSetWindowLongW(GXHWND hWnd,GXINT nIndex, GXLONG_PTR dwNewLong)
{
  //TODO: Ҫ�ع����Ͳ�����ô������
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  CHECK_LPWND_VAILD(lpWnd);
  if( ! lpWnd) {
    return 0;
  }

  GXLONG_PTR dwPrevPtr;
  if(nIndex >= 0) {
#if defined(_X64)
    GXLONG_PTR* p = (GXLONG_PTR*)((GXLPBYTE)lpWnd + sizeof(GXWnd) + nIndex * 2);
#else
    GXLONG_PTR* p = (GXLONG_PTR*)((GXLPBYTE)lpWnd + sizeof(GXWnd) + nIndex);
#endif // 
    dwPrevPtr = *p;
    *p = (GXLONG_PTR)dwNewLong;
    return dwPrevPtr;
  }

  switch(nIndex)
  {
  case GXGWL_STYLE:
    {
      GXSTYLESTRUCT gxss;
      gxss.styleNew = (GXDWORD)dwNewLong;
      gxss.styleOld = lpWnd->m_uStyle;
      dwPrevPtr = lpWnd->m_uStyle;
      gxSendMessage(hWnd, GXWM_STYLECHANGING, GXGWL_STYLE, (GXLPARAM)&gxss);
      lpWnd->m_uStyle = gxss.styleNew;
      gxSendMessage(hWnd, GXWM_STYLECHANGED, GXGWL_STYLE, (GXLPARAM)&gxss);
    }
    break;

  case GXGWL_WNDPROC:
    dwPrevPtr  = (GXLONG_PTR)lpWnd->m_lpWndProc;
    lpWnd->m_lpWndProc = (GXWNDPROC)dwNewLong;
    break;

  case GXGWL_EXSTYLE:
    dwPrevPtr = lpWnd->m_uExStyle;
    
    if(((dwPrevPtr ^ dwNewLong) & GXWS_EX_LAYERED) != 0)
      lpWnd->SetLayeredWindowStyle((dwNewLong & GXWS_EX_LAYERED) != 0);

    // ������ܷ��� SetLayeredWindowStyle ǰ��, ���ж�����Ѿ�����Layered��ֱ�ӷ���
    lpWnd->m_uExStyle = (GXULONG)dwNewLong;
    break;

  case GXGWL_USERDATA:
    dwPrevPtr = (GXLONG_PTR)lpWnd->m_dwUserData;
    lpWnd->m_dwUserData = (GXLPVOID)dwNewLong;
    break;

  case GXGWL_COBJECT:
    dwPrevPtr = (GXLONG_PTR)lpWnd->m_CObj;
    lpWnd->m_CObj = (GXDWORD_PTR)dwNewLong;
    break;

  case GXGWL_RESPONDER:
    dwPrevPtr = (GXLONG_PTR)lpWnd->m_pResponder;
    lpWnd->m_pResponder = (GXLPVOID)dwNewLong;
    break;

  default:
    ASSERT(FALSE);
  }
  return dwPrevPtr;
}

GXHWND GXDLLAPI gxSetParent(
  GXHWND hWndChild,     // handle of window whose parent is changing
  GXHWND hWndNewParent   // handle of new parent window
  )
{
  GXLPWND lpWnd = GXWND_PTR(hWndChild);
  CHECK_LPWND_VAILD(lpWnd);
  return lpWnd->SetParent(hWndNewParent);
}


//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxRedrawWindow(
  GXHWND hWnd,                // handle of window
  GXLPCRECT lprcUpdate, // address of structure with update rectangle
  GXHRGN hrgnUpdate,          // handle of update region
  GXUINT flags                // array of redraw flags
  )
{
  ASSERT(FALSE);
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxIsWindowVisible(GXHWND hWnd)
{
  //return hWnd == 0 ? FALSE : ((GXWND_PTR(hWnd)->m_uStyle & GXWS_VISIBLE) != 0);
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  CHECK_LPWND_VAILD(lpWnd);

  return lpWnd->IsVisible();
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxShowWindow(
    GXHWND hWnd,    // handle of window
    int nCmdShow    // show state of window
    )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd)
  {
    CHECK_LPWND_VAILD(lpWnd);
    return lpWnd->ShowWindow(nCmdShow);
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL gxRegisterHotKey(GXHWND hWnd, GXINT id, GXUINT fsModifiers, GXUINT vk)
{
  // ������Bug
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  LPGXHOTKEY lpHotKey = (LPGXHOTKEY)&lpStation->m_HotKeyChain;
  while(lpHotKey != NULL && lpHotKey->hWnd != NULL)
  {
    lpHotKey = lpHotKey->lpNext;
  }
  if(lpHotKey->hWnd != NULL)
  {
    lpHotKey->lpNext = new GXHOTKEY;
    lpHotKey = lpHotKey->lpNext;
  }
  lpHotKey->lpNext      = NULL;
  lpHotKey->hWnd        = hWnd;
  lpHotKey->id          = id;
  lpHotKey->fsModifiers = fsModifiers;
  lpHotKey->vk          = vk;
  return TRUE;
}

GXBOOL gxUnregisterHotKey(GXHWND hWnd, GXINT id)
{
  // ������Bug
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  LPGXHOTKEY lpHotKey = (LPGXHOTKEY)&lpStation->m_HotKeyChain;
  LPGXHOTKEY lpPrevHotKey = NULL;
  while(lpHotKey != NULL)
  {
    if(lpHotKey->hWnd == hWnd && lpHotKey->id == id)
    {
      if(lpPrevHotKey == NULL)
      {
        if(lpHotKey->lpNext != NULL)    {
          *lpHotKey = *lpHotKey->lpNext;
        }
        else  {
          lpHotKey->hWnd        = NULL;
          lpHotKey->id          = NULL;
          lpHotKey->fsModifiers = 0;
          lpHotKey->vk          = NULL;
        }
        return TRUE;
      }
      else{
        lpPrevHotKey->lpNext = lpHotKey->lpNext;
        SAFE_DELETE(lpHotKey);
        return TRUE;
      }
    }
    lpPrevHotKey = lpHotKey;
    lpHotKey = lpHotKey->lpNext;
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// 
// ö��һ�����ڵ������Ӵ���
// ���������ڵ��Ӵ��ڴ����Լ����Ӵ��ڣ�ͬ���ᱻö��
// �����ö�ٹ����У�����������FALSE�����ж�ö�٣�
// gxEnumChildWindows ��������FALSE�������ö��ȫ���Ӵ��ڲ�����TRUE
// 
GXBOOL GXDLLAPI gxEnumChildWindows(
            GXHWND hWndParent,    // handle to parent window
            GXWNDENUMPROC lpEnumFunc,  // pointer to callback function
            GXLPARAM lParam       // application-defined value
            )
{
  GXLPWND lpWnd = GXWND_PTR(hWndParent)->m_pFirstChild;
  while(lpWnd != NULL)
  {
    if(lpEnumFunc(GXWND_HANDLE(lpWnd), lParam) == FALSE)
      return FALSE;
    if(lpWnd->m_pFirstChild != NULL && 
      gxEnumChildWindows(GXWND_HANDLE(lpWnd), lpEnumFunc, lParam) == FALSE)
      return FALSE;
    lpWnd = lpWnd->m_pNextWnd;
  }
  return TRUE;
}

GXHWND GXDLLAPI gxFindWindowEx(
  GXHWND hwndParent,      // handle to parent window
  GXHWND hwndChildAfter,  // handle to a child window 
  GXLPCWSTR lpszClass,    // pointer to class name
  GXLPCWSTR lpszWindow    // pointer to window name
  )
{
  GXLPWND lpParentWnd = GXWND_PTR(hwndParent);
  GXLPWND lpChildAfter = GXWND_PTR(hwndChildAfter); // ����(����)�������������
  GXLPWND lpWnd = NULL;

  // ��λ������
  if(lpParentWnd == NULL)
  {
    GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
    lpParentWnd = lpStation->lpDesktopWnd;
  }

  // ��λ�Ӵ���,�Լ���ʼѭ���Ĵ���
  if(lpChildAfter != NULL)
  {
    if(lpChildAfter->m_pParent != lpParentWnd) {
      return NULL;
    }
    lpWnd = lpChildAfter->m_pNextWnd;
  }
  else
  {
    lpWnd = lpParentWnd->m_pFirstChild;
  }

  while(lpWnd != NULL)
  {
    if((lpszClass == NULL || (lpszClass != NULL && GXSTRCMPI(lpWnd->m_lpClsAtom->szClassName, lpszClass) == 0)) &&
      (lpszWindow == NULL || (lpszWindow != NULL && GXSTRCMPI(lpWnd->m_pText, lpszWindow) == 0)) )
    {
      return GXWND_HANDLE(lpWnd);
    }
    lpWnd = lpWnd->m_pNextWnd;
  }
  return NULL;
}

GXHWND GXDLLAPI gxFindWindow(
  GXLPCWSTR lpClassName,  // pointer to class name
  GXLPCWSTR lpWindowName  // pointer to window name
  )
{
  return gxFindWindowEx(NULL, NULL, lpClassName, lpWindowName);
}

#endif // _DEV_DISABLE_UI_CODE