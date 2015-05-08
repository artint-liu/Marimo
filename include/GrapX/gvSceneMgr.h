// GrapVR ��������
// ��������ʹ�����Գ�������
#ifndef _GRAPVR_SCENE_MANAGER_H_
#define _GRAPVR_SCENE_MANAGER_H_

class GVNode;

class GVSceneMgr : public GUnknown
{
  typedef clvector<GVNode*> GVNodeArray;
private:
  GXGraphics* m_pGraphics;
  GVSceneMgr(GXGraphics* pGraphics);
  virtual ~GVSceneMgr();

  GVNodeArray  m_aModel;

public:
  static GXHRESULT Create(GXGraphics* pGraphics, GVSceneMgr** ppSceneMgr);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef    ();
  virtual GXHRESULT Release   ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  virtual GXHRESULT ManageNode   (GVNode* pNode);
  virtual GXHRESULT UnmanageNode (GVNode* pNode);

};

#endif // _GRAPVR_SCENE_MANAGER_H_