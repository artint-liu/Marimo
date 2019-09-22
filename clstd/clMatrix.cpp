#include "clstd.h"
#include "clString.h"
#include "clUtility.h"
#include <math.h>

#if defined(_WINDOWS) && defined(_CL_ARCH_X86) && ! defined(__clang__)
#define _X86_SSE_ENABLE
#endif // #ifdef _CL_ARCH_X86

namespace clstd
{
  _float4x4 _float4x4::Identity(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  _float3x3 _float3x3::Identity(1,0,0, 0,1,0, 0,0,1);

  //////////////////////////////////////////////////////////////////////////
  _float3x3::_float3x3(const _float4x4& m4x4) {
    _11 = m4x4._11;    _12 = m4x4._12;    _13 = m4x4._13;
    _21 = m4x4._21;    _22 = m4x4._22;    _23 = m4x4._23;
    _31 = m4x4._31;    _32 = m4x4._32;    _33 = m4x4._33;
  }

  _float3x3 operator*(const _float3x3& m1, const _float3x3& m2)
  {
    _float3x3 t;
    for (size_t iRow = 0; iRow < 3; iRow++)
    {
      for (size_t iCol = 0; iCol < 3; iCol++)
      {
        t.dm[iRow][iCol] =
          m1.dm[iRow][0] * m2.dm[0][iCol] +
          m1.dm[iRow][1] * m2.dm[1][iCol] +
          m1.dm[iRow][2] * m2.dm[2][iCol] ;
      }
    }
    return t;
  }

  b32 _float3x3::ToEulerAnglesXYZ(_euler& eur) const
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz          -cy*sz           sy
    //        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
    //       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

    eur.y = float(asin(dm[2][0]));
    if ( eur.y < float(CL_HALF_PI) )
    {
      if ( eur.y > float(-CL_HALF_PI) )
      {
        eur.x = atan2(-dm[2][1], dm[2][2]);
        eur.z = atan2(-dm[1][0], dm[0][0]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(dm[0][1], dm[1][1]);
        eur.z = float(0);  // any angle works
        eur.x = eur.z - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(dm[0][1], dm[1][1]);
      eur.z = float(0);  // any angle works
      eur.x = fRpY - eur.z;
      return false;
    }
  }

  b32 _float3x3::ToEulerAnglesXZY(_euler& eur) const
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz          -sz              cz*sy
    //        sx*sy+cx*cy*sz  cx*cz          -cy*sx+cx*sy*sz
    //       -cx*sy+cy*sx*sz  cz*sx           cx*cy+sx*sy*sz

    eur.z = asin(-dm[1][0]);
    if ( eur.z < float(CL_HALF_PI) )
    {
      if ( eur.z > float(-CL_HALF_PI) )
      {
        eur.x = atan2(dm[1][2], dm[1][1]);
        eur.y = atan2(dm[2][0], dm[0][0]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(-dm[0][2], dm[2][2]);
        eur.y = float(0);  // any angle works
        eur.x = eur.y - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(-dm[0][2], dm[2][2]);
      eur.y = float(0);  // any angle works
      eur.x = fRpY - eur.y;
      return false;
    }
  }

  b32 _float3x3::ToEulerAnglesYXZ(_euler& eur) const
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz+sx*sy*sz  cz*sx*sy-cy*sz  cx*sy
    //        cx*sz           cx*cz          -sx
    //       -cz*sy+cy*sx*sz  cy*cz*sx+sy*sz  cx*cy

    eur.x = asin(-dm[2][1]);
    if ( eur.x < float(CL_HALF_PI) )
    {
      if ( eur.x > float(-CL_HALF_PI) )
      {
        eur.y = atan2(dm[2][0], dm[2][2]);
        eur.z = atan2(dm[0][1], dm[1][1]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(-dm[1][0], dm[0][0]);
        eur.z = float(0);  // any angle works
        eur.y = eur.z - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(-dm[1][0], dm[0][0]);
      eur.z = float(0);  // any angle works
      eur.y = fRpY - eur.z;
      return false;
    }
  }

  b32 _float3x3::ToEulerAnglesYZX(_euler& eur) const
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz           sx*sy-cx*cy*sz  cx*sy+cy*sx*sz
    //        sz              cx*cz          -cz*sx
    //       -cz*sy           cy*sx+cx*sy*sz  cx*cy-sx*sy*sz

    eur.z = asin(dm[0][1]);
    if ( eur.z < float(CL_HALF_PI) )
    {
      if ( eur.z > float(-CL_HALF_PI) )
      {
        eur.y = atan2(-dm[0][2], dm[0][0]);
        eur.x = atan2(-dm[2][1], dm[1][1]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(dm[1][2], dm[2][2]);
        eur.x = float(0);  // any angle works
        eur.y = eur.x - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(dm[1][2], dm[2][2]);
      eur.x = float(0);  // any angle works
      eur.y = fRpY - eur.x;
      return false;
    }
  }

  b32 _float3x3::ToEulerAnglesZXY(_euler& eur) const // debug:transpose
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz-sx*sy*sz -cx*sz           cz*sy+cy*sx*sz
    //        cz*sx*sy+cy*sz  cx*cz          -cy*cz*sx+sy*sz
    //       -cx*sy           sx              cx*cy

    eur.x = asin(dm[1][2]);
    if ( eur.x < float(CL_HALF_PI) )
    {
      if ( eur.x > float(-CL_HALF_PI) )
      {
        eur.z = atan2(-dm[1][0], dm[1][1]);
        eur.y = atan2(-dm[0][2], dm[2][2]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(dm[2][0], dm[0][0]);
        eur.y = float(0);  // any angle works
        eur.z = eur.y - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(dm[2][0], dm[0][0]);
      eur.y = float(0);  // any angle works
      eur.z = fRpY - eur.y;
      return false;
    }
  }

  b32 _float3x3::ToEulerAnglesZYX(_euler& eur) const
  {
    // 这个函数来自Orge，计算公式需要转置才能对应float3x3的分量
    // rot =  cy*cz           cz*sx*sy-cx*sz  cx*cz*sy+sx*sz
    //        cy*sz           cx*cz+sx*sy*sz -cz*sx+cx*sy*sz
    //       -sy              cy*sx           cx*cy

    eur.y = asin(-dm[0][2]);
    if ( eur.y < float(CL_HALF_PI) )
    {
      if ( eur.y > float(-CL_HALF_PI) )
      {
        eur.z = atan2(dm[0][1], dm[0][0]);
        eur.x = atan2(dm[1][2], dm[2][2]);
        return true;
      }
      else
      {
        // WARNING.  Not a unique solution.
        float fRmY = atan2(-dm[1][0], dm[2][0]);
        eur.x = float(0);  // any angle works
        eur.z = eur.x - fRmY;
        return false;
      }
    }
    else
    {
      // WARNING.  Not a unique solution.
      float fRpY = atan2(-dm[1][0], dm[2][0]);
      eur.x = float(0);  // any angle works
      eur.z = fRpY - eur.x;
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////////

  void _float3x3::FromEulerAnglesXYZ(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = mz * my * mx;
  }

  void _float3x3::FromEulerAnglesXZY(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = my * mz * mx;
  }

  void _float3x3::FromEulerAnglesYXZ(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = mz * mx * my;
  }

  void _float3x3::FromEulerAnglesYZX(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = mx * mz * my;
  }

  void _float3x3::FromEulerAnglesZXY(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = my * mx * mz;
  }

  void _float3x3::FromEulerAnglesZYX(const _euler& eur)
  {
    float3x3 mx, my, mz;
    mx.RotationX(eur.x);
    my.RotationY(eur.y);
    mz.RotationZ(eur.z);

    *this = mx * my * mz;
  }

  _float3x3& _float3x3::FromQuaternion( const _quaternion *pq )
  {
    MatrixRotationQuaternion(this, pq);
    return *this;
  }

  _float3x3& _float3x3::transpose()
  {
    clSwap(_12, _21);
    clSwap(_13, _31);
    clSwap(_23, _32);
    return *this;
  }

  _float3x3& _float3x3::RotationX( const float fAngle )
  {
    const float fCos = cos(fAngle);
    const float fSin = sin(fAngle);
    set(1,0,0,0,fCos,fSin,0,-fSin,fCos);
    return *this;
  }

  _float3x3& _float3x3::RotationY( const float fAngle )
  {
    const float fCos = cos(fAngle);
    const float fSin = sin(fAngle);
    set(fCos,0,-fSin,0,1,0,fSin,0,fCos);
    return *this;
  }

  _float3x3& _float3x3::RotationZ( const float fAngle )
  {
    const float fCos = cos(fAngle);
    const float fSin = sin(fAngle);
    set(fCos,fSin,0,-fSin,fCos,0,0,0,1);
    return *this;
  }

  clstd::_float3x3& _float3x3::RotationAxis(const _float3& vAxis, const float fAngle)
  {
    float3 v;
    Vec3Normalize(&v, &vAxis);
    const float s = sin(fAngle);
    const float c = cos(fAngle);

    dm[0][0] = (1.0f - c) * v.x * v.x + c;
    dm[1][0] = (1.0f - c) * v.x * v.y - s * v.z;
    dm[2][0] = (1.0f - c) * v.x * v.z + s * v.y;
    dm[0][1] = (1.0f - c) * v.y * v.x + s * v.z;
    dm[1][1] = (1.0f - c) * v.y * v.y + c;
    dm[2][1] = (1.0f - c) * v.y * v.z - s * v.x;
    dm[0][2] = (1.0f - c) * v.z * v.x - s * v.y;
    dm[1][2] = (1.0f - c) * v.z * v.y + s * v.x;
    dm[2][2] = (1.0f - c) * v.z * v.z + c;
    return *this;
  }

  _float3x3& _float3x3::set(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
  {
    _11 = m11; _12 = m12; _13 = m13;
    _21 = m21; _22 = m22; _23 = m23;
    _31 = m31; _32 = m32; _33 = m33;
    return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  _float4x4::_float4x4(const _float3x3& m3)
  {
    _11 = m3._11; _12 = m3._12; _13 = m3._13; _14 = 0;
    _21 = m3._21; _22 = m3._22; _23 = m3._23; _24 = 0;
    _31 = m3._31; _32 = m3._32; _33 = m3._33; _34 = 0;
    _41 = 0; _42 = 0; _43 = 0; _44 = 1;
  }

  _float4x4& _float4x4::operator=(const _float3x3& m3)
  {
    _11 = m3._11; _12 = m3._12; _13 = m3._13;
    _21 = m3._21; _22 = m3._22; _23 = m3._23;
    _31 = m3._31; _32 = m3._32; _33 = m3._33;
    return *this;
  }

  _float4x4& _float4x4::RotationX(const float fAngle)
  { 
    MatrixRotationX((_float4x4*)this, fAngle); 
    return *this; 
  }

  _float4x4& _float4x4::RotationY(const float fAngle)
  { 
    MatrixRotationY((_float4x4*)this, fAngle); 
    return *this; 
  }

  _float4x4& _float4x4::RotationZ(const float fAngle)
  { 
    MatrixRotationZ((_float4x4*)this, fAngle); 
    return *this; 
  }

  _float4x4 operator*(const _float4x4& m1, const _float4x4& m2)
  {
#ifdef _X86_SSE_ENABLE
    DECL_ALIGN_16 float M[16];
    __asm
    {
      mov    ecx, m2
      mov    eax, m1
      lea    edx, M

      MOVSS    XMM0, [EAX]
      MOVSS    XMM1, [EAX + 4]
      MOVSS    XMM2, [EAX + 8]
      MOVSS    XMM3, [EAX + 12]

      MOVSLDUP XMM0, XMM0
      MOVSLDUP XMM1, XMM1
      MOVSLDUP XMM2, XMM2
      MOVSLDUP XMM3, XMM3

      MOVLHPS  XMM0, XMM0
      MOVLHPS  XMM1, XMM1
      MOVLHPS  XMM2, XMM2
      MOVLHPS  XMM3, XMM3

      MOVUPS   XMM4, [ECX]
      MOVUPS   XMM5, [ECX + 16]
      MOVUPS   XMM6, [ECX + 32]
      MOVUPS   XMM7, [ECX + 48]

      MULPS    XMM0, XMM4
      MULPS    XMM1, XMM5
      MULPS    XMM2, XMM6
      MULPS    XMM3, XMM7

      ADDPS    XMM0, XMM1
      ADDPS    XMM2, XMM3 // TODO: 还能消除类似的数据相关
      ADDPS    XMM0, XMM2
      movaps   [edx], XMM0

      //////////////////////////////////////////////////////////////////////////
      MOVSS    XMM1, [EAX+16]
      MOVSS    XMM2, [EAX+20]
      MOVSS    XMM3, [EAX+24]
      MOVSS    XMM0, [EAX+28]

      MOVSLDUP XMM1, XMM1
      MOVSLDUP XMM2, XMM2
      MOVSLDUP XMM3, XMM3
      MOVSLDUP XMM0, XMM0

      MOVLHPS  XMM1, XMM1
      MOVLHPS  XMM2, XMM2
      MOVLHPS  XMM3, XMM3
      MOVLHPS  XMM0, XMM0

      MULPS    XMM1, XMM4
      MULPS    XMM2, XMM5
      MULPS    XMM3, XMM6
      MULPS    XMM0, XMM7

      ADDPS    XMM1, XMM2
      ADDPS    XMM3, XMM0
      ADDPS    XMM1, XMM3
      movaps   [edx+16], XMM1

      //////////////////////////////////////////////////////////////////////////
      MOVSS    XMM0, [EAX+32]
      MOVSS    XMM1, [EAX+36]
      MOVSS    XMM2, [EAX+40]
      MOVSS    XMM3, [EAX+44]

      MOVSLDUP XMM0, XMM0
      MOVSLDUP XMM1, XMM1
      MOVSLDUP XMM2, XMM2
      MOVSLDUP XMM3, XMM3

      MOVLHPS  XMM0, XMM0
      MOVLHPS  XMM1, XMM1
      MOVLHPS  XMM2, XMM2
      MOVLHPS  XMM3, XMM3

      MULPS    XMM0, XMM4
      MULPS    XMM1, XMM5
      MULPS    XMM2, XMM6
      MULPS    XMM3, XMM7

      ADDPS    XMM0, XMM1
      ADDPS    XMM2, XMM3
      ADDPS    XMM0, XMM2
      movaps   [edx+32], XMM0
      //////////////////////////////////////////////////////////////////////////
      MOVSS    XMM1, [EAX+48]
      MOVSS    XMM2, [EAX+52]
      MOVSS    XMM3, [EAX+56]
      MOVSS    XMM0, [EAX+60]

      MOVSLDUP XMM1, XMM1
      MOVSLDUP XMM2, XMM2
      MOVSLDUP XMM3, XMM3
      MOVSLDUP XMM0, XMM0

      MOVLHPS  XMM1, XMM1
      MOVLHPS  XMM2, XMM2
      MOVLHPS  XMM3, XMM3
      MOVLHPS  XMM0, XMM0

      MULPS    XMM1, XMM4
      MULPS    XMM2, XMM5
      MULPS    XMM3, XMM6
      MULPS    XMM0, XMM7

      ADDPS    XMM1, XMM2
      ADDPS    XMM3, XMM0
      ADDPS    XMM1, XMM3
      movaps   [edx + 48], XMM1
    }
    return float4x4(M);
#else
    _float4x4 t;
    MatrixMultiply((_float4x4*)&t, (const _float4x4*)&m1, (const _float4x4*)&m2);
    return t;
#endif // _X86_SSE_ENABLE
  }

  bool operator!=(const _float4x4& m1, const _float4x4& m2)
  {
    for(int i = 0; i < 16; i++) {
      if(m1.m[i] != m2.m[i])
        return true;
    }
    return false;
  }

  bool operator==(const _float4x4& m1, const _float4x4& m2)
  {
    for(int i = 0; i < 16; i++) {
      if(m1.m[i] != m2.m[i])
        return false;
    }
    return true;
  }

  //_float4x4& _float4x4::YawPitchRollA(const float fYaw, const float fPitch, const float fRoll)
  //{
  //  MatrixRotationYawPitchRoll((_float4x4*)this, CL_AGNLE2RAD(fYaw), CL_AGNLE2RAD(fPitch), CL_AGNLE2RAD(fRoll));
  //  return *this;
  //}
  //
  //_float4x4& _float4x4::YawPitchRollR(const float fYaw, const float fPitch, const float fRoll)
  //{
  //  MatrixRotationYawPitchRoll((_float4x4*)this, fYaw, fPitch, fRoll);
  //  return *this;
  //}

  _float4x4& _float4x4::RotationAxis(const float3& vAxis, const float fAngle)
  {
    MatrixRotationAxis((_float4x4*)this, (float3*)&vAxis, fAngle);
    return *this;
  }

  _float4x4& _float4x4::SetColumn(const int c, const _float4& vColumn)
  {
    if(c == 0)  {
      _11 = vColumn.x;    _21 = vColumn.y;    _31 = vColumn.z;    _41 = vColumn.w;
    }
    else if(c == 1)  {
      _12 = vColumn.x;    _22 = vColumn.y;    _32 = vColumn.z;    _42 = vColumn.w;
    }
    else if(c == 2)  {
      _13 = vColumn.x;    _23 = vColumn.y;    _33 = vColumn.z;    _43 = vColumn.w;
    }
    else if(c == 3)  {
      _14 = vColumn.x;    _24 = vColumn.y;    _34 = vColumn.z;    _44 = vColumn.w;
    }
    return *this;
  }

  _float4x4& _float4x4::SetRow(const int r, const _float4& vRow)
  {
    ((float4*)this)[r] = vRow;
    return *this;
  }

  _float4x4& _float4x4::SetRow(const int r, float x, float y, float z, float w)
  {
    ((float4*)this)[r].set(x, y, z, w);
    return *this;
  }

  _float4 _float4x4::GetColumn(const int c) const
  {
    _float4 column;

    if(c == 0)  {
      column.x = _11;   column.y = _21;   column.z = _31;   column.w = _41;
    }
    else if(c == 1)  {
      column.x = _12;   column.y = _22;   column.z = _32;   column.w = _42;
    }
    else if(c == 2)  {
      column.x = _13;   column.y = _23;   column.z = _33;   column.w = _43;
    }
    else if(c == 3)  {
      column.x = _14;   column.y = _24;   column.z = _34;   column.w = _44;
    }
    return column;
  }

  const _float4& _float4x4::GetRow(const int r) const
  {
    return ((float4*)this)[r];
  }

  _float4x4& _float4x4::LookAtLH(const float3& vEye, const float3& vLookAt, const float3& vUp)
  {
    MatrixLookAtLH((_float4x4*)this, (const float3*)&vEye, (const float3*)&vLookAt, (const float3*)&vUp);
    return *this;
  }

  _float4x4& _float4x4::PerspectiveFovLH(float fovy, float fAspect, float fNear, float fFar)
  {
    MatrixPerspectiveFovLH((_float4x4*)this, fovy, fAspect, fNear, fFar);
    return *this;
  }

  _float4x4& _float4x4::OrthoLH(float w, float h, float fNear, float fFar)
  {
    return *MatrixOrthoLH(this, w, h, fNear, fFar);
  }

  _float4x4& _float4x4::RotationQuaternion(const _quaternion *pq)
  {
    return *MatrixRotationQuaternion(this, pq);
  }

  _float4x4& _float4x4::RotationYawPitchRollA(float yaw, float pitch, float roll)
  {
    // TODO: 换用MatrixRotationYawPitchRoll
    _quaternion q;
    q.YawPitchRollA(yaw, pitch, roll);
    return *MatrixRotationQuaternion(this, &q);
  }

  _float4x4& _float4x4::RotationYawPitchRollR(float yaw, float pitch, float roll)
  {
    // TODO: 换用MatrixRotationYawPitchRoll
    _quaternion q;
    q.YawPitchRollR(yaw, pitch, roll);
    return *MatrixRotationQuaternion(this, &q);
  }

  _float4x4& _float4x4::AffineTransformation(const float3* scaling, const float3* rotationcenter, const _quaternion* rotation, const float3* translation)
  {
    return *MatrixAffineTransformation(this, scaling, rotationcenter, rotation, translation);
  }
  
  void _float4x4::DecomposeScaling(_float3* pOutScale) const
  {
    MatrixDecomposeScaling(this, pOutScale);
  }

  void _float4x4::DecomposeTranslation(_float3* pOutTranslation) const
  {
    MatrixDecomposeTranslation(this, pOutTranslation);
  }

  b32 _float4x4::Decompose(_float3* pOutScale, _quaternion* pOutRotation) const
  {
    return MatrixDecomposeScalingRotation(this, pOutScale, pOutRotation);
  }

  b32 _float4x4::Decompose(_float3* pOutScale, _quaternion* pOutRotation, _float3* pOutTranslation) const
  {
    return MatrixDecompose(pOutScale, pOutRotation, pOutTranslation, this);
  }

  _float4x4& _float4x4::transpose()
  {
#ifdef _X86_SSE_ENABLE
    __asm 
    {
      mov eax, this
      movups xmm0, [eax]        // 0 1 2 3    0 4 8 C
      movups xmm1, [eax + 16]   // 4 5 6 7 =\ 1 5 9 D
      movups xmm2, [eax + 32]   // 8 9 A B =/ 2 6 A E
      movups xmm3, [eax + 48]   // C D E F    3 7 B F

      movaps xmm4, xmm0         // 0 1 2 3 
      movaps xmm5, xmm1         // 4 5 6 7 
      movaps xmm6, xmm2         // 8 9 A B 
      movaps xmm7, xmm3         // C D E F 

      punpckldq xmm0, xmm1      // 0 4 1 5
      punpckldq xmm2, xmm3      // 8 C 9 D
      punpckhdq xmm4, xmm5      // 2 6 3 7
      punpckhdq xmm6, xmm7      // A E B F

      movaps xmm1, xmm0         // 0 4 1 5
      movaps xmm5, xmm4         // 2 6 3 7

      punpcklqdq xmm0, xmm2     // 0 4 8 C
      punpckhqdq xmm1, xmm2     // 1 5 9 D
      punpcklqdq xmm4, xmm6     // 2 6 A E
      punpckhqdq xmm5, xmm6     // 3 7 B F

      movups [eax     ], xmm0
      movups [eax + 16], xmm1
      movups [eax + 32], xmm4
      movups [eax + 48], xmm5
    }
#else
    // TODO: 使用更高效率的交换方法
    _float4x4 t = *this;
    for(int i = 0; i < 4; i++)
      for(int j = 0; j < 4; j++)
        dm[i][j] = t.dm[j][i];
#endif // #ifdef _X86_SSE_ENABLE
    return *this;
  }

  float _float4x4::inverse()
  {
    /*
    _float4x4 t = *this;
    float fDet = 1.0f / Det3x3(t._11, t._12, t._13, t._21, t._22, t._23, t._31, t._32, t._33);

    _11 =  fDet * Det3x3(t._22, t._23, t._24, t._32, t._33, t._34, t._42, t._43, t._44);
    _12 = -fDet * Det3x3(t._12, t._13, t._14, t._32, t._33, t._34, t._42, t._43, t._44);
    _13 =  fDet * Det3x3(t._12, t._13, t._14, t._22, t._23, t._24, t._42, t._43, t._44);
    _14 = -fDet * Det3x3(t._12, t._13, t._14, t._22, t._23, t._24, t._32, t._33, t._34);

    _21 = -fDet * Det3x3(t._21, t._23, t._24, t._31, t._33, t._34, t._41, t._43, t._44);
    _22 =  fDet * Det3x3(t._11, t._13, t._14, t._31, t._33, t._34, t._41, t._43, t._44);
    _23 = -fDet * Det3x3(t._11, t._13, t._14, t._21, t._23, t._24, t._41, t._43, t._44);
    _24 =  fDet * Det3x3(t._11, t._13, t._14, t._21, t._23, t._24, t._31, t._33, t._34);

    _31 =  fDet * Det3x3(t._21, t._22, t._24, t._31, t._32, t._34, t._41, t._42, t._44);
    _32 = -fDet * Det3x3(t._11, t._12, t._14, t._31, t._32, t._34, t._41, t._42, t._44);
    _33 =  fDet * Det3x3(t._11, t._12, t._14, t._21, t._22, t._24, t._41, t._42, t._44);
    _34 = -fDet * Det3x3(t._11, t._12, t._14, t._21, t._22, t._24, t._31, t._32, t._34);

    _41 = -fDet * Det3x3(t._21, t._22, t._23, t._31, t._32, t._33, t._41, t._42, t._43);
    _42 =  fDet * Det3x3(t._11, t._12, t._13, t._31, t._32, t._33, t._41, t._42, t._43);
    _43 = -fDet * Det3x3(t._11, t._12, t._13, t._21, t._22, t._23, t._41, t._42, t._43);
    _44 =  fDet * Det3x3(t._11, t._12, t._13, t._21, t._22, t._23, t._31, t._32, t._33);
    return fDet;
    /*/
#define MUL3(a, b, c) (m[a] * m[b] * m[c])
    float inv[16];

    inv[ 0] =  MUL3(5, 10, 15) - MUL3(5, 11, 14) - MUL3(9, 6, 15) + MUL3(9, 7, 14) + MUL3(13, 6, 11) - MUL3(13, 7, 10); 
    inv[ 4] = -MUL3(4, 10, 15) + MUL3(4, 11, 14) + MUL3(8, 6, 15) - MUL3(8, 7, 14) - MUL3(12, 6, 11) + MUL3(12, 7, 10); 
    inv[ 8] =  MUL3(4,  9, 15) - MUL3(4, 11, 13) - MUL3(8, 5, 15) + MUL3(8, 7, 13) + MUL3(12, 5, 11) - MUL3(12, 7,  9); 
    inv[12] = -MUL3(4,  9, 14) + MUL3(4, 10, 13) + MUL3(8, 5, 14) - MUL3(8, 6, 13) - MUL3(12, 5, 10) + MUL3(12, 6,  9);

    inv[ 1] = -MUL3(1, 10, 15) + MUL3(1, 11, 14) + MUL3(9, 2, 15) - MUL3(9, 3, 14) - MUL3(13, 2, 11) + MUL3(13, 3, 10);
    inv[ 5] =  MUL3(0, 10, 15) - MUL3(0, 11, 14) - MUL3(8, 2, 15) + MUL3(8, 3, 14) + MUL3(12, 2, 11) - MUL3(12, 3, 10);
    inv[ 9] = -MUL3(0,  9, 15) + MUL3(0, 11, 13) + MUL3(8, 1, 15) - MUL3(8, 3, 13) - MUL3(12, 1, 11) + MUL3(12, 3,  9); 
    inv[13] =  MUL3(0,  9, 14) - MUL3(0, 10, 13) - MUL3(8, 1, 14) + MUL3(8, 2, 13) + MUL3(12, 1, 10) - MUL3(12, 2,  9);
    
    inv[ 2] =  MUL3(1,  6, 15) - MUL3(1,  7, 14) - MUL3(5, 2, 15) + MUL3(5, 3, 14) + MUL3(13, 2,  7) - MUL3(13, 3,  6); 
    inv[ 6] = -MUL3(0,  6, 15) + MUL3(0,  7, 14) + MUL3(4, 2, 15) - MUL3(4, 3, 14) - MUL3(12, 2,  7) + MUL3(12, 3,  6);
    inv[10] =  MUL3(0,  5, 15) - MUL3(0,  7, 13) - MUL3(4, 1, 15) + MUL3(4, 3, 13) + MUL3(12, 1,  7) - MUL3(12, 3,  5);
    inv[14] = -MUL3(0,  5, 14) + MUL3(0,  6, 13) + MUL3(4, 1, 14) - MUL3(4, 2, 13) - MUL3(12, 1,  6) + MUL3(12, 2,  5); 
    
    inv[ 3] = -MUL3(1,  6, 11) + MUL3(1,  7, 10) + MUL3(5, 2, 11) - MUL3(5, 3, 10) - MUL3( 9, 2,  7) + MUL3( 9, 3,  6);
    inv[ 7] =  MUL3(0,  6, 11) - MUL3(0,  7, 10) - MUL3(4, 2, 11) + MUL3(4, 3, 10) + MUL3( 8, 2,  7) - MUL3( 8, 3,  6);
    inv[11] = -MUL3(0,  5, 11) + MUL3(0,  7,  9) + MUL3(4, 1, 11) - MUL3(4, 3,  9) - MUL3( 8, 1,  7) + MUL3( 8, 3,  5);
    inv[15] =  MUL3(0,  5, 10) - MUL3(0,  6,  9) - MUL3(4, 1, 10) + MUL3(4, 2,  9) + MUL3( 8, 1,  6) - MUL3( 8, 2,  5);
#undef MUL3
    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if(det == 0) {
      det = 0;
    }
    else {
      det = 1.0f / det;
    }
    m[ 0] = inv[ 0] * det;  m[ 1] = inv[ 1] * det;  m[ 2] = inv[ 2] * det;  m[ 3] = inv[ 3] * det;
    m[ 4] = inv[ 4] * det;  m[ 5] = inv[ 5] * det;  m[ 6] = inv[ 6] * det;  m[ 7] = inv[ 7] * det;
    m[ 8] = inv[ 8] * det;  m[ 9] = inv[ 9] * det;  m[10] = inv[10] * det;  m[11] = inv[11] * det;
    m[12] = inv[12] * det;  m[13] = inv[13] * det;  m[14] = inv[14] * det;  m[15] = inv[15] * det;
    return det;
    //*/
  }

  _float4x4 _float4x4::inverse(const _float4x4& m, float* fDeterminant)
  {
    //*
    _float4x4 t = m;
    t.inverse();
    return t;
    /*/
    _float4x4 t;
    return *MatrixInverseGaussJordan(&t, fDeterminant, &m);
    //*/
  }

  _float4x4& _float4x4::FromDirection( const _float3& vDir, const _float3& vUp )
  {
    float3 right, rightn, top, topn;

    Vec3Cross(&right, &vUp, &vDir);
    Vec3Cross(&top, &vDir, &right);
    Vec3Normalize(&rightn, &right);
    Vec3Normalize(&topn, &top);

    dm[0][0] = rightn.x; //  0
    dm[1][0] = rightn.y; //  4
    dm[2][0] = rightn.z; //  8
    dm[3][0] = 0.0f;     // 12

    dm[0][1] = topn.x;   //  1
    dm[1][1] = topn.y;   //  5
    dm[2][1] = topn.z;   //  9
    dm[3][1] = 0.0f;     // 13

    dm[0][2] = vDir.x;   //  2
    dm[1][2] = vDir.y;   //  6
    dm[2][2] = vDir.z;   // 10
    dm[3][2] = 0.0f;     // 14

    dm[0][3] = 0.0f;     //  3
    dm[1][3] = 0.0f;     //  7
    dm[2][3] = 0.0f;     // 11
    dm[3][3] = 1.0f;     // 15
#define MUL3(a, b, c) (m[a] * m[b] * m[c])
#define MUL2(a, b) (m[a] * m[b])

    // 求逆的简化版，直接求不会啊
    float inv[16];
    inv[ 0] =  MUL2(5, 10) - MUL2(9, 6);
    inv[ 4] = -MUL2(4, 10) + MUL2(8, 6);
    inv[ 8] =  MUL2(4,  9) - MUL2(8, 5);
    inv[12] =  0.0f;

    inv[ 1] = -MUL2(1, 10) + MUL2(9, 2);
    inv[ 5] =  MUL2(0, 10) - MUL2(8, 2);
    inv[ 9] = -MUL2(0,  9) + MUL2(8, 1);
    inv[13] =  0.0f;

    inv[ 2] =  MUL2(1,  6) - MUL2(5, 2);
    inv[ 6] = -MUL2(0,  6) + MUL2(4, 2);
    inv[10] =  MUL2(0,  5) - MUL2(4, 1);
    inv[14] =  0.0f;

    inv[ 3] = 0.0f;
    inv[ 7] = 0.0f;
    inv[11] = 0.0f;
    inv[15] = MUL3(0,  5, 10) - MUL3(0,  6,  9) - MUL3(4, 1, 10) + MUL3(4, 2,  9) + MUL3( 8, 1,  6) - MUL3( 8, 2,  5);
#undef MUL3
#undef MUL2

    for(int i = 0; i < 16; i++)
    {
      m[i] = inv[i];
    }

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  _float4x4* MatrixLookAtLH(_float4x4 *pout, const float3 *peye, const float3 *pat, const float3 *pup)
  {
    float3 right, rightn, up, upn, vec, vec2;

    Vec3Subtract(&vec2, pat, peye);
    Vec3Normalize(&vec, &vec2);
    Vec3Cross(&right, pup, &vec);
    Vec3Cross(&up, &vec, &right);
    Vec3Normalize(&rightn, &right);
    Vec3Normalize(&upn, &up);
    pout->dm[0][0] = rightn.x;
    pout->dm[1][0] = rightn.y;
    pout->dm[2][0] = rightn.z;
    pout->dm[3][0] = -Vec3Dot(&rightn,peye);
    pout->dm[0][1] = upn.x;
    pout->dm[1][1] = upn.y;
    pout->dm[2][1] = upn.z;
    pout->dm[3][1] = -Vec3Dot(&upn, peye);
    pout->dm[0][2] = vec.x;
    pout->dm[1][2] = vec.y;
    pout->dm[2][2] = vec.z;
    pout->dm[3][2] = -Vec3Dot(&vec, peye);
    pout->dm[0][3] = 0.0f;
    pout->dm[1][3] = 0.0f;
    pout->dm[2][3] = 0.0f;
    pout->dm[3][3] = 1.0f;
    return pout;
  }

  _float4x4* MatrixRotationX(_float4x4 *pout, float angle)
  {
    MatrixIdentity(pout);
    pout->dm[1][1] = cos(angle);
    pout->dm[2][2] = cos(angle);
    pout->dm[1][2] = sin(angle);
    pout->dm[2][1] = -sin(angle);
    return pout;
  }

  _float4x4* MatrixRotationY(_float4x4 *pout, float angle)
  {
    MatrixIdentity(pout);
    pout->dm[0][0] = cos(angle);
    pout->dm[2][2] = cos(angle);
    pout->dm[0][2] = -sin(angle);
    pout->dm[2][0] = sin(angle);
    return pout;
  }

  _float4x4* MatrixRotationZ(_float4x4 *pout, float angle)
  {
    MatrixIdentity(pout);
    pout->dm[0][0] = cos(angle);
    pout->dm[1][1] = cos(angle);
    pout->dm[0][1] = sin(angle);
    pout->dm[1][0] = -sin(angle);
    return pout;
  }

  _float4x4* MatrixMultiply(_float4x4 *pout, const _float4x4 *pm1, const _float4x4 *pm2)
  {
    _float4x4 out;
    /*
    int i,j;

    for (i=0; i<4; i++)
    {
      for (j=0; j<4; j++)
      {
        out.dm[i][j] = 
          pm1->dm[i][0] * pm2->dm[0][j] + 
          pm1->dm[i][1] * pm2->dm[1][j] + 
          pm1->dm[i][2] * pm2->dm[2][j] + 
          pm1->dm[i][3] * pm2->dm[3][j];
      }
    }

    *pout = out;
    /*/
    out.dm[0][0] = pm1->dm[0][0] * pm2->dm[0][0] + pm1->dm[0][1] * pm2->dm[1][0] + pm1->dm[0][2] * pm2->dm[2][0] + pm1->dm[0][3] * pm2->dm[3][0];
    out.dm[0][1] = pm1->dm[0][0] * pm2->dm[0][1] + pm1->dm[0][1] * pm2->dm[1][1] + pm1->dm[0][2] * pm2->dm[2][1] + pm1->dm[0][3] * pm2->dm[3][1];
    out.dm[0][2] = pm1->dm[0][0] * pm2->dm[0][2] + pm1->dm[0][1] * pm2->dm[1][2] + pm1->dm[0][2] * pm2->dm[2][2] + pm1->dm[0][3] * pm2->dm[3][2];
    out.dm[0][3] = pm1->dm[0][0] * pm2->dm[0][3] + pm1->dm[0][1] * pm2->dm[1][3] + pm1->dm[0][2] * pm2->dm[2][3] + pm1->dm[0][3] * pm2->dm[3][3];
    
    out.dm[1][0] = pm1->dm[1][0] * pm2->dm[0][0] + pm1->dm[1][1] * pm2->dm[1][0] + pm1->dm[1][2] * pm2->dm[2][0] + pm1->dm[1][3] * pm2->dm[3][0];
    out.dm[1][1] = pm1->dm[1][0] * pm2->dm[0][1] + pm1->dm[1][1] * pm2->dm[1][1] + pm1->dm[1][2] * pm2->dm[2][1] + pm1->dm[1][3] * pm2->dm[3][1];
    out.dm[1][2] = pm1->dm[1][0] * pm2->dm[0][2] + pm1->dm[1][1] * pm2->dm[1][2] + pm1->dm[1][2] * pm2->dm[2][2] + pm1->dm[1][3] * pm2->dm[3][2];
    out.dm[1][3] = pm1->dm[1][0] * pm2->dm[0][3] + pm1->dm[1][1] * pm2->dm[1][3] + pm1->dm[1][2] * pm2->dm[2][3] + pm1->dm[1][3] * pm2->dm[3][3];
    
    out.dm[2][0] = pm1->dm[2][0] * pm2->dm[0][0] + pm1->dm[2][1] * pm2->dm[1][0] + pm1->dm[2][2] * pm2->dm[2][0] + pm1->dm[2][3] * pm2->dm[3][0];
    out.dm[2][1] = pm1->dm[2][0] * pm2->dm[0][1] + pm1->dm[2][1] * pm2->dm[1][1] + pm1->dm[2][2] * pm2->dm[2][1] + pm1->dm[2][3] * pm2->dm[3][1];
    out.dm[2][2] = pm1->dm[2][0] * pm2->dm[0][2] + pm1->dm[2][1] * pm2->dm[1][2] + pm1->dm[2][2] * pm2->dm[2][2] + pm1->dm[2][3] * pm2->dm[3][2];
    out.dm[2][3] = pm1->dm[2][0] * pm2->dm[0][3] + pm1->dm[2][1] * pm2->dm[1][3] + pm1->dm[2][2] * pm2->dm[2][3] + pm1->dm[2][3] * pm2->dm[3][3];
    
    out.dm[3][0] = pm1->dm[3][0] * pm2->dm[0][0] + pm1->dm[3][1] * pm2->dm[1][0] + pm1->dm[3][2] * pm2->dm[2][0] + pm1->dm[3][3] * pm2->dm[3][0];
    out.dm[3][1] = pm1->dm[3][0] * pm2->dm[0][1] + pm1->dm[3][1] * pm2->dm[1][1] + pm1->dm[3][2] * pm2->dm[2][1] + pm1->dm[3][3] * pm2->dm[3][1];
    out.dm[3][2] = pm1->dm[3][0] * pm2->dm[0][2] + pm1->dm[3][1] * pm2->dm[1][2] + pm1->dm[3][2] * pm2->dm[2][2] + pm1->dm[3][3] * pm2->dm[3][2];
    out.dm[3][3] = pm1->dm[3][0] * pm2->dm[0][3] + pm1->dm[3][1] * pm2->dm[1][3] + pm1->dm[3][2] * pm2->dm[2][3] + pm1->dm[3][3] * pm2->dm[3][3];

    *pout = out;
    //*/
    return pout;
  }

  _float4x4* MatrixRotationYawPitchRoll(_float4x4 *pout, float yaw, float pitch, float roll)
  {
    // TODO: 提高效率
    _float4x4 m;

    MatrixIdentity(pout);
    MatrixRotationZ(&m, roll);
    MatrixMultiply(pout, pout, &m);
    MatrixRotationX(&m, pitch);
    MatrixMultiply(pout, pout, &m);
    MatrixRotationY(&m, yaw);
    MatrixMultiply(pout, pout, &m);
    return pout;
  }

  void MatrixIdentity(_float4x4 *pout)
  {
    memset(&pout->m, 0, sizeof(_float4x4));
    pout->_11 = pout->_22 = pout->_33 = pout->_44 = 1;
  }

  float4x4* MatrixAffineTransformation(float4x4 *pout, const float3* scaling, const float3* rotationcenter, const _quaternion* rotation, const float3* translation)
  {
    _float4x4 m1, m2, m3, m4, m5;

    if(scaling) {
      m1.Scale(scaling->x, scaling->y, scaling->z);
    }
    else {
      m1.identity();
    }

    if ( !rotationcenter )
    {
      m2.identity();
      m4.identity();
    }
    else
    {
      m2.Translate(-rotationcenter->x, -rotationcenter->y, -rotationcenter->z);
      m4.Translate(rotationcenter->x, rotationcenter->y, rotationcenter->z);
    }

    if ( !rotation ) {
      m3.identity();
    }
    else {
      m3.RotationQuaternion(rotation);
    }

    if ( !translation ) {
      m5.identity();
    }
    else {
      m5.Translate(translation->x, translation->y, translation->z);
    }

    *pout = m1 * m2 * m3 * m4 * m5;
    return pout;
  }
  _float4x4* MatrixRotationAxis(_float4x4 *pout, const float3 *pv, float angle)
  {
    float3 v;
    const float s = sin(angle);
    const float c = cos(angle);

    Vec3Normalize(&v,pv);
    MatrixIdentity(pout);
    pout->dm[0][0] = (1.0f - c) * v.x * v.x + c;
    pout->dm[1][0] = (1.0f - c) * v.x * v.y - s * v.z;
    pout->dm[2][0] = (1.0f - c) * v.x * v.z + s * v.y;
    pout->dm[0][1] = (1.0f - c) * v.y * v.x + s * v.z;
    pout->dm[1][1] = (1.0f - c) * v.y * v.y + c;
    pout->dm[2][1] = (1.0f - c) * v.y * v.z - s * v.x;
    pout->dm[0][2] = (1.0f - c) * v.z * v.x - s * v.y;
    pout->dm[1][2] = (1.0f - c) * v.z * v.y + s * v.x;
    pout->dm[2][2] = (1.0f - c) * v.z * v.z + c;
    return pout;
  }

  _float4x4* MatrixPerspectiveFovLH(_float4x4 *pout, float fovy, float aspect, float zn, float zf)
  {
    MatrixIdentity(pout);
    pout->dm[0][0] = 1.0f / (aspect * tan(fovy/2.0f));
    pout->dm[1][1] = 1.0f / tan(fovy/2.0f);
    pout->dm[2][2] = zf / (zf - zn);
    pout->dm[2][3] = 1.0f;
    pout->dm[3][2] = (zf * zn) / (zn - zf);
    pout->dm[3][3] = 0.0f;
    return pout;
  }

  _float4x4* MatrixOrthoLH(_float4x4 *pout, float w, float h, float zn, float zf)
  {
    MatrixIdentity(pout);
    pout->dm[0][0] = 2.0f / w;
    pout->dm[1][1] = 2.0f / h;
    pout->dm[2][2] = 1.0f / (zf - zn);
    pout->dm[3][3] = 1.0f;

    //  pout->dm[2][3] = 1.0f;
    pout->dm[3][2] = -zn / (zf - zn);
    return pout;
  }

  _float3x3* MatrixRotationQuaternion(_float3x3 *pout, const _quaternion *pq)
  {
    const float xx = pq->x * pq->x;
    const float yy = pq->y * pq->y;
    const float zz = pq->z * pq->z;
    const float xy = pq->x * pq->y;
    const float zw = pq->z * pq->w;
    const float xz = pq->x * pq->z;
    const float yw = pq->y * pq->w;
    const float yz = pq->y * pq->z;
    const float xw = pq->x * pq->w;

    pout->dm[0][0] = 1.0f - 2.0f * (yy + zz);
    pout->dm[0][1] = 2.0f * (xy + zw);
    pout->dm[0][2] = 2.0f * (xz - yw);
    pout->dm[1][0] = 2.0f * (xy - zw);
    pout->dm[1][1] = 1.0f - 2.0f * (xx + zz);
    pout->dm[1][2] = 2.0f * (yz + xw);
    pout->dm[2][0] = 2.0f * (xz + yw);
    pout->dm[2][1] = 2.0f * (yz - xw);
    pout->dm[2][2] = 1.0f - 2.0f * (xx + yy);
    return pout;
  }

  _float4x4* MatrixRotationQuaternion(_float4x4 *pout, const _quaternion *pq) // from WINE
  {
    MatrixIdentity(pout);
    const float xx = pq->x * pq->x;
    const float yy = pq->y * pq->y;
    const float zz = pq->z * pq->z;
    const float xy = pq->x * pq->y;
    const float zw = pq->z * pq->w;
    const float xz = pq->x * pq->z;
    const float yw = pq->y * pq->w;
    const float yz = pq->y * pq->z;
    const float xw = pq->x * pq->w;

    pout->dm[0][0] = 1.0f - 2.0f * (yy + zz);
    pout->dm[0][1] = 2.0f * (xy + zw);
    pout->dm[0][2] = 2.0f * (xz - yw);
    pout->dm[1][0] = 2.0f * (xy - zw);
    pout->dm[1][1] = 1.0f - 2.0f * (xx + zz);
    pout->dm[1][2] = 2.0f * (yz + xw);
    pout->dm[2][0] = 2.0f * (xz + yw);
    pout->dm[2][1] = 2.0f * (yz - xw);
    pout->dm[2][2] = 1.0f - 2.0f * (xx + yy);
    return pout;
  }

  _float4x4* MatrixInverse(_float4x4* pout, float* pDeterminant, const _float4x4* pin)
  {
    ASSERT(pout != pin);
    float fDet = 1.0f / _float4x4::Det3x3(pin->_11, pin->_12, pin->_13, pin->_21, pin->_22, pin->_23, pin->_31, pin->_32, pin->_33);
    *pDeterminant = fDet;

    pout->_11 =  fDet * _float4x4::Det3x3(pin->_22, pin->_23, pin->_24, pin->_32, pin->_33, pin->_34, pin->_42, pin->_43, pin->_44);
    pout->_12 = -fDet * _float4x4::Det3x3(pin->_12, pin->_13, pin->_14, pin->_32, pin->_33, pin->_34, pin->_42, pin->_43, pin->_44);
    pout->_13 =  fDet * _float4x4::Det3x3(pin->_12, pin->_13, pin->_14, pin->_22, pin->_23, pin->_24, pin->_42, pin->_43, pin->_44);
    pout->_14 = -fDet * _float4x4::Det3x3(pin->_12, pin->_13, pin->_14, pin->_22, pin->_23, pin->_24, pin->_32, pin->_33, pin->_34);

    pout->_21 = -fDet * _float4x4::Det3x3(pin->_21, pin->_23, pin->_24, pin->_31, pin->_33, pin->_34, pin->_41, pin->_43, pin->_44);
    pout->_22 =  fDet * _float4x4::Det3x3(pin->_11, pin->_13, pin->_14, pin->_31, pin->_33, pin->_34, pin->_41, pin->_43, pin->_44);
    pout->_23 = -fDet * _float4x4::Det3x3(pin->_11, pin->_13, pin->_14, pin->_21, pin->_23, pin->_24, pin->_41, pin->_43, pin->_44);
    pout->_24 =  fDet * _float4x4::Det3x3(pin->_11, pin->_13, pin->_14, pin->_21, pin->_23, pin->_24, pin->_31, pin->_33, pin->_34);

    pout->_31 =  fDet * _float4x4::Det3x3(pin->_21, pin->_22, pin->_24, pin->_31, pin->_32, pin->_34, pin->_41, pin->_42, pin->_44);
    pout->_32 = -fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_14, pin->_31, pin->_32, pin->_34, pin->_41, pin->_42, pin->_44);
    pout->_33 =  fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_14, pin->_21, pin->_22, pin->_24, pin->_41, pin->_42, pin->_44);
    pout->_34 = -fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_14, pin->_21, pin->_22, pin->_24, pin->_31, pin->_32, pin->_34);

    pout->_41 = -fDet * _float4x4::Det3x3(pin->_21, pin->_22, pin->_23, pin->_31, pin->_32, pin->_33, pin->_41, pin->_42, pin->_43);
    pout->_42 =  fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_13, pin->_31, pin->_32, pin->_33, pin->_41, pin->_42, pin->_43);
    pout->_43 = -fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_13, pin->_21, pin->_22, pin->_23, pin->_41, pin->_42, pin->_43);
    pout->_44 =  fDet * _float4x4::Det3x3(pin->_11, pin->_12, pin->_13, pin->_21, pin->_22, pin->_23, pin->_31, pin->_32, pin->_33);

    return pout;
  }

  _float4x4* MatrixInverseGaussJordan(_float4x4* pout, float* pDeterminant, const _float4x4* pin) // 高斯-约旦法（全选主元）求逆
  {
    _float4x4 m(*pin);
    u32 is[4];
    u32 js[4];
    float fDet = 1.0f;
    int f = 1;

    for (int k = 0; k < 4; k ++)
    {
      // 第一步，全选主元
      float fMax = 0.0f;
      for (u32 i = k; i < 4; i ++)
      {
        for (u32 j = k; j < 4; j ++)
        {
          const float fTest = fabs(m.dm[i][j]);
          if (fTest > fMax)
          {
            fMax  = fTest;
            is[k]  = i;
            js[k]  = j;
          }
        }
      }
      if (fabs(fMax) < 1e-10f) {
        return MatrixInverse(pout, pDeterminant, pin);
        //ASSERT(0);
        //return 0;
      }

      if (is[k] != k)
      {
        f = -f;
        clSwap32(m.dm[k][0], m.dm[is[k]][0]);
        clSwap32(m.dm[k][1], m.dm[is[k]][1]);
        clSwap32(m.dm[k][2], m.dm[is[k]][2]);
        clSwap32(m.dm[k][3], m.dm[is[k]][3]);
      }
      if (js[k] != k)
      {
        f = -f;
        clSwap32(m.dm[0][k], m.dm[0][js[k]]);
        clSwap32(m.dm[1][k], m.dm[1][js[k]]);
        clSwap32(m.dm[2][k], m.dm[2][js[k]]);
        clSwap32(m.dm[3][k], m.dm[3][js[k]]);
      }

      // 计算行列值
      fDet *= m.dm[k][k];

      // 计算逆矩阵

      // 第二步
      m.dm[k][k] = 1.0f / m.dm[k][k];  
      // 第三步
      for (u32 j = 0; j < 4; j ++)
      {
        if (j != k)
          m.dm[k][j] *= m.dm[k][k];
      }
      // 第四步
      for (u32 i = 0; i < 4; i ++)
      {
        if (i != k)
        {
          for  (u32 j = 0; j < 4; j ++)
          {
            if (j != k)
              m.dm[i][j] = m.dm[i][j] - m.dm[i][k] * m.dm[k][j];
          }
        }
      }
      // 第五步
      for (u32 i = 0; i < 4; i ++)
      {
        if (i != k)
          m.dm[i][k] *= -m.dm[k][k];
      }
    }

    for  (int k = 3; k >= 0; k --)
    {
      if (js[k] != k)
      {
        clSwap32(m.dm[k][0], m.dm[js[k]][0]);
        clSwap32(m.dm[k][1], m.dm[js[k]][1]);
        clSwap32(m.dm[k][2], m.dm[js[k]][2]);
        clSwap32(m.dm[k][3], m.dm[js[k]][3]);
      }
      if (is[k] != k)
      {
        clSwap32(m.dm[0][k], m.dm[0][is[k]]);
        clSwap32(m.dm[1][k], m.dm[1][is[k]]);
        clSwap32(m.dm[2][k], m.dm[2][is[k]]);
        clSwap32(m.dm[3][k], m.dm[3][is[k]]);
      }
    }

    *pout = m;
    *pDeterminant = fDet;
    return pout;
  }

  b32 MatrixDecompose(float3* poutscale, _quaternion* poutrotation, float3 *pouttranslation, const float4x4 *pm)
  {
    //float4x4 normalized;
    //float3 vec;

    ///*Compute the scaling part.*/
    //vec.x = pm->dm[0][0];
    //vec.y = pm->dm[0][1];
    //vec.z = pm->dm[0][2];
    //poutscale->x = float3::length(vec);

    //vec.x = pm->dm[1][0];
    //vec.y = pm->dm[1][1];
    //vec.z = pm->dm[1][2];
    //poutscale->y = float3::length(vec);

    //vec.x = pm->dm[2][0];
    //vec.y = pm->dm[2][1];
    //vec.z = pm->dm[2][2];
    //poutscale->z = float3::length(vec);

    ///*Compute the translation part.*/
    //pouttranslation->x = pm->dm[3][0];
    //pouttranslation->y = pm->dm[3][1];
    //pouttranslation->z = pm->dm[3][2];

    ///*Let's calculate the rotation now*/
    //if ( (poutscale->x == 0.0f) || (poutscale->y == 0.0f) || (poutscale->z == 0.0f) ) return FALSE;

    //normalized.dm[0][0] = pm->dm[0][0] / poutscale->x;
    //normalized.dm[0][1] = pm->dm[0][1] / poutscale->x;
    //normalized.dm[0][2] = pm->dm[0][2] / poutscale->x;
    //normalized.dm[1][0] = pm->dm[1][0] / poutscale->y;
    //normalized.dm[1][1] = pm->dm[1][1] / poutscale->y;
    //normalized.dm[1][2] = pm->dm[1][2] / poutscale->y;
    //normalized.dm[2][0] = pm->dm[2][0] / poutscale->z;
    //normalized.dm[2][1] = pm->dm[2][1] / poutscale->z;
    //normalized.dm[2][2] = pm->dm[2][2] / poutscale->z;

    //QuaternionRotationMatrix(poutrotation, &normalized);
    MatrixDecomposeTranslation(pm, pouttranslation);
    return MatrixDecomposeScalingRotation(pm, poutscale, poutrotation);
  }

  void MatrixDecomposeScaling(const _float4x4 *pm, _float3* poutscale)
  {
    float3 vec;
    /*Compute the scaling part.*/
    vec.x = pm->dm[0][0];
    vec.y = pm->dm[0][1];
    vec.z = pm->dm[0][2];
    poutscale->x = float3::length(vec);

    vec.x = pm->dm[1][0];
    vec.y = pm->dm[1][1];
    vec.z = pm->dm[1][2];
    poutscale->y = float3::length(vec);

    vec.x = pm->dm[2][0];
    vec.y = pm->dm[2][1];
    vec.z = pm->dm[2][2];
    poutscale->z = float3::length(vec);
  }

  b32 MatrixDecomposeScalingRotation(const _float4x4 *pm, _float3* poutscale, _quaternion* poutrotation)
  {
    float3 vec;
    float4x4 normalized;
    /*Compute the scaling part.*/
    vec.x = pm->dm[0][0];
    vec.y = pm->dm[0][1];
    vec.z = pm->dm[0][2];
    poutscale->x = float3::length(vec);

    vec.x = pm->dm[1][0];
    vec.y = pm->dm[1][1];
    vec.z = pm->dm[1][2];
    poutscale->y = float3::length(vec);

    vec.x = pm->dm[2][0];
    vec.y = pm->dm[2][1];
    vec.z = pm->dm[2][2];
    poutscale->z = float3::length(vec);

    /*Let's calculate the rotation now*/
    if ( (poutscale->x == 0.0f) || (poutscale->y == 0.0f) || (poutscale->z == 0.0f) ) return FALSE;

    normalized.dm[0][0] = pm->dm[0][0] / poutscale->x;
    normalized.dm[0][1] = pm->dm[0][1] / poutscale->x;
    normalized.dm[0][2] = pm->dm[0][2] / poutscale->x;
    normalized.dm[1][0] = pm->dm[1][0] / poutscale->y;
    normalized.dm[1][1] = pm->dm[1][1] / poutscale->y;
    normalized.dm[1][2] = pm->dm[1][2] / poutscale->y;
    normalized.dm[2][0] = pm->dm[2][0] / poutscale->z;
    normalized.dm[2][1] = pm->dm[2][1] / poutscale->z;
    normalized.dm[2][2] = pm->dm[2][2] / poutscale->z;

    QuaternionRotationMatrix(poutrotation, &normalized);
    return TRUE;
  }

  void MatrixDecomposeTranslation(const _float4x4 *pm, _float3 *pouttranslation)
  {
    /*Compute the translation part.*/
    pouttranslation->x = pm->dm[3][0];
    pouttranslation->y = pm->dm[3][1];
    pouttranslation->z = pm->dm[3][2];
  }

} // namespace clstd