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
    // GXResUsage::Default �����ڲ�������Ⱦ���������ü�����һ
    // GXResUsage::Read    ����һ������CPU�ض�������
    GXSTDINTERFACE(GXHRESULT  GetColorTexture       (Texture** ppColorTexture, GXResUsage eUsage)); // ֻ���� GXResUsage::Default ���� GXResUsage::Read
    // GetColorTextureUnsafe
    // GXResUsage::Default �����ڲ�������Ⱦ����������ı����ü���
    // GXResUsage::Read    ����һ������CPU�ض�������ÿ�ε��û��RenderTarget����ˢ�µ��ض������ϣ���ε��÷�����ͬ���������
    GXSTDINTERFACE(Texture*   GetColorTextureUnsafe (GXResUsage eUsage)); // ֻ���� GXResUsage::Default ���� GXResUsage::Read
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
