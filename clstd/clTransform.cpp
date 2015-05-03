#include "clstd.h"
#include "clTransform.h"
#ifdef _DEBUG
#include "clString.H"
#include "clUtility.H"
#define DBGCHECKGLOBALSCALINGCHANGED ASSERT(DbgCheckGlobalScalingChanged());
#else
#define DBGCHECKGLOBALSCALINGCHANGED
#endif // #ifdef _DEBUG
namespace clstd
{
  inline CFloat4x4& CalcAbsoluteMatrix(TRANSFORM* T, CFloat4x4& Coordinate)
  {
    T->GlobalMatrix = T->ToRelativeMatrix() * Coordinate;
    return T->GlobalMatrix;
  }

  inline CFloat4x4& CalcAbsoluteMatrix(TRANSFORM* T)
  {
    T->GlobalMatrix = T->ToRelativeMatrix();
    return T->GlobalMatrix;
  }


  TRANSFORM::TRANSFORM()
    : translation(0.0f)
    //, rotationEuler(0.0f)
    //, rotationQuaternion(0.0f)
    , rotation(0.0f,0.0f,0.0f,1.0f)
    , scaling(1.0f)
    //, GlobalScaling(1.0f)
  {
    GlobalMatrix.identity();
  }

  CFloat4x4& TRANSFORM::Set(CFloat3& vScaling, CQuaternion& quater, CFloat3& vTranslation)
  {
    scaling       = vScaling;
    rotation      = quater;
    translation   = vTranslation;
    //GlobalScaling = vScaling;
    return CalcAbsoluteMatrix(this);
  }

  CFloat4x4& TRANSFORM::Set(CFloat3& vScaling, CFloat3& vEuler, CFloat3& vTranslation)
  {
    scaling       = vScaling;
    rotation.YawPitchRollA(vEuler);
    translation   = vTranslation;
    //GlobalScaling = vScaling;
    return CalcAbsoluteMatrix(this);
  }

  CFloat4x4& TRANSFORM::Set(CFloat4x4& Coordinate, CFloat3& vScaling, CQuaternion& quater, CFloat3& vTranslation)
  {
    scaling     = vScaling;
    rotation    = quater;
    translation = vTranslation;

    CalcAbsoluteMatrix(this, Coordinate);
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
    return GlobalMatrix;
  }

  CFloat4x4& TRANSFORM::Set(CFloat4x4& Coordinate, CFloat3& vScaling, CFloat3& vEuler, CFloat3& vTranslation)
  {
    scaling     = vScaling;
    rotation.YawPitchRollA(vEuler);
    translation = vTranslation;

    CalcAbsoluteMatrix(this, Coordinate);
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
    return GlobalMatrix;
  }

  b32 TRANSFORM::SetTransform(CFloat4x4& mat)
  {
    GlobalMatrix = mat;
    b32 bret = mat.Decompose(&scaling, &rotation, &translation);
    if(bret) {
      //GlobalScaling = scaling; // 为什么有这个？
    }
    return bret;
  }   
  
  void TRANSFORM::SetPosition(CFloat3& vPos)
  {
    translation = vPos;
    //GlobalScaling = scaling;  // 为什么有这个？
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::SetScaling(CFloat3& vScaling)
  {
    scaling = vScaling;
    //GlobalScaling = vScaling; // 为什么有这个？
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::SetRotationA(CFloat3& vEuler)
  {
    rotation.YawPitchRollA(vEuler);
    //GlobalScaling = scaling; // 为什么有这个？
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::SetRotationR(CFloat3& vEuler)
  {
    rotation.YawPitchRollR(vEuler);
    //GlobalScaling = scaling; // 为什么有这个？
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::SetRotation(CQuaternion& quater)
  {
    rotation = quater;
    //GlobalScaling = scaling; // 为什么有这个？
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::RotateA(CFloat3& vEuler)
  {
    quaternion r;
    r.YawPitchRollA(vEuler);
    rotation = r * rotation;  // 在自身的旋转坐标之上做旋转
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::RotateA( CFloat4x4& Coordinate, CFloat3& vEuler )
  {
    quaternion r;
    r.YawPitchRollA(vEuler);
    rotation = r * rotation;  // 在自身的旋转坐标之上做旋转
    CalcAbsoluteMatrix(this, Coordinate);
  }

  void TRANSFORM::RotateR(CFloat3& vEuler)
  {
    quaternion r;
    r.YawPitchRollR(vEuler);
    rotation = r * rotation;  // 在自身的旋转坐标之上做旋转
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::RotateR( CFloat4x4& Coordinate, CFloat3& vEuler )
  {
    quaternion r;
    r.YawPitchRollR(vEuler);
    rotation = r * rotation;  // 在自身的旋转坐标之上做旋转
    CalcAbsoluteMatrix(this, Coordinate);
  }

  void TRANSFORM::Rotate(CQuaternion& quater)
  {
    rotation *= quater;
    CalcAbsoluteMatrix(this);
  }

  void TRANSFORM::Rotate( CFloat4x4& Coordinate, CQuaternion& quater )
  {
    rotation *= quater;
    CalcAbsoluteMatrix(this, Coordinate);
  }

  b32 TRANSFORM::SetDirection(CFloat3& vDir, CFloat3& vUp)
  {
    if((vDir - vUp) < FLT_EPSILON) {
      return FALSE;
    }

    float4x4 matRotation;
    matRotation.LookAtLH(float3::Origin, vDir, vUp);
    rotation.FromRotationMatrix(matRotation);
    //GlobalScaling = scaling;
    CalcAbsoluteMatrix(this);
    DBGCHECKGLOBALSCALINGCHANGED;
    return TRUE;
  }

  void TRANSFORM::SetPosition(CFloat4x4& Coordinate, CFloat3& vPos, ESpace eSpace)
  {
    if(eSpace == S_Self)
    {
      translation = vPos;
      CalcAbsoluteMatrix(this, Coordinate);
    }
    else if(eSpace == S_World)
    {
      GlobalMatrix.SetRow(3, float4(vPos));
      float4x4 matLocal = float4x4::inverse(Coordinate) * GlobalMatrix;
      translation = GlobalMatrix.GetRow(3);
    }
    else CLBREAK;
    //DBGCHECKGLOBALSCALINGCHANGED;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
  }

  void TRANSFORM::SetScaling(CFloat4x4& Coordinate, CFloat3& vScaling)
  {
    scaling = vScaling;
    CalcAbsoluteMatrix(this, Coordinate);
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
  }

  void TRANSFORM::SetRotationA(CFloat4x4& Coordinate, CFloat3& vEuler)
  {
    rotation.YawPitchRollA(vEuler);
    CalcAbsoluteMatrix(this, Coordinate);
    //DBGCHECKGLOBALSCALINGCHANGED;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
  }

  void TRANSFORM::SetRotationR(CFloat4x4& Coordinate, CFloat3& vEuler)
  {
    rotation.YawPitchRollR(vEuler);
    CalcAbsoluteMatrix(this, Coordinate);
    //DBGCHECKGLOBALSCALINGCHANGED;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
  }

  void TRANSFORM::SetRotation(CFloat4x4& Coordinate, CQuaternion& quater)
  {
    rotation = quater;
    CalcAbsoluteMatrix(this, Coordinate);
    //DBGCHECKGLOBALSCALINGCHANGED;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
  }

  float4x4 TRANSFORM::ToRelativeMatrix() CLCONST
  {
    float4x4 s, r, t;
    s.Scale(scaling);
    r.RotationQuaternion(&rotation);
    t.Translate(translation.x, translation.y, translation.z);
    return (s * r * t);
  }

  b32 TRANSFORM::SetDirection(CFloat4x4& Coordinate, CFloat3& vDir, CFloat3& vUp)
  {
    CFloat3 vDelta = vDir - vUp;
    if(float3::abs(vDelta) < FLT_EPSILON) {
      return FALSE;
    }

    float4x4 matRotation;
    matRotation.LookAtLH(float3::Origin, vDir, vUp);
    rotation.FromRotationMatrix(matRotation);
    CalcAbsoluteMatrix(this, Coordinate);
    //DBGCHECKGLOBALSCALINGCHANGED;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
    return TRUE;
  }

  b32 TRANSFORM::SetTransform(CFloat4x4& Coordinate, CFloat4x4& mat, ESpace eSpace)
  {
    switch(eSpace)
    {
    case clstd::S_Self:
      {
        GlobalMatrix = Coordinate * mat;
        return mat.Decompose(&scaling, &rotation, &translation);
      }
    case clstd::S_World:
      {
        GlobalMatrix = mat;
        float4x4 matLocal = float4x4::inverse(Coordinate) * GlobalMatrix;
        return matLocal.Decompose(&scaling, &rotation, &translation);
      }
    default:
      CLBREAK;
    }
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
    return TRUE;
  }

  CFloat4x4& TRANSFORM::UpdateAbsoluteMatrix(CFloat4x4& Coordinate)
  {
    GlobalMatrix = ToRelativeMatrix() * Coordinate;
    //GlobalMatrix.DecomposeScaling(&GlobalScaling);
    return GlobalMatrix;
  }

  CFloat4x4& TRANSFORM::UpdateAbsoluteMatrix()
  {
    GlobalMatrix = ToRelativeMatrix();
    //GlobalScaling = scaling;
    return GlobalMatrix;
  }

#ifdef _DEBUG
  b32 TRANSFORM::DbgCheckGlobalScalingChanged()
  {
    float3 vScaling;
    GlobalMatrix.DecomposeScaling(&vScaling);
    return 
      clstd::approximate(vScaling.x, scaling.x, 1e-3f) &&
      clstd::approximate(vScaling.y, scaling.y, 1e-3f) &&
      clstd::approximate(vScaling.z, scaling.z, 1e-3f);
  }

  euler TRANSFORM::GetEuler( RotationOrder eOrder ) const
  {
    float3x3 r = rotation.ToMatrix3x3();
    euler e;
    switch(eOrder)
    {
    case RotationOrder_EulerXYZ:      r.ToEulerAnglesXYZ(e);      break;
    case RotationOrder_EulerXZY:      r.ToEulerAnglesXZY(e);      break;
    case RotationOrder_EulerYXZ:      r.ToEulerAnglesYXZ(e);      break;
    case RotationOrder_EulerYZX:      r.ToEulerAnglesYZX(e);      break;
    case RotationOrder_EulerZXY:      r.ToEulerAnglesZXY(e);      break;
    case RotationOrder_EulerZYX:      r.ToEulerAnglesZYX(e);      break;
    }
    return e;
  }

#endif // #ifdef _DEBUG

} // namespace clstd