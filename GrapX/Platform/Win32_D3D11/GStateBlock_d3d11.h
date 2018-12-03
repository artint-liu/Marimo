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
      GXBOOL  Activate    (RasterizerStateImpl* pPrevState);
      inline  void    InlSetRasterizerState();
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

    public:
      BlendStateImpl(GraphicsImpl* pGraphicsImpl);
      GXBOOL  Initialize  (GXBLENDDESC* pDesc, GXUINT nNum);
      GXBOOL  Activate    (BlendStateImpl* pPrevState);

      virtual GXDWORD SetBlendFactor  (GXDWORD dwBlendFactor);

      inline  void    InlSetBlendState();
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
      GXBOOL  Activate    (DepthStencilStateImpl* pPrevState);

      virtual GXDWORD SetStencilRef  (GXDWORD dwStencilRef);
    };
    //////////////////////////////////////////////////////////////////////////
    class SamplerStateImpl : public SamplerState
    {
      friend class GraphicsImpl;
      friend class Canvas;
    private:
      GraphicsImpl*       m_pGraphicsImpl;
      //GXSAMPLERSTAGE        m_SamplerStage[SAMPLERCOUNT];
      ID3D11SamplerState*   m_pSampler[SAMPLERCOUNT]; // TODO: 这个可以用池来减少实际创建的数量
      GXSAMPLERDESC         m_SamplerDesc[SAMPLERCOUNT];
      //DWORD                 m_dwChangeMask;
      //static GXSAMPLERSTAGE s_DefaultSamplerState;
    private:
      SamplerStateImpl(Graphics* pGraphics);
      virtual ~SamplerStateImpl();
      static GXBOOL     InitializeStatic  ();
      GXBOOL            Initialize        (SamplerStateImpl* pDefault);
      GXBOOL            Activate          (SamplerStateImpl* pPrevSamplerState);  // 这个只能被Graphics调用!
      //void              SetStageToDevice  (DWORD dwStage);
      //void              SetStateToDevice  (DWORD dwStage, GXSamplerStateType eType);
      //void              D3D11BuildSampler (GXUINT Sampler, GXSAMPLERDESC* pSamplerDesc);
    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT AddRef    ();
      virtual GXHRESULT Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT Invoke    (GRESCRIPTDESC* pDesc) { return GX_OK; }


      //GXBOOL            ResetToDefault    ();

      //ID3D11SamplerState** GetSamplers     ();

      //GXBOOL            Set               (GXDWORD Sampler, GXSamplerStateType eType, GXDWORD dwValue);
      //GXDWORD           Get               (GXDWORD Sampler, GXSamplerStateType eType);

      GXHRESULT         SetState          (GXUINT Sampler, const GXSAMPLERDESC* pSamplerDesc);
      GXHRESULT         SetStateArray     (GXUINT nStartSlot, const GXSAMPLERDESC* pSamplerDesc, int nCount);
      GXHRESULT         ResetToDefault    ();
    };

  } // namespace D3D11
} // namespace GrapX

#endif // _STATE_BLOCK_H_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11