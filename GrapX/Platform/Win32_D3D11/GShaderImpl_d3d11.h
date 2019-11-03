#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D11_IMPLEMENT_HEADER_DEFINE_FILE_
struct STD_CANVAS_UNIFORM;

#define CANVAS_COMMON_MARK_PTR  (ID3D11Buffer*)-100

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
    class VertexDeclImpl;
    struct DEVICECONTEXT;

    class ShaderImpl : public Shader
    {
      friend class GraphicsImpl;
      typedef Marimo::DATAPOOL_DECLARATION      DATAPOOL_DECLARATION;
      typedef Marimo::DATAPOOL_TYPE_DEFINITION  DATAPOOL_TYPE_DEFINITION;
      typedef clmap<clStringA, ID3D11InputLayout*> InputLayoutDict;
    public:
      struct D3D11CB_DESC
      {
        //ID3D11Buffer* pD3D11ConstantBuffer;
        GXUINT        cbSize : 24;
        GXUINT        type : 8;  // ���cb_MarimoCommon
      };
      STATIC_ASSERT(sizeof(D3D11CB_DESC) == sizeof(GXINT));

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
      GraphicsImpl*               m_pGraphicsImpl;
      ID3D11VertexShader*         m_pD3D11VertexShader;
      ID3D11PixelShader*          m_pD3D11PixelShader;
      ID3DBlob*                   m_pD3DVertexInterCode;  // �����Ͷ����������а�
      InputLayoutDict             m_InputLayoutDict;      // ����������shaderӳ���ϵ�����

      // DataPool ����
      clstd::MemBuffer            m_buffer;
      DATAPOOL_DECLARATION*       m_pDataPoolDecl; // ����˳��: $Globals��varA��varB��varC...��������CB���ṹ����ʽ��cb_A a, cb_B b ...
      DATAPOOL_TYPE_DEFINITION*   m_pDataPoolTypeDef;
      BINDRESOURCE_DESC*          m_pBindResourceDesc;
      size_t                      m_nBindResourceDesc;

      // ��������
      // ͬʱ����VS&PS�ϼ���VS��PS���������������壬�ֳ�������
      // ��:(�ϼ�����){G:len,type},{A:len,type},{B:len,type},{C:len,type},{D:len,type},
      //   ��VS����index��G,A,D|��PS����index��G,B,C
      // D3D11 DESC ������, ȫ�����Ͷ���GXINT, �ϼ�����ӳ��ΪD3D11CB_DESCʹ��
      // D3D11 CB�ϼ���, ȫ�����Ͷ���ID3D11Buffer*, �ϼ�����ά������
      GXUINT                      m_nD3DCBPoolSize = 0;   // D3D11 CB �سߴ�
      clvector<GXINT>             m_D11ResDescPool;       // D3D11 DESC ������
      GXINT                       m_nVertexCBOffset = 0;
      GXINT                       m_nPixelCBOffset = 0;

      //ID3D11Buffer**              m_pVertexCB = NULL;
      //ID3D11Buffer**              m_pPixelCB = NULL;

      int                         m_nCanvasUniformIndex[2] = { -1, -1 };
      Marimo::DataPool*           m_pMainDataPool = NULL;

      //D3D11CB_DESC* D3D11CB_GetDescBegin() const;
      //D3D11CB_DESC* D3D11CB_GetDescEnd() const;
      //ID3D11Buffer** D3D11CB_GetPixelCBEnd() const;

    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT    AddRef            () override;
      GXHRESULT    Release           () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT   Invoke             (GRESCRIPTDESC* pDesc) override;
      Graphics*   GetGraphicsUnsafe () const override;
      void        GetDataPoolDeclaration  (Marimo::DATAPOOL_MANIFEST* pManifest) const override;
      GXBOOL      GetDataPool             (Marimo::DataPool** ppReferenceDataPool) const override;
      GXBOOL      ReleaseDeviceDependBuffer(clstd::MemBuffer& sD3DCBPool) const override;

      ShaderImpl(GraphicsImpl* pGraphicsImpl);
      virtual ~ShaderImpl();

      GXBOOL InitShader(GXLPCWSTR szResourceDir, const GXSHADER_SOURCE_DESC* pShaderDescs, GXUINT nCount);
      GXBOOL Reflect(DATAPOOL_MAPPER& decl_mapper, ID3D11ShaderReflection* pReflection);
      GXBOOL Reflect_ConstantBuffer(DataPoolDeclaration_T& aArray, DATAPOOL_MAPPER& aStructDesc, ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer, const D3D11_SHADER_BUFFER_DESC& buffer_desc);
      GXLPCSTR Reflect_MakeTypename(DATAPOOL_MAPPER& aStructDesc, D3D11_SHADER_TYPE_DESC& type_desc, ID3D11ShaderReflectionType* pReflectionType);

      GXBOOL Activate(DEVICECONTEXT* pContext);
      GXBOOL BuildIndexedCBTable(const DATAPOOL_MAPPER& combine, const DATAPOOL_MAPPER* pMapper, clvector<size_t>* pIndexTab); // ��Ϊû�д�С��ֻ����vs����ps CB��ϼ���������ϵ
      static void   SetCBDesc(D3D11CB_DESC& desc, GXLPCSTR name, size_t cbSize);
      GXBOOL GenerateCBTable();
      GXBOOL BuildCBTable(clstd::MemBuffer& sD3DCBPool) const;
      //GXBOOL ReleaseCBTable(clstd::MemBuffer& sD3DCBPool) const;

      void UploadConstBuffer(DEVICECONTEXT* pContext, clstd::MemBuffer* pD3DCBPool, Marimo::DataPool* pDataPool);
      GXBOOL CommitConstantBuffer(DEVICECONTEXT* pContext, const clstd::MemBuffer* pD3DCBPool, ID3D11Buffer* pCanvasUniform = NULL);
      const BINDRESOURCE_DESC* GetBindResource(GXUINT nIndex) const;
      const BINDRESOURCE_DESC* FindBindResource(GXLPCSTR szName) const;

      GXBOOL BuildDataPoolDecl(DATAPOOL_MAPPER& mapper); // ע���ڲ����޸�mapper
      //ID3D11Buffer* D3D11CreateBuffer(ID3D11Device* pd3dDevice, D3D11CB_DESC& desc, GXLPCSTR szName, size_t cbSize) const;
      ID3D11InputLayout* D3D11GetInputLayout(VertexDeclImpl* pVertexDecl);
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