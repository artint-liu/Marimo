// TestDataPool.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
//#include <GrapX.H>
#include <tchar.h>
#include <Marimo.H>
//#include <include/DataInfrastructure.h>
//#include <clstd/Smart/smartstream.h>
#include "GrapX/DataPool.H"
#include <clTextLines.h>
#include <clString.H>
#include "clPathFile.h"
#include <vld.h>

#include "TestDataPool.h"

GXLPCWSTR s_szExampleString[] = {
  _CLTEXT("Introduction"),
  _CLTEXT("Devices"),
  _CLTEXT("Resources"),
  _CLTEXT("Graphics Pipeline"),
  _CLTEXT("Rendering"),
  _CLTEXT("Migrating to Direct3D 11"),
  _CLTEXT("Writing HLSL Shaders in Direct3D 9"),
  _CLTEXT("Using Shaders in Direct3D 9"),
  _CLTEXT("Using Shaders in Direct3D 10"),
  _CLTEXT("Optimizing HLSL Shaders"),
  _CLTEXT("Debugging Shaders in Visual Studio (Direct3D 9)"),
  _CLTEXT("Compiling Shaders"),
  _CLTEXT("Unpacking and Packing DXGI_FORMAT for In-Place Image Editing"),
  NULL,
};

using namespace Marimo;

void TestSelfContainStruct();
void TestHugeArray();
void TestBaseProperty();
void TestCompileFromFile();
void TestComplexArray();
void TestHashBuckets();

GXUINT_PTR GetFirstElementPtr(const DataPoolVariable& var) // 获得数组对象第一个元素的指针地址
{
  ASSERT(TEST_FLAG(var.GetCaps(), DataPoolVariable::CAPS_DYNARRAY));
  return (GXUINT_PTR)(*(clBufferBase**)var.GetPtr())->GetPtr();
}

void TestDynamicStringArray(DataPool* pDataPool)
{
  GXBOOL bval;
  clStringW strTest = "[empty]";
  MOVariable varTestStringVarArray;
  MOVariable varTestStringVarArray0;
  MOVariable varTestStringVarArray1;
  MOVariable varTestStringVarArray2;
  MOVariable varTestStringVarArray3;

  bval = pDataPool->QueryByExpression("TestStringVarArray", &varTestStringVarArray);
  ASSERT(bval);
  ASSERT(varTestStringVarArray.GetLength() == 1);
  ASSERT(varTestStringVarArray.GetSize() == sizeof(clStringW) * 1);

  varTestStringVarArray.NewBack(3);
  ASSERT(varTestStringVarArray.GetLength() == 4);
  ASSERT(varTestStringVarArray.GetSize() == sizeof(clStringW) * 4);

  bval = pDataPool->QueryByExpression("TestStringVarArray[0]", &varTestStringVarArray0);
  ASSERT(bval && varTestStringVarArray0.ToStringA() == "Hello world");

  varTestStringVarArray0.Set(_CLTEXT("TestVarString"));
  ASSERT(varTestStringVarArray0.ToStringW() == _CLTEXT("TestVarString"));
  //hval = pDataPool->SetVariable("TestStringVarArray[0]", &clStringW());
  //hval = pDataPool->GetVariable("TestStringVarArray[0]", &strTest);

  //hval = pDataPool->SetVariable("TestStringVarArray[1]", &clStringW(L"AppendString"));
  //hval = pDataPool->SetVariable("TestStringVarArray[3]", &clStringW(L"AppendStr[3]"));
  bval = pDataPool->QueryByExpression("TestStringVarArray[1]", &varTestStringVarArray1);
  ASSERT(bval);
  varTestStringVarArray1.Set(_CLTEXT("AppendString"));

  bval = pDataPool->QueryByExpression("TestStringVarArray[3]", &varTestStringVarArray3);
  ASSERT(bval);
  varTestStringVarArray3.Set(_CLTEXT("AppendStr[3]"));

  varTestStringVarArray0.Free();
  varTestStringVarArray1.Free();
  varTestStringVarArray2.Free();
  varTestStringVarArray3.Free();


  bval = pDataPool->QueryByExpression("TestStringVarArray[1]", &varTestStringVarArray1);
  ASSERT(bval && varTestStringVarArray1.ToStringW() == _CLTEXT("AppendString"));

  bval = pDataPool->QueryByExpression("TestStringVarArray[2]", &varTestStringVarArray2);
  ASSERT(bval && varTestStringVarArray2.ToStringW() == _CLTEXT(""));

  bval = pDataPool->QueryByExpression("TestStringVarArray[3]", &varTestStringVarArray3);
  ASSERT(bval && varTestStringVarArray3.ToStringW() == _CLTEXT("AppendStr[3]"));

  ASSERT(varTestStringVarArray.GetSize() == 4 * sizeof(clStringW));
  ASSERT(varTestStringVarArray0.GetSize() == sizeof(clStringW));
  //int nSize;
  //nSize = pDataPool->SizeOf("TestStringVarArray");
  //nSize = pDataPool->SizeOf("TestStringVarArray[0]");
}

void TestSizeOf(DataPool* pDataPool)
{
  //GXINT nSize;
  MOVariable varLightDiffuse;
  MOVariable varLightAmbient;
  MOVariable varLightDiffuse_x;
  MOVariable varLightAmbient_x;
  GXBOOL bval;

  bval = pDataPool->QueryByExpression("g_vLightDiffuse", &varLightDiffuse);
  ASSERT(bval && varLightDiffuse.GetSize() == 12);
  
  bval = pDataPool->QueryByExpression("g_vLightAmbient", &varLightAmbient);
  ASSERT(bval && varLightAmbient.GetSize() == 12);
  
  bval = pDataPool->QueryByExpression("g_vLightDiffuse.x", &varLightDiffuse_x);
  ASSERT(bval && varLightDiffuse_x.GetSize() == 4);

  bval = pDataPool->QueryByExpression("g_vLightAmbient.x", &varLightAmbient_x);
  ASSERT(bval && varLightAmbient_x.GetSize() == 4);
  //__asm nop
}

GXBOOL IsDynamicArray(DataPool* pDataPool, GXLPCSTR szExpression)
{
  MOVariable var;
  GXBOOL bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
  //return ! var.IsFixed();
  return TEST_FLAG_NOT(var.GetCaps(), MOVariable::CAPS_FIXED);
}

void TestDynamicArrayTest(DataPool* pDataPool)
{
  GXBOOL bDynamic;
  bDynamic = IsDynamicArray(pDataPool, "g_matViewProj");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "testDynArray");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "testArray");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_matViewProjInv");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_matView");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_matViewInv");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "VarMember");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "testStringList");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_vViewPos");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "TestString");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "TestStringArray");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "TestStringVarArray");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_vLightDiffuse");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "g_vLightAmbient");
  ASSERT( ! bDynamic);
  bDynamic = IsDynamicArray(pDataPool, "testArray2");
  ASSERT( ! bDynamic);
}

void TestDataPoolAgent(DataPool* pDataPool)
{
  MOVariable sString;
  pDataPool->QueryByName("TestString", &sString);
  sString = "SetFromAgent";

  sString.Free();

  //pDataPool->GetVariable("TestString", &str);
  pDataPool->QueryByName("TestString", &sString);
  ASSERT(sString.ToStringW() == _CLTEXT("SetFromAgent"));

  MOVariable dpfloat;
  clStringW str;
  pDataPool->QueryByName("fFactor", &dpfloat);
  str = dpfloat.ToStringW();
  dpfloat = 1.23f;
  str = dpfloat.ToStringW();
  ASSERT(str == "1.23f");


  //SAFE_RELEASE(pString);
}

void TestDataPoolAgent_Primary_StaticArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  VAR VarFloatArray;
  VAR VarFloatArrayEle[20];
  //InlSetZeroT(pVarFloatArrayEle);

  TRACE("=== %s ===\n", __FUNCTION__);
  hval = pDataPool->QueryByName("testArray", &VarFloatArray);
  GXUINT_PTR lpBasePtr = (GXUINT_PTR)VarFloatArray.GetPtr();

  ASSERT(GXSUCCEEDED(hval));
  for(int i = 0; i < 20; i++)
  {
    VarFloatArrayEle[i] = VarFloatArray.IndexOf(i);
    TRACE("bval = %s.\n", VarFloatArrayEle[i].IsValid() ? "TRUE" : "FALSE");
    if(VarFloatArrayEle[i].IsValid())
    {
      ASSERT(lpBasePtr + sizeof(float) * i == (GXUINT_PTR)VarFloatArrayEle[i].GetPtr());
      VarFloatArrayEle[i].Set((float)i + 100.0f);
    }
  }
  for(int i = 0; i < 20; i++)
  {
    if(VarFloatArrayEle[i].IsValid())
    {
      ASSERT(VarFloatArrayEle[i].ToFloat() == (float)i + 100.0f);
      TRACE("pVarFloatArrayEle[%d]=%f\n", i, VarFloatArrayEle[i].ToFloat());
    }
    //SAFE_RELEASE(pVarFloatArrayEle[i]);
  }
  //SAFE_RELEASE(pVarFloatArray);
}

void TestDataPoolAgent_String_StaticArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 5;
  VAR Var;
  VAR VarString[nArray];
  //InlSetZeroT(pVarString);
  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("TestStringArray", &Var);
  GXUINT_PTR lpBasePtr = (GXUINT_PTR)Var.GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarString[i] = Var.IndexOf(i);
    TRACE("bval = %s.\n", VarString[i].IsValid() ? "TRUE" : "FALSE");
    if(VarString[i].IsValid())
    {
      ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)VarString[i].GetPtr());
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
      VarString[i].Set(s_szExampleString[i]);
    }
    ASSERT((i < 3 && VarString[i].IsValid()) || (! VarString[i].IsValid() && i >= 3));
  }

  for(int i = 0; i < nArray; i++)
  {
    if(VarString[i].IsValid())
    {
      ASSERT(VarString[i].ToStringA() == s_szExampleString[i]);
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
    }
    //SAFE_RELEASE(pVarString[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_Struct_StaticArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 10;
  const int nAvail = 6;
  GXLPCSTR szMemberName[] = {"x","y","z"};
  VAR Var;
  VAR VarVector3[nArray];
  VAR VarVector3Member[nArray][3];
  //InlSetZeroT(pVarVector3);
  //InlSetZeroT(pVarVector3Member);
  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("TestVector3", &Var);
  GXUINT_PTR lpBasePtr = (GXUINT_PTR)Var.GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarVector3[i] = Var.IndexOf(i);
    GXBOOL bval = VarVector3[i].IsValid();
    TRACE("bval = %s.\n", bval ? "TRUE" : "FALSE");
    
    // 返回值检测
    ASSERT((i < nAvail && bval) || (! bval && i >= nAvail));
    if(bval)
    {
      // 数据地址范围检测
      //ASSERT(lpBasePtr <= (GXUINT_PTR)VarVector3[i].GetPtr() && 
      //  (GXUINT_PTR)VarVector3[i].GetPtr() == (GXUINT_PTR)((GXBYTE*)lpBasePtr + sizeof(float3) * nAvail));
      ASSERT(VarVector3[i].GetPtr() == (GXBYTE*)lpBasePtr + sizeof(float3) * i);

      for(int n = 0; n < 3; n++)
      {
        VarVector3Member[i][n] = VarVector3[i].MemberOf(szMemberName[n]);
        bval = VarVector3Member[i][n].IsValid();
        VarVector3Member[i][n].Set((float)i * 5.0f + (float)n);

        // 数据地址范围检测
        //ASSERT(lpBasePtr <= (GXUINT_PTR)VarVector3Member[i][n].GetPtr() && 
        //  (GXUINT_PTR)VarVector3Member[i][n].GetPtr() == (GXUINT_PTR)((GXBYTE*)lpBasePtr + sizeof(float3) * nAvail));
        ASSERT(bval);
        ASSERT(VarVector3Member[i][n].GetPtr() == (GXBYTE*)lpBasePtr + sizeof(float3) * i + sizeof(float) * n);
      }
    }
  }

  for(int i = 0; i < nArray; i++)
  {
    if(VarVector3[i].IsValid())
    {
      TRACE("pVarVector3[%d]=", i);
      for(int n = 0; n < 3; n++)
      {
        ASSERT(VarVector3Member[i][n].ToFloat() == (float)i * 5.0f + (float)n)
        TRACE("%f,", VarVector3Member[i][n].ToFloat());
        //SAFE_RELEASE(pVarVector3Member[i][n]);
      }
      TRACE("\n");
    }
    //SAFE_RELEASE(pVarVector3[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_String_DynArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 5;
  VAR Var;
  VAR VarString[nArray];
  //InlSetZeroT(pVarString);
  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("TestStringVarArray", &Var);
  //clBuffer** lplpBuffer = (clBuffer**)Var.GetPtr();
  //GXUINT_PTR lpBasePtr = (GXUINT_PTR)(*lplpBuffer)->GetPtr();
  GXUINT_PTR lpBasePtr = GetFirstElementPtr(Var); //GXUINT_PTR)(*(clBufferBase**)Var.GetPtr())->GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarString[i] = Var.IndexOf(i);
    GXBOOL bval = VarString[i].IsValid();
    TRACE("bval = %s.\t", bval ? "TRUE" : "FALSE");
    if(bval)
    {
      ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)VarString[i].GetPtr());
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
      VarString[i].Set(s_szExampleString[i]);
    }
    else { TRACE("\n"); }
    ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }

  for(int i = 0; i < nArray; i++)
  {
    if(VarString[i].IsValid())
    {
      ASSERT(VarString[i].ToStringA() == s_szExampleString[i]);
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
    }
    //SAFE_RELEASE(pVarString[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_String_DynArray_NewBack(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 10;
  VAR Var;
  VAR VarString[nArray];
  //InlSetZeroT(pVarString);
  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("TestStringVarArray", &Var);
  GXUINT_PTR lpBasePtr = GetFirstElementPtr(Var);//(GXUINT_PTR)(*(clBufferBase**)Var.GetPtr())->GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarString[i] = Var.IndexOf(i);
    GXBOOL bval = VarString[i].IsValid();
    TRACE("bval = %s.\t", bval ? "TRUE" : "FALSE");
    if(bval)
    {
      ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)VarString[i].GetPtr());
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
    }
    else{
      VarString[i] = Var.NewBack();
      TRACE("\n");
      ASSERT(VarString[i].IsValid());
    }
    VarString[i].Set(s_szExampleString[i]);
    ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }

  for(int i = 0; i < nArray; i++)
  {
    if(VarString[i].IsValid())
    {
      ASSERT(VarString[i].ToStringA() == s_szExampleString[i]);
      TRACE("pVarString[%d]=%s\n", i, VarString[i].ToStringA());
    }
    //SAFE_RELEASE(pVarString[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_Struct_DynArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 5;
  VAR Var;
  VAR VarFloat3[nArray];
  VAR VarFloat3Member[nArray][3];
  GXLPCSTR szMemberName[] = {"x","y","z"};
  //InlSetZeroT(pVarFloat3);
  //InlSetZeroT(pVarFloat3Member);

  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("testArray2", &Var);
  GXUINT_PTR lpBasePtr = (GXUINT_PTR)Var.GetPtr();

  //clBuffer** lplpBuffer = (clBuffer**)Var.GetPtr();
  //GXUINT_PTR lpBasePtr = (GXUINT_PTR)(*lplpBuffer)->GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarFloat3[i] = Var.IndexOf(i);
    GXBOOL bval = VarFloat3[i].IsValid();
    TRACE("bval[%d] = %s.\t", i, bval ? "TRUE" : "FALSE");
    if(bval)
    {
      //ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)pVarString[i]->GetPtr());
      //TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
      //pVarString[i]->SetAsStringW(s_szExampleString[i]);
      ASSERT(VarFloat3[i].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i);
      for(int n = 0; n < 3; n++)
      {
        VarFloat3Member[i][n] = VarFloat3[i].MemberOf(szMemberName[n]);
        bval = VarFloat3Member[i][n].IsValid();
        if(bval)
        {
          TRACE("%f,", VarFloat3Member[i][n].ToFloat());
          ASSERT(VarFloat3Member[i][n].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i + sizeof(float) * n);
        }
      }
      TRACE("\n");
    }
    else { TRACE("\n"); }
    //ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }

  for(int i = 0; i < nArray; i++)
  {
    for(int n = 0; n < 3; n++)
    {
      //SAFE_RELEASE(pVarFloat3Member[i][n]);
    }
    //if(pVarString[i] != NULL)
    //{
    //  TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
    //}
    //SAFE_RELEASE(pVarFloat3[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_Struct_DynArray_NewBack(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 5;
  VAR Var;
  VAR VarFloat3[nArray];
  VAR VarFloat3Member[nArray][3];
  GXLPCSTR szMemberName[] = {"x","y","z"};
  //InlSetZeroT(pVarFloat3);
  //InlSetZeroT(pVarFloat3Member);

  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("testArray2", &Var);
  GXUINT_PTR lpBasePtr = GetFirstElementPtr(Var);//(GXUINT_PTR)(*(clBufferBase**)Var.GetPtr())->GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarFloat3[i] = Var.IndexOf(i);
    GXBOOL bval = VarFloat3[i].IsValid();
    TRACE("bval[%d] = %s.\t", i, bval ? "TRUE" : "FALSE");
    if(bval)
    {
      ASSERT(VarFloat3[i].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i);
      //ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)pVarString[i]->GetPtr());
      //TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
      //pVarString[i]->SetAsStringW(s_szExampleString[i]);
      for(int n = 0; n < 3; n++)
      {
        VarFloat3Member[i][n] = VarFloat3[i].MemberOf(szMemberName[n]);
        bval = VarFloat3Member[i][n].IsValid();
        if(bval)
        {
          TRACE("%f,", VarFloat3Member[i][n].ToFloat());
          ASSERT(VarFloat3Member[i][n].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i + sizeof(float) * n);
        }
      }
    }
    else { 
      VarFloat3[i] = Var.NewBack();
      //lpBasePtr = (GXUINT_PTR)Var.GetPtr(); 
      lpBasePtr = GetFirstElementPtr(Var); // NewBack 后地址会改变
      //(GXUINT_PTR)(*(clBufferBase**)Var.GetPtr())->GetPtr();

      ASSERT(VarFloat3[i].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i);
      for(int n = 0; n < 3; n++)
      {
        VarFloat3Member[i][n] = VarFloat3[i].MemberOf(szMemberName[n]);
        bval = VarFloat3Member[i][n].IsValid();
        VarFloat3Member[i][n].Set((float)(i * 4 + n));
        ASSERT(VarFloat3Member[i][n].GetPtr() == (GXLPBYTE)lpBasePtr + sizeof(float3) * i + sizeof(float) * n);
      }
    }
    TRACE("\n");
    //ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }

  for(int i = 0; i < nArray; i++)
  {
    for(int n = 0; n < 3; n++)
    {
      ASSERT(VarFloat3Member[i][n].ToFloat() == (float)(i * 4 + n));
      TRACE("%f, ", VarFloat3Member[i][n].ToFloat());
      //SAFE_RELEASE(pVarFloat3Member[i][n]);
    }
    TRACE("\n");
    //if(pVarString[i] != NULL)
    //{
    //  TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
    //}
    //SAFE_RELEASE(pVarFloat3[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent_Primary_DynArray(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 10;
  VAR Var;
  VAR VarElement[nArray];
  //GXLPCSTR szMemberName[] = {"x","y","z"};
  InlSetZeroT(VarElement);
  //InlSetZeroT(pVarFloat3Member);

  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("testDynArray", &Var);
  GXUINT_PTR lpBasePtr = GetFirstElementPtr(Var);//(GXUINT_PTR)Var.GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarElement[i] = Var.IndexOf(i);
    GXBOOL bval = VarElement[i].IsValid();
    TRACE("bval = %s.\t", bval ? "TRUE" : "FALSE");
    if(bval)
    {
      ASSERT(lpBasePtr + sizeof(float) * i == (GXUINT_PTR)VarElement[i].GetPtr());
      //TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
      //pVarString[i]->SetAsStringW(s_szExampleString[i]);
      //for(int n = 0; n < 3; n++)
      //{
      //  bval = pVarFloat3[i]->MemberOf(szMemberName[n], &pVarFloat3Member[i][n]);
      //  if(bval)
      //  {
      //    TRACE("%f,", pVarFloat3Member[i][n]->ToFloat());
      //  }
      //}
      TRACE("testDynArray[%d]=%f\n", i, VarElement[i].ToFloat());
    }
    else { TRACE("\n"); }
    //ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }
}

void TestDataPoolAgent_Primary_DynArray_NewBack(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;
  const int nArray = 7;
  VAR Var;
  VAR VarElement[nArray];
  float aNativeStore[nArray];

  TRACE("=== %s ===\n", __FUNCTION__);

  hval = pDataPool->QueryByName("testDynArray", &Var);
  //clBuffer** lplpBuffer = (clBuffer**)Var.GetPtr();
  GXUINT_PTR lpBasePtr = GetFirstElementPtr(Var);//(GXUINT_PTR)Var.GetPtr();

  for(int i = 0; i < nArray; i++)
  {
    VarElement[i] = Var.IndexOf(i);
    GXBOOL bval = VarElement[i].IsValid();
    TRACE("bval = %s.\t", bval ? "TRUE" : "FALSE");
    if(bval)
    {
      ASSERT(lpBasePtr + sizeof(clStringW) * i == (GXUINT_PTR)VarElement[i].GetPtr());
      //TRACE("pVarString[%d]=%s\n", i, pVarString[i]->ToStringA());
      //pVarString[i]->SetAsStringW(s_szExampleString[i]);
      //for(int n = 0; n < 3; n++)
      //{
      //  bval = pVarFloat3[i]->MemberOf(szMemberName[n], &pVarFloat3Member[i][n]);
      //  if(bval)
      //  {
      //    TRACE("%f,", pVarFloat3Member[i][n]->ToFloat());
      //  }
      //}
      TRACE("testDynArray[%d]=%f\n", i, VarElement[i].ToFloat());
      aNativeStore[i] = VarElement[i].ToFloat();
    }
    else {
      VarElement[i] = Var.NewBack();
      lpBasePtr = GetFirstElementPtr(Var);//(GXUINT_PTR)Var.GetPtr();
      TRACE("%s %s\n", VarElement[i].GetTypeName(), VarElement[i].GetName());
      ASSERT(lpBasePtr + sizeof(float) * i == (GXUINT_PTR)VarElement[i].GetPtr());

      VarElement[i].Set((float)i * 2.0f);
      aNativeStore[i] = (float)i * 2.0f;
      TRACE("\n"); 
    }
    //ASSERT((i < 4 && bval) || (! bval && i >= 4));
  }

  for(int i = 0; i < nArray; i++)
  {
    //for(int n = 0; n < 3; n++)
    //{
    //  SAFE_RELEASE(pVarFloat3Member[i][n]);
    //}
    if(VarElement[i].IsValid())
    {
      ASSERT(VarElement[i].ToFloat() == aNativeStore[i]);
      TRACE("testDynArray[%d]=%f\n", i, VarElement[i].ToFloat());
    }
    //SAFE_RELEASE(pVarElement[i]);
  }
  //SAFE_RELEASE(pVar);
}

void TestDataPoolAgent2(DataPool* pDataPool)
{
  GXHRESULT hval;
  typedef Implement::Variable VAR;

  VAR Var;

  hval = pDataPool->QueryByName("TestString", &Var);
  ASSERT(GXSUCCEEDED(hval));

  clStringW strTest = Var.ToStringW();
  Var.Set(_CLTEXT("SetFromAgent"));

  // 测试浮点数组
  TestDataPoolAgent_Primary_StaticArray(pDataPool);
  TestDataPoolAgent_String_StaticArray(pDataPool);
  TestDataPoolAgent_Struct_StaticArray(pDataPool);

  TestDataPoolAgent_String_DynArray(pDataPool);
  TestDataPoolAgent_String_DynArray_NewBack(pDataPool);
  TestDataPoolAgent_Struct_DynArray(pDataPool);
  TestDataPoolAgent_Struct_DynArray_NewBack(pDataPool);

  TestDataPoolAgent_Primary_DynArray(pDataPool);
  TestDataPoolAgent_Primary_DynArray_NewBack(pDataPool);
  TestDataPoolAgent_Primary_DynArray(pDataPool);

  //SAFE_RELEASE(pVar);
  //__asm nop
}

GXSIZE_T SizeOf(DataPool* pDataPool, GXLPCSTR szExpression)
{
  MOVariable var;
  GXBOOL bval = pDataPool->QueryByExpression(szExpression, &var);
  ASSERT(bval);
  return var.GetSize();
}

void test()
{
  //VARIABLE_DECLARATION c_float2[] = {
  //  {"float", "x"},
  //  {"float", "y"},
  //  {NULL, NULL},
  //};

  //VARIABLE_DECLARATION c_float3[] = {
  //  {"float", "x"},
  //  {"float", "y"},
  //  {"float", "z"},
  //  {NULL, NULL},
  //};

  //VARIABLE_DECLARATION c_float4[] = {
  //  {"float", "x"},
  //  {"float", "y"},
  //  {"float", "z"},
  //  {"float", "w"},
  //  {NULL, NULL},
  //};

  //VARIABLE_DECLARATION c_float3x3[] = {
  //  {"float", "m", 9},
  //  {NULL, NULL},
  //};

  //VARIABLE_DECLARATION c_float4x4[] = {
  //  {"float", "m", 16},
  //  {NULL, NULL},
  //};

  DATAPOOL_VARIABLE_DECLARATION c_vertex[] = {
    {"string", "name"},
    {"float3", "vertex", 0, -1},
    {NULL, NULL},
  };

  DATAPOOL_TYPE_DECLARATION c_InternalTypeDefine[] = {
    //{ScriptedDataPool::T_DWORD,  "float"},
    //{ScriptedDataPool::T_STRING, "string"},
    //{ScriptedDataPool::T_STRUCT, "float2"  , c_float2},
    //{ScriptedDataPool::T_STRUCT, "float3"  , c_float3},
    //{ScriptedDataPool::T_STRUCT, "float4"  , c_float4},
    //{ScriptedDataPool::T_STRUCT, "float3x3", c_float3x3},
    //{ScriptedDataPool::T_STRUCT, "float4x4", c_float4x4},
    {Marimo::T_STRUCT, "vertex", c_vertex},
    {T_UNDEFINE, NULL},
  };

  float4x4 matIdentity[4] = {float4x4::Identity, float4x4::Identity, float4x4::Identity, float4x4::Identity};

  DATAPOOL_VARIABLE_DECLARATION g_StdMtl[] = 
  {
    {"float",    "fFactor",           0,   },
    {"float4x4", "g_matViewProj",     0,   0, &matIdentity},
    {"float",    "testDynArray",      0,  -1, &matIdentity},
    {"float",    "testArray",         0,  10, &matIdentity},
    {"float4x4", "g_matViewProjInv",  0,      },
    {"float4x4", "g_matView",         0,   4  },
    {"float4x4", "g_matViewInv",      0,  -4, &matIdentity},
    {"vertex",   "VarMember",         0,      },
    {"string",   "testStringList",    0,   2  },
    {"float3",   "g_vViewPos",        0,      },
    {"float3",   "TestVector3",       0,   6  },
    {"string",   "TestString",        0,   1, L"Hello world"},
    {"string",   "TestStringArray",   0,   3, L"Hello world\0sample01\0sample02"},
    {"string",   "TestStringVarArray",0,  -1, L"Hello world"},
    {"float3",   "g_vLightDiffuse",   0,      },
    {"float3",   "g_vLightAmbient",   0,      },
    {"float3",   "testArray2",        0,  -1  },
    {NULL, NULL}
  };


  DataPool* pDataPool = NULL;
  DataPool::CreateDataPool(&pDataPool, NULL, c_InternalTypeDefine, g_StdMtl);
  
  GXBOOL bval;

  MOVariable var;

  bval = pDataPool->QueryByExpression("testArray.klm", &var); // 这个不应该有成员
  ASSERT( ! bval);

  bval = pDataPool->QueryByExpression("g_matViewProj", &var);
  ASSERT(bval);
  ASSERT(var.CastTo<float4x4>() == matIdentity[0]);

  bval = pDataPool->QueryByExpression("g_matView[0]", &var);
  ASSERT(bval);


  GXSIZE_T nSize;
  nSize = SizeOf(pDataPool, "g_matViewInv");
  ASSERT(nSize == sizeof(float4x4) * 4);

  nSize = SizeOf(pDataPool, "g_matViewInv[0]");
  ASSERT(nSize == sizeof(float4x4));



  nSize = SizeOf(pDataPool, "g_matView");
  ASSERT(nSize == sizeof(float4x4) * 4);
  
  nSize = SizeOf(pDataPool, "g_matView[0]");
  ASSERT(nSize == sizeof(float4x4));

  nSize = SizeOf(pDataPool, "g_matView[1]");
  ASSERT(nSize == sizeof(float4x4));

  float4x4 matTest;
  matTest.Scale(2.0f);
  bval = pDataPool->QueryByExpression("g_matView[1]", &var);
  ASSERT(bval);
  var.CastTo<float4x4>() = matTest;
  
  matTest.Scale(3.0f);
  bval = pDataPool->QueryByExpression("g_matView[2]", &var);
  ASSERT(bval);
  var.CastTo<float4x4>() = matTest;

  matTest.Scale(4.0f);
  bval = pDataPool->QueryByExpression("g_matView[3]", &var);
  ASSERT(bval);
  var.CastTo<float4x4>() = matTest;

  matTest.Scale(5.0f);
  bval = pDataPool->QueryByExpression("g_matView[4]", &var);  // 越界访问
  ASSERT( ! bval);

  var.Free();


  float4x4 matCheck;

  bval = pDataPool->QueryByExpression("g_matView[1]", &var);
  ASSERT(bval);
  matCheck.Scale(2.0f);
  ASSERT(matCheck == var.CastTo<float4x4>());

  bval = pDataPool->QueryByExpression("g_matView[2]", &var);
  ASSERT(bval);
  matCheck.Scale(3.0f);
  ASSERT(matCheck == var.CastTo<float4x4>());

  bval = pDataPool->QueryByExpression("g_matView[3]", &var);
  ASSERT(bval);
  matCheck.Scale(4.0f);
  ASSERT(matCheck == var.CastTo<float4x4>());

  bval = pDataPool->QueryByExpression("g_matView[4]", &var);  // 越界访问
  ASSERT( ! bval);
/*
  float f11, f22, f33, f12;
  hval = pDataPool->GetVariable("g_matView[2].m[1]", &f12);  // _12
  hval = pDataPool->GetVariable("g_matView[2].m[0]", &f11);  // _11
  hval = pDataPool->GetVariable("g_matView[2].m[5]", &f22);  // _22
  hval = pDataPool->GetVariable("g_matView[2].m[10]", &f33); // _33
  ASSERT(f12 == 0);
  ASSERT(f11 == 3.0f);
  ASSERT(f22 == 3.0f);
  ASSERT(f33 == 3.0f);

  clStringW strTest;
  hval = pDataPool->GetVariable("TestString", &strTest);
  ASSERT(GXSUCCEEDED(hval));
  int nTestString = pDataPool->SizeOf("TestString");
  ASSERT(nTestString == sizeof(clStringW) && nTestString == sizeof(void*));

  hval = pDataPool->GetVariable("TestString.unknown", &strTest);
  ASSERT(GXFAILED(hval));

  strTest.Clear();
  hval = pDataPool->GetVariable("TestStringArray", &strTest);  // 错误的寻址测试
  ASSERT(GXFAILED(hval));

  hval = pDataPool->GetVariable("TestStringArray[0]", &strTest);
  ASSERT(strTest == L"Hello world");

  hval = pDataPool->GetVariable("TestStringArray[1]", &strTest);
  ASSERT(strTest == L"sample01");

  hval = pDataPool->GetVariable("TestStringArray[2]", &strTest);
  ASSERT(strTest == L"sample02");

  hval = pDataPool->GetVariable("TestStringArray[3]", &strTest);
  ASSERT(GXFAILED(hval));

  hval = pDataPool->SetVariable("TestStringArray[2]", &clStringW(L"sample"));
  hval = pDataPool->GetVariable("TestStringArray[2]", &strTest);
  ASSERT(strTest == L"sample");
  //*/
  //////////////////////////////////////////////////////////////////////////
  TestDynamicStringArray(pDataPool);
  TestSizeOf(pDataPool);
  TestDynamicArrayTest(pDataPool);
  //TestDataPoolAgent(pDataPool);
  TestDataPoolAgent2(pDataPool);
  //////////////////////////////////////////////////////////////////////////
  /*
  float3 testFloat3(4,5,6);
  hval = pDataPool->SetVariable("testArray2[2]", &testFloat3);
  testFloat3.set(0,0,0);
  hval = pDataPool->GetVariable("testArray2[1]", &testFloat3);
  hval = pDataPool->GetVariable("testArray2[2]", &testFloat3);
  ASSERT(testFloat3 == float3(4,5,6));

  float testFloat = 123.45f;
  hval = pDataPool->SetVariable("testDynArray[0]", &testFloat);
  testFloat = 0.0f;
  hval = pDataPool->GetVariable("testDynArray[0]", &testFloat);
  ASSERT(testFloat == 123.45f);

  //////////////////////////////////////////////////////////////////////////
  hval = pDataPool->SetVariable("VarMember.name", &clStringW(L"TestVarMember"));

  testFloat3.set(1,2,3);
  hval = pDataPool->SetVariable("VarMember.vertex[0]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));

  testFloat3.set(5,4,3);
  hval = pDataPool->SetVariable("VarMember.vertex[1]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));

  testFloat3.set(5,6,7);
  hval = pDataPool->SetVariable("VarMember.vertex[2]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));

  testFloat3.set(9,8,7);
  hval = pDataPool->SetVariable("VarMember.vertex[3]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));

  //////////////////////////////////////////////////////////////////////////
  testFloat3.set(0,0,0);
  strTest = "";

  hval = pDataPool->GetVariable("VarMember.name", &strTest);
  ASSERT(GXSUCCEEDED(hval));
  ASSERT(strTest == L"TestVarMember");

  hval = pDataPool->GetVariable("VarMember.vertex[0]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));
  ASSERT(testFloat3 == float3(1,2,3));

  hval = pDataPool->GetVariable("VarMember.vertex[1]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));
  ASSERT(testFloat3 == float3(5,4,3));

  testFloat3.set(5,6,7);
  hval = pDataPool->GetVariable("VarMember.vertex[2]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));
  ASSERT(testFloat3 == float3(5,6,7));

  hval = pDataPool->GetVariable("VarMember.vertex[3]", &testFloat3);
  ASSERT(GXSUCCEEDED(hval));
  ASSERT(testFloat3 == float3(9,8,7));

  //*/
  ENUM_DATAPOOL(pDataPool);
  SAFE_RELEASE(pDataPool);
}


//class B
//{
//  int a;
//  int b;
//public:
//  B(int aa, int bb) : a(aa), b(bb){}
//  virtual int I1() = NULL;
//  virtual int I2() = NULL;
//};
//
//class BS : public B
//{
//public:
//  BS(int aa, int bb) : B(aa, bb){}
//  virtual int I1()
//  {
//    return 5;
//  }
//  virtual int I2()
//  {
//    return 6;
//  }
//};
void TestDataPool_CompileFromCode()
{
  TRACE("=== %s ===\n", __FUNCTION__);

  static char* s_szCode = 
    "int nFirst = 123456;"
    "int aInts[5] = {12,23,34,45,};"
    "string aNames[] = {\"test str 1\", \"test2\"};"
    "int nLastOne = {567890};"
    ;
  static int s_aCheckInts[] = {12,23,34,45,00}; // 检查用的数组
  DataPool* pDataPool = NULL;
  DataPool::CompileFromMemory(&pDataPool, NULL, NULL, s_szCode);

  if (pDataPool == NULL) {
    return;
  }

  MOVariable varArray;
  MOVariable varFixedArray;
  MOVariable varFirst, varLast;
  pDataPool->QueryByExpression("aNames", &varArray);

  pDataPool->QueryByExpression("nFirst", &varFirst);
  pDataPool->QueryByExpression("nLastOne", &varLast);


  pDataPool->QueryByExpression("aInts", &varFixedArray);
  for(int i = 0; i < 5; i++)
  {
    MOVariable var = varFixedArray.IndexOf(i);
    TRACE("aInts[%d] = %d\n", i, var.ToInteger());
    // 初始化的检查
    ASSERT(s_aCheckInts[i] == var.ToInteger());
  }

  ASSERT(varFirst.ToInteger() == 123456);
  ASSERT(varLast.ToInteger() == 567890);

  for(int i = 0; s_szExampleString[i] != NULL; i++)
  {
    MOVariable var = varArray.NewBack();
    var.Set(s_szExampleString[i]);
  }

  ASSERT(varFirst.ToInteger() == 123456);
  ASSERT(varLast.ToInteger() == 567890);
  for(int i = 0; s_szExampleString[i] != NULL; i++)
  {
    MOVariable var = varArray.IndexOf(i);
    TRACE("aNames[%d]=\"%s\";\n", i, var.ToStringA());
    if(i == 0) {
      ASSERT(var.ToStringA() == "test str 1");
    }
    else if(i == 1) {
      ASSERT(var.ToStringA() == "test2");
    }
    else {
      ASSERT(var.ToStringW() == s_szExampleString[i - 2]);
    }
  }

  ENUM_DATAPOOL(pDataPool);
  SAFE_RELEASE(pDataPool);
}






// 这个测试通过偏移定位文本行列和
// 通过行列找到偏移的类
void TestTextGetLines()
{
  static char aTestText[] = 
    "a\n"
    "ab\n"
    "abc\n"
    "abcd\n"
    "abcdef\n"
    "abcdefg\n"
    "abcdefgh\n";

  clstd::TextLines<char> t(aTestText, clstd::strlenT(aTestText));

  int nLine;
  int nRow;
  clsize p;

  p = t.OffsetFromPos(3, 2);
  //TRACE("[pos]=%c", aTestText[p]);
  ASSERT(aTestText[p] == 'b');

  t.PosFromOffset(p, &nLine, &nRow);
  //TRACE("%d,%d\n", nLine, nRow);
  ASSERT(nLine == 3 && nRow == 2);


  p = t.OffsetFromPos(3, 3);
  //TRACE("[pos]=%c\n", aTestText[p]);
  ASSERT(aTestText[p] == 'c');

  t.PosFromOffset(p, &nLine, &nRow);
  //TRACE("%d,%d\n", nLine, nRow);
  ASSERT(nLine == 3 && nRow == 3);


  // 错误地址访问
  p = t.OffsetFromPos(3, 4);
  //TRACE("[pos]=%c\n", aTestText[p]);
  ASSERT(p == -1);

  t.PosFromOffset(p, &nLine, &nRow);
  //TRACE("%d,%d\n", nLine, nRow);
  ASSERT(nLine == 8 && nRow == 0);


  p = t.OffsetFromPos(7, 5);
  //TRACE("[pos]=%c\n", aTestText[p]);
  ASSERT(aTestText[p] == 'e');

  t.PosFromOffset(p, &nLine, &nRow);
  //TRACE("%d,%d\n", nLine, nRow);
  ASSERT(nLine == 7 && nRow == 5);
}

void TestStringCutter()
{
  static char* teststring[] = {
    "abc",
    "abc,bcd",
    ",abc",
    "abc,",
    ",abc,",
    ",",
    ",,,abc",
    0,
  };
  
  static char* checkstring[] = {
    "abc",
    "abc","bcd",
    "","abc",
    "abc",
    "","abc",
    "",
    "","","","abc",
    "abc","bcd",
    "abc","bcd",
    "abc","bcd","e",
    0,
  };

  clStringA str;
  int n = 0;
  clstd::StringCutter<clStringA> sc;
  
  for(int i = 0; teststring[i] != 0; ++i)
  {
    sc.Set(teststring[i]);
    while( ! sc.IsEndOfString()){
      sc.Cut(str, ',');
      ASSERT(str == checkstring[n++]);
      //TRACE("%d:%s\n", i, str);
    }
  }

  for(int i = 7; i <= 9; i++)
  {
    sc.Set("abc,bcd,efg", i);
    while( ! sc.IsEndOfString()){
      sc.Cut(str, ',');
      ASSERT(str == checkstring[n++]);
      //TRACE("%s\n", str);
    }
  }
}

void TestHeaderSize()
{
  static DATAPOOL_VARIABLE_DECLARATION s_aVariable[] = 
  {
    {"float", "fFactor", 0,},
    {NULL, NULL}
  };

  DataPool* pDataPool = NULL;
  GXHRESULT hr = DataPool::CreateDataPool(&pDataPool, NULL, NULL, s_aVariable);
  if(GXSUCCEEDED(hr)) {
    pDataPool->Save(_CLTEXT("Test\\HeaderSize.dpl"));
    SAFE_RELEASE(pDataPool);
  }
}

void TestShaderConstantBuffer();

int _tmain(int argc, _TCHAR* argv[])
{
  clpathfile::LocalWorkingDirW(_CLTEXT(".."));

  // TestLines class test
  TestTextGetLines();
  TestStringCutter();

  // Data pool test
  TestShaderConstantBuffer();
  TestHashBuckets();
  TestHeaderSize();
  TestHugeArray();
  TestComplexArray();
  TestSelfContainStruct();
  TestBaseProperty();
  test();
  TestDataPool_CompileFromCode();

  // 下面这个抽取自真实游戏的样本数据，数据文本大约32MB，90万行数据
  // 由于版权问题无法提供样例文本
  TestCompileFromFile();
  return 0;
}

void CompareVariable( const DataPoolVariable& var1, const DataPoolVariable& var2 )
{
  class var_class : public DataPoolVariable
  {
  public:
    void CompareVariable(const var_class& var) const
    {
      ASSERT(m_vtbl      == var.m_vtbl);
      ASSERT(m_pDataPool == var.m_pDataPool);
      ASSERT(m_pVdd      == var.m_pVdd);
      ASSERT(m_pBuffer   == var.m_pBuffer);
      ASSERT(m_AbsOffset == var.m_AbsOffset);
    }
  };

  const var_class* pVar1 = reinterpret_cast<const var_class*>(&var1);
  const var_class* pVar2 = reinterpret_cast<const var_class*>(&var2);

  pVar1->CompareVariable(*pVar2);
}

