#include "clstd.h"
//#include "clUtility.h"
//#include "FloatX.h"

//using namespace clstd;
//float3* Vec3TransformCoord(float3 *pout, const float3 *pv, const _float4x4 *pm);
//_float4x4* MatrixLookAtLH(_float4x4 *pout, const float3 *peye, const float3 *pat, const float3 *pup);
//_float4x4* MatrixRotationX(_float4x4 *pout, float angle);
//_float4x4* MatrixRotationY(_float4x4 *pout, float angle);
//_float4x4* MatrixRotationZ(_float4x4 *pout, float angle);
//_float4x4* MatrixMultiply(_float4x4 *pout, const _float4x4 *pm1, const _float4x4 *pm2);
//_float4x4* MatrixRotationYawPitchRoll(_float4x4 *pout, float yaw, float pitch, float roll);
//void MatrixIdentity(_float4x4 *pout);
//float3* Vec3Normalize(float3 *pout, const float3 *pv);
//float3* Vec3Subtract(float3 *pOut, const float3* pV1, const float3* pV2);
//float3* Vec3Cross(float3 *pOut, const float3* pV1, const float3* pV2);
//float Vec3Dot(const float3* pV1, const float3* pV2);
//float Vec3Length(const float3 *pV);
//_float4x4* MatrixRotationAxis(_float4x4 *pout, const float3 *pv, float angle);
//_float4x4* MatrixPerspectiveFovLH(_float4x4 *pout, float fovy, float aspect, float zn, float zf);
namespace clstd
{

  _float2 _float2::AxisX(1.0f, 0.0f);
  _float2 _float2::AxisY(0.0f, 1.0f);
  _float2 _float2::Origin(0.0f, 0.0f);

  _float3 _float3::AxisX(1.0f, 0.0f, 0.0f);
  _float3 _float3::AxisY(0.0f, 1.0f, 0.0f);
  _float3 _float3::AxisZ(0.0f, 0.0f, 1.0f);
  _float3 _float3::Origin(0.0f, 0.0f, 0.0f);

  _float4 _float4::AxisX(1.0f, 0.0f, 0.0f, 1.0f);
  _float4 _float4::AxisY(0.0f, 1.0f, 0.0f, 1.0f);
  _float4 _float4::AxisZ(0.0f, 0.0f, 1.0f, 1.0f);
  _float4 _float4::Origin(0.0f, 0.0f, 0.0f, 1.0f);

  _float3::_float3(const _float4& v)
  {
    x = v.x / v.w; 
    y = v.y / v.w; 
    z = v.z / v.w;
  }

  float3& float3::operator=(const _float4 v)
  {
    x = v.x / v.w; 
    y = v.y / v.w; 
    z = v.z / v.w;
    return *this;
  }

  float3 operator*(const float3& v, const _float4x4& m)
  {
    _float4 t;
    Vec3TransformCoord(&t, &v, &m);
    const float fInverseW = 1.0f / t.w;
    return float3(t.x * fInverseW, t.y * fInverseW, t.z * fInverseW);
  }

  float4 float3::transform(const _float4x4& m) const
  {
    float4 t;
    return *Vec3TransformCoord(&t, this, &m);
  }

  float4 float4::transform(const _float4x4& m) const
  {
    float4 t;
    return *Vec4Transform(&t, this, &m);
  }
  //float3 operator * (const float3& v, const _float3x3& m)
  //{
  //  _float3 t;
  //  Vec3TransformNormal(&t, &v, &m);
  //  const float fInverseW = 1.0f / t.w;
  //  return float3(t.x * fInverseW, t.y * fInverseW, t.z * fInverseW);
  //}

  float3& float3::operator*=(const _float4x4& m)
  {
    _float4 t;
    Vec3TransformCoord(&t, (const float3*)this, (const _float4x4*)&m);
    const float fInverseW = 1.0f / t.w;
    x = t.x * fInverseW;
    y = t.y * fInverseW;
    z = t.z * fInverseW;
    return *this;
  }

  _float3 _float3::MulAsMatrix3x3(const _float4x4& m) const
  {
    float3 t;
    Vec3TransformNormal(&t, this, &m);
    return t;
  }

  void float3::Min(const float3& v)
  {
    x = x < v.x ? x : v.x;
    y = y < v.y ? y : v.y;
    z = z < v.z ? z : v.z;
  }

  void float3::Max(const float3& v)
  {
    x = x > v.x ? x : v.x;
    y = y > v.y ? y : v.y;
    z = z > v.z ? z : v.z;
  }


} // namespace clstd

//////////////////////////////////////////////////////////////////////////
namespace clstd
{
  _float4* Vec3TransformCoord(_float4 *pout, const float3 *pv, const _float4x4 *pm)
  {
    pout->x = pm->dm[0][0] * pv->x + pm->dm[1][0] * pv->y + pm->dm[2][0] * pv->z + pm->dm[3][0];
    pout->y = pm->dm[0][1] * pv->x + pm->dm[1][1] * pv->y + pm->dm[2][1] * pv->z + pm->dm[3][1];
    pout->z = pm->dm[0][2] * pv->x + pm->dm[1][2] * pv->y + pm->dm[2][2] * pv->z + pm->dm[3][2];
    pout->w = pm->dm[0][3] * pv->x + pm->dm[1][3] * pv->y + pm->dm[2][3] * pv->z + pm->dm[3][3];

    return pout;
  }

  float3* Vec3TransformNormal(float3* pout, const float3* pv, const float4x4 *pm)
  {
    const float3 v = *pv;
    pout->x = pm->dm[0][0] * v.x + pm->dm[1][0] * v.y + pm->dm[2][0] * v.z;
    pout->y = pm->dm[0][1] * v.x + pm->dm[1][1] * v.y + pm->dm[2][1] * v.z;
    pout->z = pm->dm[0][2] * v.x + pm->dm[1][2] * v.y + pm->dm[2][2] * v.z;
    return pout;
  }

  float4* Vec4Transform(float4 *pout, const float4 *pv, const float4x4 *pm)
  {
    float4 out;
    out.x = pm->dm[0][0] * pv->x + pm->dm[1][0] * pv->y + pm->dm[2][0] * pv->z + pm->dm[3][0] * pv->w;
    out.y = pm->dm[0][1] * pv->x + pm->dm[1][1] * pv->y + pm->dm[2][1] * pv->z + pm->dm[3][1] * pv->w;
    out.z = pm->dm[0][2] * pv->x + pm->dm[1][2] * pv->y + pm->dm[2][2] * pv->z + pm->dm[3][2] * pv->w;
    out.w = pm->dm[0][3] * pv->x + pm->dm[1][3] * pv->y + pm->dm[2][3] * pv->z + pm->dm[3][3] * pv->w;
    *pout = out;
    return pout;
  }


  //////////////////////////////////////////////////////////////////////////
  float3* Vec3Normalize(float3 *pout, const float3 *pv)
  {
    float3 out;
    float norm;

    norm = Vec3Length(pv);
    if ( !norm )
    {
      out.x = 0.0f;
      out.y = 0.0f;
      out.z = 0.0f;
    }
    else
    {
      out.x = pv->x / norm;
      out.y = pv->y / norm;
      out.z = pv->z / norm;
    }
    *pout = out;
    return pout;
  }
  float3* Vec3Subtract(float3* pOut, const float3* pV1, const float3* pV2)
  {
    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
  }
  float3* Vec3Cross(float3 *pOut, const float3* pV1, const float3* pV2)
  {
    float3 v;

    v.x = pV1->y * pV2->z - pV1->z * pV2->y;
    v.y = pV1->z * pV2->x - pV1->x * pV2->z;
    v.z = pV1->x * pV2->y - pV1->y * pV2->x;

    *pOut = v;
    return pOut;
  }
  float Vec3Dot(const float3* pV1, const float3* pV2)
  {
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
  }
  float Vec3Length(const float3 *pV)
  {
    return sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
  }
} // namespace clstd

// 这个顺序不对,要修改!!!
//public static Vector3 QuaternionToEuclidean(Quaternion rotation)
//{
//  Vector3 rotationaxes = new Vector3();
//  rotationaxes.X = (float)Math.asin(2 * (rotation.W * rotation.Y - rotation.Z * rotation.X));
//  float test = rotation.X * rotation.Y + rotation.Z * rotation.W;
//  if (test == .5f)
//  {
//    rotationaxes.Y = 2 * (float)Math.atan2(rotation.X, rotation.W);
//    rotationaxes.Z = 0;
//  }
//  else if (test == -.5f)
//  {
//    rotationaxes.Y = -2 * (float)Math.atan2(rotation.X, rotation.W);
//    rotationaxes.Z = 0;
//  }
//  else
//  {
//    rotationaxes.Y = (float)Math.Atan(2 * (rotation.W * rotation.Z + rotation.Y * rotation.Y) / (1 - 2 * (rotation.Y * rotation.Y + rotation.Z * rotation.Z)));
//    rotationaxes.Z = (float)Math.Atan(2 * (rotation.W * rotation.X + rotation.Y * rotation.Z) / (1 - 2 * (rotation.X * rotation.X + rotation.Y * rotation.Y)));
//  }
//  return rotationaxes;
//}