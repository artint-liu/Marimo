#define ACCESS_AS(_TYPE, _ID)   (*(_TYPE*)(pConstBuf + m_StdCanvasUniform._ID))

Canvas3DImpl::Canvas3DImpl(GraphicsImpl* pGraphics)
  : Canvas3D      (2, RESTYPE_CANVAS3D)
  , m_pGraphicsImpl (pGraphics)
  //, m_xExt          (0)
  //, m_yExt          (0)
  , m_pTarget       (NULL)
  //, m_pImage        (NULL)
  , m_pCamera       (NULL)
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
  SAFE_RELEASE(m_pTarget);
  SAFE_RELEASE(m_pBlendState);
  //SAFE_RELEASE(m_pDepthStencil);
  SAFE_RELEASE(m_pSamplerState);
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
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXSIZE* Canvas3DImpl::GetTargetDimension(GXSIZE* pSize) const
{
  *pSize = m_sExtent;
  return pSize;
}

RenderTarget* Canvas3DImpl::GetTargetUnsafe() const
{
  return m_pTarget;
}

GXBOOL Canvas3DImpl::Initialize(RenderTarget* pTarget, GXLPCVIEWPORT pViewport)
{
  Texture* pTexture = NULL;
  if(pTarget == NULL)
  {
    m_pGraphicsImpl->GetBackBuffer(&pTarget);
  }

  
  InlSetNewObjectT(m_pTarget, static_cast<RenderTargetImpl*>(pTarget));

  //if((pTexture != NULL && (pTexture->GetUsage() & GXRU_TEX_RENDERTARGET)) ||
  //  pTexture == NULL)
  {
    ASSERT(m_pTarget == NULL);

    //m_pTargetTex = (GTextureImpl*)pTexture;
    //if(m_pTargetTex != NULL)
    //{
    //  m_pTargetTex->AddRef();
    //  m_pTargetTex->GetDimension((GXUINT*)&m_xExt, (GXUINT*)&m_yExt);
    //}
    //else
    //{
    //  GXGRAPHICSDEVICE_DESC GraphDeviceDesc;
    //  m_pGraphicsImpl->GetDesc(&GraphDeviceDesc);
    //  m_xExt = GraphDeviceDesc.BackBufferWidth;
    //  m_yExt = GraphDeviceDesc.BackBufferHeight;
    //}
    m_pTarget->GetDimension(&m_sExtent);

    if(m_pCurDepthStencilState == NULL)
    {
      GXDEPTHSTENCILDESC DepthStencil(TRUE, FALSE);
      m_pGraphicsImpl->CreateDepthStencilState((DepthStencilState**)&m_pCurDepthStencilState, &DepthStencil);
    }

    m_Viewport = *pViewport;
    SetupCanvasUniform();

    SAFE_RELEASE(pTexture);

    GRESKETCH sSketch;
    sSketch.dwCategoryId = RCC_Canvas3D;
    sSketch.strResourceName.Format(_CLTEXT("%x"), this);
    m_pGraphicsImpl->RegisterResource(this, &sSketch);
    return TRUE;
  }

  SAFE_RELEASE(pTexture);
  return FALSE;
}

Graphics* Canvas3DImpl::GetGraphicsUnsafe() const
{
  return m_pGraphicsImpl;
}

GXHRESULT Canvas3DImpl::SetMaterial(Material* pMtlInst)
{
  MaterialImpl* pMtlInstImpl = (MaterialImpl*)pMtlInst;
  Shader* pShader = pMtlInstImpl->InlGetShaderUnsafe();
  //GShaderStub* pShaderStub = pMtlInstImpl->InlGetShaderStubUnsafe();

#if 0
#ifdef REFACTOR_SHADER
  pMtlInstImpl->IntCommit((GXLPCBYTE)m_CanvasUniformBuf.GetPtr());
#else
  pMtlInstImpl->IntCommit(&m_StdUniforms);
#endif // REFACTOR_SHADER
#endif // 0

  m_pGraphicsImpl->InlSetShader(pShader);
  return TRUE;
}

GXHRESULT Canvas3DImpl::SetPrimitive(Primitive* pPrimitive)
{
  return m_pGraphicsImpl->SetPrimitive(pPrimitive);
}

GXHRESULT Canvas3DImpl::SetCamera(GCamera* pCamera)
{
  m_pCamera = pCamera;
  return GX_OK;
}

GCamera* Canvas3DImpl::GetCameraUnsafe()
{
  return m_pCamera;
}

GXHRESULT Canvas3DImpl::UpdateCommonUniforms()
{
  GCAMERACONETXT CameraCtx;
  GXLPBYTE pConstBuf = (GXLPBYTE)m_CanvasUniformBuf.GetPtr();

  m_pCamera->GetContext(&CameraCtx);

#ifdef REFACTOR_SHADER
  ACCESS_AS(float4x4, id_matViewProj) = CameraCtx.matView * CameraCtx.matProjection;
  ACCESS_AS(float4x4, id_matViewProjInv) = ACCESS_AS(float4x4, id_matViewProj);
  ACCESS_AS(float4x4, id_matViewProjInv).inverse();
  ACCESS_AS(float4x4, id_matWorldViewProj) = CameraCtx.matWorld * ACCESS_AS(float4x4, id_matViewProj);

  ACCESS_AS(float3, id_vViewPos) = m_pCamera->GetPos();
  ACCESS_AS(float3, id_vViewDir) = m_pCamera->GetFront();

  GXDWORD dwTick = gxGetTickCount();
  float4& vTime = ACCESS_AS(float4, id_vTime);
  vTime.w = (float)dwTick * 1e-3f;
  vTime.z = (float)(dwTick % 10000) * 1e-3f * CL_2PI;
  vTime.x = sin(vTime.z);
  vTime.y = cos(vTime.z);

  m_ViewFrustum.set(ACCESS_AS(float4x4, id_matWorldViewProj));
#else
  m_StdUniforms.g_matViewProj = CameraCtx.matView * CameraCtx.matProjection;
  m_StdUniforms.g_matViewProjInv = m_StdUniforms.g_matViewProj;
  m_StdUniforms.g_matViewProjInv.inverse();
  m_StdUniforms.g_matWorldViewProj = CameraCtx.matWorld * m_StdUniforms.g_matViewProj;

  m_StdUniforms.g_vViewPos = m_pCamera->GetPos();
  m_StdUniforms.g_vViewDir = m_pCamera->GetFront();

  GXDWORD dwTick = gxGetTickCount();
  m_StdUniforms.g_vTime.w = (float)dwTick * 1e-3f;
  m_StdUniforms.g_vTime.z = (float)(dwTick % 10000) * 1e-3f * CL_2PI;
  m_StdUniforms.g_vTime.x = sin(m_StdUniforms.g_vTime.z);
  m_StdUniforms.g_vTime.y = cos(m_StdUniforms.g_vTime.z);

  m_ViewFrustum.set(m_StdUniforms.g_matWorldViewProj);
#endif // #ifdef REFACTOR_SHADER
  return GX_OK;
}

GXHRESULT Canvas3DImpl::Draw(GVSequence* pSequence)
{
  typedef GVSequence::RenderDescArray RenderDescArray;
  Material* pMtlInst = NULL;
  const int nArrayCount = pSequence->GetArrayCount();
  GXLPCBYTE lpCanvasUniform = (GXLPCBYTE)m_CanvasUniformBuf.GetPtr();
  for(int nArrayIndex = 0; nArrayIndex < nArrayCount; nArrayIndex++)
  {
    const RenderDescArray& aDesc = pSequence->GetArray(nArrayIndex);
    for(RenderDescArray::const_iterator it = aDesc.begin();
      it != aDesc.end(); ++it)
    {
      const GVRENDERDESC& Desc = *it;

      if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
        SetWorldMatrix(Desc.matWorld);
      }

      if(TEST_FLAG(Desc.dwFlags, GVNF_CONTAINER)) {
        continue;
      }

      // 应用材质
      if(Desc.pMaterial != NULL) {
        if(pMtlInst != Desc.pMaterial)
        {
          SetMaterial(Desc.pMaterial);
          pMtlInst = Desc.pMaterial;
        }
        else {
          // TODO: 这个是否只有在 SetWorldMatrix() 后才需要提交?
          // TODO: 应该改为一个标准接口
#ifdef REFACTOR_SHADER
          static_cast<MaterialImpl*>(pMtlInst)->IntCommit(lpCanvasUniform);
#else
          static_cast<MaterialImpl*>(pMtlInst)->IntCommit(&m_StdUniforms);
#endif // #ifdef REFACTOR_SHADER
        }
      }
      else {
        //SetMaterialInst(m_pDefault);
        ASSERT(0);
      }

      ASSERT(Desc.pPrimitive != NULL);
      m_pGraphicsImpl->SetPrimitive(Desc.pPrimitive);
      //if(Desc.pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
      if(Desc.pPrimitive->GetIndexCount() > 0)
      {
        m_pGraphicsImpl->DrawPrimitive(Desc.ePrimType, 
          Desc.BaseVertexIndex, Desc.MinIndex, Desc.NumVertices, 
          Desc.StartIndex, Desc.PrimitiveCount);
      }
      else{
        ASSERT(0);
      }
      //++m_uDrawCallCount;

      if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
        SetWorldMatrix(float4x4::Identity);
      }
    }
  }
  return GX_OK;
}

void Canvas3DImpl::SetWorldMatrix(const float4x4& matWorld)
{
#ifdef REFACTOR_SHADER
  GXLPBYTE pConstBuf = (GXLPBYTE)m_CanvasUniformBuf.GetPtr();
  ACCESS_AS(float4x4, id_matWorldViewProj) = matWorld * ACCESS_AS(float4x4, id_matViewProj);
  ACCESS_AS(float4x4, id_matWorld)         = matWorld;
  ACCESS_AS(float4x4, id_matWorldView)     = matWorld * ACCESS_AS(float4x4, id_matView);
#else
  m_StdUniforms.g_matWorldViewProj = matWorld * m_StdUniforms.g_matViewProj;
  m_StdUniforms.g_matWorld = matWorld;
  m_StdUniforms.g_matWorldView = matWorld * m_StdUniforms.g_matView;
#endif // #ifdef REFACTOR_SHADER

  //m_StdUniforms.g_matWorldViewProjInv;
  //m_StdUniforms.g_matWorldInv;
  //m_StdUniforms.g_matWorldViewInv;
}

#ifdef REFACTOR_SHADER
GXDWORD Canvas3DImpl::GetGlobalHandle(GXLPCSTR szName)
{
  Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();
  GXINT_PTR handle = pConstNameObj->AllocHandle(szName, GXUB_UNDEFINED);

  if(pConstNameObj->GetSize() > m_CanvasUniformBuf.GetSize()) {
    BroadcastCanvasUniformBufferSize(pConstNameObj->GetSize());
  }

  return (GXDWORD)handle;
}

template<typename _Ty>
GXHRESULT Canvas3DImpl::SetCanvasUniformT(GXDWORD dwGlobalHandle, const _Ty& rUniform)
{
  if(dwGlobalHandle & (sizeof(float) - 1) || dwGlobalHandle > m_CanvasUniformBuf.GetSize()) {
    return GX_FAIL;
  }

  // FIXME: 应该做尺寸检查!
  _Ty* pUniform = (_Ty*)((GXLPBYTE)m_CanvasUniformBuf.GetPtr() + dwGlobalHandle);
  *pUniform = rUniform;
  return GX_OK;
}

GXHRESULT Canvas3DImpl::SetCanvasFloat(GXDWORD dwGlobalHandle, float fValue)
{
  return SetCanvasUniformT(dwGlobalHandle, fValue);
}

GXHRESULT Canvas3DImpl::SetCanvasVector(GXDWORD dwGlobalHandle, const float4& rVector)
{
  return SetCanvasUniformT(dwGlobalHandle, rVector);
}

GXHRESULT Canvas3DImpl::SetCanvasMatrix(GXDWORD dwGlobalHandle, const float4x4& rMatrix)
{
  return SetCanvasUniformT(dwGlobalHandle, rMatrix);
}

GXHRESULT Canvas3DImpl::SetCanvasFloat(GXLPCSTR szName, float fValue)
{
  GXDWORD dwHandle = GetGlobalHandle(szName);
  return SetCanvasUniformT(dwHandle, fValue);
}

GXHRESULT Canvas3DImpl::SetCanvasVector(GXLPCSTR szName, const float4& rVector)
{
  GXDWORD dwHandle = GetGlobalHandle(szName);
  return SetCanvasUniformT(dwHandle, rVector);
}

GXHRESULT Canvas3DImpl::SetCanvasMatrix(GXLPCSTR szName, const float4x4& rMatrix)
{
  GXDWORD dwHandle = GetGlobalHandle(szName);
  return SetCanvasUniformT(dwHandle, rMatrix);
}
#endif // #ifdef REFACTOR_SHADER

GXHRESULT Canvas3DImpl::Activate()
{
  if(m_pGraphicsImpl->InlSetCanvas(this) > 0)
  {
    m_pGraphicsImpl->SetViewport(&m_Viewport);
    m_pGraphicsImpl->SetSafeClip(NULL);
    //m_pGraphicsImpl->InlSetDepthStencil(m_pTarget->IntGetDepthStencilTextureUnsafe());
    m_pGraphicsImpl->InlSetDepthStencilState(m_pCurDepthStencilState);
  }
  return GX_OK;
}

GXLPCVIEWPORT Canvas3DImpl::GetViewport() const
{
  return &m_Viewport;
}

void Canvas3DImpl::SetViewport(GXVIEWPORT* pViewport)
{
  m_Viewport = *pViewport;
  if(m_pGraphicsImpl->IsActiveCanvas(this))  {
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
  const GXSIZE sizeHalf = {m_Viewport.regn.w / 2, m_Viewport.regn.h / 2};
  const GXPOINT ptCenter = {m_Viewport.regn.x + sizeHalf.cx, 
    m_Viewport.regn.y + sizeHalf.cy};
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
  return m_pTarget->GetDepthStencilTexture(ppDepthStencil);
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

GXHRESULT Canvas3DImpl::Clear(GXDWORD dwFlags, GXCOLOR crClear, GXFLOAT z, GXDWORD dwStencil)
{
  if(m_pGraphicsImpl->IsActiveCanvas(this))
  {
    return m_pGraphicsImpl->Clear(NULL, 0, dwFlags, crClear, z, dwStencil);
  }
  return GX_FAIL;     
}

#define ALLOC_GLOBAL_CONST(_NAME, _TYPE)  m_StdCanvasUniform.id_##_NAME = pConstNameObj->AllocHandle("g_"#_NAME, _TYPE);
void Canvas3DImpl::SetupCanvasUniform()
{
  Marimo::ShaderConstName* pConstNameObj = m_pGraphicsImpl->InlGetShaderConstantNameObj();

  ALLOC_GLOBAL_CONST(matViewProj,         GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matViewProj,         GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matViewProjInv,      GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matView,             GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matViewInv,          GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matProj,             GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matProjInv,          GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorldViewProj,    GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorldViewProjInv, GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorld,            GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorldInv,         GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorldView,        GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matWorldViewInv,     GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(matMainLight,        GXUB_MATRIX4);
  ALLOC_GLOBAL_CONST(vTime,               GXUB_FLOAT4);
  ALLOC_GLOBAL_CONST(fFPS,                GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(fFOV,                GXUB_FLOAT); 
  ALLOC_GLOBAL_CONST(vViewportDim,        GXUB_FLOAT2);
  ALLOC_GLOBAL_CONST(vViewportDimInv,     GXUB_FLOAT2);
  ALLOC_GLOBAL_CONST(fFarClipPlane,       GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(fNearClipPlane,      GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(fMouseCoordX,        GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(fMouseCoordY,        GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(vViewDir,            GXUB_FLOAT3);
  ALLOC_GLOBAL_CONST(vViewPos,            GXUB_FLOAT3);
  ALLOC_GLOBAL_CONST(vMainLightDir,       GXUB_FLOAT3);
  ALLOC_GLOBAL_CONST(vLightDiffuse,       GXUB_FLOAT3);
  ALLOC_GLOBAL_CONST(fLightIntensity,     GXUB_FLOAT);
  ALLOC_GLOBAL_CONST(vLightAmbient,       GXUB_FLOAT3);

  //m_CanvasUniformBuf.Resize(pConstNameObj->GetSize(), TRUE); // FIXME: 这里有点不太对，AllocHandle中会通知m_GlobalConstantBuf尺寸已经改变。
  if(pConstNameObj->GetSize() > m_CanvasUniformBuf.GetSize()) {
    BroadcastCanvasUniformBufferSize(pConstNameObj->GetSize());
  }

  // 单独设置缓冲大小，当前的Canvas3D还没有注册，所以收不到 
  // BroadcastCanvasUniformBufferSize 产生的广播消息
  ASSERT(m_CanvasUniformBuf.GetSize() == 0); // 确保当前Canvas3D没有收到广播
  m_CanvasUniformBuf.Resize(pConstNameObj->GetSize(), TRUE);


  // TODO: 重构完成后去除这些
#ifdef REFACTOR_SHADER
  GXLPBYTE pConstBuf = (GXLPBYTE)m_CanvasUniformBuf.GetPtr();
  ACCESS_AS(float3, id_vMainLightDir) = float3::normalize(float3(1.0f));
  ACCESS_AS(float3, id_vLightDiffuse) = float3(1.5f);
  ACCESS_AS(float3, id_vLightAmbient) = float3(0.2f);
  ACCESS_AS(float, id_fLightIntensity) = 3.8f;
#else
  m_StdUniforms.g_vMainLightDir = float3::normalize(float3(1.0f));
  m_StdUniforms.g_vLightDiffuse = float3(1.5f);
  m_StdUniforms.g_vLightAmbient = float3(0.2f);
  m_StdUniforms.g_fLightIntensity = 3.8f;
#endif // #ifdef REFACTOR_SHADER
}

GXHRESULT Canvas3DImpl::Invoke( GRESCRIPTDESC* pDesc )
{
  if(pDesc->szCmdString == NULL)
  {
    if(pDesc->dwCmdCode == RC_CanvasUniformSize && m_CanvasUniformBuf.GetSize() < (clsize)pDesc->wParam)
    {
      m_CanvasUniformBuf.Resize(pDesc->wParam, TRUE);
    }
  }
  return GX_OK;
}

void Canvas3DImpl::BroadcastCanvasUniformBufferSize( GXSIZE_T cbSize )
{
  GRESCRIPTDESC sScriptDesc = {NULL};
  sScriptDesc.dwCmdCode = RC_CanvasUniformSize;
  sScriptDesc.wParam = cbSize;
  m_pGraphicsImpl->BroadcastCategoryCommand(RCC_Canvas3D, &sScriptDesc);
}
