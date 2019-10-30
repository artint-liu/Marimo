#ifndef _GRAPX_RENDERTARGET_H_
#define _GRAPX_RENDERTARGET_H_

namespace GrapX
{
  class RenderTarget : public GResource
  {
  public:
    RenderTarget() : GResource(0, ResourceType::RenderTarget) {}

    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

  public:
    GXSTDINTERFACE(GXBOOL     GetRatio              (GXSizeRatio* pWidth, GXSizeRatio* pHeight));
    GXSTDINTERFACE(GXSIZE*    GetDimension          (GXSIZE* pDimension));
    // GetColorTexture
    // GXResUsage::Default 返回内部用于渲染的纹理，引用计数加一
    // GXResUsage::Read    创建一个用于CPU回读的纹理
    GXSTDINTERFACE(GXHRESULT  GetColorTexture       (Texture** ppColorTexture, GXResUsage eUsage)); // 只接受 GXResUsage::Default 或者 GXResUsage::Read
    // GetColorTextureUnsafe
    // GXResUsage::Default 返回内部用于渲染的纹理，不会改变引用计数
    // GXResUsage::Read    返回一个用于CPU回读的纹理，每次调用会把RenderTarget内容刷新到回读纹理上，多次调用返回相同的纹理对象。
    GXSTDINTERFACE(Texture*   GetColorTextureUnsafe (GXResUsage eUsage)); // 只接受 GXResUsage::Default 或者 GXResUsage::Read
    GXSTDINTERFACE(GXHRESULT  GetDepthStencilTexture(Texture** ppDepthStencilTexture));
    GXSTDINTERFACE(GXBOOL     StretchRect           (Texture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter));
    GXSTDINTERFACE(GXBOOL     SaveToFile            (GXLPCWSTR szFilePath, GXLPCSTR pImageFormat, GXBOOL bVertFlip));
    GXSTDINTERFACE(GXBOOL     SaveToMemory          (clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip));
  };

  class CubeRenderTarget : public GResource
  {
  public:
    enum class Face: int
    {
      PositiveX,
      NegativeX,
      PositiveY,
      NegativeY,
      PositiveZ,
      NegativeZ,
    };
  public:
    CubeRenderTarget() : GResource(2, ResourceType::CubeRenderTarget) {}

    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

  public:
    GXSTDINTERFACE(RenderTarget*  GetFaceUnsafe(Face face));
    GXSTDINTERFACE(RenderTarget** GetFacesUnsafe());
    GXSTDINTERFACE(TextureCube*   GetTextureCubeUnsafe());
    GXSTDINTERFACE(GXBOOL         GetTextureCube(TextureCube** ppTextureCube));

  };
} // namespace GrapX


#endif // _GRAPX_RENDERTARGET_H_
