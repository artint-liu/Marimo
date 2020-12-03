#ifndef _GEOMETRY_IMPL_H_
#define _GEOMETRY_IMPL_H_
namespace clstd
{
  namespace geometry
  {
    struct Plane;
    struct NormalizedRay;
    struct FrustumPlanes;

    class AABB
    {
    public:
      // union
      // {
         //struct {
         //	float3 vMin, vMax;
         //};
      //   float3 m[2];
      // };
      float3 vMin, vMax;


      AABB()
        : vMin(FLT_MAX)
        , vMax(FLT_MIN)
      {
      }

      AABB(const float3& _vMin, const float3& _vMax)
        : vMin(_vMin)
        , vMax(_vMax)
      {
      }

      AABB(float minX, float minY, float minZ,
        float maxX, float maxY, float maxZ)
        : vMin(minX, minY, minZ)
        , vMax(maxX, maxY, maxZ)
      {
      }

      void Clear()
      {
        vMin = FLT_MAX;
        vMax = -FLT_MAX;
      }

      b32 IsPointIn(const float3& vPos) const
      {
        return vPos >= vMin && vPos <= vMax;
      }

      void AppendVertex(const float3& vVert)
      {
        vMin.Min(vVert);
        vMax.Max(vVert);
      }

      float3 GetCenter() const
      {
        return (vMin + vMax) * 0.5f;
      }

      float3 GetExtent() const
      {
        return (vMax - vMin) * 0.5f;
      }

      AABB& Merge(const AABB& aabb)
      {
        vMin.Min(aabb.vMin);
        vMin.Min(aabb.vMax);
        vMax.Max(aabb.vMin);
        vMax.Max(aabb.vMax);
        return *this;
      }

      operator float3*() const
      {
        return (float3*)this;
      }

      float3 GetVertex(int nIndex) const
      {
        ASSERT(nIndex >= 0 && nIndex < 8);
        float3* m = *this;
        return float3(m[nIndex & 1].x, m[(nIndex & 2) >> 1].y, m[(nIndex & 4) >> 2].z);
      }

      AABB& GetSubAsOctree(int nIndex, AABB* pSubAABB) const // 按照x,y,z轴顺序切割AABB
      {
        float3 vCenter = (vMin + vMax) * 0.5f;
        if(nIndex & 1)
        {
          pSubAABB->vMin.x = vCenter.x;
          pSubAABB->vMax.x = vMax.x;
        }
        else
        {
          pSubAABB->vMin.x = vMin.x;
          pSubAABB->vMax.x = vCenter.x;
        }
        if(nIndex & 2)
        {
          pSubAABB->vMin.y = vCenter.y;
          pSubAABB->vMax.y = vMax.y;
        }
        else
        {
          pSubAABB->vMin.y = vMin.y;
          pSubAABB->vMax.y = vCenter.y;
        }
        if(nIndex & 4)
        {
          pSubAABB->vMin.z = vCenter.z;
          pSubAABB->vMax.z = vMax.z;
        }
        else
        {
          pSubAABB->vMin.z = vMin.z;
          pSubAABB->vMax.z = vCenter.z;
        }
        return *pSubAABB;
      }

      AABB& operator+=(const float3& vOffset)
      {
        vMin += vOffset;
        vMax += vOffset;
        return *this;
      }

      AABB& operator-=(const float3& vOffset)
      {
        vMin -= vOffset;
        vMax -= vOffset;
        return *this;
      }

      AABB& Set(const float3 _vMin, const float3 _vMax)
      {
        vMin = _vMin;
        vMax = _vMax;
        return *this;
      }

      AABB& Set(float x0, float y0, float z0, float x1, float y1, float z1)
      {
        vMin.set(x0, y0, z0);
        vMax.set(x1, y1, z1);
        return *this;
      }

      int IntersectAABB     (const AABB& aabb) const;
      int IntersectFrustum  (const FrustumPlanes& frustum) const;
      int IntersectRay      (const NormalizedRay& ray, float* fDist, float3* pNormal) const;

      friend AABB operator+(const AABB& aabb, const float3& vOffset)
      {
        return AABB(aabb.vMin + vOffset, aabb.vMax + vOffset);
      }

      friend AABB operator-(const AABB& aabb, const float3& vOffset)
      {
        return AABB(aabb.vMin - vOffset, aabb.vMax - vOffset);
      }

      friend AABB operator*(const AABB& aabb, const float3& vScaling)
      {
        return AABB(aabb.vMin * vScaling, aabb.vMax * vScaling);
      }
    }; // class _AABBF

    STATIC_ASSERT(sizeof(AABB) == sizeof(float3) * 2);

    template<typename _Ty>
    class AABBI_T
    {
      typedef _vector3<_Ty> v3;
    public:
      typedef _Ty Type;
      union {
        struct {
          v3 vMin;
          v3 vMax;
        };
        struct {
          v3 m[2];
        };
      };

      AABBI_T()
        : vMin(0)
        , vMax(0)
      {
      }

      AABBI_T(const AABBI_T& aabb)
        : vMin(aabb.vMin)
        , vMax(aabb.vMax)
      {}


      AABBI_T(Type x0, Type y0, Type z0, Type x1, Type y1, Type z1)
        : vMin(x0, y0, z0)
        , vMax(x1, y1, z1)
      {
      }

      AABBI_T(const v3& _vMin, const v3& _vMax)
        : vMin(_vMin)
        , vMax(_vMax)
      {
      }

      b32 operator==(const AABBI_T& aabb) const
      {
        return (vMin == aabb.vMin && vMax == aabb.vMax);
      }

      b32 operator!=(const AABBI_T& aabb) const
      {
        return (vMin != aabb.vMin || vMax != aabb.vMax);
      }

      AABBI_T& Set(const v3& _vMin, const v3& _vMax)
      {
        vMin = _vMin;
        vMax = _vMax;
        return *this;
      }

      AABBI_T& Set(Type x0, Type y0, Type z0, Type x1, Type y1, Type z1)
      {
        vMin.set(x0, y0, z0);
        vMax.set(x1, y1, z1);
        return *this;
      }

      b32 IsPointIn(const v3& vPos) const
      {
        return vPos >= vMin && vPos <= vMax;
      }

      AABBI_T& operator+=(const v3& vOffset)
      {
        vMin += vOffset;
        vMax += vOffset;
        return *this;
      }

      AABBI_T& operator-=(const v3& vOffset)
      {
        vMin -= vOffset;
        vMax -= vOffset;
        return *this;
      }

      AABBI_T& operator=(const AABBI_T& aabb)
      {
        vMin = aabb.vMin;
        vMax = aabb.vMax;
        return *this;
      }

      friend AABBI_T operator+(const AABBI_T& aabb, const v3& vOffset)
      {
        return AABBI_T(aabb.vMin + vOffset, aabb.vMax + vOffset);
      }

      friend AABBI_T operator-(const AABBI_T& aabb, const v3& vOffset)
      {
        return AABBI_T(aabb.vMin - vOffset, aabb.vMax - vOffset);
      }

      void AppendVertex(int nIdx, const v3& vVert)
      {
        if(nIdx == 0) {
          vMin = vVert;
          vMax = vVert;
        }
        else {
          vMin.Min(vVert);
          vMax.Max(vVert);
        }
      }

      v3 GetVertex(int nIndex)
      {
        ASSERT(nIndex >= 0 && nIndex < 8);
        return v3(m[nIndex & 1].x, m[(nIndex & 2) >> 1].y, m[(nIndex & 4) >> 2].z);
      }

      AABBI_T& GetSubAsOctree(int nIndex, AABBI_T* pSubAABB) const // 按照x,y,z轴顺序切割AABB
      {
        // 避免整数误差引起的问题
        ASSERT(vMin.x + 1 < vMax.x && vMin.y + 1 < vMax.y && vMin.z + 1 < vMax.z);
        v3 vCenter = (vMin + vMax) / 2;

        if(nIndex & 1)
        {
          pSubAABB->vMin.x = vCenter.x;
          pSubAABB->vMax.x = vMax.x;
        }
        else
        {
          pSubAABB->vMin.x = vMin.x;
          pSubAABB->vMax.x = vCenter.x;
        }
        if(nIndex & 2)
        {
          pSubAABB->vMin.y = vCenter.y;
          pSubAABB->vMax.y = vMax.y;
        }
        else
        {
          pSubAABB->vMin.y = vMin.y;
          pSubAABB->vMax.y = vCenter.y;
        }
        if(nIndex & 4)
        {
          pSubAABB->vMin.z = vCenter.z;
          pSubAABB->vMax.z = vMax.z;
        }
        else
        {
          pSubAABB->vMin.z = vMin.z;
          pSubAABB->vMax.z = vCenter.z;
        }
        return *pSubAABB;
      }

      AABBI_T& Inflate(const v3& v) {
        vMin -= v;
        vMax += v;
        return *this;
      }

      int IntersectAABB(const AABBI_T& aabb) const
      {
        v3 vMinRes = v3::Max(vMin, aabb.vMin);
        v3 vMaxRes = v3::Min(vMax, aabb.vMax);
        return vMinRes <= vMaxRes;  // 这个"="以后修改要小心, 注意AABB是个薄片(vMin.x==vMax.x || vMin.y==vMax.y || vMin.z==vMax.z)的情况
      }
    };

    template<typename _Ty>
    AABB ConvertAABB(const AABBI_T<_Ty>& aabbSrc)
    {
      AABB aabb;
      for(int i = 0; i < 3; i++) {
        aabb.vMin.m[i] = (float)aabbSrc.vMin.m[i];
        aabb.vMax.m[i] = (float)aabbSrc.vMax.m[i];
      }
      return aabb;
    }
    //////////////////////////////////////////////////////////////////////////

    struct Plane
    {
      float3  vNormal;
      float   fDist;

      Plane();
      Plane(const float3* aVert3);
      Plane(const float4& vNormalDist);
      Plane(const float3& vVert0, const float3& vVert1, const float3& vVert2);
      Plane(const Plane& Plane);
      Plane(const float3& _vNormal, const float _fDist);

      operator const float4&() const
      {
        return *(float4*)this;
      }

      Plane& normalize();
      static Plane normalize(const Plane& plane);

      void set(const float3* aVert3);
      void set(const float3& vVert0, const float3& vVert1, const float3& vVert2);
      void set(const Plane& Plane);
      void set(const float3& _vNormal, const float _fDist);

    };
    //////////////////////////////////////////////////////////////////////////
    struct Triangle
    {
      // 定点顺序是CCW
      _float3 A, B, C;

      Triangle();
      Triangle(const _float3* aVertices); // 至少需要3个
      Triangle(const _float3* aVertices, int A, int B, int C); // 根据索引从顶点列表中初始化三角形
      Triangle(const _float3& A, const _float3& B, const _float3& C);

      Triangle& set(const _float3* aVertices); // 至少需要3个
      Triangle& set(const _float3* aVertices, int A, int B, int C);
      Triangle& set(const _float3& A, const _float3& B, const _float3& C);
      Triangle& flip(); // 翻转顶点顺序
      _float3 normal() const;   // 返回三角形的面法线
      AABB bounding() const;  // 返回三角形的AABB
      operator _float3*() const;

      b32 intersect(const Triangle& t); // 判断两三角形相交(无交点)
      //int intersect(const _triangle& t);  // 判断与另一个三角形相交的情况,返回值是交点数[0, 3]
    };

    STATIC_ASSERT(sizeof(Triangle) == sizeof(_float3) * 3);


    //////////////////////////////////////////////////////////////////////////
    struct Ray
    {
      float3 vOrigin;
      float3 vDirection;

      Ray(const NormalizedRay& nr);
      Ray() : vOrigin(0.0f), vDirection(0.0f) {}
      Ray(const float3& _vOrigin, const float3& _vDirection);
      Ray& set(const float3& _vOrigin, const float3& _vDirection)
      {
        vOrigin = _vOrigin;
        vDirection = _vDirection;
        return *this;
      }
    };

    struct NormalizedRay
    {
      float3 vOrigin;
      float3 vDirection;

      NormalizedRay(const float3& _vOrigin, const float3& _vDirection);
      NormalizedRay(const Ray& ray)
      {
        vOrigin = ray.vOrigin;
        vDirection = float3::normalize(ray.vDirection);
      }
    };
    //////////////////////////////////////////////////////////////////////////

    class LineSegment
    {
    public:
      float3 vPoint[2];

      LineSegment(const float3& vPoint0, const float3& vPoint1);
      LineSegment(const float3* pvPoint);
    };

    struct _frustum
    {
      float3 Origin;              // Origin of the frustum (and projection).
      _quaternion Orientation;    // Unit quaternion representing rotation.

      float RightSlope;           // Positive X slope (X/Z).
      float LeftSlope;            // Negative X slope.
      float TopSlope;             // Positive Y slope (Y/Z).
      float BottomSlope;          // Negative Y slope.
      float Near, Far;            // Z of the near plane and far plane.
    };

    struct FrustumPlanes
    {
      Plane planes[6];

      FrustumPlanes() {};
      FrustumPlanes(const float4x4& m);
      FrustumPlanes& set(const float4x4& m);
      int IntersectAABB (const AABB& aabb) const;
    };

    struct OBB
    {
      float3 Center;            // Center of the box.
      float3 Extents;           // Distance from the center to each side.
      _quaternion Orientation;  // Unit quaternion representing rotation (box -> world).
    };
  } // namespace geomerty
} // namespace clstd
//////////////////////////////////////////////////////////////////////////
//typedef clstd::_AABBF               aabb_t;
//typedef clstd::_plane               plane_t;
//typedef clstd::_ray                 ray_t;
//typedef clstd::_NormalizedRay       normalized_ray_t;
//typedef clstd::_LineSegment         line_segment_t;
//typedef clstd::_FrustumPlanes       frustum_planes_t;
//typedef clstd::OBB                  obb_t;
//typedef const aabb_t CAABB;

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _GEOMETRY_IMPL_H_