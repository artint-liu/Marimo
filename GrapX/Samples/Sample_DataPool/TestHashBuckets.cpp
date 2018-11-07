//
// 用来测试 变量/成员变量 选择生成Hash表查找差别的单元测试
//

#include <tchar.h>
#include <GrapX.H>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include <exception>
#include "TestObject.h"
#include "TestDataPool.h"
#include <Shlwapi.h>

//size_t CompareDataPool(DataPool* pDataPoolA, DataPool* pDataPoolB);

void TestHashBuckets()
{
  GXLPCWSTR szFilename = _CLTEXT("Test\\DataPool\\ComplexArray_main.txt");
  GXLPCWSTR szFilenameFmt = _CLTEXT("Test\\ComplexArray_main%s.txt");
  
  CLOG("[begin test hash buckets]");

  // 编译生成
  {
    DataPool* pPoolWithHash = NULL;
    DataPool* pPoolWithoutHash = NULL;
    GXHRESULT result = NULL;


    // 带hash的DataPool
    result = DataPool::CompileFromFileW(&pPoolWithHash, NULL, szFilename);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not compiler file\"%s\"."), szFilename);
      return;
    }

    // 不带hash的DataPool
    result = DataPool::CompileFromFileW(&pPoolWithoutHash, NULL, szFilename, NULL, DataPoolCreation_NoHashTable);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not compiler file\"%s\"."), szFilename);
      return;
    }

    // 保存结果
    clStringW str;
    str.Format(szFilenameFmt, L"_Hash");
    pPoolWithHash->Save(str);

    str.Format(szFilenameFmt, L"_NoHash");
    pPoolWithoutHash->Save(str);

    // 释放
    SAFE_RELEASE(pPoolWithoutHash);
    SAFE_RELEASE(pPoolWithHash);
  }

  // 加载测试
  {
    DataPool* pPoolWithHash = NULL;
    DataPool* pPoolWithoutHash = NULL;
    GXHRESULT result = NULL;
    clStringW str;
    DataPoolCreation eLoad = DataPoolCreation_ReadOnly;

    str.Format(szFilenameFmt, L"_Hash");
    result = DataPool::CreateFromFileW(&pPoolWithHash, NULL, str, eLoad);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not load file\"%s\"."), str);
      return;
    }

    str.Format(szFilenameFmt, L"_NoHash");
    result = DataPool::CreateFromFileW(&pPoolWithoutHash, NULL, str, eLoad);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not load file\"%s\"."), str);
      return;
    }

    CLOG("完成加载两个DataPool");

    size_t count = CompareDataPool(pPoolWithHash, pPoolWithoutHash);
    CLOG("比较变量数量：%u", count);

    count = EnumerateVariables(pPoolWithHash);
    CLOG("枚举pPoolWithHash变量数量：%u", count);
    
    count = EnumerateVariables(pPoolWithoutHash);
    CLOG("枚举pPoolWithoutHash变量数量：%u", count);

    SAFE_RELEASE(pPoolWithHash);
    SAFE_RELEASE(pPoolWithoutHash);
  }


  CLOG("[end test hash buckets]");
}