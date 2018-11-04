#ifndef _CL_UTILITY_
#define _CL_UTILITY_

template<typename _T1, typename _T2>
inline _T2 clClamp(const _T1 nMin, const _T1 nMax, _T2 *pValue)
{
  ASSERT(nMin <= nMax);
  if(*pValue < nMin) {
    *pValue = (_T2)nMin;
  }
  else if(*pValue > nMax) {
    *pValue = (_T2)nMax;
  }
  return *pValue;
}

template<typename _Ty>
inline _Ty clClamp(const _Ty nMin, const _Ty nMax, _Ty Value)
{
  ASSERT(nMin <= nMax);
  if(Value < nMin) {
    return (_Ty)nMin;
  }
  else if(Value > nMax) {
    return (_Ty)nMax;
  }
  return Value;
}

template<typename _T1, typename _T2>
inline _T1 clLerp(const _T1 First, const _T1 Second, _T2 Value)
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
  template<typename _TCh>
  _TCh* strchrT(const _TCh *str, const i32 ch);

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
  inline b32 approximate(_Ty a, _Ty b, _Ty epsilon) // 约等于
  {
    return (fabs(a - b) < epsilon);
  }

  template<typename _Ty, typename _TCh>
  _vector2<_Ty> strtovec2i(const _TCh* str) // "a,b"字符串解析为向量
  {
    _vector2<_Ty> v2(0);
    v2.x = (_Ty)xtoi(str);
    str = strchrT(str, ',');

    if(str) {
      v2.y = (_Ty)xtoi(++str);
      str = strchrT(str, ',');
    }

    return v2;
  }

  template<typename _Ty, typename _TCh>
  _vector3<_Ty> strtovec3i(const _TCh* str) // "a,b,c"字符串解析为向量
  {
    _vector3<_Ty> v3(0);
    v3.x = (_Ty)xtoi(str);
    str = strchrT(str, ',');

    if(str) {

      v3.y = (_Ty)xtoi(++str);
      str = strchrT(str, ',');

      if(str) {
        v3.z = (_Ty)xtoi(++str);
      }
    }

    return v3;
  }

  template<typename _TCh>
  float2 strtovec2f(const _TCh* str) // "a,b"字符串解析为向量
  {
    float2 v2(0.0f);
    v2.x = (float)xtof(str);
    str = strchrT(str, ',');

    if(str) {
      v2.y = (float)xtof(++str);
      str = strchrT(str, ',');
    }

    return v2;
  }

  template<typename _TCh>
  float3 strtovec3f(const _TCh* str) // "a,b,c"字符串解析为向量
  {
    float3 v3(0.0f);
    v3.x = (float)xtof(str);
    str = strchrT(str, ',');

    if(str) {

      v3.y = (float)xtof(++str);
      str = strchrT(str, ',');

      if(str) {
        v3.z = (float)xtof(++str);
      }
    }

    return v3;
  }

  inline CLDWORD MakeFourCC(CLBYTE ch0, CLBYTE ch1, CLBYTE ch2, CLBYTE ch3) // character code
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
  void QuickSortClassic(_TElement* array, int start, int end)  // 循环不包含end
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

  // 二分查找
  // fn = [](A* a, B key) -> int{}
  template<class _Ty, class _TKey, class _TFunc>
  _Ty* BinarySearch(_Ty* pArrayBegin, _Ty* pArrayEnd, const _TKey& key, _TFunc fn)
  {
    _Ty* pBegin = pArrayBegin;
    _Ty* pEnd   = pArrayEnd;

    while(pBegin + 1 < pEnd)
    {
      _Ty* pMid = pBegin + ((pEnd - pBegin) >> 1);
      const int r = fn(pMid, key);
      if(r < 0) {
        pBegin = pMid;
      }
      else if(r > 0) {
        pEnd = pMid;
      }
      else {
        return pMid;
      }
    }

    if(pBegin == pArrayBegin && fn(pBegin, key) == 0) {
      return pBegin;
    }
    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////
  //
  // 二分查找插入位置（不执行插入动作）
  // bInsert == FALSE 表示键值已经存在，返回键值位置
  // bInsert != FALSE 表示键值不存在，返回插入位置
  // fn = [](A* a, B key) -> int{}
  //
  template<class _Ty, class _TKey, class _TFunc>
  _Ty* BinaryInsertPos(_Ty* pArrayBegin, _Ty* pArrayEnd, const _TKey& key, b32* bInsert, _TFunc fn)
  {
    _Ty* pBegin = pArrayBegin;
    _Ty* pEnd   = pArrayEnd;

    while(pBegin + 1 < pEnd)
    {
      _Ty* pMid = pBegin + ((pEnd - pBegin) >> 1);
      const int r = fn(pMid, key);
      if(r < 0) {
        pBegin = pMid;
      }
      else if(r > 0) {
        pEnd = pMid;
      }
      else {
        *bInsert = FALSE;
        return pMid;
      }
    }

    if(pBegin == pArrayBegin) {
      const int r = fn(pBegin, key);
      *bInsert = (r != 0);
      return r >= 0 ? pBegin : pEnd;
    }

    *bInsert = TRUE;
    return pEnd;
  }


  //////////////////////////////////////////////////////////////////////////
  //************************************
  // Method:    ResolveString 分解字符串到一个数组中
  // Parameter: const _String& str 要分解的字符串
  // Parameter: _TCh ch 使用的分隔符
  // Parameter: _StringArray& aStr 数组对象，可以是vector，list等
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
  // 高效按照特定符号分解字符串的类
  // 除了修改目标字符串外不会产生多余的对象/内存分配
  //
  template <class _TString>
  class StringCutter
  {
    typedef typename _TString::LPCSTR T_LPCSTR;
    typedef typename _TString::TChar  T_CHAR;
    //size_t      m_start;
    size_t      m_pos;
    T_LPCSTR    m_str;
	size_t      m_length;

  public:
    StringCutter() : m_pos(0), m_str(""), m_length(0) {}
    StringCutter(T_LPCSTR str, size_t len = 0) : m_pos(0), m_str(str), m_length(len) {}

    // TODO: 增加一个返回字符串地址和长度，避免构造string的方法

    _TString& Cut(_TString& strOut, T_CHAR ch)
    {
      T_CHAR c = '\0';
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
  // 组装字符串序列
  // ch=','
  // "a","b","c","d" 组装为
  // "a,b,c,d"
  template <class _String, class _TCh, class _TStringList>
  void AssembleString(_String& str, _TCh ch, const _TStringList& aStr)
  {
    str.Clear();

    auto it = aStr.begin();
    auto itEnd = aStr.end();
    for(;it != itEnd; ++it) {
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
  void TranslateEscapeCharacter(_String& str) // 输入/输出
  {
    int i = 0;
    int d = 0;
    if(str.IsEmpty()) {
      return;
    }
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

  // 把num向上对齐到2的幂
  template<typename _Ty>
  _Ty RoundupPowOfTwo(_Ty num)
  {
    if(num == 0) {
      return 0;
    }
    _Ty i = 1;
    while(((_Ty)1 << i) < num) i++;
    return ((_Ty)1 << i);
  }

  template<typename _Ty>
  b32 IsPowerOfTwo(_Ty num)
  {
    return (num & (num - 1)) == 0;
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

#if defined(_CL_SYSTEM_ANDROID) || defined(_CL_SYSTEM_IOS)
	typedef union _LARGE_INTEGER {
		struct {
			CLDWORD LowPart;
			CLLONG HighPart;
		}u;
		CLLONGLONG QuadPart;
	} LARGE_INTEGER;
#endif

  // 手动控制的时间差计算
  class TimeTrace
  {
#if defined(_CL_SYSTEM_WINDOWS)
    LARGE_INTEGER m_Frequency;
    LARGE_INTEGER m_Begin;
    LARGE_INTEGER m_End;
#else
    timeval m_Begin;
    timeval m_End;
#endif

  public:
    TimeTrace();
    void                Begin();
    const TimeTrace&    End();
    double              GetDeltaTime() const;
    double              Dump(CLLPCSTR szName) const;
  };

  // 基于作用域的时间差计算 
  class ScopeTimeTrace
  {
    CLLPCSTR name;  // 这个其实是clStringA
    TimeTrace tt;
    double* prTotalTime;
  public:
    //ScopeTimeTrace(CLLPCSTR szName);
    ScopeTimeTrace(CLLPCSTR szName, double* prTotalTime = NULL);
    ~ScopeTimeTrace();
  };

  u32 GetTime();
  u64 GetTime64(); // Milli Seconds
  u32 GetNumberOfProcessors();  // 获得处理器数量

#if defined(_CLSTD_STRING_H_)
  clStringA ToStringA(const float4x4& m);
  clStringA ToStringA(const float3x3& m);
  clStringA ToStringA(const float3& v);
  clStringA ToStringA(const float4& v);
#endif // _CLSTD_STRING_H_

  size_t ViewMemory16(ch* szBuffer, size_t cBuffer, const void* ptr, size_t count, const void* ptrBase = NULL);
  void DumpMemory(const void* ptr, size_t count);
  extern int s_aPrimeNum[];

  class StockA& ReadUserDefault(StockA& stock);
  class StockW& ReadUserDefault(StockW& stock);

} // namespace clstd

#endif // _CL_UTILITY_