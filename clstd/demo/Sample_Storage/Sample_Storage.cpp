// Sample_Storage.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "clstd.h"
#include "clString.h"
#include "clUtility.h"
#include "clRepository.h"
#pragma comment(lib, "clstd_d.lib")

typedef clvector<int> IntArray;
typedef clset<int> IntSet;



void TestBianrySearch()
{
  IntArray aSamples;
  IntSet aSort;

  // >>>>>>>>>>>>>>>>> ������������
  clstd::Rand r;
  r.srand(100);
  const int max_sample_range = 500;
  for(int i = 0; i < 100; i++)
  {
    aSort.insert(r.rand() % max_sample_range);
  }
  aSamples.insert(aSamples.end(), aSort.begin(), aSort.end());
  // <<<<<<<<<<<<<<<<< ������������

  int*const pBegin = &aSamples.front();
  int*const pEnd = &aSamples.back() + 1;

  auto fn = [](int* a, int key) -> int
  {
    return (*a - key);
  };

  // ��׼���� - ÿ�ζ����ҵ�
  for(size_t i = 0; i < aSamples.size(); i++)
  {
    int* r = clstd::BinarySearch(pBegin, pEnd, aSamples[i], fn);
    ASSERT(r != NULL);
  }

  // ��׼���� - ÿ�ζ��Ҳ���
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

  // ��������£�ֻ��һ��Ԫ�صĲ��ң����ҵ���
  {
    int* r = clstd::BinarySearch(pBegin, pBegin + 1, *pBegin, fn);
    ASSERT(r != NULL);
  }

  // ��������£�ֻ��һ��Ԫ�صĲ��ң��Ҳ�����
  {
    int* r = clstd::BinarySearch(pBegin, pBegin + 1, *pBegin + 1, fn);
    ASSERT(r == NULL);
  }
}


void TestBianryInsertPos()
{
  IntArray aSamples;
  IntSet aSort;

  // >>>>>>>>>>>>>>>>> ������������
  clstd::Rand r;
  r.srand(100);
  const int max_sample_range = 500;
  for(int i = 0; i < 100; i++)
  {
    aSort.insert(r.rand() % max_sample_range);
  }
  aSamples.insert(aSamples.end(), aSort.begin(), aSort.end());
  // <<<<<<<<<<<<<<<<< ������������

  int*const pBegin = &aSamples.front();
  int*const pEnd = &aSamples.back() + 1;

  auto fn = [](int* a, int key) -> int
  {
    return (*a - key);
  };

  // ��׼���� - ÿ�ζ����ҵ�
  for(size_t i = 0; i < aSamples.size(); i++)
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pEnd, aSamples[i], &bInsert, fn);
    ASSERT(bInsert == FALSE);
    ASSERT(r != NULL);
  }

  // ��׼���� - ÿ�ζ��Ҳ���
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
  
  // ��������£�ֻ��һ��Ԫ�صĲ��ң����ҵ���
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin, &bInsert, fn);
    ASSERT(bInsert == FALSE);
    ASSERT(r == pBegin);
  }

  // ��������£�ֻ��һ��Ԫ�صĲ��ң��Ҳ�����
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin + 1, &bInsert, fn);
    ASSERT(bInsert == TRUE);
    ASSERT(r > pBegin);
  }

  // ��������£�ֻ��һ��Ԫ�صĲ��ң��Ҳ�����
  {
    b32 bInsert;
    int* r = clstd::BinaryInsertPos(pBegin, pBegin + 1, *pBegin - 1, &bInsert, fn);
    ASSERT(bInsert == TRUE);
    ASSERT(r == pBegin);
  }
  //*/
}

//////////////////////////////////////////////////////////////////////////
void TestRepository1()
{
  clstd::Repository repo;
  struct TESTKEY
  {
    clstd::Repository::LPCSTR name;
    int num_samples;
    char sample;
  };
  TESTKEY test[] = {
    {"test1", 100, 0x12},
    {"test2", 16, 0x34},
    {"testtest1", 34, 0x56},
    {"test5", 100, 0x78},
    {"test4", 8, 0x7a},
    {"test3", 8, 0x7b},
    {"helloworld", 90, 0x23},
    {NULL}
  };

  for(int i = 0; test[i].name != NULL; i++)
  {
    CLBYTE* pData = new CLBYTE[test[i].num_samples];
    memset(pData, test[i].sample, test[i].num_samples);
    repo.SetKey(test[i].name, pData, test[i].num_samples);
    SAFE_DELETE_ARRAY(pData)
  }

  auto it_end = repo.end();
  for(auto it = repo.begin(); it != it_end; ++it)
  {
    CLOG("%s(%d)", it.name(), it.size());
    clstd::DumpMemory(it.ptr(), it.size());
  }

  CLNOP
}

int main(int argc, char* argv[])
{
  TestBianrySearch();
  TestBianryInsertPos();
  TestRepository1();
	return 0;
}

