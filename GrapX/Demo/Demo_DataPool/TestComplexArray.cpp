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
    GXUINT nNumOfDel = 10; // ����ɾ��������
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

void TestWatcher(DataPool* pDataPool)
{
  WatchTest* pTest = new WatchTest;
  pTest->AddRef();

  // ����watch���ı����
  GXLPCSTR szWantToWatch = "sTestC.name";
  s_bWatch = TRUE;
  pDataPool->Watch(szWantToWatch, TestmpulseProc, NULL);
  pDataPool->Watch(szWantToWatch, pTest);

  MOVariable var;
  pDataPool->QueryByExpression("sTestC.name", &var);
  TRACE("before:%s\n", var.ToStringA());
  var.Set("set new string.");

  pDataPool->Ignore(szWantToWatch, TestmpulseProc);
  pDataPool->Ignore(szWantToWatch, pTest);
  s_bWatch = FALSE;
  var.Set("set new string 2.");


  // ���Խṹ������ȶ���watch
  GXBOOL result;
  result = pDataPool->Watch("DlgText", TestmpulseProc, NULL);
  ASSERT( ! result); // ��̬����-����ʧ��
  
  result = pDataPool->Watch("sTestC", TestmpulseProc, NULL);
  ASSERT( ! result); // �ṹ��-����ʧ��

  result = pDataPool->Watch("aStt", TestmpulseProc, NULL);
  ASSERT( ! result); // ��̬����-����ʧ��
  
  result = pDataPool->Watch("strName", TestmpulseProc, NULL);
  ASSERT(result); // �����ɹ�

  result = pDataPool->Watch("strName", pTest);
  ASSERT(result); // �����ɹ�

  result = pDataPool->Watch("DlgText[0]", TestmpulseProc, NULL);
  ASSERT( ! result); // ��̬����Ԫ��-����ʧ��

  result = pDataPool->Watch("aStt[0]", TestmpulseProc, NULL);
  ASSERT(result); // ��̬����-�����ɹ�

  SAFE_RELEASE(pTest);
}


// ����������ԱȽϸ��ӵĽṹ��+����
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