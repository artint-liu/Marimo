#if defined(ENABLE_GRAPHICS_API_DX9)
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_

#define GET_VSREGISTER_IDX(_HANDLE) (GXUINT)GXLOWORD(_HANDLE)
#define GET_PSREGISTER_IDX(_HANDLE)  (GXUINT)GXHIWORD(_HANDLE)

struct STANDARDMTLUNIFORMTABLE;

namespace D3D9
{
  class GraphicsImpl;
  //class IHLSLInclude;

  struct GXD3DXCONSTDESC : D3DXCONSTANT_DESC
  {
    GXDWORD       dwNameID;
    GXDWORD       dwHandle;  // 其实就是在clvector中从1开始的索引,Pixel的索引会左移16位
    GXINT_PTR     nCanvasUniform;
  };

  typedef GXD3DXCONSTDESC*        GXLPD3DXCONSTDESC;
  typedef const GXD3DXCONSTDESC*  GXLPCD3DXCONSTDESC;

  class GXCanvasImpl;
  class GShaderImpl : public GShader
  {
    friend class GraphicsImpl;
  public:
    typedef clvector<GXD3DXCONSTDESC>  ConstantDescArray;
    typedef const ConstantDescArray    ConstDescArray;

    enum CompiledType // 这个要和DX9,DX10,DX11里的定义一致
    {
      CompiledVertexShder,
      CompiledPixelShder,
      CompiledComponentVertexShder,
      CompiledComponentPixelShder,
    };

    enum ShaderFlag
    {
      ShaderFlag_PutInResourceManager = 0x00010000,
      ShaderFlag_Mask = 0xffff0000,
    };

    STATIC_ASSERT((ShaderFlag_Mask & GXSHADERCAP_MASK) == 0);

  protected:
    GraphicsImpl*  m_pGraphicsImpl;
    GXDWORD          m_dwFlag; // 高16位内部用(ShaderFlag)，低16位外部用(GXShaderCapability)
    GXINT            m_cbPixelTopIndex;
    GXINT            m_cbCacheSize;
    GXHRESULT        CleanUp              ();
    GXINT            UpdateConstTabDesc   (LPD3DXCONSTANTTABLE pct, LPD3DXCONSTANTTABLE_DESC pctd, GXUINT uHandleShift);

    GShaderImpl(Graphics* pGraphics);
    virtual  ~GShaderImpl();
  public:
    LPDIRECT3DVERTEXSHADER9       m_pVertexShader;
    LPDIRECT3DPIXELSHADER9        m_pPixelShader;
    LPD3DXCONSTANTTABLE           m_pvct;
    LPD3DXCONSTANTTABLE           m_ppct;
    clStringW                     m_strProfileDesc; // Shader 配置文件名+描述

    ConstantDescArray             m_aConstDesc;
    D3DXCONSTANTTABLE_DESC        m_VertexShaderConstTabDesc;
    D3DXCONSTANTTABLE_DESC        m_PixelShaderConstTabDesc;

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef            () override;
    virtual GXHRESULT   Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc) override;
    virtual GXHRESULT   LoadFromFile      (MOSHADER_ELEMENT_SOURCE* pSdrElementSrc) override;
    virtual GXHRESULT   LoadFromMemory    (const clBufferBase* pVertexBuf, const clBufferBase* pPixelBuf) override;
    virtual Graphics* GetGraphicsUnsafe () const override;
    virtual GXLPCWSTR   GetProfileDesc    () const override;
    CLDEPRECATED_ATTRIBUTE static  GXHRESULT   CompileShader     (clBuffer* pBuffer, LPD3DXINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled); // 编译后buffer将被二进制代码替换
    static  GXHRESULT   CompileShader     (clBuffer* pIntermediateCode, GXLPCSTR szSourceCode, size_t nSourceLen, LPD3DXINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled);

  public:
    GXHRESULT       Activate            ();
    ConstDescArray& GetConstantDescTable() const;
    GXINT           GetCacheSize        () const;
    inline GXINT    GetPixelIndexOffset () const;
    GXUINT          GetHandle           (GXLPCSTR pName) const;
    GXUniformType   GetHandleType       (GXUINT handle) const;
    GXUINT          GetStageByHandle    (GXUINT handle) const;
    void            PutInResourceMgr    ();
#ifdef REFACTOR_SHADER
    GXBOOL          CommitToDevice      (GXLPVOID lpUniform, GXSIZE_T cbSize);
#endif // #ifdef REFACTOR_SHADER
  };
  //////////////////////////////////////////////////////////////////////////
  inline GXINT GShaderImpl::GetCacheSize() const
  {
    return m_cbCacheSize;
  }
  inline GXINT GShaderImpl::GetPixelIndexOffset() const
  {
    return (m_cbPixelTopIndex >> 2) >> 2;
  }
  //////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
} // namespace D3D9

#endif // _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #if defined(ENABLE_GRAPHICS_API_DX9)