// GrapVR 的基础对象定义文件, 其他场景的节点均在此继承
#ifndef _GRAPVR_NODE_H_
#define _GRAPVR_NODE_H_

class GVScene;
//class SmartRepository;
struct GVSCENEUPDATE;
class GVNode;
//enum GXPRIMITIVETYPE;

namespace GrapX
{
  class Graphics;
  class Canvas3D;
  class Primitive;
} // namespace GrapX

//enum GVModelClass // TODO[9]: 改为GXDWORD 然后是 MAKEFOURCC 生成
//{
//  GVMC_UNKNOWN,
//  GVMC_GEOMETRY,
//  GVMC_MESH,
//  GVMC_APPIMPL,   // 应用程序实现,用户扩展
//};

// TODO[8]: Raytrace 不支持带WorldMat的物体

enum GVNodeFlags
{
  GVNF_VISIBLE        = 0x00000001,     // 用户设置的可见标志, 如果没有此标志则不参与可见性检测
  GVNF_NORAYTRACE     = 0x00000004,     // 避免RayTrace接口的探测, 影响子对象
  GVNF_UPDATEWORLDMAT = 0x00000008,     // 更新WorldMatrix, 否则按照单位矩阵更新
  GVNF_CONTAINER      = 0x00000010,     // 不包含Primitive数据的容器
  GVNF_NOCLIP         = 0x00000020,     // 无AABB与视锥的预裁剪
  GVNF_PHYSICALBODY   = 0x00000040,     // 基于物理系统的节点, 位置可能不受 Node 的父子关系影响
  GVNF_CASTSHADOW     = 0x00000080,     // 阴影投影标志
  
  //TODO: 用GVIC_UPDATE代替
  //GVNF_DONOTUPDATE    = 0x00000100,     // 避免调用节点的Update接口，这个标志只屏蔽当前节点Update，不影响子节点

  GVNF_STATIC         = 0x00000200,     // 静态物体，不支持SetPosition
  GVNF_COLLIDER       = 0x00000400,     // 有碰撞


  GVNF_USER_0         = 0x01000000,     // 用户定义标志
  GVNF_USER_1         = 0x02000000,
  GVNF_USER_2         = 0x04000000,
  GVNF_USER_3         = 0x08000000,
  GVNF_USER_4         = 0x10000000,
  GVNF_USER_5         = 0x20000000,
  GVNF_USER_6         = 0x40000000,
  GVNF_USER_7         = (int)0x80000000,

  GVNF_DEFAULT        = GVNF_VISIBLE|GVNF_CASTSHADOW,
};

// FIXME: 这个还没开始用
// 接口实现程度标志，在调用Node接口时会检测对应标志，如果标志为空则不调用Node的virtual接口。
// 基类接口实现中会去掉对应标志，所以在第一次调用接口后不会再调用了。
enum GVNodeInterfaceCaps
{
  GVIC_UPDATE    = 0x00000001,
  GVIC_NOTIFY    = 0x00000002,
  GVIC_COLLISION = 0x00000004,
};

const int DefaultRenderCategory = 0;
//enum GVRenderType
//{
//  GVRT_Normal  = 0x100, // 这个名字不够酷, 想着以后改一下
//  GVRT_Reflact = 0x200,
//  GVRT_Reflect = 0x300,
//  GVRT_Shadow  = 0x400,
//};
//
// GVNode::SetMaterialInst 和
// GVNode::SetMaterialInstFromFileW 标志:
//
#define NODEMTL_SETCHILDREN   0x00000001  // 设置子孙节点的材质
#define NODEMTL_SETSONONLY    0x00000002  // 忽略NODEMTL_SETCHILDREN标志, 只设置儿子(第一级子节点)的材质
#define NODEMTL_CLONEINST     0x00000004  // 对原有材质克隆一份新的实例
#define NODEMTL_IGNOREVERT    0x00000008  // 不测量顶点格式

enum NodeRayTraceType // 相交级别, 按照精确等级递增
{
  NRTT_NONE,
  NRTT_AABB,      // 包围盒相交
  NRTT_MESHFACE,  // 精确相交
};

struct NODERAYTRACE
{
  NodeRayTraceType eType;
  float   fSquareDist;
  float3  vLocalHit;    // 模型空间的坐标
  int     nFaceIndex;   // 不一定总有
};

struct NODENOTIFY
{
  GXUINT cbSize;
  GXDWORD code;
};

struct GVRENDERDESC
{
  GrapX::Primitive* pPrimitive;
  GXPrimitiveType   ePrimType;
  GrapX::Material*   pMaterial;
  float4x4          matWorld;   // 全局变换, 这个应该返回TRANSFORM::GlobalMatrix, 否则裁剪会有问题

  GXDWORD dwFlags;              // 参考 GVModelFlags 定义
  GXDWORD dwLayer;
  GXUINT  RenderQueue;          // 渲染队列, 尚未使用, 这个默认取自Material中的记录
  GXINT   BaseVertexIndex;      // StartVertex
  GXUINT  MinIndex;
  GXUINT  NumVertices;
  GXUINT  StartIndex;
  GXUINT  PrimitiveCount;
};

typedef clvector<GrapX::ObjectT<GrapX::Material>>  MaterialArray;

struct GVRENDERDESC2
{
  GVNode*           pNode = NULL;
  GrapX::Primitive* pPrimitive = NULL;
  GXPrimitiveType   ePrimType = GXPT_TRIANGLELIST;
  MaterialArray     materials;
  //float4x4          matWorld = float4x4::Identity;             // 全局变换, 这个应该返回TRANSFORM::GlobalMatrix, 否则裁剪会有问题

  //GXUINT            RenderQueue = GrapX::RenderQueue_Geometry;          // 渲染队列, 尚未使用, 这个默认取自Material中的记录
  GXINT             BaseVertexIndex = 0;      // StartVertex
  GXUINT            MinIndex = 0;
  GXUINT            NumVertices = 0;
  GXUINT            StartIndex = 0;
  GXUINT            PrimitiveCount = 0;
  clstd::geometry::AABB aabbAbsulate;
};

//struct GXGEOMETRYDESC
//{
//  AABB          aabb;
//  float4x4      matRelative;
//  float4x4      matAbsolute;
//};

class GXDLL GVNode : public GUnknown, public clstd::treeT<GVNode>
{
  friend class clstd::treeT<GVNode>;
public:
  typedef clstd::TRANSFORM TRANSFORM;
  typedef clstd::geometry::AABB AABB;
  typedef clstd::_quaternion Quaternion;
  typedef clstd::geometry::Ray Ray;
  typedef clstd::geometry::Plane Plane;
  typedef clstd::geometry::NormalizedRay NormalizedRay;
  typedef clstd::geometry::FrustumPlanes FrustumPlanes;
  //typedef clvector<GrapX::ObjectT<GrapX::Material>>  MaterialArray;
  enum ESpace {
    S_ABSOLUTE = clstd::S_World,
    S_RELATIVE = clstd::S_Self,
    S_PHYSICALBODY,   // 输入参数作为 T_ABSOLUTE, 但不更新子节点位置, 相对矩阵被忽略
  };
protected:
  GXDWORD       m_ClsCode;
  clStringA     m_strName;
  GXDWORD       m_dwFlags;        // GVModelFlags
  GXDWORD       m_dwLayer;        // 层掩码
  GXDWORD       m_dwInterfaceCaps;
  AABB          m_aabbLocal;

  // TODO: 考虑不用矩阵而改用平移/旋转/缩放来表示
  //float4x4      m_matRelative;    // 相对于父节点的变换矩阵
  //float4x4      m_matAbsolute;    // 最终的变换矩阵, m_matRelative * pParent->m_matAbsolute
  TRANSFORM     m_Transformation;
  // Coordinate
  virtual    ~GVNode();
private:
  void    UpdateChildPos();
protected:
  virtual GXHRESULT LoadFileA(GrapX::Graphics* pGraphics, GXLPCSTR szFilename);
  virtual GXHRESULT LoadFileW(GrapX::Graphics* pGraphics, GXLPCWSTR szFilename);
  virtual GXHRESULT LoadFile (GrapX::Graphics* pGraphics, clSmartRepository* pStorage);
public:
  //GVModel   (GVModelClass eClass);
  GVNode   (GVScene* pScene, GXDWORD dwClassCode);

  GXBOOL    Show                      (GXBOOL bShow);
  GXDWORD   SetFlags                  (GXDWORD dwNewFlags); // 设置新标志, 这个会清除原来的标志
  GXDWORD   CombineFlags              (GXDWORD dwFlags);    // 与原有标志合并
  GXDWORD   SetLayer                  (GXDWORD dwLayerMask);
  GXDWORD   GetLayer                  () const;
//void      CombineRecursiveFlags    (GXDWORD dwFlags);
//void      RemoveRecursiveFlags     (GXDWORD dwFlags);
  AABB&     GetAbsoluteAABB           (AABB& aabb) const;

  void      SetScaling                (CFloat3& vScaling/*, ESpace eTransform = S_RELATIVE*/);

  void      SetRotationA              (CFloat3& vEuler);  // Angle
  void      SetRotationR              (CFloat3& vEuler);  // Radian
  void      SetRotation               (const Quaternion& quater);
  void      RotateA                   (CFloat3& vEuler);
  void      RotateR                   (CFloat3& vEuler);
  void      Rotate                    (const Quaternion& quater);

  void      Move                      (CFloat3& vDelta);
  float3    GetPosition               (ESpace eTransform = S_RELATIVE);
  void      SetPosition               (CFloat3& vPos, ESpace eTransform = S_RELATIVE);

  void      SetTransform              (const float4x4& matTransform, ESpace eTransform = S_RELATIVE);
  void      SetTransform              (const float3* pScaling, const Quaternion* pRotation, const float3* pTranslation); // 参数为NULL则使用自身的分量
  void      SetTransformR             (const float3* pScaling, const float3* pEuler, const float3* pTranslation); // 参数为NULL则使用自身的分量

  GXBOOL    SetDirection              (CFloat3& vDir, CFloat3& vUp/*, ESpace eTransform = S_RELATIVE*/);
  GVNode*   FindChild                 (GVNode* pStart, GXLPCSTR szName);
  GVNode*   SetParent                 (GVNode* pNewParent, bool bLast = false);
  GXBOOL    SetMaterial               (GrapX::Material* pMtlInst, GXDWORD dwFlags);  // 参考 NODEMTL_* 标志
  GXBOOL    SetMaterialFromFile       (GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GXDWORD dwFlags);  // 参考 NODEMTL_* 标志

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef    ();
  virtual GXHRESULT Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual void      OnChangingFlags           (GXDWORD dwNewFlags);
  virtual GXVOID    OnNotify                  (const NODENOTIFY* pNotify);
  virtual GXBOOL    Update                    (const GVSCENEUPDATE& sContext);
  virtual GXBOOL    Collision                 ();
  virtual GXVOID    CalculateAABB             ();
  virtual void      GetRenderDesc             (int nRenderCate, GVRENDERDESC* pRenderDesc);
  virtual GVRENDERDESC2* GetRenderDesc         (int nRenderCate);
  virtual GXBOOL    RayTrace                  (const Ray& ray, NODERAYTRACE* pRayTrace);
  virtual GXBOOL    SetMaterial               (GrapX::Material* pMtlInst, int nRenderCate = DefaultRenderCategory);
  virtual GrapX::Material* SetMaterial               (GrapX::Shader* pShader, int nRenderCate = DefaultRenderCategory); // 从shader创建一个全新材质
  virtual GXBOOL    GetMaterial               (int nRenderCate, GrapX::Material** ppMtlInst);
  virtual GrapX::Material* GetMaterialUnsafe  (int nRenderCate);
  virtual GXBOOL    GetMaterialFilename       (int nRenderCate, clStringW* pstrFilename); // 参数可以为NULL, 此时用来探测是否含有材质, 返回值决定了是否含有材质
  virtual GXHRESULT Clone                     (GVNode** ppClonedNode/*, GXBOOL bRecursive*/); // 写的不好，要重构，1.继承类应该可以直接使用基类的clone函数，对扩展的成员变量进行处理 2.支持递归

  virtual GXHRESULT SaveFileA(GXLPCSTR szFilename);
  virtual GXHRESULT SaveFileW(GXLPCWSTR szFilename);
  virtual GXHRESULT SaveFile (clSmartRepository* pStorage);

  //virtual void GetGeoDesc();
  inline void             SetName           (GXLPCSTR szName);
  inline GXLPCSTR         GetName           () const;
  inline GXDWORD          GetFlags          () const;
  inline const AABB&      GetLocalAABB      () const;
  inline GXVOID           SetLocalAABB      (const AABB& aabb);
  inline GXVOID           GetRelativeAABB   (AABB& aabb) const; // 子节点在父空间的AABB
  inline float4x4         GetRelativeMatrix () const;
  inline CFloat4x4&       GetAbsoluteMatrix () const;
  inline GXDWORD          GetClass          () const;
  inline GXBOOL           IsVisible         () const;
  inline const TRANSFORM& GetTransform      () const;
  inline GXDWORD          GetInterfaceCaps  () const;
  //inline TRANSFORM&       GetTransform      ();
};
//////////////////////////////////////////////////////////////////////////
//
//  GVNode::Release():
//    在程序退出时,如果没有释放这个节点,随后会在释放根节点的遍历中释放,所以不会造成内存泄漏.
//
//////////////////////////////////////////////////////////////////////////
//
// inline function
//
void GVNode::SetName(GXLPCSTR szName)
{
  m_strName = szName;
}

GXLPCSTR GVNode::GetName() const
{
  return m_strName;
}

GXDWORD GVNode::GetFlags() const
{
  return m_dwFlags;
}

const GVNode::AABB& GVNode::GetLocalAABB() const
{
  return m_aabbLocal;
}

GXVOID GVNode::SetLocalAABB(const AABB& aabb)
{
  m_aabbLocal = aabb;
}

GXVOID GVNode::GetRelativeAABB(AABB& aabb) const // 子节点在父空间的AABB
{
  //const float4& v4 = m_matRelative.GetRow(3);
  //float3 vTranslation(v4.x, v4.y, v4.z);
  //aabb = m_aabbLocal + vTranslation;
  aabb = m_aabbLocal + m_Transformation.translation;
}

float4x4 GVNode::GetRelativeMatrix() const
{
  ASSERT( ! TEST_FLAG(m_dwFlags, GVNF_PHYSICALBODY));
  return m_Transformation.ToRelativeMatrix();
}

const float4x4& GVNode::GetAbsoluteMatrix() const
{
  return m_Transformation.GlobalMatrix;
}

GXDWORD GVNode::GetClass() const
{
  return m_ClsCode;
}

GXBOOL GVNode::IsVisible() const
{
  return TEST_FLAG(m_dwFlags, GVNF_VISIBLE);
}

const GVNode::TRANSFORM& GVNode::GetTransform() const
{
  return m_Transformation;
}

GXDWORD GVNode::GetInterfaceCaps() const
{
  return m_dwInterfaceCaps;
}
//GVNode::TRANSFORM& GVNode::GetTransform()
//{
//  return m_Transformation;
//}

//////////////////////////////////////////////////////////////////////////
#endif // _GRAPVR_NODE_H_