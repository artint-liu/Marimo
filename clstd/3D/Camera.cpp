//#include <d3dx9.h>
#include "../clstd.h"
#include "cl3d.h"
//#include "../floatx.h"
#include "Camera.h"
#include "clTransform.h"

namespace clstd
{

  Camera::Camera()
    : m_vEyePt    (0.0f)
    //, m_vLookatPt (0,0,1)
    //, m_vUpVec    (float3::AxisY)
    //, m_vLeft     (-float3::AxisX)
    , m_vRight    (float3::AxisX)
    , m_vTop      (float3::AxisY)
    , m_vFront    (float3::AxisZ)
    , m_matView   (float4x4::Identity)
    , m_matProj   (float4x4::Identity)
    //, m_matViewProj(float4x4::Identity)
    , m_fAspect   (1.0f)
    , m_fovy      (CL_PI / 4.0f)
    , m_fNear     (1.0f)
    , m_fFar      (1000.0f)
    , m_eProjType (Unknown)
  {
  }

  Camera::~Camera()
  {
    //m_vLeftVec
    //m_vFrontVec
    //m_matView
    //m_matProj
    //m_matViewProj
  }

  bool Camera::InitializePerspectiveLH(const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar, float fovy, float fAspect)
  {
    m_vEyePt    = vEye;
    //m_vLookatPt = vLookAt;
    //m_vUpVec    = vUp;
    m_fovy      = fovy;
    m_fAspect   = fAspect;
    m_fNear     = fNear;
    m_fFar      = fFar;
    m_eProjType = PerspectiveLH;

    LookAt(vLookAt, vUp);
    UpdateProjectionMatrix();
    //InitializeCommon();
    return true;
  }

  bool Camera::InitializeOrthoLH(const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar, float w, float h)
  {
    m_vEyePt    = vEye;
    //m_vLookatPt = vLookAt;
    //m_vUpVec    = vUp;
    m_fWidth    = w;
    m_fHeight   = h;
    m_fNear     = fNear;
    m_fFar      = fFar;
    m_eProjType = OrthoLH;

    LookAt(vLookAt, vUp);
    UpdateProjectionMatrix();

    //InitializeCommon();
    return true;
  }

  Camera& Camera::Translate(const float3& translation, enum ESpace space)
  {
    if(space == S_Self)
    {
      m_vEyePt += m_vRight * translation.x + m_vTop * translation.y + m_vFront * translation.z;
      OnEyePositionChanged();
    }
    else if(space == S_World)
    {
      m_vEyePt += translation;
      OnEyePositionChanged();
    }
    return *this;
  }

  Camera& Camera::Rotate(const float3& axis, float radians, enum ESpace space)
  {
    if(space == S_Self)
    {
      float4x4 mat;
      mat.RotationAxis(axis, radians);
      float4x4 matView = m_matView * mat;
      m_vRight.set(matView._11, matView._21, matView._31);
      m_vTop.set(matView._12, matView._22, matView._32);
      m_vFront.set(matView._13, matView._23, matView._33);
      //UpdateViewMatrix();
    }
    else if(space == S_World)
    {
      float3x3 mat;
      mat.RotationAxis(axis, -radians);
      float3x3 matView3x3(m_matView);
      float3x3 r = mat * matView3x3;

      m_vRight.set(r._11, r._21, r._31);
      m_vTop.set(r._12, r._22, r._32);
      m_vFront.set(r._13, r._23, r._33);

      //float4x4 matView(
      //  r._11, r._12, r._13, 0,
      //  r._21, r._22, r._23, 0,
      //  r._31, r._32, r._33, 0,
      //  -float3::dot(float3(r._11, r._21, r._31), m_vEyePt),
      //  -float3::dot(float3(r._12, r._22, r._32), m_vEyePt),
      //  -float3::dot(float3(r._13, r._23, r._33), m_vEyePt), 1);
      //SetViewMatrix(matView);
    }
    else {
      return *this;
    }

    UpdateViewMatrix();
    return *this;
  }

  Camera& Camera::SetRotation(const float3x3& mat)
  {
    m_vRight.set(mat._11, mat._21, mat._31);
    m_vTop.set(mat._12, mat._22, mat._32);
    m_vFront.set(mat._13, mat._23, mat._33);
    UpdateViewMatrix();
    return *this;
  }

  Camera& Camera::SetRotation(const float3& vRight, const float3& vTop, const float3& vFront)
  {
    m_vRight = vRight;
    m_vTop = vTop;
    m_vFront = vFront;
    UpdateViewMatrix();
    return *this;
  }

  Camera& Camera::SetRotation(const _quaternion& quat)
  {
    float3x3 mat;
    mat.FromQuaternion(&quat);
    return SetRotation(mat);
  }

  void Camera::LookAt(const float3& vWorldPos, const float3& up /*= float3::AxisY*/)
  {
    m_vFront = float3::normalize(vWorldPos - m_vEyePt);
    m_vRight = float3::normalize(float3::cross(up, m_vFront));
    m_vTop = float3::cross(m_vFront, m_vRight);
    UpdateViewMatrix();
  }

  //void Camera::InitializeCommon()
  //{
  //  //m_vUpVec.normalize();
  //}

  void Camera::OnEyePositionChanged()
  {
    m_matView._41 = -m_vEyePt.dot(m_matView._11, m_matView._21, m_matView._31);
    m_matView._42 = -m_vEyePt.dot(m_matView._12, m_matView._22, m_matView._32);
    m_matView._43 = -m_vEyePt.dot(m_matView._13, m_matView._23, m_matView._33);
  }

  void Camera::UpdateViewMatrix()
  {
    m_matView._11 = m_vRight.x;
    m_matView._21 = m_vRight.y;
    m_matView._31 = m_vRight.z;

    m_matView._12 = m_vTop.x;
    m_matView._22 = m_vTop.y;
    m_matView._32 = m_vTop.z;

    m_matView._13 = m_vFront.x;
    m_matView._23 = m_vFront.y;
    m_matView._33 = m_vFront.z;

    m_matView._14 = 0;
    m_matView._24 = 0;
    m_matView._34 = 0;
    m_matView._44 = 1;

    OnEyePositionChanged();
    //m_matView.SetColumn(0, float4(m_vRight.x, m_vRight.y, m_vRight.z, -float3::dot(m_vRight, m_vEyePt)));
    //m_matView.SetColumn(1, float4(m_vTop.x, m_vTop.y, m_vTop.z, -float3::dot(m_vTop, m_vEyePt)));
    //m_matView.SetColumn(2, float4(m_vFront.x, m_vFront.y, m_vFront.z, -float3::dot(m_vFront, m_vEyePt)));
    //m_matView.SetColumn(3, float4(0, 0, 0, 1));
  }

  void Camera::UpdateProjectionMatrix()
  {
    switch(m_eProjType)
    {
      //m_matView.LookAtLH(m_vEyePt, m_vLookatPt, m_vUpVec);
    case PerspectiveLH:
      m_matProj.PerspectiveFovLH(m_fovy, m_fAspect, m_fNear, m_fFar);
      break;
    case OrthoLH:
      m_matProj.OrthoLH(m_fWidth, m_fHeight, m_fNear, m_fFar);
      break;
    default:
      ASSERT(0);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //void Camera::RotateEye(float fHAngle, float fVAngle)
  //{
  //  float4x4  matY;
  //  float4x4  matX;
  //  float3    vEyesInput = m_vEyePt - m_vLookatPt;
  //  //float4    vEyesOutput;
  //  float    fAngle;
  //  float    fLength;
  //  float    fOriginalY;

  //  fAngle        = -atan2(vEyesInput.z, vEyesInput.x);
  //  fOriginalY    = vEyesInput.y;
  //  vEyesInput.y  = 0;
  //  fLength       = vEyesInput.length();

  //  //D3DXMatrixRotationY( &matY, fHAngle + fAngle );
  //  //D3DXMatrixRotationZ( &matX, fVAngle );
  //  matY.RotationY( fHAngle + fAngle );
  //  matX.RotationZ( fVAngle );

  //  //D3DXVec3Transform(&vEyesOutput, &D3DXVECTOR3(fLength, fOriginalY, 0), &(matX * matY));
  //  float3 vEyes(fLength, fOriginalY, 0);
  //  vEyes *= (matX * matY);
  //  m_vEyePt = m_vLookatPt + vEyes;
  //}

  //void Camera::DeltaPitch(float fDelta)
  //{
  //  float4x4 matTemp;
  //  float3 vDelta = m_vLookatPt - m_vEyePt;
  //  matTemp.RotationAxis(m_vRight, fDelta);
  //  vDelta *= matTemp;
  //  m_vLookatPt = vDelta + m_vEyePt;
  //}

  //void Camera::DeltaYaw(float fDelta)
  //{
  //  float4x4 matTemp;
  //  float3 vDelta = m_vLookatPt - m_vEyePt;
  //  matTemp.RotationAxis(m_vTop, fDelta);
  //  vDelta *= matTemp;
  //  m_vLookatPt = vDelta + m_vEyePt;
  //}

  //void Camera::DeltaRoll(float fDelta)
  //{
  //  float4x4 matTemp;
  //  float3 vDelta = m_vLookatPt - m_vEyePt;
  //  matTemp.RotationAxis(m_vFront, fDelta);
  //  vDelta *= matTemp;
  //  m_vLookatPt = vDelta + m_vEyePt;
  //}

  //void Camera::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
  //{
  //  float3    vDelta = m_vLookatPt - m_vEyePt;
  //  float4x4  matDelta;
  //  matDelta.RotationYawPitchRollR(Yaw, Pitch, Roll);
  //  m_vLookatPt = vDelta * matDelta + m_vEyePt;
  //}

  //b32 Camera::IsSimilarUpDir(CFloat3& vDir, float fEpsilon)
  //{
  //  const float fDot = float3::dot(vDir, m_vUpVec);
  //  return fDot < -fEpsilon ||fDot > fEpsilon;
  //}

  //void Camera::Forward(float fSize, bool bSetLookat)
  //{
  //  float3    vDirection = m_vLookatPt - m_vEyePt;
  //  //float3    vNormal;

  //  vDirection.normalize();
  //  vDirection = vDirection * fSize;

  //  m_vEyePt += vDirection;
  //  if(bSetLookat != false)
  //    m_vLookatPt += vDirection;
  //}

  //void Camera::Translation(float xOffset, float yOffset)
  //{

  //  float3  zaxis, xaxis, yaxis;
  //  zaxis = m_vLookatPt - m_vEyePt;
  //  zaxis.normalize();

  //  xaxis = float3::cross(m_vUpVec, zaxis);
  //  xaxis.normalize();
  //  //D3DXVec3Normalize(&xaxis, &xaxis);
  //  yaxis = float3::cross(zaxis, xaxis);
  //  yaxis.normalize();

  //  xaxis *= -xOffset;
  //  yaxis *= yOffset;

  //  m_vEyePt = m_vEyePt + xaxis + yaxis;
  //  m_vLookatPt = m_vLookatPt + xaxis + yaxis;
  //}

  void Camera::SetViewMatrix(const float4x4& matView)
  {
    m_matView = matView;
    float4 vRight = matView.GetColumn(0);
    float4 vTop = matView.GetColumn(1);
    float4 vFront = matView.GetColumn(2);
    m_vRight.set(vRight.x, vRight.y, vRight.z);
    m_vTop.set(vTop.x, vTop.y, vTop.z);
    m_vFront.set(vFront.x, vFront.y, vFront.z);
    //m_vLeft = - m_vRight;
    
    //float len = (m_vEyePt - m_vLookatPt).length();
    float4x4 matInvView = matView;
    matInvView.inverse();
    m_vEyePt = matInvView.GetRow(3);
    //m_vLookatPt = m_vEyePt + float3::normalize(m_vFront) * len;

    //m_matViewProj = m_matView * m_matProj;
  }

#if 0
  void Camera::_UpdateMat()
  {
    m_vFront = float3::normalize(m_vLookatPt - m_vEyePt);
    m_vRight = float3::normalize(float3::cross(m_vUpVec, m_vFront));
    m_vTop   = float3::cross(m_vFront, m_vRight);

    //m_vLeft = -m_vRight;

    m_matView.SetColumn(0, float4(m_vRight.x, m_vRight.y, m_vRight.z, -float3::dot(m_vRight, m_vEyePt)));
    m_matView.SetColumn(1, float4(m_vTop.x, m_vTop.y, m_vTop.z, -float3::dot(m_vTop, m_vEyePt)));
    m_matView.SetColumn(2, float4(m_vFront.x, m_vFront.y, m_vFront.z, -float3::dot(m_vFront, m_vEyePt)));
    m_matView.SetColumn(3, float4(0, 0, 0, 1));

    switch(m_eProjType)
    {
      //m_matView.LookAtLH(m_vEyePt, m_vLookatPt, m_vUpVec);
    case PerspectiveLH:
      m_matProj.PerspectiveFovLH( m_fovy, m_fAspect, m_fNear, m_fFar);
      break;
    case OrthoLH:
      m_matProj.OrthoLH( m_fWidth, m_fHeight, m_fNear, m_fFar);
      break;
    default:
      ASSERT(0);
    }

    m_matViewProj = m_matView * m_matProj;
  }
#endif
}
