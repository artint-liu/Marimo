#ifndef _CLSTD_IMAGE_H_
#define _CLSTD_IMAGE_H_

// 这个是clstd下的设备无关Image处理类
// Image数据完全在内存中，不与显卡等硬件设备关联，处理操作也依赖于cpu进行

namespace clstd
{
  // 通道含义：
  // R = 红色
  // G = 绿色
  // B = 蓝色
  // A = Alpha
  // X = 占位
  // 未来考虑支持'Y','U','V'通道

  struct IMAGEDESC
  {
    void* ptr;        // 数据地址
    int   width;      // 像素宽度
    int   height;     // 像素高度
    int   channel;    // 通道数, 1, 2, 3, 4
    int   pitch;      // 一行像素占的字节长度
    int   depth;      // 通道位数，8bit/16bit/32bits/64bits
    union {
      u8  name[4];    // 通道格式 'R','G','B','A','X'
      u32 code;
    }format;
  };

  enum ImageFilter
  {
    ImageFilter_Nearest = 0,
  };

  enum ImageColorSpace
  {
    // 枚举值与通道顺序无关
    ImageColorSpace_Unknown,  // 没有色彩空间，可能只储存了Alpha通道
    ImageColorSpace_Mix,      // 混合模式，储存了不同色彩空间的值
    ImageColorSpace_RGB,      // RGB色彩空间
    ImageColorSpace_YUV,      // YUV色彩空间
  };

  class Image
  {
    // 这里面包含了基本数据结构和方法，所有成员方法应当与数据类型无关
  protected:
    CLLPBYTE m_ptr;   // 数据地址
    int m_width;      // 像素宽度
    int m_height;     // 像素高度
    int m_channel;    // 通道数, 1, 2, 3, 4
    int m_pitch;      // 一行像素占的字节长度
    int m_depth;      // 通道位数，8bit/16bit/32bits
    union PIXELFORMAT
    {
      u8  name[4];    // 通道格式 'R','G','B','A','X'
      u32 code;
    }m_format;

    const static int s_nMaxChannel = 4;

    static b32 IntParseFormat(const char* fmt, u32* pFmtCode, int* pChannel);
    static int IntGetChanelIndex(const PIXELFORMAT& fmt, int nNumOfChannels, char chChannelCode); // 大写字母
    static ImageColorSpace IntGetColorSpace(const PIXELFORMAT& fmt, int nNumOfChannels);

  public:
    Image();
    Image(const Image& image);
    virtual ~Image();

  public:
    Image& operator=(const Image& image);
    b32         CompareFormat       (const char* fmt) const;
    b32         Set                 (int nWidth, int nHeight, const char* fmt, const void* pData);
    b32         Set                 (int nWidth, int nHeight, const char* fmt, int nChannelDepth, const void* pData, int nPitch = 0);
    int         GetWidth            () const;
    int         GetHeight           () const;
    b32         SetChannelDepth     (int nDepth); // 设置新的深度
    int         GetChannelDepth     () const;
    int         GetChannels         () const;
    int         GetPitch            () const;
    Image&      Inflate             (int left, int top, int right, int bottom); // 调整Image尺寸，参数是四个边缘扩展的像素数，可以是负数
    const void* GetPixelPtr         (int x, int y) const;
    void*       GetPixelPtr         (int x, int y);
    size_t      GetDataSize         () const;
    const void* GetLine             (int y) const;
    void*       GetLine             (int y);
    int         GetChannelOffset    (char chChannel) const;   // 通道在像素中的偏移量, 对于"AAAX"这种格式只能返回第一个Alpha通道的偏移
    char        GetChannelName      (int offset) const;
    b32         RenameChannel       (int offset, char chChannel); // 通道改名，只改变通道名称，数据不做任何处理
    b32         RenameChannel       (char* szChannel);
    b32         GetChannelPlane     (Image* pDestImage, char chChannel); // 获得通道平面，pDest将被清空
    b32         ReplaceChannel      (char chReplacedChannel, const Image* pSource, char chSrcChannel); // 从一个图像拷贝通道，如果指定的通道在源图像中不存在，会失败。
    b32         ScaleNearest        (Image* pDestImage, int nWidth, int nHeight); // 点采样缩放，这个不需要计算像素
    const char* GetFormat           () const;
    b32         SetFormat           (const char* fmt); // 更改通道顺序或者删除通道
    ImageColorSpace GetColorSpace   () const;
    b32         SetColorSpace       (ImageColorSpace eSpace);
  private:
    //template<typename _Ty>
    //void IntCopyChannel( Image* pDestImage, int nOffset, const int nPixelSize );
    template<typename _TChannel>
    static void IntCopyChannel(Image* pDestImage, int nDestIndex, const Image* pSrcImage, int nSrcIndex);

    template<typename _Ty>
    void IntStretchCopy( Image* pDestImage, int nWidth, int nHeight );

    template<typename _Ty>
    void IntStretchCopyMulti( Image* pDestImage, int nWidth, int nHeight, int nCount );

    template<typename _TDstPixel, typename _TSrcPixel, typename _TChannel>
    void ChangePixel(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch) const;

    template<typename _TChannel>
    void ChangeFormat(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch);

    template<typename _TSrcPixel, typename _TChannel>
    void ChangeFormat2(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch);

    template<typename _TDestChannel, typename _TSrcChannel>
    void ChangeDepth(CLLPBYTE pDestPtr, size_t nDestPitch, int right_shift);

    template<typename _TChannel>
    b32 RGBAToYUVA(CLLPBYTE pDestPtr, int nNewPixelSize, int nNewPitch, float mulval, int maxval, int* nChannelTable);

    template<typename _TPixel> // 块传送，dest与src必须像素格式一致，位置参数必须裁剪正确
    static void BlockTransferT(IMAGEDESC* pDest, int xDest, int yDest, IMAGEDESC* pSrc, int xSrc, int ySrc, int nCopyWidth, int nCopyHeight);

    //template<typename _Ty>
    //void IntCopyChannel( Image* pDestImage, int nOffset, const int nPixelSize );


  public:
    static void BlockTransfer(IMAGEDESC* pDest, int x, int y, int nCopyWidth, int nCopyHeight, const IMAGEDESC* pSrc);
  };

  //
  // Filter 还是 Sampler
  //
  class ImageFilterI8 : public Image
  {
  public:
    b32 SetAsInteger(Image* pSrcImage); // 按照整数像素设置
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

  // ImageFilterXXX只是Image的方法扩展，不做数据记录
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterF));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI8));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI16));
  STATIC_ASSERT(sizeof(Image) == sizeof(ImageFilterI32));
} // namespace clstd

#endif // _CLSTD_IMAGE_H_