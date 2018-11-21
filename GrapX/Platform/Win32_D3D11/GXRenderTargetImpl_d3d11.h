#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GXRENDERTARGET_D3D11_IMPLEMENT_H_
#define _GXRENDERTARGET_D3D11_IMPLEMENT_H_
class GResource;

namespace D3D11
{
  class GTextureImpl_RenderTarget;
  class GTextureImpl_DepthStencil;
}

namespace GrapX
{
  namespace D3D11
  {
    class GraphicsImpl;

    class RenderTargetImpl : public RenderTarget
    {
      friend class GraphicsImpl;
      friend class GXCanvasCoreImpl;

    protected:
      GraphicsImpl* m_pGraphics;
      GTextureImpl_RenderTarget* m_pColorTexture;
      GTextureImpl_DepthStencil* m_pDepthStencilTexture;
      const GXINT m_nWidth;
      const GXINT m_nHeight;

    public:
      RenderTargetImpl(Graphics* pGraphics, GXINT nWidth, GXINT nHeight);
      virtual ~RenderTargetImpl();

    public:
      GXHRESULT  AddRef                 () override;
      GXHRESULT  Release                () override;
      GXHRESULT  Invoke                 (GRESCRIPTDESC* pDesc) override;

      GXBOOL     GetRatio               (GXSizeRatio* pWidth, GXSizeRatio* pHeight) override;
      GXSIZE*    GetDimension           (GXSIZE* pDimension) override;
      GXHRESULT  GetColorTexture        (GTexture** ppColorTexture, GXResUsage eUsage) override; // 只接受 GXResUsage::Default 或者 GXResUsage::Read
      GTexture*  GetColorTextureUnsafe  (GXResUsage eUsage) override; // 只接受 GXResUsage::Default 或者 GXResUsage::Read
      GXHRESULT  GetDepthStencilTexture (GTexture** ppDepthStencilTexture) override;
      GXBOOL     StretchRect            (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter) override;
      GXBOOL     SaveToFile             (GXLPCWSTR szFilePath, GXLPCSTR pImageFormat) override;
      GXBOOL     SaveToMemory           (clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat) override;


    public:
      GXBOOL Initialize(GXFormat eColorFormat, GXFormat eDepthStencilFormat);
      GTextureImpl_RenderTarget* IntGetColorTextureUnsafe();
      GTextureImpl_DepthStencil* IntGetDepthStencilTextureUnsafe();
    };

  } // namespace D3D11
} // namespace GrapX
#endif // _GXRENDERTARGET_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11