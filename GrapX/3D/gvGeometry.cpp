// 全局头文件
#include <GrapX.H>
#include <User/GrapX.Hxx>

// 标准接口
#include <Include/GUnknown.H>
#include <Include/GResource.H>
#include <Include/GPrimitive.h>
#include <Include/GXGraphics.H>
#include <Include/GShader.H>

// 私有头文件
//#include <clstd/clTree.H>
//#include <clstd/Smart/smartstream.h>
#include <Utility/VertexDecl.H>
#include "gxUtility.h"

#include "gxError.H"

//#include <3D/gvNode.h>
//#include <3D/gvMesh.h>
//#include <3D/gvGeometry.h>
//#include <3D/gvScene.h>
#include "3D/GrapVR.H"
namespace PrimitiveIndicesUtility
{
  template<typename _VIndexTy>
  void FillIndicesAsQuadVertexArrayCW(int xSeg, int ySeg, int nPitch, int nBaseIndex, clvector<_VIndexTy>& aIndices);

  template<typename _VIndexTy>
  void FillIndicesAsQuadVertexArrayCCW(int xSeg, int ySeg, int nPitch, int nBaseIndex, clvector<_VIndexTy>& aIndices);
} // namespace PrimitiveIndicesUtility

GVGeometry::GVGeometry(GXGraphics* pGraphics, GEOTYPE eType, const float3& vMin, const float3& vMax)
  : GVMesh        (pGraphics, GXMAKEFOURCC('G','E','O','Y'))
  , m_eType       (GXPT_POINTLIST)
  //, m_nPrimiCount (0)
  //, m_nVertCount  (0)
  //, m_nStartIndex (0)
  , m_eGeoType    (eType)
  //, m_pMtlInst    (NULL)
  //, m_pPrimitive  (NULL)
{
  m_aabbLocal.vMin = vMin;
  m_aabbLocal.vMax = vMax;
  //m_aabbLocal.UpdateCenterExtent();
}

GVGeometry::GVGeometry()
  : GVMesh        (NULL, GXMAKEFOURCC('G','E','O','Y'))
  , m_eType       (GXPT_POINTLIST)
  , m_eGeoType    (GT_UNDEFINED)
{
  m_aabbLocal.vMin = FLT_MAX;
  m_aabbLocal.vMax = FLT_MIN;
  //m_aabbLocal.UpdateCenterExtent();
}

GVGeometry::GVGeometry(GXGraphics* pGraphics, GEOTYPE eType)
  : GVMesh        (pGraphics, GXMAKEFOURCC('G','E','O','Y'))
  , m_eType       (GXPT_POINTLIST)
  //, m_nPrimiCount (0)
  //, m_nVertCount  (0)
  //, m_nStartIndex (0)
  , m_eGeoType    (eType)
  //, m_pMtlInst    (NULL)
  //, m_pPrimitive  (NULL)
{
}

GVGeometry::~GVGeometry()
{
  SAFE_RELEASE(m_pMtlInst);
  SAFE_RELEASE(m_pPrimitive);
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVGeometry::AddRef()
{
  GXHRESULT hr = GVNode::AddRef();
  return hr;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXBOOL GVGeometry::InitializeAsAABB(GXGraphics* pGraphics, GXCOLOR clr)
{
  m_eType = GXPT_LINELIST;
  m_nPrimiCount = 12;

  GXVERTEX_P3F_C1D pVertices[8];
  GXWORD pIndices[24];

  // TODO: 貌似这个不保存,设备重置后就消失了
  //m_pPrimitive->Lock(0, 0, 0, 0, (void**)&pVertices, &pIndices);

  pVertices[0].pos.set(m_aabbLocal.vMin.x, m_aabbLocal.vMin.y, m_aabbLocal.vMin.z);
  pVertices[1].pos.set(m_aabbLocal.vMin.x, m_aabbLocal.vMin.y, m_aabbLocal.vMax.z);
  pVertices[2].pos.set(m_aabbLocal.vMax.x, m_aabbLocal.vMin.y, m_aabbLocal.vMax.z);
  pVertices[3].pos.set(m_aabbLocal.vMax.x, m_aabbLocal.vMin.y, m_aabbLocal.vMin.z);

  pVertices[4].pos.set(m_aabbLocal.vMin.x, m_aabbLocal.vMax.y, m_aabbLocal.vMin.z);
  pVertices[5].pos.set(m_aabbLocal.vMin.x, m_aabbLocal.vMax.y, m_aabbLocal.vMax.z);
  pVertices[6].pos.set(m_aabbLocal.vMax.x, m_aabbLocal.vMax.y, m_aabbLocal.vMax.z);
  pVertices[7].pos.set(m_aabbLocal.vMax.x, m_aabbLocal.vMax.y, m_aabbLocal.vMin.z);
  m_nVertCount = 8;

  for(int i = 0; i < 8; i++)
  {
    pVertices[i].color = clr;
  }

  pIndices[0] = 0;    pIndices[1] = 1;
  pIndices[2] = 1;    pIndices[3] = 2;
  pIndices[4] = 2;    pIndices[5] = 3;
  pIndices[6] = 3;    pIndices[7] = 0;

  pIndices[8]  = 4;   pIndices[9]  = 5;
  pIndices[10] = 5;   pIndices[11] = 6;
  pIndices[12] = 6;   pIndices[13] = 7;
  pIndices[14] = 7;   pIndices[15] = 4;

  pIndices[16] = 0;   pIndices[17] = 4;
  pIndices[18] = 1;   pIndices[19] = 5;
  pIndices[20] = 2;   pIndices[21] = 6;
  pIndices[22] = 3;   pIndices[23] = 7;

  //m_pPrimitive->Unlock();

  pGraphics->CreatePrimitiveVI(
    &m_pPrimitive, NULL, MOGetSysVertexDecl(GXVD_P3F_C1D), 
    GXRU_DEFAULT, 24, 8, sizeof(GXVERTEX_P3F_C1D), pIndices, pVertices);

  return TRUE;
}

GXBOOL GVGeometry::InitializeAsAxis(GXGraphics* pGraphics, const float3& vPos, float fExtent, int nLevel)
{
  GXBOOL bval = TRUE;
  if(nLevel == 0)
  {
    GXVERTEX_P3F_C1D pVertices[6];
    GXWORD           pIndices[6];

    pVertices[0].pos = vPos;
    pVertices[1].pos = vPos;
    pVertices[2].pos = vPos;
    pVertices[3].pos = vPos + float3(fExtent, 0, 0);
    pVertices[4].pos = vPos + float3(0, fExtent, 0);
    pVertices[5].pos = vPos + float3(0, 0, fExtent);

    pVertices[0].color = 0xffff0000;
    pVertices[1].color = 0xff00ff00;
    pVertices[2].color = 0xff0000ff;
    pVertices[3].color = 0xffff0000;
    pVertices[4].color = 0xff00ff00;
    pVertices[5].color = 0xff0000ff;
    m_nVertCount = 6;

    pIndices[0] = 0;    pIndices[1] = 3;
    pIndices[2] = 1;    pIndices[3] = 4;
    pIndices[4] = 2;    pIndices[5] = 5;

    CreatePrimitive(pGraphics, GXPT_LINELIST, 3, MOGetSysVertexDecl(GXVD_P3F_C1D), pVertices, 6, pIndices, 6);
  }
  else
  {
    GVGeometry* pSubGeo[6];
    const float fAxisLength = fExtent;
    const float fAxisRaidus = fAxisLength * 0.02f;
    const float fArrowRadius = fAxisRaidus * 3.0f;
    const float fArrow = fAxisRaidus * 12.0f;
    float4x4 matTransform;
    const GXDWORD dwFlags = GXVF_NORMAL|GXVF_COLOR;
    GVGeometry::CreateCylinder(pGraphics, fAxisLength * 0.5f, fAxisRaidus, fAxisLength, 1, 1, 6, 
      TRUE, 0xff00ff00, &pSubGeo[0], NULL, 0x4, dwFlags);
    GVGeometry::CreateCone(pGraphics, fAxisLength + fArrow * 0.5f, 0.0f, fArrowRadius, fArrow, 1, 1, 6, 
      TRUE, 0xff00ff00, &pSubGeo[1], NULL, 0x7, dwFlags);
    matTransform.identity();
    matTransform._11 = 0.0f;
    matTransform._22 = 0.0f;
    matTransform._12 = -1.0f;
    matTransform._21 = 1.0f;
    GVGeometry::CreateCylinder(pGraphics, fAxisLength * 0.5f, fAxisRaidus, fAxisLength, 1, 1, 6, 
      TRUE, 0xffff0000, &pSubGeo[2], &matTransform, 0x4, dwFlags);
    GVGeometry::CreateCone(pGraphics, fAxisLength + fArrow * 0.5f, 0.0f, fArrowRadius, fArrow, 1, 1, 6, 
      TRUE, 0xffff0000, &pSubGeo[3], &matTransform, 0x7, dwFlags);

    matTransform.identity();
    matTransform._22 = 0.0f;
    matTransform._33 = 0.0f;
    matTransform._23 = 1.0f;
    matTransform._32 = -1.0f;
    GVGeometry::CreateCylinder(pGraphics, fAxisLength * 0.5f, fAxisRaidus, fAxisLength, 1, 1, 6, 
      TRUE, 0xff0000ff, &pSubGeo[4], &matTransform, 0x4, dwFlags);
    GVGeometry::CreateCone(pGraphics, fAxisLength + fArrow * 0.5f, 0.0f, fArrowRadius, fArrow, 1, 1, 6,
      TRUE, 0xff0000ff, &pSubGeo[5], &matTransform, 0x7, dwFlags);

    static GXLPCSTR szNames[] = {"YAxis", "YArrow", "XAxis", "XArrow", "ZAxis", "ZArrow"};
    if( ! IntInitializeAsContainer(pGraphics, (GVNode**)&pSubGeo, 6))
    {
      bval = FALSE;
    }
    else
    {
      AABB aabb;
      m_eGeoType = GT_AXIS_MESH;
      for(int i = 0; i < 6; i++) {
        pSubGeo[i]->SetName(szNames[i]);
        pSubGeo[i]->GetRelativeAABB(aabb);
        m_aabbLocal.Merge(aabb);
      }
    }
  }
  return bval;
}

GXBOOL GVGeometry::InitializeAsQuadPlane(GXGraphics* pGraphics, const float3& vPos, const float3& vDirection, const float3& vUp, const float2& vExtent, GXUINT xSeg, GXUINT ySeg, GXDWORD dwVertexFlags)
{
  float3 zaxis = float3::normalize(vDirection);
  float3 xaxis = float3::normalize(float3::cross(vUp, zaxis));
  float3 yaxis = float3::cross(zaxis, xaxis);

  //typedef clvector<GXVERTEX_P3T2N3F>  VerticesArray;
  //typedef clvector<VIndex>            IndicesArray;

  typedef clvector<float3> Float3Array;
  typedef clvector<float2> Float2Array;

  GVMESHDATA MeshData;
  InlSetZeroT(MeshData);

  //GXVERTEX_P3T2N3F vectex;
  //VerticesArray aVertices;
  Float3Array   aNormals;
  Float3Array   aVertices;
  Float2Array   aTexcoords;
  IndicesArray  aIndices;

  aVertices.reserve((xSeg + 1) * (ySeg + 1));
  aTexcoords.reserve((xSeg + 1) * (ySeg + 1));
  aNormals.reserve((xSeg + 1) * (ySeg + 1));
  aIndices.reserve(xSeg * ySeg * 6);

  float3 vStartPos = vPos - xaxis * vExtent.x * 0.5f - yaxis * vExtent.y * 0.5f;
  float2 vGridSize(vExtent.x / (float)xSeg, vExtent.y / (float)ySeg);
  float2 vTGridSize(1.0f / (float)xSeg, 1.0f / (float)ySeg);

  for(GXUINT y = 0; y <= ySeg; y++)
  {
    float v = (float)y * vTGridSize.y;
    for(GXUINT x = 0; x <= xSeg; x++)
    {
      //vectex.pos = ;
      //vectex.texcoord.x = (float)x * vGridSize.x;
      //vectex.texcoord.y = v;
      aNormals.push_back(vDirection);
      aVertices.push_back(vStartPos + xaxis * (float)x * vGridSize.x);
      aTexcoords.push_back(float2((float)x * vTGridSize.x, v));
    }
    vStartPos = vStartPos + yaxis * vGridSize.y;
  }

  PrimitiveIndicesUtility::FillIndicesAsQuadVertexArrayCW(xSeg, ySeg, xSeg + 1, 0, aIndices);

   //= (|GXVF_COLOR|)
  MeshData.nVertexCount = aVertices.size();
  MeshData.nIndexCount  = aIndices.size();
  MeshData.pIndices     = &aIndices.front();
  MeshData.pVertices    = &aVertices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshData.pNormals = &aNormals.front();
  }

  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshData.pTexcoord0 = &aTexcoords.front();
  }

  m_eType = GXPT_TRIANGLELIST;
  m_nPrimiCount = aIndices.size() / 3;
  m_nVertCount = aVertices.size();

  return IntCreateMesh(pGraphics, &MeshData);

  //pGraphics->CreatePrimitiveVI(&m_pPrimitive, NULL, 
  //  MOGetSysVertexDecl(GXVD_P3T2N3F), GXRU_DEFAULT, aIndices.size(), aVertices.size(), 0, &aIndices.front(), &aVertices.front());

  //return TRUE;
}

GXBOOL GVGeometry::CreatePrimitive(GXGraphics* pGraphics, GXPrimitiveType eType, int nPrimCount, GXLPCVERTEXELEMENT lpVertDecl, GXLPVOID lpVertics, int nVertCount, GXWORD* pIndices, int nIdxCount)
{
  GXBOOL bval = FALSE;
  m_eType       = eType;
  m_nPrimiCount = nPrimCount;
  m_nVertCount  = nVertCount;

  const GXUINT nStride = MOGetDeclVertexSize(lpVertDecl);
  if(GXSUCCEEDED(pGraphics->CreatePrimitiveVI(&m_pPrimitive, NULL,
    lpVertDecl, GXRU_DEFAULT, nIdxCount, nVertCount, nStride, pIndices, lpVertics)))
  {
    GXVERTEXELEMENT Desc;
    int nOffset = MOGetDeclOffset(lpVertDecl, GXDECLUSAGE_POSITION, 0, &Desc);
    if(Desc.Type == GXDECLTYPE_FLOAT3)
    {
      float3* pPos = (float3*)((GXLPBYTE)lpVertics + nOffset);
      for(int i = 0; i < nVertCount; i++)
      {
        m_aabbLocal.AppendVertex(*pPos);
        pPos = (float3*)((GXLPBYTE)pPos + nStride);
      }
    }
    //m_aabbLocal.UpdateCenterExtent();
    bval = TRUE;
  }
  return bval;
}

void GVGeometry::GetRenderDesc(GVRenderType eType, GVRENDERDESC* pRenderDesc)
{
  pRenderDesc->dwFlags         = m_dwFlags;
  pRenderDesc->ePrimType       = m_eType;
  pRenderDesc->pPrimitive      = m_pPrimitive;
  pRenderDesc->pMaterial       = m_pMtlInst;
  pRenderDesc->matWorld        = m_Transformation.GlobalMatrix;
  pRenderDesc->RenderQueue     = m_pMtlInst ? m_pMtlInst->GetRenderQueue() : 0;
  pRenderDesc->BaseVertexIndex = 0;

  pRenderDesc->MinIndex        = 0;
  pRenderDesc->NumVertices     = m_nVertCount;
  pRenderDesc->StartIndex      = m_nStartIndex;
  pRenderDesc->PrimitiveCount  = m_nPrimiCount;
}

//float GVGeometry::RayTrace(const Ray& ray, float3* pHit)
GXBOOL GVGeometry::RayTrace(const Ray& ray, NODERAYTRACE* pRayTrace)
{
  //float3 vHit;
  //float fRetLength;
  switch(m_eGeoType)
  {
  case GT_AABB:
    //float4x4 matInvAbs = float4x4::inverse(m_matAbsolute);
    //Ray RayLocal(ray.vOrigin * matInvAbs, ray.vDirection.MulAsMatrix3x3(matInvAbs));

    //if(clstd::RayIntersectAABB(RayLocal, m_aabbLocal, &fRetLength, NULL)) {
    //  if(pHit != NULL) {
    //    *pHit = ray.vOrigin + ray.vDirection * fRetLength;
    //  }
    //  return fRetLength * fRetLength;
    //}
    if(GVNode::RayTrace(ray, pRayTrace)) {
      pRayTrace->eType = NRTT_MESHFACE;
      return TRUE;
    }
    return FALSE;
  case GT_AXIS:
    // 没实现!
    //ASSERT(0);
    return FALSE;
  case GT_AXIS_MESH:
  case GT_CONE:
  case GT_TORUS:
  case GT_BOX:
  case GT_QUADPLANE:
    return GVMesh::RayTrace(ray, pRayTrace);
  default:
    return GVNode::RayTrace(ray, pRayTrace);
  }
  return FALSE;
}

//GXHRESULT GVGeometry::SetMaterialInst(GXMaterialInst* pMtlInst, GXDWORD dwFlags)
//{
//  if(bSetChild) {
//    GVGeometry* pChild = (GVGeometry*)m_pFirstChild;
//    while(pChild != NULL)
//    {
//      pChild->SetMaterialInst(pMtlInst, bSetChild);
//      pChild = (GVGeometry*)(pChild->m_pNext);
//    }
//  }
//
//  SAFE_RELEASE(m_pMtlInst);
//  m_pMtlInst = pMtlInst;
//  if(m_pMtlInst != NULL) {
//    return m_pMtlInst->AddRef();
//  }
//  return GX_OK;
//}
//////////////////////////////////////////////////////////////////////////
GXHRESULT GVGeometry::CreateBox(GXGraphics* pGraphics, CFloat3& vCenter, CFloat3& vExtent, GXCOLOR clr, GVGeometry** ppGeometry, GXDWORD dwVertexFlags)
{
  const float3 v[2] = {
    vCenter - vExtent,
    vCenter + vExtent
  };
  GVGeometry* pGeometry = NULL;
  try
  {
    pGeometry = new GVGeometry(pGraphics, GT_BOX, v[0], v[1]);
    if( ! InlCheckNewAndIncReference(pGeometry)) {
      return GX_FAIL;
    }

    VIndex aIndices[36] = {
      0 +  0, 1 +  0, 2 +  0,  0 +  0, 2 +  0, 3 +  0,
      0 +  4, 1 +  4, 2 +  4,  0 +  4, 2 +  4, 3 +  4,
      0 +  8, 1 +  8, 2 +  8,  0 +  8, 2 +  8, 3 +  8,
      0 + 12, 1 + 12, 2 + 12,  0 + 12, 2 + 12, 3 + 12,
      0 + 16, 1 + 16, 2 + 16,  0 + 16, 2 + 16, 3 + 16,
      0 + 20, 1 + 20, 2 + 20,  0 + 20, 2 + 20, 3 + 20,
    };
    GVMESHDATA MeshComp;
    InlSetZeroT(MeshComp);

    MeshComp.nVertexCount = 4 * 6;
    MeshComp.nIndexCount = 3 * 2 * 6;
    MeshComp.pVertices = new float3[MeshComp.nVertexCount];
    MeshComp.pIndices = aIndices;


    MeshComp.pVertices[ 0].set(v[0].x, v[0].y, v[1].z);
    MeshComp.pVertices[ 1].set(v[0].x, v[1].y, v[1].z);
    MeshComp.pVertices[ 2].set(v[0].x, v[1].y, v[0].z);
    MeshComp.pVertices[ 3].set(v[0].x, v[0].y, v[0].z);

    MeshComp.pVertices[ 4].set(v[1].x, v[0].y, v[0].z);
    MeshComp.pVertices[ 5].set(v[1].x, v[1].y, v[0].z);
    MeshComp.pVertices[ 6].set(v[1].x, v[1].y, v[1].z);
    MeshComp.pVertices[ 7].set(v[1].x, v[0].y, v[1].z);


    MeshComp.pVertices[ 8].set(v[1].x, v[0].y, v[0].z);
    MeshComp.pVertices[ 9].set(v[1].x, v[0].y, v[1].z);
    MeshComp.pVertices[10].set(v[0].x, v[0].y, v[1].z);
    MeshComp.pVertices[11].set(v[0].x, v[0].y, v[0].z);

    MeshComp.pVertices[12].set(v[0].x, v[1].y, v[0].z);
    MeshComp.pVertices[13].set(v[0].x, v[1].y, v[1].z);
    MeshComp.pVertices[14].set(v[1].x, v[1].y, v[1].z);
    MeshComp.pVertices[15].set(v[1].x, v[1].y, v[0].z);


    MeshComp.pVertices[16].set(v[0].x, v[0].y, v[0].z);
    MeshComp.pVertices[17].set(v[0].x, v[1].y, v[0].z);
    MeshComp.pVertices[18].set(v[1].x, v[1].y, v[0].z);
    MeshComp.pVertices[19].set(v[1].x, v[0].y, v[0].z);

    MeshComp.pVertices[20].set(v[1].x, v[0].y, v[1].z);
    MeshComp.pVertices[21].set(v[1].x, v[1].y, v[1].z);
    MeshComp.pVertices[22].set(v[0].x, v[1].y, v[1].z);
    MeshComp.pVertices[23].set(v[0].x, v[0].y, v[1].z);



    if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL))
    {
      MeshComp.pNormals = new float3[MeshComp.nVertexCount];
      for(int n = 0; n < 6; n++)
      {
        float3 vNormal(0.0f);
        vNormal.m[n >> 1] = n & 1 ? 1.0f : -1.0f;
        ASSERT(vNormal.lengthsquare() == 1.0f);
        for(int i = 0; i < 4; i++)
        {
          MeshComp.pNormals[n * 4 + i] = vNormal;
        }
      }
    }

    if(TEST_FLAG(dwVertexFlags, GXVF_COLOR))
    {
      MeshComp.pColors32 = new GXColor32[MeshComp.nVertexCount];
      for(int i = 0; i < MeshComp.nVertexCount; i++)
      {
        MeshComp.pColors32[i] = clr;
      }
    }


    if(pGeometry->IntCreateMesh(pGraphics, &MeshComp))
    {
      pGeometry->m_eType = GXPT_TRIANGLELIST;
    }
    else
    {
      pGeometry->Release();
      pGeometry = NULL;
    }

    MeshComp.pIndices = NULL;
    GVMESHDATA::Destroy(&MeshComp);

    *ppGeometry = pGeometry;
    return GX_OK;
  }
  catch(...)
  {
    SAFE_DELETE(pGeometry);
    return GX_FAIL;
  }
}

GXHRESULT GVGeometry::CreateAABB(GXGraphics* pGraphics, const float3& vMin, const float3& vMax, GXCOLOR clr, GVGeometry** ppGeometry)
{
  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_AABB, vMin, vMax);
  if(pGeometry == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  GXHRESULT hval = GX_OK;
  pGeometry->AddRef();

  if( ! pGeometry->InitializeAsAABB(pGraphics, clr))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  *ppGeometry = pGeometry;  
  return hval;
}

GXHRESULT GVGeometry::CreateAxis(GXGraphics* pGraphics, const float3& vPos, float fExtent, int nLevel, GVGeometry** ppGeometry)
{
  GXHRESULT hval = GX_OK;
  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_AXIS, vPos, vPos + fExtent);
  if(pGeometry == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pGeometry->AddRef();

  if( ! pGeometry->InitializeAsAxis(pGraphics, vPos, fExtent, nLevel))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  *ppGeometry = pGeometry;
  return hval;
}

GXHRESULT GVGeometry::CreateQuadPlane(
  GXGraphics*   pGraphics, 
  const float3& vPos,
  const float3& vDirection, 
  const float3& vUp,
  const float2& vExtent,
  GXUINT        xSeg, 
  GXUINT        ySeg, 
  GVGeometry**  ppGeometry, 
  GXDWORD       dwVertexFlags)
{
  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_QUADPLANE);
  if(pGeometry == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  GXHRESULT hval = GX_OK;
  pGeometry->AddRef();

  if( ! pGeometry->InitializeAsQuadPlane(pGraphics, vPos, vDirection, vUp, vExtent, xSeg, ySeg, dwVertexFlags))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }
  *ppGeometry = pGeometry;
  return hval;
}

GXHRESULT GVGeometry::CreateCylinder(
  GXGraphics*   pGraphics, 
  float         fOffset, 
  float         fRadius, 
  float         fHeight, 
  int           nHeightSeg, 
  int           nCapSeg, 
  int           nSides, 
  GXBOOL        bSmooth, 
  GXColor32     color,
  GVGeometry**  ppGeometry,
  float4x4*     pTransform,
  GXDWORD       dwTBS,
  GXDWORD       dwVertexFlags)
{
  return CreateCone(pGraphics, fOffset, fRadius, fRadius, fHeight, 
    nHeightSeg, nCapSeg, nSides, bSmooth, color, ppGeometry, pTransform, dwTBS, dwVertexFlags);
}

typedef clvector<GXVERTEX_P3T2N3F_C1D>  VerticesArray;
typedef clvector<float3>  Vector3Array;
typedef clvector<float2>  Vector2Array;
typedef clvector<GXWORD>  IndicesArray;

// 创建一个圆片
void GenerateCircleSlice(
  float         fRadius, 
  float         yOffset, 
  GXBOOL        bDir, 
  int           nCapSeg, 
  int           nSides, 
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  if(fRadius <= 0 || nCapSeg <= 0 || nSides < 3) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  GXWORD nBaseIdx = (GXWORD)aVertices.size();
  float3 normal = bDir ? float3::AxisY : -float3::AxisY;
  float2 texcoord(0, 0);
  float3 pos(0,yOffset,0);
  
  aVertices.push_back(pos);
  aNormals.push_back(normal);
  aTexcoord.push_back(texcoord);

  for(int n = 0; n < nCapSeg; n++)
  {
    float r = fRadius * (float)(n + 1) / (float)nCapSeg;
    for(int i = 0; i < nSides; i++)
    {
      const float t = bDir 
        ? (float)i * CL_2PI / (float)nSides
        : (float)(nSides - i - 1) * CL_2PI / (float)nSides;
      pos.x = cos(t) * r;
      pos.z = sin(t) * r;
      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);

      if(n == 0)
      {
        aIndices.push_back(nBaseIdx);
        aIndices.push_back(i + 1 >= nSides ? nBaseIdx + 1 : nBaseIdx + i + 2);
        aIndices.push_back(nBaseIdx + i + 1);
      }
    }
  }

  if(nCapSeg > 1)
  {
    const int nFillBase = 1 + nBaseIdx;
    PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nCapSeg - 1, nSides, nFillBase, aIndices);
  }
}

// 创建圆锥的尖端
void GenerateTaperSides(
  float         fRadius, 
  float         fHeight, 
  float         fOffset, 
  int           nHeightSeg, 
  int           nSides, 
  GXBOOL        bDir,
  GXBOOL        bArc,
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  if(fRadius <= 0 || nSides < 3) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  const GXWORD nBaseIdx = (GXWORD)aVertices.size();
  const GXUINT nStart = 2 * nSides + nBaseIdx;
  GXVERTEX_P3T2N3F_C1D VertCenter;
  GXVERTEX_P3T2N3F_C1D Vert;
  const float fHeightBegin = bDir ? fHeight : -fHeight;
  float3 VertCenter_pos(0, fHeightBegin + fOffset , 0);
  float3 VertCenter_normal(0, 0, 0);
  float2 VertCenter_texcoord(0, 0);
  
  float3 pos = VertCenter_pos;
  float3 normal = VertCenter_normal;
  float2 texcoord = VertCenter_texcoord;
  pos.y = fOffset;

  const float fInvHeightSeg = bArc ? (CL_HALF_PI / (float)nHeightSeg) : (1.0f / (float)nHeightSeg);
  for(int n = 1; n <= nHeightSeg; n++)
  {
    float s, r;

    if(bArc)
    {
      float a = (float)n * fInvHeightSeg;
      s = (1.0f - cos(a)) * fRadius;
      r = sqrt((2.0f - s) * s * fRadius * fRadius);  // 不要问为什么,这是优化后的公式
    }
    else
    {
      s = (float)n * fInvHeightSeg;
      r = Lerp(0.0f, fRadius, s);
    }

    pos.y = Lerp(fHeightBegin, 0.0f, s) + fOffset;

    for(int i = 0; i < nSides; i++)
    {
      const float t = bDir 
        ? (float)i * CL_2PI / (float)nSides
        : (float)(nSides - i - 1) * CL_2PI / (float)nSides;

      pos.x = cos(t) * r;
      pos.z = sin(t) * r;
      
      if(n == 1) {
        aVertices.push_back(VertCenter_pos);
        aNormals.push_back(VertCenter_normal);
        aTexcoord.push_back(VertCenter_texcoord);
      }
      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);

      if(n == 1) {
        aIndices.push_back(nBaseIdx + i * 2);
        aIndices.push_back(i + 1 >= nSides ? nBaseIdx + 1 : nBaseIdx + i * 2 + 3);
        aIndices.push_back(nBaseIdx + i * 2 + 1);
      }
      else if(n == 2) {
        const GXWORD a = nBaseIdx + i * 2 + 1;
        const GXWORD c = nStart + i;

        GXWORD b = nBaseIdx + i * 2 + 3;
        GXWORD d = nStart + i + 1;
        if(i + 1 == nSides) {
          b -= nSides * 2;
          d -= nSides;
        }
        aIndices.push_back(a);
        aIndices.push_back(b);
        aIndices.push_back(c);

        aIndices.push_back(c);
        aIndices.push_back(b);
        aIndices.push_back(d);
      }
    }
  }
  PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nHeightSeg - 2, nSides, nStart, aIndices);
}

void GenerateSphereSides(
  float         fRadius, 
  float         fHemisphere,
  float         fOffset, 
  int           nHeightSeg, 
  int           nSides, 
  GXBOOL        bDir,       // 自下而上生成
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  if(fRadius <= 0 || nSides < 3 || fHemisphere >= 1.0f) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  const GXWORD nBaseIdx = (GXWORD)aVertices.size();
  const GXUINT nStart = nSides + nBaseIdx;

  float fHeightBegin = fRadius + fOffset;
  float fHeightEnd = -fRadius + fOffset;
  float3 VertCenter_pos(0, fOffset , 0);
  float2 VertCenter_texcoord(0, 0);

  float3 pos = VertCenter_pos;
  float3 normal;
  float2 texcoord = VertCenter_texcoord;

  if( ! bDir) {
    clSwap(fHeightBegin, fHeightEnd);
  }

  const float fInvHeightSeg = (CL_PI / (float)nHeightSeg);
  const float fHemisphereFactor = (1.0f - fHemisphere) * 2.0f;

  // 半球模式最下面不是用一个顶点封闭，需要（可能的）多一次循环在纬度上产生环形的顶点阵列
  int nTargetSeg = fHemisphere == 0 ? nHeightSeg : nHeightSeg + 1;
  int n;

  for(n = 1; n < nTargetSeg; n++)
  {
    float s = 1.0f - cos((float)n * fInvHeightSeg);
    if(s >= fHemisphereFactor) {
      s = fHemisphereFactor;
      nTargetSeg = n; // 用来终止循环
    }

    const float r = sqrt((2.0f - s) * s) * fRadius;  // 不要问为什么,这是优化后的公式

    pos.y = Lerp(fHeightBegin, (fHeightEnd + fHeightBegin) * 0.5f, s);

    int nTopIndex;

    if(n == 1) {
      aVertices.push_back(float3(VertCenter_pos.x, fHeightBegin, VertCenter_pos.z));
      aNormals.push_back(float3::AxisY);
      aTexcoord.push_back(VertCenter_texcoord);
    }
    else if(n == nHeightSeg - 1) {
      nTopIndex = (int)aVertices.size() + nSides;
    }

    for(int i = 0; i < nSides; i++)
    {
      const float t = bDir 
        ? (float)i * CL_2PI / (float)nSides
        : (float)(nSides - i - 1) * CL_2PI / (float)nSides;

      pos.x = cos(t) * r;
      pos.z = sin(t) * r;

      normal = pos - VertCenter_pos;
      normal.normalize();

      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);

      if(n == 1) {
        aIndices.push_back(nBaseIdx);
        aIndices.push_back(i + 1 >= nSides ? nBaseIdx + 1 : nBaseIdx + i + 2);
        aIndices.push_back(nBaseIdx + i + 1);
      }
      else if(n == nHeightSeg - 1 && fHemisphere == 0.0f ) {
        aIndices.push_back(nTopIndex);
        aIndices.push_back(i + 1 >= nSides ? nTopIndex - 1 : nTopIndex - i - 2);
        aIndices.push_back(nTopIndex - i - 1);
      }
    }
    
    // 球体时，最后封闭球的顶点
    if(n == nHeightSeg - 1 && fHemisphere == 0.0f)
    {
      aVertices.push_back(float3(VertCenter_pos.x, fHeightEnd, VertCenter_pos.z));
      aNormals.push_back(-float3::AxisY);
      aTexcoord.push_back(VertCenter_texcoord);
    }
  }

  // 自下而上生成时会把上下两个顶点的法线翻过来才对
  if( ! bDir) {
    aNormals[nBaseIdx].y = -1.f;
    if(fHemisphere == 0.0f) {
      aNormals.back().y = 1.f;
    }
  }

  if(n - 2 > 0) {
    PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(n - 2, nSides, nBaseIdx + 1, aIndices);
  }
  // 顶点分布是： 自上而下开始，一个顶点-在（经度-纬度）排列的顶点阵列-一个顶点，半球没有最后的一个顶点
  // 面分布是：最上面的圆锥面-最下面的圆锥面-（经度-纬度）排列的球侧面
}

// 创建圆锥面
void GenerateConeSides(
  float         fRadius1, 
  float         fRadius2, 
  float         fHeight, 
  float         fOffset, 
  int           nHeightSeg, 
  int           nSides,
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  if((fRadius1 <= 0 && fRadius2 <= 0)|| nHeightSeg <= 0 || nSides < 3) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  GXWORD nBaseIdx = (GXWORD)aVertices.size();
  //GXVERTEX_P3T2N3F_C1D Vert;
  float3 pos(0, fHeight * 0.5f + fOffset, 0);
  float3 normal(0,0,0);
  float2 texcoord(0, 0);

  const float fInvHeightSeg = 1.0f / (float)nHeightSeg;
  for(int n = 0; n <= nHeightSeg; n++)
  {
    const float s = (float)n * fInvHeightSeg;
    const float r = Lerp(fRadius1, fRadius2, s);
    pos.y = Lerp(fHeight * 0.5f, -fHeight * 0.5f, s) + fOffset;
    for(int i = 0; i < nSides; i++)
    {
      const float t = (float)i * CL_2PI / (float)nSides;

      // 如果生成的是圆柱体,则直接生成法线
      if(fRadius1 == fRadius2) {
        normal.x = cos(t);
        normal.z = sin(t);
        pos.x = normal.x * r;
        pos.z = normal.z * r;
      }
      else {
        pos.x = cos(t) * r;
        pos.z = sin(t) * r;
      }
      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);
    }
  }
  PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nHeightSeg, nSides, nBaseIdx, aIndices);
}

// 创建圆柱面, 这个是基于方向的
void GenerateCylinderSides(
  float         fRadius,      // 半径 
  float         fHeight,      // 高度
  float         fOffset,      // 轴向偏移
  int           nHeightSeg,   // 高度分段
  int           nSides,       // 侧面分段
  const float3& vDir,         // 圆柱朝向
  const float3& vUp,          // 侧面分段开始方向
  float4x4* pTransformation,  // 输出
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  const float d = float3::dot(vDir, vUp);
  if(fRadius == 0 || nHeightSeg <= 0 || nSides < 3 || d == 1.0f || d == -1.0f) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  // 变换矩阵
  float4x4 matT;
  matT.FromDirection(vDir, vUp);


  //matT.LookAtLH(float3::Origin, vDir, vUp);

  // 节的环绕点列表
  float3* pCircles = new float3[nSides];
  for(int i = 0; i < nSides; i++)
  {
    const float t = (float)i * CL_2PI / (float)nSides;
    pCircles[i] = float3(cos(t), sin(-t), 0) * matT;
  }


  GXWORD nBaseIdx = (GXWORD)aVertices.size();
  float3 pos;
  float3 normal(0,0,0);
  float2 texcoord(0, 0);

  const float fInvHeightSeg = 1.0f / (float)nHeightSeg;
  for(int n = 0; n <= nHeightSeg; n++)
  {
    const float t = (float)n * fInvHeightSeg;
    float h = Lerp(fHeight * 0.5f, -fHeight * 0.5f, t) + fOffset;
    texcoord.y = t;
    for(int i = 0; i < nSides; i++)
    {
      texcoord.x = (float)i / (float)nSides;

      normal = pCircles[i];
      pos = pCircles[i] * fRadius + vDir * h;

      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);
    }
  }

  SAFE_DELETE_ARRAY(pCircles);
  PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nHeightSeg, nSides, nBaseIdx, aIndices);

  if(pTransformation) {
    *pTransformation = matT;
  }
}

// 创建圆环
void GenerateTorusSides(
  float         fRadius1, 
  float         fRadius2, 
  int           nSegment, 
  int           nSides, 
  Vector3Array& aVertices, 
  Vector3Array& aNormals, 
  Vector2Array& aTexcoord, 
  IndicesArray& aIndices)
{
  if((fRadius1 <= 0 && fRadius2 <= 0)|| nSegment <= 3 || nSides < 3) {
    return;
  }
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  GXWORD nBaseIdx = (GXWORD)aVertices.size();
  float3 pos(0, 0, 0);
  float3 normal(0,0,0);
  float2 texcoord(0, 0);

  for(int n = 0; n < nSides; n++)
  {
    const float ts = (float)n * CL_2PI / (float)nSides;
    normal.x = cos(ts);
    normal.y = sin(ts);
    normal.z = 0.0f;

    pos.x = normal.x * fRadius2 - fRadius1;
    pos.y = normal.y * fRadius2;
    pos.z = 0.0f;

    const float nr = normal.x;
    const float r = pos.x;
    for(int i = 0; i < nSegment; i++)
    {
      const float t = (float)i * CL_2PI / (float)nSegment;
      const float c = cos(t);
      const float s = sin(t);

      normal.x = c * nr;
      normal.z = s * nr;
      pos.x = c * r;
      pos.z = s * r;

      aVertices.push_back(pos);
      aNormals.push_back(normal);
      aTexcoord.push_back(texcoord);
    }
  }

  const int nTotalVertCount = aVertices.size();
  const int nLastLine = (nSides - 1) * nSegment * 6;
  PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nSides, nSegment, nBaseIdx, aIndices);
  for(IndicesArray::iterator it = aIndices.begin() + nLastLine;
    it != aIndices.end(); ++it) {
    if(*it >= nTotalVertCount) {
      *it -= nTotalVertCount;
    }
  }
}

GXHRESULT GVGeometry::CreateCylinder(
  GXGraphics* pGraphics,
  float fOffset,
  float fRadius,
  float fHeight,
  int nHeightSeg,
  int nCapSeg,
  int nSides,
  GXBOOL bSmooth,
  GXColor32 color,
  const float3& vDir,
  GVGeometry** ppGeometry,
  GXDWORD dwTBS /*= 0x7*/,
  GXDWORD dwVertexFlags /*= (GXVF_NORMAL|GXVF_COLOR|GXVF_TEXCOORD)*/ )
{
  GXHRESULT hval = GX_OK;
  if(fRadius == 0 || dwTBS == 0) {
    hval = GX_FAIL;
    return hval;
  }

  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_CONE);
  if( ! InlCheckNewAndIncReference(pGeometry)) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  Vector3Array aVertices;
  Vector3Array aNormals;
  Vector2Array aTexcoord;
  IndicesArray aIndices;

  const float fHalfHeight = fHeight * .5f;
  const GXDWORD dwTop = 0x1;
  const GXDWORD dwBottom = 0x2;
  const GXDWORD dwSide = 0x4;
  
  float4x4 T;

  if(dwTBS & dwTop){
    GenerateCircleSlice(fRadius, fHalfHeight + fOffset, FALSE, nCapSeg, nSides, 
      aVertices, aNormals, aTexcoord, aIndices);
  }

  if(dwTBS & dwBottom){
    GenerateCircleSlice(fRadius, -fHalfHeight + fOffset, TRUE, nCapSeg, nSides, 
      aVertices, aNormals, aTexcoord, aIndices);
  }

  // 用来转换顶面和底面（如果有的话）
  int nSliceCount = (int)aVertices.size();

  // 选择up方向，如果up与Y轴夹角比较小，就使用X轴
  const float d = float3::dot(vDir, float3::AxisY);
  float3 vUp = (d > 0.7f || d < -0.7f) ? float3::AxisX : float3::AxisY;

  if(dwTBS & dwSide){
    GenerateCylinderSides(fRadius, fHeight, fOffset, nHeightSeg, nSides, 
      vDir, vUp, &T, aVertices, aNormals, aTexcoord, aIndices);
  }
  else {
    T.FromDirection(vDir, vUp);
  }

  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  for(int i = 0; i < nSliceCount; i++)
  {
    float3& pos = aVertices[i];
    pos = float3(pos.x, pos.z, pos.y) * T;

    // 因为变换后三角形顺序发生变化，所以Top和Bottom开始就使用了法线相反
    // 的slice，变换后三角形数据就是正确的了，但是法线相反，于是这里会对y求反
    float3& normal = aNormals[i];
    normal = float3(normal.x, normal.z, -normal.y).MulAsMatrix3x3(T);
  }

  GXColor32* pColor32 = NULL;
  if(dwVertexFlags & GXVF_COLOR)
  {
    pColor32 = new GXColor32[aVertices.size()];
    mesh::SetVertexElement(pColor32, sizeof(GXColor32), sizeof(GXColor32), &color, aVertices.size());
  }

  GVMESHDATA MeshComp = {0};
  MeshComp.nVertexCount = aVertices.size();
  MeshComp.nIndexCount = aIndices.size();
  MeshComp.pVertices = &aVertices.front();
  MeshComp.pIndices = &aIndices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshComp.pNormals = &aNormals.front();
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_COLOR)) {
    MeshComp.pColors32 = pColor32;
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshComp.pTexcoord0 = &aTexcoord.front();
  }

  if( ! pGeometry->IntCreateMesh(pGraphics, &MeshComp))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  pGeometry->m_eType = GXPT_TRIANGLELIST;
  *ppGeometry = pGeometry;
  SAFE_DELETE_ARRAY(pColor32);
  return hval;
}

GXHRESULT GVGeometry::CreateCone(
  GXGraphics*   pGraphics, 
  float         fOffset, 
  float         fRadius1, 
  float         fRadius2, 
  float         fHeight, 
  int           nHeightSeg, 
  int           nCapSeg, 
  int           nSides, 
  GXBOOL        bSmooth, 
  GXColor32     color,
  GVGeometry**  ppGeometry,
  float4x4*     pTransform,
  GXDWORD       dwTBS,
  GXDWORD       dwVertexFlags)
{
  GXHRESULT hval = GX_OK;
  if((fRadius1 <= 0 && fRadius2 <= 0) || dwTBS == 0) {
    hval = GX_FAIL;
    return hval;
  }

  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_CONE);
  if( ! InlCheckNewAndIncReference(pGeometry)) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }

  Vector3Array aVertices;
  Vector3Array aNormals;
  Vector2Array aTexcoord;
  IndicesArray aIndices;

  const float fHalfHeight = fHeight * .5f;
  const GXDWORD dwTop = 0x1;
  const GXDWORD dwBottom = 0x2;
  const GXDWORD dwSide = 0x4;

  if(fRadius1 == 0)  { // 顶面
    if(dwTBS & dwBottom) {
      GenerateCircleSlice(fRadius2, -fHalfHeight + fOffset, FALSE, nCapSeg, nSides, 
        aVertices, aNormals, aTexcoord, aIndices);
    }
    if(dwTBS & dwSide) {
      GenerateTaperSides(fRadius2, fHeight, -fHalfHeight + fOffset, nHeightSeg, nSides, TRUE, FALSE,
        aVertices, aNormals, aTexcoord, aIndices);
    }
  }
  else if(fRadius2 == 0)  { // 底面
    if(dwTBS & dwTop){
      GenerateCircleSlice(fRadius1, fHalfHeight + fOffset, TRUE, nCapSeg, nSides, 
        aVertices, aNormals, aTexcoord, aIndices);
    }
    if(dwTBS & dwSide){
      GenerateTaperSides(fRadius1, fHeight, fHalfHeight + fOffset, nHeightSeg, nSides, FALSE, FALSE,
        aVertices, aNormals, aTexcoord, aIndices);
    }
  }
  else {  // 侧面
    if(dwTBS & dwTop){
      GenerateCircleSlice(fRadius1, fHalfHeight + fOffset, TRUE, nCapSeg, nSides, 
        aVertices, aNormals, aTexcoord, aIndices);
    }
    if(dwTBS & dwBottom){
      GenerateCircleSlice(fRadius2, -fHalfHeight + fOffset, FALSE, nCapSeg, nSides, 
        aVertices, aNormals, aTexcoord, aIndices);
    }
    if(dwTBS & dwSide){
      GenerateConeSides(fRadius1, fRadius2, fHeight, fOffset, nHeightSeg, nSides, 
        aVertices, aNormals, aTexcoord, aIndices);
    }
  }

  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  if(pTransform != NULL)
  {
    for(GXUINT i = 0; i < aVertices.size(); i++)
    {
      float3& pos = aVertices[i];
      pos = pos * (*pTransform);
    }
  }

  //// 不是圆柱体, 就计算合适的法线
  //if(fRadius1 != fRadius2) {
    //mesh::CalculateNormals(&aNormals.front(), &aVertices.front(), 
    //  aVertices.size(), &aIndices.front(), aIndices.size() / 3);
  mesh::CalculateNormals(aNormals, aVertices, aIndices);
  //}

  GXColor32* pColor32 = NULL;
  if(dwVertexFlags & GXVF_COLOR)
  {
    pColor32 = new GXColor32[aVertices.size()];
    mesh::SetVertexElement(pColor32, sizeof(GXColor32), sizeof(GXColor32), &color, aVertices.size());
  }

  GVMESHDATA MeshComp;
  InlSetZeroT(MeshComp);
  //memset(&MeshComp, 0, sizeof(GVMESHDATA));
  MeshComp.nVertexCount = aVertices.size();
  MeshComp.nIndexCount = aIndices.size();
  MeshComp.pVertices = &aVertices.front();
  MeshComp.pIndices = &aIndices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshComp.pNormals = &aNormals.front();
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_COLOR)) {
    MeshComp.pColors32 = pColor32;
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshComp.pTexcoord0 = &aTexcoord.front();
  }

  if( ! pGeometry->IntCreateMesh(pGraphics, &MeshComp))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  pGeometry->m_eType = GXPT_TRIANGLELIST;
  *ppGeometry = pGeometry;
  SAFE_DELETE_ARRAY(pColor32);
  return hval;
}

GXHRESULT GVGeometry::CreateTorus(
  GXGraphics*   pGraphics, 
  float         fRadius1, 
  float         fRadius2, 
  int           nSegment, 
  int           nSides, 
  GXColor32     color, 
  GVGeometry**  ppGeometry, 
  float4x4*     pTransform, 
  GXDWORD       dwVertexFlags /* = */ )
{
  GXHRESULT hval = GX_OK;
  if((fRadius1 <= 0 && fRadius2 <= 0) || nSegment < 3 || nSides < 3) {
    hval = GX_FAIL;
    return hval;
  }

  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_TORUS);
  if(pGeometry == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return GX_FAIL;
  }
  pGeometry->AddRef();

  Vector3Array aVertices;
  Vector3Array aNormals;
  Vector2Array aTexcoord;
  IndicesArray aIndices;

  // 除了这个三角形生成 其他代码都和圆柱/圆锥生成代码基本一样啊~~
  GenerateTorusSides(fRadius1, fRadius2, nSegment, nSides, aVertices, aNormals, aTexcoord, aIndices);

  // ---
  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  if(pTransform != NULL) {
    for(GXUINT i = 0; i < aVertices.size(); i++)
    {
      float3& pos = aVertices[i];
      pos = pos * (*pTransform);
    }

    //mesh::CalculateNormals(&aNormals.front(), &aVertices.front(), 
    //  aVertices.size(), &aIndices.front(), aIndices.size() / 3);
    mesh::CalculateNormals(aNormals, aVertices, aIndices);
  }


  GXColor32* pColor32 = NULL;
  if(dwVertexFlags & GXVF_COLOR)
  {
    pColor32 = new GXColor32[aVertices.size()];
    mesh::SetVertexElement(pColor32, sizeof(GXColor32), sizeof(GXColor32), &color, aVertices.size());
  }

  GVMESHDATA MeshComp;
  InlSetZeroT(MeshComp);
  MeshComp.nVertexCount = aVertices.size();
  MeshComp.nIndexCount = aIndices.size();
  MeshComp.pVertices = &aVertices.front();
  MeshComp.pIndices = &aIndices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshComp.pNormals = &aNormals.front();
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_COLOR)) {
    MeshComp.pColors32 = pColor32;
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshComp.pTexcoord0 = &aTexcoord.front();
  }

  if( ! pGeometry->IntCreateMesh(pGraphics, &MeshComp))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }
  // ---

  pGeometry->m_eType = GXPT_TRIANGLELIST;
  *ppGeometry = pGeometry;
  SAFE_DELETE_ARRAY(pColor32);
  return hval;
}

GXHRESULT GVGeometry::CreateSphere(
  GXGraphics*   pGraphics,
  float         fOffset,
  float         fRadius,
  int           nSegments,
  int           nSides,
  float         fHemisphere,
  GXColor32     color, 
  GVGeometry**  ppGeometry,
  float4x4*     pTransform,
  GXDWORD       dwVertexFlags)
{
  //ASSERT(fHemisphere == 0); // 暂时不支持
  GXHRESULT hval = GX_OK;
  if(fRadius <= 0) {
    hval = GX_FAIL;
    return hval;
  }

  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_CONE);
  if( ! InlCheckNewAndIncReference(pGeometry)) {
    return GX_FAIL;
  }

  Vector3Array aVertices;
  Vector3Array aNormals;
  Vector2Array aTexcoord;
  IndicesArray aIndices;

  GenerateSphereSides(fRadius, fHemisphere, fOffset, nSegments, nSides, TRUE,
    aVertices, aNormals, aTexcoord, aIndices);

  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  if(pTransform != NULL)
  {
    for(GXUINT i = 0; i < aVertices.size(); i++)
    {
      float3& pos = aVertices[i];
      pos = pos * (*pTransform);
    }
  }

  GXColor32* pColor32 = NULL;
  if(dwVertexFlags & GXVF_COLOR)
  {
    pColor32 = new GXColor32[aVertices.size()];
    mesh::SetVertexElement(pColor32, sizeof(GXColor32), sizeof(GXColor32), &color, aVertices.size());
  }

  GVMESHDATA MeshComp = {0};
  MeshComp.nVertexCount = aVertices.size();
  MeshComp.nIndexCount = aIndices.size();
  MeshComp.pVertices = &aVertices.front();
  MeshComp.pIndices = &aIndices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshComp.pNormals = &aNormals.front();
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_COLOR)) {
    MeshComp.pColors32 = pColor32;
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshComp.pTexcoord0 = &aTexcoord.front();
  }

  if( ! pGeometry->IntCreateMesh(pGraphics, &MeshComp))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  pGeometry->m_eType = GXPT_TRIANGLELIST;
  *ppGeometry = pGeometry;
  SAFE_DELETE_ARRAY(pColor32);
  return hval;
}

GXHRESULT GVGeometry::CreateCapsule(
  GXGraphics*   pGraphics,
  float         fOffset,
  float         fRadius,
  float         fHeight,
  int           nSides,
  int           nHeightSeg,
  int           nParallelSeg, // 球的维度分段
  GXBOOL        bSmooth, 
  GXColor32     color, 
  GVGeometry**  ppGeometry, 
  float4x4*     pTransform, 
  GXDWORD       dwTBS,
  GXDWORD       dwVertexFlags)
{
  //ASSERT(fHemisphere == 0); // 暂时不支持
  GXHRESULT hval = GX_OK;
  if(fRadius <= 0) {
    hval = GX_FAIL;
    return hval;
  }

  GVGeometry* pGeometry = new GVGeometry(pGraphics, GT_CONE);
  if( ! InlCheckNewAndIncReference(pGeometry)) {
    return GX_FAIL;
  }

  // 这里为了体现计算过程，所以不要合并计算项
  const GXUINT nMaxVertices = nHeightSeg * nSides + nParallelSeg * nSides + 2;
  const GXUINT nMaxIndices = (nHeightSeg + (nParallelSeg + 1 - 2)) * nSides * 6 + nSides * 3 * 2;

  Vector3Array aVertices;
  Vector3Array aNormals;
  Vector2Array aTexcoord;
  IndicesArray aIndices;

  aVertices.reserve(nMaxVertices);
  aNormals.reserve(nMaxVertices);
  aTexcoord.reserve(nMaxVertices);
  aIndices.reserve(nMaxIndices);

  if(fHeight > fRadius * 2.0f)
  {
    const float fCylinderTop = fHeight * 0.5f - fRadius; // 柱面开始的y值
    GenerateSphereSides(fRadius, 0.5f, fCylinderTop + fOffset, nParallelSeg, nSides, TRUE,
      aVertices, aNormals, aTexcoord, aIndices);

    int nBaseIndex = (int)aVertices.size();
    int nTopIndex = nBaseIndex - nSides;
    if(nHeightSeg == 1)
    {
      // 高度只分为1段的直接组织索引
      PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(1, nSides, nTopIndex, aIndices);
    }
    else if(nHeightSeg > 1)
    {
      // 高度分段的按照球体的边缘数据复制分段
      for(int n = 1; n < nHeightSeg; n++)
      {
        float y = clLerp(fCylinderTop, -fCylinderTop, n / (float)nHeightSeg) + fOffset;
        for(int i = 0; i < nSides; i++)   
        {
          const float3& vPos = aVertices[nTopIndex + i];
          aVertices.push_back(float3(vPos.x, y, vPos.z));
          aNormals.push_back(aNormals[nTopIndex + i]);
          aTexcoord.push_back(aTexcoord[nTopIndex + i]);
        }
      }
      PrimitiveIndicesUtility::FillIndicesAsCycleVertexArray(nHeightSeg, nSides, nTopIndex, aIndices);
    }

    nBaseIndex = (int)aVertices.size();
    int nBegin = (int)aIndices.size();
    GenerateSphereSides(fRadius, 0.5f, -(fCylinderTop) + fOffset, nParallelSeg, nSides, FALSE, aVertices, aNormals, aTexcoord, aIndices);
    mesh::ReverseVerticesArray(&aVertices.front(), nBaseIndex, aVertices.size() - nBaseIndex, &aIndices[nBegin], (aIndices.size() - nBegin) / 3);

    if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
      std::reverse(aNormals.begin() + nBaseIndex, aNormals.end());
    }

    if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
      std::reverse(aTexcoord.begin() + nBaseIndex, aTexcoord.end());
    }
  }
  else // 饼状物体
  {
    float h = fHeight * 0.5f;
    float R = (fRadius * fRadius + h * h) / (2.0f * h);
    float fHemisphere = (R * 2.0f - h) / (R * 2.0f);
    TRACE("h=%f,R=%f,fHemisphere=%f\n", h, R, fHemisphere);

    GenerateSphereSides(R, fHemisphere, -(R - h) + fOffset, nParallelSeg, nSides, TRUE,
      aVertices, aNormals, aTexcoord, aIndices);

    GenerateSphereSides(R, fHemisphere, (R - h) + fOffset, nParallelSeg, nSides, FALSE,
      aVertices, aNormals, aTexcoord, aIndices);
  }

  // 如果断言失败说明估算顶点/索引的公式有误
  ASSERT(aVertices.size() <= nMaxVertices);
  ASSERT(aIndices.size() <= nMaxIndices);

  ASSERT(aVertices.size() == aNormals.size() &&
    aVertices.size() == aTexcoord.size());

  if(pTransform != NULL) {
    for(GXUINT i = 0; i < aVertices.size(); i++) {
      float3& pos = aVertices[i];
      pos = pos * (*pTransform);
    }
  }

  GXColor32* pColor32 = NULL;
  if(dwVertexFlags & GXVF_COLOR)
  {
    pColor32 = new GXColor32[aVertices.size()];
    mesh::SetVertexElement(pColor32, sizeof(GXColor32), sizeof(GXColor32), &color, aVertices.size());
  }

  GVMESHDATA MeshComp = {0};
  MeshComp.nVertexCount = aVertices.size();
  MeshComp.nIndexCount = aIndices.size();
  MeshComp.pVertices = &aVertices.front();
  MeshComp.pIndices = &aIndices.front();

  if(TEST_FLAG(dwVertexFlags, GXVF_NORMAL)) {
    MeshComp.pNormals = &aNormals.front();
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_COLOR)) {
    MeshComp.pColors32 = pColor32;
  }
  if(TEST_FLAG(dwVertexFlags, GXVF_TEXCOORD)) {
    MeshComp.pTexcoord0 = &aTexcoord.front();
  }

  if( ! pGeometry->IntCreateMesh(pGraphics, &MeshComp))
  {
    pGeometry->Release();
    pGeometry = NULL;
    hval = GX_FAIL;
  }

  pGeometry->m_eType = GXPT_TRIANGLELIST;
  *ppGeometry = pGeometry;
  SAFE_DELETE_ARRAY(pColor32);
  return hval;
}

GXHRESULT GVGeometry::CreateConvex(
  GXGraphics*   pGraphics,
  Plane*        pPlanes,
  int           nNumPlanes,
  GXColor32     color, 
  GVGeometry**  ppGeometry, 
  float4x4*     pTransform,
  GXDWORD       dwVertexFlags)
{
  return GX_FAIL;
}

namespace PrimitiveIndicesUtility
{
  template<typename _VIndexTy>
  void FillIndicesAsQuadVertexArrayCW(int xSeg, int ySeg, int nPitch, int nBaseIndex, clvector<_VIndexTy>& aIndices)
  {
    ASSERT(xSeg + 1 <= nPitch);
    for(int y = 0; y < ySeg; y++)
    {
      const GXUINT nStart = y * nPitch + nBaseIndex;
      for(int x = 0; x < xSeg; x++)
      {
        const _VIndexTy a = nStart + x;
        const _VIndexTy b = a + 1;
        const _VIndexTy c = a + nPitch;
        const _VIndexTy d = c + 1;
        aIndices.push_back(a);
        aIndices.push_back(b);
        aIndices.push_back(c);

        aIndices.push_back(c);
        aIndices.push_back(b);
        aIndices.push_back(d);
      }
    }
  }

  template<typename _VIndexTy>
  void FillIndicesAsQuadVertexArrayCCW(int xSeg, int ySeg, int nPitch, int nBaseIndex, clvector<_VIndexTy>& aIndices)
  {
    ASSERT(xSeg + 1 <= nPitch);
    for(int y = 0; y < ySeg; y++)
    {
      const GXUINT nStart = y * nPitch + nBaseIndex;
      for(int x = 0; x < xSeg; x++)
      {
        const _VIndexTy a = nStart + x;
        const _VIndexTy b = a + 1;
        const _VIndexTy c = a + nPitch;
        const _VIndexTy d = c + 1;
        aIndices.push_back(c);
        aIndices.push_back(b);
        aIndices.push_back(a);

        aIndices.push_back(d);
        aIndices.push_back(b);
        aIndices.push_back(c);
      }
    }
  }

  void FillIndicesAsQuadVertexArrayCW16(int xSeg, int ySeg, int nBaseIndex, clvector<u16>& aIndices, int nPitch)
  {
    FillIndicesAsQuadVertexArrayCW<u16>(xSeg, ySeg, (nPitch == 0) ? (xSeg + 1) : nPitch, nBaseIndex, aIndices);
  }

  void FillIndicesAsQuadVertexArrayCW32(int xSeg, int ySeg, int nBaseIndex, clvector<u32>& aIndices, int nPitch)
  {
    FillIndicesAsQuadVertexArrayCW<u32>(xSeg, ySeg, (nPitch == 0) ? (xSeg + 1) : nPitch, nBaseIndex, aIndices);
  }

  void FillIndicesAsQuadVertexArrayCCW16(int xSeg, int ySeg, int nBaseIndex, clvector<u16>& aIndices, int nPitch)
  {
    FillIndicesAsQuadVertexArrayCCW<u16>(xSeg, ySeg, (nPitch == 0) ? (xSeg + 1) : nPitch, nBaseIndex, aIndices);
  }

  void FillIndicesAsQuadVertexArrayCCW32(int xSeg, int ySeg, int nBaseIndex, clvector<u32>& aIndices, int nPitch)
  {
    FillIndicesAsQuadVertexArrayCCW<u32>(xSeg, ySeg, (nPitch == 0) ? (xSeg + 1) : nPitch, nBaseIndex, aIndices);
  }

  void FillIndicesAsCycleVertexArray(int nHeightSeg, int nSides, int nBaseIndex, clvector<GXWORD>& aIndices)
  {
    for(int y = 0; y < nHeightSeg; y++)
    {
      const GXUINT nStart = y * nSides + nBaseIndex;
      for(int x = 0; x < nSides; x++)
      {
        const GXWORD a = nStart + x;
        const GXWORD c = a + nSides;

        GXWORD b = a + 1;
        GXWORD d = c + 1;
        if(x + 1 == nSides) {
          b -= nSides;
          d -= nSides;
        }
        aIndices.push_back(a);
        aIndices.push_back(b);
        aIndices.push_back(c);

        aIndices.push_back(c);
        aIndices.push_back(b);
        aIndices.push_back(d);
      }
    }
  }
} // namespace PrimitiveIndicesUtility