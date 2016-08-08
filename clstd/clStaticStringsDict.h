#ifndef _CLSTD_STATICSTRINGSDICT_H_
#define _CLSTD_STATICSTRINGSDICT_H_

// ʹ����ٷ�̽���ʺ�һ�鲻��ı���ַ����Ĳ��ҷ���

// ģ��ԭ��:
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
      HashType_Local,   // ֻ�Ծֲ�hash
      HashType_String,  // �ַ���hash
    };

    HashType  eType;
    len_t     nBucket;
    int       nPos; // HashType_Local ������

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

      // IntArray ������ô�壡
      memset(&m_aCountsTab.front(), 0, sizeof(int) * m_aCountsTab.size());
      memset(&m_indices.front(), 0, sizeof(int) * m_indices.size());

      for(len_t i = 0; i < m_count; i++)
      {
        if( ! fn(i, &str, &len) || (len == 0)) {
          break;
        }

        len_t local_hash = HashChar(str, len, n);
        ASSERT(local_hash != 0);
        local_hash %= bucket;

        if(m_aCountsTab[local_hash] > 1) {
          return FALSE;
        }

        m_aCountsTab[local_hash]++;
        m_indices[local_hash] = i;
      }

      return TRUE;
    }

    template<class _Fn>
    b32 TestStep(len_t bucket, _Fn fn)
    {
      len_t len;
      CLLPCSTR str;

      // IntArray ������ô�壡
      memset(&m_aCountsTab.front(), 0, sizeof(int) * m_aCountsTab.size());
      memset(&m_indices.front(), 0, sizeof(int) * m_indices.size());

      for(len_t i = 0; i < m_count; i++)
      {
        if( ! fn(i, &str, &len) || (len == 0)) {
          break;
        }

        len_t local_hash = clstd::HashStringT(str, len) % bucket;

        if(m_aCountsTab[local_hash] > 1) {
          return FALSE;
        }

        m_aCountsTab[local_hash]++;
        m_indices[local_hash] = i;
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
      m_indices.insert(m_indices.end(), count * 2, 0);
      m_aCountsTab.insert(m_aCountsTab.end(), count * 2, 0);
    }
    
    template<class _Fn>
    b32 TestHashable(_Fn fn)
    {
      len_t nMaxLength = GetMaxLength();
      len_t nDualCount = m_count * 2;

      // �ֲ�hash
      for(len_t bucket = m_count; bucket < nDualCount; bucket++)
      {
        for(len_t n = 1; n < nMaxLength; n++) {
          if(TestStep(-(int)n, bucket)) {
            nPos = -(int)n;
            nBucket = bucket;
            eType = HashType_Local;
            return TRUE;
          }
        }

        for(len_t n = 0; n < nMaxLength; n++) {
          if(TestStep((int)n, bucket)) {
            nPos = n;
            nBucket = bucket;
            eType = HashType_Local;
            return TRUE;
          }
        }
      } // for bucket

      // �ַ���hash
      for(len_t bucket = m_count; bucket < nDualCount; bucket++) {
        if(TestStep(bucket)) {
          nPos = -1;
          nBucket = bucket;
          eType = HashType_String;
          return TRUE;
        }
      }

      eType = HashType_Failed;
      return FALSE;
    }

    //////////////////////////////////////////////////////////////////////////

    static len_t HashChar(CLLPCSTR str, len_t len, int pos)
    {
      len_t val = 0;
      if(len == 1) {
        val = (len_t)str[0];
      }
      else if(pos >= 0)
      {
        val = (len_t)(((len_t)pos + 1 < len) ? (*(short*)&str[pos]) : str[len - 2]);
      }
      else {
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