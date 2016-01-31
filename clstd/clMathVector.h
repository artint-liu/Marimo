#ifndef _CLSTD_MATH_VECTOR_IMPL_H_
#define _CLSTD_MATH_VECTOR_IMPL_H_

namespace clstd
{
  template<typename _Ty>
  struct _vector2
  {
    typedef _Ty Type;
    union
    {
      struct{
        _Ty x, y;
      };
      _Ty m[2];
    };
    static _vector2 AxisX;
    static _vector2 AxisY;
    static _vector2 Origin;

    _vector2() TRIVIAL_DEFAULT;
    _vector2(const _Ty v){x = y = v;};
    _vector2(const _Ty x, const _Ty y){this->x = x; this->y = y;};
    _vector2(const _vector2& v){x = v.x; y = v.y;};

    friend _vector2 operator + (const _vector2& v, const _Ty f){return _vector2(v.x + f, v.y + f);};
    friend _vector2 operator + (const _vector2& v1, const _vector2& v2){return _vector2(v1.x + v2.x, v1.y + v2.y);};

    _vector2 operator -() const { return _vector2(-x,-y); }
    _vector2 operator +() const { return *this; }
    bool operator <=(const _vector2& v){ return (x <= v.x && y <= v.y); }
    bool operator >=(const _vector2& v){ return (x >= v.x && y >= v.y); }

    bool operator == (const _vector2& v){ return (x == v.x && y == v.y);}
    bool operator != (const _vector2& v){ return (x != v.x || y != v.y);}
    bool operator == (const _Ty f){ return (x == f && y == f);}
    bool operator != (const _Ty f){ return (x != f || y != f);}

    _vector2& operator = (const _Ty v)  {x = y = v; return *this;}
    _vector2& operator = (const _vector2 v){x = v.x; y = v.y; return *this;}

    //operator const D3DXVECTOR2&() { return }

    _vector2& set(_Ty x, _Ty y){this->x = x; this->y = y; return *this;}
    _Ty dot(const _vector2& v) const{ return x * v.x + y * v.y; }
    _Ty lengthsquare() const { return x * x + y * y; }
    _Ty length() const { return sqrt(lengthsquare()); }
    friend _Ty dot(const _vector2& v1, const _vector2& v2){ return v1.x * v2.x + v1.y * v2.y; }
  };

  template<typename _Ty>
  struct _vector3
  {
    typedef _Ty Type;
    //STATIC_ASSERT(typeid(_Ty) == typeid(i32));
    union
    {
      struct{
        _Ty x, y, z;
      };
      _Ty m[3];
    };

    _vector3() TRIVIAL_DEFAULT;
    _vector3(const _Ty v){x = y = z = v;}
    _vector3(const _Ty x, const _Ty y, const _Ty z) {this->x = x; this->y = y; this->z = z;}
    _vector3(const _vector3& v){x = v.x; y = v.y; z = v.z;}
    //_vector3(const __Ty4& v);
    //_vector3(const unsigned long dwColor)
    //{ 
    //  x = (_Ty)((dwColor >> 16) & 0xff) / 255.0f;
    //  y = (_Ty)((dwColor >>  8) & 0xff) / 255.0f;
    //  z = (_Ty)( dwColor        & 0xff) / 255.0f;
    //}

    //operator const unsigned long()
    //{
    //  _vector3 t(*this);
    //  t.saturate();
    //  return (((unsigned long)(t.z * 255.0f)) | 
    //    ((((unsigned long)(t.y * 255.0f))) <<  8) | 
    //    ((((unsigned long)(t.x * 255.0f))) << 16) );
    //}
    //static _vector3 AxisX;
    //static _vector3 AxisY;
    //static _vector3 AxisZ;
    //static _vector3 Origin;

    friend _vector3 operator + (const _vector3& v, const _Ty f){return _vector3<_Ty>(v.x + f, v.y + f, v.z + f);};
    friend _vector3 operator + (const _vector3& v1, const _vector3& v2){return _vector3<_Ty>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);};

    friend _vector3 operator - (const _vector3& v, const _Ty f){return _vector3(v.x - f, v.y - f, v.z - f);}
    friend _vector3 operator - (const _vector3& v1, const _vector3& v2){return _vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);};

    friend _vector3 operator * (const _vector3& v, const _Ty f){return _vector3(v.x * f, v.y * f, v.z * f);};
    friend _vector3 operator * (const _vector3& v1, const _vector3& v2){return _vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);};

    friend _vector3 operator / (const _vector3& v, const _Ty f){return _vector3(v.x / f, v.y / f, v.z / f);};
    friend _vector3 operator / (const _vector3& v1, const _vector3& v2){return _vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);};

    friend _vector3 operator >> (const _vector3& v1, i32 shift) {return _vector3(v1.x >> shift, v1.y >> shift, v1.z >> shift);};
    friend _vector3 operator << (const _vector3& v1, i32 shift) {return _vector3(v1.x << shift, v1.y << shift, v1.z << shift);};
    friend _vector3 operator &  (const _vector3& v1, i32 mask)  {return _vector3(v1.x & mask, v1.y & mask, v1.z & mask);};

    //friend _vector3 operator * (const _vector3& v, const _TMat4x4& m)
    //{
    //  _float4 t;
    //  Vec3TransformCoord(&t, (const float3*)this, (const _float4x4*)&m);
    //  const float fInverseW = 1.0f / t.w;
    //  x = t.x * fInverseW;
    //  y = t.y * fInverseW;
    //  z = t.z * fInverseW;
    //  return *this;
    //}


    //_vector3<_Ty>& operator *= (const _Ty4x4& m);

    _vector3 operator -() const {return _vector3(-x,-y,-z);}
    _vector3 operator +() const {return *this;}
    bool operator <=(const _vector3& v) const { return (x <= v.x && y <= v.y && z <= v.z); }
    bool operator >=(const _vector3& v) const { return (x >= v.x && y >= v.y && z >= v.z); }
    bool operator <(const _vector3& v) const { return (x < v.x && y < v.y && z < v.z); }
    bool operator >(const _vector3& v) const { return (x > v.x && y > v.y && z > v.z); }

    bool operator <=(_Ty f) const { return (x <= f && y <= f && z <= f); }
    bool operator >=(_Ty f) const { return (x >= f && y >= f && z >= f); }
    bool operator <(_Ty f) const { return (x < f && y < f && z < f); }
    bool operator >(_Ty f) const { return (x > f && y > f && z > f); }

    bool operator == (const _vector3& v) const { return (x == v.x && y == v.y && z == v.z);}
    bool operator != (const _vector3& v) const { return (x != v.x || y != v.y || z != v.z);}
    bool operator == (const _Ty f) const { return (x == f && y == f && z == f);}
    bool operator != (const _Ty f) const { return (x != f || y != f || z != f);}

    _vector3& operator += (const _Ty f){x += f; y += f; z += f; return *this;}
    _vector3& operator += (const _vector3& v){x += v.x; y += v.y; z += v.z; return *this;}
    _vector3& operator -= (const _Ty f){x -= f; y -= f; z -= f; return *this;}
    _vector3& operator -= (const _vector3& v){x -= v.x; y -= v.y; z -= v.z; return *this;}
    _vector3& operator *= (const _Ty f){x *= f; y *= f; z *= f; return *this;}
    _vector3& operator *= (const _vector3& v){x *= v.x; y *= v.y; z *= v.z; return *this;}
    _vector3& operator /= (const _Ty f){x /= f; y /= f; z /= f; return *this;}
    _vector3& operator /= (const _vector3& v){x /= v.x; y /= v.y; z /= v.z; return *this;}
    _vector3& operator >>= (i32 shift){x >>= shift; y >>= shift; z >>= shift; return *this;}
    _vector3& operator <<= (i32 shift){x <<= shift; y <<= shift; z <<= shift; return *this;}
    _vector3& operator &= (i32 mask){x &= mask; y &= mask; z &= mask; return *this;}
    _vector3& operator = (const _Ty v)  {x = y = z = v; return *this;}
    _vector3& operator = (const _vector3 v){x = v.x; y = v.y; z = v.z; return *this;}
    //_vector3& operator = (const __Ty4 v);
    //_vector3& operator = (const unsigned long dwColor)
    //{ 
    //  x = (_Ty)((dwColor >> 16) & 0xff) / 255.0f;
    //  y = (_Ty)((dwColor >>  8) & 0xff) / 255.0f;
    //  z = (_Ty)( dwColor        & 0xff) / 255.0f;
    //  return *this;
    //}
    //__Ty2 xy() const {return __Ty2(x,y);}
    //__Ty2 yz() const {return __Ty2(y,z);}
    //__Ty2 zx() const {return __Ty2(z,x);}

    //__Ty2 yx() const {return __Ty2(y,x);}
    //__Ty2 zy() const {return __Ty2(z,y);}
    //__Ty2 xz() const {return __Ty2(x,z);}

    _vector3<_Ty>&  set         (_Ty x, _Ty y, _Ty z){this->x = x; this->y = y; this->z = z; return *this;}
    _Ty             LengthSquare() const { return x * x + y * y + z * z; }
    _Ty             length      () const { return sqrt(x * x + y * y + z * z);}
    _vector3&       saturate    (_Ty min, _Ty max)
    {
      if(x > max) x = max; else if(x < min) x = min;
      if(y > max) y = max; else if(y < min) y = min;
      if(z > max) z = max; else if(z < min) z = min;
      return *this;
    }

    _Ty dot(const _vector3& v) const{ return x * v.x + y * v.y + z * v.z; }
    _vector3 cross(const _vector3& v) const
    {
      return _vector3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x);
    }
    friend _Ty LengthSquare(const _vector3& v){ return v.x * v.x + v.y * v.y + v.z * v.z; }
    friend _Ty length(const _vector3& v){ return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);}
    friend _Ty dot(const _vector3& v1, const _vector3& v2){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
    friend _vector3 cross(const _vector3& v1, const _vector3& v2)
    {
      return _vector3(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
    }
    friend _vector3 normalize(const _vector3& v)
    {
      const _Ty l = v.length();
      if(l != 0.0f)
      {
        const _Ty fInvLen = 1.0f / l;
        return _vector3(v.x * fInvLen, v.y * fInvLen, v.z * fInvLen);
      }
      return _vector3(0.0f);
    }
    _Ty normalize()
    {
      const _Ty l = length();
      if(l != 0.0f)
      {
        const _Ty fInvLen = 1.0f / l;
        x *= fInvLen;
        y *= fInvLen;
        z *= fInvLen;
      }
      else
      {
        x = y = z = 0;
      }
      return l;
    }
    void Min(const _vector3& v)
    {
      x = x < v.x ? x : v.x;
      y = y < v.y ? y : v.y;
      z = z < v.z ? z : v.z;
    }

    void Max(const _vector3& v)
    {
      x = x > v.x ? x : v.x;
      y = y > v.y ? y : v.y;
      z = z > v.z ? z : v.z;
    }

    static _vector3 Min(const _vector3& v1, const _vector3& v2)
    {
      _vector3 v;
      v.x = v1.x < v2.x ? v1.x : v2.x;
      v.y = v1.y < v2.y ? v1.y : v2.y;
      v.z = v1.z < v2.z ? v1.z : v2.z;
      return v;
    }

    static _vector3 Max(const _vector3& v1, const _vector3& v2)
    {
      _vector3 v;
      v.x = v1.x > v2.x ? v1.x : v2.x;
      v.y = v1.y > v2.y ? v1.y : v2.y;
      v.z = v1.z > v2.z ? v1.z : v2.z;
      return v;
    }
  };

  template<typename _Ty>
  struct _vector4
  {
    typedef _Ty Type;
    union
    {
      struct{
        _Ty x, y, z, w;
      };
      _Ty m[4];
    };

    _vector4() TRIVIAL_DEFAULT;
    _vector4(const _Ty v){x = y = z = w = v;}
    _vector4(const _Ty x, const _Ty y, const _Ty z, const _Ty w) {this->x = x; this->y = y; this->z = z; this->w = w;}
    _vector4(const _vector4& v){x = v.x; y = v.y; z = v.z; w = v.w;}

    friend _vector4 operator+ (const _vector4& v, const _Ty f){return _vector4<_Ty>(v.x + f, v.y + f, v.z + f, v.w + f);};
    friend _vector4 operator+ (const _vector4& v1, const _vector4& v2){return _vector4<_Ty>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);};

    friend _vector4 operator- (const _vector4& v, const _Ty f){return _vector4(v.x - f, v.y - f, v.z - f, v.w - f);}
    friend _vector4 operator- (const _vector4& v1, const _vector4& v2){return _vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);};

    friend _vector4 operator* (const _vector4& v, const _Ty f){return _vector4(v.x * f, v.y * f, v.z * f, v.w * f);};
    friend _vector4 operator* (const _vector4& v1, const _vector4& v2){return _vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);};

    friend _vector4 operator/ (const _vector4& v, const _Ty f){return _vector4(v.x / f, v.y / f, v.z / f, v.w / f);};
    friend _vector4 operator/ (const _vector4& v1, const _vector4& v2){return _vector4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);};

    friend _vector4 operator>> (const _vector4& v1, int shift) {return _vector4(v1.x >> shift, v1.y >> shift, v1.z >> shift, v1.w >> shift);};
    friend _vector4 operator<< (const _vector4& v1, int shift) {return _vector4(v1.x << shift, v1.y << shift, v1.z << shift, v1.w << shift);};
    friend _vector4 operator&  (const _vector4& v1, int mask)  {return _vector4(v1.x & mask, v1.y & mask, v1.z & mask, v1.w & mask);};

    _vector4 operator-() const {return _vector4(-x,-y,-z,-w);}
    _vector4 operator+() const {return *this;}
    bool operator<=(const _vector4& v) const { return (x <= v.x && y <= v.y && z <= v.z && w <= v.w); }
    bool operator>=(const _vector4& v) const { return (x >= v.x && y >= v.y && z >= v.z && w >= v.w); }
    bool operator<(const _vector4& v) const { return (x < v.x && y < v.y && z < v.z && w < v.w); }
    bool operator>(const _vector4& v) const { return (x > v.x && y > v.y && z > v.z && w > v.w); }

    bool operator== (const _vector4& v) const { return (x == v.x && y == v.y && z == v.z && w == v.w);}
    bool operator!= (const _vector4& v) const { return (x != v.x || y != v.y || z != v.z || w != v.w);}
    bool operator== (const _Ty f) const { return (x == f && y == f && z == f && w == f);}
    bool operator!= (const _Ty f) const { return (x != f || y != f || z != f || w != f);}

    _vector4& operator+= (const _Ty f){x += f; y += f; z += f; w += f; return *this;}
    _vector4& operator+= (const _vector4& v){x += v.x; y += v.y; z += v.z; w += v.w; return *this;}
    _vector4& operator-= (const _Ty f){x -= f; y -= f; z -= f; w -= f; return *this;}
    _vector4& operator-= (const _vector4& v){x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;}
    _vector4& operator*= (const _Ty f){x *= f; y *= f; z *= f; w *= f; return *this;}
    _vector4& operator*= (const _vector4& v){x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this;}
    _vector4& operator/= (const _Ty f){x /= f; y /= f; z /= f; w /= f; return *this;}
    _vector4& operator/= (const _vector4& v){x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this;}
    _vector4& operator>>= (int shift){x >>= shift; y >>= shift; z >>= shift; w >>= shift; return *this;}
    _vector4& operator<<= (int shift){x <<= shift; y <<= shift; z <<= shift; w <<= shift; return *this;}
    _vector4& operator&= (int mask){x &= mask; y &= mask; z &= mask; w &= mask; return *this;}
    _vector4& operator= (const _Ty v)  {x = y = z = w = v; return *this;}
    _vector4& operator= (const _vector4& v){x = v.x; y = v.y; z = v.z; w = v.w; return *this;}

    _vector4<_Ty>&  set       (_Ty x, _Ty y, _Ty z, _Ty w){this->x = x; this->y = y; this->z = z; this->w = w; return *this;}
    _Ty             length    () const { return sqrt(x * x + y * y + z * z + w * w);}
    _vector4&       saturate  (_Ty min, _Ty max)
    {
      if(x > max) x = max; else if(x < min) x = min;
      if(y > max) y = max; else if(y < min) y = min;
      if(z > max) z = max; else if(z < min) z = min;
      if(w > max) w = max; else if(w < min) w = min;
      return *this;
    }

    _Ty dot(const _vector4& v) const{ return x * v.x + y * v.y + z * v.z + w * v.w; }
    friend _Ty length(const _vector4& v){ return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
    friend _Ty dot(const _vector4& v1, const _vector4& v2){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
    friend _vector4 normalize(const _vector4& v)
    {
      const _Ty l = v.length();
      if(l != 0.0f)
      {
        const _Ty fInvLen = 1.0f / l;
        return _vector4(v.x * fInvLen, v.y * fInvLen, v.z * fInvLen, v.w * fInvLen);
      }
      return _vector4(0.0f);
    }
    _Ty normalize()
    {
      const _Ty l = length();
      if(l != 0.0f)
      {
        const _Ty fInvLen = 1.0f / l;
        x *= fInvLen;
        y *= fInvLen;
        z *= fInvLen;
        w *= fInvLen;
      }
      else
      {
        x = y = z = w = 0;
      }
      return l;
    }
    void Min(const _vector4& v)
    {
      x = x < v.x ? x : v.x;
      y = y < v.y ? y : v.y;
      z = z < v.z ? z : v.z;
      w = w < v.w ? w : v.w;
    }

    void Max(const _vector4& v)
    {
      x = x > v.x ? x : v.x;
      y = y > v.y ? y : v.y;
      z = z > v.z ? z : v.z;
      w = w > v.w ? w : v.w;
    }
  }; // struct _vector4<_Ty>

  template<typename _Ty>
  struct _vector2_key : _vector2<_Ty>
  {
    _vector2_key(){}
    _vector2_key(const _vector2<_Ty>& v2)
    {
      *this = v2;
    }

    int compare(const _vector2_key& v2k) const
    {
      _Ty c;
      c = v2k.x - this->x;      if(c != 0) return c;
      c = v2k.y - this->y;      return c;
    }

#ifdef _DEBUG
    bool operator>= (const _vector2_key& v2k) const
    {
      // 这个要与 operator< 算法一致
      return compare(v2k) >= 0;
    }
#endif // #ifdef _DEBUG

    bool operator< (const _vector2_key& v2k) const
    {
      const bool bval = compare(v2k) < 0;

      ASSERT((bval && (v2k >= *this)) || 
        ( ! bval && ! (v2k >= *this)) || compare(v2k) == 0); // 使用交换法则验证算法
      return bval;
    }
  };

  //////////////////////////////////////////////////////////////////////////
  template<typename _Ty>
  struct _vector3_key : _vector3<_Ty>
  {
    _vector3_key(){}
    _vector3_key(const _vector3<_Ty>& v3)
    {
      *this = v3;
    }

    int compare(const _vector3_key& v3k) const
    {
      _Ty c;
      c = v3k.x - this->x;      if(c != 0) return c;
      c = v3k.y - this->y;      if(c != 0) return c;
      c = v3k.z - this->z;      return c;
    }

#ifdef _DEBUG
    bool operator>= (const _vector3_key& v3k) const
    {
      // 这个要与 operator< 算法一致
      return compare(v3k) >= 0;
    }
#endif // #ifdef _DEBUG

    bool operator< (const _vector3_key& v3k) const
    {
      const bool bval = compare(v3k) < 0;

      ASSERT((bval && (v3k >= *this)) || 
        ( ! bval && ! (v3k >= *this)) || compare(v3k) == 0); // 使用交换法则验证算法
      return bval;
    }
  };

  //////////////////////////////////////////////////////////////////////////
  template<typename _Ty>
  struct _vector4_key : _vector4<_Ty>
  {
    _vector4_key(){}
    _vector4_key(const _vector4<_Ty>& v4)
    {
      *this = v4;
    }

    int compare(const _vector4_key& v4k) const
    {
      _Ty c;
      c = v4k.x - this->x;      if(c != 0) return c;
      c = v4k.y - this->y;      if(c != 0) return c;
      c = v4k.z - this->z;      if(c != 0) return c;
      c = v4k.w - this->w;      return c;
    }

#ifdef _DEBUG
    bool operator>= (const _vector4_key& v4k) const
    {
      // 这个要与 operator< 算法一致
      return compare(v4k) >= 0;
    }
#endif // #ifdef _DEBUG

    bool operator< (const _vector4_key& v4k) const
    {
      const bool bval = compare(v4k) < 0;

      ASSERT((bval && (v4k >= *this)) || 
        ( ! bval && ! (v4k >= *this)) || compare(v4k) == 0); // 使用交换法则验证算法
      return bval;
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //template<typename _Ty>
  //inline size_t hash_value(const _vector2<_Ty>& v2)
  //{
  //  return clstd::GenerateCRC32((CLBYTE*)&v2, sizeof(_vector2<_Ty>));
  //}

  //template<typename _Ty>
  //inline size_t hash_value(const _vector3<_Ty>& v3)
  //{
  //  return clstd::GenerateCRC32((CLBYTE*)&v3, sizeof(_vector3<_Ty>));
  //}

  //template<typename _Ty>
  //inline size_t hash_value(const _vector4<_Ty>& v4)
  //{
  //  return clstd::GenerateCRC32((CLBYTE*)&v4, sizeof(_vector4<_Ty>));
  //}

} // namespace clstd

//typedef clstd::_vector3<float> float3;

#endif // _CLSTD_MATH_VECTOR_IMPL_H_