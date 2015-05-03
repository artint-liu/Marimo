#ifndef _GRAPHX_STATE_BLOCK_H_
#define _GRAPHX_STATE_BLOCK_H_

class GRasterizerState : public GResource
{
public:
  GRasterizerState() : GResource(0, RESTYPE_RASTERIZER_STATE){}
};

class GBlendState : public GResource
{
public:
  GBlendState() : GResource(0, RESTYPE_BLEND_STATE){}
  GXSTDINTERFACE(GXDWORD SetBlendFactor  (GXDWORD dwBlendFactor));
};

//////////////////////////////////////////////////////////////////////////
class GDepthStencilState : public GResource
{
public:
  GDepthStencilState() : GResource(0, RESTYPE_DEPTHSTENCIL_STATE){}
  GXSTDINTERFACE(GXDWORD SetStencilRef  (GXDWORD dwStencilRef));
};

//////////////////////////////////////////////////////////////////////////
class GSamplerState : public GResource
{
public:
  GSamplerState() : GResource(0, RESTYPE_SAMPLER_STATE){}
  GXSTDINTERFACE(GXHRESULT  AddRef          ());
  GXSTDINTERFACE(GXHRESULT  Release         ());

  GXSTDINTERFACE(GXHRESULT  Invoke          (GRESCRIPTDESC* pDesc));
  GXSTDINTERFACE(GXHRESULT  SetState        (GXUINT nSamplerSlot, GXSAMPLERDESC* pSamplerDesc));
  GXSTDINTERFACE(GXHRESULT  SetStateArray   (GXUINT nStartSlot, GXSAMPLERDESC* pSamplerDesc, int nCount));
  GXSTDINTERFACE(GXHRESULT  ResetToDefault  ());
};

#endif // _GRAPHX_STATE_BLOCK_H_