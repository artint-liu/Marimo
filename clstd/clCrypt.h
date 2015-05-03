#ifndef _CL_CRYPT_H_
#define _CL_CRYPT_H_

//struct MD5_CTX;
class clBufferBase;
struct MD5_CTX
{
  unsigned int i[2];
  unsigned int buf[4];
  unsigned char in[64];
  unsigned char digest[16];
};

struct SHA_CTX;

unsigned char *CRYPT_DEShash( unsigned char *dst, const unsigned char *key, const unsigned char *src );

CLVOID MD5Init      (MD5_CTX* ctx);
CLVOID MD5Update    (MD5_CTX* ctx, const unsigned char *buf, unsigned int len);
CLVOID MD5Final     (MD5_CTX* ctx);

CLVOID A_SHAInit    (SHA_CTX* Context);
CLVOID A_SHAUpdate  (SHA_CTX* Context, const unsigned char *Buffer, CLUINT BufferSize);
CLVOID A_SHAFinal   (SHA_CTX* Context, CLLPULONG Result);

namespace clstd
{
  class MD5Calculater
  {
    MD5_CTX ctx;
    void CheckFinal();
  public:
    MD5Calculater();
    void Clear();
    //void End();
    void Update(const u8* pBuffer, clsize cbSize);
    void Update(const clBufferBase* pBuffer);

    b32       operator=(CLCONST MD5Calculater& md5) CLCONST;
    int       Compare(CLCONST MD5Calculater& md5) CLCONST;  // return 0 if equal

    u32*      Get128Bits();
    clStringA GetAsGUID();// "48961C27-CE71-44CC-8F20-73D3D2A460B5"    {0x48961c27, 0xce71, 0x44cc, 0x8f, 0x20, 0x73, 0xd3, 0xd2, 0xa4, 0x60, 0xb5};
    clStringA ToString();

  };
} // namespace clstd

#endif // _CL_CRYPT_H_