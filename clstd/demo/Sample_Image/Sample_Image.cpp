// Sample_Image.cpp : 定义控制台应用程序的入口点。
//

#include "clstd.h"
#include "clstring.h"
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

void SaveImage(const Image& image, const ch* szFilename)
{
  TGAHEADER header = {};
  if(image.GetChannelDepth() == 8)
  {
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
  else
  {
    Image image8 = image;
    image8.SetChannelDepth(8);
    SaveImage(image8, szFilename);
  }
}

struct MYRGB
{
  u8 b, g, r;
};

struct MYRGBX
{
  u8 b, g, r, x;
};

struct MYRGB16
{
  u16 b, g, r;
};

void BuildAOX(Image& image)
{
  image.Set(256, 256, "BGR");
  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p)
  {
    p.b = x & y;
    p.g = x | y;
    p.r = x ^ y;
  });
}

void TestBaseLinearScale()
{
  Image image;
  Image dest_image;

  image.Set(64, 64, "BGR");
  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p) {
    if(x >= 15 && x <= 16 && y >= 15 && y <= 16) {
      p.r = p.g = p.b = 0;
    }
    else if(x == 31 || x == 32 || y == 31 || y ==32) {
      p.r = p.g = p.b = 0;
    } else if(x == 0) {
      p.r = 255;
      p.g = p.b = 0;
    }
    else if(y == 63) {
      p.b = 255;
      p.g = p.r = 0;
    }
    else if (x == 63) {
      p.g = 0;
      p.b = p.r = 255;
    } else {
      p.r = p.g = p.b = 255;
    }
  });
  SaveImage(image, "base-v-64.tga");

  image.ScaleFastLinear(&dest_image, 128, 128);
  SaveImage(dest_image, "base-v-128-linear.tga");

  image.ScaleFastLinear(&dest_image, 256, 256);
  SaveImage(dest_image, "base-v-256-linear.tga");

  image.ScaleFastLinear(&dest_image, 512, 512);
  SaveImage(dest_image, "base-v-512-linear.tga");


  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p) {
    if (y == 31 || y == 32) {
      p.r = p.g = p.b = 0;
    }
    else {
      p.r = p.g = p.b = 255;
    }
  });

  SaveImage(image, "base-h-64.tga");

  image.ScaleFastLinear(&dest_image, 512, 512);
  SaveImage(dest_image, "base-h-512-linear.tga");
}

void TestGrid1()
{
  Image image;
  image.Set(256, 256, "BGR");

  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p) {
    if (x & 1) {
      p.r = 255;
      p.g = 255;
      p.b = 255;
    }
    else {
      if (x & 2)
      {
        p.r = 0;
        p.g = 0;
        p.b = 255;
      }
      else {
        p.r = 255;
        p.g = 0;
        p.b = 0;
      }
    }
  });

  SaveImage(image, "TestGrid1-256.tga");
  Image dest_image;
  int nTestArray[] = { 512, 1024, 2048, 300, 350, 400, 450, 500, 550};

  for (int i = 0; i < countof(nTestArray); i++)
  {
    clStringA str;
    image.ScaleFastLinear(&dest_image, nTestArray[i], nTestArray[i]);
    str.Format("TestGrid1-%d-to-%d-linear.tga", image.GetWidth(), nTestArray[i]);
    SaveImage(dest_image, str);
  }
}

void TestGrid2()
{
  Image image;
  image.Set(1024, 1024, "BGR");

  image.ForEachPixelXY<MYRGB>([](int x, int y, MYRGB& p) {
    if (x & 1) {
      p.r = p.g = p.b = 255;
    } else {
      if (x & 2)
      {
        p.r = p.g = 0;
        p.b = 255;
      } else {
        p.r = 255;
        p.g = p.b = 0;
      }
    }
  });

  SaveImage(image, "TestGrid2-1024.tga");

  Image dest_image;
  int nTestArray[] = {512, 256, 128, 64, 1000, 950, 900, 850, 800, 750, 700, 650, 600, 550, 500, 450, 400};

  for(int i = 0; i < countof(nTestArray); i++)
  {
    clStringA str;
    str.Format("test-%d-to-%d.tga", image.GetWidth(), nTestArray[i]);

    image.ScaleFastLinear(&dest_image, nTestArray[i], nTestArray[i]);
    SaveImage(dest_image, str);
  }
}

void TestGrid2_16()
{
  Image image;
  image.Set(1024, 1024, "BGR", 16);

  image.ForEachPixelXY<MYRGB16>([](int x, int y, MYRGB16& p) {
    if (x & 1) {
      p.r = p.g = p.b = 0xffff;
    }
    else {
      if (x & 2)
      {
        p.r = p.g = 0;
        p.b = 0xffff;
      }
      else {
        p.r = 0xffff;
        p.g = p.b = 0;
      }
    }
  });

  SaveImage(image, "TestGrid2-16bits-1024.tga");

  Image dest_image;
  int nTestArray[] = { 512, 256, 128, 64, 1000, 950, 900, 850, 800, 750, 700, 650, 600, 550, 500, 450, 400 };

  for (int i = 0; i < countof(nTestArray); i++)
  {
    clStringA str;
    str.Format("test-16bits-%d-to-%d.tga", image.GetWidth(), nTestArray[i]);

    image.ScaleFastLinear(&dest_image, nTestArray[i], nTestArray[i]);
    SaveImage(dest_image, str);
  }
}


void Test16Bits()
{
  Image image;
  image.Set(256, 256, "BGR", 16);
  image.ForEachPixelXY<MYRGB16>([](int x, int y, MYRGB16& p)
  {
    p.b = (x & y) << 8;
    p.g = (x | y) << 8;
    p.r = (x ^ y) << 8;
  });
  SaveImage(image, "AOX16.tga");
}

int main()
{
  Image imageAOX;
  Image dest_image;

  BuildAOX(imageAOX);
  SaveImage(imageAOX, "AOX.tga");

  TestGrid1();
  TestGrid2();
  Test16Bits();
  TestBaseLinearScale();
  TestGrid2_16();



  //////////////////////////////////////////////////////////////////////////
  imageAOX.ScaleNearest(&dest_image, 512, 512);
  SaveImage(dest_image, "aox-512-point.tga");
  return 0;
}
