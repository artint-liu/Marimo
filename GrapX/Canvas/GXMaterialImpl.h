#ifndef _IMPLEMENT_GRAPH_X_MATERIAL_H_
#define _IMPLEMENT_GRAPH_X_MATERIAL_H_
struct STANDARDMTLUNIFORMTABLE;
class BlendState;
class DepthStencilState;

#define DEFAULT_RENDER_QUEUE 100
//////////////////////////////////////////////////////////////////////////
namespace GrapX
{
  class SamplerState;

  class MaterialImpl : public Material
  {
  private:
    struct RENDERSTATE
    {
      GXRASTERIZERDESC    RasterizerDesc;
      GXBLENDDESC         BlendDesc;
      GXDEPTHSTENCILDESC  DepthStencilDesc;
    };
    typedef clhash_map<clStringA, GXUINT> MtlStateDict;
    typedef clvector<ObjectT<Texture> > TextureArray;
    //typedef clstd::FixedBuffer clFixedBuffer;
  private:
    static MtlStateDict s_MtlStateDict;
    Graphics*             m_pGraphics;
    Shader*               m_pShader;
    Marimo::DataPool*     m_pDataPool;
    TextureArray          m_aTextures;

    //GShaderStub*        m_pShaderStub;
    //GTextureBase*       m_aTextureSlot[GX_MAX_TEXTURE_STAGE];
    //clFixedBuffer       m_Buffer;
    //GRasterizerState*   m_pRasterizer;
    //GDepthStencilState* m_pDepthStencil;
    //GBlendState*        m_pBlendState;
    //GSamplerState*      m_pSamplerState;
    //UniformArray*       m_pUniformsForReloading;    // 用来Reload前的储存数据, 将来可能还要好好想想放在什么位置合理
    int                   m_nRenderQueue;
    //GXDWORD             m_bSequential : 1;

  public:
    static GXVOID InitializeMtlStateDict();
    static GXVOID FinalizeMtlStateDict  ();
  protected:
    //int       SetSampler      (GXDEFINITION* pParameters, GXSIZE_T nCount);
    //GXHRESULT SetUniforms     (GXDEFINITION* pParameters, GXSIZE_T nCount);
    //GXHRESULT SetRenderStates (GXDEFINITION* pParameters, GXSIZE_T nCount);
    virtual ~MaterialImpl();
  public:
    MaterialImpl(Graphics* pGraphics, Shader* pShader);
//    GXHRESULT     SetShaderRef        (GShader* pShader);
//    GShader*      GetShaderUnsafe     () const;
//    GShaderStub*  GetShaderStubUnsafe () const;
//    GXUINT        GetHandle           (GXLPCSTR szName) const;
//#ifdef REFACTOR_SHADER
    GXHRESULT     IntCommit           (GXLPCBYTE lpCanvasUniform);
//#else
//    GXHRESULT     IntCommit           (const STANDARDMTLUNIFORMTABLE* pStdUniforms);
//#endif // REFACTOR_SHADER

    // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   AddRef            () override;
    GXHRESULT   Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc) override;
    Graphics*   GetGraphicsUnsafe () const override;

    Marimo::DataPoolVariable GetUniform(GXLPCSTR szName) override;
    GXBOOL       SetTexture(GXUINT nSlot, Texture* pTexture) override;
    GXBOOL       SetTexture(GXLPCSTR szSamplerName, Texture* pTexture) override;

    //virtual GXHRESULT   SetTextureSlot    (GXLPCSTR szName, GXINT nSlot);
    //virtual GXINT       GetTextureSlot    (GXLPCSTR szName);
    //virtual GXBOOL      IsSequential      () override;
    int         GetRenderQueue    () const override;
    //virtual GrapX::Graphics* GetGraphicsUnsafe () override;
    GXHRESULT   GetFilename      (clStringW* pstrFilename) override;

    GXBOOL InitMaterial();
    //virtual GXHRESULT   Clone             (GXMaterialInst** ppCuplicateMtlInst) override;

    //virtual GXHRESULT   SaveFileW         (GXLPCWSTR szFilename) override;
    //virtual GXHRESULT   SaveRepository    (clSmartRepository* pStorage) override;

    //virtual GXHRESULT   LoadFileW         (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename) override;
    //virtual GXHRESULT   LoadRepository    (GrapX::Graphics* pGraphics, clSmartRepository* pStorage) override;

    //virtual GXHRESULT GetUniformList    (UniformArray* pUniforms) override;
    //virtual GXHRESULT BindData          (MODataPool* pDataPool, GXLPCSTR szStruct) override;
    GXHRESULT BindDataByName    (GXLPCSTR szPoolName, GXLPCSTR szStruct) override;
    GXHRESULT SetParameters     (ParamType eType, GXDEFINITION* pParameters, int nCount) override;
    //virtual GXHRESULT SetFloat1ByName   (GXLPCSTR szName, float val) override;
    //virtual GXHRESULT SetFloat2ByName   (GXLPCSTR szName, const float2& vFloat2) override;
    //virtual GXHRESULT SetFloat3ByName   (GXLPCSTR szName, const float3& vFloat3) override;
    //virtual GXHRESULT SetFloat4ByName   (GXLPCSTR szName, const float4& vFloat4) override;
    //virtual GXHRESULT SetMatrixByName   (GXLPCSTR szName, const float4x4& mat) override;

    //virtual GXHRESULT SetTextureByName            (GXLPCSTR szName, GTextureBase* pTexture) override;
    //virtual GXHRESULT SetTextureByIndex           (GXUINT nIndex, GTextureBase* pTexture) override;
    //virtual GXHRESULT SetTextureByNameFromFileW   (GXLPCSTR szName, GXLPCWSTR szFilename) override;
    //virtual GXHRESULT SetTextureByIndexFromFileW  (GXUINT nIndex, GXLPCWSTR szFilename) override;

    Marimo::DataPool* GetDataPoolUnsafe() const;
    GXBOOL Commit();

    inline  Shader*      InlGetShaderUnsafe    () const
    {
      return m_pShader;
    }

    //inline  GShaderStub*  InlGetShaderStubUnsafe() const;
  };
} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_MATERIAL_H_