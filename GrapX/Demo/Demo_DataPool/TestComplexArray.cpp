#include <tchar.h>
#include <GrapX.H>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include "TestObject.h"
#include "TestDataPool.h"

void TryGet(DataPool* pDataPool, GXLPCSTR szExpression)
{
  MOVariable var;
  GXBOOL bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
}

void AllocDynArray(DataPool* pDataPool, GXLPCSTR szExpression, int nCountInc)
{
  MOVariable var;
  GXBOOL bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
  MOVariable varNew = var.NewBack(nCountInc);
  ASSERT(varNew.IsValid());
}


void TestRemoveItem(DataPool* pDataPool)
{
  MOVariable var;
  if(pDataPool->QueryByExpression("words", &var))
  {
    clStringArrayW aCheckStrings;
    GXUINT nCount = var.GetLength();
    GXUINT nNumOfDel = 10; // 测试删除的数量
    for(GXUINT i = 0; i < nCount; i++)
    {
      aCheckStrings.push_back(var[i]["word"].ToStringW());
    }

    var.Remove(nCount - nNumOfDel, nNumOfDel);
    aCheckStrings.erase(aCheckStrings.begin() + nCount - nNumOfDel, aCheckStrings.begin() + nCount);

    nCount -= nNumOfDel;

    for(GXUINT i = 0; i < nCount; i++)
    {
      auto a = aCheckStrings[i];
      auto b = var[i]["word"].ToStringW();
      ASSERT(a == b);
    }

  }
}

void GXCALLBACK sTestCImpulseProc(DATAPOOL_IMPULSE* pImpulse)
{
  TRACE("received impulse var:%s\n", pImpulse->sponsor->ToStringA());
}

void TestWatcher(DataPool* pDataPool)
{
  pDataPool->Watch("sTestC.name", sTestCImpulseProc, NULL);

  MOVariable var;
  pDataPool->QueryByExpression("sTestC.name", &var);
  TRACE("before:%s\n", var.ToStringA());
  var.Set("set new string.");
}


// 这个用来测试比较复杂的结构体+数组
void TestComplexArray()
{
  DataPool* pDataPool = NULL;
  HRESULT result;

  result = DataPool::CompileFromFileW(&pDataPool, NULL, L"Test\\DataPool\\ComplexArray_main.txt");
  ASSERT(GXSUCCEEDED(result));

  //TryGet(pDataPool, "DynTest");
  TryGet(pDataPool, "SttTestA");

  //TryGet(pDataPool, "DynTest[2]");

  TryGet(pDataPool, "SttTestA[3]");

  TryGet(pDataPool, "SttTestA[3].SttB");
  //TryGet(pDataPool, "SttTestA[3].HDyn");
  TryGet(pDataPool, "SttTestA[3].SttB[2]");
  TryGet(pDataPool, "SttTestA[3].SttB[2].SttC");
  //TryGet(pDataPool, "SttTestA[3].SttB[2].DynC");
  TryGet(pDataPool, "SttTestA[0].SttB[1].DynC");

  TryGet(pDataPool, "SttTestA[3].SttB[2].SttC[1]");
  //TryGet(pDataPool, "SttTestA[3].SttB[2].SttC[5].aDynD");
  //TryGet(pDataPool, "SttTestA[3].SttB[2].SttC[5].aSttD");
  ////TryGet(pDataPool, "SttTestA[3].SttB[2].SttC[5].aDynD[5]");

  AllocDynArray(pDataPool, "DynTestA", 10);
  AllocDynArray(pDataPool, "DynTestA[0].DynB", 2);
  AllocDynArray(pDataPool, "DynTestA[0].DynB[0].DynC", 3);
  AllocDynArray(pDataPool, "DynTestA[0].DynB[1].DynC", 30);
  AllocDynArray(pDataPool, "DynTestA[1].DynB", 20);

  //TryGet(pDataPool, "DynTestA[3]");

  //TryGet(pDataPool, "DynTestA[3].SttB");
  TryGet(pDataPool, "DynTestA[3].DynB");

  //TryGet(pDataPool, "DynTestA[3].SttB[2]");
  //TryGet(pDataPool, "DynTestA[3].SttB[2].SttC");
  //TryGet(pDataPool, "DynTestA[3].SttB[2].DynC");

  //TryGet(pDataPool, "DynTest[3].SttB[2].SttC[5]");
  //TryGet(pDataPool, "DynTest[3].SttB[2].SttC[5].aDynD");
  //TryGet(pDataPool, "DynTest[3].SttB[2].SttC[5].aSttD");
  MOVariable varDlgText;
  pDataPool->QueryByExpression("DlgText", &varDlgText);
  ASSERT(varDlgText.IsValid());

  for(int i = 0; s_szExampleString[i] != NULL; ++i)
  {
    MOVariable var = varDlgText.NewBack();
    var.Set(s_szExampleString[i]);
  }

  TestWatcher(pDataPool);
  TestRemoveItem(pDataPool);

  //ENUM_DATAPOOL(pDataPool);

#ifdef _X86
  GXLPCWSTR szFilename = L"Test\\TestComplexArray.DPL";
#else
  GXLPCWSTR szFilename = L"Test\\TestComplexArray_x64.DPL";
#endif // #ifdef _X86
  pDataPool->SaveW(szFilename);

  DataPool* pDataPoolFromFile = NULL;
  DataPool::CreateFromFileW(&pDataPoolFromFile, NULL, szFilename, DataPoolLoad_ReadOnly);

  ENUM_DATAPOOL(pDataPoolFromFile);
  SAFE_RELEASE(pDataPool);
  SAFE_RELEASE(pDataPoolFromFile);
}