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

  template<typename _Ty>
  struct TRANNUMERIC
  {
    _Ty operator()(GXLPCWSTR str, GXSIZE_T len) const;
    _Ty operator()(GXLPCSTR str, GXSIZE_T len) const;
  };

  template<> struct TRANNUMERIC<int>
  {
    int operator()(GXLPCWSTR str, GXSIZE_T len) const   { return clstd::xtoi(10, str, len); }
    int operator()(GXLPCSTR str, GXSIZE_T len) const    { return clstd::xtoi(10, str, len); }
  };

  template<> struct TRANNUMERIC<long>
  {
    long operator()(GXLPCWSTR str, GXSIZE_T len) const  { return clstd::xtoi(10, str, len); }
    long operator()(GXLPCSTR str, GXSIZE_T len) const   { return clstd::xtoi(10, str, len); }
  };

  template<> struct TRANNUMERIC<i64>
  {
    i64 operator()(GXLPCWSTR str, GXSIZE_T len) const  { return clstd::xtoi64(10, str, len); }
    i64 operator()(GXLPCSTR str, GXSIZE_T len) const   { return clstd::xtoi64(10, str, len); }
  };

  //////////////////////////////////////////////////////////////////////////

  template<typename _T>
  struct RectT
  {
    typedef clvector<RectT<_T> >  Array;
    typedef cllist<RectT<_T> >    List;

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
      return (left >= right) || (top >= bottom);
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

    GXBOOL Union(const RectT& rect)
    {
      left   = clMin(left,   rect.left);
      top    = clMin(top,    rect.top);
      right  = clMax(right,  rect.right);
      bottom = clMax(bottom, rect.bottom);
      return ((left < right) && (top < bottom));
    }

    GXBOOL Union(const RectT& rect1, const RectT& rect2)
    {
      left   = clMin(rect1.left,   rect2.left);
      top    = clMin(rect1.top,    rect2.top);
      right  = clMax(rect1.right,  rect2.right);
      bottom = clMax(rect1.bottom, rect2.bottom);
      return ((left < right) && (top < bottom));
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

    GXBOOL IsEqual(const RectT& rect) const
    {
      return left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom;
    }

    RectT& Offset(_T x, _T y)
    {
      left   += x;
      top    += y;
      right  += x;
      bottom += y;
      return *this;
    }

    RectT& Inflate(_T dx, _T dy)
    {
      left   -= dx;
      top    -= dy;
      right  += dx;
      bottom += dy;
      return *this;
    }

    RectT& Parse(GXLPCWSTR str, GXSIZE_T len = -1, GXWCHAR ch = L',')
    {
      if(len == -1) {
        len = clstd::strlenT(str);
      }

      TRANNUMERIC<typename _T> t;
      clstd::StringUtility::Resolve(str, len, ch, [&t, this](GXSIZE_T i, GXLPCWSTR szText, GXSIZE_T sub_len){
        if(i < 4)
        {
          ((_T*)this)[i] = t(szText, sub_len);
        }
      });
      return *this;
    }
  };

  //////////////////////////////////////////////////////////////////////////

  template<typename _T>
  struct RegnT
  {
    typedef clvector<RegnT<_T> >  Array;
    typedef cllist<RegnT<_T> >    List;

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

    GXBOOL Union(const RegnT& regn)
    {
      const _T right1  = GetRight();
      const _T right2  = regn.GetRight();
      const _T bottom1 = GetBottom();
      const _T bottom2 = regn.GetBottom();

      // 必须这么写，防止left/top发生变化

      left   = clMin(left, regn.left);
      top    = clMin(top,  regn.top);
      width  = clMax(right1, right2) - left;
      height = clMax(bottom1, bottom2) - top;
      return ((width > 0) && (height > 0));
    }

    GXBOOL Union(const RegnT& regn1, const RegnT& regn2)
    {
      const _T right1  = regn1.GetRight();
      const _T right2  = regn2.GetRight();
      const _T bottom1 = regn1.GetBottom();
      const _T bottom2 = regn2.GetBottom();

      left   = clMin(regn1.left, regn2.left);
      top    = clMin(regn1.top,  regn2.top);
      width  = clMax(right1, right2) - left;
      height = clMax(bottom1, bottom2) - top;
      return ((width > 0) && (height > 0));
    }

    GXBOOL Intersect(const RegnT& regn)
    {
      const _T right1  = GetRight();
      const _T right2  = regn.GetRight();
      const _T bottom1 = GetBottom();
      const _T bottom2 = regn.GetBottom();

      // 必须这么写，防止left/top发生变化

      left   = clMax(left, regn.left);
      top    = clMax(top,  regn.top);
      width  = clMin(right1, right2) - left;
      height = clMin(bottom1, bottom2) - top;
      return ((width > 0) && (bottom > 0));
    }

    GXBOOL Intersect(const RegnT& regn1, const RegnT& regn2)
    {
      const _T right1  = regn1.GetRight();
      const _T right2  = regn2.GetRight();
      const _T bottom1 = regn1.GetBottom();
      const _T bottom2 = regn2.GetBottom();

      left   = clMax(regn1.left, regn2.left);
      top    = clMax(regn1.top,  regn2.top);
      width  = clMin(right1, right2) - left;
      height = clMin(bottom1, bottom2) - top;
      return ((width > 0) && (bottom > 0));
    }

    GXBOOL IsEqual(const RegnT& regn) const
    {
      return left == regn.left && top == regn.top && width == regn.width && height == regn.height;
    }

    RegnT& Offset(_T x, _T y)
    {
      left += x;
      top  += y;
      return *this;
    }

    RegnT& Inflate(_T dx, _T dy)
    {
      left   -= dx;
      top    -= dy;
      width  += (dx * (_T)2);
      height += (dy * (_T)2);
      return *this;
    }

    RegnT& Parse(GXLPCWSTR str, GXSIZE_T len = -1, GXWCHAR ch = L',')
    {
      if(len == -1) {
        len = clstd::strlenT(str);
      }

      TRANNUMERIC<typename _T> t;
      clstd::StringUtility::Resolve(str, len, ch, [&t, this](GXSIZE_T i, GXLPCWSTR szText, GXSIZE_T sub_len){
        if(i < 4)
        {
          ((_T*)this)[i] = t(szText, sub_len);
        }
      });
      return *this;
    }

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