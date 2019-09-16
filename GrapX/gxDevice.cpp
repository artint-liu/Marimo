// 全局头文件
#include <GrapX.h>
#include "GXApp.h"
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXRenderTarget.h"
#include "GrapX/GRegion.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXUser.h"
#include "GrapX/GXKernel.h"
#include "GrapX/Platform.h"

// 私有头文件
#include "GXStation.h"
#include <User/GXWindow.h>
#include "GrapX/gUxtheme.h"
#include <User/Win32Emu/_dpa.h>
#include <User/DesktopWindowsMgr.h>
#include <Utility/LogMgr.h>
//#include <Utility/Shader/AeroShader.h>
//#include <Utility/Shader/BlurShader.h>
//#include <Utility/Shader/SimpleShader.h>
//#include <clMessageThread.h>
#include "GrapX/gxDevice.h"
//#include "thread/clMessageThread.h"
//#include "User/gxMessage.hxx"
//#include "clSchedule.h"

GXLPSTATION g_pCurStation;
static GXINSTANCE* g_pInstDll;

namespace D3D9
{
  extern const char* g_szFastGaussianBlur;
} // namespace D3D9

#if defined(_WIN32) || defined(_WINDOWS)
HINSTANCE g_hDLLModule = NULL;
//HGLOBAL   g_hGlbRects = NULL;
#else
//GXHANDLE g_hGlbRects = NULL;
#endif // defined(_WIN32) || defined(_WINDOWS)

GXLPRECT g_lpRects = NULL;
const GXUINT g_uRectCapacity = 512;

GXBOOL GXUSER_InitializeCtrlRegMap();
GXVOID GXUSER_ReleaseCtrlRegMap();
GXVOID GXUSER_InitializeWndProperty();
GXVOID GXUSER_ReleaseWndProperty();
static GXBOOL CreateStockObject(GXLPSTATION lpStation);
static GXBOOL DestroyStockObject(GXLPSTATION lpStation);
GXLRESULT  GXDLLAPI gxSendMessageW(GXHWND hWnd,GXUINT Msg,GXWPARAM wParam,GXLPARAM lParam);
extern GXHFONT g_hSystemFont;


GXINT_PTR GXCALLBACK ConsoleDlgProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  return 0;
}

//extern "C"
//{
//#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  GXBOOL GXDLLAPI GXUICreateStation(const GXCREATESTATION* lpCreateStation)
  {
    if(lpCreateStation == NULL || lpCreateStation->cbSize != sizeof(GXCREATESTATION)) {
      return FALSE;
    }

    g_pCurStation = new GXSTATION(lpCreateStation);
//#else
//  GXBOOL GXDLLAPI GXUICreateStation(IGXPlatform* lpPlatform)
//  {
//    g_pCurStation = new GXSTATION(lpPlatform);
//#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
  //{
    //GXGRAPHICSDEVICE_DESC GraphDesc;

    //g_pCurStation = new GXSTATION();

    //g_pCurStation->dwMagic              = GXSTATION_MAGIC;
    //g_pCurStation->m_dwFlags            = GXST_DRAWDEBUGMSG;
#ifdef _WIN32
    //g_pCurStation->hInstApp             = (HINSTANCE)(GXLONG _w64)GetWindowLong(hWnd, GXGWL_HINSTANCE);
    //g_pCurStation->hBindWin32Wnd        = hWnd;

    //g_pCurStation->pInstApp = new GXINSTANCE;
    //g_pCurStation->pInstApp->Initialize(g_pCurStation, (HINSTANCE)(GXLONG _w64)GetWindowLong(hWnd, GXGWL_HINSTANCE));
#endif // _WIN32
    //g_pCurStation->lpPlatform           = lpPlatform;
    //g_pCurStation->dwUIThreadId         = NULL;
    //g_pCurStation->pGraphics            = lpPlatform->m_pApp->GetGraphicsUnsafe();
    
    //g_pCurStation->pGraphics->GetDesc(&GraphDesc);

    //g_pCurStation->nWidth                     = GraphDesc.BackBufferWidth;
    //g_pCurStation->nHeight                    = GraphDesc.BackBufferHeight;
    //g_pCurStation->MonitorInfo.rcMonitor.left   = 0;
    //g_pCurStation->MonitorInfo.rcMonitor.top    = 0;
    //g_pCurStation->MonitorInfo.rcMonitor.right  = GraphDesc.BackBufferWidth;
    //g_pCurStation->MonitorInfo.rcMonitor.bottom = GraphDesc.BackBufferHeight;
    //g_pCurStation->MonitorInfo.rcWork = g_pCurStation->MonitorInfo.rcMonitor;
//#ifndef _DEV_DISABLE_UI_CODE
    //g_pCurStation->lpDesktopWnd          = new GXWnd;
    //g_pCurStation->hClassDPA            = gxDPA_Create(8);
    //g_pCurStation->m_pDesktopWindowsMgr = new DesktopWindowsMgr(g_pCurStation);
//#endif // _DEV_DISABLE_UI_CODE
    //g_pCurStation->m_ptCursor.x         = 0;
    //g_pCurStation->m_ptCursor.y         = 0;
    //g_pCurStation->m_uFrameCount        = 0;
    //g_pCurStation->m_pRichFXMgr         = NULL;

    //InlSetZeroT(g_pCurStation->m_HotKeyChain);

    g_pCurStation->pInstDll = g_pInstDll;
    g_pCurStation->Initialize();

    //g_Instance.lpStation = g_pCurStation;

    if(lpCreateStation->lpPlatform)
    {
#ifndef _DEV_DISABLE_UI_CODE
      GXWin32APIEmu::SetScreen(g_pCurStation->nWidth, g_pCurStation->nHeight);
      GXWin32APIEmu::InitializeStatic();
      GXUSER_InitializeWndProperty();

//#if defined(_WIN32) || defined(_WINDOWS)
//      if(g_hGlbRects == NULL)
//        g_hGlbRects = GlobalAlloc(GMEM_MOVEABLE, sizeof(GXRECT) * g_uRectCapacity);
//#else
//      if (g_hGlbRects == NULL) 
//        g_hGlbRects = (GXHANDLE)new GXRECT[g_uRectCapacity];
//#endif // defined(_WIN32) || defined(_WINDOWS)

#ifdef ENABLE_AERO

      g_pCurStation->pGraphics->CreateRenderTarget(&g_pCurStation->pBackDownSampTexA, NULL, GXSizeRatio::Half, GXSizeRatio::Half, Format_B8G8R8A8, Format_Unknown);
      g_pCurStation->pGraphics->CreateRenderTarget(&g_pCurStation->pBackDownSampTexB, NULL, GXSizeRatio::Half, GXSizeRatio::Half, Format_B8G8R8A8, Format_Unknown);

      ASSERT(g_pCurStation->pBackDownSampTexA && g_pCurStation->pBackDownSampTexB);
#endif // ENABLE_AERO

      GXWnd::CreateDesktop(g_pCurStation);
      CreateStockObject(g_pCurStation);
      GXUSER_InitializeCtrlRegMap();
      GXUXTHEME_Initialize(g_pCurStation->pGraphics);

      g_pCurStation->StartUserThread();
#endif // _DEV_DISABLE_UI_CODE
    }
    else {
      CLOG("create GXUI station without GXPlatform.");
    }

    //g_pCurStation->m_hConsole = gxCreateDialogParamW(NULL, L"@Resource\\console.dlg.txt", NULL, &ConsoleDlgProc, NULL);
    //gxShowWindow(g_pCurStation->m_hConsole, GXSW_HIDE);
    return TRUE;
  }

  GXVOID GXDLLAPI GXUIDestroyStation()
  {
    //gxDestroyWindow(g_pCurStation->m_hConsole);
#ifndef _DEV_DISABLE_UI_CODE
    
//#if defined(_WIN32) || defined(_WINDOWS)
//    if(g_hGlbRects != NULL)
//    {
//      GlobalFree(g_hGlbRects);
//      g_hGlbRects = NULL;
//    }
//#else
//    if (g_hGlbRects != NULL) 
//    {
//      delete (g_hGlbRects);
//      g_hGlbRects = NULL;
//    }
//#endif // defined(_WIN32) || defined(_WINDOWS)

    GXUXTHEME_Release();
    //GXDestroyRootFrame();
    
    DestroyStockObject(g_pCurStation);
    GXWnd::DestroyDesktop(g_pCurStation);

    SAFE_DELETE(g_pCurStation->m_pDesktopWindowsMgr);

    GXUSER_ReleaseCtrlRegMap();

    GXUSER_ReleaseWndProperty();
    GXWin32APIEmu::ReleaseStatic();
    //SAFE_DELETE(g_pCurStation->pRenderingLocker);
    g_pCurStation->Finalize();
#ifdef ENABLE_AERO
    SAFE_RELEASE(g_pCurStation->pBackDownSampTexA);
    SAFE_RELEASE(g_pCurStation->pBackDownSampTexB);
#endif // ENABLE_AERO
    
    if(g_pCurStation->pInstApp) {
      g_pCurStation->pInstApp->Finalize();
      delete g_pCurStation->pInstApp;
      g_pCurStation->pInstApp = NULL;
    }

    if(g_pCurStation->pInstDll)
    {
      g_pCurStation->pInstDll->Finalize();
      delete g_pCurStation->pInstDll;
      g_pCurStation->pInstDll = NULL;
      g_pInstDll = NULL;
    }

    ASSERT(g_pCurStation->m_aActiveWnds.size() == 0);
    LogMgr::Release();

    delete g_pCurStation;
    g_pCurStation = NULL;

    //MOUnregisterConsoleStaff()
#endif // _DEV_DISABLE_UI_CODE
  }

  GXBOOL GXDLLAPI GXUIResizeStation(int nWidth, int nHeight)
  {
#ifndef _DEV_DISABLE_UI_CODE
    GRegion* prgnBeforeSize = NULL;
    GRegion* prgnAfterSize = NULL;
    GXGRAPHICSDEVICE_DESC GraphDesc;

    TRACE("> g_pCurStation->pGraphics->GetDesc\n");
    g_pCurStation->pGraphics->GetDesc(&GraphDesc);
    if(GraphDesc.BackBufferWidth != nWidth || GraphDesc.BackBufferHeight != nHeight)
    {
      TRACE("> g_pCurStation->pGraphics->Enter()\n");
      g_pCurStation->pGraphics->Enter();
      g_pCurStation->pGraphics->CreateRectRgn(&prgnBeforeSize, 0, 0, 
        g_pCurStation->nWidth, g_pCurStation->nHeight);

      TRACE("> g_pCurStation->pGraphics->Resize\n");
      g_pCurStation->pGraphics->Resize(nWidth, nHeight);
      g_pCurStation->lpDesktopWnd->rectWindow.left   = 0;
      g_pCurStation->lpDesktopWnd->rectWindow.top    = 0;
      g_pCurStation->lpDesktopWnd->rectWindow.right  = nWidth;
      g_pCurStation->lpDesktopWnd->rectWindow.bottom = nHeight;
      g_pCurStation->nWidth = nWidth;
      g_pCurStation->nHeight = nHeight;
      g_pCurStation->MonitorInfo.cbSize = sizeof(GXMONITORINFO);
      g_pCurStation->MonitorInfo.dwFlags = GXMONITORINFOF_PRIMARY;
      g_pCurStation->MonitorInfo.rcMonitor.right = nWidth;
      g_pCurStation->MonitorInfo.rcMonitor.bottom = nHeight;
      g_pCurStation->MonitorInfo.rcWork = g_pCurStation->MonitorInfo.rcMonitor;
      GXWin32APIEmu::SetScreen(nWidth, nHeight);
      g_pCurStation->pGraphics->CreateRectRgn(&prgnAfterSize, 0, 0, 
        g_pCurStation->nWidth, g_pCurStation->nHeight);

      RGNCOMPLEX rc = prgnAfterSize->Subtract(prgnBeforeSize);
      if( rc == RC_SIMPLE || rc == RC_COMPLEX)
        g_pCurStation->m_pDesktopWindowsMgr->InvalidateWndRegion(NULL, prgnAfterSize, FALSE);
      SAFE_RELEASE(prgnBeforeSize);
      SAFE_RELEASE(prgnAfterSize);
      g_pCurStation->pGraphics->Leave();

      gxSendMessage(GXHWND_BROADCAST, GXWM_DISPLAYCHANGE, 32, GXMAKELPARAM(nWidth, nHeight));
      return TRUE;
    }
    return FALSE;
#else
    GXGRAPHICSDEVICE_DESC GraphDesc;
    
    g_pCurStation->pGraphics->GetDesc(&GraphDesc);
    if(GraphDesc.BackBufferWidth != nWidth || GraphDesc.BackBufferHeight != nHeight)
    {
      g_pCurStation->pGraphics->Resize(nWidth, nHeight);
      g_pCurStation->nWidth = nWidth;
      g_pCurStation->nHeight = nHeight;
      return TRUE;
    }
    return FALSE;
#endif // _DEV_DISABLE_UI_CODE
  }

  GXBOOL GXDLLAPI GXUIMakeCurrent()
  {
    g_pCurStation->dwUIThreadId = gxGetCurrentThreadId();
    return (g_pCurStation->dwUIThreadId != 0);
  }

  LPSTOCKOBJECT GXDLLAPI GXUIGetStock()
  {
    return g_pCurStation->m_pStockObject;
  }

  GXBOOL GXDLLAPI GXUISwitchConsole()
  {
    GXBOOL bval = TRUE;
    if(g_pCurStation->m_hConsole != NULL) {
      bval = gxIsWindowVisible(g_pCurStation->m_hConsole);
      GXDWORD dwCmd = bval ? GXSW_HIDE : GXSW_SHOW;
      gxShowWindow(g_pCurStation->m_hConsole, dwCmd);
    }
    return !bval;
  }
//#ifndef _DEV_DISABLE_UI_CODE
//#if defined(_WIN32) || defined(_WINDOWS)
//  GXLRESULT GXDLLAPI GXUISetCursor(GXHWND hWnd, GXWPARAM wParam, GXLPARAM lParam)
//  {
//    lParam &= 0xffff0000;
//    lParam |= (GXWnd::s_emCursorResult & 0x0000ffff);
//    return DefWindowProc((HWND)hWnd, GXWM_SETCURSOR, wParam, lParam);
//  }
//#endif // defined(_WIN32) || defined(_WINDOWS)
//#endif // _DEV_DISABLE_UI_CODE
  //////////////////////////////////////////////////////////////////////////
//};
GXBOOL CreateStockObject(GXLPSTATION lpStation)
{
  if(lpStation->m_pStockObject != NULL)
    return FALSE;
  if(lpStation->pGraphics == NULL)
    return FALSE;

  lpStation->m_pStockObject = new STOCKOBJECT;
  memset(lpStation->m_pStockObject, 0, sizeof(STOCKOBJECT));

  LPSTOCKOBJECT const lpStockObject = lpStation->m_pStockObject;
  GrapX::Graphics* pGraphics = lpStation->pGraphics;

  // TODO: 抽取资源
  pGraphics->CreateShaderFromFile(&lpStockObject->pAeroShader,   "shaders/Aero.shader.txt");
  pGraphics->CreateShaderFromFile(&lpStockObject->pBlurShader,   "shaders/Blur.shader.txt");
  pGraphics->CreateShaderFromFile(&lpStockObject->pSimpleShader, "shaders/Simple.shader.txt");

  lpStation->pGraphics->CreateEffect(&lpStockObject->pAeroEffect  , lpStockObject->pAeroShader);
  lpStation->pGraphics->CreateEffect(&lpStockObject->pBlurEffect  , lpStockObject->pBlurShader);
  lpStation->pGraphics->CreateEffect(&lpStockObject->pSimpleEffect, lpStockObject->pSimpleShader);
  GXPlatformIdentity PlatformId = lpStation->pGraphics->GetPlatformID();
  if(PlatformId == GXPLATFORM_X_OPENGLES2)
  {
    //lpStockObject->pAeroEffect->BindTextureSlot("s_baseMap", 0);
    //lpStockObject->pAeroEffect->BindTextureSlot("s_blurMap", 1);
  }
  else if(PlatformId == GXPLATFORM_WIN32_DIRECT3D9)
  {
    GXSHADER_SOURCE_DESC desc[2];
    desc[0].szSourceData  = D3D9::g_szFastGaussianBlur;
    desc[0].nSourceLen    = 0;
    desc[0].szEntry       = "vs_main";
    desc[0].szTarget      = "vs_3_0";
    desc[1].szSourceData  = D3D9::g_szFastGaussianBlur;
    desc[1].nSourceLen    = 0;
    desc[1].szEntry       = "ps_main";
    desc[1].szTarget      = "ps_3_0";
    pGraphics->CreateShaderFromSource(&lpStockObject->pFastGaussianBlurShader, desc, 2);
  }

  lpStation->pGraphics->CreateEffect(&lpStockObject->pFastGaussianBlurEffect, lpStockObject->pFastGaussianBlurShader);

  lpStockObject->pDefaultFont = lpStation->pGraphics->CreateFontW(0, 16, DEFAULT_FONT_NAMEW);

  GXDWORD white[8 * 8];
  memset(white, 0xff, sizeof(white));
  lpStation->pGraphics->CreateTexture(&lpStockObject->pWhiteTexture8x8, "White8x8", 8, 8, Format_B8G8R8A8, GXResUsage::Default, 1, white, 0);


  return TRUE;
}

GXBOOL DestroyStockObject(GXLPSTATION lpStation)
{
  LPSTOCKOBJECT const lpStockObject = lpStation->m_pStockObject;
  if( ! lpStockObject) {
    return FALSE;
  }

  SAFE_RELEASE(lpStockObject->pWhiteTexture8x8);
  SAFE_RELEASE(lpStockObject->pDefaultFont);

  SAFE_RELEASE(lpStockObject->pFastGaussianBlurEffect);
  SAFE_RELEASE(lpStockObject->pFastGaussianBlurShader);

  SAFE_RELEASE(lpStockObject->pAeroEffect);
  SAFE_RELEASE(lpStockObject->pBlurEffect);
  SAFE_RELEASE(lpStockObject->pSimpleEffect);

  SAFE_RELEASE(lpStockObject->pAeroShader);
  SAFE_RELEASE(lpStockObject->pBlurShader);
  SAFE_RELEASE(lpStockObject->pSimpleShader);

  SAFE_DELETE(lpStation->m_pStockObject);
  return TRUE;
}

//extern "C"
//{
  extern "C" GXVOID GXDLLAPI GXDrawDebugMsg(GXHSTATION hStation, GrapX::Canvas* pCanvas)
  {
#ifndef _DEV_DISABLE_UI_CODE
    //extern CD3DGraphics *g_pGraphics;
    //GXWCHAR buffer[1024];
    clStringW strInfo;

    GXLPSTATION lpStation = GXSTATION_PTR(hStation);
    GXFont* pFont = GXGDI_FONT_PTR(g_hSystemFont)->lpFont;
    clsize nCount = 0;
    //memset(buffer, 0, sizeof(GXWCHAR [1024]));

    //nCount = sprintf(buffer,L"Cursor Pos:%6d,%6d", lpStation->m_ptCursor.x, lpStation->m_ptCursor.y);
    nCount = strInfo.Format(_CLTEXT("Cursor pos:%6d,%6d"), lpStation->m_ptCursor.x, lpStation->m_ptCursor.y);
    pCanvas->TextOut(pFont, 6, 6, strInfo, (GXINT)nCount, 0xff000000);
    pCanvas->TextOut(pFont, 5, 5, strInfo, (GXINT)nCount, 0xffffffff);
    //nCount = wsprintfW(buffer,L"Mouse Over Frame: %08XH ", lpStation->m_pMouseFocus);
    nCount = strInfo.Format(_CLTEXT("Mouse over wnd: %08XH "), lpStation->m_pMouseFocus);
    if(lpStation->m_pMouseFocus != NULL && lpStation->m_pMouseFocus->m_pText > (GXWCHAR*)0xffff)
    {
      //lstrcatW(buffer, lpStation->m_pMouseFocus->m_pText);
      //nCount += lstrlenW(lpStation->m_pMouseFocus->m_pText);
      strInfo += lpStation->m_pMouseFocus->m_pText;
    }
    pCanvas->TextOut(pFont, 6, 26, strInfo, (GXINT)strInfo.GetLength(), 0xff000000);
    pCanvas->TextOut(pFont, 5, 25, strInfo, (GXINT)strInfo.GetLength(), 0xffffffff);
#endif // _DEV_DISABLE_UI_CODE
  }
/*
  GXBOOL GXDLLAPI GXEnsureDeviceCaps(LPDIRECT3D9 pD3D)
  {
    D3DCAPS9 d3dCaps;
    LogMgr::Initialize();

    if(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps) == D3DERR_NOTAVAILABLE)
    {
      MessageBoxW(NULL, L"桌面目前处于锁定状态，请稍候再试！",L"设备不可用", MB_OK);
      return FALSE;
    }
    GXDWORD dwCurMember;
#define SEL_CHECK_FLAG(MEMB)  dwCurMember = d3dCaps.MEMB;  TRACE("Check \"%s\"\n",#MEMB); LogMgr::Output(LogMgr::LC_D3D, "Check \"%s\"\r\n",#MEMB);
#define CHECK_FLAG(MASK)    if(dwCurMember & MASK) {TRACE("\t%s\n", #MASK); LogMgr::Output(LogMgr::LC_D3D, "\t%s\r\n", #MASK);}
#define CAPS_TRACE_VAL_F(VALF)  TRACE("%s = %f\n",#VALF, d3dCaps.VALF); LogMgr::Output(LogMgr::LC_D3D, "%s = %f\r\n",#VALF, d3dCaps.VALF);
#define CAPS_TRACE_VAL_I(VALI)  TRACE("%s = %d\n",#VALI, d3dCaps.VALI); LogMgr::Output(LogMgr::LC_D3D, "%s = %d\r\n",#VALI, d3dCaps.VALI);

    //DeviceType 
    //  D3DDEVTYPE_HAL 
    //AdapterOrdinal 
    LogMgr::Output(LogMgr::LC_D3D, "Device 3D Info\r\n");
    SEL_CHECK_FLAG(Caps);
    CHECK_FLAG(D3DCAPS_READ_SCANLINE);

    SEL_CHECK_FLAG(Caps2);
    CHECK_FLAG(D3DCAPS2_CANAUTOGENMIPMAP);
    CHECK_FLAG(D3DCAPS2_CANCALIBRATEGAMMA);
    CHECK_FLAG(D3DCAPS2_CANSHARERESOURCE);  //This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DCAPS2_CANMANAGERESOURCE);
    CHECK_FLAG(D3DCAPS2_DYNAMICTEXTURES);
    CHECK_FLAG(D3DCAPS2_FULLSCREENGAMMA);

    SEL_CHECK_FLAG(Caps3);
    CHECK_FLAG(D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD);
    CHECK_FLAG(D3DCAPS3_COPY_TO_VIDMEM);
    CHECK_FLAG(D3DCAPS3_COPY_TO_SYSTEMMEM);
    CHECK_FLAG(D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION);

    SEL_CHECK_FLAG(PresentationIntervals);
    CHECK_FLAG(D3DPRESENT_INTERVAL_IMMEDIATE);  
    CHECK_FLAG(D3DPRESENT_INTERVAL_ONE);
    CHECK_FLAG(D3DPRESENT_INTERVAL_TWO);
    CHECK_FLAG(D3DPRESENT_INTERVAL_THREE);
    CHECK_FLAG(D3DPRESENT_INTERVAL_FOUR);

    SEL_CHECK_FLAG(CursorCaps);
    CHECK_FLAG(D3DCURSORCAPS_COLOR);
    CHECK_FLAG(D3DCURSORCAPS_LOWRES);

    SEL_CHECK_FLAG(DevCaps);
    CHECK_FLAG(D3DDEVCAPS_CANBLTSYSTONONLOCAL);
    CHECK_FLAG(D3DDEVCAPS_CANRENDERAFTERFLIP); 
    CHECK_FLAG(D3DDEVCAPS_DRAWPRIMITIVES2); 
    CHECK_FLAG(D3DDEVCAPS_DRAWPRIMITIVES2EX); 
    CHECK_FLAG(D3DDEVCAPS_DRAWPRIMTLVERTEX); 
    CHECK_FLAG(D3DDEVCAPS_EXECUTESYSTEMMEMORY); 
    CHECK_FLAG(D3DDEVCAPS_EXECUTEVIDEOMEMORY); 
    CHECK_FLAG(D3DDEVCAPS_HWRASTERIZATION); 
    CHECK_FLAG(D3DDEVCAPS_HWTRANSFORMANDLIGHT); 
    CHECK_FLAG(D3DDEVCAPS_NPATCHES); 
    CHECK_FLAG(D3DDEVCAPS_PUREDEVICE); 
    CHECK_FLAG(D3DDEVCAPS_QUINTICRTPATCHES);
    CHECK_FLAG(D3DDEVCAPS_RTPATCHES); 
    CHECK_FLAG(D3DDEVCAPS_RTPATCHHANDLEZERO); 
    CHECK_FLAG(D3DDEVCAPS_SEPARATETEXTUREMEMORIES); 
    CHECK_FLAG(D3DDEVCAPS_TEXTURENONLOCALVIDMEM);
    CHECK_FLAG(D3DDEVCAPS_TEXTURESYSTEMMEMORY);
    CHECK_FLAG(D3DDEVCAPS_TEXTUREVIDEOMEMORY);
    CHECK_FLAG(D3DDEVCAPS_TLVERTEXSYSTEMMEMORY);
    CHECK_FLAG(D3DDEVCAPS_TLVERTEXVIDEOMEMORY);

    SEL_CHECK_FLAG(PrimitiveMiscCaps);
    CHECK_FLAG(D3DPMISCCAPS_MASKZ);
    CHECK_FLAG(D3DPMISCCAPS_CULLNONE);
    CHECK_FLAG(D3DPMISCCAPS_CULLCW);
    CHECK_FLAG(D3DPMISCCAPS_CULLCCW);
    CHECK_FLAG(D3DPMISCCAPS_COLORWRITEENABLE);
    CHECK_FLAG(D3DPMISCCAPS_CLIPPLANESCALEDPOINTS);
    CHECK_FLAG(D3DPMISCCAPS_CLIPTLVERTS);
    CHECK_FLAG(D3DPMISCCAPS_TSSARGTEMP);
    CHECK_FLAG(D3DPMISCCAPS_BLENDOP);
    CHECK_FLAG(D3DPMISCCAPS_NULLREFERENCE);
    CHECK_FLAG(D3DPMISCCAPS_INDEPENDENTWRITEMASKS);
    CHECK_FLAG(D3DPMISCCAPS_PERSTAGECONSTANT);
    CHECK_FLAG(D3DPMISCCAPS_POSTBLENDSRGBCONVERT);   // This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DPMISCCAPS_FOGANDSPECULARALPHA);
    CHECK_FLAG(D3DPMISCCAPS_SEPARATEALPHABLEND);
    CHECK_FLAG(D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS);
    CHECK_FLAG(D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING);
    CHECK_FLAG(D3DPMISCCAPS_FOGVERTEXCLAMPED);


    SEL_CHECK_FLAG(RasterCaps);
    CHECK_FLAG(D3DPRASTERCAPS_ANISOTROPY);
    CHECK_FLAG(D3DPRASTERCAPS_COLORPERSPECTIVE);
    CHECK_FLAG(D3DPRASTERCAPS_DITHER);
    CHECK_FLAG(D3DPRASTERCAPS_DEPTHBIAS);
    CHECK_FLAG(D3DPRASTERCAPS_FOGRANGE);
    CHECK_FLAG(D3DPRASTERCAPS_FOGTABLE);
    CHECK_FLAG(D3DPRASTERCAPS_FOGVERTEX);
    CHECK_FLAG(D3DPRASTERCAPS_MIPMAPLODBIAS);
    CHECK_FLAG(D3DPRASTERCAPS_MULTISAMPLE_TOGGLE);
    CHECK_FLAG(D3DPRASTERCAPS_SCISSORTEST);
    CHECK_FLAG(D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS);
    CHECK_FLAG(D3DPRASTERCAPS_WBUFFER);
    CHECK_FLAG(D3DPRASTERCAPS_WFOG);
    CHECK_FLAG(D3DPRASTERCAPS_ZBUFFERLESSHSR);
    CHECK_FLAG(D3DPRASTERCAPS_ZFOG);
    CHECK_FLAG(D3DPRASTERCAPS_ZTEST);

    SEL_CHECK_FLAG(ZCmpCaps);
    CHECK_FLAG(D3DPCMPCAPS_ALWAYS);
    CHECK_FLAG(D3DPCMPCAPS_EQUAL);
    CHECK_FLAG(D3DPCMPCAPS_GREATER);
    CHECK_FLAG(D3DPCMPCAPS_GREATEREQUAL);
    CHECK_FLAG(D3DPCMPCAPS_LESS);
    CHECK_FLAG(D3DPCMPCAPS_LESSEQUAL);
    CHECK_FLAG(D3DPCMPCAPS_NEVER);
    CHECK_FLAG(D3DPCMPCAPS_NOTEQUAL);

    SEL_CHECK_FLAG(SrcBlendCaps);
    CHECK_FLAG(D3DPBLENDCAPS_BLENDFACTOR);
    CHECK_FLAG(D3DPBLENDCAPS_BOTHINVSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_BOTHSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_DESTALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_DESTCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVDESTALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_INVDESTCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCCOLOR2); //This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DPBLENDCAPS_ONE);
    CHECK_FLAG(D3DPBLENDCAPS_SRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_SRCALPHASAT);
    CHECK_FLAG(D3DPBLENDCAPS_SRCCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_SRCCOLOR2); //This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DPBLENDCAPS_ZERO);

    SEL_CHECK_FLAG(DestBlendCaps);
    CHECK_FLAG(D3DPBLENDCAPS_BLENDFACTOR);
    CHECK_FLAG(D3DPBLENDCAPS_BOTHINVSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_BOTHSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_DESTALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_DESTCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVDESTALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_INVDESTCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_INVSRCCOLOR2); //This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DPBLENDCAPS_ONE);
    CHECK_FLAG(D3DPBLENDCAPS_SRCALPHA);
    CHECK_FLAG(D3DPBLENDCAPS_SRCALPHASAT);
    CHECK_FLAG(D3DPBLENDCAPS_SRCCOLOR);
    CHECK_FLAG(D3DPBLENDCAPS_SRCCOLOR2); //This flag is available in Direct3D 9Ex only.
    CHECK_FLAG(D3DPBLENDCAPS_ZERO);

    SEL_CHECK_FLAG(AlphaCmpCaps);
    CHECK_FLAG(D3DPCMPCAPS_ALWAYS);
    CHECK_FLAG(D3DPCMPCAPS_EQUAL);
    CHECK_FLAG(D3DPCMPCAPS_GREATER);
    CHECK_FLAG(D3DPCMPCAPS_GREATEREQUAL);
    CHECK_FLAG(D3DPCMPCAPS_LESS);
    CHECK_FLAG(D3DPCMPCAPS_LESSEQUAL);
    CHECK_FLAG(D3DPCMPCAPS_NEVER);
    CHECK_FLAG(D3DPCMPCAPS_NOTEQUAL);

    SEL_CHECK_FLAG(ShadeCaps);
    CHECK_FLAG(D3DPSHADECAPS_ALPHAGOURAUDBLEND);
    CHECK_FLAG(D3DPSHADECAPS_COLORGOURAUDRGB);
    CHECK_FLAG(D3DPSHADECAPS_FOGGOURAUD);
    CHECK_FLAG(D3DPSHADECAPS_SPECULARGOURAUDRGB);

    SEL_CHECK_FLAG(TextureCaps);
    CHECK_FLAG(D3DPTEXTURECAPS_ALPHA);
    CHECK_FLAG(D3DPTEXTURECAPS_ALPHAPALETTE);
    CHECK_FLAG(D3DPTEXTURECAPS_CUBEMAP);
    CHECK_FLAG(D3DPTEXTURECAPS_CUBEMAP_POW2);
    CHECK_FLAG(D3DPTEXTURECAPS_MIPCUBEMAP);
    CHECK_FLAG(D3DPTEXTURECAPS_MIPMAP);
    CHECK_FLAG(D3DPTEXTURECAPS_MIPVOLUMEMAP);
    CHECK_FLAG(D3DPTEXTURECAPS_NONPOW2CONDITIONAL);
    CHECK_FLAG(D3DPTEXTURECAPS_NOPROJECTEDBUMPENV);
    CHECK_FLAG(D3DPTEXTURECAPS_PERSPECTIVE);
    CHECK_FLAG(D3DPTEXTURECAPS_POW2);
    CHECK_FLAG(D3DPTEXTURECAPS_PROJECTED);
    CHECK_FLAG(D3DPTEXTURECAPS_SQUAREONLY);
    CHECK_FLAG(D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE);
    CHECK_FLAG(D3DPTEXTURECAPS_VOLUMEMAP);
    CHECK_FLAG(D3DPTEXTURECAPS_VOLUMEMAP_POW2);

    SEL_CHECK_FLAG(TextureFilterCaps);
    CHECK_FLAG(D3DPTFILTERCAPS_CONVOLUTIONMONO);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFLINEAR);

    SEL_CHECK_FLAG(CubeTextureFilterCaps);
    CHECK_FLAG(D3DPTFILTERCAPS_CONVOLUTIONMONO);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFLINEAR);

    SEL_CHECK_FLAG(VolumeTextureFilterCaps);
    CHECK_FLAG(D3DPTFILTERCAPS_CONVOLUTIONMONO);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFLINEAR);


    SEL_CHECK_FLAG(TextureAddressCaps);
    CHECK_FLAG(D3DPTADDRESSCAPS_BORDER);
    CHECK_FLAG(D3DPTADDRESSCAPS_CLAMP);
    CHECK_FLAG(D3DPTADDRESSCAPS_INDEPENDENTUV);
    CHECK_FLAG(D3DPTADDRESSCAPS_MIRROR);
    CHECK_FLAG(D3DPTADDRESSCAPS_MIRRORONCE);
    CHECK_FLAG(D3DPTADDRESSCAPS_WRAP);

    SEL_CHECK_FLAG(VolumeTextureAddressCaps);
    CHECK_FLAG(D3DPTADDRESSCAPS_BORDER);
    CHECK_FLAG(D3DPTADDRESSCAPS_CLAMP);
    CHECK_FLAG(D3DPTADDRESSCAPS_INDEPENDENTUV);
    CHECK_FLAG(D3DPTADDRESSCAPS_MIRROR);
    CHECK_FLAG(D3DPTADDRESSCAPS_MIRRORONCE);
    CHECK_FLAG(D3DPTADDRESSCAPS_WRAP);

    SEL_CHECK_FLAG(LineCaps);
    CHECK_FLAG(D3DLINECAPS_ALPHACMP);
    CHECK_FLAG(D3DLINECAPS_ANTIALIAS);
    CHECK_FLAG(D3DLINECAPS_BLEND);
    CHECK_FLAG(D3DLINECAPS_FOG);
    CHECK_FLAG(D3DLINECAPS_TEXTURE);
    CHECK_FLAG(D3DLINECAPS_ZTEST);

    CAPS_TRACE_VAL_I(MaxTextureWidth);
    CAPS_TRACE_VAL_I(MaxTextureHeight);
    CAPS_TRACE_VAL_I(MaxVolumeExtent);
    CAPS_TRACE_VAL_I(MaxTextureRepeat);
    CAPS_TRACE_VAL_I(MaxTextureAspectRatio);
    CAPS_TRACE_VAL_I(MaxAnisotropy);

    CAPS_TRACE_VAL_F(MaxVertexW);
    CAPS_TRACE_VAL_F(GuardBandLeft);
    CAPS_TRACE_VAL_F(GuardBandTop);
    CAPS_TRACE_VAL_F(GuardBandRight);
    CAPS_TRACE_VAL_F(GuardBandBottom);
    CAPS_TRACE_VAL_F(ExtentsAdjust);

    SEL_CHECK_FLAG(StencilCaps);
    CHECK_FLAG(D3DSTENCILCAPS_KEEP);
    CHECK_FLAG(D3DSTENCILCAPS_ZERO);
    CHECK_FLAG(D3DSTENCILCAPS_REPLACE);
    CHECK_FLAG(D3DSTENCILCAPS_INCRSAT);
    CHECK_FLAG(D3DSTENCILCAPS_DECRSAT);
    CHECK_FLAG(D3DSTENCILCAPS_INVERT);
    CHECK_FLAG(D3DSTENCILCAPS_INCR);
    CHECK_FLAG(D3DSTENCILCAPS_DECR);
    CHECK_FLAG(D3DSTENCILCAPS_TWOSIDED);

    SEL_CHECK_FLAG(FVFCaps);
    CHECK_FLAG(D3DFVFCAPS_DONOTSTRIPELEMENTS);
    CHECK_FLAG(D3DFVFCAPS_PSIZE);
    CHECK_FLAG(D3DFVFCAPS_TEXCOORDCOUNTMASK);

    SEL_CHECK_FLAG(TextureOpCaps);
    CHECK_FLAG(D3DTEXOPCAPS_ADD);
    CHECK_FLAG(D3DTEXOPCAPS_ADDSIGNED);
    CHECK_FLAG(D3DTEXOPCAPS_ADDSIGNED2X);
    CHECK_FLAG(D3DTEXOPCAPS_ADDSMOOTH);
    CHECK_FLAG(D3DTEXOPCAPS_BLENDCURRENTALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_BLENDDIFFUSEALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_BLENDFACTORALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_BLENDTEXTUREALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_BLENDTEXTUREALPHAPM);
    CHECK_FLAG(D3DTEXOPCAPS_BUMPENVMAP);
    CHECK_FLAG(D3DTEXOPCAPS_BUMPENVMAPLUMINANCE);
    CHECK_FLAG(D3DTEXOPCAPS_DISABLE);
    CHECK_FLAG(D3DTEXOPCAPS_DOTPRODUCT3);
    CHECK_FLAG(D3DTEXOPCAPS_LERP);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATE);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATE2X);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATE4X);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR);
    CHECK_FLAG(D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA);
    CHECK_FLAG(D3DTEXOPCAPS_MULTIPLYADD);
    CHECK_FLAG(D3DTEXOPCAPS_PREMODULATE);
    CHECK_FLAG(D3DTEXOPCAPS_SELECTARG1);
    CHECK_FLAG(D3DTEXOPCAPS_SELECTARG2);
    CHECK_FLAG(D3DTEXOPCAPS_SUBTRACT);

    CAPS_TRACE_VAL_I(MaxTextureBlendStages);
    CAPS_TRACE_VAL_I(MaxSimultaneousTextures);

    SEL_CHECK_FLAG(VertexProcessingCaps)
      CHECK_FLAG(D3DVTXPCAPS_DIRECTIONALLIGHTS);
    CHECK_FLAG(D3DVTXPCAPS_LOCALVIEWER);
    CHECK_FLAG(D3DVTXPCAPS_MATERIALSOURCE7);
    CHECK_FLAG(D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER);
    CHECK_FLAG(D3DVTXPCAPS_POSITIONALLIGHTS);
    CHECK_FLAG(D3DVTXPCAPS_TEXGEN);
    CHECK_FLAG(D3DVTXPCAPS_TEXGEN_SPHEREMAP);
    CHECK_FLAG(D3DVTXPCAPS_TWEENING);

    CAPS_TRACE_VAL_I(MaxActiveLights);
    CAPS_TRACE_VAL_I(MaxUserClipPlanes);
    CAPS_TRACE_VAL_I(MaxVertexBlendMatrices);
    CAPS_TRACE_VAL_I(MaxVertexBlendMatrixIndex);

    CAPS_TRACE_VAL_F(MaxPointSize);



    CAPS_TRACE_VAL_I(MaxPrimitiveCount);
    CAPS_TRACE_VAL_I(MaxVertexIndex);
    CAPS_TRACE_VAL_I(MaxStreams);
    CAPS_TRACE_VAL_I(MaxStreamStride);
    CAPS_TRACE_VAL_I(VertexShaderVersion);
    CAPS_TRACE_VAL_I(MaxVertexShaderConst);
    CAPS_TRACE_VAL_I(PixelShaderVersion);

    CAPS_TRACE_VAL_F(PixelShader1xMaxValue);


    SEL_CHECK_FLAG(DevCaps2);
    CHECK_FLAG(D3DDEVCAPS2_ADAPTIVETESSRTPATCH);
    CHECK_FLAG(D3DDEVCAPS2_ADAPTIVETESSNPATCH);
    CHECK_FLAG(D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES);
    CHECK_FLAG(D3DDEVCAPS2_DMAPNPATCH);
    CHECK_FLAG(D3DDEVCAPS2_PRESAMPLEDDMAPNPATCH);
    CHECK_FLAG(D3DDEVCAPS2_STREAMOFFSET);
    CHECK_FLAG(D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET);

    CAPS_TRACE_VAL_I(MasterAdapterOrdinal);
    CAPS_TRACE_VAL_I(AdapterOrdinalInGroup);
    CAPS_TRACE_VAL_I(NumberOfAdaptersInGroup);

    SEL_CHECK_FLAG(DeclTypes);
    CHECK_FLAG(D3DDTCAPS_UBYTE4);
    CHECK_FLAG(D3DDTCAPS_UBYTE4N);
    CHECK_FLAG(D3DDTCAPS_SHORT2N);
    CHECK_FLAG(D3DDTCAPS_SHORT4N);
    CHECK_FLAG(D3DDTCAPS_USHORT2N);
    CHECK_FLAG(D3DDTCAPS_USHORT4N);
    CHECK_FLAG(D3DDTCAPS_UDEC3);
    CHECK_FLAG(D3DDTCAPS_DEC3N);
    CHECK_FLAG(D3DDTCAPS_FLOAT16_2);
    CHECK_FLAG(D3DDTCAPS_FLOAT16_4);

    CAPS_TRACE_VAL_I(NumSimultaneousRTs);


    SEL_CHECK_FLAG(StretchRectFilterCaps);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFLINEAR);

    //VS20Caps 
    //D3DPSHADERCAPS2_0 
    SEL_CHECK_FLAG(VertexTextureFilterCaps);
    CHECK_FLAG(D3DPTFILTERCAPS_CONVOLUTIONMONO);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFLINEAR);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFANISOTROPIC);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MINFGAUSSIANQUAD);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFPOINT);
    CHECK_FLAG(D3DPTFILTERCAPS_MIPFLINEAR);

    CAPS_TRACE_VAL_I(MaxVShaderInstructionsExecuted);
    CAPS_TRACE_VAL_I(MaxPShaderInstructionsExecuted); 
    CAPS_TRACE_VAL_I(MaxVertexShader30InstructionSlots);
    CAPS_TRACE_VAL_I(MaxPixelShader30InstructionSlots);

    return TRUE;
  }
//};
*/

#ifdef _WIN32
GXBOOL __stdcall DllMain(__in HANDLE _HDllHandle, __in DWORD _Reason, __in_opt LPVOID _Reserved)
{
  switch(_Reason)
  {
  case DLL_PROCESS_ATTACH:
    g_hDLLModule = (HINSTANCE)_HDllHandle;
    //g_pCurStation->hInstDll = (HINSTANCE)_HDllHandle;
    g_pInstDll = new GXINSTANCE;
    g_pInstDll->Initialize(g_pCurStation, (HINSTANCE)_HDllHandle);
    //TRACE("DLL_PROCESS_ATTACH\n");
    break;
  case DLL_THREAD_ATTACH:
    //TRACE("DLL_THREAD_ATTACH\n");
    break;
  case DLL_THREAD_DETACH:
    //TRACE("DLL_THREAD_DETACH");
    break;
  case DLL_PROCESS_DETACH:
    // 这个也有可能在程序退出时被销毁了
    SAFE_DELETE(g_pInstDll);
    //TRACE("DLL_PROCESS_DETACH\n");
    break;
  }
  return TRUE;
}
#endif // _WIN32
//////////////////////////////////////////////////////////////////////////

//GXLPRECT _GlbLockStaticRects(GXUINT nCounts)
//{
//#if defined(_WIN32) || defined(_WINDOWS)
//  ASSERT(g_lpRects == 0);  // 只能锁定一次, 程序中也只能使用一次静态Rects数组
//  
//  if(nCounts > g_uRectCapacity || g_hGlbRects == NULL)
//    return new GXRECT[nCounts];
//
//  g_lpRects = (LPGXRECT)GlobalLock(g_hGlbRects);
//#else
//  //ASSERT(g_lpRects == 0);  // 只能锁定一次, 程序中也只能使用一次静态Rects数组
//  
//  if(nCounts > g_uRectCapacity || g_hGlbRects == NULL || g_lpRects != NULL)
//    return new GXRECT[nCounts];
//  
//  g_lpRects = (LPGXRECT)g_hGlbRects;
//#endif // defined(_WIN32) || defined(_WINDOWS)
//  return g_lpRects;
//}
//
//GXVOID _GlbUnlockStaticRects(GXLPRECT lpRects)
//{
//  if(lpRects != g_lpRects)
//    delete lpRects;
//  else
//  {
//#if defined(_WIN32) || defined(_WINDOWS)
//    GlobalUnlock(g_lpRects);
//#endif // defined(_WIN32) || defined(_WINDOWS)
//    g_lpRects = NULL;
//  }
//}
