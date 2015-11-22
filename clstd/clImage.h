#ifndef _CLSTD_IMAGE_H_
#define _CLSTD_IMAGE_H_

// �����clstd�µ��豸�޹�Image������
// Image������ȫ���ڴ��У������Կ���Ӳ���豸�������������Ҳ������cpu����

namespace clstd
{
  // ͨ�����壺
  // R = ��ɫ
  // G = ��ɫ
  // B = ��ɫ
  // A = Alpha
  // X = ռλ
  // δ������֧��'Y','U','V'ͨ��

  struct IMAGEDESC
  {
    void* ptr;        // ���ݵ�ַ
    int   width;      // ���ؿ��
    int   height;     // ���ظ߶�
    int   channel;    // ͨ����, 1, 2, 3, 4
    int   pitch;      // һ������ռ���ֽڳ���
    int   depth;      // ͨ��λ����8bit/16bit/32bits/64bits
    union {
      u8  name[4];    // ͨ����ʽ 'R','G','B','A','X'
      u32 code;
    }format;
  };

  enum ImageFilter
  {
    ImageFilter_Nearest = 0,
  };

  class Image
  {
    // ����������˻������ݽṹ�ͷ��������г�Ա����Ӧ�������������޹�
  protected:
    CLLPBYTE m_ptr;   // ���ݵ�ַ
    int m_width;      // ���ؿ��
    int m_height;     // ���ظ߶�
    int m_channel;    // ͨ����, 1, 2, 3, 4
    int m_pitch;      // һ������ռ���ֽڳ���
    int m_depth;      // ͨ��λ����8bit/16bit/32bits
    union
    {
      u8  name[4];    // ͨ����ʽ 'R','G','B','A','X'
      u32 code;
    }m_format;

    const static int s_nMaxChannel = 4;

    static b32 IntParseFormat(const char* fmt, u32* pFmtCode, int* pChannel);

  public:
    Image();
    virtual ~Image();

  public:
    b32       CompareFormat (const char* fmt) const;
    b32       Set           (int nWidth, int nHeight, const char* fmt, const void* pData);
    b32       Set           (int nWidth, int nHeight, const char* fmt, int nPitch, int nDepth, const void* pData);
    int       GetWidth      () const;
    int       GetHeight     () const;
/*ûʵ��*/int       Inflate       (int left, int top, int right, int bottom); // ����Image�ߴ磬�������ĸ���Ե��չ���������������Ǹ���
    const void* GetPixelPtr   (int x, int y) const;
    void*       GetPixelPtr   (int x, int y);
    const void* GetLine       (int y) const;
    void*       GetLine       (int y);
    int       GetChannelOffset(char chChannel);   // ͨ���������е�ƫ����
    b32       GetChannelPlane (Image* pDestImage, char chChannel); // ���ͨ��ƽ�棬pDest�������
    b32       ScaleNearest    (Image* pDestImage, int nWidth, int nHeight); // ��������ţ��������Ҫ��������
    const char* GetFormat     () const;
  private:
    template<typename _Ty>
    void IntCopyChannel( Image* pDestImage, int nOffset, const int nPixelSize );

    template<typename _Ty>
    void IntStretchCopy( Image* pDestImage, int nWidth, int nHeight );

    template<typename _Ty>
    void IntStretchCopyMulti( Image* pDestImage, int nWidth, int nHeight, int nCount );

    //template<typename _Ty>
    //void IntCopyChannel( Image* pDestImage, int nOffset, const int nPixelSize );

  public:
    static void BlockTransfer(IMAGEDESC* pDest, int x, int y, int nCopyWidth, int nCopyHeight, const IMAGEDESC* pSrc);
  };

  //
  // Filter ���� Sampler
  //
  class ImageFilterI8 : public Image
  {
  public:
    b32 SetAsInteger(Image* pSrcImage); // ����������������
  protected:
  private:
  };

  class ImageFilterI16 : public Image
  {
  public:
    b32 SetAsInteger(Image* pSrcImage);
  protected:
  private:
  };

  class ImageFilterI32 : public Image
  {
  public:
    b32 SetAsInteger(Image* pSrcImage);
  protected:
  private:
  };

  class ImageFilterF : public Image
  {
  public:
    b32 Set(ImageFilterI8* pSrcImage, float fLevel = (1.0f / 255.0f));
    b32 Set(ImageFilterI16* pSrcImage, float fLevel = (1.0f / 65535.0f));
    b32 Set(ImageFilterI32* pSrcImage, float fLevel = (1.0f / 255.0f));
    b32 Set (int nWidth, int nHeight, const char* fmt, const void* pData);
    b32 Clear(float value, char chChannel);
  };

  // ImageFilterXXXֻ��Image�ķ�����չ���������ݼ�¼
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterF));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI8));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI16));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI32));
} // namespace clstd

#endif // _CLSTD_IMAGE_H_