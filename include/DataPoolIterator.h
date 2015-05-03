#ifndef _MARIMO_DATA_POOL_ITERATOR_H_
#define _MARIMO_DATA_POOL_ITERATOR_H_

#ifndef _MARIMO_DATA_POOL_H_
#error ������֮ǰ����"DataPool.h"�ļ�
#endif // #ifndef _MARIMO_DATA_POOL_H_

// Data Pool ������

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
    // ����������
    //
    // �ڵ��������в���¼��Դ���ƣ������޷���õ�ǰ����ʽ���ϲ��ĸ�������õ�
    // ��Ϊû���ַ������ݹ��̣����������Ч����΢��Щ

    // ��׼����
    // ������ڱ�������ͬ���ı���
    // ����ǽṹ���ͣ�ʹ��begin���Ի�ýṹ���һ����Ա�����ĵ���
    // �ṹ������ʹ��begin�������������һ��Ԫ���µĳ�Ա�������б���
    // ע����ڿյĶ�̬���飬ToVariable�������صĿ�����һ����Ч����
    // ������������ÿ��Ԫ�ؽ��е�����ʹ��array_begin����rarray_begin����
    struct GXDLL iterator
    {
      typedef const DataPool::VARIABLE_DESC*  LPCVD;
      //typedef DataPoolUtility::element_iterator  element_iterator;
      typedef DataPoolUtility::element_reverse_iterator relement_iterator;

      DataPool*         pDataPool;
      //clStringA         ParentName;
      LPCVD             pVarDesc;
      clBufferBase*     pBuffer;
      GXUINT            nOffset;  // Parent��buffer���ƫ�ƣ�ȫ�ֱ���һ����0����Ա�����ǳ�Ա����struct��buffer��ƫ��
      GXUINT            index;    // �����Ա����,(GXUINT)-1��ʾ��������������������ֵ��ʾ����Ԫ��


      iterator& operator++()
      {
        ++pVarDesc;
        return *this;
      }

      GXBOOL operator==(const iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && nOffset == it.nOffset);
        // �����������nOffset��ͬ������pVarDesc��ͬ
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
      GXUINT            offset          () const; // ��Buffer�еľ���ƫ��, ����Ļ��ǵ�һ��Ԫ�ؿ�ʼ��λ��
      element_iterator  array_begin     () const; // ���������Ļ�����������Ԫ�صĵ���������һԪ����������ᱨ��
      element_iterator  array_end       () const;
      relement_iterator rarray_begin    () const; // ����ķ�������������һ��Ԫ�ؿ�ʼ
      relement_iterator rarray_end      () const;
      GXUINT            array_length    () const;
      clBufferBase*     child_buffer    () const; // ��̬���鷵���������еĻ��������Ƕ�̬���鷵��NULL

      DataPoolVariable  ToVariable      () const;
      DataPoolVariable& ToVariable      (DataPoolVariable& var) const;
      //GXUINT            AbsoluteOffset  () const;
      DataPool::LPCSTR  VariableName    () const;
      //clStringA         FullNameA       () const;
      //clStringA&        FullNameA       (clStringA& str) const;
      DataPool::LPCSTR  TypeName        () const;
      GXBOOL            IsArray         () const;

      void              StepArrayMember (iterator& it); // ��Ա����������it��this�ṹ�������һ����Ա������
      //                                                           ���������it����Ϊ������һ��Ԫ�صĳ�Ա����

    protected:
      void first_child    (iterator& it) const;
      void first_element  (element_iterator& it) const;
      clBufferBase* child_buffer_unsafe() const;
    };

    // ����Ԫ�ص���
    // iterator::array_begin�������Եõ��������ĵ�һ��Ԫ��
    // �����һԪ���飬Ԫ�ص�����Ȼ��Ч��������ѭ��ֻ����һ��
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

    // ����Ԫ�ط������
    // ���Ǵ���������һ��Ԫ�ؿ�ʼ��ǰ����
    struct GXDLL element_reverse_iterator : element_iterator
    {
      element_reverse_iterator& operator++();
    };

    //////////////////////////////////////////////////////////////////////////

    // ����������
    // ������������м�¼�˸�ϵVariable�����ֺ����������Եõ���������
    // FullNameA�õ�������ʹ��DataPool::QueryByExpression�ӿڻ�õı����������
    // ToVariable�õ��ĵı���������ͬһ����
    // ע�⣺DataPool::QueryByExpression������ñ�������������ǿյĶ�̬���飬����
    //      ��Ϊ��̬���鴴����Ҫ�ķ��ʻ����������������ӿհ׶�̬�����ת���õ���
    //      ��������������Ч�ģ�������ʹ��֮ǰҪʹ��DataPoolVariable::IsValid�ж�
    //      ���Ƿ���Ч��

    struct GXDLL named_iterator : iterator
    {
      typedef const DataPool::VARIABLE_DESC*  LPCVD;
      //typedef DataPoolUtility::named_element_iterator  element_iterator;
      //typedef DataPoolUtility::named_element_reverse_iterator relement_iterator;

      clStringA     ParentName; // �ϲ������ȫ��

      named_iterator& operator++()
      {
        ++pVarDesc;
        return *this;
      }

      GXBOOL operator==(const named_iterator& it) const
      {
        ASSERT(pDataPool == it.pDataPool && pBuffer == it.pBuffer && nOffset == it.nOffset);
        // �����������nOffset��ͬ������pVarDesc��ͬ
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
      //GXUINT            offset          () const; // ��Buffer�еľ���ƫ��, ����Ļ��ǵ�һ��Ԫ�ؿ�ʼ��λ��
      named_element_iterator         array_begin     () const; // ���������Ļ�����������Ԫ�صĵ���������һԪ����������ᱨ��
      named_element_iterator         array_end       () const;
      named_element_reverse_iterator rarray_begin    () const; // ����ķ�������������һ��Ԫ�ؿ�ʼ
      named_element_reverse_iterator rarray_end      () const;
      //GXUINT            array_length    () const;
      //clBufferBase*     child_buffer    () const; // ��̬���鷵���������еĻ��������Ƕ�̬���鷵��NULL

      //DataPoolVariable  ToVariable      () const;
      //DataPoolVariable& ToVariable      (DataPoolVariable& var) const;
      //GXUINT            AbsoluteOffset  () const;
      //DataPool::LPCSTR  VariableName    () const;
      clStringA         FullNameA       () const;
      clStringA&        FullNameA       (clStringA& str) const;
      //DataPool::LPCSTR  TypeName        () const;
      //GXBOOL            IsArray         () const;

      void              StepArrayMember (named_iterator& it); // ��Ա����������it��this�ṹ�������һ����Ա������
      //                                                           ���������it����Ϊ������һ��Ԫ�صĳ�Ա����

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

    // �ṹ�����ȵı������������
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
    } // EnumerateVariables

    //////////////////////////////////////////////////////////////////////////

    // ���������ȵı��������ȱ�����ͬһ�������еı���
    // �ص��лᰴ�ձ������ڻ���������
    // ����ص��еõ���һ����̬���飬��һ������Ϊ1����ô��һ���Ƕ�̬���飬������֮����յ������̬�����µ����б���
    // �ص��е�һ������Ϊ0ʱ�����б���һ�����Ƿǽṹ�����ͱ������ṹ�����ͻ�����������б��ֽ⡣
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

          // ��̬�����ȷ�һ����������֪ͨ������¼������֮�����
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
            // �ֽ�ṹ��
            if(itMember.pVarDesc->nCount > 1) {
              // �����ǵ��������ջ,�����ջ
              const relement_iterator itElementEnd = itMember.rarray_end();
              auto itElement = itMember.rarray_begin();
              ++itMember; // ����������������´��룬�ر�ע��
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
              ++itMember; // �����������������Ĵ��룬�ر�ע��
              sStack.push(c);
              goto NEW_LOOP;
            }
            break;

          default:
            // �ǽṹ������ֱ�ӵ��ûص�
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

        if(sStack.empty()) // �����ջ���ˣ���ʾ������һ����������б������Ѿ��������
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