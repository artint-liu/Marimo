#ifndef _CLSTD_TRANSFORM_H_
#define _CLSTD_TRANSFORM_H_

namespace clstd
{
  enum RotationOrder
  { 
    RotationOrder_EulerXYZ,
    RotationOrder_EulerXZY,
    RotationOrder_EulerYXZ,
    RotationOrder_EulerYZX,
    RotationOrder_EulerZXY,
    RotationOrder_EulerZYX,
  };
//#define _CLTRANSFORM_TEMPL template<class _MatT, _MatT ParentTransform>
//#define _CLTRANSFORM_IMPL Transform<_MatT, ParentTransform>

  enum ESpace
  {
    S_Self,
    S_World,
  };

  //_CLTRANSFORM_TEMPL
  struct TRANSFORM
  {
    float3      scaling;
    quaternion  rotation;
    float3      translation;
    //float3      rotationEuler;     // 角度值
    //quaternion  rotationQuaternion;
    float4x4    GlobalMatrix; // Global = Coordinate * Local
    //float3      GlobalScaling;
    
    TRANSFORM();

    // 下面这些接口设置分量后会丢失原有TRANSFORM所在空间，重新建立在笛卡尔坐标空间中
    CFloat4x4&  Set                 (CFloat3& vScaling, CQuaternion& quater, CFloat3& vTranslation);
    CFloat4x4&  Set                 (CFloat3& vScaling, CFloat3& vEuler, CFloat3& vTranslation);
    b32         SetTransform        (CFloat4x4& mat);
    void        SetPosition         (CFloat3& vPos);  // As top transform
    void        SetScaling          (CFloat3& vScaling);
    void        SetRotationA        (CFloat3& vEuler);  // Angle
    void        SetRotationR        (CFloat3& vEuler);  // Radian
    void        SetRotation         (CQuaternion& quater);
    void        RotateA             (CFloat3& vEuler);
    void        RotateR             (CFloat3& vEuler);
    void        Rotate              (CQuaternion& quater);
    b32         SetDirection        (CFloat3& vDir, CFloat3& vUp);
    CFloat4x4&  UpdateAbsoluteMatrix();

    CFloat4x4&  Set                 (CFloat4x4& Coordinate, CFloat3& vScaling, CQuaternion& quater, CFloat3& vTranslation);
    CFloat4x4&  Set                 (CFloat4x4& Coordinate, CFloat3& vScaling, CFloat3& vEuler, CFloat3& vTranslation);
    b32         SetTransform        (CFloat4x4& Coordinate, CFloat4x4& mat, ESpace eSpace);
    void        SetPosition         (CFloat4x4& Coordinate, CFloat3& vPos, ESpace eSpace);
    void        SetScaling          (CFloat4x4& Coordinate, CFloat3& vScaling);
    void        SetRotationA        (CFloat4x4& Coordinate, CFloat3& vEuler);  // Angle
    void        SetRotationR        (CFloat4x4& Coordinate, CFloat3& vEuler);  // Radian
    void        SetRotation         (CFloat4x4& Coordinate, CQuaternion& quater);
    void        RotateA             (CFloat4x4& Coordinate, CFloat3& vEuler);
    void        RotateR             (CFloat4x4& Coordinate, CFloat3& vEuler);
    void        Rotate              (CFloat4x4& Coordinate, CQuaternion& quater);
    b32         SetDirection        (CFloat4x4& Coordinate, CFloat3& vDir, CFloat3& vUp);
    // void   Forward                   (float fDistance);
    // void   Translate                 (CFloat3& translation, Space relativeTo = Space.Self);  // 相对移动
    // void   RotateAround              (CFloat3& point, CFloat3& axis, float angle);
    // void   LookAt                    (CFloat3& worldPosition, CFloat3& worldUp);
    // float3 TransformDirection        (CFloat3& direction); // Transforms direction from local space to world space.
    // float3 InverseTransformDirection (CFloat3& direction); // Transforms a direction from world space to local space.
    // float3 TransformPoint            (CFloat3& position);  // Transforms position from local space to world space.
    // float3 InverseTransformPoint     (CFloat3& position);  // Transforms position from world space to local space.
    float4x4    ToRelativeMatrix        () CLCONST;
    CFloat4x4&  UpdateAbsoluteMatrix    (CFloat4x4& Coordinate);
    euler       GetEuler                (RotationOrder eOrder) const;

    inline operator const float4x4& () const;
    //CFloat4x4&  CalcAbsoluteMatrixAsTop ();
#ifdef _DEBUG
    b32 DbgCheckGlobalScalingChanged();
#endif // #ifdef _DEBUG
  };

  TRANSFORM::operator const float4x4& () const
  {
    return GlobalMatrix;
  }
  //_CLTRANSFORM_TEMPL 
  //_CLTRANSFORM_IMPL::Transform()
  //  : translation(0.0f)
  //  , rotationEuler(0.0f)
  //  , rotationQuaternion(0.0f)
  //  , scaling(1.0f)
  //{
  //}

  //_CLTRANSFORM_TEMPL 
  //void _CLTRANSFORM_IMPL::SetPosition(CFloat3& vPos)
  //{
  //  translation = vPos;
  //}

  //_CLTRANSFORM_TEMPL 
  //void _CLTRANSFORM_IMPL::SetScaling(CFloat3& vScaling)
  //{
  //  scaling = vScaling;
  //}

  //_CLTRANSFORM_TEMPL 
  //void _CLTRANSFORM_IMPL::SetRotation(CFloat3& vEuler)
  //{
  //  rotationEuler = vEuler;
  //  // TODO: Update quaternion
  //}

  //_CLTRANSFORM_TEMPL 
  //void _CLTRANSFORM_IMPL::SetRotation(CQuaternion& quater)
  //{
  //  CLBREAK;
  //  // FIXME!!
  //}

  //_CLTRANSFORM_TEMPL 
  //float4x4 _CLTRANSFORM_IMPL::ToMatrix()
  //{
  //  float4x4 s, r, t;
  //  s.Scale(scaling);
  //  r.RotationYawPitchRollA(rotationEuler.y, rotationEuler.x, rotationEuler.z);
  //  t.Translate(translation.x, translation.y, translation.z);
  //  return (s * r * t);
  //}

} // namespace clstd

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // #ifndef _CLSTD_TRANSFORM_H_