#ifndef _CL_BUFFER_H_
#define _CL_BUFFER_H_

namespace clstd
{
  //////////////////////////////////////////////////////////////////////////
  //
  // Buffer 基类
  //
  class BufferBase
  {
  protected:
    CLBYTE*  m_lpBuffer;  // 数据指针地址
    clsize   m_uSize;     // 已经使用的大小
  protected:
    BufferBase(const CLLPVOID pData, clsize uLength)
      : m_lpBuffer((CLBYTE*)pData), m_uSize(uLength){}

    virtual ~BufferBase(){} // 析构是私有的, 防止用户 delete clBufferBase
  public:
    inline CLLPVOID   GetPtr    () const;
    inline clsize     GetSize   () const;
    inline CLBYTE*    Set       (int val);
  };

  CLLPVOID BufferBase::GetPtr() const
  {
    return m_lpBuffer;
  }

  clsize BufferBase::GetSize() const
  {
    return m_uSize;
  }

  CLBYTE* BufferBase::Set(int val)
  {
    return (CLBYTE*)memset(m_lpBuffer, val, m_uSize);
  }

  //////////////////////////////////////////////////////////////////////////
  //
  // 引用 Buffer , 在析构时不会释放指针.
  // 用于把数据封装为Buffer传递
  // clBufferBase 与此类似, 但是它的析构函数不是共有的, 所以不能直接使用
  //
  class RefBuffer : public BufferBase
  {
  public:
    RefBuffer() : BufferBase(NULL, 0){}
    RefBuffer(const CLLPVOID pData, clsize uLength)
      : BufferBase(pData, uLength){}
  };

  //////////////////////////////////////////////////////////////////////////
  //
  // 尺寸固定的 Buffer
  //
  class FixedBuffer : public BufferBase
  {
  public:
    FixedBuffer();
    FixedBuffer(clsize nSize);
    virtual ~FixedBuffer();

    // 如果新旧Size不相等, 就会分配一个新的缓冲区
    // 新的缓冲区大小与输入参数严格相等, 不会预留多余追加数据的空间
    // Resize 尺寸比原来数据区小, 会截断数据, 比原来数据区大, 不会破坏数据, 根据参数决定是否填0
    b32 Resize(clsize dwSize, b32 bZeroInit);
    void Set(CLLPVOID lpData, clsize cbSize);
    void Set(const BufferBase* pBuffer);
  };

  //////////////////////////////////////////////////////////////////////////
  //
  // 局部 Buffer
  // 这个类在定义时会带有附带一段缓冲区，在定义局部实体对象时如果使用的缓冲不大就
  // 可以直接使用这段堆栈上的缓冲区，避免了不必要的内存分配
  // 如果定义为指针，在使用缓冲尺寸不大的情况下也会减少一次内存分配
  //
  template<size_t _count>
  class LocalBuffer : public BufferBase
  {
    typedef CLBYTE _Ty;
    clsize m_nCapacity;  // 容量大小
    _Ty m_data[_count];

  public:
    LocalBuffer()
      : BufferBase(m_data, 0)
      , m_nCapacity(sizeof(_Ty) * _count)
    {}

    virtual ~LocalBuffer()
    {
      if( ! IsLocalPtr() && m_lpBuffer) {
        delete[] m_lpBuffer;
      }
      m_uSize = 0;
    }

    b32 IsLocalPtr() const
    {
      return (size_t)m_data == (size_t)m_lpBuffer;
    }

    b32 Reserve(clsize uNewCapacity) // 扩充容量（而不是尺寸）
    {
      ASSERT(m_uSize <= m_nCapacity); // 数据尺寸一定小于缓冲区容量
      if(uNewCapacity <= m_nCapacity) {
        return FALSE;
      }

      if(IsLocalPtr()) {
        ASSERT((sizeof(_Ty) * _count) == m_nCapacity); // 局部指针容量一定与局部缓冲的容量相等

        m_lpBuffer = new CLBYTE[uNewCapacity];
        memcpy(m_lpBuffer, m_data, m_uSize);
      }
      else {
        CLBYTE* pNewBuffer = new CLBYTE[uNewCapacity];
        memcpy(pNewBuffer, m_lpBuffer, m_uSize);
        delete[] m_lpBuffer;
        m_lpBuffer = pNewBuffer;
      }
      m_nCapacity = uNewCapacity;
      return TRUE;
    }

    b32 Resize(clsize dwSize, b32 bZeroInit)
    {
      ASSERT(m_uSize <= m_nCapacity); // 数据尺寸一定小于缓冲区容量

      if(dwSize > m_uSize)
      {
        if(dwSize > m_nCapacity) {
          // 对齐到n倍局部缓冲尺寸
          Reserve((dwSize / (sizeof(_Ty) * _count) + 1) * (sizeof(_Ty) * _count));
        }

        ASSERT(m_uSize <= dwSize && dwSize <= m_nCapacity);

        if(bZeroInit) {
          memset((CLBYTE*)m_lpBuffer + m_uSize, 0, dwSize - m_uSize);
        }
      }

      m_uSize = dwSize;
      return TRUE;
    }

    LocalBuffer& Append(CLLPCVOID lpData, clsize dwSize)
    {
      const clsize dwTail = m_uSize;
      Resize(m_uSize + dwSize, FALSE);
      memcpy(m_lpBuffer + dwTail, lpData, dwSize);
      return *this;
    }


  };

  //////////////////////////////////////////////////////////////////////////
  //
  // 通用 Buffer
  //
  class MemBuffer : public BufferBase
  {
  protected:
    clsize m_nCapacity;  // 容量大小
    clsize m_nPageSize;  // 页大小, 大小不够将按照这个长度增加
  public:
    MemBuffer(u32 nPageSize = 512);
    virtual ~MemBuffer();

    size_t    Reserve   (clsize dwSize); // 地址没变化返回0，否则返回一个补码的地址差
    size_t    Resize    (clsize dwSize, b32 bZeroInit); // 地址没变化返回0，否则返回一个补码的地址差
    CLLPVOID  GetPtr    () const;
    clsize    GetSize   () const;
    //b32       Add       (u32 nPos, CLLPCVOID lpData, clsize dwSize); // 这是什么鬼啊！！！
    MemBuffer&Append    (CLLPCVOID lpData, clsize dwSize);
    b32       Replace   (clsize nPos, clsize nLen, CLLPCVOID lpData, clsize cbSize);
    b32       Insert    (clsize nPos, CLLPCVOID lpData, clsize cbSize);
  };

  typedef MemBuffer Buffer;

  // 宽字符buffer
  // TODO: 这个从clStringW继承吧，重新实现Allocator
  //class WideTextBuffer : public clBuffer
  //{
  //public:
  //  WideTextBuffer(clBufferBase* pRawBuffer);
  //  WideTextBuffer(clBufferBase* pRawBuffer, CLDWORD dwBOM);
  //  clsize GetLength(); // 返回字符数，GetSize()返回的还是字节数
  //};

} // namespace clstd
//////////////////////////////////////////////////////////////////////////
//
// 写完之后没用过忘了不知道干啥的了
//
class clQueueBuffer
{ // 没测试过
  struct HEADER
  {
    HEADER* pNext;
  };
private:
  HEADER*   m_pHead;
  HEADER*   m_pTail;
  u32       m_uSize;
  u32       m_uElementSize;
public:
  clQueueBuffer(u32 uElementSize);
  void* front       ();
  void* back        ();
  void  pop         ();
  void  push        (void* pFixedData);
  void  push_front  (void* pFixedData);
  void  clear       ();
  u32   size        ();
};

//////////////////////////////////////////////////////////////////////////
//
// Buffer 处理结构
//
namespace clstd
{
  struct PROCESSBUFFER
  {
    Buffer*     pDestBuf; // 如果为 NULL, 处理函数会自己创建. 不再使用时由用户释放
    BufferBase* pSrcBuf;  // 输入的 Buffer
  };
} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CL_BUFFER_H_