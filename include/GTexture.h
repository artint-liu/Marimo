#ifndef _GRAPX_TEXTURE_H_
#define _GRAPX_TEXTURE_H_

class GXGraphics;
class GTextureBase : public GResource
{
public:
  GTextureBase(GXUINT nPriority, GXDWORD dwType) : GResource(0, dwType){}
public:
  GXSTDINTERFACE(GXHRESULT    AddRef            ());
  GXSTDINTERFACE(GXHRESULT    Release           ());
public:
  GXSTDINTERFACE(GXDWORD      GetUsage          ());
  GXSTDINTERFACE(GXFormat     GetFormat         ());
  GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
  GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe ());      // �����������ü���
  GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat));
};

class GTexture : public GTextureBase
{
public:
  typedef struct __tagLOCKEDRECT
  {
    GXINT     Pitch;
    GXLPVOID  pBits;
  }LOCKEDRECT, *LPLOCKEDRECT;

public:
  GTexture() : GTextureBase(0, RESTYPE_TEXTURE2D){}
  GXSTDINTERFACE(GXHRESULT    AddRef            ());
  GXSTDINTERFACE(GXHRESULT    Release           ());

  GXSTDINTERFACE(GXBOOL       Clear             (GXCONST GXLPRECT lpRect, GXCOLOR dwColor));  // ʵ�ֲ�ͬ, ���鲻Ҫ������ʱ����ʹ��!
  GXSTDINTERFACE(GXBOOL       GetRatio          (GXINT* pWidthRatio, GXINT* pHeightRatio));   // ȥ��Ļ����,�������Ļ��������,���ظ�ֵ�ı���,���򷵻�����ߴ�,������ļ���ȡ��������ԭʼ�ļ��Ĵ�С
  GXSTDINTERFACE(GXUINT       GetWidth          ());    // ȡm_nWidth��Ա��ֵ
  GXSTDINTERFACE(GXUINT       GetHeight         ());    // ȡm_nHeight��Ա��ֵ, �ο�GetWidth()
  GXSTDINTERFACE(GXBOOL       GetDimension      (GXUINT* pWidth, GXUINT* pHeight));  // ȡ����ĳߴ�,���ֵ���ܻ����Ļ�ߴ�仯
  //GXSTDINTERFACE(GXDWORD      GetUsage          ());
  //GXSTDINTERFACE(GXFormat     GetFormat         ());
  //GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
  GXSTDINTERFACE(GXBOOL       GetDesc           (GXBITMAP*lpBitmap));
  GXSTDINTERFACE(GXBOOL       CopyRect          (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination));
  GXSTDINTERFACE(GXBOOL       StretchRect       (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter));
  GXSTDINTERFACE(GXBOOL       LockRect          (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)); // TODO: �����Ժ��ǲ��ǲ�Ҫ��lock, ����Χ�Ľӿڴ���
  GXSTDINTERFACE(GXBOOL       UnlockRect        ());
  //GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe ());      // �����������ü���
                                              
  //GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat));
};

class GTexture3D : public GTextureBase
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
  GTexture3D() : GTextureBase(0, RESTYPE_TEXTURE3D){}

  GXSTDINTERFACE(GXHRESULT    AddRef            ());
  GXSTDINTERFACE(GXHRESULT    Release           ());

  GXSTDINTERFACE(GXBOOL       Clear             (GXCONST LPBOX lpRect, GXCOLOR dwColor));
  GXSTDINTERFACE(GXUINT       GetWidth          ());
  GXSTDINTERFACE(GXUINT       GetHeight         ());
  GXSTDINTERFACE(GXUINT       GetDepth          ());
  GXSTDINTERFACE(GXBOOL       GetDimension      (GXUINT* pWidth, GXUINT* pHeight, GXUINT* pDepth));
  //GXSTDINTERFACE(GXDWORD      GetUsage          ());
  //GXSTDINTERFACE(GXFormat     GetFormat         ());
  //GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
  GXSTDINTERFACE(GXBOOL       CopyBox           (GTexture3D* pSrc, GXCONST LPBOX lprcSource, GXUINT x, GXUINT y, GXUINT z));
  GXSTDINTERFACE(GXBOOL       LockBox           (LPLOCKEDBOX lpLockRect, GXCONST LPBOX lpBox, GXDWORD Flags)); // TODO: �����Ժ��ǲ��ǲ�Ҫ��lock, ����Χ�Ľӿڴ���
  GXSTDINTERFACE(GXBOOL       UnlockBox         ());
  //GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe ());      // �����������ü���
  //GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR szFileName, GXLPCSTR szDestFormat));
};

class GTextureCube : public GTextureBase
{
public:
  typedef struct __tagLOCKEDRECT
  {
    GXINT     Pitch;
    GXLPVOID  pBits;
  }LOCKEDRECT, *LPLOCKEDRECT;

public:
  GTextureCube() : GTextureBase(0, RESTYPE_TEXTURE_CUBE){}

  GXSTDINTERFACE(GXHRESULT    AddRef            ());
  GXSTDINTERFACE(GXHRESULT    Release           ());

  GXSTDINTERFACE(GXBOOL       Clear             (GXCONST GXLPRECT lpRect, GXCOLOR dwColor));  // ʵ�ֲ�ͬ, ���鲻Ҫ������ʱ����ʹ��!
  GXSTDINTERFACE(GXUINT       GetSize           ());    // ȡm_nWidth��Ա��ֵ
  GXSTDINTERFACE(GXDWORD      GetUsage          ());
  GXSTDINTERFACE(GXFormat     GetFormat         ());
  GXSTDINTERFACE(GXVOID       GenerateMipMaps   ());
  //GXSTDINTERFACE(GXBOOL       CopyRect          (GTexture* pSrc, GXLPCRECT lprcSource, GXLPCPOINT lpptDestination));
  //GXSTDINTERFACE(GXBOOL       StretchRect       (GTexture* pSrc, GXLPCRECT lpDest, GXLPCRECT lpSrc, GXTextureFilterType eFilter));
  //GXSTDINTERFACE(GXBOOL       LockRect          (LPLOCKEDRECT lpLockRect, GXLPCRECT lpRect, GXDWORD Flags)); // TODO: �����Ժ��ǲ��ǲ�Ҫ��lock, ����Χ�Ľӿڴ���
  //GXSTDINTERFACE(GXBOOL       UnlockRect        ());
  GXSTDINTERFACE(GXGraphics*  GetGraphicsUnsafe ());      // �����������ü���

  GXSTDINTERFACE(GXBOOL       SaveToFileW       (GXLPCWSTR pszFileName, GXLPCSTR pszDestFormat));
};

typedef GTexture* GLPTEXTURE;


#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GRAPX_TEXTURE_H_