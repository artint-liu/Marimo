GXCanvasCoreImpl::GXCanvasCoreImpl(GXGraphicsImpl* pGraphics, GXUINT nPriority, GXDWORD dwType)
  : GXCanvas            (nPriority, dwType)
  , m_pGraphics         (pGraphics)
  , m_pTargetTex        (NULL)
  , m_xExt              (0)
  , m_yExt              (0)
  , m_pBlendState       (NULL)
  , m_pSamplerState     (NULL)
  , m_pDepthStencilState(NULL)
  , m_pEffectImpl       (NULL)
  , m_pCamera           (NULL)
{
  AddRef();
}

GXCanvasCoreImpl::~GXCanvasCoreImpl()
{
  SAFE_RELEASE(m_pTargetTex);
  SAFE_RELEASE(m_pBlendState);
  SAFE_RELEASE(m_pSamplerState);
  SAFE_RELEASE(m_pDepthStencilState);
  SAFE_RELEASE(m_pEffectImpl);
  SAFE_RELEASE(m_pCamera);
}

GXBOOL GXCanvasCoreImpl::Initialize(GTexture* pTexture)
{
  if((pTexture != NULL && (pTexture->GetUsage() & GXRU_TEX_RENDERTARGET)) ||
    pTexture == NULL)
  {
    ASSERT(m_pTargetTex == NULL);

    m_pTargetTex = (GTextureImpl*)pTexture;
    if(m_pTargetTex != NULL)
    {
      m_pTargetTex->AddRef();
      m_pTargetTex->GetDimension((GXUINT*)&m_xExt, (GXUINT*)&m_yExt);
    }
    else
    {
      GXGRAPHICSDEVICE_DESC GraphDeviceDesc;
      m_pGraphics->GetDesc(&GraphDeviceDesc);
      m_xExt = GraphDeviceDesc.BackBufferWidth;
      m_yExt = GraphDeviceDesc.BackBufferHeight;
    }

    if(m_pSamplerState == NULL) {
      m_pGraphics->IntCreateSamplerState(&m_pSamplerState);
    }
    else {
      m_pSamplerState->ResetToDefault();
    }

    m_pEffectImpl = (GXEffectImpl*)m_pGraphics->IntGetEffect();
    m_pEffectImpl->AddRef();

    return TRUE;
  }
  return FALSE;
}

GXHRESULT GXCanvasCoreImpl::Invoke(GRESCRIPTDESC* pDesc)
{
  return GX_OK;
}

GXVOID GXCanvasCoreImpl::GetTargetDimension(GXSIZE* pSize) GXCONST
{
  pSize->cx = m_xExt;
  pSize->cy = m_yExt;
}

GTexture* GXCanvasCoreImpl::GetTargetUnsafe() GXCONST
{
  return m_pTargetTex;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GXCanvasCoreImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
