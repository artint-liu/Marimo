#ifndef _CLSTD_STABLE_POOL_H_
#define _CLSTD_STABLE_POOL_H_

// StablePool 特性:
// 1.除非调用清理方法, PushBack分配出的结构体指针一直有效
// 2.适合在一定程度上某个结构体持续增长的情况

namespace clstd
{
  template<class _Ty>
  class StablePool
  {
  public:
    typedef _Ty T;

    struct POOL
    {
      _Ty* pBegin;
      _Ty* pEnd;
    };

    typedef clvector<POOL> PoolList;

  protected:
    PoolList m_PoolList;
    int      m_nElementCount; // 增长数量，实际分配内存是数量*sizeof(_Ty)
    _Ty*     m_pNew;

  public:
    StablePool(int nCount)
      : m_nElementCount(nCount), m_pNew(NULL)
    {
      ASSERT(IsPowerOfTwo(nCount)); // 必须是2的幂
    }

    ~StablePool()
    {
      Clear();
    }

    _Ty* Alloc()
    {
      ASSERT(m_pNew == NULL || (
        m_pNew >= m_PoolList.back().pBegin &&
        m_pNew < m_PoolList.back().pEnd));

      if(m_pNew == NULL || (++m_pNew) == m_PoolList.back().pEnd) {
        POOL pool = { NULL, NULL };
        m_pNew = new _Ty[m_nElementCount];
        pool.pBegin = m_pNew;
        pool.pEnd = pool.pBegin + m_nElementCount;
        m_PoolList.push_back(pool);
      }
      return m_pNew;
    }

    _Ty* PushBack(const _Ty& t) // 把t追加到最后， 并且返回一个稳定地址
    {
      _Ty* ptr = Alloc();
      *ptr = t;
      return ptr;
    }

    void Clear()
    {
      std::for_each(m_PoolList.begin(), m_PoolList.end(),
        [](POOL& pool)
      {
        delete[] pool.pBegin;
      });
      m_PoolList.clear();
      m_pNew = NULL;
    }
  };
} // namespace clstd

#endif // _CLSTD_STABLE_POOL_H_