#ifndef _GRAPV_PHYSICAL_WORLD_BULLET_IMPL_H_
#define _GRAPV_PHYSICAL_WORLD_BULLET_IMPL_H_

struct TRIMESH  // TODO: 优化内存使用
{
  //dTriMeshDataID  Data;
  VIndex32*       pIndices32;
  float3*         pVertices;
};


#define BTV3(v) btVector3(v.x, v.y, v.z)

class GVPhySimulatorBt;

class SimulationThread : public clstd::MsgThreadT<clstd::THREADMSG>
{
  GVPhySimulatorBt* m_pSimulator;
public:
  SimulationThread(GVPhySimulatorBt* pSmlt)
    : m_pSimulator(pSmlt)
  {
  }
  i32 Run();
};

class GVRigidBodyBt : public GVRigidBody
{
  friend class GVPhySimulatorBt;
private:
  GVPhySimulatorBt* m_pSimulator;
  btRigidBody*      body;
  btCollisionShape* shape;

public:
  GVRigidBodyBt(GVPhySimulatorBt* pSimulator)
    : m_pSimulator(pSimulator), body(0), shape(0)  {}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT AddRef   ();
  GXHRESULT Release  ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXVOID SetLinearVelocity  (CFloat3& vVelocity);
  GXVOID SetAngularVelocity (CFloat3& vVelocity);
  GXVOID Translate          (CFloat3& vPos);

  //GVNode* Destroy();
};

class GVPhySimulatorBt : public GVPhySimulator
{
  friend class GVRigidBodyBt;
  friend class SimulationThread;

  struct RIGID{
    btRigidBody*      body;
    btCollisionShape* shape;
    GVNode* Destroy(btDynamicsWorld* dynamicsWorld);
  };
  struct COLLISION{
    btCollisionObject* object;
    btCollisionShape*  shape;
    GVNode* Destroy(btCollisionWorld* pCollisionWorld);
  };
  typedef clhash_map<GVNode*, RIGID> NodeRigidDict;
  typedef clvector<RIGID>   RigidArray;
  typedef clhash_map<GVNode*, COLLISION> NodeCollisionDict;
  typedef clvector<COLLISION>   CollisionArray;
public:
  GVPhySimulatorBt();
  ~GVPhySimulatorBt();

  virtual GXHRESULT AddRef();
  virtual GXHRESULT Release();

  virtual GXHRESULT Simulation(float fStep);
  virtual GXBOOL    CollisionDetection(GXBOOL bCollisionDetection);
  virtual GXBOOL    ContactTest(GVNode* pNode, float3* vDist);
  virtual GXBOOL    ContactPairTest(GVNode* pNodeA, GVNode* pNodeB);
  virtual GXHRESULT DeleteNode(GVNode* pNode);

  virtual GXBOOL    SetLinearVelocity  (GVNode* pNode, CFloat3& vVelocity);
  virtual GXBOOL    SetAngularVelocity (GVNode* pNode, CFloat3& vVelocity);
  virtual GXBOOL    Translate          (GVNode* pNode, CFloat3& v);
  virtual GXBOOL    SetForce           (GVNode* pNode, CFloat3& vForce);

  virtual GXHRESULT AddAsBox      (GVNode* pNode, GXBOOL bStatic);
  virtual GXHRESULT AddAsCapsule  (GVNode* pNode, CFloat3& vOrigin, float fRadius, float fHeight, GXBOOL bStatic);
  virtual GXHRESULT AddAsMesh     (GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic);
  virtual GXHRESULT AddPlane      (const Plane& plane);

  virtual GXHRESULT CDAddAsSphere   (GVNode* pNode, float fRadius, GXBOOL bStatic);
  virtual GXHRESULT CDAddAsBox      (GVNode* pNode, GXBOOL bStatic);
  virtual GXHRESULT CDAddAsCapsule  (GVNode* pNode, CFloat3& vOrigin, float fRadius, float fHeight, GXBOOL bStatic);
  virtual GXHRESULT CDAddAsMesh     (GVNode* pNode, GXLPCVOID lpVertex, int nStride, int nVertexCount, VIndex* pIndices, int nIndexCount, GXBOOL bConvex, GXBOOL bStatic);
  virtual GXHRESULT CDAddPlane      (const Plane& plane);

  static GXHRESULT IntCreateBulletSimulator(float fGravity, GXBOOL bAsync, GVPhySimulator** ppPhySimulator);
  static u32 CL_CALLBACK SimulationProc(CLMTCREATESTRUCT* pParam);
private:
  GXBOOL InitializeBullet(float fGravity, GXBOOL bAsync);
  GXBOOL InitializeDynamics(float fGravity, GXBOOL bAsync);
  GXBOOL InitializeCollision();
  GXBOOL FinalizeBullet ();
  GXBOOL DeployNodes  ();

  GXBOOL CreateRigidBody(RIGID& GXINOUT rigid, btScalar mass, const btTransform& startTransform);

private:
  //GXBOOL                                m_bDbgCollision; // 测试用，物理仿真/碰撞检测切换
  // 仿真系统用
  btDefaultCollisionConfiguration*      m_pCollisionConfigurationDyn;
  btCollisionDispatcher*                m_pDispatcherDyn;
  btBroadphaseInterface*                m_pOverlappingPairCache;
  btSequentialImpulseConstraintSolver*  m_pSolver;

  // 碰撞检测系统用
  btDefaultCollisionConfiguration*      m_pCollisionConfigurationCln;
  btCollisionDispatcher*                m_pDispatcherCln;
  btAxisSweep3*	                        m_pBroadphase;

  btDiscreteDynamicsWorld*              m_pDynamicsWorld;    // 物理仿真
  btCollisionWorld*	                    m_pCollisionWorld;   // 碰撞检测
  SimulationThread*                     m_pSimuThread;
  clstd::Locker*                        m_pLocker;
  float                                 m_fTimeScale;
  volatile float                        m_fStep;
  //btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
  NodeRigidDict                         m_RigidDict;
  RigidArray                            m_aVirtualRigid;      // 虚拟物体，没有对应Node

  NodeCollisionDict                     m_CollisionDict;
  CollisionArray                        m_aVirtualCollision;  // 虚拟碰撞物，没有对应Node
};

GVPhySimulatorBt::GVPhySimulatorBt()
  : m_pSimuThread (NULL)
  , m_pLocker     (NULL)
  , m_fTimeScale  (0.001f)

  , m_pCollisionConfigurationDyn(NULL)
  , m_pDispatcherDyn(NULL)
  , m_pOverlappingPairCache(NULL)
  , m_pSolver(NULL)

  , m_pCollisionConfigurationCln(NULL)
  , m_pDispatcherCln(NULL)
  , m_pBroadphase(NULL)

  , m_fStep       (0)
  , m_pDynamicsWorld (NULL)
  , m_pCollisionWorld(NULL)
  //, m_bDbgCollision(FALSE)
{
}

GVPhySimulatorBt::~GVPhySimulatorBt()
{
}

#endif // #ifndef _GRAPV_PHYSICAL_WORLD_BULLET_IMPL_H_