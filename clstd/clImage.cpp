#include "clstd.h"
#include "clImage.h"
#include "clUtility.h"
const int delta = 'a' - 'A';

#define CMP_CHANNEL(_IDX)                           (m_format.name[_IDX] == fmt[_IDX] || m_format.name[_IDX] + delta == fmt[_IDX])
#define MIN_PITCH_PARAM(_WIDTH, _CHANNEL, _DEPTH)   ((_WIDTH) * (_CHANNEL) * ((_DEPTH) >> 3))
#define MIN_PITCH                                   MIN_PITCH_PARAM(m_width, m_channel, m_depth)
#define GETPIXELSIZE                                (m_channel * (m_depth >> 3))

#define IS_RGB_COLORSPACE(_CHLNAME)     (_CHLNAME == 'R' || _CHLNAME == 'G' || _CHLNAME == 'B')
#define IS_RGB_COLORSPACE_L(_CHLNAME)   (_CHLNAME == 'r' || _CHLNAME == 'g' || _CHLNAME == 'b')
#define IS_YUV_COLORSPACE(_CHLNAME)     (_CHLNAME == 'Y' || _CHLNAME == 'U' || _CHLNAME == 'V')
#define IS_YUV_COLORSPACE_L(_CHLNAME)   (_CHLNAME == 'y' || _CHLNAME == 'u' || _CHLNAME == 'v')
#define IS_ALPHA_COLORSPACE(_CHLNAME)   (_CHLNAME == 'A' || _CHLNAME == 'a')
#define IS_EMPTY_COLORSPACE(_CHLNAME)   (_CHLNAME == 'X' || _CHLNAME == 'x')
#define IS_COLORCHANNLE(_CHLNAME)       (IS_RGB_COLORSPACE(_CHLNAME) || IS_RGB_COLORSPACE_L(_CHLNAME) || \
                                  IS_YUV_COLORSPACE(_CHLNAME) || IS_YUV_COLORSPACE_L(_CHLNAME) || IS_ALPHA_COLORSPACE(_CHLNAME) || IS_EMPTY_COLORSPACE(_CHLNAME))

namespace clstd
{
  Image::Image()
    : m_ptr(NULL)
    , m_width(0)
    , m_height(0)
    , m_channel(0)
    , m_pitch(0)
    , m_depth(0)
  {
    m_format.code = 0;
  }

  Image::Image(const Image& image)
    : m_ptr(NULL)
    , m_width(image.m_width)
    , m_height(image.m_height)
    , m_channel(image.m_channel)
    , m_pitch(image.m_pitch)
    , m_depth(image.m_depth)
  {
    m_format.code = image.m_format.code;
    if(image.m_ptr)
    {
      m_ptr = new u8[GetDataSize()];
      memcpy(m_ptr, image.m_ptr, GetDataSize());
    }
  }

  Image::~Image()
  {
    SAFE_DELETE_ARRAY(m_ptr);
  }

  Image& Image::operator=(const Image& image)
  {
    SAFE_DELETE_ARRAY(m_ptr);

    m_width       = image.m_width;
    m_height      = image.m_height;
    m_channel     = image.m_channel;
    m_pitch       = image.m_pitch;
    m_depth       = image.m_depth;
    m_format.code = image.m_format.code;
    if (image.m_ptr)
    {
      m_ptr = new u8[GetDataSize()];
      memcpy(m_ptr, image.m_ptr, GetDataSize());
    }
    return *this;
   }

  int Image::GetWidth() const
  {
    return m_width;
  }

  int Image::GetHeight() const
  {
    return m_height;
  }

  template<typename _TDestChannel, typename _TSrcChannel>
  void Image::ChangeDepth(CLLPBYTE pDestPtr, size_t nDestPitch, int right_shift)
  {
    STATIC_ASSERT(sizeof(_TSrcChannel) / sizeof(_TDestChannel) == 2 || sizeof(_TSrcChannel) / sizeof(_TDestChannel) == 4);
    // TODO: 未来加入四舍五入
    for(int y = 0; y < m_height; y++)
    {
      _TDestChannel* pDest = (_TDestChannel*)pDestPtr + nDestPitch * y;
      _TSrcChannel*  pSrc  = (_TSrcChannel*)GetLine(y);
      for(int x = 0; x < m_width; x++)
      {
        for(int chl = 0; chl < m_channel; chl++)
        {
          *pDest = *pSrc >> right_shift;
          pDest++;
          pSrc++;
        }
      }
    }
  }

  template<typename _TPixel>
  void Image::BlockTransferT(IMAGEDESC* pDest, int xDest, int yDest, IMAGEDESC* pSrc, int xSrc, int ySrc, int nCopyWidth, int nCopyHeight)
  {
    // 格式必须一致
    ASSERT(pDest->channel == pSrc->channel && pDest->depth == pSrc->depth && 
      pDest->format.code == pSrc->format.code && pDest->ptr != pSrc->ptr);

    // 参数必须合法，裁剪需要在外面做好
    ASSERT(xDest >= 0 && xDest < pDest->width && yDest >= 0 && yDest < pDest->height);
    ASSERT(xDest + nCopyWidth <= pDest->width && yDest + nCopyHeight <= pDest->height);
    ASSERT(xSrc >= 0 && xSrc < pSrc->width && ySrc >= 0 && ySrc < pSrc->height);
    ASSERT(xSrc + nCopyWidth <= pSrc->width && ySrc + nCopyHeight <= pSrc->height);

    for(int y = 0; y < nCopyHeight; y++)
    {
      _TPixel* d = (_TPixel*)((CLLPBYTE)pDest->ptr + pDest->pitch * (yDest + y) + xDest);
      const _TPixel* s = (const _TPixel*)((CLLPBYTE)pSrc->ptr + pSrc->pitch * (ySrc + y) + xSrc);
      for(int x = 0; x < nCopyWidth; x++)
      {
        *d++ = *s++;
      }
    }
  }

  b32 Image::SetChannelDepth(int nDepth)
  {
    if(nDepth == m_depth) {
      return TRUE;
    }

    CLLPBYTE pDestPtr = m_ptr;
    const int nDestPitch = MIN_PITCH_PARAM(m_width, m_channel, nDepth);
    if(m_depth < nDepth) {
      pDestPtr = new CLBYTE[nDestPitch * m_height];
    }

    if(m_depth == 8)
    {
      CLBREAK; // 没实现
    }
    else if(m_depth == 16)
    {
      switch(nDepth)
      {
      case 8:
        ChangeDepth<u8, u16>(pDestPtr, nDestPitch, 8);
        break;
      case 32:
      default:
        CLBREAK; // 没实现
        break;
      }
    }
    else if(m_depth == 32)
    {
      switch(nDepth)
      {
      case 8:
        ChangeDepth<u8, u16>(pDestPtr, nDestPitch, 24);
        break;
      case 16:
        ChangeDepth<u8, u16>(pDestPtr, nDestPitch, 16);
        break;
      default:
        CLBREAK;
        break;
      }
    }

    if(pDestPtr != m_ptr) {
      SAFE_DELETE_ARRAY(m_ptr);
      m_ptr = pDestPtr;
    }

    m_pitch = nDestPitch;
    m_depth = nDepth;

    return TRUE;
  }

  const void* Image::GetPixelPtr(int x, int y) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x * m_channel;
  }

  void* Image::GetPixelPtr(int x, int y)
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x * m_channel;
  }

  size_t Image::GetDataSize() const
  {
    return (size_t)(m_pitch * m_height);
  }

  int Image::GetChannelDepth() const
  {
    return m_depth;
  }

  int Image::GetChannels() const
  {
    return m_channel;
  }

  int Image::GetPitch() const
  {
    return m_pitch;
  }

  b32 Image::Set(int nWidth, int nHeight, const char* fmt, const void* pData)
  {
    if (m_width != nWidth || m_height != nHeight || m_depth != 8 ||
      MIN_PITCH_PARAM(nWidth, m_channel, m_depth) != m_pitch || !CompareFormat(fmt))
    {
      if (!IntParseFormat(fmt, &m_format.code, &m_channel)) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width = nWidth;
      m_height = nHeight;
      m_depth = 8;
      m_pitch = MIN_PITCH;
      m_ptr = new u8[GetDataSize()];
    }

    // pData == -1     不初始化内存数据
    // pData == NULL   初始化内存为0
    // pData == [else] 拷贝pData数据
    if( ! pData) {
      memset(m_ptr, 0, GetDataSize());
    } else if (pData != (const void*)-1) {
      memcpy(m_ptr, pData, GetDataSize());
    }
    return true;
  }

  b32 Image::Set(int nWidth, int nHeight, const char* fmt, int nChannelDepth, const void* pData, int nPitch)
  {
    if (m_width != nWidth || m_height != nHeight || m_depth != nChannelDepth ||
      m_pitch != nPitch || !CompareFormat(fmt))
    {
      u32 fmtcode;
      int channel;
      if (!IntParseFormat(fmt, &fmtcode, &channel)) {
        return FALSE;
      }

      if (nPitch == 0) {
        nPitch = MIN_PITCH_PARAM(nWidth, channel, nChannelDepth);
      }
      else if (MIN_PITCH_PARAM(nWidth, channel, nChannelDepth) > nPitch) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width = nWidth;
      m_height = nHeight;
      m_pitch = nPitch;
      m_channel = channel;
      m_depth = nChannelDepth;
      m_ptr = new u8[GetDataSize()];
      m_format.code = fmtcode;
    }

    // pData == -1     不初始化内存数据
    // pData == NULL   初始化内存为0
    // pData == [else] 拷贝pData数据
    if( ! pData) {
      memset(m_ptr, 0, GetDataSize());
    } else if (pData != (const void*)-1) {
      memcpy(m_ptr, pData, GetDataSize());
    }
    return true;

  }

  b32 Image::CompareFormat(const char* fmt) const
  {
    switch (m_channel)
    {
    case 1: return CMP_CHANNEL(0) && fmt[1] == '\0';
    case 2: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && fmt[2] == '\0';
    case 3: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && CMP_CHANNEL(2) && fmt[3] == '\0';
    case 4: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && CMP_CHANNEL(2) && CMP_CHANNEL(3);
    }
    return FALSE;
  }

  b32 Image::IntParseFormat(const char* fmt, u32* pFmtCode, int* pChannel)
  {
    int channel = 0;
    union
    {
      u8  name[4];
      u32 code;
    }format;
    format.code = 0;

    for (int i = 0; i < 4; i++)
    {
      if(fmt[i] == 'R' || fmt[i] == 'G' || fmt[i] == 'B' ||
        fmt[i] == 'Y' || fmt[i] == 'U' || fmt[i] == 'V' ||
        fmt[i] == 'A' || fmt[i] == 'X') {
        format.name[i] = fmt[i];
      }
      else if (fmt[i] == 'r' || fmt[i] == 'g' || fmt[i] == 'b' ||
        fmt[i] == 'y' || fmt[i] == 'u' || fmt[i] == 'v' ||
        fmt[i] == 'a' || fmt[i] == 'x') {
        format.name[i] = fmt[i] - ('a' - 'A');
      }
      else if (fmt[i] == '\0') { break; }
      else { return FALSE; }
      channel++;
    }

    // 这样写如果返回false，则不会修改fmtcode和channel的任何内容
    *pChannel = channel;
    *pFmtCode = format.code;
    return TRUE;
  }

  int Image::IntGetChanelIndex(const PIXELFORMAT& fmt, int nNumOfChannels, char chChannelCode)
  {
    for (int i = 0; i < nNumOfChannels; i++) {
      ASSERT(fmt.name[i] == 'R' || fmt.name[i] == 'G' || fmt.name[i] == 'B' || 
        fmt.name[i] == 'Y' || fmt.name[i] == 'U' || fmt.name[i] == 'V' ||
        fmt.name[i] == 'A' || fmt.name[i] == 'X');
      if (fmt.name[i] == chChannelCode) {
        return i;
      }
    }

    // 如果新通道不存在，Alpha通道返回-2，其他返回-1，
    // 在后续的设置中，Alpha通道默认填1.0，其它通道默认是0.0
    if(chChannelCode == 'A') {
      return -2;
    }
    return -1;
  }

  ImageColorSpace Image::IntGetColorSpace(const PIXELFORMAT& fmt, int nNumOfChannels)
  {
    ImageColorSpace eSpace = ImageColorSpace_Unknown;
    for (int i = 0; i < nNumOfChannels; i++)
    {
      ASSERT(fmt.name[i] == 'R' || fmt.name[i] == 'G' || fmt.name[i] == 'B' ||
        fmt.name[i] == 'Y' || fmt.name[i] == 'U' || fmt.name[i] == 'V' ||
        fmt.name[i] == 'A' || fmt.name[i] == 'X');
      
      if (fmt.name[i] == 'R' || fmt.name[i] == 'G' || fmt.name[i] == 'B')
      {
        if(eSpace == ImageColorSpace_YUV) {
          return ImageColorSpace_Mix;
        }
        eSpace = ImageColorSpace_RGB;
      }
      else if (fmt.name[i] == 'Y' || fmt.name[i] == 'U' || fmt.name[i] == 'V')
      {
        if (eSpace == ImageColorSpace_RGB) {
          return ImageColorSpace_Mix;
        }
        eSpace = ImageColorSpace_YUV;
      }
    }
    return eSpace;
  }

  const void* Image::GetLine(int y) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  void* Image::GetLine(int y)
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  Image& Image::Inflate(int left, int top, int right, int bottom)
  {
    if(left == 0 && top == 0 && right == 0 && bottom == 0) {
      return *this;
    }

    IMAGEDESC src = {};
    IMAGEDESC dest = {};
    src.ptr      = m_ptr;
    src.width    = m_width;
    src.height   = m_height;
    src.channel  = m_channel;
    src.pitch    = m_pitch;
    src.depth    = m_depth;
    src.format.code = m_format.code;

    dest.width   = m_width + left + right;
    dest.height  = m_height + top + bottom;
    if(dest.width <= 0 || dest.height <= 0) {
      m_width  = 0;
      m_height = 0;
      m_pitch  = 0;
      SAFE_DELETE_ARRAY(m_ptr);
      return *this;
    }
    dest.channel = src.channel;
    dest.depth   = src.depth;
    dest.pitch   = MIN_PITCH_PARAM(dest.width, dest.channel, dest.depth);
    dest.ptr     = new CLBYTE[dest.pitch * dest.height];
    dest.format.code = src.format.code;

    memset(dest.ptr, 0, dest.pitch * dest.height);

    const int xDest = clMax(0, left);
    const int yDest = clMax(0, top);
    const int xSrc  = clMax(0, -left);
    const int ySrc  = clMax(0, -top);
    const int nCopyWidth  = m_width + clMin(0, left) + clMin(0, right); // 只有负值才会减少width
    const int nCopyHeight = m_height + clMin(0, top) + clMin(0, bottom);

    switch(m_depth)
    {
    case 8:
    {
      if(m_channel == 1) {
        BlockTransferT<u8>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 2) {
        BlockTransferT<u16>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 3) {
        struct PIXEL { u8 c[3]; };
        STATIC_ASSERT(sizeof(PIXEL) == 3);
        BlockTransferT<PIXEL>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 4) {
        BlockTransferT<u32>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      }
      break;
    }
    case 16:
    {
      if(m_channel == 1) {
        BlockTransferT<u16>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 2) {
        BlockTransferT<u32>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 3) {
        struct PIXEL { u16 c[3]; };
        STATIC_ASSERT(sizeof(PIXEL) == 6);
        BlockTransferT<PIXEL>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 4) {
        BlockTransferT<u64>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      }
      break;
    }
    case 32:
    {
      if(m_channel == 1) {
        BlockTransferT<u32>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 2) {
        BlockTransferT<u64>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 3) {
        struct PIXEL { u32 c[3]; };
        STATIC_ASSERT(sizeof(PIXEL) == 12);
        BlockTransferT<PIXEL>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      } else if(m_channel == 4) {
        struct PIXEL { u32 c[4]; };
        BlockTransferT<PIXEL>(&dest, xDest, yDest, &src, xSrc, ySrc, nCopyWidth, nCopyHeight);
      }
      break;
    }
    default:
      CLBREAK;
    }

    m_width  = dest.width;
    m_height = dest.height;
    m_pitch  = dest.pitch;
    SAFE_DELETE_ARRAY(m_ptr);
    m_ptr = (CLLPBYTE)dest.ptr;
    return *this;
  }

  int Image::GetChannelOffset(char chChannel) const
  {
    const int index = IntGetChanelIndex(m_format, m_channel, chChannel);
    return index >= 0 ? (index * (m_depth >> 3)) : -1;
  }

  char Image::GetChannelName(int offset) const
  {
    if(offset >= 0 && offset < m_channel)
    {
      return m_format.name[offset];
    }
    return 0;
  }

  b32 Image::RenameChannel(int offset, char chChannel)
  {
    if (offset >= 0 && offset < m_channel) {
      if(IS_COLORCHANNLE(chChannel)) {
        if (chChannel >= 'a' && chChannel <= 'z') {
          chChannel = chChannel - 'a' + 'A';
        }
        m_format.name[offset] = chChannel;
        return TRUE;
      }
    }
    return FALSE;
  }

  b32 Image::RenameChannel(char* szChannel)
  {
    PIXELFORMAT format;
    int nChannel = 0;
    if( ! IntParseFormat(szChannel, &format.code, &nChannel) || nChannel != m_channel) {
      return FALSE;
    }

    m_format.code = format.code;
    return TRUE;
  }

  b32 Image::GetChannelPlane(Image* pDestImage, char chChannel)
  {
    int nChannelIndex = IntGetChanelIndex(m_format, m_channel, chChannel);
    if (nChannelIndex < 0) {
      return FALSE;
    }

    char fmt[8] = { 0 };
    fmt[0] = chChannel;

    if (!pDestImage->Set(m_width, m_height, fmt, m_depth, NULL, 0)) {
      return FALSE;
    }
    const int nPixelSize = GETPIXELSIZE;

    switch (m_depth)
    {
    case 8:
      IntCopyChannel<u8>(pDestImage, 0, this, nChannelIndex);
      break;

    case 16:
      IntCopyChannel<u16>(pDestImage, 0, this, nChannelIndex);
      break;

    case 32:
      IntCopyChannel<u32>(pDestImage, 0, this, nChannelIndex);
      break;

    default:
      CLBREAK;
      return FALSE;
    }
    return TRUE;
  }

  b32 Image::ReplaceChannel(char chReplacedChannel, const Image* pSource, char chSrcChannel)
  {
    if(m_depth != pSource->m_depth) {
      CLOG_ERROR("%s : 暂时不支持不同深度的替换", __FUNCTION__);
      return FALSE;
    }

    const int nDestChannel = IntGetChanelIndex(m_format, m_channel, chReplacedChannel);
    const int nSrcChannel  = IntGetChanelIndex(pSource->m_format, pSource->m_channel, chSrcChannel);
    if(nDestChannel < 0 || nSrcChannel < 0) {
      CLOG_ERROR("%s : 指定的通道不存在", __FUNCTION__);
      return FALSE;
    }

    if(m_width !=  pSource->m_width || m_height != pSource->m_height) {
      CLOG_ERROR("%s : 尺寸不一致", __FUNCTION__);
      return FALSE;
    }

    switch (m_depth)
    {
    case 8:
      IntCopyChannel<u8>(this, nDestChannel, pSource, nSrcChannel);
      break;

    case 16:
      IntCopyChannel<u16>(this, nDestChannel, pSource, nSrcChannel);
      break;

    case 32:
      IntCopyChannel<u32>(this, nDestChannel, pSource, nSrcChannel);
      break;

    default:
      CLBREAK;
      return FALSE;
    }
    return TRUE;
  }

  b32 Image::ScaleNearest(Image* pDestImage, int nWidth, int nHeight)
  {
    if( ! pDestImage->Set(nWidth, nHeight, (const char*)m_format.name, m_depth, (const void*)-1, 0)) {
      return FALSE;
    }

    // TODO: Copy ...
    switch (GETPIXELSIZE)
    {
    case 1: // 8 depth 1 channel
      IntStretchCopy<u8>(pDestImage, nWidth, nHeight);
      break;

    case 2: // 16 depth 1 channel
            //  8 depth 2 channel
      IntStretchCopy<u16>(pDestImage, nWidth, nHeight);
      break;

    case 3: // 8 depth 3 channel
      {
        struct u24_t { u8 m[3]; };
        IntStretchCopy<u24_t>(pDestImage, nWidth, nHeight);
      }
      break;

    case 4: // 32 depth 1 channel
            // 16 depth 2 channel
            //  8 depth 4 channel
      IntStretchCopy<u32>(pDestImage, nWidth, nHeight);
      break;

    case 6: // 16 depth 3 channel
      {
        struct u48_t { u16 m[3]; };
        IntStretchCopy<u48_t>(pDestImage, nWidth, nHeight);
      }
      break;

    case 8: // 16 depth 4 channel
            // 32 depth 2 channel
      IntStretchCopy<u64>(pDestImage, nWidth, nHeight);
      break;

    case 12: // 32 depth 3 channel
      {
        struct u96_t { u32 m[3]; };
        IntStretchCopy<u96_t>(pDestImage, nWidth, nHeight);
      }
      break;

    case 16: // 32 depth 4 channel
      {
        struct u128_t { u64 m[2]; };
        IntStretchCopy<u128_t>(pDestImage, nWidth, nHeight);
      }
      break;

    default:
      CLBREAK;
      CLOG_ERROR("%s: Unsupported pixel size.\r\n", __FUNCTION__);
      break;
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TChannel, int _channel, int _shift>
  void Image::IntFastLinearScaling(Image* pDestImage, int nWidth, int nHeight)
  {
    typedef i32 t32;
    const t32 _max = (1 << _shift);
    const t32 _mask = _max - 1;

    t32* aLine = new t32[nWidth];
    const t32 stride_fix_x = (m_width << _shift) / nWidth;
    const t32 stride_fix_y = (m_height << _shift) / nHeight;
    t32 fix_x = m_width > nWidth ? (1 << (_shift - 1)) : -(1 << (_shift - 1));
    t32 fix_y = m_height > nHeight ? (1 << (_shift - 1)) : -(1 << (_shift - 1));
    const _TChannel* pSrcLast = (const _TChannel*)GetLine(m_height - 1);

    struct PIXEL_T { _TChannel m[_channel]; };


    for (int w = 0; w < nWidth; w++) {
      aLine[w] = clClamp(t32(0), (t32)(m_width - 1) << _shift, fix_x);
      fix_x += stride_fix_x;
    }

    int y = 0;
    for (; y < nHeight; y++)
    {
      if ((fix_y >> _shift) >= (t32)(m_height - 1)) {
        break;
      }
      //const PIXEL_T* pSrcData = (const PIXEL_T*)GetLine(clMax(0, fix_y >> _shift));
      const PIXEL_T* pSrcData = (const PIXEL_T*)GetLine(clMax(0, fix_y >> _shift));
      const PIXEL_T* pSrcData2 = (const PIXEL_T*)((size_t)pSrcData + m_pitch);
      _TChannel* pDestData = (_TChannel*)pDestImage->GetLine(y);
      int x = 0;
      for (; x < nWidth; x++)
      {
        const t32 mh = aLine[x];
        const t32 xi = (mh >> _shift);
        if (xi + 1 > t32(m_width - 1)) {
          break;
        }
        const t32 xf = mh & _mask;
        const t32 yf = fix_y & _mask;
        const PIXEL_T& a = pSrcData[xi];
        const PIXEL_T& b = (&a)[1];//pSrcData[xi + 1];
        const PIXEL_T& c = pSrcData2[xi];
        const PIXEL_T& d = (&c)[1];//pSrcData2[xi + 1];

        for(int i = 0; i < _channel; i++) {
          *pDestData++ = (
            ((a.m[i] * (_max - xf) + b.m[i] * xf) >> _shift) * (_max - yf) +
            ((c.m[i] * (_max - xf) + d.m[i] * xf) >> _shift) * yf) >> _shift;
        }
        //pDestData->g = (((a.g * (255 - xf) + b.g * xf) >> 8) * (255 - yf) + ((c.g * (255 - xf) + d.g * xf) >> 8) * yf) >> 8;
        //pDestData->b = (((a.b * (255 - xf) + b.b * xf) >> 8) * (255 - yf) + ((c.b * (255 - xf) + d.b * xf) >> 8) * yf) >> 8;
        //pDestData++;
      }

      // 最后一列
      for (; x < nWidth; x++)
      {
        const t32 mh = aLine[x];
        const t32 xi = (mh >> _shift);
        const t32 yf = fix_y & _mask;
        const PIXEL_T& a = pSrcData[xi];
        const PIXEL_T& c = pSrcData2[xi];

        for (int i = 0; i < _channel; i++) {
          // 16位颜色这里会有溢出变为负值，但是看起来被截断后似乎也没什么问题
          *pDestData++ = (a.m[i] * (_max - yf) + c.m[i] * yf) >> _shift;
        }
        //pDestData->g = (a.g * (255 - yf) + c.g * yf) >> 8;
        //pDestData->b = (a.b * (255 - yf) + c.b * yf) >> 8;
        //pDestData++;
      }

      fix_y += stride_fix_y;
    }

    // 最后一行
    for (; y < nHeight; y++)
    {
      const PIXEL_T* pSrcData = (const PIXEL_T*)GetLine(m_height - 1);
      _TChannel* pDestData = (_TChannel*)pDestImage->GetLine(y);
      for (int x = 0; x < nWidth; x++)
      {
        const t32 mh = aLine[x];
        const t32 xf = mh & _mask;
        const t32 xi = (mh >> _shift);
        //const i32 yf = fix_y & 0xff;
        const PIXEL_T& a = pSrcData[xi];
        const PIXEL_T& b = xi == m_width - 1 ? a : (&a)[1];

        for (int i = 0; i < _channel; i++) {
          *pDestData++ = (a.m[i] * (_max - xf) + b.m[i] * xf) >> _shift;
        }
        //pDestData->g = (a.g * (255 - xf) + b.g * xf) >> 8;
        //pDestData->b = (a.b * (255 - xf) + b.b * xf) >> 8;
        //pDestData++;
      }
    }

    SAFE_DELETE_ARRAY(aLine);
  }

  b32 Image::ScaleFastLinear(Image* pDestImage, int nWidth, int nHeight)
  {
    if(m_width == nWidth && m_height == nHeight) {
      *pDestImage = *this;
    }
    else if( ! pDestImage->Set(nWidth, nHeight, (const char*)m_format.name, m_depth, (const void*)-1, 0)) {
      return FALSE;
    }

    switch (m_depth)
    {
    case 8:
      if (m_channel == 1) {
        IntFastLinearScaling<u8, 1, 8>(pDestImage, nWidth, nHeight);
      }
      else if (m_channel == 2) {
        IntFastLinearScaling<u8, 2, 8>(pDestImage, nWidth, nHeight);
      }
      else if(m_channel == 3) {
        IntFastLinearScaling<u8, 3, 8>(pDestImage, nWidth, nHeight);
      }
      else if(m_channel == 4) {
        IntFastLinearScaling<u8, 4, 8>(pDestImage, nWidth, nHeight);
      }
      else {
        CLBREAK;
      }
      break;
    case 16:
      if (m_channel == 1) {
        IntFastLinearScaling<u16, 1, 16>(pDestImage, nWidth, nHeight);
      }
      else if (m_channel == 2) {
        IntFastLinearScaling<u16, 2, 16>(pDestImage, nWidth, nHeight);
      }
      else if (m_channel == 3) {
        IntFastLinearScaling<u16, 3, 16>(pDestImage, nWidth, nHeight);
      }
      else if (m_channel == 4) {
        IntFastLinearScaling<u16, 4, 16>(pDestImage, nWidth, nHeight);
      }
      else {
        CLBREAK;
      }
      break;
    case 32: // TODO: 没想好这个应改为32位整数还是32位浮点数呢
      CLBREAK;
      break;
    default:
      CLBREAK;
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  //void Image::BlockTransfer(IMAGEDESC* pDest, int x, int y, int nCopyWidth, int nCopyHeight, const IMAGEDESC* pSrc)
  //{
  //  CLBREAK;
  //}

  //template<typename _Ty>
  //void Image::IntCopyChannel(Image* pDestImage, int nOffset, const int nPixelSize)
  //{
  //  _Ty* pDestData;
  //  _Ty* pSrcData;
  //  for (int y = 0; y < m_height; y++)
  //  {
  //    pDestData = (_Ty*)((CLLPBYTE)pDestImage->GetLine(y));
  //    pSrcData = (_Ty*)((CLLPBYTE)GetLine(y) + nOffset);
  //    for (int x = 0; x < m_width; x++)
  //    {
  //      *pDestData++ = *pSrcData;
  //      pSrcData = (_Ty*)((CLLPBYTE)pSrcData + nPixelSize);
  //    }
  //  }
  //}

  template<typename _TChannel>
  void Image::IntCopyChannel(Image* pDestImage, int nDestIndex, const Image* pSrcImage, int nSrcIndex)
  {
    ASSERT(pDestImage->m_width == pSrcImage->m_width);
    ASSERT(pDestImage->m_height == pSrcImage->m_height);
    ASSERT(pDestImage->m_depth == pSrcImage->m_depth);

    _TChannel* pDestData;
    _TChannel* pSrcData;
    for (int y = 0; y < pDestImage->m_height; y++)
    {
      pDestData = (_TChannel*)pDestImage->GetLine(y) + nDestIndex;
      pSrcData  = (_TChannel*)pSrcImage->GetLine(y) + nSrcIndex;
      for (int x = 0; x < pDestImage->m_width; x++)
      {
        *pDestData = *pSrcData;
        pDestData += pDestImage->m_channel;
        pSrcData  += pSrcImage->m_channel;
      }
    }
  }

  template<typename _Ty>
  void Image::IntStretchCopy(Image* pDestImage, int nWidth, int nHeight)
  {
    int* aLine = new int[nWidth];
    int delta = 0;
    int step = 0;
    //const float fStrideH = (float)m_width / (float)nWidth;
    //const float fStrideV = (float)m_height / (float)nHeight;
    //float fx = 0;
    //float fy = 0;

    for (int x = 0; x < nWidth; x++) {      
      ASSERT(step < m_width); // 如果断言在这里，就限制一下
      aLine[x] = step;
      delta += m_width;
      while(delta >= nWidth) {
        step++;
        delta -= nWidth;
      }
    }

    delta = 0;
    step = 0;
    for (int y = 0; y < nHeight; y++)
    {
      //int src_y = (int)floor(fy + 0.5f);
      const _Ty* pSrcData = (const _Ty*)GetLine(step < m_height ? step : m_height - 1);
      _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
      for (int x = 0; x < nWidth; x++)
      {
        *pDestData = pSrcData[aLine[x]];
        pDestData++;
      }
      //fy += fStrideV;
      delta += m_height;
      while (delta >= nHeight)
      {
        step++;
        delta -= nHeight;
      }
    }
    SAFE_DELETE_ARRAY(aLine);
  }

  //template<typename _Ty>
  //void Image::IntStretchCopyMulti(Image* pDestImage, int nWidth, int nHeight, int nCount)
  //{
  //  int* aLine = new int[nWidth];
  //  const float fStrideH = (float)m_width / (float)nWidth;
  //  //const float fStrideV = (float)m_height / (float)nHeight;
  //  float fx = 0;
  //  float fy = 0;

  //  for (int x = 0; x < nWidth; x++) {
  //    aLine[x] = (int)floor(fx + 0.5f);
  //    fx += fStrideH;
  //  }

  //  for (int y = 0; y < nHeight; y++)
  //  {
  //    const _Ty* pSrcData = (const _Ty*)GetLine((int)floor(fy + 0.5f));
  //    _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
  //    for (int x = 0; x < nWidth; x++)
  //    {
  //      for (int i = 0; i < nCount; i++)
  //      {
  //        *pDestData = pSrcData[aLine[x]] + i;
  //        pDestData++;
  //      }
  //    }
  //  }
  //  SAFE_DELETE_ARRAY(aLine);
  //}

  const char* Image::GetFormat() const
  {
    return reinterpret_cast<const char*>(m_format.name);
  }

  template<typename _TChannel>
  void Image::ChangeFormat(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch)
  {
    switch (nNewChannel)
    {
    case 1:
    {
      struct PIXEL { _TChannel channel[1]; };
      ChangeFormat2<PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 2:
    {
      struct PIXEL { _TChannel channel[2]; };
      ChangeFormat2<PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 3:
    {
      struct PIXEL { _TChannel channel[3]; };
      ChangeFormat2<PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 4:
    {
      struct PIXEL { _TChannel channel[4]; };
      ChangeFormat2<PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    default:
      break;
    }
  }

  template<typename _TDstPixel, typename _TChannel>
  void Image::ChangeFormat2(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch)
  {
    switch (m_channel)
    {
    case 1:
    {
      struct PIXEL { _TChannel channel[1]; };
      ChangePixel<_TDstPixel, PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 2:
    {
      struct PIXEL { _TChannel channel[2]; };
      ChangePixel<_TDstPixel, PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 3:
    {
      struct PIXEL { _TChannel channel[3]; };
      ChangePixel<_TDstPixel, PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 4:
    {
      struct PIXEL { _TChannel channel[4]; };
      ChangePixel<_TDstPixel, PIXEL, _TChannel>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    default:
      break;
    }
  }


  template<typename _TDstPixel, typename _TSrcPixel, typename _TChannel>
  void Image::ChangePixel(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch) const
  {
    _TDstPixel*pDstPixel;
    _TSrcPixel* pSrcPixel;
    _TSrcPixel  tmp; // 防止pDstPixel与pSrcPixel地址相同
    const size_t nSrcPixelSize = GETPIXELSIZE;

    for (int y = 0; y < m_height; y++)
    {
      pSrcPixel = (_TSrcPixel*)GetLine(y);
      pDstPixel = (_TDstPixel*)(pDestData + y * nNewPitch);
      for (int x = 0; x < m_width; x++)
      {
        tmp = *pSrcPixel;
        for (int c = 0; c < nNewChannel; c++)
        {
          (*pDstPixel).channel[c] = (aMapTab[c] >= 0)
            ? tmp.channel[aMapTab[c]]
            : (_TChannel)(aMapTab[c] + 1);
        }
        pDstPixel++;
        pSrcPixel++;
        //pSrcPixel = (_TSrcPixel*)((size_t)pSrcPixel + nSrcPixelSize);
      }
    }
  }

  b32 Image::SetFormat(const char* fmt)
  {
    //u32 NewFmt = 0;
    PIXELFORMAT NewFormat;
    int nNewChannel = 0;
    if( ! IntParseFormat(fmt, &NewFormat.code, &nNewChannel)) {
      // ERROR: bad format
      return FALSE;
    }

    if (NewFormat.code == m_format.code) {
      // same format
      return TRUE;
    }

    int nChannelMapTab[4] = { -1, -1, -1, -1 }; // 新通道在旧通道上的索引

    for (int i = 0; i < nNewChannel; i++)
    {
      nChannelMapTab[i] = IntGetChanelIndex(m_format, m_channel, NewFormat.name[i]);
    }

    if (nChannelMapTab[0] < 0 && nChannelMapTab[1] < 0 && nChannelMapTab[2] < 0 && nChannelMapTab[3] < 0) {
      return FALSE;
    }

    int nNewPitch = MIN_PITCH_PARAM(m_width, nNewChannel, m_depth);
    if(nNewChannel <= m_channel)
    {
      switch (m_depth)
      {
      case 8:
        ChangeFormat<u8>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      case 16:
        ChangeFormat<u16>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      case 32:
        ChangeFormat<u32>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      default:
        break;
      }
    }
    else {
      const int nNewSize = nNewPitch * m_height;
      CLBYTE* pNewPtr = new CLBYTE[nNewSize];

      switch(m_depth)
      {
      case 8:
        ChangeFormat<u8>(nChannelMapTab, nNewChannel, pNewPtr, nNewPitch);
        break;
      case 16:
        ChangeFormat<u16>(nChannelMapTab, nNewChannel, pNewPtr, nNewPitch);
        break;
      case 32:
        ChangeFormat<u32>(nChannelMapTab, nNewChannel, pNewPtr, nNewPitch);
        break;
      default:
        break;
      }

      delete[] m_ptr;
      m_ptr = pNewPtr;
      //CLBREAK; // 暂时不支持
    }

    m_pitch = nNewPitch;
    m_channel = nNewChannel;
    m_format.code = NewFormat.code;

    return TRUE;
  }

  ImageColorSpace Image::GetColorSpace() const
  {
    return IntGetColorSpace(m_format, m_channel);
  }

  template<typename _TChannel>
    b32 Image::RGBAToYUVA(CLLPBYTE pDestPtr, int nNewPixelSize, int nNewPitch, float mulval, int maxval, int* nChannelTable)
    {
      _TChannel Alpha = 0;
      float r, g, b;
      float y, u, v;
      float inv_mulval = 1.0f / mulval;
      const size_t pixel_size = GETPIXELSIZE;
      ASSERT(nNewPixelSize * m_width <= nNewPitch);

      for (int yy = 0; yy < m_height; yy++)
      {
        _TChannel* pDest  = (_TChannel*)(pDestPtr + nNewPitch * yy);
        _TChannel* pPixel = (_TChannel*)GetLine(yy);
        for (int x = 0; x < m_width; x++)
        {
          r = nChannelTable[0] < 0 ? 0.0f : pPixel[nChannelTable[0]] * inv_mulval;
          g = nChannelTable[1] < 0 ? 0.0f : pPixel[nChannelTable[1]] * inv_mulval;
          b = nChannelTable[2] < 0 ? 0.0f : pPixel[nChannelTable[2]] * inv_mulval;
          if (nChannelTable[3] >= 0)
          {
            Alpha = pPixel[nChannelTable[3]];
            pDest[3] = Alpha;
          }
          y = 0.299f * r + 0.587f * g + 0.114f * b;
          u = -0.147f * r - 0.289f * g + 0.436f * b;
          v = 0.615f * r - 0.515f * g - 0.100f * b;
          pDest[0] = (y <= 0.0f) ? 0 : ((y >= 1.0f) ? (_TChannel)maxval : (_TChannel)(y * mulval));
          pDest[1] = (u <= 0.0f) ? 0 : ((u >= 1.0f) ? (_TChannel)maxval : (_TChannel)(u * mulval));
          pDest[2] = (v <= 0.0f) ? 0 : ((v >= 1.0f) ? (_TChannel)maxval : (_TChannel)(v * mulval));

          pDest += nNewPixelSize;
          pPixel += pixel_size;
        }
      }
      return TRUE;
    }

  b32 Image::SetColorSpace(ImageColorSpace eSpace)
  {
    ImageColorSpace eCurrSpace = IntGetColorSpace(m_format, m_channel);
    if (eCurrSpace == ImageColorSpace_Mix || eCurrSpace == ImageColorSpace_Unknown || eCurrSpace == eSpace)
    {
      return FALSE;
    }

    if(eCurrSpace == ImageColorSpace_RGB && eSpace == ImageColorSpace_YUV)
    {
      int nChannelTab[4] = { -1, -1, -1, -1 }; // 新通道在旧通道上的索引
      nChannelTab[0] = IntGetChanelIndex(m_format, m_channel, 'R');
      nChannelTab[1] = IntGetChanelIndex(m_format, m_channel, 'G');
      nChannelTab[2] = IntGetChanelIndex(m_format, m_channel, 'B');
      nChannelTab[3] = IntGetChanelIndex(m_format, m_channel, 'A');
      int nNewChannel = nChannelTab[3] >= 0 ? 4 : 3; // YUVA or YUV
      int nNewPitch = MIN_PITCH_PARAM(m_width, nNewChannel, m_depth);

      CLBYTE* pNewPtr = m_ptr;
      if (nNewChannel > m_channel) {
        const size_t nNewSize = nNewPitch * m_height;
        pNewPtr = new CLBYTE[nNewSize];
      }

      b32 result = FALSE;
      switch (m_depth)
      {
      case 8:
        result = RGBAToYUVA<u8>(pNewPtr, (m_depth * nNewChannel) >> 3, nNewPitch, 255.0f, 255, nChannelTab);
        break;
      case 16:
        result = RGBAToYUVA<u16>(pNewPtr, (m_depth * nNewChannel) >> 3, nNewPitch, 65535.0f, 65535, nChannelTab);
        break;
      case 32:
        result = FALSE;
        break;
      }

      if(pNewPtr != m_ptr)
      {
        ASSERT(m_pitch != nNewPitch && m_channel != nNewChannel);
        delete m_ptr;
        m_ptr = pNewPtr;
        m_pitch = nNewPitch;
        m_channel = nNewChannel;
      }
      m_format.code = nChannelTab[3] >= 0 ? CLMAKEFOURCC('Y', 'U', 'V', 'A') : CLMAKEFOURCC('Y', 'U', 'V', 0);
      return result;
    }
    CLBREAK;
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  b32 ImageFilterF::Set( int nWidth, int nHeight, const char* fmt, const void* pData )
  {
    return Image::Set(nWidth, nHeight, fmt, 32, pData, 0);
  }

  b32 ImageFilterF::Set( ImageFilterI8* pSrcImage, float fLevel /*= (1.0f / 255.0f)*/ )
  {
    if( ! Image::Set(pSrcImage->GetWidth(), pSrcImage->GetHeight(), pSrcImage->GetFormat(), 32, NULL, 0)) {
      return FALSE;
    }

    float* pDestData;
    u8* pSrcData;

    if(m_channel == 1)
    {
      for(int y = 0; y < m_height; y++)
      {
        pDestData = (float*)GetLine(y);
        pSrcData = (u8*)(pSrcImage->GetLine(y));
        for(int x = 0; x < m_width; x++)
        {
          *pDestData++ = (float)*pSrcData * fLevel;
          pSrcData++;
        }
      }
    }
    else {
      for(int y = 0; y < m_height; y++)
      {
        pDestData = (float*)GetLine(y);
        pSrcData = (u8*)(pSrcImage->GetLine(y));
        for(int x = 0; x < m_width; x++)
        {
          for(int n = 0; n < m_channel; n++) {
            *pDestData++ = (float)*pSrcData * fLevel;
            pSrcData++;
          }
        }
      }
    }
    return TRUE;
  }

  b32 ImageFilterF::Clear( float value, char chChannel )
  {
    int nOffset = GetChannelOffset(chChannel);
    if(nOffset < 0) {
      return FALSE;
    }

    const int nPixelSize = GETPIXELSIZE;
    for(int y = 0; y < m_height; y++)
    {
      float* pData = (float*)((CLLPBYTE)GetLine(y) + nOffset);
      for(int x = 0; x < m_width; x++)
      {
        *pData = value;
        pData = (float*)((CLLPBYTE)pData + nPixelSize);
      }
    }
    return TRUE;
  }

} // namespace clstd