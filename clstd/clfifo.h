#ifndef _CLSTD_FIFO_H_
#define _CLSTD_FIFO_H_

// First input first out buffer
namespace clstd
{
  class Locker;

  class fifo
  {
  private:
    u8*       m_buffer;   // the buffer holding the data
    size_t    m_size;     // the size of the allocated buffer
    size_t    m_in;       // data is added at offset (in % size)
    size_t    m_out;      // data is extracted from off. (out % size)
    Locker*   m_pLocker;
  public:
    fifo();
    ~fifo();
  public:
    b32     Initialize  (size_t size, b32 bLock);
    size_t  put         (const u8* data, size_t len);
    size_t  get         (u8* data, size_t len);
    size_t  size        ();
  };

  template<class _Ty>
  class WIRO // Writer Input & Reader Output
  {
  protected:
    _Ty*    m_pElementArray;
    //size_t  m_size;
    size_t  m_mask;
    size_t  m_capacity;
    volatile size_t  m_in;
    volatile size_t  m_out;
#ifdef _DEBUG
    this_thread::id m_idReader;
    this_thread::id m_idWriter;
#endif // _DEBUG
  public:
    WIRO(size_t size) // 容量数，必须是 2^n - 1
      : m_pElementArray(NULL)
      , m_mask(0)
      , m_capacity(0)
      , m_in(0)
      , m_out(0)
#ifdef _DEBUG
      , m_idReader(0)
      , m_idWriter(0)
#endif // _DEBUG
    {
      SetSize(size);
    }

    WIRO()
      : m_pElementArray(NULL)
      , m_mask(0)
      , m_capacity(0)
      , m_in(0)
      , m_out(0)
#ifdef _DEBUG
      , m_idReader(0)
      , m_idWriter(0)
#endif // _DEBUG
    {
    }

    ~WIRO()
    {
      SAFE_DELETE_ARRAY(m_pElementArray);
      m_capacity = m_mask = m_in = m_out = 0;
    }

    b32 SetSize(size_t size)
    {
      // 写入线程一旦开始工作，SetSize将不再安全
      ASSERT( ! m_idWriter &&
        ( ! m_idReader || m_idReader == this_thread::GetId()));

      // is not pow of 2
      if(size & (size - 1)) {
        return FALSE;
      }
      //m_size = size;
      m_capacity = size;
      m_mask = size - 1;
      SAFE_DELETE_ARRAY(m_pElementArray);
      m_pElementArray = new _Ty[m_capacity];
      m_in = 0;
      m_out = 0;
#ifdef _DEBUG
      m_idReader = 0;
      m_idWriter = 0;
#endif // _DEBUG
      return TRUE;
    }
    
    b32 Put(const _Ty& element)
    {
#ifdef _DEBUG
      if(m_idWriter == 0) {
        m_idWriter = this_thread::GetId();
      }
      ASSERT(m_idWriter == this_thread::GetId()); // 有且只有一个写入线程
#endif // _DEBUG
      
      if(m_pElementArray) {
        const size_t next_in = (m_in + 1) & m_mask;
        if(next_in == m_out) {
          return FALSE;
        }

        m_pElementArray[m_in] = element;
        m_in = next_in;
      }
      return TRUE;
    }
    
    b32 Get(_Ty& element)
    {
#ifdef _DEBUG
      if(m_idReader == 0) {
        m_idReader = this_thread::GetId();
      }
      ASSERT(m_idReader == this_thread::GetId()); // 有且只有一个读取线程
#endif // _DEBUG

      if(m_pElementArray && m_out != m_in) {
        element = m_pElementArray[m_out];
        m_out = (m_out + 1) & m_mask;
        return TRUE;
      }
      return FALSE;
    }

    size_t GetSize() const
    {
      // 只有读取或者写入线程才能调用这个
#ifdef _DEBUG
      const this_thread::id _id = this_thread::GetId();
      if(m_idReader == _id || m_idWriter == _id); {
        return m_in - m_out;
      }
#else
      return m_in - m_out;
#endif // #ifdef _DEBUG
      return 0;
    }

    b32 IsEmpty() const
    {
      // 只有读取或者写入线程才能调用这个
#ifdef _DEBUG
      const this_thread::id _id = this_thread::GetId();
      if(m_idReader == _id || m_idWriter == _id) {
        return m_in == m_out;
      }
      return 0;
#else
      return m_in == m_out;
#endif
    }
  };

} // namespace clstd
#endif // _CLSTD_FIFO_H_