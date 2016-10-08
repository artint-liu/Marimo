#ifndef _GREGION_H_
#define _GREGION_H_

// WIN32
//#define ERROR               0
//#define NULLREGION          1
//#define SIMPLEREGION        2
//#define COMPLEXREGION       3


class GRegion : public GUnknown
{
public:
  // 基类接口
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXSTDINTERFACE(GXHRESULT  AddRef         ());
  GXSTDINTERFACE(GXHRESULT  Release        ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  // 设置形状
  GXSTDINTERFACE(GXVOID     SetEmpty       ());
  GXSTDINTERFACE(GXVOID     SetRect        (const GXRECT* lprect));
  GXSTDINTERFACE(GXVOID     SetRoundRect   (const GXRECT* lprect, GXUINT nWidthEllipse, GXUINT nHeightEllipse));
  GXSTDINTERFACE(RGNCOMPLEX Offset         (GXINT xOffset, GXINT yOffset));

  // 状态分析
  GXSTDINTERFACE(GXBOOL     Equal          (const GRegion* pRegion) const);
  GXSTDINTERFACE(GXBOOL     IsEmpty        () const);
  GXSTDINTERFACE(GXBOOL     IsRectangle    () const);
  GXSTDINTERFACE(RGNCOMPLEX GetComplexity  () const);
  GXSTDINTERFACE(GXVOID     GetBounding    (GXRECT* rect) const);
  GXSTDINTERFACE(GXUINT     GetRectCount   () const);
  GXSTDINTERFACE(GXUINT     GetRects       (GXRECT* lpRects, int nCount) const);
  GXSTDINTERFACE(GXUINT     GetScan        () const);

  GXSTDINTERFACE(GXBOOL     PtInRegion     (int x, int y) const);
  GXSTDINTERFACE(GXBOOL     RectInRegion   (const GXRECT* rect) const);

  // 逻辑操作
  GXSTDINTERFACE(GRegion*   Clone           () const);
  GXSTDINTERFACE(GRegion*   CreateIntersect (const GRegion* pRegion) const);
  GXSTDINTERFACE(GRegion*   CreateXor       (const GRegion* pRegion) const);
  GXSTDINTERFACE(GRegion*   CreateSubtract  (const GRegion* pRegion) const);
  GXSTDINTERFACE(GRegion*   CreateUnion     (const GRegion* pRegion) const);

  GXSTDINTERFACE(RGNCOMPLEX Intersect       (const GRegion* pRegion));
  GXSTDINTERFACE(RGNCOMPLEX Xor             (const GRegion* pRegion));
  GXSTDINTERFACE(RGNCOMPLEX Subtract        (const GRegion* pRegion));
  GXSTDINTERFACE(RGNCOMPLEX Union           (const GRegion* pRegion));
};

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GREGION_H_
