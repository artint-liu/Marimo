#ifndef _GRAPVR_PHYSICAL_SIMULATOR_INTERFACE_H_
#define _GRAPVR_PHYSICAL_SIMULATOR_INTERFACE_H_
//
// 这个文件只在GrapX库中声明了物理引擎的接口，以便Scene使用
// 实现方法则放在了Engine库中
//

class GVNode;

class GVRigidBody : public GUnknown
{
public:
  //GVRigidBody();
  //~GVRigidBody();
  GXSTDINTERFACE(GXHRESULT AddRef   ());
  GXSTDINTERFACE(GXHRESULT Release  ());

  GXSTDINTERFACE(GXVOID SetLinearVelocity  (CFloat3& vVelocity));
  GXSTDINTERFACE(GXVOID SetAngularVelocity (CFloat3& vVelocity));
  GXSTDINTERFACE(GXVOID Translate          (CFloat3& vPos));
};

//GVRigidBody::GVRigidBody()
//{
//}
//
//GVRigidBody::~GVRigidBody()
//{
//}

class GVPhySimulator : public GUnknown
{
public:
  //enum Rigid
public:
  GXSTDINTERFACE(GXHRESULT AddRef   ());
  GXSTDINTERFACE(GXHRESULT Release  ());

  GXSTDINTERFACE(GXHRESULT DeleteNode(GVNode* pNode));
  GXSTDINTERFACE(GXBOOL    CollisionDetection(GXBOOL bCollisionDetection)); // 执行碰撞检测前务必调用这个函数，bCollisionDetection表示是否立即检测contact point
  GXSTDINTERFACE(GXBOOL    ContactTest(GVNode* pNode, float3* vDist));
  GXSTDINTERFACE(GXBOOL    ContactPairTest(GVNode* pNodeA, GVNode* pNodeB));
  GXSTDINTERFACE(GXHRESULT Simulation(float fStep));

  // 暂定接口
  GXSTDINTERFACE(GXBOOL    SetLinearVelocity  (GVNode* pNode, CFloat3& vVelocity));
  GXSTDINTERFACE(GXBOOL    SetAngularVelocity (GVNode* pNode, CFloat3& vVelocity));
  GXSTDINTERFACE(GXBOOL    Translate          (GVNode* pNode, CFloat3& v));
  GXSTDINTERFACE(GXBOOL    SetForce           (GVNode* pNode, CFloat3& vForce));
  GXSTDINTERFACE(GXHRESULT AddAsBox           (GVNode* pNode, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT AddAsCapsule       (GVNode* pNode, CFloat3& vLocalOrigin, float fRadius, float fHeight, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT AddAsMesh          (GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT AddPlane           (const Plane& plane));
  GXSTDINTERFACE(GXHRESULT CDAddAsSphere      (GVNode* pNode, float fRadius, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT CDAddAsBox         (GVNode* pNode, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT CDAddAsCapsule     (GVNode* pNode, CFloat3& vLocalOrigin, float fRadius, float fHeight, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT CDAddAsMesh        (GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic));
  GXSTDINTERFACE(GXHRESULT CDAddPlane         (const Plane& plane));
private:
  //GVPhysiceWorld();
  //~GVPhysiceWorld();

};

//GXAPI GXHRESULT CreateODESimulator(float fGravity, GVPhysicalWorld** ppPhysWorld);

//GVPhysiceWorld::GVPhysiceWorld()
//{
//}
//
//GVPhysiceWorld::~GVPhysiceWorld()
//{
//}

#endif // _GRAPVR_PHYSICAL_SIMULATOR_INTERFACE_H_