#ifndef _GRAPVR_SCENE_H_
#define _GRAPVR_SCENE_H_

//class GXGraphics;
class GVSceneMgr;
class GVPhySimulator;
class GVNode;
class GVSequence;
//enum GVRenderType;

namespace GrapX
{
  class Graphics;
  class Camera;
  class Canvas3D;
} // namespace GrapX
//namespace Marimo
//{
//  class Component;
//} // namespace Marimo

// GVScene 主要储存Model的逻辑关系, 逻辑管理
// GVSceneMgr 主要是场景渲染管理, 显示管理

struct GVSCENEUPDATE // Node update中这个具有const限制,不能改变Canvas和Camera的状态
{
  GrapX::Canvas3D* pCanvas3D;
  GrapX::Camera*    pCamera;        // 就是储存在Canvas3D中的对象
  GXUINT      uDrawCallCount;
  GXDWORD     dwDeltaTime;    // 当前Update传入的DeltaTime(单位:ms)
  float       fDeltaTime;     // 浮点标记的DeltaTime(单位:s)
  GXDWORD     dwAbsTime;      // 绝对时间
};

class GXDLL GVScene : public GUnknown
{
  //using namespace Marimo;
  //typedef cllist<Component*> ComponentList;
  //typedef clmap<GVNode*, ComponentList> NodeComDict;
  enum ECmd {
    CMD_SetSelfTranslation, // 自身的平移
    CMD_SetWorldTranslation,
    CMD_SetRotationgEuler,  // 欧拉
    CMD_SetRotationgQuat,   // 四元数
    CMD_SetScaling,
    CMD_SetSelfTransform,   // 占用四个CMDBUFFER
    CMD_SetWorldTransform,  // 占用四个CMDBUFFER
    CMD_Postfix,            // 前面命令的后缀
  };

  struct CMDBUFFER
  {
    ECmd    eCmd;
    GVNode* pNode;
    float4  vParam;
  };

  typedef cllist<CMDBUFFER> CmdBufList;
private:
  GrapX::Graphics*  m_pGraphics;
  GVSceneMgr*       m_pSceneMgr;
  clStringW         m_strDefaultMtl;
  GVPhySimulator*   m_pPhysWorld;
  GVNode*           m_pRoot;
  GXUINT            m_uDrawCallCount;
  GXDWORD           m_dwDeltaTime;    // 当前Update传入的DeltaTime
  float             m_fDeltaTime;
  GXDWORD           m_dwAbsTime;
  CmdBufList        m_CmdBuffer;
  clstd::Locker     m_Locker;

  GVScene(GrapX::Graphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl);
  virtual ~GVScene();

private:
  GXHRESULT RenderRecursive   (GrapX::Canvas3D* pCanvas, GVNode* pParent, int nRenderCate);
  GXBOOL    RayTraceRecursive (const GVNode::NormalizedRay& ray, GVNode* pParent, const GVNode::AABB* pAABB, GVNode** ppNode, float3* pHit);
  GXHRESULT UpdateRecursive   (const GVSCENEUPDATE& sContext, GVNode* pParent);
  GXHRESULT SaveFileRecursive (clFile* pFile, GVNode* pParent, GXINOUT u32& nVertBase);
  GXHRESULT IntExecuteCommand ();

public:
  static  GXHRESULT Create(GrapX::Graphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl, GVScene** ppScene);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef                ();
  virtual GXHRESULT Release               ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXBOOL    RayTrace              (const GVNode::Ray& ray, const GVNode::AABB* pAABB, GVNode** ppModel, float3* pHit, GVNode* pParent = NULL);
  virtual GXBOOL    RayTraceFromViewport  (GrapX::Canvas3D* pCanvas, const GXPOINT* pPoint, const GVNode::AABB* pAABB, GVNode** ppNode, float3* pHit = NULL); // ppNode 不会增加引用
  virtual GXHRESULT SetPhysicalSimlator   (GVPhySimulator* pPhySimulator);
  virtual GVNode*   FindNodeUnsafe        (GXLPCSTR szName);

  virtual GXHRESULT SetNodePosition       (GVNode* pNode, CFloat3& vPos, clstd::ESpace eTransform = clstd::S_Self);
  virtual GXHRESULT SetNodeScaling        (GVNode* pNode, CFloat3& vScaling);
  virtual GXHRESULT SetNodeTransform      (GVNode* pNode, CFloat4x4& matTransform, clstd::ESpace eTransform = clstd::S_Self);

  GrapX::Graphics* GetGraphicsUnsafe();

  GXBOOL    IsChild     (GVNode* pNode);
  GXHRESULT Add         (GVNode* pNode, GVNode* pParent = NULL);
  GXHRESULT Delete      (GVNode* pNode); // Model必须在GVScene中
  GXHRESULT RenderAll   (GrapX::Canvas3D* pCanvas, int nRenderCate);  // 不做裁剪,会将整个场景中的所有可渲染物都渲染出来
  GXHRESULT SaveToFileW (GXLPCWSTR szFilename);
  GXHRESULT GetRoot     (GVNode** ppRootNode);
  
  // 参考 GVNodeFlags, 如果 dwRequired 不是 GVNode 返回标志的子集, 则不会被加到渲染队列中
  // dwRequired 不必指定 GVNF_VISIBLE 标志, 这个标志是默认的
  // dwCullingMask 剔除掩码, 默认是0, (dwCullingMask & GVNode::m_dwLayer) != 0 则不进行收集
  GXHRESULT Generate    (GrapX::Canvas3D* pCanvas, GVSequence* pRenderSequence, int nRenderCate, GXDWORD dwLayerMask, GXDWORD dwRequired);
  void      Update      (GrapX::Canvas3D* pCanvas, GXDWORD dwDeltaTime);

  inline GXDWORD        GetDeltaTime  ();
  inline float          GetDeltaTimeF ();
  inline GXDWORD        GetAbsTime    ();
  inline clstd::Locker* GetLocker     ();
};

inline GXDWORD GVScene::GetDeltaTime()
{
  return m_dwDeltaTime;
}

inline GXDWORD GVScene::GetAbsTime()
{
  return m_dwAbsTime;
}

inline float GVScene::GetDeltaTimeF()
{
  return m_fDeltaTime;
}

inline clstd::Locker* GVScene::GetLocker()
{
  return &m_Locker;
}

#endif // _GRAPVR_SCENE_H_