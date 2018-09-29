#ifndef _GREGION_IMPL_H_
#define _GREGION_IMPL_H_

//#include <Include/GRegion.h>
//#include <vector>
//using namespace std;
//#include "FixedArray.h"

//typedef struct _tagGXRGNDATAHEADER    // rgndh  
//{
//  GXUINT      dwSize;        // 结构信息头的大小，字节数
//  GXUINT      iType;         // 总是 RDH_RECTANGLES
//  GXUINT      nCount;        // 包含的Rect个数
//  GXUINT      nRgnSize;      // 全部数据的大小
//  RECT      rcBound;         // 全部区域所占用的矩形区
//} GXRGNDATAHEADER, *LPGXRGNDATAHEADER;  // 对外部用户使用的结构
//
//
//typedef struct _tagGXRGNDATA
//{
//  GXRGNDATAHEADER     rdh;
//  GXBYTE            Buffer[1];
//}GXRGNDATA, *LPGXRGNDATA;  // 对外部用户的声明

//typedef void GAllocate;

//#define ENABLE_CHECK_EMPTY_RECT

class GAllocator
{
private:
  typedef clvector<void*> VoidPtrArray;
  VoidPtrArray    aVoidPtr;
  GXUINT          nMaxCount;
  clstd::Locker*  pRefLocker; // 引用的锁, 不用释放它
public:
  GAllocator(clstd::Locker* pLocker);
  ~GAllocator();
  void* Alloc(GXSIZE_T nBytes);
  void Free(void* ptr);
};

// RGN 行的信息
struct REGIONLINEHEAD
{
  GXLONG    nBottom;      // 当前行的底部
  GXUINT    nCounts;      // 当前行具有的矩形数目
  
  inline REGIONLINEHEAD*  Next() const;
  inline GXLONG*          DataPtr() const;
  inline GXLONG           Data(GXINT nIndex) const;
  inline GXLONG           RawData(GXINT nIndex) const;
};
STATIC_ASSERT(sizeof(REGIONLINEHEAD) == 8);

struct REGIONLINE
{
  GXLONG    left;
  GXLONG    right;
};


class GRegionImpl : public GRegion
{
  friend class GAllocator;
private:
  GXINT         m_nLine;
  GXRECT        m_rcBounding;
#ifdef TEST_LINE
  vector<GXLONG>    m_aData;
#else
  LongFixedArray    m_aData;
#endif
  GAllocator*      m_pAllocator;

  inline static size_t 
    EstimateSize(const GRegionImpl* pSrc1, const GRegionImpl* pSrc2);
  
  inline static void 
    CheckCapacity(LongFixedArray& aData, const size_t sizeSrc);
  
  inline static REGIONLINEHEAD* const 
    BeginLine(LongFixedArray& aData, const size_t sizeSrc);

  inline GRegionImpl* 
    Union_NoOverlap(GRegionImpl* pNewRegion, const GRegionImpl* pRegion) const;

  GXBOOL    __cpp_MergeFragment();
  GXBOOL    __asm_MergeFragment();
#ifdef _DEBUG
  void DbgCheckEmptyRect();
  void DbgDumpRegion();
  void DbgFastCheck() const;
#endif // #ifdef _DEBUG

public:
  GRegionImpl(GAllocator* pAllocate);
  GRegionImpl(size_t cbRgnSize, GAllocator* pAllocate);
  GRegionImpl(const GXRECT& rect, GAllocator* pAllocate);
  GRegionImpl(const GXRECT* lpRects, const GXUINT nCount, GAllocator* pAllocate);
  GRegionImpl(const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse, GAllocator* pAllocate);

  //*
  void* operator new(size_t cbSize, GAllocator* pAllocate);
  void  operator delete(void* ptr);
  //*/

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT    AddRef         ();
  virtual GXHRESULT    Release        ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  // 设置形状
  virtual GXVOID      SetEmpty        ();
  virtual GXVOID      SetRect         (const GXRECT* lprect);
  virtual GXVOID      SetRoundRect    (const GXRECT* lprect, GXUINT nWidthEllipse, GXUINT nHeightEllipse);
  virtual RGNCOMPLEX  Offset          (GXINT xOffset, GXINT yOffset);

  // 状态分析
  virtual GXBOOL      Equal           (const GRegion* pRegion) const;
  virtual GXBOOL      IsEmpty         () const;
  virtual GXBOOL      IsRectangle     () const;
  virtual RGNCOMPLEX  GetComplexity   () const;
  virtual GXUINT      GetData         (clvector<GXRECT>& aRects) const;
  virtual GXVOID      GetBounding     (GXRECT* rect) const;
  virtual GXUINT      GetRectCount    () const;
  virtual GXUINT      GetRects        (GXRECT* lpRects, int nCount) const;
  virtual GXUINT      GetScan         () const;

  virtual GXBOOL      PtInRegion      (int x, int y) const;
  virtual GXBOOL      RectInRegion    (const GXRECT* rect) const;

  // 逻辑操作
  virtual GRegion*    Clone           () const;
  virtual GRegion*    CreateIntersect (const GRegion* pIRegion) const;
  virtual GRegion*    CreateXor       (const GRegion* pIRegion) const;
  virtual GRegion*    CreateSubtract  (const GRegion* pIRegion) const;
  virtual GRegion*    CreateUnion     (const GRegion* pIRegion) const;

  virtual RGNCOMPLEX  Copy            (const GRegion* pIRegion);
  virtual RGNCOMPLEX  Intersect       (const GRegion* pIRegion);
  virtual RGNCOMPLEX  Xor             (const GRegion* pIRegion);
  virtual RGNCOMPLEX  Subtract        (const GRegion* pIRegion);
  virtual RGNCOMPLEX  Union           (const GRegion* pIRegion);

private:

  RGNCOMPLEX  _Impl_GetComplexity () const;
  GXBOOL      _Impl_Intersect     (GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const;
  GXBOOL      _Impl_Xor           (GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const;
  GXBOOL      _Impl_Subtract      (GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const;
  GXBOOL      _Impl_Union         (GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const;

public:
  static GRegionImpl* CreateEmptyRgn        (GAllocator* pAllocate);
  static GRegionImpl* CreateRectRgn         (GAllocator* pAllocate, const GXRECT& rect);
  static GRegionImpl* CreateRectRgnIndirect (GAllocator* pAllocate, const GXRECT* lpRects, const GXUINT nCount);
  static GRegionImpl* CreateRoundRectRgn    (GAllocator* pAllocate, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse);
};

#if defined(_WIN32) && 0
#define MergeFragment  __asm_MergeFragment
#else
#define MergeFragment  __cpp_MergeFragment
#endif


#endif // _GREGION_IMPL_H_