#include "clstd.h"
//#include "cltypes.h"
#include "clColorSpace.h"
//#include "floatx.h"
#include "clString.h"
#include "clUtility.h"

#include <stdlib.h>

namespace clstd
{
  STATIC_ASSERT(sizeof(COLOR_ARGB) == 4);

  static float4x4 s_matRGB2Hue(
    -0.81649661f, 0.00000000f, -0.57735026f, 0.00000000f,
    0.40824831f, -0.70710677f, -0.57735026f, 0.00000000f,
    0.40824831f, 0.70710677f, -0.57735026f, 0.00000000f,
    0.00000000f, 0.00000000f, 1.7320508f, 1.0000000f);

  static float4x4 s_matHue2RGB(
    -0.81649655f, 0.40824828f, 0.40824828f, 0.00000000f,
    0.00000000f, -0.70710677f, 0.70710677f, -0.00000000f,
    -0.57735026f, -0.57735026f, -0.57735026f, 0.00000000f,
    1.0000000f, 1.0000000f, 1.0000000f, 1.0000000f);

  namespace Internal
  {
    void BuildRGB2HueMat()
    {
      float4x4 matXCHGAxis(
        0, 1, 0, 0, 0, 0, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 1);

      float4x4 matXCHGAxisPost(
        0, -1, 0, 0, -1, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1);
      s_matRGB2Hue.LookAtLH(float3(1, 1, 1), float3(-1, -1, -1), float3::AxisY);
      s_matRGB2Hue = matXCHGAxis * s_matRGB2Hue * matXCHGAxisPost;
    }

    void BuildHue2RGBMat()
    {
      float4x4 matXCHGAxis(
        0, 1, 0, 0, 0, 0, 1, 0,
        1, 0, 0, 0, 0, 0, 0, 1);

      float4x4 matXCHGAxisPost(
        0, -1, 0, 0, -1, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1);

      s_matHue2RGB.LookAtLH(float3(1, 1, 1), float3(-1, -1, -1), float3::AxisY);
      //D3DXMatrixLookAtLH(&matMagic, &D3DXVECTOR3(1,1,1), &D3DXVECTOR3(-1,-1,-1), &D3DXVECTOR3(0,1,0));
      //matMagic = matXCHGAxis * matMagic * matXCHGAxisPost;
      s_matHue2RGB = matXCHGAxis * s_matHue2RGB * matXCHGAxisPost;
      s_matHue2RGB.inverse();
      //D3DXMatrixInverse(&matMagic, &fDet, &matMagic);
    }

    void InitializeColorSpace()
    {
      BuildRGB2HueMat();
      BuildHue2RGBMat();
    }
  } // namespace Internal

  void GetHueMagicMat(_float4x4* pMagic)  // rgb to hue
  {
    *pMagic = s_matRGB2Hue;
  }

  void GetRGBMagicMat(_float4x4* pMagic)  // hue to rgb
  {
    *pMagic = s_matHue2RGB;
  }

  //////////////////////////////////////////////////////////////////////////

  void rgb2hsv(float r, float g, float b, float* h, float* s, float* v)
  {
    float3 v3(r, g, b);

    float x = v3.dot(-0.81649661f, 0.40824831f, 0.40824831f);
    float y = v3.dot(0.0f, -0.70710677f, 0.70710677f);

    *h = (atan2(y, x) + CL_PI) * CL_INVERSE2PI;

    float fMax = clMax(v3.x, clMax(v3.y, v3.z));
    float fMin = clMin(v3.x, clMin(v3.y, v3.z));

    *v = fMax;

    if (fMax == 0) {
      *s = 0;
    } else {
      *s = (fMax - fMin) / fMax;
    }
  }

  void hsv2rgb(float h, float s, float v, float* r, float* g, float* b)
  {
    const float th = (h + 0.5f) * CL_2PI;
    float3 v3(cos(th), sin(th), 0);

    float x = v3.y * 0.00000000f - v3.x * 0.81649655f;
    float y = v3.x * 0.40824828f - v3.y * 0.70710677f;
    float z = v3.x * 0.40824828f + v3.y * 0.70710677f;

    float _a = clMin(x, clMin(y, z));
    float _b = clMax(x, clMax(y, z)) - _a;

    s = s * v;
    const float scale = s / _b;
    const float base = v - s - _a * scale;

    *r = x * scale + base;
    *g = y * scale + base;
    *b = z * scale + base;
  }

  //////////////////////////////////////////////////////////////////////////

  void rgb2hsl(float r, float g, float b, float* h, float* s, float* l)
  {
    float3 v3(r, g, b);

    float x = v3.dot(-0.81649661f, 0.40824831f, 0.40824831f);
    float y = v3.dot(0.0f, -0.70710677f, 0.70710677f);

    *h = (atan2(y, x) + CL_PI) * CL_INVERSE2PI;

    float fMax = clMax(v3.x, clMax(v3.y, v3.z));
    float fMin = clMin(v3.x, clMin(v3.y, v3.z));
    float delta = fMax - fMin;

    *l = (fMin + fMax) * 0.5f;
    if(delta == 0) {
      *s = 0.0f;
    } else {
      *s = delta / (1 - fabs(2 * (*l) - 1));
      // L < 0.5  : delta / (2.0 * L)
      // L >= 0.5 : delta / (2.0 - 2.0 * L)
    }
  }

  void hsl2rgb(float h, float s, float l, float* r, float* g, float* b)
  {
    const float th = (h + 0.5f) * CL_2PI;
    float3 v3(cos(th), sin(th), 0);

    const float x = v3.y * 0.00000000f - v3.x * 0.81649655f;
    const float y = v3.x * 0.40824828f - v3.y * 0.70710677f;
    const float z = v3.x * 0.40824828f + v3.y * 0.70710677f;

    //if(l < 0.5) {
    //  k = l;
    //} else {
    //  k = 1 - l;
    //}

    //const float k = 0.5f - fabs(0.5f - l);
    //const float _a = l - s * k;
    //const float _b = 2.0f * s * k;

    //const float _min = clMin(x, clMin(y, z));
    //const float delta = _b / (clMax(x, clMax(y, z)) - _min);


    //*r = (x - _min) * delta + _a;
    //*g = (y - _min) * delta + _a;
    //*b = (z - _min) * delta + _a;

    const float k = 0.5f - fabs(0.5f - l);
    const float _a = l - s * k;
    const float _b = 2.0f * s * k;

    const float _min = clMin(x, clMin(y, z));
    const float delta = (clMax(x, clMax(y, z)) - _min);


    *r = (x - _min) / delta * _b + _a;
    *g = (y - _min) / delta * _b + _a;
    *b = (z - _min) / delta * _b + _a;
  }

  //////////////////////////////////////////////////////////////////////////

  void rgb2yuv(float r, float g, float b, float* y, float* u, float* v)
  {
    *y = getgray(r, g, b);
    *u = -0.14713f * r - 0.28886f * g + 0.436f * b;
    *v = 0.615f * r - 0.51499f * g - 0.10001f * b;
  }

  void yuv2rgb(float y, float u, float v, float* r, float* g, float* b)
  {
    //*r = y + 1.13983f * v;
    //*g = y - 0.39465f * u - 0.58060f * v;
    //*b = y + 2.03211f * u;
    *r = y + 1.402f * v;
    *g = y - 0.344f * u - 0.714f * v;
    *b = y + 1.772f * u;
  }

  float getgray(float r, float g, float b)
  {
    return r * 0.299f + g * 0.587f + b * 0.114f;
  }
  //////////////////////////////////////////////////////////////////////////
  //COLOR_ARGB::operator u32 () const
  //{
  //  return argb;
  //}

  //////////////////////////////////////////////////////////////////////////
  COLOR_HSVA_F::COLOR_HSVA_F()
    : h(0), s(0), v(0), a(1)
  {
  }

  COLOR_HSVA_F::COLOR_HSVA_F(const COLOR_RGBA_F& rgb)
  {
    rgb2hsv(rgb.r, rgb.g, rgb.b, &h, &s, &v);
    a = rgb.a;
  }

  COLOR_HSVA_F::COLOR_HSVA_F(const COLOR_ABGR_F& rgb)
  {
    rgb2hsv(rgb.r, rgb.g, rgb.b, &h, &s, &v);
    a = rgb.a;
  }

  COLOR_HSVA_F::COLOR_HSVA_F(u32 aarrggbb)
  {
    COLOR_RGBA_F rgb(aarrggbb);
    rgb2hsv(rgb.r, rgb.g, rgb.b, &h, &s, &v);
    a = rgb.a;
  }
  //////////////////////////////////////////////////////////////////////////
  //COLOR_RGBA_F::COLOR_RGBA_F()
  //{
  //  r = g = b = a = 1;
  //}
  //
  //COLOR_RGBA_F::COLOR_RGBA_F(u32 packedClr)
  //{
  //  const float fInverseFactor = 1.0f / 255.0f;
  //  a = (float)((packedClr >> 24) & 0xff) * fInverseFactor;
  //  r = (float)((packedClr >> 16) & 0xff) * fInverseFactor;
  //  g = (float)((packedClr >> 8) & 0xff) * fInverseFactor;
  //  b = (float)(packedClr & 0xff) * fInverseFactor;
  //}
  //
  //COLOR_RGBA_F::COLOR_RGBA_F(float r, float g, float b, float a)
  //{
  //  this->r = r;
  //  this->g = g;
  //  this->b = b;
  //  this->a = a;
  //}

  //COLOR_RGBA_F COLOR_RGBA_F::operator*(float value) const
  //{
  //  return COLOR_RGBA_F(r * value, g * value, b * value, a * value);
  //}

  //COLOR_RGBA_F COLOR_RGBA_F::operator/(float value) const
  //{
  //  const float fInverseValue = 1.0f / value;
  //  return COLOR_RGBA_F(r * fInverseValue, g * fInverseValue, b * fInverseValue, a * fInverseValue);
  //}


  //COLOR_RGBA_F COLOR_RGBA_F::operator-(const COLOR_RGBA_F& rgb) const
  //{
  //  return COLOR_RGBA_F(r - rgb.r, g - rgb.g, b - rgb.b, a - rgb.a);
  //}
  //
  //COLOR_RGBA_F::operator COLOR_ARGB () const
  //{
  //  COLOR_ARGB t;
  //  t.r = r < 0 ? 0 : (r > 1.0f ? 0xff : (u8)(r * 255.0f + 0.5f));
  //  t.g = g < 0 ? 0 : (g > 1.0f ? 0xff : (u8)(g * 255.0f + 0.5f));
  //  t.b = b < 0 ? 0 : (b > 1.0f ? 0xff : (u8)(b * 255.0f + 0.5f));
  //  t.a = a < 0 ? 0 : (a > 1.0f ? 0xff : (u8)(a * 255.0f + 0.5f));
  //  return t;
  //}

  //float COLOR_RGBA_F::GetGray() const
  //{
  //  return getgray(r, g, b);
  //}
  //////////////////////////////////////////////////////////////////////////
  //COLOR_ABGR_F::COLOR_ABGR_F(const COLOR_HSVA_F& hsv)
  //{
  //  hsv2rgb(hsv.h, hsv.s, hsv.v, &r, &g, &b);
  //  a = hsv.a;
  //}

  //COLOR_ABGR_F::operator COLOR_ARGB () const
  //{
  //  COLOR_ARGB t;
  //  t.r = r < 0 ? 0 : (r > 1.0f ? 0xff : (u8)(r * 255.0f + 0.5f));
  //  t.g = g < 0 ? 0 : (g > 1.0f ? 0xff : (u8)(g * 255.0f + 0.5f));
  //  t.b = b < 0 ? 0 : (b > 1.0f ? 0xff : (u8)(b * 255.0f + 0.5f));
  //  t.a = a < 0 ? 0 : (a > 1.0f ? 0xff : (u8)(a * 255.0f + 0.5f));
  //  return t;
  //}

  //float COLOR_ABGR_F::GetGray() const
  //{
  //  return getgray(r, g, b);
  //}
  //////////////////////////////////////////////////////////////////////////
  float COLOR_YUVA_F::GetGray() const
  {
    return y;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL<_TDataDef>& _COLOR_RGBTEMPL<_TDataDef>::set(const Internal::COLOR_ARGB& argb)
  {
    Internal::_Assign(*this, argb);
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL<_TDataDef>& _COLOR_RGBTEMPL<_TDataDef>::set(u8 a, u8 r, u8 g, u8 b)
  {
    this->a = a;
    this->r = r;
    this->g = g;
    this->b = b;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL<_TDataDef>& _COLOR_RGBTEMPL<_TDataDef>::set(u32 aarrggbb)
  {
    this->a = (aarrggbb >> 24) & 0xff;
    this->r = (aarrggbb >> 16) & 0xff;
    this->g = (aarrggbb >> 8 ) & 0xff;
    this->b = (aarrggbb      ) & 0xff;
    return *this;
  }

  template<class _TDataDef>
  u32 _COLOR_RGBTEMPL<_TDataDef>::RandomizeRGB() // 随机产生一个颜色
  {
    this->r = clrand() & 0xff;
    this->g = clrand() & 0xff;
    this->b = clrand() & 0xff;
    return this->data;
  }

  template<class _TDataDef>
  u32 _COLOR_RGBTEMPL<_TDataDef>::RandomizeAll() // 随机产生一个颜色和Alpha
  {
    RandomizeRGB();
    this->a = clrand() & 0xff;
    return this->data;
  }

  //////////////////////////////////////////////////////////////////////////

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(const Internal::COLOR_ARGB& value)
  {
    Internal::_UnpackColorValue(*this, value);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(const Internal::COLOR_RGBA_F& value)
  {
    Internal::_Assign(*this, value);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(const Internal::COLOR_ABGR_F& value)
  {
    Internal::_Assign(*this, value);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(const COLOR_HSVA_F& hsv)
  {
    hsv2rgb(hsv.h, hsv.s, hsv.v, &this->r, &this->g, &this->b);
    this->a = hsv.a;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(const COLOR_YUVA_F& yuv)
  {
    yuv2rgb(yuv.y, yuv.u, yuv.v, &this->r, &this->g, &this->b);
    this->a = yuv.a;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>::_COLOR_RGBTEMPL_F(float r, float g, float b, float a)
  {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator+(const _COLOR_RGBTEMPL_F& rgb) const
  {
    return _COLOR_RGBTEMPL_F(this->r + rgb.r, this->g + rgb.g, this->b + rgb.b, this->a + rgb.a);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator+(float v) const
  {
    return _COLOR_RGBTEMPL_F(this->r + v, this->g + v, this->b + v, this->a + v);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator-(const _COLOR_RGBTEMPL_F& rgb) const
  {
    return _COLOR_RGBTEMPL_F(this->r - rgb.r, this->g - rgb.g, this->b - rgb.b, this->a - rgb.a);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator-(float v) const
  {
    return _COLOR_RGBTEMPL_F(this->r - v, this->g - v, this->b - v, this->a - v);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator*(float v) const
  {
    return _COLOR_RGBTEMPL_F(this->r * v, this->g * v, this->b * v, this->a * v);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef> _COLOR_RGBTEMPL_F<_TDataDef>::operator/(float v) const
  {
    const float fInverseValue = 1.0f / v;
    return _COLOR_RGBTEMPL_F(this->r * fInverseValue, this->g * fInverseValue, this->b * fInverseValue, this->a * fInverseValue);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator+=(const _COLOR_RGBTEMPL_F& rgb)
  {
    this->r += rgb.r;
    this->g += rgb.g;
    this->b += rgb.b;
    this->a += rgb.a;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator+=(float v)
  {
    this->r += v;
    this->g += v;
    this->b += v;
    this->a += v;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator-=(const _COLOR_RGBTEMPL_F& rgb)
  {

    this->r -= rgb.r;
    this->g -= rgb.g;
    this->b -= rgb.b;
    this->a -= rgb.a;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator-=(float v)
  {
    this->r -= v;
    this->g -= v;
    this->b -= v;
    this->a -= v;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator*=(float v)
  {
    this->r *= v;
    this->g *= v;
    this->b *= v;
    this->a *= v;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator/=(float v)
  {
    const float fInverseValue = 1.0f / v;
    this->r *= fInverseValue;
    this->g *= fInverseValue;
    this->b *= fInverseValue;
    this->a *= fInverseValue;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::operator=(const float4& v4)
  {
      this->r = v4.rgba.r;
      this->g = v4.rgba.g;
      this->b = v4.rgba.b;
      this->a = v4.rgba.a;
    return *this;
  }

  template<class _TDataDef>
  float4& _COLOR_RGBTEMPL_F<_TDataDef>::ToFloat4(float4& v4) const
  {
    v4.set(this->r, this->g, this->b, this->a);
    return v4;
  }

  template<class _TDataDef>
  float4& _COLOR_RGBTEMPL_F<_TDataDef>::ToFloat4()
  {
    return *reinterpret_cast<float4*>(this);
  }

  template<class _TDataDef>
  b32 _COLOR_RGBTEMPL_F<_TDataDef>::operator==(const _COLOR_RGBTEMPL_F& rgb) const
  {
    return (this->r == rgb.r && this->g == rgb.g && this->b == rgb.b && this->a == rgb.a);
  }

  template<class _TDataDef>
  b32 _COLOR_RGBTEMPL_F<_TDataDef>::operator!=(const _COLOR_RGBTEMPL_F& rgb) const
  {
    return (this->r != rgb.r || this->g != rgb.g || this->b != rgb.b || this->a != rgb.a);
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::set(float r, float g, float b, float a)
  {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
    return *this;
  }

  template<class _TDataDef>
  _COLOR_RGBTEMPL_F<_TDataDef>& _COLOR_RGBTEMPL_F<_TDataDef>::set(u32 aarrggbb)
  {
    const float fInverseFactor = 1.0f / 255.0f;
    this->a = (float)((aarrggbb >> 24) & 0xff) * fInverseFactor;
    this->r = (float)((aarrggbb >> 16) & 0xff) * fInverseFactor;
    this->g = (float)((aarrggbb >> 8) & 0xff) * fInverseFactor;
    this->b = (float)((aarrggbb)& 0xff) * fInverseFactor;
    return *this;
  }

  template<class _TDataDef>
  u32 _COLOR_RGBTEMPL_F<_TDataDef>::ARGB() const
  {
    Internal::COLOR_ARGB ret;
    Internal::_PackColorValue(ret, *this);
    return ret.data;
  }

  template<class _TDataDef>
  u32 _COLOR_RGBTEMPL_F<_TDataDef>::ABGR() const
  {
    Internal::COLOR_ABGR ret;
    Internal::_PackColorValue(ret, *this);
    return ret.data;
  }

  template<class _TDataDef>
  float _COLOR_RGBTEMPL_F<_TDataDef>::GetGray() const
  {
    return getgray(this->r, this->g, this->b);
  }

  template struct _COLOR_RGBTEMPL_F<Internal::COLOR_RGBA_F>;
  template struct _COLOR_RGBTEMPL_F<Internal::COLOR_ABGR_F>;
  template struct _COLOR_RGBTEMPL<Internal::COLOR_ARGB>;
} // namespace clstd
