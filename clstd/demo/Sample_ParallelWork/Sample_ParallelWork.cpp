// Sample_ParallelWork.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "clstd.h"
#include "thread/clThread.h"
#include "clParallelWork.h"
#include "clUtility.h"

using namespace clstd;

struct NOR
{
  float length;
  float squarelen;
};

struct DESC
{
  float x;
  float y;
  float z;
  float nor;
};


void TestForEach1()
{
  ParallelWork pw;
  clvector<DESC> sVec;
  cllist<DESC> sList;
  sVec.reserve(1000);
  clstd::Rand r;
  for(size_t i = 0; i < sVec.capacity(); i++)
  {
    DESC d;
    d.x = r.randf();
    d.y = r.randf();
    d.z = r.randf();
    d.nor = 0;
    sVec.push_back(d);
  }
  sList.insert(sList.begin(), sVec.begin(), sVec.end());

  for(auto it = sVec.begin(); it != sVec.end(); ++it) {
    TRACE("%.2f,%.2f,%.2f,%.2f\n", it->x, it->y, it->z, it->nor);
  }

  //////////////////////////////////////////////////////////////////////////
  TRACE("ƽ����\n");
  pw.ForEach(&sVec.front(), sizeof(DESC), sVec.size(), [](void* pData)
  {
    DESC* desc = reinterpret_cast<DESC*>(pData);
    desc->nor = (desc->x * desc->x + desc->y * desc->y + desc->z * desc->z);
  });

  for(auto it = sVec.begin(); it != sVec.end(); ++it) {
    TRACE("%.2f,%.2f,%.2f,%.2f\n", it->x, it->y, it->z, it->nor);
  }

  pw.ForEach(sVec, [](void* pData)
  {
    DESC* desc = reinterpret_cast<DESC*>(pData);
    desc->nor = (desc->x * desc->x + desc->y * desc->y + desc->z * desc->z);
  });

  TRACE("Listƽ����\n");
  pw.ForEach(sList, [](void* pData)
  {
    DESC* desc = reinterpret_cast<DESC*>(pData);
    desc->nor = (desc->x * desc->x + desc->y * desc->y + desc->z * desc->z);
  });

  for(auto it = sList.begin(); it != sList.end(); ++it) {
    TRACE("%.2f,%.2f,%.2f,%.2f\n", it->x, it->y, it->z, it->nor);
  }


  //////////////////////////////////////////////////////////////////////////
  TRACE("��һ��\n");
  pw.ForEach(&sVec.front(), sizeof(DESC), sVec.size(), [](void* pData)
  {
    DESC* desc = reinterpret_cast<DESC*>(pData);
    desc->nor = sqrt(desc->x * desc->x + desc->y * desc->y + desc->z * desc->z);
  });

  for(auto it = sVec.begin(); it != sVec.end(); ++it) {
    TRACE("%.2f,%.2f,%.2f,%.2f\n", it->x, it->y, it->z, it->nor);
  }
}

void TestForEach2()
{
  ParallelWork pw;
  clvector<DESC> sVec;
  cllist<DESC> sList;
  cllist<NOR> sNors;
  sVec.reserve(1000);
  clstd::Rand r;
  for(size_t i = 0; i < sVec.capacity(); i++)
  {
    DESC d;
    d.x = r.randf();
    d.y = r.randf();
    d.z = r.randf();
    d.nor = 0;
    sVec.push_back(d);
  }
  sList.insert(sList.begin(), sVec.begin(), sVec.end());

  NOR n = { 0 };
  sNors.insert(sNors.begin(), sList.size(), n);


  TRACE("ԭʼ����\n");
  for(auto it = sNors.begin(); it != sNors.end(); ++it) {
    TRACE("%.2f,%.2f\t", it->squarelen, it->length);
  }

  //////////////////////////////////////////////////////////////////////////
  TRACE("\n=== TestForEach2 ===\n");
  pw.ForEach(sNors, sList, [](void* pData, const void* pSrc)
  {
    const DESC* desc = reinterpret_cast<const DESC*>(pSrc);
    NOR* pNor = reinterpret_cast<NOR*>(pData);

    // ��ƽ�����볤��
    pNor->squarelen = (desc->x * desc->x + desc->y * desc->y + desc->z * desc->z);
    pNor->length = sqrt(pNor->squarelen);
  });

  TRACE("\n���\n");
  for(auto it = sNors.begin(); it != sNors.end(); ++it) {
    TRACE("%.2f,%.2f\t", it->squarelen, it->length);
  }
  TRACE("\n");
}

int main()
{
  CLOG("Sample ParallelWork");

  TestForEach1();
  TestForEach2();

	return 0;
}
