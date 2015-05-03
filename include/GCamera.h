#ifndef _G_CAMERA_H_
#define _G_CAMERA_H_

class GXCanvasCore;
#include <3D/Camera.h>
//
// GCAMERACONETXT ������
enum
{
  GCC_WORLD       = 0x00000001,
  GCC_VIEW        = 0x00000002,
  GCC_PROJECTION  = 0x00000004,

  GCC_WVP      = (GCC_WORLD | GCC_VIEW | GCC_PROJECTION),
};

struct GCAMERACONETXT
{
  GXDWORD   dwMask;         // In/Out �����ʾ����, ���ص���ʵ�ʼ����
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

  GXSTDINTERFACE(CameraType    GetType           () GXCONST);
  GXSTDINTERFACE(void          DeltaPitch        (float fDelta));   // ��Ļx��
  GXSTDINTERFACE(void          DeltaYaw          (float fDelta));   // ��Ļy��
  GXSTDINTERFACE(void          DeltaRoll         (float fDelta));   // ��Ļz��
  GXSTDINTERFACE(void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll));
  GXSTDINTERFACE(void          Translation       (const float2& vOffset));  // ��Ļ�ռ�ƽ��
  GXSTDINTERFACE(void          SetPos            (const float3& vPos));
  GXSTDINTERFACE(void          SetPosFront       (const float3& vPos, const float3& vFront));
  GXSTDINTERFACE(CFloat3&      GetPos            () GXCONST);
  GXSTDINTERFACE(CFloat3&      GetUp             () GXCONST); // ��ʼ��ʱ�����ϵķ���
  GXSTDINTERFACE(CFloat3&      GetTop            () GXCONST); // ������Ķ�����,����Up,�����Ǹı�Ļ������ı�
  GXSTDINTERFACE(CFloat3&      GetRight          () GXCONST);
  GXSTDINTERFACE(CFloat3&      GetFront          () GXCONST);
  GXSTDINTERFACE(float         GetFov            () GXCONST);
  GXSTDINTERFACE(float         SetFov            (float fFov));
  //GXSTDINTERFACE(CFloat3&      GetDir            () GXCONST);
};

class GXDLL GCamera_ScreenAligned : public GCamera
{
protected:
  GXCanvasCore* m_pCanvasCore;
  static float3  m_vTop;
public:
  GCamera_ScreenAligned(GXCanvasCore* pCanvasCore);
  ~GCamera_ScreenAligned();

  static GCamera_ScreenAligned* Create(GXCanvasCore* pCanvasCore);
public:
  virtual CameraType    GetType           () GXCONST;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // ��Ļx��
  virtual void          DeltaYaw          (float fDelta);   // ��Ļy��
  virtual void          DeltaRoll         (float fDelta);   // ��Ļz��
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () GXCONST;
  virtual CFloat3&      GetUp             () GXCONST;
  virtual CFloat3&      GetTop            () GXCONST; // ������Ķ�����,����Up
  virtual CFloat3&      GetRight          () GXCONST;
  virtual CFloat3&      GetFront          () GXCONST;
  virtual float         GetFov            () GXCONST;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () GXCONST;
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
  const float3& GetLookAt   () GXCONST;
  void SetLookAt(CFloat3& vLookAt);
  void SetLookAt(const float3& vEye, CFloat3& vLookAt);
public:
  virtual CameraType    GetType           () GXCONST;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // ��Ļx��
  virtual void          DeltaYaw          (float fDelta);   // ��Ļy��
  virtual void          DeltaRoll         (float fDelta);   // ��Ļz��
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () GXCONST;
  virtual CFloat3&      GetUp             () GXCONST;
  virtual CFloat3&      GetTop            () GXCONST; // ������Ķ�����,����Up
  virtual CFloat3&      GetRight          () GXCONST;
  virtual CFloat3&      GetFront          () GXCONST;
  virtual float         GetFov            () GXCONST;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () GXCONST;
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
  virtual CameraType    GetType           () GXCONST;
  virtual GXHRESULT     GetContext        (GCAMERACONETXT* pCamContext);
  virtual void          DeltaPitch        (float fDelta);   // ��Ļx��
  virtual void          DeltaYaw          (float fDelta);   // ��Ļy��
  virtual void          DeltaRoll         (float fDelta);   // ��Ļz��
  virtual void          DeltaYawPitchRoll (const float Yaw, const float Pitch, const float Roll);
  virtual void          Translation       (const float2& vOffset);
  virtual void          SetPos            (const float3& vPos);
  virtual void          SetPosFront       (const float3& vPos, const float3& vFront);
  virtual CFloat3&      GetPos            () GXCONST;
  virtual CFloat3&      GetUp             () GXCONST;
  virtual CFloat3&      GetTop            () GXCONST; // ������Ķ�����,����Up
  virtual CFloat3&      GetRight          () GXCONST;
  virtual CFloat3&      GetFront          () GXCONST;
  virtual float         GetFov            () GXCONST;
  virtual float         SetFov            (float fFov);
  //virtual CFloat3&      GetDir            () GXCONST;
};

#endif // _G_CAMERA_H_