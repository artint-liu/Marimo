#include "clstd.h"
//#include "cltypes.h"
#include "clColorSpace.h"
//#include "floatx.h"
#include "clString.H"
#include "clUtility.H"

namespace clstd
{
  STATIC_ASSERT(sizeof(COLOR_ARGB) == 4);

  static float4x4 s_matRGB2Hue(
    -0.81649661f, 0.00000000f, -0.57735026f, 0.00000000f,
    0.40824831f, -0.70710677f, -0.57735026f, 0.00000000f,
    0.40824831f, 0.70710677f, -0.57735026f, 0.00000000f,
    0.00000000f, 0.00000000f,  1.7320508f, 1.0000000f);

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
        0,1,0,0, 0,0,1,0,
        1,0,0,0, 0,0,0,1);

      float4x4 matXCHGAxisPost(
        0,-1,0,0, -1,0,0,0,
        0,0,1,0, 0,0,0,1);
      s_matRGB2Hue.LookAtLH(float3( 1, 1, 1), float3(-1,-1,-1), float3::AxisY);
      s_matRGB2Hue = matXCHGAxis * s_matRGB2Hue * matXCHGAxisPost;
    }

    void BuildHue2RGBMat()
    {
      float4x4 matXCHGAxis(
        0,1,0,0, 0,0,1,0,
        1,0,0,0, 0,0,0,1);

      float4x4 matXCHGAxisPost(
        0,-1,0,0, -1,0,0,0,
        0,0,1,0, 0,0,0,1);

      s_matHue2RGB.LookAtLH(float3( 1, 1, 1), float3(-1, -1, -1), float3::AxisY);
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

  void rgb2hsv(float r, float g, float b, float* h, float* s, float* v)
  {
    //const float fInv2PI = 1.0f / (D3DX_PI * 2.0f);

    float3 v3(r, g, b);
    float4 v4;
    Vec3TransformCoord(&v4, &v3, &s_matRGB2Hue);
    //D3DXVECTOR4 v4;
    //D3DXVec3Transform(&v4, &v3, &matMagic);

    //v3 *= s_matRGB2Hue;

    *h = (atan2(v4.y, v4.x) + CL_PI) * CL_INVERSE2PI;

    float fMax = clMax(v3.x, clMax(v3.y, v3.z));
    float fMin = clMin(v3.x, clMin(v3.y, v3.z));

    *v = fMax;

    if(fMax == 0)
      *s = 0;
    else
      *s = (fMax - fMin) / fMax;
  }

  void hsv2rgb(float h, float s, float v, float* r, float* g, float* b)
  {
    const float c_fMid = 1.73205081f * 0.5f;
    const float th = (h + 0.5f) * CL_2PI;
    float3 v3(cos(th), sin(th), c_fMid);
    float4 v4;
    Vec3TransformCoord(&v4, &v3, &s_matHue2RGB);
    const float c_fScale = s * v;
    const float c_fBase = v - c_fScale;
    *r = v4.x * c_fScale + c_fBase;
    *g = v4.y * c_fScale + c_fBase;
    *b = v4.z * c_fScale + c_fBase;
  }

  void rgb2yuv(float r, float g, float b, float* y, float* u, float* v)
  {
    *y = getgray(r, g, b);
    *u = -0.14713f * r - 0.28886f * g + 0.436f * b;
    *v = 0.615f * r - 0.51499f * g - 0.10001f * b;
  }

  void yuv2rgb(float y, float u, float v, float* r, float* g, float* b)
  {
    *r = y + 1.13983f * v;
    *g = y - 0.39465f * u - 0.58060f * v;
    *b = y + 2.03211f * u;
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

} // namespace clstd
