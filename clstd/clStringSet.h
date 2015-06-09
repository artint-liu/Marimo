#ifndef _CLSTD_STRING_SET_H_
#define _CLSTD_STRING_SET_H_

// 这个类主要是用来
// 1.储存一个字符串集合
// 2.消除重复内容的字符串
// 3.产生一块字符串集合内存，并重定位原有指针

namespace clstd
{
  struct STRINGSETDESC
  {
    clsize offset;
    clsize index;
  };

  // 如果自己实现 _TMap 需要保证从迭代器取出的字符串是经过排序的
  template<class _TString, class _TMap = clmap<_TString, STRINGSETDESC> >
  class StringSetT : _TMap // 不用public继承
  {
    clsize m_nSpaceUsage; // 占用空间
  
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

      if(r.second) { // 加入成功，计算使用空间
        r.first->second.offset = m_nSpaceUsage;
        r.first->second.index  = _TMap::size() - 1;
        m_nSpaceUsage += ALIGN_4((sstr.GetLength() * sizeof(typename _TString::TChar)) + 1); // 4字节对齐
      }
      return r.first;
    }

    typename _TString::LPCSTR add(typename _TString::LPCSTR str)  // 添加字符串并返回有效地址
    {
      return insert(str)->first;
    }

    clsize offset(typename _TString::LPCSTR str) // 添加字符串并返回将来在缓存中的字节偏移
    {
      return insert(str)->second.offset;
    }

    clsize index(typename _TString::LPCSTR str) // 添加字符串并返回索引值，sort后可以在Table中定位新的索引和偏移
    {
      return insert(str)->second.index;
    }

    clsize buffer_size() const // 获得这个字符串集合所占用的空间, 字节数
    {
      ASSERT(ALIGN_4(m_nSpaceUsage) == m_nSpaceUsage); // 验证是四字节对齐的
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

    // 输出的table是原来字符串插入顺序经过排序后的新顺序和偏移
    // pTable[old_index].index  是排序后的顺序
    // pTable[old_index].offset 是排序后的新偏移
    clsize sort(STRINGSETDESC* pTable) // 要保证 pTable 有足够的空间储存，空间从count()获得
    {
      if(pTable == NULL) {
        return size();
      }

      clsize nSpaceUsage = 0;
      clsize nIndex = 0;
      for(auto it = _TMap::begin(); it != _TMap::end(); ++it)
      {
        STRINGSETDESC& sDesc = it->second;

        // 排序表 - 排序前的索引位置放置排序后的位置和偏移
        pTable[sDesc.index].index = nIndex;
        pTable[sDesc.index].offset = nSpaceUsage;

        // 映射数据 - 按照迭代器的顺序设置位置和偏移
        sDesc.index = nIndex;
        sDesc.offset = nSpaceUsage;
        nSpaceUsage += ALIGN_4(it->first.GetLength() + 1);
        nIndex++;
      }
      return size();
    }

    template<typename _Ty>
    b32 gather_offset(_Ty* pDest, clsize cbSize) const // cbSize用来校验写入空间,如果不够返回false
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
    typename _TString::LPCSTR gather(_TBuffer* pBuffer, clsize nOffset) // nOffset==-1表示在末尾追加,追加时会按照四字节对齐
    {
      auto start = (nOffset == -1) ? ALIGN_4(pBuffer->GetSize()) : nOffset;

      // 所需空间大于Buffer尺寸才执行Resize操作
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