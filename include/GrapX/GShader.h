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
  class BlendState;
  class TextureBase;
  class RasterizerState;
  class DepthStencilState;

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
    GXSTDINTERFACE(const BINDRESOURCE_DESC* GetBindResource(GXUINT nIndex) const);
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
    GXSTDINTERFACE(GXHRESULT    Clone(Effect** ppNewEffect));  // 克隆Effect，不会复制常量
    GXSTDINTERFACE(GXBOOL       SetTexture(GXUINT nSlot, Texture* pTexture));
    GXSTDINTERFACE(GXBOOL       SetTexture(GXLPCSTR szSamplerName, Texture* pTexture));

    //GXSTDINTERFACE(void BindTextureSlot(GXLPCSTR szTextureName, int nSlot));
  };


  // Background   1000
  // Geometry     2000
  // Alpha test   2500
  // Transparent  3000
  // Overlay      4000
  // Max          4095

  // Render queue - 参考Unity3D文档的定义
  enum RenderQueue {
    RenderQueue_Background  = 1000,
    RenderQueue_Geometry    = 2000,
    RenderQueue_AlphaTest   = 2500,
    RenderQueue_Transparent = 3000,
    RenderQueue_Overlay     = 4000,
    RenderQueue_Max         = 4095,
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

    GXSTDINTERFACE(void         SetDepthStencilState(DepthStencilState* pState));
    GXSTDINTERFACE(void         SetRasterizerState(RasterizerState* pState));
    GXSTDINTERFACE(void         SetBlendState(BlendState* pState));
    GXSTDINTERFACE(GXBOOL       SetState(GXLPCSTR szStateCommand)); // 大小写敏感，只识别全小写命令

    GXSTDINTERFACE(Graphics*    GetGraphicsUnsafe () const);
    GXSTDINTERFACE(Marimo::DataPoolVariable GetUniform(GXLPCSTR szName));
    GXSTDINTERFACE(GXBOOL       SetTexture(GXUINT nSlot, Texture* pTexture));
    GXSTDINTERFACE(GXBOOL       SetTexture(GXLPCSTR szSamplerName, Texture* pTexture));

    GXSTDINTERFACE(GXBOOL       SetFloat(GXLPCSTR szName, float value));
    GXSTDINTERFACE(float        GetFloat(GXLPCSTR szName));
    GXSTDINTERFACE(GXBOOL       SetVector(GXLPCSTR szName, float4* pVector));
    GXSTDINTERFACE(GXBOOL       GetVector(float4* pOut, GXLPCSTR szName));

    GXSTDINTERFACE(GXHRESULT  GetFilename         (clStringW* pstrFilename));
    GXSTDINTERFACE(int        SetRenderQueue      (int nRenderQueue));
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