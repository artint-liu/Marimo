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
      , m_count(0)
      , m_func(NULL)
    {
    }

    i32 StartRoutine() override
    {
      size_t i = m_index;
      while(i < m_count)
      {
        m_func((void*)((size_t)m_pArray + i * m_element));
        i += (size_t)m_num;
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

  private:
    int m_index;      // 线程索引
    int m_num;        // 总线程数

    void* m_pArray;
    size_t m_element; // 数组元素尺寸
    size_t m_count;   // 数组长度
    std::function<void(void*)> m_func;
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
    else if(m_ThreadPool.empty())
    {
      for(size_t i = 0; i < count; i++)
      {
        fn(reinterpret_cast<void*>((size_t)pArray + cbElement * i));
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