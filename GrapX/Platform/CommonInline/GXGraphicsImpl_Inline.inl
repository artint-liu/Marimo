//#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef _GXGRAPHICS_INLINE_
#error Duplicate include this file
#else
#define _GXGRAPHICS_INLINE_
//namespace D3D9
//{
class GPrimitiveVertexOnlyImpl;
class GPrimitiveVertexIndexImpl;
#ifdef _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D9_
inline GXBOOL GXGraphicsImpl::InlSetDepthStencil(GTexture* pTexture)
{
  SAFE_RELEASE(m_pCurDepthStencil);
  m_pCurDepthStencil = static_cast<GTextureImpl*>(pTexture);
  if(m_pCurDepthStencil != NULL)
  {
    m_pCurDepthStencil->AddRef();
    return GXSUCCEEDED(m_pd3dDevice->SetDepthStencilSurface(m_pCurDepthStencil->D3DSurface()));
  }
  else {
    return GXSUCCEEDED(m_pd3dDevice->SetDepthStencilSurface(m_pD3DDepthStencilSur));
  }
  return TRUE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D9_

#ifdef _GXGRAPHICS_INLINE_RENDERTARGET_D3D9_
GXBOOL GXGraphicsImpl::InlSetRenderTarget(GTexture* pTexture, GXDWORD uRenderTargetIndex)
{
  if(m_pCurRenderTarget != NULL)
  {
    if(TEST_FLAG(m_dwFlags, F_HASDRAWCALL)) {
      IntCheckRTTexture(m_pCurRenderTarget);
    }
    m_pCurRenderTarget->Release();
  }
  m_pCurRenderTarget = static_cast<GTextureImpl*>(pTexture);
  if(pTexture != NULL)
  {
    m_pCurRenderTarget->AddRef();

    return GXSUCCEEDED(m_pd3dDevice->SetRenderTarget(uRenderTargetIndex, 
      m_pCurRenderTarget->D3DSurface()));
  }
  else
    return GXSUCCEEDED(m_pd3dDevice->SetRenderTarget(uRenderTargetIndex, 
    m_pD3DOriginSur));
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_RENDERTARGET_

#ifdef _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D11_
inline GXBOOL GXGraphicsImpl::InlSetDepthStencil(GTexture* pTexture)
{
  //SAFE_RELEASE(m_pCurDepthStencil);
  //m_pCurDepthStencil = static_cast<GTextureImpl*>(pTexture);
  //if(m_pCurDepthStencil != NULL)
  //{
  //  m_pCurDepthStencil->AddRef();
  //  return GXSUCCEEDED(m_pd3dDevice->SetDepthStencilSurface(m_pCurDepthStencil->D3DSurface()));
  //}
  //else {
  //  return GXSUCCEEDED(m_pd3dDevice->SetDepthStencilSurface(m_pD3DDepthStencilSur));
  //}
  return TRUE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SETDEPTHSTENCIL_D3D11_

#ifdef _GXGRAPHICS_INLINE_RENDERTARGET_D3D11_
#endif // #ifdef _GXGRAPHICS_INLINE_RENDERTARGET_D3D11_

#if defined(_GXGRAPHICS_INLINE_CANVAS_D3D9_) || defined(_GXGRAPHICS_INLINE_CANVAS_D3D11_) || defined(_GXGRAPHICS_INLINE_CANVAS_GLES2_)
GXHRESULT GXGraphicsImpl::InlSetCanvas(GXCanvasCore *pCanvasCore)
{
  //GXCanvasImpl* pCanvas = (GXCanvasImpl*)pICanvas;
  if(m_pCurCanvasCore == (GXCanvasCore*)pCanvasCore)
    return GX_OK;

  // 释放上一个对象
  SAFE_RELEASE(m_pCurCanvasCore);

  // 引用这个对象
  GXHRESULT hr = 0;
  m_pCurCanvasCore = (GXCanvasCore*)pCanvasCore;
  if(pCanvasCore != NULL)
  {
    ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE) != 0);
    hr = m_pCurCanvasCore->AddRef();
    GTexture* pTarget = pCanvasCore->GetTargetUnsafe();
    if(m_pCurRenderTarget != pTarget) {
      InlSetRenderTarget(pTarget, 0);
    }
  }
  else {
    InlSetRenderTarget(NULL, 0);
  }

  return hr;
}
#endif // #if defined(_GXGRAPHICS_INLINE_CANVAS_D3D9_) || defined(_GXGRAPHICS_INLINE_CANVAS_D3D11_) || defined(_GXGRAPHICS_INLINE_CANVAS_GLES2_)

#ifdef _GXGRAPHICS_INLINE_TEXTURE_D3D9_
GXBOOL GXGraphicsImpl::InlSetTexture(GTexBaseImpl* pTexture, GXUINT uStage)
{
#ifdef _DEBUG
  if(uStage >= MAX_TEXTURE_STAGE) {
    TRACE("Error: Stage out of range.\n");
    return FALSE;
  }
#endif // #ifdef _DEBUG

  if(pTexture == m_pCurTexture[uStage])
    return TRUE;

  SAFE_RELEASE(m_pCurTexture[uStage]);
  m_pCurTexture[uStage] = pTexture;

  if(m_pCurTexture[uStage] == NULL)
  {
    m_pd3dDevice->SetTexture(uStage, NULL);
    return TRUE;
  }
  m_pCurTexture[uStage]->AddRef();
  m_pd3dDevice->SetTexture(uStage, pTexture->D3DTexture());
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_TEXTURE_

#ifdef _GXGRAPHICS_INLINE_TEXTURE_D3D11_
GXBOOL GXGraphicsImpl::InlSetTexture(GTexBaseImpl* pTexture, GXUINT uStage)
{
#ifdef _DEBUG
  if(uStage >= MAX_TEXTURE_STAGE) {
    TRACE("Error: Stage out of range.\n");
    return FALSE;
  }
#endif // #ifdef _DEBUG

  if(pTexture == m_pCurTexture[uStage])
    return TRUE;

  SAFE_RELEASE(m_pCurTexture[uStage]);
  m_pCurTexture[uStage] = pTexture;

  if(m_pCurTexture[uStage] == NULL)
  {
    //m_pImmediateContext->PSSetShaderResources(uStage, 1, );
    //m_pd3dDevice->SetTexture(uStage, NULL);
    return TRUE;
  }
  m_pCurTexture[uStage]->AddRef();
  //m_pd3dDevice->SetTexture(uStage, pTexture->D3DTexture());
  m_pImmediateContext->PSSetShaderResources(uStage, 1, &pTexture->D3DResourceView());
  return TRUE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_TEXTURE_D3D11_

#ifdef _GXGRAPHICS_INLINE_SHADER_D3D9_
GXBOOL GXGraphicsImpl::InlSetShader(GShader* pShader)
{
  if(m_pCurShader == pShader)
  {
#ifdef D3D9_LOW_DEBUG
    if(m_pCurShader != NULL)
    {
      LPDIRECT3DPIXELSHADER9 pPixelShader;
      LPDIRECT3DVERTEXSHADER9 pVertexShader;
      m_pd3dDevice->GetPixelShader(&pPixelShader);
      m_pd3dDevice->GetVertexShader(&pVertexShader);
      ASSERT(pPixelShader == ((GShaderImpl*)m_pCurShader)->m_pPixelShader && pVertexShader == ((GShaderImpl*)m_pCurShader)->m_pVertexShader);
      SAFE_RELEASE(pPixelShader);
      SAFE_RELEASE(pVertexShader);
    }
#endif // D3D9_LOW_DEBUG
    return TRUE;
  }

  SAFE_RELEASE(m_pCurShader);
  m_pCurShader = pShader;
  m_sStatistics.nShaderSwitch++;

  if(m_pCurShader != NULL)
  {
    m_pCurShader->AddRef();
    ((GShaderImpl*)m_pCurShader)->Activate();
  }
  else
  {
    m_pd3dDevice->SetPixelShader(NULL);
    m_pd3dDevice->SetVertexShader(NULL);
  }    
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_SHADER_D3D9_

#ifdef _GXGRAPHICS_INLINE_EFFECT_D3D9_
GXBOOL GXGraphicsImpl::InlSetEffect(GXEffectImpl* pEffectImpl)
{
  return InlSetShader(pEffectImpl->GetShaderUnsafe());
}
#endif // _GXGRAPHICS_INLINE_EFFECT_D3D9_

#ifdef _GXGRAPHICS_INLINE_SHADER_D3D11_
GXBOOL GXGraphicsImpl::InlSetShader(GShader* pShader)
{
  if(m_pCurShader == pShader)
  {
#ifdef D3D11_LOW_DEBUG
    //if(m_pCurShader != NULL)
    //{
    //  LPDIRECT3DPIXELSHADER9 pPixelShader;
    //  LPDIRECT3DVERTEXSHADER9 pVertexShader;
    //  m_pd3dDevice->GetPixelShader(&pPixelShader);
    //  m_pd3dDevice->GetVertexShader(&pVertexShader);
    //  ASSERT(pPixelShader == ((GShaderImpl*)m_pCurShader)->m_pPixelShader && pVertexShader == ((GShaderImpl*)m_pCurShader)->m_pVertexShader);
    //  SAFE_RELEASE(pPixelShader);
    //  SAFE_RELEASE(pVertexShader);
    //}
#endif // D3D9_LOW_DEBUG
    return TRUE;
  }
  ASSERT(pShader != NULL);
  SAFE_RELEASE(m_pCurShader);
  SAFE_RELEASE(m_pVertexLayout);
  m_pCurShader = pShader;

  if(m_pCurShader != NULL)
  {
    m_pCurShader->AddRef();
    ((GShaderImpl*)m_pCurShader)->Activate();
  }
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_SHADER_
#ifdef _GXGRAPHICS_INLINE_EFFECT_D3D11_
GXBOOL GXGraphicsImpl::InlSetEffect(GXEffectImpl* pEffectImpl)
{
  return InlSetShader(pEffectImpl->GetShaderUnsafe());
}
#endif // _GXGRAPHICS_INLINE_EFFECT_D3D9_

#ifdef _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_
GXHRESULT GXGraphicsImpl::InlSetVertexDecl(GVertexDeclImpl* pVertexDecl)
{
  if(m_pCurVertexDecl == pVertexDecl) {
    return GX_OK;
  }
  SAFE_RELEASE(m_pCurVertexDecl);
  m_pCurVertexDecl = pVertexDecl;
  if(m_pCurVertexDecl != NULL) {
    m_pCurVertexDecl->AddRef();
    return m_pCurVertexDecl->Activate();
  }
  return GX_OK;
}
#endif // _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D9_

#ifdef _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_
GXHRESULT GXGraphicsImpl::InlSetVertexDecl(GVertexDeclImpl* pVertexDecl)
{
  if(m_pCurVertexDecl == pVertexDecl) {
    return GX_OK;
  }

  SAFE_RELEASE(m_pCurVertexDecl);
  SAFE_RELEASE(m_pVertexLayout);

  m_pCurVertexDecl = pVertexDecl;
  if(m_pCurVertexDecl != NULL) {
    m_pCurVertexDecl->AddRef();
    //return m_pCurVertexDecl->Activate();
  }
  return GX_OK;
}
#endif // _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D1_
//} // namespace D3D9

//#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//namespace GLES2
//{
class GPrimitiveVertexOnlyImpl;
class GPrimitiveVertexIndexImpl;



#ifdef _GXGRAPHICS_INLINE_TEXTURE_GLES2_
GXBOOL GXGraphicsImpl::InlSetTexture(GTextureImpl* pTexture, GXUINT uStage)
{
  if(pTexture == m_pCurTexture[uStage])
    return TRUE;

  SAFE_RELEASE(m_pCurTexture[uStage]);
  m_pCurTexture[uStage] = pTexture;

  if(m_pCurTexture[uStage] == NULL)
  {
    GLVERIFY(glActiveTexture(GL_TEXTURE0 + uStage));
    GLVERIFY(GLBindTexture(GL_TEXTURE_2D, 0));
    return TRUE;
  }
  m_pCurTexture[uStage]->AddRef();
  GLVERIFY(glActiveTexture(GL_TEXTURE0 + uStage));
  GLVERIFY(GLBindTexture(GL_TEXTURE_2D, pTexture->m_uTexture));
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_TEXTURE_

#ifdef _GXGRAPHICS_INLINE_SHADER_GLES2_
GXBOOL GXGraphicsImpl::InlSetShader(GShader* pShader)
{
  if(m_pCurShader == pShader)
    return TRUE;

  SAFE_RELEASE(m_pCurShader);
  m_pCurShader = pShader;

  if(m_pCurShader != NULL)
  {
    m_pCurShader->AddRef();
    ((GShaderImpl*)m_pCurShader)->ApplyToDevice();
  }
  else
  {
    //m_pd3dDevice->SetPixelShader(NULL);
    //m_pd3dDevice->SetVertexShader(NULL);
  }
  return TRUE;
}
#endif // _GXGRAPHICS_INLINE_SHADER_GLES2_
#ifdef _GXGRAPHICS_INLINE_EFFECT_GLES2_
GXBOOL GXGraphicsImpl::InlSetEffect(GXEffectImpl* pEffectImpl)
{
  GXBOOL bRet = InlSetShader(pEffectImpl->GetShaderUnsafe());
  if(bRet == false)
    return bRet;

  GShaderStubImpl*pShaderStubImp = (GShaderStubImpl*)pEffectImpl->GetShaderStubUnsafe();
  pShaderStubImp->UpdateSamplerLocation();
  return bRet;
}
#endif // _GXGRAPHICS_INLINE_EFFECT_GLES2_

#ifdef _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_GLES2_
GXHRESULT GXGraphicsImpl::InlSetVertexDecl(GVertexDeclImpl* pVertexDecl)
{
  if(m_pCurVertexDecl == pVertexDecl) {
    return GX_OK;
  }
  SAFE_RELEASE(m_pCurVertexDecl);
  m_pCurVertexDecl = pVertexDecl;
  return m_pCurVertexDecl->AddRef();
  //return m_pCurVertexDecl->Activate();
}
#endif // _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_GLES2_


//} // namespace GLES2


//#ifdef _GXGRAPHICS_INLINE_SET_RENDER_STATE_
//GXBOOL GXGraphicsImpl::InlSetRenderState(GRenderState* pRenderState)
//{
//  ASSERT(pRenderState != NULL);
//
//  if(pRenderState == m_pCurRenderState)
//    return TRUE;
//
//  if(pRenderState->Update(m_pCurRenderState) != FALSE)
//  {
//    SAFE_RELEASE(m_pCurRenderState);
//    m_pCurRenderState = pRenderState;
//    m_pCurRenderState->AddRef();
//    return TRUE;
//  }
//  return FALSE;
//}
//#endif // #ifdef _GXGRAPHICS_INLINE_SET_RENDER_STATE_
template<class _TState>
inline GXBOOL GXGraphicsImpl::InlSetStateT(_TState*& pCurState, _TState* pState)
{
  ASSERT(pState);
  if(pCurState == pState) {
    return TRUE;
  }
  _TState* pPrevState = pCurState;
  pCurState = pState;
  if(pCurState->Activate(pPrevState))
  {
    SAFE_RELEASE(pPrevState);
    pCurState->AddRef();
    return TRUE;
  }

  // 如果失败就换回来
  pCurState = pPrevState;
  return FALSE;
}

#ifdef _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_
inline GXBOOL GXGraphicsImpl::InlSetRasterizerState(GRasterizerStateImpl* pRasterizerState)
{
  return InlSetStateT<GRasterizerStateImpl>(m_pCurRasterizerState, pRasterizerState);
  //ASSERT(pRasterizerState);
  //if(m_pCurRasterizerState == pRasterizerState) {
  //  return TRUE;
  //}
  //GRasterizerStateImpl* pPrevRasterizerState = m_pCurRasterizerState;
  //m_pCurRasterizerState = pRasterizerState;
  //if(m_pCurRasterizerState->Activate(pPrevRasterizerState))
  //{
  //  SAFE_RELEASE(pPrevRasterizerState);
  //  m_pCurRasterizerState->AddRef();
  //  return TRUE;
  //}
  //m_pCurRasterizerState = pPrevRasterizerState;
  //return FALSE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_

#ifdef _GXGRAPHICS_INLINE_SET_BLEND_STATE_
// TODO: 这个可以和InlSetDepthStencilState合并为模板
GXBOOL GXGraphicsImpl::InlSetBlendState(GBlendStateImpl* pBlendState)
{
  return InlSetStateT(m_pCurBlendState, pBlendState);
  //ASSERT(pBlendState != NULL); // 不能为空

  //if(m_pCurBlendState == pBlendState) {
  //  return TRUE;
  //}

  //GBlendStateImpl* pPrevState = m_pCurBlendState;
  //m_pCurBlendState = pBlendState;

  //if(m_pCurBlendState->Activate(pPrevState))
  //{
  //  SAFE_RELEASE(pPrevState);
  //  m_pCurBlendState->AddRef();
  //  return TRUE;
  //}

  //// 如果失败就换回来
  //m_pCurBlendState = pPrevState;
  //return FALSE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SET_BLEND_STATE_

#ifdef _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_
inline GXBOOL GXGraphicsImpl::InlSetDepthStencilState (GDepthStencilStateImpl* pDepthStencilState)
{
  return InlSetStateT(m_pCurDepthStencilState, pDepthStencilState);
  //ASSERT(pDepthStencilState != NULL); // 不能为空

  //if(m_pCurDepthStencilState == pDepthStencilState) {
  //  return TRUE;
  //}

  //GDepthStencilStateImpl* pPrevState = m_pCurDepthStencilState;
  //m_pCurDepthStencilState = pDepthStencilState;

  //if(m_pCurDepthStencilState->Activate(pPrevState))
  //{
  //  SAFE_RELEASE(pPrevState);
  //  m_pCurDepthStencilState->AddRef();
  //  return TRUE;
  //}

  //// 如果失败就换回来
  //m_pCurDepthStencilState = pPrevState;
  //return FALSE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_


#ifdef _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_
GXBOOL GXGraphicsImpl::InlSetSamplerState(GSamplerStateImpl* pSamplerState)
{
  return InlSetStateT(m_pCurSamplerState, pSamplerState);
  //ASSERT(pSamplerState != NULL);

  //if(pSamplerState == m_pCurSamplerState)
  //  return TRUE;

  //// 放置新的 SamplerState 到 Graphics 上
  //GSamplerStateImpl* pPrevSamplerState = m_pCurSamplerState;
  //m_pCurSamplerState = pSamplerState;

  //if(pSamplerState->Activate(pPrevSamplerState) != FALSE)
  //{
  //  // 成功的话,释放之前的,对新的增加引用
  //  SAFE_RELEASE(pPrevSamplerState);
  //  m_pCurSamplerState->AddRef();
  //  return TRUE;
  //}

  //// 如果失败就换回来
  //m_pCurSamplerState = pPrevSamplerState;
  //return FALSE;
}
#endif // #ifdef _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_

inline GXBOOL GXGraphicsImpl::IsActiveCanvas(GXCanvasCore* pCanvasCore)
{
  return m_pCurCanvasCore == pCanvasCore;
}

inline GXBOOL GXGraphicsImpl::InlIsActiveSamplerState(GSamplerStateImpl* pSamplerState)
{
  return m_pCurSamplerState == pSamplerState;
}

inline Marimo::ShaderConstName* GXGraphicsImpl::InlGetShaderConstantNameObj()
{
  return m_pShaderConstName;
}

inline GXBOOL GXGraphicsImpl::InlIsActiveRasterizerState(GRasterizerStateImpl* pRasterizerState)
{
  return m_pCurRasterizerState == pRasterizerState;
}

inline GXBOOL GXGraphicsImpl::InlIsActiveBlendState(GBlendStateImpl* pBlendState)
{
  return m_pCurBlendState == pBlendState;
}

inline GXBOOL GXGraphicsImpl::InlIsActiveDepthStencilState(GDepthStencilStateImpl* pDepthStencilState)
{
  return m_pCurDepthStencilState == pDepthStencilState;
}

//inline GXBOOL GXGraphicsImpl::IsActiveRenderState(GRenderStateImpl* pRenderState)
//{
//  return m_pCurRenderState == pRenderState;
//}


#endif // _GXGRAPHICS_INLINE_
