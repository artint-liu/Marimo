#ifndef _GRAPVR_SCENE_H_
#define _GRAPVR_SCENE_H_

class GXGraphics;
class GVSceneMgr;
class GVPhySimulator;
class GVNode;
class GCamera;
class GXCanvas3D;
class GVSequence;
enum GVRenderType;
//namespace Marimo
//{
//  class Component;
//} // namespace Marimo

// GVScene ��Ҫ����Model���߼���ϵ, �߼�����
// GVSceneMgr ��Ҫ�ǳ�����Ⱦ����, ��ʾ����

struct GVSCENEUPDATE // Node update���������const����,���ܸı�Canvas��Camera��״̬
{
  GXCanvas3D* pCanvas3D;
  GCamera*    pCamera;        // ���Ǵ�����Canvas3D�еĶ���
  GXUINT      uDrawCallCount;
  GXDWORD     dwDeltaTime;    // ��ǰUpdate�����DeltaTime(��λ:ms)
  float       fDeltaTime;     // �����ǵ�DeltaTime(��λ:s)
  GXDWORD     dwAbsTime;      // ����ʱ��
};

class GXDLL GVScene : public GUnknown
{
  //using namespace Marimo;
  //typedef cllist<Component*> ComponentList;
  //typedef clmap<GVNode*, ComponentList> NodeComDict;
  enum ECmd {
    CMD_SetSelfTranslation, // �����ƽ��
    CMD_SetWorldTranslation,
    CMD_SetRotationgEuler,  // ŷ��
    CMD_SetRotationgQuat,   // ��Ԫ��
    CMD_SetScaling,
    CMD_SetSelfTransform,   // ռ���ĸ�CMDBUFFER
    CMD_SetWorldTransform,  // ռ���ĸ�CMDBUFFER
    CMD_Postfix,            // ǰ������ĺ�׺
  };

  struct CMDBUFFER
  {
    ECmd    eCmd;
    GVNode* pNode;
    float4  vParam;
  };

  typedef cllist<CMDBUFFER> CmdBufList;
private:
  GXGraphics*       m_pGraphics;
  GVSceneMgr*       m_pSceneMgr;
  clStringW         m_strDefaultMtl;
  GVPhySimulator*   m_pPhysWorld;
  GVNode*           m_pRoot;
  GXUINT            m_uDrawCallCount;
  GXDWORD           m_dwDeltaTime;    // ��ǰUpdate�����DeltaTime
  float             m_fDeltaTime;
  GXDWORD           m_dwAbsTime;
  CmdBufList        m_CmdBuffer;
  clstd::Locker     m_Locker;

  GVScene(GXGraphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl);
  virtual ~GVScene();

private:
  GXHRESULT RenderRecursive   (GXCanvas3D* pCanvas, GVNode* pParent, GVRenderType eType);
  GXBOOL    RayTraceRecursive (const NormalizedRay& ray, GVNode* pParent, CAABB* pAABB, GVNode** ppNode, float3* pHit);
  GXHRESULT UpdateRecursive   (const GVSCENEUPDATE& sContext, GVNode* pParent);
  GXHRESULT SaveFileRecursive (clFile* pFile, GVNode* pParent, GXINOUT u32& nVertBase);
  GXHRESULT IntExecuteCommand ();

public:
  static  GXHRESULT Create(GXGraphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl, GVScene** ppScene);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef                ();
  virtual GXHRESULT Release               ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXBOOL    RayTrace              (const Ray& ray, CAABB* pAABB, GVNode** ppModel, float3* pHit, GVNode* pParent = NULL);
  virtual GXBOOL    RayTraceFromViewport  (GXCanvas3D* pCanvas, const GXPOINT* pPoint, CAABB* pAABB, GVNode** ppNode, float3* pHit = NULL);
  virtual GXHRESULT SetPhysicalSimlator   (GVPhySimulator* pPhySimulator);
  virtual GVNode*   FindNodeUnsafe        (GXLPCSTR szName);

  virtual GXHRESULT SetNodePosition       (GVNode* pNode, CFloat3& vPos, clstd::ESpace eTransform = clstd::S_Self);
  virtual GXHRESULT SetNodeScaling        (GVNode* pNode, CFloat3& vScaling);
  virtual GXHRESULT SetNodeTransform      (GVNode* pNode, CFloat4x4& matTransform, clstd::ESpace eTransform = clstd::S_Self);

  GXGraphics* GetGraphicsUnsafe();

  GXBOOL    IsChild     (GVNode* pNode);
  GXHRESULT Add         (GVNode* pNode, GVNode* pParent = NULL);
  GXHRESULT Delete      (GVNode* pNode); // Model������GVScene��
  GXHRESULT RenderAll   (GXCanvas3D* pCanvas, GVRenderType eType);  // �����ü�,�Ὣ���������е����п���Ⱦ�ﶼ��Ⱦ����
  GXHRESULT SaveToFileW (GXLPCWSTR szFilename);
  GXHRESULT GetRoot     (GVNode** ppRootNode);
  
  // �ο� GVNodeFlags, ��� dwRequired ���� GVNode ���ر�־���Ӽ�, �򲻻ᱻ�ӵ���Ⱦ������
  // dwRequired ����ָ�� GVNF_VISIBLE ��־, �����־��Ĭ�ϵ�
  GXHRESULT Generate (GXCanvas3D* pCanvas, GVSequence* pRenderSequence, GVRenderType eType, GXDWORD dwRequired);
  void      Update      (GXCanvas3D* pCanvas, GXDWORD dwDeltaTime);

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