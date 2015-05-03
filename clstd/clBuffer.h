#ifndef _CL_BUFFER_H_
#define _CL_BUFFER_H_

//////////////////////////////////////////////////////////////////////////
//
// Buffer ����
//
class clBufferBase
{
protected:
  CLBYTE*  m_lpBuffer;  // ����ָ���ַ
  clsize   m_uSize;     // �Ѿ�ʹ�õĴ�С
protected:
  clBufferBase(const CLLPVOID pData, clsize uLength)
    : m_lpBuffer((CLBYTE*)pData), m_uSize(uLength){}

  virtual ~clBufferBase(){} // ������˽�е�, ��ֹ�û� delete clBufferBase
public:
  inline CLLPVOID   GetPtr    () const;
  inline clsize     GetSize   () const;
  inline CLBYTE*    Set       (int val);
};

CLLPVOID clBufferBase::GetPtr() const
{
  return m_lpBuffer;
}

clsize clBufferBase::GetSize() const
{
  return m_uSize;
}

CLBYTE* clBufferBase::Set(int val)
{
  return (CLBYTE*)memset(m_lpBuffer, val, m_uSize);
}

//////////////////////////////////////////////////////////////////////////
//
// ���� Buffer , ������ʱ�����ͷ�ָ��.
// ���ڰ����ݷ�װΪBuffer����
// clBufferBase �������, �������������������ǹ��е�, ���Բ���ֱ��ʹ��
//
namespace clstd
{
  class RefBuffer : public clBufferBase
  {
  public:
    RefBuffer() : clBufferBase(NULL, 0){}
    RefBuffer(const CLLPVOID pData, clsize uLength)
      : clBufferBase(pData, uLength){}
  };

  //////////////////////////////////////////////////////////////////////////
  //
  // �ߴ�̶��� Buffer
  //
  class FixedBuffer : public clBufferBase
  {
  public:
    FixedBuffer();
    FixedBuffer(clsize nSize);
    virtual ~FixedBuffer();

    // ����¾�Size�����, �ͻ����һ���µĻ�����
    // �µĻ�������С����������ϸ����, ����Ԥ������׷�����ݵĿռ�
    // Resize �ߴ��ԭ��������С, ��ض�����, ��ԭ����������, �����ƻ�����, ���ݲ��������Ƿ���0
    b32 Resize(clsize dwSize, b32 bZeroInit);
    void Set(CLLPVOID lpData, clsize cbSize);
    void Set(const clBufferBase* pBuffer);
  };
}


//////////////////////////////////////////////////////////////////////////
//
// ͨ�� Buffer
//
class clBuffer : public clBufferBase
{
protected:
  clsize m_nCapacity;  // ������С
  clsize m_nPageSize;  // ҳ��С, ��С���������������������
public:
  clBuffer(u32 nPageSize = 512);
  virtual ~clBuffer();

  b32       Reserve   (clsize dwSize);
  b32       Resize    (clsize dwSize, b32 bZeroInit);
  CLLPVOID  GetPtr    () const;
  clsize    GetSize   () const;
  //b32       Add       (u32 nPos, CLLPCVOID lpData, clsize dwSize); // ����ʲô��������
  b32       Append    (CLLPCVOID lpData, clsize dwSize);
  b32       Replace   (clsize nPos, clsize nLen, CLLPCVOID lpData, clsize cbSize);
  b32       Insert    (clsize nPos, CLLPCVOID lpData, clsize cbSize);
};

namespace clstd
{
  // ���ַ�buffer
  // TODO: �����clStringW�̳аɣ�����ʵ��Allocator
  //class WideTextBuffer : public clBuffer
  //{
  //public:
  //  WideTextBuffer(clBufferBase* pRawBuffer);
  //  WideTextBuffer(clBufferBase* pRawBuffer, CLDWORD dwBOM);
  //  clsize GetLength(); // �����ַ�����GetSize()���صĻ����ֽ���
  //};
} // namespace clstd
//////////////////////////////////////////////////////////////////////////
//
// д��֮��û�ù����˲�֪����ɶ����
//
class clQueueBuffer
{ // û���Թ�
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
// Buffer ����ṹ
//
namespace clstd
{
  struct PROCESSBUFFER
  {
    clBuffer*     pDestBuf; // ���Ϊ NULL, ���������Լ�����. ����ʹ��ʱ���û��ͷ�
    clBufferBase* pSrcBuf;  // ����� Buffer
  };
} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CL_BUFFER_H_