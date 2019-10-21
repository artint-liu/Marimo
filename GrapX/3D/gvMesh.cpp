// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

// 标准接口
#include <GrapX/GResource.h>
#include <GrapX/GPrimitive.h>
#include <GrapX/GXGraphics.h>
#include <GrapX/GShader.h>

// 私有头文件
#include "clTree.h"
#include "clTransform.h"
#include "smart/SmartRepository.h"
#include <Smart/smartstream.h>
#include <GrapX/VertexDecl.h>
#include "GrapX/gxError.h"

#include <GrapX/gvNode.h>
#include <GrapX/gvMesh.h>
#include <GrapX/gvScene.h>

#include "GrapX/gxUtility.h"

// 文件储存用的Keys
#define MESH_NAME         "Mesh@Name"
#define MESH_MTLINST      "Mesh@MtlInstW"
//#define MESH_VERTEXDECL   "Mesh@VertexDeclaration"
//#define MESH_ASMVERTICES  "Mesh@AssembledVertices"
//#define MESH_INDICES      "Mesh@Indices"
//#define MESH_PRIMCOUNT    "Mesh@PrimitiveCount"
//#define MESH_STARTINDEX   "Mesh@StartIndex"
#define MESH_TRANSFORM    "Mesh@Transform"

using namespace clstd;
using namespace GrapX;

GVMesh::GVMesh(Graphics* pGraphics)
  : GVNode       (NULL, GXMAKEFOURCC('M','E','S','H'))
  //, m_pMtlInst    (NULL)
  , m_pPrimitive  (NULL)
  , m_nPrimiCount (0)
  , m_nVertCount  (0)
  , m_nStartIndex (0)
{
}

GVMesh::GVMesh(Graphics* pGraphics, GXDWORD dwClassCode)
  : GVNode       (NULL, dwClassCode)
  //, m_pMtlInst(NULL)
  , m_pPrimitive(NULL)
  , m_nPrimiCount (0)
  , m_nVertCount  (0)
  , m_nStartIndex (0)
{
}

GVMesh::~GVMesh()
{
  Clear();
}

void GVMesh::Clear()
{
  m_nPrimiCount = 0;
  m_nVertCount  = 0;
  m_nStartIndex = 0;
  m_MtlInsts.clear();
  //SAFE_RELEASE(m_pMtlInst);
  SAFE_RELEASE(m_pPrimitive);
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVMesh::AddRef()
{
  GXHRESULT hr = GVNode::AddRef();
  return hr;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXBOOL GVMesh::InitializeAsObjFromFile(Graphics* pGraphics, GXLPCWSTR szFilename, GXResUsage usage, const float4x4* pTransform)
{
  clFile file;
  GXBOOL bval = FALSE;
  if(szFilename == NULL) {
    return bval;
  }
  if(file.OpenExisting(szFilename)) {
    clstd::MemBuffer buf;
    file.Read(&buf);
    m_strName = szFilename;
    bval = InitializeAsObjFromMemory(pGraphics, &buf, usage, pTransform);
    //SAFE_DELETE(pBuffer);
  }
  return bval;
}

GXBOOL GVMesh::InitializeAsObjFromMemory(Graphics* pGraphics, clBufferBase* pBuffer, GXResUsage usage, const float4x4* pTransform)
{
  using namespace ObjMeshUtility;

  PrimaryMeshsArray aMeshs;
  const GXBOOL bval = LoadFromMemory(pBuffer, pTransform, aMeshs);
  //clStringA strFilename = m_strName;
  if(bval)
  {
    if(aMeshs.size() > 1)
    {
      m_dwFlags |= GVNF_CONTAINER;  // 标志这个对象是容器对象

      for(ObjMeshUtility::PrimaryMeshsArray::iterator it = aMeshs.begin();
        it != aMeshs.end(); ++it)
      {
        PRIMARYMESH& me = *it;
        GVMesh* pSubMesh = NULL;

        GVMesh::CreateUserPrimitive(pGraphics, me.aFaces.size(), MOGetSysVertexDecl(GXVD_P3T2N3F),
          &me.aVertices.front(), me.aVertices.size(), 
          (VIndex*)&me.aFaces.front(), me.aFaces.size() * 3, usage, &pSubMesh);

        if(me.Name.IsNotEmpty()) {
          pSubMesh->SetName(me.Name);
        }
        pSubMesh->SetParent(this);
        CLOG("Add Obj mesh:\"%s\"\n", (const char*)pSubMesh->m_strName);

        // 更新父对象的AABB
        GVMesh* pChild = (GVMesh*)m_pFirstChild;
        while(pChild != NULL)
        {
          m_aabbLocal.vMin.Min(pChild->m_aabbLocal.vMin);
          m_aabbLocal.vMax.Max(pChild->m_aabbLocal.vMax);
          pChild = (GVMesh*)(pChild->m_pNext);
        }
      }
    }
    else
    {
      // 如果之前没有子Mesh,则剩余数据就放在当前节点对象里面
      //if(aMeshVertices.size() > 0 && aFaces.size() > 0)
      //{
      PRIMARYMESH& me = aMeshs[0];
      IntCreatePrimitive(pGraphics, me.aFaces.size(), MOGetSysVertexDecl(GXVD_P3T2N3F), 
        &me.aVertices.front(), me.aVertices.size(),
          (VIndex*)&me.aFaces.front(), me.aFaces.size() * 3, usage);

      if(me.Name.IsNotEmpty()) {
        m_strName = me.Name;
      }

      return TRUE;
    }  
  }
  return bval;
}

template<typename _VIndexT>
GXBOOL GVMesh::IntCreatePrimitiveT(Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, _VIndexT* pIndices, GXSIZE_T nIdxCount, GXResUsage usage)
{
  GXBOOL bval = FALSE;
  //m_eType       = eType;
  m_nPrimiCount = (GXUINT)nPrimCount;
  m_nVertCount  = (GXUINT)nVertCount;

  const GXUINT nStride = MOGetDeclVertexSize(lpVertDecl);
  if(GXSUCCEEDED(pGraphics->CreatePrimitive(&m_pPrimitive, NULL,
    lpVertDecl, usage, (GXUINT)nVertCount, (GXUINT)nStride, lpVertics, (GXUINT)nIdxCount, sizeof(_VIndexT), pIndices)))
  {
    GXVERTEXELEMENT Desc;
    int nOffset = MOGetDeclOffset(lpVertDecl, GXDECLUSAGE_POSITION, 0, &Desc);
    if(Desc.Type == GXDECLTYPE_FLOAT3)
    {
      AABB aabbPrim;
      mesh::CalculateAABB(aabbPrim, (float3*)((GXLPBYTE)lpVertics + nOffset), (GXUINT)nVertCount, (GXUINT)nStride);
      m_aabbLocal.Merge(aabbPrim);
    }
    //m_aabbLocal.UpdateCenterExtent();
    bval = TRUE;
  }
  return bval;
}

GXBOOL GVMesh::IntCreatePrimitive(Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount, GXResUsage usage)
{
  return IntCreatePrimitiveT(pGraphics, nPrimCount, lpVertDecl, lpVertics, nVertCount, pIndices, nIdxCount, usage);
}

GXBOOL GVMesh::IntCreatePrimitive(Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex32* pIndices, GXSIZE_T nIdxCount, GXResUsage usage)
{
  return IntCreatePrimitiveT(pGraphics, nPrimCount, lpVertDecl, lpVertics, nVertCount, pIndices, nIdxCount, usage);
}

GXBOOL GVMesh::IntSetPrimitive(GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, Primitive* pPrimitive)
{
  m_pPrimitive = pPrimitive;
  if(m_pPrimitive) {
    m_pPrimitive->AddRef();
  }
  else return FALSE;
  //m_eType = eType;
  m_nPrimiCount = (GXUINT)nPrimCount;
  m_nStartIndex = (GXUINT)nStartIndex;
  m_nVertCount = m_pPrimitive->GetVertexCount();

  //GVertexDeclaration* pDeclaration = NULL;
  //pPrimitive->GetVertexDeclaration(&pDeclaration);

  GXVERTEXELEMENT Desc;
  //int nOffset = MOGetDeclOffset(pDeclaration->GetVertexElement(), GXDECLUSAGE_POSITION, 0, &Desc);
  int nOffset = pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0, &Desc);
  if(Desc.Type == GXDECLTYPE_FLOAT3)
  {
    AABB aabbPrim;
    PrimitiveUtility::MapVertices locker_v(pPrimitive, GXResMap::Read);
    PrimitiveUtility::MapIndices  locker_i(pPrimitive, GXResMap::Read);
    mesh::CalculateAABBFromIndices(aabbPrim, (GXLPCBYTE)locker_v.GetPtr() + nOffset, 
      (const VIndex*)locker_i.GetPtr() + nStartIndex, nPrimCount * 3, pPrimitive->GetVertexStride());
    m_aabbLocal.Merge(aabbPrim);
  }

  //SAFE_RELEASE(pDeclaration);
  return TRUE;
}

GVRENDERDESC2* GVMesh::GetRenderDesc(int nRenderCate)
{
  if (nRenderCate >= (int)m_MtlInsts.size()) {
    return NULL;
  }
  return &m_Renderer;
}

void GVMesh::GetRenderDesc(int nRenderCate, GVRENDERDESC* pRenderDesc)
{
  if(nRenderCate < (int)m_MtlInsts.size())
  {
    pRenderDesc->dwFlags = m_dwFlags;
    pRenderDesc->dwLayer = m_dwLayer;
    pRenderDesc->ePrimType = GXPT_TRIANGLELIST;
    pRenderDesc->pPrimitive = m_pPrimitive;
    pRenderDesc->pMaterial = m_MtlInsts[nRenderCate];
    pRenderDesc->matWorld = m_Transformation.GlobalMatrix;
    pRenderDesc->BaseVertexIndex = 0;
    pRenderDesc->RenderQueue = pRenderDesc->pMaterial ? pRenderDesc->pMaterial->GetRenderQueue() : 0;

    pRenderDesc->MinIndex = 0;
    pRenderDesc->NumVertices = m_nVertCount;
    pRenderDesc->StartIndex = m_nStartIndex;
    pRenderDesc->PrimitiveCount = m_nPrimiCount;
  }
  else
  {
    pRenderDesc->dwFlags = 0;
    pRenderDesc->dwLayer = 0;
    pRenderDesc->pPrimitive = NULL;
    pRenderDesc->PrimitiveCount = 0;
  }
}

GXBOOL GVMesh::RayTrace(const Ray& ray, NODERAYTRACE* pRayTrace) // TODO: Ray 改为 NormalizedRay
{
  //float fDist = GVNode::RayTrace(ray, pHit);
  float fDist;
  float4x4 matInvAbs = float4x4::inverse(m_Transformation.GlobalMatrix);

  // TODO: 专门增加一个构造函数和方法，用矩阵变换 NormalizedRay 和 Ray
  GVNode::NormalizedRay RayLocal(ray.vOrigin * matInvAbs, ray.vDirection.MulAsMatrix3x3(matInvAbs));

  if(clstd::geometry::IntersectRayAABB(RayLocal, m_aabbLocal, &fDist, NULL) != 0)
  {
    pRayTrace->eType = NRTT_AABB;
    pRayTrace->vLocalHit = RayLocal.vOrigin + RayLocal.vDirection * fDist;
    pRayTrace->fSquareDist = fDist * fDist;

    if(m_pPrimitive == NULL || TEST_FLAG(m_dwFlags, GVNF_CONTAINER))
    {
      return TRUE;
    }

    // --- 模型相交检测
    PrimitiveUtility::MapVertices locker_v(m_pPrimitive, GXResMap::Read);
    PrimitiveUtility::MapIndices  locker_i(m_pPrimitive, GXResMap::Read);

    // 如果锁定失败，则只返回AABB求交结果
    GXBYTE* pVertBuf = static_cast<GXBYTE*>(locker_v.GetPtr());
    VIndex* pIndicesBuf = static_cast<VIndex*>(locker_i.GetPtr());
    if( ! (pVertBuf && pIndicesBuf)) {
      return TRUE;
    }

    GXUINT nFaceCount = m_pPrimitive->GetIndexCount() / 3;
    GXUINT nStride = m_pPrimitive->GetVertexStride();
    GXINT uOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
    if(uOffset < 0 || nFaceCount == 0) {
      return TRUE;
    }

    pVertBuf += uOffset;
    fDist = FLT_MAX;
    for(GXUINT nFaceIndex = 0; nFaceIndex < nFaceCount; nFaceIndex++)
    {
      const float3& v0 = *(float3*)(pVertBuf + nStride * pIndicesBuf[nFaceIndex * 3]);
      const float3& v1 = *(float3*)(pVertBuf + nStride * pIndicesBuf[nFaceIndex * 3 + 1]);
      const float3& v2 = *(float3*)(pVertBuf + nStride * pIndicesBuf[nFaceIndex * 3 + 2]);

      float t, u, v;
      if(clstd::geometry::IntersectRayWithTriangle(RayLocal, v0, v1, v2, t, u, v) && fDist > t)
      {
        fDist = t;
        pRayTrace->nFaceIndex = nFaceIndex;
      }
    }
    // ---

    // 已经判断与 AABB 相交
    // 这里如果有更精细的相交,则更新,否则也返回 TRUE
    if(fDist != FLT_MAX)
    {
      pRayTrace->eType = NRTT_MESHFACE;
      pRayTrace->vLocalHit = RayLocal.vOrigin + RayLocal.vDirection * fDist;
      pRayTrace->fSquareDist = fDist * fDist;
    }

    return TRUE;
  }
  return FALSE;
}

GXBOOL GVMesh::SetMaterial(GrapX::Material* pMtlInst, int nRenderCate)
{
  if (nRenderCate >= (int)m_MtlInsts.size()) {
    m_MtlInsts.resize((size_t)nRenderCate + 1, GrapX::ObjectT<GrapX::Material>(NULL));
  }
  m_MtlInsts[nRenderCate] = pMtlInst;

  return TRUE;
  //return InlSetNewObjectT(m_pMtlInst, pMtlInst);
}

GXBOOL GVMesh::GetMaterial(int nRenderCate, GrapX::Material** ppMtlInst)
{
  if (nRenderCate < (int)m_MtlInsts.size()) {
    *ppMtlInst = m_MtlInsts[nRenderCate];
    (*ppMtlInst)->AddRef();
    return TRUE;
  }
  return FALSE;
  //if(m_pMtlInst != NULL) {
  //  *ppMtlInst = m_pMtlInst;
  //  m_pMtlInst->AddRef();
  //  return GX_OK;
  //}
  //return GX_FAIL;
}

GXBOOL GVMesh::GetMaterialFilename(int nRenderCate, clStringW* pstrFilename)
{
  if (nRenderCate < (int)m_MtlInsts.size() && m_MtlInsts[nRenderCate] != NULL) {
    if (pstrFilename == NULL) {
      return TRUE; // 只是探测 m_pMtlInst 是否有效就直接返回
    }
    return m_MtlInsts[nRenderCate]->GetFilename(pstrFilename);
  }
  return FALSE;

  //if(m_pMtlInst == NULL) {
  //  return GX_FAIL;
  //}
  //else if(pstrFilename == NULL) {
  //  return GX_OK; // 只是探测 m_pMtlInst 是否有效就直接返回
  //}
  //return m_pMtlInst->GetFilename(pstrFilename);
}

//////////////////////////////////////////////////////////////////////////

GXHRESULT GVMesh::CreateUserPrimitive(Graphics* pGraphics, GXSIZE_T nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, GXSIZE_T nVertCount, VIndex* pIndices, GXSIZE_T nIdxCount, GXResUsage usage, GVMesh** ppMesh)
{
  GVMesh* pMesh = new GVMesh(NULL);
  if(pMesh == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pMesh->AddRef();
  if(pMesh->IntCreatePrimitive(pGraphics, nPrimCount, lpVertDecl, lpVertics, nVertCount, pIndices, nIdxCount, usage))
  {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GXHRESULT GVMesh::CreateUserPrimitive(Graphics* pGraphics, GXSIZE_T nPrimCount, GXSIZE_T nStartIndex, Primitive* pPrimitive, GVMesh** ppMesh)
{
  GVMesh* pMesh = new GVMesh(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    return GX_FAIL;
  }

  if(pMesh->IntSetPrimitive(nPrimCount, nStartIndex, pPrimitive))
  {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GXBOOL GVMesh::IntCreateMesh(Graphics* pGraphics, const GVMESHDATA* pMeshComponent)
{
  GXBOOL bval = TRUE;
  // 检查合法性
  if(GVMESHDATA::Check(pMeshComponent))
  {
    GXVERTEXELEMENT VertElement[64];
    memset(&VertElement, 0, sizeof(VertElement));
    clstd::MemBuffer buffer;
    PrimitiveUtility::ConvertMeshDataToStream(&buffer, VertElement, pMeshComponent);

    if(TEST_FLAG(pMeshComponent->dwFlags, MESHDATA_FLAG_INDICES_32))
    {
      if (_CL_NOT_(IntCreatePrimitive(pGraphics, pMeshComponent->nIndexCount / 3, VertElement,
        buffer.GetPtr(), pMeshComponent->nVertexCount, pMeshComponent->pIndices32, pMeshComponent->nIndexCount, pMeshComponent->usage)) )
      {
        bval = FALSE;
      }
    }
    else if(_CL_NOT_(IntCreatePrimitive(pGraphics, pMeshComponent->nIndexCount / 3, VertElement, 
            buffer.GetPtr(), pMeshComponent->nVertexCount, pMeshComponent->pIndices, pMeshComponent->nIndexCount, pMeshComponent->usage)) )
    {
      bval = FALSE;
    }

    //SAFE_DELETE_ARRAY(lpStreamSource);
    return bval;
  }
  return FALSE;
}

GXBOOL GVMesh::IntInitializeAsContainer(Graphics* pGraphics, GVNode** pNodesArray, int nNodeCount)
{
  SetFlags(GetFlags() | GVNF_CONTAINER);
  for(int i = 0; i < nNodeCount; i++)
  {
    pNodesArray[i]->SetParent(this);
  }
  return TRUE;
}

//GXBOOL GVMesh::CloneMesh( GVMesh* pSource )
//{
//  m_pMtlInst = pSource->m_pMtlInst;
//  m_pMtlInst->AddRef();
//
//  m_pPrimitive = pSource->m_pPrimitive;
//  m_pPrimitive->AddRef();
//
//  m_nPrimiCount = pSource->m_nPrimiCount;
//  m_nVertCount = pSource->m_nVertCount;
//  m_nStartIndex = pSource->m_nStartIndex;
//}

GXHRESULT GVMesh::CreateMesh(Graphics* pGraphics, const GVMESHDATA* pMeshComponent, GVMesh** ppMesh)
{
  GXHRESULT hval = GX_OK;
  GVMesh* pMesh = new GVMesh(NULL);
  if(pMesh == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pMesh->AddRef();

  if(pMesh->IntCreateMesh(pGraphics, pMeshComponent)) {
    *ppMesh = pMesh;
    return hval;
  }

  SAFE_RELEASE(pMesh);
  return hval;
}

GXHRESULT GVMesh::CreateContainer(Graphics* pGraphics, GVNode** pNodesArray, int nNodeCount, GVMesh** ppMesh)
{
  GXHRESULT hval = GX_OK;
  GVMesh* pMesh = new GVMesh(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  if( ! pMesh->IntInitializeAsContainer(pGraphics, pNodesArray, nNodeCount))
  {
    pMesh->Release();
    pMesh = NULL;
    hval = GX_FAIL;
  }
  pMesh->CalculateAABB();
  *ppMesh = pMesh;
  return hval;
}

//GXHRESULT GVMesh::Clone( GVMesh** pNewMesh, GVMesh* pSourceMesh )
//{
//  GXHRESULT hval = GX_OK;
//  GVMesh* pMesh = new GVMesh(NULL);
//
//  if( ! InlCheckNewAndIncReference(pMesh)) {
//    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
//    return GX_FAIL;
//  }
//}

GXHRESULT GVMesh::Clone( GVNode** ppClonedNode/*, GXBOOL bRecursive*/ )
{
  GVMesh* pNewMesh = new GVMesh(NULL, m_ClsCode);

  if( ! InlCheckNewAndIncReference(pNewMesh)) {
    return GX_FAIL;
  }

  pNewMesh->m_strName         = m_strName + " cloned";
  pNewMesh->m_dwFlags         = m_dwFlags;
  pNewMesh->m_dwInterfaceCaps = m_dwInterfaceCaps;
  pNewMesh->m_aabbLocal       = m_aabbLocal;
  pNewMesh->m_Transformation  = m_Transformation;

  pNewMesh->m_MtlInsts = m_MtlInsts;
  //pNewMesh->m_pMtlInst->AddRef();

  pNewMesh->m_pPrimitive = m_pPrimitive;
  pNewMesh->m_pPrimitive->AddRef();

  pNewMesh->m_nPrimiCount = m_nPrimiCount;
  pNewMesh->m_nVertCount  = m_nVertCount;
  pNewMesh->m_nStartIndex = m_nStartIndex;

  *ppClonedNode = pNewMesh;
  return GX_OK;
}

GXHRESULT GVMesh::LoadObjFromFileA(Graphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh, GXResUsage usage, const float4x4* pTransform /* = NULL */)
{
  clStringW strFile = szFilename;
  return LoadObjFromFileW(pGraphics, strFile, ppMesh, usage, pTransform);
}

GXHRESULT GVMesh::LoadObjFromFileW(Graphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh, GXResUsage usage, const float4x4* pTransform /* = NULL */)
{
  GVMesh* pMesh = new GVMesh(NULL);
  if(pMesh == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pMesh->AddRef();
  if(pMesh->InitializeAsObjFromFile(pGraphics, szFilename, usage, pTransform)) {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GXHRESULT GVMesh::LoadMeshFromFileA(Graphics* pGraphics, GXLPCSTR szFilename, GVMesh** ppMesh)
{
  clStringW strFilename = szFilename;
  return LoadMeshFromFileW(pGraphics, strFilename, ppMesh);
}

GXHRESULT GVMesh::LoadMeshFromFileW(Graphics* pGraphics, GXLPCWSTR szFilename, GVMesh** ppMesh)
{
  GVMesh* pMesh = new GVMesh(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    return GX_FAIL;
  }

  if(pMesh->LoadFileW(pGraphics, szFilename)) {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GXHRESULT GVMesh::LoadMeshFromRepository(Graphics* pGraphics, SmartRepository* pStorage, GXResUsage usage, GVMesh** ppMesh)
{
  GVMesh* pMesh = new GVMesh(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    return GX_FAIL;
  }

  if(pMesh->LoadFile(pGraphics, pStorage, usage)) {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GXHRESULT GVMesh::SaveFile(SmartRepository* pStorage)
{
  pStorage->WriteStringA(NULL, MESH_NAME, GetName());

  if(m_MtlInsts.empty() == false && m_MtlInsts[0] != NULL)
  {
    clStringW strMtlFile;
    m_MtlInsts[0]->GetFilename(&strMtlFile);
    if(strMtlFile.IsNotEmpty()) {
      m_pPrimitive->GetGraphicsUnsafe()->ConvertToRelativePathW(strMtlFile);
      pStorage->WriteStringW(NULL, MESH_MTLINST, strMtlFile);
    }
  }

  if(m_pPrimitive != NULL)
  {
    RepoUtility::SavePrimitive(pStorage, "Mesh", m_pPrimitive, m_nStartIndex, m_nPrimiCount);
  }

  float4x4 matLocal = m_Transformation.ToRelativeMatrix();
  pStorage->WriteStructT(NULL, MESH_TRANSFORM, matLocal);

  ASSERT(GetFirstChild() == NULL);
  return GX_OK;
}

GXHRESULT GVMesh::LoadFile(Graphics* pGraphics, SmartRepository* pStorage, GXResUsage usage)
{
  Clear();
  clStringA strName;
  clStringW strMtlName;
  //s32 nReadSize;
  float4x4 matLocal = m_Transformation.ToRelativeMatrix();
  GXVERTEXELEMENT VertexElement[64];
  if( ! pStorage->ReadStringA(NULL, MESH_NAME, strName)) {
    CLOG_ERROR("%s : Can not find mesh name.\n", __FUNCTION__);
    //return FALSE;
  }
  pStorage->ReadStringW(NULL, MESH_MTLINST, strMtlName);    

  InlSetZeroT(VertexElement);
  //nReadSize = pStorage->ReadStructArrayT(NULL, MESH_VERTEXDECL, VertexElement, 64);
  //if(nReadSize < 0) {
  //  ASSERT(0);
  //}

  //s32 cbVertSize = pStorage->GetLength(NULL, MESH_ASMVERTICES);
  //s32 nIndexCount = pStorage->GetLength(NULL, MESH_INDICES) / sizeof(VIndex);

  //GXBYTE* pVertices = NULL;
  //VIndex* pIndices  = NULL;
  GXBOOL bval = TRUE;
  clBuffer Vertices;
  clBuffer Indices;
  try
  {
    GXSIZE_T nPrimiCount;
    GXSIZE_T nStartIndex;
  //  pStorage->Read64(NULL, MESH_PRIMCOUNT, (u32*)&nPrimiCount, 0);
  //  pStorage->Read64(NULL, MESH_STARTINDEX, (u32*)&nStartIndex, 0);
    RepoUtility::LoadPrimitive(pStorage, "Mesh", VertexElement, &Vertices, &Indices, nStartIndex, nPrimiCount);

    bval = IntCreatePrimitive(pGraphics, nPrimiCount, VertexElement, Vertices.GetPtr(), 
      nPrimiCount * 3, (VIndex*)Indices.GetPtr(), Indices.GetSize() / sizeof(VIndex), usage);

    if( ! pStorage->ReadStructT(NULL, MESH_TRANSFORM, matLocal)) {
      matLocal.identity();
    }


    if(bval) {
      SetMaterialFromFile(pGraphics, strMtlName, NODEMTL_CLONEINST);
      SetName(strName);
      SetTransform(matLocal);
    }
    //SAFE_DELETE_ARRAY(pVertices);
    //SAFE_DELETE_ARRAY(pIndices);
  }
  //catch(bad_alloc &ba)
  catch(...)
  {
    CLBREAK;
  }
  return bval;
}

GXVOID GVMesh::CalculateAABB()
{
  GVNode::CalculateAABB();

  if(m_pPrimitive) {
    int nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
    AABB aabb;
    PrimitiveUtility::MapVertices locker_v(m_pPrimitive, GXResMap::Read);
    PrimitiveUtility::MapIndices  locker_i(m_pPrimitive, GXResMap::Read);
    mesh::CalculateAABB(aabb, reinterpret_cast<float3*>((GXLPBYTE)locker_v.GetPtr() + nOffset), 
      m_pPrimitive->GetVertexCount(), m_pPrimitive->GetVertexStride());

    m_aabbLocal.Merge(aabb);
  }
}

void GVMesh::ApplyTransform()
{
  const int nCount = m_pPrimitive->GetVertexCount();
  const int nStride = m_pPrimitive->GetVertexStride();
  const float4x4 mat = m_Transformation.ToRelativeMatrix();
  PrimitiveUtility::MapVertices locker_v(m_pPrimitive, GXResMap::ReadWrite);
  const GXLPBYTE lpBuffer = static_cast<GXLPBYTE>(locker_v.GetPtr());

  int nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
  mesh::TransformPosition(mat, (float3*)(lpBuffer + nOffset), nCount, nStride);

  // 法线
  nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_NORMAL, 0);
  if(nOffset >= 0) {
    mesh::TransformVectors(mat, (float3*)(lpBuffer + nOffset), nCount, nStride);
  }

  // 切线
  nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_TANGENT, 0);
  if(nOffset >= 0) {
    mesh::TransformVectors(mat, (float3*)(lpBuffer + nOffset), nCount, nStride);
  }

  // 副法线
  nOffset = m_pPrimitive->GetElementOffset(GXDECLUSAGE_BINORMAL, 0);
  if(nOffset >= 0) {
    mesh::TransformVectors(mat, (float3*)(lpBuffer + nOffset), nCount, nStride);
  }

  // 更新顶点
  //m_pPrimitive->UpdateResouce(GPrimitive::ResourceVertices);

  // 重置变换矩阵为单位阵
  float3 vScale(1.0f);
  Quaternion sRotation(0, 0, 0, 1);
  SetTransform(&vScale, &sRotation, &float3::Origin);
}

//////////////////////////////////////////////////////////////////////////
namespace mesh
{
  typedef clvector<GXVERTEXELEMENT> VertElementArray;

  GXBOOL ValidateIndex(VIndex* pIndices, int nIndexCount, int nVertexCount)
  {
    for(int i = 0; i < nIndexCount; i++)
    {
      if(pIndices[i] >= nVertexCount) {
        return FALSE;
      }
    }
    return TRUE;
  }



  template<typename _Ty>
  inline void InlAppendVertDecl(
    GXLPCVOID pMeshComponentElement,
    GXDeclUsage eUsage, 
    VertElementArray& aVertDecl, 
    GXVERTEXELEMENT& ve)
  {
    if(pMeshComponentElement) {
      ve.Usage = eUsage;
      aVertDecl.push_back(ve);
      ve.Offset += sizeof(_Ty);
    }
  }

  void TryAppendTexcoordVertDecl(VertElementArray& aVertDecl, GXVERTEXELEMENT &ve, GXLPCVOID pElements, GXUINT UsageIndex,
    GXDWORD dwMaskFlags, GXDWORD uv_float, GXDWORD uv_float2, GXDWORD uv_float3, GXDWORD uv_float4)
  {
    ve.UsageIndex = UsageIndex;
    if (dwMaskFlags == uv_float)
    {
      ve.Type = GXDECLTYPE_FLOAT1;
      mesh::InlAppendVertDecl<float>(pElements, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);
    }
    else if (dwMaskFlags == uv_float2)
    {
      ve.Type = GXDECLTYPE_FLOAT2;
      mesh::InlAppendVertDecl<float2>(pElements, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);
    }
    else if (dwMaskFlags == uv_float3)
    {
      ve.Type = GXDECLTYPE_FLOAT3;
      mesh::InlAppendVertDecl<float3>(pElements, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);
    }
    else if (dwMaskFlags == uv_float4)
    {
      ve.Type = GXDECLTYPE_FLOAT4;
      mesh::InlAppendVertDecl<float4>(pElements, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);
    }
  }

  template<typename _Ty>
  inline void _SetVertexElementT(GXLPVOID lpFirstElement, GXUINT nStride, const _Ty& Value, GXSIZE_T nVertCount)
  {
    for(GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
      *(_Ty*)lpFirstElement = Value;
      lpFirstElement = ((GXBYTE*)lpFirstElement + nStride);
    }
  }

  GXBOOL ClearVertexElement(GXLPVOID lpFirstElement, GXSIZE_T cbElement, GXUINT nStride, GXSIZE_T nVertCount)
  {
    if(cbElement == 4) {
      _SetVertexElementT<GXDWORD>(lpFirstElement, nStride, 0, nVertCount);
    }
    else if(cbElement == 8) {
      _SetVertexElementT<float2>(lpFirstElement, nStride, 0.0f, nVertCount);
    }
    else if(cbElement == 12) {
      _SetVertexElementT<float3>(lpFirstElement, nStride, 0.0f, nVertCount);
    }
    else if(cbElement == 16) {
      _SetVertexElementT<float4>(lpFirstElement, nStride, 0.0f, nVertCount);
    }
    else
    {
      for(GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
        memset(lpFirstElement, 0, cbElement);
        lpFirstElement = ((GXBYTE*)lpFirstElement + nStride);
      }
    }
    return TRUE;
  }

  GXBOOL SetVertexElement(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID pValue, GXSIZE_T nVertCount)
  {
    if(cbElementStride == 4) {
      _SetVertexElementT<GXDWORD>(lpFirstElement, nStride, *(GXDWORD*)pValue, nVertCount);
    }
    else if(cbElementStride == 8) {
      _SetVertexElementT<float2>(lpFirstElement, nStride, *(float2*)pValue, nVertCount);
    }
    else if(cbElementStride == 12) {
      _SetVertexElementT<float3>(lpFirstElement, nStride, *(float3*)pValue, nVertCount);
    }
    else if(cbElementStride == 16) {
      _SetVertexElementT<float4>(lpFirstElement, nStride, *(float4*)pValue, nVertCount);
    }
    else return FALSE;
    return TRUE;
  }

  template<typename _Ty>
  inline void _CopyVertexElementT(GXLPVOID lpFirstElement, GXUINT nStride, _Ty* lpSource, GXSIZE_T nVertCount)
  {
    for(GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
      *(_Ty*)lpFirstElement = *lpSource++;
      lpFirstElement = ((GXBYTE*)lpFirstElement + nStride);
    }
  }

  GXBOOL CopyVertexElementFromStream(GXLPVOID lpFirstElement, GXUINT cbElementStride, GXUINT nStride, GXLPCVOID lpSourceStream, GXSIZE_T nVertCount)
  {
    if(cbElementStride == 4) {
      _CopyVertexElementT<GXDWORD>(lpFirstElement, nStride, (GXDWORD*)lpSourceStream, nVertCount);
    }
    else if(cbElementStride == 8) {
      _CopyVertexElementT<float2>(lpFirstElement, nStride, (float2*)lpSourceStream, nVertCount);
    }
    else if(cbElementStride == 12) {
      _CopyVertexElementT<float3>(lpFirstElement, nStride, (float3*)lpSourceStream, nVertCount);
    }
    else if(cbElementStride == 16) {
      _CopyVertexElementT<float4>(lpFirstElement, nStride, (float4*)lpSourceStream, nVertCount);
    }
    else
    {
      for(GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
        memcpy(lpFirstElement, lpSourceStream, cbElementStride);
        lpSourceStream = ((GXBYTE*)lpSourceStream + cbElementStride);
        lpFirstElement = ((GXBYTE*)lpFirstElement + nStride);
      }
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _Ty>
  inline void _CopyVertexElementT(void* lpDestStream, GXUINT nDestStride, const void* lpSrcStream, GXUINT nSrcStride, GXSIZE_T nVertCount)
  {
    for (GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
      *reinterpret_cast<_Ty*>(lpDestStream) = *reinterpret_cast<const _Ty*>(lpSrcStream);
      lpDestStream = ((GXBYTE*)lpDestStream + nDestStride);
      lpSrcStream = ((GXBYTE*)lpSrcStream + nSrcStride);
    }
  }

  GXBOOL GXDLL CopyVertexElementFromStream(GXLPVOID lpDestStream, GXUINT nDestStride, GXLPCVOID lpSrcStream, GXUINT nSrcStride, GXUINT cbElementStride, GXSIZE_T nVertCount)
  {
    if (cbElementStride == 4) {
      _CopyVertexElementT<GXDWORD>(lpDestStream, nDestStride, lpSrcStream, nSrcStride, nVertCount);
    }
    else if (cbElementStride == 8) {
      _CopyVertexElementT<float2>(lpDestStream, nDestStride, lpSrcStream, nSrcStride, nVertCount);
    }
    else if (cbElementStride == 12) {
      _CopyVertexElementT<float3>(lpDestStream, nDestStride, lpSrcStream, nSrcStride, nVertCount);
    }
    else if (cbElementStride == 16) {
      _CopyVertexElementT<float4>(lpDestStream, nDestStride, lpSrcStream, nSrcStride, nVertCount);
    }
    else
    {
      for (GXUINT nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++) {
        memcpy(lpDestStream, lpSrcStream, cbElementStride);
        lpDestStream = ((GXBYTE*)lpDestStream + nDestStride);
        lpSrcStream  = ((GXBYTE*)lpSrcStream + nSrcStride);
      }
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _VIndexT>
  GXBOOL CalculateNormalsT(
    float3*         pNormals, 
    const float3*   pVertices, 
    GXSIZE_T        nVertCount, 
    const _VIndexT* pIndices, 
    GXSIZE_T        nFaceCount, 
    GXUINT          nNormalStride = NULL, 
    GXUINT          nVertexStride = NULL)
  {
    if(nNormalStride == 0) {
      nNormalStride = sizeof(float3);
    }
    if(nVertexStride == 0) {
      nVertexStride = sizeof(float3);
    }

    ClearVertexElement(pNormals, sizeof(float3), nNormalStride, nVertCount);

    for(GXSIZE_T nFaceIndex = 0; nFaceIndex < nFaceCount; nFaceIndex++)
    {
      const _VIndexT ia = *pIndices++;
      const _VIndexT ib = *pIndices++;
      const _VIndexT ic = *pIndices++;

      const float3& va = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ia);
      const float3& vb = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ib);
      const float3& vc = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ic);

      float3 vNormal = float3::cross(vb - va, vc - va);
      vNormal.normalize();

      float3& vna = mesh::ElementFromStream<float3>(pNormals, nNormalStride, ia);
      float3& vnb = mesh::ElementFromStream<float3>(pNormals, nNormalStride, ib);
      float3& vnc = mesh::ElementFromStream<float3>(pNormals, nNormalStride, ic);

      vna += vNormal;
      vnb += vNormal;
      vnc += vNormal;
    }

    float3* pNormalPtr = pNormals;
    for(GXSIZE_T nVertIndex = 0; nVertIndex < nVertCount; nVertIndex++)
    {
      const float fLenSqu = pNormalPtr->lengthsquare();
      if(fLenSqu != 0 && fLenSqu != 1.0f) {
        (*pNormalPtr) *= (1.0f / sqrt(fLenSqu));
      }
      pNormalPtr = (float3*)((GXBYTE*)pNormalPtr + nNormalStride);
    }
    return TRUE;
  }

  // 16-bits indices
  GXBOOL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex* pIndices, GXSIZE_T nFaceCount)
  {
    ASSERT(aNormals.size() == aVertices.size());
    return mesh::CalculateNormalsT<VIndex>(&aNormals.front(), &aVertices.front(), aVertices.size(), pIndices, nFaceCount);
  }

  GXBOOL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex>& aIndices)
  {
    ASSERT(aNormals.size() == aVertices.size());
    return mesh::CalculateNormalsT<VIndex>(&aNormals.front(), &aVertices.front(), 
      aVertices.size(), &aIndices.front(), aIndices.size() / 3);
  }

  GXBOOL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, 
    const VIndex* pIndices, int nFaceCount, GXUINT nNormalStride, GXUINT nVertexStride)
  {
    return CalculateNormalsT<VIndex>(pNormals, pVertices, nVertCount, pIndices, nFaceCount, nNormalStride, nVertexStride);
  }

  // 32-bits indices
  GXBOOL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const VIndex32* pIndices, GXSIZE_T nFaceCount)
  {
    ASSERT(aNormals.size() == aVertices.size());
    return mesh::CalculateNormalsT<VIndex32>(&aNormals.front(), &aVertices.front(), 
      aVertices.size(), pIndices, nFaceCount);
  }

  GXBOOL CalculateNormals(clvector<float3>& aNormals, const clvector<float3>& aVertices, const clvector<VIndex32>& aIndices)
  {
    ASSERT(aNormals.size() == aVertices.size());
    return mesh::CalculateNormalsT<VIndex32>(&aNormals.front(), &aVertices.front(), 
      aVertices.size(), &aIndices.front(), aIndices.size() / 3);
  }

  GXBOOL CalculateNormals(float3* pNormals, const float3* pVertices, int nVertCount, 
    const VIndex32* pIndices, int nFaceCount, GXUINT nNormalStride, GXUINT nVertexStride)
  {
    return CalculateNormalsT<VIndex32>(pNormals, pVertices, nVertCount, pIndices, nFaceCount, nNormalStride, nVertexStride);
  }



  GXBOOL CalculateTBs(
    float4*       pTangents, 
    float4*       pBinormals, 
    const float3* pVertices, 
    const float3* pNormals, 
    const float2* pTexcoord, 
    int           nVertCount, 
    const VIndex* pIndices,
    int           nFaceCount,
    GXUINT        nTangentStride,
    GXUINT        nBinormalStride,
    GXUINT        nVertexStride,
    GXUINT        nNormalStride,
    GXUINT        nTexcoordStride)
  {
    CLBREAK; // 没测试
    if(pTangents == NULL && pBinormals == NULL) {
      return FALSE;
    }
    if(nTangentStride == 0) {
      nTangentStride = sizeof(float4);
    }
    if(nBinormalStride == 0) {
      nBinormalStride = sizeof(float4);
    }
    if(nVertexStride == 0) {
      nVertexStride = sizeof(float3);
    }
    if(nNormalStride == 0) {
      nNormalStride = sizeof(float3);
    }
    if(nTexcoordStride == 0) {
      nTexcoordStride = sizeof(float2);
    }

    ClearVertexElement(pTangents, sizeof(float4), nTangentStride, nVertCount);
    ClearVertexElement(pBinormals, sizeof(float4), nBinormalStride, nVertCount);

    for(int i=0; i < nFaceCount; i++)
    {
      const int ia = pIndices[i * 3];
      const int ib = pIndices[i * 3 + 1];
      const int ic = pIndices[i * 3 + 2];

      const float3& va = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ia);
      const float3& vb = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ib);
      const float3& vc = mesh::ElementFromStream<float3>(pVertices, nVertexStride, ic);

      const float2& ta = mesh::ElementFromStream<float2>(pTexcoord, nTexcoordStride, ia);
      const float2& tb = mesh::ElementFromStream<float2>(pTexcoord, nTexcoordStride, ib);
      const float2& tc = mesh::ElementFromStream<float2>(pTexcoord, nTexcoordStride, ic);

      const float3 p1 = vb - va;
      const float3 p2 = vc - va;

      const float2 t1 = tb - ta;
      const float2 t2 = tc - ta;

      //float du1 = m_aVerts[v1].tu - m_aVerts[v0].tu;
      //float dv1 = m_aVerts[v1].tv - m_aVerts[v0].tv;
      //float du2 = m_aVerts[v2].tu - m_aVerts[v0].tu;
      //float dv2 = m_aVerts[v2].tv - m_aVerts[v0].tv;

      const float fValue0 = (t1.x * t2.y - t2.x * t1.y);
      const float fValue1 = (t1.x * t2.y - t2.x * t1.y);
      if( fabs(fValue0) < FLT_EPSILON || fabs(fValue1) < FLT_EPSILON)
        continue;

      float3 T = (p1 * t2.y - p2 * t1.y) / fValue0;
      //float3 B = (p2 * t1.x - p1 * t2.x) / fValue1;

      if(pTangents != NULL)
      {
        float3& tangent_a = mesh::ElementFromStream<float3>(pTangents, nTangentStride, ia);
        float3& tangent_b = mesh::ElementFromStream<float3>(pTangents, nTangentStride, ib);
        float3& tangent_c = mesh::ElementFromStream<float3>(pTangents, nTangentStride, ic);

        tangent_a += T;
        tangent_b += T;
        tangent_c += T;
      }

      //tangents[v0] = tangents[v0] + T;
      //tangents[v1] = tangents[v1] + T;
      //tangents[v2] = tangents[v2] + T;

      if(pBinormals != NULL)
      {
        float3& binormal_a = mesh::ElementFromStream<float3>(pBinormals, nBinormalStride, ia);
        float3& binormal_b = mesh::ElementFromStream<float3>(pBinormals, nBinormalStride, ib);
        float3& binormal_c = mesh::ElementFromStream<float3>(pBinormals, nBinormalStride, ic);

        binormal_a += T;
        binormal_b += T;
        binormal_c += T;
      }
      //binormals[v0] = binormals[v0] + B;
      //binormals[v1] = binormals[v1] + B;
      //binormals[v2] = binormals[v2] + B;
    }

    for(int i = 0; i < nVertCount; i++)
    {
      const float3& vNormal = mesh::ElementFromStream<float3>(pNormals, nNormalStride, i);

      if(pTangents != NULL)
      {
        float3& vTangent  = mesh::ElementFromStream<float3>(pTangents, nTangentStride, i);
        vTangent.normalize();

        float3 vFinalTangent = vTangent - vNormal * float3::dot(vNormal, vTangent);
        vFinalTangent.normalize();
        (*(float4*)&vTangent).set(vFinalTangent.x, vFinalTangent.y, vFinalTangent.z, 1.0f);
      }


      if(pBinormals != NULL)
      {
        float3& vBinormal = mesh::ElementFromStream<float3>(pBinormals, nBinormalStride, i);
        vBinormal.normalize();

        float3 vFinalBinormal = vBinormal - vNormal * float3::dot(vNormal, vBinormal);
        vFinalBinormal.normalize();
        (*(float4*)&vBinormal).set(vFinalBinormal.x, vFinalBinormal.y, vFinalBinormal.z, 1.0f);
      }
    }
    return TRUE;
  }

  GXVOID CalculateAABB(GVNode::AABB& aabb, const float3* pVertices, int nVertCount, GXUINT nVertexStride)
  {
    aabb.Clear();
    if(nVertexStride == 0) {
      nVertexStride = sizeof(float3);
    }

    for(int i = 0; i < nVertCount; i++)
    {
      aabb.AppendVertex(*pVertices);
      pVertices = (float3*)((GXBYTE*)pVertices + nVertexStride);
    }
    //aabb.UpdateCenterExtent();
  }

  GXVOID CalculateAABBFromIndices(GVNode::AABB& aabb, GXLPCVOID pVertices, const VIndex* pIndex, GXSIZE_T nIndexCount, GXUINT nVertexStride)
  {
    if(nVertexStride == 0) {
      nVertexStride = sizeof(float3);
    }
    aabb.Clear();
    for(GXSIZE_T i = 0; i < nIndexCount; i++)
    {
      float3* v = (float3*)((GXBYTE*)pVertices + nVertexStride * pIndex[i]);
      aabb.AppendVertex(*v);
    }
  }

  template<typename _VIndexT>
  GXBOOL ReverseVerticesArrayT( float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, _VIndexT* pIndices, GXSIZE_T nFaceCount )
  {
    const GXSIZE_T nCountV = nCount >> 1;
    const GXSIZE_T nEnd = nBegin + nCount; // 不含
    float3* pVerticesBack = pVertices + nEnd - 1;
    pVertices += nBegin;
    for(GXSIZE_T i = 0; i < nCountV; i++)
    {
      clSwap(*pVertices, *pVerticesBack);
      pVertices++;
      pVerticesBack--;
    }
#define REVERSE_INDEX(_I)      if(_I >= nBegin && _I < nEnd) { _I = (_VIndexT)((nEnd - 1) - (_I - nBegin)); }

    if(pIndices && nFaceCount > 0)
    {
      for(GXUINT i = 0; i < nFaceCount; i++)
      {
        REVERSE_INDEX(pIndices[0]);
        REVERSE_INDEX(pIndices[1]);
        REVERSE_INDEX(pIndices[2]);
        pIndices += 3;
      }
    }
#undef REVERSE_INDEX
    return TRUE;
  }

  GXBOOL GXDLL ReverseVerticesArray( float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex* pIndices, GXSIZE_T nFaceCount )
  {
    return ReverseVerticesArrayT<VIndex>(pVertices, nBegin, nCount, pIndices, nFaceCount);
  }

  GXBOOL GXDLL ReverseVerticesArray( float3* pVertices, GXSIZE_T nBegin, GXSIZE_T nCount, VIndex32* pIndices, GXSIZE_T nFaceCount )
  {
    return ReverseVerticesArrayT<VIndex32>(pVertices, nBegin, nCount, pIndices, nFaceCount);
  }

  GXBOOL GXDLL TransformPosition( const float4x4& mat, float3* pVertices, GXUINT nCount, GXUINT nStride )
  {
    if(nStride == 0) {
      nStride = sizeof(float3);
    }

    for(GXUINT i = 0; i < nCount; i++)
    {
      *pVertices *= mat;
      pVertices = (float3*)((GXBYTE*)pVertices + nStride);
    }
    return TRUE;
  }

  GXBOOL GXDLL TransformVectors( const float4x4& mat, float3* pVertors, GXUINT nCount, GXUINT nStride )
  {
    if(nStride == 0) {
      nStride = sizeof(float3);
    }

    for(GXUINT i = 0; i < nCount; i++)
    {
      pVertors->MulAsMatrix3x3(mat);      
      pVertors = (float3*)((GXBYTE*)pVertors + nStride);
    }
    return TRUE;
  }

} // namespace mesh

GXBOOL GVMESHDATA::Check(const GVMESHDATA* pMesh)
{
  if(pMesh->nVertexCount <= 0 || pMesh->nIndexCount <= 0 || pMesh->nIndexCount % 3 != 0) {
    CLOG_ERROR("%s : Invalid vertex or index count.\n", __FUNCTION__);
    return FALSE;
  }
  if(pMesh->pVertices == NULL) {
    CLOG_ERROR("%s : vertex can not be empty.\n", __FUNCTION__);
    return FALSE;
  }
  if(pMesh->pIndices == NULL) {
    CLOG_ERROR("%s : index can not be empty.\n", __FUNCTION__);
    return FALSE;
  }

  if(pMesh->pBoneWeights != NULL && pMesh->pBoneWeights32 != NULL) {
    CLOG_WARNING("%s : Both pBoneWeights and pBoneWeights32 are not empty.\n", __FUNCTION__);
  }

  if(pMesh->pColors != NULL && pMesh->pColors32 != NULL) {
    CLOG_WARNING("%s : Both pColors and pColors32 are not empty.\n", __FUNCTION__);
  }
  return TRUE;
}

GXSIZE_T GVMESHDATA::Build(const GVMESHDATA* pMeshComponent, GXLPVERTEXELEMENT lpVertDelement)
{
  mesh::VertElementArray aVertDecl;
  GXVERTEXELEMENT ve;
  ve.Offset = 0;
  ve.Type = GXDECLTYPE_FLOAT3;
  ve.Method = GXDECLMETHOD_DEFAULT;
  ve.Usage = GXDECLUSAGE_POSITION;
  ve.UsageIndex = 0;
  aVertDecl.push_back(ve);
  ve.Offset += sizeof(float3);

  mesh::InlAppendVertDecl<float3>(pMeshComponent->pNormals, GXDECLUSAGE_NORMAL, aVertDecl, ve);
  mesh::InlAppendVertDecl<float4>(pMeshComponent->pTangents, GXDECLUSAGE_TANGENT, aVertDecl, ve);
  mesh::InlAppendVertDecl<float3>(pMeshComponent->pBinormals, GXDECLUSAGE_BINORMAL, aVertDecl, ve);

  ve.Type = GXDECLTYPE_FLOAT2;
  mesh::InlAppendVertDecl<float2>(pMeshComponent->pTexcoord0, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);

  ve.UsageIndex = 1;
  mesh::InlAppendVertDecl<float2>(pMeshComponent->pTexcoord1, GXDECLUSAGE_TEXCOORD, aVertDecl, ve);
  
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord2, 2, pMeshComponent->dwFlags & MESHDATA_FLAG_UV2_MASK, MESHDATA_FLAG_UV2_FLOAT, MESHDATA_FLAG_UV2_FLOAT2, MESHDATA_FLAG_UV2_FLOAT3, MESHDATA_FLAG_UV2_FLOAT4);
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord3, 3, pMeshComponent->dwFlags & MESHDATA_FLAG_UV3_MASK, MESHDATA_FLAG_UV3_FLOAT, MESHDATA_FLAG_UV3_FLOAT2, MESHDATA_FLAG_UV3_FLOAT3, MESHDATA_FLAG_UV3_FLOAT4);
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord4, 4, pMeshComponent->dwFlags & MESHDATA_FLAG_UV4_MASK, MESHDATA_FLAG_UV4_FLOAT, MESHDATA_FLAG_UV4_FLOAT2, MESHDATA_FLAG_UV4_FLOAT3, MESHDATA_FLAG_UV4_FLOAT4);
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord5, 5, pMeshComponent->dwFlags & MESHDATA_FLAG_UV5_MASK, MESHDATA_FLAG_UV5_FLOAT, MESHDATA_FLAG_UV5_FLOAT2, MESHDATA_FLAG_UV5_FLOAT3, MESHDATA_FLAG_UV5_FLOAT4);
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord6, 6, pMeshComponent->dwFlags & MESHDATA_FLAG_UV6_MASK, MESHDATA_FLAG_UV6_FLOAT, MESHDATA_FLAG_UV6_FLOAT2, MESHDATA_FLAG_UV6_FLOAT3, MESHDATA_FLAG_UV6_FLOAT4);
  mesh::TryAppendTexcoordVertDecl(aVertDecl, ve, pMeshComponent->pTexcoord7, 7, pMeshComponent->dwFlags & MESHDATA_FLAG_UV7_MASK, MESHDATA_FLAG_UV7_FLOAT, MESHDATA_FLAG_UV7_FLOAT2, MESHDATA_FLAG_UV7_FLOAT3, MESHDATA_FLAG_UV7_FLOAT4);

  ve.UsageIndex = 0;
  if(pMeshComponent->pBoneWeights32 != NULL || pMeshComponent->pBoneWeights != NULL)
  {
    ve.Usage = GXDECLUSAGE_BLENDWEIGHT;
    ve.Type = GXDECLTYPE_D3DCOLOR;
    aVertDecl.push_back(ve);
    ve.Offset += sizeof(GXDWORD);
  }

  if(pMeshComponent->pBoneIndices != NULL || pMeshComponent->pBoneIndices32 != NULL)
  {
    ve.Usage = GXDECLUSAGE_BLENDINDICES;
    ve.Type = GXDECLTYPE_D3DCOLOR;
    aVertDecl.push_back(ve);
    ve.Offset += sizeof(GXDWORD);
  }

  if(pMeshComponent->pColors32 != NULL)
  {
    ve.Usage = GXDECLUSAGE_COLOR;
    ve.Type = GXDECLTYPE_D3DCOLOR;
    aVertDecl.push_back(ve);
    ve.Offset += sizeof(GXDWORD);
  }
  else if(pMeshComponent->pColors != NULL)
  {
    ve.Usage = GXDECLUSAGE_COLOR;
    ve.Type = GXDECLTYPE_FLOAT4;
    aVertDecl.push_back(ve);
    ve.Offset += sizeof(float4);
  }
  const GXVERTEXELEMENT ve_end = GXDECL_END();
  aVertDecl.push_back(ve_end);
  memcpy(lpVertDelement, &aVertDecl.front(), aVertDecl.size() * sizeof(GXVERTEXELEMENT));
  return aVertDecl.size();
}

void GVMESHDATA::Destroy(GVMESHDATA* pMeshData)
{
  pMeshData->nVertexCount = 0;
  pMeshData->nIndexCount = 0;
  SAFE_DELETE(pMeshData->pVertices);
  SAFE_DELETE(pMeshData->pNormals);
  SAFE_DELETE(pMeshData->pTangents);
  SAFE_DELETE(pMeshData->pBinormals);
  SAFE_DELETE(pMeshData->pTexcoord0);
  SAFE_DELETE(pMeshData->pTexcoord1);
  SAFE_DELETE(pMeshData->pTexcoord2);
  SAFE_DELETE(pMeshData->pTexcoord3);
  SAFE_DELETE(pMeshData->pTexcoord4);
  SAFE_DELETE(pMeshData->pTexcoord5);
  SAFE_DELETE(pMeshData->pTexcoord6);
  SAFE_DELETE(pMeshData->pTexcoord7);
  SAFE_DELETE(pMeshData->pBoneWeights);
  SAFE_DELETE(pMeshData->pBoneWeights32);
  SAFE_DELETE(pMeshData->pBoneIndices);
  SAFE_DELETE(pMeshData->pBoneIndices32);
  SAFE_DELETE(pMeshData->pColors);
  SAFE_DELETE(pMeshData->pColors32);
  SAFE_DELETE(pMeshData->pIndices);
}
