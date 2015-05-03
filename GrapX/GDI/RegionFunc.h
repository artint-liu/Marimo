#ifndef REGION_FUNCTION_HEADER
#define REGION_FUNCTION_HEADER
//#include <vector>
//using namespace std;

#define FIXEDARRAY_MEM_DBG
#define ENABLE_CLSTD_ALLOCATOR

template<typename _Ty, clstd::Allocator& _Alloc>
class FixedArray
{
public:
  FixedArray()
    : m_capacity(0)
    , m_size(0)
    , m_ptr(NULL)
  {
  }

  FixedArray(const size_t uInitSize)
    : m_capacity(uInitSize)
    , m_size(0)
#ifdef ENABLE_CLSTD_ALLOCATOR
    , m_ptr((_Ty*)_Alloc.Alloc(m_capacity * sizeof(_Ty), NULL))
    //, m_ptr(new _Ty[m_capacity])
#else
    , m_ptr((_Ty*)gxHeapAlloc(gxGetProcessHeap(), NULL, m_capacity * sizeof(_Ty)))
#endif // ENABLE_CLSTD_ALLOCATOR
  {
    //TRACE("FixedArray::m_capacity * sizeof(_Ty) = %d\n", m_capacity * sizeof(_Ty));
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(((GXULONG_PTR)m_ptr & 3) == 0);
#endif // FIXEDARRAY_MEM_DBG
    //m_ptr = (_Ty*)GlobalAlloc(GMEM_FIXED, sizeof(_Ty) * uInitSize);
    //memset(m_ptr, 0xcc, m_capacity * sizeof(_Ty));
    //m_ptr[uInitSize] = 0x12345678;
  }
  ~FixedArray()
  {
    //ASSERT(m_ptr == NULL || m_ptr[m_capacity - 1] == 0x12345678);
    destroy();
  }
  
  void operator=(const FixedArray& src)
  {
    if(m_capacity < src.m_size) {
      //TRACE("recapacity %d %d\n", m_capacity, src.m_size);
      recapacity(src.m_capacity);
    }

    m_size = src.m_size;
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(m_size <= m_capacity);
#endif // FIXEDARRAY_MEM_DBG
    memcpy(m_ptr, src.m_ptr, sizeof(_Ty) * m_size);
  }

  void operator<<(FixedArray& src)  // 成员传递, 避免重新分配内存
  {
    destroy();
    m_capacity = src.m_capacity;
    m_size     = src.m_size;
    m_ptr      = src.m_ptr;
    src.m_ptr  = NULL;
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(m_size <= m_capacity);
#endif // FIXEDARRAY_MEM_DBG
  }
  size_t size() const
  {
    //ASSERT(m_ptr == NULL || m_ptr[m_capacity - 1] == 0x12345678);
    return m_size;
  }
  size_t capacity() const
  {
    return m_capacity;
  }
  _Ty& operator[](size_t nIdx)
  {
    //ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(nIdx < m_size);
#endif // FIXEDARRAY_MEM_DBG
    return m_ptr[nIdx];
  }
  _Ty& front() const
  {
    //ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
    return m_ptr[0];
  }
  _Ty& end() const
  {
    return m_ptr[m_size];
  }
  _Ty& back() const
  {
    return m_ptr[m_size - 1];
  }
  void push_back(const _Ty& t)
  {
    //ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
    m_ptr[m_size++] = t;
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(m_size <= m_capacity);
#endif // FIXEDARRAY_MEM_DBG
    //ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
  }

  //void _DbgCheck()
  //{
  //  ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
  //}

  void resize(size_t uNewSize)
  {
    //ASSERT(m_ptr[m_capacity - 1] == 0x12345678);
    m_size = uNewSize;
#ifdef FIXEDARRAY_MEM_DBG
    ASSERT(m_size <= m_capacity);
#endif // FIXEDARRAY_MEM_DBG
  }
  void recapacity(size_t uNewCapacity)
  {
#ifdef ENABLE_CLSTD_ALLOCATOR
    clsize uRetCapacity;
    m_ptr = (_Ty*)_Alloc.Realloc(m_ptr, m_capacity, uNewCapacity * sizeof(_Ty), &uRetCapacity);
    ASSERT(uNewCapacity * sizeof(_Ty) <= uRetCapacity);
    //TRACE("_Alloc.Realloc\n");
#else
    m_ptr = (_Ty*)gxHeapReAlloc(gxGetProcessHeap(), NULL, m_ptr, uNewCapacity * sizeof(_Ty));
#endif // ENABLE_CLSTD_ALLOCATOR
    m_capacity = uRetCapacity;
    //TRACE("FixedArray::m_capacity * sizeof(_Ty)(re) = %d\n", m_capacity * sizeof(_Ty));
  }
  void destroy()
  {
    if(m_ptr != NULL)
    {
#ifdef ENABLE_CLSTD_ALLOCATOR
      _Alloc.Free(m_ptr);
      //delete m_ptr;
#else
      gxHeapFree(gxGetProcessHeap(), NULL, m_ptr);
#endif // ENABLE_CLSTD_ALLOCATOR
      m_ptr = NULL;
    }
    m_size = 0;
    m_capacity = 0;
  }
  void clear()
  {
    m_size = 0;
  }

private:
  size_t  m_size;
  size_t  m_capacity;
  _Ty*  m_ptr;
};

typedef clvector<GXLONG> LongArray;
typedef clvector<GXRECT> RectArray;

extern clstd::Allocator g_Alloc_RegionLineData;
#ifdef TEST_LINE
typedef LongArray LongFixedArray;
#else
typedef FixedArray<GXLONG, g_Alloc_RegionLineData> LongFixedArray;
#endif

struct REGIONLINE;

//GXBOOL ParseEdgeSelect(int xPos, int yPos);
//GXBOOL ChangeCursor(HWND hWnd);
//void ChangeMainRect(HWND hWnd,int xPos, int yPos);

GXLONG* __cpp_Line_Copy(LongFixedArray& aDest, const GXLONG* aSrc1, const size_t aSrc1_size);
GXLONG* __asm_Line_Copy(LongFixedArray& aDest, const GXLONG* aSrc1, const size_t aSrc1_size);

GXLONG* Line_Subtract(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
GXLONG* Line_And(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
GXLONG* Line_Xor(LongFixedArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
size_t __cpp_RegionLine_Or(LongFixedArray& aDest, REGIONLINE* aSrc1, REGIONLINE* aSrc2, REGIONLINE* aSrc1End, REGIONLINE* aSrc2End);


GXINT Line_SafeSubtract(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
GXINT Line_SafeAnd(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
GXINT Line_SafeXor(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);
GXINT Line_SafeOr(LongArray& aDest, const GXLONG* aSrc1, const GXLONG* aSrc2, const size_t aSrc1_size, const size_t aSrc2_size);

#if defined (_WIN32) && 0
#define Line_Copy __asm_Line_Copy
#define Line_Or __cpp_RegionLine_Or
#else
#define Line_Copy __cpp_Line_Copy
#define Line_Or __cpp_RegionLine_Or
//#define Line_Or __cpp_Line_Or
#endif // TEST_LINE

#endif // end of REGION_FUNCTION_HEADER