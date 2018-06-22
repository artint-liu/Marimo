#include <clstd.h>
#include "clUtility.h"
#include "thread/clThread.h"
#include "clParallelWork.h"

namespace clstd
{
  //typedef clvector<> ;

  class WorkThread : public clstd::Thread
  {
  public:
    WorkThread(int index, int num)
      : m_index(index)
      , m_num(num)
      , m_pArray(NULL)
      , m_element(0)
      , m_pArray2(NULL)
      , m_element2(0)
      , m_count(0)
      , m_func(NULL)
    {
    }

    i32 StartRoutine() override
    {
      size_t i = m_index;
      if(m_func)
      {
        while(i < m_count)
        {
          m_func((void*)((size_t)m_pArray + i * m_element));
          i += (size_t)m_num;
        }
      }
      else if(m_func2)
      {
        while(i < m_count)
        {
          m_func2((void*)((size_t)m_pArray + i * m_element),
            (const void*)((size_t)m_pArray2 + i * m_element2));
          i += (size_t)m_num;
        }
      }
      return 0;
    }

    void SetData(void* pArray, size_t cbElement, size_t count, std::function<void(void*)> fn)
    {
      m_pArray = pArray;
      m_element = cbElement;
      m_count = count;
      m_func = fn;
    }

    void SetData(void* pDestArray, size_t cbDestElement, const void* pSrcArray, size_t cbSrcElement,
      size_t count, std::function<void(void*, const void*)> fn)
    {
      m_pArray   = pDestArray;
      m_element  = cbDestElement;
      m_pArray2  = pSrcArray;
      m_element2 = cbSrcElement;
      m_count    = count;
      m_func2    = fn;
    }

  private:
    int m_index;      // 线程索引
    int m_num;        // 总线程数

    void* m_pArray;
    size_t m_element; // 数组元素尺寸
    const void* m_pArray2;
    size_t m_element2;// 数组元素尺寸
    size_t m_count;   // 数组长度
    std::function<void(void*)> m_func;
    std::function<void(void*, const void*)> m_func2;
  };

  ParallelWork::ParallelWork(int nNumOfProcessors /*= 0*/)
  {
    if(nNumOfProcessors < 0) {
      return;
    }
    else if(nNumOfProcessors == 0) {
      nNumOfProcessors = GetNumberOfProcessors();
    }

    m_ThreadPool.reserve(nNumOfProcessors);
    for(int i = 0; i < nNumOfProcessors; i++)
    {
      m_ThreadPool.push_back(static_cast<clstd::Thread*>(new WorkThread(i, nNumOfProcessors)));
    }
  }


  ParallelWork::~ParallelWork()
  {
    WaitAll(TRUE);
  }

  void ParallelWork::ForEach(void* pArray, size_t cbElement, size_t count, std::function<void(void*)> fn)
  {
    if(count == 0) {
      return;
    }
    else if(m_ThreadPool.empty()) { // 如果没有线程池则直接使用循环调用的方式
      for(size_t i = 0; i < count; i++)
      {
        fn(reinterpret_cast<void*>(reinterpret_cast<size_t>(pArray) + cbElement * i));
      }
      return;
    }

    auto it_end = m_ThreadPool.end();
    for(auto it = m_ThreadPool.begin(); it != it_end; ++it)
    {
      static_cast<WorkThread*>(*it)->SetData(pArray, cbElement, count, fn);
    }

    for(auto it = m_ThreadPool.begin(); it != it_end; ++it)
    {
      (*it)->Start();
    }

    WaitAll(FALSE);
  }

  void ParallelWork::ForEach(void* pDestArray, size_t cbDestElement,
    void* pSrcArray, size_t cbSrcElement, size_t count, std::function<void(void*, const void*)> fn)
  {
    if(count == 0) {
      return;
    }
    else if(m_ThreadPool.empty()) {
      for(size_t i = 0; i < count; i++)
      {
        fn(reinterpret_cast<void*>(reinterpret_cast<size_t>(pDestArray) + cbDestElement * i),
          reinterpret_cast<const void*>(reinterpret_cast<size_t>(pSrcArray) + cbSrcElement * i));
      }
      return;
    }

    auto it_end = m_ThreadPool.end();
    for(auto it = m_ThreadPool.begin(); it != it_end; ++it)
    {
      static_cast<WorkThread*>(*it)->SetData(
        pDestArray, cbDestElement, pSrcArray, cbSrcElement, count, fn);
    }

    for(auto it = m_ThreadPool.begin(); it != it_end; ++it)
    {
      (*it)->Start();
    }

    WaitAll(FALSE);
  }

  void ParallelWork::WaitAll(b32 bDestroy)
  {
    auto it_end = m_ThreadPool.end();
    for(auto it = m_ThreadPool.begin(); it != it_end; ++it)
    {
      (*it)->Wait(-1);
      if(bDestroy) {
        SAFE_DELETE(*it);
      }
    }

    if(bDestroy) {
      m_ThreadPool.clear();
    }
  }

}