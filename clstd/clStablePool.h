#ifndef _CLSTD_STABLE_POOL_H_
#define _CLSTD_STABLE_POOL_H_

// StablePool ����:
// 1.���ǵ���������, PushBack������Ľṹ��ָ��һֱ��Ч
// 2.�ʺ���һ���̶���ĳ���ṹ��������������

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
    int      m_nElementCount; // ����������ʵ�ʷ����ڴ�������*sizeof(_Ty)
    _Ty*     m_pNew;

  public:
    StablePool(int nCount)
      : m_nElementCount(nCount), m_pNew(NULL)
    {
      ASSERT(IsPowerOfTwo(nCount)); // ������2����
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

    _Ty* PushBack(const _Ty& t) // ��t׷�ӵ���� ���ҷ���һ���ȶ���ַ
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