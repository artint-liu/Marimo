#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE_D3D11_IMPLEMENT_H_
#define _GTEXTURE_D3D11_IMPLEMENT_H_

#define ENABLE_DirectXTex

namespace GrapX
{
  namespace D3D11
  {
    class GResource;
    class GraphicsImpl;

    GXUINT CalculateNumOfMipLevels(GXUINT nWidth, GXUINT nHeight); // 根据长宽计算MipMap数量

    // 用来统一内部TextureImpl对象
    template<class _Interface>
    class TextureBaseImplT : public _Interface
    {
    protected:
      GraphicsImpl*             m_pGraphics;
      ID3D11Texture2D*          m_pD3D11Texture;
      ID3D11ShaderResourceView* m_pD3D11ShaderView;

    public:
      TextureBaseImplT(GraphicsImpl*pGraphics)
        : m_pGraphics(pGraphics)
        , m_pD3D11Texture(NULL)
        , m_pD3D11ShaderView(NULL)
      {};

      virtual GXHRESULT AddRef () = NULL;
      virtual GXHRESULT Release() = NULL;

      inline ID3D11ShaderResourceView*& D3DResourceView()
      {
        return m_pD3D11ShaderView;
      }

      inline ID3D11Texture2D* D3DTexture() const
      {
        return m_pD3D11Texture;
      }
    };

    class TextureImpl : public TextureBaseImplT<Texture>
    {
      friend class GraphicsImpl;
      friend class CanvasCoreImpl;

    public:
      //enum CREATETYPE
      //{
      //  CreationFailed    = -1, // 创建失败的
      //  Invalid           = 0,
      //  User              = 1,
      //  File              = 2,
      //  FileEx            = 3,
      //  Resource          = 4,
      //  ResourceEx        = 5,
      //  OffscreenPlainSur = 6,
      //  D3DSurfaceRef     = 7,
      //  LastType
      //};
    protected:
      virtual GXHRESULT   Invoke        (GRESCRIPTDESC* pDesc);
    public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT   AddRef        () override;
      GXHRESULT   Release       () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      GXBOOL      Clear             (GXCOLOR dwColor) override;
      //GXBOOL      GetRatio          (GXSizeRatio* pWidthRatio, GXSizeRatio* pHeightRatio) override;
      GXSIZE*     GetDimension      (GXSIZE* pDimension) override;
      GXResUsage  GetUsage          () override;
      GXFormat    GetFormat         () override;
      GXVOID      GenerateMipMaps   () override;
      GXBOOL      GetDesc           (GXBITMAP*lpBitmap) override;
      GXBOOL      CopyRect          (Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource) override;
      GXBOOL      Map               (MAPPED* pMappedRect, GXResMap eResMap) override;
      GXBOOL      Unmap             () override;
      GXBOOL      UpdateRect        (GXLPCRECT prcDest, GXLPVOID pData, GXUINT nPitch) override;
      GrapX::Graphics* GetGraphicsUnsafe () override;
      GXBOOL      SaveToMemory      (clstd::MemBuffer* pBuffer, GXLPCSTR szDestFormat) override;
      GXBOOL      SaveToFile        (GXLPCWSTR szFileName, GXLPCSTR szDestFormat) override;
    protected:
      TextureImpl(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight, GXUINT nMipLevels, GXResUsage eResUsage);
      virtual ~TextureImpl();

      GXBOOL InitTexture(GXBOOL bRenderTarget, GXLPCVOID pInitData, GXUINT nPitch);
      //void   CalcTextureActualDimension();  // TODO: D3D9 也提出这个函数
      //GXBOOL           IntGetHelpTexture();
      //ID3D11Texture2D* IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData);
      GXBOOL IntD3D11CreateResource(GXBOOL bRenderTarget, GXLPCVOID pInitData, GXUINT nPitch);
      GXUINT GetMinPitchSize() const;

      GXBOOL IntSaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat);

    protected:
      D3D11_MAPPED_SUBRESOURCE  m_sMappedResource;
      GXLPBYTE                  m_pTextureData;

      const GXUINT              m_nMipLevels;
      const GXFormat            m_Format;
      const GXResUsage          m_eResUsage;
      const GXUINT              m_nWidth;
      const GXUINT              m_nHeight;

    };

    //LPDIRECT3DTEXTURE9 GTextureImpl::D3DTexture()
    //{
    //  return m_pTexture;
    //}
    //LPDIRECT3DSURFACE9 GTextureImpl::D3DSurface()
    //{
    //  return m_pSurface;
    //}


    //////////////////////////////////////////////////////////////////////////

    class TextureImpl_RenderTarget : public TextureImpl
    {
      friend class GraphicsImpl;
    protected:
      ID3D11RenderTargetView* m_pD3D11RenderTargetView;

    public:
      TextureImpl_RenderTarget(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight);
      virtual ~TextureImpl_RenderTarget();
      GXBOOL InitRenderTexture(ID3D11Texture2D* pD3D11Texture);
      ID3D11RenderTargetView* D3DGetRenderTargetView() const;
    };

    //////////////////////////////////////////////////////////////////////////

    class TextureImpl_DepthStencil : public TextureImpl
    {
      friend class GraphicsImpl;
    protected:
      ID3D11DepthStencilView* m_pD3D11DepthStencilView;

    public:
      TextureImpl_DepthStencil(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight);
      virtual ~TextureImpl_DepthStencil();
      GXBOOL InitDepthStencil();
    };

    //////////////////////////////////////////////////////////////////////////

    class TextureImpl_GPUReadBack : public TextureImpl
    {
    protected:
    public:
      TextureImpl_GPUReadBack(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight);
      GXBOOL InitReadBackTexture();

      GXBOOL Map    (MAPPED* pMappedRect, GXResMap eResMap) override;
      GXBOOL Unmap  () override;
    };

    //////////////////////////////////////////////////////////////////////////

  } // namespace D3D11
} // namespace GrapX

#endif // _GTEXTURE_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11