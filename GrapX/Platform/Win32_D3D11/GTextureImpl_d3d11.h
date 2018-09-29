#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE_D3D11_IMPLEMENT_H_
#define _GTEXTURE_D3D11_IMPLEMENT_H_

namespace D3D11
{
  //#include <Include/GTexture.h>
  //struct TEXLOCKED
  //{
  //  GXLPVOID lpSystemMem;

  //  TEXLOCKED() : lpSystemMem(NULL) {}
  //};
  class GResource;
  class GXGraphicsImpl;
  template<class _Interface>
  class GTexBaseImplT : public _Interface
  {
  protected:
    GXGraphicsImpl*           m_pGraphics;
    ID3D11Texture2D*          m_pTexture;
    ID3D11ShaderResourceView* m_pTexRV;

  public:
    GTexBaseImplT(GXGraphicsImpl*pGraphics)
      : m_pGraphics(pGraphics)
      , m_pTexture(NULL)
      , m_pTexRV(NULL)
    {};

    virtual GXHRESULT AddRef () = NULL;
    virtual GXHRESULT Release() = NULL;

    inline ID3D11ShaderResourceView*& D3DResourceView()
    {
      return m_pTexRV;
    }
  };

  class GTextureImpl : public GTexBaseImplT<GTexture>
  {
    friend class GXGraphicsImpl;
    friend class GXCanvasCoreImpl;
  public:
    enum CREATETYPE
    {
      Invalid           = 0,
      User              = 1,
      File              = 2,
      FileEx            = 3,
      Resource          = 4,
      ResourceEx        = 5,
      OffscreenPlainSur = 6,
      D3DSurfaceRef     = 7,
      LastType
    };
  protected:
    virtual GXHRESULT   Invoke        (GRESCRIPTDESC* pDesc);
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef        ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    friend GXBOOL      GXDLLAPI GXSaveTextureToFileW      (GXLPCWSTR pszFileName, GXLPCWSTR pszDestFormat, GTexture* pTexture);

    virtual GXBOOL      Clear         (const GXLPRECT lpRect, GXCOLOR dwColor);
    virtual GXBOOL      GetRatio      (GXINT* pWidthRatio, GXINT* pHeightRatio);
    virtual GXUINT      GetWidth      ();
    virtual GXUINT      GetHeight     ();
    virtual GXBOOL      GetDimension  (GXUINT* pWidth, GXUINT* pHeight);
    virtual GXDWORD     GetUsage      ();
    virtual GXFormat    GetFormat     ();
    virtual GXVOID      GenerateMipMaps();
    virtual GXBOOL      GetDesc       (GXBITMAP*lpBitmap);
    virtual GXBOOL      CopyRect      (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination);
    virtual GXBOOL      StretchRect   (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter);
    virtual GXBOOL      LockRect      (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags);
    virtual GXBOOL      UnlockRect        ();
    virtual GXGraphics* GetGraphicsUnsafe ();

    //virtual LPDIRECT3DTEXTURE9  D3DTexture();    // 过渡函数
    //virtual LPDIRECT3DSURFACE9  D3DSurface();    // 过渡函数

    virtual GXHRESULT   Release       ();
    virtual GXBOOL      SaveToFileW   (GXLPCWSTR szFileName, GXLPCSTR szDestFormat);

    CREATETYPE          GetCreateType ();
  protected:
    void CalcTextureActualDimension();  // TODO: D3D9 也提出这个函数
    GXBOOL           IntGetHelpTexture();
    ID3D11Texture2D* IntCreateHelpTexture(int nWidth, int nHeight, GXLPVOID pData);
  protected:
    GTextureImpl(GXGraphics* pGraphics);

    CREATETYPE                m_emType;

    GXSHORT                   m_nWidthRatio;
    GXSHORT                   m_nHeightRatio;

    GXUINT                    m_nWidth;
    GXUINT                    m_nHeight;
    GXUINT                    m_nMipLevels;
    GXFormat                  m_Format;
    GXDWORD                   m_dwResUsage;
    ID3D11Texture2D*          m_pHelpTexture; // TODO: 放到Graphics里面,做hash表
    //TEXLOCKED                 m_Locked;
    //GXLPVOID                  m_lpSystemMem;    // GXRU_FREQUENTLYREAD / GXRU_FREQUENTLYWRITE 用的
    //GXPool        m_Pool;
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

    virtual GXBOOL Initialize(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
  };
  //////////////////////////////////////////////////////////////////////////
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

  public:
    GTextureFromUserRT(GXGraphicsImpl* pGraphicsImpl);
    //virtual ~GTextureFromUserRT();

    virtual GXBOOL Initialize(GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
  };
  //////////////////////////////////////////////////////////////////////////
  class GTextureFromFile : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;

  private:
    clStringW    m_strSrcFile;
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