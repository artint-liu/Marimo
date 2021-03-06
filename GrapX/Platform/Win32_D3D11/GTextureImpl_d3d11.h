#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE_D3D11_IMPLEMENT_H_
#define _GTEXTURE_D3D11_IMPLEMENT_H_

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
      GraphicsImpl*             m_pGraphicsImpl = NULL;
      ID3D11Texture2D*          m_pD3D11Texture = NULL;
      ID3D11ShaderResourceView* m_pD3D11ShaderView = NULL;

      const GXUINT              m_nMipLevels = 0;
      const GXFormat            m_Format     = Format_Unknown;
      const GXResUsage          m_eResUsage  = GXResUsage::Default;

    public:
      TextureBaseImplT(GraphicsImpl* pGraphicsImpl, GXFormat eFormat, GXUINT nMipLevels, GXResUsage eResUsage)
        : m_pGraphicsImpl(pGraphicsImpl)
        , m_pD3D11Texture(NULL)
        , m_pD3D11ShaderView(NULL)
        , m_nMipLevels(nMipLevels)
        , m_Format(eFormat)
        , m_eResUsage(eResUsage)
      {};

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      GXHRESULT   AddRef        () override;
      GXHRESULT   Release       () override;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      GXBOOL IntD3D11CreateResource(GXBOOL bRenderTarget, GXUINT nWidth, GXUINT nHeight, GXLPCVOID pInitData, GXUINT nPitch);

      GXResUsage  GetUsage          () override;
      GXFormat    GetFormat         () override;

      Graphics*   GetGraphicsUnsafe () override;

      inline ID3D11ShaderResourceView*& D3DResourceView()
      {
        return m_pD3D11ShaderView;
      }

      inline ID3D11Texture2D* D3DTexture() const
      {
        return m_pD3D11Texture;
      }
    };

    template<class _Interface>
    Graphics* TextureBaseImplT<_Interface>::GetGraphicsUnsafe()
    {
      return m_pGraphicsImpl;
    }

    class TextureImpl : public TextureBaseImplT<Texture>
    {
      friend class GraphicsImpl;
      friend class CanvasCoreImpl;

    protected:
      virtual GXHRESULT   Invoke        (GRESCRIPTDESC* pDesc);
    public:
//#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
//      GXHRESULT   AddRef        () override;
//      GXHRESULT   Release       () override;
//#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      GXBOOL      Clear             (GXColor color) override;
      //GXBOOL      GetRatio          (GXSizeRatio* pWidthRatio, GXSizeRatio* pHeightRatio) override;
      GXSIZE*     GetDimension      (GXSIZE* pDimension) override;
      GXVOID      GenerateMipMaps   () override;
      void        GetDesc           (TEXTURE_DESC* pDesc) override;
      GXBOOL      GetDesc           (GXBITMAP*lpBitmap) override;
      GXBOOL      CopyRect          (Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource) override;
      GXBOOL      Map               (MAPPED* pMappedRect, GXResMap eResMap) override;
      GXBOOL      Unmap             () override;
      GXBOOL      UpdateRect        (GXLPCRECT prcDest, GXLPVOID pData, GXUINT nPitch) override;
      GrapX::Graphics* GetGraphicsUnsafe () override;
      GXBOOL      SaveToMemory      (clstd::MemBuffer* pBuffer, GXLPCSTR szDestFormat, GXBOOL bVertFlip) override;
      GXBOOL      SaveToFile        (GXLPCWSTR szFileName, GXLPCSTR szDestFormat, GXBOOL bVertFlip) override;
    protected:
      TextureImpl(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight, GXUINT nMipLevels, GXResUsage eResUsage);
      virtual ~TextureImpl();

      GXBOOL Initialize(GXBOOL bRenderTarget, GXLPCVOID pInitData, GXUINT nPitch);
      //void   CalcTextureActualDimension();  // TODO: D3D9 也提出这个函数
      //GXBOOL           IntGetHelpTexture();
      //ID3D11Texture2D* IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData);
      GXUINT GetMinPitchSize() const;

      GXBOOL IntSaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip);

    protected:
      D3D11_MAPPED_SUBRESOURCE  m_sMappedResource;
      GXLPBYTE                  m_pTextureData;

      const GXUINT              m_nWidth;
      const GXUINT              m_nHeight;

    };

    //////////////////////////////////////////////////////////////////////////

    class TextureCubeImpl : public TextureBaseImplT<TextureCube>
    {
      friend class GraphicsImpl;
      friend class CanvasCoreImpl;

    protected:
      virtual GXHRESULT   Invoke        (GRESCRIPTDESC* pDesc);
    public:
//#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
//      GXHRESULT   AddRef        () override;
//      GXHRESULT   Release       () override;
//#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      //GXBOOL      Clear             (GXColor color) override;
      //GXBOOL      GetRatio          (GXSizeRatio* pWidthRatio, GXSizeRatio* pHeightRatio) override;
      GXUINT      GetSize           () const override;    // 取m_nWidth成员的值
      //GXResUsage  GetUsage          () override;
      //GXFormat    GetFormat         () override;
      //GXVOID      GenerateMipMaps   () override;
      //void        GetDesc           (TEXTURE_DESC* pDesc) override;
      //GXBOOL      GetDesc           (GXBITMAP*lpBitmap) override;
      //GXBOOL      CopyRect          (Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource) override;
      //GXBOOL      Map               (MAPPED* pMappedRect, GXResMap eResMap) override;
      //GXBOOL      Unmap             () override;
      //GXBOOL      UpdateRect        (GXLPCRECT prcDest, GXLPVOID pData, GXUINT nPitch) override;
      //GrapX::Graphics* GetGraphicsUnsafe () override;
      //GXBOOL      SaveToMemory      (clstd::MemBuffer* pBuffer, GXLPCSTR szDestFormat, GXBOOL bVertFlip) override;
      //GXBOOL      SaveToFile        (GXLPCWSTR szFileName, GXLPCSTR szDestFormat, GXBOOL bVertFlip) override;
    protected:
      TextureCubeImpl(GrapX::Graphics* pGraphics, GXFormat eFormat, GXUINT nSize, GXUINT nMipLevels, GXResUsage eResUsage);
      virtual ~TextureCubeImpl();

      GXBOOL InitTexture(GXBOOL bRenderTarget, GXUINT nWidth, GXUINT nHeight, GXLPCVOID pInitData, GXUINT nPitch);
      //void   CalcTextureActualDimension();  // TODO: D3D9 也提出这个函数
      //GXBOOL           IntGetHelpTexture();
      //ID3D11Texture2D* IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData);
      GXUINT GetMinPitchSize(GXUINT nWidth) const;

      //GXBOOL IntSaveToMemory(clstd::MemBuffer* pBuffer, GXLPCSTR pImageFormat, GXBOOL bVertFlip);

    protected:
      D3D11_MAPPED_SUBRESOURCE  m_sMappedResource;
      GXLPBYTE                  m_pTextureData = NULL;

      const GXUINT              m_nSize = 0;
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

    class RenderTarget_TextureImpl : public TextureImpl
    {
      friend class GraphicsImpl;
      friend class TextureImpl;

    protected:
      ID3D11RenderTargetView* m_pD3D11RenderTargetView = NULL;
      UINT m_nSlice = 0;

    public:
      RenderTarget_TextureImpl(Graphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight);
      virtual ~RenderTarget_TextureImpl();
      GXBOOL InitRenderTexture(ID3D11Texture2D* pD3D11Texture);
      GXBOOL InitRenderTexture(ID3D11Texture2D* pD3D11Texture, int nFaceIndex);
      ID3D11RenderTargetView* D3DGetRenderTargetView() const;
      
      GXBOOL CopyRect         (Texture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource) override;
    };

    //////////////////////////////////////////////////////////////////////////

    class CubeRenderTarget_TextureCubeImpl : public TextureCubeImpl
    {
      friend class GraphicsImpl;
      friend class TextureCubeImpl;

    protected:
      ID3D11RenderTargetView* m_pD3D11RenderTargetView = NULL;

    public:
      CubeRenderTarget_TextureCubeImpl(Graphics* pGraphics, GXFormat eFormat, GXUINT nSize);
      virtual ~CubeRenderTarget_TextureCubeImpl();
      GXBOOL InitRenderTexture();
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