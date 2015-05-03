#include "clstd.h"

namespace clstd
{
  template<class MatrixT>
  _quaternion* QuaternionRotationMatrixT(_quaternion *pout, CLCONST MatrixT& m)
  {
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    //int i, maxi;
    //float maxdiag, S;
    float trace;

    trace = m.dm[0][0] + m.dm[1][1] + m.dm[2][2] + 1.0f;
    if ( trace > 1.0f )
    {
      // |w| > 1/2, may as well choose w > 1/2

      const float s = 2.0f * sqrt(trace);
      pout->x = ( m.dm[1][2] - m.dm[2][1] ) / s;
      pout->y = ( m.dm[2][0] - m.dm[0][2] ) / s;
      pout->z = ( m.dm[0][1] - m.dm[1][0] ) / s;
      pout->w = s * 0.25f;
      return pout;
    }
    else {
      // |w| <= 1/2
      static size_t s_iNext[3] = { 1, 2, 0 };
      size_t i = 0;
      if ( m.dm[1][1] > m.dm[0][0] )
        i = 1;
      if ( m.dm[2][2] > m.dm[i][i] )
        i = 2;
      size_t j = s_iNext[i];
      size_t k = s_iNext[j];

      float fRoot = sqrt(m.dm[i][i] - m.dm[j][j] - m.dm[k][k] + 1.0f);
      float* apkQuat[3] = { &pout->x, &pout->y, &pout->z };
      *apkQuat[i] = 0.5f*fRoot;
      fRoot = 0.5f / fRoot;
      pout->w = (m.dm[j][k] - m.dm[k][j])*fRoot;
      *apkQuat[j] = (m.dm[i][j] + m.dm[j][i]) * fRoot;
      *apkQuat[k] = (m.dm[i][k] + m.dm[k][i]) * fRoot;

      //maxi = 0;
      //maxdiag = m.dm[0][0];
      //for (i=1; i<3; i++)
      //{
      //  if ( m.dm[i][i] > maxdiag )
      //  {
      //    maxi = i;
      //    maxdiag = m.dm[i][i];
      //  }
      //}
      //switch( maxi )
      //{
      //case 0:
      //  S = 2.0f * sqrt(1.0f + m.dm[0][0] - m.dm[1][1] - m.dm[2][2]);
      //  pout->x = 0.25f * S;
      //  pout->y = ( m.dm[0][1] + m.dm[1][0] ) / S;
      //  pout->z = ( m.dm[0][2] + m.dm[2][0] ) / S;
      //  pout->w = ( m.dm[1][2] - m.dm[2][1] ) / S;
      //  break;
      //case 1:
      //  S = 2.0f * sqrt(1.0f + m.dm[1][1] - m.dm[0][0] - m.dm[2][2]);
      //  pout->x = ( m.dm[0][1] + m.dm[1][0] ) / S;
      //  pout->y = 0.25f * S;
      //  pout->z = ( m.dm[1][2] + m.dm[2][1] ) / S;
      //  pout->w = ( m.dm[2][0] - m.dm[0][2] ) / S;
      //  break;
      //case 2:
      //  S = 2.0f * sqrt(1.0f + m.dm[2][2] - m.dm[0][0] - m.dm[1][1]);
      //  pout->x = ( m.dm[0][2] + m.dm[2][0] ) / S;
      //  pout->y = ( m.dm[1][2] + m.dm[2][1] ) / S;
      //  pout->z = 0.25f * S;
      //  pout->w = ( m.dm[0][1] - m.dm[1][0] ) / S;
      //  break;
      //}
    }
    return pout;
  }

  //////////////////////////////////////////////////////////////////////////

  _quaternion _quaternion::operator*(CLCONST _quaternion& q) CLCONST
  {
    _quaternion out;
    return *QuaternionMultiply(&out, this, &q);
  }

  _quaternion& _quaternion::operator*=(CLCONST _quaternion& q)
  {
    return *QuaternionMultiply(this, this, &q);
  }

  _quaternion& _quaternion::normalize()
  {
    return *QuaternionNormalize(this, this);
  }

  _quaternion& _quaternion::FromRotationMatrix(CLCONST _float4x4& m)
  {
    return *QuaternionRotationMatrixT(this, m);
  }

  _quaternion& _quaternion::FromRotationMatrix(CLCONST _float3x3& m)
  {

    return *QuaternionRotationMatrixT(this, m);
    //float fTrace = m.dm[0][0] + m.dm[1][1] + m.dm[2][2];
    //float fRoot;

    //if ( fTrace > 0.0 )
    //{
    //  
    //  fRoot = sqrt(fTrace + 1.0f);  // 2w
    //  w = 0.5f * fRoot;
    //  fRoot = 0.5f / fRoot;  // 1/(4w)
    //  x = (m.dm[2][1] - m.dm[1][2]) * fRoot;
    //  y = (m.dm[0][2] - m.dm[2][0]) * fRoot;
    //  z = (m.dm[1][0] - m.dm[0][1]) * fRoot;
    //}
    //else
    //{
    //  // |w| <= 1/2
    //  static size_t s_iNext[3] = { 1, 2, 0 };
    //  size_t i = 0;
    //  if ( m.dm[1][1] > m.dm[0][0] )
    //    i = 1;
    //  if ( m.dm[2][2] > m.dm[i][i] )
    //    i = 2;
    //  size_t j = s_iNext[i];
    //  size_t k = s_iNext[j];

    //  fRoot = sqrt(m.dm[i][i] - m.dm[j][j] - m.dm[k][k] + 1.0f);
    //  float* apkQuat[3] = { &x, &y, &z };
    //  *apkQuat[i] = 0.5f*fRoot;
    //  fRoot = 0.5f / fRoot;
    //  w = (m.dm[k][j] - m.dm[j][k])*fRoot;
    //  *apkQuat[j] = (m.dm[j][i] + m.dm[i][j]) * fRoot;
    //  *apkQuat[k] = (m.dm[k][i] + m.dm[i][k]) * fRoot;
    //}
    //return *this;
  }

  _quaternion& _quaternion::YawPitchRollA(float yaw, float pitch, float roll)
  {
    return *QuaternionRotationYawPitchRoll(this, CL_AGNLE2RAD(yaw), CL_AGNLE2RAD(pitch), CL_AGNLE2RAD(roll));
  }

  _quaternion& _quaternion::YawPitchRollR(float yaw, float pitch, float roll)
  {
    return *QuaternionRotationYawPitchRoll(this, yaw, pitch, roll);
  }

  _quaternion& _quaternion::YawPitchRollA(const _euler& e)
  {
    return *QuaternionRotationYawPitchRoll(this, CL_AGNLE2RAD(e.yaw), CL_AGNLE2RAD(e.pitch), CL_AGNLE2RAD(e.roll));
  }

  _quaternion& _quaternion::YawPitchRollR(const _euler& e)
  {
    return *QuaternionRotationYawPitchRoll(this, e.yaw, e.pitch, e.roll);
  }

  float3x3 _quaternion::ToMatrix3x3() const
  {
    float3x3 mat;
    return mat.FromQuaternion(this);
  }

  float4x4 _quaternion::ToMatrix() const
  {
    float4x4 mat;
    return mat.RotationQuaternion(this);
  }

  float _quaternion::GetRoll(bool reprojectAxis) const
  {
    if (reprojectAxis)
    {
      // roll = atan2(localx.y, localx.x)
      // pick parts of xAxis() implementation that we need
      //			Real fTx  = 2.0*x;
      float fTy  = 2.0f*y;
      float fTz  = 2.0f*z;
      float fTwz = fTz*w;
      float fTxy = fTy*x;
      float fTyy = fTy*y;
      float fTzz = fTz*z;

      // Vector3(1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

      return atan2(fTxy+fTwz, 1.0f-(fTyy+fTzz));

    }
    else
    {
      return atan2(2*(x*y + w*z), w*w + x*x - y*y - z*z);
    }
  }
  //-----------------------------------------------------------------------
  float _quaternion::GetPitch(bool reprojectAxis) const
  {
    if (reprojectAxis)
    {
      // pitch = atan2(localy.z, localy.y)
      // pick parts of yAxis() implementation that we need
      float fTx  = 2.0f*x;
   // float fTy  = 2.0f*y;
      float fTz  = 2.0f*z;
      float fTwx = fTx*w;
      float fTxx = fTx*x;
      float fTyz = fTz*y;
      float fTzz = fTz*z;

      // Vector3(fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx);
      return atan2(fTyz+fTwx, 1.0f-(fTxx+fTzz));
    }
    else
    {
      // internal version
      return atan2(2*(y*z + w*x), w*w - x*x - y*y + z*z);
    }
  }
  //-----------------------------------------------------------------------
  float _quaternion::GetYaw(bool reprojectAxis) const
  {
    if (reprojectAxis)
    {
      // yaw = atan2(localz.x, localz.z)
      // pick parts of zAxis() implementation that we need
      float fTx  = 2.0f*x;
      float fTy  = 2.0f*y;
      float fTz  = 2.0f*z;
      float fTwy = fTy*w;
      float fTxx = fTx*x;
      float fTxz = fTz*x;
      float fTyy = fTy*y;

      // Vector3(fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy));

      return atan2(fTxz+fTwy, 1.0f-(fTxx+fTyy));

    }
    else
    {
      // internal version
      return asin(-2*(x*z - w*y));
    }
  }


  //////////////////////////////////////////////////////////////////////////
  //
  // Quaternion
  //

  _quaternion* QuaternionBaryCentric(_quaternion *pout, CLCONST _quaternion *pq1, CLCONST _quaternion *pq2, CLCONST _quaternion *pq3, float f, float g)
  {
    _quaternion temp1, temp2;
    QuaternionSlerp(pout, QuaternionSlerp(&temp1, pq1, pq2, f + g), QuaternionSlerp(&temp2, pq1, pq3, f+g), g / (f + g));
    return pout;
  }

  _quaternion* QuaternionExp(_quaternion *pout, CLCONST _quaternion *pq)
  {
    float norm;

    norm = sqrt(pq->x * pq->x + pq->y * pq->y + pq->z * pq->z);
    if (norm )
    {
      pout->x = sin(norm) * pq->x / norm;
      pout->y = sin(norm) * pq->y / norm;
      pout->z = sin(norm) * pq->z / norm;
      pout->w = cos(norm);
    }
    else
    {
      pout->x = 0.0f;
      pout->y = 0.0f;
      pout->z = 0.0f;
      pout->w = 1.0f;
    }
    return pout;
  }

  _quaternion* QuaternionInverse(_quaternion *pout, CLCONST _quaternion *pq)
  {
    _quaternion out;
    float norm;

    norm = QuaternionLengthSq(pq);

    out.x = -pq->x / norm;
    out.y = -pq->y / norm;
    out.z = -pq->z / norm;
    out.w =  pq->w / norm;

    *pout = out;
    return pout;
  }

  _quaternion* QuaternionLn(_quaternion *pout, CLCONST _quaternion *pq)
  {
    FLOAT norm, normvec, theta;

    norm = QuaternionLengthSq(pq);
    if ( norm > 1.0001f )
    {
      pout->x = pq->x;
      pout->y = pq->y;
      pout->z = pq->z;
      pout->w = 0.0f;
    }
    else if( norm > 0.99999f)
    {
      normvec = sqrt( pq->x * pq->x + pq->y * pq->y + pq->z * pq->z );
      theta = atan2(normvec, pq->w) / normvec;
      pout->x = theta * pq->x;
      pout->y = theta * pq->y;
      pout->z = theta * pq->z;
      pout->w = 0.0f;
    }
    else
    {
      TRACE("The quaternion (%f, %f, %f, %f) has a norm <1. This should not happen. Windows returns a result anyway. This case is not implemented yet.\n", pq->x, pq->y, pq->z, pq->w);
    }
    return pout;
  }

  _quaternion* QuaternionMultiply(_quaternion *pout, CLCONST _quaternion *pq1, CLCONST _quaternion *pq2)
  {
    _quaternion out;
    out.x = pq2->w * pq1->x + pq2->x * pq1->w + pq2->y * pq1->z - pq2->z * pq1->y;
    out.y = pq2->w * pq1->y - pq2->x * pq1->z + pq2->y * pq1->w + pq2->z * pq1->x;
    out.z = pq2->w * pq1->z + pq2->x * pq1->y - pq2->y * pq1->x + pq2->z * pq1->w;
    out.w = pq2->w * pq1->w - pq2->x * pq1->x - pq2->y * pq1->y - pq2->z * pq1->z;
    *pout = out;
    return pout;
  }

  _quaternion* QuaternionNormalize(_quaternion *pout, CLCONST _quaternion *pq)
  {
    _quaternion out;
    FLOAT norm;

    norm = QuaternionLength(pq);

    out.x = pq->x / norm;
    out.y = pq->y / norm;
    out.z = pq->z / norm;
    out.w = pq->w / norm;

    *pout=out;

    return pout;
  }

  _quaternion* QuaternionRotationAxis(_quaternion *pout, CLCONST _float3 *pv, float angle)
  {
    _float3 temp;
    const float fHalfAngle = angle * 0.5f;
    Vec3Normalize(&temp, pv);
    pout->x = sin( fHalfAngle ) * temp.x;
    pout->y = sin( fHalfAngle ) * temp.y;
    pout->z = sin( fHalfAngle ) * temp.z;
    pout->w = cos( fHalfAngle );
    return pout;
  }

  _quaternion* QuaternionRotationMatrix(_quaternion *pout, CLCONST _float4x4 *pm)
  {
    return QuaternionRotationMatrixT(pout, *pm);
  }


  _quaternion* QuaternionRotationYawPitchRoll(_quaternion *pout, float yaw, float pitch, float roll)
  {
    const float fHalfYaw   = yaw * 0.5f;
    const float fHalfPitch = pitch * 0.5f;
    const float fHalfRoll  = roll * 0.5f;

    const float fSinHalfYaw = sin(fHalfYaw);
    const float fCosHalfYaw = cos(fHalfYaw);

    const float fSinHalfRoll = sin(fHalfRoll);
    const float fCosHalfRoll = cos(fHalfRoll);

    const float fSinHalfPitch = sin(fHalfPitch);
    const float fCosHalfPitch = cos(fHalfPitch);

    const float fYawPitchSCQ = fSinHalfYaw * fCosHalfPitch;
    const float fYawPitchCSQ = fCosHalfYaw * fSinHalfPitch;
    const float fYawPitchCCQ = fCosHalfYaw * fCosHalfPitch;
    const float fYawPitchSSQ = fSinHalfYaw * fSinHalfPitch;

    pout->x = (float)(fYawPitchSCQ * fSinHalfRoll + fYawPitchCSQ * fCosHalfRoll);
    pout->y = (float)(fYawPitchSCQ * fCosHalfRoll - fYawPitchCSQ * fSinHalfRoll);
    pout->z = (float)(fYawPitchCCQ * fSinHalfRoll - fYawPitchSSQ * fCosHalfRoll);
    pout->w = (float)(fYawPitchCCQ * fCosHalfRoll + fYawPitchSSQ * fSinHalfRoll);
    return pout;
  }

  //_quaternion* QuaternionDecomposeYawPitchRoll(_quaternion *pout, float yaw, float pitch, float roll)
  //{
  //  const float fHalfYaw   = yaw * 0.5f;
  //  const float fHalfPitch = pitch * 0.5f;
  //  const float fHalfRoll  = roll * 0.5f;

  //  const float fSinYawH = sin(fHalfYaw);
  //  const float fCosYawH = cos(fHalfYaw);

  //  const float fSinRollH = sin(fHalfRoll);
  //  const float fCosRollH = cos(fHalfRoll);

  //  const float fSinPitchH = sin(fHalfPitch);
  //  const float fCosPitchH = cos(fHalfPitch);

  //  pout->x = (float)(fSinYawH * fCosPitchH * fSinRollH + fCosYawH * fSinPitchH * fCosRollH);
  //  pout->y = (float)(fSinYawH * fCosPitchH * fCosRollH - fCosYawH * fSinPitchH * fSinRollH);
  //  pout->z = (float)(fCosYawH * fCosPitchH * fSinRollH - fSinYawH * fSinPitchH * fCosRollH);
  //  pout->w = (float)(fCosYawH * fCosPitchH * fCosRollH + fSinYawH * fSinPitchH * fSinRollH);

  //  return pout;
  //}

  _quaternion* QuaternionSlerp(_quaternion *pout, CLCONST _quaternion *pq1, CLCONST _quaternion *pq2, FLOAT t)
  {
    float dot, epsilon, temp, theta, u;

    epsilon = 1.0f;
    temp = 1.0f - t;
    u = t;
    dot = QuaternionDot(pq1, pq2);
    if ( dot < 0.0f )
    {
      epsilon = -1.0f;
      dot = -dot;
    }
    if( 1.0f - dot > 0.001f )
    {
      theta = acos(dot);
      const float fSinTheta = sin(theta);
      temp  = sin(theta * temp) / fSinTheta;
      u = sin(theta * u) / fSinTheta;
    }
    const float fEpsilonU = epsilon * u;
    pout->x = temp * pq1->x + fEpsilonU * pq2->x;
    pout->y = temp * pq1->y + fEpsilonU * pq2->y;
    pout->z = temp * pq1->z + fEpsilonU * pq2->z;
    pout->w = temp * pq1->w + fEpsilonU * pq2->w;
    return pout;
  }

  _quaternion* QuaternionSquad(_quaternion *pout, CLCONST _quaternion *pq1, CLCONST _quaternion *pq2, CLCONST _quaternion *pq3, CLCONST _quaternion *pq4, float t)
  {
    _quaternion temp1, temp2;

    QuaternionSlerp(pout, QuaternionSlerp(&temp1, pq1, pq4, t), QuaternionSlerp(&temp2, pq2, pq3, t), 2.0f * t * (1.0f - t));
    return pout;
  }

  void QuaternionToAxisAngle(CLCONST _quaternion *pq, _float3 *paxis, float *pangle)
  {
    paxis->x = pq->x;
    paxis->y = pq->y;
    paxis->z = pq->z;
    *pangle = 2.0f * acos(pq->w);
  }


  _euler& _euler::positivity()
  {
    while(x < 0) {
      x += CL_PI;
    }
    while(y < 0) {
      y += CL_PI;
    }
    while(z < 0) {
      z += CL_PI;
    }
    return *this;
  }

  _euler& _euler::negativity()
  {
    while(x > 0) {
      x -= CL_PI;
    }
    while(y > 0) {
      y -= CL_PI;
    }
    while(z > 0) {
      z -= CL_PI;
    }
    return *this;
  }

} // namespace clstd