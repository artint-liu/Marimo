#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include "GrapX.h"
#include "GXApp.h"
#include "User/GrapX.Hxx"

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/MOLogger.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// 私有头文件
#include <User32Ex.h>
//#ifdef _ENABLE_STMT
//#include <clstdcode\stmt\stmt.h>
//#else
//#include <clMessageThread.h>
//#endif // #ifdef _ENABLE_STMT
#include <GrapX/gxDevice.h>
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_d3d11.h"
#include "GrapX/GXUser.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "d3dx11d.lib")
#ifdef ENABLE_GRAPHICS_API_DX11
//////////////////////////////////////////////////////////////////////////

IGXPlatform_Win32D3D11::IGXPlatform_Win32D3D11()
{
  m_pApp = NULL;
}

GXHRESULT IGXPlatform_Win32D3D11::Initialize(GXApp* pApp, GXAPP_DESC* pDesc, GXGraphics** ppGraphics)
{
  const static GXLPWSTR lpClassName = L"GrapX_Win32_D3D11_Class";
  //WNDCLASSEX wcex;

  m_pApp = pApp;
  GXGraphics* pGraphics = NULL;

  if(CreateWnd(lpClassName, WndProc, pDesc, pApp) != 0) {
    return GX_FAIL;
  }


  //m_hInstance        = GetModuleHandle(NULL);
  //wcex.cbSize        = sizeof(WNDCLASSEX);
  //wcex.style         = GXCS_HREDRAW | GXCS_VREDRAW;
  //wcex.lpfnWndProc   = WndProc;
  //wcex.cbClsExtra    = 0;
  //wcex.cbWndExtra    = sizeof(GXApp*);
  //wcex.hInstance     = m_hInstance;
  //wcex.hIcon         = NULL;
  //wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  //wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  //wcex.lpszMenuName  = NULL;
  //wcex.lpszClassName = lpClassName;
  //wcex.hIconSm       = NULL;

  //if(RegisterClassEx(&wcex) == 0)
  //{
  //  return GX_FAIL;
  //}

  //if(TEST_FLAG(pDesc->dwStyle, GXADS_SIZABLE) && pDesc->nWidth == 0)
  //{
  //  GXREGN regnNewWin;
  //  // 可调整窗口
  //  // 根据宽度决定是默认尺寸还是用户指定尺寸
  //  //if(pDesc->nWidth == 0) {
  //    gxSetRegn(&regnNewWin, GXCW_USEDEFAULT, 0, GXCW_USEDEFAULT, 0);
  //  //}
  //  //else {
  //  //  RECT rcWorkArea;
  //  //  SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcWorkArea, NULL);
  //  //  gxSetRegn(&regnNewWin, 
  //  //    rcWorkArea.left + (rcWorkArea.right - rcWorkArea.left - pDesc->nWidth) / 2, 
  //  //    rcWorkArea.top + (rcWorkArea.bottom - rcWorkArea.top - pDesc->nHeight) / 2, pDesc->nWidth, pDesc->nHeight);
  //  //}
  //  m_hWnd = CreateWindowEx(
  //    NULL, lpClassName, pDesc->lpName, WS_OVERLAPPEDWINDOW,
  //    regnNewWin.left, regnNewWin.top, regnNewWin.width, regnNewWin.height, NULL, NULL, 
  //    m_hInstance, NULL);
  //}
  //else
  //{
  //  m_hWnd = CreateWindowEx(
  //    NULL, lpClassName, pDesc->lpName, WS_CAPTION | WS_SYSMENU,
  //    0, 0, 100, 100, NULL, NULL, 
  //    m_hInstance, NULL);
  //  RECT rectWorkarea;
  //  SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkarea, 0);
  //  SetClientSize(m_hWnd, NULL, 
  //    ((rectWorkarea.right - rectWorkarea.left) - pDesc->nWidth) / 2 + rectWorkarea.left,
  //    ((rectWorkarea.bottom - rectWorkarea.top) - pDesc->nHeight) / 2 + rectWorkarea.top,
  //    pDesc->nWidth, pDesc->nHeight, NULL);
  //}
  //SetWindowLong(m_hWnd, 0, (GXLONG)pApp);

  //if (m_hWnd == NULL)
  //{
  //  return GX_FAIL;
  //}
  D3D11::GRAPHICS_CREATION_DESC sDesc;
  sDesc.hWnd          = m_hWnd;
  sDesc.bWaitForVSync = pDesc->dwStyle & GXADS_WAITFORVSYNC;
  sDesc.szRootDir     = m_strRootDir;
  sDesc.pLogger       = pDesc->pLogger;
  
  pGraphics = D3D11::GXGraphicsImpl::Create(&sDesc);

  *ppGraphics = pGraphics;
  m_pLogger = pDesc->pLogger;
  if(m_pLogger) {
    m_pLogger->AddRef();
  }
  
  GXCREATESTATION stCrateStation;
  stCrateStation.cbSize = sizeof(GXCREATESTATION);
  stCrateStation.hWnd = m_hWnd;
  stCrateStation.lpPlatform = this;
  stCrateStation.lpAppDesc = pDesc;

  GXUICreateStation(&stCrateStation);

  //pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
  //m_pApp->OnCreate();
  //pGraphics->Activate(FALSE);

//#ifndef _DEV_DISABLE_UI_CODE
#ifdef _ENABLE_STMT
  STMT::CreateTask(1024 * 1024, UITask, NULL);
#else
  
  //GXSTATION* pStation = IntGetStationPtr();
  //pStation->m_pMsgThread = new GXUIMsgThread(this);
  //pStation->m_pMsgThread->Start();

    //static_cast<MessageThread*>(MessageThread::CreateThread((CLTHREADCALLBACK)UITask, this));
#endif // #ifdef _ENABLE_STMT
//#endif // _DEV_DISABLE_UI_CODE

  // 这个必须放在最后, 所有初始化完毕, 刷新窗口
  ShowWindow(m_hWnd, GXSW_SHOWDEFAULT);
  UpdateWindow(m_hWnd);

  return GX_OK;
}
GXHRESULT IGXPlatform_Win32D3D11::Finalize(GXINOUT GXGraphics** ppGraphics)
{
  GXUIDestroyStation();

  SAFE_RELEASE(*ppGraphics);
  return IMOPlatform_Win32Base::Finalize(ppGraphics);
}

GXVOID IGXPlatform_Win32D3D11::GetPlatformID(GXPlaformIdentity* pIdentity)
{
  *pIdentity = GXPLATFORM_WIN32_DIRECT3D9;
}

//GXDWORD GXCALLBACK IGXPlatform_Win32D3D11::UITask(GXLPVOID lParam)
//{
//#ifndef _DEV_DISABLE_UI_CODE
//  CLMTCREATESTRUCT* pCreateParam = (CLMTCREATESTRUCT*)lParam;
//  IGXPlatform_Win32D3D11* pPlatform = (IGXPlatform_Win32D3D11*)pCreateParam->pUserParam;
//  GXApp* pGXApp = (GXApp*)pPlatform->m_pApp;
//  
//  GXGraphics* pGraphics = pGXApp->GetGraphicsUnsafe();
//  pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
//  GXHRESULT hval = pGXApp->OnCreate();
//  pGraphics->Activate(FALSE);
//
//  if(GXFAILED(hval)) {
//    return hval;
//  }
//
//  GXMSG gxmsg;
//  while(1)
//  {
//    gxGetMessage(&gxmsg, NULL);
//    gxDispatchMessageW(&gxmsg);
//    pPlatform->AppHandle(gxmsg.message, gxmsg.wParam, gxmsg.lParam);
//
//    if(gxmsg.message == GXWM_QUIT)
//      break;
//  }
//#endif // _DEV_DISABLE_UI_CODE
//
//  pGXApp->OnDestroy();
//  return NULL;
//}

GXLPCWSTR IGXPlatform_Win32D3D11::GetRootDir()
{
  return m_strRootDir;
}

//////////////////////////////////////////////////////////////////////////

namespace GrapXToDX11
{
              
//GXFMT_R8G8B8               
//GXFMT_A8R8G8B8             
//GXFMT_X8R8G8B8             
//GXFMT_R5G6B5               
//GXFMT_X1R5G5B5             
//GXFMT_A1R5G5B5             
//GXFMT_A4R4G4B4             
//GXFMT_R3G3B2               
//GXFMT_A8                   
//GXFMT_A8R3G3B2             
//GXFMT_X4R4G4B4             
//GXFMT_A2B10G10R10          
//GXFMT_A8B8G8R8             
//GXFMT_X8B8G8R8             
//GXFMT_G16R16               
//GXFMT_A2R10G10B10          
//GXFMT_A16B16G16R16         
//GXFMT_A8P8                 
//GXFMT_P8                   
//GXFMT_L8                   
//GXFMT_A8L8                 
//GXFMT_A4L4                 
//GXFMT_V8U8                 
//GXFMT_L6V5U5               
//GXFMT_X8L8V8U8             
//GXFMT_Q8W8V8U8             
//GXFMT_V16U16               
//GXFMT_A2W10V10U10          
//GXFMT_UYVY                 
//GXFMT_R8G8_B8G8            
//GXFMT_YUY2                 
//GXFMT_G8R8_G8B8            
//GXFMT_DXT1                 
//GXFMT_DXT2                 
//GXFMT_DXT3                 
//GXFMT_DXT4                 
//GXFMT_DXT5                 
//GXFMT_D16_LOCKABLE         
//GXFMT_D32                  
//GXFMT_D15S1                
//GXFMT_D24S8                
//GXFMT_D24X8                
//GXFMT_D24X4S4              
//GXFMT_D16                  
//GXFMT_D32F_LOCKABLE        
//GXFMT_D24FS8               
//GXFMT_D32_LOCKABLE         
//GXFMT_S8_LOCKABLE          
//GXFMT_L16                  
//GXFMT_VERTEXDATA           
//GXFMT_INDEX16              
//GXFMT_INDEX32              
//GXFMT_Q16W16V16U16         
//GXFMT_MULTI2_ARGB8
//GXFMT_R16F                 
//GXFMT_G16R16F              
//GXFMT_A16B16G16R16F        
//GXFMT_R32F                 
//GXFMT_G32R32F              
//GXFMT_A32B32G32R32F        
//GXFMT_CxV8U8               
//GXFMT_A1                   
//GXFMT_A2B10G10R10_XR_BIAS  
//GXFMT_BINARYBUFFER         

  DXGI_FORMAT FormatFrom(GXFormat eFormat)
  {
    switch(eFormat)
    {
    case GXFMT_UNKNOWN: return DXGI_FORMAT_UNKNOWN;
    //case :return DXGI_FORMAT_R32G32B32A32_TYPELESS;
    //case :return DXGI_FORMAT_R32G32B32A32_FLOAT;
    //case :return DXGI_FORMAT_R32G32B32A32_UINT;
    //case :return DXGI_FORMAT_R32G32B32A32_SINT;
    //case :return DXGI_FORMAT_R32G32B32_TYPELESS;
    //case :return DXGI_FORMAT_R32G32B32_FLOAT;
    //case :return DXGI_FORMAT_R32G32B32_UINT;
    //case :return DXGI_FORMAT_R32G32B32_SINT;
    //case :return DXGI_FORMAT_R16G16B16A16_TYPELESS;
    //case :return DXGI_FORMAT_R16G16B16A16_FLOAT;
    //case :return DXGI_FORMAT_R16G16B16A16_UNORM;
    //case :return DXGI_FORMAT_R16G16B16A16_UINT;
    //case :return DXGI_FORMAT_R16G16B16A16_SNORM;
    //case :return DXGI_FORMAT_R16G16B16A16_SINT;
    //case :return DXGI_FORMAT_R32G32_TYPELESS;
    //case :return DXGI_FORMAT_R32G32_FLOAT;
    //case :return DXGI_FORMAT_R32G32_UINT;
    //case :return DXGI_FORMAT_R32G32_SINT;
    //case :return DXGI_FORMAT_R32G8X24_TYPELESS;
    //case :return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    //case :return DXGI_FORMAT_R10G10B10A2_TYPELESS;
    //case :return DXGI_FORMAT_R10G10B10A2_UNORM;
    //case :return DXGI_FORMAT_R10G10B10A2_UINT;
    //case :return DXGI_FORMAT_R11G11B10_FLOAT;
    //case :  return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    //case :return DXGI_FORMAT_R8G8B8A8_UNORM;
    //case :return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case GXFMT_A8R8G8B8:  return DXGI_FORMAT_B8G8R8A8_UNORM;
    case GXFMT_A8B8G8R8:  return DXGI_FORMAT_R8G8B8A8_UNORM;
    //case :return DXGI_FORMAT_R8G8B8A8_SNORM;
    //case :return DXGI_FORMAT_R8G8B8A8_SINT;
    //case :return DXGI_FORMAT_R16G16_TYPELESS;
    //case :return DXGI_FORMAT_R16G16_FLOAT;
    //case :return DXGI_FORMAT_R16G16_UNORM;
    //case :return DXGI_FORMAT_R16G16_UINT;
    //case :return DXGI_FORMAT_R16G16_SNORM;
    //case :return DXGI_FORMAT_R16G16_SINT;
    //case :return DXGI_FORMAT_R32_TYPELESS;
    //case :return DXGI_FORMAT_D32_FLOAT;
    //case :return DXGI_FORMAT_R32_FLOAT;
    //case :return DXGI_FORMAT_R32_UINT;
    //case :return DXGI_FORMAT_R32_SINT;
    //case :return DXGI_FORMAT_R24G8_TYPELESS;
    //case :return DXGI_FORMAT_D24_UNORM_S8_UINT;
    //case :return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    //case :return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
    //case :return DXGI_FORMAT_R8G8_TYPELESS;
    //case :return DXGI_FORMAT_R8G8_UNORM;
    //case :return DXGI_FORMAT_R8G8_UINT;
    //case :return DXGI_FORMAT_R8G8_SNORM;
    //case :return DXGI_FORMAT_R8G8_SINT;
    //case :return DXGI_FORMAT_R16_TYPELESS;
    //case :return DXGI_FORMAT_R16_FLOAT;
    //case :return DXGI_FORMAT_D16_UNORM;
    //case :return DXGI_FORMAT_R16_UNORM;
    //case :return DXGI_FORMAT_R16_UINT;
    //case :return DXGI_FORMAT_R16_SNORM;
    //case :return DXGI_FORMAT_R16_SINT;
    //case :return DXGI_FORMAT_R8_TYPELESS;
    //case :return DXGI_FORMAT_R8_UNORM;
    //case :return DXGI_FORMAT_R8_UINT;
    //case :return DXGI_FORMAT_R8_SNORM;
    //case :return DXGI_FORMAT_R8_SINT;
    case GXFMT_A8:  return DXGI_FORMAT_A8_UNORM;
    //case :return DXGI_FORMAT_R1_UNORM;
    //case :return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    //case :return DXGI_FORMAT_R8G8_B8G8_UNORM;
    //case :return DXGI_FORMAT_G8R8_G8B8_UNORM;
    //case :return DXGI_FORMAT_BC1_TYPELESS;
    //case :return DXGI_FORMAT_BC1_UNORM;
    //case :return DXGI_FORMAT_BC1_UNORM_SRGB;
    //case :return DXGI_FORMAT_BC2_TYPELESS;
    //case :return DXGI_FORMAT_BC2_UNORM;
    //case :return DXGI_FORMAT_BC2_UNORM_SRGB;
    //case :return DXGI_FORMAT_BC3_TYPELESS;
    //case :return DXGI_FORMAT_BC3_UNORM;
    //case :return DXGI_FORMAT_BC3_UNORM_SRGB;
    //case :return DXGI_FORMAT_BC4_TYPELESS;
    //case :return DXGI_FORMAT_BC4_UNORM;
    //case :return DXGI_FORMAT_BC4_SNORM;
    //case :return DXGI_FORMAT_BC5_TYPELESS;
    //case :return DXGI_FORMAT_BC5_UNORM;
    //case :return DXGI_FORMAT_BC5_SNORM;
    //case :return DXGI_FORMAT_B5G6R5_UNORM;
    //case :return DXGI_FORMAT_B5G5R5A1_UNORM;
    //case :return DXGI_FORMAT_B8G8R8A8_UNORM;
    //case :return DXGI_FORMAT_B8G8R8X8_UNORM;
    //case :return DXGI_FORMAT_B8G8R8A8_TYPELESS;
    //case :return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    //case :return DXGI_FORMAT_B8G8R8X8_TYPELESS;
    //case :return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    //case :return DXGI_FORMAT_BC6H_TYPELESS;
    //case :return DXGI_FORMAT_BC6H_UF16;
    //case :return DXGI_FORMAT_BC6H_SF16;
    //case :return DXGI_FORMAT_BC7_TYPELESS;
    //case :return DXGI_FORMAT_BC7_UNORM;
    //case :return DXGI_FORMAT_BC7_UNORM_SRGB;
    //case :return DXGI_FORMAT_FORCE_UINT;
    default:
      ASSERT(0);
    }
    return DXGI_FORMAT_UNKNOWN;
  }

  void PrimitiveDescFromResUsage(IN GXDWORD ResUsage, D3D11_BUFFER_DESC* pDesc)
  {
    if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      pDesc->Usage = D3D11_USAGE_STAGING;
      pDesc->CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    }
    if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags |= (D3D11_CPU_ACCESS_WRITE);
    }
  }

  void TextureDescFromResUsage(IN GXDWORD ResUsage, D3D11_TEXTURE2D_DESC* pDesc)
  {
    if(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM))
    {
      pDesc->BindFlags = 0;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
      //pDesc->Usage = D3D11_USAGE_STAGING;
      return;
    }
    
    if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET))
    {
      pDesc->BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      ASSERT( ! TEST_FLAG(ResUsage, GXRU_TEST_WRITE));
      pDesc->CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
      //pDesc->Usage = D3D11_USAGE_STAGING;
    }

    if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      ASSERT( ! TEST_FLAG(ResUsage, GXRU_TEST_READ));
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
    }
  }

  D3D11_MAP PrimitiveMapFromResUsage(IN GXDWORD ResUsage)
  {
    if(TEST_FLAG(ResUsage, GXRU_TEST_READ) && TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      return D3D11_MAP_READ_WRITE;
    }
    else if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      return D3D11_MAP_READ;
    }
    else if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      return D3D11_MAP_WRITE_DISCARD;
    }
    ASSERT(0);
    return (D3D11_MAP)0;
  }
  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology(GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount)
  {
    switch(eType)
    {
    case GXPT_POINTLIST:
      *pVertCount = nPrimCount;
      return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    case GXPT_LINELIST:
      *pVertCount = nPrimCount * 2;
      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    case GXPT_LINESTRIP:
      *pVertCount = nPrimCount + 1;
      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

    case GXPT_TRIANGLELIST:
      *pVertCount = nPrimCount * 3;
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    case GXPT_TRIANGLESTRIP:
      *pVertCount = nPrimCount + 2;
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    //case GXPT_TRIANGLEFAN:    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
    }
    ASSERT(0);
    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  }

  void VertexLayoutFromVertexDecl(LPCGXVERTEXELEMENT pVerticesDecl, GXD3D11InputElementDescArray* pArray)
  {
#define CASE_USAGE(USAGE) case GXDECLUSAGE_##USAGE: Desc.SemanticName = #USAGE; break;
    GXD3D11_INPUT_ELEMENT_DESC Desc;

    int nCount = GetVertexDeclLength<int>(pVerticesDecl);
    pArray->reserve(nCount + 1);

    for(int i = 0; i < nCount; i++)
    {
      Desc.SemanticIndex = 0;
      switch(pVerticesDecl[i].Usage)
      {
      CASE_USAGE(POSITION);
      CASE_USAGE(BLENDWEIGHT);
      CASE_USAGE(BLENDINDICES);
      CASE_USAGE(NORMAL);
      //CASE_USAGE(PSIZE);
      CASE_USAGE(TEXCOORD);
      CASE_USAGE(TANGENT);
      CASE_USAGE(BINORMAL);
      //CASE_USAGE(TESSFACTOR);
      CASE_USAGE(POSITIONT);
      CASE_USAGE(COLOR);
      }
      switch(pVerticesDecl[i].Type)
      {
      case GXDECLTYPE_FLOAT1:   Desc.Format = DXGI_FORMAT_R32_FLOAT;          break;
      case GXDECLTYPE_FLOAT2:   Desc.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
      case GXDECLTYPE_FLOAT3:   Desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
      case GXDECLTYPE_FLOAT4:   Desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
      case GXDECLTYPE_D3DCOLOR: Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  break;
      }

      Desc.InputSlot            = 0;
      Desc.AlignedByteOffset    = pVerticesDecl[i].Offset;
      Desc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
      Desc.InstanceDataStepRate = 0;

      pArray->push_back(Desc);
    }
    Desc.SemanticName         = "";
    Desc.SemanticIndex        = -1;
    Desc.Format               = DXGI_FORMAT_UNKNOWN;
    Desc.InputSlot            = -1;
    Desc.AlignedByteOffset    = -1;
    Desc.InputSlotClass       = (D3D11_INPUT_CLASSIFICATION)-1;
    Desc.InstanceDataStepRate = -1;
    pArray->push_back(Desc);
  }
  
  D3D11_FILTER FilterFrom(GXTextureFilterType eMag, GXTextureFilterType eMin, GXTextureFilterType eMip)
  {
    if(eMip == 0) {
      eMip = GXTEXFILTER_POINT;
    }
    ASSERT((eMag == 1 || eMag == 2) && (eMin == 1 || eMin == 2) && (eMip == 1 || eMip == 2));
    return D3D11_ENCODE_BASIC_FILTER(eMin - 1, eMag - 1, eMip - 1, 0);
  }

} // namespace GrapXToDX11

IGXPlatform_Win32D3D11* AppCreateD3D11Platform(GXApp* pApp, GXAPP_DESC* pDesc, GXGraphics** ppGraphics)
{
  return AppCreatePlatformT<IGXPlatform_Win32D3D11>(pApp, pDesc, ppGraphics);
}
//////////////////////////////////////////////////////////////////////////

STATIC_ASSERT(GXTADDRESS_WRAP       == D3D11_TEXTURE_ADDRESS_WRAP);
STATIC_ASSERT(GXTADDRESS_MIRROR     == D3D11_TEXTURE_ADDRESS_MIRROR);
STATIC_ASSERT(GXTADDRESS_CLAMP      == D3D11_TEXTURE_ADDRESS_CLAMP);
STATIC_ASSERT(GXTADDRESS_BORDER     == D3D11_TEXTURE_ADDRESS_BORDER);
STATIC_ASSERT(GXTADDRESS_MIRRORONCE == D3D11_TEXTURE_ADDRESS_MIRROR_ONCE);

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
