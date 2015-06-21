#include <tchar.h>
#include <GrapX.H>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include "TestObject.h"
#include "TestDataPool.h"

#define HUGE_NUMBER 10000

// ������Ҫ�������Խϴ������ʹ�����
void TestHugeArray()
{
  static char* s_szCode =
    "#FILE "__FILE__"                \n"
    "#LINE 16                        \n"
    "/* �����ṹ��                  */\n"
    "struct RECT                     \n"
    "{                               \n"
    "  int left;    int top;         \n"
    "  int right;   int bottom;      \n"
    "};                              \n"
    "struct POINT {                  \n"
    "  int x;   int y;               \n"
    "};                              \n"
    " /* ��ʼ���õ��� */              \n"
    "struct MODULE {                 \n"
    "  unsigned int id;              \n"
    "  string name;                  \n"
    "  RECT rect;                    \n"
    "};                              \n"
    "                                \n"
    "int a;                          \n"
    "MODULE aModules[];              \n"
    "                                \n"
    "struct FRAME_MODULE {           \n"
    "  int idx;  int rotate;         \n"
    "  POINT offset;                 \n"
    "};                              \n"
    "                                \n"
    "struct FRAME {                  \n"
    "  int id;   string name;        \n"
    "  FRAME_MODULE aFrameModules[]; \n"
    "};                              \n"
    "FRAME aFrames[];                \n"
    "array<FRAME> aFrames2;          \n"
    "                                \n"
    "// typedef int ANIM_FRAME;      \n"
    "struct ANIMATION {              \n"
    "  int id;   int rate;           \n"
    "  string name;                  \n"
    "  int aAnimFrames;              \n"
    "};                              \n"
    "                                \n"
    "ANIMATION aAnimations[];        \n"
    "object obj;";

  DataPool* pDataPool = NULL;
  clstd::TimeTrace tt;
  clStringA strNum;
  strNum.AppendInteger32(HUGE_NUMBER, 3);

  // ����DataPool
  tt.Begin();
  HRESULT result = DataPool::CompileFromMemory(&pDataPool, NULL, NULL, s_szCode);
  ASSERT(GXSUCCEEDED(result));
  tt.End().Dump("Creation time:");

  ENUM_DATAPOOL(pDataPool);

  // ��ѯModules
  MOVariable varModules;
  tt.Begin();
  GXBOOL bval;
  bval = pDataPool->QueryByExpression("aModules", &varModules);
  tt.End().Dump("��ѯaModules����ʱ��:");
  ASSERT(bval);
  //ASSERT( ! varModules.IsFixed());
  ASSERT(TEST_FLAG(varModules.GetCaps(), MOVariable::CAPS_FIXED));
  ASSERT(varModules.GetLength() == 0);
  ASSERT(varModules.GetSize() == 0);

  MOVariable varTest1(varModules);

  // ���������鲢��ֵ
  clStringA str;
  clStringA strCheck; // ���� HUGE_NUMBER ����ƴ�����ʵ���֤�ַ���
  tt.Begin();
  MOVariable varFirstElement;     // �������Զ�̬�����������ݺ��һ����Ա��Ȼ��Ч
  MOVariable varNew = varModules.NewBack(HUGE_NUMBER);
  for(int i = 0; i < HUGE_NUMBER; i++)
  {
    varNew = varModules[i];
    //ASSERT( ! varNew.IsFixed());
    ASSERT(TEST_FLAG_NOT(varNew.GetCaps(), MOVariable::CAPS_FIXED));
    if(i == 0) {
      varFirstElement = varNew;
    }
    varNew["id"] = i + 1;
    str.Format("%d+%d", i, HUGE_NUMBER - i);
    varNew["name"] = str;
    varNew["rect"]["left"] = i + 2;
  }
  str.Format("��̬��������%s�����ݣ������γ�ʼ�����ֱ�������ʱ��:", strNum);
  tt.End().Dump(str);

  ASSERT(varModules.GetLength() == HUGE_NUMBER);
  ASSERT(varModules.GetSize() == HUGE_NUMBER * (sizeof(unsigned int) + sizeof(clStringW) + sizeof(int) * 4));

  strCheck.Format("%d+%d", 0, HUGE_NUMBER);
  ASSERT(varFirstElement["id"].ToInteger() == 1);
  ASSERT(varFirstElement["name"].ToStringA() == strCheck);
  ASSERT(varFirstElement["rect"]["left"].ToStringA() == "2"); // �����������ת�ַ���


  // ���Զ�̬��Ա����
  tt.Begin();
  for(int i = 0; i < HUGE_NUMBER; i++)
  {
    MOVariable varNew = varModules[i];
    str.Format("%d+%d", i, HUGE_NUMBER - i);

    ASSERT(varNew["id"].ToInteger() == i + 1);
    ASSERT(varNew["name"].ToStringA() == str);
    ASSERT(varNew["rect"]["left"].ToInteger() == i + 2);
  }
  str.Format("��������ȡ%s����������ʱ��", strNum);
  tt.End().Dump(str);

  // �������
  MOVariable var;
  strCheck.Format("%d+%d", 100, HUGE_NUMBER - 100);
  bval = pDataPool->QueryByExpression("aModules[100].name", &var);
  ASSERT(bval);
  ASSERT(var.ToStringA() == strCheck);

  // ����objectʹ��
  TestObject* pObj0 = new TestObject;
  TestObject* pObj1 = new TestObject;
  TestObject* pObj3 = NULL;
  pObj0->AddRef();
  pObj1->AddRef();
  MOVariable varObj;
  pDataPool->QueryByExpression("obj", &varObj);
  //ASSERT(varObj.IsFixed());
  ASSERT(TEST_FLAG(varObj.GetCaps(), MOVariable::CAPS_FIXED));
  varObj.Retain(pObj0);
  varObj.Retain(pObj1);
  varObj.Query((GUnknown**)&pObj3);
  ASSERT(pObj3 == pObj1);
  SAFE_RELEASE(pObj3);
  varObj.Retain(pObj0);
  SAFE_RELEASE(pObj0);
  SAFE_RELEASE(pObj1);

  GXLPCWSTR szFilename = L"test\\TestHugeArray.DTP";

  ENUM_DATAPOOL(pDataPool);
  GXBOOL bRet = pDataPool->SaveW(szFilename);
  ASSERT(bRet);

  DataPool* pDataPoolFromFile = NULL;
  DataPool::CreateFromFileW(&pDataPoolFromFile, NULL, szFilename, DataPoolLoad_ReadOnly);

  ENUM_DATAPOOL(pDataPoolFromFile);


  SAFE_RELEASE(pDataPool);
  SAFE_RELEASE(pDataPoolFromFile);
}


// ���԰����Լ�������
void TestSelfContainStruct()
{
  // �����ǲ����Ե�
  static char code0[] =
    "struct Tree \n"
    "{           \n"
    "  Tree n;   \n"
    "};          \n"
    "Tree test\n";

  // �����ǲ����Ե�
  static char code1[] =
    "struct A   \n"
    "{          \n"
    "  A a[10]; \n"
    "};         \n"
    "A test\n";

  // �����������
  static char code2[] =
    "struct Tree   \n"
    "{             \n"
    "  Tree n[];   \n"
    "};            \n"
    "Tree root";

  DataPool* pDataPool = NULL;
  HRESULT result;

  result = DataPool::CompileFromMemory(&pDataPool, NULL, NULL, code0);
  ASSERT(GXFAILED(result));

  result = DataPool::CompileFromMemory(&pDataPool, NULL, NULL, code1);
  ASSERT(GXFAILED(result));

  result = DataPool::CompileFromMemory(&pDataPool, NULL, NULL, code2);
  ENUM_DATAPOOL(pDataPool);
  ASSERT(GXSUCCEEDED(result));

  SAFE_RELEASE(pDataPool);
}

