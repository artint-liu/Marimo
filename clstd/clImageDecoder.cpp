#include "clstd.h"
#include "clImageDecoder.h"
#include "clImage.h"
#include "clUtility.h"

namespace clstd {
  size_t DecodeImage_ETC1(Image& image, int width, int height, const void* ptr, size_t len);

  size_t DecodeImage(Image& image, DecodeImageType eType, int width, int height, const void* ptr, size_t len)
  {
    if (eType == DecodeImageType_ETC1)
    {
      return DecodeImage_ETC1(image, width, height, ptr, len);
    }
    return FALSE;
  }

#define EXTEND_5TO8BITS(_V) ((_V << 3) | (_V >> 2))
#define EXTEND_4TO8BITS(_V) (_V | (_V << 4))
#define EXTEND_HIGH4TO8BITS(_V) (_V | (_V >> 4))

  void DecodeBlock_ETC1(Image& image, int xx, int yy, const u64* ptr)
  {
    const u64 flip_bit = (u64)1 << 24;
    const u64 diff_bit = (u64)1 << 25;
    int R1, G1, B1;
    int R2, G2, B2;
    int tab_cw[2]; // tab_cw1, tab_cw2
    const size_t pitch = image.GetPitch();
    static int diff[] = {0, 1, 2, 3, -4, -3, -2, -1};

    //a) bit layout in bits 63 through 32 if diffbit = 0
    // 
    // | 7  6  5  4  3  2  1  0|15 14 13 12 11 10  9  8|23 22 21 20 19 18 17 16|31 30 29 28 27 26  25  24  |
    // |63 62 61 60|59 58 57 56|55 54 53 52|51 50 49 48|47 46 45 44|43 42 41 40|39 38 37|36 35 34| 33 |32  |
    //  ---------------------------------------------------------------------------------------------------
    // | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    // | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //  ---------------------------------------------------------------------------------------------------
    //
    //
    //b) bit layout in bits 63 through 32 if diffbit = 1
    //
    // | 7  6  5  4  3  2  1  0|15 14 13 12 11 10  9  8|23 22 21 20 19 18 17 16|31 30 29 28 27 26  25  24  |
    // |63 62 61 60 59|58 57 56|55 54 53 52 51|50 49 48|47 46 45 44 43|42 41 40|39 38 37|36 35 34| 33 |32  |
    //  ---------------------------------------------------------------------------------------------------
    // | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |diff|flip|
    // | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bit |bit |
    //  ---------------------------------------------------------------------------------------------------
    //
    //
    //c) bit layout in bits 31 through 0 (in both cases)
    //
    // | 7  6  5  4  3  2  1  0|15 14 13 12 11 10  9  8|23 22 21 20 19 18 17 16|31 30 29 28 27 26  25  24 |
    // |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3   2   1  0
    // --------------------------------------------------------------------------------------------------
    // |       most significant pixel index bits       |         least significant pixel index bits       |
    // | p| o| n| m| l| k| j| i| h| g| f| e| d| c| b| a| p| o| n| m| l| k| j| i| h| g| f| e| d| c | b | a |
    // -------------------------------------------------------------------------------------------------- 




    static int codeword[][4] = { // 根据文档调整了顺序
      { 2,   8,  -2,  -8, },
      { 5,   17, -5,  -17, },
      { 9,   29, -9,  -29, },
      { 13,  42, -13, -42, },
      { 18,  60, -18, -60, },
      { 24,  80, -24, -80, },
      { 33, 106, -33, -106, },
      { 47, 183, -47, -183, },
    };

    u32 block0 = *(u32*)ptr; // 没有翻转字节序
    u32 block1 = clstd::bitswap(((u32*)ptr)[1]);

    if (TEST_FLAG(block0, diff_bit))
    {
      u32 clr1 = (block0 & 0xf8f8f8) >> 3;
      R1 = ((u8*)&clr1)[0];
      G1 = ((u8*)&clr1)[1];
      B1 = ((u8*)&clr1)[2];

      R2 = R1 + diff[block0 & 7];
      G2 = G1 + diff[(block0 >> 8) & 7];
      B2 = B1 + diff[(block0 >> 16) & 7];

      R1 = EXTEND_5TO8BITS(R1);
      R2 = EXTEND_5TO8BITS(R2);
      G1 = EXTEND_5TO8BITS(G1);
      G2 = EXTEND_5TO8BITS(G2);
      B1 = EXTEND_5TO8BITS(B1);
      B2 = EXTEND_5TO8BITS(B2);
    }
    else {
      u32 clr1 = block0 & 0xf0f0f0;
      u32 clr2 = block0 & 0x0f0f0f;
      clr1 = clr1 | (clr1 >> 4);
      clr2 = clr2 | (clr2 << 4);
      R1 = ((u8*)&clr1)[0];
      R2 = ((u8*)&clr2)[0];
      G1 = ((u8*)&clr1)[1];
      G2 = ((u8*)&clr2)[1];
      B1 = ((u8*)&clr1)[2];
      B2 = ((u8*)&clr2)[2];
    }

    tab_cw[0] = (block0 >> 29) & 7;
    tab_cw[1] = (block0 >> 26) & 7;

    if (TEST_FLAG(block0, flip_bit))
    {
      u8* p = static_cast<u8*>(image.GetPixelPtr(xx, yy));
      for (int x = xx; x < xx + 4; x++)
      {
        for (int y = yy; y < yy + 2; y++)
        {
          int i = (x - xx) * 4 + (y - yy);
          int d = codeword[tab_cw[0]][((block1 >> (15 + i)) & 2) | ((block1 >> i) & 1)];
          *p++ = clClamp(0, 255, R1 + d);
          *p++ = clClamp(0, 255, G1 + d);
          *p   = clClamp(0, 255, B1 + d);
          p += pitch - 2;
        }
        p = p - pitch * 2 + 3;
      }

      p += pitch * 2 - 3 * 4;

      for (int x = xx; x < xx + 4; x++)
      {
        for (int y = yy + 2; y < yy + 4; y++)
        {
          int i = (x - xx) * 4 + (y - yy);
          int d = codeword[tab_cw[1]][((block1 >> (15 + i)) & 2) | ((block1 >> i) & 1)];
          *p++ = clClamp(0, 255, R2 + d);
          *p++ = clClamp(0, 255, G2 + d);
          *p   = clClamp(0, 255, B2 + d);
          p += pitch - 2;
        }
        p = p - pitch * 2 + 3;
      }
    }
    else
    {
      u8* p = static_cast<u8*>(image.GetPixelPtr(xx, yy));
      int i = 0;
      for (int x = xx; x < xx + 2; x++)
      {
        for (int y = yy; y < yy + 4; y++, i++)
        {
          int d = codeword[tab_cw[0]][((block1 >> (15 + i)) & 2) | ((block1 >> i) & 1)];
          *p++ = clClamp(0, 255, R1 + d);
          *p++ = clClamp(0, 255, G1 + d);
          *p   = clClamp(0, 255, B1 + d);
          p += pitch - 2;
        }
        p = p - pitch * 4 + 3;
      }

      for (int x = xx + 2; x < xx + 4; x++)
      {
        for (int y = yy; y < yy + 4; y++, i++)
        {
          int d = codeword[tab_cw[1]][((block1 >> (15 + i)) & 2) | ((block1 >> i) & 1)];
          *p++ = clClamp(0, 255, R2 + d);
          *p++ = clClamp(0, 255, G2 + d);
          *p   = clClamp(0, 255, B2 + d);
          p += pitch - 2;
        }
        p = p - pitch * 4 + 3;
      }
    }
  }


  size_t DecodeImage_ETC1(Image& image, int width, int height, const void* ptr, size_t len)
  {
    int extend_w = ALIGN_4(width);
    int extend_h = ALIGN_4(height);
    const size_t block_count = extend_w * extend_h / (4 * 4);
    const size_t code_len = block_count * 8;

    if (ptr == NULL) {
      return code_len;
    }
    else if (len != code_len) {
      return 0;
    }

    image.Set(width, height, "RGB", 8, NULL, extend_w * 3);

    int xx = 0;
    int yy = 0;
    const u64* pp = static_cast<const u64*>(ptr);
    for (size_t b = 0; b < block_count; b++)
    {
      DecodeBlock_ETC1(image, xx, yy, pp++);
      xx += 4;
      if (xx == extend_w) {
        xx = 0;
        yy += 4;
      }
    }
    return code_len;
  }
} // namespace clstd