#ifndef _FLOAT_X_IMPL_H_
#define _FLOAT_X_IMPL_H_

#ifndef _CL_STD_CODE_
#error Must be include "clstd.h" first.
#endif // #ifndef _CL_STD_CODE_

#include <math.h>

namespace clstd
{
  struct _float2;
  struct _float3;
  struct _float4;
  struct _float4x4;
  struct _float3x3;
  struct _quaternion;
  struct _euler;
  struct TRANSFORM;

  struct _float2
  {
    // [二维直角坐标系]
    // 右手坐标系：x轴向右，y轴向上，常见于平面解析几何坐标系
    // 左手坐标系：x轴向右，y轴向下，常见于屏幕坐标系

    typedef float Type;
    union
    {
      struct{
        float x, y;
      };
      struct{
        float s, t;
      }g;
      struct{
        float u, v;
      }d;
      float m[2];
    };
    static _float2 AxisX;
    static _float2 AxisY;
    static _float2 Origin;

    _float2(){};
    _float2(const float v){x = y = v;};
    _float2(float _x, float _y) : x(_x), y(_y) {}
    _float2(const _float2& v){x = v.x; y = v.y;};
    explicit _float2(int _x, int _y) : x((float)_x), y((float)_y) {}

    friend _float2 operator + (const _float2& v, const float f){return _float2(v.x + f, v.y + f);};
    friend _float2 operator + (const _float2& v1, const _float2& v2){return _float2(v1.x + v2.x, v1.y + v2.y);};

    friend _float2 operator - (const _float2& v, const float f){return _float2(v.x - f, v.y - f);};
    friend _float2 operator - (const _float2& v1, const _float2& v2){return _float2(v1.x - v2.x, v1.y - v2.y);};

    friend _float2 operator * (const _float2& v, const float f){return _float2(v.x * f, v.y * f);};
    friend _float2 operator * (const _float2& v1, const _float2& v2){return _float2(v1.x * v2.x, v1.y * v2.y);};

    friend _float2 operator / (const _float2& v, const float f){return _float2(v.x / f, v.y / f);};
    friend _float2 operator / (const _float2& v1, const _float2& v2){return _float2(v1.x / v2.x, v1.y / v2.y);};

    _float2 operator -() const { return _float2(-x,-y); }
    _float2 operator +() const { return *this; }
    bool operator <=(const _float2& v) const { return (x <= v.x && y <= v.y); }
    bool operator >=(const _float2& v) const{ return (x >= v.x && y >= v.y); }
    bool operator <=(float fval) const { return (x <= fval && y <= fval); }
    bool operator >=(float fval) const { return (x >= fval && y >= fval); }
    bool operator <(float fval) const { return (x < fval && y < fval); }
    bool operator >(float fval) const { return (x > fval && y > fval); }

    bool IsAllLess        (float f){ return (x <  f && y <  f); }
    bool IsAllLessEqual   (float f){ return (x <= f && y <= f); }
    bool IsAllGreater     (float f){ return (x >  f && y >  f); }
    bool IsAllGreaterEqual(float f){ return (x >= f && y >= f); }
    bool IsAnyLess        (float f){ return (x <  f || y <  f); }
    bool IsAnyLessEqual   (float f){ return (x <= f || y <= f); }
    bool IsAnyGreater     (float f){ return (x >  f || y >  f); }
    bool IsAnyGreaterEqual(float f){ return (x >= f || y >= f); }

    bool IsAllLess        (const _float2& v){ return (x <  v.x && y <  v.y); }
    bool IsAllLessEqual   (const _float2& v){ return (x <= v.x && y <= v.y); }
    bool IsAllGreater     (const _float2& v){ return (x >  v.x && y >  v.y); }
    bool IsAllGreaterEqual(const _float2& v){ return (x >= v.x && y >= v.y); }
    bool IsAnyLess        (const _float2& v){ return (x <  v.x || y <  v.y); }
    bool IsAnyLessEqual   (const _float2& v){ return (x <= v.x || y <= v.y); }
    bool IsAnyGreater     (const _float2& v){ return (x >  v.x || y >  v.y); }
    bool IsAnyGreaterEqual(const _float2& v){ return (x >= v.x || y >= v.y); }

    bool operator == (const _float2& v) const { return (x == v.x && y == v.y);}
    bool operator != (const _float2& v) const { return (x != v.x || y != v.y);}
    bool operator == (const float f) const { return (x == f && y == f);}
    bool operator != (const float f) const { return (x != f || y != f);}

    _float2& operator += (const float f){x += f; y += f; return *this;}
    _float2& operator += (const _float2& v){x += v.x; y += v.y; return *this;}
    _float2& operator -= (const float f){x -= f; y -= f; return *this;}
    _float2& operator -= (const _float2& v){x -= v.x; y -= v.y; return *this;}
    _float2& operator *= (const float f){x *= f; y *= f; return *this;}
    _float2& operator *= (const _float2& v){x *= v.x; y *= v.y; return *this;}
    _float2& operator /= (const float f){x /= f; y /= f; return *this;}
    _float2& operator /= (const _float2& v){x /= v.x; y /= v.y; return *this;}
    _float2& operator = (const float v)  {x = y = v; return *this;}
    _float2& operator = (const _float2 v){x = v.x; y = v.y; return *this;}

    //operator const D3DXVECTOR2&() { return }
    _float2& abs()
    {
      x = fabs(x);
      y = fabs(y);
      return *this;
    }

    static _float2 abs(const _float2& v)
    {
      _float2 t = v;
      return t.abs();
    }


    float dot(const _float2& v) const{ return x * v.x + y * v.y; }
    static float dot(const _float2& v1, const _float2& v2){ return v1.x * v2.x + v1.y * v2.y; }
    float lengthsquare() const { return x * x + y * y; }
    float length() const { return sqrt(lengthsquare()); }
    static _float2 normalize(const _float2& v)
    {
      const float l = v.length();
      if(l != 0.0f)
      {
        const float fInvLen = 1.0f / l;
        return _float2(v.x * fInvLen, v.y * fInvLen);
      }
      return _float2(0.0f);
    }

    float normalize()
    {
      const float l = length();
      if(l != 0.0f)
      {
        const float fInvLen = 1.0f / l;
        x *= fInvLen;
        y *= fInvLen;
      }
      else
      {
        x = y = 0;
      }
      return l;
    }

    // 设置为p0指向p1的向量的垂线
    // 右手坐标系里法线在(p1-p0)的顺时针方向
    _float2& perpendicular(const _float2& p0, const _float2& p1)
    {
      x = p1.y - p0.y;
      y = p0.x - p1.x;
      return *this;
    }

    // 设置为p0指向p1的向量的法线, 返回值是模
    // 右手坐标系里法线在(p1-p0)的顺时针方向
    float normal(const _float2& p0, const _float2& p1)
    {
      perpendicular(p0, p1);
      return normalize();
    }

    // 求作为向量时，v是否在this的顺时针方向, 
    // 右手坐标系，大于0是顺时针，等于0是重合，小于0是逆时针
    float IsClockwise(const _float2& v) const
    {
      return y * v.x - x * v.y;
    }

    _float2& set(float x, float y){this->x = x; this->y = y; return *this;}
    _float2& set(int x, int y){this->x = (float)x; this->y = (float)y; return *this;}

    b32 IsNearTo(const _float2& v, float epsilon) const
    {
      ASSERT(epsilon >= 0);
      return fabs(x - v.x) < epsilon && fabs(y - v.y) < epsilon;
    }

    Type GetMinScalar() const // 获得分量中最小的标量
    {
      return x < y ? x : y;
    }

    Type GetMaxScalar() const // 获得分量中最大的标量
    {
      return x > y ? x : y;
    }
  };
  //////////////////////////////////////////////////////////////////////////
  struct _float3
  {
    typedef float Type;
    union
    {
      struct{
        Type x, y, z;
      };
      struct{
        Type s, t, p;
      }gluv;
      struct{
        Type r, g, b;
      }rgb;
      struct{
        Type h, s, v;
      }hsv;
      Type m[3];
    };
    static _float3 AxisX;
    static _float3 AxisY;
    static _float3 AxisZ;
    static _float3 Origin;

    _float3(){};
    _float3(const float v){x = y = z = v;}
    _float3(const float x, const float y, const float z) {this->x = x; this->y = y; this->z = z;}
    _float3(const _float3& v){x = v.x; y = v.y; z = v.z;}
    _float3(const _float4& v);
    _float3(const unsigned long dwColor)
    { 
      x = (float)((dwColor >> 16) & 0xff) / 255.0f;
      y = (float)((dwColor >>  8) & 0xff) / 255.0f;
      z = (float)( dwColor        & 0xff) / 255.0f;
    }

    //operator const unsigned long()
    //{
    //  _float3 t(*this);
    //  t.saturate();
    //  return (((unsigned long)(t.z * 255.0f)) | 
    //    ((((unsigned long)(t.y * 255.0f))) <<  8) | 
    //    ((((unsigned long)(t.x * 255.0f))) << 16) );
    //}

    friend _float3 operator + (const _float3& v, const float f){return _float3(v.x + f, v.y + f, v.z + f);};
    friend _float3 operator + (const _float3& v1, const _float3& v2){return _float3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);};

    friend _float3 operator - (const _float3& v, const float f){return _float3(v.x - f, v.y - f, v.z - f);}
    friend _float3 operator - (const _float3& v1, const _float3& v2){return _float3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);};

    friend _float3 operator * (const _float3& v, const float f){return _float3(v.x * f, v.y * f, v.z * f);};
    friend _float3 operator * (const _float3& v1, const _float3& v2){return _float3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);};

    friend _float3 operator / (const _float3& v, const float f){return _float3(v.x / f, v.y / f, v.z / f);};
    friend _float3 operator / (const _float3& v1, const _float3& v2){return _float3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);};

    friend _float3 operator * (const _float3& v, const _float4x4& m);
    //friend _float3 operator * (const _float3& v, const _float3x3& m);

    _float3& operator *= (const _float4x4& m);
    //_float3& operator *= (const _float3x3& m);

    _float3 MulAsMatrix3x3(const _float4x4& m) const;

    _float3 operator -() const {return _float3(-x,-y,-z);}
    _float3 operator +() const {return *this;}
    bool operator <=(const _float3& v) const { return (x <= v.x && y <= v.y && z <= v.z); }
    bool operator >=(const _float3& v) const { return (x >= v.x && y >= v.y && z >= v.z); }
    bool operator <=(float fval) const { return (x <= fval && y <= fval && z <= fval); }
    bool operator >=(float fval) const { return (x >= fval && y >= fval && z >= fval); }
    bool operator <(const _float3& v) const { return (x < v.x && y < v.y && z < v.z); }
    bool operator >(const _float3& v) const { return (x > v.x && y > v.y && z > v.z); }
    bool operator <(float fval) const { return (x < fval && y < fval && z < fval); }
    bool operator >(float fval) const { return (x > fval && y > fval && z > fval); }

    bool IsAllLess        (float f){ return (x <  f && y <  f && z <  f); }
    bool IsAllLessEqual   (float f){ return (x <= f && y <= f && z <= f); }
    bool IsAllGreater     (float f){ return (x >  f && y >  f && z >  f); }
    bool IsAllGreaterEqual(float f){ return (x >= f && y >= f && z >= f); }
    bool IsAnyLess        (float f){ return (x <  f || y <  f || z <  f); }
    bool IsAnyLessEqual   (float f){ return (x <= f || y <= f || z <= f); }
    bool IsAnyGreater     (float f){ return (x >  f || y >  f || z >  f); }
    bool IsAnyGreaterEqual(float f){ return (x >= f || y >= f || z >= f); }

    bool IsAllLess        (const _float3& v){ return (x <  v.x && y <  v.y && z <  v.z); }
    bool IsAllLessEqual   (const _float3& v){ return (x <= v.x && y <= v.y && z <= v.z); }
    bool IsAllGreater     (const _float3& v){ return (x >  v.x && y >  v.y && z >  v.z); }
    bool IsAllGreaterEqual(const _float3& v){ return (x >= v.x && y >= v.y && z >= v.z); }
    bool IsAnyLess        (const _float3& v){ return (x <  v.x || y <  v.y || z <  v.z); }
    bool IsAnyLessEqual   (const _float3& v){ return (x <= v.x || y <= v.y || z <= v.z); }
    bool IsAnyGreater     (const _float3& v){ return (x >  v.x || y >  v.y || z >  v.z); }
    bool IsAnyGreaterEqual(const _float3& v){ return (x >= v.x || y >= v.y || z >= v.z); }

    bool operator == (const _float3& v) const { return (x == v.x && y == v.y && z == v.z);}
    bool operator != (const _float3& v) const { return (x != v.x || y != v.y || z != v.z);}
    bool operator == (const float f) const { return (x == f && y == f && z == f);}
    bool operator != (const float f) const { return (x != f || y != f || z != f);}

    _float3& operator += (const float f){x += f; y += f; z += f; return *this;}
    _float3& operator += (const _float3& v){x += v.x; y += v.y; z += v.z; return *this;}
    _float3& operator -= (const float f){x -= f; y -= f; z -= f; return *this;}
    _float3& operator -= (const _float3& v){x -= v.x; y -= v.y; z -= v.z; return *this;}
    _float3& operator *= (const float f){x *= f; y *= f; z *= f; return *this;}
    _float3& operator *= (const _float3& v){x *= v.x; y *= v.y; z *= v.z; return *this;}
    _float3& operator /= (const float f){x /= f; y /= f; z /= f; return *this;}
    _float3& operator /= (const _float3& v){x /= v.x; y /= v.y; z /= v.z; return *this;}
    _float3& operator = (const float v)  {x = y = z = v; return *this;}
    _float3& operator = (const _float3 v){x = v.x; y = v.y; z = v.z; return *this;}
    _float3& operator = (const _float4 v);
    _float3& operator = (const unsigned long dwColor)
    { 
      x = (float)((dwColor >> 16) & 0xff) / 255.0f;
      y = (float)((dwColor >>  8) & 0xff) / 255.0f;
      z = (float)( dwColor        & 0xff) / 255.0f;
      return *this;
    }
    _float2 xy() const {return _float2(x,y);}
    _float2 yz() const {return _float2(y,z);}
    _float2 zx() const {return _float2(z,x);}

    _float2 yx() const {return _float2(y,x);}
    _float2 zy() const {return _float2(z,y);}
    _float2 xz() const {return _float2(x,z);}

    _float3& set(float x, float y, float z)
    {
      this->x = x; this->y = y; this->z = z; return *this;
    }

    _float3& set(float f)
    {
      this->x = this->y = this->z = f; return *this;
    }

    _float3 cross(const _float3& v) const
    {
      return _float3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x);
    }

    static _float3 cross(const _float3& v1, const _float3& v2)
    {
      return _float3(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
    }

    float dot(const _float3& v) const
    { 
      return x * v.x + y * v.y + z * v.z; 
    }

    static float dot(const _float3& v1, const _float3& v2)
    { 
      return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float length() const 
    { 
      return sqrt(lengthsquare()); 
    }

    static float length(const _float3& v)
    { 
      return sqrt(v.lengthsquare());
    }

    float lengthsquare() const 
    { 
      return x * x + y * y + z * z; 
    }

    static float lengthsquare(const _float3& v)
    { 
      return v.lengthsquare(); 
    }

    float normalize()
    {
      const float l = length();
      if(l != 0.0f)
      {
        const float fInvLen = 1.0f / l;
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

    static _float3 normalize(const _float3& v)
    {
      const float l = v.length();
      if(l != 0.0f)
      {
        const float fInvLen = 1.0f / l;
        return _float3(v.x * fInvLen, v.y * fInvLen, v.z * fInvLen);
      }
      return _float3(0.0f);
    }

    _float3& abs()
    {
      x = fabs(x);
      y = fabs(y);
      z = fabs(z);
      return *this;
    }

    static _float3 abs(const _float3& v)
    {
      _float3 t = v;
      return t.abs();
    }

    _float3& saturate()
    {
      if(x > 1.0f) x = 1.0f; else if(x < 0.0f) x = 0.0f;
      if(y > 1.0f) y = 1.0f; else if(y < 0.0f) y = 0.0f;
      if(z > 1.0f) z = 1.0f; else if(z < 0.0f) z = 0.0f;
      return *this;
    }

    static _float3 saturate(const _float3& v)
    {
      _float3 t;
      if(v.x > 1.0f) t.x = 1.0f; else if(v.x < 0.0f) t.x = 0.0f;
      if(v.y > 1.0f) t.y = 1.0f; else if(v.y < 0.0f) t.y = 0.0f;
      if(v.z > 1.0f) t.z = 1.0f; else if(v.z < 0.0f) t.z = 0.0f;
      return t;
    }

    _float4 transform(const _float4x4& m) const;


    friend _float3 Min(const _float3& v1, const _float3& v2)
    {
      return _float3(
        v1.x < v2.x ? v1.x : v2.x,
        v1.y < v2.y ? v1.y : v2.y,
        v1.z < v2.z ? v1.z : v2.z);
    }

    friend _float3 Max(const _float3& v1, const _float3& v2)
    {
      return _float3(
        v1.x > v2.x ? v1.x : v2.x,
        v1.y > v2.y ? v1.y : v2.y,
        v1.z > v2.z ? v1.z : v2.z);
    }
    void Min(const _float3& v);
    void Max(const _float3& v);

    Type GetMinScalar() const // 获得分量中最小的标量
    {
      return x < y ? (x < z ? x : z) : (y < z ? y : z);
    }

    Type GetMaxScalar() const // 获得分量中最大的标量
    {
      return x > y ? (x > z ? x : z) : (y > z ? y : z);
    }
  };
  //////////////////////////////////////////////////////////////////////////
  struct _float4
  {
    typedef float Type;
    union
    {
      struct{
        float x, y, z, w;
      };
      struct{
        float s, t, p, q;
      }gluv;
      struct{
        float r, g, b, a;
      }rgba;
      struct{
        float h, s, v, a;
      }hsva;
      float m[4];
    };
    static _float4 AxisX;
    static _float4 AxisY;
    static _float4 AxisZ;
    static _float4 Origin;

    _float4(){};
    _float4(const float v){x = y = z = w = v;}
    _float4(const float x, const float y, const float z, const float w){this->x = x; this->y = y; this->z = z; this->w = w;}
    _float4(const _float3& v){x = v.x; y = v.y; z = v.z; w = 1.0f;}
    _float4(const _float3& v, float vw){x = v.x; y = v.y; z = v.z; w = vw;}
    _float4(const _float4& v){x = v.x; y = v.y; z = v.z; w = v.w;}
    _float4(const unsigned long dwColor)
    { 
      rgba.a = (float)((dwColor >> 24) & 0xff) / 255.0f;
      rgba.r = (float)((dwColor >> 16) & 0xff) / 255.0f;
      rgba.g = (float)((dwColor >>  8) & 0xff) / 255.0f;
      rgba.b = (float)( dwColor        & 0xff) / 255.0f;
    }

    operator const unsigned long()
    {
      _float4 t(*this);
      t.saturate();
      return (((unsigned long)(t.rgba.b * 255.0f)) | 
        ((((unsigned long)(t.rgba.g * 255.0f))) <<  8) | 
        ((((unsigned long)(t.rgba.r * 255.0f))) << 16) | 
        ((((unsigned long)(t.rgba.a * 255.0f))) << 24) );
    }

    bool operator == (const _float4& v){ return (x == v.x && y == v.y && z == v.z && w == v.w);}
    bool operator != (const _float4& v){ return (x != v.x || y != v.y || z != v.z || w != v.w);}
    bool operator == (const float f){ return (x == f && y == f && z == f && w == f);}
    bool operator != (const float f){ return (x != f || y != f || z != f || w != f);}


    friend _float4 operator + (const _float4& v, const float f){return _float4(v.x + f, v.y + f, v.z + f, v.w + f);};
    friend _float4 operator + (const _float4& v1, const _float4& v2){return _float4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);};

    friend _float4 operator - (const _float4& v, const float f){return _float4(v.x - f, v.y - f, v.z - f, v.w - f);}
    friend _float4 operator - (const _float4& v1, const _float4& v2){return _float4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);};

    friend _float4 operator * (const _float4& v, const float f){return _float4(v.x * f, v.y * f, v.z * f, v.w * f);};
    friend _float4 operator * (const _float4& v1, const _float4& v2){return _float4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);};

    friend _float4 operator / (const _float4& v, const float f){return _float4(v.x / f, v.y / f, v.z / f, v.w / f);};
    friend _float4 operator / (const _float4& v1, const _float4& v2){return _float4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);};

    _float4 operator -() const { return _float4(-x,-y,-z,-w); }
    _float4 operator +() const { return *this; }
    bool operator <(const _float4& v) const { return (x < v.x && y < v.y && z < v.z && w < v.w); }
    bool operator >(const _float4& v) const { return (x > v.x && y > v.y && z > v.z && w > v.w); }
    bool operator <(float fval) const { return (x < fval && y < fval && z < fval && w < fval); }
    bool operator >(float fval) const { return (x > fval && y > fval && z > fval && w > fval); }
    bool operator <=(const _float4& v) const { return (x <= v.x && y <= v.y && z <= v.z && w <= v.w); }
    bool operator >=(const _float4& v) const { return (x >= v.x && y >= v.y && z >= v.z && w >= v.w); }
    bool operator <=(float fval) const { return (x <= fval && y <= fval && z <= fval && w <= fval); }
    bool operator >=(float fval) const { return (x >= fval && y >= fval && z >= fval && w >= fval); }

    bool IsAllLess        (float f){ return (x <  f && y <  f && z <  f && w <  f); }
    bool IsAllLessEqual   (float f){ return (x <= f && y <= f && z <= f && w <= f); }
    bool IsAllGreater     (float f){ return (x >  f && y >  f && z >  f && w >  f); }
    bool IsAllGreaterEqual(float f){ return (x >= f && y >= f && z >= f && w >= f); }
    bool IsAnyLess        (float f){ return (x <  f || y <  f || z <  f || w <  f); }
    bool IsAnyLessEqual   (float f){ return (x <= f || y <= f || z <= f || w <= f); }
    bool IsAnyGreater     (float f){ return (x >  f || y >  f || z >  f || w >  f); }
    bool IsAnyGreaterEqual(float f){ return (x >= f || y >= f || z >= f || w >= f); }

    bool IsAllLess        (const _float4& v){ return (x <  v.x && y <  v.y && z <  v.z && w <  v.w); }
    bool IsAllLessEqual   (const _float4& v){ return (x <= v.x && y <= v.y && z <= v.z && w <= v.w); }
    bool IsAllGreater     (const _float4& v){ return (x >  v.x && y >  v.y && z >  v.z && w >  v.w); }
    bool IsAllGreaterEqual(const _float4& v){ return (x >= v.x && y >= v.y && z >= v.z && w >= v.w); }
    bool IsAnyLess        (const _float4& v){ return (x <  v.x || y <  v.y || z <  v.z || w <  v.w); }
    bool IsAnyLessEqual   (const _float4& v){ return (x <= v.x || y <= v.y || z <= v.z || w <= v.w); }
    bool IsAnyGreater     (const _float4& v){ return (x >  v.x || y >  v.y || z >  v.z || w >  v.w); }
    bool IsAnyGreaterEqual(const _float4& v){ return (x >= v.x || y >= v.y || z >= v.z || w >= v.w); }

    _float4& operator += (const float f){x += f; y += f; z += f; w += f; return *this;}
    _float4& operator += (const _float4& v){x += v.x; y += v.y; z += v.z; w += v.w; return *this;}
    _float4& operator -= (const float f){x -= f; y -= f; z -= f; w -= f; return *this;}
    _float4& operator -= (const _float4& v){x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;}
    _float4& operator *= (const float f){x *= f; y *= f; z *= f; w *= f; return *this;}
    _float4& operator *= (const _float4& v){x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this;}
    _float4& operator /= (const float f){x /= f; y /= f; z /= f; w /= f; return *this;}
    _float4& operator /= (const _float4& v){x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this;}
    _float4& operator = (const float v)  {x = y = z = w = v; return *this;}
    _float4& operator = (const _float3 v){x = v.x; y = v.y; z = v.z; w = 1.0f; return *this;}
    _float4& operator = (const _float4 v){x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
    _float4& operator = (const unsigned long dwColor)
    { 
      rgba.a = (float)((dwColor >> 24) & 0xff) / 255.0f;
      rgba.r = (float)((dwColor >> 16) & 0xff) / 255.0f;
      rgba.g = (float)((dwColor >>  8) & 0xff) / 255.0f;
      rgba.b = (float)( dwColor        & 0xff) / 255.0f;
      return *this;
    }

    _float4& set(float x, float y, float z, float w){this->x = x; this->y = y; this->z = z; this->w = w; return *this;}
    float lengthsquare() const { return x * x + y * y + z * z + w * w; }
    float length() const { return sqrt(lengthsquare()); }
    _float4& abs()
    {
      x = fabs(x);
      y = fabs(y);
      z = fabs(z);
      w = fabs(w);
      return *this;
    }

    static _float4 abs(const _float4& v)
    {
      _float4 t = v;
      return t.abs();
    }

    _float4& saturate()
    {
      if(x > 1.0f) x = 1.0f; else if(x < 0.0f) x = 0.0f;
      if(y > 1.0f) y = 1.0f; else if(y < 0.0f) y = 0.0f;
      if(z > 1.0f) z = 1.0f; else if(z < 0.0f) z = 0.0f;
      if(w > 1.0f) w = 1.0f; else if(w < 0.0f) w = 0.0f;
      return *this;
    }
    _float4 transform(const _float4x4& m) const;
    float dot(const _float4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }
    static float dot(const _float4& v1, const _float4& v2){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }

    float normalize()
    {
      const float l = length();
      if(l != 0.0f)
      {
        const float fInvLen = 1.0f / l;
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

    Type GetMinScalar() const // 获得分量中最小的标量
    {
      Type t = x < y ? x : y;
      t = t < z ? t : z;
      t = t < w ? t : w;
      return t;
    }

    Type GetMaxScalar() const // 获得分量中最大的标量
    {
      Type t = x > y ? x : y;
      t = t > z ? t : z;
      t = t > w ? t : w;
      return t;
    }
  };
} // namespace clstd

//namespace clstd
//{
//  struct transform
//  {
//    _float3 vTranslation;
//    _float3 vScaling;
//    _float3 vEuler;
//  };
//} // using namespace clstd

typedef clstd::_float2 float2;
typedef clstd::_float3 float3;
typedef clstd::_float4 float4;
typedef clstd::_float3x3 float3x3;
typedef clstd::_float4x4 float4x4;
//typedef clstd::transform transform;

typedef const float2      CFloat2;
typedef const float3      CFloat3;
typedef const float4      CFloat4;
typedef const float3x3    CFloat3x3;
typedef const float4x4    CFloat4x4;

template <typename _Tv, typename _Tl>
inline _Tv Lerp(const _Tv& t1, const _Tv& t2, const _Tl& l)
{
  return (_Tv)((t2 - t1) * l + t1);
}

namespace clstd
{
  _float4*    Vec3TransformCoord        (_float4* pout, CLCONST _float3* pv, CLCONST _float4x4* pm);
  float3*     Vec3TransformNormal       (_float3* pout, CLCONST _float3* pv, CLCONST _float4x4* pm);
  _float3*    Vec3Normalize             (_float3* pout, CLCONST _float3* pv);
  _float3*    Vec3Subtract              (_float3* pOut, CLCONST _float3* pV1, CLCONST _float3* pV2);
  _float3*    Vec3Cross                 (_float3* pOut, CLCONST _float3* pV1, CLCONST _float3* pV2);
  float       Vec3Dot                   (CLCONST _float3* pV1, CLCONST _float3* pV2);
  float       Vec3Length                (CLCONST _float3 *pV);

  float4*     Vec4Transform             (_float4 *pout, CLCONST _float4 *pv, CLCONST _float4x4 *pm);

  //float3*     Vec3TransformNormal3x3(float3* pout, CLCONST float3* pv, CLCONST float3x3 *pm);
  //float QuaternionLengthSq(CLCONST _quaternion *pQ);


} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _FLOAT_X_IMPL_H_