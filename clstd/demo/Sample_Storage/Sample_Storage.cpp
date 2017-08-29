// Sample_Storage.cpp : 定义控制台应用程序的入口点。
//
#include "clstd.h"
#include "clString.h"
#include "clUtility.h"
#include "clRepository.h"

#if defined(_CL_ARCH_X86)
# ifdef _DEBUG
#   pragma comment(lib, "clstd.Win32.Debug_MD.lib")
# else
#   pragma comment(lib, "clstd.Win32.Release_MD.lib")
# endif
#elif defined(_CL_ARCH_X64)
# ifdef _DEBUG
#   pragma comment(lib, "clstd.x64.Debug_MD.lib")
# else
#   pragma comment(lib, "clstd.x64.Release_MD.lib")
# endif
#endif

typedef clvector<int> IntArray;
typedef clset<int> IntSet;


// 二分查找模板测试
void TestBianrySearch()
{
  CLOG(__FUNCTION__);

  IntArray aSamples;
  IntSet aSort;

  // >>>>>>>>>>>>>>>>> 生成样本数据
  clstd::Rand r;
  r.srand(100);
  const int max_sample_range = 500;
  for(int i = 0; i < 100; i++)
  {
    aSort.insert(r.rand() % max_sample_range);
  }
  aSamples.insert(aSamples.end(), aSort.begin(), aSort.end());
  // <<<<<<<<<<<<<<<<< 生成样本数据

  int*const pBegin = &aSamples.front();
  int*const pEnd = &aSamples.back() + 1;

  auto fn = [](int* a, int key) -> int
  {
    return (*a - key);
  };

  // 标准查找 - 每次都能找到
  for(size_t i = 0; i < aSamples.size(); i++)
  {
    int* r = clstd::BinarySearch(pBegin, pEnd, aSamples[i], fn);
    ASSERT(r != NULL);
  }

  // 标准查找 - 每次都找不到
  int n = 0;
  for(size_t i = 0; i < max_sample_range; i++)
  {
    if(n < (int)aSamples.size() && aSamples[n] == i) {
      n++;
      continue;
    }

    int* r = clstd::BinarySearch(pBegin, pEnd, i, fn);
    ASSERT(r == NULL);
  }

  // 极端情况下，只有一个元素的查找（能找到）
  {
    int* r = clstd::BinarySearch(pBegin, pBegin + 1, *pBegin, fn);
    ASSERT(r != NULL);
  }

  // 极端情况下，只有一个元素的查找（找不到）
  {
    int* r = clstd::BinarySearch(pBegin, pBegin + 1, *pBegin + 1, fn);
    ASSERT(r == NULL);
  }
}


void TestBianryInsertPos()
{
  CLOG(__FUNCTION__);

  IntArray aSamples;
  IntSet aSort;

  // >>>>>>>>>>>>>>>>> 生成样本数据
  clstd::Rand r;
  r.srand(100);
  const int max_sample_range = 500;
  for(int i = 0; i < 100; i++)
  {
    aSort.insert(r.rand() % max_sample_range);
  }
  aSamples.insert(aSamples.end(), aSort.begin(), aSort.end());
  // <<<<<<<<<<<<<<<<< 生成样本数据

  int*const pBegin = &aSamples.front();
  int*const pEnd = &aSamples.back() + 1;

  auto fn = [](int* a, int key) -> int
  {
    return (*a - key);
  };

  // 标准查找 - 每次都能找到
  for(size_t i = 0; i < aSamples.size(); i++)
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pEnd, aSamples[i], &bInsert, fn);
    ASSERT(bInsert == FALSE);
    ASSERT(r != NULL);
  }

  // 标准查找 - 每次都找不到
  int n = 0;
  for(size_t i = 0; i < max_sample_range; i++)
  {
    if(n < (int)aSamples.size() && aSamples[n] == i) {
      n++;
      continue;
    }

    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pEnd, i, &bInsert, fn);
    ASSERT(bInsert == TRUE);
    ASSERT(r <= pEnd);
    ASSERT(r >= pBegin);
    if(r < pEnd) {
      ASSERT((int)i < *r);
      if(r > pBegin) {
        ASSERT((int)i > *(r - 1));
      }
      else if(r == pBegin) {
        ASSERT((int)i < *pBegin);
      }
    } else if(r == pEnd) {
      ASSERT((int)i > *(pEnd - 1));
    }
  }
  
  // 极端情况下，只有一个元素的查找（能找到）
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin, &bInsert, fn);
    ASSERT(bInsert == FALSE);
    ASSERT(r == pBegin);
  }

  // 极端情况下，只有一个元素的查找（找不到）
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin + 1, &bInsert, fn);
    ASSERT(bInsert == TRUE);
    ASSERT(r > pBegin);
  }

  // 极端情况下，只有一个元素的查找（找不到）
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin - 1, &bInsert, fn);
    ASSERT(bInsert == TRUE);
    ASSERT(r == pBegin);
  }
  //*/
}

//////////////////////////////////////////////////////////////////////////
void DumpRepoMem(clstd::Repository& repo)
{
  auto it_end = repo.end();
  for(auto it = repo.begin(); it != it_end; ++it)
  {
    CLOG("%s(%d:%d)", it.name(), it.ptr(), it.size());
    clstd::DumpMemory(it.ptr(), it.size());
  }
}

void TestRepository1()
{
  CLOG(__FUNCTION__);

  clstd::Repository repo;
  struct TESTKEY
  {
    clstd::Repository::LPCSTR name;
    int num_samples;
    unsigned char sample;
  };

  TESTKEY test[] = {
    {"test1", 100, 0x12},
    {"test2", 16, 0x34},
    {"testtest1", 34, 0x56},
    {"test5", 100, 0x78},
    {"test4", 8, 0x7a},
    {"test2", 32, 0x35},
    {"test3", 8, 0x7b},
    {"helloworld", 90, 0x23},
    {"test5", 100, 0x77},
    {"testtest1", 32, 0xA6},
    {"test2", 8, 0x33},
    {NULL}
  };

  for(int i = 0; test[i].name != NULL; i++)
  {
    CLBYTE* pData = new CLBYTE[test[i].num_samples];
    memset(pData, test[i].sample, test[i].num_samples);
    CLOG("test[i].name: %s, %x, %d", test[i].name, pData, test[i].num_samples);
    repo.SetKey(test[i].name, pData, test[i].num_samples);
    CLOG("test[i].name: %s, %x, %d", test[i].name, pData, test[i].num_samples);
    SAFE_DELETE_ARRAY(pData)
  }

  DumpRepoMem(repo);

  repo.SaveToFile("test.repo");

  repo.LoadFromFile("test.repo");
  CLOG("=========================");
  DumpRepoMem(repo);
}

int main(int argc, char* argv[])
{
  size_t n = 0 - 10;
  CLOG("%llu", n);
  TestBianrySearch();
  TestBianryInsertPos();
  TestRepository1();
	return 0;
}

