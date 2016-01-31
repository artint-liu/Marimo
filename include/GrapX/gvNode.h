// GrapVR �Ļ����������ļ�, ���������Ľڵ���ڴ˼̳�
#ifndef _GRAPVR_NODE_H_
#define _GRAPVR_NODE_H_

class GPrimitive;
class GVScene;
class GXCanvas3D;
class GXMaterialInst;
//class SmartRepository;
struct GVSCENEUPDATE;
//enum GXPRIMITIVETYPE;

//enum GVModelClass // TODO[9]: ��ΪGXDWORD Ȼ���� MAKEFOURCC ����
//{
//  GVMC_UNKNOWN,
//  GVMC_GEOMETRY,
//  GVMC_MESH,
//  GVMC_APPIMPL,   // Ӧ�ó���ʵ��,�û���չ
//};

// TODO[8]: Raytrace ��֧�ִ�WorldMat������

enum GVNodeFlags
{
  GVNF_VISIBLE        = 0x00000001,     // �û����õĿɼ���־, ���û�д˱�־�򲻲���ɼ��Լ��
  GVNF_NORAYTRACE     = 0x00000004,     // ����RayTrace�ӿڵ�̽��, Ӱ���Ӷ���
  GVNF_UPDATEWORLDMAT = 0x00000008,     // ����WorldMatrix, �����յ�λ�������
  GVNF_CONTAINER      = 0x00000010,     // ������Primitive���ݵ�����
  GVNF_NOCLIP         = 0x00000020,     // ��AABB����׶��Ԥ�ü�
  GVNF_PHYSICALBODY   = 0x00000040,     // ��������ϵͳ�Ľڵ�, λ�ÿ��ܲ��� Node �ĸ��ӹ�ϵӰ��
  GVNF_CASTSHADOW     = 0x00000080,     // ��ӰͶӰ��־
  
  //TODO: ��GVIC_UPDATE����
  //GVNF_DONOTUPDATE    = 0x00000100,     // ������ýڵ��Update�ӿڣ������־ֻ���ε�ǰ�ڵ�Update����Ӱ���ӽڵ�

  GVNF_STATIC         = 0x00000200,     // ��̬���壬��֧��SetPosition
  GVNF_COLLIDER       = 0x00000400,     // ����ײ


  GVNF_USER_0         = 0x01000000,     // �û������־
  GVNF_USER_1         = 0x02000000,
  GVNF_USER_2         = 0x04000000,
  GVNF_USER_3         = 0x08000000,
  GVNF_USER_4         = 0x10000000,
  GVNF_USER_5         = 0x20000000,
  GVNF_USER_6         = 0x40000000,
  GVNF_USER_7         = (int)0x80000000,

  GVNF_DEFAULT        = GVNF_VISIBLE|GVNF_CASTSHADOW,
};

// FIXME: �����û��ʼ��
// �ӿ�ʵ�̶ֳȱ�־���ڵ���Node�ӿ�ʱ�����Ӧ��־�������־Ϊ���򲻵���Node��virtual�ӿڡ�
// ����ӿ�ʵ���л�ȥ����Ӧ��־�������ڵ�һ�ε��ýӿں󲻻��ٵ����ˡ�
enum GVNodeInterfaceCaps
{
  GVIC_UPDATE    = 0x00000001,
  GVIC_NOTIFY    = 0x00000002,
  GVIC_COLLISION = 0x00000004,
};

enum GVRenderType
{
  GVRT_Normal  = 0x100, // ������ֲ�����, �����Ժ��һ��
  GVRT_Reflact = 0x200,
  GVRT_Reflect = 0x300,
  GVRT_Shadow  = 0x400,
};
//
// GVNode::SetMaterialInst ��
// GVNode::SetMaterialInstFromFileW ��־:
//
#define NODEMTL_SETCHILDREN   0x00000001  // ��������ڵ�Ĳ���
#define NODEMTL_SETSONONLY    0x00000002  // ����NODEMTL_SETCHILDREN��־, ֻ���ö���(��һ���ӽڵ�)�Ĳ���
#define NODEMTL_CLONEINST     0x00000004  // ��ԭ�в��ʿ�¡һ���µ�ʵ��
#define NODEMTL_IGNOREVERT    0x00000008  // �����������ʽ

enum NodeRayTraceType // �ཻ����, ���վ�ȷ�ȼ�����
{
  NRTT_NONE,
  NRTT_AABB,      // ��Χ���ཻ
  NRTT_MESHFACE,  // ��ȷ�ཻ
};

struct NODERAYTRACE
{
  NodeRayTraceType eType;
  float   fSquareDist;
  float3  vLocalHit;    // ģ�Ϳռ������
  int     nFaceIndex;   // ��һ������
};

struct NODENOTIFY
{
  GXUINT cbSize;
  GXDWORD code;
};

struct GVRENDERDESC
{
  GPrimitive*       pPrimitive;
  GXPrimitiveType   ePrimType;
  GXMaterialInst*   pMaterial;
  float4x4          matWorld;   // ȫ�ֱ任, ���Ӧ�÷���TRANSFORM::GlobalMatrix, ����ü���������

  GXDWORD dwFlags;              // �ο� GVModelFlags ����
  GXUINT  RenderQueue;          // ��Ⱦ����, ��δʹ��, ���Ĭ��ȡ��Material�еļ�¼
  GXINT   BaseVertexIndex;      // StartVertex
  GXUINT  MinIndex;
  GXUINT  NumVertices;
  GXUINT  StartIndex;
  GXUINT  PrimitiveCount;
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
  enum ESpace {
    S_ABSOLUTE = clstd::S_World,
    S_RELATIVE = clstd::S_Self,
    S_PHYSICALBODY,   // ���������Ϊ T_ABSOLUTE, ���������ӽڵ�λ��, ��Ծ��󱻺���
  };
protected:
  GXDWORD       m_ClsCode;
  clStringA     m_strName;
  GXDWORD       m_dwFlags;        // GVModelFlags
  GXDWORD       m_dwInterfaceCaps;
  AABB          m_aabbLocal;

  // TODO: ���ǲ��þ��������ƽ��/��ת/��������ʾ
  //float4x4      m_matRelative;    // ����ڸ��ڵ�ı任����
  //float4x4      m_matAbsolute;    // ���յı任����, m_matRelative * pParent->m_matAbsolute
  TRANSFORM     m_Transformation;
  // Coordinate
  virtual    ~GVNode();
private:
  void    UpdateChildPos();
protected:
  virtual GXHRESULT LoadFileA(GXGraphics* pGraphics, GXLPCSTR szFilename);
  virtual GXHRESULT LoadFileW(GXGraphics* pGraphics, GXLPCWSTR szFilename);
  virtual GXHRESULT LoadFile (GXGraphics* pGraphics, clSmartRepository* pStorage);
public:
  //GVModel   (GVModelClass eClass);
  GVNode   (GVScene* pScene, GXDWORD dwClassCode);

  GXBOOL    Show                      (GXBOOL bShow);
  GXDWORD   SetFlags                  (GXDWORD dwNewFlags); // �����±�־, ��������ԭ���ı�־
  GXDWORD   CombineFlags              (GXDWORD dwFlags);    // ��ԭ�б�־�ϲ�
//void      CombineRecursiveFlags    (GXDWORD dwFlags);
//void      RemoveRecursiveFlags     (GXDWORD dwFlags);
  GXVOID    GetAbsoluteAABB           (AABB& aabb) const;

  void      SetScaling                (CFloat3& vScaling/*, ESpace eTransform = S_RELATIVE*/);

  void      SetRotationA              (CFloat3& vEuler);  // Angle
  void      SetRotationR              (CFloat3& vEuler);  // Radian
  void      SetRotation               (CQuaternion& quater);
  void      RotateA                   (CFloat3& vEuler);
  void      RotateR                   (CFloat3& vEuler);
  void      Rotate                    (CQuaternion& quater);

  void      Move                      (CFloat3& vDelta);
  float3    GetPosition               (ESpace eTransform = S_RELATIVE);
  void      SetPosition               (CFloat3& vPos, ESpace eTransform = S_RELATIVE);

  void      SetTransform              (const float4x4& matTransform, ESpace eTransform = S_RELATIVE);
  void      SetTransform              (const float3* pScaling, const quaternion* pRotation, const float3* pTranslation); // ����ΪNULL��ʹ������ķ���
  void      SetTransformR             (const float3* pScaling, const float3* pEuler, const float3* pTranslation); // ����ΪNULL��ʹ������ķ���

  GXBOOL    SetDirection              (CFloat3& vDir, CFloat3& vUp/*, ESpace eTransform = S_RELATIVE*/);
  GVNode*   FindChild                 (GVNode* pStart, GXLPCSTR szName);
  GVNode*   SetParent                 (GVNode* pNewParent, bool bLast = false);
  GXHRESULT SetMaterialInst           (GXMaterialInst* pMtlInst, GXDWORD dwFlags);  // �ο� NODEMTL_* ��־
  GXHRESULT SetMaterialInstFromFileW  (GXGraphics* pGraphics, GXLPCWSTR szFilename, GXDWORD dwFlags);  // �ο� NODEMTL_* ��־

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef    ();
  virtual GXHRESULT Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXVOID    OnNotify                  (const NODENOTIFY* pNotify);
  virtual GXBOOL    Update                    (const GVSCENEUPDATE& sContext);
  virtual GXBOOL    Collision                 ();
  virtual GXVOID    CalculateAABB             ();
  virtual void      GetRenderDesc             (GVRenderType eType, GVRENDERDESC* pRenderDesc);
  virtual GXBOOL    RayTrace                  (const Ray& ray, NODERAYTRACE* pRayTrace);
  virtual GXHRESULT SetMaterialInstDirect     (GXMaterialInst* pMtlInst);
  virtual GXHRESULT GetMaterialInst           (GXMaterialInst** ppMtlInst);
  virtual GXHRESULT GetMaterialInstFilenameW  (clStringW* pstrFilename); // ��������ΪNULL, ��ʱ����̽���Ƿ��в���, ����ֵ�������Ƿ��в���
  virtual GXHRESULT Clone                     (GVNode** ppClonedNode/*, GXBOOL bRecursive*/); // д�Ĳ��ã�Ҫ�ع���1.�̳���Ӧ�ÿ���ֱ��ʹ�û����clone����������չ�ĳ�Ա�������д��� 2.֧�ֵݹ�

  virtual GXHRESULT SaveFileA(GXLPCSTR szFilename);
  virtual GXHRESULT SaveFileW(GXLPCWSTR szFilename);
  virtual GXHRESULT SaveFile (clSmartRepository* pStorage);

  //virtual void GetGeoDesc();
  inline void             SetName           (GXLPCSTR szName);
  inline GXLPCSTR         GetName           () const;
  inline GXDWORD          GetFlags          () const;
  inline const AABB&      GetLocalAABB      () const;
  inline GXVOID           SetLocalAABB      (const AABB& aabb);
  inline GXVOID           GetRelativeAABB   (AABB& aabb) const; // �ӽڵ��ڸ��ռ��AABB
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
//    �ڳ����˳�ʱ,���û���ͷ�����ڵ�,�������ͷŸ��ڵ�ı������ͷ�,���Բ�������ڴ�й©.
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

const AABB& GVNode::GetLocalAABB() const
{
  return m_aabbLocal;
}

GXVOID GVNode::SetLocalAABB(const AABB& aabb)
{
  m_aabbLocal = aabb;
}

GXVOID GVNode::GetRelativeAABB(AABB& aabb) const // �ӽڵ��ڸ��ռ��AABB
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