// 全局头文件
#include <GrapX.h>

// 标准接口
#include "GrapX/GCamera.h"
#include "GrapX/GResource.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXKernel.h"

#include <3D/Camera.h>

// 私有头文件
//#include "GCamera.h"
namespace GrapX
{
  Camera::Camera()
  {
  }

  Camera::~Camera()
  {
  }

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT Camera::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }

  GXHRESULT Camera::Release()
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

  GXHRESULT Camera::GetContext(GCAMERACONETXT* pCamContext)
  {
    return GX_FAIL;
  }
} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
float3 GCamera_ScreenAligned::m_vTop = -float3::AxisY;
GCamera_ScreenAligned::GCamera_ScreenAligned(GrapX::CanvasCore* pCanvasCore)
  : m_pCanvasCore(pCanvasCore)
{
}

GCamera_ScreenAligned::~GCamera_ScreenAligned()
{
}

GCamera_ScreenAligned* GCamera_ScreenAligned::Create(GrapX::CanvasCore* pCanvasCore)
{
  GCamera_ScreenAligned* pCamScrAligned = new GCamera_ScreenAligned(pCanvasCore);
  pCamScrAligned->AddRef();
  return pCamScrAligned;
}

//CameraType GCamera_ScreenAligned::GetType() const
//{
//  return CAM_SCRALIGNED;
//}

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

GrapX::Camera& GCamera_ScreenAligned::Rotate(const float3& axis, float radians, enum clstd::ESpace space)
{
  return *this;
}

GrapX::Camera& GCamera_ScreenAligned::RotateAround(const float3& point, const float3& axis, float radians)
{
  return *this;
}

//void GCamera_ScreenAligned::DeltaPitch(float fDelta)
//{
//}
//
//void GCamera_ScreenAligned::DeltaYaw(float fDelta)
//{
//}
//
//void GCamera_ScreenAligned::DeltaRoll(float fDelta)
//{
//}
//
//void GCamera_ScreenAligned::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
//{
//}

GrapX::Camera& GCamera_ScreenAligned::Translate(const float3& vOffset, clstd::ESpace space)
{
  return *this;
}

GrapX::Camera& GCamera_ScreenAligned::SetPos(const float3& vPos)
{
  return *this;
}

//void GCamera_ScreenAligned::SetPosFront(const float3& vPos, const float3& vFront)
//{
//}

const float3& GCamera_ScreenAligned::GetPos() const
{
  return float3::Origin;
}

GrapX::Camera& GCamera_ScreenAligned::SetRotation(const float3x3& mat)
{
  return *this;
}

GrapX::Camera& GCamera_ScreenAligned::SetRotation(const float3& vRight, const float3& vTop, const float3& vFront)
{
  return *this;
}

GrapX::Camera& GCamera_ScreenAligned::SetRotation(const clstd::_quaternion& quat)
{
  return *this;
}

const float3& GCamera_ScreenAligned::GetUp() const
{
  return m_vTop;
}

const float3& GCamera_ScreenAligned::GetTop() const
{
  return m_vTop;
}

const float3& GCamera_ScreenAligned::GetRight() const
{
  return float3::AxisX;
}

const float3& GCamera_ScreenAligned::GetFront() const
{
  return float3::AxisZ;
}

float GCamera_ScreenAligned::GetFov() const
{
  return 0.0f;
}

float GCamera_ScreenAligned::SetFov( float fFov )
{
  return 0.0f;
}

const float4x4& GCamera_ScreenAligned::GetViewMatrix() const
{
  return float4x4::Identity;
}

void GCamera_ScreenAligned::SetViewMatrix(const float4x4& matView)
{
}

//CFloat3& GCamera_ScreenAligned::GetDir() const
//{
//  return float3::AxisZ;
//}
//////////////////////////////////////////////////////////////////////////
#if 0
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

const float3& GCamera_Trackball::GetLookAt() const
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

CameraType GCamera_Trackball::GetType() const
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

const float3& GCamera_Trackball::GetPos() const
{
  return m_vEyePt;
}

const float3& GCamera_Trackball::GetUp() const
{
  return m_vUpVec;
}

const float3& GCamera_Trackball::GetTop() const
{
  return m_vTop;
}
const float3& GCamera_Trackball::GetRight() const
{
  return m_vRight;
}

const float3& GCamera_Trackball::GetFront() const
{
  return m_vFront;
}

//CFloat3& GCamera_Trackball::GetDir() const
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

float GCamera_Trackball::GetFov() const
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

const float4x4& GCamera_Trackball::GetViewMatrix() const
{
  return m_matView;
}

void GCamera_Trackball::SetViewMatrix(const float4x4& matView)
{
  clstd::Camera::SetViewMatrix(matView);
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

CameraType GCamera_FirstPerson::GetType() const
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

const float3& GCamera_FirstPerson::GetPos() const
{
  return m_vEyePt;
}

const float3& GCamera_FirstPerson::GetUp() const
{
  return m_vUpVec;
}

const float3& GCamera_FirstPerson::GetTop() const
{
  return m_vTop;
}
const float3& GCamera_FirstPerson::GetRight() const
{
  return m_vRight;
}

const float3& GCamera_FirstPerson::GetFront() const
{
  return m_vFront;
}

float GCamera_FirstPerson::GetFov() const
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

const float4x4& GCamera_FirstPerson::GetViewMatrix() const
{
  return m_matView;
}

void GCamera_FirstPerson::SetViewMatrix(const float4x4& matView)
{
  clstd::Camera::SetViewMatrix(matView);
}

//CFloat3& GCamera_FirstPerson::GetDir() const
//{
//
//}
#endif

namespace GrapX
{
  namespace Implement
  {
    class CameraImpl : public GrapX::Camera, public clstd::Camera
    {
    public:
      CameraImpl();
      GXBOOL Initialize(const float fAspect, const float fovy, const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar);
      GXBOOL InitializeOrthoGraphic(float w, float h, const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar);

      GXHRESULT GetContext (GCAMERACONETXT* pCamContext) override;
      GrapX::Camera&      Translate         (const float3& vOffset, clstd::ESpace space) override;
      GrapX::Camera&      SetPos            (const float3& vPos) override;
      //void          SetPosFront       (const float3& vPos, const float3& vFront) override;
      GrapX::Camera&      Rotate            (const float3& axis, float radians, clstd::ESpace space) override;
      GrapX::Camera&      RotateAround      (const float3& point, const float3& axis, float radians) override;
      CFloat3&            GetPos            () const override;
      //CFloat3&      GetUp             () const override; // 初始化时的向上的方向
      GrapX::Camera&      SetRotation       (const float3& vRight, const float3& vTop, const float3& vFront);
      GrapX::Camera&      SetRotation       (const float3x3& mat);
      GrapX::Camera&      SetRotation       (const clstd::_quaternion& quat);
      CFloat3&      GetTop            () const override; // 摄像机的顶方向,不是Up,俯仰角改变的话这个会改变
      CFloat3&      GetRight          () const override;
      CFloat3&      GetFront          () const override;
      float         GetFov            () const override;
      float         SetFov            (float fFov) override;
      const float4x4& GetViewMatrix   () const override;
      void          SetViewMatrix     (const float4x4& matView) override;
    };

    CameraImpl::CameraImpl()
    {
    }

    //GXHRESULT CameraImpl::Create(CameraImpl** ppCamera)
    //{
    //  CameraImpl* pCamera = new CameraImpl;
    //  pCamera->AddRef();

    //  *ppCamera = pCamera;
    //  return GX_OK;
    //}

    GXBOOL CameraImpl::Initialize(float fAspect, float fovy, const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar)
    {
      if(!clstd::Camera::InitializePerspectiveLH(vEye, vLookAt, vUp, fNear, fFar, fovy, fAspect)) {
        return FALSE;
      }
      return TRUE;
    }

    GXBOOL CameraImpl::InitializeOrthoGraphic(float w, float h, const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar)
    {
      if(!clstd::Camera::InitializeOrthoLH(vEye, vLookAt, vUp, fNear, fFar, w, h)) {
        return FALSE;
      }
      return TRUE;
    }


    //CameraType CameraImpl::GetType() const
    //{
    //  return CAM_FIRSTPERSON;
    //}

    GXHRESULT CameraImpl::GetContext(GCAMERACONETXT* pCamContext)
    {
      pCamContext->matWorld = float4x4::Identity;
      pCamContext->matView = m_matView;
      pCamContext->matProjection = m_matProj;
      return GX_OK;
    }

    //void CameraImpl::DeltaPitch(float fDelta)
    //{
    //  clstd::Camera::DeltaPitch(fDelta);
    //  UpdateMat();
    //}

    //void CameraImpl::DeltaYaw(float fDelta)
    //{
    //  clstd::Camera::DeltaYaw(fDelta);
    //  UpdateMat();
    //}

    //void CameraImpl::DeltaRoll(float fDelta)
    //{
    //  clstd::Camera::DeltaRoll(fDelta);
    //  UpdateMat();
    //}

    //void CameraImpl::DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll)
    //{
    //  clstd::Camera::DeltaYawPitchRoll(Yaw, Pitch, Roll);
    //  UpdateMat();
    //}

    GrapX::Camera& CameraImpl::Translate(const float3& vOffset, clstd::ESpace space)
    {
      //float3 v3Offset = -m_vRight * vOffset.x + m_vTop * vOffset.y;
      //m_vLookatPt += v3Offset;
      //m_vEyePt += v3Offset;
      //UpdateMat();
      clstd::Camera::Translate(vOffset, space);
      return *this;
    }

    GrapX::Camera& CameraImpl::SetPos(const float3& vPos)
    {
      //m_vLookatPt += (vPos - m_vEyePt);
      m_vEyePt = vPos;
      clstd::Camera::OnEyePositionChanged();
      return *this;
      //UpdateMat();
    }

    //void CameraImpl::SetPosFront(const float3& vPos, const float3& vFront)
    //{
    //  m_vEyePt = vPos;
    //  m_vLookatPt = vPos + vFront * 1.0f;
    //  UpdateMat();
    //}

    GrapX::Camera& CameraImpl::Rotate(const float3& axis, float radians, clstd::ESpace space)
    {
      clstd::Camera::Rotate(axis, radians, space);
      return *this;
    }

    GrapX::Camera& CameraImpl::RotateAround(const float3& point, const float3& axis, float radians)
    {
      float3x3 mat, matT;
      float3x3 matView3(m_matView);
      mat.RotationAxis(axis, -radians);
      matT = mat;
      matT.transpose();
      m_vEyePt = (m_vEyePt - point) * matT + point;
      mat = mat * matView3;
      clstd::Camera::SetRotation(mat);
      return *this;
    }

    const float3& CameraImpl::GetPos() const
    {
      return m_vEyePt;
    }

    GrapX::Camera& CameraImpl::SetRotation(const float3& vRight, const float3& vTop, const float3& vFront)
    {
      clstd::Camera::SetRotation(vRight, vTop, vFront);
      return *this;
    }

    GrapX::Camera& CameraImpl::SetRotation(const float3x3& mat)
    {
      clstd::Camera::SetRotation(mat);
      return *this;
    }

    GrapX::Camera& CameraImpl::SetRotation(const clstd::_quaternion& quat)
    {
      clstd::Camera::SetRotation(quat);
      return *this;
    }

    //const float3& CameraImpl::GetUp() const
    //{
    //  return m_vUpVec;
    //}

    const float3& CameraImpl::GetTop() const
    {
      return m_vTop;
    }
    const float3& CameraImpl::GetRight() const
    {
      return m_vRight;
    }

    const float3& CameraImpl::GetFront() const
    {
      return m_vFront;
    }

    float CameraImpl::GetFov() const
    {
      return m_fovy;
    }

    float CameraImpl::SetFov(float fFov)
    {
      float fPrev = m_fovy;
      m_fovy = fFov;
      //UpdateMat();
      UpdateProjectionMatrix();
      return fPrev;
    }

    const float4x4& CameraImpl::GetViewMatrix() const
    {
      return m_matView;
    }

    void CameraImpl::SetViewMatrix(const float4x4& matView)
    {
      clstd::Camera::SetViewMatrix(matView);
    }
  }
}

BOOL GrapX::Camera::Create(GrapX::Camera** ppCamera, float fAspect, float fovy, const float3& vEye, const float3& vLookAt, const float3& vUp /*= float3::AxisY*/, float fNear /*= 1.0f*/, float fFar /*= 1000.0f*/)
{
  GrapX::Implement::CameraImpl* pCamera = new GrapX::Implement::CameraImpl;
  pCamera->AddRef();

  if(_CL_NOT_(pCamera->Initialize(fAspect, fovy, vEye, vLookAt, vUp, fNear, fFar))) {
    pCamera->Release();
    return FALSE;
  }

  *ppCamera = pCamera;
  return GX_OK;
}

BOOL GrapX::Camera::CreateOrthographic(Camera** ppCamera, float w, float h, const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar)
{
  GrapX::Implement::CameraImpl* pCamera = new GrapX::Implement::CameraImpl;
  pCamera->AddRef();

  if(_CL_NOT_(pCamera->InitializeOrthoGraphic(w, h, vEye, vLookAt, vUp, fNear, fFar))) {
    pCamera->Release();
    return FALSE;
  }

  *ppCamera = pCamera;
  return GX_OK;
}