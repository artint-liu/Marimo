#ifdef ENABLE_GRAPHICS_API_DX11
#ifndef _STATE_BLOCK_H_
#define _STATE_BLOCK_H_

//#include <Include/GUnknown.h>




#define RENDERSTATECOUNT  103    // 枚举项的总数量
#define LASTRENDERSTATEENUM  209    // 最后一个枚举值

#define SAMPLERCOUNT    16
#define SAMPSTATECOUNT    13
#define LASTSAMPSTSTEENUM  13

//#include <vector>
//using namespace std;

class Graphics;
namespace GrapX
{
  namespace D3D11
  {
    class GraphicsImpl;
    class SamplerStateImpl;
    struct DEVICECONTEXT;

    class RasterizerStateImpl : public RasterizerState
    {
    private:
      GraphicsImpl*           m_pGraphicsImpl;
      D3D11_RASTERIZER_DESC   m_RasterizerDesc;
      ID3D11RasterizerState*  m_pRasterizerState;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT AddRef            ();
      virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }

    public:
      RasterizerStateImpl(GraphicsImpl* pGraphicsImpl);
      GXBOOL  Initialize  (GXRASTERIZERDESC* pDesc);
      GXBOOL  Activate    (DEVICECONTEXT* pContext, GXUINT slot, RasterizerStateImpl* pPrevState);
      inline  void    InlSetRasterizerState(DEVICECONTEXT* pContext);
    };

    class BlendStateImpl : public BlendState
    {
    private:
      GraphicsImpl*       m_pGraphicsImpl;
      GXDWORD             m_BlendFactor;
      D3D11_BLEND_DESC    m_BlendDesc;
      ID3D11BlendState*   m_pBlendState;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT AddRef            ();
      virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }
      static D3D11_BLEND ColorBlendToAlphaBlend(GXBlend blend); // 从颜色合成枚举中获得alpha应该使用的合成枚举
    public:
      BlendStateImpl(GraphicsImpl* pGraphicsImpl);
      GXBOOL  Initialize  (GXBLENDDESC* pDesc, GXUINT nNum);
      GXBOOL  Activate    (DEVICECONTEXT* pContext, GXUINT slot, BlendStateImpl* pPrevState);

      GXDWORD SetBlendFactor  (CanvasCore* pCanvasCore, GXDWORD dwBlendFactor) override;

      inline  void    InlSetBlendState(ID3D11DeviceContext* pd3dContext);
    };
    //////////////////////////////////////////////////////////////////////////
    class DepthStencilStateImpl : public DepthStencilState
    {
    private:
      GraphicsImpl*       m_pGraphicsImpl;
      GXDWORD             m_StencilRef;

      D3D11_DEPTH_STENCIL_DESC  m_DepthStencilDesc;
      ID3D11DepthStencilState*  m_pDepthStencilState;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT AddRef            ();
      virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }


    public:
      DepthStencilStateImpl(GraphicsImpl* pGraphicsImpl);
      GXBOOL  Initialize  (GXDEPTHSTENCILDESC* pDesc);
      GXBOOL  Activate    (DEVICECONTEXT* pContext, GXUINT slot, DepthStencilStateImpl* pPrevState);

      GXDWORD SetStencilRef  (CanvasCore* pCanvasCore, GXDWORD dwStencilRef) override;
      GXDWORD SetStencilRef  (DEVICECONTEXT* pContext, GXDWORD dwStencilRef);
    };
    //////////////////////////////////////////////////////////////////////////
    class SamplerStateImpl : public SamplerState
    {
      friend class GraphicsImpl;
      friend class Canvas;

    private:
      GraphicsImpl*       m_pGraphicsImpl;
      ID3D11SamplerState*   m_pD3D11SamplerState;
      GXSAMPLERDESC         m_SamplerDesc;

    protected:
      SamplerStateImpl(Graphics* pGraphics);
      virtual ~SamplerStateImpl();
      GXBOOL            Initialize        (const GXSAMPLERDESC* pDesc);

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT AddRef    () override;
      GXHRESULT Release   () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT Invoke    (GRESCRIPTDESC* pDesc) override { return GX_OK; }

      GXHRESULT  GetDesc         (GXSAMPLERDESC* pSamplerDesc) override;
      GXBOOL     Activate        (DEVICECONTEXT* pContext, GXUINT slot, SamplerStateImpl* pPrevSamplerState);  // 这个只能被Graphics调用!

      static void InterfaceDescToD3D11Desc(D3D11_SAMPLER_DESC& D3D11Desc, const GXSAMPLERDESC& desc);
      static void D3D11DescToInterfaceDesc(GXSAMPLERDESC& desc, const D3D11_SAMPLER_DESC& D3D11Desc);
    };

  } // namespace D3D11
} // namespace GrapX

#endif // _STATE_BLOCK_H_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11