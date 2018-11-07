//
// �������� ����/��Ա���� ѡ������Hash����Ҳ��ĵ�Ԫ����
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

  // ��������
  {
    DataPool* pPoolWithHash = NULL;
    DataPool* pPoolWithoutHash = NULL;
    GXHRESULT result = NULL;


    // ��hash��DataPool
    result = DataPool::CompileFromFileW(&pPoolWithHash, NULL, szFilename);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not compiler file\"%s\"."), szFilename);
      return;
    }

    // ����hash��DataPool
    result = DataPool::CompileFromFileW(&pPoolWithoutHash, NULL, szFilename, NULL, DataPoolCreation_NoHashTable);
    if(GXFAILED(result)) {
      SAFE_RELEASE(pPoolWithHash);
      SAFE_RELEASE(pPoolWithoutHash);
      CLOG_ERRORW(_CLTEXT("can not compiler file\"%s\"."), szFilename);
      return;
    }

    // ������
    clStringW str;
    str.Format(szFilenameFmt, L"_Hash");
    pPoolWithHash->Save(str);

    str.Format(szFilenameFmt, L"_NoHash");
    pPoolWithoutHash->Save(str);

    // �ͷ�
    SAFE_RELEASE(pPoolWithoutHash);
    SAFE_RELEASE(pPoolWithHash);
  }

  // ���ز���
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

    CLOG("��ɼ�������DataPool");

    size_t count = CompareDataPool(pPoolWithHash, pPoolWithoutHash);
    CLOG("�Ƚϱ���������%u", count);

    count = EnumerateVariables(pPoolWithHash);
    CLOG("ö��pPoolWithHash����������%u", count);
    
    count = EnumerateVariables(pPoolWithoutHash);
    CLOG("ö��pPoolWithoutHash����������%u", count);

    SAFE_RELEASE(pPoolWithHash);
    SAFE_RELEASE(pPoolWithoutHash);
  }


  CLOG("[end test hash buckets]");
}