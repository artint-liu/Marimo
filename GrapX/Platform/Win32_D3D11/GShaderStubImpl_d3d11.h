#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifdef ENABLE_GRAPHICS_API_DX11

#ifndef _SHADERSTUB_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADERSTUB_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_

namespace D3D11
{
  class GShaderStubImpl : public GShaderStub
  {
    // Common 与 Unusual 相对
    struct COMMONUNIFORM
    {
      GXLPCCONSTDESC      pConstDesc;
      GXUINT              cbSize;
      GXSHORT             nOffsetOf;
      GXSHORT             nBinderIdx;
      GXBOOL   SortCompare(COMMONUNIFORM& Stru);
      GXVOID   SortSwap   (COMMONUNIFORM& Stru);
    };
    struct BINDERSECTDESC
    {
      LPCDATALAYOUT  lpDefine;
      GXINT nTopIndex;
    };
    typedef clvector<COMMONUNIFORM>   CommonUniformArray;
    typedef clvector<BINDERSECTDESC>  BinderSectDescArray; // TODO: 以后要储存LPCDATALAYOUT创建出来的对象再做绑定, 直接使用这个不安全

  private:
    GXGraphicsImpl*     m_pGraphicsImpl;
    GShaderImpl*        m_pShaderImpl;
    CommonUniformArray  m_aCommonUniforms;
    BinderSectDescArray m_aBinderSectDesc;

  private:
    GXBOOL        IntSortByBinder            ();   // m_aCommonUniforms 按照Binder索引顺序排序
    GXBOOL        IntGenerateBinderTopIndex  ();   // 整理出每个Binder在m_aCommonUniforms中的开始索引
    GXBOOL        IntSetUniform              (GXLPCCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom = NULL);
    GXBOOL        IntIsTextureHandle         (GXUINT uHandle);
  public:
    GShaderStubImpl(GXGraphics* pGraphics);
    GXHRESULT     SetShaderRef            (GShader* pShader);
    virtual GXHRESULT     BindData                (MODataPool* pDataPool, GXLPCSTR szStruct);
    virtual GXHRESULT     FindDataPoolByName      (GXLPCSTR szName, MODataPool** ppDataPool);
    virtual GXHRESULT     BindCommonUniform       (LPCDATALAYOUT lpUniformDef);
    virtual GXINT         FindUniformDef          (LPCDATALAYOUT lpUniformDef);
    GXBOOL        CommitUniform           (int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset);
    GShader*      GetShaderUnsafe         () const;
    GXUINT        GetHandleByName         (GXLPCSTR pName) const;
    GXUINT        GetHandleByIndex        (GXUINT nIndex) const;
    GXUniformType GetHandleType           (GXUINT handle) const;
    GXUINT        GetSamplerStageByHandle (GXUINT handle) const;
    GXBOOL        SetUniformByHandle      (clBufferBase* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount);
    GXBOOL        GetUniformByIndex       (GXUINT nIndex, UNIFORMDESC* pDesc) const;
    GXBOOL        SetTextureByHandle      (GTextureBase** pTextureArray, GXUINT uHandle, GTextureBase* pTexture);
    GXBOOL        SetTextureByIndex       (GTextureBase** pTextureArray, GXUINT nIndex, GTextureBase* pTexture);
#ifdef REFACTOR_SHADER
    GXBOOL        CommitToDevice          (GXLPVOID lpUniform, GXUINT cbSize);
    GXBOOL        UpdateCanvasUniform     (GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXUINT cbSize);
    GXBOOL        UpdateUniform           (int nDefIdx, GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXUINT cbSize);
#endif // #ifdef REFACTOR_SHADER

    // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef          ();
    virtual GXHRESULT Release         ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Invoke          (GRESCRIPTDESC* pDesc) { return GX_OK; }

    virtual GXHRESULT SetTextureSlot  (GXLPCSTR pName, GXINT nSlot);
    virtual GXINT     GetTextureSlot  (GXLPCSTR pName);
  };
} // namespace D3D11

#endif // #ifndef _SHADERSTUB_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)