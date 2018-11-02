// 全局头文件
#include <GrapX.h>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GXKernel.h"
#include "GrapX/GShader.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"

// 私有头文件
#include <clTree.h>
#include <clTransform.h>
#include <thread/clSignal.h>
#include <thread/clMessageThread.h>
#include <smart/SmartRepository.h>
#include <GrapX/GVNode.h>
#include <GrapX/gvScene.h>

GVNode::GVNode(GVScene* pScene, GXDWORD dwClassCode)
  : m_ClsCode (dwClassCode)
  , m_dwFlags (GVNF_DEFAULT)
  , m_dwInterfaceCaps(0xffffffff)
{
  //m_matRelative.identity();
  //m_matAbsolute.identity();
  m_aabbLocal.Clear();
}

//GVNode::GVNode(GVModelClass eClass)
//  : m_pScene  (NULL)
//  , m_eClass  (eClass)
//  , m_dwFlags (0xffffffff)
//{
//}

GVNode::~GVNode()
{
  PickNode();
  while(m_pFirstChild != NULL)
  {
    GVNode* pChild = (GVNode*)m_pFirstChild;
    pChild->PickNode();
    SAFE_RELEASE(pChild);
  }
}

GXBOOL GVNode::Show(GXBOOL bShow)
{
  GXBOOL bret = TEST_FLAG(m_dwFlags, GVNF_VISIBLE);
  if(bShow) {
    SET_FLAG(m_dwFlags, GVNF_VISIBLE);
  }
  else {
    RESET_FLAG(m_dwFlags, GVNF_VISIBLE);
  }
  return bret;
}

GXDWORD GVNode::SetFlags(GXDWORD dwNewFlags)
{
  GXDWORD dwOldFlags = m_dwFlags;
  m_dwFlags = dwNewFlags;
  return dwOldFlags;
}

GXDWORD GVNode::CombineFlags(GXDWORD dwFlags)
{
  GXDWORD dwOldFlags = m_dwFlags;
  m_dwFlags |= dwFlags;
  return dwOldFlags;
}

void GVNode::UpdateChildPos()
{
  GVNode* pNode = GetFirstChild();
  while(pNode != NULL)
  {
    //pNode->m_matAbsolute = m_matAbsolute * pNode->GetRelativeMatrix();
    pNode->m_Transformation.UpdateAbsoluteMatrix(m_Transformation.GlobalMatrix);

    // TODO: 这个要重新设计
    if(pNode->m_Transformation.GlobalMatrix != float4x4::Identity) {
      SET_FLAG(pNode->m_dwFlags, GVNF_UPDATEWORLDMAT);
    }

    if(pNode->GetFirstChild() != NULL) {
      pNode->UpdateChildPos();
    }
    pNode = pNode->GetNext();
  }
}

//float3 GVNode::GetScale(TRANSFORM eTransform/* = S_RELATIVE*/)
//{
//  float3 vScaling;
//  if(eTransform == S_RELATIVE) {
//
//    m_matRelative.AffineTransformation(&vScaling, )
//    return float3(m_matRelative);
//  }
//  else if(eTransform == T_ABSOLUTE) {
//    return m_matAbsolute.GetRow(3);
//  }
//  return float3::Origin;
//
//}

GXBOOL GVNode::SetDirection(CFloat3& vDir, CFloat3& vUp)
{
  GVNode* const pParent = GetParent();
  if(pParent) {
    return m_Transformation.SetDirection(pParent->m_Transformation.GlobalMatrix, vDir, vUp);
  }
  else {
    return m_Transformation.SetDirection(vDir, vUp);
  }
}

void GVNode::SetTransform(const float4x4& matTransform, ESpace eTransform/* = S_RELATIVVE*/)
{
  GVNode* const pParent = GetParent();
  if(pParent == NULL)
  {
    m_Transformation.SetTransform(matTransform);
  }
  else if(eTransform == S_RELATIVE || eTransform == S_ABSOLUTE)
  {
    ASSERT(pParent != NULL);
    m_Transformation.SetTransform(pParent->m_Transformation.GlobalMatrix, matTransform, (clstd::ESpace)eTransform);
  }
  else if(eTransform == S_PHYSICALBODY && TEST_FLAG(m_dwFlags, GVNF_PHYSICALBODY))
  {
    m_Transformation.SetTransform(matTransform);
    // TODO: 分解Local transform
    return;
  }
  else return;
  UpdateChildPos();

  // TODO: 这个要重新设计
  if(m_Transformation.GlobalMatrix != float4x4::Identity) {
    SET_FLAG(m_dwFlags, GVNF_UPDATEWORLDMAT);
  }
}

void GVNode::SetTransform( const float3* pScaling, const Quaternion* pRotation, const float3* pTranslation )
{
  if( ! pScaling && ! pRotation && ! pTranslation) {
    return;
  }
  if( ! pScaling) {
    pScaling = &m_Transformation.scaling;
  }
  if( ! pRotation) {
    pRotation = &m_Transformation.rotation;
  }
  if( ! pTranslation) {
    pTranslation = &m_Transformation.translation;
  }

  float4x4 s, r, t;
  s.Scale(*pScaling);
  r.RotationQuaternion(pRotation);
  t.Translate(*pTranslation);
  SetTransform(s * r * t);
}

void GVNode::SetTransformR( const float3* pScaling, const float3* pEuler, const float3* pTranslation )
{
  Quaternion q;
  if( ! pScaling && ! pEuler && ! pTranslation) {
    return;
  }
  
  if( ! pScaling) {
    pScaling = &m_Transformation.scaling;
  }
  
  if( ! pEuler) {
    q = m_Transformation.rotation;
  }
  else {
    q.YawPitchRollR(*pEuler);
  }

  if( ! pTranslation) {
    pTranslation = &m_Transformation.translation;
  }

  float4x4 s, r, t;
  s.Scale(*pScaling);
  r.RotationQuaternion(&q);
  t.Translate(*pTranslation);
  SetTransform(s * r * t);
}

GVNode* GVNode::FindChild(GVNode* pStart, GXLPCSTR szName)
{
  GVNode* pChild = NULL;
  if(pStart == NULL) {
    pChild = static_cast<GVNode*>(m_pFirstChild);
  }
  else if(pStart->m_pParent == this) {
    pChild = pStart;
  }
  else {
    CLOG_ERROR("%s : 不是子对象.\n", __FUNCTION__);
    ASSERT(0);
    return pChild;
  }

  while(pChild) {
    if(pChild->m_strName == szName) {
      break;
    }
    pChild = static_cast<GVNode*>(pChild->m_pNext);
  }
  return pChild;
}

GVNode* GVNode::SetParent(GVNode* pNewParent, bool bLast/* = false*/)
{
  GVNode* pRetNode = clstd::treeT<GVNode>::SetParent(pNewParent, bLast);
  float4x4 matTransform = m_Transformation.GlobalMatrix;
  SetTransform(matTransform);
  return pRetNode;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVNode::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}

GXHRESULT GVNode::Release()
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

GXVOID GVNode::OnNotify( const NODENOTIFY* pNotify )
{
  RESET_FLAG(m_dwInterfaceCaps, GVIC_NOTIFY);
}

GXBOOL GVNode::Update(const GVSCENEUPDATE& sContext)
{
  //SET_FLAG(m_dwFlags, GVNF_DONOTUPDATE);
  RESET_FLAG(m_dwInterfaceCaps, GVIC_UPDATE);
  return TRUE;
}

GXBOOL GVNode::Collision()
{
  RESET_FLAG(m_dwInterfaceCaps, GVIC_COLLISION);
  return TRUE;
}

void GVNode::GetRenderDesc(GVRenderType eType, GVRENDERDESC* pRenderDesc)
{
  memset(pRenderDesc, 0, sizeof(GVRENDERDESC));
}

GXHRESULT GVNode::SetMaterialInstDirect(GXMaterialInst* pMtlInst)
{
  return GX_OK;
}

GXHRESULT GVNode::GetMaterialInst(GXMaterialInst** ppMtlInst)
{
  *ppMtlInst = NULL;
  CLBREAK;
  return GX_FAIL;
}

GXHRESULT GVNode::GetMaterialInstFilenameW(clStringW* pstrFilename)
{
  CLBREAK;
  return GX_FAIL;
}

GXHRESULT GVNode::SetMaterialInst(GXMaterialInst* pMtlInst, GXDWORD dwFlags)
{
  // 如果要根据顶点属性加载纹理的话, 就使用文件名方式加载
  if(TEST_FLAG_NOT(dwFlags, NODEMTL_IGNOREVERT))
  {
    clStringW strFilename;
    pMtlInst->GetFilenameW(&strFilename);
    return SetMaterialInstFromFileW(pMtlInst->GetGraphicsUnsafe(), strFilename, dwFlags);
  }

  // 设置子节点
  if(TEST_FLAG(dwFlags, NODEMTL_SETCHILDREN|NODEMTL_SETSONONLY)) {
    const GXDWORD dwChildFlags = TEST_FLAG(dwFlags, NODEMTL_SETSONONLY)
      ? dwFlags & (~(NODEMTL_SETCHILDREN|NODEMTL_SETSONONLY)) : dwFlags;
    GVNode* pChild = GetFirstChild();
    while(pChild != NULL)
    {
      pChild->SetMaterialInst(pMtlInst, dwChildFlags);
      pChild = pChild->GetNext();
    }
  }

  ASSERT(TEST_FLAG(dwFlags, NODEMTL_IGNOREVERT));
  return SetMaterialInstDirect(pMtlInst);
}

GXHRESULT GVNode::SetMaterialInstFromFileW(GXGraphics* pGraphics, GXLPCWSTR szFilename, GXDWORD dwFlags)
{
  // 设置子节点
  if(TEST_FLAG(dwFlags, NODEMTL_SETCHILDREN|NODEMTL_SETSONONLY)) {
    const GXDWORD dwChildFlags = TEST_FLAG(dwFlags, NODEMTL_SETSONONLY)
      ? dwFlags & (~(NODEMTL_SETCHILDREN|NODEMTL_SETSONONLY)) : dwFlags;
    GVNode* pChild = GetFirstChild();
    while(pChild != NULL)
    {
      pChild->SetMaterialInstFromFileW(pGraphics, szFilename, dwChildFlags);
      pChild = pChild->GetNext();
    }
  }

  clStringW strFilename = szFilename;
  GXMaterialInst* pMtlInst = NULL;
  const MtlLoadType eLoadType = TEST_FLAG(dwFlags, NODEMTL_CLONEINST) 
    ? MLT_CLONE : MLT_REFERENCE;


  // 没设置忽略顶点格式
  if(TEST_FLAG_NOT(dwFlags, NODEMTL_IGNOREVERT))
  {
    GVRENDERDESC renderdesc;
    GetRenderDesc(GVRT_Normal, &renderdesc);
    if(renderdesc.pPrimitive == NULL) {
      CLOG_WARNING("%s : Primitive is null.\r\n", __FUNCTION__);
      return GX_OK;
    }

    GVertexDeclaration* pVertDecl = NULL;
    if(GXSUCCEEDED(renderdesc.pPrimitive->GetVertexDeclaration(&pVertDecl)))
    {
      GXLPCVERTEXELEMENT lpVertElement = pVertDecl->GetVertexElement();
      clStringA strMacros;

      MOTestShaderCapsString(lpVertElement, strMacros);
      SAFE_RELEASE(pVertDecl);

      if(strMacros.IsNotEmpty())
      {
        // 如果用户已经指定了宏开关, 则用';'追加
        // TODO: 增加"!MACRO_SWITCHER"方式来强制去掉某个宏的功能
        strFilename.Append(strFilename.Find('|') != clStringW::npos ? ';' : '|');
        strFilename.Append(strMacros);
      }
    }
  }

  pGraphics->CreateMaterialFromFile(&pMtlInst, strFilename, eLoadType);
  GXHRESULT hval = SetMaterialInstDirect(pMtlInst);
  SAFE_RELEASE(pMtlInst);

  return hval;
}

GXHRESULT GVNode::SaveFileA(GXLPCSTR szFilename)
{
  clStringW Filename = szFilename;
  return SaveFileW(Filename);
}

GXHRESULT GVNode::SaveFileW(GXLPCWSTR szFilename)
{
  clSmartRepository Storage;
  GXHRESULT hval = SaveFile(&Storage);
  if(GXSUCCEEDED(hval)) {
    hval = Storage.SaveW(szFilename) ? GX_OK : GX_FAIL;
  }
  return hval;
}

GXHRESULT GVNode::SaveFile(clSmartRepository* pStorage)
{
  return GX_FAIL;
}

GXHRESULT GVNode::LoadFileA(GXGraphics* pGraphics, GXLPCSTR szFilename)
{
  return LoadFileW(pGraphics, clStringW(szFilename));
}

GXHRESULT GVNode::LoadFileW(GXGraphics* pGraphics, GXLPCWSTR szFilename)
{
  clSmartRepository Storage;
  GXBOOL bval = Storage.LoadW(szFilename);
  if(bval) {
    return LoadFile(pGraphics, &Storage);
  }
  return GX_FAIL;
}

GXHRESULT GVNode::LoadFile(GXGraphics* pGraphics, clSmartRepository* pStorage)
{
  return GX_FAIL;
}


//float GVNode::RayTrace(const Ray& ray, float3* pHit)
GXBOOL GVNode::RayTrace(const Ray& ray, NODERAYTRACE* pRayTrace)
{
  float fRetLength;

  float4x4 matInvAbs = float4x4::inverse(m_Transformation.GlobalMatrix);
  Ray RayLocal(ray.vOrigin * matInvAbs, ray.vDirection.MulAsMatrix3x3(matInvAbs));

  if(clstd::geometry::IntersectRayAABB(RayLocal, m_aabbLocal, &fRetLength, NULL) != 0)
  {
    pRayTrace->eType = NRTT_AABB;
    pRayTrace->vLocalHit = RayLocal.vOrigin + RayLocal.vDirection * fRetLength;
    pRayTrace->fSquareDist  = fRetLength * fRetLength;
    pRayTrace->nFaceIndex   = -1;
    return TRUE;
  }
  return FALSE;
}

void GVNode::SetScaling(const float3& vScaling/*, ESpace eTransform = S_RELATIVE*/)
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetScaling(pParent->m_Transformation.GlobalMatrix, vScaling);
  }
  else {
    m_Transformation.SetScaling(vScaling);
  }
  UpdateChildPos();

  if(m_Transformation.GlobalMatrix != float4x4::Identity) {
    SET_FLAG(m_dwFlags, GVNF_UPDATEWORLDMAT);
  }
}

void GVNode::SetRotationA( CFloat3& vEuler )
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }
  
  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetRotationA(pParent->GetTransform().GlobalMatrix, vEuler);
  }
  else {
    m_Transformation.SetRotationA(vEuler);
  }
  UpdateChildPos();
}

void GVNode::SetRotationR( CFloat3& vEuler )
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }
  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetRotationR(pParent->GetTransform(), vEuler);
  }
  else {
    m_Transformation.SetRotationR(vEuler);
  }
  UpdateChildPos();
}

void GVNode::SetRotation(const Quaternion& quater)
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }
  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetRotation(pParent->GetTransform(),quater);
  }
  else {
    m_Transformation.SetRotation(quater);
  }
  UpdateChildPos();
}

void GVNode::RotateA( CFloat3& vEuler )
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.RotateA(pParent->GetTransform(), vEuler);
  }
  else {
    m_Transformation.RotateA(vEuler);
  }
  UpdateChildPos();
}

void GVNode::RotateR( CFloat3& vEuler )
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.RotateR(pParent->GetTransform(), vEuler);
  }
  else {
    m_Transformation.RotateR(vEuler);
  }
  UpdateChildPos();
}

void GVNode::Rotate(const Quaternion& quater)
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.Rotate(pParent->GetTransform(),quater);
  }
  else {
    m_Transformation.Rotate(quater);
  }
  UpdateChildPos();
}

void GVNode::Move(CFloat3& vDelta)
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetPosition(pParent->GetTransform(), m_Transformation.translation + vDelta, (clstd::ESpace)S_RELATIVE);
  }
  else {
    float3 vPos = m_Transformation.GlobalMatrix.GetRow(3);
    m_Transformation.SetPosition(vPos + vDelta);
  }
  UpdateChildPos();

  if(m_Transformation.GlobalMatrix != float4x4::Identity) {
    SET_FLAG(m_dwFlags, GVNF_UPDATEWORLDMAT);
  }
}

float3 GVNode::GetPosition(ESpace eTransform/* = S_RELATIVE*/)
{
  if(eTransform == S_RELATIVE) {
    return m_Transformation.translation;
  }
  else if(eTransform == S_ABSOLUTE) {
    return m_Transformation.GlobalMatrix.GetRow(3);
  }
  return float3::Origin;
}

void GVNode::SetPosition(const float3& vPos, ESpace eSpace/* = S_RELATIVVE*/)
{
  if(TEST_FLAG(m_dwFlags, GVNF_STATIC)) {
    return;
  }

  GVNode* const pParent = GetParent();
  if(pParent) {
    m_Transformation.SetPosition(pParent->GetTransform(), vPos, (clstd::ESpace)eSpace);
  }
  else {
    m_Transformation.SetPosition(vPos);
  }
  UpdateChildPos();

  if(m_Transformation.GlobalMatrix != float4x4::Identity) {
    SET_FLAG(m_dwFlags, GVNF_UPDATEWORLDMAT);
  }
}

GXVOID GVNode::GetAbsoluteAABB( AABB& aabb ) const
{
  // 这个计算出的AABB在有旋转的情况下是不太准确的
  float3 vTranslation;
  float3 vScaling;
  m_Transformation.GlobalMatrix.DecomposeTranslation(&vTranslation);
  m_Transformation.GlobalMatrix.DecomposeScaling(&vScaling);

  aabb.Set(m_aabbLocal.vMin * vScaling + vTranslation, 
      m_aabbLocal.vMax * vScaling + vTranslation);
  //aabb.Set(m_aabbLocal.vMin * m_Transformation.GlobalMatrix, 
  //  m_aabbLocal.vMax * m_Transformation.GlobalMatrix);
}

GXVOID GVNode::CalculateAABB()
{
  GVNode* pNode = m_pFirstChild;
  m_aabbLocal.Clear();
  while(pNode) {
    pNode->CalculateAABB();
    m_aabbLocal.Merge(pNode->m_aabbLocal);
    pNode = pNode->m_pNext;
  }
}

GXHRESULT GVNode::Clone( GVNode** ppClonedNode/*, GXBOOL bRecursive*/ )
{
  GVNode* pNewNode = new GVNode(NULL, m_ClsCode);

  if( ! InlCheckNewAndIncReference(pNewNode)) {
    return GX_FAIL;
  }

  pNewNode->m_strName         = m_strName;
  pNewNode->m_dwFlags         = m_dwFlags;
  pNewNode->m_dwInterfaceCaps = m_dwInterfaceCaps;
  pNewNode->m_aabbLocal       = m_aabbLocal;
  pNewNode->m_Transformation  = m_Transformation;

  //if(bRecursive) {
  //  GVNode* pChildNode = GetFirstChild();
  //  while(pChildNode) {
  //    GVNode* pNewChildNode = NULL;
  //    if(GXSUCCEEDED(pChildNode->Clone(&pNewChildNode, bRecursive))) {
  //      pNewChildNode->SetParent(pNewNode);
  //    }
  //    pChildNode = pChildNode->GetNext();
  //  }
  //}

  *ppClonedNode = pNewNode;
  return GX_OK;
}

//void GVNode::CombineRecursiveFlags( GXDWORD dwFlags )
//{
//
//}
//
//void GVNode::RemoveRecursiveFlags( GXDWORD dwFlags )
//{
//
//}
