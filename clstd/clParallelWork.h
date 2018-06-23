#ifndef _CLSTD_PARALLEL_WORK_H_
#define _CLSTD_PARALLEL_WORK_H_

// 多线程处理大量并行工作的类

namespace clstd
{
  class ParallelWork
  {
  public:
    ParallelWork(int nNumOfProcessors = 0);
    virtual ~ParallelWork();

    void ForEach(void* pArray, size_t cbElement, size_t count, std::function<void(void*)> fn);
    void ForEach(void* pDestArray, size_t cbDestElement, void* pSrcArray, size_t cbSrcElement, size_t count, std::function<void(void*, const void*)> fn);

    template<class _ElementT>
    void ForEach(clvector<_ElementT>& rArray, std::function<void(void*)> fn)
    {
      ForEach(&rArray.front(), sizeof(_ElementT), rArray.size(), fn);
    }

    template<class _DestT, class _SrcT>
    void ForEach(clvector<_DestT>& rDestArray, const clvector<_SrcT>& rSrcArray, std::function<void(void*, const void*)> fn)
    {
      ForEach(&rDestArray.front(), sizeof(_DestT), &rDestArray.front(), sizeof(_SrcT),
        rDestArray.size() < rSrcArray.size() ? rDestArray.size() : rSrcArray.size(), fn);
    }

    template<class _ElementT>
    void ForEach(cllist<_ElementT>& rList, std::function<void(void*)> fn)
    {
      clvector<_ElementT> rArray;
      rArray.insert(rArray.begin(), rList.begin(), rList.end());
      ForEach(&rArray.front(), sizeof(_ElementT), rArray.size(), fn);
      
      cllist<_ElementT>::iterator it_dest = rList.begin();
      clvector<_ElementT>::iterator it_src = rArray.begin();
      for(; it_dest != rList.end(); ++it_dest, ++it_src)
      {
        *it_dest = *it_src;
      }
    }

    template<class _DestT, class _SrcT>
    void ForEach(cllist<_DestT>& rDestList, const cllist<_SrcT>& rSrcList, std::function<void(void*, const void*)> fn)
    {
      clvector<_DestT> rDestArray;
      clvector<_SrcT> rSrcArray;
      rDestArray.insert(rDestArray.begin(), rDestList.begin(), rDestList.end());
      rSrcArray.insert(rSrcArray.begin(), rSrcList.begin(), rSrcList.end());

      ForEach(&rDestArray.front(), sizeof(_DestT), &rSrcArray.front(), sizeof(_SrcT), 
        rDestArray.size() < rSrcArray.size() ? rDestArray.size() : rSrcArray.size(), fn);

      cllist<_DestT>::iterator it_dest = rDestList.begin();
      clvector<_DestT>::iterator it_src = rDestArray.begin();
      for(; it_dest != rDestList.end(); ++it_dest, ++it_src)
      {
        *it_dest = *it_src;
      }
    }

  protected:
    void WaitAll(b32 bDestroy);
    clvector<clstd::Thread*> m_ThreadPool;
  };

} // namespace clstd

#endif // _CLSTD_PARALLEL_WORK_H_