#ifndef _CAMERA_H_
#define _CAMERA_H_

namespace clstd
{
  // 左手坐标系
  class Camera
  {
  private:
    enum ProjectionType
    {
      Unknown,
      PerspectiveLH,
      OrthoLH,
    };
  protected:
    //void InitializeCommon();
  public:
    float3    m_vEyePt;
    //float3    m_vLookatPt;

    //float3    m_vUpVec;   // 这个向量用于指示摄像机方向,只是参考方向
    //float3    m_vLeft;    // 摄像机空间向左向量
    float3    m_vRight;   // 摄像机空间向右向量
    float3    m_vTop;     // 摄像机空间向上的向量, 这个向量与左前向量构成直角坐标系
    float3    m_vFront;   // 摄像机空间向前的向量

    float4x4  m_matView;
    float4x4  m_matProj;
    //float4x4  m_matViewProj;

    union
    {
      struct {
        float     m_fWidth;
        float     m_fHeight;
      };

      struct {
        float     m_fAspect;
        float     m_fovy;
      };
    };

    float     m_fNear;
    float     m_fFar;

    ProjectionType  m_eProjType;

    bool InitializePerspectiveLH  (const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar, float fovy, float fAspect);
    bool InitializeOrthoLH        (const float3& vEye, const float3& vLookAt, const float3& vUp, float fNear, float fFar, float w, float h);
    Camera& Translate (const float3& translation, enum ESpace space);
    Camera& Rotate    (const float3& axis, float radians, enum ESpace space);
    void LookAt(const float3& vWorldPos, const float3& up = float3::AxisY);
    //void RotateEye  (float fHAngle, float fVAngle);   // 旋转观察位置
    //void Forward    (float fSize, bool bSetLookat);
    //void Translation(float xOffset, float yOffset);

    void SetViewMatrix(const float4x4& matView);

    //void DeltaPitch (float fDelta);   // 屏幕x轴
    //void DeltaYaw   (float fDelta);   // 屏幕y轴
    //void DeltaRoll  (float fDelta);   // 屏幕z轴

    //void DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll);
    //b32  IsSimilarUpDir(CFloat3& vDir, float fEpsilon);  // 检查Front与Up方向, 如果近似则返回true    

    //void _UpdateMat();
    void OnEyePositionChanged(); // 摄像机位置发生变化时调用
    void UpdateViewMatrix(); // 摄像机位置发生变化时调用
    void UpdateProjectionMatrix();


    Camera();
    ~Camera();
  };
}

typedef clstd::Camera    Camera;

#endif // end of _CAMERA_H_