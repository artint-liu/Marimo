#ifndef _MARIMO_DATA_POOL_ITERATOR_H_
#define _MARIMO_DATA_POOL_ITERATOR_H_

#ifndef _MARIMO_DATA_POOL_H_
#error 必须在之前包含"DataPool.h"文件
#endif // #ifndef _MARIMO_DATA_POOL_H_

// Data Pool 迭代器

namespace Marimo
{
  class DataPool;

  namespace DataPoolUtility
  {
    struct iterator;
    struct element_iterator;
    struct element_reverse_iterator;
    struct named_iterator;
    struct named_element_iterator;
    struct named_element_reverse_iterator;
    // 匿名迭代器
    //
    // 在迭代过程中不记录来源名称，就是无法获得当前迭代式从上层哪个变量获得的
    // 因为没有字符串传递过程，这个迭代器效率稍微高些

    // 标准迭代
    // 这个用于遍历所有同级的变量
    // 如果是结构类型，使用begin可以获得结构体第一个成员变量的迭代
    // 结构体数组使用begin方法，将对其第一个元素下的成员变量进行遍历
    // 注意对于空的动态数组，ToVariable方法返回的可能是一个无效变量
    // 如果想对数组中每个元素进行迭代，使用array_begin或者rarray_begin方法
    struct GXDLL iterator
    {
      typedef const DATAPOOL_VARIABLE_DESC*  LPCVD;
      //typedef DataPoolUtility::element_iterator  element_iterator;
      typedef DataPoolUtility::element_reverse_iterator relement_iterator;

      DataPool*         pDataPool;
      //clStringA         ParentName;
      LPCVD             pVarDesc;
      clBufferBase*     pBuffer;
      GXUINT            nOffset;  // Parent在buffer里的偏移，全局变量一定是0，成员变量是成员所属struct在buffer的偏移
      GXUINT            index;    // 数组成员索引,(GXUINT)-1表示非数组或者数组对象，其它值表示数组元素


      iterator& operator++()
      {
        ++pVarDesc;
        return *this;
      }

      GXBOOL operator==(const iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && nOffset == it.nOffset);
        // 在这种情况下nOffset不同，但是pVarDesc相同
        // struct A{ int a; };
        // A a1;  A a2;
        return pVarDesc == it.pVarDesc;
      }

      GXBOOL operator!=(const iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer);
        return pVarDesc != it.pVarDesc;
      }


      iterator  begin           () const;
      iterator  end             () const;
      GXUINT            offset          () const; // 在Buffer中的绝对偏移, 数组的话是第一个元素开始的位置
      element_iterator  array_begin     () const; // 如果是数组的话，这是数组元素的迭代方法，一元变量这个不会报错
      element_iterator  array_end       () const;
      relement_iterator rarray_begin    () const; // 数组的反向迭代，从最后一个元素开始
      relement_iterator rarray_end      () const;
      GXUINT            array_length    () const;
      clBufferBase*     child_buffer    () const; // 动态数组返回它所持有的缓冲区，非动态数组返回NULL

      DataPoolVariable  ToVariable      () const;
      DataPoolVariable& ToVariable      (DataPoolVariable& var) const;
      //GXUINT            AbsoluteOffset  () const;
      DataPool::LPCSTR  VariableName    () const;
      //clStringA         FullNameA       () const;
      //clStringA&        FullNameA       (clStringA& str) const;
      DataPool::LPCSTR  TypeName        () const;
      GXBOOL            IsArray         () const;

      void              StepArrayMember (iterator& it); // 成员变量步进，it是this结构体数组的一个成员变量，
      //                                                           这个方法将it调整为数组下一个元素的成员变量

    protected:
      void first_child    (iterator& it) const;
      void first_element  (element_iterator& it) const;
      clBufferBase* child_buffer_unsafe() const;
    };

    // 数组元素迭代
    // iterator::array_begin方法可以得到这个数组的第一个元素
    // 如果是一元数组，元素迭代仍然有效，但迭代循环只进行一次
    struct GXDLL element_iterator : iterator
    {
      element_iterator& operator++();

      GXBOOL operator==(const element_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && 
          nOffset == it.nOffset && pVarDesc == it.pVarDesc);
        return index == it.index;
      }

      GXBOOL operator!=(const element_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && 
          nOffset == it.nOffset && pVarDesc == it.pVarDesc);
        return index != it.index;
      }

      GXUINT operator-(const element_iterator& it) const
      {
        return index - it.index;
      }
    };

    // 数组元素反向迭代
    // 就是从数组的最后一个元素开始向前迭代
    struct GXDLL element_reverse_iterator : element_iterator
    {
      element_reverse_iterator& operator++();
    };

    //////////////////////////////////////////////////////////////////////////

    // 具名迭代器
    // 这个迭代过程中记录了父系Variable的名字和索引，可以得到完整命名
    // FullNameA得到的命名使用DataPool::QueryByExpression接口获得的变量与迭代器
    // ToVariable得到的的变量对象是同一个。
    // 注意：DataPool::QueryByExpression方法获得变量，如果变量是空的动态数组，它将
    //      会为动态数组创建必要的访问缓冲区。而迭代器从空白动态数组获转换得到的
    //      变量，可能是无效的，所以在使用之前要使用DataPoolVariable::IsValid判断
    //      它是否有效。

    struct GXDLL named_iterator : iterator
    {
      typedef const DATAPOOL_VARIABLE_DESC*  LPCVD;
      //typedef DataPoolUtility::named_element_iterator  element_iterator;
      //typedef DataPoolUtility::named_element_reverse_iterator relement_iterator;

      clStringA     ParentName; // 上层变量的全名

      named_iterator& operator++()
      {
        ++pVarDesc;
        return *this;
      }

      GXBOOL operator==(const named_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && nOffset == it.nOffset);
        // 在这种情况下nOffset不同，但是pVarDesc相同
        // struct A{ int a; };
        // A a1;  A a2;
        return pVarDesc == it.pVarDesc;
      }

      GXBOOL operator!=(const named_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer);
        return pVarDesc != it.pVarDesc;
      }


      named_iterator    begin           () const;
      named_iterator    end             () const;
      //GXUINT            offset          () const; // 在Buffer中的绝对偏移, 数组的话是第一个元素开始的位置
      named_element_iterator         array_begin     () const; // 如果是数组的话，这是数组元素的迭代方法，一元变量这个不会报错
      named_element_iterator         array_end       () const;
      named_element_reverse_iterator rarray_begin    () const; // 数组的反向迭代，从最后一个元素开始
      named_element_reverse_iterator rarray_end      () const;
      //GXUINT            array_length    () const;
      //clBufferBase*     child_buffer    () const; // 动态数组返回它所持有的缓冲区，非动态数组返回NULL

      //DataPoolVariable  ToVariable      () const;
      //DataPoolVariable& ToVariable      (DataPoolVariable& var) const;
      //GXUINT            AbsoluteOffset  () const;
      //DataPool::LPCSTR  VariableName    () const;
      clStringA         FullNameA       () const;
      clStringA&        FullNameA       (clStringA& str) const;
      //DataPool::LPCSTR  TypeName        () const;
      //GXBOOL            IsArray         () const;

      void              StepArrayMember (named_iterator& it); // 成员变量步进，it是this结构体数组的一个成员变量，
      //                                                           这个方法将it调整为数组下一个元素的成员变量

    protected:
      //void first_child    (named_iterator& it) const;
      //void first_element  (element_iterator& it) const;
      //clBufferBase* child_buffer_unsafe() const;
    };


    struct GXDLL named_element_iterator : named_iterator
    {
      named_element_iterator& operator++();

      GXBOOL operator==(const named_element_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && 
          nOffset == it.nOffset && pVarDesc == it.pVarDesc);
        return index == it.index;
      }

      GXBOOL operator!=(const named_element_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && 
          nOffset == it.nOffset && pVarDesc == it.pVarDesc);
        return index != it.index;
      }

      GXUINT operator-(const named_element_iterator& it) const
      {
        return index - it.index;
      }
    };

    struct GXDLL named_element_reverse_iterator : named_element_iterator
    {
      named_element_reverse_iterator& operator++();
    };

    //////////////////////////////////////////////////////////////////////////
    /*
    // 结构体优先的遍历，深度优先
    template<class TArrayFunc, class TVarFunc>
    void EnumerateVariables(int nDepth, DataPool::iterator& itBegin, DataPool::iterator& itEnd, TArrayFunc _arrayFunc, TVarFunc _varFunc)
    {
      typedef DataPoolUtility::element_iterator element_iterator;
      for(auto itMember = itBegin; itMember != itEnd; ++itMember)
      {
        if(itMember.pVarDesc->IsDynamicArray()) {
          if(itMember.child_buffer()) {
            _arrayFunc(itMember, itMember.child_buffer(), nDepth);
          }
          else { continue; }
        }

        switch(itMember.pVarDesc->GetTypeCategory())
        {
        case T_STRUCT:
          if(itMember.IsArray()) {
            const element_iterator itElementEnd = itMember.array_end();
            for(auto itElement = itMember.array_begin(); itElement != itElementEnd; ++itElement) {
              EnumerateVariables(nDepth + 1, itElement.begin(), itElement.end(), _arrayFunc, _varFunc);
            }
          }
          else {
            EnumerateVariables(nDepth + 1, itMember.begin(), itMember.end(), _arrayFunc, _varFunc);
          }
          break;

        default:
          if(itMember.IsArray()) {
            auto itElementEnd = itMember.array_end();
            for(auto itElement = itMember.array_begin(); itElement != itElementEnd; ++itElement)
            {
              _varFunc(itElement, nDepth);
            }
          }
          else {
            _varFunc(itMember, nDepth);
          }
          break;
        }
      } // for
    } // EnumerateVariables//*/

    //////////////////////////////////////////////////////////////////////////

    // 缓冲区优先的遍历，会先遍历在同一个缓冲中的变量
    // 回调中会按照变量所在缓冲区排序
    // 如果回调中得到了一个动态数组，第一个参数为1，那么它一定是动态数组，并且在之后会收到这个动态数组下的所有变量
    // 回调中第一个参数为0时，所有变量一定都是非结构体类型变量，结构体类型会在这个遍历中被分解。
    template<class _TIter, class _TEleIter, class _TEleRIter, class TVarFunc>
    void EnumerateVariables2(_TIter& itBegin, _TIter& itEnd, TVarFunc _varFunc)
    {
      typedef _TEleIter  element_iterator;
      typedef _TEleRIter relement_iterator;

      struct L_CONTEXT
      {
        int nDepth;
        _TIter itMember;
        _TIter itEnd;
        L_CONTEXT& set(int _nDepth, _TIter& _itMember, _TIter& _itEnd)
        {
          nDepth = _nDepth;
          itMember = _itMember;
          itEnd = _itEnd;
          return *this;
        }
      };

      struct L_ACONTEXT
      {
        int nDepth;
        element_iterator itElement;
        element_iterator itEnd;
        L_ACONTEXT& set(int _nDepth, element_iterator& _itElement, element_iterator& _itEnd)
        {
          nDepth = _nDepth;
          itElement = _itElement;
          itEnd = _itEnd;
          return *this;
        }
      };

      typedef cllist<L_ACONTEXT> IterList;
      typedef clstack<L_CONTEXT> RecusiveStack;

      IterList sDynArray;
      RecusiveStack sStack;
      L_CONTEXT c;
      L_ACONTEXT c2;

      sStack.push(c.set(0, itBegin, itEnd));

NEW_LOOP:
      while(1) {
        _TIter& itMember = sStack.top().itMember;
        const int nDepth = sStack.top().nDepth;
        while(itMember != sStack.top().itEnd) {

          // 动态数组先发一个关于它的通知，并记录下来在之后遍历
          if(itMember.pVarDesc->IsDynamicArray()) {
            ASSERT(itMember.index == (GXUINT)-1);
            if(itMember.child_buffer()) {
              auto _arr_begin = itMember.array_begin();
              auto _arr_end = itMember.array_end();
              sDynArray.push_back(c2.set(nDepth + 1, _arr_begin, _arr_end));
            }
            _varFunc(1, itMember, nDepth);
            ++itMember;
            continue;
          }

          switch(itMember.pVarDesc->GetTypeCategory())
          {
          case T_STRUCT:
            // 分解结构体
            if(itMember.pVarDesc->nCount > 1) {
              // 这里是倒序迭代入栈,正序出栈
              const relement_iterator itElementEnd = itMember.rarray_end();
              auto itElement = itMember.rarray_begin();
              ++itMember; // 这个严重依赖于上下代码，特别注意
              for(; itElement != itElementEnd; ++itElement) {
                auto _it_begin = itElement.begin();
                auto _it_end = itElement.end();
                sStack.push(c.set(nDepth + 1, _it_begin, _it_end));
              }
              goto NEW_LOOP;
            }
            else {    
              auto _it_begin = itMember.begin();
              auto _it_end = itMember.end();
              c.set(nDepth + 1, _it_begin, _it_end);
              ++itMember; // 这个严重依赖于上面的代码，特别注意
              sStack.push(c);
              goto NEW_LOOP;
            }
            break;

          default:
            // 非结构体类型直接调用回调
            if(itMember.pVarDesc->nCount > 1) {
              const element_iterator itElementEnd = itMember.array_end();
              for(auto itElement = itMember.array_begin(); itElement != itElementEnd; ++itElement)
              {
                _varFunc(0, itElement, nDepth);
              }
            }
            else {
              _varFunc(0, itMember, nDepth);
            }
            break;
          }
          ++itMember;
        } // while

        sStack.pop();        

        if(sStack.empty()) // 如果堆栈空了，表示基于这一个缓冲的所有变量都已经遍历完毕
        {
          while( ! sDynArray.empty())
          {
            L_ACONTEXT& ac = sDynArray.front();

            if(ac.itElement.pVarDesc->GetTypeCategory() == T_STRUCT) {
              if(ac.itElement != ac.itEnd) {
                auto _it_begin = ac.itElement.begin();
                auto _it_end = ac.itElement.end();
                sStack.push(c.set(nDepth, _it_begin, _it_end));
                ++ac.itElement;
                goto NEW_LOOP;
              }
              else {
                sDynArray.pop_front();
              }
            }
            else {
              for(; ac.itElement != ac.itEnd; ++ac.itElement)
              {
                _varFunc(0, ac.itElement, ac.nDepth);
              }
              sDynArray.pop_front();
            }

          } // while( ! sDynArray.empty())
          break; // break while(1)
        }
      } // while(1);

      ASSERT(sDynArray.empty() && sStack.empty());

    } // EnumerateVariables2

  } // DataPoolUtility

} // namespace Marimo

#endif // #ifndef _MARIMO_DATA_POOL_ITERATOR_H_