#ifndef _GRAPHX_STATE_BLOCK_H_
#define _GRAPHX_STATE_BLOCK_H_

namespace GrapX
{
  class CanvasCore;
  class RasterizerState : public GResource
  {
  public:
    RasterizerState() : GResource(0, ResourceType::RasterizerState) {}
  };

  class BlendState : public GResource
  {
  public:
    BlendState() : GResource(0, ResourceType::BlendState) {}
    GXSTDINTERFACE(GXDWORD SetBlendFactor  (CanvasCore* pCanvasCore, GXDWORD dwBlendFactor));
  };

  //////////////////////////////////////////////////////////////////////////
  class DepthStencilState : public GResource
  {
  public:
    DepthStencilState() : GResource(0, ResourceType::DepthStencilState) {}
    GXSTDINTERFACE(GXDWORD SetStencilRef  (CanvasCore* pCanvasCore, GXDWORD dwStencilRef));
  };

  //////////////////////////////////////////////////////////////////////////
  class SamplerState : public GResource
  {
  public:
    SamplerState() : GResource(0, ResourceType::SamplerState) {}
    GXSTDINTERFACE(GXHRESULT  AddRef          ());
    GXSTDINTERFACE(GXHRESULT  Release         ());

    GXSTDINTERFACE(GXHRESULT  Invoke          (GRESCRIPTDESC* pDesc));
    GXSTDINTERFACE(GXHRESULT  GetDesc         (GXSAMPLERDESC* pSamplerDesc));
  };
} // namespace GrapX

#endif // _GRAPHX_STATE_BLOCK_H_