#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_CANVAS_D3D11_
#define _GXGRAPHICS_INLINE_RENDERTARGET_D3D11_
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_
#define _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_
#define _GXGRAPHICS_INLINE_SHADER_D3D11_

// ȫ��ͷ�ļ�
#include <GrapX.h>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GRegion.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXRenderTarget.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXCanvas3D.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/MOLogger.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

// ƽ̨���
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GPrimitiveImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderStubImpl_d3d11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_d3d11.h"

// ˽��ͷ�ļ�
#include <clPathFile.h>
#include "Platform/Win32_D3D11/GStateBlock_d3d11.h"
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "Canvas/GXEffectImpl.h"
//#include "Console.h"
#include <Smart/SmartStream.h>
#include <Smart/SmartProfile.h>

#include "GrapX/GXKernel.h"
#include "GrapX/GXUser.h"
#include <GDI/RegionFunc.h>
#include <GDI/GRegionImpl.h>
#include "Platform/Win32_D3D11/GTextureImpl_d3d11.h"

#include "Canvas/GXImageImpl.h"
#include "Platform/Win32_D3D11/GXCanvasImpl_d3d11.h"

#include "GrapX/gxError.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <Canvas/GFTFontImpl.h>
#include <GDI/GXShaderMgr.h>

#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_d3d11.h"
#include "Platform/Win32_D3D11/GXRenderTargetImpl_d3d11.h"
//#include <FreeImage.h>
#include "clImage.h"

// Canvas3D�õ�
#include "GrapX/StdMtl.h"
#include "Canvas/GXCanvas3DImpl.h"
#include "Platform/Win32_D3D11/GXCanvas3DImpl_d3d11.h"
#include "GrapX/GCamera.h"
#include "GrapX/GrapVR.h"  // Canvas3D �õ�
#include "Canvas/GXMaterialImpl.h"
// </Canvas3D�õ�>
#ifdef ENABLE_GRAPHICS_API_DX11
//////////////////////////////////////////////////////////////////////////
#define D3D11_GRAPHICS_IMPL
namespace GrapX
{
  namespace D3D11
  {
    extern const char* g_szBaseShader;
    typedef LPD3DINCLUDE LPD3DXINCLUDE;

#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/GXGraphicsImpl.inl"

    Graphics* GraphicsImpl::Create(const GRAPHICS_CREATION_DESC* pDesc)
    {
      GraphicsImpl* pGraphics = new GraphicsImpl;
      if(!InlCheckNewAndIncReference(pGraphics)) {
        return NULL;
      }

      if(!pGraphics->Initialize(pDesc)) {
        pGraphics->Release();
        return NULL;
      }

      return (Graphics*)pGraphics;
    }

    GraphicsImpl::GraphicsImpl()
      : s_uCanvasCacheCount     (4)
      , m_hWnd                  (NULL)
      , m_driverType            (D3D_DRIVER_TYPE_NULL)
      , m_featureLevel          (D3D_FEATURE_LEVEL_11_0)
      , m_pd3dDevice            (NULL)
      , m_pImmediateContext     (NULL)
      , m_pSwapChain            (NULL)
      , m_dwFlags               (NULL)
      , m_pIdentity             (GXPLATFORM_UNKNOWN)
      , m_pBasicShader          (NULL)
      , m_pBasicEffect          (NULL)
      , m_pCurRenderTargetView  (NULL)
      , m_pCurDepthStencilView  (NULL)
      , m_pVertexLayout         (NULL)
      , m_eCurTopology          (D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
      , m_pCurPrimitive         (NULL)
      , m_pCurRenderTarget      (NULL)
      , m_pDefaultSamplerState  (NULL)
      , m_aCanvasPtrCache       (NULL)
      , m_pCurVertexDecl        (NULL)
      , m_pCurCanvasCore        (NULL)
      , m_pCurRasterizerState   (NULL)
      , m_pDefaultBlendState    (NULL)
      , m_pDefaultDepthStencilState(NULL)
      , m_pDefaultRasterizerState(NULL)
      , m_pCurBlendState        (NULL)
      , m_pCurDepthStencilState (NULL)
      , m_pCurSamplerState      (NULL)
      , m_pCurShader            (NULL)
      , m_pBackBufferRenderTarget  (NULL)
      , m_pTempBuffer           (NULL)
      , m_dwBackBufferStencil   (1)
      , m_pGraphicsLocker       (NULL)
      , m_nGraphicsCount        (0)
      , m_dwThreadId            (0)
      , m_pLogger               (NULL)
      , m_pShaderConstName      (NULL)
      , m_pWhiteTexture8x8      (NULL)
    {
      memset(m_pCurTexture, 0, sizeof(Texture*) * MAX_TEXTURE_STAGE);
    }

    HRESULT GraphicsImpl::Initialize(const GRAPHICS_CREATION_DESC* pDesc)
    {
      m_strResourceDir = pDesc->szRootDir;
      m_hWnd = pDesc->hWnd;
      m_dwFlags |= F_CREATEDEVICE;

      if(pDesc->pLogger) {
        m_pLogger = pDesc->pLogger;
        m_pLogger->AddRef();
      }
      else {
        clStringW strFilePath;
        clpathfile::CombinePath(strFilePath, pDesc->szRootDir, _CLTEXT("grapx.log"));
        MOCreateFileLoggerW(&m_pLogger, strFilePath, FALSE);
      }

      m_pBackBufferRenderTarget = new RenderTargetImpl_BackBuffer(this);
      if(InlIsFailedToNewObject(m_pBackBufferRenderTarget)) {
        return E_OUTOFMEMORY;
      }

      // ������־
      UINT createDeviceFlags = 0;
      if(TEST_FLAG(pDesc->dwCreateFlags, GRAPHICS_CREATION_FLAG_DEBUG))
      {
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
      }

      static D3D_DRIVER_TYPE driverTypes[] =
      {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
      };
      UINT numDriverTypes = CLARRAYSIZE(driverTypes);

      static D3D_FEATURE_LEVEL featureLevels[] =
      {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
      };
      UINT numFeatureLevels = CLARRAYSIZE(featureLevels);

      RECT rcWndClient;
      GetClientRect(pDesc->hWnd, &rcWndClient);
      ASSERT(rcWndClient.left == 0 && rcWndClient.top == 0);

      DXGI_SWAP_CHAIN_DESC& sd = m_SwapChainDesc;
      InlSetZeroT(sd);
      sd.BufferCount = 1;
      sd.BufferDesc.Width = rcWndClient.right;
      sd.BufferDesc.Height = rcWndClient.bottom;
      sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      sd.BufferDesc.RefreshRate.Numerator = 60;
      sd.BufferDesc.RefreshRate.Denominator = 1;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.OutputWindow = m_hWnd;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.Windowed = TRUE;

      IntEnumAdapter();

      HRESULT hval = E_FAIL;

      for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
      {
        m_driverType = driverTypes[driverTypeIndex];
        hval = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
          D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
        if(SUCCEEDED(hval)) { break; }
      }
      if(FAILED(hval)) {
        return hval;
      }

      m_pBackBufferRenderTarget->InitializeWithSwapChain(m_pSwapChain);
      InlSetRenderTarget(m_pBackBufferRenderTarget, 0);

      Clear(NULL, 0, GXCLEAR_DEPTH | GXCLEAR_STENCIL, 0, 1.0f, 0);

      // Setup the viewport
      D3D11_VIEWPORT vp;
      vp.Width = (FLOAT)sd.BufferDesc.Width;
      vp.Height = (FLOAT)sd.BufferDesc.Height;
      vp.MinDepth = 0.0f;
      vp.MaxDepth = 1.0f;
      vp.TopLeftX = 0;
      vp.TopLeftY = 0;
      m_pImmediateContext->RSSetViewports(1, &vp);

      m_pIdentity = GXPLATFORM_WIN32_DIRECT3D11;
      //m_pGraphicsLocker = new clstd::Locker;
      InitCommon();


      GXSHADER_SOURCE_DESC aDesc[2];
      InlSetZeroT(aDesc);

      aDesc[0].szSourceData = g_szBaseShader;
      aDesc[0].nSourceLen = 0;
      aDesc[0].szEntry = "vs_main";
      aDesc[0].szTarget = "vs_4_0";
      aDesc[1].szSourceData = g_szBaseShader;
      aDesc[1].nSourceLen = 0;
      aDesc[1].szEntry = "ps_main";
      aDesc[1].szTarget = "ps_4_0";

      CreateShaderFromSource(&m_pBasicShader, aDesc, 2);
      CreateEffect(&m_pBasicEffect, m_pBasicShader);

      return TRUE;
    }

    void GraphicsImpl::IntGetDimension(GXUINT& nWidth, GXUINT& nHeight)
    {
      if(m_pCurRenderTarget) {
        GXSIZE sDimension;
        m_pCurRenderTarget->GetDimension(&sDimension);
        nWidth = sDimension.cx;
        nHeight = sDimension.cy;
      }
      else {
        nWidth = m_SwapChainDesc.BufferDesc.Width;
        nHeight = m_SwapChainDesc.BufferDesc.Height;
      }
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT GraphicsImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT GraphicsImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);

      if(nRefCount > 0) {
        return nRefCount;
      }

      SAFE_RELEASE(m_pBasicEffect);
      SAFE_RELEASE(m_pBasicShader);

      ReleaseCommon();

      SAFE_RELEASE(m_pBackBufferRenderTarget);
      SAFE_RELEASE(m_pTempBuffer);

      SAFE_RELEASE(m_pCurShader);
      SAFE_RELEASE(m_pCurPrimitive);
      for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
        SAFE_RELEASE(m_pCurTexture[i]);
      }

      SAFE_RELEASE(m_pCurRenderTarget);
      SAFE_RELEASE(m_pCurCanvasCore);

      SAFE_RELEASE(m_pCurVertexDecl);
      INVOKE_LOST_DEVICE;

      SAFE_RELEASE(m_pVertexLayout);
      SAFE_RELEASE(m_pCurDepthStencilView);
      SAFE_RELEASE(m_pCurRenderTargetView);
      SAFE_RELEASE(m_pImmediateContext);
      SAFE_RELEASE(m_pSwapChain);

      if(m_dwFlags & F_CREATEDEVICE)
      {

//#if defined(DEBUG) || defined(_DEBUG)
//        ID3D11Debug *pD3D11Debug;
//        HRESULT hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pD3D11Debug));
//        if(SUCCEEDED(hr)) {
//          hr = pD3D11Debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
//        }
//        SAFE_RELEASE(pD3D11Debug);
//#endif
        SAFE_RELEASE(m_pd3dDevice);
        m_hWnd = NULL;
      }

      SAFE_RELEASE(m_pLogger);

      SAFE_DELETE(m_pGraphicsLocker);
      delete this;
      return GX_OK;
    }

    GXHRESULT GraphicsImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      INVOKE_DESC_CHECK(pDesc);
      switch(pDesc->dwCmdCode)
      {
      case RC_LostDevice:
      {
        InlSetCanvas(NULL);
        InlSetShader(NULL);
        InlSetVertexDecl(NULL);
        SetPrimitive(NULL, 0);
        for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
          InlSetTexture(NULL, i);
        }

        //SAFE_RELEASE(m_pDeviceOriginTex);
      }
      break;
      case RC_ResetDevice:
      {
        //m_pCurRenderState->Update(NULL);
        m_pCurSamplerState->Activate(0, NULL);
      }
      break;
      case RC_ResizeDevice:
      {

      }
      break;
      }
      return FALSE;
    }
    GXPlatformIdentity GraphicsImpl::GetPlatformID() const
    {
      ASSERT(m_pIdentity == GXPLATFORM_WIN32_DIRECT3D11);
      return m_pIdentity;
    }

    GXBOOL GraphicsImpl::Activate(GXBOOL bActive)
    {
      if(bActive != FALSE) {
        SET_FLAG(m_dwFlags, F_ACTIVATE);
      }
      else {
        RESET_FLAG(m_dwFlags, F_ACTIVATE);

        // TODO: ��������Ϊʲô����Ҫ�ָ�ԭ����BackBuffer
        //if(m_pCurRenderTarget != m_pDeviceOriginTex) {
        //  InlSetCanvas(NULL);
        //}
        // ���˾���ע����

#if defined(_DEBUG) && 0
        LPDIRECT3DSURFACE9 lpRenderTar;
        m_pd3dDevice->GetRenderTarget(0, &lpRenderTar);
        ASSERT(lpRenderTar == m_pCurRenderTarget->D3DSurface());
        lpRenderTar->Release();
#endif

        // 2012-08-04 û�б�Ҫ�ָ���Ⱦ�����!
        //GXREGN regn = {0,0,m_d3dpp.BackBufferWidth,m_d3dpp.BackBufferHeight};
        //SetSafeClip(&regn);
      }
      return TRUE;
    }

    GXHRESULT GraphicsImpl::Test()
    {
      return 0;
    }

    GXHRESULT GraphicsImpl::Begin()
    {
      m_pGraphicsLocker->Lock();
      if((++m_nGraphicsCount) == 1)
      {
        if(GXSUCCEEDED(Test()) /*&&
          /*GXSUCCEEDED(m_pImmediateContext->Begin())*/)
        {
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

    GXHRESULT GraphicsImpl::End()
    {
      GXINT nCount = --m_nGraphicsCount;
      if(nCount == 0)
      {
        if(TEST_FLAG(m_dwFlags, F_SHOWCONSOLE))
        {
          Canvas* pCanvas = LockCanvas(NULL, NULL, NULL);
          //m_pConsole->Render(pCanvas);
          SAFE_RELEASE(pCanvas);
        }

        ASSERT(m_dwThreadId == GetCurrentThreadId());
        m_dwThreadId = NULL;
        Activate(FALSE);
        //HRESULT hval = m_pd3dDevice->EndScene();
        //ASSERT(SUCCEEDED(hval));
      }
      m_pGraphicsLocker->Unlock();
      return nCount;
    }
    GXHRESULT GraphicsImpl::Present()
    {
      m_pGraphicsLocker->Lock();  // TODO: ������ô��ʡ���Lock
      HRESULT hval = m_pSwapChain->Present(0, 0);
#ifdef _DEBUG
      // ���߳�ʱ�ı��豸��С���ܻᵼ��ʧ��
      if(GXFAILED(hval)) {
        TRACE("Warning: Present failed(Code: %d).\n", hval);
      }
#endif // _DEBUG
      m_pGraphicsLocker->Unlock();
      return hval;
    }

    GXHRESULT GraphicsImpl::Resize(int nWidth, int nHeight)
    {
      ASSERT(0);
      //OnDeviceEvent(DE_LostDevice);
      //CallResourceEvent(DE_LostDevice);

      //m_d3dpp.BackBufferWidth = nWidth;
      //m_d3dpp.BackBufferHeight = nHeight;
      //m_pd3dDevice->Reset(&m_d3dpp);

      //OnDeviceEvent(DE_ResetDevice);
      //CallResourceEvent(DE_ResetDevice);

      //Activate(FALSE);
      return GX_OK;
    }

    GXHRESULT GraphicsImpl::SetPrimitive(Primitive* pPrimitive, GXUINT uStreamSource)
    {
      if(m_pCurPrimitive == pPrimitive)
        return S_OK;

      SAFE_RELEASE(m_pCurPrimitive);
      m_pCurPrimitive = pPrimitive;

      if(m_pCurPrimitive == NULL)
        return S_OK;

      m_pCurPrimitive->AddRef();

      // Ӧ�ö�������
      GPrimitiveVertexIndexImpl* pPrimImpl = static_cast<GPrimitiveVertexIndexImpl*>(pPrimitive);
      if(pPrimImpl->m_pVertexDecl != NULL) {
        InlSetVertexDecl(pPrimImpl->m_pVertexDecl);
      }


      GPrimitiveVertexIndexImpl* pPrimitiveImpl = (GPrimitiveVertexIndexImpl*)pPrimitive;
      UINT offset = 0;
      m_pImmediateContext->IASetVertexBuffers(0, 1, &pPrimitiveImpl->m_pD3D11VertexBuffer,
        &pPrimitiveImpl->m_uVertexStride, &offset);

      if(pPrimitiveImpl->GetIndexCount()) {
        m_pImmediateContext->IASetIndexBuffer(pPrimitiveImpl->m_pD3D11IndexBuffer, pPrimitiveImpl->m_D3DIndexFormat, offset);
      }

      return S_OK;
    }

    GXBOOL GraphicsImpl::SetTexture(TextureBase* pTexture, GXUINT uStage)
    {
      return (InlSetTexture(reinterpret_cast<TexBaseImpl*>(pTexture), uStage) != StateResult::Failed);
    }

    GXBOOL GraphicsImpl::SetSafeClip(const GXREGN* pRegn)
    {
#ifdef _43A2DE06_6673_4ddd_9C1C_881493B776A0_
      D3DVIEWPORT9 Viewport;
      Viewport.X = (GXDWORD)pRegn->left;
      Viewport.Y = (GXDWORD)pRegn->top;
      Viewport.Width = (GXDWORD)pRegn->width;
      Viewport.Height = (GXDWORD)pRegn->height;
      Viewport.MinZ = 0.0f;
      Viewport.MaxZ = 1.0f;

      GXUINT nWidth, nHeight;
      m_pCurRenderTarget->GetDimension(&nWidth, &nHeight);

      //m_matWorld._11 = ( 2.0f / (float)nWidth)  * ((float)nWidth  / (float)Viewport.Width);
      //m_matWorld._22 = (-2.0f / (float)nHeight) * ((float)nHeight / (float)Viewport.Height);
      m_matWorld._11 = (2.0f / (float)Viewport.Width);
      m_matWorld._22 = (-2.0f / (float)Viewport.Height);
      m_matWorld._41 = -1.0f - ((float)pRegn->left * 2.0f / (float)Viewport.Width);
      m_matWorld._42 = 1.0f + ((float)pRegn->top  * 2.0f / (float)Viewport.Height);

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
      if(pRegn)
      {
        gxRegnToRect(&rect, (const GXLPREGN)pRegn);
      }
      else
      {
        GXSIZE sDimension;
        m_pCurRenderTarget->GetDimension(&sDimension);
        rect.set(0, 0, sDimension.cx, sDimension.cy);
      }
      m_pImmediateContext->RSSetScissorRects(1, (RECT*)&rect);

      //HRESULT hval = m_pd3dDevice->SetScissorRect((RECT*)&rect);
  //#ifdef _DEBUG
  //    if(GXFAILED(hval))
  //    {
  //      TRACE(__FUNCTION__" failed:%d", hval);
  //    }
  //#endif // _DEBUG
  //    return GXSUCCEEDED(hval);
      return TRUE;
#endif
    }

    GXBOOL GraphicsImpl::SetViewport(const GXVIEWPORT* pViewport)
    {
      D3D11_VIEWPORT Viewport;
      if(pViewport == NULL)
      {
        GXUINT width, height;
        IntGetDimension((GXUINT&)width, (GXUINT&)height);
        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width = static_cast<FLOAT>(width);
        Viewport.Height = static_cast<FLOAT>(height);
        Viewport.MinDepth = 0.f;
        Viewport.MaxDepth = 1.f;
      }
      else
      {
        Viewport.TopLeftX = (FLOAT)pViewport->regn.left;
        Viewport.TopLeftY = (FLOAT)pViewport->regn.top;
        Viewport.Width = (FLOAT)pViewport->regn.width;
        Viewport.Height = (FLOAT)pViewport->regn.height;
        Viewport.MinDepth = pViewport->fNear;
        Viewport.MaxDepth = pViewport->fFar;
      }

      HRESULT hval = GX_OK;
      m_pImmediateContext->RSSetViewports(1, &Viewport);
      //#ifdef _DEBUG
      //    if(GXFAILED(hval))
      //    {
      //      TRACE(__FUNCTION__" failed:%d", hval);
      //    }
      //#endif // _DEBUG
      return GXSUCCEEDED(hval);
    }
    //GXBOOL GXGraphicsImpl::Clear(const GXRECT*lpRects, GXUINT nCount, D3DCOLOR crClear)
    //{
    //  return GXSUCCEEDED(m_pd3dDevice->Clear(nCount, (const D3DRECT*)lpRects, D3DCLEAR_TARGET, crClear, 1.0f, NULL));
    //}

#if 0
    GXBOOL GXGraphicsImpl::IntCheckSizeOfTargetAndDepth()
    {
      if(m_pCurDepthStencil == 0 && m_pCurRenderTarget == 0) {
        return TRUE;
      }
      GXINT nDepthSurWidth;
      GXINT nDepthSurHeight;
      GXINT nColorSurWidth;
      GXINT nColorSurHeight;

      if(m_pCurDepthStencil != NULL) {
        nDepthSurWidth = m_pCurDepthStencil->m_nWidth;
        nDepthSurHeight = m_pCurDepthStencil->m_nHeight;
      }
      else {
        nDepthSurWidth = m_SwapChainDesc.BufferDesc.Width;//m_d3dpp.BackBufferWidth;
        nDepthSurHeight = m_SwapChainDesc.BufferDesc.Height;
      }

      if(m_pCurRenderTarget != NULL) {
        nColorSurWidth = m_pCurRenderTarget->m_nWidth;
        nColorSurHeight = m_pCurRenderTarget->m_nHeight;
      }
      else {
        nColorSurWidth = m_SwapChainDesc.BufferDesc.Width;//m_d3dpp.BackBufferWidth;
        nColorSurHeight = m_SwapChainDesc.BufferDesc.Height;//m_d3dpp.BackBufferHeight;
      }

      if(nDepthSurWidth >= nColorSurWidth &&
        nDepthSurHeight >= nColorSurHeight)
      {
        return TRUE;
      }
      TRACE(" ! Depth and stencil buffer size is less than back buffer\n");
      return FALSE;
    }
#endif

    GXHRESULT GraphicsImpl::Clear(const GXRECT* lpRects, GXUINT nCount, GXDWORD dwFlags, const GXColor& crClear, float z, GXDWORD dwStencil)
    {
      if(lpRects == NULL || nCount == 0)
      {
        if(TEST_FLAG(dwFlags, GXCLEAR_TARGET)) {
          //GXColor color = crClear;
          m_pImmediateContext->ClearRenderTargetView(m_pCurRenderTargetView, (const FLOAT*)&crClear);
        }

#if 0
        // TODO: ���ԵĴ���
        m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        // /���ԵĴ���
#endif

    // �������� "GXCLEAR_*" �� "D3D11_CLEAR_*" �Ķ�����ֵ��Ҫ��, �������ı�
        UINT uClearFlags = (dwFlags & (GXCLEAR_DEPTH | GXCLEAR_STENCIL)) >> 1;

        if(uClearFlags != 0 && m_pCurDepthStencilView) {
          m_pImmediateContext->ClearDepthStencilView(m_pCurDepthStencilView, uClearFlags, z, (UINT8)dwStencil);
        }
      }
      else
      {
        DepthStencilState* pDepthStencil = NULL;
        GXDEPTHSTENCILDESC desc(TRUE, TRUE);
        desc.DepthFunc = GXCMP_ALWAYS;
        desc.FrontFace.StencilDepthFailOp = GXSTENCILOP_KEEP;
        desc.FrontFace.StencilFailOp = GXSTENCILOP_KEEP;
        desc.FrontFace.StencilFunc = GXCMP_ALWAYS;
        desc.FrontFace.StencilPassOp = GXSTENCILOP_REPLACE;

        desc.BackFace.StencilDepthFailOp = GXSTENCILOP_KEEP;
        desc.BackFace.StencilFailOp = GXSTENCILOP_KEEP;
        desc.BackFace.StencilFunc = GXCMP_ALWAYS;
        desc.BackFace.StencilPassOp = GXSTENCILOP_REPLACE;
        CreateDepthStencilState(&pDepthStencil, &desc);


        DepthStencilStateImpl* pSavedDepthStencilState = m_pCurDepthStencilState;
        pSavedDepthStencilState->AddRef();

        InlSetDepthStencilState(static_cast<DepthStencilStateImpl*>(pDepthStencil));
        InlSetShader(m_pBasicShader);
        pDepthStencil->SetStencilRef(dwStencil);

        //GXRASTERIZERDESC ras_desc;
        //CreateRasterizerState()

        Primitive* pPrimitive = NULL;
        clstd::LocalBuffer<sizeof(CANVAS_PRMI_VERT) * 6 * 64> buf;
        buf.Resize(sizeof(CANVAS_PRMI_VERT) * 6 * nCount, FALSE);
        CANVAS_PRMI_VERT* pVert = reinterpret_cast<CANVAS_PRMI_VERT*>(buf.GetPtr());
        GXCOLOR color = crClear.ARGB();
        for(GXUINT i = 0; i < nCount; i++)
        {
          pVert[0].pos.set((float)lpRects[i].left, (float)lpRects[i].top, z, 1);
          pVert[0].texcoord.set(0, 0);
          pVert[0].color = color;

          pVert[1].pos.set((float)lpRects[i].right, (float)lpRects[i].top, z, 1);
          pVert[1].texcoord.set(0, 0);
          pVert[1].color = color;

          pVert[2].pos.set((float)lpRects[i].left, (float)lpRects[i].bottom, z, 1);
          pVert[2].texcoord.set(0, 0);
          pVert[2].color = color;

          pVert[3].pos.set((float)lpRects[i].left, (float)lpRects[i].bottom, z, 1);
          pVert[3].texcoord.set(0, 0);
          pVert[3].color = color;

          pVert[4].pos.set((float)lpRects[i].right, (float)lpRects[i].top, z, 1);
          pVert[4].texcoord.set(0, 0);
          pVert[4].color = color;

          pVert[5].pos.set((float)lpRects[i].right, (float)lpRects[i].bottom, z, 1);
          pVert[5].texcoord.set(0, 0);
          pVert[5].color = color;

          pVert += 6;
        }


        //CreatePrimitiveV(&pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P4T2F_C1D), GXRU_DEFAULT, 6 * nCount, sizeof(CANVAS_PRMI_VERT), buf.GetPtr());
        CreatePrimitive(&pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P4T2F_C1D), GXResUsage::Default, 6 * nCount, sizeof(CANVAS_PRMI_VERT), buf.GetPtr(), 0, 0, NULL);
        Primitive* pSavedPrimitive = m_pCurPrimitive;
        if(pSavedPrimitive) {
          pSavedPrimitive->AddRef();
        }

        SetTexture(m_pWhiteTexture8x8, 0);


        SetPrimitive(pPrimitive);
        GXDWORD dwFlags = m_dwFlags;
        m_dwFlags = F_ACTIVATE;
        DrawPrimitive(GXPT_TRIANGLELIST, 0, nCount * 2);
        m_dwFlags = dwFlags;

        if(pSavedPrimitive)
        {
          SetPrimitive(static_cast<Primitive*>(pSavedPrimitive));
          SAFE_RELEASE(pSavedPrimitive);
        }
        else
        {
          SetPrimitive(NULL);
        }

        InlSetDepthStencilState(pSavedDepthStencilState);
        SAFE_RELEASE(pSavedDepthStencilState);

        //SAFE_RELEASE(m_pCurPrimitive);
        //m_pCurPrimitive = pSavedPrimitive;
        //SAFE_RELEASE(pSavedPrimitive);

        SAFE_RELEASE(pPrimitive);
        SAFE_RELEASE(pDepthStencil);
      }
      return 0;

      STATIC_ASSERT((GXCLEAR_DEPTH >> 1) == D3D11_CLEAR_DEPTH);
      STATIC_ASSERT((GXCLEAR_STENCIL >> 1) == D3D11_CLEAR_STENCIL);
    }

    GXVOID GraphicsImpl::BuildInputLayout()
    {
      // ����shader�Ͷ��������������m_pVertexLayout
      ASSERT(m_pVertexLayout == NULL);
      ShaderImpl* pShaderImpl = static_cast<ShaderImpl*>(m_pCurShader);

      ID3D11InputLayout* pInputLayout = pShaderImpl->D3D11GetInputLayout(m_pCurVertexDecl);
      m_pVertexLayout = pInputLayout;
      m_pVertexLayout->AddRef();
      m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
    }

    void GraphicsImpl::IntEnumAdapter()
    {
      IDXGIFactory* pFactory = NULL;
      IDXGIAdapter * pAdapter;

      // Create a DXGIFactory object.
      if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
      {
        return ;
      }
      
      DXGI_ADAPTER_DESC desc;
      CLOG("Adapter list");
      for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
      {
        pAdapter->GetDesc(&desc);
        CLOGW(_CLTEXT("%s.VendorId:%u.DeviceId:%u.SubSysId:%u.Revision:%u"), desc.Description, desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision);
        SAFE_RELEASE(pAdapter);
      }


      SAFE_RELEASE(pFactory);
    }

    GXHRESULT GraphicsImpl::DrawPrimitive(const GXPrimitiveType eType, const GXUINT StartVertex,
      const GXUINT PrimitiveCount)
    {
      ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));
      //#ifdef _DEBUG
      //    IntCheckSizeOfTargetAndDepth();
      //#endif // IntCheckSizeOfTargetAndDepth

      GXUINT nVertCount = 0;
      InlUpdateTopology(eType, PrimitiveCount, &nVertCount);

      if(m_pVertexLayout == NULL) {
        BuildInputLayout();
      }

      ShaderImpl* pShaderImpl = static_cast<ShaderImpl*>(m_pCurShader);
      pShaderImpl->CheckUpdateConstBuf();
      //m_pCurRenderState->IntCheckUpdate();

      // NOTE: Ϊë����Ҫ�ύ����״̬��?
      //m_pImmediateContext->PSSetSamplers(0, SAMPLERCOUNT, m_pCurSamplerState->GetSamplers());

      m_pImmediateContext->Draw(nVertCount, StartVertex); // FIXME: ��һ����������
      return 0;
    }

    GXHRESULT GraphicsImpl::DrawPrimitive(
      const GXPrimitiveType eType, const GXINT BaseVertexIndex,
      const GXUINT MinIndex, const GXUINT NumVertices,
      const GXUINT StartIndex, const GXUINT PrimitiveCount)
    {
#ifdef D3D11_LOW_DEBUG
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

#endif // D3D11_LOW_DEBUG
      //#ifdef _DEBUG
      //    IntCheckSizeOfTargetAndDepth();
      //#endif // IntCheckSizeOfTargetAndDepth

      UINT nIndexCount = 0;
      InlUpdateTopology(eType, PrimitiveCount, &nIndexCount);

      if(m_pVertexLayout == NULL) {
        BuildInputLayout();
      }
      else {
        m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
      }

      ShaderImpl* pShaderImpl = static_cast<ShaderImpl*>(m_pCurShader);
      pShaderImpl->CheckUpdateConstBuf();
      //m_pCurRenderState->IntCheckUpdate();

      // NOTE: Ϊë����Ҫ�ύ����״̬��?
      //m_pImmediateContext->PSSetSamplers(0, SAMPLERCOUNT, m_pCurSamplerState->GetSamplers());

      ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));

      m_pImmediateContext->DrawIndexed(nIndexCount, StartIndex, BaseVertexIndex); // FIXME: ��һ����������
      return 0;
    }

    GXHRESULT GraphicsImpl::CreateRenderTarget(
      RenderTarget** ppRenderTarget, GXLPCWSTR szName, GXINT nWidth, GXINT nHeight,
      GXFormat eColorFormat, GXFormat eDepthStencilFormat)
    {
      GRESKETCH rs = { RESTYPE_RENDERTARGET };

      GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppRenderTarget), &rs);
      if(GXSUCCEEDED(hr)) {
        return hr;
      }

      RenderTargetImpl* pTarget = new RenderTargetImpl(this, nWidth, nHeight);
      if(InlIsFailedToNewObject(pTarget)) {
        return GX_ERROR_OUROFMEMORY;
      }

      if(_CL_NOT_(pTarget->Initialize(eColorFormat, eDepthStencilFormat)))
      {
        SAFE_RELEASE(pTarget);
        return GX_FAIL;
      }

      RegisterResource(pTarget, &rs);
      *ppRenderTarget = pTarget;
      return GX_OK;
    }

    GXHRESULT GraphicsImpl::CreateTexture(Texture** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height,
      GXFormat Format, GXResUsage eResUsage, GXUINT MipLevels, GXLPCVOID pInitData, GXUINT nPitch)
    {
      GRESKETCH rs = { RCC_Texture2D };
      GXHRESULT hr = GX_FAIL;

      // ������������Դ
      if(szName) {
        rs.strResourceName = szName;
        hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
        if(GXSUCCEEDED(hr)) {
          return hr;
        }
      }


      *ppTexture = NULL;
      TextureImpl* pTexture = new TextureImpl(this, Format, Width, Height, MipLevels, eResUsage);

      if(InlIsFailedToNewObject(pTexture)) {
        return GX_FAIL;
      }

      if(_CL_NOT_(pTexture->InitTexture(FALSE, pInitData, nPitch)))
      {
        pTexture->Release();
        pTexture = NULL;
        return GX_FAIL;
      }

      RegisterResource(pTexture, szName ? &rs : NULL);
      *ppTexture = pTexture;
      return GX_OK;
    }

    GXHRESULT GraphicsImpl::CreateTexture(Texture** ppTexture, GXLPCSTR szName, GXResUsage eUsage, Texture* pSourceTexture)
    {
      CLBREAK;
      return GX_FAIL;
    }


    GXHRESULT GraphicsImpl::CreateTextureFromMemory(Texture** ppTexture, GXLPCWSTR szName, clstd::Buffer* pBuffer, GXUINT MipLevels, GXResUsage eUsage)
    {
      GRESKETCH rs = { RCC_Texture2D };
      GXHRESULT hr = GX_FAIL;

      // ������������Դ
      if(szName) {
        rs.strResourceName = szName;
        hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
        if(GXSUCCEEDED(hr)) {
          return hr;
        }
      }

      clstd::MemBuffer buffer;
      clstd::IMAGEDESC sImgDesc;
      DECODE_TEXTURE_DESC desc = {&sImgDesc, &buffer};
      GXFormat format = Texture::DecodeToMemory(&desc, pBuffer->GetPtr(), (GXUINT)pBuffer->GetSize(), TRUE);

      if (format == Format_BC2 || format == Format_BC3)
      {
        hr = CreateTexture(ppTexture, NULL, sImgDesc.width, sImgDesc.height,
          format, eUsage, desc.nMipLevels, sImgDesc.ptr, sImgDesc.pitch);
      }
      else
      {
        hr = CreateTexture(ppTexture, NULL, sImgDesc.width, sImgDesc.height,
          format, eUsage, MipLevels, sImgDesc.ptr, sImgDesc.pitch);
      }

      // �����ֵ�Ҫע��һ��
      if(GXSUCCEEDED(hr) && szName)
      {
        m_ResMgr.Unregister(*ppTexture); // TODO: ��ʱ��ôд�ɣ������ĺ��Ĺ��ܻ��ǵ��ᵽIntCreate��ȥ
        m_ResMgr.Register(&rs, *ppTexture);
      }

      return hr;
    }

    GXHRESULT GraphicsImpl::CreateTextureFromFile(Texture** ppTexture, GXLPCWSTR szFilePath, GXUINT MipLevels, GXResUsage eUsage)
    {
      GRESKETCH rs = { RCC_Texture2D };
      clpathfile::CombinePath(rs.strResourceName, m_strResourceDir, szFilePath);

      // ������������Դ
      GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
      if(GXSUCCEEDED(hr)) {
        return hr;
      }

      clstd::File file;
      if(file.OpenExisting(rs.strResourceName))
      {
        clstd::MemBuffer buffer;
        if(file.Read(&buffer))
        {
          hr = CreateTextureFromMemory(ppTexture, NULL, &buffer, MipLevels, eUsage);
          if(GXSUCCEEDED(hr)) {
            m_ResMgr.Unregister(*ppTexture); // TODO: ��ʱ��ôд�ɣ������ĺ��Ĺ��ܻ��ǵ��ᵽIntCreate��ȥ
            m_ResMgr.Register(&rs, *ppTexture);
          }
          return hr;
        }
      }
      return GX_E_OPEN_FAILED;
    }


    GXHRESULT GraphicsImpl::CreateTexture3D(
      Texture3D** ppTexture, GXLPCSTR szName,
      GXUINT Width, GXUINT Height, GXUINT Depth,
      GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
    {
      CLBREAK;
      return GX_OK;
    }

    GXHRESULT GraphicsImpl::CreateTexture3DFromFile(
      Texture3D** ppTexture, GXLPCWSTR pSrcFile)
    {
      //return CreateTexture3DFromFileExW(ppTexture, pSrcFile, GX_DEFAULT, GX_DEFAULT, GX_DEFAULT,
      //  GX_FROM_FILE,  GXFMT_UNKNOWN, GXRU_DEFAULT, GX_DEFAULT, GX_DEFAULT, 0, NULL);
      CLBREAK;
      return GX_OK;
    }

    GXBOOL GraphicsImpl::CreateTextureCube(TextureCube** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height,
      GXFormat Format, GXResUsage eResUsage, GXUINT MipLevels, GXLPCVOID pInitData, GXUINT nPitch)
    {
      GRESKETCH rs = { RCC_TextureCube };

      // ������������Դ
      if(szName) {
        rs.strResourceName = szName;
        GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
        if(GXSUCCEEDED(hr)) {
          return TRUE;
        }
      }

      if ((pInitData && Width != Height * 6 && Width * 6 != Height) ||
        (pInitData == NULL && Width != Height)) {
        CLOG_ERROR("%s:�г�ʼ������ʱ����ҪWidth��Height��ȣ��г�ʼ������ʱ��Width��Height��ֵӦ����1:6��6:1", __FUNCTION__);
        return FALSE;
      }

      *ppTexture = NULL;
      TextureCubeImpl* pTexture = new TextureCubeImpl(this, Format, clMin(Width, Height), MipLevels, eResUsage);

      if(InlIsFailedToNewObject(pTexture)) {
        return FALSE;
      }

      if(_CL_NOT_(pTexture->InitTexture(FALSE, Width, Height, pInitData, nPitch)))
      {
        pTexture->Release();
        pTexture = NULL;
        return FALSE;
      }

      RegisterResource(pTexture, szName ? &rs : NULL);
      *ppTexture = pTexture;
      return TRUE;
    }

    GXBOOL GraphicsImpl::CreateTextureCubeFromMemory(TextureCube** ppTexture, GXLPCWSTR szName, clstd::Buffer* pBuffer, GXUINT MipLevels, GXResUsage eUsage)
    {
      GRESKETCH rs = { RCC_TextureCube };
      GXBOOL bval = FALSE;

      // ������������Դ
      if(szName) {
        rs.strResourceName = szName;
        GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
        if(GXSUCCEEDED(hr)) {
          return FALSE;
        }
      }

      clstd::MemBuffer buffer;
      clstd::IMAGEDESC sImgDesc;
      DECODE_TEXTURE_DESC desc = { &sImgDesc, &buffer };
      GXFormat format = Texture::DecodeToMemory(&desc, pBuffer->GetPtr(), (GXUINT)pBuffer->GetSize(), TRUE);

      if(format != GXFormat::Format_Unknown && (sImgDesc.width == sImgDesc.height * 6 || sImgDesc.width * 6 == sImgDesc.height))
      {
        bval = CreateTextureCube(ppTexture, NULL, sImgDesc.width, sImgDesc.height,
          format, eUsage, MipLevels, sImgDesc.ptr, sImgDesc.pitch);

        // �����ֵ�Ҫע��һ��
        if(bval && szName)
        {
          m_ResMgr.Unregister(*ppTexture); // TODO: ��ʱ��ôд�ɣ������ĺ��Ĺ��ܻ��ǵ��ᵽIntCreate��ȥ
          m_ResMgr.Register(&rs, *ppTexture);
          bval = TRUE;
        }
      }

      return bval;
    }

    GXBOOL GraphicsImpl::CreateTextureCubeFromFile(TextureCube** ppTexture, GXLPCWSTR szFilePath, GXUINT MipLevels, GXResUsage eUsage)
    {
      GRESKETCH rs = { RCC_TextureCube };
      clpathfile::CombinePath(rs.strResourceName, m_strResourceDir, szFilePath);

      // ������������Դ
      GXHRESULT hr = m_ResMgr.Find(reinterpret_cast<GResource**>(ppTexture), &rs);
      if(GXSUCCEEDED(hr)) {
        return TRUE;
      }

      clstd::File file;
      if(file.OpenExisting(rs.strResourceName))
      {
        clstd::MemBuffer buffer;
        if(file.Read(&buffer))
        {
          GXBOOL bval = CreateTextureCubeFromMemory(ppTexture, NULL, &buffer, MipLevels, eUsage);
          if(bval) {
            m_ResMgr.Unregister(*ppTexture); // TODO: ��ʱ��ôд�ɣ������ĺ��Ĺ��ܻ��ǵ��ᵽIntCreate��ȥ
            m_ResMgr.Register(&rs, *ppTexture);
          }
          return bval;
        }
      }
      return FALSE;
    }

    //GXHRESULT GXGraphicsImpl::CreateTextureCubeFromFileEx(TextureCube** ppTexture, 
    //  GXLPCWSTR pSrcFile, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, 
    //  GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo)
    //{
    //  CLBREAK;
    //  return GX_OK;
    //}


    //GXLRESULT GXGraphicsImpl::CreateOffscreenPlainSurface(
    //  Texture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage)
    //{
    //  *ppTexture = NULL;
    //  TextureOffscreenPlainSur *pGTex;
    //  pGTex = new TextureOffscreenPlainSur(this);
    //  if(pGTex)
    //  {
    //    pGTex->AddRef();
    //    RegisterResource(pGTex, NULL);
    //    pGTex->m_emType     = TextureImpl::OffscreenPlainSur;
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

  //////////////////////////////////////////////////////////////////////////

    //GXBOOL GraphicsImpl::D3DGetSwapChainDesc(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    //{
    //  *pSwapChainDesc = m_SwapChainDesc;
    //  return TRUE;
    //}

    GXBOOL GraphicsImpl::GetDesc(GXGRAPHICSDEVICE_DESC* pDesc)
    {
      pDesc->cbSize = sizeof(GXGRAPHICSDEVICE_DESC);
      pDesc->BackBufferWidth = m_SwapChainDesc.BufferDesc.Width;
      pDesc->BackBufferHeight = m_SwapChainDesc.BufferDesc.Height;
      pDesc->BackBufferCount = m_SwapChainDesc.BufferCount;
      pDesc->BackBufferFormat = (GXFormat)GXFMT_UNKNOWN;//m_SwapChainDesc.BufferDesc.Format;  // FIXME: �������!
      pDesc->DepthStencilFormat = (GXFormat)GXFMT_UNKNOWN;
      pDesc->RefreshRateInHz = m_SwapChainDesc.BufferDesc.RefreshRate.Numerator;
      pDesc->dwFlags = NULL;
      return TRUE;
    }

    // ��˽�� inline �����Լ��� cpp �ļ���
    inline void GraphicsImpl::InlUpdateTopology(GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount)
    {
      const D3D_PRIMITIVE_TOPOLOGY eTopology = GrapXToDX11::PrimitiveTopology(eType, nPrimCount, pVertCount);
      if(m_eCurTopology != eTopology) {
        m_pImmediateContext->IASetPrimitiveTopology(eTopology);
        m_eCurTopology = eTopology;
      }
    }

    GXBOOL GraphicsImpl::InlSetRenderTarget(RenderTarget* pTarget, GXUINT uRenderTargetIndex)
    {
      if(pTarget == NULL)
      {
        pTarget = m_pBackBufferRenderTarget;
      }

      RenderTargetImpl* pTargetImpl = static_cast<RenderTargetImpl*>(pTarget);

      if(m_pCurRenderTarget != pTargetImpl)
      {
        SAFE_RELEASE(m_pCurRenderTarget);
        SAFE_RELEASE(m_pCurRenderTargetView);
        SAFE_RELEASE(m_pCurDepthStencilView);
        m_pCurRenderTarget = pTargetImpl;
        m_pCurRenderTargetView = pTargetImpl->IntGetColorTextureUnsafe()->m_pD3D11RenderTargetView;
        if(pTargetImpl->IntGetDepthStencilTextureUnsafe())
        {
          m_pCurDepthStencilView = pTargetImpl->IntGetDepthStencilTextureUnsafe()->m_pD3D11DepthStencilView;
          m_pCurDepthStencilView->AddRef();
        }
        m_pCurRenderTarget->AddRef();
        m_pCurRenderTargetView->AddRef();

        m_pImmediateContext->OMSetRenderTargets(1, &m_pCurRenderTargetView, m_pCurDepthStencilView);
      }
      return TRUE;
    }

    GXBOOL GraphicsImpl::InlSetRenderTarget(Canvas3DImpl* pCanvas3D)
    {
      RenderTargetImpl** pTargets = &pCanvas3D->m_pTargets[0];
      RenderTargetImpl* pTargetImpl = static_cast<RenderTargetImpl*>(pTargets[0]);

      SAFE_RELEASE(m_pCurRenderTarget);
      SAFE_RELEASE(m_pCurRenderTargetView);
      SAFE_RELEASE(m_pCurDepthStencilView);

      if(pTargetImpl->IntGetDepthStencilTextureUnsafe())
      {
        m_pCurDepthStencilView = pTargetImpl->IntGetDepthStencilTextureUnsafe()->m_pD3D11DepthStencilView;
        m_pCurDepthStencilView->AddRef();
      }

      ID3D11RenderTargetView* rtv[MRT_SUPPORT_COUNT];

      m_pCurRenderTarget = pTargetImpl;
      m_pCurRenderTargetView = pTargetImpl->IntGetColorTextureUnsafe()->m_pD3D11RenderTargetView;
      m_pCurRenderTarget->AddRef();
      m_pCurRenderTargetView->AddRef();

      for(GXUINT i = 0; i < pCanvas3D->m_nTargetCount; i++)
      {
        rtv[i] = pTargets[i]->IntGetColorTextureUnsafe()->m_pD3D11RenderTargetView;
      }

      m_pImmediateContext->OMSetRenderTargets(pCanvas3D->m_nTargetCount, rtv, m_pCurDepthStencilView);
      return TRUE;
    }

    GXBOOL GraphicsImpl::SetSamplerState(GXUINT nStartSlot, GXUINT nSamplerCount, SamplerState** pSamplerStateArray)
    {
      SAFE_RELEASE(m_pCurSamplerState);
      ID3D11SamplerState*  temp_array[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
      SamplerStateImpl** pImplArray = reinterpret_cast<SamplerStateImpl**>(pSamplerStateArray);
      for(GXUINT i = 0; i < nSamplerCount; i++)
      {
        temp_array[i] = pImplArray[i]->m_pD3D11SamplerState;
      }
      m_pImmediateContext->PSSetSamplers(nStartSlot, nSamplerCount, temp_array);
      return TRUE;
    }

    StateResult GraphicsImpl::InlSetTexture(TexBaseImpl* pTexture, GXUINT uStage)
    {
#ifdef _DEBUG
      if (uStage >= MAX_TEXTURE_STAGE) {
        TRACE("Error: Stage out of range.\n");
        return StateResult::Failed;
      }
#endif // #ifdef _DEBUG

      if (pTexture == m_pCurTexture[uStage]) {
        return StateResult::Same;
      }

      SAFE_RELEASE(m_pCurTexture[uStage]);
      m_pCurTexture[uStage] = pTexture;

      if (m_pCurTexture[uStage] == NULL)
      {
        m_pImmediateContext->PSSetShaderResources(uStage, 1, NULL); // FIXME: ����ΪNULL
        return StateResult::Ok;
      }

      m_pCurTexture[uStage]->AddRef();
      ASSERT(pTexture->D3DResourceView());
      m_pImmediateContext->PSSetShaderResources(uStage, 1, &pTexture->D3DResourceView());
      return StateResult::Ok;
    }

    StateResult GraphicsImpl::InlSetShader(Shader* pShader) // û�иı䷵�� FALSE
    {
      if(m_pCurShader == pShader)
      {
        return StateResult::Same;
      }

      ASSERT(pShader != NULL);
      SAFE_RELEASE(m_pCurShader);
      SAFE_RELEASE(m_pVertexLayout);
      m_pCurShader = pShader;

      if(m_pCurShader != NULL)
      {
        m_pCurShader->AddRef();
        ((ShaderImpl*)m_pCurShader)->Activate();
      }
      return StateResult::Ok;
    }

    GXBOOL GraphicsImpl::IntSetEffect(EffectImpl* pEffectImpl)
    {
      ShaderImpl* pShaderImpl = static_cast<ShaderImpl*>(pEffectImpl->GetShaderUnsafe());
      return InlSetShader(pShaderImpl) != StateResult::Failed;
    }

  } // namespace D3D11
}
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)