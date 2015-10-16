
//////////////////////////////////////////////////////////////////////////
GXCanvasImpl::GXCanvasImpl(GXGraphicsImpl* pGraphics, GXBOOL bStatic)
  : GXCanvasCoreImpl    (pGraphics, 2, RESTYPE_CANVAS2D)
  , m_bStatic           (bStatic)
  , m_pTargetImage      (NULL)
  , m_xAbsOrigin        (0)
  , m_yAbsOrigin        (0)
  , m_xOrigin           (0)
  , m_yOrigin           (0)
  //, m_pBlendingState    (NULL)
  //, m_pOpaqueState      (NULL)
  , m_pPrimitive        (NULL)
  , m_lpLockedVertex    (NULL)
  , m_lpLockedIndex     (NULL)
  , m_pWhiteTex         (NULL)
  , m_uIndexCount       (NULL)
  , m_uVertCount        (0)
  , m_uVertIndexSize    (0)
  , m_aBatch            (NULL)
  , m_uBatchCount       (0)
  , m_uBatchSize        (0)
  , s_uDefVertIndexSize (128)
  , s_uDefBatchSize     (32)
  , m_dwStencil         (0)
  , m_pClipRegion       (NULL)
  , m_dwTexVertColor    (-1)
  , m_dwColorAdditive   (0)
  , m_dwTexSlot         (NULL)
  , m_pRasterizerState  (NULL)
{
  InlSetZeroT(m_rcClip);
  InlSetZeroT(m_rcAbsClip);
  gxRtlZeroMemory(&m_aTextureStage, sizeof(m_aTextureStage));
  gxRtlZeroMemory(&m_pBlendingState, sizeof(m_pBlendingState));
  gxRtlZeroMemory(&m_pOpaqueState, sizeof(m_pOpaqueState));
  gxRtlZeroMemory(&m_pCanvasStencil, sizeof(m_pCanvasStencil));
}

GXCanvasImpl::~GXCanvasImpl()
{
}

GXBOOL GXCanvasImpl::Initialize(GTexture* pTexture, GXCONST REGN* pRegn)
{
  ASSERT(m_uBatchCount == 0);

  if(GXCanvasCoreImpl::Initialize(pTexture) == TRUE)
  {
    if(pRegn != NULL)
    {
      GXRECT rcTexture;

      m_xAbsOrigin 
        = m_LastState.xOrigin
        = m_xOrigin 
        = m_rcClip.left 
        = pRegn->left;

      m_yAbsOrigin 
        = m_LastState.yOrigin
        = m_yOrigin 
        = m_rcClip.top
        = pRegn->top;

      m_rcClip.right  = pRegn->left + pRegn->width;
      m_rcClip.bottom = pRegn->top  + pRegn->height;

      gxSetRect(&rcTexture, 0, 0, m_xExt, m_yExt);
      gxIntersectRect(&m_rcClip, &rcTexture, &m_rcClip);
    }
    else
    {
      m_xAbsOrigin
        = m_LastState.xOrigin
        = m_xOrigin 
        = m_rcClip.left 
        = 0;

      m_yAbsOrigin
        = m_LastState.yOrigin
        = m_yOrigin 
        = m_rcClip.top
        = 0;
      m_rcClip.right  = m_xExt;
      m_rcClip.bottom = m_yExt;
    }

    m_rcAbsClip = m_rcClip;
    m_LastState.rcClip = m_rcClip;
    m_LastState.pEffectImpl = m_pEffectImpl;

    m_uVertIndexSize = s_uDefVertIndexSize;
    m_uBatchSize     = s_uDefBatchSize;

    if(m_pPrimitive == NULL) {
      m_pGraphics->CreatePrimitiveVI(&m_pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P4T2F_C1D), 
        GXRU_FREQUENTLYWRITE, m_uVertIndexSize, m_uVertIndexSize, sizeof(CANVAS_PRMI_VERT), 
        NULL, NULL);
    }

    if(m_aBatch == NULL)
      m_aBatch = new BATCH[m_uBatchSize];

    if(m_pCamera == NULL) {
      m_pCamera = GCamera_ScreenAligned::Create((GXCanvasCore*)(GXCanvas*)this);
    }

    // 初始化渲染模式
    m_LastState.eCompMode = CM_SourceOver;
    m_LastState.dwColorAdditive = 0;
    m_aBatch[m_uBatchCount++].Set(CF_CompositingMode, 0, 0, CM_SourceOver);
    m_dwTexVertColor = (GXDWORD)-1;
    m_dwColorAdditive = 0;
    m_eStyle = PS_Solid;

    //GXUIGetStock()->pSimpleShader->SetColor(0xffffffff);

    // 初始化空纹理时的替换纹理
    if(m_pWhiteTex == NULL) {
      GXHRESULT hval = m_pGraphics->CreateTexture(&m_pWhiteTex, "CanvasWhiteTex8x8", 8, 8, 0, GXFMT_A8R8G8B8, GXRU_DEFAULT);
      if(hval == 0) { // 只有在首次创建时才清除颜色
        m_pWhiteTex->Clear(NULL, 0xffffffff);
      }
    }

    // 创建BlendState
    if(m_pBlendingState[0] == NULL || m_pBlendingState[1] == NULL ||
      m_pOpaqueState[0] == NULL || m_pOpaqueState[1] == NULL)
    {
      GXBLENDDESC AlphaBlendState(TRUE);
      GXBLENDDESC OpaqueState;

      ASSERT(m_pBlendingState[0] == NULL);
      ASSERT(m_pBlendingState[1] == NULL);
      ASSERT(m_pOpaqueState[0] == NULL);
      ASSERT(m_pOpaqueState[1] == NULL);

      // ---
      GXRASTERIZERDESC RasterizerDesc;
      RasterizerDesc.ScissorEnable = TRUE;
      m_pGraphics->CreateRasterizerState((GRasterizerState**)&m_pRasterizerState, &RasterizerDesc);

      // ---
      m_pGraphics->CreateBlendState((GBlendState**)&m_pBlendingState[0], &AlphaBlendState, 1);

      // ---
      AlphaBlendState.SeparateAlphaBlend = TRUE;
      AlphaBlendState.BlendOpAlpha = GXBLENDOP_ADD;
      AlphaBlendState.SrcBlendAlpha = GXBLEND_INVDESTALPHA;
      AlphaBlendState.DestBlendAlpha = GXBLEND_ONE;
      m_pGraphics->CreateBlendState((GBlendState**)&m_pBlendingState[1], &AlphaBlendState, 1);

      // ---
      m_pGraphics->CreateBlendState((GBlendState**)&m_pOpaqueState[0], &OpaqueState, 1);

      // ---
      OpaqueState.BlendEnable = TRUE;
      m_pGraphics->CreateBlendState((GBlendState**)&m_pOpaqueState[1], &OpaqueState, 1);
    }

    if(m_pCanvasStencil[0] == NULL || m_pCanvasStencil[1] == NULL)
    {
      ASSERT(m_pCanvasStencil[0] == NULL && m_pCanvasStencil[1] == NULL);
      GXDEPTHSTENCILDESC DepthStencil(FALSE, FALSE);
      m_pGraphics->CreateDepthStencilState((GDepthStencilState**)&m_pCanvasStencil[0], &DepthStencil);

      DepthStencil.StencilEnable = TRUE;
      DepthStencil.StencilReadMask = 0xff;
      DepthStencil.StencilWriteMask = 0xff;
      DepthStencil.FrontFace.StencilFunc = GXCMP_EQUAL;
      m_pGraphics->CreateDepthStencilState((GDepthStencilState**)&m_pCanvasStencil[1], &DepthStencil);
    }

    // 初始化寄存器常量
    GCAMERACONETXT gcc;
    gcc.dwMask = GCC_WORLD;
    m_pCamera->GetContext(&gcc);
    m_LastState.matTransform = gcc.matWorld;
    m_CanvasCommConst.matWVProj = gcc.matWorld;
    m_CanvasCommConst.colorMul.set(1,1,1,1);
    m_CanvasCommConst.colorAdd.set(0,0,0,1);

    ASSERT(m_dwTexSlot == NULL);
    return TRUE;
  }
  return FALSE;
}
GXINT GXCanvasImpl::UpdateStencil(GRegion* pClipRegion)
{
  if(pClipRegion != m_pClipRegion)
  {
    SAFE_RELEASE(m_pClipRegion);
    m_pClipRegion = pClipRegion;
  }

  RGNCOMPLEX eCompx = RC_SIMPLE;
  REGN rgClip;

  if(m_pClipRegion == NULL)
  {
    m_rcClip = m_rcAbsClip;
    //m_LastState.rcClip = m_rcAbsClip;
    //m_pRenderState->Set(GXRS_STENCILENABLE, FALSE);
    m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
  }
  else
  {
    eCompx = m_pClipRegion->GetComplexity();
    switch(eCompx)
    {
    case RC_ERROR:
      m_rcClip = m_rcAbsClip;
      //m_LastState.rcClip = m_rcAbsClip;
      //m_pRenderState->Set(GXRS_STENCILENABLE, FALSE);
      m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
      break;
    case RC_NULL:
      m_pClipRegion->GetBounding(&m_rcClip);
      //m_LastState.rcClip = m_rcClip;
      break;
    case RC_SIMPLE:
      m_pClipRegion->GetBounding(&m_rcClip);
      //m_LastState.rcClip = m_rcClip;
      //m_pRenderState->Set(GXRS_STENCILENABLE, FALSE);
      m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);
      break;
    case RC_COMPLEX:
      {
        const GXUINT nRectCount = m_pClipRegion->GetRectCount();
        GXRECT* lpRects = _GlbLockStaticRects(nRectCount);

        m_pClipRegion->GetRects(lpRects, nRectCount);
        m_pClipRegion->GetBounding(&m_rcClip);
        //m_LastState.rcClip = m_rcClip;

        // 如果不先设置Clear在区域之外的会失败
        gxRectToRegn(&rgClip, &m_rcClip);
        m_pGraphics->SetSafeClip(&rgClip);

        // TODO: 考虑是否在以后用快速Region求补来填充空白区域呢?
        // 如下填充实现了: 在一个矩形区域内,需要绘制图形的部分模板值>1,其他部分模板值为0
        const GXDWORD dwFlags = GXCLEAR_STENCIL;
        //const GXDWORD dwFlags = GXCLEAR_STENCIL|GXCLEAR_TARGET;
        m_pGraphics->Clear(&m_rcClip, 1, dwFlags, 0xff0000ff, 0, 0);
        m_pGraphics->Clear(lpRects, nRectCount, dwFlags, 0xff00ff00, 0, m_dwStencil);

        //m_pRenderState->Set(GXRS_STENCILENABLE,    TRUE);
        //m_pRenderState->Set(GXRS_STENCILFAIL,      GXSTENCILOP_KEEP);
        //m_pRenderState->Set(GXRS_STENCILZFAIL,     GXSTENCILOP_KEEP);
        //m_pRenderState->Set(GXRS_STENCILPASS,      GXSTENCILOP_KEEP);
        //m_pRenderState->Set(GXRS_STENCILFUNC,      GXCMP_EQUAL);
        //m_pRenderState->Set(GXRS_STENCILREF,       m_dwStencil);
        //m_pRenderState->Set(GXRS_STENCILMASK,      0xffffffff);
        //m_pRenderState->Set(GXRS_STENCILWRITEMASK, 0xffffffff);
        m_pCanvasStencil[1]->SetStencilRef(m_dwStencil);
        m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[1]);

        _GlbUnlockStaticRects(lpRects);
      }
      return (GXINT)eCompx;
    }
  }
  gxRectToRegn(&rgClip, &m_rcClip);
  m_pGraphics->SetSafeClip(&rgClip);
  return (GXINT)eCompx;
}

GXBOOL GXCanvasImpl::Initialize(GXImage* pImage, GXCONST REGN* pRegn)
{
  // TODO: 如果不支持Renderable则将内部纹理重新创建为RenderTarget
  ASSERT(m_pTargetImage == NULL);
  m_pTargetImage = pImage;
  if(pImage != NULL)
  {
    m_pTargetImage->AddRef();
    return Initialize(pImage->GetTextureUnsafe(), pRegn);
  }
  return Initialize((GTexture*)NULL, pRegn);
}

//#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXCanvasImpl::Release()
{
  if(m_uBatchCount != 0) {
    Flush();
  }

  // 最多两个引用
  ASSERT(m_uRefCount > 0 && m_uRefCount <= 3);
  m_uRefCount--;

  // Graphics 内置 Canvas 在一个引用计数时释放
  // 否则在引用计数为 0 时释放
  if((m_bStatic == TRUE && m_uRefCount == 1) ||
    m_uRefCount == 0)
  {
    SAFE_RELEASE(m_pEffectImpl);
    SAFE_RELEASE(m_pTargetTex);
    SAFE_RELEASE(m_pTargetImage);
    m_dwStencil = 0;

    SAFE_RELEASE(m_pClipRegion);

    if(m_dwTexSlot != NULL)
    {
      ASSERT(m_aTextureStage[0] == NULL);
      for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && m_dwTexSlot != 0; i++)
      {
        if(m_aTextureStage[i] != NULL)
        {
          m_pGraphics->InlSetTexture(NULL, i);
          m_aTextureStage[i]->Release();
          m_aTextureStage[i] = NULL;
          RESETBIT(m_dwTexSlot, i);
        }
      }
    }
    ASSERT(m_dwTexSlot == NULL);

    m_uVertCount  = 0;
    m_uIndexCount = 0;
    m_uBatchCount = 0;
  }

  if(m_uRefCount == 0)
  {
    SAFE_RELEASE(m_pOpaqueState[0]);
    SAFE_RELEASE(m_pOpaqueState[1]);
    SAFE_RELEASE(m_pRasterizerState);
    SAFE_RELEASE(m_pBlendingState[0]);
    SAFE_RELEASE(m_pBlendingState[1]);
    SAFE_RELEASE(m_pCanvasStencil[0]);
    SAFE_RELEASE(m_pCanvasStencil[1]);
    SAFE_RELEASE(m_pWhiteTex);
    SAFE_RELEASE(m_pPrimitive);
    SAFE_DELETE_ARRAY(m_aBatch);
    delete this;
    return GX_OK;
  }
  return m_uRefCount;
}
//#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXULONG GXCanvasImpl::GetRef()
{
  ASSERT(m_uRefCount > 0 && m_uRefCount <= 3);
  return m_uRefCount;
}

GXBOOL GXCanvasImpl::_CanFillBatch(GXUINT uVertCount, GXUINT uIndexCount)
{
  return ((m_uVertCount + uVertCount) < m_uVertIndexSize && 
    (m_uIndexCount + uIndexCount) < m_uVertIndexSize &&
    (m_uBatchCount + 1) < m_uBatchSize);
}

// 返回值是Index的索引基值
GXUINT GXCanvasImpl::PrepareBatch(CanvasFunc eFunc, GXUINT uVertCount, GXUINT uIndexCount, GXLPARAM lParam)
{
  if(_CanFillBatch(uVertCount, uIndexCount) == FALSE)
    Flush();

  // 队列为空或者与上一个命令不符,则新建一个命令
  if(m_uBatchCount == 0 || m_aBatch[m_uBatchCount - 1].eFunc != eFunc ||
    (eFunc == CF_Textured && m_aBatch[m_uBatchCount - 1].comm.lParam != lParam))    // TODO: 是不是为了通用,以后可以改成仅仅比较lParam参数?
    //        因为 lParam 不同肯定是不同的队列
  {
    m_aBatch[m_uBatchCount++].Set(eFunc, uVertCount, uIndexCount, lParam);
    return 0;
  }
  ASSERT(m_uBatchCount > 0);
  const GXUINT uBaseIndex = m_aBatch[m_uBatchCount - 1].uVertexCount;
  m_aBatch[m_uBatchCount - 1].uVertexCount += uVertCount;
  m_aBatch[m_uBatchCount - 1].uIndexCount  += uIndexCount;
  m_aBatch[m_uBatchCount - 1].comm.lParam  = lParam;
  return uBaseIndex;
}


void GXCanvasImpl::SetStencil(GXDWORD dwStencil)
{
  m_dwStencil = dwStencil;
}

void GXCanvasImpl::_SetPrimitivePos(GXUINT nIndex, const GXINT _x, const GXINT _y)
{
  //m_lpLockedVertex[nIndex].x = (GXFLOAT)_x;
  //m_lpLockedVertex[nIndex].y = (GXFLOAT)_y;
  // TODO: 貌似这里应该是m_xOrigin, m_yOrigin, 因为 m_LastState 已经是最后的结果了, m_xOrigin是中间结果
  m_lpLockedVertex[nIndex].x = (GXFLOAT)m_LastState.xOrigin + _x;
  m_lpLockedVertex[nIndex].y = (GXFLOAT)m_LastState.yOrigin + _y;
}

void GXCanvasImpl::BATCH::Set(CanvasFunc _eFunc, GXUINT _uVertexCount, GXUINT _uIndexCount, GXLPARAM _lParam)
{
  eFunc        = _eFunc;
  uVertexCount = _uVertexCount;
  uIndexCount  = _uIndexCount;
  comm.lParam  = _lParam;
  if(eFunc == CF_Textured) {
    ((GTexture*)comm.lParam)->AddRef();
  }
}

void GXCanvasImpl::BATCH::Set2(CanvasFunc _eFunc, GXINT x, GXINT y)
{
  eFunc  = _eFunc;
  PosI.x = x;
  PosI.y = y;
  PosI.z = 0;
  PosI.w = 0;
}

void GXCanvasImpl::BATCH::SetFloat4(CanvasFunc _eFunc, float x, float y, float z, float w)
{
  eFunc  = _eFunc;
  PosF.x = x;
  PosF.y = y;
  PosF.z = z;
  PosF.w = w;
}

void GXCanvasImpl::BATCH::SetRenderState(GXUINT nCode, GXDWORD dwValue)
{
  eFunc            = CF_RenderState;
  nRenderStateCode = nCode;
  dwStateValue     = dwValue;
  comm.lParam      = NULL;
}

GXBOOL GXCanvasImpl::CommitState()
{
  REGN regn;

  // 如果返回0, 说明Graphics使用的就是当前的 Canvas, 所以不用初始化这些东东
  if(m_pGraphics->InlSetCanvas(this) > 0)
  {
    m_pGraphics->SetPrimitive(m_pPrimitive);
    //m_pGraphics->InlSetRenderState(m_pRenderState);
    m_pGraphics->InlSetRasterizerState(m_pRasterizerState);
    m_pGraphics->InlSetSamplerState(m_pSamplerState);
    m_pGraphics->InlSetDepthStencil(NULL);

    m_pGraphics->InlSetEffect((GXEffectImpl*)m_pEffectImpl);
    UpdateStencil(m_pClipRegion);

    gxRectToRegn(&regn, &m_rcClip);
    m_pGraphics->SetSafeClip(&regn);  // TODO: 是不是应该把这个改为GXRECT
    m_pGraphics->SetViewport(NULL);

    float4 vColorAdditive = m_dwColorAdditive;
    m_CanvasCommConst.colorAdd = vColorAdditive;
    //*(float4*)&FXPCOMMREG(ColorAdd) = vColorAdditive;

    //m_pGraphics->SetVertexShaderConstantF(0, (GXFLOAT*)&m_aVertexShaderRegister, CANVAS_SHARED_SHADER_REGCOUNT);
    //m_pGraphics->SetPixelShaderConstantF(0, (GXFLOAT*)&m_aPixelShaderRegister, CANVAS_SHARED_SHADER_REGCOUNT);
    m_pEffectImpl->CommitUniform(this, -1);

    m_pGraphics->InlSetTexture(reinterpret_cast<GTexBaseImpl*>(m_pWhiteTex), 0);

    GXDWORD dwTexSlot = m_dwTexSlot;
    if(dwTexSlot != NULL)
    {
      for(GXUINT i = 1; i < GX_MAX_TEXTURE_STAGE && dwTexSlot != 0; i++)
      {
        if(m_aTextureStage[i] != NULL)
        {
          m_pGraphics->InlSetTexture(reinterpret_cast<GTexBaseImpl*>(m_aTextureStage[i]), i);
          RESETBIT(dwTexSlot, i);
        }
      }
    }
  }
  return TRUE;
}


GXGraphics* GXCanvasImpl::GetGraphicsUnsafe() GXCONST
{
  return m_pGraphics;
}

GXBOOL GXCanvasImpl::SetTransform(const float4x4* matTransform)
{
  if(matTransform == NULL) {
    return FALSE;
  }

  if(m_uBatchCount + 4 >= m_uBatchSize) {
    Flush();
  }

  const float* m = matTransform->m;
  m_aBatch[m_uBatchCount++].SetFloat4(CF_SetTransform, m[ 0], m[ 1], m[ 2], m[ 3]);
  m_aBatch[m_uBatchCount++].SetFloat4(CF_SetTransform, m[ 4], m[ 5], m[ 6], m[ 7]);
  m_aBatch[m_uBatchCount++].SetFloat4(CF_SetTransform, m[ 8], m[ 9], m[10], m[11]);
  m_aBatch[m_uBatchCount++].SetFloat4(CF_SetTransform, m[12], m[13], m[14], m[15]);

  m_LastState.matTransform = *matTransform;
  return TRUE;
}

GXBOOL GXCanvasImpl::GetTransform(float4x4* matTransform) GXCONST
{
  *matTransform = m_LastState.matTransform;
  return TRUE;
}

GXBOOL GXCanvasImpl::SetViewportOrg(GXINT x, GXINT y, GXLPPOINT lpPoint)
{
  // !!! 没测试过
  if(lpPoint)
  {
    if(m_uBatchCount > 0)
      Flush();
    lpPoint->x = m_xAbsOrigin - m_xOrigin;
    lpPoint->y = m_yAbsOrigin - m_yOrigin;

    ASSERT(m_xOrigin == m_LastState.xOrigin);
    ASSERT(m_yOrigin == m_LastState.yOrigin);

    m_xOrigin = m_xAbsOrigin - x;
    m_yOrigin = m_yAbsOrigin - y;
    ASSERT(0);  // TODO: 验证 这个不就是 m_LastState(xOrigin/yOrigin) 里的值吗?
  }
  else
  {
    if( ! ((m_uBatchCount + 1) < m_uBatchSize)) {
      Flush();
    }
    m_aBatch[m_uBatchCount++].Set2(CF_SetViewportOrg, m_xAbsOrigin - x, m_yAbsOrigin - y);
  }
  m_LastState.xOrigin = m_xAbsOrigin - x;
  m_LastState.yOrigin = m_yAbsOrigin - y;
  return TRUE;
}

GXBOOL GXCanvasImpl::GetViewportOrg(GXLPPOINT lpPoint) GXCONST
{
  if(lpPoint == NULL)
    return FALSE;

  lpPoint->x = m_xAbsOrigin - m_LastState.xOrigin;
  lpPoint->y = m_yAbsOrigin - m_LastState.yOrigin;

  return TRUE;
}

GXVOID GXCanvasImpl::EnableAlphaBlend(GXBOOL bEnable)
{

}

GXBOOL GXCanvasImpl::Flush()
{
  if(m_uBatchCount == 0)
    return FALSE;

  GXBOOL bEmptyRect = gxIsRectEmpty(&m_rcClip);

  if(m_lpLockedVertex != NULL)
  {
    m_pPrimitive->Unlock();
    m_lpLockedVertex = NULL;
  }
  m_pGraphics->Enter();
  CommitState();  // TODO: 如果 bEmptyRect 为 TRUE, 则不提交状态

  GXUINT nBaseVertex = 0;
  GXUINT nStartIndex = 0;

  for(GXUINT i = 0; i < m_uBatchCount; i++)
  {
    const CanvasFunc eFunc = m_aBatch[i].eFunc;

    if(bEmptyRect && (eFunc > CF_DrawFirst && eFunc < CF_DrawLast)) {
      continue;
    }

    switch(eFunc)
    {
    case CF_LineList:
      {
        TRACE_BATCH("CF_LineList\n");
        if(bEmptyRect == FALSE) {
          m_pGraphics->DrawPrimitive(GXPT_LINELIST, 
          nBaseVertex, 0, m_aBatch[i].uVertexCount, nStartIndex, m_aBatch[i].uIndexCount / 2);
        }

        nBaseVertex += m_aBatch[i].uVertexCount;
        nStartIndex += m_aBatch[i].uIndexCount;
      }
      break;
    case CF_Points:
      {
        TRACE_BATCH("CF_Points\n");
        if(bEmptyRect == FALSE)
          m_pGraphics->DrawPrimitive(GXPT_POINTLIST, nBaseVertex, m_aBatch[i].uVertexCount);
        nBaseVertex += m_aBatch[i].uVertexCount;
      }
      break;
    case CF_Triangle:
      {
        TRACE_BATCH("CF_Trangle\n");
        if(bEmptyRect == FALSE)
          m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST, 
          nBaseVertex, 0, m_aBatch[i].uVertexCount, nStartIndex, m_aBatch[i].uIndexCount / 3);
        nBaseVertex += m_aBatch[i].uVertexCount;
        nStartIndex += m_aBatch[i].uIndexCount;
      }
      break;
    case CF_Textured:
      {
        TRACE_BATCH("CF_Textured\n");
        GTexture* pTexture = (GTexture*)m_aBatch[i].comm.lParam;

        if(bEmptyRect == FALSE)
        {
          m_pGraphics->InlSetTexture(reinterpret_cast<GTexBaseImpl*>(pTexture), 0);
          m_pGraphics->DrawPrimitive(GXPT_TRIANGLELIST, 
            nBaseVertex, 0, m_aBatch[i].uVertexCount, nStartIndex, m_aBatch[i].uIndexCount / 3);
        }

        nBaseVertex += m_aBatch[i].uVertexCount;
        nStartIndex += m_aBatch[i].uIndexCount;

        m_pGraphics->InlSetTexture(reinterpret_cast<GTexBaseImpl*>(m_pWhiteTex), 0);
        pTexture->Release();
      }
      break;
    case CF_RenderState:
      {
        TRACE_BATCH("CF_RenderState\n");
        ASSERT(0); // 已经去掉了
        //m_pRenderState->Set(
        //  (GXRenderStateType)m_aBatch[i].nRenderStateCode, m_aBatch[i].dwStateValue);
      }
      break;
    case CF_SetSamplerState:
      {
        GXSAMPLERDESC* pDesc = (GXSAMPLERDESC*)m_aBatch[i].comm.lParam;
        m_pSamplerState->SetState(m_aBatch[i].comm.dwFlag, pDesc);
        SAFE_DELETE(pDesc);
      }
      break;

    case CF_Clear:
      {
        TRACE_BATCH("CF_Clear\n");
        if(m_pClipRegion == NULL)
        {
          GXRECT rect;
          rect.left   = m_rcClip.left;
          rect.top    = m_rcClip.top;
          rect.right  = m_rcClip.right;
          rect.bottom = m_rcClip.bottom;
          const GXHRESULT hRet = // Debug
            m_pGraphics->Clear(&rect, 1, m_aBatch[i].comm.dwFlag, (GXCOLOR)m_aBatch[i].comm.wParam, 1.0f, m_dwStencil);
          ASSERT(GXSUCCEEDED(hRet));
        }
        else
        {
          const GXUINT nRectCount = m_pClipRegion->GetRectCount();
          GXRECT* lpRects = _GlbLockStaticRects(nRectCount);
          m_pClipRegion->GetRects(lpRects, nRectCount);
          m_pGraphics->Clear(lpRects, nRectCount, m_aBatch[i].comm.dwFlag, (GXCOLOR)m_aBatch[i].comm.wParam, 1.0f, m_dwStencil);
          _GlbUnlockStaticRects(lpRects);
        }
      }
      break;
    case CF_CompositingMode:
      {
        TRACE_BATCH("CF_CompositingMode\n");
        //LPGXRENDERSTATE lpRenderStateBlock = NULL;
        GBlendStateImpl* pBlendState = NULL;

        // 判断是不是最终渲染目标
        if(m_pTargetTex == NULL)
        {
          if(m_aBatch[i].comm.lParam == CM_SourceCopy)
            pBlendState = m_pOpaqueState[0];
          else 
            pBlendState = m_pBlendingState[0];
        }
        else
        {
          if(m_aBatch[i].comm.lParam == CM_SourceCopy)
            pBlendState = m_pOpaqueState[1];
          else 
            pBlendState = m_pBlendingState[1];
        }
        //m_pRenderState->SetBlock(lpRenderStateBlock);
        m_pGraphics->InlSetBlendState(pBlendState);
      }
      break;
    case CF_Effect:
      {
        TRACE_BATCH("CF_Effect\n");
        if(m_pEffectImpl == (GXEffectImpl*)m_aBatch[i].comm.lParam)
          break;
        SAFE_RELEASE(m_pEffectImpl);
        m_pEffectImpl = (GXEffectImpl*)m_aBatch[i].comm.lParam;
        m_pGraphics->InlSetEffect(m_pEffectImpl);
        m_pEffectImpl->CommitUniform(this, -1);
      }
      break;
    case CF_ColorAdditive:
      {
        TRACE_BATCH("CF_ColorAdditive\n");
        m_dwColorAdditive = (GXDWORD)m_aBatch[i].comm.lParam;
        m_CanvasCommConst.colorAdd = m_dwColorAdditive;
        m_pEffectImpl->CommitUniform(this, MEMBER_OFFSET(GXCANVASCOMMCONST, colorAdd));
      }
      break;
#ifdef GLES2_CANVAS_IMPL
      //case CF_SetPixelEffectConst:
      //  ASSERT(0);
      //  break;
    //case CF_SetSamplerState:
    //  break;
#elif defined(D3D9_CANVAS_IMPL)
    case CF_SetUniform1f:
      {
        m_pEffectImpl->SetUniformByHandle(this, m_aBatch[i].Handle, (float*)&m_aBatch[i].PosF, 1);
      }
      break;
    case CF_SetUniform2f:
      {
        //float4 fValue(m_aBatch[i].PosF.x, m_aBatch[i].PosF.y, 0, 1);
        m_pEffectImpl->SetUniformByHandle(this, m_aBatch[i].Handle, (float*)&m_aBatch[i].PosF, 2);
      }
      break;
    case CF_SetUniform3f:
      {
        //float4 fValue(m_aBatch[i].PosF.x, m_aBatch[i].PosF.y, 0, 1);
        m_pEffectImpl->SetUniformByHandle(this, m_aBatch[i].Handle, (float*)&m_aBatch[i].PosF, 3);
      }
      break;
    case CF_SetUniform4f:
      {
        //float4 fValue(m_aBatch[i].PosF.x, m_aBatch[i].PosF.y, 0, 1);
        m_pEffectImpl->SetUniformByHandle(this, m_aBatch[i].Handle, (float*)&m_aBatch[i].PosF, 4);
      }
      break;
    case CF_SetUniform4x4f:
      {
        float* pMat = (float*)m_aBatch[i].comm.lParam;
        m_pEffectImpl->SetUniformByHandle(this, m_aBatch[i].Handle, pMat, 16);
        delete pMat;
      }
      break;
    //case CF_SetEffectConst:
    //  {
    //    const GXUINT uCount = m_aBatch[i].uVertexCount;
    //    const GXUINT uIndex = m_aBatch[i].comm.wParam;
    //    clBuffer* pBuffer = (clBuffer*)m_aBatch[i].lParam;
    //    //memcpy(&m_aPixelShaderRegister[uIndex], pBuffer->GetPtr(), pBuffer->GetSize());
    //    //for(GXUINT i = 0; i < uCount; i++)
    //    //{
    //    //  m_aPixelShaderRegister[uIndex + u].x = 
    //    //}
    //    ASSERT((pBuffer->GetSize()%sizeof(float4)) == 0);
    //    m_pGraphics->SetPixelShaderConstantF(uIndex, (GXFLOAT*)pBuffer->GetPtr(), pBuffer->GetSize()/sizeof(float4));
    //    delete pBuffer;
    //  }
    //  break;
#elif defined(D3D11_CANVAS_IMPL)
#else
#error 需要定义inl的环境
#endif
    case CF_SetViewportOrg:
      TRACE_BATCH("CF_SetViewportOrg\n");
      m_xOrigin = m_aBatch[i].PosI.x;
      m_yOrigin = m_aBatch[i].PosI.y;
      break;
    case CF_SetClipBox:
      {
        TRACE_BATCH("CF_SetClipBox\n");
        GXREGN rgClip;
        SAFE_RELEASE(m_pClipRegion);

        m_rcClip.left   = GXLOWORD(m_aBatch[i].comm.wParam);
        m_rcClip.right  = GXHIWORD(m_aBatch[i].comm.wParam);

        m_rcClip.top    = GXLOWORD(m_aBatch[i].comm.lParam);
        m_rcClip.bottom = GXHIWORD(m_aBatch[i].comm.lParam);

        //m_LastState.rcClip = m_rcClip;
        //m_pRenderState->Set(GXRS_STENCILENABLE, FALSE);
        m_pGraphics->InlSetDepthStencilState(m_pCanvasStencil[0]);

        gxRectToRegn(&rgClip, &m_rcClip);
        m_pGraphics->SetSafeClip(&rgClip);

        if(gxIsRectEmpty(&m_rcClip) == TRUE)
          bEmptyRect = TRUE;
      }
      break;
    case CF_ResetClipBox:
      TRACE_BATCH("CF_ResetClipBox\n");
      UpdateStencil(NULL);
      break;
    case CF_ResetTextClip:
      {
        GXREGN rgClip;
        if(m_pClipRegion != NULL)
        {
          GXRECT rcReset;
          m_pClipRegion->GetBounding(&rcReset);
          gxRectToRegn(&rgClip, &rcReset);
        }
        else
          gxRectToRegn(&rgClip, &m_rcAbsClip);
        m_pGraphics->SetSafeClip(&rgClip);
      }
      break;
    case CF_SetTextClip:
      {
        GXRECT rcClip;
        GXREGN rgClip;
        rcClip.left   = GXLOWORD(m_aBatch[i].comm.wParam);
        rcClip.right  = GXHIWORD(m_aBatch[i].comm.wParam);
        rcClip.top    = GXLOWORD(m_aBatch[i].comm.lParam);
        rcClip.bottom = GXHIWORD(m_aBatch[i].comm.lParam);
        gxRectToRegn(&rgClip, &m_rcClip);
        m_pGraphics->SetSafeClip(&rgClip);
      }
      break;
    case CF_SetRegion:
      {
        TRACE_BATCH("CF_SetRegion\n");
        if(UpdateStencil((GRegion*)m_aBatch[i].comm.lParam) == RC_NULL)
          bEmptyRect = TRUE;
        else
          bEmptyRect = FALSE;
      }
      break;
    case CF_NoOperation:
      TRACE_BATCH("CF_NoOperation\n");
      // 啥也没有!!
      break;
    case CF_SetExtTexture:
      {
        const GXUINT uStage = (GXUINT)m_aBatch[i].comm.wParam;
        GTexture* pTexture = (GTexture*)m_aBatch[i].comm.lParam;
        m_pGraphics->InlSetTexture(reinterpret_cast<GTexBaseImpl*>(pTexture), uStage);
        SAFE_RELEASE(m_aTextureStage[uStage]);
        m_aTextureStage[uStage] = (GTextureImpl*)pTexture;

        if(pTexture != NULL)
          SETBIT(m_dwTexSlot, uStage);
        else
          RESETBIT(m_dwTexSlot, uStage);
      }
      break;
    case CF_SetTransform:
      {
        ASSERT(m_aBatch[i].eFunc == CF_SetTransform && i + 3 < m_uBatchSize);
        ASSERT(m_aBatch[i + 1].eFunc == CF_SetTransform);
        ASSERT(m_aBatch[i + 2].eFunc == CF_SetTransform);
        ASSERT(m_aBatch[i + 3].eFunc == CF_SetTransform);

        float* m = m_CanvasCommConst.matWVProj.m;
        m[ 0] = m_aBatch[i    ].PosF.x;   m[ 1] = m_aBatch[i    ].PosF.y;   m[ 2] = m_aBatch[i    ].PosF.z;   m[ 3] = m_aBatch[i    ].PosF.w;
        m[ 4] = m_aBatch[i + 1].PosF.x;   m[ 5] = m_aBatch[i + 1].PosF.y;   m[ 6] = m_aBatch[i + 1].PosF.z;   m[ 7] = m_aBatch[i + 1].PosF.w;
        m[ 8] = m_aBatch[i + 2].PosF.x;   m[ 9] = m_aBatch[i + 2].PosF.y;   m[10] = m_aBatch[i + 2].PosF.z;   m[11] = m_aBatch[i + 2].PosF.w;
        m[12] = m_aBatch[i + 3].PosF.x;   m[13] = m_aBatch[i + 3].PosF.y;   m[14] = m_aBatch[i + 3].PosF.z;   m[15] = m_aBatch[i + 3].PosF.w;
        m_pEffectImpl->CommitUniform(this, -1);
        i += 3;
      }
      break;
    default:
      ASSERT(0);
    }
  }
  m_uVertCount  = 0;
  m_uIndexCount = 0;
  m_uBatchCount = 0;

  m_pGraphics->Leave();
  return TRUE;
}

GXBOOL GXCanvasImpl::SetSamplerState(GXUINT Sampler, GXSAMPLERDESC* pDesc)
{
  GXSAMPLERDESC* pDescStore = new GXSAMPLERDESC;
  *pDescStore = *pDesc;

  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();
  m_aBatch[m_uBatchCount].eFunc = CF_SetSamplerState;
  m_aBatch[m_uBatchCount].comm.dwFlag = Sampler;  // 反正多余, 都写成一样了
  m_aBatch[m_uBatchCount].comm.wParam = Sampler;
  m_aBatch[m_uBatchCount].comm.lParam = (GXLPARAM)pDescStore;
  m_uBatchCount++;

  return TRUE;
  //return GXSUCCEEDED(m_pGraphics->D3DGetDevice()->SetSamplerState(Sampler, Type, Value));
}

GXBOOL GXCanvasImpl::SetRenderState(GXRenderStateType eType, GXDWORD dwValue)
{
  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();
  // TODO: 想办法保证, 当GXCanvasImpl不在设备上时只改变GRenderState的变量.
  m_aBatch[m_uBatchCount++].SetRenderState(eType, dwValue);
  return TRUE;
}
GXBOOL GXCanvasImpl::SetRenderStateBlock(GXLPCRENDERSTATE lpBlock)
{
  ASSERT(0); // 已经去掉了
  return TRUE;
  //return m_pRenderState->SetBlock(lpBlock);
}

GXBOOL GXCanvasImpl::SetEffect(GXEffect* pEffect)
{
  if(m_LastState.pEffectImpl == pEffect)
  {
    return FALSE;
  }
  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();
  if(pEffect != NULL)
    pEffect->AddRef();
  m_aBatch[m_uBatchCount++].Set(CF_Effect, 0, 0, (GXLPARAM)pEffect);
  m_LastState.pEffectImpl = (GXEffectImpl*)pEffect;

  const GShaderImpl* pShaderImpl = (GShaderImpl*)m_LastState.pEffectImpl->GetShaderUnsafe();
  GXCONST GXINT nCacheSize = pShaderImpl->GetCacheSize();
  if((GXINT)m_UniformBuffer.GetSize() < nCacheSize)
  {
    m_UniformBuffer.Resize(nCacheSize, TRUE);
  }
  return TRUE;
}
#ifdef GLES2_CANVAS_IMPL
GXBOOL GXCanvasImpl::SetEffectConst(GXLPCSTR lpName, void* pData, int nPackCount)
{
  return FALSE;
}
#elif defined(D3D9_CANVAS_IMPL)
GXBOOL GXCanvasImpl::SetEffectConst(GXLPCSTR lpName, void* pData, int nPackCount)
{
  //GXEffectImpl* pEffect = (GEffectImpl*)m_LastState.pEffect;
  //if(pShader == NULL)
  //{
  //  ASSERT(0);
  //  return FALSE;
  //}
  //D3DXHANDLE hHandle = pShader->m_ppct->GetConstantByName(NULL, lpName);
  //if(hHandle == NULL)
  //{
  //  TRACE("Don't has constant register named %s\n.", lpName);
  //  return FALSE;
  //}
  //D3DXCONSTANT_DESC d3dcd;
  //GXUINT uCount;
  //if(FAILED(pShader->m_ppct->GetConstantDesc(hHandle, &d3dcd, &uCount)))
  //{
  //  ASSERT(0);
  //  return FALSE;
  //}
  //if(d3dcd.Type == D3DXPT_FLOAT)
  //{
  //  if(!((m_uBatchCount + 1) < m_uBatchSize))
  //    Flush();
  //  clBuffer* pBuffer = new clBuffer;
  //  pBuffer->Append(pData, nPackCount * sizeof(float4));
  //  m_aBatch[m_uBatchCount].Set(CF_SetPixelShaderConst, 0, 0, (GXLPARAM)pBuffer);
  //  m_aBatch[m_uBatchCount].comm.wParam = d3dcd.RegisterIndex;
  //  m_uBatchCount++;
  //  return TRUE;
  //}
  //else
  //{
  //  ASSERT(0);
    return FALSE;
  //}
}
#elif defined(D3D11_CANVAS_IMPL)
GXBOOL GXCanvasImpl::SetEffectConst(GXLPCSTR lpName, void* pData, int nPackCount)
{
  return FALSE;
}
#else
#error 需要定义inl的环境
#endif

GXDWORD GXCanvasImpl::SetParametersInfo(CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam)
{
  GXDWORD dwRet = NULL;

  ASSERT(sizeof(uParam) >= sizeof(GXDWORD));

  switch(eAction)
  {
  case CPI_SETTEXTURECOLOR:
    {
      dwRet = m_dwTexVertColor;
      m_dwTexVertColor = (GXDWORD)uParam;
    }
    break;
  case CPI_SETCOLORADDITIVE:
    {
      dwRet = m_LastState.dwColorAdditive;
      m_LastState.dwColorAdditive = (GXDWORD)uParam;

      // 检测是否是新的叠加颜色
      if(m_LastState.dwColorAdditive != dwRet)
      {
        // 如果上一个命令也是设置叠加颜色
        if( m_uBatchCount > 0 && 
          m_aBatch[m_uBatchCount - 1].eFunc == CF_ColorAdditive &&
          m_aBatch[m_uBatchCount - 1].comm.lParam == (GXDWORD)uParam)
        {
          m_uBatchCount--;
          dwRet = (GXDWORD)uParam;
        }
        else
        {
          if(!((m_uBatchCount + 1) < m_uBatchSize))
            Flush();
          m_aBatch[m_uBatchCount++].Set(CF_ColorAdditive, 0, dwRet, m_LastState.dwColorAdditive);
        }
      }
    }
    break;
  case CPI_SETTEXTCLIP:
    {
      if(!((m_uBatchCount + 1) < m_uBatchSize))
        Flush();

      GXLPRECT lpRect = (GXLPRECT)pParam;
      // 这个函数里面将 RECT的left 和 right, top 和 bottom 压缩储存

      if(lpRect == NULL)  // 复位模式
      {
        m_aBatch[m_uBatchCount].eFunc       = CF_ResetTextClip;
        m_aBatch[m_uBatchCount].comm.dwFlag = NULL;
        m_aBatch[m_uBatchCount].comm.wParam = NULL;
        m_aBatch[m_uBatchCount].comm.lParam = NULL;
        m_uBatchCount++;
      }
      else  // 用户设置
      {
        // 转换为 RenderTarget 空间的坐标
        GXRECT rcUserClip = *lpRect;
        gxOffsetRect(&rcUserClip, m_LastState.xOrigin, m_LastState.yOrigin);

        // 与系统区域裁剪
        gxIntersectRect(&rcUserClip, &m_rcAbsClip, &rcUserClip);    

        m_aBatch[m_uBatchCount].eFunc       = CF_SetTextClip;
        m_aBatch[m_uBatchCount].comm.dwFlag = NULL;
        m_aBatch[m_uBatchCount].comm.wParam = GXMAKELONG(rcUserClip.left, rcUserClip.right);
        m_aBatch[m_uBatchCount].comm.lParam = GXMAKELONG(rcUserClip.top, rcUserClip.bottom);

        m_uBatchCount++;
      }
      return TRUE;
    }
    break;
//  case CPI_SETPIXELSIZEINV:
    break;
  case CPI_SETEXTTEXTURE:
    {
      if(uParam > 0 && uParam < GX_MAX_TEXTURE_STAGE)
      {
        if(!((m_uBatchCount + 1) < m_uBatchSize))
          Flush();

        m_aBatch[m_uBatchCount].eFunc       = CF_SetExtTexture;
        m_aBatch[m_uBatchCount].comm.dwFlag = NULL;
        m_aBatch[m_uBatchCount].comm.wParam = uParam;
        m_aBatch[m_uBatchCount].comm.lParam = (GXLPARAM)pParam;
        m_uBatchCount++;
        if(pParam != NULL)
          ((GTexture*)pParam)->AddRef();
      }
      else
        dwRet = 0;
      dwRet = uParam;
    }
    break;
  }
  return dwRet;
}

PenStyle GXCanvasImpl::SetPenStyle(PenStyle eStyle)
{
  if(eStyle < 0 || eStyle > PS_DashDotDot) {
    return m_eStyle;
  }

  PenStyle ePrevStyle = m_eStyle;
  m_eStyle = eStyle;
  return ePrevStyle;
}

GXBOOL GXCanvasImpl::Clear(GXCOLORREF crClear)
{
  if(m_uBatchCount > 0)
  {
    // 如果调用了Clear, 之前的所有绘图缓冲全部失效了
    for(GXINT i = m_uBatchCount - 1; i >= 0; i--)
    {
      if(m_aBatch[i].eFunc == CF_SetRegion)  // 设置裁剪区之前的绘图命令还是有效的,
        goto FLUSH_CMD;            // 因为新的裁剪区如果较小,之前的绘图操作会留在旧的裁剪区
      else if(m_aBatch[i].eFunc > CF_DrawFirst && m_aBatch[i].eFunc < CF_DrawLast)
        m_aBatch[i].eFunc = CF_NoOperation;
    }

    // 清空顶点和索引
    m_uVertCount  = 0;
    m_uIndexCount = 0;
  }
FLUSH_CMD:
  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();

  // 如果上一个命令有清除功能,则直接返回!
  if(m_aBatch[m_uBatchCount - 1].eFunc == CF_Clear)
  {
    if((m_aBatch[m_uBatchCount - 1].comm.dwFlag & GXCLEAR_TARGET) != 0)
      return TRUE;
    else
      m_aBatch[m_uBatchCount - 1].comm.dwFlag |= GXCLEAR_TARGET;
    return TRUE;
  }

  m_aBatch[m_uBatchCount].eFunc       = CF_Clear;
  m_aBatch[m_uBatchCount].comm.dwFlag = GXCLEAR_TARGET;
  m_aBatch[m_uBatchCount].comm.wParam = crClear;
  m_aBatch[m_uBatchCount].comm.lParam = NULL;
  m_uBatchCount++;
  return TRUE;
}

GXINT GXCanvasImpl::SetCompositingMode(CompositingMode eMode)
{
  if(m_LastState.eCompMode == eMode)
    return eMode;
  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();

  // 如果与上一条命令相同则冲销掉
  // 如果两个CF_CompositingMode命令间全部是非绘图命令则不能冲销
  if(m_uBatchCount > 0 && m_aBatch[m_uBatchCount - 1].eFunc == CF_CompositingMode)
    m_aBatch[m_uBatchCount - 1].Set(CF_CompositingMode, 0, 0, eMode);
  else
    m_aBatch[m_uBatchCount++].Set(CF_CompositingMode, 0, 0, eMode);
  CompositingMode ePrevCompMode = m_LastState.eCompMode;
  m_LastState.eCompMode = eMode;
  return (GXINT)ePrevCompMode;
}

GXBOOL GXCanvasImpl::SetRegion(GRegion* pRegion, GXBOOL bAbsOrigin)
{
  GRegion* pSurfaceRegion = NULL;

  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();
  if(pRegion != NULL)
  {
    // TODO: 测试如果区域小于屏幕区,就不用这个屏幕区的Region裁剪
    m_pGraphics->CreateRectRgn(&pSurfaceRegion, 0, 0, m_xExt, m_yExt);

    if(bAbsOrigin == TRUE)
      pSurfaceRegion->Intersect(pRegion);
    else
    {
      GRegion* pAbsRegion = pRegion->Clone();
      pAbsRegion->Offset(m_LastState.xOrigin, m_LastState.yOrigin);
      pSurfaceRegion->Intersect(pAbsRegion);
      SAFE_RELEASE(pAbsRegion);
    }
  }

  m_aBatch[m_uBatchCount].eFunc       = CF_SetRegion;
  m_aBatch[m_uBatchCount].comm.dwFlag = NULL;
  m_aBatch[m_uBatchCount].comm.wParam = NULL;
  m_aBatch[m_uBatchCount].comm.lParam = (GXLPARAM)pSurfaceRegion;
  m_uBatchCount++;

  return TRUE;
}

GXBOOL GXCanvasImpl::SetClipBox(const GXLPRECT lpRect)
{
  if(!((m_uBatchCount + 1) < m_uBatchSize))
    Flush();

  // 这个函数里面将 RECT的left 和 right, top 和 bottom 压缩储存

  if(lpRect == NULL)  // 复位模式
  {
    m_aBatch[m_uBatchCount].eFunc       = CF_ResetClipBox;
    m_aBatch[m_uBatchCount].comm.dwFlag = NULL;
    m_aBatch[m_uBatchCount].comm.wParam = NULL;
    m_aBatch[m_uBatchCount].comm.lParam = NULL;
    m_uBatchCount++;

    m_LastState.rcClip = m_rcAbsClip;
  }
  else  // 用户设置
  {
    // 转换为 RenderTarget 空间的坐标
    GXRECT rcUserClip = *lpRect;
    gxOffsetRect(&rcUserClip, m_LastState.xOrigin, m_LastState.yOrigin);

    // 与系统区域裁剪
    gxIntersectRect(&m_LastState.rcClip, &m_rcAbsClip, &rcUserClip);    

    m_aBatch[m_uBatchCount].eFunc       = CF_SetClipBox;
    m_aBatch[m_uBatchCount].comm.dwFlag = NULL;

    m_aBatch[m_uBatchCount].comm.wParam = GXMAKELONG(m_LastState.rcClip.left, m_LastState.rcClip.right);
    m_aBatch[m_uBatchCount].comm.lParam = GXMAKELONG(m_LastState.rcClip.top, m_LastState.rcClip.bottom);

    m_uBatchCount++;
  }
  return TRUE;
}

GXINT GXCanvasImpl::GetClipBox(GXLPRECT lpRect)
{
  if(lpRect != NULL)
  {
    *lpRect = m_LastState.rcClip;
    gxOffsetRect(lpRect, -m_xAbsOrigin, -m_yAbsOrigin);
  }
  return RC_SIMPLE;
}

GXDWORD GXCanvasImpl::GetStencilLevel()
{
  return m_dwStencil;
}

GXBOOL GXCanvasImpl::GetUniformData(CANVASUNIFORM* pCanvasUniform)
{
  pCanvasUniform->pCommon = GetCommonConst();
  pCanvasUniform->pUnusualBuf = &GetUniformBuffer();
  return TRUE;     
}

GXBOOL GXCanvasImpl::Scroll(int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate)
{
  Flush();

  SCROLLTEXTUREDESC ScrollTexDesc;
  GRegion* prgnClip;
  m_pGraphics->CreateRectRgn(&prgnClip, lprcClip->left, lprcClip->top, lprcClip->right, lprcClip->bottom);

  ScrollTexDesc.pOperationTex = m_pTargetTex;
  ScrollTexDesc.pTempTex    = NULL;
  ScrollTexDesc.dx      = dx;
  ScrollTexDesc.dy      = dy;
  ScrollTexDesc.lprcScroll  = lprcScroll;
  ScrollTexDesc.lprgnClip    = prgnClip;
  ScrollTexDesc.lpprgnUpdate  = lpprgnUpdate;
  ScrollTexDesc.lprcUpdate  = lprcUpdate;
  m_pGraphics->ScrollTexture(&ScrollTexDesc);
  SAFE_RELEASE(prgnClip);
  return TRUE;
}

#define CHECK_LOCK if(m_lpLockedVertex == NULL)  \
  m_pPrimitive->Lock(0, 0, 0, 0, (void**)&m_lpLockedVertex, &m_lpLockedIndex, GXLOCK_DISCARD);
#define SET_BATCH_INDEX(_OFFSET, _INDEX)  m_lpLockedIndex[m_uIndexCount + _OFFSET] = uBaseIndex + _INDEX

GXBOOL GXCanvasImpl::SetPixel(GXINT xPos, GXINT yPos, GXCOLORREF crPixel)
{
  //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

  GXUINT uBaseIndex = PrepareBatch(CF_Points, 1, 0, NULL);
  CHECK_LOCK;

  m_lpLockedVertex[m_uVertCount].z = 0;
  m_lpLockedVertex[m_uVertCount].w = 1;
  m_lpLockedVertex[m_uVertCount].u = 0;
  m_lpLockedVertex[m_uVertCount].v = 0;
  m_lpLockedVertex[m_uVertCount].color = (GXDWORD)crPixel;

  _SetPrimitivePos(m_uVertCount, xPos, yPos);

  m_uVertCount++;
  //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
  return TRUE;
}

GXBOOL GXCanvasImpl::DrawLine(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine)
{
  //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

  GXUINT uBaseIndex = PrepareBatch(CF_LineList, 2, 2, NULL);
  CHECK_LOCK;

  for(int i = 0; i < 2; i++)
  {
    m_lpLockedVertex[m_uVertCount + i].z = 0;
    m_lpLockedVertex[m_uVertCount + i].w = 1;
    m_lpLockedVertex[m_uVertCount + i].u = 0;
    m_lpLockedVertex[m_uVertCount + i].v = 0;
    m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)crLine;
  }

  _SetPrimitivePos(m_uVertCount,     left, top);
  _SetPrimitivePos(m_uVertCount + 1, right, bottom);

  SET_BATCH_INDEX(0,0);
  SET_BATCH_INDEX(1,1);

  m_uVertCount  += 2;
  m_uIndexCount += 2;

  //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GXCanvasImpl::InlDrawRectangle(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crRect)
{
  //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

  GXUINT uBaseIndex = PrepareBatch(CF_LineList, 4, 8, NULL);
  CHECK_LOCK;

  for(int i = 0; i < 8; i++)
  {
    m_lpLockedVertex[m_uVertCount + i].z = 0;
    m_lpLockedVertex[m_uVertCount + i].w = 1;
    m_lpLockedVertex[m_uVertCount + i].u = 0;
    m_lpLockedVertex[m_uVertCount + i].v = 0;
    m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)crRect;
  }

  _SetPrimitivePos(m_uVertCount,     left,      top);
  _SetPrimitivePos(m_uVertCount + 1, right - 1, top);
  _SetPrimitivePos(m_uVertCount + 2, right - 1, bottom - 1);
  _SetPrimitivePos(m_uVertCount + 3, left,      bottom - 1);

  SET_BATCH_INDEX(0, 0);
  SET_BATCH_INDEX(1, 1);
  SET_BATCH_INDEX(2, 1);
  SET_BATCH_INDEX(3, 2);
  SET_BATCH_INDEX(4, 2);
  SET_BATCH_INDEX(5, 3);
  SET_BATCH_INDEX(6, 3);
  SET_BATCH_INDEX(7, 0);

  m_uVertCount  += 4;
  m_uIndexCount += 8;

  //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
  return TRUE;
}

GXBOOL GXCanvasImpl::DrawRectangle(GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect)
{
  return InlDrawRectangle(x, y, x + w, y + h, crRect);
}

GXBOOL GXCanvasImpl::DrawRectangle(GXLPCRECT lprc, GXCOLORREF crRect)
{
  return InlDrawRectangle(lprc->left, lprc->top, lprc->right, lprc->bottom, crRect);
}

GXBOOL GXCanvasImpl::DrawRectangle(GXLPCREGN lprg, GXCOLORREF crRect)
{
  return InlDrawRectangle(lprg->left, lprg->top, lprg->left + lprg->width, lprg->top + lprg->height, crRect);
}

//////////////////////////////////////////////////////////////////////////
GXBOOL GXCanvasImpl::InlFillRectangle(GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crFill)
{
  //GXDWORD dwPrevClrAdd = SetParametersInfo(CPI_SETCOLORADDITIVE, 0xffffff, NULL);

  GXUINT uBaseIndex = PrepareBatch(CF_Triangle, 4, 6, NULL);
  CHECK_LOCK;

  for(int i = 0; i < 4; i++)
  {
    m_lpLockedVertex[m_uVertCount + i].z = 0;
    m_lpLockedVertex[m_uVertCount + i].w = 1;
    m_lpLockedVertex[m_uVertCount + i].u = 0;
    m_lpLockedVertex[m_uVertCount + i].v = 0;
    m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)crFill;
  }

  _SetPrimitivePos(m_uVertCount + 0, left,  top   );
  _SetPrimitivePos(m_uVertCount + 1, right, top   );
  _SetPrimitivePos(m_uVertCount + 2, right, bottom);
  _SetPrimitivePos(m_uVertCount + 3, left,  bottom);

  SET_BATCH_INDEX(0, 0);
  SET_BATCH_INDEX(1, 1);
  SET_BATCH_INDEX(2, 3);
  SET_BATCH_INDEX(3, 3);
  SET_BATCH_INDEX(4, 1);
  SET_BATCH_INDEX(5, 2);

  m_uVertCount  += 4;
  m_uIndexCount += 6;

  //SetParametersInfo(CPI_SETCOLORADDITIVE, dwPrevClrAdd, NULL);
  return TRUE;
}

GXBOOL GXCanvasImpl::FillRectangle(GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill)
{
  return InlFillRectangle(x, y, x + w, y + h, crFill);
}

GXBOOL GXCanvasImpl::FillRectangle(GXLPCRECT lprc, GXCOLORREF crFill)
{
  return InlFillRectangle(lprc->left, lprc->top, lprc->right, lprc->bottom, crFill);
}

GXBOOL GXCanvasImpl::FillRectangle(GXLPCREGN lprg, GXCOLORREF crFill)
{
  return InlFillRectangle(lprg->left, lprg->top, lprg->left + lprg->width, lprg->top + lprg->height, crFill);
}

//////////////////////////////////////////////////////////////////////////

GXBOOL GXCanvasImpl::InvertRect(GXINT x, GXINT y, GXINT w, GXINT h)
{
  return TRUE;
}

GXBOOL GXCanvasImpl::ColorFillRegion(GRegion* pRegion, GXCOLORREF crFill)
{
  if(pRegion == NULL)
    return FALSE;
  int nCount = pRegion->GetRectCount();
  GXRECT* pRects = _GlbLockStaticRects(nCount);
  pRegion->GetRects(pRects, nCount);

  for(int i = 0; i < nCount; i++)
  {
    FillRectangle(pRects[i].left, pRects[i].top, 
      pRects[i].right - pRects[i].left,
      pRects[i].bottom - pRects[i].top, crFill);
  }
  _GlbUnlockStaticRects(pRects);
  return TRUE;
}
GXBOOL GXCanvasImpl::DrawUserPrimitive(GTexture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount)
{
  if(m_uVertIndexSize < uVertCount || m_uVertIndexSize < uIdxCount)
    return FALSE;

  GXUINT uBaseIndex = PrepareBatch(CF_Textured, uVertCount, uIdxCount, (GXLPARAM)pTexture);
  CHECK_LOCK;

  //memcpy(m_lpLockedVertex + m_uVertCount, lpVertices, uVertCount * sizeof(PRIMITIVE));
  PRIMITIVE* pVertex = m_lpLockedVertex + m_uVertCount;
  for(GXUINT i = 0; i < uVertCount; i++, pVertex++)
  {
    *pVertex = *((PRIMITIVE*)lpVertices + i);
    pVertex->x += (GXFLOAT)m_LastState.xOrigin;
    pVertex->y += (GXFLOAT)m_LastState.yOrigin;
  }

  for(GXUINT i = 0; i < uIdxCount; i++) {
    m_lpLockedIndex[m_uIndexCount + i] = uBaseIndex + pIndices[i];
  }

  m_uVertCount  += uVertCount;
  m_uIndexCount += uIdxCount;

  return TRUE;
}
GXBOOL GXCanvasImpl::DrawTexture(GTexture*pTexture, const GXREGN *rcDest)
{
  if(pTexture == NULL) {
    return FALSE;
  }
  GXUINT uBaseIndex = PrepareBatch(CF_Textured, 4, 6, (GXLPARAM)pTexture);
  CHECK_LOCK;

  for(int i = 0; i < 4; i++)
  {
    m_lpLockedVertex[m_uVertCount + i].z = 0;
    m_lpLockedVertex[m_uVertCount + i].w = 1;
    m_lpLockedVertex[m_uVertCount + i].color = (GXDWORD)m_dwTexVertColor;
  }
#ifdef GLES2_CANVAS_IMPL
  m_lpLockedVertex[m_uVertCount    ].SetTexcoord(0, 1);
  m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1, 1);
  m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1, 0);
  m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0, 0);
#elif defined(D3D9_CANVAS_IMPL)
  GXUINT nTexWidth;
  GXUINT nTexHeight;
  pTexture->GetDimension(&nTexWidth, &nTexHeight);

  const GXFLOAT fHalfTexelKernelX = 0.5f / (GXFLOAT)nTexWidth; 
  const GXFLOAT fHalfTexelKernelY = 0.5f / (GXFLOAT)nTexHeight; 

  m_lpLockedVertex[m_uVertCount    ].SetTexcoord(0 + fHalfTexelKernelX, 0 + fHalfTexelKernelY);
  m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1 + fHalfTexelKernelX, 0 + fHalfTexelKernelY);
  m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1 + fHalfTexelKernelX, 1 + fHalfTexelKernelY);
  m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0 + fHalfTexelKernelX, 1 + fHalfTexelKernelY);
#elif defined(D3D11_CANVAS_IMPL)
  m_lpLockedVertex[m_uVertCount    ].SetTexcoord(0, 0);
  m_lpLockedVertex[m_uVertCount + 1].SetTexcoord(1, 0);
  m_lpLockedVertex[m_uVertCount + 2].SetTexcoord(1, 1);
  m_lpLockedVertex[m_uVertCount + 3].SetTexcoord(0, 1);
#else
#error 需要定义inl的环境
#endif

  _SetPrimitivePos(m_uVertCount + 0, rcDest->left,                 rcDest->top);
  _SetPrimitivePos(m_uVertCount + 1, rcDest->left + rcDest->width, rcDest->top);
  _SetPrimitivePos(m_uVertCount + 2, rcDest->left + rcDest->width, rcDest->top + rcDest->height);
  _SetPrimitivePos(m_uVertCount + 3, rcDest->left,                 rcDest->top + rcDest->height);

  SET_BATCH_INDEX(0, 0);
  SET_BATCH_INDEX(1, 1);
  SET_BATCH_INDEX(2, 3);
  SET_BATCH_INDEX(3, 3);
  SET_BATCH_INDEX(4, 1);
  SET_BATCH_INDEX(5, 2);

  m_uVertCount  += 4;
  m_uIndexCount += 6;
  return TRUE;
}

GXBOOL GXCanvasImpl::DrawTexture(GTexture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc)
{
#include "GXCanvasImpl_DrawTexture.inl"

  const GXUINT uWidth  = rcSrc->width;
  const GXUINT uHeight = rcSrc->height;

  _SetPrimitivePos(m_uVertCount + 0, xPos,          yPos);
  _SetPrimitivePos(m_uVertCount + 1, xPos + uWidth, yPos);
  _SetPrimitivePos(m_uVertCount + 2, xPos + uWidth, yPos + uHeight);
  _SetPrimitivePos(m_uVertCount + 3, xPos,          yPos + uHeight);

  SET_BATCH_INDEX(0, 0);
  SET_BATCH_INDEX(1, 1);
  SET_BATCH_INDEX(2, 3);
  SET_BATCH_INDEX(3, 3);
  SET_BATCH_INDEX(4, 1);
  SET_BATCH_INDEX(5, 2);

  m_uVertCount  += 4;
  m_uIndexCount += 6;
  return TRUE;
}

GXBOOL GXCanvasImpl::DrawTexture(GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc)
{
#include "GXCanvasImpl_DrawTexture.inl"

  // 0---1
  // | / |
  // 3---2

  const GXUINT uDestRight  = (rcDest->left + rcDest->width);
  const GXUINT uDestBottom = (rcDest->top  + rcDest->height);

  _SetPrimitivePos(m_uVertCount + 0, rcDest->left, rcDest->top);
  _SetPrimitivePos(m_uVertCount + 1, uDestRight,   rcDest->top);
  _SetPrimitivePos(m_uVertCount + 2, uDestRight,   uDestBottom);
  _SetPrimitivePos(m_uVertCount + 3, rcDest->left, uDestBottom);

  SET_BATCH_INDEX(0, 0);
  SET_BATCH_INDEX(1, 1);
  SET_BATCH_INDEX(2, 3);
  SET_BATCH_INDEX(3, 3);
  SET_BATCH_INDEX(4, 1);
  SET_BATCH_INDEX(5, 2);

  m_uVertCount  += 4;
  m_uIndexCount += 6;

  return TRUE;
}

//
// GXCanvasImpl::DrawTexture 根据定义做了特殊优化，不能随便修改
// 不可以修改这个定义顺序，不可以随便添加定义，小心出错！
STATIC_ASSERT(Rotate_None           == 0);
STATIC_ASSERT(Rotate_CW90           == 1);
STATIC_ASSERT(Rotate_180            == 2);
STATIC_ASSERT(Rotate_CCW90          == 3);
STATIC_ASSERT(Rotate_FlipHorizontal == 4);
STATIC_ASSERT(Rotate_CW90_Flip      == 5);
STATIC_ASSERT(Rotate_180_Flip       == 6);
STATIC_ASSERT(Rotate_CCW90_Flip     == 7);

GXBOOL GXCanvasImpl::DrawTexture(GTexture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation)
{
#include "GXCanvasImpl_DrawTexture.inl"

  // 0---1
  // | / |
  // 3---2

  const GXUINT uDestRight  = (rcDest->left + rcDest->width);
  const GXUINT uDestBottom = (rcDest->top  + rcDest->height);
  const GXPOINT aPos[] = {
    {rcDest->left, rcDest->top},
    {uDestRight,   rcDest->top},
    {uDestRight,   uDestBottom},
    {rcDest->left, uDestBottom},};

  if(eRotation > Rotate_CCW90_Flip) {
    eRotation = Rotate_None;
  }

  if(eRotation < Rotate_FlipHorizontal) {
    int i = (int)eRotation;
    _SetPrimitivePos(m_uVertCount + 0, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
    _SetPrimitivePos(m_uVertCount + 1, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
    _SetPrimitivePos(m_uVertCount + 2, aPos[i].x, aPos[i].y);   i = (i + 1) & 3;
    _SetPrimitivePos(m_uVertCount + 3, aPos[i].x, aPos[i].y);

    SET_BATCH_INDEX(0, 0);
    SET_BATCH_INDEX(1, 1);
    SET_BATCH_INDEX(2, 3);
    SET_BATCH_INDEX(3, 3);
    SET_BATCH_INDEX(4, 1);
    SET_BATCH_INDEX(5, 2);
  }
  else {
    int i = ((int)eRotation - Rotate_FlipHorizontal + 1) & 3;
    _SetPrimitivePos(m_uVertCount + 0, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
    _SetPrimitivePos(m_uVertCount + 1, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
    _SetPrimitivePos(m_uVertCount + 2, aPos[i].x, aPos[i].y);   i = (i - 1) & 3;
    _SetPrimitivePos(m_uVertCount + 3, aPos[i].x, aPos[i].y);

    SET_BATCH_INDEX(0, 0);
    SET_BATCH_INDEX(1, 3);
    SET_BATCH_INDEX(2, 1);
    SET_BATCH_INDEX(3, 3);
    SET_BATCH_INDEX(4, 2);
    SET_BATCH_INDEX(5, 1);
  }

  m_uVertCount  += 4;
  m_uIndexCount += 6;

  return TRUE;
}

GXBOOL GXCanvasImpl::DrawImage(GXImage*pImage, const GXREGN *rgDest)
{
  GXREGN rcSrc = {0,0};
  pImage->GetDimension((GXINT*)&rcSrc.width, (GXINT*)&rcSrc.height);
  return DrawTexture(pImage->GetTextureUnsafe(), rgDest, &rcSrc);
}

GXBOOL GXCanvasImpl::DrawImage(GXImage*pImage, GXINT xPos, GXINT yPos, const GXREGN *rgSrc)
{
  GXREGN regn;
  if(rgSrc == NULL)
  {
    regn.left = 0;
    regn.top = 0;
    pImage->GetDimension((GXINT*)&regn.width, (GXINT*)&regn.height);
    rgSrc = &regn;
  }
  return DrawTexture(pImage->GetTextureUnsafe(), xPos, yPos, rgSrc);
}

GXBOOL GXCanvasImpl::DrawImage(GXImage*pImage, const GXREGN *rgDest, const GXREGN *rgSrc)
{
  GXREGN regn;
  if(rgSrc == NULL)
  {
    regn.left = 0;
    regn.top = 0;
    pImage->GetDimension((GXINT*)&regn.width, (GXINT*)&regn.height);
    rgSrc = &regn;
  }
  return DrawTexture(pImage->GetTextureUnsafe(), rgDest, rgSrc);
}

GXBOOL GXCanvasImpl::DrawImage(GXImage*pImage, const GXREGN* rgDest, const GXREGN* rgSrc, RotateType eRotation)
{
  return DrawTexture(pImage->GetTextureUnsafe(), rgDest, rgSrc, eRotation);
}
//
//GXCONST GXCANVASCOMMCONST& GXCanvasImpl::GetCommonConst()
//{
//  return m_CanvasCommConst;
//}
//clBuffer& GXCanvasImpl::GetUniformBuffer()
//{
//  return m_UniformBuffer;
//}

#ifdef GLES2_CANVAS_IMPL
#elif defined(D3D9_CANVAS_IMPL)
#elif defined(D3D11_CANVAS_IMPL)
#else
#error 需要定义inl的环境
#endif
