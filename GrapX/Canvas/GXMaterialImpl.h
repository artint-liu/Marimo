#ifndef _IMPLEMENT_GRAPH_X_MATERIAL_H_
#define _IMPLEMENT_GRAPH_X_MATERIAL_H_
struct STANDARDMTLUNIFORMTABLE;
class BlendState;
class DepthStencilState;

namespace Marimo
{
  class DataPoolVariable;
} // namespace Marimo

#define DEFAULT_RENDER_QUEUE GrapX::RenderQueue_Geometry
#define MATERIAL_FLAG_UNIFORM_CHANGED 0x0001
#define MATERIAL_FLAG_TEXTURE_CHANGED 0x0002
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

    struct TEXTUREUNIT
    {
      ObjectT<TextureBase> texture;
      MOVarFloat4 TexelSize;
      void UpdateTexelSize(GXDWORD& dwFlags);
    };
    typedef clhash_map<clStringA, GXUINT> MtlStateDict;
    typedef clvector<ObjectT<SamplerState> > SamplerStateArray;
    typedef clvector<TEXTUREUNIT> TextureUnitArray;

  private:
    static MtlStateDict s_MtlStateDict;
    Graphics*             m_pGraphics;
    Shader*               m_pShader = NULL;
    Marimo::DataPool*     m_pDataPool = NULL;
    SamplerStateArray     m_aSamplerStates;
    TextureUnitArray      m_aTextures;

    RasterizerState*      m_pRasterizer = NULL;
    DepthStencilState*    m_pDepthStencil = NULL;
    BlendState*           m_pBlendState = NULL;
    int                   m_nRenderQueue = DEFAULT_RENDER_QUEUE;
    GXDWORD               m_dwFlags = 0;

  public:
    static GXVOID InitializeMtlStateDict();
    static GXVOID FinalizeMtlStateDict  ();
  protected:
    virtual ~MaterialImpl();

  public:
    MaterialImpl(Graphics* pGraphics, Shader* pShader);
    GXHRESULT     IntCommit           (GXLPCBYTE lpCanvasUniform);

    // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   AddRef            () override;
    GXHRESULT   Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc) override;
    Graphics*   GetGraphicsUnsafe () const override;

    void SetDepthStencilState(DepthStencilState* pState) override;
    void SetRasterizerState(RasterizerState* pState) override;
    void SetBlendState(BlendState* pState) override;
    GXBOOL SetState(GXLPCSTR szStateCommand) override;

    Marimo::DataPoolVariable GetUniform(GXLPCSTR szName) override;
    GXBOOL    SetTexture(GXUINT nSlot, TextureBase* pTexture) override;
    GXBOOL    GetTexture(GXLPCSTR szSamplerName, TextureBase** ppTexture) override;
    GXBOOL    SetTexture(GXLPCSTR szSamplerName, TextureBase* pTexture) override;

    int       SetRenderQueue    (int nRenderQueue) override;
    int       GetRenderQueue    () const override;
    GXBOOL    GetFilename       (clStringW* pstrFilename) override;

    GXBOOL    InitMaterial();


    GXBOOL    SetFloat(GXLPCSTR szName, float value) override;
    float     GetFloat(GXLPCSTR szName) override;
    GXBOOL    SetVector(GXLPCSTR szName, float4* pVector) override;
    GXBOOL    GetVector(float4* pOut, GXLPCSTR szName) override;

    //virtual GXHRESULT   Clone             (GXMaterialInst** ppCuplicateMtlInst) override;

    //virtual GXHRESULT   SaveFileW         (GXLPCWSTR szFilename) override;
    //virtual GXHRESULT   SaveRepository    (clSmartRepository* pStorage) override;

    //virtual GXHRESULT   LoadFileW         (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename) override;
    //virtual GXHRESULT   LoadRepository    (GrapX::Graphics* pGraphics, clSmartRepository* pStorage) override;

    //virtual GXHRESULT GetUniformList    (UniformArray* pUniforms) override;
    //virtual GXHRESULT BindData          (MODataPool* pDataPool, GXLPCSTR szStruct) override;
    GXHRESULT BindDataByName    (GXLPCSTR szPoolName, GXLPCSTR szStruct) override;
    GXHRESULT SetParameters     (ParamType eType, GXDEFINITION* pParameters, int nCount) override;

    Marimo::DataPool* GetDataPoolUnsafe() const;
    GXBOOL CommitTextures(GXBOOL bMaterialChanged);
    GXBOOL CommitStates();

    inline  Shader*      InlGetShaderUnsafe    () const
    {
      return m_pShader;
    }

    inline GXDWORD GetFlags() const
    {
      return m_dwFlags;
    }

    inline void ClearFlags()
    {
      m_dwFlags = 0;
    }

    //inline  GShaderStub*  InlGetShaderStubUnsafe() const;
  };
} // namespace GrapX
#endif // _IMPLEMENT_GRAPH_X_MATERIAL_H_