#ifndef _GREGION_IMPL_H_
#define _GREGION_IMPL_H_

//#include <Include/GRegion.h>
//#include <vector>
//using namespace std;
//#include "FixedArray.h"

//typedef struct _tagGXRGNDATAHEADER    // rgndh  
//{
//  GXUINT      dwSize;        // �ṹ��Ϣͷ�Ĵ�С���ֽ���
//  GXUINT      iType;         // ���� RDH_RECTANGLES
//  GXUINT      nCount;        // ������Rect����
//  GXUINT      nRgnSize;      // ȫ�����ݵĴ�С
//  RECT      rcBound;         // ȫ��������ռ�õľ�����
//} GXRGNDATAHEADER, *LPGXRGNDATAHEADER;  // ���ⲿ�û�ʹ�õĽṹ
//
//
//typedef struct _tagGXRGNDATA
//{
//  GXRGNDATAHEADER     rdh;
//  GXBYTE            Buffer[1];
//}GXRGNDATA, *LPGXRGNDATA;  // ���ⲿ�û�������

//typedef void GAllocate;

//#define ENABLE_CHECK_EMPTY_RECT

class GAllocator
{
private:
  typedef clvector<void*> VoidPtrArray;
  VoidPtrArray    aVoidPtr;
  GXUINT          nMaxCount;
  clstd::Locker*  pRefLocker; // ���õ���, �����ͷ���
public:
  GAllocator(clstd::Locker* pLocker);
  ~GAllocator();
  void* Alloc(GXSIZE_T nBytes);
  void Free(void* ptr);
};

// RGN �е���Ϣ
struct REGIONLINEHEAD
{
  GXLONG    nBottom;      // ��ǰ�еĵײ�
  GXUINT    nCounts;      // ��ǰ�о��еľ�����Ŀ
  
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

  // ������״
  virtual GXVOID      SetEmpty        ();
  virtual GXVOID      SetRect         (const GXRECT* lprect);
  virtual GXVOID      SetRoundRect    (const GXRECT* lprect, GXUINT nWidthEllipse, GXUINT nHeightEllipse);
  virtual RGNCOMPLEX  Offset          (GXINT xOffset, GXINT yOffset);

  // ״̬����
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

  // �߼�����
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