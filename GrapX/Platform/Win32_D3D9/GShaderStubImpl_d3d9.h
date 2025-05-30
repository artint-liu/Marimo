#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADERSTUB_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADERSTUB_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_

namespace D3D9
{
  class GShaderStubImpl : public GShaderStub
  {
    // Common 与 Unusual 相对
    struct COMMONUNIFORM
    {
      GXLPCD3DXCONSTDESC  pConstDesc;
      GXUINT              cbSize;
      GXSHORT             nOffsetOf;
      GXSHORT             nBinderIdx;
      GXBOOL   SortCompare(COMMONUNIFORM& Stru);
      GXVOID   SortSwap   (COMMONUNIFORM& Stru);
    };
    struct BINDERSECTDESC
    {
      GXINT           nTopIndex;
      LPCDATALAYOUT   lpDefine;

      // DataPool 使用的段
      //MODataPool*     pDataPool;
      //clStringA       StructName;
      MOVariable      Var;
    };
    typedef clvector<COMMONUNIFORM>   CommonUniformArray;
    typedef clvector<BINDERSECTDESC>  BinderSectDescArray; // TODO: 以后要储存LPCDATALAYOUT创建出来的对象再做绑定, 直接使用这个不安全

  private:
    GraphicsImpl*     m_pGraphicsImpl;
    GShaderImpl*        m_pShaderImpl;
    CommonUniformArray  m_aCommonUniforms;
    BinderSectDescArray m_aBinderSectDesc;

  private:
    GXBOOL        IntSortByBinder            ();   // m_aCommonUniforms 按照Binder索引顺序排序
    GXBOOL        IntGenerateBinderTopIndex  ();   // 整理出每个Binder在m_aCommonUniforms中的开始索引
#ifdef REFACTOR_SHADER
    GXBOOL        IntUpdateUniform           (const GXLPCD3DXCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom);
#else
    //GXBOOL        IntSetUniform              (const GXLPCD3DXCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom = NULL);
#endif
    GXBOOL        IntSetUniform              (const GXLPCD3DXCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom = NULL);
    GXBOOL        IntIsTextureHandle         (GXUINT uHandle);
    GXINT         IntGetDataPool             (MODataPool* pDataPool);

  public:
    GShaderStubImpl(Graphics* pGraphics);
    virtual GXHRESULT     SetShaderRef            (GShader* pShader) override;
    virtual GXHRESULT     BindData                (MODataPool* pDataPool, GXLPCSTR szStruct) override;
    virtual GXHRESULT     FindDataPoolByName      (GXLPCSTR szName, MODataPool** ppDataPool) override;
    virtual GXHRESULT     BindCommonUniform       (LPCDATALAYOUT lpUniformDef) override;
    virtual GXINT         FindUniformDef          (LPCDATALAYOUT lpUniformDef) override;
    virtual GXBOOL        CommitUniform           (int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset) override;
    virtual GShader*      GetShaderUnsafe         () const override;
    virtual GXUINT        GetHandleByName         (GXLPCSTR pName) const override;
    virtual GXUINT        GetHandleByIndex        (GXUINT nIndex) const override;
    virtual GXUniformType GetHandleType           (GXUINT handle) const override;
    virtual GXUINT        GetSamplerStageByHandle (GXUINT handle) const override;
    virtual GXBOOL        GetUniformByIndex       (GXUINT nIndex, UNIFORMDESC* pDesc) const override;
    virtual GXBOOL        SetUniformByHandle      (clBufferBase* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount) override;
    virtual GXBOOL        SetTextureByHandle      (GTextureBase** pTextureArray, GXUINT uHandle, GTextureBase* pTexture) override;
    virtual GXBOOL        SetTextureByIndex       (GTextureBase** pTextureArray, GXUINT uIndex, GTextureBase* pTexture) override;
#ifdef REFACTOR_SHADER
    virtual GXBOOL        CommitToDevice          (GXLPVOID lpUniform, GXSIZE_T cbSize) override;
    virtual GXBOOL        UpdateCanvasUniform     (GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXSIZE_T cbSize) override;
    virtual GXBOOL        UpdateUniform           (int nDefIdx, GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXSIZE_T cbSize) override;
#endif // #ifdef REFACTOR_SHADER

    // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT     AddRef                  () override;
    virtual GXHRESULT     Release                 () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT     Invoke                  (GRESCRIPTDESC* pDesc) override;

    virtual GXHRESULT     SetTextureSlot          (GXLPCSTR pName, GXINT nSlot);
    virtual GXINT         GetTextureSlot          (GXLPCSTR pName);
  };
  //////////////////////////////////////////////////////////////////////////

} // namespace D3D9

#endif // #ifndef _SHADERSTUB_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)