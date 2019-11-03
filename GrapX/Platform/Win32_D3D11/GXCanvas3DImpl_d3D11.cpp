
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
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
//#include "GrapX/MOLogger.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GPrimitiveImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderStubImpl_d3d11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_d3d11.h"

// 私有头文件
//#include <clPathFile.h>
#include "Platform/Win32_D3D11/GStateBlock_d3d11.h"
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "Canvas/GXEffectImpl.h"
#include "Canvas/GXMaterialImpl.h"
//#include "Console.h"
//#include <Smart/SmartStream.h>
//#include <Smart/SmartProfile.h>

//#include "GrapX/GXKernel.h"
//#include "GrapX/GXUser.h"
//#include <GDI/RegionFunc.h>
//#include <GDI/GRegionImpl.h>
#include "Platform/Win32_D3D11/GTextureImpl_d3d11.h"

//#include "Canvas/GXImageImpl.h"
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
//#include "Canvas/GXMaterialImpl.h"
//#include <FreeImage.h>

// Canvas3D用的

#include "GrapX/GCamera.h"
#include "GrapX/GrapVR.h"  // Canvas3D 用的
#include "Canvas/GXCanvas3DImpl.h"
#include "GXCanvas3DImpl_d3D11.h"
// </Canvas3D用的>

#define ACCESS_AS(_TYPE, _ID)   (*(_TYPE*)(pConstBuf + m_StdCanvasUniform._ID))

namespace GrapX
{
  namespace D3D11
  {
#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"

    Canvas3DImpl::Canvas3DImpl(GraphicsImpl* pGraphics)
      : m_pGraphicsImpl (pGraphics)
      //, m_xExt          (0)
      //, m_yExt          (0)
      //, m_pTarget       (NULL)
      //, m_pImage        (NULL)
      , m_pCamera       (NULL)
      //, m_pCurMaterialImpl  (NULL)
      , m_pBlendState   (NULL)
      //, m_pDepthStencil (NULL)
      , m_pSamplerState (NULL)
      , m_pCurDepthStencilState (NULL)
    {
      InlSetZeroT(m_sExtent);
#ifdef REFACTOR_SHADER
#else
      memset(&m_StdUniforms, 0, sizeof(m_StdUniforms));
#endif // REFACTOR_SHADER
    }

    Canvas3DImpl::~Canvas3DImpl()
    {
      m_pGraphicsImpl->UnregisterResource(this);
      SAFE_RELEASE(m_pCurDepthStencilState);
      //SAFE_RELEASE(m_pImage);
      SAFE_RELEASE(m_pCamera);
      //SAFE_RELEASE(m_pCurMaterialImpl);
      //SAFE_RELEASE(m_pTarget);
      SAFE_RELEASE(m_pBlendState);
      //SAFE_RELEASE(m_pDepthStencil);
      SAFE_RELEASE(m_pSamplerState);
      SAFE_RELEASE(m_pD3DCanvasBuffer);
    }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT Canvas3DImpl::AddRef()
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }

    GXHRESULT Canvas3DImpl::Release()
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      ASSERT(m_nRefCount >= 0);
      if (nRefCount == 0)
      {
        delete this;
        return GX_OK;
      }
      return nRefCount;
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXBOOL Canvas3DImpl::SetTarget(RenderTarget** pTargetArray, GXUINT nCount)
    {
      if((pTargetArray == NULL && nCount != 0) || (pTargetArray != NULL && nCount == 0))
      {
        CLOG_ERROR("%s(%d): 使用后台缓冲必须两个参数都为NULL", __FUNCTION__, __LINE__);
        return FALSE;
      }

      for(int i = 0; i < MRT_SUPPORT_COUNT; i++) {
        m_pTargets[i] = NULL;
      }


      if((pTargetArray == NULL && nCount == 0) || (pTargetArray[0] == NULL && nCount == 1))
      {
        RenderTarget* pTarget = NULL;
        m_pGraphicsImpl->GetBackBuffer(&pTarget);
        m_pTargets[0] = static_cast<RenderTargetImpl*>(pTarget);
        m_nTargetCount = 1;
        m_pTargets[0]->GetDimension(&m_sExtent);
        SAFE_RELEASE(pTarget);

        if(m_pGraphicsImpl->IsActiveCanvas(this))
        {
          m_pGraphicsImpl->InlSetRenderTarget(NULL, 0);
        }

      }
      else
      {
        GXSIZE sExtent, sOther;
        if(pTargetArray[0] == NULL) {
          CLOG_ERROR("%s(%d): 后台缓冲不能使用多目标渲染", __FUNCTION__, __LINE__);
          return FALSE;
        }

        pTargetArray[0]->GetDimension(&sExtent);

        for(GXUINT i = 1; i < nCount; i++)
        {
          if(pTargetArray[i] == NULL) {
            CLOG_ERROR("%s(%d): 后台缓冲不能使用多目标渲染", __FUNCTION__, __LINE__);
            return FALSE;
          }
          pTargetArray[i]->GetDimension(&sOther);
          if(sOther.cx != sExtent.cx || sOther.cy != sExtent.cy) {
            CLOG_ERROR("%s(%d): 多目标渲染尺寸不一致", __FUNCTION__, __LINE__);
            return NULL;
          }
        }

        for(size_t i = 0; i < nCount; i++)
        {
          m_pTargets[i] = static_cast<RenderTargetImpl*>(pTargetArray[i]);
        }

        if(m_pGraphicsImpl->IsActiveCanvas(this))
        {
          //m_pGraphicsImpl->InlSetRenderTarget(this);
          m_pGraphicsImpl->InlSetCanvas(NULL);
        }

        m_sExtent = sExtent;
        m_nTargetCount = nCount;
      }

      // TODO: 设置深度模板缓冲

      m_Viewport.regn.Set(0, 0, m_sExtent.cx, m_sExtent.cy);
      return TRUE;
    }

    GXBOOL Canvas3DImpl::SetTarget(RenderTarget* pTarget)
    {
      return Canvas3DImpl::SetTarget(&pTarget, 1);
    }

    GXSIZE* Canvas3DImpl::GetTargetDimension(GXSIZE* pSize) const
    {
      *pSize = m_sExtent;
      return pSize;
    }

    RenderTarget* Canvas3DImpl::GetTargetUnsafe() const
    {
      return m_pTargets[0];
    }

    float Canvas3DImpl::GetAspect() const
    {
      return (float)m_Viewport.regn.w / (float)m_Viewport.regn.h;
    }

    RenderTarget* Canvas3DImpl::GetTargetUnsafe(GXUINT index) const
    {
      return index < m_nTargetCount ? m_pTargets[index].CastPtr<RenderTarget>() : NULL;
    }

    GXBOOL Canvas3DImpl::Initialize(RenderTarget** pTargetArray, GXUINT nCount, GXLPCVIEWPORT pViewport)
    {
      D3D11_BUFFER_DESC bd = {sizeof(STD_CANVAS_UNIFORM), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0};
      HRESULT hr = m_pGraphicsImpl->D3DGetDevice()->CreateBuffer(&bd, NULL, &m_pD3DCanvasBuffer);


      //m_pTargets[0]->GetDimension(&m_sExtent);
      if(_CL_NOT_(Canvas3DImpl::SetTarget(pTargetArray, nCount))) {
        return FALSE;
      }
      m_Viewport = *pViewport;

      // TODO: 去掉这个
      if (m_pCurDepthStencilState == NULL)
      {
        GXDEPTHSTENCILDESC DepthStencil(TRUE, FALSE);
        m_pGraphicsImpl->CreateDepthStencilState((DepthStencilState**)&m_pCurDepthStencilState, &DepthStencil);
      }

      SetupCanvasUniform();

      Camera::Create(&m_pCamera, GetAspect(), CL_DEG2RAD(75.0f), float3::Origin, float3::AxisZ);

      //SAFE_RELEASE(pTexture);

      GRESKETCH sSketch;
      sSketch.dwCategoryId = RCC_Canvas3D;
      sSketch.strResourceName.Format(_CLTEXT("%x"), this);
      m_pGraphicsImpl->RegisterResource(this, &sSketch);
      return SUCCEEDED(hr);
    }

    Graphics* Canvas3DImpl::GetGraphicsUnsafe() const
    {
      return m_pGraphicsImpl;
    }

    GXBOOL Canvas3DImpl::SetMaterial(Material* pMtlInst)
    {
      MaterialImpl* pMtlInstImpl = reinterpret_cast<MaterialImpl*>(pMtlInst);
      ShaderImpl* pShaderImpl = reinterpret_cast<ShaderImpl*>(pMtlInstImpl->InlGetShaderUnsafe());

      StateResult r = m_pGraphicsImpl->InlSetShader(pShaderImpl);
      if(r == StateResult::Failed) {
        return FALSE;
      }

      if(m_CurMaterialImpl.operator!=(pMtlInstImpl))
      {
        m_CurMaterialImpl = pMtlInstImpl;
        pMtlInstImpl->CommitStates();
        r = StateResult::Ok;
      }

      pMtlInstImpl->CommitTextures(r != StateResult::Same);
      if(pMtlInstImpl->GetDataPoolUnsafe() && ((pMtlInstImpl->GetFlags() & MATERIAL_FLAG_UNIFORM_CHANGED) || r != StateResult::Same))
      {
        if(pMtlInstImpl->GetFlags() & MATERIAL_FLAG_UNIFORM_CHANGED) {
          pShaderImpl->UploadConstBuffer(m_pContext, pMtlInstImpl->GetDeviceDependBuffer(), pMtlInstImpl->GetDataPoolUnsafe());
        }
        pShaderImpl->CommitConstantBuffer(m_pContext, pMtlInstImpl->GetDeviceDependBuffer(), m_pD3DCanvasBuffer);
      }
      pMtlInstImpl->ClearFlags();

      return TRUE;
    }

    GXHRESULT Canvas3DImpl::SetPrimitive(Primitive* pPrimitive)
    {
      return m_pGraphicsImpl->SetPrimitive(pPrimitive);
    }

    void Canvas3DImpl::SetCamera(Camera* pCamera)
    {
      if(pCamera)
      {
        SAFE_RELEASE(m_pCamera);
        m_pCamera = pCamera;
        pCamera->AddRef();
      }
    }

    Camera* Canvas3DImpl::GetCameraUnsafe()
    {
      return m_pCamera;
    }

    GXHRESULT Canvas3DImpl::UpdateCommonUniforms()
    {
      GCAMERACONETXT CameraCtx;
      //GXLPBYTE pConstBuf = (GXLPBYTE)m_CanvasUniformBuf.GetPtr();

      m_pCamera->GetContext(&CameraCtx);


      GXDWORD dwTick = gxGetTickCount();
      //float4& vTime = ACCESS_AS(float4, id_vTime);
      //vTime.w = (float)dwTick * 1e-3f;
      //vTime.z = (float)(dwTick % 10000) * 1e-3f * CL_2PI;
      //vTime.x = sin(vTime.z);
      //vTime.y = cos(vTime.z);
      m_StdCanvasUniform._CameraWorldPos = m_pCamera->GetPos();
      m_StdCanvasUniform._CameraWorldDir = m_pCamera->GetFront();

      m_StdCanvasUniform._Time.x = (float)dwTick * 1e-3f;
      m_StdCanvasUniform._Time.y = (float)(dwTick % 10000) * 1e-3f * CL_2PI;
      m_StdCanvasUniform._Time.z = cos(m_StdCanvasUniform._Time.y);
      m_StdCanvasUniform._Time.w = sin(m_StdCanvasUniform._Time.y);

      m_StdCanvasUniform.MARIMO_MATRIX_VP = CameraCtx.matView * CameraCtx.matProjection;
      m_StdCanvasUniform.MARIMO_MATRIX_I_VP = m_StdCanvasUniform.MARIMO_MATRIX_VP;
      m_StdCanvasUniform.MARIMO_MATRIX_I_VP.inverse();

      m_ViewFrustum.set(m_StdCanvasUniform.MARIMO_MATRIX_VP);

      SetWorldMatrix(float4x4::Identity);
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::Draw(GVSequence* pSequence)
    {
      typedef GVSequence::RenderDescArray RenderDescArray;
      Material* pMtlInst = NULL;
      const int nArrayCount = pSequence->GetArrayCount();
      const int nRenderCategory = pSequence->GetRenderCategory();

      for (int nArrayIndex = 0; nArrayIndex < nArrayCount; nArrayIndex++)
      {
        const RenderDescArray& aDesc = pSequence->GetArray(nArrayIndex);
#ifdef ENABLE_MULTIMAP_RENDERING_SORTING
        for(const clpair<int, GVRENDERDESC2*>& r_pair : aDesc)
        {
          const GVRENDERDESC2* pRenderer = r_pair.second;
#else
        for (const GVRENDERDESC2* pRenderer : aDesc)
        {
#endif
          const GXDWORD dwFlags = pRenderer->pNode->GetFlags();
          if (TEST_FLAG(dwFlags, GVNF_UPDATEWORLDMAT)) {
            SetWorldMatrix(pRenderer->pNode->GetTransform().GlobalMatrix);
          }

          ASSERT(TEST_FLAG(dwFlags, GVNF_CONTAINER) == 0);
          Material* pMaterial = pRenderer->materials[nRenderCategory];

          // 应用材质
          if (pMaterial != NULL) {
            if (pMtlInst != pMaterial)
            {
              SetMaterial(pMaterial);
              pMtlInst = pMaterial;
            }
          }
          else {
            //SetMaterialInst(m_pDefault);
            ASSERT(0);
          }

          ASSERT(pRenderer->pPrimitive != NULL);
          m_pGraphicsImpl->SetPrimitive(pRenderer->pPrimitive);
          //if(Desc.pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
          if (pRenderer->pPrimitive->GetIndexCount() > 0)
          {
            m_pGraphicsImpl->DrawPrimitive(pRenderer->ePrimType,
              pRenderer->BaseVertexIndex, pRenderer->MinIndex, pRenderer->NumVertices,
              pRenderer->StartIndex, pRenderer->PrimitiveCount);
          }
          else {
            ASSERT(0);
          }
          //++m_uDrawCallCount;

          if (TEST_FLAG(dwFlags, GVNF_UPDATEWORLDMAT)) {
            SetWorldMatrix(float4x4::Identity);
          }
        }
      }
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::Draw(Material* pMaterial, GVNode* pNode, const float4x4* pTransform)
    {
      GVRENDERDESC Desc;
      const int nRenderCate = 0;
      pNode->GetRenderDesc(nRenderCate, &Desc);

      if (TEST_FLAG(Desc.dwFlags, GVNF_CONTAINER)) {
        return GX_FAIL;
      }

      if (pTransform) {
        SetWorldMatrix(*pTransform);
      }

      m_CurMaterialImpl = NULL;
      //m_pGraphicsImpl->InlSetShader(pShader);
      SetMaterial(pMaterial);

      ASSERT(Desc.pPrimitive != NULL);
      m_pGraphicsImpl->SetPrimitive(Desc.pPrimitive);
      //if(Desc.pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
      if (Desc.pPrimitive->GetIndexCount() > 0)
      {
        m_pGraphicsImpl->DrawPrimitive(Desc.ePrimType,
          Desc.BaseVertexIndex, Desc.MinIndex, Desc.NumVertices,
          Desc.StartIndex, Desc.PrimitiveCount);
      }
      else {
        ASSERT(0);
      }

      if (pTransform) {
        SetWorldMatrix(float4x4::Identity);
      }

      return GX_OK;
    }

    void Canvas3DImpl::SetWorldMatrix(const float4x4& matWorld)
    {
      GCAMERACONETXT CameraCtx;
      m_pCamera->GetContext(&CameraCtx);

      m_StdCanvasUniform.MARIMO_MATRIX_M = matWorld;
      m_StdCanvasUniform.MARIMO_MATRIX_V = CameraCtx.matView;
      m_StdCanvasUniform.MARIMO_MATRIX_P = CameraCtx.matProjection;
      m_StdCanvasUniform.MARIMO_MATRIX_MVP = matWorld * CameraCtx.matView * CameraCtx.matProjection;

      m_pGraphicsImpl->D3DGetDeviceContext()->UpdateSubresource(m_pD3DCanvasBuffer, 0, NULL, &m_StdCanvasUniform, 0, 0);
    }

#ifdef REFACTOR_SHADER
    //GXDWORD Canvas3DImpl::GetGlobalHandle(GXLPCSTR szName)
    //{
    //  Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();
    //  GXINT_PTR handle = pConstNameObj->AllocHandle(szName, GXUB_UNDEFINED);

    //  //if(pConstNameObj->GetSize() > m_CanvasUniformBuf.GetSize()) {
    //  //  BroadcastCanvasUniformBufferSize(pConstNameObj->GetSize());
    //  //}

    //  return (GXDWORD)handle;
    //}

    template<typename _Ty>
    GXHRESULT Canvas3DImpl::SetCanvasUniformT(GXDWORD dwGlobalHandle, const _Ty& rUniform)
    {
      //if(dwGlobalHandle & (sizeof(float) - 1) || dwGlobalHandle > m_CanvasUniformBuf.GetSize()) {
      //  return GX_FAIL;
      //}

      //// FIXME: 应该做尺寸检查!
      //_Ty* pUniform = (_Ty*)((GXLPBYTE)m_CanvasUniformBuf.GetPtr() + dwGlobalHandle);
      //*pUniform = rUniform;
      return GX_OK;
    }

    //GXHRESULT Canvas3DImpl::SetCanvasFloat(GXDWORD dwGlobalHandle, float fValue)
    //{
    //  return SetCanvasUniformT(dwGlobalHandle, fValue);
    //}

    //GXHRESULT Canvas3DImpl::SetCanvasVector(GXDWORD dwGlobalHandle, const float4& rVector)
    //{
    //  return SetCanvasUniformT(dwGlobalHandle, rVector);
    //}

    //GXHRESULT Canvas3DImpl::SetCanvasMatrix(GXDWORD dwGlobalHandle, const float4x4& rMatrix)
    //{
    //  return SetCanvasUniformT(dwGlobalHandle, rMatrix);
    //}

    //GXHRESULT Canvas3DImpl::SetCanvasFloat(GXLPCSTR szName, float fValue)
    //{
    //  GXDWORD dwHandle = GetGlobalHandle(szName);
    //  return SetCanvasUniformT(dwHandle, fValue);
    //}

    //GXHRESULT Canvas3DImpl::SetCanvasVector(GXLPCSTR szName, const float4& rVector)
    //{
    //  GXDWORD dwHandle = GetGlobalHandle(szName);
    //  return SetCanvasUniformT(dwHandle, rVector);
    //}

    //GXHRESULT Canvas3DImpl::SetCanvasMatrix(GXLPCSTR szName, const float4x4& rMatrix)
    //{
    //  GXDWORD dwHandle = GetGlobalHandle(szName);
    //  return SetCanvasUniformT(dwHandle, rMatrix);
    //}
#endif // #ifdef REFACTOR_SHADER

    GXBOOL Canvas3DImpl::Begin()
    {
      if (m_pGraphicsImpl->InlSetCanvas(this) > 0)
      {
        m_pGraphicsImpl->SetViewport(&m_Viewport);
        m_pGraphicsImpl->SetSafeClip(NULL);
        //m_pGraphicsImpl->InlSetDepthStencil(m_pTarget->IntGetDepthStencilTextureUnsafe());
        m_pGraphicsImpl->InlSetDepthStencilState(m_pCurDepthStencilState);
        m_CurMaterialImpl = NULL;
        m_pContext = m_pGraphicsImpl->GetCurrentContext();
      }
      return TRUE;
    }

    GXBOOL Canvas3DImpl::End()
    {
      m_pContext = NULL;
      return TRUE;
    }

    GXLPCVIEWPORT Canvas3DImpl::GetViewport() const
    {
      return &m_Viewport;
    }

    void Canvas3DImpl::SetViewport(GXVIEWPORT* pViewport)
    {
      m_Viewport = *pViewport;
      if (m_pGraphicsImpl->IsActiveCanvas(this)) {
        m_pGraphicsImpl->SetViewport(pViewport);
      }
    }

    //////////////////////////////////////////////////////////////////////////
    //
    // 各种坐标系的变换
    //
    GXHRESULT Canvas3DImpl::TransformPosition(const float3* pPos, GXOUT float4* pView)
    {
      GCAMERACONETXT ctx;
      ctx.dwMask = GCC_WVP;
      m_pCamera->GetContext(&ctx);

      *pView = pPos->transform(ctx.matWorld * ctx.matView * ctx.matProjection);
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::PositionToView(const float3* pPos, GXOUT float3* pView)
    {
      GCAMERACONETXT ctx;
      ctx.dwMask = GCC_WVP;
      m_pCamera->GetContext(&ctx);

      float4 vOut = pPos->transform(ctx.matWorld * ctx.matView * ctx.matProjection);
      *pView = vOut;
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::PositionToScreen(const float3* pPos, GXOUT GXPOINT* ptScreen)
    {
      float3 vView;
      PositionToView(pPos, &vView);

      ptScreen->x = (GXINT)((vView.x + 1.0f) * 0.5f * (float)m_Viewport.regn.w);
      ptScreen->y = (GXINT)((1.0f - vView.y) * 0.5f * (float)m_Viewport.regn.h);
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::PositionFromScreen(const GXPOINT* pScreen, float fDepth, GXOUT float3* pWorldPos)
    {
      const GXSIZE sizeHalf = { m_Viewport.regn.w / 2, m_Viewport.regn.h / 2 };
      const GXPOINT ptCenter = { m_Viewport.regn.x + sizeHalf.cx,
        m_Viewport.regn.y + sizeHalf.cy };
      float3 vPos((float)(pScreen->x - ptCenter.x) / (float)sizeHalf.cx,
        -(float)(pScreen->y - ptCenter.y) / (float)sizeHalf.cy, fDepth);

      return PositionFromView(&vPos, pWorldPos);
    }

    GXHRESULT Canvas3DImpl::PositionFromView(const float3* pView, GXOUT float3* pWorldPos)
    {
      GCAMERACONETXT ctx;
      ctx.dwMask = GCC_WVP;
      m_pCamera->GetContext(&ctx);

      float4x4 matWVPInv = float4x4::inverse(ctx.matWorld * ctx.matView * ctx.matProjection);
      *pWorldPos = *pView * matWVPInv;
      return GX_OK;
    }

    GXHRESULT Canvas3DImpl::RayFromScreen(const GXPOINT* pScreen, GXOUT Ray* pRay)
    {
      float3 vWorldPos;
      PositionFromScreen(pScreen, 1.0f - 1e-5f, &vWorldPos);
      const float3 vCameraPos = m_pCamera->GetPos();
      pRay->set(vCameraPos, vWorldPos - vCameraPos);
      return GX_OK;
    }
    //////////////////////////////////////////////////////////////////////////

    GXHRESULT Canvas3DImpl::GetDepthStencil(Texture** ppDepthStencil)
    {
      //if(m_pDepthStencil == NULL) {
      //  return GX_FAIL;
      //}

      //*ppDepthStencil = m_pDepthStencil;
      //return m_pDepthStencil->AddRef();
      return m_pTargets[0]->GetDepthStencilTexture(ppDepthStencil);
    }

    const Canvas3D::FrustumPlanes* Canvas3DImpl::GetViewFrustum()
    {
      return &m_ViewFrustum;
    }

#ifdef REFACTOR_SHADER
#else
    STANDARDMTLUNIFORMTABLE* Canvas3DImpl::GetStandardUniform()
    {
      return &m_StdUniforms;
    }
#endif // REFACTOR_SHADER

    GXBOOL Canvas3DImpl::Clear(GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags)
    {
      GXColor color = crClear;
      return Canvas3DImpl::Clear(color, z, dwStencil, dwFlags);
    }

    GXBOOL Canvas3DImpl::Clear(const GXColor& crClear, GXFLOAT z, GXDWORD dwStencil, GXDWORD dwFlags)
    {
      if (m_pGraphicsImpl->IsActiveCanvas(this))
      {
        m_pGraphicsImpl->Clear(NULL, 0, dwFlags, crClear, z, dwStencil);

        if (TEST_FLAG(dwFlags, GXCLEAR_TARGET))
        {
          for (GXUINT i = 1; i < m_nTargetCount; i++)
          {
            m_pGraphicsImpl->D3DGetDeviceContext()->ClearRenderTargetView(
              m_pTargets[i]->IntGetColorTextureUnsafe()->D3DGetRenderTargetView(), (const FLOAT*)&crClear);
          }
        }
        return TRUE;
      }
      return FALSE;
    }

#define ALLOC_GLOBAL_CONST(_NAME, _TYPE)  m_StdCanvasUniform.id_##_NAME = pConstNameObj->AllocHandle("g_"#_NAME, _TYPE);
    void Canvas3DImpl::SetupCanvasUniform()
    {
      Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();

      //ALLOC_GLOBAL_CONST(matViewProj,         GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matViewProj,         GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matViewProjInv,      GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matView,             GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matViewInv,          GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matProj,             GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matProjInv,          GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorldViewProj,    GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorldViewProjInv, GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorld,            GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorldInv,         GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorldView,        GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matWorldViewInv,     GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(matMainLight,        GXUB_MATRIX4);
      //ALLOC_GLOBAL_CONST(vTime,               GXUB_FLOAT4);
      //ALLOC_GLOBAL_CONST(fFPS,                GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(fFOV,                GXUB_FLOAT); 
      //ALLOC_GLOBAL_CONST(vViewportDim,        GXUB_FLOAT2);
      //ALLOC_GLOBAL_CONST(vViewportDimInv,     GXUB_FLOAT2);
      //ALLOC_GLOBAL_CONST(fFarClipPlane,       GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(fNearClipPlane,      GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(fMouseCoordX,        GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(fMouseCoordY,        GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(vViewDir,            GXUB_FLOAT3);
      //ALLOC_GLOBAL_CONST(vViewPos,            GXUB_FLOAT3);
      //ALLOC_GLOBAL_CONST(vMainLightDir,       GXUB_FLOAT3);
      //ALLOC_GLOBAL_CONST(vLightDiffuse,       GXUB_FLOAT3);
      //ALLOC_GLOBAL_CONST(fLightIntensity,     GXUB_FLOAT);
      //ALLOC_GLOBAL_CONST(vLightAmbient,       GXUB_FLOAT3);

      //m_CanvasUniformBuf.Resize(pConstNameObj->GetSize(), TRUE); // FIXME: 这里有点不太对，AllocHandle中会通知m_GlobalConstantBuf尺寸已经改变。
      //if(pConstNameObj->GetSize() > m_CanvasUniformBuf.GetSize()) {
      //  BroadcastCanvasUniformBufferSize(pConstNameObj->GetSize());
      //}

      // 单独设置缓冲大小，当前的Canvas3D还没有注册，所以收不到 
      // BroadcastCanvasUniformBufferSize 产生的广播消息
      //ASSERT(m_CanvasUniformBuf.GetSize() == 0); // 确保当前Canvas3D没有收到广播
      //m_CanvasUniformBuf.Resize(pConstNameObj->GetSize(), TRUE);


      // TODO: 重构完成后去除这些
#ifdef REFACTOR_SHADER
  //GXLPBYTE pConstBuf = (GXLPBYTE)m_CanvasUniformBuf.GetPtr();
  //ACCESS_AS(float3, id_vMainLightDir) = float3::normalize(float3(1.0f));
  //ACCESS_AS(float3, id_vLightDiffuse) = float3(1.5f);
  //ACCESS_AS(float3, id_vLightAmbient) = float3(0.2f);
  //ACCESS_AS(float, id_fLightIntensity) = 3.8f;
      m_StdCanvasUniform.MARIMO_MATRIX_M = float4x4::Identity;
      m_StdCanvasUniform.MARIMO_MATRIX_V = float4x4::Identity;
      m_StdCanvasUniform.MARIMO_MATRIX_P = float4x4::Identity;
      m_StdCanvasUniform.MARIMO_MATRIX_MVP = float4x4::Identity;

#else
      m_StdUniforms.g_vMainLightDir = float3::normalize(float3(1.0f));
      m_StdUniforms.g_vLightDiffuse = float3(1.5f);
      m_StdUniforms.g_vLightAmbient = float3(0.2f);
      m_StdUniforms.g_fLightIntensity = 3.8f;
#endif // #ifdef REFACTOR_SHADER
    }

    GXHRESULT Canvas3DImpl::Invoke(GRESCRIPTDESC* pDesc)
    {
      if (pDesc->szCmdString == NULL)
      {
        //if(pDesc->dwCmdCode == RC_CanvasUniformSize && m_CanvasUniformBuf.GetSize() < (clsize)pDesc->wParam)
        //{
        //  m_CanvasUniformBuf.Resize(pDesc->wParam, TRUE);
        //}
      }
      return GX_OK;
    }

    void Canvas3DImpl::BroadcastCanvasUniformBufferSize(GXSIZE_T cbSize)
    {
      GRESCRIPTDESC sScriptDesc = { NULL };
      sScriptDesc.dwCmdCode = RC_CanvasUniformSize;
      sScriptDesc.wParam = cbSize;
      m_pGraphicsImpl->BroadcastCategoryCommand(RCC_Canvas3D, &sScriptDesc);
    }
  }
}
