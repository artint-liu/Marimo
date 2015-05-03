#ifndef _CAMERA_H_
#define _CAMERA_H_

namespace clstd
{
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
    void InitializeCommon();
  public:
    float3    m_vEyePt;
    float3    m_vLookatPt;

    float3    m_vUpVec;   // �����������ָʾ���������,ֻ�ǲο�����
    float3    m_vLeft;    // ������ռ���������
    float3    m_vRight;   // ������ռ���������
    float3    m_vTop;     // ������ռ����ϵ�����, �����������ǰ��������ֱ������ϵ
    float3    m_vFront;   // ������ռ���ǰ������

    float4x4  m_matView;
    float4x4  m_matProj;
    float4x4  m_matViewProj;

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

    void RotateEye  (float fHAngle, float fVAngle);   // ��ת�۲�λ��
    void Forward    (float fSize, bool bSetLookat);
    void Translation(float xOffset, float yOffset);

    void DeltaPitch (float fDelta);   // ��Ļx��
    void DeltaYaw   (float fDelta);   // ��Ļy��
    void DeltaRoll  (float fDelta);   // ��Ļz��

    void DeltaYawPitchRoll(const float Yaw, const float Pitch, const float Roll);
    b32  IsSimilarUpDir(CFloat3& vDir, float fEpsilon);  // ���Front��Up����, ��������򷵻�true

    void UpdateMat();

    Camera();
    ~Camera();
  };
}

typedef clstd::Camera    Camera;

#endif // end of _CAMERA_H_