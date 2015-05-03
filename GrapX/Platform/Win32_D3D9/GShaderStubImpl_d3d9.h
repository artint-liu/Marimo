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
    GXGraphicsImpl*     m_pGraphicsImpl;
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
    GShaderStubImpl(GXGraphics* pGraphics);
    virtual GXHRESULT     SetShaderRef            (GShader* pShader);
    virtual GXHRESULT     BindData                (MODataPool* pDataPool, GXLPCSTR szStruct);
    virtual GXHRESULT     FindDataPoolByName      (GXLPCSTR szName, MODataPool** ppDataPool);
    virtual GXHRESULT     BindCommonUniform       (LPCDATALAYOUT lpUniformDef);
    virtual GXINT         FindUniformDef          (LPCDATALAYOUT lpUniformDef);
    virtual GXBOOL        CommitUniform           (int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset);
    virtual GShader*      GetShaderUnsafe         () GXCONST;
    virtual GXUINT        GetHandleByName         (GXLPCSTR pName) GXCONST;
    virtual GXUINT        GetHandleByIndex        (GXUINT nIndex) GXCONST;
    virtual GXUniformType GetHandleType           (GXUINT handle) GXCONST;
    virtual GXUINT        GetSamplerStageByHandle (GXUINT handle) GXCONST;
    virtual GXBOOL        GetUniformByIndex       (GXUINT nIndex, UNIFORMDESC* pDesc) GXCONST;
    virtual GXBOOL        SetUniformByHandle      (clBufferBase* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount);
    virtual GXBOOL        SetTextureByHandle      (GTextureBase** pTextureArray, GXUINT uHandle, GTextureBase* pTexture);
    virtual GXBOOL        SetTextureByIndex       (GTextureBase** pTextureArray, GXUINT uIndex, GTextureBase* pTexture);
#ifdef REFACTOR_SHADER
    virtual GXBOOL        CommitToDevice          (GXLPVOID lpUniform, GXUINT cbSize);
    virtual GXBOOL        UpdateCanvasUniform     (GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXUINT cbSize);
    virtual GXBOOL        UpdateUniform           (int nDefIdx, GXLPCBYTE lpCanvasUniform, GXLPVOID lpUniform, GXUINT cbSize);
#endif // #ifdef REFACTOR_SHADER

    // 接口实现
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT     AddRef                  ();
    virtual GXHRESULT     Release                 ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT     Invoke                  (GRESCRIPTDESC* pDesc);

    virtual GXHRESULT     SetTextureSlot          (GXLPCSTR pName, GXINT nSlot);
    virtual GXINT         GetTextureSlot          (GXLPCSTR pName);
  };
  //////////////////////////////////////////////////////////////////////////

} // namespace D3D9

#endif // #ifndef _SHADERSTUB_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)