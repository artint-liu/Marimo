#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_

//////////////////////////////////////////////////////////////////////////
// ����ShaderConst��Uniform�ĺ���:
// Const��ʾ����,VertexShader �� PixelShader �ֱ�������Ե�Const����
// ������Shader�ϲ�ʹ��ʱ, ͬ��ͬ����(��ͬ����Ӧ�����Ժ��ⱨ��)��Const��ϲ�Ϊһ����¼,
// ���Ϊ Uniform
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
      ID3DBlob*                   m_pD3DVertexInterCode; // �����Ͷ����������а�

      // DataPool ����
      DATAPOOL_DECLARATION*       m_pDataPoolDecl; // ����˳��: $Globals��varA��varB��varC...��������CB���ṹ����ʽ��cb_A a, cb_B b ...
      DATAPOOL_TYPE_DEFINITION*   m_pDataPoolTypeDef;
      BINDRESOURCE_DESC*          m_pBindResourceDesc;
      size_t                      m_nBindResourceDesc;
      clstd::MemBuffer            m_buffer;

      // ��������
      // ͬʱ����VS��PS�ϼ���VS��PS���������������壬
      // ��(�ϼ�:){G,len},{A,len},{B,len},{C,len},{D,len},|��VS:��G,A,D|��PS:��G,B,C
      // ֻ�кϼ��������ü���
      struct D3D11CB_DESC
      {
        ID3D11Buffer* pD3D11ConstantBuffer;
        GXUINT        cbSize;
      };
      clstd::MemBuffer            m_D11ResDescPool; // D3D11 ������
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
      GXBOOL BuildIndexedCBTable(const DATAPOOL_MAPPER& combine, const DATAPOOL_MAPPER* pMapper, clvector<size_t>* pIndexTab); // ��Ϊû�д�С��ֻ����vs����ps CB��ϼ���������ϵ
      GXBOOL BuildCBTable(Marimo::DataPool* pDataPool); // ��һ�δ���Effect����Materialʱ����D3D CB
      GXBOOL CommitConstantBuffer(Marimo::DataPool* pDataPool);
      const BINDRESOURCE_DESC* FindBindResource(GXLPCSTR szName) const;

      GXBOOL BuildDataPoolDecl(DATAPOOL_MAPPER& mapper); // ע���ڲ����޸�mapper
      ID3D11Buffer* D3D11CreateBuffer(D3D11CB_DESC& desc, size_t cbSize);

      void DbgCheck(INTERMEDIATE_CODE::Array& aInterCode);

      GXINT   GetCacheSize() const; // �ɵļ��ݽӿ�
      GXBOOL  CheckUpdateConstBuf (); // �ɵļ��ݽӿ�


      static TargetType TargetNameToType  (GXLPCSTR szTargetName);
      static GXHRESULT  CompileShader     (INTERMEDIATE_CODE* pInterCode, const GXSHADER_SOURCE_DESC* pShaderDesc, ID3DInclude* pInclude);
    };

  } // namespace D3D11
} // namespace GrapX

#endif // _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11