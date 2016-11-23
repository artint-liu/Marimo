#include <tchar.h>
#include <GrapX.H>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include <exception>
#include "TestObject.h"
#include "TestDataPool.h"
#include <Shlwapi.h>

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

static GXBOOL s_bWatch = FALSE;

void GXCALLBACK TestmpulseProc(DATAPOOL_IMPULSE* pImpulse)
{
  ASSERT(s_bWatch);
  TRACE("received impulse var:%s\n", pImpulse->sponsor->ToStringA());
}

class WatchTest : public DataPoolWatcher
{
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT AddRef()
  {
    return ++m_nRefCount;
  }

  GXHRESULT Release()
  {
    if(--m_nRefCount == 0)
    {
      delete this;
      return GX_OK;
    }
    return m_nRefCount;
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXVOID OnImpulse(LPCDATAIMPULSE pImpulse)
  {
    ASSERT(s_bWatch);
  }
};

void TestException(DataPool* pDataPool)
{
  MOVariable var;
  MOVariable var2;
  GXBOOL result = pDataPool->QueryByExpression("sTestC", &var); 
  ASSERT(result) ; // 静态结构体这个一定存在
  
  GXUINT Length = 0;
  try
  {
    var2 = var.IndexOf(1);
  }
  catch (GXLPCSTR str)
  {
    TRACE("访问结构体的\"数组元素\"会引发异常\n");
    TRACE("Exception:%s\n", str);
  }
}

void TestWatcher(DataPool* pDataPool)
{
  WatchTest* pTest = new WatchTest;
  pTest->AddRef();

  // 测试watch并改变变量
  GXLPCSTR szWantToWatch = "sTestC.name";
  s_bWatch = TRUE;

  // 关注变量
  pDataPool->Watch(szWantToWatch, TestmpulseProc, NULL);
  pDataPool->Watch(szWantToWatch, pTest);
  
  auto LambdaWatcher = pDataPool->WatchFor(szWantToWatch, [](LPCDATAIMPULSE pImpulse){
    ASSERT(s_bWatch);
    TRACE("received impulse var in lambda:%s\n", pImpulse->sponsor->ToStringA());
  });

  // 修改Var数据
  auto var = (*pDataPool)["sTestC.name"];
  TRACE("before:%s\n", var.ToStringA());
  var.Set("set new string.");

  // 注销数据关注
  pDataPool->Ignore(szWantToWatch, TestmpulseProc);
  pDataPool->Ignore(szWantToWatch, pTest);
  pDataPool->Ignore(szWantToWatch, LambdaWatcher);
  s_bWatch = FALSE;
  var.Set("set new string 2.");


  // 测试结构，数组等对象watch
  GXBOOL result;
  result = pDataPool->Watch("DlgText", TestmpulseProc, NULL);
  ASSERT( result); // 动态数组-期望成功
  
  result = pDataPool->Watch("sTestC", TestmpulseProc, NULL);
  ASSERT( ! result); // 结构体-期望失败

  result = pDataPool->Watch("aStt", TestmpulseProc, NULL);
  ASSERT( ! result); // 静态数组-期望失败
  
  result = pDataPool->Watch("strName", TestmpulseProc, NULL);
  ASSERT(result); // 期望成功

  result = pDataPool->Watch("strName", pTest);
  ASSERT(result); // 期望成功

  result = pDataPool->Watch("DlgText[0]", TestmpulseProc, NULL);
  ASSERT(result); // 动态数组元素-期望成功

  result = pDataPool->Watch("aStt[0]", TestmpulseProc, NULL);
  ASSERT(result); // 静态数组-期望成功

  SAFE_RELEASE(pTest);
}

#define TEST_GETNAMEID(NAME, RESULT) id = pDataPool->GetNameId(NAME); ASSERT((id != 0) == RESULT);

void TestGetNameId(DataPool* pDataPool)
{
  GXUINT id;
  TEST_GETNAMEID("CET4_WORD",  TRUE);
  TEST_GETNAMEID("DlgText",    TRUE);
  TEST_GETNAMEID("DynB",       TRUE);
  TEST_GETNAMEID("DynC",       TRUE);
  TEST_GETNAMEID("DynTestA",   TRUE);
  TEST_GETNAMEID("HEAD_A",     TRUE);
  TEST_GETNAMEID("LAST_C",     TRUE);
  TEST_GETNAMEID("MID_B",      TRUE);
  TEST_GETNAMEID("SttB",       TRUE);
  TEST_GETNAMEID("SttC",       TRUE);
  TEST_GETNAMEID("SttTestA",   TRUE);
  TEST_GETNAMEID("WORDCLASS",  TRUE);
  TEST_GETNAMEID("a",          TRUE);
  TEST_GETNAMEID("aDyn",       TRUE);
  TEST_GETNAMEID("aDynD",      TRUE);
  TEST_GETNAMEID("aPos",       TRUE);
  TEST_GETNAMEID("aStt",       TRUE);
  TEST_GETNAMEID("aSttD",      TRUE);
  TEST_GETNAMEID("aVertices",  TRUE);
  TEST_GETNAMEID("ad",         TRUE);
  TEST_GETNAMEID("bResult",    TRUE);
  TEST_GETNAMEID("chinese",    TRUE);
  TEST_GETNAMEID("conj",       TRUE);
  TEST_GETNAMEID("float",      TRUE);
  TEST_GETNAMEID("float3",     TRUE);
  TEST_GETNAMEID("float4",     TRUE);
  TEST_GETNAMEID("int",        TRUE);
  TEST_GETNAMEID("n",          TRUE);
  TEST_GETNAMEID("name",       TRUE);
  TEST_GETNAMEID("nameA",      TRUE);
  TEST_GETNAMEID("nameB",      TRUE);
  TEST_GETNAMEID("namesA",     TRUE);
  TEST_GETNAMEID("namesB",     TRUE);
  TEST_GETNAMEID("prep",       TRUE);
  TEST_GETNAMEID("sTestC",     TRUE);
  TEST_GETNAMEID("strName",    TRUE);
  TEST_GETNAMEID("string",     TRUE);
  TEST_GETNAMEID("vi",         TRUE);
  TEST_GETNAMEID("vi_n",       TRUE);
  TEST_GETNAMEID("vt",         TRUE);
  TEST_GETNAMEID("vt_vi_n",    TRUE);
  TEST_GETNAMEID("w",          TRUE);
  TEST_GETNAMEID("word",       TRUE);
  TEST_GETNAMEID("word_class", TRUE);
  TEST_GETNAMEID("words",      TRUE);
  TEST_GETNAMEID("x",          TRUE);
  TEST_GETNAMEID("y",          TRUE);
  TEST_GETNAMEID("z",          TRUE);

  TEST_GETNAMEID("AAA",       FALSE);
  TEST_GETNAMEID("sjd",       FALSE);
  TEST_GETNAMEID("zzz",       FALSE);
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

  TestException(pDataPool);
  TestWatcher(pDataPool);
  TestRemoveItem(pDataPool);

  ENUM_DATAPOOL(pDataPool);

#ifdef _X86
  GXLPCWSTR szFilename = L"Test\\TestComplexArray.DPL";
  GXLPCWSTR szFilename2 = L"Test\\TestComplexArray_x64.DPL";  // 跨平台的文件名
#else
  GXLPCWSTR szFilename = L"Test\\TestComplexArray_x64.DPL";
  GXLPCWSTR szFilename2 = L"Test\\TestComplexArray.DPL";      // 跨平台的文件名
#endif // #ifdef _X86
  pDataPool->SaveW(szFilename);

  DataPool* pDataPoolFromFile = NULL;
  DataPool::CreateFromFileW(&pDataPoolFromFile, NULL, szFilename, DataPoolLoad_ReadOnly);

  pDataPool->ExportDataToFile(L"Test\\TestComplexArray_export.txt");

  TestGetNameId(pDataPoolFromFile);
  TestGetNameId(pDataPool);

  ENUM_DATAPOOL(pDataPoolFromFile);
  SAFE_RELEASE(pDataPool);
  SAFE_RELEASE(pDataPoolFromFile);

  // 异构架文件加载测试
  if(PathFileExists(szFilename2))
  {
    DataPool* pAlternative = NULL;
    result = DataPool::CreateFromFileW(&pAlternative, NULL, szFilename2, DataPoolLoad_ReadOnly);
    if(GXSUCCEEDED(result)) {
      TestGetNameId(pAlternative);
      ENUM_DATAPOOL(pAlternative);
      SAFE_RELEASE(pAlternative);
    }
    else {
      CLBREAK;
    }
  }
}