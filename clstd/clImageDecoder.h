#ifndef _CLSTD_IMAGE_DECODER_H_
#define _CLSTD_IMAGE_DECODER_H_

namespace clstd
{
  class Image;
  enum DecodeImageType
  {
    DecodeImageType_ETC1,
  };

  size_t DecodeImage(Image& image, DecodeImageType eType, int width, int height, const void* ptr, size_t len);
} // namespace clstd

#endif // _CLSTD_IMAGE_DECODER_H_