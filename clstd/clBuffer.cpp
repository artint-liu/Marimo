#include "clstd.h"
//#include "clBuffer.H"
#include "clmemory.h"
#include "clString.H"
#include "clUtility.H"
//#include "third_party/zlib/zlib.h"

//int zlib_compress (
//  Bytef *dest,
//  uLongf *destLen,
//  const Bytef *source,
//  uLong sourceLen);
//uLong zlib_compressBound (
//  uLong sourceLen);

//clBufferBase::clBufferBase(const CLLPVOID pData, u32 uLength)
//  : m_lpBuffer  ((CLBYTE*)pData)
//  , m_uSize     (uLength)
//{
//  if(m_uSize == LEN_WIDESTRING)
//  {
//    m_uSize = wcslen((const wchar_t*)pData);
//  }
//  else if(m_uSize == LEN_ANSISTRING)
//  {
//    m_uSize = strlen((const char*)pData);
//  }
//}
//////////////////////////////////////////////////////////////////////////
namespace clstd
{
  FixedBuffer::FixedBuffer()
    : BufferBase(NULL, 0)
  {
  }

  FixedBuffer::FixedBuffer(clsize nSize)
    : BufferBase(nSize != 0 ? new CLBYTE[nSize] : NULL, nSize)
  {
  }

  FixedBuffer::~FixedBuffer()
  {
    SAFE_DELETE(m_lpBuffer);
  }

  b32 FixedBuffer::Resize(clsize dwSize, b32 bZeroInit)
  {
    if(dwSize == m_uSize) {
      return FALSE;
    }

    if(dwSize == 0) {
      SAFE_DELETE(m_lpBuffer);
    }
    else
    {
      CLBYTE* pNewBuffer = new CLBYTE[dwSize];
      if(m_lpBuffer != NULL) {
        memcpy(pNewBuffer, m_lpBuffer, clMin(m_uSize, dwSize));
        delete m_lpBuffer;
      }
      m_lpBuffer = pNewBuffer;

      if(bZeroInit && dwSize > m_uSize) {
        memset(m_lpBuffer + m_uSize, 0, dwSize - m_uSize);
      }
    }

    m_uSize = dwSize;
    return TRUE;
  }

  void FixedBuffer::Set(CLLPVOID lpData, clsize cbSize)
  {
    Resize(cbSize, FALSE);
    ASSERT(cbSize == m_uSize);
    memcpy(m_lpBuffer, lpData, cbSize);
  }

  void FixedBuffer::Set(const BufferBase* pBuffer)
  {
    Set(pBuffer->GetPtr(), pBuffer->GetSize());
  }

  //////////////////////////////////////////////////////////////////////////  
  MemBuffer::MemBuffer(u32 nPageSize)
    : BufferBase(NULL, 0)
    , m_nCapacity(0)
    , m_nPageSize(nPageSize)
  {
  }

  MemBuffer::~MemBuffer()
  {
    SAFE_DELETE(m_lpBuffer);
    m_uSize = 0;
    m_nCapacity = 0;
  }

  b32 MemBuffer::Reserve(clsize dwSize)
  {
    if(dwSize > m_uSize)
    {
      auto nOriginSize = m_uSize;
      b32 r = Resize(dwSize, FALSE);
      m_uSize = nOriginSize;
      return r;
    }
    return FALSE;
  }

  size_t MemBuffer::Resize(clsize dwSize, b32 bZeroInit)
  {
    CLBYTE*const pPrevBuffer = m_lpBuffer;
    if(dwSize >= m_nCapacity)
    {
      m_nCapacity = ((dwSize / m_nPageSize) + 1) * m_nPageSize;
      CLBYTE* pNewBuffer = new CLBYTE[m_nCapacity];

      memcpy(pNewBuffer, m_lpBuffer, m_uSize);
      delete m_lpBuffer;
      m_lpBuffer = pNewBuffer;
    }

    if(bZeroInit && dwSize > m_uSize) {
      memset(m_lpBuffer + m_uSize, 0, dwSize - m_uSize);
    }

    m_uSize = dwSize;
    return ((size_t)m_lpBuffer - (size_t)pPrevBuffer);
  }

  clsize MemBuffer::GetSize() const
  {
    return m_uSize;
  }

  CLLPVOID MemBuffer::GetPtr() const
  {
    return m_lpBuffer;  
  }

  //b32 clBuffer::Add(u32 nPos, CLLPCVOID lpData, clsize dwSize)
  //{
  //  if(nPos > m_uSize)
  //    return FALSE;
  //  clsize dwTail = m_uSize - nPos;
  //
  //  Resize(m_uSize + dwSize, FALSE);
  //
  //  memcpy(m_lpBuffer + nPos + dwSize, m_lpBuffer + nPos, dwTail);
  //  memcpy(m_lpBuffer + nPos, lpData, dwSize);
  //  return TRUE;
  //}

  MemBuffer& MemBuffer::Append(CLLPCVOID lpData, clsize dwSize)
  {
    const clsize dwTail = m_uSize;
    Resize(m_uSize + dwSize, FALSE);
    memcpy(m_lpBuffer + dwTail, lpData, dwSize);
    return *this;
  }

  b32 MemBuffer::Replace(clsize nPos, clsize nLen, CLLPCVOID lpData, clsize cbSize)
  {
    if(nPos + nLen >= m_uSize)
    {
      m_uSize = nPos;
      Append(lpData, cbSize);
      return TRUE;
    }

    if(nLen != cbSize)
    {
      clsize dwTail = m_uSize - (nPos + nLen);
      Resize(m_uSize - nLen + cbSize, FALSE);
      memcpy(m_lpBuffer + nPos + cbSize, m_lpBuffer + nPos + nLen, dwTail);
    }

    if(lpData != NULL && cbSize != 0) {
      memcpy(m_lpBuffer + nPos, lpData, cbSize);
    }
    return TRUE;
  }

  b32 MemBuffer::Insert(clsize nPos, CLLPCVOID lpData, clsize cbSize)
  {
    return Replace(nPos, 0, lpData, cbSize);
  }
} // namespace clstd
//////////////////////////////////////////////////////////////////////////
clQueueBuffer::clQueueBuffer(u32 uElementSize)
  : m_pHead(NULL)
  , m_pTail(NULL)
  , m_uSize(0)
  , m_uElementSize(uElementSize)
{
}

void* clQueueBuffer::front()
{
  return ((u8*)m_pHead) + sizeof(HEADER);
}

void* clQueueBuffer::back()
{
  return ((u8*)m_pTail) + sizeof(HEADER);
}

void clQueueBuffer::pop()
{
  if(m_pHead == NULL) {
    return;
  }

  HEADER* pNext = m_pHead->pNext;
  
  delete m_pHead;
  m_pHead = pNext;

  m_uSize--;
}

void clQueueBuffer::push(void* pFixedData)
{
   HEADER* pNew = (HEADER*)new u8[sizeof(HEADER) + m_uElementSize];
   
   pNew->pNext = NULL;
   memcpy(((u8*)pNew) + sizeof(HEADER), pFixedData, m_uElementSize);

   ASSERT(m_pTail->pNext == NULL);
   m_pTail->pNext = pNew;
   m_pTail = pNew;
   m_uSize++;
}

void clQueueBuffer::push_front(void* pFixedData)
{
  HEADER* pNew = (HEADER*)new u8[sizeof(HEADER) + m_uElementSize];

  pNew->pNext = m_pHead;
  memcpy(((u8*)pNew) + sizeof(HEADER), pFixedData, m_uElementSize);

  m_pHead = pNew;
  m_uSize++;
}

void clQueueBuffer::clear()
{
  HEADER* pHead = m_pHead;
  HEADER* pNext = NULL;
  while(pHead != NULL) {
    pNext = pHead->pNext;
    delete pHead;
    pHead = pNext;
  }
  m_pHead = NULL;
  m_pTail = NULL;
  m_uSize = 0;
}

u32 clQueueBuffer::size()
{
  return m_uSize;
}
