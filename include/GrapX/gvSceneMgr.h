// GrapVR 场景管理
// 开发初期使用线性场景划分
#ifndef _GRAPVR_SCENE_MANAGER_H_
#define _GRAPVR_SCENE_MANAGER_H_

class GVNode;

class GVSceneMgr : public GUnknown
{
  typedef clvector<GVNode*> GVNodeArray;
private:
  GrapX::Graphics* m_pGraphics;
  GVSceneMgr(GrapX::Graphics* pGraphics);
  virtual ~GVSceneMgr();

  GVNodeArray  m_aModel;

public:
  static GXHRESULT GXDLLAPI Create(GrapX::Graphics* pGraphics, GVSceneMgr** ppSceneMgr);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef    ();
  virtual GXHRESULT Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXHRESULT ManageNode   (GVNode* pNode);
  virtual GXHRESULT UnmanageNode (GVNode* pNode);

};

#endif // _GRAPVR_SCENE_MANAGER_H_