#ifndef _GRAPX_RESOURCE_H_
#define _GRAPX_RESOURCE_H_


#define INVOKE_DEVICE_EVENT(RESCMD) {\
GRESCRIPTDESC Desc;\
InlSetZeroT(Desc);\
Desc.dwCmdCode = RESCMD;\
Invoke(&Desc);}


#define INVOKE_LOST_DEVICE    INVOKE_DEVICE_EVENT(RC_LostDevice)
#define INVOKE_RESET_DEVICE   INVOKE_DEVICE_EVENT(RC_ResetDevice)
#define INVOKE_RESIZE_DEVICE  INVOKE_DEVICE_EVENT(RC_ResizeDevice)

#define INVOKE_DESC_CHECK(pDesc)\
if(pDesc->szCmdString != NULL && pDesc->dwCmdCode != NULL)\
{ CLOG_ERROR("%s : Bad script desc param.\n", __FUNCTION__); return GX_FAIL; }

namespace GrapX
{
  enum class ResourceType : unsigned int
  {
    Unknown,            // [优先级]
    Shader,             //    0
    ShaderStub,         //    1
    ShaderEffect,       //    2
    ShaderMaterial,     //    2
    RasterizerState,    //    0
    BlendState,         //    0
    DepthStencilState,  //    0
    SamplerState,       //    0
    VertexDeclaration,  //    0
    RenderTexture,      //    0
    CubeRenderTexture,  //    0
    DepthStenciltexture,//    0
    Texture2D,          //    0
    Texture3D,          //    0
    TextureCube,        //    0
    Primitive,          //    0
    //RESTYPE_INDEXED_PRIMITIVE,  //    0
    Region,             //    -

    Font,               //    1
    Image,              //    1
    RenderTarget,       //    1
    Graphics,           //    0
    CubeRenderTarget,   //    2
    Canvas2D,           //    2
    Canvas3D,           //    2
  };
} // namespace GrapX

struct GRESKETCH // resource sketch
{
  GXDWORD   dwCategoryId;     // 类别ID, 类别字符串的Hash值或者MakFourCC值
  clStringW strResourceName;  // 资源ID, 资源类自己计算的特定识别符, 空表示该类不以ID区分对象
};

struct GRESCRIPTDESC
{
  GXLPCSTR  szCmdString;  // 命令字符串, 建议使用小写, 和 dwCmdCode 互斥
  GXDWORD   dwCmdCode;    // 命令代码, 和 szCmdString 任选其一使用, 使用前应该检测两者不能同时非0
  GXBOOL    bBroadcast;   // 是否为广播命令
  GXDWORD   dwTime;       // 发送时间, 这个根据Cmd实际需求选择是否填写
  GXLPCSTR  szParameters; // Parameter 按照不同命令作用不同
  GXWPARAM  wParam;
  GXLPARAM  lParam;
};

typedef const GRESKETCH* LPCRESKETCH;
namespace GrapX
{
  class GResource : public GUnknown
  {
  public:
  protected:
    GResource(GXUINT nPriority, ResourceType eType)
      : GUnknown()
      , m_dwPriority(nPriority)
      , m_eResType(eType)
    {
      ASSERT(nPriority < 4);
      ASSERT((int)eType < 256);
    }
    virtual ~GResource() {}

  public:
    GXDWORD         m_dwPriority : 2; // 值越小优先级越高, 在广播时越先处理
    ResourceType    m_eResType : 8;
    GXSTDINTERFACE(GXHRESULT  Invoke            (GRESCRIPTDESC* pDesc));

    virtual GXUINT  GetPriority () const
    {
      return (GXUINT)m_dwPriority;
    }

    virtual GXUINT  SetPriority (GXUINT nNewPriority)
    {
      GXUINT nPrevPriority = (GXUINT)m_dwPriority;
      m_dwPriority = (GXDWORD)nNewPriority;
      return nPrevPriority;
    }

    virtual ResourceType GetType() const
    {
      return (ResourceType)m_eResType;
    }
  };

} // namespace GrapX

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPX_RESOURCE_H_