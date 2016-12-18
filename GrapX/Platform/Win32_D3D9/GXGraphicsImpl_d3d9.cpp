#ifdef ENABLE_GRAPHICS_API_DX9
//#define D3D9_LOW_DEBUG
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_CANVAS_D3D9_
#define _GXGRAPHICS_INLINE_RENDERTARGET_D3D9_
#define _GXGRAPHICS_INLINE_TEXTURE_D3D9_
#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
//#include "GrapX/GUnknown.H"
#include "GrapX/GResource.H"
#include "GrapX/GRegion.H"
#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.H"
#include "GrapX/GStateBlock.H"
#include "GrapX/GTexture.H"
#include "GrapX/GXGraphics.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXImage.H"
#include "GrapX/GXFont.H"
#include "GrapX/GXCanvas3D.h"
#include "GrapX/MOLogger.h"

// 平台相关
#include "GrapX/Platform.h"
#include "GrapX/DataPool.H"
#include "GrapX/DataPoolVariable.H"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D9.h"
#include "Platform/Win32_D3D9/GPrimitiveImpl_d3d9.h"
#include "Platform/Win32_D3D9/GShaderImpl_d3d9.h"
#include "Platform/Win32_D3D9/GShaderStubImpl_d3d9.h"
#include "Platform/Win32_D3D9/GVertexDeclImpl_d3d9.H"

// 私有头文件
#include <clPathFile.H>
#include "Platform/Win32_D3D9/GStateBlock_d3d9.H"
#include <GrapX/VertexDecl.H>
#include "Canvas/GXResourceMgr.h"
#include "Canvas/GXEffectImpl.h"
//#include "Console.h"
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStock.h>

#include "GrapX/GXKernel.H"
#include "GrapX/GXUser.H"
#include <GDI/RegionFunc.H>
#include <GDI/GRegionImpl.H>
#include "Platform/Win32_D3D9/GTextureImpl_d3d9.H"
#include "Platform/Win32_D3D9/GTexture3DImpl_d3d9.H"

#include "Canvas/GXImageImpl.H"
#include "Platform/Win32_D3D9/GXCanvasImpl_d3d9.H"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <Canvas/GFTFontImpl.H>
#include <GDI/GXShaderMgr.h>

#include "GrapX/gxError.H"

#include "Platform/Win32_D3D9/GXGraphicsImpl_d3d9.H"
// Canvas3D用的
#include "GrapX/GCamera.h"
#include "GrapX/GrapVR.H"  // Canvas3D 用的
#include "Canvas/GXMaterialImpl.h"
// </Canvas3D用的>

//////////////////////////////////////////////////////////////////////////
#define D3D9_GRAPHICS_IMPL

namespace D3D9
{
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/GXGraphicsImpl.inl"

  GXGraphics* GXGraphicsImpl::Create(const GRAPHICS_CREATION_DESC* pDesc)
  {
    GXGraphicsImpl* pGraphics = new GXGraphicsImpl;

    if( ! InlCheckNewAndIncReference(pGraphics)) {
      return NULL;
    }

    if(pGraphics->Initialize(pDesc) != FALSE) {
      return (GXGraphics*)pGraphics;
    }

    pGraphics->Release();
    return NULL;
  }

  GXGraphicsImpl::GXGraphicsImpl()
    : s_uCanvasCacheCount  (4)
    , m_hWnd                (NULL)
    , m_pd3dDevice          (NULL)
    , m_dwFlags             (NULL)
    , m_pIdentity           (GXPLATFORM_UNKNOWN)
    , m_pSimpleShader       (NULL)
    , m_pSimpleEffect       (NULL)
    , m_pD3DOriginSur       (NULL)
    , m_pD3DDepthStencilSur (NULL)
    , m_pDeviceOriginTex    (NULL)
    , m_pCurPrimitive       (NULL)
    , m_pCurRenderTarget    (NULL)
    , m_pCurDepthStencil    (NULL)
    , m_aCanvasPtrCache     (NULL)
    , m_pCurVertexDecl      (NULL)
    , m_pCurCanvasCore      (NULL)
    , m_pDefaultRasterizerState(NULL)
    , m_pCurRasterizerState (NULL)
    , m_pDefaultBlendState  (NULL)
    , m_pDefaultDepthStencilState(NULL)
    , m_pCurBlendState      (NULL)
    , m_pCurDepthStencilState(NULL)
    , m_pCurSamplerState    (NULL)
    , m_pDefaultSamplerState(NULL)
    , m_pCurShader          (NULL)
    , m_pBackBufferTex      (NULL)
    , m_pBackBufferImg      (NULL)
    , m_dwBackBufferStencil (1)
    , m_pGraphicsLocker     (NULL)
    , m_nGraphicsCount      (0)
    , m_dwThreadId          (0)
    //, m_pConsole            (NULL)
    , m_pLogger             (NULL)
    , m_pRgnAllocator       (NULL)
    , m_pShaderConstName    (NULL)
  {
    InlSetZeroT(m_d3dpp);
    memset(m_pCurTexture, 0, sizeof(GTexture*) * MAX_TEXTURE_STAGE);
    m_pRgnAllocator = new GAllocator(NULL);
  }

  GXBOOL GXGraphicsImpl::Initialize(const GRAPHICS_CREATION_DESC* pDesc)
  {
    m_strResourceDir = pDesc->szRootDir;
    m_hWnd = pDesc->hWnd;
    m_dwFlags |= F_CREATEDEVICE;

    m_d3dpp.Windowed               = TRUE;
    m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.BackBufferFormat       = D3DFMT_UNKNOWN;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.BackBufferCount        = 1;
    //m_d3dpp.MultiSampleType     = D3DMULTISAMPLE_8_SAMPLES;
    m_d3dpp.FullScreen_RefreshRateInHz = 0;
    m_d3dpp.PresentationInterval   = pDesc->bWaitForVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

    GXUINT AdapterToUse = D3DADAPTER_DEFAULT;
    D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

    m_pIdentity = GXPLATFORM_WIN32_DIRECT3D9;

    if(pDesc->pLogger) {
      m_pLogger = pDesc->pLogger;
      m_pLogger->AddRef();
    }
    else {
      clStringW strFilePath;
      clpathfile::CombinePathW(strFilePath, pDesc->szRootDir, L"grapx.log");
      MOCreateFileLoggerW(&m_pLogger, strFilePath, FALSE);
    }

    //m_pConsole = new GXConsole;
    //if(m_pConsole->Initialize(this) != 0) {
    //  TRACE("Console 创建失败!\n");
    //  ASSERT(0);
    //}


#ifdef _DEBUG
    for (GXUINT Adapter = 0 ;Adapter < pDesc->lpD3D->GetAdapterCount() ;Adapter++)
    {
      D3DADAPTER_IDENTIFIER9 Identifier;
      GXHRESULT Res;
      Res = pDesc->lpD3D->GetAdapterIdentifier(Adapter,0,&Identifier);
      if (strstr(Identifier.Description,"PerfHUD") != 0)
      {
        AdapterToUse = Adapter;
        DeviceType = D3DDEVTYPE_REF;
        break;
      }
    }
#endif
    //m_pGraphicsLocker = new clstd::Locker;

    if( FAILED( pDesc->lpD3D->CreateDevice( AdapterToUse, DeviceType, pDesc->hWnd,
      D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
      &m_d3dpp, &m_pd3dDevice ) ) )
    {
      return FALSE;
    }

    InitCommon();

    if(pDesc->pParameter)
    {
      const GXDEFINITION* pParameter = pDesc->pParameter;
      for (int i = 0; 
        (pParameter[i].szName != NULL || pParameter[i].szValue != NULL); 
        i++)
      {
        if(GXSTRCMP(pParameter[i].szName, "GraphicsEnvSet/ShaderComposer") == 0)
        {
          clStringW strPathW(pParameter[i].szValue);
          SetShaderComposerPathW(strPathW);
        }
      }
    }

    if(GXSUCCEEDED(CreateShaderFromFileA(&m_pSimpleShader, "shaders\\Simple.shader.txt")))
    {
      CreateEffect(&m_pSimpleEffect, m_pSimpleShader);
    }
    else
    {
      TRACE("初始化SimpleShader失败!\n");
    }

    return TRUE;
  }



#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GXGraphicsImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GXGraphicsImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);

    if(nRefCount > 0) {
      return nRefCount;
    }

    SAFE_RELEASE(m_pSimpleEffect);
    SAFE_RELEASE(m_pSimpleShader);
    SAFE_RELEASE(m_pCurShader);

    ReleaseCommon();
    
    for(RTTexSet::iterator it = m_setRTTexture.begin();
      it != m_setRTTexture.end(); ++it) {
      (*it)->Release();
    }
    m_setRTTexture.clear();


    //m_pShaderMgr->Destroy();
    //delete m_pShaderMgr;

    SAFE_RELEASE(m_pBackBufferTex);
    SAFE_RELEASE(m_pBackBufferImg);

    //SAFE_RELEASE(m_pCurRenderState);
    SAFE_RELEASE(m_pCurPrimitive);
    for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
      SAFE_RELEASE(m_pCurTexture[i]);
    }
    SAFE_RELEASE(m_pCurDepthStencil);
    SAFE_RELEASE(m_pCurRenderTarget);
    SAFE_RELEASE(m_pCurCanvasCore);

    SAFE_RELEASE(m_pCurVertexDecl);
    //OnDeviceEvent(DE_LostDevice);
    INVOKE_LOST_DEVICE;
    if(m_dwFlags & F_CREATEDEVICE)
    {
      SAFE_RELEASE(m_pd3dDevice);
      memset(&m_d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS));
      m_hWnd = NULL;
    }
    SAFE_DELETE(m_pRgnAllocator);

//#ifdef _DEBUG
//    if(m_aResource.size() > 0)
//      TRACE("GXGraphics 释放后有 %d 个对象没有正常释放.\n", m_aResource.size());
//#endif // _DEBUG
    //m_pConsole->Finalize();
    //delete m_pConsole;
    
    SAFE_RELEASE(m_pLogger);

    SAFE_DELETE(m_pGraphicsLocker);
    delete this;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);

    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      {
        InlSetCanvas(NULL);
        InlSetShader(NULL);
        InlSetVertexDecl(NULL);
        SetPrimitiveVI(NULL, 0);
        for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
          InlSetTexture(NULL, i);
        }

        SAFE_RELEASE(m_pDeviceOriginTex);
        SAFE_RELEASE(m_pD3DOriginSur);
        SAFE_RELEASE(m_pD3DDepthStencilSur);
      }
      break;
    case RC_ResetDevice:
      {
        m_pd3dDevice->GetRenderTarget(0, &m_pD3DOriginSur);
        m_pd3dDevice->GetDepthStencilSurface(&m_pD3DDepthStencilSur);
        CreateTextureFromD3DSurface(&m_pDeviceOriginTex, m_pD3DOriginSur);

        //m_pCurRenderState->Update(NULL);
        m_pCurRasterizerState->Activate(NULL);
        m_pCurBlendState->Activate(NULL);
        m_pCurDepthStencilState->Activate(NULL);
        m_pCurSamplerState->Activate(NULL);
      }
      break;
    case RC_ResizeDevice:
      {

      }
      break;
    }
    return FALSE;
  }
  void GXGraphicsImpl::GetPlatformID(GXPlaformIdentity* pIdentity)
  {
    ASSERT(m_pIdentity == GXPLATFORM_WIN32_DIRECT3D9);
    *pIdentity = m_pIdentity;
  }

  GXBOOL GXGraphicsImpl::Activate(GXBOOL bActive)
  {
    if(bActive != FALSE) {
      SET_FLAG(m_dwFlags, F_ACTIVATE);
    }
    else {
      RESET_FLAG(m_dwFlags, F_ACTIVATE);

      // TODO: 忘了这里为什么必须要恢复原来的BackBuffer
      if(m_pCurRenderTarget != m_pDeviceOriginTex) {
        //InlSetRenderTarget(m_pDeviceOriginTex, 0);
        InlSetCanvas(NULL);
      }

#if defined(_DEBUG) && 0
      LPDIRECT3DSURFACE9 lpRenderTar;
      m_pd3dDevice->GetRenderTarget(0, &lpRenderTar);
      ASSERT(lpRenderTar == m_pCurRenderTarget->D3DSurface());
      lpRenderTar->Release();
#endif

      // 2012-08-04 没有必要恢复渲染区域吧!
      //GXREGN regn = {0,0,m_d3dpp.BackBufferWidth,m_d3dpp.BackBufferHeight};
      //SetSafeClip(&regn);
    }
    return TRUE;
  }

  GXHRESULT GXGraphicsImpl::Test()
  {
    GRESCRIPTDESC Desc;
    const GXHRESULT hr = m_pd3dDevice->TestCooperativeLevel();

    switch(hr)
    {
    case D3DERR_DEVICELOST:
      {
        m_pGraphicsLocker->Lock();
        TRACE("D3DERR_DEVICELOST\n");
        InlSetZeroT(Desc);
        Desc.dwCmdCode = RC_LostDevice;

        Invoke(&Desc);
        BroadcastScriptCommand(&Desc);
        m_dwFlags |= F_DEVICEHASLOST;
        m_pGraphicsLocker->Unlock();
      }
      break;
    case D3DERR_DEVICENOTRESET:
      {
        m_pGraphicsLocker->Lock();
        TRACE("D3DERR_DEVICENOTRESET\n");
        if((m_dwFlags & F_DEVICEHASLOST) == 0)
        {
          InlSetZeroT(Desc);
          Desc.dwCmdCode = RC_LostDevice;

          Invoke(&Desc);
          BroadcastScriptCommand(&Desc);
        }

        // Reset 需要在创建D3D设备的线程来调用
        if(m_dwFlags & F_CREATEDEVICE)
        {
          GXLRESULT hval = SendMessage(m_hWnd, WM_GX_RESETDEVICED3D9, NULL, (LPARAM)&m_d3dpp);
          V(hval);
        }
        InlSetZeroT(Desc);
        Desc.dwCmdCode = RC_ResetDevice;

        Invoke(&Desc);
        BroadcastScriptCommand(&Desc);
        m_dwFlags &= (~F_DEVICEHASLOST);
        m_pGraphicsLocker->Unlock();
      }
      break;
    case D3DERR_DRIVERINTERNALERROR:
      {
        TRACE("D3DERR_DRIVERINTERNALERROR\n");
      }
      break;
    }
    return hr;
  }
  GXHRESULT GXGraphicsImpl::Begin()
  {
    m_pGraphicsLocker->Lock();

    if((++m_nGraphicsCount) == 1)
    {
      if(GXSUCCEEDED(Test()) &&
        GXSUCCEEDED(m_pd3dDevice->BeginScene()))
      {
        m_sStatistics.nDrawCount    = 0;
        m_sStatistics.nShaderSwitch = 0;
        m_sStatistics.nTriangles    = 0;
        m_sStatistics.nVertices     = 0;

        Activate(TRUE);
        m_dwThreadId = GetCurrentThreadId();
        return m_nGraphicsCount;
      }
      --m_nGraphicsCount;
      m_pGraphicsLocker->Unlock();
      return GX_FAIL;
    }
    return m_nGraphicsCount;
  }

  GXHRESULT GXGraphicsImpl::End()
  {
    GXINT nCount = --m_nGraphicsCount;
    if(nCount == 0)
    {
      if(TEST_FLAG(m_dwFlags, F_SHOWCONSOLE))
      {
        GXCanvas* pCanvas = LockCanvas(NULL, NULL, NULL);
        //m_pConsole->Render(pCanvas);
        SAFE_RELEASE(pCanvas);
      }

      ASSERT(m_dwThreadId == GetCurrentThreadId());
      m_dwThreadId = NULL;
      Activate(FALSE);

      HRESULT hval = m_pd3dDevice->EndScene();
      ASSERT(SUCCEEDED(hval));
    }
    m_pGraphicsLocker->Unlock();
    return nCount;
  }

  GXHRESULT GXGraphicsImpl::Present()
  {
    // 这个放到Locker外面，避免Window窗口Resize时偶尔导致很久才响应。
    HRESULT hval =  m_pd3dDevice->Present(0, 0, 0, 0);

    m_pGraphicsLocker->Lock();  // TODO: 想想怎么节省这个Lock
#ifdef _DEBUG
    // 多线程时改变设备尺寸可能会导致失败
    if(GXFAILED(hval)) {
      TRACE("Warning: Present failed(Code: %d).\n", hval);
    }
#endif // _DEBUG

    // 备份需要备份的RT纹理
    for(RTTexSet::iterator it = m_setRTTexture.begin();
      it != m_setRTTexture.end(); ++it) {
        (*it)->IntHoldRenderTarget();
        (*it)->Release();
    }
    m_setRTTexture.clear();

    m_pGraphicsLocker->Unlock();
    return hval;
  }

  GXHRESULT GXGraphicsImpl::Resize(int nWidth, int nHeight)
  {
    GRESCRIPTDESC Desc;
    InlSetZeroT(Desc);
    Desc.dwCmdCode = RC_LostDevice;
    Invoke(&Desc);
    BroadcastScriptCommand(&Desc);

    m_d3dpp.BackBufferWidth = nWidth;
    m_d3dpp.BackBufferHeight = nHeight;
    HRESULT hval = m_pd3dDevice->Reset(&m_d3dpp);
    if(FAILED(hval)) {
      CLOG_ERROR("Can't reset d3d9 device.\n");
      CLBREAK;
    }

    Desc.dwCmdCode = RC_ResetDevice;
    Invoke(&Desc);
    BroadcastScriptCommand(&Desc);

    //Activate(FALSE);
    return GX_OK;
  }
  
  GXVOID GXGraphicsImpl::SetShaderComposerPathW(GXLPCWSTR szPath)
  {
    m_strShaderComposerPath = szPath;
    
    GShader::Load(m_strShaderComposerPath, m_strResourceDir, 
      InlGetPlatformStringA(), &m_ShaderExtElement, NULL);
  }

  GXHRESULT GXGraphicsImpl::SetPrimitiveV(GPrimitiveV* pPrimitive, GXUINT uStreamSource)
  {
    if(m_pCurPrimitive == pPrimitive)
      return S_OK;

    if(m_pCurPrimitive != NULL)
      m_pCurPrimitive->Release();

    m_pCurPrimitive = pPrimitive;
    if(m_pCurPrimitive == NULL)
      return S_OK;

    m_pCurPrimitive->AddRef();

    // 应用顶点声明
    GPrimitiveVImpl* pPrimImpl = static_cast<GPrimitiveVImpl*>(pPrimitive);
    if(pPrimImpl->m_pVertexDecl != NULL) {
      InlSetVertexDecl(pPrimImpl->m_pVertexDecl);
    }

    GPrimitiveVImpl* pPrimitiveImpl = (GPrimitiveVImpl*) pPrimitive;
    m_pd3dDevice->SetStreamSource(uStreamSource, pPrimitiveImpl->m_pVertexBuffer, 
      0, pPrimitiveImpl->m_uElementSize);

    return S_OK;
  }
  
  GXHRESULT GXGraphicsImpl::SetPrimitiveVI(GPrimitiveVI* pPrimitive, GXUINT uStreamSource)
  {
    if(m_pCurPrimitive == pPrimitive)
      return S_OK;

    SAFE_RELEASE(m_pCurPrimitive);
    m_pCurPrimitive = pPrimitive;

    if(m_pCurPrimitive == NULL)
      return S_OK;

    m_pCurPrimitive->AddRef();

    // 应用顶点声明
    GPrimitiveVIImpl* pPrimImpl = static_cast<GPrimitiveVIImpl*>(pPrimitive);
    if(pPrimImpl->m_pVertexDecl != NULL) {
      InlSetVertexDecl(pPrimImpl->m_pVertexDecl);
    }


    GPrimitiveVIImpl* pPrimitiveImpl = (GPrimitiveVIImpl*)pPrimitive;
    m_pd3dDevice->SetStreamSource(uStreamSource, pPrimitiveImpl->m_pVertexBuffer, 
      0, pPrimitiveImpl->m_uElementSize);

    m_pd3dDevice->SetIndices(pPrimitiveImpl->m_pIndexBuffer);

    return S_OK;
  }
  
  GXHRESULT GXGraphicsImpl::SetTexture(GTextureBase* pTexture, GXUINT uStage)
  {
    return InlSetTexture(reinterpret_cast<GTexBaseImpl*>(pTexture), uStage);
  }

  //////////////////////////////////////////////////////////////////////////
  void GXGraphicsImpl::IntGetDimension(GXUINT& nWidth, GXUINT& nHeight)
  {
    if(m_pCurRenderTarget) {
      m_pCurRenderTarget->GetDimension(&nWidth, &nHeight);
    }
    else {
      nWidth = m_d3dpp.BackBufferWidth;
      nHeight = m_d3dpp.BackBufferHeight;
    }
  }

  GXBOOL GXGraphicsImpl::SetSafeClip(const GXREGN* pRegn)
  {
#ifdef _43A2DE06_6673_4ddd_9C1C_881493B776A0_
    D3DVIEWPORT9 Viewport;
    Viewport.X      = (GXDWORD)pRegn->left;
    Viewport.Y      = (GXDWORD)pRegn->top;
    Viewport.Width  = (GXDWORD)pRegn->width;
    Viewport.Height = (GXDWORD)pRegn->height;
    Viewport.MinZ   = 0.0f;
    Viewport.MaxZ   = 1.0f;

    GXUINT nWidth, nHeight;
    m_pCurRenderTarget->GetDimension(&nWidth, &nHeight);

    //m_matWorld._11 = ( 2.0f / (float)nWidth)  * ((float)nWidth  / (float)Viewport.Width);
    //m_matWorld._22 = (-2.0f / (float)nHeight) * ((float)nHeight / (float)Viewport.Height);
    m_matWorld._11 = ( 2.0f / (float)Viewport.Width);
    m_matWorld._22 = (-2.0f / (float)Viewport.Height);
    m_matWorld._41 = -1.0f - ((float)pRegn->left * 2.0f / (float)Viewport.Width);
    m_matWorld._42 =  1.0f + ((float)pRegn->top  * 2.0f / (float)Viewport.Height);

    //SetVertexShaderConstantF(FXVCOMMREG(matWVProj), (const float*)&m_matWorld, 4);

#ifdef _DEBUG
    if(m_pCurCanvas != NULL && m_pCurCanvas->m_pTargetTex != NULL)
    {
      ASSERT(pRegn->left >= 0 && pRegn->top >= 0);
      GXLONG nScreenWidth, nScreenHeight;
      m_pCurCanvas->m_pTargetTex->GetDimension((GXUINT*)&nScreenWidth, (GXUINT*)&nScreenHeight);
      ASSERT(pRegn->left + pRegn->width <= nScreenWidth && 
        pRegn->top + pRegn->height <= nScreenHeight);
    }
#endif // _DEBUG
    return GXSUCCEEDED(m_pd3dDevice->SetViewport(&Viewport));
#else
    GXRECT rect;
    if(pRegn) {
      gxRegnToRect(&rect, (const GXLPREGN)pRegn);
    }
    else {
      rect.left = 0;
      rect.top = 0;
      IntGetDimension((GXUINT&)rect.right, (GXUINT&)rect.bottom);
    }
    //m_pCurRenderState->Set(GXRS_SCISSORTESTENABLE, TRUE);

    HRESULT hval = m_pd3dDevice->SetScissorRect((RECT*)&rect);
#ifdef _DEBUG
    if(GXFAILED(hval))
    {
      TRACE("%s : failed : %d", __FUNCTION__, hval);
    }
#endif // _DEBUG
    return GXSUCCEEDED(hval);
#endif
  }
  GXBOOL GXGraphicsImpl::SetViewport(const GXVIEWPORT* pViewport)
  {
    D3DVIEWPORT9 Viewport;
    if(pViewport != NULL)
    {
      Viewport.X      = (GXDWORD)pViewport->regn.left;
      Viewport.Y      = (GXDWORD)pViewport->regn.top;
      Viewport.Width  = (GXDWORD)pViewport->regn.width;
      Viewport.Height = (GXDWORD)pViewport->regn.height;
      Viewport.MinZ   = pViewport->fNear;
      Viewport.MaxZ   = pViewport->fFar;
    }
    else
    {
      Viewport.X      = 0;
      Viewport.Y      = 0;
      IntGetDimension((GXUINT&)Viewport.Width, (GXUINT&)Viewport.Height);
      Viewport.MinZ   = 0.f;
      Viewport.MaxZ   = 1.f;
    }

    HRESULT hval = m_pd3dDevice->SetViewport(&Viewport);
#ifdef _DEBUG
    if(GXFAILED(hval))
    {
      TRACE("%s : failed : %d", __FUNCTION__, hval);
    }
#endif // _DEBUG
    return GXSUCCEEDED(hval);
  }

  GXBOOL GXGraphicsImpl::IntCheckSizeOfTargetAndDepth()
  {
    if(m_pCurDepthStencil == 0 && m_pCurRenderTarget == 0) {
      return TRUE;
    }

    DWORD dwStencilEnable = 0;
    DWORD dwDepthEnable = 0;
    m_pd3dDevice->GetRenderState(D3DRS_STENCILENABLE, &dwStencilEnable);
    m_pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwDepthEnable);
    ASSERT(dwStencilEnable == m_pCurDepthStencilState->m_Desc.StencilEnable && 
      dwDepthEnable == m_pCurDepthStencilState->m_Desc.DepthWriteMask);

    GXINT nDepthSurWidth;
    GXINT nDepthSurHeight;
    GXINT nColorSurWidth;
    GXINT nColorSurHeight;

    if(m_pCurDepthStencil != NULL) {
      nDepthSurWidth = m_pCurDepthStencil->m_nWidth;
      nDepthSurHeight = m_pCurDepthStencil->m_nHeight;
    }
    else {
      nDepthSurWidth = m_d3dpp.BackBufferWidth;
      nDepthSurHeight = m_d3dpp.BackBufferHeight;
    }

    if(m_pCurRenderTarget != NULL) {
      nColorSurWidth = m_pCurRenderTarget->m_nWidth;
      nColorSurHeight = m_pCurRenderTarget->m_nHeight;
    }
    else {
      nColorSurWidth = m_d3dpp.BackBufferWidth;
      nColorSurHeight = m_d3dpp.BackBufferHeight;
    }

    if(nDepthSurWidth >= nColorSurWidth &&
      nDepthSurHeight >= nColorSurHeight)
    {
      return TRUE;
    }
    TRACE(" ! Depth and stencil buffer size is less than back buffer\n");
    return FALSE;    
  }

  GXHRESULT GXGraphicsImpl::Clear(const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, D3DCOLOR crClear, float z, GXDWORD dwStencil)
  {
    return m_pd3dDevice->Clear(nCount, (const D3DRECT*)lpRects, dwFlags, crClear, z, dwStencil);
  }

  GXBOOL GXGraphicsImpl::IntCheckRTTexture(GTextureImpl* pRTTexture)
  {
    // 接口设计验证
    ASSERT(pRTTexture != NULL);
#ifdef _DEBUG
    if(pRTTexture == m_pCurRenderTarget) {
      LPDIRECT3DSURFACE9 pCurSurface;
      m_pd3dDevice->GetRenderTarget(0, &pCurSurface);
      ASSERT(pCurSurface != m_pD3DOriginSur);
      SAFE_RELEASE(pCurSurface);
    }
#endif // m_setRTTexture
    ASSERT(pRTTexture->GetCreationType() == GTextureImpl::User || pRTTexture->GetCreationType() == GTextureImpl::D3DSurfaceRef);

    GTextureFromUser* pUserTexture = static_cast<GTextureFromUser*>(pRTTexture);

    if(pUserTexture->IntIsNeedBackupRTTexture() && 
      m_setRTTexture.find(pUserTexture) == m_setRTTexture.end())
    {
      pUserTexture->AddRef();
      m_setRTTexture.insert(pUserTexture);
    }
    return TRUE;
  }

  GXHRESULT GXGraphicsImpl::DrawPrimitive(const GXPrimitiveType eType, const GXUINT StartVertex, 
    const GXUINT PrimitiveCount)
  {
    ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));
#ifdef _DEBUG
    IntCheckSizeOfTargetAndDepth();
    if(PrimitiveCount == 0) {
      CLBREAK;
    }
#endif // IntCheckSizeOfTargetAndDepth

    m_sStatistics.nDrawCount++;
    if(eType == GXPT_TRIANGLEFAN || eType == GXPT_TRIANGLELIST || eType == GXPT_TRIANGLESTRIP) {
      m_sStatistics.nTriangles += PrimitiveCount;
    }

    HRESULT hval = m_pd3dDevice->DrawPrimitive((D3DPRIMITIVETYPE)eType, StartVertex, PrimitiveCount);

    // 检查当前渲染目标,如果是一个纹理就根据其标志检查是否需要备份内容
    if(SUCCEEDED(hval)) {
      SET_FLAG(m_dwFlags, F_HASDRAWCALL);
    }

    return hval;
  }

  GXHRESULT GXGraphicsImpl::DrawPrimitive(
    const GXPrimitiveType eType, const GXINT BaseVertexIndex, 
    const GXUINT MinIndex, const GXUINT NumVertices, 
    const GXUINT StartIndex, const GXUINT PrimitiveCount)
  {
#ifdef D3D9_LOW_DEBUG
    LPDIRECT3DPIXELSHADER9 pPixelShader;
    LPDIRECT3DVERTEXSHADER9 pVertexShader;
    m_pd3dDevice->GetPixelShader(&pPixelShader);
    m_pd3dDevice->GetVertexShader(&pVertexShader);
    ASSERT(pPixelShader != NULL && pVertexShader != NULL);
    SAFE_RELEASE(pPixelShader);
    SAFE_RELEASE(pVertexShader);

    LPDIRECT3DINDEXBUFFER9 pIndexBuf;
    m_pd3dDevice->GetIndices(&pIndexBuf);
    ASSERT(pIndexBuf != NULL);
    SAFE_RELEASE(pIndexBuf);

#endif // D3D9_LOW_DEBUG
#ifdef _DEBUG
    IntCheckSizeOfTargetAndDepth();
    if(PrimitiveCount == 0) {
      CLBREAK;
    }
#endif // IntCheckSizeOfTargetAndDepth
    if(eType == GXPT_POINTLIST) {
      CLOG_ERROR("Can't draw indexed point list primitive.\n");
      CLBREAK;
    }
    ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));

    m_sStatistics.nDrawCount++;
    if(eType == GXPT_TRIANGLEFAN || eType == GXPT_TRIANGLELIST || eType == GXPT_TRIANGLESTRIP) {
      m_sStatistics.nTriangles += PrimitiveCount;
    }

    HRESULT hval = m_pd3dDevice->DrawIndexedPrimitive(
      (D3DPRIMITIVETYPE)eType, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);

    // 检查当前渲染目标,如果是一个纹理就根据其标志检查是否需要备份内容
    if(SUCCEEDED(hval)) {
      SET_FLAG(m_dwFlags, F_HASDRAWCALL);
    }
    return hval;
  }


  GXHRESULT GXGraphicsImpl::CreateTexture(GTexture** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT MipLevels, 
    GXFormat Format, GXDWORD ResUsage)
  {
    GRESKETCH ResFeatDesc;
    GTextureFromUser *pGTex;
    *ppTexture = NULL;

    // 命名纹理查找
    if(szName != NULL)
    {
      GrapXInternal::ResourceSketch::GenerateTextureNameA(&ResFeatDesc, szName);
      GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
      if(pResource != NULL) {
        GXHRESULT hval = pResource->AddRef();
        *ppTexture = static_cast<GTexture*>(pResource);
        return hval;
      }
    }

    // 参数验证
    if( ! MarimoVerifier::Texture::CreateParam(
      "CreateTexture Error:", Width, Height, 1, MipLevels, Format, ResUsage))
    {
      return GX_FAIL;
    }

    pGTex = new GTextureFromUser(this, Width, Height, MipLevels, Format, ResUsage);
    if(pGTex != NULL)
    {
      pGTex->AddRef();

      if(szName != NULL) {
        pGTex->m_Name = szName;
      }

      //GRESCRIPTDESC Desc;
      //InlSetZeroT(Desc);
      //Desc.dwCmdCode = RC_ResetDevice;

      if(GXFAILED(pGTex->CreateRes(/*&Desc*/)))
      {
        pGTex->m_emType = GTextureImpl::CreationFailed;
        SAFE_RELEASE(pGTex);
        return GX_FAIL;
      }
      *ppTexture = pGTex;

      if(szName != NULL) // 注册命名纹理
      {
        ASSERT(ResFeatDesc.dwCategoryId == GXMAKEFOURCC('T','E','X','N'));
        RegisterResource(pGTex, &ResFeatDesc);
      }
      else {
        RegisterResource(pGTex, NULL);
      }
      return GX_OK;
    }
    return GX_FAIL;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureFromFileW(GTexture** ppTexture, GXLPCWSTR pSrcFile)
  {
    // 注意 MipLevels 使用 D3DX_FROM_FILE 标志, 根据文件储存来决定
    GXHRESULT lr = CreateTextureFromFileExW(ppTexture, pSrcFile, 
      D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, GXFMT_UNKNOWN, 
      GXRU_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL);
    if(GXSUCCEEDED(lr))
    {
      ((GTextureFromFile*)*ppTexture)->m_emType = GTextureImpl::File;
    }
    return lr;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureFromFileExW(
    GTexture**    ppTexture, 
    GXLPCWSTR     pSrcFile, 
    GXUINT        Width, 
    GXUINT        Height, 
    GXUINT        MipLevels, 
    GXFormat      Format, 
    GXDWORD       ResUsage, 
    GXDWORD       Filter /* = D3DX_FILTER_NONE */, 
    GXDWORD       MipFilter /* = D3DX_FILTER_NONE */, 
    GXCOLORREF    ColorKey /* = 0 */, 
    OUT LPGXIMAGEINFOX pSrcInfo)
  {
    //*ppTexture = NULL;
    GXHRESULT hval = GX_OK;

    // 合法性校验
    if(!MarimoVerifier::Texture::CreateFromFileParam("CreateTextureFromFileExW Error: ",
      Width, Height, 1, MipLevels, Format, ResUsage, Filter, MipFilter)) {
      return GX_FAIL;
    }

    GRESKETCH ResFeatDesc;
    GTextureFromFile *pTexture = NULL;
    BEGIN_SCOPED_LOCKER(m_pGraphicsLocker);

    clStringW strSrcFile;
    pSrcFile = IntToAbsPathW(strSrcFile, pSrcFile);

    // 收集资源特征
    GrapXInternal::ResourceSketch::GenerateTexture(&ResFeatDesc, pSrcFile, 
      Width, Height, 1, MipLevels, Format, MipFilter, Filter, MipFilter, ColorKey);

    // 资源查找
    GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
    if(pResource != NULL) {
      hval = pResource->AddRef();
      *ppTexture = static_cast<GTexture*>(pResource);
      return hval;
    }

    m_pLogger->OutputFormatW(L"Load texture from file: %s", pSrcFile);

    pTexture = new GTextureFromFile(pSrcFile, Width, Height, MipLevels, Format, ResUsage, Filter, MipFilter, ColorKey, this);
    if( ! InlCheckNewAndIncReference(pTexture))
    {
      m_pLogger->OutputFormatW(L"...Failed.\n");
      return GX_FAIL;
    }
    END_SCOPED_LOCKER;

    // D3DX 加载函数在安全锁之外, 避免加载函数Load时间太长导致渲染线程卡顿
    if(GXFAILED(pTexture->Create(pSrcInfo)))
    {
      pTexture->m_emType = GTextureImpl::CreationFailed;
      pTexture->Release();
      pTexture = NULL;
      m_pLogger->OutputFormatW(L"...Failed.\n");
      hval = GX_FAIL;
    }

    clstd::ScopedLocker sl(m_pGraphicsLocker);

    if(GXSUCCEEDED(hval))
    {
      m_ResMgr.Register(&ResFeatDesc, pTexture);
      m_pLogger->OutputFormatW(L"...Succeeded.\n");
    }
    *ppTexture = pTexture;
    return hval;
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3D(
    GTexture3D** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3DFromFileW(GTexture3D** ppTexture, GXLPCWSTR pSrcFile)
  {
    // 注意 MipLevels 使用 D3DX_FROM_FILE 标志, 根据文件储存来决定
    return CreateTexture3DFromFileExW(ppTexture, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
      D3DX_FROM_FILE, GXFMT_UNKNOWN, GXRU_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL);
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3DFromFileExW(GTexture3D** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT Depth, 
    GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo)
  {
    GXHRESULT hval = GX_OK;

    // 合法性校验
    if(!MarimoVerifier::Texture::CreateFromFileParam("CreateTextureFromFileExW Error: ",
      Width, Height, Depth, MipLevels, Format, ResUsage, Filter, MipFilter)) {
        return GX_FAIL;
    }

    GRESKETCH ResFeatDesc;
    GTexture3DFromFile *pTexture = NULL;
    BEGIN_SCOPED_LOCKER(m_pGraphicsLocker);

    // 收集资源特征
    GrapXInternal::ResourceSketch::GenerateTexture(&ResFeatDesc, pSrcFile, 
      Width, Height, Depth, MipLevels, Format, MipFilter, Filter, MipFilter, ColorKey);

    // 资源查找
    GResource* pResource = m_ResMgr.Find(&ResFeatDesc);
    if(pResource != NULL) {
      hval = pResource->AddRef();
      *ppTexture = static_cast<GTexture3D*>(pResource);
      return hval;
    }

    m_pLogger->OutputFormatW(L"Load texture3D from file: %s", pSrcFile);
    pTexture = new GTexture3DFromFile(pSrcFile, Width, Height, Depth, MipLevels, Format, ResUsage, Filter, MipFilter, ColorKey, this);
    if( ! InlCheckNewAndIncReference(pTexture))
    {
      m_pLogger->OutputFormatW(L"...Failed.\n");
      return GX_FAIL;
    }
    END_SCOPED_LOCKER;

    // D3DX 加载函数在安全锁之外, 避免加载函数Load时间太长导致渲染线程卡顿
    if(GXFAILED(pTexture->Create(pSrcInfo)))
    {
      //pTexture->m_emType = GTextureImpl::CreationFailed;
      pTexture->Release();
      pTexture = NULL;
      m_pLogger->OutputFormatW(L"...Failed.\n");
      hval = GX_FAIL;
    }

    clstd::ScopedLocker sl(m_pGraphicsLocker);

    if(GXSUCCEEDED(hval))
    {
      m_ResMgr.Register(&ResFeatDesc, pTexture);
      m_pLogger->OutputFormatW(L"...Succeeded.\n");
    }
    *ppTexture = pTexture;
    return hval;
  }

  //
  // Cube Texture
  //
  GXHRESULT GXGraphicsImpl::CreateTextureCube(GTextureCube** ppTexture, 
    GXLPCSTR szName, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureCubeFromFileW(GTextureCube** ppTexture, GXLPCWSTR pSrcFile)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureCubeFromFileExW(GTextureCube** ppTexture, 
    GXLPCWSTR pSrcFile, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, 
    GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo)
  {
    CLBREAK;
    return GX_OK;
  }


  //GXLRESULT GXGraphicsImpl::CreateOffscreenPlainSurface(
  //  GTexture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage)
  //{
  //  *ppTexture = NULL;
  //  GTextureOffscreenPlainSur *pGTex;
  //  pGTex = new GTextureOffscreenPlainSur(this);
  //  if(pGTex)
  //  {
  //    pGTex->AddRef();
  //    RegisterResource(pGTex);
  //    pGTex->m_emType     = GTextureImpl::OffscreenPlainSur;
  //    pGTex->m_nWidth     = Width;
  //    pGTex->m_nHeight    = Height;
  //    pGTex->m_nMipLevels = 1;
  //    pGTex->m_Format     = Format;
  //    pGTex->m_dwResUsage = ResUsage;

  //    GRESCRIPTDESC Desc;
  //    InlSetZeroT(Desc);
  //    Desc.dwCmdCode = RC_ResetDevice;

  //    if(pGTex->Invoke(&Desc) == FALSE)
  //    {
  //      SAFE_DELETE(pGTex);
  //      return GX_FAIL;
  //    }
  //    *ppTexture = pGTex;
  //    return GX_OK;
  //  }
  //  return GX_FAIL;
  //}

  GXHRESULT GXGraphicsImpl::CreateTextureFromD3DSurface(
    GTextureImpl** ppTexture, LPDIRECT3DSURFACE9 lpD3DSurface)
  {
    GTextureImpl_FromD3DSurface* pGTex;
    pGTex = new GTextureImpl_FromD3DSurface(this, lpD3DSurface);

    if(pGTex == NULL || pGTex->D3DSurface() == NULL)
    {
      SAFE_RELEASE(pGTex);
      return -1;
    }
    pGTex->AddRef();
    RegisterResource(pGTex, NULL);
    *ppTexture = pGTex;
    return GX_OK;
  }
  
  

//////////////////////////////////////////////////////////////////////////

  GXBOOL GXGraphicsImpl::D3DGetPresentParam(D3DPRESENT_PARAMETERS* pd3dpp)
  {
    *pd3dpp = m_d3dpp;
    return TRUE;
  }

  GXBOOL GXGraphicsImpl::GetDesc(GXGRAPHICSDEVICE_DESC* pDesc)
  {
    pDesc->cbSize             = sizeof(GXGRAPHICSDEVICE_DESC);
    pDesc->BackBufferWidth    = m_d3dpp.BackBufferWidth;
    pDesc->BackBufferHeight   = m_d3dpp.BackBufferHeight;
    pDesc->BackBufferCount    = m_d3dpp.BackBufferCount;
    pDesc->BackBufferFormat   = (GXFormat)m_d3dpp.BackBufferFormat;
    pDesc->DepthStencilFormat = (GXFormat)m_d3dpp.AutoDepthStencilFormat;
    pDesc->RefreshRateInHz    = m_d3dpp.FullScreen_RefreshRateInHz;
    pDesc->dwFlags            = NULL;
    return TRUE;
  }

  void ConvertTexResUsageToNative(IN GXDWORD ResUsage, OUT DWORD& D3DUsage, OUT D3DPOOL& Pool)
  {
    GXDWORD dwTexUsage = ResUsage & GXRU_TEX_MASK;
    if(dwTexUsage == GXRU_TEX_RENDERTARGET)
    {
      ASSERT(TEST_FLAG_NOT(ResUsage, GXRU_TEX_DEPTHSTENCIL));
      D3DUsage = D3DUSAGE_RENDERTARGET;
      Pool = D3DPOOL_DEFAULT;
    }
    else if(ResUsage == GXRU_TEX_DEPTHSTENCIL)
    {
      D3DUsage = D3DUSAGE_DEPTHSTENCIL;
      Pool = D3DPOOL_DEFAULT;
    }
    else if(ResUsage == GXRU_SYSTEMMEM)
    {
      D3DUsage = 0;
      Pool = D3DPOOL_SYSTEMMEM;
    }
    else if(TEST_FLAG(ResUsage, GXRU_FREQUENTLYREAD|GXRU_FREQUENTLYWRITE))
    {
      ASSERT(TEST_FLAG_NOT(ResUsage, GXRU_TEX_DEPTHSTENCIL));
      D3DUsage = D3DUSAGE_DYNAMIC;
      Pool = D3DPOOL_DEFAULT;
    }
    else if(ResUsage == 0)
    {
      D3DUsage = 0;
      Pool = D3DPOOL_DEFAULT;
    }
    else
      ASSERT(0);
  }

  void ConvertPrimResUsageToNative(IN GXDWORD ResUsage, OUT DWORD* Usage, OUT D3DPOOL* Pool)
  {
    *Usage = NULL;
    *Pool = D3DPOOL_DEFAULT;
    if(ResUsage & (GXRU_FREQUENTLYWRITE | GXRU_MIGHTBEWRITE))
    {
      *Usage |= D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    }

    if((ResUsage & (GXRU_FREQUENTLYREAD | GXRU_MIGHTBEREAD)) == 0)
    {
      *Usage |= D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    }
  }

  void ConvertNativeToTexResUsage(IN DWORD Usage, IN D3DPOOL Pool, OUT GXDWORD& ResUsage)
  {
    if(Usage & D3DUSAGE_RENDERTARGET && Pool == D3DPOOL_DEFAULT)
    {
      ResUsage = GXRU_TEX_RENDERTARGET;
    }
    else
      ASSERT(0);
  }

  void ConvertTextureFileSaveFormat (IN GXLPCSTR szFormat, OUT D3DXIMAGE_FILEFORMAT* d3diff)
  {
    *d3diff = D3DXIFF_DDS;
    if(IS_PTR(szFormat))
    {
      if(GXSTRCMPI(szFormat, "BMP") == 0)
        *d3diff = D3DXIFF_BMP;
      else if(GXSTRCMPI(szFormat, "JPG") == 0)
        *d3diff = D3DXIFF_JPG;
      else if(GXSTRCMPI(szFormat, "TGA") == 0)
        *d3diff = D3DXIFF_TGA;
      else if(GXSTRCMPI(szFormat, "PNG") == 0)
        *d3diff = D3DXIFF_PNG;
      else if(GXSTRCMPI(szFormat, "DDS") == 0)
        *d3diff = D3DXIFF_DDS;
      else if(GXSTRCMPI(szFormat, "PPM") == 0)
        *d3diff = D3DXIFF_PPM;
      else if(GXSTRCMPI(szFormat, "DIB") == 0)
        *d3diff = D3DXIFF_DIB;
      else if(GXSTRCMPI(szFormat, "HDR") == 0)
        *d3diff = D3DXIFF_HDR;
      else if(GXSTRCMPI(szFormat, "PFM") == 0)
        *d3diff = D3DXIFF_PFM;
    }
    else if((GXUINT_PTR)szFormat <= 8) {
      *d3diff = (D3DXIMAGE_FILEFORMAT)(GXUINT_PTR)szFormat;
    }
  }
} // D3D9
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

//////////////////////////////////////////////////////////////////////////
//
// namespace MarimoVerifier
// 
namespace MarimoVerifier
{
  namespace Texture
  {
    //static GXUINT c_nMaxTextureSize = 65536;  // 这个真不知道各个平台的限制, 先按一个最大可能来定义吧!
    GXBOOL CreateFromFileParam(
      GXLPCSTR  szPrefix,
      GXUINT    Width, 
      GXUINT    Height, 
      GXUINT    Depth, 
      GXUINT    MipLevels, 
      GXFormat  Format, 
      GXDWORD   ResUsage, 
      GXDWORD   Filter, 
      GXDWORD   MipFilter)
    {
      const GXFormatCategory eCate = Format == GXFMT_UNKNOWN 
        ? GXFMTCATE_OTHER : GetGraphicsFormatCategory(Format);
      if(eCate == GXFMTCATE_DEPTHSTENCIL) {
        CLOG_ERROR("%s can not create depth-stencil from file.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(Format != GXFMT_UNKNOWN && eCate != GXFMTCATE_COLOR && eCate != GXFMTCATE_COMPRESSEDCOLOR)
      {
        CLOG_ERROR("%s \"Format\" is out of range.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET))
      {
        CLOG_ERROR("%s Can't create as render target.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      return TRUE;
    }

    GXBOOL CreateParam(
      GXLPCSTR  szPrefix,
      GXUINT    Width, 
      GXUINT    Height, 
      GXUINT    Depth, 
      GXUINT    MipLevels, 
      GXFormat  Format, 
      GXDWORD   ResUsage)
    {
      //const LPCSTR szPrefix = "CreateTextureFromFile Error:";

      // 纹理尺寸是可以为负的, 这样会与屏幕保持一个比例
      if(Width == 0 || Height == 0)
      {
        CLOG_ERROR("%s Invalid texture size.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      if(Format == GXFMT_UNKNOWN)
      {
        CLOG_ERROR("%s Texture format must be specified.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }

      const GXFormatCategory eCate = GetGraphicsFormatCategory(Format);
      if(eCate == GXFMTCATE_DEPTHSTENCIL) {
        if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET)) {
          CLOG_ERROR("%s Can not specify \"GXRU_TEX_RENDERTARGET\" in depth-stencil texture.\n", szPrefix);
          ASSERT(0);
          return FALSE;
        }
      }

      if(Format != GXFMT_UNKNOWN && (eCate != GXFMTCATE_COLOR && eCate != GXFMTCATE_DEPTHSTENCIL))
      {
        CLOG_ERROR("%s \"Format\" is out of range.\n", szPrefix);
        ASSERT(0);
        return FALSE;
      }
      return TRUE;
    }
  } // namespace Texture
} // namespace MarimoVerifier
#endif // #ifdef ENABLE_GRAPHICS_API_DX9
