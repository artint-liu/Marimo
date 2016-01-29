#ifndef _IMPLEMENT_GRAPH_X_MATERIAL_H_
#define _IMPLEMENT_GRAPH_X_MATERIAL_H_
struct STANDARDMTLUNIFORMTABLE;
class GBlendState;
class GDepthStencilState;
class GSamplerState;
//////////////////////////////////////////////////////////////////////////
class GXMaterialInstImpl : public GXMaterialInst
{
private:
  struct RENDERSTATE
  {
    GXRASTERIZERDESC    RasterizerDesc;
    GXBLENDDESC         BlendDesc;
    GXDEPTHSTENCILDESC  DepthStencilDesc;
  };
  typedef clhash_map<clStringA, GXUINT> MtlStateDict;
  typedef clstd::FixedBuffer clFixedBuffer;
private:
  static MtlStateDict s_MtlStateDict;
  GXGraphics*         m_pGraphics;
  GShaderStub*        m_pShaderStub;
  GTextureBase*       m_aTextureSlot[GX_MAX_TEXTURE_STAGE];
  clFixedBuffer       m_Buffer;
  GRasterizerState*   m_pRasterizer;
  GDepthStencilState* m_pDepthStencil;
  GBlendState*        m_pBlendState;
  GSamplerState*      m_pSamplerState;
  UniformArray*       m_pUniformsForReloading;    // 用来Reload前的储存数据, 将来可能还要好好想想放在什么位置合理
  int                 m_nRenderQueue;
  GXDWORD             m_bSequential : 1;

public:
  static GXVOID InitializeMtlStateDict();
  static GXVOID FinalizeMtlStateDict  ();
protected:
  int       SetSampler      (GXDEFINITION* pParameters, GXSIZE_T nCount);
  GXHRESULT SetUniforms     (GXDEFINITION* pParameters, GXSIZE_T nCount);
  GXHRESULT SetRenderStates (GXDEFINITION* pParameters, GXSIZE_T nCount);
  virtual ~GXMaterialInstImpl();
public:
  GXMaterialInstImpl(GXGraphics* pGraphics);
  GXHRESULT     SetShaderRef        (GShader* pShader);
  GShader*      GetShaderUnsafe     () GXCONST;
  GShaderStub*  GetShaderStubUnsafe () GXCONST;
  GXUINT        GetHandle           (GXLPCSTR szName) GXCONST;
#ifdef REFACTOR_SHADER
  GXHRESULT     IntCommit           (GXLPCBYTE lpCanvasUniform);
#else
  GXHRESULT     IntCommit           (const STANDARDMTLUNIFORMTABLE* pStdUniforms);
#endif // REFACTOR_SHADER

  // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT   AddRef            ();
  virtual GXHRESULT   Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc);
  virtual GXHRESULT   SetTextureSlot    (GXLPCSTR szName, GXINT nSlot);
  virtual GXINT       GetTextureSlot    (GXLPCSTR szName);
  virtual GXBOOL      IsSequential      ();
  virtual int         GetRenderQueue    () const;
  virtual GXGraphics* GetGraphicsUnsafe ();
  virtual GXHRESULT   GetFilenameW      (clStringW* pstrFilename);
  virtual GXHRESULT   Clone             (GXMaterialInst** ppCuplicateMtlInst);

  virtual GXHRESULT   SaveFileW         (GXLPCWSTR szFilename);
  virtual GXHRESULT   SaveRepository    (clSmartRepository* pStorage);

  virtual GXHRESULT   LoadFileW         (GXGraphics* pGraphics, GXLPCWSTR szFilename);
  virtual GXHRESULT   LoadRepository    (GXGraphics* pGraphics, clSmartRepository* pStorage);

  virtual GXHRESULT GetUniformList    (UniformArray* pUniforms);
  virtual GXHRESULT BindData          (MODataPool* pDataPool, GXLPCSTR szStruct);
  virtual GXHRESULT BindDataByName    (GXLPCSTR szPoolName, GXLPCSTR szStruct);
  virtual GXHRESULT SetParameters     (ParamType eType, GXDEFINITION* pParameters, int nCount);
  virtual GXHRESULT SetFloat1ByName   (GXLPCSTR szName, float val);
  virtual GXHRESULT SetFloat2ByName   (GXLPCSTR szName, const float2& vFloat2);
  virtual GXHRESULT SetFloat3ByName   (GXLPCSTR szName, const float3& vFloat3);
  virtual GXHRESULT SetFloat4ByName   (GXLPCSTR szName, const float4& vFloat4);
  virtual GXHRESULT SetMatrixByName   (GXLPCSTR szName, const float4x4& mat);

  virtual GXHRESULT SetTextureByName            (GXLPCSTR szName, GTextureBase* pTexture);
  virtual GXHRESULT SetTextureByIndex           (GXUINT nIndex, GTextureBase* pTexture);
  virtual GXHRESULT SetTextureByNameFromFileW   (GXLPCSTR szName, GXLPCWSTR szFilename);
  virtual GXHRESULT SetTextureByIndexFromFileW  (GXUINT nIndex, GXLPCWSTR szFilename);

  inline  GShader*      InlGetShaderUnsafe    () GXCONST;
  inline  GShaderStub*  InlGetShaderStubUnsafe() GXCONST;
};
//////////////////////////////////////////////////////////////////////////
inline GShader* GXMaterialInstImpl::InlGetShaderUnsafe() GXCONST
{
  return m_pShaderStub->GetShaderUnsafe();
}

inline GShaderStub* GXMaterialInstImpl::InlGetShaderStubUnsafe() GXCONST
{
  return m_pShaderStub;
}

#endif // _IMPLEMENT_GRAPH_X_MATERIAL_H_