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
  // 通用 Buffer
  //
  class Buffer : public BufferBase
  {
  protected:
    clsize m_nCapacity;  // 容量大小
    clsize m_nPageSize;  // 页大小, 大小不够将按照这个长度增加
  public:
    Buffer(u32 nPageSize = 512);
    virtual ~Buffer();

    b32       Reserve   (clsize dwSize);
    b32       Resize    (clsize dwSize, b32 bZeroInit);
    CLLPVOID  GetPtr    () const;
    clsize    GetSize   () const;
    //b32       Add       (u32 nPos, CLLPCVOID lpData, clsize dwSize); // 这是什么鬼啊！！！
    b32       Append    (CLLPCVOID lpData, clsize dwSize);
    b32       Replace   (clsize nPos, clsize nLen, CLLPCVOID lpData, clsize cbSize);
    b32       Insert    (clsize nPos, CLLPCVOID lpData, clsize cbSize);
  };

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