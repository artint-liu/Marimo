#include "GrapX.h"

#include "clTree.h"
#include "clTransform.h"

#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.h"
#include "GrapX/GXKernel.h"
#include "GrapX/gxError.h"
#include "GrapX/gvNode.h"
#include "GrapX/GVSequence.h"

//////////////////////////////////////////////////////////////////////////
GVSequence::GVSequence()
{
  //InlSetZeroT(m_aRenderQueueSlot);
}

GVSequence::~GVSequence()
{
  Clear(DefaultRenderCategory);
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

void GVSequence::Clear(int nRenderCate)
{
  for(int i = 0; i < c_nNeedRefactorRenderDescCount; i++)
  {
    //for(GVRENDERDESC2* pRenderer : m_aRenderDesc[i])
    //{
    //  SAFE_RELEASE(desc.pPrimitive);
    //  SAFE_RELEASE(desc.pMaterial);
    //}
    m_aRenderDesc[i].clear();
  }
  m_nRenderCate = nRenderCate;
}

int GVSequence::GetArrayCount()
{
  return c_nNeedRefactorRenderDescCount;
}

int GVSequence::GetRenderCategory() const
{
  return m_nRenderCate;
}

const GVSequence::RenderDescArray& GVSequence::GetArray(int nIndex)
{
  ASSERT(nIndex >= 0 && nIndex < c_nNeedRefactorRenderDescCount);
  return m_aRenderDesc[nIndex];
}

int GVSequence::Add(GVRENDERDESC2* pDesc)
{
  //int nRenderQueue = pDesc->RenderQueue & 0xffff;
  int nRenderQueue = pDesc->materials[m_nRenderCate]->GetRenderQueue();
  ASSERT(nRenderQueue >= 0 && nRenderQueue <= (int)GrapX::RenderQueue_Max);

  if(pDesc->pPrimitive) {
    //pDesc->pPrimitive->AddRef();
  }

  //ASSERT(pDesc->pMaterial);
  //pDesc->pMaterial->AddRef();

  if(nRenderQueue < GrapX::RenderQueue_Background) {
    m_aRenderDesc[0].push_back(pDesc);
  }
  else if(nRenderQueue < GrapX::RenderQueue_Geometry) {
    m_aRenderDesc[1].push_back(pDesc);
  }
  else if(nRenderQueue < GrapX::RenderQueue_AlphaTest) {
    m_aRenderDesc[2].push_back(pDesc);
  }
  else if(nRenderQueue < GrapX::RenderQueue_Transparent) {
    m_aRenderDesc[3].push_back(pDesc);
  }
  else if(nRenderQueue < GrapX::RenderQueue_Overlay) {
    m_aRenderDesc[4].push_back(pDesc);
  }
  else if(nRenderQueue < GrapX::RenderQueue_Max) {
    m_aRenderDesc[5].push_back(pDesc);
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
  int nQueue = (int)GrapX::RenderQueue_Geometry;
  str.MakeUpper();
  if(str.BeginsWith("BACKGROUND")) {
    nQueue = GrapX::RenderQueue_Background;
  }  
  else if(str.BeginsWith("GEOMETRY")) {
    nQueue = GrapX::RenderQueue_Geometry;
  }
  else if(str.BeginsWith("ALPHATEST")) {
    nQueue = GrapX::RenderQueue_AlphaTest;
  }
  else if(str.BeginsWith("TRANSPARENT")) {
    nQueue = GrapX::RenderQueue_Transparent;
  }
  else if(str.BeginsWith("OVERLAY")) {
    nQueue = GrapX::RenderQueue_Overlay;
  }
  else if(str.BeginsWith("MAX")) {
    nQueue = GrapX::RenderQueue_Max;
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
