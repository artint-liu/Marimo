#include <tchar.h>
#include <GrapX.H>
#include <GrapX/DataPool.h>
#include <GrapX/DataPoolVariable.h>
#include "TestObject.h"
#include "TestDataPool.h"

#define HUGE_NUMBER 10000

// 这里主要用来测试较大的数组使用情况
void TestHugeArray()
{
  static char* s_szCode =
    "#FILE "__FILE__"                \n"
    "#LINE 16                        \n"
    "/* 基础结构体                  */\n"
    "struct RECT                     \n"
    "{                               \n"
    "  int left;    int top;         \n"
    "  int right;   int bottom;      \n"
    "};                              \n"
    "struct POINT {                  \n"
    "  int x;   int y;               \n"
    "};                              \n"
    " /* 开始有用的了 */              \n"
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

  // 创建DataPool
  tt.Begin();
  HRESULT result = DataPool::CompileFromMemory(&pDataPool, NULL, NULL, s_szCode);
  ASSERT(GXSUCCEEDED(result));
  tt.End().Dump("Creation time:");

  ENUM_DATAPOOL(pDataPool);

  // 查询Modules
  MOVariable varModules;
  tt.Begin();
  GXBOOL bval;
  bval = pDataPool->QueryByExpression("aModules", &varModules);
  tt.End().Dump("查询aModules所用时间:");
  ASSERT(bval);
  //ASSERT( ! varModules.IsFixed());
  ASSERT(TEST_FLAG(varModules.GetCaps(), MOVariable::CAPS_FIXED));
  ASSERT(varModules.GetLength() == 0);
  ASSERT(varModules.GetSize() == 0);

  MOVariable varTest1(varModules);

  // 创建新数组并赋值
  clStringA str;
  clStringA strCheck; // 根据 HUGE_NUMBER 定义拼出合适的验证字符串
  tt.Begin();
  MOVariable varFirstElement;     // 用来测试动态数组新增数据后第一个成员仍然有效
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
  str.Format("动态数组增加%s条数据，并依次初始化部分变量所需时间:", strNum);
  tt.End().Dump(str);

  ASSERT(varModules.GetLength() == HUGE_NUMBER);
  ASSERT(varModules.GetSize() == HUGE_NUMBER * (sizeof(unsigned int) + sizeof(clStringW) + sizeof(int) * 4));

  strCheck.Format("%d+%d", 0, HUGE_NUMBER);
  ASSERT(varFirstElement["id"].ToInteger() == 1);
  ASSERT(varFirstElement["name"].ToStringA() == strCheck);
  ASSERT(varFirstElement["rect"]["left"].ToStringA() == "2"); // 这个用了数字转字符串


  // 测试动态成员访问
  tt.Begin();
  for(int i = 0; i < HUGE_NUMBER; i++)
  {
    MOVariable varNew = varModules[i];
    str.Format("%d+%d", i, HUGE_NUMBER - i);

    ASSERT(varNew["id"].ToInteger() == i + 1);
    ASSERT(varNew["name"].ToStringA() == str);
    ASSERT(varNew["rect"]["left"].ToInteger() == i + 2);
  }
  str.Format("从数组中取%s条数据所需时间", strNum);
  tt.End().Dump(str);

  // 随机访问
  MOVariable var;
  strCheck.Format("%d+%d", 100, HUGE_NUMBER - 100);
  bval = pDataPool->QueryByExpression("aModules[100].name", &var);
  ASSERT(bval);
  ASSERT(var.ToStringA() == strCheck);

  // 测试object使用
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


// 测试包含自己的数组
void TestSelfContainStruct()
{
  // 这种是不可以的
  static char code0[] =
    "struct Tree \n"
    "{           \n"
    "  Tree n;   \n"
    "};          \n"
    "Tree test\n";

  // 这种是不可以的
  static char code1[] =
    "struct A   \n"
    "{          \n"
    "  A a[10]; \n"
    "};         \n"
    "A test\n";

  // 这种是允许的
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

