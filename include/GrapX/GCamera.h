#ifndef _G_CAMERA_H_
#define _G_CAMERA_H_

namespace GrapX
{
  class CanvasCore;
} // namespace GrapX

#include <3D/Camera.h>
//
// GCAMERACONETXT 的掩码
enum
{
  GCC_WORLD       = 0x00000001,
  GCC_VIEW        = 0x00000002,
  GCC_PROJECTION  = 0x00000004,

  GCC_WVP      = (GCC_WORLD | GCC_VIEW | GCC_PROJECTION),
};

struct GCAMERACONETXT
{
  GXDWORD   dwMask;         // In/Out 输入表示需求, 返回的是实际计算的
  float4x4  matWorld;       // Out
  float4x4  matView;        // Out
  float4x4  matProjection;  // Out
};

enum CameraType
{
  CAM_SCRALIGNED,
  CAM_TRACKBALL,
  CAM_FIRSTPERSON,
};

class GXDLL GCamera : public GUnknown
{
protected:
public:
  GCamera();
  virtual ~GCamera();
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef     ();
  virtual GXHRESULT Release    ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXHRESULT GetContext (GCAMERACONETXT* pCamContext);

  GXSTDINTERFACE(CameraType    GetType           () const);
  GXSTDINTERFACE(void          DeltaPitch        (float fDelta));   // 屏幕x轴
  GXSTDINTERFACE(void          DeltaYaw          (float fDelta));   // 屏幕y轴
  GXSTDINTERFACE(void          DeltaRoll         (float fDelta));   // 屏幕z轴
  GXSTDINTERFACE(void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll));
  GXSTDINTERFACE(void          Translation       (const float2& vOffset));  // 屏幕空间平移
  GXSTDINTERFACE(void          SetPos            (const float3& vPos));
  GXSTDINTERFACE(void          SetPosFront       (const float3& vPos, const float3& vFront));
  GXSTDINTERFACE(CFloat3&      GetPos            () const);
  GXSTDINTERFACE(CFloat3&      GetUp             () const); // 初始化时的向上的方向
  GXSTDINTERFACE(CFloat3&      GetTop            () const); // 摄像机的顶方向,不是Up,俯仰角改变的话这个会改变
  GXSTDINTERFACE(CFloat3&      GetRight          () const);
  GXSTDINTERFACE(CFloat3&      GetFront          () const);
  GXSTDINTERFACE(float         GetFov            () const);
  GXSTDINTERFACE(float         SetFov            (float fFov));
  //GXSTDINTERFACE(CFloat3&      GetDir            () const);
};

class GXDLL GCamera_ScreenAligned : public GCamera
{
protected:
  GrapX::CanvasCore* m_pCanvasCore;
  static float3  m_vTop;
public:
  GCamera_ScreenAligned(GrapX::CanvasCore* pCanvasCore);
  ~GCamera_ScreenAligned();

  static GCamera_ScreenAligned* Create(GrapX::CanvasCore* pCanvasCore);
public:
  virtual CameraType    GetType           () const;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // 屏幕x轴
  virtual void          DeltaYaw          (float fDelta);   // 屏幕y轴
  virtual void          DeltaRoll         (float fDelta);   // 屏幕z轴
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () const;
  virtual CFloat3&      GetUp             () const;
  virtual CFloat3&      GetTop            () const; // 摄像机的顶方向,不是Up
  virtual CFloat3&      GetRight          () const;
  virtual CFloat3&      GetFront          () const;
  virtual float         GetFov            () const;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () const;
};


class GXDLL GCamera_Trackball : public GCamera, clstd::Camera
{
protected:
  GCamera_Trackball();
public:
  static GXHRESULT Create(GCamera_Trackball** ppCamera);
public:
  GXHRESULT     Initialize       (const float3& vEye, const float3& vLookAt, const float fAspect, const float fovy, const float3& vUp = float3::AxisY, float fNear = 1.0f, float fFar = 1000.0f);
  GXHRESULT     InitializeOrtho  (const float3& vEye, const float3& vLookAt, const float w, const float h, const float3& vUp = float3::AxisY, float fNear = 1.0f, float fFar = 1000.0f);
  const float3& GetLookAt   () const;
  void SetLookAt(CFloat3& vLookAt);
  void SetLookAt(const float3& vEye, CFloat3& vLookAt);
public:
  virtual CameraType    GetType           () const;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // 屏幕x轴
  virtual void          DeltaYaw          (float fDelta);   // 屏幕y轴
  virtual void          DeltaRoll         (float fDelta);   // 屏幕z轴
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () const;
  virtual CFloat3&      GetUp             () const;
  virtual CFloat3&      GetTop            () const; // 摄像机的顶方向,不是Up
  virtual CFloat3&      GetRight          () const;
  virtual CFloat3&      GetFront          () const;
  virtual float         GetFov            () const;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () const;
};

class GXDLL GCamera_FirstPerson : public GCamera, clstd::Camera
{
protected:
  GCamera_FirstPerson();
public:
  static GXHRESULT Create(GCamera_FirstPerson** ppCamera);
public:
  GXHRESULT Initialize(const float3& vEye, const float3& vLookAt, const float fAspect, const float fovy, const float3& vUp = float3::AxisY, float fNear = 1.0f, float fFar = 1000.0f);
public:
  virtual CameraType    GetType           () const;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // 屏幕x轴
  virtual void          DeltaYaw          (float fDelta);   // 屏幕y轴
  virtual void          DeltaRoll         (float fDelta);   // 屏幕z轴
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () const;
  virtual CFloat3&      GetUp             () const;
  virtual CFloat3&      GetTop            () const; // 摄像机的顶方向,不是Up
  virtual CFloat3&      GetRight          () const;
  virtual CFloat3&      GetFront          () const;
  virtual float         GetFov            () const;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () const;
};

#endif // _G_CAMERA_H_