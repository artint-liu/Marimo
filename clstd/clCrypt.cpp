
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

  MD5Calculater& MD5Calculater::Update(const void* pBuffer, clsize cbSize)
  {
    MD5Update(&ctx, (const u8*)pBuffer, (unsigned long)cbSize);
    return *this;
  }

  MD5Calculater& MD5Calculater::Update(const BufferBase* pBuffer)
  {
    Update((u8*)pBuffer->GetPtr(), pBuffer->GetSize());
    return *this;
  }

  b32 MD5Calculater::operator==(const MD5Calculater& md5) const
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

  clStringA& MD5Calculater::ToStringA(clStringA& str)
  {
    CheckFinal();
    str.Format("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
      ctx.digest[0], ctx.digest[1], ctx.digest[2], ctx.digest[3],
      ctx.digest[4], ctx.digest[5], ctx.digest[6], ctx.digest[7],
      ctx.digest[8], ctx.digest[9], ctx.digest[10], ctx.digest[11],
      ctx.digest[12], ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return str;
  }
  
  clStringW& MD5Calculater::ToStringW(clStringW& str)
  {
    CheckFinal();
    str.Format(_CLTEXT("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"),
      ctx.digest[0], ctx.digest[1], ctx.digest[2], ctx.digest[3],
      ctx.digest[4], ctx.digest[5], ctx.digest[6], ctx.digest[7],
      ctx.digest[8], ctx.digest[9], ctx.digest[10], ctx.digest[11],
      ctx.digest[12], ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return str;
  }

  clStringA MD5Calculater::ToStringA()
  {
    clStringA t;
    return ToStringA(t);
  }

  clStringW MD5Calculater::ToStringW()
  {
    clStringW t;
    return ToStringW(t);
  }

  //////////////////////////////////////////////////////////////////////////
  void SHACalculater::CheckFinal()
  {
    if(m_State[0] == 0 && m_State[1] == 0 &&
      m_State[2] == 0 && m_State[3] == 0 && m_State[4] == 0)
    {
      A_SHAFinal(&ctx, m_State);
    }
  }

  SHACalculater::SHACalculater()
  {
    InlSetZeroT(m_State);
    A_SHAInit(&ctx);
  }
  
  void SHACalculater::Clear()
  {
    memset(&ctx, 0, sizeof(SHA_CTX));
    InlSetZeroT(m_State);
    A_SHAInit(&ctx);
  }

  SHACalculater& SHACalculater::Update(const void* pBuffer, clsize cbSize)
  {
    A_SHAUpdate(&ctx, (const unsigned char*)pBuffer, (CLUINT)cbSize);
    return *this;
  }

  SHACalculater& SHACalculater::Update(const BufferBase* pBuffer)
  {
    A_SHAUpdate(&ctx, (const unsigned char*)pBuffer->GetPtr(), (CLUINT)pBuffer->GetSize());
    return *this;
  }

  b32 SHACalculater::operator=(const SHACalculater& _sha) const
  {
    return Compare(_sha) == 0;
  }

  int SHACalculater::Compare(const SHACalculater& _sha) const
  {
    i8 r = 0;
    STATIC_ASSERT(countof(ctx.State) == 5);
    for(int i = 0; i < countof(ctx.State); i++) {
      r = (i8)ctx.State[i] - (i8)_sha.ctx.State[i];
      if(r != 0) {
        break;
      }
    }
    return (int)r;
  }

  u32* SHACalculater::Get160Bits()
  {
    CheckFinal();
    return m_State;
  }

  template <class _TStr>
  _TStr& SHACalculater::ToStringT(_TStr& str)
  {
    CheckFinal();
    str.Clear();
    str.Reserve(sizeof(m_State) * 2);
    u8* p = (u8*)m_State;
    for(int i = 0; i < sizeof(m_State); i++)
    {
      u8 c = (*p & 0xf0) >> 4;
      str.Append(c + (c < 10 ? '0' : ('A' - 10)));

      c = (*p++ & 0xf);
      str.Append(c + (c < 10 ? '0' : ('A' - 10)));
    }
    return str;
  }

  clStringA& SHACalculater::ToStringA(clStringA& str)
  {
    return ToStringT(str);
  }
  
  clStringW& SHACalculater::ToStringW(clStringW& str)
  {
    return ToStringT(str);
  }

  clStringA SHACalculater::ToStringA()
  {
    clStringA str;
    return ToStringT(str);
  }

  clStringW SHACalculater::ToStringW()
  {
    clStringW str;
    return ToStringT(str);
  }

} // namespace clstd