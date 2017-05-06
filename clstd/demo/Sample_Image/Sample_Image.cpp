// Sample_Image.cpp : 定义控制台应用程序的入口点。
//

#include "clstd.h"
#include "clImage.h"

using namespace clstd;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#pragma pack(1)               // Dont pad the following struct.
#endif

struct TGAHEADER
{
  u8  idLength;           // Length of optional identification sequence.
  u8  paletteType;        // Is a palette present? (1=yes)
  u8  imageType;          // Image data type (0=none, 1=indexed, 2=rgb,
                        // 3=grey, +8=rle packed).
  u16 firstPaletteEntry;  // First palette index, if present.
  u16 numPaletteEntries;  // Number of palette entries, if present.
  u8  paletteBits;        // Number of bits per palette entry.
  u16 x;                  // Horiz. pixel coord. of lower left of image.
  u16 y;                  // Vert. pixel coord. of lower left of image.
  u16 width;              // Image width in pixels.
  u16 height;             // Image height in pixels.
  u8  depth;              // Image color depth (bits per pixel).
  u8  descriptor;         // Image attribute flags.
};

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

void SaveImage(Image& image, const ch* szFilename)
{
  TGAHEADER header = {};
  header.imageType = 2;
  header.width = image.GetWidth();
  header.height = image.GetHeight();
  header.depth = image.GetChannelDepth() * image.GetChannels();
  header.descriptor = 0x20;

  File file;
  if (file.CreateAlways(szFilename))
  {
    file.Write(&header, sizeof(TGAHEADER));
    file.Write(image.GetLine(0), image.GetDataSize());
  }
}

int main()
{
  Image image;
  image.Set(256, 256, "RGB");
  struct MYRGB
  {
    u8 R, G, B;
  };
  struct MYRGBX
  {
    u8 R, G, B, X;
  };

  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p){
    if (x & 1) {
      p.R = 255;
      p.G = 255;
      p.B = 255;
    }
    else {
      if(x & 2)
      {
        p.R = 0;
        p.G = 0;
        p.B = 255;
      }
      else {
        p.R = 255;
        p.G = 0;
        p.B = 0;
      }
    }
  });
  SaveImage(image, "test-256.tga");
  Image dest_image;
  image.ScaleNearest(&dest_image, 512, 512);
  SaveImage(dest_image, "test-512.tga");
  return 0;
}
