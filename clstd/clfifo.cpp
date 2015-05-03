#include "clstd.h"
#include "clString.H"
#include "clUtility.H"
#include "clfifo.h"

#ifdef _WINDOWS
#ifdef _X86
static inline void barrier(void)
{
  __asm nop
  __asm nop
  __asm nop
  __asm nop
}

#define smp_mb()  barrier()
#define smp_wmb() barrier()
#elif defined(_X64)
extern "C" void barrier(void);
#define smp_mb()  barrier()
#define smp_wmb() barrier()
#endif // #ifdef _X86
#elif defined(_IOS)
static inline void barrier(void)
{
  asm volatile("" : : : "memory");
}
#define smp_mb()  barrier()
#define smp_wmb() barrier()
#endif // #if defined(_WIN32) || defined(_WINDOWS)

namespace clstd
{

  fifo::fifo()
    : m_buffer  (NULL)
    , m_size    (NULL)
    , m_in      (NULL)
    , m_out     (NULL)
    , m_pLocker (NULL)
  {
  }

  fifo::~fifo()
  {
    SAFE_DELETE_ARRAY(m_buffer);
    SAFE_DELETE(m_pLocker);
  }


  b32 fifo::Initialize(u32 size, b32 bLock)
  {
    if (size & (size - 1)) {   
      if(size > 0x80000000) {
        return FALSE;
      }
      size = clstd::RoundupPowOfTwo(size);   
    }
    m_size = size;

    m_buffer = new u8[m_size];
    if(m_buffer == NULL) {
      return FALSE;
    }

    if(bLock) {
      m_pLocker = new clstd::Locker;
    }
    return TRUE;
  }

  u32 fifo::put(const u8* data, u32 len)
  {
    unsigned int l;   

    len = clMin(len, m_size - m_in + m_out);

    smp_mb();
    if(m_pLocker) {
      m_pLocker->Lock();
    }

    // first put the data starting from fifo->in to buffer end
    l = clMin(len, m_size - (m_in & (m_size - 1)));   
    memcpy(m_buffer + (m_in & (m_size - 1)), data, l);   

    // then put the rest (if any) at the beginning of the buffer
    memcpy(m_buffer, data + l, len - l);

    smp_wmb();
    if(m_pLocker) {
      m_pLocker->Unlock();
    }

    m_in += len;  
    return len;   
  }

  u32 fifo::get(u8* data, u32 len)
  {
    unsigned int l;   
    len = clMin(len, m_in - m_out);

    smp_mb();
    if(m_pLocker) {
      m_pLocker->Lock();
    }

    // first get the data from fifo->out until the end of the buffer
    l = clMin(len, m_size - (m_out & (m_size - 1)));   
    memcpy(data, m_buffer + (m_out & (m_size - 1)), l);   

    // then get the rest (if any) from the beginning of the buffer
    memcpy(data + l, m_buffer, len - l);   

    smp_wmb();
    if(m_pLocker) {
      m_pLocker->Unlock();
    }

    m_out += len;
    return len;   
  }

  u32 fifo::size()
  {
    return m_in - m_out;
  }
} // namespace clstd
//
//struct kfifo *kfifo_alloc(unsigned int size, gfp_t gfp_mask, spinlock_t *lock)   
//{   
//    unsigned char *buffer;   
//    struct kfifo *ret;   
//  
//    /*  
//     * round up to the next power of 2, since our 'let the indices  
//     * wrap' tachnique works only in this case.  
//     */   
//  
//    buffer = kmalloc(size, gfp_mask);   
//    if (!buffer)   
//        return ERR_PTR(-ENOMEM);   
//  
//    ret = kfifo_init(buffer, size, gfp_mask, lock);   
//  
//    if (IS_ERR(ret))   
//        kfree(buffer);   
//  
//    return ret;   
//}   
//
//unsigned int __kfifo_put(struct kfifo *fifo,   
//             unsigned char *buffer, unsigned int len)   
//{   
//    unsigned int l;   
//  
//    len = min(len, fifo->size - fifo->in + fifo->out);   
//  
//    /*  
//     * Ensure that we sample the fifo->out index -before- we  
//     * start putting bytes into the kfifo.  
//     */   
//  
//    smp_mb();   
//  
//    /* first put the data starting from fifo->in to buffer end */   
//    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));   
//    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);   
//  
//    /* then put the rest (if any) at the beginning of the buffer */   
//    memcpy(fifo->buffer, buffer + l, len - l);   
//  
//    /*  
//     * Ensure that we add the bytes to the kfifo -before-  
//     * we update the fifo->in index.  
//     */   
//  
//    smp_wmb();   
//  
//    fifo->in += len;   
//  
//    return len;   
//}  
//  
//unsigned int __kfifo_get(struct kfifo *fifo,   
//             unsigned char *buffer, unsigned int len)   
//{   
//    unsigned int l;   
//  
//    len = min(len, fifo->in - fifo->out);   
//  
//    /*  
//     * Ensure that we sample the fifo->in index -before- we  
//     * start removing bytes from the kfifo.  
//     */   
//  
//    smp_rmb();   
//  
//    /* first get the data from fifo->out until the end of the buffer */   
//    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));   
//    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);   
//  
//    /* then get the rest (if any) from the beginning of the buffer */   
//    memcpy(buffer + l, fifo->buffer, len - l);   
//  
//    /*  
//     * Ensure that we remove the bytes from the kfifo -before-  
//     * we update the fifo->out index.  
//     */   
//  
//    smp_mb();   
//  
//    fifo->out += len;   
//  
//    return len;   
//}   