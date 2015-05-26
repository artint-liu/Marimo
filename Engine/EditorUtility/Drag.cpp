#include "GrapX.H"

#include <GrapX/GXUser.H>

//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GShader.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXCanvas3D.h"

#include "GrapX/GrapVR.H"
#include "Engine.h"
#include "Engine/Drag.h"

namespace EditorUtility
{
  void GAMEENGINE_API GenerateRectHandles(GXLPCRECT lprc, int nHandleSize, GXLPREGN rgHandles)
  {
    int& s = nHandleSize;
    gxSetRegn(rgHandles + 0, lprc->left - s, lprc->top - s, s, s);
    gxSetRegn(rgHandles + 1, (lprc->left + lprc->right - s) >> 1, lprc->top - s, s, s);
    gxSetRegn(rgHandles + 2, lprc->right, lprc->top - s, s, s);
    gxSetRegn(rgHandles + 3, lprc->left - s, (lprc->top + lprc->bottom - s) >> 1, s, s);
    gxSetRegn(rgHandles + 4, lprc->right, (lprc->top + lprc->bottom - s) >> 1, s, s);
    gxSetRegn(rgHandles + 5, lprc->left - s, lprc->bottom, s, s);
    gxSetRegn(rgHandles + 6, (lprc->left + lprc->right - s) >> 1, lprc->bottom, s, s);
    gxSetRegn(rgHandles + 7, lprc->right, lprc->bottom, s, s);
  }

  int GAMEENGINE_API HitTestRectHandles(GXLPCREGN rgHandles, GXLPCPOINT pt)
  {
    for(int i = 0; i < 8; i++)
    {
      if(gxPtInRegn(&rgHandles[i], *pt)) {
        return i;
      }
    }
    return -1;
  }


  GXLRESULT TrackDragAction(GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin, DragProc lpDragFunc, GXLPARAM lParam)
  {
    GXMSG msg;

    // ptOrigin参数和拖拽修改的值如果指向同一地址会有问题
    // 这里要形成内存副本，防止指针所指向的值改变
    GXPOINT ptCursorCopy;
    GXPOINT ptOriginCopy;

    if(ptCursor) {
      ptCursorCopy = *ptCursor;
      ptCursor = &ptCursorCopy;
    }

    if(ptOrigin) {
      ptOriginCopy = *ptOrigin;
      ptOrigin = &ptOriginCopy;
    }

    while(gxGetMessage(&msg, NULL))
    {
      if(msg.message == GXWM_KEYDOWN && msg.wParam == VK_ESCAPE) {
        break;
      }
      else if(msg.message == GXWM_MOUSEMOVE)
      {
        GXPOINT ptDelta = {GXGET_X_LPARAM(msg.lParam) - ptCursorCopy.x,
          GXGET_Y_LPARAM(msg.lParam) - ptCursorCopy.y};
        GXBOOL bret = lpDragFunc(&ptDelta, ptOrigin, lParam);
        if( ! bret) {
          return -1;
        }
      }
      else if(msg.message == GXWM_LBUTTONUP || msg.message == GXWM_RBUTTONUP) {
        return 1;
      }
      else if(msg.message > GXWM_MOUSEFIRST && msg.message <= GXWM_MOUSELAST) {
        break;
      }
      gxTranslateMessage(&msg);
      gxDispatchMessageW(&msg);
    }
    return 0;
  }

  GXLRESULT CDrag::Track( GXLPCPOINT ptCursor, GXLPCPOINT ptOrigin )
  {
    GXMSG msg;

    // ptOrigin参数和拖拽修改的值如果指向同一地址会有问题
    // 这里要形成内存副本，防止指针所指向的值改变
    GXPOINT ptCursorCopy;
    GXPOINT ptOriginCopy;

    if(ptCursor) {
      ptCursorCopy = *ptCursor;
      //ptCursor = &ptCursorCopy;
    }
    else {
      ptCursorCopy.x = 0;
      ptCursorCopy.y = 0;
    }

    if(ptOrigin) {
      ptOriginCopy = *ptOrigin;
      ptOrigin = &ptOriginCopy;
    }

    while(gxGetMessage(&msg, NULL))
    {
      if(msg.message == GXWM_KEYDOWN && msg.wParam == VK_ESCAPE) {
        break;
      }
      else if(msg.message == GXWM_MOUSEMOVE)
      {
        GXPOINT ptDelta = {GXGET_X_LPARAM(msg.lParam) - ptCursorCopy.x,
          GXGET_Y_LPARAM(msg.lParam) - ptCursorCopy.y};
        GXBOOL bret = OnDrag(&ptDelta, ptOrigin);
        if( ! bret) {
          return -1;
        }
      }
      else if(msg.message == GXWM_LBUTTONUP || msg.message == GXWM_RBUTTONUP) {
        return 1;
      }
      else if(msg.message > GXWM_MOUSEFIRST && msg.message <= GXWM_MOUSELAST) {
        break;
      }
      gxTranslateMessage(&msg);
      gxDispatchMessageW(&msg);
    }
    return 0;
  }

} // namespace EditorUtility