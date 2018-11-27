#ifndef _SHADER_CLASS_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_HEADER_DEFINE_FILE_

//////////////////////////////////////////////////////////////////////////
// 关于ShaderConst和Uniform的含义:
// Const表示常量,VertexShader 和 PixelShader 分别包含各自的Const常量
// 当两个Shader合并使用时, 同名同长度(不同长度应该在以后检测报错)的Const会合
// 并为一个记录, 这称为 Uniform
// Canvas3D 可以将指定名字的Uniform赋予全局属性，使其在Canvas3D中具有一致的值，
// 这称为 Canvas-Uniform

// GShader
// GXEffect = GShader + buffer
// GXMaterial = GXEffect + render state

namespace Marimo {
  class DataPool;
  class DataPoolVariable;
  struct DATAPOOL_MANIFEST;
} // namespace Marimo
typedef Marimo::DataPool MODataPool;

//class GXGraphics;
namespace clstd
{
  class SmartStockA;
} // namespace clstd
class clConstBuffer;
//class SmartRepository;
struct MOSHADER_ELEMENT_SOURCE;
struct MTLFILEPARAMDESC;
struct STANDARDMTLUNIFORMTABLE;

namespace GrapX
{
  class TextureBase;
  class GVertexDeclaration : public GResource
  {
  public:
    GVertexDeclaration() : GResource(0, RESTYPE_VERTEX_DECLARATION) {}

    GXSTDINTERFACE(GXHRESULT          AddRef            ());
    GXSTDINTERFACE(GXHRESULT          Release           ());
    GXSTDINTERFACE(GXUINT             GetStride         ());  // 顶点格式所需要的最小长度
    GXSTDINTERFACE(GXINT              GetElementOffset  (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc = NULL));
    GXSTDINTERFACE(GXLPCVERTEXELEMENT GetVertexElement  ());
    // TODO: 将来实现 GXSTDINTERFACE(GXINT        GetOffset         (GXIN GXDeclUsage Usage, GXIN GXUINT UsageIndex, GXOUT LPGXVERTEXELEMENT lpDesc = NULL));
    // TODO: 将来实现 GXSTDINTERFACE(GXUINT       GetElementCount   ());
  };

  class GUniformBinderDecl : public GResource
  {
  public:
    GUniformBinderDecl() : GResource(0, RESTYPE_UNKNOWN) {}

    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());
  };

  //////////////////////////////////////////////////////////////////////////

  class Shader : public GResource
  {
  public:
    enum class BindType : GXUINT
    {
      Texture = 0,
      Sampler = 1,
    };

    struct BINDRESOURCE_DESC
    {
      GXLPCSTR  name;
      BindType  type;
      int       slot;
    };

  public:
    Shader() : GResource(0, RESTYPE_SHADER) {}
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXSTDINTERFACE(Graphics*    GetGraphicsUnsafe       () const);
    GXSTDINTERFACE(void         GetDataPoolDeclaration  (Marimo::DATAPOOL_MANIFEST* pManifest) const);
    GXSTDINTERFACE(const BINDRESOURCE_DESC* FindBindResource(GXLPCSTR szName) const);
  };

  class Effect : public GResource
  {
  public:
    Effect() : GResource(0, RESTYPE_SHADER_EFFECT) {}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXSTDINTERFACE(Graphics*    GetGraphicsUnsafe () const);
    GXSTDINTERFACE(Marimo::DataPoolVariable GetUniform(GXLPCSTR szName));

    GXSTDINTERFACE(void BindTextureSlot(GXLPCSTR szTextureName, int nSlot));
  };

  class Material : public GResource
  {
  public:
    enum ParamType // 旧兼容接口
    {
      PT_UNIFORMS,
      PT_RENDERSTATE,
    };

  public:
    Material() : GResource(0, RESTYPE_SHADER_MATERIAL) {}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXSTDINTERFACE(Graphics*    GetGraphicsUnsafe () const);

    GXSTDINTERFACE(GXHRESULT  GetFilename         (clStringW* pstrFilename));
    GXSTDINTERFACE(int        GetRenderQueue      () const);

    // 旧兼容接口
    GXSTDINTERFACE(GXHRESULT  SetParameters       (ParamType eType, GXDEFINITION* pParameters, int nCount = 0));  // 更新列表中的参数值, 如果nCount为0, 则列表的结尾必须以空字符串或者NULL结尾
    GXSTDINTERFACE(GXHRESULT  BindDataByName              (GXLPCSTR szPoolName, GXLPCSTR szStruct));
  };

} // namespace GrapX

//////////////////////////////////////////////////////////////////////////

// 根据绑定声明生成声明源代码
GXBOOL MOGenerateDeclarationCodes(DATALAYOUT* lpDataLayout, GXDWORD dwPlatfomCode, clBuffer** ppBuffer);

#endif // end of _SHADER_CLASS_HEADER_DEFINE_FILE_