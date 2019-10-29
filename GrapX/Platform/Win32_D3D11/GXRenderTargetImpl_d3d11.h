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

    template<class _TargetInterfaceT>
    class RenderTargetBase : public _TargetInterfaceT
    {
    protected:
      GraphicsImpl* m_pGraphics;
      TextureImpl_RenderTarget* m_pColorTexture = NULL;
      TextureImpl_DepthStencil* m_pDepthStencilTexture = NULL;

    public:
      RenderTargetBase(GraphicsImpl* pGraphics);
      RenderTargetBase(GraphicsImpl* pGraphics, TextureImpl_RenderTarget* pColorTexture, TextureImpl_DepthStencil* pDepthTexture); // 只组装，后面没有创建
      virtual ~RenderTargetBase();
      //GXBOOL Initialize(GXDWORD dwResType, GXUINT width, GXUINT height, GXFormat eColorFormat, GXFormat eDepthStencilFormat);

    protected:
      GXBOOL InitDepthStencil(GXFormat eDepthStencilFormat, GXUINT nWidth, GXUINT nHeight);
    };

    //////////////////////////////////////////////////////////////////////////

    class RenderTargetImpl : public RenderTargetBase<RenderTarget>
    {
      friend class GraphicsImpl;
      friend class CanvasCoreImpl;

    protected:
      TextureImpl_GPUReadBack*  m_pReadBackTexture = NULL;
      const GXINT m_nWidth;
      const GXINT m_nHeight;

    public:
      RenderTargetImpl(Graphics* pGraphics, GXINT nWidth, GXINT nHeight);
      RenderTargetImpl(Graphics* pGraphics, GXINT nWidth, GXINT nHeight, TextureImpl_RenderTarget* pColorTexture, TextureImpl_DepthStencil* pDepthTexture); // 只组装，后面没有创建

      virtual ~RenderTargetImpl();

    public:
      GXHRESULT  AddRef                 () override;
      GXHRESULT  Release                () override;
      GXHRESULT  Invoke                 (GRESCRIPTDESC* pDesc) override;

      GXBOOL     GetRatio               (GXSizeRatio* pWidth, GXSizeRatio* pHeight) override;
      GXSIZE*    GetDimension           (GXSIZE* pDimension) override;
      GXHRESULT  GetColorTexture        (Texture** ppColorTexture, GXResUsage eUsage) override; // 只接受 GXResUsage::Default 或者 GXResUsage::Read
      Texture*   GetColorTextureUnsafe  (GXResUsage eUsage) override; // 只接受 GXResUsage::Default 或者 GXResUsage::Read
      GXHRESULT  GetDepthStencilTexture (Texture** ppDepthStencilTexture) override;
      GXBOOL     StretchRect            (Texture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter) override;
      GXBOOL     SaveToFile             (GXLPCWSTR szFilePath, GXLPCSTR pImageFormat, GXBOOL bVertFlip) override;
      GXBOOL     SaveToMemory           (clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip) override;


    public:
      GXBOOL Initialize(GXFormat eColorFormat, GXFormat eDepthStencilFormat);
      TextureImpl_RenderTarget* IntGetColorTextureUnsafe();
      TextureImpl_DepthStencil* IntGetDepthStencilTextureUnsafe();

    protected:
      GXBOOL IntCreateReadBackTexture(TextureImpl_GPUReadBack** ppReadBackTex);

    };

    class CubeRenderTargetImpl : public RenderTargetBase<CubeRenderTarget>
    {
      friend class GraphicsImpl;
      typedef TextureImpl_RenderTarget CubeFaceRenderTargetTextureImpl;
    protected:
      CubeFaceRenderTargetTextureImpl* m_pCubeFace[6] = { NULL };
      RenderTargetImpl* m_pRenderTargetFace[countof(m_pCubeFace)] = { NULL };
      //u8                m_CubeFaceTextureBuffer[countof(m_pCubeFace) * sizeof(CubeFaceRenderTargetTextureImpl)];
      //u8                m_CubeFaceRenderTargetBuffer[countof(m_pCubeFace) * sizeof(RenderTargetImpl)];

      TextureImpl_GPUReadBack*  m_pReadBackTexture = NULL;

    protected:
      CubeRenderTargetImpl(Graphics* pGraphics);
      virtual ~CubeRenderTargetImpl();

      GXHRESULT  AddRef                 () override;
      GXHRESULT  Release                () override;
      GXHRESULT  Invoke                 (GRESCRIPTDESC* pDesc) override;

      GXBOOL Initialize(GXUINT nSize, GXFormat eColorFormat, GXFormat eDepthStencilFormat);

      RenderTarget*  GetFaceUnsafe(Face face) override;
      RenderTarget** GetFacesUnsafe() override;
    };

    //////////////////////////////////////////////////////////////////////////

    class RenderTargetImpl_BackBuffer : public RenderTargetImpl
    {
    public:
      GXHRESULT  Release                () override;

      RenderTargetImpl_BackBuffer(Graphics* pGraphics);
      GXBOOL InitializeWithSwapChain(IDXGISwapChain* pSwapChain);
    };

  } // namespace D3D11
} // namespace GrapX
#endif // _GXRENDERTARGET_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11