template<
  class BlendStateTempl,
  class SamplerStateTempl,
  class RasterizerStateTempl,
  class DepthStencilStateTempl>
  struct DEVICECONTEXT_TEMPL
{
  BlendStateTempl*         pBlendState = NULL;
  SamplerStateTempl*       pSamplerState = NULL;
  RasterizerStateTempl*    pRasterizerState = NULL;
  DepthStencilStateTempl*  pDepthStencilState = NULL;

  inline GXBOOL InlIsActiveSamplerState(SamplerStateTempl* _pSamplerState)
  {
    return this->pSamplerState == _pSamplerState;
  }

  inline GXBOOL InlIsActiveRasterizerState(RasterizerStateTempl* _pRasterizerState)
  {
    return this->pRasterizerState == _pRasterizerState;
  }

  inline GXBOOL InlIsActiveBlendState(BlendStateTempl* _pBlendState)
  {
    return this->pBlendState == _pBlendState;
  }

  inline GXBOOL InlIsActiveDepthStencilState(DepthStencilStateTempl* _pDepthStencilState)
  {
    return this->pDepthStencilState == _pDepthStencilState;
  }

  template<class _TDrive, class _TStateObject>
  inline GXBOOL InlSetStateT(GXUINT slot, _TStateObject*& pCurState, _TStateObject* pState)
  {
    ASSERT(pState);
    if (pCurState == pState) {
      return TRUE;
    }
    _TStateObject* pPrevState = pCurState;
    pCurState = pState;
    if (pCurState->Activate(static_cast<_TDrive*>(this), slot, pPrevState)) // slot为了模板兼容，并不是所有对象都使用这个
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
  inline GXBOOL InlSetRasterizerState(RasterizerStateImpl* _pRasterizerState)
  {
    return InlSetStateT<DEVICECONTEXT>(0, this->pRasterizerState, _pRasterizerState);
  }
#endif // #ifdef _GXGRAPHICS_INLINE_SET_RASTERIZER_STATE_

#ifdef _GXGRAPHICS_INLINE_SET_BLEND_STATE_
  GXBOOL InlSetBlendState(BlendStateImpl* _pBlendState)
  {
    return InlSetStateT<DEVICECONTEXT>(0, this->pBlendState, _pBlendState);
  }
#endif // #ifdef _GXGRAPHICS_INLINE_SET_BLEND_STATE_

#ifdef _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_
  inline GXBOOL InlSetDepthStencilState(DepthStencilStateImpl* _pDepthStencilState)
  {
    return InlSetStateT<DEVICECONTEXT>(0, this->pDepthStencilState, _pDepthStencilState);
  }
#endif // #ifdef _GXGRAPHICS_INLINE_SET_DEPTHSTENCIL_STATE_

#ifdef _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_
  GXBOOL InlSetSamplerState(GXUINT slot, SamplerStateImpl* _pSamplerState)
  {
    return InlSetStateT<DEVICECONTEXT>(slot, this->pSamplerState, _pSamplerState);
  }
#endif // #ifdef _GXGRAPHICS_INLINE_SET_SAMPLER_STATE_

}; // struct DEVICECONTEXT_TEMPL