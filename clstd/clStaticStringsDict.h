#ifndef _CLSTD_STATICSTRINGSDICT_H_
#define _CLSTD_STATICSTRINGSDICT_H_

// 使用穷举法探测适合一组不会改变的字符串的查找方法

// 模板原型:
// [](clsize index, CLLPCSTR* str, clsize* len) -> b32 {}

namespace clstd
{
  class StaticStringsDict
  {
  public:
    typedef clvector<int> IntArray;
    typedef size_t len_t;

  public:
    enum HashType {
      HashType_Failed,
      HashType_Local,   // 只对局部hash
      HashType_String,  // 字符串hash
    };

    HashType  eType;
    len_t     nBucket;  // 索引表是两倍桶数量
    int       nPos;     // HashType_Local 才有用

  protected:
    len_t     m_count;
    IntArray  m_indices;
    IntArray  m_aCountsTab;
    //_Fn       m_fn;

    template<class _Fn>
    len_t GetMaxLength(_Fn fn)
    {
      len_t nMaxLength = 0;
      len_t len;
      CLLPCSTR str;

      for(len_t i = 0; i < m_count; i++) {
        if( ! fn(i, &str, &len) || (len == 0)) {
          break;
        }
        nMaxLength = clMax(nMaxLength, len);
      }
      return nMaxLength;
    }

    template<class _Fn>
    b32 TestStep(int n, len_t bucket, _Fn fn)
    {
      len_t len;
      CLLPCSTR str;

      // IntArray 可以这么清！
      memset(&m_aCountsTab.front(), 0, sizeof(int) * m_aCountsTab.size());
      memset(&m_indices.front(), -1, sizeof(int) * m_indices.size());

      for(len_t i = 0; i < m_count; i++)
      {
        if( ! fn(i, &str, &len) || (len == 0)) {
          break;
        }

        len_t local_hash = HashChar(str, len, n);
        //TRACE("hash(%s,%d,%d)=%d %% %d=%d\n", str, len, n, local_hash, bucket, local_hash % bucket);

        ASSERT(local_hash != 0);
        local_hash %= bucket;

        if(m_aCountsTab[local_hash] > 1) {
          return FALSE;
        }

        if(m_aCountsTab[local_hash]++ == 0) {
          m_indices[local_hash * 2] = (int)i;
        } else {
          m_indices[local_hash * 2 + 1] = (int)i;
        }
      }

      return TRUE;
    }

    template<class _Fn>
    b32 TestStep(len_t bucket, _Fn fn)
    {
      len_t len;
      CLLPCSTR str;

      // IntArray 可以这么清！
      memset(&m_aCountsTab.front(), 0, sizeof(int) * m_aCountsTab.size());
      memset(&m_indices.front(), -1, sizeof(int) * m_indices.size());

      for(len_t i = 0; i < m_count; i++)
      {
        if( ! fn(i, &str, &len) || (len == 0)) {
          break;
        }

        len_t local_hash = clstd::HashStringT(str, len) % bucket;
        //TRACE("hash(%s,%d)=%d %% %d=%d\n", str, len, clstd::HashStringT(str, len), bucket, local_hash);

        if(m_aCountsTab[local_hash] > 1) {
          return FALSE;
        }

        if(m_aCountsTab[local_hash]++ == 0) {
          m_indices[local_hash * 2] = (int)i;
        } else {
          m_indices[local_hash * 2 + 1] = (int)i;
        }
      }

      return TRUE;
    }
  
  public:
    StaticStringsDict()
    {}

    StaticStringsDict(len_t count)
      : m_count(count)
      //, m_fn(fn)
    {
      Reset(count);
    }

    void Reset(len_t count)
    {
      m_count = count;
      m_indices.insert(m_indices.end(), count * 4, -1);
      m_aCountsTab.insert(m_aCountsTab.end(), count * 4, 0);
    }
    
    template<class _Fn>
    b32 TestHashable(_Fn fn)
    {
      len_t nMaxLength = GetMaxLength(fn);
      len_t nDualCount = m_count * 2;

      // 局部hash
      for(len_t bucket = m_count; bucket < nDualCount; bucket++)
      {
        for(len_t n = 1; n < nMaxLength; n++) {
          if(TestStep(-(int)n, bucket, fn)) {
            nPos = -(int)n;
            nBucket = bucket;
            eType = HashType_Local;
            return TRUE;
          }
        }

        for(len_t n = 0; n < nMaxLength; n++) {
          if(TestStep((int)n, bucket, fn)) {
            nPos = (int)n;
            nBucket = bucket;
            eType = HashType_Local;
            return TRUE;
          }
        }
      } // for bucket

      // 字符串hash
      for(len_t bucket = m_count; bucket < nDualCount; bucket++) {
        if(TestStep(bucket, fn)) {
          nPos = -1;
          nBucket = bucket;
          eType = HashType_String;
          return TRUE;
        }
      }

      eType = HashType_Failed;
      return FALSE;
    }

    template<class _TList>
    void Copy(_TList& _list)
    {
      _list.insert(_list.end(), m_indices.begin(), m_indices.begin() + nBucket * 2);
    }

    //////////////////////////////////////////////////////////////////////////

    static len_t HashChar(CLLPCSTR str, len_t len, int pos)
    {
      len_t val = 0;
      if(len == 1) {
        val = (len_t)str[0];
      }
      else if(pos >= 0) // 从字符串开始位置
      {
        val = (len_t)(((len_t)pos + 1 < len) ? (*(short*)&str[pos]) : str[len - 2]);
      }
      else { // 从字符串结尾位置
        pos = -pos;
        val = (len_t)(((len_t)pos <= len) ? (*(short*)&str[len - pos]) : str[0]);
      }

      return val ^ (len << 5);
    }
    
    //static len_t HashChars4(CLLPCSTR str, len_t len, int pos)
    //{
    //  if(len == 1) {
    //    return (len_t)str[0];
    //  }
    //  else if(len == 2) {
    //    return (len_t)((str[1] << 8) | str[0]);
    //  }
    //  else if(len == 3) {
    //    return (len_t)((str[2] << 16) | (str[1] << 8) | str[0]);
    //  }
    //  else if(len == 4) {
    //    return (len_t)((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
    //  }

    //  if(pos >= 0)
    //  {
    //    return (len_t)(((len_t)pos + 4 < len)
    //      ? (*(short*)&str[pos]) : str[len - 5]);
    //  }
    //  else {
    //    pos = -pos;
    //    return (len_t)(((len_t)pos <= len)
    //      ? (*(short*)&str[len - pos]) : str[0]);
    //  }
    //}
  };
} // namespace clstd

#endif // _CLSTD_STATICSTRINGSDICT_H_