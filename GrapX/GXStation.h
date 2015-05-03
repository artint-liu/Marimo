#ifndef _GRAPX_STATION_
#define _GRAPX_STATION_

class GTexture;
class GXGraphics;
class CAeroShader;
class CBlurShader;
class CSimpleShader;


//extern GXLPSTATION g_pCurStation;

//////////////////////////////////////////////////////////////////////////
//
//    层次关系
//
//////////////////////////////////////////////////////////////////////////
//
//  GXStation
//  {
//    Window
//    D3DDevice(D3DGraphics, 唯一,或者多个Station共享)
//    RootGXWindow
//    GXWindow(KeyFocus)
//    GXWindow(MouseFocus)
//    GXWindow(Capture)
//    GXInstance
//    {
//      GXWindow-s
//      {
//        GXWndCanvas
//        GXGDI
//      }
//    }
//    GXWindow(Prev KeyFocus)
//    GXWindow(Prev MouseFocus)
//    GXWindow(Prev Capture)
//  }
//  g_pCurStation



#endif // _GRAPX_STATION_