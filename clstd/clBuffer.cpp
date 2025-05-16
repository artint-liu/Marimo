#include "clstd.h"
//#include "clBuffer.h"
//#include "clmemory.h"
#include "clString.h"
#include "clUtility.h"
#include "clStringCommon.hxx"
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
  CLBYTE* BufferBase::Set(int val)
  {
    return (CLBYTE*)memset(m_lpBuffer, val, m_uSize);
  }

  BufferBase& BufferBase::operator=(const BufferBase&)
  {
    CLBREAK; // 不能使用基类赋值
    return *this;
  }

  //////////////////////////////////////////////////////////////////////////

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

  void FixedBuffer::Set(CLLPCVOID lpData, clsize cbSize)
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

  size_t MemBuffer::Reserve(clsize dwSize)
  {
    if(dwSize > m_uSize)
    {
      auto nOriginSize = m_uSize;
      size_t delta = Resize(dwSize, FALSE);
      m_uSize = nOriginSize;
      return delta;
    }
    return 0;
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

  clsize MemBuffer::GetCapacity() const
  {
    return m_nCapacity;
  }

  CLLPVOID MemBuffer::GetPtr() const
  {
    return m_lpBuffer;  
  }

  CLLPVOID MemBuffer::GetEnd() const
  {
    return reinterpret_cast<CLLPVOID>(reinterpret_cast<clsize>(m_lpBuffer) + m_uSize);
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

  MemBuffer& MemBuffer::Append(const BufferBase& buf)
  {
    Append(buf.GetPtr(), buf.GetSize());
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

  MemBuffer& MemBuffer::operator=(const MemBuffer& buf)
  {
    Replace(0, m_uSize, buf.GetPtr(), buf.GetSize());
    return *this;
  }


  namespace StringUtility
  {
    template<typename _TChar>
    class _DelegateBuffer : public MemBuffer
    {
    public:
      typedef _TChar TChar;
      void Append(TChar c) {
        MemBuffer::Append(&c, sizeof(TChar));
      }
    };

    MemBuffer& ConvertToUtf8(MemBuffer& strUtf8, const wch* szUnicode, size_t nUnicode)
    {
      _DelegateBuffer<ch>* local_buffer = (_DelegateBuffer<ch>*)&strUtf8;
      return StringCommon::ConvertToUtf8T(*local_buffer, szUnicode, nUnicode);
    }
    
    MemBuffer& ConvertFromUtf8(MemBuffer& strUnicode, const ch* szUtf8, size_t nUtf8)
    {
      _DelegateBuffer<wch>* local_buffer = (_DelegateBuffer<wch>*)&strUnicode;
      return StringCommon::ConvertFromUtf8(*local_buffer, szUtf8, nUtf8);
    }
  } // namespace StringUtility

#ifdef _CL_SYSTEM_WINDOWS
  SharedBuffer::SharedBuffer(CLLPCSTR szName, u32 flags, clsize cbBuffer)
  {
    DWORD dwFlags = PAGE_READONLY;
    if (TEST_FLAG(flags, Flags_Write))
    {
      dwFlags = PAGE_READWRITE;
    }
    m_hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, dwFlags, 0, cbBuffer, szName);
    m_size = cbBuffer;
    if (m_hMapFile != NULL)
    {
      m_pSharedMemory = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, cbBuffer);
    }
  }

  SharedBuffer::~SharedBuffer()
  {
    if (m_pSharedMemory)
    {
      UnmapViewOfFile(m_pSharedMemory);
      m_pSharedMemory = NULL;
    }

    if (m_hMapFile)
    {
      CloseHandle(m_hMapFile);
    }
  }

  clsize SharedBuffer::Write(const u8* ptr, clsize count)
  {
    if (m_pSharedMemory)
    {
      clsize cbCopy = min(count, m_size);
      memcpy(m_pSharedMemory, ptr, cbCopy);
      return cbCopy;
    }
    return 0;
  }

  clsize SharedBuffer::Read(u8* ptr, clsize count)
  {
    if (m_pSharedMemory)
    {
      clsize cbCopy = min(count, m_size);
      memcpy(ptr, m_pSharedMemory, cbCopy);
      return cbCopy;
    }
    return 0;
  }
#endif


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
