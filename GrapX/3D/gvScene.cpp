// 全局头文件
#include <GrapX.h>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GTexture.h"
#include "GrapX/GVPhySimulator.h"
#include "GrapX/GCamera.h"
#include "GrapX/GXKernel.h"

// 私有头文件
#include "clTree.h"
#include "clTransform.h"

#include "GrapX/GXCanvas3D.h"
#include "GrapX/VertexDecl.h"

#include <GrapX/gvNode.h>
#include <GrapX/gvMesh.h>
#include <GrapX/gvScene.h>
#include <GrapX/gvSceneMgr.h>
#include <GrapX/gvSequence.h>
#include "GrapX/gxUtility.h"

using namespace GrapX;
//#define DEFAULT_MTL_FILENAME "shaders\\Diffuse_shader.txt"

typedef clstack<GVNode*> GVNodeStack;

GVScene::GVScene(GrapX::Graphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl)
  : m_pGraphics     (pGraphics)
  , m_pSceneMgr     (pSceneMgr)
  , m_strDefaultMtl (szDefaultMtl)
  , m_pPhysWorld    (NULL)
  , m_pRoot         (NULL)
  , m_uDrawCallCount(0)
  , m_dwDeltaTime   (0)
  , m_fDeltaTime    (0)
  , m_dwAbsTime     (0)
  //, m_pLocker       (NULL)
{
  m_pGraphics->AddRef();
  m_pSceneMgr->AddRef();
  
  m_pRoot = new GVNode(this, GXMAKEFOURCC('R','O','O','T'));
  m_pRoot->AddRef();

  //m_pLocker = new clstd::Locker;
}

GVScene::~GVScene()
{
  //SAFE_DELETE(m_pLocker);
  SAFE_RELEASE(m_pPhysWorld);
  SAFE_RELEASE(m_pSceneMgr);
  SAFE_RELEASE(m_pGraphics);
  SAFE_RELEASE(m_pRoot);
}

//////////////////////////////////////////////////////////////////////////

GXHRESULT GVScene::Create(GrapX::Graphics* pGraphics, GVSceneMgr* pSceneMgr, GXLPCWSTR szDefaultMtl, GVScene** ppScene)
{
  GVSceneMgr* _pSceneMgr = NULL;
  if(pSceneMgr == NULL)
  {
    GVSceneMgr::Create(pGraphics, &pSceneMgr);
    _pSceneMgr = pSceneMgr;
  }
  GVScene* pScene = new GVScene(pGraphics, pSceneMgr, szDefaultMtl);
  if( ! InlCheckNewAndIncReference(pScene))
  {
    return GX_FAIL;
  }

  *ppScene = pScene;
  SAFE_RELEASE(_pSceneMgr);
  return GX_OK;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVScene::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVScene::Release()
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

//////////////////////////////////////////////////////////////////////////

GrapX::Graphics* GVScene::GetGraphicsUnsafe()
{
  return m_pGraphics;
}

GXBOOL GVScene::IsChild(GVNode* pNode)
{
  if(pNode == NULL) {
    return FALSE;
  }
  GVNode* pParentModel = pNode;
  do {
    if(pParentModel == m_pRoot) {
      return TRUE;
    }
    pParentModel = (GVNode*)pParentModel->GetParent();
  }while(pParentModel != NULL);

  return FALSE;
}

GXHRESULT GVScene::Add(GVNode* pNode, GVNode* pParent)
{
  clstd::ScopedSafeLocker lock(&m_Locker);

  // 加入前先检查节点和子节点的材质是否为空
  // 如果先加入到场景树中会遍历兄弟节点,这是不希望的.
  //const clStringW strFilename = DEFAULT_MTL_FILENAME;
  GVNode* pTraversal = pNode;
  GVNodeStack NodeStack;

  while(1)
  {
    while(pTraversal) {

      //
      // Set MtlInst if not empty
      //
      GVRENDERDESC sDesc;
      pTraversal->GetRenderDesc(GVRT_Normal, &sDesc);

      GXBOOL bHasMesh = sDesc.pPrimitive != NULL;

      if(bHasMesh && GXFAILED(pTraversal->GetMaterialFilename(NULL))) {
        pTraversal->SetMaterialFromFile(m_pGraphics, m_strDefaultMtl, NULL);
          //FALSE, MLT_REFERENCE);
      }

      if(pTraversal->GetFirstChild()) {
        NodeStack.push(pTraversal->GetNext());
        pTraversal = pTraversal->GetFirstChild();
        continue;
      }

      pTraversal = pTraversal->GetNext();
    };
    if(NodeStack.empty()) break;

    pTraversal = NodeStack.top();
    NodeStack.pop();
  }

  // 加入到场景树中
  if(pNode->GetParent() == NULL)
  {
    pNode->SetParent(pParent != NULL ? pParent : m_pRoot);
  }
  else
  {
    if(m_pRoot->IsAncestorOf(pNode))
    {
      return GX_OK;
    }
    else
    {
      CLOG_ERROR("%s : Node has been added another scene.!\n", __FUNCTION__);
      return GX_FAIL;
    }    
  }

  pNode->AddRef();
  return m_pSceneMgr->ManageNode(pNode);
}

GXHRESULT GVScene::Delete(GVNode* pNode)
{
  if(pNode == NULL) {
    return GX_FAIL;
  }

  clstd::ScopedSafeLocker lock(&m_Locker);

  if( ! IsChild(pNode)) {
    return GX_FAIL;
  }

  pNode->SetParent(NULL);
  m_pSceneMgr->UnmanageNode(pNode);
  if(m_pPhysWorld != NULL && TEST_FLAG(pNode->GetFlags(), GVNF_PHYSICALBODY))
  {
    m_pPhysWorld->DeleteNode(pNode);
  }
  SAFE_RELEASE(pNode);
  return GX_OK;
}

GXBOOL GVScene::RayTraceRecursive(const GVNode::NormalizedRay& ray, GVNode* pParent, const GVNode::AABB* pAABB, GVNode** ppNode, float3* pHit)
{
  GVNode* pNode = (GVNode*)pParent->GetFirstChild();
  GVNode* pHitNode = NULL;
  GVNode::AABB aabbNode;
  GXBOOL result = FALSE;
  NODERAYTRACE nrt;
  NODERAYTRACE nrtTest;
  nrt.eType = NRTT_NONE;
  nrt.fSquareDist = FLT_MAX;
  nrtTest.fSquareDist = FLT_MAX;

  clstack<GVNode*>      NodeStack;

  clstd::ScopedSafeLocker lock(&m_Locker);

  while(true)
  {
    while(pNode != NULL) {
      const GXDWORD dwNodeFlags = pNode->GetFlags();

      // 不进行Raytrace或者不可见
      if(TEST_FLAG(dwNodeFlags, GVNF_NORAYTRACE) || TEST_FLAG_NOT(dwNodeFlags, GVNF_VISIBLE))
      {
        pNode = pNode->GetNext();
        //continue;
      }

      // 如果仅是容器，则直接转入子节点的测试
      else if(TEST_FLAG(dwNodeFlags, GVNF_CONTAINER) && pNode->GetFirstChild() != NULL)
      {
        NodeStack.push(pNode->GetNext());
        pNode = pNode->GetFirstChild();
        //continue;
      }

      else
      {
        // 如果限制了AABB范围，则在指定范围内的Node才会做RayTrace测试
        if(pAABB) {
          pNode->GetAbsoluteAABB(aabbNode);
          if( ! IntersectAABBAABB(*pAABB, aabbNode)) {
            pNode = pNode->GetNext();
            continue;
          }
        }

        // 精确测试
        if(pNode->RayTrace(ray, &nrtTest)) {
          
          // 1.如果相交的精确等级更高，则直接选择这个Node
          // 2.精确等级相同则按照距离判断
          if(nrtTest.eType > nrt.eType ||
            (nrtTest.eType == nrt.eType && nrtTest.fSquareDist < nrt.fSquareDist)) {

              pHitNode = pNode;
              nrt = nrtTest;
              result = TRUE;
              ASSERT(pHitNode);
          }
        }

        if(pNode->GetFirstChild()) {
          NodeStack.push(pNode->GetNext());
          pNode = pNode->GetFirstChild();
        }
        else {
          pNode = pNode->GetNext();
        }
      }

    } // while

    if(NodeStack.empty()) {
      break;
    }

    pNode = NodeStack.top();
    NodeStack.pop();

  } // while(true)


  if(ppNode != NULL) {
    *ppNode = pHitNode;
  }

  if(pHit != NULL) {
    *pHit = nrt.vLocalHit;
  }
  return result;
}

GXBOOL GVScene::RayTrace(const GVNode::Ray& ray, const GVNode::AABB* pAABB, GVNode** ppModel, float3* pHit, GVNode* pParent)
{
  return RayTraceRecursive(ray, pParent ? pParent : m_pRoot , pAABB, ppModel, pHit);
}

GXBOOL GVScene::RayTraceFromViewport(GrapX::Canvas3D* pCanvas, const GXPOINT* pPoint, const GVNode::AABB* pAABB, GVNode** ppModel, float3* pHit)
{
  float3 vPos;
  pCanvas->PositionFromScreen(pPoint, 1.0f - 1e-5f, &vPos);
  GrapX::Camera* pCamera = pCanvas->GetCameraUnsafe();
  const float3 vCameraPos = pCamera->GetPos();

  clstd::ScopedSafeLocker lock(&m_Locker);
  return RayTraceRecursive(GVNode::Ray(vCameraPos, vPos - vCameraPos), m_pRoot, pAABB, ppModel, pHit);
}

GXHRESULT GVScene::SetPhysicalSimlator(GVPhySimulator* pPhySimulator)
{
  return InlSetNewObjectT(m_pPhysWorld, pPhySimulator);
}

GVNode* GVScene::FindNodeUnsafe(GXLPCSTR szName)
{
  return m_pRoot->FindChild(NULL, szName);
}

GXHRESULT GVScene::UpdateRecursive(const GVSCENEUPDATE& sContext, GVNode* pParent)
{
  GVNode* pNode = (GVNode*)pParent->GetFirstChild();
  while(pNode)
  {
    // 如果通过可见性决定是否更新,会导致skeleton无法更新,因为它可能是不可见的
    GXBOOL bUpdateChild = pNode->Update(sContext);
    
    // 根据返回值确定是否Update子节点
    if(bUpdateChild && pNode->GetFirstChild()) {
      UpdateRecursive(sContext, pNode);
    }

    pNode = pNode->GetNext();
  }
  return GX_OK;
}

GXHRESULT GVScene::RenderRecursive(GrapX::Canvas3D* pCanvas, GVNode* pParent, GVRenderType eType)
{
  GVRENDERDESC Desc;
  GVNode* pModel = pParent->GetFirstChild();

  GrapX::Graphics* pGraphics = pCanvas->GetGraphicsUnsafe();

  while(pModel != NULL)
  {
    pModel->GetRenderDesc(eType, &Desc);

    if( ! TEST_FLAG(Desc.dwFlags, GVNF_VISIBLE)) {
      // 准备下一次循环
      pModel = pModel->GetNext();
      continue;
    }

    if(TEST_FLAG_NOT(Desc.dwFlags, GVNF_CONTAINER) && Desc.PrimitiveCount != 0)
    {
      if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
        pCanvas->SetWorldMatrix(Desc.matWorld);
      }

      // 应用材质
      if(Desc.pMaterial != NULL) {
        pCanvas->SetMaterial(Desc.pMaterial);
      }
      //else {
      //  ASSERT(0);
      //}

      ASSERT(Desc.pPrimitive != NULL);
      pGraphics->SetPrimitive(Desc.pPrimitive);
      //if(Desc.pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
      if(Desc.pPrimitive->GetIndexCount() > 0)
      {
        pGraphics->DrawPrimitive(Desc.ePrimType, 
          Desc.BaseVertexIndex, Desc.MinIndex, Desc.NumVertices, 
          Desc.StartIndex, Desc.PrimitiveCount);
      }
      else{
        ASSERT(0);
      }
      ++m_uDrawCallCount;

      if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
        pCanvas->SetWorldMatrix(float4x4::Identity);
      }
    }

    if(pModel->GetFirstChild() != NULL) {
      RenderRecursive(pCanvas, pModel, eType);
    }

    // 准备下一次循环
    pModel = pModel->GetNext();
  }
  return GX_OK;
}

GXHRESULT GVScene::RenderAll( Canvas3D* pCanvas, GVRenderType eType )
{
  m_uDrawCallCount = 0;

  pCanvas->UpdateCommonUniforms();
  pCanvas->Begin();
  pCanvas->Clear(0, 1.0f, NULL);

  GXHRESULT hr = RenderRecursive(pCanvas, m_pRoot, eType);
  pCanvas->End();
  return hr;
}

GXHRESULT GVScene::SaveFileRecursive(clFile* pFile, GVNode* pParent, GXINOUT u32& nVertBase)
{
  clBuffer buffer;
  GVRENDERDESC Desc;
  GVNode* pNode = pParent->GetFirstChild();

  while(pNode != NULL)
  {
    pNode->GetRenderDesc(GVRT_Normal, &Desc);

    if( ! TEST_FLAG(Desc.dwFlags, GVNF_VISIBLE)) {
      // 准备下一次循环
      pNode = pNode->GetNext();
      continue;
    }

    if(TEST_FLAG_NOT(Desc.dwFlags, GVNF_CONTAINER))
    {
      GXBOOL bval = ObjMeshUtility::SavePrimitive(&buffer, "noname", &Desc, nVertBase);
      if(bval)
      {
        pFile->Write(buffer.GetPtr(), (u32)buffer.GetSize());
        buffer.Resize(0, FALSE);
      }
      //if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
      //  pCanvas->SetWorldMatrix(Desc.matWorld);
      //}

      //// 应用材质
      //if(Desc.pMaterial != NULL) {
      //  pCanvas->SetMaterialInst(Desc.pMaterial);
      //}
      //else {
      //  ASSERT(0);
      //}

      //ASSERT(Desc.pPrimitive != NULL);
      //pGraphics->SetPrimitive(Desc.pPrimitive);
      //if(Desc.pPrimitive->GetType() == RESTYPE_INDEXED_PRIMITIVE)
      //{
      //  pGraphics->DrawPrimitive(Desc.ePrimType, 
      //    Desc.BaseVertexIndex, Desc.MinIndex, Desc.NumVertices, 
      //    Desc.StartIndex, Desc.PrimitiveCount);
      //}
      //else{
      //  ASSERT(0);
      //}
      //++m_uDrawCallCount;

      //if(TEST_FLAG(Desc.dwFlags, GVNF_UPDATEWORLDMAT)) {
      //  pCanvas->SetWorldMatrix(float4x4::Identity);
      //}
    }

    if(pNode->GetFirstChild() != NULL) {
      SaveFileRecursive(pFile, pNode, nVertBase);
    }

    // 准备下一次循环
    pNode = pNode->GetNext();
  }
  return GX_OK;
}

GXHRESULT GVScene::SaveToFileW(GXLPCWSTR szFilename)
{
  clFile file;
  if(file.CreateAlways(szFilename)) {
    u32 nVertBase = 1;
    return SaveFileRecursive(&file, m_pRoot, nVertBase);
  }
  return GX_FAIL;
}

GXHRESULT GVScene::Generate( Canvas3D* pCanvas, GVSequence* pRenderSequence, GVRenderType eType, GXDWORD dwRequired )
{  
  GVNode::AABB          aabbAbs;
  GVRENDERDESC          Desc;
  const GVNode::FrustumPlanes*  pFrustum;
  clstack<GVNode*>      NodeStack;

  pCanvas->UpdateCommonUniforms();
  pFrustum = pCanvas->GetViewFrustum();
  pRenderSequence->Clear();

  GVNode* pNode = m_pRoot->GetFirstChild();
  while(true)
  {
    while(pNode != NULL)
    {
      pNode->GetRenderDesc(eType, &Desc);

      if( ! TEST_FLAGS_ALL(Desc.dwFlags, (GVNF_VISIBLE | dwRequired))) {
        // 准备下一次循环
        pNode = pNode->GetNext();
        continue;
      }

      if(TEST_FLAG_NOT(Desc.dwFlags, GVNF_CONTAINER) && Desc.PrimitiveCount)
      {
        ASSERT(Desc.pPrimitive != NULL);
        ASSERT(Desc.pMaterial != NULL);
        
        // 能不能写的再优雅一些了？？？
        if(TEST_FLAG(Desc.dwFlags, GVNF_NOCLIP))
          goto JMP_ADD;

        pNode->GetAbsoluteAABB(aabbAbs);
        if(pFrustum->IntersectAABB(aabbAbs) == 0)
          goto JMP_NEXT;
JMP_ADD:  // 主要是这个不想写两遍
        pRenderSequence->Add(&Desc);
        ++m_uDrawCallCount;
      }
JMP_NEXT:

      if(pNode->GetFirstChild() != NULL) {
        NodeStack.push(pNode->GetNext());
        pNode = pNode->GetFirstChild();
        continue;
      }
      // 准备下一次循环
      pNode = pNode->GetNext();
    }

    if( ! NodeStack.empty()) {
      pNode = NodeStack.top();
      NodeStack.pop();
    }
    else break;
  }

  return GX_OK;
}

void GVScene::Update(Canvas3D* pCanvas, GXDWORD dwDeltaTime)
{
  // dwDeltaTime 为 0 也要更新, 因为可能有需要针对摄像机调整方向的Node
  BEGIN_SCOPED_LOCKER(m_Locker);
  if( ! m_CmdBuffer.empty()) {
    IntExecuteCommand();
  }
  END_SCOPED_LOCKER;

  m_dwDeltaTime = dwDeltaTime;
  m_dwAbsTime += dwDeltaTime;
  m_fDeltaTime = (float)m_dwDeltaTime * 1e-3f;

  GVSCENEUPDATE sContext;
  sContext.pCanvas3D       = pCanvas;
  sContext.pCamera         = pCanvas->GetCameraUnsafe();
  sContext.uDrawCallCount  = m_uDrawCallCount;
  sContext.dwDeltaTime     = m_dwDeltaTime;
  sContext.fDeltaTime      = m_fDeltaTime;
  sContext.dwAbsTime       = m_dwAbsTime;

  GVNodeStack NodeStack;
  int nTrackUpdateCount = 0;

  // 锁定
  clstd::ScopedSafeLocker lock(&m_Locker);

  GVNode* pNode = m_pRoot->GetFirstChild();
  while(1)
  {
    while(pNode)
    {
      // 如果通过可见性决定是否更新,会导致skeleton无法更新,因为它可能是不可见的
      GXBOOL bUpdateChild = TEST_FLAG_NOT(pNode->GetInterfaceCaps(), GVIC_UPDATE)
        ? TRUE : pNode->Update(sContext);

      nTrackUpdateCount++;

      // 根据返回值确定是否Update子节点
      if(bUpdateChild && pNode->GetFirstChild()) {
        NodeStack.push(pNode->GetNext());
        pNode = pNode->GetFirstChild();
        continue;
      }

      pNode = pNode->GetNext();
    }

    if(NodeStack.empty()) {
      break;
    }
    pNode = NodeStack.top();
    NodeStack.pop();
  }
  //UpdateRecursive(sUpdate, m_pRoot);
}

GXHRESULT GVScene::SetNodePosition( GVNode* pNode, CFloat3& vPos, clstd::ESpace eTransform /*= clstd::S_Self*/ )
{
  CMDBUFFER cmd;
  cmd.eCmd = eTransform == clstd::S_Self 
    ? CMD_SetSelfTranslation 
    : CMD_SetWorldTranslation;
  cmd.pNode  = pNode;
  cmd.vParam = vPos;

  clstd::ScopedSafeLocker lock(&m_Locker);
  m_CmdBuffer.push_back(cmd);
  return GX_OK;
}

GXHRESULT GVScene::SetNodeScaling( GVNode* pNode, CFloat3& vScaling )
{
  CMDBUFFER cmd;
  cmd.eCmd   = CMD_SetScaling;
  cmd.pNode  = pNode;
  cmd.vParam = vScaling;

  clstd::ScopedSafeLocker lock(&m_Locker);
  m_CmdBuffer.push_back(cmd);
  return GX_OK;
}

GXHRESULT GVScene::SetNodeTransform( GVNode* pNode, CFloat4x4& matTransform, clstd::ESpace eTransform)
{
  CMDBUFFER cmd;
  cmd.eCmd = eTransform == clstd::S_Self 
    ? CMD_SetSelfTranslation 
    : CMD_SetWorldTranslation;
  cmd.pNode  = pNode;
  cmd.vParam = matTransform.GetRow(0);

  clstd::ScopedSafeLocker lock(&m_Locker);
  m_CmdBuffer.push_back(cmd);

  cmd.eCmd = CMD_Postfix;
  cmd.vParam = matTransform.GetRow(1);
  m_CmdBuffer.push_back(cmd);

  cmd.vParam = matTransform.GetRow(2);
  m_CmdBuffer.push_back(cmd);

  cmd.vParam = matTransform.GetRow(3);
  m_CmdBuffer.push_back(cmd);
  return GX_OK;
}

GXHRESULT GVScene::IntExecuteCommand()
{
  ASSERT( ! m_CmdBuffer.empty());
  GXHRESULT hval = GX_FAIL;
  for(CmdBufList::iterator it = m_CmdBuffer.begin();
    it != m_CmdBuffer.end(); ++it) {
      switch(it->eCmd)
      {
      case CMD_SetSelfTranslation:
        it->pNode->SetPosition(*(float3*)&it->vParam, GVNode::S_RELATIVE);
        break;
      
      case CMD_SetWorldTranslation:
        it->pNode->SetPosition(*(float3*)&it->vParam, GVNode::S_ABSOLUTE);
        break;
      
      case CMD_SetScaling:
        it->pNode->SetScaling(*(float3*)&it->vParam);
        break;

      case CMD_SetSelfTransform:
      case CMD_SetWorldTransform:
        {          
          float4x4 matTransform;
          matTransform.SetRow(0, it->vParam);

          if((++it) == m_CmdBuffer.end()) {
            goto FAILED_RET;
          }
          matTransform.SetRow(1, it->vParam);

          if((++it) == m_CmdBuffer.end()) {
            goto FAILED_RET;
          }
          matTransform.SetRow(2, it->vParam);

          if((++it) == m_CmdBuffer.end()) {
            goto FAILED_RET;
          }
          matTransform.SetRow(3, it->vParam);

          it->pNode->SetTransform(matTransform, it->eCmd == CMD_SetSelfTransform
            ? GVNode::S_RELATIVE : GVNode::S_ABSOLUTE);
        }
        break;

      case CMD_SetRotationgEuler: 
        CLBREAK; // 尚未实现
        break;
      case CMD_SetRotationgQuat:
        CLBREAK; // 尚未实现
        break;
      default:
        CLBREAK;
      }
  }
  hval = GX_OK;
FAILED_RET:
  m_CmdBuffer.clear();
  return hval;
}

GXHRESULT GVScene::GetRoot( GVNode** ppRootNode )
{
  *ppRootNode = m_pRoot;
  if(m_pRoot) {
    return m_pRoot->AddRef();
  }
  return GX_FAIL;
}
