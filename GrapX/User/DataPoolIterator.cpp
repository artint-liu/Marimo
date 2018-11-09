﻿#include "GrapX.h"
#include "GrapX.Hxx"

#include "GrapX/DataPool.h"
#include "GrapX/DataPoolIterator.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/GXKernel.h"

#include "DataPoolImpl.h"
using namespace clstd;

namespace Marimo
{
  //////////////////////////////////////////////////////////////////////////
  namespace DataPoolUtility
  {
    void iterator::first_child( iterator& it ) const
    {
      it.pDataPool = pDataPool;

      if(pVarDesc->GetTypeDesc()->Cate == DataPoolTypeClass::Structure) {
        it.pVarDesc = pVarDesc->MemberBeginPtr();
        //&pDataPool->m_aMembers[pVarDesc->GetTypeDesc()->nMemberIndex];
        it.nOffset  = offset();
      }
      else {
        // 设为无效
        it.pVarDesc = NULL;
        it.nOffset  = 0;
        it.pBuffer  = NULL;
        return;
      }

      if(pVarDesc->IsDynamicArray()) // 动态数组
      {
        it.pBuffer = pBuffer; // child_buffer();
        it.nOffset = 0;
      }
      else {
        it.pBuffer = pBuffer;
      }

      // 数组元素
      if(index != (GXUINT)-1) {
        it.nOffset += index * pVarDesc->TypeSize();
      }
    }

    clBufferBase* iterator::child_buffer() const
    {
      ASSERT(pVarDesc->IsDynamicArray());
      return pBuffer ? child_buffer_unsafe() : NULL;
    }

    void iterator::StepArrayMember(iterator& it)
    {
      // 判断是this与it是父子关系
      //ASSERT((GXINT_PTR)it.pVarDesc >= (GXINT_PTR)&pDataPool->m_aMembers[pVarDesc->GetTypeDesc()->nMemberIndex] &&
      //  (GXINT_PTR)it.pVarDesc < (GXINT_PTR)&pDataPool->m_aMembers[pVarDesc->GetTypeDesc()->nMemberIndex + pVarDesc->GetTypeDesc()->nMemberCount]);
      ASSERT((GXUINT_PTR)it.pVarDesc >= (GXUINT_PTR)pVarDesc->MemberBeginPtr() &&
        (GXUINT_PTR)it.pVarDesc < (GXUINT_PTR)pVarDesc->MemberBeginPtr() + pVarDesc->MemberCount());
      ASSERT(IsArray() && index != (GXUINT)-1); // 必须是数组
      index++;
      //FullNameA(it.ParentName);
      it.nOffset += pVarDesc->TypeSize();
    }

    clBufferBase* iterator::child_buffer_unsafe() const
    {
      return reinterpret_cast<const DataPoolImpl::VARIABLE_DESC*>(pVarDesc)->GetAsBuffer((GXBYTE*)pBuffer->GetPtr() + nOffset);
    }

    iterator iterator::begin() const
    {
      iterator it;
      first_child(it);
      it.index = (GXUINT)-1;

      //FullNameA(it.ParentName);
      return it;
    }

    iterator iterator::end() const
    {
      iterator it;
      first_child(it);
      it.index = -1;

      ASSERT(it.pVarDesc != NULL || pVarDesc->GetTypeDesc()->Cate != DataPoolTypeClass::Structure); // 下面的判断好像不准确，这里断言一下验证

      if(it.pVarDesc != NULL) {
        // 如果是结构体，把pVarDesc调整到结构体末尾
        it.pVarDesc += pVarDesc->MemberCount(); // 注意：可能是无效地址
      }
      return it;
    }

    void iterator::first_element(element_iterator& it) const
    {
      ASSERT(index == -1);
      it.pDataPool = pDataPool;
      it.pVarDesc  = pVarDesc;
      if(pVarDesc->IsDynamicArray()) {
        it.pBuffer   = child_buffer();
        it.nOffset   = 0;
      }
      else {
        it.pBuffer   = pBuffer;
        it.nOffset   = nOffset;
      }

      //it.ParentName = ParentName;

    }

    element_iterator iterator::array_begin() const
    {
      ASSERT(index == -1);

      element_iterator iter;
      first_element(iter);
      iter.index = 0;

      return iter;
    }

    element_iterator iterator::array_end() const
    {
      ASSERT(index == -1);

      element_iterator iter;
      first_element(iter);
      iter.index = array_length();
      return iter;
    }

    element_reverse_iterator iterator::rarray_begin() const
    {
      relement_iterator iter;
      first_element(iter);
      iter.index = array_length() - 1;

      return iter;
    }

    element_reverse_iterator iterator::rarray_end() const
    {
      relement_iterator iter;
      first_element(iter);
      iter.index = -1;
      return iter;
    }

    DataPool::LPCSTR iterator::TypeName() const
    {
      return (DataPool::LPCSTR)pVarDesc->TypeName();
    }

    GXBOOL iterator::IsArray() const
    {
      return pVarDesc->IsDynamicArray() || pVarDesc->nCount > 1;
    }

    DataPool::LPCSTR iterator::VariableName() const
    {
      return (DataPool::LPCSTR)pVarDesc->VariableName();
    }

    DataPoolVariable iterator::ToVariable() const
    {
      DataPoolVariable var;
      return ToVariable(var);
    }

    DataPoolVariable& iterator::ToVariable(DataPoolVariable& var) const
    {
      DataPoolVariable::VTBL* vtbl;
      GXUINT nElementOffset = 0;

      var.Free();

      if(index == (GXUINT)-1) {
        vtbl = reinterpret_cast<DataPoolVariable::VTBL*>(
          reinterpret_cast<const DataPoolImpl::VARIABLE_DESC*>(pVarDesc)->GetMethod());
      }
      else {
        vtbl = reinterpret_cast<DataPoolVariable::VTBL*>(
          reinterpret_cast<const DataPoolImpl::VARIABLE_DESC*>(pVarDesc)->GetUnaryMethod());
        if(IS_MARK_NX16B(pDataPool->GetFlags())) {
          nElementOffset = index * ALIGN_16(pVarDesc->TypeSize());
        }
        else {
          nElementOffset = index * pVarDesc->TypeSize();
        }
      }

      // 动态数组使用单独的buffer， 同时偏移也重新计算
      //if(pVarDesc->IsDynamicArray()) {
      //  if(pBuffer) {
      //    new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
      //      child_buffer_unsafe(), nElementOffset);
      //  }
      //}
      //else {
      //  new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
      //    pBuffer, offset() + nElementOffset);
      //}

      if(pVarDesc->IsDynamicArray()) {
        if(index == -1)
        {
          if(pBuffer) {
            //new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
            //  child_buffer_unsafe(), nElementOffset);
            new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
              pBuffer, offset() + nElementOffset);
          }
        }
        else {
          ASSERT(nOffset == 0);
          new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
            pBuffer, nElementOffset);
        }
      }
      else {
        new(&var) DataPoolVariable(vtbl, pDataPool, pVarDesc, 
          pBuffer, offset() + nElementOffset);
      }

      return var;
    }

    GXUINT iterator::offset() const
    {
      return pVarDesc->nOffset + nOffset;
    }

    GXUINT iterator::array_length() const
    {
      if(pVarDesc->IsDynamicArray()) {
        clBufferBase* pChildBuffer = child_buffer();
        return pChildBuffer == NULL ? 0
          : (GXUINT)pChildBuffer->GetSize() / pVarDesc->TypeSize();
      }
      else {
        return pVarDesc->nCount;
      }
    }

    //////////////////////////////////////////////////////////////////////////

    element_iterator& element_iterator::operator++()
    {
      ++index;
      return *this;
    }

    element_reverse_iterator& element_reverse_iterator::operator++()
    {
      --index;
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////

    void named_iterator::StepArrayMember(named_iterator& it)
    {
      iterator::StepArrayMember(it);
      FullNameA(it.ParentName);
    }

    named_iterator named_iterator::begin() const
    {
      named_iterator it;
      iterator::first_child((iterator&)it);
      it.index = (GXUINT)-1;

      FullNameA(it.ParentName);
      return it;
    }

    named_iterator named_iterator::end() const
    {
      named_iterator it;
      iterator::first_child((iterator&)it);
      it.index = -1;

      //ASSERT(it.pVarDesc != NULL); // 下面的判断好像不准确，这里断言一下验证
      ASSERT(it.pVarDesc != NULL || pVarDesc->GetTypeDesc()->Cate != DataPoolTypeClass::Structure); // 下面的判断好像不准确，这里断言一下验证

      if(it.pVarDesc != NULL) {
        // 如果是结构体，把pVarDesc调整到结构体末尾
        it.pVarDesc += pVarDesc->MemberCount(); // 注意：可能是无效地址
      }
      return it;
    }

    named_element_iterator named_iterator::array_begin() const
    {
      ASSERT(index == -1);

      named_element_iterator iter;
      first_element((DataPoolUtility::element_iterator&)iter);
      iter.ParentName = ParentName;
      iter.index = 0;

      return iter;
    }

    named_element_iterator named_iterator::array_end() const
    {
      ASSERT(index == -1);

      named_element_iterator iter;
      first_element((DataPoolUtility::element_iterator&)iter);
      iter.ParentName = ParentName;
      iter.index = array_length();
      return iter;
    }

    named_element_reverse_iterator named_iterator::rarray_begin() const
    {
      named_element_reverse_iterator iter;
      first_element((DataPoolUtility::element_iterator&)iter);
      iter.ParentName = ParentName;
      iter.index = array_length() - 1;

      return iter;
    }

    named_element_reverse_iterator named_iterator::rarray_end() const
    {
      named_element_reverse_iterator iter;
      first_element((DataPoolUtility::element_iterator&)iter);
      iter.ParentName = ParentName;
      iter.index = -1;
      return iter;
    }


    clStringA named_iterator::FullNameA() const
    {
      clStringA str;
      return FullNameA(str);
    }

    clStringA& named_iterator::FullNameA(clStringA& str) const
    {
      str = ParentName.IsEmpty()
        ? clStringA((DataPool::LPCSTR)pVarDesc->VariableName())
        : ParentName + "." + (DataPool::LPCSTR)pVarDesc->VariableName();

      if(index != (GXUINT)-1) {
        if(pVarDesc->nCount > 1 || pVarDesc->IsDynamicArray()) {
          str.Append('[');
          str.AppendInteger32(index);
          str.Append(']');
        }
      }
      return str;
    }

    //////////////////////////////////////////////////////////////////////////

    named_element_iterator& named_element_iterator::operator++()
    {
      ++index;
      return *this;
    }

    named_element_reverse_iterator& named_element_reverse_iterator::operator++()
    {
      --index;
      return *this;
    }


  } // namespace DataPoolUtility
} // namespace Marimo
