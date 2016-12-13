#ifndef _CL_ALLOCATOR_H_
#define _CL_ALLOCATOR_H_

//#define TRACK_ALLOCATOR
namespace clstd
{
  struct ALLOCPLOY;
  class Allocator;

  namespace Internal
  {
    class MemUnitGroup
    {
    public: // debug public
      void*   m_pBegin;       // 内存池开始地址
      void*   m_pEnd;         // 内存池结束地址
    private:
      void**  m_paPtrTable;   // 内存池指针表, 所有可用指针都放这里
      u32*    m_pMaskTable;   // 内存池指针表掩码, 用来预先判断指针是否可用
      u32     m_uGrain;       // 当前内存池的粒度, 字节数
      u32     m_uCount;       // 可用的数量, 粒度数
      u32     m_uCapacity;    // 总容量, 粒度数
    public:
      static clsize GetPtrTableSize  (u32 uGrain, u32 uCapacity);
      static clsize GetMaskTableSize (u32 uGrain, u32 uCapacity);

      MemUnitGroup      ();
      void* Initialize  (void* pBegin, u32 uGrain, u32 uCapacity);
      void  Finalize    (const ch* szName);
      u32   GetGrain    ();
      bool  PtrInPool   (void* ptr);
      bool  MatchSize   (clsize uSize);
      void* Alloc       ();
      void  Free        (void* ptr);
    };

    class MemPool
    {
    private:
      Allocator*        m_pAllocator;
      u8*               m_pPool;
      clsize            m_uGroupCount;
      MemUnitGroup*     m_aGroup;
      clsize            m_uAllocCount;    // 只记录在池中分配的数量, 池满后转到下一个Pool中
      const int         m_nIndex;         // 在链中的索引，用于统计
      MemPool*          m_pNext;

    private:
      MemPool             (Allocator* pAllocator, int nIndex, clsize nGroupCount);
      void    SetPool     (MemUnitGroup* aGroup, u8* pPool);
      clsize  FreeInGroups(void* ptr);
    public:
      void    DeallocPool (b32 bDelNext);

    public:
      void*   Alloc       (clsize nBytes, clsize* pCapacity);
      clsize  Free        (void* ptr);
      void*   Realloc     (void* ptr, void* pNewPtr, clsize nBytes, clsize* pCapacity);

      static  MemPool* CreateMemPool(Allocator* pAllocator, int nIndex);
    };
  } // namespace Internal

  struct ALLOCPLOY
  {
    u32    uGrain;
    u32    uCapacity;
  };

  class Allocator
  {
    typedef Internal::MemUnitGroup MemUnitGroup;
    typedef Internal::MemPool      MemPool;
    friend class Internal::MemPool;
  protected:
    const ch*           m_szPoolName;
    clsize              m_nMaxUnit;         // 最大单元的尺寸, 如果请求内存大于这个就直接new
    clsize              m_nMemPoolSize;
    MemPool*            m_pMemPoolObj;
    int                 m_nMaxPoolIndex;
    int                 m_nPloyCount;
    const ALLOCPLOY*    m_aPloy;
    //u8*                 m_pPool;
    //clsize              m_uBlockPoolCount;
    //MemUnitGroup*       m_aBlockPool;
    //u32                 m_uAllocCount;    // 只记录在池中分配的数量,池满后new出来的不记录
    clstd::Locker       m_locker;
    //u32                 m_bDestructor;
#ifdef TRACK_ALLOCATOR
    u8*                 m_pPoolEnd;
    clhash_map<void*, int>  m_dbgAlloc;
#endif // TRACK_ALLOCATOR
    void CalcMemPool     ();

  public:
    Allocator(const ch*szName, const ALLOCPLOY* pAllocPloy);
    ~Allocator();

    void*   Alloc             (clsize nBytes, clsize* pCapacity);
    void    Free              (void* ptr);
    void*   Realloc           (void* ptr, clsize nOldRefBytes, clsize nBytes, clsize* pCapacity);
    //u32     GetLastCapacity   ();  // TODO: 最后一次分配的容量, 感觉用法有点山寨!
  };

  // 使用new/delete的标准分配函数
  class StdAllocator
  {
  public:
    void*   Alloc             (clsize nBytes, clsize* pCapacity);
    void    Free              (void* ptr);
    void*   Realloc           (void* ptr, clsize nOldRefBytes, clsize nBytes, clsize* pCapacity);
  };

  template<size_t _count>
  class LocalAllocator
  {
    s8 m_buffer[_count];
  public:
    void*   Alloc             (clsize nBytes, clsize* pCapacity);
    void    Free              (void* ptr);
    void*   Realloc           (void* ptr, clsize nOldRefBytes, clsize nBytes, clsize* pCapacity);
  };

  template<size_t _count>
  void* clstd::LocalAllocator<_count>::Alloc( clsize nBytes, clsize* pCapacity )
  {
    if(nBytes <= _count) {
      *pCapacity = _count;
      return m_buffer;
    }
    else {
      *pCapacity = nBytes;
      return malloc(nBytes);
    }
  }

} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CL_ALLOCATOR_H_