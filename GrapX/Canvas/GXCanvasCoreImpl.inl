CanvasCoreImpl::CanvasCoreImpl(GraphicsImpl* pGraphics, GXUINT nPriority, GXDWORD dwType)
  : Canvas            (nPriority, dwType)
  , m_pGraphics         (pGraphics)
  , m_pTargetTex        (NULL)
  //, m_xExt              (0)
  //, m_yExt              (0)
  , m_pBlendStateImpl       (NULL)
  , m_pSamplerState     (NULL)
  , m_pDepthStencilState(NULL)
  //, m_pEffectImpl       (NULL)
  , m_pCamera           (NULL)
{
  InlSetZeroT(m_sExtent);
  AddRef();
}

CanvasCoreImpl::~CanvasCoreImpl()
{
  SAFE_RELEASE(m_pTargetTex);
  SAFE_RELEASE(m_pBlendStateImpl);
  SAFE_RELEASE(m_pSamplerState);
  SAFE_RELEASE(m_pDepthStencilState);
  //SAFE_RELEASE(m_pEffectImpl);
  SAFE_RELEASE(m_pCamera);
}

GXBOOL CanvasCoreImpl::Initialize(RenderTarget* pTarget)
{
  //if(pTarget == NULL)
  //{
  //  return FALSE;
  //}

  m_pTargetTex = static_cast<RenderTargetImpl*>(pTarget);
  if(m_pTargetTex != NULL)
  {
    m_pTargetTex->AddRef();
    m_pTargetTex->GetDimension(&m_sExtent);
  }
  else
  {
    GXGRAPHICSDEVICE_DESC GraphDeviceDesc;
    m_pGraphics->GetDesc(&GraphDeviceDesc);
    
    m_sExtent.cx = GraphDeviceDesc.BackBufferWidth;
    m_sExtent.cy = GraphDeviceDesc.BackBufferHeight;
  }

  SAFE_RELEASE(m_pSamplerState);
  m_pSamplerState = m_pGraphics->m_pDefaultSamplerState;
  m_pSamplerState->AddRef();

  return TRUE;
}

GXHRESULT CanvasCoreImpl::Invoke(GRESCRIPTDESC* pDesc)
{
  return GX_OK;
}

void CanvasCoreImpl::CommitState()
{
  m_pGraphics->InlSetBlendState(m_pBlendStateImpl);
}

GXSIZE* CanvasCoreImpl::GetTargetDimension(GXSIZE* pSize) const
{
  *pSize = m_sExtent;
  return pSize;
}

RenderTarget* CanvasCoreImpl::GetTargetUnsafe() const
{
  return m_pTargetTex;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT CanvasCoreImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
