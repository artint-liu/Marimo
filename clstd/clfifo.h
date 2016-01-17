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

} // namespace clstd
#endif // _CLSTD_FIFO_H_