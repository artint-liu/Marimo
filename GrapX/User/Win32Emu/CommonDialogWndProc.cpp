// ȫ��ͷ�ļ�
#include <GrapX.H>
#include <User/GrapX.Hxx>

//#include <Smart/smartstream.h>
//#include <Smart/SmartProfile.h>

// ƽ̨���
#include "Include/GUnknown.H"
#include "Include/GResource.H"
#include "Include/DataPool.H"
#include "Include/DataPoolVariable.H"
//#include "Include/DataInfrastructure.H"
//#include "clstd/clPathFile.H"

// ˽��ͷ�ļ�
#include "User/GXWindow.h"
#include "Include/GXUser.H"
//#include "Include/GXGDI.H"
//#include "Include/GXKernel.H"
//#include "Controls/GXUICtrlBase.h"
//#include "Controls/GXUIStatic.h"
//#include "Controls/GXUIButton.h"
//#include "Controls/GXUIToolbar.h"
//#include "Controls/GXUIList.h"
//#include "Controls/GXUISlider.h"
//#include "Controls/PropertySheet.h"
//#include "Controls/PropertyList.h"
#include "User/UILayoutMgr.h"
//#include "gxDevice.H"

GXLRESULT GXCALLBACK CommDialogWndProcEx(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  GXDLGPROC lpDlgProc = NULL;

  if(message == GXWM_NCCREATE) {
    const GXCREATESTRUCTW cs = *(GXLPCREATESTRUCTW)lParam;
    LPDLGLOG pDlgLog = (DLGLOG*)cs.lpCreateParams;
    gxSetWindowLongW(hWnd, GXDWL_DLGPROC, (GXLONG_PTR)pDlgLog->pDlgProc);
    gxSetWindowLongW(hWnd, GXDWL_DLGLOG, (GXLONG_PTR)pDlgLog);
    gxSendMessage(hWnd, GXWM_SETCOBJECT, NULL, (GXLPARAM)pDlgLog->lParam);
    pDlgLog = NULL;
  }
  else if(message != GXWM_CREATE) {
    //
    lpDlgProc = (GXDLGPROC)gxGetWindowLong(hWnd, GXDWL_DLGPROC);
  }

  switch(message)
  {
  case GXWM_NCDESTROY:
    {
      if(lpDlgProc != NULL) {
        lpDlgProc(hWnd, message, wParam, lParam);
      }

      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      SAFE_DELETE(pDlgLog);
      gxSetWindowLongW(hWnd, GXDWL_DLGLOG, NULL);
      break;
    }

  case GXWM_GETIDNAMEW:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      return pDlgLog ? (GXLRESULT)(GXLPCWSTR)pDlgLog->strName : NULL;
    }

    // ����GXDWL_MSGRESULT, ֱ�Ӱ�Dlg�������ķ���ֵ����Wnd�ķ���ֵ
  case GXWM_ERASEBKGND:
    if(lpDlgProc != NULL) {
      return lpDlgProc(hWnd, message, wParam, lParam);
    }
    break;
  case GXWM_SETCURSOR:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        if(pDlgLog->pLayout->OnSetCursor(GXLOWORD(lParam)))
          break;
      }
      return gxDefWindowProc(hWnd, message, wParam, lParam);
    }
    break;
  case GXWM_LBUTTONDOWN:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        GXPOINT pt = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        pDlgLog->pLayout->OnLButtonDown(wParam, &pt);
      }
    }
    break;
  case GXWM_LBUTTONUP:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        GXPOINT pt = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        pDlgLog->pLayout->OnLButtonUp(wParam, &pt);
      }
    }
    break;
  case GXWM_MOUSEMOVE:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        GXPOINT pt = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        pDlgLog->pLayout->OnMouseMove(wParam, &pt);
      }
    }
    break;
  case GXWM_NCHITTEST:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        GXPOINT pt = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        pDlgLog->pLayout->OnHitTest(&pt);// TODO: ��������falseҲ�ĵ���pDlgLog->pDlgProc,�Ժ��һ��,����pDlgLog->pLayout��������
      }

      //if( ! pDlgLog->pDlgProc(hWnd, message, wParam, lParam)) {
      //  return gxDefWindowProc(hWnd, message, wParam, lParam);
      //}
      //return gxSetWindowLongW(hWnd, GXDWL_MSGRESULT, 0);
      goto RET_FROM_MSGRESULT;
    }
    break;
  case GXWM_SIZE:
    {
      LPDLGLOG pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
      if(pDlgLog->pLayout != NULL) {
        GXSIZE size = {GXGET_X_LPARAM(lParam), GXGET_Y_LPARAM(lParam)};
        pDlgLog->pLayout->OnSize((GXDWORD)wParam, size);
      }
    }
    break;
  default:
    goto RET_FROM_MSGRESULT;
  }
  return 0;

RET_FROM_MSGRESULT:
  if(lpDlgProc != NULL) {
    const GXLRESULT lval = lpDlgProc(hWnd, message, wParam, lParam);
    if(lval != 0) {
      return gxSetWindowLongW(hWnd, GXDWL_MSGRESULT, 0);
    }
    //return lval; // ���ܵ�������
  }
  return gxDefWindowProc(hWnd, message, wParam, lParam);
}

GXLRESULT GXCALLBACK CommDialogWndProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  //if(message == GXWM_NCCREATE) {
  //  const GXCREATESTRUCTW cs = *(LPGXCREATESTRUCTW)lParam;
  //  pDlgLog = (DLGLOG*)cs.lpCreateParams;
  //  gxSetWindowLongW(hWnd, GXDWL_DLGPROC, (GXLONG_PTR)pDlgLog->pDlgProc);
  //  gxSetWindowLongW(hWnd, GXDWL_DLGLOG, (GXLONG_PTR)pDlgLog);
  //  pDlgLog = NULL;
  //}
  //else {
  //  pDlgLog = (DLGLOG*)gxGetWindowLong(hWnd, GXDWL_DLGLOG);
  //}
  if(message == GXWM_NCCREATE)
  {
    GXCREATESTRUCTW cs = *(GXLPCREATESTRUCTW)lParam;
    LPDLGLOG pDlgLog = new DLGLOG;
    GXLPARAM *aParam = (GXLPARAM*)cs.lpCreateParams;
    pDlgLog->cbSize = sizeof(DLGLOG);
    pDlgLog->pDlgProc = (GXDLGPROC)aParam[1];
    cs.lpCreateParams = pDlgLog;
    return CommDialogWndProcEx(hWnd, message, wParam, (GXLPARAM)&cs);
  }
  return CommDialogWndProcEx(hWnd, message, wParam, lParam);

  //ASSERT(0); // TODO: �޸��˶Ի������ݴ���, ���û�и����޸�, �޸���ȥ��!
  //if(message == GXWM_NCCREATE)
  //{
  //  GXCREATESTRUCTW cs = *(LPGXCREATESTRUCTW)lParam;
  //  GXLPARAM *aParam = (GXLPARAM*)cs.lpCreateParams;
  //  cs.lpCreateParams = (GXLPVOID)aParam[0];  // [�û�����][�Ի�������]
  //  gxSetWindowLongW(hWnd, GXDWL_DLGPROC, aParam[1]);

  //  return TRUE;
  //}
  // else
  //if(lpWndProc)
  //  return gxCallWindowProcW(lpWndProc, hWnd, message, wParam, lParam);
  //return 0L;
}