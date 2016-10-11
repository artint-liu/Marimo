#ifndef _CLSTD_QUATERNION_H_
#define _CLSTD_QUATERNION_H_

#ifndef _CL_STD_CODE_
#error Must be include "clstd.h" first.
#endif // #ifndef _CL_STD_CODE_

namespace clstd
{
  struct _quaternion
  {
    union
    {
      struct
      {
        float x, y, z, w;
      };
      float m[4];
    };
    _quaternion(){}
    _quaternion(float f): x(f), y(f), z(f), w(f){}
    _quaternion(float x, float y, float z, float w) {this->x = x; this->y = y; this->z = z; this->w = w;}

    bool operator==(const _quaternion& q) const
    {
      return q.x == x && q.y == y && q.z == z && q.w == w;
    }

    bool operator!=(const _quaternion& q) const
    {
      return q.x != x || q.y != y || q.z != z || q.w != w;
    }

    _float3 operator*(const _float3& v) const
    {
      // nVidia SDK implementation
      _float3 uv, uuv;
      _float3 qvec(x, y, z);
      uv  = qvec.cross(v);
      uuv = qvec.cross(uv);
      uv  *= (2.0f * w);
      uuv *= 2.0f;

      return v + uv + uuv;
    }

    _quaternion operator*(const _quaternion& q) const;
    _quaternion& operator*=(const _quaternion& q);

    _quaternion& normalize();

    _quaternion& FromRotationMatrix(const _float4x4& m);
    _quaternion& FromRotationMatrix(const _float3x3& m);

    float3x3 ToMatrix3x3() const;
    float4x4 ToMatrix() const;

    _quaternion& YawPitchRollA(float yaw, float pitch, float roll);
    _quaternion& YawPitchRollR(float yaw, float pitch, float roll);
    _quaternion& YawPitchRollA(const _euler& e);
    _quaternion& YawPitchRollR(const _euler& e);

       		/** Calculate the local roll element of this quaternion.
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Y of the quaternion onto the X and
			Y axes, the angle between them is returned. If set to false though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
             may involve less axial rotation.  The co-domain of the returned value is 
             from -180 to 180 degrees.
		*/
		float GetRoll(bool reprojectAxis = true) const; // OGRE
   		/** Calculate the local pitch element of this quaternion
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Z of the quaternion onto the X and
			Y axes, the angle between them is returned. If set to true though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
            may involve less axial rotation.  The co-domain of the returned value is 
            from -180 to 180 degrees.
		*/
		float GetPitch(bool reprojectAxis = true) const; // OGRE
   		/** Calculate the local yaw element of this quaternion
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Y of the quaternion onto the X and
			Z axes, the angle between them is returned. If set to true though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
			may involve less axial rotation. The co-domain of the returned value is 
            from -180 to 180 degrees.
		*/
		float GetYaw(bool reprojectAxis = true) const;// OGRE
  };

  struct _euler
  {
    union
    {
      struct {
        float x, y, z;
      };
      struct {
        float pitch, yaw, roll;
      };
      float m[3];
    };
    _euler(){}
    _euler(const _float3& v) : x(v.x), y(v.y), z(v.z) {}
    _euler(float vx, float vy, float vz) : x(vx), y(vy), z(vz) {}

    operator _float3() const
    {
      return _float3(x, y, z);
    }

    _euler& positivity(); // 正数化，所有分量转为正数
    _euler& negativity(); // 负数化，所有分量转为负数
  };


  inline float QuaternionLengthSq(const _quaternion *pQ)
  {
    return pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w;
  }

  inline float QuaternionLength(const _quaternion *pQ)
  {
#ifdef __cplusplus
    return sqrtf(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w);
#else
    return (FLOAT) sqrt(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w);
#endif
  }

  inline float QuaternionDot(const _quaternion *pQ1, const _quaternion *pQ2)
  {
    return pQ1->x * pQ2->x + pQ1->y * pQ2->y + pQ1->z * pQ2->z + pQ1->w * pQ2->w;
  }


  _quaternion* QuaternionBaryCentric          (_quaternion *pout, const _quaternion *pq1, const _quaternion *pq2, const _quaternion *pq3, float f, float g);
  _quaternion* QuaternionExp                  (_quaternion *pout, const _quaternion *pq);
  _quaternion* QuaternionInverse              (_quaternion *pout, const _quaternion *pq);
  _quaternion* QuaternionLn                   (_quaternion *pout, const _quaternion *pq);
  _quaternion* QuaternionMultiply             (_quaternion *pout, const _quaternion *pq1, const _quaternion *pq2);
  _quaternion* QuaternionRotationAxis         (_quaternion *pout, const _float3 *pv, float angle);
  _quaternion* QuaternionRotationMatrix       (_quaternion *pout, const _float4x4 *pm);
  _quaternion* QuaternionRotationYawPitchRoll (_quaternion *pout, float yaw, float pitch, float roll);
  _quaternion* QuaternionSlerp                (_quaternion *pout, const _quaternion *pq1, const _quaternion *pq2, float t);
  _quaternion* QuaternionSquad                (_quaternion *pout, const _quaternion *pq1, const _quaternion *pq2, const _quaternion *pq3, const _quaternion *pq4, float t);
  _quaternion* QuaternionNormalize            (_quaternion *pout, const _quaternion *pq);
  void         QuaternionToAxisAngle          (const _quaternion *pq, _float3 *paxis, float *pangle);
} // namespace clstd

typedef clstd::_quaternion quaternion;
typedef clstd::_euler euler;
typedef const quaternion  CQuaternion;
typedef const euler       CEuler;

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // #ifndef _CLSTD_QUATERNION_H_