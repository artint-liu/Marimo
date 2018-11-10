#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_

//////////////////////////////////////////////////////////////////////////
// 关于ShaderConst和Uniform的含义:
// Const表示常量,VertexShader 和 PixelShader 分别包含各自的Const常量
// 当两个Shader合并使用时, 同名同长度(不同长度应该在以后检测报错)的Const会合并为一个记录,
// 这称为 Uniform

namespace D3D11
{
  class GXGraphicsImpl;

  struct GXCONSTDESC
  {
    clStringA     strName;
    GXDWORD       dwNameID;
    UINT          nConstBuf;    // Vertex/Pixel shader 上的 const buffer 索引
    UINT          nCBArrayIdx;  // 对象内部的 const buffer 队列索引
    GXDWORD       dwHandle;  // 其实就是在clvector中从1开始的索引,Pixel的索引会左移16位

    UINT          StartOffset;    // Offset in constant buffer's backing store
    UINT          Size;           // Size of variable (in bytes)
    UINT          uFlags;         // Variable flags
    UINT          StartTexture;   // First texture index (or -1 if no textures used)
    UINT          TextureSize;    // Number of texture slots possibly used.
    UINT          StartSampler;   // First sampler index (or -1 if no textures used)
    UINT          SamplerSize;    // Number of sampler slots possibly used.
    D3D_SHADER_VARIABLE_CLASS   Class;          // Variable class (e.g. object, matrix, etc.)
    D3D_SHADER_VARIABLE_TYPE    Type;           // Variable type (e.g. float, sampler, etc.)
    UINT          Rows;           // Number of rows (for matrices, 1 for other numeric, 0 if not applicable)
    UINT          Columns;        // Number of columns (for vectors & matrices, 1 for other numeric, 0 if not applicable)
    UINT          Elements;       // Number of elements (0 if not an array)
    UINT          Members;        // Number of members (0 if not a structure)
    UINT          MemberOffset;         // Offset from the start of structure (0 if not a structure member)
  };

  struct GXSDRBUFFERPAIR  // TODO: 这个结构设计不好,应该进行优化
  {
    UINT          nIdx;
    ID3D11Buffer* pD3D11ResBufer;
    GXBYTE*       pUserBuffer;
    GXUINT        BufferSize;
    GXUINT        bNeedUpdate : 1;
    GXUINT        bPS : 1;
  };

  typedef GXCONSTDESC*        GXLPCONSTDESC;
  typedef const GXCONSTDESC*  GXLPCCONSTDESC;

  class GXCanvasImpl;
  class GShaderImpl : public GShader
  {
    friend class GXGraphicsImpl;
  public:
    typedef clvector<GXCONSTDESC>     ConstantDescArray;
    typedef clvector<GXSDRBUFFERPAIR> BufPairArray;

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

  protected:
    GXGraphicsImpl*   m_pGraphicsImpl;
    GXDWORD           m_dwFlag;
    GXINT             m_cbPixelTopIndex;
    GXINT             m_cbCacheSize;
    BufPairArray      m_aBufPairs; // TODO: 设计不好,要优化
    GXHRESULT         CleanUp                 ();
    GXINT             UpdateConstTabDesc      (ID3D11ShaderReflection* pct, D3D11_SHADER_DESC* pctd, GXUINT uHandleShift);
    UINT              CreateShaderConstBuffer (UINT cbSize, UINT nIdx, GXBOOL bPS);

    virtual  ~GShaderImpl();
  public:
    ID3D11VertexShader*         m_pVertexShader;
    ID3D11PixelShader*          m_pPixelShader;
    ID3D11ShaderReflection*     m_pvct;
    ID3D11ShaderReflection*     m_ppct;
    //clStringA                   m_strVertexFile;
    //clStringA                   m_strPixelFile;
    clStringW                   m_strProfileDesc; // Shader 配置文件名
    clBuffer*                   m_pVertexBuf;   // 用来和顶点声明进行绑定

    ConstantDescArray           m_aConstDesc;
    D3D11_SHADER_DESC           m_VertexShaderConstTabDesc;
    D3D11_SHADER_DESC           m_PixelShaderConstTabDesc;

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef            ();
    virtual GXHRESULT   Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc) { return GX_OK; }


    virtual GXHRESULT   LoadFromFile      (MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
    virtual GXHRESULT   LoadFromMemory    (const clBufferBase* pVertexBuf, const clBufferBase* pPixelBuf);

    //virtual GXDWORD     GetFlags          () GXCONST;
    virtual GXGraphics* GetGraphicsUnsafe () const;
    virtual GXLPCWSTR   GetProfileDesc    () const;
    CLDEPRECATED_ATTRIBUTE static  GXHRESULT   CompileShader     (clBuffer* pBuffer, LPD3DINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled); // 编译后buffer将被二进制代码替换
    static  GXHRESULT   CompileShader     (clBuffer* pIntermediateCode, GXLPCSTR szSourceCode, size_t nSourceLen, LPD3DINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled);
  public:
    typedef const ConstantDescArray ConstDescArray;

    GXHRESULT       Activate            ();
    GXBOOL          CheckUpdateConstBuf ();
    ConstDescArray& GetConstantDescTable() const;
    GXINT           GetCacheSize        () const;
    inline GXINT    GetPixelIndexOffset () const;
    GXUINT          GetHandle           (GXLPCSTR pName) const;
    GXUniformType   GetHandleType       (GXUINT handle) const;
    GXUINT          GetStageByHandle    (GXUINT handle) const;
    void            PutInResourceMgr    ();
#ifdef REFACTOR_SHADER
    GXBOOL          CommitToDevice      (GXLPVOID lpUniform, GXUINT cbSize);
#endif // #ifdef REFACTOR_SHADER

    inline BufPairArray& GetPairs       ();

    GShaderImpl(GXGraphics* pGraphics);
  };
  //////////////////////////////////////////////////////////////////////////
  inline GXINT GShaderImpl::GetCacheSize() const
  {
    return m_cbCacheSize;
  }
  inline GXINT GShaderImpl::GetPixelIndexOffset() const
  {
    return m_cbPixelTopIndex;
    //return (m_cbPixelTopIndex >> 2) >> 2;  // sizeof(float4)
  }
  inline GShaderImpl::BufPairArray& GShaderImpl::GetPairs()
  {
    return m_aBufPairs;
  }
  //////////////////////////////////////////////////////////////////////////
  //class GShaderStubImpl : public GShaderStub
  //{
  //  // Common 与 Unusual 相对
  //  struct COMMONUNIFORM
  //  {
  //    GXLPCCONSTDESC      pConstDesc;
  //    GXUINT              cbSize;
  //    GXSHORT             nOffsetOf;
  //    GXSHORT             nBinderIdx;
  //    GXBOOL   SortCompare(COMMONUNIFORM& Stru);
  //    GXVOID   SortSwap   (COMMONUNIFORM& Stru);
  //  };
  //  struct BINDERSECTDESC
  //  {
  //    LPCDATALAYOUT  lpDefine;
  //    GXINT nTopIndex;
  //  };
  //  typedef clvector<COMMONUNIFORM>    CommonUniformArray;
  //  typedef clvector<BINDERSECTDESC>  BinderSectDescArray; // TODO: 以后要储存LPCDATALAYOUT创建出来的对象再做绑定, 直接使用这个不安全

  //private:
  //  GXGraphicsImpl*     m_pGraphicsImpl;
  //  GShaderImpl*        m_pShaderImpl;
  //  CommonUniformArray  m_aCommonUniforms;
  //  BinderSectDescArray m_aBinderSectDesc;

  //private:
  //  GXBOOL        IntSortByBinder            ();   // m_aCommonUniforms 按照Binder索引顺序排序
  //  GXBOOL        IntGenerateBinderTopIndex  ();   // 整理出每个Binder在m_aCommonUniforms中的开始索引
  //  GXBOOL        IntSetUniform              (GXLPCCONSTDESC lpConstDesc, GXDWORD dwHandle, const float* fValue, GXINT nFloatCount, float4* pUnusualUnifom = NULL);
  //  GXBOOL        IntIsTextureHandle         (GXUINT uHandle);
  //public:
  //  GShaderStubImpl(GXGraphics* pGraphics);
  //  GXHRESULT     SetShaderRef            (GShader* pShader);
  //  virtual GXHRESULT     BindData                (MODataPool* pDataPool, GXLPCSTR szStruct);
  //  virtual GXHRESULT     FindDataPoolByName      (GXLPCSTR szName, MODataPool** ppDataPool);
  //  virtual GXHRESULT     BindCommonUniform       (LPCDATALAYOUT lpUniformDef);
  //  virtual GXINT         FindUniformDef          (LPCDATALAYOUT lpUniformDef);
  //  GXBOOL        CommitUniform           (int nDefIdx, GXLPCVOID lpData, GXUINT uCommonOffset);
  //  GShader*      GetShaderUnsafe         () GXCONST;
  //  GXUINT        GetHandleByName         (GXLPCSTR pName) GXCONST;
  //  GXUINT        GetHandleByIndex        (GXUINT nIndex) GXCONST;
  //  GXUniformType GetHandleType           (GXUINT handle) GXCONST;
  //  GXUINT        GetSamplerStageByHandle (GXUINT handle) GXCONST;
  //  GXBOOL        SetUniformByHandle      (clBuffer* pUnusualUnifom, GXUINT uHandle, float* fValue, GXINT nFloatCount);
  //  GXBOOL        GetUniformByIndex       (GXUINT nIndex, UNIFORMDESC* pDesc) GXCONST;
  //  GXBOOL        SetTextureByHandle      (GTexture** pTextureArray, GXUINT uHandle, GTexture* pTexture);


  //  // 接口实现
  //  virtual GXHRESULT AddRef          ();
  //  virtual GXHRESULT Release         ();
  //  virtual GXHRESULT Invoke          (GRESCRIPTDESC* pDesc) { return GX_OK; }

  //  virtual GXHRESULT SetTextureSlot  (GXLPCSTR pName, GXINT nSlot);
  //  virtual GXINT     GetTextureSlot  (GXLPCSTR pName);
  //};
  //////////////////////////////////////////////////////////////////////////


} // namespace D3D11

namespace GrapX
{
  namespace D3D11
  {
    class ShaderImpl : public Shader
    {
      typedef ::D3D11::GXGraphicsImpl GXGraphicsImpl;
      typedef clvector<Marimo::DATAPOOL_VARIABLE_DECLARATION> DataPoolVariableDeclaration_T;

    protected:
      enum class TargetType : GXUINT
      {
        Undefine = 0,
        Vertex,
        Pixel,
      };

      struct INTERMEDIATE_CODE
      {
        typedef clvector<INTERMEDIATE_CODE> Array;
        TargetType type;
        ID3DBlob*  pCode;
        ID3D11ShaderReflection* pReflection;
      };

    protected:
      GXGraphicsImpl* m_pGraphicsImpl;
      ID3D11VertexShader*         m_pD3D11VertexShader;
      ID3D11PixelShader*          m_pD3D11PixelShader;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT    AddRef            () override;
      GXHRESULT    Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT   Invoke             (GRESCRIPTDESC* pDesc) override;
      GXGraphics*  GetGraphicsUnsafe () const override;

      ShaderImpl(GXGraphicsImpl* pGraphicsImpl);
      virtual ~ShaderImpl();

      GXBOOL InitShader(GXLPCWSTR szResourceDir, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount);
      GXBOOL Reflect(ID3D11ShaderReflection* pReflection);
      GXBOOL Reflect_ConstantBuffer(DataPoolVariableDeclaration_T& aArray, ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer, const D3D11_SHADER_BUFFER_DESC& buffer_desc);

      static TargetType TargetNameToType  (GXLPCSTR szTargetName);
      static GXHRESULT  CompileShader     (INTERMEDIATE_CODE* pInterCode, const GXSHADER_SOURCE_DESC* pShaderDesc, ID3DInclude* pInclude);
    };
  } // namespace D3D11
} // namespace GrapX

#endif // _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11