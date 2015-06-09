#ifndef _CLSTD_STRING_SET_H_
#define _CLSTD_STRING_SET_H_

// �������Ҫ������
// 1.����һ���ַ�������
// 2.�����ظ����ݵ��ַ���
// 3.����һ���ַ��������ڴ棬���ض�λԭ��ָ��

namespace clstd
{
  struct STRINGSETDESC
  {
    clsize offset;
    clsize index;
  };

  // ����Լ�ʵ�� _TMap ��Ҫ��֤�ӵ�����ȡ�����ַ����Ǿ��������
  template<class _TString, class _TMap = clmap<_TString, STRINGSETDESC> >
  class StringSetT : _TMap // ����public�̳�
  {
    clsize m_nSpaceUsage; // ռ�ÿռ�
  
    //template<class _Pair>
    //void calcsize(const _TString& sstr, _Pair& r)
    //{
    //}

  public:
    StringSetT() : m_nSpaceUsage(0) {}

    typename _TMap::iterator insert(typename _TString::LPCSTR str)
    {
      const _TString sstr(str);
      STRINGSETDESC sDesc = {0, -1};
      auto r = _TMap::insert(clmake_pair(sstr, sDesc));
      //calcsize(sstr, r);

      if(r.second) { // ����ɹ�������ʹ�ÿռ�
        r.first->second.offset = m_nSpaceUsage;
        r.first->second.index  = _TMap::size() - 1;
        m_nSpaceUsage += ALIGN_4((sstr.GetLength() * sizeof(typename _TString::TChar)) + 1); // 4�ֽڶ���
      }
      return r.first;
    }

    typename _TString::LPCSTR add(typename _TString::LPCSTR str)  // ����ַ�����������Ч��ַ
    {
      return insert(str)->first;
    }

    clsize offset(typename _TString::LPCSTR str) // ����ַ��������ؽ����ڻ����е��ֽ�ƫ��
    {
      return insert(str)->second.offset;
    }

    clsize index(typename _TString::LPCSTR str) // ����ַ�������������ֵ��sort�������Table�ж�λ�µ�������ƫ��
    {
      return insert(str)->second.index;
    }

    clsize buffer_size() const // �������ַ���������ռ�õĿռ�, �ֽ���
    {
      ASSERT(ALIGN_4(m_nSpaceUsage) == m_nSpaceUsage); // ��֤�����ֽڶ����
      return m_nSpaceUsage;
    }

    typename _TMap::iterator find(typename _TString::LPCSTR str)
    {
      return _TMap::find(str);
    }

    typename _TMap::iterator begin() const
    {
      return _TMap::begin();
    }

    typename _TMap::iterator end()
    {
      return _TMap::end();
    }

    clsize size() const
    {
      return _TMap::size();
    }

    // �����table��ԭ���ַ�������˳�򾭹���������˳���ƫ��
    // pTable[old_index].index  ��������˳��
    // pTable[old_index].offset ����������ƫ��
    clsize sort(STRINGSETDESC* pTable) // Ҫ��֤ pTable ���㹻�Ŀռ䴢�棬�ռ��count()���
    {
      if(pTable == NULL) {
        return size();
      }

      clsize nSpaceUsage = 0;
      clsize nIndex = 0;
      for(auto it = _TMap::begin(); it != _TMap::end(); ++it)
      {
        STRINGSETDESC& sDesc = it->second;

        // ����� - ����ǰ������λ�÷���������λ�ú�ƫ��
        pTable[sDesc.index].index = nIndex;
        pTable[sDesc.index].offset = nSpaceUsage;

        // ӳ������ - ���յ�������˳������λ�ú�ƫ��
        sDesc.index = nIndex;
        sDesc.offset = nSpaceUsage;
        nSpaceUsage += ALIGN_4(it->first.GetLength() + 1);
        nIndex++;
      }
      return size();
    }

    template<typename _Ty>
    b32 gather_offset(_Ty* pDest, clsize cbSize) const // cbSize����У��д��ռ�,�����������false
    {
      if(cbSize < size() * sizeof(_Ty)) {
        return FALSE;
      }
      for(auto it = _TMap::begin(); it != _TMap::end(); ++it) {
        *pDest++ = it->second.offset;
      }
      return TRUE;
    }

    void gather(typename _TString::LPCSTR pDest) const
    {
      for(auto it = _TMap::begin(); it != _TMap::end(); ++it) {
        memcpy((CLLPBYTE)pDest + it->second.offset, (typename _TString::LPCSTR)it->first, 
          (it->first.GetLength() + 1) * sizeof(typename _TString::TChar));
      }
    }

    template<class _TBuffer>
    typename _TString::LPCSTR gather(_TBuffer* pBuffer, clsize nOffset) // nOffset==-1��ʾ��ĩβ׷��,׷��ʱ�ᰴ�����ֽڶ���
    {
      auto start = (nOffset == -1) ? ALIGN_4(pBuffer->GetSize()) : nOffset;

      // ����ռ����Buffer�ߴ��ִ��Resize����
      if(start + m_nSpaceUsage > pBuffer->GetSize()) {
        pBuffer->Resize(start + m_nSpaceUsage, FALSE);
      }

      auto ptr = (typename _TString::LPCSTR)((CLLPBYTE)pBuffer->GetPtr() + start);
      gather(ptr);
      return ptr;
    }
  };

  typedef StringSetT<clStringW>  StringSetW;
  typedef StringSetT<clStringA>  StringSetA;
} // namespace clstd

#endif // _CLSTD_STRING_SET_H_