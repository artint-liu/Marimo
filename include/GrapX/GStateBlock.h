#ifndef _GRAPHX_STATE_BLOCK_H_
#define _GRAPHX_STATE_BLOCK_H_

namespace GrapX
{
  class RasterizerState : public GResource
  {
  public:
    RasterizerState() : GResource(0, RESTYPE_RASTERIZER_STATE) {}
  };

  class BlendState : public GResource
  {
  public:
    BlendState() : GResource(0, RESTYPE_BLEND_STATE) {}
    GXSTDINTERFACE(GXDWORD SetBlendFactor  (GXDWORD dwBlendFactor));
  };

  //////////////////////////////////////////////////////////////////////////
  class DepthStencilState : public GResource
  {
  public:
    DepthStencilState() : GResource(0, RESTYPE_DEPTHSTENCIL_STATE) {}
    GXSTDINTERFACE(GXDWORD SetStencilRef  (GXDWORD dwStencilRef));
  };

  //////////////////////////////////////////////////////////////////////////
  class SamplerState : public GResource
  {
  public:
    SamplerState() : GResource(0, RESTYPE_SAMPLER_STATE) {}
    GXSTDINTERFACE(GXHRESULT  AddRef          ());
    GXSTDINTERFACE(GXHRESULT  Release         ());

    GXSTDINTERFACE(GXHRESULT  Invoke          (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(GXHRESULT  GetDesc         (GXSAMPLERDESC* pSamplerDesc));
  };
} // namespace GrapX

#endif // _GRAPHX_STATE_BLOCK_H_