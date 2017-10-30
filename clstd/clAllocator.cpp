#include "clstd.h"
namespace clstd
{
  // MemPool 内存结构:
  // MemPool class
  // MemUnitGroup struct 0
  // MemUnitGroup struct 1
  // ...
  // MemUnitGroup struct n
  // MemUnitGroup 0's memory pool
  // MemUnitGroup 1's memory pool
  // MemUnitGroup 0's memory pool
  // ...
  // MemUnitGroup n's memory pool

  namespace Internal
  {
    MemUnitGroup::MemUnitGroup()
      : m_pBegin    (NULL)
      , m_pEnd      (NULL)
      , m_paPtrTable(NULL)
      , m_pMaskTable(NULL)
      , m_uGrain    (0)
      , m_uCount    (0)
      , m_uCapacity (0)
    {
    }

    void* MemUnitGroup::Initialize(void* pBegin, u32 uGrain, u32 uCapacity)
    {
#ifdef _DEBUG
      for(int i = 0; i < 1024 / 4; i++)
        ((u32*)pBegin)[i] = 0xcc90cc90;

      pBegin = ((u8*)pBegin + 1024);
      const clsize cbMask = GetMaskTableSize(uGrain, uCapacity) - 1024;
#else
      const clsize cbMask = GetMaskTableSize(uGrain, uCapacity);
#endif // _DEBUG

      const clsize cbPtr  = GetPtrTableSize(uGrain, uCapacity);

      m_pBegin = (u8*)pBegin + cbMask + cbPtr;
      m_uGrain = uGrain;
      m_uCount = uCapacity;
      m_uCapacity = uCapacity;
      m_pEnd = ((u8*)m_pBegin + uGrain * uCapacity);

      m_pMaskTable = (u32*)pBegin;
      m_paPtrTable = (void**)((u8*)pBegin + cbMask);
      pBegin = (u8*)pBegin + cbMask + cbPtr;
      for(u32 i = 0; i < uCapacity; i++)
      {
        m_paPtrTable[i] = ((u8*)pBegin + uGrain * i);
      }
      memset(m_pMaskTable, 0, cbMask);
      return m_pEnd;
    }

    void MemUnitGroup::Finalize(const ch* szName)
    {
#ifdef _DEBUG
      if(m_uCount != m_uCapacity)
      {
        TRACE("### Something in Allocator(name:%s, count:%d) not released! ###\n", 
          szName == NULL ? "<noname>" : szName, m_uCapacity - m_uCount);
      }
      u32* pPatternCheck = m_pMaskTable - 1024 / sizeof(u32);
      for(int i = 0; i < 1024 / 4; i++)
      {
        if(pPatternCheck[i] != 0xcc90cc90)
        {
          TRACE("### Pattern has been modified! ###\n");
          break;
        }
      }
#endif // _DEBUG
    }

    clsize MemUnitGroup::GetPtrTableSize(u32 uGrain, u32 uCapacity)
    {
      return (uCapacity * sizeof(void*));
    }

    clsize MemUnitGroup::GetMaskTableSize(u32 uGrain, u32 uCapacity)
    {
      clsize cbMask = ((uCapacity + 7) / 8);
      cbMask = ((cbMask + 3) / 4) * 4;  // 4字节对齐
#ifdef _DEBUG
      // Debug 版在Block前面预留1k的校验区域
      cbMask += 1024;
#endif // _DEBUG
      return cbMask;
    }

    u32 MemUnitGroup::GetGrain()
    {
      return m_uGrain;
    }

    bool MemUnitGroup::PtrInPool(void* ptr)
    {
      return ((u32_ptr)ptr >= (u32_ptr)m_pBegin) &&
        ((u32_ptr)ptr <= (u32_ptr)m_pEnd - m_uGrain) &&
        (((u32_ptr)ptr - (u32_ptr)m_pBegin) % m_uGrain) == 0;
    }

    bool MemUnitGroup::MatchSize(clsize uSize)
    {
      return m_uGrain >= uSize;
    }

    void* MemUnitGroup::Alloc()
    {
      if(m_uCount == 0)
        return NULL;

      void* ptr = m_paPtrTable[--m_uCount];
      u32_ptr idxMask = ((u32_ptr)ptr - (u32_ptr)m_pBegin) / m_uGrain;
      ASSERT(TESTBIT(m_pMaskTable[idxMask >> 5], idxMask & 31) == 0);
      SETBIT(m_pMaskTable[idxMask >> 5], idxMask & 31);
      return ptr;
    }

    void MemUnitGroup::Free(void* ptr)
    {
      u32_ptr idxMask = ((u32_ptr)ptr - (u32_ptr)m_pBegin) / m_uGrain;
      ASSERT(TESTBIT(m_pMaskTable[idxMask >> 5], idxMask & 31) != 0);

      m_paPtrTable[m_uCount++] = ptr;
      RESETBIT(m_pMaskTable[idxMask >> 5], idxMask & 31);
    }

    MemPool::MemPool(Allocator* pAllocator, int nIndex, clsize nGroupCount)
      : m_pAllocator  (pAllocator)
      , m_pPool       (NULL)
      , m_uGroupCount (nGroupCount)
      , m_aGroup      (NULL)
      , m_uAllocCount (0)
      , m_nIndex      (nIndex)
      , m_pNext       (NULL)
    {
      ASSERT(m_uGroupCount != 0);
    }

    void MemPool::SetPool(MemUnitGroup* aGroup, u8* pPool)
    {
      ASSERT(m_pPool == NULL && 
        m_aGroup == NULL && m_uAllocCount == 0);

      const ALLOCPLOY* const pAllocPloy = m_pAllocator->m_aPloy;

      m_pPool = (u8*)pPool;
#ifdef TRACK_ALLOCATOR
      m_pPoolEnd = (u8*)aGroup + uTotalSize;
#endif // TRACK_ALLOCATOR

      m_aGroup = aGroup;

      void* pBlockBegin = pPool;
      for(clsize i = 0; i < m_uGroupCount; i++)
      {
        pBlockBegin = m_aGroup[i].Initialize(
          pBlockBegin, pAllocPloy[i].uGrain, pAllocPloy[i].uCapacity);
      }
    }

    void MemPool::DeallocPool(b32 bDelNext)
    {
      if( ! m_pPool) {
        return;
      }
      //if(m_uAllocCount != 0) {
      //  return;
      //}
      const ch* szPoolName = m_pAllocator->m_szPoolName == NULL ? "<noname>" : m_pAllocator->m_szPoolName;

      for(clsize i = 0; i < m_uGroupCount; i++)
        m_aGroup[i].Finalize(szPoolName);

      //delete m_pPool;

      //TRACE("# Allocator's(%s) pool has been released!\n", szPoolName);

      m_pPool       = NULL;
      m_uGroupCount = 0;
      m_aGroup      = NULL;

      if(bDelNext && m_pNext)
      {
        m_pNext->DeallocPool(bDelNext);
        delete m_pNext;
        m_pNext = NULL;
      }

      //m_uLastCapacity   = 0;
    }

    void* MemPool::Alloc(clsize nBytes, clsize* pCapacity)
    {
      void* ptr;

      ASSERT(m_pPool);
      ASSERT(nBytes <= m_pAllocator->m_nMaxUnit);
      //if(m_pPool == NULL) {
      //  AllocPool();
      //}

      for(clsize i = 0; i < m_uGroupCount; i++)
      {
        if(m_aGroup[i].MatchSize(nBytes) && 
          (ptr = m_aGroup[i].Alloc()) != NULL)
        {
          if(pCapacity) {
            *pCapacity = m_aGroup[i].GetGrain();
          }
          m_uAllocCount++;
#ifdef TRACK_ALLOCATOR
          TRACE("alloc:%08lx Grain:%d nBytes:%d\n", ptr, m_uLastCapacity, nBytes);
          if(m_dbgAlloc.find(ptr) == m_dbgAlloc.end())
            m_dbgAlloc[ptr] = m_uLastCapacity;
          else
            ASSERT(0);
#endif // TRACK_ALLOCATOR
          return ptr;
        }
      }

      if( ! m_pNext) {
        m_pNext = MemPool::CreateMemPool(m_pAllocator, m_nIndex + 1);
      }
      return m_pNext->Alloc(nBytes, pCapacity);
    }

    clsize MemPool::FreeInGroups(void* ptr)
    {
      for(clsize i = 0; i < m_uGroupCount; i++)
      {
        if(m_aGroup[i].PtrInPool(ptr))
        {
#ifdef TRACK_ALLOCATOR
          TRACE("free:%08lx\n", ptr);
          m_dbgAlloc.erase(m_dbgAlloc.find(ptr));
#endif // TRACK_ALLOCATOR
          m_aGroup[i].Free(ptr);          
          return --m_uAllocCount;
        }
      }
      
      // 进入到这个函数时已经判断ptr是否属于MemPool
      // 中的某个Group了, 所以绝对不可能到这里
      CLBREAK;
      return 0;
    }

    clsize MemPool::Free(void* ptr)
    {
      if(ptr >= m_aGroup[0].m_pBegin && ptr <= m_aGroup[m_uGroupCount - 1].m_pEnd) {
        return FreeInGroups(ptr);
      }
      else if( ! m_pNext) {
        return (clsize)-1;
      }
      else if(clsize ret = m_pNext->Free(ptr)) {
        return ret;
      }

      MemPool* pNext = m_pNext;
      m_pNext = m_pNext->m_pNext;

      pNext->DeallocPool(FALSE);
      delete pNext;

      return m_uAllocCount;      
    }

    void* MemPool::Realloc(void* ptr, void* pNewPtr, clsize nBytes, clsize* pCapacity)
    {
      // 返回值: 1.NULL说明这个内存不在MemPool管理中
      //        2.等于ptr, 说明原有容量够用
      //        3.新的pNewtr, 重新分配了内存或者使用参数内存并释放了原有的
      ASSERT((pNewPtr != NULL) ||
        (nBytes <= m_pAllocator->m_nMaxUnit && pNewPtr == NULL));

      if(ptr >= m_aGroup[0].m_pBegin && ptr <= m_aGroup[m_uGroupCount - 1].m_pEnd) {
        for(clsize i = 0; i < m_uGroupCount; i++)
        {
          if(m_aGroup[i].PtrInPool(ptr))
          {
            const clsize nCapacity = m_aGroup[i].GetGrain();

            // 原有的容量能够满足需求
            if(nBytes <= nCapacity) {
              if(pCapacity) {
                *pCapacity = nCapacity;
              }
              return ptr;
            }
            // 原有容量不够用就分配一个新的内存
            if( ! pNewPtr) {
              pNewPtr = m_pAllocator->m_pMemPoolObj->Alloc(nBytes, pCapacity);
            }
            ASSERT(pNewPtr != NULL);

            memcpy(pNewPtr, ptr, nCapacity);
            m_uAllocCount--;
            m_aGroup[i].Free(ptr);
            return pNewPtr;
          }
        }
        CLBREAK;  // 不应该出现ptr输入这个MemPool却不输入MemGroups的情况!
      }

      if(m_pNext) {
        return m_pNext->Realloc(ptr, pNewPtr, nBytes, pCapacity);
      }

      return NULL;
    }

    MemPool* MemPool::CreateMemPool(Allocator* pAllocator, int nIndex)
    {
      const int& nPloyCount = pAllocator->m_nPloyCount;
      const clsize cbGroupArray = nPloyCount * sizeof(MemUnitGroup);
      const clsize cbPoolObjSize = sizeof(MemPool);

      CLBYTE* pAllThings = new CLBYTE[pAllocator->m_nMemPoolSize];
      MemPool* pPoolObj = new(pAllThings) MemPool(pAllocator, nIndex, nPloyCount);
      pPoolObj->SetPool((MemUnitGroup*)(pAllThings + cbPoolObjSize), pAllThings + cbPoolObjSize + cbGroupArray);

      // 更新最大索引记录
      pAllocator->m_nMaxPoolIndex = pAllocator->m_nMaxPoolIndex > nIndex
        ? pAllocator->m_nMaxPoolIndex : nIndex;

      return pPoolObj;
    }

  } // namespace Internal

  Allocator::Allocator(const ch* szName, const ALLOCPLOY* pAllocPloy)
    : m_szPoolName      (szName)
		, m_nMaxUnit(0)
		, m_nMemPoolSize(0)
		, m_pMemPoolObj(NULL)
		, m_nMaxPoolIndex   (0)
		, m_nPloyCount(0)
		, m_aPloy(pAllocPloy)
	{
    ASSERT(pAllocPloy != NULL);
    CalcMemPool();
  }

  Allocator::~Allocator()
  {
    if(m_pMemPoolObj)
    {
      m_pMemPoolObj->DeallocPool(TRUE);
      delete m_pMemPoolObj;
      m_pMemPoolObj = NULL;
    }
    TRACE("# Allocator(%s) total allocate %lu bytes.\n", 
      m_szPoolName, (m_nMaxPoolIndex + 1) * m_nMemPoolSize);
  }

  void* Allocator::Alloc(clsize nBytes, clsize* pCapacity)
  {
    if(nBytes > m_nMaxUnit)
    {
      if(pCapacity) {
        *pCapacity = nBytes;
      }
      return new u8[nBytes];
    }

    m_locker.Lock();
    if( ! m_pMemPoolObj) {
      m_pMemPoolObj = MemPool::CreateMemPool(this, 0);
    }

    void* ptr = m_pMemPoolObj->Alloc(nBytes, pCapacity);
    ASSERT(ptr);
    m_locker.Unlock();
    return ptr;
  }

  void Allocator::Free(void* ptr)
  {
    m_locker.Lock();
    if(m_pMemPoolObj && m_pMemPoolObj->Free(ptr) == (clsize)-1) {
      delete (u8*)ptr;
    }
    m_locker.Unlock();
  }

  void* Allocator::Realloc(void* ptr, clsize nOldRefBytes, clsize nBytes, clsize* pCapacity)
  {
    // nOldRefBytes 只是参考, 具体还是要根据ptr亲自找到真实容量, 
    // 也就是说 nOldRefBytes 是不被信任的, 不要参与断言

    void* pNew = NULL;

    m_locker.Lock();
    if( ! m_pMemPoolObj) // ptr肯定是不受MemPool控制的
    {      
      pNew = Alloc(nBytes, pCapacity);
      m_locker.Unlock();

      memcpy(pNew, ptr, nBytes < nOldRefBytes ? nBytes : nOldRefBytes);
      delete (u8*)ptr;
      return pNew;
    }
    m_locker.Unlock();

    if(nBytes > m_nMaxUnit) {
      if(pCapacity) {
        *pCapacity = nBytes;
      }
      pNew = new u8[nBytes];
      m_locker.Lock();
      void* pRet = m_pMemPoolObj->Realloc(ptr, pNew, nBytes, pCapacity);
      m_locker.Unlock();
      ASSERT(pRet == NULL || pRet == pNew);

      // 原有内存也不在MemPool管理之中
      if( ! pRet) {

        // 参考大小完全够用, 更新数据, 然后返回
        if(nOldRefBytes >= nBytes) {
          if(pCapacity) {
            *pCapacity = nOldRefBytes;
          }
          delete[] (u8*)pNew;
          return ptr;
        }

        // 处理分配了更大内存的情况
        memcpy(pNew, ptr, nOldRefBytes);
        delete (u8*)ptr;
        if(pCapacity) {
          *pCapacity = nBytes;
        }
      }
      return pNew;
    }
    else {
      m_locker.Lock();
      pNew = m_pMemPoolObj->Realloc(ptr, NULL, nBytes, pCapacity);
      m_locker.Unlock();

      if(pNew) {
        return pNew;
      }
      else if(nBytes < nOldRefBytes) {
        if(pCapacity) {
          *pCapacity = nOldRefBytes;
        }
        return ptr;
      }

      // 如果ptr不再MemPool管理中, 则一定比 m_nMaxUnit 大
      m_locker.Lock();
      pNew = m_pMemPoolObj->Alloc(nBytes, pCapacity);
      ASSERT(pNew);
      m_locker.Unlock();

      // 如果执行这个分支
      // ptr 是不受MemPool控制的, 容量一定大于nBytes
      ASSERT(nBytes < nOldRefBytes); // 这个只是检查, 如果nOldRefBytes不靠谱则断言无效

      memcpy(pNew, ptr, nBytes); // 截断方式的复制
      delete (u8*)ptr;
      return pNew;
    }
  }

  void Allocator::CalcMemPool()
  {
    const ALLOCPLOY* const pAllocPloy = m_aPloy;
    auto& i = m_nPloyCount;
    for(i = 0;; i++)
    {
      if(pAllocPloy[i].uGrain == 0 || pAllocPloy[i].uCapacity == 0)
        break;
      m_nMemPoolSize += (pAllocPloy[i].uGrain * pAllocPloy[i].uCapacity);
      m_nMemPoolSize += MemUnitGroup::GetPtrTableSize(pAllocPloy[i].uGrain, pAllocPloy[i].uCapacity);
      m_nMemPoolSize += MemUnitGroup::GetMaskTableSize(pAllocPloy[i].uGrain, pAllocPloy[i].uCapacity);

      m_nMaxUnit = pAllocPloy[i].uGrain > m_nMaxUnit
        ? pAllocPloy[i].uGrain : m_nMaxUnit;
    }
    //m_uGroupCount = i;
    const clsize cbGroupArray = m_nPloyCount * sizeof(MemUnitGroup);
    const clsize cbPoolObjSize = sizeof(MemPool);
    m_nMemPoolSize += cbGroupArray + cbPoolObjSize;

    //TRACE("# Allocator pool size: %d\n", uTotalSize);
  }
  
  //////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
# define STD_ALLOC_EXTRA_LENGTH 32
#else
# define STD_ALLOC_EXTRA_LENGTH 0
#endif

#define ENABLE_HEAP_ALLOC

#ifdef _CL_SYSTEM_WINDOWS
  StdAllocator::StdAllocator()
    : m_heap(INVALID_HANDLE_VALUE)
    //, m_nAllocCount(0)
  {
#if defined(ENABLE_HEAP_ALLOC)
    m_heap = HeapCreate(0, 1024 * 1024, 0);
#endif
  }

  StdAllocator::~StdAllocator()
  {
#if defined(ENABLE_HEAP_ALLOC)
    //if(m_nAllocCount == 0) {
      HeapDestroy(m_heap);
      m_heap = INVALID_HANDLE_VALUE;
    //}
#endif
  }
#endif // #ifdef _CL_SYSTEM_WINDOWS



  void* StdAllocator::Alloc( clsize nBytes, clsize* pCapacity )
  {
    const clsize nCapacity = ALIGN_4(nBytes + sizeof(clsize));
#ifdef _CL_SYSTEM_WINDOWS
# if defined(ENABLE_HEAP_ALLOC)
    //if(m_heap == INVALID_HANDLE_VALUE) {
    //  m_heap = HeapCreate(0, 1024 * 1024, 0);
    //}
    ASSERT(m_heap != INVALID_HANDLE_VALUE); // 不支持全局/静态类使用这个分配
    void* ptr = HeapAlloc(m_heap, 0, sizeof(CLBYTE) * (nCapacity + STD_ALLOC_EXTRA_LENGTH));
    //m_nAllocCount++;
# else
    void* ptr = new CLBYTE[(nCapacity + STD_ALLOC_EXTRA_LENGTH)];
# endif
#else
    void* ptr = malloc(sizeof(CLBYTE) * (nCapacity + STD_ALLOC_EXTRA_LENGTH)); // new CLBYTE[nCapacity + STD_ALLOC_EXTRA_LENGTH];
#endif

    *(clsize*)ptr = nCapacity;
    *pCapacity = nCapacity - sizeof(clsize);

#if defined(_DEBUG) && (STD_ALLOC_EXTRA_LENGTH > 0)
    /*
    memset((CLBYTE*)ptr + nCapacity, 0xcc, STD_ALLOC_EXTRA_LENGTH);
    /*/
    for(int i = 0; i < STD_ALLOC_EXTRA_LENGTH; i++)
    {
      ((CLBYTE*)ptr + nCapacity)[i] = (i + 1);
    }
    //*/
#endif // #if defined(_DEBUG) && (STD_ALLOC_EXTRA_LENGTH > 0)

    return ((clsize*)ptr) + 1;
  }

  void StdAllocator::Free( void* ptr )
  {
    ptr = ((clsize*)ptr) - 1;
#if defined(_DEBUG) && (STD_ALLOC_EXTRA_LENGTH > 0)
    CLBYTE* pCheck = (CLBYTE*)ptr + *(clsize*)ptr;
    for(int i = 0; i < STD_ALLOC_EXTRA_LENGTH; ++i) {
      /*
      if(*pCheck != 0xcc) {
        CLBREAK; // 内存写入溢出
      }
      /*/
      if(*pCheck != (i + 1)) {
        CLBREAK;
      }
      //*/
      ++pCheck;
    }
#endif // #if defined(_DEBUG) && (STD_ALLOC_EXTRA_LENGTH > 0)
    //delete[] (u8*)ptr;
#ifdef _CL_SYSTEM_WINDOWS
# if defined(ENABLE_HEAP_ALLOC)
    HeapFree(m_heap, 0, ptr);
    //--m_nAllocCount;
# else
    delete ptr;
# endif
#else
    free(ptr);
#endif
  }

  void* StdAllocator::Realloc( void* ptr, clsize nOldRefBytes, clsize nBytes, clsize* pCapacity )
  {
    nOldRefBytes = *(((clsize*)ptr) - 1);
    if(nOldRefBytes >= nBytes) {
      return ptr;
    }

    void* ptrNew = Alloc(nBytes, pCapacity);
    memcpy(ptrNew, ptr, nOldRefBytes);
    Free(ptr);
    return ptrNew;
  }

} // namespace clstd
