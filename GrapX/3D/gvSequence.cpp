#include "GrapX.H"

#include "clTree.H"
#include "clTransform.h"

#include "GrapX/GResource.H"
#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.H"
#include "GrapX/GXKernel.H"
#include "GrapX/gxError.H"
#include "GrapX/gvNode.h"
#include "GrapX/GVSequence.h"

//////////////////////////////////////////////////////////////////////////
GVSequence::GVSequence()
{
  //InlSetZeroT(m_aRenderQueueSlot);
}

GVSequence::~GVSequence()
{
  Clear();
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVSequence::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVSequence::Release()
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

void GVSequence::Clear()
{
  for(int i = 0; i < c_nNeedRefactorRenderDescCount; i++)
  {
    for(auto it = m_aRenderDesc[i].begin();
      it != m_aRenderDesc[i].end(); ++it)
    {
      SAFE_RELEASE(it->pPrimitive);
      SAFE_RELEASE(it->pMaterial);
    }
    m_aRenderDesc[i].clear();
  }

}

int GVSequence::GetArrayCount()
{
  return c_nNeedRefactorRenderDescCount;
}

const GVSequence::RenderDescArray& GVSequence::GetArray(int nIndex)
{
  ASSERT(nIndex >= 0 && nIndex < c_nNeedRefactorRenderDescCount);
  return m_aRenderDesc[nIndex];
}

int GVSequence::Add(GVRENDERDESC* pDesc)
{
  int nRenderQueue = pDesc->RenderQueue & 0xffff;
  ASSERT(nRenderQueue >= 0 && nRenderQueue <= (int)RenderQueue_Max);

  if(pDesc->pPrimitive) {
    pDesc->pPrimitive->AddRef();
  }
  ASSERT(pDesc->pMaterial);
  pDesc->pMaterial->AddRef();

  if(nRenderQueue < RenderQueue_Background) {
    m_aRenderDesc[0].push_back(*pDesc);
  }
  else if(nRenderQueue < RenderQueue_Geometry) {
    m_aRenderDesc[1].push_back(*pDesc);
  }
  else if(nRenderQueue < RenderQueue_AlphaTest) {
    m_aRenderDesc[2].push_back(*pDesc);
  }
  else if(nRenderQueue < RenderQueue_Transparent) {
    m_aRenderDesc[3].push_back(*pDesc);
  }
  else if(nRenderQueue < RenderQueue_Overlay) {
    m_aRenderDesc[4].push_back(*pDesc);
  }
  else if(nRenderQueue < RenderQueue_Max) {
    m_aRenderDesc[5].push_back(*pDesc);
  }

  return 0;
}

GXHRESULT GVSequence::Create(GVSequence** ppSequence)
{
  GVSequence* pSequence = new GVSequence;
  if(pSequence == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  pSequence->AddRef();
  *ppSequence = pSequence;
  return GX_OK;
}

//////////////////////////////////////////////////////////////////////////

int GXDLLAPI MOParseRenderQueue( GXLPCSTR szName )
{
  // 解析简单的RenderQueue表达式
  // 包括：
  // @.直接指定数字
  // @.渲染队列的枚举
  // @.枚举+数字的加减表达式
  if(szName[0] >= '0' && szName[0] <= '9') {
    return GXATOI(szName);
  }

  clStringA str = szName;
  int nQueue = (int)RenderQueue_Geometry;
  str.MakeUpper();
  if(str.BeginsWith("BACKGROUND")) {
    nQueue = RenderQueue_Background;
  }  
  else if(str.BeginsWith("GEOMETRY")) {
    nQueue = RenderQueue_Geometry;
  }
  else if(str.BeginsWith("ALPHATEST")) {
    nQueue = RenderQueue_AlphaTest;
  }
  else if(str.BeginsWith("TRANSPARENT")) {
    nQueue = RenderQueue_Transparent;
  }
  else if(str.BeginsWith("OVERLAY")) {
    nQueue = RenderQueue_Overlay;
  }
  else if(str.BeginsWith("MAX")) {
    nQueue = RenderQueue_Max;
  }

  size_t p = str.FindAny("+-");
  if(p != clStringA::npos) {
    str.Remove(0, p);
    if(str[0] == '+') {
      ASSERT(str[0] == '+');
      str.TrimLeft("+ \t");
      nQueue += str.ToInteger();
    }
    else {
      ASSERT(str[0] == '-');
      str.TrimLeft("+ \t");
      nQueue -= str.ToInteger();
    }
  }
  return nQueue;
}
