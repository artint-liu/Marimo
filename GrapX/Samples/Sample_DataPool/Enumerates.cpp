#include <Marimo.H>
//#include <include/DataInfrastructure.h>
#include "GrapX/DataPoolIterator.h"
#include "TestDataPool.h"
#define CHECK_VAR
#define LOG TRACE
size_t EnumerateVariables(DataPool* pDataPool, int nDepth, DataPoolIterator& itBegin, DataPoolIterator& itEnd);

size_t EunmerateArray(DataPool* pDataPool, int nDepth, DataPoolElementIterator& itBegin, DataPoolElementIterator& itEnd)
{
  size_t count = 0;
  MOVariable var;
#ifdef CHECK_VAR
  MOVariable varCheck;
#endif // #ifdef CHECK_VAR
  for(auto it = itBegin; it != itEnd; ++it, ++count)
  {
    it.ToVariable(var);
#ifdef CHECK_VAR
    pDataPool->QueryByExpression(it.FullNameA(), &varCheck);
#endif // #ifdef CHECK_VAR

    LOG("%*c%s[%08x:%d] %*s=\"%s\"\n", nDepth * 2, '!', it.TypeName(), 
      it.pBuffer ? (GXINT_PTR)it.pBuffer->GetPtr() : NULL, it.offset(),
      30, it.FullNameA(), var.IsValid() ? var.ToStringA() : "<null>");

#ifdef CHECK_VAR
    ASSERT(varCheck.GetPtr() == var.GetPtr());
    ASSERT(varCheck.GetCaps() == var.GetCaps());
#endif // #ifdef CHECK_VAR

    var.GetFullName();

    count += EnumerateVariables(pDataPool, nDepth + 1, it.begin(), it.end());
  }
  return count;
}

size_t EnumerateVariables(DataPool* pDataPool, int nDepth, DataPoolIterator& itBegin, DataPoolIterator& itEnd)
{
  size_t count = 0;
  MOVariable var;
#ifdef CHECK_VAR
  MOVariable varCheck;
#endif // #ifdef CHECK_VAR
  clStringW nameCheck;
  for(auto it = itBegin; it != itEnd; ++it, ++count)
  {
    MOVariable var = it.ToVariable();
    auto length = it.array_length();
    auto bArray = it.IsArray();
    auto name = it.FullNameA();

    // #.iterator 只做遍历，不会影响动态数组buffer的产生
    // #.QueryByExpression 查询到空数组时会创建一个有一定容量长度是0的动态缓冲区
    // 所以这里var有可能无效但是varCheck是有效的
#ifdef CHECK_VAR
    pDataPool->QueryByExpression(name, &varCheck);
#endif // #ifdef CHECK_VAR

    LOG("%*c%s %s[%08x:%d])(x%d) %*s=\"%s\"\n", nDepth * 2, '+', it.TypeName(), it.VariableName(), 
      it.pBuffer ? (GXINT_PTR)it.pBuffer->GetPtr() : NULL, it.offset(),
      length, 30, name, var.IsValid() ? var.ToStringA() : "<null>");

    nameCheck = var.GetFullName();

#ifdef CHECK_VAR
    //*
    if(var.IsValid())
    {
      ASSERT(varCheck.GetPtr() == var.GetPtr());
      ASSERT(varCheck.GetCaps() == var.GetCaps());
    }
    else if(varCheck.IsValid())
    {
      // VTBL* m_vtbl;
      ASSERT(((GXLONG_PTR*)&var)[0] == ((GXLONG_PTR*)&varCheck)[0]);

      // DataPool* m_pDataPool;
      ASSERT(((GXLONG_PTR*)&var)[1] == ((GXLONG_PTR*)&varCheck)[1]);

      // DPVDD* m_pVdd;
      ASSERT(((GXLONG_PTR*)&var)[2] == ((GXLONG_PTR*)&varCheck)[2]);

      // clBufferBase* m_pBuffer;
      ASSERT(((GXLONG_PTR*)&var)[3] == 0 && ((GXLONG_PTR*)&varCheck)[3] != 0);

      // GXUINT m_AbsOffset; // 相对于 m_pBuffer 指针的偏移      
      ASSERT(*(GXUINT*)(&(((GXLONG_PTR*)&var)[4])) == *(GXUINT*)(&(((GXLONG_PTR*)&varCheck)[4])));
    }//*/
#endif // #ifdef CHECK_VAR

    if(bArray) {
      count += EunmerateArray(pDataPool, nDepth + 1, it.array_begin(), it.array_end());
      for(auto itVar = var.array_begin(); itVar != var.array_end(); ++itVar)
      {
        auto v1 = itVar.ToVariable();
        auto v2 = var[itVar - var.array_begin()];
        LOG("[]name:%s\n", var.GetName());
        CompareVariable(v1, v2);
      }
    }
    else {
      count += EnumerateVariables(pDataPool, nDepth + 1, it.begin(), it.end());

      for(auto itVar = var.begin(); itVar != var.end(); ++itVar)
      {
        auto v1 = itVar.ToVariable();
        auto v2 = var[itVar.VariableName()];
        CompareVariable(v1, v2);
      }
    }
  }
  return count;
}

//////////////////////////////////////////////////////////////////////////

size_t EnumerateVariables(DataPool* pDataPool)
{
  TRACE("=======================================================\n");
  const size_t count = EnumerateVariables(pDataPool, 0, pDataPool->named_begin(), pDataPool->named_end());
  TRACE("=======================================================\n");
  return count;
}

struct ITER_DESC
{
  DataPool* pDataPool;
  DataPoolIterator itBegin;
  DataPoolIterator itEnd;
};

struct ITER_ELEMENT_DESC
{
  DataPool* pDataPool;
  DataPoolElementIterator itBegin;
  DataPoolElementIterator itEnd;
};

size_t CompareDataPool(int nDepth, ITER_DESC* pDescA, ITER_DESC* pDescB);


size_t CompareDataPoolArray(int nDepth, ITER_ELEMENT_DESC* pDescA, ITER_ELEMENT_DESC* pDescB)
{
  auto itA = pDescA->itBegin;
  auto itB = pDescB->itBegin;
  size_t count = 0;

  for(; itA != pDescA->itEnd; ++itA, ++itB, ++count)
  {
    ITER_DESC desc_A = {pDescA->pDataPool, itA.begin(), itA.end()};
    ITER_DESC desc_B = {pDescB->pDataPool, itB.begin(), itB.end()};
    count += CompareDataPool(nDepth + 1, &desc_A, &desc_B);
  }

  ASSERT(itB == pDescB->itEnd);
  return count;
}


size_t CompareDataPool(int nDepth, ITER_DESC* pDescA, ITER_DESC* pDescB)
{
  size_t count = 0; // count of variable
  auto itA = pDescA->itBegin;
  auto itB = pDescB->itBegin;
  for(; itA != pDescA->itEnd; ++itA, ++itB, ++count)
  {
    if(GXSTRCMP(itA.TypeName(), itB.TypeName()) != 0) {
      TRACE("[TypeName] %s != %s\n", itA.TypeName(), itB.TypeName());
      CLBREAK;
    }

    if(GXSTRCMP(itA.VariableName(), itB.VariableName()) != 0) {
      TRACE("[VariableName] %s != %s\n", itA.VariableName(), itB.VariableName());
      CLBREAK;
    }

    DataPoolVariable varA = itA.ToVariable();
    DataPoolVariable varB = itB.ToVariable();
    clStringW strA = varA.ToStringW();
    clStringW strB = varB.ToStringW();
    if(strA != strB) {
      TRACE("[Var] %s != %s(值不相同)\n", strA, strB);
      CLBREAK;
    }

    if(itA.IsArray() && itB.IsArray())
    {
      if(varA.GetLength() != varB.GetLength()) {
        TRACE("数组长度不一致(%d != %d)\n", varA.GetLength(), varB.GetLength());
        CLBREAK;
      }

      ITER_ELEMENT_DESC desc_A = {pDescA->pDataPool, itA.array_begin(), itA.array_end()};
      ITER_ELEMENT_DESC desc_B = {pDescB->pDataPool, itB.array_begin(), itB.array_end()};
      count += CompareDataPoolArray(nDepth + 1, &desc_A, &desc_B);
    }
    else if( ! itA.IsArray() && ! itB.IsArray())
    {
      ITER_DESC desc_A = {pDescA->pDataPool, itA.begin(), itA.end()};
      ITER_DESC desc_B = {pDescB->pDataPool, itB.begin(), itB.end()};
      count += CompareDataPool(nDepth + 1, &desc_A, &desc_B);
    }
    else
    {
      CLBREAK;
    }
  }

  if(itB != pDescB->itEnd) {
    TRACE("长度不一致\n");
    CLBREAK;
  }
  return count;
}

size_t CompareDataPool(DataPool* pDataPoolA, DataPool* pDataPoolB)
{
  ITER_DESC desc_A = {pDataPoolA, pDataPoolA->named_begin(), pDataPoolA->named_end()};
  ITER_DESC desc_B = {pDataPoolB, pDataPoolB->named_begin(), pDataPoolB->named_end()};
  return CompareDataPool(0, &desc_A, &desc_B);
}

//////////////////////////////////////////////////////////////////////////
struct ENUMERATE_CONTEXT
{
  int nNumString;
  int nNumArray;  // 只是动态数组
};


b32 EnumeratePtrControl(ENUMERATE_CONTEXT& ctx, int nDepth, DataPoolIterator* pParent, DataPoolIterator& itBegin, DataPoolIterator& itEnd);

void EnumStrings(ENUMERATE_CONTEXT& ctx, int nDepth, DataPoolIterator& itArray)
{
  MOVariable var;
  auto itEnd = itArray.array_end();
  for(auto it = itArray.array_begin(); it != itEnd; ++it)
  {
    it.ToVariable(var);
    TRACE("%*c %s=\"%s\"\n", nDepth * 2, 'T', it.FullNameA(), var.IsValid() ? var.ToStringA() : "<null>");
    ctx.nNumString++;
  }
}

//void EnumStructs(ENUMERATE_CONTEXT& ctx, int nDepth, DataPool::iterator& itStructs)
//{
//  auto itEnd = itStructs.array_end();
//  for(auto itElement = itStructs.array_begin(); itElement != itEnd; ++itElement)
//  {
//    EnumeratePtrControl(ctx, nDepth + 1, &itElement, itElement.begin(), itElement.end());
//  }
//
//}
//
//void CheckIter(const DataPool::iterator& it)
//{
//  MOVariable var1;
//  MOVariable var2;
//  it.pDataPool->QueryByExpression(it.FullNameA(), &var1);
//  it.ToVariable(var2);
//  ASSERT(var1.GetPtr() == var2.GetPtr());
//}


b32 EnumeratePtrControl(ENUMERATE_CONTEXT& ctx, int nDepth, DataPoolIterator* pParent, DataPoolIterator& itBegin, DataPoolIterator& itEnd)
{
  b32 bval = FALSE;
  MOVariable var;
  MOVariable varCheck;

  //if(pParent && pParent->pVarDesc->IsDynamicArray())
  //{
  //  TRACE("*%08x\n", pParent->pBuffer);
  //  ctx.nNumArray++;
  //}

  for(auto itMember = itBegin; itMember != itEnd; ++itMember)
  {
    if(itMember.pVarDesc->IsDynamicArray() && itMember.child_buffer())
    {
      for(auto itTest = itMember.array_begin(); itTest != itMember.array_end(); ++itTest)
      {
        CLNOP;
      }
      TRACE("*%s[x%d %08x]\n", itMember.FullNameA(), itMember.array_length(), itMember.child_buffer());
      //CheckIter(itMember);
      ctx.nNumArray++;
    }

    switch(itMember.pVarDesc->GetTypeCategory())
    {
    case DataPoolTypeClass::String:
      if(itMember.IsArray())
      {
        if(pParent && pParent->IsArray())
        {
          int nCount = pParent->array_length();
          auto itStep = itMember;
          auto parentindex = pParent->index;
          ASSERT(pParent->index == 0);

          for(int i = 0; i < nCount; i++)
          {
            EnumStrings(ctx, nDepth + 1, itStep);
            pParent->StepArrayMember(itStep);
          }

          pParent->index = parentindex; // 恢复父索引
          bval = TRUE;
        }
        else {
          EnumStrings(ctx, nDepth + 1, itMember);
        }
      }
      else {
        if(pParent && pParent->IsArray())
        {
          int nCount = pParent->array_length();
          auto itStep = itMember;
          auto parentindex = pParent->index;
          ASSERT(pParent->index == 0);

          for(int i = 0; i < nCount; i++)
          {
            //EnumStrings(ctx, nDepth + 1, itStep);
            itStep.ToVariable(var);
            TRACE("%*c %s=\"%s\"\n", nDepth * 2, 'T', itStep.FullNameA(), var.IsValid() ? var.ToStringA() : "<null>");
            ctx.nNumString++;
            pParent->StepArrayMember(itStep);
          }

          pParent->index = parentindex; // 恢复父索引
          bval = TRUE;
        }
        else {
          itMember.ToVariable(var);
          TRACE("%*c %s=\"%s\"\n", nDepth * 2, 'T', itMember.FullNameA(), var.IsValid() ? var.ToStringA() : "<null>");
          ctx.nNumString++;
        }
      }
      break;

    case DataPoolTypeClass::Structure:
      if(itMember.IsArray())
      {
        //*
        if(pParent && pParent->IsArray())
        {
          int nCount = pParent->array_length();
          auto itStep = itMember;
          auto parentindex = pParent->index;
          ASSERT(pParent->index == 0);

          for(int i = 0; i < nCount; i++)
          {
            auto itElement = itStep.array_begin();
            EnumeratePtrControl(ctx, nDepth + 1, &itElement, itElement.begin(), itElement.end());

            pParent->StepArrayMember(itStep);
          }

          pParent->index = parentindex; // 恢复父索引
          bval = TRUE;
        }
        else {
          auto itElement = itMember.array_begin();
          EnumeratePtrControl(ctx, nDepth + 1, &itElement, itElement.begin(), itElement.end());
        }
        /*/
        auto itEnd = itMember.array_end();
        auto itElement = itMember.array_begin();
        for(auto itElement = itMember.array_begin(); itElement != itEnd; ++itElement)
        {
          if(EnumeratePtrControl(ctx, nDepth + 1, &itElement, itElement.begin(), itElement.end())) {
            break;
          }
        }
        //*/
      }
      else {
        if(pParent && pParent->IsArray())
        {
          int nCount = pParent->array_length();
          auto itStep = itMember;
          auto parentindex = pParent->index;
          ASSERT(pParent->index == 0);

          for(int i = 0; i < nCount; i++)
          {
            //auto itElement = itStep.array_begin();
            EnumeratePtrControl(ctx, nDepth + 1, &itStep, itStep.begin(), itStep.end());

            pParent->StepArrayMember(itStep);
          }

          pParent->index = parentindex; // 恢复父索引
          bval = TRUE;
        }
        else
        {
          EnumeratePtrControl(ctx, nDepth + 1, &itMember, itMember.begin(), itMember.end());
        }
      }
      break;
    }
  }

  return bval;
}

size_t EnumeratePtrControl(DataPool* pDataPool)
{
  ENUMERATE_CONTEXT ctx = {0};
  TRACE("=======================================================\n");
  const size_t count = EnumeratePtrControl(ctx, 0, NULL, pDataPool->named_begin(), pDataPool->named_end());
  TRACE("Arrays:%d Strings:%d\n", ctx.nNumArray, ctx.nNumString);
  TRACE("=======================================================\n");
  return count;
}

void EnumeratePtrControl2(DataPool* pDataPool)
{
  ENUMERATE_CONTEXT ctx = {0};
  TRACE("=======================================================\n");
  //EnumeratePtrControl(ctx, 0, pDataPool->begin(), pDataPool->end());
  /*
  DataPoolUtility::EnumerateVariables(0, pDataPool->begin(), pDataPool->end(), 
    [&ctx](DataPool::iterator& it, clBufferBase* pBuffer, int nDepth)
  {
    //TRACE("*[%08x] %s\n", pBuffer, it.FullNameA());
    if(pBuffer)
    {
      ctx.nNumArray++;
    }
  },

    [&ctx](DataPool::iterator& it, int nDepth) 
  {
    if(it.pVarDesc->TypeCategory() == T_STRING)
    {
      ctx.nNumString++;
      TRACE("*%s\n", it.FullNameA());
    }

    if(it.pVarDesc->IsDynamicArray())
    {
      //if(it.child_buffer())
      //{
      //  //ctx.nNumArray++;
      //}
      //TRACE("#[%08x] %s\n", it.child_buffer(), it.FullNameA());
    }
    else
    {
      //TRACE("#[%08x] %s\n", it.pBuffer, it.FullNameA());
    }
  });
  /*/

  DataPoolUtility::EnumerateVariables2<DataPoolIterator, DataPoolUtility::named_element_iterator,
    DataPoolUtility::named_element_reverse_iterator>(pDataPool->named_begin(), pDataPool->named_end(), 
    [&ctx](int bArray, DataPoolIterator& it, int nDepth) 
  {
    if(bArray)
    {
      ASSERT(it.pVarDesc->IsDynamicArray() && it.index == (GXUINT)-1);
      if(it.child_buffer())
      {
        ctx.nNumArray++;
      }
    }

    if(it.pVarDesc->GetTypeCategory() == DataPoolTypeClass::String)
    {
      ctx.nNumString++;
      //TRACE("*%s\n", it.FullNameA());
    }

    if(it.pVarDesc->IsDynamicArray())
    {
      TRACE("#[%08x] %s\n", it.pBuffer, it.FullNameA());
    }
    else
    {
      TRACE("#[%08x] %s\n", it.pBuffer, it.FullNameA());
    }
  });

  //*/
  TRACE("Arrays:%d Strings:%d\n", ctx.nNumArray, ctx.nNumString);
  TRACE("=======================================================\n");
}