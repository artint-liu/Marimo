#ifndef _GRAPX_TEXTURE_H_
#define _GRAPX_TEXTURE_H_

#define ENABLE_DirectXTex
//class GXGraphics;

namespace clstd
{
  class Image;
} // namespace clstd

namespace GrapX
{
  class TextureBase : public GResource
  {
  public:
    TextureBase(GXUINT nPriority, GXDWORD dwType) : GResource(0, dwType) {}

  public:
    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

  public:
    GXSTDINTERFACE(GXResUsage   GetUsage          ());
    GXSTDINTERFACE(GXFormat     GetFormat         ());
    //GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
    GXSTDINTERFACE(Graphics*    GetGraphicsUnsafe ());      // 不会增加引用计数
  };

  struct TEXTURE_DESC
  {
    GXUINT      Width;
    GXUINT      Height;
    GXUINT      MipLevels;
    GXFormat    Format;
    GXResUsage  Usage;
  };

  // Texture 2D
  class Texture : public TextureBase
  {
  public:
    struct MAPPED
    {
      GXINT     Pitch;
      GXLPVOID  pBits;
    };

  public:
    // TODO: 
    // 放到Graphics中
    // GXSTDINTERFACE(GXBOOL       StretchRect       (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter));
    // 放到GXImage中
    

    Texture() : TextureBase(0, RESTYPE_TEXTURE2D) {}
    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

    GXSTDINTERFACE(GXBOOL       Clear             (GXColor color));  // 实现不同, 建议不要在运行时随意使用!
    GXSTDINTERFACE(GXSIZE*      GetDimension      (GXSIZE* pDimension));  // 取纹理的尺寸, 这个值可能会跟屏幕尺寸变化
    GXSTDINTERFACE(void         GetDesc           (TEXTURE_DESC* pDesc));
    GXSTDINTERFACE(GXBOOL       GetDesc           (GXBITMAP*lpBitmap));
    GXSTDINTERFACE(GXBOOL       CopyRect          (Texture* pSourceTexture, GXLPCPOINT lpptDestination, GXLPCRECT lprcSource));
    GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
    GXSTDINTERFACE(GXBOOL       Map               (MAPPED* pLockRect, GXResMap eResMap)); // TODO: 考虑以后是不是不要用lock, 用外围的接口代替
    GXSTDINTERFACE(GXBOOL       Unmap             ());
    GXSTDINTERFACE(GXBOOL       UpdateRect        (GXLPCRECT prcDest, GXLPVOID pData, GXUINT nPitch));

    GXSTDINTERFACE(GXBOOL       SaveToMemory     (clstd::MemBuffer* pBuffer, GXLPCSTR szDestFormat, GXBOOL bVertFlip));
    GXSTDINTERFACE(GXBOOL       SaveToFile       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat, GXBOOL bVertFlip));

    static GXBOOL GXDLLAPI EncodeToMemory (clstd::MemBuffer* pBuffer, GXLPCVOID pBitsData, GXFormat format, GXUINT width, GXUINT height, GXUINT cbPitch, GXLPCSTR szImageFormat, GXBOOL bVertFlip);
    static GXFormat GXDLLAPI Texture::DecodeToMemory(clstd::Image* pImage, GXLPCVOID pBitsData, GXUINT cbData, GXBOOL bVertFlip);
  };

  class Texture3D : public TextureBase
  {
  public:
    typedef struct __tagLOCKEDBOX
    {
      GXINT     RowPitch;
      GXINT     SlicePitch;
      GXLPVOID  pBits;
    }LOCKEDBOX, *LPLOCKEDBOX;

    typedef struct __tagBOX {
      GXUINT Left;
      GXUINT Top;
      GXUINT Right;
      GXUINT Bottom;
      GXUINT Front;
      GXUINT Back;
    } BOX, *LPBOX;


  public:
    Texture3D() : TextureBase(0, RESTYPE_TEXTURE3D) {}

    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

    GXSTDINTERFACE(GXBOOL       Clear             (const LPBOX lpRect, GXCOLOR dwColor));
    GXSTDINTERFACE(GXUINT       GetWidth          ());
    GXSTDINTERFACE(GXUINT       GetHeight         ());
    GXSTDINTERFACE(GXUINT       GetDepth          ());
    GXSTDINTERFACE(GXBOOL       GetDimension      (GXUINT* pWidth, GXUINT* pHeight, GXUINT* pDepth));
    //GXSTDINTERFACE(GXDWORD      GetUsage          ());
    //GXSTDINTERFACE(GXFormat     GetFormat         ());
    //GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
    GXSTDINTERFACE(GXBOOL       CopyBox           (Texture3D* pSrc, const LPBOX lprcSource, GXUINT x, GXUINT y, GXUINT z));
    GXSTDINTERFACE(GXBOOL       LockBox           (LPLOCKEDBOX lpLockRect, const LPBOX lpBox, GXDWORD Flags)); // TODO: 考虑以后是不是不要用lock, 用外围的接口代替
    GXSTDINTERFACE(GXBOOL       UnlockBox         ());
    //GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe ());      // 不会增加引用计数
    //GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat));
  };

  class TextureCube : public TextureBase
  {
  public:
  public:
    struct MAPPED
    {
      GXINT     Pitch;
      GXLPVOID  pBits;
    };

    typedef struct __tagLOCKEDRECT
    {
      GXINT     Pitch;
      GXLPVOID  pBits;
    }LOCKEDRECT, *LPLOCKEDRECT;

  public:
    TextureCube() : TextureBase(0, RESTYPE_TEXTURE_CUBE) {}

    GXSTDINTERFACE(GXHRESULT    AddRef            ());
    GXSTDINTERFACE(GXHRESULT    Release           ());

    //GXSTDINTERFACE(GXBOOL       Clear             (const GXLPRECT lpRect, GXCOLOR dwColor));  // 实现不同, 建议不要在运行时随意使用!
    GXSTDINTERFACE(GXUINT       GetSize           () const);    // 取m_nWidth成员的值
    GXSTDINTERFACE(GXResUsage   GetUsage          ());
    GXSTDINTERFACE(GXFormat     GetFormat         ());
    //GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
    //GXSTDINTERFACE(GXBOOL       CopyRect          (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination));
    //GXSTDINTERFACE(GXBOOL       StretchRect       (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter));
    //GXSTDINTERFACE(GXBOOL       LockRect          (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)); // TODO: 考虑以后是不是不要用lock, 用外围的接口代替
    //GXSTDINTERFACE(GXBOOL       UnlockRect        ());
    GXSTDINTERFACE(GrapX::Graphics*  GetGraphicsUnsafe ());      // 不会增加引用计数

    //GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR pszFileName, GXLPCSTR pszDestFormat));
  };
} // namespace GrapX

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPX_TEXTURE_H_