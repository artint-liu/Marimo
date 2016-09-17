#ifndef _GRAPX_REGN_H_
#define _GRAPX_REGN_H_

#define REFACTOR_RECTREGN

namespace Marimo
{
  template<typename _T>
  struct RectT;

  template<typename _T>
  struct RegnT;

  //////////////////////////////////////////////////////////////////////////
  //
  // 聚合类
  //
  template<typename _T>
  struct RECTT
  {
    _T left, top, right, bottom;

    operator const RectT<_T>&() {
      return *this;
    }

    operator RectT<_T>&() {
      return *this;
    }

    operator const RectT<_T>*() {
      return reinterpret_cast<const RectT<_T>*>(this);
    }

    operator RectT<_T>*() {
      return reinterpret_cast<RectT<_T>*>(this);
    }
  };

  //////////////////////////////////////////////////////////////////////////

  template<typename _T>
  struct REGNT
  {
    _T left, top, width, height;

    operator const RegnT<_T>&() {
      return *this;
    }

    operator RegnT<_T>&() {
      return *this;
    }

    operator const RegnT<_T>*() {
      return reinterpret_cast<const RegnT<_T>*>(this);
    }

    operator RegnT<_T>*() {
      return reinterpret_cast<RegnT<_T>*>(this);
    }
  };    
  //////////////////////////////////////////////////////////////////////////

  template<typename _T>
  struct RectT
  {
    _T left, top, right, bottom;

    RectT() CLTRIVIAL_DEFAULT;

    RectT(_T v)
      : left(v), top(v), right(v), bottom(v)
    {}

    RectT(_T l, _T t, _T r, _T b)
      : left(l), top(t), right(r), bottom(b)
    {}

    template<typename _T2>
    RectT(const RECTT<_T2>& rect)
    {
      operator=(rect);
    }

    template<typename _T2>
    RectT(const REGNT<_T2>& regn)
    {
      operator=(regn);
    }

    template<typename _T2>
    RectT(const RegnT<_T2>& regn)
    {
      operator=(regn);
    }

    template<typename _T2>
    RectT& operator=(const RegnT<_T2>& regn)
    {
      left   = static_cast<_T>(regn.left);
      top    = static_cast<_T>(regn.top);
      right  = static_cast<_T>(regn.left + regn.width);
      bottom = static_cast<_T>(regn.top + regn.height);
      return *this;
    }

    template<typename _T2>
    RectT& operator=(const REGNT<_T2>& regn)
    {
      left   = static_cast<_T>(regn.left);
      top    = static_cast<_T>(regn.top);
      right  = static_cast<_T>(regn.left + regn.width);
      bottom = static_cast<_T>(regn.top + regn.height);
      return *this;
    }

    template<typename _T2>
    RectT& operator=(const RECTT<_T2>& rect)
    {
      left   = static_cast<_T>(rect.left);
      top    = static_cast<_T>(rect.top);
      right  = static_cast<_T>(rect.right);
      bottom = static_cast<_T>(rect.bottom);
      return *this;
    }

    RectT& set(_T v)
    {
      left = top = right = bottom = v;
      return *this;
    }

    RectT& set(_T l, _T t, _T r, _T b)
    {
      left = l; top = t; right = r; bottom = b;
      return *this;
    }

    GXBOOL IsEmpty() const
    {
      return (left <= right) || (top <= bottom);
    }

    GXBOOL IsPointIn(_T x, _T y) const
    {
      return (left <= x && right > x && top <= y && bottom > y);
    }

    _T GetWidth() const
    {
      return right - left;
    }

    _T GetHeight() const
    {
      return bottom - top;
    }

    GXBOOL Intersect(const RectT& rect)
    {
      left   = clMax(left,   rect.left);
      top    = clMax(top,    rect.top);
      right  = clMin(right,  rect.right);
      bottom = clMin(bottom, rect.bottom);
      return ((left < right) && (top < bottom));
    }

    GXBOOL Intersect(const RectT& rect1, const RectT& rect2)
    {
      left   = clMax(rect1.left,   rect2.left);
      top    = clMax(rect1.top,    rect2.top);
      right  = clMin(rect1.right,  rect2.right);
      bottom = clMin(rect1.bottom, rect2.bottom);
      return ((left < right) && (top < bottom));
    }

    GXBOOL IsIntersecting(const RectT& rect) const
    {
      return ((clMax(left, rect.left) < clMin(right, rect.right)) &&
        (clMax(top, rect.top) < clMin(bottom, rect.bottom)));
    }
  };

  //////////////////////////////////////////////////////////////////////////

  template<typename _T>
  struct RegnT
  {
    _T left, top, width, height;

    RegnT() CLTRIVIAL_DEFAULT;

    RegnT(_T v)
      : left(v), top(v), width(v), height(v)
    {}

    RegnT(_T l, _T t, _T w, _T h)
      : left(l), top(t), width(w), height(h)
    {}

    template<typename _T2>
    RegnT(const RECTT<_T2>& rect)
    {
      operator=(rect);
    }

    template<typename _T2>
    RegnT(const REGNT<_T2>& regn)
    {
      operator=(regn);
    }

    template<typename _T2>
    RegnT(const RectT<_T2>& rect)
    {
      operator=(rect);
    }
    
    template<typename _T2>
    RegnT& operator=(const RectT<_T2>& rect)
    {
      left   = static_cast<_T>(rect.left);
      top    = static_cast<_T>(rect.top);
      width  = static_cast<_T>(rect.right - rect.left);
      height = static_cast<_T>(rect.bottom - rect.top);
      return *this;
    }

    template<typename _T2>
    RegnT& operator=(const REGNT<_T2>& regn)
    {
      left   = static_cast<_T>(regn.left);
      top    = static_cast<_T>(regn.top);
      width  = static_cast<_T>(regn.width);
      height = static_cast<_T>(regn.height);
      return *this;
    }

    template<typename _T2>
    RegnT& operator=(const RECTT<_T2>& rect)
    {
      left   = static_cast<_T>(rect.left);
      top    = static_cast<_T>(rect.top);
      width  = static_cast<_T>(rect.right - rect.left);
      height = static_cast<_T>(rect.bottom - rect.top);
      return *this;
    }

    RegnT& set(_T v)
    {
      left = top = width = height = v;
      return *this;
    }

    RegnT& set(_T l, _T t, _T w, _T h)
    {
      left = l; top = t; width = w; height = h;
      return *this;
    }

    GXBOOL IsEmpty() const
    {
      return (width <= 0) || (height <= 0);
    }

    GXBOOL IsPointIn(_T x, _T y) const
    {
      return (left <= x && (left + width) > x && top <= y && (top + height) > y);
    }

    _T GetRight() const
    {
      return left + width;
    }

    _T GetBottom() const
    {
      return top + height;
    }

    //GXBOOL Intersect(const RegnT& regn)
    //{
    //  left   = clMax(left,   rect.left);
    //  top    = clMax(top,    rect.top);
    //  right  = clMin(right,  rect.right);
    //  bottom = clMin(bottom, rect.bottom);
    //  return ((left < right) && (top < bottom));
    //}

    //GXBOOL Intersect(const RegnT& regn1, const RegnT& regn2)
    //{
    //  left   = clMax(rect1.left,   rect2.left);
    //  top    = clMax(rect1.top,    rect2.top);
    //  right  = clMin(rect1.right,  rect2.right);
    //  bottom = clMin(rect1.bottom, rect2.bottom);
    //  return ((left < right) && (top < bottom));
    //}

  };

  //////////////////////////////////////////////////////////////////////////
  //
  // 保证转换的一致性  
  //
  STATIC_ASSERT(sizeof(RegnT<long>) == sizeof(REGNT<long>));
  STATIC_ASSERT(offsetof(RegnT<long>, left)   == offsetof(REGNT<long>, left));
  STATIC_ASSERT(offsetof(RegnT<long>, top)    == offsetof(REGNT<long>, top));
  STATIC_ASSERT(offsetof(RegnT<long>, width)  == offsetof(REGNT<long>, width));
  STATIC_ASSERT(offsetof(RegnT<long>, height) == offsetof(REGNT<long>, height));

  STATIC_ASSERT(sizeof(RectT<long>) == sizeof(RECTT<long>));
  STATIC_ASSERT(offsetof(RectT<long>, left)   == offsetof(RECTT<long>, left));
  STATIC_ASSERT(offsetof(RectT<long>, top)    == offsetof(RECTT<long>, top));
  STATIC_ASSERT(offsetof(RectT<long>, right)  == offsetof(RECTT<long>, right));
  STATIC_ASSERT(offsetof(RectT<long>, bottom) == offsetof(RECTT<long>, bottom));

  STATIC_ASSERT(sizeof(RegnT<float>) == sizeof(REGNT<float>));
  STATIC_ASSERT(sizeof(RectT<float>) == sizeof(RECTT<float>));
} // namespace Marimo

#endif // _GRAPX_REGN_H_