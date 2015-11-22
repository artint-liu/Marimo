#include "clstd.h"
#include "clImage.h"
const int delta = 'a' - 'A';

#define CMP_CHANNEL(_IDX)                           (m_format.name[_IDX] == fmt[_IDX] || m_format.name[_IDX] + delta == fmt[_IDX])
#define MIN_PITCH_PARAM(_WIDTH, _CHANNEL, _DEPTH)   ((_WIDTH) * (_CHANNEL) * ((_DEPTH) >> 3))
#define MIN_PITCH                                   MIN_PITCH_PARAM(m_width, m_channel, m_depth)
#define GETPIXELSIZE                                (m_channel * (m_depth >> 3))
namespace clstd
{
  Image::Image()
    : m_ptr     (NULL)
    , m_width   (0)
    , m_height  (0)
    , m_channel (0)
		, m_pitch(0)
		, m_depth(0)
  {
    m_format.code = 0;
  }

  Image::~Image()
  {
    SAFE_DELETE_ARRAY(m_ptr);
  }

  int Image::GetWidth() const
  {
    return m_width;
  }

  int Image::GetHeight() const
  {
    return m_height;
  }

  const void* Image::GetPixelPtr( int x, int y ) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x;
  }

  void* Image::GetPixelPtr( int x, int y )
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x;
  }

  b32 Image::Set( int nWidth, int nHeight, const char* fmt, const void* pData )
  {
    if(m_width != nWidth || m_height != nHeight || m_depth != 8 || 
      MIN_PITCH_PARAM(nWidth, m_channel, m_depth) != m_pitch || ! CompareFormat(fmt))
    {
      if( ! IntParseFormat(fmt, &m_format.code, &m_channel)) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width  = nWidth;
      m_height = nHeight;
      m_pitch  = MIN_PITCH;
      m_depth  = 8;
      m_ptr    = new u8[m_pitch * m_height];
    }

    if(pData) {
      memcpy(m_ptr, pData, m_pitch * m_height);
    }
    return true;
  }

  b32 Image::Set( int nWidth, int nHeight, const char* fmt, int nPitch, int nDepth, const void* pData )
  {
    if(m_width != nWidth || m_height != nHeight || m_depth != nDepth || 
      m_pitch != nPitch || ! CompareFormat(fmt))
    {
      u32 fmtcode;
      int channel;
      if( ! IntParseFormat(fmt, &fmtcode, &channel)) {
        return FALSE;
      }

      if(nPitch == 0) {
        nPitch = MIN_PITCH_PARAM(nWidth, channel, nDepth);
      }
      else if(MIN_PITCH_PARAM(nWidth, channel, nDepth) < nPitch) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width   = nWidth;
      m_height  = nHeight;
      m_pitch   = nPitch;
      m_channel = channel;
      m_depth   = nDepth;
      m_ptr     = new u8[m_pitch * m_height];
      m_format.code = fmtcode;
    }

    if(pData) {
      memcpy(m_ptr, pData, m_pitch * m_height);
    }
    return true;

  }

  b32 Image::CompareFormat( const char* fmt ) const
  {
    switch(m_channel)
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

    for(int i = 0; i < 4; i++)
    {
      if(fmt[i] == 'R' || fmt[i] == 'G' || fmt[i] == 'B' || fmt[i] == 'A' || fmt[i] == 'X') {
        format.name[i] = fmt[i];
      }
      else if(fmt[i] == 'r' || fmt[i] == 'g' || fmt[i] == 'b' || fmt[i] == 'a' || fmt[i] == 'x') {
        format.name[i] = fmt[i] - ('a' - 'A');
      }
      else if(fmt[i] == '\0') { break; }
      else { return FALSE; }
      channel++;
    }

    // 这样写如果返回false，则不会修改fmtcode和channel的任何内容
    *pChannel = channel;
    *pFmtCode = format.code;
    return TRUE;
  }

  const void* Image::GetLine( int y ) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  void* Image::GetLine( int y )
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  int Image::Inflate( int left, int top, int right, int bottom )
  {
    // 没实现
    CLBREAK;
    return 0;
  }

  int Image::GetChannelOffset( char chChannel )
  {
    for(int i = 0; i < m_channel; i++)
    {
      const u8& c = m_format.name[i];
      if(c == 0x20 || c == '\0') {
        break;
      }
      else if(c == chChannel || c == chChannel + delta) {
        return i * (m_depth >> 3);
      }
    }
    return -1;
  }

  b32 Image::GetChannelPlane( Image* pDestImage, char chChannel )
  {
    int nOffset = GetChannelOffset(chChannel);
    if(nOffset < 0) {
      return FALSE;
    }

    char fmt[8] = {0};
    fmt[0] = chChannel;

    if( ! pDestImage->Set(m_width, m_height, fmt, 0, m_depth, NULL) ) {
      return FALSE;
    }
    const int nPixelSize = GETPIXELSIZE;

    switch(m_depth)
    {
    case 8:
      IntCopyChannel<u8>(pDestImage, nOffset, nPixelSize);
      break;

    case 16:
      IntCopyChannel<u16>(pDestImage, nOffset, nPixelSize);
      break;

    case 32:
      IntCopyChannel<u32>(pDestImage, nOffset, nPixelSize);
      break;

    default:
      CLBREAK;
      return FALSE;
    }
    return TRUE;
  }

  b32 Image::ScaleNearest( Image* pDestImage, int nWidth, int nHeight )
  {
    if( ! pDestImage->Set(nWidth, nHeight, (const char*)m_format.name, 0, m_depth, NULL)) {
      return FALSE;
    }

    // TODO: Copy ...
    switch(GETPIXELSIZE)
    {
    case 1: // 8 depth 1 channel
      IntStretchCopy<u8>(pDestImage, nWidth, nHeight);
      break;

    case 2: // 16 depth 1 channel
            //  8 depth 2 channel
      IntStretchCopy<u16>(pDestImage, nWidth, nHeight);
      break;

    case 3: // 8 depth 3 channel
      IntStretchCopyMulti<u8>(pDestImage, nWidth, nHeight, 3);
      break;

    case 4: // 32 depth 1 channel
            // 16 depth 2 channel
            //  8 depth 4 channel
      IntStretchCopy<u32>(pDestImage, nWidth, nHeight);
      break;

    case 6: // 16 depth 3 channel
      IntStretchCopyMulti<u16>(pDestImage, nWidth, nHeight, 3);
      break;

    case 8: // 16 depth 4 channel
            // 32 depth 2 channel
      IntStretchCopy<u64>(pDestImage, nWidth, nHeight);
      break;

    case 12: // 32 depth 3 channel
      IntStretchCopyMulti<u32>(pDestImage, nWidth, nHeight, 3);
      break;

    case 16: // 32 depth 4 channel
      IntStretchCopyMulti<u32>(pDestImage, nWidth, nHeight, 4);
      break;

    default:
      CLBREAK;
      CLOG_ERROR("%s: Unsupported pixel size.\r\n", __FUNCTION__);
      break;
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  void Image::BlockTransfer( IMAGEDESC* pDest, int x, int y, int nCopyWidth, int nCopyHeight, const IMAGEDESC* pSrc )
  {

  }

  template<typename _Ty>
  void Image::IntCopyChannel( Image* pDestImage, int nOffset, const int nPixelSize )
  {
    _Ty* pDestData;
    _Ty* pSrcData;
    for(int y = 0; y < m_height; y++)
    {
      pDestData = (_Ty*)((CLLPBYTE)pDestImage->GetLine(y));
      pSrcData = (_Ty*)((CLLPBYTE)GetLine(y) + nOffset);
      for(int x = 0; x < m_width; x++)
      {
        *pDestData++ = *pSrcData;
        pSrcData = (_Ty*)((CLLPBYTE)pSrcData + nPixelSize);
      }
    }
  }

  template<typename _Ty>
  void Image::IntStretchCopy( Image* pDestImage, int nWidth, int nHeight )
  {
    int* aLine = new int[nWidth];
    const float fStrideH = (float)m_width / (float)nWidth;
    const float fStrideV = (float)m_height / (float)nHeight;
    float fx = 0;
    float fy = 0;

    for(int x = 0; x < nWidth; x++) {
      aLine[x] = (int)floor(fx + 0.5f);
      fx += fStrideH;
    }

    for(int y = 0; y < nHeight; y++)
    {
      const _Ty* pSrcData = (const _Ty*)GetLine((int)floor(fy + 0.5f));
      _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
      for(int x = 0; x < nWidth; x++)
      {
        *pDestData = pSrcData[aLine[x]];
        pDestData++;
      }
      fy += fStrideV;
    }
    SAFE_DELETE_ARRAY(aLine);
  }

  template<typename _Ty>
  void Image::IntStretchCopyMulti( Image* pDestImage, int nWidth, int nHeight, int nCount )
  {
    int* aLine = new int[nWidth];
    const float fStrideH = (float)m_width / (float)nWidth;
    const float fStrideV = (float)m_height / (float)nHeight;
    float fx = 0;
    float fy = 0;

    for(int x = 0; x < nWidth; x++) {
      aLine[x] = (int)floor(fx + 0.5f);
      fx += fStrideH;
    }

    for(int y = 0; y < nHeight; y++)
    {
      const _Ty* pSrcData = (const _Ty*)GetLine((int)floor(fy + 0.5f));
      _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
      for(int x = 0; x < nWidth; x++)
      {
        for(int i = 0; i < nCount; i++)
        {
          *pDestData = pSrcData[aLine[x] + i];
          pDestData++;
        }
      }
    }
    SAFE_DELETE_ARRAY(aLine);
  }

  const char* Image::GetFormat() const
  {
    return (const char*)m_format.name;
  }

  //////////////////////////////////////////////////////////////////////////

  b32 ImageFilterF::Set( int nWidth, int nHeight, const char* fmt, const void* pData )
  {
    return Image::Set(nWidth, nHeight, fmt, 0, 32, pData);
  }

  b32 ImageFilterF::Set( ImageFilterI8* pSrcImage, float fLevel /*= (1.0f / 255.0f)*/ )
  {
    if( ! Image::Set(pSrcImage->GetWidth(), pSrcImage->GetHeight(), pSrcImage->GetFormat(), 0, 32, NULL)) {
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