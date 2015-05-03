// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
#include <Include/GUnknown.H>
#include <Include/GResource.H>
#include <Include/GRegion.H>
#include <Include/GXGraphics.H>
#include <Include/GXCanvas.H>
#include <Include/GXSprite.H>
#include <Include/GXFont.H>
#include <Include/GXImage.H>

// 私有头文件
#include "Include/GXKernel.H"
#include "Include/GXUser.H"
#include "Include/GXGDI.H"
#include <User/GXWindow.h>
#include "Utility/AeroCommon.H"
#include "User/DesktopWindowsMgr.h"
#include "User/WindowsSurface.h"
#include "thread/clLocker.h"

#ifndef _DEV_DISABLE_UI_CODE
//////////////////////////////////////////////////////////////////////////
//
// 内部类型定义
//

//////////////////////////////////////////////////////////////////////////
//
// 内部变量定义
//
//GXHRESDATA hControlRegMap;

//////////////////////////////////////////////////////////////////////////
//
// 内部函数定义
//
GXBOOL GXUSER_InitializeCtrlRegMap();
GXVOID GXUSER_ReleaseCtrlRegMap();
GXVOID GXUSER_InitializeWndProperty();
GXVOID GXUSER_ReleaseWndProperty();
void  LISTVIEW_Register();
void  LISTVIEW_Unregister();
GXVOID  HEADER_Register();
GXVOID  HEADER_Unregister();
GXVOID  TREEVIEW_Register ();
GXVOID  TREEVIEW_Unregister ();

GXVOID  TOOLTIPS_Register();
GXVOID  TOOLTIPS_Unregister();
extern GXWNDCLASSEX WndClassEx_Menu;
extern GXWNDCLASSEX WndClassEx_MyButton;
extern GXWNDCLASSEX WndClassEx_MyEdit;
extern GXWNDCLASSEX WndClassEx_MyEdit_1_3_30;
extern GXWNDCLASSEX WndClassEx_MyListbox;
extern GXWNDCLASSEX WndClassEx_MyStatic;
extern GXWNDCLASSEX WndClassEx_GXUIEdit_1_3_30;
//////////////////////////////////////////////////////////////////////////
//
// 外部引用函数
// 
GXLRESULT DEFWNDPROC_NcHitTest(GXHWND hWnd, GXINT xPos, GXINT yPos);

GXBOOL GXUSER_InitializeCtrlRegMap()
{
  //pControlRegMap = NEW GrapXControlRegMap;
  //hControlRegMap = GrapXResData_Create(GXRESDATA_STRMAP);
  LISTVIEW_Register();
  HEADER_Register();
  TREEVIEW_Register();
  TOOLTIPS_Register();
  
  gxRegisterClassExW(&WndClassEx_Menu);
  gxRegisterClassExW(&WndClassEx_MyButton);
  gxRegisterClassExW(&WndClassEx_MyEdit);
  gxRegisterClassExW(&WndClassEx_MyEdit_1_3_30);
  gxRegisterClassExW(&WndClassEx_MyListbox);
  gxRegisterClassExW(&WndClassEx_MyStatic);
  gxRegisterClassExW(&WndClassEx_GXUIEdit_1_3_30);

  return TRUE;
}

GXVOID GXUSER_ReleaseCtrlRegMap()
{
  LISTVIEW_Unregister();
  HEADER_Unregister();
  TREEVIEW_Unregister();
  TOOLTIPS_Unregister();
  gxUnregisterClassW(WndClassEx_Menu.lpszClassName, NULL);
  gxUnregisterClassW(WndClassEx_MyButton.lpszClassName, NULL);
  gxUnregisterClassW(WndClassEx_MyEdit.lpszClassName, NULL);
  gxUnregisterClassW(WndClassEx_MyListbox.lpszClassName, NULL);
  gxUnregisterClassW(WndClassEx_MyStatic.lpszClassName, NULL);
  gxUnregisterClassW(GXWC_DIALOGW, NULL);
  //GrapXResData_Destroy(hControlRegMap);
  //SAFE_DELETE(pControlRegMap);
}

//////////////////////////////////////////////////////////////////////////
//GXVOID _gxScreenToRenderTarget(GXHWND hWnd, GXLPPOINT lpPoint)
//{
//  LPGXWND lpWnd = GXWND_PTR(hWnd);
//  LPGXWND pParent = GXWND_PTR(lpWnd->m_pParent);
//  LPGXWND lpDesktop = GXLPWND_STATION_PTR(lpWnd)->lpRootFrame;
//  while(pParent != NULL && pParent != lpDesktop)
//  {
//    lpWnd = pParent;
//    pParent = lpWnd->m_pParent;
//  }
//  // 窗口不在最大化时
//  lpPoint->x -= (lpWnd->rectWindow.left/* - FRAME_NC_GLOW_LEFT*/);
//  lpPoint->y -= (lpWnd->rectWindow.top/*  - FRAME_NC_GLOW_TOP */);
//
//  // 窗口最大化时：没有阴影
//  // TODO:
//  //lpPoint->x -= pFrame->rectWindow.left;
//  //lpPoint->y -= pFrame->rectWindow.top;
//}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
int GXDLLAPI gxLoadStringW(
         GXHINSTANCE hInstance,  // handle of module containing string resource 
         GXUINT uID,  // resource identifier 
         GXLPWSTR lpBuffer,  // address of buffer for resource 
         int nBufferMax   // size of buffer 
         )
{
  ASSERT(FALSE);
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
GXHWND GXDLLAPI gxGetParent(
           GXHWND hWnd      // handle of child window
           )
{
  return GXWND_HANDLE(GXWND_PTR(hWnd)->m_pParent);
}
//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxIsWindowEnabled(
             GXHWND hWnd   // handle of window to test
             )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  return lpWnd->IsEnabled();
}



//////////////////////////////////////////////////////////////////////////
int GXDLLAPI gxDrawTextW(
         GXHDC hDC,        // handle to device context 
         GXLPCWSTR lpString,    // pointer to string to draw 
         int nCount,        // string length, in characters 
         GXLPRECT lpRect,      // pointer to structure with formatting dimensions  
         GXUINT uFormat       // text-drawing flags 
         )
{
  const GXDWORD mask = GXDT_BOTTOM | GXDT_CALCRECT | GXDT_CENTER | GXDT_EXPANDTABS | GXDT_LEFT |
    GXDT_NOCLIP | GXDT_RIGHT | GXDT_RTLREADING | GXDT_SINGLELINE | GXDT_TOP | GXDT_VCENTER | GXDT_WORDBREAK/* | GXDT_NOPREFIX*/; 
  LPGXGDIFONT pFont = (LPGXGDIFONT)((LPGXGDIDC)hDC)->hFont;
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hDC);

  //lpDC->pCanvas->EnableAlphaBlend((lpDC->flag & GXDCFLAG_OPAQUEBKMODE) == 0);

  GXUINT uFmt = (uFormat & mask);
  //if(uFormat & GDT_GLOW)
  //{
  //  lpDC->pCanvas->DrawGlowText(
  //    pFont->lpFont, lpString, nCount, lpRect, uFmt & (~GXDT_TABSTOP), /*((LPGXGDIDC)hDC)->crText*/0xffffffff, 7);// TODO: 临时写成白色
  //}
  GXCanvas* pCanvas = lpDC->pCanvas;
  //if(lpDC->flag & GXDCFLAG_OPAQUEBKMODE)
  //{
  //  GXSIZE size;
  //  gxGetTextExtentPointW(hDC, lpString, nCount, &size);
  //  pCanvas->FillRectangle(lpRect->left, lpRect->top, size.cx, size.cy, ((LPGXGDIDC)hDC)->crTextBack);
  //}
  return lpDC->pCanvas->DrawTextW(
    pFont->lpFont, lpString, nCount, lpRect, uFmt & (~GXDT_TABSTOP), ((LPGXGDIDC)hDC)->crText);
  return 0;
}

int GXDLLAPI gxDrawTextA(
  GXHDC hDC,        // handle to device context 
  GXLPCSTR lpString,    // pointer to string to draw 
  int nCount,        // string length, in characters 
  GXLPRECT lpRect,      // pointer to structure with formatting dimensions  
  GXUINT uFormat       // text-drawing flags 
  )
{
  clStringW strW = AnsiStringToUnicodeString(lpString);
  return gxDrawTextW(hDC, strW, nCount, lpRect, uFormat);
}

//////////////////////////////////////////////////////////////////////////
int GXDLLAPI gxFrameRect(
        GXHDC hDC,        // handle to device context 
        GXCONST GXRECT *lprc,    // pointer to rectangle coordinates  
        GXHBRUSH hbr       // handle to brush 
        )
{
  //ASSERT(FALSE);
  //TRACE_UNACHIEVE("=== gxFrameRect ===\n");
  ASSERT(((LPGXGDIBRUSH)hbr)->uStyle == GXBS_SOLID);
  ((LPGXGDIDC)hDC)->pCanvas->DrawRectangle(lprc->left, lprc->top, lprc->right - lprc->left - 1, lprc->bottom - lprc->top - 1, ((LPGXGDIBRUSH)hbr)->crColor);
  return 0;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxDrawEdge(
        GXHDC hdc,      // handle to device context
        GXLPRECT qrc,      // pointer to rectangle coordinates
        GXUINT edge,      // type of inner and outer edge to draw
        GXUINT grfFlags      // type of border
        )
{
  //ASSERT(FALSE);
  //TRACE_UNACHIEVE("=== gxDrawEdge ===\n");
  GXWnd::s_pCommonSpr->PaintModule3x3(((LPGXGDIDC)hdc)->pCanvas, IDCOMMON_GROUPBOX_TOPLEFT, TRUE, qrc);  
  GXDWORD mask = GXBF_ADJUST | GXBF_MIDDLE;
  //ASSERT( (grfFlags & mask) == mask );
  if(grfFlags & GXBF_ADJUST)
  {
    if(grfFlags & GXBF_MIDDLE)
    {
      gxInflateRect(qrc, -3, -3);
    }
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI gxClientToScreen(
  GXHWND hWnd,        // window handle for source coordinates 
  GXLPPOINT lpPoint   // pointer to structure containing screen coordinates  
  )
{
  GXWND_PTR(hWnd)->ClientToScreen(lpPoint, 1);
  return TRUE;
}

GXBOOL GXDLLAPI gxScreenToClient(
  GXHWND hWnd,        // window handle for source coordinates 
  GXLPPOINT lpPoint   // address of structure containing coordinates  
  )
{
  GXWND_PTR(hWnd)->ScreenToClient(lpPoint, 1);
  return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//
//gxDragDetect
// GXBOOL gxDragDetect(GXHWND hwnd, GXLPPOINT pt);
// hwnd 需要监控的窗口句柄
// pt   鼠标初始位置, 坐标空间是客户区.
// 返回值:
//    在按住鼠标左键并且移出参考区域时,返回非0
//    在松开左键时如果没有移出区域,返回0
// 说明:  
//    这个函数首先根据参数计算出一个矩形参考区域,矩形大小取自 gxGetSystemMetrics(GXSM_CXDRAG)和
//    gxGetSystemMetrics(GXSM_CYDRAG) 两个函数.

GXBOOL GXDLLAPI gxDragDetect(
        GXHWND hwnd,   
        GXLPPOINT pt  
        )
{
  GXINT cxDrag = gxGetSystemMetrics(GXSM_CXDRAG);
  GXINT cyDrag = gxGetSystemMetrics(GXSM_CYDRAG);
  GXRECT rect;

  //gxClientToScreen(hwnd, pt);
  gxSetRect(&rect, 
    pt->x - cxDrag, pt->y - cyDrag, 
    pt->x + cxDrag, pt->y + cyDrag);

  // 典型移动处理
  GXMSG msg;
  while(gxGetMessage(&msg, NULL))
  {
    if(msg.message == GXWM_KEYDOWN && msg.wParam == VK_ESCAPE)
    {
      break;
    }
    else if(msg.message == GXWM_MOUSEMOVE && msg.hwnd == hwnd)
    {
      GXPOINT msgpt = {GXGET_X_LPARAM(msg.lParam), GXGET_Y_LPARAM(msg.lParam)};
      if( ! gxPtInRect(&rect, msgpt))
      {
        return TRUE;
      }
      //else {
      //  TRACE("Out of rect\n");
      //}
    }
    else if(msg.message > GXWM_MOUSEFIRST &&
      msg.message <= GXWM_MOUSELAST)
    {
      break;
    }


    //TRACE("gxTranslateMessage\n");

    gxTranslateMessage(&msg);
    gxDispatchMessageW(&msg);
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////
//gxGetWindowRect
GXBOOL GXDLLAPI gxGetWindowRect(
           GXHWND hWnd,    // handle of window
           GXLPRECT lpRect   // address of structure for window coordinates
           )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL) {
    return FALSE;
  }
  lpWnd->GetWindowRect(lpRect);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXDLLAPI GXInvalidateWindowRgn(GXHWND hWnd, GRegion* pRegion, GXBOOL bErase)
{
  GXLPWND lpWnd          = GXWND_PTR(hWnd);
  GXLPSTATION lpStation  = GXLPWND_STATION_PTR(lpWnd);
  GRegion* pUpdateRegion = NULL; // Screen Coordinate
  GRegion* pWndRegion    = NULL; // Screen Coordinate
  GXBOOL   bval          = TRUE;

  if(pRegion->IsEmpty()) {
    bval = FALSE;
    goto FUNC_RET;
  }

  if(pRegion)
  {
    pWndRegion = pRegion->Clone();
    pWndRegion->Offset(lpWnd->rectWindow.left, lpWnd->rectWindow.top);
  }

  lpWnd->GetSystemRegion(GSR_WINDOW|GSR_CLIPSIBLINGS|GSR_PARENTCLIP, &pUpdateRegion);

  if(pWndRegion && pUpdateRegion->Intersect(pWndRegion) == RC_NULL) {
      bval = FALSE;
      goto FUNC_RET;
  }

  // 与已有的区域求并集
  GXWindowsSurface* pWinsSur = lpWnd->GetTopSurface();
  pWinsSur->InvalidateRegion(pUpdateRegion);

FUNC_RET:
  SAFE_RELEASE(pUpdateRegion);
  SAFE_RELEASE(pWndRegion);
  return bval;
}

//////////////////////////////////////////////////////////////////////////
// 其实 GXInvalidateWindowRect 可以使用 GXInvalidateWindowRgn 实现
// 但是单独实现性能更好些
//
GXBOOL GXDLLAPI GXInvalidateWindowRect(
      GXHWND    hWnd,
      GXLPCRECT lpRect,   // Window空间的
      GXBOOL    bErase)
{
  GXLPWND lpWnd         = GXWND_PTR(hWnd);
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);
  GRegion* prgnWindow   = NULL;
  GRegion* pRegion      = NULL;
  GXBOOL   bval         = TRUE;
  GXRECT  rcUpdate;

  if(lpRect != NULL)
  {
    // 校验传入的坐标是否可能是屏幕空间的
    ASSERT(lpRect->right <= lpWnd->rectWindow.right - lpWnd->rectWindow.left &&
      lpRect->bottom <= lpWnd->rectWindow.bottom - lpWnd->rectWindow.top);

    // 如果用户定义的更新区域
    // 那么将区域影射到屏幕坐标，然后与窗口坐标求交
    rcUpdate = *lpRect;
    gxOffsetRect(&rcUpdate, lpWnd->rectWindow.left, lpWnd->rectWindow.top);
    gxIntersectRect(&rcUpdate, &rcUpdate, &lpWnd->rectWindow);
  }
  else    
    rcUpdate = lpWnd->rectWindow;

  if(gxIsRectEmpty(&rcUpdate)) {
    bval = FALSE;
    goto FUNC_RET;
  }

  // 创建更新区域
  lpStation->pGraphics->CreateRectRgn(&pRegion, rcUpdate.left, rcUpdate.top, rcUpdate.right, rcUpdate.bottom);
  lpWnd->GetSystemRegion(GSR_WINDOW|GSR_CLIPSIBLINGS|GSR_PARENTCLIP, &prgnWindow);
  
  if(pRegion->Intersect(prgnWindow) == RC_NULL) {
    bval = FALSE;
    goto FUNC_RET;
  }

  // 与已有的区域求并集
  GXWindowsSurface* pWinsSur = lpWnd->GetTopSurface();
  pWinsSur->InvalidateRegion(pRegion);

FUNC_RET:
  SAFE_RELEASE(prgnWindow);
  SAFE_RELEASE(pRegion);
  return bval;
}

//////////////////////////////////////////////////////////////////////////
//gxInvalidateRect
GXBOOL GXDLLAPI gxInvalidateRect(
          GXHWND    hWnd,      // handle of window with changed update region  
          GXLPCRECT lpRect,    // address of rectangle coordinates 
          GXBOOL    bErase     // erase-background flag 
          )
{
  if( ! hWnd) {
    GXLPSTATION lpStation = IntGetStationPtr();
    GRegion* pRegion = NULL;
    if(lpRect) {
      lpStation->pGraphics->CreateRectRgn(&pRegion, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }
    lpStation->m_pDesktopWindowsMgr->InvalidateWndRegion(NULL, pRegion, FALSE);
    SAFE_RELEASE(pRegion);
  }
  else {
    GXLPWND lpWnd = GXWND_PTR(hWnd);
    if(lpWnd && lpWnd->IsAncestorVisible()) {
      return lpWnd->InvalidateRect(lpRect, bErase);
    }
  }
  return FALSE;
}

GXBOOL GXDLLAPI gxValidateRect(
          GXHWND    hWnd,    // handle of window 
          GXLPCRECT lpRect  // address of validation rectangle coordinates 
          )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  GXLPWND lpCanvasWnd = lpWnd;

  if(lpWnd->m_prgnUpdate == NULL)
  {
    //ASSERT(0);
    return FALSE;
  }

  // 获得具有 Surface 的 Window
  GXWindowsSurface* pSurface = lpWnd->GetTopSurface();
  //while( lpCanvasWnd->m_pWinsSurface == NULL && gxIsTopLevelWindow(GXWND_HANDLE(lpCanvasWnd)) == FALSE)
  //{
  //  lpCanvasWnd = lpCanvasWnd->m_pParent;
  //}

  if(pSurface == NULL)
  {
    ASSERT(0);
    return FALSE;
  }

  if(lpRect == NULL) {
    SAFE_RELEASE(lpWnd->m_prgnUpdate);
  }
  else {
    GXLPSTATION lpStation  = GXLPWND_STATION_PTR(lpWnd);
    GRegion* prgnValid = NULL;
    GXRECT rect = *lpRect;
    GXRECT rcScreen;

    // 客户区在屏幕空间的区域
    lpWnd->GetBoundingRect(FALSE, &rcScreen);
    gxOffsetRect(&rect, rcScreen.left, rcScreen.top);

    lpStation->pGraphics->CreateRectRgn(&prgnValid, rect.left, rect.top, rect.right, rect.bottom);

    if(lpWnd->m_prgnUpdate->Subtract(prgnValid) == RC_NULL) {
      SAFE_RELEASE(lpWnd->m_prgnUpdate);
    }
    SAFE_RELEASE(prgnValid);
  }

  //if(lpWnd->m_pWinsSurface != NULL)
  //  gxSetRect(&lpWnd->m_pWinsSurface->rcScrUpdate, 0, 0, 0, 0);
  //// TODO: 是否要枚举上面的父对象，如果更新区和这个窗口相等则去掉？
  return TRUE;
}

GXBOOL GXDLLAPI gxGetUpdateRect(
  GXHWND    hWnd,     // handle of window
  GXLPRECT  lpRect,   // address of update rectangle coordinates 
  GXBOOL    bErase    // erase flag
  )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL || lpWnd->m_prgnUpdate == NULL) {
    if(lpRect != NULL) {
      gxSetRectEmpty(lpRect);
    }
    return FALSE;
  }

  if(bErase) {
    ASSERT(0);  // FIXME: 暂时不支持!
  }

  if(lpRect != NULL) {
    lpWnd->m_prgnUpdate->GetBounding(lpRect);
    gxOffsetRect(lpRect, -lpWnd->rectWindow.left, -lpWnd->rectWindow.top);
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//gxIsWindow
GXBOOL GXDLLAPI gxIsWindow(
        GXHWND hWnd   // handle of window
        )
{
#ifdef ENABLE_SEH
  __try{
#endif // ENABLE_SEH
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  return lpWnd != NULL && TEST_FLAG_NOT(lpWnd->m_uState, WIS_DESTROYTHISWND);
#ifdef ENABLE_SEH
  }__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH){
    ASSERT(FALSE);
    return FALSE;
  }
#endif // ENABLE_SEH
  //return hWnd != NULL ? TRUE : FALSE;
}
GXBOOL GXDLLAPI gxIsChild(
             GXHWND hWndParent,  // handle of parent window
             GXHWND hWnd   // handle of window to test
             )
{
  GXLPWND lpWndParent = GXWND_PTR(hWndParent);
  GXLPWND lpWnd       = GXWND_PTR(hWnd);

  if(lpWndParent == NULL || lpWnd == NULL) {
    return FALSE;
  }

  while(lpWnd->m_pParent != NULL)
  {
    lpWnd = lpWnd->m_pParent;
    if(lpWnd == lpWndParent) {
      return TRUE;
    }
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//gxReleaseCapture
GXBOOL GXDLLAPI gxReleaseCapture()
{
  // TODO: 发送失去窗口捕获信息
  GXLPSTATION pStation = IntGetStationPtr();
  if(pStation->m_pCapture == NULL)
  {
    return FALSE;
  }
  pStation->m_pCapture = NULL;
  GXPOINT ptCursor;
  gxGetCursorPos(&ptCursor);
//#if defined(_WIN32) || defined(_WINDOWS)  
//  ScreenToClient(pStation->hBindWin32Wnd, (LPPOINT)&ptCursor);
//#endif // #if defined(_WIN32) || defined(_WINDOWS)  
  GXWnd::AnalyzeMouseMoveMsg(NULL, &ptCursor);
  
  return TRUE;
  ////ASSERT(FALSE);
  //GXWnd::gxReleaseCapture();
  //return FALSE;
}
//////////////////////////////////////////////////////////////////////////
//gxSendMessageA
//gxSetCapture
GXHWND GXDLLAPI gxSetCapture(
        GXHWND hWnd   // handle of window to receive mouse capture
        )
{
  // TODO: 检测Disabled
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  GXCLPSTATION pStation = GXLPWND_STATION_PTR(lpWnd);

  GXHWND hPrevWnd = GXWND_HANDLE(pStation->m_pCapture);
  pStation->m_pCapture = GXWND_PTR(hWnd);
  return hPrevWnd;
}
//////////////////////////////////////////////////////////////////////////
//gxGetCapture
GXHWND GXDLLAPI gxGetCapture()
{
  GXCLPSTATION pStation = IntGetStationPtr();
  return GXWND_HANDLE(pStation->m_pCapture);
}
//////////////////////////////////////////////////////////////////////////
//gxSetFocus
GXHWND GXDLLAPI gxSetFocus(
        GXHWND hWnd     // handle of window to receive focus
        )
{
  // Win32测试： SetFocus 可以设置隐藏窗口
  GXLPSTATION pStation = NULL;
  LPGXWND lpWnd = GXWND_PTR(hWnd);

  if(lpWnd == NULL) {
    pStation = IntGetStationPtr();
  }
  else {
    if( ! lpWnd->IsVisible()) {
      return FALSE;
    }
    pStation = GXLPWND_STATION_PTR(lpWnd);
  }

  GXHWND hPrevWnd = GXWND_HANDLE(pStation->m_pKeyboardFocus);
  if(hPrevWnd != NULL) {
    gxSendMessage(hPrevWnd, GXWM_KILLFOCUS, (GXWPARAM)hWnd, NULL);
  }

  pStation->m_pKeyboardFocus = GXWND_PTR(hWnd);

  // 不判断 lpWnd->IsVisible() !
  if(hWnd != NULL) {
    gxSendMessage(hWnd, GXWM_SETFOCUS, (GXWPARAM)hPrevWnd, NULL);
  }

  return hPrevWnd;
}

GXHWND GXDLLAPI gxGetFocus()
{
  const GXLPSTATION pStation = IntGetStationPtr();
  GXLPWND lpFocus = pStation->m_pKeyboardFocus;
  return GXWND_HANDLE(lpFocus);
}

//////////////////////////////////////////////////////////////////////////
//gxGetDlgItem
GXHWND GXDLLAPI gxGetDlgItem(
        GXHWND hDlg,  // handle of dialog box
        GXINT nIDDlgItem   // identifier of control
        )
{
  //ASSERT(FALSE);
  GXWnd* pWnd = GXWND_PTR(hDlg)->m_pFirstChild;
  while(pWnd != NULL)
  {
    if(pWnd->m_pMenu == (GXHMENU)(GXINT_PTR)nIDDlgItem)
      return GXWND_HANDLE(pWnd);
    pWnd = pWnd->m_pNextWnd;
  }
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
//
//编辑词条GetNextDlgGroupltem
//函数功能：该函数检索控件组的第一个控件的句柄，该控件组跟随对话框中指定的控件。
//函数原型：HWND GetNextDlgGroupltem(HWND hDlg，HWND hctl，GXBOOL bPrevious);
//参数：
//hDlg：标识正在被搜寻的对话框。
//hCtl：指定用来作为搜寻开始点的控件。如果此参数为空，函数将以(或者最前一个)最后一个点为搜寻开始点。
//BPrevious：指定参数如何搜寻，如果此参数为TRUE，则函数寻找以前的控件组中的控件。
//  如果为FALSE，则函数寻找控件组中的下一个控件。
//
//返回值：如果GetNextDlgGroupltem函数调用成功，则返回值为控件组中以前的（或下一个）控件。
//  如果函数调用失败，则返回值为零。若想获得更多的错误信息，请调用GetLastError函数。
//备往：GetNextDlgGroupltem函数按照对话框模板中被创建的顺序（或相反的顺序）寻找控件。
//  控件组的第一个控件一定有WS_GROUP类型；所有其他的控件组的控件一定被顺序创建且一定没有WS_GROUP类型。
//  当寻找以前的控件时，函数返回第一个位置上可视的、且不失效的控件。如果由hCtl给定的控件有WS_GROUP
//  类型，则函数暂时反向寻找具有WS_GROUP类型的第一个控件，然后重新回到原来的方向进行寻找，返回可视的、
//  且不失效的第一个控件，如果没有发现控件，则返回hCtl。
//  当寻找下一个控件时，函数返回第一个位置上的可视控件，且没有WS_GROUP类型。如果遇到一个有WS_GROUP
//  类型的控件，则函数反向寻找具有WS_GROUP类型的第一个控件，且如果此控件为可视的、且没有失效，则返回
//  此控件。否则，函数重新回到原来方向的寻找，返回可视的、且不失效的第一个控件。如果没有发现控件。
//  则返回hCtl。
//速查：Windows NT：3.1及以上版本；Windows：95及以上版本；Windows CE：1.0及以上版本；头文件：winuser.h；库文件：user32.lib。
//引自：Artint搜索于百度百科，对于原文有改动。
//备注：原文译的真他妈的晕
GXHWND GXDLLAPI gxGetNextDlgGroupItem(
               GXHWND hDlg,  // handle of dialog box
               GXHWND hCtl,  // handle of control
               GXBOOL bPrevious   // direction flag
               )
{
#define INVALIDATE_FRAME(FRAME)  (FRAME && (FRAME->m_uStyle & (GXWS_VISIBLE | GXWS_DISABLED)) != GXWS_VISIBLE && ((FRAME->m_uStyle & GXWS_GROUP) != 0))
  GXWnd *pWnd = GXWND_PTR(hCtl);
  if(bPrevious == FALSE)
  {
    if(pWnd == NULL)
      pWnd = GXWND_PTR(hDlg)->m_pFirstChild;
    do {
      pWnd = pWnd->m_pNextWnd;
    } while(INVALIDATE_FRAME(pWnd));
    if(pWnd == NULL)
    {
      pWnd = hCtl == 0 ? GXWND_PTR(hDlg)->m_pFirstChild : GXWND_PTR(hCtl);
      if(pWnd == 0)
        return hCtl;
      do {
        pWnd = pWnd->m_pPrevWnd;
      } while((pWnd->m_uStyle & GXWS_GROUP) == 0 &&
        pWnd->m_pPrevWnd != NULL);
    }
    else if(pWnd->m_uStyle & GXWS_GROUP)
    {
      do {
        pWnd = pWnd->m_pPrevWnd;
      } while((pWnd->m_uStyle & GXWS_GROUP) == 0 &&
        pWnd->m_pPrevWnd != NULL);
    }
    return pWnd == 0 ? (hCtl) : GXWND_HANDLE(pWnd);
  }
  else
  {
    if(pWnd == NULL)
    {
      pWnd = GXWND_PTR(hDlg)->m_pFirstChild;
      if(pWnd == NULL)
        return 0;
      GET_LAST_WINDOW(pWnd);
      //while(pWnd->m_pNextFrame != NULL)
      //  pWnd = pWnd->m_pNextFrame;
    }
    if(pWnd->m_uStyle & GXWS_GROUP)
    {
      while(pWnd->m_pNextWnd != NULL && (pWnd->m_pNextWnd->m_uStyle & GXWS_GROUP) == 0)
        pWnd = pWnd->m_pNextWnd;
    }
    do{
      pWnd = pWnd->m_pPrevWnd;
    } while(INVALIDATE_FRAME(pWnd));
    return pWnd == NULL ? hCtl : GXWND_HANDLE(pWnd);
  }
  //TRACE_UNACHIEVE("=== gxGetNextDlgGroupItem(bPrevious == FALSE) ===\n");
  //ASSERT(FALSE);
  return NULL;
#undef INVALIDATE_FRAME
}
GXBOOL GXDLLAPI gxCheckRadioButton(
  GXHWND hDlg,  // handle to dialog box
  int nIDFirstButton,  // identifier of first radio button in group
  int nIDLastButton,  // identifier of last radio button in group
  int nIDCheckButton  // identifier of radio button to select
  )
{
  GXLPWND lpWnd = GXWND_PTR(hDlg)->m_pFirstChild;
  GXBOOL bRet = FALSE;
  while(lpWnd != NULL)
  {
    int nID = (int)lpWnd->m_pMenu;
    if(nID >= nIDFirstButton &&
      nID <= nIDLastButton )
    {
      if(nID == nIDCheckButton)
      {
        gxSendMessage(GXWND_HANDLE(lpWnd), GXBM_SETCHECK, GXBST_CHECKED, 0);
        bRet = TRUE;
      }
      else
        gxSendMessage(GXWND_HANDLE(lpWnd), GXBM_SETCHECK, GXBST_UNCHECKED, 0);
    }
    lpWnd = lpWnd->m_pNextWnd;
  }
  return bRet;
}
//////////////////////////////////////////////////////////////////////////
//gxIntersectRect
//GXBOOL gxIntersectRect(
//           GXLPRECT lprcDst,      // address of structure for intersection
//           GXCONST GXRECT *lprcSrc1,  // address of structure with first rectangle
//           GXCONST GXRECT *lprcSrc2   // address of structure with second rectangle
//           )
//{
//  ASSERT(FALSE);
//  return FALSE;
//}
//////////////////////////////////////////////////////////////////////////
GXINT GXDLLAPI gxMapWindowPoints(
          GXHWND hWndFrom,    // handle of window to be mapped from 
          GXHWND hWndTo,      // handle of window to be mapped to 
          GXLPPOINT lpPoints,    // address of structure array with points to map 
          GXUINT cPoints       // number of structures in array 
          )
{
  //GXPOINT  ptDelta;
  //GXLPPOINT  _lpPoints = lpPoints;
  //GXLPWND lpDesktop = IntGetStationPtr()->lpRootFrame;
  //GXHWND _hWndFrom = hWndFrom == GXHWND_DESKTOP 
  //  ? GXWND_HANDLE(lpDesktop) 
  //  : hWndFrom;

  //ptDelta.x = GXWND_PTR(_hWndFrom)->rectWindow.left - GXWND_PTR(hWndTo)->rectWindow.left;
  //ptDelta.y = GXWND_PTR(_hWndFrom)->rectWindow.top  - GXWND_PTR(hWndTo)->rectWindow.top;

  //for(GXUINT i = 0;i < cPoints; i++)
  //{
  //  _lpPoints->x += ptDelta.x;
  //  _lpPoints->y += ptDelta.y;
  //  _lpPoints++;
  //}
  //return GXMAKELONG(ptDelta.x, ptDelta.y);
  GXLPWND lpWndFrom = GXWND_PTR(hWndFrom);
  GXLPWND lpWndTo = hWndTo ? GXWND_PTR(hWndTo) : lpWndFrom->GetDesktop();
  return lpWndTo->MapWindowPoints(lpWndFrom, lpPoints, cPoints);
}

//////////////////////////////////////////////////////////////////////////
//#ifdef _ENABLE_STMT
//GXBOOL GXDLLAPI gxGetMessage(
//          GXLPMSG lpMsg,
//          GXHWND hWnd
//          )
//{
//  while(GXWnd::s_idxMsgStackBottom == GXWnd::s_idxMsgStackTop)
//  {
//    lpMsg->message = GXWM_NULL;
//    GXWndCanvas::EndDevice();
//    STMT::WaitForTaskEvent(STMT::hDefHandle);
//  }
//  *lpMsg = GXWnd::s_msg[GXWnd::s_idxMsgStackTop];
//  GXWnd::s_msg[GXWnd::s_idxMsgStackTop].message = GXWM_NULL;
//  GXWnd::s_idxMsgStackTop++;
//  GXWnd::s_idxMsgStackTop &= 63;
//  return TRUE;
//}
//#else

//#endif  // #ifdef _ENABLE_STMT



//////////////////////////////////////////////////////////////////////////

GXBOOL GXDLLAPI gxScrollWindow(
  GXHWND hWnd,                 // handle of window to scroll
  int XAmount,                 // amount of horizontal scrolling
  int YAmount,                 // amount of vertical scrolling
  GXCONST GXRECT *lpRect,      // address of structure with scroll rectangle
  GXCONST GXRECT *lpClipRect   // address of structure with clip rectangle
  )
{
  return gxScrollWindowEx(hWnd, XAmount, YAmount, lpRect, lpClipRect, NULL, 
    NULL, GXSW_SCROLLCHILDREN|GXSW_INVALIDATE) != GXERROR;
}

GXBOOL GXDLLAPI gxScrollDC(
  GXIN   GXHDC hDC,
  GXIN   int dx,
  GXIN   int dy,
  GXIN   const GXRECT *lprcScroll,
  GXIN   const GXRECT *lprcClip,
  GXIN   GXHRGN hrgnUpdate,
  GXOUT  LPGXRECT lprcUpdate
  )
{
  return FALSE;
}

GXBOOL GXDLLAPI gxIsIconic(
  GXHWND hWnd   // handle of window
  )
{
  return (gxGetWindowLongW(hWnd, GXGWL_STYLE) & GXWS_MINIMIZE) != 0;
}

GXBOOL GXDLLAPI gxIsZoomed(
  GXHWND hWnd   // handle of window
  )
{
  return (gxGetWindowLongW(hWnd, GXGWL_STYLE) & GXWS_MAXIMIZE) != 0;
}

//////////////////////////////////////////////////////////////////////////

GXHWND GXDLLAPI gxGetAncestor(          
           GXHWND hwnd,
           GXUINT gaFlags
           )
{
  ASSERT(FALSE);
  return NULL;
}

GXLONG_PTR GXDLLAPI gxGetClassLongW(
            GXHWND hWnd,  // handle of window
            int nIndex   // offset of value to retrieve 
            )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL) {
    return 0;
  }
  switch(nIndex)
  {
  case GXGCL_HCURSOR:
    return (GXLONG_PTR)lpWnd->m_lpClsAtom->hCursor;
  default:
    ASSERT(NULL);
  }
  return 0;
}

GXBOOL GXDLLAPI gxCallMsgFilterW(
            GXLPMSG lpMsg,  // pointer to structure with message data
            int nCode   // hook code 
            )
{
  TRACE_UNACHIEVE("=== gxCallMsgFilterW ===\n");
  //ASSERT(NULL);
  return FALSE;
}

GXBOOL GXDLLAPI gxUpdateWindow(
            GXHWND hWnd   // handle of window  
            )
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);
  lpStation->Enter();
  if((lpWnd->m_uStyle & GXWS_VISIBLE) &&
    lpWnd->m_prgnUpdate != NULL &&  // 创建窗口后将产生这个区域,但是有可能被Station线程调用WM_PAINT而清掉这个区域
    gxIsRectEmpty(&lpWnd->rectWindow) == FALSE)
  {
    GXWindowsSurface* pSurface = lpWnd->GetTopSurface();
    lpWnd->UpdateWholeWindow(pSurface);
    lpStation->Leave();
    return TRUE;
  }
  lpStation->Leave();
  return FALSE;
}

GXLONG GXDLLAPI gxGdiGetCharDimensions(
                GXHDC        hdc,
                GXLPTEXTMETRICW  lptm,
                GXLONG*      height
                )
{
  GXTEXTMETRICW tm;
  LPGXGDIDC lpDC = GXGDI_DC_PTR(hdc);
  
  GXGDI_FONT_PTR(lpDC->hFont)->lpFont->GetMetricW(&tm);

  if(lptm != NULL)
    *lptm = tm;
  if(height)  *height = tm.tmHeight;
  return tm.tmAveCharWidth;
}




GXHMONITOR GXDLLAPI gxMonitorFromPoint(
              GXPOINT pt,      // point 
              GXDWORD dwFlags  // determine return value
              )
{
  //D3DPRESENT_PARAMETERS d3dpp;

  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  //GXWndCanvas::GetDeviceParam(&d3dpp);

  //g_MonitorInfo.cbSize = sizeof(GXMONITORINFO);
  //g_MonitorInfo.rcMonitor.left   = 0;
  //g_MonitorInfo.rcMonitor.top    = 0;
  //g_MonitorInfo.rcMonitor.right  = g_SystemMetrics[GXSM_CXSCREEN];
  //g_MonitorInfo.rcMonitor.bottom = g_SystemMetrics[GXSM_CYSCREEN];
  //g_MonitorInfo.rcWork = g_MonitorInfo.rcMonitor;
  //g_MonitorInfo.dwFlags = GXMONITORINFOF_PRIMARY;
  if(dwFlags == GXMONITOR_DEFAULTTONULL)
  {
    return gxPtInRect((GXLPRECT)&lpStation->MonitorInfo.rcMonitor, pt) == FALSE ? NULL : &lpStation->MonitorInfo;
  }
  return &lpStation->MonitorInfo;
}
GXHMONITOR GXDLLAPI gxMonitorFromRect(
             GXLPCRECT lprc,    // rectangle
             GXDWORD dwFlags    // determine return value
             )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  //ASSERT(false);
  return &lpStation->MonitorInfo;
}
GXBOOL GXDLLAPI gxGetMonitorInfoW(
             GXHMONITOR hMonitor,  // handle to display monitor
             GXLPMONITORINFO lpmi  // display monitor information 
             )
{
  GXLPSTATION lpStation = GXSTATION_PTR(GXUIGetStation());
  ASSERT(hMonitor == &lpStation->MonitorInfo);
  *lpmi = *(GXLPMONITORINFO)hMonitor;
  return TRUE;
}

GXHWND GXDLLAPI gxGetDesktopWindow()
{
  LPGXWND lpDesktop = IntGetStationPtr()->lpDesktopWnd;
  return GXWND_HANDLE(lpDesktop);
}
GXBOOL GXDLLAPI gxDestroyIcon(
               GXHICON hIcon   // handle to icon to destroy
               )
{
  if(hIcon == NULL)
    return FALSE;
  LPGXICON lpIcon = GXICON_PTR(hIcon);
  SAFE_RELEASE(lpIcon->pImgIcon);
  SAFE_DELETE(lpIcon);
  return TRUE;
}
GXHICON GXDLLAPI gxCreateIconIndirect(
             GXPICONINFO piconinfo   // pointer to icon information structure
             )
{
  ASSERT(FALSE);
  return NULL;
}
int GXDLLAPI gxGetSystemMetrics(
           int nIndex   // system metric or configuration setting to retrieve  
           )
{
#if defined(_WIN32) || defined(_WINDOWS)
  switch(nIndex)
  {
  case GXSM_CXICONSPACING:
  case GXSM_CYICONSPACING:
  case GXSM_CXEDGE:
  case GXSM_CYEDGE:
  case GXSM_CXSMICON:
  case GXSM_CYSMICON:
  case GXSM_CXICON:
  case GXSM_CYICON:
  case GXSM_CXHSCROLL:
  case GXSM_CYHSCROLL:
  case GXSM_CXDOUBLECLK:
  case GXSM_CYDOUBLECLK:
  case GXSM_CXDRAG:
  case GXSM_CYDRAG:
  case GXSM_CYMENU:
  case GXSM_CYMENUSIZE:
    return GetSystemMetrics(nIndex);
  //case SM_CXSCREEN:
  //  return g_pCurStation->d3dpp.BackBufferWidth;
  //case SM_CYSCREEN:
  //  return g_pCurStation->d3dpp.BackBufferHeight;
  }
#endif // #if defined(_WIN32) || defined(_WINDOWS)
  return g_SystemMetrics[nIndex];
}

GXBOOL GXDLLAPI gxSystemParametersInfoW(
                  GXUINT uiAction, 
                  GXUINT uiParam, 
                  GXLPVOID pvParam, 
                  GXUINT fWinIni)
{
#if defined(_WIN32) || defined(_WINDOWS)
  GXBOOL bRet = SystemParametersInfoW((GXUINT)uiAction, (GXUINT)uiParam, (GXLPVOID)pvParam, (GXUINT)fWinIni);
#elif defined(_IOS)
  GXBOOL bRet = TRUE;
#endif // #if defined(_WIN32) || defined(_WINDOWS)

  if(bRet == TRUE)
  {
    if( uiAction == SPI_GETICONTITLELOGFONT )
    {
      GXSTRCPYN(((GXLOGFONTW*)pvParam)->lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
    }
    else if( uiAction == SPI_GETNONCLIENTMETRICS )
    {
      STATIC_ASSERT(sizeof(GXNONCLIENTMETRICSW) == sizeof(NONCLIENTMETRICSW));
      const size_t sizeofNONCLIENTMETRICS = sizeof(GXNONCLIENTMETRICSW);
      ASSERT(sizeofNONCLIENTMETRICS == uiParam);
      GXNONCLIENTMETRICSW* pncm = (GXNONCLIENTMETRICSW*)pvParam;
      GXSTRCPYN(pncm->lfCaptionFont.lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
      GXSTRCPYN(pncm->lfSmCaptionFont.lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
      GXSTRCPYN(pncm->lfMenuFont.lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
      GXSTRCPYN(pncm->lfStatusFont.lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
      GXSTRCPYN(pncm->lfMessageFont.lfFaceName, L"Font\\msyh.ttf", GXLF_FACESIZE);
    }
    else if( uiAction == SPI_GETWHEELSCROLLLINES ||
      uiAction == SPI_GETFLATMENU)
    {
      return bRet;
    }
    else
      ASSERT(FALSE);
  }
  else
    ASSERT(FALSE);
  return bRet;
}

#if defined(_WIN32) || defined(_WINDOWS)
GXHCURSOR GXDLLAPI gxLoadCursorW(
           GXHINSTANCE hInstance,  // handle of application instance
           GXLPCWSTR lpCursorName   // name string or cursor resource identifier  
           )
{
  return (GXHCURSOR)LoadCursorW((HINSTANCE)hInstance, (GXLPCWSTR)lpCursorName);
}

GXHCURSOR GXDLLAPI gxLoadCursorA(
  GXHINSTANCE hInstance,  // handle of application instance
  GXLPCSTR lpCursorName   // name string or cursor resource identifier  
  )
{
  return (GXHCURSOR)LoadCursorA((HINSTANCE)hInstance, (GXLPCSTR)lpCursorName);
}
#elif defined(_IOS)
GXHCURSOR GXDLLAPI gxLoadCursorW(
                                 GXHINSTANCE hInstance,  // handle of application instance
                                 GXLPCWSTR lpCursorName   // name string or cursor resource identifier  
                                 )
{
  return NULL;
}

GXHCURSOR GXDLLAPI gxLoadCursorA(
                                 GXHINSTANCE hInstance,  // handle of application instance
                                 GXLPCSTR lpCursorName   // name string or cursor resource identifier  
                                 )
{
  return NULL;
}
#endif // #if defined(_WIN32) || defined(_WINDOWS)


GXHCURSOR GXDLLAPI gxSetCursor(
          GXHCURSOR hCursor   // handle of cursor
          )
{
#if defined(_WIN32) || defined(_WINDOWS)
  GXLPSTATION lpStation = IntGetStationPtr();
  GXHCURSOR hPrevCursor = (GXHCURSOR)lpStation->hCursor;
  lpStation->hCursor = (HCURSOR)hCursor;
  return hPrevCursor;
  //return (GXHCURSOR)SetCursor((HCURSOR)hCursor);
#elif defined(_IOS)
  return NULL;  
#endif // defined(_WIN32) || defined(_WINDOWS)
}

int GXDLLAPI gxShowCursor(
  GXBOOL bShow   // cursor visibility flag  
  )
{
#if defined(_WIN32) || defined(_WINDOWS)
  int nval = ShowCursor(bShow);
  return nval;
#elif defined(_IOS)
  return NULL;  
#endif // defined(_WIN32) || defined(_WINDOWS)
}

GXBOOL GXDLLAPI gxGetCursorPos(
              GXLPPOINT lpPoint   // address of structure for cursor position  
              )
{
  GXLPSTATION lpStation = IntGetStationPtr();
  *lpPoint = lpStation->m_ptCursor;
  return TRUE;

//#if defined(_WIN32) || defined(_WINDOWS)
//  GXBOOL r = GetCursorPos((LPPOINT)lpPoint);
//  ScreenToClient(IntGetStationPtr()->hBindWin32Wnd, (LPPOINT)lpPoint);
//  return (GXBOOL)r;
//#elif defined(_IOS)
//  return TRUE;
//#endif // defined(_WIN32) || defined(_WINDOWS)
}

GXBOOL GXDLLAPI gxSetCursorPos(int X, int Y)
{
  GXLPSTATION lpStation = IntGetStationPtr();
  GXPOINT pos = {X, Y};
  if( ! lpStation->SetCursorPos(&pos))
    return FALSE;
#if defined(_WIN32) || defined(_WINDOWS)
  ClientToScreen(lpStation->hBindWin32Wnd, (LPPOINT)&pos);
  //SendMessage(lpStation->hBindWin32Wnd, WM_USER + 57, 0, 0);
  GXBOOL bret = SetCursorPos(pos.x, pos.y);
  //SendMessage(lpStation->hBindWin32Wnd, WM_USER + 58, 0, 0);
  return bret;
#else
  return TRUE;
#endif // #if defined(_WIN32) || defined(_WINDOWS)
}

GXLONG GXDLLAPI gxGetDialogBaseUnits()
{
#if defined(_WIN32) || defined(_WINDOWS)
  const GXLONG nUnit = GetDialogBaseUnits();
  return nUnit;
#else
  return 0x00100008;
#endif // defined(_WIN32) || defined(_WINDOWS)
}

GXLRESULT GXDLLAPI gxCallWindowProcW(
             GXWNDPROC lpPrevWndFunc,  // pointer to previous procedure
             GXHWND hWnd,  // handle to window
             GXUINT Msg,  // message
             GXWPARAM wParam,  // first message parameter
             GXLPARAM lParam   // second message parameter
             )
{
  if(lpPrevWndFunc != NULL)
    return lpPrevWndFunc(hWnd, Msg, wParam, lParam);
  return NULL;
}


GXBOOL GXDLLAPI gxTrackMouseEvent(
           GXLPTRACKMOUSEEVENT lpEventTrack  // pointer to a TRACKMOUSEEVENT structure 
           )
{
  //ASSERT(FALSE);
  return NULL;
}

GXBOOL GXDLLAPI gxIsWindowUnicode(
           GXHWND hWnd   // handle of window
           )
{
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//GXBOOL IsPointInFrame(GXWnd *pFrame, GXPOINT *ppt)
//{
//  return( pFrame->rectWindow.left  <=  ppt->x &&
//    pFrame->rectWindow.right    >  ppt->x &&
//    pFrame->rectWindow.top    <=  ppt->y &&
//    pFrame->rectWindow.bottom  >  ppt->y );    
//}

LPGXWND GXWnd::ChildWindowFromPoint(LPGXPOINT lpPoint, GXLRESULT* lpHitTest, GXBOOL* bEnabled)
{
  CHECK_LPWND_VAILD(this);

  GXLPWND pWnd = m_pFirstChild;
  GXLPWND pParent = NULL;
  GXLRESULT htParent = GXHTNOWHERE;
  *lpHitTest = GXHTERROR;
  //GXBOOL& enabled = *bEnabled;
  *bEnabled = IsEnabled(); // 包含了对父级的检查

  // 在窗口之外
  if( ! gxPtInRect(&rectWindow, *lpPoint)) {
    return NULL;
  }

  if( ! (*bEnabled)) {
    return this;
  }

  // 没有子对象
  if(pWnd == NULL) {
    *lpHitTest = m_lpWndProc(GXWND_HANDLE(this), GXWM_NCHITTEST, NULL, GXMAKELONG(lpPoint->x, lpPoint->y));
    return this;
  }

  // 循环到最后一个窗口, 因为最后一个窗口是最前显示的
  GET_LAST_WINDOW(pWnd);
  //while(pWnd->m_pNextFrame != NULL) {
  //  pWnd = pWnd->m_pNextFrame;
  //}

  do {
    if( ((pWnd->m_uStyle & GXWS_VISIBLE) != 0) &&
      //((pWnd->m_uStyle & GXWS_DISABLED) == 0) &&
      ((pWnd->m_uState & WIS_DESTROYTHISWND) == 0) &&
      ((pWnd->m_uExStyle & (GXWS_EX_TRANSPARENT | GXWS_EX_IMPALPABLE)) == 0) &&
      gxPtInRect(&pWnd->rectWindow, *lpPoint) )
    {
      GXLRESULT ht = GXHTERROR;

      // 如果被禁止, 则直接返回
      if(TEST_FLAG(pWnd->m_uStyle, GXWS_DISABLED)) {
        *lpHitTest = ht;
        *bEnabled  = FALSE;
        return pWnd;
      }
      
      ht = pWnd->m_lpWndProc(GXWND_HANDLE(pWnd), GXWM_NCHITTEST, NULL, GXMAKELONG(lpPoint->x, lpPoint->y));

      if(ht == GXHTCLIENT) // 在客户区, 检测子对象
      {
        pParent  = pWnd;
        htParent = ht;
        pWnd     = pWnd->m_pFirstChild;

        if(pWnd == NULL) {
          break;
        }

        GET_LAST_WINDOW(pWnd);
        //while(pWnd->m_pNextFrame != NULL) {
        //  pWnd = pWnd->m_pNextFrame;
        //}
        continue;
      }
      else if(ht > GXHTCLIENT)  // 不在客户区
      {
        *lpHitTest = ht;
        return pWnd;
      }
      // 其他情况,比如返回 GXHTTRANSPARENT 则继续
    }
    pWnd = pWnd->m_pPrevWnd;
  }while(pWnd != NULL);

  if(pParent != NULL) {
    *lpHitTest = htParent;
    return pParent;
  }
  return NULL;
}

GXHWND GXDLLAPI gxWindowFromPoint(
           GXPOINT *lpPoint   // structure with point
           )
{
  GXLRESULT ht;
  GXBOOL bEnabled;
  LPGXWND lpDesktop = IntGetStationPtr()->lpDesktopWnd;
  LPGXWND lpWnd = lpDesktop->ChildWindowFromPoint(lpPoint, &ht, &bEnabled);
  CHECK_LPWND_VAILD(lpWnd);

  if(lpWnd == NULL || ! bEnabled) {
    return NULL;
  }
  return GXWND_HANDLE(lpWnd);
}

GXHWND GXDLLAPI gxChildWindowFromPoint(
              GXHWND hWndParent,  // handle to parent window
              GXPOINT Point   // structure with point coordinates
              )
{
  GXLRESULT ht;
  GXBOOL bEnabled;
  LPGXWND lpWnd = GXWND_PTR(hWndParent);
  lpWnd = lpWnd->ChildWindowFromPoint(&Point, &ht, &bEnabled);
  if(lpWnd == NULL)
    return NULL;
  return GXWND_HANDLE(lpWnd);
}

GXBOOL GXDLLAPI gxAdjustWindowRectEx(
            GXLPRECT lpRect,  // pointer to client-rectangle structure
            GXDWORD dwStyle,  // window styles
            GXBOOL bMenu,  // menu-present flag
            GXDWORD dwExStyle   // extended style
            )
{
  NOT_IMPLEMENT_FUNC_MAKER;
  return NULL;
}
int GXDLLAPI gxGetDlgCtrlID(
         GXHWND hwndCtl   // handle of control  
         )
{
  ASSERT(FALSE);
  return NULL;
}

GXBOOL GXDLLAPI gxEnableWindow(
          GXHWND hWnd,  // handle to window
          GXBOOL bEnable   // flag for enabling or disabling input
          )
{
  // TODO: 如果 disable window 则去掉 Capture 属性
  //return GXWND_PTR(hWnd)->gxEnableWindow(bEnable);
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  CHECK_LPWND_VAILD(lpWnd);

  GXBOOL bPrev = (lpWnd->m_uStyle & GXWS_DISABLED) == 0;
  if(bEnable == FALSE) {
    SET_FLAG(lpWnd->m_uStyle, GXWS_DISABLED);
  }
  else {
    RESET_FLAG(lpWnd->m_uStyle, GXWS_DISABLED);
  }
  gxSendMessage(hWnd, GXWM_ENABLE, bEnable, 0);
  return bPrev;
}

GXBOOL GXDLLAPI gxEndDialog(
         GXHWND hDlg,  // handle to dialog box
         int nResult   // value to return
         )
{
  GXWND_PTR(hDlg)->m_uState |= WIS_ENDDIALOG;
  gxSetWindowLong(hDlg, GXDWL_MSGRESULT, nResult);
  return TRUE;
}

GXUINT GXDLLAPI gxGetDoubleClickTime()
{
  return 500; // TODO: 放到系统配置里面
}

GXSHORT GXDLLAPI gxGetAsyncKeyState(
  int vKey   // virtual-key code
  )
{
#if defined(_WIN32) || defined(_WINDOWS)
  return GetAsyncKeyState(vKey);
#else
  return 0;
#endif // defined(_WIN32) || defined(_WINDOWS)
}

GXSHORT GXDLLAPI gxGetKeyState(
  int nVirtKey   // virtual-key code
  )
{
  // 看下文档，这个似乎是和消息队列相关的
#if defined(_WIN32) || defined(_WINDOWS)
  return GetKeyState(nVirtKey);
#else
  return 0;
#endif // defined(_WIN32) || defined(_WINDOWS)
}

GXBOOL GXDLLAPI gxFlashWindow(
  GXHWND hWnd,	  // handle to window to flash  
  GXBOOL bInvert 	// flash status 
  )
{
  return FALSE;
}

LPGXGRAPHICS GXDLLAPI GXGetGraphics(GXHWND hWnd)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL)
  {
    GXLPSTATION pStation = IntGetStationPtr();
    return pStation->pGraphics;
  }

  return GXLPWND_STATION_PTR(lpWnd)->pGraphics;
}

GXHWND GXDLLAPI GXGetDlgItemByName(GXHWND hWnd, GXLPCWSTR szName)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(TEST_FLAG(lpWnd->m_uState, WIS_ISDIALOGEX))
  {
    DLGLOG* pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
    if(pDlgLog != NULL && pDlgLog->cbSize == sizeof(DLGLOG))
    {
      return pDlgLog->GetItem(szName);
    }
  }
  return NULL;
}

LPGXWNDCANVAS GXDLLAPI GXGetWndCanvas(GXHDC hdc)
{
  return GXGDI_DC_PTR(hdc)->pWndCanvas;
}
#endif // #ifndef _DEV_DISABLE_UI_CODE
GXDWORD GXDLLAPI gxCharUpperBuffW(
  GXLPWSTR lpsz,  // pointer to buffer containing characters to process 
  GXDWORD cchLength   // number of characters to process  
  )
{
  // TODO: 要支持所有拉丁文字
  GXDWORD i;
  for(i = 0; i < cchLength; i++)
  {
    const GXWCHAR c = lpsz[i];
    if(c == 0)
      return i;
    else if(c >= 'a' && c <= 'z')
    {
      lpsz[i] = c - 'a' + 'A';
    }
  }
  return i;
}

GXDWORD GXDLLAPI gxCharLowerBuffW(
  GXLPWSTR lpsz,  // pointer to buffer containing characters to process 
  GXDWORD cchLength   // number of bytes or characters to process  
  )
{
  // TODO: 要支持所有拉丁文字
  GXDWORD i;
  for(i = 0; i < cchLength; i++)
  {
    const GXWCHAR c = lpsz[i];
    if(c == 0)
      return i;
    else if(c >= 'A' && c <= 'Z')
    {
      lpsz[i] = c - 'A' + 'a';
    }
  }
  return i;
}
