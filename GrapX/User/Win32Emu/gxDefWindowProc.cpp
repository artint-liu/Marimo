#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
//#include <GrapX/GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GTexture.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GXCanvas.h>
#include <GrapX/GXImage.h>

// 私有头文件
#include <clutility.h>

#include "GXStation.h"
#include "User/WindowsSurface.h"
#include <User/GXWindow.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/gUxtheme.h"
#include <User/ScrollBar.h>
#include <Utility/HLSL/FXCommRegister.h>

#define DRAW_CAPTION

GXBOOL GXDLLAPI gxGetMessage(GXLPMSG lpMsg, GXHWND hWnd);
GXLRESULT GXDLLAPI gxSendMessageW(GXHWND hWnd,GXUINT Msg,GXWPARAM wParam,GXLPARAM lParam);
extern "C" GXBOOL GXDLLAPI GXUIPostRootMessage(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
GXLRESULT DEFWNDPROC_NcPaint_HasCaption(LPGXWND lpWnd, GXHRGN hRgn);

GXUINT MENU_DrawMenuBar( GXHDC hDC, LPGXRECT lprect, GXHWND hWnd,GXBOOL suppress_draw);
void MENU_TrackMouseMenuBar( GXHWND hWnd, GXINT ht, GXPOINT pt );


const GXFLOAT  c_flScaleWidth  = 2.0f;
const GXFLOAT  c_flScaleHeight  = 2.0f;

// 用于拖动窗口时手感的改善
GXHWND  g_hSensorWnd;
GXPOINT g_ptSensorCursor;
GXPOINT g_ptSensorWnd;
GXHICON g_hSensorCursor;
//////////////////////////////////////////////////////////////////////////

GXLRESULT DEFWNDPROC_SetText(GXHWND hWnd, GXLPCWSTR pszName)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  GXINT  nLength = BYTE_ALIGN_32(GXSTRLEN(pszName) + 1);

  if(lpWnd->m_pText != NULL)
  {
    GXINT nCurrentLength = BYTE_ALIGN_32(GXSTRLEN(lpWnd->m_pText) + 1);
    if(nLength > nCurrentLength)
    {
      SAFE_DELETE(lpWnd->m_pText);
      lpWnd->m_pText = new GXWCHAR[nLength];
    }
    GXSTRCPY(lpWnd->m_pText, pszName);
  }
  else if(IS_IDENTIFY(pszName))
  {
    SAFE_DELETE(lpWnd->m_pText);
    lpWnd->m_pText = (GXWCHAR*)pszName;
  }
  else
  {
    lpWnd->m_pText = new GXWCHAR[nLength];
    GXSTRCPY(lpWnd->m_pText, pszName);
  }
  return TRUE;
}

GXLRESULT DEFWNDPROC_GetTextLength(GXHWND hWnd)
{
  if(hWnd == NULL)
  {
    return 0;
  }

  LPGXWND lpWnd = GXWND_PTR(hWnd);
  CHECK_LPWND_VAILD(lpWnd);
  GXINT nLength = (GXINT)GXSTRLEN(lpWnd->m_pText);
  return nLength;
}

GXLRESULT DEFWNDPROC_GetText(GXHWND hWnd, GXLPWSTR pszName, int ccTextMax)
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  if(lpWnd == NULL)
  {
    return 0;
  }
  CHECK_LPWND_VAILD(lpWnd);

  GXINT nLength = (GXINT)GXSTRLEN(lpWnd->m_pText) + 1;

  if(nLength > ccTextMax)
  {
    nLength = ccTextMax;
  }

  GXSTRCPYN(pszName, lpWnd->m_pText, nLength);
  return nLength;
}

#define FRAME_CAPTION_HEIGHT    g_SystemMetrics[GXSM_CYCAPTION]
#define FRAME_SIZEBOX_LEFT      g_SystemMetrics[GXSM_CYFIXEDFRAME]  // TODO: 没测试
#define FRAME_SIZEBOX_RIGHT     g_SystemMetrics[GXSM_CYFIXEDFRAME]  // TODO: 没测试
#define FRAME_SIZEBOX_TOP       g_SystemMetrics[GXSM_CXFIXEDFRAME]  // TODO: 没测试
#define FRAME_SIZEBOX_BOTTOM    g_SystemMetrics[GXSM_CXFIXEDFRAME]  // TODO: 没测试
#define FRAME_CLOSEBTN_WIDTH    43
#define FRAME_CLOSEBTN_HEIGHT   15  // 没用上
#define FRAME_MAXBTN_WIDTH      26

GXLRESULT DEFWNDPROC_NcHitTest(GXHWND hWnd, GXINT xPos, GXINT yPos)
{
  //if(fwKeys & MK_CONTROL)
  //  return HTCAPTION;
  GXLRESULT ret = GXHTCLIENT;
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  GXPOINT ptCursor = {xPos, yPos};    // TODO: 优化掉
  GXLPWND lpDesktop = GXLPWND_STATION_PTR(lpWnd)->lpDesktopWnd;
  //GXULONG uPrevState = m_uState & FIS_MASK;
  ////if((GetKeyState(GXVK_LCONTROL) & 0xffff0000) != 0)
  ////{
  ////  return HTCAPTION;
  ////}
  //m_uState &= (~FIS_MASK);
  if(gxGetAsyncKeyState(GXVK_LCONTROL) & 0x00008000)
    return GXHTCAPTION;
  GXRECT rect = lpWnd->rectWindow;
  if(gxPtInRect(&rect, ptCursor) == FALSE)
    return GXHTNOWHERE;
  if(WINSTYLE_HASCAPTION(lpWnd->m_uStyle))
  {
    // 标题区
    if(xPos > rect.left && xPos < rect.right && yPos > rect.top && yPos <= rect.top + FRAME_CAPTION_HEIGHT)
    {
      if(xPos < rect.left + FRAME_SIZEBOX_LEFT)
        ret = GXHTTOPLEFT;
      else if(xPos > rect.right - FRAME_SIZEBOX_RIGHT)
        ret = GXHTTOPRIGHT;
      else if(FALSE && (lpWnd->m_uStyle & GXWS_SYSMENU) && xPos > rect.right - (FRAME_SIZEBOX_RIGHT + FRAME_CLOSEBTN_WIDTH))
        ret = GXHTCLOSE;    // 暂时不支持
      else if(FALSE && (lpWnd->m_uStyle & GXWS_SYSMENU) && (lpWnd->m_uStyle & GXWS_MAXIMIZEBOX) &&
        (xPos > rect.right - (FRAME_SIZEBOX_RIGHT + FRAME_CLOSEBTN_WIDTH + FRAME_MAXBTN_WIDTH)))
        ret = GXHTMAXBUTTON;  // 暂时不支持
      else if(FALSE && (lpWnd->m_uStyle & GXWS_SYSMENU) && (lpWnd->m_uStyle & GXWS_MINIMIZEBOX) &&
        (xPos > rect.right - (FRAME_SIZEBOX_RIGHT + FRAME_CLOSEBTN_WIDTH + FRAME_MAXBTN_WIDTH * 2)))
        ret = GXHTMINBUTTON;  // 暂时不支持
      else if(yPos < rect.top + FRAME_SIZEBOX_TOP)
        ret = GXHTTOP;
      else
        ret = GXHTCAPTION;
    }
    else if(yPos > rect.top + FRAME_CAPTION_HEIGHT && yPos < rect.bottom - FRAME_SIZEBOX_BOTTOM)
    {
      if( xPos > rect.left && xPos < rect.left + FRAME_SIZEBOX_LEFT)
        ret = GXHTLEFT;
      else if(xPos > rect.right - FRAME_SIZEBOX_RIGHT && xPos < rect.right)
        ret = GXHTRIGHT;
    }
    else if(yPos >= rect.bottom - FRAME_SIZEBOX_BOTTOM && yPos < rect.bottom)
    {
      if(xPos < rect.left + FRAME_SIZEBOX_LEFT)
        ret = GXHTBOTTOMLEFT;
      else if(xPos > rect.right - FRAME_SIZEBOX_RIGHT)
        ret =  GXHTBOTTOMRIGHT;
      else if(yPos > rect.bottom - FRAME_SIZEBOX_BOTTOM)
        ret = GXHTBOTTOM;
    }
    else
      ret = GXHTNOWHERE;

    // 没有Resizing属性
    if((lpWnd->m_uStyle & GXWS_THICKFRAME) == 0 && ret >= GXHTLEFT && ret <= GXHTBOTTOMRIGHT)
    {
      if(ret >= GXHTTOP && ret <= GXHTTOPRIGHT)
        return GXHTCAPTION;
      return GXHTBORDER;
    }
    if(ret != GXHTCLIENT && ret != GXHTNOWHERE)
      return ret;
    rect.left   += FRAME_SIZEBOX_LEFT  ;
    rect.top    += FRAME_SIZEBOX_TOP   ;
    rect.right  += FRAME_SIZEBOX_RIGHT ;
    rect.bottom += FRAME_SIZEBOX_BOTTOM;
  }

  // 检测矩形鼠标在滚动条区域
  if(lpWnd->m_uStyle & GXWS_VSCROLL && lpWnd->m_uStyle & GXWS_HSCROLL)
  {
    if(xPos > rect.right - g_SystemMetrics[GXSM_CXVSCROLL] && 
      xPos < rect.right && 
      yPos < rect.bottom - g_SystemMetrics[GXSM_CYHSCROLL])
      ret = GXHTVSCROLL;
    else if(yPos > rect.bottom - g_SystemMetrics[GXSM_CYHSCROLL] &&
      yPos < rect.bottom &&
      xPos < rect.right - g_SystemMetrics[GXSM_CXVSCROLL])
      ret = GXHTHSCROLL;
    else if(yPos > rect.bottom - g_SystemMetrics[GXSM_CYHSCROLL] && 
      xPos > rect.right - g_SystemMetrics[GXSM_CXVSCROLL])
      ret = GXHTGROWBOX;
  }
  else if(lpWnd->m_uStyle & GXWS_VSCROLL)
  {
    if(xPos > rect.right - g_SystemMetrics[GXSM_CXVSCROLL] && xPos < rect.right)
      ret = GXHTVSCROLL;
  }
  else if(lpWnd->m_uStyle & GXWS_HSCROLL)
  {
    if(yPos > rect.bottom - g_SystemMetrics[GXSM_CYHSCROLL] && yPos < rect.bottom)
      ret = GXHTHSCROLL;
  }
  if(WINSTYLE_HASMENU(lpWnd, lpDesktop) && ret == GXHTCLIENT && yPos < rect.top + FRAME_CAPTION_HEIGHT * 2)
  {//TODO: 这个是临时代码
    return GXHTMENU;
  }
  return ret;
}
#define GXSC_OPERATION_DEFAULT      0
#define GXSC_OPERATION_MOUSEMOVE    2
#define GXSC_OPERATION_TOPLEFT      4
#define GXSC_OPERATION_TOP          3
#define GXSC_OPERATION_TOPRIGHT     5
#define GXSC_OPERATION_LEFT         1
#define GXSC_OPERATION_RIGHT        2
#define GXSC_OPERATION_BOTTOMLEFT   7
#define GXSC_OPERATION_BOTTOM       6
#define GXSC_OPERATION_BOTTOMRIGHT  8
GXLRESULT DEFWNDPROC_NcLButtonDown(GXHWND hWnd, GXUINT nHittest, GXINT xScrPos, GXINT yScrPos)
{
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  //TRACE("DEFWNDPROC_NcLButtonDown\n");
  // SC_SIZE 移动时低4位的位置码
  // 4 3 5
  // 1   2
  // 7 6 8
  // 鼠标拖动窗口移动时是2
  // 用键盘控制窗口移动时是0
  GXDWORD uCmdType = GXSC_DEFAULT;
  if(nHittest == GXHTVSCROLL)
  {
    uCmdType = GXSC_VSCROLL;
  }
  else if(nHittest == GXHTHSCROLL)
  {
    uCmdType = GXSC_HSCROLL;
  }
  else if(nHittest == GXHTCAPTION)
  {
    uCmdType = GXSC_MOVE | GXSC_OPERATION_MOUSEMOVE;
#ifdef ENABLE_DYNMAIC_EFFECT
    CGXFrame::s_ptPrevMousePos.x = xScrPos;
    CGXFrame::s_ptPrevMousePos.y = yScrPos;

    //GXRECT rcWin;
    //GXRECT rcWinUV;
    //GXSIZE sizeTex;
    //gxGetWindowRect(&rcWin);        // 获得Win的尺寸
    //gxGetRenderingRect(this, NULL, NULL, &rcWinUV);  // 转换为渲染纹理的坐标
    //gxGetOverlappedRect(&rcWin,&rcWin);    // 转换为窗口覆盖的区域
    //gxGetOverlappedRect(&rcWinUV,&rcWinUV);  // 转换为渲染纹理覆盖的区域
    //sizeTex.cx = CGraphics::s_d3dpp.BackBufferWidth;
    //sizeTex.cy = CGraphics::s_d3dpp.BackBufferHeight;
    //m_pKGrid->BuildGrid(&rcWin, &rcWinUV, &sizeTex);
    if(GXWND_PTR(hWnd)->m_pCanvas != NULL)
      GXWND_PTR(hWnd)->m_pCanvas->pKGrid->BuildGrid();
#endif // </ENABLE_DYNMAIC_EFFECT>
  }
  else if(nHittest == GXHTLEFT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_LEFT;
  }
  else if(nHittest == GXHTRIGHT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_RIGHT;
  }
  else if(nHittest == GXHTTOP)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_TOP;
  }
  else if(nHittest == GXHTBOTTOM)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_BOTTOM;
  }
  else if(nHittest == GXHTTOPLEFT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_TOPLEFT;
  }
  else if(nHittest == GXHTTOPRIGHT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_TOPRIGHT;
  }
  else if(nHittest == GXHTBOTTOMLEFT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_BOTTOMLEFT;
  }
  else if(nHittest == GXHTBOTTOMRIGHT)
  {
    uCmdType = GXSC_SIZE | GXSC_OPERATION_BOTTOMRIGHT;
  }
  else if(nHittest == GXHTMENU)
  {
    uCmdType = GXSC_MOUSEMENU;
  }
  return gxSendMessageW(hWnd, GXWM_SYSCOMMAND, uCmdType, GXMAKELPARAM(xScrPos, yScrPos));
}

//////////////////////////////////////////////////////////////////////////
GXUINT g_MsgForScrollMsg;
GXWPARAM g_WParamForScrollMsg;
GXVOID GXCALLBACK Scroll_TimerProc(GXHWND hWnd,GXUINT uMsg,GXUINT idEvent,GXDWORD dwTime)
{
  gxSendMessageW(hWnd, g_MsgForScrollMsg, g_WParamForScrollMsg, NULL);
  //TRACE("VScroll_TimerProc\n");
}
//////////////////////////////////////////////////////////////////////////


GXLRESULT DEFWNDPROC_SysCommand_SizeMove(GXHWND hWnd, GXUINT uCmdType, GXINT xScrPos, GXINT yScrPos)
{
  GXPOINT      ptCursor;
  GXSIZE      sizeWindow;
  GXMINMAXINFO  mmi;
  GXRECT      rect;
  GXRECT      rcWin;
  GXDWORD      emCmdType = uCmdType & 0xFFF0;
  GXDWORD      emOpType  = uCmdType & 0xF;
  GXLPWND      lpWnd = GXWND_PTR(hWnd);

  GXWnd::gxGetCursorPos(&ptCursor);
  lpWnd->GetWindowRect(&rect);
  lpWnd->_GetSystemMinMaxInfo(&mmi);

  if(lpWnd->m_pParent != NULL)
  {
    lpWnd->m_pParent->ScreenToClient((GXLPPOINT)&rect, 2);
  }
  rcWin = rect;

  sizeWindow.cx = rect.right  - rect.left;
  sizeWindow.cy = rect.bottom - rect.top;
#ifndef ENABLE_DYNMAIC_EFFECT
  if(emCmdType == GXSC_MOVE)
  {
    g_hSensorWnd = hWnd;
    g_ptSensorCursor.x = xScrPos;
    g_ptSensorCursor.y = yScrPos;
    g_ptSensorWnd.x = rect.left;
    g_ptSensorWnd.y = rect.top;
    g_hSensorCursor = GXWnd::s_hCursorArrow;
    gxShowCursor(FALSE);
  }
#endif // ENABLE_DYNMAIC_EFFECT
  gxSetCapture(hWnd);
  while(1)
  {
    GXMSG gxmsg;
    gxGetMessage(&gxmsg, NULL);
    if(gxmsg.message == GXWM_NCLBUTTONUP || gxmsg.message == GXWM_LBUTTONUP)
    {
      gxReleaseCapture();
      GXUIPostRootMessage(gxmsg.hwnd, GXWM_LBUTTONUP, gxmsg.wParam, gxmsg.lParam);
      g_hSensorWnd = NULL;
      if(g_hSensorCursor != NULL)
        gxShowCursor(TRUE);
      g_hSensorCursor = NULL;
      return 0;
    }

    gxGetCursorPos(&gxmsg.pt);
    if(emCmdType == GXSC_MOVE)
    {
      rect.left   = rcWin.left + (gxmsg.pt.x - xScrPos);
      rect.top    = rcWin.top  + (gxmsg.pt.y - yScrPos);
      rect.right  = rect.left + sizeWindow.cx;
      rect.bottom = rect.top  + sizeWindow.cy;

#ifdef ENABLE_DYNMAIC_EFFECT
      GXPOINT ptCurCursor;
      gxGetCursorPos(&ptCurCursor);
      if(GXWND_PTR(hWnd)->m_pCanvas != NULL)
        GXWND_PTR(hWnd)->m_pCanvas->pKGrid->DragGrid(
          &D3DXVECTOR2((float)xScrPos, (float)yScrPos),
          &D3DXVECTOR2((float)(ptCurCursor.x - CGXFrame::s_ptPrevMousePos.x), (float)(ptCurCursor.y - CGXFrame::s_ptPrevMousePos.y)));
      //TRACE("%d,%d\n",xScrPos,yScrPos);
      CGXFrame::s_ptPrevMousePos.x = ptCurCursor.x;
      CGXFrame::s_ptPrevMousePos.y = ptCurCursor.y;
#endif // </ENABLE_DYNMAIC_EFFECT>
    }
    else if(emCmdType == GXSC_SIZE)
    {
      // 传送系统尺寸到用户设置
      gxSendMessageW(hWnd, GXWM_GETMINMAXINFO, NULL, (GXLPARAM)&mmi);
      //WinMsg_GetMinMaxInfo(&mmi);
      // 没有对用户返回值进行合法性检查

      // 对于尺寸不能在处理之前判断， 因为鼠标移动量可能很大。
      // 可能之前的尺寸远小于限制值，加上偏移之后的值又会大于限制值。
      // 应该在加上偏移之后调整到限制范围之内
      GXLONG  *  pxSize, *  pySize;        // 指向调整尺寸的变量
      GXLONG  *  pxBase, *  pyBase;        // 指向基准尺寸的指针
      GXLONG    xMinTrackInc,  yMinTrackInc,  // 四个限定值
        xMaxTrackInc,  yMaxTrackInc;
      if(emOpType == GXSC_OPERATION_LEFT)
      {
        rect.left = rcWin.left + (gxmsg.pt.x - xScrPos);

        pxSize = &rect.left;
        pySize = NULL;
        pxBase = &rect.right;
        pyBase = NULL;
        xMinTrackInc = -mmi.ptMinTrackSize.x;
        xMaxTrackInc = -mmi.ptMaxTrackSize.x;
      }
      else if(emOpType == GXSC_OPERATION_RIGHT)
      {
        rect.right = rcWin.right + (gxmsg.pt.x - xScrPos);

        pxSize = &rect.right;
        pySize = NULL;
        pxBase = &rect.left;
        pyBase = NULL;
        xMinTrackInc = mmi.ptMinTrackSize.x;
        xMaxTrackInc = mmi.ptMaxTrackSize.x;
      }
      else if(emOpType == GXSC_OPERATION_TOP)
      {
        rect.top = rcWin.top + (gxmsg.pt.y - yScrPos);

        pxSize = NULL;
        pySize = &rect.top;
        pxBase = NULL;
        pyBase = &rect.bottom;
        yMinTrackInc = -mmi.ptMinTrackSize.y;
        yMaxTrackInc = -mmi.ptMaxTrackSize.y;
      }
      else if(emOpType == GXSC_OPERATION_BOTTOM)
      {
        rect.bottom = rcWin.bottom + (gxmsg.pt.y - yScrPos);

        pxSize = NULL;
        pySize = &rect.bottom;
        pxBase = NULL;
        pyBase = &rect.top;
        yMinTrackInc = mmi.ptMinTrackSize.y;
        yMaxTrackInc = mmi.ptMaxTrackSize.y;
      }
      else if(emOpType == GXSC_OPERATION_TOPLEFT)
      {
        rect.left = rcWin.left + (gxmsg.pt.x - xScrPos);
        rect.top  = rcWin.top  + (gxmsg.pt.y - yScrPos);

        pxSize = &rect.left;
        pxBase = &rect.right;
        xMinTrackInc = -mmi.ptMinTrackSize.x;
        xMaxTrackInc = -mmi.ptMaxTrackSize.x;

        pySize = &rect.top;
        pyBase = &rect.bottom;
        yMinTrackInc = -mmi.ptMinTrackSize.y;
        yMaxTrackInc = -mmi.ptMaxTrackSize.y;
      }
      else if(emOpType == GXSC_OPERATION_TOPRIGHT)
      {
        rect.right = rcWin.right + (gxmsg.pt.x - xScrPos);
        rect.top   = rcWin.top   + (gxmsg.pt.y - yScrPos);

        pxSize = &rect.right;
        pxBase = &rect.left;
        xMinTrackInc = mmi.ptMinTrackSize.x;
        xMaxTrackInc = mmi.ptMaxTrackSize.x;

        pySize = &rect.top;
        pyBase = &rect.bottom;
        yMinTrackInc = -mmi.ptMinTrackSize.y;
        yMaxTrackInc = -mmi.ptMaxTrackSize.y;
      }
      else if(emOpType == GXSC_OPERATION_BOTTOMLEFT)
      {
        rect.left   = rcWin.left   + (gxmsg.pt.x - xScrPos);
        rect.bottom = rcWin.bottom + (gxmsg.pt.y - yScrPos);

        pxSize = &rect.left;
        pxBase = &rect.right;
        xMinTrackInc = -mmi.ptMinTrackSize.x;
        xMaxTrackInc = -mmi.ptMaxTrackSize.x;

        pySize = &rect.bottom;
        pyBase = &rect.top;
        yMinTrackInc = mmi.ptMinTrackSize.y;
        yMaxTrackInc = mmi.ptMaxTrackSize.y;
      }
      else if(emOpType == GXSC_OPERATION_BOTTOMRIGHT)
      {
        rect.right  = rcWin.right  + (gxmsg.pt.x - xScrPos);
        rect.bottom = rcWin.bottom + (gxmsg.pt.y - yScrPos);

        pxSize = &rect.right;
        pxBase = &rect.left;
        xMinTrackInc = mmi.ptMinTrackSize.x;
        xMaxTrackInc = mmi.ptMaxTrackSize.x;

        pySize = &rect.bottom;
        pyBase = &rect.top;
        yMinTrackInc = mmi.ptMinTrackSize.y;
        yMaxTrackInc = mmi.ptMaxTrackSize.y;
      }
      sizeWindow.cx = rect.right - rect.left;
      sizeWindow.cy = rect.bottom - rect.top;
      if(pxSize != NULL)
      {
        if(sizeWindow.cx < mmi.ptMinTrackSize.x)
          *pxSize = *pxBase + xMinTrackInc;
        else if(sizeWindow.cx > mmi.ptMaxTrackSize.x)
          *pxSize = *pxBase + xMaxTrackInc;
      }
      if(pySize != NULL)
      {
        if(sizeWindow.cy < mmi.ptMinTrackSize.y)
          *pySize = *pyBase + yMinTrackInc;
        else if(sizeWindow.cy > mmi.ptMaxTrackSize.y)
          *pySize = *pyBase + yMaxTrackInc;
      }
    }

    gxMoveWindow(hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
    //lpWnd->MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
  }
  return 0;
}

GXLRESULT DEFWNDPROC_SysCommand_Scroll(GXHWND hWnd, GXUINT uCmdType, GXINT xScrPos, GXINT yScrPos)
{
  //TRACE("DEFWNDPROC_SysCommand\n");
  LPGXWND lpWnd = GXWND_PTR(hWnd);
  
  {
    GXSCROLLBARDRAWINGINFO sbdi;
    LPGXSCROLLBAR pTrackScroll = NULL;
    GXPOINT ptHit = {0,0};
    GXINT  nPos;
    GXINT  nRevise = 0;// 如果过 VScroll 和 HScroll 同时存在，用来校正另外一者的高度（宽度）
    if(uCmdType == GXSC_VSCROLL)
    {
      g_MsgForScrollMsg = GXWM_VSCROLL;
      if(lpWnd->m_uStyle & GXWS_HSCROLL)
        nRevise = g_SystemMetrics[GXSM_CYHSCROLL];
      GXGetScrollBarDrawingInfo(&sbdi, lpWnd->m_lpVScrollBar, lpWnd->rectWindow.bottom - lpWnd->rectWindow.top - nRevise);
      if(yScrPos < lpWnd->rectWindow.top + GX_SCROLLBAR_BUTTON_HEIGHT)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_LINEUP, 0);
      else if(yScrPos > lpWnd->rectWindow.bottom - GX_SCROLLBAR_BUTTON_HEIGHT - nRevise)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_LINEDOWN, 0);
      else if(yScrPos < lpWnd->rectWindow.top + GX_SCROLLBAR_BUTTON_HEIGHT + sbdi.nTrackPos)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_PAGEUP, 0);
      else if(yScrPos > lpWnd->rectWindow.bottom - GX_SCROLLBAR_BUTTON_HEIGHT - (sbdi.nTotalPage - sbdi.nTrackPos) + sbdi.nPageSize - nRevise)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_PAGEDOWN, 0);
      else
      {
        pTrackScroll = lpWnd->m_lpVScrollBar;
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_THUMBTRACK, pTrackScroll->m_nPos);
        pTrackScroll->m_nTrackPos = pTrackScroll->m_nPos;
      }
    }
    else
    {
      g_MsgForScrollMsg = GXWM_HSCROLL;
      if(lpWnd->m_uStyle & GXWS_VSCROLL)
        nRevise = g_SystemMetrics[GXSM_CXVSCROLL];
      GXGetScrollBarDrawingInfo(&sbdi, lpWnd->m_lpHScrollBar, lpWnd->rectWindow.right - lpWnd->rectWindow.left - nRevise);
      if(xScrPos < lpWnd->rectWindow.left + GX_SCROLLBAR_BUTTON_WIDTH)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_LINELEFT, 0);
      else if(xScrPos > lpWnd->rectWindow.right - GX_SCROLLBAR_BUTTON_WIDTH - nRevise)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_LINERIGHT, 0);
      else if(xScrPos < lpWnd->rectWindow.left + GX_SCROLLBAR_BUTTON_WIDTH + sbdi.nTrackPos)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_PAGELEFT, 0);
      else if(xScrPos > lpWnd->rectWindow.right - GX_SCROLLBAR_BUTTON_WIDTH - (sbdi.nTotalPage - sbdi.nTrackPos) + sbdi.nPageSize - nRevise)
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_PAGERIGHT, 0);
      else
      {
        pTrackScroll = lpWnd->m_lpHScrollBar;
        g_WParamForScrollMsg = GXMAKEWPARAM(GXSB_THUMBTRACK, pTrackScroll->m_nPos);
        pTrackScroll->m_nTrackPos = pTrackScroll->m_nPos;
      }
    }

    gxSendMessageW(GXWND_HANDLE(lpWnd), g_MsgForScrollMsg, g_WParamForScrollMsg, NULL);
    if(GXLOWORD(g_WParamForScrollMsg) != GXSB_THUMBTRACK)
      gxSetTimer(hWnd, 65534, 300, Scroll_TimerProc);
    else
    {
      ASSERT(pTrackScroll != NULL);
      nPos = pTrackScroll->m_nPos;
      ptHit.x = xScrPos;
      ptHit.y = yScrPos;
    }
    while(1)
    {
      GXMSG gxmsg;
      GXINT nDelta;
      //TRACE("Enter gxGetMessage\n");
      gxGetMessage(&gxmsg, NULL);
      if(GXLOWORD(g_WParamForScrollMsg) == GXSB_THUMBTRACK)
      {
        if(uCmdType == GXSC_VSCROLL)
          nDelta = (gxmsg.pt.y - ptHit.y);
        else
          nDelta = (gxmsg.pt.x - ptHit.x);
        nDelta = nDelta * (pTrackScroll->m_nMax - pTrackScroll->m_nMin) / sbdi.nTotalPage;
        nDelta += nPos;
        //nPos = hWnd->m_lpVScrollBar->m_nPos + nDelta;
        //TRACE("GXSB_THUMBTRACK - Delta:%d\n",nDelta);
        if(  nDelta != pTrackScroll->m_nTrackPos  && 
          nDelta >= pTrackScroll->m_nMin    && 
          nDelta <= pTrackScroll->m_nMax    )
        {
          pTrackScroll->m_nTrackPos = nDelta;
          //TRACE("GXSB_THUMBTRACK:%d\n",nDelta);
          gxSendMessageW(hWnd, g_MsgForScrollMsg, GXMAKEWPARAM(GXSB_THUMBTRACK, nDelta), 0);
        }
      }

      if(gxmsg.message == GXWM_LBUTTONUP)
      {
        gxKillTimer(hWnd, 65534);
        if(GXLOWORD(g_WParamForScrollMsg) == GXSB_THUMBTRACK)
        {
          //TRACE("GXSB_THUMBTRACK:%d\n",nDelta);
          gxSendMessageW(hWnd, g_MsgForScrollMsg, GXMAKEWPARAM(GXSB_THUMBPOSITION, pTrackScroll->m_nTrackPos), 0);
        }
        gxSendMessageW(hWnd, g_MsgForScrollMsg, GXMAKEWPARAM(GXSB_ENDSCROLL, 0), 0);
        GXUIPostRootMessage((GXHWND)gxmsg.hwnd, GXWM_LBUTTONUP, gxmsg.wParam, gxmsg.lParam);
        return 0;
      }
    }
  }
  return 0;
}
GXLRESULT DEFWNDPROC_SysCommand(GXHWND hWnd, GXUINT uCmdType, GXINT xScrPos, GXINT yScrPos)
{
  GXWORD emCmdType = (GXWORD)(uCmdType & 0xFFF0);
  if(emCmdType == GXSC_VSCROLL || emCmdType == GXSC_HSCROLL)
    return DEFWNDPROC_SysCommand_Scroll(hWnd, uCmdType, xScrPos, yScrPos);
  else if(emCmdType == GXSC_MOVE || emCmdType == GXSC_SIZE)
    return DEFWNDPROC_SysCommand_SizeMove(hWnd, uCmdType, xScrPos, yScrPos);
  else if(emCmdType == GXSC_MOUSEMENU)
  {
    GXPOINT ptPos;
    ptPos.x = xScrPos;
    ptPos.y = yScrPos;
    MENU_TrackMouseMenuBar(hWnd, GXHTMENU, ptPos);
    return 0L;
  }
  TRACE("不太支持的 SysCommand 消息 %d\n", emCmdType);
  return 0L;
}
//////////////////////////////////////////////////////////////////////////

//(P)WM_NCLBUTTONDOWN
//(S)WM_SYSCOMMAND
//(S)WM_HSCROLL
//(R)WM_HSCROLL
//...
//(R)WM_SYSCOMMAND

GXLRESULT DEFWNDPROC_NcPaint(LPGXWND lpWnd, GXHRGN hRgn)
{
  //GXDWORD uStyle = gxGetWindowLong(GWL_STYLE);
  //GXDWORD uStyle = gxGetWindowLong(GWL_STYLE);
  GXLPWND lpDesktop = GXLPWND_STATION_PTR(lpWnd)->lpDesktopWnd;
  GXRECT  rect;
  if((lpWnd->m_uStyle & (GXWS_THICKFRAME | GXWS_CAPTION | GXWS_VSCROLL | GXWS_HSCROLL)) != 0 )
  {
    GXGDIREGION* prgnUpdate = GXGDI_RGN_PTR(hRgn);
    GXWndCanvas Canvas(GXWND_HANDLE(lpWnd), prgnUpdate->lpRegion, GXDCX_WINDOW|GXDCX_PARENTCLIP | GXDCX_CLIPSIBLINGS);
    GXRECT  rcWindow;
    lpWnd->GetWindowRect(&rcWindow);
    gxOffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);

    if(WINSTYLE_HASCAPTION(lpWnd->m_uStyle))
    {
      // 绘制 Frame 前关闭 Alpha 合成
      // 使背景色就是 Frame 图片的颜色， 
      // 并保留 Frame 中的 Alpha 通道
      //if(s_pMouseFocusFrame != this)
      //  m_uState &= (~(FIS_MAXBUTTON | FIS_MINBUTTON | FIS_CLOSE));
      DEFWNDPROC_NcPaint_HasCaption(lpWnd, hRgn);
      //_GXDrawFrameEdge(&Canvas, &rcWindow, lpWnd->m_uStyle, lpWnd->m_uExStyle, lpWnd->m_uState);

      // 如果窗口具有 WS_THICKFRAME | WS_BORDER | WS_CAPTION 等属性
      // 则将边缘缩减，便于绘制滚动条
      rcWindow.left  += FRAME_NC_EDGE_LEFT;
      rcWindow.top  += FRAME_NC_EDGE_CAPTION;
      rcWindow.right  -= FRAME_NC_EDGE_RIGHT;
      rcWindow.bottom  -= FRAME_NC_EDGE_BOTTOM;
    }
    if((lpWnd->m_uStyle & GXWS_VSCROLL) != 0)
    {
      if(lpWnd->m_lpVScrollBar == NULL)
      {
        lpWnd->m_lpVScrollBar = new GXSCROLLBAR;
        memset(lpWnd->m_lpVScrollBar, 0, sizeof(GXSCROLLBAR));
      }
      //if(!(m_lpVScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
      {
        rect.left = rcWindow.right - g_SystemMetrics[GXSM_CXVSCROLL];
        rect.top = rcWindow.top;
        rect.right = rcWindow.right;
        rect.bottom = rcWindow.bottom;
        if(lpWnd->m_uStyle & GXWS_HSCROLL)
          rect.bottom -= g_SystemMetrics[GXSM_CYHSCROLL];
        GXPaintVScrollBar(&Canvas, lpWnd->m_lpVScrollBar, &rect, GXPSB_NORMAL);
      }
    }
    if((lpWnd->m_uStyle & GXWS_HSCROLL) != 0)
    {
      if(lpWnd->m_lpHScrollBar == NULL)
      {
        lpWnd->m_lpHScrollBar = new GXSCROLLBAR;
        memset(lpWnd->m_lpHScrollBar, 0, sizeof(GXSCROLLBAR));
      }
      //if(!(m_lpHScrollBar->m_uFlag & GXSIF_DISABLENOSCROLL))
      {
        rect.left   = rcWindow.left;
        rect.top    = rcWindow.bottom - g_SystemMetrics[GXSM_CYHSCROLL];
        rect.right  = rcWindow.right;
        rect.bottom = rcWindow.bottom;
        if(lpWnd->m_uStyle & GXWS_VSCROLL)
          rect.right -= g_SystemMetrics[GXSM_CXVSCROLL];
        GXPaintHScrollBar(&Canvas, lpWnd->m_lpHScrollBar, &rect, GXPSB_NORMAL);
      }
    }
  }
  if(WINSTYLE_HASMENU(lpWnd, lpDesktop))
    //lpWnd->m_pMenu != NULL && (lpWnd->m_uStyle & WS_CHILD) == NULL && (lpWnd->m_pParent == NULL || lpWnd->m_pParent == &CGXFrame::s_TheRootFrame))
  {
    GXHDC hdc = gxGetWindowDC(GXWND_HANDLE(lpWnd));
    GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(CM_SourceCopy);
    gxGetClientRect(GXWND_HANDLE(lpWnd), &rect);
    if(WINSTYLE_HASCAPTION(lpWnd->m_uStyle))
    {
      rect.left  += FRAME_NC_EDGE_LEFT;
      rect.top   += FRAME_NC_EDGE_CAPTION;
      rect.right -= FRAME_NC_EDGE_RIGHT;
    }
    MENU_DrawMenuBar(hdc, &rect, GXWND_HANDLE(lpWnd), FALSE);
    gxReleaseDC(GXWND_HANDLE(lpWnd), hdc);
  }
  return GX_OK;
}
GXLRESULT DEFWNDPROC_NcPaint_HasCaption(LPGXWND lpWnd, GXHRGN hRgn)
{
  GXHWND hWnd = GXWND_HANDLE(lpWnd);
  GXHTHEME hTheme = gxGetWindowTheme(hWnd);
  if(!hTheme)
    return GX_FAIL;

  const GXDWORD dwDCXFlags = GXDCX_CACHE|GXDCX_WINDOW|GXDCX_PARENTCLIP|GXDCX_CLIPSIBLINGS;
  GXRECT rect;  // 矩形区域是坐标区
  GXHDC hdc = gxGetDCEx(hWnd, hRgn, dwDCXFlags);
  GXREGN rgWin;
  GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(CM_SourceCopy);
  gxRectToRegn(&rgWin, &lpWnd->rectWindow);

  // 标题栏
  gxSetRect(&rect, 0, 0, rgWin.width, g_SystemMetrics[GXSM_CYCAPTION]);
  gxDrawThemeBackground(hTheme, hdc, GXWP_CAPTION, GXCS_ACTIVE, &rect, NULL);

#ifdef DRAW_CAPTION
  if(lpWnd->m_pText != NULL)
  {
    GXRECT rcCaption = rect;
    gxOffsetRect(&rcCaption, 10, 5);
    LPGXGDIFONT pFont = (LPGXGDIFONT)GXGDI_DC_PTR(hdc)->hFont;
    gxDrawTextW(hdc, lpWnd->m_pText, -1, &rcCaption, GXDT_SINGLELINE);
  }
#endif

  // 左右边界
  gxSetRect(&rect, 0, g_SystemMetrics[GXSM_CYCAPTION], g_SystemMetrics[GXSM_CYFIXEDFRAME], rgWin.height - g_SystemMetrics[GXSM_CXFIXEDFRAME]);
  gxDrawThemeBackground(hTheme, hdc, GXWP_FRAMELEFT, GXFS_ACTIVE, &rect, NULL);

  rect.left = rgWin.width - g_SystemMetrics[GXSM_CYFIXEDFRAME];
  rect.right = rgWin.width;
  gxDrawThemeBackground(hTheme, hdc, GXWP_FRAMERIGHT, GXFS_ACTIVE, &rect, NULL);

  // 下边界
  gxSetRect(&rect, 0, rgWin.height - g_SystemMetrics[GXSM_CXFIXEDFRAME], rgWin.width, rgWin.height);
  gxDrawThemeBackground(hTheme, hdc, GXWP_FRAMEBOTTOM, GXFS_ACTIVE, &rect, NULL);

  gxReleaseDC(hWnd, hdc);
  return GX_OK;
}

GXLRESULT DEFWNDPROC_EraseBkGnd(GXHWND hWnd, GXHDC hdc)
{
  GXRECT rcClient;
  gxGetClientRect(hWnd, &rcClient);
  GXHTHEME hTheme = gxGetWindowTheme(hWnd);
  if(!hTheme)
    return 0;
  GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(CM_SourceCopy);
  gxDrawThemeBackground(hTheme, hdc, GXWP_DIALOG, 0, &rcClient, NULL);
  GXGDI_DC_PTR(hdc)->pCanvas->SetCompositingMode(CM_SourceOver);
  return -1;  // 返回非0表示已经处理
}
GXLRESULT DEFWNDPROC_SetCursor(GXHWND hWnd, GXHWND hCursorWnd, GXLPARAM lParam)
{
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  if(gxSendMessageW(GXWND_HANDLE(lpWnd->m_pParent), GXWM_SETCURSOR, (GXWPARAM)hCursorWnd, lParam) == TRUE)
    return TRUE;
  if(hWnd != hCursorWnd)
    return FALSE;
  return IntSetCursor((GXWPARAM)hCursorWnd, lParam);
  //if(GXLOWORD(lParam) == GXHTCLIENT && lpWnd->m_lpClsAtom != NULL && lpWnd->m_lpClsAtom->hCursor != NULL)
  //  gxSetCursor(lpWnd->m_lpClsAtom->hCursor);
  //return TRUE;
}
//GXLRESULT DEFWNDPROC_DisplayChange(LPGXWND lpWnd, GXINT nColorDepth, GXLPARAM dwScreenSize)
//{
//  //ASSERT(0);
//  // TODO: 本来写的重新实现,但是我忘了为啥要重新实现了呢....
//  // TODO: 重新实现
//
//  if(lpWnd->m_pFirstChild != NULL)
//    gxSendMessageW(GXWND_HANDLE(lpWnd->m_pFirstChild), GXWM_DISPLAYCHANGE, nColorDepth, dwScreenSize);
//  if(lpWnd->m_pNextWnd != NULL)
//    gxSendMessageW(GXWND_HANDLE(lpWnd->m_pNextWnd), GXWM_DISPLAYCHANGE, nColorDepth, dwScreenSize);
//  return 1L;
//}

GXLRESULT DEFWNDPROC_LostResetDevice(GXUINT message, LPGXWND lpWnd, GXINT nColorDepth, GXLPARAM dwScreenSize)
{
  GXBOOL bLostDevice = (dwScreenSize == 0L);
#ifdef ENABLE_DYNMAIC_EFFECT
  if( lpWnd->m_pCanvas != NULL && bLostDevice)
  {
    SAFE_DELETE(lpWnd->m_pCanvas->pKGrid);
  }
#endif // ENABLE_DYNMAIC_EFFECT
  if(lpWnd->m_pFirstChild != NULL)
    gxSendMessageW(GXWND_HANDLE(lpWnd->m_pFirstChild), message, nColorDepth, dwScreenSize);
  if(lpWnd->m_pNextWnd != NULL)
    gxSendMessageW(GXWND_HANDLE(lpWnd->m_pNextWnd), message, nColorDepth, dwScreenSize);
  return 1L;
}
//////////////////////////////////////////////////////////////////////////
  //static 
  //  BEGIN_RENDERSTATE_BLOCK(s_RenderState)
  //  RENDERSTATE_BLOCK(GXRS_CULLMODE, GXCULL_NONE)
  //  RENDERSTATE_BLOCK(GXRS_ALPHABLENDENABLE, TRUE)
  //  RENDERSTATE_BLOCK(GXRS_SRCBLEND, GXBLEND_SRCALPHA)
  //  RENDERSTATE_BLOCK(GXRS_DESTBLEND, GXBLEND_INVSRCALPHA)
  //  END_RENDERSTATE_BLOCK

//GXLRESULT DEFWNDPROC_CalcCanvseSize(GXLPWND lpWnd, LPCANVASSIZEINFO lpCanvasInfo)
//{
//  ASSERT(lpWnd->m_pWinsSurface != NULL);
//
//  GXWndCanvas::GXGetRenderSurfaceExt(
//    lpWnd, 
//    (GXINT*)&lpCanvasInfo->sizeRenderingSur.cx, 
//    (GXINT*)&lpCanvasInfo->sizeRenderingSur.cy);
//
//  GXWndCanvas::GXGetRenderingRect(lpWnd, NULL, NULL, 
//    &lpCanvasInfo->rcRenderingSrc);
//
//  gxRectToRegn(&lpCanvasInfo->rgDest, &lpWnd->rectWindow);
//  GXGetOverlappedRegion(&lpCanvasInfo->rgDest, &lpCanvasInfo->rgDest);
//  return GX_OK;
//}
//GXLRESULT DEFWNDPROC_SetRenderCanvasState(GXLPWND lpWnd, LPCANVASSIZEINFO lpCanvasInfo, GXBOOL bEnd)
//{
//#ifdef ENABLE_AERO
//  GXLPSTATION lpStation = GXLPWND_STATION_PTR(lpWnd);
//
//  if(bEnd == 0)
//  {
//#ifdef ENABLE_DYNMAIC_EFFECT
//    GXRECT rcDest;
//    gxSetRect(
//      &rcDest, 
//      0,
//      0,
//      (GXINT)((GXFLOAT)g_SystemMetrics[SM_CXSCREEN] / c_flScaleWidth),
//      (GXINT)((GXFLOAT)g_SystemMetrics[SM_CYSCREEN] / c_flScaleHeight)
//      );
//    V(g_pd3dDevice->StretchRect(g_pGraphics->GetOriginalSrc(), NULL, 
//      GXLPWND_STATION_PTR(lpWnd)->pBackDownSampSur, (RECT*)&rcDest, GXTEXF_LINEAR));
//#else
//    GXRECT rcDest;
//    GXRECT rcSrc;
//    gxSetRect(
//      &rcSrc, 
//      (lpCanvasInfo->rgDest.left + 1 - 8) & (~1),
//      (lpCanvasInfo->rgDest.top  + 1 - 8) & (~1),
//      (lpCanvasInfo->rgDest.left + lpCanvasInfo->rgDest.width  + 1 + 8) & (~1),
//      (lpCanvasInfo->rgDest.top  + lpCanvasInfo->rgDest.height + 1 + 8) & (~1)
//      );
//    clClamp((GXLONG)0, (GXLONG)g_SystemMetrics[SM_CXSCREEN], &rcSrc.left  );
//    clClamp((GXLONG)0, (GXLONG)g_SystemMetrics[SM_CYSCREEN], &rcSrc.top   );
//    clClamp((GXLONG)0, (GXLONG)g_SystemMetrics[SM_CXSCREEN], &rcSrc.right );
//    clClamp((GXLONG)0, (GXLONG)g_SystemMetrics[SM_CYSCREEN], &rcSrc.bottom);
//    rcDest = rcSrc;
//    rcDest.left   = (GXLONG)(rcDest.left   / c_flScaleWidth );
//    rcDest.top    = (GXLONG)(rcDest.top    / c_flScaleHeight);
//    rcDest.right  = (GXLONG)(rcDest.right  / c_flScaleWidth );
//    rcDest.bottom = (GXLONG)(rcDest.bottom / c_flScaleHeight);
//
//    if(!gxIsRectEmpty(&rcSrc) && !gxIsRectEmpty(&rcDest))
//    {
//      lpCanvasInfo->pCanvas->Flush();
//      lpStation->pBackDownSampTexA->StretchRect(lpStation->pGraphics->GetDeviceOriginTex(), 
//        &rcDest, &rcSrc, GXTEXFILTER_LINEAR);
//      lpStation->pGraphics->SetTexture(lpStation->pBackDownSampTexA, 1);
//    }
//#endif // ENABLE_DYNMAIC_EFFECT
//
//    lpStation->pGraphics->SetPixelShaderConstantF(FXPCOMMREGIDX(PixelSize), 
//        (float*)&float4(
//          (float)(c_flScaleWidth /g_SystemMetrics[SM_CXSCREEN]),
//          (float)(c_flScaleHeight/g_SystemMetrics[SM_CYSCREEN]),0,0
//        ), 1);
//    lpCanvasInfo->pEffect = (GXEffect*)lpStation->m_pStockObject->pAeroEffect;
//  }
//  else
//  {
//    lpStation->pGraphics->SetTexture(lpStation->pBackDownSampTexA, 0);
//  }
//#endif  // </ENABLE_AERO>
//  return GX_OK;
//}
//GXLRESULT DEFWNDPROC_RenderCanvas(GXLPWND lpWnd, LPCANVASSIZEINFO lpCanvasInfo)
//{
//  GXPOINT ptCurrent;
//  GXCanvas* pCanvas = lpCanvasInfo->pCanvas;
//
//#ifdef ENABLE_DYNMAIC_EFFECT
//  if(lpWnd == GXWND_PTR(g_hSensorWnd))
//  {
//    gxGetCursorPos(&ptCurrent);
//
//    lpCanvasInfo->rgDest.width  = lpWnd->rectWindow.right  - lpWnd->rectWindow.left;
//    lpCanvasInfo->rgDest.height = lpWnd->rectWindow.bottom - lpWnd->rectWindow.top;
//
//    lpCanvasInfo->rgDest.left = g_ptSensorWnd.x + (ptCurrent.x - g_ptSensorCursor.x);
//    lpCanvasInfo->rgDest.top  = g_ptSensorWnd.y + (ptCurrent.y - g_ptSensorCursor.y);
//  }
//  if(//(CGXFrame::s_pMouseFocusFrame == pFrame /*&& CGXFrame::s_emSysCommand == SCA_MOVING*/) && (/*CGXFrame::s_emSysCommand != SCA_MOVING && */
//    lpWnd->m_pCanvas->pKGrid->IsActive())
//  {
//    lpWnd->m_pCanvas->pKGrid->Update();
//    g_pd3dDevice->SetTexture(0, lpWnd->m_pCanvas->m_pRenderTar->GetTexture());
//    g_pd3dDevice->SetStreamSource( 0, lpWnd->m_pCanvas->pKGrid->GetVerticesBuffer(), 0, sizeof(POS_TEXCOORD_COLOR) );
//    g_pd3dDevice->SetIndices(lpWnd->m_pCanvas->pKGrid->GetIndicesBuffer());
//    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, FACE_COUNT * FACE_COUNT, 0, FACE_COUNT * FACE_COUNT * 2);
//  }
//  else
//  {
//    gxGetOverlappedRect(&lpCanvasInfo->rcRenderingSrc, &lpCanvasInfo->rcRenderingSrc);
//    gxRectToRegn((GXLPREGN)&lpCanvasInfo->rcRenderingSrc, &lpCanvasInfo->rcRenderingSrc);
//    g_pGraphics->DrawTexture(lpWnd->m_pCanvas->m_pRenderTar->GetTexture(), &lpCanvasInfo->rgDest, (LPREGN)&lpCanvasInfo->rcRenderingSrc);
//  }
//#else  // </ENABLE_DYNMAIC_EFFECT>
//#ifdef _DEBUG
//  if(lpWnd == GXWND_PTR(g_hSensorWnd))
//  {
//    gxGetCursorPos(&ptCurrent);
//
//    lpCanvasInfo->rgDest.width  = lpWnd->rectWindow.right  - lpWnd->rectWindow.left;
//    lpCanvasInfo->rgDest.height = lpWnd->rectWindow.bottom - lpWnd->rectWindow.top;
//
//    lpCanvasInfo->rgDest.left = g_ptSensorWnd.x + (ptCurrent.x - g_ptSensorCursor.x);
//    lpCanvasInfo->rgDest.top  = g_ptSensorWnd.y + (ptCurrent.y - g_ptSensorCursor.y);
//  }
//#endif
//  pCanvas->SetEffect(lpCanvasInfo->pEffect);
//  GXGetOverlappedRect(&lpCanvasInfo->rcRenderingSrc, &lpCanvasInfo->rcRenderingSrc);
//  gxRectToRegn((GXLPREGN)&lpCanvasInfo->rcRenderingSrc, &lpCanvasInfo->rcRenderingSrc);
//  pCanvas->DrawTexture(lpWnd->m_pWinsSurface->m_pRenderTar->GetTextureUnsafe(), &lpCanvasInfo->rgDest, (LPREGN)&lpCanvasInfo->rcRenderingSrc);
//
//#endif  // </ENABLE_DYNMAIC_EFFECT>
//  if(lpWnd == GXWND_PTR(g_hSensorWnd) && g_hSensorCursor)
//  {
//    GXLPICON pIcon = GXICON_PTR(g_hSensorCursor);
//    GXREGN rgDest = {
//      ptCurrent.x - pIcon->xHotspot, 
//      ptCurrent.y - pIcon->yHotspot, 
//      pIcon->pImgIcon->GetWidth(), 
//      pIcon->pImgIcon->GetHeight()
//    };
//    pCanvas->DrawTexture(pIcon->pImgIcon->GetTextureUnsafe(), &rgDest);
//  }
//  return GX_OK;     
//}

//GXLRESULT DEFWNDPROC_FlushCanvas(GXHWND hWnd, GXCanvas* pCanvas)
//{
//  CANVASSIZEINFO CanvasSizeInfo;
//  CanvasSizeInfo.pCanvas = pCanvas;
//
//  gxSendMessageW(hWnd, GXWM_CALCCANVASSIZE    , NULL  , (GXLPARAM)&CanvasSizeInfo);
//  gxSendMessageW(hWnd, GXWM_SETRENDERCANVASSTATE  , 0    , (GXLPARAM)&CanvasSizeInfo);
//  gxSendMessageW(hWnd, GXWM_RENDERCANVAS      , NULL  , (GXLPARAM)&CanvasSizeInfo);
//  gxSendMessageW(hWnd, GXWM_SETRENDERCANVASSTATE  , 1    , (GXLPARAM)&CanvasSizeInfo);
//  return GX_OK;
//}

GXLRESULT GXDLLAPI gxDefWindowProcW(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  //GXINT xPos, yPos;
  GXLPWND lpWnd = GXWND_PTR(hWnd);
  switch(message)
  {
  case GXWM_PAINT:
    {
      GXPAINTSTRUCT ps;
      gxBeginPaint(hWnd, &ps);
      gxEndPaint(hWnd, &ps);
    }
    return 0;
  case GXWM_SETCURSOR:
    return DEFWNDPROC_SetCursor(hWnd, (GXHWND)wParam, lParam);
  case GXWM_SETTEXT:
    return DEFWNDPROC_SetText(hWnd, (GXLPCWSTR)lParam);
  case GXWM_GETTEXT:
    return DEFWNDPROC_GetText(hWnd, (GXLPWSTR)lParam, (int)wParam);
  case GXWM_GETTEXTLENGTH:
    return DEFWNDPROC_GetTextLength(hWnd);

  case GXWM_CTLCOLORSTATIC:
    {
      //MSDN: The WM_CTLCOLORSTATIC message is sent to the parent window of a static 
      //      control when the control is about to be drawn. By responding to this message, 
      //      the parent window can use the given device context handle to set the text and
      //      background colors of the static control. 
      GXHDC hdc = (GXHDC)wParam;
      GXHBRUSH hBrush = gxGetCtrlBrush(CTLBRUSH_STATIC);
      GXLOGBRUSH sLogBrush;
      gxGetObject(hBrush, sizeof(sLogBrush), &sLogBrush);
      GXSetBkColor(hdc, sLogBrush.lbColor);
      return (GXLRESULT)hBrush;
    }
  case GXWM_CTLCOLORLISTBOX:
    return (GXLRESULT)gxGetCtrlBrush(CTLBRUSH_LISTBOX);
  case GXWM_CTLCOLOREDIT:
    return (GXLRESULT)gxGetCtrlBrush(CTLBRUSH_EDIT);
  case GXWM_NCPAINT:
    return DEFWNDPROC_NcPaint(lpWnd, (GXHRGN)wParam);
  case GXWM_ERASEBKGND:
    return DEFWNDPROC_EraseBkGnd(hWnd, ((GXHDC)wParam));
  case GXWM_NCHITTEST:
    //UNPACKPARAM(xPos, yPos, lParam);
    return DEFWNDPROC_NcHitTest(hWnd, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));
  case GXWM_NCLBUTTONDOWN:
    //UNPACKPARAM(xPos, yPos, lParam);
    return DEFWNDPROC_NcLButtonDown(hWnd, (GXUINT)wParam, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));
  case GXWM_RBUTTONUP:
    return gxSendMessageW(hWnd, GXWM_CONTEXTMENU, (GXWPARAM)hWnd, lParam);
  case GXWM_SYSCOMMAND:
    //UNPACKPARAM(xPos, yPos, lParam);
    return DEFWNDPROC_SysCommand(hWnd, (GXUINT)wParam, GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam));
  case GXWM_NCCREATE:
    gxOpenThemeData(hWnd, L"WINDOW");
    return TRUE;
  case GXWM_CREATE:
  case GXWM_SIZE:
    break;

  //case GXWM_FLUSHCANVAS:
  //  return DEFWNDPROC_FlushCanvas(hWnd, (GXCanvas*)lParam);
  //case GXWM_CALCCANVASSIZE:
  //  return DEFWNDPROC_CalcCanvseSize(lpWnd, (LPCANVASSIZEINFO)lParam);
  //case GXWM_SETRENDERCANVASSTATE:
  //  return DEFWNDPROC_SetRenderCanvasState(lpWnd, (LPCANVASSIZEINFO)lParam, (GXBOOL)wParam);
  //case GXWM_RENDERCANVAS:
  //  return DEFWNDPROC_RenderCanvas(lpWnd, (LPCANVASSIZEINFO)lParam);

  case GXWM_NCDESTROY:
    {
      GXHTHEME hTheme = gxGetWindowTheme(hWnd);
      gxCloseThemeData (hTheme);
    }
    break;
  case GXWM_DISPLAYCHANGE:
    //return DEFWNDPROC_DisplayChange(lpWnd, (GXINT)wParam, lParam);
    return 0L;
  case GXWM_DATAPOOLOPERATION:
    return -1;
  case GXWM_NOTIFYFORMAT:
    {
      switch (lParam)
      {
      case GXNF_QUERY:
        return GXNFR_UNICODE;

      case GXNF_REQUERY:
        return gxSendMessageW ((GXHWND)wParam, GXWM_NOTIFYFORMAT,
          (GXWPARAM)hWnd, (GXLPARAM)GXNF_QUERY);
      }
      return 0;
    }
  //case GXWM_RESETD3DDEVICE:
  //case GXWM_LOSTD3DDEVICE:
  //  return DEFWNDPROC_LostResetDevice(message, lpWnd, (GXINT)wParam, lParam);
    //default:
    //  ASSERT(FALSE);
  }
  return 0;
}
GXLRESULT GXDLLAPI gxDefWindowProcA(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  ASSERT(FALSE);
  return 0;
}
//////////////////////////////////////////////////////////////////////////



#endif // _DEV_DISABLE_UI_CODE