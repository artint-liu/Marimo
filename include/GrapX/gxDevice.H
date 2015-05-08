#ifndef _GRAPX_DEVICE_H_
#define _GRAPX_DEVICE_H_
class GXGraphics;
class GXWnd;
class IGXPlatform;

// TODO: 暂时不用
//struct INPUTDEVICESPARAM
//{
//  enum MouseKey
//  {
//    GXMK_CONTROL = MK_CONTROL,
//    GXMK_LBUTTON = MK_LBUTTON,
//    GXMK_MBUTTON = MK_MBUTTON,
//    GXMK_RBUTTON = MK_RBUTTON,
//    GXMK_SHIFT   = MK_SHIFT,
//  };
//  GXWORD  dwKey;
//  GXDWORD  dwMoustBtn;
//  GXPOINT ptCursor;
//};
//
//typedef GXLRESULT (GXCALLBACK* GXMOUSEMOVE)(GXHWND, GXUINT, GXWPARAM, GXLPARAM);

extern "C"
{
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXBOOL          GXDLLAPI GXUICreateStation      (HWND hWnd, IGXPlatform* lpPlatform);
#ifndef _DEV_DISABLE_UI_CODE
  GXBOOL          GXDLLAPI GXUIPostRootMessage    (GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam);
  GXLRESULT       GXDLLAPI GXUISetCursor          (GXWPARAM wParam, GXLPARAM lParam);
#endif // _DEV_DISABLE_UI_CODE
#else
  GXBOOL          GXDLLAPI GXUICreateStation      (IGXPlatform* lpPlatform);
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXVOID          GXDLLAPI GXUIDestroyStation     ();
  GXBOOL          GXDLLAPI GXUIResizeStation      (int nWidth, int nHeight);
  //GXBOOL          GXDLLAPI GXUITestStation        ();
  GXVOID          GXDLLAPI GXUIUpdateTimerEvent   ();  // TODO: 感觉这个要整合封装起来
  LPSTOCKOBJECT   GXDLLAPI GXUIGetStock           ();
  GXBOOL          GXDLLAPI GXUISwitchConsole      ();
  GXBOOL          GXDLLAPI GXUIMakeCurrent        ();

  GXHRESULT       GXDLLAPI GXRenderRootFrame      ();
  GXVOID          GXDLLAPI GXDrawDebugMsg         (GXHSTATION hStation, GXCanvas* pCanvas);
};

#endif // _GRAPX_DEVICE_H_