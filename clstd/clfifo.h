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
    u32       m_size;     // the size of the allocated buffer
    u32       m_in;       // data is added at offset (in % size)
    u32       m_out;      // data is extracted from off. (out % size)
    Locker*   m_pLocker;
  public:
    fifo();
    ~fifo();
  public:
    b32   Initialize  (u32 size, b32 bLock);
    u32   put         (const u8* data, u32 len);
    u32   get         (u8* data, u32 len);
    u32   size        ();
  };

} // namespace clstd
#endif // _CLSTD_FIFO_H_