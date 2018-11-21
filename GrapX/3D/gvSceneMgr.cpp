// ȫ��ͷ�ļ�
#include <GrapX.h>

// ��׼�ӿ�
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXKernel.h"

// ˽��ͷ�ļ�
//#include <vector>
//#include <clstd/clTree.h>

//#include <3D/GVNode.h>
//#include <3D/gvMesh.h>
#include "GrapX/gvSceneMgr.h"
#include "GrapX/GrapVR.h"

GVSceneMgr::GVSceneMgr(GrapX::Graphics* pGraphics)
  : m_pGraphics(pGraphics)
{
}

GVSceneMgr::~GVSceneMgr()
{
  for(GVNodeArray::iterator it = m_aModel.begin();
    it != m_aModel.end(); ++it)
  {
    SAFE_RELEASE(*it);
  }
  m_aModel.clear();
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVSceneMgr::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVSceneMgr::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXHRESULT GVSceneMgr::Create(GrapX::Graphics* pGraphics, GVSceneMgr** ppSceneMgr)
{

  GVSceneMgr* pSceneMgr = new GVSceneMgr(pGraphics);
  GXHRESULT hr = pSceneMgr->AddRef();
  if(GXSUCCEEDED(hr))
  {
    *ppSceneMgr = pSceneMgr;
  }
  else
  {
    pSceneMgr->Release();
  }
  return hr;
}

GXHRESULT GVSceneMgr::ManageNode(GVNode* pNode)
{
  m_aModel.push_back(pNode);
  pNode->AddRef();
  return (GXHRESULT)m_aModel.size();
}

GXHRESULT GVSceneMgr::UnmanageNode(GVNode* pNode)
{
  for(GVNodeArray::iterator it = m_aModel.begin();
    it != m_aModel.end(); ++it)
  {
    if(*it == pNode) {
      m_aModel.erase(it);
      SAFE_RELEASE(pNode);
      return GX_OK;
    }
  }
  return GX_FAIL;
}