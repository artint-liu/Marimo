// 全局头文件
#include <GrapX.H>

// 标准接口
//#include <GrapX/GUnknown.H>
#include <GrapX/GResource.H>
#include <GrapX/GRegion.H>
#include "GrapX/GXUser.H"

// 私有头文件
#include "RegionFunc.H"
#include "GRegionImpl.H"


typedef clvector<GXBYTE> ByteArray;
GXINT Line_SafeSubtract(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  ByteArray aLogic;
  GXLONG i = 0;
  for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
  {
    for(; i < *it; i++)
      aLogic.push_back(0);
    ++it;
    for(; i < *it; i++)
      aLogic.push_back(0xff);
  }
  i = 0;
  for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
  {
    GXLONG l = *it;
    ++it;
    GXLONG r = *it;

    for(i = l; i < r; i++)
    {
      if(i >= (int)aLogic.size())
        break;
      if(aLogic[i] == 0xff)
        aLogic[i] = 0;
    }
  }
  GXBYTE bSign = 0;
  i = 0;
  for(ByteArray::iterator it = aLogic.begin(); it != aLogic.end(); ++it, i++)
  {
    if(bSign != *it)
    {
      aDest.push_back(i);
      bSign = *it;
    }
  }
  if(bSign != 0)
    aDest.push_back(i);
  ASSERT((aDest.size() & 1) == 0);
  return 0;
}

GXINT Line_SafeAnd(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  ByteArray aLogic;
  int i = 0;
  for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
  {
    for(; i < *it; i++)
      aLogic.push_back(0);
    ++it;
    for(; i < *it; i++)
      aLogic.push_back(0x0f);
  }
  i = 0;
  for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
  {
    GXLONG l = *it;
    ++it;
    GXLONG r = *it;

    for(i = l; i < r; i++)
    {
      if(i >= (int)aLogic.size())
        break;
      aLogic[i] |= 0xf0;
    }
  }
  GXBYTE bSign = 0;
  i = 0;
  for(ByteArray::iterator it = aLogic.begin(); it != aLogic.end(); ++it, i++)
  {
    if(*it != 0xff)
      *it = 0;
    if(bSign != *it)
    {
      aDest.push_back(i);
      bSign = *it;
    }
  }
  if(aDest.size() & 1)
    aDest.push_back(i);
  return 0;
}

GXINT Line_SafeXor(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  ByteArray aLogic;
  int i = 0;
  if(aSrc1[aSrc1_size - 1] >= aSrc2[aSrc2_size - 1])
  {
    for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
    {
      for(; i < *it; i++)
        aLogic.push_back(0);
      ++it;
      for(; i < *it; i++)
        aLogic.push_back(0x0f);
    }
    i = 0;
    for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
    {
      GXLONG l = *it;
      ++it;
      GXLONG r = *it;

      for(i = l; i < r; i++)
      {
        if(i >= (int)aLogic.size())
          break;
        aLogic[i] |= 0xf0;
      }
    }
  }
  else
  {
    for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
    {
      for(; i < *it; i++)
        aLogic.push_back(0);
      ++it;
      for(; i < *it; i++)
        aLogic.push_back(0x0f);
    }
    i = 0;
    for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
    {
      GXLONG l = *it;
      ++it;
      GXLONG r = *it;

      for(i = l; i < r; i++)
      {
        if(i >= (int)aLogic.size())
          break;
        aLogic[i] |= 0xf0;
      }
    }
  }

  GXBYTE bSign = 0;
  i = 0;
  for(ByteArray::iterator it = aLogic.begin(); it != aLogic.end(); ++it, i++)
  {
    if(*it == 0xff || *it == 0)
      *it = 0;
    else 
      *it = 0xff;

    if(bSign != *it)
    {
      aDest.push_back(i);
      bSign = *it;
    }
  }
  if(aDest.size() & 1)
    aDest.push_back(i);
  return 0;
}

GXINT Line_SafeOr(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  ByteArray aLogic;
  int i = 0;
  if(aSrc1[aSrc1_size - 1] >= aSrc2[aSrc2_size - 1])
  {
    for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
    {
      for(; i < *it; i++)
        aLogic.push_back(0);
      ++it;
      for(; i < *it; i++)
        aLogic.push_back(0xff);
    }
    i = 0;
    for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
    {
      GXLONG l = *it;
      ++it;
      GXLONG r = *it;

      for(i = l; i < r; i++)
      {
        if(i >= (int)aLogic.size())
          break;
        aLogic[i] = 0xff;
      }
    }
  }
  else
  {
    for(const GXLONG* it = aSrc2; it != &aSrc2[aSrc2_size]; ++it)
    {
      for(; i < *it; i++)
        aLogic.push_back(0);
      ++it;
      for(; i < *it; i++)
        aLogic.push_back(0xff);
    }
    i = 0;
    for(const GXLONG* it = aSrc1; it != &aSrc1[aSrc1_size]; ++it)
    {
      GXLONG l = *it;
      ++it;
      GXLONG r = *it;

      for(i = l; i < r; i++)
      {
        if(i >= (int)aLogic.size())
          break;
        aLogic[i] = 0xff;
      }
    }
  }

  GXBYTE bSign = 0;
  i = 0;
  for(ByteArray::iterator it = aLogic.begin(); it != aLogic.end(); ++it, i++)
  {
    if(bSign != *it)
    {
      aDest.push_back(i);
      bSign = *it;
    }
  }
  aDest.push_back(i);
  return 0;
}

GXLONG* __cpp_Line_Copy(LongFixedArray& aDest, const GXLONG* aSrc1, const size_t aSrc1_size)
{
  size_t s1 = 0;
  for(; s1 < aSrc1_size; s1++)
  {
    aDest.push_back(aSrc1[s1]);
  }
  return NULL;
}

#if defined(_WIN32) && defined(_X86)
__declspec( naked ) GXLONG* __asm_Line_Copy(LongFixedArray& aDest, const GXLONG* aSrc1, const size_t aSrc1_size)
{
  __asm {
    push  edi
    mov    edi, [esp + 8]    // aDest
    mov    ecx, [esp + 16]    // aSrc_size
    jecxz  F_RET
    mov    eax, [edi] + LongFixedArray.m_size
    add    [edi] + LongFixedArray.m_size, ecx
    mov    edx, [esp + 12]    // aSrc1
    mov    edi, [edi] + LongFixedArray.m_ptr
    push  ebx
    lea    edi, [edi + eax * 4]
    shr    ecx, 1
COPY_L:
    mov    eax, [edx]
    mov    ebx, [edx + 4]
    mov    [edi], eax
    mov    [edi + 4], ebx
    add    edx, 8
    add    edi, 8
    loop   COPY_L

    pop    ebx
F_RET:
    pop    edi
    xor    eax, eax
    ret
  }
}

#endif // #if defined(_WIN32) && defined(_X86)

GXLONG* Line_Subtract(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  size_t s1 = 0;
  size_t s2 = 0;

  GXLONG l = aSrc1[s1];
  GXLONG r = aSrc1[s1 + 1];

  while(1)
  {
    if(r <= aSrc2[s2])
    {
      aDest.push_back(l);
      aDest.push_back(r);
      goto INC_S1;
    }
    else if(aSrc2[s2 + 1] <= l)
      goto INC_S2;

    if(l < aSrc2[s2])
    {
      aDest.push_back(l);
      aDest.push_back(aSrc2[s2]);
    }
    if(aSrc2[s2 + 1] < r)
    {
      l = aSrc2[s2 + 1];
      goto INC_S2;
    }
INC_S1:
    s1 += 2;
    if(s1 >= aSrc1_size)
      return NULL;
    l = aSrc1[s1];
    r = aSrc1[s1 + 1];
    continue;
INC_S2:
    s2 += 2;
    if(s2 >= aSrc2_size)
      break;
  }

  aDest.push_back(l);
  aDest.push_back(r);
  while(1)
  {
    s1 += 2;
    if(s1 >= aSrc1_size)
      return NULL;
    aDest.push_back(aSrc1[s1]);
    aDest.push_back(aSrc1[s1 + 1]);
  }
  return NULL;
}

GXLONG* Line_And(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  size_t s1 = 0;
  size_t s2 = 0;

  while(1)
  {
    if(aSrc1[s1 + 1] <= aSrc2[s2])
      goto INC_S1;
    else if(aSrc2[s2 + 1] <= aSrc1[s1])
      goto INC_S2;

    if(aSrc1[s1] < aSrc2[s2])
      aDest.push_back(aSrc2[s2]);
    else
      aDest.push_back(aSrc1[s1]);

    if(aSrc2[s2 + 1] < aSrc1[s1 + 1])
    {
      aDest.push_back(aSrc2[s2 + 1]);
      goto INC_S2;
    }
    else
      aDest.push_back(aSrc1[s1 + 1]);

INC_S1:
    s1 += 2;
    if(s1 >= aSrc1_size)
      return 0;
    continue;
INC_S2:
    s2 += 2;
    if(s2 >= aSrc2_size)
      return 0;
  }
}

size_t __cpp_RegionLine_Or(LongFixedArray& aDest, REGIONLINE* aSrc1, REGIONLINE* aSrc2, REGIONLINE* aSrc1End, REGIONLINE* aSrc2End)
{
  REGIONLINE* pDest = (REGIONLINE*)&aDest.end();
  const REGIONLINE* pBegin = pDest;
  REGIONLINE* pEnd = pDest;

  if(aSrc1->left < aSrc2->left)
  {
    *pEnd = *aSrc1;
    pEnd++;
  }
  else
  {
    *pEnd = *aSrc2;
    pEnd++;
  }

  while(aSrc1 != aSrc1End && aSrc2 != aSrc2End)
  {
    if(aSrc1->left < aSrc2->left)
    {
      if(pDest->right >= aSrc1->left && pDest->right < aSrc1->right)
        pDest->right = aSrc1->right;
      else if(pDest->right < aSrc1->left)
      {
        *pEnd++ = *aSrc1;
        pDest++;
      }
      aSrc1++;
    }
    else
    {
      if(pDest->right >= aSrc2->left && pDest->right < aSrc2->right)
        pDest->right = aSrc2->right;
      else if(pDest->right < aSrc2->left)
      {
        *pEnd++ = *aSrc2;
        pDest++;
      }
      aSrc2++;
    }
  }
  if(aSrc1 != aSrc1End)
  {
    while(pDest->right >= aSrc1->left && aSrc1 != aSrc1End)
    {
      if(pDest->right < aSrc1->right)
        pDest->right = aSrc1->right;
      aSrc1++;
    }
    while(aSrc1 != aSrc1End)
    {
      *pEnd++ = *aSrc1++;
    }
  }
  else  // (aSrc2 != aSrc2End)
  {
    while(pDest->right >= aSrc2->left && aSrc2 != aSrc2End)
    {
      if(pDest->right < aSrc2->right)
        pDest->right = aSrc2->right;
      aSrc2++;
    }
    pDest++;
    while(aSrc2 != aSrc2End)
    {
      *pEnd++ = *aSrc2++;
    }
  }
  const size_t sizeCombine = (size_t)(pEnd - pBegin) << 1;
  aDest.resize(aDest.size() + sizeCombine);
  return sizeCombine;
}



GXLONG* Line_Xor(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size)
{
  size_t s1 = 0;
  size_t s2 = 0;

  while(1)
  {
    if(s1 >= aSrc1_size)
      goto FINI_S2;
    if(s2 >= aSrc2_size)
      goto FINI_S1;

    if(aSrc1[s1] < aSrc2[s2])
    {
      aDest.push_back(aSrc1[s1]);
      s1++;
      continue;
    }
    else if(aSrc1[s1] > aSrc2[s2])
    {
      aDest.push_back(aSrc2[s2]);
      s2++;
      continue;
    }
    s1++;
    s2++;
  }


FINI_S1:
    while(s1 < aSrc1_size)
    {
      aDest.push_back(aSrc1[s1]);
      s1++;
    }
    return 0;
FINI_S2:
    while(s2 < aSrc2_size)
    {
      aDest.push_back(aSrc2[s2]);
      s2++;
    }
    return 0;
}