#ifndef _CLSTD_MATRIX_H_
#define _CLSTD_MATRIX_H_

#ifndef _CL_STD_CODE_
#error Must be include "clstd.h" first.
#endif // #ifndef _CL_STD_CODE_

namespace clstd
{
  struct _euler;
  struct _float4x4;

  struct _float3x3
  {
    static _float3x3 Identity;
    union
    {
      struct{
        float 
          _11, _12, _13,
          _21, _22, _23,
          _31, _32, _33;
      };
      struct{
        float dm[3][3];
      }; // means dual dimension
      float m[9];
    };
    _float3x3() CLTRIVIAL_DEFAULT;
    _float3x3(const _float4x4& m4x4);
    _float3x3(
      float m11, float m12, float m13,
      float m21, float m22, float m23,
      float m31, float m32, float m33)
    {
      _11 = m11; _12 = m12; _13 = m13;
      _21 = m21; _22 = m22; _23 = m23;
      _31 = m31; _32 = m32; _33 = m33;
    }

    friend _float3x3 operator*(CLCONST _float3x3& m1, CLCONST _float3x3& m2);
    _float3x3& transpose();
    _float3x3& set(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);

    _float3x3& RotationX(const float fAngle); // radian
    _float3x3& RotationY(const float fAngle); // radian
    _float3x3& RotationZ(const float fAngle); // radian


    // 欧拉角转换部分算法来自 Euler angle convertion algorithm is from Ogre
    // The matrix must be orthonormal.  The decomposition is yaw*pitch*roll
    // where yaw is rotation about the Up vector, pitch is rotation about the
    // Right axis, and roll is rotation about the Direction axis.
    b32   ToEulerAnglesXYZ    (_euler& eur) CLCONST;
    b32   ToEulerAnglesXZY    (_euler& eur) CLCONST;
    b32   ToEulerAnglesYXZ    (_euler& eur) CLCONST;
    b32   ToEulerAnglesYZX    (_euler& eur) CLCONST;
    b32   ToEulerAnglesZXY    (_euler& eur) CLCONST;
    b32   ToEulerAnglesZYX    (_euler& eur) CLCONST;

    void  FromEulerAnglesXYZ  (CLCONST _euler& eur);
    void  FromEulerAnglesXZY  (CLCONST _euler& eur);
    void  FromEulerAnglesYXZ  (CLCONST _euler& eur);
    void  FromEulerAnglesYZX  (CLCONST _euler& eur);
    void  FromEulerAnglesZXY  (CLCONST _euler& eur);
    void  FromEulerAnglesZYX  (CLCONST _euler& eur);

    _float3x3& FromQuaternion(CLCONST _quaternion *pq);
  };

  //////////////////////////////////////////////////////////////////////////
  struct _float4x4
  {
    static _float4x4 Identity;
    union
    {
      struct{
        float 
          _11, _12, _13, _14,
          _21, _22, _23, _24,
          _31, _32, _33, _34,
          _41, _42, _43, _44;
      };
      struct{
        float dm[4][4];
      }; // means dual dimension
      float m[16];
    };
    _float4x4() CLTRIVIAL_DEFAULT;
    _float4x4(float* a)
    {
      m[ 0] = a[ 0];  m[ 1] = a[ 1];  m[ 2] = a[ 2];  m[ 3] = a[ 3];
      m[ 4] = a[ 4];  m[ 5] = a[ 5];  m[ 6] = a[ 6];  m[ 7] = a[ 7];
      m[ 8] = a[ 8];  m[ 9] = a[ 9];  m[10] = a[10];  m[11] = a[11];
      m[12] = a[12];  m[13] = a[13];  m[14] = a[14];  m[15] = a[15];
    }
    _float4x4(float f)
    {
      _11 = _12 = _13 = _14 =
        _21 = _22 = _23 = _24 =
        _31 = _32 = _33 = _34 =
        _41 = _42 = _43 = _44 = f;
    }

    _float4x4(float m11, float m12, float m13, float m14,
      float m21, float m22, float m23, float m24,
      float m31, float m32, float m33, float m34,
      float m41, float m42, float m43, float m44)
    {
      _11 = m11; _12 = m12; _13 = m13; _14 = m14;
      _21 = m21; _22 = m22; _23 = m23; _24 = m24;
      _31 = m31; _32 = m32; _33 = m33; _34 = m34;
      _41 = m41; _42 = m42; _43 = m43; _44 = m44;
    }

    _float4x4(CLCONST _float3x3& m3);

    _float4x4& operator=(CLCONST _float3x3& m3);

    void identity()
    {
      _11 = _22 = _33 = _44 = 1.0f;
      _12 = _13 = _14 = _21 = _23 = _24 =
        _31 = _32 = _34 = _41 = _42 = _43 = 0.0f;
    }

    _float4x4& Scale(float x, float y, float z)
    {
      _11 = x;
      _22 = y;
      _33 = z;
      _44 = 1.0f;
      _12 = _13 = _14 = _21 = _23 = _24 =
        _31 = _32 = _34 = _41 = _42 = _43 = 0.0f;
      return *this;
    }

    _float4x4& Scale(const _float3& v)
    {
      return Scale(v.x, v.y, v.z);
    }

    _float4x4& Scale(float scale)
    {
      return Scale(scale, scale, scale);
    }

    _float4x4& Translate(const float3& v)
    {
      _11 = _22 = _33 = _44 = 1.0f;
      _12 = _13 = _14 = _21 = _23 = _24 =
        _31 = _32 = _34 = 0.0f;
      _41 = v.x;
      _42 = v.y;
      _43 = v.z;
      return *this;
    }

    _float4x4& Translate(float x, float y, float z)
    {
      _11 = _22 = _33 = _44 = 1.0f;
      _12 = _13 = _14 = _21 = _23 = _24 =
        _31 = _32 = _34 = 0.0f;
      _41 = x;
      _42 = y;
      _43 = z;
      return *this;
    }

    _float4x4& transpose();

    float inverse();  // 返回值是Determinant
    static _float4x4 inverse(const _float4x4& m, float* fDeterminant = NULL);

    friend _float4x4 operator*(const _float4x4& m1, const _float4x4& m2);
    friend bool operator!=(const _float4x4& m1, const _float4x4& m2);
    friend bool operator==(const _float4x4& m1, const _float4x4& m2);

    _float4x4& RotationX(const float fAngle);
    _float4x4& RotationY(const float fAngle);
    _float4x4& RotationZ(const float fAngle);
    //_float4x4& YawPitchRollA(const float fYaw, const float fPitch, const float fRoll);
    //_float4x4& YawPitchRollR(const float fYaw, const float fPitch, const float fRoll);
    _float4x4& RotationAxis(const _float3& vAxis, const float fAngle);

    static float Det3x3(
      float m11, float m12, float m13,
      float m21, float m22, float m23,
      float m31, float m32, float m33)
    {
      return 
        m11 * m22 * m33 + m21 * m32 * m13 + m31 * m12 * m23 -
        m13 * m22 * m31 - m23 * m32 * m11 - m33 * m12 * m21;
    }
    _float4x4& SetColumn(const int c, const _float4& vColume);
    _float4x4& SetRow(const int r, const _float4& vRow);
    _float4x4& SetRow(const int r, float x, float y, float z, float w);

    _float4     GetColumn(const int l) const;
    const _float4& GetRow(const int r) const;

    _float4x4& LookAtLH(CLCONST _float3& vEye, CLCONST _float3& vLookAt, CLCONST _float3& vUp);
    _float4x4& FromDirection(CLCONST _float3& vDir, CLCONST _float3& vUp);  // vDir 和 vUp 都是已经归一化的
    _float4x4& PerspectiveFovLH(float fovy, float fAspect, float fNear, float fFar);
    _float4x4& OrthoLH(float w, float h, float fNear, float fFar);
    _float4x4& RotationQuaternion(CLCONST _quaternion *pq);
    _float4x4& RotationYawPitchRollA(float yaw, float pitch, float roll); // 基于角度
    _float4x4& RotationYawPitchRollR(float yaw, float pitch, float roll); // 基于弧度 y(yaw)->x(pitch)->z(roll)

    _float4x4& AffineTransformation (CLCONST _float3* scaling, CLCONST _float3* rotationcenter, CLCONST _quaternion* rotation, CLCONST _float3* translation);
    void       DecomposeScaling     (_float3* pOutScale) CLCONST;
    void       DecomposeTranslation (_float3* pOutTranslation) CLCONST;
    b32        Decompose            (_float3* pOutScale, _quaternion* pOutRotation) CLCONST;
    b32        Decompose            (_float3* pOutScale, _quaternion* pOutRotation, _float3* pOutTranslation) CLCONST;
  };

  //////////////////////////////////////////////////////////////////////////
  _float4x4*  MatrixInverse             (_float4x4* pout, float* pDeterminant, const _float4x4* pin);
  _float4x4*  MatrixInverseGaussJordan  (_float4x4* pout, float* pDeterminant, const _float4x4* pin);
  b32         MatrixDecompose           (_float3* poutscale, _quaternion* poutrotation, _float3 *pouttranslation, CLCONST _float4x4 *pm);
  void        MatrixDecomposeScaling          (CLCONST _float4x4 *pm, _float3* poutscale);
  b32         MatrixDecomposeScalingRotation  (CLCONST _float4x4 *pm, _float3* poutscale, _quaternion* poutrotation);
  void        MatrixDecomposeTranslation      (CLCONST _float4x4 *pm, _float3 *pouttranslation);
  _float4x4*  MatrixLookAtLH            (_float4x4* pout, CLCONST _float3 *peye, CLCONST _float3 *pat, CLCONST _float3 *pup);
  _float4x4*  MatrixRotationX           (_float4x4* pout, float angle);
  _float4x4*  MatrixRotationY           (_float4x4* pout, float angle);
  _float4x4*  MatrixRotationZ           (_float4x4* pout, float angle);
  _float4x4*  MatrixMultiply            (_float4x4* pout, CLCONST _float4x4 *pm1, CLCONST _float4x4 *pm2);
  _float4x4*  MatrixRotationYawPitchRoll(_float4x4* pout, float yaw, float pitch, float roll);
  void        MatrixIdentity            (_float4x4* pout);
  _float4x4*  MatrixRotationAxis        (_float4x4* pout, CLCONST _float3 *pv, float angle);
  _float4x4*  MatrixPerspectiveFovLH    (_float4x4* pout, float fovy, float aspect, float zn, float zf);
  _float4x4*  MatrixOrthoLH             (_float4x4* pout, float w, float h, float zn, float zf);
  _float4x4*  MatrixRotationQuaternion  (_float4x4* pout, CLCONST _quaternion *pq);
  _float4x4*  MatrixAffineTransformation(_float4x4* pout, CLCONST _float3* scaling, CLCONST _float3* rotationcenter, CLCONST _quaternion* rotation, CLCONST _float3* translation);

} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // #ifndef _CLSTD_MATRIX_H_