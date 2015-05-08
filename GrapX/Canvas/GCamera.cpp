// ȫ��ͷ�ļ�
#include <GrapX.H>

// ��׼�ӿ�
#include "GrapX/GCamera.H"
#include "GrapX/GResource.H"
#include "GrapX/GXCanvas.H"
#include "GrapX/GXKernel.H"

// ˽��ͷ�ļ�
//#include "GCamera.h"

GCamera::GCamera()
{
}

GCamera::~GCamera()
{
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GCamera::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GCamera::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT GCamera::GetContext(GCAMERACONETXT* pCamContext)
{
  return GX_FAIL;
}

//////////////////////////////////////////////////////////////////////////
float3 GCamera_ScreenAligned::m_vTop = -float3::AxisY;
GCamera_ScreenAligned::GCamera_ScreenAligned(GXCanvasCore* pCanvasCore)
  : m_pCanvasCore(pCanvasCore)
{
}

GCamera_ScreenAligned::~GCamera_ScreenAligned()
{
}

GCamera_ScreenAligned* GCamera_ScreenAligned::Create(GXCanvasCore* pCanvasCore)
{
  GCamera_ScreenAligned* pCamScrAligned = new GCamera_ScreenAligned(pCanvasCore);
  pCamScrAligned->AddRef();
  return pCamScrAligned;
}

CameraType GCamera_ScreenAligned::GetType() GXCONST
{
  return CAM_SCRALIGNED;
}

GXHRESULT GCamera_ScreenAligned::GetContext(GCAMERACONETXT* pCamContext)
{
  GXSIZE size;
  pCamContext->dwMask = GCC_WORLD;
  float4x4& matWorld = pCamContext->matWorld;
  m_pCanvasCore->GetTargetDimension(&size);

  matWorld.identity();
  matWorld._11 =  2.0f / (GXFLOAT)size.cx;
  matWorld._22 = -2.0f / (GXFLOAT)size.cy;
  matWorld._41 = -1.0f;
  matWorld._42 =  1.0f;
  return GX_OK;
}

void GCamera_ScreenAligned::DeltaPitch(float fDelta)
{
}

void GCamera_ScreenAligned::DeltaYaw(float fDelta)
{
}

void GCamera_ScreenAligned::DeltaRoll(float fDelta)
{
}

void GCamera_ScreenAligned::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
{
}

void GCamera_ScreenAligned::Translation(const float2& vOffset)
{
}

void GCamera_ScreenAligned::SetPos(const float3& vPos)
{
}

void GCamera_ScreenAligned::SetPosFront(const float3& vPos, const float3& vFront)
{
}

const float3& GCamera_ScreenAligned::GetPos() GXCONST
{
  return float3::Origin;
}

const float3& GCamera_ScreenAligned::GetUp() GXCONST
{
  return m_vTop;
}

const float3& GCamera_ScreenAligned::GetTop() GXCONST
{
  return m_vTop;
}

const float3& GCamera_ScreenAligned::GetRight() GXCONST
{
  return float3::AxisX;
}

const float3& GCamera_ScreenAligned::GetFront() GXCONST
{
  return float3::AxisZ;
}

float GCamera_ScreenAligned::GetFov() GXCONST
{
  return 0.0f;
}

float GCamera_ScreenAligned::SetFov( float fFov )
{
  return 0.0f;
}

//CFloat3& GCamera_ScreenAligned::GetDir() GXCONST
//{
//  return float3::AxisZ;
//}
//////////////////////////////////////////////////////////////////////////
GCamera_Trackball::GCamera_Trackball()
  : GCamera()
{
}

GXHRESULT GCamera_Trackball::Initialize(const float3& vEye, const float3& vLookAt, const float fAspect, const float fovy, const float3& vUp, float fNear, float fFar)
{
  if( ! clstd::Camera::InitializePerspectiveLH(vEye, vLookAt, vUp, fNear, fFar, fovy, fAspect)) {
    return GX_FAIL;
  }
  UpdateMat();
  return GX_OK;
}

GXHRESULT GCamera_Trackball::InitializeOrtho(const float3& vEye, const float3& vLookAt, const float w, const float h, const float3& vUp, float fNear, float fFar)
{
  if( ! clstd::Camera::InitializeOrthoLH(vEye, vLookAt, vUp, fNear, fFar, w, h)) {
    return GX_FAIL;
  }
  UpdateMat();
  return GX_OK;
}

const float3& GCamera_Trackball::GetLookAt() GXCONST
{
  return m_vLookatPt;
}

void GCamera_Trackball::SetLookAt(CFloat3& vLookAt)
{
  float3 vDelta = vLookAt - m_vLookatPt;
  m_vEyePt += vDelta;
  m_vLookatPt += vDelta;
  UpdateMat();
}

void GCamera_Trackball::SetLookAt(const float3& vEye, CFloat3& vLookAt)
{
  m_vEyePt = vEye;
  m_vLookatPt = vLookAt;
  UpdateMat();
}

CameraType GCamera_Trackball::GetType() GXCONST
{
  return CAM_TRACKBALL;
}

GXHRESULT GCamera_Trackball::GetContext(GCAMERACONETXT* pCamContext)
{
  pCamContext->matWorld = float4x4::Identity;
  pCamContext->matView = m_matView;
  pCamContext->matProjection = m_matProj;
  return GX_OK;
}

void GCamera_Trackball::DeltaPitch(float fDelta)
{
  //clstd::Camera::DeltaPitch(fDelta);
  float4x4 matTemp;
  float3 vDelta = m_vLookatPt - m_vEyePt;
  matTemp.RotationAxis(m_vRight, fDelta);
  vDelta *= matTemp;
  m_vEyePt = m_vLookatPt - vDelta;
  UpdateMat();
}

void GCamera_Trackball::DeltaYaw(float fDelta)
{
  //clstd::Camera::DeltaYaw(fDelta);
  float4x4 matTemp;
  float3 vDelta = m_vLookatPt - m_vEyePt;
  matTemp.RotationAxis(m_vTop, fDelta);
  vDelta *= matTemp;
  m_vEyePt = m_vLookatPt - vDelta;
  UpdateMat();
}

void GCamera_Trackball::DeltaRoll(float fDelta)
{
  ASSERT(0);
}

void GCamera_Trackball::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
{
  ASSERT(0);
}

void GCamera_Trackball::Translation(const float2& vOffset)
{
  float3 v3Offset = -m_vRight * vOffset.x + m_vTop * vOffset.y;
  m_vLookatPt += v3Offset;
  m_vEyePt += v3Offset;
  UpdateMat();
}

void GCamera_Trackball::SetPos(const float3& vPos)
{
  m_vEyePt = vPos;
  UpdateMat();
}

void GCamera_Trackball::SetPosFront(const float3& vPos, const float3& vFront)
{
  m_vEyePt = vPos;
  m_vLookatPt = vPos + vFront * 1.0f;
  UpdateMat();
}

const float3& GCamera_Trackball::GetPos() GXCONST
{
  return m_vEyePt;
}

const float3& GCamera_Trackball::GetUp() GXCONST
{
  return m_vUpVec;
}

const float3& GCamera_Trackball::GetTop() GXCONST
{
  return m_vTop;
}
const float3& GCamera_Trackball::GetRight() GXCONST
{
  return m_vRight;
}

const float3& GCamera_Trackball::GetFront() GXCONST
{
  return m_vFront;
}

//CFloat3& GCamera_Trackball::GetDir() GXCONST
//{
//
//}

GXHRESULT GCamera_Trackball::Create(GCamera_Trackball** ppCamera)
{
  GCamera_Trackball* pCamera = new GCamera_Trackball;
  pCamera->AddRef();

  *ppCamera = pCamera;
  return GX_OK;
}

float GCamera_Trackball::GetFov() GXCONST
{
  return m_fovy;
}

float GCamera_Trackball::SetFov( float fFov )
{
  float fPrev = m_fovy;
  m_fovy = fFov;
  UpdateMat();
  return fPrev;
}

////////////////////////////////////////////////////////////////////////////
GCamera_FirstPerson::GCamera_FirstPerson()
{
}

GXHRESULT GCamera_FirstPerson::Create(GCamera_FirstPerson** ppCamera)
{
  GCamera_FirstPerson* pCamera = new GCamera_FirstPerson;
  pCamera->AddRef();

  *ppCamera = pCamera;
  return GX_OK;
}

GXHRESULT GCamera_FirstPerson::Initialize(const float3& vEye, const float3& vLookAt, const float fAspect, const float fovy, const float3& vUp, float fNear, float fFar)
{
  if( ! clstd::Camera::InitializePerspectiveLH(vEye, vLookAt, vUp, fNear, fFar, fovy, fAspect)) {
    return GX_FAIL;
  }
  UpdateMat();
  return GX_OK;
}

CameraType GCamera_FirstPerson::GetType() GXCONST
{
  return CAM_FIRSTPERSON;
}

GXHRESULT GCamera_FirstPerson::GetContext(GCAMERACONETXT* pCamContext)
{
  pCamContext->matWorld = float4x4::Identity;
  pCamContext->matView = m_matView;
  pCamContext->matProjection = m_matProj;
  return GX_OK;
}

void GCamera_FirstPerson::DeltaPitch(float fDelta)
{
  clstd::Camera::DeltaPitch(fDelta);
  UpdateMat();
}

void GCamera_FirstPerson::DeltaYaw(float fDelta)
{
  clstd::Camera::DeltaYaw(fDelta);
  UpdateMat();
}

void GCamera_FirstPerson::DeltaRoll(float fDelta)
{
  clstd::Camera::DeltaRoll(fDelta);
  UpdateMat();
}

void GCamera_FirstPerson::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
{
  clstd::Camera::DeltaYawPitchRoll(Yaw, Pitch, Roll);
  UpdateMat();
}

void GCamera_FirstPerson::Translation(const float2& vOffset)
{
  float3 v3Offset = -m_vRight * vOffset.x + m_vTop * vOffset.y;
  m_vLookatPt += v3Offset;
  m_vEyePt += v3Offset;
  UpdateMat();
}

void GCamera_FirstPerson::SetPos(const float3& vPos)
{
  m_vLookatPt += (vPos - m_vEyePt);
  m_vEyePt = vPos;
  UpdateMat();
}

void GCamera_FirstPerson::SetPosFront(const float3& vPos, const float3& vFront)
{
  m_vEyePt = vPos;
  m_vLookatPt = vPos + vFront * 1.0f;
  UpdateMat();
}

const float3& GCamera_FirstPerson::GetPos() GXCONST
{
  return m_vEyePt;
}

const float3& GCamera_FirstPerson::GetUp() GXCONST
{
  return m_vUpVec;
}

const float3& GCamera_FirstPerson::GetTop() GXCONST
{
  return m_vTop;
}
const float3& GCamera_FirstPerson::GetRight() GXCONST
{
  return m_vRight;
}

const float3& GCamera_FirstPerson::GetFront() GXCONST
{
  return m_vFront;
}

float GCamera_FirstPerson::GetFov() GXCONST
{
  return m_fovy;
}

float GCamera_FirstPerson::SetFov( float fFov )
{
  float fPrev = m_fovy;
  m_fovy = fFov;
  UpdateMat();
  return fPrev;
}

//CFloat3& GCamera_FirstPerson::GetDir() GXCONST
//{
//
//}
