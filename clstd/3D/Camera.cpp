//#include <d3dx9.h>
#include "../clstd.h"
#include "cl3d.h"
//#include "../floatx.h"
#include "Camera.h"

namespace clstd
{

  Camera::Camera()
    : m_vEyePt    (0.0f)
    , m_vLookatPt (0,0,1)
    , m_vUpVec    (float3::AxisY)
    , m_vLeft     (-float3::AxisX)
    , m_vRight    (float3::AxisX)
    , m_vTop      (float3::AxisY)
    , m_vFront    (float3::AxisZ)
    , m_matView   (float4x4::Identity)
    , m_matProj   (float4x4::Identity)
    , m_matViewProj(float4x4::Identity)
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
    m_vLookatPt = vLookAt;
    m_vUpVec    = vUp;
    m_fovy      = fovy;
    m_fAspect   = fAspect;
    m_fNear     = fNear;
    m_fFar      = fFar;
    m_eProjType = PerspectiveLH;

    InitializeCommon();
    return true;
  }

  bool Camera::InitializeOrthoLH(const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar, float w, float h)
  {
    m_vEyePt    = vEye;
    m_vLookatPt = vLookAt;
    m_vUpVec    = vUp;
    m_fWidth    = w;
    m_fHeight   = h;
    m_fNear     = fNear;
    m_fFar      = fFar;
    m_eProjType = OrthoLH;

    InitializeCommon();
    return true;
  }

  void Camera::InitializeCommon()
  {
    m_vUpVec.normalize();
  }

  //////////////////////////////////////////////////////////////////////////
  void Camera::RotateEye(float fHAngle, float fVAngle)
  {
    float4x4  matY;
    float4x4  matX;
    float3    vEyesInput = m_vEyePt - m_vLookatPt;
    //float4    vEyesOutput;
    float    fAngle;
    float    fLength;
    float    fOriginalY;

    fAngle        = -atan2(vEyesInput.z, vEyesInput.x);
    fOriginalY    = vEyesInput.y;
    vEyesInput.y  = 0;
    fLength       = vEyesInput.length();

    //D3DXMatrixRotationY( &matY, fHAngle + fAngle );
    //D3DXMatrixRotationZ( &matX, fVAngle );
    matY.RotationY( fHAngle + fAngle );
    matX.RotationZ( fVAngle );

    //D3DXVec3Transform(&vEyesOutput, &D3DXVECTOR3(fLength, fOriginalY, 0), &(matX * matY));
    float3 vEyes(fLength, fOriginalY, 0);
    vEyes *= (matX * matY);
    m_vEyePt = m_vLookatPt + vEyes;
  }

  void Camera::DeltaPitch(float fDelta)
  {
    float4x4 matTemp;
    float3 vDelta = m_vLookatPt - m_vEyePt;
    matTemp.RotationAxis(m_vRight, fDelta);
    vDelta *= matTemp;
    m_vLookatPt = vDelta + m_vEyePt;
  }

  void Camera::DeltaYaw(float fDelta)
  {
    float4x4 matTemp;
    float3 vDelta = m_vLookatPt - m_vEyePt;
    matTemp.RotationAxis(m_vTop, fDelta);
    vDelta *= matTemp;
    m_vLookatPt = vDelta + m_vEyePt;
  }

  void Camera::DeltaRoll(float fDelta)
  {
    float4x4 matTemp;
    float3 vDelta = m_vLookatPt - m_vEyePt;
    matTemp.RotationAxis(m_vFront, fDelta);
    vDelta *= matTemp;
    m_vLookatPt = vDelta + m_vEyePt;
  }

  void Camera::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
  {
    float3    vDelta = m_vLookatPt - m_vEyePt;
    float4x4  matDelta;
    matDelta.RotationYawPitchRollR(Yaw, Pitch, Roll);
    m_vLookatPt = vDelta * matDelta + m_vEyePt;
  }

  b32 Camera::IsSimilarUpDir(CFloat3& vDir, float fEpsilon)
  {
    const float fDot = float3::dot(vDir, m_vUpVec);
    return fDot < -fEpsilon ||fDot > fEpsilon;
  }

  void Camera::Forward(float fSize, bool bSetLookat)
  {
    float3    vDirection = m_vLookatPt - m_vEyePt;
    //float3    vNormal;

    vDirection.normalize();
    vDirection = vDirection * fSize;

    m_vEyePt += vDirection;
    if(bSetLookat != false)
      m_vLookatPt += vDirection;
  }

  void Camera::Translation(float xOffset, float yOffset)
  {

    float3  zaxis, xaxis, yaxis;
    zaxis = m_vLookatPt - m_vEyePt;
    zaxis.normalize();

    xaxis = float3::cross(m_vUpVec, zaxis);
    xaxis.normalize();
    //D3DXVec3Normalize(&xaxis, &xaxis);
    yaxis = float3::cross(zaxis, xaxis);
    yaxis.normalize();

    xaxis *= -xOffset;
    yaxis *= yOffset;

    m_vEyePt = m_vEyePt + xaxis + yaxis;
    m_vLookatPt = m_vLookatPt + xaxis + yaxis;
  }

  void Camera::UpdateMat()
  {
    m_vFront = float3::normalize(m_vLookatPt - m_vEyePt);
    m_vRight = float3::normalize(float3::cross(m_vUpVec, m_vFront));
    m_vTop   = float3::cross(m_vFront, m_vRight);

    m_vLeft = -m_vRight;

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
}
