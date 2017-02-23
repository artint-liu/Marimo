
#include "clstd.h"
#include "clString.h"
#include "clCrypt.h"
//#include "clBuffer.h"

namespace clstd
{
  MD5Calculater::MD5Calculater()
  {
    Clear();
  }

  void MD5Calculater::Clear()
  {
    memset(&ctx, 0, sizeof(ctx));
    MD5Init(&ctx);
  }

  //void MD5Calculater::End()
  //{
  //  MD5Final(&ctx);
  //}
  void MD5Calculater::CheckFinal()
  {
    if(*(u32*)&ctx.digest[0] == 0 && *(u32*)&ctx.digest[4] == 0 &&
      *(u32*)&ctx.digest[8] == 0 && *(u32*)&ctx.digest[12] == 0)
    {
      MD5Final(&ctx);
    }
  }

  void MD5Calculater::Update(const void* pBuffer, clsize cbSize)
  {
    MD5Update(&ctx, (const u8*)pBuffer, (unsigned long)cbSize);
  }

  void MD5Calculater::Update(const BufferBase* pBuffer)
  {
    Update((u8*)pBuffer->GetPtr(), pBuffer->GetSize());
  }

  b32 MD5Calculater::operator=(const MD5Calculater& md5) const
  {
    return Compare(md5) == 0;
  }

  int MD5Calculater::Compare(const MD5Calculater& md5) const
  {
    i8 r = 0;
    for(int i = 0; i < 16; i++) {
      r = (i8)ctx.digest[i] - (i8)md5.ctx.digest[i];
      if(r != 0) {
        break;
      }
    }
    return (int)r;
  }

  u32* MD5Calculater::Get128Bits()
  {
    CheckFinal();
    return (u32*)ctx.digest;
  }

  clStringA MD5Calculater::GetAsGUIDA()
  {
    clStringA t;
    CheckFinal();
    t.Format("%08X-%04X-%04X-%2X%2X-%2X%2X%2X%2X%2X%2X",
      *(u32*)ctx.digest, (u32)*(u16*)&ctx.digest[4], (u32)*(u16*)&ctx.digest[6],
      ctx.digest[8], ctx.digest[9], ctx.digest[10], ctx.digest[11], ctx.digest[12], 
      ctx.digest[13], ctx.digest[14],ctx.digest[15]);
    return t;
  }

  clStringW MD5Calculater::GetAsGUIDW()
  {
    clStringW t;
    CheckFinal();
    t.Format(_CLTEXT("%08X-%04X-%04X-%2X%2X-%2X%2X%2X%2X%2X%2X"),
      *(u32*)ctx.digest, (u32)*(u16*)&ctx.digest[4], (u32)*(u16*)&ctx.digest[6],
      ctx.digest[8], ctx.digest[9], ctx.digest[10], ctx.digest[11], ctx.digest[12],
      ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return t;
  }

  clStringA MD5Calculater::ToStringA()
  {
    clStringA t;
    CheckFinal();
    t.Format("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
      ctx.digest[0], ctx.digest[1], ctx.digest[2], ctx.digest[3],
      ctx.digest[4], ctx.digest[5], ctx.digest[6], ctx.digest[7],
      ctx.digest[8], ctx.digest[9], ctx.digest[10], ctx.digest[11],
      ctx.digest[12], ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return t;
  }

  clStringW MD5Calculater::ToStringW()
  {
    clStringW t;
    CheckFinal();
    t.Format(_CLTEXT("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"),
      ctx.digest[ 0], ctx.digest[ 1], ctx.digest[ 2], ctx.digest[ 3], 
      ctx.digest[ 4], ctx.digest[ 5], ctx.digest[ 6], ctx.digest[ 7], 
      ctx.digest[ 8], ctx.digest[ 9], ctx.digest[10], ctx.digest[11], 
      ctx.digest[12], ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return t;
  }

} // namespace clstd