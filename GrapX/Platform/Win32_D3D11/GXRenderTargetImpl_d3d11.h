#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GXRENDERTARGET_D3D11_IMPLEMENT_H_
#define _GXRENDERTARGET_D3D11_IMPLEMENT_H_

namespace D3D11
{
  class GResource;
  class GXGraphicsImpl;
  class GTextureImpl_RenderTarget;
  class GTextureImpl_DepthStencil;

  class GXRenderTargetImpl : public GXRenderTarget
  {
    friend class GXGraphicsImpl;
    friend class GXCanvasCoreImpl;

  protected:
    GXGraphics* m_pGraphics;
    GTextureImpl_RenderTarget* m_pColorTexture;
    GTextureImpl_DepthStencil* m_pDepthStencilTexture;

  public:
    GXRenderTargetImpl(GXGraphics* pGraphics);
    virtual ~GXRenderTargetImpl();

  public:
    GXHRESULT  AddRef            () override;
    GXHRESULT  Release           () override;

    GXBOOL     GetRatio              (GXSizeRatio* pWidth, GXSizeRatio* pHeight) override;
    GXSIZE*    GetDimension          (GXSIZE* pDimension) override;
    GXHRESULT  GetColorTexture       (GTexture** ppColorTexture, GXResUsage eUsage) override; // ֻ���� GXResUsage::Default ���� GXResUsage::Read
    GTexture*  GetColorTextureUnsafe (GXResUsage eUsage) override; // ֻ���� GXResUsage::Default ���� GXResUsage::Read
    GXHRESULT  GetDepthStencilTexture(GTexture** ppDepthStencilTexture) override;
    GXBOOL     StretchRect           (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter) override;
    GXBOOL     SaveToFile            (GXLPCWSTR szFilePath, GXLPCSTR pImageFormat) override;
    GXBOOL     SaveToMemory          (clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat) override;


  public:
    GTextureImpl_RenderTarget* IntGetColorTextureUnsafe();
    GTextureImpl_DepthStencil* IntGetDepthStencilTextureUnsafe();
  };

  class GTextureImpl_RenderTarget : public GTexureBaseImplT<GTexture>
  {
    friend class GXGraphicsImpl;
  protected:
    ID3D11RenderTargetView* m_pD3D11RenderTargetView;
  };

  class GTextureImpl_DepthStencil : public GTexureBaseImplT<GTexture>
  {
    friend class GXGraphicsImpl;
  protected:
    ID3D11DepthStencilView* m_pD3D11DepthStencilView;
  };

} // namespace D3D11
#endif // _GXRENDERTARGET_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11