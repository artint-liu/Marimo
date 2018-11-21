#ifndef _STATE_BLOCK_H_
#define _STATE_BLOCK_H_

//#include <Include/GUnknown.h>




#define RENDERSTATECOUNT  103    // 枚举项的总数量
#define LASTRENDERSTATEENUM  209    // 最后一个枚举值

#define SAMPLERCOUNT      16
#define SAMPSTATECOUNT    13
#define LASTSAMPSTSTEENUM 13

//#include <vector>
//using namespace std;

class Graphics;

namespace D3D9
{
  class GraphicsImpl;
  //class GRenderStateImpl : public GUnknown
  //{
  //  friend class GXGraphicsImpl;
  //  friend class GXCanvasCoreImpl;
  //  friend class GXCanvas3DImpl;
  //private:
  //  struct GROUPMASK  // 分组掩码 
  //  {
  //    union{
  //      GXDWORD  dw;
  //      GXWORD  w[2];
  //      GXBYTE  b[4];
  //    };
  //  };
  //private:
  //  GRenderStateImpl(GXGraphics* pGraphics);
  //  static GXBOOL     InitializeStatic  ();
  //  GXBOOL            Update            (GRenderStateImpl* pPrevState);
  //  GXBOOL            ResetToDefault    ();
  //public:
  //  virtual GXLRESULT AddRef            ();
  //  virtual GXLRESULT Release           ();

  //  GXBOOL            Set               (GXRenderStateType eType, GXDWORD dwValue);
  //  GXDWORD           Get               (GXRenderStateType eType);
  //  GXBOOL            SetBlock          (GXLPCRENDERSTATE lpBlock);
  //private:
  //  static GXINT                s_aEnumToIdx           [LASTRENDERSTATEENUM];
  //  static GXDWORD              s_aRenderStateValue    [RENDERSTATECOUNT];
  //  static GXRenderStateType    s_aRenderStateTypeList [RENDERSTATECOUNT + 1];
  //  GXGraphicsImpl*   m_pGraphics;
  //  GROUPMASK         m_aChanged[(RENDERSTATECOUNT + 31) / 32];
  //  GXDWORD           m_aRenderStateValue[RENDERSTATECOUNT];
  //  GXBOOL            m_bOnDevice;
  //};

  class GRasterizerStateImpl : public GRasterizerState
  {
  private:
    GraphicsImpl*   m_pGraphicsImpl;
    GXRASTERIZERDESC  m_RasterizerDesc;
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef        () override;
    virtual GXHRESULT Release       () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) override { return GX_OK; }

  public:
    GRasterizerStateImpl(GraphicsImpl*        pGraphicsImpl);
    virtual ~GRasterizerStateImpl();
    GXBOOL  Initialize  (GXRASTERIZERDESC*      pDesc);
    GXBOOL  Activate    (GRasterizerStateImpl*  pPrevState);
  };

  class GBlendStateImpl : public GBlendState
  {
  private:
    GraphicsImpl* m_pGraphicsImpl;
    GXDWORD         m_BlendFactor;
    GXBLENDDESC     m_BlendDesc;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef            () override;
    virtual GXHRESULT Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }

  public:
    GBlendStateImpl(GraphicsImpl* pGraphicsImpl);
    virtual ~GBlendStateImpl();
    GXBOOL  Initialize  (GXBLENDDESC* pDesc, GXUINT nNum);
    GXBOOL  Activate    (GBlendStateImpl* pPrevState);

    virtual GXDWORD SetBlendFactor  (GXDWORD dwBlendFactor);

    inline  GXBOOL IsInvalidBlend (GXBlend eBlend);
  };

  inline GXBOOL GBlendStateImpl::IsInvalidBlend(GXBlend eBlend)
  {
    return eBlend < 1 || eBlend > 17;
  }
  //////////////////////////////////////////////////////////////////////////
  class GDepthStencilStateImpl : public GDepthStencilState
  {
    friend class GraphicsImpl;
  private:
    GraphicsImpl*     m_pGraphicsImpl;
    GXDWORD             m_StencilRef;
    GXDEPTHSTENCILDESC  m_Desc;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef            () override;
    virtual GXHRESULT Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) override { return GX_OK; }


  public:
    GDepthStencilStateImpl(GraphicsImpl* pGraphicsImpl);
    virtual ~GDepthStencilStateImpl();
    GXBOOL  Initialize      (GXDEPTHSTENCILDESC* pDesc);
    GXBOOL  Activate        (GDepthStencilStateImpl* pPrevState);
    GXBOOL  CheckStencilOp  (GXDEPTHSTENCILOP* pStencilOp, GXLPCSTR szPrefix);

    virtual GXDWORD SetStencilRef  (GXDWORD dwStencilRef) override;
  };
  //////////////////////////////////////////////////////////////////////////

  //struct GXSAMPLERSTATE
  //{
  //  D3DSAMPLERSTATETYPE eType;
  //  GXDWORD             dwValue;
  //};

  //struct GXSAMPLERSTAGE
  //{
  //  union {
  //    struct
  //    {
  //      GXDWORD dwMask;     // 如果State记录与默认值相符,则对应位是"0"
  //      GXDWORD dwAddressU;
  //      GXDWORD dwAddressV;
  //      GXDWORD dwAddressW;
  //      GXDWORD dwBorderColor;
  //      GXDWORD dwMagFilter;
  //      GXDWORD dwMinFilter;
  //      GXDWORD dwMipFilter;
  //      GXDWORD dwMipmapLodBias;
  //      GXDWORD dwMaxMipLevel;
  //      GXDWORD dwMaxAnisotropy;
  //      GXDWORD dwSRGBTexture;
  //      GXDWORD dwElementIndex;
  //      GXDWORD dwDMapOffset;
  //    };
  //    //STATIC_ASSERT(sizeof(DETAIL) == sizeof(GXDWORD) * 13);
  //    GXDWORD m[14];
  //  };
  //};

  //typedef GXSAMPLERSTATE* LPGXSAMPLERSTATE;
  //typedef GXSAMPLERSTATE* GXLPSAMPLERSTATE;
  //typedef GXSAMPLERSTAGE* GXLPSAMPLERSTAGE;
  //typedef const GXSAMPLERSTAGE* GXLPCSAMPLERSTAGE;
  //void IntSetSamplerToDefault(GXLPSAMPLERSTAGE lpSampStage);

  class GSamplerStateImpl : public GSamplerState
  {
    friend class GraphicsImpl;
    friend class GXCanvas;
  private:
    GraphicsImpl*       m_pGraphicsImpl;
    //GXSAMPLERSTAGE        m_SamplerStage[SAMPLERCOUNT];
    GXSAMPLERDESC         m_SamplerDesc[SAMPLERCOUNT];
    GXDWORD               m_dwDefaultMask[SAMPLERCOUNT];  // 如果是默认值,对应位置0
    //static GXSAMPLERSTAGE s_DefaultSamplerState;
  private:
    GSamplerStateImpl(Graphics* pGraphics);
    virtual ~GSamplerStateImpl();

    static GXBOOL     InitializeStatic  ();
    GXBOOL            Initialize        (GSamplerStateImpl* pDefault);
    GXBOOL            Activate          (GSamplerStateImpl* pPrevSamplerState);  // 这个只能被Graphics调用!
    void              SetStageToDevice  (GXUINT Stage, const GXSAMPLERDESC* pPrevSampDesc);
    void              IntUpdateStates   (GXUINT Sampler, const GXSAMPLERDESC* pSamplerDesc, const GXSAMPLERDESC* pDefault);
    //void              SetStateToDevice  (DWORD dwStage, D3DSAMPLERSTATETYPE eType);
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef          () override;
    virtual GXHRESULT Release         () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke          (GRESCRIPTDESC* pDesc) override { return GX_OK; }
    virtual GXHRESULT SetState        (GXUINT nSamplerSlot, GXSAMPLERDESC* pSamplerDesc) override;
    virtual GXHRESULT SetStateArray   (GXUINT nStartSlot, GXSAMPLERDESC* pSamplerDesc, int nCount) override;
    virtual GXHRESULT ResetToDefault  () override;
    //GXBOOL            ResetToDefault    ();

    GXBOOL            Set               (GXDWORD Sampler, D3DSAMPLERSTATETYPE eType, GXDWORD dwValue);
    GXDWORD           Get               (GXDWORD Sampler, D3DSAMPLERSTATETYPE eType);
  };
} // namespace D3D9

#endif // _STATE_BLOCK_H_