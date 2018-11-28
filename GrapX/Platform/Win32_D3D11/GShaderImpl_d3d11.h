#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_

//////////////////////////////////////////////////////////////////////////
// 关于ShaderConst和Uniform的含义:
// Const表示常量,VertexShader 和 PixelShader 分别包含各自的Const常量
// 当两个Shader合并使用时, 同名同长度(不同长度应该在以后检测报错)的Const会合并为一个记录,
// 这称为 Uniform
namespace GrapX
{
  namespace D3D11
  {
    typedef clvector<Marimo::DATAPOOL_DECLARATION>      DataPoolDeclaration_T;
    typedef clvector<Marimo::DATAPOOL_TYPE_DEFINITION>  DataPoolTypeDefinition_T;
    struct DATAPOOL_MAPPER;

    class ShaderImpl : public Shader
    {
      friend class GraphicsImpl;
      typedef Marimo::DATAPOOL_DECLARATION      DATAPOOL_DECLARATION;
      typedef Marimo::DATAPOOL_TYPE_DEFINITION  DATAPOOL_TYPE_DEFINITION;
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
      GraphicsImpl* m_pGraphicsImpl;
      ID3D11VertexShader*         m_pD3D11VertexShader;
      ID3D11PixelShader*          m_pD3D11PixelShader;
      ID3DBlob*                   m_pD3DVertexInterCode; // 用来和顶点声明进行绑定

      // DataPool 声明
      DATAPOOL_DECLARATION*       m_pDataPoolDecl; // 变量顺序: $Globals（varA，varB，varC...），各种CB（结构体形式）cb_A a, cb_B b ...
      DATAPOOL_TYPE_DEFINITION*   m_pDataPoolTypeDef;
      BINDRESOURCE_DESC*          m_pBindResourceDesc;
      size_t                      m_nBindResourceDesc;
      clstd::MemBuffer            m_buffer;

      // 常量缓冲
      // 同时储存VS，PS合集，VS，PS独立连续常量缓冲，
      // 如(合集:){G,len},{A,len},{B,len},{C,len},{D,len},|（VS:）G,A,D|（PS:）G,B,C
      // 只有合集遵守引用计数
      struct D3D11CB_DESC
      {
        ID3D11Buffer* pD3D11ConstantBuffer;
        GXUINT        cbSize;
      };
      clstd::MemBuffer            m_D11ResDescPool; // D3D11 描述池
      ID3D11Buffer**              m_pVertexCB;
      ID3D11Buffer**              m_pPixelCB;

      D3D11CB_DESC* D3D11CB_GetDescBegin() const;
      D3D11CB_DESC* D3D11CB_GetDescEnd() const;
      ID3D11Buffer** D3D11CB_GetPixelCBEnd() const;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT    AddRef            () override;
      GXHRESULT    Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT   Invoke             (GRESCRIPTDESC* pDesc) override;
      Graphics*   GetGraphicsUnsafe () const override;
      void        GetDataPoolDeclaration  (Marimo::DATAPOOL_MANIFEST* pManifest) const override;

      ShaderImpl(GraphicsImpl* pGraphicsImpl);
      virtual ~ShaderImpl();

      GXBOOL InitShader(GXLPCWSTR szResourceDir, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount);
      GXBOOL Reflect(DATAPOOL_MAPPER& decl_mapper, ID3D11ShaderReflection* pReflection);
      GXBOOL Reflect_ConstantBuffer(DataPoolDeclaration_T& aArray, DATAPOOL_MAPPER& aStructDesc, ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer, const D3D11_SHADER_BUFFER_DESC& buffer_desc);
      GXLPCSTR Reflect_MakeTypename(DATAPOOL_MAPPER& aStructDesc, D3D11_SHADER_TYPE_DESC& type_desc, ID3D11ShaderReflectionType* pReflectionType);

      GXBOOL Activate();
      GXBOOL BuildIndexedCBTable(const DATAPOOL_MAPPER& combine, const DATAPOOL_MAPPER* pMapper, clvector<size_t>* pIndexTab); // 因为没有大小，只生成vs或者ps CB与合集的索引关系
      GXBOOL BuildCBTable(Marimo::DataPool* pDataPool); // 第一次创建Effect或者Material时创建D3D CB
      GXBOOL CommitConstantBuffer(Marimo::DataPool* pDataPool);
      const BINDRESOURCE_DESC* FindBindResource(GXLPCSTR szName) const;

      GXBOOL BuildDataPoolDecl(DATAPOOL_MAPPER& mapper); // 注意内部会修改mapper
      ID3D11Buffer* D3D11CreateBuffer(D3D11CB_DESC& desc, size_t cbSize);

      void DbgCheck(INTERMEDIATE_CODE::Array& aInterCode);

      GXINT   GetCacheSize() const; // 旧的兼容接口
      GXBOOL  CheckUpdateConstBuf (); // 旧的兼容接口


      static TargetType TargetNameToType  (GXLPCSTR szTargetName);
      static GXHRESULT  CompileShader     (INTERMEDIATE_CODE* pInterCode, const GXSHADER_SOURCE_DESC* pShaderDesc, ID3DInclude* pInclude);
    };

  } // namespace D3D11
} // namespace GrapX

#endif // _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11