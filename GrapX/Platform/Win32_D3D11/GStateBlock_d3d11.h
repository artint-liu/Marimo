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

class GXGraphics;

namespace D3D11
{
  class GXGraphicsImpl;
  class GSamplerStateImpl;
  //class GRenderState : public GUnknown
  //{
  //  friend class GXGraphicsImpl;
  //  friend class GXCanvasCoreImpl;
  //  friend class GXCanvas3DImpl;
  //private:
  //  struct GROUPMASK  // 分组掩码 
  //  {
  //    union{
  //      GXDWORD dw;
  //      GXWORD  w[2];
  //      GXBYTE  b[4];
  //    };
  //  };
  //private:
  //  GRenderState(GXGraphics* pGraphics);
  //  static GXBOOL     InitializeStatic  ();
  //  GXBOOL            Update            (GRenderState* pPrevState);
  //  GXBOOL            ResetToDefault    ();
  //public:
  //  virtual GXLRESULT AddRef            ();
  //  virtual GXLRESULT Release           ();

  //  GXBOOL            Set               (GXRenderStateType eType, GXDWORD dwValue);
  //  GXDWORD           Get               (GXRenderStateType eType);
  //  GXBOOL            SetBlock          (GXLPCRENDERSTATE lpBlock);

  //public:
  //  GXBOOL            IntCheckUpdate    ();

  //private:
  //  static GXINT                s_aEnumToIdx           [LASTRENDERSTATEENUM];
  //  static GXDWORD              s_aRenderStateValue    [RENDERSTATECOUNT];
  //  static GXRenderStateType    s_aRenderStateTypeList [RENDERSTATECOUNT + 1];
  //  GXGraphicsImpl*           m_pGraphicsImpl;

  //  D3D11_RASTERIZER_DESC     m_RasterizerDesc;
  //  ID3D11RasterizerState*    m_pRasterizerState;

  //  //GXColor                   m_BlendFactor;
  //  //D3D11_BLEND_DESC          m_BlendState;
  //  //ID3D11BlendState*         m_pBlendState;

  //  GROUPMASK                 m_aChanged[(RENDERSTATECOUNT + 31) / 32];
  //  GXDWORD                   m_aRenderStateValue[RENDERSTATECOUNT];
  //  //GXBOOL      m_bOnDevice;
  //};

  class GRasterizerStateImpl : public GRasterizerState
  {
  private:
    GXGraphicsImpl*         m_pGraphicsImpl;
    D3D11_RASTERIZER_DESC   m_RasterizerDesc;
    ID3D11RasterizerState*  m_pRasterizerState;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef            ();
    virtual GXHRESULT Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }

  public:
    GRasterizerStateImpl(GXGraphicsImpl* pGraphicsImpl);
    GXBOOL  Initialize  (GXRASTERIZERDESC* pDesc);
    GXBOOL  Activate    (GRasterizerStateImpl* pPrevState);
    inline  void    InlSetRasterizerState();
  };

  class GBlendStateImpl : public GBlendState
  {
  private:
    GXGraphicsImpl*     m_pGraphicsImpl;
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
    GBlendStateImpl(GXGraphicsImpl* pGraphicsImpl);
    GXBOOL  Initialize  (GXBLENDDESC* pDesc, GXUINT nNum);
    GXBOOL  Activate    (GBlendStateImpl* pPrevState);

    virtual GXDWORD SetBlendFactor  (GXDWORD dwBlendFactor);

    inline  void    InlSetBlendState();
  };
  //////////////////////////////////////////////////////////////////////////
  class GDepthStencilStateImpl : public GDepthStencilState
  {
  private:
    GXGraphicsImpl*     m_pGraphicsImpl;
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
    GDepthStencilStateImpl(GXGraphicsImpl* pGraphicsImpl);
    GXBOOL  Initialize  (GXDEPTHSTENCILDESC* pDesc);
    GXBOOL  Activate    (GDepthStencilStateImpl* pPrevState);

    virtual GXDWORD SetStencilRef  (GXDWORD dwStencilRef);
  };
  //////////////////////////////////////////////////////////////////////////
  class GSamplerStateImpl : public GSamplerState
  {
    friend class GXGraphicsImpl;
    friend class GXCanvas;
  private:
    GXGraphicsImpl*       m_pGraphicsImpl;
    //GXSAMPLERSTAGE        m_SamplerStage[SAMPLERCOUNT];
    ID3D11SamplerState*   m_pSampler[SAMPLERCOUNT]; // TODO: 这个可以用池来减少实际创建的数量
    GXSAMPLERDESC         m_SamplerDesc[SAMPLERCOUNT];
    //DWORD                 m_dwChangeMask;
    //static GXSAMPLERSTAGE s_DefaultSamplerState;
  private:
    GSamplerStateImpl(GXGraphics* pGraphics);
    virtual ~GSamplerStateImpl();
    static GXBOOL     InitializeStatic  ();
    GXBOOL            Activate          (GSamplerStateImpl* pPrevSamplerState);  // 这个只能被Graphics调用!
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

    GXHRESULT         SetState          (GXUINT Sampler, GXSAMPLERDESC* pSamplerDesc);
    GXHRESULT         SetStateArray     (GXUINT nStartSlot, GXSAMPLERDESC* pSamplerDesc, int nCount);
    GXHRESULT         ResetToDefault    ();
  };

} // namespace D3D11

#endif // _STATE_BLOCK_H_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11