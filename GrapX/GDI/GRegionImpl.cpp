// 全局头文件
#include <GrapX.h>

// 标准接口
//#include <GrapX/GUnknown.h>
#include <GrapX/GResource.h>
#include <GrapX/GRegion.h>

// 私有头文件
#include <clUtility.h>
#include "GrapX/GXUser.h"
#include "GrapX/GXKernel.h"

#include "RegionFunc.h"
#include "GRegionImpl.h"

// 关掉这个警告不但会影响编译输出信息,还会影响编译器行为
// 使用这条预编译指令时, GRegionImpl::delete能够正确调用,并且也能正确调用GRegionImpl的析构函数
// 如果关掉它, delete会被链接到全局的delete函数上,并且也不太能够正确调用析构函数
#pragma warning(disable : 4291)

#ifdef _DEBUG
#define DBG_FAST_CHECK  DbgFastCheck(); pRegion->DbgFastCheck();
#else
#define DBG_FAST_CHECK
#endif // #ifdef _DEBUG


#define REGION_TRACE //TRACE
#define SAFE_DELETE_RGN(x,ALLOCATE)      if((x) != NULL) {delete(x); (x) = 0;}
static const size_t s_grain = 0x1F;  // 31, 内存的粒度

// 内存池分配策略
const static clstd::ALLOCPLOY aclAllocPloy[] =
{
  {32, 128},
  {64, 128},
  {128, 128},
  {256, 64},
  {512, 32},
  {1024, 32},
  {0,0},
};

clstd::Allocator g_Alloc_RegionLineData("RegionLineDataPool", aclAllocPloy);


GRegionImpl::GRegionImpl(GAllocator* pAllocate)
: m_nLine (0)
, m_aData (0) // Empty区域不需要预留数据区
, m_pAllocator(pAllocate)
{
  // 创建为 Empty 的区域不需要初始化 m_aData (预留数据区)
  // 这是由算法决定的,比如"0|A",结果就是A
  gxSetRectEmpty(&m_rcBounding);
  AddRef();
}

GRegionImpl::GRegionImpl(size_t cbRgnSize, GAllocator* pAllocate)
: m_nLine(0)
, m_aData(cbRgnSize)
, m_pAllocator(pAllocate)
{
  m_rcBounding.left   = 0;
  m_rcBounding.top    = 0;
  m_rcBounding.right  = 0;
  m_rcBounding.bottom = 0;
  AddRef();
}

GRegionImpl::GRegionImpl(const GXRECT& rect, GAllocator* pAllocate)
: m_nLine(1)
, m_aData(2 + 2)
, m_pAllocator(pAllocate)
{
  AddRef();
  if(gxIsRectEmpty(&rect) != FALSE)
  {
    SetEmpty();
    return;
  }

  m_rcBounding = rect;
  //if(rect.left > rect.right)
  //  Swap(m_rcBounding.left, m_rcBounding.right);
  //if(rect.top > rect.bottom)
  //  Swap(m_rcBounding.top, m_rcBounding.bottom);

  //REGIONLINEHEAD rlh;
  //rlh.nCounts  = 2;
  //rlh.nBottom  = m_rcBounding.bottom;
  //m_aData.push_back(rlh);

  m_aData.push_back(m_rcBounding.bottom);
  m_aData.push_back(2);
  m_aData.push_back(m_rcBounding.left);
  m_aData.push_back(m_rcBounding.right);

  //rlh.left  = m_rcBounding.left;
  //rlh.right = m_rcBounding.right;
  //m_aData.push_back(rlh);
}

GRegionImpl::GRegionImpl(const GXRECT* lpRects, const GXUINT nCount, GAllocator* pAllocate)
: m_nLine(0)
, m_aData(2 + 2)
, m_pAllocator(pAllocate)
{
  m_rcBounding.left   = 0;
  m_rcBounding.top    = 0;
  m_rcBounding.right  = 0;
  m_rcBounding.bottom = 0;
  AddRef();

  if(nCount > 0)
  {
    GRegionImpl* pSrc1 = new(pAllocate) GRegionImpl(lpRects[0], pAllocate);
    GRegionImpl* pDest = pSrc1;

    if(nCount > 1)
    {
      for(size_t i = 1; i < nCount; i++)
      {
        GRegionImpl* pSrc2 = new(pAllocate) GRegionImpl(lpRects[i], pAllocate);
        pDest = (GRegionImpl*)pSrc1->CreateUnion(pSrc2);
        SAFE_DELETE_RGN(pSrc1, pAllocate);
        SAFE_DELETE_RGN(pSrc2, pAllocate);
        pSrc1 = pDest;
      }
      m_aData.recapacity((pDest->m_aData.size() + s_grain) & (~s_grain));
      m_nLine = pDest->m_nLine;
      m_aData = pDest->m_aData;
      m_rcBounding = pDest->m_rcBounding;
      SAFE_DELETE_RGN(pDest, pAllocate);
    }
    else
    {
      m_aData.recapacity((pDest->m_aData.size() + s_grain) & (~s_grain));
      m_nLine = pSrc1->m_nLine;
      m_aData = pSrc1->m_aData;
      m_rcBounding = pSrc1->m_rcBounding;
      SAFE_DELETE_RGN(pSrc1, pAllocate);
    }
  }
}

GRegionImpl::GRegionImpl(const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse, GAllocator* pAllocate)
: m_nLine(0)
, m_aData((((nHeightEllipse + 1) * 2 + 1) * 4 + s_grain) & (~s_grain))  // 估算的
, m_pAllocator(pAllocate)
{
  AddRef();
  m_rcBounding = rect;
  GXUINT nWidth  = clMin((GXUINT)(rect.right - rect.left) / 2, nWidthEllipse);
  GXUINT nHeight = clMin((GXUINT)(rect.bottom - rect.top) / 2, nHeightEllipse);

  GXRECT rcBase = rect;
  gxInflateRect(&rcBase, -(GXINT)nWidth, -(GXINT)nHeight);

  GXINT x = 0;
  GXINT y = -(GXINT)nHeight;
  const int wp2 = nWidth * nWidth;
  const int hp2 = nHeight * nHeight;
  int n = wp2 * hp2 - (hp2 * x * x + wp2 * y * y);

  while(y <= 0)
  {
    const int d1 = n;
    const int d2 = n - hp2 * x * 2 - hp2;

    const GXDWORD r = (GXDWORD)(d1 ^ d2);
    if((r & 0x80000000) == 0x80000000 || r == 0)
    {
      y++;
      n -= wp2 * y * 2 - wp2;
      
      if(x > 0 || rcBase.left != rcBase.right) // 防止写入一个空矩形
      {
        REGIONLINEHEAD* const prlhDest = (REGIONLINEHEAD*)&m_aData.end();
        prlhDest->nBottom = rcBase.top + y;
        prlhDest->nCounts = 2;
        m_aData.resize(m_aData.size() + 2);

        m_aData.push_back(rcBase.left - x);
        m_aData.push_back(rcBase.right + x);
        m_nLine++;
      }
      else 
        rcBase.top--; // 产生空矩形后的高度修正
    }
    else
    {
      x++;
      n = d2;
    }
  }
  // 上下是对称的,所以要保存上面的数据索引
  const REGIONLINEHEAD* prlhTop = (REGIONLINEHEAD*)&m_aData.end();
  GXUINT nLineCount = m_nLine;

  // 这个是圆角中间那段矩形
  if(rcBase.bottom - rcBase.top > 0)
  {
    REGIONLINEHEAD* const prlhDest = (REGIONLINEHEAD*)&m_aData.end();
    prlhDest->nBottom = rcBase.bottom;
    prlhDest->nCounts = 2;
    m_aData.resize(m_aData.size() + 2);

    m_aData.push_back(m_rcBounding.left);
    m_aData.push_back(m_rcBounding.right);
    m_nLine++;
  }

  for(GXUINT i = 0; i < nLineCount; i++)
  {
    prlhTop = (REGIONLINEHEAD*)(((GXLONG*)prlhTop) - 4);
    REGIONLINEHEAD* const prlhDest = (REGIONLINEHEAD*)&m_aData.end();
    prlhDest->nBottom = m_rcBounding.bottom - (prlhTop->nBottom - m_rcBounding.top) + 1;
    prlhDest->nCounts = 2;
    m_aData.resize(m_aData.size() + 2);

    m_aData.push_back(prlhTop->Data(0));
    m_aData.push_back(prlhTop->Data(1));
    m_nLine++;
  }
  MergeFragment();
  //DbgDumpRegion();
}
//*/
GAllocator::GAllocator(clstd::Locker* pLocker)
  : nMaxCount(0)
  , pRefLocker(pLocker)
{
}

GAllocator::~GAllocator()
{
  ASSERT(aVoidPtr.size() == 0);
}

void* GAllocator::Alloc(GXSIZE_T nBytes)
{
  void* ptrNew = new GXBYTE[nBytes];//gxHeapAlloc(gxGetProcessHeap(), NULL, nBytes);

  //BEGIN_SCOPE_SAFE_LOCKER(pRefLocker);
  //nMaxCount = clMax(nMaxCount, (GXUINT)aVoidPtr.size() + 1);
  //aVoidPtr.push_back(ptrNew);
  //END_SCOPE_SAFE_LOCKER;
  return ptrNew;
}

void GAllocator::Free(void* ptr)
{
  delete ptr;
  //for(VoidPtrArray::iterator it = aVoidPtr.begin();
  //  it != aVoidPtr.end(); ++it)
  //{
  //  if(ptr == *it)
  //  {
  //    aVoidPtr.erase(it);
  //    delete ptr;
  //    //gxHeapFree(gxGetProcessHeap(), NULL, ptr);
  //    return;
  //  }
  //}
  //ASSERT(0);
  //gxHeapFree(gxGetProcessHeap(), NULL, ptr);
}

void* GRegionImpl::operator new(size_t cbSize, GAllocator* pAllocate)
{
  ASSERT(cbSize == sizeof(GRegionImpl));
  GRegionImpl* pRegion = (GRegionImpl*)pAllocate->Alloc(cbSize);
  pRegion->m_pAllocator = pAllocate;
  return pRegion;
  //return gxHeapAlloc(gxGetProcessHeap(), NULL, cbSize);
}
void GRegionImpl::operator delete(void* ptr)
{
  // 访问 ((GRegionImpl*)ptr)->m_pAllocator ...
  //gxHeapFree(gxGetProcessHeap(), NULL, ptr);
  GRegionImpl* pRegion = (GRegionImpl*)ptr;
  pRegion->m_pAllocator->Free(ptr);
}
//*/

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GRegionImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GRegionImpl::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == NULL)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

REGIONLINEHEAD* REGIONLINEHEAD::Next() const
{
  return (REGIONLINEHEAD*)((GXBYTE*)this + sizeof(REGIONLINEHEAD) + nCounts * sizeof(GXLONG));
}

GXLONG* REGIONLINEHEAD::DataPtr() const
{
  return (GXLONG*)this + (sizeof(REGIONLINEHEAD) / sizeof(GXLONG));
}
GXLONG REGIONLINEHEAD::Data(GXINT nIndex) const
{
  return *((GXLONG*)this + nIndex + (sizeof(REGIONLINEHEAD) / sizeof(GXLONG)));
}
GXLONG REGIONLINEHEAD::RawData(GXINT nIndex) const
{
  return *((GXLONG*)this + nIndex);
}



size_t GRegionImpl::EstimateSize(const GRegionImpl* pSrc1, const GRegionImpl* pSrc2)
{
  const size_t total = (pSrc1->m_aData.size() + pSrc2->m_aData.size()) * 2;
  const size_t sizeNeed = (total + s_grain) & (~s_grain);
  return sizeNeed;
}

void GRegionImpl::CheckCapacity(LongFixedArray& aData, const size_t sizeSrc)
{
  const size_t uSize = aData.size() + sizeSrc;
  if( uSize > aData.capacity() )
    aData.recapacity((uSize + s_grain) & (~s_grain));
}

REGIONLINEHEAD* const GRegionImpl::BeginLine(LongFixedArray& aData, const size_t sizeSrc)
{
  CheckCapacity(aData, sizeSrc);
  REGIONLINEHEAD* const prlhDest = (REGIONLINEHEAD*)&aData.end();
  aData.resize(aData.size() + 2);
  return prlhDest;
}

GRegionImpl* GRegionImpl::Union_NoOverlap(GRegionImpl* pNewRegion, const GRegionImpl* pRegion) const
{
  size_t nCount1 = m_aData.size();
  size_t nCount2 = pRegion->m_aData.size();

  GXLONG nTouchBottom = m_rcBounding.bottom;

  REGIONLINEHEAD* const prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* const prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();

  if(m_rcBounding.top >= pRegion->m_rcBounding.bottom)
  {
    clSwap((GXULONG_PTR&)prlhSrc1, (GXULONG_PTR&)prlhSrc2);
    clSwap(nCount1, nCount2);
    nTouchBottom = pRegion->m_rcBounding.bottom;
  }

  // 复制第一个区域
  Line_Copy(pNewRegion->m_aData, (const GXLONG*)prlhSrc1, nCount1);

  REGIONLINEHEAD* const prlhDest = (REGIONLINEHEAD*)&pNewRegion->m_aData.end();
  prlhDest->nBottom = clMax(m_rcBounding.top, pRegion->m_rcBounding.top);

  // 避免在两个边界恰好相邻的区域插入空行
  if(prlhDest->nBottom > nTouchBottom)
  {
    prlhDest->nCounts = 0;
    pNewRegion->m_aData.resize(pNewRegion->m_aData.size() + 2);
    pNewRegion->m_nLine = 1;
  }

  // 复制第二个区域
  Line_Copy(pNewRegion->m_aData, (const GXLONG*)prlhSrc2, nCount2);
  pNewRegion->m_nLine += m_nLine + pRegion->m_nLine;
  return pNewRegion;
}

void GRegionImpl::SetEmpty()
{
  gxSetRectEmpty(&m_rcBounding);
  m_nLine = 0;
  m_aData.clear();
}

GXVOID GRegionImpl::SetRect(const GXRECT* lprect)
{
  if(gxIsRectEmpty(lprect) != FALSE)
  {
    SetEmpty();
    return;
  }

  m_rcBounding = *lprect;
  if(m_aData.size() < 4) {
    m_aData.recapacity(4);
  }
  m_nLine = 1;
  m_aData.clear();
  m_aData.push_back(m_rcBounding.bottom);
  m_aData.push_back(2);
  m_aData.push_back(m_rcBounding.left);
  m_aData.push_back(m_rcBounding.right);
}

GXVOID GRegionImpl::SetRoundRect(const GXRECT* lprect, GXUINT nWidthEllipse, GXUINT nHeightEllipse)
{
  ASSERT(0);
}

RGNCOMPLEX GRegionImpl::Offset(GXINT xOffset, GXINT yOffset)
{
  REGION_TRACE("GRegionImpl::Offset\n");
  if(xOffset == 0 && yOffset == 0)
    return RC_ERROR;
  REGIONLINEHEAD* prlhSrc = (REGIONLINEHEAD*)&m_aData.front();
  if(prlhSrc == NULL || m_aData.size() == 0)
    return RC_NULL;
  
  gxOffsetRect(&m_rcBounding, xOffset, yOffset);

  if(xOffset != 0)
  {
    if(yOffset == 0)
    {
      while(1){
        GXLONG* pLine = prlhSrc->DataPtr();
        for(size_t i = 0; i < prlhSrc->nCounts; i += 2)
        {
          *pLine++ += xOffset;
          *pLine++ += xOffset;
        }
        if(prlhSrc->nBottom >= m_rcBounding.bottom)
          break;
        prlhSrc = prlhSrc->Next();
      }
    }
    else  // xOffset != 0 && yOffset != 0
    {
      while(1)
      {
        prlhSrc->nBottom += yOffset;
        GXLONG* pLine = prlhSrc->DataPtr();
        for(size_t i = 0; i < prlhSrc->nCounts; i += 2)
        {
          *pLine++ += xOffset;
          *pLine++ += xOffset;
        }
        if(prlhSrc->nBottom >= m_rcBounding.bottom)
          break;
        prlhSrc = prlhSrc->Next();
      }
    }
  }
  else  // xOffset == 0 && yOffset != 0
  {
    while(1)
    {
      prlhSrc->nBottom += yOffset;
      if(prlhSrc->nBottom >= m_rcBounding.bottom)
        break;
      prlhSrc = prlhSrc->Next();
    }
  }
  return _Impl_GetComplexity();
}

GXBOOL GRegionImpl::Equal(const GRegion* pRegion) const
{
  ASSERT(0);
  return FALSE;
}

GXBOOL GRegionImpl::IsEmpty() const
{
  return m_nLine == 0;
}

GXBOOL GRegionImpl::IsRectangle() const
{
  return (m_nLine == 1) &&
    ((REGIONLINEHEAD*)&m_aData.front())->nCounts == 2;
}

RGNCOMPLEX GRegionImpl::GetComplexity() const
{
  return _Impl_GetComplexity();
}
//GRegionImpl* GRegionImpl::Clone(GXBOOL bIgnoreSameLine)
//{
//  GRegionImpl* pRegion = new GRegionImpl;
//  pRegion->m_nLine    = m_nLine;
//  pRegion->m_rcBounding = m_rcBounding;
//  if(bIgnoreSameLine != FALSE)
//    pRegion->m_aData = m_aData;
//  else
//  {
//    __asm int 3
//  }
//  return pRegion;
//}

//GRegionImpl* GRegionImpl::Subtract(GRegionImpl* pRegion)
//{
//  GRegionImpl*      pNewRegion = new GRegionImpl;
//  struct REGIONV
//  {
//    GXLONG nTop;
//    GXLONG nBtm;
//  };
//  SubtractRect(&pNewRegion->m_rcBounding, &m_rcBounding, &pRegion->m_rcBounding);
//
//  REGIONLINEHEAD rlh;
//  REGIONLINEHEAD* prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
//  REGIONLINEHEAD* prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();
//
//  REGIONV Dest, Src1, Src2;
//  Src1.nTop = m_rcBounding.top;
//  Src1.nBtm = prlhSrc1->nBottom;
//
//  Src2.nTop = pRegion->m_rcBounding.top;
//  Src2.nBtm = prlhSrc2->nBottom;
//
//  if(Src1.nTop < Src2.nTop)
//  {
//    Dest.nTop = Src1.nTop;
//    Dest.nBtm = Min(Src2.nTop, Src1.nBtm);
//  }
//  else if(Src1.nTop > Src2.nTop)
//  {
//    Dest.nTop = Src2.nTop;
//    Dest.nBtm = Min(Src1.nTop, Src2.nBtm);
//  }
//  else
//  {
//    Dest.nTop = Src1.nTop;
//    Dest.nBtm = Min(Src1.nBtm, Src2.nBtm);
//  }
//
//  GXLONG* pSrc1 = prlhSrc1->DataPtr();
//  GXLONG* pSrc2 = prlhSrc2->DataPtr();
//
//  REGIONLINEHEAD* pDestLine = NULL;//(REGIONLINEHEAD*)&pNewRegion->m_aData.front();
//  while(1)
//  {
//    GXBOOL bInSect1 = Dest.nTop >= Src1.nTop && Dest.nBtm <= Src1.nBtm;
//    GXBOOL bInSect2 = Dest.nTop >= Src2.nTop && Dest.nBtm <= Src2.nBtm;
//    if(bInSect1 && bInSect2)
//    {
//      //pDestLine
//
//      const size_t nBegin = pNewRegion->m_aData.size();
//      pNewRegion->m_aData.push_back(0);
//      Line_Subtract(pNewRegion->m_aData, pSrc1, pSrc2, (size_t)prlhSrc1->nCounts, (size_t)prlhSrc2->nCounts);
//      const size_t nEnd = pNewRegion->m_aData.size();
//      pDestLine = (REGIONLINEHEAD*)&pNewRegion->m_aData[nBegin];
//      pDestLine->nBottom = Dest.nBtm;
//      pDestLine->nCounts = nEnd - nBegin - 1;
//
//      Dest.nTop = Dest.nBtm;
//
//      if(Dest.nBtm == Src1.nBtm)
//      {
//        if(prlhSrc1->nBottom == m_rcBounding.bottom)
//          break;
//        prlhSrc1 = (REGIONLINEHEAD*)((GXBYTE*)prlhSrc1 + prlhSrc1->nCounts * sizeof(GXLONG));
//        Src1.nTop = Src1.nBtm;
//        Src1.nBtm = prlhSrc1->nBottom;
//
//        Dest.nBtm = Src1.nBtm;
//      }
//      if(Dest.nBtm == Src2.nBtm)
//      {
//        if(prlhSrc2->nBottom != pRegion->m_rcBounding.bottom)
//        {
//          prlhSrc2 = (REGIONLINEHEAD*)((GXBYTE*)prlhSrc2 + prlhSrc2->nCounts * sizeof(GXLONG));
//          Src2.nTop = Src2.nBtm;
//          Src2.nBtm = prlhSrc2->nBottom;
//          Dest.nBtm = Min(Dest.nBtm, Src2.nBtm);
//        }
//      }
//    }
//    else if(bInSect1)
//    {
//      //ASSERT(Dest.nBtm == Src1.nBtm);
//      //pNewRegion->m_aData.push_back(0);
//      //pDestLine = (REGIONLINEHEAD*)&(pNewRegion->m_aData.end() - 1);
//      //pDestLine->nBottom = Dest.nBtm;
//      //pDestLine->nCounts = Line_Subtract(pNewRegion->m_aData, pSrc1, pSrc2, (size_t)prlhSrc1->nCounts, (size_t)prlhSrc2->nCounts);
//
//      const size_t nBegin = pNewRegion->m_aData.size();
//      pNewRegion->m_aData.push_back(0);
//      Line_Copy(pNewRegion->m_aData, pSrc1, (size_t)prlhSrc1->nCounts);
//      const size_t nEnd = pNewRegion->m_aData.size();
//      pDestLine = (REGIONLINEHEAD*)&pNewRegion->m_aData[nBegin];
//      pDestLine->nBottom = Dest.nBtm;
//      pDestLine->nCounts = nEnd - nBegin - 1;
//
//      Dest.nTop = Dest.nBtm;
//      if(Dest.nBtm == Src1.nBtm)
//      {
//        prlhSrc1 = (REGIONLINEHEAD*)((GXBYTE*)prlhSrc1 + prlhSrc1->nCounts * sizeof(GXLONG));
//        if(prlhSrc1->nBottom == m_rcBounding.bottom)
//          break;
//      }
//      Dest.nBtm = Min(Src1.nBtm, Src2.nBtm);
//    }
//    else 
//      ASSERT(0);
//    //// |[]|
//    ////      |[]|
//    //if(Src1.nBtm <= Src2.nTop)
//    //{
//    //  // 复制整行
//    //}
//    ////      |[]|
//    //// |[]|
//    //else if(Src1.nTop >= Src2.nBtm)
//    //{
//    //  // 复制整行
//    //}
//    //// |[][]|
//    ////   |[][][]|
//    //// 或者
//    //// |[][][][]|
//    ////   |[][]|
//    //else if(Src1.nTop < Src2.nTop)
//    //{
//    //  // 复制整行
//    //  // 交集运算
//
//    //}
//    ////   |[][]|
//    //// |[][]|
//    //// 或者
//    ////   |[][]|
//    //// |[][][][]|
//    //else if(Src1.nTop > Src2.nTop)
//    //{
//
//    //}
//    //else if(Src1.nTop == Src2.nTop)
//    //{
//
//    //}
//    //else
//    //  ASSERT(0);
//
//  }
//  return pNewRegion;
//}
GRegion* GRegionImpl::Clone() const
{
  REGION_TRACE("GRegionImpl::Clone()\n");
  GRegionImpl* pNewRegion = new(m_pAllocator) GRegionImpl(m_aData.size(), m_pAllocator);

#ifdef _DEBUG
  DbgFastCheck();
#endif // #ifdef _DEBUG

  pNewRegion->m_rcBounding = m_rcBounding;
  pNewRegion->m_aData     = m_aData;
  pNewRegion->m_nLine     = m_nLine;
  return pNewRegion;
}

GRegion* GRegionImpl::CreateSubtract(const GRegion* pIRegion) const
{
  REGION_TRACE("GRegionImpl::CreateSubtract\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  if(IsEmpty() != FALSE) // TODO: 这个地方设计的可能有些问题，0-B结果是B
    return new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);
  else if(pRegion->IsEmpty() != FALSE)
    return Clone();
  
  GRegionImpl* const pNewRegion = new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);

  if( m_rcBounding.top    >= pRegion->m_rcBounding.bottom ||
    m_rcBounding.bottom <= pRegion->m_rcBounding.top    || 
    m_rcBounding.left   >= pRegion->m_rcBounding.right  ||
    m_rcBounding.right  <= pRegion->m_rcBounding.left   )
  {
    pNewRegion->m_rcBounding = m_rcBounding;
    pNewRegion->m_nLine = m_nLine;
    pNewRegion->m_aData = m_aData;
    return pNewRegion;
  }

  _Impl_Subtract(pNewRegion, pRegion);

  return pNewRegion;
}

GRegion* GRegionImpl::CreateIntersect(const GRegion* pIRegion) const
{
  REGION_TRACE("GRegionImpl::CreateIntersect\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;
  GRegionImpl* const pNewRegion = new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);

  DBG_FAST_CHECK;

  _Impl_Intersect(pNewRegion, pRegion);

  return pNewRegion;
}

GRegion* GRegionImpl::CreateXor(const GRegion* pIRegion) const
{
  REGION_TRACE("GRegionImpl::CreateXor\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  if(m_nLine == 0 && pRegion->m_nLine == 0)
    return new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);
  else if(m_nLine == 0)
    return pRegion->Clone();
  else if(pRegion->m_nLine == 0)
    return Clone();

  GRegionImpl* pNewRegion = new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);
  _Impl_Xor(pNewRegion, pRegion);
  
  return pNewRegion;
}

GRegion* GRegionImpl::CreateUnion(const GRegion* pIRegion) const
{
  REGION_TRACE("GRegionImpl::CreateUnion\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  // 其中之一为空
  if(m_nLine == 0 && pRegion->m_nLine == 0)
    return new(m_pAllocator) GRegionImpl(m_pAllocator);
  else if(m_nLine == 0)
    return pRegion->Clone();
  else if(pRegion->m_nLine == 0)
    return Clone();

  GRegionImpl* pNewRegion = new(m_pAllocator) GRegionImpl(EstimateSize(this, pRegion), m_pAllocator);

  _Impl_Union(pNewRegion, pRegion);

  return pNewRegion;
}

RGNCOMPLEX GRegionImpl::Copy(const GRegion* pIRegion)
{
  REGION_TRACE("GRegionImpl::Copy\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  m_nLine       = pRegion->m_nLine;
  m_rcBounding  = pRegion->m_rcBounding;
  m_aData       = pRegion->m_aData;
  m_pAllocator  = pRegion->m_pAllocator;
  return _Impl_GetComplexity();
}

RGNCOMPLEX GRegionImpl::Intersect(const GRegion* pIRegion)
{
  REGION_TRACE("GRegionImpl::Intersect\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;
  GRegionImpl DestRegion(EstimateSize(this, pRegion), m_pAllocator);

  DBG_FAST_CHECK;

  _Impl_Intersect(&DestRegion, pRegion);

  m_nLine     = DestRegion.m_nLine;
  m_rcBounding = DestRegion.m_rcBounding;
  m_aData     << DestRegion.m_aData;

  return _Impl_GetComplexity();
}

RGNCOMPLEX GRegionImpl::Xor(const GRegion* pIRegion)
{
  REGION_TRACE("GRegionImpl::Xor\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  if(m_nLine == 0 && pRegion->m_nLine == 0)
    return RC_NULL;
  else if(m_nLine == 0)
    return Copy(pRegion);
  else if(pRegion->m_nLine == 0)
    return _Impl_GetComplexity();

  GRegionImpl DestRegion(EstimateSize(this, pRegion), m_pAllocator);
  _Impl_Xor(&DestRegion, pRegion);

  m_nLine     = DestRegion.m_nLine;
  m_rcBounding = DestRegion.m_rcBounding;
  m_aData     << DestRegion.m_aData;

  return _Impl_GetComplexity();
}

RGNCOMPLEX GRegionImpl::Subtract(const GRegion* pIRegion)
{
  REGION_TRACE("GRegionImpl::Subtract\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  if(IsEmpty() != FALSE)
    return RC_NULL;
  else if(pRegion->IsEmpty() != FALSE)
    return _Impl_GetComplexity();

  // 不相交
  if( m_rcBounding.top    >= pRegion->m_rcBounding.bottom ||
    m_rcBounding.bottom <= pRegion->m_rcBounding.top    || 
    m_rcBounding.left   >= pRegion->m_rcBounding.right  ||
    m_rcBounding.right  <= pRegion->m_rcBounding.left   )
  {
    return _Impl_GetComplexity();
  }

  GRegionImpl DestRegion(EstimateSize(this, pRegion), m_pAllocator);
  _Impl_Subtract(&DestRegion, pRegion);

  m_nLine     = DestRegion.m_nLine;
  m_rcBounding = DestRegion.m_rcBounding;
  m_aData     << DestRegion.m_aData;

  return _Impl_GetComplexity();
}

RGNCOMPLEX GRegionImpl::Union(const GRegion* pIRegion)
{
  REGION_TRACE("GRegionImpl::Union\n");
  GRegionImpl* const pRegion = (GRegionImpl*)pIRegion;

  DBG_FAST_CHECK;

  // 其中之一为空
  if(m_nLine == 0 && pRegion->m_nLine == 0)
    return RC_NULL;
  else if(m_nLine == 0)
    return Copy(pRegion);
  else if(pRegion->m_nLine == 0)
    return _Impl_GetComplexity();


  GRegionImpl DestRegion(EstimateSize(this, pRegion), m_pAllocator);
  _Impl_Union(&DestRegion, pRegion);

  m_nLine     = DestRegion.m_nLine;
  m_rcBounding = DestRegion.m_rcBounding;
  m_aData     << DestRegion.m_aData;

  return _Impl_GetComplexity();
}

RGNCOMPLEX  GRegionImpl::_Impl_GetComplexity() const
{
  if(m_nLine == 0)
    return RC_NULL;
  else if(m_nLine == 1 && 
    ((REGIONLINEHEAD*)&m_aData.front())->nCounts == 2)
    return RC_SIMPLE;
  else
    return RC_COMPLEX;
}

GXBOOL GRegionImpl::_Impl_Intersect(GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const
{
  if(gxIntersectRect(&pDestRegion->m_rcBounding, &m_rcBounding, &pRegion->m_rcBounding) == FALSE)
  {
    gxSetRectEmpty(&pDestRegion->m_rcBounding);
    return TRUE;
  }
  REGIONLINEHEAD* prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();

  if(m_rcBounding.top < pRegion->m_rcBounding.top)
    while(prlhSrc1->nBottom <= pDestRegion->m_rcBounding.top)
      prlhSrc1 = prlhSrc1->Next();
  else if(m_rcBounding.top > pRegion->m_rcBounding.top)
    while(prlhSrc2->nBottom <= pDestRegion->m_rcBounding.top)
      prlhSrc2 = prlhSrc2->Next();

  GXUINT nBegin, nEnd;
  while(1)
  {
    nBegin = (GXUINT)pDestRegion->m_aData.size();
    REGIONLINEHEAD* const prlhDest = 
      BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts + 2);

    if(prlhSrc1->nCounts != 0 && prlhSrc2->nCounts != 0)
    {
      Line_And(pDestRegion->m_aData, prlhSrc1->DataPtr(), prlhSrc2->DataPtr(), (size_t)prlhSrc1->nCounts, (size_t)prlhSrc2->nCounts);
      nEnd = (GXUINT)pDestRegion->m_aData.size();
      prlhDest->nCounts = (nEnd - nBegin - 2);
    }
    else
      prlhDest->nCounts = 0;
    pDestRegion->m_nLine++;
    const GXBOOL bTouchSrc1 = prlhSrc1->nBottom <= prlhSrc2->nBottom;
    const GXBOOL bTouchSrc2 = prlhSrc1->nBottom >= prlhSrc2->nBottom;

    if(bTouchSrc1)
    {
      prlhDest->nBottom = prlhSrc1->nBottom;
      if(prlhSrc1->nBottom >= pDestRegion->m_rcBounding.bottom)
        break;
      prlhSrc1 = prlhSrc1->Next();
    }

    if(bTouchSrc2)
    {
      prlhDest->nBottom = prlhSrc2->nBottom;
      if(prlhSrc2->nBottom >= pDestRegion->m_rcBounding.bottom)
        break;
      prlhSrc2 = prlhSrc2->Next();
    }
  }
  //ASSERT(((REGIONLINEHEAD*)&pNewRegion->m_aData[nBegin])->nBottom == pNewRegion->m_rcBounding.bottom);
  pDestRegion->MergeFragment();
#if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  pDestRegion->DbgCheckEmptyRect();
#endif // #if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  return TRUE;
}

GXBOOL GRegionImpl::_Impl_Xor(GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const
{
  // 这里不像其他合并算法计算包围盒, 这个只是估算
  gxUnionRect(&pDestRegion->m_rcBounding, &m_rcBounding, &pRegion->m_rcBounding);

  // 水平方向上完全没有交集
  if(m_rcBounding.bottom <= pRegion->m_rcBounding.top ||
    m_rcBounding.top >= pRegion->m_rcBounding.bottom )
  {
    Union_NoOverlap(pDestRegion, pRegion);
    return TRUE;
  }

  //REGIONLINEHEAD rlh;
  REGIONLINEHEAD* prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();
  //REGIONLINEHEAD* prlhDest = NULL;
  GXUINT nBegin, nEnd;
  GXLONG nTouchBottom;


  // 左右边界如果有相同的,可能会在异或时改变
  //GXBOOL bSameLeftRight = (m_rcBounding.left == pDestRegion->m_rcBounding.left) || 
  //  (m_rcBounding.right == pDestRegion->m_rcBounding.right);

  nTouchBottom = pRegion->m_rcBounding.top;

  if(m_rcBounding.top > pRegion->m_rcBounding.top)
  {
    clSwap((GXULONG_PTR&)prlhSrc1, (GXULONG_PTR&)prlhSrc2);
    nTouchBottom = m_rcBounding.top;
    pDestRegion->m_rcBounding.top = pRegion->m_rcBounding.top;  // 计算边界:Top
  }
  else if(m_rcBounding.top == pRegion->m_rcBounding.top)
    goto SAME_LINE;
  else
    pDestRegion->m_rcBounding.top = m_rcBounding.top;  // 计算边界:Top

  while(1)
  {
    //nBegin = pDestRegion->m_aData.size();
    //pDestRegion->m_aData.push_back(0);
    //pDestRegion->m_aData.push_back(0);

    REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);
    Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
    prlhDest->nCounts = prlhSrc1->nCounts;
    pDestRegion->m_nLine++;

    //nEnd = pDestRegion->m_aData.size();
    //prlhDest = (REGIONLINEHEAD*)&pDestRegion->m_aData[nBegin];
    //prlhDest->nCounts = (nEnd - nBegin - 2);

    if(prlhSrc1->nBottom >= nTouchBottom)
    {
      prlhDest->nBottom = nTouchBottom;
      if(prlhSrc1->nBottom == nTouchBottom)
        prlhSrc1 = prlhSrc1->Next();
      break;
    }

    prlhDest->nBottom = prlhSrc1->nBottom;
    prlhSrc1 = prlhSrc1->Next();
  }

SAME_LINE:
  if(m_rcBounding.top > pRegion->m_rcBounding.top)
    clSwap((GXULONG_PTR&)prlhSrc1, (GXULONG_PTR&)prlhSrc2);

  while(1)
  {
    nBegin = (GXUINT)pDestRegion->m_aData.size();
    REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts + 2);
    //CheckCapacity(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts);
    //prlhDest = (REGIONLINEHEAD*)&pDestRegion->m_aData[nBegin];

    //pDestRegion->m_aData.push_back(0);
    //pDestRegion->m_aData.push_back(0);
    Line_Xor(pDestRegion->m_aData, prlhSrc1->DataPtr(), prlhSrc2->DataPtr(), (size_t)prlhSrc1->nCounts, (size_t)prlhSrc2->nCounts);
    nEnd = (GXUINT)pDestRegion->m_aData.size();
    prlhDest->nCounts = (nEnd - nBegin - 2);

    pDestRegion->m_nLine++;

    const GXBOOL bTouchSrc1 = prlhSrc1->nBottom <= prlhSrc2->nBottom;
    const GXBOOL bTouchSrc2 = prlhSrc1->nBottom >= prlhSrc2->nBottom;

    if(bTouchSrc1)
    {
      prlhDest->nBottom = prlhSrc1->nBottom;
      if(prlhSrc1->nBottom == m_rcBounding.bottom)
      {
        clSwap((GXLONG_PTR&)prlhSrc1, (GXLONG_PTR&)prlhSrc2);
        break;
      }
      prlhSrc1 = prlhSrc1->Next();
    }

    if(bTouchSrc2)
    {
      prlhDest->nBottom = prlhSrc2->nBottom;
      if(prlhSrc2->nBottom == pRegion->m_rcBounding.bottom)
        break;
      prlhSrc2 = prlhSrc2->Next();
    }
  }
  if(m_rcBounding.bottom == pRegion->m_rcBounding.bottom)
  {
    pDestRegion->m_rcBounding.bottom = m_rcBounding.bottom;  // 计算边界: Bottom
    goto FINAL_RET;
  }
  else if(prlhSrc2->nBottom == prlhSrc1->nBottom)  // 其实这个执行了 bTouchSrc2 这个分支, 
    prlhSrc1 = prlhSrc1->Next();        // 对于都到底边时,对 prlhSrc2 步进一次

  nTouchBottom = clMax(m_rcBounding.bottom, pRegion->m_rcBounding.bottom);

  while(1)
  {
    //pDestRegion->m_aData.push_back(prlhSrc1->dwData);
    //pDestRegion->m_aData.push_back(prlhSrc1->nBottom);
    //pDestRegion->m_aData.push_back((const GXLONG&)prlhSrc1->nCounts);
    REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);
    Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
    *prlhDest = *prlhSrc1;
    pDestRegion->m_nLine++;

    if(prlhSrc1->nBottom == nTouchBottom)
      break;
    prlhSrc1 = prlhSrc1->Next();
  }
  pDestRegion->m_rcBounding.bottom = nTouchBottom;  // 计算边界: Bottom

FINAL_RET:
  //{
  //  if(bSameLeftRight == FALSE)
  //  {
  //    pDestRegion->MergeFragment();
  //    return pDestRegion;
  //  }

  //  REGIONLINEHEAD* prlhDest = (REGIONLINEHEAD*)&pDestRegion->m_aData.front();
  //  pDestRegion->m_rcBounding.left  = 0x7fffffff;
  //  pDestRegion->m_rcBounding.right = 0x80000000;
  //  while(1)
  //  {
  //    if(prlhDest->nBottom >= pDestRegion->m_rcBounding.bottom)
  //      break;
  //    prlhDest = prlhDest->Next();
  //  }
  //}
  pDestRegion->MergeFragment();
#if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  pDestRegion->DbgCheckEmptyRect();
#endif // #if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  return FALSE;
}

GXBOOL GRegionImpl::_Impl_Subtract(GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const
{
  pDestRegion->m_rcBounding = m_rcBounding;

  //REGIONLINEHEAD rlh;
  REGIONLINEHEAD* prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();
  //REGIONLINEHEAD* prlhDest = NULL;
  GXUINT nBegin, nEnd;

  //GXLONG* pSrc1 = prlhSrc1->DataPtr();
  //GXLONG* pSrc2 = prlhSrc2->DataPtr();

  if(m_rcBounding.top < pRegion->m_rcBounding.top)
  {
    while(1)
    {
      //nBegin = pDestRegion->m_aData.size();
      //pDestRegion->m_aData.resize(nBegin + 2);
      //Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
      //nEnd = pDestRegion->m_aData.size();
      //prlhDest = (REGIONLINEHEAD*)&pDestRegion->m_aData[nBegin];
      //prlhDest->nCounts = (nEnd - nBegin - 2);
      REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);
      Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
      prlhDest->nCounts = prlhSrc1->nCounts;

      pDestRegion->m_nLine++;

      if(prlhSrc1->nBottom >= pRegion->m_rcBounding.top)
      {
        prlhDest->nBottom = pRegion->m_rcBounding.top;
        if(prlhSrc1->nBottom == pRegion->m_rcBounding.top)
          prlhSrc1 = prlhSrc1->Next();
        break;
      }

      prlhDest->nBottom = prlhSrc1->nBottom;
      prlhSrc1 = prlhSrc1->Next();
    }
  }
  else if(m_rcBounding.top > pRegion->m_rcBounding.top)
  {
    while(prlhSrc2->nBottom <= m_rcBounding.top)
      prlhSrc2 = prlhSrc2->Next();
  }
  while(1)
  {
    nBegin = (GXUINT)pDestRegion->m_aData.size();
    //CheckCapacity(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts);
    //prlhDest = (REGIONLINEHEAD*)&pDestRegion->m_aData[nBegin];
    //pDestRegion->m_aData.push_back(0);
    //pDestRegion->m_aData.push_back(0);
    REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts + 2);

    if((size_t)prlhSrc1->nCounts != 0)
    {
      if((size_t)prlhSrc2->nCounts == 0)
        Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
      else
        Line_Subtract(pDestRegion->m_aData, prlhSrc1->DataPtr(), prlhSrc2->DataPtr(), 
        (size_t)prlhSrc1->nCounts, (size_t)prlhSrc2->nCounts);

      nEnd = (GXUINT)pDestRegion->m_aData.size();
      prlhDest->nCounts = (nEnd - nBegin - 2);
    }
    else
      prlhDest->nCounts = 0;

    pDestRegion->m_nLine++;

    const GXBOOL bTouchSrc1 = prlhSrc1->nBottom <= prlhSrc2->nBottom;
    const GXBOOL bTouchSrc2 = prlhSrc1->nBottom >= prlhSrc2->nBottom;

    if(bTouchSrc1)
    {
      prlhDest->nBottom = prlhSrc1->nBottom;
      if(prlhSrc1->nBottom == m_rcBounding.bottom)
      {
        pDestRegion->MergeFragment();
        return TRUE;
      }
      prlhSrc1 = prlhSrc1->Next();
    }

    if(bTouchSrc2)
    {
      prlhDest->nBottom = prlhSrc2->nBottom;
      if(prlhSrc2->nBottom == pRegion->m_rcBounding.bottom)
        break;
      prlhSrc2 = prlhSrc2->Next();
    }
  }
  while(1)
  {
    //pDestRegion->m_aData.push_back(prlhSrc1->nBottom);
    //pDestRegion->m_aData.push_back((const GXLONG&)prlhSrc1->nCounts);
    REGIONLINEHEAD* const prlhDest = BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);
    Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);

    *prlhDest = *prlhSrc1;
    pDestRegion->m_nLine++;

    if(prlhSrc1->nBottom == m_rcBounding.bottom)
    {
      break;
    }
    prlhSrc1 = prlhSrc1->Next();
  }
  pDestRegion->MergeFragment();
#if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  pDestRegion->DbgCheckEmptyRect();
#endif // #if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  return FALSE;
}

GXBOOL GRegionImpl::_Impl_Union(GRegionImpl* pDestRegion, const GRegionImpl* pRegion) const
{
  gxUnionRect(&pDestRegion->m_rcBounding, &m_rcBounding, &pRegion->m_rcBounding);

  // 水平方向上完全没有交集
  if(m_rcBounding.bottom <= pRegion->m_rcBounding.top ||
    m_rcBounding.top >= pRegion->m_rcBounding.bottom )
  {
    Union_NoOverlap(pDestRegion, pRegion);
    return TRUE;
  }

  REGIONLINEHEAD* prlhSrc1 = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* prlhSrc2 = (REGIONLINEHEAD*)&pRegion->m_aData.front();

  GXLONG nTouchBottom;
  if(m_rcBounding.top > pRegion->m_rcBounding.top)
  {
    clSwap((GXULONG_PTR&)prlhSrc1, (GXULONG_PTR&)prlhSrc2);
    nTouchBottom = m_rcBounding.top;
  }
  else if(m_rcBounding.top < pRegion->m_rcBounding.top)
    nTouchBottom = pRegion->m_rcBounding.top;
  else //  if(m_rcBounding.top == pRegion->m_rcBounding.top)
    goto SAME_LINE;

  while(1)
  {
    REGIONLINEHEAD* const prlhDest = 
      BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);

    Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
    prlhDest->nCounts = prlhSrc1->nCounts;

    pDestRegion->m_nLine++;

    if(prlhSrc1->nBottom >= nTouchBottom)
    {
      prlhDest->nBottom = nTouchBottom;
      if(prlhSrc1->nBottom == nTouchBottom)
        prlhSrc1 = prlhSrc1->Next();
      break;
    }

    prlhDest->nBottom = prlhSrc1->nBottom;
    prlhSrc1 = prlhSrc1->Next();
  }

SAME_LINE:
  if(m_rcBounding.top > pRegion->m_rcBounding.top)
    clSwap((GXULONG_PTR&)prlhSrc1, (GXULONG_PTR&)prlhSrc2);

  while(1)
  {
    REGIONLINEHEAD* const prlhDest = 
      BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + prlhSrc2->nCounts + 2);

    if(prlhSrc1->nCounts != 0 && prlhSrc2->nCounts != 0)
    {

      prlhDest->nCounts = (GXUINT)Line_Or(pDestRegion->m_aData, 
        (REGIONLINE*)prlhSrc1->DataPtr(), (REGIONLINE*)prlhSrc2->DataPtr(), 
        (REGIONLINE*)prlhSrc1->DataPtr() + (prlhSrc1->nCounts >> 1), 
        (REGIONLINE*)prlhSrc2->DataPtr() + (prlhSrc2->nCounts >> 1));
    }
    else
    {
      if(prlhSrc1->nCounts != 0)
      {
        Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), prlhSrc1->nCounts);
        prlhDest->nCounts = prlhSrc1->nCounts;
      }
      else if(prlhSrc2->nCounts != 0)
      {
        Line_Copy(pDestRegion->m_aData, prlhSrc2->DataPtr(), prlhSrc2->nCounts);
        prlhDest->nCounts = prlhSrc2->nCounts;
      }
      else
        prlhDest->nCounts = 0;
    }
    pDestRegion->m_nLine++;

    const GXBOOL bTouchSrc1 = prlhSrc1->nBottom <= prlhSrc2->nBottom;
    const GXBOOL bTouchSrc2 = prlhSrc1->nBottom >= prlhSrc2->nBottom;

    if(bTouchSrc1)
    {
      prlhDest->nBottom = prlhSrc1->nBottom;
      if(prlhSrc1->nBottom == m_rcBounding.bottom)
      {
        clSwap((GXLONG_PTR&)prlhSrc1, (GXLONG_PTR&)prlhSrc2);
        break;
      }
      prlhSrc1 = prlhSrc1->Next();
    }

    if(bTouchSrc2)
    {
      prlhDest->nBottom = prlhSrc2->nBottom;
      if(prlhSrc2->nBottom == pRegion->m_rcBounding.bottom)
        break;
      prlhSrc2 = prlhSrc2->Next();
    }
  }
  if(m_rcBounding.bottom == pRegion->m_rcBounding.bottom)
  {
    goto FINAL_RET;
  }
  else if(prlhSrc2->nBottom == prlhSrc1->nBottom)  // 其实这个执行了 bTouchSrc2 这个分支, 
    prlhSrc1 = prlhSrc1->Next();        // 对于都到底边时,对 prlhSrc2 步进一次

  nTouchBottom = clMax(m_rcBounding.bottom, pRegion->m_rcBounding.bottom);

  while(1)
  {
    REGIONLINEHEAD* const prlhDest = 
      BeginLine(pDestRegion->m_aData, prlhSrc1->nCounts + 2);

    *prlhDest = *prlhSrc1;

    Line_Copy(pDestRegion->m_aData, prlhSrc1->DataPtr(), (size_t)prlhSrc1->nCounts);
    pDestRegion->m_nLine++;

    if(prlhSrc1->nBottom == nTouchBottom)
      break;
    prlhSrc1 = prlhSrc1->Next();
  }

FINAL_RET:
  pDestRegion->MergeFragment();
#if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  pDestRegion->DbgCheckEmptyRect();
#endif // #if defined(_DEBUG) && defined(ENABLE_CHECK_EMPTY_RECT)
  return FALSE;
}


GXBOOL GRegionImpl::PtInRegion(int x, int y) const
{
  GXPOINT pt = {x, y};
  const REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();

  if(gxPtInRect(&m_rcBounding, pt) == FALSE)
    return FALSE;

  while(true)
  {
    if(prlh->nBottom > y)
    {
      const REGIONLINE* prl = (REGIONLINE*)prlh->DataPtr();
      const size_t uCount = prlh->nCounts >> 1;
      for(size_t i = 0; i < uCount; i++)
      {
        if( x >= prl->left && x < prl->right )
          return TRUE;
        prl++;
      }
      break;
    }

    if(prlh->nBottom == m_rcBounding.bottom) {
      break;
    }
    prlh = prlh->Next();
  }//while(prlh->nBottom != m_rcBounding.bottom);
  return FALSE;
}

GXBOOL GRegionImpl::RectInRegion(const GXRECT* rect) const
{
  const REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  if( m_rcBounding.left >= rect->right ||
    m_rcBounding.right <= rect->left ||
    m_rcBounding.top >= rect->bottom ||
    m_rcBounding.bottom <= rect->top )
    return FALSE;

  do
  {
    if(prlh->nBottom < m_rcBounding.top)
      continue;
    
    const REGIONLINE* prl = (REGIONLINE*)prlh->DataPtr();
    const size_t uCount = prlh->nCounts >> 1;
    for(size_t i = 0; i < uCount; i++)
    {
      const long left = clMax(rect->left, prl->left);
      const long right = clMax(rect->left, prl->left);
      if( left < right )
        return TRUE;
      prl++;
    }

    if(prlh->nBottom >= m_rcBounding.bottom)
      break;
    prlh = prlh->Next();
  }while(prlh->nBottom != m_rcBounding.bottom);
  return FALSE;
}

GXUINT GRegionImpl::GetScan() const
{
  return m_nLine;
}

GXUINT GRegionImpl::GetData(clvector<GXRECT>& aRects) const
{
  GXUINT nLineTop = m_rcBounding.top;
  GXRECT rect;
  aRects.clear();
  if(m_nLine == 0)
    return 0;

  REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  GXUINT nLineBottom = (GXUINT)prlh->nBottom;
  for(GXINT j = 0; j < m_nLine; j++)
  {
    GXLONG* pData = (GXLONG*)((GXBYTE*)prlh + sizeof(REGIONLINEHEAD));
    //GXTRACE("第 %d 行 Rect: ", j);
    for(GXLONG i = 0; i < (GXLONG)prlh->nCounts; i+=2)
    {
      //GXTRACE("(%d, %d, %d, %d),", GETRGNLINE_REGION(lpRgnLine, i), nLineTop, GETRGNLINE_REGION(lpRgnLine, i + 1), nLineBottom);
      //if(nDataSize < nCount && lpRgnData != 0)
      rect.left   = *pData++;
      rect.top    = nLineTop;
      rect.right  = *pData++;
      rect.bottom = nLineBottom;

      aRects.push_back(rect);
    }
#ifdef _DEBUG
    REGIONLINEHEAD* prlhTemp = prlh;
#endif
    prlh = prlh->Next();
    //GXTRACE("[End]\n");
    //lpRgnLine = (REGIONLINEHEAD*)GETRGNLINE_NEXTLINE(lpRgnLine);
    nLineTop = nLineBottom;
    nLineBottom = (GXUINT)prlh->nBottom;
  }
  return (GXUINT)m_aData.size();
}

GXUINT GRegionImpl::GetRectCount() const
{
  return ((GXUINT)m_aData.size() >> 1) - m_nLine;
}

GXUINT GRegionImpl::GetRects(GXRECT* lpRects, int nCount) const
{
  GXUINT nLineTop = m_rcBounding.top;
  if(m_nLine == 0)
    return 0;

  REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  GXUINT nLineBottom = (GXUINT)prlh->nBottom;
  int nIndex = 0;
  for(GXINT j = 0; j < m_nLine; j++)
  {
    GXLONG* pData = (GXLONG*)((GXBYTE*)prlh + sizeof(REGIONLINEHEAD));
    //GXTRACE("第 %d 行 Rect: ", j);
    for(GXLONG i = 0; i < (GXLONG)prlh->nCounts; i+=2)
    {
      lpRects[nIndex].left  = *pData++;
      lpRects[nIndex].top    = nLineTop;
      lpRects[nIndex].right  = *pData++;
      lpRects[nIndex].bottom  = nLineBottom;
      nIndex++;
      if(nIndex >= nCount)
        goto FUN_RET;
    }

    prlh = prlh->Next();
    nLineTop = nLineBottom;
    nLineBottom = (GXUINT)prlh->nBottom;
  }
FUN_RET:
  return nIndex;
}

GXBOOL GRegionImpl::__cpp_MergeFragment()
{
  REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD* prlh2;
  size_t uDestIdx = 0;
  GXLONG nBottom = 0;

  //GXUINT uDbgPrevLine = m_nLine;  // Debug
  //size_t nDataSize = m_aData.size(); // Debug
  //REGIONLINEHEAD prlh_Dbg = *prlh; // Debug
  //void*pptr = &m_aData.front(); // Debug
  ////ASSERT((int)(m_aData.size() / 2 - m_nLine) > 0 || prlh->nCounts == 0);

  while(prlh->nCounts == 0 && m_nLine > 0)
  {
    m_rcBounding.top = (GXLONG)prlh->nBottom;
    prlh = prlh->Next();
    m_nLine--;
  }
  if(m_nLine <= 0)
  {
    m_aData.clear();
    gxSetRect(&m_rcBounding, 0, 0, 0, 0);
    return FALSE;
  }
  m_rcBounding.left  = LONG_MAX;
  m_rcBounding.right = LONG_MIN;

  prlh2 = prlh->Next();
  if(m_nLine > 1)
  {
    while(1)
    {
      if(prlh->nCounts != prlh2->nCounts)
        goto STEP_LINE;
      else
      {
        for(int i = 0; i < (int)prlh->nCounts; i++)
          if(prlh->Data(i) != prlh2->Data(i))
            goto STEP_LINE;

        m_nLine--;
        prlh->nBottom = prlh2->nBottom;
        goto NEXT_LINE2;
      }
STEP_LINE:
      nBottom = (GXLONG)prlh->nBottom;
      if(prlh->nCounts > 0)
      {
        m_rcBounding.left  = clMin(m_rcBounding.left, prlh->Data(0));
        m_rcBounding.right = clMax(m_rcBounding.right, prlh->Data((GXINT)prlh->nCounts - 1));

        // memcpy 之后 prlh 指向的地址可能被改变, 所以要预先保存
        const size_t uCount = (size_t)prlh->nCounts + 2;
        if(&m_aData[uDestIdx] != (GXLONG*)prlh)
          memcpy(&m_aData[uDestIdx], prlh, uCount * sizeof(GXLONG));
        uDestIdx += uCount;
      }
      else
      {
        m_aData[uDestIdx++] = prlh->nBottom;
        m_aData[uDestIdx++] = (GXLONG)prlh->nCounts;// TODO: 0 ?
      }

      prlh = prlh2;

NEXT_LINE2:
      if(prlh2->nBottom >= m_rcBounding.bottom)
        break;
      prlh2 = prlh2->Next();
    }
  }
  if(prlh->nCounts > 0)
  {
    m_rcBounding.left  = clMin(m_rcBounding.left, prlh->Data(0));
    m_rcBounding.right = clMax(m_rcBounding.right, prlh->Data((GXINT)prlh->nCounts - 1));
    m_rcBounding.bottom = (GXLONG)prlh->nBottom;

    const size_t uCount = (size_t)prlh->nCounts + 2;
    if(&m_aData[uDestIdx] != (GXLONG*)prlh)
      memcpy(&m_aData[uDestIdx], prlh, uCount * sizeof(GXLONG));
    uDestIdx += uCount;
  }
  else
  {
    m_rcBounding.bottom = nBottom;
    m_nLine--;
  }
  m_aData.resize(uDestIdx);
  return TRUE;
}
#if defined(_WIN32) && 0
GXBOOL GRegionImpl::__asm_MergeFragment()
{
  REGIONLINEHEAD*  prlh = (REGIONLINEHEAD*)&m_aData.front();
  REGIONLINEHEAD*  prlh2;
  size_t        uDestIdx = 0;
  GXLONG        nBottom;
  GRegionImpl*      pThis = this;

  while(prlh->nCounts == 0 && m_nLine > 0)
    //prlh->nBottom != m_rcBounding.bottom)
  {
    m_rcBounding.top = (GXLONG)prlh->nBottom;
    prlh = prlh->Next();
    m_nLine--;
  }
  if(m_nLine <= 0)
  {
    m_aData.clear();
    gxSetRect(&m_rcBounding, 0, 0, 0, 0);
    return FALSE;
  }
  m_rcBounding.left  = LONG_MAX;
  m_rcBounding.right = LONG_MIN;

  prlh2 = prlh->Next();
  if(m_nLine > 1)
  {
    while(1)
    {
      __asm
      {
        //if(prlh->nCounts != prlh2->nCounts)
        //  goto STEP_LINE;
        mov    edi, prlh
        mov    esi, prlh2
        mov    edx, [esi] + REGIONLINEHEAD::nBottom
        mov    ecx, [edi] + REGIONLINEHEAD::nCounts
        cmp    ecx, [esi] + REGIONLINEHEAD::nCounts
        jne    STEP_LINE
        jecxz  SAME_LINE

        //else
        //{
        //  for(int i = 0; i < (int)prlh->nCounts; i++)
        //    if(prlh->Data(i) != prlh2->Data(i))
        //      goto STEP_LINE;

        add    esi, 8  // sizeof(REGIONLINEHEAD)
        add    edi, 8  // sizeof(REGIONLINEHEAD)
        CLD
        repe  cmpsd
        //test  ecx, ecx
        jnz    STEP_LINE

SAME_LINE:
        mov    eax, pThis
        mov    edi, prlh
        dec    [eax] + m_nLine
        mov    [edi] + REGIONLINEHEAD::nBottom, edx
        jmp    NEXT_LINE2
        //  m_nLine--;
        //  prlh->nBottom = prlh2->nBottom;
        //  goto NEXT_LINE2;
        //}

      }
STEP_LINE:
      __asm 
      {
        //pushad

        //nBottom = (GXLONG)prlh->nBottom;
        mov    eax, prlh
        mov    esi, pThis
        mov    edi, [esi] + m_aData + LongFixedArray.m_ptr
        mov    edx, [eax] + REGIONLINEHEAD::nBottom
        mov    ecx, [eax] + REGIONLINEHEAD::nCounts
        mov    nBottom, edx
        //if(prlh->nCounts != 0)
        jecxz    EMPTY_LINE

        //{
        //  m_rcBounding.left  = Min(m_rcBounding.left, prlh->RawData(2));
        //  m_rcBounding.right = Max(m_rcBounding.right, prlh->RawData(prlh->nCounts + 1));

        mov    ebx, [esi] + m_rcBounding + RECT.left
        mov    edx, [esi] + m_rcBounding + RECT.right
        cmp    ebx, [eax] + 8  // sizeof(REGIONLINEHEAD)
        cmovg  ebx, [eax] + 8  // sizeof(REGIONLINEHEAD)
        cmp    edx, [eax + ecx * 4 + 4]
        cmovl  edx, [eax + ecx * 4 + 4]
        mov    [esi] + m_rcBounding + RECT.left, ebx
        mov    [esi] + m_rcBounding + RECT.right, edx

        //  // memcpy 之后 prlh 指向的地址可能被改变, 所以要预先保存
        //  const size_t uCount = (size_t)prlh->nCounts + 2;
        //  if(&m_aData[uDestIdx] != (GXLONG*)prlh)
        //    memcpy(&m_aData[uDestIdx], prlh, uCount * sizeof(GXLONG));
        //  uDestIdx += uCount;
        //}

        mov    edx, uDestIdx
        add    ecx, 2
        lea    edi, [edi + edx * 4]
        mov    esi, eax
        add    uDestIdx, ecx
        cmp    esi, edi
        je    END_COPY_LINE
        rep    movsd

        jmp    END_COPY_LINE
EMPTY_LINE:
        //else
        //  m_aData[uDestIdx++] = prlh->dwData;
        mov    eax, uDestIdx
        mov    [edi + eax * 4], edx  // [nBottom : 0]
        mov    DWORD PTR [edi + eax * 4 + 4], 0
        add    eax, 2
        mov    uDestIdx, eax
END_COPY_LINE:
        // prlh = prlh2;
        push  prlh2
        pop    prlh
        
        //popad
      }
      //__asm movzx eax, prlh->nBottom


NEXT_LINE2:
      if(prlh2->nBottom >= m_rcBounding.bottom)
        break;
      prlh2 = prlh2->Next();
    }
  }
  if(prlh->nCounts > 0)
  {
    m_rcBounding.left  = clMin(m_rcBounding.left, prlh->Data(0));
    m_rcBounding.right = clMax(m_rcBounding.right, prlh->Data((GXINT)prlh->nCounts - 1));
    m_rcBounding.bottom = (GXLONG)prlh->nBottom;

    const size_t uCount = (size_t)prlh->nCounts + 2;
    if(&m_aData[uDestIdx] != (GXLONG*)prlh)
      memcpy(&m_aData[uDestIdx], prlh, uCount * sizeof(GXLONG));
    uDestIdx += uCount;
  }
  else
  {
    m_rcBounding.bottom = nBottom;
    m_nLine--;
  }
  m_aData.resize(uDestIdx);
  return TRUE;
}
#endif // _WIN32
#ifdef _DEBUG
void GRegionImpl::DbgCheckEmptyRect()
{
  GXINT nLineTop = m_rcBounding.top;
  if(m_nLine == 0)
    return;

  REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  GXINT nLineBottom = (GXINT)prlh->nBottom;
  for(GXINT j = 0; j < m_nLine; j++)
  {
    GXLONG* pData = (GXLONG*)((GXBYTE*)prlh + sizeof(REGIONLINEHEAD));
    for(GXLONG i = 0; i < (GXLONG)prlh->nCounts; i+=2)
    {
      const GXINT nLeft = *pData++;
      const GXINT nRight = *pData++;
      if(nLeft == nRight || nLineTop == nLineBottom) {
        ASSERT(0);  // 有空的矩形,j:行数
      }
    }
    prlh = prlh->Next();
    nLineTop = nLineBottom;
    nLineBottom = (GXINT)prlh->nBottom;
  }
}

void GRegionImpl::DbgDumpRegion()
{
  GXINT nLineTop = m_rcBounding.top;
  if(m_nLine == 0)
    return;

  REGIONLINEHEAD* prlh = (REGIONLINEHEAD*)&m_aData.front();
  GXINT nLineBottom = (GXINT)prlh->nBottom;
  TRACE("LineCount:%d;(%d,%d)(%d,%d)\n", m_nLine, m_rcBounding.left, m_rcBounding.top, m_rcBounding.right, m_rcBounding.bottom);
  for(GXINT j = 0; j < m_nLine; j++)
  {
    TRACE("%3d: %d~%d ", j, nLineTop, nLineBottom);
    GXLONG* pData = (GXLONG*)((GXBYTE*)prlh + sizeof(REGIONLINEHEAD));
    for(GXLONG i = 0; i < (GXLONG)prlh->nCounts; i+=2)
    {
      const GXINT nLeft = *pData++;
      const GXINT nRight = *pData++;
      TRACE("(%d,%d);", nLeft, nRight);
    }
    TRACE("\n");
    prlh = prlh->Next();
    nLineTop = nLineBottom;
    nLineBottom = (GXINT)prlh->nBottom;
  }
  TRACE("Safe End\n");
}


void GRegionImpl::DbgFastCheck() const
{
  ASSERT((gxIsRectEmpty(&m_rcBounding) && m_nLine == 0 && m_aData.size() == 0) ||
    ( ! gxIsRectEmpty(&m_rcBounding) && m_nLine > 0 && m_aData.size() > 0));
}
#endif // #ifdef _DEBUG

void GRegionImpl::GetBounding(GXRECT* rect) const
{
  *rect = m_rcBounding;
}

GRegionImpl* GRegionImpl::CreateEmptyRgn(GAllocator* pAllocate)
{
  return new(pAllocate) GRegionImpl(pAllocate);
}

GRegionImpl* GRegionImpl::CreateRectRgn(GAllocator* pAllocate, const GXRECT& rect)
{
  return new(pAllocate) GRegionImpl(rect, pAllocate);
}

GRegionImpl* GRegionImpl::CreateRectRgnIndirect(GAllocator* pAllocate, const GXRECT* lpRects, const GXUINT nCount)
{
  return new(pAllocate) GRegionImpl(lpRects, nCount, pAllocate);
}

GRegionImpl* GRegionImpl::CreateRoundRectRgn(GAllocator* pAllocate, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse)
{
  return new(pAllocate) GRegionImpl(rect, nWidthEllipse, nHeightEllipse, pAllocate);
}

namespace GrapX
{
  class RegionFactoryImpl : public RegionFactory
  {
  private:
    GAllocator* m_pRgnAllocator;

  public:
    RegionFactoryImpl()
      : m_pRgnAllocator(new GAllocator(NULL))
    {
    }

    virtual ~RegionFactoryImpl()
    {
      SAFE_DELETE(m_pRgnAllocator);
    }

    // 基类接口
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXHRESULT AddRef() override
    {
      return gxInterlockedIncrement(&m_nRefCount);
    }
    
    GXHRESULT Release() override
    {
      GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
      if(nRefCount == 0) {
        delete this;
        return 0;
      }
      return nRefCount;
    }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXHRESULT CreateRectRgn(GRegion** ppRegion, const GXINT left, const GXINT top, const GXINT right, const GXINT bottom) override
    {
      GXRECT rect;
      rect.left = left;
      rect.top = top;
      rect.right = right;
      rect.bottom = bottom;

      if(gxIsRectEmpty(&rect)) {
        *ppRegion = (GRegion*)GRegionImpl::CreateEmptyRgn(m_pRgnAllocator);
      }
      else {
        *ppRegion = (GRegion*)GRegionImpl::CreateRectRgn(m_pRgnAllocator, rect);
      }
      if(*ppRegion != NULL) {
        return GX_OK;
      }
      return GX_FAIL;
    }

    GXHRESULT CreateRectRgnIndirect(GRegion** ppRegion, const GXRECT* lpRects, const GXUINT nCount) override
    {
      *ppRegion = (GRegion*)GRegionImpl::CreateRectRgnIndirect(m_pRgnAllocator, lpRects, nCount);
      if(*ppRegion != NULL) {
        return GX_OK;
      }
      return GX_FAIL;
    }

    GXHRESULT CreateRoundRectRgn(GRegion** ppRegion, const GXRECT& rect, const GXUINT nWidthEllipse, const GXUINT nHeightEllipse) override
    {
      *ppRegion = (GRegion*)GRegionImpl::CreateRoundRectRgn(m_pRgnAllocator, rect, nWidthEllipse, nHeightEllipse);
      if(*ppRegion != NULL) {
        return GX_OK;
      }
      return GX_FAIL;
    }
  };
  
  GXBOOL GXDLLAPI CreateRegionFactory(RegionFactory** ppFactory)
  {
    *ppFactory = new RegionFactoryImpl();
    if(InlIsFailedToNewObject(*ppFactory)) {
      return FALSE;
    }
    return TRUE;
  }

} // namespace GrapX