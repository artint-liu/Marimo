#include <tchar.h>
#include <Marimo.H>
//#include <GrapX/DataInfrastructure.h>
#include "GrapX/DataPool.H"
#include "TestObject.h"
#include "TestDataPool.h"

// 主要用来测试基本属性是否正确

template<class _Ty>
void CheckUnary( DataPool* pDataPool, GXLPCSTR szExpression, GXBOOL bFixed ) 
{
  GXBOOL bval;
  MOVariable var;
  bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
  ASSERT(var.GetLength() == 1);
  ASSERT(var.GetSize() == sizeof(_Ty));
  //ASSERT(var.IsFixed() == bFixed);
  ASSERT(TEST_FLAG(var.GetCaps(), MOVariable::CAPS_FIXED) == bFixed);
  TRACE("%s=%s\n", szExpression, var.ToStringW());
}

template<class _Ty>
void CheckArray( DataPool* pDataPool, GXLPCSTR szExpression, GXBOOL bFixed, int nPurposeCount) 
{
  GXBOOL bval;
  MOVariable var;
  int len;
  int size;
  bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
  ASSERT((len = var.GetLength()) == nPurposeCount);
  ASSERT((size = var.GetSize()) == sizeof(_Ty) * nPurposeCount);
  //ASSERT(var.IsFixed() == bFixed);
  ASSERT(TEST_FLAG(var.GetCaps(), MOVariable::CAPS_FIXED) == bFixed);

  var = var[0];
  if(var.IsValid())
  {
    TRACE(__FUNCTION__" %s=%s\n", szExpression, var.ToStringW());
    ASSERT(var.GetLength() == 1);
    ASSERT(var.GetSize() == sizeof(_Ty));
  }
}

template<class _Ty>
void CheckElement(DataPool* pDataPool, GXLPCSTR szExpression, GXBOOL bFixed, int nIndex ) 
{
  GXBOOL bval;
  MOVariable var;
  MOVariable var0;
  MOVariable var1;

  bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);

  clStringA strElement;
  strElement.Format("%s[%d]", szExpression, nIndex);
  bval = pDataPool->QueryByExpression(strElement, &var0);
  ASSERT(bval);
  ASSERT(var0.GetLength() == 1);
  ASSERT(var0.GetSize() == sizeof(_Ty));
  //ASSERT(var0.IsFixed() == bFixed);
  ASSERT(TEST_FLAG(var0.GetCaps(), MOVariable::CAPS_FIXED) == bFixed);
  var1 = var[nIndex];
  ASSERT(var0.GetPtr() == var1.GetPtr());
}

void TestBaseProperty()
{
  TestObject* pObject;
  TestObject::Create(&pObject);

  static float4x4 matIdentity[4] = {float4x4::Identity, float4x4::Identity, float4x4::Identity, float4x4::Identity};
  static TestObject* aManyObject[12] = {pObject, pObject, pObject, pObject, pObject, pObject, pObject, pObject, pObject, pObject, pObject, pObject};

  // 因为引用了12次
  for(int i = 0; i < sizeof(aManyObject) / sizeof(aManyObject[0]); i++) {
    pObject->AddRef();
  }

  static DATAPOOL_VARIABLE_DECLARATION g_StdMtl[] = 
  {
    {"float",     "fFactor",          0,   },
    {"float",     "aFactor",          0,   3},
    {"float",     "aDynFactor",       0,  -7, &matIdentity},

    {"float4x4",  "matViewProj",      0,   0, &matIdentity},
    {"float4x4",  "testDynArray",     0,  -2, &matIdentity},
    {"float4x4",  "testArray",        0,  10, &matIdentity},

    {"string",    "strName",          0,  },
    {"string",    "aNames",           0,  5},
    {"string",    "aManyNames",       0,  -2, L"hello world\0abcabc\0"},

    {"object",    "sObject",          0,  },
    {"object",    "aObjects",         0,  7},
    {"object",    "aManyObjects",     0,  -12, &aManyObject},

    {"vertex",    "VarMember",        0,      },

    {NULL, NULL}
  };

  static DATAPOOL_VARIABLE_DECLARATION c_vertex[] = {
    {"string", "name"},
    {"float3", "pos"},
    {"float3", "vertices", 0, -1},
    {NULL, NULL},
  };

  static DATAPOOL_TYPE_DECLARATION c_InternalTypeDefine[] = {
    {DataPoolTypeClass::Structure, "vertex", c_vertex},
    {DataPoolTypeClass::Undefine, NULL},
  };


  DataPool* pDataPool = NULL;
  DataPool::CreateDataPool(&pDataPool, NULL, c_InternalTypeDefine, g_StdMtl);

  MOVariable var;

  CheckUnary<float>(pDataPool, "fFactor", TRUE);
  CheckArray<float>(pDataPool, "aFactor", TRUE, 3);
  CheckArray<float>(pDataPool, "aDynFactor", TRUE, 7);

  // 测试两种方式得到的数组元素为同一个
  CheckElement<float>(pDataPool, "aFactor", TRUE, 0);
  CheckElement<float>(pDataPool, "aFactor", TRUE, 1);


  CheckUnary<float4x4>(pDataPool, "matViewProj", TRUE);
  CheckArray<float4x4>(pDataPool, "testDynArray", TRUE, 2);

  CheckElement<float4x4>(pDataPool, "testDynArray", FALSE, 0);
  CheckElement<float4x4>(pDataPool, "testDynArray", FALSE, 1);


  CheckArray<float4x4>(pDataPool, "testArray", TRUE, 10);
  CheckElement<float4x4>(pDataPool, "testArray", TRUE, 0);
  CheckElement<float4x4>(pDataPool, "testArray", TRUE, 1);



  CheckUnary<clStringW>(pDataPool, "strName", TRUE);
  CheckArray<clStringW>(pDataPool, "aNames", TRUE, 5);
  CheckArray<clStringW>(pDataPool, "aManyNames", TRUE, 2);
  CheckElement<clStringW>(pDataPool, "aNames", TRUE, 1);
  CheckElement<clStringW>(pDataPool, "aManyNames", FALSE, 1);
  
  CheckUnary<GUnknown*>(pDataPool, "sObject", TRUE);
  CheckArray<GUnknown*>(pDataPool, "aObjects", TRUE, 7);
  CheckArray<GUnknown*>(pDataPool, "aManyObjects", TRUE, 12);
  CheckElement<GUnknown*>(pDataPool, "aObjects", TRUE, 1);
  CheckElement<GUnknown*>(pDataPool, "aObjects", TRUE, 3);
  CheckElement<GUnknown*>(pDataPool, "aManyObjects", FALSE, 1);
  CheckElement<GUnknown*>(pDataPool, "aManyObjects", FALSE, 5);

  CheckUnary<clStringW>(pDataPool, "VarMember.name", TRUE);
  CheckArray<float3>(pDataPool, "VarMember.vertices", TRUE, 0);
  CheckUnary<float>(pDataPool, "VarMember.pos.x", TRUE);


  MOVariable varArray;
  MOVariable varArray0;

  pDataPool->QueryByExpression("VarMember.vertices", &varArray);
  varArray0 = varArray.NewBack();

  ENUM_DATAPOOL(pDataPool);
  SAFE_RELEASE(pObject);
  SAFE_RELEASE(pDataPool);
}