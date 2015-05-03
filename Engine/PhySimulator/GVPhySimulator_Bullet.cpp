
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
// Bullet 物理库的引擎必须放在最前面
// 在01024左右的版本发现由于某处没有查出的定义, 导致 new btBoxShape 分配的字节数与
// Bullet内部初始化的字节数不一致, 这导致了最后free时不断报内存写入越界的问题.
// 由于Bullet头文件比较复杂且很难定位错误, 尝试将其头文件放在最前面才解决这个问题.
// 可以确定的是Marimo中的某个宏定义导致了Bullet btBoxShape长度的变化.
// 在实际调试过程中也发现sizeof(btBoxShape)有时为64字节, 有时会显示80字节.


#include <GrapX.H>
#include <GUnknown.H>
#include <GXKernel.H>
#include <GXUser.H>
#include <GVPhySimulator.h>
#include <gxError.h>
#include "3D/GrapVR.H"

#include "GameEngine.h"
#include "GVPhySimulator_Bullet.h"

#define MAX_LOADSTRING 100
#define MAX_CONTACTS 8          // maximum number of contact points per body
#define DENSITY 5.0f

#if defined(_X86)
#ifdef _DEBUG
#pragma comment(lib, "BulletDynamics_vs2010_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_debug.lib")
#else
#pragma comment(lib, "BulletDynamics_vs2010.lib")
#pragma comment(lib, "BulletCollision_vs2010.lib")
#pragma comment(lib, "LinearMath_vs2010.lib")
#endif // #ifdef _DEBUG
#elif defined(_X64)
#ifdef _DEBUG
#pragma comment(lib, "BulletDynamics_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_x64_debug.lib")
#else
#pragma comment(lib, "BulletDynamics_vs2010_x64_release.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_release.lib")
#pragma comment(lib, "LinearMath_vs2010_x64_release.lib")
#endif // #ifdef _DEBUG
#endif // #if defined(_X86)

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVRigidBodyBt::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVRigidBodyBt::Release()
{
  const GXLONG nRef = gxInterlockedDecrement(&m_nRefCount);
  if(nRef == 0) {
    delete this;
    return GX_OK;
  }
  return nRef;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXVOID GVRigidBodyBt::SetLinearVelocity(CFloat3& vVelocity)
{
  if(body != 0) {
    body->setLinearVelocity(btVector3(vVelocity.x, vVelocity.y, vVelocity.z));
  }
}

GXVOID GVRigidBodyBt::SetAngularVelocity(CFloat3& vVelocity)
{
  if(body != 0) {
    body->setAngularVelocity(btVector3(vVelocity.x, vVelocity.y, vVelocity.z));
  }
}

GXVOID GVRigidBodyBt::Translate( CFloat3& vPos )
{
  if(body) {
    body->translate(btVector3(vPos.x, vPos.y, vPos.z));
  }
}

//GVNode* GVRigidBodyBt::Destroy()
//{
//  GVNode* pNode = NULL;
//  if (body != NULL)
//  {
//    if(body->getMotionState())
//    {
//      delete body->getMotionState();
//    }
//    pNode = (GVNode*)body->getUserPointer();
//    m_pSimulator->m_dynamicsWorld->removeCollisionObject(body);
//    delete body;
//  }
//  if(shape != NULL)
//  {
//    if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
//    {
//      btTriangleMesh* trimesh = (btTriangleMesh*)shape->getUserPointer();
//      delete trimesh;
//      shape->setUserPointer(NULL);
//    }
//  }
//  delete shape;
//  return pNode;
//}


GVNode* GVPhySimulatorBt::RIGID::Destroy(btDynamicsWorld* dynamicsWorld)
{
  GVNode* pNode = NULL;

  if (body != NULL)
  {
    if(body->getMotionState())
    {
      delete body->getMotionState();
    }
    pNode = (GVNode*)body->getUserPointer();
    dynamicsWorld->removeCollisionObject(body);
    delete body;
  }
  if(shape != NULL)
  {
    if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
    {
      btTriangleMesh* trimesh = (btTriangleMesh*)shape->getUserPointer();
      delete trimesh;
      shape->setUserPointer(NULL);
    }
  }
  delete shape;
  return pNode;
}

GVNode* GVPhySimulatorBt::COLLISION::Destroy( btCollisionWorld* pCollisionWorld )
{
  GVNode* pNode = NULL;

  if (object != NULL)
  {
    pNode = (GVNode*)object->getUserPointer();
    pCollisionWorld->removeCollisionObject(object);
    delete object;
  }
  if(shape != NULL)
  {
    if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
    {
      btTriangleMesh* trimesh = (btTriangleMesh*)shape->getUserPointer();
      delete trimesh;
      shape->setUserPointer(NULL);
    }
  }
  delete shape;
  return pNode;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVPhySimulatorBt::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT GVPhySimulatorBt::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount > 0) {
    return nRefCount;
  }
  FinalizeBullet();
  delete this;
  return GX_OK;
}

GXBOOL GVPhySimulatorBt::InitializeDynamics(float fGravity, GXBOOL bAsync)
{
  ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
  m_pCollisionConfigurationDyn = new btDefaultCollisionConfiguration();

  ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
  m_pDispatcherDyn = new	btCollisionDispatcher(m_pCollisionConfigurationDyn);

  ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  m_pOverlappingPairCache = new btDbvtBroadphase();

  ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
  m_pSolver = new btSequentialImpulseConstraintSolver;

  m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pDispatcherDyn, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfigurationDyn);

  m_pDynamicsWorld->setGravity(btVector3(0,fGravity,0));

  // 异步模式
  if(bAsync) {
    m_pSimuThread = new SimulationThread(this);
    m_pSimuThread->Start();
      //static_cast<SimulationThread*>(SimulationThread::CreateThread(SimulationProc, this));
    m_pLocker = new clstd::Locker;
  }

  return TRUE;
}

GXBOOL GVPhySimulatorBt::InitializeCollision()
{
  m_pCollisionConfigurationCln = new btDefaultCollisionConfiguration();
  m_pDispatcherCln = new btCollisionDispatcher(m_pCollisionConfigurationCln);
  btVector3	worldAabbMin(-1000,-1000,-1000);
  btVector3	worldAabbMax(1000,1000,1000);

  m_pBroadphase = new btAxisSweep3(worldAabbMin,worldAabbMax);

  //SimpleBroadphase is a brute force alternative, performing N^2 aabb overlap tests
  //SimpleBroadphase*	broadphase = new btSimpleBroadphase;

  m_pCollisionWorld = new btCollisionWorld(m_pDispatcherCln, m_pBroadphase, m_pCollisionConfigurationCln);
  return TRUE;
}

GXBOOL GVPhySimulatorBt::FinalizeBullet()
{
  // 先停止异步线程后才能在本地线程安全释放 Bullet 对象
  if(m_pSimuThread != NULL)
  {
    m_pSimuThread->PostQuitMessage(0);
    m_pSimuThread->WaitThreadQuit(-1);
    delete m_pSimuThread;
    m_pSimuThread = NULL;
  }
  SAFE_DELETE(m_pLocker);
  // </停止异步线程>

  for(NodeRigidDict::iterator it = m_RigidDict.begin();
    it != m_RigidDict.end(); ++it)
  {
    RIGID& rigid = it->second;
    GVNode* pNode = it->first;
    GVNode* pBodyNode = rigid.Destroy(m_pDynamicsWorld);
    SAFE_RELEASE(pNode);
  }
  m_RigidDict.clear();

  for(RigidArray::iterator it = m_aVirtualRigid.begin();
    it != m_aVirtualRigid.end(); ++it)
  {
    RIGID& rigid = *it;
    rigid.Destroy(m_pDynamicsWorld);
  }
  m_aVirtualRigid.clear();

  //remove the rigidbodies from the dynamics world and delete them
  for (int i=m_pDynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
  {
    btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getMotionState())
    {
      delete body->getMotionState();
    }
    m_pDynamicsWorld->removeCollisionObject( obj );
    delete obj;
  }

  //delete collision shapes
  //for (int j=0;j<m_collisionShapes.size();j++)
  //{
  //  btCollisionShape* shape = m_collisionShapes[j];
  //  m_collisionShapes[j] = 0;
  //  delete shape;
  //}

  //delete dynamics world
  SAFE_DELETE(m_pDynamicsWorld);
  SAFE_DELETE(m_pSolver);
  SAFE_DELETE(m_pOverlappingPairCache);
  SAFE_DELETE(m_pDispatcherDyn);
  SAFE_DELETE(m_pCollisionConfigurationDyn);


  //
  // 碰撞检测部分
  //
  for(NodeCollisionDict::iterator it = m_CollisionDict.begin();
    it != m_CollisionDict.end(); ++it)
  {
    COLLISION& coll = it->second;
    GVNode* pNode = it->first;
    GVNode* pBodyNode = coll.Destroy(m_pCollisionWorld);
    SAFE_RELEASE(pNode);
  }
  m_CollisionDict.clear();


  for(CollisionArray::iterator it = m_aVirtualCollision.begin();
    it != m_aVirtualCollision.end(); ++it) {
    COLLISION& coll = *it;
    GVNode* pNode = coll.Destroy(m_pCollisionWorld);
    SAFE_RELEASE(pNode);
  }
  m_aVirtualCollision.clear();

  for (int i = m_pCollisionWorld->getNumCollisionObjects() - 1; i >= 0; i--)
  {
    btCollisionObject* obj = m_pCollisionWorld->getCollisionObjectArray()[i];
    m_pCollisionWorld->removeCollisionObject( obj );
    delete obj;
  }

  SAFE_DELETE(m_pCollisionConfigurationCln);
  SAFE_DELETE(m_pDispatcherCln);
  SAFE_DELETE(m_pBroadphase);
  SAFE_DELETE(m_pCollisionWorld);


  //next line is optional: it will be cleared by the destructor when the array goes out of scope
  //m_collisionShapes.clear();


  return TRUE;
}

GXHRESULT GVPhySimulatorBt::DeleteNode(GVNode* pNode)
{
  NodeRigidDict::iterator it = m_RigidDict.find(pNode);
  if(it == m_RigidDict.end()) {
    return FALSE;
  }
  clstd::ScopedSafeLocker lock(m_pLocker);

  RIGID& rigid = it->second;
  GVNode* pBodyNode = rigid.Destroy(m_pDynamicsWorld);
  ASSERT(pBodyNode == NULL || pBodyNode == pNode);
  pNode->SetFlags(pNode->GetFlags() & (~GVNF_PHYSICALBODY));
  SAFE_RELEASE(pNode);

  m_RigidDict.erase(it);
  return TRUE;
}

GXBOOL GVPhySimulatorBt::SetLinearVelocity(GVNode* pNode, CFloat3& vVelocity)
{
  NodeRigidDict::iterator it = m_RigidDict.find(pNode);
  if(it == m_RigidDict.end()) {
    return FALSE;
  }
  clstd::ScopedSafeLocker lock(m_pLocker);

  if(it->second.body != 0) {
    it->second.body->setLinearVelocity(btVector3(vVelocity.x, vVelocity.y, vVelocity.z));
  }
  return TRUE;
}

GXBOOL GVPhySimulatorBt::SetAngularVelocity(GVNode* pNode, CFloat3& vVelocity)
{
  NodeRigidDict::iterator it = m_RigidDict.find(pNode);
  if(it == m_RigidDict.end()) {
    return FALSE;
  }
  clstd::ScopedSafeLocker lock(m_pLocker);

  if(it->second.body != 0) {
    it->second.body->setAngularVelocity(btVector3(vVelocity.x, vVelocity.y, vVelocity.z));
  }
  return TRUE;
}

GXBOOL GVPhySimulatorBt::Translate( GVNode* pNode, CFloat3& v )
{
  GXDWORD dwFlags = pNode->GetFlags();
  if(TEST_FLAG(dwFlags, GVNF_COLLIDER)) {
    NodeCollisionDict::iterator it = m_CollisionDict.find(pNode);
    if(it == m_CollisionDict.end()) {
      return FALSE;
    }
    clstd::ScopedSafeLocker lock(m_pLocker);

    if(it->second.object != 0) {
      it->second.object->getWorldTransform().setOrigin(BTV3(v));
    }

  }
  else if(TEST_FLAG(dwFlags, GVNF_PHYSICALBODY)) {
    NodeRigidDict::iterator it = m_RigidDict.find(pNode);
    if(it == m_RigidDict.end()) {
      return FALSE;
    }
    clstd::ScopedSafeLocker lock(m_pLocker);

    if(it->second.body != 0) {
      it->second.body->setActivationState(DISABLE_DEACTIVATION);
      it->second.body->getWorldTransform().setOrigin(BTV3(v));
      it->second.body->forceActivationState(ACTIVE_TAG);
      it->second.body->setDeactivationTime( 0.f );
    }
  }
  else { return FALSE; }
  return TRUE;
}

GXBOOL GVPhySimulatorBt::SetForce(GVNode* pNode, CFloat3& vForce)
{
  NodeRigidDict::iterator it = m_RigidDict.find(pNode);
  if(it == m_RigidDict.end()) {
    return FALSE;
  }

  //if(it->second.body != 0) {
  //  dBodyAddForce(it->second.body, vForce.x, vForce.y, vForce.z);
  //}
  return TRUE;
}

GXBOOL GVPhySimulatorBt::CreateRigidBody(RIGID& GXINOUT rigid, btScalar mass, const btTransform& startTransform)
{
  btVector3 localInertia(0,0,0);
  const GXBOOL bDynamic = (mass != 0.f);

  if (bDynamic) {
    rigid.shape->calculateLocalInertia(mass,localInertia);
  }

  //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
  btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, rigid.shape, localInertia);
  rigid.body = new btRigidBody(rbInfo);
  
  clstd::ScopedSafeLocker lock(m_pLocker);
  m_pDynamicsWorld->addRigidBody(rigid.body);
  return TRUE;
}

GXHRESULT GVPhySimulatorBt::AddAsBox(GVNode* pNode, GXBOOL bStatic)
{
  RIGID rigid;
  AABB aabb;
  float3 vCenter;
  float3 vExtent;
  pNode->GetAbsoluteAABB(aabb);
  vCenter = aabb.GetCenter();
  vExtent = aabb.GetExtent();

  rigid.shape = new btBoxShape(btVector3(vExtent.x, vExtent.y, vExtent.z));

  // Create Dynamic Objects
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setOrigin(btVector3(vCenter.x, vCenter.y, vCenter.z));
  CreateRigidBody(rigid, bStatic ? 0.f : 1.f, startTransform);

  rigid.body->setUserPointer(pNode);

  DeleteNode(pNode);
  pNode->CombineFlags(GVNF_PHYSICALBODY);
  m_RigidDict[pNode] = rigid;
  pNode->AddRef();

  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::AddAsCapsule( GVNode* pNode, CFloat3& vLocalOrigin, float fRadius, float fHeight, GXBOOL bStatic )
{
  float3 vOrigin = pNode->GetPosition() + vLocalOrigin;
  RIGID rigid;
  rigid.shape = new btCapsuleShape(fRadius, fHeight - fRadius * 2.0f);

  // Create Dynamic Objects
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setOrigin(BTV3(vOrigin));
  CreateRigidBody(rigid, bStatic ? 0.f : 1.f, startTransform);

  rigid.body->setUserPointer(pNode);

  DeleteNode(pNode);
  pNode->CombineFlags(GVNF_PHYSICALBODY);
  m_RigidDict[pNode] = rigid;
  pNode->AddRef();
  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::AddAsMesh(GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic)
{
  RIGID rigid;
  if(nVertexCount == 0 || nIndexCount == 0) {
    DeleteNode(pNode);
    return GX_OK;
  }

  btTriangleMesh* trimesh = new btTriangleMesh();
  trimesh->preallocateVertices(nIndexCount);  // 顶点数和索引数一致
  trimesh->preallocateIndices(nIndexCount);

  const int nFaceCount = nIndexCount / 3;
  for (int i = 0; i < nFaceCount; i++)
  {
    const int index0 = (int)pIndices[i*3];
    const int index1 = (int)pIndices[i*3+1];
    const int index2 = (int)pIndices[i*3+2];

    const float3* v0 = (float3*)((GXLPBYTE)lpVertex + nStride * index0);
    const float3* v1 = (float3*)((GXLPBYTE)lpVertex + nStride * index1);
    const float3* v2 = (float3*)((GXLPBYTE)lpVertex + nStride * index2);

    btVector3 vertex0(v0->x, v0->y, v0->z);
    btVector3 vertex1(v1->x, v1->y, v1->z);
    btVector3 vertex2(v2->x, v2->y, v2->z);

    trimesh->addTriangle(vertex0,vertex1,vertex2);
  }

  btTransform startTransform;
  startTransform.setIdentity();

  rigid.shape = new btBvhTriangleMeshShape(trimesh, false);
  rigid.shape->setUserPointer(trimesh);
  
  CreateRigidBody(rigid, bStatic ? 0.f : 1.f, startTransform);

  ASSERT(rigid.shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE);
  rigid.body->setUserPointer(pNode);

  DeleteNode(pNode);
  pNode->CombineFlags(GVNF_PHYSICALBODY);
  m_RigidDict[pNode] = rigid;
  pNode->AddRef();

  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::AddPlane(const Plane& plane)
{
  btTransform startTransform;
  Plane p = Plane::normalize(plane);
  RIGID rigid;
  btCollisionShape* groundShape = new btStaticPlaneShape(
    btVector3(plane.vNormal.x, plane.vNormal.y, plane.vNormal.z), plane.fDist);
  rigid.shape = groundShape;
  startTransform.setIdentity();
  CreateRigidBody(rigid, 0.0f, startTransform);
  m_aVirtualRigid.push_back(rigid);
  return GX_OK;
}

//////////////////////////////////////////////////////////////////////////

GXHRESULT GVPhySimulatorBt::CDAddAsSphere( GVNode* pNode, float fRadius, GXBOOL bStatic )
{
  AABB aabb;
  float3 vCenter;
  //float3 vExtent;
  pNode->GetAbsoluteAABB(aabb);
  vCenter = aabb.GetCenter();
  //vExtent = aabb.GetExtent();

  COLLISION coll;
  coll.shape = new btSphereShape(fRadius);
  coll.object = new btCollisionObject;
  coll.object->setCollisionShape(coll.shape);
  coll.object->getWorldTransform().setOrigin(BTV3(vCenter));
  coll.object->setUserPointer(pNode);
  m_pCollisionWorld->addCollisionObject(coll.object);
  pNode->CombineFlags(GVNF_COLLIDER);
  m_CollisionDict[pNode] = coll;
  pNode->AddRef();
  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::CDAddAsBox( GVNode* pNode, GXBOOL bStatic )
{
  AABB aabb;
  float3 vCenter;
  float3 vExtent;
  pNode->GetAbsoluteAABB(aabb);
  vCenter = aabb.GetCenter();
  vExtent = aabb.GetExtent();

  COLLISION coll;
  coll.shape = new btBoxShape(BTV3(vExtent));
  coll.object = new btCollisionObject;
  coll.object->setCollisionShape(coll.shape);
  coll.object->getWorldTransform().setOrigin(BTV3(vCenter));
  coll.object->setUserPointer(pNode);
  m_pCollisionWorld->addCollisionObject(coll.object);
  pNode->CombineFlags(GVNF_COLLIDER);
  m_CollisionDict[pNode] = coll;
  pNode->AddRef();
  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::CDAddAsCapsule( GVNode* pNode, CFloat3& vLocalOrigin, float fRadius, float fHeight, GXBOOL bStatic )
{
  float3 vOrigin = pNode->GetPosition() + vLocalOrigin;
  COLLISION coll;
  coll.shape = new btCapsuleShape(fRadius, fHeight - fRadius * 2.0f);
  coll.object = new btCollisionObject;
  coll.object->setCollisionShape(coll.shape);
  coll.object->getWorldTransform().setOrigin(BTV3(vOrigin));
  coll.object->setUserPointer(pNode);
  m_pCollisionWorld->addCollisionObject(coll.object);
  pNode->CombineFlags(GVNF_COLLIDER);
  m_CollisionDict[pNode] = coll;
  pNode->AddRef();
  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::CDAddAsMesh( GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic )
{
  return GX_OK;
}

GXHRESULT GVPhySimulatorBt::CDAddPlane(const Plane& plane)
{
  btTransform startTransform;
  COLLISION coll;
  btCollisionShape* groundShape = new btStaticPlaneShape(BTV3(plane.vNormal), plane.fDist);
  coll.shape = groundShape;
  coll.object = new btCollisionObject;
  coll.object->setCollisionShape(coll.shape);
  m_pCollisionWorld->addCollisionObject(coll.object);
  m_aVirtualCollision.push_back(coll);
  return GX_OK;
}
//////////////////////////////////////////////////////////////////////////

GXHRESULT GVPhySimulatorBt::Simulation(float fStep)
{
  if(m_pLocker != NULL)
  {
    // 异步模式下物理仿真在另外的线程中进行
    clstd::ScopedLocker lock(m_pLocker);
    m_fStep = fStep;
    DeployNodes();
  }
  else
  {
    m_pDynamicsWorld->stepSimulation(fStep);
    DeployNodes();
  }

  return GX_OK;
}

GXBOOL GVPhySimulatorBt::DeployNodes() // 部署节点的位置
{
  btScalar	m[16];
  btVector3 vMin, vMax;
  btTransform trans;

  for (int i = m_pDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
  {
    btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getMotionState())
    {
      GVNode* pNode = (GVNode*)body->getUserPointer();
      if(pNode == NULL)
        continue;

      body->getMotionState()->getWorldTransform(trans);
      body->getAabb(vMin, vMax);

      trans.getOpenGLMatrix(m);

      float4x4 matTransform(
            m[ 0], m[ 4], m[ 8], 0.0f,
            m[ 1], m[ 5], m[ 9], 0.0f,
            m[ 2], m[ 6], m[10], 0.0f,
            m[12], m[13], m[14], 1.0f);

      pNode->SetLocalAABB(AABB(
        float3(vMin.getX() - m[12], vMin.getY() - m[13], vMin.getZ() - m[14]), 
        float3(vMax.getX() - m[12], vMax.getY() - m[13], vMax.getZ() - m[14])));
      pNode->SetTransform(matTransform, GVNode::S_PHYSICALBODY);
    }
  }
  return TRUE;
}

GXHRESULT GVPhySimulatorBt::IntCreateBulletSimulator(float fGravity, GXBOOL bAsync, GVPhySimulator** ppPhySimulator)
{
  GVPhySimulatorBt* pPhySimulator = new GVPhySimulatorBt;
  if(pPhySimulator == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pPhySimulator->AddRef();

  if( ! pPhySimulator->InitializeBullet(fGravity, bAsync)) {
    pPhySimulator->Release();
    pPhySimulator = NULL;
  }

  *ppPhySimulator = pPhySimulator;
  return GX_OK;

}

GXBOOL GVPhySimulatorBt::InitializeBullet( float fGravity, GXBOOL bAsync )
{
  return InitializeDynamics(fGravity, bAsync) && InitializeCollision();
}

GXBOOL GVPhySimulatorBt::CollisionDetection(GXBOOL bCollisionDetection)
{
  if( ! m_pCollisionWorld) {
    return FALSE;
  }

  m_pCollisionWorld->performDiscreteCollisionDetection();

  if(bCollisionDetection)
  {
    int numManifolds = m_pCollisionWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
      btPersistentManifold* contactManifold = m_pCollisionWorld->getDispatcher()->getManifoldByIndexInternal(i);
      const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
      const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

      int numContacts = contactManifold->getNumContacts();
      for (int j=0;j<numContacts;j++)
      {
        btManifoldPoint& pt = contactManifold->getContactPoint(j);

        GVNode* pNode0 = reinterpret_cast<GVNode*>(contactManifold->getBody0()->getUserPointer());
        GVNode* pNode1 = reinterpret_cast<GVNode*>(contactManifold->getBody1()->getUserPointer());

        btVector3 ptA = pt.getPositionWorldOnA();
        btVector3 ptB = pt.getPositionWorldOnB();

        btVector3 d = ptA - ptB;
        //TRACE("%f,%f,%f\n", d.x(), d.y(), d.z());

      }

      //you can un-comment out this line, and then all points are removed
      //contactManifold->clearManifold();	
    }
  }
  return TRUE;
}

struct MOContactCallback : public btCollisionWorld::ContactResultCallback
{
  float fDist;
  float3 vNormal;
  float3 vDir;
  GXBOOL bRet;

  MOContactCallback()
    : fDist(0.0f)
    , vDir(0.0f)
    , bRet(FALSE)
  {
  }

  virtual btScalar addSingleResult(btManifoldPoint& cp,
    const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
    const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
  {
    btVector3 v = cp.m_localPointA - cp.m_localPointB;
    btScalar l = v.length();
    fDist = cp.getDistance();
    if(fDist < 0.0f)
    {
      vNormal.set(cp.m_normalWorldOnB.x(), cp.m_normalWorldOnB.y(), cp.m_normalWorldOnB.z());
      //TRACE("v:%f,%f,%f  %f\n", cp.m_normalWorldOnB.x(), cp.m_normalWorldOnB.y(), cp.m_normalWorldOnB.z(), fDist);
      vDir += vNormal * fDist;
      bRet = TRUE;
    }
    return 0;
  }
};

GXBOOL GVPhySimulatorBt::ContactTest( GVNode* pNode, float3* vDist )
{
  MOContactCallback sCallback;
  auto it = m_CollisionDict.find(pNode);
  if(it != m_CollisionDict.end())
  {
    //TRACE("contactTest\n");
    m_pCollisionWorld->contactTest(it->second.object, sCallback);
    *vDist = sCallback.vDir;//sCallback.fDist * sCallback.vNormal;
  }
  return sCallback.bRet;
}

GXBOOL GVPhySimulatorBt::ContactPairTest( GVNode* pNodeA, GVNode* pNodeB )
{
  MOContactCallback sCallback;
  auto itA = m_CollisionDict.find(pNodeA);
  auto itB = m_CollisionDict.find(pNodeB);
  if(itA != m_CollisionDict.end() && itB != m_CollisionDict.end())
  {
    m_pCollisionWorld->contactPairTest(itA->second.object, itB->second.object, sCallback);
    return TRUE;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//u32 CL_CALLBACK GVPhySimulatorBt::SimulationProc(CLMTCREATESTRUCT* pParam)
i32 SimulationThread::Run()
{
  //GVPhySimulatorBt* pThis    = static_cast<GVPhySimulatorBt*>(pParam->pUserParam);
  //SimulationThread* pContext = static_cast<SimulationThread*>(pParam->pThis);

  clstd::THREADMSG msg;
  const u32 nTimeOut = 20;
  //const float fTimeScale = 0.01f;
  //GXDWORD dwTick = gxGetTickCount();
  while(i32 ret = GetMessageTimeOut(&msg, nTimeOut))
  {
    if(ret == -1)
    {
      const GXDWORD dwCurTick = gxGetTickCount();
      //const GXDWORD dwDelta = dwCurTick - dwTick;
      clstd::ScopedLocker lock(m_pSimulator->m_pLocker);
      m_pSimulator->m_pDynamicsWorld->stepSimulation(m_pSimulator->m_fStep, 0);
      m_pSimulator->m_fStep = 0;
      //dwTick = dwCurTick;
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern "C"
{
  GAMEENGINE_API GXHRESULT CreateBulletSimulator(float fGravity, GXBOOL bAsync, GVPhySimulator** ppPhySimulator)
  {
    return GVPhySimulatorBt::IntCreateBulletSimulator(fGravity, bAsync, ppPhySimulator);
  }
} // extern "C"
