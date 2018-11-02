#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE_D3D11_IMPLEMENT_H_
#define _GTEXTURE_D3D11_IMPLEMENT_H_

namespace D3D11
{
  class GResource;
  class GXGraphicsImpl;

  // 用来统一内部TextureImpl对象
  template<class _Interface>
  class GTexureBaseImplT : public _Interface
  {
  protected:
    GXGraphicsImpl*           m_pGraphics;
    ID3D11Texture2D*          m_pD3D11Texture;
    ID3D11ShaderResourceView* m_pD3D11ShaderView;

  public:
    GTexureBaseImplT(GXGraphicsImpl*pGraphics)
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
  };

  class GTextureImpl : public GTexureBaseImplT<GTexture>
  {
    friend class GXGraphicsImpl;
    friend class GXCanvasCoreImpl;

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

    GXBOOL      Clear             (const GXLPRECT lpRect, GXCOLOR dwColor) override;
    //GXBOOL      GetRatio          (GXSizeRatio* pWidthRatio, GXSizeRatio* pHeightRatio) override;
    GXSIZE*     GetDimension      (GXSIZE* pDimension) override;
    GXResUsage  GetUsage          () override;
    GXFormat    GetFormat         () override;
    GXVOID      GenerateMipMaps   () override;
    GXBOOL      GetDesc           (GXBITMAP*lpBitmap) override;
    GXBOOL      CopyRect          (GTexture* pSrc, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource) override;
    GXBOOL      MapRect           (MAPPEDRECT* pLockRect, GXLPCRECT pRect, GXResMap eResMap) override;
    GXBOOL      UnmapRect         () override;
    GXGraphics* GetGraphicsUnsafe () override;

  protected:
    GTextureImpl(GXGraphics* pGraphics, GXFormat eFormat, GXUINT nWidth, GXUINT nHeight, GXUINT nMipLevels, GXResUsage eResUsage);
    virtual ~GTextureImpl();

    GXBOOL InitTexture(GXLPCVOID pInitData, GXUINT nPitch);
    //void   CalcTextureActualDimension();  // TODO: D3D9 也提出这个函数
    //GXBOOL           IntGetHelpTexture();
    //ID3D11Texture2D* IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData);
    GXBOOL IntD3D11CreateResource(GXLPCVOID pInitData, GXUINT nPitch);
    GXUINT GetMinPitchSize() const;

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
#if 0
  class GTextureFromUser : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;
  protected:
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);
    virtual GXBOOL    CopyRect      (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination);

  public:
    GTextureFromUser(GXGraphicsImpl* pGraphicsImpl);
    //virtual ~GTextureFromUser();

    virtual GXBOOL InitTexture(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
  };
#endif
  //////////////////////////////////////////////////////////////////////////
#if 0
  class GTextureFromUserRT : public GTextureFromUser
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;
  private:
    //virtual GXLRESULT AddRef        ();

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);
    virtual GXBOOL    CopyRect      (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination);

  protected:
    ID3D11RenderTargetView*   m_pRenderTargetView;

    ID3D11Texture2D*          m_pDepthStencil;
    ID3D11DepthStencilView*   m_pDepthStencilView;

  public:
    GTextureFromUserRT(GXGraphicsImpl* pGraphicsImpl);
    virtual ~GTextureFromUserRT();

    virtual GXBOOL InitTexture(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
  };
#endif
  //////////////////////////////////////////////////////////////////////////
#if 0
  class GTextureFromFile : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;

  private:
    clStringW      m_strSrcFile;
    GXDWORD        m_Filter;
    GXDWORD        m_MipFilter;
    GXCOLORREF      m_ColorKey;
    //D3DXIMAGE_INFO*    m_pSrcInfo;
    //GXPALETTEENTRY*    m_pPalette;

    //static UINT ConvertParamSizeToD3D(GXUINT nSize);

  public:
    HRESULT Create(LPGXIMAGEINFOX pSrcInfo);
    GTextureFromFile(GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, 
      GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter, 
      GXDWORD MipFilter, GXCOLORREF ColorKey, GXGraphics* pGraphics);
    virtual ~GTextureFromFile();
  };
#endif
  //////////////////////////////////////////////////////////////////////////

  //class GTextureOffscreenPlainSur : public GTextureImpl
  //{
  //  friend class GXGraphicsImpl;
  //  friend class GTextureImpl;
  //private:
  //  virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc) { return GX_OK; }

  //  virtual GXBOOL    LockRect      (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags);
  //  virtual GXBOOL    UnlockRect    ();

  //public:
  //  virtual GXBOOL    CopyRect    (GTexture* pSrc);
  //  GTextureOffscreenPlainSur(GXGraphics* pGraphics);
  //  ~GTextureOffscreenPlainSur();
  //};

/*  class GTextureImpl_FromD3DSurface : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;
  private:
    virtual GXLRESULT  OnDeviceEvent  (DeviceEvent eEvent);
  public:
    GTextureImpl_FromD3DSurface(GXGraphics* pGraphics, LPDIRECT3DSURFACE9 lpd3dSurface);
    ~GTextureImpl_FromD3DSurface();
  };*/
}

#endif // _GTEXTURE_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11