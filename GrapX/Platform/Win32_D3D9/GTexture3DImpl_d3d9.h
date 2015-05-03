#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GTEXTURE3D_D3D9_IMPLEMENT_H_
#define _GTEXTURE3D_D3D9_IMPLEMENT_H_

namespace D3D9
{
  class GXGraphicsImpl;
  //////////////////////////////////////////////////////////////////////////
  class GTexture3DImpl : public GTexBaseImplT<GTexture3D>
  {
  protected:
    //GXGraphicsImpl*           m_pGraphicsImpl;
    //LPDIRECT3DVOLUMETEXTURE9  m_pTexture;
    //CREATETYPE                m_emType;

    //clStringA                 m_Name;
    GXUINT                    m_nWidth;
    GXUINT                    m_nHeight;
    GXUINT                    m_nDepth;
    GXUINT                    m_nMipLevels;
    GXFormat                  m_Format;
    GXDWORD                   m_dwResUsage;

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT    AddRef            ();
    virtual GXHRESULT    Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXBOOL       Clear             (GXCONST LPBOX lpRect, GXCOLOR dwColor);
    virtual GXUINT       GetWidth          ();
    virtual GXUINT       GetHeight         ();
    virtual GXUINT       GetDepth          ();
    virtual GXBOOL       GetDimension      (GXUINT* pWidth, GXUINT* pHeight, GXUINT* pDepth);
    virtual GXDWORD      GetUsage          ();
    virtual GXFormat     GetFormat         ();
    virtual GXVOID       GenerateMipMaps   ();
    virtual GXBOOL       CopyBox           (GTexture3D* pSrc, GXCONST LPBOX lprcSource, GXUINT x, GXUINT y, GXUINT z);
    virtual GXBOOL       LockBox           (LPLOCKEDBOX lpLockRect, GXCONST LPBOX lpBox, GXDWORD Flags);
    virtual GXBOOL       UnlockBox         ();
    virtual GXGraphics*  GetGraphicsUnsafe ();      // 不会增加引用计数

    virtual GXBOOL       SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat);

    GTexture3DImpl(GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, 
      GXDWORD ResUsage, GXGraphics* pGraphics);

    virtual ~GTexture3DImpl();

  };

  class GTexture3DFromFile : public GTexture3DImpl
  {
  private:
    clStringW       m_strSrcFile;
    GXDWORD         m_Filter;
    GXDWORD         m_MipFilter;
    GXCOLORREF      m_ColorKey;

  public:
    GTexture3DFromFile(GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, GXUINT Depth, GXUINT MipLevels, GXFormat Format, 
      GXDWORD ResUsage, GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXGraphics* pGraphics);
    virtual ~GTexture3DFromFile();

    HRESULT Create(LPGXIMAGEINFOX pSrcInfo);

    virtual GXHRESULT    Invoke            (GRESCRIPTDESC* pDesc);
  private:

  };

} // namespace D3D9

#endif // _GTEXTURE3D_D3D9_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
