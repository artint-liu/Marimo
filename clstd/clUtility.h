#ifndef _CL_UTILITY_
#define _CL_UTILITY_

#define clrand    rand
#define clrand_s  rand_s

template<typename _T1, typename _T2>
inline _T2 clClamp(CLCONST _T1 nMin, CLCONST _T1 nMax, _T2 *pValue)
{
  if(*pValue < nMin)
    *pValue = (_T2)nMin;
  else if(*pValue > nMax)
    *pValue = (_T2)nMax;
  return *pValue;
}

template<typename _Ty>
inline _Ty clClamp(CLCONST _Ty nMin, CLCONST _Ty nMax, _Ty Value)
{
  if(Value < nMin)
    return (_Ty)nMin;
  else if(Value > nMax)
    return (_Ty)nMax;
  return Value;
}

template<typename _T1, typename _T2>
inline _T1 clLerp(CLCONST _T1 First, CLCONST _T1 Second, _T2 Value)
{
  return (_T1)((Second - First) * Value) + First;
}

template<typename _T1, typename _T2>
inline _T1 clMax(_T1 a, _T2 b)
{
  return a > b ? a : b;
}

template<typename _T1, typename _T2>
inline _T1 clMin(_T1 a, _T2 b)
{
  return (_T1)(a < b ? a : b);
}

template<typename _T>
inline void clSwap(_T& a, _T& b)
{
  _T t = a;
  a = b;
  b = t;
}

template<typename _T>
inline void clSwapX(_T& a, _T& b)
{
  a = a ^ b;
  b = a ^ b;
  a = a ^ b;
}

template<typename _T>
inline void clSwap32(_T& a, _T& b)
{
  (*(u32*)&a) = ((*(u32*)&a) ^ (*(u32*)&b));
  (*(u32*)&b) = ((*(u32*)&a) ^ (*(u32*)&b));
  (*(u32*)&a) = ((*(u32*)&a) ^ (*(u32*)&b));
}

template <typename _Ty>
inline _Ty cuberoot(_Ty f)
{
  const _Ty y = _Ty(1.0 / 3.0);
  return f < 0 ? -pow(-f, y) : pow(f, y);
}

namespace clstd
{
  template<typename _Ty>
  inline _Ty saturate(_Ty Value)
  {
    if(Value < (_Ty)0)
      return (_Ty)0;
    else if(Value > (_Ty)1)
      return (_Ty)1;
    return Value;
  }

  template<typename _Ty>
  inline b32 approximate(_Ty a, _Ty b, _Ty epsilon)
  {
    return (fabs(a - b) < epsilon);
  }

  template<typename _Ty, typename _TCh>
  _vector2<_Ty> strtovec2i(const _TCh* str)
  {
    _vector2<_Ty> v2(0);
    v2.x = (_Ty)_xstrtoi<i32>(str);
    str = strchrT(str, ',');

    if(str) {
      v2.y = (_Ty)_xstrtoi<i32>(++str);
      str = strchrT(str, ',');
    }

    return v2;
  }

  template<typename _Ty, typename _TCh>
  _vector3<_Ty> strtovec3i(const _TCh* str)
  {
    _vector3<_Ty> v3(0);
    v3.x = (_Ty)_xstrtoi<i32>(str);
    str = strchrT(str, ',');

    if(str) {

      v3.y = (_Ty)_xstrtoi<i32>(++str);
      str = strchrT(str, ',');

      if(str) {
        v3.z = (_Ty)_xstrtoi<i32>(++str);
      }
    }

    return v3;
  }

  template<typename _TCh>
  float2 strtovec2f(const _TCh* str)
  {
    float2 v2(0.0f);
    v2.x = (float)_xstrtof(str);
    str = strchrT(str, ',');

    if(str) {
      v2.y = (float)_xstrtof(++str);
      str = strchrT(str, ',');
    }

    return v2;
  }

  template<typename _TCh>
  float3 strtovec3f(const _TCh* str)
  {
    float3 v3(0.0f);
    v3.x = (float)_xstrtof(str);
    str = strchrT(str, ',');

    if(str) {

      v3.y = (float)_xstrtof(++str);
      str = strchrT(str, ',');

      if(str) {
        v3.z = (float)_xstrtof(++str);
      }
    }

    return v3;
  }

  inline CLDWORD MakeFourCC(CLBYTE ch0, CLBYTE ch1, CLBYTE ch2, CLBYTE ch3)
  {
    return CLMAKEFOURCC(ch0, ch1, ch2, ch3);
  }

  template <class _TElement>
  void BubbleSort(_TElement* array, int count)
  {
    for(int i = 0; i < count - 1; i++)
    {
      for(int n = 0; n < count - i - 1; n++)
      {
        ASSERT(n + 1 < count);
        if(array[n].SortCompare(array[n + 1]))
        {
          array[n].SortSwap(array[n + 1]);
        }
      }
    }
  }

  template <class _TElement>
  void QuickSortClassic(_TElement* array, int start, int end)  // ѭ��������end
  {
    if(start >= end)
      return;

    const _TElement& x = array[end - 1];
    int i = start;
    for(int j = start; j < end; j++)
    {
      if(x.SortCompare(array[j])) {
        if(i != j) {
          array[i].SortSwap(array[j]);
        }
        i++;
      }
    }
    if(i != end - 1)
    {
      array[i].SortSwap(array[end - 1]);
    }
    QuickSort(array, start, i);
    QuickSort(array, i + 1, end);
  }

  template <class _TElement, typename _Ty>
  void QuickSort(_TElement* array, _Ty start, _Ty end)
  {
    struct CONTEXT{
      _Ty s, e;
    };
    clstack<CONTEXT> aStack;
    CONTEXT ctx;
    while(1) {
      const _TElement& x = array[end - 1];
      _Ty i = start;
      for(_Ty j = start; j < end - 1; j++)
      {
        if(x.SortCompare(array[j]))
        {
          if(j != i)
            array[i].SortSwap(array[j]);
          i++;
        }
      }
      if(i != end - 1)
        array[i].SortSwap(array[end - 1]);

      if(start < i) {
        ctx.s = start;
        ctx.e = i;
        aStack.push(ctx);
      }
      if(i + 1 < end) {
        start = i + 1;
        continue;
      }
      if(aStack.size() > 0) {
        start = aStack.top().s;
        end   = aStack.top().e;
        aStack.pop();
      }
      else break;
    };
  }
  //////////////////////////////////////////////////////////////////////////
  //************************************
  // Method:    ResolveString �ֽ��ַ�����һ��������
  // Parameter: const _String& str Ҫ�ֽ���ַ���
  // Parameter: _TCh ch ʹ�õķָ���
  // Parameter: _StringArray& aStr ������󣬿�����vector��list��
  //************************************
  template <class _String, typename _TCh, class _StringArray>
  void ResolveString(const _String& str, _TCh ch, _StringArray& aStr)
  {
    size_t start = 0;
    size_t pos;
    size_t len = str.GetLength();
    do {
      pos = str.Find(ch, start);
      if(pos == _String::npos) {
        pos = len;
      }
      _String strPart = str.SubString(start, pos - start);
      aStr.push_back(strPart);
      start = pos + 1;
    }while(pos != len);
  }

  //////////////////////////////////////////////////////////////////////////
  //
  // ��Ч�����ض����ŷֽ��ַ�������
  // �����޸�Ŀ���ַ����ⲻ���������Ķ���/�ڴ����
  //
  template <class _TString>
  class StringCutter
  {
    typedef typename _TString::LPCSTR T_LPCSTR;
    typedef typename _TString::TChar  T_CHAR;
    //size_t      m_start;
    size_t      m_pos;
    size_t      m_length;
    T_LPCSTR    m_str;
      
  public:
    StringCutter() : m_pos(0), m_str(""), m_length(0) {}
    StringCutter(T_LPCSTR str, size_t len = 0) : m_pos(0), m_str(str), m_length(len) {}

    // TODO: ����һ�������ַ�����ַ�ͳ��ȣ����⹹��string�ķ���

    _TString& Cut(_TString& strOut, T_CHAR ch)
    {
      T_CHAR c;
      const size_t start = m_pos;
      while((m_length == 0 || m_pos < m_length) && (c = m_str[m_pos]) != '\0') {
        if(c == ch) {
          break;
        }
        m_pos++;
      }

      strOut.Clear();
      strOut.Append(&m_str[start], m_pos - start);
      m_pos = (c == '\0' || (m_length != 0 && m_pos == m_length)) ? m_pos : m_pos + 1;
      return strOut;
    }

    void Set(T_LPCSTR str, size_t len = 0)
    {
      m_pos = 0;
      m_str = str;
      m_length = len;
    }

    b32 IsEndOfString()
    {
      return m_str[m_pos] == '\0' || (m_pos == m_length && m_length != 0);
    }

    void Reset()
    {
      m_pos = 0;
    }
  };  

  //////////////////////////////////////////////////////////////////////////

  template <class _String, class _TCh>
  void AssembleString(_String& str, _TCh ch, const clvector<_String>& aStr)
  {
    str.Empty();

    typename clvector<_String>::const_iterator it = aStr.begin();
    for(;it != aStr.end(); ++it) {
        str += (*it);
        str.Append(ch);
    }
  }

  template <typename _TCh, class _StringT>
  void ParseCommandLine(const _TCh* szCommandLine, clvector<_StringT>& aStrings)
  {
    const _TCh* c = szCommandLine;
    while(*c == ' ') {
      c++;
    }

    _StringT str;

    while(*c != '\0')
    {
      if(*c == '\"') {
        c++;
        if(*c == '\0') {
          goto FINAL_RET;
        }

        const _TCh* e = clstd::strchrT(c, '\"');
        if(e == NULL) {
          goto FINAL_RET;
        }
        str.Append(c, e - c);
        c = e;
      }
      else if(*c == 0x20) {
        if(str.IsNotEmpty()) {
          aStrings.push_back(str);
          str.Clear();
        }
      }
      else {
        str.Append(*c);
      }
      c++;
    }

FINAL_RET:
    if(str.IsNotEmpty()) {
      aStrings.push_back(str);
    }
  }

  template <class _String>
  void TranslateEscapeCharacter(_String& str) // ����/���
  {
    int i = 0;
    int d = 0;
    typename _String::TChar* pStr = str.GetBuffer();
    do {
      if(pStr[i] == '\\') {
        i++;
        switch(pStr[i])
        {
        case 'n':
          pStr[d++] = '\n'; break;
        case 'r':
          pStr[d++] = '\r'; break;
        case 't':
          pStr[d++] = '\t'; break;
        case '\\':
          pStr[d++] = '\\'; break;
        default:
          pStr[d++] = pStr[i];  break;
        }
      }
      else {
        pStr[d++] = pStr[i];
      }
    } while(pStr[i++] != '\0');

    str.ReleaseBuffer();
  }

  u32 GenerateCRC32(CLBYTE* pData, u32 nLength);

  template<typename _Ty>
  inline size_t hash_value(const _vector2<_Ty>& v2)
  {
    return clstd::GenerateCRC32((CLBYTE*)&v2, sizeof(_vector2<_Ty>));
  }

  template<typename _Ty>
  inline size_t hash_value(const _vector3<_Ty>& v3)
  {
    return clstd::GenerateCRC32((CLBYTE*)&v3, sizeof(_vector3<_Ty>));
  }

  template<typename _Ty>
  inline size_t hash_value(const _vector4<_Ty>& v4)
  {
    return clstd::GenerateCRC32((CLBYTE*)&v4, sizeof(_vector4<_Ty>));
  }


  template<typename _Ty>
  _Ty RoundupPowOfTwo(_Ty num)
  {
    if(num == 0) {
      return 0;
    }
    int i = 1;
    while((_Ty)(1 << i) < num) i++;
    return (1 << i);
  }

  class Rand
  {
  private:
    unsigned int m_holdrand;

  public:
    void srand(unsigned int seed)
    {
      m_holdrand = seed;
    }

    int rand()
    {
      return( ((m_holdrand = m_holdrand * 214013L
        + 2531011L) >> 16) & 0x7fff );
    }

    float randf() // [0, 1]
    {
#ifdef _DISABLE_IEEE754
      const float fRand = (float)rand() * (1.0f / (float)0x7fff);
      ASSERT(fRand <= 1.0f);
      return fRand;
#else
      // from cloudwu/ffr.c
      union {
        u32 d;
        float f;
      }u;
      u.d = ((rand() & 0x7fff) << 8) | 0x3f800000;
      return u.f - 1.0f;
#endif // #ifdef _DISABLE_IEEE754
    }

    float randf2() // [-1.0, 1.0]
    {
#ifdef _DISABLE_IEEE754
      const float fMask = (float)((0x7fff >> 1) + 1);
      const float fRand = ((float)rand() - fMask) * (1.0f / (float)fMask);
      ASSERT(fRand >= -1.0f && fRand <= 1.0f);
      return fRand;
#else
      // from cloudwu/ffr.c
      union {
        u32 d;
        float f;
      }u;
      u.d = ((rand() & 0x7fff) << 8) | 0x40000000;
      return u.f - 3.0f;
#endif // #ifdef _DISABLE_IEEE754
    }
  };

#if defined(_ANDROID) || defined(_IOS)
	typedef union _LARGE_INTEGER {
		struct {
			CLDWORD LowPart;
			CLLONG HighPart;
		}u;
		CLLONGLONG QuadPart;
	} LARGE_INTEGER;
#endif

  class TimeTrace
  {
    LARGE_INTEGER m_Frequency;
    LARGE_INTEGER m_Begin;
    LARGE_INTEGER m_End;

  public:
    TimeTrace();
    void                Begin();
    CLCONST TimeTrace&  End();
    double              GetDeltaTime() CLCONST;
    double              Dump(CLLPCSTR szName) CLCONST;
  };

  class ScopeTimeTrace
  {
    CLLPCSTR name;  // �����ʵ��clStringA
    TimeTrace tt;
    double* prTotalTime;
  public:
    //ScopeTimeTrace(CLLPCSTR szName);
    ScopeTimeTrace(CLLPCSTR szName, double* prTotalTime = NULL);
    ~ScopeTimeTrace();
  };

  u64 GetTime64(); // Milli Seconds
  u32 GetNumberOfProcessors();  // ��ô���������

  clStringA ToStringA(const float4x4& m);
  clStringA ToStringA(const float3x3& m);
  clStringA ToStringA(const float3& v);
  clStringA ToStringA(const float4& v);

  extern int s_aPrimeNum[];
} // namespace clstd

#endif // _CL_UTILITY_