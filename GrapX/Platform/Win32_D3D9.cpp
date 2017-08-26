#ifdef ENABLE_GRAPHICS_API_DX9
#if (defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)) && ! defined(__clang__)

// 全局头文件
#include "GrapX.H"
#include "GXApp.H"
#include "User/GrapX.Hxx"

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/MOLogger.H"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"

// 私有头文件
#include <User32Ex.H>
#include "Console.h"
//#ifdef _ENABLE_STMT
//#include <clstdcode\stmt\stmt.h>
//#else
//#include <clMessageThread.h>
//#endif // #ifdef _ENABLE_STMT
#include <GrapX/gxDevice.H>
#include "Canvas/GXResourceMgr.h"
#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
#include "GrapX/GXUser.H"
#include "thread/clMessageThread.h"
#include "User/gxMessage.hxx"

//////////////////////////////////////////////////////////////////////////

IGXPlatform_Win32D3D9::IGXPlatform_Win32D3D9()
  : m_pd3d9     (NULL)
{
  m_pApp = NULL;
}

IGXPlatform_Win32D3D9::~IGXPlatform_Win32D3D9()
{
}

GXHRESULT IGXPlatform_Win32D3D9::Initialize(GXApp* pApp, GXAPP_DESC* pDesc, GXGraphics** ppGraphics)
{
  const static GXLPWSTR lpClassName = L"GrapX_Win32_D3D9_Class";
  //WNDCLASSEX wcex;

  m_pApp = pApp;
  GXGraphics* pGraphics = NULL;

  if( NULL == ( m_pd3d9 = Direct3DCreate9( D3D_SDK_VERSION ) ) )
  {
    //LogMgr::Output(LC_GrapX, "");
    return GX_FAIL;
  }

  if(CreateWnd(lpClassName, WndProc, pDesc, pApp) != 0) {
    return GX_FAIL;
  }

  //m_dwAppDescStyle = pDesc->dwStyle;

  D3D9::GRAPHICS_CREATION_DESC sDesc;
  sDesc.hWnd       = m_hWnd;
  sDesc.bWaitForVSync = pDesc->dwStyle & GXADS_WAITFORVSYNC;
  sDesc.lpD3D      = m_pd3d9;
  sDesc.szRootDir  = m_strRootDir;
  sDesc.pLogger    = pDesc->pLogger;
  sDesc.pParameter = pDesc->pParameter;
  pGraphics = D3D9::GXGraphicsImpl::Create(&sDesc);

  if( ! pGraphics) {
    CLOG_ERROR("Can not create D3D9 Graphics.\n");
    return -1;
  }
  *ppGraphics = pGraphics;
  m_pLogger = sDesc.pLogger;
  if(m_pLogger) {
    m_pLogger->AddRef();
  }

  GXCREATESTATION stCrateStation;
  stCrateStation.cbSize     = sizeof(GXCREATESTATION);
  stCrateStation.hWnd       = m_hWnd;
  stCrateStation.lpPlatform = this;
  stCrateStation.lpAppDesc  = pDesc;
  GXUICreateStation(&stCrateStation);

  //pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
  //m_pApp->OnCreate();
  //pGraphics->Activate(FALSE);

//#ifndef _DEV_DISABLE_UI_CODE
#ifdef _ENABLE_STMT
  STMT::CreateTask(1024 * 1024, UITask, NULL);
#else
  GXSTATION* pStation = GrapX::Internal::GetStationPtr();

# ifdef REFACTOR_SYSQUEUE
  pStation->m_pSysMsg = new GrapX::Internal::SystemMessage(this);
  //pStation->m_pSysMsg->Start();
  pStation->m_pMsgThread = new GXUIMsgThread(this);
  pStation->m_pMsgThread->Start();
# else
  pStation->m_pMsgThread = new GXUIMsgThread(this);
  pStation->m_pMsgThread->Start();
# endif
    //static_cast<MessageThread*>(MessageThread::CreateThread((CLTHREADCALLBACK)UITask, this));
#endif // #ifdef _ENABLE_STMT
//#endif // _DEV_DISABLE_UI_CODE

  // 这个必须放在最后, 所有初始化完毕, 刷新窗口
  ShowWindow(m_hWnd, GXSW_SHOWDEFAULT);
  UpdateWindow(m_hWnd);

  return GX_OK;
}

GXHRESULT IGXPlatform_Win32D3D9::Finalize(GXINOUT GXGraphics** ppGraphics)
{
  GXUIDestroyStation();

  SAFE_RELEASE(*ppGraphics);
  SAFE_RELEASE(m_pd3d9);
  return IMOPlatform_Win32Base::Finalize(ppGraphics);
}

LRESULT GXCALLBACK IGXPlatform_Win32D3D9::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_GX_RESETDEVICED3D9:
    {
      GXApp* pApp = (GXApp*)GetWindowLong(hWnd, 0);
      GXGraphics* pGraphics = pApp->GetGraphicsUnsafe();
      D3D9::GXGraphicsImpl* pGraphicsImpl 
        = static_cast<D3D9::GXGraphicsImpl*>(pGraphics);
      return pGraphicsImpl->D3DGetDevice()->Reset((D3DPRESENT_PARAMETERS*)lParam);
    }
    break;
  }
  return IMOPlatform_Win32Base::WndProc(hWnd, message, wParam, lParam);
}

GXVOID IGXPlatform_Win32D3D9::GetPlatformID(GXPlaformIdentity* pIdentity)
{
  *pIdentity = GXPLATFORM_WIN32_DIRECT3D9;
}


//GXDWORD GXCALLBACK IGXPlatform_Win32D3D9::UITask(GXLPVOID lParam)
//{
//#ifndef _DEV_DISABLE_UI_CODE
//  CLMTCREATESTRUCT* pCreateParam = (CLMTCREATESTRUCT*)lParam;
//  IGXPlatform_Win32D3D9* pPlatform = (IGXPlatform_Win32D3D9*)pCreateParam->pUserParam;
//  GXApp* pApp = (GXApp*)pPlatform->m_pApp;
//  GXGraphics* pGraphics = pApp->GetGraphicsUnsafe();
//
//  pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
//  GXHRESULT hval = pApp->OnCreate();
//  pGraphics->Activate(FALSE);
//
//  if(GXFAILED(hval)) {
//    return (GXDWORD)hval;
//  }
//
//  GXMSG gxmsg;
//  while(gxGetMessage(&gxmsg, NULL))
//  {
//    gxDispatchMessageW(&gxmsg);
//    pPlatform->AppHandle(gxmsg.message, gxmsg.wParam, gxmsg.lParam);
//  }
//#endif // _DEV_DISABLE_UI_CODE
//
//  pApp->OnDestroy();
//  return NULL;
//}

GXLPCWSTR IGXPlatform_Win32D3D9::GetRootDir()
{
  return m_strRootDir;
}

//GXDWORD IGXPlatform_Win32D3D9::GetAppDescStyle() const
//{
//  return m_dwAppDescStyle;
//}

//////////////////////////////////////////////////////////////////////////

IGXPlatform_Win32D3D9* AppCreateD3D9Platform(GXApp* pApp, GXAPP_DESC* pDesc, GXGraphics** ppGraphics)
{
  return AppCreatePlatformT<IGXPlatform_Win32D3D9>(pApp, pDesc, ppGraphics);
}
//////////////////////////////////////////////////////////////////////////
STATIC_ASSERT(GXCLEAR_TARGET  == D3DCLEAR_TARGET );
STATIC_ASSERT(GXCLEAR_DEPTH   == D3DCLEAR_ZBUFFER);
STATIC_ASSERT(GXCLEAR_STENCIL == D3DCLEAR_STENCIL);

STATIC_ASSERT(GXFILL_POINT      == D3DFILL_POINT);
STATIC_ASSERT(GXFILL_WIREFRAME  == D3DFILL_WIREFRAME);
STATIC_ASSERT(GXFILL_SOLID      == D3DFILL_SOLID);

STATIC_ASSERT(GXPT_POINTLIST      == D3DPT_POINTLIST    );
STATIC_ASSERT(GXPT_LINELIST       == D3DPT_LINELIST     );
STATIC_ASSERT(GXPT_LINESTRIP      == D3DPT_LINESTRIP    );
STATIC_ASSERT(GXPT_TRIANGLELIST   == D3DPT_TRIANGLELIST );
STATIC_ASSERT(GXPT_TRIANGLESTRIP  == D3DPT_TRIANGLESTRIP);
STATIC_ASSERT(GXPT_TRIANGLEFAN    == D3DPT_TRIANGLEFAN  );

STATIC_ASSERT(GXFILTER_NONE             == D3DX_FILTER_NONE);
STATIC_ASSERT(GXFILTER_POINT            == D3DX_FILTER_POINT);
STATIC_ASSERT(GXFILTER_LINEAR           == D3DX_FILTER_LINEAR);
STATIC_ASSERT(GXFILTER_TRIANGLE         == D3DX_FILTER_TRIANGLE);
STATIC_ASSERT(GXFILTER_BOX              == D3DX_FILTER_BOX);
STATIC_ASSERT(GXFILTER_MIRROR_U         == D3DX_FILTER_MIRROR_U);
STATIC_ASSERT(GXFILTER_MIRROR_V         == D3DX_FILTER_MIRROR_V);
STATIC_ASSERT(GXFILTER_MIRROR_W         == D3DX_FILTER_MIRROR_W);
STATIC_ASSERT(GXFILTER_MIRROR           == D3DX_FILTER_MIRROR);
STATIC_ASSERT(GXFILTER_DITHER           == D3DX_FILTER_DITHER);
STATIC_ASSERT(GXFILTER_DITHER_DIFFUSION == D3DX_FILTER_DITHER_DIFFUSION);
STATIC_ASSERT(GXFILTER_SRGB_IN          == D3DX_FILTER_SRGB_IN);
STATIC_ASSERT(GXFILTER_SRGB_OUT         == D3DX_FILTER_SRGB_OUT);
STATIC_ASSERT(GXFILTER_SRGB             == D3DX_FILTER_SRGB);

// 检查定义是否正确
//STATIC_ASSERT(GXPOOL_DEFAULT   == D3DPOOL_DEFAULT);
//STATIC_ASSERT(GXPOOL_SYSTEMMEM == D3DPOOL_SYSTEMMEM);
//
//STATIC_ASSERT(GXUSAGE_RENDERTARGET       == D3DUSAGE_RENDERTARGET);
//STATIC_ASSERT(GXUSAGE_DEPTHSTENCIL       == D3DUSAGE_DEPTHSTENCIL);
//STATIC_ASSERT(GXUSAGE_DYNAMIC            == D3DUSAGE_DYNAMIC);
//STATIC_ASSERT(GXUSAGE_AUTOGENMIPMAP      == D3DUSAGE_AUTOGENMIPMAP);
//STATIC_ASSERT(GXUSAGE_DMAP               == D3DUSAGE_DMAP);
//STATIC_ASSERT(GXUSAGE_WRITEONLY          == D3DUSAGE_WRITEONLY);
//STATIC_ASSERT(GXUSAGE_SOFTWAREPROCESSING == D3DUSAGE_SOFTWAREPROCESSING);
//STATIC_ASSERT(GXUSAGE_DONOTCLIP          == D3DUSAGE_DONOTCLIP);
//STATIC_ASSERT(GXUSAGE_POINTS             == D3DUSAGE_POINTS);
//STATIC_ASSERT(GXUSAGE_RTPATCHES          == D3DUSAGE_RTPATCHES);
//STATIC_ASSERT(GXUSAGE_NPATCHES           == D3DUSAGE_NPATCHES);

STATIC_ASSERT(GXFMT_R8G8B8               == D3DFMT_R8G8B8);
STATIC_ASSERT(GXFMT_A8R8G8B8             == D3DFMT_A8R8G8B8);
STATIC_ASSERT(GXFMT_X8R8G8B8             == D3DFMT_X8R8G8B8);
STATIC_ASSERT(GXFMT_R5G6B5               == D3DFMT_R5G6B5);
STATIC_ASSERT(GXFMT_X1R5G5B5             == D3DFMT_X1R5G5B5);
STATIC_ASSERT(GXFMT_A1R5G5B5             == D3DFMT_A1R5G5B5);
STATIC_ASSERT(GXFMT_A4R4G4B4             == D3DFMT_A4R4G4B4);
STATIC_ASSERT(GXFMT_R3G3B2               == D3DFMT_R3G3B2);
STATIC_ASSERT(GXFMT_A8                   == D3DFMT_A8);
STATIC_ASSERT(GXFMT_A8R3G3B2             == D3DFMT_A8R3G3B2);
STATIC_ASSERT(GXFMT_X4R4G4B4             == D3DFMT_X4R4G4B4);
STATIC_ASSERT(GXFMT_A2B10G10R10          == D3DFMT_A2B10G10R10);
STATIC_ASSERT(GXFMT_A8B8G8R8             == D3DFMT_A8B8G8R8);
STATIC_ASSERT(GXFMT_X8B8G8R8             == D3DFMT_X8B8G8R8);
STATIC_ASSERT(GXFMT_G16R16               == D3DFMT_G16R16);
STATIC_ASSERT(GXFMT_A2R10G10B10          == D3DFMT_A2R10G10B10);
STATIC_ASSERT(GXFMT_A16B16G16R16         == D3DFMT_A16B16G16R16);
STATIC_ASSERT(GXFMT_A8P8                 == D3DFMT_A8P8);
STATIC_ASSERT(GXFMT_P8                   == D3DFMT_P8);
STATIC_ASSERT(GXFMT_L8                   == D3DFMT_L8);
STATIC_ASSERT(GXFMT_A8L8                 == D3DFMT_A8L8);
STATIC_ASSERT(GXFMT_A4L4                 == D3DFMT_A4L4);
STATIC_ASSERT(GXFMT_V8U8                 == D3DFMT_V8U8);
STATIC_ASSERT(GXFMT_L6V5U5               == D3DFMT_L6V5U5);
STATIC_ASSERT(GXFMT_X8L8V8U8             == D3DFMT_X8L8V8U8);
STATIC_ASSERT(GXFMT_Q8W8V8U8             == D3DFMT_Q8W8V8U8);
STATIC_ASSERT(GXFMT_V16U16               == D3DFMT_V16U16);
STATIC_ASSERT(GXFMT_A2W10V10U10          == D3DFMT_A2W10V10U10);
//STATIC_ASSERT(GXFMT_UYVY                 == D3DFMT_UYVY);
//STATIC_ASSERT(GXFMT_R8G8_B8G8            == D3DFMT_R8G8_B8G8);
//STATIC_ASSERT(GXFMT_YUY2                 == D3DFMT_YUY2);
//STATIC_ASSERT(GXFMT_G8R8_G8B8            == D3DFMT_G8R8_G8B8);
STATIC_ASSERT(GXFMT_DXT1                 == D3DFMT_DXT1);
STATIC_ASSERT(GXFMT_DXT2                 == D3DFMT_DXT2);
STATIC_ASSERT(GXFMT_DXT3                 == D3DFMT_DXT3);
STATIC_ASSERT(GXFMT_DXT4                 == D3DFMT_DXT4);
STATIC_ASSERT(GXFMT_DXT5                 == D3DFMT_DXT5);
STATIC_ASSERT(GXFMT_D16_LOCKABLE         == D3DFMT_D16_LOCKABLE);
STATIC_ASSERT(GXFMT_D32                  == D3DFMT_D32);
STATIC_ASSERT(GXFMT_D15S1                == D3DFMT_D15S1);
STATIC_ASSERT(GXFMT_D24S8                == D3DFMT_D24S8);
STATIC_ASSERT(GXFMT_D24X8                == D3DFMT_D24X8);
STATIC_ASSERT(GXFMT_D24X4S4              == D3DFMT_D24X4S4);
STATIC_ASSERT(GXFMT_D16                  == D3DFMT_D16);
STATIC_ASSERT(GXFMT_D32F_LOCKABLE        == D3DFMT_D32F_LOCKABLE);
STATIC_ASSERT(GXFMT_D24FS8               == D3DFMT_D24FS8);
STATIC_ASSERT(GXFMT_D32_LOCKABLE         == D3DFMT_D32_LOCKABLE);
STATIC_ASSERT(GXFMT_S8_LOCKABLE          == D3DFMT_S8_LOCKABLE);
STATIC_ASSERT(GXFMT_L16                  == D3DFMT_L16);
STATIC_ASSERT(GXFMT_VERTEXDATA           == D3DFMT_VERTEXDATA);
STATIC_ASSERT(GXFMT_INDEX16              == D3DFMT_INDEX16);
STATIC_ASSERT(GXFMT_INDEX32              == D3DFMT_INDEX32);
//STATIC_ASSERT(GXFMT_Q16W16V16U16         == D3DFMT_Q16W16V16U16);
//STATIC_ASSERT(GXFMT_MULTI2_ARGB8         == D3DFMT_MULTI2_ARGB8);
STATIC_ASSERT(GXFMT_R16F                 == D3DFMT_R16F);
STATIC_ASSERT(GXFMT_G16R16F              == D3DFMT_G16R16F);
STATIC_ASSERT(GXFMT_A16B16G16R16F        == D3DFMT_A16B16G16R16F);
STATIC_ASSERT(GXFMT_R32F                 == D3DFMT_R32F);
STATIC_ASSERT(GXFMT_G32R32F              == D3DFMT_G32R32F);
STATIC_ASSERT(GXFMT_A32B32G32R32F        == D3DFMT_A32B32G32R32F);
STATIC_ASSERT(GXFMT_CxV8U8               == D3DFMT_CxV8U8);
STATIC_ASSERT(GXFMT_A1                   == D3DFMT_A1);
//STATIC_ASSERT(GXFMT_A2B10G10R10_XR_BIAS  == D3DFMT_A2B10G10R10_XR_BIAS);
//STATIC_ASSERT(GXFMT_BINARYBUFFER         == D3DFMT_BINARYBUFFER);

// 宏定义检查
STATIC_ASSERT(GXTEXFILTER_NONE            == D3DTEXF_NONE);
STATIC_ASSERT(GXTEXFILTER_POINT           == D3DTEXF_POINT);
STATIC_ASSERT(GXTEXFILTER_LINEAR          == D3DTEXF_LINEAR);
STATIC_ASSERT(GXTEXFILTER_ANISOTROPIC     == D3DTEXF_ANISOTROPIC);
STATIC_ASSERT(GXTEXFILTER_PYRAMIDALQUAD   == D3DTEXF_PYRAMIDALQUAD);
STATIC_ASSERT(GXTEXFILTER_GAUSSIANQUAD    == D3DTEXF_GAUSSIANQUAD);

//STATIC_ASSERT(GXSAMP_ADDRESSU      == D3DSAMP_ADDRESSU);
//STATIC_ASSERT(GXSAMP_ADDRESSV      == D3DSAMP_ADDRESSV);
//STATIC_ASSERT(GXSAMP_ADDRESSW      == D3DSAMP_ADDRESSW);
//STATIC_ASSERT(GXSAMP_BORDERCOLOR   == D3DSAMP_BORDERCOLOR);
//STATIC_ASSERT(GXSAMP_MAGFILTER     == D3DSAMP_MAGFILTER);
//STATIC_ASSERT(GXSAMP_MINFILTER     == D3DSAMP_MINFILTER);
//STATIC_ASSERT(GXSAMP_MIPFILTER     == D3DSAMP_MIPFILTER);
//STATIC_ASSERT(GXSAMP_MIPMAPLODBIAS == D3DSAMP_MIPMAPLODBIAS);
//STATIC_ASSERT(GXSAMP_MAXMIPLEVEL   == D3DSAMP_MAXMIPLEVEL);
//STATIC_ASSERT(GXSAMP_MAXANISOTROPY == D3DSAMP_MAXANISOTROPY);
//STATIC_ASSERT(GXSAMP_SRGBTEXTURE   == D3DSAMP_SRGBTEXTURE);
//STATIC_ASSERT(GXSAMP_ELEMENTINDEX  == D3DSAMP_ELEMENTINDEX);
//STATIC_ASSERT(GXSAMP_DMAPOFFSET    == D3DSAMP_DMAPOFFSET);

// STATIC_ASSERT(MK_XXX == GXMK_XXX)
STATIC_ASSERT(MK_LBUTTON == GXMK_LBUTTON);
STATIC_ASSERT(MK_RBUTTON == GXMK_RBUTTON);
STATIC_ASSERT(MK_SHIFT   == GXMK_SHIFT);
STATIC_ASSERT(MK_CONTROL == GXMK_CONTROL);
STATIC_ASSERT(MK_MBUTTON == GXMK_MBUTTON);

#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
