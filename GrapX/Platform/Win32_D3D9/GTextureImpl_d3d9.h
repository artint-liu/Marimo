#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE_D3D9_IMPLEMENT_H_
#define _GTEXTURE_D3D9_IMPLEMENT_H_

namespace D3D9
{
  //#include <Include/GTexture.h>
  class GXGraphicsImpl;

  template<class _Interface>
  class GTexBaseImplT : public _Interface
  {
  protected:
    GXGraphicsImpl*         m_pGraphicsImpl;
    LPDIRECT3DBASETEXTURE9  m_pTexture;
  public:
    GTexBaseImplT(GXGraphicsImpl* pGraphicsImpl)
      : m_pGraphicsImpl(pGraphicsImpl)
      , m_pTexture     (NULL)
    {
    }
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT AddRef () = NULL;
    virtual GXHRESULT Release() = NULL;
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    inline LPDIRECT3DBASETEXTURE9 D3DTexture() const
    {
      return m_pTexture;
    }
  };




  class GTextureImpl : public GTexBaseImplT<GTexture>
  {
    friend class GXGraphicsImpl;
    friend class GXCanvasCoreImpl;
  public:
    enum CREATIONTYPE
    {
      CreationFailed    = -1, // ´´½¨Ê§°ÜµÄ
      Invalid           = 0,
      User              = 1,
      File              = 2,
      //FileEx            = 3,
      //Resource          = 4,
      //ResourceEx        = 5,
      //OffscreenPlainSur = 6,
      D3DSurfaceRef     = 7,
      LastType
    };
  protected:
    virtual GXHRESULT CreateRes     ();
    virtual GXHRESULT DestroyRes    ();
    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);

    static  GXBOOL    FillSystemMemSurface(LPDIRECT3DSURFACE9 pSurface, D3DFORMAT fmt, GXCONST GXLPRECT lpRect, GXCOLOR dwColor);
  public:
    friend GXBOOL      GXDLLAPI GXSaveTextureToFileW      (GXLPCWSTR pszFileName, GXLPCWSTR pszDestFormat, GTexture* pTexture);


    virtual GXBOOL      Clear         (GXCONST GXLPRECT lpRect, GXCOLOR dwColor);
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

    //inline LPDIRECT3DTEXTURE9  D3DTexture();
    inline LPDIRECT3DSURFACE9  D3DSurface();

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef        ();
    virtual GXHRESULT   Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXBOOL      SaveToFileW   (GXLPCWSTR szFileName, GXLPCSTR szDestFormat);

    CREATIONTYPE        GetCreationType ();
  protected:
    GTextureImpl(GXGraphics* pGraphics);
    GTextureImpl(GXGraphics* pGraphics, CREATIONTYPE eCreateType, GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);

    //GXGraphicsImpl*     m_pGraphics;
    //LPDIRECT3DTEXTURE9  m_pTexture;
    LPDIRECT3DSURFACE9  m_pSurface;
    CREATIONTYPE        m_emType;

    clStringA           m_Name;
    GXSHORT             m_nWidthRatio;
    GXSHORT             m_nHeightRatio;

    GXUINT              m_nWidth;
    GXUINT              m_nHeight;
    GXUINT              m_nMipLevels;
    GXFormat            m_Format;
    GXDWORD             m_dwResUsage;
    //GXPool        m_Pool;
  };

  //inline LPDIRECT3DTEXTURE9  GTextureImpl::D3DTexture()
  //{
  //  return m_pTexture;
  //}

  inline LPDIRECT3DSURFACE9  GTextureImpl::D3DSurface()
  {
    return m_pSurface;
  }

  //////////////////////////////////////////////////////////////////////////
  class GTextureFromUser : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;
  protected:
    LPDIRECT3DTEXTURE9  m_pSysMemTexture;
  public:
    GTextureFromUser(GXGraphics* pGraphics, GXUINT WidthRatio, GXUINT HeightRatio, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage);
    virtual ~GTextureFromUser();
  protected:
    GXBOOL  IntCreateShaderTexture(LPDIRECT3DDEVICE9 lpd3dDevice);
    GXBOOL  IntHoldRenderTarget   ();
    GXBOOL  IntIsNeedBackupRTTexture() const;

    //virtual GXLRESULT AddRef        ();
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);
    virtual GXHRESULT CreateRes     ();
    virtual GXHRESULT DestroyRes    ();
    virtual GXBOOL    Clear         (GXCONST GXLPRECT lpRect, GXCOLOR dwColor);
    virtual GXBOOL    CopyRect      (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination);
    virtual GXBOOL    LockRect      (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags);
    virtual GXBOOL    UnlockRect    ();
  };
  //////////////////////////////////////////////////////////////////////////
  class GTextureFromFile : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;

  private:
    clStringW       m_strSrcFile;
    GXDWORD         m_Filter;
    GXDWORD         m_MipFilter;
    GXCOLORREF      m_ColorKey;
    //D3DXIMAGE_INFO*    m_pSrcInfo;
    //GXPALETTEENTRY*    m_pPalette;

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);

  public:
    static UINT ConvertParamSizeToD3D(GXUINT nSize);

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
  //  virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);
  //  virtual GXBOOL    LockRect      (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags);
  //  virtual GXBOOL    UnlockRect    ();

  //public:
  //  virtual GXBOOL    CopyRect    (GTexture* pSrc);
  //  GTextureOffscreenPlainSur(GXGraphics* pGraphics);
  //  ~GTextureOffscreenPlainSur();
  //};

  class GTextureImpl_FromD3DSurface : public GTextureImpl
  {
    friend class GXGraphicsImpl;
    friend class GTextureImpl;
  private:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT Release       ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXHRESULT Invoke        (GRESCRIPTDESC* pDesc);

  public:
    GTextureImpl_FromD3DSurface(GXGraphics* pGraphics, LPDIRECT3DSURFACE9 lpd3dSurface);
    ~GTextureImpl_FromD3DSurface();
  };

  //////////////////////////////////////////////////////////////////////////
} // namespace D3D9

#endif // _GTEXTURE_D3D9_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
